// The one way to put a finished report page on disk and hand it to the
// browser. Only the fill comparison writes a page now (hydra_fillcompare,
// src/cli/fillcompare.cpp). The browser call sits behind a settable seam so a
// test can record opens instead of performing them. Every launch through the
// Windows shell lives here, so start_program (the updater's installer start)
// sits beside open_in_browser.

#ifndef HYDRA_APP_REPORT_FILES_H
#define HYDRA_APP_REPORT_FILES_H

#include <filesystem>
#include <functional>
#include <memory>
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

// A program start_program started. Closes its process handle when the last
// copy goes.
class StartedProgram {
public:
    explicit StartedProgram(void* process) : process_(process) {}
    ~StartedProgram();
    StartedProgram(const StartedProgram&) = delete;
    StartedProgram& operator=(const StartedProgram&) = delete;
    // Whether its process has ended. False when Windows handed back no
    // process to watch.
    bool ended() const;

private:
    void* process_;  // a Windows HANDLE; void* so this header needs no windows.h
};

// Starts the program at `exe` with `params` through the shell's "open" verb,
// so a program that asks for administrator rights raises its own UAC prompt.
// The path goes through the same shell_path guard open_in_browser uses.
// Throws std::runtime_error with Windows' reason when the program doesn't
// start.
std::shared_ptr<StartedProgram> start_program(const std::wstring& exe, const std::wstring& params);

// Copies a report page to %TEMP%\Hydra\<its name>, overwriting an older copy,
// and returns the copy's path; empty when the copy fails or its path is one
// fits_shell refuses. A report is one self-contained file, so the copy
// shows the same page.
std::filesystem::path copy_to_short_temp(const std::filesystem::path& page);

// How write_file_replacing lays down line ends. The caller picks, because
// the two files it writes differ (D114 item 28).
enum class LineEnds {
    AsGiven,  // the bytes exactly as passed: the report page
    Crlf,     // a Windows text file, each "\n" written as "\r\n": the settings file
};

// Writes `bytes` to a "<path>.tmp" sibling first and then renames it over
// the target, so a reader never sees a half-written file: the previous one
// stays whole right up until the new one is complete. False when the temp
// file won't open or write, or the swap fails; the temp is removed then.
// The one write-then-rename helper (D114 item 20): the report page and the
// settings file both save through it, each with its own LineEnds.
bool write_file_replacing(const std::filesystem::path& outpath, const std::string& bytes,
                          LineEnds ends);

// Writes a built page to disk through write_file_replacing, bytes as given.
// Throws std::runtime_error("cannot write <path>") when it fails.
void write_report_file(const std::filesystem::path& outpath, const std::string& html);

}  // namespace hydra::app

#endif  // HYDRA_APP_REPORT_FILES_H
