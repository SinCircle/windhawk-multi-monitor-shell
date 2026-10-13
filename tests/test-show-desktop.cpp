// Deterministic transaction tests; optional native tests affect only our fixtures.
#include "../multi-monitor-shell.wh.cpp"
#include <cassert>
#include <cstdio>
#include <algorithm>

namespace Fixture {
using namespace ShowDesktop;
struct State {
    HMONITOR monitor;
    bool minimized = false, alive = true, marked = false;
    bool markOk = true, showOk = true;
    UINT restore = SW_SHOWNOACTIVATE;
    int lastCommand = -1;
};
std::map<HWND, State> windows;
HMONITOR a = reinterpret_cast<HMONITOR>(1), b = reinterpret_cast<HMONITOR>(2);
HWND W(int id) { return reinterpret_cast<HWND>(static_cast<INT_PTR>(id)); }
ULONGLONG ticks = 10000;
bool collectOk = true, defer = false;
int calls = 0, focusCalls = 0, originalCalls = 0;
void* originalSelf = nullptr;
WPARAM originalKey = 0;
LPARAM originalDetails = 0;
bool Collect(HMONITOR monitor, std::vector<Window>& output) {
    for (auto& [hwnd, state] : windows)
        if (state.alive && !state.minimized && state.monitor == monitor)
            output.push_back({hwnd, 1, 1, state.restore});
    return collectOk;
}
bool Mark(Window& window) {
    auto& state = windows.at(window.hwnd);
    if (!state.alive || !state.markOk || state.marked) return false;
    state.marked = true;
    window.marker = reinterpret_cast<HANDLE>(1);
    return true;
}
bool Owns(const Window& window) {
    auto& state = windows.at(window.hwnd);
    return state.alive && state.marked;
}
void Forget(const Window& window) { windows.at(window.hwnd).marked = false; }
HMONITOR Monitor(HWND hwnd) { return windows.at(hwnd).monitor; }
BOOL WINAPI Iconic(HWND hwnd) { return windows.at(hwnd).minimized; }
BOOL WINAPI Show(HWND hwnd, int command) {
    auto& state = windows.at(hwnd);
    ++calls;
    state.lastCommand = command;
    if (!state.showOk) return FALSE;
    if (!defer) state.minimized = command == SW_MINIMIZE;
    return TRUE;
}
HWND WINAPI Foreground() { return W(1); }
BOOL WINAPI Focus(HWND) { ++focusCalls; return TRUE; }
ULONGLONG WINAPI Now() { return ticks; }
void __cdecl Original(void* self, WPARAM key, LPARAM details) {
    ++originalCalls; originalSelf = self; originalKey = key; originalDetails = details;
}
void Reset() {
    batches.clear(); unloading = false;
    windows.clear(); ticks = 10000; calls = focusCalls = 0;
    collectOk = true; defer = false;
    windows.emplace(W(1), State{a});
    windows.emplace(W(2), State{a, false, true, false, true, true, SW_SHOWMAXIMIZED});
    windows.emplace(W(3), State{a, true});
    windows.emplace(W(4), State{b});
    api = {Collect, Mark, Owns, Forget, Monitor, Iconic, Show, Foreground, Focus, Now};
}
int groups = 0;
void Pass(const char* message) { ++groups; std::printf("PASS: %s\n", message); }
void Run() {
    Reset(); ToggleMonitor(a);
    assert(windows.at(W(1)).minimized && windows.at(W(2)).minimized);
    assert(!windows.at(W(4)).minimized && calls == 2);
    ToggleMonitor(a);
    assert(!windows.at(W(1)).minimized && !windows.at(W(2)).minimized);
    assert(windows.at(W(3)).minimized && batches.empty());
    assert(windows.at(W(2)).lastCommand == SW_SHOWMAXIMIZED);
    assert(windows.at(W(1)).lastCommand == SW_SHOWNOACTIVATE && focusCalls == 1);
    Pass("one monitor only, original minimized state and maximized restore");

    Reset(); ToggleMonitor(a); ToggleMonitor(b); ToggleMonitor(a);
    assert(!windows.at(W(1)).minimized && windows.at(W(4)).minimized);
    assert(batches.size() == 1); ToggleMonitor(b); assert(batches.empty());
    Pass("independent restore batches across monitors");

    Reset(); defer = true; ToggleMonitor(a); ToggleMonitor(a);
    assert(calls == 4 && batches.empty());
    assert(windows.at(W(1)).lastCommand == SW_SHOWNOACTIVATE);
    Pass("rapid second press queues restore after pending minimize");

    Reset(); ToggleMonitor(a); ticks += 2000;
    windows.at(W(1)).minimized = false;
    windows.at(W(1)).lastCommand = -1;
    ToggleMonitor(a);
    assert(windows.at(W(1)).lastCommand == -1 && !windows.at(W(1)).marked);
    assert(!windows.at(W(2)).minimized && batches.empty());
    Pass("manually restored window is left alone");

    Reset(); ToggleMonitor(a); windows.at(W(1)).alive = false;
    windows.at(W(2)).monitor = b; calls = 0; ToggleMonitor(a);
    assert(calls == 0 && batches.empty());
    Pass("closed and moved windows are never restored on another monitor");

    Reset(); ToggleMonitor(a);
    windows.at(W(1)).marked = false; // HWND reused, even with same process/thread.
    windows.at(W(1)).lastCommand = -1; ToggleMonitor(a);
    assert(windows.at(W(1)).lastCommand == -1);
    Pass("lost identity marker prevents restoring a reused handle");

    Reset(); windows.at(W(1)).markOk = false; windows.at(W(2)).showOk = false;
    ToggleMonitor(a); assert(batches.empty());
    assert(!windows.at(W(1)).minimized && !windows.at(W(2)).marked);
    Pass("permission and minimize failure leave no managed batch");

    Reset(); collectOk = false; ToggleMonitor(a); ToggleMonitor(nullptr);
    assert(calls == 0 && batches.empty());
    Pass("enumeration and target lookup failures make no global changes");

    Reset(); defer = true; ToggleMonitor(a); ticks += 2000; calls = 0; Prune();
    assert(batches.empty() && calls == 0 && !windows.at(W(1)).marked);
    Pass("expired unprocessed minimize records are released");

    Reset(); ToggleMonitor(a); ToggleMonitor(b); Shutdown();
    assert(batches.empty() && windows.at(W(3)).minimized);
    assert(!windows.at(W(1)).minimized && !windows.at(W(4)).minimized);
    calls = 0; ToggleMonitor(a); assert(calls == 0);
    Pass("unload restores managed windows and blocks new operations");

    Reset(); HotkeyOriginal = Original;
    HotkeyHook(reinterpret_cast<void*>(123), 0x305, 0x12345678);
    assert(originalCalls == 1 && originalSelf == reinterpret_cast<void*>(123));
    assert(originalKey == 0x305 && originalDetails == 0x12345678 && calls == 0);
    Pass("unrelated global hotkeys preserve this, id and details");
    std::printf("PASS: %d desktop regression groups. No user windows were changed.\n", groups);
}
} // namespace Fixture

namespace NativeFixture {
std::vector<HWND> windows;
std::vector<HMONITOR> monitors;
bool Collect(HMONITOR monitor, std::vector<ShowDesktop::Window>& output) {
    if (!ShowDesktop::CollectWindows(monitor, output)) return false;
    std::erase_if(output, [](auto& window) {
        return std::find(windows.begin(), windows.end(), window.hwnd) == windows.end();
    });
    return true;
}
void Pump() {
    const auto deadline = GetTickCount64() + 120;
    do {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg); DispatchMessageW(&msg);
        }
        Sleep(5);
    } while (GetTickCount64() < deadline);
}
HWND Create(HMONITOR monitor, DWORD ex = 0, HWND owner = nullptr) {
    MONITORINFO info{sizeof(info)}; assert(GetMonitorInfoW(monitor, &info));
    HWND hwnd = CreateWindowExW(ex, L"MMS.WindowFixture", L"Multi-monitor Shell test fixture",
                               WS_OVERLAPPEDWINDOW, info.rcWork.left + 80, info.rcWork.top + 80,
                               360, 200, owner, nullptr, GetModuleHandleW(nullptr), nullptr);
    assert(hwnd); windows.push_back(hwnd); ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    return hwnd;
}
int Run() {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR m, HDC, LPRECT, LPARAM) -> BOOL {
        monitors.push_back(m); return TRUE;
    }, 0);
    if (monitors.size() < 2) { std::puts("SKIP: native fixture test requires two monitors."); return 77; }
    WNDCLASSW cls{}; cls.lpfnWndProc = DefWindowProcW;
    cls.hInstance = GetModuleHandleW(nullptr); cls.lpszClassName = L"MMS.WindowFixture";
    assert(RegisterClassW(&cls));
    HWND normal = Create(monitors[0]);
    HWND maximized = Create(monitors[0]); ShowWindow(maximized, SW_MAXIMIZE);
    HWND minimized = Create(monitors[0]); ShowWindow(minimized, SW_MINIMIZE);
    HWND other = Create(monitors[1]);
    HWND tool = Create(monitors[0], WS_EX_TOOLWINDOW);
    HWND owner = Create(monitors[0]);
    HWND dialog = Create(monitors[1], WS_EX_TOOLWINDOW, owner);
    Pump();
    ShowDesktop::api = {}; ShowDesktop::api.collect = Collect;
    ShowDesktop::unloading = false;
    std::vector<ShowDesktop::Window> selected;
    assert(Collect(monitors[0], selected));
    bool ok = selected.size() == 2;
    std::printf("Native selection: %zu candidates (expected 2)\n", selected.size());
    for (const auto& candidate : selected)
        std::printf("  selected fixture %zd\n", std::find(windows.begin(), windows.end(), candidate.hwnd) - windows.begin());
    std::printf("Native dialog: visible=%d eligible=%d ownerMatches=%d otherMonitor=%d\n",
                IsWindowVisible(dialog), ShowDesktop::VisibleOnDesktop(dialog),
                ShowDesktop::RootOwner(dialog) == owner,
                ShowDesktop::WindowMonitor(dialog) == monitors[1]);
    ShowDesktop::ToggleMonitor(monitors[0]); Pump();
    bool minimizedOk = IsIconic(normal) && IsIconic(maximized) && IsIconic(minimized);
    bool isolationOk = !IsIconic(other) && !IsIconic(tool) && !IsIconic(owner) && IsWindowVisible(dialog);
    ok &= minimizedOk && isolationOk;
    std::printf("Native first press: minimize=%d isolation=%d\n", minimizedOk, isolationOk);
    ShowDesktop::ToggleMonitor(monitors[1]); Pump();
    bool independentOk = IsIconic(other) && IsIconic(normal);
    ok &= independentOk;
    std::printf("Native other screen: independent=%d\n", independentOk);
    ShowDesktop::ToggleMonitor(monitors[0]); Pump();
    bool restoredOk = !IsIconic(normal) && IsZoomed(maximized) && IsIconic(minimized) && IsIconic(other);
    ok &= restoredOk;
    std::printf("Native second press: restored=%d normalIconic=%d maxZoomed=%d preIconic=%d otherIconic=%d\n",
                restoredOk, IsIconic(normal), IsZoomed(maximized), IsIconic(minimized), IsIconic(other));
    ShowDesktop::Shutdown(); Pump();
    bool unloadedOk = !IsIconic(other) && IsIconic(minimized) && ShowDesktop::batches.empty();
    ok &= unloadedOk;
    std::printf("Native unload: restored=%d\n", unloadedOk);
    for (auto hwnd : windows) if (IsWindow(hwnd)) DestroyWindow(hwnd);
    std::printf("%s: native two-monitor fixtures, real DWM filtering, owned dialog isolation, async minimize/restore and unload.\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
} // namespace NativeFixture

int main(int argc, char** argv) {
    if (argc == 2 && std::string(argv[1]) == "--window-fixtures") return NativeFixture::Run();
    Fixture::Run(); return 0;
}
