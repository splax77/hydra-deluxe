// Five-lane drum charts read the way Clone Hero 1.1 converts them at load.
// The plan is docs/superpowers/plans/2026-11-12-five-lane-drums.md (task T2),
// and the game's rule is its "What the game does" section. These tests were
// written before the readers change, so every one but the unchanged-chart test
// fails on the readers as they stood (each says why in one line).
//
// Every expected chord is a literal Chord::code() string; core/model.h says
// how that spells a chord. The .chart fixtures are text files in
// testdata/five_lane; the .mid fixtures are built here with tests/midi_util.h.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "load_util.h"
#include "midi_util.h"
#include "parse/song.h"
#include "song_digest.h"  // chart_parse_hash, digest_settings
#include "testdata_dirs.h"  // testdata_root

using namespace hydra;

namespace {

// Expert's pitches in a .mid: the kick, the five pads, the 2x kick.
constexpr uint8_t kKick = 96, kRed = 97, kYellow = 98, kBlue = 99, kOrange = 100, kGreen = 101;
constexpr uint8_t kKick2x = 95;
// Easy's green pad, six above Easy's 2x kick (59).
constexpr uint8_t kEasyGreen = 65;
// The shared drum-track markers.
constexpr uint8_t kSolo = 103, kYellowTom = 110, kStarPower = 116;
constexpr uint8_t kPlain = 100;  // a velocity that is neither an accent nor a ghost

// One drum-track event at an absolute tick.
struct At {
    uint32_t tick;
    std::vector<uint8_t> ev;  // a delta-0 event from midi_util.h
};

At on(uint32_t tick, uint8_t pitch, uint8_t velocity = kPlain) {
    return {tick, testmidi::note_on(pitch, velocity)};
}
At off(uint32_t tick, uint8_t pitch) { return {tick, testmidi::note_off(pitch)}; }
At text(uint32_t tick, const std::string& s) { return {tick, testmidi::text_event(s)}; }

// A .mid whose PART DRUMS holds `events`, in tick order (events on one tick
// keep the order given).
std::vector<uint8_t> drum_mid(std::vector<At> events) {
    std::stable_sort(events.begin(), events.end(),
                     [](const At& a, const At& b) { return a.tick < b.tick; });
    std::vector<std::vector<uint8_t>> ev = {testmidi::track_name("PART DRUMS")};
    uint32_t prev = 0;
    for (const At& e : events) {
        ev.push_back(testmidi::after(e.tick - prev, e.ev));
        prev = e.tick;
    }
    ev.push_back(testmidi::end_of_track());
    return testmidi::smf_drums(testmidi::concat(ev));
}

// Fixture A in .mid: one note on each pad, then orange and green on one tick,
// then a kick. testdata/five_lane/five_lane_a.chart is its .chart twin.
std::vector<uint8_t> mid_a() {
    return drum_mid({on(0, kRed), on(480, kYellow), on(960, kBlue), on(1440, kOrange),
                     on(1920, kGreen), on(2400, kOrange), on(2400, kGreen), on(2880, kKick)});
}

std::string fixture(const std::string& name) {
    return corpus::testdata_root() + "/five_lane/" + name;
}

const ChartReading kPro{};                                   // Expert, Pro Drums on
const ChartReading kPlainDrums{Difficulty::Expert, false};   // Expert, Pro Drums off

// Each chord's code, in song order.
std::vector<std::string> codes(const Song& song) {
    std::vector<std::string> out;
    for (const SongTimestamp& ts : song.sequence) out.push_back(ts.chord.code());
    return out;
}

// Each chord's tick, in song order.
std::vector<int64_t> ticks(const Song& song) {
    std::vector<int64_t> out;
    for (const SongTimestamp& ts : song.sequence) out.push_back(ts.timecode.ticks());
    return out;
}

// The chord at `tick`; fails the test when there is none.
std::string code_at(const Song& song, int64_t tick) {
    for (const SongTimestamp& ts : song.sequence)
        if (ts.timecode.ticks() == tick) return ts.chord.code();
    FAIL("no chord at tick " << tick);
    return {};
}

// Fixture A's chords once converted, by the plan's five-lane branch table.
const std::vector<std::string> kAConverted = {".n...", "..N..", "...n.", "....N",
                                               "....n", "...nN", "n...."};

}  // namespace

// ---- .mid --------------------------------------------------------------

TEST_CASE("five-lane .mid: every note is kept") {
    // Fails today: the .mid reader drops pitch 101, so a chord and two notes go.
    const Song song = load_songbytes_mid(mid_a(), kPro);
    CHECK(song.sequence.size() == 7);
    CHECK(song.note_count() == 8);
}

TEST_CASE("five-lane .mid: yellow is a cymbal, blue a tom, orange a green cymbal, green a green tom") {
    // Fails today: the chart is not converted (blue reads as a cymbal, green is dropped).
    CHECK(codes(load_songbytes_mid(mid_a(), kPro)) == kAConverted);
}

TEST_CASE("five-lane .mid: orange and green on one tick read as a green cymbal and a blue tom") {
    // Fails today: green is dropped, so the tick holds only the green cymbal.
    const Song song = load_songbytes_mid(drum_mid({on(0, kOrange), on(0, kGreen)}), kPro);
    CHECK(codes(song) == std::vector<std::string>{"...nN"});
}

TEST_CASE("five-lane .mid: a green pad on Easy alone still converts Expert") {
    // Fails today: with no mode decided from Easy's pitch 65, Expert's blue reads as a cymbal.
    const Song song = load_songbytes_mid(
        drum_mid({on(0, kEasyGreen), on(0, kYellow), on(480, kBlue), on(960, kOrange)}), kPro);
    CHECK(codes(song) == std::vector<std::string>{"..N..", "...n.", "....N"});
}

TEST_CASE("five-lane .mid: an accent and a ghost on the green pad survive the conversion") {
    // Fails today: pitch 101 is dropped, and its velocity mark with it.
    const Song song = load_songbytes_mid(
        drum_mid({text(0, "[ENABLE_CHART_DYNAMICS]"), on(0, kRed), on(480, kGreen, 127),
                  on(960, kGreen, 1)}),
        kPro);
    CHECK(codes(song) == std::vector<std::string>{".n...", "....a", "....g"});
}

TEST_CASE("five-lane .mid: Star Power, a solo and a 2x kick read the same beside a green pad") {
    // Fails today: the 2x kick's chord loses its green tom.
    // The phrase runs from 0 to 1000 and the solo from 0 to 1440, so both
    // cover all three chords; the 2x kick shares its tick with the green pad.
    auto chart = [](bool with_green) {
        std::vector<At> ev = {on(0, kStarPower), on(0, kSolo), on(0, kRed), on(480, kKick2x),
                              on(960, kRed), off(1000, kStarPower), off(1440, kSolo)};
        if (with_green) ev.push_back(on(480, kGreen));
        return load_songbytes_mid(drum_mid(ev), kPro);
    };
    const Song with = chart(true);
    const Song without = chart(false);
    CHECK(codes(with) == std::vector<std::string>{".n...", "N...n", ".n..."});
    REQUIRE(with.sequence.size() == without.sequence.size());
    for (size_t i = 0; i < with.sequence.size(); ++i) {
        CAPTURE(i);
        CHECK(with.sequence[i].flag_sp == without.sequence[i].flag_sp);
        CHECK(with.sequence[i].flag_solo == without.sequence[i].flag_solo);
        CHECK(with.sequence[i].chord.at(NoteColor::Kick) ==
              without.sequence[i].chord.at(NoteColor::Kick));
    }
    CHECK(with.sequence.back().flag_sp);
    REQUIRE(with.solo_sections.size() == 1);
    CHECK(with.solo_sections[0].first == 0);
    CHECK(with.solo_sections[0].last == 2);
}

TEST_CASE("five-lane .mid: with a tom marker the four-lane branch makes the green pad a tom") {
    // Fails today: green is dropped, so neither the green tom nor the blue tom appears.
    // The yellow tom marker covers only tick 960 and makes this a Pro chart,
    // so the plan's "The four-lane branch" applies.
    const Song song = load_songbytes_mid(
        drum_mid({on(0, kYellow), on(0, kGreen), on(480, kOrange), on(480, kGreen),
                  on(960, kYellowTom), on(960, kYellow), off(1000, kYellowTom)}),
        kPro);
    CHECK(codes(song) == std::vector<std::string>{"..N.n", "...nN", "..n.."});
}

TEST_CASE("five-lane .mid: with Pro Drums off no converted cymbal counts") {
    // Fails today: green is dropped, so a chord and two notes are missing.
    const Song song = load_songbytes_mid(mid_a(), kPlainDrums);
    CHECK(codes(song) == std::vector<std::string>{".n...", "..n..", "...n.", "....n", "....n",
                                                  "...nn", "n...."});
    for (const SongTimestamp& ts : song.sequence)
        for (const ChordNote& note : ts.chord.note_list()) CHECK_FALSE(note.is_cymbal());
}

TEST_CASE("five-lane .mid: a disco section flips the converted chord") {
    // Fails today: green is dropped from the flipped chord.
    // The flip is Chord::apply_disco_flip's; it does not touch the green lane.
    const Song song = load_songbytes_mid(
        drum_mid({text(0, "[mix 3 drums0d]"), on(0, kRed), on(0, kGreen), on(480, kYellow)}),
        kPro);
    CHECK(codes(song) == std::vector<std::string>{"..N.n", ".n..."});
}

TEST_CASE("five-lane .mid: blue, orange and green on one tick load with a green cymbal and a blue tom") {
    // Fails today: blue reads as a cymbal. How many blue notes the tick holds is T4's (gap G2).
    const Song song =
        load_songbytes_mid(drum_mid({on(0, kBlue), on(0, kOrange), on(0, kGreen)}), kPro);
    REQUIRE(song.sequence.size() == 1);
    const Chord& chord = song.sequence[0].chord;
    REQUIRE(chord.at(NoteColor::Blue).has_value());
    REQUIRE(chord.at(NoteColor::Green).has_value());
    CHECK_FALSE(chord.at(NoteColor::Blue)->is_cymbal());
    CHECK(chord.at(NoteColor::Green)->is_cymbal());
}

TEST_CASE("five-lane: the .mid reader and the .chart reader give the same chords for fixture A") {
    // Fails today: the .mid reads yellow and blue as cymbals, the .chart as toms, and both drop green.
    // The plan asks for the fast .mid reader against the full one, but the
    // full reader has no entry a test can reach: every .mid load goes through
    // parse_mid_lean. So this pins the lean reader against the other reader.
    const Song mid = load_songbytes_mid(mid_a(), kPro);
    const Song chart = load_songpath_chart(fixture("five_lane_a.chart"), kPro);
    CHECK(ticks(mid) == ticks(chart));
    CHECK(codes(mid) == codes(chart));
    CHECK(mid.note_count() == chart.note_count());
}

// ---- .chart ------------------------------------------------------------

TEST_CASE("five-lane .chart: every note is kept") {
    // Fails today: the .chart reader drops N 5, so a chord and two notes go.
    const Song song = load_songpath_chart(fixture("five_lane_a.chart"), kPro);
    CHECK(song.sequence.size() == 7);
    CHECK(song.note_count() == 8);
}

TEST_CASE("five-lane .chart: yellow is a cymbal, blue a tom, orange a green cymbal, green a green tom") {
    // Fails today: the chart is not converted (yellow and orange read as toms, N 5 is dropped).
    CHECK(codes(load_songpath_chart(fixture("five_lane_a.chart"), kPro)) == kAConverted);
}

TEST_CASE("five-lane .chart: N 4 and N 5 on one tick read as a green cymbal and a blue tom") {
    // Fails today: N 5 is dropped and N 4 reads as a green tom.
    CHECK(code_at(load_songpath_chart(fixture("five_lane_a.chart"), kPro), 2400) == "...nN");
}

TEST_CASE("five-lane .chart: N 5 in EasyDrums alone still converts ExpertDrums") {
    // Fails today: Expert is not converted, so its yellow and N 4 read as toms.
    // This assumes the .chart reader keeps the green-pad flag for the whole
    // file, as the .mid reader does; T1's reading of gap G5 may overturn it.
    CHECK(codes(load_songpath_chart(fixture("five_lane_easy_only.chart"), kPro)) ==
          std::vector<std::string>{"..N..", "...n.", "....N"});
}

TEST_CASE("five-lane .chart: N 38 and N 44 put an accent and a ghost on the green pad") {
    // Fails today: N 5, N 38 and N 44 are all dropped.
    CHECK(codes(load_songpath_chart(fixture("five_lane_dynamics.chart"), kPro)) ==
          std::vector<std::string>{".n...", "....a", "....g"});
}

// ---- four-lane charts ----------------------------------------------------

TEST_CASE("five-lane: four-lane charts parse to their pinned digests") {
    // Green today and must stay green: these charts have no green pad
    // (plan premise P26), so the conversion must leave them alone.
    // Each digest is the one testdata/parse_edge/expected_expert.tsv pins,
    // read from that file, under the settings it was captured with.
    const app::AnalysisSettings settings =
        digest::digest_settings("Expert", true, true).batch_run().settings;
    const std::string edge = corpus::testdata_root() + "/parse_edge/";
    for (const std::string name : {"m01_base.mid", "c01_base.chart", "c20_disco.chart"}) {
        CAPTURE(name);
        std::ifstream in(edge + "expected_expert.tsv", std::ios::binary);
        std::string line;
        uint64_t pinned = 0;
        bool found = false;
        while (std::getline(in, line)) {
            if (line.rfind(name + "\t", 0) != 0) continue;
            const size_t t1 = line.find('\t');
            const size_t t2 = line.find('\t', t1 + 1);
            pinned = std::stoull(line.substr(t1 + 1, t2 - t1 - 1), nullptr, 16);
            found = true;
        }
        REQUIRE(found);
        CHECK(digest::chart_parse_hash(edge + name, settings) == pinned);
    }
}
