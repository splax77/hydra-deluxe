#include "ui/details_view.h"
#include "ui/details_parts.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "app/display_format.h"  // kFactSeparator
#include "app/dynamics_breakdown.h"  // dynamics_kick2x_fact, fact_separator_dot
#include "app/path_view.h"
#include "app/user_messages.h"  // stale_text, kNoPathsFound
#include "core/model.h"
#include "imgui.h"
#include "parse/song.h"  // display_title, display_artist, display_charter
#include "ui/app_shell.h"  // library_hidden
#include "ui/fonts.h"
#include "ui/generation.h"
#include "ui/preview_controller.h"
#include "ui/text_role.h"
#include "ui/theme.h"
#include "ui/widgets.h"

#include <algorithm>
#include <cstdio>
#include <optional>
#include <string>

namespace hydra::ui {

namespace {

// Title, "artist · charted by charter", and hide library / previous / next /
// close at the right. Clone Hero rich-text tags (<color=...>) are stripped for
// display; the title is display_title's, the artist display_artist's and the
// charter display_charter's, so a title or a missing artist reads "(unknown)".
void render_panel_header(AppState& app) {
    const ImGuiStyle& style = ImGui::GetStyle();
    const float button = ImGui::GetFrameHeight();
    const float library_w = std::max(button_slot_width("Hide library"),
                                     button_slot_width("Show library"));
    const float buttons_w = library_w + button * 3.0f + style.ItemSpacing.x * 3.0f;
    const float left_x = ImGui::GetCursorPosX();
    const float top_y = ImGui::GetCursorPosY();
    const float text_w = ImGui::GetContentRegionAvail().x - buttons_w - style.ItemSpacing.x;

    // The buttons first, at the right edge, so the title can take the rest.
    ImGui::SetCursorPosX(left_x + text_w + style.ItemSpacing.x);
    const bool hidden = library_hidden();
    if (ImGui::Button(hidden ? "Show library" : "Hide library", ImVec2(library_w, button)))
        remember_library_hidden(!hidden);
    hint(hidden ? "Put the library back beside this song"
                : "Give this song the whole window. < and > still step through the list.");
    ImGui::SameLine();
    const bool can_prev = app.can_select_relative(-1);
    begin_disabled_button(!can_prev);
    if (ImGui::Button("<##prevsong", ImVec2(button, button))) app.select_relative(-1);
    end_disabled_button(!can_prev);
    hint("Previous song in the list");
    ImGui::SameLine();
    const bool can_next = app.can_select_relative(1);
    begin_disabled_button(!can_next);
    if (ImGui::Button(">##nextsong", ImVec2(button, button))) app.select_relative(1);
    end_disabled_button(!can_next);
    hint("Next song in the list");
    ImGui::SameLine();
    if (ImGui::Button("X##closepanel", ImVec2(button, button))) app.show_details = false;
    hint("Close (Esc)");

    ImGui::SetCursorPos(ImVec2(left_x, top_y));
    const store::ChartLibraryEntry& song = *app.selected;
    {
        ScopedFont font(TextRole::SongTitle);
        text_ellipsized(display_title(song.title).c_str(), text_w);
    }
    std::string byline = display_artist(song.artist);
    const std::string charter = display_charter(song.charter);
    if (!charter.empty()) byline += app::kFactSeparator + ("charted by " + charter);
    ImGui::PushStyleColor(ImGuiCol_Text, kSubtleTextColor);
    text_ellipsized(byline.c_str(), text_w);
    ImGui::PopStyleColor();
}

// The one line that stands in for a Ready record's content when it has no
// paths: kNoPathsFound. Returns true only when the record is ready to draw.
// Every other state draws nothing here: the progress box, the cancel line,
// the error and the notices say why (render_record_state,
// render_panel_notices). The headline and render_record_state both read it,
// so the states read the same everywhere.
bool render_state_line(AppState& app) {
    if (!app.viewed.ready()) return false;
    if (app.viewed.record->paths.empty()) {
        ImGui::TextUnformatted(app::kNoPathsFound);
        return false;
    }
    return true;
}

// The optimal score and path in gold with one line of facts under it. Stars
// are the summary row's (T7), never worked out again here. A held row
// (detail::draw_held_row).
void render_headline(AppState& app) {
    const float left_x = ImGui::GetCursorPosX();
    const float text_w = ImGui::GetContentRegionAvail().x;

    detail::draw_held_row(app, app.details_ui.headline, [&] {
        if (!render_state_line(app)) return;
        const Path& best = app.viewed.record->best_path();
        // render_state_line returned true, so the record has paths and the
        // optimal score has a value.
        const int64_t optimal = *app.viewed.record->optimal_score();
        ImGui::PushStyleColor(ImGuiCol_Text, kBestPathColor);
        {
            ScopedFont font(TextRole::HeadlineScore);
            ImGui::TextUnformatted(group_thousands(optimal).c_str());
        }
        ImGui::SameLine(0.0f, px(18.0f));
        {
            // SameLine starts the path at the line's top; drop it so its
            // baseline sits on the score's.
            const float shift = baseline_shift(TextRole::HeadlinePath, TextRole::HeadlineScore);
            ScopedFont font(TextRole::HeadlinePath);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + shift);
            text_ellipsized(best.pathstring().c_str(), left_x + text_w - ImGui::GetCursorPosX());
        }
        ImGui::PopStyleColor();

        // Each path's hardest timing sits beside it in the path list, so the
        // headline doesn't repeat the optimal one's.
        const store::PathSummary& summary = app.viewed.summary;
        std::string facts = "Optimal path";
        if (summary.stars) facts += app::kFactSeparator + counted(*summary.stars, "star", "stars");
        const std::string kick2x =
            app.viewed.dynamics ? app::dynamics_kick2x_fact(*app.viewed.dynamics) : std::string();
        // The facts in the subtle grey, then the 2x kick fact in the score's
        // gold, drawn as parts one space apart (app::TwoPartLine). Each part
        // is cut to the room left; a part that had to be cut ends the line.
        const float right_x = left_x + text_w;
        auto part = [&](const std::string& text) {
            const float room = right_x - ImGui::GetCursorPosX();
            text_ellipsized(text.c_str(), room);
            return text_width(text) <= room;
        };
        ImGui::PushStyleColor(ImGuiCol_Text, kSubtleTextColor);
        bool whole = part(facts);
        if (!kick2x.empty() && whole) {
            same_line_one_space();
            whole = part(app::fact_separator_dot());
        }
        ImGui::PopStyleColor();
        if (!kick2x.empty() && whole) {
            same_line_one_space();
            ImGui::PushStyleColor(ImGuiCol_Text, kBestPathColor);
            part(kick2x);
            ImGui::PopStyleColor();
        }
    });
}

// The lines that say why analysis can't run, each with its remedy.
void render_panel_notices(AppState& app) {
    // The error is a warning line with a remedy, not the button's label.
    if (!app.selected_file_ok(ImGui::GetTime())) {
        ImGui::TextColored(kWarningColor, "Song file not found.");
        ImGui::SameLine();
        if (ImGui::SmallButton("Rescan library")) {
            app.request_scan = true;   // the main window starts the scan
            app.show_details = false;  // tick() runs close_details() on the edge
        }
    }
    // Without the file's error: the toolbar above already shows it (D114
    // item 13).
    if (app.analysis_blocked()) {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextColored(kWarningColor, "%s", app::kAnalysisOffSentence);
        ImGui::PopTextWrapPos();
    }
}

// The click's progress box, its cancel line and its error. Display only;
// AppState::tick() owns the job. The box shows only once the job has run
// kViewProgressDelaySeconds, and there's no "Done!" (D87 items 6 and 10).
void render_view_progress(AppState& app) {
    if (!app.view_progress_shown()) return;
    // The latest request's own job; none while it waits for an older,
    // cancelled one to exit (AppState::view_pending).
    const ViewJob* job = app.view_pending ? nullptr : app.view_job.get();
    ImGui::BeginChild("analyzeprogress", ImVec2(0, px(140)), ImGuiChildFlags_Borders);
    if (job && job->is_cancelled()) {
        ImGui::TextUnformatted("Cancelling...");
    } else {
        double t = ImGui::GetTime();
        int dots = (int)(t * 2) % 4;
        ImGui::Text("Analyzing chart%.*s", dots, "...");
        // A real bar once the search starts reporting; until the first tick
        // (parse + graph build) there's nothing to show, so leave it off.
        const float f = job ? job->progress() : -1.0f;
        if (f >= 0.0f) progress_bar_percent(f);
        // A long chart can take a while; the user needs an out that isn't
        // killing the app.
        if (ImGui::Button("Cancel")) app.cancel_view();
    }
    ImGui::EndChild();
}

}  // namespace

namespace detail {

// The states a record-backed tab shows before its own content: the click's
// progress, its cancel line, its error and "No paths found.". Returns true
// only when the record is ready to draw. The Paths and Stars tabs both call
// this, so the states read the same on each.
bool render_record_state(AppState& app) {
    switch (app.viewed.state) {
        case ViewedSong::State::Analyzing:
            render_view_progress(app);
            return false;
        case ViewedSong::State::Cancelled:
            ImGui::TextUnformatted("Analysis cancelled.");
            ImGui::SameLine();
            if (ImGui::SmallButton("Try again")) app.start_view();
            return false;
        case ViewedSong::State::Failed:
            ImGui::PushStyleColor(ImGuiCol_Text, kWarningColor);
            ImGui::TextWrapped("%s", app.viewed.message.c_str());
            ImGui::PopStyleColor();
            error_detail_line(app.viewed.error);
            if (ImGui::Button("Continue")) app.dismiss_view_error();
            return false;
        default:
            return render_state_line(app);
    }
}

void draw_held_row(AppState& app, HeldRow& row, const std::function<void()>& draw) {
    if (!draw) {
        if (const std::optional<float> held = row.keep(app.view_holds_space()))
            ImGui::Dummy(ImVec2(0.0f, *held));
        return;
    }
    const float top_y = ImGui::GetCursorPosY();
    ImGui::BeginGroup();
    draw();
    // Whether anything was drawn: the cursor moved down past it.
    const bool drew = ImGui::GetCursorPosY() != top_y;
    if (!drew) draw_held_row(app, row);
    ImGui::EndGroup();
    if (drew) row.height = ImGui::GetItemRectSize().y;
}

}  // namespace detail

void render_song_panel(AppState& app) {
    if (!app.selected) return;

    // All of this panel's own state lives on AppState (DetailsViewState).
    const Path*& selected_path = app.details_ui.selected_path;
    GenerationWatcher& record_watcher = app.details_ui.record_watcher;

    // selected_path points into app.viewed's record, so it is only valid for
    // the record generation it was chosen under. Re-sync wherever the record
    // may have changed: at the top of the frame, and after the header, whose
    // previous / next buttons swap the song in the middle of this frame.
    auto sync_selected_path = [&] {
        if (record_watcher.changed(app.record_generation)) {
            selected_path = app.viewed.ready() && !app.viewed.record->paths.empty()
                                ? &app.viewed.record->best_path()
                                : nullptr;
        }
    };
    sync_selected_path();

    // Escape closes the panel like its X -- but not while a popup (a
    // confirm, an open combo) or a text box has the key.
    if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel) &&
        !ImGui::GetIO().WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Escape, false))
        app.show_details = false;

    // Ctrl+C copies the selected path -- unless a text input has focus. Same
    // call as the Copy path button, so it flashes "Copied!" too.
    if (selected_path && !ImGui::GetIO().WantTextInput &&
        ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_C))
        detail::copy_selected_path(app);

    render_panel_header(app);
    if (!app.selected) return;
    sync_selected_path();
    ImGui::Spacing();
    render_headline(app);
    render_panel_notices(app);
    ImGui::Spacing();

    // The tabs, as before. The Preview only renders while its tab is the
    // active one; leaving it pauses playback.
    if (ImGui::BeginTabBar("##DetailsTabs")) {
        if (ImGui::BeginTabItem("Paths")) {
            if (detail::render_record_state(app))
                detail::render_path_panel(app, selected_path);
            ImGui::EndTabItem();
        }

        // "Show in Preview >" on an activation row asks for the Preview tab;
        // the tab consumes the jump once it draws.
        const ImGuiTabItemFlags preview_flags =
            app.details_ui.paths_tab.ui().preview_jump ? ImGuiTabItemFlags_SetSelected : 0;
        bool preview_shown = false;
        if (ImGui::BeginTabItem("Preview", nullptr, preview_flags)) {
            preview_shown = true;
            detail::render_preview_panel(app, selected_path);
            ImGui::EndTabItem();
        }
        if (!preview_shown && app.preview) app.preview->pause();

        if (ImGui::BeginTabItem("Dynamics")) {
            detail::render_dynamics_panel(app);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Stars")) {
            detail::render_stars_panel(app);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

}  // namespace hydra::ui
