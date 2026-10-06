#include "replay_json.h"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

#include "core/timing.h"  // SongTiming
#include "store/record_store.h"  // summarize_record

namespace hydra {

namespace {

// Which field names this window's squeezed-out chord. The stamped tick does
// when the activation has one (-1 or no key means none). Only without it is
// the SqOut entry's offset read, for dumps written before the tick existed;
// `score` in tools/replay.cpp resolves that offset with resolve_sqout_note.
// That bare offset is the one deliberate exception to the record reader's
// rule (windows_for_path drops a window it cannot name a squeeze-out chord
// for); only an old or hand-edited JSON carries one, and the CLI prints which
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
        // before the field) reads as none, as it always did.
        if (act.contains("sqin_ticks") && act["sqin_ticks"].is_array())
            for (const nlohmann::json& t : act["sqin_ticks"])
                if (t.is_number()) w.sqin_ticks.push_back(t.get<int64_t>());

        // A dumped window came off a record, so it says how it ended (D36).
        // No key (an older dump, or a typed list) reads as a typed window.
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

// The path list `dump` and `target` both print. One shape, so anything that
// reads dump's JSON reads target's too. The chart timing is no longer read
// here: nominal_deact_tick comes off the stored history (nominal_end()).
nlohmann::json paths_json(const std::vector<const Path*>& all, const SongTiming& /*timing*/) {
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

            acts.push_back(nlohmann::json{
                {"act_tick", act_tick},
                {"deact_tick", d ? *d : -1},
                {"sqout_tick", act.sqout_tick ? *act.sqout_tick : -1},
                {"nominal_deact_tick", nominal},
                {"sp_meter", act.sp_meter()},
                {"skips", act.skips()},
                // The stored list the engine wrote, never recomputed: under
                // the 1.0 fill rule it need not be the fills nearest the act.
                {"skipped_fill_ticks", act.skipped_fill_ticks},
                {"chord_code", act.chord.code()},
                {"sqinouts", sq},
                {"sqin_ticks", sqins},
                {"from_record", true},
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
    // summarize_record owns "what is this record's best score", absent when
    // the record holds no paths; best_path_text owns its text.
    const std::optional<int64_t> best = store::summarize_record(rec).score;
    return nlohmann::json{{"score", best ? nlohmann::json(*best) : nlohmann::json(nullptr)},
                          {"bestpath", store::best_path_text(rec)}};
}

}  // namespace hydra
