// How a chart reaches the Path Builder from outside: File > Open's dialog and
// a file dragged onto the window. The command line is main.cpp's.

#ifndef HYDRA_PATHBUILDER_PB_OPEN_H
#define HYDRA_PATHBUILDER_PB_OPEN_H

#include <optional>
#include <string>
#include <vector>

#include "ui/win32_dialogs.h"

namespace hydra::pathbuilder {

// The type list File > Open shows: the chart files Hydra reads (the extensions
// chart_extensions in parse/chart_files.h lists), then every file.
std::vector<ui::FileTypeFilter> chart_file_types();

// Shows File > Open's dialog over `owner`. The picked file, or nullopt when
// the player cancelled; a dialog that never appeared also sets *failed.
std::optional<std::string> ask_for_chart(HWND owner, bool* failed);

// The first file of a WM_DROPFILES drop (its wParam), UTF-8, and lets the
// drop go (DragFinish). Nullopt when the drop names no file.
std::optional<std::string> take_dropped_file(void* hdrop);

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_OPEN_H
