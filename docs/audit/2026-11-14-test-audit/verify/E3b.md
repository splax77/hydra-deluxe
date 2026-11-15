Agent a20621a11fdb4438d (T5-V-R24-E3b)

# Verify E3b: records for test_review_rules.py (File 4 of records/E3.md)

Read: all of `C:\Users\Patrick\.claude\gates\tests\test_review_rules.py` (1,017 lines), all of `gates\review_rules.py` (476), `gates\common.py` (records, read_*), `gates\waiver.py`, `gates\percall.py` 1-110, `gates\publish_gate.py` 10-108, `gates\ref_judge.py` 225-300 and 376-415, `hooks\lib\doc_review_rules.ps1` (key function only), `settings.json` (grep), `docs\agents\doc-review.md:17`, `docs\agents\rulings\review-and-hooks.md:46-47`, `rulings\code-and-engine.md:50-51`, `reflog_moves.json` (grep). Not read: `prepass.py`, `test_review_dispatch.py`, `settings.local.json`, D-numbers (the records cite none for this file). Nothing was run. Every mark below is "by reading" unless it says otherwise; no record cites a run that exists in the records folders, so nothing here is "proven by".

## Count

The test file has 120 `def test_` cases. The records name 120 `**test_...**` entries. Compared by name with duplicates counted (the repeated names in LibraryAllow/KeyAllow and RealApp/Waivers match one for one): no test case without a record, no extra record. All 120 records were opened. Not reached: none.

## Findings that apply to many records

**F1 (question 2, missed callers).** The File 4 header gives the callers as `ref_judge.review_reason` (:241) and `tests\replay.py:102` only. The code has more, all read by me:
- `publish_gate.py`: `normalized` :24, `doc_key` :100, `_clean_keys` :80, `waived` :101, `waiver_phrase` :107, `git` :39 and :86. Registered in `settings.json:190`.
- `percall.py`: `human_prompt_text` :34, `is_library_allow` :51, `is_key_allow` :91. Registered in `settings.json:158`.
- `waiver.py`: `WAIVER_PHRASE` :19, `is_library_allow` :23, `is_key_allow` :25 (run by `prompt.py` on UserPromptSubmit, per its docstring; `prompt.py` not opened).
- `agent_stop.py`: `git`, `_first`, `_names`, `_DOC_MARKER_PATH` :57-104, `doc_key` :147. Registered in `settings.json:225`.
- `ruling_rules.py`: `git` :400, `normalized` :409.
The records for LibraryAllow, KeyAllow and HumanPromptText say `percall.py` and `publish_gate.py` were "not opened"; they are real callers. The `doc_key` / `normalized` records (:328, :331) name `publish_gate.py` as "not read". A caller the record missed is a finding for every record that rests on those functions (listed in the table).

**F2 (prefilter in human_prompt_text, missed by the records).** `review_rules.py:95` skips any transcript line that lacks the text `"human"` or lacks the prompt id, before the field checks at :101-104 run. Consequences, all by reading:
- A record with `turnOrigin:"sdk"` and no other `"human"` text (the fixtures at :367, :389 and subcase 1 of :893) is dropped by :95. The `turnOrigin != human` check at :103 never runs. Deleting :103 leaves all three green.
- Subcase `{"promptId":"p2"}` of :893 is dropped by :95 (the line has no `p1`). The check at :102 never runs. Deleting :102 leaves it green.
- So the records' Q8 line "fails if the filter dropped ..." is right only for `isMeta`, `isSidechain`, `toolUseResult` and `type` (:104, :101).

**F3 (comment-rule cases whose named trap is not the cause of the refusal).** In `ui_edit` the code review is already clean, so the only way to a refusal is `change_is_comments_only` returning False. A refusal-expected case can pass because any text difference counts. By reading:
- :614 `Best path` vs `Best  path` differs with or without string handling.
- :626 `one   ` vs `one` differs with or without raw-string handling (without it the lines still differ; only :631 needs the raw string to keep a blank line, because `finish` drops empty lines, :324).
- :725 old `"/* not a comment"` vs new `"/* still not */"`: if strings were not skipped, the old file's comment runs to end of file and the new one still has `int z = 3;` as code, so they still differ. The string-skip rule is caught by :708 (edit after the string) through the fixture, not by :725.
- :759 and :764 are in the record list below.

**F4 (shared 3:G-RR).** I agree it holds: every case builds a temp repo with real git; none uses a real ref move except ReflogReplay. No case proposes a saved expected-output file. Note for the main session only: ReflogReplay leans on a saved list, `reflog_moves.json` (294 moves, one `exception` entry), which is a snapshot file read by the test. The record does not propose adding one, so the ruling at `rulings/code-and-engine.md:50` ("we don't need golden anymore, remove it all") is not triggered by the record, but the existing file is the same kind of thing.

## Splits (one line each)

1. Two checks for "did the user type this" sit inside `human_prompt_text`: the text prefilter (:95) and the field checks (:101-104). They overlap, and the tests cannot tell them apart (F2). Whether `prepass.py` is a third owner (the records' split 1) I did not open; not verified.
2. "Same document" is answered twice: `review_rules._doc_review` :257 compares the recorded repo path string exactly; `publish_gate._signed_off` :83-88 compares absolute, lower-cased paths through `document_path_id`. Same question (has this file been signed off before), two comparisons. The record at :308 sees only the first.
3. The doc key has an older second owner on disk: `hooks\lib\doc_review_rules.ps1` `Get-DocReviewKeyOfBytes` (:167-176) is a SHA-1 of "blob N" plus the normalized bytes, a different key from `doc_key` (sha-256 of the normalized bytes). Nothing in `settings.json` wires a `hooks\*.ps1` script (grep for `.ps1` finds only `gates\` scripts); other settings files not checked.
4. `publish_gate.is_small_text` :29 re-does CRLF/CR to LF on top of `review_rules.normalized` (:24): a second copy of the line-end rule.
5. "Which docs are public": `review_rules.PUBLIC_DOCS` (:24) names four pages; `agent_stop.py:104` fingerprints every `.md` and the marker. Two answers to "which documents get a key" (the second is for signing off, so may be intended).
6. "allow library run" has a filed ruling line (`review-and-hooks.md:47`: "i want to be able to give explicit approval for a full library run"); "allow key run" has none under `## Confirmed` (grep for "key run" in `docs/agents/rulings/` finds nothing; `docs/agents/clone-hero-mcp.md:29` mentions it). Both go through the same matcher. Agree with the record. The trailing "." or "!" tolerance (`review_rules.py:52, :55`) has no source line I could find; agree.
7. The doc-key literal `d821ef04...` typed in two test files (record split 3): :335 is read here; the other copy (`test_review_dispatch.py:23`) not read by me.
8. Doc review "once": `docs/agents/doc-review.md:17` says "Once a fresh agent has signed off a version of a document, later edits to it pass with no new review", and `publish_gate.py:74-78` calls that "the user's rule (Hydra CLAUDE.md, Doc review)". So the page, the hook comment and the test agree on once per path. It is still not a line under `## Confirmed`, and `review_rules.py:23` says "the user's call, 2026-10-10" with no filed line I could find. The record's question stays open, but its "no ruling found" should name `doc-review.md:17`.

## Records

Format: name, agree or dispute, what I checked. "Verdict" is the record's verdict. Q3 for every Investigate "claim not checked only": I agree each is a git or transcript claim checked only on a hand-built repo or hand-typed record, and no real capture is named in the records folders. Where I write only "agree", I read the assertion lines and the production lines the record names and they hold as written.

### CodeReview (:99)

1. test_fast_forward_to_clean_key_passes: agree (Investigate, claim not checked only). :104 turns red if `_code_review` :249 ignored a clean key. Callers: F1.
2. test_unreviewed_tip_refused_naming_key_and_phrase: agree. :111 passes through the advice line (`explain_main_move` :426-427 holds the full `new`); :112 `waive derive-once <10>` from `waiver_phrase` :77-78.
3. test_blocked_verdict_does_not_count: agree. `_latest_verdicts` :190 keeps the last record per key; keeping the first would make :119 red.
4. test_cause_no_verdict_at_all: agree. The test text is a prefix of production's `... for %s or any commit before it on its branch` (:222), so a later wording change after the key would not turn it red (prefix check).
5. test_cause_blocked_verdict: agree (:216-217).
6. test_cause_commits_after_the_reviewed_one: agree with Investigate. Checked: count is `rev-list --count new ^k ^base` = 1 (:229). I did not open a source for the rule "a docs-only commit after review needs its own review"; `doc-review.md:17` is about docs only, so it does not say. The record's question stands.
7. test_cause_merge_that_differs_from_a_clean_merge: agree. Parents [old, k]; `merged=[k]` (:224). This and test_merge_with_extra_edit_refused share one scenario.
8. test_cause_names_a_waiver_the_user_did_not_type: agree on verdict; correction to the record's reason. The transcript line has no `"human"` text at all, so :95 drops it, not a missing `turnOrigin` check. The real disqualifier values are tested under HumanPromptText, and see F2.
9. test_cause_blocked_commit_earlier_on_the_branch: agree (:218-221).
10. test_cause_merge_of_a_branch_with_unreviewed_commits_is_commits_after: agree. `merged` is empty because t is not clean (:224), so :229 counts 2.
11. test_two_causes_are_joined_without_changing_a_path: agree. Doc cause text is :454-458 and the join is :466.
12. test_clean_merge_of_old_main_and_clean_key_passes: agree. `_carries` :239-243 compares merge-tree with the new tree.
13. test_catch_up_merge_on_branch_carries_the_review: agree.
14. test_merge_with_conflict_resolution_refused: agree. :237 passes through the advice (full key), not the cause (which has only `new[:10]`). Fine.
15. test_merge_with_extra_edit_refused: agree, Remove. Same scenario as record 7, whose `assert_cause` also checks the reason, so the weaker one is covered.
16. test_ci_setup_is_code: agree. `.github` is in `BUILTIN_CODE` :22.
17. test_change_outside_code_paths_needs_no_code_review: agree.
18. test_marker_gate_lines_name_the_code_paths: agree. Three assertions read: :263, :265, :267 each reach `_is_code` :180-185.
19. test_derive_once_marker_change_needs_review: agree (:19, :181).
20. test_bad_gate_line_raises: agree, Rewrite. The input `gate: ../up/` matches `_GATE_PATH` and is raised by the `..` check (:173), so `ValueError` is exact. Any exception, including a git failure, passes today. I also checked that `ref_judge.main` turns an exception into a refusal (:400-402), so "the move is refused" holds.
21. test_same_tip_passes: agree.
22. test_new_branch_old_zero_judges_the_whole_tree: agree. Three claims in one test (:286, :290, :291), as the record says; :291 hits the `_ZERO.match(new)` early return (:404), not any judging.

### DocReview (:294)

23. test_public_doc_without_record_refused: agree.
24. test_public_doc_with_exact_record_passes: agree.
25. test_same_path_with_earlier_clean_record_passes: agree, Investigate; see splits 2 and 8.
26. test_internal_doc_is_not_gated: agree.
27. test_marker_change_always_needs_its_own_record: agree. Without the `not _DOC_MARKER_PATH` guard at :257, :320 turns red.
28. test_marker_deletion_refused: agree (:439-446).
29. test_doc_key_drops_bom_and_crlf: agree. Q2 callers add F1 (`publish_gate`, `agent_stop`, `ruling_rules`). Q3: the real owners of the key all call the same function, so "the real README.md's key computed by `publish_gate.py` and by `doc_key`" is one function twice, not two checks. The capture the record asks for does not give an independent owner. Verdict stays Investigate (claim not checked only) with that note.
30. test_doc_key_matches_the_publish_gate_vector: DISPUTE the fix, and move the verdict to Remove. `publish_gate.py` has no key function of its own: it calls `review_rules.doc_key(raw)` (:100). The record's "compare with publish_gate's own key function" would compare the function with itself. The test's name claims something that cannot be tested. The real second owner is the old PowerShell key (split 3), a different key. The BOM/CRLF rule is already tested at :329 by an independent sha-256; non-ASCII does not matter because the key is a hash of bytes (no decode, :73-74). So the promise is covered by record 29, and the literal is a snapshot ("the key from one run", comment :332).

### NoMarker (:338)

31. test_repo_without_markers_passes_everything: agree.

### Waivers (:347)

32. test_human_typed_waiver_counts: agree.
33. test_sdk_waiver_does_not_count: DISPUTE. The record's assertion is right but the line that refuses is the text prefilter :95, not the `turnOrigin` check :103 (F2). Deleting :103 leaves this green. Verdict should be Rewrite: make the record carry the text `"human"` somewhere else in the line so :103 is the thing under test.
34. test_waiver_of_another_kind_does_not_count: DISPUTE the reason (verdict moves from "claim not checked only" to a Q8/Q11 Investigate). The record says it fails if `_waiver_records` :120 stopped matching kind. It would not: with the kind filter gone, `waived` still compares the typed text "waive doc-review ..." against the derive-once regex (:128) and refuses. And `waiver.py:19-22` writes the kind and the prefix from the same regex, so a waiver whose kind and typed text disagree cannot be made by the hook (only by editing waivers.jsonl). Q11 fails as well.
35. test_clean_merge_of_old_main_and_waived_tip_passes: agree.
36. test_merge_with_unwaived_untyped_tip_refused: DISPUTE as record 33: `origin="sdk"` is dropped by :95; :103 is not exercised. Rewrite.
37. test_doc_waiver_counts_for_the_doc: agree.

### RealApp (:402)

38. test_ui_change_needs_realapp_record: agree. Checked the four assertions against :428-437; `change_is_comments_only` returns False at :354 because the old file does not exist.
39. test_realapp_record_in_range_allows: agree.
40. test_record_before_a_later_non_ui_commit_allows: agree.
41. test_later_ui_commit_is_not_covered: agree. The later change is real code (`int x;` to `int x = 2;`), so `ui_changes` counts it.
42. test_record_on_the_branch_covers_the_merge: agree. Two rules carry here: `_carries` for the code review and `realapp_checked` for the record.
43. test_realapp_record_outside_range_refused: agree (:386).
44. test_dirty_realapp_record_refused: agree (:385).
45. test_unused_window_refused: agree (:385).
46. test_malformed_records_count_for_nothing: agree, Rewrite. Each variant reaches its own branch: "not json" and the truncated line are skipped by `common.records` (:146-154); `[1, 2]` is dropped by the `isinstance(o, dict)` filter at :154; the variants fail at :385-386 (`kind`, `is not False`, `type(n) is not int` for "2" and for True, `isinstance(c, str)` for the list, `c not in branch` for the short hash). A failure of one variant is not named.
47. test_created_main_needs_a_record_on_its_history: agree.
48. test_realapp_waiver: agree. Three cases, each a different branch (:430-431 prefix, kind, match).
49. test_clean_merge_of_old_main_and_waived_tip_passes (RealApp): agree. Carry rule for waivers, :431.
50. test_ui_path_outside_the_marker_code_paths_needs_no_record: agree. Both `refusal` calls hit `_is_code` :180-185.
51. test_waiver_hook_records_the_phrase: agree with Investigate. Callers: `waiver._record` is called by `waiver.body` (:34); `prompt.py` not opened. Both assertions can fail (pattern removed; `fullmatch` weakened to a prefix match).
52. test_non_ui_change_skips_realapp: DISPUTE in part. :529 can fail (the mock raises if `realapp_checked` runs; `explain_main_move` :429 calls it only when `ui` is non-empty). :530 cannot fail: `common.records` opens the file for reading (`common.py:148`) and nothing in production writes `realapp.jsonl`, so the file never exists in this test. The record says :530 "fails if the check ran for non-UI changes"; it does not. Verdict should be Rewrite (drop :530); a dead line is a line that goes (rubric 8).
53. test_deleting_a_ui_file_counts: agree. `deleted` carries the path (:414, :421) and `ls-tree` for the new rev gives nothing (:352-354).

### RealApp, comment-only rule (:540-:835)

For all 54: checked the shared helper `ui_edit` (old committed on main and cleaned, new on a branch and cleaned) and `change_is_comments_only` :345-363 against `_c_code` :262-342. A refusal can come only from that function returning False (or from `realapp_checked`, which has no record file here). Agree with the shared Q3/Q11 wording. Individual notes:

54. test_comment_only_ui_change_needs_no_record: agree.
55. test_comment_edit_on_a_code_line_needs_no_record: agree.
56. test_comment_edit_on_an_include_line_needs_no_record: agree (the include has quotes and no `<`, so `_LOOSE_INCLUDE` :40 does not hit).
57. test_adding_a_trailing_comment_needs_no_record: agree.
58. test_removing_a_trailing_comment_needs_no_record: agree.
59. test_trailing_comment_on_a_last_line_with_no_line_end_needs_no_record: agree (:339-341).
60. test_removing_two_trailing_comments_needs_no_record: agree. The `finish` trim at :320-322 pops two separate comment pieces.
61. test_shifting_the_blanks_before_a_trailing_comment_needs_no_record: agree. The sample text "(T7)" is free input.
62. test_comment_edit_inside_a_code_line_needs_no_record: agree (a block comment that spans a line end becomes one space).
63. test_trailing_comment_edit_in_a_crlf_file_needs_no_record: agree (:318-319).
64. test_block_comment_across_a_line_end_does_not_end_a_directive: agree. Old lines `#define A 1`, `+2;`; new one line. Genuine.
65. test_comment_between_tokens_stands_for_a_space: agree. Old becomes `#define F (x) x`. The C rule behind it is not read in any file; record says so.
66. test_blanks_inside_a_string_count: DISPUTE (F3). Refuses for any whitespace change; it cannot catch a string-handling bug. Rewrite, or Remove as covered by test_whitespace_change_on_a_code_line_counts (:717), which reaches the same compare.
67. test_macro_taking_in_the_next_line_counts: agree. Overlaps :622 (same compare, line structure differs); both stay green or red together.
68. test_code_moved_onto_a_directive_line_counts: agree; overlap as record 67.
69. test_trailing_blanks_inside_a_raw_string_count: DISPUTE (F3). Not caused by raw-string handling. Rewrite: it needs an edit that only raw-string handling can see (:631 is that case).
70. test_blank_line_inside_a_raw_string_counts: agree. This one does depend on raw handling (an empty line is dropped at :324 otherwise).
71. test_trailing_blanks_on_a_code_line_count: agree. `finish` trims only behind a comment (:320).
72. test_crlf_to_lf_counts: agree.
73. test_lf_to_crlf_counts: agree (mirror of 72).
74. test_line_end_change_behind_a_trailing_comment_counts: agree (:318-319 keeps `\r\n` in the line).
75. test_line_end_change_on_a_joined_line_counts: agree (joins kept exact, :277).
76. test_joining_two_halves_of_a_word_counts: agree.
77. test_join_inside_a_raw_string_counts: agree (:306 searches the unjoined text).
78. test_join_does_not_close_a_raw_string: agree. Genuine: without raw handling `// b` is a comment and the edit is comment-only.
79. test_comment_marks_in_an_include_name_are_code: agree (`_LOOSE_INCLUDE`).
80. test_comment_marks_in_an_include_name_behind_a_comment_are_code: agree. Same one regex as 79 and 81; the three differ only in the prefix text. They catch an anchoring change on the regex, nothing else.
81. test_comment_marks_in_an_include_name_behind_a_comment_across_lines_are_code: agree; overlap as record 80.
82. test_lone_carriage_return_in_a_comment_counts: agree (`_LONE_CR` :43, :357).
83. test_comment_edit_after_an_include_name_needs_no_record: agree (`<a.h>` has no `/` inside, so the regex does not hit).
84. test_bytes_that_are_not_utf8_count: agree. Genuine: with `errors="replace"` instead of `surrogateescape` (:271) both bytes would read the same and :702 would be True.
85. test_block_comment_edit_needs_no_record: agree.
86. test_one_code_line_among_comments_counts: agree. This is also the case that catches skipping strings (F3): unskipped, the string's `/*` would swallow `int z`, and the result would be None.
87. test_removed_code_line_counts: agree.
88. test_whitespace_change_on_a_code_line_counts: agree.
89. test_commenting_out_code_counts: agree.
90. test_comment_marks_inside_a_string_are_code: DISPUTE (F3). Cannot fail from string handling; it is refused either way. Rewrite so the edit sits after the string, or Remove as covered by record 86.
91. test_comment_line_inside_a_joined_macro_counts: agree.
92. test_blank_line_inside_a_joined_macro_counts: agree.
93. test_comment_line_after_a_joined_macro_needs_no_record: agree.
94. test_comment_ending_in_a_backslash_swallows_the_next_line: agree.
95. test_backslash_and_blank_at_a_line_end_counts: agree (`_LOOSE_SPLICE` :46). The comment's claim "compilers differ" is not proven here.
96. test_char_literal_after_a_keyword_hides_no_comment_mark: agree. Genuine: if `return'` were read as a digit separator, `'/*'` would open a comment and the `z` edit would vanish.
97. test_digit_separator_is_not_a_character_literal: DISPUTE. The record says it checks the digit rule at :309. It cannot fail from that rule: a wrongly opened character literal at `1'000'000` stops at the line end (:310) and nothing after it on the line is a comment mark, so the result is the same. Verdict: Rewrite. A case that does depend on it needs a comment mark after an odd number of separators (for example `int n = 1'000; /* a */`).
98. test_digit_separator_edit_counts: DISPUTE. Any change to the digits is a code change whether or not separators are read as literals, so it cannot catch the digit rule either. Rewrite together with record 97, or Remove (covered by 88).
99. test_raw_string_with_comment_marks_is_code: agree. Genuine.
100. test_line_comment_marks_inside_a_block_comment_are_comment: agree on verdict. Q8 note: a bug that read `//` inside the block as a line comment would still leave the second line `c */` identical in both files, so this case would stay green; it guards almost nothing.
101. test_block_comment_start_inside_a_line_comment_hides_nothing: agree. Genuine.
102. test_a_failing_git_is_not_swallowed: agree. The mock makes the first `git` call raise (:352 through `_first`).
103. test_later_root_commit_with_ui_files_voids_the_record: agree. Traced: `(parts + [None])[1]` gives None, `change_is_comments_only` returns False (:350), so `realapp_checked` is False. The record's remark that :804 has no comment is right.
104. test_new_ui_file_of_comments_counts: agree.
105. test_non_c_ui_file_counts_even_for_comment_lines: agree (:33 does not list `.rc`).
106. test_comment_only_commit_after_the_check_is_covered: agree.
107. test_realapp_and_review_problems_both_named: agree.

### LibraryAllow and KeyAllow (:839, :852)

108. test_accepts_the_phrase_alone (LibraryAllow): agree, Investigate. Callers: `percall.py:51`, `waiver.py:23` (record had "not opened"). The tolerance of "." and "!" is unsourced (split 6).
109. test_refuses_anything_more_or_less (LibraryAllow): agree.
110. test_the_phrase (KeyAllow): agree. `KEY_ALLOW_TEXT` compared with a retyped copy of itself; the text appears in `percall.py:96` for the user to type.
111. test_accepts_the_phrase_alone (KeyAllow): agree. Callers `percall.py:91`, `waiver.py:25`.
112. test_refuses_anything_more_or_less (KeyAllow): agree.
113. test_the_library_phrase_and_the_key_phrase_are_distinct: agree.

### HumanPromptText (:871)

114. test_human_record_gives_its_text: agree. Callers `percall.py:34`, `review_rules.waived` :127.
115. test_text_blocks_are_joined: agree.
116. test_non_text_block_never_counts: agree (:110).
117. test_each_disqualifier: DISPUTE. See F2: subcases `turnOrigin:"sdk"` and `promptId:"p2"` are dropped by the prefilter, so :102 and :103 are not exercised; deleting either leaves the test green. The record says six subTests each reach their disqualifier. Verdict should be Rewrite (a fault in the test itself).
118. test_missing_file_or_id_gives_none: agree.
119. test_broken_line_is_skipped: agree. The bad line holds `p1` and `"human"`, so it passes :95 and reaches `json.loads` (:98-100).

### ReflogReplay (:980)

120. test_every_historical_main_move_allows: agree, Investigate. Checked: runs only with `GATES_REFLOG_REPLAY=1` (:980); reads the real repos and the old state, which still exists (`hooks\state\derive_once\reviews.jsonl` and `doc_review\reviews.jsonl` are both present); `reflog_moves.json` has 294 `"repo"` lines (268 hydra) and one `exception`. The one exception text says the new gate "matches waivers to the commit id only" (data line). Not run; whether it holds today is unknown. The expected list is a saved snapshot (F4).

## Totals for this part

Records checked: 120. Disputed: 11 (records 30, 33, 34, 36, 52, 66, 69, 90, 97, 98, 117). Agreed: 109, some with a correction to the record's reason (4, 8, 29, 100, 108, 110, 111, 114).

Agree with a Keep: none; the part has no Keep verdicts. Question 3 for every "claim not checked only" holds as written. No rewrite in this part proposes a saved expected-output file.
