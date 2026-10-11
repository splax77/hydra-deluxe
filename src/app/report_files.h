// The one way to put a finished report page on disk and hand it to the
// browser. Only the fill comparison writes a page now (hydra_fillcompare,
// src/cli/fillcompare.cpp). The browser call sits behind a settable seam so a
// test can record opens instead of performing them.

#ifndef HYDRA_APP_REPORT_FILES_H
#define HYDRA_APP_REPORT_FILES_H

#include <filesystem>
#include <functional>
#include <string>

namespace hydra::app {

// The "open a file in the browser" seam behind open_in_browser (ShellExecute
// by default). A harness with no desktop installs one that just records the
// path; an empty function restores the default.
using OpenInBrowserFn = std::function<bool(const std::wstring& path)>;
void set_open_in_browser(OpenInBrowserFn fn);

// Hands one file to the default browser through that seam. Returns false when
// the shell refuses (e.g. the file doesn't exist yet). A path fits_shell
// refuses (too long, or prefixed) goes to the browser as its short 8.3 name;
// with no short name the browser gets copy_to_short_temp's copy instead
// (docs/adr/0020).
bool open_in_browser(const std::wstring& path);

// Copies a report page to %TEMP%\Hydra\<its name>, overwriting an older copy,
// and returns the copy's path; empty when the copy fails or its path is one
// fits_shell refuses. A report is one self-contained file, so the copy
// shows the same page.
std::filesystem::path copy_to_short_temp(const std::filesystem::path& page);

// Writes `bytes` to a "<path>.tmp" sibling first and then renames it over
// the target, so a reader never sees a half-written file: the previous one
// stays whole right up until the new one is complete. False when the temp
// file won't open or write, or the swap fails; the temp is removed then.
// The one write-then-rename helper (D114 item 20): the report page and the
// settings file both save through it.
bool write_file_replacing(const std::filesystem::path& outpath, const std::string& bytes);

// Writes a built page to disk through write_file_replacing. Throws
// std::runtime_error("cannot write <path>") when it fails.
void write_report_file(const std::filesystem::path& outpath, const std::string& html);

}  // namespace hydra::app

#endif  // HYDRA_APP_REPORT_FILES_H
