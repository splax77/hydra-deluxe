// Note Shuffle (D104): apply_note_shuffle against the decoded reference.
//
// Every expected chord below is a literal from one run of
// docs/audit/note-shuffle/probes/vectors/gen_vectors.py, which feeds the same
// notes to docs/audit/note-shuffle/probes/capped/shuffle_ref_capped.py. Rerun
// that script to regenerate them; its output is kept beside it in
// vectors_output.txt. Chords are spelled as Chord::code() spells them.

#include "doctest.h"

#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "core/model.h"
#include "core/timing.h"
#include "parse/note_shuffle.h"
#include "parse/song.h"

#ifndef HYDRA_SOURCE_DIR
#error "HYDRA_SOURCE_DIR must be defined (see CMakeLists.txt)"
#endif

using namespace hydra;

namespace {

struct Row {
    int64_t tick;
    const char* code;
};
using Rows = std::vector<Row>;

// A chord from its Chord::code() spelling, for building made-up inputs. The
// test checks each one spells back the same, so this reads the format and
// owns none of it.
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

std::vector<std::string> codes_of(const std::vector<SongTimestamp>& seq) {
    std::vector<std::string> out;
    for (const SongTimestamp& ts : seq) out.push_back(ts.chord.code());
    return out;
}

const std::string kSongs = std::string(HYDRA_SOURCE_DIR) + "/docs/audit/note-shuffle/game-tests/songs/";

Song load_game_song(const std::string& folder, const char* file, bool pro) {
    return load_songpath(kSongs + folder + "/" + file, pro, /*bass2x=*/false, Difficulty::Expert);
}

struct GameSong {
    const char* folder;
    const char* file;
    bool pro;
    Rows want;
};

}  // namespace

TEST_CASE("Note Shuffle: the game-test songs shuffle as the game showed them") {
    // A1, A2, B, C2, D2 and E with Pro Drums on are the runs the game
    // confirmed (docs/audit/note-shuffle/game-tests/README.md). The Pro Drums
    // off rows are the reference's too; the game was not filmed for them.
    const std::vector<GameSong> songs = {
        {"NS A1 tick scale chart192", "notes.chart", true,
         {{960, "nn..."}, {1056, "..n.."}, {1152, "...n."}, {1248, "..n.."}, {1344, "n..n."}, {1440, "n...."}, {1536, "..n.."}, {1632, "...n."}, {1728, "nn..."}, {1824, "....n"}, {1920, "..N.."}, {2016, "...n."}, {2112, "n.N.."}, {2208, "....N"}, {2304, "...n."}, {2400, "n.n.."}}},
        {"NS A1 tick scale chart192", "notes.chart", false,
         {{960, "nn..."}, {1056, "...n."}, {1152, ".n..."}, {1248, "...n."}, {1344, "n.n.."}, {1440, "n...."}, {1536, ".n..."}, {1632, "..n.."}, {1728, "n..n."}, {1824, "..n.."}, {1920, ".n..."}, {2016, "..n.."}, {2112, "n...n"}, {2208, ".n..."}, {2304, "....n"}, {2400, "n..n."}}},
        {"NS A2 tick scale mid960", "notes.mid", true,
         {{4800, "nn..."}, {5280, "....n"}, {5760, "..N.."}, {6240, "....n"}, {6720, "n..N."}, {7200, "n...."}, {7680, ".n..."}, {8160, "...N."}, {8640, "nn..."}, {9120, "..n.."}, {9600, ".n..."}, {10080, "..N.."}, {10560, "nn..."}, {11040, "....N"}, {11520, "..n.."}, {12000, "n..n."}}},
        {"NS A2 tick scale mid960", "notes.mid", false,
         {{4800, "nn..."}, {5280, "...n."}, {5760, "..n.."}, {6240, "...n."}, {6720, "n.n.."}, {7200, "n...."}, {7680, "....n"}, {8160, "..n.."}, {8640, "n..n."}, {9120, "....n"}, {9600, "..n.."}, {10080, "....n"}, {10560, "n.n.."}, {11040, "....n"}, {11520, "...n."}, {12000, "nn..."}}},
        {"NS B note order mid480", "notes.mid", true,
         {{1920, "nn..n"}, {2160, "nnn.."}, {2400, "...nN"}, {2640, "n.n.n"}, {2880, "...Nn"}, {3120, "n.n.."}, {3360, "nn..."}, {3600, ".n..n"}}},
        {"NS B note order mid480", "notes.mid", false,
         {{1920, "n.nn."}, {2160, "nn.n."}, {2400, "...nn"}, {2640, "nn..n"}, {2880, "...nn"}, {3120, "nn..."}, {3360, "n..n."}, {3600, "...nn"}}},
        {"NS C2 control four colour last", "notes.mid", true,
         {{1920, "nn..."}, {2160, "..n.."}, {2400, "....n"}, {2640, "..n.."}, {2880, "n..n."}, {3120, "..n.."}, {3360, ".n..."}, {3600, "..n.."}, {3840, ".nNNN"}}},
        {"NS C2 control four colour last", "notes.mid", false,
         {{1920, "nn..."}, {2160, "..n.."}, {2400, "...n."}, {2640, "..n.."}, {2880, "n..n."}, {3120, "..n.."}, {3360, "....n"}, {3600, "..n.."}, {3840, ".nnnn"}}},
        {"NS D2 control same notes later", "notes.mid", true,
         {{3840, "nnNn."}, {4320, "....n"}, {4560, ".n..."}, {4800, "....n"}, {5040, "nn..."}, {5280, "....n"}, {5520, "..N.."}, {5760, "n...n"}, {6000, "..N.."}}},
        {"NS D2 control same notes later", "notes.mid", false,
         {{3840, "nn.nn"}, {4320, "..n.."}, {4560, "....n"}, {4800, "..n.."}, {5040, "n...n"}, {5280, "..n.."}, {5520, "....n"}, {5760, "n.n.."}, {6000, "....n"}}},
        {"NS E star cutoffs", "notes.mid", true,
         {{1920, ".n..."}, {2160, "....n"}, {2400, ".n..."}, {2640, "..n.."}, {2880, "...N."}, {3120, "..N.."}, {3360, "n..N."}, {3600, "..n.."}, {3840, "...N."}, {4080, "n...."}, {4320, "n.N.."}, {4560, "...N."}, {4800, "....N"}, {5040, "..N.."}, {5280, "n...N"}, {5520, "..n.."}, {5760, "nn..."}, {6000, "...N."}, {6240, "n...n"}, {6480, "..n.."}, {6720, "....N"}, {6960, "..n.."}, {7200, "n..n."}, {7440, "..n.."}, {7680, "....n"}, {7920, "n...."}, {8160, "n.n.."}, {8400, "...N."}, {8640, "....N"}, {8880, "..n.."}, {9120, "n...n"}, {9360, "..n.."}, {9600, "nn..."}, {9840, "..n.."}, {10080, "n..N."}, {10320, ".n..."}, {10560, "....n"}, {10800, ".n..."}, {11040, "n...N"}, {11280, "..n.."}, {11520, "...n."}, {11760, "n...."}, {12000, "n...N"}, {12240, "...N."}, {12480, "....N"}, {12720, "...N."}, {12960, "nn..."}, {13200, "...n."}, {13440, "n.N.."}, {13680, "...n."}, {13920, "n.n.."}, {14160, "...N."}, {14400, "....n"}, {14640, "...N."}, {14880, "n...n"}, {15120, "..n.."}, {15360, "....n"}, {15600, "n...."}, {15840, "n..n."}, {16080, "....N"}, {16320, "..N.."}, {16560, "...n."}, {16800, "n.n.."}, {17040, "...n."}, {17280, "n...N"}, {17520, "...n."}, {17760, "n...n"}, {18000, "..n.."}, {18240, "...N."}, {18480, "..n.."}, {18720, "n..n."}, {18960, "....n"}, {19200, "..N.."}, {19440, "n...."}, {19680, "nn..."}, {19920, "..N.."}, {20160, "....n"}, {20400, "...N."}, {20640, "n...N"}, {20880, "...N."}, {21120, "n...N"}, {21360, ".n..."}}},
        {"NS E star cutoffs", "notes.mid", false,
         {{1920, ".n..."}, {2160, "...n."}, {2400, ".n..."}, {2640, "....n"}, {2880, "...n."}, {3120, "....n"}, {3360, "n.n.."}, {3600, "....n"}, {3840, "...n."}, {4080, "n...."}, {4320, "n...n"}, {4560, "...n."}, {4800, "....n"}, {5040, "..n.."}, {5280, "nn..."}, {5520, "....n"}, {5760, "n.n.."}, {6000, "...n."}, {6240, "n.n.."}, {6480, ".n..."}, {6720, "....n"}, {6960, ".n..."}, {7200, "n..n."}, {7440, "..n.."}, {7680, ".n..."}, {7920, "n...."}, {8160, "n.n.."}, {8400, ".n..."}, {8640, "....n"}, {8880, "..n.."}, {9120, "nn..."}, {9360, "...n."}, {9600, "nn..."}, {9840, "...n."}, {10080, "n...n"}, {10320, "..n.."}, {10560, "....n"}, {10800, "..n.."}, {11040, "n...n"}, {11280, "...n."}, {11520, "..n.."}, {11760, "n...."}, {12000, "nn..."}, {12240, "..n.."}, {12480, "...n."}, {12720, "..n.."}, {12960, "nn..."}, {13200, "...n."}, {13440, "n...n"}, {13680, "..n.."}, {13920, "n..n."}, {14160, "..n.."}, {14400, "....n"}, {14640, "..n.."}, {14880, "n..n."}, {15120, "..n.."}, {15360, ".n..."}, {15600, "n...."}, {15840, "n.n.."}, {16080, "....n"}, {16320, "..n.."}, {16560, "...n."}, {16800, "n.n.."}, {17040, ".n..."}, {17280, "n..n."}, {17520, "..n.."}, {17760, "n..n."}, {18000, "....n"}, {18240, "...n."}, {18480, "....n"}, {18720, "nn..."}, {18960, "...n."}, {19200, "..n.."}, {19440, "n...."}, {19680, "n...n"}, {19920, "...n."}, {20160, "....n"}, {20400, ".n..."}, {20640, "n..n."}, {20880, ".n..."}, {21120, "n..n."}, {21360, "..n.."}}},
    };
    for (const GameSong& s : songs) {
        INFO(s.folder << (s.pro ? ", Pro Drums on" : ", Pro Drums off"));
        Song song = load_game_song(s.folder, s.file, s.pro);
        CHECK(apply_note_shuffle(song.sequence, s.pro) == NoteShuffleResult::Shuffled);
        check_rows(song.sequence, s.want);
    }
}

TEST_CASE("Note Shuffle: C1 and D1 freeze the game, and the sequence is left as it was") {
    // C1: a four-colour chord, then a lone snare that can draw no pad. D1: the
    // first four notes all on tick 0, so the generator's state is zero.
    for (const char* folder : {"NS C1 four colour then snare", "NS D1 four notes on tick 0"}) {
        for (bool pro : {true, false}) {
            INFO(folder << (pro ? ", Pro Drums on" : ", Pro Drums off"));
            Song song = load_game_song(folder, "notes.mid", pro);
            const std::vector<std::string> before = codes_of(song.sequence);
            CHECK(apply_note_shuffle(song.sequence, pro) == NoteShuffleResult::GameFreezes);
            CHECK(codes_of(song.sequence) == before);
        }
    }
}

TEST_CASE("Note Shuffle: the copy rule checks a chord's first note two back, later notes one back") {
    // Chord 3's first note matches chord 1's shape (two back) and copies its
    // pad. Chord 4's first note has chord 3's shape (one back) but not chord
    // 2's (two back), so it redraws; its second note then matches chord 3
    // (one back) and copies. gen_vectors.py confirms each branch fired.
    std::vector<SongTimestamp> seq =
        sequence_of({{480, ".nN.."}, {960, "...n."}, {1440, ".nN.."}, {1920, ".nN.."}});
    REQUIRE(apply_note_shuffle(seq, true) == NoteShuffleResult::Shuffled);
    check_rows(seq, {{480, ".n..n"}, {960, "...n."}, {1440, ".n..n"}, {1920, "...nn"}});
}

TEST_CASE("Note Shuffle: a yellow tom blocks the yellow cymbal in its chord") {
    // At tick 1221 the chord's first note lands on the yellow tom. Blocking
    // only the exact lane drawn would then give the yellow cymbal too; the
    // game blocks both forms of the colour (gen_vectors.py checks that this
    // chord is where the two differ).
    std::vector<SongTimestamp> seq = sequence_of(
        {{501, ".n.n."}, {741, ".n.n."}, {981, ".n..n"}, {1221, ".n.n."}, {1461, ".n..n"}, {1701, ".n.n."}});
    REQUIRE(apply_note_shuffle(seq, true) == NoteShuffleResult::Shuffled);
    check_rows(seq, {{501, ".n.n."}, {741, "..nn."}, {981, "...nN"}, {1221, "..nN."}, {1461, "..nn."}, {1701, "..nN."}});
}

TEST_CASE("Note Shuffle: with Pro Drums off only the four toms are drawn") {
    std::vector<SongTimestamp> seq = sequence_of(
        {{960, "n.n.."}, {1080, "..n.."}, {1200, ".n..."}, {1320, "..n.."}, {1440, "n.n.."}, {1560, "n...."},
         {1680, ".n..."}, {1800, "..n.."}, {1920, "n...n"}, {2040, "...n."}, {2160, ".n.nn"}, {2280, "....n"},
         {2400, "n.n.."}, {2520, "...n."}, {2640, ".n..."}, {2760, "n...n"}});
    REQUIRE(apply_note_shuffle(seq, false) == NoteShuffleResult::Shuffled);
    check_rows(seq, {{960, "nn..."}, {1080, "...n."}, {1200, "....n"}, {1320, "...n."}, {1440, "n...n"}, {1560, "n...."},
                     {1680, "...n."}, {1800, "....n"}, {1920, "n..n."}, {2040, ".n..."}, {2160, ".nn.n"}, {2280, "...n."},
                     {2400, "n.n.."}, {2520, "...n."}, {2640, ".n..."}, {2760, "n.n.."}});
}

TEST_CASE("Note Shuffle: ghosts and accents ride along, and the 2x mark stays on the kick") {
    std::vector<SongTimestamp> seq = sequence_of({{480, "NgA.."}, {960, "...a."}, {1440, "n...g"}, {1920, ".a..N"}});
    REQUIRE(apply_note_shuffle(seq, true) == NoteShuffleResult::Shuffled);
    check_rows(seq, {{480, "Ng..A"}, {960, "...a."}, {1440, "n...G"}, {1920, "..nA."}});
}

TEST_CASE("Note Shuffle: the seed wraps around 64 bits") {
    // The first four notes' lane times tick sum to 1623313478486445056608,
    // which wraps to 4514400.
    constexpr int64_t t0 = 4611686018427400249;  // 2^62 + 12345
    std::vector<SongTimestamp> seq = sequence_of({{t0, "....N"}, {t0 + 480, "....N"}, {t0 + 960, "...N."},
                                                  {t0 + 1440, "..N.."}, {t0 + 1920, ".n..."}, {t0 + 2400, ".n..N"}});
    REQUIRE(apply_note_shuffle(seq, true) == NoteShuffleResult::Shuffled);
    check_rows(seq, {{t0, ".n..."}, {t0 + 480, "....n"}, {t0 + 960, "...n."},
                     {t0 + 1440, "..N.."}, {t0 + 1920, "....n"}, {t0 + 2400, "...nN"}});
}

TEST_CASE("Note Shuffle: a first chord on tick 0 with a seed that is not zero") {
    // The game's tick bookkeeping starts at 0, so every note on tick 0 takes
    // the same-chord path, the first one included.
    std::vector<SongTimestamp> seq =
        sequence_of({{0, "nn.n."}, {480, "..N.."}, {960, ".n..."}, {1440, "..N.."}, {1920, ".nN.."}});
    REQUIRE(apply_note_shuffle(seq, true) == NoteShuffleResult::Shuffled);
    check_rows(seq, {{0, "nn..n"}, {480, "...N."}, {960, "....n"}, {1440, "...N."}, {1920, ".n.N."}});
}

TEST_CASE("Note Shuffle: the gameplay video's chart") {
    // The video's chart file is not in the repo. Its notes are listed in
    // docs/audit/note-shuffle/verify/check_video.py, and gen_vectors.py checks
    // the reference's result against every tick the video showed.
    std::vector<SongTimestamp> seq = sequence_of(
        {{2640, "n...."}, {2700, "n...."}, {2760, "n...."}, {2820, ".n..."}, {2880, "..n.."}, {2940, "...n."},
         {3000, "....n"}, {3060, "....n"}, {3120, "....n"}, {3180, "....n"}, {3240, "....n"}, {3300, "....n"},
         {3360, "....n"}, {3420, "....n"}, {3480, "....n"}, {3540, "....n"}, {3600, "....n"}, {3660, "....n"},
         {3720, "...n."}, {3780, "..n.."}, {4080, "nn..N"}, {4200, "n.n.n"}});
    REQUIRE(apply_note_shuffle(seq, true) == NoteShuffleResult::Shuffled);
    check_rows(seq, {{2640, "n...."}, {2700, "n...."}, {2760, "n...."}, {2820, "...n."}, {2880, "..n.."}, {2940, "...N."},
                     {3000, "....n"}, {3060, "..N.."}, {3120, "....n"}, {3180, "..N.."}, {3240, "....n"}, {3300, "..N.."},
                     {3360, "....n"}, {3420, "..N.."}, {3480, "....n"}, {3540, "..N.."}, {3600, "....n"}, {3660, "..N.."},
                     {3720, "....n"}, {3780, ".n..."}, {4080, "n..nN"}, {4200, "nnn.."}});
}
