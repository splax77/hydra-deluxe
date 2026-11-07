// Starting another program: Hydra's own sibling exes (the Path Builder) and
// the browser that opens a long report page (report_files.cpp). The one place
// a command line is quoted and the one place a program is started with
// CreateProcessW.

#ifndef HYDRA_APP_LAUNCH_H
#define HYDRA_APP_LAUNCH_H

#include <optional>
#include <string>
#include <vector>

namespace hydra::app {

// The Path Builder's file name. The installer puts it beside Hydra.exe, and
// its Start Menu shortcut points at it (installer/hydra.iss).
inline constexpr const char* kPathBuilderExe = "HydraPathBuilder.exe";

// One argument written so the program's own split of its command line
// (CommandLineToArgvW, split_command_line_utf8 in core/winstr.h) reads it back
// whole: always in double quotes, with the backslashes and quotes inside it
// escaped by that split's rules.
std::wstring quote_argument(const std::wstring& arg);

// A whole command line: the program's path, then each argument, each quoted
// by quote_argument and one space apart.
std::wstring command_line(const std::wstring& exe, const std::vector<std::wstring>& args);

// Starts `exe` with `args` and returns at once, without waiting for it.
// Throws std::runtime_error carrying Windows' reason when it cannot start;
// a screen shows it through plain_error_line (app/user_messages.h).
void launch_program(const std::wstring& exe, const std::vector<std::wstring>& args);

// The full path (UTF-8) of the program called `name` in Hydra's own folder
// (exe_dir), or nullopt when no such file is there. The answer for Hydra's
// folder is read once per name and kept, because the folder's contents don't
// change while Hydra runs.
std::optional<std::string> sibling_exe_path(const std::string& name);

// Test seam: a folder sibling_exe_path looks in instead of Hydra's own, read
// afresh on every call; empty puts Hydra's folder back. Tests set it through
// ScopedHook.
void set_sibling_dir_for_test(std::optional<std::string> dir);

// Starts the sibling program `name` with `paths` (UTF-8 file paths, each
// handed over in win32_path's form) as its arguments. Throws like
// launch_program, and also when sibling_exe_path finds no such file.
void launch_sibling(const std::string& name, const std::vector<std::string>& paths);

}  // namespace hydra::app

#endif  // HYDRA_APP_LAUNCH_H
