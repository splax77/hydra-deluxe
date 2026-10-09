#include "app/report_files.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>  // AssocQueryStringW

#include <fstream>
#include <stdexcept>
#include <system_error>
#include <utility>

#include "app/config.h"
#include "core/error_kind.h"
#include "core/winstr.h"

namespace hydra::app {

namespace {

std::filesystem::path db_folder() {
    return std::filesystem::u8path(app::db_path()).parent_path();
}

// The user's Documents folder, or nullopt when Windows can't name one.
std::optional<std::filesystem::path> known_documents_dir() {
    PWSTR raw = nullptr;
    std::optional<std::filesystem::path> out;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &raw)) &&
        raw)
        out = std::filesystem::path(raw);
    CoTaskMemFree(raw);  // safe on nullptr
    return out;
}

DocumentsDirFn g_documents_dir;
OpenInBrowserFn g_open_in_browser;

}  // namespace

std::filesystem::path reports_dir() {
    // A harness pointed the app at a scratch database: keep its pages there.
    if (!path_overrides().db_path.empty()) return db_folder();

    std::optional<std::filesystem::path> docs =
        g_documents_dir ? g_documents_dir() : known_documents_dir();
    if (!docs || docs->empty()) return db_folder();

    const std::filesystem::path dir = *docs / L"Hydra";
    std::error_code ec;
    std::filesystem::create_directories(os_path(dir), ec);
    if (!std::filesystem::is_directory(os_path(dir), ec)) return db_folder();
    return dir;
}

void set_documents_dir_lookup(DocumentsDirFn fn) { g_documents_dir = std::move(fn); }

std::wstring report_html_path() {
    return (reports_dir() / std::filesystem::u8path(kPathReportFileName)).wstring();
}

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

void write_report_file(const std::filesystem::path& outpath, const std::string& html) {
    // A reader can open the page at any moment, including while we're
    // mid-write. To make sure they never see a half-written page, we finish
    // writing under a temp name first and only swap it into place with one
    // rename once it's complete.
    std::filesystem::path tmp = outpath;
    tmp += ".tmp";
    const auto cannot_write = [&outpath] {
        return KindedError(ErrorKind::ReportWrite, "cannot write " + outpath.u8string());
    };

    std::ofstream f(os_path(tmp), std::ios::binary | std::ios::trunc);
    if (!f) throw cannot_write();
    f << html;
    f.close();
    if (!f) {
        std::error_code ec;
        std::filesystem::remove(os_path(tmp), ec);
        throw cannot_write();
    }

    try {
        std::filesystem::rename(os_path(tmp), os_path(outpath));
    } catch (const std::filesystem::filesystem_error&) {
        std::error_code ec;
        std::filesystem::remove(os_path(tmp), ec);
        throw cannot_write();
    }
}

}  // namespace hydra::app
