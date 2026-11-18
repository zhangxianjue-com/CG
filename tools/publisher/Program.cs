using System.Diagnostics;
using System.Net;
using System.Net.Http.Headers;
using System.Text;
using System.Text.Json;

namespace CGPublish;

/// <summary>
/// CG 笔记发布器：一键把 notes/ 里的 Excalidraw 笔记构建成 VitePress 站点并发布到 GitHub Pages。
/// GUI（双击运行）或 CLI（CG笔记发布器.exe --cli all）两种用法。
/// </summary>
internal static class Program
{
    [STAThread]
    private static async Task<int> Main(string[] args)
    {
        if (args.Length > 0 && args[0] == "--cli")
        {
            return await Cli.MainAsync(args.Skip(1).ToArray());
        }

        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        Application.SetHighDpiMode(HighDpiMode.PerMonitorV2);
        Application.Run(new MainForm());
        return 0;
    }
}

/// <summary>发布流程引擎（GUI 与 CLI 共用）</summary>
internal sealed class PublishEngine
{
    public Action<string, bool>? Log; // (line, isError)

    /// <summary>从起始目录向上找 .git 仓库根</summary>
    public static string? FindRepoRoot(string start)
    {
        var d = new DirectoryInfo(start);
        while (d != null)
        {
            if (Directory.Exists(Path.Combine(d.FullName, ".git")))
                return d.FullName;
            d = d.Parent;
        }
        return null;
    }

    private static string DocsDir(string repoRoot) => Path.Combine(repoRoot, "docs");
    private static string DistDir(string repoRoot) => Path.Combine(DocsDir(repoRoot), ".vitepress", "dist");
    private static string TempDir(string repoRoot) => Path.Combine(repoRoot, ".publish-github");

    private void Info(string line) => Log?.Invoke(line, false);
    private void Err(string line) => Log?.Invoke(line, true);

    /// <summary>用 cmd 包裹执行命令（统一 PATH 解析，chcp 65001 保证 UTF-8 输出）</summary>
    private async Task<bool> RunAsync(string workDir, string cmdline)
    {
        var psi = new ProcessStartInfo
        {
            FileName = "cmd.exe",
            Arguments = $"/d /s /c \"{cmdline}\"",
            WorkingDirectory = workDir,
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            StandardOutputEncoding = Encoding.UTF8,
            StandardErrorEncoding = Encoding.UTF8,
            CreateNoWindow = true,
        };
        using var p = new Process { StartInfo = psi };
        p.OutputDataReceived += (_, e) => { if (e.Data != null) Info(e.Data); };
        p.ErrorDataReceived += (_, e) => { if (e.Data != null) Err(e.Data); };
        try
        {
            p.Start();
        }
        catch (Exception ex)
        {
            Err($"无法启动命令：{cmdline}\n{ex.Message}");
            return false;
        }
        p.BeginOutputReadLine();
        p.BeginErrorReadLine();
        await p.WaitForExitAsync();
        return p.ExitCode == 0;
    }

    private async Task<string?> CaptureAsync(string workDir, string cmdline)
    {
        var psi = new ProcessStartInfo
        {
            FileName = "cmd.exe",
            Arguments = $"/d /s /c \"{cmdline}\"",
            WorkingDirectory = workDir,
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            StandardOutputEncoding = Encoding.UTF8,
            CreateNoWindow = true,
        };
        using var p = Process.Start(psi);
        if (p == null) return null;
        var stdout = await p.StandardOutput.ReadToEndAsync();
        await p.WaitForExitAsync();
        return stdout.Trim();
    }

    /// <summary>读取远端仓库 URL（git remote get-url origin）</summary>
    public async Task<string?> GetRemoteUrlAsync(string repoRoot) =>
        await CaptureAsync(repoRoot, "git remote get-url origin");

    /// <summary>解析 owner/repo，如 git@github.com:zhangxianjue-com/CG.git → (zhangxianjue-com, CG)</summary>
    public static (string owner, string repo)? ParseOwnerRepo(string remoteUrl)
    {
        var u = remoteUrl.Trim();
        if (u.StartsWith("git@")) u = u[(u.IndexOf(':') + 1)..];
        else if (u.StartsWith("ssh://"))
        {
            u = u[(u.IndexOf('@') + 1)..].TrimStart('/');
            var i = u.IndexOf('/');
            if (i >= 0) u = u[(i + 1)..];
        }
        else
        {
            var i = u.IndexOf("://");
            if (i < 0) return null;
            u = u[(i + 3)..];
            i = u.IndexOf('/');
            if (i < 0) return null;
            u = u[(i + 1)..];
        }
        if (u.EndsWith(".git")) u = u[..^4];
        var parts = u.Split('/', StringSplitOptions.RemoveEmptyEntries);
        if (parts.Length != 2 || parts[0].Contains(':') || parts[0].Contains('\\')) return null;
        return (parts[0], parts[1]);
    }

    /// <summary>Pages 站点地址：项目页 https://owner.github.io/repo/，用户页为根路径</summary>
    public static string SiteUrl(string owner, string repo) =>
        repo.Equals($"{owner}.github.io", StringComparison.OrdinalIgnoreCase)
            ? $"https://{owner}.github.io/"
            : $"https://{owner}.github.io/{repo}/";

    /// <summary>构建：转换笔记 + VitePress 打包（首次自动 npm install）</summary>
    public async Task<bool> BuildAsync(string repoRoot)
    {
        var docs = DocsDir(repoRoot);
        if (!Directory.Exists(docs))
        {
            Err($"找不到 docs 目录：{docs}");
            return false;
        }
        Info("→ 检查 Node.js");
        if (!await RunAsync(docs, "node --version"))
        {
            Err("未找到 Node.js，请先安装（https://nodejs.org）并加入 PATH");
            return false;
        }
        if (!Directory.Exists(Path.Combine(docs, "node_modules")))
        {
            Info("→ 首次构建：安装依赖 npm install");
            if (!await RunAsync(docs, "npm install"))
            {
                Err("依赖安装失败，请检查网络后重试");
                return false;
            }
        }
        Info("→ 转换笔记并构建 VitePress 站点（npm run docs:build）");
        if (!await RunAsync(docs, "npm run docs:build"))
        {
            Err("站点构建失败");
            return false;
        }
        Info("✓ 构建完成：" + DistDir(repoRoot));
        return true;
    }

    /// <summary>发布：把 dist 推送到 gh-pages 分支（GitHub Pages 的部署分支）</summary>
    public async Task<bool> PublishAsync(string repoRoot, string remoteUrl)
    {
        if (string.IsNullOrWhiteSpace(remoteUrl))
        {
            Err("远端仓库地址为空，请先填写（git remote get-url origin 获取）");
            return false;
        }
        var dist = DistDir(repoRoot);
        if (!Directory.Exists(dist) || !File.Exists(Path.Combine(dist, "index.html")))
        {
            Err("构建产物不存在，请先构建站点");
            return false;
        }
        var temp = TempDir(repoRoot);
        if (Directory.Exists(temp)) Directory.Delete(temp, true);
        Directory.CreateDirectory(temp);

        Info("→ 初始化 gh-pages 部署分支");
        if (!await RunAsync(temp, "git init -b gh-pages")) return false;
        if (!await RunAsync(temp, $"git remote add origin {remoteUrl}")) return false;

        Info("→ 复制构建产物");
        CopyDir(dist, temp);

        Info("→ 提交并推送 gh-pages 分支");
        if (!await RunAsync(temp, "git add -A")) return false;
        var msg = $"publish: {DateTime.Now:yyyy-MM-dd HH:mm:ss}";
        if (!await RunAsync(temp, $"git -c user.name=CGPublish -c user.email=publish@cg.local commit -m \"{msg}\""))
        {
            Err("提交失败（可能没有文件变化）");
            return false;
        }
        if (!await RunAsync(temp, "git push --force origin gh-pages"))
        {
            Err("推送失败：请确认 SSH 密钥（~/.ssh）有该仓库的写权限");
            return false;
        }
        Info("✓ 已推送到 gh-pages 分支");
        return true;
    }

    private static void CopyDir(string src, string dst)
    {
        Directory.CreateDirectory(dst);
        foreach (var f in Directory.GetFiles(src))
            File.Copy(f, Path.Combine(dst, Path.GetFileName(f)), true);
        foreach (var d in Directory.GetDirectories(src))
            CopyDir(d, Path.Combine(dst, Path.GetFileName(d)));
    }

    /// <summary>通过 GitHub API 开启/核对 Pages 的部署分支设置（需要 Token）</summary>
    public async Task<string> CheckPagesAsync(string owner, string repo, string? token)
    {
        var http = new HttpClient();
        http.DefaultRequestHeaders.UserAgent.ParseAdd("CGPublish/1.0");
        if (!string.IsNullOrWhiteSpace(token))
            http.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", token);
        var url = $"https://api.github.com/repos/{owner}/{repo}/pages";
        try
        {
            using var get = await http.GetAsync(url);
            if (get.StatusCode == HttpStatusCode.NotFound)
            {
                return "Pages 尚未开启：请在仓库 Settings → Pages 里选择 Source = gh-pages 分支（只做一次），\n" +
                       "或在发布器里填入有 repo 权限的 GitHub Token 后点击「检查/开启 Pages」。";
            }
            if (get.StatusCode == HttpStatusCode.Unauthorized)
            {
                return "Token 无效或没有仓库权限（401）";
            }
            if (!get.IsSuccessStatusCode)
            {
                return $"查询失败 HTTP {(int)get.StatusCode}：{await get.Content.ReadAsStringAsync()}";
            }
            var json = JsonDocument.Parse(await get.Content.ReadAsStringAsync());
            var branch = json.RootElement.TryGetProperty("source", out var src) &&
                         src.TryGetProperty("branch", out var b) ? b.GetString() : null;
            var status = json.RootElement.TryGetProperty("status", out var st) ? st.GetString() : null;
            var htmlUrl = json.RootElement.TryGetProperty("html_url", out var h) ? h.GetString() : null;
            var site = htmlUrl ?? SiteUrl(owner, repo);
            return branch == "gh-pages"
                ? $"✓ Pages 已开启（分支：gh-pages，状态：{status}）\n站点地址：{site}"
                : $"⚠ Pages 已开启但部署分支是「{branch}」，请在仓库设置里改成 gh-pages。\n站点地址：{site}";
        }
        catch (Exception ex)
        {
            return $"查询异常：{ex.Message}";
        }
    }

    /// <summary>用 Token 自动把 Pages 部署分支设为 gh-pages</summary>
    public async Task<string> EnablePagesAsync(string owner, string repo, string token)
    {
        if (string.IsNullOrWhiteSpace(token))
            return "未填写 Token：请在仓库 Settings → Pages 手动选择 Source = gh-pages 分支（只做一次）。";
        var http = new HttpClient();
        http.DefaultRequestHeaders.UserAgent.ParseAdd("CGPublish/1.0");
        http.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", token);
        var url = $"https://api.github.com/repos/{owner}/{repo}/pages";
        const string body = "{\"source\":{\"branch\":\"gh-pages\",\"path\":\"/\"}}";
        try
        {
            using var get = await http.GetAsync(url);
            var payload = new StringContent(body, Encoding.UTF8, "application/json");
            HttpResponseMessage resp;
            if (get.StatusCode == HttpStatusCode.NotFound)
            {
                resp = await http.PostAsync(url, payload); // 新建 Pages 站点
            }
            else if (get.StatusCode == HttpStatusCode.Unauthorized)
            {
                return "Token 无效或没有仓库权限（401）";
            }
            else if (get.IsSuccessStatusCode)
            {
                var json = JsonDocument.Parse(await get.Content.ReadAsStringAsync());
                var branch = json.RootElement.TryGetProperty("source", out var src) &&
                             src.TryGetProperty("branch", out var b) ? b.GetString() : null;
                if (branch == "gh-pages") return "✓ Pages 已配置为 gh-pages 分支，无需修改";
                using var req = new HttpRequestMessage(HttpMethod.Put, url) { Content = payload };
                resp = await http.SendAsync(req); // 更新部署分支
            }
            else
            {
                return $"查询失败 HTTP {(int)get.StatusCode}";
            }
            var text = await resp.Content.ReadAsStringAsync();
            return resp.IsSuccessStatusCode
                ? $"✓ Pages 已开启（gh-pages 分支）。首次部署约 1 分钟后可访问：{SiteUrl(owner, repo)}"
                : $"开启失败 HTTP {(int)resp.StatusCode}：{text}";
        }
        catch (Exception ex)
        {
            return $"开启异常：{ex.Message}";
        }
    }

    /// <summary>访问站点，确认部署生效</summary>
    public async Task<string> VerifyAsync(string owner, string repo)
    {
        var url = SiteUrl(owner, repo);
        try
        {
            using var resp = await new HttpClient().GetAsync(url);
            var code = (int)resp.StatusCode;
            if (code == 200)
                return $"✓ 站点可访问：{url}";
            return $"站点暂时返回 HTTP {code}（首次开启 Pages 需要约 1 分钟构建）：{url}";
        }
        catch (Exception ex)
        {
            return $"访问失败（{ex.Message}）：{url}";
        }
    }
}

/// <summary>命令行模式：CG笔记发布器.exe --cli [build|publish|all|pages|enable|verify|open] [--repo 路径] [--remote url] [--token xxx]</summary>
internal static class Cli
{
    public static async Task<int> MainAsync(string[] args)
    {
        // WinExe 无控制台句柄，Console.OutputEncoding 会静默失败；直接替换输出流为 UTF-8
        try
        {
            Console.SetOut(new StreamWriter(Console.OpenStandardOutput(), new UTF8Encoding(false)) { AutoFlush = true });
            Console.SetError(new StreamWriter(Console.OpenStandardError(), new UTF8Encoding(false)) { AutoFlush = true });
        }
        catch { /* 输出不可用时忽略 */ }
        var cmd = args.FirstOrDefault(a => !a.StartsWith("--")) ?? "all";
        string? repo = null, remote = null, token = null;
        for (var i = 0; i < args.Length; i++)
        {
            if (args[i] == "--repo" && i + 1 < args.Length) repo = args[++i];
            else if (args[i] == "--remote" && i + 1 < args.Length) remote = args[++i];
            else if (args[i] == "--token" && i + 1 < args.Length) token = args[++i];
        }

        var engine = new PublishEngine { Log = (line, isError) => Console.WriteLine(isError ? $"[ERR] {line}" : line) };
        var repoRoot = repo ?? PublishEngine.FindRepoRoot(AppContext.BaseDirectory);
        if (repoRoot == null)
        {
            Console.WriteLine("找不到仓库根目录（.git），请用 --repo 指定");
            return 2;
        }
        Console.WriteLine($"仓库：{repoRoot}");
        remote ??= await engine.GetRemoteUrlAsync(repoRoot);

        switch (cmd)
        {
            case "build":
                return await engine.BuildAsync(repoRoot) ? 0 : 1;

            case "publish":
                return await engine.PublishAsync(repoRoot, remote ?? "") ? 0 : 1;

            case "all":
            {
                if (!await engine.BuildAsync(repoRoot)) return 1;
                if (!await engine.PublishAsync(repoRoot, remote ?? "")) return 1;
                var parsed = PublishEngine.ParseOwnerRepo(remote ?? "");
                if (parsed == null)
                {
                    Console.WriteLine("无法从远端地址解析 owner/repo，跳过 Pages 检查");
                    return 0;
                }
                var (owner, repoName) = parsed.Value;
                Console.WriteLine(await engine.EnablePagesAsync(owner, repoName, token ?? ""));
                Console.WriteLine(await engine.VerifyAsync(owner, repoName));
                return 0;
            }

            case "pages":
            case "verify":
            {
                var parsed = PublishEngine.ParseOwnerRepo(remote ?? "");
                if (parsed == null)
                {
                    Console.WriteLine("无法从远端地址解析 owner/repo");
                    return 2;
                }
                var (owner, repoName) = parsed.Value;
                Console.WriteLine(cmd == "pages"
                    ? await engine.CheckPagesAsync(owner, repoName, token)
                    : await engine.VerifyAsync(owner, repoName));
                return 0;
            }

            case "enable":
            {
                var parsed = PublishEngine.ParseOwnerRepo(remote ?? "");
                if (parsed == null)
                {
                    Console.WriteLine("无法从远端地址解析 owner/repo");
                    return 2;
                }
                var (owner, repoName) = parsed.Value;
                Console.WriteLine(await engine.EnablePagesAsync(owner, repoName, token ?? ""));
                return 0;
            }

            case "open":
            {
                var parsed = PublishEngine.ParseOwnerRepo(remote ?? "");
                if (parsed == null)
                {
                    Console.WriteLine("无法从远端地址解析 owner/repo");
                    return 2;
                }
                var (owner, repoName) = parsed.Value;
                Process.Start(new ProcessStartInfo(PublishEngine.SiteUrl(owner, repoName)) { UseShellExecute = true });
                return 0;
            }

            default:
                Console.WriteLine("用法：CG笔记发布器.exe --cli [build|publish|all|pages|enable|verify|open] [--repo 路径] [--remote url] [--token xxx]");
                return 2;
        }
    }
}

/// <summary>主窗口</summary>
internal sealed class MainForm : Form
{
    private readonly TextBox _repoBox = new() { Anchor = AnchorStyles.Left | AnchorStyles.Right };
    private readonly TextBox _remoteBox = new() { Anchor = AnchorStyles.Left | AnchorStyles.Right };
    private readonly TextBox _tokenBox = new() { Anchor = AnchorStyles.Left | AnchorStyles.Right, UseSystemPasswordChar = true };
    private readonly RichTextBox _logBox = new()
    {
        ReadOnly = true,
        BackColor = Color.FromArgb(30, 30, 30),
        ForeColor = Color.Gainsboro,
        Font = new Font("Consolas", 9f),
        DetectUrls = true,
        Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right,
        BorderStyle = BorderStyle.None,
    };
    private readonly Label _statusLabel = new() { AutoSize = true };
    private readonly Button _btnAll, _btnBuild, _btnPublish, _btnPages, _btnOpen;
    private bool _busy;

    public MainForm()
    {
        Text = "CG 笔记发布器 — Excalidraw 笔记 → VitePress → GitHub Pages";
        Font = new Font("Microsoft YaHei UI", 9f);
        AutoScaleMode = AutoScaleMode.Dpi;
        ClientSize = new Size(820, 600);
        MinimumSize = new Size(720, 480);
        StartPosition = FormStartPosition.CenterScreen;

        var repoLabel = new Label { Text = "仓库目录", AutoSize = true };
        var btnBrowse = new Button { Text = "浏览…", AutoSize = true };
        btnBrowse.Click += (_, _) =>
        {
            using var dlg = new FolderBrowserDialog { Description = "选择 CG 仓库根目录（含 .git）" };
            if (dlg.ShowDialog(this) == DialogResult.OK) _repoBox.Text = dlg.SelectedPath;
        };
        var remoteLabel = new Label { Text = "远端仓库", AutoSize = true };
        var tokenLabel = new Label { Text = "GitHub Token（可选：自动开启 Pages 用）", AutoSize = true };

        _btnAll = new Button { Text = "🚀 一键构建并发布", AutoSize = true, BackColor = Color.FromArgb(46, 160, 67), ForeColor = Color.White, FlatStyle = FlatStyle.Flat, Padding = new Padding(10, 4, 10, 4) };
        _btnBuild = new Button { Text = "构建站点", AutoSize = true };
        _btnPublish = new Button { Text = "发布到 GitHub Pages", AutoSize = true };
        _btnPages = new Button { Text = "检查/开启 Pages", AutoSize = true };
        _btnOpen = new Button { Text = "打开站点", AutoSize = true };

        var btnClear = new Button { Text = "清空日志", AutoSize = true };
        btnClear.Click += (_, _) => _logBox.Clear();

        _btnAll.Click += async (_, _) => await RunAsync(async () =>
        {
            if (!await engine().BuildAsync(Repo)) return false;
            if (!await engine().PublishAsync(Repo, Remote)) return false;
            var parsed = PublishEngine.ParseOwnerRepo(Remote);
            if (parsed != null)
            {
                var (o, r) = parsed.Value;
                var enableResult = await engine().EnablePagesAsync(o, r, Token);
                Append(enableResult, enableResult.StartsWith("✓"));
                Append(await engine().VerifyAsync(o, r), false);
                _siteUrl = PublishEngine.SiteUrl(o, r);
            }
            return true;
        }, "发布完成");

        _btnBuild.Click += async (_, _) => await RunAsync(() => engine().BuildAsync(Repo), "构建完成");
        _btnPublish.Click += async (_, _) => await RunAsync(() => engine().PublishAsync(Repo, Remote), "推送完成");
        _btnPages.Click += async (_, _) => await RunAsync(async () =>
        {
            var parsed = PublishEngine.ParseOwnerRepo(Remote);
            if (parsed == null)
            {
                Append("远端地址无法解析出 owner/repo", true);
                return false;
            }
            var (o, r) = parsed.Value;
            var result = string.IsNullOrWhiteSpace(Token)
                ? await engine().CheckPagesAsync(o, r, Token)
                : await engine().EnablePagesAsync(o, r, Token);
            Append(result, result.StartsWith("✓"));
            return result.StartsWith("✓");
        }, null);
        _btnOpen.Click += (_, _) =>
        {
            if (!string.IsNullOrWhiteSpace(_siteUrl))
                Process.Start(new ProcessStartInfo(_siteUrl) { UseShellExecute = true });
        };

        // 布局：TableLayoutPanel
        var root = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 1, RowCount = 2, Padding = new Padding(10) };
        root.RowStyles.Add(new RowStyle(SizeType.Absolute, 150));
        root.RowStyles.Add(new RowStyle(SizeType.Percent, 100));

        var top = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 3, RowCount = 4, AutoSize = true };
        top.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 120));
        top.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        top.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 90));
        top.Controls.Add(repoLabel, 0, 0);
        top.Controls.Add(_repoBox, 1, 0);
        top.Controls.Add(btnBrowse, 2, 0);
        top.Controls.Add(remoteLabel, 0, 1);
        top.Controls.Add(_remoteBox, 1, 1);
        top.Controls.Add(new Panel(), 2, 1);
        top.Controls.Add(tokenLabel, 0, 2);
        top.Controls.Add(_tokenBox, 1, 2);
        top.Controls.Add(new Panel(), 2, 2);
        var btns = new FlowLayoutPanel { Dock = DockStyle.Fill, AutoSize = true, WrapContents = true, Margin = new Padding(0, 8, 0, 0) };
        btns.Controls.AddRange(new Control[] { _btnAll, _btnBuild, _btnPublish, _btnPages, _btnOpen, btnClear });
        top.Controls.Add(btns, 1, 3);
        top.SetColumnSpan(btns, 2);

        var bottom = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 1, RowCount = 2 };
        bottom.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
        bottom.RowStyles.Add(new RowStyle(SizeType.Absolute, 26));
        bottom.Controls.Add(_logBox, 0, 0);
        bottom.Controls.Add(_statusLabel, 0, 1);

        root.Controls.Add(top, 0, 0);
        root.Controls.Add(bottom, 0, 1);
        Controls.Add(root);

        Shown += async (_, _) =>
        {
            var repo = PublishEngine.FindRepoRoot(AppContext.BaseDirectory);
            if (repo != null)
            {
                _repoBox.Text = repo;
                var remote = await Task.Run(() => engine().GetRemoteUrlAsync(repo));
                if (!string.IsNullOrWhiteSpace(remote))
                {
                    _remoteBox.Text = remote;
                    var parsed = PublishEngine.ParseOwnerRepo(remote);
                    if (parsed != null) _siteUrl = PublishEngine.SiteUrl(parsed.Value.owner, parsed.Value.repo);
                }
            }
            Append($"CG 笔记发布器已就绪。仓库：{_repoBox.Text}", false);
            Append("流程：转换笔记 → VitePress 构建 → 推送 gh-pages 分支（GitHub Pages 会自动部署该分支）", false);
            if (string.IsNullOrWhiteSpace(Token))
                Append("提示：未填 Token 时，首次需在仓库 Settings → Pages 手动选择 gh-pages 分支（只做一次）", false);
        };
    }

    private string Repo => _repoBox.Text.Trim();
    private string Remote => _remoteBox.Text.Trim();
    private string Token => _tokenBox.Text.Trim();
    private string? _siteUrl;

    private PublishEngine engine()
    {
        var e = new PublishEngine
        {
            Log = (line, isError) =>
            {
                if (IsDisposed) return;
                try { BeginInvoke(() => Append(line, isError)); } catch (InvalidOperationException) { }
            },
        };
        return e;
    }

    private async Task RunAsync(Func<Task<bool>> work, string? doneText)
    {
        if (_busy) return;
        _busy = true;
        SetButtons(false);
        _statusLabel.Text = "运行中…";
        try
        {
            var ok = await Task.Run(work);
            _statusLabel.Text = doneText != null ? (ok ? "✓ " + doneText : "✗ 失败，详见日志") : (ok ? "✓ 完成" : "✗ 失败，详见日志");
            _statusLabel.ForeColor = ok ? Color.FromArgb(46, 160, 67) : Color.Firebrick;
        }
        catch (Exception ex)
        {
            Append(ex.ToString(), true);
            _statusLabel.Text = "✗ 出错";
            _statusLabel.ForeColor = Color.Firebrick;
        }
        finally
        {
            _busy = false;
            SetButtons(true);
        }
    }

    private void SetButtons(bool enabled)
    {
        _btnAll.Enabled = _btnBuild.Enabled = _btnPublish.Enabled = _btnPages.Enabled = _btnOpen.Enabled = enabled;
    }

    private void Append(string line, bool isError)
    {
        _logBox.SelectionStart = _logBox.TextLength;
        _logBox.SelectionColor = isError ? Color.FromArgb(255, 123, 114) : Color.Gainsboro;
        _logBox.AppendText(line + Environment.NewLine);
        _logBox.ScrollToCaret();
    }
}
