// The one test definition of "the same Song": what a chart read out of a
// container must match when its notes file is read directly. Audit finding
// 122: test_sng.cpp and test_srb.cpp each had their own, and the .sng one
// compared only ticks, so a loader that dropped SP phrases or fills passed.

#ifndef HYDRA_TESTS_SONG_EQUAL_H
#define HYDRA_TESTS_SONG_EQUAL_H

#include <cstddef>

#include "parse/song.h"

namespace testsong {

// Tick resolution, the meter and tempo maps, the features, and per note the
// tick, chord code, solo and SP flags and fill length.
inline bool songs_equal(const hydra::Song& a, const hydra::Song& b) {
    if (a.tick_resolution() != b.tick_resolution()) return false;
    if (a.tpm_changes != b.tpm_changes) return false;
    if (a.bpm_changes != b.bpm_changes) return false;
    if (a.features != b.features) return false;
    if (a.sequence.size() != b.sequence.size()) return false;
    for (size_t i = 0; i < a.sequence.size(); ++i) {
        const hydra::SongTimestamp& x = a.sequence[i];
        const hydra::SongTimestamp& y = b.sequence[i];
        if (x.timecode.ticks() != y.timecode.ticks()) return false;
        if (x.chord.code() != y.chord.code()) return false;
        if (x.flag_solo != y.flag_solo || x.flag_sp != y.flag_sp) return false;
        if (x.activation_length != y.activation_length) return false;
    }
    return true;
}

}  // namespace testsong

#endif  // HYDRA_TESTS_SONG_EQUAL_H
