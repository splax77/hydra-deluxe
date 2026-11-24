# Open questions

This page lists every split between Clone Hero and Hydra that has no ruling yet, each with an id. Under "Recorded differences" it also keeps the splits the user chose to record without changing Hydra.
It describes build 1.1 (v1.1.0.6142) unless a question says otherwise.
Last write-back: 2026-11-12.

Each question gives the game's real rule, an example from each end, and what Hydra does today with its owner. Every question here is named by the `Ruling:` line of at least one rule block on the area pages, and every `Q-nn` a `Ruling:` line names has its heading here.

Questions Q-76 to Q-79, and Q-72, are tool findings in `tools/ch_probe/constants.py`, not splits between the game and Hydra: no rule in src/ reads those values. Q-34 and Q-76 are the same finding, raised from page 3 and page 7.

## Questions

### Q-10 A spaced .chart mix event: a disco marker in the game, plain text in Hydra

The game's real rule: the .chart E handler tests the trimmed text with "starts with `[mix`" and "starts with `mix`" (code/6142/chart_e_handler_213CAF0.asm lines 116 to 168) and hands the whole text to the mix parser (lines 319 to 330), which turns underscores into spaces and splits on spaces (code/6142/mix_parser_215C750.asm lines 98 to 174). So `E [mix 3 drums0d]` with ordinary spaces is a marker.

One end: `E [mix_3_drums0d]` (one word). Both the game and Hydra read it as Expert disco on.
Other end: `E [mix 3 drums0d]` (four words). The game flips red and yellow on Expert from that tick; Hydra sees a plain text event and flips nothing.

What Hydra does today: `classify_chart_line` in src/parse/song.cpp lines 1305 to 1308 reads an E line as a disco marker only when the line splits into exactly two words (`t.count == 2`). Owner: the .chart reader in src/parse/song.cpp. The corpus file testdata/parse_edge/c20_disco.chart holds one spaced form and 18 files use underscores; the library was not searched. Page row NL-19.

### Q-11 Nested .chart solos: a stack in the game, one flag in Hydra

The game's real rule: `solo` pushes a new solo record and `soloend` pops the most recent one, writing length = end tick minus start tick plus 1; a `soloend` with nothing open is ignored (code/6142/chart_e_handler_213CAF0.asm lines 231 to 306).

One end: `solo` at 0, `soloend` at 200 (no nesting). Both agree: one solo from 0 to 200.
Other end: `solo` at 0, `solo` at 100, `soloend` at 200, `soloend` at 300. The game ends the inner solo at 200 and the outer at 300, so ticks 201 to 300 are inside a solo. Hydra turns its flag off at 200, so ticks 201 to 300 are outside any solo. How the score code reads the outer record is unread (CR-35), so the score effect is not yet shown.

What Hydra does today: `flag_solo_` (a plain bool, line 1495 of src/parse/song.cpp), which `op_solo` sets at line 1466. Lines 1706 and 1710 turn the `solo` and `soloend` events into the operations that reach it. Owner: the .chart reader. Page row CR-34.

### Q-12 Mix-event spellings: separators and digit forms

The game's real rule: the mix parser drops one [ and ], turns underscores into spaces, needs the first space-separated word to be exactly "mix", reads the next word with an integer parse (0 to 3), then needs the rest after trimming to start with "drums" (code/6142/mix_parser_215C750.asm lines 59 to 290).

One end: `[mix.3.drums0d]` or `[mix-3-drums0d]`. Hydra reads them as markers (the corpus file c20_disco.chart line 63 uses `mix.2.drums1d`); the game finds no space and rejects them.
Other end: `[mix  3  drums0d]` (two spaces) or `[mix 03 drums0d]`. The game accepts them; Hydra's fixed byte positions reject them.

What Hydra does today: `disco_head` in src/parse/song.cpp lines 304 to 307 needs "mix", one byte of any kind, the difficulty digit, one byte, then "drums"; `peel_brackets` lines 294 to 298. Owner: the disco marker matchers in src/parse/song.cpp. Related and ruled: D19's on/off letter rule. Page row NL-18.

### Q-13 The fifth drum pad

The game's real rule: .chart N 5 and .mid pitches 65, 77, 89 and 101 are a fifth pad (type 17), with N 38 and N 44 as its accent and ghost (NL-04, NL-05). At load the game converts a five-pad chart to four-lane Pro Drums for every player: yellow to the yellow cymbal, blue to the blue tom, the fourth pad to the green cymbal and the fifth pad to the green tom, or the blue tom when the fourth pad shares its tick (NL-75). A chart with fifth-pad notes and Pro Drums marks takes the four-lane branch unless song.ini sets five_lane_drums (NL-74, NL-76). Five bot runs of 2026-11-12 on the Five Lane A to E probe charts confirm the branches, the key and the merge (NL-74 to NL-77).

One end: a chart with only N 0 to 4 and no five_lane_drums key. Both agree.
Other end: Tool's Schism (Neversoft chart), with 166 fifth-pad notes on Expert. The game scores Notes 106,125 by the 6142 rule. Before the conversion Hydra dropped the 166 notes, paid blue as a cymbal and gave Notes 100,480; now it gives 106,125.

What Hydra does today: `convert_drum_pads` in src/parse/song.cpp makes the game's conversion once for both readers, from `DrumChartFlags` and song.ini's two keys (`ChartDrumKeys`). Owner: that function. Page rows NL-04 and NL-74 to NL-77. `hydra_replay score --prodrums 1` matches the bot totals on all five probe charts, Five Lane A to E.

Decided and built: in session c61c1986 (2026-11-12) the user chose "Yes, start it" on "Start the plan with the recommended answers (four-lane only, follow the game, bump the stamp, no scan)?" (D337 in docs/audit/2026-10-03-fix-decisions.md). So Hydra follows the game's conversion for a four-lane player, the FiveLaneDrums kit player stays out of scope, saved results were marked Stale once, and the library is not scanned.

The all-toms tail is built too. For a chart with neither flag up, the game's four-lane branch ends by making every pad but the kick a tom (NL-74), and probe F's bot run showed it: 5,750. In the same session the user typed "Yes, plan it" to copying it, and "yes run it as workflow" to "Start the all-toms plan with the four recommended answers?" (D340 in docs/audit/2026-10-03-fix-decisions.md). So `convert_drum_pads` now ends its four-lane branch with the tail, and `hydra_replay score --prodrums 1` gives probe F's 5,750. song.ini's two keys are read by the game's getter rule, so `pro_drums = 1` keeps a .mid's cymbals, as probe G's run showed (CR-18, CR-54). The library scan the user allowed found 8 charts that move, all loose .mid (NL-74).

### Q-14 Two pads of one colour on one tick

The game's real rule: the track finalizer merges notes with the same tick and the same note type into the longest one (first on a tie), or-ing every flag bit except the low three (code/6142/kick_merge_215B8C0.asm lines 54 to 128); nothing limits it to kicks.

One end: a 2x kick and a normal kick on one tick. Hydra merges them too (`Chord::add_kick`, D105), so both agree.
Other end: a .mid with two red note-ons on one tick under the dynamics tag, velocity 127 then velocity 1. The game keeps one red note carrying both the accent and the ghost bit. Hydra's second `Chord::add_note` throws "Duplicate note." (src/core/model.cpp line 244) before the mark is set, and the per-op catch (src/parse/song.cpp line 1047 for .mid, 1809 for .chart) drops it, so Hydra keeps one red note with only the accent.

What Hydra does today: `Chord::add_note` in src/core/model.cpp line 242. Owner: the chord model in src/core/model.cpp. Page row NL-32. The five-lane conversion's one same-lane case (blue, fourth pad and fifth pad on one tick) is merged in `convert_drum_pads` instead (NL-77).

### Q-15 A .mid "[sectionX]" with no space

The game's real rule: the events reader tests "starts with `[section`" (docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm lines 100 to 125) and cuts the name from character 9 (lines 146 to 158), so it never checks for the space.

One end: `[section Intro]`. Both read a section named Intro.
Other end: `[sectionX]`. The game would make a section with a short or empty name; Hydra makes none. Not run in the game.

What Hydra does today: `marker_name` in src/parse/section_names.cpp line 19 needs "section " with its space. Owner: src/parse/section_names.cpp. Page row CR-29.

### Q-16 A .chart event with text outside its quotes

The game's real rule: the .chart [Events] reader takes the event's text from just after the first double quote on the line to just before the last one, then tests that text for a section (page 1, CR-32, which cites the decompile). What it does with a line that holds only one quote was not worked out.

One end: `E "section intro"`. The game and Hydra both read the text `section intro`, so both make a section.
Other end: a hand-edited line with text outside the quotes. For `E "section intro" x` the decompile has the game read `section intro`, which would make a section; Hydra keeps the whole `"section intro" x`, which does not start with "section ", so it makes none. For `E intro "x"` the game reads `x` and Hydra reads `intro "x"`; neither is a section, but the two texts differ. No chart with such a line has been tried in the game.

What Hydra does today: `classify_chart_line` in src/parse/song.cpp lines 1314 to 1316 trims the text after `E` and takes one pair of quotes off only when the trimmed text both starts and ends with a quote; otherwise it keeps the text whole. That text then goes to `section_name_of` (line 1635) and to the end-event test (lines 1639 to 1643). Owner: the .chart reader in src/parse/song.cpp. No library count (no library run), so how many charts hold such a line is not known. The question: should Hydra's .chart reader take the text between the first and last quote, as the game does, or keep its whole-text rule? Page row CR-32.

### Q-30 A song.ini delay with a decimal point

The game's rule: the ini getter at 0x21647A0 tests `String.Contains(".")` on the delay text; with a "." it parses a float and returns trunc(value x 1000), so the text is seconds; without one it parses an integer of milliseconds; unreadable text gives 0 (code/6142/ini_int_getter_21647A0.asm lines 32 to 66, row TM-02).

One end: `delay = 500` is 500 ms in both the game and Hydra.
Other end: `delay = 0.5` is 500 ms in the game and 0.5 ms in Hydra.

Hydra today: `parse_delay_ms` (src/app/preview_source.cpp line 113) reads any finite number as milliseconds. It moves only the Preview's audio, never a score.

### Q-31 The chart Offset is rounded up to whole milliseconds

The game's rule: when the metadata delay is still 0, the chart reader stores ceil(Offset x 1000) whole ms (code/6142/chart_song_metadata_35C980.asm lines 634 to 663, row TM-03).

One end: `Offset = 0.25` is 250 ms in both.
Other end: `Offset = 0.2504` is 251 ms in the game and 250.4 ms in Hydra; a negative Offset also rounds up.

Hydra today: `preview_audio_offset_ms` (src/app/preview_source.cpp line 311) keeps the fraction. D274 item 6 ("Check the game, then honor") rules on the choice, not on the rounding.

### Q-32 The game's per-note hit window against Hydra's fixed 85 ms

The game's rule: each side's half-gap to the neighbour is clamped into [front, back] (0.0375 to 0.085 s normal, 0.025 to 0.04 s precision), the two are summed and passed to the formula; normal mode gives ((x x 0.0110924369747899 - x^2 x 2.628539e-05) x 170 - 20) ms, and each side gets half (code/6142/half_gap_clamp_20F7210.asm, note_window_20F3A90.asm, hit_window_formula_20DDDA0.asm; rows TM-17 to TM-23, TM-27, TM-31 to TM-33).

One end: a note with both neighbours 170 ms or farther gets 85.72 ms a side, close to Hydra's 85.
Other end: a note with both neighbours 75 ms or closer gets 48.15 ms a side (in precision mode about 25 ms), against Hydra's 85.

Hydra today: `kTimingStepMs = 85.0` (src/core/model.h line 114) for badges, Ratings, tiers and the squeeze search. The early-fill reach is the `Early fill window` setting, at most this number (FL-24). D270 item 8 rules on Hydra's own two numbers only; D28 shelved the edge steps. Whether the hit check holds a press to the stored window was measured only on notes with the widest side, 85.72 ms; the edge on notes with closer neighbours is not proven (TM-27).

On 2026-11-12 (session 375e58cd) the user ruled on the row twice and on the search once. On the row, first: "just show it alongside. try again with a maximum of 8 words for the new info." Later, when the row first left some badges out: "i explicitly asked for the new info to show on all 3 kinds of badges. why did you not build it?" and "i didn't ask you to invent your own rules. i asked you to build what i asked." So the Paths tab's Dynamic Hit Window row shows the game's per-note window beside the 85 for each squeeze in, squeeze out and early fill badge, with how much it leaves spare or falls short, and nothing else moves. Its owner is src/core/note_window.h. On the search: "ok well i am ruling it now: hydra searching for squeezes up to 500 ms is an intentional deviation from the game, not a bug." So the search still finds squeezes the game's windows cannot cover, and the row is where such a squeeze shows as short. The question stays open: none of these rulings says whether the ratings, tiers or early-fill reach should ever follow the game's window, and precision mode is not covered.

### Q-34 The formula tables in constants.py carry each other's names

This is the same finding as Q-76, raised from page 3.

The game's rule: the formula at 0x20DDDA0 uses one constant set when bit 0x1000 is clear and the other when it is set (decompile lines 767 to 781, row TM-28).

One end: `RVA_FORMULA_PRECISION` (tools/ch_probe/constants.py line 175) holds the bit-clear constants, which reproduce the live 96.29 to 171.43 ms.
Other end: `RVA_FORMULA_NORMAL` (line 166) holds the bit-set constants. The probe's normal-window prediction was fed from it, giving -242 and -78 ms; it lived in tools/ch_probe/experiments/analysis.py (removed; `git show a081f660:tools/ch_probe/experiments/analysis.py`).

Hydra today: no `src/` owner; the fix is a rename in the tool.

### Q-36 The generator at the tail of a song

The game's rule: when no chord is strictly after a measure line, the nearest-chord wrapper 0x5DDB50 searches downward from the carried start index (the last chord that took a fill), so it compares the line with that older chord and the line is skipped (code/6142/nearest_chord_loop_5DDB50.asm lines 29 to 45, back_search_2C7CE00.asm lines 20 to 34, fill_generator_20D2B30.asm lines 116 to 126; row FL-18).

One end: a line with a chord after it: both pick the same chord.
Other end: the song's last chord a few ticks before the final measure line, nothing after it, the last fill four or more lines earlier: Hydra places a fill on that chord, the game does not.

Hydra today: `Song::place_fills_ch11` (src/core/song.cpp lines 202 to 212) takes the true last chord.

### Q-37 A written fill that lands on no chord still stops generation in the game

The game's rule: the note builder compares the track's written-fill list count with 0 before any fill is landed: `cmp dword ptr [rax+0x18], r13d; je` to the generator, with r13d = 0 (code/6142/note_builder_20D2EA0.asm lines 280 to 299; rows FL-27, FL-28, FL-35; the 1.0 build does the same, code/4045/fill_generator.cs line 22).

One end: a chart whose written fills all land: neither generates fills, same result.
Other end: a chart whose only written fills reach no chord: Hydra generates fills across the chart, the game generates none.

Hydra today: `place_authored_fills` (src/parse/song.cpp line 383) drops the unlanded fills, then `Song::check_activations` (src/core/song.cpp lines 138 and 139) returns only if a chord holds one. A written fill the coda skipped is not this case: it stops generation as in the game (src/core/song.cpp line 137, rows FL-27 and FL-08).

### Q-39 A 1.0 written fill's landing window has no +1

The game's rule (1.0): a written fill lands no later than end + resolution x 6 / 192 ticks, with no +1 (code/4045/fill_generator.cs lines 82 to 115; row FL-38). In 1.1 it is trunc(resolution / 32) + 1 (FL-03).

One end: a chord inside res/32: both builds and Hydra land it.
Other end: a chord exactly res/32 + 1 ticks after the end: 1.1 and Hydra land it, 1.0 drops the fill.

Hydra today: `place_authored_fills` (src/parse/song.cpp lines 390 and 391) uses 1.1's +1 under both rules.

### Q-51 Hydra pays every accent and ghost; the game pays one only when the hit is marked as performed

The game's real rule. The drum score method (RVA 0x20DEEC0, `code/6142/note_score_add_20DEEC0.asm` lines 19 to 24) doubles a gem's value only when the DynamicsHit flag (mask 4) of note +0x84 is set (`call` the flag test, `lea r8d,[rbx+rbx]`, `cmove`). The name is the dump's: the word at +0x84 has the enum type whose value 4 is DynamicsHit (`dump.cs` lines 548388 and 548231; page 6, SC-12). The hit handler (RVA 0x20DE770, `code/6142/note_hit_dynamics_20DE770.asm` lines 29 to 102) sets that mark by comparing the lane's signed byte (engine +0x309 to +0x310) with the threshold at +0x2C0: a ghost when the byte is at least 0 and below the threshold, an accent when it is at or above it, nothing when it is negative.

Example from one end: the ProDrums bot on Trippin SP Spike hit all 9 accents and 34 ghosts as performed (scorestats-Trippin_SP_Spike-20261028-212754.json: accent_notes_hit 9, ghost_notes_hit 34), so its score 290,975 equals Hydra's.

Example from the other end: run 1, a key-press run on the same chart (`run1_cal0_scorestats.json`), counted 0 of 9 and 0 of 34; base_score 73,520 sits 2,285 above note_score 71,235, and the running score difference (game minus Hydra) in run1_cal0_score_compare.txt steps down at each of the 43 accent and ghost chords, ending 13,760 below Hydra's total of 378,855 (the game also made no fifth activation, so not all of that gap is dynamics). The user said in session 6a549de0: "the code is the source of truth but from experience i can tell you dynamics only work with a real drum kit. keyboard inputs never register dynamics."

What Hydra does today. `category_scores` in src/core/scoring.cpp (the accent and ghost extras, `kNoteBasePoints` each) pays every accent and ghost as performed, for every player; there is no velocity or performed state in Hydra. The question: keep pricing every dynamic as performed (the kit player's optimum, today's behaviour), or show the keyboard case (no dynamic credit) somewhere? D208 ("we don't need to change anything, matching the game is correct.") is about which breakdown line the extra points sit in, not this.

### Q-72 The tools call 0x100DAB0 the hit check, but it is the other instantiation's method

The game's real rule: `0x100DAB0` is virtual slot 25 of the generic base #8253 instantiated on the 8-byte input struct #8533. The drum engine's own copy of that slot is `0x100DAE0`, on the #8663 instantiation (`dump.cs` lines 527331 to 527338; the drum engine class #8255 derives from the #8663 instantiation at line 527421). The drum copy compares the previous input struct with the current one and calls `0x2176020`, whose body was not read (GD lines 2247 to 2275). Neither copy is shown to be a hit check. Page row IN-22.

One end: a hook at `0x100DAE0` during a drums song fires every frame. hydra-data/ch-mcp-runs/2026-10-28/l3/trace-wm-count.txt (outside the repo) counts 20,831 calls in a 62 s song with 142 notes, so it is not once per note.
Other end: a breakpoint at `0x100DAB0`, where the tool points, never fires in a drums song, because the drum engine never runs that instantiation.

What the tool does today: `RVA_HIT_CHECK = 0x100DAB0` (tools/ch_probe/constants.py line 55), with a comment at lines 52 to 54 calling it the hit-decision comparison. The runner that set a breakpoint there was tools/ch_probe/experiments/active_probe.py (removed; `git show a081f660:tools/ch_probe/experiments/active_probe.py`), line 215. Owner: tools/ch_probe/constants.py. Nothing in src/ reads it. The question: point the constant at the drum copy `0x100DAE0` with a comment that says what the code shows (a per-frame input compare), or drop the constant (the breakpoint went with the removed runner). Q-77 is the same kind of slip and could be fixed in the same change.

### Q-74 Vocal notes move Hydra's song end, but not the game's

The game's real rule: the .mid vocals method (RVA `0x21579B0`, decompile lines 43955 to 44246) writes its pitches and phrase markers only to the chart-wide lyric list, through 0x2152CC0 (calls at lines 44067 to 44231). No direct call to the gem adder 0x20CEC60 appears in lines 43900 to 44300. The game's last note end is the largest gem end over every track's gem list (CR-43, code/6142/chart_length_212E130.asm lines 33 to 63), so a vocal note never moves it. Two callees were not read: the track lookup 0x20CFD40 (line 44151) and FUN_18212d1e0 (lines 44227 and 44239). The .chart vocals and the binary note-stream loaders were not checked. Page row CR-44.

One end: a chart whose vocals end before its last drum or guitar note. Both take the same last note end.
Other end: a .mid whose vocals track runs past every instrument note, with no end event and audio shorter than the vocals. Hydra's last note end is the last vocal note; the game's is the last instrument gem, so the two song lengths differ.

What Hydra does today: `MidiFile::last_note_end_tick` in src/parse/midi.cpp lines 464 to 487 reads a vocals track's low pitches as notes (`kVocalsTrackName` in src/parse/midi.h line 122). Owner: src/parse/midi.cpp, whose result becomes the one song end D255 made every part of Hydra use. D255 item 3 ruled "All instruments" for the last note end; it was decided before the vocals method was read, and it does not say whether vocals count as an instrument.

### Q-75 A .mid [end] event with white space round it: an end event in the game, plain text in Hydra

The game's real rule: the .mid events reader (RVA `0x21544E0`) trims the white space off both ends of the event text (docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm line 98), then tests it for equality with `[end]` (lines 218 to 243; the compare is an ordinal, case-sensitive SequenceEqual on character spans, not a starts-with test). A match goes to the end setter. So `[end]` with a space or tab before or after it is an end event. Page row CR-39, which is Proven; the exact and case-sensitive part of the test is the same in Hydra and is not in question.

One end: a text event `[end]`, or `[END]`, or `[end] x`. The game and Hydra agree on each: the first ends the song, the other two do not.
Other end: a text event `[end] ` (one trailing space) or ` [end]`. The game trims it and ends the song there; Hydra's .mid pass requires the whole text to start with "[" and end with "]", so it sees a plain text event and finds no end event.

What Hydra does today: the .mid pass at src/parse/song.cpp lines 1175 to 1184 skips any text that does not start with "[" and end with "]" before it calls `is_end_event` (line 46), and the text reader in src/parse/midi.cpp (line 131) trims nothing. Owner: src/parse/song.cpp. No library count (no library run), so how many charts hold a padded end event is not known. The question: should Hydra's .mid end test trim the text first, as the game does?

### Q-76 The two window-formula tables in constants.py carry each other's names

This is the same finding as Q-34, raised from page 7.

The game's real rule (GD lines 768 to 781, the window formula at RVA `0x20DDDA0`): `if ((*(uint *)(param_1 + 0x198) & 0x1000) == 0)` the branch returns `((dVar1 - dVar2 * _DAT_183140620) * _DAT_183140708 - DAT_183063fc0) / DAT_183064c28` with `dVar1 = param_2 * _DAT_1831406b0`. That is the normal branch (bit clear), and its four addresses are the ones `RVA_FORMULA_PRECISION` lists (constants.py lines 175 to 180). The bit-set (precision) branch uses the four `RVA_FORMULA_NORMAL` lists (lines 166 to 171). The comments above each table describe the right shape for the name but sit over the other branch's addresses.

One end: with the constants under `RVA_FORMULA_PRECISION` fed into the normal shape, an input of 170 ms gives 171.43126 ms and 75 ms gives 96.29316 ms, the two ends of `tools/ch_probe/experiments/results/poll_windows.csv`. Other end: the table now named normal gives -242 and -78 ms, which no run ever stored.

What the tool does today: no tool predicts a window now. The normal-window prediction read `RVA_FORMULA_NORMAL`, so it predicted the wrong curve; it lived in tools/ch_probe/experiments/analysis.py (removed; `git show a081f660:tools/ch_probe/experiments/analysis.py`), lines 227 to 251. The constants read in tools/ch_probe/engine.py still files each table's values under that table's name. Owner: tools/ch_probe/constants.py (a swap of the two table names). Nothing in src/ reads it. Page row IN-16.

### Q-77 RVA_BASE_ENGINE_CTOR_DRUMS holds the other instantiation's constructor

The game's real rule (`dump.cs` lines 527295 to 527306, `script.json`): `0x100E960` is the `.ctor` of #8253 on #8533 and on #8332; the drum engine's base is #8253 on #8663 (`dump.cs` line 527421), whose `.ctor` is `0x100EAE0`. hydra-data/ch-mcp-runs/2026-10-28/l3/trace-wm-count.txt (outside the repo) lists ".ctor (base 0x100eae0)" in the drum engine's hooked set.

One end: a hook placed at `0x100E960` during a drums song never fires for the drum engine. Other end: a hook at `0x100EAE0` fires once when the drum engine is built.

What the tool does today: `RVA_BASE_ENGINE_CTOR_DRUMS = 0x100E960` (tools/ch_probe/constants.py line 46). Nothing uses it except a test fixture copy (tools/ch_mcp/tests/test_dump_facts.py line 409). Owner: constants.py. Page row IN-23. Same family as Q-72.

### Q-79 OFF_SP_OVERFLOW (+0x130) is described as set only on an overfill, but every gain sets it

The game's real rule: the gain method `0x20F4FA0` sets the byte at +0x130 on every gain (`code/6142/sp_gain_20F4FA0.asm` lines 44 to 57, then it sends the new meter ratio), and the per-update method `0x20F6800` also sets it at its clamp (GD lines 53552 to 53557: amount above max sets amount = max and sets the flag). In the same method's Star-Power-on branch the flag is tested at line 53558, the end time is recomputed, and line 53572 clears it; no clear was found for the Star-Power-off case.

One end: a quarter-meter phrase with no clamp: the flag turns on (run1_cal0 first flip at 5.715 s with 1440 ticks in the meter). Other end: Star Power active: the flag reads 0 in all 11,436 active rows of run1_cal0.

What the tool does today: `OFF_SP_OVERFLOW = 0x130` (constants.py line 140) with the comment at lines 137 to 140 saying it is set only when a phrase overfills the meter. Owner: constants.py (name and comment). Page row IN-27.

### Q-83 Two sections on one tick: one section in the game, two in Hydra

The game's real rule: both chart readers hand each section to the same add-a-section method (RVA `0x212C350`). That method looks at the last section it stored. If the new one has the same tick, it overwrites that section's name and adds nothing (C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\ghidra\ghidra_decompile_all.txt lines 14575 to 14585, outside the repo). So the last name written on a tick wins, and the tick has one section.

One end: sections on different ticks. Both sides keep every one.
Other end: a .mid with "[section Intro]" and "[section Verse 1]" both at tick 0. The game has one section, "Verse 1". Hydra has two: the Preview time box says "Section Verse 1" (it takes the last section at or before the playhead), the timeline draws a zero-length "Intro" band, and `hydra_replay score` lists both.

What Hydra does today: `sort_practice_sections` in src/parse/song.cpp line 64 keeps both, in the file's order (a stable sort by tick). Owner: src/parse/song.cpp. No library count (no library run), so how many charts hold two sections on one tick is not known. The question: should the later name replace the earlier one on the same tick, as in the game? Page row CR-24.

### Q-84 A .mid section text with white space round it: a section in the game, plain text in Hydra

The game's real rule: the .mid events reader (RVA `0x21544E0`) trims the white space off both ends of the event text (docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm line 98) before it tests for a start of `[section` (lines 99 to 128). So `[section Intro]` with a space or tab before or after it is a section. Q-75 asks the same question for `[end]`.

One end: a text event `[section Intro]`. Both sides read a section named Intro.
Other end: a text event `[section Intro] ` (one space after the bracket). The game trims it and makes a section; Hydra sees a plain text event and makes none.

What Hydra does today: the .mid pass at src/parse/song.cpp line 1180 skips any text that does not start with "[" and end with "]", with no trim, before it calls `section_name_of` (line 1185). Owner: src/parse/song.cpp. No library count (no library run), so how many charts hold a padded section text is not known. The question: should Hydra's .mid reader trim the text first, as the game does? Page row CR-24.

## Recorded differences

Each entry below is a difference between the game and Hydra that the user chose to record, not change. It is closed. It keeps its heading because a block's `Ruling:` line names it. Hydra stays as it is.

### Q-50 A hit judged in a frame past the Star Power end pays without Star Power in the game (recorded, closed)

The game's rule (page 5, SP-28 and SP-29). When Star Power's end falls between the last frame's clock and this frame's clock, the event step queues the end time (`code/6142/advance_events_20F6A50.asm` lines 219 to 230). The catch-up then runs a nested frame update at that queued time, with the press route (slot 21) skipped; slot 18 still runs (page 5, SP-30) (`code/6142/event_catch_up_20F6FF0.asm` lines 95 to 104, the call at line 103; `code/6142/base_frame_update_20F4550.asm` lines 22 to 33 and 41). That nested update's end check ends Star Power (`code/6142/base_frame_update_20F4550.asm` line 37, `code/6142/sp_end_check_20F63B0.asm` lines 1 to 8). Only then does the real frame run its press route and judge its hits, at the frame clock. So a hit judged at a frame clock past the end pays single. Hits come first, with Star Power still on, only when the frame clock equals the end exactly: the end is then not queued, and the press route runs before the frame's own end check.

One end: the end is at 10,000.0 ms and a frame's clock is exactly 10,000.0 ms. A hit judged in that frame pays doubled.
Other end: the end is at 10,000.0 ms and the frame clock is 10,011.7 ms. The nested update at 10,000.0 ms has already ended Star Power, so a hit judged in that frame pays single.

What Hydra does: Hydra has no frames. It pays a note by its chart time against one end: `paid_by_sp_walk` (offset at or below 0) in src/core/backend_value.h line 28. How far the two can differ depends on the frame rate, which the player sets in the game's settings.

Ruling: the user chose to record this, 2026-11-11 (session 428cf079): "#3 isnt even possible to quantify because framerate is not permanently set, it can be changed in the settings". Page rows SP-28 and SP-29.

### Q-80 The game refuses a .srb whose version is not 20210228; Hydra never checks (recorded, closed)

The game's rule (page 1, CR-51): the .srb reader compares the song-info stream's first four bytes with 0x1346234 (20210228) and stops reading the bundle when they differ.

One end: a bundle with version 20210228. The game and Hydra both read it. None of the 30 shipped bundles is known to have another version.
Other end: a bundle with any other version. The game stops reading it; Hydra reads it as usual.

What Hydra does: `srb_parse_metadata` in src/parse/srb.cpp line 136 reads up to the checksum and never tests the version.

Ruling: the user chose "Record it only" (2026-11-08, session 77057df2; docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md line 18). Page row CR-51.

### Q-81 The game drops a drum lane whose first pad chord starts too late; Hydra keeps it (recorded, closed)

The game's rule (page 2, NL-58): the game drops a lane whose first chord with a non-kick pad, searched from one tick before the lane's start, lands more than resolution x 12 / 192 ticks after the start, rounded half to even (30 ticks at resolution 480). At exactly the start plus that cut-off the lane is kept.

One end: a lane whose first pad chord is on the lane's start. The game and Hydra both light it.
Other end: a lane whose first pad chord is 240 ticks in at resolution 480, as the double lane at tick 273,840 in Job for a Cowboy - Embedded is. The game drops the lane; Hydra lights it. The approved search of the library's notes files found 75 such lanes in 50 charts (user-answers.md line 21).

What Hydra does: `light_drum_lanes` in src/parse/song.cpp line 203 applies no cut-off.

Ruling: the user chose "Record it only" (docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md line 21). D112 ("Any note inside the lane (Recommended)") rules on which notes light a lane. Page row NL-58.

### Q-82 Two written fills on one chord: the game's deadline comes from the earlier fill, Hydra's from the later (recorded, closed)

The game's rule (page 4, FL-10): when two written fills pick one chord, the game keeps a record for each, and the chord's activation flag is cleared at the earlier-starting fill's deadline.

One end: two fills that start on the same tick. Both sides agree.
Other end: at resolution 480, fills from 9600 and from 10080 that both end on a chord at 10560. The game clears the flag at current tick 7680; Hydra prices the deadline at current tick 8160. Two charts in the library have such chords, and keeping only the earlier fill changes neither best score: Pangaea - A Gateway to Nothing stays 722,900 and RAT KING - Open Hands to the Void stays 260,970 (user-answers.md lines 24 and 25).

What Hydra does: `place_authored_fills` in src/parse/song.cpp line 405 keeps the later fill's length through `apply_fill_end` (line 354).

Ruling: the user, 2026-11-08: "Record it only" (docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md line 25). Page row FL-10.

## Reserved ids settled before write-up

Three of the six splits known before the reference was built turned out settled once the game's code was read. They have no heading here, and no `Ruling:` line names them.

Q-70 asked whether any written fill stops generated fills. The game's side is FL-27 on page 4: the note builder runs the generator only when the track's written-fill list is empty. The one case where Hydra still differs is narrower, a written fill that lands on no chord, and it is Q-37.

Q-71 asked why a 2x kick written before a normal kick on one tick lowered Hydra's Dynamics totals by one with 2x Bass off (the derivation audit of 2026-10-03). NL-29 on page 2 shows the game merges the two kicks whatever their order. Since D105, `Chord::add_kick` in src/core/model.cpp merges them in either order too, so the two agree. NL-30 waits on a live run of both file orders.

Q-73 asked about a song.ini delay equal to the chart Offset. TM-04 on page 3 shows both routes store the same milliseconds in the same field, so that edge changes nothing.
