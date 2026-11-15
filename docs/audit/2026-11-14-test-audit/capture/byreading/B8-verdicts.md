Batch B8, agent ac6aa0045022c98fb. All 35 records judged by reading; no capture run served any of them (all "unplaced" or "no run").

Production code read: tools/check_handoff.py, tools/doc_tokens.py, tools/check_pointers.py, tools/effort_test_faults.py, tools/ch_probe/ocr.py. Every test runs the real script (subprocess, or import for score_fault), not a copy.

| Record | Test (file:line) | Claim in a few words | Verdict | Evidence |
|---|---|---|---|---|
| D10 63 | tools/ch_probe/tests/test_ocr.py:37 | parse_accuracy_text("") gives None | HOLDS BY READING | ocr.py:38-39 `if not text: return None` |
| D14 test_existing_claims_listed | test_check_handoff.py:94 | real commit, branch, path, function listed EXISTS, exit 0 | HOLDS BY READING | scrape() check_handoff.py:282-316; find_commit 159, find_branch 167, find_function 175, find_path 199; "on main" from contains() 68 |
| D14 test_missing_claims_fail | :105 | 4 made-up names are MISSING, "4 missing", exit 1 | HOLDS BY READING | scrape 296-300, 314-316; find_* return None |
| D14 test_changed_since_date | :114 | a commit after the handoff date lists the file under CHANGED SINCE | HOLDS BY READING | changes_since 105-112; scrape 303-309; date from file name, handoff_date 398-403 |
| D14 test_unchanged_is_not_listed | :125 | untouched file not listed as changed | HOLDS BY READING | changes_since returns [] for README.md (only commits dated 10-01 and 10-03) |
| D14 test_prose_is_ignored | :132 | `yes`, `--repo` are not claims | HOLDS BY READING | doc_tokens.shape 101-112 gives None for both; scrape skips None (check_handoff 294) |
| D14 test_other_repo_root_on_line | :138 | a repo root named on the line makes that repo searched; without it the hash is MISSING | HOLDS BY READING | other_repos 136-153, look_up 265-277. Needs git's toplevel path to match the temp path (side notes) |
| D14 test_date_sources | :150 | date from file name, else git date, else today with warning; --since wins | HOLDS BY READING | handoff_date 398-413; main 425, 445 |
| D14 test_qualified_function_found_where_written | :183 | Class::fn found in the file that writes it, not an earlier bare use or a doc | HOLDS BY READING | find_function 175-196: fixed grep of qualified name first, docs excluded (grep 96-97), gives src/player.cpp:2 |
| D14 test_path_outside_the_repo_or_in_the_wrong_case_is_missing (record gives no line; test is at :203) | :203 | wrong-case, ../ and /README.md paths are MISSING | HOLDS BY READING | find_path 199-214: exact_case 114-125, relative 127-133 return None; "/README.md" joins to a place that does not exist |
| D14 test_qualified_method_needs_its_class_in_the_file | :210 | Gadget::draw found only in a file naming Gadget; Widget::run and Gadget::compute_total missing | HOLDS BY READING | find_function 188-191 owner_files filter |
| D14 test_session_id_is_not_a_commit | :221 | hex after "session" and UUID pieces are not claims | HOLDS BY READING | doc_tokens.tokens 146-149, _ID_WORD 55, _BARE_HEX lookarounds 50 |
| D14 test_claims_table_statuses | :234 | 14 table rows get ok / not-on-main / not-pushed / missing / manual and the exact summary | HOLDS BY READING | row_status 353-368, table 371-393; traced each row: 4 ok, 5 missing, 1, 1, 3 |
| D14 test_claims_table_only_rows | :260 | only rows under ## Claims are read; prose names ignored | HOLDS BY READING | claims_rows 336-350, main 443-444 |
| D14 test_claims_exit_code | :268 | exit 0 only when all rows are ok or manual | HOLDS BY READING | table 393 |
| D14 test_claims_rows_that_must_not_pass | :278 | empty value, ".", outside file, tail name, wrong case, prose, wrong class are missing; unknown kind is "unknown" | HOLDS BY READING | row_status 366-368, find_path 212, find_function 181-183 and 188-191. The 7th row (bare "compute_total") is expected ok and is ok via `_IDENT.match` at 181 |
| D14 test_claims_table_with_no_readable_row_fails | :297 | a Claims heading with no rows prints error and exits 1 | HOLDS BY READING | table 374-376, 391-392 |
| D14 test_row_without_outer_pipes_is_read | :303 | row without edge pipes is still read | HOLDS BY READING | claims_rows 345 (`line.count("\|") >= 2`), split_row 321-333 |
| D14 test_missing_path_reported | test_check_pointers.py:76 | a missing path is reported as file:line: path (missing) | HOLDS BY READING | check 232-263, judge 136-157, _broken 192-203 |
| D14 test_hook_names | :83 | bare hook name found in gates; old-hooks name "retired"; unknown "missing" | HOLDS BY READING | judge 151-152 adds gates folders; old_hooks is not a root, so it falls to _broken then old_names 201-202 |
| D14 test_absolute_path | :91 | live absolute path passes, dead one missing | HOLDS BY READING | judge 144-149 |
| D14 test_sessions_line_skipped | :99 | pointers on a "Sessions:" line are not checked or counted | HOLDS BY READING | check 251 |
| D14 test_exit_code | :107 | exit 0 with none broken, 1 with one; summary counts | HOLDS BY READING | main 310-314, summary 266 |
| D14 test_wrong_case_is_broken | :139 | wrong-case pointer is broken, hint names the real file | HOLDS BY READING | judge 155 `_exact_case`; _broken first test (k == written, 196) gives "found at src/real.cpp" |
| D14 test_tail_match_is_never_found | :145 | other/real.cpp and bare real.cpp are broken, real file as hint | HOLDS BY READING | judge never matches by tail; _broken tests 2 and 3 (196-197) give the hint |
| D14 test_docs_read_at_any_depth_and_archive_left_out | :156 | docs/agents subfolders and CLAUDE.md read; memory archive not | HOLDS BY READING | documents 63-79 (os.walk of docs/agents; memory listdir top only, `isfile`) |
| D14 test_fenced_block_and_sibling_and_other_repo | :165 | bare path in a fenced block is read; sibling name and hooks-root path resolve | HOLDS BY READING | doc_tokens.tokens 134-153 (fenced lines still scanned for bare paths); judge roots 150 |
| D14 test_not_file_names_are_ignored | :173 | branch name, registry key, URL, slash words, elided path, owner/proj are not pointers | HOLDS BY READING | is_pointer 111-134; traced each of the 8 tokens |
| D14 test_shorthand_names_are_ignored | :181 | widget ids, counters, .git files, slash lists are not pointers; a real missing one still shows | HOLDS BY READING | is_pointer 122-126, _is_alternatives 91-97; traced each token |
| D14 test_memory_note_shown_by_short_name | :191 | memory folder named once at top, notes shown by short name | HOLDS BY READING | main 299-304 (folder in header only), documents 78 (`folder/name`) |
| D14 test_git_show_fallback_that_opens | :210 | a line whose own git show opens passes; the same pointer on another line stays broken, either order | HOLDS BY READING | has_fallback 159-176, check 258-262 (verdict cached, fallback asked per line); 8-char hash accepted by GIT_SHOW 38 |
| D14 test_git_show_fallback_that_does_not_open | :224 | git show at wrong commit, wrong file, zero hash, wrong case stays broken | HOLDS BY READING | has_fallback 168-174: `target != p` skip; cat-file -e fails |
| D14 test_git_show_fallback_in_hooks_repo | :235 | "hooks repo" on the line sends git show to the hooks repo | HOLDS BY READING | has_fallback 166 |
| D14 test_quoted_pointer_skipped_only_in_rulings | :244 | pointers inside the user's double quotes skipped only on rulings pages | HOLDS BY READING | check 245, 253; in_quotes 49-60; 1+3+3 = 7 checked, 7 broken |
| D14 test_a_hand_fix_of_one_of_three_faults... | test_effort_test_faults.py:99 | one fault fixed by hand = F1 fixed, F2 and F3 missed, no false finding, run fail | HOLDS BY READING | score 165-184, score_fault 148-162 (fewer copies of planted text = fixed), changed_hunks 91-117 (hunk at tip line 11), planted_lines 78-88 (blame gives line 11 to F1's commit) so no false finding |

Side notes

- Column 4 of every D14 row says "a real handoff from docs/handoffs/", "the real docs" or "a real effort-test wave". No test uses one. Each builds a small temp git repo and a hand-written handoff, note or key (test_check_handoff.py:63-81, test_check_pointers.py:39-54, test_effort_test_faults.py:35-59). The verdicts above are about the scripts' rules on those inputs. Whether the scripts behave on the real files is not tested.
- The record for test_path_outside_the_repo_or_in_the_wrong_case_is_missing has a cut-off line number ("tools/test_check_handoff.py:"); the test is at line 203.
- test_check_handoff.py:138 passes only if git's rev-parse --show-toplevel prints the same folder as the temp path (check_handoff.py:149 `_same`). A short-form (8.3) Windows temp path would make it fail, not pass wrongly.
- test_check_handoff.py:159-161 needs the temp folder to sit outside any git repo (handoff_date falls back to git log, line 407).
- test_check_handoff.py:159-160 reads datetime.date.today(); a run across midnight could fail it.
- test_effort_test_faults.py:52-54: F2's key line is 3 but its planted_text sits on line 4 (the comment's second line). The script copes (score_fault 154-155 adds every row holding the text); test :99 does not depend on it.
- test_check_pointers.py:235 turns the "claude" temp folder into a git repo as a stand-in for the hooks repo.
- D10 63 sits in the record as "live game screen read by OCR", but the test only feeds a string to the pure parser (test_ocr.py:21, 38); the claim it makes needs no live game.

Counts: SHOWN BY RUN 0; HOLDS BY READING 35; CONTRADICTED 0; CANNOT TELL 0; not reached 0.
