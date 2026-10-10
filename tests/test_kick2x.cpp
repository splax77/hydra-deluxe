// Step 2, T2 (D20, findings 10, 12 and 255): each difficulty reads its own 2x
// kick, and the Dynamics tab counts kicks by one rule.

#include "doctest.h"

#include <cstdint>
#include <iterator>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "app/dynamics_breakdown.h"
#include "chart_text.h"
#include "core/model.h"
#include "difficulty_literals.h"
#include "midi_util.h"
#include "parse/song.h"

#ifndef HYDRA_INPUT_DIR
#error "HYDRA_INPUT_DIR must be defined (see CMakeLists.txt)"
#endif
#ifndef HYDRA_TESTDATA_DIR
#error "HYDRA_TESTDATA_DIR must be defined (see CMakeLists.txt)"
#endif

using namespace hydra;

TEST_CASE(".mid: each difficulty reads its own 2x kick pitch, as Clone Hero does") {
    // Clone Hero (0x2155050 at 0x21555CD) flags 59, 71, 83 and 95 as
    // DoubleKick, each in its own difficulty: one below that difficulty's kick.
    // One 2x kick per difficulty, 64 ticks apart, in kLiterals' order (tests/
    // difficulty_literals.h): Expert's at tick 0, Hard's at 64, Medium's at
    // 128, Easy's at 192.
    using namespace testmidi;
    constexpr uint32_t kStep = 64;
    std::vector<std::vector<uint8_t>> ev = {track_name("PART DRUMS"), set_tempo()};
    for (size_t i = 0; i < std::size(testdiff::kLiterals); ++i) {
        const std::vector<uint8_t> kick =
            note_on(static_cast<uint8_t>(testdiff::kLiterals[i].kick2x), 100);
        ev.push_back(i == 0 ? kick : after(kStep, kick));
    }
    ev.push_back(end_of_track());
    const std::vector<uint8_t> mid = smf(concat(ev));
    for (size_t i = 0; i < std::size(testdiff::kLiterals); ++i) {
        const testdiff::Literals& w = testdiff::kLiterals[i];
        CAPTURE(w.name);
        const Song on = load_songbytes_mid(mid, true, true, w.d);
        REQUIRE(on.sequence.size() == 1);
        CHECK(on.sequence[0].timecode.ticks() == static_cast<int64_t>(i * kStep));
        CHECK(on.sequence[0].chord.at(NoteColor::Kick)->is2x);
        CHECK(load_songbytes_mid(mid, true, false, w.d).sequence.empty());
    }
}

TEST_CASE(".mid: Expert's 2x kick never swallows a Hard kick on the same tick") {
    // Finding 255: Hard reads only its own 2x kick (83), so Expert's 95 on
    // the same tick is dropped. With the 95 first, Hard used to lose its real
    // kick to it.
    using namespace testmidi;
    const std::vector<uint8_t> mid = smf(concat(
        {track_name("PART DRUMS"), set_tempo(), note_on(95, 100), note_on(84, 100),
         end_of_track()}));
    const Song hard = load_songbytes_mid(mid, true, /*bass2x=*/true, Difficulty::Hard);
    REQUIRE(hard.sequence.size() == 1);
    REQUIRE(hard.sequence[0].chord.at(NoteColor::Kick).has_value());
    CHECK_FALSE(hard.sequence[0].chord.at(NoteColor::Kick)->is2x);
}

TEST_CASE(".chart: N 32 is a 2x kick only in the section being read") {
    const std::vector<uint8_t> data =
        testchart::chart_bytes(testchart::section("ExpertDrums", "  0 = N 32 0\n") +
                               testchart::section("HardDrums", "  192 = N 1 0\n  384 = N 32 0\n"));
    const Song on = load_songbytes_chart(data, true, true, Difficulty::Hard);
    REQUIRE(on.sequence.size() == 2);  // Expert's tick-0 N 32 is not Hard's
    CHECK(on.sequence[1].timecode.ticks() == 384);
    CHECK(on.sequence[1].chord.at(NoteColor::Kick)->is2x);
    CHECK(load_songbytes_chart(data, true, false, Difficulty::Hard).sequence.size() == 1);
}

TEST_CASE("Dynamics: one kick total follows the 2x Bass setting") {
    app::DynamicsBreakdown bd;
    bd.rows[static_cast<size_t>(app::DynamicsRow::RedSnare)] = {1, 2, 3};
    bd.rows[static_cast<size_t>(app::DynamicsRow::Kick)] = {0, 1, 40};
    bd.rows[static_cast<size_t>(app::DynamicsRow::Kick2x)] = {0, 0, 12};
    CHECK(bd.kicks_total(false).all() == 41);
    CHECK(bd.kicks_total(true).all() == 53);
    // Totals: the 6 red pads plus 41 or 53 kicks; accents 2 red plus 1 kick.
    CHECK(bd.played_total(false).all() == 47);
    CHECK(bd.played_total(true).all() == 59);
    CHECK(bd.played_total(false).accent == 3);
    CHECK(bd.played_total(true).accent == 3);
}

TEST_CASE("Dynamics: Car Bomb - The Sentinel at Hard counts Hard's own kicks") {
    // Finding 10's chart. Hard has 1,147 kicks (pitch 84) and no 2x kicks of
    // its own; Expert's 95 shares a tick with 409 of them. Before D20 the
    // Dynamics parse at Hard showed 828 kicks plus 319 "2x kicks" (the 95 came
    // first on those ticks), and Totals said 1,719 instead of 2,038.
    using app::DynamicsRow;
    const std::string path =
        std::string(HYDRA_INPUT_DIR) + "/common/IB24/T7/Car Bomb - The Sentinel/notes.mid";
    const app::DynamicsBreakdown bd = app::count_dynamics(
        load_songpath(path, /*pro=*/true, app::kDynamicsParseBass2x, Difficulty::Hard));
    CHECK(bd.row(DynamicsRow::Kick).all() == 1147);
    CHECK(bd.row(DynamicsRow::Kick2x).all() == 0);
    CHECK(bd.played_total(true).all() == 2038);
    CHECK(bd.played_total(false).all() == 2038);
}

// ---- D105: a 2x kick and a normal kick on one tick ------------------------

namespace {

// One character per chord: '.' no kick, 'n' a plain kick, 'x' a 2x kick.
std::string kick_row(const Song& song) {
    std::string row;
    for (const SongTimestamp& ts : song.sequence) {
        const auto& kick = ts.chord.at(NoteColor::Kick);
        row += !kick ? '.' : lane_flag(*kick) ? 'x' : 'n';
    }
    return row;
}

int note_total(const Song& song) {
    int n = 0;
    for (const SongTimestamp& ts : song.sequence) n += ts.chord.count();
    return n;
}

}  // namespace

TEST_CASE("D105: songs F1 (.mid) and F2 (.chart) read as Clone Hero played them") {
    // The game-test songs from docs/audit/note-shuffle/game-tests (README,
    // "F1 and F2"), copied here. Nine chords, each with a red snare. Chord 3
    // has a normal kick then a 2x kick in the file, chord 5 the same pair the
    // other way round, chord 7 a 2x kick alone, chords 1 and 9 a normal kick.
    // Clone Hero 1.1 showed a kick on chords 1 and 9 only, 11 notes, with 2x
    // kick off, and one kick on chords 1, 3, 5, 7 and 9, 14 notes, with it on
    // (FINDINGS.md, "Game tests").
    const std::string dir = std::string(HYDRA_TESTDATA_DIR) + "/kick2x_merge/";
    for (const char* file : {"notes.mid", "notes.chart"}) {
        CAPTURE(file);
        for (bool pro : {true, false}) {
            CAPTURE(pro);
            const Song off = load_songpath(dir + file, pro, /*bass2x=*/false);
            CHECK(kick_row(off) == "n.......n");
            CHECK(note_total(off) == 11);
            const Song on = load_songpath(dir + file, pro, /*bass2x=*/true);
            CHECK(kick_row(on) == "n.x.x.x.n");
            CHECK(note_total(on) == 14);
        }
    }
}

TEST_CASE("D105 .mid: the merged kick keeps the ghost or accent from either kick") {
    // Clone Hero ORs the two kicks' flags (0x215B8C0), so a mark on either
    // kick lands on the merged 2x kick, in both file orders. Ticks 0 and 64
    // put the ghost on the normal kick, then on the 2x kick.
    using namespace testmidi;
    const std::vector<uint8_t> mid = smf(concat(
        {track_name("PART DRUMS"), set_tempo(), text_event("[ENABLE_CHART_DYNAMICS]"),
         note_on(96, 1), note_on(95, 100),             // tick 0: ghost normal, plain 2x
         after(64, note_on(95, 1)), note_on(96, 100),  // tick 64: ghost 2x, plain normal
         end_of_track()}));
    const Song on = load_songbytes_mid(mid, true, /*bass2x=*/true);
    REQUIRE(on.sequence.size() == 2);
    CHECK(on.sequence[0].chord.code() == "G....");
    CHECK(on.sequence[1].chord.code() == "G....");
    // With 2x Bass off the merged kick is a 2x kick, so it goes: no chords.
    CHECK(load_songbytes_mid(mid, true, /*bass2x=*/false).sequence.empty());
}

TEST_CASE("D105 .mid: a ghost and an accent kick on one tick merge into an accent") {
    // Clone Hero's merged kick carries both marks (0x215B8C0), and its
    // note-list builder (0x20D4BE0) plays a note with both as an accent. So
    // the merged 2x kick is an accent in both file orders.
    using namespace testmidi;
    const std::vector<uint8_t> mid = smf(concat(
        {track_name("PART DRUMS"), set_tempo(), text_event("[ENABLE_CHART_DYNAMICS]"),
         note_on(96, 127), note_on(95, 1),             // tick 0: accent normal, ghost 2x
         after(64, note_on(95, 1)), note_on(96, 127),  // tick 64: ghost 2x, accent normal
         end_of_track()}));
    const Song on = load_songbytes_mid(mid, true, /*bass2x=*/true);
    REQUIRE(on.sequence.size() == 2);
    CHECK(on.sequence[0].chord.code() == "A....");
    CHECK(on.sequence[1].chord.code() == "A....");
    // With 2x Bass off the merged kick is a 2x kick, so it goes: no chords.
    CHECK(load_songbytes_mid(mid, true, /*bass2x=*/false).sequence.empty());
}

TEST_CASE(".chart: a pad with both an accent and a ghost marker is an accent") {
    // The game ORs both markers onto the chart note and plays it as an
    // accent (0x20D4BE0), whichever marker comes first. Tick 0 has the
    // accent marker first, tick 192 the ghost marker first; both on red.
    const std::vector<uint8_t> data = testchart::chart_bytes(testchart::section(
        "ExpertDrums",
        "  0 = N 1 0\n  0 = N 34 0\n  0 = N 40 0\n"
        "  192 = N 1 0\n  192 = N 40 0\n  192 = N 34 0\n"));
    for (bool pro : {true, false}) {
        CAPTURE(pro);
        const Song song = load_songbytes_chart(data, pro, true);
        REQUIRE(song.sequence.size() == 2);
        CHECK(song.sequence[0].chord.code() == ".a...");
        CHECK(song.sequence[1].chord.code() == ".a...");
    }
}

TEST_CASE("D105 .mid: a kick the 2x Bass setting removes leaves no mark before the tag") {
    // The Dynamics tab counts ghost and accent velocities read before a late
    // [ENABLE_CHART_DYNAMICS] tag. A kick that 2x Bass off removes is never
    // priced, so its mark doesn't count, wherever the tag sits.
    using namespace testmidi;
    SUBCASE("the tag on a later tick") {
        const std::vector<uint8_t> mid = smf(concat(
            {track_name("PART DRUMS"), set_tempo(),
             note_on(96, 1), note_on(95, 100), note_on(97, 100),  // tick 0
             after(64, note_on(97, 1)),                           // tick 64: ghost red
             after(64, text_event("[ENABLE_CHART_DYNAMICS]")), note_on(97, 100),
             end_of_track()}));
        const Song off = load_songbytes_mid(mid, true, /*bass2x=*/false);
        CHECK(off.dynamics_marks_before_tag == 1);
        REQUIRE(off.dynamics_late_tag_tick.has_value());
        CHECK(*off.dynamics_late_tag_tick == 128);
        CHECK(load_songbytes_mid(mid, true, /*bass2x=*/true).dynamics_marks_before_tag == 2);
    }
    SUBCASE("the tag between the two kicks on their own tick") {
        const std::vector<uint8_t> mid = smf(concat(
            {track_name("PART DRUMS"), set_tempo(), note_on(96, 1),
             text_event("[ENABLE_CHART_DYNAMICS]"), note_on(95, 100), note_on(97, 100),
             end_of_track()}));
        const Song off = load_songbytes_mid(mid, true, /*bass2x=*/false);
        CHECK(off.dynamics_marks_before_tag == 0);
        CHECK_FALSE(off.dynamics_late_tag_tick.has_value());
        const Song on = load_songbytes_mid(mid, true, /*bass2x=*/true);
        CHECK(on.dynamics_marks_before_tag == 1);
        CHECK(on.dynamics_late_tag_tick == std::optional<int64_t>(0));
    }
}
