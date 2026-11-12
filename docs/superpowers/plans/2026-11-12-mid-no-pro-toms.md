# A drum chart with neither track flag up is all toms

Plan PL-MID-NO-PRO, written 2026-11-12 in session c61c1986. Nothing here starts until the user says yes (agent rule 3).

## What this is about

Clone Hero keeps two yes-or-no flags on every drum track. The first says "this chart has a fifth pad". The second says "this chart has Pro Drums marks" (a mark says whether a pad is a tom or a cymbal). When both are down, the game ends its load step by making every pad a tom. Only the kick is left alone.

Hydra does not do this. It keeps the reader's own cymbals, so on such a chart it pays yellow, blue and green as cymbals (65 points) where the game pays toms (50 points). The user said "Yes, plan it" to making Hydra follow the game here.

## The rule, in the game's terms

The step is the track method at 0x215D450. It has two branches. The five-lane branch returns as soon as its pass ends. The four-lane branch goes on to a tail. The tail tests the fifth-pad flag (+0xA1) and then the Pro flag (+0xA2). If either is up, it returns. If both are down, it walks every note, asks one question of it (the call to 0x214C8A0), and for each note that answers no it clears the cymbal bit and sets the tom bit. Probe F shows the notes that answer yes are the kicks: all 14 pads paid 50 and the total matched.

Where the two flags start depends on the file format.

A .mid starts the fifth-pad flag from song.ini's `five_lane_drums` and the Pro flag from song.ini's `pro_drums`. The track can then only raise them: a fifth-pad note raises the first, and a tom marker (pitch 110, 111 or 112) raises the second.

A .chart starts both at zero whatever song.ini says. N 5 raises the first and N 66, 67 or 68 raises the second.

Both starting rules come from Hydra's own comments on the game's listings; the lines were not opened for this plan (premise P6, which T1 settles).

The branch test at the top of the method (lines 26 to 37) sends a chart to the five-lane branch when the fifth-pad flag is up and the Pro flag is down, or when song.ini's `five_lane_drums` reads true. Every other chart takes the four-lane branch (premise P32).

So a .mid reaches the tail only when it has no fifth-pad note, no tom marker, and neither key reads as true. A .chart reaches it when it has no N 5 and no N 66 to 68 and no `five_lane_drums` key (that key picks the five-lane branch on its own).

### How the game reads the two keys as yes or no

The game's getter at 0x1D4480 is short and its rule is plain (premise P7; T1 commits its listing).

It asks the ini object for the key's text. A missing key gives the default, which the caller sets to false for both keys. If the text parses as a whole number (Int32.TryParse), the answer is "the number is not zero". Otherwise, if the text equals "true" with case ignored, the answer is true. Otherwise, if it equals "false" with case ignored, the answer is false. Anything else gives the default, false.

The number test is an unsigned compare against zero (`seta`, at 0x1D4566 to 0x1D456B in the listing), so a negative number such as `-1` is also true. Which blanks and signs Int32.TryParse accepts is .NET's rule, not read in the listing: its default style accepts blanks around the digits and a leading plus or minus sign, and refuses a decimal point or a value past 32 bits (premise P31, which T1 reads). Blanks around a number therefore change nothing, and Gap 2 below is only about the words true and false.

So `pro_drums = 1` is true in the game, and `pro_drums = yes` is false. Hydra's `drum_key_is_true` takes only "true", so today it reads `pro_drums = 1` as unset. That matters here: 14 library charts write `pro_drums = 1`, and without this fix the new tail would wrongly turn them all to toms.

### Gaps, each with the step that closes it

Gap 1: no run has played a .mid whose only raised flag comes from `pro_drums`. The getter read says `pro_drums = 1` keeps the cymbals. LS1 plays it.

Gap 2: trailing blanks. `pro_drums = True` followed by a space is honoured by Hydra, which trims the value. Whether the game's ini object trims a value before the getter sees it was not read. T1 reads the ini text lookup (0x1BD150) and the ini parser behind it, capped at 30 calls. If T1 cannot answer, LS2 plays it. Until one of them answers, Hydra keeps trimming, as it does today (premise P11).

Gap 3: a disco section on a chart that reaches the tail. The game's code says the disco flag is put on red and yellow notes before the tail, and the flip itself happens later, in the builder's Pro Drums step, by lane. So a red disco note should still become a yellow cymbal. Hydra's order is the same: the conversion, then the flip. No run has seen it. LS3 plays it.

Gap 4: the listing of the getter is in a scratch folder today. T1 commits it under `docs/clone-hero/code/6142/` with its note, so the pages can cite it.

## Where Hydra changes

**One home for the tail.** The tail goes inside `convert_drum_pads` in `src/parse/song.cpp`, at the end of its four-lane branch. That function is already the one place a chart's raw pads become the chord's four lanes, and both readers call it. When the chart took the four-lane branch and both `DrumChartFlags::fifth_pad` and `DrumChartFlags::pro_marks` are down, every pad in the chord becomes a tom. The comment "Left out: the all-toms tail" goes away. No second owner is added.

**The Pro Drums toggle and `cymbal_counts`.** Nothing changes in either. The .mid reader still marks each pad from its tom markers through `cymbal_counts`, and the conversion then clears those marks on a tail chart. With Pro Drums off there were no cymbals to clear. With Pro Drums on, a tail chart now reads the same as with it off, except for the disco flip, which stays behind the Pro Drums gate in `emit_chord_timestamp` and still runs after the conversion (Gap 3).

**The key reader.** `drum_key_is_true` takes the game's getter rule: a whole number that is not zero, or "true" in any case. It stays the one reader of both keys, so `five_lane_drums = 1` also starts to count. That second effect is question 1.

**.chart.** Nothing a .chart reads changes. Hydra's .chart reader already makes a pad a tom unless N 66 to 68 marks it, so a .chart with both flags down is already all toms. A test pins that.

**.sng.** A .sng carries its song.ini keys in its metadata block, and `load_container_sng` already hands them to the same readers. So a .mid inside a .sng follows the same rule with no new code. The game's .sng reader is known to read both keys, but its lines were not read one by one (premise P13).

**.srb.** An .srb holds no song.ini keys, so `load_container_srb` reads both as unset. A .mid inside an .srb with no tom marker and no fifth pad will therefore become all toms. That follows from the one rule, and no special case is planned. Whether Clone Hero opens an .srb at all was not checked, so this is question 3.

**The results stamp.** `kResultsStamp` in `src/store/stored_versions.h` is "2.14.1+fivelane". Release 2.14.1 has not shipped (the newest tag is v2.14.0). The header says a chart-reader change that makes a chart read differently needs a bump, and that changes in one release share one bump. D337 item 3 moved the suffix inside the unshipped 2.14.1 once already, from "+coda" to "+fivelane", because builds from main had saved rows (the commit that did it, `e6f3b078`, labels it D336 by mistake; D336 is the startup library bar). The same is true now for any row a main build saved since the five-lane merge. Whether to move it again is question 2.

## What the user will see (agent rule 11)

Every figure below is with Pro Drums on, Expert. With Pro Drums off nothing changes on these two charts.

| Chart | Figure | Before | After | Source |
|---|---|---|---|---|
| Probe F | total, no activations | 6,470 | 5,750 | `hydra_replay score`, run by the main session today; the game scored 5,750 |
| Probe F | Notes / Combo Bonus | 2,330 / 4,140 | 2,150 / 3,600 | worked out from 50 and 65 per note; the game's file shows 2,150 and 3,600 |
| Fortunate Son | total, no activations | 239,905 | 205,150 | `hydra_replay score` with Pro on and Pro off, run by the main session today |
| Fortunate Son | Notes / Combo Bonus | 60,820 / 179,085 | 52,000 / 153,150 | same runs |
| Fortunate Son | cymbal notes | 588 | 0 | same runs |

The before figures for probe F's parts are sums, not a Hydra run. The chart (premise P5) has 29 kicks, then 2 red, 3 yellow, 4 blue and 5 green notes, one note per tick. 29 kicks and 2 red at 50, and 12 yellow, blue and green notes at 65, give 2,330. A note's combo share is its points times the multiplier minus one (`docs/clone-hero/06-scoring.md` SC-05 and SC-08). The multiplier steps up at the 10th, 20th and 30th note, so the 29 kicks earn 10 x 50 x 1 + 10 x 50 x 2 = 1,500 and all 14 pads sit at 4x, where each note earns 3 times its points: 3 x (2 x 50 + 12 x 65) = 3 x 880 = 2,640. 1,500 + 2,640 = 4,140. With every pad at 50 the notes are 2,150 and the combo is 1,500 + 14 x 150 = 3,600, the figures in the game's file.

These screens read the changed values. Their exact text needs a build, so MS4 captures each one before and after and shows the user before merge.

The Library row shows the chart's best score. The Paths tab shows each path's total, the score breakdown, and each activation's chord by name, such as "[Kick - Green cymbal]", which becomes a tom's name. The Preview draws the notes, so 588 cymbals on Fortunate Son are drawn as toms, and its next-activation box names the chord the same way. The Dynamics tab lists cymbal rows under Pro Drums, and those rows empty out for these charts. The best path itself may change, because cymbals no longer pay more. Every saved result reads Stale once if question 2 is answered yes.

The best-path totals, the path strings and the on-screen text for both charts are not known yet. They need a build (MS3 and MS4).

## How many charts change

The loose-file scan of today read 15,019 .mid drum charts. 21 have pads, no tom marker, no fifth pad and no key equal to "true". One is probe F. Six have no `pro_drums` key, all from Guitar Hero: Warriors of Rock; these six change. Fourteen write `pro_drums = 1`; by the getter read these do not change, and LS1 checks that in the game.

Charts inside .sng files were not scanned. Counting them is not needed to build the rule, because the code path is the same. It is needed to tell the user how many library charts change, and to answer question 1 with a real count of `five_lane_drums` written as a number. MS5 is that scan, and it runs only with the user's yes (question 4).

## Questions for the user

Each is asked before any build starts. None is answered by silence.

1. **Should `five_lane_drums` follow the game's yes-or-no rule too?** The game's real rule, for both keys: the value is true when it parses as a whole number that is not zero, or when it is "true" in any letter case; "false", zero, a missing key and any other text are false. One end: `five_lane_drums = True`. The game and Hydra both say true today. Other end: `five_lane_drums = 1`. The game says true, and sends the chart down the five-lane branch (yellow and the fourth pad become cymbals, blue a tom). Hydra says unset today. Hydra's real predicate today is `drum_key_is_true`: `equals_ci(trim_view(value), "true")`, at `src/parse/song.cpp` line 2250, so only the word true, in any case, with blanks around it. Today's screens for such a chart read the four-lane branch's toms and cymbals; after the change they read the five-lane branch's (the same difference NL-74 to NL-76 describe), and the score changes with them. For `pro_drums` the fix is needed for the tail to be right, so it is in the plan either way. For `five_lane_drums` it is a visible change your words did not ask for. Recommended: yes, one reader for both keys, as in the game. How many library charts write `five_lane_drums` as a number is not counted; MS5 counts it if you allow the scan.

2. **Should every saved result read Stale once more?** The rule in `stored_versions.h`: bump the stamp when a chart reads differently, and changes in one release share one bump. One end: move "2.14.1+fivelane" to "2.14.1+alltoms". Every saved result reads Stale once and asks to be analysed again, as it did after the five-lane merge. Other end: keep the stamp. Nothing reads Stale, but a result saved by a main build since the five-lane merge for one of the changed charts (the six Warriors of Rock charts and any .sng chart like them) keeps its old, too-high score until that chart is analysed again by hand. Recommended: move it, as D337 item 3 did.

3. **Should a .mid inside an .srb follow the same rule?** The real predicate in Hydra: an .srb has no song.ini keys, so its .mid reaches the tail whenever it has no tom marker and no fifth pad. One end: an .srb whose .mid has tom markers. Nothing changes. Other end: an .srb whose .mid has none. Its yellow, blue and green notes become toms, and its score drops by 15 points per such note before the multiplier. Whether Clone Hero opens an .srb was not checked, so there may be no game result to match. Recommended: yes, one rule and no special case.

4. **May the main session scan the library's .sng and loose charts once, read-only?** It reads each chart's keys and drum track and counts the charts that reach the tail, and the charts that write either key as a number. It runs no analysis and changes nothing. It is a whole-library read, so it needs your yes. Recommended: yes.

## Global constraints

Agents never build and never run `hydra_tests.exe`, `hydra_uitest.exe` or `hydra_replay.exe`. The main session builds and runs every test that needs a build. No whole-library run happens without the user's yes in chat. No new number is introduced: 50 and 65 are the game's note values on the scoring page, and nothing else is tuned. A test pins a literal from one run or calls the production function; it never works the expected value out again.

## The tasks

T1, T2, T3 and T4 are independent and launch together. Each is one Opus executor, under 100 calls and 20 minutes, in its own worktree cut from main. T5 and T6 come after the main-session steps that feed them.

### T1: Commit the getter listing and read the trimming rule

Owned files: `docs/clone-hero/code/6142/ini_bool_getter_1D4480.asm` and its `.md`, any new listing it makes for the ini text lookup, Int32.TryParse or 0x214C8A0, each with its `.md`, the index rows in `docs/clone-hero/code/README.md`, `docs/clone-hero/code/facts-6142.tsv`, and the one sentence of `docs/clone-hero/code/6142/five_lane_convert_215D450.md` that cites `getters.asm` for 0x214C8A0 (corrected if the new listing shows the citation is wrong).

The executor makes the listing with `py -m tools.ch_mcp.chdis 1D4480 600` and writes the note beside it. Then it reads the text lookup at 0x1BD150, and whatever fills the ini object, to learn whether a value's trailing blanks are cut before the getter sees them. It refreshes the facts table with `tools/ch_mcp/dump_facts.py --tsv`. No build is needed.

Stop condition: 30 calls on the trimming read. If it is not answered by then, the report says "trimming not found", names where it looked, and LS2 is used instead.

It also opens three places this plan leans on but did not read: the .mid drum reader's lines 80 to 99 and the .chart loader's lines 158 to 214 (where the flags start, premise P6), and the .sng metadata reader's lines for the two keys (premise P13). Its report says, with listing lines, whether each matches what this plan says. It also lists Int32.TryParse (RVA 0x1B4F710, the call at 0x1D44F9) to read which blanks and signs the game's number test accepts (premise P31), and the tail's one question at 0x214C8A0, which `five_lane_convert_215D450.md` calls the kick test but which is not in the `getters.asm` it cites (premise P3). These two reads share the 30-call cap with the trimming read; if one is not answered, the report says so and the plan's wording stays marked unchecked.

Acceptance: the report confirms or contradicts P3, P6, P13 and P31 with listing lines. The committed listing shows the Int32.TryParse call, the two String.Equals calls with comparison type 5 and the three return paths. The docs check `hydra_tests.exe "-tc=docs match code*"` passes on the branch (main session runs it). The report states the trimming rule with listing lines, or states that it was not found.

### T2: The tail and the key reader, tests first

Owned files: `src/parse/song.cpp`, `src/parse/song.h` (comments only, if a comment there goes stale) and `tests/test_five_lane.cpp`.

The executor writes the tests first and commits them alone, so the main session can run them red (MS1). Then it adds the tail to `convert_drum_pads` and the getter rule to `drum_key_is_true`, and fixes the comments that say the tail is left out or the getter is unread. Before it commits, it lists each question the change answers and greps for another owner, as the preamble asks.

Acceptance, each a test in `tests/test_five_lane.cpp` that fails on main today unless marked "holds today":

1. A .mid with pads, no tom marker, no fifth pad and no keys, read with Pro Drums on, has no cymbal on any note.
2. The same .mid with one tom marker anywhere on the track keeps its cymbals (holds today; fails if the tail ignores the Pro flag).
3. The same .mid with a fifth-pad note on Easy only keeps the five-lane branch's cymbals (holds today; fails if the tail ignores the fifth-pad flag).
4. The same .mid with `five_lane_drums = True` keeps the five-lane branch's cymbals (holds today; fails if the tail runs after the five-lane branch).
5. The same .mid with `pro_drums = True` keeps its cymbals (holds today), and with `pro_drums = 1` keeps them too (both hold today, because main has no tail; they fail if the tail is added without the key fix, and test 6's "1" row is the red proof of the key read).
6. The key reader, tested through `chart_drum_keys`: "1", "2", "-1", "true", "TRUE" and "True" read as set; "0", "false", "yes", "" and a missing key read as unset. The getter's listing gives these rows. Four more rows follow .NET's number rule (premise P31): "+1" and "1 " read as set, "1.0" and "99999999999" read as unset; if T1 finds the rule different, a fresh executor changes those four rows. One row per text, so a broken row names its text.
7. A .chart with pads, no N 5 and no N 66 to 68 parses to the same chords before and after (holds today; pinned by its digest).
8. A .mid inside a .sng built in the test, with no keys in its metadata, has no cymbal; the same .sng with `pro_drums = 1` in its metadata keeps them.
9. A .mid inside an .srb built in the test, with no tom marker, has no cymbal (only if question 3 is yes; if no, the test pins that it keeps them and T2 adds the exception at the .srb loader).
10. A tail .mid with a disco section over a red and a yellow note, Pro Drums on: the red note reads as a yellow cymbal and the yellow note as red (holds today; fails if the tail is put after the flip).
11. A tail .mid read with Pro Drums off gives the same chords as before the change (holds today).
12. Probe F's notes, built in the test, score 5,750 with Pro Drums on through the production scoring call the probe C and E test already uses. The literal 5,750 is the game's own total.

### T3: The results stamp (only if question 2 is yes)

Owned files: `src/store/stored_versions.h` and `tests/test_store_records.cpp`.

The stamp moves to "2.14.1+alltoms" and its accepted list holds the new stamp alone. The test that names the stamp (`tests/test_store_records.cpp` lines 601 to 613) is updated, and the status table at line 630 changes: its "2.14.1+fivelane" row becomes Stale and a "2.14.1+alltoms" row reads Ready.

Acceptance: `test_store_records.cpp` holds a check that `kResultsStamp.is_current("2.14.1+fivelane")` is false and that the written stamp is "2.14.1+alltoms". If question 2 is no, T3 is dropped and the Promise map row moves to Out of scope.

### T4: The probe makers, the corpus scout and the library scan script

Owned files: a new folder `docs/audit/2026-11-12-mid-no-pro/`.

The executor writes three small Python scripts there. The first makes the probe charts: G (probe F's notes with `pro_drums = 1`), H (`pro_drums = True` with a trailing blank) and J (probe F's notes with a disco section over the two red and three yellow notes). It starts from `C:\Users\Patrick\AppData\Local\Temp\claude\73923404e4ab348b\c61c1986-81b3-4539-9926-15e8e54d6782\scratchpad\make_probe_f.py`, which it copies in because scratch files are not kept. It takes the output folder as an argument and refuses a folder that exists, as probe F's maker does. The executor runs it into its own scratch folder to print each probe's song.ini; it does not write into `C:\Clone Hero\songs`, which it does not own. The main session runs the maker into `C:\Clone Hero\songs\Hydra Probe\` before each live step. The second script lists the charts under `testdata/` that reach the tail, and the tests under `tests/` that build a four-pad .mid or .chart in code with no tom marker and no fifth pad (by test name), so the main session knows which pinned test values will move. The executor runs it, because it reads only the checked-in files, and commits its output. The third is the library scan for MS5: it reads loose charts and .sng files, and counts the charts that reach the tail and the charts that write either key as a number. The executor does not run the third script.

Acceptance: the list of corpus charts and in-test builders is committed with a count on its first line. Each probe's song.ini is printed in the report. The library script refuses to run without an explicit folder argument, and the report says so with the line that enforces it.

### MS1: Build T2's test commit and run it red

The main session builds T2's first commit and runs `hydra_tests.exe "-tc=five-lane*"`. Tests 1, 6 (its rows "1", "2", "-1", "+1" and "1 "), 8 (its first half), 9 (if question 3 is yes) and 12 must fail, and the exact red lines go to the reviewer. A test in that list that passes is reported, not explained away.

### MS2: Build the whole change, run green, and collect the moved values

The main session builds the joined branch, gets the test list from `hydra_precheck.exe --tests`, and runs it. Any pinned value that moved because a corpus chart or an in-test chart reaches the tail (T4's list) is collected with its test name and its new value. Cap: one build and one run. A failure that is not on T4's list is a real failure and goes to a fresh executor.

### T6: Re-pin the moved corpus values

Owned files: the test files MS2 names, and nothing else.

A fresh executor replaces each moved literal with the value from MS2's run and names the corpus chart in its commit message. It changes no value for a chart that is not on T4's list.

Acceptance: every changed literal is matched in the commit message to a chart on T4's list. The main session's second run of the same test list passes.

### MS3: Score the charts with the built branch

The main session runs `hydra_replay score` on probe F, on probes G and J once their live steps have run, and on Fortunate Son, with Pro Drums on and off. Pass lines: probe F 5,750 with Pro on; Fortunate Son 205,150 with Pro on, equal to its Pro off total; probe G equal to the game's LS1 total; probe J equal to the game's LS3 total. It also runs the same command on main for the before figures (probe F 6,470, Fortunate Son 239,905), which settles premises P15 and P16.

### MS4: Capture every screen, before and after

The main session opens Fortunate Son and probe F in Hydra built from main and from the branch, with `hydra_uitest` or the `hydra` MCP server, and copies the text of the Library row, the Paths tab, the Preview's next-activation box and the Dynamics tab. The before half needs only the build of main that already exists, so the main session may capture it before the user answers the questions, and show it with them (agent rule 11). It shows the user the before and after text. The merge waits for the user's yes on what they see (agent rules 11 and 12). This step settles premise P17.

### MS5: Count the library charts (only with the user's yes to question 4)

The main session runs T4's third script once on `C:\Clone Hero\songs`. Cap: one run, 30 minutes; if it has not finished, stop it and report how far it got. The counts go to the user and to T5.

### T5: Write the facts back to the reference pages

Owned files: `docs/clone-hero/01-reading-a-chart.md`, `docs/clone-hero/02-note-list.md`, `docs/clone-hero/questions.md`, `docs/clone-hero/live-checks.md`, `docs/clone-hero/README.md`, and the decision-log entry the main session numbers, which quotes the user's "Yes, plan it" and their answers to the four questions in their exact words (premise P29).

It runs after the live steps and MS3, on the same branch, so the facts land in the same merge as the code. NL-74 takes probe F's run (5,750, with its score file) as the tail's live proof, and its Hydra line stops saying the tail is left out. CR-18 takes the `pro_drums` result from LS1. A new block on page 1, with the next free CR number, holds the getter's rule, citing T1's listing, and LS2's result if it ran. NL-22 takes LS3's result. Q-13's "still not copied" paragraph is rewritten to say the tail is built. The scan count (21 of 15,019 loose .mid charts, six without the key, fourteen with `pro_drums = 1`) goes on NL-74 with the scan file's path, and MS5's counts beside it if it ran. The README's block counts and write-back date are refreshed with the recount script it names.

Acceptance: the docs check passes on the branch. Each of the four facts (probe F, the scan count, the getter rule, the `pro_drums` run) is found by grep on its page, and the report quotes the line.

### MS6: Review, merge, and the plan leaves the repo

One fresh Sonnet reviewer reads the branch, fixes what it finds and signs off. The main session merges. This plan file is removed in that merge, and the docs check proves nothing still points at it.

## The live steps

Each run is a bot run on Clone Hero v1.1.0.6142-final, ProDrums Expert, about 2 minutes, because the probe audio is 125 seconds long whatever `song_length` says. Each step waits for the user's go, and the go request shows the step's call list. Cap for every step: two runs. If the second run also gives no score file, stop and report the tool's message.

### LS1: Probe G, a .mid with `pro_drums = 1` and no tom marker

Calls: `ch_start` (never tried), `ch_wait_stats` (seen: probe F run, 2026-11-12 13:11), `ch_stop` (seen: probe F run, 2026-11-12 13:11)

`ch_start` is called with probe G's folder and `player="ProDrums,Expert"`. It ran this way on probe F today, but not on this chart. Pass line: 6,470 (note 2,330, combo 4,140, 43 of 43) means the key raised the Pro flag and the cymbals were kept, as the getter read says. 5,750 means the game did not take `1` as true, and the plan stops for a new reading before T2 merges. This step settles premise P10 and Gap 1.

### LS2: Probe H, `pro_drums = True` with a trailing blank (only if T1 does not find the trimming rule)

Calls: `ch_start` (never tried), `ch_wait_stats` (seen: probe F run, 2026-11-12 13:11), `ch_stop` (seen: probe F run, 2026-11-12 13:11)

Pass line: 6,470 means the game trims and Hydra's trim stays. 5,750 means it does not, and `drum_key_is_true` stops trimming trailing blanks, with a test row for it added to T2's test 6 by a fresh executor. This step settles premise P11.

### LS3: Probe J, a disco section on a chart that reaches the tail

Calls: `ch_start` (never tried), `ch_wait_stats` (seen: probe F run, 2026-11-12 13:11), `ch_stop` (seen: probe F run, 2026-11-12 13:11)

Pass line: 5,870 (note 2,180, combo 3,690; each of the two red notes that flips to a yellow cymbal adds 15 to the notes and 3 x 15 = 45 to the combo at 4x, premise P33, so 5,750 + 2 x 60) means the two red notes flipped to yellow cymbals, as the code says, and Hydra's order is right. 5,750 means no cymbal survives the tail, and the flip on a tail chart becomes a question for the user before merge. Any other total stops the plan. This step settles premise P12 and Gap 3.

## Out of scope, for the user to approve

The FiveLaneDrums kit player stays out, as D337 left it. Build 1.0 is not checked: its dump holds no code. The song cache's reader of the two flags (CR-18's Unknown part) is not touched. Whether Clone Hero opens an .srb is not researched; question 3 decides Hydra's side.

## Promise map

| Promise | Plan line | Task | The check that proves it |
|---|---|---|---|
| A chart with both flags down is all toms | "One home for the tail" | T2 | tests 1 and 12; MS1 runs them red; MS3 probe F 5,750 |
| The tail runs only when both flags are down | "The rule, in the game's terms" | T2 | tests 2, 3 and 4, one per flag and one for the key |
| The tail has one home, in `convert_drum_pads` | "One home for the tail" | T2 | tests 7 and 8 reach it through both readers and the .sng loader; the reviewer greps for a second owner |
| `pro_drums = 1` counts as set, by the game's getter rule | "The key reader" | T2, LS1 | tests 5 and 6; LS1's 6,470 |
| The getter rule is the game's, text by text | "How the game reads the two keys" | T1, T2 | T1's committed listing; test 6's rows |
| Trailing blanks follow the game | Gap 2 | T1 or LS2 | T1's report with listing lines, or LS2's total |
| Nothing a .chart reads changes | ".chart" | T2 | test 7 |
| A .sng follows the same rule | ".sng" | T2 | test 8 |
| An .srb follows the user's answer | ".srb", question 3 | T2 | test 9 |
| Pro Drums off reads as before | "The Pro Drums toggle" | T2 | test 11 |
| The disco flip still runs after the conversion | Gap 3 | T2, LS3 | test 10; LS3's 5,870 |
| No whole-library run without the user's yes | "Global constraints", MS5 | T4, MS5 | T4's script refuses without a folder argument; MS5 runs only after question 4 |
| Saved results read Stale once, if the user says so | "The results stamp" | T3 | the stamp check in `test_store_records.cpp` |
| The user sees every changed screen before merge | "What the user will see" | MS4 | MS4's before and after text, and the user's yes |
| Fortunate Son scores 205,150 with Pro on | rule 11 table | MS3 | MS3's pass line |
| Moved corpus values are re-pinned only for tail charts | T6 | T4, T6 | T6's commit message matched to T4's list; MS2's second run |
| Probe F, the scan count and the getter rule reach the pages in the same merge | T5 | T5 | grep lines in T5's report; the docs check |
| Agents never build | "Global constraints" | every task | each brief names no build; the build hook refuses |
| Every live step stops after two failed runs | "The live steps" | LS1 to LS3 | the cap line in each go request |
| The plan leaves the repo when its work merges | MS6 | MS6 | the docs check |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The tail sits after the four-lane pass, tests +0xA1 then +0xA2, and for each note that answers no to 0x214C8A0 clears bit 0x20 and sets bit 0x10. | code | `docs/clone-hero/code/6142/five_lane_convert_215D450.asm` lines 137 and 187 to 222 | checked 2026-11-12 |
| P2: The five-lane branch returns without reaching the tail. | code | `docs/clone-hero/code/6142/five_lane_convert_215D450.asm` lines 56, 119 to 122 and 233 | checked 2026-11-12 |
| P3: The notes the tail leaves alone are the kicks. | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\five-lane\scorestats-Mid_No_Pro_F-20261112-131107.json` (score 5,750, note_score 2,150, combo_score 3,600, 43 of 43, v1.1.0.6142-final, ProDrums Expert) | checked 2026-11-12; the function 0x214C8A0 itself was not read, and the pads' 50-point result is what shows it answers no for yellow, blue and the fourth pad; T1 lists it |
| P4: Probe F's song.ini holds neither key. | chart | `C:\Clone Hero\songs\Hydra Probe\Mid No Pro F\song.ini` | checked 2026-11-12 |
| P5: Probe F's notes.mid holds 29 kicks, 2 red, 3 yellow, 4 blue and 5 green notes, no tom marker and no fifth pad. | chart | `C:\Clone Hero\songs\Hydra Probe\Mid No Pro F\notes.mid`, recounted with mido: PART DRUMS pitches 96 x 29, 97 x 2, 98 x 3, 99 x 4, 100 x 5, one note per tick, no 101 and no 110 to 112 | checked 2026-11-12 |
| P6: A .mid starts the two flags from song.ini's two keys and a .chart starts both at zero. | unchecked | `docs/clone-hero/01-reading-a-chart.md` block CR-18 says so but is labelled Unknown, and the .mid reader and .chart loader lines it cites were not opened | T1 |
| P7: The game's bool getter returns "not zero" for a whole number, true for "true" and false for "false" with case ignored, and the caller's default (false) otherwise. | code | the game's DLL at RVA 0x1D4480, listed with `py -m tools.ch_mcp.chdis 1D4480 600`; the listing is in the session scratch folder until T1 commits it | checked 2026-11-12 |
| P8: Both keys are read through that getter with a default of false. | code | `docs/clone-hero/code/6142/ini_song_reader_1EFC60.asm` lines 76 to 89 | checked 2026-11-12 |
| P9: Hydra's `drum_key_is_true` takes only "true", trimmed, in any case. | code | `src/parse/song.cpp` line 2250 | checked 2026-11-12 |
| P10: In the game, `pro_drums = 1` on a .mid with no tom marker keeps the cymbals. | unchecked | `docs/clone-hero/01-reading-a-chart.md` CR-18 says no run has checked `pro_drums` | LS1 |
| P11: The game cuts trailing blanks from a key's value before the getter reads it. | unchecked | `docs/clone-hero/code/6142/ini_int_getter_21647A0.md` names the lookup 0x1BD150 but not its trimming | T1 |
| P12: A red disco note on a tail chart still becomes a yellow cymbal under Pro Drums. | unchecked | `docs/clone-hero/02-note-list.md` NL-22 gives the flip by lane in the builder and says no run has checked it; the disco step runs before the conversion per `docs/clone-hero/code/6142/note_flags_from_markers_215E280.md` | LS3 |
| P13: The game's .sng metadata reader reads both keys the same way as the song.ini reader. | unchecked | `docs/clone-hero/code/6142/sng_meta_reader_1EFF90.md` says those lines were "not read line by line" | T1 |
| P14: `convert_drum_pads` is the one place raw pads become lanes, both readers reach it through `emit_chord_timestamp`, and its comment says the tail is left out. | code | `src/parse/song.cpp` lines 471 to 541 | checked 2026-11-12 |
| P15: Hydra on main scores probe F 6,470 with Pro Drums on and 5,750 with it off. | unchecked | the main session's `hydra_replay score` run, as it reported it to the plan's reviewer; no result file was kept. The 6,470 equals the sum in "What the user will see" (2,330 + 4,140), so it is consistent with the chart | MS3 |
| P16: Hydra on main scores Fortunate Son 239,905 with Pro on (588 cymbal notes) and 205,150 with Pro off. | unchecked | the main session's `hydra_replay score` run, as it reported it to the plan's reviewer; no result file was kept. The two splits agree with each other: 60,820 - 52,000 = 8,820 = 588 x 15 | MS3 |
| P17: The Library row, the Paths tab, the Preview and the Dynamics tab are the screens that read the changed values. | unchecked | a grep of `src/ui` and `src/app` for the Pro Drums setting found `paths_tab.cpp`, `preview_load_job.cpp`, `dynamics_tab.cpp` and `path_view.cpp`; no screen was opened | MS4 |
| P18: The loose-file scan found 21 of 15,019 .mid drum charts: seven with no `pro_drums` key (probe F and six Warriors of Rock charts) and fourteen with `pro_drums = 1`. | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\five-lane\nopro_library_scan.txt` | checked 2026-11-12; the scan script was not read |
| P19: The .mid reader starts every yellow, blue and green pad as a cymbal and takes the flags track-wide from `midi_drum_chart_flags`. | code | `src/parse/song.cpp` lines 909 to 922 and 1275 to 1285 | checked 2026-11-12 |
| P20: The .chart reader takes only `five_lane_drums` from the keys. | code | `src/parse/song.cpp` lines 2055 to 2058 | checked 2026-11-12 |
| P21: A .chart pad is a tom unless N 66 to 68 marks it, so a .chart with both flags down is already all toms in Hydra. | code | `src/parse/song.cpp` lines 985 to 994 (a pad takes `flag_cymbals_`, which starts all Normal at line 1034 and is set only by the cymbal marker at line 970), `ChartParser::note_optype` at line 1928 | checked 2026-11-12; test 7 pins it |
| P22: A .sng's keys reach the readers through `chart_drum_keys`, and an .srb passes none. | code | `src/parse/song.cpp` lines 2341 to 2377 | checked 2026-11-12 |
| P23: The disco flip runs after the conversion in Hydra, only under Pro Drums, and turns red into a yellow cymbal. | code | `src/parse/song.cpp` lines 531 to 541 and `src/core/model.cpp` lines 212 to 227 | checked 2026-11-12 |
| P24: `kResultsStamp` is "2.14.1+fivelane", its rule asks for a bump when a chart reads differently, and 2.14.1 has not shipped. | code | `src/store/stored_versions.h` lines 42 to 55; `CMakeLists.txt` line 21 (version 2.14.0); `git tag` (newest v2.14.0) | checked 2026-11-12 |
| P25: `tests/test_five_lane.cpp` holds the five-lane tests, a pinned-digest test for four-lane charts and a probe C and E score test; `tests/test_store_records.cpp` names the stamp. | code | `tests/test_five_lane.cpp` lines 125 to 349 (test names only); `tests/test_store_records.cpp` | checked 2026-11-12 |
| P26: Some checked-in corpus charts reach the tail, so pinned test values will move. | unchecked | `testdata/input` was not scanned | T4 |
| P27: `ch_start` with a song folder and `player="ProDrums,Expert"`, `ch_wait_stats` and `ch_stop` ran today on probe F. | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\five-lane\scorestats-Mid_No_Pro_F-20261112-131107.json` shows the run's result; the calls themselves are reported by the brief | checked 2026-11-12 for the result file only |
| P28: The probe audio is 125 seconds long, so a run takes about 2 minutes. | chart | `C:\Clone Hero\songs\Hydra Probe\Mid No Pro F\song.ogg`: last Ogg page position 5,512,500 samples at 44,100 Hz = 125.0 s | checked 2026-11-12 |
| P29: The user asked for this plan with the words "Yes, plan it", on the question "Should Hydra follow the game on these charts?" | ruling | the user's words as the main session quoted them in the plan reviewer's brief (task RV-MID-NO-PRO-PLAN); `docs/agents/rulings/clone-hero.md` holds the earlier "Yes, plan it" of the five-lane plan, not this one, and T5 writes this one | checked 2026-11-12 for the quote in the brief |
| P31: The game's number test, Int32.TryParse(string, out int), accepts blanks around the digits and a leading plus or minus sign, and refuses a decimal point and a value past 32 bits. | unchecked | `docs/clone-hero/code/6142/ini_int_getter_21647A0.md` names the same call without its number style; the listing of Int32.TryParse was not opened | T1 |
| P32: The method's branch test sends a chart to the five-lane branch when +0xA1 is up and +0xA2 is down, or when the song settings byte +0x10 is set; the settings byte starts from song.ini's `five_lane_drums`. | code | `docs/clone-hero/code/6142/five_lane_convert_215D450.asm` lines 26 to 37, `docs/clone-hero/code/6142/chart_settings_ctor_20E73E0.md` line 2 (+0x10 and +0x11 are the two flags' starting values) | checked 2026-11-12 for the jumps; that +0x10 is `five_lane_drums` rests on probes C and E (NL-74, NL-75) and the ctor note |
| P30: A text event `[mix 3 drums0d]` on a .mid drum track opens an Expert disco section in the game. | data | `docs/clone-hero/02-note-list.md` NL-22 (Proven in the game's code) and `docs/clone-hero/code/6142/disco_apply_215CD50.md`; the .mid text form itself was not opened | checked 2026-11-12 for the span rule; LS3's go request names this row |
| P33: A gem pays 50, a cymbal gem 65; the multiplier is 2x from the 10th gem, 3x from the 20th and 4x from the 30th; a gem's combo share outside Star Power is its points times the multiplier minus one. The expected totals of probes F, G and J are worked out from these. | data | `docs/clone-hero/06-scoring.md` SC-01, SC-02, SC-05 and SC-08, each labelled "Proven in the game's code" | checked 2026-11-12; probe F's 5,750 = 2,150 + 3,600 matches the sums |
