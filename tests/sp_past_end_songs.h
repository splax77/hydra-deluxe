// Two hand-built songs whose last activation's Star Power runs past the
// chart's final note. Shared by test_sp_end.cpp and test_engine_sweeps.cpp,
// so each song is written out once. record_fixtures.h owns the builder.

#ifndef HYDRA_TESTS_SP_PAST_END_SONGS_H
#define HYDRA_TESTS_SP_PAST_END_SONGS_H

#include "parse/song.h"
#include "record_fixtures.h"

namespace hydra::test {

// Two SP phrases, then a fill at 2304. At cap 4 the activation there runs
// past the chart's last note (5280). Several cases share this song.
inline Song past_last_note_song() {
    return build_tail_song({{0, true, false},
                            {768, true, false},
                            {1536},
                            {2304, false, true},
                            {3072},
                            {3840},
                            {4608},
                            {5136},
                            {5280}});
}

// The same shape, but the note at 3840 ends an SP phrase, collected while
// the activation from 2304 is running. Several cases share this song.
inline Song mid_sp_phrase_song() {
    return build_tail_song({{0, true, false},
                            {768, true, false},
                            {1536},
                            {2304, false, true},
                            {3072},
                            {3840, true, false},
                            {4608},
                            {5376},
                            {6144},
                            {6720},
                            {6816}});
}

}  // namespace hydra::test

#endif  // HYDRA_TESTS_SP_PAST_END_SONGS_H
