# Capture list CL2 (records A28 to A50, B1 to B7)

Scout CL2, task T5-CL2. Read-only; nothing run. I read every "claim not checked" hit in `records/A28` to `records/B7` with its record heading (an extractor over the files, not a line-by-line read of each whole file), and the matching lines of `verify/`. Record ids are the record's own number or name; `A39` and `B5`/`B6` records have no number, so I cite the line in the records file. Charts are named only as the records name them; a path is given only where I found it in `testdata/input`.

Chart paths found in `testdata/input` (all under `common/`): Burnout = `Summer Blast _25 Setlist/Tier 4/Green Day - Burnout`; Don Broco - Actors = `Summer Blast _25 Setlist/Tier 1/Don Broco - Actors`; The Spiraling Void = `Summer Blast _25 Setlist/Tier 7/The Faceless - The Spiraling Void`; Acid Romance = `IB24/T3/Alpha Wolf - Acid Romance`; White Washed = `IB24/T5/August Burns Red - White Washed`; Tapped Out = `IB24/T5/Mike Orlando - Tapped Out`; Set the World on Fire = `IB24/T5/Symphony X - Set the World on Fire`; Levitating = `IB24/T6/Levitating (feat. DaBaby) (Dua Lipa) (L)`; Pathfinder = `IB24/T6/Pathfinder - When The Sunrise Breaks The Darkness`; Tapestry = `IB24/T7/Ne Obliviscaris - Tapestry of the Starless Abstract (Shortened)`; flammarker = `testdata/input/test_flammarker/flammarker_ingame.mid`. Not found there: "What's My Age Again? (Sync Chart)", Dumpweed, Freak, Rescued, Ghost, As I Am, Thrice - Deadbolt, SoundHaven - Triad.

## 1. Engine dumps

Settings are "not stated" where the record does not give them. "Burnout dump" means a `hydra_replay dump` and `schedule` of the Burnout chart above.

| Chart | Settings asked | Read from it | Records |
|---|---|---|---|
| A chart whose last SP end is past its last note (none of the seven records; no chart named) | cap 4 | activation `deact_ms` vs `song_length_ms`; the Backend timings table | A28 1, 2, 3 |
| A chart with a tempo change exactly on an SP end tick (none of the seven; none named) | not stated | the SqOut/SqIn tick and ms answers at that end | A28 6 |
| believer path 4 act 1 and beg path 0 act 0 (record charts) | not stated | stored `bank_rise_ticks` (not in any dump/schedule); bank arrival at deact 57600 / 27648 | A28 22, A29 2, 4, 15 |
| Pitch Dark (record chart) | cap 4 | stored `bank_rise_ticks`; bank at clamped steps; a nonzero gauge reading | A29 2, 3, 5, 6, 14, 24, 25, 27 |
| Burnout or Blight (A29 15 names these as the needed shape) | not stated | a squeezed chord strictly before the stored end (`sqout_tick` < `deact_tick`) | A29 15 |
| A chart with a time-signature change inside an SP window; a chart with a bar arriving on an activation chord; a chart with no SP and no path | not stated | step ticks, window, curve | A29 7, 13, 18, 21 |
| Believer path 4 act 1 and 2, Pitch Dark act 0 and 1, `late_sqin_twice` | not stated | stored `end_ratio` early/late, ratio per SqIn, tier text, `counted_without_squeeze`, `cap_clamped` | A30 1, 13, 14, 15, 21, 23, 25, 26 |
| A chart with a clamped activation with an uncounted row and no squeeze; a backend row between 0 and 3 ms of the end | not stated | row offsets, "SP overfilled" | A30 24, 25 |
| "What's My Age Again? (Sync Chart)", Dumpweed, "As I Am m81" (not in corpus) | not stated | SqOut offsets and scales | A30 6, 7, 8; B7 13 |
| A chart whose scan stored an unknown transfer scale | not stated | a saved library row with unset scale | A30 20, 29 |
| A real chart with a mid-SP fold of a tied variant under a squeezing lead (`folded_sqin` has none); a real squeezing chart for `clamped_sqin_a/b`; `folded_sqin` phrase chords | not stated | variant tree, `sp_end_steps` of tied paths, banked phrase chords, SqOut offsets | A35 1, 2, 5, 6 |
| Thrice - Deadbolt (cap 4) and SoundHaven - Triad (cap 2) (named by D34; not in corpus) | cap 4; cap 2 | the lost "+" with twin SP ends one tick apart | A35 7, 8 |
| Don Broco - Actors (`Summer Blast _25 Setlist/Tier 1/Don Broco - Actors`) | score depth 40 | paths 29 and 30 tie at 233,100; a variant that finished the song keeps its bank | A35 9, 13 |
| A real chart with a folded variant carrying a late early fill (Beg's E0 is 0.0 ms only) | not stated | E mark offset 49.5 ms | A35 12 |
| Corpus chart with a real Pro Drums solo/fill/ghost/lane state; folded_sqin activation 0 skipping fills | not stated | fill and solo spans, sp_active sweep | A36 6, 7, 11, 13, 17, 18, 21 |
| First corpus chart with paths | 4 bars; also caps 4, 32, 64 | whole record read back; record under another build and other rules | A33 T6, T7, T24, T25, T28, T32; A34 R1, R2, R3, R5, R6, R21 |
| Corpus charts that really fail (no notes, bad tempo "line 12", the four failure kinds), plus a corpus chart analysed under a second real `hydra_rules.ini` | not stated | a real failure record; a record under two rules | A33 T13, T14, T15, T16, T19, T21, T12, T18, T31, T32, T33, T34 |
| A real chart from `records-engine/` whose squeeze-in step is inside Star Power (hardest figure sign) | not stated | hardest figure sign | A34 R18 |
| Burnout dump (path list "3- 1 2" 378,315; "0 4 1" 378,175; "0 0 0 0" 375,955; backend rows -489.1, -326.1, -163.0; multiplier squeeze `+15` "Hit [Red snare] first."; chart end/`[end]` event and tempo map; best path "9 1" 364,675 and one-window "9" 349,875; fills at ticks 21120 (m12), 36480, 74880; first fill window end, best finish 378,175, cost 378,315; m32 squeeze-out 163.0 ms ticks 70800, 71040; schedule with three activation ticks and chords, second activation) | normal; leeway 500 and defaults for A49 9; the narrow-panel cases use the default | the values in the parentheses | A44 1, 3, 4, 6, 8, 9, 10, 11; A45 11, 12, 14; A48 1, 4, 5, 6, 12, 13, 14, 15; A49 1, 2, 9 |
| White Washed (`IB24/T5/August Burns Red - White Washed`) | not stated | window 4's squeeze-out 130.4 ms, chord 253320, stored end 253440 | A48 7 |
| Tapped Out (`IB24/T5/Mike Orlando - Tapped Out`) | not stated | activation 4 record | A44 11, 12 |
| Tapestry (`IB24/T7/Ne Obliviscaris - Tapestry of the Starless Abstract (Shortened)`) | not stated | backend rows with a squeezed-out row, `counted_without_squeeze` false | A44 5; A41 16 |
| The Spiraling Void (`Summer Blast _25 Setlist/Tier 7/The Faceless - The Spiraling Void`) | not stated | activation count (comment says 9) | A44 7 |
| Set the World on Fire (`IB24/T5/Symphony X - Set the World on Fire`) | not stated | length 5:58.957; dump for the long timeline | A48 2; A49 3, 4 |
| Acid Romance (`IB24/T3/Alpha Wolf - Acid Romance`) | 2x Bass on and off | `count_dynamics` rows; kicks 418 and 37 of 455 | A39 (line 74); B1 29, 30, 31 |
| Pathfinder (`IB24/T6/Pathfinder - When The Sunrise Breaks The Darkness`) | difficulty Hard | `count_dynamics` rows | A39 (line 114) |
| Levitating (`IB24/T6/Levitating (feat. DaBaby) (Dua Lipa) (L)`) | not stated | solo start and end ticks | A45 5 |
| Chart 0 of `testdata/input` (the first chart) | not stated | engine total; schedule (activation start, SP end); store row after a Preview-only run | A45 2, 4, 6, 16 |
| Beg's chart read for its difficulties (parser's difficulty list) | not stated | difficulty list | A45 15 |
| A corpus chart with a tempo change, for `hydra_replay score` | not stated | `act_ms`, `deact_ms` per window | B1 20 |
| Every corpus chart | not stated | each note goes to the Dynamics row its lane and flag name | B1 27 |
| Real dump with empty `paths` list; a real activation with both a squeeze-out and a squeeze-in on one anchor; a tempo change just before a squeezed anchor; a squeezed-out chord whose gems pay different multipliers; a squeezed-out chord that is also a declined fill; a whole-chord squeeze-out with no inside group; a read end past a tempo change; a tempo change on an end tick (none in the seven) | not stated | the schedule refusals and splits | B3 8, 16, 17, 19; B4 (whole_chord no inside group, skipped fill, past-tempo-change, tempo on an end, read_at_own_ms) |
| Pitch Dark schedule | not stated | `last_send_ms` per press vs chord ms + side - 3; `bank_rise_ticks` for `sp_ready_tick` | B3 15; B4 (sp_ready_tick) |
| Freak 387/1144, Rescued 561/931, Ghost 756 (not in corpus) | not stated | `anchor_side`, `squeeze_side`, `gap` | B3 10, 11 |
| flammarker (`test_flammarker/flammarker_ingame.mid`) | not stated | "(No activations.)" path at 155295 read back; long text with the multiplier squeeze reads no windows | B2 (pathstring read, "(No activations.)" and long-text cases); A49 5 |
| First corpus chart whose best path has two windows | not stated | the refusal sentence text | B2 (pathstring refused token) |
| Corpus .mid with sysex, with two drum tracks, with Windows-1252 text, format-0, every corpus .mid read by lean and full readers | not stated | kept messages, drum track pick, text decode | B2 (midi: running status, two drum tracks, sysex, text decode, lean reader, is_midi_header) |
| Corpus .chart with a solo, with an end event, with an Offset; a real double lane | not stated | solo, end, offset, lane group | B6 (lines 344, 394, 251, 911, 480) |
| A real chart with no tick-0 tempo, a 4096 bottom number, a mid-measure meter change, a 6/8 section, a signature between beats, a signature before tick 0, a tempo change; Pitch Dark meter | not stated | tempo/meter in force, measure count, `plusmeasure` | B7 3 to 15, 17, 18, 20 |

## 2. Screen captures

| Screen | Chart and path | Moment or state | Read | Records |
|---|---|---|---|---|
| Backend timings table (Paths tab) | chart whose SP outlasts its last note | final rows | rows measured from the SP end | A28 1, 2, 3 |
| Preview gauge and drain box | Pitch Dark | at the clamped step tick 113040 (cap); at the squeeze-in chord 128520; at 100975.37 ms; two moments on both sides of a tempo change; inside a window with a meter change; a step tick; a nonzero reading | gauge `x/4`, "empties in" | A29 2, 3, 5, 6, 8, 14, 17, 21, 24, 25, 27 |
| Preview gauge | Believer path 4 act 1 / Beg path 0 act 0 | at the squeezed chord (equal to the stored end); strictly early chord needs Burnout or Blight | gauge, x8, "empties in" | A29 15, 20, 26 |
| Preview gauge | unanalysed chart; chart with no SP | idle | gauge, idle drain box | A29 1, 13, 18 |
| Paths tab | Believer path 4 act 2 | scaled early read | line colour (orange) | A30 19 |
| Paths-tab tooltip | Believer path 4 act 1 | "normal scale" | tooltip text | A30 27 |
| Path report | any chart | Beyond tier text and report footer | tier text | A30 28 |
| Preview/Library error line | a song whose audio is not audio | status | error text | A31 SR10 |
| Library/report name column; "Hydra scans your library again" message | corpus scan with a rename or two copies | after upgrade | names, message | A31 ST3, ST5, ST7 |
| Window status line, startup box, tool stderr | real refused chart, real bad rules/settings file, locked database, old library set aside, missing chart folder, missing song file, failed save | each | the sentence | A32 13, 17, 21; A33 T15; A37 1 to 22; A41 8, 9; A42 10; B1 (db fail); B2 pathstring sentences; B6 (lines 733, 984, 1006, 1038) |
| Window, buttons, tables | any chart | each of the three button states, disabled button and input, selected row (Ready gold, not-analysed grey, stale orange), hover, report chips of every tier, version bar's three states, highway at a Star Power moment | pixels and contrast | A34 T1 to T12 |
| Preview highway | corpus chart | toggles on/off, ghost, taken and offered fills, touching solos, touching fills, a chord one tick after a phrase end, drum lanes, scrolled highway, path switch | gem and span lighting | A36 1 to 4, 6 to 14, 18 to 21, 23 |
| Library, Path Builder, report, batch and settings windows | corpus | see the records: batch strip steps, confirm window, toolbar at cap 6/8 and Hard, finished strip, status line, Song folders box | text and layout | A37 batch-reports 1 to 15 |
| `hydra_uitest` script verbs | real library | `window`, `check`/`uncheck`, `key`, `wait`, `expect-text`, `dump`, `rect`, `state`, `reset`, `quit`, unknown verb, failed click | transcript and exit code | A38 (all 12 verbs, none in any `.cmd.txt`); A40; A43 L1 to L11 |
| Preview, song panel, settings | Beg, chart 0, Burnout; Acid Romance | loading bar mid-load; score box at song end; first activation; two paths before/after a pick; both panel widths; solo box at first chord, last note, bonus, tail; failed Preview of a real broken chart; no-notes sentence | text and pixels | A39; A45 1 to 7, 11 to 16 |
| Dynamics tab | Acid Romance, Pathfinder | 2x Bass on and off; Hard | rows, Kicks line, colours, dots | A39; B1 29 to 40 |
| Library | corpus | scrolled middle, before and after a row click; Next scroll; sort; widths before and after a batch; narrow library with the long Tapestry title; search box and popup; Stale chip tooltip; not-analysed tooltip; startup read bar | positions, widths, tooltips | A41 1 to 18; A42 1, 2, 5, 6, 7, 8, 9, 10, 11, 12; A37 10, 11 |
| Path report window | corpus rows, first chart | sort, filters, states, keys, close, both windows, chip label at 900 px with Beyond tier, `tn` row chip, column widths, left cells, header at two widths, window fits the screen; asks/counts/opens; picker and comparison; reopen; out-of-date strip | text, layout | A46 all; A47 1, 2, 4, 5, 6, 7, 8, 9, 10 |
| Paths tab | Burnout | narrowest song panel (share 0.99) with every fold open and "Copied!"; first backend table; row 1 with three injected badges; limits 30 and 500 | layout | A44 3, 6, 8, 9 |
| Paths tab | The Spiraling Void; Tapped Out act 4 | both widths; narrowest panel | layout | A44 7, 12 |
| Path Builder | Burnout; Set the World on Fire (0:00-3:00, 2:59-5:59); White Washed; a chart with no Star Power; flammarker | head row at 800/400 px, timeline scroll, clock under drag, hover paused/playing, fill card, cant-activate reason, locked-fill text, tight-window note colour, open with missing path, bad settings, bad rules, paste refused | text, positions | A48 1 to 15; A49 1 to 9; B2 (paste sentences) |
| Settings panel | any | during a batch (greyed controls); panel with library hidden; header's two buttons | layout | A39 (lines 214 to 514) |
| Preview | Beg, chart with no song | a lane; no song empty scene time box; beat lines at mid-measure change and 6/8; time box at fractional ms; before tick 0 | text | B2 (mid lane velocity); B7 6 to 12, 14, 18, 21 |

## 3. Real inputs for files, store and settings

| What | Compared against what | Records |
|---|---|---|
| A real stem per format: WAV, FLAC (incl. ID3 tag, pipe-written zero-frame header), MP3 (long), Ogg Vorbis, Opus (R128 gain, chained, damaged, mid-page seek, last-page granule); a real PNG named `song.ogg`; a real missing stem | the independent whole-file decode (`old_full_decode`, miniaudio, `stb_vorbis_decode_memory`) | A31 SR1 to SR15 |
| A real SQLite file with a real aborting trigger; corpus-scan rows with a rename, two copies, a dropped chart, a chart found twice, edited chart (changed bytes), Stale records | the table's old rows; names; SQL counts | A31 ST2 to ST14, ST17, ST19, ST24 |
| Real library files: 1.6 (none exists), 2.2.0 to 2.7.0 (none checked in), 2.8.0 with a charts row, v2.12.1, v2.1.0-schema4, a truncated copy of a real library, a real library with Ready rows and figures missing, a library with two profiles | `classify_library_file`, open and migration results | A32 1 to 7, 9 to 13, 17, 20, 22, 28 to 31; A42 5, 7 |
| A copy of a real `hydra.db` from an older release (old stamp, old-build failure record, `rules_fp` bytes, unknown settings -1, old stamps from the release list) | what `lookup` returns (Stale/Ready), fingerprint the build computed | A33 T11, T17, T29, T30; A34 R7, R10, R16 |
| A real broken `hydra_rules.ini` and `hydra_settings.ini` (e.g. `sp_cap=banana`, `max_tied_paths = 0`, a `sp_cap=0`) | the loader's sentence in Hydra, the tool stderr, the Path Builder | A29 11; A33 T18, T31; A37 6, 13, 14; A39 (line 434); A41 8, 9; A49 7, 8; B1 9, 10, 11 |
| A real `hydra_ui.ini` Hydra wrote after a sort on Best path; one from an older Hydra with five columns; real `Size`/`LibraryShare` | the loaded column order and layout | A39 (line 234); A42 3, 4 |
| Corpus `testdata/input`: every file read with `read_file_bytes`; corpus root in several spellings; `parent_folder` of every chart path; a real library folder with a non-ASCII or fullwidth-slash name; a real empty file; a real file past 2 GB (cannot be in `testdata/input`) | `std::filesystem` (`file_size`, `equivalent`, `parent_path`, `ifstream`) | A37 winstr 1 to 19 |
| Real chart files with a refused tempo (zero BPM, no tick-0 tempo, unreadable Resolution), a real `.mid` with zero tempo or non-MIDI named `.mid`, a SMPTE file, the fuzz crash file | the refusal sentence; both readers | A37 1 to 5; B2 midi; B7 3 to 5 |
| A real corrupt `hydra.db` (truncated copy of a real library); a real new hydra.db written by this build over `testdata/input` | the open error sentence; `user_version` 0 | A32 2, 22; A43 L14; B1 2 |
| A saved real GitHub release reply (`latest.json`) and a real dmleaderboards reply (`canned_dm_*` are canned) | version-bar and comparison text | A40 H21; A41 10; A42 11; A47 5 |
| A real second `.chart`/`.mid` with an Offset; a real chart with only-tags artist | parser; `hydra_batch` (unknown) | B1 14; B6 (line 911) |
| An independent mido reading of a real malformed file or Rock Band rip .mid | lean reader messages | B2 midi (running status, five-byte delta, 1,000,000-byte cap) |
| A real `.chart` with no tick-0 tempo, `.chart` with an end event (no rule page for it) | loader and refusal text | B6 (lines 251, 966); B7 3 |
| Single-owner scan: plant one extra line in a real file; a planted real topic file; real rows and baseline | the scan's message shown on real input | A49 10, 13, 15, 16, 18, 20, 21, 23, 24 |
| A real batch over the corpus; store's own count of saved records | the strip and the Analyzed chip | A37 batch-reports 15; A41 18 |
| A hand-built chart's real counterpart: a field-verified chart per D115 | the grid | B7 7 to 9 |

## 4. Real hook inputs

| Hook | Recorded call or state needed | Records |
|---|---|---|
| `ShellExecute` (`shell_execute_ok`) | a real call on a missing file and on a real openable file, with the returned values | A37 winstr 11 |
| Process exit code of a real tool (`run_tool`, `hydra_batch` with a failing store) | the real exit code and stderr | A37 18, 21; A40 E10 |
| Real process command line / argv for a library folder with a fullwidth slash or non-ASCII name | the real `split_command_line_utf8` result from a real launch | A37 winstr 17, 18 |
| Real audio device handle close and real thread | a real run on the device | A39 (line 154) |
| `hydra_uitest` crash and device-removed path (`on_crash`, exit code 3), bad flag, `--all`, `--jobs`, `--db`, `--ini`, `--live`, a deliberately hung test | a captured run's output and exit code | A40 E6, E7, E8, E9, E10; A43 M1 to M5 |
| Live session (attached swapchain), a live session closed by its window | a live transcript | A40 E4, E13, H4, H15; A43 L1 to L11 |
| Real HTTP failure from each server (update server, dmleaderboards: refusal, timeout) | the details line | A37 16, 17 |
| Real Clone Hero probe chart (Edge Walk Pair) refusal | a live run or schedule | B3 18 |

## Counts

"Not-checked records" is the number of record sections that say "claim not checked" (or a close form such as "not captured"); `B5` and `B6` have no section headings, so their counts are approximate. "Lines" is the number of rows of mine that name the record (rows merge many records).

| Records file | Not-checked records | Rows naming them |
|---|---|---|
| A28 | 9 | 7 |
| A29 | 26 | 8 |
| A30 | 18 | 8 |
| A31 | 30 | 4 |
| A32 | 20 | 4 |
| A33 | 24 | 5 |
| A34 | 25 | 6 |
| A35 | 12 | 5 |
| A36 | 20 | 3 |
| A37 | 55 | 8 |
| A38 | 13 | 1 |
| A39 | about 22 | 7 |
| A40 | 49 | 3 |
| A41 | 18 | 3 |
| A42 | 12 | 5 |
| A43 | 18 | 4 |
| A44 | 12 | 4 |
| A45 | 13 | 4 |
| A46 | 12 | 2 |
| A47 | 10 | 3 |
| A48 | 15 | 3 |
| A49 | 18 | 4 |
| A50 | 0 | 0 |
| B1 | 25 | 6 |
| B2 | 49 | 5 |
| B3 | 12 | 3 |
| B4 | 12 | 2 |
| B5 | about 7 | 1 |
| B6 | about 40 | 3 |
| B7 | 19 | 2 |

Limits. The verify files were read through the same extractor only for their counts of "not checked" lines (they restate the records' hits); I did not read each verify file whole, so a need that only a verify file names may be missing. A41 names capture ids L1 to L6 and A32 names file letters (a) to (h); their definitions sit in those records' own headers and were not copied here. The extractor cut each hit to 240 characters, so rows that depend on text after the cut (some A39, A40, A49 and B6 rows) are named by line and may need the full record read.
