# Open questions

This page lists every split between Clone Hero and Hydra that has no ruling yet, each with an id.
It describes build 1.1 (v1.1.0.6142) unless a question says otherwise.
Last write-back: 2026-10-31.

Each question gives the game's real rule, an example from each end, and what Hydra does today with its owner. Every question here is named by the `Ruling:` line of at least one rule block on the area pages, and every `Q-nn` a `Ruling:` line names has its heading here. The docs test checks both directions.

Questions Q-76 to Q-79, and Q-72, are tool findings in `tools/ch_probe/constants.py`, not splits between the game and Hydra: no rule in src/ reads those values. Q-34 and Q-76 are the same finding, raised from page 3 and page 7.

## Questions

### Q-10 A spaced .chart mix event: a disco marker in the game, plain text in Hydra

The game's real rule: the .chart E handler tests the trimmed text with "starts with `[mix`" and "starts with `mix`" (code/6142/chart_e_handler_213CAF0.asm lines 116 to 168) and hands the whole text to the mix parser (lines 319 to 330), which turns underscores into spaces and splits on spaces (code/6142/mix_parser_215C750.asm lines 98 to 174). So `E [mix 3 drums0d]` with ordinary spaces is a marker.

One end: `E [mix_3_drums0d]` (one word). Both the game and Hydra read it as Expert disco on.
Other end: `E [mix 3 drums0d]` (four words). The game flips red and yellow on Expert from that tick; Hydra sees a plain text event and flips nothing.

What Hydra does today: `classify_chart_line` in src/parse/song.cpp lines 1252 to 1259 reads an E line as a disco marker only when the line splits into exactly two words (`t.count == 2`). Owner: the .chart reader in src/parse/song.cpp. The corpus file testdata/parse_edge/c20_disco.chart holds one spaced form and 18 files use underscores; the library was not searched. Page row NL-19.

### Q-11 Nested .chart solos: a stack in the game, one flag in Hydra

The game's real rule: `solo` pushes a new solo record and `soloend` pops the most recent one, writing length = end tick minus start tick plus 1; a `soloend` with nothing open is ignored (code/6142/chart_e_handler_213CAF0.asm lines 231 to 306).

One end: `solo` at 0, `soloend` at 200 (no nesting). Both agree: one solo from 0 to 200.
Other end: `solo` at 0, `solo` at 100, `soloend` at 200, `soloend` at 300. The game ends the inner solo at 200 and the outer at 300, so ticks 201 to 300 are inside a solo. Hydra turns its flag off at 200, so ticks 201 to 300 are outside any solo. How the score code reads the outer record is unread (CR-35), so the score effect is not yet shown.

What Hydra does today: `flag_solo_` (a plain bool) in src/parse/song.cpp lines 1417, 1643 and 1647. Owner: the .chart reader. Page row CR-34.

### Q-12 Mix-event spellings: separators and digit forms

The game's real rule: the mix parser drops one [ and ], turns underscores into spaces, needs the first space-separated word to be exactly "mix", reads the next word with an integer parse (0 to 3), then needs the rest after trimming to start with "drums" (code/6142/mix_parser_215C750.asm lines 59 to 290).

One end: `[mix.3.drums0d]` or `[mix-3-drums0d]`. Hydra reads them as markers (the corpus file c20_disco.chart line 63 uses `mix.2.drums1d`); the game finds no space and rejects them.
Other end: `[mix  3  drums0d]` (two spaces) or `[mix 03 drums0d]`. The game accepts them; Hydra's fixed byte positions reject them.

What Hydra does today: `disco_head` in src/parse/song.cpp lines 293 to 296 needs "mix", one byte of any kind, the difficulty digit, one byte, then "drums"; `peel_brackets` lines 283 to 287. Owner: the disco marker matchers in src/parse/song.cpp. Related and ruled: D19's on/off letter rule. Page row NL-18.

### Q-13 The fifth drum pad

The game's real rule: .chart N 5 is a note of type 17 with the Tom flag and raises track flag +0xA1 (code/6142/chart_n_handler_213D620.asm lines 99 to 105); .mid pitches 65, 77, 89 and 101 are that pad (code/6142/pitch_difficulty_map_210D9C0.asm and the type map in docs/audit/note-shuffle/step0/open2/f210d7b0.asm); N 38 and N 44 are its accent and ghost (code/6142/chart_modifier_decode_213D160.asm lines 11 to 80). What the game then does with a type-17 note in four-lane Pro Drums is unread.

One end: a chart with only N 0 to 4. Both agree.
Other end: an Expert section with `N 5 0` lines and `N 38` accents. The game holds one more note on those ticks; Hydra's chord has one gem fewer and no mark.

What Hydra does today: `chart_pad_color` in src/parse/song.cpp lines 1668 to 1676 knows N 1 to 4, and `ChartParser::note_optype` lines 1700 to 1708 knows 34 to 37 and 40 to 43; the .mid reader takes `note <= base_ + 4`. Owner: the two readers in src/parse/song.cpp. No library count (no library run). Page row NL-04.

### Q-14 Two pads of one colour on one tick

The game's real rule: the track finalizer merges notes with the same tick and the same note type into the longest one (first on a tie), or-ing every flag bit except the low three (code/6142/kick_merge_215B8C0.asm lines 54 to 128); nothing limits it to kicks.

One end: a 2x kick and a normal kick on one tick. Hydra merges them too (`Chord::add_kick`, D105), so both agree.
Other end: a .mid with two red note-ons on one tick under the dynamics tag, velocity 127 then velocity 1. The game keeps one red note carrying both the accent and the ghost bit. Hydra's second `Chord::add_note` throws "Duplicate note." (src/core/model.cpp line 244) before the mark is set, and the per-op catch (src/parse/song.cpp line 1019 for .mid, 1746 for .chart) drops it, so Hydra keeps one red note with only the accent.

What Hydra does today: `Chord::add_note` in src/core/model.cpp line 242. Owner: the chord model in src/core/model.cpp. Page row NL-32.

### Q-15 A .mid "[sectionX]" with no space

The game's real rule: the events reader tests "starts with `[section`" (docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm lines 100 to 125) and cuts the name from character 9 (lines 146 to 158), so it never checks for the space.

One end: `[section Intro]`. Both read a section named Intro.
Other end: `[sectionX]`. The game would make a section with a short or empty name; Hydra makes none. Not run in the game.

What Hydra does today: `marker_name` in src/parse/section_names.cpp line 13 needs "section " with its space. Owner: src/parse/section_names.cpp. Page row CR-29.

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

Hydra today: `kHitWindowMs = 85.0` (src/core/model.h line 104) for badges, Ratings, tiers and the early-fill reach. D270 item 8 rules on Hydra's own two numbers only; D28 shelved the edge steps. Whether the hit check enforces the stored window was never measured (TM-27).

### Q-34 The formula tables in constants.py carry each other's names

This is the same finding as Q-76, raised from page 3.

The game's rule: the formula at 0x20DDDA0 uses one constant set when bit 0x1000 is clear and the other when it is set (decompile lines 767 to 781, row TM-28).

One end: `RVA_FORMULA_PRECISION` (tools/ch_probe/constants.py line 175) holds the bit-clear constants, which reproduce the live 96.29 to 171.43 ms.
Other end: `RVA_FORMULA_NORMAL` (line 166) holds the bit-set constants, and `predicted_window_normal` in the probe's analysis.py is fed from it, giving -242 and -78 ms.

Hydra today: no `src/` owner; the fix is a rename in the tool.

### Q-36 The generator at the tail of a song

The game's rule: when no chord is strictly after a measure line, the nearest-chord wrapper 0x5DDB50 searches downward from the carried start index (the last chord that took a fill), so it compares the line with that older chord and the line is skipped (code/6142/nearest_chord_loop_5DDB50.asm lines 29 to 45, back_search_2C7CE00.asm lines 20 to 34, fill_generator_20D2B30.asm lines 116 to 126; row FL-18).

One end: a line with a chord after it: both pick the same chord.
Other end: the song's last chord a few ticks before the final measure line, nothing after it, the last fill four or more lines earlier: Hydra places a fill on that chord, the game does not.

Hydra today: `Song::place_fills_ch11` (src/core/song.cpp lines 199 to 209) takes the true last chord.

### Q-37 A written fill that lands on no chord still stops generation in the game

The game's rule: the note builder compares the track's written-fill list count with 0 before any fill is landed: `cmp dword ptr [rax+0x18], r13d; je` to the generator, with r13d = 0 (code/6142/note_builder_20D2EA0.asm lines 280 to 299; rows FL-27, FL-28, FL-35; the 1.0 build does the same, code/4045/fill_generator.cs line 20).

One end: a chart whose written fills all land: neither generates fills, same result.
Other end: a chart whose only written fills reach no chord: Hydra generates fills across the chart, the game generates none.

Hydra today: `place_authored_fills` (src/parse/song.cpp line 363) drops the unlanded fills, then `Song::check_activations` (src/core/song.cpp lines 106 and 107) returns only if a chord holds one.

### Q-38 The 1.0 generator against Hydra's 1.0 generator

The game's rule (1.0): candidates are the second measure line, then every fourth line, the next line after a miss (`i = 1; i += 4; i -= 3`); a chord counts within 0.25 s of real time (`num5 > 0.25f`); the fill starts half way in real time between the previous line and the chosen line and ends a 64th note after the activation note (code/4045/fill_generator.cs lines 30 to 77; rows FL-36, FL-42).

One end: a steady-tempo chart with chords on the lines: the two may pick the same chords; the deadlines agree at a steady tempo.
Other end: a fast tempo where half a beat is longer than 0.25 s, or a chord on the first measure line, or a tempo change inside a fill: the chosen chords, the fill starts and the deadlines can differ.

Hydra today: `Song::place_fills_ch10` (src/core/song.cpp lines 135 to 167): nearest chord per line, half a beat limit, fill half a grid measure long. D115 item 6 and card c0e1259d ("Keep 1.0 exactly as before") were decided before the 1.0 code was read.

### Q-39 A 1.0 written fill's landing window has no +1

The game's rule (1.0): a written fill lands no later than end + resolution x 6 / 192 ticks, with no +1 (code/4045/fill_generator.cs lines 80 to 113; row FL-38). In 1.1 it is trunc(resolution / 32) + 1 (FL-03).

One end: a chord inside res/32: both builds and Hydra land it.
Other end: a chord exactly res/32 + 1 ticks after the end: 1.1 and Hydra land it, 1.0 drops the fill.

Hydra today: `place_authored_fills` (src/parse/song.cpp line 363) uses 1.1's +1 under both rules.

### Q-40 A 1.0 written fill on the coda tick

The game's rule (1.0): a written fill that starts on the tick of the chart's "coda" event is skipped (code/4045/fill_generator.cs lines 88 to 91; row FL-39).

One end: a chart with no coda event: nothing differs.
Other end: a chart with a written fill starting on the coda tick: 1.0 drops that fill, Hydra keeps it. Whether 1.1 does the same was not read.

Hydra today: no rule; a grep of `src/` for `coda` finds no chart rule.

### Q-50 Does a hit up to one frame past the Star Power end still score under Star Power in the game (not settled)

Reviewer correction, 2026-10-30. What the listing proves is the call order in the frame update (events step, end check, drain) and that the end check turns Star Power off once clock >= end. It does not prove that a hit is scored inside the first step: `0x20F4950` shows only the event step (slot 8), `0x20F6FF0` and the clock set, with no press handler or score add. The paragraph below states the consequence if hits are scored there; page 5 SP-29 is labelled Unknown for this reason, and live check SP-29 settles it.

The game's real rule. Each frame, the base frame update (RVA 0x20F4550, `code/6142/base_frame_update_20F4550.asm`) handles inputs and notes first (line 17, `call 0x1820f4950`), then runs the end check (line 37, `call qword ptr [rax + 0x228]`, slot 15), then the drain (line 45, slot 12). The end check (RVA 0x20F63B0, `code/6142/sp_end_check_20F63B0.asm` lines 1 to 8) is `comisd clock(+0x100), end(+0x120); jb return`, then `cmp byte [+0x42], 0; je return`, then a jump to the deactivation (slot 23). So Star Power turns off only in the first frame whose clock is at or past the stored end, and every hit handled earlier in that same frame is scored with Star Power still on. The event step also calls a handler with the end time when the end falls inside its step (`code/6142/advance_events_20F6A50.asm` lines 219 to 230), but that handler (slot 16, RVA 0x20F43A0, read by the page reviewer with chdis) only adds the time to a list at +0x180 and never touches the active flag, so it does not end Star Power earlier. Whether a hit can be judged at a sub-step clock between frames is not read (SP-30).

Example from one end: the end is at 10,000.0 ms; the game runs at 60 fps; the previous frame's clock was 9,995 ms and this frame's is 10,011.7 ms. A backend note at 10,005 ms, hit exactly on time, is handled in the 10,011.7 ms frame before the end check and pays doubled.

Example from the other end: at a high frame rate (run 1's poller saw the flag clear 0.9 to 4.8 ms after each end, so frames were a few ms apart) the same note's frame already has Star Power off when the note is handled, and it pays single. The slack is one frame, so it shrinks with the frame rate.

What Hydra does today. Hydra has no frame model. It pays a note by its chart time against one end: `paid_by_sp_walk` (offset at or below 0) in src/core/backend_value.h line 30 owns "inside Star Power", and `counted_without_squeeze` (line 41) adds DragonDelgar's 3 ms counting rule for backend rows only. So the 10,005 ms note above is a backend at +5 ms in Hydra: not paid by the SP walk, and not counted (5 is not below 3). The owner of the SP end is the search graph (`ScoreGraph::add_deact_edge`, src/search/graph.cpp). The user's D269 call 4 ruling ("there should never be two different answers for where star power ends. ever. the 3ms rule is for whether a backend is included in the optimal score or uncounted...") rules that Hydra has one end and that the 3 ms rule is only for counting; it does not name the game's frame slack, so this is asked whole: should Hydra keep pricing by chart time against the bare end (frame-rate independent, today's behaviour), or model the game's frame slack?

### Q-51 Hydra pays every accent and ghost; the game pays one only when the hit is marked as performed

The game's real rule. The drum score method (RVA 0x20DEEC0, `code/6142/note_score_add_20DEEC0.asm` lines 19 to 24) doubles a gem's value only when bit 4 of note +0x84 is set (`call` the bit test, `lea r8d,[rbx+rbx]`, `cmove`). The hit handler (RVA 0x20DE770, `code/6142/note_hit_dynamics_20DE770.asm` lines 29 to 102) sets that mark by comparing the lane's signed byte (engine +0x309 to +0x310) with the threshold at +0x2C0: an accent when the byte is at least 0 and below the threshold, a ghost when it is at or above it, nothing when it is negative.

Example from one end: the ProDrums bot on Trippin SP Spike hit all 9 accents and 34 ghosts as performed (scorestats-Trippin_SP_Spike-20261028-212754.json: accent_notes_hit 9, ghost_notes_hit 34), so its score 290,975 equals Hydra's.

Example from the other end: run 1, a key-press run on the same chart (`run1_cal0_scorestats.json`), counted 0 of 9 and 0 of 34; base_score 73,520 sits 2,285 above note_score 71,235, and the running score difference (game minus Hydra) in run1_cal0_score_compare.txt steps down at each of the 43 accent and ghost chords, ending 13,760 below Hydra's total of 378,855 (the game also made no fifth activation, so not all of that gap is dynamics). The user said in session 6a549de0: "the code is the source of truth but from experience i can tell you dynamics only work with a real drum kit. keyboard inputs never register dynamics."

What Hydra does today. `category_scores` in src/core/scoring.cpp (the accent and ghost extras, `kNoteBasePoints` each) pays every accent and ghost as performed, for every player; there is no velocity or performed state in Hydra. The question: keep pricing every dynamic as performed (the kit player's optimum, today's behaviour), or show the keyboard case (no dynamic credit) somewhere? D208 ("we don't need to change anything, matching the game is correct.") is about which breakdown line the extra points sit in, not this.

### Q-72 The tools call 0x100DAB0 the hit check, but it is the other instantiation's method

The game's real rule: `0x100DAB0` is virtual slot 25 of the generic base #8253 instantiated on the 8-byte input struct #8533. The drum engine's own copy of that slot is `0x100DAE0`, on the #8663 instantiation (`dump.cs` lines 527331 to 527338; the drum engine class #8255 derives from the #8663 instantiation at line 527421). The drum copy compares the previous input struct with the current one and calls `0x2176020`, whose body was not read (GD lines 2247 to 2275). Neither copy is shown to be a hit check. Page row IN-22.

One end: a hook at `0x100DAE0` during a drums song fires every frame. `RUNS\l3\trace-wm-count.txt` counts 20,831 calls in a 62 s song with 142 notes, so it is not once per note.
Other end: a breakpoint at `0x100DAB0`, where the tool points, never fires in a drums song, because the drum engine never runs that instantiation.

What the tool does today: `RVA_HIT_CHECK = 0x100DAB0` (tools/ch_probe/constants.py line 55), with a comment at lines 52 to 54 calling it the hit-decision comparison, and tools/ch_probe/experiments/active_probe.py line 215 sets a breakpoint there. Owner: tools/ch_probe/constants.py. Nothing in src/ reads it. The question: point the constant at the drum copy `0x100DAE0` with a comment that says what the code shows (a per-frame input compare), or drop the constant and the breakpoint. Q-77 is the same kind of slip and could be fixed in the same change.

### Q-74 Vocal notes move Hydra's song end, but not the game's

The game's real rule: the .mid vocals method (RVA `0x21579B0`, decompile lines 43955 to 44246) writes its pitches and phrase markers only to the chart-wide lyric list, through 0x2152CC0 (calls at lines 44067 to 44231). No direct call to the gem adder 0x20CEC60 appears in lines 43900 to 44300. The game's last note end is the largest gem end over every track's gem list (CR-43, code/6142/chart_length_212E130.asm lines 33 to 63), so a vocal note never moves it. Two callees were not read: the track lookup 0x20CFD40 (line 44151) and FUN_18212d1e0 (lines 44227 and 44239). The .chart vocals and the binary note-stream loaders were not checked. Page row CR-44.

One end: a chart whose vocals end before its last drum or guitar note. Both take the same last note end.
Other end: a .mid whose vocals track runs past every instrument note, with no end event and audio shorter than the vocals. Hydra's last note end is the last vocal note; the game's is the last instrument gem, so the two song lengths differ.

What Hydra does today: `MidiFile::last_note_end_tick` in src/parse/midi.cpp lines 464 to 487 reads a vocals track's low pitches as notes (`kVocalsTrackName` in src/parse/midi.h line 122). Owner: src/parse/midi.cpp, whose result becomes the one song end D255 made every part of Hydra use. D255 item 3 ruled "All instruments" for the last note end; it was decided before the vocals method was read, and it does not say whether vocals count as an instrument.

### Q-75 A .mid [end] event with white space round it: an end event in the game, plain text in Hydra

The game's real rule: the .mid events reader (RVA `0x21544E0`) trims the white space off both ends of the event text (docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm line 98), then tests it for equality with `[end]` (lines 218 to 243; the compare is an ordinal, case-sensitive SequenceEqual on character spans, not a starts-with test). A match goes to the end setter. So `[end]` with a space or tab before or after it is an end event. Page row CR-39, which is Proven; the exact and case-sensitive part of the test is the same in Hydra and is not in question.

One end: a text event `[end]`, or `[END]`, or `[end] x`. The game and Hydra agree on each: the first ends the song, the other two do not.
Other end: a text event `[end] ` (one trailing space) or ` [end]`. The game trims it and ends the song there; Hydra's .mid pass requires the whole text to start with "[" and end with "]", so it sees a plain text event and finds no end event.

What Hydra does today: the .mid pass at src/parse/song.cpp lines 1126 to 1135 skips any text that does not start with "[" and end with "]" before it calls `is_end_event` (line 46), and the text reader in src/parse/midi.cpp (line 131) trims nothing. Owner: src/parse/song.cpp. No library count (no library run), so how many charts hold a padded end event is not known. The question: should Hydra's .mid end test trim the text first, as the game does?

### Q-76 The two window-formula tables in constants.py carry each other's names

This is the same finding as Q-34, raised from page 7.

The game's real rule (GD lines 768 to 781, the window formula at RVA `0x20DDDA0`): `if ((*(uint *)(param_1 + 0x198) & 0x1000) == 0)` the branch returns `((dVar1 - dVar2 * _DAT_183140620) * _DAT_183140708 - DAT_183063fc0) / DAT_183064c28` with `dVar1 = param_2 * _DAT_1831406b0`. That is the normal branch (bit clear), and its four addresses are the ones `RVA_FORMULA_PRECISION` lists (constants.py lines 175 to 180). The bit-set (precision) branch uses the four `RVA_FORMULA_NORMAL` lists (lines 166 to 171). The comments above each table describe the right shape for the name but sit over the other branch's addresses.

One end: with the constants under `RVA_FORMULA_PRECISION` fed into the normal shape, an input of 170 ms gives 171.43126 ms and 75 ms gives 96.29316 ms, the two ends of `tools/ch_probe/experiments/results/poll_windows.csv`. Other end: the table now named normal gives -242 and -78 ms, which no run ever stored.

What the tool does today: the normal-window prediction in `predicted_window_normal` in tools/ch_probe/experiments/analysis.py (lines 227 to 251, fed by `normal_formula_constants` at lines 312 to 316) reads `RVA_FORMULA_NORMAL`, so it predicts the wrong curve. Owner: tools/ch_probe/constants.py (a swap of the two table names). Nothing in src/ reads it. Page row IN-16.

### Q-77 RVA_BASE_ENGINE_CTOR_DRUMS holds the other instantiation's constructor

The game's real rule (`dump.cs` lines 527295 to 527306, `script.json`): `0x100E960` is the `.ctor` of #8253 on #8533 and on #8332; the drum engine's base is #8253 on #8663 (`dump.cs` line 527421), whose `.ctor` is `0x100EAE0`. `RUNS\l3\trace-wm-count.txt` lists ".ctor (base 0x100eae0)" in the drum engine's hooked set.

One end: a hook placed at `0x100E960` during a drums song never fires for the drum engine. Other end: a hook at `0x100EAE0` fires once when the drum engine is built.

What the tool does today: `RVA_BASE_ENGINE_CTOR_DRUMS = 0x100E960` (tools/ch_probe/constants.py line 46). Nothing uses it except a test fixture copy (tools/ch_mcp/tests/test_dump_facts.py line 410). Owner: constants.py. Page row IN-23. Same family as Q-72.

### Q-78 OFF_HIT_TIME (+0x2E0) is labelled the hit time, but the only run that logged it read 0.0 throughout

The game's real layout (`dump.cs` line 527437): #8255 has a private double at +0x2E0. The code that writes it has now been read: it is the start of a double lane's round, written only while a note flagged 0xC000 is handled (page 07 IN-12), so it is not a hit time at all. Live: `overlay-spike/run1_cal0_sp_watch.csv` has its column at 0.0 in all 40,922 rows of a key-press run that hit every note.

One end: if it were the hit time, it would equal the song clock at each hit within 10 ms (`HIT_TIME_MAX_GAP_MS`, constants.py line 297). Other end: it stays 0.0 through hits, as run1_cal0 shows.

What the tool does today: `OFF_HIT_TIME = 0x2E0` (constants.py line 106) with the comment "NOT yet confirmed live" (lines 103 to 106). Owner: constants.py; the comment should say what the run showed, or the constant should go, now that the writer is read: it should say "double-lane round start". Page rows IN-12 and IN-15.

### Q-79 OFF_SP_OVERFLOW (+0x130) is described as set only on an overfill, but every gain sets it

The game's real rule: the gain method `0x20F4FA0` sets the byte at +0x130 on every gain (`code/6142/sp_gain_20F4FA0.asm` lines 44 to 57, then it sends the new meter ratio), and the per-update method `0x20F6800` also sets it at its clamp (GD lines 53552 to 53557: amount above max sets amount = max and sets the flag). In the same method's Star-Power-on branch the flag is tested at line 53558, the end time is recomputed, and line 53572 clears it; no clear was found for the Star-Power-off case.

One end: a quarter-meter phrase with no clamp: the flag turns on (run1_cal0 first flip at 5.715 s with 1440 ticks in the meter). Other end: Star Power active: the flag reads 0 in all 11,436 active rows of run1_cal0.

What the tool does today: `OFF_SP_OVERFLOW = 0x130` (constants.py line 140) with the comment at lines 137 to 140 saying it is set only when a phrase overfills the meter. Owner: constants.py (name and comment). Page row IN-27.

## Reserved ids settled before write-up

Three of the six splits known before the reference was built turned out settled once the game's code was read. They have no heading here, and no `Ruling:` line names them.

Q-70 asked whether any written fill stops generated fills. The game's side is FL-27 on page 4: the note builder runs the generator only when the track's written-fill list is empty. The one case where Hydra still differs is narrower, a written fill that lands on no chord, and it is Q-37.

Q-71 asked why a 2x kick written before a normal kick on one tick lowered Hydra's Dynamics totals by one with 2x Bass off (the derivation audit of 2026-10-03). NL-29 on page 2 shows the game merges the two kicks whatever their order. Since D105, `Chord::add_kick` in src/core/model.cpp merges them in either order too, so the two agree. NL-30 waits on a live run of both file orders.

Q-73 asked about a song.ini delay equal to the chart Offset. TM-04 on page 3 shows both routes store the same milliseconds in the same field, so that edge changes nothing.
