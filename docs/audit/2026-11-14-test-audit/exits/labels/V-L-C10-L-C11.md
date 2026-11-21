Agent id: abc691bf2f776e9ad (task T8-E3-V-L-C10-L-C11, session f3bbb32b)

Both scouts' files are other agents' (aceef9d2955df9fd0 and abafe42180ba3b706), so the check could go ahead.

## Counts

- L-C10: 70 rows in the batch, 70 lines in the label file (each batch row appears exactly once, checked by diffing the two row lists), 62 rows agreed, 8 changed.
- L-C11: 108 rows in the batch, 108 lines in the label file (same check), 101 rows agreed, 7 changed.

Both batches are rows of `counts/T5-IR19.md` (L-C10) and `counts/T5-IR2.md` (L-C11). The dispatch note named `T5-IR10`; the batch file and the scout's file both say `T5-IR19`, and I checked against those.

The ends tables are right for every key they list: each named row is in the batch and carries that key. Two changes below touch them (see "Ends" under the tables).

## Rows I change

### L-C10

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-C10 | counts/T5-IR19.md:84 | F-row | F-row, F-record, F-db | The test (`test_store_open.cpp:353`) saves a `prepare_record(... at_cap(4))` record and then drops the album column and edits the stamp in raw SQL, which is the same hand-SQL fault rows 69 to 71 carry F-db for. |
| L-C10 | counts/T5-IR19.md:85 | F-row | F-row, F-record, F-db | Same seed (`seed_without_album`, called at `test_store_open.cpp:386`) and two more hand-written `ALTER TABLE ... DROP COLUMN` statements at 387. |
| L-C10 | counts/T5-IR19.md:86 | F-db, P-other | F-db, P-other, F-row, F-record | `seed_v220_layout` (`test_store_open.cpp:466`) writes `library_row` rows and `prepare_record(... at_cap(8))` records before the hand SQL, the same helpers the scout keyed F-row and F-record for on `test_store_library.cpp` rows. |
| L-C10 | counts/T5-IR19.md:87 | F-db | F-db, F-row, F-record | Same seed (`test_store_open.cpp:620` calls `seed_v220_layout`). |
| L-C10 | counts/T5-IR19.md:88 | F-db | F-db, F-row, F-record | Same seed (`test_store_open.cpp:636`). |
| L-C10 | counts/T5-IR19.md:89 | F-db | F-db, F-row, F-record | Same seed (`test_store_open.cpp:649`). |
| L-C10 | counts/T5-IR19.md:95 | F-record | F-record, F-row | The test (`test_store_open.cpp:1418`) also builds its chart list with `library_row("a", "A")` and `library_row("b", "B")` (line 1425). |
| L-C10 | counts/T5-IR19.md:101 | P-message | P-message, plus X9 part: no key | The counter's X9 mark is `FakeClipboard` (`uitest_paths.cpp:69`, a stand-in for the Windows clipboard), and no key in `keys.md` covers that, so the row needs a "no key" for its X9 part beside P-message. |

Rows 63, 65 and 82 stay as the scout labelled them, and the three are consistent. Rows 63 and 65 are synthetic audio bytes (a fixture decoded by the real stem reader; `test_stem_reader.cpp:225`, `:557`). That is input under question 1 of `questions.md`, and F-db defines a hand-written SQL database, so no key fits; `no key` is right. Row 82 (`test_store_open.cpp:183`) is `CREATE TABLE ...` statements that build library files by hand for the classifier, which is what F-db names (question 3 of `questions.md`, source of F-db), so F-db is right even though the counter's reason calls them input. The counter's reason is what disagrees with the key list, not the scout.

The short names are confirmed. `counts/T5-IR19.md` line 7 states `sl.cpp` is `tests/test_store_library.cpp` and `so.cpp` is `tests/test_store_open.cpp`. I also checked that the named lines are what the rows say: `test_store_library.cpp` lines 88, 116, 165, 184, 216, 241, 284 and 304 are the test cases of rows 66 to 73, and `test_store_open.cpp` lines 183, 269, 353, 383 and 570 are the classify, 2.8.0, album and 2.2.0 tests of rows 82 to 86.

Ends for L-C10: nothing in the table breaks. After my changes, F-row also has rows 84 to 89 and 95, F-record has 84 to 89, F-db has 84 and 85; the plainest and least-fitting rows the scout named still carry their keys.

### L-C11

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-C11 | counts/T5-IR2.md:60 | F-schedule | F-schedule, K-selfmade | The counter says the expected text is "rebuilt by calling the producer (`edge_probe.probe_chords`)", and `keys.md` does not limit K keys to record rows. |
| L-C11 | counts/T5-IR2.md:61 | C-rig, Z-cite | C-rig, Z-cite, K-selfmade | The counter says the expected value is "again from the producer". |
| L-C11 | counts/T5-IR2.md:62 | C-rig | C-rig, K-selfmade | The test (`test_game_tools.py:2193`) builds its expected reply from the exception `edge_probe.load_probes(path)` raises (`f"refused: ValueError: {caught.exception}; nothing started"`). |
| L-C11 | counts/T5-IR2.md:63 | C-rig | C-rig, K-selfmade | The test (`test_game_tools.py:2202`) takes its expected chords from `GT.chords_to_play`, the loader the code under test uses. |
| L-C11 | counts/T5-IR2.md:113 | C-stub, E-echo | C-stub, E-echo, K-selfmade | The counter says the expected line is "built by producer". |
| L-C11 | counts/T5-IR2.md:121 | C-stub | C-stub, K-selfmade | The counter's whole reason is "producer-built expected line". |
| L-C11 | counts/T5-IR2.md:122 | C-stub, F-row, P-message | C-stub, F-row, P-message, K-selfmade | The counter says the expected first-press line comes from `GT.first_press_line(ROWS)`, the producer, on typed ROWS. |

For rows 60 and 121, the scout named the fault but could find no key; K-selfmade is the key (its definition is "a value checked against the function that made it, or made by production code").

Ends for L-C11: K-selfmade is a new key in this batch, so it needs an ends line. Plainest row is `counts/T5-IR2.md:121` (the only reason that is just the self-made expected line); least fitting is `counts/T5-IR2.md:122` (a producer call mixed with typed rows). The other 15 ends lines the scout wrote all check out against the rows.

Rows 74, 75 and 89: I agree with O-gamemem. The test (`D5.md:322` to `:335`) feeds typed launch-agent status dictionaries and byte strings (the in-game launch agent's own report about the patch). That is the game's own message, which O-gamemem names ("the game's method calls or messages"). Row 89 also keeps C-stub for the fake agent. Rows 76 to 78 and 71 to 73 and 81 treat a fake launch agent as C-stub, and that is a different fault (fake code, not a typed reply), so the split is consistent.

## Not changed but doubtful

- L-C11 rows 58, 67, 68, 91, 92, 93, 94, 96, 100, 101, 106, 107 and 108 carry F-record, defined as "records written to the store and read back". Most of these are hand-typed game save files or the tool's own marker file in a fake game folder (`ORIGINAL`, `USER_SETTINGS`, `scoredata.bin` at `test_game_tools.py:2228`, `:2632`, `:2640`). The nearest other key is O-scorestats, but that names the game's stats or score JSON, which is a narrower fit for `settings.ini` and for the tool's own marker. I kept the scout's key, because no other key is closer, and put the doubt here so the main session can see it. The rows' X9 faults all have an X9-family key.
- L-C10 rows 104 and 105 (hand-written settings file and rules file) carry only P-message; their X9 part is input under question 1, as the scout wrote.

## Side notes

- `tests/ui/uitest_paths.cpp:69` (`FakeClipboard`) is a stand-in for the Windows clipboard and has no key; revision 1's one revision is spent, so row `counts/T5-IR19.md:101` is a `no key` question of its own (the X9 part).
- `tests/test_store_library.cpp:1312`-area row 33 (`counts/T5-IR19.md:33`) uses an injected read-fault seam. The counter marks it X9 only for the seeded library, so F-row covers it; no key is needed for the seam itself.
- `counts/T5-IR19.md:7` and the batch file for L-C10 are both accurate about the short names; no stated mapping was missing, because the counter file states it in its own line 7.
- `tools/ch_mcp/tests/test_game_tools.py:2640` (`test_a_live_runs_ch_stop_leaves_every_file`) and `:2688` write `b"changed by the game"` into every file in `ORIGINAL` and require it back. That is the E-echo the scout gave. It also means the stand-in is the game's own saves; see "Not changed but doubtful".
- Row `counts/T5-IR2.md:102` (`test_game_tools.py:2752`) types a by-hand command "copied from `bot_run._restore_hint`", so the expected value is a copy of the code's own text. The scout's P-message fits; the copy is the same pin the key names.
- No split found in either batch.

## Not checked

- L-C10 rows 23 to 43 and 54 to 62 and 98 to 105: I read the counter's reason and checked each fault mark has a key of its family, but did not open `test_app_library.cpp`, `test_app_reports.cpp`, `test_app_click.cpp`, `test_dynamics_breakdown.cpp`, `pbtest_coach.cpp` or `pbtest_window.cpp`. The reasons settle the keys for these rows.
- L-C11 rows 40 to 57, 59, 64 to 90, 95, 97 to 99, 102 to 150 other than those named above: reasons only, `test_game_tools.py` opened at lines 2176 to 2210, 2228 to 2285, 2614 to 2694, 2752 only. I did not open the D5 record text beyond `D5.md:18` to `:22` and `:322` to `:335`.
