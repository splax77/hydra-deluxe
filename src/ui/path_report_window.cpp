// The path report's window: the shared report frame (ui/report_window.h)
// filled with the path report's rows, columns (app/path_report_view.h),
// tiles (path_tiles) and its Timing and Best path only controls.

#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "app/path_report_view.h"
#include "imgui.h"
#include "ui/app_state.h"  // path_report_input
#include "ui/report_window.h"

namespace hydra::ui {

namespace {

using app::report::GeneratedReport;
using app::report::ReportRow;
using app::report_view::TableView;
namespace view_rules = app::path_report_view;

// Everything the window keeps between frames. A new result starts it fresh.
struct PathWindow {
    std::weak_ptr<const GeneratedReport> built_from;
    std::unique_ptr<TableView<ReportRow>> view;
    std::vector<view_rules::TimingChoice> choices;
    std::vector<std::string> labels;
    int timing = 0;
    bool best_only = true;
    std::vector<app::report::Tile> tiles;
    bool tiles_dirty = true;
    report_frame::Memory memory;
};

PathWindow& path_window() {
    static PathWindow w;
    return w;
}

// The controls' choices, handed to the view.
void apply_filters(PathWindow& w) {
    w.view->set_search(w.memory.search);
    w.view->set_keep(view_rules::path_keep(w.choices[static_cast<size_t>(w.timing)].tier,
                                           w.best_only));
    w.tiles_dirty = true;
}

void rebuild(PathWindow& w, const std::shared_ptr<const GeneratedReport>& result) {
    w.memory.reset();
    w.view.reset();
    w.timing = 0;
    w.best_only = true;
    w.tiles_dirty = true;
    w.built_from = result;
    if (!result || result->paths.empty()) return;
    std::vector<std::string> texts;
    texts.reserve(result->paths.size());
    for (const ReportRow& r : result->paths) texts.push_back(view_rules::path_search_text(r));
    w.view = std::make_unique<TableView<ReportRow>>(
        result->paths, std::move(texts), view_rules::path_columns(result->hit_window_ms));
    w.view->set_sort({view_rules::path_first_sort()});
    w.choices = view_rules::timing_choices(result->hit_window_ms);
    w.labels.clear();
    for (const view_rules::TimingChoice& c : w.choices) w.labels.push_back(c.label);
    apply_filters(w);
}

// Tiles, controls, count line and table: a Ready report with rows.
void draw_body(PathWindow& w, const report_frame::Frame& frame, const PathReportInput& input) {
    TableView<ReportRow>& view = *w.view;
    if (w.tiles_dirty) {
        w.tiles = app::report::path_tiles(view.rows(), view.visible(),
                                          input.result->hit_window_ms);
        w.tiles_dirty = false;
    }
    report_frame::tiles(w.tiles);

    if (report_frame::search_box(w.memory, "Search song, artist, charter, or path notation"))
        apply_filters(w);
    if (report_frame::dropdown("##timing", w.labels, w.timing)) apply_filters(w);
    ImGui::SameLine();
    if (ImGui::Checkbox("Best path only", &w.best_only)) apply_filters(w);
    report_frame::count_line(w.memory, view.count_line(view_rules::kNoun));

    if (view.visible().empty()) {
        // Clear filters leaves Best path only as it is.
        if (report_frame::nothing_matches()) {
            w.memory.search[0] = '\0';
            w.timing = 0;
            apply_filters(w);
        }
        return;
    }
    report_frame::RowLook<ReportRow> look;
    look.best = [](const ReportRow& r) { return r.optimal; };
    look.chip = [](const ReportRow& r) { return app::report::tier_token(r.tok); };
    report_frame::table("##pathtable", w.memory, view, look, input.callbacks,
                        ImGui::GetContentRegionAvail().y - report_frame::footer_height(frame));
}

}  // namespace

void draw_path_report_window(bool* open, const PathReportInput& input) {
    PathWindow& w = path_window();
    if (!*open) {
        w.memory.was_open = false;
        return;
    }
    if (!report_frame::same_result(w.built_from, input.result)) rebuild(w, input.result);
    const GeneratedReport* result = input.result.get();

    report_frame::Frame frame = report_frame::frame_from(input);
    frame.window_name = "Path report \xE2\x80\x94 Hydra###pathreport";  // U+2014
    frame.heading = "Path Index";
    frame.building_subtitle = "Building the report from your library...";
    frame.progress = std::make_pair(input.progress_done, input.progress_total);
    frame.failure_sentence = "The path report could not be built.";
    if (result) {
        frame.notice = app::report::left_out_line(result->failures);
        for (const app::report::ReportFailure& f : result->failures)
            frame.notice_files.push_back(f.notespath);
        if (!w.view) frame.empty_text = result->why_empty;
    }

    if (report_frame::begin(open, frame, w.memory) && w.view) draw_body(w, frame, input);
    report_frame::end(open, frame, w.memory);
}

PathReportInput path_report_input(AppState& app) {
    const PathReportSlot& slot = app.path_report;
    PathReportInput in = input_from_slot(slot, app.path_report_build());
    if (in.state == ReportBuild::Building && app.report_job)
        std::tie(in.progress_done, in.progress_total) = app.report_job->progress();
    // The row index is into this frame's result, which the click keeps alive.
    in.callbacks.row_click = [&app, result = slot.result](size_t row) {
        const ReportRow& r = result->paths[row];
        app.select_chart(r.hyhash, r.mode);
    };
    in.callbacks.refresh = [&app] { app.request_path_report(); };
    in.callbacks.try_again = [&app] { app.request_path_report(); };
    in.callbacks.cancel = [&app] { app.cancel_path_report(); };
    // The window clears window_open itself through its `open` flag.
    return in;
}

}  // namespace hydra::ui
