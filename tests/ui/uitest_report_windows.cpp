// The report windows' GUI tests. Those in register_report_window_tests
// open the path report or the comparison window on the sample rows
// (tests/report_samples.h) through ReportWindowInput, the struct AppState
// fills in the app, and drive the window by its labels. Their own GUI
// function draws the windows. Among them are the
// column widths and the window's fit to the screen, from the one column-width
// rule plan,
// docs/superpowers/plans/2026-10-10-one-column-width-rule.md (removed; git show 444631e6:docs/superpowers/plans/2026-10-10-one-column-width-rule.md),
// and the cells' and header's alignment, from the column-shares plan,
// docs/superpowers/plans/2026-10-14-column-shares-scroll-left-align.md (removed; git show 444631e6:docs/superpowers/plans/2026-10-14-column-shares-scroll-left-align.md).
// Those in report_window_flow_tests
// run end to end: the app draws both windows itself from run_frame, so they
// have no GUI function. Under hydra_uitest
// there is no platform backend, so each report window draws inside the main
// viewport, over the main window's rectangle (D103 item 14): close it before
// clicking the toolbar or the finished strip.

#include "uitest_harness.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../report_samples.h"  // tests/ is not on the runner's include path
#include "../scoped_hook.h"
#include "../wait_util.h"
#include "imgui_internal.h"
#include "app/dm_report.h"
#include "app/html_page.h"  // kNothingMatches
#include "app/path_report_view.h"  // timing_choices
#include "app/report.h"
#include "app/user_messages.h"  // kPathReportFailed, kComparisonFailed, kLeaderboardWaking
#include "core/error_kind.h"
#include "core/model.h"  // kCloneHeroSpCap
#include "net/dmbot_client.h"
#include "parse/song.h"  // NoNotesError
#include "store/record_store.h"  // failure_record
#include "ui/library_jobs.h"  // set_report_hold_for_test
#include "ui/report_window.h"
#include "ui/theme.h"  // chip_color

namespace uitest {

// Defined in uitest_batch_reports.cpp.
bool batch_search(ImGuiTestContext* ctx, const std::string& search);
void dismiss_done(ImGuiTestContext* ctx);
void start_library_batch(ImGuiTestContext* ctx);

namespace {

using hydra::app::dm_report::GeneratedDmReport;
using hydra::app::report::GeneratedReport;
using hydra::ui::DmReportInput;
using hydra::ui::PathReportInput;
using hydra::ui::ReportOutOfDate;
using hydra::ui::ReportBuild;

// What one chip drew, read from the vertices it added to the draw list in its
// token's colour, beside the width of its label as ImGui measures it.
struct ChipDrawn {
    hydra::app::report::ChipToken token = hydra::app::report::ChipToken::t0;
    int vertices = 0;   // in the token's colour
    float left = 0.0f;  // the leftmost and rightmost of them
    float right = 0.0f;
    float label_width = 0.0f;
};

// What the windows were handed, and what they handed back.
struct Fixture {
    bool path_open = false;
    bool dm_open = false;
    PathReportInput path;
    DmReportInput dm;
    // The windows' own state, as AppState holds it in the app.
    std::shared_ptr<hydra::ui::PathWindow> path_state = hydra::ui::new_path_window();
    std::shared_ptr<hydra::ui::DmWindow> dm_state = hydra::ui::new_dm_window();
    std::vector<size_t> path_clicks;
    std::vector<size_t> dm_clicks;
    int refreshes = 0;
    int cancels = 0;
    int retries = 0;
    int compares = 0;
    int closes = 0;
    std::string text;  // what the windows drew last frame
    // report-window-chip-outline: when set, each frame draws one chip per
    // token (note_chips) and keeps what each drew.
    bool want_chips = false;
    std::vector<ChipDrawn> chips;
};

Fixture& fixture() {
    static Fixture f;
    return f;
}

std::shared_ptr<GeneratedReport> sample_path_result() {
    auto r = std::make_shared<GeneratedReport>();
    // The sample rows counted against the sample library, as generate_report
    // hands them over: each row carries its chart's copies, and
    // library_charts is the library's own count.
    const hydra::store::ChartIndex library = report_samples::sample_index();
    r->paths = report_samples::counted_against(report_samples::sample_path_rows(), library);
    r->library_charts = library.count(library.song_ids());
    r->rows = static_cast<int64_t>(r->paths.size());
    r->subtitle = "sample path subtitle";
    r->footer = "sample path footer";
    return r;
}

std::shared_ptr<GeneratedDmReport> sample_dm_result() {
    auto r = std::make_shared<GeneratedDmReport>();
    r->rows = report_samples::sample_dm_rows();
    r->stats = hydra::app::dm_report::tally_dm_rows(r->rows);
    r->username = "SamplePlayer";
    r->subtitle = "sample dm subtitle";
    r->footer = "sample dm footer";
    return r;
}

hydra::ui::ReportCallbacks recording_callbacks(std::vector<size_t>& clicks) {
    hydra::ui::ReportCallbacks cb;
    cb.row_click = [&clicks](size_t row) { clicks.push_back(row); };
    cb.refresh = [] { ++fixture().refreshes; };
    cb.cancel = [] { ++fixture().cancels; };
    cb.try_again = [] { ++fixture().retries; };
    cb.compare_another = [] { ++fixture().compares; };
    cb.close = [] { ++fixture().closes; };
    return cb;
}

// Both windows closed, each holding a freshly built sample report.
void reset_fixture() {
    Fixture& f = fixture();
    f = Fixture{};
    f.path.result = sample_path_result();
    f.path.state = ReportBuild::Ready;
    f.path.built = std::chrono::system_clock::now();
    f.path.callbacks = recording_callbacks(f.path_clicks);
    f.dm.result = sample_dm_result();
    f.dm.state = ReportBuild::Ready;
    f.dm.built = std::chrono::system_clock::now();
    f.dm.callbacks = recording_callbacks(f.dm_clicks);
}

ImGuiWindow* path_window();

// What `chip` draws for `token` in the current window: the vertices it adds
// in the token's colour, and how far left and right they reach.
ChipDrawn draw_chip(hydra::app::report::ChipToken token) {
    static const std::string label = "Sample chip";
    const ImDrawList* list = ImGui::GetWindowDrawList();
    const int before = list->VtxBuffer.Size;
    hydra::ui::report_frame::chip(label, token);
    ChipDrawn d;
    d.token = token;
    d.label_width = ImGui::CalcTextSize(label.c_str()).x;
    const ImU32 color = ImGui::GetColorU32(hydra::ui::chip_color(token));
    for (int i = before; i < list->VtxBuffer.Size; ++i) {
        const ImDrawVert& v = list->VtxBuffer[i];
        if (v.col != color) continue;
        d.left = d.vertices == 0 ? v.pos.x : (std::min)(d.left, v.pos.x);
        d.right = d.vertices == 0 ? v.pos.x : (std::max)(d.right, v.pos.x);
        ++d.vertices;
    }
    return d;
}

// Draws one chip per token in a window of its own, big enough that none is
// clipped, and notes what each drew.
void note_chips() {
    using hydra::app::report::ChipToken;
    Fixture& f = fixture();
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(400.0f, 400.0f));
    ImGui::Begin("###chipoutline", nullptr, ImGuiWindowFlags_NoSavedSettings);
    f.chips.clear();
    for (ChipToken token : {ChipToken::t0, ChipToken::t1, ChipToken::t2, ChipToken::t3,
                            ChipToken::t4, ChipToken::t5, ChipToken::tn, ChipToken::muted})
        f.chips.push_back(draw_chip(token));
    ImGui::End();
}

// The test's GUI function: both windows, every frame, with their text kept.
void draw_windows(ImGuiTestContext*) {
    Fixture& f = fixture();
    ImGui::LogToBuffer();
    hydra::ui::draw_path_report_window(&f.path_open, f.path, *f.path_state);
    hydra::ui::draw_dm_report_window(&f.dm_open, f.dm, *f.dm_state);
    ImGuiContext& g = *ImGui::GetCurrentContext();
    f.text.assign(g.LogBuffer.begin(), g.LogBuffer.end());
    ImGui::LogFinish();
    if (f.want_chips) note_chips();
}

bool shows(const std::string& s) { return fixture().text.find(s) != std::string::npos; }

// A text box's or a dropdown's shown text as the frame text logs it.
std::string boxed(const std::string& s) { return "{ " + s + " }"; }

ImGuiWindow* path_window() { return window_named("###pathreport"); }
ImGuiWindow* dm_window() { return window_named("###dmreport"); }

// Opens the path report and points the ref at it, with Optimal paths only
// unticked: none of the sample rows is an optimal path.
void open_path_all_rows(ImGuiTestContext* ctx) {
    fixture().path_open = true;
    ctx->Yield(3);
    IM_CHECK(path_window() != nullptr);
    if (ctx->IsError()) return;
    ctx->SetRef(path_window());
    ctx->ItemClick("Optimal paths only");
    ctx->Yield(2);
}

void open_dm(ImGuiTestContext* ctx) {
    fixture().dm_open = true;
    ctx->Yield(3);
    IM_CHECK(dm_window() != nullptr);
    if (ctx->IsError()) return;
    ctx->SetRef(dm_window());
}

// Library row `row`'s title in quotes, so the word search matches it as one
// phrase. Read it before any search narrows the rows.
std::string quoted_title(Harness& h, size_t row) {
    return "\"" + h.app->library_row_at(row).title + "\"";
}

// The corpus scanned and its first chart batched, the batch's report built
// while the path window is shut. False when a step failed.
bool batch_first_chart(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return false;
    IM_CHECK_RETV(batch_search(ctx, quoted_title(h, 0)), false);
    dismiss_done(ctx);
    return true;
}

// The fixture's path window on the real report that batch built, through
// the window input AppState hands the app's own window, with the fixture's
// recording callbacks in place of the app's. The comparison side holds no
// rows: a real one needs dmleaderboards, so it is left as the app's window
// is before its first comparison lands. False when a step failed.
bool real_path_fixture(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!batch_first_chart(ctx)) return false;
    IM_CHECK_RETV(wait_until(ctx, [&] {
        return h.app->report_job == nullptr && h.app->path_report.result != nullptr;
    }, 60), false);
    IM_CHECK_RETV(!h.app->path_report.window_open, false);  // auto-open is off
    reset_fixture();
    Fixture& f = fixture();
    f.path = hydra::ui::path_report_input(*h.app);
    f.path.callbacks = recording_callbacks(f.path_clicks);
    IM_CHECK_RETV(f.path.state == ReportBuild::Ready, false);
    IM_CHECK_RETV(f.path.result != nullptr && !f.path.result->paths.empty(), false);
    f.dm.result = nullptr;
    f.dm.state = ReportBuild::Building;
    return true;
}

/// The path window's count line for `shown` of the fixture's rows, as its
// owner (report_view::count_line) words it.
std::string fixture_path_count(size_t shown) {
    return hydra::app::report_view::count_line(shown, fixture().path.result->paths.size(),
                                               hydra::app::path_report_view::kNoun);
}

// How many of the fixture's rows Optimal paths only keeps, by the window's
// own keep-rule (path_keep) with every timing tier.
size_t fixture_optimal_rows() {
    const auto keep = hydra::app::path_report_view::path_keep(std::nullopt, true);
    const std::vector<hydra::app::report::ReportRow>& rows = fixture().path.result->paths;
    return static_cast<size_t>(std::count_if(rows.begin(), rows.end(), keep));
}

// The fixture's row the last click reported, or none when no click came.
const hydra::app::report::ReportRow* last_clicked_row() {
    const Fixture& f = fixture();
    if (f.path_clicks.empty() || f.path_clicks.back() >= f.path.result->paths.size())
        return nullptr;
    return &f.path.result->paths[f.path_clicks.back()];
}

// The first sort is Score, highest first; a header click flips it. The rows
// are the real report a batch of the corpus's first chart built.
void test_sort(ImGuiTestContext* ctx) {
    if (!real_path_fixture(ctx)) return;
    Fixture& f = fixture();
    const std::vector<hydra::app::report::ReportRow>& rows = f.path.result->paths;
    f.path_open = true;
    ctx->Yield(3);
    // Optimal paths only starts ticked: only the optimal paths show.
    IM_CHECK(shows(fixture_path_count(fixture_optimal_rows())));
    open_path_all_rows(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(shows(fixture_path_count(rows.size())));
    // The flip shows only where two paths differ in score.
    const auto [low, high] = std::minmax_element(
        rows.begin(), rows.end(),
        [](const auto& a, const auto& b) { return a.score < b.score; });
    IM_CHECK_RETV(low->score < high->score, );
    // The rows are read back through their clicks: a cut cell's text never
    // reaches the frame's text.
    ctx->ItemClick("**/1");
    IM_CHECK_RETV(last_clicked_row() != nullptr, );
    IM_CHECK_EQ(last_clicked_row()->score, high->score);
    ctx->ItemClick("**/Score");
    ctx->Yield(2);
    ctx->ItemClick("**/1");
    IM_CHECK_RETV(last_clicked_row() != nullptr, );
    IM_CHECK_EQ(last_clicked_row()->score, low->score);
}

// The search box's hint, then a word no row holds, then Clear filters.
void test_filters(ImGuiTestContext* ctx) {
    if (!real_path_fixture(ctx)) return;
    Fixture& f = fixture();
    const std::vector<hydra::app::report::ReportRow>& rows = f.path.result->paths;
    open_path_all_rows(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(shows("Search song, artist, charter, or path notation"));
    // A word none of the rows' search texts holds, so the search keeps none.
    const std::string word = "zzz";
    for (const auto& r : rows)
        IM_CHECK_RETV(hydra::app::path_report_view::path_search_text(r).find(word) ==
                          std::string::npos, );
    ctx->ItemInputValue("##search", word.c_str());
    ctx->Yield(2);
    IM_CHECK(shows(fixture_path_count(0)));
    IM_CHECK(shows(hydra::app::html::kNothingMatches));
    IM_CHECK(!shows("Nothing matches those filters."));
    ctx->ItemClick("Clear filters");
    ctx->Yield(2);
    // The search is empty again and Optimal paths only stayed unticked:
    // every row is back.
    IM_CHECK(shows(fixture_path_count(rows.size())));
    IM_CHECK(!shows(boxed(word)));
}

/ Every state the frame draws, over the real report a batch of the corpus's
// first chart built.
void test_states(ImGuiTestContext* ctx) {
    if (!real_path_fixture(ctx)) return;
    Fixture& f = fixture();
    const std::string subtitle = f.path.result->subtitle;
    IM_CHECK_RETV(!subtitle.empty(), );
    f.path.state = ReportBuild::Building;
    f.path_open = true;
    ctx->Yield(3);
    // The subtitle while it builds (D103 item 21, board 2a), not the old
    // result's. The build only reads the store, so no bar counts anything.
    IM_CHECK(shows("Building the report from your library..."));
    IM_CHECK(!shows(subtitle));
    IM_CHECK(!shows("Analyzing "));
    ctx->SetRef(path_window());
    ctx->ItemClick("Cancel");
    IM_CHECK_EQ(f.cancels, 1);

    f.path.state = ReportBuild::Cancelled;
    ctx->Yield(2);
    IM_CHECK(shows("Report cancelled."));
    ctx->ItemClick("Try again");
    IM_CHECK_EQ(f.retries, 1);

    f.path.state = ReportBuild::Failed;
    ctx->Yield(2);
    IM_CHECK(shows(hydra::app::kPathReportFailed));

    // Out of date: the strip shows the sentence its owner writes for each
    // reason, and its Refresh asks for a rebuild.
    namespace frame = hydra::ui::report_frame;
    f.path.state = ReportBuild::Ready;
    f.path.out_of_date = ReportOutOfDate::Library;
    ctx->Yield(2);
    IM_CHECK(shows(frame::out_of_date_text(ReportOutOfDate::Library, std::nullopt)));
    IM_CHECK(shows("Built "));
    ctx->ItemClick("**/Refresh##outofdate");  // inside the strip's child
    IM_CHECK_EQ(f.refreshes, 1);
    // With a batch's finish time, the library line names it (D103 item 21,
    // board 2b).
    f.path.batch_finished = std::chrono::system_clock::now();
    ctx->Yield(2);
    IM_CHECK(shows(frame::out_of_date_text(ReportOutOfDate::Library, f.path.batch_finished)));
    // The settings line names no batch, even with a finish time set.
    f.path.out_of_date = ReportOutOfDate::Settings;
    ctx->Yield(2);
    IM_CHECK(shows(frame::out_of_date_text(ReportOutOfDate::Settings, f.path.batch_finished)));
    IM_CHECK(!shows(frame::out_of_date_text(ReportOutOfDate::Library, f.path.batch_finished)));
    f.path.out_of_date = ReportOutOfDate::None;
    f.path.batch_finished.reset();
    f.path_open = false;

    // The comparison while its first fetch runs (real_path_fixture leaves
    // it building with no rows), then failed.
    open_dm(ctx);
    IM_CHECK(shows("Fetching scores and building the report..."));
    IM_CHECK(!shows("Building the report from your library..."));
    IM_CHECK(shows(hydra::app::kLeaderboardWaking));
    ctx->ItemClick("Cancel");
    IM_CHECK_EQ(f.cancels, 2);
    f.dm.state = ReportBuild::Failed;
    ctx->Yield(2);
    IM_CHECK(shows(hydra::app::kComparisonFailed));  // D114 item 14
    ctx->ItemClick("Compare another player...");
    IM_CHECK_EQ(f.compares, 1);
    f.dm_open = false;

    // Analysis off with no report in memory (D103 item 24): the toolbar's
    // sentence and the rules file's error, as AppState hands them over.
    Harness& h = harness(ctx);
    reset_app(h, "max_tied_paths = 0\n");
    IM_CHECK_RETV(h.app->analysis_blocked(), );
    f.path = hydra::ui::path_report_input(*h.app);
    IM_CHECK(f.path.state == ReportBuild::None);
    f.path_open = true;
    ctx->Yield(3);
    IM_CHECK(shows(hydra::app::kAnalysisOffSentence));
    IM_CHECK(shows("max_tied_paths"));
    f.path_open = false;
    ctx->Yield(2);
}

// The path table as ImGui holds it, with the ref on the path window.
ImGuiTable* path_table(ImGuiTestContext* ctx) {
    ctx->SetRef(path_window());
    return ImGui::TableFindByID(ctx->GetID("##pathtable"));
}

// Display row `n`'s rect (its "#" Selectable, which spans the row), or none
// when the table did not draw it.
std::optional<ImRect> drawn_row(ImGuiTestContext* ctx, size_t n) {
    const ImGuiTestItemInfo info =
        ctx->ItemInfo(("**/" + std::to_string(n)).c_str(), ImGuiTestOpFlags_NoError);
    if (info.ID == 0) return std::nullopt;
    return info.RectFull;
}

// In a path window too short for the fixture's rows: a row click leaves the
// list where it is (the user's rule, 2026-10-14: a click never scrolls);
// Down onto the first row below the view brings it in at the bottom edge, not
// the middle; Up onto a row already on screen moves nothing (answer 4).
void check_keys_scroll_just_enough(ImGuiTestContext* ctx) {
    ImGuiWindow* window = path_window();
    ImGuiTable* table = path_table(ctx);
    IM_CHECK_RETV(window != nullptr && table != nullptr && table->InnerWindow != nullptr, );
    const ImVec2 size_before = window->Size;
    // Room for the header row and about two and a half rows, in
    // rows as the table drew them. With more on screen, centring the next row
    // would stop at the end of the list, where it meets the bottom edge too.
    const std::optional<ImRect> first = drawn_row(ctx, 1);
    IM_CHECK_RETV(first.has_value(), );
    const ImGuiStyle& style = ImGui::GetStyle();
    const float above = table->OuterRect.Min.y - window->Pos.y;
    const float footer = ImGui::GetTextLineHeightWithSpacing();
    ctx->WindowResize("", ImVec2(size_before.x, above + footer + style.WindowPadding.y +
                                                    style.ScrollbarSize +
                                                    first->GetHeight() * 3.5f));
    ctx->Yield(3);
    ImGuiWindow* list = table->InnerWindow;
    IM_CHECK_RETV(list->ScrollMax.y > 0.0f, );
    ImGui::SetScrollY(list, 0.0f);
    ctx->Yield(3);

    // The last row wholly on screen, and the row after it, partly or not at
    // all on screen. Centring that next row must move the list further than
    // bringing it to the bottom edge, so it sits well above the last row.
    size_t last_shown = 0;
    const size_t row_count = fixture().path.result->paths.size();
    for (size_t n = 1; n <= row_count; ++n) {
        const std::optional<ImRect> r = drawn_row(ctx, n);
        if (r && fully_shown(list, *r)) last_shown = n;
    }
    ctx->LogInfo("last row wholly on screen: %zu of %zu", last_shown, row_count);
    IM_CHECK_RETV(last_shown >= 2 && last_shown <= 3, );
    // Centring the next row moves the list past the bottom edge only with two
    // rows or more below it; with fewer, the list's end stops it there too.
    IM_CHECK_RETV(row_count >= last_shown + 3, );

    // A click on that row keeps the list still.
    const float scroll = list->Scroll.y;
    ctx->ItemClick(("**/" + std::to_string(last_shown)).c_str());
    ctx->Yield(3);
    ctx->LogInfo("scroll before the click %.1f, after %.1f", scroll, list->Scroll.y);
    IM_CHECK_EQ(list->Scroll.y, scroll);

    // Down onto the next row: it comes in at the bottom edge.
    ctx->KeyPress(ImGuiKey_DownArrow);
    ctx->Yield(3);
    const std::optional<ImRect> next = drawn_row(ctx, last_shown + 1);
    IM_CHECK_RETV(next.has_value(), );
    ctx->LogInfo("row bottom %.1f, list bottom %.1f, spacing %.1f, scroll %.1f", next->Max.y,
                 list->InnerRect.Max.y, style.ItemSpacing.y, list->Scroll.y);
    IM_CHECK(fully_shown(list, *next));
    IM_CHECK_GE(next->Max.y, list->InnerRect.Max.y - style.ItemSpacing.y);

    // Up onto the row above, which is on screen: nothing moves.
    const float after_down = list->Scroll.y;
    ctx->KeyPress(ImGuiKey_UpArrow);
    ctx->Yield(3);
    IM_CHECK_EQ(list->Scroll.y, after_down);

    ctx->WindowResize("", size_before);
    ctx->Yield(2);
}

// Row clicks, the arrow keys, Esc and Ctrl+W.
// The rows are the real report a batch of the corpus's first chart built.
void test_keys(ImGuiTestContext* ctx) {
    if (!real_path_fixture(ctx)) return;
    Fixture& f = fixture();
    open_path_all_rows(ctx);
    if (ctx->IsError()) return;
    // The second shown row's place among the rows, read by its own click.
    ctx->ItemClick("**/2");
    IM_CHECK_RETV(f.path_clicks.size() == size_t{1}, );
    const size_t second = f.path_clicks.back();
    // A click on the first shown row reports it, and Down moves to the second.
    ctx->ItemClick("**/1");
    IM_CHECK_RETV(f.path_clicks.size() == size_t{2}, );
    IM_CHECK(f.path_clicks.back() != second);
    ctx->KeyPress(ImGuiKey_DownArrow);
    ctx->Yield(2);
    IM_CHECK_EQ(f.path_clicks.back(), second);
    check_keys_scroll_just_enough(ctx);
    if (ctx->IsError()) return;

    ctx->KeyPress(ImGuiKey_Escape);
    ctx->Yield(2);
    IM_CHECK(!f.path_open);
    IM_CHECK(path_window() == nullptr);
    IM_CHECK_EQ(f.closes, 1);
    // Reopening, the window takes the rows up again. They come as AppState
    // hands rows built for an opening (keep_filters), so Optimal paths only
    // stays unticked.
    f.path.keep_filters = true;
    f.path_open = true;
    ctx->Yield(2);
    IM_CHECK(shows(fixture_path_count(f.path.result->paths.size())));
    ctx->SetRef(path_window());
    ctx->WindowFocus("");
    ctx->KeyPress(ImGuiMod_Ctrl | ImGuiKey_W);
    ctx->Yield(2);
    IM_CHECK(!f.path_open);
    IM_CHECK_EQ(f.closes, 2);
}

// The header's Close shuts the window as Esc does, in every build state and
// in both windows, over the real report a batch of the corpus's first chart
// built.
void test_close_button(ImGuiTestContext* ctx) {
    if (!real_path_fixture(ctx)) return;
    Fixture& f = fixture();
    open_path_all_rows(ctx);
    ctx->ItemClick("Close");
    ctx->Yield(2);
    IM_CHECK(!f.path_open);
    IM_CHECK(path_window() == nullptr);
    IM_CHECK_EQ(f.closes, 1);

    // Outside Ready the header has no Refresh, but Close is still there.
    for (ReportBuild state : {ReportBuild::None, ReportBuild::Cancelled, ReportBuild::Failed}) {
        f.path.state = state;
        f.path_open = true;
        ctx->Yield(3);
        IM_CHECK_RETV(path_window() != nullptr, );
        ctx->SetRef(path_window());
        IM_CHECK(ctx->ItemExists("Close"));
        IM_CHECK(!ctx->ItemExists("Refresh"));
    }
    // Shut by the fixture, not by the window, so no close callback runs.
    f.path_open = false;
    ctx->Yield(2);
    IM_CHECK_EQ(f.closes, 1);

    // Closing while the report builds.
    f.path.state = ReportBuild::Building;
    f.path_open = true;
    ctx->Yield(3);
    IM_CHECK_RETV(path_window() != nullptr, );
    ctx->SetRef(path_window());
    IM_CHECK(ctx->ItemExists("Cancel"));  // the building frame's own button
    ctx->ItemClick("Close");
    ctx->Yield(2);
    IM_CHECK(!f.path_open);
    IM_CHECK(path_window() == nullptr);
    IM_CHECK_EQ(f.closes, 2);
    IM_CHECK_EQ(f.cancels, 0);  // Close is not the build's Cancel

    // The comparison has it too, while its first fetch runs
    // (real_path_fixture leaves it so).
    open_dm(ctx);
    ctx->ItemClick("Close");
    ctx->Yield(2);
    IM_CHECK(!f.dm_open);
    IM_CHECK(dm_window() == nullptr);
    IM_CHECK_EQ(f.closes, 3);
}

/// A new result that lands while the path window is hidden frees the old one
// at once: the hidden window's view must not keep it alive until the reopen.
// Each result is a copy of the real report a batch of the corpus's first
// chart built, held by nothing but the window's input, so only the window's
// view can keep it alive.
void test_hidden_new_result(ImGuiTestContext* ctx) {
    if (!real_path_fixture(ctx)) return;
    Fixture& f = fixture();
    const GeneratedReport built = *f.path.result;
    f.path.result = std::make_shared<GeneratedReport>(built);
    f.path_open = true;
    ctx->Yield(3);
    IM_CHECK(path_window() != nullptr);
    f.path_open = false;
    ctx->Yield(2);

    std::weak_ptr<const GeneratedReport> old_path = f.path.result;
    f.path.result = std::make_shared<GeneratedReport>(built);
    ctx->Yield(2);
    IM_CHECK(old_path.expired());

    f.path_open = true;
    ctx->Yield(3);
    IM_CHECK(path_window() != nullptr);
    IM_CHECK(shows(fixture_path_count(fixture_optimal_rows())));
}

// Both report windows open at once: the path window on the real report a
// batch of the corpus's first chart built, and the comparison while its
// first fetch runs (real_path_fixture leaves it so).
void test_both(ImGuiTestContext* ctx) {
    if (!real_path_fixture(ctx)) return;
    Fixture& f = fixture();
    f.path_open = true;
    ctx->Yield(3);
    IM_CHECK_RETV(path_window() != nullptr, );
    f.dm_open = true;
    ctx->Yield(3);
    IM_CHECK(path_window() != nullptr);
    IM_CHECK(dm_window() != nullptr);
    IM_CHECK(shows(fixture_path_count(fixture_optimal_rows())));
    IM_CHECK(shows("Fetching scores and building the report..."));
}

// ---- Column widths and the window's size (one column-width rule) ----------

// In a narrow window the chips and numbers show in full and the table
// scrolls sideways. A cut cell draws past ImGui's text log, so a label in the
// frame's text was drawn whole. The rows are the real report a batch of the
// corpus's first chart built.
void test_chip_label(ImGuiTestContext* ctx) {
    if (!real_path_fixture(ctx)) return;
    open_path_all_rows(ctx);
    if (ctx->IsError()) return;
    ctx->WindowResize("", ImVec2(900.0f, 600.0f));
    ctx->Yield(3);
    Fixture& f = fixture();
    const std::vector<hydra::app::report::ReportRow>& rows = f.path.result->paths;

    // Every row's Timing chip.
    for (const auto& r : rows)
        IM_CHECK(shows(hydra::app::path_report_view::tier_label(r.tier)));

    ImGuiTable* table = path_table(ctx);
    IM_CHECK_RETV(table != nullptr && table->InnerWindow != nullptr, );
    IM_CHECK(table->InnerWindow->ScrollbarX);

    // Every number column's cells, read at both ends of the sideways scroll
    // (a column scrolled out of view draws nothing).
    const auto columns = hydra::app::path_report_view::path_columns();
    std::vector<bool> read(columns.size(), false);
    for (bool right_end : {false, true}) {
        ImGui::SetScrollX(table->InnerWindow, right_end ? table->InnerWindow->ScrollMax.x : 0.0f);
        ctx->Yield(3);
        for (size_t c = 0; c < columns.size(); ++c) {
            if (!columns[c].numeric || !table->Columns[static_cast<int>(c) + 1].IsRequestOutput)
                continue;
            read[c] = true;
            for (const auto& r : rows) IM_CHECK(shows(columns[c].cell(r)));
        }
    }
    for (size_t c = 0; c < columns.size(); ++c)
        if (columns[c].numeric) IM_CHECK(read[c]);
}

/// Every chip draws its outline in its token's colour (D308 item 5: the window
// draws the outline, so the chips match). The outline sits a chip's padding
// out from the label on each side, so the marks in the token's colour reach
// wider than the label; the label's glyphs alone stay within its width.
void test_chip_outline(ImGuiTestContext* ctx) {
    reset_app(harness(ctx));
    reset_fixture();
    Fixture& f = fixture();
    f.want_chips = true;
    ctx->Yield(3);
    f.want_chips = false;
    IM_CHECK_RETV(!f.chips.empty(), );
    for (const ChipDrawn& d : f.chips) {
        ctx->LogInfo("token %d: %d vertices in its colour, from %.1f to %.1f, label %.1f wide",
                     static_cast<int>(d.token), d.vertices, d.left, d.right, d.label_width);
        IM_CHECK_GT(d.vertices, 0);
        IM_CHECK_GT(d.right - d.left, d.label_width);
    }
}

// With the window wide, the path table follows the width rule
// (column_widths.h): every number column shows each of its cells whole, the
// table needs no sideways scroll, and the columns fill the table's room. The
// rows are the real report a batch of the corpus's first chart built.
void test_column_widths(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!real_path_fixture(ctx)) return;
    open_path_all_rows(ctx);
    if (ctx->IsError()) return;
    ctx->WindowResize("", ImVec2(static_cast<float>(h.width), 600.0f));
    ctx->Yield(3);
    const std::vector<hydra::app::report::ReportRow>& rows = fixture().path.result->paths;
    ImGuiTable* table = path_table(ctx);
    IM_CHECK_RETV(table != nullptr && table->InnerWindow != nullptr, );
    IM_CHECK(!table->InnerWindow->ScrollbarX);

    // A cut cell draws past ImGui's text log, so a number in the frame's text
    // was drawn whole.
    const auto columns = hydra::app::path_report_view::path_columns();
    for (const auto& column : columns)
        if (column.numeric)
            for (const auto& r : rows) IM_CHECK(shows(column.cell(r)));

    // place_columns hands the last open text column the pixels its rounding
    // lost, so all the room is used but a fraction of one pixel.
    int last = -1;
    for (int c = 0; c < table->ColumnsCount; ++c)
        if (table->Columns[c].IsEnabled) last = c;
    IM_CHECK_RETV(last >= 0, );
    const float right = table->Columns[last].MaxX;
    ctx->LogInfo("last column ends at %.1f, the table's room at %.1f", right,
                 table->InnerClipRect.Max.x);
    IM_CHECK_LT(std::fabs(table->InnerClipRect.Max.x - right), 1.0f);
}

/ The leftmost x of the text glyphs drawn last frame in `color` inside
// column `c` of `table`, between `top` and `bottom`: where that cell's text
// starts. Glyphs are told from filled rects by their texture coordinates (a
// rect samples the font atlas's white pixel). None when no such glyph.
std::optional<float> text_start(const ImGuiTable* table, int c, ImU32 color, float top,
                                float bottom) {
    const ImGuiTableColumn& col = table->Columns[c];
    const ImVec2 white = ImGui::GetDrawListSharedData()->TexUvWhitePixel;
    std::optional<float> left;
    for (const ImDrawVert& v : table->InnerWindow->DrawList->VtxBuffer) {
        if (v.col != color || (v.uv.x == white.x && v.uv.y == white.y)) continue;
        if (v.pos.y < top || v.pos.y > bottom || v.pos.x < col.MinX || v.pos.x > col.MaxX)
            continue;
        left = left ? (std::min)(*left, v.pos.x) : v.pos.x;
    }
    return left;
}

// Every cell's text starts at its column's left edge: a number in Score and
// the dim row number in "#" alike (Change 3 of the column-shares plan this
// file's header points at). A shorter number shows it: right-aligned, it
// would start a digit's width in from where a longer one starts. The rows are
// the real report a batch of the corpus's first chart built.
void test_cells_left(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!real_path_fixture(ctx)) return;
    open_path_all_rows(ctx);
    if (ctx->IsError()) return;
    // Wide enough that every column is on screen.
    ctx->WindowResize("", ImVec2(static_cast<float>(h.width), 600.0f));
    ctx->Yield(3);
    ImGuiTable* table = path_table(ctx);
    IM_CHECK_RETV(table != nullptr && table->InnerWindow != nullptr, );
    const auto columns = hydra::app::path_report_view::path_columns();
    int score = -1;
    for (size_t c = 0; c < columns.size(); ++c)
        if (columns[c].title == "Score") score = static_cast<int>(c) + 1;
    IM_CHECK_RETV(score > 0 && table->Columns[score].IsVisibleX, );

    const float pad = ImGui::GetStyle().CellPadding.x;
    const ImU32 text = ImGui::GetColorU32(ImGuiCol_Text);
    const ImU32 dim = ImGui::GetColorU32(hydra::ui::kDimTextColor);
    const std::pair<int, ImU32> cells[] = {{0, dim}, {score, text}};
    // Every row wholly on screen, from the bottom up, so the lowest score
    // shown is read first.
    size_t checked = 0;
    for (size_t n = fixture().path.result->paths.size(); n >= 1; --n) {
        const std::optional<ImRect> row = drawn_row(ctx, n);
        if (!row || !fully_shown(table->InnerWindow, *row)) continue;
        ++checked;
        for (const auto& [c, color] : cells) {
            const std::optional<float> x = text_start(table, c, color, row->Min.y, row->Max.y);
            IM_CHECK_RETV(x.has_value(), );
            ctx->LogInfo("row %zu column %d: text at %.1f, column starts at %.1f", n, c, *x,
                         table->Columns[c].WorkMinX);
            IM_CHECK_LE(std::fabs(*x - table->Columns[c].WorkMinX), pad);
        }
    }
    // Row 1 always fits a window this tall, so at least it was read.
    IM_CHECK_GT(checked, size_t{0});
}

// The reason store_first_chart_failure's record keeps: the NoNotesError's own
// words, for Expert Pro Drums.
std::string no_notes_reason() {
    return hydra::NoNotesError(hydra::Difficulty::Expert, true).what();
}

// The scanned library's first chart stored as a chart that can't be analyzed:
// the record failure_record makes from a NoNotesError, the call a batch makes
// for a chart with no Expert Pro Drums notes (app/analysis.cpp), saved on the
// scratch db. It returns the chart's title.
std::string store_first_chart_failure(Harness& h) {
    const std::string md5 = h.app->library_row_at(0).entry.md5;
    const std::string title = h.app->library_row_at(0).title;
    h.app->store->save(hydra::store::failure_record(md5, h.app->settings.profile(),
                                                    hydra::ErrorKind::NoNotes, no_notes_reason()));
    h.app->refresh_library_summaries();
    return title;
}

// A report too narrow for the header's buttons beside "Built HH:MM": the
// buttons start a line of their own, Close last at its right edge (the user's
// answer 7 and their choice of 2026-10-14). A report with no rows has no
// controls row or table to hold the window wide. The only report with no rows
// that builds is one whose charts the store holds as can't be analyzed
// (ReportJob::run fails a page with no rows otherwise), so the corpus is
// scanned, its first chart stored so, and the report's batch declined: the
// app's own window lists only what is stored.
void open_path_report_declining_batch(ImGuiTestContext* ctx, bool refresh = false);
bool path_report_ready(ImGuiTestContext* ctx);
void test_header_narrow(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    store_first_chart_failure(h);
    open_path_report_declining_batch(ctx);
    IM_CHECK_RETV(path_report_ready(ctx), );
    IM_CHECK_RETV(h.app->path_report.result != nullptr, );
    IM_CHECK_RETV(h.app->path_report.result->paths.empty(), );
    ImGuiWindow* window = path_window();
    IM_CHECK_RETV(window != nullptr, );
    ctx->SetRef(window);
    ctx->WindowResize("", ImVec2(900.0f, 400.0f));
    ctx->Yield(3);
    const ImRect wide = ctx->ItemInfo("Refresh").RectFull;
    const ImRect wide_close = ctx->ItemInfo("Close").RectFull;
    // "Built HH:MM" as the header drew it for this report's build time.
    const std::string drawn = visible_text(h);
    const size_t built_at = drawn.find("Built ");
    IM_CHECK_RETV(built_at != std::string::npos, );
    const std::string built = drawn.substr(built_at, std::string("Built 00:00").size());
    // Wide: Refresh and Close share the heading's line, Close at its right
    // edge with Refresh just before it.
    IM_CHECK_LE(std::fabs(wide_close.Max.x - window->WorkRect.Max.x), 1.0f);
    IM_CHECK_EQ(wide_close.Min.y, wide.Min.y);
    IM_CHECK_LT(wide.Max.x, wide_close.Min.x);

    // Room for "Hydra Path Index  Built HH:MM", but not for Refresh beside it.
    const ImGuiStyle& style = ImGui::GetStyle();
    const float header_w = ImGui::CalcTextSize("Hydra").x + ImGui::CalcTextSize("Path Index").x +
                           ImGui::CalcTextSize(built.c_str()).x + style.ItemSpacing.x * 3.0f;
    ctx->WindowResize("", ImVec2(header_w + style.WindowPadding.x * 2.0f + style.ScrollbarSize +
                                     wide.GetWidth() * 0.5f,
                                 400.0f));
    ctx->Yield(3);
    const ImRect narrow = ctx->ItemInfo("Refresh").RectFull;
    const ImRect narrow_close = ctx->ItemInfo("Close").RectFull;
    ctx->LogInfo("Refresh wide %.1f..%.1f at y %.1f, narrow %.1f..%.1f at y %.1f, right edge %.1f",
                 wide.Min.x, wide.Max.x, wide.Min.y, narrow.Min.x, narrow.Max.x, narrow.Min.y,
                 window->WorkRect.Max.x);
    ctx->LogInfo("Close wide %.1f..%.1f, narrow %.1f..%.1f at y %.1f", wide_close.Min.x,
                 wide_close.Max.x, narrow_close.Min.x, narrow_close.Max.x, narrow_close.Min.y);
    IM_CHECK_GE(narrow.Min.y, wide.Min.y + ImGui::GetTextLineHeightWithSpacing());
    IM_CHECK_EQ(narrow_close.Min.y, narrow.Min.y);
    IM_CHECK_LT(narrow.Max.x, narrow_close.Min.x);
    IM_CHECK_LE(std::fabs(narrow_close.Max.x - window->WorkRect.Max.x), 1.0f);
    h.app->close_path_report();
    ctx->Yield(2);
}

// The fixture's path window's entry in the saved-window text, as ImGui writes
// it once the window has been placed at `pos` and sized `size`, then shut.
// Empty when the window did not open.
std::string saved_path_window_entry(ImGuiTestContext* ctx, ImVec2 pos, ImVec2 size) {
    Fixture& f = fixture();
    f.path_open = true;
    ctx->Yield(3);
    ImGuiWindow* w = path_window();
    IM_CHECK_RETV(w != nullptr, std::string());
    ImGui::SetWindowPos(w, pos);
    ImGui::SetWindowSize(w, size);
    ctx->Yield(2);
    f.path_open = false;
    ctx->Yield(2);
    const std::string all = ImGui::SaveIniSettingsToMemory();
    // The entry is headed by the name ImGui keeps the window's settings
    // under, which the save above has just filled in (ImGui cuts the window's
    // name down to the part its id hashes, CreateNewWindowSettings).
    ImGuiWindowSettings* settings = ImGui::FindWindowSettingsByWindow(w);
    IM_CHECK_RETV(settings != nullptr, std::string());
    const std::string head = std::string("[Window][") + settings->GetName() + "]\n";
    const size_t start = all.find(head);
    IM_CHECK_RETV(start != std::string::npos, std::string());
    const size_t end = all.find("\n[", start);
    return all.substr(start, end == std::string::npos ? std::string::npos : end + 1 - start);
}

// A saved size bigger than the screen opens fitted to it; with the main
// window maximized the report opens at the whole work area. The runner's work
// area is its display, and it has no OS frame. Each saved entry is the one
// ImGui writes for the window after it was placed so, over the real report a
// batch of the corpus's first chart built.
void test_fits_screen(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!real_path_fixture(ctx)) return;
    const hydra::ui::WindowPlacement main_before = hydra::ui::window_placement();
    const ImVec2 origin = ImGui::GetMainViewport()->Pos;
    const ImVec2 display(static_cast<float>(h.width), static_cast<float>(h.height));
    Fixture& f = fixture();

    // Half again as big as the screen each way. It is written while the
    // window is open (an open window stays put) and read back once it is
    // shut, so the next opening is placed afresh.
    const std::string big_ini =
        saved_path_window_entry(ctx, origin, ImVec2(display.x * 1.5f, display.y * 1.5f));
    IM_CHECK_RETV(!big_ini.empty(), );
    ImGui::LoadIniSettingsFromMemory(big_ini.c_str(), big_ini.size());
    f.path_open = true;
    ctx->Yield(3);
    ImGuiWindow* w = path_window();
    IM_CHECK_RETV(w != nullptr, );
    IM_CHECK_LE(w->SizeFull.x, display.x);
    IM_CHECK_LE(w->SizeFull.y, display.y);
    IM_CHECK_GE(w->Pos.x, origin.x);
    IM_CHECK_GE(w->Pos.y, origin.y);
    IM_CHECK_LE(w->Pos.x + w->SizeFull.x, origin.x + display.x);
    IM_CHECK_LE(w->Pos.y + w->SizeFull.y, origin.y + display.y);
    f.path_open = false;
    ctx->Yield(2);

    // A small saved size, but the main window is maximized: the work area.
    const std::string small_ini = saved_path_window_entry(
        ctx, ImVec2(origin.x + 100.0f, origin.y + 100.0f), ImVec2(600.0f, 400.0f));
    IM_CHECK_RETV(!small_ini.empty(), );
    ImGui::LoadIniSettingsFromMemory(small_ini.c_str(), small_ini.size());
    hydra::ui::WindowPlacement maximized = main_before;
    maximized.valid = true;
    maximized.maximized = true;
    hydra::ui::remember_window_placement(maximized);
    f.path_open = true;
    ctx->Yield(3);
    w = path_window();
    IM_CHECK(w != nullptr);
    if (w) {
        IM_CHECK_EQ(w->Pos.x, origin.x);
        IM_CHECK_EQ(w->Pos.y, origin.y);
        IM_CHECK_EQ(w->SizeFull.x, display.x);
        IM_CHECK_EQ(w->SizeFull.y, display.y);
    }
    f.path_open = false;
    ctx->Yield(2);
    hydra::ui::remember_window_placement(main_before);
}

// ---- End to end, through AppState -----------------------------------------

// The tail of the path window's count line for the report in memory,
// " of N path" (the "s" left off, so one path matches too).
std::string path_count_tail(Harness& h) {
    const size_t n = h.app->path_report.result ? h.app->path_report.result->paths.size() : 0;
    return " of " + std::to_string(n) + " path";
}

// Esc on a report window, as a user closes it.
void press_escape_on(ImGuiTestContext* ctx, ImGuiWindow* window) {
    IM_CHECK_RETV(window != nullptr, );
    ctx->SetRef(window);
    ctx->WindowFocus("");
    ctx->KeyPress(ImGuiKey_Escape);
    ctx->Yield(2);
}

// Holds every path report build before it reads the store until release(),
// so a test can look at the building state. The job copies the hold when it
// is made; the destructor lets any held build go and removes the seam.
std::atomic<bool> g_report_release{true};
class ReportHold {
public:
    ReportHold() {
        g_report_release = false;
        seam_.emplace(hydra::ui::set_report_hold_for_test, [] {
            testwait::wait_until([] { return g_report_release.load(); },
                                 "the test to release the path report's build");
        });
    }
    ~ReportHold() { release(); }
    ReportHold(const ReportHold&) = delete;
    ReportHold& operator=(const ReportHold&) = delete;
    void release() {
        g_report_release = true;
        seam_.reset();
    }

private:
    std::optional<ScopedHook> seam_;
};

// A path report build just started under `hold`: the window shows the
// building subtitle with no count, then, once released, the rows.
void held_build_then_rows(ImGuiTestContext* ctx, ReportHold& hold) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(wait_until(ctx, [&] {
        return h.app->report_job != nullptr &&
               h.app->path_report_build() == hydra::ui::ReportBuild::Building;
    }, 10), );
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "Building the report from your library..."));
    IM_CHECK(!on_screen(h, "Analyzing "));
    hold.release();
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->report_job == nullptr &&
               h.app->path_report_build() == hydra::ui::ReportBuild::Ready;
    }, 60));
    ctx->Yield(2);
    IM_CHECK(on_screen(h, path_count_tail(h)));
}

// The toolbar's "Open path report" (or, with `refresh`, the open window's
// Refresh) with charts that have no result: with the confirm threshold at 0
// the path window asks first, and its Cancel leaves the report to list only
// what is stored. Returns with the threshold put back and that report's build
// started; the caller waits for it.
void open_path_report_declining_batch(ImGuiTestContext* ctx, bool refresh) {
    Harness& h = harness(ctx);
    h.app->report_confirm_above = 0;
    if (refresh) {
        ctx->SetRef(path_window());
        ctx->ItemClick("Refresh");
    } else {
        ctx->SetRef("//Hydra");
        ctx->ItemClick("**/Open path report");
    }
    ctx->Yield(2);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK(h.app->path_report_awaits_confirm());
    IM_CHECK(window_named("Analyze library") == nullptr);  // no main-window popup
    ctx->SetRef(path_window());
    ctx->ItemClick("Cancel");
    ctx->Yield(2);
    IM_CHECK(!h.app->path_report_awaits_confirm());
    IM_CHECK(h.app->path_report_batch() == nullptr);
    IM_CHECK(h.app->path_report_build() != hydra::ui::ReportBuild::Cancelled);
    h.app->report_confirm_above = hydra::ui::AppState::kReportConfirmAbove;
}

// Waits up to `seconds` for `seen` to hold of the running batch's snapshot,
// read beside what the last frame drew. The batch runs the real analyzer on
// the corpus, so it can end before then: that fails at once, because the
// running state can no longer be seen.
bool while_batch_runs(ImGuiTestContext* ctx,
                      const std::function<bool(const hydra::ui::BatchJob::Snapshot&)>& seen,
                      double seconds) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(h.app->batch_job != nullptr, false);
    bool ended = false;
    const bool met = wait_until(ctx, [&] {
        const hydra::ui::BatchJob::Snapshot s = h.app->batch_job->snapshot();
        if (!s.finished && seen(s)) return true;
        ended = s.finished;
        return ended;
    }, seconds);
    IM_CHECK_RETV(!ended, false);  // the batch ended before its running state showed
    return met;
}

// More rows to run than the threshold (lowered to 0 here): the path window
// itself asks, in the batch confirm's words, and its Analyze runs the batch
// the report then waits on, its count over the bar. No "Analyze library"
// popup opens in the main window. The running state is read, and the window's
// Cancel pressed, while the real batch over the corpus is paused.
void test_open_asks(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    struct Threshold {
        Harness& h;
        ~Threshold() { h.app->report_confirm_above = hydra::ui::AppState::kReportConfirmAbove; }
    } threshold{h};
    h.app->report_confirm_above = 0;
    ctx->SetRef("//Hydra");
    ctx->ItemClick("**/Open path report");
    ctx->Yield(2);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK_RETV(h.app->path_report_awaits_confirm(), );
    IM_CHECK(h.app->batch_job == nullptr);
    IM_CHECK(on_screen(h, h.app->batch_question(false)));
    IM_CHECK(window_named("Analyze library") == nullptr);

    ctx->SetRef(path_window());
    ctx->ItemClick("Analyze");
    IM_CHECK_RETV(h.app->batch_job != nullptr, );
    IM_CHECK(h.app->path_report_batch() == h.app->batch_job.get());
    // The window's Cancel stops only a batch still running
    // (AppState::cancel_path_report), and the corpus batch can end within
    // the frames a click takes. So the batch is paused at once, as the batch
    // strip's Pause pauses it (library_dialogs.cpp): it finishes the charts it
    // has started and takes no more. pause() leaves a batch that has already
    // finished as it is, which fails here.
    h.app->batch_job->pause();
    {
        const hydra::ui::BatchJob::Snapshot s = h.app->batch_job->snapshot();
        IM_CHECK_RETV(s.paused && !s.finished, );
    }
    IM_CHECK_RETV(while_batch_runs(ctx, [&](const hydra::ui::BatchJob::Snapshot& s) {
        return !s.preparing && s.total > 0 && on_screen(h, "Analyzing ");
    }, 10), );
    IM_CHECK(window_named("Analyze library") == nullptr);
    IM_CHECK(!h.app->batch_confirm_pending);

    // The window's Cancel stops the report's own batch.
    ctx->SetRef(path_window());
    ctx->ItemClick("Cancel");
    ctx->Yield(2);
    IM_CHECK(h.app->batch_job->is_cancelled());
    IM_CHECK(h.app->path_report_build() == hydra::ui::ReportBuild::Cancelled);
    IM_CHECK(wait_until(ctx, [&] { return h.app->batch_job->snapshot().finished; }, 60));
    h.app->close_path_report();
    ctx->Yield(2);
}

// Waits up to 60 s for the path report's build to end, and says whether it
// ended Ready. A build that ends any other way (failed or cancelled) can no
// longer become Ready, so the wait ends there at once.
bool path_report_ready(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    const bool ended = wait_until(ctx, [&] {
        return h.app->report_job == nullptr &&
               h.app->path_report_build() != hydra::ui::ReportBuild::Building;
    }, 60);
    ctx->Yield(2);
    const hydra::ui::ReportBuild build = h.app->path_report_build();
    if (ended && build != hydra::ui::ReportBuild::Ready)
        ctx->LogError("the path report's build ended %s, not Ready: %s",
                      build == hydra::ui::ReportBuild::Failed ? "failed" : "otherwise",
                      h.app->path_report.message.c_str());
    return ended && build == hydra::ui::ReportBuild::Ready;
}

// The path window opens from the finished strip's "Open report", from the
// toolbar's "Open path report", and by itself after a batch with "Open
// automatically" on. While it builds it shows the building state.
void test_open_path(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    const std::string first = quoted_title(h, 0);
    const std::string second = quoted_title(h, 1);
    // Opening with nothing analyzed runs a batch first: test_open_analyzes.

    // The finished strip's Open report.
    IM_CHECK(batch_search(ctx, first));
    IM_CHECK(path_window() == nullptr);  // auto-open is off
    ImGuiWindow* done = window_named("##batchdone");
    IM_CHECK_RETV(done != nullptr, );
    ctx->SetRef(done);
    ctx->ItemClick("Open report");
    ctx->Yield(2);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK(on_screen(h, path_count_tail(h)));
    press_escape_on(ctx, path_window());
    IM_CHECK(!h.app->path_report.window_open);
    IM_CHECK(path_window() == nullptr);
    dismiss_done(ctx);

    // The toolbar's Open path report. The Esc above only hid the window
    // (D271 item 7), so it shows the same rows at once, with no build.
    ctx->SetRef("//Hydra");
    ctx->ItemClick("**/Open path report");
    ctx->Yield(2);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK(h.app->report_job == nullptr);
    IM_CHECK(h.app->path_report_build() == hydra::ui::ReportBuild::Ready);
    IM_CHECK(on_screen(h, path_count_tail(h)));
    // Its Refresh, the batch declined, so the second chart below still has
    // no result.
    open_path_report_declining_batch(ctx, /*refresh=*/true);
    IM_CHECK_RETV(path_report_ready(ctx), );
    IM_CHECK(on_screen(h, path_count_tail(h)));
    h.app->close_path_report();  // it covers the toolbar
    ctx->Yield(2);

    // Open automatically: the next batch's report opens its window by itself.
    h.app->settings.auto_open_report = true;
    h.app->commit_settings();
    IM_CHECK(batch_search(ctx, second));
    ctx->Yield(2);
    IM_CHECK(h.app->path_report.window_open);
    IM_CHECK_RETV(path_window() != nullptr, );

    // Refresh with every build held before it reads the store: the window
    // shows the building state. Its batch over the charts with no result is
    // declined, so the build alone is held.
    ReportHold hold;
    open_path_report_declining_batch(ctx, /*refresh=*/true);
    held_build_then_rows(ctx, hold);
}

// Opening the report with nothing analyzed runs a batch over the corpus first
// (D249 item 8): the window shows the batch's own count over
// its bar, then the rows once the batch's report lands. The batch runs the
// real analyzer, and its count is read while it runs.
void test_open_analyzes(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;

    ctx->SetRef("//Hydra");
    IM_CHECK(ctx->ItemExists("**/Open path report"));  // offered with nothing analyzed
    ctx->ItemClick("**/Open path report");
    IM_CHECK_RETV(h.app->batch_job != nullptr, );
    IM_CHECK(h.app->path_report_batch() == h.app->batch_job.get());
    // The window shows the batch's own total over its bar.
    IM_CHECK(while_batch_runs(ctx, [&](const hydra::ui::BatchJob::Snapshot& s) {
        return !s.preparing && s.total > 0 && on_screen(h, "Analyzing ") &&
               on_screen(h, " of " + hydra::counted(s.total, "chart", "charts"));
    }, 10));
    IM_CHECK_RETV(path_window() != nullptr, );

    IM_CHECK(wait_until(ctx, [&] {
        return h.app->report_job == nullptr &&
               h.app->path_report_build() == hydra::ui::ReportBuild::Ready;
    }, 300));
    ctx->Yield(2);
    IM_CHECK(on_screen(h, path_count_tail(h)));
    // The coverage lines: the batch gave every chart a result, so none is
    // left without one, and any chart it couldn't analyze is named by the
    // report's own line on screen.
    IM_CHECK_RETV(h.app->path_report.result != nullptr, );
    const hydra::app::report::GeneratedReport& page = *h.app->path_report.result;
    // The tiles over the rows, each under the label its owner gives it.
    for (const hydra::app::report::Tile& tile :
         hydra::app::report::path_tiles(page.paths, {}, page.library_charts))
        IM_CHECK(on_screen(h, tile.label));
    IM_CHECK_EQ(page.no_result_charts, int64_t{0});
    IM_CHECK(page.no_result_line.empty());
    IM_CHECK_EQ(page.songs + page.cant_analyze_charts, page.library_charts);
    if (!page.cant_analyze_line.empty()) IM_CHECK(on_screen(h, page.cant_analyze_line));
    h.app->close_path_report();
    ctx->Yield(2);
}

// A chart whose failure the store kept (failure_record, saved on the scratch
// db the way a batch saves one, as library-cant-analyze does) is named under
// the coverage line's Show list, beneath its reason. The confirm is declined,
// so the report lists only what is stored: no rows, the coverage lines, and
// "Nothing to show" (D249 item 5).
void test_cant_analyze_list(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    const std::string title = store_first_chart_failure(h);

    open_path_report_declining_batch(ctx);
    IM_CHECK_RETV(path_report_ready(ctx), );
    IM_CHECK_RETV(h.app->path_report.result != nullptr, );
    const hydra::app::report::GeneratedReport& page = *h.app->path_report.result;
    IM_CHECK(page.paths.empty());
    IM_CHECK(on_screen(h, hydra::app::report::kNothingToShow));
    IM_CHECK(!page.cant_analyze_line.empty());
    IM_CHECK(on_screen(h, page.cant_analyze_line));
    IM_CHECK(!on_screen(h, no_notes_reason()));

    ctx->SetRef(path_window());
    ctx->ItemClick("**/Show list##cantanalyze");
    ctx->Yield(2);
    IM_CHECK(on_screen(h, no_notes_reason()));
    IM_CHECK(on_screen(h, title));
    IM_CHECK(on_screen(h, "Hide list"));
    h.app->close_path_report();
    ctx->Yield(2);
}

// The path report lists the current chart mode's records only, and a row
// click selects its chart without switching the chart mode.
void test_row_click(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    const std::string title = quoted_title(h, 0);
    const std::string md5 = h.app->library_row_at(0).entry.md5;
    auto selected_md5 = [&] { return h.app->selected ? h.app->selected->md5 : std::string(); };

    // The chart stored under two chart modes; the report lists the second.
    const std::string mode_a = h.app->settings.chartmode_key();
    IM_CHECK(batch_search(ctx, title));
    dismiss_done(ctx);
    h.app->settings.view_prodrums = !h.app->settings.view_prodrums;
    h.app->commit_settings();
    const std::string mode_b = h.app->settings.chartmode_key();
    IM_CHECK(mode_a != mode_b);
    IM_CHECK(batch_search(ctx, title));
    dismiss_done(ctx);

    // Optimal paths only leaves the one row of the current mode.
    h.app->show_path_report();
    ctx->Yield(3);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK(on_screen(h, "1" + path_count_tail(h)));
    ctx->SetRef(path_window());
    ctx->ItemClick("**/1");
    ctx->Yield(2);
    IM_CHECK_STR_EQ(selected_md5().c_str(), md5.c_str());
    IM_CHECK_STR_EQ(h.app->settings.chartmode_key().c_str(), mode_b.c_str());
    h.app->close_path_report();  // it covers the toolbar
    ctx->Yield(2);
}

// The path window's count line for the report in memory, "n of M paths", as
// drawn this frame; empty when it isn't on screen.
std::string shown_count(Harness& h) {
    const std::string text = visible_text(h);
    const std::string tail = path_count_tail(h);
    const size_t at = text.find(tail);
    if (at == std::string::npos) return {};
    size_t start = at;
    while (start > 0 && (std::isdigit(static_cast<unsigned char>(text[start - 1])) ||
                         text[start - 1] == ','))
        --start;
    return text.substr(start, at + tail.size() - start);
}

// The first chart batched, its report built while the window is shut, then
// the window opened on that report. False when a step failed.
bool open_batch_report(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!batch_first_chart(ctx)) return false;
    h.app->show_path_report();
    ctx->Yield(3);
    IM_CHECK_RETV(path_window() != nullptr, false);
    IM_CHECK_RETV(h.app->report_job == nullptr, false);  // the batch's report, kept while shut
    IM_CHECK_RETV(h.app->path_report.result != nullptr, false);
    return true;
}

// A batch's report, built while the window is shut, shows at once. Esc then
// only hides the window and its rows stay (D271 item 7), so opening it again
// shows the same rows at once, with no build and no building state.
void test_reopen(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_batch_report(ctx)) return;
    const std::string count = shown_count(h);
    IM_CHECK_RETV(!count.empty(), );
    const GeneratedReport* rows = h.app->path_report.result.get();

    press_escape_on(ctx, path_window());
    IM_CHECK(!h.app->path_report.window_open);
    IM_CHECK(path_window() == nullptr);
    IM_CHECK(h.app->path_report.result != nullptr);
    IM_CHECK(h.app->path_report.result.get() == rows);

    ctx->SetRef("//Hydra");
    ctx->ItemClick("**/Open path report");
    ctx->Yield(2);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK(h.app->report_job == nullptr);
    IM_CHECK(h.app->path_report_build() == hydra::ui::ReportBuild::Ready);
    IM_CHECK(!on_screen(h, "Building the report from your library..."));
    IM_CHECK(h.app->path_report.result.get() == rows);
    IM_CHECK_STR_EQ(shown_count(h).c_str(), count.c_str());  // the same rows as before
}

// A report that lands while the path window is hidden lets the one it
// replaced go at once (D280): the app draws the hidden window every frame,
// so the window's view stops holding the old rows now, not at the reopen.
// The build is held until the window is hidden, so it lands while hidden.
void test_hidden_lets_go(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_batch_report(ctx)) return;
    const std::weak_ptr<const GeneratedReport> old = h.app->path_report.result;

    ReportHold hold;
    open_path_report_declining_batch(ctx, /*refresh=*/true);
    if (ctx->IsError()) return;
    IM_CHECK_RETV(wait_until(ctx, [&] {
        return h.app->report_job != nullptr &&
               h.app->path_report_build() == hydra::ui::ReportBuild::Building;
    }, 10), );
    press_escape_on(ctx, path_window());
    IM_CHECK_RETV(!h.app->path_report.window_open, );
    IM_CHECK(!old.expired());  // nothing has replaced it yet

    hold.release();
    IM_CHECK_RETV(path_report_ready(ctx), );
    IM_CHECK(!h.app->path_report.window_open);
    IM_CHECK(h.app->path_report.result != nullptr);
    IM_CHECK(old.expired());
}

// The path table's sort as ImGui holds it: table column index and direction,
// first key first. The window hands the table its view's sort.
std::vector<std::pair<int, int>> table_sort(ImGuiTestContext* ctx) {
    std::vector<std::pair<int, int>> out;
    ctx->SetRef(path_window());
    const ImGuiTableSortSpecs* specs = ctx->TableGetSortSpecs("##pathtable");
    if (!specs) return out;
    for (int i = 0; i < specs->SpecsCount; ++i)
        out.emplace_back(specs->Specs[i].ColumnIndex, static_cast<int>(specs->Specs[i].SortDirection));
    return out;
}

bool best_only_ticked(ImGuiTestContext* ctx) {
    ctx->SetRef(path_window());
    return (ctx->ItemInfo("Optimal paths only").StatusFlags & ImGuiItemStatusFlags_Checked) != 0;
}

// Closing the window and opening it again keeps its search, timing,
// Optimal paths only and sort (the user's answer to D103 item 28's
// follow-up, kept by D271 item 7). Refresh still resets all four.
void test_reopen_keeps_filters(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_batch_report(ctx)) return;
    // Read only before the Refresh below replaces these rows.
    const GeneratedReport& report = *h.app->path_report.result;
    const std::string default_count = shown_count(h);
    const auto default_sort = table_sort(ctx);
    IM_CHECK_RETV(!default_count.empty() && !default_sort.empty(), );
    IM_CHECK(best_only_ticked(ctx));

    // Sort by Song, then flip it: Z to A.
    ctx->SetRef(path_window());
    ctx->ItemClick("**/Song");
    ctx->Yield(2);
    ctx->ItemClick("**/Song");
    ctx->Yield(2);
    const auto sort = table_sort(ctx);
    IM_CHECK_RETV(!sort.empty() && sort != default_sort, );
    IM_CHECK_EQ(sort.front().second, static_cast<int>(ImGuiSortDirection_Descending));

    // Every path, then a search that keeps them (the chart's own song).
    ctx->SetRef(path_window());
    ctx->ItemClick("Optimal paths only");
    ctx->Yield(2);
    IM_CHECK(!best_only_ticked(ctx));
    const std::string all_count = shown_count(h);
    const std::string search = report.paths.front().song;
    ctx->SetRef(path_window());
    ctx->ItemInputValue("##search", search.c_str());
    ctx->Yield(2);
    IM_CHECK_STR_EQ(shown_count(h).c_str(), all_count.c_str());
    const std::string search_shown = boxed(search);
    IM_CHECK(on_screen(h, search_shown));

    // A timing tier a path has, so the table stays up to show its sort.
    const auto choices = hydra::app::path_report_view::timing_choices();
    std::string label;
    for (size_t i = 1; i < choices.size() && label.empty(); ++i)
        for (const auto& r : report.paths)
            if (r.tier == *choices[i].tier) label = choices[i].label;
    IM_CHECK_RETV(!label.empty(), );
    ctx->SetRef(path_window());
    ctx->ComboClick(("##timing/" + label).c_str());
    ctx->Yield(2);
    const std::string count = shown_count(h);
    IM_CHECK_RETV(!count.empty() && count != default_count, );
    const std::string timing_shown = boxed(label);
    const std::string default_timing = boxed(choices.front().label);
    IM_CHECK(on_screen(h, timing_shown));

    // Esc only hides the window; opening it again shows it as it was left.
    press_escape_on(ctx, path_window());
    IM_CHECK(path_window() == nullptr);
    ctx->SetRef("//Hydra");
    ctx->ItemClick("**/Open path report");
    ctx->Yield(2);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK_STR_EQ(shown_count(h).c_str(), count.c_str());
    IM_CHECK(on_screen(h, search_shown));
    IM_CHECK(on_screen(h, timing_shown));
    IM_CHECK(!best_only_ticked(ctx));
    IM_CHECK(table_sort(ctx) == sort);

    // Refresh starts all four over. Its batch is declined too, so the rows
    // stay the same chart's.
    open_path_report_declining_batch(ctx, /*refresh=*/true);
    IM_CHECK_RETV(path_report_ready(ctx), );
    ctx->Yield(1);
    IM_CHECK_STR_EQ(shown_count(h).c_str(), default_count.c_str());
    IM_CHECK(!on_screen(h, search_shown));
    IM_CHECK(on_screen(h, default_timing));
    IM_CHECK(best_only_ticked(ctx));
    IM_CHECK(table_sort(ctx) == default_sort);
}

// A window's state goes with its AppState (D308 item 3): a new AppState, as
// the runner builds one per test, opens the path window fresh. The fixture
// draws each AppState's own window state over the real report a batch of the
// corpus's first chart built. The second AppState's window gets back the
// very report the first one showed, because a new report starts a window
// over by itself and would prove nothing.
void test_fresh_state(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!real_path_fixture(ctx)) return;
    Fixture& f = fixture();
    f.path_state = h.app->path_window_ui;
    const std::shared_ptr<hydra::ui::PathWindow> first_path = h.app->path_window_ui;
    const std::shared_ptr<hydra::ui::DmWindow> first_dm = h.app->dm_window_ui;
    const std::shared_ptr<const GeneratedReport> report = f.path.result;
    const std::string typed = report->paths.front().song;

    open_path_all_rows(ctx);
    if (ctx->IsError()) return;
    ctx->ItemInputValue("##search", typed.c_str());
    ctx->Yield(2);
    IM_CHECK_RETV(shows(boxed(typed)), );
    IM_CHECK_RETV(!best_only_ticked(ctx), );
    f.path_open = false;
    ctx->Yield(2);

    // The runner's step between tests: the old AppState goes and a new one
    // comes. The first one's window states are kept alive here, so the new
    // ones cannot take their addresses.
    reset_app(h);
    IM_CHECK(h.app->path_window_ui != first_path);
    IM_CHECK(h.app->dm_window_ui != first_dm);
    f.path_state = h.app->path_window_ui;
    f.path.result = report;
    f.path_open = true;
    ctx->Yield(3);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK(shows("Search song, artist, charter, or path notation"));
    IM_CHECK(!shows(boxed(typed)));
    IM_CHECK(best_only_ticked(ctx));
    IM_CHECK(shows(fixture_path_count(fixture_optimal_rows())));
    f.path_open = false;
    ctx->Yield(2);
}

/ After a batch the report goes out of date with the batch's time; a setting
// the report reads gives the settings sentence; Refresh rebuilds and clears
// the strip.
void test_out_of_date(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(batch_search(ctx, quoted_title(h, 0)));
    dismiss_done(ctx);
    IM_CHECK(!h.app->path_report.batch_finished.has_value());

    // A batch that is stopped builds no report, so the one in memory stays,
    // out of date. The real batch over the corpus is stopped once it reaches
    // its charts, before it can end by itself.
    ctx->SetRef("//Hydra");
    ctx->ItemInputValue("**/##search", "");
    IM_CHECK_RETV(wait_until(ctx, [&] { return h.app->search.empty(); }, 5), );
    start_library_batch(ctx);
    IM_CHECK_RETV(while_batch_runs(ctx, [](const hydra::ui::BatchJob::Snapshot& s) {
        return !s.preparing && s.total > 0;
    }, 30), );
    h.app->batch_job->stop();
    IM_CHECK_RETV(wait_until(ctx, [&] { return h.app->batch_job->snapshot().finished; }, 300), );
    ctx->Yield(3);
    IM_CHECK(h.app->report_job == nullptr);
    IM_CHECK_RETV(h.app->path_report.batch_finished.has_value(), );
    dismiss_done(ctx);

    // Each strip sentence as its owner writes it for this report.
    namespace frame = hydra::ui::report_frame;
    const std::string library_line =
        frame::out_of_date_text(ReportOutOfDate::Library, h.app->path_report.batch_finished);
    const std::string settings_line =
        frame::out_of_date_text(ReportOutOfDate::Settings, std::nullopt);
    h.app->show_path_report();
    ctx->Yield(3);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK(on_screen(h, library_line));

    // A setting the report reads.
    h.app->settings.sp_cap = 5;
    h.app->commit_settings();
    ctx->Yield(2);
    IM_CHECK(on_screen(h, settings_line));
    IM_CHECK(!on_screen(h, library_line));

    // Nothing is analyzed at cap 5, so a report built there would fail.
    // Changing the cap back still leaves the rows out of date: the report
    // was built before both changes.
    h.app->settings.sp_cap = hydra::kCloneHeroSpCap;
    h.app->commit_settings();
    ctx->Yield(2);
    IM_CHECK(on_screen(h, settings_line));

    ctx->SetRef(path_window());
    ctx->ItemClick("**/Refresh##outofdate");
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->report_job == nullptr &&
               h.app->path_report_build() == hydra::ui::ReportBuild::Ready;
    }, 60));
    ctx->Yield(2);
    IM_CHECK(!on_screen(h, settings_line));
    IM_CHECK(!on_screen(h, library_line));

    // The report lists one chart mode, so changing it (here Pro Drums) puts
    // the rows out of date too.
    h.app->settings.view_prodrums = !h.app->settings.view_prodrums;
    h.app->commit_settings();
    ctx->Yield(2);
    IM_CHECK(on_screen(h, settings_line));
}

}  // namespace

// The end-to-end scripts, registered by uitest_tests.cpp with no GUI function:
// the app draws the windows itself.
const std::vector<TestEntry>& report_window_flow_tests() {
    static const std::vector<TestEntry> entries = {
        {"report-window-open-path", test_open_path},
        {"report-window-open-analyzes", test_open_analyzes},
        {"report-window-open-asks", test_open_asks},
        {"report-window-cant-analyze-list", test_cant_analyze_list},
        {"report-window-row-click", test_row_click},
        {"report-window-reopen", test_reopen},
        {"report-window-hidden-lets-go", test_hidden_lets_go},
        {"report-window-reopen-keeps-filters", test_reopen_keeps_filters},
        {"report-window-out-of-date", test_out_of_date},
    };
    return entries;
}

void register_report_window_tests(Harness& h) {
    static const TestEntry kTests[] = {
        {"report-window-sort", test_sort},
        {"report-window-filters", test_filters},
        {"report-window-states", test_states},
        {"report-window-keys", test_keys},
        {"report-window-close-button", test_close_button},
        {"report-window-hidden-new-result", test_hidden_new_result},
        {"report-windows-both", test_both},
        {"report-window-chip-label", test_chip_label},
        {"report-window-chip-outline", test_chip_outline},
        {"report-window-column-widths", test_column_widths},
        {"report-window-cells-left", test_cells_left},
        {"report-window-header-narrow", test_header_narrow},
        {"report-window-fits-screen", test_fits_screen},
        {"report-window-fresh-state", test_fresh_state},
    };
    for (const TestEntry& e : kTests) {
        ImGuiTest* t = IM_REGISTER_TEST(h.engine, "hydra", e.name);
        t->UserData = &h;
        t->GuiFunc = draw_windows;
        t->TestFunc = e.fn;
    }
}

}  // namespace uitest
