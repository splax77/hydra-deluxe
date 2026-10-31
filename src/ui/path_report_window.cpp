// The path report's window: the shared report frame (ui/report_window.h)
// filled with the path report's rows, columns (app/path_report_view.h),
// tiles (path_tiles) and its Timing and Optimal paths only controls.

#include <algorithm>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "app/analysis.h"        // kBatchPreparingText
#include "app/display_format.h"  // kFactSeparator
#include "app/html_page.h"       // search_hint
#include "app/path_report_view.h"
#include "app/user_messages.h"
#include "imgui.h"
#include "ui/app_state.h"  // path_report_input
#include "ui/report_window.h"

namespace hydra::ui {

namespace {

using app::report::GeneratedReport;
using app::report::ReportRow;
using app::report_view::TableView;
namespace view_rules = app::path_report_view;

// Everything the window keeps between frames. A new result starts it fresh,
// except one built because the window opened (PathReportInput::keep_filters).
struct PathWindow {
    std::weak_ptr<const GeneratedReport> built_from;
    std::unique_ptr<TableView<ReportRow>> view;
    // The view's rows measured for the column-width rule; goes with them.
    MeasuredWidths widths;
    std::vector<view_rules::TimingChoice> choices;
    std::vector<std::string> labels;
    int timing = app::report_view::kEveryRowChoice;
    bool best_only = true;
    // The view's sort, kept while the window has no rows.
    std::vector<app::report_view::SortSpec> sort;
    std::vector<app::report::Tile> tiles;
    bool tiles_dirty = true;
    // The can't-analyze line's Show list is expanded.
    bool list_open = false;
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

// Lets the window's view of the rows go, and the cells fitted from them,
// keeping the sort they were in.
void drop_rows(PathWindow& w) {
    if (w.view) w.sort = w.view->sort();
    w.view.reset();
    w.widths = {};
    w.memory.fitted.clear();
    w.tiles.clear();
}

// Takes up a new result. `keep`: the search, timing, Optimal paths only and sort
// stay as the user left them; otherwise they start over.
void rebuild(PathWindow& w, const std::shared_ptr<const GeneratedReport>& result, bool keep) {
    drop_rows(w);
    const report_frame::Memory kept = w.memory;
    w.memory.reset();
    if (keep) {
        std::copy(std::begin(kept.search), std::end(kept.search), std::begin(w.memory.search));
    } else {
        w.timing = app::report_view::kEveryRowChoice;
        w.best_only = true;
        w.sort.clear();
    }
    w.tiles_dirty = true;
    w.built_from = result;
    if (!result || result->paths.empty()) return;
    std::vector<std::string> texts;
    texts.reserve(result->paths.size());
    for (const ReportRow& r : result->paths) texts.push_back(view_rules::path_search_text(r));
    w.view = std::make_unique<TableView<ReportRow>>(
        app::report_view::shared_rows(result, &GeneratedReport::paths), std::move(texts),
        view_rules::path_columns());
    w.widths = report_frame::measure(*w.view);
    // A kept sort naming a column these rows lack falls back to the first.
    const auto has_column = [&w](const app::report_view::SortSpec& s) {
        const auto& cols = w.view->columns();
        return std::any_of(cols.begin(), cols.end(), [&s](const auto& c) { return c.id == s.column; });
    };
    if (w.sort.empty() || !std::all_of(w.sort.begin(), w.sort.end(), has_column))
        w.sort = {view_rules::path_first_sort()};
    w.view->set_sort(w.sort);
    // The timing kept is an index into the choices. Choices whose labels
    // differ from the kept ones would make it mean another tier, so it
    // starts over.
    w.choices = view_rules::timing_choices();
    std::vector<std::string> labels;
    for (const view_rules::TimingChoice& c : w.choices) labels.push_back(c.label);
    if (labels != w.labels) w.timing = app::report_view::kEveryRowChoice;
    w.labels = std::move(labels);
    apply_filters(w);
}

// The coverage lines (D249 item 5): the can't-analyze line
// with its Show list, then the no-result line. The words are the report's
// own (GeneratedReport); the window only draws them.
void draw_coverage(PathWindow& w, const GeneratedReport& r) {
    if (!r.cant_analyze_line.empty()) {
        ImGui::TextUnformatted(r.cant_analyze_line.c_str());
        if (!r.cant_analyze.empty()) {
            ImGui::SameLine();
            if (ImGui::TextLink(w.list_open ? "Hide list##cantanalyze" : "Show list##cantanalyze"))
                w.list_open = !w.list_open;
        }
        if (w.list_open && !r.cant_analyze.empty()) {
            // Scrolls on its own past half the room left, so a long list
            // leaves the table room to show.
            ImGui::SetNextWindowSizeConstraints(
                ImVec2(0.0f, 0.0f), ImVec2(FLT_MAX, ImGui::GetContentRegionAvail().y * 0.5f));
            ImGui::BeginChild("##cantanalyzelist", ImVec2(0.0f, 0.0f),
                              ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);
            ImGui::PushTextWrapPos(0.0f);
            for (const app::report::CantAnalyzeGroup& group : r.cant_analyze) {
                if (!group.sentence.empty())
                    ImGui::TextColored(kDimTextColor, "%s", group.sentence.c_str());
                for (const app::report::CantAnalyzeChart& c : group.charts) {
                    const std::string line =
                        c.artist.empty() ? c.title : c.title + app::kFactSeparator + c.artist;
                    ImGui::TextWrapped("%s", line.c_str());
                }
            }
            ImGui::PopTextWrapPos();
            ImGui::EndChild();
        }
    }
    if (!r.no_result_line.empty()) {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextUnformatted(r.no_result_line.c_str());
        ImGui::PopTextWrapPos();
    }
}

// Tiles, coverage lines, controls, count line and table: a Ready report with
// rows.
void draw_body(PathWindow& w, const report_frame::Frame& frame, const PathReportInput& input) {
    TableView<ReportRow>& view = *w.view;
    if (w.tiles_dirty) {
        w.tiles = app::report::path_tiles(view.rows(), view.visible(),
                                          input.result->library_charts);
        w.tiles_dirty = false;
    }
    report_frame::tiles(w.tiles);
    draw_coverage(w, *input.result);

    // What path_search_text folds for each row (app/path_report_view.cpp).
    static const std::string search_hint =
        app::html::search_hint({"song", "artist", "charter", "path notation"});
    if (report_frame::search_box(w.memory, search_hint.c_str())) apply_filters(w);
    if (report_frame::dropdown("##timing", w.labels, w.timing)) apply_filters(w);
    ImGui::SameLine();
    if (ImGui::Checkbox("Optimal paths only", &w.best_only)) apply_filters(w);
    report_frame::count_line(w.memory, view.count_line(view_rules::kNoun));

    if (view.visible().empty()) {
        // Clear filters leaves Optimal paths only as it is.
        if (report_frame::nothing_matches()) {
            w.memory.clear_search();
            w.timing = app::report_view::kEveryRowChoice;
            apply_filters(w);
        }
        return;
    }
    report_frame::RowLook<ReportRow> look;
    look.chip = [](const ReportRow& r) { return app::report::tier_token(r.tok); };
    report_frame::table("##pathtable", w.memory, view, w.widths, look, input.callbacks,
                        ImGui::GetContentRegionAvail().y - report_frame::footer_height(frame));
}

}  // namespace

void draw_path_report_window(bool* open, const PathReportInput& input) {
    PathWindow& w = path_window();
    if (!*open) {
        w.memory.was_open = false;
        // The view keeps its whole result alive (shared_rows), so a result
        // that replaced it while hidden lets it go now, not at the reopen.
        if (!report_frame::same_result(w.built_from, input.result)) drop_rows(w);
        return;
    }
    if (!report_frame::same_result(w.built_from, input.result))
        rebuild(w, input.result, input.keep_filters);
    const GeneratedReport* result = input.result.get();

    report_frame::Frame frame = report_frame::frame_from(input);
    frame.window_name = "Path report \xE2\x80\x94 Hydra###pathreport";  // U+2014
    frame.heading = "Path Index";
    frame.building_subtitle = "Building the report from your library...";
    if (input.batch) {
        // The batch the build waits on: the strip's words while it prepares,
        // then its own count over a bar it fills (D249
        // item 8). Both numbers are the snapshot's, as the strip shows them.
        const PathReportInput::WaitedBatch& b = *input.batch;
        frame.building_lines = {b.preparing ? std::string(app::kBatchPreparingText)
                                            : "Analyzing " + group_thousands(b.completed) +
                                                  " of " + counted(b.total, "chart", "charts")};
        frame.progress = std::make_pair(b.completed, b.total);
    } else {
        // The build itself only reads the store, so it has no count to show
        // and draws no bar.
        frame.progress_bar = false;
        // The window asks whether to analyze, in the batch confirm's words;
        // its Cancel is the "no" (AppState::cancel_path_report).
        if (!input.confirm_question.empty()) {
            frame.building_lines = {input.confirm_question};
            frame.analyze_button = true;
            frame.analyze_enabled = input.can_analyze;
        }
    }
    frame.placeholder_rows = true;
    frame.analysis_off = input.analysis_off;
    frame.failure_sentence = app::kPathReportFailed;
    if (result && !w.view) frame.empty_text = result->why_empty;

    const bool body = report_frame::begin(open, frame, w.memory);
    if (body && w.view) {
        draw_body(w, frame, input);
    } else if (w.memory.visible && input.state == ReportBuild::Ready && result && !w.view) {
        // No table rows: the lines still say what became of the library.
        draw_coverage(w, *result);
    }
    report_frame::end(open, frame, w.memory);
}

PathReportInput path_report_input(AppState& app) {
    const PathReportSlot& slot = app.path_report;
    PathReportInput in = input_from_slot(slot, app.path_report_build());
    in.analysis_off = app.analysis_off_lines();
    in.keep_filters = slot.built_on_open;
    if (const BatchJob* batch = app.path_report_batch()) {
        const BatchJob::Snapshot s = batch->snapshot();
        in.batch = PathReportInput::WaitedBatch{s.preparing, s.completed, s.total};
    }
    // The report's own plan, never redone: the popup's question with its
    // redo box clear, and the batch start_batch(false) runs.
    if (app.path_report_awaits_confirm()) {
        in.confirm_question = app.batch_question(false);
        in.can_analyze = app.can_analyze();
    }
    // The row index is into this frame's result, which the click keeps alive.
    // The page lists one profile, so a click selects the chart and never
    // switches the chart mode: it asks in the mode the settings already hold.
    in.callbacks.row_click = [&app, result = slot.result](size_t row) {
        app.select_chart(result->paths[row].hyhash, app.settings.chartmode_key());
    };
    in.callbacks.refresh = [&app] { app.request_path_report(); };
    in.callbacks.try_again = [&app] { app.request_path_report(); };
    in.callbacks.cancel = [&app] { app.cancel_path_report(); };
    in.callbacks.analyze = [&app] { app.start_path_report_batch(); };
    // The window clears window_open itself through its `open` flag; the
    // close then goes through its one owner, which ends a question the
    // window was asking.
    in.callbacks.close = [&app] { app.close_path_report(); };
    return in;
}

}  // namespace hydra::ui
