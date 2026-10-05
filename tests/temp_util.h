// The one place a test builds a per-process scratch path (audit finding 287).
// Every name lives in the Windows temp folder and carries a tag and this
// process's id, so two test processes running at once never share a file.
// The temp-folder lookup here is the only one under tests/ (the scan in
// test_single_owner.cpp checks it).
//
// Both functions hand back UTF-8, the form every path helper in core/winstr
// takes; a caller that needs the wide form for a Win32 call converts once
// there with hydra::utf8_to_wide. The tag is UTF-8 too and is used as given,
// non-ASCII included.

#ifndef HYDRA_TESTS_TEMP_UTIL_H
#define HYDRA_TESTS_TEMP_UTIL_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "core/winstr.h"

namespace testtemp {

// A scratch file name: <temp>hydra_test_<tag>_<pid><ext>. Nothing is made on
// disk.
inline std::string temp_path(const std::string& tag, const std::string& ext) {
    wchar_t tmp[MAX_PATH];
    GetTempPathW(MAX_PATH, tmp);
    return hydra::wide_to_utf8(tmp) + "hydra_test_" + tag + "_" +
           std::to_string(GetCurrentProcessId()) + ext;
}

// A scratch folder, <temp>hydra_test_<tag>_<pid>, made if it is missing. It
// may still hold an earlier run's files; a caller that needs it empty clears
// it first.
inline std::string temp_dir(const std::string& tag) {
    const std::string dir = temp_path(tag, "");
    std::filesystem::create_directories(hydra::os_path(dir));
    return dir;
}

// Writes `data` to `path` byte for byte, replacing the file. The one writer
// for a test's binary fixture files; a text file goes through
// audiochart::write_text_file.
inline void write_bytes(const std::string& path, const std::vector<uint8_t>& data) {
    std::FILE* f = hydra::fopen_utf8(path, L"wb");
    if (f == nullptr) throw std::runtime_error("cannot write " + path);
    if (!data.empty()) std::fwrite(data.data(), 1, data.size(), f);
    std::fclose(f);
}

// A scratch file (temp_path) that is deleted when it goes out of scope.
// It removes the file on construction too, so a leftover from a crashed run
// does not confuse the test. Callers that need the wide path for Win32
// calls use hydra::win32_path on path.
struct ScopedFile {
    std::string path;
    ScopedFile(const std::string& tag, const std::string& ext)
        : path(temp_path(tag, ext)) { remove(); }
    ~ScopedFile() { remove(); }
    ScopedFile(ScopedFile&& o) noexcept : path(std::move(o.path)) { o.path.clear(); }
    ScopedFile(const ScopedFile&) = delete;
    ScopedFile& operator=(const ScopedFile&) = delete;
    operator const std::string&() const { return path; }
    operator std::filesystem::path() const { return hydra::os_path(path); }
private:
    void remove() {
        if (path.empty()) return;
        std::error_code ec;
        std::filesystem::remove(hydra::os_path(path), ec);
    }
};

}  // namespace testtemp

#endif  // HYDRA_TESTS_TEMP_UTIL_H
