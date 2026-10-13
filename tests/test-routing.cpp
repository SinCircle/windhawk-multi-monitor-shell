// Tests include the production source and replace only OS input/monitor queries.
// No windows are opened, keystrokes sent, or hooks installed in Explorer.
#include <windhawk_utils.h>
#include <winrt/windows.foundation.collections.h>
#include <atomic>
#include <functional>
#include <string>
#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <thread>

namespace Fixture {
HMONITOR primary = reinterpret_cast<HMONITOR>(1);
HMONITOR secondary = reinterpret_cast<HMONITOR>(2);
POINT cursor{-900, 100};
bool cursorOk = true, infoOk = true, monitorOk = true, deviceOk = true;
bool connectResult = true, invalidRect = false, winPressed = false;
int connects = 0, fallbacks = 0;
POINT connected{};
std::function<HRESULT()> onShow;
BOOL WINAPI Cursor(POINT* point) { if (!cursorOk) return FALSE; *point = cursor; return TRUE; }
HMONITOR WINAPI MonitorAt(POINT point, DWORD) {
    return monitorOk ? (point.x < 0 ? secondary : primary) : nullptr;
}
HMONITOR WINAPI MonitorForWindow(HWND window, DWORD) {
    return monitorOk ? reinterpret_cast<HMONITOR>(window) : nullptr;
}
BOOL WINAPI Info(HMONITOR monitor, LPMONITORINFO info) {
    if (!infoOk || (monitor != primary && monitor != secondary)) return FALSE;
    bool main = monitor == primary;
    info->rcMonitor = main ? RECT{0, 0, 2560, 1600} : RECT{-1920, -360, 0, 720};
    if (invalidRect) info->rcMonitor.right = info->rcMonitor.left;
    info->rcWork = info->rcMonitor;
    info->rcWork.bottom -= 48;
    info->dwFlags = main ? MONITORINFOF_PRIMARY : 0;
    if (info->cbSize == sizeof(MONITORINFOEXW))
        wcscpy_s(reinterpret_cast<MONITORINFOEXW*>(info)->szDevice,
                 CCHDEVICENAME, main ? L"DISPLAY-A" : L"DISPLAY-B");
    return TRUE;
}
// MinGW exposes a second C++ overload for MONITORINFOEXW.
BOOL WINAPI Info(HMONITOR monitor, LPMONITORINFOEXW info) {
    return Info(monitor, reinterpret_cast<LPMONITORINFO>(info));
}
BOOL WINAPI Enumerate(HDC, LPCRECT, MONITORENUMPROC callback, LPARAM data) {
    RECT rect{};
    if (!callback(primary, nullptr, &rect, data)) return TRUE;
    callback(secondary, nullptr, &rect, data);
    return TRUE;
}
BOOL WINAPI Devices(LPCWSTR name, DWORD, PDISPLAY_DEVICEW device, DWORD) {
    if (!deviceOk) return FALSE;
    wcscpy_s(device->DeviceID, 128,
             wcscmp(name, L"DISPLAY-A") == 0 ? L"MONITOR#PRIMARY#A" : L"MONITOR#SECONDARY#B");
    return TRUE;
}
SHORT WINAPI Key(int) { return winPressed ? static_cast<SHORT>(0x8000) : 0; }
bool WINAPI Connect(void*, HWND, POINT point) { ++connects; connected = point; return connectResult; }
// On this Windows build native Adjust requires a primary monitor and resets a
// valid secondary selection. Model that observable behavior for regressions.
HRESULT WINAPI Adjust(void*) { ++fallbacks; connected = {1280, 800}; return S_FALSE; }
HRESULT WINAPI Show(void*, int, int) { return onShow ? onShow() : E_UNEXPECTED; }
}

#define GetCursorPos Fixture::Cursor
#define MonitorFromPoint Fixture::MonitorAt
#define MonitorFromWindow Fixture::MonitorForWindow
#define GetMonitorInfoW Fixture::Info
#define EnumDisplayMonitors Fixture::Enumerate
#define EnumDisplayDevicesW Fixture::Devices
#define GetAsyncKeyState Fixture::Key
#include "../multi-monitor-shell.wh.cpp"
#undef GetCursorPos
#undef MonitorFromPoint
#undef MonitorFromWindow
#undef GetMonitorInfoW
#undef EnumDisplayMonitors
#undef EnumDisplayDevicesW
#undef GetAsyncKeyState

int groups = 0;
void Pass(const char* text) { ++groups; std::printf("PASS: %s\n", text); }
void Reset() {
    using namespace Fixture;
    cursor = {-900, 100};
    cursorOk = infoOk = monitorOk = deviceOk = connectResult = true;
    invalidRect = winPressed = false;
    connects = fallbacks = 0;
    connected = {};
    StartMenu::settings = {};
    StartMenu::inShow = false;
    StartMenu::target = nullptr;
    StartMenu::ConnectOriginal = Connect;
    StartMenu::AdjustOriginal = Adjust;
    StartMenu::ShowOriginal = Show;
    onShow = [] { return StartMenu::AdjustHook(nullptr); };
}

void TestStart() {
    using namespace Fixture;
    Reset();
    assert(StartMenu::AdjustHook(nullptr) == S_FALSE && fallbacks == 1 && connects == 0);
    Pass("unrelated monitor adjustments keep their native behavior");

    Reset();
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_OK);
    assert(connects == 1 && fallbacks == 0 && connected.x == -960 && connected.y == 180);
    assert(!StartMenu::inShow && !StartMenu::target);
    Pass("cursor on secondary display, negative origin, scope cleanup");

    Reset();
    cursor = {500, 500};
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_OK && connected.x == 1280);
    Pass("cursor on primary display");

    Reset();
    StartMenu::settings.location = StartMenu::Location::primary;
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_OK && connected.x == 1280);
    Pass("explicit primary ignores the cursor on secondary");

    Reset();
    StartMenu::settings.location = StartMenu::Location::custom;
    StartMenu::settings.monitor = 2;
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_OK && connected.x == -960);
    StartMenu::settings.monitor = 99;
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_FALSE && fallbacks == 1);
    StartMenu::settings.monitor = -1;
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_FALSE && fallbacks == 2);
    Pass("custom monitor selection and invalid indices");

    Reset();
    StartMenu::settings.location = StartMenu::Location::custom;
    StartMenu::settings.monitor = 1;
    StartMenu::settings.interfaceName = L"SECONDARY";
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_OK && connected.x == -960);
    StartMenu::settings.interfaceName = L"UNPLUGGED";
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_FALSE);
    assert(connects == 1 && fallbacks == 1);
    Pass("interface name takes precedence; missing name keeps native selection");

    Reset();
    StartMenu::settings.location = StartMenu::Location::custom;
    StartMenu::settings.interfaceName = L"SECONDARY";
    deviceOk = false;
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_FALSE && connects == 0);
    Pass("display interface enumeration failure");

    Reset();
    cursorOk = false;
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_FALSE && connects == 0);
    cursorOk = true; monitorOk = false;
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_FALSE && connects == 0);
    Pass("cursor and monitor lookup failures");

    Reset();
    onShow = [] { infoOk = false; return StartMenu::AdjustHook(nullptr); };
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_FALSE && connects == 0 && fallbacks == 1);
    Pass("display disconnect between invocation and positioning");

    Reset();
    invalidRect = true;
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_FALSE && connects == 0);
    Pass("invalid monitor rectangle");

    Reset();
    connectResult = false;
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_OK && connects == 1 && fallbacks == 0);
    assert(connected.x == -960);
    Pass("a false native Connect return does not reset a valid secondary selection");

    Reset();
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_OK && connected.x == -960);
    // First invocation changes monitor; subsequent ones need no transition.
    connectResult = false;
    for (int attempt = 0; attempt < 5; ++attempt) {
        assert(StartMenu::ShowHook(nullptr, 0, 0) == S_OK);
        assert(connected.x == -960 && fallbacks == 0);
    }
    cursor = {100, 100}; connectResult = true;
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_OK && connected.x == 1280);
    connectResult = false;
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_OK && connected.x == 1280);
    cursor = {-900, 100}; connectResult = true;
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_OK && connected.x == -960);
    connectResult = false;
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_OK && connected.x == -960 && fallbacks == 0);
    Pass("repeat opens on secondary and primary, then switch back to secondary");

    Reset();
    onShow = [] { cursor = {200, 200}; return StartMenu::AdjustHook(nullptr); };
    assert(StartMenu::ShowHook(nullptr, 0, 0) == S_OK && connected.x == -960);
    Pass("mouse movement during opening does not change the captured display");

    Reset();
    {
        StartMenu::ShowScope parent;
        assert(StartMenu::target == secondary);
        cursor = {200, 200};
        assert(StartMenu::ShowHook(nullptr, 0, 0) == S_OK && connected.x == 1280);
        assert(StartMenu::inShow && StartMenu::target == secondary);
    }
    assert(!StartMenu::inShow && !StartMenu::target);
    Pass("nested invocations restore the outer selection");

    Reset();
    onShow = []() -> HRESULT { throw std::runtime_error("test"); };
    try { StartMenu::ShowHook(nullptr, 0, 0); assert(false); }
    catch (const std::runtime_error&) {}
    assert(!StartMenu::inShow && !StartMenu::target);
    Pass("scope state also restores during exception unwinding");

    Reset();
    {
        StartMenu::ShowScope parent;
        std::thread other([] { assert(!StartMenu::inShow && !StartMenu::target); });
        other.join();
        assert(StartMenu::inShow && StartMenu::target == secondary);
    }
    Pass("Start invocation context is isolated per thread");
}

void TestAltTab() {
    using namespace Fixture;
    Reset();
    AltTab::g_settings = {AltTab::Location::cursor, AltTab::WindowsToShow::cursorMonitor,
                         AltTab::Location::primary, AltTab::WindowsToShow::allMonitors};
    RECT rect{};
    assert(AltTab::HandleAltTabWindow(&rect) && rect.left == -1920 && rect.bottom == 672);
    winPressed = true;
    assert(!AltTab::HandleAltTabWindow(&rect));
    AltTab::g_settings.winAltTabLocation = AltTab::Location::cursor;
    assert(AltTab::HandleAltTabWindow(&rect) && rect.left == -1920);
    Pass("Alt+Tab and Win+Alt+Tab retain independent locations and use work area");

    winPressed = false;
    assert(AltTab::IsWindowOnCursorMonitor(reinterpret_cast<HWND>(secondary)));
    assert(!AltTab::IsWindowOnCursorMonitor(reinterpret_cast<HWND>(primary)));
    cursorOk = false;
    assert(AltTab::IsWindowOnCursorMonitor(reinterpret_cast<HWND>(primary)));
    cursorOk = true; monitorOk = false;
    assert(AltTab::IsWindowOnCursorMonitor(reinterpret_cast<HWND>(primary)));
    Pass("Alt+Tab monitor filtering preserves native visibility if monitor queries fail");

    Reset();
    AltTab::g_settings.altTabWindows = AltTab::WindowsToShow::cursorMonitor;
    assert(AltTab::CreateInstanceHook([] {
        assert(AltTab::g_threadIdForXamlAltTabViewHost_CreateInstance == GetCurrentThreadId());
        return S_OK;
    }) == S_OK);
    assert(AltTab::g_threadIdForXamlAltTabViewHost_CreateInstance == 0);
    assert(AltTab::g_lastThreadIdForXamlAltTabViewHost_CreateInstance == GetCurrentThreadId());
    winPressed = true;
    assert(AltTab::CreateInstanceHook([] {
        assert(AltTab::g_threadIdForXamlAltTabViewHost_CreateInstance == 0);
        assert(AltTab::g_lastThreadIdForXamlAltTabViewHost_CreateInstance == 0);
        assert(AltTab::g_CreateInstance_TickCount == 0);
        return S_FALSE;
    }) == S_FALSE);
    Pass("all-monitors invocation clears the previous Alt+Tab filtering interval");

    Reset();
    AltTab::g_settings.altTabWindows = AltTab::WindowsToShow::cursorMonitorExceptPrimary;
    cursor = {100, 100};
    AltTab::CreateInstanceHook([] {
        assert(AltTab::g_threadIdForXamlAltTabViewHost_CreateInstance == 0);
        return S_OK;
    });
    cursor = {-100, 100};
    AltTab::CreateInstanceHook([] {
        assert(AltTab::g_threadIdForXamlAltTabViewHost_CreateInstance == GetCurrentThreadId());
        return S_OK;
    });
    Pass("primary-monitor exception retains upstream behavior");

    HWND window = reinterpret_cast<HWND>(primary);
    assert(AltTab::GetWindowHandleFromApplicationView(nullptr, &window) == E_INVALIDARG && !window);
    BOOL reload = FALSE;
    assert(Wh_ModSettingsChanged(&reload) && reload);
    Pass("null view is rejected and settings request a safe mod reload");
}
void TestNotifications() {
    using namespace Fixture;
    namespace N = Notifications;
    Reset();
    N::settings = {};
    N::MonitorFromPointOriginal = MonitorAt;
    assert(N::MonitorFromPointHook({}, MONITOR_DEFAULTTOPRIMARY) == primary);
    Pass("unrelated monitor queries retain their native result");

    N::ToastPositionOriginal = [](void*) -> HRESULT {
        assert(N::MonitorFromPointHook({}, MONITOR_DEFAULTTOPRIMARY) == secondary);
        assert(N::MonitorFromPointHook({100, 100}, MONITOR_DEFAULTTONEAREST) == primary);
        cursor = {100, 100};
        assert(N::MonitorFromPointHook({}, MONITOR_DEFAULTTOPRIMARY) == secondary);
        return S_FALSE;
    };
    assert(N::ToastPositionHook(nullptr) == S_FALSE);
    assert(N::active == N::Kind::none && !N::target && !N::selecting);
    Pass("toast routes only primary query, captures cursor and preserves HRESULT");

    cursor = {-900, 100};
    N::CenterPositionOriginal = [](void*, RECT*) -> HRESULT {
        assert(N::MonitorFromPointHook({100, 100}, MONITOR_DEFAULTTOPRIMARY) == secondary);
        return S_OK;
    };
    RECT rect{};
    for (int n = 0; n < 5; ++n) assert(N::CenterPositionHook(nullptr, &rect) == S_OK);
    Pass("Win+N overrides cached invocation point on repeated opens");

    {
        N::PositionScope outer(N::Kind::center);
        assert(N::target == secondary);
        cursor = {100, 100};
        { N::PositionScope inner(N::Kind::toast); assert(N::target == primary); }
        assert(N::target == secondary && N::active == N::Kind::center);
        N::selecting = true;
        assert(N::MonitorFromPointHook({}, MONITOR_DEFAULTTOPRIMARY) == primary);
        N::selecting = false;
        std::thread other([] { assert(N::active == N::Kind::none && !N::target); });
        other.join();
    }
    Pass("nested notifications and separate threads cannot leak monitor context");

    cursor = {-900, 100};
    N::settings.location = StartMenu::Location::primary;
    { N::PositionScope s(N::Kind::center); assert(N::target == primary); }
    N::settings.location = StartMenu::Location::custom;
    N::settings.monitor = 2;
    { N::PositionScope s(N::Kind::toast); assert(N::target == secondary); }
    N::settings.monitor = 1;
    N::settings.interfaceName = L"SECONDARY";
    { N::PositionScope s(N::Kind::center); assert(N::target == secondary); }
    N::settings.interfaceName = L"UNPLUGGED";
    { N::PositionScope s(N::Kind::center); assert(!N::target); }
    Pass("notification primary/custom/interface settings are independent of Start");

    N::settings = {};
    cursorOk = false;
    { N::PositionScope s(N::Kind::toast); assert(!N::target); }
    cursorOk = true;
    {
        N::PositionScope s(N::Kind::center);
        infoOk = false;
        assert(N::MonitorFromPointHook({}, MONITOR_DEFAULTTOPRIMARY) == primary);
    }
    infoOk = true; invalidRect = true;
    { N::PositionScope s(N::Kind::center); assert(!N::target); }
    invalidRect = false;
    Pass("cursor failure, disconnect and invalid display bounds keep native behavior");

    N::ToastPositionOriginal = [](void*) -> HRESULT { throw std::runtime_error("test"); };
    try { N::ToastPositionHook(nullptr); assert(false); }
    catch (const std::runtime_error&) {}
    assert(N::active == N::Kind::none && !N::target);
    Pass("notification scope restores on exception without masking native errors");
}

int main() {
    TestStart();
    TestAltTab();
    TestNotifications();
    std::printf("PASS: %d routing/regression groups. Explorer was not touched.\n", groups);
}
