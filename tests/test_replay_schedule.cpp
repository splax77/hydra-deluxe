// hydra_replay schedule (tools/replay_schedule.h): the geometry it reads off
// the engine, the game's per-note window and the constants, and the rule that
// nothing under src/ reads any of it.

#include "doctest.h"

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "json.hpp"

#include "app/analysis.h"
#include "app/config.h"
#include "core/model.h"
#include "core/replay.h"
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

// The colours a declined chord of `colours` sends under `mode`: the chord is
// built for real, so its notes come in Chord::note_list(true) order and its
// activation note is the engine's.
std::set<NoteColor> kept_colours(schedule::DeclineMode mode, const std::set<NoteColor>& colours) {
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
    for (size_t k : schedule::kept_gems(mode, chord, notes).places) out.insert(notes[k].color);
    return out;
}

// A side at the floor and at the cap: half of TM-21's whole window at a
// summed gap of 75 and of 170 (TM-22 prints them as 48.15 and 85.72).
const doctest::Approx kFloorSide = doctest::Approx(96.2931672 / 2);
const doctest::Approx kCapSide = doctest::Approx(171.4313075 / 2);

}  // namespace

TEST_CASE("schedule: every end step names its anchor") {
    const Song song = plain_song({0, 768, 1536, 2304, 3072, 3840});
    const schedule::ChordIndex chords = schedule::chord_index(song);

    Activation act;
    act.timecode = song.timecode(768);
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
    const schedule::ChordIndex chords = schedule::chord_index(song);

    // A SqIn on the step at 3072 and a SqOut of the chord at 4000. Each
    // ratio is a value no recompute would give, so a recompute shows.
    Activation act;
    act.timecode = song.timecode(768);
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
    // The mode's rule, on Freak's two shapes (plan question 4): chord 162
    // (kick, blue cymbal, green cymbal) and chord 51 (kick, blue tom).
    using schedule::DeclineMode;
    using Colours = std::set<NoteColor>;
    const Colours c162 = {NoteColor::Kick, NoteColor::Blue, NoteColor::Green};
    const Colours c51 = {NoteColor::Kick, NoteColor::Blue};
    CHECK(kept_colours(DeclineMode::AllButTopPad, c162) == Colours{NoteColor::Kick, NoteColor::Blue});
    CHECK(kept_colours(DeclineMode::KickOnly, c162) == Colours{NoteColor::Kick});
    CHECK(kept_colours(DeclineMode::Nothing, c162).empty());
    CHECK(kept_colours(DeclineMode::AllButTopPad, c51) == Colours{NoteColor::Kick});
    CHECK(kept_colours(DeclineMode::KickOnly, c51) == Colours{NoteColor::Kick});
    CHECK(kept_colours(DeclineMode::Nothing, c51).empty());
    // No pad gem: nothing is sent.
    CHECK(kept_colours(DeclineMode::AllButTopPad, Colours{NoteColor::Kick}).empty());
    // The names: one home, read and written there.
    CHECK(schedule::decline_mode_from_name("kick_only") == DeclineMode::KickOnly);
    CHECK(schedule::decline_mode_from_name("nothing") == DeclineMode::Nothing);
    CHECK(schedule::decline_mode_from_name("all_but_top_pad") == DeclineMode::AllButTopPad);
    CHECK_FALSE(schedule::decline_mode_from_name("kick").has_value());

    // On the corpus: the declined rows are exactly the skipped fills, and
    // each one's kept gems are the mode's.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int declined_rows = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.reading());
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(path, cfg);
        for (const Path* p : rec.all_paths()) {
            std::vector<const Activation*> acts;
            std::set<int64_t> skipped;
            for (const Activation& a : p->walk_activations()) {
                acts.push_back(&a);
                skipped.insert(a.skipped_fill_ticks.begin(), a.skipped_fill_ticks.end());
            }
            const PathReplay pr = replay_stored_path(song, *p, cfg.rules);
            const json s = schedule::schedule_json(song, acts, pr.result, DeclineMode::AllButTopPad);
            std::set<int64_t> marked;
            for (const json& row : s["chords"]) {
                if (!row["declined"].get<bool>()) continue;
                ++declined_rows;
                marked.insert(row["tick"].get<int64_t>());
                const ReplayChord& c = pr.result.chords[row["index"].get<size_t>()];
                CHECK(row["kept_gems"].get<std::vector<size_t>>() ==
                      schedule::kept_gems(DeclineMode::AllButTopPad, song.sequence[static_cast<size_t>(c.index)].chord,
                                          c.notes)
                          .places);
            }
            INFO(path);
            CHECK(marked == skipped);
        }
    }
    CHECK(declined_rows > 0);
}

TEST_CASE("schedule: the window formula and clamps are pinned") {
    // TM-21's three worked values.
    CHECK(schedule::whole_window_ms(170.0) == doctest::Approx(171.4313).epsilon(1e-6));
    CHECK(schedule::whole_window_ms(75.0) == doctest::Approx(96.2932).epsilon(1e-6));
    CHECK(schedule::whole_window_ms(80.0) == doctest::Approx(102.2586).epsilon(1e-6));
    // TM-22: the floor and the cap of a side.
    CHECK(schedule::window_side_ms(1.0, 1.0) == kFloorSide);
    CHECK(schedule::window_side_ms(10000.0, 10000.0) == kCapSide);
    // TM-19: a missing neighbour, or one at the same time, gives the back.
    CHECK(schedule::half_gap_ms(std::nullopt) == 85.0);
    CHECK(schedule::half_gap_ms(0.0) == 85.0);
    CHECK(schedule::half_gap_ms(10.0) == 37.5);
    CHECK(schedule::half_gap_ms(100.0) == 50.0);
    // Freak's chord 263: 56.18 ms from both neighbours.
    CHECK(schedule::window_side_ms(56.18, 56.18) == kFloorSide);

    const Song song = plain_song({0, 768});
    const ReplayResult r = replay_path(song, {});
    const json c = schedule::schedule_json(song, {}, r, schedule::DeclineMode::Nothing)["constants"];
    CHECK(c["frame_ms"].get<double>() == 3.0);
    CHECK(c["judge_early_ms"].get<double>() == 3.0);
    CHECK(c["judge_late_ms"].get<double>() == 6.0);
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
        const json s = schedule::schedule_json(song, {}, r, schedule::DeclineMode::Nothing);
        REQUIRE(s["chords"].size() == 3);
        CHECK(s["chords"][1]["side_ms"].get<double>() == kFloorSide);
    }
    // Chords 917 ticks (2,388 ms) and 2,999 ticks (7,810 ms) either side of
    // the middle one: both half-gaps clamp to the back, so the side is the cap.
    {
        const Song song = plain_song({0, 917, 917 + 2999});
        const ReplayResult r = replay_path(song, {});
        const json s = schedule::schedule_json(song, {}, r, schedule::DeclineMode::Nothing);
        REQUIRE(s["chords"].size() == 3);
        CHECK(s["chords"][1]["side_ms"].get<double>() == kCapSide);
    }
    // A chord's own gems are not each other's neighbours (TM-51): a
    // two-gem chord between far neighbours still gets the cap.
    {
        Song song = plain_song({0, 917, 917 + 2999});
        song.sequence[1].chord.add_note(NoteColor::Kick);
        const ReplayResult r = replay_path(song, {});
        const json s = schedule::schedule_json(song, {}, r, schedule::DeclineMode::Nothing);
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
    // The decline flag is refused the same way, naming the modes.
    const std::optional<std::string> d = schedule::decline_flag_refusal(std::string("kick"));
    REQUIRE(d.has_value());
    CHECK(d->find("all_but_top_pad") != std::string::npos);
    CHECK(schedule::decline_flag_refusal(std::nullopt).has_value());
    CHECK_FALSE(schedule::decline_flag_refusal(std::string("nothing")).has_value());
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
    // 263 (side 48.15), ratio 1.0.
    const schedule::SqueezeGeometry g = sqout(48.15, 60545.26, 53.59, 60573.35, 1.0);
    const schedule::SqueezeSplit s = schedule::split_squeeze(g);
    CHECK(within_cent(s.anchor_aim_ms, -24.82));
    CHECK(within_cent(s.squeeze_aim_ms, 24.27));
    CHECK(within_cent(s.anchor_room_ms, 23.32));
    CHECK(within_cent(s.squeeze_room_ms, 23.32));
    CHECK(within_cent(s.planned_end_ms, 60551.53));
    CHECK(within_cent(s.aim_end_ms, 60554.53));
    CHECK(within_cent(schedule::sqout_fallback_after_ms(60545.26, 53.59), 60577.85));
    CHECK(within_cent(s.aim_end_ms + s.squeeze_room_ms, 60577.85));
    CHECK(s.feasible);
    // The cap's side on 263 (the pruned fixture's 85.72) gives another lead.
    CHECK_FALSE(within_cent(schedule::split_squeeze(sqout(85.72, 60545.26, 53.59, 60573.35, 1.0)).anchor_aim_ms, -24.82));

    // The whole file on the overfill shape: the lead goes on chord 2, the
    // chord the clamp anchors the end on, never on the activation's chord 1.
    const Overfill o = overfill();
    const ReplayResult r = replay_path(o.song, {});
    const json sch = schedule::schedule_json(o.song, {&o.act}, r, schedule::DeclineMode::Nothing);
    INFO(sch["refusals"].dump());
    CHECK(sch["refusals"].empty());  // and no "not planned yet" line
    const std::vector<json> anchor = presses_of(sch, 2, "anchor");
    REQUIRE(anchor.size() == 1);
    CHECK(within_cent(anchor[0]["aim_ms"].get<double>(), -26.52));
    REQUIRE(presses_of(sch, 1, "plain").size() == 1);
    CHECK(presses_of(sch, 1, "plain")[0]["aim_ms"].get<double>() == -2.0);
    // The read after chord 2 sees the clamped end, 8,026.04 ms, planned
    // 23.52 ms earlier (the lead less one frame).
    const json& reads = sch["chords"][2]["reads"];
    REQUIRE(reads.size() == 1);
    CHECK(within_cent(reads[0]["stored_end_ms"].get<double>(), 8026.04));
    CHECK(within_cent(reads[0]["planned_end_ms"].get<double>(), 8002.52));
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
        {"Freak 1144", 73.76, 53.59, 84.27, 1.0, -65.72, 39.55, 8.04},
        {"Rescued 561", 75.93, 61.47, 99.04, 1.0284, -69.36, 48.89, 6.58},
        {"Rescued 931", 76.49, 61.94, 100.00, 1.0045, -70.63, 50.08, 5.86},
        {"Ghost 756", 78.75, 66.34, 109.25, 1.0133, -73.88, 55.47, 4.87},
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
    const json s = schedule::schedule_json(song, {&act}, r, schedule::DeclineMode::Nothing);
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
    // kRoomMarginMs: a 25 ms gap and 29 ms sides leave 3.0 ms of room each
    // (feasible); 28.9 ms sides leave 2.9 (not).
    const schedule::SqueezeSplit three = schedule::split_squeeze(sqout(29.0, 1000.0, 29.0, 1025.0, 1.0));
    CHECK(three.anchor_room_ms == doctest::Approx(3.0));
    CHECK(three.squeeze_room_ms == doctest::Approx(3.0));
    CHECK(three.feasible);
    const schedule::SqueezeSplit short_room = schedule::split_squeeze(sqout(28.9, 1000.0, 28.9, 1025.0, 1.0));
    CHECK(short_room.anchor_room_ms == doctest::Approx(2.9));
    CHECK_FALSE(short_room.feasible);
}

TEST_CASE("schedule: the fallback block") {
    // Freak 1144 (141,571.98 ms, side 53.59): past 141,604.57 the late gem
    // joins the inside group.
    CHECK(within_cent(schedule::sqout_fallback_after_ms(141571.98, 53.59), 141604.57));
    // A squeeze-in pressed before a moved end: chord.ms - side + 15 + 3.
    CHECK(schedule::sqin_fallback_before_ms(1000.0, 50.0) == doctest::Approx(968.0));

    // The overfill shape's two-gem squeeze-out: the late press's block names
    // the last inside group's gems.
    {
        const Overfill o = overfill();
        const ReplayResult r = replay_path(o.song, {});
        const json s = schedule::schedule_json(o.song, {&o.act}, r, schedule::DeclineMode::Nothing);
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
        const json s = schedule::schedule_json(song, {&act}, r, schedule::DeclineMode::Nothing);
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
    const json s = schedule::schedule_json(o.song, {&o.act}, r, schedule::DeclineMode::Nothing);
    REQUIRE(!s["presses"].empty());
    for (const json& p : s["presses"]) {
        const json& row = s["chords"][p["chord"].get<size_t>()];
        CHECK(p["last_send_ms"].get<double>() ==
              doctest::Approx(schedule::last_send_ms(row["ms"].get<double>(), row["side_ms"].get<double>())));
    }
    // TM-27: a press sent exactly on its side's edge is inside.
    std::vector<schedule::PadChord> one(1);
    one[0].index = 0;
    one[0].ms = 1000.0;
    one[0].side_ms = 50.0;
    one[0].chart_pads = {{NoteColor::Red, false}};
    one[0].sent_pads = one[0].chart_pads;
    CHECK_FALSE(schedule::far_press_refusal(one, 0, 1050.0, one[0].chart_pads).has_value());
    CHECK(schedule::far_press_refusal(one, 0, 1050.5, one[0].chart_pads).has_value());
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
    // The Edge Walk Pair shape: two kicks 100 ms apart, chords 10 and 11.
    const std::vector<double> sides = schedule::chord_sides_ms({1000.0, 1100.0});
    std::vector<schedule::PadChord> pair(2);
    for (size_t i = 0; i < 2; ++i) {
        pair[i].index = 10 + static_cast<int>(i);
        pair[i].ms = 1000.0 + 100.0 * static_cast<double>(i);
        pair[i].side_ms = sides[i];
        pair[i].chart_pads = {{NoteColor::Kick, false}};
        pair[i].sent_pads = pair[i].chart_pads;
    }
    const std::vector<schedule::Pad> kick = {{NoteColor::Kick, false}};
    // Chord 11 pressed 60 ms early lands inside chord 10's window.
    const std::optional<std::string> r = schedule::far_press_refusal(pair, 1, 1040.0, kick);
    REQUIRE(r.has_value());
    CHECK(r->find("chord 11") != std::string::npos);
    CHECK(r->find("chord 10") != std::string::npos);
    // Chord 10 pressed 60 ms late reaches chord 11, which the player presses:
    // the game gives the press to the earlier note, so it passes.
    CHECK_FALSE(schedule::far_press_refusal(pair, 0, 1060.0, kick).has_value());
    // Declined (no kick sent), chord 11 would take the press: refused.
    pair[1].sent_pads.clear();
    const std::optional<std::string> d = schedule::far_press_refusal(pair, 0, 1060.0, kick);
    REQUIRE(d.has_value());
    CHECK(d->find("chord 10") != std::string::npos);
    CHECK(d->find("chord 11") != std::string::npos);
}

TEST_CASE("schedule: a late gem that pays less than an inside gem is refused") {
    std::vector<std::string> refusals;
    CHECK_FALSE(schedule::squeeze_out_inside_groups(7, {1, 2}, refusals).has_value());
    REQUIRE(refusals.size() == 1);
    CHECK(refusals[0].find("chord 7") != std::string::npos);
    CHECK(refusals[0].find("late gem pays x1 and an inside gem x2") != std::string::npos);
    refusals.clear();
    const auto ok = schedule::squeeze_out_inside_groups(7, {2, 2, 1}, refusals);
    REQUIRE(ok.has_value());
    CHECK(refusals.empty());
    CHECK(*ok == std::vector<std::vector<size_t>>{{2}, {1}});
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
    const json s = schedule::schedule_json(o.song, {&o.act}, r, schedule::DeclineMode::Nothing);
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
    const json s = schedule::schedule_json(song, {&act}, r, schedule::DeclineMode::Nothing);
    const json& reads = s["chords"][1]["reads"];
    REQUIRE(reads.size() == 1);
    CHECK(within_cent(reads[0]["stored_end_ms"].get<double>(), 7812.5));
    CHECK(within_cent(reads[0]["planned_end_ms"].get<double>(), 7813.5));
    // A led anchor: the stored end moved by ratio x (lead - one frame);
    // Freak 263's 24.82 ms lead at ratio 1.0 plans 60,551.53.
    CHECK(within_cent(schedule::split_squeeze(sqout(48.15, 60545.26, 53.59, 60573.35, 1.0)).planned_end_ms, 60551.53));
}

TEST_CASE("schedule: the equal-room split of a squeeze-in is the mirror of a squeeze-out's") {
    // No chart in the corpus has a timed squeeze-in, so this is hand-built: the
    // anchor (side 60) goes late for a chord (side 50) 40 ms past the end at
    // 1,000 ms, ratio 1.0 on the late side (the early side, 2.0, is one a
    // squeeze-in never reads). Worked by hand from the split rule in
    // replay_schedule.cpp: L = (60 - 50 + 40 + 15 - 6 - 3) / 2 = 28; the end
    // lands at 1,000 + 1 x (28 + 3) = 1,031; the chord is pressed at
    // 1,031 - 15 - 1,040 = -24; both rooms are 26.
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
    CHECK(s.anchor_aim_ms == doctest::Approx(28.0));
    CHECK(s.squeeze_aim_ms == doctest::Approx(-24.0));
    CHECK(s.anchor_room_ms == doctest::Approx(26.0));
    CHECK(s.squeeze_room_ms == doctest::Approx(26.0));
    CHECK(s.planned_end_ms == doctest::Approx(1031.0));
    CHECK(s.aim_end_ms == doctest::Approx(1031.0));
    CHECK(s.feasible);
}
