#include "app/launch.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <map>
#include <mutex>
#include <stdexcept>
#include <system_error>
#include <utility>

#include "app/config.h"  // exe_dir
#include "core/winstr.h"

namespace hydra::app {

namespace {

std::optional<std::string> g_sibling_dir_for_test;

// The program called `name` in `dir`, or nullopt when no such file is there.
std::optional<std::string> exe_in(const std::string& dir, const std::string& name) {
    const std::string path = join_folder(dir, name);
    if (!file_exists_utf8(path) || is_directory_utf8(path)) return std::nullopt;
    return path;
}

}  // namespace

std::wstring quote_argument(const std::wstring& arg) {
    // A run of backslashes is literal unless a quote follows it, so the run
    // is doubled before a quote inside the argument (and that quote gets one
    // more) and before the closing quote; anywhere else it stays as it is.
    std::wstring out = L"\"";
    size_t backslashes = 0;
    for (const wchar_t c : arg) {
        if (c == L'\\') {
            ++backslashes;
            continue;
        }
        if (c == L'"') {
            out.append(backslashes * 2 + 1, L'\\');
        } else {
            out.append(backslashes, L'\\');
        }
        out.push_back(c);
        backslashes = 0;
    }
    out.append(backslashes * 2, L'\\');
    out.push_back(L'"');
    return out;
}

std::wstring command_line(const std::wstring& exe, const std::vector<std::wstring>& args) {
    // The program's own name is split by a different rule: everything between
    // its quotes is taken as written, backslashes included. A path holds no
    // quote, so plain quotes are all it needs.
    std::wstring cmd = L"\"" + exe + L"\"";
    for (const std::wstring& arg : args) cmd += L" " + quote_argument(arg);
    return cmd;
}

void launch_program(const std::wstring& exe, const std::vector<std::wstring>& args) {
    std::wstring cmd = command_line(exe, args);
    STARTUPINFOW si{};
    si.cb = sizeof si;
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(exe.c_str(), cmd.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr,
                        &si, &pi)) {
        std::string reason = std::system_category().message(static_cast<int>(GetLastError()));
        // Some of Windows' reasons name the program as "%1" ("%1 is not a
        // valid Win32 application."); put its path there.
        const size_t at = reason.find("%1");
        if (at != std::string::npos) reason.replace(at, 2, wide_to_utf8(exe));
        throw std::runtime_error(reason);
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}

std::optional<std::string> sibling_exe_path(const std::string& name) {
    if (g_sibling_dir_for_test) return exe_in(*g_sibling_dir_for_test, name);
    // The song panel asks every frame; the file system is asked once per name.
    static std::mutex mutex;
    static std::map<std::string, std::optional<std::string>> found;
    const std::lock_guard<std::mutex> lock(mutex);
    const auto it = found.find(name);
    if (it != found.end()) return it->second;
    return found.emplace(name, exe_in(exe_dir(), name)).first->second;
}

void set_sibling_dir_for_test(std::optional<std::string> dir) {
    g_sibling_dir_for_test = std::move(dir);
}

void launch_sibling(const std::string& name, const std::vector<std::string>& paths) {
    const std::optional<std::string> exe = sibling_exe_path(name);
    if (!exe) throw std::runtime_error(name + " is not in Hydra's folder");
    std::vector<std::wstring> wide;
    wide.reserve(paths.size());
    for (const std::string& path : paths) wide.push_back(win32_path(path));
    launch_program(win32_path(*exe), wide);
}

}  // namespace hydra::app
