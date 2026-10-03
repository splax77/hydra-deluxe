#define _CRT_SECURE_NO_WARNINGS  // _wfopen

#include "core/winstr.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>  // CommandLineToArgvW

#include <stdexcept>

namespace hydra {

std::wstring utf8_to_wide(const std::string& s) {
    if (s.empty()) return L"";
    int wlen = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()),
                                   nullptr, 0);
    std::wstring w(static_cast<size_t>(wlen), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), &w[0], wlen);
    return w;
}

std::string wide_to_utf8(const std::wstring& w) {
    if (w.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, w.data(), static_cast<int>(w.size()),
                                  nullptr, 0, nullptr, nullptr);
    std::string s(static_cast<size_t>(len), '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), static_cast<int>(w.size()), &s[0], len,
                        nullptr, nullptr);
    return s;
}

namespace {

// The longest path every wide file function takes without the prefix.
// CreateDirectoryW's limit is the tightest: MAX_PATH minus room for an 8.3
// file name, so anything at or past it gets the prefix.
constexpr size_t kPlainPathLimit = MAX_PATH - 12;

bool starts_with(const std::wstring& s, const wchar_t* prefix) {
    return s.rfind(prefix, 0) == 0;
}

}  // namespace

std::wstring win32_path(const std::wstring& path) {
    if (path.size() < kPlainPathLimit) return path;
    if (starts_with(path, L"\\\\?\\") || starts_with(path, L"\\\\.\\")) return path;
    // Make it full first: under the prefix Windows no longer resolves "." or
    // ".." and no longer accepts '/' as a separator.
    DWORD need = GetFullPathNameW(path.c_str(), 0, nullptr, nullptr);
    if (need == 0) return path;  // leave it to the caller's own error path
    std::wstring full(need, L'\0');
    DWORD got = GetFullPathNameW(path.c_str(), need, &full[0], nullptr);
    if (got == 0 || got >= need) return path;
    full.resize(got);
    if (starts_with(full, L"\\\\?\\") || starts_with(full, L"\\\\.\\")) return full;
    if (starts_with(full, L"\\\\")) return L"\\\\?\\UNC\\" + full.substr(2);  // \\server\share
    return L"\\\\?\\" + full;
}

std::wstring win32_path(const std::string& utf8_path) {
    return win32_path(utf8_to_wide(utf8_path));
}

std::wstring shell_path(const std::wstring& path) {
    if (path.size() < MAX_PATH) return path;
    const std::wstring full = win32_path(path);
    const DWORD need = GetShortPathNameW(full.c_str(), nullptr, 0);
    if (need == 0) return L"";  // missing file
    std::wstring s(need, L'\0');
    const DWORD got = GetShortPathNameW(full.c_str(), &s[0], need);
    if (got == 0 || got >= need) return L"";
    s.resize(got);
    if (starts_with(s, L"\\\\?\\UNC\\")) s = L"\\\\" + s.substr(8);
    else if (starts_with(s, L"\\\\?\\")) s = s.substr(4);
    // A drive without short names hands the long path back.
    return s.size() < MAX_PATH ? s : L"";
}

std::filesystem::path os_path(const std::filesystem::path& p) {
    return std::filesystem::path(win32_path(p.native()));
}

std::filesystem::path os_path(const std::string& utf8_path) {
    return std::filesystem::path(win32_path(utf8_path));
}

std::FILE* fopen_utf8(const std::string& utf8_path, const wchar_t* mode) {
    return _wfopen(win32_path(utf8_path).c_str(), mode);
}

bool file_exists_utf8(const std::string& utf8_path) {
    return GetFileAttributesW(win32_path(utf8_path).c_str()) != INVALID_FILE_ATTRIBUTES;
}

bool is_directory_utf8(const std::string& utf8_path) {
    const DWORD attrs = GetFileAttributesW(win32_path(utf8_path).c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

std::vector<DirEntry> list_dir(const std::string& dir_utf8) {
    std::vector<DirEntry> out;
    // The pattern is built before win32_path so the prefix rule sees its full
    // length, "\*" included.
    const std::wstring pattern = win32_path(utf8_to_wide(dir_utf8) + L"\\*");
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(pattern.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return out;
    do {
        const std::wstring name = fd.cFileName;
        if (name == L"." || name == L"..") continue;
        DirEntry e;
        e.name = wide_to_utf8(name);
        e.is_dir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        e.size = (static_cast<uint64_t>(fd.nFileSizeHigh) << 32) | fd.nFileSizeLow;
        e.mtime = (static_cast<uint64_t>(fd.ftLastWriteTime.dwHighDateTime) << 32) |
                  fd.ftLastWriteTime.dwLowDateTime;
        out.push_back(std::move(e));
    } while (FindNextFileW(h, &fd));
    FindClose(h);
    return out;
}

std::string exe_path_utf8() {
    std::wstring buf(MAX_PATH, L'\0');
    for (;;) {
        const DWORD n = GetModuleFileNameW(nullptr, &buf[0], static_cast<DWORD>(buf.size()));
        if (n == 0) return "";
        if (n < buf.size()) {
            buf.resize(n);
            return wide_to_utf8(buf);
        }
        buf.resize(buf.size() * 2);  // truncated: try a bigger buffer
    }
}

uint64_t file_size_bytes(const std::string& utf8_path) {
    WIN32_FILE_ATTRIBUTE_DATA fa;
    if (!GetFileAttributesExW(win32_path(utf8_path).c_str(), GetFileExInfoStandard, &fa))
        throw std::runtime_error("cannot read file size: " + utf8_path);
    return (static_cast<uint64_t>(fa.nFileSizeHigh) << 32) | fa.nFileSizeLow;
}

std::vector<uint8_t> read_file_bytes(const std::string& utf8_path) {
    std::FILE* f = fopen_utf8(utf8_path, L"rb");
    if (f == nullptr) throw std::runtime_error("cannot open file: " + utf8_path);
    // std::ftell returns a 32-bit long on Windows and fails past 2 GB, which
    // used to hand back an empty buffer; the 64-bit pair has no such limit.
    const bool seeked = _fseeki64(f, 0, SEEK_END) == 0;
    const long long size = seeked ? _ftelli64(f) : -1;
    if (size < 0 || _fseeki64(f, 0, SEEK_SET) != 0) {
        std::fclose(f);
        throw std::runtime_error("cannot read file size: " + utf8_path);
    }
    std::vector<uint8_t> buf(static_cast<size_t>(size));
    if (size > 0) buf.resize(std::fread(buf.data(), 1, buf.size(), f));
    std::fclose(f);
    return buf;
}

std::string read_file_text(const std::string& utf8_path) {
    std::vector<uint8_t> bytes = read_file_bytes(utf8_path);
    return std::string(bytes.begin(), bytes.end());
}

std::vector<std::string> split_command_line_utf8(const std::wstring& command_line) {
    // An empty string makes CommandLineToArgvW return the exe path instead.
    if (command_line.empty()) return {};
    int argc = 0;
    LPWSTR* wargv = CommandLineToArgvW(command_line.c_str(), &argc);
    if (wargv == nullptr) throw std::runtime_error("cannot split the command line");
    std::vector<std::string> args;
    args.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; ++i) args.push_back(wide_to_utf8(wargv[i]));
    LocalFree(wargv);
    return args;
}

std::vector<std::string> utf8_argv() {
    return split_command_line_utf8(GetCommandLineW());
}

}  // namespace hydra
