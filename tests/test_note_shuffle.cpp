// Note Shuffle (D104): apply_note_shuffle on real charts, checked against the
// game's shuffle rules (docs/clone-hero/02-note-list.md, NL-40 to NL-50 and
// NL-72), and against the chords the game showed in one filmed run.
//
// The rule checks read only the chart before and after the shuffle, so they
// hold whatever the generator draws: the kick stays, each chord keeps its
// pad count and its ghost and accent marks, Pro Drums off draws no cymbal, a
// chord whose pattern repeats the one two chords back copies its pad, and any
// other chord's first pad avoids the previous chord's colours. Chords are
// spelled as Chord::code() spells them.

#include "doctest.h"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "core/model.h"
#include "core/timing.h"
#include "corpus_util.h"
#include "load_util.h"
#include "parse/chart_files.h"
#include "parse/note_shuffle.h"
#include "parse/song.h"
#include "source_tree.h"

using namespace hydra;

namespace {

struct Row {
    int64_t tick;
    const char* code;
};
using Rows = std::vector<Row>;

// A chord from its Chord::code() spelling. The test checks each one spells
// back the same, so this reads the format and owns none of it.
Chord chord_of(std::string_view code) {
    Chord c;
    for (size_t i = 0; i < code.size(); ++i) {
        const char ch = code[i];
        if (ch == '.') continue;
        ChordNote& n = c.add_note(static_cast<NoteColor>(i + 1));
        const char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        n.dynamictype = lower == 'g'   ? NoteDynamicType::Ghost
                        : lower == 'a' ? NoteDynamicType::Accent
                                       : NoteDynamicType::Normal;
        if (ch != lower) set_lane_flag(n);
    }
    return c;
}

std::vector<SongTimestamp> sequence_of(const Rows& rows) {
    std::vector<SongTimestamp> seq;
    for (const Row& r : rows) {
        SongTimestamp ts;
        ts.timecode = Timecode::raw(r.tick);
        ts.chord = chord_of(r.code);
        REQUIRE(ts.chord.code() == r.code);
        seq.push_back(ts);
    }
    return seq;
}

void check_rows(const std::vector<SongTimestamp>& seq, const Rows& want) {
    REQUIRE(seq.size() == want.size());
    for (size_t i = 0; i < want.size(); ++i) {
        INFO("chord " << i << " at tick " << want[i].tick);
        CHECK(seq[i].timecode.ticks() == want[i].tick);
        CHECK(seq[i].chord.code() == std::string(want[i].code));
    }
}

const std::string kSongs =
    (sourcetree::root() / "docs/audit/note-shuffle/game-tests/songs/").u8string();

Song load_game_song(const std::string& folder, const char* file, bool pro) {
    return load_songpath(kSongs + folder + "/" + file,
                         ChartReading{Difficulty::Expert, pro, /*bass2x=*/false, false});
}

// A chord's pads as the game's lane mask, the kick left out: the chord's
// pattern in the copy rule (NL-46). Each bit is lane_bit's.
uint32_t pad_mask(const Chord& c) {
    uint32_t m = 0;
    for (const ChordNote& n : c.note_list())
        if (n.colortype != NoteColor::Kick) m |= lane_bit(n);
    return m;
}

// A chord's pad colours, one bit per NoteColor, a tom and a cymbal of one
// colour alike: what blocks a drawn pad (NL-45).
uint32_t pad_colours(const Chord& c) {
    uint32_t m = 0;
    for (const ChordNote& n : c.note_list())
        if (n.colortype != NoteColor::Kick) m |= 1u << static_cast<int>(n.colortype);
    return m;
}

size_t count_bits(uint32_t x) {
    size_t k = 0;
    for (; x != 0; x &= x - 1u) ++k;
    return k;
}

// The set bit of x with `rank` set bits below it, or 0 when x has too few.
uint32_t bit_of_rank(uint32_t x, size_t rank) {
    for (size_t i = 0; x != 0; ++i, x &= x - 1u)
        if (i == rank) return x & (~x + 1u);
    return 0;
}

// The chord's first pad in the game's note order (Chord::note_list, kick
// left out): the note the copy rule reads first.
ChordNote first_pad(const Chord& c) {
    for (const ChordNote& n : c.note_list())
        if (n.colortype != NoteColor::Kick) return n;
    throw std::logic_error("first_pad: a chord with no pad");
}

// A chord's ghost and accent pads, the kick left out.
std::pair<size_t, size_t> pad_marks(const Chord& c) {
    std::pair<size_t, size_t> marks{0, 0};
    for (const ChordNote& n : c.note_list()) {
        if (n.colortype == NoteColor::Kick) continue;
        if (n.is_ghost()) ++marks.first;
        if (n.is_accent()) ++marks.second;
    }
    return marks;
}

// How often each rule had something to check, so a test can show its rule
// was reached on the charts it read.
struct RuleTally {
    size_t chords = 0;        // chords with a pad; a lone kick is skipped (NL-44)
    size_t multi_pad = 0;     // chords with two or more pads
    size_t copies = 0;        // chords whose first pad took the copy rule (NL-46)
    size_t draws = 0;         // chords whose first pad was drawn (NL-45)
    size_t marked = 0;        // ghost and accent pads (NL-49)
    size_t kicks_2x = 0;      // 2x kicks (NL-44)
    size_t cymbals_out = 0;   // cymbal pads after the shuffle (NL-42)
    size_t tick0_starts = 0;  // charts whose first pad sits on tick 0 (NL-72)

    void add(const RuleTally& o) {
        chords += o.chords;
        multi_pad += o.multi_pad;
        copies += o.copies;
        draws += o.draws;
        marked += o.marked;
        kicks_2x += o.kicks_2x;
        cymbals_out += o.cymbals_out;
        tick0_starts += o.tick0_starts;
    }
};

// Checks the shuffled `after` against the unshuffled `before`, read with Pro
// Drums `pro`, by the rules this file's header names.
RuleTally check_shuffle_rules(const std::vector<SongTimestamp>& before,
                              const std::vector<SongTimestamp>& after, bool pro) {
    RuleTally t;
    REQUIRE(after.size() == before.size());
    std::vector<size_t> padded;  // the chords with a pad, in order
    for (size_t i = 0; i < before.size(); ++i) {
        const Chord& in = before[i].chord;
        const Chord& out = after[i].chord;
        const std::string where =
            "tick " + std::to_string(before[i].timecode.ticks()) + ": " + in.code() + " became " + out.code();
        INFO(where);
        CHECK(after[i].timecode.ticks() == before[i].timecode.ticks());
        // NL-44: the kick stays as it was, with its 2x and its mark.
        CHECK(out.at(NoteColor::Kick) == in.at(NoteColor::Kick));
        const std::optional<ChordNote>& kick = in.at(NoteColor::Kick);
        if (kick && kick->is2x) ++t.kicks_2x;
        // Each pad gets one new pad, never one whose colour the chord already
        // holds, so the chord keeps its count.
        const size_t pads_in = count_bits(pad_mask(in));
        CHECK(count_bits(pad_mask(out)) == pads_in);
        CHECK(count_bits(pad_colours(out)) == pads_in);
        // NL-49: a moved pad keeps its ghost or accent.
        const std::pair<size_t, size_t> marks = pad_marks(in);
        CHECK(pad_marks(out) == marks);
        t.marked += marks.first + marks.second;
        for (const ChordNote& n : out.note_list()) {
            if (!n.is_cymbal()) continue;
            ++t.cymbals_out;
            // NL-42: with Pro Drums off the draw reaches red and the toms only.
            CHECK(pro);
        }
        if (pads_in == 0) continue;
        padded.push_back(i);
        ++t.chords;
        if (pads_in >= 2) ++t.multi_pad;
    }
    // NL-72: a first pad on tick 0 joins the starting chord, so that chord
    // never takes a pattern slot and the chord two after it copies nothing.
    const bool first_on_tick0 = !padded.empty() && before[padded[0]].timecode.ticks() == 0;
    if (first_on_tick0) ++t.tick0_starts;
    for (size_t k = 1; k < padded.size(); ++k) {
        const Chord& in = before[padded[k]].chord;
        const Chord& out = after[padded[k]].chord;
        const std::string where = "tick " + std::to_string(before[padded[k]].timecode.ticks()) + ": " +
                                  in.code() + " became " + out.code();
        INFO(where);
        const uint32_t pattern = pad_mask(in);
        const bool two_back_counts = k >= 2 && !(k == 2 && first_on_tick0);
        if (two_back_counts && pad_mask(before[padded[k - 2]].chord) == pattern) {
            // NL-46: the first pad takes the pad of the same rank in the
            // output of the chord two back.
            const uint32_t first_bit = lane_bit(first_pad(in));
            const uint32_t copied =
                bit_of_rank(pad_mask(after[padded[k - 2]].chord), count_bits(pattern & (first_bit - 1u)));
            CHECK((pad_mask(out) & copied) != 0u);
            ++t.copies;
        } else {
            // NL-45: the first pad is drawn clear of the previous chord's
            // colours, so the chord holds at least one colour that one lacks.
            CHECK((pad_colours(out) & ~pad_colours(after[padded[k - 1]].chord)) != 0u);
            ++t.draws;
        }
    }
    return t;
}

// Shuffles `seq` with Pro Drums `pro`, requires the game would finish, and
// checks the result by the rules.
RuleTally shuffle_and_check(std::vector<SongTimestamp>& seq, bool pro) {
    const std::vector<SongTimestamp> before = seq;
    REQUIRE(apply_note_shuffle(seq, pro) == NoteShuffleResult::Shuffled);
    return check_shuffle_rules(before, seq, pro);
}

// Every corpus chart read on Expert with Pro Drums `pro` and 2x kicks kept,
// shuffled and checked; what each rule saw, summed. No corpus chart freezes
// the game (no chord holds more than two pads, and no chart's first four
// notes share tick 0), so each one must shuffle.
RuleTally shuffle_corpus(bool pro) {
    RuleTally sum;
    const ChartReading reading{Difficulty::Expert, pro, /*bass2x=*/true, /*noteshuffle=*/false};
    for (const std::string& path : corpus::chart_paths()) {
        INFO(path);
        std::vector<SongTimestamp> seq = corpus::song(path, reading).sequence;
        sum.add(shuffle_and_check(seq, pro));
    }
    return sum;
}

}  // namespace

TEST_CASE("Note Shuffle: the game-test songs shuffle by every rule") {
    // The songs the user ran in the game with Note Shuffle on: A1 is a .chart
    // at 192 ticks a beat, A2 a .mid at 960, B tests the note order inside a
    // chord, C2 and D2 are the controls for the two freezes, E is a long
    // chart.
    const std::vector<std::pair<const char*, const char*>> songs = {
        {"NS A1 tick scale chart192", "notes.chart"},  {"NS A2 tick scale mid960", "notes.mid"},
        {"NS B note order mid480", "notes.mid"},       {"NS C2 control four colour last", "notes.mid"},
        {"NS D2 control same notes later", "notes.mid"}, {"NS E star cutoffs", "notes.mid"},
    };
    for (const std::pair<const char*, const char*>& s : songs) {
        for (bool pro : {true, false}) {
            const std::string where = std::string(s.first) + (pro ? ", Pro Drums on" : ", Pro Drums off");
            INFO(where);
            Song song = load_game_song(s.first, s.second, pro);
            const RuleTally t = shuffle_and_check(song.sequence, pro);
            CHECK(t.chords > 0);
        }
    }
}

TEST_CASE("Note Shuffle: C1 and D1 freeze the game, and the sequence is left as it was") {
    // C1: a four-colour chord, then a lone snare that can draw no pad. D1: the
    // first four notes all on tick 0, so the generator's state is zero.
    for (const char* folder : {"NS C1 four colour then snare", "NS D1 four notes on tick 0"}) {
        for (bool pro : {true, false}) {
            INFO(folder << (pro ? ", Pro Drums on" : ", Pro Drums off"));
            Song song = load_game_song(folder, "notes.mid", pro);
            const std::vector<std::string> before = chord_codes(song.sequence);
            CHECK(apply_note_shuffle(song.sequence, pro) == NoteShuffleResult::GameFreezes);
            CHECK(chord_codes(song.sequence) == before);
        }
    }
}

TEST_CASE("Note Shuffle: on every corpus chart a chord that repeats the pattern two back copies, and others draw") {
    for (bool pro : {true, false}) {
        const std::string setting = pro ? "Pro Drums on" : "Pro Drums off";
        INFO(setting);
        const RuleTally t = shuffle_corpus(pro);
        CHECK(t.copies > 0);
        CHECK(t.draws > 0);
    }
}

TEST_CASE("Note Shuffle: on every corpus chart each chord keeps its pad count, so no colour is drawn twice") {
    // A drawn pad whose colour the chord already holds, as a yellow tom
    // beside a yellow cymbal, would leave the chord a pad short.
    for (bool pro : {true, false}) {
        const std::string setting = pro ? "Pro Drums on" : "Pro Drums off";
        INFO(setting);
        CHECK(shuffle_corpus(pro).multi_pad > 0);
    }
}

TEST_CASE("Note Shuffle: with Pro Drums off only red and the three toms are drawn") {
    // check_shuffle_rules fails on any cymbal with Pro Drums off. The same
    // corpus with Pro Drums on gets cymbals, so the draw's range is what
    // differs.
    CHECK(shuffle_corpus(false).chords > 0);
    CHECK(shuffle_corpus(true).cymbals_out > 0);
}

TEST_CASE("Note Shuffle: on every corpus chart ghosts and accents ride along, and the 2x mark stays on the kick") {
    const RuleTally t = shuffle_corpus(true);
    CHECK(t.marked > 0);
    CHECK(t.kicks_2x > 0);
}

TEST_CASE("Note Shuffle: a chart whose first chord is on tick 0 still shuffles") {
    // The hand-written charts in these two folders start with one pad on tick
    // 0; their next notes are later, so the seed is not zero. The corpus scan
    // lists neither folder (no song.ini), so they are read by path.
    RuleTally sum;
    for (const char* folder : {"test_fast_tempo", "test_folded_sqin"}) {
        for (const std::filesystem::directory_entry& e :
             std::filesystem::directory_iterator(std::filesystem::u8path(corpus::root() + "/" + folder))) {
            const std::string path = e.path().u8string();
            if (chart_format_of(path) != ChartFormat::Chart) continue;
            INFO(path);
            Song song = load_songpath(path, ChartReading{Difficulty::Expert, true, /*bass2x=*/true, false});
            sum.add(shuffle_and_check(song.sequence, true));
        }
    }
    CHECK(sum.tick0_starts > 0);
}

TEST_CASE("Note Shuffle: the drum lanes' bits are the game's") {
    // The game's lane map, docs/clone-hero/02-note-list.md NL-50 (Proven in
    // the game's code): kick 0x1, red 0x2, yellow, blue and green tom 0x4,
    // 0x8 and 0x10, yellow, blue and green cymbal 0x20, 0x40 and 0x80.
    struct Pad {
        NoteColor color;
        bool cymbal;
        uint32_t bit;
    };
    const Pad pads[] = {
        {NoteColor::Kick, false, 0x01},  {NoteColor::Red, false, 0x02},   {NoteColor::Yellow, false, 0x04},
        {NoteColor::Blue, false, 0x08},  {NoteColor::Green, false, 0x10}, {NoteColor::Yellow, true, 0x20},
        {NoteColor::Blue, true, 0x40},   {NoteColor::Green, true, 0x80},
    };
    for (const Pad& p : pads) {
        ChordNote n{p.color};
        if (p.cymbal) set_lane_flag(n);
        INFO(color_str(p.color) << (p.cymbal ? " cymbal" : ""));
        CHECK(lane_bit(n) == p.bit);
    }
}

TEST_CASE("Note Shuffle: the gameplay video's chart shuffles into the chords the game showed") {
    // A real game run: the 2026-10-08 bot video on Expert Pro Drums
    // (docs/clone-hero/02-note-list.md NL-71). The input is the video chart's
    // notes from the video scout's MIDI dump, and each expected chord is the
    // chord the video's frames showed at that tick; the chart file itself is
    // not in the repo. docs/audit/note-shuffle/verify/check_video.py lists
    // both, as `chart` and `seen`.
    std::vector<SongTimestamp> seq = sequence_of(
        {{2640, "n...."}, {2700, "n...."}, {2760, "n...."}, {2820, ".n..."}, {2880, "..n.."}, {2940, "...n."},
         {3000, "....n"}, {3060, "....n"}, {3120, "....n"}, {3180, "....n"}, {3240, "....n"}, {3300, "....n"},
         {3360, "....n"}, {3420, "....n"}, {3480, "....n"}, {3540, "....n"}, {3600, "....n"}, {3660, "....n"},
         {3720, "...n."}, {3780, "..n.."}, {4080, "nn..N"}, {4200, "n.n.n"}});
    shuffle_and_check(seq, true);
    check_rows(seq, {{2640, "n...."}, {2700, "n...."}, {2760, "n...."}, {2820, "...n."}, {2880, "..n.."}, {2940, "...N."},
                     {3000, "....n"}, {3060, "..N.."}, {3120, "....n"}, {3180, "..N.."}, {3240, "....n"}, {3300, "..N.."},
                     {3360, "....n"}, {3420, "..N.."}, {3480, "....n"}, {3540, "..N.."}, {3600, "....n"}, {3660, "..N.."},
                     {3720, "....n"}, {3780, ".n..."}, {4080, "n..nN"}, {4200, "nnn.."}});
}
