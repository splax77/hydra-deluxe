# Plan: reverse D13 everywhere, and the step panel from the fourth mockup (2026-10-31)

Task id `PBR4-PLAN`, session `711858a7-e2d0-4bc0-89e7-f2226a576639`. Planner: Fable. Branch `claude/pbr3-plan4` (cut from `claude/pb-redesign`, the Path Builder redesign branch, not on `main` yet). Status: plan written; nothing is built. Every file and line named below was read in this worktree on 2026-10-31.

The requirement is the user's own words in `docs/superpowers/specs/2026-10-31-path-builder-mockup-4/answers.md`, and the page beside it, `step-panel-mockup.html`. The user approved planning the two changes together: to "Reverse D13 everywhere, planned with the step panel?" the user said "Yes, plan it". The page itself is not approved; this plan goes to the user for its own yes. Plan 3 (`docs/superpowers/plans/2026-10-31-path-builder-redesign-3.md`) holds every earlier answer; they stand unless this plan says otherwise.

## The plain version

Two things are wrong, and they are one bug seen from two sides. In the engine, a squeeze-in whose note sits exactly on the Star Power end is treated as needing no timing, while a squeeze-out on that same note is treated as needing 0.0 ms. That is D13, from 2026-10-03. The user never asked for it: "0ms is not the same thing as no timing" and "i never asked for suqeeze ins to be treated differently for no reasons". Every screen that reads that answer then says something odd: the Paths tab tells you to hit the note "no more than 0.0 ms late" where the squeeze-out says "more than 0.0 ms late" (the user: "why does the squeeze in text say "no more than 0.0ms late" instead of "more than 0.0ms early" ??? who approved this text?? this makes no sense"), and the Path Builder's ending button read "no timing needed" for The Reaper's m91 ending.

Part A fixes the one answer at its one owner: a squeeze-in at offset 0 needs 0.0 ms, the same as the squeeze-out beside it. A squeeze-in already inside Star Power (a negative offset) still needs none. Nothing about where Star Power ends moves, and no score moves. Then every reader is checked: the badges, the hardest figure, the sentences, the library and report columns, the search, the all-0 list and the builder. The stored summaries hold the old answer, so the results stamp moves and the user is asked about the re-analysis that costs.

Part B rebuilds the builder's steps panel the way the fourth page shows it. A step's row carries its span, its phrases and its squeezes in the list rows' existing short form. The opened step shows only its endings, one line each, the kept one marked, and Remove. The paragraphs, the cost sentence, the leftover line and the glossary go. An earlier step, opened later, shows the same ending lines greyed. Every squeeze word on the panel comes from one function, and a scan rule keeps it that way.

## Part A. Reverse D13: what the code does today, and what moves

### The owner

`SPSqueeze::needs_no_timing` (`src/core/model.h` line 383) answers "is this squeeze already done with no frontend timing". It reads `core::paid_by_sp_walk(offset_ms)` (`src/core/backend_value.h` line 30, `offset_ms <= 0.0`): a squeeze-in needs none when its note is at or before the end, a squeeze-out when its note is after it. That is D13 (`docs/audit/2026-10-03-fix-decisions.md` line 31). The squeeze's figure is `difficulty()` (line 369, through `squeeze_difficulty`, line 275: the offset for a squeeze-in, minus the offset for a squeeze-out), so at offset 0 both figures are 0.0 and only the yes/no differs.

The fill rule beside it already says what the user wants. `early_fill_needs_timing` (line 324) is "the fill's difficulty is 0.0 or more": zero slack counts as a timing. The new squeeze rule is the same sentence: a squeeze needs timing when its difficulty is 0.0 or more. One helper in `model.h`, `timing_needs_hitting(double difficulty_ms)` (`>= 0.0`), becomes the one home of "a timing of zero must still be hit", and both `early_fill_needs_timing` and `SPSqueeze::needs_no_timing` (which returns its negation) call it. `paid_by_sp_walk` is not touched and nothing else reads it differently: Star Power still ends at one point (memory note `sp-end-is-one-point-leeway-is-delgars`, the user on 2026-10-29: "there should never be two different answers for where star power ends. ever."). The search, the SP walk, the backend pricing and the scores do not read `needs_no_timing`, so no score moves; the test that pins the corpus digest proves it.

### Every reader, and what moves

Each reader was found by grepping `needs_no_timing`, `needs_timing`, `difficulty()`, `hardest`, `badge_timings` and `hardest_ms` over `src/`, `tests/` and `docs/`. Session cards and the audit ledgers were left out; they record history.

`Activation::badge_timings` (`src/core/model.cpp` line 497) lists every squeeze with `required = !needs_no_timing()`. A squeeze-in at 0 becomes required. Its badge text, "squeeze in 0.0 ms", is the same words as today (the badge always carried the figure, D80), so the Paths tab's row badge does not change; its colour does not either (`is_difficult` is false at 0.0).

`Activation::hardest` (line 503) weighs required squeezes only. A squeeze-in at 0 now takes part with 0.0 ms. On an activation with nothing else to time, `hardest()` goes from empty to `{SqueezeIn, 0.0}`, so `difficulty()` goes from empty to 0.0 and `needs_timing()` from false to true. `Path::difficulty` and `Path::needs_timing` follow. On an activation that also has a squeeze-out at 0 (the corpus never pairs them on one activation; see the counts) the figure stays 0.0 and only the part's name can change, by list order.

The Paths tab's path button (`src/app/path_view.cpp` line 522) prints `p->difficulty()`: such a path now shows "0.0 ms" after its notation instead of nothing. The sentences (`squeeze_sentences`, line 95) read `needs_no_timing()` to choose the wording, so they move by themselves: the squeeze-in at 0 reads "Hit the SP phrase's last note more than 0.0 ms early so it lands before Star Power ends", the earn wording a squeeze-in had before D13 (commit `c68982cf` added the other branch), and the squeeze-out on the same note keeps "more than 0.0 ms late". The comments there cite D13 and are rewritten to cite the new record.

The squeeze rating (`src/core/squeeze_rating.cpp` line 156) passes `sq.needs_no_timing()` to `rate_note` as `inside`, which picks the transfer scale's side (early for a note inside Star Power, late for one past it). That argument is about where the note is, not whether it needs timing: a note at offset 0 is inside Star Power (the SP walk pays it, `paid_by_sp_walk`), the same as the squeezed-out row at line 124 reads it. So the rating keeps reading `paid_by_sp_walk(sq.offset_ms)` directly and no rating figure moves. This is the one-point rule applied, not a new choice; a test pins the scale side at 0 for both kinds.

The stored summary (`src/store/record_store.cpp` line 584) writes `path.difficulty()` as `hardest_ms`, and the library's "Hardest ms" column, the report's "Hardest ms" and "Timing" columns (`src/app/report.cpp` line 54 and `tier_for`), the report's "No timing needed" filter and the search's `squeeze<`, `squeeze<=`, `squeeze>` and `squeeze>=` (`src/app/library_query.cpp` line 418) all read that stored number. For a best path whose only timing is a squeeze-in at 0: the dash becomes "0.0", the tier "None" becomes "Normal", the path leaves "No timing needed", `squeeze>=0` now matches it, `squeeze<0` and any negative `squeeze<=` no longer do (D266: "A path that needs no timing still passes any limit" is unchanged as a rule; this path simply needs timing now). Saved rows hold the old number until re-analysed; see "Stored results".

The best all-0 path does not move. D85 and D102 made it a hard 0 ms limit through `Path::within_ms_limit`, which asks each squeeze `SPSqueeze::within_limit` by its own figure (`model.h` line 377) and never `needs_no_timing`; the all-0 pass's engine rule (`allzero_options`, `src/search/pather.cpp` line 69) does the same. A squeeze-in at 0.0 is within 0 ms before and after. Memory note `best-all-0-path-always-shown` says never to tighten it as a side effect; a corpus test pins that no chart loses its all-0 path.

The Path limit and the search's `act_within_limit` read `within_limit` too, so no path is added to or dropped from a limited search. `Activation::is_difficult` and `Path::is_difficult` read `difficulty()` against the 2 ms floor, which 0.0 never passes.

The Path Builder reads the answer in three places: `timing_text` (`src/pathbuilder/pb_coach.cpp` line 26, `Activation::difficulty()`) feeds `CoachRow::timing` and `EndChoice::timing`, and the ending button printed `kNoTimingNeededText` when that was empty (`pb_window.cpp` line 985); `squeeze_short_text` (`pb_text.cpp` line 141) and `squeeze_end_text` (line 153) read `badge_timings` and print ", no timing" or " that needs no timing" for a squeeze that is not required. After Part A, m91's ending reads "squeeze-in 0.0 ms" from the short form with no builder change. Part B then removes the button's own wording and `timing_text`'s use for endings.

`tight_window_note` (`pb_coach.cpp` line 14) reads `difficulty()` to name the figure. Under a negative limit, a step with a squeeze-in at 0 now reads "needs 0.0 ms, your limit is -200 ms" instead of "over your limit of -200 ms" (the ruling "Note, limit only" covered the case with no figure to name, which this no longer is). Whether the note is drawn at all is open question Q1.

Tests that pin the old answer and go red by design, each repinned by the task that owns its file: `SPSqueeze::needs_no_timing: a note on the SP end is inside SP` and the `on_end_in` block of `Activation::needs_timing` (`tests/test_model.cpp` lines 344 to 357 and 844 to 855); the "no more than 0.0 ms late (eff. 0.0 ms)" pins in `tests/test_activation_rows.cpp` lines 977 and 980; the hand-built "squeeze-in, no timing" pin in `tests/test_pb_text.cpp` lines 85 to 97 (a squeeze-in at 0.0 and a squeeze-out at 0.001); `tests/test_engine_sweeps.cpp` line 555 if a corpus chart's zero-ms set moves. `tests/test_pb_coach.cpp` line 166 (White Washed's squeeze-in with 130.4 ms to spare) still needs none and stays.

### Docs that state the rule

`CONTEXT.md` "Needs no timing" (line 674) defines it by the SP walk and names `needs_no_timing` as the one answer; it is rewritten to the new rule ("a squeeze whose difficulty is below 0: a squeeze-in already inside Star Power, or a squeeze-out already past it; at exactly 0 both need 0.0 ms"). `CONTEXT.md` line 259 (a path that needs no timing reaches no floor) stays true. `docs/UserGuide.md` lines 251, 484 and 499 describe "needs no timing" without the boundary and stay true; its one sentence example (line 245, Burnout's 163 ms squeeze-out) is unchanged, so no public page changes and no doc review is needed. `README.md` and `docs/differences-from-public-hydra.md` do not state the rule (grepped for "no timing", "no more than", "ms late", "ms early", "free squeeze" on 2026-10-31). The Clone Hero pages live on `main` only (`docs/clone-hero/`, not on this branch); `05-star-power.md`, `06-scoring.md` and `03-timing.md` on `main` were grepped for "no timing", "needs none", "offset 0", "exactly on the SP end" and say nothing about which squeeze needs timing, which is Hydra's display rule, not a game fact. So no Clone Hero page changes. The decision record: a new D-record superseding D13, numbered after the last on `main`, which is D300 (`git show main:docs/audit/2026-10-03-fix-decisions.md`, read 2026-10-31), so D301; this branch's copy of the file ends at D267, and plan 3's DOC-7 also adds a record after `main`'s last, so the catch-up merge renumbers whichever lands second. D13 gets a line under it, "D301 reverses this", as D14 has for D256.

### Stored results

`src/store/stored_versions.h` says the results stamp must move when a summary column changes, and `hardest_ms` is one (`kSummaryColumnList`, record_store.cpp). It is "2.12.1+ch10grid" today. The saved `hardest_ms` of a path whose only timing is a squeeze-in at 0 reads empty where this build computes 0.0, so the library column, the report tier and the `squeeze` search would disagree with a fresh analysis on those rows. The rule says bump; the cost is that every saved result reads Stale and asks for re-analysis, as D105, D250 and the 2026-10-24 bump each did with the user's acceptance. That is open question Q2, with the corpus numbers so the user can weigh it. The stamp written becomes the version of the release that ships it plus a tag (as "2.8.0+efill85"), for example "2.14.0+sqin0" (the project version is 2.14.0, `CMakeLists.txt` line 21), with `accepted` shrunk to it alone.

### Measured on the corpus

Tool and command, run on 2026-10-31 from this worktree, never over the user's library: `build-cpp\Release\hydra_replay.exe dump --chart <file> --out <json>` on each of the 97 chart folders under `testdata/input` (one `notes.chart`, `notes.mid` or `.sng` per folder; the "about 115" in the brief counts folders with no chart file too), at the dump's default cap, depth and limit, then a script over the JSON (`scratchpad\d13_count.py` and `d13_count2.py`, scratch files, not evidence; the numbers below are what they printed). `dump` lists `rec.all_paths()`, the shown paths and the all-0 paths.

97 charts, 382 paths, 1,488 activations. 45 activations on 40 paths on 13 charts carry a squeeze-in at exactly 0.0 ms. None of the 45 has a squeeze-out that needs timing beside it, so each one's hardest figure moves from "none" to 0.0 ms unless a required early fill at or past its deadline already names it (the dump does not write `e_offset`; the executor's corpus test counts it exactly). 6 of the 13 charts carry one on their best path: Evans Blue "Beg" (`1+ 1+`), Smash Mouth "I'm A Believer" (`1 0+ 0 0`), The Fall of Troy "Act One, Scene One", CHON "Book", sungazer "All These People" and black midi "Sugar/Tzu" (`0 E3+ E5 E0`; the others are Tigran Hamasyan "Entertain Me", Ne Obliviscaris "Tapestry of the Starless Abstract", Xane60 & SoundHaven "Bug Thief", Matt McGuire "Avalanche", HopH2O "I Am... All Of Me", Victoria "Grow" and The Faceless "The Spiraling Void"). So 6 library rows change their Hardest ms from a dash to 0.0 and their tier from "None" to "Normal". 45 Paths-tab sentences change from "no more than 0.0 ms late" to "more than 0.0 ms early"; the 28 squeeze-outs at exactly 0.0 keep "more than 0.0 ms late"; the 120 squeeze-ins inside Star Power (offset below 0) keep their wording. Ratings that change: 0, because the rating keeps the SP walk's side. Paths that leave or join the all-0 list: 0 (the limit test is by figure). Scores that change: 0.

One more thing the count showed: every two-ending activation at this boundary in the corpus pairs a squeeze-in at 0.0 (ending one phrase later) with a squeeze-out at 0.0 (ending one phrase earlier), 17 such pairs on 8 charts, The Reaper's shape exactly. The user's own example, "Bound" by Hail The Sun (activation 2 at m92.1.0), is in the user's library and not the corpus, so it belongs to the real-app check only.

## Part B. The step panel: what the code does today, and what moves

The panel is `PbApp::draw_steps` (`src/pathbuilder/pb_window.cpp` line 896). Each step is a compact row (`step_short`, `pb_text.cpp` line 210: "m32 → m36" and "2 phrases") with its cost at the right; the opened step draws `step_sentence`'s paragraph, its "why" line and the violet note (lines 959 to 971), then, for the last step only, the "End it at:" buttons from `DecisionList::ends()` (lines 973 to 996: the end's measure, `EndChoice::timing` or `kNoTimingNeededText`, and `end_costs`), then Remove. After the steps come `leftover_text` (line 1021) and `glossary_text` (line 1032).

Endings exist only for the last step by a screen rule: `PbApp::select_step` (line 277) calls `DecisionList::select` only when the index is the last window ("the user's answer Q6" of the first plan). `DecisionList::select` itself (`pb_coach.cpp` line 626) works for any row: it takes the windows before the row and starts `end_choices` on a worker, which runs one `search_target` with the window pinned by its activation alone and one `best_finish` per distinct end. A new path key clears `ends_` (line 430).

### How an earlier step gets its endings: computed when opened

The user's design requirement, word for word: "the real app should only calculate paths as needed in real time. calculating and storing every possible path combination would be insanely inefficient for long tracks".

Two ways fit it. Keep the endings the step had when it was last (one small list per step, no new searches), or compute them when the step is opened (the same searches the last step costs today). This plan computes when opened. Why: the figures are the same either way. An ending's cost is its best finish with the windows before it pinned and nothing after, and the player builds left to right, so the windows before an earlier step cannot change without removing it; the list computed later is the list it had. Keeping would need a store that survives undo, redo, paste and "Finish the rest for me", where the endings were never computed, so those steps would have none to show; and `DecisionList::select` already does the computing for any row. The cost of one open is one `search_target` and one `best_finish` per ending, the same as opening the last step today, started only when the player opens the step, and nothing is stored past the current path (a new key clears it, as today). While the job runs the opened step shows its Remove button and no ending lines, as today's last step does; no new "pending" text is added.

### The words come from one place

Every squeeze word and figure on the panel comes from `squeeze_short_text` over `Activation::badge_timings` ("squeeze-out 0.0 ms", "squeeze-in 0.0 ms", "squeeze-in, no timing" for one inside Star Power), the form the "Where to activate next" rows already print. `EndChoice::timing` and `kNoTimingNeededText` go. The main session wrote nothing of its own on the page: the row's "3 phrases · squeeze-out 0.0 ms" and the ending line "ends m89 · squeeze-out 0.0 ms" join `measure_at`, `counted` and the short form with `app::kFactSeparator`; "End it at:" is `kEndItAtLabel`; "best" and "−10,380" are `cost_or_best` over `end_costs`. Two figures on the page are not the engine's (answers.md: the −12,440 row cost and the m91 figure shown as the raw 0.0); the built panel shows the ladder's cost and the short form's figure, whatever they are.

## Prior art

None searched. The panel's words and layout are fixed by the user's page, and Part A is a reversal to the code's own earlier rule; no outside source would change a choice here.

## Tasks

Each task fits one Opus executor at about 100 tool calls and 20 minutes. Each runs in its own worktree under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\`, forked from `claude/pbr3-plan4` or as "Blockers" says. No two tasks that run at once own one file. Each runs only the tests it names; tests pin literals from one run or call the production function. One fresh reviewer per task (D107). Every word the player reads has one home. An executor that reaches its budget commits what builds and hands the rest to a fresh executor.

### Wave 1: A1 and B2 start together at the user's yes

#### A1. The one answer: a squeeze at exactly 0 needs 0.0 ms (core)

Owns `src/core/model.h`, `src/core/model.cpp`, `src/core/squeeze_rating.cpp`, `tests/test_model.cpp`, `tests/test_squeeze_rating.cpp` (if it exists; else the rating pin goes in `test_model.cpp`).

What it builds. `timing_needs_hitting(double difficulty_ms)` in `model.h` beside `early_fill_needs_timing`, which now calls it. `SPSqueeze::needs_no_timing()` returns `!timing_needs_hitting(difficulty())` and its comment points at the new record instead of D13 and no longer reads `paid_by_sp_walk`. Comments in `model.h` (lines 322, 380, 547, 660, 669, 682) and `model.cpp` (lines 504, 532) that cite D13 cite D301 (or the number REV settles) and say the rule's new home. `rate_activation` passes `core::paid_by_sp_walk(sq.offset_ms)` as `inside` for a squeeze-in (line 156), with the comment saying why: the side is where the note is, the one SP end. No other logic changes; the executor confirms with a grep that `paid_by_sp_walk`'s body and the search are untouched.

Tests, `test_model.cpp`, pinned by the production function: `SPSqueeze::needs_no_timing: a squeeze at exactly 0 needs 0.0 ms, one inside needs none` replaces the D13 case (squeeze-in at 0.0, -0.0 and 0.001 need timing, at -0.001 none; squeeze-out at 0.0 and -0.001 need timing, at 0.001 none); `Activation::needs_timing`'s `on_end_in` block flips: `needs_timing()` true, `difficulty() == 0.0`, `hardest()` names `SqueezeIn` at 0.0, and the path with `on_end_in` needs timing; `timing_needs_hitting` pinned at -0.001, 0.0 and 0.001 and shown equal to `early_fill_needs_timing` at the same values; `rate_note side at the SP end: a squeeze-in and a squeeze-out at 0 both read the early side` (an activation with a scale whose early and late differ; `note_effective_ms` for the squeeze-in at 0.0 uses `early`, as the squeezed-out row's does). `test_sp_end.cpp`'s `paid_by_sp_walk` pins stay.

Run: `build-cpp\Release\hydra_tests.exe "-sf=*test_model.cpp"`; `"-sf=*test_sp_end.cpp"`; `"-sf=*test_squeeze_rating.cpp"` if present; `"-sf=*single_owner*"`.

Done when they pass. Blockers: none. earlyStart: at the user's yes.

#### B2. The coach: an ending carries its activation, and the kept one is known (no screen code)

Owns `src/pathbuilder/pb_coach.h`, `src/pathbuilder/pb_coach.cpp`, `tests/test_pb_coach.cpp`.

What it builds. `EndChoice` drops `timing` and gains `Activation activation` (the window's activation in the path `search_target` found, `acts[k]` at line 99), so the screen can print `squeeze_short_text` and the end measure from it. `timing_text` stays only for `CoachRow::timing` if a screen reads it; the executor greps `pb_window.cpp` and the tests (today only `test_pb_coach.cpp` lines 83 and 145 read it) and removes `CoachRow::timing` and `timing_text` if nothing on screen does, with their test lines. One function answers "is this ending the one the step kept": `bool is_kept_end(const PinnedWindow& window, const EndChoice& end)` in `pb_coach`, true when `end.window`'s `deact_tick` and `squeezed_out` equal the window's (the pair `end_choices` already keys on, line 92). `DecisionList::select` is unchanged (it computes for any row).

Tests, `test_pb_coach.cpp`, from one run on White Washed (`coach end: choosing the last window's end pins it`, line 206, already reads two ends): each end's `activation.notationstr()` equals its `token`, the squeeze-in end's `squeeze_short_text` reads "squeeze-in, no timing" and the squeeze-out end's "squeeze-out 130.4 ms" (via `pb_text.h`); `coach end: the kept end is the window's own end`: `is_kept_end(four[3], ends[k])` is true for exactly the end whose `deact_tick` is the window's; `coach end: an earlier window's endings are the same list` : `end_choices` over `before` for window 2 of the five gives the same ends and best finishes whether or not windows 3 and 4 exist (they are not in `before`), pinned from one run.

Run: `build-cpp\Release\hydra_tests.exe "-sf=*test_pb_coach.cpp"`; `"-sf=*single_owner*"`.

Done when they pass. Blockers: none. earlyStart: at the user's yes.

### Wave 2: A2, A3 and B1 fork from A1's commit; B3 from B1 and B2

#### A2. The Paths tab's sentences and badges at the boundary

Owns `src/app/path_view.cpp`, `tests/test_activation_rows.cpp`.

What it builds. No logic change in `path_view.cpp`: the sentence branch already reads `needs_no_timing()`. Its comments (lines 85, 102 to 105) cite the new record and say the wording follows the owner. `longest_activation_badge`'s comment (line 85 to 87) stays true (a squeeze that needs no timing has a figure below 0 now, still inside the window).

Tests, `test_activation_rows.cpp`, pinned from one run: `squeeze sentences at the SP end: the squeeze-in earns, the squeeze-out earns` replaces the lines 977 and 980 pins: a squeeze-in at 0.0 reads "Hit the SP phrase's last note more than 0.0 ms early so it lands before Star Power ends." (with its eff. suffix when the scale is not 1) and a squeeze-out at 0.0 on the same activation shape reads "Hit the SP phrase's last note more than 0.0 ms late so it lands after Star Power ends."; a squeeze-in at -0.001 keeps "no more than 0.0 ms late"; `badge_timings` for a squeeze-in at 0.0 reads "squeeze in 0.0 ms", `required` true, `difficult` false, and the path button's timing reads "0.0 ms" (the A2 block at line 718 gains the 0.0 case beside the -315.8 one).

Run: `build-cpp\Release\hydra_tests.exe "-sf=*test_activation_rows.cpp"`; `"-sf=*test_path_view.cpp"`.

Done when they pass. Blockers: A1 (earlyStart from its executor commit).

#### A3. The stored summary, the library, the report and the corpus counts

Owns `src/store/stored_versions.h`, `tests/test_engine_sweeps.cpp`, `tests/test_record_store.cpp` (the stamp's pin, if one exists there; the executor greps `kResultsStamp` in `tests/`), `tests/test_library_query.cpp`.

What it builds. If Q2's answer is "bump": `kResultsStamp` becomes `"2.14.0+sqin0"` (or the release version REV names), `accepted` shrunk to it, with the header's comment line for it in the style of the ones above. If Q2's answer is "no bump": nothing in the header, and the plan's "Out of scope" gains the stale rows by name. Either way the tests below.

Tests, pinned from one run on the corpus: `corpus: squeeze-ins at exactly 0 and what they change` in `test_engine_sweeps.cpp` beside the zero-ms sweep at line 555: over `testdata/input`, count activations with a squeeze-in at offset 0.0 (45), those whose `difficulty()` is 0.0 (the exact count the dump could not give), best paths whose `difficulty()` is 0.0 through such a squeeze-in (6), and that every chart's all-0 list is the same set of pathstrings as before (pin the count of charts with an all-0 path); the corpus digest test (whatever pins scores on the corpus today) is run and must not move, which proves no score moved. `test_library_query.cpp` gains `squeeze filters: a path whose hardest is 0.0 passes squeeze<=0, matches squeeze>=0, fails squeeze<0 and squeeze<=-5` with `facts.hardest_ms = 0.0` (D266's strict rule, pinned at the boundary, memory note `boundary-cases-follow-the-rule`).

Run: `build-cpp\Release\hydra_tests.exe "-sf=*test_engine_sweeps.cpp"` (the corpus test; the executor names its `-tc=` if the file holds a whole-library guard); `"-sf=*test_library_query.cpp"`; `"-sf=*test_record_store.cpp"`.

Done when they pass. Blockers: A1 (earlyStart from its executor commit); Q2 answered before the stamp line is written (the tests can start first).

#### B1. The text: the row's squeezes, the ending line, and what goes (no screen code)

Owns `src/pathbuilder/pb_text.h`, `src/pathbuilder/pb_text.cpp`, `tests/test_pb_text.cpp`, `tests/single_owner/rules/pathbuilder/squeeze-words.json` (new).

What it builds. `squeeze_short_text` leaves the anonymous namespace and is declared in `pb_text.h` as the one home of a squeeze's words and figure on the builder. `StepShort` gains `std::string squeezes` (the short form; empty when the activation has none), so the row reads "m83 → m89", "3 phrases · squeeze-out 0.0 ms" (page line 91: the saved span joins the squeezes with " · " only when there are any). `end_choice_text(const SongTiming&, const Activation&)` returns "ends m89 · squeeze-out 0.0 ms" (page line 104: `measure_at` of the deact node, then the short form when there is one; an activation with no deact node reads its `notationstr`, as the button did). `step_sentence`, `StepText`, `leftover_text`, `kNothingLeftText` and `glossary_text` are removed with their declarations (answers.md: gone from the opened step; the leftover line and glossary sit after the steps and the page has neither). `squeeze_end_text` stays: the fill card's "Hydra would end it at" line reads it. The scan rule: question "What words and figure does the Path Builder show for a squeeze?", owner `squeeze_short_text` in `pb_text.cpp`, scope `src/pathbuilder/`, pattern matching a string literal holding `squeeze-in`, `squeeze-out` or `no timing`, or a read of `Activation::difficulty()` to make text (`timing_text`), outside the owner's lines; `must_match` holds today's `kNoTimingNeededText` line and `pb_window.cpp` line 985; `must_not_match` the owner's two lines and `squeeze_end_text`'s (it reads the same `squeezes_of` and is the long form the card needs; the rule's note says so).

Tests, `test_pb_text.cpp`, from one run on White Washed (the existing fixtures at lines 57 to 97): `step short: the row carries the squeezes in the short form` ("m132 → m136", "2 phrases · squeeze-out 130.4 ms"; a step with none has an empty `squeezes`); `end choice text: the end measure and the short form` ("ends m136 · squeeze-out 130.4 ms"; an end with a squeeze-in inside Star Power reads "ends m136 · squeeze-in, no timing"); the hand-built pin at lines 85 to 97 becomes two: a squeeze-in at -0.001 reads ", no timing" and a squeeze-in at 0.0 reads "squeeze-in 0.0 ms" (A1's rule); the `step sentence` tests (lines 201 to 224) go with the function.

Run: `build-cpp\Release\hydra_tests.exe "-sf=*test_pb_text.cpp"`; `"-sf=*single_owner*"` (the new rule must pass once B3 lands; until then the executor runs it to see its `must_match` lines caught, and B3's run proves it clean).

Done when they pass. Blockers: A1 (earlyStart from its executor commit; only the 0.0 pin needs it).

#### B3. The screen: the step panel as the page shows it

Owns `src/pathbuilder/pb_window.h`, `src/pathbuilder/pb_window.cpp`, `tests/pb_ui/pbtest_coach.cpp`, `tests/testdata_dirs.h`.

What it builds. `draw_steps` draws each row as today with `brief.squeezes` appended to the dim text after " · " when not empty (page line 91). The opened step draws, when `d.ends().size() > 1`: `kEndItAtLabel`, then one line per end: a radio mark (`ui::dot_item` in the teal of the page's `.end.on` or the existing selection colour, filled for the kept end as `is_kept_end` says, hollow otherwise; the executor uses the colours `text_role.h` and the theme already have and stops to ask if none fits), `end_choice_text`, and `cost_or_best(end_costs(...)[e])` at the right in its tier colour, through the existing `text_at_right`. On the last step each line is a `Selectable` named `##end_<deact_tick>` that calls `d.choose_end` as the button did; on an earlier step the lines are inside `ImGui::BeginDisabled`/`EndDisabled` (greyed, page line 30, and not clickable). A step with one ending draws no lines. Then Remove. `select_step` calls `state.decisions().select(index)` for every index (the earlier-step rule above). The sentence, "why" line, `leftover_text` block (lines 1020 to 1026), `glossary_text` calls (lines 905 and 1032) and `kNoTimingNeededText` go. The violet note's fate is Q1: if kept, it is drawn under the ending lines as the one violet line, from `tight_window_note`, as today. `kNoStepsText` stays for the empty panel.

Tests, `pbtest_coach.cpp`: `finish-for-me` reads the compact rows instead of the sentence (lines 96 to 98 go; the row "m58 → m68 4 phrases −13,640" is already pinned); `tight-window-note` follows Q1 (kept: reads the note under the ending lines, no sentence; dropped: the test goes); `choose-an-ending` on Smash Mouth "I'm A Believer" (`testdata/input/common/IB24/T1/Smash Mouth - I_m A Believer (The Monkees cover) [highfine]/notes.chart`, a `believer_notes()` helper in `testdata_dirs.h` beside `white_washed_notes`; from the dump: best path `1 0+ 0 0`, activations at ticks 26880 and 52992, the second ending at 59136 with a squeeze-in at 0.0 or at 57600 with a squeeze-out at 0.0, path `1 0- 1`): click the fill at 26880, then the fill at 52992, `wait_for_edit`; the second step opens with two lines, "ends m<59136> · squeeze-in 0.0 ms" marked and "best", and "ends m<57600> · squeeze-out 0.0 ms" with its cost (measures and cost pinned from one run); the row reads "m<52992> → m<59136> 2 phrases · squeeze-in 0.0 ms" (phrases from the run); click the squeeze-out line, `wait_for_edit`: the kept mark moves, the row reads "squeeze-out 0.0 ms" and the "Your score" figure moves (pinned); click it back; click the next fill the list offers, `wait_for_edit`, then click step 2's row: the same two lines are on screen, greyed (`ItemInfo` of `##end_57600` reports disabled, or a click on it changes nothing: the windows count stays 3 and the kept mark stays), the kept one still marked; nothing on screen reads "Activate at", "This choice costs", "You still have Star Power", "no timing needed" or "Cost: how many points". Every assertion reads the screen.

Run: `build-cpp\Release\hydra_pathbuilder_uitest.exe --test choose-an-ending --test finish-for-me --test tight-window-note --test cant-activate-reason --test locked-fill-says-left-to-right --test hover-shows-never-plays --test fill-card --test row-click-moves-clock`; `hydra_tests.exe "-sf=*single_owner*"` (B1's rule must be clean here).

Done when they pass. Blockers: B1 and B2 (earlyStart from their executor commits merged together); Q1 answered.

### Wave 3

#### B4. The click-through tests that pinned the removed sentences

Owns `tests/pb_ui/pbtest_build.cpp`, `tests/pb_ui/pbtest_window.cpp`.

What it builds. `build-a-path-by-clicking` (lines 159 to 206): `step_one` and `step_two` sentences go; the test reads the compact rows it already pins, asserts the opened step shows Remove and no "Activate at", and that the first step's row stays when the second opens. `open-starts-empty` (`pbtest_window.cpp` line 157) drops its sentence read and keeps the row. `build-a-path-on-a-long-song` keeps its row reads (the executor greps the file for "Activate at", "This choice", "You still have" and "Cost:").

Run: `hydra_pathbuilder_uitest.exe --test build-a-path-by-clicking --test open-starts-empty --test build-a-path-on-a-long-song --test follow-the-playhead --test scroll-the-timeline --test no-star-power --test file-open-error`.

Done when they pass. Blockers: B3 (earlyStart from its executor commit). `build-a-path-by-clicking` and `open-starts-empty` are red from B3 until B4 mends them, so B3 and B4 merge into `claude/pb-redesign` together and nothing moves to `main` before B4 does.

#### RA-7. The real-app check (the main session, after A1 to A3, B1 to B4 merge)

A check, not a build, capped at 20 minutes. As plan 3's RA-6: a throwaway check branch from `claude/pb-redesign` plus `claude/pb-hy1`, built, run from a scratch folder on fresh copies of the user's `hydra.db`, `hydra_settings.ini` and `hydra_ui.ini` with installed Hydra closed; songs opened by search, never by scrolling the library. In the builder, The Reaper: activate at m48 and m83, open step 2 and read its two lines ("ends m89 · squeeze-out 0.0 ms" and "ends m91 · squeeze-in 0.0 ms", one marked, one "best"); switch the ending and read the row; activate at m103, open step 2 again and see the lines greyed with the kept one marked. In Hydra's Paths tab, "Bound" by Hail The Sun, activation 2 at m92.1.0: the squeeze-in reads "more than 0.0 ms early", the squeeze-out "more than 0.0 ms late", the badge "squeeze in 0.0 ms", the path button its figure; and the library's Hardest ms once the song is re-analysed. If Q2 bumped the stamp, the user sees the library read Stale at open, as the question said. Each number on screen is checked against the data (`hydra_replay score` or `target`) before it is called right or wrong. The user says yes or names what to change before DOC-8 starts.

#### DOC-8. The records and the pages

Owns `docs/audit/2026-10-03-fix-decisions.md` (D301: the user's words from answers.md and the coordinator's message, the rule, what moved and the corpus counts; a line under D13; and this plan's "The user's answers" once given), `CONTEXT.md` ("Needs no timing", line 674, and a Path Builder line if one names the step sentence), `docs/agents/ui-testing.md` (the new test name `choose-an-ending`, the item names `##end_<tick>`), `docs/superpowers/specs/2026-10-28-path-builder-design.md` (a dated line naming the fourth mockup). No public page changes (see "Docs that state the rule"), so no doc review; if RA-7 finds a User Guide sentence that does, it gets the doc review then.

Run: `hydra_tests.exe "-tc=docs match code*"`.

Done when it passes and the reviewer signs off. Blockers: RA-7's yes.

#### REV and the merge (the main session)

After DOC-8 merges into `claude/pb-redesign`: the catch-up merge with `main` (the decisions file: keep this branch's D265 to D267 and `main`'s D268 to D300, renumber D301 if `main` or plan 3's DOC-7 took it), one fresh review of the whole range, then `claude/pb-redesign` to `main`, then `claude/pb-hy1`. Each merge runs what `hydra_precheck.exe --tests` prints plus the three repo-wide checks (CLAUDE.md rule 1).

## Waves, in one line each

A1 and B2 start together at the user's yes. A2, A3 and B1 fork from A1's commit (A3's stamp line waits for Q2). B3 forks from B1 and B2 together once Q1 is answered. B4 forks from B3. A1, A2, A3, B1, B2 merge into `claude/pb-redesign` as they pass; B3 and B4 merge together. RA-7, then DOC-8 after the user's yes, then REV and the merges.

## Out of scope (for the user to confirm)

- The short form's ", no timing" for a squeeze inside Star Power ("squeeze-in, no timing") where the Paths tab's badge shows the figure ("squeeze in -130.4 ms", D80). The brief says the panel uses the existing short form, so it stays; making the two agree is its own question.
- The page's −12,440 row cost and its raw 0.0 figure: the panel shows the ladder's cost and the short form, whatever they are (answers.md).
- "Nothing left on the table after your last activation." (`kNothingLeftText`) goes with the leftover line it shares; the page shows no line after the steps.
- Any "pending" text while an opened step's endings are being computed: today's last step shows none and the page shows none.
- A store of endings per step (the "keep when computed" design): computing when opened gives the same figures.
- Renaming `needs_no_timing`: the name still says what it tests; the reviewer may rename if it reads better.
- Everything in the earlier plans' "Out of scope" lists.

## Open questions for the user

**Q1. The violet tight-window note on a step.** The code's rule (`tight_window_note`, `src/pathbuilder/pb_coach.cpp` line 14): when a Path limit is set in `hydra_settings.ini` and the step fails `Activation::within_ms_limit` (any squeeze or required early fill over the limit), the opened step shows one violet line naming the figure and the limit, "needs 130.4 ms, your limit is 10 ms", or "over your limit of -200 ms" when the step has no figure. One side: White Washed, step 4 (m132 → m136, squeeze-out 130.4 ms) under the 10 ms limit shows the line. Other side: every Burnout step under the same limit shows nothing, and The Reaper's m83 step under the default 10 ms limit shows nothing (0.0 ms is within it). The page never showed a limited step. Keep it (drawn under the ending lines, the only violet text on the panel, the `tight-window-note` test kept) or drop it (the panel shows only what the page shows; the Path limit then has no sign on the builder)? Recommendation: keep it; it was the user's own ruling ("Optimal, flag tight") and the page did not cover the case.

**Q2. The results stamp.** The rule in `src/store/stored_versions.h`: when a summary column changes, bump the stamp, and every saved result reads Stale and asks for re-analysis. The column that changes is `hardest_ms`, for a best path whose only timing is a squeeze-in at exactly 0.0: 6 of the 97 corpus charts (about 6%), so on the user's library some hundreds of rows would show a dash where a fresh analysis shows 0.0, the "No timing needed" filter would keep them and `squeeze>=0` would miss them until each is re-analysed by hand. One side: bump, the whole library reads Stale once, as D105, D250 and the 2026-10-24 bump did. Other side: no bump, saved rows stay, and those rows disagree with a fresh analysis until re-analysed, which Hydra does not flag. Recommendation: bump, because the header's rule is the one rule for stored data (memory note `stored-versions-one-rule`), and a release that ships two bumps shares one (D23).

## Promise map

| Promise | Plan line | Task | Check that proves it |
|---|---|---|---|
| A squeeze-in at offset 0 needs 0.0 ms, like the squeeze-out on the same note; one at a negative offset needs none | the user: "0ms is not the same thing as no timing"; brief | A1 | `SPSqueeze::needs_no_timing: a squeeze at exactly 0 needs 0.0 ms, one inside needs none` (both kinds, both sides of 0) |
| The fill rule and the squeeze rule share one home for "zero must be hit" | "The owner" | A1 | `timing_needs_hitting` pinned equal to `early_fill_needs_timing` at -0.001, 0.0, 0.001; `single_owner` scan |
| Scores do not change | brief | A1, A3 | the corpus digest test does not move; `corpus: squeeze-ins at exactly 0...` pins scores unchanged |
| Star Power ends at one point; `paid_by_sp_walk` and the SP walk untouched | brief; memory note | A1 | `test_sp_end.cpp` pins unchanged; reviewer diff of `backend_value.h` and `src/search` is empty |
| The rating's scale side at 0 is the early side for both kinds; no rating moves | "Every reader" | A1 | `rate_note side at the SP end...`; corpus count of ratings changed is 0 in A3's test |
| `badge_timings`, `hardest`, `difficulty`, `needs_timing` move as stated | "Every reader" | A1, A2 | `Activation::needs_timing`'s `on_end_in` block; the A2 badge pin ("squeeze in 0.0 ms", required) |
| The squeeze-in sentence at 0 reads "more than 0.0 ms early", the squeeze-out "more than 0.0 ms late"; inside stays "no more than ... late" | coordinator's message with the user's words | A2 | `squeeze sentences at the SP end: ...` pins all three |
| The library column, report tier and `squeeze` search read the new hardest; D266's strict rule holds at the boundary | "Every reader" | A3 | `squeeze filters: a path whose hardest is 0.0 ...`; the stamp line (Q2) |
| The all-0 list does not change | D85, D102, memory note | A3 | `corpus: squeeze-ins at exactly 0...` pins every chart's all-0 pathstrings |
| The Path limit and limited searches do not change | "Every reader" | A1 | `within_limit` untouched (reviewer diff); `test_engine_sweeps.cpp` line 555's sweep |
| Stored results: the stamp moves or the user accepts stale rows | "Stored results" | A3, Q2 | the stamp pin in `test_record_store.cpp` (or the header's `static_assert`) |
| The corpus counts are measured, not guessed | brief | A3 | the corpus test pins 45 / 6 / exact `difficulty()` count |
| The docs name the rule once; D301 supersedes D13; no public page changes | "Docs that state the rule" | DOC-8 | `docs match code`; reviewer grep of "no timing" over `docs/UserGuide.md` and `README.md` |
| A step's row shows span, phrases and squeezes in the short form; no squeeze, no words | page line 91 | B1, B3, B4 | `step short: the row carries...`; the rows read in `choose-an-ending` and `build-a-path-by-clicking` |
| The opened step with more than one ending shows "End it at:" and one line per ending, kept marked, others clickable | page lines 100 to 105 | B1, B2, B3 | `end choice text...`; `coach end: the kept end...`; `choose-an-ending` (mark moves on click) |
| An earlier step shows its endings greyed, kept marked, not clickable | page line 30; answers.md | B3 | `choose-an-ending` (disabled item or a click that changes nothing) |
| Endings are computed when the step is opened, never stored for every combination | the user's design requirement | B2, B3 | `coach end: an earlier window's endings are the same list`; reviewer: `select` runs one job per open, `ends_` cleared on a new key (line 430) |
| A step with one ending shows no lines, only Remove | answers.md | B3 | `build-a-path-by-clicking` (Burnout's steps have one ending) |
| The sentence, cost sentence, before/after line, leftover line and glossary are gone | answers.md | B1, B3, B4 | the functions removed (compile); `choose-an-ending` asserts none of the five texts is on screen |
| Every squeeze word and figure comes from `squeeze_short_text` over `badge_timings`; no invented text | brief | B1, B3 | the scan rule `pathbuilder/squeeze-words.json`; `kNoTimingNeededText` and `EndChoice::timing` gone (compile) |
| The ending button no longer reads `Activation::difficulty()` | brief | B2 | `timing_text` no longer feeds `EndChoice` (compile; scan rule) |
| A click-through test walks The Reaper-like case on a corpus chart | brief | B3 | `choose-an-ending` on Smash Mouth "I'm A Believer" |
| The old GUI pins are repinned | brief | B3, B4 | their Run lines |
| The real-app check covers The Reaper at m83 and Bound's sentences | brief; coordinator | RA-7 | the user's yes in chat |
| Numbers on screen are checked against the data | memory `check-the-screen-against-the-data` | RA-7 | `hydra_replay score`/`target` on the built path |
| No new number or text without the user | brief | Q1, Q2 | the questions; executors stop and ask |
| No whole-library run | preamble | every task | every test names a corpus chart; RA-7 opens songs by search |
| Agents never run the full suite | rule 1 | every task | each task's Run line |
