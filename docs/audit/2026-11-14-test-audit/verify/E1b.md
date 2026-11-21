Agent adb68c300f281d0d2 (task T5-V-R24-E1b, session 16892967-2cf8-4437-a7cd-e3dad250f621)

# Verify E1b: test_bash_windows_rule.py, test_delete_rule.py, test_media_rule.py

Counts: 47 + 68 + 31 = 146 test cases, 146 records, no case without a record, no record unreached. Every duplicate name has its own record (`test_refused_in_every_subagent` at delete :260 and :346, `test_reason_names_the_scratch_folder_and_both_worktrees_folders` at :242 and :330). Everything is "by reading" unless a line says "proven by". I ran nothing. The hooks repo was read at its working tree.

What I read: all three test files in full; `bash_windows_rule.py` 1-247, `delete_rule.py` 1-188, `media_rule.py` 1-56 in full; `percall.py` 45-140; `common.py:42` and `prompt.py:10-34` by range; `tests/probe_results.md` lines 93, 98, 100; Hydra `docs/agents/rulings/review-and-hooks.md` whole; D189 (decision log :891) and D273 items 2 and 3 (:1266, :1419); `gates/README.md:19` by grep.

## Disputes first

Verdicts I dispute (each is also in its record below):

1. `test_the_limit_is_the_users_number` Keep -> Investigate. The "filed ruling" the record cites (`review-and-hooks.md:40`) is under `## Unconfirmed`, not `## Confirmed`. The only line naming the user choosing 1,000 is :43, also Unconfirmed. The preamble line 29 is a hook description, not a ruling. The Confirmed line :11 says the user is "not too concerned about the exact number". So 1000 has no citable reason, and the test compares the constant with a typed copy of itself (a snapshot of the number).
2. `test_scanner_is_the_one_reader` Keep -> Rewrite. The detector (line 316-317) only sees a string constant exactly equal to `'`, `"` or `'"`. A second reader that uses `chr(39)`, the reversed pair `"'`, or a regex held in a function body is not seen. The "independent owner" is the same file's own docstring (:5).
3. `test_the_boundary_with_a_commit_in_the_command` Investigate (RC) -> Rewrite, and `test_a_cat_heredoc_message_is_commit_text` the same. The numbers MAX-4, MAX-3, MAX-2 are raw results with no working in the test. They are not symmetric: the text before the `&&` counts " && " (4 characters), but the text after counts only "&& " (3), because `_commit_text` holds the space before the operator (`bash_windows_rule.py:165`, `held.update(range(piece[0].start, scan.ends[last].at))`, where `at` is the operator's own index). A reader cannot tell this from the test. The record's "readable" is wrong.
4. `test_every_refusal_has_instead` Investigate (RC; owner not opened) -> Rewrite. The owner of 300 is `common.py:42` (`"reason": reason[:300]`), a bare literal. The test types its own 300 (line 307), a copy. If common.py changed to 200, the test would stay green while the claim in its message ("the refusal log keeps the first 300 characters") became false.
5. `test_switch_force` (delete :355): the record's "overlaps :346 (same words)" is wrong. `git switch --force` is not in :346's list (it has `switch -f` and `switch --discard-changes`). Verdict stands (Investigate) but it is not a duplicate.
6. `test_powershell_list_of_paths` (delete :218): the record's claim "a comma list is judged path by path" is wrong. `NOT_LITERAL` holds "," (`delete_rule.py:26`), and `_inside` refuses any token containing it (:91), so a list of two inside paths is refused too. The test has one outside path, so it cannot tell "judged path by path" from "any list refused".
7. `test_script_entry_point` (delete :406) Keep -> Investigate, and the three `Crash` tests with it (:388, :395, :401). `delete_rule.body` and the `__main__` line (`delete_rule.py:181-187`) have no production caller: `settings.json:158` registers `percall.py` only; `prompt.py:18` lists `posture`, `percall`, `waiver` (not delete_rule); `percall.py:116` calls `delete_rule.check`, never `body`. grep of `gates/*.py` for `delete_rule` found no other caller. So four tests pin a standalone script nobody runs. The crash path that matters is percall's, which `test_bash_windows_rule.py:161` and `test_media_rule.py:217` already drive through the real `percall.body`. Failure the entry test names (script not wired) can no longer happen in practice.

Splits (each its own line):

- S1. "Is this a git commit?" is answered by quote position. `git "commit"` counts (`_is_git_commit` reads `.value`, :153, test :108), but `"git" commit` does not (`_command_at` demands `raw == value`, :141, pinned as refused at test :114). Same question, two answers.
- S2. Delete rule and ruling words. The only Confirmed line (`review-and-hooks.md:17`) says "recursively delete only inside its scratch folder or a worktree that sits in a `worktrees\` folder". The code adds forced non-recursive deletes (`rm -f notes.txt` refused, test :196; `del -Force` :240) and a long git list. None of that has a Confirmed line: the test docstrings (delete :287-288, :338-339) cite "the user's ruling, 2026-10-25"; the only ruling lines I can find for that date are :13-:17 and the Unconfirmed :46-:47, and none names find, `git switch`, `git reset --merge` or `git worktree prune`. grep of `docs/agents/rulings/` for those words found nothing.
- S3. Worktree location. The Confirmed ruling words (:17) are "a worktree that sits in a `worktrees\` folder". The code accepts only `...\hydra-test\.claude\worktrees` and `C:\Users\Patrick\.claude\worktrees` (`delete_rule.py:17-18`). This verify worktree sits in `...\hydra-test\worktrees\test-audit-records`, which the code does not accept (the preamble says the same). Every path in the tests is a copy of the first two folders, so no test touches a worktree in the folder where audit worktrees actually are.
- S4. Where cwd is read. `delete_rule.py` never reads `data["cwd"]` (no `cwd` in the code; the comment at :15-16 says so), while `media_rule.py:50` judges `data.get("cwd")` for a yt-dlp call with no output. Different questions, so not a bug, but the delete tests (:57, :73, :112, :142, :150, :242, :260, :346) vary `cwd` as if it mattered. It does nothing; those loops repeat the same call (the :150 loop makes 78 calls that are 26 different ones).
- S5. The 1,000 limit: preamble line 29 states it, the hook comment calls it "the user's number", and no Confirmed line holds it (see 1).
- S6. Bash rule against the quoted-operator reading. `delete_rule.pieces` cuts at every `;`, `&`, `|` even inside quotes (`OPERATORS.split`, :77), so `git commit -m "a; rm -rf C:/Users/x"` is cut into a piece starting `rm -rf C:/Users/x"` and refused, while `bash_windows_rule._scan` reads quotes properly. The tests :105 and :358 pin only quote shapes with no operator inside, so the hole is untested. Not a record dispute; a gap.
- S7. Media rule and delete rule words. `media_rule._segments` cuts only at an operator standing alone as a word (:23), while `delete_rule.pieces` cuts at operator characters anywhere. media_rule borrows `dr._roots`, `dr._inside`, `dr._norm`, `dr.OPERATORS`, `dr._word` and ignores worktree roots on purpose (test :113).

Rewrites that propose a saved expected-output file: none. No record proposes one.

Callers the records missed (question 2). The records list `percall.py` and `shell_output.py:59,132`. My search of `gates/*.py`, `settings.json`, `gates/README.md` also finds: `bash_windows_rule.py:14,141` calls `delete_rule._word`; `media_rule.py:4,14,23,48,52` calls `dr._word`, `dr.OPERATORS`, `dr._roots`, `dr._norm`, `dr._inside`. A change in `delete_rule` therefore changes the Bash rule's command reading and the media guard; the delete tests do not say so. For `bash_windows_rule` and `media_rule` the record's callers hold (`percall.py:116/129` and `:117/133`).

Hook decisions with a real measurement the records missed. `gates/tests/probe_results.md` holds Git Bash measurements: line 93 (a table of doubled-backslash cases), line 98 (`cmd`: `cmd.exe /d /c "echo hi"` hangs at its prompt; with `MSYS_NO_PATHCONV=1` or `//c` it prints `hi`; `cmd //c dir /b C:/Windows/system.ini` fails; `cmd //c "echo hi" > /dev/null` exits 0), line 100 (`git show origin/main:.gitignore` fails bare, single- and double-quoted; `main:.gitignore` and the PATHCONV form work; `echo` and `grep` are not rewritten). The records say "probe_results.md not read" and mark question 3 "claim not checked". Those lines are a real measurement of the mechanism and the exact commands appear in the tests. "Proven by" probe_results.md lines 93, 98, 100 for the lines named below. The file is not in the records folders, so the main session should confirm it before a Keep follows.

Question 3 rule applied to the three files. Every test here feeds the hook a hand-typed payload, so it is "claim not checked" unless a measurement above covers it. I agree with that default, and with "Investigate (RC)" as "claim not checked only". The two Keeps (:52 in the bash file and :406 in the delete file) are the ones I dispute above; the third Keep (`test_scanner_is_the_one_reader`) too.

---

## test_bash_windows_rule.py (47 records, 47 checked)

Shared header of the record: Q2 holds (`percall.py:116,129`; `bash_windows_rule.check/reasons` :233-246). "Rule numbers: 1,000 (ruling `review-and-hooks.md:40`)" does not hold (line 40 is Unconfirmed, see dispute 1). The five shapes named in preamble line 29 are real.

- test_length_boundary (:48): Agree Investigate (RC). Line 49: `_too_long` uses `>` (:174); `>=` goes red here. Line 50: `> MAX+1` goes red. Working from the constant, so a changed MAX stays green; line 53 pins it.
- test_the_limit_is_the_users_number (:52): Dispute. Keep -> Investigate (dispute 1). Line 53 can fail only if the constant moves; no Confirmed line says why 1000.
- test_length_is_raw_len (:55): Agree Investigate (RC). Line 56 `len(text)` -> byte length goes red (é is two bytes in UTF-8). Whether the real Bash tool measures characters is not shown; probe_results.md has no line on it.
- test_the_refusal_names_the_write_tool_and_no_commit_route (:59): Agree Rewrite, partly for another reason. Line 61 holds ("Write tool" is at :212-213). Lines 62 and 63 are not "old text" only: preamble line 61 forbids `-F`, so "the refusal never suggests `-F`" is a property. "budget" has no source. Rewrite keeps 61 and 62, drops 63.
- test_a_commits_text_does_not_count (:69): Agree Investigate (RC). Lines 70-71 go red if `_commit_text` returns an empty set or stops at `&&`. Rule is `review-and-hooks.md:40` (Unconfirmed).
- test_the_boundary_with_a_commit_in_the_command (:74): Dispute. Rewrite (dispute 3). Line 76 passes at 1000 including " && "; line 78 passes at "&& " + 997 = 1000; the 4 and the 3 are not in the test.
- test_each_commit_is_taken_out (:81): Agree Investigate (RC). Line 82 goes red if only the first commit piece is held.
- test_a_long_heredoc_before_a_short_commit_is_refused (:84): Agree Investigate (RC). Line 85: the `cat` heredoc body belongs to piece 0, which is not a commit. Line 86: body owner is index 0 (`bodies[owner]`, :75), and `_commit_text` only holds `bodies[k:last+1]` (:166); a change to attribute it to the commit goes red.
- test_a_multi_line_quoted_message_is_commit_text (:88): Agree Investigate (RC). Lines 89, 90 pass because the quoted text is inside the commit piece's words.
- test_a_cat_heredoc_message_is_commit_text (:92): Dispute. Rewrite (dispute 3). Lines 94-95 are fine; 96 and 97 carry the 3 vs 2 offset.
- test_an_operator_inside_the_messages_substitution_stays_in_it (:99): Agree Investigate (RC). Line 100 goes red if the `depth` check (:163-164) is dropped.
- test_a_heredoc_the_commit_opened_is_commit_text (:102): Agree Investigate (RC). The preamble forbids `-F -` for agents (line 61); no hook refuses it (grep for `-F` in `gates/*.py` finds nothing); so a long `-F -` body being exempt is the hook's choice with no ruling. Goes to the user.
- test_first_word_forms_that_count (:106): Agree Investigate (RC). Split S1. Each of six forms goes red if `_command_at`/`GIT_OPTIONS_WITH_VALUE` drops it.
- test_only_a_git_commit_piece_counts (:111): Agree Investigate, but not "claim not checked only". Line 114 expects `git --no-pager commit -m <3000>` and `"git" commit -m <3000>` to be refused as too long; both are real commit spellings. Whether that is wanted is a question for the user (S1). The record's production finding holds (`bash_windows_rule.py:25` lists `-C`, `-c` only).
- test_the_other_parts_still_read_a_commits_text (:117): Agree Investigate (RC). Lines 119-122 each go red if the commit's text were exempt from its part. The `[path quote]` line works because the whole command is one commit piece with an unclosed quote.
- test_refused_in_main_and_in_a_subagent (:133): Agree Investigate (RC). Real `percall.body` on a typed `cmd /c dir`. Proven by probe_results.md:98 that `cmd /c` hangs; the percall routing is the hook's real decision. Closest to Keep-eligible, after the main session confirms the probe.
- test_status_call_passes_through_percall (:139): Agree Investigate (RC). The command is built from `progress.STATUS_PREFIX`, not a recorded status call. Line 144 goes red if `percall.py:112` loses `not progress.is_status_call(data)`.
- test_a_spent_agent_can_still_make_a_long_commit (:146): Agree Investigate (RC). Line 152 proves spent; 154 goes red if the Bash rule refused the commit, or if the pass-list stopped letting `git` through. The commit string is 1016 characters in total, so it also tests the commit exemption.
- test_powershell_tool_not_read (:156): Agree Investigate (RC). Line 158 is weak: `assertIsNotNone(judged(command))` passes on length alone. Line 159 goes red if `reasons` lost its `tool_name != "Bash"` return (:235).
- test_crash_denies (:161): Agree Investigate (RC). Overlaps `test_common.py:60`, `test_delete_rule.py:388`, `test_media_rule.py:217`. Line 169 depends on the TypeError at :239; in the main session `_library_refusal` and `_key_refusal` run first on a list command (`percall.py:129`); `_library_refusal` uses `str(...)` so it does not crash first (`:48`); `_key_refusal` not opened.
- test_backslashes_single_heredoc_bare_refused (:173): Agree Investigate (RC). Proven by probe_results.md:93 for the single-quote and bare forms (table); the heredoc form not seen there.
- test_backslashes_double_pass (:178): Agree Investigate. The record says the ruling was "not found". It exists at `review-and-hooks.md:41` (Unconfirmed): "backslash pairs in double quotes stay out". Not citable as Confirmed.
- test_single_backslashes_pass (:181): Agree Investigate (RC). Passes because `PAIR` needs two backslashes in a row (:26).
- test_unclosed_quote_is_inside (:184): Agree Investigate (RC). Line 185 refused (single quote unclosed holds the rest), 186 passes (double). Goes red if `_scan` treated the end as bare.
- test_heredoc_runs_to_its_end_word_and_no_further (:188): Agree Investigate (RC). Line 189-191 each go red if the end-word/dash/never-ends logic (:66-75) changed.
- test_two_heredocs_on_one_line (:193): Agree Investigate (RC). Goes red if `pending` kept only one.
- test_here_string_has_no_body (:196): Agree Investigate (RC). Goes red if `<<<` matched as a heredoc (group 1 unset, :103).
- test_cmd_slash_refused_and_pathconv_passes (:201): Agree Investigate; lines 202-203 are proven by probe_results.md:98 (the same `cmd.exe /d /c "echo hi"`). Line 204 `cmd --version` is not in the probe. So only line 204 is unchecked.
- test_cmd_by_its_last_path_part (:206): Agree Investigate (RC). Goes red if `_word` stopped taking the last path part.
- test_leading_assignment_is_not_the_command (:209): Agree Investigate (RC).
- test_cmd_must_pass_shapes (:212): Agree Investigate (RC). `cmd //c` (line 213) is proven by probe_results.md:98. The `bash -c "cmd /c dir"` and `find -exec cmd /c` passes (lines 217-218) are not measured anywhere in probe_results.md (grep found no wrapper or `-exec` text). The record says it "would fail at runtime", which is also unmeasured. "On purpose" (docstring :7) is a claim.
- test_a_redirect_target_is_not_a_flag (:222): Agree Investigate (RC). Line 223 is proven by probe_results.md:98; line 224 (`2>`) is not.
- test_a_later_single_slash_word_is_still_read (:226): Agree Investigate (RC). Line 227 is the exact probe command (probe_results.md:98), so this line is proven by it.
- test_each_bare_operator_ends_a_piece (:229): Agree Investigate (RC).
- test_an_escaped_operator_ends_no_piece (:234): Agree Investigate (RC). Passes because `\;` is consumed as an escape pair (:122-126).
- test_pathconv_in_another_piece_does_not_count (:237): Agree Investigate (RC).
- test_ref_dotpath (:242): Agree Investigate (RC). Lines 243-245 are proven by probe_results.md:100 (all three quote forms fail; the PATHCONV form works). The passes at 246-248 (`python -c`, heredoc text) are covered there only for echo and grep.
- test_trailing_backslash_quote (:253): Agree Investigate (RC). No probe line found for it.
- test_path_quote_must_pass_shapes (:260): Agree Investigate (RC). Goes red if `PATH_QUOTE` matched an escaped `\"` inside a closed quote.
- test_a_shift_is_not_a_heredoc (:270): Agree Investigate (RC). Line 271 goes red if `<<2` became a heredoc (the body would hide `a\\b`); 273 if it swallowed `cmd /c dir`.
- test_a_comment_holds_no_quote (:276): Agree Investigate (RC).
- test_ansi_c_quote_reads_an_escaped_quote (:282): Agree Investigate (RC). Line 284 refuses `$'a\\b'`, where bash reads `\\` as one backslash on purpose; the refusal text ("halved or dropped") does not describe that case. No ruling either way.
- test_a_substitution_in_double_quotes_quotes_afresh (:286): Agree Investigate (RC).
- test_odd_commands_do_not_crash (:292): Agree Investigate (RC). Lines 293-294 have no assertion beyond "no exception". Line 296 goes red if `reasons` stopped coping with missing `tool_input`.
- test_every_refusal_has_instead (:299): Dispute. Rewrite (dispute 4). The `[path quote]` reason puts "Instead:" at about index 200, so the 300 is within about 100 of failing; that is the one real use for the check.
- test_reasons_lists_every_part_that_hits (:309): Agree Investigate (RC). Line 311 types the PARTS order as a literal list, a copy of `PARTS` (:210-229).
- test_scanner_is_the_one_reader (:313): Dispute. Keep -> Rewrite (dispute 2). Lines 324-327 (one scan per call, every part given the Scan) are sound by reading; the weak line is 316-318.

Bash file totals: 47 records; agree 41 (all Investigate (RC) or Rewrite as recorded), dispute 6 (:52, :74, :92, :299, :313 changed verdict; :59 agreed with a different reason, counted as agree below). Counting disputes as a changed verdict: 5; agreed: 42.

---

## test_delete_rule.py (68 records, 68 checked)

I read `delete_rule.py` in full; the record said "body of `_piece_refusal`/`check` not read". Q2 holds for `percall.py:116` (subagent only; `_main_session` never calls it, :126-139), and misses the callers in Callers above. Q3: ruling `review-and-hooks.md:17` is a Confirmed line and states the roots rule. All commands are hand-typed with paths copied from real shapes; "claim not checked" stands. Q7: not read (no `git log -S`). Default for every row below: Agree Investigate (RC) unless stated.

Allowed class
- test_rm_rf_literal_scratch_path (:48): Agree. Goes red if `_inside` stopped accepting a scratch child (:96-101).
- test_rm_r_quoted_scratch_path (:51): Agree. `_inside` strips one pair of quotes (:90).
- test_rm_rf_posix_drive_form_of_scratch (:54): Agree. `_norm` turns `/c/` into `c:` (:40-41).
- test_remove_item_in_own_worktree (:57): Agree. Needs two path parts under a worktrees folder (:100).
- test_remove_item_literalpath_in_hydra_worktree (:60): Agree.
- test_two_scratch_paths (:63): Agree.
- test_backgrounded_literal_delete (:66): Agree. `&` ends the piece (:29).
- test_case_slashes_and_trailing_separators (:69): Agree. The `cwd=HOOKS_WT.lower()` at :73 does nothing (S4); the case and slash parts are real.
- test_msys_form_of_a_worktree_folder_is_still_refused (:76): Agree. Misfiled in `Allowed`; it asserts refusal. The `cwd` is default and inert.
- test_plain_rm_without_flags (:81): Agree. The code agrees with the preamble ("recursive or forced"); the Confirmed line says "recursively".
- test_plain_remove_item_without_flags (:84): Agree.
- test_no_delete_word_is_never_refused (:87): Agree. Eight commands, none a delete word, so each passes because `_piece_refusal` returns None at :164.
- test_no_delete_word_passes_even_without_scratch_or_worktree (:93): Agree.
- test_other_tools_are_not_read (:96): Agree. Goes red if :168 lost the tool check.
- test_rm_inside_grep_pattern_without_flags (:99): Agree. Passes because the first word is `grep`.
- test_grep_for_a_find_delete_pattern (:102): Agree, and it duplicates the first line of :326 (same `grep` shape, passes on the first word). Remove-candidate for one of the two; :326 line 328 (`find . -name "-delete"`) is distinct.
- test_delete_word_not_in_command_position (:105): Agree. S6: quote shapes with no operator in them.
- test_rm_rf_inside_scratch (:109): Agree Remove. Same shape as :48 (`rm -rf <scratch>/x` vs `/build-cache`).
- test_own_worktree_that_is_not_the_session_folder (:112): Agree Investigate. The test name says "own worktree", but the hook cannot tell (comment :15-16): any worktree under the two folders passes. Line 114 etc. pass for another agent's worktree too. `cwd` inert (S4). S2, S3.

Refused class
- test_plain_rm_rf_of_a_home_folder (:128): Agree.
- test_plain_git_clean (:131): Agree. Preamble line 39 only; no Confirmed line (S2).
- test_variable_path (:134): Agree. `$` in `NOT_LITERAL`.
- test_variable_inside_scratch_path (:139): Agree.
- test_main_checkout_path_from_main_checkout (:142): Agree. `cwd` inert (S4); the claim "not a root even from the main checkout" holds trivially.
- test_root_that_is_a_prefix_of_a_sibling (:145): Agree. Goes red if `startswith(r + "/")` (:97) lost its slash.
- test_worktree_folder_or_worktrees_folder_itself (:150): Agree. 13 paths x 3 cwd x 2 commands; only the 13 x 2 differ (S4). Goes red if the `> 1` at :100 became `> 0`.
- test_other_worktrees_folders_do_not_count (:158): Agree.
- test_dotdot (:162): Agree.
- test_wildcards (:166): Agree.
- test_backtick_and_percent (:170): Agree. `rd /s /q` is in `CMD_STYLE_WORDS`.
- test_relative_path (:174): Agree.
- test_real_path_outside_roots (:178): Agree.
- test_mixed_scratch_and_real (:183): Agree.
- test_root_itself (:187): Agree.
- test_flag_after_path_and_split_flags (:191): Agree. Goes red if flags were only read before the path.
- test_force_without_recursive (:196): Agree. S2: `-f` alone is refused by the hook; the Confirmed line says "recursively".
- test_unclosed_quote (:199): Agree. A leftover `"` is in `NOT_LITERAL`.
- test_path_with_a_space (:202): Agree. Refused because the first half starts with a quote; the rule is "cannot read it as one path".
- test_no_path_at_all (:205): Agree.
- test_extra_tokens_after_path (:209): Agree. The redirect line (211) is refused only because `>` is itself a non-path token.
- test_single_ampersand_needs_no_spaces (:214): Agree.
- test_powershell_list_of_paths (:218): Agree Investigate, but the claim is wrong (dispute 6).
- test_braces (:221): Agree. Refused because both examples hold a comma; `{a}` with no comma is not in `NOT_LITERAL`.
- test_second_delete_in_chain (:225): Agree.
- test_full_path_and_case (:229): Agree.
- test_cmd_and_alias_forms (:234): Agree.
- test_reason_names_the_scratch_folder_and_both_worktrees_folders (:242): Agree. The line 248 negative is weak (`MAIN_CHECKOUT + ")"` can appear only if the main checkout is shown as a bare root). The two expected folders are typed copies of `WORKTREE_FOLDERS` (S3).
- test_missing_scratchpad_dir_refuses_scratch_like_path (:250): Agree. Matches probe_results.md:77 (the field is absent in a plain `claude -p`).

GitWords class
- test_refused_in_every_subagent (:260): Agree. `git branch -D`, `git restore .`, `git checkout -- .` have no Confirmed line (S2). cwd loop is inert (S4).
- test_branch_delete_with_force_written_out (:269): Agree.
- test_long_options_of_rm (:273): Agree. Lines 277-278: `--` and `--verbose` are not force flags (:86 requires `t[:2] != "--"`).
- test_harmless_git_passes (:280): Agree.

FindDelete class
- test_literal_starting_paths_inside_roots_pass (:299): Agree. Docstring's "user's ruling, 2026-10-25" not found in `## Confirmed` (S2).
- test_find_without_a_delete_passes (:305): Agree.
- test_no_starting_path_means_the_current_folder (:309): Agree.
- test_starting_path_outside_roots (:314): Agree.
- test_full_path_exe_and_chained_find (:321): Agree.
- test_quoted_find_words_are_not_read (:326): Agree. First line duplicates :102. Second line (328) passes because the token still carries its quotes, so `a == "-delete"` is false (:137).
- test_reason_names_the_scratch_folder_and_both_worktrees_folders (:330): Agree. Same claim as :242 for find; the two share the refusal text (`_instead`), so one of the two could go.

MoreGitWords class
- test_refused_in_every_subagent (:346): Agree.
- test_switch_force (:355): Agree, not a duplicate (dispute 5).
- test_git_words_inside_a_message_are_not_read (:358): Agree. Passes because the subcommand is `commit`; quote reading plays no part.
- test_global_options_that_take_a_separate_value (:361): Agree.
- test_harmless_neighbours_pass (:366): Agree. The line between `git checkout -- src/a.cpp` (passes) and `git checkout -- .` (refused) is a "." word; `git checkout -- src/` or `./` passes. No ruling.

Crash class
- test_crash_in_check_denies (:388): Investigate (RC) as recorded; add: it tests `delete_rule.body`, which no production code calls (dispute 7).
- test_non_string_command_denies (:395): Same; the TypeError comes from `check` (:171-172); percall's own path is covered at bash :161.
- test_body_wraps_refusal_and_allow (:401): Same.
- test_script_entry_point (:406): Dispute. Keep -> Investigate (dispute 7). The subprocess is real, but the script is not registered; the deny is the hook's real decision for `rm -rf $dir` only. A real process is a real mechanism, so its claim "the script prints a deny and exits 0" is true; the claim that this matters is not shown.

Delete file totals: 68 records; agree 66, dispute 2 (:406, and :109 is agreed). Counting by changed verdict: :406 only, plus the three Crash tests re-tied to the same finding but left as Investigate: so 1 disputed, 67 agreed. Wrong sub-claims noted in :218 (and the :355 overlap).

---

## test_media_rule.py (31 records, 31 checked)

I read `media_rule.py` in full; the record said not read. Rule source: D189 (`fix-decisions.md:891`: chose "Main session free (Recommended)"; the question text is shortened in the log), D273 item 3 (:1266, "The main session writes ffmpeg or yt-dlp output under `C:\ytdl` freely; other such output outside scratch asks first in the main session and is refused in a subagent") and D273's second item (:1419: the user chose "Leave it asking" for a relative yt-dlp `-o`). These are decision-log lines of the user's choices, not `## Confirmed` rulings. Q2 holds (`percall.py:117,133`). Q3: typed commands, "claim not checked"; D273 :1419 says the replay found no yt-dlp call in 14 days, so the yt-dlp shapes have no real call behind them. Default: Agree Investigate (RC).

MainSession
- test_ytdlp_template_into_ytdl_is_free (:52): Agree. `TEMPLATE.sub("x", ...)` (:52 of the hook) turns the template into a literal under `C:\ytdl`.
- test_cmd_variables_are_not_templates (:56): Agree. `%dir%` is hit by the frame-number branch `%d`, leaving `xir%`, which is still refused for its `%`; `%TEMP%` stays whole. Pass via the leftover `%`.
- test_a_quoted_path_with_a_space_asks_as_the_delete_rule_refuses_it (:60): Agree. The name's claim about delete_rule is not tested. The ask comes from the leftover `'` in `NOT_LITERAL`.
- test_ytdlp_paths_folder_is_free (:63): Agree.
- test_ytdlp_with_no_output_judges_the_current_folder (:67): Agree. The one place in these three hooks that reads `cwd` (S4).
- test_ffmpeg_into_ytdl_and_scratch_is_free (:71): Agree. Line 73 depends on `%04d` being read as a frame number.
- test_ffmpeg_output_elsewhere_asks (:76): Agree. Lines 78-79: the roots text lists scratch and `C:\ytdl` (:53-54 of the hook).
- test_ytdlp_output_elsewhere_asks (:81): Agree.
- test_no_scratchpad_dir_still_allows_ytdl (:84): Agree.

Subagent
- test_output_beside_an_input_outside_scratch_is_refused (:100): Agree. `scale=1280:-2` is not read as an output only because the second pattern needs a dot and `=` stops the match (comment at :10).
- test_output_inside_scratch_passes (:105): Agree.
- test_ytdl_is_the_main_sessions_folder_only (:110): Agree.
- test_worktree_is_not_a_media_root (:113): Agree. S7.
- test_unreadable_paths_count_as_outside (:117): Agree. Six shapes, each refused for a different cause (`$`, relative, `..`, a quote left in, `~`, relative template).
- test_glued_option_values_are_judged (:125): Agree.
- test_quoted_input_with_spaces_is_read_not_written (:130): Agree. Passes through the `odd(...)` quote-count skip (:31-37).
- test_null_sinks_and_pipes_write_nothing (:133): Agree.
- test_full_path_and_exe_spellings_are_ffmpeg (:139): Agree.
- test_one_bad_segment_is_enough (:144): Agree.

OtherCommandsUntouched
- test_commands_that_are_not_ffmpeg_or_ytdlp (:149): Agree.
- test_probe_only_ffmpeg_writes_nothing (:156): Agree.
- test_the_names_inside_quoted_text_are_not_commands (:163): Agree. The comment at :159 says these shapes came from a replay of recorded calls; the shapes are typed here and I did not see the recordings. Passes through the odd-quote skip (:32).
- test_a_name_lookup_does_not_spill_into_the_next_command (:169): Agree, with a fragile pass. In `(Get-Command ffmpeg ...).Source;` the leftover word `SilentlyContinue).Source` is saved from being read as an output only because ".Source" is six letters and `PATH_SHAPE` allows an extension of at most five (`{0,4}` after one letter, :10). A `.Path` would be read as an output (by reading; not run).
- test_an_operator_glued_to_the_output_is_not_part_of_it (:174): Agree.
- test_an_unquoted_filter_is_not_an_output (:178): Agree.
- test_an_output_after_a_quoted_filter_with_an_operator_is_still_judged (:181): Agree. Line 184 goes red if the odd-quote check at :34 stopped protecting the `;` inside the filter.

ThroughPercall
- test_main_session_ffmpeg_elsewhere_asks (:198): Agree. Real `percall.body`; `ask` comes from `percall.py:134`.
- test_main_session_ytdl_passes (:203): Agree. `assertNotIn(what, ("ask","deny"))` also passes when `what` is None; fine for "not asked".
- test_subagent_outside_scratch_is_denied (:207): Agree.
- test_subagent_inside_scratch_passes (:213): Agree.
- test_a_crash_in_the_rule_denies_in_both_sessions (:217): Agree. Same rule as bash :161, `test_common.py:60`. Patching `media_rule.check` works because `percall.py` looks it up at call time (:117, :133).

Media file totals: 31 records; agree 31, dispute 0.

---

## Totals

- test_bash_windows_rule.py: 47 records checked; agree 42, dispute 5 (:52, :74, :92, :299, :313).
- test_delete_rule.py: 68 records checked; agree 67, dispute 1 (:406); the Crash class (:388, :395, :401) carries the same finding but keeps its Investigate verdict.
- test_media_rule.py: 31 records checked; agree 31, dispute 0.
- Records not reached: none. Test cases with no record: none.
