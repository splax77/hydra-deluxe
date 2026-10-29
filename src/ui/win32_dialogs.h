// Small Win32 dialog helpers Dear ImGui has no native equivalent for.

#ifndef HYDRA_UI_WIN32_DIALOGS_H
#define HYDRA_UI_WIN32_DIALOGS_H

#include <optional>
#include <string>
#include <vector>

struct HWND__;
typedef HWND__* HWND;

namespace hydra::ui {

// The modern folder-picker (IFileOpenDialog + FOS_PICKFOLDERS), mirroring
// DearPyGui's directory_selector file dialog. Returns nullopt if the user
// cancels — and also on COM/dialog failure, which additionally sets *failed
// so the caller can tell the user (a cancel must stay silent, but a dialog
// that never appeared shouldn't). UTF-8 in, UTF-8 out.
std::optional<std::string> browse_for_folder(HWND owner, bool* failed = nullptr);

// One line of a file dialog's type list: the name the user reads and the
// patterns it shows, separated by ';' ("Charts", "*.mid;*.chart"). UTF-8.
struct FileTypeFilter {
    std::string name;
    std::string patterns;
};

// The same dialog picking one existing file, its type list `types` (the first
// is selected). Cancel and failure answer as browse_for_folder's do.
std::optional<std::string> browse_for_file(HWND owner, const std::vector<FileTypeFilter>& types,
                                           bool* failed = nullptr);

}  // namespace hydra::ui

#endif  // HYDRA_UI_WIN32_DIALOGS_H
