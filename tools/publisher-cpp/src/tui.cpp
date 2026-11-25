#include "tui.h"

#include "engine.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX  // 防 windows.h 的 min/max 宏污染 FTXUI 模板
#include <windows.h>
#include <shellapi.h>
#include <shobjidl.h>

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <atomic>
#include <chrono>
#include <ctime>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using namespace ftxui;

namespace cgpublish {

namespace {

constexpr const char* kSpinner[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};

// 共享状态：UI 线程与工作线程都要访问，用互斥锁保护
struct AppState {
  std::mutex mu;
  std::deque<std::pair<std::string, bool>> lines;  // (文本, 是否错误)
  std::string repo;
  std::string status = "就绪";
  std::string siteUrl;
  bool busy = false;
  bool statusOk = true;
  int spinnerIdx = 0;
};

std::string NowStamp() {
  std::time_t t = std::time(nullptr);
  std::tm tm{};
  localtime_s(&tm, &t);
  char buf[16];
  std::strftime(buf, sizeof(buf), "[%H:%M:%S]", &tm);
  return buf;
}

// Win32 文件夹选择框（IFileOpenDialog，FOS_PICKFOLDERS）
std::string PickFolder() {
  IFileOpenDialog* dlg = nullptr;
  if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&dlg))))
    return {};
  dlg->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
  std::string result;
  if (SUCCEEDED(dlg->Show(nullptr))) {
    IShellItem* item = nullptr;
    if (SUCCEEDED(dlg->GetResult(&item))) {
      PWSTR p = nullptr;
      if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &p))) {
        result = ToUtf8(p);
        CoTaskMemFree(p);
      }
      item->Release();
    }
  }
  dlg->Release();
  return result;
}

}  // namespace

int TuiMain() {
  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleTitleW(L"yushu-project-management");

  // ScreenInteractive 不可拷贝/移动（make_shared 不可行）：
  // 用堆上包装器持有，工作线程捕获 shared_ptr 副本——UI 退出后只要还有任务在跑，
  // screen 就不会析构，杜绝悬垂指针（C++17 保证 elision，成员初始化无需移动构造）
  struct ScreenHolder {
    ScreenInteractive screen = ScreenInteractive::Fullscreen();
  };
  auto holder = std::make_shared<ScreenHolder>();
  auto st = std::make_shared<AppState>();
  auto running = std::make_shared<std::atomic<bool>>(true);

  st->repo = FindRepoRoot(ExeDir());
  if (!st->repo.empty()) {
    auto remote = GetRemoteUrl(st->repo);
    auto p = ParseOwnerRepo(remote);
    if (p) st->siteUrl = SiteUrl(p->first, p->second);
  }
  {
    std::lock_guard<std::mutex> lk(st->mu);
    st->lines.emplace_back("yushu-project-management 已就绪。仓库：" + st->repo, false);
    st->lines.emplace_back(
        "流程：VitePress 构建 → dist 产物推送到 gh-pages 分支（Pages 部署 gh-pages 根目录）",
        false);
    if (!st->siteUrl.empty())
      st->lines.emplace_back("站点地址：" + st->siteUrl, false);
  }

  // 日志回调：带时间戳入队 + 触发重绘（可从工作线程调用）
  auto doLog = [st, holder, running](const std::string& line, bool isErr) {
    {
      std::lock_guard<std::mutex> lk(st->mu);
      st->lines.emplace_back(NowStamp() + " " + line, isErr);
      while (st->lines.size() > 2000) st->lines.pop_front();
    }
    if (running->load()) holder->screen.PostEvent(Event::Custom);
  };

  auto setDone = [st, holder, running](bool ok, const std::string& text) {
    {
      std::lock_guard<std::mutex> lk(st->mu);
      st->busy = false;
      st->statusOk = ok;
      st->status = ok ? "✓ " + text : "✗ " + text;
    }
    if (running->load()) holder->screen.PostEvent(Event::Custom);
  };

  // 启动后台任务；busy 时忽略再次触发
  auto runTask = [st, holder, running](std::function<void()> work) {
    {
      std::lock_guard<std::mutex> lk(st->mu);
      if (st->busy) return;
      st->busy = true;
      st->statusOk = false;
      st->status = "运行中…";
    }
    holder->screen.PostEvent(Event::Custom);
    std::thread([work = std::move(work)] { work(); }).detach();
  };

  // ---------- 动作（按钮与快捷键共用） ----------

  auto actionAll = [st, doLog, setDone, runTask] {
    std::string repo = st->repo;
    runTask([repo, doLog, setDone, st] {
      bool ok = BuildAsync(repo, doLog);
      if (ok) ok = PublishAsync(repo, doLog);
      if (ok) {
        auto remote = GetRemoteUrl(repo);
        auto p = ParseOwnerRepo(remote);
        if (p) {
          {
            std::lock_guard<std::mutex> lk(st->mu);
            st->siteUrl = SiteUrl(p->first, p->second);
          }
          doLog(VerifyAsync(p->first, p->second), false);
        }
      }
      setDone(ok, "发布完成");
    });
  };

  auto actionBuild = [st, doLog, setDone, runTask] {
    std::string repo = st->repo;
    runTask([repo, doLog, setDone] {
      bool ok = BuildAsync(repo, doLog);
      setDone(ok, "构建完成");
    });
  };

  auto actionPublish = [st, doLog, setDone, runTask] {
    std::string repo = st->repo;
    runTask([repo, doLog, setDone] {
      bool ok = PublishAsync(repo, doLog);
      setDone(ok, "发布完成");
    });
  };

  auto actionOpen = [st, doLog] {
    std::string url;
    {
      std::lock_guard<std::mutex> lk(st->mu);
      url = st->siteUrl;
    }
    if (url.empty()) {
      doLog("站点地址未知：请先构建/发布一次，或确认仓库有 origin 远端", true);
      return;
    }
    ShellExecuteW(nullptr, L"open", ToWide(url).c_str(), nullptr, nullptr,
                  SW_SHOWNORMAL);
  };

  auto actionClear = [st, holder] {
    {
      std::lock_guard<std::mutex> lk(st->mu);
      st->lines.clear();
    }
    holder->screen.PostEvent(Event::Custom);
  };

  auto actionBrowse = [st, holder] {
    std::string folder = PickFolder();
    if (!folder.empty()) {
      st->repo = folder;
      holder->screen.PostEvent(Event::Custom);
    }
  };

  auto actionQuit = [holder] { holder->screen.Exit(); };

  // ---------- 组件 ----------

  auto inputRepo = Input(&st->repo, "仓库目录");
  auto btnBrowse = Button(" b 浏览… ", actionBrowse);
  auto btnAll = Button(" 1 一键构建并发布 ", actionAll);
  auto btnBuild = Button(" 2 构建站点 ", actionBuild);
  auto btnPublish = Button(" 3 发布到 GitHub Pages ", actionPublish);
  auto btnOpen = Button(" 4 打开站点 ", actionOpen);
  auto btnClear = Button(" c 清空日志 ", actionClear);
  auto btnQuit = Button(" q 退出 ", actionQuit);

  // 日志视图：最近 25 行，错误红色
  auto logView = Renderer([st] {
    std::vector<Element> els;
    {
      std::lock_guard<std::mutex> lk(st->mu);
      size_t begin = st->lines.size() > 25 ? st->lines.size() - 25 : 0;
      for (size_t i = begin; i < st->lines.size(); i++) {
        std::string textPart = st->lines[i].first;
        size_t sep = textPart.find(' ');
        Element line = sep == std::string::npos
                           ? paragraph(textPart)
                           : hbox({
                                 text(textPart.substr(0, sep)) | dim,
                                 paragraph(textPart.substr(sep + 1)),
                             });
        els.push_back(line | color(st->lines[i].second ? Color(Color::Red)
                                                       : Color(Color::Default)));
      }
    }
    if (els.empty())
      els.push_back(text("（暂无日志）") | dim | center);
    return vbox(std::move(els)) | flex;
  });

  auto repoRow = Container::Horizontal({inputRepo, btnBrowse});
  auto row1 = Container::Horizontal({btnAll, btnBuild, btnPublish, btnOpen});
  auto row2 = Container::Horizontal({btnClear, btnQuit});
  auto main = Container::Vertical({repoRow, row1, row2, logView});

  // 外层渲染：头部色带 + 面板 + 按钮行 + 日志 + 状态脚
  auto component = Renderer(main, [&] {
    std::string siteUrl, status;
    bool busy = false, statusOk = true;
    int spin = 0;
    {
      std::lock_guard<std::mutex> lk(st->mu);
      siteUrl = st->siteUrl;
      status = st->status;
      busy = st->busy;
      statusOk = st->statusOk;
      spin = st->spinnerIdx;
    }

    // 头部
    Element header = hbox({
                         text(" yushu-project-management ") | bold,
                         text(" Markdown → VitePress → GitHub Pages ") | dim,
                         filler(),
                         text(siteUrl.empty() ? "站点地址未识别" : siteUrl) | dim,
                     }) |
                     bgcolor(Color::Blue) | color(Color::White);

    // 状态（busy 时动画）
    Element statusEl = text((busy ? std::string(kSpinner[spin]) + " " : "") + status) |
                       color(busy ? Color(Color::Yellow)
                                  : (statusOk ? Color(Color::Green)
                                              : Color(Color::Red)));

    return vbox({
        header,
        window(text(" 仓库 "),
               hbox({inputRepo->Render() | flex, btnBrowse->Render()})),
        hbox({btnAll->Render(), btnBuild->Render(), btnPublish->Render(),
              btnOpen->Render()}),
        hbox({btnClear->Render(), btnQuit->Render(), filler(),
              text("Tab 切换 · 1-4 快捷操作 · b 浏览 · c 清空 · q 退出") | dim}),
        window(text(" 日志 "), logView->Render()) | flex,
        hbox({statusEl, filler(),
              text(busy ? "任务进行中，请稍候…" : "就绪，等待操作") | dim}),
    });
  });

  component |= CatchEvent([&](Event e) {
    if (e == Event::Escape) {
      holder->screen.Exit();
      return true;
    }
    if (!e.is_character()) return false;
    std::string c = e.character();
    // 输入框聚焦时不拦截字符，避免快捷操作干扰输入
    if (inputRepo->Focused()) return false;
    if (c == "q" || c == "Q") { actionQuit(); return true; }
    if (c == "1") { actionAll(); return true; }
    if (c == "2") { actionBuild(); return true; }
    if (c == "3") { actionPublish(); return true; }
    if (c == "4") { actionOpen(); return true; }
    if (c == "b" || c == "B") { actionBrowse(); return true; }
    if (c == "c" || c == "C") { actionClear(); return true; }
    return false;
  });

  // 动画线程：busy 期间每 150ms 推进一次 spinner 并触发重绘
  std::thread([st, holder, running] {
    while (running->load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(150));
      bool busy = false;
      {
        std::lock_guard<std::mutex> lk(st->mu);
        busy = st->busy;
        if (busy) st->spinnerIdx = (st->spinnerIdx + 1) % 10;
      }
      if (busy && running->load()) holder->screen.PostEvent(Event::Custom);
    }
  }).detach();

  // 焦点默认放在主操作按钮上（树组装完成后设置才生效），避免误触输入框；
  // 输入框聚焦时快捷键自动让位（见 CatchEvent）
  row1->SetActiveChild(btnAll);
  main->SetActiveChild(row1);

  holder->screen.Loop(component);
  running->store(false);
  return 0;
}

}  // namespace cgpublish
