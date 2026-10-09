// The one way to put a finished report page on disk and hand it to the
// browser. Only the fill comparison writes a page now (hydra_fillcompare,
// src/cli/fillcompare.cpp). The browser call sits behind a settable seam so a
// test can record opens instead of performing them.

#ifndef HYDRA_APP_REPORT_FILES_H
#define HYDRA_APP_REPORT_FILES_H

#include <filesystem>
#include <functional>
#include <optional>
#include <string>

namespace hydra::app {

// ---- left only until tests/test_app_state.cpp stops naming report_html_path
// (RW-T7's handoff). Nothing in the app reads these; delete all four then.

// The folder the path report page was saved in: Documents\Hydra, made on
// first use. It falls back to the database's folder when Documents can't be
// found or the Hydra folder can't be made there. When a harness has
// overridden the database path (app::set_path_overrides), it is that
// database's folder instead.
std::filesystem::path reports_dir();

// The seam behind reports_dir's Documents lookup (SHGetKnownFolderPath by
// default). An empty function restores the default.
using DocumentsDirFn = std::function<std::optional<std::filesystem::path>()>;
void set_documents_dir_lookup(DocumentsDirFn fn);

// The old path report page's file name, and its path in reports_dir().
inline constexpr const char* kPathReportFileName = "hydra_paths.html";
std::wstring report_html_path();

// ---- the page plumbing hydra_fillcompare uses

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

// Writes a built page to disk. The page is written to a "<path>.tmp" sibling
// first and then renamed over the target, so a reader who opens the report
// mid-write never sees a half-written page — the previous page stays
// readable right up until the new one is complete. Throws
// std::runtime_error("cannot write <path>") when the file won't open or the
// swap fails.
void write_report_file(const std::filesystem::path& outpath, const std::string& html);

}  // namespace hydra::app

#endif  // HYDRA_APP_REPORT_FILES_H
