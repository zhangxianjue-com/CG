using System.Diagnostics;
using System.Net;
using System.Text;

namespace CGPublish;

/// <summary>
/// CG 笔记发布器：一键把 notes/ 里的 Excalidraw 笔记构建成 VitePress 站点并发布到 GitHub Pages。
/// 部署方式：把构建产物提交进 main 分支并推送（GitHub Pages 部署 main 分支根目录，根 index.html 跳转到站点）。
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

    private void Info(string line) => Log?.Invoke(line, false);
    private void Err(string line) => Log?.Invoke(line, true);

    /// <summary>用 cmd 包裹执行命令（统一 PATH 解析）</summary>
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

    /// <summary>发布：把构建产物提交进 main 分支并推送（GitHub Pages 部署 main 分支根目录）</summary>
    public async Task<bool> PublishAsync(string repoRoot)
    {
        var dist = DistDir(repoRoot);
        if (!Directory.Exists(dist) || !File.Exists(Path.Combine(dist, "index.html")))
        {
            Err("构建产物不存在，请先构建站点");
            return false;
        }
        Info("→ 拉取远端更新（git pull --rebase origin main）");
        if (!await RunAsync(repoRoot, "git pull --rebase origin main"))
        {
            Err("拉取失败：如有未提交的修改请先处理，然后重试");
            return false;
        }
        Info("→ 暂存构建产物（git add -f docs/.vitepress/dist）");
        if (!await RunAsync(repoRoot, "git add -f docs/.vitepress/dist")) return false;

        var hasChanges = await CaptureAsync(repoRoot, "git diff --cached --quiet && echo YES || echo NO");
        if (hasChanges?.Trim() == "YES")
        {
            Info("站点内容无变化，跳过提交");
            return true;
        }
        var msg = $"deploy: CG 笔记站点 {DateTime.Now:yyyy-MM-dd HH:mm}";
        var attribution = "Co-Authored-By: Claude Code <noreply@anthropic.com>";
        if (!await RunAsync(repoRoot, $"git -c user.name=CGPublish -c user.email=publish@cg.local commit -m \"{msg}\" -m \"{attribution}\""))
        {
            Err("提交失败");
            return false;
        }
        Info("→ 推送 main 分支（触发 GitHub Pages 重新部署）");
        if (!await RunAsync(repoRoot, "git push origin main"))
        {
            Err("推送失败：请确认 SSH 密钥（~/.ssh）有该仓库的写权限");
            return false;
        }
        Info("✓ 已推送，GitHub Pages 将自动重新部署");
        return true;
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
            return $"站点暂时返回 HTTP {code}（Pages 部署中，约 1 分钟内生效）：{url}";
        }
        catch (Exception ex)
        {
            return $"访问失败（{ex.Message}）：{url}";
        }
    }
}

/// <summary>命令行模式：CG笔记发布器.exe --cli [build|publish|all|open] [--repo 路径]</summary>
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
        string? repo = null;
        for (var i = 0; i < args.Length; i++)
        {
            if (args[i] == "--repo" && i + 1 < args.Length) repo = args[++i];
        }

        var engine = new PublishEngine { Log = (line, isError) => Console.WriteLine(isError ? $"[ERR] {line}" : line) };
        var repoRoot = repo ?? PublishEngine.FindRepoRoot(AppContext.BaseDirectory);
        if (repoRoot == null)
        {
            Console.WriteLine("找不到仓库根目录（.git），请用 --repo 指定");
            return 2;
        }
        Console.WriteLine($"仓库：{repoRoot}");

        switch (cmd)
        {
            case "build":
                return await engine.BuildAsync(repoRoot) ? 0 : 1;

            case "publish":
                return await engine.PublishAsync(repoRoot) ? 0 : 1;

            case "all":
            {
                if (!await engine.BuildAsync(repoRoot)) return 1;
                if (!await engine.PublishAsync(repoRoot)) return 1;
                var remote = await engine.GetRemoteUrlAsync(repoRoot);
                var parsed = remote != null ? PublishEngine.ParseOwnerRepo(remote) : null;
                if (parsed != null)
                    Console.WriteLine(await engine.VerifyAsync(parsed.Value.owner, parsed.Value.repo));
                return 0;
            }

            case "open":
            {
                var remote = await engine.GetRemoteUrlAsync(repoRoot);
                var parsed = remote != null ? PublishEngine.ParseOwnerRepo(remote) : null;
                if (parsed == null)
                {
                    Console.WriteLine("无法从远端地址解析 owner/repo");
                    return 2;
                }
                Process.Start(new ProcessStartInfo(PublishEngine.SiteUrl(parsed.Value.owner, parsed.Value.repo)) { UseShellExecute = true });
                return 0;
            }

            default:
                Console.WriteLine("用法：CG笔记发布器.exe --cli [build|publish|all|open] [--repo 路径]");
                return 2;
        }
    }
}

/// <summary>主窗口</summary>
internal sealed class MainForm : Form
{
    private readonly TextBox _repoBox = new() { Anchor = AnchorStyles.Left | AnchorStyles.Right };
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
    private readonly Button _btnAll, _btnBuild, _btnPublish, _btnOpen;
    private bool _busy;
    private string? _siteUrl;

    public MainForm()
    {
        Text = "CG 笔记发布器 — Excalidraw 笔记 → VitePress → GitHub Pages";
        Font = new Font("Microsoft YaHei UI", 9f);
        AutoScaleMode = AutoScaleMode.Dpi;
        ClientSize = new Size(820, 560);
        MinimumSize = new Size(720, 440);
        StartPosition = FormStartPosition.CenterScreen;

        var repoLabel = new Label { Text = "仓库目录", AutoSize = true };
        var btnBrowse = new Button { Text = "浏览…", AutoSize = true };
        btnBrowse.Click += (_, _) =>
        {
            using var dlg = new FolderBrowserDialog { Description = "选择 CG 仓库根目录（含 .git）" };
            if (dlg.ShowDialog(this) == DialogResult.OK) _repoBox.Text = dlg.SelectedPath;
        };

        _btnAll = new Button { Text = "🚀 一键构建并发布", AutoSize = true, BackColor = Color.FromArgb(46, 160, 67), ForeColor = Color.White, FlatStyle = FlatStyle.Flat, Padding = new Padding(10, 4, 10, 4) };
        _btnBuild = new Button { Text = "构建站点", AutoSize = true };
        _btnPublish = new Button { Text = "发布到 GitHub Pages", AutoSize = true };
        _btnOpen = new Button { Text = "打开站点", AutoSize = true };
        var btnClear = new Button { Text = "清空日志", AutoSize = true };
        btnClear.Click += (_, _) => _logBox.Clear();

        _btnAll.Click += async (_, _) => await RunAsync(async () =>
        {
            if (!await NewEngine().BuildAsync(Repo)) return false;
            if (!await NewEngine().PublishAsync(Repo)) return false;
            var remote = await NewEngine().GetRemoteUrlAsync(Repo);
            var parsed = remote != null ? PublishEngine.ParseOwnerRepo(remote) : null;
            if (parsed != null)
            {
                _siteUrl = PublishEngine.SiteUrl(parsed.Value.owner, parsed.Value.repo);
                Append(await NewEngine().VerifyAsync(parsed.Value.owner, parsed.Value.repo), false);
            }
            return true;
        }, "发布完成");

        _btnBuild.Click += async (_, _) => await RunAsync(() => NewEngine().BuildAsync(Repo), "构建完成");
        _btnPublish.Click += async (_, _) => await RunAsync(() => NewEngine().PublishAsync(Repo), "发布完成");
        _btnOpen.Click += (_, _) =>
        {
            if (!string.IsNullOrWhiteSpace(_siteUrl))
                Process.Start(new ProcessStartInfo(_siteUrl) { UseShellExecute = true });
        };

        // 布局
        var root = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 1, RowCount = 2, Padding = new Padding(10) };
        root.RowStyles.Add(new RowStyle(SizeType.Absolute, 90));
        root.RowStyles.Add(new RowStyle(SizeType.Percent, 100));

        var top = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 3, RowCount = 2, AutoSize = true };
        top.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 120));
        top.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        top.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 90));
        top.Controls.Add(repoLabel, 0, 0);
        top.Controls.Add(_repoBox, 1, 0);
        top.Controls.Add(btnBrowse, 2, 0);
        var btns = new FlowLayoutPanel { Dock = DockStyle.Fill, AutoSize = true, WrapContents = true, Margin = new Padding(0, 8, 0, 0) };
        btns.Controls.AddRange(new Control[] { _btnAll, _btnBuild, _btnPublish, _btnOpen, btnClear });
        top.Controls.Add(btns, 1, 1);
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
                var remote = await Task.Run(() => NewEngine().GetRemoteUrlAsync(repo));
                var parsed = remote != null ? PublishEngine.ParseOwnerRepo(remote) : null;
                if (parsed != null) _siteUrl = PublishEngine.SiteUrl(parsed.Value.owner, parsed.Value.repo);
            }
            Append($"CG 笔记发布器已就绪。仓库：{_repoBox.Text}", false);
            Append("流程：转换笔记 → VitePress 构建 → 提交 dist 到 main 分支并推送（GitHub Pages 部署 main，自动生效）", false);
        };
    }

    private string Repo => _repoBox.Text.Trim();

    private PublishEngine NewEngine()
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
        _btnAll.Enabled = _btnBuild.Enabled = _btnPublish.Enabled = _btnOpen.Enabled = enabled;
    }

    private void Append(string line, bool isError)
    {
        _logBox.SelectionStart = _logBox.TextLength;
        _logBox.SelectionColor = isError ? Color.FromArgb(255, 123, 114) : Color.Gainsboro;
        _logBox.AppendText(line + Environment.NewLine);
        _logBox.ScrollToCaret();
    }
}
