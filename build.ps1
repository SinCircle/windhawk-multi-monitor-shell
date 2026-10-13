[CmdletBinding()]
param(
    [string]$WindhawkRoot,
    [string]$BuildDirectory = (Join-Path $PSScriptRoot 'work'),
    [ValidatePattern('^(local@)?[a-z0-9-]+$')]
    [string]$RuntimeModId = 'multi-monitor-shell',
    [switch]$Test,
    [switch]$CheckSymbols
)
$ErrorActionPreference = 'Stop'
if (-not $WindhawkRoot) {
    $taskWindhawkProcess = Get-Process windhawk -ErrorAction SilentlyContinue |
        Where-Object Path | Select-Object -First 1
    if ($taskWindhawkProcess) { $WindhawkRoot = Split-Path $taskWindhawkProcess.Path -Parent }
}
if (-not $WindhawkRoot) { $WindhawkRoot = Join-Path $env:ProgramFiles 'Windhawk' }
$WindhawkRoot = (Resolve-Path -LiteralPath $WindhawkRoot).Path
$taskIni = Get-Content -LiteralPath (Join-Path $WindhawkRoot 'windhawk.ini')
$taskEngineLine = $taskIni | Where-Object { $_ -match '^EnginePath=' } | Select-Object -First 1
if (-not $taskEngineLine) { throw 'EnginePath is missing from windhawk.ini.' }
$taskEngine = Join-Path $WindhawkRoot ($taskEngineLine.Substring('EnginePath='.Length))
$taskDataLine = $taskIni | Where-Object { $_ -match '^AppDataPath=' } | Select-Object -First 1
if (-not $taskDataLine) { throw 'AppDataPath is missing from windhawk.ini.' }
$taskDataRoot = [Environment]::ExpandEnvironmentVariables($taskDataLine.Substring('AppDataPath='.Length))
if (-not [IO.Path]::IsPathRooted($taskDataRoot)) { $taskDataRoot = Join-Path $WindhawkRoot $taskDataRoot }
$taskCompiler = Join-Path $WindhawkRoot 'Compiler'
$taskClang = Join-Path $taskCompiler 'bin\clang++.exe'
if (-not (Test-Path -LiteralPath $taskClang)) { throw 'Windhawk C++ compiler not found.' }
New-Item -ItemType Directory -Force -Path $BuildDirectory | Out-Null
$BuildDirectory = (Resolve-Path -LiteralPath $BuildDirectory).Path
$taskBaseFlags = @('-x','c++','-std=c++23','-target','x86_64-w64-mingw32',
    '-DUNICODE','-D_UNICODE','-DWINVER=0x0A00','-D_WIN32_WINNT=0x0A00',
    '-D_WIN32_IE=0x0A00','-DNTDDI_VERSION=0x0A000008','-D__USE_MINGW_ANSI_STDIO=0',
    '-DWH_MOD','-include','windhawk_api.h','-Wall','-Wextra','-Werror',
    '-Wno-unused-parameter','-Wno-missing-field-initializers',
    '-Wno-cast-function-type-mismatch','-Wno-pragma-pack',
    '-Wno-pragma-system-header-outside-header',
    '-I',(Join-Path $taskCompiler 'include'))
$taskLibraries = @('-L',(Join-Path $taskCompiler 'lib'),'-lc++','-lunwind',
    '-lole32','-loleaut32','-lversion')
$taskSource = Join-Path $PSScriptRoot 'multi-monitor-shell.wh.cpp'
$taskVersionMatch = [regex]::Match((Get-Content -LiteralPath $taskSource -Raw), '(?m)^// @version\s+(\S+)')
if (-not $taskVersionMatch.Success) { throw 'Mod version not found.' }
$taskVersion = $taskVersionMatch.Groups[1].Value
$taskDll = Join-Path $BuildDirectory ('multi-monitor-shell_' + $taskVersion + '.dll')
& $taskClang @taskBaseFlags ('-DWH_MOD_ID=L"' + $RuntimeModId + '"') ('-DWH_MOD_VERSION=L"' + $taskVersion + '"') `
    '-O2' '-shared' '-Wl,--export-all-symbols' $taskSource '-L' (Join-Path $taskEngine '64') '-lwindhawk' @taskLibraries '-o' $taskDll
if ($LASTEXITCODE -ne 0) { throw "Mod compilation failed: $LASTEXITCODE" }
Write-Output "BUILT: $taskDll"

$taskSavedPath = $env:PATH
try {
    # Resolve the bundled runtime without copying DLLs to Windows or installing anything.
    $env:PATH = (Join-Path $taskDataRoot 'Engine\Mods\64') + ';' + (Join-Path $taskCompiler 'bin') + ';' + $env:PATH
    if ($Test) {
        $taskTestExe = Join-Path $BuildDirectory 'test-routing.exe'
        & $taskClang @taskBaseFlags '-DWH_EDITING' '-O1' (Join-Path $PSScriptRoot 'tests\test-routing.cpp') `
            @taskLibraries '-o' $taskTestExe
        if ($LASTEXITCODE -ne 0) { throw "Test compilation failed: $LASTEXITCODE" }
        & $taskTestExe
        if ($LASTEXITCODE -ne 0) { throw "Routing tests failed: $LASTEXITCODE" }
    }
    if ($CheckSymbols) {
        $taskSymbolCache = Join-Path $BuildDirectory 'symbols'
        & python (Join-Path $PSScriptRoot 'tools\fetch-symbols.py') --cache $taskSymbolCache --source $taskSource
        if ($LASTEXITCODE -ne 0) { throw "Symbol preparation failed: $LASTEXITCODE" }
        $taskSymbolExe = Join-Path $BuildDirectory 'check-symbols.exe'
        & $taskClang @taskBaseFlags '-DWH_EDITING' '-O1' '-municode' `
            (Join-Path $PSScriptRoot 'tests\check-symbols.cpp') @taskLibraries '-ldbghelp' '-o' $taskSymbolExe
        if ($LASTEXITCODE -ne 0) { throw "Symbol checker compilation failed: $LASTEXITCODE" }
        & $taskSymbolExe $taskSymbolCache (Join-Path $taskSymbolCache 'required.txt')
        if ($LASTEXITCODE -ne 0) { throw "Current Windows symbols do not match: $LASTEXITCODE" }
    }
} finally { $env:PATH = $taskSavedPath }
Write-Output 'Completed. No mod was installed and Explorer was not modified.'
