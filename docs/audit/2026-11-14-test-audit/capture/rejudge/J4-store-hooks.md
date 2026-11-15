J4 (runs R5, R6, R8; 26 records), agent a97759a3cb8bea4b9.

Paths: R5 = capture/runs/R5, R6 = capture/runs/R6, R8 = capture/runs/R8/reflog_replay.txt (all under docs/audit/2026-11-14-test-audit/).

| Record | Test | Run | Verdict | Evidence |
|---|---|---|---|---|
| A9 29 | tests/test_config.cpp:696 | R5 | NOT SHOWN | The settings file read (R5/hydra_settings.ini.used) has only clean whole numbers (sp_cap=4, depth_value=4, mslimit_value=10, preview_volume=23). Nothing spelled 8.0, 1e1 or a too-big number was fed in. |
| A27 SF9 | tests/test_saved_figure_fill.cpp:341 | R5 | SHOWN | R5/hydra.db read-only: records 98, record_paths 374; 128 rows have sqin_count>0 and 0 of them have hardest_ms NULL (hydra_legacy.db: 134 and 0). Matches assertions 359 (each squeeze-in path has a figure), 362 (squeezed>0: 128>0) and 363 (no row missing a figure). Both charts of the record ran: batch.txt:89 "Foo Fighters - Everlong  1 2 3+" and batch.txt:110 "Smash Mouth - I'm A Believer". This answers the record's "Everlong: no record". |
| A34 R8 | tests/test_store_results.cpp:339 | R5 | NOT SHOWN | Claim is how Profile operator== compares. Each db holds one profiles row (chartmode, sp_cap 4, ms_enabled 1, ms_value 10, depth 4, legacy_fills 0 vs 1, efill_window_ms 60); no run compares two profiles, and "off stores 0" is not reached (ms limit was on). |
| A33 T2 | tests/test_store_records.cpp:99 | R5 | NOT SHOWN | Claim is an in-memory store's reader() being the store itself. R5 wrote file stores; no output touches it. |
| B1 7 | tests/test_cli.cpp:312 | R5 | NOT SHOWN | Misfiled to R5. The claim is the refusal when hydra_batch gets no folder, or both a folder and --whole-library. R5 ran with one folder and succeeded ("98 analyzed ... 0 failed"). |
| WN4b 31 | tests/test_saved_figure_fill.cpp:341 (delta record of A27 SF9) | R5 | SHOWN | Same test, same evidence as A27 SF9. profiles.efill_window_ms = 60 in both dbs, matching the record's "at --efill 60" window question; Everlong and Believer analyzed; all 128 squeeze-in rows hold hardest_ms. |
| DL1 171 | tests/test_store_records.cpp:171 | R5 | NOT SHOWN | Claim: the file store's reader gives the writer's answer. R5 only wrote the file; nothing read it back through both. |
| DL1 178 | tests/test_store_records.cpp:178 | R5 | NOT SHOWN | Needs a second rules file and a reader lookup. Nothing read the dbs back. |
| DL1 456 | tests/test_store_records.cpp:456 | R5 | NOT SHOWN | Claim is WAL growth while a hold is open. R5 left no -wal file and printed no log size. |
| DL1 536 | tests/test_store_records.cpp:536 | R5 | NOT SHOWN | In-memory own-reader; test-only case. R5 used files. |
| DL1 Profile | tests/test_store_results.cpp:339 (delta) | R5 | NOT SHOWN | Same as A34 R8. |
| A17 35 | tests/test_long_paths.cpp:167 | R5 | NOT SHOWN | Needs a path over 260 characters. R5's db path is docs/audit/.../R5/hydra.db (batch.txt:1), short. |
| A17 39 | tests/test_long_paths.cpp:261 | R5 | NOT SHOWN | Needs a report page copied to a short temp path for the browser. The batch ran with auto_open_report=0 and made no report. |
| DL2 1 | tests/test_store_open.cpp:183 | R6 | NOT SHOWN | Claim: classify_library_file names each generation. R6/v2.12.1_library.txt and R6/v2.1.0-schema4.txt show the same state text: "store=ready", "charts=0", status="", "No charts found yet". No generation name or refusal sentence appears for either file. |
| A6 28 | tests/test_app_library.cpp:761 | R6 | NOT SHOWN | Claim: after the set-aside box, an unreadable chart folder gives "Your library needs a scan, but <folder> cannot be read...". Both R6 outputs show status="" and no box; that state was never reached. |
| A32 3 | tests/test_store_open.cpp:174 | R6 | NOT SHOWN | Older twin of DL2 1. Two different generations giving the same text settles nothing about naming them. |
| E3 RJ:147 test_commit_with_trailers_passes | test_ref_judge.py:147 | R8 | NOT SHOWN | The replay calls review_rules.explain_main_move only (test line 996). The claim is ref_judge's commit-trailer rule. The 65 refusals do not bear: each is a main move judged for src/ui real-app or reviewer verdict, none for trailers. |
| E3 RJ:153 test_a_commit_origin_already_has_passes | test_ref_judge.py:153 | R8 | NOT SHOWN | Same: ref_judge, not explain_main_move. Refusals do not bear. |
| E3 RJ:221 non-fast-forward origin-tracked refused | test_ref_judge.py:221 | R8 | NOT SHOWN | Same. review_rules.py:399-401 docstring says rewinding main is ref_judge's rule, "not judged here". No refusal names a rewind. |
| E3 RJ:232 local-only rewind passes | test_ref_judge.py:232 | R8 | NOT SHOWN | Same. Refusals do not bear. |
| E3 RJ:285 fast-forward push passes | test_ref_judge.py:285 | R8 | NOT SHOWN | Same. The 65 refused moves are main moves refused by the review rule, not the push judge. |
| E3 RR:252 test_ci_setup_is_code | test_review_rules.py:252 | R8 | NOT SHOWN | No refusal or tally line names .github, and the replay prints no changed-file list. Refusals do not bear. |
| E3 RR:313 test_internal_doc_is_not_gated | test_review_rules.py:313 | R8 | NOT SHOWN | All 65 refusals are code/real-app/verdict causes; none is a doc-review cause; the output does not say which moves touched docs/agents. Does not bear. |
| E3 RR:324 test_marker_deletion_refused | test_review_rules.py:324 | R8 | NOT SHOWN | No refusal mentions a marker. Does not bear. |
| E3 RR:328 test_doc_key_drops_bom_and_crlf | test_review_rules.py:328 | R8 | NOT SHOWN | Internal key function; the output prints no key. Does not bear. |
| E3 RR:341 test_repo_without_markers_passes_everything | test_review_rules.py:341 | R8 | SHOWN (weak) | reflog_replay.txt line 1: `reflog replay: 294 moves; {"carry": 71, "no-gate": 26, "no-review-needed": 96, "refused": 65, "verdict": 36}`. 26 real moves came back "no-gate" (allowed). All 65 refusals are repo "hydra", a marked repo. Weak: the output prints no per-move line for the 26, so which repo they are is not in the output. |

## The 65 refusals (R8) and the test

The test (test_review_rules.py:1004-1006) expects exactly the one move marked "exception" in reflog_moves.json (f7f64cb18c..fa19f5c942). It got 65. The failure text: "Lists differ: [('hydra', 'fc5df54d16', 'be044b34fc'), ...] != [('hydra', 'f7f64cb18c', 'fa19f5c942')]".
- All 65 are repo "hydra". 63 name only the real-app cause, for example "refused hydra fc5df54d16..be044b34fc: main was not moved: 1 file(s) under src/ui change and no real-app check is recorded for a commit on this branch after its last src/ui change. It stays at fc5df54d16."
- 2 name a missing reviewer verdict: "refused hydra f7f64cb18c..fa19f5c942: main was not moved: no reviewer verdict is recorded for fa19f5c942 or any commit before it on its branch. Also: 1 file(s) under src/ui change and no real-app check ..." (the expected one) and one for f4c87ac805.
- So 64 refusals are unexpected. A likely cause read from the test code, not shown by the output: convert_old_records (test lines 940-980) carries over only derive-once reviews, doc-review reviews and waivers from the old state, never real-app records, so a replay cannot find an old real-app check. Not confirmed by a run.
- None of the 65 bears on any RJ or RR claim in this slice. They bear on the replay test's own claim, which no record in this slice holds.

## Side notes

- A32 3 cites test_store_open.cpp:174 and DL2 1 cites :183 for the same test (classify_library_file); the line moved between rounds.
- R6: the two outputs show identical state text, and both files hold 0 charts rows. The run cannot tell the too-old 2.1.0 file from the 2.12.1 file that opens. The v2.12.1 file's records (3) and record_paths (7) do not show on screen.
- B1 7 is filed under R5 in CLEAN.md but needs a refusal run.
- A17 35 and A17 39 sit under R5 but R5 touches neither a long path nor a report page. A9 29 sits under R5 but R5's settings hold no decimal spelling.
- CLEAN.md's R8 command (tests.test_review_rules from the gates folder) cannot import in isolated mode; capture/runs/index.md line 19 says the documented command was used instead.
- R5 charts table has 0 rows (the batch writes records only), so a record's song cannot be named from the db; Everlong and Believer are identified from batch.txt only.
- Three version strings in one run set: profiles.hyversion '3.1.2+efillwindow' (R5 dbs), 'Hydra Deluxe 3.1.1' (R6 screen), last_run_version=3.1.0 (R5 settings copy).

Count: SHOWN 3, CONTRADICTED 0, NOT SHOWN 23.
