# Third-party notices

This combined project is distributed under GPL-3.0-only. See LICENSE.

## Alt+Tab per monitor

- Author: L3r0y and contributors.
- Version: 1.2.
- Source: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/alt-tab-per-monitor.wh.cpp
- Input: the user's installed source, copied on 2026-10-08 without altering the installation.
- Input SHA-256: `155B569275DF24049189B52362B929657BB684D0C33EB9BB0D91ED414C624DE6`.
- License: MIT. The source has no explicit license declaration; the Windhawk mods repository README specifies MIT for such contributions as retrieved on 2026-10-08.
- Changes: distinct mod identity, namespace separation, Chinese settings, feature switch, reload on setting changes, null-view guard, preserve visibility if monitor queries fail, reset the old filter interval on a new invocation, integration tests.

The following MIT notice is also embedded in the single-file mod:

Copyright (c) L3r0y and contributors

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

## Start menu open location

- Author: Michael Maltsev (m417z).
- Version: 1.0.
- Source: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/start-menu-open-location.wh.cpp
- Retrieved: 2026-10-08.
- Input SHA-256: `1659C1B5DCD6D6C25364CBA846496FCA550C015758E4CCF0BFE92CF15BC848AE`.
- License: GNU General Public License v3.0, explicitly stated in the upstream source.
- Changes: namespace separation, feature switch, primary/custom/cursor modes, immutable settings during each load, capture target at invocation, nested scope restoration, monitor-query failure and disconnected-display fallbacks, integration tests. Version 0.1.1 restores the upstream treatment of ConnectToMonitor's boolean return value; it must not be interpreted as a generic success flag.

Windhawk and its compiler/runtime are external dependencies and are not bundled in this source project. The GPL license text is included without modification.

## Customize Windows notifications placement

- Author: Michael Maltsev (m417z).
- Version: 1.2.4, retrieved 2026-10-08.
- Source: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/notifications-placement.wh.cpp
- Input SHA-256: 6DB88DF8163514053CDF7899A8B4D5EADA3A5ED5729C76DAC26B0AB3068D0D1C.
- License: GNU General Public License v3.0, explicitly stated in the upstream source.
- Reused approach: scope CToastCenterExperienceManager::PositionView and redirect its primary MonitorFromPoint query.
- Changes: thread-local nested scope, capture target before native positioning, checked monitor lookup, independent switches, native layout and animations, Win+N routing verified using the local Microsoft PDB, integration tests. No upstream corner-positioning or XAML animation code is included.
