# Open 2 scout: note order inside a chord (Clone Hero v1.1.0.6142)

All addresses are RVAs in GameAssembly.dll. `dis.py` is the capstone wrapper used
(`py -I dis.py <rva> [maxbytes]`, `py -I dis.py callers <rva>...`). It adds the user
site-packages path by hand because `-I` hides it.

## Listings and what each one shows

| file | RVA | what it is |
|---|---|---|
| f20d2ea0_build.asm | 0x20D2EA0 | per-player note builder; drum transform order: flam, 2x-kick drop, 5-lane remap, disco flip / cymbal fold, then shuffle |
| f20d4be0_notelist.asm | 0x20D4BE0 | source list -> play-note list, in list order; chord chain (+0x10 prev, +0x18 next) linked when +0x2c != +0x28 |
| f214c9e0.asm | 0x214C9E0 | chord-head helper: returns self when +0x34 is set (always set for drums) |
| f20d36a0_lanemap.asm | 0x20D36A0 | source type -> lane bit (13 kick=1, 14 red=2, 15 Y=4/0x20, 16 B=8/0x40, 17 G=0x10/0x80; cymbal when flag bit 5) |
| f215c320.asm | 0x215C320 | track finalize, called by the .mid parser (0x2155050) and the .chart parser (0x2161A80) |
| f215c040.asm, f215e020.asm | 0x215C040, 0x215E020 | within-tick bubble sort: type ascending, ties flags ascending |
| f215b8c0.asm | 0x215B8C0 | same tick + same type: flags merged into the longest note (first on ties) |
| f5db0c0.asm, f2d4d0.asm | 0x5DB0C0, 0x2D4D0 | same tick + same type (key = +0x20 type): drop all but the longest (first on ties) |
| f5db4c0.asm | 0x5DB4C0 | sustain trim; re-runs the chord indexer |
| f215be50.asm | 0x215BE50 | chord indexer: +0x28 own index, +0x2c first index on the tick, +0x30 last, +0x34 = 1 for drums |
| f215bbf0.asm, f20cec60.asm | 0x215BBF0, 0x20CEC60 | add-note: pool get, +0x28 = Count, List.Add; no dedup at insert |
| f2155050.asm | 0x2155050 | .mid drum track parser; DoubleKick flag (8) for MIDI 59/71/83/95 via bitmask 0x1001001001; kick gets no tom/cymbal flag; velocity 1 ghost, 127 accent |
| f210d7b0.asm, midi_type_table.txt, midi_case_targets.txt | 0x210D7B0 | MIDI note -> type: 95 and 96 both KickDrum (13) |
| f210d9c0.asm | 0x210D9C0 | MIDI note -> difficulty |
| f213d620_chart_N_line.asm | 0x213D620 | .chart N line: N 0 kick flags 0, N 32 kick + DoubleKick, N 1..4 tom flag; its own note |
| drum_pretransforms_*.asm | 0x20EF450 etc | flam insert (list index i+1, chain orig->copy), 2x-kick remove + unlink, disco flip, cymbal fold |
| f214be70.asm | 0x214BE70 | play-note copy ctor: copies tick, mask, flags and both chain pointers |
| f214bcd0.asm | 0x214BCD0 | chord pattern: OR of masks walking next to the end, then prev to the start |
| f20f0c30_shuffle.asm | 0x20F0C30 | shuffle; seed loop at 0x20F0D90 sums mask*tick of the first four list entries, no lane filter |
| f214cee0.asm, f215e280.asm | 0x214CEE0, 0x215E280 | marker phrases applied to flags (tom, cymbal, disco, flam, ghost, accent) |
| getters.asm | various | source-note flag getters used by the builder |
| callers*.txt | | call-site searches |

## Findings in one paragraph each

Order in the list: at parse time the track finalizer sorts every group of notes on one
tick by note type ascending (kick 13, red 14, yellow 15, blue 16, green 17/18), ties by
flags ascending, with a stable bubble sort. Cymbal is a flag, not a type, so a yellow
cymbal sorts with yellow. File order is thrown away. The per-player builder keeps that
order, and the chord chain is built in the same order with the kick as the chain head.

Flam: the copy is inserted at list index i+1, right after its original, before the rest
of the chord; chain orig.next = copy, copy.prev = orig; copy.next is the copied old next,
but the old next's prev still points at the original.

2x kick with a normal kick on one tick: both parsers emit two KickDrum notes, one with
the DoubleKick flag. The finalizer merges the flags (union) into the longest one (first on
ties) and deletes the other. So one kick remains, flagged DoubleKick. With 2x Bass off
it is removed (no kick at all on that tick); with 2x Bass on there is one kick.

Seed: the first four list entries, kicks included, after flam insertion and 2x-kick
removal.
