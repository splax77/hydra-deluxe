Batch B9, agent ac27e4d31849eca34. All 35 records reached. Judged by reading only; no test run. No capture run served any of them.

Files read (all under worktrees/test-audit-records): tools/effort_test_faults.py (F), tools/effort_test_scout.py (S), tools/effort_test_shuffle.py (SH), tools/effort_test_taint.py (T), tools/ch_probe/ocr.py, and the five test files.

| Record | Test | Claim | Verdict | Evidence |
|---|---|---|---|---|
| D10 63 | tools/ch_probe/tests/test_ocr.py:37 | empty text gives None | HOLDS BY READING | ocr.py:38-39 `if not text: return None`. The test calls the real parse_accuracy_text. |
| D14 test_no_commits_and_an_empty_report | test_effort_test_faults.py:110 | no edits and no report: 0 fixed, 3 missed, run fail | HOLDS BY READING | F score_fault:152 (equal text count, not fixed), :156-162 (no file paragraph, missed), score:183 (fail), changed_hunks empty. |
| D14 test_every_fault_caught | faults:113 | all three planted texts removed: 3 fixed, run pass | HOLDS BY READING | F score_fault:152 (count 0 < tip count) for F1, F2, F3; false_findings:120-126 sees every hunk on planted lines (blame at tip, :78-88); score:183 pass. The report line is never needed (all three are fixed by the edit). |
| D14 test_named_by_file_and_key_line | faults:119 | four spellings of file plus line 9 name F1 | HOLDS BY READING | F mentions_file:133-135 (backslash fold, lower case), mentions_line:138-140 (`L?9`), score_fault:156-159. All four spellings traced. |
| D14 test_named_by_the_line_the_fault_has_at_the_tip | faults:124 | line 11 (its place at the tip) names F1 | HOLDS BY READING | F score_fault:154-155 adds every tip row holding planted_text; F2 inserts two lines above, so F1 is at 11. |
| D14 test_named_by_the_words_of_the_fault | faults:128 | the key's `what` words, case and line breaks changed, name F1 | HOLDS BY READING | F mentions_words:143-145, squash:129-130 collapses whitespace and lower-cases; one paragraph (single newline). |
| D14 test_named_by_the_planted_text | faults:132 | planted text (double space) names F1 | HOLDS BY READING | F mentions_words with squash on both sides. |
| D14 test_a_line_with_no_file_does_not_name_a_fault | faults:141 | "Line 9 ..." with no file name stays missed, no note | HOLDS BY READING | F score_fault:156-162: with_file is empty, so returns ("missed", ""). |
| D14 test_a_paragraph_about_one_fault... | faults:144 | a paragraph naming F2 does not name F1 | HOLDS BY READING | F score_fault:156-158 splits on blank lines. F1 numbers are {9,11} and the first paragraph has neither; F2 numbers {3,4} match "line 3". |
| D14 test_a_longer_file_name_is_not_the_faults_file | faults:149 | mycalc.py is not calc.py | HOLDS BY READING | F mentions_file:135 lookbehind `(?<![\w.-])`; "y" blocks the match. |
| D14 test_a_number_inside_a_longer_number... | faults:152 | 19, 9ms, 1.9 are not line 9 | HOLDS BY READING | F mentions_line:140 `(?<![\w.])L?(?:9\|11)(?!\w)`. 19: 9 preceded by "1". 9ms: followed by "m". 1.9: preceded by ".". |
| D14 test_fixed_wins_over_named | faults:156 | a fixed fault with a report naming it reads "fixed" | HOLDS BY READING | F score_fault:152 returns before the naming checks. |
| D14 test_fixing_the_planted_copy_counts... | faults:160 | one of two copies changed counts as fixed; unchanged is missed | HOLDS BY READING | Calls the real score_fault. Count 1 < 2 gives "fixed" (:152). Equal counts with empty report give "missed" (:162). |
| D14 test_removing_the_faults_file_counts_as_fixed | faults:169 | deleting the file counts as fixed | HOLDS BY READING | F file_at:72-76 gives None for a missing file; score_fault:152 `reviewed_text is None` gives fixed. |
| D14 test_an_edit_away_from_planted_lines... | faults:175 | stray edit at tip line 6 is one false finding | HOLDS BY READING | F changed_hunks:91-117 gives (calc.py, 6, 1); planted {3,4,11} (blame, :78-88) does not hit; false_findings:120-126. Line 6 follows from F2's two inserted lines. |
| D14 test_neighbouring_changed_lines_are_one_false_finding | faults:181 | two adjacent changed lines make one false finding | HOLDS BY READING | `git diff -U0` gives one hunk `-5,2`; changed_hunks:115 appends one entry. |
| D14 test_an_edit_on_a_reference_line... | faults:186 | editing the reference line is a false finding | HOLDS BY READING | planted_lines:80-87 counts only blame lines from fault commits, and the reference commit is not one of them. |
| D14 test_an_edit_in_a_file_with_no_fault... | faults:190 | a new file gives "false finding: src/new.py line 0" | HOLDS BY READING | Hunk header `@@ -0,0 +1 @@` gives (path, 0, 0); false_findings:123 touched=(0,1); no planted lines for that path. Path slicing at :107 traced. |
| D14 test_removing_a_whole_multi_line_fault... | faults:194 | removing both multi-line faults: 2 fixed, 1 missed, 0 false | HOLDS BY READING | Removed hunks (calc 3-4, other 2-3) fall wholly on f2/f3 blame lines; F1 text unchanged, so missed. |
| D14 test_a_line_added_beside_a_planted_line... | faults:199 | an added line just above a planted line is no false finding | HOLDS BY READING | Pure insertion hunk `-10,0`: false_findings:123 touched=(10,11); 11 is planted. |
| D14 test_removed_lines_that_look_like_diff_headers... | faults:204 | removed lines "--- a/x" and "-++ b/y" are one false finding at line 4, not read as headers | HOLDS BY READING | F changed_hunks:101-116 gives hunk (other.py,4,2); lines 4-5 come from the extra commit, not a fault commit, so they are a false finding. See side note 4: this input cannot make the code fail. |
| D14 test_path_and_quoted_line_pass | test_effort_test_scout.py:45 | path in backticks plus the line passes | HOLDS BY READING | S has_path:31-33, has_line:36-37, check:40-45. |
| D14 test_an_absolute_windows_path_passes | scout:49 | C:\repo\tools\check_pointers.py passes | HOLDS BY READING | S has_path:33 folds backslashes to "/"; the lookbehind allows the "/" before "tools". |
| D14 test_a_wrong_path_fails | scout:53 | other file name fails | HOLDS BY READING | S has_path finds no match; has_line finds the line. |
| D14 test_a_wrong_line_fails | scout:57 | "= 151" fails | HOLDS BY READING | S has_line:37 substring "MAX_CALLS = 150" is not in "MAX_CALLS = 151". |
| D14 test_a_longer_file_name_is_not_the_path | scout:60 | mytools/, .pyc, .py.bak do not match | HOLDS BY READING | S has_path:33 lookbehind `(?<![\w.-])` and lookahead `(?![\w-]\|\.\w)`. |
| D14 test_the_match_is_case_exact | scout:64 | max_calls and TOOLS/ fail | HOLDS BY READING | S has_path and has_line use no IGNORECASE and no lower-casing. |
| D14 test_each_answer_file_gets_a_line | scout:69 | two answer files give two lines in order | HOLDS BY READING | S main:57-59 and :63-64. |
| D14 test_a_missing_answer_file_fails_loudly | scout:84 | a missing answer file exits 2 | HOLDS BY READING | S main:58 open raises FileNotFoundError (an OSError); caught at :60, returns 2. |
| D14 test_each_labelled_file_holds_the_run_the_key_says | test_effort_test_shuffle.py:95 | R-file diff and test text belong to the branch the key maps | HOLDS BY READING | SH build:76-90 uses one labels dict for the files and the key. The diff has "+work of run N". hide_run_names:51-56 leaves ", run N\n" at the end of the test text. |
| D14 test_a_run_name_is_replaced_by_that_runs_label | shuffle:112 | names replaced by label; eff-q-10 left alone | HOLDS BY READING | Calls the real hide_run_names; regex :55 lookarounds `(?<![\w-])`/`(?![\w-])`; traced to the expected string. |
| D14 test_the_order_is_shuffled | shuffle:118 | different seeds' orders differ (more than one order in 20 shuffles) | HOLDS BY READING | SH labelled:44-48 calls rng.shuffle; the production build passes SystemRandom (:76). |
| D14 test_a_key_in_a_folder_beside_the_bundle_is_allowed | shuffle:145 | key in "bundle-keys" is accepted | HOLDS BY READING | SH check_inputs:66-69: "bundle-keys" is not equal to nor under "bundle\\". |
| D14 test_an_unknown_branch_writes_nothing | shuffle:156 | missing branch exits 2, no key, no folder | HOLDS BY READING | SH build:79-84 runs every git diff and builds files in memory before makedirs (:85); git() raises CannotShuffle (:36-40); main returns 2 (:106-108). |
| D14 test_a_broken_transcript_line_fails_loudly | test_effort_test_taint.py:216 | a non-JSON transcript line exits 2 | HOLDS BY READING | T tool_calls:130 json.loads raises JSONDecodeError (a ValueError); main catches ValueError at :163 and returns 2. |

Side notes
1. tools/test_effort_test_faults.py:51-57: the F2 key line is 3 but its planted text sits at line 4 at its own commit; the F3 key line is 2 but its text is at line 3. Tests still pass because score_fault also uses the lines where the text stands (F:154-155). The fixture key is slightly off.
2. test_effort_test_faults.py:113: the report line "other.py line 2 holds the copy." does nothing, because all three faults are fixed by the edit. The test name says "caught".
3. test_ocr.py:37: the records file says the claim needs a live game screen. The test only feeds a string to a pure parser (the test's docstring and ocr.py:29-48), so no game is needed. Its path is tools/ch_probe/tests/test_ocr.py, not tools/test_ocr.py.
4. test_effort_test_faults.py:204: changed_hunks only treats a row as a header when it starts with `diff --git a/` or `Binary files `. In `-U0` output every content row starts with +, - or space, so no removed line can look like one. The skip at F:116 and this input cannot make the test fail, so it passes whatever the skip does. The claim is true, but the test cannot tell.
5. test_effort_test_faults.py:99-108 and others hard-code expected score lines. These are the script's own output format and not copied numbers, so I do not count them as a fault.
6. S has_line:37 is a substring match: "MAX_CALLS = 1500" would pass for the key "MAX_CALLS = 150". No test pins that either way.
7. test_effort_test_taint.py:216 only checks the exit code, not the stderr text.

Counts: SHOWN BY RUN 0; HOLDS BY READING 35; CONTRADICTED 0; CANNOT TELL 0; not reached 0.
