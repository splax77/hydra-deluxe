#include "replay_json.h"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

#include "core/squeeze_rating.h"  // steady_spans
#include "core/timing.h"  // SongTiming
#include "store/record_store.h"  // record_of, RecordRow::best_path_text

namespace hydra {

namespace {

// Which field names this window's squeezed-out chord. The stamped tick does
// when the activation has one (-1 or no key means none). Only without it is
// the SqOut entry's offset read, for dumps written before the tick existed;
// `score` in tools/replay.cpp resolves that offset with resolve_window_sqout.
// That bare offset is the one deliberate exception to the record reader
// (windows_for_path takes a squeeze-out's tick and offset from one stored
// row); only an old or hand-edited JSON carries one, and the CLI prints which
// chord it picked, so the user sees the guess. The writer below still writes
// both, so an older hydra_replay can read a new dump.
void read_sqout(const nlohmann::json& act, const std::string& where, ReplayWindow& w) {
    if (act.contains("sqout_tick") && act["sqout_tick"].is_number() &&
        act["sqout_tick"].get<int64_t>() >= 0) {
        w.sqout_tick = act["sqout_tick"].get<int64_t>();
        return;
    }
    if (!act.contains("sqinouts") || !act["sqinouts"].is_array()) return;
    for (const nlohmann::json& sq : act["sqinouts"]) {
        if (!sq.is_object()) continue;
        if (squeeze_kind_from_name(sq.value("kind", std::string())) != SqueezeKind::SqOut)
            continue;
        if (!sq.contains("offset_ms") || !sq["offset_ms"].is_number())
            throw std::runtime_error(where + " has a SqOut with no offset_ms");
        w.sqout_offset_ms = sq["offset_ms"].get<double>();
    }
}

}  // namespace

std::vector<ReplayWindow> windows_from_json(const nlohmann::json& path) {
    if (!path.is_object() || !path.contains("activations") ||
        !path["activations"].is_array())
        throw std::runtime_error("this path has no \"activations\" array");

    std::vector<ReplayWindow> out;
    int index = 0;
    for (const nlohmann::json& act : path["activations"]) {
        const std::string where = "activation " + std::to_string(index++);
        if (!act.is_object() || !act.contains("act_tick") ||
            !act.contains("deact_tick") || !act["act_tick"].is_number() ||
            !act["deact_tick"].is_number())
            throw std::runtime_error(where +
                                     " has no act_tick/deact_tick number");

        ReplayWindow w;
        w.act_tick = act["act_tick"].get<int64_t>();
        w.deact_tick = act["deact_tick"].get<int64_t>();
        // -1 is how the dump writes a missing value. A window with no
        // deactivation node cannot be replayed, and guessing one would print a
        // wrong score with no hint why.
        if (w.act_tick < 0)
            throw std::runtime_error(where + " has no activation tick");
        if (w.deact_tick < 0)
            throw std::runtime_error(
                where +
                " has no deactivation node; the record it came from predates "
                "the field, so this path cannot be replayed");
        if (w.deact_tick < w.act_tick)
            throw std::runtime_error(where +
                                     " deactivates before it activates");

        read_sqout(act, where, w);

        // The phrases this window squeezed in (D34). No key (a dump written
        // before the field) reads as none.
        if (act.contains("sqin_ticks") && act["sqin_ticks"].is_array())
            for (const nlohmann::json& t : act["sqin_ticks"])
                if (t.is_number()) w.sqin_ticks.push_back(t.get<int64_t>());

        // A dumped window came off a record, so it says how it ended (ADR
        // 0014). No key (an older dump, or a typed list) reads as a typed
        // window.
        w.from_record = act.value("from_record", false);
        out.push_back(w);
    }
    return out;
}

nlohmann::json score_json(const ReplayScore& s) {
    nlohmann::json j = nlohmann::json::object();
    for (const ReplayScoreField& f : kReplayScoreFields) j[f.name] = s.*(f.member);
    return j;
}

namespace {

nlohmann::json or_null(const std::optional<double>& v) {
    return v ? nlohmann::json(*v) : nlohmann::json(nullptr);
}

}  // namespace

// The path list `dump` and `target` both print. One shape, so anything that
// reads dump's JSON reads target's too. nominal_deact_tick comes off the
// stored history (nominal_end()). The end fields, each null when the
// activation has no deact tick:
//   end_anchor_tick         - the note whose timing moves the end
//                             (Activation::deact_anchor_tick).
//   end_scale_early / _late - how far the end moves per ms of early or late
//                             activation: the transfer_post the search stored,
//                             never worked out again here; null when the
//                             search stored none.
//   steady_ms_before_act / _after_act - from end_anchor_tick, and
//   steady_ms_before_end / _after_end - from deact_tick: how far the ratios
//                             stay exact (steady_spans, core/squeeze_rating.h).
//                             null on a side means no change on that side.
nlohmann::json paths_json(const std::vector<const Path*>& all, const SongTiming& timing) {
    nlohmann::json paths = nlohmann::json::array();
    int index = 0;
    for (const Path* p : all) {
        nlohmann::json acts = nlohmann::json::array();
        for (const Activation& act : p->walk_activations()) {
            nlohmann::json sq = nlohmann::json::array();
            for (const SPSqueeze& s2 : act.sqinouts)
                sq.push_back(nlohmann::json{{"kind", s2.type_name()},
                                            {"offset_ms", s2.offset()}});

            const int64_t act_tick = act.timecode.ticks();
            const std::optional<int64_t> d = act.deact_tick();
            // The plain end the activation's bars gave, read off the history.
            const int64_t nominal = act.nominal_end().value_or(-1);
            // The phrases it squeezed in, so `score --path` skips them as the
            // engine did (D34).
            const nlohmann::json sqins = sqin_phrase_ticks(act);
            const std::optional<int64_t> anchor = act.deact_anchor_tick();
            const std::optional<int64_t> ready = act.sp_ready_tick();
            nlohmann::json end_anchor = nullptr;
            nlohmann::json end_scale_early = nullptr;
            nlohmann::json end_scale_late = nullptr;
            SteadySpans spans;
            if (d && anchor) {
                end_anchor = *anchor;
                if (act.transfer_post) {
                    end_scale_early = act.transfer_post->early;
                    end_scale_late = act.transfer_post->late;
                }
                spans = steady_spans(*anchor, *d, timing);
            }

            acts.push_back(nlohmann::json{
                {"act_tick", act_tick},
                {"deact_tick", d ? *d : -1},
                {"sqout_tick", act.sqout_tick ? *act.sqout_tick : -1},
                {"nominal_deact_tick", nominal},
                {"sp_meter", act.sp_meter()},
                // Activation::sp_ready_tick; null when too few bars are
                // stored to reach the activation level.
                {"sp_ready_tick", ready ? nlohmann::json(*ready) : nlohmann::json(nullptr)},
                {"sp_ready_ms", ready ? nlohmann::json(timing.timecode(*ready).ms())
                                      : nlohmann::json(nullptr)},
                {"skips", act.skips()},
                // The stored list the engine wrote, never recomputed: under
                // the 1.0 fill rule it need not be the fills nearest the act.
                {"skipped_fill_ticks", act.skipped_fill_ticks},
                {"chord_code", act.chord.code()},
                {"sqinouts", sq},
                {"sqin_ticks", sqins},
                {"from_record", true},
                {"end_anchor_tick", end_anchor},
                {"end_scale_early", end_scale_early},
                {"end_scale_late", end_scale_late},
                {"steady_ms_before_act", or_null(spans.before_anchor_ms)},
                {"steady_ms_after_act", or_null(spans.after_anchor_ms)},
                {"steady_ms_before_end", or_null(spans.before_end_ms)},
                {"steady_ms_after_end", or_null(spans.after_end_ms)},
            });
        }
        paths.push_back(nlohmann::json{
            {"index", index++},
            {"pathstring", p->pathstring()},
            {"total", p->totalscore()},
            {"score", score_json(score_of(*p))},
            {"activations", acts},
        });
    }
    return paths;
}

nlohmann::json result_json(const HydraRecord& rec) {
    // HydraRecord::optimal_score owns "what is this record's best score",
    // absent when the record holds no paths; RecordRow::best_path_text owns
    // its text. The replay has no profile, so it builds the record unchecked.
    const std::optional<int64_t> best = rec.optimal_score();
    return nlohmann::json{{"score", best ? nlohmann::json(*best) : nlohmann::json(nullptr)},
                          {"bestpath", store::record_of(std::string(), rec).best_path_text()}};
}

}  // namespace hydra
