#include "pathbuilder/pb_window.h"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <optional>
#include <string>
#include <utility>

#include "imgui.h"

#include "app/config.h"          // chartmode_key
#include "app/display_format.h"  // format_cost, kFactSeparator
#include "app/path_costs.h"      // end_costs
#include "app/preview_view.h"    // path_overlay_key, fill_index_on_chord
#include "app/report.h"          // ChipToken
#include "app/rules_file.h"      // RulesFileError
#include "app/user_messages.h"
#include "pathbuilder/pb_files.h"
#include "pathbuilder/pb_menu.h"
#include "pathbuilder/pb_open.h"
#include "pathbuilder/pb_text.h"
#include "search/graph.h"  // fill_rule_for
#include "ui/fonts.h"  // rem
#include "ui/list_scroll.h"  // scroll_block_into_view
#include "ui/preview_controller.h"
#include "ui/preview_panel.h"
#include "ui/preview_timeline.h"
#include "ui/text_role.h"
#include "ui/theme.h"    // chip_color
#include "ui/widgets.h"  // WarnColor, error_detail_line

namespace hydra::pathbuilder {

namespace {

// The second mockup's sizes, each in rem: a multiple of the font size
// (ui::rem), as the user's answer to plan Q1 chose. Each names its line
// of docs/superpowers/specs/2026-10-31-path-builder-mockup-2/mockup_template.html.
// The side column beside the Preview and the gap before it: line 91,
// `.lower{grid-template-columns:minmax(0,1fr) 27rem;gap:.6rem}`.
constexpr float kSideColumnRem = 27.0f;
constexpr float kLowerGapRem = 0.6f;
// The head row's gaps: line 34, `.head{...gap:2rem}` (before the scores and
// before the buttons), and line 37, `.nums{gap:1.6rem}` (between the scores).
constexpr float kHeadGapRem = 2.0f;
constexpr float kScoresGapRem = 1.6f;
// How wide the hover tooltip may grow before it wraps: line 87, `.tip{max-width:24rem}`.
constexpr float kTooltipRem = 24.0f;
// The gap before "Show Hydra's best path" in the head row:
// docs/superpowers/specs/2026-10-31-path-builder-mockup-5/top-mockup.html
// line 42, `.bestbox{margin-left:.8rem}`.
constexpr float kBestBoxGapRem = 0.8f;
// The gap between a fill card's labels and its figures:
// docs/superpowers/specs/2026-10-31-path-builder-mockup-3/mockup_template.html
// line 90, `.tip .kv{...column-gap:1.4rem}`.
constexpr float kCardColumnGapRem = 1.4f;
// How tall the list may grow before it scrolls: line 126, `.choices{max-height:21rem}`.
constexpr float kChoicesMaxRem = 21.0f;
// A list row's tier dot: line 129, `.ch .dot{width:.7rem}`.
constexpr float kTierDotRem = 0.7f;
// A step's numbered disc: line 136, `.step .n{width:1.6rem}`.
constexpr float kStepDiscRem = 1.6f;
// How strongly an earlier step draws an ending it did not keep:
// docs/superpowers/specs/2026-10-31-path-builder-mockup-4/step-panel-mockup.html
// line 30, `.end.fixed:not(.on){opacity:0.45}`.
constexpr float kFixedEndAlpha = 0.45f;

using ui::rem;

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

// One warning: the plain sentence in the warning colour, its details line
// under it.
void warning_line(const std::string& sentence, const std::string& detail) {
    {
        ui::WarnColor warn;
        ImGui::TextWrapped("%s", sentence.c_str());
    }
    ui::error_detail_line(detail);
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

// `text` at the right edge of the line the cursor is on, which starts at
// window x `line_x` and is `line_w` wide, in `col`.
void text_at_right(const std::string& text, float line_x, float line_w, const ImVec4& col) {
    ImGui::SameLine(line_x + line_w - ImGui::CalcTextSize(text.c_str()).x);
    ImGui::TextColored(col, "%s", text.c_str());
}

// A fill's tooltip, the words fill_hover_card gives: the heading, the rows as
// a two-column table of dim labels and their figures (the Cost figure in its
// tier's colour), then the grey lines, all inside kTooltipRem.
void draw_fill_card(const FillHoverCard& card) {
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(rem(kTooltipRem));
    ImGui::TextUnformatted(card.heading.c_str());
    if (!card.rows.empty()) {
        // Without borders a table puts one cell padding on each side of the
        // gap between its columns, so half the gap goes on each side. Only
        // the sideways padding changes.
        ImVec2 padding = ImGui::GetStyle().CellPadding;
        padding.x = rem(kCardColumnGapRem) * 0.5f;
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, padding);
        if (ImGui::BeginTable("##fillcard", 2, ImGuiTableFlags_SizingFixedFit)) {
            const ImVec4 text = ImGui::GetStyleColorVec4(ImGuiCol_Text);
            const ImVec4 dim = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
            for (const CardRow& row : card.rows) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextColored(dim, "%s", row.label.c_str());
                ImGui::TableNextColumn();
                const bool tinted = card.cost_tier && row.label == kCostLabel;
                ui::ScopedBaselineFont figure(ui::TextRole::Figure);
                ImGui::TextColored(tinted ? tier_color(*card.cost_tier) : text, "%s", row.value.c_str());
            }
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
    }
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    for (const std::string& line : card.grey) ImGui::TextUnformatted(line.c_str());
    ImGui::PopStyleColor();
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
}

// The "Your activations" label's tooltip: what each look of a fill means, one
// key per line (the fifth mockup's `title` on that label, line 553).
void draw_activations_hover() {
    struct Key {
        ui::LegendSwatch swatch;
        ImU32 col;
        const char* text;
    };
    const Key keys[] = {
        {ui::LegendSwatch::Tinted, ImGui::GetColorU32(tier_color(app::CostTier::Green)), kLegendBestText},
        {ui::LegendSwatch::Tinted, ImGui::GetColorU32(tier_color(app::CostTier::Yellow)),
         kLegendUnderOnePercentText},
        {ui::LegendSwatch::Tinted, ImGui::GetColorU32(tier_color(app::CostTier::Red)), kLegendCostsMoreText},
        {ui::LegendSwatch::Activation, 0, kLegendMineText},
        {ui::LegendSwatch::Cant, 0, kLegendCantText},
        {ui::LegendSwatch::Locked, 0, kLegendLockedText},
    };
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(rem(kTooltipRem));
    for (const Key& key : keys) {
        ui::legend_swatch(key.swatch, key.col);
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::TextDisabled("%s", key.text);
    }
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
}

}  // namespace

PbApp::PbApp(ID3D11Device* device, ID3D11DeviceContext* context)
    : volume_pct_(app::Settings{}.preview_volume) {
    if (device && context) preview_ = std::make_unique<ui::PreviewController>(device, context);
}

PbApp::~PbApp() { close_preview(); }

void PbApp::open(const std::string& notespath) {
    // The player never sees a path built on settings or rules they didn't
    // choose (D304 items 8 and 13). Nothing has started, and any song open
    // stays.
    const auto refuse = [this](const std::exception& e) {
        set_refusal(app::plain_error(e), app::plain_error_detail(e));
    };
    try {
        state.open(notespath);
    } catch (const app::SettingsFileError& e) {
        refuse(e);
        return;
    } catch (const app::RulesFileError& e) {
        refuse(e);
        return;
    }
    // The file read, so a refusal of its last read no longer holds.
    set_refusal({}, {});
    // The first open's settings give the run its volume; after that the
    // player's slider is the only thing that moves it (plan Q2).
    if (!volume_from_file_ && state.preview_volume()) {
        volume_pct_ = *state.preview_volume();
        volume_from_file_ = true;
    }
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
    pending_select_.reset();
    selected_.reset();
    hovered_fill_.reset();
    row_hovered_fill_.reset();
    hold_for_drag(false);
    view_.reset();
    overlay_built_ = false;
}

void PbApp::set_refusal(std::string sentence, std::string detail) {
    refusal_ = std::move(sentence);
    refusal_detail_ = std::move(detail);
    refusal_generation_ = state.history().generation();
}

void PbApp::start_over() {
    pending_select_.reset();
    state.history().edit(PlayerPath{});
}

void PbApp::select_step(size_t index) {
    selected_ = index;
    selected_generation_ = state.history().generation();
    steps_block_h_ = -1.0f;  // draw_steps scrolls the opened step into view
    // Every opened step works out its endings now, never ahead of time (the
    // fourth mockup's plan, "computed when opened"); draw_steps lets only the
    // last step's be chosen.
    state.decisions().select(index);
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
    // A refusal shows as an open's error does, until the player's path
    // changes. Each one here has its own reason and its own clearing, so when
    // two hold at once both show.
    const uint64_t generation = state.history().generation();
    if (!refusal_.empty() && refusal_generation_ == generation) warning_line(refusal_, refusal_detail_);
    const DecisionList& d = state.decisions();
    if (d.message_current(generation)) warning_line(d.message(), d.detail());
    if (const std::string error = d.error(); !error.empty()) warning_line(error, d.error_detail());
    if (state.opening()) ImGui::TextWrapped("Opening %s ...", state.requested_path().c_str());
    if (!state.error().empty()) warning_line(state.error(), state.error_detail());

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
        if (pending_select_) select_step(*pending_select_);
        pending_select_.reset();
    }

    draw_head(chart);
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
        ImGui::SameLine(0.0f, rem(i == 0 ? kHeadGapRem : kScoresGapRem));
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
    // The title and scores stay put; from here each item flows onto the next
    // line when it would pass the window's edge.
    const ImGuiStyle& style = ImGui::GetStyle();
    ui::flow_next(ui::button_slot_width(kUndoLabel), rem(kHeadGapRem));
    ImGui::BeginDisabled(!h.can_undo());
    if (ImGui::Button(kUndoLabel)) {
        pending_select_.reset();
        h.undo();
    }
    ImGui::EndDisabled();
    ui::flow_next(ui::button_slot_width(kStartOverLabel), style.ItemSpacing.x);
    ImGui::BeginDisabled(h.current().windows.empty());
    if (ImGui::Button(kStartOverLabel)) start_over();
    ImGui::EndDisabled();
    ui::flow_next(ui::button_slot_width(kFinishForMeLabel), style.ItemSpacing.x);
    ImGui::BeginDisabled(!d.can_finish());
    if (ImGui::Button(kFinishForMeLabel)) {
        pending_select_.reset();
        d.finish(state);
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("%s", kFinishTooltip);
    ui::flow_next(ui::checkbox_slot_width(kShowBestPathLabel), rem(kBestBoxGapRem));
    ImGui::Checkbox(kShowBestPathLabel, &show_best_);
}

void PbApp::hold_for_drag(bool held) {
    dragging_ = held;
    if (preview_) preview_->set_scrubbing(held, ui::PreviewController::ScrubHold::Timeline);
}

void PbApp::draw_timeline(const OpenChart& chart) {
    if (!preview_ || !preview_->shows_path(overlay_key_)) {
        // No rows this frame, so a drag has lost its item: ImGui ends it.
        hold_for_drag(false);
        if (preview_ && preview_->loading()) {
            const ui::PreviewController::LoadProgress p = preview_->load_progress();
            ImGui::ProgressBar(p.fraction, ImVec2(-1, 0), p.label.c_str());
        }
        return;
    }
    const app::PreviewScene& scene = preview_->scene();
    const DecisionList& d = state.decisions();
    const double length_ms = preview_->playback_end_ms();
    move_view_with_clock(length_ms);

    ui::TimelineInput in;
    in.scene = &scene;
    in.length_ms = length_ms;
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
    in.view = view_;

    const ui::TimelineEvents ev = ui::PreviewTimeline{}.draw(in);
    if (ev.hovered_activations_label) draw_activations_hover();

    // The scrollbar or the wheel moved the window by hand; while
    // playing, that stops the window following the playhead until Play is
    // pressed again (the third mockup's setView with byUser).
    if (ev.view) {
        view_ = *ev.view;
        if (preview_->playing()) follow_ = false;
    }

    // The fill under the pointer, on the timeline or by its list row. A hover
    // only lights things; it never touches the Preview.
    hovered_fill_ = ev.hovered_fill && *ev.hovered_fill < scene.fills.size() ? ev.hovered_fill : in.highlighted;

    // The drag holds the song through the controller's scrubbing rule, and the
    // clock follows it.
    hold_for_drag(ev.dragged_ms.has_value());
    if (ev.dragged_ms) preview_->seek_ms(*ev.dragged_ms);

    // The clock as the drag left it: next frame's move_view_with_clock sees
    // every later change, and none of the drag's own seeks.
    last_position_ms_ = preview_->position_ms();
    was_playing_ = preview_->playing();

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
            n.tier = mine->tier;
            n.best_fill = best_fill_name(scene, cands).value_or(std::string());
            n.hydra_end = d.candidate_end(static_cast<size_t>(mine - cands.begin()));
            numbers = n;
        }
        draw_fill_card(fill_hover_card(scene, i, numbers));
    }

    if (ev.clicked_fill && *ev.clicked_fill < scene.fills.size()) {
        click_fill(chart, *ev.clicked_fill);
    } else if (ev.clicked_band) {
        select_step(*ev.clicked_band);
    } else if (ev.clicked_ms) {
        preview_->seek_ms(*ev.clicked_ms);
    }
}

void PbApp::move_view_with_clock(double length_ms) {
    const double pos = preview_->position_ms();
    const bool playing = preview_->playing();
    if (!view_) {
        view_ = ui::open_view(length_ms);
        follow_ = true;
    } else if (dragging_) {
        // The pointer is on the window, so the window holds still under it.
    } else if (playing) {
        // A press of Play turns following back on (the third mockup's
        // togglePlay, line 985); the window then turns its pages with the
        // playhead (follow, line 362).
        if (!was_playing_) follow_ = true;
        if (follow_) view_ = ui::follow_playing(*view_, pos, length_ms);
    } else if (!was_playing_ && pos != last_position_ms_) {
        // The paused clock jumped (the scrubber, a jump button, "< Act",
        // "Act >" or a click on the timeline): a jump out of the window
        // brings the window with it (seek, line 813). A clock that was
        // playing last frame only ran, so pausing moves nothing.
        view_ = ui::show(*view_, pos, length_ms);
    }
}

void PbApp::scroll_to_fill(const app::PreviewScene& scene, size_t index, double length_ms) {
    if (!view_ || index >= scene.fills.size()) return;
    const double at = ui::timeline_fill_ms(scene.fills[index]);
    if (!ui::in_view(*view_, at)) view_ = ui::centre_on(*view_, at, length_ms);
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
    } else if (fill_refusal(scene, index) == FillRefusal::None) {
        // A refused fill's click makes no window; its hover card says why.
        const PathHistory& h = state.history();
        pending_select_ = h.current().windows.size();
        state.decisions().click_fill(state.search_graph(), chart.settings, h.current().windows,
                                     f.chord_tick, h.generation());
    }
    // Whatever the click did, the Preview stops on the fill's dot. This comes
    // last so select_step's seek to the activation does not stand.
    preview_->pause();
    preview_->seek_ms(ui::timeline_fill_ms(f));
}

void PbApp::draw_preview(const OpenChart& chart) {
    if (!preview_) return;
    // The panel draws only a loaded controller, so its error is the
    // builder's to show.
    if (preview_->has_error()) {
        warning_line(preview_->error(), preview_->error_detail());
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
        if (ImGui::Selectable(id.c_str(), hovered_fill_ == fill)) {
            scroll_to_fill(scene, *fill, preview_->playback_end_ms());
            click_fill(chart, *fill);
        }
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
        // The compact row: the step's number in a disc of the path's colour,
        // where it starts and ends, the phrases it spends and its timings
        // (StepShort), and its cost at the right. The whole row is one
        // selectable; its pieces are drawn over it.
        const std::string number = std::to_string(i + 1);
        const StepShort brief = step_short(chart.song.timing(), acts[i]);
        const std::optional<int64_t> cost = i < list.rows.size() ? list.rows[i].cost : std::nullopt;
        const std::optional<app::CostTier> tier = i < list.rows.size() ? list.rows[i].tier : std::nullopt;
        const ImVec2 start = ImGui::GetCursorPos();
        const ImVec2 start_screen = ImGui::GetCursorScreenPos();
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
        const std::string saved =
            brief.squeezes.empty() ? brief.saved : brief.saved + app::kFactSeparator + brief.squeezes;
        ImGui::TextDisabled("%s", saved.c_str());
        const std::string cost_label = cost ? cost_or_best(*cost) : std::string(kPendingText);
        text_at_right(cost_label, start.x, row_w,
                      tier ? tier_color(*tier) : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::SetCursorPosY(start.y + row_h + ImGui::GetStyle().ItemSpacing.y);

        // The opened step: one line per ending when it has more than one
        // (the fourth mockup's step panel), the violet note when the step is
        // over the Path limit, then Remove. The endings are the ones
        // DecisionList::select worked out when the step opened; until they
        // land the step shows no lines.
        if (sel == i) {
            if (d.selected() == i && d.ends().size() > 1 && i < windows.size()) {
                ImGui::TextDisabled("%s", kEndItAtLabel);
                std::vector<int64_t> finishes;
                for (const EndChoice& c : d.ends()) finishes.push_back(c.best_finish);
                const std::vector<int64_t> costs = app::end_costs(finishes);
                for (size_t e = 0; e < d.ends().size(); ++e) {
                    const EndChoice& c = d.ends()[e];
                    const bool kept = is_kept_end(windows[i], c);
                    ImGui::PushID(static_cast<int>(e));
                    // Only the step being built can change its end. An
                    // earlier step's lines take no clicks; the one it kept
                    // keeps its full colour, the others are greyed.
                    if (!last) {
                        ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, kept ? 1.0f : kFixedEndAlpha);
                        ImGui::BeginDisabled();
                    }
                    // The line is one selectable, in the selected look for
                    // the kept end; its mark, words and cost are drawn over it.
                    const ImVec2 line = ImGui::GetCursorPos();
                    const float line_w = ImGui::GetContentRegionAvail().x;
                    const float line_h = ImGui::GetFrameHeight();
                    const std::string id =
                        "##end_" + (c.window.deact_tick ? std::to_string(*c.window.deact_tick)
                                                        : c.activation.notationstr());
                    ImGui::SetNextItemAllowOverlap();
                    bool picked = ImGui::Selectable(id.c_str(), kept, 0, ImVec2(0, line_h));
                    ImGui::SetCursorPos(line);
                    picked = ImGui::RadioButton("##mark", kept) || picked;
                    ImGui::SameLine();
                    ImGui::SetCursorPosY(line.y + (line_h - ImGui::GetTextLineHeight()) * 0.5f);
                    ImGui::TextUnformatted(end_choice_text(chart.song.timing(), c.activation).c_str());
                    const ImVec4 cost_col = run && !run->ladder.bf.empty()
                                                ? tier_color(app::cost_tier(costs[e], run->ladder.bf.front()))
                                                : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
                    text_at_right(cost_or_best(costs[e]), line.x, line_w, cost_col);
                    if (!last) {
                        ImGui::EndDisabled();
                        ImGui::PopStyleVar();
                    }
                    ImGui::SetCursorPosY(line.y + line_h + ImGui::GetStyle().ItemSpacing.y);
                    if (picked && last) {
                        pending_select_ = i;
                        d.choose_end(c.window, h.generation());
                    }
                    ImGui::PopID();
                }
            }
            const std::string note = tight_window_note(acts[i], chart.settings.ms_filter);
            if (!note.empty()) {
                ImGui::PushStyleColor(ImGuiCol_Text, violet);
                ImGui::PushTextWrapPos(0.0f);
                ImGui::TextWrapped("%s", note.c_str());
                ImGui::PopTextWrapPos();
                ImGui::PopStyleColor();
            }
            if (ImGui::Button(last ? kRemoveLastLabel : kRemoveEarlierLabel)) {
                pending_select_.reset();
                d.click_remove(state.search_graph(), chart.settings, windows, windows[i].act_tick,
                               h.generation());
            }
            // The panel scrolls the opened step into view, row, ending
            // lines, note and button, as little as it can (its top first
            // when it is taller than the panel). It does so when the step
            // opens and whenever the block's height changes, as when its
            // endings land; a scroll by hand leaves the height alone, so the
            // panel never pulls back against it.
            const float block_bottom = ImGui::GetCursorScreenPos().y;
            const float block_h = block_bottom - start_screen.y;
            if (block_h != steps_block_h_) {
                ui::scroll_block_into_view(ui::ScrollTo::NearestEdge, start_screen.x, start_screen.y,
                                           start_screen.x + row_w, block_bottom);
                steps_block_h_ = block_h;
            }
        }
        ImGui::Separator();
        ImGui::PopID();
    }
    if (!list.failure.empty()) {
        ui::WarnColor warn;
        ImGui::TextWrapped("%s", list.failure.c_str());
    }
}

}  // namespace hydra::pathbuilder
