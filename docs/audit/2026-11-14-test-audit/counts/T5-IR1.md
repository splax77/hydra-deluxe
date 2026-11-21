Agent id: a6b83a3740a205101 (task T5-IR1, session 63c70014-4328-49dc-9205-742235732752)

Recount of the Investigate records in `records/D14.md` by the user's rulings (constants, non-code sources, direct checks, nothing fake). D14 holds 121 records: 119 Investigate and 2 Rewrite (`test_tables_print_in_the_fixed_order`, `test_the_brief_names_the_three_inputs_and_the_labels`). The Rewrite two are not counted. I read all 119 Investigate records and the code of all seven test files (`tools/test_check_handoff.py`, `test_check_pointers.py`, `test_compare_db.py`, `test_effort_test_faults.py`, `test_effort_test_scout.py`, `test_effort_test_shuffle.py`, `test_effort_test_taint.py`). I did not open the scripts under test again except where a record already quoted them; the record's own line references were checked against the test code only. Nothing was run.

## How I called the faults (judgment calls the reader may overrule)

1. Hand-built input that is built to exercise one rule is not X9: a temp git repo made with real git, a note or handoff text with one pointer in it, a fault key, a reviewer's report. Hand-written stand-ins for a real artifact the real tool reads are X9: the hand-made sqlite stores in `test_compare_db.py` (a cut-down schema the file itself calls a stand-in for a layout the store no longer writes, and hand-typed library rows), and the hand-written transcripts in `test_effort_test_taint.py` (the brief names a hand-written transcript as X9). If the user rules that hand-written handoffs, notes, keys and reports count as X9 too, then every one of the 119 records is X9, because none uses real input (the records' "claim not checked" fault).
2. X10 means: the test requires the tool's own message prose, hint wording or a typed copy of a constant (labels, file-name set, hash set, hook-protocol keys) where the exit code or the verdict already carries the behaviour. I did not mark the tool's verdict words (`missing`, `ok`, `pass`, `fail`, `names main`, `path outside`) or its summary count lines, since the test must read the behaviour from that output. That leaves a grey zone; I marked only the clear cases.
3. X11 and Snapshot: none found. Typed-in values that come back (a branch name, a hash, `first shuffle`) are checked as behaviour of the code (found, listed, kept), not as a pass-through. Counts like `line 6` and `1 fixed, 2 missed` follow from fixtures built in the test by a rule, so they are not snapshots.
4. "Named" says whether the record's own text already flagged that fault (typed copy, stand-in, fake, copy of a constant). "Claim not checked" alone is the audit's reason for Investigate, not a naming of these faults.

## Table

Files are `tools/` test files. Columns: record (test line), faults, faulty check lines, reason, named.

### test_check_handoff.py (20 records)

| Record | Faults | Lines | Reason | Named |
|---|---|---|---|---|
| test_existing_claims_listed :94 | none | | real git repo built by rule | |
| test_missing_claims_fail :105 | none | | | |
| test_changed_since_date :114 | none | | | |
| test_unchanged_is_not_listed :125 | none | | | |
| test_prose_is_ignored :132 | none | | | |
| test_other_repo_root_on_line :138 | none | | | |
| test_date_sources :150 | none | | | |
| test_usage :165 | X10 | 168, 171 | the word "usage" typed; exit 2 already carries the behaviour | no |
| test_doc_mention_is_not_a_function :175 | X10 | 179 | hint prose "named only in docs, docs/notes.md:1" typed; MISSING and exit 1 carry the verdict | no |
| test_qualified_function_found_where_written :183 | none | | | |
| test_path_matching_only_by_its_tail_is_missing :191 | X10 | 200 | hint prose "a different file ends the same way" typed | no |
| test_path_outside_the_repo_or_in_the_wrong_case_is_missing :203 | none | | | |
| test_qualified_method_needs_its_class_in_the_file :210 | none | | | |
| test_session_id_is_not_a_commit :221 | none | | | |
| test_claims_table_statuses :234 | none | | | |
| test_claims_table_only_rows :260 | none | | | |
| test_claims_exit_code :268 | none | | | |
| test_claims_rows_that_must_not_pass :278 | none | | | |
| test_claims_table_with_no_readable_row_fails :297 | none | | | |
| test_row_without_outer_pipes_is_read :303 | none | | | |

### test_check_pointers.py (18 records)

| Record | Faults | Lines | Reason | Named |
|---|---|---|---|---|
| test_missing_path_reported :76 | none | | | |
| test_hook_names :83 | none | | | |
| test_absolute_path :91 | none | | | |
| test_sessions_line_skipped :99 | none | | | |
| test_exit_code :107 | none | | | |
| test_session_start_output :117 | X10 | 127, 128, 136, 137 | Claude Code's hook-reply key names (`hookSpecificOutput`, `SessionStart`, `systemMessage`) and the script name typed as the expected reply | no |
| test_wrong_case_is_broken :139 | none | | | |
| test_tail_match_is_never_found :145 | none | | | |
| test_docs_read_at_any_depth_and_archive_left_out :156 | none | | | |
| test_fenced_block_and_sibling_and_other_repo :165 | none | | | |
| test_not_file_names_are_ignored :173 | none | | | |
| test_shorthand_names_are_ignored :181 | none | | | |
| test_memory_note_shown_by_short_name :191 | none | | | |
| test_git_show_fallback_that_opens :210 | none | | | |
| test_git_show_fallback_that_does_not_open :224 | none | | | |
| test_git_show_fallback_in_hooks_repo :235 | none | | | |
| test_quoted_pointer_skipped_only_in_rulings :244 | none | | | |
| test_bad_argument :261 | X10 | 265 | "usage" typed; exit 2 carries the behaviour | no |

### test_compare_db.py (7 records; the changed-order test is Rewrite and not counted)

| Record | Faults | Lines | Reason | Named |
|---|---|---|---|---|
| test_result_id_order_alone_compares_clean :205 | X9 | 21-35, 44-71, 207-208 | hand-made `results`/`paths`/`path_refs` store in a schema the file calls a stand-in for an older layout; hand-typed rows `RESULTS` | yes (F1, "old layout only") |
| test_one_changed_payload_byte_is_reported :230 | X9 | 232-233 | same stand-in store, hand-typed payload bytes | yes (block Q11) |
| test_chart_scan_order_is_part_of_the_answer :241 | X9 | 243-251 | same stand-in store, hand-inserted `charts` rows | yes (block Q11) |
| test_summary_only_old_layout_matches_new_layout :258 | X9, X10 | X9 94-123, 260-261; X10 267-272 | two stand-in layouts, neither written by the store now; skipped-table and skipped-column lists typed back, a copy of `SUMMARY_LEFT_OUT` | yes (both) |
| test_summary_only_reports_one_changed_score :275 | X9 | 94-123, 277-278 | same two stand-in layouts | yes (F1) |
| test_profile_id_order_alone_compares_clean :286 (2 cases) | X9 | 129-186, 288-291 | hand-typed `PROFILES`, `RECORDS` rows in a cut-down record schema (value columns dropped) laid on a stand-in store | no |
| test_one_changed_record_path_is_reported :299 | X9 | 129-186, 301-304 | same hand-built record rows | no |

### test_effort_test_faults.py (26 records)

| Record | Faults | Lines | Reason | Named |
|---|---|---|---|---|
| test_a_hand_fix_of_one_of_three_faults... :99 | none | | | |
| test_no_commits_and_an_empty_report... :110 | none | | | |
| test_every_fault_caught_passes_the_run :113 | none | | | |
| test_named_by_file_and_key_line :119 | none | | | |
| test_named_by_the_line_the_fault_has_at_the_tip :124 | none | | | |
| test_named_by_the_words_of_the_fault :128 | none | | | |
| test_named_by_the_planted_text :132 | none | | | |
| test_a_file_alone_does_not_name_a_fault :136 | X10 | 137-139 | the script's own note text typed back | yes ("typed copy of the message") |
| test_a_line_with_no_file_does_not_name_a_fault :141 | none | | | |
| test_a_paragraph_about_one_fault... :144 | none | | | |
| test_a_longer_file_name_is_not_the_faults_file :149 | none | | | |
| test_a_number_inside_a_longer_number... :152 | none | | | |
| test_fixed_wins_over_named :156 | none | | | |
| test_fixing_the_planted_copy_counts... :160 | none | | | |
| test_removing_the_faults_file_counts_as_fixed :169 | none | | | |
| test_an_edit_away_from_planted_lines... :175 | none | | | |
| test_neighbouring_changed_lines_are_one_false_finding :181 | none | | | |
| test_an_edit_on_a_reference_line... :186 | none | | | |
| test_an_edit_in_a_file_with_no_fault... :190 | none | | | |
| test_removing_a_whole_multi_line_fault... :194 | none | | | |
| test_a_line_added_beside_a_planted_line... :199 | none | | | |
| test_removed_lines_that_look_like_diff_headers... :204 | none | | | |
| test_a_planted_text_missing_at_the_tip_fails_loudly :220 | X10 | 223 | message text "planted_text is not in src/calc.py"; exit 2 and empty stdout carry the behaviour | no |
| test_an_empty_key_field_fails_loudly :225 | X10 | 228 | message "fault F2 has no what" | no |
| test_a_tip_that_does_not_descend... :230 | X10 | 231 | message "does not descend" | no |
| test_an_unknown_tip_fails_loudly :233 | X10 | 234 | "git rev-parse" typed from the script's error | no |

### test_effort_test_scout.py (10 records)

| Record | Faults | Lines | Reason | Named |
|---|---|---|---|---|
| test_path_and_quoted_line_pass :45 | none | | | |
| test_an_absolute_windows_path_passes :49 | none | | | |
| test_a_wrong_path_fails :53 | none | | | |
| test_a_wrong_line_fails :57 | none | | | |
| test_a_longer_file_name_is_not_the_path :60 | none | | | |
| test_the_match_is_case_exact :64 | none | | | |
| test_each_answer_file_gets_a_line :69 | none | | | |
| test_an_unknown_question_fails_loudly :74 | X10 | 77 | message "no question Q9" typed | no |
| test_an_empty_key_fails_loudly :79 | X10 | 82 | message "empty key" typed | no |
| test_a_missing_answer_file_fails_loudly :84 | none | | exit code only | |

### test_effort_test_shuffle.py (13 records; the brief-inputs test is Rewrite and not counted)

| Record | Faults | Lines | Reason | Named |
|---|---|---|---|---|
| test_key_file_records_each_label_once :85 | X10 | 92 | label set R1 to R6 typed, a copy of what `labelled` makes | no |
| test_each_labelled_file_holds_the_run_the_key_says :95 | none | | | |
| test_the_bundle_holds_only_the_labelled_files_and_no_run_name :104 | X10 | 107-108 | the twelve bundle file names typed (`R%d.diff`, `R%d.test.txt`) | no |
| test_a_run_name_is_replaced_by_that_runs_label :112 | none | | | |
| test_the_order_is_shuffled :118 | none | | | |
| test_a_count_other_than_six_is_refused :128 | X10 | 129 | message "got 5 runs, need 6" and a typed copy of the count 6 | yes (block Q5, the 6 is a copy) |
| test_a_repeated_branch_is_refused :132 | X10 | 133 | message "named twice" | no |
| test_an_existing_key_is_never_overwritten :135 | X10 | 138 | message "already exists" (the file check at 140 is fine) | no |
| test_a_key_inside_the_bundle_is_refused :142 | X10 | 143 | message "outside the bundle" | no |
| test_a_key_in_a_folder_beside_the_bundle_is_allowed :145 | none | | | |
| test_a_full_bundle_folder_is_refused :150 | X10 | 154 | message "not empty" | no |
| test_an_unknown_branch_writes_nothing :156 | none | | | |
| test_the_brief_holds_no_level_cost_or_agent_id :164 | X10 | 167-168 | typed word list ("high", "low", "$", ...) searched in a template; "low" matches "follow" | yes (substring, leans on wording) |

### test_effort_test_taint.py (25 records)

All but two build a transcript by hand (`run_script` :37-54, :154-159, :195-199) and feed it to the script. The brief names a hand-written stand-in for a real transcript as X9. The record says the record shape matches a real subagent transcript; that makes the shape right, not the content real.

| Record | Faults | Lines | Reason | Named |
|---|---|---|---|---|
| test_plan_acceptance_lists_first_and_third_only :65 | X9, X12 | X9 37-54, 66-68; X12 test name :65 | hand-written transcript; the test name cites the plan's acceptance (plan removed) | X12 yes, X9 no |
| test_the_range_form_covers_only_the_runs_own_key :76 | X9 | 37-54, 77-79 | hand-written transcript | no |
| test_a_second_ref_beside_the_range_is_listed :81 | X9 | 37-54, 82 | same | no |
| test_every_spelling_of_main_is_listed :84 | X9 | 37-54, 85-87 | same | no |
| test_file_names_that_hold_main_are_not_listed :89 | X9 | 37-54, 90-92 | same | no |
| test_other_branches_are_listed_and_the_runs_own_is_not :94 | X9 | 37-54, 95-101 | same | no |
| test_a_longer_branch_with_the_runs_name_as_a_start :103 | X9 | 37-54, 104 | same | no |
| test_dot_claude_folders_are_not_branches :106 | X9 | 37-54, 107-108 | same | no |
| test_paths_outside_the_worktree_in_every_spelling :110 | X9 | 37-54, 111-115 | same | no |
| test_a_sibling_worktree_is_outside :117 | X9 | 37-54, 118-119 | same | no |
| test_paths_inside_the_worktree_are_not_listed :121 | X9 | 37-54, 122-123 | same | no |
| test_a_reviewers_isolation_copy_counts_as_its_own :125 | X9 | 37-54, 127-128 | same | no |
| test_a_run_with_no_worktree_has_every_checkout_path_listed :130 | X9 | 37-54, 131 | same | no |
| test_relative_climbs_out_of_the_worktree_are_listed :133 | X9 | 37-54, 134-136 | same | no |
| test_a_climb_with_nothing_after_it... :138 | X9 | 37-54, 139-142 | same | no |
| test_home_spellings_of_the_checkout_are_outside :144 | X9 | 37-54, 145-151 | same | no |
| test_tool_calls_with_no_id_are_each_listed :153 | X9 | 154-164 | hand-written transcript | no |
| test_git_all_forms_are_listed :166 | X9 | 37-54, 167-173 | same | no |
| test_each_merge_prefix_is_listed :175 | X9, X10 | X9 37-54, 177; X10 176 | hand-written transcript; the four merge short hashes typed, a copy of `MERGE_PREFIXES` | no |
| test_text_a_call_writes_or_describes_is_not_read :179 | X9 | 37-54, 180-184 | hand-written transcript | no |
| test_a_write_outside_the_worktree_is_listed :186 | X9 | 37-54, 187 | same | no |
| test_other_tools_are_read_field_by_field :189 | X9 | 37-54, 190-192 | same | no |
| test_a_call_repeated_in_two_records_is_listed_once :194 | X9 | 195-205 | hand-written transcript | no |
| test_an_unreadable_transcript_fails_loudly :207 | X10 | 214 | message "cannot read the wave" typed; exit 2 and empty stdout carry the behaviour | no |
| test_a_broken_transcript_line_fails_loudly :216 | none | | exit code only; the broken line is the input under test | |

## Counts

- Investigate records read: 119 (D14 holds 121; the two Rewrite records are left out).
- Records with at least one fault: 51 (68 with none).
- Records per fault: X9 30 (7 in compare_db, 23 in taint), X10 23, X12 1, X11 0, Snapshot 0. Two records carry two or more faults (summary-only old layout: X9 and X10; merge prefix: X9 and X10) and one carries X9 and X12 (plan acceptance). 
- Records whose fault the record already named: 9 (compare_db 5, all of them the X9 stand-in schema; effort faults 1; effort shuffle 2; taint 1, the X12 plan citation).
- Under the wider reading (judgment call 1), all 119 records are X9, because none uses a real handoff, real memory notes, a real fault key or a real transcript; that is the "claim not checked" fault the audit already gave them.

## Clearest five

1. `tools/test_compare_db.py:205` (and the other six): the whole file compares hand-made two-row stores in a schema the file itself says the store no longer writes (F1), so it tests layouts that exist only in old baselines.
2. `tools/test_compare_db.py:286` and `:299`: hand-typed `PROFILES` and `RECORDS` rows in a record schema with the value columns cut away, laid over the stand-in store.
3. `tools/test_effort_test_taint.py:37-54`: every listing test runs on a transcript the test writes by hand, never a real wave.
4. `tools/test_effort_test_taint.py:176`: the four merge short hashes are typed copies of `MERGE_PREFIXES`.
5. `tools/test_effort_test_shuffle.py:164`: a typed banned-word list searched as substrings in a template ("low" would match "follow").

## Records not reached

None. All 119 Investigate records and all seven test files were read.
