# Capture list CL1: records A1 to A27

Task T5-CL1. Scout read the "3 Real chart" answer of every record in `records/A1.md` to `A27.md` that says "claim not checked" (571 hits for the records' own question 3, plus A2, A4, A5 and A8 whose records write the line in a bullet form), and the lines of the matching `verify/` files that name a capture, a dump or a real input (including `A3-A4-reread`, `A4-q3`, `A7-reread`, `A26-resend`, `A27-resend`). Records are named `<file> <record id>`. A22 has no numbered records; its records are named by their line in `records/A22.md`.

Reading limits, stated plainly. Many records write only "claim not checked." with no further words (A6, A7, A8 tests 18 to 38, A25 R11 and similar). For those, the need is what the file's own shared note or title gives, and the line below says so. The charts the records name that are not among the seven dumped ones were looked up in `testdata/input`; the path is written where one exists. One fact the main session should know first: `records-engine/` now holds `burnout.efill60.dump.json` and `burnout.efill85.dump.json`. Records A2 13, A3 15, A17 18/19/22, A19 PL10/PL11 say "Burnout is not among the seven"; those engine dumps now exist for Burnout, but its schedule and screen do not. Not taken apart here.

## 1. Engine dumps

| Chart | Settings asked | What is read | Records |
|---|---|---|---|
| Burnout: `testdata/input/common/Summer Blast _25 Setlist/Tier 4/Green Day - Burnout` | default (record gives none); Paths tab at default | best path, its three activations, one multiplier squeeze with the fold's sum, stars (7?) and hardest timing (163.0 ms?), squeeze-out room (5.4 ms safe) | A2 13; A3 15; A17 18, 19, 22; A19 PL10, PL11, PL14 |
| Tapped Out: `testdata/input/common/IB24/T5/Mike Orlando - Tapped Out` | default; best path | activation 4's squeeze-out (out of reach?), schedule with `anchor_room_ms` / `squeeze_room_ms` | A3 14, 21 |
| Focus: `testdata/input/common/Summer Blast _25 Setlist/Tier 7/SoundHaven - Focus [Ariana Grande]`; Epidermis: `.../Tier 8/Venetian Snares - Epidermis`; with Burnout and Tapped Out | default | the Paths tab row set, to check one row changes nothing else | A3 21 |
| Weltseele (user's own chart, not in `testdata/input`; none found) or any chart with a fill left over | default | an early fill with time to spare: a negative E offset | A3 1 |
| Any chart with an orange (difficult) badge, e.g. `late_sqin_twice` activation 1 | default | squeeze-in over 2.0 ms past the difficult floor; and the 2.0 ms edge | A3 2; A18 10 |
| `late_sqin_twice` (`testdata/input/test_fast_tempo/late_sqin_twice.chart`) | default | scale ratio 0.06, a -495.0 row "Normal (eff. 934.0 ms)", four squeeze-in rows with safe/short tags | A3 9, 14 (verify A3, A3-A4-reread) |
| A chart where a transfer scale is unknown | default | "Transfer scale unknown" text on a real record | A3 11 |
| Any chart with a snare, tom or ghost chord on an activation | Pro Drums on and off | chord names | A3 12; A19 PL12 |
| Beg (`testdata/input/common/IB24/T1/Evans Blue - Beg [highfine]`), activation 3 | default and an early-fill window change (60) | E0 edge, edited figures (-0.1, 20 ms to spare, optional), the 85 ms edges, `e_offset` | A3 19; A18 9, 18 (the "1- 0 0 E0" row) |
| The first corpus chart that yields paths (`corpus::analyzed_by_ten_scores`, depth 10 scores, 10 ms limit) | depth 10 scores, 10 ms | the rows on the Paths tab beside the test's rows; scale line | A2 4, 5; A19 PL6, PL7, PL8 |
| The first corpus chart with paths, with a multiplier squeeze (check if any) | default | whether `build_multsqueezes` loop runs at all | A19 PL8 |
| Chart whose all-0 list has a duplicate and a variant | default; all-0 list | all-0 dump with a variant | A19 PL5; A13 9 |
| Slander (D85's chart; not found in `testdata/input`) | all-0 | "Best all-0 path" / `allzero_paths` | A13 9 |
| One corpus chart with a squeeze-in at exactly 0 | default | squeeze-in offset 0 and what the library column shows | A13 10; A17 21 |
| Any corpus chart with CH 1.0 fills; short-fill chart and long-fill chart | `--legacy-fills` and normal | fill deadline under both rules, record has paths, skipped fill ticks, which fill is stored | A13 6, 17, 18, 22, 23; A14 24; A20 14; A22 line 303 area (Ch10/Ch11 corpus chart); A24 4 |
| A chart with a fill at an odd resolution (not divisible by 16), fast tempo (250 ms preroll) and slow (10000 ms) | `--legacy-fills` | the fill deadline | A13 19, 20, 21 |
| Corpus charts with fewer than four phrases | default | record hash at the song's phrase count | A13 16 |
| Chart with a fill whose reach is exactly deadline + 85 ms | default (and 60) | E mark at the edge (also SPLIT FL-23) | A14 25 |
| Chart where an earlier-ready path is knocked out by a later one | default | which activation survives | A14 23 |
| Hail The Sun: `testdata/input/common/IB24/T4/Hail The Sun - Missed Injections` | default | path "3", fills 244800 and 281759 | A14 29 |
| Allister - Overrated: `testdata/input/common/IB24/T1/Allister - Overrated` | default | empty path, every fill in trailing fills; best score 228710, path "1 2"; fill card costs | A14 32; A19 PB3, PB8, PB9, PB10; A24 3 |
| A chart whose last activation window outlasts the last note | default | `deact_tick` past the last note | A14 30, 31 |
| Pitch Dark (`testdata/input/test_pitch_dark_ratio/notes.mid`) | default | solo section last chord and its score (dump has `"solo": 29800`) | A23 3 (verify A23); A18 (solo SC-20 row 3) |
| A chart with a solo section | default | solo bonus, last chord | A18 3; A20 7, 8; A15 28; A23 3 |
| A chart with an energy-gem phrase and a chord on a phrase's last tick plus one a tick later | default | phrase membership per gem | A15 12, 24 |
| A chart with a drum lane | default | lanes in the record | A15 17, 18; A20 9 |
| A chart with two touching taken fills | default | the highway | A15 14 |
| A chart with an activation whose fill is hidden (path that activates early) | default | hidden fill | A15 16 |
| Pitch Dark paths 393,885 etc. with tied top-score variants; a real tied-top chart (`tests/test_tied_variants.cpp` names them) | default | tied variants, optimal marks | A18 11, 12, 22; A25 R1 |
| A chart with a squeeze-in or squeeze-out at a non-zero offset (-50, +150, 5 ms), a +3.0 ms backend row, and a row at each ladder edge | default; leeway 3 ms | the uncounted tags, the 2.0 edge | A18 30; A26 RU12, RU13, RU14 |
| Chart with chord tie or tied gems | default | per-gem value, basescore | A18 1, 2 |
| A chart with an empty chord, a 12.6 ms squeeze, a nonzero squeeze-out and a multiplier squeeze straddling chords (flammarker_ingame) | default | the per-chord multiplier, the straddling chord | A18 5, 13, 14, 41 |
| Believer (`testdata/input/common/IB24/T1/Smash Mouth - I_m A Believer (The Monkees cover) [highfine]`) path 4, activation 1 | default; and `whole_chord` rule | squeezed-out chord 465 at tick 57600, points paid, `deact_tick`, warning text | A18 48; A23 8, 10, 13, 15; A24 8, 13, 14, 15; A26 RU8 |
| Believer path 0 activation 1 SqIn, path 1 "1 0+ 2" | default | squeeze figures 0.0 | A27 SF3 (verify says now checked), SF1 |
| Chart with phrase chords either side of one SP end (two equally spaced; a banked and a newer phrase) | default | squeeze-out chord choice | A24 9, 10, 12, 16, 17 |
| A chart where one phrase is squeezed twice | default | squeeze history | A24 16 |
| A schedule or dump with chord-level `points.sp` beside the gems | default | chord SP total | A24 6 |
| A real chart with one tempo and meter; one with a tempo marker on the activation tick (Rescued; not found); one with a tempo change on the tick; one with a meter change between the ticks | default | end ratios 1/null; tempo-marker numbers | A23 17, 18, 19, 20; A24 1 |
| An activation with no deact tick (`deact_tick` -1) | default | null end fields | A24 2 |
| Folded_sqin and the other seven's `deact`/stored ratios | default | `end_ratio` pairs | A24 1 |
| Chart with an accent cymbal at several multipliers and a ghost/accent kick | default | dynamics bonus, chord_score_table rows | A24 19, 23; A18 25 |
| `hydra_replay score --path` text on a real dump (typed squeeze-out) | default | warning wording and cost | A23 11; A24 14 |
| Several charts for tied-path caps: Sugar (`testdata/input/common/IB24/T6/black midi - Sugar／Tzu (Smoochums, Vasasasasa)`) legacy fills, ms 0, tie limit 1 ("0 E3+ E5 E1"); HopH2O (`.../T6/HopH2O - Nasty (Tinashe)` or `.../Tier 5/HopH2O - I Am... All Of Me (Crush 40)`; the record says only "HopH2O") | legacy fills, 0 ms, tie limit 1 | tied-path count, path over Path limit kept/dropped | A26 RU15, RU16, RU17, RU18, RU19, RU20 |
| A chart with no authored fills | default | generated-fill values | A26 RU21 |
| Charts whose targeted search names activation ticks: Allies (`testdata/input/common/Summer Blast _25 Setlist/Tier 3/Mutemath - Allies`); Believer path 4 with the squeeze-out flipped; any corpus chart with a root that misses a tick and a variant that does not | `hydra_replay target`; default | windows, kept paths, `failed_...`, one-window pin paths | A27 ST1, ST2, ST3, ST4, ST6, ST7, ST8, ST10, ST11, ST12; A8 14, 15, 16, 17 (best_finish on Pitch Dark and Allies) |
| Everlong (`testdata/input/common/IB24/T4/Foo Fighters - Everlong`) | default | rank 1 and 2 path text, SqIn figure inside Star Power | A27 SF5, SF7, SF9 |
| Pitch Dark (and Beg) with ratio fields; Trivium: `testdata/input/common/IB24/T3/Trivium - Feast of Fire` | default | fill states (Path Builder) | A19 PB7, PB10 |
| August Burns Red - White Washed: `testdata/input/common/IB24/T5/August Burns Red - White Washed` | default | best path activation 3 (squeezed-out chord tick 253320, stored end 253440) | A19 PB2, PB4, PB5, PB6 |
| How You Remind Me: `testdata/input/common/Summer Blast _25 Setlist/Tier 1/Nickelback - How You Remind Me` | default | the Dynamics tab Totals (905 notes) | A13 3, 4 |
| Corpus charts with a chart score for ties and paths not named (the test's own first chart) | default | engine records for every chart "unnamed" | A13 1, 2, 7, 11, 15; A26 RU26 |
| Chart with Hard missing (corpus: `corpus::first_chart_without_notes`) and a chart with no tempo at tick 0, or a bad Resolution | default | the failure sentence | A4 verify 19; A27 SI10, SI11 |
| Hormone's Echo (not in corpus; the user's, D78) | default | a real first-track-name file | A4 41 |
| `.mid` charts: one with two events tracks; one ending after `[section _ENDOFSONG]`; one with a drum track named PART DRUM; one with a stray modifier; one with a bad Offset | default | practice sections, time box, notes | A27 SI1, SI2, SI3, SI5, SI7 |
| Corpus `.chart` Offset lines and `song.ini` delay values (the fixtures in testdata/input have no song.ini per the screen index) | read by `parse_finite_number` | each value | A27 SI6, SI7, SI8; A27 SA3 |
| Charts with real stems (`.opus`, `.ogg`) and one damaged stem copy | default | mapped sizes, a stem that will not open | A27 SA1, SA2; A22 lines 104, 304 |
| A real chart with a negative `delay` | default | playhead offset sign | A27 SA3 |
| Fuzz seed 5 and 199 charts are generated; the capture asks for the real corpus equivalent | default | `hydra_replay target` on a corpus chart | A27 ST1 to ST3, ST6, ST7 |
| 4/4 to 3/4 chart; 4/4 to 6/8 chart; 6/8 chart; a real 4/4 chart | default | engine measure grid | A20 15, 16, 17, 18, 19 |
| A chart with an audio longer than its stated length, and charts with a stated length | default | song length, beat lines stop | A20 3, 20; A21 2, 4, 13, 14, 15 |

## 2. Screen captures

| Screen | Chart and path | Moment or state | What is read | Records |
|---|---|---|---|---|
| Paths tab | Beg (activation 3), Pitch Dark, late_sqin_twice, Believer path 4, Burnout, Tapped Out | badges and rows for early fill with time to spare, orange badge (late_sqin_twice activation 1), squeeze sentences, near-1 multiplier, unknown scale, chord names with Pro Drums on/off, the backend limit set (hidden rows count), a limit at +60, the Transfer scale hint tooltip, the "SP overfilled" text, the timeline labels, the Copy path text of an activation with a squeeze and of the user's row | colours, text, copied text vs the badge | A2 3, 6, 8, 9, 12, 14; A3 1, 2, 3, 4, 7, 8, 9, 10, 11, 12, 14 to 21; A18 5, 10, 21 |
| Paths tab | Pitch Dark, Burnout | the path buttons: hardest timing, 1-bar cap reason, groups and "below optimal", Collapse/Expand all, folds after a path change | text | A19 PL2, PL3, PL4, PL5, PL10, PL11, PL12, PL13, PL14, PL15 |
| Preview picker | chart with tied paths | repeated labels get separate ids | the picker list | A19 PL15 |
| Path Builder (not captured at all) | White Washed, Overrated, Feast of Fire | choices hint, candidate row clock/phrases, cost order, step short, end choice, fill refusal, fill card states | text and card | A19 PB1 to PB10 |
| Preview highway (real GPU) | Pitch Dark `song_start`, a chart with cymbal, ghost, accent, 2x kick | drawn lanes, colours, SP window clip, taken/offered/hidden fills, hit flash 50/150/200 ms, lit lanes | picture | A15 6 to 10, 12, 15, 16, 24, 27, 28; A20 1, 2, 4, 6, 7, 9, 10, 11, 12, 13, 27, 28, 29; A22 renderer lines 375 to 543 (screen B) |
| Preview highway | a chart with ghost path (Path Builder) and a ghost fill | ghost fills' strips and edge | picture | A15 20, 21, 29; A20 29 |
| Preview highway / time box / scrubber | a corpus chart with a stated length; one with audio past its length; one with Pro Drums on and off; a chart in two modes at the song's end (SP drain box) | end of chart, scrubber at right end, beat lines stop | picture/text | A20 3, 15 to 20; A21 2, 4, 5, 13, 14, 15, 22 |
| Preview panel | corpus chart with audio; no audio device; audio device that won't start; assets folder missing; a chart lacking the chosen difficulty; Preview volume slider | warning hint, "Reinstall Hydra", failure line, scrubber, volume range | text | A21 8, 9, 11, 26, 27 |
| Preview loading bar (screen A) | real chart with real stems, mid-load; stemless chart; load longer than 3 s | bar fraction, label, time left | text | A22 lines 37, 74, 104, 137, 166, 197 (and 272, 304 inputs) |
| Preview "timeline rows" (screen C, Path Builder) | Pitch Dark, Believer path 0 and path 4 with a saved path; a chart whose last section is "End"; a real section list | timeline rows, meter curve, saved_enough mark | text and screenshot | A22 timeline lines 626 to 1095 (21 records) |
| Preview time box | a corpus `.mid` | practice sections, after `[section _ENDOFSONG]` | text | A27 SI1, SI2 |
| Preview score box | Beg: combo 239 | combo at a chord | text | A23 2 |
| Library view | corpus library, with a query typed (accented `beyonce`, full-width, quoted phrase, `artist:`, `stars:N`, `squeeze<=N`, bad operator, empty box), highlight and the "Library n of m charts" label | search match count, highlight, error line | text | A17 1 to 4, 7, 9, 10, 14, 16 to 26, 28; A6 1, 2 |
| Library view | Burnout analyzed, stars and hardest columns, and a chart whose hardest is exactly 0.0 | stars and squeeze filters, column text | text | A17 18 to 23 (C2, C3) |
| Library view | real chart set | row refresh after batch, sort held order, selection highlight, status line during batch, scan, notices, startup screen's words, failure sentences | text | A6 3 to 15, 18, 20, 21, 24, 26, 28, 32, 34 to 38 |
| Library view | library of 12,345 charts; empty library in both states; real Library at 1,000 px, narrow widths, song panel open, sorted header, dragged column, hidden column in `hydra_ui.ini`, long titles | count label, batch confirm, column widths | text/picture | A8 1, 12, 22 to 27, 30, 32, 35, 39 |
| Batch confirm and batch strip | library with charts that cannot be analyzed; default settings and Hard with Pro Drums off; SP cap 1 and 1000; Fills line in both fill states; a chart over an hour | confirm lines, time left, the strip mid-run, "can't analyze" line | text | A8 2 to 5, 8, 10, 11; A16 3, 4, 9, 10, 11, 16, 17 |
| Settings bar | each setting changed, locked; Reset tooltip in three states; 1.0 fill tooltip and the fill report side by side | summary text, tooltip | text | A8 6, 7, 11; A9 39, 41; A13 24, 25 |
| Scan dialog and version bar | mid-scan | counts 1,234 and the title | text | A8 9; A9 42; A22 line 137 |
| Library | row for a copies count (chart in several folders) | copies figure | text | A1 `sample_index`, `counted_against`; A25 P5, R7, R8 |
| Library and Paths header | a chart whose title/artist/charter is only tags; tagged name `<color=red> Hoph2o </color>`; empty artist | "(unknown)", cleaned name | text | A27 SI12, SI13, SI15 to SI19 |
| Path report window | corpus charts with filters on; a chart in several folders; shared title and artist; typed search with an accented artist; sort by header click; Timing sort; hover text; dropdown; a window in "out of date" state | tiles, counts, rows, search, column order, header measure | text | A1 `sample_path_rows`; A25 P3, P4, P5, P12, R5, R7 to R9, R11 to R13, R16 to R22, R24, R27, R29, R30; A26 RV1, RV2, RV5 to RV10, RV14; A7 (window records, see section 4) |
| Path report window, closed | build, cancel, reopen, Refresh, `--open-report`, batch, setting change | window state and rows | text | A7 1 to 23 |
| Comparison (Compare) window | a real DMBot player; one 150 percent score and one "unknown" entry; off-speed score above optimal; tagged real chart; empty charter; scanned-but-unanalyzed chart; no-path chart; result for a chart outside the library; cap 8 with both caps analyzed; filter on | status words, delta cell, % of opt cell, chip colours, nine tiles, header, subtitle, footer, "out of date" notice, search "beyonce" finds "Beyonce" | text and picture | A10 1, 2, 4 to 15, 19, 20, 28, 29, 30; A9 25, 43 to 46; A26 RV11 to RV13; A1 `sample_dm_rows` |
| Library toolbar | Compare button disabled (non-Expert, Note Shuffle on, cap 8) and its tooltip; the window's failed-build state after Refresh at Hard | tooltip and disabled state | text | A10 21 to 25 |
| Fill report page (FILLPAGE) | a corpus where one rule only analyzed; a chart that fails under one rule; a chart with no path; a duplicated chart; one chart; empty db; accented and tagged real chart; scrolled and print preview; both colour schemes | rows, status words, tiles, the `#` column, title, count line, sort | text/picture | A14 1 to 22; A25 P1, P2, P7 to P11; A13 24 |
| Report table (path report and others) | the real table drawn twice at one width and at two; a dragged column then Clear filters | cell fits, column widths | picture | A14 33 to 38; A8 18 to 21, 28, 29, 31, 33, 34, 36 to 38 |
| Report-open buttons at a long path | long chart path | button text | text | verify A17 (C7) |
| Library tips popup | none named | popup | text | verify A17 (C6) |
| Window title bar and taskbar | the Hydra window | title and id | picture | A9 42 |
| Path Builder / Paths tab | a path report with settings changed | the "out of date" notice | text | A25 R29; A10 30 |
| Report windows placement (open, maximized, second monitor, gone monitor, DPI scale) | `hydra_ui.ini` states | window placement and scale | picture | A7 24 to 47 |

## 3. Real inputs for files, store and settings

| What | Compared against what | Records |
|---|---|---|
| A real `hydra_settings.ini` from the machine, and a real 2.11.x one | the load/save keys; defaults; the `#` comment rule with a real `#` library path (a folder like "#1 Hits"?, not searched); `sp_cap=auto` file from 1.8.4; `hit_window_ms=85` older file; CRLF bytes; refused-file message on screen | A9 1 to 6, 8 to 15, 18 to 24, 27 to 38, 40; A5 21, 22, 23; A21 26, 28 |
| A real `hydra.db` (current and older, v2.1.0 schema 4 under `testdata/store/`) | the `chartmode` column keys vs `chartmode_key`; profile columns; display_title on "<unknown title>" | A9 16, 17, 23; A27 SI14; A5 4, 5, 24 |
| A real `hydra_rules.ini`, an old one with `fill_length_measures`; a real settings folder with no rules file | rules load, error text orange line under the toolbar | A26 RU2 to RU6, RU22; A5 21 |
| A real stored library file saved by an older build | the startup open's words (Believer, corpus chart) | A16 18 |
| A real `hydra_ui.ini` read and written back; a real settings file for the UI scale and the library split | the placement text, fonts | A7 25, 36 to 38, 41 to 44; A17 36 |
| Real hydra_batch database made with folder arguments; one without a chart library | the "no chart library" sentence | A10 27; A14 21; A25 R27 |
| A saved real DMBot `/all-users` and `/user/<id>/scores` response (source named) with a tagged name | the parsers' result, the joined rows | A10 3, 11; A25 P6; A26 RV12 |
| Real WinHTTP against a closed loopback port, and a loopback server | job reaction to a cancelled / failed fetch | A16 23, 24; A10 16 |
| A corpus `song.ini` in Windows-1252 from a real chart folder; invalid UTF-8 name; `Notes.mid`/`Song.ini` with capitals; a folder without audio | the decoder, `pick_notes_file`, `remembered_failure` | A4-q3 35, 40, 27; A17 6 |
| Corpus scan rows (`discover_charts` on `testdata/input`, a folder given as a second root) run through `run_batch` and a store, with Stop after the fifth report | `ChartIndex`, store contents, counts | A4-q3 2, 6, 12, 17 to 22, 42, 43 |
| A real folder to scan with a corpus root (a scan that throws) | the files and store | A16 1; A16 21 (results read in slices) |
| A real `.opus`/`.ogg` stems and damaged stem; real missing song file on a corpus chart | mapped size, failure plain words | A27 SA1, SA2; A16 11 |
| Real unreadable stem; the shipped `assets/preview/textures` folder (28 files) | every `TextureId` maps to a file | A22 lines 304, 338; A15 23 |
| The real docs: every backticked span of `docs/clone-hero/` pages vs `dump_facts.py`; `facts-6142.tsv`; the real README line 57; a real marked doc line; a real plan with one row broken; a real doc and source file with one dangling path; every `.cpp .py .ps1 .yml` through the lexer | span-by-span against the tools | A11 3, 4, 6; A12 1, 2, 3, 4, 7, 9, 11, 14 |
| This repo's git history `c6debcd^2...c6debcd`, and a real range outside a fixture | the reviewed findings | A19 PC1, PC2, PC3 |
| A real `hydra_tests.exe` run: unquoted `-sf=*test_x.cpp` from PowerShell; `-sf=a -sf=b`; both spellings; a filter naming no case; an exclude-only run; the lines from `tools/check_shards.ps1` | doctest's own count | A26 RG1 to RG6 |
| `testdata/preview/notes.mid` and its saved Onyx screenshot, with `golden.json` | engine record for that chart | A21 29, 30 |
| A real song with a `.mid`/`.chart` Offset or `song.ini` delay, a negative one | `parse_finite_number` against the game's reader | A27 SI6, SI7, SI8 |
| A rendered report page opened in a browser (search with accent and two words; contrast in both themes; header stick; print) | the shared script and fold table | A17 11, 29 (C5); A25 P2, P9, P10 |
| A real fill comparison page written twice to a real folder | no `.tmp` left, bytes | A25 P1 |
| A real `hydra_batch` with the library (a chart folder with a bad song) | "can't analyze" count | A8 13 (a tool rule, not a chart matter) |

## 4. Real hook inputs

None. No A1 to A27 record names a hook, recorded hook call or hook state as its missing input. Searched: the "3 Real chart" answers, the verify files' "not checked" and "needed" lines, and a grep of `records/A1.md` to `A27.md` for "hook" (the hits are test hooks inside C++ tests, e.g. A7's `wait_until` hook, not Claude Code hooks). Zero rows.

## Per records file

"Not checked" is the number of records whose question 3 says "claim not checked" (counted from the file; A2, A4, A5, A8 counted from their bullet form; A4 also from `verify/A4-q3.md`, which adds the Keeps it flips). "Lines" is the number of lines in the four tables above that name the file's records, counted by hand from the merged rows (a merged row counts once for each file it names).

| File | Not checked | Lines |
|---|---|---|
| A1 | 4 | 3 |
| A2 | 10 | 8 |
| A3 | 18 | 12 |
| A4 | 1 (plus 14 from verify A4-q3) | 6 |
| A5 | 10 | 5 |
| A6 | 27 | 3 |
| A7 | 47 | 4 |
| A8 | 39 | 6 |
| A9 | 46 | 6 |
| A10 | 28 | 5 |
| A11 | 3 | 1 |
| A12 | 9 | 1 |
| A13 | 24 | 13 |
| A14 | 36 | 10 |
| A15 | 23 | 9 |
| A16 | 12 | 6 |
| A17 | 29 | 7 |
| A18 | 19 | 10 |
| A19 | 27 | 11 |
| A20 | 26 | 9 |
| A21 | 16 | 6 |
| A22 | 36 | 5 |
| A23 | 11 | 6 |
| A24 | 17 | 10 |
| A25 | 33 | 7 |
| A26 | 42 | 6 |
| A27 | 38 | 12 |

Gaps to say plainly: the per-record counts include the terse "claim not checked." records of A6, A7, A8 (18 to 38) and A25 (R11, R12, R13, R20, R24, R30), whose need is only implied by the file title, so their rows are grouped by the screen the file names. The "lines per file" counts are by hand and can be off by one or two. The `verify/` reread files for A3-A4, A7 were read only through their capture lines.
