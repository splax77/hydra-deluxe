// The one place a test builds a per-process scratch path (audit finding 287).
// Every name lives in the Windows temp folder and carries a tag and this
// process's id, so two test processes running at once never share a file.
// The temp-folder lookup here is the only one under tests/ (a single-owner
// row in tests/single_owner/rules/tests.json checks it).
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

// The Windows temp folder, with its trailing backslash, as Windows hands it
// back. The one temp-folder lookup under tests/.
inline std::string temp_root() {
    wchar_t tmp[MAX_PATH];
    GetTempPathW(MAX_PATH, tmp);
    return hydra::wide_to_utf8(tmp);
}

// A scratch file name: <temp>hydra_test_<tag>_<pid><ext>. Nothing is made on
// disk.
inline std::string temp_path(const std::string& tag, const std::string& ext) {
    return temp_root() + "hydra_test_" + tag + "_" + std::to_string(GetCurrentProcessId()) + ext;
}

// True when `utf8_path` names something strictly inside the temp folder
// (temp_root), after both are made absolute and their "..", short names and
// links resolved as far as the disk allows. The folder itself, a path that
// only starts with the same letters, and a path that cannot be resolved are
// all false. Live mode uses it to refuse a seed that is not a copy.
inline bool is_under_temp(const std::string& utf8_path) {
    if (utf8_path.empty()) return false;
    std::error_code ec;
    const std::filesystem::path root =
        std::filesystem::weakly_canonical(hydra::os_path(temp_root()), ec);
    if (ec) return false;
    const std::filesystem::path p = std::filesystem::weakly_canonical(
        std::filesystem::absolute(hydra::os_path(utf8_path), ec), ec);
    if (ec) return false;
    auto r = root.begin();
    auto q = p.begin();
    for (; r != root.end(); ++r) {
        if (r->empty()) continue;  // the trailing separator's empty last part
        if (q == p.end()) return false;
        // Windows paths ignore case.
        if (CompareStringOrdinal(r->c_str(), -1, q->c_str(), -1, TRUE) != CSTR_EQUAL) return false;
        ++q;
    }
    for (; q != p.end(); ++q)
        if (!q->empty()) return true;  // something below the folder itself
    return false;
}

// A scratch folder, <temp>hydra_test_<tag>_<pid>, made if it is missing. It
// may still hold an earlier run's files; a caller that needs it empty clears
// it first.
inline std::string temp_dir(const std::string& tag) {
    const std::string dir = temp_path(tag, "");
    std::filesystem::create_directories(hydra::os_path(dir));
    return dir;
}

// A fresh, empty scratch folder whose name holds a non-ASCII "Zoë" (as UTF-8)
// ahead of the tag. For the tests that prove a reader copes with a path the
// ANSI code page cannot spell.
inline std::string non_ascii_dir(const std::string& tag) {
    const std::string dir = temp_dir("Zo\xC3\xAB_" + tag);
    std::error_code ec;
    std::filesystem::remove_all(hydra::os_path(dir), ec);
    std::filesystem::create_directories(hydra::os_path(dir));
    return dir;
}

// Deletes a scratch folder and what is in it. Nothing happens if it is gone.
inline void remove_dir(const std::string& utf8_dir) {
    std::error_code ec;
    std::filesystem::remove_all(hydra::os_path(utf8_dir), ec);
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
