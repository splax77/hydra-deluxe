// Five-lane drum charts read the way Clone Hero 1.1 converts them at load.
// The game's rule is NL-74 to NL-77 on docs/clone-hero/02-note-list.md; the
// readers' one home for it is convert_drum_pads in parse/song.cpp.
//
// Every expected chord is a literal Chord::code() string; core/model.h says
// how that spells a chord. The .chart fixtures are text files in
// testdata/five_lane; the .mid fixtures are built here with tests/midi_util.h.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "chart_text.h"    // chart_bytes, section
#include "core/replay.h"  // replay_path
#include "load_util.h"
#include "midi_util.h"
#include "parse/song.h"
#include "sng_util.h"     // make_sng
#include "srb_util.h"     // make_srb, make_metadata
#include "temp_util.h"      // temp_dir, write_bytes
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

// Fixture A's notes as (tick, Expert pitch), in tick order: one note on each
// pad, then orange and green on one tick, then a kick.
// testdata/five_lane/five_lane_a.chart is its .chart twin.
const std::vector<std::pair<uint32_t, uint8_t>> kANotes = {
    {0, kRed},      {480, kYellow},  {960, kBlue},   {1440, kOrange},
    {1920, kGreen}, {2400, kOrange}, {2400, kGreen}, {2880, kKick}};

// Fixture A in .mid.
std::vector<uint8_t> mid_a() {
    std::vector<At> ev;
    for (const auto& note : kANotes) ev.push_back(on(note.first, note.second));
    return drum_mid(ev);
}

// The ticks fixture A writes notes on, each once, in song order.
std::vector<int64_t> a_ticks() {
    std::vector<int64_t> out;
    for (const auto& note : kANotes) {
        const int64_t tick = note.first;
        if (out.empty() || out.back() != tick) out.push_back(tick);
    }
    return out;
}

std::string five_lane_path(const std::string& name) {
    return corpus::testdata_root() + "/five_lane/" + name;
}

const ChartReading kPro{};                                   // Expert, Pro Drums on
const ChartReading kPlainDrums{Difficulty::Expert, false};   // Expert, Pro Drums off

// Each chord's tick, in song order.
std::vector<int64_t> ticks(const Song& song) {
    std::vector<int64_t> out;
    for (const SongTimestamp& ts : song.sequence) out.push_back(ts.timecode.ticks());
    return out;
}

// Fixture A's chords once converted, by the five-lane branch (NL-74, NL-75).
const std::vector<std::string> kAConverted = {".n...", "..N..", "...n.", "....N",
                                               "....n", "...nN", "n...."};

// A reading of fixture A keeps every note: one note for each note the fixture
// writes, and one chord for each tick it writes on.
void check_keeps_every_a_note(const Song& song) {
    CHECK(song.note_count() == static_cast<int>(kANotes.size()));
    CHECK(song.sequence.size() == a_ticks().size());
}

// A song folder in the temp folder holding `notes_name` (its bytes) and a
// song.ini whose [song] section holds `ini_lines`. Returns the notes file's
// path. Writing it again with other lines changes only the song.ini.
std::string song_folder(const std::string& tag, const std::string& notes_name,
                        const std::vector<uint8_t>& notes, const std::string& ini_lines) {
    const std::string dir = testtemp::temp_dir("five_lane_" + tag);
    const std::string notes_path = dir + "\\" + notes_name;
    testtemp::write_bytes(notes_path, notes);
    const std::string ini = "[song]\nname = Five Lane " + tag + "\n" + ini_lines;
    testtemp::write_bytes(dir + "\\song.ini", std::vector<uint8_t>(ini.begin(), ini.end()));
    return notes_path;
}

// A .chart whose ExpertDrums section holds `drum_lines` (tests/chart_text.h).
std::vector<uint8_t> expert_drums_chart(const std::string& drum_lines) {
    return testchart::chart_bytes(testchart::section("ExpertDrums", drum_lines), 480);
}

// The score with no Star Power anywhere, as `hydra_replay score` gives it
// with no path.
int64_t no_sp_total(const Song& song) { return replay_path(song, {}).final.total(); }

// How many notes in the song are cymbals.
int cymbal_count(const Song& song) {
    int n = 0;
    for (const SongTimestamp& ts : song.sequence)
        for (const ChordNote& note : ts.chord.note_list())
            if (note.is_cymbal()) ++n;
    return n;
}

// A four-pad .mid with no tom marker and no fifth pad: a kick, then one note
// on each of the first four pads. `extra` adds events to it.
std::vector<uint8_t> no_pro_mid(std::vector<At> extra = {}) {
    std::vector<At> ev = {on(0, kKick), on(480, kRed), on(960, kYellow), on(1440, kBlue),
                          on(1920, kOrange)};
    ev.insert(ev.end(), extra.begin(), extra.end());
    return drum_mid(ev);
}

// no_pro_mid's chords with every pad a tom, and with the reader's own
// cymbals kept, and as the five-lane branch converts them.
const std::vector<std::string> kNoProToms = {"n....", ".n...", "..n..", "...n.", "....n"};
const std::vector<std::string> kNoProCymbals = {"n....", ".n...", "..N..", "...N.", "....N"};
const std::vector<std::string> kNoProFiveLane = {"n....", ".n...", "..N..", "...n.", "....N"};

}  // namespace

// ---- .mid --------------------------------------------------------------

TEST_CASE("five-lane .mid: every note is kept") {
    check_keeps_every_a_note(load_songbytes_mid(mid_a(), kPro));
}

TEST_CASE("five-lane .mid: yellow is a cymbal, blue a tom, orange a green cymbal, green a green tom") {
    CHECK(chord_codes(load_songbytes_mid(mid_a(), kPro)) == kAConverted);
}

TEST_CASE("five-lane .mid: a green pad on Easy alone still converts Expert") {
    const Song song = load_songbytes_mid(
        drum_mid({on(0, kEasyGreen), on(0, kYellow), on(480, kBlue), on(960, kOrange)}), kPro);
    CHECK(chord_codes(song) == std::vector<std::string>{"..N..", "...n.", "....N"});
}

TEST_CASE("five-lane .mid: an accent and a ghost on the green pad survive the conversion") {
    const Song song = load_songbytes_mid(
        drum_mid({text(0, "[ENABLE_CHART_DYNAMICS]"), on(0, kRed), on(480, kGreen, 127),
                  on(960, kGreen, 1)}),
        kPro);
    CHECK(chord_codes(song) == std::vector<std::string>{".n...", "....a", "....g"});
}

TEST_CASE("five-lane .mid: Star Power, a solo and a 2x kick read the same beside a green pad") {
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
    CHECK(chord_codes(with) == std::vector<std::string>{".n...", "N...n", ".n..."});
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
    // The yellow tom marker covers only tick 960 and makes this a Pro chart,
    // so the four-lane branch applies (NL-76).
    const Song song = load_songbytes_mid(
        drum_mid({on(0, kYellow), on(0, kGreen), on(480, kOrange), on(480, kGreen),
                  on(960, kYellowTom), on(960, kYellow), off(1000, kYellowTom)}),
        kPro);
    CHECK(chord_codes(song) == std::vector<std::string>{"..N.n", "...nN", "..n.."});
}

TEST_CASE("five-lane .mid: with Pro Drums off no converted cymbal counts") {
    const Song song = load_songbytes_mid(mid_a(), kPlainDrums);
    CHECK(chord_codes(song) == std::vector<std::string>{".n...", "..n..", "...n.", "....n", "....n",
                                                  "...nn", "n...."});
    for (const SongTimestamp& ts : song.sequence)
        for (const ChordNote& note : ts.chord.note_list()) CHECK_FALSE(note.is_cymbal());
}

TEST_CASE("five-lane .mid: a disco section flips the converted chord") {
    // The flip is Chord::apply_disco_flip's; it does not touch the green lane.
    const Song song = load_songbytes_mid(
        drum_mid({text(0, "[mix 3 drums0d]"), on(0, kRed), on(0, kGreen), on(480, kYellow)}),
        kPro);
    CHECK(chord_codes(song) == std::vector<std::string>{"..N.n", ".n..."});
}

TEST_CASE("five-lane .mid: blue, orange and green on one tick load with a green cymbal and a blue tom") {
    const Song song =
        load_songbytes_mid(drum_mid({on(0, kBlue), on(0, kOrange), on(0, kGreen)}), kPro);
    REQUIRE(song.sequence.size() == 1);
    // hydra:: because temp_util.h's <windows.h> also declares the GDI function ::Chord.
    const hydra::Chord& chord = song.sequence[0].chord;
    REQUIRE(chord.at(NoteColor::Blue).has_value());
    REQUIRE(chord.at(NoteColor::Green).has_value());
    CHECK_FALSE(chord.at(NoteColor::Blue)->is_cymbal());
    CHECK(chord.at(NoteColor::Green)->is_cymbal());
}

TEST_CASE("five-lane: the .mid reader and the .chart reader put fixture A's chords on its ticks") {
    CHECK(ticks(load_songbytes_mid(mid_a(), kPro)) == a_ticks());
    CHECK(ticks(load_songpath_chart(five_lane_path("five_lane_a.chart"), kPro)) == a_ticks());
}

// ---- .chart ------------------------------------------------------------

TEST_CASE("five-lane .chart: every note is kept") {
    check_keeps_every_a_note(load_songpath_chart(five_lane_path("five_lane_a.chart"), kPro));
}

TEST_CASE("five-lane .chart: yellow is a cymbal, blue a tom, orange a green cymbal, green a green tom") {
    CHECK(chord_codes(load_songpath_chart(five_lane_path("five_lane_a.chart"), kPro)) == kAConverted);
}

TEST_CASE("five-lane .chart: N 5 in EasyDrums alone still converts ExpertDrums") {
    // This assumes the .chart reader keeps the green-pad flag for the whole
    // file, as the .mid reader does (CR-18).
    CHECK(chord_codes(load_songpath_chart(five_lane_path("five_lane_easy_only.chart"), kPro)) ==
          std::vector<std::string>{"..N..", "...n.", "....N"});
}

TEST_CASE("five-lane .chart: N 38 and N 44 put an accent and a ghost on the green pad") {
    CHECK(chord_codes(load_songpath_chart(five_lane_path("five_lane_dynamics.chart"), kPro)) ==
          std::vector<std::string>{".n...", "....a", "....g"});
}

// ---- song.ini's five_lane_drums and pro_drums ----------------------------
//
// Each test below loads a chart from a folder with a song.ini beside it, the
// way every chart path is loaded, so the readers' song.ini keys are in play.

TEST_CASE("five-lane .chart: five_lane_drums takes the five-lane branch despite a Pro mark") {
    // Probe C's shape: a fifth pad and a yellow cymbal mark.
    const std::vector<uint8_t> chart =
        expert_drums_chart("  0 = N 2 0\n  0 = N 66 0\n  480 = N 3 0\n  960 = N 4 0\n  1440 = N 5 0\n");
    // The same chart read again after its song.ini changes: no key first.
    const std::string path = song_folder("key_pro_mark", "notes.chart", chart, "");
    CHECK(chord_codes(load_songpath(path, kPro)) ==
          std::vector<std::string>{"..N..", "...n.", "....n", "....n"});
    song_folder("key_pro_mark", "notes.chart", chart, "five_lane_drums = True\n");
    CHECK(chord_codes(load_songpath(path, kPro)) ==
          std::vector<std::string>{"..N..", "...n.", "....N", "....n"});
}

TEST_CASE("five-lane .chart: five_lane_drums alone makes a four-pad chart's yellow and N 4 cymbals") {
    // Probe E's shape: no fifth pad and no cymbal mark.
    const std::string path = song_folder("key_four_pad_chart", "notes.chart",
                                         expert_drums_chart("  0 = N 2 0\n  480 = N 3 0\n  960 = N 4 0\n"),
                                         "five_lane_drums = True\n");
    CHECK(chord_codes(load_songpath(path, kPro)) ==
          std::vector<std::string>{"..N..", "...n.", "....N"});
}

TEST_CASE("five-lane .mid: five_lane_drums takes the five-lane branch despite a tom marker") {
    const std::string path = song_folder(
        "key_tom_marker", "notes.mid",
        drum_mid({on(0, kYellowTom), on(0, kYellow), off(240, kYellowTom), on(480, kBlue),
                  on(960, kGreen)}),
        "five_lane_drums = True\n");
    CHECK(chord_codes(load_songpath(path, kPro)) ==
          std::vector<std::string>{"..N..", "...n.", "....n"});
}

TEST_CASE("five-lane .mid: five_lane_drums alone makes a four-pad chart's blue a tom") {
    const std::string path =
        song_folder("key_four_pad_mid", "notes.mid",
                    drum_mid({on(0, kYellow), on(480, kBlue), on(960, kOrange)}),
                    "five_lane_drums = True\n");
    CHECK(chord_codes(load_songpath(path, kPro)) ==
          std::vector<std::string>{"..N..", "...n.", "....N"});
}

TEST_CASE("five-lane .mid: pro_drums = True with a fifth pad takes the four-lane branch") {
    // The game's code starts the Pro flag from pro_drums (CR-18); no run has shown it.
    const std::string path = song_folder("pro_drums_key", "notes.mid",
                                         drum_mid({on(0, kBlue), on(480, kGreen)}),
                                         "pro_drums = True\n");
    CHECK(chord_codes(load_songpath(path, kPro)) == std::vector<std::string>{"...N.", "....n"});
}

TEST_CASE("five-lane .mid: blue, orange and green on one tick keep the green pad's accent") {
    // The game merges the two blue toms into one carrying both notes' marks
    // (NL-77); probe D showed the merge as one note.
    const Song song = load_songbytes_mid(
        drum_mid({text(0, "[ENABLE_CHART_DYNAMICS]"), on(480, kBlue), on(480, kOrange),
                  on(480, kGreen, 127)}),
        kPro);
    CHECK(chord_codes(song) == std::vector<std::string>{"...aN"});
}

TEST_CASE("five-lane: probes C and E score what Clone Hero 1.1 scored") {
    // The totals are the bot runs of 2026-11-12 on v1.1.0.6142, ProDrums
    // Expert, every note hit, no Star Power (hydra-data
    // ch-mcp-runs/2026-11-12/five-lane). The folders are copies of the probe
    // songs, each with its song.ini.
    CHECK(no_sp_total(load_songpath(five_lane_path("probe_c/notes.chart"), kPro)) == 7890);
    CHECK(no_sp_total(load_songpath(five_lane_path("probe_e/notes.chart"), kPro)) == 6490);
}

// ---- neither flag up: every pad a tom --------------------------------------
//
// A four-lane chart with no fifth pad and no Pro marks ends Clone Hero's
// conversion with every pad a tom (NL-74 on docs/clone-hero/02-note-list.md).
// Hydra's one home for it is convert_drum_pads.

TEST_CASE("five-lane all-toms: a .mid with neither flag up has no cymbal") {
    const Song song = load_songbytes_mid(no_pro_mid(), kPro);
    CHECK(chord_codes(song) == kNoProToms);
    CHECK(cymbal_count(song) == 0);
}

TEST_CASE("five-lane all-toms: one tom marker keeps the reader's cymbals") {
    // The marker covers a tick with no note, so it marks no pad; it only
    // raises the Pro flag.
    const Song song =
        load_songbytes_mid(no_pro_mid({on(2400, kYellowTom), off(2410, kYellowTom)}), kPro);
    CHECK(chord_codes(song) == kNoProCymbals);
}

TEST_CASE("five-lane all-toms: a fifth pad on Easy alone keeps the five-lane branch's cymbals") {
    const Song song = load_songbytes_mid(no_pro_mid({on(0, kEasyGreen)}), kPro);
    CHECK(chord_codes(song) == kNoProFiveLane);
}

TEST_CASE("five-lane all-toms: five_lane_drums = True keeps the five-lane branch's cymbals") {
    const std::string path =
        song_folder("all_toms_five_lane_key", "notes.mid", no_pro_mid(), "five_lane_drums = True\n");
    CHECK(chord_codes(load_songpath(path, kPro)) == kNoProFiveLane);
}

TEST_CASE("five-lane all-toms: pro_drums = True or 1 keeps the reader's cymbals") {
    const std::string path =
        song_folder("all_toms_pro_key", "notes.mid", no_pro_mid(), "pro_drums = True\n");
    CHECK(chord_codes(load_songpath(path, kPro)) == kNoProCymbals);
    song_folder("all_toms_pro_key", "notes.mid", no_pro_mid(), "pro_drums = 1\n");
    CHECK(chord_codes(load_songpath(path, kPro)) == kNoProCymbals);
}

TEST_CASE("five-lane all-toms: the two keys read yes or no by the game's bool getter") {
    // The rows follow Clone Hero's bool getter at 0x1D4480, which reads both
    // keys. "+1", "1 ", "1.0" and "99999999999" follow .NET's Int32.TryParse,
    // the getter's number test.
    struct Row {
        const char* text;
        bool set;
    };
    const Row rows[] = {
        {"1", true},      {"2", true},     {"-1", true},          {"true", true},
        {"TRUE", true},   {"True", true},  {"+1", true},          {"1 ", true},
        {"0", false},     {"false", false}, {"yes", false},       {"", false},
        {"1.0", false},   {"99999999999", false},
    };
    for (const Row& row : rows) {
        CAPTURE(std::string(row.text));
        const ChartDrumKeys keys = chart_drum_keys(std::map<std::string, std::string>{
            {"five_lane_drums", row.text}, {"pro_drums", row.text}});
        CHECK(keys.five_lane_drums == row.set);
        CHECK(keys.pro_drums == row.set);
    }
    const ChartDrumKeys missing = chart_drum_keys(std::map<std::string, std::string>{});
    CHECK_FALSE(missing.five_lane_drums);
    CHECK_FALSE(missing.pro_drums);
}

TEST_CASE("five-lane all-toms: a .chart with neither flag up reads as before") {
    // A .chart pad is a tom unless N 66 to 68 marks it, so this chart was
    // all toms before the conversion had its tail.
    const std::vector<uint8_t> chart = expert_drums_chart(
        "  0 = N 0 0\n  480 = N 1 0\n  960 = N 2 0\n  1440 = N 3 0\n  1920 = N 4 0\n");
    CHECK(chord_codes(load_songbytes_chart(chart, kPro)) == kNoProToms);
}

TEST_CASE("five-lane all-toms: a .mid inside a .sng follows its metadata's pro_drums") {
    // Clone Hero's .sng bool getter (0x2642C0) reads pro_drums = 1 as unset,
    // where a song.ini's getter reads it as set.
    const Song bare = load_songbytes_sng(testsng::make_sng({}, {{"notes.mid", no_pro_mid()}}), kPro);
    CHECK(chord_codes(bare) == kNoProToms);
    CHECK(cymbal_count(bare) == 0);
    const Song pro = load_songbytes_sng(
        testsng::make_sng({{"pro_drums", "True"}}, {{"notes.mid", no_pro_mid()}}), kPro);
    CHECK(chord_codes(pro) == kNoProCymbals);
    const Song one = load_songbytes_sng(
        testsng::make_sng({{"pro_drums", "1"}}, {{"notes.mid", no_pro_mid()}}), kPro);
    CHECK(chord_codes(one) == kNoProToms);
    CHECK(cymbal_count(one) == 0);
}

TEST_CASE("five-lane all-toms: a .sng's two keys read yes or no by the game's .sng bool getter") {
    // The rows follow Clone Hero's .sng bool getter at 0x2642C0, which reads
    // both keys through .NET's Boolean.TryParse.
    struct Row {
        const char* text;
        bool set;
    };
    const Row rows[] = {
        {"true", true}, {"TRUE", true}, {" True ", true}, {"1", false},     {"-1", false},
        {"yes", false}, {"", false},    {"0", false},     {"false", false},
    };
    for (const Row& row : rows) {
        CAPTURE(std::string(row.text));
        const ChartDrumKeys keys = chart_drum_keys(std::vector<std::pair<std::string, std::string>>{
            {"five_lane_drums", row.text}, {"pro_drums", row.text}});
        CHECK(keys.five_lane_drums == row.set);
        CHECK(keys.pro_drums == row.set);
    }
    const ChartDrumKeys missing =
        chart_drum_keys(std::vector<std::pair<std::string, std::string>>{});
    CHECK_FALSE(missing.five_lane_drums);
    CHECK_FALSE(missing.pro_drums);
}

TEST_CASE("five-lane all-toms: a .mid inside an .srb with no tom marker has no cymbal") {
    // An .srb carries no song.ini keys, so neither key can raise a flag.
    const Song song = load_songbytes_srb(
        testsrb::make_srb(testsrb::make_metadata("notes.mid"), no_pro_mid()), kPro);
    CHECK(chord_codes(song) == kNoProToms);
    CHECK(cymbal_count(song) == 0);
}

TEST_CASE("five-lane all-toms: a disco section still flips red to a yellow cymbal") {
    // The flip runs after the conversion, as the game's builder does it later
    // than its load step (NL-22).
    const Song song = load_songbytes_mid(
        drum_mid({text(0, "[mix 3 drums0d]"), on(0, kRed), on(480, kYellow)}), kPro);
    CHECK(chord_codes(song) == std::vector<std::string>{"..N..", ".n..."});
}

TEST_CASE("five-lane all-toms: probe F scores what Clone Hero 1.1 scored") {
    // Probe F's notes (Mid No Pro F): 29 kicks, then 2 red, 3 yellow, 4 blue
    // and 5 fourth-pad notes, one per tick, with no song.ini key. The total is
    // the bot run of 2026-11-12 on v1.1.0.6142, ProDrums Expert, every note
    // hit, no Star Power (hydra-data ch-mcp-runs/2026-11-12/five-lane,
    // scorestats-Mid_No_Pro_F-20261112-131107.json).
    std::vector<At> ev;
    uint32_t tick = 0;
    for (const auto& [pitch, count] : std::vector<std::pair<uint8_t, int>>{
             {kKick, 29}, {kRed, 2}, {kYellow, 3}, {kBlue, 4}, {kOrange, 5}})
        for (int i = 0; i < count; ++i, tick += 480) ev.push_back(on(tick, pitch));
    const Song song = load_songbytes_mid(drum_mid(ev), kPro);
    REQUIRE(song.note_count() == 43);
    CHECK(no_sp_total(song) == 5750);
}
