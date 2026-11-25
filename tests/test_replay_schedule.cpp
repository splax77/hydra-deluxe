// hydra_replay schedule (tools/replay_schedule.h): the geometry it reads off
// the engine, the game's per-note window and the constants, and the rule that
// nothing under src/ reads any of it.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "json.hpp"

#include "app/analysis.h"
#include "app/config.h"
#include "app/song_length.h"
#include "core/model.h"
#include "core/note_window.h"
#include "core/replay.h"
#include "core/rules.h"
#include "core/scoring.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "replay_schedule.h"
#include "source_tree.h"  // for_each_source_file, the shared walk of the repo

using namespace hydra;
using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {

// A plain chart at 120 BPM, 192 ticks a beat (2.604 ms a tick), with one Red
// chord on each tick of `ticks`.
Song plain_song(const std::vector<int64_t>& ticks) {
    Song song(192);
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (int64_t tick : ticks) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);
        song.sequence.push_back(ts);
    }
    return song;
}

// The colours a declined chord of `colours` sends: the chord is built for
// real, so its notes come in Chord::note_list(true) order and its activation
// note is the engine's.
std::set<NoteColor> kept_colours(const std::set<NoteColor>& colours) {
    Chord chord;
    for (NoteColor c : colours) chord.add_note(c);
    std::vector<ReplayNote> notes;
    const Chord::NoteList ordering = chord.note_list(true);
    for (size_t k = 0; k < ordering.size(); ++k) {
        ReplayNote r;
        r.color = ordering[k].colortype;
        notes.push_back(r);
    }
    std::set<NoteColor> out;
    for (size_t k : schedule::kept_gems(chord, notes).places) out.insert(notes[k].color);
    return out;
}

// Every stored path of every corpus chart under `cfg`: replayed, and its
// schedule built by schedule::schedule_json. Every chart the corpus scan
// lists has Expert drum notes, so a chart that loads empty fails here by name
// rather than being passed over.
// `check` gets the chart's path, its song, the path's activations in walk
// order, the replay and the schedule.
using ScheduleCheck = std::function<void(const std::string& path, const Song& song,
                                         const std::vector<const Activation*>& acts,
                                         const PathReplay& pr, const json& sch)>;

// One stored path of a corpus chart: its activations in walk order, its
// replay and its schedule. The activation pointers point into the record
// corpus::analyzed caches for the run; the replay and the schedule are
// plain values.
struct StoredPathSchedule {
    std::vector<const Activation*> acts;
    PathReplay replay;
    json schedule;
};

// Every stored path of corpus chart `path` under `cfg`, in all_paths()
// order, replayed and scheduled once per run and shared by every case.
// Keyed the way corpus::analyzed keys its records, so other settings get
// their own entry. A throw is kept and rethrown to every caller, as
// corpus::analyzed does.
const std::vector<StoredPathSchedule>& stored_path_schedules(const std::string& path,
                                                             const app::AnalysisSettings& cfg) {
    static std::map<std::string, corpus::detail::Outcome<std::vector<StoredPathSchedule>>> cache;
    std::ostringstream key;
    key << path << '|' << app::chartmode_key(cfg.reading()) << '|' << corpus::settings_key(cfg);
    auto [it, fresh] = cache.try_emplace(key.str());
    corpus::detail::Outcome<std::vector<StoredPathSchedule>>& o = it->second;
    if (fresh) {
        try {
            const Song& song = corpus::song(path, cfg.reading());
            const HydraRecord& rec = corpus::analyzed(path, cfg);
            std::vector<StoredPathSchedule> out;
            for (const Path* p : rec.all_paths()) {
                StoredPathSchedule s;
                for (const Activation& a : p->walk_activations()) s.acts.push_back(&a);
                s.replay = replay_stored_path(song, *p, cfg.rules);
                s.schedule = schedule::schedule_json(song, s.acts, s.replay.result, cfg.rules);
                out.push_back(std::move(s));
            }
            o.value.emplace(std::move(out));
        } catch (...) {
            o.error = std::current_exception();
        }
    }
    if (o.error) std::rethrow_exception(o.error);
    return *o.value;
}

void for_each_corpus_schedule(const app::AnalysisSettings& cfg, const ScheduleCheck& check) {
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.reading());
        const std::string empty_msg = path + " loads with no notes";
        CHECK_MESSAGE(!song.is_empty(), empty_msg);
        if (song.is_empty()) continue;
        for (const StoredPathSchedule& s : stored_path_schedules(path, cfg))
            check(path, song, s.acts, s.replay, s.schedule);
    }
}

// Every activation of every stored corpus path (for_each_corpus_schedule),
// with the activation_json it writes over its chart's whole chord list and
// the refusal lines that call added.
using ActivationCheck = std::function<void(const std::string& path, const Activation& act, const json& j,
                                           const std::vector<std::string>& refusals)>;

void for_each_corpus_activation_json(const app::AnalysisSettings& cfg, const ActivationCheck& check) {
    for_each_corpus_schedule(cfg, [&](const std::string& path, const Song& song,
                                      const std::vector<const Activation*>& acts, const PathReplay&,
                                      const json&) {
        const ChordTimes chords = chord_times(song);
        for (const Activation* a : acts) {
            std::vector<std::string> refusals;
            const json j = schedule::activation_json(*a, chords, song.timing(), refusals);
            check(path, *a, j, refusals);
        }
    });
}

}  // namespace

TEST_CASE("schedule: every end step names its anchor") {
    // On the corpus: an end is measured from the latest clamped step at or
    // before it, else from the activation's own chord. Each step the file
    // writes names that anchor, Activation::end_anchor_tick agrees, and the
    // final end is read after the last step's anchor.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    for_each_corpus_activation_json(cfg, [&](const std::string& path, const Activation& act, const json& j,
                                        const std::vector<std::string>&) {
        INFO(path << ": the activation at tick " << act.timecode.ticks());
        const json& steps = j["sp_end_steps"];
        REQUIRE(steps.size() == act.sp_end_steps.size());
        int64_t anchor = act.timecode.ticks();
        for (size_t s = 0; s < act.sp_end_steps.size(); ++s) {
            const SpEndStep& st = act.sp_end_steps[s];
            if (st.kind == SpEndKind::Clamped) anchor = st.tick;
            INFO("step " << s);
            CHECK(steps[s]["tick"].get<int64_t>() == st.tick);
            CHECK(steps[s]["end_tick"].get<int64_t>() == st.end_tick);
            CHECK(steps[s]["anchor_tick"].get<int64_t>() == anchor);
            CHECK(act.end_anchor_tick(s) == anchor);
        }
        if (act.sp_end_steps.empty()) {
            CHECK(j["read_end_after_tick"].is_null());
        } else {
            CHECK(j["read_end_after_tick"].get<int64_t>() == anchor);
        }
    });
}

TEST_CASE("schedule: a squeeze carries its own end, anchor and stored ratio") {
    // On the corpus: each squeeze in the file names the end, anchor and chord
    // the record gives it, and the ratio the record stored (a SqIn's own,
    // a SqOut's on transfer_post), null with a refusal line when unset.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int squeezes = 0;
    for_each_corpus_activation_json(cfg, [&](const std::string& path, const Activation& act, const json& j,
                                        const std::vector<std::string>& refusals) {
        INFO(path << ": the activation at tick " << act.timecode.ticks());
        const json& q = j["squeezes"];
        REQUIRE(q.size() == act.sqinouts.size());
        for (size_t k = 0; k < act.sqinouts.size(); ++k) {
            ++squeezes;
            INFO("squeeze " << k);
            const std::optional<int64_t> end = act.squeeze_end_tick(k);
            const std::optional<int64_t> anchor = act.squeeze_anchor_tick(k);
            const std::optional<int64_t> chord = act.squeeze_chord_tick(k);
            CHECK(q[k]["end_tick"] == (end ? json(*end) : json(nullptr)));
            CHECK(q[k]["anchor_tick"] == (anchor ? json(*anchor) : json(nullptr)));
            CHECK(q[k]["chord_tick"] == (chord ? json(*chord) : json(nullptr)));
            const std::optional<TransferScale>& stored = stored_scale(act, k);
            if (stored) {
                CHECK(q[k]["ratio"]["early"].get<double>() == stored->early);
                CHECK(q[k]["ratio"]["late"].get<double>() == stored->late);
            } else {
                CHECK(q[k]["ratio"]["early"].is_null());
                CHECK(q[k]["ratio"]["late"].is_null());
                const std::string field =
                    &stored == &act.transfer_post ? "Activation::transfer_post" : "SPSqueeze::transfer";
                CHECK(std::any_of(refusals.begin(), refusals.end(),
                                  [&](const std::string& r) { return r.find(field) != std::string::npos; }));
            }
        }
    });
    // The ends_sp test below meets early squeeze-outs on this same corpus, so
    // the sweep holds squeezes; none read means the walk lost them.
    CHECK(squeezes > 0);
}

TEST_CASE("schedule: a squeeze's ratio and anchor press are the owner's") {
    // On every corpus path: each squeeze's written ratio is stored_scale's, and
    // each timed squeeze's anchor is pressed on squeeze_multiplier's side, the
    // side the badge's safe sum is priced on (core/squeeze_rating).
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int ratios = 0;
    int presses = 0;
    for_each_corpus_schedule(cfg, [&](const std::string& path, const Song&, const std::vector<const Activation*>& acts,
                                      const PathReplay&, const json& sch) {
        const json& list = sch["activations"];
        REQUIRE(list.size() == acts.size());
        for (size_t a = 0; a < acts.size(); ++a) {
            const Activation& act = *acts[a];
            INFO(path << ": the activation at tick " << act.timecode.ticks());
            REQUIRE(list[a]["squeezes"].size() == act.sqinouts.size());
            for (size_t k = 0; k < act.sqinouts.size(); ++k) {
                INFO("squeeze " << k);
                const json& q = list[a]["squeezes"][k];
                const std::optional<TransferScale>& stored = stored_scale(act, k);
                const std::optional<PricedScale> priced = squeeze_multiplier(act, k);
                CHECK(priced.has_value() == stored.has_value());
                if (stored) {
                    ++ratios;
                    CHECK(q["ratio"]["early"].get<double>() == stored->early);
                    CHECK(q["ratio"]["late"].get<double>() == stored->late);
                } else {
                    CHECK(q["ratio"]["early"].is_null());
                    CHECK(q["ratio"]["late"].is_null());
                }
                if (!q.contains("anchor_aim_ms")) continue;  // not timed: no anchor press
                REQUIRE(priced.has_value());
                ++presses;
                // split_at_lead writes the anchor's aim as its lead, negative
                // when the anchor is pressed early. A lead is never below zero
                // (anchor_lead_ms refuses one), so the sign bit carries the
                // press even at a lead of zero.
                const Press written = std::signbit(q["anchor_aim_ms"].get<double>()) ? Press::Early : Press::Late;
                CHECK(written == priced->press);
            }
        }
    });
    // A corpus with no stored scale or no timed squeeze would pass every
    // check above whatever the tool read.
    CHECK(ratios > 0);
    CHECK(presses > 0);
}

TEST_CASE("schedule: paid_by matches in_sp on every corpus path") {
    // On the corpus: each payment a chord carries names one of the path's
    // windows, the chord sits at or after that window's activation, and the
    // payment's offset is the chord's distance from that window's SP end on
    // the chart's own clock.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.reading());
        if (song.is_empty()) continue;  // no notes to read under cfg, so no path to replay
        INFO(path);
        for (const StoredPathSchedule& s : stored_path_schedules(path, cfg)) {
            const PathReplay& pr = s.replay;
            for (const ReplayChord& c : pr.result.chords) {
                INFO("chord " << c.index);
                for (const ReplayPayment& pay : c.paid_by) {
                    const auto w = std::find_if(pr.windows.begin(), pr.windows.end(), [&](const ReplayWindow& x) {
                        return x.act_tick == pay.act_tick && x.deact_tick == pay.deact_tick;
                    });
                    REQUIRE(w != pr.windows.end());
                    CHECK(c.tick >= w->act_tick);
                    CHECK(pay.offset_ms == offset_from_sp_end(c.ms, song.timing().timecode(w->deact_tick).ms()));
                }
            }
        }
    }
}

TEST_CASE("schedule: declined chords are the skipped fill ticks") {
    // On the corpus: the declined rows are exactly the skipped fills, and
    // each one sends every gem of its chord but the chord's activation note
    // (Chord::activation_note).
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    size_t declined_rows = 0;
    size_t skipped_ticks = 0;
    for_each_corpus_schedule(cfg, [&](const std::string& path, const Song& song,
                                      const std::vector<const Activation*>& acts, const PathReplay& pr,
                                      const json& s) {
        INFO(path);
        std::set<int64_t> skipped;
        for (const Activation* a : acts)
            skipped.insert(a->skipped_fill_ticks.begin(), a->skipped_fill_ticks.end());
        skipped_ticks += skipped.size();
        std::set<int64_t> marked;
        for (const json& row : s["chords"]) {
            if (!row["declined"].get<bool>()) continue;
            ++declined_rows;
            marked.insert(row["tick"].get<int64_t>());
            const ReplayChord& c = pr.result.chords[row["index"].get<size_t>()];
            const Chord& chord = song.sequence[static_cast<size_t>(c.index)].chord;
            std::multiset<NoteColor> want;
            for (const ChordNote& n : chord.note_list(true))
                if (n.colortype != chord.activation_note().colortype) want.insert(n.colortype);
            std::multiset<NoteColor> sent;
            for (size_t k : row["kept_gems"].get<std::vector<size_t>>()) sent.insert(c.notes.at(k).color);
            INFO("chord " << c.index);
            CHECK(sent == want);
        }
        CHECK(marked == skipped);
    });
    CHECK(declined_rows == skipped_ticks);
    // With no skipped fill on any stored path every check above is empty.
    CHECK(skipped_ticks > 0);
}

TEST_CASE("schedule: ends_sp marks the chord where each window's Star Power is over") {
    // On the corpus: the rows marked ends_sp are exactly the chords whose tick
    // is some activation's Activation::window_end_tick(), read from the record.
    // An early squeeze-out is the case where that chord is not the
    // deactivation node, so the sweep must meet at least one.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int early_sqouts = 0;
    for_each_corpus_schedule(cfg, [&](const std::string& path, const Song&,
                                      const std::vector<const Activation*>& acts, const PathReplay& pr,
                                      const json& s) {
        std::set<int64_t> chord_ticks;
        for (const ReplayChord& c : pr.result.chords) chord_ticks.insert(c.tick);
        std::set<int64_t> want;
        for (const Activation* a : acts) {
            const std::optional<int64_t> end = a->window_end_tick();
            if (end && chord_ticks.count(*end) > 0) want.insert(*end);
            if (a->sqout_tick && end == a->sqout_tick && a->deact_tick() != a->sqout_tick)
                ++early_sqouts;
        }
        std::set<int64_t> marked;
        for (const json& row : s["chords"])
            if (row["ends_sp"].get<bool>()) marked.insert(row["tick"].get<int64_t>());
        INFO(path);
        CHECK(marked == want);
    });
    CHECK(early_sqouts > 0);
}

TEST_CASE("schedule: the window constants are written") {
    // The constants the file writes are the game's window edges and formula
    // constants, core/note_window.h's, and nothing of the schedule's own.
    const Song song = plain_song({0, 768});
    const ReplayResult r = replay_path(song, {});
    const json s = schedule::schedule_json(song, {}, r, core::default_rules());
    const json& c = s["constants"];
    CHECK(c["window_front_ms"].get<double>() == kWindowFrontMs);
    CHECK(c["window_back_ms"].get<double>() == kWindowBackMs);
    CHECK(c["formula_linear"].get<double>() == kFormulaLinear);
    CHECK(c["formula_square"].get<double>() == kFormulaSquare);
    CHECK(c["formula_scale"].get<double>() == kFormulaScale);
    CHECK(c["formula_offset_ms"].get<double>() == kFormulaOffsetMs);
    CHECK(c.size() == 6);
}

TEST_CASE("schedule: the constants hold no frame, judge delay, lead, lag, gap or margin") {
    // The schedule keeps no millisecond value of its own: the frame length
    // and the judge delay are the player's, from the frame step it measures,
    // and every press time is worked out from a chord's time, its window or
    // a Star Power end. Fails if any of the seven keys comes back.
    const Song song = plain_song({0, 768});
    const ReplayResult r = replay_path(song, {});
    const json c = schedule::schedule_json(song, {}, r, core::default_rules())["constants"];
    for (const char* key : {"frame_ms", "judge_early_ms", "sp_out_lag_ms", "room_margin_ms", "press_lead_ms",
                            "sp_end_lead_ms", "chord_order_gap_ms", "judge_late_ms"}) {
        INFO(key);
        CHECK_FALSE(c.contains(key));
    }
}

TEST_CASE("schedule: a plain press is judged at its chord's own time, banded by its window") {
    // A plain press's judge target is its chord's time (aim 0), and its band
    // is the chord's whole window: from first_judge_ms to last_judge_ms, both
    // edges inside. No row holds a send time.
    const Song song = plain_song({0, 768, 960});
    const ReplayResult r = replay_path(song, {});
    const json s = schedule::schedule_json(song, {}, r, core::default_rules());
    REQUIRE(s["presses"].size() == song.sequence.size());
    for (const json& p : s["presses"]) {
        const json& row = s["chords"][p["chord"].get<size_t>()];
        const double ms = row["ms"].get<double>();
        const double side = row["side_ms"].get<double>();
        CHECK(p["rule"].get<std::string>() == "plain");
        CHECK(p["aim_ms"].get<double>() == 0.0);
        CHECK(p["band"]["from_ms"].get<double>() == doctest::Approx(ms - side));
        CHECK(p["band"]["to_ms"].get<double>() == doctest::Approx(ms + side));
        CHECK_FALSE(p["band"]["from_strict"].get<bool>());
        CHECK(p["window_from_ms"].get<double>() == doctest::Approx(ms - side));
        CHECK(p["window_to_ms"].get<double>() == doctest::Approx(ms + side));
        CHECK(p["room_ms"].get<double>() == doctest::Approx(side));
        CHECK_FALSE(p.contains("last_send_ms"));
        CHECK_FALSE(p.contains("after_ms"));
    }
}

TEST_CASE("schedule: sides come from the whole chart") {
    // Each chord's side is core::chord_sides_ms over the whole chord list the
    // file writes, never a pruned one. The floor is the side between two
    // neighbours whose half-gaps both sit on the window's front edge, the cap
    // the side between two on its back edge.
    const double floor_side = window_side_ms(2 * kWindowFrontMs, 2 * kWindowFrontMs);
    const double cap_side = window_side_ms(2 * kWindowBackMs, 2 * kWindowBackMs);
    const auto middle_side = [](const Song& song) {
        const ReplayResult r = replay_path(song, {});
        const json s = schedule::schedule_json(song, {}, r, core::default_rules());
        REQUIRE(s["chords"].size() == 3);
        std::vector<double> ms;
        for (const json& row : s["chords"]) ms.push_back(row["ms"].get<double>());
        const double side = s["chords"][1]["side_ms"].get<double>();
        CHECK(side == doctest::Approx(chord_sides_ms(ms)[1]));
        return side;
    };
    // Chords 20 ticks (52.08 ms) apart: both half-gaps clamp to the front.
    CHECK(middle_side(plain_song({0, 20, 40})) == doctest::Approx(floor_side));
    // Chords 917 ticks (2,388 ms) and 2,999 ticks (7,810 ms) either side:
    // both half-gaps clamp to the back.
    CHECK(middle_side(plain_song({0, 917, 917 + 2999})) == doctest::Approx(cap_side));
    // A chord's own gems are not each other's neighbours (TM-51): a
    // two-gem chord between far neighbours still gets the cap.
    Song two_gems = plain_song({0, 917, 917 + 2999});
    two_gems.sequence[1].chord.add_note(NoteColor::Kick);
    CHECK(middle_side(two_gems) == doctest::Approx(cap_side));
}

TEST_CASE("schedule: a missing --path refuses by name") {
    // hydra_replay schedule reads its path from the file its --path flag
    // names (cmd_schedule, tools/replay.cpp): no file named is refused with
    // one line naming that flag, and a real dump file named is not.
    const std::optional<std::string> r = schedule::path_flag_refusal("");
    REQUIRE(r.has_value());
    CHECK(r->find("--path") != std::string::npos);
    CHECK(r->find('\n') == std::string::npos);  // main prints it as one "error:" line
    CHECK_FALSE(schedule::path_flag_refusal(corpus::testdata_root() + "/replay/window_map-dump.json").has_value());
}

TEST_CASE("schedule: a chart with no path refuses by name") {
    // A real dump (hydra_replay dump of the Window Map chart) holds a
    // path, so it plans. The same dump with its paths taken out is refused
    // in one line that names the file it was read from.
    const std::string file = corpus::testdata_root() + "/replay/window_map-dump.json";
    json doc = corpus::load_json(file);
    REQUIRE(!doc["paths"].empty());
    CHECK_FALSE(schedule::no_path_refusal(doc, file).has_value());
    doc["paths"] = json::array();
    const std::optional<std::string> r = schedule::no_path_refusal(doc, file);
    REQUIRE(r.has_value());
    CHECK(r->find(file) != std::string::npos);
    CHECK(r->find('\n') == std::string::npos);
    doc.erase("paths");
    CHECK(schedule::no_path_refusal(doc, file).has_value());
}

TEST_CASE("schedule: nothing under src/ reads the schedule") {
    // No file under src/ includes the tool's header or names its constants,
    // so no Hydra screen reads the player's numbers.
    bool saw_model_h = false;
    std::string first_hit;
    sourcetree::for_each_source_file([&](const fs::path& path, const std::string& rel) {
        if (rel.rfind("src/", 0) != 0) return;
        if (rel == "src/core/model.h") saw_model_h = true;
        std::ifstream in(path, std::ios::binary);
        const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        for (const char* name : {"replay_schedule.h", "far_press_refusal", "band_json"})
            if (text.find(name) != std::string::npos && first_hit.empty())
                first_hit = rel + " names " + name;
    });
    // The walk reached src/core, where the screens' own window constants live;
    // a walk that never got there would find no hit and prove nothing.
    CHECK(saw_model_h);
    CHECK_MESSAGE(first_hit.empty(), first_hit);
}

// ---- TC2: the aims, rooms, fallbacks and refusals ---------------------------

namespace {

// A squeeze-out's geometry from the plan's table (design point 6): the two
// sides, the gap from the squeeze chord to the end, and the early ratio. The
// late ratio is one no squeeze-out reads, so reading the wrong side shows.
schedule::SqueezeGeometry sqout(double anchor_side, double squeeze_ms, double squeeze_side, double end_ms,
                                double early_ratio) {
    schedule::SqueezeGeometry g;
    g.anchor_press = Press::Early;  // a timed squeeze-out's anchor
    g.anchor_chord = 1;
    g.anchor_side_ms = anchor_side;
    g.squeeze_chord = 2;
    g.squeeze_ms = squeeze_ms;
    g.squeeze_side_ms = squeeze_side;
    g.end_ms = end_ms;
    g.ratio = TransferScale{early_ratio, 2.0};
    return g;
}

// The presses of chord `chord` the schedule `s` writes under `rule`.
std::vector<json> presses_of(const json& s, int chord, const std::string& rule) {
    std::vector<json> out;
    for (const json& p : s["presses"])
        if (p["chord"].get<int>() == chord && p["rule"] == rule) out.push_back(p);
    return out;
}

// The places of chord row `row`'s gems whose colour is not `chord`'s
// activation note (Chord::activation_note): the gems a skipped fill keeps,
// read by colour from the row the schedule writes.
std::vector<size_t> places_but_activation_note(const json& row, const Chord& chord) {
    const std::string top = color_str(chord.activation_note().colortype);
    std::vector<size_t> out;
    for (size_t k = 0; k < row["gems"].size(); ++k)
        if (row["gems"][k]["color"].get<std::string>() != top) out.push_back(k);
    return out;
}

// The equal-room split's rule on one squeeze's two presses `s`, worked from
// geometry `g`: the anchor goes the way its press says by the equal-room
// lead, its room is press_room_ms of its aim, the squeeze press's room is
// press_room_ms at the end it must clear, the two rooms are equal, the
// squeeze press's band runs from that end (a squeeze-out's late gem, strict,
// up to the window's back edge) or to it (a squeeze-in, from the window's
// front edge), its aim is the band's middle, and that end moves the way the
// anchor goes.
void check_equal_room_split(const schedule::SqueezeGeometry& g, const schedule::SqueezeSplit& s) {
    const bool early = g.anchor_press == Press::Early;
    const double lead = schedule::equal_room_lead_ms(g);
    CHECK(s.anchor_aim_ms == doctest::Approx(early ? -lead : lead));
    CHECK(s.anchor_room_ms == doctest::Approx(schedule::press_room_ms(s.anchor_aim_ms, g.anchor_side_ms)));
    CHECK(s.squeeze_room_ms ==
          doctest::Approx(schedule::press_room_ms(s.aim_end_ms - g.squeeze_ms, g.squeeze_side_ms)));
    CHECK(s.anchor_room_ms == doctest::Approx(s.squeeze_room_ms));
    CHECK(s.aim_end_ms == s.planned_end_ms);
    if (early) {
        CHECK(s.squeeze_band.from_ms == s.aim_end_ms);
        CHECK(s.squeeze_band.from_strict);
        CHECK(s.squeeze_band.to_ms == schedule::last_judge_ms(g.squeeze_ms, g.squeeze_side_ms));
        CHECK(s.planned_end_ms < g.end_ms);
    } else {
        CHECK(s.squeeze_band.from_ms == schedule::first_judge_ms(g.squeeze_ms, g.squeeze_side_ms));
        CHECK_FALSE(s.squeeze_band.from_strict);
        CHECK(s.squeeze_band.to_ms == s.aim_end_ms);
        CHECK(s.planned_end_ms > g.end_ms);
    }
    CHECK(g.squeeze_ms + s.squeeze_aim_ms == doctest::Approx(schedule::band_middle_ms(s.squeeze_band)));
    CHECK(g.squeeze_ms + s.squeeze_aim_ms - s.squeeze_band.from_ms ==
          doctest::Approx(s.squeeze_band.to_ms - (g.squeeze_ms + s.squeeze_aim_ms)));
}

// One chord alone on the kick pad at `ms` with side `side_ms`, for the
// far-press check: with no neighbour, a press of it is refused only when it
// leaves its own window.
std::vector<schedule::PadChord> lone_kick_chord(double ms, double side_ms) {
    std::vector<schedule::PadChord> one(1);
    one[0].index = 0;
    one[0].ms = ms;
    one[0].side_ms = side_ms;
    one[0].chart_pads = {schedule::Pad{NoteColor::Kick, false}};
    return one;
}

// Whether a press of the lone chord `one` judged at `judge_ms` stays in its
// window (far_press_refusal gives no line).
bool lone_press_inside(const std::vector<schedule::PadChord>& one, double judge_ms) {
    return !schedule::far_press_refusal(one, 0, judge_ms, judge_ms, one[0].chart_pads).has_value();
}

// Freak's overfill shape on a hand-built chart (120 BPM, 2.604 ms a tick):
// Red chords at ticks 0, 768, 1536, 2304, 3072 and 3840; the activation on
// chord 1, the cap clamping on chord 2's phrase, and chord 4 (Red and Kick)
// squeezed out of the final end at tick 3082, 26.04 ms after it. Every chord
// is 2,000 ms from its neighbours, so every side is the cap.
struct Overfill {
    Song song;
    Activation act;
};
Overfill overfill() {
    Overfill o{plain_song({0, 768, 1536, 2304, 3072, 3840}), Activation{}};
    o.song.sequence[4].chord.add_note(NoteColor::Kick);
    o.act.timecode = o.song.timecode(768);
    o.act.bank_rise_ticks = {0, 768};  // two bars, so the ready point is known
    o.act.sp_end_steps = {{768, 3000, SpEndKind::Activation}, {1536, 3082, SpEndKind::Clamped}};
    SPSqueeze out{};
    out.kind = SqueezeKind::SqOut;
    out.offset_ms = -26.04;
    o.act.sqinouts.push_back(out);
    o.act.sqout_tick = 3072;
    o.act.transfer_post = TransferScale{1.0, 1.0};
    return o;
}

}  // namespace

TEST_CASE("schedule: the equal-room split on Freak's clamped squeeze") {
    // A squeeze-out whose anchor side is narrower than the squeeze chord's
    // side: the split keeps the rule (check_equal_room_split), the anchor
    // goes early, and both presses keep room inside their windows.
    const schedule::SqueezeGeometry g = sqout(48.15, 60545.26, 53.59, 60573.35, 1.0);
    const schedule::SqueezeSplit s = schedule::split_squeeze(g);
    check_equal_room_split(g, s);
    CHECK(s.anchor_aim_ms < 0.0);
    CHECK(s.squeeze_room_ms > 0.0);
    // The lead reads the anchor's side: a wider anchor side takes a longer
    // lead, so the anchor keeps the extra room and gives part of it over.
    const schedule::SqueezeSplit wider = schedule::split_squeeze(sqout(85.72, 60545.26, 53.59, 60573.35, 1.0));
    CHECK(wider.anchor_aim_ms < s.anchor_aim_ms);

    // The whole file on the overfill shape: the lead goes on chord 2, the
    // chord the clamp anchors the end on, never on the activation's chord 1.
    const Overfill o = overfill();
    const ReplayResult r = replay_path(o.song, {});
    const json sch = schedule::schedule_json(o.song, {&o.act}, r, core::default_rules());
    INFO(sch["refusals"].dump());
    CHECK(sch["refusals"].empty());  // and no "not planned yet" line
    const std::vector<json> anchor = presses_of(sch, 2, "anchor");
    REQUIRE(anchor.size() == 1);
    CHECK(anchor[0]["aim_ms"].get<double>() < 0.0);
    CHECK(presses_of(sch, 1, "anchor").empty());
    REQUIRE(presses_of(sch, 1, "plain").size() == 1);
    CHECK(presses_of(sch, 1, "plain")[0]["aim_ms"].get<double>() == 0.0);
    // The read after chord 2 sees the clamped end (the step on chord 2, whose
    // end tick is 3082), planned where the squeeze's split plans it.
    const json& q = sch["activations"][0]["squeezes"][0];
    const json& reads = sch["chords"][2]["reads"];
    REQUIRE(reads.size() == 1);
    CHECK(reads[0]["stored_end_ms"].get<double>() == doctest::Approx(o.song.timecode(o.act.sp_end_steps.back().end_tick).ms()));
    CHECK(reads[0]["planned_end_ms"].get<double>() == doctest::Approx(q["planned_end_ms"].get<double>()));
    CHECK(reads[0]["planned_end_ms"].get<double>() < reads[0]["stored_end_ms"].get<double>());
    // The squeeze-out chord: the late gem (place 0) is banded after the end
    // the read plans, retimed from the read after chord 2.
    const std::vector<json> late = presses_of(sch, 4, "squeeze_out_late");
    REQUIRE(late.size() == 1);
    CHECK(late[0]["gems"].get<std::vector<size_t>>() == std::vector<size_t>{0});
    CHECK(late[0]["aim_end_ms"].get<double>() == doctest::Approx(reads[0]["planned_end_ms"].get<double>()));
    CHECK(late[0]["band"]["from_ms"].get<double>() == doctest::Approx(late[0]["aim_end_ms"].get<double>()));
    CHECK(late[0]["end_read_after"].get<int>() == 2);
    CHECK(late[0]["timed_from_end"].get<size_t>() == 0);
    CHECK(q["anchor_room_ms"].get<double>() == doctest::Approx(q["squeeze_room_ms"].get<double>()));
    // Feasible is Hydra's safe sum for this squeeze.
    const std::optional<WindowEnds> ends = squeeze_window_ends(o.act, 0, chord_times(o.song));
    REQUIRE(ends.has_value());
    CHECK(q["safe_ms"].get<double>() == ends->safe_ms);
    CHECK(q["feasible"].get<bool>() == !ends->is_short());
}

TEST_CASE("schedule: feasible is Hydra's safe sum, short below zero") {
    // Two squeeze-outs on the overfill shape, one whose safe sum is small and
    // positive and one whose sum is below zero. The test moves the squeezed
    // chord against its end until squeeze_window_ends reads each, and checks
    // the file's "feasible" is !is_short() for that same activation; it works
    // out no sum of its own.
    bool saw_safe = false;
    bool saw_short = false;
    // The walk starts one tick before the end (tick 3082), where the sum is
    // widest, and moves the chord away from the end, so the sum falls through
    // every value down past zero. One tick is 2.604 ms, finer than the 5 ms
    // band the small sum is looked for in, so the walk cannot step over it.
    const int64_t end_tick = 3082;
    for (int64_t chord_tick = end_tick - 1; chord_tick >= 2310; --chord_tick) {
        Overfill o{plain_song({0, 768, 1536, 2304, chord_tick, 3840}), Activation{}};
        o.song.sequence[4].chord.add_note(NoteColor::Kick);
        o.act.timecode = o.song.timecode(768);
        o.act.bank_rise_ticks = {0, 768};
        o.act.sp_end_steps = {{768, 3000, SpEndKind::Activation}, {1536, end_tick, SpEndKind::Clamped}};
        SPSqueeze out{};
        out.kind = SqueezeKind::SqOut;
        out.offset_ms = offset_from_sp_end(o.song.timecode(chord_tick).ms(), o.song.timecode(end_tick).ms());
        o.act.sqinouts.push_back(out);
        o.act.sqout_tick = chord_tick;
        o.act.transfer_post = TransferScale{1.0, 1.0};
        const std::optional<WindowEnds> ends = squeeze_window_ends(o.act, 0, chord_times(o.song));
        if (!ends) continue;
        const bool small_safe = !ends->is_short() && ends->safe_ms < 5.0;
        if (!(small_safe && !saw_safe) && !(ends->is_short() && !saw_short)) continue;
        const json sch = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}), core::default_rules());
        const json& q = sch["activations"][0]["squeezes"][0];
        if (!q.contains("anchor_room_ms")) continue;  // the split was refused for another reason
        INFO("chord tick " << chord_tick << " safe " << ends->safe_ms);
        CHECK(q["feasible"].get<bool>() == !ends->is_short());
        CHECK(q["safe_ms"].get<double>() == ends->safe_ms);
        if (small_safe) saw_safe = true;
        else saw_short = true;
        if (saw_safe && saw_short) break;
    }
    CHECK(saw_safe);
    CHECK(saw_short);
}

TEST_CASE("schedule: the equal-room split holds on far squeeze-outs with uneven sides and ratios") {
    // Four squeeze-outs 84 to 110 ms inside their end, each with a wider
    // anchor side than squeeze side and an early ratio at or above 1: the
    // split keeps its rule on each (check_equal_room_split), and the rooms
    // it leaves are inside both windows.
    struct Shape {
        double anchor_side, squeeze_side, gap, ratio;
    };
    const Shape shapes[] = {
        {73.76, 53.59, 84.27, 1.0},
        {75.93, 61.47, 99.04, 1.0284},
        {76.49, 61.94, 100.00, 1.0045},
        {78.75, 66.34, 109.25, 1.0133},
    };
    for (const Shape& shape : shapes) {
        INFO("gap " << shape.gap << " ratio " << shape.ratio);
        const double end = 100000.0;
        const schedule::SqueezeGeometry g =
            sqout(shape.anchor_side, end - shape.gap, shape.squeeze_side, end, shape.ratio);
        const schedule::SqueezeSplit s = schedule::split_squeeze(g);
        check_equal_room_split(g, s);
        CHECK(s.anchor_aim_ms < 0.0);
        CHECK(s.anchor_room_ms > 0.0);
    }
}

TEST_CASE("schedule: a squeeze that needs no timing is pressed plain") {
    // A squeeze-in whose chord (tick 3072) already sits inside the end it
    // squeezes in against (tick 3104, 32 ticks later): its offset is the
    // chord's distance from that end, so it needs no timing.
    const Song song = plain_song({0, 768, 1536, 2304, 3072, 3840});
    Activation act;
    act.timecode = song.timecode(768);
    act.sp_end_steps = {{768, 3104, SpEndKind::Activation}, {3072, 3900, SpEndKind::SqIn}};
    SPSqueeze in{};
    in.kind = SqueezeKind::SqIn;
    in.offset_ms = offset_from_sp_end(song.timecode(3072).ms(), song.timecode(3104).ms());
    REQUIRE(in.needs_no_timing());
    in.transfer = TransferScale{1.0, 1.0};
    act.sqinouts.push_back(in);
    act.transfer_post = TransferScale{1.0, 1.0};
    const ReplayResult r = replay_path(song, {});
    const json s = schedule::schedule_json(song, {&act}, r, core::default_rules());
    const json& q = s["activations"][0]["squeezes"][0];
    CHECK(q["needs_no_timing"].get<bool>());
    CHECK(q["feasible"].get<bool>());
    const std::vector<json> p = presses_of(s, 4, "needs_no_timing");
    REQUIRE(p.size() == 1);
    // Pressed as a plain chord is: judged at its own time.
    CHECK(p[0]["aim_ms"].get<double>() == 0.0);
    CHECK_FALSE(p[0].contains("fallback"));
    CHECK(presses_of(s, 1, "plain").size() == 1);  // no lead on the activation
}

TEST_CASE("schedule: a squeeze the game cannot make is not feasible") {
    // A 200 ms gap against two 53.59 ms sides: both rooms negative, and that
    // is a feasibility, not a refusal. Whether the file calls it feasible is
    // the safe-sum test's ("feasible is Hydra's safe sum").
    const schedule::SqueezeGeometry too_wide = sqout(53.59, 1000.0, 53.59, 1200.0, 1.0);
    const schedule::SqueezeSplit s = schedule::split_squeeze(too_wide);
    CHECK(s.anchor_room_ms < 0.0);
    CHECK(s.squeeze_room_ms < 0.0);
    std::vector<std::string> refusals;
    CHECK(schedule::anchor_lead_ms({too_wide}, refusals).has_value());
    CHECK(refusals.empty());
    // With equal sides the lead does not read the sides, so both rooms move
    // one for one with them, and no margin of the schedule's own sits between.
    const schedule::SqueezeSplit base = schedule::split_squeeze(sqout(40.0, 1000.0, 40.0, 1025.0, 1.0));
    const schedule::SqueezeSplit wider = schedule::split_squeeze(sqout(41.0, 1000.0, 41.0, 1025.0, 1.0));
    CHECK(wider.anchor_room_ms == doctest::Approx(base.anchor_room_ms + 1.0));
    CHECK(wider.squeeze_room_ms == doctest::Approx(base.squeeze_room_ms + 1.0));
}

TEST_CASE("schedule: a split that would press the anchor the other way is refused") {
    // A lead below zero would judge the anchor past its own time the other
    // way. A squeeze-out whose squeeze side outreaches the anchor side by
    // more than the gap gives a lead below zero.
    const schedule::SqueezeGeometry g = sqout(20.0, 1000.0, 60.0, 1010.0, 1.0);
    REQUIRE(schedule::equal_room_lead_ms(g) < 0.0);
    std::vector<std::string> refusals;
    CHECK_FALSE(schedule::anchor_lead_ms({g}, refusals).has_value());
    REQUIRE(refusals.size() == 1);
    CHECK(refusals[0].find("the other way") != std::string::npos);
}

TEST_CASE("schedule: the fallback block") {
    // Each threshold is an edge of the chord's window, where the press's band
    // runs out. One chord at 1,000 ms with a 50 ms side, alone on its pad
    // (whole numbers, so each edge is exact in a double).
    const std::vector<schedule::PadChord> one = lone_kick_chord(1000.0, 50.0);
    // A squeeze-out's late gem is judged after its end. At the threshold the
    // end sits on the window's back edge, the last judge time still inside.
    const double after = schedule::sqout_fallback_after_ms(one[0].ms, one[0].side_ms);
    CHECK(after == schedule::last_judge_ms(one[0].ms, one[0].side_ms));
    CHECK(lone_press_inside(one, after));
    CHECK_FALSE(lone_press_inside(one, after + 1.0));
    // A squeeze-in is judged by its end. At the threshold the end sits on the
    // window's front edge, the first judge time inside.
    const double before = schedule::sqin_fallback_before_ms(one[0].ms, one[0].side_ms);
    CHECK(before == schedule::first_judge_ms(one[0].ms, one[0].side_ms));
    CHECK(lone_press_inside(one, before));
    CHECK_FALSE(lone_press_inside(one, before - 1.0));

    // The overfill shape's two-gem squeeze-out: the late press's block names
    // the last inside group's gems.
    {
        const Overfill o = overfill();
        const ReplayResult r = replay_path(o.song, {});
        const json s = schedule::schedule_json(o.song, {&o.act}, r, core::default_rules());
        const std::vector<json> late = presses_of(s, 4, "squeeze_out_late");
        const std::vector<json> inside = presses_of(s, 4, "squeeze_out_inside");
        REQUIRE(late.size() == 1);
        REQUIRE(inside.size() == 1);
        const json& fb = late[0]["fallback"];
        CHECK(fb["action"] == "join_group");
        CHECK(fb["retimed"] == "inside");
        CHECK(fb["join_gems"] == inside[0]["gems"]);
        CHECK_FALSE(fb.contains("before_end_ms"));
        const json& row = s["chords"][4];
        CHECK(fb["if_end_after_ms"].get<double>() ==
              doctest::Approx(schedule::sqout_fallback_after_ms(row["ms"].get<double>(), row["side_ms"].get<double>())));
        // A plain press carries no block.
        REQUIRE(presses_of(s, 0, "plain").size() == 1);
        CHECK_FALSE(presses_of(s, 0, "plain")[0].contains("fallback"));
    }

    // A hand-built timed squeeze-in: chord 4 at tick 3010 sits 10 ticks past
    // the activation's end at tick 3000. Its fallback, if the end comes in
    // before the threshold, is a plain press.
    {
        const Song song = plain_song({0, 768, 1536, 2304, 3010, 3840});
        Activation act;
        act.timecode = song.timecode(768);
        act.sp_end_steps = {{768, 3000, SpEndKind::Activation}, {3010, 3600, SpEndKind::SqIn}};
        SPSqueeze in{};
        in.kind = SqueezeKind::SqIn;
        in.offset_ms = offset_from_sp_end(song.timecode(3010).ms(), song.timecode(3000).ms());
        in.transfer = TransferScale{1.0, 1.0};
        act.sqinouts.push_back(in);
        act.transfer_post = TransferScale{1.0, 1.0};
        const ReplayResult r = replay_path(song, {});
        const json s = schedule::schedule_json(song, {&act}, r, core::default_rules());
        const std::vector<json> p = presses_of(s, 4, "squeeze_in");
        REQUIRE(p.size() == 1);
        const json& fb = p[0]["fallback"];
        CHECK(fb["action"] == "plain");
        CHECK(fb["retimed"] == "plain");
        CHECK(fb["aim_ms"].get<double>() == 0.0);
        const json& row = s["chords"][4];
        CHECK(fb["if_end_before_ms"].get<double>() ==
              doctest::Approx(schedule::sqin_fallback_before_ms(row["ms"].get<double>(), row["side_ms"].get<double>())));
        // Its band runs from its window's front edge to its own end, and it
        // aims at the middle.
        const json& band = p[0]["band"];
        CHECK(band["from_ms"].get<double>() == doctest::Approx(p[0]["window_from_ms"].get<double>()));
        CHECK(band["to_ms"].get<double>() == doctest::Approx(p[0]["own_end_ms"].get<double>()));
        CHECK(row["ms"].get<double>() + p[0]["aim_ms"].get<double>() ==
              doctest::Approx((band["from_ms"].get<double>() + band["to_ms"].get<double>()) / 2.0));
        // The activation leads it late.
        REQUIRE(presses_of(s, 1, "anchor").size() == 1);
        CHECK(presses_of(s, 1, "anchor")[0]["aim_ms"].get<double>() > 0.0);
    }
}

TEST_CASE("schedule: every press row carries its chord's window edges") {
    // The last judge time is the window's back edge, the latest a press may
    // be judged and still hit: judged then it stays in, 1 ms later it leaves.
    // The first is the front edge, the same side the other way.
    const std::vector<schedule::PadChord> one = lone_kick_chord(1000.0, 50.0);
    const double last = schedule::last_judge_ms(one[0].ms, one[0].side_ms);
    CHECK(lone_press_inside(one, last));
    CHECK_FALSE(lone_press_inside(one, last + 1.0));
    const double first = schedule::first_judge_ms(one[0].ms, one[0].side_ms);
    CHECK(lone_press_inside(one, first));
    CHECK_FALSE(lone_press_inside(one, first - 1.0));
    CHECK(one[0].ms - first == doctest::Approx(last - one[0].ms));
    // Every press in the file carries its chord's two edges and a band inside
    // them, and every chord of the overfill shape is pressed.
    const Overfill o = overfill();
    const ReplayResult r = replay_path(o.song, {});
    const json s = schedule::schedule_json(o.song, {&o.act}, r, core::default_rules());
    std::set<size_t> pressed;
    for (const json& p : s["presses"]) pressed.insert(p["chord"].get<size_t>());
    CHECK(pressed.size() == o.song.sequence.size());
    for (const json& p : s["presses"]) {
        const json& row = s["chords"][p["chord"].get<size_t>()];
        const double ms = row["ms"].get<double>();
        const double side = row["side_ms"].get<double>();
        CHECK(p["window_from_ms"].get<double>() == doctest::Approx(schedule::first_judge_ms(ms, side)));
        CHECK(p["window_to_ms"].get<double>() == doctest::Approx(schedule::last_judge_ms(ms, side)));
        CHECK(p["band"]["from_ms"].get<double>() >= p["window_from_ms"].get<double>() - 1e-9);
        CHECK(p["band"]["to_ms"].get<double>() <= p["window_to_ms"].get<double>() + 1e-9);
        CHECK_FALSE(p.contains("last_send_ms"));
    }
}

TEST_CASE("schedule: a squeeze-in's threshold moves with its window and nothing else") {
    // The threshold holds no fixed lead and no frame: moving the chord moves
    // it by the same amount, widening the side moves it earlier by the same
    // amount, and it is the window's front edge itself.
    const double base = schedule::sqin_fallback_before_ms(1000.0, 50.0);
    CHECK(schedule::sqin_fallback_before_ms(1037.5, 50.0) == doctest::Approx(base + 37.5));
    CHECK(schedule::sqin_fallback_before_ms(1000.0, 61.25) == doctest::Approx(base - 11.25));
    CHECK(schedule::sqin_fallback_before_ms(1037.5, 61.25) == doctest::Approx(base + 37.5 - 11.25));
    CHECK(base == schedule::first_judge_ms(1000.0, 50.0));
    // The squeeze-out's threshold the same way, at the back edge.
    const double out = schedule::sqout_fallback_after_ms(1000.0, 50.0);
    CHECK(schedule::sqout_fallback_after_ms(1037.5, 61.25) == doctest::Approx(out + 37.5 + 11.25));
    CHECK(out == schedule::last_judge_ms(1000.0, 50.0));
}

TEST_CASE("schedule: the far-press check covers a press's whole band") {
    // Two Snare chords: chord 0 at 1,000 ms with a 50 ms side, and chord 1 at
    // 1,080 ms with a 50 ms side. Chord 0 gets no press on the pad, so its
    // note stays unhit. A press of chord 1 aimed at 1,080 ms is clear of chord
    // 0's window (which ends at 1,050 ms), but a band reaching back to
    // 1,040 ms has its far edge inside it, so it is refused. The test fails
    // if the check reads only the target.
    const schedule::Pad snare{NoteColor::Red, false};
    std::vector<schedule::PadChord> two(2);
    two[0].index = 0;
    two[0].ms = 1000.0;
    two[0].side_ms = 50.0;
    two[0].chart_pads = {snare};
    two[1].index = 1;
    two[1].ms = 1080.0;
    two[1].side_ms = 50.0;
    two[1].chart_pads = {snare};
    CHECK_FALSE(schedule::far_press_refusal(two, 1, 1080.0, 1080.0, {snare}).has_value());
    const std::optional<std::string> r = schedule::far_press_refusal(two, 1, 1040.0, 1080.0, {snare});
    REQUIRE(r.has_value());
    CHECK(r->find("chord 0") != std::string::npos);
    // Once chord 0's own press on the pad is surely judged before the band
    // starts, the band is clear.
    two[0].sends = {schedule::PadSend{snare, 1000.0, 1000.0}};
    CHECK_FALSE(schedule::far_press_refusal(two, 1, 1040.0, 1080.0, {snare}).has_value());
    // A press of chord 0 that may still be judged as late as the band's
    // start leaves the earlier note unhit then.
    two[0].sends = {schedule::PadSend{snare, 990.0, 1040.0}};
    CHECK(schedule::far_press_refusal(two, 1, 1040.0, 1080.0, {snare}).has_value());
}

TEST_CASE("schedule: an activation that needs both directions is refused naming both chords") {
    schedule::SqueezeGeometry out = sqout(60.0, 1000.0, 60.0, 1030.0, 1.0);
    out.anchor_chord = 5;
    out.squeeze_chord = 17;
    schedule::SqueezeGeometry in = out;
    in.anchor_press = Press::Late;  // a timed squeeze-in's anchor
    in.squeeze_chord = 23;
    in.squeeze_ms = 1040.0;
    std::vector<std::string> refusals;
    CHECK_FALSE(schedule::anchor_lead_ms({out, in}, refusals).has_value());
    REQUIRE(refusals.size() == 1);
    // The line names the anchor, the chord it would go early for and the one
    // it would go late for.
    CHECK(refusals[0].find("anchor chord " + std::to_string(out.anchor_chord)) != std::string::npos);
    CHECK(refusals[0].find("early for chord " + std::to_string(out.squeeze_chord)) != std::string::npos);
    CHECK(refusals[0].find("late for chord " + std::to_string(in.squeeze_chord)) != std::string::npos);
}

TEST_CASE("schedule: a lead across a steady span is refused naming the span") {
    // A squeeze-out whose anchor leads early by the equal-room split: a tempo
    // change nearer the anchor than that lead breaks the ratio, one farther
    // away does not.
    schedule::SqueezeGeometry g = sqout(48.15, 60545.26, 53.59, 60573.35, 1.0);
    const double lead = schedule::equal_room_lead_ms(g);
    REQUIRE(lead > 0.0);
    g.steady.before_anchor_ms = lead / 2;
    std::vector<std::string> refusals;
    CHECK_FALSE(schedule::anchor_lead_ms({g}, refusals).has_value());
    REQUIRE(refusals.size() == 1);
    // The line names the span that was crossed and how long it is.
    CHECK(refusals[0].find("before_anchor_ms") != std::string::npos);
    CHECK(refusals[0].find(std::to_string(*g.steady.before_anchor_ms)) != std::string::npos);
    g.steady.before_anchor_ms = lead * 2;
    refusals.clear();
    CHECK(schedule::anchor_lead_ms({g}, refusals).has_value());
    CHECK(refusals.empty());
}

TEST_CASE("schedule: a press that reaches a neighbour on its pad is refused naming both chords") {
    // Two kick chords 100 ms apart, each with the side core::chord_sides_ms
    // gives it.
    const std::vector<double> sides = chord_sides_ms({1000.0, 1100.0});
    std::vector<schedule::PadChord> pair(2);
    const schedule::Pad kick_pad{NoteColor::Kick, false};
    for (size_t i = 0; i < 2; ++i) {
        pair[i].index = 10 + static_cast<int>(i);
        pair[i].ms = 1000.0 + 100.0 * static_cast<double>(i);
        pair[i].side_ms = sides[i];
        pair[i].chart_pads = {kick_pad};
    }
    const std::vector<schedule::Pad> kick = {kick_pad};
    // The first chord judged at its own time: the second judged 60 ms early
    // lands inside the first's window after the first is hit, so it passes.
    pair[0].sends = {{kick_pad, pair[0].ms, pair[0].ms}};
    const double early_judge = pair[1].ms - 60.0;
    CHECK_FALSE(schedule::far_press_refusal(pair, 1, early_judge, early_judge, kick).has_value());
    // The first chord not pressed: the second's press would take it. Refused,
    // naming both chords.
    pair[0].sends.clear();
    const std::optional<std::string> r = schedule::far_press_refusal(pair, 1, early_judge, early_judge, kick);
    REQUIRE(r.has_value());
    CHECK(r->find("chord " + std::to_string(pair[1].index)) != std::string::npos);
    CHECK(r->find("chord " + std::to_string(pair[0].index)) != std::string::npos);
}

TEST_CASE("schedule: a late gem that pays less than an inside gem is refused") {
    // Place 0 is late at x1, place 1 inside at x2: no press order pays that.
    const int chord = 7;
    const std::vector<int> mults = {1, 2};
    std::vector<std::string> refusals;
    CHECK_FALSE(schedule::squeeze_out_inside_groups(chord, mults, {0, 1}, {0}, refusals).has_value());
    REQUIRE(refusals.size() == 1);
    // The line names the chord and both multipliers.
    CHECK(refusals[0].find("chord " + std::to_string(chord)) != std::string::npos);
    CHECK(refusals[0].find("x" + std::to_string(mults[0])) != std::string::npos);
    CHECK(refusals[0].find("x" + std::to_string(mults[1])) != std::string::npos);
    refusals.clear();
    const auto ok = schedule::squeeze_out_inside_groups(chord, {2, 2, 1}, {0, 1, 2}, {0}, refusals);
    REQUIRE(ok.has_value());
    CHECK(refusals.empty());
    CHECK(*ok == std::vector<std::vector<size_t>>{{2}, {1}});
    // Only the gems the player sends go inside: a declined chord that keeps
    // places 0 and 2 presses place 2 alone inside.
    const auto kept = schedule::squeeze_out_inside_groups(chord, {2, 2, 1}, {0, 2}, {0}, refusals);
    REQUIRE(kept.has_value());
    CHECK(*kept == std::vector<std::vector<size_t>>{{2}});
}

TEST_CASE("schedule: press groups") {
    // Places: 0 the flagged green cymbal (x2), 1 an x1 gem, 2 an x2 gem.
    CHECK(schedule::press_groups({2, 1, 2}, {0, 1, 2}, size_t{0}) ==
          std::vector<std::vector<size_t>>{{0}, {1}, {2}});
    // No flagged gem and one multiplier: one group.
    CHECK(schedule::press_groups({1, 1}, {0, 1}, std::nullopt) == std::vector<std::vector<size_t>>{{0, 1}});
    // A declined chord that sends nothing is still one press.
    CHECK(schedule::press_groups({1, 1}, {}, std::nullopt) == std::vector<std::vector<size_t>>(1));

    // In the file: an activation chord of Red, Green and Kick at one
    // multiplier presses Green (its activation note) first, then the rest the
    // frame after. The later group's row names the group it follows and
    // carries no gap in ms: the same aim and band as its head.
    Overfill o = overfill();
    o.song.sequence[1].chord.add_note(NoteColor::Green);
    o.song.sequence[1].chord.add_note(NoteColor::Kick);
    const ReplayResult r = replay_path(o.song, {});
    const json s = schedule::schedule_json(o.song, {&o.act}, r, core::default_rules());
    const std::vector<json> act = presses_of(s, 1, "plain");
    REQUIRE(act.size() == 2);
    const json& gems = s["chords"][1]["gems"];
    CHECK(gems[act[0]["gems"][0].get<size_t>()]["color"] == "Green");
    CHECK(act[0]["group"].get<size_t>() == 0);
    CHECK(act[0]["follows"].is_null());
    CHECK(act[0]["aim_ms"].get<double>() == 0.0);
    CHECK(act[1]["group"].get<size_t>() == 1);
    CHECK(act[1]["follows"].get<size_t>() == 0);
    CHECK(act[1]["aim_ms"].get<double>() == act[0]["aim_ms"].get<double>());
    CHECK(act[1]["band"] == act[0]["band"]);
    CHECK_FALSE(act[1].contains("after_ms"));
    // The two-gem squeeze-out: the inside gem judged by the end the late
    // gem's band starts at, aimed at the middle of its end-band, the late gem
    // last.
    const std::vector<json> inside = presses_of(s, 4, "squeeze_out_inside");
    const std::vector<json> late = presses_of(s, 4, "squeeze_out_late");
    REQUIRE(inside.size() == 1);
    REQUIRE(late.size() == 1);
    CHECK(inside[0]["band"]["to_ms"].get<double>() == doctest::Approx(late[0]["aim_end_ms"].get<double>()));
    CHECK(inside[0]["band"]["from_ms"].get<double>() == doctest::Approx(inside[0]["window_from_ms"].get<double>()));
    CHECK(s["chords"][4]["ms"].get<double>() + inside[0]["aim_ms"].get<double>() ==
          doctest::Approx((inside[0]["band"]["from_ms"].get<double>() + inside[0]["band"]["to_ms"].get<double>()) /
                          2.0));
    size_t inside_at = 0, late_at = 0;
    for (size_t k = 0; k < s["presses"].size(); ++k) {
        const json& p = s["presses"][k];
        if (p["chord"].get<int>() != 4) continue;
        (p["rule"] == "squeeze_out_inside" ? inside_at : late_at) = k;
    }
    CHECK(inside_at < late_at);
}

TEST_CASE("schedule: a plain press plans the end as stored") {
    // A plain activation: the read after its chord is the step's stored end
    // (tick 3000). The press is judged at its chord's own time, so the end is
    // planned where it is stored, with no frame or lead in it.
    const Song song = plain_song({0, 768, 1536});
    Activation act;
    act.timecode = song.timecode(768);
    act.sp_end_steps = {{768, 3000, SpEndKind::Activation}};
    act.transfer_post = TransferScale{1.0, 1.0};
    const ReplayResult r = replay_path(song, {});
    const json s = schedule::schedule_json(song, {&act}, r, core::default_rules());
    const json& reads = s["chords"][1]["reads"];
    REQUIRE(reads.size() == 1);
    CHECK(reads[0]["stored_end_ms"].get<double>() == doctest::Approx(song.timecode(act.sp_end_steps.front().end_tick).ms()));
    CHECK(reads[0]["planned_end_ms"].get<double>() == reads[0]["stored_end_ms"].get<double>());
}

namespace {

// The hand-built squeeze-in the next test works by hand: an anchor (side 60)
// late for a chord (side 50) 40 ms past the end at 1,000 ms, ratio 1.0 on the
// late side. The read-end test further down reuses it.
schedule::SqueezeGeometry hand_built_squeeze_in() {
    schedule::SqueezeGeometry g;
    g.anchor_press = Press::Late;  // a timed squeeze-in's anchor
    g.anchor_chord = 1;
    g.anchor_side_ms = 60.0;
    g.squeeze_chord = 2;
    g.squeeze_ms = 1040.0;
    g.squeeze_side_ms = 50.0;
    g.end_ms = 1000.0;
    g.ratio = TransferScale{2.0, 1.0};
    return g;
}

}  // namespace

TEST_CASE("schedule: the equal-room split of a squeeze-in is the mirror of a squeeze-out's") {
    // The hand-built squeeze-in (an anchor late for a chord 40 ms past the
    // end, its early ratio one a squeeze-in never reads): the split keeps the
    // rule (check_equal_room_split) with the anchor going late and the end
    // moving later. Its squeeze-out mirror, the same sides and gap on the
    // other side of the end, goes early. Pitch Dark's own squeeze-ins are
    // checked the same way in its test further down.
    const schedule::SqueezeGeometry g = hand_built_squeeze_in();
    const schedule::SqueezeSplit s = schedule::split_squeeze(g);
    check_equal_room_split(g, s);
    CHECK(s.anchor_aim_ms > 0.0);
    CHECK(s.anchor_room_ms > 0.0);
    schedule::SqueezeGeometry mirror = g;
    mirror.anchor_press = Press::Early;
    mirror.squeeze_ms = g.end_ms - (g.squeeze_ms - g.end_ms);
    const schedule::SqueezeSplit m = schedule::split_squeeze(mirror);
    check_equal_room_split(mirror, m);
    CHECK(m.anchor_aim_ms < 0.0);
}

TEST_CASE("schedule: a squeeze whose end lies on its own chord is split with the anchor on its own time") {
    // The Fall of Troy - Mouths Like Sidewinder Missiles [highfine] has two
    // such squeezes: chord 740, a squeeze-out whose end tick is its own tick
    // 48480, led by chord 704; and chord 1240, a squeeze-in whose end tick is
    // its own tick 83328, led by chord 1208. All four chords have the same
    // side. With no gap and equal sides the equal-room lead is exactly zero:
    // the anchor is judged on its own time, which moves no end, and the
    // squeeze press alone sits on the far side of the end. That is a press,
    // not one the other way, so it is planned and not refused.
    //
    // The overfill shape with its clamped end moved onto chord 4's own tick
    // is the squeeze-out: every side there is the cap, so the sides are equal.
    Overfill o{plain_song({0, 768, 1536, 2304, 3072, 3840}), Activation{}};
    o.song.sequence[4].chord.add_note(NoteColor::Kick);
    o.act.timecode = o.song.timecode(768);
    o.act.bank_rise_ticks = {0, 768};
    o.act.sp_end_steps = {{768, 3000, SpEndKind::Activation}, {1536, 3072, SpEndKind::Clamped}};
    SPSqueeze out{};
    out.kind = SqueezeKind::SqOut;
    out.offset_ms = offset_from_sp_end(o.song.timecode(3072).ms(), o.song.timecode(3072).ms());
    REQUIRE_FALSE(out.needs_no_timing());
    o.act.sqinouts.push_back(out);
    o.act.sqout_tick = 3072;
    o.act.transfer_post = TransferScale{1.0, 1.0};
    const json sch = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}), core::default_rules());
    INFO(sch["refusals"].dump());
    CHECK(sch["refusals"].empty());
    const json& q = sch["activations"][0]["squeezes"][0];
    REQUIRE(q.contains("anchor_aim_ms"));
    const double end_ms = q["end_ms"].get<double>();
    CHECK(end_ms == sch["chords"][4]["ms"].get<double>());  // the end is on chord 4's own time
    CHECK(q["anchor_aim_ms"].get<double>() == 0.0);
    CHECK(q["planned_end_ms"].get<double>() == end_ms);  // the anchor moves no end
    CHECK(q["anchor_room_ms"].get<double>() == doctest::Approx(q["squeeze_room_ms"].get<double>()));
    const std::vector<json> anchor = presses_of(sch, 2, "anchor");
    REQUIRE(anchor.size() == 1);
    CHECK(anchor[0]["aim_ms"].get<double>() == 0.0);
    // The late gem is judged after the end, which is its own chord's time.
    const std::vector<json> late = presses_of(sch, 4, "squeeze_out_late");
    REQUIRE(late.size() == 1);
    CHECK(late[0]["band"]["from_ms"].get<double>() == end_ms);
    CHECK(late[0]["band"]["from_strict"].get<bool>());
    CHECK(late[0]["aim_ms"].get<double>() > 0.0);

    // The squeeze-in, chord 1240's shape: the hand-built squeeze-in with its
    // chord moved onto its end and the anchor's side set to the chord's.
    schedule::SqueezeGeometry in = hand_built_squeeze_in();
    in.squeeze_ms = in.end_ms;
    in.anchor_side_ms = in.squeeze_side_ms;
    REQUIRE(schedule::equal_room_lead_ms(in) == 0.0);
    std::vector<std::string> refusals;
    const std::optional<double> lead = schedule::anchor_lead_ms({in}, refusals);
    INFO((refusals.empty() ? std::string() : refusals[0]));
    REQUIRE(lead.has_value());
    CHECK(*lead == 0.0);
    CHECK(refusals.empty());
    const schedule::SqueezeSplit s = schedule::split_at_lead(in, *lead);
    CHECK(s.anchor_aim_ms == 0.0);
    CHECK(s.planned_end_ms == in.end_ms);
    CHECK(s.squeeze_band.to_ms == in.end_ms);  // judged by the end, its own time
    CHECK(s.squeeze_aim_ms < 0.0);
    CHECK(s.anchor_room_ms == doctest::Approx(s.squeeze_room_ms));
}

TEST_CASE("schedule: moving_side reads the anchor's press, not the squeeze's kind") {
    // On every scale the corpus stores (Activation::transfer_post): the side
    // an anchor moves the end by is the one core::transfer_side gives its
    // press, and a geometry's ratio follows its anchor's press the same way.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int unequal = 0;
    for_each_corpus_activation_json(cfg, [&](const std::string& path, const Activation& act, const json&,
                                        const std::vector<std::string>&) {
        if (!act.transfer_post) return;
        const TransferScale& scale = *act.transfer_post;
        if (scale.early != scale.late) ++unequal;
        INFO(path << ": the activation at tick " << act.timecode.ticks());
        schedule::SqueezeGeometry g = hand_built_squeeze_in();
        g.ratio = scale;
        for (const Press press : {Press::Early, Press::Late}) {
            CHECK(schedule::moving_side(press, scale) == transfer_side(scale, press));
            g.anchor_press = press;
            CHECK(schedule::moving_ratio(g) == transfer_side(scale, press));
        }
    });
    // A wrong pick shows only on a scale whose two sides differ; a corpus
    // with none would make every check above pass whichever side was read.
    CHECK(unequal > 0);
}

// ---- F1: the song end, the ready point, the late gems and the build -------
// docs/superpowers/plans/2026-11-08-player-audit-fixes.md, task F1.

namespace {

// The rules with the squeeze-out rule set to `rule`, everything else default.
core::Rules rules_with(core::SqOutRule rule) {
    core::Rules r = core::default_rules();
    r.sqout_rule = rule;
    return r;
}

}  // namespace

TEST_CASE("schedule: song_length_ms is app::song_length_ms") {
    // On the corpus, the field is the owner's answer for the chart's own
    // metadata, the call `hydra_replay schedule` and `dump` make.
    // Every corpus chart is asked, none skipped: a chart with no notes under
    // cfg still has metadata and an end event the owner may answer from.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.reading());
        INFO(path);
        const store::ChartTimingMeta meta = app::chart_timing_meta(std::nullopt, path);
        const std::optional<double> want = app::song_length_ms(meta, song);
        const json got = schedule::song_length_json(meta, song);
        if (!want) {
            CHECK(got.is_null());
            continue;
        }
        REQUIRE(got.is_number());
        CHECK(got.get<double>() == *want);
    }
    // The number half is met on a chart known to have a length: Pitch Dark's
    // schedule (testdata/replay/test_pitch_dark_ratio-schedule.json, written
    // by hydra_replay schedule) carries one.
    const std::string pitch = corpus::chart_named("test_pitch_dark_ratio");
    REQUIRE_FALSE(pitch.empty());
    const Song& pitch_song = corpus::song(pitch, cfg.reading());
    const store::ChartTimingMeta pitch_meta = app::chart_timing_meta(std::nullopt, pitch);
    REQUIRE(corpus::load_json(corpus::testdata_root() + "/replay/test_pitch_dark_ratio-schedule.json")["song_length_ms"]
                .is_number());
    CHECK(schedule::song_length_json(pitch_meta, pitch_song).is_number());

    // A hand-built song with no notes' end, no end event and no stated
    // length has no length: null, never a guess.
    const Song bare = plain_song({0, 768});
    REQUIRE_FALSE(app::song_length_ms(store::ChartTimingMeta{}, bare).has_value());
    CHECK(schedule::song_length_json(store::ChartTimingMeta{}, bare).is_null());
}

TEST_CASE("schedule: sp_ready_tick is the second bank arrival") {
    // Hand-built: three arrivals that are neither the activation's tick nor
    // any SP-end step's tick, so a value read from sp_end_steps or the
    // phrases shows. kSpActivationBars is 2 (core/timing.h), so the second
    // arrival, tick 384, is the ready point.
    const Song song = plain_song({0, 192, 384, 768, 1536});
    const ChordTimes chords = chord_times(song);
    Activation act;
    act.timecode = song.timecode(768);
    act.bank_rise_ticks = {192, 384, 576};
    act.sp_end_steps = {{768, 3840, SpEndKind::Activation}, {1536, 4608, SpEndKind::Collected}};
    act.transfer_post = TransferScale{1.0, 1.0};
    REQUIRE(act.sp_ready_tick() == std::optional<int64_t>(384));
    std::vector<std::string> refusals;
    const json j = schedule::activation_json(act, chords, song.timing(), refusals);
    CHECK(j["sp_ready_tick"].get<int64_t>() == 384);
    // 384 ticks at 120 BPM and 192 ticks a beat: two beats, 1,000 ms.
    CHECK(j["sp_ready_ms"].get<double>() == doctest::Approx(1000.0));
    CHECK(refusals.empty());

    // One arrival: null, and a refusal naming the activation.
    act.bank_rise_ticks = {384};
    CHECK_FALSE(act.sp_ready_tick().has_value());
    const json one = schedule::activation_json(act, chords, song.timing(), refusals);
    CHECK(one["sp_ready_tick"].is_null());
    CHECK(one["sp_ready_ms"].is_null());
    REQUIRE(refusals.size() == 1);
    CHECK(refusals[0].find("the activation at") != std::string::npos);
    CHECK(refusals[0].find("ready") != std::string::npos);

    // On every corpus path: each activation's field is the engine's own
    // ready point (Activation::sp_ready_tick), in ticks and through the
    // tempo map, and that point is no later than the activation.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int acts_seen = 0;
    for_each_corpus_schedule(cfg, [&](const std::string& path, const Song& s,
                                      const std::vector<const Activation*>& acts, const PathReplay&,
                                      const json& sch) {
        REQUIRE(sch["activations"].size() == acts.size());
        for (size_t k = 0; k < acts.size(); ++k) {
            INFO(path << " activation " << k);
            const Activation& a = *acts[k];
            ++acts_seen;
            // A real activation spends at least kSpActivationBars bars.
            REQUIRE(a.bank_rise_ticks.size() >= static_cast<size_t>(kSpActivationBars));
            const std::optional<int64_t> want = a.sp_ready_tick();
            REQUIRE(want.has_value());
            const json& row = sch["activations"][k];
            CHECK(row["sp_ready_tick"].get<int64_t>() == *want);
            CHECK(row["sp_ready_ms"].get<double>() == s.timing().timecode(*want).ms());
            CHECK(*want <= a.timecode.ticks());
        }
    });
    CHECK(acts_seen > 0);
}

TEST_CASE("schedule: the late press holds sqout_late_notes") {
    // The overfill shape (Freak 387's): chord 4, Red and Kick, squeezed out.
    const Overfill o = overfill();
    const ReplayResult r = replay_path(o.song, {});
    const Chord& chord4 = o.song.sequence[4].chord;

    // first_note: the late press holds place 0, the inside press place 1.
    const json first = schedule::schedule_json(o.song, {&o.act}, r, rules_with(core::SqOutRule::FirstNote));
    INFO(first["refusals"].dump());
    CHECK(first["refusals"].empty());
    const std::vector<json> late_first = presses_of(first, 4, "squeeze_out_late");
    const std::vector<json> inside_first = presses_of(first, 4, "squeeze_out_inside");
    REQUIRE(late_first.size() == 1);
    REQUIRE(inside_first.size() == 1);
    CHECK(late_first[0]["gems"].get<std::vector<size_t>>() == std::vector<size_t>{0});
    CHECK(inside_first[0]["gems"].get<std::vector<size_t>>() == std::vector<size_t>{1});
    CHECK(late_first[0]["gems"].get<std::vector<size_t>>() ==
          sqout_late_notes(chord4, core::SqOutRule::FirstNote));

    // whole_chord: the late press holds every place, and no gem is pressed
    // inside.
    const json whole = schedule::schedule_json(o.song, {&o.act}, r, rules_with(core::SqOutRule::WholeChord));
    INFO(whole["refusals"].dump());
    CHECK(whole["refusals"].empty());
    const std::vector<json> late_whole = presses_of(whole, 4, "squeeze_out_late");
    REQUIRE(late_whole.size() == 1);
    CHECK(late_whole[0]["gems"].get<std::vector<size_t>>() == std::vector<size_t>{0, 1});
    CHECK(late_whole[0]["gems"].get<std::vector<size_t>>() ==
          sqout_late_notes(chord4, core::SqOutRule::WholeChord));
    CHECK(presses_of(whole, 4, "squeeze_out_inside").empty());
    // The late press's aim is the squeeze's, under either rule.
    CHECK(late_whole[0]["aim_ms"] == late_first[0]["aim_ms"]);
}

TEST_CASE("schedule: a squeeze-out with no inside group gives its late gem an end-band fallback") {
    // Under whole_chord, the overfill's two-gem chord 4 ...
    const Overfill o = overfill();
    const json whole = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}),
                                               rules_with(core::SqOutRule::WholeChord));
    // ... and, under first_note, the same shape with a one-gem chord 4.
    Overfill single = overfill();
    single.song.sequence[4].chord = Chord{};
    single.song.sequence[4].chord.add_note(NoteColor::Red);
    const json one_gem = schedule::schedule_json(single.song, {&single.act}, replay_path(single.song, {}),
                                                 rules_with(core::SqOutRule::FirstNote));
    for (const json* s : {&whole, &one_gem}) {
        INFO((*s)["refusals"].dump());
        CHECK(presses_of(*s, 4, "squeeze_out_inside").empty());
        const std::vector<json> late = presses_of(*s, 4, "squeeze_out_late");
        REQUIRE(late.size() == 1);
        // With nothing to join, the late gem's fallback is an end-band of its
        // own: from its window's front edge to its own end, aimed at the
        // middle, paid inside. It holds no before_end_ms.
        const json& fb = late[0]["fallback"];
        CHECK(fb["action"] == "end_band");
        CHECK(fb["retimed"] == "inside");
        CHECK_FALSE(fb.contains("join_gems"));
        CHECK_FALSE(fb.contains("before_end_ms"));
        CHECK(fb["band"]["from_ms"].get<double>() == doctest::Approx(late[0]["window_from_ms"].get<double>()));
        CHECK(fb["band"]["to_ms"].get<double>() == doctest::Approx(late[0]["own_end_ms"].get<double>()));
        CHECK((*s)["chords"][4]["ms"].get<double>() + fb["aim_ms"].get<double>() ==
              doctest::Approx((fb["band"]["from_ms"].get<double>() + fb["band"]["to_ms"].get<double>()) / 2.0));
    }
    CHECK(presses_of(one_gem, 4, "squeeze_out_late")[0]["gems"].get<std::vector<size_t>>() ==
          std::vector<size_t>{0});
}

TEST_CASE("schedule: under first_note the late press is the first gem and the rest go inside") {
    // The overfill shape: chord 4, Red and Kick, squeezed out. The late
    // gem's fallback joins the inside group, so it lists the inside gems.
    const Overfill o = overfill();
    const ReplayResult r = replay_path(o.song, {});
    const json s = schedule::schedule_json(o.song, {&o.act}, r, rules_with(core::SqOutRule::FirstNote));
    const std::vector<json> late = presses_of(s, 4, "squeeze_out_late");
    const std::vector<json> inside = presses_of(s, 4, "squeeze_out_inside");
    REQUIRE(late.size() == 1);
    REQUIRE(inside.size() == 1);
    CHECK(late[0]["fallback"]["join_gems"] == inside[0]["gems"]);

    // On the corpus under the app's default rules (first_note): every late
    // press holds the gems sqout_late_notes names for its chord, and no
    // inside press holds any of them. Beg's best path squeezes chord 422
    // out (testdata/input/common/IB24/T1/Evans Blue - Beg [highfine]), so
    // the corpus holds at least one late press.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    REQUIRE(cfg.rules.sqout_rule == core::SqOutRule::FirstNote);
    int late_rows = 0;
    for_each_corpus_schedule(cfg, [&](const std::string& path, const Song& song,
                                      const std::vector<const Activation*>&, const PathReplay&, const json& sch) {
        for (const json& press : sch["presses"]) {
            if (press["rule"] != "squeeze_out_late" && press["rule"] != "squeeze_out_inside") continue;
            INFO(path << " chord " << press["chord"].get<int>());
            const Chord& chord = song.sequence.at(press["chord"].get<size_t>()).chord;
            const std::vector<size_t> late_gems = sqout_late_notes(chord, cfg.rules.sqout_rule);
            const std::vector<size_t> gems = press["gems"].get<std::vector<size_t>>();
            if (press["rule"] == "squeeze_out_late") {
                ++late_rows;
                CHECK(gems == late_gems);
            } else {
                for (size_t g : late_gems) CHECK(std::find(gems.begin(), gems.end(), g) == gems.end());
            }
        }
    });
    REQUIRE(late_rows > 0);
}

TEST_CASE("schedule: hydra_replay_build is written") {
    const Song song = plain_song({0, 768});
    const json s = schedule::schedule_json(song, {}, replay_path(song, {}), core::default_rules());
    REQUIRE(s.contains("hydra_replay_build"));
    const std::string build = s["hydra_replay_build"].get<std::string>();
    CHECK(build == std::string(schedule::replay_build()));
    // The stamp has the compiler's own date-and-time form: this file's
    // compiler stamp is the same length.
    const std::string compiler_stamp = __DATE__ " " __TIME__;
    CHECK(build.size() == compiler_stamp.size());
}

// ---- F2: one frame, TM-50, flagged skips, one decline rule -----------------
// docs/superpowers/plans/2026-11-08-player-audit-fixes.md, task F2.

namespace {

// Whether some line of `s`'s "refusals" holds every one of `parts`.
bool refusal_says(const json& s, const std::vector<std::string>& parts) {
    for (const json& line : s["refusals"]) {
        const std::string text = line.get<std::string>();
        bool all = true;
        for (const std::string& p : parts) all = all && text.find(p) != std::string::npos;
        if (all) return true;
    }
    return false;
}

// Whether `s` writes any press for chord `chord`.
bool has_press(const json& s, int chord) {
    for (const json& p : s["presses"])
        if (p["chord"].get<int>() == chord) return true;
    return false;
}

// One chord of `count` single-pad chords on one pad, at `ms[i]`, for the
// far-press check; indexes start at `first_index`.
std::vector<schedule::PadChord> pad_row(const std::vector<double>& ms, const schedule::Pad& pad, int first_index) {
    const std::vector<double> sides = chord_sides_ms(ms);
    std::vector<schedule::PadChord> out(ms.size());
    for (size_t i = 0; i < ms.size(); ++i) {
        out[i].index = first_index + static_cast<int>(i);
        out[i].ms = ms[i];
        out[i].side_ms = sides[i];
        out[i].chart_pads = {pad};
    }
    return out;
}

}  // namespace

TEST_CASE("schedule: every rule reads judge times, with no frame in them") {
    // One chord at chord_ms with a side of side_ms.
    const double chord_ms = 1000.0;
    const double side_ms = 50.0;
    // A press aimed on the back edge is judged on it: no room left; one ms
    // inside leaves one ms. The same on the front edge.
    CHECK(schedule::press_room_ms(side_ms, side_ms) == doctest::Approx(0.0));
    CHECK(schedule::press_room_ms(side_ms - 1.0, side_ms) == doctest::Approx(1.0));
    CHECK(schedule::press_room_ms(-side_ms, side_ms) == doctest::Approx(0.0));
    // The far-press check: judged on the back edge it is inside (TM-27:
    // equal is inside); half a ms later it leaves its window, and a band
    // whose upper edge leaves it is refused the same way.
    const double latest = schedule::last_judge_ms(chord_ms, side_ms);
    const std::vector<schedule::PadChord> one = lone_kick_chord(chord_ms, side_ms);
    CHECK(lone_press_inside(one, latest));
    const std::optional<std::string> out =
        schedule::far_press_refusal(one, 0, latest + 0.5, latest + 0.5, one[0].chart_pads);
    REQUIRE(out.has_value());
    CHECK(out->find("leaves its own window") != std::string::npos);
    CHECK(schedule::far_press_refusal(one, 0, chord_ms, latest + 0.5, one[0].chart_pads).has_value());
    // The three bands: a window band is the whole window, an end-band runs
    // from the front edge to the end with the end inside, and the late gem's
    // runs from its end, strict, to the back edge.
    const schedule::Band w = schedule::window_band(chord_ms, side_ms);
    CHECK(w.from_ms == chord_ms - side_ms);
    CHECK(w.to_ms == chord_ms + side_ms);
    CHECK_FALSE(w.from_strict);
    const schedule::Band e = schedule::end_band(chord_ms, side_ms, 1010.0);
    CHECK(e.from_ms == chord_ms - side_ms);
    CHECK(e.to_ms == 1010.0);
    CHECK_FALSE(e.from_strict);
    CHECK(schedule::band_middle_ms(e) == doctest::Approx(980.0));
    // An end past the back edge gives the whole window, not a band that
    // leaves it.
    CHECK(schedule::end_band(chord_ms, side_ms, chord_ms + side_ms + 20.0).to_ms == chord_ms + side_ms);
    const schedule::Band l = schedule::late_gem_band(chord_ms, side_ms, 1010.0);
    CHECK(l.from_ms == 1010.0);
    CHECK(l.to_ms == chord_ms + side_ms);
    CHECK(l.from_strict);
    CHECK(schedule::band_middle_ms(l) == doctest::Approx(1030.0));
}

TEST_CASE("schedule: far press follows TM-50") {
    // Two snares 100 ms apart, chords 20 and 21, each side 76.57 ms.
    const schedule::Pad snare{NoteColor::Red, false};
    std::vector<schedule::PadChord> two = pad_row({1000.0, 1100.0}, snare, 20);

    // Chord 20 judged 20 ms late (1,020), chord 21 judged 50 ms early
    // (1,050), inside chord 20's window: 20 is hit first, so neither is
    // refused.
    two[0].sends = {{snare, 1020.0, 1020.0}};
    two[1].sends = {{snare, 1050.0, 1050.0}};
    CHECK_FALSE(schedule::far_press_refusal(two, 0, 1020.0, 1020.0, {snare}).has_value());
    CHECK_FALSE(schedule::far_press_refusal(two, 1, 1050.0, 1050.0, {snare}).has_value());

    // The same with chord 20's press judged after chord 21's: 21's press
    // lands while 20 is unhit, and the game gives it to 20. Refused, naming
    // both.
    two[0].sends = {{snare, 1060.0, 1060.0}};
    const std::optional<std::string> later = schedule::far_press_refusal(two, 1, 1050.0, 1050.0, {snare});
    REQUIRE(later.has_value());
    CHECK(later->find("chord 21") != std::string::npos);
    CHECK(later->find("chord 20") != std::string::npos);
    CHECK(later->find("still unhit") != std::string::npos);

    // Judged at the same time, 20 is still unhit when 21's press lands.
    two[0].sends = {{snare, 1050.0, 1050.0}};
    CHECK(schedule::far_press_refusal(two, 1, 1050.0, 1050.0, {snare}).has_value());

    // A press inside a later same-pad note's window when that note gets no
    // press: chord 20 judged at 1,063, inside chord 21's window (from
    // 1,023.43). No refusal: the later-note branch is gone.
    two[1].sends.clear();
    CHECK_FALSE(schedule::far_press_refusal(two, 0, 1063.0, 1063.0, {snare}).has_value());
}

// Each skip the schedule can make has its own test case below, so a failure
// names its reason; the last one checks the flag on every corpus path.

TEST_CASE("schedule: a squeeze with no end in the record is refused by name") {
    // A squeeze whose end is not in the record (no SP-end steps): its chord
    // is refused, not pressed as if it were no squeeze.
    {
        const Song song = plain_song({0, 768, 1536, 2304, 3072, 3840});
        Activation act;
        act.timecode = song.timecode(768);
        act.bank_rise_ticks = {0, 768};
        SPSqueeze out{};
        out.kind = SqueezeKind::SqOut;
        out.offset_ms = -10.0;
        act.sqinouts.push_back(out);
        act.sqout_tick = 3072;
        act.transfer_post = TransferScale{1.0, 1.0};
        const json s = schedule::schedule_json(song, {&act}, replay_path(song, {}), core::default_rules());
        INFO(s["refusals"].dump());
        CHECK(s["chords"][4]["refused"].get<bool>());
        CHECK_FALSE(has_press(s, 4));
        CHECK(refusal_says(s, {"chord 4", "no end or anchor"}));
        CHECK_FALSE(s["chords"][3]["refused"].get<bool>());
    }
}

TEST_CASE("schedule: a squeeze led by a chord the replay lacks is refused by name") {
    // A squeeze led by a chord the replay does not have (the replay is of
    // a shorter chart): the squeeze chord is refused, and the read after
    // that missing chord is refused too.
    {
        const Song song = plain_song({0, 768, 1536, 2304, 3072, 3840});
        const Song shorter = plain_song({0, 768, 1536, 2304, 3072});
        Activation act;
        act.timecode = song.timecode(768);
        act.bank_rise_ticks = {0, 768};
        act.sp_end_steps = {{768, 3000, SpEndKind::Activation}, {3840, 4000, SpEndKind::Clamped}};
        SPSqueeze out{};
        out.kind = SqueezeKind::SqOut;
        out.offset_ms = -10.0;
        act.sqinouts.push_back(out);
        act.sqout_tick = 0;  // chord 0, led by the clamp on chord 5
        act.transfer_post = TransferScale{1.0, 1.0};
        const json s = schedule::schedule_json(song, {&act}, replay_path(shorter, {}), core::default_rules());
        INFO(s["refusals"].dump());
        REQUIRE(s["chords"].size() == 5);
        CHECK(s["chords"][0]["refused"].get<bool>());
        CHECK_FALSE(has_press(s, 0));
        CHECK(refusal_says(s, {"chord 0", "led by chord 5", "not of one chart"}));
        CHECK(refusal_says(s, {"read after chord 5", "not among the replay's 5 chords"}));
    }
}

TEST_CASE("schedule: an anchor that is also an end is refused by name") {
    // A chord that leads a squeeze and is also an end: the overfill's anchor,
    // chord 2, is also where a second activation ends.
    {
        const Overfill o = overfill();
        Activation early;
        early.timecode = o.song.timecode(0);
        early.bank_rise_ticks = {0, 0};
        early.sp_end_steps = {{0, 1536, SpEndKind::Activation}};
        early.transfer_post = TransferScale{1.0, 1.0};
        const json s = schedule::schedule_json(o.song, {&early, &o.act}, replay_path(o.song, {}),
                                               core::default_rules());
        INFO(s["refusals"].dump());
        CHECK(s["chords"][2]["refused"].get<bool>());
        CHECK_FALSE(has_press(s, 2));
        CHECK(refusal_says(s, {"chord 2", "leads a squeeze and is also a squeeze or an end"}));
    }
}

TEST_CASE("schedule: a squeeze with no stored ratio is refused by name") {
    // A squeeze with no stored ratio cannot be split: refused, not pressed.
    {
        Overfill o = overfill();
        o.act.transfer_post.reset();
        const json s = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}), core::default_rules());
        INFO(s["refusals"].dump());
        CHECK(s["chords"][4]["refused"].get<bool>());
        CHECK_FALSE(has_press(s, 4));
        CHECK(refusal_says(s, {"chord 4", "could not split"}));
    }
}

TEST_CASE("schedule: a squeeze timed from an unplanned read is refused by name") {
    // A squeeze whose anchor's read is not planned: the replay has the same
    // chord count but its chord 2 sits at tick 1540, not the 1536 the
    // activation's SP-end step names, so no step matches the replay's chord
    // 2 and the end read after it cannot be planned. The squeeze chord 4 is
    // timed from that read: refused, not pressed.
    {
        const Overfill o = overfill();
        const Song moved = plain_song({0, 768, 1540, 2304, 3072, 3840});
        const json s = schedule::schedule_json(o.song, {&o.act}, replay_path(moved, {}), core::default_rules());
        INFO(s["refusals"].dump());
        CHECK(s["chords"][4]["refused"].get<bool>());
        CHECK_FALSE(has_press(s, 4));
        CHECK(refusal_says(s, {"chord 4", "read after chord 2", "not planned"}));
    }
}

TEST_CASE("schedule: a skipped fill with no chord under it writes a refusal") {
    // A skipped fill with no chord under it: the two inputs are not of one
    // chart. The line is written; there is no chord row to flag.
    {
        Overfill o = overfill();
        o.act.skipped_fill_ticks = {3000};
        const json s = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}), core::default_rules());
        INFO(s["refusals"].dump());
        CHECK(refusal_says(s, {"no chord at skipped fill"}));
    }
}

TEST_CASE("schedule: a refused far press is not written as a press row") {
    // A refused far press is not written as a press row. The overfill shape
    // with chord 3 moved to tick 3060, 31.25 ms before the squeezed-out
    // chord 4 (Red and Kick), and chord 3 (Red alone) a skipped fill: its
    // only gem is its activation note, so the player sends it nothing, and
    // chord 4's inside Red press lands in chord 3's window while 3 is unhit.
    {
        Overfill o{plain_song({0, 768, 1536, 3060, 3072, 3840}), Activation{}};
        o.song.sequence[4].chord.add_note(NoteColor::Kick);
        o.act = overfill().act;
        o.act.skipped_fill_ticks = {3060};
        const json s = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}), core::default_rules());
        INFO(s["refusals"].dump());
        CHECK(s["chords"][3]["declined"].get<bool>());
        CHECK(s["chords"][3]["kept_gems"].empty());
        CHECK(s["chords"][4]["refused"].get<bool>());
        CHECK_FALSE(has_press(s, 4));
        CHECK(refusal_says(s, {"chord 4", "chord 3", "no press"}));
        CHECK_FALSE(s["chords"][3]["refused"].get<bool>());
    }
}

TEST_CASE("schedule: on every corpus path a chord is refused exactly when it has no press") {
    // On every corpus path: a row flagged refused has no press, a row with
    // no press is flagged refused, and a flag always comes with a line.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int rows = 0;
    for_each_corpus_schedule(cfg, [&](const std::string& path, const Song&, const std::vector<const Activation*>&,
                                      const PathReplay&, const json& s) {
        std::set<int> pressed;
        for (const json& press : s["presses"]) pressed.insert(press["chord"].get<int>());
        bool any_refused = false;
        for (const json& row : s["chords"]) {
            ++rows;
            const int idx = row["index"].get<int>();
            INFO(path << " chord " << idx);
            const bool refused = row["refused"].get<bool>();
            any_refused = any_refused || refused;
            CHECK(refused == (pressed.count(idx) == 0));
        }
        if (any_refused) CHECK_FALSE(s["refusals"].empty());
    });
    CHECK(rows > 0);
}

TEST_CASE("schedule: a squeeze-out that is also a skipped fill") {
    // The overfill shape, with the squeezed-out chord 4 a skipped fill. The
    // fill keeps every gem but the chord's activation note
    // (places_but_activation_note); the late gems are the ones the rule pays
    // outside (sqout_late_notes); the fill's kept gems that are not late are
    // the inside group.

    // Red and Kick under first_note: the late gem is one the fill keeps, so
    // it goes late, and the activation note is pressed nowhere.
    {
        Overfill o = overfill();
        o.act.skipped_fill_ticks = {3072};
        const Chord& chord4 = o.song.sequence[4].chord;
        const json s = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}),
                                               rules_with(core::SqOutRule::FirstNote));
        INFO(s["refusals"].dump());
        CHECK(s["refusals"].empty());
        const json& row = s["chords"][4];
        CHECK(row["declined"].get<bool>());
        CHECK_FALSE(row["refused"].get<bool>());
        const std::vector<size_t> kept = places_but_activation_note(row, chord4);
        CHECK(row["kept_gems"].get<std::vector<size_t>>() == kept);
        const std::vector<json> late = presses_of(s, 4, "squeeze_out_late");
        REQUIRE(late.size() == 1);
        const std::vector<size_t> late_gems = sqout_late_notes(chord4, core::SqOutRule::FirstNote);
        CHECK(late[0]["gems"].get<std::vector<size_t>>() == late_gems);
        // Every kept gem is late here, so there is no inside group.
        CHECK(late_gems == kept);
        CHECK(presses_of(s, 4, "squeeze_out_inside").empty());
    }

    // Red, Yellow and Kick under first_note: the fill keeps two gems; the
    // late one goes late and the other is the inside group.
    {
        Overfill o = overfill();
        o.song.sequence[4].chord.add_note(NoteColor::Yellow);
        o.act.skipped_fill_ticks = {3072};
        const Chord& chord4 = o.song.sequence[4].chord;
        const json s = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}),
                                               rules_with(core::SqOutRule::FirstNote));
        INFO(s["refusals"].dump());
        CHECK(s["refusals"].empty());
        const json& row = s["chords"][4];
        const std::vector<size_t> kept = places_but_activation_note(row, chord4);
        CHECK(row["kept_gems"].get<std::vector<size_t>>() == kept);
        const std::vector<size_t> late_gems = sqout_late_notes(chord4, core::SqOutRule::FirstNote);
        std::vector<size_t> inside_gems;
        for (size_t g : kept)
            if (std::find(late_gems.begin(), late_gems.end(), g) == late_gems.end()) inside_gems.push_back(g);
        REQUIRE_FALSE(inside_gems.empty());
        const std::vector<json> late = presses_of(s, 4, "squeeze_out_late");
        const std::vector<json> inside = presses_of(s, 4, "squeeze_out_inside");
        REQUIRE(late.size() == 1);
        REQUIRE(inside.size() == 1);
        CHECK(late[0]["gems"].get<std::vector<size_t>>() == late_gems);
        CHECK(inside[0]["gems"].get<std::vector<size_t>>() == inside_gems);
        CHECK(late[0]["fallback"]["join_gems"] == inside[0]["gems"]);
    }

    // A chord whose late gem is the activation note the fill leaves out:
    // refused naming the chord, and not pressed. Red alone under first_note,
    // and Red and Kick under whole_chord (every gem is late, Red among them).
    struct LeftOut {
        bool red_alone;
        core::SqOutRule rule;
    };
    for (const LeftOut& c : {LeftOut{true, core::SqOutRule::FirstNote}, LeftOut{false, core::SqOutRule::WholeChord}}) {
        Overfill o = overfill();
        if (c.red_alone) {
            o.song.sequence[4].chord = Chord{};
            o.song.sequence[4].chord.add_note(NoteColor::Red);
        }
        o.act.skipped_fill_ticks = {3072};
        const Chord& chord4 = o.song.sequence[4].chord;
        const json s = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}), rules_with(c.rule));
        INFO(s["refusals"].dump());
        // The case's premise: some late gem is not kept.
        const std::vector<size_t> kept = places_but_activation_note(s["chords"][4], chord4);
        const std::vector<size_t> late_gems = sqout_late_notes(chord4, c.rule);
        REQUIRE(std::any_of(late_gems.begin(), late_gems.end(), [&](size_t g) {
            return std::find(kept.begin(), kept.end(), g) == kept.end();
        }));
        CHECK(s["chords"][4]["refused"].get<bool>());
        CHECK_FALSE(has_press(s, 4));
        CHECK(refusal_says(s, {"chord 4", "skipped fill", "activation note"}));
    }
}

TEST_CASE("schedule: one decline rule") {
    // kept_gems is every gem but Chord::activation_note, on Freak's two
    // shapes: chord 162 (kick, blue cymbal, green cymbal) and chord 51
    // (kick, blue tom).
    using Colours = std::set<NoteColor>;
    CHECK(kept_colours(Colours{NoteColor::Kick, NoteColor::Blue, NoteColor::Green}) ==
          Colours{NoteColor::Kick, NoteColor::Blue});
    CHECK(kept_colours(Colours{NoteColor::Kick, NoteColor::Blue}) == Colours{NoteColor::Kick});
    // A kick alone is its own activation note: nothing is sent.
    CHECK(kept_colours(Colours{NoteColor::Kick}).empty());
    // Every colour of a five-gem chord but Green, the first in the order.
    CHECK(kept_colours(Colours{NoteColor::Kick, NoteColor::Red, NoteColor::Yellow, NoteColor::Blue, NoteColor::Green}) ==
          Colours{NoteColor::Kick, NoteColor::Red, NoteColor::Yellow, NoteColor::Blue});

    // The schedule writes no decline mode.
    const Song song = plain_song({0, 768});
    CHECK_FALSE(schedule::schedule_json(song, {}, replay_path(song, {}), core::default_rules()).contains("decline"));

    // On every corpus path: each declined chord keeps every gem but its
    // activation note, and nothing else. Several corpus charts skip fills on
    // their best path (Beg, Believer, Pitch Dark among them), so the corpus
    // holds declined chords.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int declined_rows = 0;
    for_each_corpus_schedule(cfg, [&](const std::string& path, const Song& s, const std::vector<const Activation*>&,
                                      const PathReplay&, const json& sch) {
        for (size_t idx = 0; idx < sch["chords"].size(); ++idx) {
            const json& row = sch["chords"][idx];
            if (!row["declined"].get<bool>()) continue;
            ++declined_rows;
            INFO(path << " chord " << idx);
            CHECK(row["kept_gems"].get<std::vector<size_t>>() ==
                  places_but_activation_note(row, s.sequence.at(idx).chord));
        }
    });
    REQUIRE(declined_rows > 0);
}

TEST_CASE("schedule: the first press group is the activation note") {
    // The overfill's activation chord 1, standing in for Freak's activation
    // chord 232 (that chart is not in the repo): Kick, Yellow and a Blue
    // cymbal. Its activation note is the Blue cymbal, pressed alone first.
    {
        Overfill o = overfill();
        o.song.sequence[1].chord.add_note(NoteColor::Kick);
        o.song.sequence[1].chord.add_note(NoteColor::Yellow);
        o.song.sequence[1].chord.add_note(NoteColor::Blue).cymbaltype = NoteCymbalType::Cymbal;
        const json s = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}), core::default_rules());
        INFO(s["refusals"].dump());
        const std::vector<json> act = presses_of(s, 1, "plain");
        REQUIRE(!act.empty());
        REQUIRE(act[0]["gems"].size() == 1);
        const json& gem = s["chords"][1]["gems"][act[0]["gems"][0].get<size_t>()];
        CHECK(gem["color"] == "Blue");
        CHECK(gem["cymbal"].get<bool>());
        CHECK(act[0]["follows"].is_null());
    }

    // On every corpus path: each activation chord's first press holds its
    // activation note alone. Every activation sits on a chord. Two kinds of
    // activation chord are passed over, each by name and count: one the
    // schedule refused (its row says so, and it has no press), and one
    // pressed first by a squeeze rule, whose gems come from the squeeze
    // (sqout_late_notes, or the whole chord for a squeeze-in), not from the
    // activation-first groups.
    const std::set<std::string> squeeze_rules{"squeeze_out_inside", "squeeze_out_late", "squeeze_in"};
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int seen = 0;
    int refused_acts = 0;
    int squeeze_first = 0;
    for_each_corpus_schedule(cfg, [&](const std::string& path, const Song& song,
                                      const std::vector<const Activation*>&, const PathReplay& pr,
                                      const json& s) {
        for (size_t k = 0; k < s["activations"].size(); ++k) {
            const json& a = s["activations"][k];
            const std::string where = path + " activation " + std::to_string(k);
            INFO(where);
            REQUIRE_FALSE(a["chord"].is_null());
            const int ci = a["chord"].get<int>();
            const json* first = nullptr;
            for (const json& press : s["presses"])
                if (press["chord"].get<int>() == ci) {
                    first = &press;
                    break;
                }
            if (!first) {
                ++refused_acts;
                CHECK(s["chords"][static_cast<size_t>(ci)]["refused"].get<bool>());
                continue;
            }
            if (squeeze_rules.count(first->at("rule").get<std::string>()) > 0) {
                ++squeeze_first;
                continue;
            }
            INFO(path << " chord " << ci << " " << first->at("rule").get<std::string>());
            ++seen;
            const ReplayChord& c = pr.result.chords[static_cast<size_t>(ci)];
            const NoteColor top = song.sequence[static_cast<size_t>(ci)].chord.activation_note().colortype;
            const std::vector<size_t> gems = first->at("gems").get<std::vector<size_t>>();
            REQUIRE(gems.size() == 1);
            CHECK(c.notes[gems[0]].color == top);
        }
    });
    // The passed-over chords, by kind, for a failure's reader.
    INFO("refused activation chords " << refused_acts << ", pressed first by a squeeze " << squeeze_first);
    // Pitch Dark's best path presses its activation chord 144
    // (testdata/input/test_pitch_dark_ratio) plain, Yellow first, so the
    // corpus holds a checked activation chord.
    REQUIRE(seen > 0);
}

// ---- T2: each read at its own step's scale, and the two ends of a press ----
// `docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md` (removed; `git show 48401572:docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md`),
// task T2.

namespace {

// Pitch Dark (testdata/input/test_pitch_dark_ratio, a copy of the library
// chart) under the app's default settings: the activations and schedule of
// its first stored path with a timed squeeze-in, built as the corpus sweeps
// build theirs. Its schedule from hydra_replay
// (testdata/replay/test_pitch_dark_ratio-schedule.json) holds two such
// presses, each timed from the read after its anchor; a path with none
// fails here at once.
struct PitchDark {
    const Song* song = nullptr;
    std::vector<const Activation*> acts;
    json sch;
};
PitchDark pitch_dark() {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const std::string path = corpus::chart_named("test_pitch_dark_ratio");
    if (path.empty()) throw std::runtime_error("the corpus has no test_pitch_dark_ratio chart");
    PitchDark pd;
    pd.song = &corpus::song(path, cfg.reading(), cfg.rules);
    const HydraRecord& rec = corpus::analyzed(path, cfg);
    for (const Path* p : rec.all_paths()) {
        std::vector<const Activation*> acts;
        for (const Activation& a : p->walk_activations()) acts.push_back(&a);
        const PathReplay pr = replay_stored_path(*pd.song, *p, cfg.rules);
        json sch = schedule::schedule_json(*pd.song, acts, pr.result, cfg.rules);
        const bool timed_in = std::any_of(sch["presses"].begin(), sch["presses"].end(),
                                          [](const json& press) { return press["rule"] == "squeeze_in"; });
        if (!timed_in) continue;
        pd.acts = std::move(acts);
        pd.sch = std::move(sch);
        return pd;
    }
    throw std::runtime_error("no stored Pitch Dark path has a timed squeeze-in");
}

// The one read after `chord`.
const json& read_after(const json& sch, int chord) {
    const json& reads = sch["chords"][static_cast<size_t>(chord)]["reads"];
    if (reads.size() != 1) throw std::runtime_error("chord " + std::to_string(chord) + " has no single read");
    return reads[0];
}

// The own end the player's sum gives for a read of `read_ms` on press `p`
// (retime_from_end in tools/ch_probe/experiments/play_chart.py does it live).
double own_end_at_read(const json& p, double read_ms) {
    const double stored = p["read_stored_end_ms"].get<double>();
    const double c = p["conversion"][read_ms >= stored ? "late" : "early"].get<double>();
    return p["own_stored_end_ms"].get<double>() + c * (read_ms - stored);
}

// The overfill shape with a third step: the clamp on chord 2 sets an end at
// tick 2950, chord 3 collects to the final end at tick 3082, and chord 4
// (Red and Kick) is squeezed out of that final end. So the read after chord
// 2 sees the clamp's end and the squeeze-out's own end is the final one, a
// step later. `change_at` puts a tempo change (120 to 150 BPM) on that tick.
// The stored ratio is what the engine would store, step_transfer_scale of
// the last step.
Overfill three_step_overfill(std::optional<int64_t> change_at) {
    Song song(192);
    song.bpm_changes[0] = 120.0;
    if (change_at) song.bpm_changes[*change_at] = 150.0;
    song.build_timing();
    for (int64_t tick : {0, 768, 1536, 2304, 3072, 3840}) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);
        song.sequence.push_back(ts);
    }
    Overfill o{std::move(song), Activation{}};
    o.song.sequence[4].chord.add_note(NoteColor::Kick);
    o.act.timecode = o.song.timecode(768);
    o.act.bank_rise_ticks = {0, 768};
    o.act.sp_end_steps = {{768, 2900, SpEndKind::Activation},
                          {1536, 2950, SpEndKind::Clamped},
                          {2304, 3082, SpEndKind::Collected}};
    SPSqueeze out{};
    out.kind = SqueezeKind::SqOut;
    out.offset_ms = o.song.timecode(3072).ms() - o.song.timecode(3082).ms();
    o.act.sqinouts.push_back(out);
    o.act.sqout_tick = 3072;
    o.act.transfer_post = step_transfer_scale(o.act, 2, o.song.timing());
    return o;
}

}  // namespace

TEST_CASE("schedule: every end-band press runs from its window's front edge to its own end") {
    // A chord on a final end, each inside group of a squeeze-out, a chord
    // paid inside before a moved end and a squeeze-in: each band runs from
    // the front edge of its chord's window to the end it must beat, and its
    // target is the band's middle. A chord paid inside before a moved end is
    // banded only when its own time is after that end.
    std::set<std::string> seen;
    auto check = [&](const json& sch) {
        for (const json& p : sch["presses"]) {
            const std::string rule = p["rule"].get<std::string>();
            if (rule != "sp_end" && rule != "squeeze_out_inside" && rule != "inside_moved_end" &&
                rule != "squeeze_in")
                continue;
            seen.insert(rule);
            INFO("chord " << p["chord"].get<int>() << " " << rule);
            const double ms = sch["chords"][p["chord"].get<size_t>()]["ms"].get<double>();
            const json& band = p["band"];
            CHECK(band["from_ms"].get<double>() == doctest::Approx(p["window_from_ms"].get<double>()));
            CHECK(band["to_ms"].get<double>() ==
                  doctest::Approx(std::min(p["own_end_ms"].get<double>(), p["window_to_ms"].get<double>())));
            CHECK_FALSE(band["from_strict"].get<bool>());
            CHECK(ms + p["aim_ms"].get<double>() ==
                  doctest::Approx((band["from_ms"].get<double>() + band["to_ms"].get<double>()) / 2.0));
            if (rule == "inside_moved_end") CHECK(ms > p["own_end_ms"].get<double>());
        }
    };
    check(pitch_dark().sch);
    const Overfill o = overfill();
    check(schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}), core::default_rules()));
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    for_each_corpus_schedule(cfg, [&](const std::string& path, const Song&, const std::vector<const Activation*>&,
                                      const PathReplay&, const json& sch) {
        INFO(path);
        check(sch);
    });
    INFO("rules seen: " << seen.size());
    CHECK(seen.count("squeeze_out_inside") == 1);
    CHECK(seen.count("squeeze_in") == 1);
    CHECK(seen.count("sp_end") == 1);
}

TEST_CASE("schedule: a chord paid inside before a moved end is banded only when it is after that end") {
    // The overfill shape with the squeeze-out's end one tick after chord 5
    // (tick 3073), so the anchor's early lead moves the final end before
    // chord 5's own time. Chord 4 (Yellow, tick 3068) sits between that moved
    // end and chord 5; chord 3 (Red, tick 2304) sits well before it. A Blue
    // chord at tick 3080, past the end, gives chord 5 a near neighbour on
    // each side, so its side is narrow and the lead long. Both chords 3 and 4
    // are paid inside by the window; SP-29 pays chord 4 inside only if it is
    // judged by the moved end, so it gets the end-band row, and chord 3 is
    // pressed plain at its own time.
    Overfill o{plain_song({0, 768, 1536, 2304, 3068, 3072, 3080}), Activation{}};
    o.song.sequence[4].chord = Chord{};
    o.song.sequence[4].chord.add_note(NoteColor::Yellow);
    o.song.sequence[5].chord.add_note(NoteColor::Kick);
    o.song.sequence[6].chord = Chord{};
    o.song.sequence[6].chord.add_note(NoteColor::Blue);
    const int64_t end_tick = 3073;
    o.act.timecode = o.song.timecode(768);
    o.act.bank_rise_ticks = {0, 768};
    o.act.sp_end_steps = {{768, 3000, SpEndKind::Activation}, {1536, end_tick, SpEndKind::Clamped}};
    SPSqueeze out{};
    out.kind = SqueezeKind::SqOut;
    out.offset_ms = offset_from_sp_end(o.song.timecode(3072).ms(), o.song.timecode(end_tick).ms());
    o.act.sqinouts.push_back(out);
    o.act.sqout_tick = 3072;
    o.act.transfer_post = TransferScale{1.0, 1.0};
    ReplayWindow w;
    w.act_tick = 768;
    w.deact_tick = end_tick;
    w.sqout_tick = 3072;
    const json s = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {w}), core::default_rules());
    INFO(s["refusals"].dump());
    CHECK(s["refusals"].empty());
    // The moved end is the read after chord 2, the anchor the clamp names,
    // as the anchor's lead plans it: earlier than stored.
    const json& reads = s["chords"][2]["reads"];
    REQUIRE(reads.size() == 1);
    const double moved_end = reads[0]["planned_end_ms"].get<double>();
    REQUIRE(moved_end < reads[0]["stored_end_ms"].get<double>());
    const double ms3 = s["chords"][3]["ms"].get<double>();
    const double ms4 = s["chords"][4]["ms"].get<double>();
    // Both chords are paid inside by the activation, one on each side of the
    // moved end.
    REQUIRE_FALSE(s["chords"][3]["paid_by"].empty());
    REQUIRE_FALSE(s["chords"][4]["paid_by"].empty());
    REQUIRE(ms3 < moved_end);
    REQUIRE(ms4 > moved_end);
    // Chord 4, after the moved end: one end-band row, judged by that end.
    const std::vector<json> banded = presses_of(s, 4, "inside_moved_end");
    REQUIRE(banded.size() == 1);
    CHECK(presses_of(s, 4, "plain").empty());
    CHECK(banded[0]["band"]["to_ms"].get<double>() == doctest::Approx(moved_end));
    CHECK(banded[0]["band"]["from_ms"].get<double>() == doctest::Approx(banded[0]["window_from_ms"].get<double>()));
    // Chord 3, before the moved end: plain, at its own time, in its window.
    CHECK(presses_of(s, 3, "inside_moved_end").empty());
    const std::vector<json> plain = presses_of(s, 3, "plain");
    REQUIRE(plain.size() == 1);
    CHECK(plain[0]["aim_ms"].get<double>() == 0.0);
    CHECK(plain[0]["band"]["from_ms"].get<double>() == plain[0]["window_from_ms"].get<double>());
    CHECK(plain[0]["band"]["to_ms"].get<double>() == plain[0]["window_to_ms"].get<double>());
}

TEST_CASE("schedule: Pitch Dark prices each read at its own step's scale") {
    const PitchDark pd = pitch_dark();
    const json& sch = pd.sch;
    INFO(sch["refusals"].dump());
    const SongTiming& timing = pd.song->timing();

    // Every read is priced at its own step's scale (step_transfer_scale of
    // the step it names), and a read after a plain-pressed chord, judged at
    // its own time, is planned at its stored end.
    std::set<int> led;
    for (const json& p : sch["presses"])
        if (p["rule"] == "anchor") led.insert(p["chord"].get<int>());
    for (size_t x = 0; x < sch["chords"].size(); ++x) {
        for (const json& r : sch["chords"][x]["reads"]) {
            INFO("the read after chord " << x);
            const Activation& act = *pd.acts.at(r["activation"].get<size_t>());
            const size_t step = r["step"].get<size_t>();
            const std::optional<TransferScale> own = step_transfer_scale(act, step, timing);
            REQUIRE(own.has_value());
            CHECK(r["ratio"]["early"].get<double>() == own->early);
            CHECK(r["ratio"]["late"].get<double>() == own->late);
            CHECK(r["stored_end_ms"].get<double>() ==
                  doctest::Approx(timing.timecode(act.sp_end_steps.at(step).end_tick).ms()));
            if (led.count(static_cast<int>(x)) == 0) {
                CHECK(r["planned_end_ms"].get<double>() == r["stored_end_ms"].get<double>());
            }
        }
    }

    // Every press timed from an end: its aim assumes the read after the chord
    // it names, as planned when that chord is led and else as planned or as
    // stored; its read end is that read's stored end, and its own end follows
    // from the read it assumed through its conversion (the player's sum).
    int timed = 0;
    for (const json& p : sch["presses"]) {
        if (p["end_read_after"].is_null()) continue;
        ++timed;
        INFO("chord " << p["chord"].get<int>() << " " << p["rule"].get<std::string>());
        const json& r = read_after(sch, p["end_read_after"].get<int>());
        const double aim_end = p["aim_end_ms"].get<double>();
        CHECK((aim_end == r["planned_end_ms"].get<double>() || aim_end == r["stored_end_ms"].get<double>()));
        if (led.count(p["end_read_after"].get<int>()) > 0) {
            CHECK(aim_end == r["planned_end_ms"].get<double>());
        }
        CHECK(p["read_stored_end_ms"].get<double>() == r["stored_end_ms"].get<double>());
        CHECK(p["own_end_ms"].get<double>() == doctest::Approx(own_end_at_read(p, p["aim_end_ms"].get<double>())));
        const json& row = sch["chords"][p["chord"].get<size_t>()];
        if (p["rule"] == "squeeze_in") {
            // Its plain fallback's threshold is in read terms: fed back
            // through the conversion it gives the own-terms threshold.
            const double read_threshold = p["fallback"]["if_end_before_ms"].get<double>();
            CHECK(own_end_at_read(p, read_threshold) ==
                  doctest::Approx(schedule::sqin_fallback_before_ms(row["ms"].get<double>(),
                                                                    row["side_ms"].get<double>())));
        }
        if (p["rule"] == "sp_end") {
            // A press on the final end converts from the read's step to the
            // final end's ratio.
            const json& act = sch["activations"][r["activation"].get<size_t>()];
            for (const char* side : {"early", "late"})
                CHECK(p["conversion"][side].get<double>() * r["ratio"][side].get<double>() ==
                      doctest::Approx(act["end_ratio"][side].get<double>()));
        }
    }
    CHECK(timed > 0);  // pitch_dark() chose a path with a timed squeeze-in

    // Pitch Dark's timed squeezes keep the equal-room split: where an anchor
    // leads one squeeze alone, both rooms are equal, each is press_room_ms of
    // its aim (the squeeze press's at the end it must clear) and chord side,
    // a squeeze-in's anchor goes late and a squeeze-out's early, and the
    // squeeze is feasible exactly when Hydra's safe sum for it is not short.
    std::map<int, int> squeezes_on;
    for (const json& a : sch["activations"])
        for (const json& q : a["squeezes"])
            if (q.contains("anchor_room_ms")) ++squeezes_on[q["anchor_chord"].get<int>()];
    const ChordTimes pd_chords = chord_times(*pd.song);
    int squeeze_ins = 0;
    for (size_t ai = 0; ai < sch["activations"].size(); ++ai) {
        const json& a = sch["activations"][ai];
        for (size_t k = 0; k < a["squeezes"].size(); ++k) {
            const json& q = a["squeezes"][k];
            if (!q.contains("anchor_room_ms") || squeezes_on[q["anchor_chord"].get<int>()] != 1) continue;
            INFO("the squeeze on chord " << q["chord"].dump());
            const bool is_in = q["kind"] == "SqIn";
            if (is_in) ++squeeze_ins;
            const double anchor_aim = q["anchor_aim_ms"].get<double>();
            const double anchor_side = sch["chords"][q["anchor_chord"].get<size_t>()]["side_ms"].get<double>();
            const double squeeze_side = sch["chords"][q["chord"].get<size_t>()]["side_ms"].get<double>();
            const double anchor_room = q["anchor_room_ms"].get<double>();
            const double squeeze_room = q["squeeze_room_ms"].get<double>();
            CHECK((is_in ? anchor_aim > 0.0 : anchor_aim < 0.0));
            CHECK(anchor_room == doctest::Approx(schedule::press_room_ms(anchor_aim, anchor_side)));
            const double squeeze_ms = sch["chords"][q["chord"].get<size_t>()]["ms"].get<double>();
            CHECK(squeeze_room == doctest::Approx(schedule::press_room_ms(q["aim_end_ms"].get<double>() - squeeze_ms,
                                                                         squeeze_side)));
            CHECK(anchor_room == doctest::Approx(squeeze_room));
            const std::optional<WindowEnds> ends = squeeze_window_ends(*pd.acts.at(ai), k, pd_chords);
            REQUIRE(ends.has_value());
            CHECK(q["feasible"].get<bool>() == !ends->is_short());
        }
    }
    CHECK(squeeze_ins > 0);

    // Every press with no end carries none of the end fields.
    for (const json& p : sch["presses"]) {
        if (!p["end_read_after"].is_null()) continue;
        CHECK(p["own_end_ms"].is_null());
        CHECK(p["conversion"].is_null());
    }
}

TEST_CASE("schedule: Pitch Dark's read range is its read end's steady span") {
    const PitchDark pd = pitch_dark();
    const json& sch = pd.sch;
    INFO(sch["refusals"].dump());

    // Each timed press's read range is its read end as stored, less the
    // steady span before that end and plus the one after it
    // (steady_spans from the read's chord to its step's end), each side
    // null where no tempo change bounds it. A press with no end has none.
    const SongTiming& timing = pd.song->timing();
    int timed = 0;
    for (const json& p : sch["presses"]) {
        INFO("chord " << p["chord"].get<int>() << " " << p["rule"].get<std::string>());
        if (p["end_read_after"].is_null()) {
            CHECK(p["read_range_ms"].is_null());
            continue;
        }
        ++timed;
        const int x = p["end_read_after"].get<int>();
        const json& r = read_after(sch, x);
        const Activation& act = *pd.acts.at(r["activation"].get<size_t>());
        const int64_t end_tick = act.sp_end_steps.at(r["step"].get<size_t>()).end_tick;
        const SteadySpans spans =
            steady_spans(sch["chords"][static_cast<size_t>(x)]["tick"].get<int64_t>(), end_tick, timing);
        const double stored = r["stored_end_ms"].get<double>();
        const json& range = p["read_range_ms"];
        REQUIRE(range.is_array());
        REQUIRE(range.size() == 2);
        if (spans.before_end_ms) {
            CHECK(range[0].get<double>() == doctest::Approx(stored - *spans.before_end_ms));
        } else {
            CHECK(range[0].is_null());
        }
        if (spans.after_end_ms) {
            CHECK(range[1].get<double>() == doctest::Approx(stored + *spans.after_end_ms));
        } else {
            CHECK(range[1].is_null());
        }
    }
    CHECK(timed > 0);  // pitch_dark() chose a path with a timed squeeze-in
}

TEST_CASE("schedule: a lead that moves the read end past a tempo change is refused naming its span") {
    // The hand-built squeeze-in of the mirror case above. Its read end is a
    // step of its own, with a tempo change 10 ms after it; the read end's
    // ratio sets how far the anchor's lead moves it.
    schedule::SqueezeGeometry g = hand_built_squeeze_in();
    const double lead = schedule::equal_room_lead_ms(g);
    schedule::ReadEnd read;
    read.tick = 900;
    read.steady.after_end_ms = 10.0;

    // A lead that moves the read end 20 ms is refused, naming the read end's
    // span.
    read.ratio = TransferScale{1.0, 20.0 / lead};
    g.read_end = read;
    std::vector<std::string> refusals;
    CHECK_FALSE(schedule::anchor_lead_ms({g}, refusals).has_value());
    REQUIRE(refusals.size() == 1);
    CHECK(refusals[0].find("anchor chord 1") != std::string::npos);
    CHECK(refusals[0].find("the read end at tick 900") != std::string::npos);
    CHECK(refusals[0].find("the read end's steady span after_end_ms") != std::string::npos);

    // One that moves it 5 ms is not.
    read.ratio = TransferScale{1.0, 5.0 / lead};
    g.read_end = read;
    refusals.clear();
    CHECK(schedule::anchor_lead_ms({g}, refusals).has_value());
    CHECK(refusals.empty());
}

TEST_CASE("schedule: a tempo change on an end splits the conversion's sides") {
    // The change on the own end's tick: that end's scale has two sides, the
    // read end's has one, so the conversion's sides differ, and the file
    // carries both.
    {
        const Overfill o = three_step_overfill(int64_t{3082});
        const json s = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}), core::default_rules());
        INFO(s["refusals"].dump());
        const std::vector<json> late = presses_of(s, 4, "squeeze_out_late");
        REQUIRE(late.size() == 1);
        const std::optional<TransferScale> own = step_transfer_scale(o.act, 2, o.song.timing());
        const std::optional<TransferScale> read = step_transfer_scale(o.act, 1, o.song.timing());
        REQUIRE(own.has_value());
        REQUIRE(read.has_value());
        REQUIRE(own->early != own->late);
        const json& conv = late[0]["conversion"];
        CHECK(conv["early"].get<double>() != conv["late"].get<double>());
        const TransferScale want = schedule::end_conversion(*own, *read);
        CHECK(conv["early"].get<double>() == want.early);
        CHECK(conv["late"].get<double>() == want.late);
        CHECK(late[0]["end_read_after"].get<int>() == 2);
        CHECK(read_after(s, 2)["step"].get<size_t>() == 1);
    }
    // A change on the anchor's tick moves both ends' scales alike, so it
    // cancels in the conversion; with no change at all the sides are 1.
    for (const std::optional<int64_t> change : {std::optional<int64_t>{1536}, std::optional<int64_t>{}}) {
        const Overfill o = three_step_overfill(change);
        const json s = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}), core::default_rules());
        INFO(s["refusals"].dump());
        const std::vector<json> late = presses_of(s, 4, "squeeze_out_late");
        REQUIRE(late.size() == 1);
        const json& conv = late[0]["conversion"];
        CHECK(conv["early"].get<double>() == doctest::Approx(conv["late"].get<double>()));
    }
}

TEST_CASE("schedule: read_at_own_ms runs the conversion backwards on its own side") {
    // The read that gives an own end is the stored read plus the own end's
    // distance from the stored own end, divided by the conversion's side for
    // that distance. The sides differ, so a wrong pick shows.
    schedule::EndLink l;
    const double read_stored = 1000.0;
    const double own_stored = 2000.0;
    const TransferScale sides{0.5, 2.0};
    l.read_stored_end_ms = read_stored;
    l.own_stored_end_ms = own_stored;
    l.conversion = sides;
    // At or after the own stored end: the late side.
    const double after = own_stored + 10.0;
    CHECK(schedule::read_at_own_ms(l, after) == doctest::Approx(read_stored + (after - own_stored) / sides.late));
    CHECK(schedule::read_at_own_ms(l, own_stored) == doctest::Approx(read_stored));
    // Before it: the early side.
    const double before = own_stored - 10.0;
    CHECK(schedule::read_at_own_ms(l, before) == doctest::Approx(read_stored + (before - own_stored) / sides.early));
    // Two ends of one step: the conversion moves the read one for one, on
    // both sides.
    l.conversion = schedule::end_conversion(TransferScale{1.3, 1.1}, TransferScale{1.3, 1.1});
    CHECK(schedule::read_at_own_ms(l, after) == doctest::Approx(read_stored + (after - own_stored)));
    CHECK(schedule::read_at_own_ms(l, before) == doctest::Approx(read_stored + (before - own_stored)));
}
