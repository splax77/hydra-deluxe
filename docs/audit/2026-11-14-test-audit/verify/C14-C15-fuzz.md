Agent: a5879b5b32043c1a3 (task T5-V-R19b)

# Verify C14-C15 fuzz records

Records checked: 22 of 22 (six fuzz targets, sixteen headed engine records covering 23 functions). I agree with 16 verdicts and dispute 6, all six being Keeps. Marks are "by reading" except where stated; I opened `tests/fuzz/harness.h` (lines 68-84), `tests/test_fuzz_regressions.cpp` line 57, `tests/single_owner/engine.cpp` lines 157, 176-178, 283-285, 306-323 and `tests/single_owner/scan.cpp` lines 233, 436-447 and 479. I did not open the parsers or the fuzz runs' CI logs, and I ran nothing.

## What the record gets right

- `run_one` ends in `return 0` after catching `std::exception` (`harness.h:71-79`), so `CHECK(run_one(...) == 0)` at `tests/test_fuzz_regressions.cpp:57` cannot fail by its value. Confirmed by reading.
- `scan.cpp:233` (`files > 100`) and `scan.cpp:479` (`rules().size() > 100`) are floors, as the record says.
- `engine.cpp:157` has `t.tm_hour = 12` with the comment "noon, so a daylight-saving change cannot move the day".

## Fuzz targets (6): all Investigate, all agreed

chart, mid, sng, srb, audio and image. Each promises "no input crashes, hangs or reads out of bounds" and has no assertion, so the real mechanism (the fuzz exe under the fuzz sanitizer on the real parser) is what must be checked; the record names the replay run it needs. "Claim not checked only" is the right form, with F3 (default settings only), the memory-versus-file byte source (sng, srb) and the `decode_audio` wrapper (audio) as open questions. I agree these go to the main session as questions, not rewrites. The image target has no saved input and no JPEG seed; agreed.

## Engine records (16)

Agreed Investigate (9 engine records, 15 with the six fuzz targets): `parse_baseline`, `parse_dates`, `today_iso`, `baseline_due`, `copy_entry_name`/`clone_entry_name`, `compile_rule`/`flags_line`, `lexer_language`/`skipped_comment_line`, `in_scope`, and `start_scan`/`scan_file`. The empty real baseline (`baseline.json`, 51 bytes) means no real entry has ever passed through them; agreed.
Agreed Keep (1): `walk_language`/`walk_reads_for`. By reading `scan.cpp:436-447` it states real tracked paths and both languages (a `.py` is read only for a `"py"` row, `winstr.cpp` only for a C++ row, `tests/single_owner/scan.cpp` never), and the independent owner of "is it C++" is `is_cpp_source` in `listed.h`. Each line can fail: removing the `tests/single_owner/` exclusion at `engine.cpp:311` turns the `scan.cpp:444` line red.

## Disputed (6 Keeps, each should be Investigate)

The brief says one answer of "partly" or "not read" rules Keep out, and that a fault the record admits is still a fault. In each case the record itself names the gap.

1. `load_tables` (engine.cpp:270). Record: Keep. It claims the baseline is read too, but the real baseline holds no entry, so half the claim is unchecked on real input; its only positive evidence is `rules().size() > 100` at `scan.cpp:479`, a floor (the record calls the same shape a stand-in for `scan_file`). Investigate.
2. `load_rules` and `rule_files` (253, 263). Record: Keep. The record says the sort-removed bug "may not be caught on this machine" because Windows lists directories by name. A bug that would go unnoticed is a failed question 8. Investigate.
3. `select_rule_files` (242). Record: Keep. Its own question 3 says "independent owner of which files: none; the area-folder shape is ADR 0029, not opened". Not read rules Keep out. ADR 0029 exists (`docs/adr/0029-the-single-owner-rules-are-data.md`) and should be opened. Investigate until then.
4. `parse_rule` and `parse_rules_file` (86, 140). Record: Keep. The record says an empty `"owner"` would pass (the `required_string` empty test is not caught by any made-up row), and a bad regex gives no row name. A named uncaught bug fails question 8. Investigate.
5. `iso_plus_days` (151). Record: Keep. Its question 8 says deleting `tm_hour = 12` (engine.cpp:157) is "not caught on a machine with no daylight-saving change on those dates". The line is there to stop exactly that bug. Investigate; the three calendar cases themselves I agree with.
6. `listed_lines` (327). Record: Keep. The record says the baseline half "has no assertion" and is unchecked because the real baseline is empty. A half-unchecked claim cannot be a Keep. Investigate.
The seventh Keep, `walk_language`/`walk_reads_for`, I agree with. Tally: 15 Investigate and 1 Keep agreed (16); 6 Keeps disputed; 22 in all.

## Question 3 for the fuzz and engine records

The record says "claim not checked" for each fuzz target and for the engine functions with no real entry, naming what is needed. I agree. Nothing in `records-engine/` or `records-screen/` bears on these claims, so no Engine/Claim lines can be finished from them.

## Splits (one line each)

1. `run_one` always returns 0 while one test checks that value (`test_fuzz_regressions.cpp:57`, B1's).
2. The row scan reads comments by `skipped_comment_line` (C++ `//` only) and the owner-gone guard by `codelex` pieces: two comment rules.
3. `scan_single_file` (`scan.cpp:394`) repeats the row-scan loop outside the engine.
4. `fuzz_audio` fuzzes `decode_audio`, which only tests call, while the Preview reads stems through `open_song_stems`.
5. The default scope of a row (`src`, `tools`) and the folders `walk_visits` names (`src`, `tools`, `tests`) are two lists that agree by convention (`in_scope` record).
6. `today_iso` has no comparison with an independent date, and an empty result would make every entry look not due (a silent pass).

## Not reached

None. All 22 records were opened. The engine functions' bodies were read only for the lines named above.
