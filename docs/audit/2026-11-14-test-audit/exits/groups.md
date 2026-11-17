# Groups: each key, its rows, and the two rows to open first

T8 step E3 (`docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md`). Written by the main session, session f3bbb32b, 2026-11-16.

A key names one ruling and one way of breaking it (`keys.md`). Rows that share a key share one question for the user at E7. This file says how many rows each key holds and which two rows show its ends: the plainest fit, and the one that fits least well. The main session opens both ends beside the test at E7 and marks them here with the date read. Nothing here is an exit.

## How the labels were merged

Every row comes from the 41 batches under `labels/batches/`. A batch holds every count row whose fault mark (after answer 4, `answer4.tsv`) is not `none`, and every Rewrite or Remove record, 3,558 rows in all. A scout labelled each batch, and a verifier from a different agent re-checked it. Each row takes the verifier's keys where the verifier changed it, and the scout's keys otherwise. For L-R02 and L-R03 the second verifier (`V2-L-R02-L-R03.md`) stands, since the first checked only part of the batch. The six round-1 rows that gave Revision 1 its keys take those keys.

The merge is `labels/merge_labels.py`, and its output, one line per row, is `labels/labels.tsv`. It fails if a batch row has no label line, or if a verifier changes a row its batch lacks. Verifiers changed 499 of the 3,558 rows. Every one of the 53 keys is used at least once.

The two end rows come from the scouts' "Ends" tables. An end is kept only if the row still carries its key after the verifiers' changes (50 named ends lost their key and were dropped). Where several batches named an end for one key, the pick is from the batch that holds the most rows of that key, and the cell says how many others were named. `labels/groups.py` builds the table. For a few small keys, such as F-summary, the scouts named the same row at both ends.

## Calls the verifiers made that E7 should weigh

These are labels, not rulings. Each one widens or narrows a key, so the question for that key should show it.

- **P-builtchart read broadly.** The L-R19 verifier moved about 20 rows from P-other to P-builtchart, because they type the engine's answer on a named real chart. `keys.md` defines P-builtchart as "the engine's answer on a chart the test built", but it cites Allies' best-path ticks, which come from a real chart. Read narrowly, those rows go back to P-other.
- **K-floor for existence checks and fixed waits.** The L-R19 verifier gave K-floor to two rows that need the best path to have an activation (`records/window-WN4a.md:431`, `records/window-WN5.md:187`). The L-R17/L-R18 verifier gave it to a bare `Sleep(20)` and to fixed 50 ms and 200 ms waits. Read narrowly, K-floor is "a floor with no reason", and those rows go back to no key.
- **Group notes are not each record's text.** Some record files carry a note at the top that names fake gates or hand-typed payloads for the whole file (`records/E2.md:398`, `records/E1.md:7`). The rule kept here is that a record takes a key only for what its own entry names. If a file's group note counts, rows such as `records/E2.md:563` and `:584` would also take C-gate and O-payload.
- **Fake-input marks with no fake key.** Several count rows keep the counter's X9 mark but no X9-family key, because the scout read their hand-written input as input under answer 1, not a stand-in. They are in the no-key list below, among them `counts/T5-IR17.md:96` to `:103` (ini text) and rows in L-C10, L-C17 and L-C21. Either the counter's mark or answer 1's reading is wrong for each, and the question says which.
- **C-stub stretched.** C-stub ("a stub program") was used for fakes that are not programs: a fake audio-device factory, a helper on an `object()` driver, `FakePopen`, and a lambda in place of a hook body. The fake Windows debug API was keyed C-stub in one batch and C-reader in another.
- **Z-cite under-used.** Two verifiers found records that quote a "pinned from one run" comment without Z-cite, and added it in their batches. Batches whose verifier did not look for this may still lack it.

## The groups

| Key | Records | Count rows | All rows | Plainest row | Least-fitting row |
|---|---|---|---|---|---|
| F-result | 35 | 139 | 174 | `counts/T5-IR20.md:42` (L-C12); 24 more named | `counts/T5-IR9.md:24` (L-C19); 22 more named |
| F-row | 35 | 179 | 214 | `counts/T5-IR19.md:66` (L-C10); 17 more named | `counts/T5-IR19.md:33` (L-C10); 16 more named |
| F-record | 25 | 102 | 127 | `counts/T5-IR19.md:97` (L-C10); 16 more named | `counts/T5-IR19.md:83` (L-C10); 16 more named |
| F-summary | 0 | 14 | 14 | `counts/T5-IR16.md:45` (L-C07); 3 more named | `counts/T5-IR16.md:45` (L-C07); 3 more named |
| F-schedule | 4 | 57 | 61 | `counts/T5-IR9.md:46` (L-C19); 7 more named | `counts/T5-IR8.md:75` (L-C18); 6 more named |
| F-sample | 1 | 95 | 96 | `counts/T5-IR6.md:60` (L-C16); 7 more named | `counts/T5-IR6.md:38` (L-C16); 7 more named |
| F-presslog | 1 | 36 | 37 | `counts/T5-IR16.md:46` (L-C07); 6 more named | `counts/T5-IR16.md:45` (L-C07); 5 more named |
| F-watchlog | 8 | 55 | 63 | `counts/T5-IR9.md:33` (L-C19); 4 more named | `counts/T5-IR9.md:23` (L-C19); 3 more named |
| F-verdict | 0 | 33 | 33 | `counts/T5-IR4.md:86` (L-C14) | `counts/T5-IR4.md:126` (L-C14) |
| F-db | 7 | 20 | 27 | `counts/T5-IR19.md:87` (L-C10); 4 more named | `counts/T5-IR19.md:82` (L-C10); 3 more named |
| F-window | 0 | 3 | 3 | `counts/T5-IR12.md:92` (L-C03); 1 more named | `counts/T5-IR12.md:93` (L-C03); 1 more named |
| O-reply | 19 | 12 | 31 | `records/A10.md:68` (L-R01); 10 more named | `records/A10.md:578` (L-R01); 10 more named |
| O-transcript | 1 | 39 | 40 | `counts/T5-IR1.md:147` (L-C01); 4 more named | `counts/T5-IR1.md:164` (L-C01); 4 more named |
| O-gamemem | 21 | 318 | 339 | `counts/T5-IR17.md:23` (L-C08); 17 more named | `counts/T5-IR17.md:66` (L-C08); 15 more named |
| O-process | 2 | 39 | 41 | `counts/T5-IR6.md:34` (L-C16); 4 more named | `counts/T5-IR6.md:52` (L-C16); 3 more named |
| O-payload | 1 | 5 | 6 | `counts/T5-KR1.md:24` (L-C20); 3 more named | `counts/T5-KR2.md:58` (L-C20); 3 more named |
| O-scorestats | 0 | 17 | 17 | `counts/T5-IR8.md:32` (L-C18); 4 more named | `counts/T5-IR8.md:30` (L-C18); 3 more named |
| O-ocr | 0 | 8 | 8 | `counts/T5-IR11.md:77` (L-C02) | `counts/T5-IR11.md:84` (L-C02) |
| C-analyzer | 20 | 37 | 57 | `counts/T5-KR4.md:23` (L-C21); 7 more named | `counts/T5-KR5.md:56` (L-C21); 7 more named |
| C-reader | 0 | 79 | 79 | `counts/T5-IR7.md:67` (L-C17); 8 more named | `counts/T5-KR1.md:63` (L-C20); 8 more named |
| C-worker | 1 | 13 | 14 | `counts/T5-KR3.md:84` (L-C21); 3 more named | `counts/T5-KR3.md:90` (L-C21); 3 more named |
| C-rig | 0 | 208 | 208 | `counts/T5-IR8.md:25` (L-C18); 8 more named | `counts/T5-IR8.md:65` (L-C18); 7 more named |
| C-clock | 17 | 132 | 149 | `counts/T5-IR7.md:64` (L-C17); 17 more named | `counts/T5-IR7.md:46` (L-C17); 15 more named |
| C-session | 0 | 111 | 111 | `counts/T5-IR3.md:71` (L-C13); 6 more named | `counts/T5-IR3.md:110` (L-C13); 6 more named |
| C-stub | 11 | 211 | 222 | `counts/T5-IR2.md:83` (L-C11); 14 more named | `counts/T5-IR2.md:121` (L-C11); 14 more named |
| C-gate | 0 | 45 | 45 | `counts/T5-KR2.md:75` (L-C20); 1 more named | `counts/T5-KR2.md:65` (L-C20); 1 more named |
| C-ownchecker | 28 | 23 | 51 | `records/D2.md:1881` (L-R14); 8 more named | `records/D2.md:1527` (L-R14); 8 more named |
| P-burnout | 53 | 2 | 55 | `records/window-WN5.md:289` (L-R19); 7 more named | `records/window-WN2.md:240` (L-R19); 7 more named |
| P-pbticks | 16 | 4 | 20 | `records/A19.md:320` (L-R03); 3 more named | `records/A19.md:369` (L-R03); 3 more named |
| P-allister | 5 | 0 | 5 | `records/A19.md:386` (L-R03) | `records/A19.md:303` (L-R03) |
| P-tiers | 7 | 12 | 19 | `counts/T5-IR14.md:82` (L-C05); 7 more named | `counts/T5-IR14.md:81` (L-C05); 7 more named |
| P-message | 217 | 686 | 903 | `counts/T5-IR2.md:42` (L-C11); 39 more named | `counts/T5-IR2.md:98` (L-C11); 39 more named |
| P-placeholder | 2 | 13 | 15 | `counts/T5-IR17.md:145` (L-C08); 5 more named | `counts/T5-IR17.md:152` (L-C08); 5 more named |
| P-setting | 55 | 44 | 99 | `counts/T5-IR21.md:51` (L-C12); 16 more named | `counts/T5-IR21.md:48` (L-C12); 15 more named |
| P-name | 39 | 49 | 88 | `records/A9.md:31` (L-R08); 29 more named | `records/B2.md:227` (L-R08); 29 more named |
| P-onefile | 5 | 8 | 13 | `records/C14.md:17` (L-R10); 7 more named | `records/C14.md:17` (L-R10); 7 more named |
| P-builtchart | 145 | 1 | 146 | `records/T3.md:28` (L-R18); 11 more named | `records/delta-DL5.md:211` (L-R18); 11 more named |
| P-other | 464 | 202 | 666 | `records/E2.md:517` (L-R16); 39 more named | `records/E3.md:33` (L-R16); 38 more named |
| E-echo | 18 | 251 | 269 | `counts/T5-KR2.md:103` (L-C20); 29 more named | `counts/T5-KR1.md:98` (L-C20); 28 more named |
| Z-cite | 197 | 142 | 339 | `records/A22.md:646` (L-R03); 37 more named | `records/A21.md:287` (L-R03); 37 more named |
| S-snapshot | 206 | 48 | 254 | `records/A20.md:521` (L-R03); 34 more named | `records/A19.md:440` (L-R03); 34 more named |
| K-invented | 5 | 0 | 5 | `records/T3-run2.md:456` (L-R17) | `records/T3-run2.md:554` (L-R17) |
| K-cantfail | 113 | 0 | 113 | `records/delta-DL4.md:304` (L-R18); 17 more named | `records/delta-DL1.md:430` (L-R18); 17 more named |
| K-floor | 118 | 0 | 118 | `records/C1.md:1038` (L-R09); 15 more named | `records/B3.md:139` (L-R09); 15 more named |
| K-skip | 97 | 0 | 97 | `records/B6.md:18` (L-R09); 13 more named | `records/B4.md:162` (L-R09); 13 more named |
| K-selfmade | 143 | 3 | 146 | `records/C1.md:246` (L-R09); 19 more named | `records/B3.md:91` (L-R09); 19 more named |
| K-comment | 247 | 0 | 247 | `records/A8.md:333` (L-R07); 15 more named | `records/A6.md:119` (L-R07); 15 more named |
| K-unreachable | 34 | 8 | 42 | `records/C12.md:459` (L-R10); 14 more named | `records/C2.md:542` (L-R10); 14 more named |
| O-toolout | 2 | 6 | 8 | `counts/T5-KR1.md:29` (L-C20); 2 more named | `counts/T5-KR1.md:70` (L-C20); 2 more named |
| K-duplicate | 179 | 0 | 179 | `records/D6.md:803` (L-R15); 16 more named | `records/D6.md:230` (L-R15); 16 more named |
| K-namebody | 69 | 0 | 69 | `records/D5.md:465` (L-R15); 15 more named | `records/D8.md:1004` (L-R15); 15 more named |
| K-partial | 33 | 0 | 33 | `records/A21.md:13` (L-R03); 10 more named | `records/A20.md:269` (L-R03); 10 more named |
| K-nocode | 53 | 8 | 61 | `records/D3.md:772` (L-R14); 14 more named | `records/D2.md:1831` (L-R14); 13 more named |

"Records" counts Rewrite and Remove records; "Count rows" counts the counter's faulty rows. A row with two keys counts under both. The ends columns name a row and the batch whose scout named it.

## Rows no key fits

After Revision 1, a row no key fits is a question of its own (`keys.md`, "Revision 1"). These are the rows whose label still says `no key`, 107 in all: 98 with no key at all, and 9 that carry a key for one fault and say `no key` for another (such as the eight `records/D13.md` rows whose claim a Clone Hero page refutes). The reason is the label's own sentence, cut at 240 characters; the full text is in the batch's label or verifier file.

The rows fall into a few kinds, which E7 may ask about together: claims never checked on real input (the most common Rewrite fault, and the one the key list lacks), fixed sleeps and waits on the real clock, flaky tests, bundled claims in one test, hand-written input under a fake-input mark, and a hand-built delta map (`counts/T5-IR9.md`, L-C19), which none of F-result's kinds covers.

| Row | Batch | Keys it does carry | Why no key fits, from the label |
|---|---|---|---|
| `counts/T5-IR11.md:67` | L-C02 | none | The code shows no recorder: `InputDriver()` is used as it is and 0x99 is a typed input (test_input_driver.py:68-73), so the counter's X9 reason does not match the test and no key fits. |
| `counts/T5-IR17.md:82` | L-C08 | none | `Folders` writes the game's files by hand as input, and `game_run()` is a few file writes that play the game's run; none of the keys fits that, and it is not a result, record or game memory. |
| `counts/T5-IR17.md:96` | L-C08 | none | `set_game_setting` runs for real on a temp file; only the ini text is typed, which `keys.md` question 1 calls input. |
| `counts/T5-IR17.md:97` | L-C08 | none | Same: hand-written ini text (BOM, no final newline) is input to the real function. |
| `counts/T5-IR17.md:98` | L-C08 | none | Same. |
| `counts/T5-IR17.md:99` | L-C08 | none | Same. |
| `counts/T5-IR17.md:100` | L-C08 | none | Same. |
| `counts/T5-IR17.md:101` | L-C08 | none | Same, for `get_game_setting`. |
| `counts/T5-IR17.md:102` | L-C08 | none | Same. |
| `counts/T5-IR17.md:103` | L-C08 | none | Same. |
| `counts/T5-IR17.md:104` | L-C08 | none | Hand-typed `song.ini` text is input to the real `song_ini_pro_drums`; the split with song.cpp is a side note, not a key. |
| `counts/T5-IR17.md:105` | L-C08 | none | Same, hand-typed `song.ini` text as input. |
| `counts/T5-IR19.md:63` | L-C10 | none | Counter's own reason says synthetic audio files are input the real reader reads, so no stand-in; no key fits. |
| `counts/T5-IR19.md:65` | L-C10 | none | Hand-built ID3 prefix is input to the real reader, so no stand-in; no key fits. |
| `counts/T5-IR19.md:101` | L-C10 | P-message | The counter's X9 mark is `FakeClipboard` (`uitest_paths.cpp:69`, a stand-in for the Windows clipboard), and no key in `keys.md` covers that, so the row needs a "no key" for its X9 part beside P-message. |
| `counts/T5-IR3.md:25` | L-C13 | none | Hand-cut `dump.cs` fragment read by the real parser is input (keys.md, question 1); nothing else here is a stand-in. |
| `counts/T5-IR3.md:26` | L-C13 | none | Same: hand-cut dump fragment is input. |
| `counts/T5-IR3.md:27` | L-C13 | none | Same. |
| `counts/T5-IR3.md:28` | L-C13 | none | Same. |
| `counts/T5-IR3.md:29` | L-C13 | none | Same. |
| `counts/T5-IR3.md:30` | L-C13 | none | Same. |
| `counts/T5-IR3.md:31` | L-C13 | none | Same. |
| `counts/T5-IR3.md:32` | L-C13 | none | Same. |
| `counts/T5-IR3.md:33` | L-C13 | none | A hand-written class line read by the real parser is input. |
| `counts/T5-IR3.md:34` | L-C13 | none | Hand-cut fragment is input. |
| `counts/T5-IR3.md:39` | L-C13 | none | Hand-cut fragment is input. |
| `counts/T5-IR3.md:45` | L-C13 | none | Tiny hand dump with a hand page is input. |
| `counts/T5-IR3.md:46` | L-C13 | none | Same. |
| `counts/T5-IR3.md:48` | L-C13 | none | Tiny hand dump and page are input. |
| `counts/T5-IR3.md:49` | L-C13 | none | Same. |
| `counts/T5-IR3.md:50` | L-C13 | none | A hand constants file the real reader reads is input. |
| `counts/T5-IR3.md:51` | L-C13 | none | Tiny hand dump is input. |
| `counts/T5-IR3.md:52` | L-C13 | none | Same. |
| `counts/T5-IR3.md:53` | L-C13 | none | `test_python_name_and_file_names_are_not_game_facts` passes typed names to `D.game_fact`; no dump, game memory or message is faked. |
| `counts/T5-IR3.md:57` | L-C13 | none | Tiny hand dump, hand page and hand-edited table are input. |
| `counts/T5-IR3.md:58` | L-C13 | none | Hand page is input. |
| `counts/T5-IR3.md:62` | L-C13 | none | Map dump and hand page are input. |
| `counts/T5-IR7.md:55` | L-C17 | none | TP:1254 only builds three `Press` objects and calls `retime_from_end` on them; `_play_schedule` and the fakes are in other tests (1223), not this one, so it is the same hand-built-Press case as :74 to :79. |
| `counts/T5-IR7.md:74` | L-C17 | none | Hand-picked ratios 0.5 and 2.0 given to a pure function; hand-built input, no fake class and no typed product value. |
| `counts/T5-IR7.md:75` | L-C17 | none | Hand-built range (990, 1010) given to the real function; input only. |
| `counts/T5-IR7.md:76` | L-C17 | none | Hand-built range with 1e9 stand-ins; input only. |
| `counts/T5-IR7.md:77` | L-C17 | none | Hand-built press with no fallback; input only. |
| `counts/T5-IR7.md:78` | L-C17 | none | Hand-built fallback block; input only. |
| `counts/T5-IR7.md:79` | L-C17 | none | Hand-built presses; input only. |
| `counts/T5-IR9.md:92` | L-C19 | none | Hand-built delta map for the pure `over_share` function; input to real code with golden.json's own limits, and no key fits. |
| `counts/T5-KR4.md:62` | L-C21 | none | Built from one made-up row that the real code reads, which is input; no stand-in for the code's output. |
| `counts/T5-KR5.md:28` | L-C21 | none | X9 rests on a wrong md5 as a probe over a real chart and path; that is input. |
| `counts/T5-KR5.md:31` | L-C21 | none | Real scan, real store; the counter's X9 mark has no stand-in under it. |
| `counts/T5-KR5.md:117` | L-C21 | none | Hand-built scene notes are chart input to the drawing code, not a stand-in for any of its output. |
| `records/A14.md:586` | L-R02 | none | Uses a counting fit in place of the real `render::ellipsize`; that fake function fits no key (C-stub is a program). |
| `records/A14.md:603` | L-R02 | none | Same counting-fit stand-in for the real cut; checks width refit only. |
| `records/A14.md:620` | L-R02 | none | Same counting-fit stand-in; checks new text refit only. |
| `records/A14.md:637` | L-R02 | none | Same counting-fit stand-in; checks clear forgets cells. |
| `records/A14.md:654` | L-R02 | none | Same counting-fit stand-in; checks `Memory::reset` empties the cache. |
| `records/A14.md:671` | L-R02 | none | Checks that an internal reset key moves, standing in for "a dragged column lets go"; no key names a stand-in for a screen result. |
| `records/A18.md:285` | L-R02 | none | K-nocode means a test of no Hydra code, but `needs_timing` is Hydra code that has no production caller (record line 289, finding P1); the scout's own side note says it was used loosely. |
| `records/A24.md:98` | L-R04 | none | Leans on the first .mid in file order; no typed constant and no key names a hidden dependence on corpus order. |
| `records/A24.md:120` | L-R04 | none | Same fault: checks one chart picked by file order, not every corpus chart. |
| `records/A25.md:535` | L-R04 | none | Walks the table through a wrapper that no production code calls; the walk is a property over the owner's table, so no key names a dead wrapper. |
| `records/A27.md:201` | L-R04 | none | Checks `readers.size() == 2` where identity matters; a weak check, not a fake, typed constant or other listed fault. |
| `records/A39.md:149` | L-R06 | none | Sets the button's two flags by hand and checks `active()`/`playing()` as a stand-in for the audio device; none of the keys names a copy of a button body. |
| `records/A4.md:145` | L-R06 | none | If workers stick, the test hangs the suite instead of failing; no key names a missing cap on a wait. |
| `records/A46.md:150` | L-R07 | none | Counts vertices of two drawn chips to stand for "an outline is drawn"; a proxy check, no typed answer, fake or cite, so no key fits. |
| `records/A7.md:420` | L-R07 | none | Reads the `report_started` flag, not the strip's drawn words that the claim is about; a proxy, and no key names it. |
| `records/A9.md:79` | L-R08 | none | Fault is only that no real settings file is loaded; inputs are the file's own text, so no key fits. |
| `records/A9.md:143` | L-R08 | none | Fault is only a missing real-file load; the inputs are file text, so no key fits. |
| `records/A9.md:207` | L-R08 | none | Fault is only a missing real-file load with a `#` folder; inputs are text, so no key fits. |
| `records/A9.md:271` | L-R08 | none | Fault is only that no real chart-mode keys from a hydra.db are used; the checks are properties, so no key fits. |
| `records/A9.md:399` | L-R08 | none | Fault is only a missing real-input check; the lines compare the owner's own values, so no key fits. |
| `records/A9.md:431` | L-R08 | none | Fault is only a missing real old-file load; it loops the owner's own list, so no key fits. |
| `records/A9.md:558` | L-R08 | none | Fault is only a missing real settings-file copy; the checks are properties, so no key fits. |
| `records/A9.md:574` | L-R08 | none | Fault is only a missing real-file byte comparison; line 817 is partly implied by 821, so no key fits. |
| `records/A9.md:590` | L-R08 | none | Fault is only a missing screen capture of the save-failed notice; no key fits. |
| `records/C6.md:774` | L-R11 | none | Rows 60 and 390 are unexplained free inputs; the fault is unexplained inputs, which no key covers. |
| `records/C6.md:1018` | L-R11 | none | Seven unrelated claims in one case with unexplained widths; the first failure hides the rest, and no key names that. |
| `records/C7.md:268` | L-R11 | none | Relies on timing (the first request still running); no clock or other key names that. |
| `records/D13.md:501` | L-R13 | F-watchlog, P-other | The record says TM-14 refutes the claim the tests pin (the +0x2E0 field is a hit time at every chord), and no key names that fault. |
| `records/D13.md:513` | L-R13 | F-watchlog, P-other, P-message | Line 2, 3 of the record says the test "differs from TM-14". |
| `records/D13.md:517` | L-R13 | F-watchlog, P-other | Record says "As 90", which is the TM-14 contradiction. |
| `records/D13.md:521` | L-R13 | F-watchlog, P-other | Record says "As 90". |
| `records/D13.md:525` | L-R13 | F-watchlog | Record says "As 90"; the scout's own note says the row "sits in the refuted claim". |
| `records/D13.md:529` | L-R13 | F-watchlog | Record says the test "sits inside the refuted claim". |
| `records/D13.md:533` | L-R13 | F-watchlog | Record says "As 90". |
| `records/D13.md:590` | L-R13 | F-watchlog, P-message | Record says "the PASS +0x2E0 line on hand rows is the refuted claim". |
| `records/D2.md:673` | L-R14 | none | Three scenes in one test (a split-the-claims fault); no key names a bundled test. |
| `records/D2.md:731` | L-R14 | none | Eight bad-row claims in one test with joined substring checks; no key names a bundled test. |
| `records/D2.md:764` | L-R14 | none | Four claims in one test (rows, error, count, warning); no key names a bundled test. |
| `records/D2.md:775` | L-R14 | none | Three claims in one test; no key names a bundled test. |
| `records/D2.md:1365` | L-R14 | none | Two claims in one test plus a mid-test re-point of `out_path`; no key names a bundled test. |
| `records/D3.md:484` | L-R14 | none | Fixed `time.sleep(1.0)` leans on timing; no key names a sleep. |
| `records/D3.md:540` | L-R14 | none | Fixed 0.5 s sleep races the slow call; no key names a sleep. |
| `records/D3.md:546` | L-R14 | none | Same 0.5 s sleep fault; no key names a sleep. |
| `records/D3.md:554` | L-R14 | none | Same 0.3 s sleep fault; no key names a sleep. |
| `records/D3.md:561` | L-R14 | none | Same 1.0 s sleep fault; no key names a sleep. |
| `records/D4.md:761` | L-R14 | none | 200 rows is an unnamed free input and only column 0 is checked; no key names an unnamed input or a thin check. |
| `records/E3.md:526` | L-R16 | none | Checks for any "__" in the shim, which a random temp folder name can contain, so it can fail by luck; none of the keys covers a flaky loose check. |
| `records/T3-run2.md:71` | L-R17 | none | Only fault is that no real chart is run; the section names are the ones the test was given. |
| `records/T3-run2.md:136` | L-R17 | none | Pure arithmetic on a playhead with no chart; the only fault is no real chart. |
| `records/T3-run2.md:151` | L-R17 | none | Same: clamps checked on typed numbers, no chart. |
| `records/T3-run2.md:166` | L-R17 | none | Same: no chart; the expected values follow the rule. |
| `records/T3-run2.md:181` | L-R17 | none | Checks no path hides the box; only a real chart is missing. |
| `records/T3-run2.md:213` | L-R17 | none | Start values read as the rule; only a real chart is missing. |
| `records/T3-run2.md:228` | L-R17 | none | The rule is cited (D48 Q27); only a real chart is missing. |
| `records/T3-run2.md:331` | L-R17 | none | Reads the engine's totals; only the chart behind it is unnamed. |
| `records/T3-run2.md:719` | L-R17 | none | Checks a positivity rule on three numbers; only a real chart is missing. |
| `records/delta-DL2.md:361` | L-R18 | none | Helper StoreLockHolder lacks exception handling and duplicates HeldSaveGroup; a helper fault, not one of the keys. |
| `records/delta-DL3.md:599` | L-R18 | none | Reads the raw report_started flag where the claim is about the strip's drawn text; no key fits. |
