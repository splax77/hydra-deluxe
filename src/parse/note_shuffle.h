// Clone Hero 1.1's Note Shuffle modifier, drums only (D104).
//
// The game rearranges a chart's pads with a fixed generator seeded from the
// chart's own first notes, so the same chart and drum setting always shuffle
// the same way. apply_note_shuffle repeats that on a finished chord list. It
// is built from the code of the decoded reference,
// docs/audit/note-shuffle/probes/capped/shuffle_ref_capped.py, which the game
// tests in docs/audit/note-shuffle/game-tests matched.

#ifndef HYDRA_PARSE_NOTE_SHUFFLE_H
#define HYDRA_PARSE_NOTE_SHUFFLE_H

#include <vector>

#include "parse/song.h"

namespace hydra {

// Shuffled: the sequence now holds the game's shuffled chords. GameFreezes:
// the game never finishes shuffling this chart (it hangs while loading), and
// the sequence is left exactly as it was.
enum class NoteShuffleResult { Shuffled, GameFreezes };

// Shuffles `sequence` in place as the game does. `pro` is the Pro Drums
// setting the song was read with. Kicks never move. Each moved note keeps its
// ghost or accent mark and takes its colour and cymbal flag from its new pad.
// Throws std::logic_error if the shuffle ever gives a note no pad, or two
// notes of one chord the same colour; neither can happen, so either means a
// bug here.
NoteShuffleResult apply_note_shuffle(std::vector<SongTimestamp>& sequence, bool pro);

}  // namespace hydra

#endif  // HYDRA_PARSE_NOTE_SHUFFLE_H
