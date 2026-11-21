Agent id: a16183c631e06d6b8 (task T8-E3-V-L-C01-L-C14, session f3bbb32b)

I opened the tests for the rows where the counter's reason did not settle the key: the gate tests under `C:\Users\Patrick\.claude\gates\tests\` (test_refusals.py, test_review_dispatch.py, test_ref_judge.py, test_review_rules.py), the gate scripts that print the refusal texts (review_dispatch.py, ref_judge.py, review_rules.py, ruling_rules.py, common.py, media_rule.py), `tools/test_effort_test_taint.py`, `tools/test_compare_db.py`, and the C++ tests `test_precheck_pick_exe.cpp`, `test_precheck_pick_lines.cpp`, `test_precheck_git.cpp`, `test_precheck_check24.cpp`, `test_precheck_rows.cpp`, `test_precheck_pick_names.cpp` and `test_preview_clock.cpp`. I did not open the records. I did not read the tools tests behind the repeated P-message rows of T5-IR1 (lines 27 to 30, 62, 87, 102 to 105, 118 to 136); I took the counter's reason for those.

## Counts

- L-C01: 85 rows in the batch, 85 lines in the label file (same set, no row twice), 81 rows I agree with, 4 rows I change.
- L-C14: 81 rows in the batch, 81 lines in the label file (same set, no row twice), 79 rows I agree with, 2 rows I change.

## Rows I change

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-C01 | counts/T5-IR10.md:59 | no key | O-toolout | Revision 1 added O-toolout for exactly this row: `test_precheck_pick_lines.cpp:84` types `git diff` header and hunk lines by hand and feeds them to `ChangedLines::parse_diff`. |
| L-C01 | counts/T5-IR10.md:75 | C-gate, P-message | P-message | The counter marks only X10, and `test_precheck_rows.cpp` writes its rule rows as a real `rules/planted.json` into a temp git repo that the real scan reads, which `keys.md` (question 1) calls input, not a stand-in. |
| L-C01 | counts/T5-IR10.md:92 | C-gate, P-message | P-message | Same: `test_precheck_check24.cpp:148` commits a real rules JSON file into a temp repo for the real scan; the counter marks X10 only and the fault is the typed `E ...` message wording. |
| L-C01 | counts/T5-IR10.md:93 | C-gate, P-message | F-row, P-message | `test_precheck_check24.cpp:188` hands the check a built `Check4Rows` (the rows the scan would produce), which is not a gate module or a rules file, so C-gate does not fit; F-row is the nearest ("rows"). |
| L-C14 | counts/T5-IR4.md:36 | P-other | P-message | `test_review_dispatch.py:148` requires "main" in the dispatch hook's refusal text (review_dispatch.py:113 prints "already on main"), so it is a word of a printed message; a weak fit, since the word is also a branch name. |
| L-C14 | counts/T5-IR4.md:87 | F-verdict, P-message | P-message | `test_review_rules.py:106` writes no verdict at all (no `clean()` call, only a branch and a commit), so F-verdict does not fit; the counter's X9 mark has no matching evidence in the test (see side notes). |

## Ends table

L-C01. The scout's ends table is right for every key except C-gate. Both of its C-gate rows, IR10:75 and IR10:93, lose that key above, and no other L-C01 row carries it, so C-gate has no end in this batch. Two keys the batch now carries have no end: O-toolout (only row IR10:59) and F-row (only row IR10:93). Every other end row (P-message, P-name, P-other, F-db, O-transcript, Z-cite, F-result, C-clock, E-echo) is in the batch and carries its key.

L-C14. All eight ends rows are in the batch and carry their key. After my changes P-other has no row in this batch (its only row, IR4:36, moves to P-message), so its end line goes; every other end stands. P-message's plainest row, IR4:34, and least-fitting row, IR4:72, still carry it. IR4:36 is now a better "least fitting" for P-message than IR4:72.

## Checks of P-message in L-C14

Each typed string below is text the code under test prints, which revision 1 allows. I found the print site for: "leave isolation out" and "branch's own worktree" (review_dispatch.py:26-27), "no review key" (:158), "one commit" (:162), "no local main branch" (:109), "does not know" (:107), "already on main" (:113), "crashed" (common.py:65), "media guard" (media_rule.py:53), "is not a descendant" (ref_judge.py:100), "delete main" (:121 and :154), "symbolic" (:113), "missing Task:" (ref_judge.py:203, built from "is missing %s"), `--trailer "Task:` and "Session:" (:226-229), "no reviewer verdict is recorded for" (review_rules.py:222), ". Also: " (:466), "waive %s %s" (:78), "N file(s) under src/ui" (:433), "approval test" (ruling_rules.py:348), and "ruling_rules.py --check" (CHECK_COMMAND, ruling_rules.py:108). Every one is printed by the code under test, so the scout's P-message rows stand. I did not trace the cause texts for rows :90, :91, :92 and :93 sentence by sentence; they come from the same function as :89, and the test builds them from the `%s` forms of the code's own message.

## Side notes

- `T5-IR4.md:87` (test_review_rules.py:106): the counter marks X9 ("hand-written verdict context"), but the test writes no verdict, so the X9 mark has no key that fits. The brief's rule "each fault mark needs a key of its family" and the test disagree here; I followed the test. The main session may want the counter's mark looked at.
- `T5-IR10.md:93` (test_precheck_check24.cpp:188): `keys.md` question 1 calls a hand-written rule input, yet the counter marks X9. I used F-row as the nearest key. If question 1 is read to cover it, the row has no X9 key at all.
- `T5-IR10.md:56` and `:104` (test_precheck_pick_exe.cpp:180, test_precheck_git.cpp:222): the hand-built values are a `TestPick` and an `Items` list, the precheck's own output types. F-result is described in game words (activations, paths, scores), and F-row is described as library and report rows; neither names these exactly. I left the scout's F-result.
- `T5-IR4.md:126` (test_review_rules.py:524): `mock.patch.object(review_rules, "realapp_checked", ...)` replaces a function that reads the real-app record file. The scout's C-gate is the nearest key; C-reader ("a fake reader") also fits. I left it.
- `T5-IR4.md:27` (test_refusals.py:69): three lambdas stand in for hooks, keyed C-gate by the scout because no key names a lambda stand-in; the same test also feeds `call()` payloads (O-payload), which the counter did not mark.
- `T5-IR4.md:66`, `:72`, `:73` and others in RJ assert "refs/heads/main" or "symbolic" in stderr; those are printed messages, but only rows the counter marked X10 got P-message.
- Row numbers in the batch files were checked against the test sources by test name and line; they matched wherever I looked.

## Not checked

- Records (`records/*.md`) for every row: not opened; the brief's rule is to open them where the reason does not settle the key, and the tests settled each question I had.
- The tools tests behind these L-C01 rows were not opened: counts/T5-IR1.md:27, 28, 30, 50, 62, 87, 102, 103, 104, 105, 118, 119, 126, 128, 131, 132, 133, 134, 136, 138 (test_check_handoff.py, test_check_pointers.py, test_effort_test_faults.py, test_effort_test_scout.py, test_effort_test_shuffle.py). I took the counter's reason that the text is typed; each label (P-message, P-name, P-other) follows that reason.
- The C++ rows counts/T5-IR10.md:27, 29-37, 47, 48, 52-55, 77, 89-91, 95, 97, 100, 103 (test_precheck_pick.cpp, test_precheck_pick_exe.cpp lines 108 to 163, test_precheck_rows.cpp:218, test_precheck_check1.cpp, test_precheck_check24.cpp:97 and :122, test_precheck_check3.cpp, test_precheck_check5.cpp, test_precheck_git.cpp:165): not opened beyond the reason. Whether each typed string is printed by the picker or the checks was not traced.
- counts/T5-IR1.md:68-74 (compare_db) I read the schema and builders only, not each test body.
