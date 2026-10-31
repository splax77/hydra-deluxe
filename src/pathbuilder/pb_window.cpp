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
#include "app/display_format.h"  // clock_str, format_cost, format_cost_short, kFactSeparator
#include "app/path_costs.h"      // end_costs
#include "app/preview_view.h"    // path_overlay_key, fill_index_on_chord
#include "app/report.h"          // ChipToken
#include "app/user_messages.h"
#include "core/timing.h"  // measure_label
#include "pathbuilder/pb_files.h"
#include "pathbuilder/pb_menu.h"
#include "pathbuilder/pb_open.h"
#include "search/graph.h"  // fill_rule_for
#include "ui/fonts.h"      // px
#include "ui/preview_controller.h"
#include "ui/preview_timeline.h"
#include "ui/text_role.h"
#include "ui/theme.h"    // chip_color, kAccentColor, kStarPowerColor
#include "ui/widgets.h"  // WarnColor, error_detail_line

namespace hydra::pathbuilder {

namespace {

// The highway's width beside the steps panel: the mockup's
// `.lower{grid-template-columns:600px minmax(0,1fr)}`.
constexpr float kHighwayW = 600.0f;
// How wide a score's hint and the hover tooltip may grow before they wrap:
// `.num .hint{max-width:190px}` and `.tip{max-width:360px}`.
constexpr float kScoreHintW = 190.0f;
constexpr float kTooltipW = 360.0f;
// The space between the legend's items: `.legend{gap:18px}`.
constexpr float kLegendGap = 18.0f;

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

// A cost under a fill or an end choice: "best" when it loses nothing, else
// the cost written by `format` (format_cost_short or format_cost).
std::string cost_or_best(int64_t cost, std::string (*format)(int64_t)) {
    return app::costs_nothing(cost) ? std::string(kBestFillLabel) : format(cost);
}

}  // namespace

PbApp::PbApp(ID3D11Device* device, ID3D11DeviceContext* context) {
    if (device && context) preview_ = std::make_unique<ui::PreviewController>(device, context);
}

PbApp::~PbApp() { close_preview(); }

void PbApp::open(const std::string& notespath) { state.open(notespath); }

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
    {
        ui::ScopedFont title(ui::TextRole::Title);
        ImGui::TextWrapped("%s", chart_title_line(chart).c_str());
    }
    ImGui::TextDisabled("%s", app::chartmode_key(chart.settings.reading()).c_str());

    const Path* shown = shown_path();
    if (!chart.has_star_power()) {
        ImGui::TextWrapped("%s", kNothingToBuildText);
        open_preview(chart, shown);
        draw_highway(chart);
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

    text_with_star(intro_text(), intro_star_at());
    draw_scores(chart);
    draw_guide(chart);
    draw_legend(chart);
    draw_timeline(chart);

    if (!ImGui::GetIO().WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Escape, false))
        selected_.reset();

    ImGui::BeginChild("##highway", ImVec2(ui::px(kHighwayW), 0), ImGuiChildFlags_Borders);
    draw_highway(chart);
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("##steps", ImVec2(0, 0), ImGuiChildFlags_Borders);
    draw_steps(chart);
    ImGui::EndChild();
}

void PbApp::draw_scores(const OpenChart& chart) {
    const app::LadderRun* run = state.decisions().run();
    const bool complete = run && run->complete;
    const std::optional<int64_t> mine =
        complete ? std::optional<int64_t>(run->ladder.player_total) : std::nullopt;
    const std::optional<int64_t> best =
        run && !run->ladder.bf.empty() ? std::optional<int64_t>(run->ladder.bf.front()) : std::nullopt;
    const std::optional<int64_t> reach =
        complete ? std::optional<int64_t>(run->ladder.bf.back()) : std::nullopt;

    const char* labels[] = {kYourScoreLabel, kBestPossibleLabel, kBestReachLabel};
    const char* hints[] = {kYourScoreHint, kBestPossibleHint, kBestReachHint};
    const std::optional<int64_t> values[] = {mine, best, reach};
    for (size_t i = 0; i < 3; ++i) {
        if (i > 0) ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ui::px(kScoreHintW));
        ImGui::TextDisabled("%s", labels[i]);
        {
            ui::ScopedFont figure(ui::TextRole::Figure);
            ImGui::TextUnformatted(figure_or_pending(values[i]).c_str());
        }
        dim_wrapped(hints[i]);
        ImGui::PopTextWrapPos();
        ImGui::EndGroup();
    }

    PathHistory& h = state.history();
    DecisionList& d = state.decisions();
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
    (void)chart;
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
    const std::pair<ui::LegendSwatch, const char*> items[] = {
        {ui::LegendSwatch::Offered, kLegendCanText},
        {ui::LegendSwatch::Activation, kLegendMineText},
        {ui::LegendSwatch::Cant, kLegendCantText},
    };
    for (const auto& [swatch, text] : items) {
        ui::legend_swatch(swatch);
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::TextDisabled("%s", text);
        ImGui::SameLine(0.0f, ui::px(kLegendGap));
    }
    ImGui::TextDisabled("%s", kLegendSeekText);
    ImGui::SameLine(0.0f, ui::px(kLegendGap));
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
    // The screen's left-to-right rule: an offered fill fill_reason refuses
    // is drawn as can't.
    for (size_t i = 0; i < scene.fills.size(); ++i)
        if (scene.fills[i].state == app::PreviewFillState::Offered && !fill_reason(scene, i).empty())
            in.cant_ticks.push_back(scene.fills[i].chord_tick);
    for (const app::CandidateCost& c : d.candidates())
        in.labels.push_back(ui::TimelineFillLabel{
            c.act_tick, cost_or_best(c.cost, app::format_cost_short),
            ImGui::GetColorU32(tier_color(c.tier))});

    const ui::TimelineEvents ev = ui::PreviewTimeline{}.draw(in);

    // While the pointer is on a fill the highway shows it, paused, with the
    // fill's start on the strike line (the user's answer Q4); when the
    // pointer leaves, the Preview goes back to the time and play state it had.
    if (ev.hovered_fill && *ev.hovered_fill < scene.fills.size()) {
        if (!hovered_fill_) {
            hover_return_ms_ = preview_->position_ms();
            hover_return_playing_ = preview_->playing();
            preview_->pause();
        }
        if (hovered_fill_ != ev.hovered_fill) preview_->seek_ms(scene.fills[*ev.hovered_fill].span.start_ms);
        hovered_fill_ = ev.hovered_fill;

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
        ImGui::PushTextWrapPos(ui::px(kTooltipW));
        ImGui::TextUnformatted(fill_hover_text(scene, i, numbers).c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    } else if (hovered_fill_) {
        hovered_fill_.reset();
        preview_->seek_ms(hover_return_ms_);
        if (hover_return_playing_) preview_->play();
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
    const std::string reason = fill_reason(scene, index);
    if (!reason.empty()) {
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

void PbApp::draw_highway(const OpenChart& chart) {
    if (!preview_) return;
    if (preview_->has_error()) {
        {
            ui::WarnColor warn;
            ImGui::TextWrapped("%s", preview_->error().c_str());
        }
        ui::error_detail_line(preview_->error_detail());
        return;
    }
    if (preview_->loading()) return;

    ImGui::TextUnformatted(kHighwayHeading);
    ImGui::SameLine();
    if (ImGui::Button(preview_->playing() ? kPauseLabel : kPlayLabel)) preview_->toggle();
    ImGui::SameLine();
    // The mockup's "where" line, the measure and the clock at the highway's
    // time: the measure is the tick the Preview's time box shows at that time
    // (display_tick_at_ms), named by measure_label.
    const double now_ms = preview_->position_ms();
    std::string where = app::clock_str(now_ms);
    if (const std::optional<SongTiming>& timing = preview_->scene().timing)
        where = measure_label(timing->timecode(timing->display_tick_at_ms(now_ms))) + app::kFactSeparator + where;
    ImGui::TextDisabled("%s", where.c_str());
    if (!ImGui::GetIO().WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Space, false))
        preview_->toggle();

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const int w = static_cast<int>(avail.x);
    const int h = static_cast<int>(avail.y);
    if (w <= 0 || h <= 0) return;
    // The image is a named item so a click on it, and the GUI test, can find
    // it; the highway image is drawn at the same place.
    const ImVec2 img_min = ImGui::GetCursorScreenPos();
    const ImVec2 img_size(static_cast<float>(w), static_cast<float>(h));
    ImGui::InvisibleButton("##highway_image", img_size);
    const bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
    if (ID3D11ShaderResourceView* srv = preview_->render(w, h)) {
        ImGui::SetCursorScreenPos(img_min);
        ImGui::Image((ImTextureID)(intptr_t)srv, img_size);
    }
    if (clicked && chart.has_star_power()) {
        const ImVec2 m = ImGui::GetIO().MousePos;
        if (const std::optional<size_t> fill =
                preview_->fill_at_image(render::ImagePoint{m.x - img_min.x, m.y - img_min.y}, w, h))
            click_fill(chart, *fill);
    }
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
        const std::string number = std::to_string(i + 1) + ".";
        // The whole step is one selectable; its text is drawn over it.
        const ImVec2 start = ImGui::GetCursorPos();
        const float wrap = ImGui::GetContentRegionAvail().x;
        float height = ImGui::CalcTextSize(text ? text->sentence.c_str() : kPendingText, nullptr, false, wrap).y;
        if (text) height += ImGui::CalcTextSize(text->why.c_str(), nullptr, false, wrap).y;
        if (text && !text->note.empty()) height += ImGui::CalcTextSize(text->note.c_str(), nullptr, false, wrap).y;
        ImGui::SetNextItemAllowOverlap();
        if (ImGui::Selectable("##step", sel == i, 0, ImVec2(0, height))) {
            if (sel == i)
                selected_.reset();
            else
                select_step(i);
        }
        ImGui::SetCursorPos(start);
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextWrapped("%s %s", number.c_str(), text ? text->sentence.c_str() : kPendingText);
        if (text) {
            dim_wrapped(text->why);
            if (!text->note.empty()) {
                ImGui::PushStyleColor(ImGuiCol_Text, violet);
                ImGui::TextWrapped("%s", text->note.c_str());
                ImGui::PopStyleColor();
            }
        }
        ImGui::PopTextWrapPos();

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
                    label += cost_or_best(costs[e], app::format_cost);
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
