# Delta DL5: tests/test_fills_and_skips.cpp, test_batch_text.cpp, test_cli.cpp, test_activation_rows.cpp, test_docs_match_code.cpp, test_engine_sweeps.cpp, test_preview_controller.cpp, tests/ui/uitest_details.cpp, uitest_library.cpp, uitest_live.cpp, helpers corpus_util.h and song_digest.h, rule file concept-G5.json, tools/test_compare_db.py

Auditor: T5-DL5, agent a3849b055ee0dd929. Read-only; nothing run. Diffs read: `git diff c10c7f54 5ec682c7 -- <file>` for every file above.

## What every record shares

- What changed in main: the Early fill window became a setting. `kDefaultEarlyFillWindowMs = 60`, range 0 to `kMaxEarlyFillWindowMs = 85` (`src/core/model.h:112,115`). Every E test reads the window the search ran under: `fill_refuses(e_offset, window_ms)` (model.h:331), `is_e0(e_offset, skips, window_ms)` (model.h:335), `allzero_activation` (model.h:381), `Activation::e_window_ms` (model.h:606, stamped at `src/search/engine.cpp:2030`), `HydraRecord::early_fill_window_ms` (model.h:977, stamped at `src/search/pather.cpp:548`). The setting is `Settings::early_fill_window_ms` (`src/app/config.h:195`, key table `config.cpp:201`), part of the store lens (`config.cpp:432`, `record_store.cpp:183`).
- The window's number comes from filed rulings, lines under `## Confirmed` in `docs/agents/rulings/code-and-engine.md`: line 55 "so we should use the hit window. and stop inventing our own numbers." (D344), line 56 "make the early fill threshold an adjustable setting pinned to one number in both directions", line 57 "analysis settings panel, default 60ms, supports 0ms to 85ms". So 60 and 85 are rule numbers, with those lines as their reason. 70 and the typed ticks are the tests' own inputs.
- Question 3 for the E tests: `records-engine/index.md:29` is the only early-fill record: `beg` path 0 "1- 0 0 E0", activation 3, `e_offset` 0, and `records-screen/beg.txt` shows "Early fill: 0.0 ms (required)". It was made at the 85 ms code (`index.md:5`) and `index.md:33` says "Any other E record is retaken once the setting lands." No record exists at window 60 or at 70 ms from a deadline. The needed input for a real check: `hydra_replay dump --efill 60` and `--efill 85` (flag exists, `tools/replay.cpp:229,842`) on one corpus chart with SP ready 0 to 85 ms around a fill deadline, plus the Paths tab capture of its "Early fill" line at each window.
- A comment in these tests quotes "the user's `Add a line` answer" (test_batch_text.cpp:183). I grepped the repo outside this audit folder: no session card, ruling or decision holds that text (`docs/sessions/index.md` has no card for session a3ba79d1). Unverified claim.

---

## test_fills_and_skips.cpp

### 1. Skipped fills: under the 1.1 rule a fill exactly at the reach edge is stored (tests/test_fills_and_skips.cpp:82)

Delta round: changed, replaces A14.md record 25. The old verdict (Rewrite per B13) still holds, and the new lines add a second window.

Read: diff; test lines 82-120 as shown in the diff; `fill_refuses` (model.h:331); `ScoreGraph` constructor parameter (`src/search/graph.h:201`, `graph.cpp:95`); `fill_e_offset` (model.h:329); A14 record 25; rulings lines 55-57. Not read: `run_search` body, `build_fixture_song`, `activation_fill_deadline_ms` body, FL-23 (cited in A14).
1 Claim: Under the 1.1 rule a fill stays offered when Star Power comes ready no later than the early-fill window after the fill's deadline, and is refused one millisecond later; at the 85 ms window the edge is 7285 ms and at the 60 ms default it is 7260 ms.
2 Owner: `fill_refuses` (model.h:331), read by the search at `engine.cpp:862,1007`; the window comes from the graph (`graph.cpp:100`). Engine owns it. The display's copies: the Paths tab prints from the stored `e_offset`. Callers of the window: engine.cpp:859-869,1007,1224,1937,2030.
3 Real chart: claim not checked. Engine: `records-engine/index.md:29` has only `beg`'s E0 at `e_offset` 0; no chart there sits at the edge, at either window. Claim: edge at deadline plus window. Needed: a corpus chart whose SP-ready note is within 1 ms of a deadline plus 60, with `hydra_replay dump --efill 60` and `--efill 85`.
4 Direct: `REQUIRE(deadline == 7200.0)` raw result (working only in the comment above it). `edge = 7285` and `default_edge = 7260` are raw results: deadline plus window as one number, the working sits in a comment (B13 again). The four `CHECK`s read as the claim only with that comment.
5 Reason: 85 = `kHitWindowMs`, 60 = `kDefaultEarlyFillWindowMs`, both named and backed by the filed rulings above. 7285 and 7260 are not named; they should read `deadline + kHitWindowMs` and `deadline + kDefaultEarlyFillWindowMs`. The comment says 7285 is "pinned as a number so a window the graph dropped fails here": that is a reason to pin the window, not to type the sum.
6 Display: not a display test; the stored `skipped_fill_ticks` is what the Paths tab reads.
7 Failure: reach edge wrong by a tick, or the graph ignoring the window it is given (the new `window_ms` argument). Possible: `ScoreGraph` stores the window but "does not read it" (graph.h:195-196), the engine reads `graph.early_fill_window_ms()`; a dropped argument would fall back to 60 and fail the 85 run.
8 Deleted: REQUIRE 7200 and `timecode == ready`: wrong fixture arithmetic unseen (reasoned). `passed_over(edge, 85) == {9600}`: reach too short (reasoned). `passed_over(edge+1, 85).empty()`: reach too long. Same two for 60: each fails if the 60 run is given 85 (reasoned).
9 Stand-in: it leans on 125 BPM giving one tick per ms, checked by the REQUIRE.
10 Same rule: `tests/test_model.cpp` early-fill window test (A14 record 25 item 10) states the edge at function level. One verdict.
11 Real: the edge needs SP ready within 1 ms of a boundary; a real chart that does this is not named.
Verdict: Rewrite. Write the sums from named inputs (`deadline = fill_a_start - 4 beats`, `edge = deadline + window`) for both windows; check one real chart at both windows when the main session dumps it.
Today it catches: reach edge at exactly the window, at two windows, and the graph forgetting the window.
Production finding: none.
Side notes: the helper `lone_fill_activation` was removed with the old tests below and replaced by `early_fill_song` (line 111) and `window_settings` (141).

### 2. search: a fill that needs a 70 ms early hit is taken at an 85 ms window, not at 60 (tests/test_fills_and_skips.cpp:181)

Delta round: changed, replaces A14.md record 26 (the old test, named "a fill that needs a 70 ms early hit is taken as a required early fill"). The old verdict (Rewrite: name 70 as ready minus deadline; claim not checked at 70 ms) still holds, and the test now also types `failed_tick`.

Read: diff lines (test body 181-215 in the file); helpers `early_fill_song` (111), `window_settings` (141), `check_window_carried` (149, 160), `activates_at` (166); `search_graph` (`src/search/pather.h:201`), `search_target` (pather.h:162), `activation_pins` (pather.h:124), `paths_from_pathstring_on_graph` (`src/search/pathstring_read.h:44`), `TargetResult` (pather.h:133-138); `is_e0`, `early_fill_difficulty` (model.h:335,340); old record A14 26; `records-engine/index.md:29`. Not read: bodies of the four search routines, `analyze_chart`.
1 Claim: When Star Power comes ready 70 ms after a fill's deadline, a search with the window at 85 ms takes the fill and stores it as a required early fill of 70 ms, on every route into the engine (analysis, pinned target, path text read back); with the window at the 60 ms default the fill never spawns, so no path activates at it and the path text fails to place.
2 Owner: `fill_refuses` / `is_e0` / `early_fill_difficulty` (model.h:331-340), the engine stamps `e_offset` and `e_window_ms`. Displays (Paths tab "Early fill: N ms (required)", `path_view.cpp:278`) read the stored values. Engine owns it. Callers of the window as in record 1.
3 Real chart: claim not checked. Engine: `beg` activation 3, E0, `e_offset` 0 (index.md:29). Claim: required early fill at 70 ms and no spawn at 60. Different case (0 ms vs 70 ms; one window). Needed: a chart dumped at `--efill 60` and `--efill 85` (see the shared note).
4 Direct: `act_ticks == {9600}` typed tick (fixture grid point). `is_E0()`: the claim. `e_difficulty() == {70.0}`: raw, 70 is ready minus deadline (7270 - 7200) and says so nowhere. `check_window_carried` (every activation and variant carries the window, and the record carries it): a property. `activates_at(narrow, 9600)` false: the claim. `narrow_pin.paths.empty()`: the claim. `failed_tick == {9600}`: typed tick, reads as "the fill's activation was the one that failed". `CHECK_THROWS(paths_from_pathstring_on_graph(narrow_graph, text))`: a throw of any kind passes, so it cannot tell "the fill never spawns" from any other fault.
5 Reason: 70 sits inside 60..85 on purpose so one input splits the two windows; the test says it only in the title. 60 and 85 as the rulings above. 9600 and 7270 are grid points of the fixture (comment at test 1 lines 31-39).
6 Display: the record is checked, not a screen.
7 Failure: the window not reaching one of the three routes (the new test's point: "Every route to the engine is read"). A pathstring read back on a graph built at another window would place a fill the search refused. Possible.
8 Deleted: REQUIRE `!wide.paths.empty()` and `act_ticks == {9600}`: the search not taking the fill (reasoned). `is_E0`: fill taken as optional (reasoned). `e_difficulty == 70`: sign or magnitude (reasoned). `check_window_carried(wide)`: a graph or activation falling back to the default (reasoned). `narrow` lines: the 60 window not refusing (reasoned). `wide_pin.paths.size() >= 1`: pin unrealizable. `failed_tick`: wrong failing window named. `act_ticks(read)`: text not placed at 85. `CHECK_THROWS`: text placed at 60 (but any throw passes).
9 Stand-in: `CHECK_THROWS` stands for "this specific refusal". `paths.size() >= 1` is a count where "the 9600 pin is the one realized" is meant.
10 Same rule: tests 3 and 4 below; `tests/test_model.cpp` E0 window tests (A14 record 26 item 10).
11 Real: `beg` has an E0 at 0; one at 70 ms is not named.
Verdict: Rewrite. Name 70 as `ready - deadline` from the fixture's two ticks; make the throw check name the refusal; check on a real chart at both windows (claim not checked).
Today it catches: the 85 window taking the late fill as E0 with the right figure, the 60 window refusing it, and the window being carried to the record, the pin search and the path reader.
Production finding: none.

### 3. search: SP ready 70 ms before a fill's deadline marks the activation E at 85, not at 60 (tests/test_fills_and_skips.cpp:220)

Delta round: changed, replaces A14.md record 27. The old verdict (Rewrite: l159 expects a value out of the owner) still holds for the 85 branch.

Read: test body (220-247) from the diff; `is_e0`, `early_fill_difficulty` (model.h:335,340); `Activation::is_e_critical`, `is_E0` (`model.cpp:473,475`); A14 record 27.
1 Claim: When Star Power comes ready 70 ms before a fill's deadline, the activation is E-critical and E0 with its early fill listed at 70 ms to spare at a window of 85 ms, and a plain activation with no early fill at the 60 ms default.
2 Owner: `is_e0` (model.h:335) read by `Activation::is_e_critical` and `is_E0` (model.cpp:473,475). Engine; E badge and Paths tab read it.
3 Real chart: claim not checked. Engine: `beg` E0 at `e_offset` 0 (index.md:29); that is inside both windows, so it does not split them. Needed: as the shared note.
4 Direct: `e_offset == 70.0` raw (deadline minus ready). `is_e_critical() == wide`, `is_E0() == wide`, `e_difficulty().has_value() == wide`: the claim stated as a flag per window. `e_difficulty() == early_fill_difficulty(70.0)` at 85: the expected value comes out of production code, a copy (B11), and it also compares a value to its own owner's output.
5 Reason: 70 splits the two windows; named only in the title. `wide` is `w == kHitWindowMs`.
6 Display: record only.
7 Failure: the E mark's edge reading the 60 default or a fixed constant instead of the setting (this test's point; commit not read). Possible.
8 Deleted: `e_offset == 70`: mis-stamped offset (reasoned). The three booleans: E dropped at 85 or added at 60 (reasoned). `has_value`: figure listed at 60. The last `CHECK`: figure sign flipped, only if the owner changes unevenly. `check_window_carried`: as test 2.
9 Stand-in: the last `CHECK` is a copy.
10 Same rule: tests 2 and 4; `tests/test_model.cpp` E0 tests.
11 Real: no record chart has the case.
Verdict: Rewrite. Take 70 from `deadline - ready`; replace the last `CHECK` with the figure `-e_offset` written out, or drop it; check on a real chart at both windows.
Today it catches: E mark inside 85 and not inside 60, with the window carried.
Production finding: none.

### 4. all-0 pass: passing over a fill with 70 ms to spare is an E1 at 85, a skip at 60 (tests/test_fills_and_skips.cpp:249)

Delta round: new.

Read: test body (249-293) from the diff; `early_fill_song` second-fill branch (111-133); `attach_allzero`, `search_allzero`, `allzero_options` (`src/search/pather.cpp:46-107`, 69-78); `allzero_activation` (model.h:381); `Path::is_allzero` (`model.cpp:855`); `all_allzero_paths` (model.cpp:895) and its one `src/` caller `path_view.cpp:526`; D85/D102 as quoted in model.h comments (not opened).
1 Claim: Passing over a fill with 70 ms to spare counts as the player's early fill (an E1, so the path stays all-0) when the window is 85 ms, so the best all-0 path activates at the later, better fill (tick 19200); at the 60 ms default it is a plain skip, so the best path is no longer all-0 and the all-0 pass's own answer activates at the first fill (tick 9600).
2 Owner: `allzero_activation` (model.h:381), read by `Path::is_allzero` (model.cpp:855) and by the all-0 search through `no_skips` (engine). Display: the Paths tab's all-0 section (`path_view.cpp:526`). Engine owns it.
3 Real chart: claim not checked. No record chart shows an E1 or an all-0 answer under either window. Needed: a corpus chart with an activation that passes over exactly one fill within 85 ms of SP ready, dumped at `--efill 60` and `--efill 85`, plus the Paths tab's all-0 section for each.
4 Direct: `act_ticks == {19200}` and `{9600}`: typed grid ticks. `is_allzero()`, `within_ms_limit(allzero_limit)`: the claim. `wide.allzero_paths.empty()` and `narrow.allzero_paths.front()`: depend on how `attach_allzero` stores the answer (the test's own comment, 237-240, says so). `allzero_limit` is read from `allzero_options()`, not typed (good).
5 Reason: 70 splits the windows; the 19200 fill and its three notes (19440, 19920, 20400) are chosen so a later activation scores more (comment at `early_fill_song`). Named in the comment, not in the constants.
6 Display: record only; the section the user sees is `path_view.cpp:526`, not read.
7 Failure: the all-0 pass reading a fixed window instead of the setting. Possible; `allzero_activation` now takes the window.
8 Deleted: `act_ticks == {19200}`: best path not passing A over (reasoned). `is_allzero()`/`within_ms_limit`: E1 not counted at 85 (reasoned). `allzero_paths.empty()`: the all-0 pass redoing a path already first (reasoned). `narrow ... !is_allzero()`: skip counted as E1 at 60. `allzero_paths.front()` at 9600: pass answer missing or wrong.
9 Stand-in: `wide.allzero_paths.empty()` pins how the answer is stored, not what it is. A `HydraRecord` accessor for "the all-0 answer" does not exist (grep `allzero_answer|best_allzero|all_allzero_paths` in `src`: only `all_allzero_paths`, which returns the list, not the answer). The two-way comment shows the display reads two places for one answer. Searched: src (`model.cpp:873-895`, `path_view.cpp:526`, `pather.cpp:46`).
10 Same rule: tests 2 and 3 for the window; `test_model.cpp` allzero tests for `allzero_activation`.
11 Real: one E1 per chart is common in dumps ("E1" in pathstrings); the 70 ms split is not shown.
Verdict: Rewrite. State the all-0 answer through one reader and not through two storage places; name 70, 9600 and 19200 from the fixture's named inputs; check on a real chart at both windows.
Today it catches: the all-0 section following the window (the new behaviour).
Production finding: the all-0 answer is stored in two places (`record.paths` best or `record.allzero_paths`, `pather.cpp:53-56`) with no reader that returns "the answer"; the display and tests each re-derive it. A split to the main session, not a verdict.

### Removed: lone_fill_activation helper and the two old test names

Delta round: removed. `lone_fill_activation(int64_t)` (A14 record 26, 27's shared helper) was replaced in the same commit range by `early_fill_song` + `window_settings`; the tests 2 and 3 above carry the old records on. The commit that did it: find with `git log c10c7f54..5ec682c7 -- tests/test_fills_and_skips.cpp` (not run: the diff was read instead, commit hash not looked up).

---

## test_batch_text.cpp

### 5. batch text: the confirm's Early fill window line reads the window in ms (tests/test_batch_text.cpp:185)

Delta round: new.

Read: diff; `batch_settings_summary` (`src/ui/library_dialogs.cpp:70-80`, 350-363); `describe_settings` (`src/app/config.cpp:462-468`); `format_ms_whole` (name only). Callers of `batch_settings_summary` in `src/`: `library_dialogs.cpp:350` (the confirm), `library_parts.h:79` declaration; the CLI uses `describe_settings` directly (`src/cli/batch.cpp:145-152`).
1 Claim: The confirm that opens before a batch lists the Early fill window as "60 ms" by default and "85 ms" when set to 85.
2 Owner: `describe_settings` (config.cpp:467) is the one wording owner, read by the confirm, the settings button (`library_dialogs.cpp:100`) and the CLI header. The test calls `batch_settings_summary`, which copies `values.early_fill` (library_dialogs.cpp:80). A display over an engine setting. Fine.
3 Real chart: claim not checked: this is a dialog's text. Capture needed: the confirm dialog of "Analyze library" at default settings and with the window at 85. No chart is involved.
4 Direct: two typed strings "60 ms" and "85 ms". 60 is the default from the ruling (line 57), 85 the setting's maximum; but the typed text is the whole expected output.
5 Reason: 60 and 85 as in the shared note; the comment's "user's 'Add a line' answer" is unverified (see shared note).
6 Display: the expected side is typed text. The owner `describe_settings` could have been called with the same settings; it was not.
7 Failure: the confirm missing the window or reading the wrong field. Possible.
8 Deleted: line 1 (default): confirm printing another field or none (reasoned). line 2: window not read from the settings (reasoned).
9 Stand-in: none.
10 Same rule: the CLI header test (test 7, "Early fill : 60 ms") and `tests/test_config.cpp` `describe_settings` tests (name only, not read) state the same wording.
11 Real: yes, a player reaches the confirm at the default.
Verdict: Investigate. Claim not checked only: a capture of the confirm at both values is needed. The typed text is the same wording style as the sibling Fills test (A8 record 10, also Investigate).

### 6. batch text: the Early fill window help text (tests/test_batch_text.cpp:194)

Delta round: new.

Read: diff; `early_fill_window_help_text` (`src/ui/settings_bar.cpp:35-42`); its callers (`settings_bar.cpp:156`, declaration `library_parts.h:53`); `fill_refuses`, `is_e0` (model.h:331,335); `hit_window_text` (name only); A8 record 11 (the sibling test).
1 Claim: The tooltip of the Early fill window row states four rules: ready up to the window after a deadline still summons the fill, later never spawns, ready less than the window before it marks the activation E, at 0 no activation is an early fill, and the hit window (85 ms) is the most the setting allows.
2 Owner: the sentences are display wording over `fill_refuses` (inclusive at `-window`), `is_e0` (strict `< window`) and `kMaxEarlyFillWindowMs`. I checked each sentence against them: "up to this many ms after" matches `e_offset < -window` refusing; "less than this many ms before" matches `e_offset < window`; "At 0 no activation is an early fill" holds because `e_offset < 0` is the refused side; "85 ms" is `hit_window_text()`. The test's own comment names `fill_refuses` and `is_e0` as the source. Only the settings panel calls the text.
3 Real chart: claim not checked. Screen needed: the tooltip on hover in the settings panel (the UI test at record 17 hovers it). The rule side was checked against the model.h functions, not on a chart.
4 Direct: one string typed whole, 5 sentences; "85 ms" typed inside it, not read from `kMaxEarlyFillWindowMs` or `hit_window_text()` (the UI test at record 17 does read `hit_window_text()`).
5 Reason: the rules are `model.h:321-340`. The whole-sentence equality is the same stand-in A8 record 11 found for the 1.0 tooltip.
6 Display: the expected string is typed, not built from the owners.
7 Failure: the tooltip describing the old fixed 85 ms reach, or the 0 case. Possible.
8 Deleted: the one equality: any wording change fails it (a snapshot of text); it cannot say which rule broke.
9 Stand-in: whole-sentence equality for "each sentence states a rule".
10 Same rule: A8 record 11; `uitest_library.cpp:342` (record 17) checks the 85 ms text.
11 Real: yes.
Verdict: Rewrite. One check per rule, each tied to its owner: build the 85 from `kMaxEarlyFillWindowMs`, and check the other four sentences by name (contains), as A8 record 11 asked for the sibling. Claim not checked on the screen.
Today it catches: any wording change of the tooltip.
Production finding: none.

---

## test_cli.cpp

### 7. hydra_batch fills a new database under the rule it ran under (tests/test_cli.cpp:183)

Delta round: changed, replaces B1.md record 1. One line was added: `CHECK(contains(r.output, "Early fill : 60 ms"))` (line 195). The old verdict (Rewrite: compare each header line with `describe_settings`) still holds, and the new line is one more typed word.

Read: diff; `src/cli/batch.cpp:145-152` (prints `described.early_fill`); `describe_settings` (config.cpp:462-468); B1 record 1.
1 Claim: The batch tool's header prints the early-fill window in the app's words, "Early fill : 60 ms" at the default.
2 Owner: `describe_settings` (callers: `cli/batch.cpp:145`, `library_dialogs.cpp:65`, `tools/bench.cpp` per B1). The test types the text. Engine/app owns the words; the console is a display.
3 Real chart: the whole test runs the real exe on a corpus `.chart`; the new line is checked on that real run against typed words, not against the owner. Claim as typed: the header says 60 ms; owner: `Settings{}.early_fill_window_ms` = `kDefaultEarlyFillWindowMs` (60, ruling line 57). Agrees on the day.
4 Direct: new line: typed "Early fill : 60 ms". Rest as B1 record 1.
5 Reason: 60 is the filed default (ruling line 57), but the line types a result of two functions, not 60 with its source.
6 Display: Fails as in B1 record 1: console text typed where `describe_settings` is the owner.
7 Failure: the header dropping or mis-reading the new setting. Possible.
8 Deleted: new line: header missing the field (reasoned); also fails on any legitimate change of the default (snapshot).
9 Stand-in: typed words for "same words as the app".
10 Same rule: test 5 above; B1 records 3, 12, 16 type the same header lines.
11 Real: yes.
Verdict: Rewrite, unchanged from B1 record 1: compare each header line with `describe_settings` output for the same settings.
Production finding: none.

### 8. hydra_replay refuses --ms, --efill and --depth values that are not whole numbers (tests/test_cli.cpp:636)

Delta round: changed, replaces B1.md record 19 (the old test, named "refuses --ms and --depth values..."). The old verdict was Investigate (exit code 2 and wording have no decision beyond D308 item 1; 99999999999 has no stated meaning). It still holds, with two more rows.

Read: diff; `tools/replay.cpp:184` (`setting_flag_number`), 224-229, 842; B1 record 19. D308 not re-opened (B1's reading kept).
1 Claim: `hydra_replay dump` stops with exit 2 and a line naming the flag for `--efill abc` as for `--ms abc`, `--depth 1x` and an out-of-range `--depth`; whole numbers (`--efill 85`, and `--efill 900`, pulled to the range's edge) still run.
2 Owner: `setting_flag_number` (replay.cpp:184), callers `replay.cpp:224,229,841`. A tool's argument check; no `src/` caller.
3 Real chart: runs the real `hydra_replay` on a corpus chart copy. The claim checked on real input against the owner's message form (typed copy, not the owner). Agrees as typed.
4 Direct: exit 2; typed lines (the message form copied from replay.cpp); 0 for accepted values. New rows: `--efill abc` refused with the typed line; `--efill 85` and `--efill 900` exit 0. 900 stands for "past the range"; the comment says "A number past a setting's range is no typo: settings_from pulls it in" but nothing here reads the pulled-in value, so 900 and 85 differ in no check.
5 Reason: 85 = `kMaxEarlyFillWindowMs`, 900 = free input for "past the range", named only in a comment; neither read from `config`.
6 Display: typed copy of a tool message.
7 Failure: `--efill abc` silently meaning 0 (D308's N-0285 shape). Possible.
8 Deleted: each typed line and exit 2: reasoned. `--efill 900` exit 0: reasoned (a refusal of out-of-range values would make a clamped setting fail).
9 Stand-in: exit 0 for "it was applied"; the efill value reaching the search is not checked (`dump` output's `early_fill_window_ms` field exists on the record, model.h:977, not read here).
10 Same rule: `tests/test_replay_schedule.cpp` greps replay's source (B1 record 19, not read).
11 Real: yes.
Verdict: Investigate, as B1 record 19. Add: the new rows do not check that the number reaches the search.
Production finding: none.

---

## test_activation_rows.cpp

The helper `analyzed_like_the_gui` (test_activation_rows.cpp:1221-1232) changed: it now sets `settings.early_fill_window_ms = kHitWindowMs` (85) before analysing, with the comment "the window these rows' paths and figures were pinned under (Get Ur Freak On's E marks need it)". Its callers are the seven tests below (grep `tapped_out()|focus()|epidermis()|analyzed_like_the_gui` in the file).

Helper judgement: it does what its callers rely on (a corpus chart analysed once, at 85 ms), but its name is false. The GUI's default window is 60 ms (`config.h:195`, `Settings{}`), so "like the gui" is no longer true for these charts. A comment above it says "analyzed the way burnout() analyzes it (song_details_fixtures.h)"; I did not open `song_details_fixtures.h`, so whether `burnout()` also pins 85 is not read. Finding (split): the screens a player sees at the default 60 ms window may show different E marks, paths and rows on these same charts than the figures these tests type; the test says Get Ur Freak On's E marks "need" 85. A capture of the Paths tab on Get Ur Freak On at 60 and at 85 is needed. Reached by: all seven tests below.

Shared read for 9 to 15: lines 1221-1236 (helper), 1304-1475 and the diff; the old A3 records 14-19, 20-21 (verdicts read through grep). Display code: `build_activations` and window rows (`src/app/path_view.cpp`, not opened). Shared Q3: the charts (Tapped Out, Focus, Epidermis, Get Ur Freak On, Burnout) are not among the seven in `records-engine/`; claim not checked, charts needed, with `hydra_replay schedule` at both windows. Shared Q6: every row types figures from `hydra_replay schedule` ("The figures are the ones hydra_replay schedule prints", test comment 1209) instead of reading the engine's record.

### 9. dynamic hit window: Tapped Out's squeeze out is out of reach (tests/test_activation_rows.cpp:1308)

Delta round: changed (helper), replaces A3.md record 14. The old verdict (Investigate: split on "out of reach", needs Tapped Out's schedule) still holds. Does the window matter for it? `tapped_out()` activation 4's squeeze row; the window changes which paths are best, so the pinned path may differ at the default; at 85 it is as pinned.
Claim: Tapped Out's best path's fourth activation shows a squeeze out whose window row reads "frontend 77.9 ms [K G] backend 72.0 ms [ R B ]" tagged "65.9 ms short". Q4: typed figures (raw results). Q6: fails, expected typed. Q8: any row figure change. Verdict: Investigate (as A3 14): the engine's feasibility answer was not compared; chart needed.
Production finding: none new.

### 10. dynamic hit window: a squeeze in with a timing names its anchor the frontend (tests/test_activation_rows.cpp:1354)

Delta round: changed (helper), replaces A3.md record 16 (Remove: the corpus test at 1604 checks the same promise on every chart). Old verdict still holds. The subcases use `focus()` and `epidermis()` and `REQUIRE(pathstring == "0 3+ 0 0")` for Focus: a pathstring that depends on the window, now pinned at 85 by the helper.
Claim: a squeeze-in with a timing names its anchor first as the frontend and the squeezed note as the backend, with typed figures (61.9, 48.1, 85.1, 48.1, 67.8, 184.1 ms). Q3: claim not checked. Q4: all typed raw figures. Q6: fails. Verdict: Remove (as A3 16; the typed ms go with records 9 and 11).

### 11. dynamic hit window: an early fill gets one window, required or optional (tests/test_activation_rows.cpp:1396)

Delta round: changed (helper), replaces A3.md record 17 (Rewrite: take the row's end from the record's SP-ready chord, add an optional-fill case). Old verdict still holds, with a stronger reason: this test is about the early fill, so the window it is run under changes its subject. At the helper's 85 ms, activation 3 is "Early fill: 0.0 ms (required)" and activation 4 "(optional)", typed. Whether either exists at the default 60 ms was not checked.
Q3: Engine: `beg` activation 3 "E0 m83.1.0 2 bars early fill 0.0 ms" (`records-screen/beg.txt`), at 85. Claim: Tapped Out's activation 3 reads the same 0.0 ms (required). Same kind of thing, different chart; the 85 ms record is a record of the code before the setting. Verdict: Rewrite (as A3 17). Add: run it at the default window or say why only 85 is meant.

### 12. dynamic hit window: a squeeze in that needs no timing still gets its row (tests/test_activation_rows.cpp:1439)

Delta round: changed (helper; also the only caller that passes a name: "Eric Moore - Get Ur Freak On"), replaces A3.md record 18 (Rewrite: use `late_sqin_twice` activation 1 and the record's squeezes). Old verdict still holds. The `REQUIRE(best.pathstring() == "5- 2+ E2- E0 E0")` depends on the window: its comment says "Activations 3 to 5 carry an E because Star Power comes ready less than the early-fill window before each one's fill deadline (is_e0 owns the window)". At 60 ms the E marks may not appear, which is why the helper pins 85. So this test's whole input is a window the app does not default to. Claim not checked (no record). Verdict: Rewrite. Production finding: none.

### 13. dynamic hit window: a row for every timing badge, edited figures too (tests/test_activation_rows.cpp:1469)

Delta round: changed (helper), replaces A3.md record 19 (Rewrite: replace the typed 102.9 with the sum from its parts; take cases from Beg and Believer). Old verdict still holds. Subcases at 1471 (`focus()`), 1504 and 1524 (`tapped_out()`). Claim not checked on a chart.

### 14. dynamic hit window: no chord times, no row anywhere (tests/test_activation_rows.cpp:1543)

Delta round: changed (helper; loops `tapped_out(), burnout(), focus(), epidermis()`), replaces A3.md record 20 (Investigate: does any screen call with no chord times?). The old verdict still holds, and the window change does not touch the question.

### 15. dynamic hit window: the row sits alongside and changes no other field (tests/test_activation_rows.cpp:1556)

Delta round: changed (helper; same four charts), replaces A3.md record 21 (Rewrite: drop the floor, use named charts). Old verdict still holds. The loop reads four charts at 85; a property over every field of every row, which is a good shape; the floor and the silent skip remain as A3 21 said (lines not re-read).

---

## test_docs_match_code.cpp

### 16. docs match code: every marked default equals the code's value (tests/test_docs_match_code.cpp:472)

Delta round: changed, replaces A11.md record 2. `known_defaults()` gained two entries: `Settings::early_fill_window_ms` (read from `Settings{}`) and `kDefaultEarlyFillWindowMs` (lines 328-329, 333-334). The old verdict (Rewrite: the test cannot see unmarked defaults) still holds.

Read: lines 308-413, 472-484; `docs/UserGuide.md:135,266`, `CONTEXT.md:613` and `docs/clone-hero/04-fills.md:244` (marker lines found by grep with `-o`, the numbers beside them not opened); `model.h:112`; `config.h:195`; A11 record 2.
1 Claim: every number written next to a `<!-- default: NAME -->` marker equals the code's value for that name, and every marker the test knows is used by some doc; now including the early-fill window default of 60 ms in the User Guide, CONTEXT.md and the Clone Hero fills page.
2 Owner: `Settings{}`, `kDefaultEarlyFillWindowMs` (model.h:112), each read from production. The two new entries are two names for one owner: `Settings::early_fill_window_ms` is initialised from `kDefaultEarlyFillWindowMs` (config.h:195), so the table holds one rule twice. Not a split (they cannot disagree), but a second name.
3 Real chart: not a chart claim. Checked on real input: the real docs (UserGuide.md, CONTEXT.md, 04-fills.md) against the owners. Not run; the number beside each new marker I did not open.
4 Direct: `problems.empty()` a property; each problem names doc, line, marker and both values.
5 Reason: no typed numbers; 1000 cap as A11.
6 Display: the docs are the display; expected side is the owners.
7 Failure: as A11 record 2.
8 Deleted: as A11 record 2, plus: if a doc's early-fill default stayed at the old value 85 the test would go red (reasoned).
9 Stand-in: as A11 record 2 (unmarked defaults are a silent skip).
10 Same rule: the marker-check test at line 486 (record 17).
11 Real: yes.
Verdict: Rewrite, as A11 record 2 (the missing half: tie every Default cell to an owner). The new entries need nothing more.
Production finding: none.

### 17. docs match code: the marker check catches a wrong number and a wrong word (tests/test_docs_match_code.cpp:486)

Delta round: changed only through its helper (`check_markers` calls `known_defaults()`, whose table grew). The test's own lines are unchanged. The old record is A11.md (the record whose heading names this test; I did not find it by name in this audit's search and did not open it, so I do not say whether its verdict holds). Not changed in effect: its four expected problems come from names that did not change (`Settings::mslimit_value`, `mslimit_enabled`, `no_such_default`, `kStarMultipliers[1]`). Read: lines 486-504. Verdict: not judged beyond this; question 1 to 11 not asked, because no line or table entry it reads changed. Listed as not reached for a full record.

---

## test_engine_sweeps.cpp

### 18. corpus: squeeze-ins at exactly 0 and what they change (tests/test_engine_sweeps.cpp:581)

Delta round: changed, replaces A13.md record 10. Three snapshot totals moved: `saved_negative` 102 to 90, `allzero_charts` 68 to 69, `allzero_paths` 69 to 70 (lines 664, 666, 667). Old verdict (Rewrite per B6: drop the totals, keep "every saved row shows the timing the engine worked out") still holds, and the three numbers moving is itself the proof.

Read: lines 575-668; the diff; A13 record 10 (lines 387-423). The analysis settings are `Settings().to_analysis_settings()` (line 582): now the 60 ms default, since `to_analysis_settings` passes `early_fill_window_ms` (config.cpp:411). The earlier numbers were taken at 85.
1 to 11 as A13 record 10, with this change: Q4 and Q5: nine typed totals (98, 56, 56, 5, 374, 90, 79, 69, 70); each is a snapshot. Three of them changed because a setting default changed from the window the corpus was read at; nobody could say why 90 is right and 102 was not: it is "what the code printed once". This is the rubric's own opening example. Q9: the corpus count (98) is also a snapshot of how many charts the corpus has.
3 Real chart: claim not checked: the library column was not captured (A13). Needed: the library row of a corpus chart with a squeeze-in at 0.
Verdict: Rewrite per B6 (as A13 record 10). Drop all nine totals, keep `shown_differs.size() == 0`.
Today it catches: shown timing differing from the engine's own figure (kept); any count change (dropped on purpose).
Production finding: none.

### 19. settings_key: the text for SearchSettings{} is pinned, and every field moves it (tests/test_engine_sweeps.cpp:745)

Delta round: changed, replaces A13.md record 14. `settings_key` now includes the early-fill window (corpus_util.h:227); the expected text became "4|0|4|none|0|60|12303881770575177486|", and `changed[6].early_fill_window_ms = 85.0` joined the list. The old verdict (Rewrite: derive the field list, drop the literal key) still holds.

Read: lines 741-767; `settings_key` (`tests/corpus_util.h:220-228`); `corpus::analyzed` (242); the rule file `concept-G5.json`; `SearchSettings` (`src/search/pather.h:45` early_fill_window_ms; other fields not read); A13 record 14.
1 Claim: the test cache's key for the default settings is one fixed text, and changing each of seven listed fields changes the key.
2 Owner: `corpus::settings_key`, a test helper; callers: `corpus::analyzed` (corpus_util.h:242) and this test. The app's own key is `store::Lens` (config.cpp:426), which includes the window; the test's key is a second encoder of the same question ("which settings change a stored analysis"), and the rule file `concept-G5.json` names it the cache's own, not the app's.
3 Real chart: test infrastructure; claim not checked on real input (as A13 record 14).
4 Direct: the default key text typed (snapshot; 60 now written into it with a comment "default early-fill window"; 12303881770575177486 is the rules fingerprint). The `!=` loop: a property.
5 Reason: 60 = default (ruling line 57). The fingerprint number has no reason here. Seven fields by hand still stand in for "every field" (A13).
6 Display: no.
7 Failure: two settings sharing a cached analysis. This commit's case: the window entered the key, otherwise tests that change the window would read the 60 ms result. Still possible.
8 Deleted: key text equality: key shape drift (snapshot). `changed[6]` line: window left out of the key (reasoned). Others as A13.
9 Stand-in: seven hand-listed fields for "every field".
10 Same rule: `concept-G5.json` (record 24) lists the same fields by regex; `store::Lens` tests (not read).
11 Real: yes.
Verdict: Rewrite, as A13 record 14.
Production finding: none.
Split: the test cache key (`corpus::settings_key`) and the app's key (`store::Lens::from`, config.cpp:426) answer "which settings change a stored analysis" separately; this change added the window to both by hand. The rule file `concept-G5.json` names the test one as owner and says the app files under `store::Lens`.

---

## test_preview_controller.cpp

### 20. a cancelled click job stops before counting (tests/test_preview_controller.cpp:361)

Delta round: changed, replaces A21.md record 7. The job now takes a store (`ViewJob(..., store)`, `library_jobs.cpp:662`), so the test builds a real `StoreFile` and `RecordStore` before the job (lines 366-369 in the file) with the comment "Before the job, which writes to it on its thread and must not outlive it". The old verdict (Rewrite: the lines are true by construction; check no parse happened) still holds.

Read: lines 357-373 from the diff; `ViewJob::ViewJob`, `run`, `write_results` (`src/ui/library_jobs.cpp:662-791`); `StoreFile` (`tests/store_file_fixtures.h:22-33`); A21 record 7.
1 Claim: a click job cancelled before it starts stops before it counts anything, and ends not ok and cancelled.
2 Owner: `ViewJob::run` (library_jobs.cpp:686). Its cancel checks: `throw_if_cancelled` at 708, 727 and 759/779. Caller in `src/`: `app_state.cpp:412`. Note: `run` reads the chart file's hash first (`chart_changed_since`, line 697-706) before the first cancel check at 708, so a cancelled job still reads the file; "stops before decoding or counting" is true only from line 708 on.
3 Real chart: runs a real thread and the first corpus `.chart`; a real store file. Claim and engine agree in outcome (`ok` false, `is_cancelled` true); whether anything was counted is not observed. Real mechanism checked only for the flag.
4 Direct: `CHECK_FALSE(job.ok())`, `CHECK(job.is_cancelled())`. Neither shows that nothing was parsed or counted.
5 Reason: no numbers.
6 Display: no.
7 Failure: as A21 record 7 (D87 item 1 does not say what the comment says).
8 Deleted: `is_cancelled()`: true by construction (`cancel()` sets it). `ok()` false: a job that ran everything and then returned Cancelled also passes (reasoned). The store the test now builds is not read after the job, so nothing checks that the job wrote nothing to it, which was the new reason to build it.
9 Stand-in: both checks stand for "stopped before counting". The new store would let the test check `record_count() == 0`; it does not.
10 Same rule: A21 record 6 (cancelled preview load).
11 Real: yes.
Verdict: Rewrite. Check the store holds no record after the job (the fixture is now there) and that no progress step was taken.
Production finding: none.

---

## tests/ui (changed bodies; `hydra_uitest` drives these, not run)

### 21. test_settings_lock (tests/ui/uitest_details.cpp:1289)

Delta round: changed, replaces A39.md record "test_settings_lock". The old verdict (Investigate: claim not checked only, capture of the settings panel during a batch needed; five-control list stands for "every control") still holds. Three lines added: `##earlyfillwindow` enabled with no batch (line ~1311 in the diff), disabled during a batch (~1336), enabled again after (~1356). The list of greyed controls grows to six; Q9 the same stand-in.
Read: diff; `render_rules_group` (`settings_bar.cpp:133-169`: the input sits between `begin_disabled_input(locked)` and `end_disabled_input(locked)`). Q2: `AppState::settings_lock` owns the state; `settings_bar.cpp:158-167` draws. Q3: claim not checked; a capture of the panel during a batch with the new row greyed. Q8: the three new lines each fail if the row ignores `locked` (reasoned). Verdict: Investigate, claim not checked only (screen capture of the settings panel during a batch).
Production finding: none.

### 22. test_settings_reset (tests/ui/uitest_library.cpp:298)

Delta round: changed, replaces A41.md record 7. The old verdict (Investigate: claim not checked only, needs a capture before and after the click) still holds; the test now sets a third change (`##earlyfillwindow` = 85) and expects the label "Analysis settings: Pro Drums off \xC2\xB7 SP cap 5 bars \xC2\xB7 Early fill window 85 ms", then `Analysis settings: defaults` and that the window equals `Settings{}.early_fill_window_ms` live and in the saved ini (lines 61-69 of the diff: `defaults` is a real `app::Settings`, an owner read, good).
Read: diff; `library_dialogs.cpp:100` (`AnalysisSetting::EarlyFillWindow` text "Early fill window " + `values.early_fill`); `settings_bar.cpp:156-167`; A41 record 7. Q4: the typed label (display string built from three settings) is the same typed-label fault A41 gave; the new default checks read the owner. Q5: 85 is `kMaxEarlyFillWindowMs`, 5 and 85 free inputs chosen to differ from defaults. Verdict: Investigate, claim not checked only: capture needed of the settings button label with three changes, then "defaults".
Production finding: none.

### 23. test_settings_early_fill (tests/ui/uitest_library.cpp:342)

Delta round: new.

Read: diff (the whole test, 342-385); `early_fill_window_help_text` (`settings_bar.cpp:35-42`), `kEarlyFillWindowName` (name only), `hit_window_text` (name only); `Settings::clamp` (name only: the range is the key table's, `config.cpp:201`, 0 to `kMaxEarlyFillWindowMs`); `render_rules_group` (settings_bar.cpp:133-169).
1 Claim: In the settings panel, the "Early fill window" row shows a (?) whose tooltip is the help text, which names the hit window; typing 85 sets the setting to 85 and the settings button reads "Analysis settings: Early fill window 85 ms"; -5 lands on 0 and 200 lands on 85; the value is saved to the ini after the panel closes.
2 Owner: `Settings::clamp` and the key table (config.cpp:201) own the range; `describe_settings` the label; `early_fill_window_help_text` the tooltip. All read by the screen; the test reads the real widgets and `h.app->settings`.
3 Real chart: this is a screen claim. Claim not checked: capture needed of the settings panel with the Early fill window row at 85 and at 0, and the tooltip open. Not a chart claim.
4 Direct: the help text contains `hit_window_text() + " ms, the hit window"` (reads the owner for the number: good). The (?) hover position is computed from `CalcTextSize` and `ItemInnerSpacing`: a copy of the layout code (`begin_row`) as the test's own comment says; a layout change fails it for no behavioural reason. `on_screen(h, help)` the tooltip is drawn. Typed labels "Analysis settings: Early fill window 85 ms" and "... 0 ms". `early_fill_window_ms == 85` after 85, `== 0` after -5, `== 85` after 200: 85 is `kMaxEarlyFillWindowMs` typed, -5 and 200 free inputs below and above. `load_file(ini).early_fill_window_ms == 85` after `flush_settings`.
5 Reason: 85 and 0 are the range's ends (ruling line 57 "supports 0ms to 85ms"; filed). -5 and 200 stand for "outside the range"; the comment says so.
6 Display: labels typed; the setting is read from the app state, which is the engine side.
7 Failure: the box not clamping, or the setting not reaching the saved file. Possible.
8 Deleted: help contains the hit window: tooltip drops the cap (reasoned). `on_screen(help)`: tooltip not shown (reasoned). Each `early_fill_window_ms ==` line: clamp fault (reasoned). Last line: not saved.
9 Stand-in: none besides the layout copy.
10 Same rule: `test_config.cpp` clamp tests (name only, not read) for `Settings::clamp`; record 6 for the help text.
11 Real: yes.
Verdict: Investigate. Claim not checked only: needs a capture of the panel row (value 85, tooltip open). The hover position copies layout code; if the capture is made the same pass should replace it with a widget id.
Production finding: none.

### 24. test_command_bare_ref_ambiguous (tests/ui/uitest_live.cpp:537)

Delta round: changed, replaces A43.md record L9. Old verdict (Rewrite: check the reply is a refusal and that the N names listed equal N, with no panel count typed) is partly met: the count 3 is no longer typed (the settings panel now has more number inputs than before: with `##earlyfillwindow` there are four "-" buttons in the panel, `settings_bar.cpp:144,160` and others, plus `paths_tab` not in this panel). The test now checks the head `refused: "-" matches ` and that the tail ` widgets, and **/- would take only one of them; name one with a longer ref: "-" id=0x` appears somewhere. It still does not check that the number matches the names listed, or that it is at least two.
Read: diff; A43 record L9. Q3: claim not checked: a live transcript of `click -` with the panel open (A43). Q8: `substr(0, head.size()) == head` the reply starts as a refusal; `find(tail)` the wording after the number. A reply "matches 1 widgets" or "matches abc widgets" passes. Q9: the number is now ignored, so the stand-in is worse in one way and better in another (no unrelated break when a panel gains an input). Verdict: Rewrite, as L9: read N from the reply, require N >= 2 and N listed `id=0x` entries.
Side note: the same change shows the old record's point: adding one number input made the old test fail for an unrelated reason (the count went from 3 to 4); the new comment confirms it.

---

## Helpers and rule file

### 25. corpus_util.h: settings_key (tests/corpus_util.h:220)

Delta round: new function body line (changed). Judgement: it adds `s.early_fill_window_ms` to the key text, between `legacy_fills` and the rules fingerprint (line 227). It does what `corpus::analyzed` relies on (distinct settings, distinct cache entries) for the window. Callers: `corpus::analyzed` (corpus_util.h:242, used by about 30 test files; `Grep` count 100 occurrences of `settings_key|digest_settings|corpus::analyzed(` across 32 files) and `test_engine_sweeps.cpp:747,763` (record 19). Gap: the key lists fields one by one, so a later `SearchSettings` field is silently left out (A13 record 14). Not read: `SearchSettings` fields other than the window.
Split: see record 19 (test key vs `store::Lens`).

### 26. song_digest.h: digest_settings (tests/song_digest.h:327)

Delta round: changed. It now sets `s.early_fill_window_ms = kMaxEarlyFillWindowMs` (85), with the comment: "The window is the hit window, the one the digests were pinned under, stated here so they keep describing that search whatever the setting's default is." Judgement: it does what its callers rely on (the pinned digests stay at the window they were captured under), but those digests are saved expected output pinned from one run (`testdata/parse_edge/expected_expert.tsv`, read by `test_five_lane.cpp:290-297`), and `docs/agents/rulings/code-and-engine.md:50` reads "we don't need golden anymore, remove it all". So the helper's job is to keep a snapshot green; it is not a fault of the helper alone, but each caller inherits the digest-file question. Callers: `test_five_lane.cpp:293` (test "five-lane: four-lane charts parse to their pinned digests", line 287), `test_perf_digest.cpp:37` (a helper, used by the digest tests at lines 128-164), `test_song_parse.cpp:1355` (a helper; its test cases not found by name, not opened). Not read: the callers' test bodies beyond `test_five_lane.cpp:285-304`.
Finding (display/test split): the helper's comment says it is "built the way the app builds them from an ini that sets only these three things", while the app's default window is 60; so digests are pinned at a window the app does not default to. Same shape as the `analyzed_like_the_gui` finding above.
Not full-judged: digest tests (the five-lane digest test, the perf-digest and song-parse digest tests) are outside the diff (their lines did not change), so they have no delta record; the helper changed under them.

### 27. tests/single_owner/rules/tests/concept-G5.json

Delta round: changed. The pattern gained `\bs\.early_fill_window_ms\s*<<` and `must_match` gained a second line, `k << s.early_fill_window_ms << '|';`. The `owner_lines` text was updated to the new `settings_key` line. Judgement: the rule asks "which settings change a stored analysis?" and names `corpus::settings_key` the owner; a second place that streams one of the three fields is the thing it flags. `must_match` lines match the pattern (the window alternative matches both); `must_not_match` lines contain none of the three streams. Not read: `tests/single_owner/engine.h` (how `must_match` and `line_text` are checked) and the `search/concept-G5.json` sibling's text. Does not check: that `settings_key` includes every field (that is test 19). Note: the rule's note says "The app files a stored analysis under store::Lens; this text key is the test cache's only", which is the split named in record 19.

### 28. tools/test_compare_db.py: record tables with efill_window_ms

Delta round: changed. `RECORD_SCHEMA` gained `efill_window_ms INTEGER NOT NULL` in the profiles table and in its UNIQUE key; `PROFILES` gained a 60 in each tuple; `add_records` inserts the column. Judgement: the fixture is a hand copy of the store's `profiles` table, and must match `record_store.cpp:183` ("... legacy_fills, efill_window_ms") for the same reason the old copy did; I did not open the store's CREATE statement, so I cannot say the column order and the UNIQUE key agree. `tools/compare_db.py` reads profiles by the unique key generically (lines 106-146), so it does not name the column and the fixture would pass without the change.
Tests that call `add_records`: `test_profile_id_order_alone_compares_clean` (tools/test_compare_db.py:286) and `test_one_changed_record_path_is_reported` (:299). Their claims are about the compare tool on id order and one changed path; the new column changes neither. Records for these two tests: Delta round: changed through the helper; old record D14.md (not opened by me). Not judged beyond the helper: question 1 to 11 not asked for the two tests. Listed as not reached for a full record.
Q3 for the fixture: claim not checked on real input (a hand-built database; a real saved library copy at the new schema is needed).

---

## Not reached for a full record (no new line in their own bodies)

- `docs match code: the marker check catches a wrong number and a wrong word` (tests/test_docs_match_code.cpp:486): only the helper table beneath it changed; recorded as record 17 without the twelve questions.
- `test_profile_id_order_alone_compares_clean` (tools/test_compare_db.py:286) and `test_one_changed_record_path_is_reported` (tools/test_compare_db.py:299): changed only through `add_records`/`PROFILES`; no full record. Old record: D14.md (not opened).
- Digest tests beneath `digest_settings` (test_five_lane.cpp:287, test_perf_digest.cpp:128-164, test_song_parse.cpp's callers): their own lines are outside this slice's diff; the helper is judged in record 26.
- Records 14 and 15 and 13 (activation rows) are written short: they carry the old verdict and say why it holds, but the twelve questions were not asked line by line, because the changed line is the helper alone. Old records A3 13 to 22 were read via their Verdict lines and the headings; the bodies of A3 14 to 19 were not opened.
- The commit hash for the removal of `lone_fill_activation` was not looked up.

## Charts and captures needed and not available

- Early-fill dumps at both windows (`hydra_replay dump --efill 60`, `--efill 85`, and `schedule`) on a corpus chart with SP ready 0 to 85 ms around a fill deadline, and one with an E1 pass-over: records 1 to 4.
- Paths tab captures at 60 and 85 for Get Ur Freak On, Tapped Out, Focus, Epidermis, Burnout (not in the seven charts): records 9 to 15 and the helper finding.
- Screen captures: the batch confirm at default and at 85 (record 5); the tooltip on hover (record 6); the settings panel with the early-fill row at 85, 0 and during a batch (records 21 to 23); a live transcript of `click -` with the panel open (record 24); the library row of a corpus chart with a squeeze-in at 0 (record 18).
- A real saved library copy at the new profiles schema (record 28).

## Splits

- Test key vs app key: `corpus::settings_key` and `store::Lens::from` both answer "which settings change a stored analysis"; the window was added to each by hand (records 19, 25; `concept-G5.json`).
- `analyzed_like_the_gui` and `digest_settings` run at 85 ms; the app's default is 60 ms. A screen on the same charts at the default may differ from the figures the tests type (records 9 to 15, 26).
- The all-0 answer is stored in `record.paths` best or `record.allzero_paths` with no reader for "the answer" (record 4).

## Count

Test-case records: 24 (records 1 to 24); helper and rule-file records: 4 (25 to 28).
Written in full: 1 to 8, 16, 18 to 24. Written short (old verdict carried, helper-only change): 9 to 15, 17.
Verdicts on test cases: Rewrite 15 (1, 2, 3, 4, 6, 7, 11, 12, 13, 15, 16, 18, 19, 20, 24), Investigate 7 (5, 8, 9, 14, 21, 22, 23), Remove 1 (10), no verdict 1 (17). Records 25 to 28 are helper and rule-file judgements with no verdict.
New: records 4, 5, 6, 23. Changed: records 1 to 3, 7 to 22 and 24. Removed: the helper `lone_fill_activation` only (no whole test removed; two tests renamed, carried as records 2 and 3).
