# note_flags_from_markers_215E280 (build 6142, RVA 0x215E280)

A method of the chart track class (TypeInfo 0x3C962D0) that turns the track's markers into note flags. It runs for any chart format; its argument is the track.

Lines 21 to 45: five calls to the apply routine 0x215DBD0 (`docs/audit/note-shuffle/step0/open2/f215dbd0.asm`), with marker kinds 7, 6, 8, 10 and 9 in that order. The apply routine looks up the markers of that kind in the track's dictionary at +0x98.

Lines 46 to 70: walk the track's note list at +0x60 and call the flag pass 0x214CEE0 (`docs/audit/note-shuffle/step0/open2/f214cee0.asm`) on each note. That pass sets the note's flag word at +0x24 from its markers: kind 6 sets 16 (Tom), kind 7 sets 32 (Cymbal), kind 8 sets 256 (Flam), kind 9 sets 1024 (Accent) and kind 10 sets 512 (Ghost). The flag names come from the enum at dump.cs lines 546911 to 546926 (TypeDefIndex 8567).

Lines 82 to 90: three more track steps, 0x215CD50 (disco), 0x215C040 and 0x215D450. Lines 95 to 101: error paths.

So a note gets the flam flag here only if a kind-8 marker covers it. Which code makes kind-8 markers is traced in `docs/audit/2026-11-08-clone-hero-unknowns/W3.md`, block NL-39.
