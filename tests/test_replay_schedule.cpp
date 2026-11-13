// hydra_replay schedule (tools/replay_schedule.h): the geometry it reads off
// the engine, the game's per-note window and the constants, and the rule that
// nothing under src/ reads any of it.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
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

// A side at the floor and at the cap: half of TM-21's whole window at a
// summed gap of 75 and of 170 (TM-22 prints them as 48.15 and 85.72).
const doctest::Approx kFloorSide = doctest::Approx(96.2931672 / 2);
const doctest::Approx kCapSide = doctest::Approx(171.4313075 / 2);

// Every stored path of every corpus chart that has notes under `cfg`:
// replayed, and its schedule built by schedule::schedule_json.
// `check` gets the chart's path, its song, the path's activations in walk
// order, the replay and the schedule.
using ScheduleCheck = std::function<void(const std::string& path, const Song& song,
                                         const std::vector<const Activation*>& acts,
                                         const PathReplay& pr, const json& sch)>;

void for_each_corpus_schedule(const app::AnalysisSettings& cfg, const ScheduleCheck& check) {
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.reading());
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(path, cfg);
        for (const Path* p : rec.all_paths()) {
            std::vector<const Activation*> acts;
            for (const Activation& a : p->walk_activations()) acts.push_back(&a);
            const PathReplay pr = replay_stored_path(song, *p, cfg.rules);
            const json sch = schedule::schedule_json(song, acts, pr.result, cfg.rules);
            check(path, song, acts, pr, sch);
        }
    }
}

}  // namespace

TEST_CASE("schedule: every end step names its anchor") {
    const Song song = plain_song({0, 768, 1536, 2304, 3072, 3840});
    const ChordTimes chords = chord_times(song);

    Activation act;
    act.timecode = song.timecode(768);
    act.bank_rise_ticks = {0, 768};  // two bars, so the ready point is known
    act.sp_end_steps = {{768, 3840, SpEndKind::Activation},
                        {1536, 4608, SpEndKind::Collected},
                        {2304, 4800, SpEndKind::Clamped},
                        {3072, 5000, SpEndKind::SqIn}};
    SPSqueeze in{};
    in.kind = SqueezeKind::SqIn;
    in.offset_ms = 10.0;
    in.transfer = TransferScale{1.0, 1.0};
    act.sqinouts.push_back(in);
    act.transfer_post = TransferScale{1.0, 1.0};

    std::vector<std::string> refusals;
    const json j = schedule::activation_json(act, chords, song.timing(), refusals);
    const json& steps = j["sp_end_steps"];
    REQUIRE(steps.size() == 4);
    // The clamp at 2304 anchors its own step and every later one; the steps
    // before it are anchored on the activation.
    const int64_t want[] = {768, 768, 2304, 2304};
    for (size_t s = 0; s < 4; ++s) {
        INFO("step " << s);
        CHECK(steps[s]["anchor_tick"].get<int64_t>() == want[s]);
        CHECK(steps[s]["anchor_tick"].get<int64_t>() == act.end_anchor_tick(s));
    }
    CHECK(steps[2]["kind"] == "clamped");
    CHECK(j["read_end_after_tick"].get<int64_t>() == 2304);
    CHECK(j["read_end_after"].get<int>() == 3);  // the chord on 2304
    CHECK(j["chord"].get<int>() == 1);
    CHECK(refusals.empty());

    // No clamp: every end moves with the activation's own chord.
    Activation plain;
    plain.timecode = song.timecode(768);
    plain.sp_end_steps = {{768, 3840, SpEndKind::Activation}, {1536, 4608, SpEndKind::Collected}};
    plain.transfer_post = TransferScale{1.0, 1.0};
    const json p = schedule::activation_json(plain, chords, song.timing(), refusals);
    CHECK(p["read_end_after_tick"].get<int64_t>() == 768);
    CHECK(p["read_end_after"].get<int>() == 1);
    CHECK(p["sp_end_steps"][1]["anchor_tick"].get<int64_t>() == 768);
}

TEST_CASE("schedule: a squeeze carries its own end, anchor and stored ratio") {
    const Song song = plain_song({0, 768, 1536, 2304, 3072, 3840, 4000});
    const ChordTimes chords = chord_times(song);

    // A SqIn on the step at 3072 and a SqOut of the chord at 4000. Each
    // ratio is a value no recompute would give, so a recompute shows.
    Activation act;
    act.timecode = song.timecode(768);
    act.bank_rise_ticks = {0, 768};  // two bars, so the ready point is known
    act.sp_end_steps = {{768, 3000, SpEndKind::Activation},
                        {2304, 3500, SpEndKind::Clamped},
                        {3072, 3900, SpEndKind::SqIn}};
    SPSqueeze in{};
    in.kind = SqueezeKind::SqIn;
    in.offset_ms = 20.0;
    in.transfer = TransferScale{1.02, 1.03};
    SPSqueeze out{};
    out.kind = SqueezeKind::SqOut;
    out.offset_ms = -30.0;
    act.sqinouts = {in, out};
    act.sqout_tick = 4000;
    act.transfer_post = TransferScale{1.04, 1.05};

    std::vector<std::string> refusals;
    const json j = schedule::activation_json(act, chords, song.timing(), refusals);
    const json& q = j["squeezes"];
    REQUIRE(q.size() == 2);

    CHECK(q[0]["kind"] == "SqIn");
    CHECK(q[0]["chord_tick"].get<int64_t>() == 3072);
    CHECK(q[0]["end_tick"].get<int64_t>() == *act.squeeze_end_tick(0));
    CHECK(q[0]["end_tick"].get<int64_t>() == 3500);
    CHECK(q[0]["anchor_tick"].get<int64_t>() == *act.squeeze_anchor_tick(0));
    CHECK(q[0]["anchor_tick"].get<int64_t>() == 2304);
    CHECK(q[0]["ratio"]["early"].get<double>() == 1.02);
    CHECK(q[0]["ratio"]["late"].get<double>() == 1.03);

    CHECK(q[1]["kind"] == "SqOut");
    CHECK(q[1]["chord_tick"].get<int64_t>() == 4000);
    CHECK(q[1]["end_tick"].get<int64_t>() == 3900);
    CHECK(q[1]["anchor_tick"].get<int64_t>() == 2304);
    CHECK(q[1]["ratio"]["early"].get<double>() == 1.04);
    CHECK(q[1]["ratio"]["late"].get<double>() == 1.05);
    CHECK(refusals.empty());

    // A transfer_post left unset: null, and a line saying so.
    act.transfer_post.reset();
    refusals.clear();
    const json k = schedule::activation_json(act, chords, song.timing(), refusals);
    CHECK(k["squeezes"][1]["ratio"]["early"].is_null());
    CHECK(k["squeezes"][1]["ratio"]["late"].is_null());
    REQUIRE(refusals.size() == 1);
    CHECK(refusals[0].find("transfer_post") != std::string::npos);
}

TEST_CASE("schedule: paid_by matches in_sp on every corpus path") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int rows = 0;
    std::string first_bad;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.reading());
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(path, cfg);
        for (const Path* p : rec.all_paths()) {
            const PathReplay pr = replay_stored_path(song, *p, cfg.rules);
            for (const ReplayChord& c : pr.result.chords) {
                ++rows;
                if (c.paid_by.empty() == c.in_sp && first_bad.empty())
                    first_bad = path + ": chord " + std::to_string(c.index) + " paid_by and in_sp disagree";
                for (const ReplayPayment& pay : c.paid_by) {
                    bool found = false;
                    for (const ReplayWindow& w : pr.windows) {
                        if (w.act_tick != pay.act_tick || w.deact_tick != pay.deact_tick) continue;
                        found = true;
                        const double deact_ms = song.timing().timecode(w.deact_tick).ms();
                        if (pay.offset_ms != offset_from_sp_end(c.ms, deact_ms) && first_bad.empty())
                            first_bad = path + ": chord " + std::to_string(c.index) + " has the wrong offset";
                    }
                    if (!found && first_bad.empty())
                        first_bad = path + ": chord " + std::to_string(c.index) + " names no window";
                }
            }
        }
    }
    CHECK(rows > 0);
    CHECK_MESSAGE(first_bad.empty(), first_bad);
}

TEST_CASE("schedule: declined chords are the skipped fill ticks") {
    // On the corpus: the declined rows are exactly the skipped fills, and
    // each one's kept gems are kept_gems'.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int declined_rows = 0;
    for_each_corpus_schedule(cfg, [&](const std::string& path, const Song& song,
                                      const std::vector<const Activation*>& acts, const PathReplay& pr,
                                      const json& s) {
        std::set<int64_t> skipped;
        for (const Activation* a : acts)
            skipped.insert(a->skipped_fill_ticks.begin(), a->skipped_fill_ticks.end());
        std::set<int64_t> marked;
        for (const json& row : s["chords"]) {
            if (!row["declined"].get<bool>()) continue;
            ++declined_rows;
            marked.insert(row["tick"].get<int64_t>());
            const ReplayChord& c = pr.result.chords[row["index"].get<size_t>()];
            CHECK(row["kept_gems"].get<std::vector<size_t>>() ==
                  schedule::kept_gems(song.sequence[static_cast<size_t>(c.index)].chord, c.notes).places);
        }
        INFO(path);
        CHECK(marked == skipped);
    });
    CHECK(declined_rows > 0);
}

TEST_CASE("schedule: the window constants are written") {
    // The formula's own values are pinned in tests/test_note_window.cpp.
    const Song song = plain_song({0, 768});
    const ReplayResult r = replay_path(song, {});
    const json c = schedule::schedule_json(song, {}, r, core::default_rules())["constants"];
    CHECK(c["frame_ms"].get<double>() == 3.0);
    CHECK(c["judge_early_ms"].get<double>() == 3.0);
    CHECK_FALSE(c.contains("judge_late_ms"));  // one frame everywhere (D319 item 3)
    CHECK(c["press_lead_ms"].get<double>() == 2.0);
    CHECK(c["sp_end_lead_ms"].get<double>() == 15.0);
    CHECK(c["sp_out_lag_ms"].get<double>() == 15.0);
    CHECK(c["chord_order_gap_ms"].get<double>() == 8.0);
    CHECK(c["room_margin_ms"].get<double>() == 3.0);
    CHECK(c["window_front_ms"].get<double>() == 37.5);
    CHECK(c["window_back_ms"].get<double>() == 85.0);
}

TEST_CASE("schedule: sides come from the whole chart") {
    // Chords 20 ticks (52.08 ms) apart: the middle one's half-gaps both
    // clamp to the front, so its side is the floor, as Freak's chord 263
    // gets from neighbours 56.18 ms away. The sides are read off the whole
    // chord list the file writes, never a pruned one.
    {
        const Song song = plain_song({0, 20, 40});
        const ReplayResult r = replay_path(song, {});
        const json s = schedule::schedule_json(song, {}, r, core::default_rules());
        REQUIRE(s["chords"].size() == 3);
        CHECK(s["chords"][1]["side_ms"].get<double>() == kFloorSide);
    }
    // Chords 917 ticks (2,388 ms) and 2,999 ticks (7,810 ms) either side of
    // the middle one: both half-gaps clamp to the back, so the side is the cap.
    {
        const Song song = plain_song({0, 917, 917 + 2999});
        const ReplayResult r = replay_path(song, {});
        const json s = schedule::schedule_json(song, {}, r, core::default_rules());
        REQUIRE(s["chords"].size() == 3);
        CHECK(s["chords"][1]["side_ms"].get<double>() == kCapSide);
    }
    // A chord's own gems are not each other's neighbours (TM-51): a
    // two-gem chord between far neighbours still gets the cap.
    {
        Song song = plain_song({0, 917, 917 + 2999});
        song.sequence[1].chord.add_note(NoteColor::Kick);
        const ReplayResult r = replay_path(song, {});
        const json s = schedule::schedule_json(song, {}, r, core::default_rules());
        REQUIRE(s["chords"][1]["gems"].size() == 2);
        CHECK(s["chords"][1]["side_ms"].get<double>() == kCapSide);
    }
}

TEST_CASE("schedule: a missing --path refuses by name") {
    const std::optional<std::string> r = schedule::path_flag_refusal("");
    REQUIRE(r.has_value());
    CHECK(r->find("--path") != std::string::npos);
    CHECK(r->find('\n') == std::string::npos);  // main prints it as one "error:" line
    CHECK_FALSE(schedule::path_flag_refusal("dump.json").has_value());
}

TEST_CASE("schedule: a chart with no path refuses by name") {
    const json empty{{"paths", json::array()}};
    const std::optional<std::string> r = schedule::no_path_refusal(empty, "freak_dump.json");
    REQUIRE(r.has_value());
    CHECK(r->find("freak_dump.json") != std::string::npos);
    CHECK(r->find("no path") != std::string::npos);
    CHECK(r->find('\n') == std::string::npos);
    CHECK(schedule::no_path_refusal(json::object(), "x.json").has_value());
    const json one{{"paths", json::array({json::object()})}};
    CHECK_FALSE(schedule::no_path_refusal(one, "x.json").has_value());
}

TEST_CASE("schedule: nothing under src/ reads the schedule") {
    int files = 0;
    std::string first_hit;
    std::string model_h;
    sourcetree::for_each_source_file([&](const fs::path& path, const std::string& rel) {
        if (rel.rfind("src/", 0) != 0) return;
        ++files;
        std::ifstream in(path, std::ios::binary);
        const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        if (rel == "src/core/model.h") model_h = text;
        for (const char* name : {"replay_schedule.h", "kFrameMs", "kRoomMarginMs"})
            if (text.find(name) != std::string::npos && first_hit.empty())
                first_hit = rel + " names " + name;
    });
    CHECK(files > 0);
    CHECK_MESSAGE(first_hit.empty(), first_hit);

    // The screens keep Hydra's own two numbers (plan rule 11).
    CHECK(model_h.find("constexpr double kHitWindowMs = 85.0;") != std::string::npos);
    CHECK(model_h.find("constexpr double kSqueezeWindowMs = 500.0;") != std::string::npos);
}

// ---- TC2: the aims, rooms, fallbacks and refusals ---------------------------

namespace {

// A squeeze-out's geometry from the plan's table (design point 6): the two
// sides, the gap from the squeeze chord to the end, and the early ratio. The
// late ratio is one no squeeze-out reads, so reading the wrong side shows.
schedule::SqueezeGeometry sqout(double anchor_side, double squeeze_ms, double squeeze_side, double end_ms,
                                double early_ratio) {
    schedule::SqueezeGeometry g;
    g.kind = SqueezeKind::SqOut;
    g.anchor_chord = 1;
    g.anchor_side_ms = anchor_side;
    g.squeeze_chord = 2;
    g.squeeze_ms = squeeze_ms;
    g.squeeze_side_ms = squeeze_side;
    g.end_ms = end_ms;
    g.ratio = TransferScale{early_ratio, 2.0};
    return g;
}

// The tolerance TC2's acceptance in
// docs/superpowers/plans/2026-11-08-replay-press-schedule.md gives ("within
// 0.01"): the plan's figures are printed to two places.
bool within_cent(double got, double want) { return std::abs(got - want) < 0.01; }

// The presses of chord `chord` the schedule `s` writes under `rule`.
std::vector<json> presses_of(const json& s, int chord, const std::string& rule) {
    std::vector<json> out;
    for (const json& p : s["presses"])
        if (p["chord"].get<int>() == chord && p["rule"] == rule) out.push_back(p);
    return out;
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
    // Chord 387 at 60,545.26 (side 53.59), the end at 60,573.35, anchor chord
    // 263 (side 48.15), ratio 1.0. Worked by hand from the split rule with
    // one frame (3 ms): L = (48.15 - 53.59 + 28.09 + 15) / 2 + 3 = 21.825;
    // the end lands at 60,573.35 - (21.825 - 3) = 60,554.525; the late gem
    // goes at 60,554.525 + 15 - 60,545.26 = 24.265; both rooms are 26.325.
    const schedule::SqueezeGeometry g = sqout(48.15, 60545.26, 53.59, 60573.35, 1.0);
    const schedule::SqueezeSplit s = schedule::split_squeeze(g);
    CHECK(within_cent(s.anchor_aim_ms, -21.825));
    CHECK(within_cent(s.squeeze_aim_ms, 24.265));
    CHECK(within_cent(s.anchor_room_ms, 26.325));
    CHECK(within_cent(s.squeeze_room_ms, 26.325));
    CHECK(within_cent(s.planned_end_ms, 60554.525));
    CHECK(within_cent(s.aim_end_ms, 60554.525));
    // 60,545.26 + 53.59 - 15 - 3.
    CHECK(within_cent(schedule::sqout_fallback_after_ms(60545.26, 53.59), 60580.85));
    CHECK(within_cent(s.aim_end_ms + s.squeeze_room_ms, 60580.85));
    CHECK(s.feasible);
    // The cap's side on 263 (the pruned fixture's 85.72) gives another lead.
    CHECK_FALSE(within_cent(schedule::split_squeeze(sqout(85.72, 60545.26, 53.59, 60573.35, 1.0)).anchor_aim_ms, -21.825));

    // The whole file on the overfill shape: the lead goes on chord 2, the
    // chord the clamp anchors the end on, never on the activation's chord 1.
    const Overfill o = overfill();
    const ReplayResult r = replay_path(o.song, {});
    const json sch = schedule::schedule_json(o.song, {&o.act}, r, core::default_rules());
    INFO(sch["refusals"].dump());
    CHECK(sch["refusals"].empty());  // and no "not planned yet" line
    const std::vector<json> anchor = presses_of(sch, 2, "anchor");
    REQUIRE(anchor.size() == 1);
    // Both sides are the cap, so L = 26.04 / 2 + 15 / 2 + 3 = 23.52.
    CHECK(within_cent(anchor[0]["aim_ms"].get<double>(), -23.52));
    REQUIRE(presses_of(sch, 1, "plain").size() == 1);
    CHECK(presses_of(sch, 1, "plain")[0]["aim_ms"].get<double>() == -2.0);
    // The read after chord 2 sees the clamped end, 8,026.04 ms, planned
    // 20.52 ms earlier (the lead less one frame).
    const json& reads = sch["chords"][2]["reads"];
    REQUIRE(reads.size() == 1);
    CHECK(within_cent(reads[0]["stored_end_ms"].get<double>(), 8026.04));
    CHECK(within_cent(reads[0]["planned_end_ms"].get<double>(), 8005.52));
    // The squeeze-out chord: the late gem (place 0) 20.52 ms late, assuming
    // the end at 8,005.52, retimed from the read after chord 2.
    const std::vector<json> late = presses_of(sch, 4, "squeeze_out_late");
    REQUIRE(late.size() == 1);
    CHECK(late[0]["gems"].get<std::vector<size_t>>() == std::vector<size_t>{0});
    CHECK(within_cent(late[0]["aim_ms"].get<double>(), 20.52));
    CHECK(within_cent(late[0]["aim_end_ms"].get<double>(), 8005.52));
    CHECK(late[0]["end_read_after"].get<int>() == 2);
    CHECK(late[0]["timed_from_end"].get<size_t>() == 0);
    const json& q = sch["activations"][0]["squeezes"][0];
    CHECK(q["feasible"].get<bool>());
    CHECK(within_cent(q["anchor_room_ms"].get<double>(), q["squeeze_room_ms"].get<double>()));
}

TEST_CASE("schedule: Freak 1144, Rescued 561 and 931, Ghost 756") {
    struct Row {
        const char* name;
        double anchor_side, squeeze_side, gap, ratio, anchor_aim, squeeze_aim, room;
    };
    const Row rows[] = {
        // With one frame each lead is 3 ms shorter and each room 3 ms
        // wider than the two-frame table; the late gem's aim is the same.
        {"Freak 1144", 73.76, 53.59, 84.27, 1.0, -62.72, 39.55, 11.04},
        {"Rescued 561", 75.93, 61.47, 99.04, 1.0284, -66.35, 48.89, 9.58},
        {"Rescued 931", 76.49, 61.94, 100.00, 1.0045, -67.63, 50.08, 8.86},
        {"Ghost 756", 78.75, 66.34, 109.25, 1.0133, -70.88, 55.47, 7.87},
    };
    for (const Row& row : rows) {
        INFO(row.name);
        const double end = 100000.0;
        const schedule::SqueezeSplit s =
            schedule::split_squeeze(sqout(row.anchor_side, end - row.gap, row.squeeze_side, end, row.ratio));
        CHECK(within_cent(s.anchor_aim_ms, row.anchor_aim));
        CHECK(within_cent(s.squeeze_aim_ms, row.squeeze_aim));
        CHECK(within_cent(s.anchor_room_ms, row.room));
        CHECK(within_cent(s.squeeze_room_ms, row.room));
        CHECK(within_cent(s.anchor_room_ms, s.squeeze_room_ms));
        CHECK(s.anchor_room_ms > 0.0);
        CHECK(s.feasible);
    }
}

TEST_CASE("schedule: a squeeze that needs no timing is pressed plain") {
    // Freak 717's shape: a squeeze-in whose chord already sits 84.27 ms
    // inside its end.
    const Song song = plain_song({0, 768, 1536, 2304, 3072, 3840});
    Activation act;
    act.timecode = song.timecode(768);
    act.sp_end_steps = {{768, 3500, SpEndKind::Activation}, {3072, 3900, SpEndKind::SqIn}};
    SPSqueeze in{};
    in.kind = SqueezeKind::SqIn;
    in.offset_ms = -84.27;
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
    CHECK(p[0]["aim_ms"].get<double>() == -2.0);
    CHECK_FALSE(p[0].contains("fallback"));
    CHECK(presses_of(s, 1, "plain").size() == 1);  // no lead on the activation
}

TEST_CASE("schedule: a squeeze the game cannot make is not feasible") {
    // A 200 ms gap against two 53.59 ms sides: both rooms negative, and that
    // is a feasibility, not a refusal.
    const schedule::SqueezeGeometry too_wide = sqout(53.59, 1000.0, 53.59, 1200.0, 1.0);
    const schedule::SqueezeSplit s = schedule::split_squeeze(too_wide);
    CHECK_FALSE(s.feasible);
    CHECK(s.anchor_room_ms < 0.0);
    CHECK(s.squeeze_room_ms < 0.0);
    std::vector<std::string> refusals;
    CHECK(schedule::anchor_lead_ms({too_wide}, refusals).has_value());
    CHECK(refusals.empty());
    // kRoomMarginMs: a 25 ms gap and 26 ms sides leave 3.0 ms of room each
    // (L = (25 + 15) / 2 + 3 = 23; feasible); 25.9 ms sides leave 2.9 (not).
    const schedule::SqueezeSplit three = schedule::split_squeeze(sqout(26.0, 1000.0, 26.0, 1025.0, 1.0));
    CHECK(three.anchor_room_ms == doctest::Approx(3.0));
    CHECK(three.squeeze_room_ms == doctest::Approx(3.0));
    CHECK(three.feasible);
    const schedule::SqueezeSplit short_room = schedule::split_squeeze(sqout(25.9, 1000.0, 25.9, 1025.0, 1.0));
    CHECK(short_room.anchor_room_ms == doctest::Approx(2.9));
    CHECK_FALSE(short_room.feasible);
}

TEST_CASE("schedule: the fallback block") {
    // Freak 1144 (141,571.98 ms, side 53.59): past 141,607.57 the late gem
    // joins the inside group (141,571.98 + 53.59 - 15 - 3).
    CHECK(within_cent(schedule::sqout_fallback_after_ms(141571.98, 53.59), 141607.57));
    // A squeeze-in pressed before a moved end: chord.ms - side + 15 + 3.
    CHECK(schedule::sqin_fallback_before_ms(1000.0, 50.0) == doctest::Approx(968.0));

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
        const json& row = s["chords"][4];
        CHECK(fb["if_end_after_ms"].get<double>() ==
              doctest::Approx(schedule::sqout_fallback_after_ms(row["ms"].get<double>(), row["side_ms"].get<double>())));
        // A plain press carries no block.
        REQUIRE(presses_of(s, 0, "plain").size() == 1);
        CHECK_FALSE(presses_of(s, 0, "plain")[0].contains("fallback"));
    }

    // A hand-built timed squeeze-in: chord 4 at tick 3010 sits 26.04 ms past
    // the activation's end at tick 3000; question 3 (a) presses it plain.
    {
        const Song song = plain_song({0, 768, 1536, 2304, 3010, 3840});
        Activation act;
        act.timecode = song.timecode(768);
        act.sp_end_steps = {{768, 3000, SpEndKind::Activation}, {3010, 3600, SpEndKind::SqIn}};
        SPSqueeze in{};
        in.kind = SqueezeKind::SqIn;
        in.offset_ms = 26.04;
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
        CHECK(fb["aim_ms"].get<double>() == -2.0);
        const json& row = s["chords"][4];
        CHECK(fb["if_end_before_ms"].get<double>() ==
              doctest::Approx(schedule::sqin_fallback_before_ms(row["ms"].get<double>(), row["side_ms"].get<double>())));
        // The activation leads it late.
        REQUIRE(presses_of(s, 1, "anchor").size() == 1);
        CHECK(presses_of(s, 1, "anchor")[0]["aim_ms"].get<double>() > 0.0);
    }
}

TEST_CASE("schedule: last_send_ms is the side less one frame") {
    CHECK(schedule::last_send_ms(1000.0, 50.0) == doctest::Approx(1047.0));
    const Overfill o = overfill();
    const ReplayResult r = replay_path(o.song, {});
    const json s = schedule::schedule_json(o.song, {&o.act}, r, core::default_rules());
    REQUIRE(!s["presses"].empty());
    for (const json& p : s["presses"]) {
        const json& row = s["chords"][p["chord"].get<size_t>()];
        CHECK(p["last_send_ms"].get<double>() ==
              doctest::Approx(schedule::last_send_ms(row["ms"].get<double>(), row["side_ms"].get<double>())));
    }
}

TEST_CASE("schedule: an activation that needs both directions is refused naming both chords") {
    schedule::SqueezeGeometry out = sqout(60.0, 1000.0, 60.0, 1030.0, 1.0);
    out.anchor_chord = 5;
    out.squeeze_chord = 17;
    schedule::SqueezeGeometry in = out;
    in.kind = SqueezeKind::SqIn;
    in.squeeze_chord = 23;
    in.squeeze_ms = 1040.0;
    std::vector<std::string> refusals;
    CHECK_FALSE(schedule::anchor_lead_ms({out, in}, refusals).has_value());
    REQUIRE(refusals.size() == 1);
    CHECK(refusals[0].find("anchor chord 5") != std::string::npos);
    CHECK(refusals[0].find("early for chord 17") != std::string::npos);
    CHECK(refusals[0].find("late for chord 23") != std::string::npos);
}

TEST_CASE("schedule: a lead across a steady span is refused naming the span") {
    // Freak 387's split leads the anchor 24.82 ms early; a tempo change 10 ms
    // before the anchor breaks the ratio.
    schedule::SqueezeGeometry g = sqout(48.15, 60545.26, 53.59, 60573.35, 1.0);
    g.steady.before_anchor_ms = 10.0;
    std::vector<std::string> refusals;
    CHECK_FALSE(schedule::anchor_lead_ms({g}, refusals).has_value());
    REQUIRE(refusals.size() == 1);
    CHECK(refusals[0].find("before_anchor_ms") != std::string::npos);
    CHECK(refusals[0].find("tempo or time signature change") != std::string::npos);
    // One the lead stays inside passes.
    g.steady.before_anchor_ms = 30.0;
    refusals.clear();
    CHECK(schedule::anchor_lead_ms({g}, refusals).has_value());
    CHECK(refusals.empty());
}

TEST_CASE("schedule: a press that reaches a neighbour on its pad is refused naming both chords") {
    // The Edge Walk Pair shape: two kicks 100 ms apart, chords 10 and 11,
    // each side 76.57 ms (TM-50's run).
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
    // Chord 10 pressed on time, at 998: chord 11 pressed 60 ms early lands
    // inside chord 10's window after 10 is hit, so it passes.
    pair[0].sends = {{kick_pad, 998.0}};
    CHECK_FALSE(schedule::far_press_refusal(pair, 1, 1040.0, kick).has_value());
    // Chord 10 not pressed: chord 11's press would take it. Refused.
    pair[0].sends.clear();
    const std::optional<std::string> r = schedule::far_press_refusal(pair, 1, 1040.0, kick);
    REQUIRE(r.has_value());
    CHECK(r->find("chord 11") != std::string::npos);
    CHECK(r->find("chord 10") != std::string::npos);
    CHECK(r->find("no press") != std::string::npos);
}

TEST_CASE("schedule: a late gem that pays less than an inside gem is refused") {
    std::vector<std::string> refusals;
    CHECK_FALSE(schedule::squeeze_out_inside_groups(7, {1, 2}, {0, 1}, {0}, refusals).has_value());
    REQUIRE(refusals.size() == 1);
    CHECK(refusals[0].find("chord 7") != std::string::npos);
    CHECK(refusals[0].find("late gem pays x1 and an inside gem x2") != std::string::npos);
    refusals.clear();
    const auto ok = schedule::squeeze_out_inside_groups(7, {2, 2, 1}, {0, 1, 2}, {0}, refusals);
    REQUIRE(ok.has_value());
    CHECK(refusals.empty());
    CHECK(*ok == std::vector<std::vector<size_t>>{{2}, {1}});
    // Only the gems the player sends go inside: a declined chord that keeps
    // places 0 and 2 presses place 2 alone inside.
    const auto kept = schedule::squeeze_out_inside_groups(7, {2, 2, 1}, {0, 2}, {0}, refusals);
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
    // multiplier presses Green (its activation note) first, then the rest
    // 8 ms after.
    Overfill o = overfill();
    o.song.sequence[1].chord.add_note(NoteColor::Green);
    o.song.sequence[1].chord.add_note(NoteColor::Kick);
    const ReplayResult r = replay_path(o.song, {});
    const json s = schedule::schedule_json(o.song, {&o.act}, r, core::default_rules());
    const std::vector<json> act = presses_of(s, 1, "plain");
    REQUIRE(act.size() == 2);
    const json& gems = s["chords"][1]["gems"];
    CHECK(gems[act[0]["gems"][0].get<size_t>()]["color"] == "Green");
    CHECK(act[0]["after_ms"].is_null());
    CHECK(act[1]["after_ms"].get<double>() == 8.0);
    CHECK(act[1]["aim_ms"].get<double>() == doctest::Approx(-2.0 + 8.0));
    // The two-gem squeeze-out: the inside gem 15 ms before the assumed end
    // (8,005.52 - 15 - 8,000 = -9.48), the late gem last.
    const std::vector<json> inside = presses_of(s, 4, "squeeze_out_inside");
    REQUIRE(inside.size() == 1);
    CHECK(within_cent(inside[0]["aim_ms"].get<double>(), -9.48));
    size_t inside_at = 0, late_at = 0;
    for (size_t k = 0; k < s["presses"].size(); ++k) {
        const json& p = s["presses"][k];
        if (p["chord"].get<int>() != 4) continue;
        (p["rule"] == "squeeze_out_inside" ? inside_at : late_at) = k;
    }
    CHECK(inside_at < late_at);
}

TEST_CASE("schedule: the planned end counts one frame") {
    // A plain activation: the nominal end plus one frame less the 2 ms lead.
    const Song song = plain_song({0, 768, 1536});
    Activation act;
    act.timecode = song.timecode(768);
    act.sp_end_steps = {{768, 3000, SpEndKind::Activation}};
    act.transfer_post = TransferScale{1.0, 1.0};
    const ReplayResult r = replay_path(song, {});
    const json s = schedule::schedule_json(song, {&act}, r, core::default_rules());
    const json& reads = s["chords"][1]["reads"];
    REQUIRE(reads.size() == 1);
    CHECK(within_cent(reads[0]["stored_end_ms"].get<double>(), 7812.5));
    CHECK(within_cent(reads[0]["planned_end_ms"].get<double>(), 7813.5));
    // A led anchor: the stored end moved by ratio x (lead - one frame);
    // Freak 263's 21.825 ms lead at ratio 1.0 plans 60,554.525.
    CHECK(within_cent(schedule::split_squeeze(sqout(48.15, 60545.26, 53.59, 60573.35, 1.0)).planned_end_ms, 60554.525));
}

TEST_CASE("schedule: the equal-room split of a squeeze-in is the mirror of a squeeze-out's") {
    // No chart in the corpus has a timed squeeze-in, so this is hand-built: the
    // anchor (side 60) goes late for a chord (side 50) 40 ms past the end at
    // 1,000 ms, ratio 1.0 on the late side (the early side, 2.0, is one a
    // squeeze-in never reads). Worked by hand from the split rule in
    // replay_schedule.cpp, one frame (3 ms) on each press:
    // L = (60 - 50 + 40 + 15 - 3 - 3) / 2 = 29.5; the end lands at
    // 1,000 + 1 x (29.5 + 3) = 1,032.5; the chord is pressed at
    // 1,032.5 - 15 - 1,040 = -22.5; both rooms are 27.5.
    schedule::SqueezeGeometry g;
    g.kind = SqueezeKind::SqIn;
    g.anchor_chord = 1;
    g.anchor_side_ms = 60.0;
    g.squeeze_chord = 2;
    g.squeeze_ms = 1040.0;
    g.squeeze_side_ms = 50.0;
    g.end_ms = 1000.0;
    g.ratio = TransferScale{2.0, 1.0};
    const schedule::SqueezeSplit s = schedule::split_squeeze(g);
    CHECK(s.anchor_aim_ms == doctest::Approx(29.5));
    CHECK(s.squeeze_aim_ms == doctest::Approx(-22.5));
    CHECK(s.anchor_room_ms == doctest::Approx(27.5));
    CHECK(s.squeeze_room_ms == doctest::Approx(27.5));
    CHECK(s.planned_end_ms == doctest::Approx(1032.5));
    CHECK(s.aim_end_ms == doctest::Approx(1032.5));
    CHECK(s.feasible);
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
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int with_length = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.reading());
        if (song.is_empty()) continue;
        INFO(path);
        const store::ChartTimingMeta meta = app::chart_timing_meta(std::nullopt, path);
        const std::optional<double> want = app::song_length_ms(meta, song);
        const json got = schedule::song_length_json(meta, song);
        if (!want) {
            CHECK(got.is_null());
            continue;
        }
        ++with_length;
        REQUIRE(got.is_number());
        CHECK(got.get<double>() == *want);
    }
    CHECK(with_length > 0);

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

    // On every corpus path: each activation's field is its stored
    // kSpActivationBars-th arrival, in ticks and through the tempo map.
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
            const int64_t want = a.bank_rise_ticks.at(static_cast<size_t>(kSpActivationBars) - 1);
            const json& row = sch["activations"][k];
            CHECK(row["sp_ready_tick"].get<int64_t>() == want);
            CHECK(row["sp_ready_ms"].get<double>() == s.timing().timecode(want).ms());
            CHECK(want <= a.timecode.ticks());
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

TEST_CASE("schedule: a whole_chord chord with no inside group has an empty inside list and an empty join_gems") {
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
        const json& fb = late[0]["fallback"];
        CHECK(fb["action"] == "join_group");
        CHECK(fb["join_gems"].is_array());
        CHECK(fb["join_gems"].empty());
    }
    CHECK(presses_of(one_gem, 4, "squeeze_out_late")[0]["gems"].get<std::vector<size_t>>() ==
          std::vector<size_t>{0});
}

TEST_CASE("schedule: first_note writes the same schedule as before") {
    // The overfill shape's literals, pinned before F1 in "the equal-room
    // split on Freak's clamped squeeze" and "press groups", under the rules
    // passed by name.
    const Overfill o = overfill();
    const ReplayResult r = replay_path(o.song, {});
    const json s = schedule::schedule_json(o.song, {&o.act}, r, rules_with(core::SqOutRule::FirstNote));
    const std::vector<json> late = presses_of(s, 4, "squeeze_out_late");
    const std::vector<json> inside = presses_of(s, 4, "squeeze_out_inside");
    REQUIRE(late.size() == 1);
    REQUIRE(inside.size() == 1);
    CHECK(late[0]["gems"].get<std::vector<size_t>>() == std::vector<size_t>{0});
    CHECK(within_cent(late[0]["aim_ms"].get<double>(), 20.52));
    CHECK(within_cent(late[0]["aim_end_ms"].get<double>(), 8005.52));
    CHECK(within_cent(inside[0]["aim_ms"].get<double>(), -9.48));
    CHECK(late[0]["fallback"]["join_gems"] == inside[0]["gems"]);

    // On the corpus under the app's default rules (first_note): every late
    // press holds place 0 alone, and no inside press holds place 0, as the
    // schedule did before the rules reached it.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    REQUIRE(cfg.rules.sqout_rule == core::SqOutRule::FirstNote);
    int late_rows = 0;
    for_each_corpus_schedule(cfg, [&](const std::string& path, const Song&, const std::vector<const Activation*>&,
                                      const PathReplay&, const json& sch) {
        for (const json& press : sch["presses"]) {
            INFO(path << " chord " << press["chord"].get<int>());
            const std::vector<size_t> gems = press["gems"].get<std::vector<size_t>>();
            if (press["rule"] == "squeeze_out_late") {
                ++late_rows;
                CHECK(gems == std::vector<size_t>{0});
            } else if (press["rule"] == "squeeze_out_inside") {
                CHECK(std::find(gems.begin(), gems.end(), size_t{0}) == gems.end());
            }
        }
    });
    // The corpus may hold no timed squeeze-out; the overfill literals above
    // then carry the pin alone.
    WARN(late_rows > 0);
}

TEST_CASE("schedule: hydra_replay_build is written") {
    const Song song = plain_song({0, 768});
    const json s = schedule::schedule_json(song, {}, replay_path(song, {}), core::default_rules());
    REQUIRE(s.contains("hydra_replay_build"));
    const std::string build = s["hydra_replay_build"].get<std::string>();
    CHECK_FALSE(build.empty());
    CHECK(build == std::string(schedule::replay_build()));
    // __DATE__ " " __TIME__: "Mmm dd yyyy hh:mm:ss".
    CHECK(build.size() == 20);
}

// ---- F2: one frame, TM-50, flagged skips, one decline rule -----------------
// docs/superpowers/plans/2026-11-08-player-audit-fixes.md, task F2.

namespace {

// The text of a file in the source tree, by its path from the repo root.
std::string source_text(const std::string& rel) {
    std::ifstream in(sourcetree::root() / fs::u8path(rel), std::ios::binary);
    REQUIRE_MESSAGE(in.good(), "cannot read " << rel);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

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

TEST_CASE("schedule: one frame everywhere") {
    // A 50 ms side at 1,000 ms: every rule's latest judged time is 1,050,
    // the side's edge, reached by a send one frame (3 ms) earlier.
    CHECK(schedule::last_send_ms(1000.0, 50.0) == doctest::Approx(1047.0));
    // A press aimed 47 ms late is judged at 50: no room left; 46 leaves 1.
    CHECK(schedule::press_room_ms(47.0, 50.0) == doctest::Approx(0.0));
    CHECK(schedule::press_room_ms(46.0, 50.0) == doctest::Approx(1.0));
    // An early aim is measured from the aim itself: -50 leaves none.
    CHECK(schedule::press_room_ms(-50.0, 50.0) == doctest::Approx(0.0));
    // The far-press check: sent at 1,047 it is judged on the edge, inside
    // (TM-27: equal is inside); half a ms later it leaves its window.
    const schedule::Pad red{NoteColor::Red, false};
    std::vector<schedule::PadChord> one(1);
    one[0].index = 0;
    one[0].ms = 1000.0;
    one[0].side_ms = 50.0;
    one[0].chart_pads = {red};
    CHECK_FALSE(schedule::far_press_refusal(one, 0, 1047.0, {red}).has_value());
    const std::optional<std::string> out = schedule::far_press_refusal(one, 0, 1047.5, {red});
    REQUIRE(out.has_value());
    CHECK(out->find("leaves its own window") != std::string::npos);
    // The squeeze-out fallback: the late press goes 15 ms (sp_out_lag_ms)
    // after the read end, so past 1,032 it would be sent after 1,047.
    CHECK(schedule::sqout_fallback_after_ms(1000.0, 50.0) == doctest::Approx(1032.0));
    // The squeeze-in fallback reads the same one frame on the early edge.
    CHECK(schedule::sqin_fallback_before_ms(1000.0, 50.0) == doctest::Approx(968.0));

    // The second frame constant is gone from the code and the file.
    for (const char* rel : {"tools/replay_schedule.cpp", "tools/replay_schedule.h"}) {
        INFO(rel);
        const std::string text = source_text(rel);
        CHECK(text.find("kJudgeLateMs") == std::string::npos);
        CHECK(text.find("judge_late") == std::string::npos);
    }
}

TEST_CASE("schedule: far press follows TM-50") {
    // Two snares 100 ms apart, chords 20 and 21, each side 76.57 ms.
    const schedule::Pad snare{NoteColor::Red, false};
    std::vector<schedule::PadChord> two = pad_row({1000.0, 1100.0}, snare, 20);

    // Chord 20 sent 20 ms late (1,020), chord 21 sent 50 ms early (1,050),
    // inside chord 20's window: 20 is hit first, so neither is refused.
    two[0].sends = {{snare, 1020.0}};
    two[1].sends = {{snare, 1050.0}};
    CHECK_FALSE(schedule::far_press_refusal(two, 0, 1020.0, {snare}).has_value());
    CHECK_FALSE(schedule::far_press_refusal(two, 1, 1050.0, {snare}).has_value());

    // The same with chord 20's press sent after chord 21's: 21's press lands
    // while 20 is unhit, and the game gives it to 20. Refused, naming both.
    two[0].sends = {{snare, 1060.0}};
    const std::optional<std::string> later = schedule::far_press_refusal(two, 1, 1050.0, {snare});
    REQUIRE(later.has_value());
    CHECK(later->find("chord 21") != std::string::npos);
    CHECK(later->find("chord 20") != std::string::npos);
    CHECK(later->find("still unhit") != std::string::npos);

    // Sent at the same clock, 20 is still unhit when 21's press lands.
    two[0].sends = {{snare, 1050.0}};
    CHECK(schedule::far_press_refusal(two, 1, 1050.0, {snare}).has_value());

    // A press inside a later same-pad note's window when that note gets no
    // press: chord 20 sent at 1,060 is judged at 1,063, inside chord 21's
    // window (from 1,023.43). No refusal: the later-note branch is gone.
    two[1].sends.clear();
    CHECK_FALSE(schedule::far_press_refusal(two, 0, 1060.0, {snare}).has_value());
}

TEST_CASE("schedule: every skip writes a refusal and a flag") {
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

    // A skipped fill with no chord under it: the two inputs are not of one
    // chart. The line is written; there is no chord row to flag.
    {
        Overfill o = overfill();
        o.act.skipped_fill_ticks = {3000};
        const json s = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}), core::default_rules());
        INFO(s["refusals"].dump());
        CHECK(refusal_says(s, {"no chord at skipped fill"}));
    }

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
    // The overfill shape, with the squeezed-out chord 4 a skipped fill.
    // Places follow Chord::note_list(true): plain gems in lane order.

    // Red and Kick: the activation note is Red (place 1); the late gem under
    // first_note is place 0, the Kick, which the fill keeps. The kick goes
    // late, and Red is pressed nowhere.
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
        CHECK(row["kept_gems"].get<std::vector<size_t>>() == std::vector<size_t>{0});
        const std::vector<json> late = presses_of(s, 4, "squeeze_out_late");
        REQUIRE(late.size() == 1);
        CHECK(late[0]["gems"].get<std::vector<size_t>>() == sqout_late_notes(chord4, core::SqOutRule::FirstNote));
        CHECK(late[0]["gems"].get<std::vector<size_t>>() == std::vector<size_t>{0});
        CHECK(presses_of(s, 4, "squeeze_out_inside").empty());
        CHECK(row["gems"][1]["color"] == color_str(chord4.activation_note().colortype));
    }

    // Red, Yellow and Kick: the activation note is Yellow (place 2). The
    // fill keeps Kick and Red; Kick goes late, Red is the inside group.
    {
        Overfill o = overfill();
        o.song.sequence[4].chord.add_note(NoteColor::Yellow);
        o.act.skipped_fill_ticks = {3072};
        const json s = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}),
                                               rules_with(core::SqOutRule::FirstNote));
        INFO(s["refusals"].dump());
        CHECK(s["refusals"].empty());
        CHECK(s["chords"][4]["kept_gems"].get<std::vector<size_t>>() == std::vector<size_t>{0, 1});
        const std::vector<json> late = presses_of(s, 4, "squeeze_out_late");
        const std::vector<json> inside = presses_of(s, 4, "squeeze_out_inside");
        REQUIRE(late.size() == 1);
        REQUIRE(inside.size() == 1);
        CHECK(late[0]["gems"].get<std::vector<size_t>>() == std::vector<size_t>{0});
        CHECK(inside[0]["gems"].get<std::vector<size_t>>() == std::vector<size_t>{1});
        CHECK(late[0]["fallback"]["join_gems"] == inside[0]["gems"]);
    }

    // Red alone: the late gem, place 0, is the activation note the fill
    // leaves out. Refused naming the chord, and not pressed.
    {
        Overfill o = overfill();
        o.song.sequence[4].chord = Chord{};
        o.song.sequence[4].chord.add_note(NoteColor::Red);
        o.act.skipped_fill_ticks = {3072};
        const json s = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}),
                                               rules_with(core::SqOutRule::FirstNote));
        CHECK(s["chords"][4]["refused"].get<bool>());
        CHECK_FALSE(has_press(s, 4));
        CHECK(refusal_says(s, {"chord 4", "skipped fill", "activation note"}));
    }

    // Red and Kick under whole_chord: every gem is late, Red among them.
    {
        Overfill o = overfill();
        o.act.skipped_fill_ticks = {3072};
        const json s = schedule::schedule_json(o.song, {&o.act}, replay_path(o.song, {}),
                                               rules_with(core::SqOutRule::WholeChord));
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

    // The modes, their names and the flag are gone. hydra_replay's flag
    // loop sends any other flag to unknown_option_line, so --decline now
    // exits 2 with "Unknown option: --decline".
    const std::string header = source_text("tools/replay_schedule.h");
    const std::string code = source_text("tools/replay_schedule.cpp");
    const std::string replay = source_text("tools/replay.cpp");
    for (const std::string* text : {&header, &code, &replay}) {
        CHECK(text->find("DeclineMode") == std::string::npos);
        CHECK(text->find("decline_mode") == std::string::npos);
        CHECK(text->find("kick_only") == std::string::npos);
        CHECK(text->find("all_but_top_pad") == std::string::npos);
    }
    CHECK(replay.find("\"--decline\"") == std::string::npos);
    CHECK(replay.find("hydra::unknown_option_line(k)") != std::string::npos);
    // The schedule no longer writes a mode.
    const Song song = plain_song({0, 768});
    CHECK_FALSE(schedule::schedule_json(song, {}, replay_path(song, {}), core::default_rules()).contains("decline"));
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
        CHECK(act[0]["after_ms"].is_null());
    }

    // On every corpus path: each activation chord's first press holds its
    // activation note alone.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int seen = 0;
    for_each_corpus_schedule(cfg, [&](const std::string& path, const Song& song,
                                      const std::vector<const Activation*>&, const PathReplay& pr,
                                      const json& s) {
        for (const json& a : s["activations"]) {
            if (a["chord"].is_null()) continue;
            const int ci = a["chord"].get<int>();
            const json* first = nullptr;
            for (const json& press : s["presses"])
                if (press["chord"].get<int>() == ci) {
                    first = &press;
                    break;
                }
            if (!first || (first->at("rule") != "plain" && first->at("rule") != "anchor")) continue;
            INFO(path << " chord " << ci);
            ++seen;
            const ReplayChord& c = pr.result.chords[static_cast<size_t>(ci)];
            const NoteColor top = song.sequence[static_cast<size_t>(ci)].chord.activation_note().colortype;
            const std::vector<size_t> gems = first->at("gems").get<std::vector<size_t>>();
            REQUIRE(gems.size() == 1);
            CHECK(c.notes[gems[0]].color == top);
        }
    });
    CHECK(seen > 0);
}

// ---- T2: each read at its own step's scale, and the two ends of a press ----
// docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md, task T2.
// The literals are the plan's, read from Pitch Dark's chart and the run of
// 2026-11-12.

namespace {

// Chord 243's own planned end (the prep schedule's planned_end_ms) and its
// conversion, pinned from one run of this test.
constexpr double kPitchDark243OwnEndMs = 100949.4546;
constexpr double kPitchDark243Conversion = 1.0;

// The tolerance T2's acceptance in
// docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md gives a
// conversion ("within 0.00001"): the plan prints it to five places.
bool within_conversion(double got, double want) { return std::abs(got - want) < 0.00001; }

// Pitch Dark (testdata/input/test_pitch_dark_ratio, a copy of the library
// chart) under the app's default settings: path 0's activations and its
// schedule, built as the corpus sweeps build theirs.
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
    const std::vector<const Path*> paths = rec.all_paths();
    if (paths.empty()) throw std::runtime_error("Pitch Dark has no path");
    for (const Activation& a : paths.front()->walk_activations()) pd.acts.push_back(&a);
    const PathReplay pr = replay_stored_path(*pd.song, *paths.front(), cfg.rules);
    pd.sch = schedule::schedule_json(*pd.song, pd.acts, pr.result, cfg.rules);
    return pd;
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

TEST_CASE("schedule: Pitch Dark prices each read at its own step's scale") {
    const PitchDark pd = pitch_dark();
    const json& sch = pd.sch;
    INFO(sch["refusals"].dump());

    // Row P15: path 0 is the path the run played. Activation 1 has the four
    // steps of row P3; if this fails, the rest of the case does not apply.
    REQUIRE(pd.acts.size() == 3);
    const std::vector<SpEndStep>& steps = pd.acts[1]->sp_end_steps;
    REQUIRE(steps.size() == 4);
    CHECK(steps[0].tick == 178560);
    CHECK(steps[1].tick == 182280);
    CHECK(steps[1].end_tick == 198570);
    CHECK(steps[2].tick == 194080);
    CHECK(steps[2].end_tick == 202440);
    CHECK(steps[3].tick == 202480);
    CHECK(steps[3].end_tick == 206280);
    REQUIRE(steps[1].tick == 182280);
    REQUIRE(steps[3].end_tick == 206280);

    const SongTiming& timing = pd.song->timing();
    const double plain_judged = sch["constants"]["judge_early_ms"].get<double>() -
                                sch["constants"]["press_lead_ms"].get<double>();

    // The read after chord 466 sees the clamp's end (step 1), in the 5/4
    // measure, priced at that step's scale, not the squeeze's.
    const json& r466 = read_after(sch, 466);
    CHECK(r466["step"].get<size_t>() == 1);
    CHECK(within_cent(r466["planned_end_ms"].get<double>(), 158262.53));
    CHECK_FALSE(within_cent(r466["planned_end_ms"].get<double>(), 158254.77));

    // The reads after the two activation chords, each at its own step 0.
    struct ActChord {
        int chord;
        size_t act;
    };
    for (const ActChord ac : {ActChord{437, 1}, ActChord{144, 0}}) {
        INFO("chord " << ac.chord);
        const json& r = read_after(sch, ac.chord);
        CHECK(r["step"].get<size_t>() == 0);
        const std::optional<TransferScale> own = step_transfer_scale(*pd.acts[ac.act], 0, timing);
        REQUIRE(own.has_value());
        CHECK(r["ratio"]["early"].get<double>() == own->early);
        CHECK(r["ratio"]["late"].get<double>() == own->late);
        CHECK(r["planned_end_ms"].get<double>() - r["stored_end_ms"].get<double>() ==
              doctest::Approx(own->late * plain_judged));
    }
    CHECK_FALSE(within_cent(read_after(sch, 437)["planned_end_ms"].get<double>(), 154250.379));

    // The anchor and squeeze aims of the two timed squeezes do not change.
    REQUIRE(presses_of(sch, 466, "anchor").size() == 1);
    CHECK(within_cent(presses_of(sch, 466, "anchor")[0]["aim_ms"].get<double>(), 24.124));
    REQUIRE(presses_of(sch, 166, "anchor").size() == 1);
    CHECK(within_cent(presses_of(sch, 166, "anchor")[0]["aim_ms"].get<double>(), 64.571));

    // Chord 644, the squeeze-in: its own end is step 2's, its read end step
    // 1's, and the conversion is 1.1067 over 1.3926 on both sides.
    const std::vector<json> p644 = presses_of(sch, 644, "squeeze_in");
    REQUIRE(p644.size() == 1);
    const json& s644 = p644[0];
    CHECK(within_cent(s644["aim_ms"].get<double>(), -18.316));
    CHECK(within_cent(s644["own_end_ms"].get<double>(), 161386.46));
    CHECK(within_conversion(s644["conversion"]["early"].get<double>(), 0.79467));
    CHECK(within_conversion(s644["conversion"]["late"].get<double>(), 0.79467));
    CHECK(s644["aim_end_ms"].get<double>() == r466["planned_end_ms"].get<double>());
    CHECK(within_cent(s644["own_stored_end_ms"].get<double>(), 161356.443));
    CHECK(within_cent(s644["read_stored_end_ms"].get<double>(), 158224.757));
    CHECK(s644["end_read_after"].get<int>() == 466);
    // Its plain fallback's threshold is in read terms: fed back through the
    // conversion it gives the own-terms threshold.
    const json& row644 = sch["chords"][644];
    const double read_threshold = s644["fallback"]["if_end_before_ms"].get<double>();
    CHECK(own_end_at_read(s644, read_threshold) ==
          doctest::Approx(schedule::sqin_fallback_before_ms(row644["ms"].get<double>(),
                                                            row644["side_ms"].get<double>())));

    // Chord 676, on the final end: aimed from chord 466's planned read, at
    // a final end the late anchor moves later, with the final end's
    // conversion against the read's step.
    const std::vector<json> p676 = presses_of(sch, 676, "sp_end");
    REQUIRE_FALSE(p676.empty());
    const json& act1 = sch["activations"][1];
    for (const json& p : p676) {
        CHECK(p["aim_end_ms"].get<double>() == r466["planned_end_ms"].get<double>());
        CHECK(within_cent(p["own_stored_end_ms"].get<double>(), 164341.3));
        CHECK(p["own_end_ms"].get<double>() > p["own_stored_end_ms"].get<double>());
        for (const char* side : {"early", "late"})
            CHECK(p["conversion"][side].get<double>() * r466["ratio"][side].get<double>() ==
                  doctest::Approx(act1["end_ratio"][side].get<double>()));
    }

    // Chord 243, activation 0's squeeze-in.
    const std::vector<json> p243 = presses_of(sch, 243, "squeeze_in");
    REQUIRE(p243.size() == 1);
    CHECK(within_cent(p243[0]["aim_ms"].get<double>(), -40.919));
    CHECK(p243[0]["own_end_ms"].get<double>() == doctest::Approx(kPitchDark243OwnEndMs));
    CHECK(p243[0]["conversion"]["early"].get<double>() == doctest::Approx(kPitchDark243Conversion));
    CHECK(p243[0]["conversion"]["late"].get<double>() == doctest::Approx(kPitchDark243Conversion));

    // Every press with no end carries none of the end fields.
    for (const json& p : sch["presses"]) {
        if (!p["end_read_after"].is_null()) continue;
        CHECK(p["own_end_ms"].is_null());
        CHECK(p["conversion"].is_null());
    }
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
    schedule::EndLink l;
    l.read_stored_end_ms = 1000.0;
    l.own_stored_end_ms = 2000.0;
    l.conversion = TransferScale{0.5, 2.0};
    // At or after the own stored end: the late side.
    CHECK(schedule::read_at_own_ms(l, 2010.0) == doctest::Approx(1005.0));
    CHECK(schedule::read_at_own_ms(l, 2000.0) == doctest::Approx(1000.0));
    // Before it: the early side.
    CHECK(schedule::read_at_own_ms(l, 1990.0) == doctest::Approx(980.0));
    // Two ends of one step: the conversion moves the read one for one, on
    // both sides.
    l.conversion = schedule::end_conversion(TransferScale{1.3, 1.1}, TransferScale{1.3, 1.1});
    CHECK(schedule::read_at_own_ms(l, 2010.0) == doctest::Approx(1010.0));
    CHECK(schedule::read_at_own_ms(l, 1990.0) == doctest::Approx(990.0));
}
