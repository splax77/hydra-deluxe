// The main window: settings panel (chart folders, view/search controls,
// scan/analyze buttons) and the paginated library table. Also owns the two
// progress modals those buttons kick off (scan, batch analyze). Mirrors
// hydra_app.py's view_main + build_main_ui's "mainwindowcontent" group plus
// the "scanprogress"/"batchprogress" modal windows.

#ifndef HYDRA_UI_LIBRARY_VIEW_H
#define HYDRA_UI_LIBRARY_VIEW_H

#include "ui/app_state.h"

namespace hydra::ui {

void render_main_window(AppState& app);

// What the window shows while the startup open runs (AppState::store_ready is
// false): nothing until AppState::store_open_shown, then a box centred in the
// window that says what the open is doing (StoreOpenProgress). Same window
// name and flags as the main window, so its saved settings carry over.
void render_startup_screen(AppState& app);

// How wide the progress boxes are, before px(): the scan modal and the
// startup screen. Fixed so live counts never resize them.
inline constexpr float kProgressBoxWidth = 460.0f;

// The library's narrowest and widest widths with the song panel open, in
// `room` px for the two together: the library keeps kMinLibraryW and the
// panel kMinSongPanelW (both in details_view.h, through px()); when the room
// can't hold both, the library keeps its minimum.
struct LibrarySplitBounds {
    float min_w = 0.0f;
    float max_w = 0.0f;
};
LibrarySplitBounds library_split_bounds(float room);

// How wide the library is with the song panel open: `share` of `room`, held
// inside library_split_bounds(room).
float library_split_width(float room, float share);

}  // namespace hydra::ui

#endif  // HYDRA_UI_LIBRARY_VIEW_H
