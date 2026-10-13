# 多屏随行 / Multi-monitor Shell

把 Alt+Tab、开始菜单、通知横幅、Win+N 通知中心和 Win+D 单屏显示桌面整合进一个独立 Windhawk 模组。

模组 ID：`multi-monitor-shell` · 版本：`0.3.0` · 目标平台：Windows 11 x64

它仍由 Windhawk 管理，可以单独安装、配置、启停和继续开发。

模组源码：[multi-monitor-shell.wh.cpp](multi-monitor-shell.wh.cpp) · [验证记录](VALIDATION.md) · [上游来源与许可证](THIRD_PARTY_NOTICES.md)

## 功能与默认设置

| 功能 | 显示位置 | 窗口范围 |
|---|---|---|
| Alt+Tab | 鼠标所在屏幕 | 仅该屏幕的窗口 |
| Win+Alt+Tab | 主屏幕 | 所有屏幕的窗口 |
| 开始菜单 | 鼠标所在屏幕 | 保留 Windows 原生菜单内容 |
| 通知横幅 | 鼠标所在屏幕 | 保留 Windows 原生通知内容与动画 |
| Win+N 通知中心 | 鼠标所在屏幕 | 保留 Windows 原生通知列表与日历 |
| Win+D 显示桌面 | 鼠标所在屏幕 | 最小化本屏应用窗口，再按一次恢复本次最小化的窗口 |

Alt+Tab 默认值保留了整合时原模组的现有配置。位置和窗口范围可以分别调整，例如将 Alt+Tab 的窗口范围改为“所有屏幕”。

开始菜单还支持“主屏幕”和“指定显示器”。指定显示器时，可用从 1 开始的枚举序号，也可用显示器接口名称片段；填写接口名称后优先按名称匹配。枚举序号可能不同于 Windows 设置中的显示器编号。需要查看接口名称时，填写一个临时名称、开启 Windhawk 模组日志并打开开始菜单，日志会列出枚举到的接口。

开始菜单在打开时选定屏幕，打开后不会随鼠标移动。找不到指定显示器、显示器中途断开、鼠标位置读取失败时，回退到原生行为。

0.1.1 修复开始菜单偶尔回到主屏：原生选屏函数返回 `false` 也可能表示无需切换，不能当成失败再次调用系统默认选屏。修正版按上游方式处理，保留已选定的屏幕。

五项功能可以分别关闭。通知横幅和 Win+N 通知中心共用“通知：显示位置”，可选择鼠标所在屏幕、主屏幕或指定显示器，独立于开始菜单的设置。保存设置会重新加载本模组，使设置变化与系统钩子同步。

0.2.0 在 Explorer 计算通知布局前选择显示器，由 Windows 使用目标屏幕的尺寸和缩放信息。通知不会因鼠标移动而持续追随；系统下一次重新定位时才重新选屏。本模组不改变勿扰、专注、应用通知权限等系统规则。

0.3.0 新增 Win+D 单屏显示桌面，也可在设置中改为“当前活动窗口所在屏幕”。每块屏幕分别记录恢复状态；原本已最小化的窗口不恢复，最大化窗口恢复为最大化。窗口跨越多屏时按 Windows 判定的所属显示器处理。工具窗口、隐藏窗口、其他虚拟桌面的窗口以及带有跨屏附属窗口的主窗口会跳过；管理员窗口可能受 Windows 权限限制而保持原状。这里处理的是 Win+D，不改任务栏显示桌面按钮或触控板手势。

该功能只在按键时枚举窗口，没有后台轮询、计时器或额外常驻线程。只在内存中保存待恢复窗口的标识和状态；窗口关闭、移屏或被手动恢复后会在后续操作中清理。关闭或重载模组时尝试恢复仍由它管理的窗口。

## 安装

1. 在 Windhawk 中关闭原版 **Alt+Tab per monitor**；若装有 **Start menu open location**、**Customize Windows notifications placement** 或 **Win+D per monitor**，也先关闭它们。保留原模组便于恢复。
2. 使用 Windhawk 的“创建新模组”，把 `multi-monitor-shell.wh.cpp` 全部内容粘贴到编辑器，替换模板。
3. 编译并启用，退出编辑模式。在模组列表中可看到 **多屏随行：Alt+Tab、开始菜单、通知与桌面**。
4. 打开该模组设置，确认五个功能开关及显示位置。

这是完整的单文件模组；安装时不需要复制 `tests`、`tools` 或外部头文件。不要同时启用会修改相同系统函数的原版模组。

恢复原行为：关闭本模组。恢复此前 Alt+Tab 配置：关闭本模组后重新开启原版 Alt+Tab per monitor。

## 构建与检查

要求：Windows x64、Windhawk 1.7.3（本机验证版本）及其编译器。符号检查额外需要 Python 3.8+ 和微软公共符号服务器的网络访问。

在项目文件夹中运行：

```powershell
# 构建 DLL，自动查找正在运行的 Windhawk
.\build.ps1

# 构建并运行隔离测试
.\build.ps1 -Test

# 额外验证当前 Windows 的全部必要内部符号
.\build.ps1 -Test -CheckSymbols

# 可选：短暂创建虚构测试窗口，验证真实双屏最小化和恢复；只操作这些测试窗口
.\build.ps1 -TestWindowFixtures

# Windhawk 未运行或安装在其他目录时
.\build.ps1 -WindhawkRoot 'D:\Programs\Windhawk' -Test
```

默认构建产物位于项目的 `work` 文件夹；可用 `-BuildDirectory` 指定其他目录。脚本只编译和测试，不安装模组、不更改注册表、不向 Explorer 注入代码。日常安装建议让 Windhawk 从源码编译并管理 DLL。

## 项目结构

```text
multi-monitor-shell.wh.cpp  完整模组，含中文设置和上游署名
build.ps1                  可复现的构建、测试和符号检查
tests/test-routing.cpp     直接包含生产源码的隔离测试
tests/test-show-desktop.cpp Win+D 状态测试和可选双屏窗口测试
tests/check-symbols.cpp    使用 DbgHelp 检查精确系统符号
tools/fetch-symbols.py     按本机 DLL 的 GUID 和 age 下载匹配 PDB
VALIDATION.md              本次验证结果与双屏验收步骤
THIRD_PARTY_NOTICES.md     上游版本、来源和许可证
LICENSE                    GPL-3.0 许可证全文
```

## 验证范围

0.3.0 已通过严格编译、39 组隔离测试和真实双屏测试窗口检查，并确认当前系统所需的 14 个符号全部存在。完整结果及实际安装验收状态见 [VALIDATION.md](VALIDATION.md)。

2026-10-13 已确认 0.3.0 在测试机器的 Explorer 中实际加载成功（Loaded）。0.1.1 的开始菜单连续打开选屏已由用户确认，0.3.0 保留该修复；通知横幅与 Win+N 的实际位置、裁切和长期稳定性仍待人工验收。Windows 更新可能改变内部函数；符号匹配只确认接口可定位，不保证未来版本的行为兼容性。

若某个启用功能缺少必要符号，初始化返回失败并写入日志；不会把只有另一项功能工作的状态报告为完整加载成功。可以关闭不兼容功能后单独使用另一项。

## 开关已开启，但功能没有生效

Windhawk 显示“已启用”不代表模组已成功加载。首次使用新模组 ID 时，Windhawk 可能需要重新下载 Windows 的调试符号；下载中断会导致初始化失败并留下失败缓存。旧版 Alt+Tab 模组可以继续工作，也不能排除新 ID 的符号下载问题。

2026-10-08 在本机确认过此情况：`twinui.pcshell.pdb` 只下载到 19,845,120 字节，完整文件是 32,780,288 字节，运行状态为 `explorer.exe|Unloaded`。补齐与当前系统 DLL 匹配的 PDB 并清除该模组的符号失败缓存后，需在 Windhawk 中关闭再打开本模组，触发重新加载。

项目的 `-CheckSymbols` 使用项目构建目录中的独立缓存，不会自动把符号安装到 Windhawk 的缓存中。故“检查通过”与“Windhawk 已成功加载”是两个不同检查项。启用日志后应检查符号下载、初始化错误和实际进程加载状态，再判断是否为设置问题。

## 来源与许可

基于 L3r0y 的 [Alt+Tab per monitor 1.2](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/alt-tab-per-monitor.wh.cpp) 和 m417z 的 [Start menu open location 1.0](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/start-menu-open-location.wh.cpp)。

通知横幅定位入口参考 m417z 的 [Customize Windows notifications placement 1.2.4](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/notifications-placement.wh.cpp)；Win+N 路由依据本机系统符号与调用路径实现。

Win+D 的热键入口参考 easyatm 的 [Win+D per monitor 1.3.260330](https://github.com/ramensoftware/windhawk-mods/blob/df4fa4ff702cbeb34e8ec0eb2e1ae65dd3e26767/mods/win-d-per-monitor.wh.cpp)，按屏幕记录和恢复窗口的逻辑独立实现。

整体项目使用 GPL-3.0-only。Alt+Tab 部分保留 MIT 许可声明。具体来源及修改见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
