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
