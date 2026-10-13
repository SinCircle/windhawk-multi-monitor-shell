#include <windows.h>
#include <dbghelp.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>

std::set<std::string> missing;
BOOL CALLBACK Found(PSYMBOL_INFO symbol, ULONG, PVOID) {
    char name[8192]{};
    if (!UnDecorateSymbolName(symbol->Name, name, sizeof(name), 0x20800)) return TRUE;
    std::string value = name;
    for (size_t at; (at = value.find(" __ptr64")) != std::string::npos;) value.erase(at, 8);
    // Windhawk's DIA undecorator omits ptr64 qualifiers.
    for (auto it = missing.begin(); it != missing.end(); ++it) {
        std::string expected = *it;
        while (!expected.empty() && expected.back() == ' ') expected.pop_back();
        while (!value.empty() && value.back() == ' ') value.pop_back();
        if (value == expected) {
            std::printf("FOUND: %s\n", value.c_str());
            missing.erase(it);
            break;
        }
    }
    return TRUE;
}
int wmain(int argc, wchar_t** argv) {
    if (argc != 3) return 2;
    std::ifstream input{std::filesystem::path(argv[2])};
    for (std::string line; std::getline(input, line);) if (!line.empty()) missing.insert(line);
    if (missing.size() != 13) return 3;
    wchar_t system[MAX_PATH]{};
    if (!GetSystemDirectoryW(system, MAX_PATH)) return 4;
    auto dll = std::filesystem::path(system) / L"twinui.pcshell.dll";
    HANDLE process = GetCurrentProcess();
    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_EXACT_SYMBOLS);
    if (!SymInitializeW(process, argv[1], FALSE)) return 5;
    // DbgHelp maps symbols for inspection; never execute shell DLL initialization.
    auto base = SymLoadModuleExW(process, nullptr, dll.c_str(), nullptr, 0, 0, nullptr, 0);
    if (!base || !SymEnumSymbols(process, base, "*", Found, nullptr)) {
        std::printf("Symbol load failed: %lu\n", GetLastError());
        SymCleanup(process);
        return 6;
    }
    for (auto& name : missing) std::printf("MISSING: %s\n", name.c_str());
    SymCleanup(process);
    if (!missing.empty()) return 1;
    std::puts("PASS: all 13 required Win11 symbols match; no hooks were installed.");
    return 0;
}
