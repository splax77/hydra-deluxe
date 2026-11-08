#include "replay_schedule.h"

#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>

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
            const auto n = act_number.find(p.act_tick);
            paid.push_back(nlohmann::json{
                {"activation", n != act_number.end() ? nlohmann::json(n->second) : nlohmann::json(nullptr)},
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
            row["kept_gems"] = kept_gems(mode, song.sequence[i].chord, c.notes).places;
        }
        chord_list.push_back(std::move(row));
    }

    // Every skipped fill must sit on a chord, or the two inputs are not of
    // one chart.
    for (int64_t t : declined)
        if (!chords.at(t))
            refusals.push_back("no chord at skipped fill " + tick_text(t));

    // The aims, rooms, fallbacks and feasibility come with the next task of
    // the plan; until then the player has nothing to send.
    refusals.push_back("the presses are not planned yet: this schedule holds the geometry only");

    return nlohmann::json{{"constants", constants_json()},
                          {"decline", decline_mode_name(mode)},
                          {"activations", act_list},
                          {"chords", chord_list},
                          {"presses", nlohmann::json::array()},
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
