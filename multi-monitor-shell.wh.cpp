// ==WindhawkMod==
// @id              multi-monitor-shell
// @name            Multi-monitor Shell
// @name:zh-CN      多屏随行：Alt+Tab、开始菜单、通知与桌面
// @description     Per-monitor Alt+Tab, Start, notifications, and Win+D show desktop.
// @description:zh-CN 让 Alt+Tab、开始菜单与通知跟随鼠标屏幕，Win+D 仅最小化或恢复当前屏幕的窗口。
// @version         0.3.0
// @author          SinCircle; Local integration; L3r0y, m417z and easyatm (upstream)
// @include         explorer.exe
// @architecture    amd64
// @compilerOptions -lole32 -loleaut32 -lversion -ldwmapi
// @license         GPL-3.0-only
// ==/WindhawkMod==

// Integration and changes: 2026-10-13. This combined work is GPL-3.0-only.
// Start routing is adapted from m417z's start-menu-open-location v1.0 (GPL-3.0).
// Alt+Tab is adapted from L3r0y's alt-tab-per-monitor v1.2 (MIT).
// Upstream: https://github.com/ramensoftware/windhawk-mods/tree/main/mods
//
// MIT notice for the Alt+Tab portions and Win+D upstream reference:
// Copyright (c) L3r0y and contributors
// Copyright (c) easyatm and contributors
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

// ==WindhawkModReadme==
/*
# 多屏随行 / Multi-monitor Shell

一个独立模组，统一管理 Alt+Tab、开始菜单、通知和 Win+D 的作用屏幕。

- Alt+Tab 默认跟随鼠标，只列出该屏幕的窗口。
- Win+Alt+Tab 默认在主屏显示所有屏幕的窗口。
- 开始菜单默认跟随鼠标，也可以指定主屏或固定显示器。
- 通知横幅和 Win+N 通知中心默认跟随鼠标，保留系统原生布局。
- Win+D 仅最小化鼠标所在屏幕的应用窗口，再按一次恢复这一批窗口。
- 五项功能可单独开关；保存设置会重新加载本模组。

开始菜单使用 Windows 原生布局，不修改固定应用、账户或搜索内容。
鼠标屏幕在开始菜单打开时选取，不会在打开后追着鼠标移动。

启用前关闭原版 **Alt+Tab per monitor** 和 **Start menu open location**，
以及 **Customize Windows notifications placement**、**Win+D per monitor**，以免同时修改相同系统函数。
需要恢复时关闭本模组并重新开启原版即可。

目标平台：Windows 11 x64。使用系统内部符号，Windows 更新后可能需要适配。
本项目的编译、隔离测试和符号检查不等于真实 Explorer 中的界面验收。

基于 [L3r0y 的 Alt+Tab per monitor](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/alt-tab-per-monitor.wh.cpp)
和 [m417z 的 Start menu open location](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/start-menu-open-location.wh.cpp)。
通知横幅的定位入口参考 [m417z 的 notifications-placement](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/notifications-placement.wh.cpp)。
整体按 GPL-3.0-only 发布；Alt+Tab 部分保留 MIT 声明。
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- altTabEnabled: true
  $name: 启用 Alt+Tab 多屏控制
- altTabLocation: cursor
  $name: "Alt+Tab：显示位置"
  $options:
  - cursor: 鼠标所在屏幕
  - primary: 主屏幕
- altTabWindows: cursorMonitor
  $name: "Alt+Tab：窗口范围"
  $options:
  - cursorMonitor: 仅鼠标所在屏幕
  - allMonitors: 所有屏幕
  - cursorMonitorExceptPrimary: 副屏仅本屏，主屏显示全部
- winAltTabLocation: primary
  $name: "Win+Alt+Tab：显示位置"
  $options:
  - primary: 主屏幕
  - cursor: 鼠标所在屏幕
- winAltTabWindows: allMonitors
  $name: "Win+Alt+Tab：窗口范围"
  $options:
  - allMonitors: 所有屏幕
  - cursorMonitor: 仅鼠标所在屏幕
  - cursorMonitorExceptPrimary: 副屏仅本屏，主屏显示全部
- startMenuEnabled: true
  $name: 启用开始菜单多屏控制
- startMenuLocation: cursor
  $name: "开始菜单：显示位置"
  $options:
  - cursor: 鼠标所在屏幕
  - primary: 主屏幕
  - custom: 指定显示器
- startMenuMonitor: 1
  $name: "开始菜单：显示器序号"
  $description: 仅“指定显示器”生效。从 1 开始的枚举顺序，可能不同于 Windows 设置中的编号。找不到时保持系统行为。
- startMenuMonitorInterfaceName: ""
  $name: "开始菜单：显示器接口名称（可选）"
  $description: 仅“指定显示器”生效。填写接口名称或其片段时优先于序号，可避免插拔导致序号变化。找不到时保持系统行为。
- notificationToastEnabled: true
  $name: 启用通知横幅多屏控制
- notificationCenterEnabled: true
  $name: 启用 Win+N 通知中心多屏控制
- notificationLocation: cursor
  $name: "通知：显示位置"
  $description: 通知横幅和 Win+N 通知中心共用此位置设置，在系统定位时选择显示器。
  $options:
  - cursor: 鼠标所在屏幕
  - primary: 主屏幕
  - custom: 指定显示器
- notificationMonitor: 1
  $name: "通知：显示器序号"
  $description: 仅“指定显示器”生效。从 1 开始的枚举顺序，可能不同于 Windows 设置中的编号。
- notificationMonitorInterfaceName: ""
  $name: "通知：显示器接口名称（可选）"
  $description: 仅“指定显示器”生效。名称或片段优先于序号；目标不可用时保持系统行为。
- showDesktopEnabled: true
  $name: 启用 Win+D 单屏显示桌面
  $description: 再按一次只恢复本次最小化的窗口；原本已经最小化的窗口保持不动。关闭或重载模组会恢复仍由本功能管理的窗口。
- showDesktopLocation: cursor
  $name: "Win+D：作用屏幕"
  $options:
  - cursor: 鼠标所在屏幕
  - foreground: 当前活动窗口所在屏幕
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#include <atomic>
#include <functional>
#include <string>
#include <map>
#include <mutex>
#include <set>
#include <vector>
#include <dwmapi.h>
#include <winrt/windows.foundation.collections.h>

namespace AltTab {
enum class Location {
    primary,
    cursor,
};

enum class WindowsToShow {
    allMonitors,
    cursorMonitor,
    cursorMonitorExceptPrimary,
};

struct {
    Location altTabLocation;
    WindowsToShow altTabWindows;
    Location winAltTabLocation;
    WindowsToShow winAltTabWindows;
} g_settings;

enum class WinVersion {
    Unsupported,
    Win10,
    Win11,
};

WinVersion g_winVersion;

std::atomic<DWORD> g_threadIdForAltTabShowWindow;
std::atomic<DWORD> g_lastThreadIdForXamlAltTabViewHost_CreateInstance;
std::atomic<DWORD> g_threadIdForXamlAltTabViewHost_CreateInstance;
std::atomic<ULONGLONG> g_CreateInstance_TickCount;
constexpr ULONGLONG kDeltaThreshold = 200;

VS_FIXEDFILEINFO* GetModuleVersionInfo(HMODULE hModule, UINT* puPtrLen) {
    void* pFixedFileInfo = nullptr;
    UINT uPtrLen = 0;

    HRSRC hResource =
        FindResource(hModule, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
    if (hResource) {
        HGLOBAL hGlobal = LoadResource(hModule, hResource);
        if (hGlobal) {
            void* pData = LockResource(hGlobal);
            if (pData) {
                if (!VerQueryValue(pData, L"\\", &pFixedFileInfo, &uPtrLen) ||
                    uPtrLen == 0) {
                    pFixedFileInfo = nullptr;
                    uPtrLen = 0;
                }
            }
        }
    }

    if (puPtrLen) {
        *puPtrLen = uPtrLen;
    }

    return (VS_FIXEDFILEINFO*)pFixedFileInfo;
}

WinVersion GetWindowsVersion() {
    VS_FIXEDFILEINFO* fixedFileInfo = GetModuleVersionInfo(nullptr, nullptr);
    if (!fixedFileInfo) {
        return WinVersion::Unsupported;
    }

    WORD major = HIWORD(fixedFileInfo->dwFileVersionMS);
    WORD minor = LOWORD(fixedFileInfo->dwFileVersionMS);
    WORD build = HIWORD(fixedFileInfo->dwFileVersionLS);
    WORD qfe = LOWORD(fixedFileInfo->dwFileVersionLS);

    Wh_Log(L"Version: %u.%u.%u.%u", major, minor, build, qfe);

    switch (major) {
        case 10:
            if (build < 22000) {
                return WinVersion::Win10;
            } else {
                return WinVersion::Win11;
            }
            break;
    }

    return WinVersion::Unsupported;
}

bool IsWinKeyPressed() {
    return GetAsyncKeyState(VK_LWIN) < 0 || GetAsyncKeyState(VK_RWIN) < 0;
}

bool HandleAltTabWindow(RECT* rect) {
    auto location = IsWinKeyPressed() ? g_settings.winAltTabLocation
                                      : g_settings.altTabLocation;

    if (location == Location::primary) {
        return false;
    }

    POINT pt;
    if (!GetCursorPos(&pt)) {
        return false;
    }

    auto hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

    MONITORINFO monInfo;
    monInfo.cbSize = sizeof(MONITORINFO);
    if (!GetMonitorInfo(hMon, &monInfo)) {
        return false;
    }

    *rect = monInfo.rcWork;
    return true;
}

void* CWin32ApplicationView_vtable;
void* CWinRTApplicationView_vtable;

using CWin32ApplicationView_v_GetNativeWindow_t =
    HRESULT(WINAPI*)(void* pThis, HWND* windowHandle);
CWin32ApplicationView_v_GetNativeWindow_t
    CWin32ApplicationView_v_GetNativeWindow;

using CWinRTApplicationView_v_GetNativeWindow_t =
    HRESULT(WINAPI*)(void* pThis, HWND* windowHandle);
CWinRTApplicationView_v_GetNativeWindow_t
    CWinRTApplicationView_v_GetNativeWindow;

HRESULT GetWindowHandleFromApplicationView(void* applicationView,
                                           HWND* windowHandle) {
    *windowHandle = nullptr;
    if (!applicationView) return E_INVALIDARG;
    void* vtable = *(void**)applicationView;
    HRESULT hr = E_FAIL;

    if (vtable == CWin32ApplicationView_vtable) {
        hr = CWin32ApplicationView_v_GetNativeWindow(applicationView,
                                                     windowHandle);
    } else if (vtable == CWinRTApplicationView_vtable) {
        hr = CWinRTApplicationView_v_GetNativeWindow(applicationView,
                                                     windowHandle);
    }

    return hr;
}

bool IsWindowOnCursorMonitor(HWND windowHandle) {
    POINT pt;
    if (!GetCursorPos(&pt)) {
        return true;  // Keep native visibility if the cursor cannot be queried.
    }

    auto hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    auto hMonFromWindow =
        MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST);

    return !hMon || !hMonFromWindow || hMon == hMonFromWindow;
}

using CVirtualDesktop_IsViewVisible_t = HRESULT(WINAPI*)(void* pThis,
                                                         void* applicationView,
                                                         BOOL* isVisible);
CVirtualDesktop_IsViewVisible_t CVirtualDesktop_IsViewVisible_Original;
HRESULT WINAPI CVirtualDesktop_IsViewVisible_Hook(void* pThis,
                                                  void* applicationView,
                                                  BOOL* isVisible) {
    Wh_Log(L">");
    auto ret = CVirtualDesktop_IsViewVisible_Original(pThis, applicationView,
                                                      isVisible);
    if (FAILED(ret)) {
        return ret;
    }

    if (g_threadIdForXamlAltTabViewHost_CreateInstance !=
        GetCurrentThreadId()) {
        // A focused window might be added after a short period. Filter windows
        // using our monitor rules if the alt tab window was just opened.
        // Otherwise, don't play with the filter anymore, as it's also used by
        // other components such as Win+Tab and the taskbar.
        if ((GetTickCount64() - g_CreateInstance_TickCount) > kDeltaThreshold ||
            g_lastThreadIdForXamlAltTabViewHost_CreateInstance !=
                GetCurrentThreadId()) {
            return ret;
        }
    }

    if (!*isVisible) {
        return ret;
    }

    HWND windowHandle;
    HRESULT hr =
        GetWindowHandleFromApplicationView(applicationView, &windowHandle);

    if (FAILED(hr) || !windowHandle) {
        return ret;
    }

    if (!IsWindowOnCursorMonitor(windowHandle)) {
        *isVisible = FALSE;
    }

    return ret;
}

using XamlAltTabViewHost_Show_t = HRESULT(WINAPI*)(void* pThis,
                                                   void* param1,
                                                   int param2,
                                                   void* param3);
XamlAltTabViewHost_Show_t XamlAltTabViewHost_Show_Original;
HRESULT WINAPI XamlAltTabViewHost_Show_Hook(void* pThis,
                                            void* param1,
                                            int param2,
                                            void* param3) {
    Wh_Log(L">");
    g_threadIdForAltTabShowWindow = GetCurrentThreadId();
    HRESULT ret =
        XamlAltTabViewHost_Show_Original(pThis, param1, param2, param3);
    g_threadIdForAltTabShowWindow = 0;
    return ret;
}

using CAltTabViewHost_Show_t = HRESULT(WINAPI*)(void* pThis,
                                                void* param1,
                                                int param2,
                                                void* param3);
CAltTabViewHost_Show_t CAltTabViewHost_Show_Original;
HRESULT WINAPI CAltTabViewHost_Show_Hook(void* pThis,
                                         void* param1,
                                         int param2,
                                         void* param3) {
    Wh_Log(L">");
    g_threadIdForAltTabShowWindow = GetCurrentThreadId();
    HRESULT ret = CAltTabViewHost_Show_Original(pThis, param1, param2, param3);
    g_threadIdForAltTabShowWindow = 0;
    return ret;
}

using ITaskGroupWindowInformation_Position_t =
    HRESULT(WINAPI*)(void* pThis, winrt::Windows::Foundation::Rect* rect);
ITaskGroupWindowInformation_Position_t
    ITaskGroupWindowInformation_Position_Original;
HRESULT WINAPI ITaskGroupWindowInformation_Position_Hook(
    void* pThis,
    winrt::Windows::Foundation::Rect* rect) {
    if (g_threadIdForAltTabShowWindow != GetCurrentThreadId()) {
        return ITaskGroupWindowInformation_Position_Original(pThis, rect);
    }
    g_threadIdForAltTabShowWindow = 0;

    RECT newRectNative;
    if (!HandleAltTabWindow(&newRectNative)) {
        return ITaskGroupWindowInformation_Position_Original(pThis, rect);
    }

    winrt::Windows::Foundation::Rect newRect{
        static_cast<float>(newRectNative.left),
        static_cast<float>(newRectNative.top),
        static_cast<float>(newRectNative.right - newRectNative.left),
        static_cast<float>(newRectNative.bottom - newRectNative.top),
    };

    HRESULT ret =
        ITaskGroupWindowInformation_Position_Original(pThis, &newRect);

    return ret;
}

using CMultitaskingViewFrame_CreateFrame_t = HRESULT(WINAPI*)(void* pThis,
                                                              RECT* rect,
                                                              void* param2);
CMultitaskingViewFrame_CreateFrame_t
    CMultitaskingViewFrame_CreateFrame_Original;
HRESULT WINAPI CMultitaskingViewFrame_CreateFrame_Hook(void* pThis,
                                                       RECT* rect,
                                                       void* param2) {
    if (g_threadIdForAltTabShowWindow != GetCurrentThreadId()) {
        return CMultitaskingViewFrame_CreateFrame_Original(pThis, rect, param2);
    }
    g_threadIdForAltTabShowWindow = 0;

    RECT newRect;
    if (!HandleAltTabWindow(&newRect)) {
        return CMultitaskingViewFrame_CreateFrame_Original(pThis, rect, param2);
    }

    HRESULT ret =
        CMultitaskingViewFrame_CreateFrame_Original(pThis, &newRect, param2);

    return ret;
}

HRESULT CreateInstanceHook(std::function<HRESULT()> original) {
    g_lastThreadIdForXamlAltTabViewHost_CreateInstance = 0;
    g_CreateInstance_TickCount = 0;
    auto windows = IsWinKeyPressed() ? g_settings.winAltTabWindows
                                     : g_settings.altTabWindows;

    if (windows == WindowsToShow::allMonitors) {
        return original();
    }

    if (windows == WindowsToShow::cursorMonitorExceptPrimary) {
        POINT pt;
        if (GetCursorPos(&pt)) {
            HMONITOR hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
            MONITORINFO monInfo;
            monInfo.cbSize = sizeof(MONITORINFO);
            if (GetMonitorInfo(hMon, &monInfo) &&
                (monInfo.dwFlags & MONITORINFOF_PRIMARY)) {
                return original();
            }
        }
    }

    g_threadIdForXamlAltTabViewHost_CreateInstance = GetCurrentThreadId();
    g_lastThreadIdForXamlAltTabViewHost_CreateInstance = GetCurrentThreadId();
    g_CreateInstance_TickCount = GetTickCount64();
    HRESULT ret = original();
    g_threadIdForXamlAltTabViewHost_CreateInstance = 0;
    return ret;
}

using XamlAltTabViewHost_CreateInstance_t = HRESULT(WINAPI*)(void* pThis,
                                                             void* param1,
                                                             void* param2,
                                                             void* param3);
XamlAltTabViewHost_CreateInstance_t XamlAltTabViewHost_CreateInstance_Original;
HRESULT WINAPI XamlAltTabViewHost_CreateInstance_Hook(void* pThis,
                                                      void* param1,
                                                      void* param2,
                                                      void* param3) {
    Wh_Log(L">");
    return CreateInstanceHook([=]() {
        return XamlAltTabViewHost_CreateInstance_Original(pThis, param1, param2,
                                                          param3);
    });
}

using CAltTabViewHost_CreateInstance_t = HRESULT(WINAPI*)(void* pThis,
                                                          void* param1,
                                                          void* param2,
                                                          void* param3,
                                                          void* param4,
                                                          void* param5,
                                                          void* param6,
                                                          void* param7,
                                                          void* param8,
                                                          void* param9,
                                                          void* param10,
                                                          void* param11);
CAltTabViewHost_CreateInstance_t CAltTabViewHost_CreateInstance_Original;
HRESULT WINAPI CAltTabViewHost_CreateInstance_Hook(void* pThis,
                                                   void* param1,
                                                   void* param2,
                                                   void* param3,
                                                   void* param4,
                                                   void* param5,
                                                   void* param6,
                                                   void* param7,
                                                   void* param8,
                                                   void* param9,
                                                   void* param10,
                                                   void* param11) {
    Wh_Log(L">");
    return CreateInstanceHook([=]() {
        return CAltTabViewHost_CreateInstance_Original(
            pThis, param1, param2, param3, param4, param5, param6, param7,
            param8, param9, param10, param11);
    });
}

using CAltTabViewHost_CreateInstance_Win11_t = HRESULT(WINAPI*)(void* pThis,
                                                                void* param1,
                                                                void* param2,
                                                                void* param3,
                                                                void* param4,
                                                                void* param5,
                                                                void* param6,
                                                                void* param7,
                                                                void* param8,
                                                                void* param9,
                                                                void* param10);
CAltTabViewHost_CreateInstance_Win11_t
    CAltTabViewHost_CreateInstance_Win11_Original;
HRESULT WINAPI CAltTabViewHost_CreateInstance_Win11_Hook(void* pThis,
                                                         void* param1,
                                                         void* param2,
                                                         void* param3,
                                                         void* param4,
                                                         void* param5,
                                                         void* param6,
                                                         void* param7,
                                                         void* param8,
                                                         void* param9,
                                                         void* param10) {
    Wh_Log(L">");
    return CreateInstanceHook([=]() {
        return CAltTabViewHost_CreateInstance_Win11_Original(
            pThis, param1, param2, param3, param4, param5, param6, param7,
            param8, param9, param10);
    });
}

void LoadSettings() {
    auto loadLocation = [](PCWSTR name, Location defaultVal) {
        PCWSTR val = Wh_GetStringSetting(name);
        Location result = defaultVal;
        if (val && wcscmp(val, L"primary") == 0) {
            result = Location::primary;
        } else if (val && wcscmp(val, L"cursor") == 0) {
            result = Location::cursor;
        }
        Wh_FreeStringSetting(val);
        return result;
    };

    auto loadWindows = [](PCWSTR name, WindowsToShow defaultVal) {
        PCWSTR val = Wh_GetStringSetting(name);
        WindowsToShow result = defaultVal;
        if (val && wcscmp(val, L"allMonitors") == 0) {
            result = WindowsToShow::allMonitors;
        } else if (val && wcscmp(val, L"cursorMonitor") == 0) {
            result = WindowsToShow::cursorMonitor;
        } else if (val && wcscmp(val, L"cursorMonitorExceptPrimary") == 0) {
            result = WindowsToShow::cursorMonitorExceptPrimary;
        }
        Wh_FreeStringSetting(val);
        return result;
    };

    g_settings.altTabLocation =
        loadLocation(L"altTabLocation", Location::cursor);
    g_settings.altTabWindows =
        loadWindows(L"altTabWindows", WindowsToShow::cursorMonitor);
    g_settings.winAltTabLocation =
        loadLocation(L"winAltTabLocation", Location::primary);
    g_settings.winAltTabWindows =
        loadWindows(L"winAltTabWindows", WindowsToShow::allMonitors);
}

BOOL Init(HMODULE twinuiPcshellModule) {
    LoadSettings();
    g_winVersion = GetWindowsVersion();

    if (g_winVersion == WinVersion::Win11) {
        // twinui.pcshell.dll
        WindhawkUtils::SYMBOL_HOOK twinuiPcshellSymbolHooks[] = {
            {
                {LR"(public: virtual long __cdecl CVirtualDesktop::IsViewVisible(struct IApplicationView *,int *))"},
                &CVirtualDesktop_IsViewVisible_Original,
                CVirtualDesktop_IsViewVisible_Hook,
            },
            {
                {LR"(const CWin32ApplicationView::`vftable'{for `IApplicationView'})"},
                &CWin32ApplicationView_vtable,
            },
            {
                {LR"(private: virtual long __cdecl CWin32ApplicationView::v_GetNativeWindow(struct HWND__ * *))"},
                &CWin32ApplicationView_v_GetNativeWindow,
            },
            {
                {LR"(const CWinRTApplicationView::`vftable'{for `IApplicationView'})"},
                &CWinRTApplicationView_vtable,
            },
            {
                {LR"(private: virtual long __cdecl CWinRTApplicationView::v_GetNativeWindow(struct HWND__ * *))"},
                &CWinRTApplicationView_v_GetNativeWindow,
            },
            {
                {LR"(public: virtual long __cdecl XamlAltTabViewHost::Show(struct IImmersiveMonitor *,enum ALT_TAB_VIEW_FLAGS,struct IApplicationView *))"},
                &XamlAltTabViewHost_Show_Original,
                XamlAltTabViewHost_Show_Hook,
            },
            {
                {LR"(public: __cdecl winrt::impl::consume_Windows_Internal_Shell_TaskGroups_ITaskGroupWindowInformation<struct winrt::Windows::Internal::Shell::TaskGroups::ITaskGroupWindowInformation>::Position(struct winrt::Windows::Foundation::Rect const &)const )"},
                &ITaskGroupWindowInformation_Position_Original,
                ITaskGroupWindowInformation_Position_Hook,
            },
            {
                {LR"(long __cdecl XamlAltTabViewHost_CreateInstance(struct XamlViewHostInitializeArgs const &,struct _GUID const &,void * *))"},
                &XamlAltTabViewHost_CreateInstance_Original,
                XamlAltTabViewHost_CreateInstance_Hook,
            },
            // For the old Win10 (non-XAML) Alt+Tab (can be enabled with
            // ExplorerPatcher):
            {
                {LR"(public: virtual long __cdecl CAltTabViewHost::Show(struct IImmersiveMonitor *,enum ALT_TAB_VIEW_FLAGS,struct IApplicationView *))"},
                &CAltTabViewHost_Show_Original,
                CAltTabViewHost_Show_Hook,
                true,
            },
            {
                {LR"(public: virtual long __cdecl CMultitaskingViewFrame::CreateFrame(struct Geometry::CRect const &,enum MultitaskingViewZOrder))"},
                &CMultitaskingViewFrame_CreateFrame_Original,
                CMultitaskingViewFrame_CreateFrame_Hook,
                true,
            },
            {
                {LR"(long __cdecl CAltTabViewHost_CreateInstance(struct IMultitaskingData *,struct IMultitaskingViewManagerInternal *,struct IApplicationViewSwitcher *,struct IImmersiveAppCrusher *,struct IMultitaskingViewVisibilityServiceInternal *,struct IMultitaskingViewGestureState *,struct IApplicationViewCollection *,struct ITouchGestureSettings *,struct _GUID const &,void * *))"},
                &CAltTabViewHost_CreateInstance_Win11_Original,
                CAltTabViewHost_CreateInstance_Win11_Hook,
                true,
            },
        };

        if (!HookSymbols(twinuiPcshellModule, twinuiPcshellSymbolHooks,
                         ARRAYSIZE(twinuiPcshellSymbolHooks))) {
            Wh_Log(L"HookSymbols failed");
            return FALSE;
        }
    } else if (g_winVersion == WinVersion::Win10) {
        // twinui.pcshell.dll
        WindhawkUtils::SYMBOL_HOOK twinuiPcshellSymbolHooks[] = {
            {
                {LR"(public: virtual long __cdecl CVirtualDesktop::IsViewVisible(struct IApplicationView *,int *))"},
                &CVirtualDesktop_IsViewVisible_Original,
                CVirtualDesktop_IsViewVisible_Hook,
            },
            {
                {LR"(const CWin32ApplicationView::`vftable'{for `IApplicationView'})"},
                &CWin32ApplicationView_vtable,
            },
            {
                {LR"(private: virtual long __cdecl CWin32ApplicationView::v_GetNativeWindow(struct HWND__ * *))"},
                &CWin32ApplicationView_v_GetNativeWindow,
            },
            {
                {LR"(const CWinRTApplicationView::`vftable'{for `IApplicationView'})"},
                &CWinRTApplicationView_vtable,
            },
            {
                {LR"(private: virtual long __cdecl CWinRTApplicationView::v_GetNativeWindow(struct HWND__ * *))"},
                &CWinRTApplicationView_v_GetNativeWindow,
            },
            {
                {LR"(public: virtual long __cdecl CAltTabViewHost::Show(struct IImmersiveMonitor *,enum ALT_TAB_VIEW_FLAGS,struct IApplicationView *))"},
                &CAltTabViewHost_Show_Original,
                CAltTabViewHost_Show_Hook,
            },
            {
                {LR"(public: virtual long __cdecl CMultitaskingViewFrame::CreateFrame(struct Geometry::CRect const &,enum MultitaskingViewZOrder))"},
                &CMultitaskingViewFrame_CreateFrame_Original,
                CMultitaskingViewFrame_CreateFrame_Hook,
            },
            {
                {LR"(long __cdecl CAltTabViewHost_CreateInstance(struct IMultitaskingData *,struct IMultitaskingViewManagerInternal *,struct IApplicationViewSwitcher *,struct IImmersiveAppCrusher *,struct IMultitaskingViewVisibilityServiceInternal *,struct IMultitaskingViewGestureState *,struct IApplicationViewCollection *,struct Windows::Internal::ComposableShell::Tabs::ITabController *,struct ITabViewManager *,struct _GUID const &,void * *))"},
                &CAltTabViewHost_CreateInstance_Original,
                CAltTabViewHost_CreateInstance_Hook,
            },
        };

        if (!HookSymbols(twinuiPcshellModule, twinuiPcshellSymbolHooks,
                         ARRAYSIZE(twinuiPcshellSymbolHooks))) {
            Wh_Log(L"HookSymbols failed");
            return FALSE;
        }
    } else {
        Wh_Log(L"Unsupported Windows version");
        return FALSE;
    }

    return TRUE;
}


}  // namespace AltTab

namespace StartMenu {
enum class Location { cursor, primary, custom };
struct Settings {
    Location location = Location::cursor;
    int monitor = 1;
    std::wstring interfaceName;
} settings;

// Set only for the dynamic extent of this thread's Start invocation.
// A nested call must restore its parent's target, not clear it.
thread_local bool inShow = false;
thread_local HMONITOR target = nullptr;

HMONITOR SelectMonitor(const Settings& selection = settings) {
    if (selection.location == Location::cursor) {
        POINT point{};
        if (!GetCursorPos(&point)) return nullptr;
        return MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);
    }
    if (selection.location == Location::primary) {
        return MonitorFromPoint(POINT{}, MONITOR_DEFAULTTOPRIMARY);
    }
    struct Search { const Settings& selection; int index = 0; HMONITOR found = nullptr; } search{selection};
    EnumDisplayMonitors(nullptr, nullptr,
        [](HMONITOR monitor, HDC, LPRECT, LPARAM data) -> BOOL {
            auto& state = *reinterpret_cast<Search*>(data);
            const auto& selection = state.selection;
            ++state.index;
            if (selection.interfaceName.empty()) {
                if (state.index != selection.monitor) return TRUE;
            } else {
                MONITORINFOEXW info{};
                info.cbSize = sizeof(info);
                if (!GetMonitorInfoW(monitor, &info)) return TRUE;
                DISPLAY_DEVICEW device{};
                device.cb = sizeof(device);
                if (!EnumDisplayDevicesW(info.szDevice, 0, &device,
                                         EDD_GET_DEVICE_INTERFACE_NAME)) return TRUE;
                Wh_Log(L"Found display device %s, interface name: %s",
                       info.szDevice, device.DeviceID);
                if (!wcsstr(device.DeviceID, selection.interfaceName.c_str())) return TRUE;
            }
            state.found = monitor;
            return FALSE;
        }, reinterpret_cast<LPARAM>(&search));
    return search.found;
}

struct ShowScope {
    bool previousInShow = inShow;
    HMONITOR previousTarget = target;
    ShowScope() { target = SelectMonitor(); inShow = true; }
    ~ShowScope() { target = previousTarget; inShow = previousInShow; }
    ShowScope(const ShowScope&) = delete;
    ShowScope& operator=(const ShowScope&) = delete;
};

using Connect_t = bool(WINAPI*)(void*, HWND, POINT);
using Show_t = HRESULT(WINAPI*)(void*, int, int);
using Adjust_t = HRESULT(WINAPI*)(void*);
Connect_t ConnectOriginal;
Show_t ShowOriginal;
Adjust_t AdjustOriginal;

HRESULT WINAPI ShowHook(void* self, int method, int flags) {
    ShowScope scope;
    return ShowOriginal(self, method, flags);
}

HRESULT WINAPI AdjustHook(void* self) {
    if (!inShow || !target) return AdjustOriginal(self);
    MONITORINFO info{};
    info.cbSize = sizeof(info);
    // A display can disappear after the keyboard/click event.
    if (!GetMonitorInfoW(target, &info)) return AdjustOriginal(self);
    const RECT& rect = info.rcMonitor;
    if (rect.right <= rect.left || rect.bottom <= rect.top) return AdjustOriginal(self);
    POINT center{rect.left + (rect.right - rect.left) / 2,
                 rect.top + (rect.bottom - rect.top) / 2};
    // The return value is NOT a success flag. In this Windows implementation it
    // is false when the helper already targets this monitor (no transition),
    // even though the selection is valid. Calling AdjustOriginal in that case
    // reselects the primary monitor. Match upstream: connect, then return S_OK.
    ConnectOriginal(self, nullptr, center);
    return S_OK;
}

void LoadSettings() {
    PCWSTR location = Wh_GetStringSetting(L"startMenuLocation");
    settings.location = Location::cursor;
    if (location && wcscmp(location, L"primary") == 0) settings.location = Location::primary;
    else if (location && wcscmp(location, L"custom") == 0) settings.location = Location::custom;
    Wh_FreeStringSetting(location);
    settings.monitor = Wh_GetIntSetting(L"startMenuMonitor");
    PCWSTR name = Wh_GetStringSetting(L"startMenuMonitorInterfaceName");
    settings.interfaceName = name ? name : L"";
    Wh_FreeStringSetting(name);
}

BOOL Init(HMODULE shell) {
    LoadSettings();
    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {
            {LR"(public: bool __cdecl ImmersiveMonitorHelper::ConnectToMonitor(struct HWND__ *,struct tagPOINT))"},
            &ConnectOriginal,
        },
        {
            {LR"(public: virtual long __cdecl XamlLauncher::ShowStartView(enum IMMERSIVELAUNCHERSHOWMETHOD,enum IMMERSIVELAUNCHERSHOWFLAGS))"},
            &ShowOriginal, ShowHook,
        },
        {
            {LR"(public: long __cdecl ImmersiveMonitorHelper::AdjustMonitorConnectedIfNeeded(void))"},
            &AdjustOriginal, AdjustHook,
        },
    };
    return WindhawkUtils::HookSymbols(shell, hooks, ARRAYSIZE(hooks));
}
}  // namespace StartMenu

namespace Notifications {
// Explorer chooses the monitor before the shell calculates bounds, DPI and
// clipping. Route that choice, leaving native layout and animation intact.
// Toast PositionView entry point: m417z, notifications-placement v1.2.4 (GPL3).
// Win+N GetViewPosition entry point verified against the local Microsoft PDB.
StartMenu::Settings settings;
enum class Kind { none, toast, center };
thread_local Kind active = Kind::none;
thread_local HMONITOR target = nullptr;
thread_local bool selecting = false;

using Position_t = HRESULT(WINAPI*)(void*);
using ViewPosition_t = HRESULT(WINAPI*)(void*, RECT*);
Position_t ToastPositionOriginal;
ViewPosition_t CenterPositionOriginal;
decltype(&MonitorFromPoint) MonitorFromPointOriginal;

bool IsValidMonitor(HMONITOR monitor) {
    MONITORINFO info{sizeof(info)};
    return monitor && GetMonitorInfoW(monitor, &info) &&
           info.rcMonitor.right > info.rcMonitor.left &&
           info.rcMonitor.bottom > info.rcMonitor.top;
}

struct PositionScope {
    Kind previousActive = active;
    HMONITOR previousTarget = target;
    explicit PositionScope(Kind kind) {
        // A nested notification positioning call must sample the real cursor,
        // not the enclosing hook's redirected MonitorFromPoint result.
        struct SelectionScope {
            bool previous = selecting;
            SelectionScope() { selecting = true; }
            ~SelectionScope() { selecting = previous; }
        } selectionScope;
        HMONITOR selected = StartMenu::SelectMonitor(settings);
        target = IsValidMonitor(selected) ? selected : nullptr;
        active = kind;
    }
    ~PositionScope() { target = previousTarget; active = previousActive; }
    PositionScope(const PositionScope&) = delete;
    PositionScope& operator=(const PositionScope&) = delete;
};

HMONITOR WINAPI MonitorFromPointHook(POINT point, DWORD flags) {
    // Toast's native primary-monitor query is (0,0). Win+N may instead use
    // the cached invocation point through GetMonitorFromLastInvocationPoint.
    if (!selecting && active != Kind::none && target &&
        (active == Kind::center || (point.x == 0 && point.y == 0)) &&
        IsValidMonitor(target)) {
        return target;
    }
    return MonitorFromPointOriginal(point, flags);
}

HRESULT WINAPI ToastPositionHook(void* self) {
    PositionScope scope(Kind::toast);
    Wh_Log(L"Notification banner target: %p", target);
    return ToastPositionOriginal(self);
}

HRESULT WINAPI CenterPositionHook(void* self, RECT* rect) {
    PositionScope scope(Kind::center);
    Wh_Log(L"Notification center target: %p", target);
    return CenterPositionOriginal(self, rect);
}

void LoadSettings() {
    PCWSTR location = Wh_GetStringSetting(L"notificationLocation");
    settings.location = StartMenu::Location::cursor;
    if (location && wcscmp(location, L"primary") == 0)
        settings.location = StartMenu::Location::primary;
    else if (location && wcscmp(location, L"custom") == 0)
        settings.location = StartMenu::Location::custom;
    Wh_FreeStringSetting(location);
    settings.monitor = Wh_GetIntSetting(L"notificationMonitor");
    PCWSTR name = Wh_GetStringSetting(L"notificationMonitorInterfaceName");
    settings.interfaceName = name ? name : L"";
    Wh_FreeStringSetting(name);
}

BOOL Init(HMODULE shell, bool toast, bool center) {
    LoadSettings();
    if (toast) {
        WindhawkUtils::SYMBOL_HOOK hooks[] = {
            {{LR"(private: long __cdecl CToastCenterExperienceManager::PositionView(void))"},
             &ToastPositionOriginal, ToastPositionHook},
        };
        if (!WindhawkUtils::HookSymbols(shell, hooks, ARRAYSIZE(hooks))) return FALSE;
    }
    if (center) {
        WindhawkUtils::SYMBOL_HOOK hooks[] = {
            {{LR"(private: long __cdecl CActionCenterExperienceManager::GetViewPosition(struct tagRECT &))"},
             &CenterPositionOriginal, CenterPositionHook},
        };
        if (!WindhawkUtils::HookSymbols(shell, hooks, ARRAYSIZE(hooks))) return FALSE;
    }
    return WindhawkUtils::SetFunctionHook(MonitorFromPoint, MonitorFromPointHook,
                                           &MonitorFromPointOriginal);
}
}  // namespace Notifications

namespace ShowDesktop {
// The Win+D hotkey ID and Explorer entry point were identified in easyatm's
// win-d-per-monitor v1.3.260330. The per-monitor transaction below is independent.
// Unlike a two-argument free function, this member method has a this pointer
// AND both hotkey arguments. Preserve all three for unrelated shortcuts.
using Hotkey_t = void(__cdecl*)(void*, WPARAM, LPARAM);
Hotkey_t HotkeyOriginal;
constexpr WPARAM kWinD = 0x201;
constexpr PCWSTR kProperty = L"Windhawk.MultiMonitorShell.ShowDesktop";
constexpr ULONGLONG kPendingMilliseconds = 1000;
bool useForeground = false;
bool unloading = false;
std::mutex mutex;
ULONG_PTR nextMarker = 0;

struct Window {
    HWND hwnd = nullptr;
    DWORD processId = 0;
    DWORD threadId = 0;
    UINT restoreCommand = SW_SHOWNOACTIVATE;
    HANDLE marker = nullptr;
};
struct Batch {
    HMONITOR monitor = nullptr;
    HWND foreground = nullptr;
    ULONGLONG requestedAt = 0;
    std::vector<Window> windows;
};
std::map<HMONITOR, Batch> batches;

HMONITOR WindowMonitor(HWND hwnd) {
    return MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
}
bool VisibleOnDesktop(HWND hwnd) {
    if (!IsWindowVisible(hwnd) || IsIconic(hwnd)) return false;
    DWORD cloaked = 0;
    // If the compositor cannot identify a window, leave it alone.
    return SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked,
                                           sizeof(cloaked))) && !cloaked;
}

HWND RootOwner(HWND hwnd) {
    // GA_ROOTOWNER follows GetParent, which omits owners of overlapped windows.
    // GW_OWNER also covers non-popup owned windows that hide with their owner.
    for (int depth = 0; depth < 64; ++depth) {
        HWND owner = GetWindow(hwnd, GW_OWNER);
        if (!owner) return hwnd;
        if (owner == hwnd) return nullptr;
        hwnd = owner;
    }
    return nullptr;
}

bool CollectWindows(HMONITOR target, std::vector<Window>& result) {
    struct Entry { HWND hwnd; HWND owner; HMONITOR monitor; };
    std::vector<Entry> visible;
    if (!EnumWindows([](HWND hwnd, LPARAM data) -> BOOL {
            if (!VisibleOnDesktop(hwnd)) return TRUE;
            auto& list = *reinterpret_cast<std::vector<Entry>*>(data);
            list.push_back({hwnd, RootOwner(hwnd), WindowMonitor(hwnd)});
            return TRUE;
        }, reinterpret_cast<LPARAM>(&visible))) return false;

    // Minimizing an owner also hides its owned dialogs. Do not minimize that
    // owner if any visible member of its ownership group is on another screen.
    std::set<HWND> crossMonitorOwners;
    for (const auto& entry : visible) {
        if (entry.monitor != target && entry.owner && entry.owner != entry.hwnd)
            crossMonitorOwners.insert(entry.owner);
    }
    for (const auto& entry : visible) {
        if (entry.monitor != target || entry.owner != entry.hwnd ||
            crossMonitorOwners.contains(entry.hwnd)) continue;
        const auto style = GetWindowLongPtrW(entry.hwnd, GWL_STYLE);
        const auto exStyle = GetWindowLongPtrW(entry.hwnd, GWL_EXSTYLE);
        if (!(style & WS_MINIMIZEBOX) || (style & WS_CHILD) ||
            (exStyle & (WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE))) continue;
        WCHAR className[128]{};
        if (!GetClassNameW(entry.hwnd, className, ARRAYSIZE(className))) continue;
        if (!wcscmp(className, L"Progman") || !wcscmp(className, L"WorkerW") ||
            !wcscmp(className, L"Shell_TrayWnd") ||
            !wcscmp(className, L"Shell_SecondaryTrayWnd")) continue;
        WINDOWPLACEMENT placement{sizeof(placement)};
        if (!GetWindowPlacement(entry.hwnd, &placement)) continue;
        Window window;
        window.hwnd = entry.hwnd;
        window.threadId = GetWindowThreadProcessId(entry.hwnd, &window.processId);
        if (!window.threadId) continue;
        window.restoreCommand = placement.showCmd == SW_SHOWMAXIMIZED
                                    ? SW_SHOWMAXIMIZED : SW_SHOWNOACTIVATE;
        result.push_back(window);
    }
    return true;
}

bool SameWindow(const Window& window) {
    DWORD processId = 0;
    const DWORD threadId = GetWindowThreadProcessId(window.hwnd, &processId);
    return threadId && threadId == window.threadId && processId == window.processId;
}
bool MarkWindow(Window& window) {
    if (!SameWindow(window) || GetPropW(window.hwnd, kProperty)) return false;
    if (!nextMarker) nextMarker = static_cast<ULONG_PTR>(GetTickCount64()) | 1;
    window.marker = reinterpret_cast<HANDLE>(++nextMarker);
    return SetPropW(window.hwnd, kProperty, window.marker) != FALSE;
}
bool OwnsWindow(const Window& window) {
    // Properties disappear when a HWND is destroyed, protecting against handle
    // reuse even when the new window belongs to the same process and thread.
    return SameWindow(window) && window.marker &&
           GetPropW(window.hwnd, kProperty) == window.marker;
}
void ForgetWindow(const Window& window) {
    if (OwnsWindow(window)) RemovePropW(window.hwnd, kProperty);
}

// Effects are injectable so the restore state machine can be tested without
// minimizing any user windows. Production uses the Win32 functions below.
struct Api {
    bool (*collect)(HMONITOR, std::vector<Window>&) = CollectWindows;
    bool (*mark)(Window&) = MarkWindow;
    bool (*owns)(const Window&) = OwnsWindow;
    void (*forget)(const Window&) = ForgetWindow;
    HMONITOR (*monitor)(HWND) = WindowMonitor;
    decltype(&IsIconic) iconic = IsIconic;
    decltype(&ShowWindowAsync) show = ShowWindowAsync;
    decltype(&GetForegroundWindow) foreground = GetForegroundWindow;
    decltype(&SetForegroundWindow) focus = SetForegroundWindow;
    decltype(&GetTickCount64) now = GetTickCount64;
} api;

bool Pending(const Batch& batch) {
    return api.now() - batch.requestedAt < kPendingMilliseconds;
}
void Prune() {
    for (auto group = batches.begin(); group != batches.end();) {
        auto& batch = group->second;
        for (auto it = batch.windows.begin(); it != batch.windows.end();) {
            if (!api.owns(*it) || api.monitor(it->hwnd) != batch.monitor ||
                (!api.iconic(it->hwnd) && !Pending(batch))) {
                api.forget(*it);
                it = batch.windows.erase(it);
            } else ++it;
        }
        if (batch.windows.empty()) group = batches.erase(group);
        else ++group;
    }
}

void Restore(Batch& batch) {
    HWND focus = nullptr;
    // Enumeration is top-to-bottom; restore from bottom to top. Never use
    // TOPMOST/NOTOPMOST tricks, which would change an app's always-on-top state.
    for (auto it = batch.windows.rbegin(); it != batch.windows.rend(); ++it) {
        if (api.owns(*it) && api.monitor(it->hwnd) == batch.monitor &&
            (api.iconic(it->hwnd) || Pending(batch))) {
            // A quick second press also queues a restore behind an outstanding
            // async minimize request instead of losing that window's record.
            if (api.show(it->hwnd, static_cast<int>(it->restoreCommand))) {
                if (it->hwnd == batch.foreground) focus = it->hwnd;
            }
        }
        api.forget(*it);
    }
    if (focus) api.focus(focus);
}

void ToggleMonitor(HMONITOR monitor) {
    std::lock_guard lock(mutex);
    if (unloading || !monitor) return;
    Prune();
    if (auto existing = batches.find(monitor); existing != batches.end()) {
        Restore(existing->second);
        batches.erase(existing);
        return;
    }
    std::vector<Window> candidates;
    if (!api.collect(monitor, candidates) || candidates.empty()) return;
    auto& batch = batches[monitor];
    batch.monitor = monitor;
    batch.foreground = api.foreground();
    batch.requestedAt = api.now();
    for (auto& window : candidates) {
        // Recheck after enumeration; a user or app may have changed its state.
        if (api.iconic(window.hwnd) || api.monitor(window.hwnd) != monitor ||
            !api.mark(window)) continue;
        batch.windows.push_back(window);
        if (!api.show(window.hwnd, SW_MINIMIZE)) {
            api.forget(window);
            batch.windows.pop_back();
        }
    }
    if (batch.windows.empty()) batches.erase(monitor);
}

HMONITOR SelectMonitor() {
    HMONITOR monitor = nullptr;
    if (useForeground) {
        if (HWND foreground = GetForegroundWindow()) monitor = WindowMonitor(foreground);
    } else {
        POINT cursor{};
        if (GetCursorPos(&cursor)) monitor = MonitorFromPoint(cursor, MONITOR_DEFAULTTONEAREST);
    }
    MONITORINFO info{sizeof(info)};
    return monitor && GetMonitorInfoW(monitor, &info) &&
                   info.rcMonitor.right > info.rcMonitor.left &&
                   info.rcMonitor.bottom > info.rcMonitor.top ? monitor : nullptr;
}

void __cdecl HotkeyHook(void* self, WPARAM hotkey, LPARAM details) {
    if (hotkey != kWinD) {
        HotkeyOriginal(self, hotkey, details);
        return;
    }
    // No global fallback: failure to determine one monitor must never minimize
    // the user's windows on all screens while this feature is enabled.
    try { ToggleMonitor(SelectMonitor()); }
    catch (...) { Wh_Log(L"Win+D monitor operation failed; global action suppressed."); }
}

BOOL Init() {
    PCWSTR location = Wh_GetStringSetting(L"showDesktopLocation");
    useForeground = location && wcscmp(location, L"foreground") == 0;
    Wh_FreeStringSetting(location);
    HMODULE explorer = GetModuleHandleW(nullptr);
    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {{LR"(protected: void __cdecl CTray::_HandleGlobalHotkey(unsigned __int64,__int64))"},
         &HotkeyOriginal, HotkeyHook},
    };
    return explorer && WindhawkUtils::HookSymbols(explorer, hooks, ARRAYSIZE(hooks));
}

void Shutdown() {
    std::lock_guard lock(mutex);
    unloading = true;
    for (auto& [monitor, batch] : batches) Restore(batch);
    batches.clear();
}
}  // namespace ShowDesktop

BOOL Wh_ModInit() {
    const bool altTabEnabled = Wh_GetIntSetting(L"altTabEnabled") != 0;
    const bool startMenuEnabled = Wh_GetIntSetting(L"startMenuEnabled") != 0;
    const bool toastEnabled = Wh_GetIntSetting(L"notificationToastEnabled") != 0;
    const bool centerEnabled = Wh_GetIntSetting(L"notificationCenterEnabled") != 0;
    const bool desktopEnabled = Wh_GetIntSetting(L"showDesktopEnabled") != 0;
    if (desktopEnabled && !ShowDesktop::Init()) {
        Wh_Log(L"Win+D symbols unavailable; initialization stopped.");
        return FALSE;
    }
    if (!altTabEnabled && !startMenuEnabled && !toastEnabled && !centerEnabled) return TRUE;
    // Explorer owns this module. Do not add a reference on every settings reload.
    HMODULE shell = GetModuleHandleW(L"twinui.pcshell.dll");
    if (!shell) {
        Wh_Log(L"twinui.pcshell.dll is not loaded; initialization stopped.");
        return FALSE;
    }
    if (altTabEnabled && !AltTab::Init(shell)) {
        Wh_Log(L"Alt+Tab symbols unavailable; initialization stopped.");
        return FALSE;
    }
    if (startMenuEnabled && !StartMenu::Init(shell)) {
        Wh_Log(L"Start menu symbols unavailable; initialization stopped.");
        return FALSE;
    }
    if ((toastEnabled || centerEnabled) && !Notifications::Init(shell, toastEnabled, centerEnabled)) {
        Wh_Log(L"Notification symbols unavailable; initialization stopped.");
        return FALSE;
    }
    return TRUE;
}

void Wh_ModBeforeUninit() {
    ShowDesktop::Shutdown();
}

BOOL Wh_ModSettingsChanged(BOOL* reload) {
    // Windhawk removes/reinstalls hooks, keeping settings immutable while in use.
    *reload = TRUE;
    return TRUE;
}
