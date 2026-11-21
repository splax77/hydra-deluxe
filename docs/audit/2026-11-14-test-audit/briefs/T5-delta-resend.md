# Brief: T5, delta round re-sends (DL5b, SW5, SW6)

Read `docs/agents/brief-preamble.md` first, in the worktree `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\test-audit-records` (branch `claude/test-audit-records`). Its rules bind you. Session id: e633f7f6-f313-4787-b622-8d901b577b64. Caps, status rule, commit rule (your one file, staged by name, the preamble's trailers, retry on `index.lock` or a moved HEAD) and the run rule (you run no `hydra_*` program and build nothing) are as in the first brief named below.

## DL5b (task id T5-DL5b)

Your first brief is `briefs/T5-delta.md`; follow it, slice DL5. The DL5 auditor wrote `records/delta-DL5.md` (commit e23b878c) and gave these records only a short form, carrying an old verdict forward without the twelve questions. Write each in full, in a new file you own, `records/delta-DL5b.md`, each record's delta line naming the short record it replaces:

- "dynamic hit window: no chord times, no row anywhere" (`tests/test_activation_rows.cpp:1543`)
- "dynamic hit window: the row sits alongside and changes no other field" (`tests/test_activation_rows.cpp:1556`)
- the activation-rows tests at `tests/test_activation_rows.cpp` lines 1308, 1354, 1396, 1439 and 1469 (open their old records in `records/A3.md` and the code itself)
- "docs match code: the marker check catches a wrong number and a wrong word" (`tests/test_docs_match_code.cpp:486`), with its old record in `records/A11.md` or `A12.md`
- `tools/test_compare_db.py::test_profile_id_order_alone_compares_clean` (:286) and `::test_one_changed_record_path_is_reported` (:299), with old record `records/D14.md`
- the test cases that call the changed helper `digest_settings` (`tests/song_digest.h`): `tests/test_five_lane.cpp:287`, `tests/test_perf_digest.cpp:128` to `164`, and whatever test case reaches `tests/test_song_parse.cpp:1355`; one record each

Also find the commit that removed `lone_fill_activation` from `tests/test_fills_and_skips.cpp` and name it. Line numbers are at `5ec682c7`; if one is off, find the test by name.

## SW5 and SW6 (task ids T5-SW5, T5-SW6)

Your first brief is `briefs/T5-sweep.md`; follow its test and its file shape. The list `sweep/rewrites.tsv` missed Rewrite records in files whose records use other heading forms, so the first sweep did not judge them. In your files below, find every record whose verdict is Rewrite, and judge each one that has no line yet in `sweep/SW1.md` to `SW4.md` (grep those files for the record's heading or number). Write them to `sweep/<your id>.md`, with the count table per file. Name in your report how many Rewrite records each file holds in all, and how many were new to you.

- SW5: `records/A2.md`, `A8.md`, `A22.md`, `A32.md`, `A39.md`, `B5.md`, `B6.md`, `B7.md`, `C1.md`, `C3.md`
- SW6: `records/C6.md`, `C9.md`, `C14.md`, `D1.md`, `D3.md`, `D5.md`, `D13.md`, `D15.md`, `E1.md`, `E3.md`, `E4.md`

## Final report

As the first brief says, plus the counts asked above.
