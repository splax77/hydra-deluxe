// Smoke test for parse/song: every chart in the corpus parses without
// throwing, and the resulting timestamp sequence holds the structural
// invariants the search depends on (monotone ticks, decodable chords,
// consistent activation-fill placement). Covers .mid, .chart, and .sng.
// The odd and malformed input cases and the display-text cases are in
// test_song_input.cpp.

#include "doctest.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <optional>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/dynamics_breakdown.h"
#include "app/user_messages.h"
#include "core/error_kind.h"
#include "core/model.h"
#include "chart_text.h"
#include "corpus_util.h"
#include "midi_util.h"
#include "multidiff_chart.h"
#include "parse/chart_files.h"
#include "parse/midi.h"
#include "parse/song.h"
#include "song_digest.h"
#include "source_tree.h"
#include "ui/preview_controller.h"  // PreviewSongKey

using namespace hydra;

namespace {

// Whether check_invariants checks that every authored activation fill is positive. A
// chart with a fill starting on its own last note has a 0-length activation,
// so its case skips that one check.
enum class FillCheck { Check, Skip };

// The structural invariants the search depends on, checked on one parsed song.
// Each failed invariant is one failed CHECK naming `what`.
void check_invariants(const Song& song, const std::string& what,
                      FillCheck fill_check = FillCheck::Check) {
    bool ticks_ok = true, codes_ok = true, fills_ok = true;
    // A generated fill starts on its measure line, so its length may be 0 or
    // negative (Song::check_activations); the fill check below is for the
    // chart's own fills only. That length is answer 6 in
    // docs/superpowers/plans/2026-10-12-one-record-per-chart.md (removed; `git show 444631e6:docs/superpowers/plans/2026-10-12-one-record-per-chart.md`).
    const bool generated = std::find(song.features.begin(), song.features.end(),
                                     kGeneratedFillsFeature) != song.features.end();
    int64_t prev = -1;
    for (const SongTimestamp& ts : song.sequence) {
        // Strictly increasing: two chords never share a tick.
        if (ts.timecode.ticks() <= prev) ticks_ok = false;
        prev = ts.timecode.ticks();

        // Every chord spells as a code (Chord::code throws on one it can't).
        if (ts.chord.code().empty()) codes_ok = false;

        // An authored activation fill's length is always positive.
        if (!generated && ts.activation_length.has_value() && *ts.activation_length <= 0)
            fills_ok = false;
    }
    CHECK_MESSAGE(ticks_ok, what << ": ticks not strictly increasing");
    CHECK_MESSAGE(codes_ok, what << ": a chord with no code");
    if (fill_check == FillCheck::Check)
        CHECK_MESSAGE(fills_ok, what << ": non-positive activation fill");
}

}  // namespace

namespace {

// The corpus as the scan lists it. A missing or empty corpus is a broken
// tree, so a sweep over it fails at once instead of passing on nothing.
const std::vector<std::string>& corpus_sweep() {
    const std::vector<std::string>& paths = corpus::chart_paths();
    REQUIRE_MESSAGE(!paths.empty(), "the corpus scan found no chart under " << corpus::root());
    return paths;
}

}  // namespace

TEST_CASE("song parse holds its invariants over the corpus") {
    // Every corpus chart is a drum chart (each .chart has an [ExpertDrums]
    // section and each .mid a PART DRUMS track), so an empty song at Expert is
    // a parse that lost the notes, not a chart to pass over.
    for (const std::string& path : corpus_sweep()) {
        const Song& song = corpus::song(path, ChartReading{});
        CHECK_MESSAGE(!song.is_empty(), path << ": no Expert notes parsed");
        check_invariants(song, path);
    }
}

TEST_CASE("memory: a loaded song's chord list holds no spare capacity") {
    // Every loader ends at load_songbytes_mid or load_songbytes_chart, so the
    // corpus covers .mid, .chart and the containers alike. Parsed fresh here,
    // not through corpus::song's cache. An empty song is checked too: its list
    // must hold no spare room either.
    for (const std::string& path : corpus_sweep()) {
        const Song song = load_songpath(path, ChartReading{});
        CHECK_MESSAGE(song.sequence.capacity() == song.sequence.size(), path);
    }
}

namespace {

// One `key = words` line of a .chart section, as the file spells it: the key
// is the tick in a track section ("768") or a name in [Song] ("Resolution"),
// and the words are the right-hand side split on spaces ("N", "1", "0").
struct ChartTextLine {
    std::string key;
    std::vector<std::string> words;
};

// A .chart's sections by name, read from the file's own lines and not
// through the parser. A later section of a name replaces an earlier one.
std::map<std::string, std::vector<ChartTextLine>> chart_text_sections(const std::string& path) {
    std::map<std::string, std::vector<ChartTextLine>> sections;
    std::vector<ChartTextLine>* current = nullptr;
    std::string text = corpus::read_bytes(path);
    const std::string bom = "\xEF\xBB\xBF";  // a UTF-8 byte order mark, which chart editors may write
    if (text.compare(0, bom.size(), bom) == 0) text.erase(0, bom.size());
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const size_t first = line.find_first_not_of(" \t");
        if (first == std::string::npos) continue;
        const std::string trimmed = line.substr(first);
        if (trimmed.front() == '[' && trimmed.back() == ']') {
            current = &sections[trimmed.substr(1, trimmed.size() - 2)];
            current->clear();
            continue;
        }
        const size_t eq = trimmed.find(" = ");
        if (current == nullptr || eq == std::string::npos) continue;
        ChartTextLine l;
        l.key = trimmed.substr(0, eq);
        std::istringstream words(trimmed.substr(eq + 3));
        for (std::string w; words >> w;) l.words.push_back(w);
        if (!l.words.empty()) current->push_back(std::move(l));
    }
    return sections;
}

// Whether `lines` hold an N line (a note).
bool has_note_line(const std::vector<ChartTextLine>& lines) {
    return std::any_of(lines.begin(), lines.end(),
                       [](const ChartTextLine& l) { return l.words[0] == "N"; });
}

// Whether a .chart's own text holds an N line in `section`.
bool chart_text_has_notes(const std::string& path, const std::string& section) {
    const auto sections = chart_text_sections(path);
    const auto it = sections.find(section);
    return it != sections.end() && has_note_line(it->second);
}

// Whether a .mid's drum track holds a note-on of one of `difficulty`'s drum
// pitches: its kick, its 2x kick and the four pads after the kick
// (DifficultyChartCodes), read with the file reader and not the song parser.
bool mid_track_has_notes(const std::string& path, Difficulty difficulty) {
    const MidiFile file = MidiFile::from_file(path);
    const MidiTrack* drums = file.drums_track();
    if (drums == nullptr) return false;
    const DifficultyChartCodes& codes = difficulty_chart_codes(difficulty);
    const int lowest = codes.kick2x_pitch();
    const int highest = codes.kick_pitch + 4;
    for (const Message& m : drums->messages)
        if (m.is_note_on() && m.note >= lowest && m.note <= highest) return true;
    return false;
}

}  // namespace

TEST_CASE("song parse holds its invariants at Hard too") {
    // Same sweep at Hard. A chart parses to Hard notes exactly when its own
    // file holds Hard notes: an N line in the .chart's Hard section, or a
    // note-on at a Hard pitch in the .mid's drum track.
    for (const std::string& path : corpus_sweep()) {
        const Song& song = corpus::song(path, ChartReading{Difficulty::Hard});
        const ChartFormat format = chart_format_of(path);
        REQUIRE_MESSAGE((format == ChartFormat::Mid || format == ChartFormat::Chart),
                        path << ": a corpus chart in a format this test cannot read by itself");
        const bool file_has_hard =
            format == ChartFormat::Mid
                ? mid_track_has_notes(path, Difficulty::Hard)
                : chart_text_has_notes(path, difficulty_chart_codes(Difficulty::Hard).chart_section());
        CHECK_MESSAGE(song.is_empty() == !file_has_hard,
                      path << ": the file " << (file_has_hard ? "has" : "has no")
                           << " Hard notes, but the parsed song "
                           << (song.is_empty() ? "is empty" : "has notes"));
        check_invariants(song, path + " [Hard]");
    }
}

namespace {

bool has_cymbal(const Song& song) {
    for (const SongTimestamp& ts : song.sequence)
        for (const ChordNote& n : ts.chord.notes())
            if (n.cymbaltype != NoteCymbalType::Normal) return true;
    return false;
}

}  // namespace

TEST_CASE("cymbal_counts: a cymbal marker counts only under Pro Drums") {
    // With Pro Drums off no pad's marker counts; with it on, exactly the pads
    // that can carry a cymbal at all (allows_cymbals, the owner of that list).
    for (const NoteColor pad : {NoteColor::Kick, NoteColor::Red, NoteColor::Yellow,
                                NoteColor::Blue, NoteColor::Green}) {
        CAPTURE(color_str(pad));
        CHECK_FALSE(cymbal_counts(pad, false));
        CHECK(cymbal_counts(pad, true) == allows_cymbals(pad));
    }

    // Both readers ask it: every cymbal a corpus chart parses to under Pro
    // Drums sits on a pad whose marker counts, and the first corpus .mid and
    // .chart that hold a cymbal under Pro Drums hold none with Pro Drums off.
    for (const std::string& path : corpus::chart_paths())
        for (const SongTimestamp& ts : corpus::song(path, ChartReading{}).sequence)
            for (const ChordNote& n : ts.chord.notes())
                if (n.cymbaltype != NoteCymbalType::Normal)
                    CHECK_MESSAGE(cymbal_counts(n.colortype, true), path);
    bool mid_seen = false, chart_seen = false;
    for (const std::string& path : corpus::chart_paths()) {
        const ChartFormat format = chart_format_of(path);
        bool& seen = format == ChartFormat::Mid ? mid_seen : chart_seen;
        if (seen || (format != ChartFormat::Mid && format != ChartFormat::Chart)) continue;
        if (!has_cymbal(corpus::song(path, ChartReading{}))) continue;
        seen = true;
        const Song plain = load_songpath(path, ChartReading{Difficulty::Expert, false});
        CHECK_FALSE(plain.is_empty());
        CHECK_MESSAGE(!has_cymbal(plain), path);
        if (mid_seen && chart_seen) break;
    }
    CHECK(mid_seen);
    CHECK(chart_seen);
}

TEST_CASE(".chart: each difficulty reads its own section") {
    const std::vector<uint8_t> data = multidiff::chart_bytes();

    Song expert = load_songbytes_chart(data, ChartReading{});
    Song hard = load_songbytes_chart(data, ChartReading{Difficulty::Hard});
    Song medium = load_songbytes_chart(data, ChartReading{Difficulty::Medium});
    Song easy = load_songbytes_chart(data, ChartReading{Difficulty::Easy});

    CHECK(expert.sequence.size() == multidiff::kExpertChords);
    CHECK(hard.sequence.size() == multidiff::kHardChords);
    CHECK(easy.sequence.size() == multidiff::kEasyChords);
    // No [MediumDrums] section at all: a missing difficulty parses empty
    // rather than throwing or falling back to another difficulty.
    CHECK(medium.is_empty());
    CHECK(medium.sequence.size() == multidiff::kMediumChords);

    // The sections really are different charting, not the same notes read
    // four times.
    REQUIRE(!expert.sequence.empty());
    REQUIRE(!hard.sequence.empty());
    REQUIRE(!easy.sequence.empty());
    CHECK(expert.sequence[0].chord.code() != hard.sequence[0].chord.code());
    CHECK(easy.sequence[0].chord.code() != hard.sequence[0].chord.code());

    // Everything inside the section follows it: the ghost, the pro cymbal, the
    // SP phrase and the activation fill are all in [HardDrums] alone.
    CHECK(hard.sequence[0].flag_sp);
    bool any_activation = false;
    for (const SongTimestamp& ts : hard.sequence)
        if (ts.has_activation()) any_activation = true;
    CHECK(any_activation);
    // The fill was charted, so no fills had to be synthesized.
    CHECK(hard.features.empty());
    // Pro cymbals are still a pro-drums-only reading at Hard.
    Song hard_nonpro = load_songbytes_chart(data, ChartReading{Difficulty::Hard, false, true});
    CHECK(hard_nonpro.sequence[1].chord.code() != hard.sequence[1].chord.code());

    check_invariants(hard, "multidiff [Hard]");
    check_invariants(easy, "multidiff [Easy]");
}

TEST_CASE(".mid: each difficulty reads only the kick at its own pitch") {
    // One kick per difficulty in one drum track, each a beat after the one
    // before, at that difficulty's kick pitch (difficulty_chart_codes). Each
    // difficulty reads its own kick and none of the other three.
    using namespace testmidi;
    constexpr uint32_t kBeat = 480;  // midi_util's division, one quarter note
    std::vector<std::vector<uint8_t>> parts = {track_name("PART DRUMS")};
    bool first = true;
    for (const Difficulty d : kAllDifficulties) {
        const uint8_t kick = static_cast<uint8_t>(difficulty_chart_codes(d).kick_pitch);
        parts.push_back(first ? note_on(kick, 100) : after(kBeat, note_on(kick, 100)));
        first = false;
    }
    parts.push_back(end_of_track());
    const std::vector<uint8_t> data = smf_drums(concat(parts));

    int64_t want_tick = 0;
    for (const Difficulty d : kAllDifficulties) {
        CAPTURE(difficulty_name(d));
        const Song song = load_songbytes_mid(data, ChartReading{d});
        REQUIRE(song.sequence.size() == 1);
        CHECK(song.sequence[0].timecode.ticks() == want_tick);
        REQUIRE(song.sequence[0].chord.notes().size() == 1);
        CHECK(song.sequence[0].chord.notes()[0].colortype == NoteColor::Kick);
        want_tick += kBeat;
    }
}

TEST_CASE(".chart: [Events] section markers become practice sections") {
    const std::vector<uint8_t> data = testchart::chart_bytes(
        testchart::section("Events",
                           "  1536 = E \"prc_chorus_1a\"\n"
                           "  0 = E \"section Intro\"\n"
                           "  768 = E \"section Verse 1\"\n"
                           "  1920 = E \"lighting (blackout)\"\n") +
        testchart::section("ExpertDrums", "  0 = N 0 0\n  192 = N 1 0\n"));
    Song song = load_songbytes_chart(data, ChartReading{});

    // Both spellings are read, non-section text events are not, and the file's
    // own order does not have to be sorted. Names show as Clone Hero shows
    // them: chorus_1a is in its list as "Chorus 1a".
    REQUIRE(song.practice_sections.size() == 3);
    CHECK(song.practice_sections[0].tick == 0);
    CHECK(song.practice_sections[0].name == "Intro");
    CHECK(song.practice_sections[1].tick == 768);
    CHECK(song.practice_sections[1].name == "Verse 1");
    CHECK(song.practice_sections[2].tick == 1536);
    CHECK(song.practice_sections[2].name == "Chorus 1a");

    // A chart with no [Events] section has no sections and still parses.
    Song plain = load_songbytes_chart(multidiff::chart_bytes(), ChartReading{});
    CHECK(plain.practice_sections.empty());

    // The game's .chart reader has no _ENDOFSONG veto, unlike its .mid reader
    // (block CR-24 on docs/clone-hero/01-reading-a-chart.md), so this line is a
    // section.
    const Song endofsong = load_songbytes_chart(
        testchart::chart_bytes(
            testchart::section("Events", "  0 = E \"section Intro\"\n"
                                         "  768 = E \"section _ENDOFSONG\"\n") +
            testchart::section("ExpertDrums", "  0 = N 0 0\n  192 = N 1 0\n")),
        ChartReading{});
    REQUIRE(endofsong.practice_sections.size() == 2);
    CHECK(endofsong.practice_sections[1].tick == 768);
    CHECK(endofsong.practice_sections[1].name == " ENDOFSONG");
}

TEST_CASE(".chart: E \"end\" and E \"[end]\" in [Events] are the end event") {
    // [Events] out of tick order: the first end event in file order is the one
    // kept, not the one with the lowest tick. "End" is the wrong case, and an
    // "end" in the drum section is not an event of the song.
    const auto events_then_drums = [](const std::string& events) {
        return testchart::chart_bytes(
            testchart::section("Events", events) +
            testchart::section("ExpertDrums", "  0 = N 0 0\n  100 = E \"end\"\n  192 = N 1 0\n"));
    };
    const Song plain = load_songbytes_chart(events_then_drums("  500 = E \"End\"\n"
                                                              "  3000 = E \"end\"\n"
                                                              "  2000 = E \"end\"\n"),
                                            ChartReading{});
    CHECK(plain.end_event_tick == std::optional<int64_t>(3000));

    const Song bracketed = load_songbytes_chart(events_then_drums("  500 = E \"End\"\n"
                                                                  "  4000 = E \"[end]\"\n"
                                                                  "  2000 = E \"end\"\n"),
                                                ChartReading{});
    CHECK(bracketed.end_event_tick == std::optional<int64_t>(4000));

    const Song wrong_case = load_songbytes_chart(events_then_drums("  500 = E \"End\"\n"
                                                                   "  600 = E \"[END]\"\n"),
                                                 ChartReading{});
    CHECK_FALSE(wrong_case.end_event_tick.has_value());

    // Two [Events] sections: the first end event in the file stays, even though
    // a later section replaces the practice sections; a first section with no
    // end event leaves the second one's to count.
    const auto two_events = [](const std::string& first, const std::string& second) {
        return testchart::chart_bytes(
            testchart::section("Events", first) + testchart::section("Events", second) +
            testchart::section("ExpertDrums", "  0 = N 0 0\n  192 = N 1 0\n"));
    };
    CHECK(load_songbytes_chart(two_events("  3000 = E \"end\"\n", "  2000 = E \"end\"\n"),
                               ChartReading{})
              .end_event_tick == std::optional<int64_t>(3000));
    CHECK(load_songbytes_chart(two_events("  100 = E \"section A\"\n", "  2000 = E \"end\"\n"),
                               ChartReading{})
              .end_event_tick == std::optional<int64_t>(2000));

    // A chart with no [Events] section has no end event.
    CHECK_FALSE(load_songbytes_chart(multidiff::chart_bytes(), ChartReading{})
                    .end_event_tick.has_value());
}

TEST_CASE(".chart: the last note end is the latest N line's tick plus sustain in any section") {
    // The drum section's last note sits at tick 768, a guitar sustain runs
    // past it, and a Hard bass note ends at 1000. Events and phrase (S) lines
    // are not notes, even the ones at later ticks.
    constexpr int64_t kGuitarTick = 600, kGuitarSustain = 900;
    const std::vector<uint8_t> data = testchart::chart_bytes(
        testchart::section("Events", "  5000 = E \"section Outro\"\n") +
        testchart::section("ExpertDrums", "  0 = N 0 0\n  768 = N 1 0\n  700 = S 2 4000\n") +
        testchart::section("ExpertSingle", "  0 = N 0 0\n" +
                                               testchart::line(kGuitarTick, "N 2 " +
                                                               std::to_string(kGuitarSustain))) +
        testchart::section("HardDoubleBass", "  1000 = N 1 0\n"));
    const Song song = load_songbytes_chart(data, ChartReading{});
    const int64_t guitar_sustain_end = kGuitarTick + kGuitarSustain;
    CHECK(song.last_note_end_tick == std::optional<int64_t>(guitar_sustain_end));

    // Drums only: the last drum note's own sustain counts too.
    constexpr int64_t kDrumTick = 192, kDrumSustain = 48;
    const Song drums = load_songbytes_chart(
        testchart::chart_bytes(testchart::section(
            "ExpertDrums", "  0 = N 0 0\n" + testchart::line(kDrumTick, "N 1 " +
                                                              std::to_string(kDrumSustain)))),
        ChartReading{});
    const int64_t drum_sustain_end = kDrumTick + kDrumSustain;
    CHECK(drums.last_note_end_tick == std::optional<int64_t>(drum_sustain_end));

    // No N line anywhere: unset.
    const Song none = load_songbytes_chart(
        testchart::chart_bytes(testchart::section("Events", "  0 = E \"end\"\n")), ChartReading{});
    CHECK_FALSE(none.last_note_end_tick.has_value());
}

namespace {

// The latest N line's tick plus sustain in `lines`, or nullopt with none.
std::optional<int64_t> latest_note_end(const std::vector<ChartTextLine>& lines) {
    std::optional<int64_t> end;
    for (const ChartTextLine& l : lines) {
        if (l.words[0] != "N" || l.words.size() < 3) continue;
        const int64_t e = std::stoll(l.key) + std::stoll(l.words[2]);
        if (!end || e > *end) end = e;
    }
    return end;
}

// Milliseconds from tick 0 to `tick` through the chart's own B lines (tempo
// in thousandths of a beat a minute), each beat `resolution` ticks long.
double chart_text_ms(const std::map<std::string, std::vector<ChartTextLine>>& sections,
                     int64_t tick) {
    int64_t resolution = 0;
    for (const ChartTextLine& l : sections.at("Song"))
        if (l.key == "Resolution") resolution = std::stoll(l.words[0]);
    REQUIRE(resolution > 0);
    std::map<int64_t, double> bpm;
    for (const ChartTextLine& l : sections.at("SyncTrack"))
        if (l.words[0] == "B") bpm[std::stoll(l.key)] = std::stod(l.words[1]) / 1000.0;
    REQUIRE_MESSAGE((!bpm.empty() && bpm.begin()->first == 0), "no tempo at tick 0");
    double ms = 0;
    for (auto it = bpm.begin(); it != bpm.end() && it->first < tick; ++it) {
        const auto next = std::next(it);
        const int64_t until = next == bpm.end() ? tick : (std::min)(next->first, tick);
        const double beats = static_cast<double>(until - it->first) / static_cast<double>(resolution);
        ms += beats * 60000.0 / it->second;
    }
    return ms;
}

}  // namespace

TEST_CASE("corpus .chart: the last note end spans every section, through the tempo map") {
    // Each corpus .chart's last note end is the latest N line's tick plus
    // sustain over all its sections, read from the file's own lines, and its
    // time is that tick through the chart's own B lines. A .mid's comes from
    // MidiFile::last_note_end_tick, whose tests are in test_midi.cpp.
    int past_drums = 0;
    for (const std::string& path : corpus_sweep()) {
        if (chart_format_of(path) != ChartFormat::Chart) continue;
        CAPTURE(path);
        const auto sections = chart_text_sections(path);
        std::optional<int64_t> want, drums_end;
        for (const auto& [name, lines] : sections) {
            const std::optional<int64_t> end = latest_note_end(lines);
            if (!end) continue;
            if (!want || *end > *want) want = end;
            for (const Difficulty d : kAllDifficulties)
                if (name == difficulty_chart_codes(d).chart_section() &&
                    (!drums_end || *end > *drums_end))
                    drums_end = end;
        }
        REQUIRE(want.has_value());
        if (!drums_end || *want > *drums_end) ++past_drums;

        const Song song = load_songpath(path, ChartReading{});
        REQUIRE(song.last_note_end_tick.has_value());
        CHECK(*song.last_note_end_tick == *want);
        // One microsecond: far under any error a wrong tempo segment makes,
        // far over the rounding of two sums of doubles.
        CHECK(std::abs(song.timecode(*want).ms() - chart_text_ms(sections, *want)) < 1e-3);
    }
    // The rule is about sections past the drums, so the corpus must hold a
    // chart whose guitar, bass or other section ends after its drums (it
    // holds several, Chaotrope - Senescence among them).
    CHECK(past_drums > 0);
}

namespace {

// The .chart fixture with one solo whose `E soloend` sits on its third note.
std::vector<uint8_t> solo_end_chart() {
    return testchart::chart_bytes(
        testchart::section("ExpertDrums",
                           "  0 = E solo\n  0 = N 1 0\n"
                           "  192 = N 2 0\n"
                           "  384 = N 3 0\n  384 = E soloend\n"
                           "  576 = N 4 0\n"));
}

// A .mid with one solo marker held from tick 0 to tick 960 (480 ticks a beat,
// midi_util's division) over three notes: Red at 0, Yellow at 480 and Blue on
// the marker's note-off tick.
std::vector<uint8_t> solo_marker_mid() {
    using namespace testmidi;
    return smf_drums(concat({
        track_name("PART DRUMS"),
        note_on(103, 100), note_on(97, 100),   // tick 0: solo on, Red
        after(480, note_on(98, 100)),          // tick 480: Yellow
        after(480, note_on(103, 0)),           // tick 960: solo marker off
        note_on(99, 100),                      // tick 960: Blue
        end_of_track(),
    }));
}

}  // namespace

// In a .chart the `E soloend` event sits on the solo's last note, so that note
// is in the solo. The parser runs solo end after the notes at its tick.
TEST_CASE(".chart: the note on the solo end tick is in the solo") {
    Song song = load_songbytes_chart(solo_end_chart(), ChartReading{});
    REQUIRE(song.sequence.size() == 4);
    CHECK(song.sequence[2].flag_solo);
    CHECK_FALSE(song.sequence[3].flag_solo);
}

// The parser keeps each time signature as the chart wrote it, beside the
// measure length the engine reads. The length alone cannot tell 6/8 from 3/4.
TEST_CASE(".chart: time signatures are kept as written") {
    constexpr int64_t kQuarter = 192;  // the chart's resolution, ticks per quarter note
    const std::vector<uint8_t> data = testchart::chart_bytes(
        testchart::section("ExpertDrums", "  0 = N 0 0\n  1536 = N 1 0\n"), kQuarter, "",
        testchart::kSync44At120 + "  768 = TS 6 3\n  1344 = TS 3\n");
    Song song = load_songbytes_chart(data, ChartReading{});

    // A 6/8 measure is six eighth notes and a 3/4 measure three quarter
    // notes, the same length.
    const int64_t six_eighths = 6 * (kQuarter / 2);
    const int64_t three_quarters = 3 * kQuarter;
    CHECK(song.tpm_changes.at(768) == six_eighths);
    CHECK(song.tpm_changes.at(1344) == three_quarters);
    CHECK(song.timesig_changes.at(0) == std::make_pair(4, 4));
    CHECK(song.timesig_changes.at(768) == std::make_pair(6, 8));
    CHECK(song.timesig_changes.at(1344) == std::make_pair(3, 4));

    // A chart with no signature at all reads the default, 4/4 from tick 0.
    Song blank(480);
    CHECK(blank.timesig_changes.size() == 1);
    CHECK(blank.timesig_changes.at(0) == std::make_pair(4, 4));
}

// In a .mid the solo is a held marker note (103); its note-off tick is where
// the marker stops covering, so a note on that tick is outside the solo.
TEST_CASE(".mid: the note on the solo marker's note-off tick is outside the solo") {
    const Song song = load_songbytes_mid(solo_marker_mid(), ChartReading{});
    REQUIRE(song.sequence.size() == 3);
    CHECK(song.sequence[0].flag_solo);
    CHECK(song.sequence[1].flag_solo);
    CHECK_FALSE(song.sequence[2].flag_solo);
}

TEST_CASE("song: solo_sections lists each run of solo chords once") {
    // The two fixtures above: .chart chords 0 to 2 are in the solo, .mid
    // chords 0 and 1.
    const Song chart = load_songbytes_chart(solo_end_chart(), ChartReading{});
    REQUIRE(chart.solo_sections.size() == 1);
    CHECK(chart.solo_sections[0].first == 0);
    CHECK(chart.solo_sections[0].last == 2);

    const Song mid = load_songbytes_mid(solo_marker_mid(), ChartReading{});
    REQUIRE(mid.solo_sections.size() == 1);
    CHECK(mid.solo_sections[0].first == 0);
    CHECK(mid.solo_sections[0].last == 1);

    // Two solos split by one plain chord: chords 0 and 1, then chord 3.
    const Song two =
        load_songbytes_chart(testchart::chart_bytes(testchart::kTwoSolosDrums), ChartReading{});
    REQUIRE(two.sequence.size() == 4);
    REQUIRE(two.solo_sections.size() == 2);
    CHECK(two.solo_sections[0].first == 0);
    CHECK(two.solo_sections[0].last == 1);
    CHECK(two.solo_sections[1].first == 3);
    CHECK(two.solo_sections[1].last == 3);

    // No solo at all: no sections.
    const Song plain = load_songbytes_chart(
        testchart::chart_bytes(testchart::section("ExpertDrums", "  0 = N 1 0\n  192 = N 2 0\n")),
        ChartReading{});
    REQUIRE(plain.sequence.size() == 2);
    CHECK(plain.solo_sections.empty());
}

// ---- drum lanes (Song::drum_lanes; the pad rule is light_drum_lanes) ----

namespace {

using Pads = std::vector<NoteColor>;

// The lane of `song` that starts at `start`, or nullptr.
const DrumLane* lane_at(const Song& song, int64_t start) {
    const auto it = std::find_if(song.drum_lanes.begin(), song.drum_lanes.end(),
                                 [start](const DrumLane& l) { return l.start_tick == start; });
    return it == song.drum_lanes.end() ? nullptr : &*it;
}

Song lane_chart(const std::string& lines, bool pro = true, bool noteshuffle = false) {
    return load_songbytes_chart(testchart::chart_bytes(testchart::section("ExpertDrums", lines)),
                                ChartReading{Difficulty::Expert, pro, true, noteshuffle},
                                core::default_rules());
}

}  // namespace

namespace {

// Whether some chord of `song` from one tick before `lane` starts up to its
// end holds a note of colour `pad`.
bool lane_covers_pad(const Song& song, const DrumLane& lane, NoteColor pad) {
    for (const SongTimestamp& ts : song.sequence) {
        const int64_t t = ts.timecode.ticks();
        if (t < lane.start_tick - 1 || t >= lane.end_tick) continue;
        for (const ChordNote& n : ts.chord.notes())
            if (n.colortype == pad) return true;
    }
    return false;
}

// With Note Shuffle on, a lane lights every pad its notes land on (D254;
// light_drum_lanes in parse/song.cpp). So each pad colour is lit exactly when
// a chord under the lane in `song` holds it, read from the song's own chords,
// and the lit pads are in red-to-green order with each colour once.
void check_lane_lights_its_pads(const Song& song, const DrumLane& lane) {
    for (const NoteColor pad :
         {NoteColor::Red, NoteColor::Yellow, NoteColor::Blue, NoteColor::Green}) {
        const bool lit = std::find(lane.pads.begin(), lane.pads.end(), pad) != lane.pads.end();
        CHECK_MESSAGE(lit == lane_covers_pad(song, lane, pad),
                      "pad " << static_cast<int>(pad) << " of the lane at " << lane.start_tick);
    }
    CHECK(std::is_sorted(lane.pads.begin(), lane.pads.end()));
    CHECK(std::adjacent_find(lane.pads.begin(), lane.pads.end()) == lane.pads.end());
}

}  // namespace

// Every corpus .chart's lanes against its own S 65 and S 66 lines, and every
// corpus .mid's lanes against the marker notes in its drum track.
TEST_CASE("drum lanes: each corpus lane is a lane line or marker of its own file") {
    const std::string expert_drums = difficulty_chart_codes(Difficulty::Expert).chart_section();
    int chart_lanes = 0, mid_lanes = 0;
    for (const std::string& path : corpus_sweep()) {
        CAPTURE(path);
        const ChartFormat format = chart_format_of(path);
        const Song song = load_songpath(path, ChartReading{});
        if (format == ChartFormat::Chart) {
            // S 65 is a single lane and S 66 a double one, its length in
            // ticks. Every lane line in the corpus starts on a chord with a
            // pad, so each one is lit and none is dropped.
            const auto sections = chart_text_sections(path);
            const auto drums = sections.find(expert_drums);
            REQUIRE(drums != sections.end());
            size_t lines = 0;
            for (const ChartTextLine& l : drums->second) {
                if (l.words[0] != "S" || l.words.size() < 3) continue;
                if (l.words[1] != "65" && l.words[1] != "66") continue;
                ++lines;
                const int64_t start = std::stoll(l.key);
                const DrumLane* lane = lane_at(song, start);
                REQUIRE_MESSAGE(lane != nullptr, "no lane from the line at tick " << start);
                const int64_t line_end = start + std::stoll(l.words[2]);  // the line's tick plus its length
                CHECK(lane->end_tick == line_end);
                CHECK(lane->double_lane == (l.words[1] == "66"));
            }
            CHECK(song.drum_lanes.size() == lines);
            chart_lanes += static_cast<int>(lines);
        } else if (format == ChartFormat::Mid) {
            // Each lane starts on a marker note's note-on and ends on its
            // note-off, both read from the drum track by the file reader.
            const MidiFile file = MidiFile::from_file(path);
            const MidiTrack* track = file.drums_track();
            REQUIRE(track != nullptr);
            std::map<int, int64_t> open;  // marker pitch -> its note-on tick
            std::vector<std::pair<int64_t, int64_t>> markers;  // start, end
            int64_t tick = 0;
            for (const Message& m : track->messages) {
                tick += m.time;
                if (!is_midi_marker_pitch(m.note)) continue;
                if (m.is_note_on()) {
                    open.emplace(m.note, tick);
                } else if (m.type == Message::Type::NoteOn || m.type == Message::Type::NoteOff) {
                    const auto it = open.find(m.note);
                    if (it == open.end()) continue;
                    markers.emplace_back(it->second, tick);
                    open.erase(it);
                }
            }
            for (const DrumLane& lane : song.drum_lanes) {
                CAPTURE(lane.start_tick);
                const std::pair<int64_t, int64_t> span{lane.start_tick, lane.end_tick};
                const bool on_marker =
                    std::find(markers.begin(), markers.end(), span) != markers.end();
                CHECK_MESSAGE(on_marker, "lane ends at " << lane.end_tick);
            }
            mid_lanes += static_cast<int>(song.drum_lanes.size());
        }
        // A single lane lights one pad, a double lane at most two, and each
        // pad lit is a colour a chord under the lane holds.
        for (const DrumLane& lane : song.drum_lanes) {
            CAPTURE(lane.start_tick);
            CHECK(lane.pads.size() >= 1);
            CHECK(lane.pads.size() <= (lane.double_lane ? 2u : 1u));
            if (!lane.double_lane)
                for (const NoteColor pad : lane.pads) CHECK(lane_covers_pad(song, lane, pad));
        }
    }
    // The corpus holds lanes of both kinds of file (Apex and Victoria - Grow
    // among the .chart files, CHON - Book and Rush - YYZ among the .mid
    // files), so neither half of this test runs on nothing.
    CHECK(chart_lanes > 0);
    CHECK(mid_lanes > 0);
}

TEST_CASE(".chart: a double lane on a three-pad chord takes its second pad from the next chord") {
    // Red, yellow cymbal and blue tom, then a green tom. The first pad is the
    // three-pad chord's lowest, red. The second comes from the next chord,
    // as in the game (NL-55), so neither the yellow nor the blue is lit.
    const Song song = lane_chart("  0 = N 1 0\n  0 = N 2 0\n  0 = N 66 0\n  0 = N 3 0\n"
                                 "  0 = S 66 192\n  96 = N 4 0\n");
    REQUIRE(song.drum_lanes.size() == 1);
    CHECK(song.drum_lanes[0].double_lane);
    CHECK(song.drum_lanes[0].pads == Pads{NoteColor::Red, NoteColor::Green});

    // With Pro Drums off there are no cymbals; the second pad still comes
    // from the next chord.
    const Song plain = lane_chart("  0 = N 1 0\n  0 = N 2 0\n  0 = N 66 0\n  0 = N 3 0\n"
                                  "  0 = S 66 192\n  96 = N 4 0\n",
                                  /*pro=*/false);
    REQUIRE(plain.drum_lanes.size() == 1);
    CHECK(plain.drum_lanes[0].pads == Pads{NoteColor::Red, NoteColor::Green});
}

TEST_CASE(".chart: a double lane on a two-pad chord lights the next chord's highest pad") {
    // NL-55's example: red and yellow cymbal, then a blue tom. The game
    // lights red and blue.
    const Song song =
        lane_chart("  0 = N 1 0\n  0 = N 2 0\n  0 = N 66 0\n  0 = S 66 192\n  96 = N 3 0\n");
    REQUIRE(song.drum_lanes.size() == 1);
    CHECK(song.drum_lanes[0].pads == Pads{NoteColor::Red, NoteColor::Blue});
}

TEST_CASE(".chart: a double lane on a two-pad chord skips a lone kick") {
    // Red and yellow cymbal, a kick alone, then yellow tom and blue tom. The
    // kick lights no pad, and the highest of the next chord is blue.
    const Song song = lane_chart("  0 = N 1 0\n  0 = N 2 0\n  0 = N 66 0\n  0 = S 66 192\n"
                                 "  48 = N 0 0\n  96 = N 2 0\n  96 = N 3 0\n");
    REQUIRE(song.drum_lanes.size() == 1);
    CHECK(song.drum_lanes[0].pads == Pads{NoteColor::Red, NoteColor::Blue});
}

TEST_CASE(".chart: a double lane takes its second pad from a chord past the lane's end") {
    // Red and yellow cymbal under a lane from 0 to 96, then a green tom at
    // 192, after the lane has ended.
    const Song song =
        lane_chart("  0 = N 1 0\n  0 = N 2 0\n  0 = N 66 0\n  0 = S 66 96\n  192 = N 4 0\n");
    REQUIRE(song.drum_lanes.size() == 1);
    CHECK(song.drum_lanes[0].end_tick == 96);
    CHECK(song.drum_lanes[0].pads == Pads{NoteColor::Red, NoteColor::Green});
}

TEST_CASE(".chart: a double lane whose next chord repeats its first pad lights it once") {
    // Red and yellow cymbal, then a red: the second pad is red again, so the
    // lane lights red alone.
    const Song song =
        lane_chart("  0 = N 1 0\n  0 = N 2 0\n  0 = N 66 0\n  0 = S 66 192\n  96 = N 1 0\n");
    REQUIRE(song.drum_lanes.size() == 1);
    CHECK(song.drum_lanes[0].pads == Pads{NoteColor::Red});
}

TEST_CASE(".chart: a double lane on the song's last chord lights its first pad alone") {
    // Red and yellow cymbal and nothing after: with no later chord the lane
    // lights red alone (the user's choice, D333).
    const Song song = lane_chart("  0 = N 1 0\n  0 = N 2 0\n  0 = N 66 0\n  0 = S 66 192\n");
    REQUIRE(song.drum_lanes.size() == 1);
    CHECK(song.drum_lanes[0].pads == Pads{NoteColor::Red});
}

TEST_CASE(".chart: N 66, 67 and 68 mark the yellow, blue and green cymbals; N 65 marks none") {
    // One chord per marker, each over the four pads, then a chord with all
    // four markers. N 65 would be a red cymbal, which the game has no pad
    // for, so it changes nothing.
    const std::string lines =
        "  0 = N 1 0\n  0 = N 2 0\n  0 = N 3 0\n  0 = N 4 0\n  0 = N 65 0\n"
        "  96 = N 1 0\n  96 = N 2 0\n  96 = N 3 0\n  96 = N 4 0\n  96 = N 66 0\n"
        "  192 = N 1 0\n  192 = N 2 0\n  192 = N 3 0\n  192 = N 4 0\n  192 = N 67 0\n"
        "  288 = N 1 0\n  288 = N 2 0\n  288 = N 3 0\n  288 = N 4 0\n  288 = N 68 0\n"
        "  384 = N 1 0\n  384 = N 2 0\n  384 = N 3 0\n  384 = N 4 0\n"
        "  384 = N 65 0\n  384 = N 66 0\n  384 = N 67 0\n  384 = N 68 0\n";
    const Song pro = lane_chart(lines);
    REQUIRE(pro.sequence.size() == 5);
    CHECK(pro.sequence[0].chord.code() == ".nnnn");
    CHECK(pro.sequence[1].chord.code() == ".nNnn");
    CHECK(pro.sequence[2].chord.code() == ".nnNn");
    CHECK(pro.sequence[3].chord.code() == ".nnnN");
    CHECK(pro.sequence[4].chord.code() == ".nNNN");

    // With Pro Drums off every marker is ignored.
    const Song plain = lane_chart(lines, /*pro=*/false);
    REQUIRE(plain.sequence.size() == 5);
    for (const SongTimestamp& ts : plain.sequence) CHECK(ts.chord.code() == ".nnnn");
}

TEST_CASE(".chart: a double lane on a one-pad chord takes the next chord with a pad") {
    // Red, then a kick alone (skipped), then yellow tom and blue tom: the
    // highest of those is blue.
    const Song song = lane_chart("  0 = N 1 0\n  0 = S 66 192\n  48 = N 0 0\n"
                                 "  96 = N 2 0\n  96 = N 3 0\n");
    REQUIRE(song.drum_lanes.size() == 1);
    CHECK(song.drum_lanes[0].pads == Pads{NoteColor::Red, NoteColor::Blue});

    // Two reds in a row light one column, stored once.
    const Song same = lane_chart("  0 = N 1 0\n  0 = S 66 192\n  96 = N 1 0\n");
    REQUIRE(same.drum_lanes.size() == 1);
    CHECK(same.drum_lanes[0].pads == Pads{NoteColor::Red});

    // No later chord with a pad: only the first pad.
    const Song last = lane_chart("  0 = N 4 0\n  0 = S 66 192\n  96 = N 0 0\n");
    REQUIRE(last.drum_lanes.size() == 1);
    CHECK(last.drum_lanes[0].pads == Pads{NoteColor::Green});
}

TEST_CASE(".chart: a lane over kicks only, or over nothing, is dropped") {
    // Kicks inside the lane; the red at 192 sits on the lane's end, outside.
    const Song kicks =
        lane_chart("  0 = N 0 0\n  0 = S 65 192\n  96 = N 0 0\n  192 = N 1 0\n");
    CHECK(kicks.drum_lanes.empty());
    // A lane past the last note.
    const Song empty = lane_chart("  0 = N 1 0\n  384 = S 65 192\n");
    CHECK(empty.drum_lanes.empty());
}

TEST_CASE(".chart: a lane starts on the first pad from one tick before it to its end") {
    // A note one tick before the lane counts.
    const Song early = lane_chart("  99 = N 4 0\n  100 = S 65 50\n");
    REQUIRE(early.drum_lanes.size() == 1);
    CHECK(early.drum_lanes[0].pads == Pads{NoteColor::Green});
    // Two ticks before does not; the blue inside the lane does.
    const Song late = lane_chart("  98 = N 4 0\n  100 = S 65 50\n  140 = N 3 0\n");
    REQUIRE(late.drum_lanes.size() == 1);
    CHECK(late.drum_lanes[0].start_tick == 100);
    CHECK(late.drum_lanes[0].end_tick == 150);
    CHECK(late.drum_lanes[0].pads == Pads{NoteColor::Blue});
}

TEST_CASE(".chart: a lane shows on each difficulty whose section holds it") {
    const std::vector<uint8_t> data = testchart::chart_bytes(
        testchart::section("ExpertDrums", "  0 = N 1 0\n  0 = S 65 192\n") +
        testchart::section("HardDrums", "  0 = N 2 0\n"));
    CHECK(load_songbytes_chart(data, ChartReading{Difficulty::Expert}).drum_lanes.size() == 1);
    const Song hard = load_songbytes_chart(data, ChartReading{Difficulty::Hard});
    REQUIRE(hard.sequence.size() == 1);
    CHECK(hard.drum_lanes.empty());
}

TEST_CASE("drum lanes take their pads from the shuffled notes with Note Shuffle on") {
    // Every corpus chart is read with Note Shuffle off and on. The lanes sit
    // on the same ticks either way, because the game's lane holds the notes
    // the shuffle recolours (D112's 2026-10-17 correction). With the shuffle
    // on, each lane's pads are those of the shuffled notes under it, taken
    // from the shuffled song's own chords (check_lane_lights_its_pads).
    size_t lanes = 0;
    for (const std::string& path : corpus_sweep()) {
        CAPTURE(path);
        const Song plain = load_songpath(path, ChartReading{});
        std::optional<Song> shuffled;
        try {
            shuffled = load_songpath(
                path, ChartReading{Difficulty::Expert, true, true, /*noteshuffle=*/true},
                core::default_rules());
        } catch (const KindedError& e) {
            // A chart the game freezes on has no shuffled song to compare
            // (the freeze test below); any other error fails here.
            REQUIRE(e.kind() == ErrorKind::NoteShuffleFreezes);
            continue;
        }
        REQUIRE(shuffled->drum_lanes.size() == plain.drum_lanes.size());
        for (size_t i = 0; i < plain.drum_lanes.size(); ++i) {
            CAPTURE(i);
            const DrumLane& lane = shuffled->drum_lanes[i];
            CHECK(lane.start_tick == plain.drum_lanes[i].start_tick);
            CHECK(lane.end_tick == plain.drum_lanes[i].end_tick);
            CHECK(lane.double_lane == plain.drum_lanes[i].double_lane);
            check_lane_lights_its_pads(*shuffled, lane);
        }
        lanes += shuffled->drum_lanes.size();
    }
    // The lane-line test above finds lanes in both kinds of corpus file, so
    // this walk checks some.
    CHECK(lanes > 0);
}

TEST_CASE(".chart: with Note Shuffle off, a lane over several pads keeps the game's pads") {
    // Red, yellow cymbal, blue tom and green cymbal in turn, under a single
    // lane and then a double lane. The game's rule picks from the first notes
    // only, so the later pads stay dark (D112).
    const std::string notes = "  0 = N 1 0\n  48 = N 2 0\n  48 = N 66 0\n  96 = N 3 0\n"
                              "  144 = N 4 0\n  144 = N 68 0\n  192 = N 1 0\n";
    const Song single = lane_chart(notes + "  0 = S 65 240\n");
    REQUIRE(single.drum_lanes.size() == 1);
    CHECK(single.drum_lanes[0].pads == Pads{NoteColor::Red});
    const Song twin = lane_chart(notes + "  0 = S 66 240\n");
    REQUIRE(twin.drum_lanes.size() == 1);
    CHECK(twin.drum_lanes[0].pads == Pads{NoteColor::Red, NoteColor::Yellow});
}

TEST_CASE(".chart: with Note Shuffle on, a single lane over a red roll lights every pad") {
    // A red snare roll under one single lane. Note Shuffle spreads the roll
    // over other pads, and the lane lights each pad the shuffled notes under
    // it land on (D254), not only the first.
    std::string lines = "  0 = S 65 768\n";
    for (int64_t tick = 0; tick < 768; tick += 48) lines += testchart::line(tick, "N 1 0");
    const Song song = lane_chart(lines, /*pro=*/true, /*noteshuffle=*/true);
    REQUIRE(song.drum_lanes.size() == 1);
    const DrumLane& lane = song.drum_lanes[0];
    // The shuffle moved some of the roll off red, so a lane showing only its
    // first pad fails below.
    const bool spread = lane_covers_pad(song, lane, NoteColor::Yellow) ||
                        lane_covers_pad(song, lane, NoteColor::Blue) ||
                        lane_covers_pad(song, lane, NoteColor::Green);
    REQUIRE(spread);
    check_lane_lights_its_pads(song, lane);
}

TEST_CASE("Song::note_count: the chart's note total under each 2x Bass setting") {
    // Tick 0: a kick, a 2x kick on the same tick, and a red. Tick 192: a lone
    // 2x kick. Tick 384: a yellow. Tick 576: a 2x kick written before a kick
    // on the same tick.
    const std::vector<uint8_t> data = testchart::chart_bytes(testchart::section(
        "ExpertDrums",
        "  0 = N 0 0\n  0 = N 32 0\n  0 = N 1 0\n"
        "  192 = N 32 0\n"
        "  384 = N 2 0\n"
        "  576 = N 32 0\n  576 = N 0 0\n"));
    // Kicks on one tick merge into one kick that keeps the 2x mark (NL-29),
    // so ticks 0, 192 and 576 each hold one 2x kick.
    const int kick_ticks = 3;
    // The red at tick 0 and the yellow at tick 384.
    const int pad_notes = 2;
    CHECK(load_songbytes_chart(data, ChartReading{}).note_count() == kick_ticks + pad_notes);
    // 2x Bass off removes every 2x kick (NL-27), and each merged kick is one.
    CHECK(load_songbytes_chart(data, ChartReading{Difficulty::Expert, true, /*bass2x=*/false})
              .note_count() == pad_notes);
    // A chart with no notes counts none.
    CHECK(Song(192).note_count() == 0);
}

TEST_CASE(".mid: EVENTS text metas become practice sections") {
    using namespace testmidi;
    const std::vector<uint8_t> tempo_track =
        concat({track_name("tempo"), set_tempo(), end_of_track()});
    const std::vector<uint8_t> events_track = concat({
        track_name("EVENTS"),
        text_event("[section Intro]"),
        after(384, text_event("[prc_verse_1]")),
        after(384, text_event("[crowd_realtime]")),
        end_of_track(),
    });
    const std::vector<uint8_t> drums_track = concat({
        track_name("PART DRUMS"),
        note_on(96, 100),  // Expert kick
        note_off(96),
        end_of_track(),
    });

    Song song = load_songbytes_mid(smf_tracks({tempo_track, events_track, drums_track}),
                                   ChartReading{});
    REQUIRE(song.sequence.size() == 1);
    REQUIRE(song.practice_sections.size() == 2);
    CHECK(song.practice_sections[0].tick == 0);
    CHECK(song.practice_sections[0].name == "Intro");
    CHECK(song.practice_sections[1].tick == 384);
    // verse_1 is in Clone Hero's list as "Verse 1".
    CHECK(song.practice_sections[1].name == "Verse 1");
}

namespace {
std::vector<uint8_t> chart_with(int64_t resolution, const std::string& sync) {
    return testchart::chart_bytes(testchart::section("ExpertDrums", "  0 = N 0 0\n  768 = N 1 0\n"),
                                  resolution, "", sync);
}
}  // namespace

// Timing that would make a measure or beat last zero, negative or infinite
// time is refused at load with a plain error naming the tick (D6). A time
// signature with a top number of 0 is the one line that is ignored instead.
TEST_CASE(".chart: a timing line that can't measure time is refused") {
    const std::string ok = "  0 = TS 4\n  0 = B 120000\n";
    CHECK_NOTHROW(load_songbytes_chart(chart_with(192, ok), ChartReading{}));
    struct Bad {
        const char* sync;
        const char* error;
    };
    for (const Bad& bad :
         {Bad{"  0 = TS 4\n  0 = B 0\n", "the tempo at tick 0 is not above 0 BPM"},
          Bad{"  0 = TS 4\n  0 = B 120000\n  384 = B -60000\n",
              "the tempo at tick 384 is not above 0 BPM"},
          Bad{"  0 = TS -3\n  0 = B 120000\n",
              "the time signature at tick 0 makes a measure -576 ticks long"},
          // A beat shorter than one tick (4096 > 192 x 4) is refused where the
          // signature is read, before its measure length is checked.
          Bad{"  0 = TS 1 12\n  0 = B 120000\n",
              "the time signature at tick 0 has a bottom number that is out of range"},
          // 4/1024 at 192 makes a 3-tick measure, but its beat is under a tick.
          Bad{"  0 = TS 4\n  0 = B 120000\n  384 = TS 4 10\n",
              "the time signature at tick 384 has a bottom number that is out of range"},
          Bad{"  0 = TS 4\n  0 = B 120000\n  384 = TS 4 40\n",
              "the time signature at tick 384 has a bottom number that is out of range"},
          Bad{"  0 = TS 4\n  0 = B 120000\n  384 = TS 4 -1\n",
              "the time signature at tick 384 has a bottom number that is out of range"}}) {
        CAPTURE(bad.sync);
        CHECK_THROWS_WITH_AS(load_songbytes_chart(chart_with(192, bad.sync), ChartReading{}),
                             bad.error, ChartFileError);
    }
    CHECK_THROWS_WITH_AS(load_songbytes_chart(chart_with(0, ok), ChartReading{}),
                         "the chart's resolution is 0, and it must be above 0",
                         ChartFileError);
}

TEST_CASE(".chart: a TS 0 timing line is still ignored") {
    Song song = load_songbytes_chart(
        chart_with(192, "  0 = TS 4\n  0 = B 120000\n  768 = TS 0\n"), ChartReading{});
    CHECK(song.tpm_changes.size() == 1);
    CHECK(song.tpm_changes.at(0) == 768);
    // Ignored whatever its bottom number says.
    Song odd = load_songbytes_chart(
        chart_with(192, "  0 = TS 4\n  0 = B 120000\n  768 = TS 0 40\n"), ChartReading{});
    CHECK(odd.tpm_changes.size() == 1);
}

TEST_CASE(".mid: a timing line that can't measure time") {
    auto track = [](std::vector<uint8_t> timing) {
        return testmidi::smf_drums(testmidi::concat({testmidi::track_name("PART DRUMS"),
                                                     testmidi::note_on(96, 100),
                                                     testmidi::end_of_track()}),
                                   testmidi::concat({testmidi::set_tempo(), timing}));
    };
    // TS 0/4: ignored, the 4/4 default stays (finding 100's crash).
    Song song = load_songbytes_mid(track({0x00, 0xFF, 0x58, 0x04, 0x00, 0x02, 0x18, 0x08}),
                                   ChartReading{});
    CHECK(song.tpm_changes.at(0) == 480 * 4);
    // A 0 us tempo would be an infinite BPM: refused, and the message says so
    // (D12) instead of calling it "not above 0 BPM".
    CHECK_THROWS_WITH_AS(load_songbytes_mid(track(testmidi::set_tempo(0)), ChartReading{}),
                         "the tempo at tick 0 is infinite (0 microseconds per beat)",
                         ChartFileError);
    // A denominator exponent of 40: refused, never shifted.
    CHECK_THROWS_WITH_AS(
        load_songbytes_mid(track({0x00, 0xFF, 0x58, 0x04, 0x04, 40, 0x18, 0x08}), ChartReading{}),
        "the time signature at tick 0 has a bottom number that is out of range",
        ChartFileError);
}

// A division of 0 ticks per beat and a negative .chart resolution cannot place
// any note in time. Both were already refused (through check_timing_maps);
// this pins it.
TEST_CASE("a resolution that is 0 or negative is refused in both formats") {
    std::vector<uint8_t> mid = testmidi::smf_drums(testmidi::concat(
        {testmidi::track_name("PART DRUMS"), testmidi::note_on(96, 100),
         testmidi::end_of_track()}));
    mid[12] = 0x00;  // division = 0
    mid[13] = 0x00;
    CHECK_THROWS_WITH_AS(load_songbytes_mid(mid, ChartReading{}),
                         "the chart's resolution is 0, and it must be above 0", ChartFileError);
    CHECK_THROWS_WITH_AS(
        load_songbytes_chart(chart_with(-192, "  0 = TS 4\n  0 = B 120000\n"), ChartReading{}),
        "the chart's resolution is -192, and it must be above 0", ChartFileError);
}

TEST_CASE("mid: kick velocity is read as ghost/accent, like a pad's") {
    // Clone Hero prices a velocity-1 kick as a ghost and a velocity-127 kick
    // as an accent, both worth double.
    auto drums_track = [](bool dynamics) {
        std::vector<std::vector<uint8_t>> ev;
        ev.push_back(testmidi::track_name("PART DRUMS"));
        if (dynamics)
            ev.push_back(testmidi::text_event("[ENABLE_CHART_DYNAMICS]"));
        ev.push_back(testmidi::note_on(96, 1));     // tick 0:   kick, vel 1
        ev.push_back({0x40, 0x90, 95, 127});        // tick 64:  2x kick, vel 127
        ev.push_back({0x40, 0x90, 97, 1});          // tick 128: Red pad, vel 1
        ev.push_back(testmidi::end_of_track());
        return testmidi::smf_drums(testmidi::concat(ev));
    };

    SUBCASE("with [ENABLE_CHART_DYNAMICS]") {
        Song song = load_songbytes_mid(drums_track(true), ChartReading{});
        REQUIRE(song.sequence.size() == 3);

        const auto& kick = song.sequence[0].chord.at(NoteColor::Kick);
        REQUIRE(kick.has_value());
        CHECK(kick->dynamictype == NoteDynamicType::Ghost);
        CHECK(kick->is2x == false);
        CHECK(kick->str() == "Kick (Ghost)");

        const auto& kick2x = song.sequence[1].chord.at(NoteColor::Kick);
        REQUIRE(kick2x.has_value());
        CHECK(kick2x->dynamictype == NoteDynamicType::Accent);
        CHECK(kick2x->is2x == true);
        CHECK(kick2x->str() == "2x kick (Accent)");

        const auto& red = song.sequence[2].chord.at(NoteColor::Red);
        REQUIRE(red.has_value());
        CHECK(red->dynamictype == NoteDynamicType::Ghost);

        check_invariants(song, "kick dynamics fixture");
    }

    SUBCASE("without the text event, every note is Normal") {
        Song song = load_songbytes_mid(drums_track(false), ChartReading{});
        REQUIRE(song.sequence.size() == 3);
        CHECK(song.sequence[0].chord.at(NoteColor::Kick)->dynamictype ==
              NoteDynamicType::Normal);
        CHECK(song.sequence[1].chord.at(NoteColor::Kick)->dynamictype ==
              NoteDynamicType::Normal);
        CHECK(song.sequence[1].chord.at(NoteColor::Kick)->is2x == true);
        CHECK(song.sequence[2].chord.at(NoteColor::Red)->dynamictype ==
              NoteDynamicType::Normal);
        check_invariants(song, "kick dynamics fixture (no marker)");
    }
}

TEST_CASE("mid: Won't Get Fooled Again (O) reads each kick's velocity as its dynamic") {
    // The real chart's kicks, read straight from its drum track's note-ons,
    // against the parser's kicks, tick by tick.
    const std::string path =
        corpus::testdata_root() + "/midi/wgfa_onyxite/notes.mid";
    Song song = load_songpath(path, ChartReading{Difficulty::Expert});
    REQUIRE(!song.is_empty());
    // The dynamics tag comes before every marked note, so every kick's
    // velocity counts (Song::dynamics_late_tag_tick).
    REQUIRE(song.dynamics_enabled);
    REQUIRE(!song.dynamics_late_tag_tick.has_value());

    // Each tick's kick and 2x kick note-on velocities, from the file.
    const DifficultyChartCodes& codes = difficulty_chart_codes(Difficulty::Expert);
    const MidiFile file = MidiFile::from_file(path);
    const MidiTrack* track = file.drums_track();
    REQUIRE(track != nullptr);
    std::map<int64_t, std::vector<int>> kick_velocities;
    int64_t tick = 0;
    for (const Message& m : track->messages) {
        tick += m.time;
        if (m.is_note_on() && (m.note == codes.kick_pitch || m.note == codes.kick2x_pitch()))
            kick_velocities[tick].push_back(m.velocity);
    }

    // Clone Hero reads a kick's velocity as a pad's: 127 an accent, 1 a
    // ghost (NL-10). Kicks on one tick merge with both marks (NL-29), and a
    // note with both plays as an accent (NL-16). The NL rules are on
    // docs/clone-hero/02-note-list.md.
    int ghosts = 0;
    for (const SongTimestamp& ts : song.sequence) {
        const auto& kick = ts.chord.at(NoteColor::Kick);
        if (!kick.has_value()) continue;
        const int64_t at = ts.timecode.ticks();
        CAPTURE(at);
        const auto it = kick_velocities.find(at);
        REQUIRE(it != kick_velocities.end());
        const std::vector<int>& vels = it->second;
        const bool accent = std::find(vels.begin(), vels.end(), 127) != vels.end();
        const bool ghost = std::find(vels.begin(), vels.end(), 1) != vels.end();
        const NoteDynamicType want = accent  ? NoteDynamicType::Accent
                                     : ghost ? NoteDynamicType::Ghost
                                             : NoteDynamicType::Normal;
        CHECK(kick->dynamictype == want);
        if (want == NoteDynamicType::Ghost) ++ghosts;
        kick_velocities.erase(it);
    }
    // Every kick in the file is a kick in the song.
    CHECK(kick_velocities.empty());
    // The chart holds ghost kicks, so a ghost kick the parser loses fails above.
    CHECK(ghosts > 0);

    // Every chord code still round-trips, ghost kicks included. The fill check
    // is skipped: this chart has a fill that starts on its own last note, so
    // its activation length is 0.
    check_invariants(song, path, FillCheck::Skip);
}

TEST_CASE("mid: the marker pitch table is the one list the parser reads") {
    // Expert's pads, from its kick pitch (difficulty_chart_codes): yellow,
    // blue and green at tick 0, then a red alone at 480 and at 960. Each
    // marker has a note under it to act on: a tom marker a cymbal pad, the
    // flam marker a lone note, and the solo, SP, fill and lane markers
    // (NL-02 on docs/clone-hero/02-note-list.md) any note.
    const int kick = difficulty_chart_codes(Difficulty::Expert).kick_pitch;
    const auto pad = [kick](int above_kick) { return static_cast<uint8_t>(kick + above_kick); };
    const auto chart = [&pad](std::optional<int> marker) {
        std::vector<std::vector<uint8_t>> ev;
        ev.push_back(testmidi::track_name("PART DRUMS"));
        if (marker) ev.push_back(testmidi::note_on(static_cast<uint8_t>(*marker), 100));
        ev.push_back(testmidi::note_on(pad(2), 100));
        ev.push_back(testmidi::note_on(pad(3), 100));
        ev.push_back(testmidi::note_on(pad(4), 100));
        ev.push_back(testmidi::after(480, testmidi::note_on(pad(1), 100)));
        ev.push_back(testmidi::after(480, testmidi::note_on(pad(1), 100)));
        if (marker) ev.push_back(testmidi::note_off(static_cast<uint8_t>(*marker)));
        ev.push_back(testmidi::end_of_track());
        return load_songbytes_mid(testmidi::smf_drums(testmidi::concat(ev)),
                                  ChartReading{Difficulty::Expert, true, true});
    };
    const uint64_t plain = digest::song_digest(chart(std::nullopt));

    // No marker sits below the lowest one and no pad at or above it
    // (song.cpp's static_asserts beside the table), so from there up a pitch
    // changes the song exactly when the table holds it.
    int lowest = 0;
    while (lowest <= 127 && !is_midi_marker_pitch(lowest)) ++lowest;
    REQUIRE(lowest <= 127);
    for (int pitch = lowest; pitch <= 127; ++pitch) {
        const bool read = digest::song_digest(chart(pitch)) != plain;
        CHECK_MESSAGE(read == is_midi_marker_pitch(pitch), "pitch " << pitch);
    }
}

TEST_CASE("chart_files: loose-folder notes names match in any case") {
    CHECK(notes_file_format("notes.mid") == ChartFormat::Mid);
    CHECK(notes_file_format("NOTES.MID") == ChartFormat::Mid);
    CHECK(notes_file_format("Notes.Chart") == ChartFormat::Chart);
    CHECK(notes_file_format("notes.sng") == ChartFormat::None);
    CHECK(notes_file_format("mynotes.mid") == ChartFormat::None);
    CHECK(is_song_ini("song.ini"));
    CHECK(is_song_ini("Song.INI"));
    CHECK_FALSE(is_song_ini("song.ini.bak"));
}

TEST_CASE("chart_files: a path's format comes from its extension in any case") {
    CHECK(chart_format_of("C:\\songs\\a\\notes.mid") == ChartFormat::Mid);
    CHECK(chart_format_of("x.CHART") == ChartFormat::Chart);
    CHECK(chart_format_of("C:\\songs\\bundle.SNG") == ChartFormat::Sng);
    CHECK(chart_format_of("pack.Srb") == ChartFormat::Srb);
    CHECK(chart_format_of("notes.txt") == ChartFormat::None);
    CHECK(chart_format_of("mid") == ChartFormat::None);
}

TEST_CASE("mid: a stray SP note-off flags nothing") {
    std::vector<std::vector<uint8_t>> ev;
    ev.push_back(testmidi::track_name("PART DRUMS"));
    ev.push_back(testmidi::note_on(116, 100));       // tick 0:   SP phrase starts
    ev.push_back(testmidi::note_on(96, 100));        // tick 0:   kick, inside the phrase
    ev.push_back({0x81, 0x70, 0x80, 116, 0});        // tick 240: SP phrase ends
    ev.push_back({0x81, 0x70, 0x90, 96, 100});       // tick 480: kick, outside any phrase
    ev.push_back({0x81, 0x70, 0x80, 116, 0});        // tick 720: stray SP note-off
    ev.push_back(testmidi::end_of_track());
    Song song = load_songbytes_mid(testmidi::smf_drums(testmidi::concat(ev)), ChartReading{});

    REQUIRE(song.sequence.size() == 2);
    CHECK(song.sequence[0].flag_sp);
    CHECK_FALSE(song.sequence[1].flag_sp);
}

TEST_CASE(".chart: [Song] Offset is read in seconds") {
    std::string text = multidiff::chart_text();
    const std::string at = "  Resolution = 192\n";
    text.insert(text.find(at) + at.size(), "  Offset = 0.25\n");
    const std::vector<uint8_t> data(text.begin(), text.end());

    Song song = load_songbytes_chart(data, ChartReading{});
    REQUIRE(song.chart_offset_s.has_value());
    CHECK(*song.chart_offset_s == doctest::Approx(0.25));

    Song plain = load_songbytes_chart(multidiff::chart_bytes(), ChartReading{});
    CHECK_FALSE(plain.chart_offset_s.has_value());
}

// ---- the parsers' hand matchers against pinned tables ----------------------
//
// The parsers match the disco-flip markers and the .chart section header by
// hand, for speed, and must agree with std::regex patterns that are not in the
// code. The disco and section-header cases read their expected answers from
// tables pinned from one run of those patterns. The dynamics tag is Clone
// Hero's two exact strings (finding 64, D24), so its cases check against those
// two strings. Every case drives the real parsers with strings built around
// every byte value, so any difference in what matches shows up.

namespace {

// For one disco marker shape: the bytes that turn the flip on and the bytes
// that turn it off. Any other byte leaves the flip as it was.
struct ByteRange {
    int lo, hi;
};
struct DiscoShapeAnswer {
    std::vector<ByteRange> on, off;
};

bool in_ranges(const std::vector<ByteRange>& ranges, int byte) {
    for (const ByteRange& r : ranges)
        if (byte >= r.lo && byte <= r.hi) return true;
    return false;
}

// One row per shape in disco_candidates, in its order, for both readers.
// Both readers take their text through decode_chart_text
// (parse/text_decode.h), so a lone byte from 0x80 up reaches the matcher as a
// character of two or three bytes and never fills a one-character spot.
const DiscoShapeAnswer kDiscoAnswers[9] = {
    {{{0x00, 0x09}, {0x0B, 0x0C}, {0x0E, 0x7F}}, {}},
    {{{0x00, 0x09}, {0x0B, 0x0C}, {0x0E, 0x7F}}, {}},
    {{{0x30, 0x39}}, {}},
    {{{0x64, 0x64}}, {{0x30, 0x39}, {0x5D, 0x5D}}},
    {{}, {{0x30, 0x39}}},
    {{{0x5D, 0x5D}}, {}},
    {{{0x5B, 0x5B}}, {}},
    {{{0x64, 0x64}}, {{0x5D, 0x5D}}},
    {{{0x5B, 0x5B}}, {}},
};

bool want_flip(const DiscoShapeAnswer& a, int byte, bool prior_on) {
    if (in_ranges(a.on, byte)) return true;
    if (in_ranges(a.off, byte)) return false;
    return prior_on;
}

// Disco markers with byte `b` placed in every spot the regexes care about.
std::vector<std::string> disco_candidates(char b) {
    const std::string c(1, b);
    return {"mix" + c + "3_drums0d", "mix_3" + c + "drums0d", "mix_3_drums" + c + "d",
            "mix_3_drums" + c, "mix_3_drums" + c + "dnoflip", "[mix_3_drums0d" + c,
            c + "mix_3_drums0d", "mix_3_drums0" + c, c + "mix_3_drums0d]"};
}

}  // namespace

TEST_CASE(".chart: disco markers match the pinned tables") {
    int checked = 0;
    for (int byte = 0; byte < 256; ++byte) {
        const char b = static_cast<char>(byte);
        // A .chart event word never holds whitespace or '=' (both split it).
        if (std::isspace(static_cast<unsigned char>(b)) || b == '=') continue;
        const std::vector<std::string> markers = disco_candidates(b);
        for (size_t shape = 0; shape < markers.size(); ++shape) {
            const std::string& marker = markers[shape];
            for (bool prior_on : {false, true}) {
                const std::vector<uint8_t> data = testchart::chart_bytes(testchart::section(
                    "ExpertDrums", std::string(prior_on ? "  0 = E mix_3_drums0d\n" : "") +
                                       "  0 = N 0 0\n  192 = E " + marker + "\n  192 = N 1 0\n"));
                const Song song = load_songbytes_chart(data, ChartReading{});
                REQUIRE(song.sequence.size() == 2);

                // A flipped red pad reads as a yellow cymbal.
                const bool flipped = song.sequence[1].chord.at(NoteColor::Yellow).has_value();
                CHECK_MESSAGE(flipped == want_flip(kDiscoAnswers[shape], byte, prior_on),
                              "byte " << byte << ", shape " << shape << ", prior " << prior_on);
                ++checked;
            }
        }
    }
    CHECK(checked > 4000);
}

TEST_CASE(".mid: disco markers match the pinned tables") {
    using namespace testmidi;
    int checked = 0;
    for (int byte = 0; byte < 256; ++byte) {
        const char b = static_cast<char>(byte);
        const std::vector<std::string> markers = disco_candidates(b);
        for (size_t shape = 0; shape < markers.size(); ++shape) {
            const std::string& marker = markers[shape];
            for (bool prior_on : {false, true}) {
                std::vector<std::vector<uint8_t>> ev = {track_name("PART DRUMS")};
                if (prior_on) ev.push_back(text_event("mix_3_drums0d"));
                ev.push_back(note_on(96, 100));                    // tick 0: kick
                ev.push_back(after(480, text_event(marker)));  // tick 480
                ev.push_back(note_on(97, 100));                    // tick 480: red
                ev.push_back(end_of_track());
                const Song song = load_songbytes_mid(smf_drums(concat(ev)), ChartReading{});
                REQUIRE(song.sequence.size() == 2);
                const bool flipped = song.sequence[1].chord.at(NoteColor::Yellow).has_value();
                CHECK_MESSAGE(flipped == want_flip(kDiscoAnswers[shape], byte, prior_on),
                              "byte " << byte << ", shape " << shape << ", prior " << prior_on);
                ++checked;
            }
        }
    }
    // The dynamics marker's two exact spellings are pinned as literal lists in
    // "dynamics tag: only Clone Hero's two exact spellings count"
    // (test_dynamics_tag.cpp).
    CHECK(checked > 4000);
}

TEST_CASE(".chart: section headers are found as the pinned table says") {
    // The first line outside a section must hold a `[name]`. No header throws
    // ChartFileError; a header that is not [Song] leaves the chart without its
    // [Song] section, which is also a ChartFileError (no Resolution). Each
    // line's outcome is pinned from one run of a std::regex pattern.
    enum class Outcome { NoHeader, SongHeader, OtherHeader };
    struct HeaderCase {
        const char* line;
        Outcome outcome;
    };
    const HeaderCase cases[] = {
        {"[Song]", Outcome::SongHeader},
        {"x[Song]", Outcome::SongHeader},
        {"[Song]x", Outcome::SongHeader},
        {"[Song] [x]", Outcome::OtherHeader},
        {"[[Song]", Outcome::OtherHeader},
        {"[Song]]", Outcome::OtherHeader},
        {"[So\rng]", Outcome::NoHeader},
        {"[Song\r]", Outcome::NoHeader},
        {"[x\r][Song]", Outcome::SongHeader},
        {"[x\r]]", Outcome::NoHeader},
        {"]Song[", Outcome::NoHeader},
        {"[", Outcome::NoHeader},
        {"Song", Outcome::NoHeader},
        {"[]", Outcome::OtherHeader},
        {"\x01[Song]", Outcome::SongHeader},
        {"[\r[Song]", Outcome::SongHeader},
        {"[a\r[b]", Outcome::OtherHeader},
        {"[Song]\r]", Outcome::SongHeader},
        {"[x]\r[Song]", Outcome::OtherHeader},
        {"\xC3\xA9[Song]\xC3\xA9", Outcome::SongHeader},
        {"[S\xC3\xA9]", Outcome::OtherHeader},
    };
    for (const HeaderCase& c : cases) {
        const std::string raw = c.line;
        CAPTURE(raw);
        const std::string text = raw + "\n{\n  Resolution = 192\n}\n"
                                       "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n";
        const std::vector<uint8_t> data(text.begin(), text.end());
        switch (c.outcome) {
            case Outcome::NoHeader:
                CHECK_THROWS_AS(load_songbytes_chart(data, ChartReading{}), ChartFileError);
                break;
            case Outcome::SongHeader:
                CHECK(load_songbytes_chart(data, ChartReading{}).tick_resolution() == 192);
                break;
            case Outcome::OtherHeader:
                CHECK_THROWS_AS(load_songbytes_chart(data, ChartReading{}), ChartFileError);
                break;
        }
    }
}

TEST_CASE("the no-notes error carries no_notes_message's sentence") {
    const NoNotesError err(Difficulty::Hard, true);
    CHECK(std::string(err.what()) == "No Hard Pro Drums notes in this chart.");
    // Callers that catch any chart-file problem still catch this one.
    CHECK_THROWS_AS(throw NoNotesError(Difficulty::Hard, true), ChartFileError);
}

TEST_CASE("load-and-check throws the no-notes error for a missing difficulty") {
    const std::string no_hard = corpus::first_chart_without_notes(Difficulty::Hard);
    const std::string has_hard = corpus::first_chart_with_notes(Difficulty::Hard);
    CAPTURE(no_hard);
    CAPTURE(has_hard);

    CHECK_THROWS_WITH_AS(load_songpath_with_notes(no_hard, ChartReading{Difficulty::Hard}),
                         "No Hard Pro Drums notes in this chart.", NoNotesError);
    CHECK_NOTHROW(load_songpath_with_notes(has_hard, ChartReading{Difficulty::Hard}));
}

// ---- the Note Shuffle switch on the loaders (D104) ---------------------------

namespace {

const std::string kShuffleSongs =
    (sourcetree::root() / "docs/audit/note-shuffle/game-tests/songs/").u8string();

Song load_shuffle_song(const std::string& file, bool pro, bool bass2x, bool noteshuffle) {
    return load_songpath(kShuffleSongs + file,
                         ChartReading{Difficulty::Expert, pro, bass2x, noteshuffle});
}

}  // namespace

TEST_CASE("Note Shuffle switch: a game-confirmed song loads shuffled on and as written off") {
    const std::string a1 = "NS A1 tick scale chart192/notes.chart";
    // The game-confirmed A1 run with Pro Drums on, the literal test_note_shuffle
    // pins from the reference.
    const std::vector<std::string> shuffled = {
        "nn...", "..n..", "...n.", "..n..", "n..n.", "n....", "..n..", "...n.",
        "nn...", "....n", "..N..", "...n.", "n.N..", "....N", "...n.", "n.n.."};
    CHECK(chord_codes(load_shuffle_song(a1, true, false, true)) == shuffled);

    const std::vector<std::string> off = chord_codes(load_shuffle_song(a1, true, false, false));
    CHECK(off == chord_codes(load_songpath(kShuffleSongs + a1,
                                           ChartReading{Difficulty::Expert, true, false, false})));
    CHECK(off != shuffled);

    // The app's switch reaches the analysis settings.
    app::Settings s;
    CHECK_FALSE(s.to_analysis_settings().noteshuffle);
    s.view_noteshuffle = true;
    CHECK(s.to_analysis_settings().noteshuffle);
}

TEST_CASE("Note Shuffle switch: a chart the game freezes on gives the freeze error") {
    const std::string c1 = "NS C1 four colour then snare/notes.mid";
    const char* sentence = "Clone Hero freezes loading this chart with Note Shuffle on.";
    for (const bool pro : {true, false}) {
        CAPTURE(pro);
        bool threw = false;
        try {
            load_shuffle_song(c1, pro, false, true);
        } catch (const KindedError& e) {
            threw = true;
            CHECK(e.kind() == ErrorKind::NoteShuffleFreezes);
            CHECK(app::plain_error(e) == sentence);
        }
        CHECK(threw);
        CHECK_NOTHROW(load_shuffle_song(c1, pro, false, false));
    }

    // Analysis shows it the way it shows an unreadable chart: the error
    // escapes analyze_chart_file, and its sentence is the freeze's.
    app::Settings s;
    s.view_noteshuffle = true;
    bool threw = false;
    try {
        app::analyze_chart_file(kShuffleSongs + c1, s.to_analysis_settings());
    } catch (const std::exception& e) {
        threw = true;
        CHECK(app::plain_error(e) == sentence);
    }
    CHECK(threw);
}

TEST_CASE("Note Shuffle switch: the Preview's song key differs when only the switch differs") {
    const ui::PreviewSongKey off{"md5", {Difficulty::Expert, true, true, false}, kTestFillRule};
    ui::PreviewSongKey on = off;
    on.reading.noteshuffle = true;
    CHECK(off != on);
    CHECK(off == ui::PreviewSongKey{"md5", {Difficulty::Expert, true, true}, kTestFillRule});
}

TEST_CASE("Note Shuffle switch: the Dynamics pads follow the analysis's shuffle") {
    // The pad rows count the notes of the song read shuffled, and the kick
    // rows the notes of the song read as written with its 2x kicks kept, each
    // counted here from the song's own chords. Which row a note belongs in is
    // app::dynamics_row_for's rule.
    using Rows = std::array<app::DynamicsCounts, static_cast<size_t>(app::DynamicsRow::Count)>;
    const auto rows_from_chords = [](const Song& song) {
        Rows rows{};
        for (const SongTimestamp& ts : song.sequence) {
            for (const ChordNote& note : ts.chord.note_list()) {
                app::DynamicsCounts& c = rows[static_cast<size_t>(app::dynamics_row_for(note))];
                if (note.dynamictype == NoteDynamicType::Ghost) ++c.ghost;
                else if (note.dynamictype == NoteDynamicType::Accent) ++c.accent;
                else ++c.normal;
            }
        }
        return rows;
    };
    const std::string a2 = "NS A2 tick scale mid960/notes.mid";
    const std::string path = kShuffleSongs + a2;
    const Rows kicks =
        rows_from_chords(load_shuffle_song(a2, true, app::kDynamicsParseBass2x, false));
    for (const bool bass2x : {true, false}) {
        CAPTURE(bass2x);
        const app::DynamicsBreakdown bd =
            app::dynamics_for_settings(path, {Difficulty::Expert, true, bass2x, true}, nullptr);
        const Rows pads = rows_from_chords(load_shuffle_song(a2, true, bass2x, true));
        for (size_t i = 0; i < bd.rows.size(); ++i) {
            CAPTURE(i);
            const bool pad = app::is_pad_row(static_cast<app::DynamicsRow>(i));
            CHECK(bd.rows[i] == (pad ? pads[i] : kicks[i]));
        }
    }
}

// ---- the crafted edge files (testdata/parse_edge, speedups task P1) ----
//
// gen_edge.py writes 41 small charts, each one odd or broken in its own way,
// with a comment above each saying what it tests. This test reads every file
// listed in list.txt under two settings and checks what that comment says,
// against the well-formed file the others vary (c01_base.chart for .chart,
// m01_base.mid for .mid): the file parses to the same song as the base, or to
// a different one, or fails with the error the comment names. No saved digest
// or saved failure text is read. A failure's type comes from the reader that
// throws it, and files that must fail the same way are compared with each
// other.

namespace {

std::string edge_dir() { return corpus::testdata_root() + "/parse_edge/"; }

// What reading one file at one setting gave: its digest, and the failure text
// (the exception's type, then its message) when it did not parse.
struct EdgeResult {
    uint64_t hash = 0;
    std::string fail;
    bool parsed() const { return fail.empty(); }
};

EdgeResult read_edge(const std::string& file, const app::BatchRun& run) {
    EdgeResult r;
    r.hash = digest::chart_parse_hash(edge_dir() + file, run.settings, &r.fail);
    return r;
}

// The failure text is the throwing exception's type name, then ": ".
template <class E>
bool fails_with(const EdgeResult& r) {
    const std::string type = std::string(typeid(E).name()) + ": ";
    return !r.parsed() && r.fail.compare(0, type.size(), type) == 0;
}

bool fails_in(const EdgeResult& r, const char* what) {
    return !r.parsed() && r.fail.find(what) != std::string::npos;
}

// The file names list.txt holds, in order.
std::vector<std::string> edge_listed() {
    std::vector<std::string> listed;
    std::ifstream list(edge_dir() + "list.txt", std::ios::binary);
    std::string name;
    while (std::getline(list, name)) {
        if (!name.empty() && name.back() == '\r') name.pop_back();
        if (!name.empty()) listed.push_back(name);
    }
    return listed;
}

// One setting's check of every listed file. `expert` is true for the Expert
// run. Each check names its file; a listed file no check names fails the
// closing count.
void check_edge_files(const app::BatchRun& run, bool expert) {
    const std::vector<std::string> listed = edge_listed();
    REQUIRE(!listed.empty());
    std::map<std::string, EdgeResult> got;
    for (const std::string& f : listed) got[f] = read_edge(f, run);
    std::set<std::string> named;

    // File names are std::string so doctest prints the name, not an address.
    auto of = [&](const std::string& f) -> const EdgeResult& {
        const auto it = got.find(f);
        REQUIRE_MESSAGE(it != got.end(), f << " is not in list.txt");
        named.insert(f);
        return it->second;
    };
    const EdgeResult chart_base = of("c01_base.chart");
    const EdgeResult mid_base = of("m01_base.mid");
    REQUIRE_MESSAGE(chart_base.parsed(), chart_base.fail);
    REQUIRE_MESSAGE(mid_base.parsed(), mid_base.fail);

    auto same_as = [&](const std::string& f, const EdgeResult& base) {
        const EdgeResult& r = of(f);
        CHECK_MESSAGE(r.parsed(), f << " failed: " << r.fail);
        CHECK_MESSAGE(r.hash == base.hash, f << " parses to a different song than its base");
    };
    auto differs_from = [&](const std::string& f, const EdgeResult& base) {
        const EdgeResult& r = of(f);
        CHECK_MESSAGE(r.parsed(), f << " failed: " << r.fail);
        CHECK_MESSAGE(r.hash != base.hash, f << " parses to the same song as its base");
    };
    auto parses = [&](const std::string& f) {
        const EdgeResult& r = of(f);
        CHECK_MESSAGE(r.parsed(), f << " failed: " << r.fail);
    };
    auto same_failure = [&](const std::string& a, const std::string& b) {
        CHECK_MESSAGE(of(a).fail == of(b).fail, a << " and " << b << " fail differently");
    };
    auto other_failure = [&](const std::string& a, const std::string& b) {
        CHECK_MESSAGE(of(a).fail != of(b).fail, a << " and " << b << " fail the same way");
    };

    // A bad or overflowing number refuses the chart wherever it sits, in a
    // section the reader skips or not. std::stoi and std::stoll name which
    // call read it: a note value or meter is an int, a tick or length is 64
    // bits.
    CHECK(fails_with<std::invalid_argument>(of("c02_bad_number_skipped.chart")));
    CHECK(fails_in(of("c02_bad_number_skipped.chart"), "stoi"));
    CHECK(fails_with<std::out_of_range>(of("c03_overflow_skipped.chart")));
    CHECK(fails_in(of("c03_overflow_skipped.chart"), "stoi"));
    CHECK(fails_with<std::out_of_range>(of("c04_overflow_length_skipped.chart")));
    CHECK(fails_in(of("c04_overflow_length_skipped.chart"), "stoll"));
    CHECK(fails_with<std::invalid_argument>(of("c05_bad_number_drums.chart")));
    CHECK(fails_in(of("c05_bad_number_drums.chart"), "stoi"));
    CHECK(fails_with<std::out_of_range>(of("c06_overflow_drums.chart")));
    CHECK(fails_in(of("c06_overflow_drums.chart"), "stoll"));
    CHECK(fails_with<std::out_of_range>(of("c07_int_overflow_drums.chart")));
    CHECK(fails_in(of("c07_int_overflow_drums.chart"), "stoi"));
    CHECK(fails_with<std::invalid_argument>(of("c08_bad_number_events.chart")));
    CHECK(fails_in(of("c08_bad_number_events.chart"), "stoll"));
    CHECK(fails_with<std::out_of_range>(of("c09_overflow_events.chart")));
    CHECK(fails_in(of("c09_overflow_events.chart"), "stoi"));
    CHECK(fails_with<std::invalid_argument>(of("c10_bad_number_song.chart")));
    CHECK(fails_in(of("c10_bad_number_song.chart"), "stoi"));
    CHECK(fails_with<std::out_of_range>(of("c11_overflow_song.chart")));
    CHECK(fails_in(of("c11_overflow_song.chart"), "stoll"));
    // The same bad value fails the same way whichever section it is in.
    same_failure("c02_bad_number_skipped.chart", "c05_bad_number_drums.chart");
    same_failure("c03_overflow_skipped.chart", "c07_int_overflow_drums.chart");
    same_failure("c04_overflow_length_skipped.chart", "c06_overflow_drums.chart");

    // Keys that stoll reads whole are ticks, so Expert gains notes. The Hard
    // section is the base's.
    if (expert) differs_from("c12_odd_keys.chart", chart_base);
    else same_as("c12_odd_keys.chart", chart_base);
    // Number spellings that parse change both difficulties and the sync track.
    differs_from("c13_odd_numbers.chart", chart_base);
    // The later copy of every section replaces the earlier one.
    differs_from("c14_duplicate_sections.chart", chart_base);
    // The Expert drum section is never closed: the file ends inside it, and
    // Expert has no notes the reader keeps. Hard is the base's.
    if (expert) CHECK(fails_with<NoNotesError>(of("c15_unterminated_drums.chart")));
    else same_as("c15_unterminated_drums.chart", chart_base);
    // An unclosed second copy leaves the first copy standing.
    same_as("c16_unterminated_second_copy.chart", chart_base);
    // A section left open swallows the next header: Expert ends with no notes
    // the same way, and Hard reads a different set of lines.
    if (expert) {
        CHECK(fails_with<NoNotesError>(of("c17_header_inside_section.chart")));
        same_failure("c17_header_inside_section.chart", "c15_unterminated_drums.chart");
    } else {
        differs_from("c17_header_inside_section.chart", chart_base);
    }
    parses("c18_unsorted_ticks.chart");
    // CRLF line ends, a stray CR, tabs and trailing spaces change nothing.
    same_as("c19_crlf.chart", chart_base);
    differs_from("c20_disco.chart", chart_base);

    // A chart without a Resolution is refused as a chart file, with or
    // without a [Song] section, and the same way; a Resolution with no digits
    // is refused differently.
    CHECK(fails_with<ChartFileError>(of("c21_missing_song.chart")));
    CHECK(fails_with<ChartFileError>(of("c22_missing_resolution.chart")));
    same_failure("c21_missing_song.chart", "c22_missing_resolution.chart");
    differs_from("c23_resolution_junk.chart", chart_base);
    CHECK(fails_with<ChartFileError>(of("c24_bad_resolution.chart")));
    other_failure("c24_bad_resolution.chart", "c22_missing_resolution.chart");
    CHECK(fails_with<ChartFileError>(of("c25_bad_header.chart")));
    differs_from("c26_header_junk.chart", chart_base);
    differs_from("c27_empty_values.chart", chart_base);
    // Tempo and meter lines in the Expert drum section never run there, but
    // the notes and phrase around them are Expert's. Hard is the base's.
    if (expert) differs_from("c28_timing_in_drums.chart", chart_base);
    else same_as("c28_timing_in_drums.chart", chart_base);

    // .mid files. A second drum track is ignored, and so is an unknown chunk.
    same_as("m02_two_drum_tracks.mid", mid_base);
    differs_from("m03_late_names.mid", mid_base);
    // A meta over the size cap fails the file, even in a track the song
    // parser never reads.
    CHECK(fails_with<MidiError>(of("m04_oversize_meta_other.mid")));
    CHECK(fails_with<MidiError>(of("m05_oversize_meta_drums.mid")));
    same_failure("m04_oversize_meta_other.mid", "m05_oversize_meta_drums.mid");
    // Format 0 holds one track, and the reader takes no instrument from the
    // first track (D253, midi.h), so neither difficulty has drum notes. Expert
    // fails as the unclosed chart section does.
    CHECK(fails_with<NoNotesError>(of("m06_format0.mid")));
    if (expert) same_failure("m06_format0.mid", "c15_unterminated_drums.chart");
    CHECK(fails_with<MidiError>(of("m07_smpte.mid")));
    other_failure("m07_smpte.mid", "m04_oversize_meta_other.mid");
    differs_from("m08_truncated.mid", mid_base);
    differs_from("m09_running_status_after_meta.mid", mid_base);
    same_as("m10_unknown_chunk.mid", mid_base);
    // A status byte the reader cannot size stops the drum track there:
    // Expert loses the notes after it, and Hard's come before it.
    if (expert) differs_from("m11_bad_status.mid", mid_base);
    else same_as("m11_bad_status.mid", mid_base);
    CHECK(fails_with<ChartFileError>(of("m12_zero_tempo.mid")));
    differs_from("m13_clipped_and_short_metas.mid", mid_base);

    // Every listed file has a check above.
    for (const std::string& f : listed) CHECK_MESSAGE(named.count(f) == 1, f << " has no check");
}

}  // namespace

TEST_CASE("the crafted edge files parse as each one's generator comment says") {
    SUBCASE("Expert, Pro Drums, 2x Bass") {
        check_edge_files(digest::digest_settings("Expert", true, true).batch_run(), true);
    }
    SUBCASE("Hard, Pro Drums off, 2x Bass off") {
        check_edge_files(digest::digest_settings("Hard", false, false).batch_run(), false);
    }
}
