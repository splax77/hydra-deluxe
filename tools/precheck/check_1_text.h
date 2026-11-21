// The fixed sentences check 1 (check_1.cpp) prints in its items. They live
// here so the check's test can compare an item's printed text with them.
#pragma once

namespace precheck {

// The reason of a helper item, before any known copy's note.
inline constexpr const char* kHelperWhy = "a test helper written in two files; keep one in a shared header";

// The hand-typed MIDI variable-length delta: the shared header that owns the
// spelling, the item's description (before the quoted line) and its reason.
inline constexpr const char* kVarlenOwner = "tests/midi_util.h";
inline constexpr const char* kVarlenWhat = "a MIDI variable-length delta encoded by hand";
inline constexpr const char* kVarlenWhy = "the varlen writer belongs in tests/midi_util.h";

}  // namespace precheck
