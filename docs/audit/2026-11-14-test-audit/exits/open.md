# The open list: the 20 records nobody could settle by reading

## How each was settled (main session, session e3d99f2b, 2026-11-21)

The user's answer to question 3 (card `docs/sessions/2026-11-19-8e6a2908.md`): chose "Run the 12, use the files, drop the 2 scripts as tests". The runs below were made on main at ac50ea5193bf6a0eb3ddd032785d3d930dc36909, built in worktrees/tf-r1-build.

Group 1, the runs:

- `records/A12.md:173`: GONE. The docs tests were removed from main by debd48412 ("Remove the docs tests; the code lexer case moves to its own file"); no test in tests/ names a Premise map. There is nothing left to run.
- `records/A16.md:382`: not a test, closed by the user. The test only prints read times and runs only when HYDRA_TIMING_DB names a library file, so it checks nothing. Asked whether to run it on a copy of the library file or close it as not a test, the user said "close it" (2026-11-21).
- `records/A20.md:236`: holds. `hydra_tests "-tc=build_preview_scene: under the 1.0 rule the offered fill is the one the engine charged"` passed, 11 of 11 assertions.
- `records/A31.md:24`: holds. "StemReader: straight-through read equals the full decode" passed, 301 of 301 assertions.
- `records/A31.md:233`: holds for both readings of the doubt above. "StemReader: a FLAC behind an ID3 tag decodes the same as the plain file" passed, 5 of 5, and "StemReader: a damaged Opus stem plays again after a seek to before the damage" passed.
- `records/A38.md:88`: holds. A script with `expect-text zzqq-not-on-screen` failed with exit 1 ('"zzqq-not-on-screen" is not on screen'); one with `expect-not-text Scan library` failed with exit 1 ('"Scan library" is on screen'); a script with the true forms of both passed.
- `records/A44.md:129`: holds. `hydra_uitest --test paths-long-path` passed.
- `records/A45.md:105`: holds. `hydra_uitest --test preview-overlay-fit` passed.
- `records/A45.md:250`: holds. `hydra_uitest --test settings-change-holds-layout` passed.
- `records/window-WN3.md:243`: holds. "clamped_sqin charts: every squeeze-out ends SP at the end its record names" passed, 271 of 271.
- `records/C11.md:686`: holds. "SP end clamp: a folded variant's Clamped step on its leader's SqIn phrase becomes SqIn" passed, 2 of 2.
- `records/delta-DL1.md:303`: holds. "the log keeps growing while a hold is open" passed, 6 of 6.

Group 2, the files: all six hold. `python -m pytest -rs test_frida_tools.py test_method_watch.py` in tools/ch_mcp/tests ran with the real dumps in hydra-data/ch-dumps (builds 4045, 6142 and 6161 present): 70 passed, 132 subtests passed, none skipped.

A fresh closure checker (session e3d99f2b, task O1-check) read each closure against the code at ac50ea51: 17 PASS, A12 GONE, A16 waits (since closed by the user), no REOPEN. It found the A31.md:233 doubt is the verdict file's: B1's "A31 SR14" line describes SR15 (A31.md:248), and both tests were run. Every test line cited above is stale; the names still match.

Group 3, the two scripts: by the user's choice, `tools/ch_probe/experiments/key_delivery_test.py` and `pad_flash_test.py` are not tests. They leave the test ledger; the scripts stay as experiments.

T8 step E6 (`docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md`). Written by the main session, session f3bbb32b, 2026-11-16.

The by-reading pass judged 397 clean Investigate records. For 20 of them the scout could not tell whether the test's claim holds, because settling it needs a run, a file from outside the repo, or a person watching the live game (`capture/byreading/SUMMARY.md`, "Cannot tell (20)"). Each row below names the record, its test, what its verdict file says would settle it, and the mutation run's numbers where the run held that test.

Nothing here is run. Each group becomes one question in T10: may the run happen? If not, the records stay on this list and the audit's final report names them as open. No row is marked "holds", and none may leave by exit 4. Until T10 answers, each leaves by exit 2.

The mutation numbers do not settle any row. The run planted its own kinds of bug, not the one a record asks about. Every number below carries the run's limits:

> Mutation run evidence, with four limits: (1) the tool plants its own kinds of bug, not the change an auditor named, so caught nothing is not proof a test is empty; (2) the run went several bugs at once, so a timed-out bug lists its red tests but credits none of them as catches; (3) each verdict comes from one run of its bug; (4) the run held only the C++ tests in hydra_tests, so a test outside it has no evidence here, which is not the same as catching nothing. See docs/audit/2026-11-14-test-audit/mutation/README.md.

In the "Run" column, "caught" is `mut_caught` and "reaches" is `mut_reaches`, both from `mutation/tests.txt`. A record with "not in the run" is a test the run did not hold (limit 4).

## Group 1: needs the test, or one run on the corpus (12)

T10 asks whether the main session may run these tests, or the named dump, on the checked-in corpus. None needs the user's library except A16 22, which says so.

| Ledger id | Test | What would settle it (verdict file) | Run |
|---|---|---|---|
| `records/A12.md:173` | `tests/test_docs_match_code.cpp:1887` (record says 1883), every plan that counts holds a Premise map in the shape | the docs check run on the real plans (B1 line 10) | caught 0, reaches 2; the ledger has it `none`, and `mutation/none-reread.md` matches it to that test |
| `records/A16.md:382` | `tests/test_library_jobs.cpp:1274`, timing: the startup library read by part | the test asserts nothing and returns at once unless `HYDRA_TIMING_DB` names a file; it then only prints times, and needs the user's library (B1 line 11) | caught 0, reaches 0, in the caught-nothing list |
| `records/A20.md:236` | `tests/test_preview_scene.cpp:289`, under the 1.0 rule the offered fill is the one the engine charged | the search run on `make_ch10_fill_song` (B1 line 25) | caught 15, reaches 316, shared 0 |
| `records/A31.md:24` | `tests/test_stem_reader.cpp:225`, straight-through read equals the full decode | the five decoders run on the fixtures (B1 line 49) | caught 60, reaches 198, shared 0; red in 1 timed-out bug, running in 2 crashed bugs |
| `records/A31.md:233` | `tests/test_stem_reader.cpp:557` | the test run, or a read of the Opus reader and libopus (B1 line 50) | caught 12, reaches 38, shared 0 |
| `records/A38.md:88` | `expect-text` / `expect-not-text` in the uitest script runner | one script line that should fail (B2 line 22) | not in the run |
| `records/A44.md:129` | `tests/ui/uitest_paths.cpp:540`, `test_paths_long_path` | the test run, or a dump of `##pathlist` and `##pathdetails` at both shares on The Spiraling Void (B2 line 30) | not in the run |
| `records/A45.md:105` | `tests/ui/uitest_preview.cpp:534`, `test_preview_overlay_fit` | the test run, or `hydra_uitest` at that share and width printing the boxes (B2 line 33) | not in the run |
| `records/A45.md:250` | `tests/ui/uitest_preview.cpp:922`, `test_settings_change_holds_layout` | the test run, or tab and scrubber rects dumped before and during a settings change on Burnout (B2 line 35) | not in the run |
| `records/window-WN3.md:243` | `tests/test_tied_variants.cpp:263`, on the clamped_sqin charts every squeeze-out ends SP at the end its record names | the test run, or a `hydra_replay` dump of both charts at SP cap 3 (B5 line 40) | caught 80, reaches 567, shared 0; the earlier record for this test, `records/A35.md:85`, carries the same numbers |
| `records/C11.md:686` | `tests/test_sp_end_clamp.cpp:284`, a folded variant's Clamped step on its leader's SqIn phrase becomes SqIn | one run of the chart, `hydra_replay` dump or the test itself (B10 line 23) | caught 51, reaches 349, shared 0 |
| `records/delta-DL1.md:303` | `tests/test_store_records.cpp:456`, the log keeps growing while a hold is open | a real run of the test; it depends on SQLite's checkpoint rules and file sizes (B6 line 50) | caught 4, reaches 75, shared 0 |

Two rows need a note.

`records/A31.md:233` does not agree with its verdict. The ledger's record title for line 557 is "StemReader: a FLAC behind an ID3 tag decodes the same as the plain file", and the run's test at that line has the same name. The B1 verdict for "A31 SR14" describes a different claim: "damaged Opus stem plays again after a seek". So the scout may have judged the wrong test, or the record's number may point at the wrong line. Which one is not settled here. It goes to T9 as a closure doubt and to T10 with this group.

`records/A12.md:173` is the docs check, which the main session runs as part of every merge's tests. A run of it on the plans settles this row without a question, if T10 agrees that a run counts.

## Group 2: needs files from outside the repo (6)

T10 asks whether the real `dump.cs` and `facts.json` for the game build these tests name may be used. Each test skips itself when the file is absent, so in the repo it passes without checking anything.

| Ledger id | Test | What would settle it (verdict file) | Run |
|---|---|---|---|
| `records/D3.md:224` | `test_frida_tools.py:588`, each `menu_fields.json` name sits at its dump line | the real `dump.cs` of the build in `menu_fields.json`; the test skips without it (B6 line 14) | not in the run |
| `records/D3.md:230` | `test_frida_tools.py:624`, each enum word and value sits in its dump enum | the same `dump.cs` (B6 line 15) | not in the run |
| `records/D8.md:226` | `test_method_watch.py:422`, `start_candidates.json` describes the facts' build | the real `facts.json` for that build (B6 line 31) | not in the run |
| `records/D8.md:234` | `test_method_watch.py:426`, every candidate RVA is a method of its class with its name | the same `facts.json` (B6 line 32) | not in the run |
| `records/D8.md:240` | `test_method_watch.py:435`, every candidate name is a method of its class | the same `facts.json` (B6 line 33) | not in the run |
| `records/D8.md:253` | `test_method_watch.py:453`, each named assembly holds its class in the dump | `facts.json` and `dump.cs` (B6 line 34) | not in the run |

## Group 3: needs the live game and a person watching (2)

T10 asks whether these two are tests at all. Each is a script with no assertion: it sends keys and prints what a person should look for.

| Ledger id | Script | What would settle it (verdict file) | Run |
|---|---|---|---|
| `records/D16.md:17` | `tools/ch_probe/experiments/key_delivery_test.py`, `main` at line 46 | a person reading a focused Notepad window (B6 line 7) | not in the run; the ledger has it `none` |
| `records/D16.md:51` | `tools/ch_probe/experiments/pad_flash_test.py`, `main` at line 49 | a person watching the live highway (B6 line 8) | not in the run; the ledger has it `none` |

## How this list was checked

The 20 rows are the 20 `CANNOT TELL` table rows in `capture/byreading/B*-verdicts.md`, found by searching for that verdict word in every verdict file. The search hit 21 lines; the 21st is B4's totals line, which counts zero. 16 of the 20 reach a ledger row through its `byreading_verdict` column. The other four (A12 10, A16 22, A38 11 and WN3 10) do not carry that column, so the main session matched them to their ledger rows by record number and test line. That gap is the ledger's to close at E7.
