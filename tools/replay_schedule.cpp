#include "replay_schedule.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <stdexcept>
#include <tuple>

#include "core/squeeze_rating.h"  // steady_spans

namespace hydra::schedule {

namespace {

// ---- the constants ---------------------------------------------------------
// Each is written into the file's "constants" object, where the tests pin it.

// The game's normal-mode clamps on one half-gap: TM-18 on
// docs/clone-hero/03-timing.md (0.0375 s and 0.085 s).
constexpr double kWindowFrontMs = 37.5;
constexpr double kWindowBackMs = 85.0;

// The normal-mode window formula's constants: TM-21 on
// docs/clone-hero/03-timing.md.
constexpr double kFormulaLinear = 0.0110924369747899;
constexpr double kFormulaSquare = 2.628539e-05;
constexpr double kFormulaScale = 170.0;
constexpr double kFormulaOffsetMs = 20.0;

// The game's frame period: one frame at the game's framerate = 330 cap
// (TM-49 on docs/clone-hero/03-timing.md) and the 2.98 to 3.03 ms the live
// activations showed (SP-47 on docs/clone-hero/05-star-power.md). The user
// set it: question 1 of docs/superpowers/plans/2026-11-08-player-freak-fixes.md.
constexpr double kFrameMs = 3.0;

// A press is judged between one and two frames after the clock the player
// read when it sent it (TM-27 on docs/clone-hero/03-timing.md, SP-47).
constexpr double kJudgeEarlyMs = 1 * kFrameMs;
constexpr double kJudgeLateMs = 2 * kFrameMs;

// Each chord is pressed this many ms before its time (D51).
constexpr double kPressLeadMs = 2.0;

// A chord that ends Star Power is pressed this many ms before its time, so
// the hit lands in a frame before the end check. The user set it: question 3
// of the optimal-score run plan, which has left the tree (removed; git show
// d7e47a20:docs/superpowers/plans/2026-11-06-optimal-score-run.md).
constexpr double kSpEndLeadMs = 15.0;

// A squeezed-out chord's late gem is pressed this many ms after the Star
// Power end, so the hit lands after the frame that ends it. The user set it:
// question 2 of docs/superpowers/plans/2026-11-07-player-refinements.md.
constexpr double kSpOutLagMs = 15.0;

// A chord whose gems the game would score in an order that costs points is
// pressed in groups, each this many ms after the one before (SC-38 on
// docs/clone-hero/06-scoring.md says why). The user set it ("8 ms"), and said
// to split "Only where it matters": the amendment after LS-R1b run 1 in
// docs/superpowers/plans/2026-11-07-player-refinements.md.
constexpr double kChordOrderGapMs = 8.0;

// How far inside its window every press of a squeeze must stay, in the worst
// frame case, for the squeeze to be feasible at 100% speed. The user set it:
// question 2 of docs/superpowers/plans/2026-11-08-player-freak-fixes.md.
constexpr double kRoomMarginMs = 3.0;

nlohmann::json constants_json() {
    return nlohmann::json{
        {"window_front_ms", kWindowFrontMs},
        {"window_back_ms", kWindowBackMs},
        {"formula_linear", kFormulaLinear},
        {"formula_square", kFormulaSquare},
        {"formula_scale", kFormulaScale},
        {"formula_offset_ms", kFormulaOffsetMs},
        {"frame_ms", kFrameMs},
        {"judge_early_ms", kJudgeEarlyMs},
        {"judge_late_ms", kJudgeLateMs},
        {"press_lead_ms", kPressLeadMs},
        {"sp_end_lead_ms", kSpEndLeadMs},
        {"sp_out_lag_ms", kSpOutLagMs},
        {"chord_order_gap_ms", kChordOrderGapMs},
        {"room_margin_ms", kRoomMarginMs},
    };
}

nlohmann::json or_null(const std::optional<double>& v) {
    return v ? nlohmann::json(*v) : nlohmann::json(nullptr);
}

nlohmann::json or_null(const std::optional<int>& v) {
    return v ? nlohmann::json(*v) : nlohmann::json(nullptr);
}

const char* step_kind_name(SpEndKind kind) {
    switch (kind) {
        case SpEndKind::Activation: return "activation";
        case SpEndKind::Collected: return "collected";
        case SpEndKind::Clamped: return "clamped";
        case SpEndKind::SqIn: return "squeeze_in";
    }
    return "unknown";
}

nlohmann::json scale_json(const std::optional<TransferScale>& s) {
    if (!s) return nlohmann::json{{"early", nullptr}, {"late", nullptr}};
    return nlohmann::json{{"early", s->early}, {"late", s->late}};
}

nlohmann::json spans_json(const SteadySpans& s) {
    return nlohmann::json{{"before_anchor_ms", or_null(s.before_anchor_ms)},
                          {"after_anchor_ms", or_null(s.after_anchor_ms)},
                          {"before_end_ms", or_null(s.before_end_ms)},
                          {"after_end_ms", or_null(s.after_end_ms)}};
}

std::string tick_text(int64_t tick) { return "tick " + std::to_string(tick); }

}  // namespace

// ---- the game's per-note window ------------------------------------------

// TM-19: half the gap, held between the front and the back; a missing
// neighbour, or one at the same time, gives the back.
double half_gap_ms(std::optional<double> gap_ms) {
    if (!gap_ms || *gap_ms == 0.0) return kWindowBackMs;
    return std::min(std::max(*gap_ms / 2.0, kWindowFrontMs), kWindowBackMs);
}

// TM-21. The function has no clamp of its own; its input is clamped by
// half_gap_ms on each side (TM-22).
double whole_window_ms(double x) {
    return (x * kFormulaLinear - x * x * kFormulaSquare) * kFormulaScale - kFormulaOffsetMs;
}

// TM-20: the note gets half the formula's output on each side.
double window_side_ms(std::optional<double> gap_before_ms, std::optional<double> gap_after_ms) {
    return whole_window_ms(half_gap_ms(gap_before_ms) + half_gap_ms(gap_after_ms)) / 2.0;
}

// TM-51: the neighbours are the nearest notes at another tick, so the
// chords either side.
std::vector<double> chord_sides_ms(const std::vector<double>& chord_ms) {
    std::vector<double> sides;
    sides.reserve(chord_ms.size());
    for (size_t i = 0; i < chord_ms.size(); ++i) {
        const std::optional<double> before =
            i > 0 ? std::optional<double>(chord_ms[i] - chord_ms[i - 1]) : std::nullopt;
        const std::optional<double> after = i + 1 < chord_ms.size()
                                                ? std::optional<double>(chord_ms[i + 1] - chord_ms[i])
                                                : std::nullopt;
        sides.push_back(window_side_ms(before, after));
    }
    return sides;
}

// ---- the decline mode ------------------------------------------------------

const char* decline_mode_name(DeclineMode mode) {
    switch (mode) {
        case DeclineMode::KickOnly: return "kick_only";
        case DeclineMode::Nothing: return "nothing";
        case DeclineMode::AllButTopPad: return "all_but_top_pad";
    }
    return "unknown";
}

std::optional<DeclineMode> decline_mode_from_name(std::string_view name) {
    for (DeclineMode m : kDeclineModes)
        if (name == decline_mode_name(m)) return m;
    return std::nullopt;
}

std::string decline_mode_list() {
    std::string out;
    for (DeclineMode m : kDeclineModes) {
        if (!out.empty()) out += ", ";
        out += decline_mode_name(m);
    }
    return out;
}

// kick_only sends at most one gem, the first kick. all_but_top_pad sends
// every gem but the chord's activation note (Chord::activation_note owns the
// pad order, the one FL-09 on docs/clone-hero/04-fills.md reads), so a chord
// whose activation note is its kick has no pad gem and sends nothing. A Chord
// holds one note per colour, so the top pad is never ambiguous.
KeptGems kept_gems(DeclineMode mode, const Chord& chord, const std::vector<ReplayNote>& notes) {
    KeptGems out;
    if (mode == DeclineMode::Nothing) return out;
    if (mode == DeclineMode::KickOnly) {
        for (size_t k = 0; k < notes.size(); ++k)
            if (notes[k].color == NoteColor::Kick) {
                out.places.push_back(k);
                break;
            }
        return out;
    }
    if (notes.empty()) return out;
    const NoteColor top = chord.activation_note().colortype;
    if (top == NoteColor::Kick) return out;  // no pad gem: nothing is sent
    for (size_t k = 0; k < notes.size(); ++k)
        if (notes[k].color != top) out.places.push_back(k);
    return out;
}

// ---- the aims ----------------------------------------------------------------

double moving_ratio(const SqueezeGeometry& g) {
    return g.kind == SqueezeKind::SqOut ? g.ratio.early : g.ratio.late;
}

// Question 1 (a) of the plan, equal room: both presses keep the same room in
// the worst frame case. A squeeze-out (design point 4): the anchor goes L
// early, judged kJudgeEarlyMs to kJudgeLateMs later, so the end lands at most
// r(L - jl) early; the late gem goes kSpOutLagMs after that end and is judged
// up to jl later. Solving side_a - L = side_s - (gap + lag - r(L - jl)) - jl
// gives L = (side_a - side_s + gap + lag) / (1 + r) + jl.
//
// A squeeze-in is the mirror: the anchor goes L late, judged up to jl later,
// so its room is side_a - L - jl and the end lands at least r(L + je) late;
// the squeeze press goes kSpEndLeadMs before that end, and its room is
// side_s minus how far before the chord that aim is. Solving
// side_a - L - jl = side_s - (gap + lead - r(L + je)) gives
// L = (side_a - side_s + gap + lead - jl - r je) / (1 + r), gap being how far
// the chord sits past the end.
double equal_room_lead_ms(const SqueezeGeometry& g) {
    const double r = moving_ratio(g);
    if (g.kind == SqueezeKind::SqOut) {
        const double gap = g.end_ms - g.squeeze_ms;
        return (g.anchor_side_ms - g.squeeze_side_ms + gap + kSpOutLagMs) / (1.0 + r) + kJudgeLateMs;
    }
    const double gap = g.squeeze_ms - g.end_ms;
    return (g.anchor_side_ms - g.squeeze_side_ms + gap + kSpEndLeadMs - kJudgeLateMs - r * kJudgeEarlyMs) /
           (1.0 + r);
}

double press_room_ms(double aim_ms, double side_ms) {
    return side_ms - std::max(std::abs(aim_ms), std::abs(aim_ms + kJudgeLateMs));
}

SqueezeSplit split_at_lead(const SqueezeGeometry& g, double lead_ms) {
    const double r = moving_ratio(g);
    SqueezeSplit s;
    if (g.kind == SqueezeKind::SqOut) {
        s.anchor_aim_ms = -lead_ms;
        s.planned_end_ms = g.end_ms - r * (lead_ms - kJudgeEarlyMs);
        // The latest the end can land: the anchor judged two frames late.
        s.aim_end_ms = g.end_ms - r * (lead_ms - kJudgeLateMs);
        s.squeeze_aim_ms = s.aim_end_ms + kSpOutLagMs - g.squeeze_ms;
    } else {
        s.anchor_aim_ms = lead_ms;
        // The earliest the end can land: the anchor judged one frame late.
        s.planned_end_ms = g.end_ms + r * (lead_ms + kJudgeEarlyMs);
        s.aim_end_ms = s.planned_end_ms;
        s.squeeze_aim_ms = s.aim_end_ms - kSpEndLeadMs - g.squeeze_ms;
    }
    s.anchor_room_ms = press_room_ms(s.anchor_aim_ms, g.anchor_side_ms);
    s.squeeze_room_ms = press_room_ms(s.squeeze_aim_ms, g.squeeze_side_ms);
    s.feasible = s.anchor_room_ms >= kRoomMarginMs && s.squeeze_room_ms >= kRoomMarginMs;
    return s;
}

SqueezeSplit split_squeeze(const SqueezeGeometry& g) { return split_at_lead(g, equal_room_lead_ms(g)); }

std::optional<double> anchor_lead_ms(const std::vector<SqueezeGeometry>& on_one_anchor,
                                     std::vector<std::string>& refusals) {
    if (on_one_anchor.empty()) return std::nullopt;
    const SqueezeGeometry* early = nullptr;
    const SqueezeGeometry* late = nullptr;
    for (const SqueezeGeometry& g : on_one_anchor) (g.kind == SqueezeKind::SqOut ? early : late) = &g;
    const int anchor = on_one_anchor.front().anchor_chord;
    if (early && late) {
        refusals.push_back("anchor chord " + std::to_string(anchor) + " would have to be pressed early for chord " +
                           std::to_string(early->squeeze_chord) + " and late for chord " +
                           std::to_string(late->squeeze_chord) + "; no press does both");
        return std::nullopt;
    }
    const SqueezeGeometry* need = nullptr;
    double lead = 0.0;
    for (const SqueezeGeometry& g : on_one_anchor) {
        const double l = equal_room_lead_ms(g);
        if (!need || l > lead) {
            need = &g;
            lead = l;
        }
    }
    const bool is_out = need->kind == SqueezeKind::SqOut;
    const char* way = is_out ? "early" : "late";
    // Below the plain press's own lead the split would press the anchor the
    // other way, where the ratio and the frame model it rests on do not hold.
    if (lead < kPressLeadMs) {
        refusals.push_back("the equal-room split for chord " + std::to_string(need->squeeze_chord) +
                           " would press anchor chord " + std::to_string(anchor) + " " +
                           std::to_string(lead) + " ms " + way + ", less than a plain press; no rule times that");
        return std::nullopt;
    }
    // The ratio holds only while the tempo stays steady (steady_spans): on
    // the anchor's side the press moves into, and on the end's.
    const double r = moving_ratio(*need);
    const std::optional<double> at_anchor = is_out ? need->steady.before_anchor_ms : need->steady.after_anchor_ms;
    const std::optional<double> at_end = is_out ? need->steady.before_end_ms : need->steady.after_end_ms;
    const char* anchor_name = is_out ? "before_anchor_ms" : "after_anchor_ms";
    const char* end_name = is_out ? "before_end_ms" : "after_end_ms";
    for (const auto& [span, moved, name] : {std::tuple{at_anchor, lead, anchor_name},
                                            std::tuple{at_end, lead * r, end_name}}) {
        if (span && moved > *span) {
            refusals.push_back("anchor chord " + std::to_string(anchor) + " pressed " + std::to_string(lead) +
                               " ms " + way + " for chord " + std::to_string(need->squeeze_chord) +
                               " moves past a tempo or time signature change: the steady span " + name + " is " +
                               std::to_string(*span) + " ms");
            return std::nullopt;
        }
    }
    return lead;
}

double last_send_ms(double chord_ms, double side_ms) { return chord_ms + side_ms - kJudgeEarlyMs; }

// The late press goes kSpOutLagMs after the read end and is judged up to
// kJudgeLateMs later.
double sqout_fallback_after_ms(double chord_ms, double side_ms) {
    return chord_ms + side_ms - kSpOutLagMs - kJudgeLateMs;
}

// Design point 5's threshold as the plan writes it: the press goes
// kSpEndLeadMs before the read end, held kJudgeEarlyMs further inside.
double sqin_fallback_before_ms(double chord_ms, double side_ms) {
    return chord_ms - side_ms + kSpEndLeadMs + kJudgeEarlyMs;
}

// _order_groups in tools/ch_probe/experiments/play_chart.py, moved: "An
// activation's chord presses its activating_gem first; the rest go in groups
// by multipliers, the lower multiplier first." The user's "Only where it
// matters" (kChordOrderGapMs's source) is why a chord whose gems share one
// multiplier stays one group.
std::vector<std::vector<size_t>> press_groups(const std::vector<int>& multipliers,
                                              const std::vector<size_t>& places,
                                              std::optional<size_t> first) {
    std::vector<std::vector<size_t>> groups;
    if (places.empty()) return std::vector<std::vector<size_t>>(1);
    std::vector<size_t> rest;
    for (size_t p : places)
        if (!first || p != *first) rest.push_back(p);
    if (first && std::find(places.begin(), places.end(), *first) != places.end()) groups.push_back({*first});
    std::set<int> levels;
    for (size_t p : rest) levels.insert(multipliers[p]);
    for (int m : levels) {
        std::vector<size_t> g;
        for (size_t p : rest)
            if (multipliers[p] == m) g.push_back(p);
        groups.push_back(std::move(g));
    }
    return groups;
}

// _squeeze_out_inside_groups in play_chart.py, moved: "every gem but the late
// one (place 0 ...), the lower multiplier first. The late gem goes after them
// all, so a late gem the score file pays at a lower multiplier than an inside
// gem is an order no press gives." Design point 5 of
// docs/superpowers/plans/2026-11-07-player-limits.md and the user's answer to
// its question 5 say why that is Hydra's question, not the player's.
std::optional<std::vector<std::vector<size_t>>> squeeze_out_inside_groups(
    int chord, const std::vector<int>& multipliers, std::vector<std::string>& refusals) {
    std::vector<size_t> inside;
    for (size_t p = 1; p < multipliers.size(); ++p) inside.push_back(p);
    if (inside.empty()) return std::vector<std::vector<size_t>>{};
    int highest = multipliers[0];
    for (size_t p : inside) highest = std::max(highest, multipliers[p]);
    if (highest > multipliers[0]) {
        refusals.push_back("chord " + std::to_string(chord) + " is a squeeze-out whose late gem pays x" +
                           std::to_string(multipliers[0]) + " and an inside gem x" + std::to_string(highest) +
                           ": Hydra assumes an order no press gives, since the late gem goes last; this is a "
                           "question about Hydra's path for the user, not a player bug");
        return std::nullopt;
    }
    return press_groups(multipliers, inside, std::nullopt);
}

// _refuse_far_presses in play_chart.py, moved, for one press.
std::optional<std::string> far_press_refusal(const std::vector<PadChord>& chords, size_t place,
                                             double send_ms, const std::vector<Pad>& pads) {
    const PadChord& own = chords[place];
    auto inside = [&](const PadChord& c) { return std::abs(send_ms - c.ms) <= c.side_ms; };
    auto has = [](const std::vector<Pad>& list, const Pad& p) {
        return std::find(list.begin(), list.end(), p) != list.end();
    };
    const std::string who = "a press of chord " + std::to_string(own.index) + " sent at " +
                            std::to_string(send_ms) + " ms";
    if (!inside(own))
        return who + " leaves its own window (" + std::to_string(own.side_ms) + " ms a side)";
    for (const Pad& pad : pads) {
        for (size_t i = place; i-- > 0;) {
            if (!has(chords[i].chart_pads, pad)) continue;
            if (inside(chords[i]))
                return who + " lands inside the window of chord " + std::to_string(chords[i].index) +
                       ", the note before it on its " + color_str(pad.first) + (pad.second ? " cymbal" : "") +
                       " pad";
            break;
        }
        for (size_t i = place + 1; i < chords.size(); ++i) {
            if (!has(chords[i].chart_pads, pad)) continue;
            if (!has(chords[i].sent_pads, pad) && inside(chords[i]))
                return who + " lands inside the window of chord " + std::to_string(chords[i].index) +
                       ", the next note on its " + color_str(pad.first) + (pad.second ? " cymbal" : "") +
                       " pad, which the player does not press";
            break;
        }
    }
    return std::nullopt;
}

// ---- the file --------------------------------------------------------------

nlohmann::json replay_note_json(const ReplayNote& n) {
    return nlohmann::json{{"color", color_str(n.color)},
                          {"cymbal", n.cymbal},
                          {"sp_points", n.sp_points},
                          {"multiplier", n.multiplier},
                          {"dynamics_bonus", n.dynamics_bonus},
                          {"dynamic", dynamic_str(n.dynamic)}};
}

std::optional<int> ChordIndex::at(int64_t tick) const {
    const auto it = std::lower_bound(ticks.begin(), ticks.end(), tick);
    if (it == ticks.end() || *it != tick) return std::nullopt;
    return static_cast<int>(it - ticks.begin());
}

ChordIndex chord_index(const Song& song) {
    ChordIndex idx;
    idx.ticks.reserve(song.sequence.size());
    for (const SongTimestamp& ts : song.sequence) idx.ticks.push_back(ts.timecode.ticks());
    return idx;
}

nlohmann::json activation_json(const Activation& act, const ChordIndex& chords,
                               const SongTiming& timing, std::vector<std::string>& refusals) {
    const int64_t act_tick = act.timecode.ticks();
    const std::string who = "the activation at " + tick_text(act_tick);
    auto ms_of = [&](int64_t tick) { return timing.timecode(tick).ms(); };

    nlohmann::json steps = nlohmann::json::array();
    for (size_t s = 0; s < act.sp_end_steps.size(); ++s) {
        const SpEndStep& st = act.sp_end_steps[s];
        const int64_t anchor = act.end_anchor_tick(s);
        steps.push_back(nlohmann::json{{"tick", st.tick},
                                       {"chord", or_null(chords.at(st.tick))},
                                       {"kind", step_kind_name(st.kind)},
                                       {"end_tick", st.end_tick},
                                       {"end_ms", ms_of(st.end_tick)},
                                       {"anchor_tick", anchor},
                                       {"anchor_chord", or_null(chords.at(anchor))}});
    }

    const std::optional<int64_t> nominal = act.nominal_end();
    const std::optional<int64_t> deact = act.deact_tick();
    const std::optional<int64_t> read_after = act.deact_anchor_tick();
    if (!deact) refusals.push_back(who + " has no SP-end steps, so its end is not known");

    nlohmann::json squeezes = nlohmann::json::array();
    for (size_t k = 0; k < act.sqinouts.size(); ++k) {
        const SPSqueeze& q = act.sqinouts[k];
        const bool is_in = q.kind == SqueezeKind::SqIn;
        const std::optional<size_t> end_step = act.squeeze_end_step(k);
        const std::optional<int64_t> end = act.squeeze_end_tick(k);
        const std::optional<int64_t> anchor = act.squeeze_anchor_tick(k);
        // A SqIn's chord is the step it set; a SqOut's is the chord the
        // engine stamped (Activation::sqout_tick).
        std::optional<int64_t> chord_tick;
        if (is_in) {
            if (end_step && *end_step + 1 < act.sp_end_steps.size())
                chord_tick = act.sp_end_steps[*end_step + 1].tick;
        } else {
            chord_tick = act.sqout_tick;
        }
        const std::string what = who + "'s " + q.type_name() + " " + std::to_string(k);
        if (!chord_tick) refusals.push_back(what + " names no chord in the record");
        if (!end || !anchor) refusals.push_back(what + " has no end in the record");

        // The ratios as stored: a SqIn's own, a SqOut's on transfer_post.
        const std::optional<TransferScale>& ratio = is_in ? q.transfer : act.transfer_post;
        if (!ratio)
            refusals.push_back(what + " has no stored ratio (" +
                               (is_in ? "SPSqueeze::transfer" : "Activation::transfer_post") +
                               " is unset), so its lead cannot be planned");

        nlohmann::json one{{"index", k},
                           {"kind", q.type_name()},
                           {"offset_ms", q.offset()},
                           {"chord_tick", chord_tick ? nlohmann::json(*chord_tick) : nlohmann::json(nullptr)},
                           {"chord", chord_tick ? or_null(chords.at(*chord_tick)) : nlohmann::json(nullptr)},
                           {"end_tick", end ? nlohmann::json(*end) : nlohmann::json(nullptr)},
                           {"end_ms", end ? nlohmann::json(ms_of(*end)) : nlohmann::json(nullptr)},
                           {"anchor_tick", anchor ? nlohmann::json(*anchor) : nlohmann::json(nullptr)},
                           {"anchor_chord", anchor ? or_null(chords.at(*anchor)) : nlohmann::json(nullptr)},
                           {"ratio", scale_json(ratio)},
                           {"needs_no_timing", q.needs_no_timing()}};
        one["steady"] = end && anchor ? spans_json(steady_spans(*anchor, *end, timing))
                                      : nlohmann::json(nullptr);
        squeezes.push_back(std::move(one));
    }

    return nlohmann::json{
        {"act_tick", act_tick},
        {"chord", or_null(chords.at(act_tick))},
        {"nominal_end_tick", nominal ? nlohmann::json(*nominal) : nlohmann::json(nullptr)},
        {"nominal_end_ms", nominal ? nlohmann::json(ms_of(*nominal)) : nlohmann::json(nullptr)},
        {"deact_tick", deact ? nlohmann::json(*deact) : nlohmann::json(nullptr)},
        {"deact_ms", deact ? nlohmann::json(ms_of(*deact)) : nlohmann::json(nullptr)},
        {"read_end_after_tick", read_after ? nlohmann::json(*read_after) : nlohmann::json(nullptr)},
        {"read_end_after", read_after ? or_null(chords.at(*read_after)) : nlohmann::json(nullptr)},
        {"end_ratio", scale_json(act.transfer_post)},
        {"skipped_fill_ticks", act.skipped_fill_ticks},
        {"sp_end_steps", steps},
        {"squeezes", squeezes},
    };
}

nlohmann::json schedule_json(const Song& song, const std::vector<const Activation*>& acts,
                             const ReplayResult& replay, DeclineMode mode) {
    const ChordIndex chords = chord_index(song);
    std::vector<std::string> refusals;

    nlohmann::json act_list = nlohmann::json::array();
    std::map<int64_t, size_t> act_number;  // act tick -> place in path order
    std::set<int64_t> declined;
    std::set<int64_t> final_ends;
    for (size_t a = 0; a < acts.size(); ++a) {
        const Activation& act = *acts[a];
        nlohmann::json one = activation_json(act, chords, song.timing(), refusals);
        one["index"] = a;
        act_list.push_back(std::move(one));
        act_number[act.timecode.ticks()] = a;
        declined.insert(act.skipped_fill_ticks.begin(), act.skipped_fill_ticks.end());
        if (const std::optional<int64_t> d = act.deact_tick()) final_ends.insert(*d);
    }

    std::vector<double> chord_ms;
    chord_ms.reserve(replay.chords.size());
    for (const ReplayChord& c : replay.chords) chord_ms.push_back(c.ms);
    const std::vector<double> sides = chord_sides_ms(chord_ms);
    const size_t n = replay.chords.size();

    // Each chord's gem multipliers, its pads, and the gems the player sends:
    // every gem, or a declined chord's kept gems.
    std::vector<std::vector<int>> mults(n);
    std::vector<std::vector<size_t>> sent(n);
    std::vector<PadChord> pad_chords(n);
    for (size_t i = 0; i < n; ++i) {
        const ReplayChord& c = replay.chords[i];
        PadChord& pc = pad_chords[i];
        pc.index = c.index;
        pc.ms = c.ms;
        pc.side_ms = sides[i];
        for (const ReplayNote& note : c.notes) {
            mults[i].push_back(note.multiplier);
            pc.chart_pads.push_back({note.color, note.cymbal});
        }
        if (declined.count(c.tick) > 0) {
            sent[i] = kept_gems(mode, song.sequence[i].chord, c.notes).places;
        } else {
            for (size_t k = 0; k < c.notes.size(); ++k) sent[i].push_back(k);
        }
        for (size_t k : sent[i]) pc.sent_pads.push_back(pc.chart_pads[k]);
    }

    nlohmann::json chord_list = nlohmann::json::array();
    for (size_t i = 0; i < replay.chords.size(); ++i) {
        const ReplayChord& c = replay.chords[i];
        nlohmann::json gems = nlohmann::json::array();
        for (size_t k = 0; k < c.notes.size(); ++k) {
            nlohmann::json g = replay_note_json(c.notes[k]);
            g["place"] = k;
            gems.push_back(std::move(g));
        }
        nlohmann::json paid = nlohmann::json::array();
        for (const ReplayPayment& p : c.paid_by) {
            const auto act_place = act_number.find(p.act_tick);
            paid.push_back(nlohmann::json{
                {"activation", act_place != act_number.end() ? nlohmann::json(act_place->second) : nlohmann::json(nullptr)},
                {"act_tick", p.act_tick},
                {"offset_ms", p.offset_ms}});
        }
        const bool is_declined = declined.count(c.tick) > 0;
        nlohmann::json row{{"index", c.index},
                           {"tick", c.tick},
                           {"ms", c.ms},
                           {"side_ms", sides[i]},
                           {"gems", gems},
                           {"ends_sp", final_ends.count(c.tick) > 0},
                           {"declined", is_declined},
                           {"paid_by", paid}};
        if (is_declined) {
            row["kept_gems"] = sent[i];
        }
        row["reads"] = nlohmann::json::array();
        chord_list.push_back(std::move(row));
    }

    // Every skipped fill must sit on a chord, or the two inputs are not of
    // one chart.
    for (int64_t t : declined)
        if (!chords.at(t))
            refusals.push_back("no chord at skipped fill " + tick_text(t));

    const SongTiming& timing = song.timing();
    auto ms_of = [&](int64_t tick) { return timing.timecode(tick).ms(); };
    auto valid = [&](int chord) { return chord >= 0 && static_cast<size_t>(chord) < n; };

    // ---- each timed squeeze's geometry, grouped by the chord that leads it
    struct Timed {
        size_t act;
        size_t k;
        SqueezeGeometry g;
        std::optional<SqueezeSplit> split;
    };
    std::vector<Timed> timed;
    std::map<int, std::vector<size_t>> by_anchor;               // anchor chord -> timed
    std::map<int, std::pair<size_t, size_t>> squeeze_at;        // squeeze chord -> (activation, squeeze)
    std::map<std::pair<size_t, size_t>, size_t> timed_of;       // (activation, squeeze) -> timed
    for (size_t a = 0; a < acts.size(); ++a) {
        const Activation& act = *acts[a];
        for (size_t k = 0; k < act.sqinouts.size(); ++k) {
            // activation_json read these off the engine; a null was refused there.
            const nlohmann::json& q = act_list[a]["squeezes"][k];
            if (q["chord"].is_null() || q["anchor_chord"].is_null() || q["end_tick"].is_null()) continue;
            const int cs = q["chord"].get<int>();
            const int ca = q["anchor_chord"].get<int>();
            if (!valid(cs) || !valid(ca)) continue;
            if (squeeze_at.count(cs) > 0) {
                refusals.push_back("chord " + std::to_string(cs) + " is two squeezes; no rule times both");
                continue;
            }
            squeeze_at[cs] = {a, k};
            const SPSqueeze& sq = act.sqinouts[k];
            if (sq.needs_no_timing()) continue;
            const std::optional<TransferScale>& ratio =
                sq.kind == SqueezeKind::SqIn ? sq.transfer : act.transfer_post;
            if (!ratio) continue;  // activation_json's refusal names it
            SqueezeGeometry g;
            g.kind = sq.kind;
            g.anchor_chord = ca;
            g.anchor_ms = replay.chords[static_cast<size_t>(ca)].ms;
            g.anchor_side_ms = sides[static_cast<size_t>(ca)];
            g.squeeze_chord = cs;
            g.squeeze_ms = replay.chords[static_cast<size_t>(cs)].ms;
            g.squeeze_side_ms = sides[static_cast<size_t>(cs)];
            g.end_ms = q["end_ms"].get<double>();
            g.ratio = *ratio;
            g.steady = steady_spans(q["anchor_tick"].get<int64_t>(), q["end_tick"].get<int64_t>(), timing);
            timed_of[{a, k}] = timed.size();
            by_anchor[ca].push_back(timed.size());
            timed.push_back({a, k, g, std::nullopt});
        }
    }

    // ---- one lead per anchor, then each squeeze's split at it
    std::map<int, double> lead_of;
    std::map<int, SqueezeKind> way_of;
    std::map<int, double> ratio_of;
    for (const auto& [anchor, list] : by_anchor) {
        std::vector<SqueezeGeometry> geos;
        for (size_t t : list) geos.push_back(timed[t].g);
        const std::optional<double> lead = anchor_lead_ms(geos, refusals);
        if (!lead) continue;
        lead_of[anchor] = *lead;
        way_of[anchor] = geos.front().kind;
        ratio_of[anchor] = moving_ratio(geos.front());
        for (size_t t : list) timed[t].split = split_at_lead(timed[t].g, *lead);
    }
    for (size_t a = 0; a < acts.size(); ++a) {
        for (size_t k = 0; k < acts[a]->sqinouts.size(); ++k) {
            nlohmann::json& q = act_list[a]["squeezes"][k];
            if (acts[a]->sqinouts[k].needs_no_timing()) {
                q["feasible"] = true;
                continue;
            }
            const auto it = timed_of.find({a, k});
            if (it == timed_of.end() || !timed[it->second].split) {
                q["feasible"] = false;  // a refusal says why
                continue;
            }
            const SqueezeSplit& s = *timed[it->second].split;
            q["anchor_aim_ms"] = s.anchor_aim_ms;
            q["squeeze_aim_ms"] = s.squeeze_aim_ms;
            q["anchor_room_ms"] = s.anchor_room_ms;
            q["squeeze_room_ms"] = s.squeeze_room_ms;
            q["planned_end_ms"] = s.planned_end_ms;
            q["aim_end_ms"] = s.aim_end_ms;
            q["feasible"] = s.feasible;
        }
    }

    // ---- the reads: after the activation's own press, after the chord its
    // final end moves with, and after every chord that leads a squeeze. Each
    // read sees the end in force after that chord's step; its planned value
    // is that end moved by the chord's press judged kJudgeEarlyMs after its
    // aim (design point 4).
    std::map<std::pair<size_t, int>, double> read_stored;  // (activation, chord) -> end as stored
    std::vector<double> final_shift(acts.size(), 0.0);
    std::vector<std::optional<int>> final_anchor(acts.size());
    std::map<int64_t, size_t> end_act;  // final end tick -> activation
    for (size_t a = 0; a < acts.size(); ++a) {
        const Activation& act = *acts[a];
        if (const std::optional<int64_t> d = act.deact_tick()) end_act[*d] = a;
        if (const std::optional<int64_t> t = act.deact_anchor_tick()) final_anchor[a] = chords.at(*t);
        std::set<int> read_chords;
        if (const std::optional<int> c = chords.at(act.timecode.ticks())) read_chords.insert(*c);
        if (final_anchor[a]) read_chords.insert(*final_anchor[a]);
        for (const Timed& t : timed)
            if (t.act == a) read_chords.insert(t.g.anchor_chord);
        for (int x : read_chords) {
            if (!valid(x)) continue;
            const int64_t tick = replay.chords[static_cast<size_t>(x)].tick;
            std::optional<double> stored;
            for (const SpEndStep& st : act.sp_end_steps)
                if (st.tick == tick) stored = ms_of(st.end_tick);
            if (!stored) {
                refusals.push_back("the activation at " + tick_text(act.timecode.ticks()) +
                                   " has no SP-end step on chord " + std::to_string(x) +
                                   ", so the end read after it cannot be planned");
                continue;
            }
            double shift = 0.0;
            if (lead_of.count(x) > 0) {
                const double l = lead_of[x];
                shift = way_of[x] == SqueezeKind::SqOut ? -ratio_of[x] * (l - kJudgeEarlyMs)
                                                        : ratio_of[x] * (l + kJudgeEarlyMs);
            } else if (act.transfer_post) {
                shift = act.transfer_post->late * (kJudgeEarlyMs - kPressLeadMs);
            } else {
                refusals.push_back("the activation at " + tick_text(act.timecode.ticks()) +
                                   " has no stored ratio (Activation::transfer_post is unset), so the end "
                                   "read after chord " + std::to_string(x) + " cannot be planned");
                continue;
            }
            read_stored[{a, x}] = *stored;
            chord_list[static_cast<size_t>(x)]["reads"].push_back(
                nlohmann::json{{"activation", a}, {"stored_end_ms", *stored}, {"planned_end_ms", *stored + shift}});
        }
        // An end a lead moves earlier puts the chords before it at risk; one
        // moved later leaves them inside already (design point 3 of
        // docs/superpowers/plans/2026-11-07-player-limits.md).
        if (final_anchor[a] && lead_of.count(*final_anchor[a]) > 0 &&
            way_of[*final_anchor[a]] == SqueezeKind::SqOut)
            final_shift[a] = -ratio_of[*final_anchor[a]] * (lead_of[*final_anchor[a]] - kJudgeEarlyMs);
    }

    // ---- the presses, chord by chord
    struct Press {
        size_t chord = 0;
        std::vector<size_t> gems;
        double aim_ms = 0.0;
        std::optional<double> after_ms;
        const char* rule = "plain";
        std::optional<size_t> act;       // timed_from_end
        std::optional<int> read_after;   // the chord whose read it is retimed from
        std::optional<double> aim_end_ms;
        nlohmann::json fallback;         // null: none
        bool checked = false;            // the far-press check reads it
    };
    std::vector<Press> presses;
    auto add_groups = [&](size_t i, const std::vector<std::vector<size_t>>& groups, double first_aim_ms,
                          bool checked, const char* rule, std::optional<size_t> act = std::nullopt,
                          std::optional<int> read_after = std::nullopt,
                          std::optional<double> aim_end_ms = std::nullopt) {
        for (size_t g = 0; g < groups.size(); ++g) {
            Press p;
            p.chord = i;
            p.gems = groups[g];
            p.aim_ms = first_aim_ms + static_cast<double>(g) * kChordOrderGapMs;
            if (g > 0) p.after_ms = static_cast<double>(g) * kChordOrderGapMs;
            p.rule = rule;
            p.act = act;
            p.read_after = read_after;
            p.aim_end_ms = aim_end_ms;
            p.checked = checked || g > 0;
            presses.push_back(std::move(p));
        }
    };
    // The activating note goes first on an activation's chord (row P27:
    // Chord::activation_note names it).
    auto flagged = [&](size_t i) -> std::optional<size_t> {
        const ReplayChord& c = replay.chords[i];
        if (act_number.count(c.tick) == 0 || c.notes.empty()) return std::nullopt;
        const NoteColor top = song.sequence[i].chord.activation_note().colortype;
        for (size_t k = 0; k < c.notes.size(); ++k)
            if (c.notes[k].color == top) return k;
        return std::nullopt;
    };
    // The end a timed press's aim assumed, in the terms of the read it is
    // retimed from: that read's stored end moved as the press's own end was.
    auto in_read_terms = [&](size_t a, int anchor, double moved_ms) -> std::optional<double> {
        const auto it = read_stored.find({a, anchor});
        if (it == read_stored.end()) return std::nullopt;
        return it->second + moved_ms;
    };

    for (size_t i = 0; i < n; ++i) {
        const ReplayChord& c = replay.chords[i];
        const int ci = static_cast<int>(i);
        const double side = sides[i];
        const std::vector<std::vector<size_t>> groups = press_groups(mults[i], sent[i], flagged(i));
        const bool on_end = final_ends.count(c.tick) > 0;

        if (lead_of.count(ci) > 0) {
            if (squeeze_at.count(ci) > 0 || on_end) {
                refusals.push_back("chord " + std::to_string(ci) +
                                   " leads a squeeze and is also a squeeze or an end; no rule times both");
                continue;
            }
            const double l = lead_of[ci];
            add_groups(i, groups, way_of[ci] == SqueezeKind::SqOut ? -l : l, true, "anchor");
            continue;
        }

        if (squeeze_at.count(ci) > 0) {
            const auto [a, k] = squeeze_at[ci];
            const auto it = timed_of.find({a, k});
            if (acts[a]->sqinouts[k].needs_no_timing()) {
                add_groups(i, groups, -kPressLeadMs, false, "needs_no_timing");
                continue;
            }
            if (it == timed_of.end() || !timed[it->second].split) continue;  // refused above
            const Timed& t = timed[it->second];
            const SqueezeSplit& s = *t.split;
            const int anchor = t.g.anchor_chord;
            const std::optional<double> aim_end = in_read_terms(a, anchor, s.aim_end_ms - t.g.end_ms);
            if (!aim_end) continue;  // the read's refusal names it
            // Thresholds in the same terms as the read.
            const double terms = *aim_end - s.aim_end_ms;
            if (t.g.kind == SqueezeKind::SqOut) {
                // _timed_presses in play_chart.py, moved: "The gem Hydra's
                // FirstNote rule pays outside is note 0 of
                // Chord::note_list(true)", so the late gem is place 0.
                const std::optional<std::vector<std::vector<size_t>>> inside =
                    squeeze_out_inside_groups(ci, mults[i], refusals);
                if (!inside) continue;
                // "The last inside group goes down at the inside time,
                // SP_END_LEAD_MS before the end, so no later group can land
                // close to the end check and pay outside."
                const double last_inside_aim = s.aim_end_ms - kSpEndLeadMs - c.ms;
                const double first_inside_aim =
                    last_inside_aim - static_cast<double>(inside->empty() ? 0 : inside->size() - 1) * kChordOrderGapMs;
                if (!inside->empty())
                    add_groups(i, *inside, first_inside_aim, true, "squeeze_out_inside", a, anchor, *aim_end);
                Press late;
                late.chord = i;
                late.gems = {0};
                late.aim_ms = s.squeeze_aim_ms;
                late.rule = "squeeze_out_late";
                late.act = a;
                late.read_after = anchor;
                late.aim_end_ms = *aim_end;
                late.checked = true;
                // The freak-fixes plan's question 3 (a): past the threshold
                // the late gem joins the last inside group, pressed
                // kSpEndLeadMs before the read end, and the row says
                // retimed=inside.
                late.fallback = nlohmann::json{
                    {"if_end_after_ms", sqout_fallback_after_ms(c.ms, side) + terms},
                    {"action", "join_group"},
                    {"join_gems", inside->empty() ? std::vector<size_t>{} : inside->back()},
                    {"before_end_ms", kSpEndLeadMs},
                    {"retimed", "inside"}};
                presses.push_back(std::move(late));
            } else {
                const size_t first = presses.size();
                add_groups(i, press_groups(mults[i], sent[i], std::nullopt), s.squeeze_aim_ms, true, "squeeze_in",
                           a, anchor, *aim_end);
                // Question 3 (a): before the threshold the chord is pressed
                // plain, so it scores and the combo holds.
                for (size_t p = first; p < presses.size(); ++p)
                    presses[p].fallback = nlohmann::json{
                        {"if_end_before_ms", sqin_fallback_before_ms(c.ms, side) + terms},
                        {"action", "plain"},
                        {"aim_ms", -kPressLeadMs + presses[p].after_ms.value_or(0.0)},
                        {"retimed", "plain"}};
            }
            continue;
        }

        // A chord on a final end (when its activation does not squeeze out)
        // goes kSpEndLeadMs before that end, as a lead moved it earlier.
        if (on_end && acts[end_act[c.tick]]->sqout_tick == std::nullopt) {
            const size_t a = end_act[c.tick];
            const double shift = final_shift[a];
            const std::optional<double> aim_end =
                final_anchor[a] ? in_read_terms(a, *final_anchor[a], shift) : std::nullopt;
            if (aim_end) {
                add_groups(i, groups, shift - kSpEndLeadMs, true, "sp_end", a, *final_anchor[a], *aim_end);
                continue;
            }
        }

        // A chord paid inside, before an end a lead moved earlier, goes
        // kSpEndLeadMs before that end when its plain press would come later.
        bool placed = false;
        for (const ReplayPayment& pay : c.paid_by) {
            const auto an = act_number.find(pay.act_tick);
            if (an == act_number.end()) continue;
            const size_t a = an->second;
            if (final_shift[a] >= 0.0 || !final_anchor[a]) continue;
            if (chords.at(pay.act_tick) == std::optional<int>(ci)) continue;
            const std::optional<int64_t> d = acts[a]->deact_tick();
            if (!d) continue;
            const double aim = ms_of(*d) + final_shift[a] - kSpEndLeadMs - c.ms;
            if (aim >= -kPressLeadMs) continue;
            const std::optional<double> aim_end = in_read_terms(a, *final_anchor[a], final_shift[a]);
            if (!aim_end) continue;
            add_groups(i, groups, aim, true, "inside_moved_end", a, *final_anchor[a], *aim_end);
            placed = true;
            break;
        }
        if (!placed) add_groups(i, groups, -kPressLeadMs, false, "plain");
    }

    // ---- rooms, latest sends, and the far-press check
    std::stable_sort(presses.begin(), presses.end(), [&](const Press& x, const Press& y) {
        return replay.chords[x.chord].ms + x.aim_ms < replay.chords[y.chord].ms + y.aim_ms;
    });
    std::set<size_t> far_refused;
    nlohmann::json press_list = nlohmann::json::array();
    for (const Press& p : presses) {
        const ReplayChord& c = replay.chords[p.chord];
        const double side = sides[p.chord];
        if (p.checked && !p.gems.empty() && far_refused.count(p.chord) == 0) {
            std::vector<Pad> pads;
            for (size_t g : p.gems) pads.push_back(pad_chords[p.chord].chart_pads[g]);
            if (const std::optional<std::string> r = far_press_refusal(pad_chords, p.chord, c.ms + p.aim_ms, pads)) {
                refusals.push_back(*r);
                far_refused.insert(p.chord);
            }
        }
        nlohmann::json row{{"chord", c.index},
                           {"gems", p.gems},
                           {"aim_ms", p.aim_ms},
                           {"after_ms", or_null(p.after_ms)},
                           {"rule", p.rule},
                           {"timed_from_end", p.act ? nlohmann::json(*p.act) : nlohmann::json(nullptr)},
                           {"end_read_after", or_null(p.read_after)},
                           {"aim_end_ms", or_null(p.aim_end_ms)},
                           {"room_ms", press_room_ms(p.aim_ms, side)},
                           {"last_send_ms", last_send_ms(c.ms, side)}};
        if (!p.fallback.is_null()) row["fallback"] = p.fallback;
        press_list.push_back(std::move(row));
    }

    return nlohmann::json{{"constants", constants_json()},
                          {"decline", decline_mode_name(mode)},
                          {"activations", act_list},
                          {"chords", chord_list},
                          {"presses", press_list},
                          {"refusals", refusals}};
}

// ---- the command's refusals ------------------------------------------------

std::optional<std::string> path_flag_refusal(const std::string& path_file) {
    if (!path_file.empty()) return std::nullopt;
    return "schedule needs --path <dump.json>: the path it plans is read from a file "
           "hydra_replay dump or target wrote";
}

std::optional<std::string> no_path_refusal(const nlohmann::json& doc, const std::string& path_file) {
    if (doc.is_object() && doc.contains("paths") && doc["paths"].is_array() && !doc["paths"].empty())
        return std::nullopt;
    return "--path file " + path_file + " holds no path, so the chart has no Star Power path to plan";
}

std::optional<std::string> decline_flag_refusal(const std::optional<std::string>& name) {
    if (!name)
        return "schedule needs --decline <mode>, what a declined chord sends: " + decline_mode_list();
    if (decline_mode_from_name(*name)) return std::nullopt;
    return "unknown decline mode \"" + *name + "\"; modes: " + decline_mode_list();
}

nlohmann::json schedule_for_windows(const Song& song, const SearchSettings& settings,
                                    const std::vector<ReplayWindow>& windows,
                                    DeclineMode mode) {
    TargetResult found = search_target(song, settings, pinned_windows(song, windows));
    if (found.paths.empty())
        throw std::runtime_error("the engine cannot realize this path: " + found.failed_reason);
    const Path& path = found.paths.front();
    std::vector<const Activation*> acts;
    for (const Activation& act : path.walk_activations()) acts.push_back(&act);
    const ReplayResult replay = replay_path(song, windows_for_path(path), settings.rules);
    return schedule_json(song, acts, replay, mode);
}

}  // namespace hydra::schedule
