Agent id: af64b768e02aa6d62 (task T5-V-DL5, session e633f7f6-f313-4787-b622-8d901b577b64)

# Verify, delta round, slice DL5

Plain version first. I checked every record in `records/delta-DL5.md` and `records/delta-DL5b.md` against the code. DL5b replaces the short DL5 records 9 to 15, 17, 26 and 28, so I judged those only through DL5b. I agree with all 36 test-case verdicts, six of them with notes the main session should read. I dispute none. I found one in-scope group with no record (the tests that read the corpus cache), one wrong claim in the engine records index, one place where the user's own words disagree with each other and with the tests, and a handful of small errors in the records. I ran nothing. Every mark below is "by reading" unless it says "proven by".

Read: both records files whole; the diff `c10c7f54..5ec682c7` of all 14 files; `docs/agents/test-rubric.md`; the test bodies and helpers named below; the production code each record names; `records-engine/index.md` lines 5, 29, 33; `records-screen/beg.txt` lines 65-67; the filed rulings `code-and-engine.md` lines 50 and 55-57 and `ui-and-display.md` lines 10-15 and 102-111. The worktree is at `65909a1f`; `git diff 5ec682c7 HEAD -- src tests tools` is empty, so the code I read is the code at `5ec682c7`.

## 1. Scope

I listed the in-scope test cases from the diff myself, then added every test that calls a changed helper.

| File | In-scope test cases | Record |
|---|---|---|
| test_fills_and_skips.cpp | 4: the 1.1 edge test (changed), "a fill that needs a 70 ms early hit..." (renamed, changed), "SP ready 70 ms before..." (renamed, changed), "all-0 pass..." (new) | DL5 1 to 4. Removed helper `lone_fill_activation`: DL5 "Removed" note and DL5b (commit `b1b0ed0f`, which I confirmed with `git log`). |
| test_batch_text.cpp | 2 new | DL5 5, 6 |
| test_cli.cpp | 2 changed | DL5 7, 8 |
| test_activation_rows.cpp | 7, all through `analyzed_like_the_gui` (Tapped Out 1308, Focus/Epidermis 1354, Tapped Out 1396, Get Ur Freak On 1439, 1469, 1543, 1556) | DL5b 1 to 7 |
| test_docs_match_code.cpp | 2 (472 and 486, through `known_defaults()`) | DL5 16, DL5b 8 |
| test_engine_sweeps.cpp | 2 | DL5 18, 19 |
| test_preview_controller.cpp | 1 | DL5 20 |
| tests/ui | 4 (`test_settings_lock`, `test_settings_reset`, `test_settings_early_fill` new, `test_command_bare_ref_ambiguous`) | DL5 21 to 24 |
| tools/test_compare_db.py | 2 | DL5b 9, 10 |
| Tests under `digest_settings` | 10 (five-lane 287; test_perf_digest 128, 132, 136, 142, 149, 154, 160, 164; crafted edge files 1385) | DL5b 11 to 20 |
| Helpers and rule file | `corpus::settings_key`, `concept-G5.json`, `analyzed_like_the_gui`, `digest_settings` | DL5 25, 27; helpers are judged inside the test records |

That is 36 in-scope test cases and 36 test records (16 standing DL5 records plus 20 DL5b records). `test_activation_rows.cpp:1331` (Burnout) is not in scope: `burnout()` did not change. The five-lane test and the crafted-files test call `digest_settings` but never read its new window line (`chart_parse_hash`, song_digest.h:289-304, reads only `reading()`, `rules` and `legacy_fills`). They are records by courtesy, which is fine ("a record for a test that did not change is a note, not a fault").

**Finding, scope.** `corpus::settings_key` (corpus_util.h:228) is a helper the corpus cache calls, and the same range also moved the default window the corpus analyses run at from a fixed 85 ms to 60 ms. Tests that read `corpus::analyzed(...)` therefore changed what they read, and none has a record. DL5 names the helper (record 25, "about 30 test files") but judges no test under it. In DL5's own files they are: `Skipped fills: every corpus root passes over real fills in order`, the three `trailing fills:` corpus cases (`a corpus path lists...`, `a path whose last window outlasts the chart lists none`, `a full pin of a corpus path agrees with the free search`), `dynamic hit window: every corpus row` (test_activation_rows.cpp:1607), and seven in test_engine_sweeps.cpp (`search invariants hold across the corpus and config knobs`, `the engine's limit walk agrees with Activation::within_ms_limit`, `the graph finds the chart's multiplier squeezes once, in chart order`, `note total: every path of How You Remind Me...`, `note total: the engine, the Song and the Dynamics tab agree...`, `legacy fill deadline analyzes a chart end to end`, and the squeeze-ins-at-0 test that has record 18). The brief says a changed called helper puts a test in scope. I list these as "no record" and leave it to the main session whether they count. Two of them hold typed pins that now run at 60 ms and could move: the trailing-fills case types `"3"`, tick 123838 and ticks 244800 and 281759 for Hail The Sun (test_fills_and_skips.cpp:362-364), and the corpus-row case needs `short_tags > 0` and `note_pressed_early > 0` (test_activation_rows.cpp:1695-1700).

## 2. Records in delta-DL5.md that stand (16)

Marks: "reading" means I read the code and the claim holds. Each verdict is mine; "agree" means I agree with the record's verdict.

**DL5 1. Skipped fills: under the 1.1 rule a fill exactly at the reach edge is stored (fills:82). Rewrite. Agree.**
Claims by reading: `fill_refuses` is `e_offset < -window_ms` (model.h:331), so ready at deadline plus window is kept and one ms later refused; deadline 7200 and edges 7285 and 7260 follow. The graph takes the window (graph.cpp:95-100, pather.cpp:265). Question 2 callers: engine.cpp:859-869, 1007, 1224, 1937, 2030, 2176; no display calls it. Delta line: the old A14 record 25 said Rewrite per B13; still stands.
Question 8 per assertion: `REQUIRE(deadline == 7200.0)` reds if the fixture tempo or the 1.1 rule moves. `passed_over(7285, 85) == {9600}` reds if `fill_refuses` becomes `<=` (the fill at e_offset -85 is then refused, skipped list empty). `passed_over(7286, 85).empty()` reds if the reach grows by a tick. A graph that drops the window falls back to 60, so the 85 pair reds (7285 gives -85, below -60); the 60 pair cannot see a dropped window, only a wrong one. The record says each 60 line fails "if the 60 run is given 85": true, and also true the other way round only for the 85 pair. Verdict reason is sound: 7285 and 7260 are raw results (rubric opens with exactly this example). Splits: none. Saved-file proposal: none.

**DL5 2. a fill that needs a 70 ms early hit is taken at an 85 ms window, not at 60 (fills:181). Rewrite. Agree.**
By reading: `early_fill_song(7270)` gives e_offset 7200-7270 = -70; at 85, `fill_refuses(-70, 85)` false, `is_e0(-70, 0, 85)` true; at 60, `fill_refuses(-70, 60)` true. `e_difficulty()` is `early_fill_difficulty(e_offset)` = 70 (model.cpp:477, model.h:340). `TargetResult::failed_tick` is a real field (pather.h:138). Question 8: `CHECK_THROWS` passes on any exception, so it cannot tell "refused fill" from any fault: the record is right, and it is the weakest line. The record's remaining claims hold. Old record A14 26 said Rewrite; still stands.

**DL5 3. SP ready 70 ms before a fill's deadline marks the activation E at 85, not at 60 (fills:220). Rewrite. Agree.**
By reading: e_offset 7200-7130 = 70; `is_e0(70, 0, 85)` true, at 60 false; `e_difficulty()` has a value only when `is_E0()` (model.cpp:477-480). The last `CHECK` compares `e_difficulty()` with `early_fill_difficulty(70.0)`, a value taken from the owner: a copy (rubric question 5). Agree it should be written as `-70.0` from the named inputs. Note: Beg's E0 at e_offset 0 does not split the windows at 1 to 85 but the index says it sits "inside the window at any value from 0 to 85 ms" (`records-engine/index.md:33`). That is wrong at window 0, where `is_e0` is a strict `<` (model.h:336) and e_offset 0 is not E0. The tooltip text agrees with the code, not the index. See Findings.

**DL5 4. all-0 pass: passing over a fill with 70 ms to spare is an E1 at 85, a skip at 60 (fills:249). Rewrite. Agree, with one condition.**
By reading: `allzero_activation` is `skips == 0 || (skips == 1 && is_e0(e_offset, 0, window))` (model.h:381); `Path::is_allzero` passes each activation's own `e_window_ms` (model.cpp:855); `attach_allzero` returns early when the best path is itself all-0 inside the limit (pather.cpp:53-56), which is why `wide.allzero_paths.empty()` holds; otherwise `search_allzero` fills it (pather.cpp:80-107). The test's own comment (lines 245-248) states the two-way storage, and the record's split is real. Condition: the record's rewrite ("state the all-0 answer through one reader") asks for a reader that does not exist. The rubric forbids proposing a new owner, so the rewrite cannot go ahead until the main session decides the split (see Splits, S1). The verdict stays Rewrite for the other faults (typed ticks 19200, 9600 named only in comments).

**DL5 5. batch text: the confirm's Early fill window line reads the window in ms (batch_text:185). Investigate, claim not checked only. Agree, marginal.**
By reading: `batch_settings_summary` copies `values.early_fill` from `describe_settings(s.to_analysis_settings())` (library_dialogs.cpp:64-66, 80), `format_ms_whole` of the setting (config.cpp:467). Callers in `src/`: library_dialogs.cpp:350 only. The expected side is two typed strings. If the main session counts typed expected text for a display as a fault of its own (rubric question 6), this record is a Rewrite, not an Investigate: the test could call `describe_settings` with the same settings. I agree with Investigate because the typed text is the user-visible wording and the A8 sibling went the same way. Needed capture: the Analyze-library confirm at 60 and at 85.

**DL5 6. batch text: the Early fill window help text (batch_text:194). Rewrite. Agree.**
By reading: every sentence matches `fill_refuses`, `is_e0` and `kMaxEarlyFillWindowMs`: "up to this many ms after" is the inclusive `-window`; "less than this many ms before" is `e_offset < window`; "At 0 no activation is an early fill" holds because `e_offset < 0` is only possible on the refused side. The test types the whole 5-sentence string and types "85 ms"; the owner builds it from `hit_window_text()` (settings_bar.cpp:41). Agree the fix is per-rule checks tied to owners. Not a Q3-only fault.

**DL5 7. hydra_batch fills a new database under the rule it ran under (cli:183). Rewrite. Agree.**
By reading: `cli/batch.cpp:152` prints `described.early_fill`; the test types `"Early fill : 60 ms"`. The test runs the real exe on a corpus chart (test_cli.cpp:163 copies the corpus chart). B1 record 1 said Rewrite (compare with `describe_settings`); still stands.

**DL5 8. hydra_replay refuses --ms, --efill and --depth values that are not whole numbers (cli:636). Investigate. Agree.**
By reading: `setting_flag_number` (replay.cpp:184-187) gives the typed message; `--efill` is read at replay.cpp:229 and 842. The accepted `--efill 85` and `--efill 900` only check exit 0, so the number reaching the search is unchecked, as the record says. B1 record 19 said Investigate (D308 item 1); stands. Not opened by me: D308 itself (the record kept B1's reading).

**DL5 16. docs match code: every marked default equals the code's value (docs:472). Rewrite. Agree.**
By reading: the two new names are `Settings::early_fill_window_ms` and `kDefaultEarlyFillWindowMs` (test_docs_match_code.cpp:328-334); `config.h:195` initialises the first from the second, so they cannot disagree. The markers exist with the number 60 beside each: `docs/UserGuide.md:135`, `CONTEXT.md:613`, `docs/clone-hero/04-fills.md:244` (I read the lines; DL5 did not open the numbers). Claim holds on the real docs. The verdict rests on the A11 reason (unmarked defaults are invisible), unchanged.

**DL5 18. corpus: squeeze-ins at exactly 0 and what they change (sweeps:581). Rewrite. Agree.**
By reading: nine typed totals (98, 56, 56, 5, 374, 90, 79, 69, 70) at lines 656-667. Three moved with the default window, which is the proof they are snapshots. Commit `e300f2fd` says the counts "return to the values the default window gives (90, 69, 70)", which is another sign nobody derived them. `shown_differs.size() == 0` is the one property worth keeping (A13 10 / B6).

**DL5 19. settings_key: the text for SearchSettings{} is pinned, and every field moves it (sweeps:745). Rewrite. Agree.**
By reading: `SearchSettings` has seven fields (pather.h:31-48) and the test now changes seven (`changed[0..6]`), so today the hand list covers every field; the weakness is only the next field (A13 14). Line correction: the owner line is `corpus_util.h:228`, not 227 (records 19 and 25 say 227). `store::Lens::from` (config.cpp:426-432, record_store.h:150) also names the window, so the two keys agree today: see Splits S2.

**DL5 20. a cancelled click job stops before counting (preview_controller:361). Rewrite. Agree.**
By reading: `ViewJob::run` hashes the chart file (library_jobs.cpp:697-706) before the first `throw_if_cancelled()` at 708, so "stops before counting" holds from 708 only. The new store is built and never read; `RecordStore::record_count()` exists (record_store.h:534). The record's rewrite (check the store holds nothing after) is possible with the fixture now present. `ok()` false and `is_cancelled()` true are true by construction: yes.

**DL5 21. test_settings_lock (uitest_details:1289). Investigate, claim not checked only. Agree.**
By reading: the new input sits between `begin_disabled_input(locked)` and `end_disabled_input(locked)` (settings_bar.cpp:158-167). Capture needed: the panel during a batch.

**DL5 22. test_settings_reset (uitest_library:298). Investigate, claim not checked only. Agree.**
By reading: the typed label "Analysis settings: Pro Drums off · SP cap 5 bars · Early fill window 85 ms" is built by `library_dialogs.cpp:98-100`; the new default checks read `Settings{}` (the owner). The typed label is the same fault A41 7 named.

**DL5 23. test_settings_early_fill (uitest_library:342, new). Investigate, claim not checked only. Agree.**
By reading: the clamp is `Settings::clamp` over the key table (config.cpp:201, settings_bar.cpp:161-162); -5 gives 0, 200 gives 85; the help-text check reads `hit_window_text()`; the ini check reads the saved file. The (?) hover x position copies the row's layout code (the record says so); that is a fault beyond Q3, so the "claim not checked only" words are slightly too strong: a layout copy fails for no behaviour. I still agree with Investigate because the record already says the capture pass should replace it with a widget id.

**DL5 24. test_command_bare_ref_ambiguous (uitest_live:537). Rewrite. Agree.**
By reading: the panel now has four number inputs (`##spcap`, `##earlyfillwindow`, `##depthvalue`, `##mslimitvalue`, settings_bar.cpp:144, 160, 184, 209), so the old typed 3 would break. The new test checks head and tail only, so "matches 1 widgets" or "matches abc widgets" passes: true. A43 L9 said Rewrite (check N equals the names listed, at least two); stands.

**DL5 25. corpus_util.h settings_key (helper). No verdict. Agree.** Line is 228. See scope finding for the callers.

**DL5 27. concept-G5.json (rule file). No verdict. Agree.** By reading: the pattern's new alternative `\bs\.early_fill_window_ms\s*<<` matches both `must_match` lines and neither `must_not_match` line. I grepped `tests` and `tools` for `early_fill_window_ms *<<`: only `corpus_util.h:228` and the rule file itself. So the rule flags no second encoder today.

## 3. Records in delta-DL5b.md (20)

DL5b replaces DL5 9 to 15, 17, 26 and 28. It corrects DL5 on three points, all of which I confirm: DL5 17 was the A11 record 3 (found); Burnout runs at 60 ms while the other four charts run at 85 (burnout() in song_details_fixtures.h:24-33 uses `scratch_settings().to_analysis_settings()` alone; `scratch_settings` sets no window); the digest window line cannot reach parse digests. One DL5b overstatement: it says the helper's comment "analyzed the way burnout() analyzes it" is no longer true. The comment (test_activation_rows.cpp:1217-1220) now ends "with the early-fill window at kHitWindowMs", so it states the difference. The helper's name `analyzed_like_the_gui` is what is false, as DL5 said.

**DL5b 1. Tapped Out's squeeze out is out of reach (1308). Investigate. Agree.** By reading: `squeeze_safe_ms` is `anchor_side * ratio + squeeze_side - needed` and `safe_is_short` is `< 0` (note_window.cpp:67-73); `squeeze_window_ends` is called at path_view.cpp:227 only; `replay_schedule.cpp:189-191` answers "does it fit" with rooms and `kRoomMarginMs = 3.0`. Two answers to one question: Splits S3. The `short` edge at 0 is an Unconfirmed line (`ui-and-display.md:107`), not a ruling, as the record says.

**DL5b 2. a squeeze in with a timing names its anchor the frontend (1354). Remove. Agree.** By reading: the corpus test (1607-1701) checks `figures[0].word == "frontend"`, `figures[1].word == "backend"` and the two ticks against `squeeze_anchor_tick` and `squeeze_chord_tick` on every row (1659-1677) and requires `note_pressed_early > 0`. That is the same promise from the owner's own record. The Remove holds while the corpus test is green at the default window (its loud guards fail if no early-pressed squeeze remains), so I do not dispute it.

**DL5b 3. an early fill gets one window, required or optional (1396). Rewrite. Agree.** By reading: `early_fill_window_end` uses `act.sp_ready_tick()` and `e_difficulty(true)` (note_window.cpp:132-140); the warn border is filled from the same `badge_timings()` loop the test re-runs (path_view.cpp:276-277), so line 1423 is a copy: record right. The optional-fill case contradicts an Unconfirmed line: Findings F3.

**DL5b 4. a squeeze in that needs no timing still gets its row (1439). Rewrite. Agree.** By reading: the pinned pathstring at 1451 holds E marks that depend on the window. The corpus test already checks "rows equal badges" (line 1647) on every corpus path, so the "gets a row" half is partly covered; whether the corpus holds a no-timing squeeze at 60 ms is unknown, which is why Rewrite and not Remove is right.

**DL5b 5. a row for every timing badge, edited figures too (1469). Rewrite. Agree.** By reading: `early_fill_safe_ms(ready_side, e_difficulty)` with e_offset 20 gives 82.9 - (-20) = 102.9; the typed 102.9 is a raw sum as the record says. The three subcases match the rows they name.

**DL5b 6. no chord times, no row anywhere (1543). Remove. Agree, with a correction.** By reading: `paths_tab.cpp:716-720` is the only caller of `PathsTabCache::details`, and passes `app.viewed.chords`, which is set in the same block as `v.record` (app_state.cpp:512-517); no other write sets `viewed.record`. So no screen builds the null case. Correction: the record says the default argument `chords = nullptr` "serves no caller". In `src/` and `tools/` that is true, but 28 calls in test_activation_rows.cpp and one each in test_path_list.cpp and test_preview_boxes.cpp use it. That does not change the verdict (the failure no screen can reach), but the production finding about removing the default argument would break those tests, so it is the main session's call.

**DL5b 7. the row sits alongside and changes no other field (1556). Rewrite. Agree.** By reading: the loop mixes Burnout (60 ms) with three charts at 85 ms; the baseline is the null-chords build no screen makes (see 6). `rows_shown >= 5` is a floor with a comment, not a reason.

**DL5b 8. docs: the marker check catches a wrong number and a wrong word (docs:486). Rewrite. Agree.** By reading: lines 489-503 as quoted; the test calls `check_markers`, which reads `known_defaults()`; the four expected problem lines use only the old names. 11 and 0.5 are "one more" and "not 0.1" with no source. Q3 holds (no real marked line fed with a changed number).

**DL5b 9. test_profile_id_order_alone_compares_clean (compare_db:286). Investigate, claim not checked only. Agree.** By reading: the fixture's column order and UNIQUE key now match `record_store.cpp:183` and 271-279 (`each_column(kLensColumnList...)` then hyversion, rules_fp). The claim is about two store files; the only input is a hand-built two-profile database. The brief says the real input is real store files; correct.

**DL5b 10. test_one_changed_record_path_is_reported (compare_db:299). Investigate, claim not checked only. Agree, with a note.** The record's own question 9 says the substring checks (308, 309) pass on any output holding those words. Under the brief a test with a second fault is not "only" question 3. The substring checks are how a text report is checked and the weakness is small, so I do not dispute, but the main session should read "claim not checked only" here as "plus weak substring checks".

**DL5b 11. five-lane: four-lane charts parse to their pinned digests (five_lane:287). Remove. Agree.** By reading: `list.txt` lines 1, 20, 29 and `expected_expert.tsv` lines 1, 20, 29 hold exactly `c01_base.chart`, `c20_disco.chart`, `m01_base.mid`; the crafted-files test (`test_song_parse.cpp:1385`) reads the same file under the same settings. So Remove is sound as long as 1385 stays. But 1385 is itself Rewrite for reading a saved expected file (rulings `code-and-engine.md:50`); the record says both go together if the golden question goes against saved digests. The main session should read the two as one decision, not a Remove now.

**DL5b 12 to 15. test_perf_digest.cpp:128, 132, 136, 142 (prepared-row digests). Investigate. Agree.** By reading: the file holds eight cases (the header comment at 126 says "six digests", the record is right); each hashes the whole corpus through `analyze_chart_file` with the window pinned at 85 by `digest_settings` (song_digest.h:327-333). Digest literals are numbers "pinned from one run": the rubric calls them snapshots, and `code-and-engine.md:50` ("we don't need golden anymore, remove it all") bears on them, so the question goes to the user. Not a defect of the window line.

**DL5b 16, 17. test_perf_digest.cpp:149, 154 (parse digests). Investigate. Agree.** By reading: the window line is never read (song_digest.h:292-293).

**DL5b 18, 19. test_perf_digest.cpp:160, 164 (parse digests at Medium and Easy). Investigate. Agree, with a note.** `check_parse_digest(..., 68)` compares `loading_chart_count` with a typed 68 (test_perf_digest.cpp:161, 165). The typed 68 is a total over `testdata/input`, which the rubric calls never fine on its own, independent of the golden question. The record says so under "Side finding" but does not carry it to the verdict. I keep Investigate (the hash literal decides the case) and ask the main session to treat the 68 as a Rewrite-grade fault whichever way the user answers.

**DL5b 20. the crafted edge files parse to their pinned lines (song_parse:1385). Rewrite. Agree.** By reading: 41 lines in each of `list.txt`, `expected_expert.tsv`, `expected_hard.tsv`; the test reads the saved lines and compares. It proposes the opposite of a saved file ("check that stated intent per file"), so it does not break the filed ruling.

## 4. Question 3, per the brief

- Charts: no record reached "Keep"; none of the 36 verdicts is Keep. Good.
- Every E claim: the only early-fill engine record is `beg` (`records-engine/index.md:29`), E0 at e_offset 0, made at the 85 ms code (line 5). No record exists at 60 ms or at 70 ms from a deadline. The records are right that the claim is not checked.
- Screen claims (batch confirm, tooltip, settings panel, button label, lock): no capture exists. Records 5, 21 to 23 correctly say "claim not checked".
- Files and store claims (compare_db, docs): handled as real docs (record 16) and as hand-built stores (DL5b 9, 10). Correct.
- Disputed Keeps: none. Rewrites whose only fault is question 3: none. Investigates that carry another fault: DL5b 10, 23 and 18/19 (notes above).

## 5. Question 2, callers I searched myself

`is_e0`, `fill_refuses`, `allzero_activation`: only `src/core/model.*` and `src/search/engine.cpp` (grep over `src` and `tools`, `.cpp` and `.h`). `describe_settings`: `config.cpp`, `cli/batch.cpp:145`, `ui/library_dialogs.cpp:65`, `tools/bench.cpp:99`. `batch_settings_summary`: library_dialogs.cpp:350. `early_fill_window_help_text`: settings_bar.cpp:156. `build_activations`: `PathsTabCache::details` (path_view.cpp:637) only. `squeeze_window_ends` and `early_fill_window_end`: path_view.cpp:227 and 238. `all_allzero_paths`: path_view.cpp:526 only. No record missed a caller. The all-0 answer is read in two places (`record.paths` best and `allzero_paths`), confirmed.

## 6. Splits (every one, one line each)

- S1. The all-0 answer is stored in `record.paths` (best path, when it is all-0 inside the limit) or in `record.allzero_paths`, and the display re-derives "already listed" by `path_identity` (path_view.cpp:522-532), while `attach_allzero` decides the same thing by `is_allzero` and `within_ms_limit` (pather.cpp:55). Comments say they agree by construction. DL5 record 4.
- S2. Two keys answer "which settings change a stored analysis": `corpus::settings_key` (corpus_util.h:228) and `store::Lens::from` (config.cpp:426-432). Both now name the window; both were edited by hand. DL5 19, 25, 27. Agreeing today.
- S3. "Does this squeeze fit the hit windows": the Paths tab's sum (`squeeze_safe_ms`, short below zero) against the schedule's `feasible` (rooms against a 3 ms margin, replay_schedule.cpp:189-191). On Beg both say fits, with 84.4 against 73.85. DL5b 1.
- S4. The window the activation-row tests run at: four charts at 85 ms, Burnout and the corpus test at the 60 ms default. DL5b 6, 7 and the corpus test at 1607.
- S5. The four prepared-row digests are pinned at 85 ms; the app starts at 60 ms. DL5b 12 to 15.
- S6 (missed by the auditors). The user's words on the Dynamic Hit Window row disagree with each other and with the tests. `ui-and-display.md:110` (Unconfirmed, 2026-11-12, "Yes, ship it now") says: (3) no row for a timing that needs no hitting, (4) no row for an optional early fill. The tests at 1396, 1439 and 1469 pin a row for both, and cite "every badge gets a row" (the 2026-11-12 line). The one filed line (`ui-and-display.md:11`, "i explicitly asked for the new info to show on all 3 kinds of badges.") says all three kinds of badges, which does not settle "optional" or "needs no timing". Line 108 (also Unconfirmed) says "On screen every badge has a timing". Two Unconfirmed lines answer one question differently; the tests follow the second. This goes to the user.
- S7 (missed). `records-engine/index.md:33` says Beg's E0 at e_offset 0 is "inside the window at any value from 0 to 85 ms". At window 0 `is_e0` (strict `<`) says it is not E0, and the tooltip (settings_bar.cpp:39) agrees with the code. The index text is wrong at the lower edge. Not a code split; a claim in a record file.

## 7. Findings that are not verdicts

- F1. The corpus-cache tests have no record (section 1).
- F2. Small errors in the records: `settings_key` line is 228, not 227 (DL5 19, 25); DL5b says `tests/test_replay.cpp` "uses the same helper" but it never calls `digest_settings` (it includes `song_digest.h`, 2 uses of `digest::`); DL5b 6 on the default argument (above); DL5b on the comment that "is no longer true" (above); DL5 record 5 and 6 cite "the user's `Add a line` answer" (test_batch_text.cpp:183, a comment): I grepped `docs/` and found it only inside DL5's own text, so DL5 is right that it is unverified.
- F3. Rulings cited by the tests: the comments at test_activation_rows.cpp:1407, 1429, 1467 and 1642 call "No word", "every badge gets a row" and the 2026-11-12 answer "the user's ruling". Only `ui-and-display.md:14` ("change Clone Hero to Dynamic Hit Window and mock it up") and `:11` are filed. DL5b notes this. The 60, 0 to 85 and one-number-on-both-sides words are filed (`code-and-engine.md:55-57`), so the window numbers are rule numbers with a filed reason; 70 and the typed ticks are the tests' own inputs.
- F4. No record proposes a saved expected-output file. DL5b 20 proposes the opposite. The digest records (11 to 19) name `code-and-engine.md:50` correctly and leave the question to the user.

## 8. Tally

36 in-scope test cases found. 36 records (16 DL5, 20 DL5b). Verdicts agreed: 36 of 36 as written, with six notes that the main session should read (DL5 4, 5, 23; DL5b 10, 11, 18/19). Disputes with the verdict itself: 0. Disputes with the record's reasoning or facts: DL5b 6 (default argument claim), DL5b on the comment, DL5 19 and 25 (line number), `records-engine/index.md:33` (not a DL5 record). In-scope tests with no record: the corpus-cache group in section 1 (about 12 tests in DL5's own files). Not reached: none.
