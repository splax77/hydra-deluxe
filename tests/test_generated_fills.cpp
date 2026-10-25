// Tests for generated fills (Song::check_activations, D115 B2-3).
//
// A chart with no fills of its own gets fills placed the way Clone Hero
// 1.1.0.6142's generator places them (0x20D2B30): on the chord nearest a
// measure line of Clone Hero's grid (MeasureIndex), starting on the line.
// Clone Hero 1.0 mode reads the same grid with its own fill rule.
// Each case pins a literal from the rule as the plan reads it, never a
// second copy of the rule.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "corpus_util.h"
#include "parse/song.h"
#include "search/graph.h"
#include "song_digest.h"

using namespace hydra;

namespace {

// A song at resolution 480 and 120 BPM with one red note at each tick in
// `chords`. `seven_eight_at`, when above 0, switches 4/4 to 7/8 there. `fills`
// picks the game version whose generator runs.
Song song_with_chords(const std::vector<int64_t>& chords, int64_t seven_eight_at = 0,
                      FillDeadlineRule fills = FillDeadlineRule::Ch11) {
    Song song(480);
    song.bpm_changes[0] = 120.0;
    if (seven_eight_at > 0) apply_timesig(song, seven_eight_at, 7, 8);
    song.build_timing();
    for (int64_t tick : chords) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);
        song.sequence.push_back(ts);
    }
    song.check_activations(core::default_rules(), fills);
    return song;
}

// Each generated fill as {start tick, chord tick}.
std::vector<std::pair<int64_t, int64_t>> fills_of(const Song& song) {
    std::vector<std::pair<int64_t, int64_t>> out;
    for (const SongTimestamp& ts : song.sequence)
        if (ts.has_activation())
            out.emplace_back(fill_start_tick(ts.timecode.ticks(), *ts.activation_length),
                             ts.timecode.ticks());
    return out;
}

using Fills = std::vector<std::pair<int64_t, int64_t>>;

}  // namespace

TEST_CASE("generated fills: a chord 241 ticks after the second measure line gets a fill") {
    // Resolution 480: half a beat is 240 ticks, and Clone Hero adds one tick.
    const Song song = song_with_chords({2161});
    REQUIRE(song.sequence.size() == 1);
    const SongTimestamp& ts = song.sequence[0];
    REQUIRE(ts.has_activation());
    CHECK(*ts.activation_length == 241);
    // The fill starts on the line at tick 1920, so its deadline is 4 quarter
    // notes before that line: tick 0, which is 0 ms.
    CHECK(activation_fill_deadline_ms(song.timing(), ts.timecode.ticks(), *ts.activation_length,
                                      FillDeadlineRule::Ch11) == 0.0);
}

TEST_CASE("generated fills: Clone Hero 1.0 mode gives half-measure fills, 1.1 line-start fills") {
    // Two chords 10 ticks after the measure lines at 1920 and 9600
    // (resolution 480, 4/4).
    const std::vector<int64_t> chords = {1930, 9610};
    // 1.1: each fill starts on its line, so its length is 10 ticks.
    CHECK(fills_of(song_with_chords(chords, 0, FillDeadlineRule::Ch11)) ==
          Fills{{1920, 1930}, {9600, 9610}});
    // 1.0: each fill is half a 4/4 measure (960 ticks) long, ending on its chord.
    const Song old = song_with_chords(chords, 0, FillDeadlineRule::Ch10);
    CHECK(fills_of(old) == Fills{{970, 1930}, {8650, 9610}});
    CHECK(*old.sequence[0].activation_length == 960);
}

TEST_CASE("generated fills: a chord 242 ticks from its line gets none") {
    CHECK(fills_of(song_with_chords({2162})).empty());
    CHECK(fills_of(song_with_chords({1920 - 242})).empty());
}

TEST_CASE("generated fills: a chord exactly on the line has length 0") {
    CHECK(fills_of(song_with_chords({1920})) == Fills{{1920, 1920}});
    CHECK(*song_with_chords({1920}).sequence[0].activation_length == 0);
}

TEST_CASE("generated fills: a chord just before its line has a negative length") {
    // User answer 6 (2026-10-12): the fill starts on the line, as Clone Hero's does.
    const Song song = song_with_chords({1820});
    REQUIRE(song.sequence[0].has_activation());
    CHECK(*song.sequence[0].activation_length == -100);
}

TEST_CASE("generated fills: the 1.0 deadline of a 0 or negative length is the line minus 250 ms") {
    // The line at tick 1920 is 2000 ms in (120 BPM, resolution 480). The 1.0 rule
    // measures back from the fill's start with at least 250 ms of lead, so a fill
    // starting on that line has its deadline at 1750 ms whether its chord sits on
    // the line (length 0) or 100 ticks before it (length -100).
    for (int64_t chord : {1920, 1820}) {
        INFO("chord tick " << chord);
        const Song song = song_with_chords({chord});
        REQUIRE(song.sequence[0].has_activation());
        CHECK(activation_fill_deadline_ms(song.timing(), chord, *song.sequence[0].activation_length,
                                          FillDeadlineRule::Ch10) == doctest::Approx(1750.0));
    }
}

TEST_CASE("generated fills: the nearer chord wins, and a tie goes to the later one") {
    CHECK(fills_of(song_with_chords({1900, 1930})) == Fills{{1920, 1930}});
    CHECK(fills_of(song_with_chords({1910, 1930})) == Fills{{1920, 1930}});
    CHECK(fills_of(song_with_chords({1915, 1930})) == Fills{{1920, 1915}});
}

TEST_CASE("generated fills: after a fill the next candidate is 4 lines on") {
    std::vector<int64_t> chords;
    for (int64_t line = 0; line <= 12; ++line) chords.push_back(line * 1920);
    CHECK(fills_of(song_with_chords(chords)) ==
          Fills{{1920, 1920}, {9600, 9600}, {17280, 17280}});
}

TEST_CASE("generated fills: the first line is never a candidate") {
    // A chord on tick 0 sits on the first measure line; the generator starts
    // at the second.
    CHECK(fills_of(song_with_chords({0})).empty());
}

TEST_CASE("generated fills: a first chord in measure 3 still counts lines from the second") {
    // Lines 1 and 2 have no chord near them, so each passes to the next line;
    // the first fill is on line 3 and the next ones on lines 7 and 11.
    std::vector<int64_t> chords;
    for (int64_t line = 3; line <= 12; ++line) chords.push_back(line * 1920);
    CHECK(fills_of(song_with_chords(chords)) ==
          Fills{{5760, 5760}, {13440, 13440}, {21120, 21120}});
}

TEST_CASE("generated fills: a 4/4 to 7/8 change places fills on the restarted grid") {
    // The scout's map: 7/8 from tick 2880, one and a half 4/4 measures in.
    // Clone Hero's grid restarts there, so its lines run 0, 1920, 2880, 4560,
    // 6240, ... (1680 ticks per 7/8 measure).
    const Song song = song_with_chords({3000, 4660, 9650}, 2880);
    CHECK(fills_of(song) == Fills{{2880, 3000}, {9600, 9650}});
}

TEST_CASE("generated fills: Clone Hero 1.0 mode reads the same restarted grid") {
    // The same map and chords as above. 1.0 asks the grid too, so its fills
    // sit on the lines at 2880 and 9600, each half a 7/8 measure (840 ticks)
    // long, ending on its chord.
    const Song song = song_with_chords({3000, 4660, 9650}, 2880, FillDeadlineRule::Ch10);
    CHECK(fills_of(song) == Fills{{2160, 3000}, {8810, 9650}});
}

TEST_CASE("generated fills: a 1.0 fill whose chord sits on a meter change takes the new meter") {
    // 7/8 from tick 3840, a 4/4 measure line. The chord on that tick is in
    // the 7/8 span, so its fill is half a 7/8 measure: 840 ticks.
    const Song song = song_with_chords({3840}, 3840, FillDeadlineRule::Ch10);
    CHECK(fills_of(song) == Fills{{3000, 3840}});
}

TEST_CASE("generated fills: a chart with its own fills gets none generated") {
    Song song(480);
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (int64_t tick : {1920, 3840}) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);
        song.sequence.push_back(ts);
    }
    song.sequence[1].activation_length = 480;
    song.check_activations(core::default_rules(), kTestFillRule);
    CHECK(fills_of(song) == Fills{{3360, 3840}});
    CHECK(std::find(song.features.begin(), song.features.end(), "Auto-Generated Fills") ==
          song.features.end());
}

// ---- the corpus pin -------------------------------------------------------
//
// For every corpus chart whose fills are generated, its fills as
// "start:chord" pairs, folded to one FNV-1a hash, with the count beside it.
// Pinned from one run of the code at D115 B2-3. A mismatch prints the
// chart's new line in the table's own form, so a deliberate change re-pins
// by copying it.

namespace {

struct FillPin {
    const char* chart;  // relative to testdata/input
    int count;
    uint64_t hash;
};

// clang-format off
const FillPin kFillPins[] = {
    {"common/IB24/T1/Earth, Wind & Fire - September/notes.mid", 28, 0x013938619242b6e7ull},
    {"common/IB24/T2/Karnivool - Themata/notes.mid", 62, 0xe5e7d3a19439f24dull},
    {"common/IB24/T2/Stone Temple Pilots - Trippin on a Hole in a Paper Heart/notes.chart", 20, 0xbe255891bceb1d8dull},
    {"common/IB24/T3/Good Tiger - Where Are the Birds/notes.chart", 22, 0x68bd08ab965baa51ull},
    {"common/IB24/T3/The Ghost Inside - Get What You Give - 02 - Outlive/notes.mid", 28, 0x27447b68c1aa5a45ull},
    {"common/IB24/T4/1.O.M. - Apex/notes.chart", 39, 0x0cccaf3dadedf579ull},
    {"common/IB24/T4/A Lot Like Birds - Think Dirty out Loud/notes.mid", 39, 0x9e0f0e6cc122fbe3ull},
    {"common/IB24/T4/Dance Gavin Dance - Chocolate Jackalope/notes.mid", 31, 0x2285e7e0e04e2229ull},
    {"common/IB24/T4/Haken - The Good Doctor/notes.mid", 29, 0x3eee6eda54601bf3ull},
    {"common/IB24/T5/Satyr - Whelmed/notes.mid", 48, 0xa31ba79be5ac1e82ull},
    {"common/IB24/T5/Sianvar - Omniphobia/notes.mid", 46, 0x546ca00880e1b7adull},
    {"common/IB24/T5/TEXTURES - Laments Of An Icarus/notes.mid", 40, 0x788a2e86a84aeb45ull},
    {"common/IB24/T6/Fallen Monarch - Blight/notes.chart", 106, 0x7e0c170fdfbb797aull},
    {"common/IB24/T6/Levitating (feat. DaBaby) (Dua Lipa) (L)/notes.chart", 22, 0x44dae46b05c95d99ull},
    {"common/IB24/T6/Lockslip - Lockslip - 02 - Bend/notes.mid", 28, 0xd11f1b26ac54fe8eull},
    {"common/IB24/T6/Pathfinder - When The Sunrise Breaks The Darkness/notes.mid", 69, 0x045f1334ae5249cfull},
    {"common/IB24/T7/Conquering Dystopia - Prelude to Obliteration/notes.chart", 34, 0xb571e6db9e8eb9c3ull},
    {"common/IB24/T7/SoundHaven - Snowball Earth [Indistinct]/notes.mid", 35, 0xacd70ef16d84bc59ull},
    {"common/Summer Blast _25 Setlist/Tier 1/Jason Paige - Pokemon Theme/notes.chart", 29, 0x496cbc1fe4f1e360ull},
    {"common/Summer Blast _25 Setlist/Tier 2/Dance Gavin Dance - Young Robot/notes.mid", 24, 0xf466f5103b4427eeull},
    {"common/Summer Blast _25 Setlist/Tier 2/Moonlight Haze - Lunaris/notes.chart", 17, 0x12d7a6ac68549009ull},
    {"common/Summer Blast _25 Setlist/Tier 2/Thornhill - Limbo/notes.mid", 22, 0x67ae178856dc8485ull},
    {"common/Summer Blast _25 Setlist/Tier 3/Mutemath - Allies/notes.chart", 19, 0xecaef36aa42059b3ull},
    {"common/Summer Blast _25 Setlist/Tier 4/Sufferer - Chair/notes.mid", 29, 0xe8e83a1221f502abull},
    {"common/Summer Blast _25 Setlist/Tier 5/Interloper - Pathkeeper/notes.mid", 52, 0x3a686ecd3d11dbe9ull},
    {"common/Summer Blast _25 Setlist/Tier 5/Victoria - Grow/notes.chart", 30, 0x87e962a06e45523full},
    {"common/Summer Blast _25 Setlist/Tier 6/Casiopea - Swallow/notes.mid", 42, 0x7827ac50a6a9599bull},
    {"common/Summer Blast _25 Setlist/Tier 7/Kinglet - Blister/notes.mid", 44, 0xfa77b8dd1a42c0a1ull},
    {"common/Summer Blast _25 Setlist/Tier 8/Equipoise - Alchemic Web Of Deceit/notes.mid", 46, 0xff4cbb3ba247844bull},
    {"common/Summer Blast _25 Setlist/Tier 8/Travis Orbin - Dr. Jackle_Dr. Jekyll [Miles Davis]/notes.mid", 60, 0xf03192f16c202dcfull},
    {"common/Summer Blast _25 Setlist/Tier 8/Xane60 - Godzilla (Eminem)/notes.chart", 36, 0x83b4d76e8b285807ull},
};
// clang-format on

// The pin's hash: the shared FNV-1a over the text's bytes (tests/song_digest.h).
uint64_t pin_hash(const std::string& s) {
    return digest::fnv(digest::kSeed, s.data(), s.size());
}

}  // namespace

TEST_CASE("generated fills: every corpus chart's generated fills are pinned") {
    std::map<std::string, const FillPin*> pins;
    for (const FillPin& p : kFillPins) pins[p.chart] = &p;

    const std::string root = corpus::root();
    int charts = 0;
    std::string fresh;
    bool all_match = true;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        if (std::find(song.features.begin(), song.features.end(), "Auto-Generated Fills") ==
            song.features.end())
            continue;
        std::string rel = path.substr(root.size());
        std::replace(rel.begin(), rel.end(), '\\', '/');
        rel.erase(0, rel.find_first_not_of('/'));
        std::string text;
        int count = 0;
        for (const auto& [start, chord] : fills_of(song)) {
            text += std::to_string(start) + ":" + std::to_string(chord) + ",";
            ++count;
        }
        char line[64];
        std::snprintf(line, sizeof line, "%d, 0x%016llxull},\n", count,
                      static_cast<unsigned long long>(pin_hash(text)));
        const std::string pin_line = "    {\"" + rel + "\", " + line;
        fresh += pin_line;
        ++charts;
        const auto it = pins.find(rel);
        const bool match = it != pins.end() && it->second->count == count &&
                           it->second->hash == pin_hash(text);
        CHECK_MESSAGE(match, "new pin: " << pin_line);
        all_match = all_match && match;
    }
    CHECK(charts == static_cast<int>(pins.size()));
    if (!all_match) MESSAGE("the whole table:\n" << fresh);
}
