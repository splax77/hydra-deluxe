// The Paths tab's GUI tests, driven on Burnout (Green Day) from the scratch
// library: the path buttons, the activation rows and their folds, the backend
// table and its limit, the two folds under the list, and Copy path. Labels are
// the label contract of the UI redesign (D288; docs/superpowers/plans/2026-09-27-ui-redesign.md (removed; git show 444631e6:docs/superpowers/plans/2026-09-27-ui-redesign.md)).

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

#include "uitest_harness.h"

#include "app/config.h"
#include "app/display_format.h"  // kAverageMultiplierLabel
#include "app/path_view.h"
#include "core/backend_value.h"  // counted_without_squeeze
#include "core/model.h"
#include "core/note_window.h"     // role_word
#include "core/squeeze_rating.h"  // rate_activation
#include "imgui_internal.h"
#include "ui/activation_row_layout.h"
#include "ui/app_state.h"
#include "ui/badge_pill.h"
#include "ui/column_widths.h"
#include "ui/details_view.h"
#include "ui/fonts.h"  // px()
#include "ui/preview_controller.h"
#include "ui/text_role.h"
#include "ui/theme.h"  // kBestPathColor, kWarningColor

namespace uitest {

// Opens every fold of the Paths tab that is closed: the Backend timings fold
// of each open activation row (the user may have closed it), and the
// Multiplier squeeze and Score breakdown folds. It clicks only the closed
// ones, so a second call changes nothing.
// The layout checks call it after Expand all, so the whole tab is on screen.
// Declared in uitest_harness.h for the layout sweep in uitest_details.cpp.
void open_paths_folds(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    hydra::app::PathsTabUi& ui = h.app->details_ui.paths_tab.ui();
    if (const hydra::app::PathsTabCache::Details* d = h.app->details_ui.paths_tab.built_details()) {
        const std::vector<hydra::app::ActivationRowView>& acts = d->activations.acts;
        for (size_t i = 0; i < acts.size(); ++i) {
            if (i >= ui.act_open.size() || !ui.act_open[i]) continue;
            if (i >= ui.backends_open.size() || ui.backends_open[i]) continue;
            char label[48];
            std::snprintf(label, sizeof(label), "**/Backend timings##act%d", acts[i].number);
            ctx->ItemClick(label);
            ctx->Yield(1);
        }
    }
    if (!ui.mult_open && ctx->ItemExists("**/Multiplier squeeze##mult")) {
        ctx->ItemClick("**/Multiplier squeeze##mult");
        ctx->Yield(1);
    }
    if (!ui.breakdown_open && ctx->ItemExists("**/Score breakdown##breakdown")) {
        ctx->ItemClick("**/Score breakdown##breakdown");
        ctx->Yield(1);
    }
    ctx->Yield(1);
}

namespace {

// Keeps a test off the real Windows clipboard: while alive, ImGui reads and
// writes text() instead, and the previous handlers come back afterwards.
struct FakeClipboard {
    static std::string& text() {
        static std::string t;
        return t;
    }
    ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
    const char* (*old_get)(ImGuiContext*) = pio.Platform_GetClipboardTextFn;
    void (*old_set)(ImGuiContext*, const char*) = pio.Platform_SetClipboardTextFn;
    FakeClipboard() {
        text().clear();
        pio.Platform_GetClipboardTextFn = [](ImGuiContext*) -> const char* {
            return text().c_str();
        };
        pio.Platform_SetClipboardTextFn = [](ImGuiContext*, const char* s) {
            text() = s ? s : "";
        };
    }
    ~FakeClipboard() {
        pio.Platform_GetClipboardTextFn = old_get;
        pio.Platform_SetClipboardTextFn = old_set;
    }
};

// Puts the system clipboard's text back when the test ends, however it ends,
// for a test that lets Copy path write the real clipboard.
struct KeptClipboardText {
    std::string text;
    bool had_text = false;
    KeptClipboardText() {
        const char* t = ImGui::GetClipboardText();
        had_text = t != nullptr && *t != '\0';
        if (had_text) text = t;
    }
    ~KeptClipboardText() {
        if (had_text) ImGui::SetClipboardText(text.c_str());
    }
    KeptClipboardText(const KeptClipboardText&) = delete;
    KeptClipboardText& operator=(const KeptClipboardText&) = delete;
};

// A fresh app with Burnout analyzed and its Paths tab showing.
bool open_burnout(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return false;
    open_titled(ctx, "burnout", "Burnout");
    if (ctx->IsError()) return false;
    wait_song_analyzed(ctx);  // waits, then lands on the Paths tab
    if (ctx->IsError()) return false;
    IM_CHECK_RETV(h.app->viewed.ready(), false);
    ctx->Yield(2);
    return true;
}

// The path list as the Paths tab drew it: the cache's buttons, asked with the
// tab's own key (render_path_panel in paths_tab.cpp), so nothing is rebuilt.
const hydra::app::PathButtonsView& shown_buttons(hydra::ui::AppState& app) {
    return app.details_ui.paths_tab.buttons(*app.viewed.record, app.record_generation.n,
                                            app.settings.depth_mode, app.settings.depth_value);
}

// The ImGui id of path button `i` (path_button in paths_tab.cpp).
std::string path_button_ref(size_t i) {
    return "**/##path" + std::to_string(i);
}

// The path list: its headings, every button the list view holds with its own
// title, timing and detail line, and clicks that change the shared selection.
// Every expected text is the list view's (app::build_path_buttons) and every
// expected path is the record's.
void test_paths_list(ImGuiTestContext* ctx) {
    using Group = hydra::app::PathButtonView::Group;
    Harness& h = harness(ctx);
    if (!open_burnout(ctx)) return;
    const hydra::HydraRecord& rec = *h.app->viewed.record;
    const hydra::app::PathButtonsView& list = shown_buttons(*h.app);
    IM_CHECK(!list.buttons.empty());
    if (list.buttons.empty()) return;
    // The record's best path is the first button, under Optimal.
    IM_CHECK(list.buttons[0].path == &rec.best_path());
    IM_CHECK(list.buttons[0].group == Group::Optimal);
    size_t within = list.buttons.size();
    size_t allzero = list.buttons.size();
    for (size_t i = 0; i < list.buttons.size(); ++i) {
        const hydra::app::PathButtonView& b = list.buttons[i];
        IM_CHECK(b.path != nullptr);
        if (b.path == nullptr) return;
        // Each button names its own path, and shows what the view gives it.
        IM_CHECK_STR_EQ(b.notation.c_str(), b.path->pathstring().c_str());
        IM_CHECK(on_screen(h, b.title));
        if (!b.timing.empty()) IM_CHECK(on_screen(h, b.timing));
        if (!b.detail.empty()) IM_CHECK(on_screen(h, b.detail));
        // A path in a fold has its score on the fold, not on its button.
        if (!b.fold.empty()) {
            IM_CHECK(on_screen(h, b.fold));
            IM_CHECK(on_screen(h, b.fold_count));
            IM_CHECK(!on_screen(h, b.fold + hydra::app::kFactSeparator + b.notation));
        }
        if (b.group == Group::Within && within == list.buttons.size()) within = i;
        if (b.group == Group::AllZero && allzero == list.buttons.size()) allzero = i;
    }
    // Burnout's list has all three groups, so all three headings show.
    IM_CHECK(within < list.buttons.size());
    IM_CHECK(allzero < list.buttons.size());
    if (within >= list.buttons.size() || allzero >= list.buttons.size()) return;
    IM_CHECK(on_screen(h, "Optimal"));
    IM_CHECK(on_screen(h, list.within_label));
    IM_CHECK(on_screen(h, hydra::app::kBestAllZeroLabel));
    IM_CHECK(!on_screen(h, "0 ms limit"));
    IM_CHECK(!on_screen(h, "hardest squeeze"));
    // Those three are the only headings: these two never show.
    IM_CHECK(!on_screen(h, "Optimal Path"));
    IM_CHECK(!on_screen(h, "More Paths"));

    IM_CHECK(h.app->details_ui.selected_path == &rec.best_path());
    ctx->ItemClick("**/##act2");  // a row open on this path
    ctx->Yield(2);
    {
        const std::vector<char>& open = h.app->details_ui.paths_tab.ui().act_open;
        IM_CHECK_GE(open.size(), static_cast<size_t>(2));
        if (open.size() < 2) return;
        std::vector<char> want(open.size(), 0);
        want[1] = 1;
        IM_CHECK(open == want);
    }
    ctx->ItemClick(path_button_ref(within).c_str());
    ctx->Yield(2);
    IM_CHECK(h.app->details_ui.selected_path == list.buttons[within].path);
    // A new path opens with every row closed, whatever the last one had open.
    {
        const std::vector<char>& open = h.app->details_ui.paths_tab.ui().act_open;
        IM_CHECK(std::all_of(open.begin(), open.end(), [](char c) { return c == 0; }));
    }
    ctx->ItemClick(path_button_ref(allzero).c_str());
    ctx->Yield(2);
    IM_CHECK(h.app->details_ui.selected_path == list.buttons[allzero].path);

    // A closed fold hides its paths and nothing else; opening it brings them
    // back. The fold is the first Within score's.
    const std::string fold = list.buttons[within].fold;
    IM_CHECK(!fold.empty());
    if (fold.empty()) return;
    auto in_fold = [&](size_t i) {
        return list.buttons[i].group == Group::Within && list.buttons[i].fold == fold;
    };
    // A title another shown button's title contains stays on screen with the
    // fold closed, so only the others are looked for.
    auto title_elsewhere = [&](size_t i) {
        for (size_t j = 0; j < list.buttons.size(); ++j)
            if (!in_fold(j) && list.buttons[j].title.find(list.buttons[i].title) != std::string::npos)
                return true;
        return false;
    };
    const std::string fold_ref = "**/" + fold + "##fold";
    ctx->ItemClick(fold_ref.c_str());
    ctx->Yield(2);
    IM_CHECK(on_screen(h, fold));
    for (size_t i = 0; i < list.buttons.size(); ++i) {
        if (!in_fold(i))
            IM_CHECK(on_screen(h, list.buttons[i].title));
        else if (!title_elsewhere(i))
            IM_CHECK(!on_screen(h, list.buttons[i].title));
    }
    ctx->ItemClick(fold_ref.c_str());
    ctx->Yield(2);
    for (size_t i = 0; i < list.buttons.size(); ++i)
        if (in_fold(i)) IM_CHECK(on_screen(h, list.buttons[i].title));
}

// One timeline mark as the Paths tab last drew it: where its gold bar sits,
// and whether an orange or a grey stroke is drawn around it.
struct DrawnMark {
    float x = 0.0f;
    float y = 0.0f;  // the middle of the gold bar, for hovering it
    bool orange = false;
    bool grey = false;
};

// The timeline's marks, read from the vertices the Paths tab drew last frame,
// left to right. The strip is the only gold above the first activation row in
// the details column, so its gold vertices give each mark's place; a stroke
// counts as the mark's when it sits within `reach` of it.
std::vector<DrawnMark> drawn_marks(ImGuiTestContext* ctx) {
    std::vector<DrawnMark> marks;
    ImGuiWindow* details = window_named("##pathdetails");
    if (!details) return marks;
    const float row_top = ctx->ItemInfo("**/##act1").RectFull.Min.y;
    const ImU32 gold = ImGui::GetColorU32(hydra::ui::kBestPathColor);
    const ImU32 orange = ImGui::GetColorU32(hydra::ui::kWarningColor);
    const ImU32 grey = ImGui::GetColorU32(ImGuiCol_TextDisabled);
    const ImVector<ImDrawVert>& vtx = details->DrawList->VtxBuffer;
    const float reach = hydra::ui::px(6.0f);
    float top = FLT_MAX, bottom = -FLT_MAX;
    std::vector<float> xs;
    for (const ImDrawVert& v : vtx) {
        if (v.col != gold || v.pos.y >= row_top) continue;
        top = (std::min)(top, v.pos.y);
        bottom = (std::max)(bottom, v.pos.y);
        xs.push_back(v.pos.x);
    }
    std::sort(xs.begin(), xs.end());
    // One mark per cluster of gold, at the middle of its left and right edge.
    float left = 0.0f, right = 0.0f;
    for (size_t i = 0; i < xs.size(); ++i) {
        if (i == 0 || xs[i] - right > reach) {
            if (i > 0) marks.push_back({(left + right) * 0.5f});
            left = xs[i];
        }
        right = xs[i];
    }
    if (!xs.empty()) marks.push_back({(left + right) * 0.5f});
    for (DrawnMark& m : marks) {
        m.y = (top + bottom) * 0.5f;
        for (const ImDrawVert& v : vtx) {
            if (v.pos.y < top - reach || v.pos.y > bottom + reach) continue;
            if (std::fabs(v.pos.x - m.x) > reach) continue;
            if (v.col == orange) m.orange = true;
            if (v.col == grey) m.grey = true;
        }
    }
    return marks;
}

// The open record's activation rows: the same call the Paths tab makes, with
// the same inputs (render_path_panel in paths_tab.cpp), so this is the cached
// view it draws and nothing is rebuilt.
const hydra::app::ActivationsView& shown_activations(hydra::ui::AppState& app) {
    return app.details_ui.paths_tab
        .details(*app.details_ui.selected_path, *app.viewed.record, app.record_generation.n,
                 app.viewed.timing ? &*app.viewed.timing : nullptr, app.settings.backend_limit(),
                 app.settings.rules, app.viewed.song_length_ms, app.settings.view_prodrums,
                 app.viewed.chords ? &*app.viewed.chords : nullptr)
        .activations;
}

// The outline a timeline mark is drawn with.
enum class Outline { None, Orange, Grey };

// The outline each activation's mark should have, from the engine's badges on
// its row (app::activation_badges): none without a badge, orange when any
// badge is a warning, grey otherwise.
std::vector<Outline> badge_outlines(const hydra::app::ActivationsView& view) {
    std::vector<Outline> out;
    for (const hydra::app::ActivationRowView& a : view.acts) {
        bool warn = false;
        for (const hydra::app::ActivationBadge& b : a.badges) warn = warn || b.warn;
        out.push_back(a.badges.empty() ? Outline::None : warn ? Outline::Orange : Outline::Grey);
    }
    return out;
}

// Each mark's drawn outline, against the outline its activation's badges
// call for (badge_outlines), one per activation in order.
void check_marks(ImGuiTestContext* ctx, hydra::ui::AppState& app) {
    const hydra::app::ActivationsView& view = shown_activations(app);
    const std::vector<Outline> expected = badge_outlines(view);
    const std::vector<DrawnMark> marks = drawn_marks(ctx);
    IM_CHECK_EQ(marks.size(), expected.size());
    if (marks.size() != expected.size() || view.acts.size() != expected.size()) return;
    // Every mark is logged before any check, so a failure shows them all.
    for (size_t i = 0; i < marks.size(); ++i) {
        std::string badges;
        for (const hydra::app::ActivationBadge& b : view.acts[i].badges)
            badges += "[" + b.text + (b.warn ? ", orange] " : "] ");
        ctx->LogInfo("mark %d at %.1f: badges %s, orange %d, grey %d", view.acts[i].number,
                     marks[i].x, badges.c_str(), marks[i].orange, marks[i].grey);
    }
    for (size_t i = 0; i < marks.size(); ++i) {
        IM_CHECK_EQ(marks[i].orange, expected[i] == Outline::Orange);
        IM_CHECK_EQ(marks[i].grey, expected[i] == Outline::Grey);
    }
}

// The song length paths-rows hands the open record before checking its
// timeline marks, in ms: an input, since the GUI test library has no audio.
constexpr double kMarksSongLengthMs = 600000.0;

// Whether `view`'s rows hold an outline of kind `o`: a chart's test of the
// marks covers that colour only when one of its rows calls for it.
bool has_outline(const hydra::app::ActivationsView& view, Outline o) {
    const std::vector<Outline> all = badge_outlines(view);
    return std::find(all.begin(), all.end(), o) != all.end();
}

// The activation rows: one line each, all closed at first, one open at a
// time, Expand all and Collapse all, and the summary beside the "Activations"
// heading. Every expected text is the activations view's
// (app::build_activations), which the engine's record fills.
void test_paths_rows(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_burnout(ctx)) return;
    // Copies: a longer song length below rebuilds the view.
    const std::vector<hydra::app::ActivationRowView> acts = shown_activations(*h.app).acts;
    IM_CHECK_GE(acts.size(), static_cast<size_t>(2));
    if (acts.size() < 2) return;
    const hydra::app::ActivationRowView& row1 = acts[0];
    IM_CHECK(on_screen(h, "Activations"));
    IM_CHECK(on_screen(h, shown_activations(*h.app).summary));
    IM_CHECK(!on_screen(h, "bars each"));
    for (const hydra::app::ActivationRowView& a : acts) {
        IM_CHECK(on_screen(h, a.measure));
        for (const hydra::app::ActivationBadge& b : a.badges) IM_CHECK(on_screen(h, b.text));
    }
    // Row 1 is Burnout's squeeze out: it has a sentence to show when opened.
    IM_CHECK(!row1.squeeze_sentences.empty());
    if (row1.squeeze_sentences.empty()) return;
    // Every row starts closed: no chord, sentence or backend heading shows.
    IM_CHECK(!on_screen(h, row1.chord));
    for (const hydra::app::TextLine& s : row1.squeeze_sentences) IM_CHECK(!on_screen(h, s.text));
    for (const hydra::app::ActivationRowView& a : acts) IM_CHECK(!on_screen(h, a.backends_label));
    IM_CHECK(!ctx->ItemExists("**/Show in Preview >##showact1"));
    // A click opens row 1: its chord, sentences and backend heading show; the
    // other rows stay shut.
    ctx->ItemClick("**/##act1");
    ctx->Yield(2);
    IM_CHECK(on_screen(h, row1.chord));
    for (const hydra::app::TextLine& s : row1.squeeze_sentences) IM_CHECK(on_screen(h, s.text));
    IM_CHECK(on_screen(h, row1.backends_label));
    for (size_t i = 1; i < acts.size(); ++i)
        if (acts[i].backends_label != row1.backends_label)
            IM_CHECK(!on_screen(h, acts[i].backends_label));
    IM_CHECK(!on_screen(h, "SqOut: Note timing"));
    IM_CHECK(!on_screen(h, "Frontend:"));
    // The timeline outlines follow each row's badges; Burnout's squeeze out
    // is a warning, so an orange outline is among them. The open record gets
    // a length as an input: ten minutes, past either chart's last activation.
    h.app->viewed.song_length_ms = kMarksSongLengthMs;
    ctx->Yield(2);
    IM_CHECK(has_outline(shown_activations(*h.app), Outline::Orange));
    check_marks(ctx, *h.app);
    // Hovering row 1's mark names the activation and every one of its badges:
    // each of them shows once more than before the hover.
    {
        const std::vector<DrawnMark> marks = drawn_marks(ctx);
        IM_CHECK(!marks.empty());
        if (marks.empty()) return;
        std::vector<std::string> named = {row1.measure};
        for (const hydra::app::ActivationBadge& b : row1.badges) named.push_back(b.text);
        std::vector<size_t> before;
        for (const std::string& s : named) before.push_back(count_on_screen(h, s));
        ctx->MouseMoveToPos(ImVec2(marks[0].x, marks[0].y));
        ctx->Yield(2);
        for (size_t k = 0; k < named.size(); ++k)
            IM_CHECK_GT(count_on_screen(h, named[k]), before[k]);
    }

    // "Show in Preview" asks the Preview for activation 1 (the Preview tab
    // reads preview_jump) and switches to the Preview tab, which starts the
    // Preview.
    ctx->ItemClick("**/Show in Preview >##showact1");
    IM_CHECK(h.app->details_ui.paths_tab.ui().preview_jump == std::optional<size_t>(0));
    IM_CHECK(wait_until(ctx, [&] { return h.app->preview && h.app->preview->active(); }, 10));
    IM_CHECK(wait_until(ctx, [&] { return !h.app->preview->loading(); }, 120));
    h.app->details_ui.paths_tab.ui().preview_jump.reset();
    ctx->ItemClick("##DetailsTabs/Paths");
    ctx->Yield(2);

    // Row 2 opens and row 1 closes. The two headings are told apart only
    // where their texts differ.
    const hydra::app::ActivationRowView& row2 = acts[1];
    const bool labels_differ = row2.backends_label != row1.backends_label;
    ctx->ItemClick("**/##act2");  // opens row 2, closes row 1
    ctx->Yield(2);
    IM_CHECK(on_screen(h, row2.backends_label));
    if (labels_differ) IM_CHECK(!on_screen(h, row1.backends_label));
    ctx->ItemClick("**/##act2");  // closes it again
    ctx->Yield(2);
    IM_CHECK(!on_screen(h, row2.backends_label));

    ctx->ItemClick("**/Expand all");
    ctx->Yield(2);
    for (const hydra::app::ActivationRowView& a : acts) IM_CHECK(on_screen(h, a.backends_label));
    IM_CHECK(h.app->details_ui.paths_tab.ui().all_open());
    ctx->ItemClick("**/Collapse all");
    ctx->Yield(2);
    for (const hydra::app::ActivationRowView& a : acts) IM_CHECK(!on_screen(h, a.backends_label));

    // Beg (Evans Blue): its optimal path has timing badges that are no
    // warning, so a grey outline is among its marks.
    open_titled(ctx, "evans blue", "Beg");
    if (ctx->IsError()) return;
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    ctx->Yield(2);
    h.app->viewed.song_length_ms = kMarksSongLengthMs;
    ctx->Yield(2);
    const hydra::app::ActivationsView& beg = shown_activations(*h.app);
    IM_CHECK(has_outline(beg, Outline::Grey));
    check_marks(ctx, *h.app);
    for (const hydra::app::ActivationRowView& a : beg.acts)
        for (const hydra::app::ActivationBadge& b : a.badges) IM_CHECK(on_screen(h, b.text));
}

// The Rating cell of activation 1's squeezed-out row that costs points, the
// one row build_activations warn-colours, as the view the tab drew holds it.
// Empty when the row has no such row.
std::string costly_sqout_rating(hydra::ui::AppState& app) {
    const hydra::app::ActivationsView& view = shown_activations(app);
    if (view.acts.empty()) return "";
    for (const hydra::app::BackendRowView& row : view.acts[0].backends)
        if (row.warn) return row.rating;
    return "";
}

// The backend table folds per activation, and the Backend limit on the Paths
// tab hides rows from it. Every expected text is the activations view's.
void test_paths_backend_timings(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_burnout(ctx)) return;
    // Burnout's activation 1 squeezes out a row that costs points.
    const std::string sqout = costly_sqout_rating(*h.app);
    IM_CHECK(!sqout.empty());
    if (sqout.empty()) return;
    const std::string label_all = shown_activations(*h.app).acts[0].backends_label;
    const size_t rows_all = shown_activations(*h.app).acts[0].backends.size();
    // Row 1 opens with its backend table open; its fold closes, reopens and
    // closes it, and the table stays closed when the row is shut and reopened.
    ctx->ItemClick("**/##act1");
    ctx->Yield(2);
    IM_CHECK(h.app->details_ui.paths_tab.ui().backends_open[0] == 1);
    IM_CHECK(on_screen(h, label_all));  // the fold's heading
    IM_CHECK(on_screen(h, sqout));
    ctx->ItemClick("**/Backend timings##act1");
    ctx->Yield(2);
    IM_CHECK(h.app->details_ui.paths_tab.ui().backends_open[0] == 0);
    IM_CHECK(!on_screen(h, sqout));
    ctx->ItemClick("**/Backend timings##act1");
    ctx->Yield(2);
    IM_CHECK(h.app->details_ui.paths_tab.ui().backends_open[0] == 1);
    IM_CHECK(on_screen(h, sqout));
    ctx->ItemClick("**/Backend timings##act1");
    ctx->Yield(2);
    IM_CHECK(h.app->details_ui.paths_tab.ui().backends_open[0] == 0);
    ctx->ItemClick("**/##act1");  // shut the row, then open it again
    ctx->Yield(2);
    ctx->ItemClick("**/##act1");
    ctx->Yield(2);
    IM_CHECK(h.app->details_ui.paths_tab.ui().backends_open[0] == 0);
    IM_CHECK(!on_screen(h, sqout));
    // The rest of this test reads the table, so it opens again.
    ctx->ItemClick("**/Backend timings##act1");
    ctx->Yield(2);
    IM_CHECK(h.app->details_ui.paths_tab.ui().backends_open[0] == 1);
    IM_CHECK(on_screen(h, sqout));
    // The table has no explanation paragraph above it.
    IM_CHECK(!on_screen(h, "Timing is how far each note sits"));

    // Off by default, at the default limit Settings holds, and the number box
    // is inert until it is ticked.
    IM_CHECK(!h.app->settings.backendlimit_enabled);
    IM_CHECK_EQ(h.app->settings.backendlimit_value, hydra::app::Settings{}.backendlimit_value);
    IM_CHECK((ctx->ItemInfo("**/##backendlimitvalue").ItemFlags & ImGuiItemFlags_Disabled) != 0);
    ctx->ItemClick("**/Hide backend rows beyond##backendlimit");
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.backendlimit_enabled; }, 5));
    IM_CHECK(hydra::app::Settings::load_file(h.ini_path).backendlimit_enabled);

    // At 30 ms Burnout's rows far from the SP end go and the squeezed-out row
    // stays. The heading and the line under the table say what was hidden,
    // as build_activations words it from the rows it hid.
    ctx->ItemInputValue("**/##backendlimitvalue", 30);
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.backendlimit_value == 30; }, 5));
    IM_CHECK_EQ(hydra::app::Settings::load_file(h.ini_path).backendlimit_value, 30);
    ctx->Yield(2);
    std::string hidden_line;
    {
        const hydra::app::ActivationRowView& limited = shown_activations(*h.app).acts[0];
        hidden_line = limited.backends_hidden_line;
        IM_CHECK(!hidden_line.empty());
        IM_CHECK_LT(limited.backends.size(), rows_all);
        IM_CHECK(on_screen(h, limited.backends_label));
        IM_CHECK(on_screen(h, hidden_line));
    }
    IM_CHECK_STR_EQ(costly_sqout_rating(*h.app).c_str(), sqout.c_str());
    IM_CHECK(on_screen(h, sqout));

    // The limit reaches the engine's whole squeeze window (kSqueezeWindowMs)
    // and no further: the box clamps a larger value as Settings::clamp does,
    // back to the window.
    const int window = static_cast<int>(hydra::kSqueezeWindowMs);
    ctx->ItemInputValue("**/##backendlimitvalue", window);
    ctx->Yield(2);
    IM_CHECK_EQ(h.app->settings.backendlimit_value, window);
    ctx->ItemInputValue("**/##backendlimitvalue", window + 100);
    ctx->Yield(2);
    IM_CHECK_EQ(hydra::app::Settings::clamp(&hydra::app::Settings::backendlimit_value, window + 100),
                window);
    IM_CHECK_EQ(h.app->settings.backendlimit_value, window);

    // Display only: the record is still the analyzed one. Unticking restores
    // every row and persists too.
    IM_CHECK(h.app->viewed.ready());
    IM_CHECK(!h.app->view_job);  // the backend limit never re-analyzes
    ctx->ItemClick("**/Hide backend rows beyond##backendlimit");
    IM_CHECK(wait_until(ctx, [&] { return !h.app->settings.backendlimit_enabled; }, 5));
    IM_CHECK(!hydra::app::Settings::load_file(h.ini_path).backendlimit_enabled);
    ctx->Yield(2);
    IM_CHECK_EQ(shown_activations(*h.app).acts[0].backends.size(), rows_all);
    IM_CHECK(on_screen(h, label_all));
    IM_CHECK(!on_screen(h, hidden_line));
}

// The two folds under the list, and Copy path with its "Copied!" flash.
void test_paths_folds_copy(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_burnout(ctx)) return;
    FakeClipboard clipboard;

    // Both folds start open; a click closes each and another reopens it.
    // Burnout's record has a multiplier squeeze, so its fold has rows; the
    // texts are the squeeze views the tab drew (app::build_multsqueezes' form).
    IM_CHECK(h.app->details_ui.paths_tab.ui().mult_open);
    IM_CHECK(h.app->details_ui.paths_tab.ui().breakdown_open);
    const hydra::app::PathsTabCache::Details* built = h.app->details_ui.paths_tab.built_details();
    IM_CHECK(built != nullptr && !built->squeezes.empty());
    if (built == nullptr || built->squeezes.empty()) return;
    const std::vector<hydra::app::MultSqueezeView> squeezes = built->squeezes;
    // beside the Multiplier squeeze fold
    IM_CHECK(on_screen(h, hydra::app::multsqueeze_summary(squeezes)));
    for (const hydra::app::MultSqueezeView& sq : squeezes) {
        IM_CHECK(on_screen(h, sq.label));
        IM_CHECK(on_screen(h, sq.howto));
    }
    ctx->ItemClick("**/Multiplier squeeze##mult");
    ctx->Yield(2);
    IM_CHECK(!on_screen(h, squeezes[0].howto));
    ctx->ItemClick("**/Multiplier squeeze##mult");
    ctx->Yield(2);
    IM_CHECK(on_screen(h, squeezes[0].howto));

    IM_CHECK(on_screen(h, "Total Score:"));
    IM_CHECK(on_screen(h, std::string(hydra::app::kAverageMultiplierLabel) + ":"));
    ctx->ItemClick("**/Score breakdown##breakdown");
    ctx->Yield(2);
    IM_CHECK(!on_screen(h, "Total Score:"));
    ctx->ItemClick("**/Score breakdown##breakdown");
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "Total Score:"));

    IM_CHECK(!on_screen(h, "Copied!"));
    ctx->ItemClick("**/Copy path");
    ctx->Yield(2);
    const hydra::HydraRecord& rec = *h.app->viewed.record;
    IM_CHECK_STR_EQ(FakeClipboard::text().c_str(),
                    rec.best_path().pathstring_verbose(rec.multsqueezes).c_str());
    IM_CHECK(on_screen(h, "Copied!"));

    // Ctrl+C goes through the same call: it copies again and still flashes.
    FakeClipboard::text().clear();
    ctx->KeyPress(ImGuiMod_Ctrl | ImGuiKey_C);
    ctx->Yield(2);
    IM_CHECK_STR_EQ(FakeClipboard::text().c_str(),
                    rec.best_path().pathstring_verbose(rec.multsqueezes).c_str());
    IM_CHECK(on_screen(h, "Copied!"));

    // The folds keep the user's last choice on the next song: closed here,
    // still closed there.
    ctx->ItemClick("**/Multiplier squeeze##mult");
    ctx->ItemClick("**/Score breakdown##breakdown");
    ctx->Yield(2);
    IM_CHECK(!h.app->details_ui.paths_tab.ui().mult_open);
    IM_CHECK(!h.app->details_ui.paths_tab.ui().breakdown_open);
    const int burnout_generation = h.app->record_generation.n;
    open_titled(ctx, "tapestry", "Tapestry of the Starless Abstract (Shortened)");
    if (ctx->IsError()) return;
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    ctx->Yield(2);
    // Another song's analysis has landed: the open record is a new one.
    IM_CHECK(h.app->record_generation.n != burnout_generation);
    IM_CHECK(h.app->viewed.ready());
    IM_CHECK(!h.app->details_ui.paths_tab.ui().mult_open);
    IM_CHECK(!h.app->details_ui.paths_tab.ui().breakdown_open);
    IM_CHECK(!on_screen(h, "Total Score:"));
}

// Every squeezed-out backend row on Tapestry's optimal path, against the
// engine's own answer for it: a row the engine does not count
// (core::counted_without_squeeze) costs nothing, so it is not warn-coloured,
// and its Rating starts with the engine's word for it (summarystr, which adds
// the "(uncounted)" tag); a counted one is warn-coloured. Each row's Rating and
// each squeeze sentence is on screen. The path has at least one uncounted
// squeezed-out row, the case this test is for.
void test_paths_uncounted(ImGuiTestContext* ctx) {
    static const char* kTitle = "Tapestry of the Starless Abstract (Shortened)";
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    open_titled(ctx, "tapestry", kTitle);
    if (ctx->IsError()) return;
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    ctx->ItemClick("**/Expand all");
    ctx->Yield(2);
    // With no backend limit every rated row is shown, in the engine's order.
    IM_CHECK(!h.app->settings.backend_limit().has_value());
    const hydra::Path* path = h.app->details_ui.selected_path;
    IM_CHECK(path != nullptr);
    if (path == nullptr) return;
    const double leeway = h.app->settings.rules.backend_leeway_ms;
    const hydra::app::ActivationsView& view = shown_activations(*h.app);
    const hydra::ActivationWalk acts = path->walk_activations();
    IM_CHECK_EQ(acts.size(), view.acts.size());
    if (acts.size() != view.acts.size()) return;
    // Every opened row shows its backend table with no further click.
    size_t uncounted = 0;
    for (size_t i = 0; i < acts.size(); ++i) {
        const hydra::ActivationRating rate = hydra::rate_activation(acts[i], leeway);
        const hydra::app::ActivationRowView& av = view.acts[i];
        IM_CHECK_EQ(av.backends.size(), rate.backends.size());
        if (av.backends.size() != rate.backends.size()) return;
        for (size_t r = 0; r < rate.backends.size(); ++r) {
            const hydra::BackendRating& br = rate.backends[r];
            if (!br.squeezed_out) continue;
            const bool counted = hydra::core::counted_without_squeeze(br.row.offset(), leeway);
            const hydra::app::BackendRowView& row = av.backends[r];
            const std::string word = br.row.summarystr(true, br.note.effective_ms, leeway);
            ctx->LogInfo("activation %d squeezed-out row: counted %d, rating \"%s\"", av.number,
                         counted, row.rating.c_str());
            IM_CHECK_EQ(row.rating.rfind(word, 0), static_cast<size_t>(0));
            IM_CHECK_EQ(row.warn, counted);
            IM_CHECK(on_screen(h, row.rating));
            if (!counted) ++uncounted;
        }
        for (const hydra::app::TextLine& s : av.squeeze_sentences) IM_CHECK(on_screen(h, s.text));
    }
    IM_CHECK_GT(uncounted, static_cast<size_t>(0));
    IM_CHECK(!on_screen(h, "costs no points"));
    IM_CHECK(!on_screen(h, "banks for later"));
}

// At the panel's narrowest the Paths tab still fits: with every row, every
// backend table, both folds under the rows and "Copied!" showing, nothing in
// the right column runs past its edge, and the path list keeps at least
// kMinPathListW (Burnout's optimal row, with its timing beside it, is a little
// wider than that).
void test_paths_fit_narrow(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_burnout(ctx)) return;
    // Copy path writes the system clipboard, as it does for the player.
    KeptClipboardText kept_clipboard;
    hydra::ui::remember_library_share(0.99f);  // the library as wide as it goes
    h.app->library_ui.panel_was_open = false;   // the split sets its width next frame
    ctx->Yield(3);
    ImGuiWindow* panel = ctx->WindowInfo("//Hydra/##songpanel").Window;
    IM_CHECK_FLOAT_NEAR_EQ(panel->Size.x, hydra::ui::px(hydra::ui::kMinSongPanelW), 1.0f);

    ctx->ItemClick("**/Expand all");  // every row
    ctx->Yield(1);
    open_paths_folds(ctx);  // every backend table and both folds
    ctx->ItemClick("**/Copy path");
    ctx->Yield(2);
    // The flash is up: Copy path started it, and it lasts kCopiedSeconds.
    const double copied_at = h.app->details_ui.copied_at;
    IM_CHECK_GE(copied_at, 0.0);
    IM_CHECK_LT(ImGui::GetTime() - copied_at, hydra::ui::AppState::kCopiedSeconds);
    ImGuiWindow* details = window_named("##pathdetails");
    ImGuiWindow* list = window_named("##pathlist");
    IM_CHECK(details != nullptr && list != nullptr);
    if (ctx->IsError()) return;
    IM_CHECK_LE(details->ContentSize.x, details->ContentRegionRect.GetWidth() + 0.5f);
    IM_CHECK_GE(list->Size.x, hydra::ui::px(hydra::ui::kMinPathListW) - 0.5f);
}

// The scratch library's longest path (9 activations): the path list takes at
// most its share of the tab, the activations get the rest, and nothing in
// either column runs past its edge (a long title wraps instead).
void test_paths_long_path(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    open_titled(ctx, "spiraling void", "The Spiraling Void");
    if (ctx->IsError()) return;
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    ctx->Yield(2);
    ctx->ItemClick("**/##act1");  // row 1 with its backend table, and both folds
    ctx->Yield(2);
    open_paths_folds(ctx);
    for (float share : {0.01f, 0.99f}) {  // the widest panel, then the narrowest
        hydra::ui::remember_library_share(share);
        h.app->library_ui.panel_was_open = false;
        ctx->Yield(3);
        ImGuiWindow* list = window_named("##pathlist");
        ImGuiWindow* details = window_named("##pathdetails");
        IM_CHECK(list != nullptr && details != nullptr);
        if (ctx->IsError()) return;
        const float tab = details->Pos.x + details->Size.x - list->Pos.x;
        IM_CHECK_LE(list->Size.x, (std::max)(hydra::ui::px(hydra::ui::kMinPathListW),
                                             tab * hydra::ui::kMaxPathListShare) + 1.0f);
        IM_CHECK_GE(details->Size.x, hydra::ui::px(hydra::ui::kMinPathDetailsW) - 1.0f);
        IM_CHECK_LE(list->ContentSize.x, list->ContentRegionRect.GetWidth() + 0.5f);
        IM_CHECK_LE(details->ContentSize.x, details->ContentRegionRect.GetWidth() + 0.5f);
    }
}

// The narrowest song panel: the library as wide as it goes.
bool narrowest_panel(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    hydra::ui::remember_library_share(0.99f);
    h.app->library_ui.panel_was_open = false;  // the split sets its width next frame
    ctx->Yield(3);
    ImGuiWindow* panel = ctx->WindowInfo("//Hydra/##songpanel").Window;
    IM_CHECK_RETV(panel != nullptr, false);
    IM_CHECK_RETV(std::fabs(panel->Size.x - hydra::ui::px(hydra::ui::kMinSongPanelW)) <= 1.0f,
                  false);
    return true;
}

// The one backend table drawn this frame: the only four-column table in the
// details column. Null when there is none, or more than one (more than one
// activation row is open).
ImGuiTable* backend_table() {
    ImGuiContext& g = *ImGui::GetCurrentContext();
    ImGuiWindow* details = window_named("##pathdetails");
    if (!details) return nullptr;
    ImGuiTable* found = nullptr;
    for (int n = 0; n < g.Tables.GetMapSize(); ++n) {
        ImGuiTable* t = g.Tables.TryGetMapData(n);
        if (!t || t->ColumnsCount != 4 || t->LastFrameActive < g.FrameCount - 2) continue;
        if (t->OuterWindow != details) continue;
        if (found) return nullptr;
        found = t;
    }
    return found;
}

// At the narrowest panel, Burnout's first backend table: Timing, Chord and
// Points are as wide as their text, and the squeezed-out rating, the longest
// line, wraps inside its Rating cell instead of running past it.
void test_paths_backend_fit(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_burnout(ctx)) return;
    if (!narrowest_panel(ctx)) return;
    ctx->ItemClick("**/##act1");  // row 1 alone, so its table is the only one
    ctx->Yield(3);
    // Burnout's squeezed-out row, the table's longest Rating line.
    const std::string sqout = costly_sqout_rating(*h.app);
    IM_CHECK(!sqout.empty());
    IM_CHECK(on_screen(h, sqout));
    ImGuiTable* t = backend_table();
    IM_CHECK(t != nullptr);
    if (ctx->IsError()) return;
    // Rating's content never reaches past its cell's right edge.
    const ImGuiTableColumn& rating = t->Columns[3];
    const float content_max = (std::max)(rating.ContentMaxXUnfrozen, rating.ContentMaxXFrozen);
    ctx->LogInfo("rating column %.1f..%.1f, content to %.1f", rating.WorkMinX, rating.WorkMaxX,
                 content_max);
    IM_CHECK_LE(content_max, rating.WorkMaxX + 0.5f);
    // The squeezed-out line is longer than the cell, so it did wrap: the
    // cell is narrower than the line.
    const float line_w = hydra::ui::text_size(hydra::ui::TextRole::Figure, sqout.c_str()).x;
    IM_CHECK_LT(rating.WorkMaxX - rating.WorkMinX, line_w);
    // The three fixed columns fit their text: no cell runs past its column,
    // and each is as wide as the width rule gives it for the rows the tab
    // drew, its header measured as a Label and its cells in the role the tab
    // draws them in (text_role.h).
    const hydra::app::PathsTabCache::Details* d = h.app->details_ui.paths_tab.built_details();
    IM_CHECK(d != nullptr && !d->activations.acts.empty());
    if (ctx->IsError()) return;
    const std::vector<hydra::app::BackendRowView>& rows = d->activations.acts[0].backends;
    struct FixedColumn {
        const char* header;
        hydra::ui::TextRole role;
        std::string hydra::app::BackendRowView::*cell;
    };
    const FixedColumn fixed[] = {
        {"Timing", hydra::ui::TextRole::Figure, &hydra::app::BackendRowView::timing},
        {"Chord", hydra::ui::TextRole::Notation, &hydra::app::BackendRowView::chord},
        {"Points", hydra::ui::TextRole::Figure, &hydra::app::BackendRowView::points}};
    for (int c = 0; c < 3; ++c) {
        const ImGuiTableColumn& col = t->Columns[c];
        IM_CHECK_LE((std::max)(col.ContentMaxXUnfrozen, col.ContentMaxXFrozen), col.WorkMaxX + 0.5f);
        std::vector<hydra::ui::ColumnSpec> spec = {
            {fixed[c].header, false, 0.0f, hydra::ui::measure_in_role(fixed[c].role)}};
        spec[0].header_width_of = hydra::ui::measure_in_role(hydra::ui::TextRole::Label);
        std::string hydra::app::BackendRowView::*cell = fixed[c].cell;
        const float rule_w =
            hydra::ui::place_columns(
                hydra::ui::measure_widths(spec, rows.size(),
                                          [&](size_t r, size_t) { return rows[r].*cell; }),
                spec, hydra::ui::TableRoom{})
                .widths[0];
        ctx->LogInfo("column %s drawn %.1f, rule %.1f", fixed[c].header, col.WidthGiven, rule_w);
        IM_CHECK_EQ(col.WidthGiven, rule_w);
    }
    // And the table stays inside the details column.
    ImGuiWindow* details = window_named("##pathdetails");
    IM_CHECK_LE(details->ContentSize.x, details->ContentRegionRect.GetWidth() + 0.5f);
    ctx->LogInfo("fixed widths %.1f %.1f %.1f", t->Columns[0].WidthGiven, t->Columns[1].WidthGiven,
                 t->Columns[2].WidthGiven);
}

// The pills the Paths tab drew last frame inside `band` (one row's rect), in
// the window whose name holds `window`, read from the vertices of their fill
// colour `fill` (the shared pill's unless the caller looks for a Dynamic Hit
// Window tag's): each pill's rect, left to right and top to bottom. Pills are
// the only shapes in the shared fill on a row. A timing box's own background
// is that colour too, so in a box the caller tells it from a pill.
std::vector<ImRect> drawn_pills(const ImRect& band, const char* window = "##pathdetails",
                                ImU32 fill = hydra::ui::pill_u32(hydra::ui::kPillFill)) {
    std::vector<ImRect> pills;
    ImGuiWindow* details = window_named(window);
    if (!details) return pills;
    // A filled rounded rect is one run of vertices, its anti-aliased fringe
    // interleaved in the same colour at zero alpha; a vertex of any other
    // colour ends the run.
    const ImU32 fringe = fill & ~IM_COL32_A_MASK;
    ImRect cur;
    bool open = false;
    for (const ImDrawVert& v : details->DrawList->VtxBuffer) {
        if (open && v.col == fringe) continue;
        const bool in = v.col == fill && band.Contains(v.pos);
        if (in) {
            if (!open) cur = ImRect(v.pos, v.pos);
            cur.Add(v.pos);
            open = true;
        } else if (open) {
            pills.push_back(cur);
            open = false;
        }
    }
    if (open) pills.push_back(cur);
    return pills;
}

// How much narrower a drawn_pills rect is than the pill it fills: the solid
// vertices of an anti-aliased fill sit half a fringe inside each edge
// (ImDrawList::AddConvexPolyFilled), so one fringe across the width.
float drawn_pill_inset() {
    ImGuiWindow* details = window_named("##pathdetails");
    if (!details) return 0.0f;
    const ImDrawList* dl = details->DrawList;
    if (!(dl->Flags & ImDrawListFlags_AntiAliasedFill)) return 0.0f;
    return dl->_FringeScale;
}

// Every glyph drawn inside `band` comes from the Figure role's font (the mono
// font, D257): each character of `texts` was drawn with that font's glyph and
// none with the Prose font's. A glyph is told by its place in the font atlas,
// the uv of its top-left vertex.
bool row_text_all_mono(ImGuiTestContext* ctx, const ImRect& band,
                       const std::vector<std::string>& texts) {
    ImGuiWindow* details = window_named("##pathdetails");
    if (!details) return false;
    const float size = drawn_font_size();
    ImFontBaked* mono = hydra::ui::font_for(hydra::ui::TextRole::Figure).font->GetFontBaked(size);
    ImFontBaked* main_baked =
        hydra::ui::font_for(hydra::ui::TextRole::Prose).font->GetFontBaked(size);
    auto drawn = [&](const ImFontGlyph* g) {
        for (const ImDrawVert& v : details->DrawList->VtxBuffer)
            if (band.Contains(v.pos) && v.uv.x == g->U0 && v.uv.y == g->V0) return true;
        return false;
    };
    bool ok = true;
    for (const std::string& t : texts) {
        for (const char c : t) {
            if (c == ' ') continue;
            const ImWchar code = static_cast<unsigned char>(c);
            const ImFontGlyph* m = mono->FindGlyph(code);
            const ImFontGlyph* d = main_baked->FindGlyph(code);
            if (!m || !drawn(m) || (d && drawn(d))) {
                ctx->LogInfo("'%c' of \"%s\" is not drawn in the mono font alone", c, t.c_str());
                ok = false;
            }
        }
    }
    return ok;
}

// The activation rows at the narrowest panel: the measure, the bars and the
// badges never overlap, on Burnout's own rows and on the longest pieces real
// charts have (an 11-character measure, a two-digit bar count) and the
// longest badge a row can show (longest_activation_badge). Every piece of a
// row is drawn and measured in the mono font (D257).
void test_paths_row_layout(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_burnout(ctx)) return;
    if (!narrowest_panel(ctx)) return;
    const float row_w = ctx->ItemInfo("**/##act1").RectFull.GetWidth();
    IM_CHECK_GT(row_w, 0.0f);
    const float scale = hydra::ui::px(1.0f);
    const float gap = hydra::ui::px(hydra::ui::kRowBarsGap);
    // The row is one Figure scope: its layout measures and gets its line
    // height in that role.
    auto figure_w = [](const std::string& s) {
        return hydra::ui::text_size(hydra::ui::TextRole::Figure, s.c_str()).x;
    };
    float line = 0.0f;
    {
        hydra::ui::ScopedFont font(hydra::ui::TextRole::Figure);
        line = ImGui::GetTextLineHeight();
    }

    // Every pill is sized for the longest badge, as the Paths tab sizes it.
    const std::string longest = hydra::app::longest_activation_badge();
    const float pill_text_w = figure_w(longest);

    // Leaves the layout in `l`, so a caller can see where the badges went
    // (IM_CHECK returns early, so the lambda can't return it).
    hydra::ui::ActivationRowLayout l;
    auto check = [&](const std::string& widest_measure, const std::string& measure,
                     const std::string& bars, const std::vector<std::string>& badges) {
        std::vector<float> badge_ws;
        for (const std::string& b : badges) badge_ws.push_back(figure_w(b));
        l = hydra::ui::activation_row_layout(figure_w(widest_measure), figure_w(bars), badge_ws,
                                             pill_text_w, row_w, line, scale);
        const float measure_end = l.measure_x + figure_w(measure);
        const float bars_end = l.bars_x + figure_w(bars);
        ctx->LogInfo("measure %s ends %.1f, bars at %.1f..%.1f, %d badges, pill from %.1f, "
                     "%d lines under the measure (row %.1f)",
                     measure.c_str(), measure_end, l.bars_x, bars_end, static_cast<int>(badges.size()),
                     l.badge_pill_min, l.badge_lines, row_w);
        IM_CHECK_LE(measure_end + gap, l.bars_x + 0.01f);
        IM_CHECK_GE(l.bars_x, hydra::ui::px(hydra::ui::kRowMinBarsX));
        // On the row's own line the pills start after the bars end.
        if (!badges.empty() && l.badge_lines == 0) IM_CHECK_LT(bars_end, l.badge_pill_min);
    };
    auto texts = [](const hydra::app::ActivationRowView& a) {
        std::vector<std::string> out;
        for (const hydra::app::ActivationBadge& b : a.badges) out.push_back(b.text);
        return out;
    };
    const hydra::app::ActivationsView& view = shown_activations(*h.app);
    std::string widest;
    for (const hydra::app::ActivationRowView& a : view.acts)
        if (figure_w(a.measure) > figure_w(widest)) widest = a.measure;
    IM_CHECK(!view.acts.empty());
    if (view.acts.empty()) return;
    for (const hydra::app::ActivationRowView& a : view.acts)
        check(widest, a.measure, a.bars, texts(a));
    // One, two and three of the longest badge beside the longest pieces. In
    // the mono font even one of them reaches the bars at the narrowest panel,
    // so all three cases go under the measure.
    check("m1024.1.120", "m1024.1.120", "12 bars", {longest});
    check("m1024.1.120", "m1024.1.120", "12 bars", {longest, longest});
    check("m1024.1.120", "m1024.1.120", "12 bars", {longest, longest, longest});
    IM_CHECK_GE(l.badge_lines, 2);  // three don't fit on one line under the measure

    // The drawn row: Burnout's row 1 keeps its one-line height while its badge
    // fits, all in the mono font.
    const ImGuiStyle& style = ImGui::GetStyle();
    const ImRect row1_rect = ctx->ItemInfo("**/##act1").RectFull;
    const float one_line = row1_rect.GetHeight();
    check(widest, view.acts[0].measure, view.acts[0].bars, texts(view.acts[0]));
    IM_CHECK_EQ(l.badge_lines, 0);
    // The drawn row is as tall as the layout's row; the Selectable's rect
    // reaches half the item spacing past its size on each side.
    IM_CHECK_EQ(one_line, l.row_h + style.ItemSpacing.y);
    {
        std::vector<std::string> pieces = texts(view.acts[0]);
        pieces.push_back(std::to_string(view.acts[0].number));
        pieces.push_back(view.acts[0].notation);
        pieces.push_back(view.acts[0].measure);
        pieces.push_back(view.acts[0].bars);
        IM_CHECK(row_text_all_mono(ctx, row1_rect, pieces));
    }
    // Three badges of different lengths, the longest among them, on row 1 of
    // the cached view the tab draws. The cache is only rebuilt when its
    // inputs change, so they stay for the frames below and are put back after.
    hydra::app::PathsTabCache::Details* built = const_cast<hydra::app::PathsTabCache::Details*>(
        h.app->details_ui.paths_tab.built_details());
    IM_CHECK(built != nullptr);
    if (!built) return;
    hydra::app::ActivationRowView& row1 = built->activations.acts[0];
    const std::vector<hydra::app::ActivationBadge> kept = row1.badges;
    row1.badges = {hydra::app::ActivationBadge{"early fill 0.0 ms", false},
                   hydra::app::ActivationBadge{longest, false},
                   hydra::app::ActivationBadge{"squeeze in -224.0 ms", false}};
    check(widest, row1.measure, row1.bars, texts(row1));
    ctx->Yield(2);
    const ImRect grown_rect = ctx->ItemInfo("**/##act1").RectFull;
    // The taller row can bring in the list's scroll bar, which narrows every
    // row, so the expected layout is the one at the width the row now has.
    std::vector<float> three_ws;
    for (const std::string& b : texts(row1)) three_ws.push_back(figure_w(b));
    const hydra::ui::ActivationRowLayout three = hydra::ui::activation_row_layout(
        figure_w(widest), figure_w(row1.bars), three_ws, pill_text_w, grown_rect.GetWidth(), line,
        scale);
    const std::vector<ImRect> pills = drawn_pills(grown_rect);
    ctx->LogInfo("row 1 height %.1f with its own badge, %.1f with three (%d lines under the "
                 "measure, %d pills drawn, row %.1f wide, %.1f before)",
                 one_line, grown_rect.GetHeight(), three.badge_lines, static_cast<int>(pills.size()),
                 grown_rect.GetWidth(), row_w);
    IM_CHECK_GE(three.badge_lines, 1);
    IM_CHECK_EQ(grown_rect.GetHeight(), three.row_h + style.ItemSpacing.y);
    IM_CHECK_EQ(static_cast<int>(pills.size()), 3);
    // The row's top, less the half spacing, to its first line's text is the
    // top padding; the bars' text bottom sits one line below that.
    const float row_top = grown_rect.Min.y + style.ItemSpacing.y * 0.5f;
    const float row_bottom = grown_rect.Max.y - style.ItemSpacing.y * 0.5f;
    const float top_pad = three.text_y;
    const float bars_bottom = row_top + three.text_y + line;
    for (const ImRect& p : pills) {
        ctx->LogInfo("pill %.1f,%.1f .. %.1f,%.1f", p.Min.x, p.Min.y, p.Max.x, p.Max.y);
        // Under the measure, clear of the first line by the top padding, inside
        // the row with that margin below, and no further right than a
        // flush-right pill.
        IM_CHECK_GE(p.Min.x + 0.5f, grown_rect.Min.x + three.measure_x - hydra::ui::px(hydra::ui::kRowBadgePad));
        IM_CHECK_GE(p.Min.y - bars_bottom + 0.5f, top_pad);
        IM_CHECK_GE(row_bottom - p.Max.y + 0.5f, top_pad);
        IM_CHECK_LE(p.Max.x, grown_rect.Max.x - hydra::ui::px(hydra::ui::kRowBadgeRight) +
                                 hydra::ui::px(hydra::ui::kRowBadgePad) + 0.5f);
    }
    // One width: every drawn pill is as wide as the layout's pill, and pills
    // on different lines share both edges.
    const float inset = drawn_pill_inset();
    ctx->LogInfo("row pill width %.3f (longest badge text %.3f), drawn %.3f, fill inset %.3f",
                 three.pill_w, pill_text_w, pills[0].GetWidth(), inset);
    for (const ImRect& p : pills) {
        IM_CHECK_LE(std::fabs(p.GetWidth() - pills[0].GetWidth()), 0.01f);
        IM_CHECK_LE(std::fabs(p.GetWidth() + inset - three.pill_w), 0.01f);
        if (p.Min.y != pills[0].Min.y) {
            IM_CHECK_LE(std::fabs(p.Min.x - pills[0].Min.x), 0.01f);
            IM_CHECK_LE(std::fabs(p.Max.x - pills[0].Max.x), 0.01f);
        }
    }
    // Each badge's text is centred in its pill: the text's middle is the
    // pill's middle. The text's left edge is its first glyph's leftmost vertex
    // less that glyph's offset in the font; its right edge is that plus the
    // text's measured width. ImFont::RenderText floors the text position, so
    // the text may sit up to a pixel left of the exact middle.
    {
        ImGuiWindow* details = window_named("##pathdetails");
        IM_CHECK(details != nullptr);
        if (!details) return;
        ImFontBaked* mono = hydra::ui::font_for(hydra::ui::TextRole::Figure)
                                .font->GetFontBaked(drawn_font_size());
        const ImU32 dim = ImGui::GetColorU32(ImGuiCol_TextDisabled);
        for (size_t k = 0; k < pills.size(); ++k) {
            const std::string& t = row1.badges[k].text;
            ImRect band = pills[k];
            band.Expand(inset + 1.0f);
            float xmin = FLT_MAX;
            for (const ImDrawVert& v : details->DrawList->VtxBuffer)
                if (v.col == dim && band.Contains(v.pos)) xmin = (std::min)(xmin, v.pos.x);
            const ImFontGlyph* first = mono->FindGlyph(static_cast<unsigned char>(t[0]));
            IM_CHECK(first != nullptr && xmin != FLT_MAX);
            if (!first || xmin == FLT_MAX) return;
            const float text_x = xmin - first->X0;
            const float text_end = text_x + figure_w(t);
            const float left = text_x - pills[k].Min.x;
            const float right = pills[k].Max.x - text_end;
            ctx->LogInfo("badge \"%s\" text %.2f..%.2f in pill %.2f..%.2f: %.2f left, %.2f right",
                         t.c_str(), text_x, text_end, pills[k].Min.x, pills[k].Max.x, left, right);
            const float off = (text_x + text_end) * 0.5f - pills[k].GetCenter().x;
            IM_CHECK_LE(off, 0.01f);
            IM_CHECK_GE(off, -1.0f);
        }
    }
    row1.badges = kept;
    ctx->Yield(2);
    IM_CHECK_EQ(ctx->ItemInfo("**/##act1").RectFull.GetHeight(), one_line);
}

// The GUI test library has no audio, but the click gives Burnout a length
// from its chart, so the timeline places a mark for every activation, each
// inside the song. The click works the length out with its analysis, so no
// other job reads the chart for it, and the length stays as it was while the
// song stays open.
void test_paths_length_from_click(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_burnout(ctx)) return;
    hydra::ui::AppState& app = *h.app;
    IM_CHECK(app.viewed.ready());
    IM_CHECK(app.viewed.song_length_ms.has_value());
    if (!app.viewed.song_length_ms) return;
    const double length = *app.viewed.song_length_ms;
    IM_CHECK_GT(length, 0.0);
    IM_CHECK(!app.view_job);
    ctx->Yield(2);
    // Every activation falls inside the song, so each has its place on the
    // timeline and its own mark.
    const hydra::app::ActivationsView& view = shown_activations(app);
    IM_CHECK(!view.acts.empty());
    for (const hydra::app::ActivationRowView& a : view.acts) {
        IM_CHECK(a.song_fraction.has_value());
        if (!a.song_fraction) return;
        IM_CHECK_GE(*a.song_fraction, 0.0);
        IM_CHECK_LE(*a.song_fraction, 1.0);
    }
    IM_CHECK_EQ(drawn_marks(ctx).size(), view.acts.size());
    const hydra::HydraRecord* record = &*app.viewed.record;

    // Nothing reads it again while the song stays open.
    ctx->Yield(5);
    IM_CHECK(!app.view_job);
    IM_CHECK(&*app.viewed.record == record);
    IM_CHECK(app.viewed.song_length_ms == length);
}

// The pieces of Tapped Out (Mike Orlando)'s optimal path, activation 4: its
// squeeze out, which the game's windows cannot cover. Literals from T0's run;
// tests/test_activation_rows.cpp pins the same row's text.
constexpr const char* kTappedOutWindowPieces[] = {"Dynamic Hit Window:", "frontend 77.9 ms [K   G]",
                                                  "backend 72.0 ms [ R B ]", "65.9 ms short"};

// Opens Tapped Out in the app already running and waits for its Paths tab.
bool open_tapped_out(ImGuiTestContext* ctx) {
    open_titled(ctx, "tapped out", "Tapped Out");
    if (ctx->IsError()) return false;
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return false;
    ctx->Yield(2);
    return true;
}

// Tapped Out's activation 4 row fits inside its box: the box's content is
// no wider than the box (the box's ##sq<act>_<k> child, its one squeeze).
void check_tapped_out_box_fits() {
    ImGuiWindow* box = window_named("##sq4_0");
    IM_CHECK(box != nullptr);
    if (!box) return;
    IM_CHECK_LE(box->ContentSize.x, box->ContentRegionRect.GetWidth() + 0.5f);
}

// The pieces a Dynamic Hit Window row draws, from the row view the tab drew
// (app::WindowRowView): the label, each end's word when it has one, its
// window and its chord, and the tag.
std::vector<std::string> window_pieces(const hydra::app::WindowRowView& w) {
    std::vector<std::string> out = {hydra::app::kDynamicHitWindowLabel};
    for (const hydra::app::WindowFigure& f : w.figures)
        out.push_back((f.word.empty() ? std::string() : f.word + " ") + f.figure + " " + f.chord);
    out.push_back(w.tag);
    return out;
}

// Whether an activation's opened row holds any Dynamic Hit Window row.
bool has_window_row(const hydra::app::ActivationRowView& a) {
    if (a.early_fill_window) return true;
    for (const std::optional<hydra::app::WindowRowView>& w : a.squeeze_windows)
        if (w) return true;
    return false;
}

// An early fill's Dynamic Hit Window row on screen: its pieces, and no
// squeeze word (core::role_word) before its window, since its one note is
// neither end of a squeeze.
void check_early_fill_window(Harness& h, const hydra::app::WindowRowView& w) {
    for (const std::string& piece : window_pieces(w)) IM_CHECK(on_screen(h, piece));
    const std::string front = hydra::role_word(hydra::EndRole::Frontend);
    const std::string back = hydra::role_word(hydra::EndRole::Backend);
    for (const hydra::app::WindowFigure& f : w.figures) {
        IM_CHECK(!on_screen(h, front + " " + f.figure));
        IM_CHECK(!on_screen(h, back + " " + f.figure));
    }
}

// The Dynamic Hit Window row: on Burnout's squeeze out, on no activation
// without a timing badge, and on Tapped Out's squeeze out and both its early
// fills, each in a box. Every expected text is the row view the tab drew.
void test_paths_hit_window(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_burnout(ctx)) return;
    IM_CHECK(!on_screen(h, hydra::app::kDynamicHitWindowLabel));  // every row starts closed

    // Burnout's activation 1 has a squeeze with a window row.
    ctx->ItemClick("**/##act1");
    ctx->Yield(2);
    const hydra::app::ActivationsView& burnout = shown_activations(*h.app);
    IM_CHECK_GE(burnout.acts.size(), static_cast<size_t>(2));
    if (burnout.acts.size() < 2) return;
    size_t k1 = burnout.acts[0].squeeze_windows.size();
    for (size_t k = 0; k < burnout.acts[0].squeeze_windows.size(); ++k)
        if (burnout.acts[0].squeeze_windows[k] && k1 == burnout.acts[0].squeeze_windows.size()) k1 = k;
    IM_CHECK(k1 < burnout.acts[0].squeeze_windows.size());
    if (k1 >= burnout.acts[0].squeeze_windows.size()) return;
    const hydra::app::WindowRowView w1 = *burnout.acts[0].squeeze_windows[k1];
    for (const std::string& piece : window_pieces(w1)) IM_CHECK(on_screen(h, piece));
    // The tag is a badge: its pill, in the squeeze's box, is as wide as the
    // pill of the activation row's badge above it, and painted in its own
    // tag's fill (window_tag_pill), not the shared pill's.
    {
        const std::string box_id = "##sq1_" + std::to_string(k1);
        const std::vector<ImRect> row_pills = drawn_pills(ctx->ItemInfo("**/##act1").RectFull);
        ImGuiWindow* box = window_named(box_id.c_str());
        IM_CHECK(box != nullptr);
        IM_CHECK_EQ(row_pills.size(), burnout.acts[0].badges.size());
        IM_CHECK(!row_pills.empty());
        if (!box || row_pills.empty()) return;
        // The box's own background, in the shared fill, starts at its top;
        // nothing below it is in the shared fill. Fails if the tag's pill is
        // painted in the shared fill.
        int shared_below_top = 0;
        for (const ImRect& p : drawn_pills(box->Rect(), box_id.c_str()))
            if (p.Min.y > box->Pos.y + 2.0f) ++shared_below_top;
        IM_CHECK_EQ(shared_below_top, 0);
        // Fails if the tag is not drawn in its own tag's fill.
        const std::vector<ImRect> tags =
            drawn_pills(box->Rect(), box_id.c_str(), hydra::ui::window_tag_pill(w1.is_short).fill);
        IM_CHECK_EQ(static_cast<int>(tags.size()), 1);
        if (tags.size() != 1) return;
        ctx->LogInfo("row badge pill %.3f wide, tag pill %.3f wide", row_pills[0].GetWidth(),
                     tags[0].GetWidth());
        IM_CHECK_LE(std::fabs(tags[0].GetWidth() - row_pills[0].GetWidth()), 0.01f);
        IM_CHECK_LE(std::fabs(tags[0].GetHeight() - row_pills[0].GetHeight()), 0.01f);
    }

    // Activation 2 has no window row in the view, and none shows.
    IM_CHECK(!has_window_row(burnout.acts[1]));
    ctx->ItemClick("**/##act2");  // opens row 2, closes row 1
    ctx->Yield(2);
    {
        std::vector<char> want(h.app->details_ui.paths_tab.ui().act_open.size(), 0);
        want[1] = 1;
        IM_CHECK(h.app->details_ui.paths_tab.ui().act_open == want);
    }
    IM_CHECK(!on_screen(h, hydra::app::kDynamicHitWindowLabel));

    // Tapped Out's activation 4 has a squeeze with a window row and an early
    // fill with its own box and row; activation 3 has an early fill with its
    // own box and row.
    if (!open_tapped_out(ctx)) return;
    const std::vector<hydra::app::ActivationRowView> tapped = shown_activations(*h.app).acts;
    IM_CHECK_GE(tapped.size(), static_cast<size_t>(4));
    if (tapped.size() < 4) return;
    const hydra::app::ActivationRowView& act4 = tapped[3];
    const hydra::app::ActivationRowView& act3 = tapped[2];
    IM_CHECK(!act4.squeeze_windows.empty() && act4.squeeze_windows[0].has_value());
    IM_CHECK(!act4.early_fill.empty() && act4.early_fill_window.has_value());
    IM_CHECK(!act3.early_fill.empty() && act3.early_fill_window.has_value());
    if (ctx->IsError()) return;
    ctx->ItemClick("**/##act4");
    ctx->Yield(2);
    for (const std::string& piece : window_pieces(*act4.squeeze_windows[0]))
        IM_CHECK(on_screen(h, piece));
    IM_CHECK(on_screen(h, act4.early_fill));
    IM_CHECK(window_named("##ef4") != nullptr);
    check_early_fill_window(h, *act4.early_fill_window);
    if (ctx->IsError()) return;
    check_tapped_out_box_fits();
    if (ctx->IsError()) return;

    ctx->ItemClick("**/##act3");  // opens row 3, closes row 4
    ctx->Yield(2);
    IM_CHECK(on_screen(h, act3.early_fill));
    IM_CHECK(window_named("##ef3") != nullptr);
    check_early_fill_window(h, *act3.early_fill_window);
}

// At the narrowest panel, all four pieces of Tapped Out's activation 4 row
// are still on screen, and the row stays inside its box.
void test_paths_hit_window_narrow(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    if (!open_tapped_out(ctx)) return;
    if (!narrowest_panel(ctx)) return;
    ctx->ItemClick("**/##act4");
    ctx->Yield(3);
    for (const char* piece : kTappedOutWindowPieces) IM_CHECK(on_screen(h, piece));
    check_tapped_out_box_fits();
}

}  // namespace

// Registers this file's tests; register_tests() (uitest_tests.cpp) calls it.
void register_paths_tests(Harness& h) {
    struct Entry {
        const char* name;
        void (*fn)(ImGuiTestContext*);
    };
    const Entry entries[] = {
        {"paths-list", test_paths_list},
        {"paths-rows", test_paths_rows},
        {"paths-backend-timings", test_paths_backend_timings},
        {"paths-folds-copy", test_paths_folds_copy},
        {"paths-uncounted", test_paths_uncounted},
        {"paths-fit-narrow", test_paths_fit_narrow},
        {"paths-long-path", test_paths_long_path},
        {"paths-backend-fit", test_paths_backend_fit},
        {"paths-row-layout", test_paths_row_layout},
        {"paths-length-from-click", test_paths_length_from_click},
        {"paths-hit-window", test_paths_hit_window},
        {"paths-hit-window-narrow", test_paths_hit_window_narrow},
    };
    for (const Entry& e : entries) {
        ImGuiTest* t = IM_REGISTER_TEST(h.engine, "hydra", e.name);
        t->UserData = &h;
        t->TestFunc = e.fn;
    }
}

}  // namespace uitest
