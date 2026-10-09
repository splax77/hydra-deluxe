// The dmleaderboards comparison's window: the shared report frame
// (ui/report_window.h) filled with the comparison's rows, columns
// (app/dm_report_view.h), tiles (dm_tiles) and its Status control.

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "app/dm_report_view.h"
#include "imgui.h"
#include "ui/app_state.h"  // dm_report_input
#include "ui/report_window.h"

namespace hydra::ui {

namespace {

using app::dm_report::DmReportRow;
using app::dm_report::GeneratedDmReport;
using app::report_view::TableView;
namespace view_rules = app::dm_report_view;

// Everything the window keeps between frames. A new result starts it fresh.
struct DmWindow {
    std::weak_ptr<const GeneratedDmReport> built_from;
    std::unique_ptr<TableView<DmReportRow>> view;
    std::vector<view_rules::StatusChoice> choices;
    std::vector<std::string> labels;
    int status = 0;
    std::vector<app::report::Tile> tiles;
    bool tiles_dirty = true;
    report_frame::Memory memory;
};

DmWindow& dm_window() {
    static DmWindow w;
    return w;
}

// The controls' choices, handed to the view.
void apply_filters(DmWindow& w) {
    w.view->set_search(w.memory.search);
    w.view->set_keep(view_rules::dm_keep(w.choices[static_cast<size_t>(w.status)].status));
    w.tiles_dirty = true;
}

void rebuild(DmWindow& w, const std::shared_ptr<const GeneratedDmReport>& result) {
    w.memory.reset();
    w.view.reset();
    w.status = 0;
    w.tiles_dirty = true;
    w.built_from = result;
    if (!result || result->rows.empty()) return;
    std::vector<std::string> texts;
    texts.reserve(result->rows.size());
    for (const DmReportRow& r : result->rows) texts.push_back(view_rules::dm_search_text(r));
    w.view = std::make_unique<TableView<DmReportRow>>(result->rows, std::move(texts),
                                                      view_rules::dm_columns());
    w.view->set_sort({view_rules::dm_first_sort()});
    w.choices = view_rules::status_choices();
    w.labels.clear();
    for (const view_rules::StatusChoice& c : w.choices) w.labels.push_back(c.label);
    apply_filters(w);
}

// Tiles, controls, count line and table: a Ready comparison with rows.
void draw_body(DmWindow& w, const report_frame::Frame& frame, const DmReportInput& input) {
    TableView<DmReportRow>& view = *w.view;
    if (w.tiles_dirty) {
        w.tiles = app::dm_report::dm_tiles(view.rows(), view.visible());
        w.tiles_dirty = false;
    }
    report_frame::tiles(w.tiles);

    if (report_frame::search_box(w.memory, "Search song, artist, or charter")) apply_filters(w);
    if (report_frame::dropdown("##status", w.labels, w.status)) apply_filters(w);
    report_frame::count_line(w.memory, view.count_line(view_rules::kNoun));

    if (view.visible().empty()) {
        if (report_frame::nothing_matches()) {
            w.memory.search[0] = '\0';
            w.status = 0;
            apply_filters(w);
        }
        return;
    }
    report_frame::RowLook<DmReportRow> look;
    look.chip = [](const DmReportRow& r) { return app::dm_report::status_token(r.status); };
    // A score whose chart the library doesn't list has nothing to select
    // (D103 item 12).
    look.clickable = [](const DmReportRow& r) {
        return r.status != app::dm_report::kStatusNotInLibrary;
    };
    look.unclickable_hint = "Not in your library";
    report_frame::table("##dmtable", w.memory, view, look, input.callbacks,
                        ImGui::GetContentRegionAvail().y - report_frame::footer_height(frame));
}

}  // namespace

void draw_dm_report_window(bool* open, const DmReportInput& input) {
    DmWindow& w = dm_window();
    if (!*open) {
        w.memory.was_open = false;
        return;
    }
    if (!report_frame::same_result(w.built_from, input.result)) rebuild(w, input.result);
    const GeneratedDmReport* result = input.result.get();
    const std::string player =
        !input.player.empty() ? input.player : result ? result->username : std::string();

    report_frame::Frame frame = report_frame::frame_from(input);
    frame.window_name = "dmleaderboards: " + player + " \xE2\x80\x94 Hydra###dmreport";  // U+2014
    frame.heading = "vs dmleaderboards";
    // The picker box's two sentences; the server gives no count.
    frame.building_lines = {"Fetching scores and building the report...",
                            "The leaderboard server can take a moment to wake up."};
    frame.failure_sentence = "Could not build the report.";
    frame.compare_another = true;
    // A player with no scores: the counts as the picker box gave them.
    if (result && !w.view) frame.empty_text = app::dm_report::counts_phrase(result->stats);

    if (report_frame::begin(open, frame, w.memory) && w.view) draw_body(w, frame, input);
    report_frame::end(open, frame, w.memory);
}

DmReportInput dm_report_input(AppState& app) {
    const DmReportSlot& slot = app.dm_report;
    DmReportInput in;
    in.result = slot.result;
    in.state = app.dm_report_build();
    if (slot.result) in.built = slot.built_at;
    in.out_of_date = slot.out_of_date;
    in.batch_finished = slot.batch_finished;
    in.failure_message = slot.message;
    in.failure_error = slot.error;
    in.player = app.dm_player_name();
    // The row index is into this frame's result, which the click keeps alive.
    // A score the library doesn't list never calls it (the window's
    // clickable rule).
    in.callbacks.row_click = [&app, result = slot.result](size_t row) {
        app.select_chart(result->rows[row].identifier, result->chartmode);
    };
    in.callbacks.refresh = [&app] { app.request_dm_report(); };
    in.callbacks.try_again = [&app] { app.request_dm_report(); };
    // Cancel while fetching closes the window and goes back to the player
    // list (D103 item 10).
    in.callbacks.cancel = [&app] {
        app.cancel_dm_report();
        app.dm_report.window_open = false;
        app.reopen_dm_picker();
    };
    in.callbacks.compare_another = [&app] { app.reopen_dm_picker(); };
    // The window clears window_open itself through its `open` flag.
    return in;
}

}  // namespace hydra::ui
