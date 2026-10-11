#include "app/report_files.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>  // ShellExecuteW
#include <shlwapi.h>   // AssocQueryStringW

#include <fstream>
#include <stdexcept>
#include <system_error>
#include <utility>

#include "core/error_kind.h"
#include "core/winstr.h"

namespace hydra::app {

namespace {

OpenInBrowserFn g_open_in_browser;

}  // namespace

void set_open_in_browser(OpenInBrowserFn fn) { g_open_in_browser = std::move(fn); }

namespace {

bool shell_open(const std::wstring& path) {
    HINSTANCE rc = ShellExecuteW(nullptr, L"open", path.c_str(), nullptr,
                                 nullptr, SW_SHOWNORMAL);
    return shell_execute_ok(rc);
}

// Starts the program Windows opens .html files with, on `page`. For a short
// 8.3 path: the shell expands one back to the long path and then fails, but
// Firefox, Edge and Chrome all open it when it's handed to them directly
// (measured 2026-10-03, docs/adr/0020).
bool launch_html_viewer(const std::wstring& page) {
    DWORD n = 0;
    AssocQueryStringW(ASSOCF_NONE, ASSOCSTR_EXECUTABLE, L".html", L"open", nullptr, &n);
    if (n == 0) return false;
    std::wstring exe(n, L'\0');
    if (FAILED(AssocQueryStringW(ASSOCF_NONE, ASSOCSTR_EXECUTABLE, L".html", L"open",
                                 &exe[0], &n)))
        return false;
    exe.resize(wcslen(exe.c_str()));
    std::wstring cmd = L"\"" + exe + L"\" \"" + page + L"\"";
    STARTUPINFOW si{};
    si.cb = sizeof si;
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(exe.c_str(), &cmd[0], nullptr, nullptr, FALSE, 0, nullptr, nullptr,
                        &si, &pi))
        return false;
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

}  // namespace

std::filesystem::path copy_to_short_temp(const std::filesystem::path& page) {
    wchar_t tmp[MAX_PATH + 1];
    const DWORD n = GetTempPathW(MAX_PATH + 1, tmp);
    if (n == 0 || n > MAX_PATH) return {};
    const std::filesystem::path dir = std::filesystem::path(tmp) / L"Hydra";
    const std::filesystem::path copy = dir / page.filename();
    if (!fits_shell(copy.native())) return {};
    std::error_code ec;
    std::filesystem::create_directories(os_path(dir), ec);
    std::filesystem::copy_file(os_path(page), os_path(copy),
                               std::filesystem::copy_options::overwrite_existing, ec);
    return ec ? std::filesystem::path() : copy;
}

bool open_in_browser(const std::wstring& path) {
    if (g_open_in_browser) return g_open_in_browser(path);
    if (fits_shell(path)) return shell_open(path);
    // The shell can't open the page at this path, so hand the browser the
    // page's short name. When there is no short name, or the .html viewer
    // won't launch, open a copy at a short path instead.
    const std::wstring short_form = shell_path(path);
    if (!short_form.empty() && launch_html_viewer(short_form)) return true;
    const std::filesystem::path copy = copy_to_short_temp(path);
    return !copy.empty() && shell_open(copy.wstring());
}

bool write_file_replacing(const std::filesystem::path& outpath, const std::string& bytes,
                          LineEnds ends) {
    // A reader can open the file at any moment, including mid-write. The
    // bytes go to a temp name first and are swapped into place with one
    // rename once complete. On FAT and exFAT a replacing rename is a delete
    // and then a rename (store/upgrade_files.cpp says why the library swap
    // cannot accept that); a stop between the two loses the old file, which
    // is acceptable here because a report page and the settings are written
    // again on the next save.
    std::filesystem::path tmp = outpath;
    tmp += ".tmp";
    std::error_code ec;

    // Text mode is Windows' own "\n" to "\r\n" conversion, the one the
    // settings file was written with before it came through here.
    std::ios::openmode mode = std::ios::out | std::ios::trunc;
    if (ends == LineEnds::AsGiven) mode |= std::ios::binary;
    std::ofstream f(os_path(tmp), mode);
    if (!f) return false;
    f << bytes;
    f.close();
    if (!f) {
        std::filesystem::remove(os_path(tmp), ec);
        return false;
    }
    std::filesystem::rename(os_path(tmp), os_path(outpath), ec);
    if (ec) {
        std::error_code ignored;
        std::filesystem::remove(os_path(tmp), ignored);
        return false;
    }
    return true;
}

void write_report_file(const std::filesystem::path& outpath, const std::string& html) {
    if (!write_file_replacing(outpath, html, LineEnds::AsGiven))
        throw KindedError(ErrorKind::ReportWrite, "cannot write " + outpath.u8string());
}

}  // namespace hydra::app
