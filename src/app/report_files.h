// Where the two HTML report pages live on disk, and the one way to put a
// finished page there and hand it to the browser. Both report entry points
// read this: the GUI's ReportJob/DmReportJob (ui/library_jobs.h, ui/dm_jobs.h)
// and the hydra_report CLI, which previously carried its own copy of the
// write + ShellExecute sequence. The browser call sits behind a settable seam
// so the headless GUI test runner can record opens instead of performing them.

#ifndef HYDRA_APP_REPORT_FILES_H
#define HYDRA_APP_REPORT_FILES_H

#include <filesystem>
#include <functional>
#include <optional>
#include <string>

namespace hydra::app {

// The folder every report page is saved in: Documents\Hydra, made on first
// use. It falls back to the database's folder when Documents can't be found
// or the Hydra folder can't be made there. When a harness has overridden the
// database path (app::set_path_overrides), reports stay next to that
// database instead, so no test ever writes into the real Documents folder.
std::filesystem::path reports_dir();

// The seam behind reports_dir's Documents lookup (SHGetKnownFolderPath by
// default). A test installs one that returns a scratch folder, or nullopt to
// act like a machine with no Documents folder; an empty function restores
// the default.
using DocumentsDirFn = std::function<std::optional<std::filesystem::path>()>;
void set_documents_dir_lookup(DocumentsDirFn fn);

// Each report page's file name, typed once (finding 202). UTF-8, so the CLI's
// std::string --out default reads them as they are; report_html_path and
// dm_report_html_path build their paths from them.
inline constexpr const char* kPathReportFileName = "hydra_paths.html";
inline constexpr const char* kDmReportFileName = "hydra_dmcompare.html";

// Where the batch path report lives on disk (in reports_dir()).
std::wstring report_html_path();

// Where the comparison page lives on disk (in reports_dir()).
std::wstring dm_report_html_path();

// Whether a previously built report page exists on disk (gates the library
// view's "Open path report" button).
bool report_file_exists();

// The "open a file in the browser" seam behind every report open (ShellExecute
// by default). A harness with no desktop (the GUI test runner) installs one
// that just records the path; an empty function restores the default.
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

// Opens the report page in the default browser. Returns false when the shell
// refuses (e.g. the file doesn't exist yet).
bool open_report_in_browser();
bool open_dm_report_in_browser();

// Writes a built page to disk. The page is written to a "<path>.tmp" sibling
// first and then renamed over the target, so a reader who opens the report
// mid-write never sees a half-written page — the previous page stays
// readable right up until the new one is complete. Throws
// std::runtime_error("cannot write <path>") when the file won't open or the
// swap fails.
void write_report_file(const std::filesystem::path& outpath, const std::string& html);

}  // namespace hydra::app

#endif  // HYDRA_APP_REPORT_FILES_H
