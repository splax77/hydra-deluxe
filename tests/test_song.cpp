// Smoke test for parse/song: every chart in the corpus parses without
// throwing, and the resulting timestamp sequence holds the structural
// invariants the search depends on (monotone ticks, decodable chords,
// consistent activation-fill placement). Covers .mid, .chart, and .sng.

#include "doctest.h"

#include <cctype>
#include <cstdint>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include "core/model.h"
#include "core/strutil.h"
#include "corpus_util.h"
#include "midi_util.h"
#include "multidiff_chart.h"
#include "parse/chart_files.h"
#include "parse/song.h"

#ifndef HYDRA_TESTDATA_DIR
#error "HYDRA_TESTDATA_DIR must be defined (see CMakeLists.txt)"
#endif

using namespace hydra;

namespace {

// The structural invariants the search depends on, checked on one parsed song.
// Returns false (with a message naming `what`) on the first violation.
void check_invariants(const Song& song, const std::string& what) {
    bool ticks_ok = true, codes_ok = true, fills_ok = true;
    int64_t prev = -1;
    for (const SongTimestamp& ts : song.sequence) {
        // Strictly increasing: two chords never share a tick.
        if (ts.timecode.ticks() <= prev) ticks_ok = false;
        prev = ts.timecode.ticks();

        // Every chord code decodes back to the same chord.
        const std::string code = ts.chord.code();
        if (code.empty() || Chord::from_code(code).code() != code) codes_ok = false;

        // An activation fill length is always positive.
        if (ts.activation_length.has_value() && *ts.activation_length <= 0)
            fills_ok = false;
    }
    CHECK_MESSAGE(ticks_ok, what << ": ticks not strictly increasing");
    CHECK_MESSAGE(codes_ok, what << ": chord code round-trip");
    CHECK_MESSAGE(fills_ok, what << ": non-positive activation fill");
}

}  // namespace

TEST_CASE("song parse holds its invariants over the corpus") {
    int charts = 0, nonempty = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true);
        ++charts;
        if (song.is_empty()) continue;
        ++nonempty;
        check_invariants(song, path);
    }

    CHECK(nonempty > 0);
    MESSAGE("checked " << charts << " charts (" << nonempty << " non-empty)");
}

TEST_CASE("song parse holds its invariants at Hard too") {
    // Same sweep at Hard. Not every chart has Hard charting (an empty song is
    // the honest answer there), but plenty do, and whatever parses must hold
    // exactly the same structure Expert does.
    int charts = 0, nonempty = 0, mids = 0, mids_nonempty = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true, Difficulty::Hard);
        ++charts;
        const bool is_mid = ends_with(path, ".mid");
        if (is_mid) ++mids;
        if (song.is_empty()) continue;
        ++nonempty;
        if (is_mid) ++mids_nonempty;
        check_invariants(song, path + " [Hard]");
    }

    // The corpus has 18 .chart files with a [HardDrums] section, so this is a
    // real sweep and not a silently-empty one.
    CHECK(nonempty > 0);
    // .mid charts keep every difficulty in the one "PART DRUMS" track, so a
    // corpus .mid with Hard notes proves the pitch base works end to end.
    CHECK(mids_nonempty > 0);
    MESSAGE("checked " << charts << " charts at Hard (" << nonempty << " non-empty, "
                       << mids_nonempty << "/" << mids << " .mid)");
}

TEST_CASE(".chart: each difficulty reads its own section") {
    const std::vector<uint8_t> data = multidiff::chart_bytes();

    Song expert = load_songbytes_chart(data, true, true);
    Song hard = load_songbytes_chart(data, true, true, Difficulty::Hard);
    Song medium = load_songbytes_chart(data, true, true, Difficulty::Medium);
    Song easy = load_songbytes_chart(data, true, true, Difficulty::Easy);

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
    Song hard_nonpro = load_songbytes_chart(data, false, true, Difficulty::Hard);
    CHECK(hard_nonpro.sequence[1].chord.code() != hard.sequence[1].chord.code());

    check_invariants(hard, "multidiff [Hard]");
    check_invariants(easy, "multidiff [Easy]");
}

TEST_CASE(".mid: each difficulty reads its own pitch base") {
    // Corpus .mid files that carry Hard notes parse to a different sequence
    // than their Expert one — the pitch base is what selects the difficulty.
    int compared = 0, differed = 0;
    for (const std::string& path : corpus::chart_paths()) {
        if (!ends_with(path, ".mid")) continue;
        Song expert = load_songpath_mid(path, true, true);
        Song hard = load_songpath_mid(path, true, true, Difficulty::Hard);
        if (hard.is_empty()) continue;
        ++compared;
        if (hard.sequence.size() != expert.sequence.size()) ++differed;
        check_invariants(hard, path + " [Hard]");
        // Medium and Easy sit at their own bases and must not throw either.
        check_invariants(load_songpath_mid(path, true, true, Difficulty::Medium),
                         path + " [Medium]");
        check_invariants(load_songpath_mid(path, true, true, Difficulty::Easy),
                         path + " [Easy]");
        if (compared >= 5) break;  // a handful is enough; the sweep covers the rest
    }
    CHECK(compared > 0);
    // Hard is a reduction of Expert, so the two streams are not the same one
    // read twice. (Per-chart the counts could coincide; across the sample they
    // cannot all coincide.)
    CHECK(differed > 0);
}

TEST_CASE(".chart: [Events] section markers become practice sections") {
    const std::string text =
        "[Song]\n"
        "{\n"
        "  Resolution = 192\n"
        "}\n"
        "[SyncTrack]\n"
        "{\n"
        "  0 = TS 4\n"
        "  0 = B 120000\n"
        "}\n"
        "[Events]\n"
        "{\n"
        "  1536 = E \"prc_chorus_1a\"\n"
        "  0 = E \"section Intro\"\n"
        "  768 = E \"section Verse 1\"\n"
        "  1920 = E \"lighting (blackout)\"\n"
        "}\n"
        "[ExpertDrums]\n"
        "{\n"
        "  0 = N 0 0\n"
        "  192 = N 1 0\n"
        "}\n";
    const std::vector<uint8_t> data(text.begin(), text.end());
    Song song = load_songbytes_chart(data, true, true);

    // Both spellings are read, non-section text events are not, and the file's
    // own order does not have to be sorted.
    REQUIRE(song.practice_sections.size() == 3);
    CHECK(song.practice_sections[0].tick == 0);
    CHECK(song.practice_sections[0].name == "Intro");
    CHECK(song.practice_sections[1].tick == 768);
    CHECK(song.practice_sections[1].name == "Verse 1");
    CHECK(song.practice_sections[2].tick == 1536);
    CHECK(song.practice_sections[2].name == "chorus_1a");

    // A chart with no [Events] section has no sections and still parses.
    Song plain = load_songbytes_chart(multidiff::chart_bytes(), true, true);
    CHECK(plain.practice_sections.empty());
}

// In a .chart the `E soloend` event sits on the solo's last note, so that note
// is in the solo. The parser runs solo end after the notes at its tick.
TEST_CASE(".chart: the note on the solo end tick is in the solo") {
    const std::string text =
        "[Song]\n{\n  Resolution = 192\n}\n"
        "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n"
        "[ExpertDrums]\n{\n"
        "  0 = E solo\n  0 = N 1 0\n"
        "  192 = N 2 0\n"
        "  384 = N 3 0\n  384 = E soloend\n"
        "  576 = N 4 0\n"
        "}\n";
    const std::vector<uint8_t> data(text.begin(), text.end());
    Song song = load_songbytes_chart(data, true, true);
    REQUIRE(song.sequence.size() == 4);
    CHECK(song.sequence[2].flag_solo);
    CHECK_FALSE(song.sequence[3].flag_solo);
}

// The parser keeps each time signature as the chart wrote it, beside the
// measure length the engine reads. The length alone cannot tell 6/8 from 3/4.
TEST_CASE(".chart: time signatures are kept as written") {
    const std::string text =
        "[Song]\n{\n  Resolution = 192\n}\n"
        "[SyncTrack]\n{\n"
        "  0 = TS 4\n  0 = B 120000\n"
        "  768 = TS 6 3\n"
        "  1344 = TS 3\n"
        "}\n"
        "[ExpertDrums]\n{\n  0 = N 0 0\n  1536 = N 1 0\n}\n";
    const std::vector<uint8_t> data(text.begin(), text.end());
    Song song = load_songbytes_chart(data, true, true);

    // 6/8 and 3/4 are both 576 ticks at 192 per quarter note.
    CHECK(song.tpm_changes.at(768) == 576);
    CHECK(song.tpm_changes.at(1344) == 576);
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
    using namespace testmidi;
    // Delta 480 (one beat at 480 tpqn) as a two-byte variable-length number.
    const std::vector<uint8_t> beat = {0x83, 0x60};
    auto at_beat = [&](std::vector<uint8_t> ev) {  // replace the leading 0 delta
        ev.erase(ev.begin());
        std::vector<uint8_t> out = beat;
        out.insert(out.end(), ev.begin(), ev.end());
        return out;
    };
    const std::vector<uint8_t> track = concat({
        track_name("PART DRUMS"), set_tempo(),
        note_on(103, 100), note_on(97, 100),   // tick 0: solo on, Red
        at_beat(note_on(98, 100)),             // tick 480: Yellow
        at_beat(note_on(103, 0)),              // tick 960: solo marker off
        note_on(99, 100),                      // tick 960: Blue
        end_of_track(),
    });
    Song song = load_songbytes_mid(smf(track), true, true);
    REQUIRE(song.sequence.size() == 3);
    CHECK(song.sequence[0].flag_solo);
    CHECK(song.sequence[1].flag_solo);
    CHECK_FALSE(song.sequence[2].flag_solo);
}

namespace {

void put_varlen(std::vector<uint8_t>& out, uint32_t v) {
    uint8_t stack[5];
    int n = 0;
    do {
        stack[n++] = static_cast<uint8_t>(v & 0x7F);
        v >>= 7;
    } while (v != 0);
    while (n > 0) {
        --n;
        out.push_back(static_cast<uint8_t>(stack[n] | (n > 0 ? 0x80 : 0x00)));
    }
}

void put_meta(std::vector<uint8_t>& out, uint32_t delta, uint8_t type,
              const std::string& payload) {
    put_varlen(out, delta);
    out.push_back(0xFF);
    out.push_back(type);
    put_varlen(out, static_cast<uint32_t>(payload.size()));
    out.insert(out.end(), payload.begin(), payload.end());
}

void put_bytes(std::vector<uint8_t>& out, std::initializer_list<int> bytes) {
    for (int b : bytes) out.push_back(static_cast<uint8_t>(b));
}

void put_track(std::vector<uint8_t>& file, const std::vector<uint8_t>& events) {
    const char* tag = "MTrk";
    file.insert(file.end(), tag, tag + 4);
    uint32_t len = static_cast<uint32_t>(events.size());
    for (int shift = 24; shift >= 0; shift -= 8)
        file.push_back(static_cast<uint8_t>((len >> shift) & 0xFF));
    file.insert(file.end(), events.begin(), events.end());
}

}  // namespace

TEST_CASE(".mid: EVENTS text metas become practice sections") {
    std::vector<uint8_t> tempo_track;
    put_meta(tempo_track, 0, 0x03, "tempo");
    put_varlen(tempo_track, 0);  // set_tempo 500000 us/qn = 120 BPM
    put_bytes(tempo_track, {0xFF, 0x51, 0x03, 0x07, 0xA1, 0x20});
    put_meta(tempo_track, 0, 0x2F, "");

    std::vector<uint8_t> events_track;
    put_meta(events_track, 0, 0x03, "EVENTS");
    put_meta(events_track, 0, 0x01, "[section Intro]");
    put_meta(events_track, 384, 0x01, "[prc_verse_1]");
    put_meta(events_track, 384, 0x01, "[crowd_realtime]");
    put_meta(events_track, 0, 0x2F, "");

    std::vector<uint8_t> drums_track;
    put_meta(drums_track, 0, 0x03, "PART DRUMS");
    put_bytes(drums_track, {0x00, 0x90, 0x60, 0x64});  // note_on 96, Expert kick
    put_bytes(drums_track, {0x00, 0x80, 0x60, 0x00});
    put_meta(drums_track, 0, 0x2F, "");

    std::vector<uint8_t> file;
    put_bytes(file, {'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 1, 0, 3, 0, 192});
    put_track(file, tempo_track);
    put_track(file, events_track);
    put_track(file, drums_track);

    Song song = load_songbytes_mid(file, true, true);
    REQUIRE(song.sequence.size() == 1);
    REQUIRE(song.practice_sections.size() == 2);
    CHECK(song.practice_sections[0].tick == 0);
    CHECK(song.practice_sections[0].name == "Intro");
    CHECK(song.practice_sections[1].tick == 384);
    CHECK(song.practice_sections[1].name == "verse_1");
}

namespace {
std::vector<uint8_t> chart_with(const std::string& resolution, const std::string& sync) {
    std::string s = "[Song]\n{\n  Resolution = " + resolution + "\n}\n"
                    "[SyncTrack]\n{\n" + sync + "}\n"
                    "[ExpertDrums]\n{\n  0 = N 0 0\n  768 = N 1 0\n}\n";
    return std::vector<uint8_t>(s.begin(), s.end());
}
}  // namespace

// Timing that would make a measure or beat last zero, negative or infinite
// time is refused at load with a plain error naming the tick (D6). A time
// signature with a top number of 0 is the one line that is ignored instead.
TEST_CASE(".chart: a timing line that can't measure time is refused") {
    const std::string ok = "  0 = TS 4\n  0 = B 120000\n";
    CHECK_NOTHROW(load_songbytes_chart(chart_with("192", ok), true, true));
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
          Bad{"  0 = TS 1 12\n  0 = B 120000\n",
              "the time signature at tick 0 makes a measure 0 ticks long"},
          Bad{"  0 = TS 4\n  0 = B 120000\n  384 = TS 4 40\n",
              "the time signature at tick 384 has a bottom number that is out of range"},
          Bad{"  0 = TS 4\n  0 = B 120000\n  384 = TS 4 -1\n",
              "the time signature at tick 384 has a bottom number that is out of range"}}) {
        CAPTURE(bad.sync);
        CHECK_THROWS_WITH_AS(load_songbytes_chart(chart_with("192", bad.sync), true, true),
                             bad.error, ChartFileError);
    }
    CHECK_THROWS_WITH_AS(load_songbytes_chart(chart_with("0", ok), true, true),
                         "the chart's resolution is 0, and it must be above 0",
                         ChartFileError);
}

TEST_CASE(".chart: a TS 0 timing line is still ignored") {
    Song song = load_songbytes_chart(
        chart_with("192", "  0 = TS 4\n  0 = B 120000\n  768 = TS 0\n"), true, true);
    CHECK(song.tpm_changes.size() == 1);
    CHECK(song.tpm_changes.at(0) == 768);
    // Ignored whatever its bottom number says.
    Song odd = load_songbytes_chart(
        chart_with("192", "  0 = TS 4\n  0 = B 120000\n  768 = TS 0 40\n"), true, true);
    CHECK(odd.tpm_changes.size() == 1);
}

TEST_CASE(".mid: a timing line that can't measure time") {
    auto track = [](std::vector<uint8_t> timing) {
        return testmidi::smf(testmidi::concat({testmidi::track_name("PART DRUMS"),
                                               testmidi::set_tempo(), timing,
                                               testmidi::note_on(96, 100),
                                               testmidi::end_of_track()}));
    };
    // TS 0/4: ignored, the 4/4 default stays (finding 100's crash).
    Song song = load_songbytes_mid(track({0x00, 0xFF, 0x58, 0x04, 0x00, 0x02, 0x18, 0x08}),
                                   true, true);
    CHECK(song.tpm_changes.at(0) == 480 * 4);
    // A 0 us tempo would be an infinite BPM: refused, and the message says so
    // (D12) instead of calling it "not above 0 BPM".
    CHECK_THROWS_WITH_AS(load_songbytes_mid(track(testmidi::set_tempo(0)), true, true),
                         "the tempo at tick 0 is infinite (0 microseconds per beat)",
                         ChartFileError);
    // A denominator exponent of 40: refused, never shifted.
    CHECK_THROWS_WITH_AS(
        load_songbytes_mid(track({0x00, 0xFF, 0x58, 0x04, 0x04, 40, 0x18, 0x08}), true, true),
        "the time signature at tick 0 has a bottom number that is out of range",
        ChartFileError);
}

// A division of 0 ticks per beat and a negative .chart resolution cannot place
// any note in time. Both were already refused (through check_timing_maps);
// this pins it.
TEST_CASE("a resolution that is 0 or negative is refused in both formats") {
    std::vector<uint8_t> mid = testmidi::smf(testmidi::concat(
        {testmidi::track_name("PART DRUMS"), testmidi::set_tempo(),
         testmidi::note_on(96, 100), testmidi::end_of_track()}));
    mid[12] = 0x00;  // division = 0
    mid[13] = 0x00;
    CHECK_THROWS_WITH_AS(load_songbytes_mid(mid, true, true),
                         "the chart's resolution is 0, and it must be above 0", ChartFileError);
    CHECK_THROWS_WITH_AS(
        load_songbytes_chart(chart_with("-192", "  0 = TS 4\n  0 = B 120000\n"), true, true),
        "the chart's resolution is -192, and it must be above 0", ChartFileError);
}

TEST_CASE("mid: kick velocity is read as ghost/accent, like a pad's") {
    // Clone Hero prices a velocity-1 kick as a ghost and a velocity-127 kick
    // as an accent, both worth double. Hydra used to hand every kick Normal.
    auto drums_track = [](bool dynamics) {
        std::vector<std::vector<uint8_t>> ev;
        ev.push_back(testmidi::track_name("PART DRUMS"));
        ev.push_back(testmidi::set_tempo());
        if (dynamics)
            ev.push_back(testmidi::text_event("[ENABLE_CHART_DYNAMICS]"));
        ev.push_back(testmidi::note_on(96, 1));     // tick 0:   kick, vel 1
        ev.push_back({0x40, 0x90, 95, 127});        // tick 64:  2x kick, vel 127
        ev.push_back({0x40, 0x90, 97, 1});          // tick 128: Red pad, vel 1
        ev.push_back(testmidi::end_of_track());
        return testmidi::smf(testmidi::concat(ev));
    };

    SUBCASE("with [ENABLE_CHART_DYNAMICS]") {
        Song song = load_songbytes_mid(drums_track(true), true, true);
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
        CHECK(kick2x->str() == "Kick (Accent, 2x)");

        const auto& red = song.sequence[2].chord.at(NoteColor::Red);
        REQUIRE(red.has_value());
        CHECK(red->dynamictype == NoteDynamicType::Ghost);

        check_invariants(song, "kick dynamics fixture");
    }

    SUBCASE("without the text event, every note is Normal") {
        Song song = load_songbytes_mid(drums_track(false), true, true);
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

TEST_CASE("mid: Won't Get Fooled Again (O) has 36 ghost kicks") {
    // The chart that proved the bug: Hydra called it 1,134,335 while a real FC
    // on Hydra's own path scored 1,142,235. 36 velocity-1 kicks, no accents.
    const std::string path =
        std::string(HYDRA_TESTDATA_DIR) + "/midi/wgfa_onyxite/notes.mid";
    Song song = load_songpath(path, true, true, Difficulty::Expert);
    REQUIRE(!song.is_empty());

    int ghost = 0, accent = 0, kicks = 0;
    for (const SongTimestamp& ts : song.sequence) {
        const auto& kick = ts.chord.at(NoteColor::Kick);
        if (!kick.has_value()) continue;
        ++kicks;
        if (kick->dynamictype == NoteDynamicType::Ghost) ++ghost;
        if (kick->dynamictype == NoteDynamicType::Accent) ++accent;
    }
    CHECK(kicks > 36);
    CHECK(ghost == 36);
    CHECK(accent == 0);

    // Every chord code still round-trips, which is what the new ghost-kick
    // codes have to prove. (The full check_invariants sweep is not used here:
    // this chart has a fill that starts on its own last note, so its
    // activation length is 0 — true before this change and unrelated to it.)
    bool codes_ok = true, ticks_ok = true;
    int64_t prev = -1;
    for (const SongTimestamp& ts : song.sequence) {
        if (ts.timecode.ticks() <= prev) ticks_ok = false;
        prev = ts.timecode.ticks();
        const std::string code = ts.chord.code();
        if (code.empty() || Chord::from_code(code).code() != code) codes_ok = false;
    }
    CHECK(codes_ok);
    CHECK(ticks_ok);
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
    ev.push_back(testmidi::set_tempo());
    ev.push_back(testmidi::note_on(116, 100));       // tick 0:   SP phrase starts
    ev.push_back(testmidi::note_on(96, 100));        // tick 0:   kick, inside the phrase
    ev.push_back({0x81, 0x70, 0x80, 116, 0});        // tick 240: SP phrase ends
    ev.push_back({0x81, 0x70, 0x90, 96, 100});       // tick 480: kick, outside any phrase
    ev.push_back({0x81, 0x70, 0x80, 116, 0});        // tick 720: stray SP note-off
    ev.push_back(testmidi::end_of_track());
    Song song = load_songbytes_mid(testmidi::smf(testmidi::concat(ev)), true, true);

    REQUIRE(song.sequence.size() == 2);
    CHECK(song.sequence[0].flag_sp);
    CHECK_FALSE(song.sequence[1].flag_sp);
}

TEST_CASE(".chart: [Song] Offset is read in seconds") {
    std::string text = multidiff::chart_text();
    const std::string at = "  Resolution = 192\n";
    text.insert(text.find(at) + at.size(), "  Offset = 0.25\n");
    const std::vector<uint8_t> data(text.begin(), text.end());

    Song song = load_songbytes_chart(data, true, true);
    REQUIRE(song.chart_offset_s.has_value());
    CHECK(*song.chart_offset_s == doctest::Approx(0.25));

    Song plain = load_songbytes_chart(multidiff::chart_bytes(), true, true);
    CHECK_FALSE(plain.chart_offset_s.has_value());
}

// ---- the parsers' hand matchers against the regexes they replaced ----------
//
// The parsers used std::regex for the disco-flip markers and the .chart section
// header, and still match them exactly as those regexes did. The dynamics tag
// is now Clone Hero's two exact strings (finding 64). All of them match by
// hand now, for speed. These cases keep the old regexes as the oracle and
// drive the real parsers with strings built around every byte value, so any
// difference in what matches shows up.

namespace {

const std::regex& oracle_disco_on() {
    static const std::regex r(R"(\[?mix.3.drums\d?d\]?)");
    return r;
}
const std::regex& oracle_disco_off() {
    static const std::regex r(R"(\[?mix.3.drums\d?(dnoflip)?\]?)");
    return r;
}

// Disco markers with byte `b` placed in every spot the regexes care about.
std::vector<std::string> disco_candidates(char b) {
    const std::string c(1, b);
    return {"mix" + c + "3_drums0d", "mix_3" + c + "drums0d", "mix_3_drums" + c + "d",
            "mix_3_drums" + c, "mix_3_drums" + c + "dnoflip", "[mix_3_drums0d" + c,
            c + "mix_3_drums0d", "mix_3_drums0" + c, c + "mix_3_drums0d]"};
}

// The MIDI reader stores each text byte as latin-1 decoded to UTF-8.
std::string latin1_to_utf8(const std::string& s) {
    std::string out;
    for (unsigned char b : s) {
        if (b < 0x80) {
            out.push_back(static_cast<char>(b));
        } else {
            out.push_back(static_cast<char>(0xC0 | (b >> 6)));
            out.push_back(static_cast<char>(0x80 | (b & 0x3F)));
        }
    }
    return out;
}

// Replace an event's leading zero delta with one beat (480 ticks).
std::vector<uint8_t> one_beat_later(std::vector<uint8_t> ev) {
    ev.erase(ev.begin());
    std::vector<uint8_t> out = {0x83, 0x60};
    out.insert(out.end(), ev.begin(), ev.end());
    return out;
}

}  // namespace

TEST_CASE(".chart: disco markers match the regexes they replaced") {
    int checked = 0;
    for (int byte = 0; byte < 256; ++byte) {
        const char b = static_cast<char>(byte);
        // A .chart event word never holds whitespace or '=' (both split it).
        if (std::isspace(static_cast<unsigned char>(b)) || b == '=') continue;
        for (const std::string& marker : disco_candidates(b)) {
            for (bool prior_on : {false, true}) {
                const std::string text =
                    "[Song]\n{\n  Resolution = 192\n}\n"
                    "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n"
                    "[ExpertDrums]\n{\n" +
                    std::string(prior_on ? "  0 = E mix_3_drums0d\n" : "") +
                    "  0 = N 0 0\n  192 = E " + marker + "\n  192 = N 1 0\n}\n";
                const std::vector<uint8_t> data(text.begin(), text.end());
                const Song song = load_songbytes_chart(data, true, true);
                REQUIRE(song.sequence.size() == 2);

                const bool off = std::regex_match(marker, oracle_disco_off());
                const bool on = std::regex_match(marker, oracle_disco_on());
                const bool want_flip = off ? false : on ? true : prior_on;
                // A flipped red pad reads as a yellow cymbal.
                const bool flipped = song.sequence[1].chord.at(NoteColor::Yellow).has_value();
                CHECK_MESSAGE(flipped == want_flip,
                              "byte " << byte << " marker index, prior " << prior_on);
                ++checked;
            }
        }
    }
    CHECK(checked > 4000);
}

TEST_CASE(".mid: disco and dynamics markers match the regexes they replaced") {
    using namespace testmidi;
    int checked = 0;
    for (int byte = 0; byte < 256; ++byte) {
        const char b = static_cast<char>(byte);
        for (const std::string& marker : disco_candidates(b)) {
            const std::string as_read = latin1_to_utf8(marker);
            const bool on = std::regex_match(as_read, oracle_disco_on());
            const bool off = std::regex_match(as_read, oracle_disco_off());
            for (bool prior_on : {false, true}) {
                std::vector<std::vector<uint8_t>> ev = {track_name("PART DRUMS"), set_tempo()};
                if (prior_on) ev.push_back(text_event("mix_3_drums0d"));
                ev.push_back(note_on(96, 100));                    // tick 0: kick
                ev.push_back(one_beat_later(text_event(marker)));  // tick 480
                ev.push_back(note_on(97, 100));                    // tick 480: red
                ev.push_back(end_of_track());
                const Song song = load_songbytes_mid(smf(concat(ev)), true, true);
                REQUIRE(song.sequence.size() == 2);
                const bool want_flip = on ? true : off ? false : prior_on;
                const bool flipped = song.sequence[1].chord.at(NoteColor::Yellow).has_value();
                CHECK_MESSAGE(flipped == want_flip, "byte " << byte << ", prior " << prior_on);
                ++checked;
            }
        }

        const std::string c(1, b);
        for (const std::string& marker : std::vector<std::string>{
                 "[ENABLE_CHART_DYNAMICS]", "ENABLE_CHART_DYNAMICS", "[ENABLE_CHART_DYNAMICS",
              "ENABLE_CHART_DYNAMICS]", "[[ENABLE_CHART_DYNAMICS]", "[ENABLE_CHART_DYNAMICS]]",
              "", "[]", c + "ENABLE_CHART_DYNAMICS", "ENABLE_CHART_DYNAMICS" + c,
              "ENABLE_CHART" + c + "DYNAMICS"}) {
            // Clone Hero's two exact strings (finding 64, D24).
            const std::string read = latin1_to_utf8(marker);
            const bool want = read == "ENABLE_CHART_DYNAMICS" || read == "[ENABLE_CHART_DYNAMICS]";
            const Song song = load_songbytes_mid(
                smf(concat({track_name("PART DRUMS"), set_tempo(), text_event(marker),
                            note_on(97, 127), end_of_track()})),
                true, true);
            REQUIRE(song.sequence.size() == 1);
            const bool accent = song.sequence[0].chord.at(NoteColor::Red)->is_accent();
            CHECK_MESSAGE(accent == want, "byte " << byte << ": dynamics marker");
            ++checked;
        }
    }
    CHECK(checked > 6000);
}

TEST_CASE(".chart: section headers are found as the regex found them") {
    // The first line outside a section must hold a `[name]`, found as
    // std::regex_search(`\[.*\]`) found it: leftmost '[', greedy to the last
    // ']' reachable without crossing a '\r'. No header throws ChartFileError;
    // a header that is not [Song] leaves the chart without its [Song] section.
    static const std::regex header(R"(\[.*\])");
    const std::vector<std::string> lines = {
        "[Song]", "x[Song]", "[Song]x", "[Song] [x]", "[[Song]", "[Song]]",
        "[So\rng]", "[Song\r]", "[x\r][Song]", "[x\r]]", "]Song[", "[", "Song",
        "[]", "\x01[Song]", "[\r[Song]", "[a\r[b]", "[Song]\r]", "[x]\r[Song]",
        "\xC3\xA9[Song]\xC3\xA9", "[S\xC3\xA9]",
    };
    for (const std::string& raw : lines) {
        const std::string text = raw + "\n{\n  Resolution = 192\n}\n"
                                       "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n";
        const std::vector<uint8_t> data(text.begin(), text.end());
        const std::string line = trim(raw);
        std::smatch m;
        if (!std::regex_search(line, m, header)) {
            CHECK_THROWS_AS(load_songbytes_chart(data, true, true), ChartFileError);
            continue;
        }
        const std::string bracket = m.str(0);
        if (bracket.substr(1, bracket.size() - 2) == "Song") {
            CHECK_MESSAGE(load_songbytes_chart(data, true, true).tick_resolution() == 192, raw);
        } else {
            CHECK_THROWS_AS(load_songbytes_chart(data, true, true), std::out_of_range);
        }
    }
}

// Malformed .chart lines keep the handling they have always had. Each case
// was checked against the regex-era reader before it was replaced.
TEST_CASE(".chart: malformed lines keep their handling") {
    const std::string song = "[Song]\n{\n  Resolution = 192\n}\n";
    const std::string sync = "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n";
    auto parse = [](const std::string& text) {
        const std::vector<uint8_t> data(text.begin(), text.end());
        return load_songbytes_chart(data, true, true);
    };

    // A blank line outside a section is not a header: the chart is refused.
    CHECK_THROWS_AS(parse(song + "\n" + sync), ChartFileError);
    // So is trailing whitespace after the last section, or a stray '}'.
    CHECK_THROWS_AS(parse(song + sync + "   "), ChartFileError);
    CHECK_THROWS_AS(parse(song + "}\n" + sync), ChartFileError);

    // A second '=' ends the value: "N 1 0 = junk" reads as "N 1 0".
    {
        Song s = parse(song + sync + "[ExpertDrums]\n{\n  0 = N 1 0 = junk\n  192 = N 2 0=\n}\n");
        REQUIRE(s.sequence.size() == 2);
        CHECK(s.sequence[0].chord.at(NoteColor::Red).has_value());
        CHECK(s.sequence[1].chord.at(NoteColor::Yellow).has_value());
    }

    // Lines with no '=' or no value, and an N with too few words, add nothing.
    {
        Song s = parse(song + sync +
                       "[ExpertDrums]\n{\n  garbage\n  =\n  0 = N 1\n  0 = N 3 0\n}\n");
        REQUIRE(s.sequence.size() == 1);
        CHECK(s.sequence[0].chord.count() == 1);
        CHECK(s.sequence[0].chord.at(NoteColor::Blue).has_value());
    }

    // A note number that is not a number refuses the chart, as std::stoi does.
    CHECK_THROWS_AS(parse(song + sync + "[ExpertDrums]\n{\n  0 = N x 0\n}\n"),
                    std::invalid_argument);

    // A section that never closes is dropped.
    CHECK(parse(song + sync + "[ExpertDrums]\n{\n  0 = N 1 0\n").sequence.empty());

    // A repeated section replaces the earlier one.
    {
        Song s = parse(song + "[Song]\n{\n  Resolution = 480\n}\n" + sync +
                       "[ExpertDrums]\n{\n  0 = N 1 0\n}\n[ExpertDrums]\n{\n  0 = N 2 0\n}\n");
        CHECK(s.tick_resolution() == 480);
        REQUIRE(s.sequence.size() == 1);
        CHECK(s.sequence[0].chord.at(NoteColor::Yellow).has_value());
    }

    // Tabs and CRLF are whitespace; tick keys read like std::stoll ("+192").
    {
        Song s = parse("\t[Song]\v\f\n{\n Resolution\t=\t192 \r\n}\r\n" + sync +
                       "[ExpertDrums]\n{\n  0\t=\tN\t1\t0\n  +192 = N 2 0\n}\n");
        REQUIRE(s.sequence.size() == 2);
        CHECK(s.sequence[1].timecode.ticks() == 192);
    }

    // Generic E events: quotes and outer spaces come off, inner spaces stay.
    {
        Song s = parse(song + sync +
                       "[Events]\n{\n  0 = E   \"section  Intro  \"  \n  96 = E\n"
                       "  48 = E section Verse\n  10 = E \"prc_chorus\"\n}\n"
                       "[ExpertDrums]\n{\n  0 = N 1 0\n}\n");
        REQUIRE(s.practice_sections.size() == 3);
        CHECK(s.practice_sections[0].tick == 0);
        CHECK(s.practice_sections[0].name == " Intro  ");
        CHECK(s.practice_sections[1].tick == 10);
        CHECK(s.practice_sections[1].name == "chorus");
        CHECK(s.practice_sections[2].tick == 48);
        CHECK(s.practice_sections[2].name == "Verse");
    }
}
