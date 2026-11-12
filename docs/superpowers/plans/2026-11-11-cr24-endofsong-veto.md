# Hydra drops the "[section _ENDOFSONG]" section, as the game does (CR-24, 2026-11-11)

Task id PLAN-CR24. This plan changes no code. It says what to change, how to test it and what the user will see.

## The short version

Some .mid charts end with a text event that reads "[section _ENDOFSONG]". Clone Hero does not make a section from it. Hydra does, and shows a last section named " ENDOFSONG" (the underscore becomes a space). The user chose "Match the game".

The fix is small. Hydra's rule for "is this text a section" lives in one file, `src/parse/section_names.cpp`. That rule gains one test, for .mid files only: a `section ` text that holds _ENDOFSONG anywhere is not a section. The .chart reader keeps its rule, because the game's .chart reader has no such test. Saved results do not change, so the results stamp stays.

Two tasks do the work. T1 is the code and its tests. T2 is the Clone Hero page, and it runs after T1 because the page quotes T1's line numbers. A short scout task, T0, settles one fact first. The main session then looks at one real chart in the real app (T3).

## The game's rule, checked against the listing

Block CR-24 on `docs/clone-hero/01-reading-a-chart.md` (line 247) says: "A `[section` text whose untrimmed text contains _ENDOFSONG is not made a section; it falls through to the later tests. _ENDOFSONG has no branch of its own: that veto is all it does."

The listing agrees. I opened `docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm`. Lines 99 to 128 trim the text and test whether it starts with `[section`. Lines 129 to 136 then call String.Contains on the event's own text (the untrimmed one, read from the event object) with the literal _ENDOFSONG. A hit jumps to the same place a failed `[section` test jumps to, which is the `[prc` test. A miss goes on to cut the name and add the section. The block and the listing do not disagree.

Three details of the rule matter for the code, and each comes from the listing.

The test is "contains", not "equals". So "[section Outro_ENDOFSONG]" is vetoed too.

The veto sits only on the `[section` path. The listing holds one Contains call (line 134) and no other. A `[prc_...]` text never reaches it, so a `[prc` text holding _ENDOFSONG stays a section in the game.

The veto is in the .mid reader only. The game's .chart reader for an `[Events]` line is `docs/clone-hero/code/6142/events_section_2151830.asm`. Its list of text literals (lines 41 to 59) holds `section`, `prc`, `coda`, `end` and the lyric and phrase words, and no _ENDOFSONG. Its section path (lines 325 to 528) calls starts-with, a slice, the name table and the underscore swap, and no Contains. Both readers then call the same add-a-section method, RVA 0x212C350. I read that method in the decompile (lines 14559 to 14624 of `ghidra_decompile_all.txt`). It tests no text. So a .chart line `E "section _ENDOFSONG"` is a section in the game.

One detail is not proven: whether the game's Contains is case-sensitive. That is premise P9, and T0 checks it before T1 writes the compare.

## Hydra's code today

The rule lives in one place. `marker_name` in `src/parse/section_names.cpp` (line 13) says a text body is a section when it starts with `section ` or "prc_". `is_section_marker` (line 21) and `section_display_name` (line 25) both call it. Nothing in it looks for _ENDOFSONG.

Both chart readers reach it through one door, `section_name_of` in `src/parse/song.cpp` (line 34). The .mid reader calls the door at line 1185, after it has taken the square brackets off. The .chart reader calls it at line 1633, on the quoted text of an `E` line. So the two readers share one rule today, and the door does not know which reader is asking.

I searched for other copies. A grep of `src`, `tests` and `tools` for ENDOFSONG finds nothing. A grep of the same folders for `"section `, `[section` and `prc_` finds only `section_names.cpp`, its generated name table and the tests. The Python tools hold no copy of the section rule.

Three places read the finished section list (`Song::practice_sections`). `build_preview_scene` in `src/app/preview_view.cpp` (line 397) copies it into the preview scene. From the scene, the Preview tab's time box writes its "Section ..." line (line 592), and the path timeline draws its Section row (`src/ui/preview_timeline.cpp`, lines 271 to 274 and 452). The third reader is the `hydra_replay` tool, which writes the list as "sections" in its score output (`tools/replay.cpp`: lines 469 to 473 build the list and line 479 writes it). No code under `src/search`, `src/core` or `src/store` reads it.

## The change

The door learns which reader is asking. `section_names.h` gains a two-value type that names the reader, .mid or .chart. `is_section_marker` takes it. `section_name_of` in `song.cpp` takes it too, and each reader passes its own value. Inside `marker_name`, a body that starts with `section ` and comes from a .mid is not a section when it holds the veto text. The veto text is one named constant in `section_names.cpp`, and it appears nowhere else in `src` or `tools`.

One existing check pins the rule's lines. The single-owner scan row "What is a section marker's shown name?" in `tests/single_owner/rules/parse-fills.json` (lines 4 to 46) lists two owner lines of `marker_name` by their exact text: `if (body.rfind("section ", 0) == 0) return body.substr(8);` and the same for `prc_`. The scan fails if either line is gone or reworded. T1 keeps both lines as they are and puts the veto on its own line after them, or it updates the row's `owner_lines` in the same commit. The row's `note` says both readers go through `section_name_of`, which stays true.

Hydra tests the body between the brackets, where the game tests the whole text. The two give the same answer. Hydra only gets this far when the text starts with `[` and ends with `]`, and the veto text holds no bracket.

`ChartFormat` in `src/parse/chart_files.h` is not the right type for this. It has five values, and two of them (.sng and .srb) are containers, not readers.

## Does the results stamp change

No. The stamp rule in `src/store/stored_versions.h` says to bump `kResultsStamp` when "a fresh analysis would compute differently". The analysis never reads sections: no file under `src/search`, `src/core` or `src/store` names `practice_sections`. Sections are read fresh from the chart file each time a song is opened. So no saved row is wrong after this change, and nothing needs re-analysis. `kChartMetaStamp` covers the song id and the song.ini names, and this change touches neither.

## What the user will see

Two screens and one tool show section names. Each changes only for a .mid chart that holds a `[section ..._ENDOFSONG...]` text.

The numbers below are for "The Cure - Fascination Street" (the World of Rock chart). They come from `docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md` line 19, which reports them. I did not open that chart, because finding it means searching the user's library. So they are premise P10, marked unchecked, and T3 checks them in the real app. No chart under `testdata` holds the text: a byte search of the whole `testdata` folder for ENDOFSONG finds no file.

On the Preview tab, the time box has a "Section" line. Today, from 311,982 ms to the end of the song, it reads "Section  ENDOFSONG" (two spaces, because the name starts with one). After the change it keeps reading "Section Chorus Out", the section that began at 273,514 ms.

On the path timeline, the Section row today draws a last band labelled " ENDOFSONG" from 311,982 ms to the song's end. The "Chorus Out" band stops at 311,982 ms, where that band starts. After the change there is no such band, and "Chorus Out" runs to the song's end.

In `hydra_replay score`, the "sections" list today ends with an entry named " ENDOFSONG" at 311,982 ms. After the change that entry is gone.

Nothing else changes. Scores, paths, the library list and saved records do not read sections.

## Global constraints

The veto text is written once in Hydra's code. T1's acceptance check proves it.

A .chart file reads exactly as it does today. The game's .chart reader has no veto, so "Match the game" means no change there. T1 pins it with a test.

A `prc_` body is never vetoed. T1 pins it with a test.

CR-29 stays out. That block is the `[section` versus `section ` split (the game needs no space after the word, Hydra does). This change can avoid it and does: the veto is added behind Hydra's existing `section ` test, and that test is not touched. Question Q-15 stays open as it is (question 3 above).

Agents do not build and do not run `hydra_tests.exe`. The main session builds T1's branch and runs its tests.

## Questions

Answered 2026-11-11 (session 428cf079): the user wrote "go with recommended". This plan's default stands for all three: questions 1 and 2 stay out of this plan as open splits, and CR-29 stays with Q-15 and live step LS-U1. The executor of T2 records all three on the Clone Hero pages as open questions, not as rulings.

The user's ruling ("Match the game") covers the veto. These three splits are different: the work found them or touches them. This plan leaves each one out unless the user says otherwise, and the main session asks them before the merge. Each is the game's rule against Hydra's real rule, with a chart example from each side.

1. Two sections on one tick. The game's add-a-section method (RVA 0x212C350, decompile lines 14575 to 14585) looks at the last section it stored. If the new one has the same tick, it overwrites that section's name and adds nothing, so the last one written wins and there is one section. Hydra keeps both, in the file's order (`sort_practice_sections`, `src/parse/song.cpp` line 64). Example: a .mid with "[section Intro]" and "[section Verse 1]" both at tick 0. The game has one section, "Verse 1". Hydra has two: the Preview time box says "Section Verse 1" (it takes the last section at or before the playhead), the timeline draws a zero-length "Intro" band, and `hydra_replay score` lists both. Should Hydra match, so the later name replaces the earlier one on the same tick? Nobody has counted how many of the user's charts do this.
2. Text with spaces around it. The game trims an events-track text before it tests it. Hydra needs the first character to be `[` and the last to be `]`, with no trim (`src/parse/song.cpp` line 1180). That guard sits before the `[end]` test too. Example: the text "[section Intro] " (a space after the bracket) is a section in the game and is ignored by Hydra. "[end] " would likewise set the end event in the game and not in Hydra. Should Hydra trim first? Nobody has searched the library for such texts.
3. The `[section` versus `section ` split (CR-29, question Q-15 on `docs/clone-hero/questions.md`). The game tests the start `[section` with no space after the word. Hydra tests `section ` with the space. Example: "[section_intro]" (9 of the user's files, per `docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md` line 19) is not a section in Hydra. In the game it enters the section branch, and what name it cuts is unread. This plan does not touch it. Q-15 already holds the question, and live step LS-U1 settles it. Is it right to leave it there?

## Tasks

T1 runs after T0. T2 runs after T1, because the page quotes line numbers in `section_names.cpp` that T1 moves. Both merge together.

### T0: is the game's Contains case-sensitive (scout; Sonnet; first)

The game calls String.Contains at RVA 0x197F670 (listing line 134). The decompile on this machine does not hold that method. The scout disassembles it with `tools/ch_mcp/chdis.py` and reads which compare it uses. This settles premise P9.

Acceptance criteria. The scout's report quotes the lines that show the compare kind. If they show an ordinal compare (byte for byte, so case counts), T1 goes ahead as written. If they show anything else, or the scout cannot tell within 30 tool calls, it stops and reports, and T1 does not start until the main session has asked the user. The scout also hands T2 the listing lines, so the fact can go on the page.

### T1: the veto, in the one owner, with tests (C++; Opus; after T0)

The executor owns `src/parse/section_names.h`, `src/parse/section_names.cpp`, `src/parse/song.cpp`, `tests/test_section_names.cpp`, `tests/test_midi.cpp`, `tests/test_song_input.cpp`, `tests/test_song_parse.cpp` (its ".chart: [Events] section markers" case, line 234, is where criterion 5 goes) and `tests/single_owner/rules/parse-fills.json`. It makes the change described under "The change".

Acceptance criteria, each with its test. The .mid tests build their files with the `testmidi` helpers that `tests/test_midi.cpp` already uses (`smf_tracks`, `text_event`), and load them with `load_songbytes_mid`.

1. A .mid whose events track holds "[section Intro]" and later "[section _ENDOFSONG]" loads with one section, named "Intro". Today it loads with two, so this test fails on today's code.
2. A .mid that holds "[section Intro]" and "[section Outro_ENDOFSONG_x]" loads with one section. This proves the test is "contains". It fails on today's code, which gives two.
3. A .mid that holds "[section _endofsong]" (lower case) keeps that section, shown as " endofsong". This pins case, as T0 found it. It passes today; it guards against a case-blind compare.
4. A .mid that holds "[prc__ENDOFSONG]" keeps that section, shown as " ENDOFSONG". It passes today; it guards against a veto on the `prc_` path.
5. A .chart whose `[Events]` holds `E "section _ENDOFSONG"` keeps that section, shown as " ENDOFSONG". It passes today; it guards against a veto on the .chart reader.
6. On the song from criterion 1, the scene from `build_preview_scene` holds one section, and the Preview time box at a time after the vetoed marker reads "Section Intro". `tests/test_song_input.cpp` already reads the time box this way. Today the box reads "Section  ENDOFSONG" there, so this test fails on today's code.
7. The veto text is in one place. The executor's report shows the output of `git grep -c ENDOFSONG -- src tools`, and it is the single line `src/parse/section_names.cpp:1`. The reviewer runs the same command.
8. `kResultsStamp` and `kChartMetaStamp` are unchanged: `git diff main -- src/store/stored_versions.h` prints nothing.
9. The existing checks in `tests/test_section_names.cpp` still hold for both readers, with the new argument passed.
10. The single-owner scan still passes: the two `marker_name` owner lines in `parse-fills.json` still match a line of `section_names.cpp` verbatim.

The executor cannot run these tests. It hands the branch back, and the main session builds and runs `hydra_tests.exe` with the section-names, midi, song-input and song-parse cases, and `-tc="single-owner*"` for criterion 10.

### T2: the Clone Hero page takes the change (docs; Opus; after T1, same merge)

The executor owns `docs/clone-hero/01-reading-a-chart.md`, `docs/clone-hero/README.md`, `docs/clone-hero/questions.md` and `docs/agents/rulings/clone-hero.md`. CLAUDE.md wants the game fact on its page in the same merge as the code, so T2's branch merges with T1's.

Block CR-24 changes like this, ready to apply.

Its `Label:` line becomes "Proven in the game's code". The block was Unknown only because Hydra's change was planned and not made; its own evidence line says so.

Its `Rule:` line loses its last sentence about Hydra having no veto. In its place: "Hydra applies the same veto to a .mid `section ` text. The game's .chart reader has no such test, so a .chart `section _ENDOFSONG` line is a section in both. Hydra also tests `section ` with the space, where the game tests `[section` without one (CR-29)."

Its `Evidence:` line loses the sentence "The block stays Unknown because ...". It gains the .chart proof: `code/6142/events_section_2151830.asm` lines 41 to 59 (the literals, with no _ENDOFSONG) and lines 325 to 528 (the section path, with no Contains). It also gains T0's finding on case, with the lines T0 quotes.

Its `Hydra:` line becomes: "`marker_name` in src/parse/section_names.cpp holds the veto, for a .mid `section ` body only; `is_section_marker` asks it, and `section_name_of` in src/parse/song.cpp tells it which reader is asking." The executor fills in the line numbers from T1's finished branch. The coda sentence that follows stays. The sentence "A grep of src/ for `ENDOFSONG` finds nothing" goes.

Its `Ruling:` line becomes: "the user chose \"Match the game\" for the _ENDOFSONG veto (2026-11-08, session 77057df2; `docs/sessions/2026-11-08-77057df2.md`). Hydra matches since this change."

Its `Found:` line gains "; Hydra's veto added 2026-11-11, session 428cf079".

Acceptance criteria.

1. The block reads as above, with its seven lines in order. The docs check (`hydra_tests.exe "-tc=docs match code*"`, run by the main session) passes.
2. The line numbers that blocks CR-28 and CR-29, and question Q-15 on `questions.md`, give for `marker_name`, `is_section_marker` and `section_display_name` match T1's branch. The reviewer opens the file and checks each.
3. CR-24 stays in the page's "Rows waiting on a live run" list, and its row on `live-checks.md` is untouched. That row asks a different thing (whether these texts change the song length or score), and this change does not answer it.
4. The README's count table matches the page. Page 1 goes from 34 to 35 proven in the game's code and from 7 to 6 Unknown. The "All pages" row goes from 272 to 273 and from 47 to 46. The executor runs `docs/audit/2026-11-08-clone-hero-unknowns/main/recount_labels.py` and pastes its output in the report; the numbers match the table.
5. `docs/agents/rulings/clone-hero.md` gains one entry dated 2026-11-08 that quotes "Match the game" for the _ENDOFSONG veto and names the session card. That file holds no such entry today.
6. The page's "Last write-back" date is the merge day.

### T3: one real chart, in the real app (main session; before the merge)

The main session opens Hydra's real window on the snapshot copy of the library, built from the merged T1 branch. It reaches "Fascination Street" through the search box, never by scrolling. This settles premise P10.

Acceptance criteria. The Preview time box at the end of the song reads "Section Chorus Out", or whatever that chart's last real section is. The timeline's Section row holds no band named " ENDOFSONG". The main session notes the text it read in the merge report. If the chart is not in the library, or it never showed " ENDOFSONG" on today's build, the main session says so and tells the user before the merge; it does not pick another chart by scanning.

## Prior art

I searched the web for other tools that handle this text. Two searches for _ENDOFSONG with chart-parser words found no page that names it. I then opened YARG.Core's .mid reader (`MoonscraperChartParser/IO/Midi/MidReader.cs` on its master branch). It does not hold the text; it hands every events-track text to a section parser in another class, which I did not open. Moonscraper's own reader was not at the path I tried. So I found no other tool with this veto. The game's listing is the only source for the rule.

## Promise map

| Promise | Plan line | Task | The check that proves it |
|---|---|---|---|
| A .mid "[section _ENDOFSONG]" text is not a section | The change; T1 criterion 1 | T1 | The loader test with "Intro" and "_ENDOFSONG": one section. Fails today with two |
| The test is "contains", anywhere in the text | The game's rule; T1 criterion 2 | T1 | The loader test with "Outro_ENDOFSONG_x": one section. Fails today |
| Case counts, as T0 finds it | T0; T1 criterion 3 | T0, T1 | T0's quoted lines; the lower-case test keeps its section |
| A `prc_` body is never vetoed | Global constraints; T1 criterion 4 | T1 | The "[prc__ENDOFSONG]" test keeps its section |
| A .chart file reads as it does today | Global constraints; T1 criterion 5 | T1 | The .chart `E "section _ENDOFSONG"` test keeps its section |
| The Preview time box no longer shows " ENDOFSONG" | What the user will see; T1 criterion 6 | T1 | The time-box test reads "Section Intro". Fails today |
| The timeline and `hydra_replay` lose the entry | What the user will see | T1, T3 | Both read `Song::practice_sections`, which criterion 1 pins; T3 reads the timeline row in the real app |
| The veto text is written once | Global constraints; T1 criterion 7 | T1 | `git grep -c ENDOFSONG -- src tools` prints one line with count 1 |
| The stamps do not change | Does the results stamp change; T1 criterion 8 | T1 | `git diff main -- src/store/stored_versions.h` prints nothing |
| CR-29 is not touched | Global constraints; T1 criterion 9 | T1 | The existing `is_section_marker` checks for "sectionIntro" still hold |
| The single-owner scan still passes | The change; T1 criterion 10 | T1 | `-tc="single-owner*"`, run by the main session; it fails if `marker_name`'s two pinned lines are reworded |
| T1 waits for T0, and stops if the compare is not ordinal | T0 | T0 | T0's report quotes the compare lines, or says it stopped |
| T2 runs after T1 and quotes its line numbers | T2 | T2 | T2 criterion 2 |
| The three splits are asked before the merge | Questions | Main session | The user's answer to each |
| The page takes the change in the same merge | T2 | T2 | T2 criterion 1, the docs check |
| Line numbers on the page match the code | T2 criterion 2 | T2 | The reviewer opens `section_names.cpp` at each cited line |
| The live-check row for CR-24 stays | T2 criterion 3 | T2 | `git diff main -- docs/clone-hero/live-checks.md` prints nothing |
| The README counts match the page | T2 criterion 4 | T2 | The pasted output of `recount_labels.py` |
| The ruling is in the rulings folder | T2 criterion 5 | T2 | The reviewer reads the new entry against the session card |
| The page's write-back date is the merge day | T2 criterion 6 | T2 | The reviewer reads line 5 of the page |
| A real chart is checked in the real app before the merge | T3 | T3 | The text the main session read, in the merge report |
| No library scan | T3 | T3 | T3 reaches the song by search, and stops if it is missing |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: the game's .mid events reader skips the section step when a `[section` text contains _ENDOFSONG | code | `docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm` lines 99 to 136 | checked 2026-11-11, read the starts-with test, the Contains call and both jumps |
| P2: that listing holds one Contains call, so the `[prc` path has no veto | code | `docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm` line 134, by a search of all 401 lines | checked 2026-11-11 |
| P3: the game's .chart `[Events]` reader has no _ENDOFSONG literal and no Contains call | code | `docs/clone-hero/code/6142/events_section_2151830.asm` lines 41 to 59 and 325 to 528, and a search of all 868 lines | checked 2026-11-11 |
| P4: the add-a-section method both readers call tests no text | code | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\ghidra\ghidra_decompile_all.txt` lines 14559 to 14624 | checked 2026-11-11 |
| P5: Hydra's section rule lives only in `marker_name`, and both readers reach it through `section_name_of` | code | `src/parse/section_names.cpp` lines 13 to 26; `src/parse/song.cpp` lines 34, 1185 and 1633 | checked 2026-11-11, with greps of src, tests and tools |
| P6: only the preview scene and `hydra_replay` read the section list | code | `src/app/preview_view.cpp` lines 397 and 592; `src/ui/preview_timeline.cpp` lines 271 and 452; `tools/replay.cpp` lines 469 and 479 | checked 2026-11-11, by a grep for `practice_sections` |
| P7: the results stamp is bumped only when a fresh analysis would compute differently | code | `src/store/stored_versions.h`, the comment above `kResultsStamp` | checked 2026-11-11 |
| P8: the user chose "Match the game" for the _ENDOFSONG veto | ruling | `docs/sessions/2026-11-08-77057df2.md` line 41 | checked 2026-11-11 |
| P9: the game's Contains is case-sensitive | unchecked | `docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm` line 134 names the call but not its compare; the decompile holds no function at RVA 0x197F670 | T0 |
| P10: "Fascination Street" shows " ENDOFSONG" at 311,982 ms after "Chorus Out" at 273,514 ms | unchecked | `docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md` line 19 only reports it | T3 |
| P11: no chart under testdata holds the text | chart | `testdata`, by a byte search of every file for ENDOFSONG | checked 2026-11-11, no file found |
| P12: the name "_ENDOFSONG" is not in the game's name table, so it shows as " ENDOFSONG" | data | `data/clone-hero-1.1.0.6142-section-names.tsv`, a count of ENDOFSONG gives 0 | checked 2026-11-11 |
| P13: the tests can build a .mid with text events in memory | code | `tests/test_midi.cpp` lines 625 to 641 | checked 2026-11-11 |
| P14: `docs/agents/rulings/clone-hero.md` holds no entry for this ruling | code | `docs/agents/rulings/clone-hero.md`, a grep for ENDOFSONG and CR-24 finds nothing | checked 2026-11-11 |
| P15: page 1 counts 34 proven and 7 Unknown today | code | `docs/clone-hero/README.md` lines 55 to 64 | checked 2026-11-11 |
| P16: the single-owner scan row for `marker_name` pins its two prefix lines by exact text | code | `tests/single_owner/rules/parse-fills.json` lines 4 to 46 | checked 2026-11-11 |
| P17: the game's add-a-section method overwrites the last section's name when the tick is equal | code | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\ghidra\ghidra_decompile_all.txt` lines 14575 to 14585 | checked 2026-11-11, read as the plan's author reads it; question 1 asks the user |
| P18: Hydra's .mid reader keeps a text only when it starts with `[` and ends with `]`, with no trim | code | `src/parse/song.cpp` line 1180 | checked 2026-11-11 |
| P19: the Preview time box takes the last section at or before the playhead; the timeline draws a band from each section to the next | code | `src/app/preview_view.cpp` lines 589 to 592; `src/ui/preview_timeline.cpp` lines 271 to 274 | checked 2026-11-11 |
