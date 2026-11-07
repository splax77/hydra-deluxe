// The song panel's pieces, shared between the files that draw it. Only the
// details files include this; everything else uses details_view.h.
//
// details_panel.cpp  the panel itself: its header, the headline and analyze
//                    button, the analyze progress, and the record states the
//                    tabs share (the analyze job's lifecycle is AppState::tick)
// paths_tab.cpp      the Paths tab: the path list and a path's details
// preview_tab.cpp    the Preview tab: opens the controller and draws the panel
//                    (the panel itself is ui/preview_panel.h)
// dynamics_tab.cpp   the Dynamics tab
// stars_tab.cpp      the Stars tab

#ifndef HYDRA_UI_DETAILS_PARTS_H
#define HYDRA_UI_DETAILS_PARTS_H

#include <exception>
#include <functional>
#include <string>

#include "app/dynamics_breakdown.h"
#include "ui/app_state.h"

namespace hydra::ui::detail {

// details_panel.cpp. The states a record-backed tab shows before its own
// content (the click's progress, cancelled, error, no paths). True only when
// the record is ready to draw.
bool render_record_state(AppState& app);

// details_panel.cpp. One frame of a HeldRow, a row that keeps its place while
// the panel holds (AppState::view_holds_space). `draw` draws the row as one
// group. When it draws something, the row remembers the group's height; when
// it draws nothing, the row fills its kept height (HeldRow::keep) with empty
// space instead. A row that knows it has nothing to draw passes no `draw`:
// then no group is opened, so it takes no room at all unless held.
void draw_held_row(AppState& app, HeldRow& row, const std::function<void()>& draw = {});

// details_panel.cpp. The song panel header's button that opens the song in
// the Path Builder, and what it says when hovered: kPathBuilderHint while the
// builder can start, path_builder_missing_hint() when its exe
// (app::sibling_exe_path) is not there and the button is greyed.
inline constexpr const char* kOpenInPathBuilderLabel = "Open in Path Builder";
inline constexpr const char* kPathBuilderHint =
    "Build your own Star Power path for this song and see what each choice costs";
std::string path_builder_missing_hint();

// details_panel.cpp. The status line's problem when the Path Builder will not
// start: the sentence, then the failure's one line (app::plain_error_line).
std::string path_builder_failed_text(const std::exception& e);

// paths_tab.cpp. The path list on the left and the selected path's details
// on the right. A click in the list changes `selected_path`.
void render_path_panel(AppState& app, const Path*& selected_path);

// Copy the selected path's verbose string (Path::pathstring_verbose) and
// start the "Copied!" flash. The Copy path button and Ctrl+C both call it.
void copy_selected_path(AppState& app);

// preview_tab.cpp. The Preview for `selected_path`: it opens the controller,
// adds Hydra's own rows and draws ui/preview_panel.h's panel.
void render_preview_panel(AppState& app, const Path* selected_path);

// dynamics_tab.cpp.
void render_dynamics_panel(AppState& app);

// The Chart section's "Dynamics enabled" line for a stored breakdown.
std::string dynamics_enabled_text(const app::DynamicsBreakdown& bd);

// stars_tab.cpp.
void render_stars_panel(AppState& app);

}  // namespace hydra::ui::detail

#endif  // HYDRA_UI_DETAILS_PARTS_H
