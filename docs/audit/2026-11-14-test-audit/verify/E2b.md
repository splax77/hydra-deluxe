Agent abc2f69589f80db00 (T5-V-R24-E2b, session 16892967-2cf8-4437-a7cd-e3dad250f621)

# Verify E2 part b: the 75 records for test_progress.py

Result: 75 records, 75 test cases, every test case has a record (the names and line numbers match the test file one for one; I diffed the two lists). I agree with 48 verdicts and dispute 27. Nothing was left unopened.

Read for every record: all 771 lines of `C:\Users\Patrick\.claude\gates\tests\test_progress.py`; all of `gates\progress.py` (102 lines) and all of `gates\percall.py` (176 lines); `common.py` (`run`, `check_id`, `status_path`, `read_agent`); `stall.py` `_session_agents`; `agent_stop.handback_path`; `review_rules.py:50-86`; `status_append.ps1`; `settings.json:8` and `:158`; `gates\tests\test_percall_key.py:1-56`; `test_docs_numbers.py:23-24, 95-120`; `tools/ch_mcp/server.py:383-395, 463-480` and `game_tools.py:70-71, 1072-1135` in the worktree; `docs/agents/rulings/review-and-hooks.md` and `agents-and-workflow.md` with their section heads; `docs/agents/test-rubric.md`. I ran nothing. Every mark below is "by reading" unless it says otherwise; no existing run in the records folders covers these tests.

## Findings that apply to many records

1. The Keeps contradict the record's own file-level answer. The head of the test_progress block says question 3 is "claim not checked" for every test, because every payload is hand-typed. The brief and rubric rule Keep out on one such answer. All seven Keeps (341, 347, 354, 362, 394, 573, 659) rest on a hand-typed payload plus an exception injected by a patch (`break_progress`, a `boom` function, a mock `OSError`, a fake `human_prompt_text`). A crash claim is not a threads, timing or transport claim, so "real mechanism" does not rescue it. Verdict for all seven should be Investigate, "claim not checked only", capture needed: a recorded real subagent call (what `tests\replay.py` and `replay_extract.py` read) fed to `percall.body`. I dispute all seven.

2. The record's 5 and 10 minute answer is wrong. It says no ruling was found for 5 and 10 minutes. `docs/agents/rulings/review-and-hooks.md:34`, under `## Confirmed` (which runs lines 7-36), says: "demand status updates every 5 minutes of real time and reject any calls with no updates after 10 minutes" (2026-09-04, session 48cdbb34). So 5 and 10 are rule numbers with a filed ruling, as 20 and 30 are (line 24, quoted by the record). The 100 and 150 call numbers have no Confirmed ruling: the only lines are `agents-and-workflow.md:43` ("soft cap 100 calls (hand off), hard cap 150") and `review-and-hooks.md:52, 60`, all under `## Unconfirmed`. The record also cites `review-and-hooks.md:50` and `:47` as if they were rulings; both are Unconfirmed too (that section starts at line 37), so neither is citable.

3. The "Rewrite: take the number from `progress.X`" remedy is wrong and I dispute every record that proposes it (5, 6, 7, 9, the four TimeBudget, seven StatusAge, 566, 728, 732; listed below). The rubric (Q4/Q5 and "Named inputs") says a rule number typed with its ruling beside it passes, and an expected value taken from the code under test is a copy. Seeds and times read from `progress.CALL_STOP` etc. make the test agree with any value the constant takes. Only the pin at `test_progress_holds_the_numbers_in_one_block` (line 758) would still catch an edit, and it types the literals anyway. For 20, 30, 5 and 10 the right verdict is Investigate, "claim not checked only", with the ruling line written beside the number. For 100 and 150 the verdict is Investigate with a question to the user, because question 5 has no Confirmed ruling (the record says so itself and still writes Rewrite).

4. The record never opened `test_percall_key.py`. I did: lines 41-43 pin the same two key-tool names and lines 45-49 pin the same `keys` route, so tests 498 and 606 repeat promises already made (verdict Remove, or one shared verdict).

5. The key names are checkable now against an existing owner. `tools/ch_mcp/server.py:463` (`ch_press`), `:472` (`ch_key`), `:383-390` (`ch_play_chart`, `route` is "kit" or "keys"), `game_tools.py:70-71` (`ROUTE_KIT = "kit"`, `ROUTE_KEYS = "keys"`). The server also refuses any route not in `ROUTES` (`game_tools.py:1102-1103`, 'unknown route'), so the odd spellings in test 642 ("Keys", "keys ", "pads", "") pass the hook and then die in the server: that answers the record's open "can the schema send them" (question 11), by reading.

## Callers of `progress.py` (question 2), my own search of `gates\` and `settings.json`

`progress.check`: only `percall.py:105`. `percall.py` runs from `settings.json:158` (agrees with the record). `progress.is_status_call`: `percall.py:112`, `shell_output.py:78`, `tests\replay.py:248`, `tests\test_bash_windows_rule.py:141`. Constants: `percall.py` (`SHELL_TOOLS`, `REPORT_TOOLS`), `test_docs_numbers.py` (docs agree with the numbers), `test_bash_windows_rule.py:147` (`CALL_STOP`). `stall.py:41-55` reads the agent records `progress.check` writes (`last`, `finished`) without importing progress. The record named `percall` and `settings.json` only and missed `shell_output.py:78` and `stall.py`; the first matters because a change to `is_status_call` changes the rewrite too, and no test in this file covers that.

## Splits (one line each)

- S1 (auditor's): the budget numbers have one home, `progress.py:14-16`, and this file types 100, 150, 20, 30, 5 and 10 as seeds and times in about 25 tests; only line 758 ties them.
- S2: the status helper's path is spelled three times: `progress.py:22` (`STATUS_PREFIX`), `settings.json:8` (the allow rule), and `STATUS_CMD` typed at `test_progress.py:25` (StatusShape reads `progress.STATUS_PREFIX`, the other tests do not). `test_docs_numbers.py:183` checks the docs against `STATUS_PREFIX`; nothing checks `settings.json` against it.
- S3: the status folder is `common.status_path` (`common.py:119`, `STATE_ROOT\status`) and also a hard-coded `...\gates\state\status` in `status_append.ps1:10`. Test 143 and `touch_status` (line 128) rebuild the path by `os.path.join` a third time.
- S4: the key tool names are `percall.KEY_TOOLS` (`percall.py:16`), the server's own function names (`server.py:463, 472`), and two test files (test_progress 498 and test_percall_key 41-43).
- S5: the agent record's shape (`first`, `last`, `calls`, `warned_calls`, `warned_minutes`, `finished`) is written in `progress.check` (line 49-50) and typed again in `seed()` (test lines 122-125); `stall.py` and `agent_stop.py` read it too. A field rename in progress.py would break the real path and leave `seed()` writing the old shape.
- S6: the allow phrase. `docs/agents/brief-preamble.md` says the user must send "exactly `allow library run`"; `review_rules.py:52, 55` also accepts any case, surrounding spaces and one trailing `.` or `!` (tests 431 and 543 pin that). Document and hook answer "what counts as the allow" differently. No Confirmed ruling names the tolerance (the record's open question stands; I agree it goes to the user).
- S7 (production, missed by the auditor): `progress.py:61` exempts a call from the call and time stop when `command.startswith("git ")`. A chained call such as `git status; ls` is exempt from the budget stop, while `delete_rule` and `bash_windows_rule` judge command words by pieces. Test 186 only feeds plain `git status` and `git commit -m x`. The README says "git commands ... get through", so the prefix test is a second, weaker answer to "is this a git command". No test in this file pins the chained case either way. A finding against `progress.py`, not against a test.
- S8: tests 168/182 state the same check (see 182); 437/533 (the same `_allow_refusal` rule for the library and key phrases), 442/551, 501/509/620/627 (one-use rule, one owner `percall._allow_refusal`) are one rule in several tests.

No rewrite in this slice proposes a saved expected-output file. The two uses of "capture" are recorded real calls, not golden files.

## Per record

The marks "agree" and "dispute" are about the record's verdict. Line numbers are the test file's.

### FirstCall
- test_first_call_gives_id_notice (139): agree, Investigate (claim not checked only). Q8: line 142 (`"a1"` in the text) is implied by line 143 (the path ends in `a1.md`), so 142 catches nothing 143 does not; the record counts it as its own catch. Lines 144, 145 hold (`progress.py:58-59, 49`).
- test_second_call_is_silent (147): agree. 149 would fail on a repeated notice (`first_call` at progress.py:58); 150, 151 hold.
- test_status_helper_is_not_counted (153): agree. Line 155 (`assertIsNone`) is weak: a counted status call at seed 7 would also return None (no notes at call 8, status age 0). Line 156 carries the claim.
- test_id_notice_waits_for_first_counted_call (158): agree. 160 and 161 hold (progress.py:53-58 makes the record with zero calls).

### CallBudget
- test_call_100_warns_once (166): dispute. Verdict Investigate: question 5 has no Confirmed ruling for 100 (record admits it) and claim not checked; the remedy "seed from `CALL_WARN - 2`" copies the owner. Line numbers in the record are off by one: 168 is "no warning at 99", 170 is "not denied at 100", 171 is the warning, 172 is "once". Each holds (progress.py:67-70).
- test_call_150_denies_read (174): dispute, same reasoning (150 unruled; Investigate). Line 176 holds (progress.py:79).
- test_call_149_still_passes (178): dispute, same.
- test_call_99_does_not_warn (182): agree, Remove. Identical to line 168 (seed 98, first call, no "wrapping up").
- test_call_150_lets_report_git_and_status_through (186): dispute. The remedy reads the pass list from `progress.REPORT_TOOLS`; that would make line 191 unable to catch a tool dropped from that list, which is the very bug the record says 191 catches. Verdict Investigate (150 unruled, claim not checked). Lines 191, 192 hold (progress.py:61, 77-79). See S7: the git entries never include a chained command.

### TimeBudget
- test_minute_20_warns_once (196): dispute (20 has a Confirmed ruling, review-and-hooks.md:24, and the test uses it as a rule number; remedy wrong; Investigate, claim not checked only, cite the ruling beside the number). Lines 201-203 hold (progress.py:71-74). The `touch_status(now)` in setup keeps the status age at zero, so only the minute rule is tested.
- test_minute_19_is_silent (205): dispute, same.
- test_minute_30_minus_a_second_passes (211): dispute, same.
- test_minute_30_denies (217): dispute, same. 221 holds (progress.py:79). 222 passes only because git is exempt (progress.py:61), as the record says.

### StatusAge
- test_5_minutes_old_nags (226): dispute (5 has Confirmed ruling at review-and-hooks.md:34, which the record said did not exist; Investigate, claim not checked only). Holds: progress.py:90-93.
- test_under_5_minutes_is_silent (233): dispute, same.
- test_just_under_10_minutes_nags_but_passes (238): dispute, same.
- test_nag_repeats_on_every_call (245): agree, Investigate. 6 minutes is a free input inside the window between the two ruled numbers 5 and 10, so question 5 is met by its position, not unnamed.
- test_10_minutes_old_denies (251): dispute, same.
- test_no_status_file_counts_from_first_call (258): dispute, same.
- test_status_helper_passes_when_overdue (262): dispute, same.
- test_fresh_status_file_resets (266): dispute, same. The reset is by file mtime (`os.utime`), a real mechanism; it still has a hand-typed payload.

### Routing
- test_main_session_goes_to_stall_and_is_never_counted (275): agree, Investigate. Holds: percall.py:126-139, 163-164; stall is a fake, so which hook answers is checked, not what stall says.
- test_user_prompt_goes_to_stall (282): agree.
- test_subagent_bash_runs_progress_then_delete_rule (286): agree, Investigate. Line 298 adds nothing: the exact-list equality at 297 already rules stall out in each round. The record lists 298 as a separate catch.
- test_shell_call_runs_the_real_rewrite_without_crashing (300): agree, Rewrite. It cannot tell "rewrite ran and did nothing" from "rewrite never ran"; the word "crashed" is `percall.py:153`'s own text, so line 304 does catch a crash.
- test_delete_rule_reason_becomes_deny (306): agree, Investigate (the real rule's reasons are in test_delete_rule.py).
- test_subagent_read_skips_delete_rule (314): agree.
- test_progress_deny_wins (318): agree with Rewrite, but not with the record's reason or remedy. By reading `percall.py:105-123`: if the early return at 106-107 were deleted, the call falls through to the last line, `return out`, which is the same deny. Line 320 stays green, so the test does not pin "deny wins". A real pin needs a competing refusal (set `fakes["delete_reason"]`) and an assertion on whose text comes back. The record's remedy (seed from `CALL_STOP - 1`) is the copy problem again.

### Crash
- test_subagent_crash_denies (341): dispute Keep, finding 1. Holds: percall.py:165-169 re-raises, `common.run` (common.py:48-66) denies with `boom` in the text.
- test_subagent_crash_lets_report_tool_through (347): dispute Keep, finding 1. Holds: percall.py:168-171.
- test_main_session_crash_is_loud_not_a_deny (354): dispute Keep, finding 1. Holds: percall.py:135-139. It also edits `sys.modules["stall"].run` and relies on the fake's cleanup.
- test_bad_session_id_denies (362): dispute Keep. Besides finding 1: the only assertion is "is a deny". Any crash in the path (an import error, a typo) would pass. `common.check_id` (common.py:109-112) raises the bad-id error, and the deny text names it, but nothing asserts that text. A weak Q8 line the record did not see.

### HandbackSave
- test_saved_past_the_call_budget_and_the_minutes_and_never_denied (379): agree, Investigate. Holds: percall.py:100-107 saves first; progress.py:77-78 never denies a report tool.
- test_second_handback_overwrites_the_first (385): agree.
- test_handback_without_a_string_message_saves_nothing (390): agree.
- test_failed_save_lets_the_handback_through_and_says_so (394): dispute Keep, finding 1. The failure is a patched `OSError`, not a real full disk (the record says so), and the payload is hand-typed. Holds: percall.py:165-171.

### WholeLibrary
- test_refused_with_no_allow (413): agree, Investigate. The phrase at 416 is typed, not read from `LIBRARY_ALLOW_TEXT` (S6, S8).
- test_refused_in_a_subagent_with_no_allow (418): agree. It asserts only a deny; a refusal from another rule would also pass.
- test_one_human_allow_passes_once (423): agree. Holds: percall.py:23-42. The transcript read is faked.
- test_allow_spelling_case_and_one_trailing_mark (431): agree Investigate; goes to the user (S6). The record's remark that line 47 of review-and-hooks.md is the closest ruling is wrong: it is Unconfirmed.
- test_refused_for_sdk_or_peer_typed_allow (437): agree.
- test_allow_from_another_session_does_not_count (442): agree.
- test_subagent_uses_the_allow_after_delete_rule_passes (446): agree.
- test_two_calls_racing_for_one_allow_only_one_passes (456): agree, Investigate. The brief says a race is checked on the real mechanism; the rival here is a patched writer on the real file, not a second process, so it fails question 3. Needs two real concurrent calls on the real waiver file.
- test_command_without_flag_passes (468): agree.

### KeyRoute
- test_both_tools_refused_with_no_allow_in_main_and_subagent (487): agree. Holds: percall.py:92-96 contains all six words.
- test_the_two_tool_names (498): dispute. Verdict Remove (same promise as test_percall_key.py:41-43); the real names are in `server.py:463, 472` (finding 5), so the "needs the server's list" capture already exists in the repo.
- test_one_human_allow_passes_exactly_one_call (501): agree.
- test_one_allow_passes_one_subagent_call (509): agree.
- test_two_allows_pass_two_calls (515): agree.
- test_other_tools_pass_untouched (523): agree. `mcp__other__ch_key` is a real shape for the exact-name rule (percall.py:72).
- test_refused_for_sdk_or_peer_typed_allow (533): agree, same rule as 437.
- test_recorded_text_that_is_not_the_allow_does_not_count (539): agree.
- test_allow_spelling_case_and_one_trailing_mark (543): agree; one question to the user together with 431 (S6).
- test_allow_from_another_session_does_not_count (551): agree.
- test_library_and_key_allows_do_not_stand_in_for_each_other (555): agree.
- test_out_of_budget_subagent_keeps_its_allow (566): dispute. Investigate: 150 unruled and claim not checked; remedy copies the constant. Holds: progress's deny at percall.py:105-107 returns before `_key_refusal`, so nothing is spent.
- test_a_crash_in_the_check_denies (573): dispute Keep, finding 1. Holds (percall.py:34 raises through to `common.run`).

### PlayChartKeysRoute
- test_the_route_that_counts (606): dispute. Remove (same as test_percall_key.py:45-49); the real name is `ROUTE_KEYS = "keys"` in `game_tools.py:71`.
- test_keys_route_refused_with_no_allow_in_main_and_subagent (609): agree. All seven phrases are in percall.py:92-96 and `_key_call` (line 81).
- test_one_allow_passes_exactly_one_keys_call (620): agree.
- test_one_allow_passes_one_subagent_keys_call (627): agree.
- test_the_allow_is_the_same_kind_as_the_key_tools (633): agree.
- test_other_routes_and_no_route_pass_without_reading_the_transcript (642): agree, with the note in finding 5 that the server refuses unknown routes.
- test_keys_route_on_another_tool_passes (651): agree.
- test_a_crash_in_the_check_denies (659): dispute Keep, finding 1.

### StatusShape
- test_status_line_naming_xargs_rm_passes_percall_uncounted (695): agree. The real `delete_rule` is loaded here, so the verdict in this path is real; the call is still hand-typed. Holds: progress.py:26-27 and 53-56; shell_output.py:78 is the second reader.
- test_chained_status_call_is_counted_and_refused (702): agree. Line 709 asserts only that the two letters "rm" occur in the refusal; many unrelated words contain them, so it can pass for the wrong refusal. Lines 708 and 710 carry the claim.
- test_line_with_subexpression_or_backtick_is_not_a_status_call (712): agree. Needs the real PowerShell parse (not run).
- test_shapes_that_run_a_second_command_are_not_status_calls (716): agree, same.
- test_chained_status_call_past_budget_is_refused (728): dispute. Investigate (150 unruled, claim not checked); remedy copies the constant.
- test_plain_status_call_past_budget_passes (732): dispute. The fifth item of test 186 (status call at seed 150 not denied) and test 153 (not counted) together make this promise. Remove, or Investigate if the main session wants the combination kept.

### NumbersLiveOnce
- test_budget_numbers_are_defined_only_in_progress (739): agree, Investigate. Settled by reading: it does not overlap `test_docs_numbers.py`; that file scans only the docs in its `DOCS` tuple (lines 23-24), this one scans the hook code. The real folder is real input with owner `progress.py`. Remaining fault: the regex pairing 100/150/20/30/5/10 with "calls|minutes|budget" on one line is a stand-in, and it lists only top-level `gates\*.py`, not `lib`, `.ps1` files or `settings.json`.
- test_progress_holds_the_numbers_in_one_block (758): agree with Investigate, but not with the reason. 20, 30, 5 and 10 each have a Confirmed ruling (finding 2); only 100 and 150 need the question to the user.
- test_progress_compiles_only_the_status_shape (763): agree, Investigate; the design reason ("Plan section 4") is not a ruling and was not opened.

## Count

75 records, 48 agree, 27 dispute: 7 Keeps (341, 347, 354, 362, 394, 573, 659) that should be Investigate; 2 Investigates (498, 606) that should be Remove; 18 Rewrites (166, 174, 178, 186, 196, 205, 211, 217, 226, 233, 238, 251, 258, 262, 266, 566, 728, 732) that should be Investigate, except 732, which should be Remove. 7 + 2 + 18 = 27.
