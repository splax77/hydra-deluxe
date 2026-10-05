// The one place a test builds a per-process scratch path (audit finding 287).
// Every name lives in the Windows temp folder and carries a tag and this
// process's id, so two test processes running at once never share a file.
// The GetTempPathW call here is the only one under tests/ (the scan in
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

#include <filesystem>
#include <string>

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

}  // namespace testtemp

#endif  // HYDRA_TESTS_TEMP_UTIL_H
