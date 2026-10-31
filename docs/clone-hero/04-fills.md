# Fills [FL]

This page covers written fills, the fills the game generates, their deadlines, and when a fill shows up.
It describes build 1.1 (v1.1.0.6142), and build 1.0 (v1.0.0.4045) under its own heading.
"The decompile" on this page means Ghidra's full decompile of build 6142, which `code/README.md` describes; it is not in the repo.
Last write-back: 2026-10-30.

## Written fills

### FL-01 Written fills come from MIDI note 120 and .chart S 64

Rule: The game adds a written fill at four places: the `.chart` reader (event type 0x40, written `S 64`), the `.mid` reader (MIDI note 120, through the helper 0x20CEDC0), and two cache paths.
Label: Proven in the game's code
Evidence: the callers of the fill initialiser `0x20FB9A0`, found by a call scan: in 0x20CEDC0, `0x20E5E70`, `0x213E0C0`, `0x215B7C0` and `0x215BD50`. The `.mid` reader tests the event byte against 0x78 (note 120) before it calls 0x20CEDC0; the `.chart` reader's test is in the decompile lines 25570 to 25594. That two of the paths read a cache is the game survey's reading, not checked.
Game code (6142): the `.mid` fill helper 0x20CEDC0, listed at `code/6142/mid_fill_reader_20CEDC0.asm`; the `.chart` reader `0x213E0C0`; the cache reader `0x20E49D0`, listed at `code/6142/fill_cache_caller_20E49D0.asm`.
Hydra: `kFillMarkerPitch` in src/parse/song.cpp line 684; both parsers end in `place_authored_fills` (lines 1119 and 1874).
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-02 The add-fill method appends a fill and decides nothing

Rule: `0x215BD50` takes a fill object from the track's pool, fills in its start tick, length and flag, and appends it to the track's fill list at `#8661+0x88`. It decides nothing, and it has no direct caller.
Label: Proven in the game's code
Evidence: `code/6142/add_fill_215BD50.asm` lines 17 to 30 (the pool at `+0x30` and the constructor call) and 31 to 66 (the append). A scan of every direct call and jump, the `.data` pointers and the metadata slots found no caller; a delegate call would not show in such a scan, so who calls it stays open. Its body is the same as `0x215B7C0`, listed at `code/6142/add_fill_twin_215B7C0.asm`.
Game code (6142): the add-fill method of class #8661 (scrambled name), RVA `0x215BD50`, `code/6142/add_fill_215BD50.asm` lines 17 to 66.
Hydra: no owner: Hydra has no add-fill step; `place_authored_fills` (src/parse/song.cpp line 363) places each fill.
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-03 A written fill lands on a chord up to trunc(resolution / 32) + 1 ticks after its end

Rule: A written fill may land on a chord up to trunc(resolution x 0.03125) + 1 ticks after the fill's end. The 1/32 and the +1 are fixed in the game.
Label: Proven in the game's code
Evidence: `code/6142/fill_to_activation_20CFF60.asm` lines 55 and 56 (the resolution at `[chart+0xb8]` times 0.03125), 61 (the truncating convert) and 62 (`inc rdi`); `code/6142/engine_ctor_20DF680.asm` line 57 shows `[chart+0xb8]` is the resolution.
Game code (6142): the fill-to-activation step of class #8265 (scrambled name), RVA `0x20CFF60`, `code/6142/fill_to_activation_20CFF60.asm` lines 55 to 63.
Hydra: `place_authored_fills` in src/parse/song.cpp lines 373 and 374, with the default `Rules::fill_land_slop_beats` of 1/32 in src/core/rules.h line 51; a user's `hydra_rules.ini` may change the 1/32, never the +1.
Ruling: none needed: same rule. D22 records it: "The fill landing window is floor(resolution x slop) + 1 ticks, as Clone Hero computes it (0x20D0088-0x20D008D). The +1 applies on top of a user's `hydra_rules.ini` slop too." (the decision log's words)
Found: 2026-10-03, session cebf3006; listed 2026-10-30, session b02ab3e6 (the reference build).

### FL-04 Each written fill is searched on its own, from the first chord

Rule: The game searches for each written fill's chord on its own, always from chord index 0, so it carries no search position from one fill to the next. A fill whose skip byte at `+0x20` is nonzero is passed over.
Label: Proven in the game's code
Evidence: `code/6142/fill_to_activation_20CFF60.asm` lines 83 to 87 (the skip test), 92 to 96 (the arguments: a constant 0 stored to the stack at line 93, then the call to `0x5DE030`) and 147 (the loop back). This answers D30's open sentence.
Game code (6142): RVA `0x20CFF60`, `code/6142/fill_to_activation_20CFF60.asm` lines 78 to 98 and 147.
Hydra: `place_authored_fills` in src/parse/song.cpp lines 376 to 388 (a fresh search over the whole chord list per fill).
Ruling: none needed: same rule. D30 had left it open: "Open: the search takes a start index, so Clone Hero may carry position between fills; not traced."
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-05 Which chord a written fill takes when chords sit on both sides

Rule: The game takes the first chord strictly after the fill's end and the last chord at or before it. The one at or before is dropped if it is earlier than the fill's start, and the one after is dropped if it is later than end + slop; a chord exactly at the start or at end + slop stays. When both stay, the closer wins, and a tie goes to the later chord.
Label: Proven in the game's code
Evidence: `code/6142/nearest_chord_5DE030.asm` lines 28 (the forward search), 39 (the backward search), 90 to 99 (the strict "outside" tests) and 100 to 113 (`tick(after) - target <= target - tick(before)` returns the after chord); the helpers' meanings are in `code/6142/first_after_search_2C7CF70.asm` lines 65 and 66 and `code/6142/back_search_2C7CE00.asm` lines 69 to 79. The addresses 0x2C7CF70 and 0x2C7CE00 are compiler-generated helpers with no dump entry. Before these listings the rule rested only on the 2026-10-03 listing, which was lost.
Game code (6142): the nearest-chord helper, RVA `0x5DE030`, `code/6142/nearest_chord_5DE030.asm` lines 28 to 117.
Hydra: `place_authored_fills` in src/parse/song.cpp lines 377 to 388.
Ruling: none needed: same rule. D30: "Each fill takes the last chord at or before its end, but not before its start, or the first chord after its end within the slop. A tie goes to the later chord, and a fill with no chord in bounds is dropped." (the decision log's words)
Found: 2026-10-03, session cebf3006; listed 2026-10-30, session b02ab3e6 (the reference build).

### FL-06 With no chord at or before the end, the game takes the next chord however far

Rule: When no chord sits at or before a written fill's end, the game takes the first chord after the end with no check against end + slop, however far away. Hydra drops the fill instead.
Label: Proven in the game's code
Evidence: `code/6142/nearest_chord_5DE030.asm` lines 40 to 60: the before side is -1, so the after chord is returned with no bound test.
Game code (6142): RVA `0x5DE030`, `code/6142/nearest_chord_5DE030.asm` lines 40 to 60.
Hydra: `place_authored_fills` in src/parse/song.cpp lines 382 and 383 apply the bounds on every side.
Ruling: D30: "Clone Hero's unbounded one-side fallback (B2) is not adopted." ADR 0023 lists it under deliberate differences.
Found: 2026-10-03, session cebf3006; listed 2026-10-30, session b02ab3e6 (the reference build).

### FL-07 With no chord after the end, the game falls back to chord 0

Rule: When no chord sits after a written fill's end, the game's backward search restarts at its start index, which is 0 for written fills. So it returns chord 0 of the song, if that chord is before the end tick, with no bound test. The record's wording, "takes the chord before however far", is wrong for this side. Hydra drops the fill.
Label: Proven in the game's code
Evidence: `code/6142/nearest_chord_5DE030.asm` lines 28 to 32 (the start index swapped in when the after side is -1) and 61 to 69 (the before chord returned with no bound test); `code/6142/back_search_2C7CE00.asm` lines 20 to 34; the start index is the constant 0 at `code/6142/fill_to_activation_20CFF60.asm` line 93. The address 0x2C7CE00 is a compiler-generated helper with no dump entry. This is a reading of a loop nobody ran.
Game code (6142): RVA `0x5DE030` lines 28 to 69; the backward helper 0x2C7CE00 lines 20 to 34.
Hydra: `place_authored_fills` in src/parse/song.cpp lines 380 to 383 drop the fill.
Ruling: D30: "Clone Hero's unbounded one-side fallback (B2) is not adopted." It does not say which chord the game takes on this side, but Hydra drops the fill on both sides, so the ruling covers it.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-08 What the written fill's skip byte means

Rule: What the byte at a written fill's `+0x20` means, and what sets it, is not known.
Label: Unknown
Evidence: it is read only at `code/6142/fill_to_activation_20CFF60.asm` line 86; the constructor-like method `0x20FB9A0` that the add-fill method calls was not read.
Game code (6142): not found: looked in the eight fill listings under `code/6142/` (`plain_drums_20EFEE0`, `fold_20F16E0`, `fill_to_activation_20CFF60`, `nearest_chord_5DE030`, `fill_generator_20D2B30`, `fill_deadline_20DDB70`, `grid_restart_20DACA0` and `add_fill_215BD50`) and in `docs/audit/ch-evidence.md`.
Hydra: no owner: Hydra's written fill has no such field.
Ruling: none found
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-09 A landed fill flags one note of its chord

Rule: A landed written fill sets the activation flag 0x800 on one note of its chord and makes a record: the fill start tick at `+0x28`, the chord at `+0x20`, the chord tick minus the fill start at `+0x30`, and an "active" byte at `+0x38` set to 1. The flagged note is picked by `0x20D51D0`, which the drum-lanes notes read as the highest-pad note.
Label: Proven in the game's code
Evidence: `code/6142/fill_to_activation_20CFF60.asm` line 106 (`or dword ptr [rax + 0x7c], 0x800`) and lines 129 to 139 (the record fields). That `0x20D51D0` returns the highest-pad note comes from `docs/audit/drum-lanes/FINDINGS-play-time.md` line 50 only.
Game code (6142): RVA `0x20CFF60`, `code/6142/fill_to_activation_20CFF60.asm` lines 99 to 147.
Hydra: `SongTimestamp::activation_length` (src/core/song.h line 36) on the chord, and `Chord::activation_note` for the note; Hydra keeps no per-note flag.
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-10 Two written fills that pick the same chord

Rule: When two written fills pick one chord, Hydra keeps only the later fill's length; the game is said to keep both in its list, and its deadline loop clears the flag for whichever is due first.
Label: Unknown
Evidence: not checked: it needs a trace of how two fills reach one chord in `0x20CFF60`. Only the deadline loop's clear was read, in the decompile lines 693 to 705.
Game code (6142): RVA `0x20DDB70`, `code/6142/fill_deadline_20DDB70.asm`.
Hydra: `place_authored_fills` in src/parse/song.cpp line 388 and its comment at line 356.
Ruling: none found
Found: 2026-10-30, session b02ab3e6 (the reference build).

## Build 1.1 generated fills and the deadline

### FL-11 The note builder is the generator's one caller, on drum tracks only

Rule: The generator `0x20D2B30` has one direct caller, the note builder `0x20D2EA0`. The builder reaches the fill step only for the Drums (6) and ProDrums (9) instruments; every other instrument gets an empty fill list. The builder has four call sites, so every route that builds notes gets the same fill decision.
Label: Proven in the game's code
Evidence: a scan of every direct call in the DLL finds one reference to `0x20D2B30`, at 0x20D33A4; `code/6142/note_builder_20D2EA0.asm` line 296 is the call, lines 153 to 156 the instrument tests, line 228 the only path to line 278. The enum values are in `dump.cs` lines 533184 to 533197. The four builder call sites match the committed `docs/audit/note-shuffle/step0/open34/callers.txt` lines 1 to 4.
Game code (6142): the note builder of class #8265 (scrambled name), RVA `0x20D2EA0`, `code/6142/note_builder_20D2EA0.asm` lines 153 to 296; the generator, RVA `0x20D2B30`.
Hydra: `Song::check_activations` in src/core/song.cpp line 105 is the one place that picks the generator; Hydra loads drums only.
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-12 Written and generated fills reach the engine the same way

Rule: Written fills and generated fills reach the engine as the same record type, in the same slot of the builder's result. With no chords the generator returns an empty list at once, and it never writes to the track's fill list.
Label: Proven in the game's code
Evidence: `dump.cs` lines 528351 and 528413 (both steps return a list of #8322); `code/6142/note_builder_20D2EA0.asm` lines 289 to 299, 358 to 365, 393 and 424. `code/6142/fill_generator_20D2B30.asm` lines 49 to 63 (the empty return) and 153 to 179 (its only stores).
Game code (6142): RVA `0x20D2EA0` and RVA `0x20D2B30`, lines as above.
Hydra: `SongTimestamp::activation_length` (src/core/song.h line 36) holds written and generated fills alike; `Song::check_activations` returns on an empty chord list (src/core/song.cpp line 111).
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-13 The generator starts at the second measure line

Rule: The generator's first candidate is the second measure line (index 1); index 0 is never tried.
Label: Proven in the game's code
Evidence: `code/6142/fill_generator_20D2B30.asm` lines 71 (`mov ebx, 1`), 77 (the loop test) and 79 to 97.
Game code (6142): the generator of class #8265 (scrambled name), RVA `0x20D2B30`, `code/6142/fill_generator_20D2B30.asm` lines 71 to 97.
Hydra: `Song::place_fills_ch11` in src/core/song.cpp line 195 (`line = 1`).
Ruling: D229: chose "Adopt CH's rule (Recommended)" to "Clone Hero starts a generated fill on the measure line ...".
Found: 2026-10-12, session 28fad19c.

### FL-14 A generated fill's chord must sit within trunc(resolution / 2) + 1 ticks of its line

Rule: A generated fill's chord must be within trunc(resolution x 0.5) + 1 ticks of its measure line, on either side, edge included.
Label: Proven in the game's code
Evidence: `code/6142/fill_generator_20D2B30.asm` lines 66 and 67 (the resolution times 0.5, decoded in the listing's comment), 70 to 72 (the truncating convert and `inc rax`) and 116 to 126 (a distance over the limit skips, so equal stays). The 0.5 has no meter term, so the limit is half a quarter note in every meter; no 6/8 or 7/8 run exists for it either.
Game code (6142): RVA `0x20D2B30`, `code/6142/fill_generator_20D2B30.asm` lines 64 to 126.
Hydra: `Song::place_fills_ch11` in src/core/song.cpp lines 183, 184 and 210, with the default `fill_max_distance_beats` of 0.5 in src/core/rules.h line 45.
Ruling: D234: chose "Copy Clone Hero (Recommended)" to "Clone Hero can place a generated fill on a chord just before its measure line ...".
Found: 2026-10-12, session 08261223; listed 2026-10-30, session b02ab3e6 (the reference build).

### FL-15 The nearest chord wins, a tie goes to the later one, and the search start moves only on a placement

Rule: The generator takes the chord nearest the line, and a tie goes to the later chord. Its search starts at the last chord that took a fill (0 before the first), and a skipped line does not move that start.
Label: Proven in the game's code
Evidence: `code/6142/nearest_chord_loop_5DDB50.asm` lines 79 to 84 (the `jg` at line 83 returns the before chord only when the after chord is strictly farther, so an equal distance returns the later chord); `code/6142/fill_generator_20D2B30.asm` lines 73 (start 0), 98 to 104 (the call), 128 (the start stored only for an accepted chord) and 180, 184 and 189 (restored on every skip).
Game code (6142): the nearest-chord wrapper, RVA `0x5DDB50`, `code/6142/nearest_chord_loop_5DDB50.asm` lines 79 to 84; RVA `0x20D2B30` lines 73 to 192.
Hydra: `Song::place_fills_ch11` in src/core/song.cpp lines 199 to 208: the tie goes to the later chord, but its pointer moves on every line. That picks the same chord except at the tail of a song (FL-18).
Ruling: none needed: same rule, except FL-18. D74 records the tie as "unverified against Clone Hero" (the decision log's words).
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-16 A generated fill runs from its line to its chord

Rule: A generated fill starts on its measure line and ends at its chord, so its length is the chord tick minus the line tick, which is zero or negative for a chord on or before the line.
Label: Proven in the game's code
Evidence: `code/6142/fill_generator_20D2B30.asm` lines 145 to 148, 168 (the record's `+0x28` = the line tick) and 169 to 173 (`+0x30` = chord tick minus line tick).
Game code (6142): RVA `0x20D2B30`, `code/6142/fill_generator_20D2B30.asm` lines 145 to 173.
Hydra: `Song::place_fills_ch11` in src/core/song.cpp line 220.
Ruling: D229: chose "Adopt CH's rule (Recommended)" to "Clone Hero starts a generated fill on the measure line ...".
Found: 2026-10-12, session 28fad19c; listed 2026-10-30, session b02ab3e6 (the reference build).

### FL-17 The next candidate is four lines on after a fill, one line on after a miss

Rule: After a generated fill the next candidate is four measure lines later; after a line with no chord close enough it is the next line. The four is a literal in the code.
Label: Proven in the game's code
Evidence: `code/6142/fill_generator_20D2B30.asm` line 182 (`add ebx, 4` after a placement) and lines 184 to 192 (`sub ebx, 3` then `add ebx, 4`, a step of one, on the two skip paths).
Game code (6142): RVA `0x20D2B30`, `code/6142/fill_generator_20D2B30.asm` lines 181 to 192.
Hydra: `fill_cooldown_measures` (default 4, src/core/rules.h line 44), used at src/core/song.cpp line 223. The game's 4 is fixed; Hydra's is a user setting that equals it by default.
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-18 At the tail of a song the generator compares the line with an older chord

Rule: When no chord sits after a measure line, the game's "chord before the line" is searched downward from the carried start index, so it finds the last chord that took a fill, not the song's last chord. That chord is far from the line, so the line is skipped. Hydra takes the song's true last chord, which can be a few ticks before the line and take a fill.
Label: Proven in the game's code
Evidence: `code/6142/nearest_chord_loop_5DDB50.asm` lines 25 to 29 (the start index swapped in when the after side is -1) and 51, 52 and 93 (the before chord returned); `code/6142/back_search_2C7CE00.asm` lines 20 to 34; `code/6142/fill_generator_20D2B30.asm` lines 116 to 126 (the distance test then fails). The backward search's range comes from the enumerator constructor `0x5FB640` and its `MoveNext` `0xFEC570`, which `code/6142/back_search_2C7CE00.md` line 8 describes, so the code label holds. The address 0x2C7CE00 is a compiler-generated helper with no dump entry. No run has shown it.
Game code (6142): RVA `0x5DDB50` lines 25 to 29 and 51 to 93; RVA 0x2C7CE00 lines 20 to 34; RVA `0x20D2B30` lines 98 to 126.
Hydra: `Song::place_fills_ch11` in src/core/song.cpp lines 199 to 209 (the last chord when nothing follows the line).
Ruling: open split: see questions page Q-36
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-19 The 1.1 fill deadline

Rule: Just before the engine adds Star Power, if the meter is below half, the addition would reach half, and Star Power is not active, every recorded fill whose start tick is at or before (current tick + 4 quarter notes) loses its record's active byte and its chord's 0x800 flag. A fill whose start is one tick past that stays. Fills are tested in list order from a carried index, and the first fill past the deadline ends the loop.
Label: Proven in the game's code
Evidence: `code/6142/fill_deadline_20DDB70.asm` lines 6 to 16 (the three guards, which include the "meter reaches half" trigger), 24 to 26 (the deadline = `+0x300` + `+0x108`), 45 (the compare that stops at the first fill past the deadline), 47 to 51 (the clears) and 69 (the tail jump to `0x20F4FA0`). The decompile lines 668 to 710 agree.
Game code (6142): the deadline method of class #8255 (scrambled name), RVA `0x20DDB70`, `code/6142/fill_deadline_20DDB70.asm` lines 6 to 69.
Hydra: `activation_fill_deadline_ms` in src/search/graph.cpp lines 62 to 70 (fill start minus `kCh11FillLeadBeats` times the resolution; `kCh11FillLeadBeats = 4` in src/search/graph.h line 71).
Ruling: none needed: same rule (ADR 0010: "Hydra's rule already said this").
Found: 2026-10-12, the fill evidence read; listed 2026-10-30, session b02ab3e6 (the reference build).

### FL-20 The four-beat lead is the resolution times 4, in every meter

Rule: The lead at `#8255+0x300` is the chart resolution, truncated to a whole number and multiplied by 4, written once in the drum engine's constructor. It has no meter term, so it is 4 quarter notes in every time signature.
Label: Proven in the game's code
Evidence: `code/6142/engine_ctor_20DF680.asm` lines 57 (the truncating convert of `[chart+0xb8]`) and 68 and 69 (`shl rax, 2` at 0x20DF7B7, then the store to `+0x300`). No 6/8 or 7/8 run exists.
Game code (6142): the drums engine constructor, RVA `0x20DF680`, `code/6142/engine_ctor_20DF680.asm` lines 57 to 69.
Hydra: `kCh11FillLeadBeats` times `tick_resolution` in src/search/graph.cpp lines 68 and 69.
Ruling: none needed: same rule. D26 had kept "6/8 beats" as "unverified against Clone Hero"; this code read settles it.
Found: 2026-10-12, the fill evidence read; listed 2026-10-30, session b02ab3e6 (the reference build).

### FL-21 The deadline's "half" is twice a base of resolution times 7.5

Rule: The half meter the deadline tests is `#8288+0x1B8`, twice a base derived from the resolution times 7.5; the full meter at `+0x1B0` is four times that base, so a full bar is 30 times the resolution when that product is whole.
Label: Proven in the game's code
Evidence: `code/6142/engine_ctor_20F79E0.asm` lines 162 to 170 (the multiply by 7.5 at line 162 and the store to `+0x1b8` at line 168). A scorestats file of Trippin SP Spike has `sp_bar_ticks` 5760, which is 4 x (192 x 7.5). What the call to 0x2C49A60 in between does was not read.
Game code (6142): the base engine constructor, RVA `0x20F79E0`, `code/6142/engine_ctor_20F79E0.asm` lines 162 to 170.
Hydra: none here; the meter size is page 5's row.
Ruling: none needed: page 5 owns the meter size
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-22 What the long at engine+0x108 holds

Rule: The deadline adds `+0x300` to the engine field at #8288+0x108. One reading calls that field the current tick; another reads it as the song clock in whole milliseconds.
Label: Unknown
Evidence: only the add at `code/6142/fill_deadline_20DDB70.asm` lines 24 to 26. `+0x108` is not among the fields named in `tools/ch_probe/experiments/results/overlay-spike/il2cpp-fields.md`, and its writer was not looked for. The current-tick reading is `docs/superpowers/plans/2026-10-12-one-record-per-chart/part-B2-timing.md` line 21. The whole-milliseconds reading comes from session 8157f2dc on 2026-09-25 (condensed transcript lines 1394 to 1422). A tick compare against fill start ticks fits the current-tick reading; neither is proven.
Game code (6142): field #8288+0x108; its writer was not read.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-23 Hydra's exact boundary at the deadline

Rule: Hydra is said to clear a fill when its ready time is at or past fill start - 4 x resolution and keep it one tick before, which is the game's boundary.
Label: Unknown
Evidence: not checked: it needs a read of how the sign of `fill_e_offset` maps to "usable" in src/search/engine.cpp lines 772 to 830 and 970.
Game code (6142): RVA `0x20DDB70`, `code/6142/fill_deadline_20DDB70.asm` line 45.
Hydra: `activation_fill_deadline_ms` in src/search/graph.cpp line 62; `fill_e_offset` in src/core/model.h line 314.
Ruling: none found
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-24 The game's deadline is an exact tick compare; Hydra's early fill is its own model

Rule: The game's deadline is an exact tick compare with no millisecond allowance. Hydra's early-fill reach, 85 ms on the orange side (Star Power ready after the deadline), and its 60 ms white slack are Hydra's own model of an early hit moving the Star Power ready moment.
Label: Hydra assumes it, never checked
Evidence: the game side has no millisecond term (`code/6142/fill_deadline_20DDB70.asm` lines 24 to 51). Hydra: `fill_refuses` refuses when the offset is below `-kHitWindowMs` and `is_e0` reads `kEarlyFillSlackMs = 60.0` (src/core/model.h lines 316, 320 and 135). The reach is `kHitWindowMs` (D270 item 8). Whether the game's own hit timing reaches that far is TM-27's question.
Game code (6142): RVA `0x20DDB70`; the fill-end grant in FL-31.
Hydra: `fill_refuses` and `is_e0` in src/core/model.h lines 316 and 320.
Ruling: D250: the user asked to "change the early fill search to allow orange timing up to 85ms", and on the charts that gained an E fill said "Keep them. The extra E fills in the notation are fine." D270 item 8 then made the reach the one hit window number.
Found: 2026-10-15, session c35a4b7d.

### FL-25 The game did not offer an early fill on Trippin path 3

Rule: On Trippin SP Spike, the game did not offer path 3's E0 fill at tick 34944 in three bot runs; it activated at the next fill, tick 38016, each time. Why is not known.
Label: Proven in a live run
Evidence: `tools/ch_probe/experiments/results/overlay-spike/run1_cal0_sp_compare.txt` line 34, `run2_cal30_sp_compare.txt` line 34 and `run2b_vid30_sp_compare.txt` line 32 each say "FAIL activation 3 not on the path's chord: game tick 38016, path act_tick 34944". The bot hit notes a median 0.42 ms early, not 85 ms early. The cause is not known; the observation itself is the live result.
Game code (6142): not found: no function decides this one alone.
Hydra: `is_e0` and `fill_refuses` in src/core/model.h lines 320 and 316.
Ruling: none found
Found: 2026-10-28, session f756d547.

## The grid restart at a meter change

### FL-26 The measure grid restarts on the first beat at or after a meter change

Rule: The grid is built beat by beat from tick 0 to the song's end, each step trunc(resolution x 4.0 / denominator) ticks. A new measure begins on the first beat at or after a time-signature change, where a change means the numerator or the denominator differs from the previous beat's signature; the beat and measure counters go back to zero there. Only measure lines go into the list the generator reads. The end is the song-end call `0x212E130`, so measure lines past the last chord exist.
Label: Proven in the game's code
Evidence: `code/6142/grid_restart_20DACA0.asm` lines 24 to 33 (the song-end call), 66 to 106 (the signature in force), 107 to 126 (the change test and the reset), 127 to 136 (the wrap), 185 to 192 (the step) and 193 and 194 (the stop); `code/6142/grid_add_line_20CF2B0.asm` lines 107 to 119 (only kind 0 adds a measure line). The 4.0 is a `.rdata` value with no name in `constants.py`, and the kinds 1 and 2 as "beat" and "sub-beat" are a reading of how the code uses them, not checked. The address 0x20CF2B0 is a compiler-generated helper with no dump entry.
Game code (6142): the grid builder of class #8257 (scrambled name), RVA `0x20DACA0`, `code/6142/grid_restart_20DACA0.asm` lines 24 to 198; RVA 0x20CF2B0 lines 107 to 119.
Hydra: `MeasureIndex` in src/core/timing.h line 120 and `MeasureIndex::first_measure_line_at_or_after` in src/core/timing.cpp line 196; `Song::place_fills_ch11` stops at the last reachable chord.
Ruling: D230: chose "CH's grid, everywhere (Recommended)" to "Hydra has one measure grid ...".
Found: 2026-10-12, session 28fad19c; listed 2026-10-30, session b02ab3e6 (the reference build).

## Whether written fills stop generated ones

### FL-27 A track gets generated fills only when its written-fill list is empty

Rule: The note builder runs the generator only when the track's fill list has a count of 0. With one or more written fills in the list it places them and skips the generator, so any written fill on a track stops generated fills for that whole track.
Label: Proven in the game's code
Evidence: `code/6142/note_builder_20D2EA0.asm` lines 278 to 299: the list at `[rbp+0x88]` (line 280), its count compared with `r13d` (line 285), which is zeroed at line 134 and not written before; equal jumps to the generator call, else line 289 calls `0x20CFF60`. The committed `docs/audit/note-shuffle/step0/open2-refute/f20D2EA0.asm` lines 134 and 281 to 297 show the same test. This settles the reserved question Q-70 on the game's side.
Game code (6142): RVA `0x20D2EA0`, the test at 0x20D3375, `code/6142/note_builder_20D2EA0.asm` lines 278 to 299.
Hydra: `Song::check_activations` in src/core/song.cpp lines 105 to 107 (`if (ts.has_activation()) return;`).
Ruling: none needed: same rule, for every chart whose written fills land; the one case that differs is FL-28. D271 item 2 asked for a live check ("Yes, check it"), which would only confirm this.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-28 A written fill that lands nowhere still stops generation in the game

Rule: The game counts the written fills before any of them lands on a chord, so a written fill that finds no chord still stops generation. Hydra tests whether any chord ended up holding a fill, after the landing step has dropped some. On a chart whose only written fills reach no chord, Hydra generates fills across the chart and the game generates none.
Label: Proven in the game's code
Evidence: `code/6142/note_builder_20D2EA0.asm` lines 285 to 299: the count test comes before the landing call to `0x20CFF60`. Hydra side: `place_authored_fills` drops a fill with no chord (src/parse/song.cpp line 363 on), then `Song::check_activations` tests the landed chords (src/core/song.cpp lines 106 and 107).
Game code (6142): RVA `0x20D2EA0`, `code/6142/note_builder_20D2EA0.asm` lines 280 to 299.
Hydra: `place_authored_fills` in src/parse/song.cpp line 363; `Song::check_activations` in src/core/song.cpp line 105.
Ruling: open split: see questions page Q-37
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-29 The fill list belongs to one instrument and difficulty

Rule: The list the builder tests belongs to one (instrument, difficulty) track. A fill written for Expert only does not stop generation on Hard, unless the reader also adds it to Hard's track. The `.chart` reader adds a fill to its own section's track; what the `.mid` reader does with note 120 across the four difficulties is not known.
Label: Unknown
Evidence: `code/6142/note_builder_20D2EA0.asm` lines 36 to 62 and `code/6142/track_lookup_20CFD40.asm` lines 31 to 44 (index = instrument x a static + difficulty); the `.chart` side is the decompile lines 25570 to 25594. The per-track list is proven; the block stays Unknown because the `.mid` half, which most charts use, was not read, and Hydra's side was not checked.
Game code (6142): the track lookup 0x20CFD40; the `.chart` reader `0x213E0C0`; the `.mid` reader `0x2155050`, not read for this.
Hydra: `Song::check_activations` in src/core/song.cpp line 105 runs once per loaded difficulty; whether the `.mid` parser hands every difficulty the same fills was not checked.
Ruling: none found
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-30 A stitched chart loses its generated fills unless every song's fills are written

Rule: A stitched chart (several songs joined) gets no generated fills once any part of it holds a written fill, because the test is track-wide (FL-27).
Label: Proven in the game's code
Evidence: follows from FL-27's listing; the stitched chart itself was never loaded in Clone Hero (card for session 436770bd, line 25). In that session, 24 of the source songs had no fill markers, so the stitched chart would have lost all their activation fills (card line 11). One more source, Metropolis Pt. 1, got no fills written at first; the card says the cause was not found. One possible cause is that its one written fill landed on no chord, which FL-28 would count as stopping generation; no session record shows that was the cause. The way round it is to write every song's fills out: take each source's fill chords from `hydra_replay score` (the `is_fill` chords) and write an explicit fill span ending on each one.
Game code (6142): RVA `0x20D2EA0`, `code/6142/note_builder_20D2EA0.asm` lines 278 to 299.
Hydra: `Song::check_activations` in src/core/song.cpp lines 106 and 107.
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build).

## The search position

The search position of the written-fill search is FL-04, and the generator's is FL-15 and FL-18.

## The fill-end miss grant

### FL-31 A missed fill-end note is hit for you when Star Power can activate

Rule: When Star Power can be activated, a missed fill-end note is granted as a hit (the first branch of the miss handler). A correct press on a fill-end note while Star Power can activate first makes a virtual call, which the drum-lanes notes read as activating Star Power, then hits the note.
Label: Proven in the game's code
Evidence: committed listing `docs/audit/drum-lanes/f20dcad0.asm` lines 6 to 16 (the call to `0x20F57F0`, the Star Power ready test, then the flag getter `0x214BB50` on the note, then the branch). The press side is `docs/audit/drum-lanes/play/f20deaa0.asm` lines 120 to 130; that its vtable call is the activation is the FINDINGS file's reading, and the slot was not resolved.
Game code (6142): RVA `0x20DCAD0`, `docs/audit/drum-lanes/f20dcad0.asm` lines 6 to 16; the press handler RVA `0x20DEAA0`.
Hydra: no owner: a grep of `src/` for a fill-end grant finds nothing; Hydra's paths hit every note.
Ruling: none needed: Hydra's paths hit every note, so the grant changes nothing they score
Found: drum-lanes FINDINGS-play-time.md.

## When a fill shows up

### FL-32 When a fill appears on screen in the game

Rule: When the game draws a fill on the highway, as against when it sets the activation flag, is not known.
Label: Unknown
Evidence: looked in the builder, the generator's record fields, the decompile, `docs/audit/ch-evidence.md` and `docs/audit/drum-lanes/FINDINGS-play-time.md`; no fill-draw function is named. The list's consumer is the drum engine, not read for drawing.
Game code (6142): not found: looked in `0x20D2EA0`, `0x20D2B30` and the decompile.
Hydra: the Preview's own rule is FL-33.
Ruling: none needed: no game rule known
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-33 The Preview lights a generated fill for the half measure before its chord

Rule: Hydra's Preview lights a generated 1.1 fill for the half measure before its chord, as a display-only choice; engine timing is unchanged.
Label: Hydra assumes it, never checked
Evidence: `kGeneratedFillDrawnMeasures` and `drawn_fill_start_tick` in src/app/preview_view.cpp lines 67 to 82.
Game code (6142): not found (FL-32).
Hydra: `drawn_fill_start_tick` in src/app/preview_view.cpp line 75.
Ruling: card 7bd3ac91, the user: "Bring back the old look: light the lanes for the half measure before the chord, display only, engine timing unchanged."
Found: card 7bd3ac91.

### FL-34 An E1 counts as no skip for the best all-0 path

Rule: An activation that passed over only its own early fill still fits the best all-0 path. This is a Hydra rule, not a game fact.
Label: Hydra assumes it, never checked
Evidence: `allzero_activation` in src/core/model.h line 347.
Game code (6142): none: not a game rule.
Hydra: `allzero_activation` in src/core/model.h line 347.
Ruling: D102, the user's words: "allow E1 paths. if you dont get the early fill you arent skipping anything, you still activate immediately"
Found: 2026-10-07, D102.

## Build 1.0.0.4045

### FL-35 In 1.0 any written fill on a track stops generated fills

Rule: In Clone Hero 1.0 a drum track with even one written fill gets no generated fills; the choice is made once per track by the fill list's count.
Label: Proven in the game's code
Evidence: `code/4045/fill_generator.cs` lines 19 to 27; line 20 tests `Count == 0` to choose the generator, else the written fills are placed. The same count-before-landing question as FL-28 applies.
Game code (4045): the 1.0 note builder (scrambled class and method names), `code/4045/fill_generator.cs` lines 19 to 27.
Hydra: `Song::check_activations` in src/core/song.cpp lines 105 to 107, before the 1.0 or 1.1 choice at line 112.
Ruling: none needed: same rule, except FL-28's case (Q-37)
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-36 The 1.0 generator differs from Hydra's 1.0 generator

Rule: The 1.0 generator tries the second measure line first and then every fourth line, and after a line with no chord close enough it tries the very next line. A chord counts as close enough within 0.25 seconds of real time of the line. The fill starts half way, in real time, between the previous measure line and the chosen line, and ends one 64th note (resolution x 12 / 192 ticks) after the chord's activation note. Hydra's 1.0 generator keeps the nearest chord per line, does not start at the second line, uses a limit of half a beat, and makes the fill half a grid measure long.
Label: Proven in the game's code
Evidence: `code/4045/fill_generator.cs` lines 30 to 77: line 40 (`i = 1`), 44 to 49 and 53 to 56 (`i += 4`; `i -= 3` on a miss), line 53 (`num5 > 0.25f`), lines 39, 42, 43 and 58 to 74 (the fill's start and end). Hydra's side: `Song::place_fills_ch10` in src/core/song.cpp lines 135 to 167 and `kCh10FillLengthMeasures` at line 123.
Game code (4045): the 1.0 generator (scrambled name), `code/4045/fill_generator.cs` lines 30 to 77.
Hydra: `Song::place_fills_ch10` in src/core/song.cpp line 135.
Ruling: open split: see questions page Q-38. D115 item 6 says "Clone Hero 1.0 mode keeps its old fill generator exactly." (the decision log's words), and card c0e1259d has the user choosing "Keep 1.0 exactly as before (Recommended)"; both came before the 1.0 code was read, so neither names the game's side.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-37 The 1.0 fill's activation note is the highest pad

Rule: The 1.0 fill's activation note is the highest drum pad in the chord (the highest set bit of the pad lanes); a chord with only kicks uses the lowest kick bit.
Label: Proven in the game's code
Evidence: `code/4045/fill_generator.cs` lines 117 to 141, with the two bit helpers named in `code/4045/fill_generator.md`.
Game code (4045): a scrambled method of the 1.0 note builder, `code/4045/fill_generator.cs` lines 117 to 141.
Hydra: `Chord::activation_note` in src/core/model.cpp; not compared with this rule.
Ruling: none found
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-38 A 1.0 written fill lands within a 32nd note, with no +1

Rule: A 1.0 written fill is attached to the chord nearest its end tick, no earlier than the fill start and no later than the end plus a 32nd note (resolution x 6 / 192 ticks). A tie goes to the later chord, a fill with no chord is dropped, and its length is rebuilt to end one 64th note after the activation note. Hydra places written fills the same way under 1.0 and 1.1, with 1.1's +1 tick.
Label: Proven in the game's code
Evidence: `code/4045/fill_generator.cs` lines 80 to 113 and the helper at lines 144 to 193. Hydra's side: `place_authored_fills` takes only a slop and has no rule argument (src/parse/song.cpp line 363, called at lines 1119 and 1874).
Game code (4045): a scrambled method of the 1.0 note builder, `code/4045/fill_generator.cs` lines 80 to 113.
Hydra: `place_authored_fills` in src/parse/song.cpp line 363.
Ruling: open split: see questions page Q-39
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-39 A 1.0 written fill on the coda tick is skipped

Rule: A 1.0 written fill that starts on the same tick as the chart's "coda" event is skipped, because that is the big rock ending.
Label: Proven in the game's code
Evidence: `code/4045/fill_generator.cs` lines 88 to 91; the coda object is made from an event named "coda" in the 1.0 decompile.
Game code (4045): a scrambled method of the 1.0 note builder, `code/4045/fill_generator.cs` lines 88 to 91.
Hydra: no owner: a case-insensitive grep of `src/` for `coda` finds no chart rule.
Ruling: open split: see questions page Q-40
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-40 The 1.0 measure lines restart at every meter change

Rule: In 1.0 the measure lines come from a beat walk that restarts its counters at every time-signature change. A numerator of 1 makes every beat after the first a beat, not a measure.
Label: Proven in the game's code
Evidence: `code/4045/fill_generator.cs` lines 197 to 255 (209 to 214, 215 to 218 and 221 to 231). That the two fields are the numerator and denominator is a reading from how the step uses them, not checked.
Game code (4045): a scrambled grid method, `code/4045/fill_generator.cs` lines 197 to 255.
Hydra: `MeasureIndex`, used by `Song::place_fills_ch10` at src/core/song.cpp line 136; the numerator-1 case was not compared.
Ruling: none found
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-41 The 1.0 game vets fills once, when the meter crosses half

Rule: The 1.0 game vets fills only at the moment the Star Power meter rises across one half while Star Power is not active; it does not look again while the meter stays above half.
Label: Proven in the game's code
Evidence: `code/4045/fill_deadline.cs` lines 7 to 14: line 9 holds the test, line 11 the call.
Game code (4045): a scrambled method of `DrumsPlayer`, `code/4045/fill_deadline.cs` lines 7 to 14.
Hydra: `activation_fill_deadline_ms` in src/search/graph.cpp line 62; whether the graph models "the meter crosses half at that moment" was not checked.
Ruling: none found
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-42 The 1.0 deadline is the fill's own length, held between 0.25 s and 10 s

Rule: A 1.0 fill loses its activation flag when it starts earlier than now plus the larger of its own length and 0.25 s; fills starting more than 10 s ahead are never examined. So the lead is the fill's length (to the activation note plus a 64th note), held between 0.25 s and 10 s of real time.
Label: Proven in the game's code
Evidence: `code/4045/fill_deadline.cs` lines 17 to 31 (21 to 24 the 10 s stop, 25 to 29 the 0.25 s and length test). Hydra's Ch10 branch: preroll = max(250, min(fill end - pad, 10000)) ms, with the pad measured from the tick resolution / 16 before the fill start (src/search/graph.cpp lines 82 to 90). The public Python Hydra 1.2 had the same formula (`git show ce3edf94^:hydra/hypath.py` lines 226 to 236). The two agree at a steady tempo; across a tempo change the 64th note is measured from a different tick. Over the library's 18,494 drum charts on 2026-08-30, Hydra scored 17,815 the same under the 1.0 and 1.1 deadlines, 600 higher under 1.0 and 79 higher under 1.1, the largest gain Night Verses at +28,900 (card d69fdea1 line 23; the report build-cpp/fillcompare/fill_compare.html is not committed). A chart can score higher under 1.1 because a fill longer than 4 beats needs more lead under 1.0 (FL-19). These are Hydra's numbers under the two deadlines with Hydra's own fill generator, not game runs; FL-36 shows the game's 1.0 generator differs from Hydra's, so they are not the game's 1.0 scores.
Game code (4045): a scrambled method of `DrumsPlayer`, `code/4045/fill_deadline.cs` lines 17 to 31.
Hydra: `activation_fill_deadline_ms` (the `FillDeadlineRule::Ch10` branch) in src/search/graph.cpp lines 62 to 91.
Ruling: none needed: same rule at a steady tempo; the tempo-change difference is part of Q-38
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-43 A 1.0 run started a fill that the 1.1 rule rejects

Rule: In Splax's Desecration Day full combo on Clone Hero v1.0.0.4080 (video uploaded 2026-03-17), the player activated at tick 25680, a fill the 1.1 rule rejects, and scored 955,660, above Hydra's 1.1 optimal 955,500. Hydra's 1.0 rule lets that fill through: the player's activations equal Hydra's 1.0 optimal path.
Label: Proven in a live run
Evidence: `C:\ytdl\videos\Desecration Day Pro Drums FC [68yrLU78FLo].fcreport.md` line 23 (activation 1 at tick 25680) and lines 36 to 38 (optimal 959,340 with Hydra path 0 `0 3- 4- 4 1 0`, actual 955,660); the results screenshot shows "Clone Hero v1.0.0.4080-final". The build is 4080, not 4045, and it is a third-party video, not a controlled run. It shows Hydra's rule agrees with one fill, not the formula. Hydra's own timing of that fill: under the 1.1 rule the meter reaches half 363 ms too late for the fill to appear, and under the 1.0 rule it appears with 212 ms to spare (session d46444c2, condensed transcript line 67; Hydra's numbers, not read from the game). The fcreport's lines 37 and 38 put the played path's ceiling at 957,320 and the run 1,660 below it.
Game code (4045): see FL-42.
Hydra: `kLegacyFillsFlag` in src/app/config.h line 53; `fill_rule_for` in src/search/graph.h line 45.
Ruling: none needed: same rule
Found: card d46444c2.

### FL-44 Hydra picks the 1.0 rule only from a flag or setting

Rule: Hydra uses the 1.0 rule only when told to, by `--legacy-fills` or the app's 1.0 fills setting; it never detects a chart's or a run's game version.
Label: Hydra assumes it, never checked
Evidence: `fill_rule_for` in src/search/graph.h line 45 and `kLegacyFillsFlag` in src/app/config.h line 53.
Game code (4045): none: not a game rule.
Hydra: `fill_rule_for` in src/search/graph.h line 45.
Ruling: D115 item 6: "`hydra_batch` may write `--legacy-fills` results into the app's own hydra.db" (the decision log's words).
Found: 2026-10-30, session b02ab3e6 (the reference build).

### FL-45 Since 2026-10-24 Hydra's 1.0 generator counts measures on Clone Hero's grid

Rule: Hydra's 1.0 generator counts measures on Clone Hero's grid, not the old Python counter, so it no longer equals Python Hydra 1.2 on charts with mid-measure meter changes. Colors (BTBAM), played on 1.0.0.4080, used 57 activations under both rules, so it carries no fill-rule evidence.
Label: Unknown
Evidence: not checked: the card's "11 of 97 corpus charts changed fills, no best score moved" needs a recount, and the Colors numbers (8,409,815 played; 8,729,975 under 1.0 and 8,727,795 under 1.1, card f520253b lines 20 and 21) need the run's `refs.json` entry opened. The code side was read: src/core/song.cpp lines 136 and 146 to 148 use `measure_index`.
Game code (4045): see FL-40.
Hydra: `Song::place_fills_ch10` in src/core/song.cpp line 136.
Ruling: card a124ccd9, the user: "Yes. Move the 1.0 generator onto MeasureIndex/MeasureCount, keep only its own fill rule, and show me which test-corpus charts' 1.0 fills and scores move before merging."
Found: cards a124ccd9 and f520253b.

## Rows waiting on a live run

FL-18, FL-24, FL-25, FL-27, FL-28, FL-29, FL-32. See `live-checks.md`.

## Open splits

FL-18 (Q-36), FL-28 (Q-37), FL-36 and FL-42 (Q-38), FL-38 (Q-39), FL-39 (Q-40). See `questions.md`.
