Agent a5cb278836ca14089 (T5-V-R24-E2a, session 16892967-2cf8-4437-a7cd-e3dad250f621)

# Verify, round 24, slice E2 part a

Scope: records in `records/E2.md` for `test_percall_rewrite.py` (18), `test_posture.py` (25), `test_publish.py` (40). 83 records, 83 test cases, every name matches. No test case lacks a record; none was left unreached. Nothing was run. Every mark below is "by reading" unless it says otherwise; no run in the records folders covers these three files.

Read for all records: the three test files whole (`C:\Users\Patrick\.claude\gates\tests\`); `percall.py`, `posture.py`, `prompt.py`, `publish_gate.py`, `publish.ps1` whole; `shell_output.py` lines 20-83; `progress.py` lines 20-70; `delete_rule.py` `pieces`/`word`; `review_rules.py` lines 26-28, 77-131, 193-196; `common.py:66`; `settings.json` lines 9, 125, 141, 158, 190; `gates\README.md` lines 47-79; Hydra `docs/agents/rulings/review-and-hooks.md:22,50`; plan `docs/superpowers/plans/2026-10-31-harness-hooks-and-tools.md` lines 83-96 and Q8. Not read: `test_stall.py`, `test_shell_output.py`, `test_review_rules.py`, `common.run` beyond line 66, `agent_stop.py`.

## Shared answers, checked

- Question 2 callers, percall: the record says the only caller is the harness through `settings.json:158`. Missed: `prompt.py:18` lists `("percall", "closed")` in `PARTS` and calls `percall.body` at `prompt.py:28`, and `settings.json:141` runs `prompt.py`. So percall.body also runs on every UserPromptSubmit through the prompt hook. Finding against group R's question 2 (it does not change any verdict). Posture, publish_gate and publish.ps1 callers hold (`settings.json:125,190`, `:9`; my grep of `gates\*.py|ps1|md` found no other caller; README lines 47, 61, 77, 79 only describe them).
- Question 3 everywhere: "claim not checked" holds, since every payload is typed. Read with the brief's hook rule, that also rules out every Keep that rests on a typed payload (see Disputes).
- Posture 60 minutes: the record says the comment "the user's answer to Q8" was not opened. I opened the plan: T2 says "The 60 is my pick, not a user decision ... the user settles it in Q8 before the build." No filed ruling or card answer was found (grep of `docs/agents/rulings/`, `docs/` for "HX-P2", "resend"). `posture.py:16` calls it the user's answer; that is a comment, so it stays a claim. Goes to the user as a question.
- Record correction: the posture file says "why 10000 (FIELD_CAP): not found". `prompt.py:19-21` gives it: "Claude Code's cap on one hook's additionalContext ... (code.claude.com/docs/en/hooks, 'capped at 10,000 characters')". That is an outside document cited in a comment, not opened by me (no web call), so it is a claim about Claude Code, not checked.
- No rewrite in the three files proposes a saved expected-output file.

## test_percall_rewrite.py (18 records: 13 agree, 5 dispute)

Callers and `settings.json` as above. Per record, the hook code read is `percall.py` `body`, `_subagent`, `_main_session`, `_with_rewrite`.

1. test_a_build_gets_the_first_call_notice_and_the_rewrite_in_one_answer (:52): Agree Rewrite. Lines 54-59 each can fail (extra key, notice dropped, tail wrong, decision added). `BASH_TAIL` (:25) is a typed copy of `shell_output.py:35`; `FIRST_CALL_NOTICE` copies `progress.py:64`.
2. test_a_refused_call_gets_the_deny_alone (:61): Agree Investigate, claim not checked only. Line 63 fails if `delete_rule.check` stops denying `rm -rf C:\` (`percall.py:116`); line 64 fails if a rewrite lands on a deny (`_with_rewrite:147`).
3. test_a_refused_delete_before_a_build_is_not_rewritten (:66): Agree Investigate, claim not checked only. Line 69 reads the owner's wording ("rm"), a copy.
4. test_a_rule_refusing_after_the_budget_check_keeps_the_rewrite_out (:72): Agree Investigate, claim not checked only. `--whole-library` typed; owner `percall.py:14`. The refusal runs at `:116`, after `progress.check`, as the name says.
5. test_a_call_past_the_budget_is_not_rewritten (:77): Agree Rewrite (use `progress.CALL_STOP` for `calls`).
6. test_git_status_is_not_rewritten (:84): Agree Investigate, claim not checked only. Line 86 does not fail on a crash in the first-call path; it raises KeyError only if the whole reply is None.
7. test_the_status_call_is_not_rewritten (:88): Agree Investigate, claim not checked only. I checked line 91 can fail: `delete_rule.pieces` cuts on `;` with no quote reading (`delete_rule.py:75-77`), so "cmake next" becomes a command word unless `shell_output.rewrite` returns None for a status call (`shell_output.py:78`). Good input. The `or {}` still lets a None reply pass, which is fine for "not rewritten".
8. test_a_crash_keeps_the_notice_adds_a_message_and_leaves_the_command (:93): Dispute Keep. Input is a typed payload plus a patched `rewrite` that raises; the brief's hook reading puts that at "claim not checked". Should be Investigate, "claim not checked only"; capture: a recorded real build PreToolUse call run through the same patch. Lines 96-99 each can fail (`percall.py:152-155`).
9. test_a_crash_never_refuses_through_the_whole_hook (:101): Dispute Keep, same reason; Investigate, claim not checked only. Note the crash is swallowed inside `_with_rewrite:152`, so `common.run`'s closed mode never sees it; lines 108-110 would go red only if that try block is removed.
10. test_a_build_with_nothing_else_to_say_gets_a_new_answer (:114): Agree Rewrite (typed tail, whole-dict snapshot).
11. test_the_rewrite_rides_beside_a_stall_notice (:118): Agree Rewrite.
12. test_the_rewrite_sits_beside_stalls_crash_message (:124): Agree Rewrite.
13. test_a_media_rule_ask_is_not_rewritten (:129): Agree Investigate, claim not checked only. Fails at 131 if `media_rule.check(main=True)` stops asking (`percall.py:133`); at 132 if `_with_rewrite` skipped its permissionDecision test (`:147`).
14. test_a_refusal_is_not_rewritten (:134): Agree Investigate, claim not checked only. Duplicates 4's rule on the main side (`percall.py:129`).
15. test_git_status_answers_nothing (:139): Agree Investigate, claim not checked only.
16. test_a_crash_with_nothing_else_to_say_is_only_the_message (:142): Dispute Keep; Investigate, claim not checked only. Line 145 and 146 each can fail (`percall.py:155`).
17. test_a_crash_keeps_stalls_message_too (:148): Dispute Keep; Investigate, claim not checked only. The "\n" is the owner's join (`percall.py:155`), typed in the test.
18. test_a_tool_that_is_not_a_shell_never_reaches_the_rewrite (:155): Dispute Keep; Investigate, claim not checked only (typed Read call). Line 158 does fail if the SHELL_TOOLS test at `percall.py:147` is dropped, because the patched rewrite raises and the message is added.

## test_posture.py (25 records: 18 agree, 7 dispute)

19. test_first_prompt_sends (:57): Agree Investigate, claim not checked only. 58 fails on no send (`posture.py:51`); 59 on no write (`:50`).
20. test_recent_send_is_skipped (:61): Agree Investigate, claim not checked only.
21. test_resend_after_interval (:67): Dispute Remove. Line 70 (the new time is stored after a late resend) is covered by no other test: `test_interval_boundary` never reads the state file, `test_skip_does_not_push_the_next_send_back` and `test_prompt_after_compaction...` would stay green if the write at `posture.py:50` ran only when `sent is None`. The plan also names this test as acceptance criterion AC3 for T2 (plan line 94), so a Remove leaves that line pointing at nothing. Verdict should be Rewrite (use `posture.RESEND_MINUTES` instead of 61); the record's "other paths check the state write" does not hold for the resend path.
22. test_interval_boundary (:72): Agree Investigate, claim not checked only (also the 60). Line 74 fails if `<` becomes `>=`'s opposite shift; 75 fails if `<` becomes `<=` (`posture.py:48`).
23. test_skip_does_not_push_the_next_send_back (:77): Agree Rewrite (61 from 60). Line 80 fails if a skip re-stamps, by reading (a re-stamp at 30 leaves 31 minutes at 61).
24. test_missing_file_is_loud (:82): Agree Investigate, claim not checked only. The record's "duplicate with test_stall.py": not read by either of us.
25. test_empty_file_is_loud (:90): Agree Investigate, claim not checked only.
26. test_bad_state_is_replaced (:99): Agree Investigate, claim not checked only. Each input of the four reaches `_last_send` (`posture.py:25-30`): list gives AttributeError, junk and "" give ValueError, "noon" fails the number test.
27. test_compaction_sends_at_once (:108): Agree Investigate, claim not checked only.
28. test_prompt_after_compaction_does_not_send_twice (:113): Agree Investigate, claim not checked only.
29. test_compaction_with_no_state_sends (:119): Agree Investigate, claim not checked only.
30. test_compaction_leaves_other_sessions (:123): Dispute Investigate; it is a Rewrite, a fault in the test itself. Line 126 cannot fail: if a compaction wrongly reset a shared clock, session 2's clock would move from T0 to T0+MIN, and its next prompt at T0+2*MIN is still inside the interval, so it is skipped either way. The record's "126 catches a shared state file (reasoned)" is wrong. The check needs session 2's prompt at a time that sends only if its own clock was not touched (for instance `T0 + RESEND_MINUTES*MIN`, after a compaction at a later time of session 1). Also the state path is per session (`posture.py:22`), so the bug is reachable only by a path change there.
31. test_compaction_with_missing_file_is_loud (:128): Agree Investigate, claim not checked only.
32. test_no_session_id_sends_and_writes_nothing (:133): Agree Investigate, claim not checked only, and whether the harness ever omits `session_id` (question 11) is unshown.
33. test_all_three_parts_run (:196): Agree Investigate, claim not checked only. Line 202 holds because `stall` really runs through the real percall (`percall.py:136`).
34. test_resend_rule_still_holds (:205): Agree Investigate, claim not checked only. Overlaps 20, as stated.
35. test_contexts_are_joined_whole (:209): Agree Investigate, claim not checked only; the "\n\n" is `prompt.py:22` JOIN, typed in the test (a copy).
36. test_messages_are_kept_whole_and_named (:215): Agree Investigate, claim not checked only; a prompt with no `prompt_id` is hand-made.
37. test_posture_crash_leaves_the_other_two (:223): Dispute Keep; Investigate, claim not checked only (typed prompt; crash from a patched body). Lines 227-230 each can fail (`prompt.py:41-62`).
38. test_percall_crash_is_logged_and_leaves_the_other_two (:232): Dispute Keep, same.
39. test_waiver_crash_leaves_the_other_two (:243): Dispute Keep, same.
40. test_part_that_cannot_import_leaves_the_other_two (:250): Dispute Keep, same. The ModuleNotFoundError is real, but the payload is typed.
41. test_unpaired_surrogate_in_the_prompt_stops_no_part (:258): Dispute Keep, same; the capture needed is a real prompt that holds a lone surrogate, and whether one can reach the hook is unshown (the comment is a claim). Line 261 does fail if `prompt.py:40` used utf-8 for the re-encode.
42. test_over_cap_context_is_said_out_loud (:264): Agree Investigate, claim not checked only; the 10000 has a cited source in `prompt.py:19-21` (see above), not "not found". Line 269 types the owner's text "%d characters".
43. test_at_cap_context_says_nothing (:271): Agree Investigate, claim not checked only. 273 fails if `>` becomes `>=` (`prompt.py:54`).

## test_publish.py (40 records: 37 agree, 3 dispute)

Hook code read: `publish_gate.py` whole, `publish.ps1` whole. The 20 and 2000 are `publish_gate.py:14`, whose comment says "The user's decisions of 2026-10-10". Only "over 20 lines" is in a filed line (`review-and-hooks.md:22`); 2000 has none (grep of `docs/agents/rulings/`). The comment is a claim.

44. test_21_lines_without_a_record_is_refused (:100): Agree Investigate, claim not checked only.
45. test_20_lines_and_2000_characters_pass (:107): Agree Investigate (2000 has no filed ruling: question to the user). My count: 19*99 + 18 newlines + 1 + 100 = 2000, and the owner counts UTF-16 units, equal for ASCII.
46. test_2001_characters_are_refused (:112): Agree, as 45.
47. test_blank_lines_and_crlf_do_not_count (:116): Agree Investigate, claim not checked only. Line 117 does fail if blank lines count (58 lines).
48. test_clean_record_passes_and_a_later_blocked_one_refuses (:119): Agree Investigate, claim not checked only (`review_rules.py:193-196` last verdict wins).
49. test_edited_plan_passes_when_its_file_was_signed_off (:127): Agree Investigate; the rule "later edits pass" is cited in `publish_gate.py:75` as the user's, but Hydra CLAUDE.md says only "each document gets it once"; no filed ruling. Question to the user.
50. test_edited_artifact_page_passes_when_its_file_was_signed_off (:135): Agree, as 49.
51. test_path_match_ignores_case_and_slash_style (:144): Agree Investigate, claim not checked only.
52. test_blocked_record_for_the_path_does_not_pass_it (:149): Agree Investigate, claim not checked only.
53. test_blocked_after_clean_path_does_not_count (:154): Agree Investigate, claim not checked only.
54. test_missing_page_outside_a_gated_repo_is_not_the_gates_business (:162): Agree Investigate, claim not checked only. Line 165 fails if `refusal` read the missing file before `gated_repo` (`publish_gate.py:94-96`).
55. test_typed_waiver_passes_and_an_untyped_one_does_not (:167): Agree Investigate, claim not checked only. Line 173 types the phrase, which `review_rules.waiver_phrase` (:77) owns; the test should call the owner. Not a reason to change the verdict.
56. test_derive_once_waiver_does_not_count (:176): Dispute Investigate; it is a Rewrite (fault in the test). The waiver record has kind "derive-once" and the typed text says "waive derive-once": two things differ at once. `_waiver_records` filters by kind (`review_rules.py:120`) and `WAIVER_PHRASE["doc-review"]` rejects that text (`:27,128`). Removing either one alone leaves line 182 green, so it catches no single fault. The record's "182 catches kinds mixed" is wrong. A rewrite sets one thing at a time: a derive-once record with the doc-review text typed, and a doc-review record with the derive-once text.
57. test_repo_without_marker_is_never_gated (:184): Agree Investigate, claim not checked only. The `nogit` folder sits under a temp folder; had the temp folder lain inside a repo, `gated_repo` would raise (`publish_gate.py:54-57`); not a fault here.
58. test_artifact_publish_reads_file_path (:191): Agree Investigate, claim not checked only (`publish_gate.py:117-119`).
59. test_hook_script_fails_closed_on_bad_input (:201): Agree Keep, with a note. The input is not a hook call but malformed bytes on a real subprocess's stdin, which is the transport the claim is about, and no recorded payload exists for it. If the main session reads question 3 strictly for hook tests, it is Investigate. Lines 208-209 fail if `common.run` closed mode stops denying.
60. test_repo_path_record_passes_a_plan_in_that_repo_only (:220): Agree Investigate, claim not checked only.
61. test_full_path_record_still_passes (:228): Agree Investigate, claim not checked only.
62. test_refusal_names_the_commit_and_review_route (:233): Agree Rewrite (typed copy of `publish_gate.py:104-105`; line 236 keeps what it has today). The same route sentence also sits in README and CLAUDE.md: a copy.
63. test_key_flag_is_gone_and_so_is_doc_key_ps1 (:240): Agree Rewrite. Line 247 copies `publish_gate.py:136` exactly; 246 and 248 stay.
64. test_refused_check_never_calls_gh (:286): Dispute Keep; Investigate, claim not checked only. The body is `lines(30)`, a made-up text ("xxxxxxxxa" rows), never a real plan or note; the real pwsh, real script and real git are used, but the input is hand-built. Also line 290 looks for the word "review", which any reason text containing "reviewer" meets.
65. test_check_logs_refusal (:292): Dispute Keep; same, Investigate, claim not checked only. Lines 298-302 can each fail (`publish_gate.py:128-133`, `common.log_refusal`); the input is the same typed body.
66. test_passing_check_calls_gh (:304): Agree Investigate, claim not checked only (real `gh issue comment` shape not seen).
67. test_clean_record_in_the_copy_state_lets_a_pr_through (:310): Agree Investigate, claim not checked only.
68. test_body_flags_in_extra_args_are_refused (:319): Agree Investigate, claim not checked only (`publish.ps1:30,44-46`).
69. test_other_ways_to_give_gh_text_are_refused (:325): Agree Investigate, claim not checked only.
70. test_a_flag_shaped_target_is_refused_and_a_capital_B_is_not_a_body_flag (:331): Agree Investigate, claim not checked only (`publish.ps1:48`; `-B` is in `$ValueLetters`, `:24`).
71. test_a_value_stuck_on_a_short_flag_is_not_read_as_flags (:339): Agree Investigate, claim not checked only.
72. test_text_flags_behind_switches_in_one_word_are_refused (:349): Agree Investigate, claim not checked only.
73. test_every_short_flag_letter_is_read_the_way_gh_reads_it (:358): Agree Investigate, claim not checked only. The two letter lists at :361 and :365 are typed copies of `publish.ps1:21,24`, so the test agrees with the script by construction; the owner is real `gh <cmd> --help`.
74. test_a_tag_with_plus_or_at_is_a_target (:371): Agree Investigate, claim not checked only; whether a release tag with `+` or `@` can occur is unshown.
75. test_a_target_starting_with_a_dash_is_refused (:378): Agree Investigate, claim not checked only; overlaps 70.
76. test_issue_opens_with_title_and_body_file (:389): Agree Investigate, claim not checked only.
77. test_issue_needs_a_title (:395): Agree Investigate, claim not checked only (`publish.ps1:50`).
78. test_long_unreviewed_issue_is_refused_and_a_signed_off_one_passes (:401): Agree Investigate, claim not checked only.
79. test_release_edit_takes_notes_from_the_body_file (:413): Agree Investigate, claim not checked only (`publish.ps1:59`).
80. test_release_edit_passes_a_title_when_given (:419): Agree Investigate, claim not checked only.
81. test_release_edit_needs_a_tag (:424): Agree Investigate, claim not checked only (`publish.ps1:49`).
82. test_long_unreviewed_release_notes_are_refused (:430): Agree Investigate, claim not checked only.
83. test_text_flags_are_refused_for_the_new_kinds (:436): Agree Investigate, claim not checked only.

## Splits (each its own line; the main session picks the exit)

- `posture.py:16` says 60 minutes is "the user's answer to Q8"; the plan T2 says it is "my pick, not a user decision" and Q8 recommends 60; no filed answer found.
- `publish_gate.py:14` holds 20 and 2000; `test_publish.py` types 19*99+100 and `lines(21)` as its own copies of them (:108-113, :101).
- `publish.ps1:21,24` hold the letter lists; `test_publish.py:361,365` type the same letters again.
- `shell_output.py:35` holds the Bash tail; `test_percall_rewrite.py:25` types it again; `progress.py:64` holds the first-call wording that `:26` types again.
- `review_rules.waiver_phrase` (`:77`) builds the phrase with a 10-character key prefix; the accepting patterns take 7 to 64 characters for doc-review and 7 to 40 for derive-once (`:26-27`), and `_waiver_records` needs at least 7 (`:120`). Two answers to how long a typed prefix is. Not an auditor finding in the records.
- `percall.py:14` holds `--whole-library`; `test_percall_rewrite.py:73,135` and others type it again.
- The refusal route sentence at `publish_gate.py:104-105` is typed again at `test_publish.py:235`.
- The owner caller list for percall in the record omits `prompt.py:18`.

## Result

Records checked: 83. Agree 68, dispute 15 (percall 5, posture 7, publish 3). The 15 disputes: percall records 8, 9, 16, 17, 18; posture 21, 30, 37, 38, 39, 40, 41; publish 56, 64, 65 (numbering above). No record unreached. Not stopped on anything.
