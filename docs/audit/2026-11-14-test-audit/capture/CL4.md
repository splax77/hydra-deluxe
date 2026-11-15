# Capture list CL4 (T5-CL4): E1 to E4, delta-*, window-*

Scout CL4, agent a34f9d9fb0418c238. I read the records files' "claim not checked" records by script (every record whose text says "not checked"), then read each hit's own words in `delta-DL1` to `DL5b`, `window-WN1` to `WN8`. For E1 to E4 I read the shared answers and the per-section needs, and list the records by test line. I read the matching `verify/` files by grep for "not checked", "capture", "needs a": they agree with the records and add no new thing the records do not name, except that the E verify files turn some Keep records into "claim not checked" disputes (named in section 4). I judged no test and changed no record.

How to read the charts column: a path is a real path under `testdata/input` (found by search). "Corpus" means the checked-in `testdata/input`; no row needs the whole library. Every window-* record is at 60 ms because the default early-fill window changed; every dump the records ask for is "at `--efill 60`" (and often also 85 to compare), since all seven stored dumps in `records-engine/` were made at 85.

Record ids: `WN4b 3` means record 3 of `records/window-WN4b.md`. `DL5 1` is record 1 of `records/delta-DL5.md`, and so on. E files use the test's line number, like `E2 test_progress.py:139`.

## 1. Engine dumps

| Chart | Settings asked | What is read from it | Records that need it |
|---|---|---|---|
| `testdata/input/common/Summer Blast _25 Setlist/Tier 4/Green Day - Burnout/notes.mid` (Burnout) | `hydra_replay dump` and `schedule`, `--efill 60` (and 85 to see if the path moves); the settings of each test: default scores depth 2, 10 ms, Pro Drums (Path Builder: default, depth 2); also both leeways (rules) for WN6 15 | best path string (typed "3- 1 2"), total (typed 378,315 and 7 stars), activation count and ticks (typed 3), first activation squeeze-out ms (typed 163.0; 5.4 ms safe), second path (`all_paths()[1]`), path-list buttons and folds, backend rows and rating lines, multiplier squeeze, uncounted rows, offered fill ticks (is tick 36480 still offered), ladder `bf` list, candidate costs, score under each leeway (typed 381,075) | WN2 7, 14; WN4 44, 7-33 (pb_coach/pb_text rows that name Burnout; see the generic row below); WN4b 1, 5, 9, 10, 12; WN4c 1, 2; WN5 1, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16; WN6 1, 3, 4, 7, 8, 9, 10, 11, 13, 14, 15; WN7 1, 2, 3, 4; DL5 delta rows for `test_activation_rows.cpp:582`, `:1331`; DL5b shared note (Burnout "not among the seven") |
| `testdata/input/common/IB24/T5/August Burns Red - White Washed/notes.mid` (White Washed) | `--efill 60`, Path Builder settings (default, depth 2) | best path's five activations, which one is over 10 ms and its figure; all-0 path text and sixth activation's squeeze-in figure; fourth activation's `sp_end_steps` (two ends?), squeeze-out (typed 130.4 ms, m135); are fill ticks 165120, 199680, 245760 offered | WN4b 3, 4, 6, 7, 8, 13, 14, 15, 16, 18, 19, 20; WN6 5 |
| `testdata/input/common/IB24/T1/Smash Mouth - I_m A Believer (The Monkees cover) [highfine]/notes.chart` (Believer) | `--efill 60`, default `app::Settings` cap and depth | path 0 and ranks 1 to 5 (typed "1 0+ 0 0"; rank 2 "1 0+ 2"), squeeze-in at exactly 0.0 ms, figures per rank, schedule act ticks 26880, 52992, deact 59136, end 57600, end 74496 | WN1 W6; WN4b 25, 26, 27, 30, 31; WN6 6 |
| `testdata/input/common/IB24/T4/Foo Fighters - Everlong/notes.mid` (Everlong) | `--efill 60` (no record at any window today), default settings | rank 1 text (typed "1 2 3+"), squeeze figure -189.8375..., Paths tab text | WN4b 28, 29, 31 |
| `testdata/input/common/IB24/T3/Trivium - Feast of Fire/notes.mid` | `--efill 60`, Path Builder settings | fill ticks offered with no window pinned, best path's activation and end ticks (72480, 180000, 195360, 156960) | WN4b 21, 24 |
| `testdata/input/common/IB24/T1/Allister - Overrated/notes.mid` | `--efill 60`, Path Builder settings | `trailing_skipped_fill_ticks` of the no-window path (typed 55680 ... 167040); BF(0) (typed 228,710), best finish with the m30 fill pinned (typed 226,210), 2,500 | WN4b 17, 22, 23, 24 |
| `testdata/input/common/IB24/T5/Mike Orlando - Tapped Out/notes.mid` (Tapped Out) | `--efill 60` and 85; `hydra_replay dump` and `schedule` | activations 3 and 4, `e_offset`, skips, the tag; whether an E0 exists at 60; squeeze-out reach; optional early fill (activation 4); typed 82.9, 85.7, 61.9, 48.1, 85.1, 67.8, 184.1 ms | WN5 16, 17; DL5 rows 10, 11, 13 (`test_activation_rows.cpp:1396`, `:1469`); DL5b 1, 3, 5 |
| `testdata/input/common/IB24/T7/Ne Obliviscaris - Tapestry of the Starless Abstract (Shortened)/notes.mid` (Tapestry) | `--efill 60` | an uncounted squeezed-out row among the optimal path's activations | WN5 11 |
| `testdata/input/common/Summer Blast _25 Setlist/Tier 3/Mutemath - Allies/notes.chart` | `--efill 60`, default `app::Settings` | best path (first stored path), 4 pins, ticks 49920, 22272, best total, `best.size() >= 3`, `afters.size() > 1` | WN1 W12, W13; WN2 12 |
| `testdata/input/common/IB24/T6/black midi - Sugar／Tzu (Smoochums, Vasasasasa)/notes.mid` (Sugar) | `--efill 60`, legacy fills, ms 0, tie limit 1 | tied-path set, typed "0 E3+ E5 E1" | WN1 W3 |
| A chart named HopH2O (two matches: `testdata/input/common/IB24/T6/HopH2O - Nasty (Tinashe)/notes.mid` and `testdata/input/common/Summer Blast _25 Setlist/Tier 5/HopH2O - I Am... All Of Me (Crush 40)/notes.mid`; the record gives only "HopH2O", do not guess) | `--efill 60`, cap 4, ms 10 | tied path kept over the Path limit, typed 694985, "0+ 2 0 0- 2 E0", "3 E0 1 0- 2 E0", "78.9 ms" | WN1 W4 |
| `testdata/input/common/Summer Blast _25 Setlist/Tier 4/Avenged Sevenfold - Unbound (The Wild Ride)/notes.mid` (Unbound) | `--efill 60`, cap 4, ms 10, tie limit 4 | the two paths over the Path limit, typed 859580, "2 2+ 3 0+", "2 2+ 0- 3" | WN1 W5 |
| Fuzz seed 5 and seed 199 (`fuzz_chart(5)`, `fuzz_chart(199)` written to a file; not in `testdata/input`) | `--efill 60`, cap `fc.cap`, Scores depth 40 | six path strings and totals, the promoted vector, `realized_prefix == 2`, path "0+- 0-", tick 12864, tie counts | WN1 W7, W8, W9, W10, W11 |
| `testdata/input/common/IB24/T1/Evans Blue - Beg [highfine]/notes.chart` (Beg) | `--efill 60` and 85; and window 0 for the E rule | best path "1- 0 0 E0", E0 at `e_offset` 0 (activation 3), the Paths tab row; the walk at 60 against 20 ms of limit; E1 checks; row 4 early fill 0.0 ms | WN3 1, 3; WN5 8; DL4 test_model 5, 6, 3 (capture b); DL5 2, 3 |
| `testdata/input/test_pitch_dark_ratio` (the chart folder) | `--efill 60` (record names `pitch_dark_ratio`) | skipped fills 86400, 94080, 101760; all-0 and E1 evidence; schedule step scales | WN3 3, 4; WN4a 7 |
| `testdata/input/test_folded_sqin/folded_sqin.chart` | `--efill 60` | skipped fills 4608, 10752; SqIn paths exist at 60 | WN3 4; WN2 22 |
| `testdata/input/test_folded_sqin/relabel_case.chart` | `--efill 60` | every `e_offset` stays out of the 60 to 85 band; phrase at 9000 | WN3 9 |
| `testdata/input/test_folded_sqin/clamped_sqin_a.chart` and `clamped_sqin_b.chart` | `--efill 60` | which squeeze-outs exist (`sqouts > 0`); fills 14784, 14880 | WN3 10 |
| `testdata/input/test_fast_tempo/spent_then_next.chart` | `--efill 60`, cap 2 | is "0++" still a path with a squeezed-in phrase; `hydra_replay score --path` | WN4 5; WN4a 4 |
| The fast-tempo charts (`testdata/input/test_fast_tempo/`, both charts the records name: "both charts" at `test_fast_tempo.cpp:229`) | `--efill 60` | does a path reach the SP end; SqIn step per SqIn; seeds 15648, 13248 at 16224 | WN2 15, 16, 17, 18, 19, 21 |
| `testdata/input/common/Summer Blast _25 Setlist/Tier 8/Eric Moore - Get Ur Freak On [Missy Elliot]/notes.chart` | `--efill 60` and 85, `schedule` | the E marks that "need" 85 | DL5 rows for `test_activation_rows` (shared Get Ur Freak On note) |
| `testdata/input/common/Summer Blast _25 Setlist/Tier 7/SoundHaven - Focus [Ariana Grande]/notes.mid` and `testdata/input/common/Summer Blast _25 Setlist/Tier 8/Venetian Snares - Epidermis/notes.mid` | `--efill 60` and 85, `schedule` | pathstring "0 3+ 0 0" for Focus; the squeeze-in with timing names its anchor | DL5 10; DL5b 2, 4 |
| A corpus chart whose SP-ready note is within 1 ms of a fill deadline plus 60 (the record says "a corpus chart", names none) | `--efill 60` and `--efill 85` dumps | fill at the reach edge stored or not | DL5 1 |
| A corpus chart that needs a 70 ms early hit (not named) | `--efill 60` and 85 | required early fill at 70 ms, no spawn at 60; E at 85 not at 60 | DL5 2, 3 |
| A corpus chart with an activation passing over exactly one fill within 85 ms of SP ready (not named) | `--efill 60` and 85, plus the Paths tab's all-0 section | an E1 at 85, a skip at 60 | DL5 4; DL4 test_model 2 (capture c: a real all-0 path with an E1 at 60 and 85) |
| A real chart where a fill refuses (engine `refused` count at `engine.cpp:862`; not named) | `--efill` 60 | the refusal and E0 at the same window | DL4 test_model 4 |
| A real chart analysed at windows 0, 60, 85, filed to the store (not named; "first corpus chart") | `--efill` 0, 60, 85 | `early_fill_window_ms` on the record; two records for one chart; the early-fill result that differs between windows | DL1 591, 608; DL4 test_model 3, 5, 6; DL4 test_config 5 (capture b) |
| A real 1.0-rule chart (`--legacy-fills`; not named) | `--efill 60` | the stored passed-over fill beside the nearest fill; the offered fill the engine charged | WN4 6, 32; WN4a 6, 9 |
| The corpus, every chart under default `SearchSettings{}` (checked-in `testdata/input`, not the library) | `--efill 60` | path counts and totals the floors depend on: replay equals engine, targeted search, tied variants, SP-end history, banks, stars (base score sum), best finish, squeeze corpus, activation rows, engine sweeps (E1 and 0 ms charts, saved negatives, `allzero_charts`) | WN2 1, 2, 3, 4, 6, 7, 8, 10, 11, 13; WN3 3, 4, 6, 7; WN4 1, 2, 3, 4, 45; WN4a 1, 2, 3; WN4c 3; DL5 18 |
| "The first corpus chart with paths" (`first_chart_with_paths`; unnamed, main session must read which chart it is) | `--efill 60`, depth 2 or 0 as each test sets (sample_chart depth 0; preview tests depth 2 and 10, 10 ms) | best path's score (typed 9956 needs optimal about 224,700 to 229,900), stars (typed seven), activation count, at least two unique path labels | WN7 5; WN8 1 (`test_dm_report.cpp:61`); WN4a 10, 11, 12, 13, 14, 15; WN5 2, 3 (chart 0) |
| The chart `test_app_click.cpp:162` uses (not named by the test) | `--efill 60` | keeps more than one path at 60 | WN4 38; WN4c 4 |
| The chart each of these tests uses (each test names its own; the record's wording is "dump of the chart the test uses (named in the test)") | `--efill 60` | the path, count, E mark or activation the old record took from an 85 dump | WN4 1-45 not listed above: replay, `test_pb_coach.cpp`, `test_pb_text.cpp`, `test_preview_scene.cpp`, `test_pathstring_read.cpp`, `test_saved_figure_fill.cpp`, `test_cli.cpp:581` |
| Same, for fillcompare | `--efill 60` | is the equal score "1 same" at 60 | WN4 43; WN4c 5 |
| `testdata/preview/notes.mid` | `hydra_replay dump` at `--efill 60` and 85 | whether an activation is open at 36913 ms | WN4c 6 |
| Engine run of Pitch Dark and Burnout for the preview windows | `--efill 60` | ticks of the squeezed chord (A29 #27) | WN2 7 |
| Settings-key and digest: no dump; a golden digest of the corpus at 60 | not asked | none: records call these "the golden question" with no independent owner | DL5b 11-18 (nothing to capture; listed so none is missed) |

## 2. Screen captures

| Screen | Chart and path | Moment or state | What is read | Records |
|---|---|---|---|---|
| Library table row and details panel | a real corpus chart (not named by the records) | during a batch save group; after a click's save; click replaced, cancelled, closed or dropped | row text and status, panel text equals the store's answer | DL3 1, 2, 5, 6, 7, 8, 9, 10 |
| Library rows and chips | a corpus chart held in two folders (not named) | batch landing mid-run, after a batch, paused, stopped, failed as a whole | row text, chip counts (Analyzed, Stale, Can't analyze), sort holding order until the sort | DL3 12-20, 28; DL4 jobs 1-7 and library model 1-5 (capture a) |
| Library status line | a real fault (broken reader, a lookup the batch could not make) | a failed read | the status line's text, the store's sentence | DL3 16, 25 |
| Library column / row of a corpus chart with a squeeze-in at 0 | a corpus chart (not named) | after analysis | the Timing column text | DL5 18; WN1 W1 (Library Timing column reads the best path figure) |
| Paths tab count | the W2 fixture chart | 60 ms | count of kept paths | WN1 W2 |
| Path report window and comparison (DM) report window | a real set from the corpus | build while a save group is open; out-of-date strip after a click | the report text and the "out of date" strip | DL3 11, 32, 33, 34, 35, 36, 39, 40 |
| Settings button label and tooltip, Analysis settings panel | no chart | early-fill window at 85 and 0, and with three changes then "defaults"; panel during a batch with the `##earlyfillwindow` row greyed | label "Analysis settings: ... Early fill window 85 ms", tooltip, greyed controls | DL4 test_config 8; DL5 21, 22, 23, 6 |
| "Analyze library" confirm dialog | none | default settings and window 85 | the Early fill window line | DL5 5 |
| Live transcript of `click -` with the settings panel open | none | the panel's number inputs | the reply text and the match count | DL5 24 |
| Paths tab | Get Ur Freak On; Burnout; Beg; Tapped Out; Tapestry (paths above) | at 60 and 85; narrowest panel (Burnout); all-0 section | row text, badges, widths, E marks | DL5 rows for `test_activation_rows`; WN5 7 to 17; DL5 4 |
| Preview tab overlay | chart 0 (first library row; main session must read it), Burnout | one capture per path; a changed chart file; drain box; scrub marks; path picker | overlay per path, hidden path after a change | WN5 2, 3, 4, 5, 6; WN4a 15 |
| Preview golden: Hydra frame against the Onyx screenshot | `testdata/preview/notes.mid` | at 60 and 85 | whether the overlay matches the screenshot | WN4c 6 |
| Path Builder window | Burnout, a long song (not named), Set the World on Fire (`testdata/input/common/IB24/T5/Symphony X - Set the World on Fire/notes.chart`) | build by clicking; finish-for-me; locked fill; offered list near 3:58; refused settings; refused rules | the top line, card, step row, clock | WN6 1, 2, 4, 7, 8, 9, 10, 11, 12, 13, 14, 15 |
| Path button list (Paths tab buttons) | Burnout | its path list beside typed values | button groups | WN7 1, 2 |

## 3. Real inputs for files, store and settings

| What | Compared against what | Records |
|---|---|---|
| A real library `hydra.db` from release 2.2.0 to 2.7.0 (README says none is checked in) | the first open turns its results into records and keeps every status; a stop mid-conversion leaves the results table whole | DL2 5, 6 |
| A real library file from release 2.8.0 (without the failure columns, with a charts row) | opens and gains the failure columns and the album column in one open | DL2 2, 4 |
| A real library file from a release before the album column, non-empty charts table | gains the album column in place | DL2 3 |
| A real library file from a release between the album column and the early-fill column (the real v2.12.1 file classifies `LacksAlbumColumn`, since album is named first) | `classify_library_file`; profiles table rebuilt with the early-fill window keeping every row | DL2 1, 8, 9 |
| A real v2.12.1 copy with the rebuild forced to fail | open fails as DatabaseUpgrade and changes nothing | DL2 10 |
| A real library copy (a scan of `testdata/input` or a copy of the user's library) read through the writer and the reader, with raw SQL as the independent reader | reader gives the writer's answer; chart index; list by name; rescan seen; own reader; counts | DL1 171, 260, 334; DL2 11, 12, 14, 15, 17 |
| A real older-release `hydra.db` | every accepted results stamp reads Ready, others Stale | DL1 1206 |
| A real second `hydra_rules.ini`, and a real broken one | a record made under other rules reads Stale; a failure under other rules; a save under another real rules file reads Ready | DL1 178, 917, 1288, 1328 |
| A real profile on a real library | Profile compares on every part of the identity | DL1 `test_store_results.cpp:339` |
| A real `hydra_settings.ini` copy, with and without the early-fill line | settings round trip, load and save name the same keys, missing file gives defaults, window loads 60 without the line, range edges | DL4 test_config 1, 2, 3, 4, 6, 7 (capture e) |
| A real saved library copy at the new schema | `tools/test_compare_db.py` record tables with `efill_window_ms` | DL5 28 |
| Two real stores written by `hydra_batch` over `testdata/input` (one with profiles inserted in another order; one with a result changed), compared by an independent row-by-row read | `test_compare_db.py` profile-order and changed-record tests | DL5b 9, 10 |
| A real marked line from the docs (such as `UserGuide.md:135`) with its number changed | docs-match-code marker check | DL5b 8 |
| A real `hydra_rules.ini`/stored record of the early-fill answer under a real older release | which window an old row belongs under (no ruling found) | DL2 8 |
| A real library file (a real library from a release with the failure columns) for a first-run missing INI | default settings on a first run | DL4 test_config 3 |

## 4. Real hook inputs

All E1 to E4 records are about hooks in `C:\Users\Patrick\.claude\gates`, not Hydra. Their records say the input is hand-typed ("claim not checked"), and name the real thing: a recorded call replayed by `gates/tests/replay.py` (extracted by `replay_extract.py`), a real transcript, or a real repo. The records give these recorded states per hook; I did not open the extracted calls.

| Hook (test file) | The recorded call or state needed | Records |
|---|---|---|
| `agent_stop.py` (test_agent_stop.py) | a real recorded reviewer hand-back with no sign-off line; a real hand-back saying BLOCKED; a real recorded stop payload with real transcript; a real failing git call; a real repo with a real unreadable object (or the user's word a mock of git is enough). Verify E1a/E1b also dispute Keep to "claim not checked" for stop-payload tests at :164, :171, :343, :353, :359, :365, :371, :378, :416 | E1 test_agent_stop.py:188, 195, 287, 302, 316, 343, 474; verify/E1a.md disputes 1, 2, 5-10, 13 |
| `common.py` (test_common.py) | a real Claude Code deny reply or the protocol page | E1 test_common.py:28 |
| `percall.py` key rule (test_percall_key.py) | replay of recorded call (capture RC); a real waiver record and real user prompt from a transcript; the real `refusals.jsonl` shape | E1 test_percall_key.py:41, 45, 51, 58, 157, 201 |
| `percall.py` docs numbers, bash windows rule, delete rule, media rule (test_docs_numbers.py, test_bash_windows_rule.py, test_delete_rule.py, test_media_rule.py) | replay of recorded Bash calls and recorded subagent shell calls (capture RC); records say commands are typed by the test "claim not checked unless stated" in their shared Q3, with no per-record "not checked" | E1 shared note (test_docs_numbers.py:192, test_bash_windows_rule.py:313 and the files' other records) |
| `percall.py` rewrite (Group R, test_percall_rewrite.py) | a recorded refused delete from `state\refusals`; a recorded real status call from `state\status`; a recorded chained call; a real ffmpeg call | E2 test_percall_rewrite.py:52, 61, 66, 72, 84, 88, 93, 129, 134, 139 |
| `posture.py` (Group P, test_posture.py) | a real compaction event; real stored state (bad state) | E2 test_posture.py:57, 61, 72, 82, 90, 99, 108, 113, 119, 123, 128, 133 |
| PromptHook (Group H, test_posture.py:139-274) | a real prompt payload | E2 test_posture.py:196, 205, 209, 215, 264, 271 |
| `progress.py` (test_progress.py) | a real status line with the matching words; a real refusal; a real command; a real transcript with a typed allow; a run on real PowerShell; rulings named | E2 test_progress.py:139, 147, 153, 158, 186, 245, 275, 282, 286, 306, 314, 379, 385, 390, 413, 418, 423, 431, 437, 442, 446, 456, 468, 487, 498, 501, 509, 515, 523, 533, 539, 543, 551, 555, 566, 606, 609, 620, 627, 633, 642, 651, 695, 702, 712, 716, 728, 732 |
| `publish_gate.py` (test_publish.py) | a real gh help; a real plan from a real session; a real review record; a real `gh` | E2 test_publish.py:100, 107, 116, 119, 127, 144, 149, 154, 162, 167, 176, 184, 191, 220, 228, 304, 310, 319, 325, 331, 339, 349, 358, 371, 378, 389, 395, 401, 413, 419, 424, 430, 436 |
| `refusals`, `common.deny` (test_refusals.py) | the real ref moves of `C:\Users\Patrick\.claude` and the Hydra repo (`tests\reflog_moves.json` lists 294 of them; `ReflogReplay` reads them only with `GATES_REFLOG_REPLAY=1` and only for the main-review rule) | E3 test_refusals.py:43, 52, 60, 69 |
| `review_dispatch.py` (test_review_dispatch.py) | a recorded real reviewer dispatch payload (`tests\replay.py:229` replays such payloads; these tests do not use them) | E3 test_review_dispatch.py:76, 85, 90, 103, 123, 128, 135, 139, 147, 150, 154, 161, 165, 169, 175, 183, 189, 194, 198, 241, 247, 251, 255, 260, 264, 270, 279, 297, 303, 308, 328, 345 |
| `ref_judge.py` (test_ref_judge.py) | the judge fed the real ref moves (`reflog_moves.json`) for trailers and rewrites; real transcript records for a typed prompt, a tool-result answer and a sub-agent prompt, from a real session file (this session's transcript holds 0 lines with `"promptSource":"typed"`) | E3 test_ref_judge.py:136, 147, 153, 184, 201, 210, 221, 232, 239, 248, 268, 274, 285, 293, 303, 311, 335, 349, 362 |
| `review_rules.py` (test_review_rules.py) | `explain_main_move` on the real moves in `reflog_moves.json`; `change_is_comments_only` over the real Hydra commits that touched `src/ui` (real diffs); a record on the new history; a real transcript for a human prompt | E3 test_review_rules.py: 110 records at lines 100-902 (list in `eids` of records/E3.md File 4; all lines carrying "not checked": 100, 106, 114, 129, 134, 149, 164, 174, 182, 195, 203, 213, 224, 252, 256, 260, 269, 279, 282, 295, 303, 313, 317, 324, 328, 341, 358, 364, 370, 376, 386, 395, 425, 433, 439, 446, 453, 461, 466, 471, 485, 494, 503, 512, 519, 524, 532, 558, 564, 568, 573, 577, 581, 585, 589, 594, 599, 604, 609, 614, 618, 622, 626, 631, 636, 640, 645, 650, 654, 658, 662, 667, 673, 677, 681, 685, 690, 694, 704, 708, 713, 717, 721, 725, 731, 736, 740, 744, 748, 753, 759, 764, 769, 774, 779, 784, 795, 807, 813, 820, 827, 835, 839, 845, 881, 884, 888, 892, 898, 902) |
| `ruling_rules.py` (test_ruling_rules.py) | a real card for a real session; a real transcript through `prepass.condense`; a real ruling-file diff | E4 test_ruling_rules.py:130, 133, 136, 139, 145, 150, 159, 168, 171, 174, 181, 186, 197, 200, 203, 206, 209, 222, 226, 230, 234, 239, 243, 247, 253, 259, 267, 283, 286, 291, 297, 302, 308, 313, 319, 322, 326, 350, 355, 363, 373, 378, 385, 392, 412, 428, 437 |
| `stall.py` (test_stall.py) | a real `agent_stop` record with a real problem; a real transcript record carrying `usage_line` | E4 test_stall.py:62, 68, 72, 77, 84, 88, 94, 104, 112, 123, 136, 141, 155, 165, 169, 177, 196, 205, 211, 216, 220, 223, 227, 265, 270, 275, 290, 295, 300, 305, 315, 356, 360, 381, 410 |
| `shell_output.py` (test_shell_output.py) | a real PostToolUse payload with a real `persistedOutputPath` and saved file (verify/E4a disputes 7, 9, 10, 11: `{}` and `C:/no/such/file.txt` are invented; `RuntimeError("boom")` patch at :331 is not a thing the hook met) | E4 test_shell_output.py:52, 56, 60, 64, 68, 72, 89, 93, 102, 106, 109, 114, 117, 121, 124, 138, 157, 165, 174, 178, 182, 186, 189, 194, 205, 209, 214, 224, 234, 241, 260, 268, 277, 283, 290, 295, 299, 304, 308, 312, 316, 328, 333, 343, 349, 353, 365, 370, 402, 407; verify/E4a.md disputes 2, 7, 9, 10, 11 |
| `session_cards_start.ps1` (test_h53_session_cards_start.ps1) | the record says "3 for all: claim not checked"; the real form agents use (a real session folder and real cards, a real bare id) | E4 test_h53_session_cards_start.ps1:110, 111, 112, 113, 115, 120, 121, 122, 123, 131, 132, 133, 135, 136, 141, 143, 144, 147, 150, 155, 156, 158, 163, 164, 167, 172, 173, 176, 183, 184, 185, 189, 190, 191, 213, 225, 226, 232, 237, 240, 243, 253, 254, 265 |
| cleanup sweep (test_h52_cleanup_sweep.ps1) | the record's shared Q3 says typed line and temp folder; the real state it sweeps; I did not itemise the exact real state each record needs | E4 test_h52_cleanup_sweep.ps1:167, 168, 170, 171, 172, 188, 189, 190, 193, 194, 207, 222, 223, 224, 225, 227, 229, 235, 292, 413, 415, 416, 417, 418, 419, 421, 423, 424, 425, 426, 427, 428, 429, 430, 431, 434, 435, 441, 444, 464, 480, 481 |
| `test_status_append.ps1` | none: record has 0 "not checked" | none |

## Counts per records file

"Not-checked records" is a script count of records whose text says "not checked" (includes a shared-section hit where the file has one; I give the file's own figure in brackets where a header or shared block inflates it). "Lines" is the number of table rows above that name the file.

| Records file | "Claim not checked" records | Lines written for them |
|---|---|---|
| E1 | 14 (test_agent_stop 7, test_common 1, test_percall_key 6); plus the shared note for 4 more test files | 6 rows (section 4) |
| E2 | 109 (R 10, P 12, H 6, progress 48, publish 33) | 5 rows |
| E3 | 165 (refusals 4, review_dispatch 32, ref_judge 19, review_rules 110) | 4 rows |
| E4 | 219 (ruling_rules 48, stall 35, shell_output 50, h53 44, h52 42) | 6 rows |
| delta-DL1 | 14 (15 counted with the shared header) | 7 rows (sections 1 to 3) |
| delta-DL2 | 15 | 6 rows |
| delta-DL3 | 29 (30 with a header) | 5 rows |
| delta-DL4 | 28 (29 with header) | 12 rows |
| delta-DL5 | 18 (19 with header) | 14 rows |
| delta-DL5b | 18 (19 with header; records 11 to 20 are "golden digest" with nothing to capture) | 9 rows |
| window-WN1 | 13 (W1 to W13) | 8 rows |
| window-WN2 | 21 | 8 rows |
| window-WN3 | 8 (script count; grep hit 14 lines) | 7 rows |
| window-WN4 | 45 (of 55 verdicts) | 12 rows |
| window-WN4a | 15 | 8 rows |
| window-WN4b | 32 (33 with a header; 30 C records) | 10 rows |
| window-WN4c | 6 | 5 rows |
| window-WN5 | 17 (of 19 counted) | 9 rows |
| window-WN6 | 15 | 8 rows |
| window-WN7 | 4 (records 1, 2, 3, 5 per script; record 4 says the same of Burnout's preconditions) | 4 rows |
| window-WN8 | 2 (a C record and a gaps section) | 2 rows |

Counts are approximate for the two-record WN8 and for shared-note files; the table rows merge requests for the same chart, so a record appears on several rows.
