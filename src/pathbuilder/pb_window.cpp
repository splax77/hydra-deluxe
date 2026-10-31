#include "pathbuilder/pb_window.h"

#include <algorithm>
#include <cfloat>
#include <cstdint>
#include <exception>
#include <optional>
#include <string>
#include <utility>

#include "imgui.h"
#include "imgui_internal.h"  // ImFontCalcTextSizeEx, CalcWrapWidthForPos, the window's text cursor

#include "app/config.h"          // chartmode_key
#include "app/display_format.h"  // format_cost, kFactSeparator
#include "app/path_costs.h"      // end_costs
#include "app/preview_view.h"    // path_overlay_key, fill_index_on_chord
#include "app/report.h"          // ChipToken
#include "app/user_messages.h"
#include "core/timing.h"  // measure_label
#include "pathbuilder/pb_files.h"
#include "pathbuilder/pb_menu.h"
#include "pathbuilder/pb_open.h"
#include "search/graph.h"  // fill_rule_for
#include "ui/preview_controller.h"
#include "ui/preview_panel.h"
#include "ui/preview_timeline.h"
#include "ui/text_role.h"
#include "ui/theme.h"    // chip_color, kAccentColor, kStarPowerColor
#include "ui/widgets.h"  // WarnColor, error_detail_line

namespace hydra::pathbuilder {

namespace {

// The second mockup's sizes, each in rem: a multiple of the font size
// (rem() below), as the user's answer to plan Q1 chose. Each names its line
// of docs/superpowers/specs/2026-10-31-path-builder-mockup-2/mockup_template.html.
// The side column beside the Preview and the gap before it: line 91,
// `.lower{grid-template-columns:minmax(0,1fr) 27rem;gap:.6rem}`.
constexpr float kSideColumnRem = 27.0f;
constexpr float kLowerGapRem = 0.6f;
// How wide the hover tooltip may grow before it wraps: line 87, `.tip{max-width:24rem}`.
constexpr float kTooltipRem = 24.0f;
// The space between the legend's items: line 48, `.legend{gap:1.1rem}`.
constexpr float kLegendGapRem = 1.1f;
// How tall the list may grow before it scrolls: line 126, `.choices{max-height:21rem}`.
constexpr float kChoicesMaxRem = 21.0f;
// A list row's tier dot: line 129, `.ch .dot{width:.7rem}`.
constexpr float kTierDotRem = 0.7f;
// A step's numbered disc: line 136, `.step .n{width:1.6rem}`.
constexpr float kStepDiscRem = 1.6f;

float rem(float n) { return n * ImGui::GetFontSize(); }

// What the window says when the file dialog never appeared: File > Open's,
// then Open path...'s or Save path...'s (a dropped file opens as a chart, so
// dragging is no way round for those).
constexpr const char* kDialogFailedText =
    "The file dialog didn't open. Drag a chart file onto this window instead.";
constexpr const char* kPathDialogFailedText =
    "The file dialog didn't open, so no path file was opened or saved.";

// The report chips the cost tiers wear (spec decision 3, plan Q1).
ImVec4 tier_color(app::CostTier tier) {
    using app::report::ChipToken;
    switch (tier) {
        case app::CostTier::Green: return ui::chip_color(ChipToken::t0);
        case app::CostTier::Yellow: return ui::chip_color(ChipToken::t1);
        case app::CostTier::Red: return ui::chip_color(ChipToken::t3);
    }
    return ui::chip_color(ChipToken::muted);
}

// `text` wrapped at the window's edge, with the Star Power star the mockup
// writes as "(★)" at byte `star_at` (pb_text's intro_star_at and
// GuideMessage::star_at), one text line tall. The sentence is drawn as one
// text item with no-break spaces where the star goes, so it wraps as one
// sentence; the star is drawn over that gap, found by ImGui's own wrap
// layout of the same text.
void text_with_star(const std::string& text, std::optional<size_t> star_at) {
    ui::ScopedFont prose(ui::TextRole::Prose);
    if (!star_at || *star_at > text.size()) {
        ImGui::TextWrapped("%s", text.c_str());
        return;
    }
    const float size = ImGui::GetTextLineHeight();
    constexpr const char* kNoBreakSpace = "\xC2\xA0";  // U+00A0, which ImGui never wraps at
    const float space_w = ImGui::CalcTextSize(kNoBreakSpace).x;
    std::string gap;
    float gap_w = 0.0f;
    while (space_w > 0.0f && gap_w < size) {
        gap += kNoBreakSpace;
        gap_w += space_w;
    }
    const std::string open = text.substr(0, *star_at) + kStarOpen;
    const std::string drawn = open + gap + kStarClose + text.substr(*star_at);

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const bool own_wrap = window->DC.TextWrapPos < 0.0f;  // as TextWrapped does
    if (own_wrap) ImGui::PushTextWrapPos(0.0f);
    const ImVec2 pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
    const float wrap_w = ImGui::CalcWrapWidthForPos(window->DC.CursorPos, window->DC.TextWrapPos);
    ImGui::TextUnformatted(drawn.c_str(), drawn.c_str() + drawn.size());
    if (own_wrap) ImGui::PopTextWrapPos();

    // Where the gap starts: the end of `open` in the wrapped layout, given
    // as the bottom of its line.
    ImVec2 end;
    ImFontCalcTextSizeEx(ui::font_for(ui::TextRole::Prose).font, ImGui::GetFontSize(), FLT_MAX, wrap_w, drawn.c_str(),
                         drawn.c_str() + open.size(), drawn.c_str() + drawn.size(), nullptr, &end,
                         0);
    const ImVec2 at(pos.x + end.x + (gap_w - size) * 0.5f, pos.y + end.y - ImGui::GetFontSize());
    ui::draw_star(ImGui::GetWindowDrawList(), at, size, true, ImGui::GetColorU32(ui::kStarPowerColor));
}

// Dim text that wraps at the window's edge, as the mockup's muted lines do.
void dim_wrapped(const std::string& text) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::TextWrapped("%s", text.c_str());
    ImGui::PopStyleColor();
}

// A figure that has not landed yet reads kPendingText.
std::string figure_or_pending(const std::optional<int64_t>& v) {
    return v ? group_thousands(*v) : std::string(kPendingText);
}

// The first candidate that costs nothing: the best next fill (candidate_costs
// gives the best one cost 0), or null while none has landed.
const app::CandidateCost* best_candidate(const std::vector<app::CandidateCost>& candidates) {
    for (const app::CandidateCost& c : candidates)
        if (app::costs_nothing(c.cost)) return &c;
    return nullptr;
}

// The name of the best next fill, once it has landed and the scene has it.
std::optional<std::string> best_fill_name(const app::PreviewScene& scene,
                                          const std::vector<app::CandidateCost>& candidates) {
    const app::CandidateCost* best = best_candidate(candidates);
    if (!best) return std::nullopt;
    const std::optional<size_t> i = app::fill_index_on_chord(scene, best->act_tick);
    if (!i) return std::nullopt;
    return fill_name(scene, scene.fills[*i]);
}

// The cost of a list row, a step or an end choice: "best" when it loses
// nothing, else the cost in full (format_cost).
std::string cost_or_best(int64_t cost) {
    return app::costs_nothing(cost) ? std::string(kBestFillLabel) : app::format_cost(cost);
}

// `text` at the right edge of the line the cursor is on, which starts at
// window x `line_x` and is `line_w` wide, in `col`.
void text_at_right(const std::string& text, float line_x, float line_w, const ImVec4& col) {
    ImGui::SameLine(line_x + line_w - ImGui::CalcTextSize(text.c_str()).x);
    ImGui::TextColored(col, "%s", text.c_str());
}

}  // namespace

PbApp::PbApp(ID3D11Device* device, ID3D11DeviceContext* context)
    : volume_pct_(app::Settings{}.preview_volume) {
    if (device && context) preview_ = std::make_unique<ui::PreviewController>(device, context);
}

PbApp::~PbApp() { close_preview(); }

void PbApp::open(const std::string& notespath) {
    // The one read of the settings file per open: the chart's settings and
    // the Preview's volume both come from it.
    const app::Settings settings = app::Settings::load();
    volume_pct_ = settings.preview_volume;
    state.open_with(notespath, settings);
}

bool PbApp::busy() const { return state.busy() || (preview_ && preview_->busy()); }

void PbApp::close_preview() {
    if (preview_) preview_->close();
}

const Path* PbApp::shown_path() const {
    const std::optional<Path>& mine = state.history().current().path;
    if (mine) return &*mine;
    const OpenChart* chart = state.chart();
    return chart && chart->empty_path ? &*chart->empty_path : nullptr;
}

std::optional<size_t> PbApp::selected_step() const {
    if (selected_generation_ != state.history().generation()) return std::nullopt;
    return selected_;
}

void PbApp::take_in_chart() {
    state.poll();
    const OpenChart* chart = state.chart();
    if (chart == chart_seen_) return;
    chart_seen_ = chart;
    set_refusal({}, {});
    guide_message_.reset();
    pending_guide_.reset();
    pending_select_.reset();
    selected_.reset();
    hovered_fill_.reset();
    row_hovered_fill_.reset();
    overlay_built_ = false;
}

void PbApp::set_refusal(std::string sentence, std::string detail) {
    refusal_ = std::move(sentence);
    refusal_detail_ = std::move(detail);
    refusal_generation_ = state.history().generation();
}

void PbApp::set_guide_message(GuideMessage m) {
    guide_message_ = std::move(m);
    guide_message_generation_ = state.history().generation();
}

void PbApp::start_over() {
    pending_guide_.reset();
    pending_select_.reset();
    state.history().edit(PlayerPath{});
    set_guide_message(started_over_message());
}

void PbApp::select_step(size_t index) {
    selected_ = index;
    selected_generation_ = state.history().generation();
    // End choices are offered on the last activation only (the user's answer Q6).
    if (index + 1 == state.history().current().windows.size()) state.decisions().select(index);
    if (preview_) preview_->seek_activation(index);
}

void PbApp::act_on_menu() {
    const OpenChart* chart = state.chart();
    PathHistory& h = state.history();
    // Copy and Save path read the player's own path, never the empty one.
    const std::optional<Path>& mine = h.current().path;
    // Paste and Open path... place windows on the chart's fills, so they need
    // a chart with Star Power (kNothingToBuildText says why there is none).
    const bool can_build = chart != nullptr && chart->has_star_power();
    MenuFlags flags;
    flags.can_undo = chart != nullptr && h.can_undo();
    flags.can_redo = chart != nullptr && h.can_redo();
    flags.can_start_over = can_build && !h.current().windows.empty();
    flags.can_open_path = can_build;
    flags.can_save_path = chart != nullptr && mine.has_value();
    flags.can_copy = chart != nullptr && mine.has_value();
    flags.can_paste = can_build;
    const MenuChoice choice = draw_menu(flags);
    if (choice != MenuChoice::None) {
        set_refusal({}, {});
        pending_guide_.reset();
        pending_select_.reset();
    }

    switch (choice) {
        case MenuChoice::Open: {
            bool failed = false;
            if (const std::optional<std::string> path = ask_for_chart(owner, &failed)) open(*path);
            dialog_failed_ = failed ? kDialogFailedText : nullptr;
            break;
        }
        case MenuChoice::OpenPath: {
            bool failed = false;
            if (const std::optional<std::string> file = ask_for_path_file(owner, &failed)) {
                if (const std::optional<Refusal> refused = open_path_file(h, *chart, *file))
                    set_refusal(refused->message, refused->detail);
            }
            dialog_failed_ = failed ? kPathDialogFailedText : nullptr;
            break;
        }
        case MenuChoice::SavePath: {
            bool failed = false;
            if (const std::optional<std::string> file = ask_where_to_save_path(owner, &failed)) {
                try {
                    save_path_file(*file, *mine, chart->song);
                } catch (const std::exception& e) {
                    set_refusal(app::plain_error(e), app::plain_error_detail(e));
                }
            }
            dialog_failed_ = failed ? kPathDialogFailedText : nullptr;
            break;
        }
        case MenuChoice::Copy:
            ImGui::SetClipboardText(copy_text(chart->record, *mine).c_str());
            break;
        case MenuChoice::Paste: {
            const char* text = ImGui::GetClipboardText();
            if (const std::optional<Refusal> refused = paste_path(h, *chart, text ? text : ""))
                set_refusal(refused->message, refused->detail);
            break;
        }
        case MenuChoice::Undo:
            h.undo();
            break;
        case MenuChoice::Redo:
            h.redo();
            break;
        case MenuChoice::StartOver:
            start_over();
            break;
        case MenuChoice::None:
            break;
    }
}

void PbApp::draw() {
    take_in_chart();

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    constexpr ImGuiWindowFlags kFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoSavedSettings |
                                        ImGuiWindowFlags_MenuBar |
                                        ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin(kMainWindowName, nullptr, kFlags);
    act_on_menu();

    if (dialog_failed_) {
        ui::WarnColor warn;
        ImGui::TextWrapped("%s", dialog_failed_);
    }
    // A refusal shows as an open's error does, until the player's path changes.
    if (!refusal_.empty() && refusal_generation_ == state.history().generation()) {
        {
            ui::WarnColor warn;
            ImGui::TextWrapped("%s", refusal_.c_str());
        }
        ui::error_detail_line(refusal_detail_);
    }
    if (state.opening()) ImGui::TextWrapped("Opening %s ...", state.requested_path().c_str());
    if (!state.error().empty()) {
        {
            ui::WarnColor warn;
            ImGui::TextWrapped("%s", state.error().c_str());
        }
        ui::error_detail_line(state.error_detail());
    }

    if (const OpenChart* chart = state.chart())
        draw_body(*chart);
    else if (!state.opening() && state.error().empty())
        ImGui::TextWrapped("Open a chart with File > Open, or drag one onto this window.");
    ImGui::End();
}

bool PbApp::open_preview(const OpenChart& chart, const Path* shown) {
    if (!preview_) return false;
    // An edit after an undo can put the new path at the old one's address,
    // so the history's generation is compared as well as the pointer.
    const uint64_t generation = state.history().generation();
    if (!overlay_built_ || shown != overlay_for_ || generation != overlay_generation_) {
        overlay_key_ = app::path_overlay_key(shown);
        overlay_for_ = shown;
        overlay_generation_ = generation;
        overlay_built_ = true;
    }
    const app::AnalysisSettings& s = chart.settings;
    preview_->set_volume(volume_pct_);  // before the audio exists too, as Hydra's tab does
    preview_->open(chart.entry, s.reading(), fill_rule_for(s.legacy_fills), shown, overlay_key_,
                   s.sp_cap, s.rules);
    // Hydra's best path as faint outlines, when the player asks for it; the
    // controller does nothing when it already has this one.
    preview_->set_ghost_path(show_best_ && chart.has_star_power() ? &chart.record.best_path()
                                                                  : nullptr);
    preview_->poll();
    // The scene is the shown path's once its overlay is in.
    return !preview_->has_error() && !preview_->loading() && preview_->shows_path(overlay_key_);
}

void PbApp::draw_body(const OpenChart& chart) {
    const Path* shown = shown_path();
    if (!chart.has_star_power()) {
        draw_head(chart);
        ImGui::TextWrapped("%s", kNothingToBuildText);
        open_preview(chart, shown);
        draw_preview(chart);
        return;
    }

    DecisionList& d = state.decisions();
    d.show(state.search_graph(), chart.settings, shown,
           "player " + std::to_string(state.history().generation()));
    if (open_preview(chart, shown)) d.price_fills(preview_->scene().trailing_skipped_fill_ticks);
    if (d.take_player_changed()) {
        if (pending_guide_) set_guide_message(std::move(*pending_guide_));
        if (pending_select_) select_step(*pending_select_);
        pending_guide_.reset();
        pending_select_.reset();
    }

    draw_head(chart);
    text_with_star(intro_text(), intro_star_at());
    draw_guide(chart);
    draw_legend(chart);
    draw_timeline(chart);

    if (!ImGui::GetIO().WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Escape, false))
        selected_.reset();

    // Under the timeline: the Preview panel takes the width the side column
    // leaves (the user's answer 4).
    const float side_w = rem(kSideColumnRem);
    const float gap = rem(kLowerGapRem);
    ImGui::BeginChild("##preview", ImVec2((std::max)(1.0f, ImGui::GetContentRegionAvail().x - side_w - gap), 0));
    draw_preview(chart);
    ImGui::EndChild();
    ImGui::SameLine(0.0f, gap);
    ImGui::BeginChild("##side", ImVec2(0, 0));
    draw_choices(chart);
    ImGui::Spacing();
    ImGui::BeginChild("##steps", ImVec2(0, 0), ImGuiChildFlags_Borders);
    draw_steps(chart);
    ImGui::EndChild();
    ImGui::EndChild();
}

void PbApp::draw_head(const OpenChart& chart) {
    ImGui::BeginGroup();
    {
        ui::ScopedFont title(ui::TextRole::Title);
        ImGui::TextUnformatted(chart_title_line(chart).c_str());
    }
    ImGui::TextDisabled("%s", app::chartmode_key(chart.settings.reading()).c_str());
    ImGui::EndGroup();
    if (!chart.has_star_power()) return;

    const app::LadderRun* run = state.decisions().run();
    const bool complete = run && run->complete;
    const std::optional<int64_t> mine =
        complete ? std::optional<int64_t>(run->ladder.player_total) : std::nullopt;
    const std::optional<int64_t> best =
        run && !run->ladder.bf.empty() ? std::optional<int64_t>(run->ladder.bf.front()) : std::nullopt;
    const std::optional<int64_t> reach =
        complete ? std::optional<int64_t>(run->ladder.bf.back()) : std::nullopt;

    // The mockup's order, each hint the tooltip on its figure (plan Q9). The
    // player's own score is the big figure; the two best ones are dim.
    const char* labels[] = {kYourScoreLabel, kBestReachLabel, kBestPossibleLabel};
    const char* hints[] = {kYourScoreHint, kBestReachHint, kBestPossibleHint};
    const std::optional<int64_t> values[] = {mine, reach, best};
    for (size_t i = 0; i < 3; ++i) {
        ImGui::SameLine(0.0f, rem(kLegendGapRem));
        ImGui::BeginGroup();
        ImGui::TextDisabled("%s", labels[i]);
        if (i == 0) {
            ui::ScopedFont figure(ui::TextRole::Figure);
            ImGui::TextUnformatted(figure_or_pending(values[i]).c_str());
        } else {
            ImGui::TextDisabled("%s", figure_or_pending(values[i]).c_str());
        }
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(rem(kTooltipRem));
            ImGui::TextUnformatted(hints[i]);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
        ImGui::EndGroup();
    }

    PathHistory& h = state.history();
    DecisionList& d = state.decisions();
    ImGui::SameLine(0.0f, rem(kLegendGapRem));
    ImGui::BeginDisabled(!h.can_undo());
    if (ImGui::Button(kUndoLabel)) {
        pending_guide_.reset();
        pending_select_.reset();
        h.undo();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(h.current().windows.empty());
    if (ImGui::Button(kStartOverLabel)) start_over();
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!d.can_finish());
    if (ImGui::Button(kFinishForMeLabel)) {
        pending_guide_.reset();
        pending_select_.reset();
        const size_t kept = h.current().windows.size();
        const uint64_t before = h.generation();
        d.finish(state);
        if (h.generation() != before) set_guide_message(finished_for_player_message(kept));
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("%s", kFinishTooltip);
}

GuideFacts PbApp::guide_facts(const OpenChart& chart) const {
    GuideFacts f;
    const PathHistory& h = state.history();
    const DecisionList& d = state.decisions();
    if (guide_message_ && guide_message_generation_ == h.generation()) f.message = guide_message_;
    else if (!d.message().empty()) f.message = GuideMessage{d.message(), true};
    if (const app::LadderRun* run = d.run(); run && run->complete) f.player_total = run->ladder.player_total;
    if (!h.current().windows.empty() && h.current().path) {
        const ActivationWalk acts = h.current().path->walk_activations();
        if (acts.size() > 0)
            if (const std::optional<int64_t> end = acts[acts.size() - 1].deact_tick())
                f.last_end = measure_label(chart.song.timing().timecode(*end));
    }
    if (!preview_ || !preview_->shows_path(overlay_key_)) return f;
    const app::PreviewScene& scene = preview_->scene();
    f.fills_after_last = scene.trailing_skipped_fill_ticks.size();
    for (size_t i = 0; i < scene.fills.size(); ++i) {
        const PreviewFill& fill = scene.fills[i];
        if (fill.state != app::PreviewFillState::Offered || !fill_reason(scene, i).empty()) continue;
        f.earliest_fill = fill_name(scene, fill);
        f.earliest_ms = scene.timing->timecode(fill.chord_tick).ms();
        break;
    }
    f.best_fill = best_fill_name(scene, d.candidates());
    return f;
}

void PbApp::draw_guide(const OpenChart& chart) {
    const GuideFacts facts = guide_facts(chart);
    guide_ = guide_text(facts);
    // The guide's edge: the mockup's `.guide{border-left:4px solid var(--glow)}`,
    // red for a refusal (`.guide.refuse`).
    ImGui::PushStyleColor(ImGuiCol_Border, guide_.refuse ? ui::chip_color(app::report::ChipToken::t3)
                                                         : ui::kAccentColor);
    ImGui::BeginChild("##guide", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
    text_with_star(guide_.text, guide_.star_at);
    if (!guide_message_ && !state.decisions().message().empty())
        ui::error_detail_line(state.decisions().detail());
    const std::string error = state.decisions().error();
    if (!error.empty()) {
        {
            ui::WarnColor warn;
            ImGui::TextWrapped("%s", error.c_str());
        }
        ui::error_detail_line(state.decisions().error_detail());
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void PbApp::draw_legend(const OpenChart& chart) {
    struct Item {
        ui::LegendSwatch swatch;
        ImU32 col;
        const char* text;
    };
    const Item items[] = {
        {ui::LegendSwatch::Tinted, ImGui::GetColorU32(tier_color(app::CostTier::Green)), kLegendBestText},
        {ui::LegendSwatch::Tinted, ImGui::GetColorU32(tier_color(app::CostTier::Yellow)),
         kLegendUnderOnePercentText},
        {ui::LegendSwatch::Tinted, ImGui::GetColorU32(tier_color(app::CostTier::Red)), kLegendCostsMoreText},
        {ui::LegendSwatch::Offered, 0, kLegendCanText},
        {ui::LegendSwatch::Activation, 0, kLegendMineText},
        {ui::LegendSwatch::Cant, 0, kLegendCantText},
        {ui::LegendSwatch::Locked, 0, kLegendLockedText},
    };
    // The entries flow left to right and wrap at the window's edge, as the
    // mockup's `.legend{flex-wrap:wrap}` does.
    const float gap = rem(kLegendGapRem);
    const float swatch_w = ImGui::GetTextLineHeight();
    bool first = true;
    const auto next_entry = [&](float w) {
        if (!first && ui::fits_on_line(w, gap)) ImGui::SameLine(0.0f, gap);
        first = false;
    };
    for (const Item& item : items) {
        next_entry(swatch_w + ImGui::CalcTextSize(item.text).x);
        ImGui::BeginGroup();
        ui::legend_swatch(item.swatch, item.col);
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::TextDisabled("%s", item.text);
        ImGui::EndGroup();
    }
    next_entry(ImGui::CalcTextSize(kLegendSeekText).x);
    ImGui::TextDisabled("%s", kLegendSeekText);
    next_entry(ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x +
               ImGui::CalcTextSize(kShowBestPathLabel).x);
    ImGui::Checkbox(kShowBestPathLabel, &show_best_);
    (void)chart;
}

void PbApp::draw_timeline(const OpenChart& chart) {
    if (!preview_ || !preview_->shows_path(overlay_key_)) {
        if (preview_ && preview_->loading()) {
            const ui::PreviewController::LoadProgress p = preview_->load_progress();
            ImGui::ProgressBar(p.fraction, ImVec2(-1, 0), p.label.c_str());
        }
        return;
    }
    const app::PreviewScene& scene = preview_->scene();
    const DecisionList& d = state.decisions();

    ui::TimelineInput in;
    in.scene = &scene;
    in.length_ms = preview_->playback_end_ms();
    in.position_ms = preview_->position_ms();
    in.selected = selected_step();
    // Which look each fill gets is fill_refusal's kind: a fill before the
    // last activation is locked whatever its state, and an offered fill under
    // a running window is can't.
    for (size_t i = 0; i < scene.fills.size(); ++i) {
        const FillRefusal kind = fill_refusal(scene, i);
        if (kind == FillRefusal::BeforeLast)
            in.locked_ticks.push_back(scene.fills[i].chord_tick);
        else if (kind == FillRefusal::SpRunning && scene.fills[i].state == app::PreviewFillState::Offered)
            in.cant_ticks.push_back(scene.fills[i].chord_tick);
    }
    for (const app::CandidateCost& c : d.candidates()) {
        in.tints.push_back(ui::TimelineFillTint{c.act_tick, ImGui::GetColorU32(tier_color(c.tier))});
        if (app::costs_nothing(c.cost)) in.best_ticks.push_back(c.act_tick);
    }
    in.best_label = kBestFillLabel;
    const std::optional<size_t> row_hovered = row_hovered_fill_;
    row_hovered_fill_.reset();
    if (row_hovered && *row_hovered < scene.fills.size()) in.highlighted = row_hovered;

    const ui::TimelineEvents ev = ui::PreviewTimeline{}.draw(in);

    // While the pointer is on a fill, or on its row in the list, the highway
    // shows it, paused, with the fill's start on the strike line (the user's
    // answer Q4); when the pointer leaves, the Preview goes back to the time
    // and play state it had.
    const std::optional<size_t> hovered =
        ev.hovered_fill && *ev.hovered_fill < scene.fills.size() ? ev.hovered_fill : in.highlighted;
    if (hovered) {
        if (!hovered_fill_) {
            hover_return_ms_ = preview_->position_ms();
            hover_return_playing_ = preview_->playing();
            preview_->pause();
        }
        if (hovered_fill_ != hovered) preview_->seek_ms(scene.fills[*hovered].span.start_ms);
        hovered_fill_ = hovered;
    } else if (hovered_fill_) {
        hovered_fill_.reset();
        preview_->seek_ms(hover_return_ms_);
        if (hover_return_playing_) preview_->play();
    }

    // The fill's own tooltip, on the fill only; a list row shows none.
    if (ev.hovered_fill && *ev.hovered_fill < scene.fills.size()) {
        const size_t i = *ev.hovered_fill;
        std::optional<FillHoverNumbers> numbers;
        const std::vector<app::CandidateCost>& cands = d.candidates();
        const app::LadderRun* run = d.run();
        const auto mine = std::find_if(cands.begin(), cands.end(), [&](const app::CandidateCost& c) {
            return c.act_tick == scene.fills[i].chord_tick;
        });
        if (mine != cands.end() && best_candidate(cands) && run && run->complete) {
            FillHoverNumbers n;
            n.best_finish = mine->best_finish;
            n.best_now = run->ladder.bf.back();
            n.cost = mine->cost;
            n.best_fill = best_fill_name(scene, cands).value_or(std::string());
            n.hydra_end = d.candidate_end(static_cast<size_t>(mine - cands.begin()));
            numbers = n;
        }
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(rem(kTooltipRem));
        ImGui::TextUnformatted(fill_hover_text(scene, i, numbers).c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }

    if (ev.clicked_fill && *ev.clicked_fill < scene.fills.size()) {
        // A click keeps the highway where the hover put it.
        hover_return_ms_ = scene.fills[*ev.clicked_fill].span.start_ms;
        click_fill(chart, *ev.clicked_fill);
    } else if (ev.clicked_band) {
        select_step(*ev.clicked_band);
        hover_return_ms_ = preview_->position_ms();
    } else if (ev.clicked_ms) {
        preview_->seek_ms(*ev.clicked_ms);
    }
}

void PbApp::click_fill(const OpenChart& chart, size_t index) {
    if (!preview_ || !preview_->shows_path(overlay_key_)) return;
    const app::PreviewScene& scene = preview_->scene();
    if (index >= scene.fills.size()) return;
    const PreviewFill& f = scene.fills[index];
    if (f.state == app::PreviewFillState::Taken) {
        // The step standing on this fill is the one whose taken_fill is it.
        for (size_t k = 0; k < scene.activations.size(); ++k) {
            if (scene.activations[k].taken_fill != index) continue;
            select_step(k);
            break;
        }
        return;
    }
    FillRefusal kind = FillRefusal::None;
    const std::string reason = fill_reason(scene, index, kind);
    if (kind == FillRefusal::BeforeLast) {
        set_guide_message(locked_message(fill_name(scene, f)));
        return;
    }
    if (kind != FillRefusal::None && kind != FillRefusal::Taken) {
        set_guide_message(cant_activate_message(fill_name(scene, f), reason));
        return;
    }
    if (f.state != app::PreviewFillState::Offered) return;
    const PathHistory& h = state.history();
    pending_guide_.reset();
    pending_select_ = h.current().windows.size();
    guide_message_.reset();
    state.decisions().click_fill(state.search_graph(), chart.settings, h.current().windows,
                                 f.chord_tick, h.generation());
}

void PbApp::draw_preview(const OpenChart& chart) {
    if (!preview_) return;
    // The panel draws only a loaded controller, so its error is the
    // builder's to show.
    if (preview_->has_error()) {
        {
            ui::WarnColor warn;
            ImGui::TextWrapped("%s", preview_->error().c_str());
        }
        ui::error_detail_line(preview_->error_detail());
        return;
    }
    // Loading shows as the progress bar in draw_timeline.
    if (preview_->loading()) return;

    // Hydra's own Preview panel. The builder keeps the volume for the run and
    // never writes the settings file (plan Q2), so a let-go slider stores
    // nothing.
    ui::PreviewPanelState panel;
    panel.volume_pct = volume_pct_;
    ui::draw_preview_panel(*preview_, panel, {});
    if (panel.volume_changed) {
        volume_pct_ = app::Settings::clamp(&app::Settings::preview_volume, panel.volume_pct);
        preview_->set_volume(volume_pct_);
    }
    if (panel.image_click && chart.has_star_power()) {
        const ui::PreviewImageClick& c = *panel.image_click;
        if (const std::optional<size_t> fill = preview_->fill_at_image(c.at, c.width, c.height))
            click_fill(chart, *fill);
    }
}

void PbApp::draw_choices(const OpenChart& chart) {
    ImGui::TextUnformatted(kChoicesHeading);
    dim_wrapped(kChoicesHint);
    if (!preview_ || !preview_->shows_path(overlay_key_)) return;
    const app::PreviewScene& scene = preview_->scene();
    const DecisionList& d = state.decisions();
    const std::vector<app::CandidateCost>& cands = d.candidates();
    const std::vector<int64_t> order = choice_order(scene.trailing_skipped_fill_ticks, cands);
    if (order.empty()) {
        dim_wrapped(state.history().current().windows.empty() ? kNoChoicesText : kNoChoicesFinishedText);
        return;
    }

    // The list scrolls once it is taller than the mockup's limit.
    const float row_h = ImGui::GetTextLineHeightWithSpacing();
    const float list_h = (std::min)(rem(kChoicesMaxRem), row_h * static_cast<float>(order.size()) +
                                                              ImGui::GetStyle().WindowPadding.y * 2.0f);
    ImGui::BeginChild("##choices", ImVec2(0, list_h), ImGuiChildFlags_Borders);
    for (const int64_t tick : order) {
        const std::optional<size_t> fill = app::fill_index_on_chord(scene, tick);
        if (!fill) continue;
        const auto priced = std::find_if(cands.begin(), cands.end(),
                                         [&](const app::CandidateCost& c) { return c.act_tick == tick; });
        const bool landed = priced != cands.end();
        const Activation* end = landed ? d.candidate_end(static_cast<size_t>(priced - cands.begin())) : nullptr;

        // The row is one selectable; its pieces are drawn over it.
        const ImVec2 start = ImGui::GetCursorPos();
        const float row_w = ImGui::GetContentRegionAvail().x;
        ImGui::SetNextItemAllowOverlap();
        const std::string id = "##choice_" + std::to_string(tick);
        if (ImGui::Selectable(id.c_str(), hovered_fill_ == fill)) click_fill(chart, *fill);
        if (ImGui::IsItemHovered()) row_hovered_fill_ = fill;
        ImGui::SetCursorPos(start);

        // A priced row wears its tier's dot; a row still being priced wears
        // the timeline's mark for a fill it can activate on.
        const float dot_w = rem(kTierDotRem);
        ImGui::SetCursorPosY(start.y + (ImGui::GetTextLineHeight() - dot_w) * 0.5f);
        if (landed)
            ui::dot_item(ImGui::GetColorU32(tier_color(priced->tier)), dot_w);
        else
            ui::legend_swatch(ui::LegendSwatch::Offered);
        ImGui::SameLine();
        ImGui::SetCursorPosY(start.y);
        ImGui::TextUnformatted(fill_name(scene, scene.fills[*fill]).c_str());
        ImGui::SameLine();
        const std::string cost = landed ? cost_or_best(priced->cost) : std::string(kPendingText);
        const float cost_w = ImGui::CalcTextSize(cost.c_str()).x;
        const float middle_w = start.x + row_w - cost_w - ImGui::GetStyle().ItemSpacing.x - ImGui::GetCursorPosX();
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ui::text_ellipsized(candidate_row_text(scene, *fill, end).c_str(), (std::max)(0.0f, middle_w));
        ImGui::PopStyleColor();
        text_at_right(cost, start.x, row_w,
                      landed ? tier_color(priced->tier) : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    }
    ImGui::EndChild();
}

void PbApp::draw_steps(const OpenChart& chart) {
    ImGui::TextUnformatted(kStepsHeading);
    ImGui::Separator();
    const PathHistory& h = state.history();
    DecisionList& d = state.decisions();
    const std::vector<PinnedWindow>& windows = h.current().windows;
    if (windows.empty() || !h.current().path) {
        dim_wrapped(kNoStepsText);
        ImGui::Spacing();
        dim_wrapped(glossary_text());
        return;
    }
    const Path& path = *h.current().path;
    const ActivationWalk acts = path.walk_activations();
    const app::LadderRun* run = d.run();
    const CoachList& list = d.list();
    const std::optional<size_t> sel = selected_step();
    const ImVec4 violet = ui::chip_color(app::report::ChipToken::t5);

    for (size_t i = 0; i < acts.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        const bool last = i + 1 == acts.size();
        // Each step's three numbers come from one ladder: the cost cost_list
        // worked out (CoachRow::cost) and the best finishes either side of it.
        std::optional<StepText> text;
        if (i < list.rows.size() && list.rows[i].cost && run && run->ladder.bf.size() > i + 1)
            text = step_sentence(chart.song.timing(), acts[i], *list.rows[i].cost, run->ladder.bf[i],
                                 run->ladder.bf[i + 1], chart.settings.ms_filter);
        // The compact row: the step's number in a disc of the path's colour,
        // where it starts and ends, the phrases it spends, and its cost at
        // the right. The whole row is one selectable; its pieces are drawn
        // over it.
        const std::string number = std::to_string(i + 1);
        const StepShort brief = step_short(chart.song.timing(), acts[i]);
        const std::optional<int64_t> cost = i < list.rows.size() ? list.rows[i].cost : std::nullopt;
        const std::optional<app::CostTier> tier = i < list.rows.size() ? list.rows[i].tier : std::nullopt;
        const ImVec2 start = ImGui::GetCursorPos();
        const float row_w = ImGui::GetContentRegionAvail().x;
        const float disc_w = rem(kStepDiscRem);
        const float row_h = (std::max)(disc_w, ImGui::GetTextLineHeight());
        ImGui::SetNextItemAllowOverlap();
        if (ImGui::Selectable("##step", sel == i, 0, ImVec2(0, row_h))) {
            if (sel == i)
                selected_.reset();
            else
                select_step(i);
        }
        ImGui::SetCursorPos(start);
        ui::dot_item(ui::kTimelinePathColor, disc_w, number.c_str());
        const float text_y = start.y + (row_h - ImGui::GetTextLineHeight()) * 0.5f;
        ImGui::SameLine();
        ImGui::SetCursorPosY(text_y);
        ImGui::TextUnformatted(brief.span.c_str());
        ImGui::SameLine();
        ImGui::SetCursorPosY(text_y);
        ImGui::TextDisabled("%s", brief.saved.c_str());
        const std::string cost_label = cost ? cost_or_best(*cost) : std::string(kPendingText);
        text_at_right(cost_label, start.x, row_w,
                      tier ? tier_color(*tier) : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::SetCursorPosY(start.y + row_h + ImGui::GetStyle().ItemSpacing.y);

        // The selected step opens to its full sentence.
        if (sel == i) {
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextWrapped("%s", text ? text->sentence.c_str() : kPendingText);
            if (text) {
                dim_wrapped(text->why);
                if (!text->note.empty()) {
                    ImGui::PushStyleColor(ImGuiCol_Text, violet);
                    ImGui::TextWrapped("%s", text->note.c_str());
                    ImGui::PopStyleColor();
                }
            }
            ImGui::PopTextWrapPos();
        }

        if (sel == i) {
            if (last && d.selected() == i && d.ends().size() > 1) {
                ImGui::TextDisabled("%s", kEndItAtLabel);
                std::vector<int64_t> finishes;
                for (const EndChoice& c : d.ends()) finishes.push_back(c.best_finish);
                const std::vector<int64_t> costs = app::end_costs(finishes);
                for (size_t e = 0; e < d.ends().size(); ++e) {
                    const EndChoice& c = d.ends()[e];
                    std::string label = c.window.deact_tick
                                            ? measure_label(chart.song.timing().timecode(*c.window.deact_tick))
                                            : c.token;
                    label += app::kFactSeparator;
                    label += c.timing.empty() ? std::string(kNoTimingNeededText) : c.timing;
                    label += app::kFactSeparator;
                    label += cost_or_best(costs[e]);
                    ImGui::PushID(static_cast<int>(e));
                    if (ImGui::Button(label.c_str())) {
                        pending_guide_.reset();
                        pending_select_ = i;
                        d.choose_end(c.window, h.generation());
                    }
                    ImGui::PopID();
                }
            }
            if (ImGui::Button(last ? kRemoveLastLabel : kRemoveEarlierLabel)) {
                pending_select_.reset();
                pending_guide_ = removed_message(i, last);
                d.click_remove(state.search_graph(), chart.settings, windows, windows[i].act_tick,
                               h.generation());
            }
        }
        ImGui::Separator();
        ImGui::PopID();
    }
    if (list.after_last) {
        const std::string left = leftover_text(*list.after_last);
        if (*list.after_last > 0)
            ImGui::TextWrapped("%s", left.c_str());
        else
            ImGui::TextColored(tier_color(app::CostTier::Green), "%s", left.c_str());
    }
    if (!list.failure.empty()) {
        ui::WarnColor warn;
        ImGui::TextWrapped("%s", list.failure.c_str());
    }
    ImGui::Spacing();
    dim_wrapped(glossary_text());
}

}  // namespace hydra::pathbuilder
