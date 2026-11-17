Agent id: acdae1ae465721485 (task T8-E3-V-L-R16-L-R20, session f3bbb32b)

I opened every record row of both batches at its line in `row`, under AUD/records. I did not open any test. Labelled from the record text only; the run column was not used.

## Counts

- L-R16: rows in the batch 92, lines in the label file 92 (every row once, same order), rows I agree with 91, rows I change 1.
- L-R20: rows in the batch 16, lines in the label file 16 (every row once, same order), rows I agree with 8, rows I change 8.

Ends tables: every key in both ends tables is carried by the rows named in it, and those rows are in the batch. Two ends-table cells change because of my row changes (see below).

## Rows I change

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-R16 | records/D9.md:270 | F-result, K-nocode | F-schedule, P-other, K-nocode | The record's fault is a hand-built `Schedule` fixture (F-schedule, "schedules"), not a result; it also types 7201/7200 as literals where the half is a named value (record lines 274-275). |
| L-R20 | records/window-WN6.md:76 | P-burnout, Z-cite | P-burnout, P-pbticks, Z-cite | The record lists "ticks 36480 and 74880 (harness)" among the typed figures, which is X2 (P-pbticks). |
| L-R20 | records/window-WN6.md:92 | P-other, Z-cite | P-pbticks, P-other, Z-cite | The typed tick 133920 (`kLongSongFillTick`) is named by X2 in cross-findings.md line 37 as a Path Builder fill tick; the card figures stay P-other. |
| L-R20 | records/window-WN6.md:172 | P-burnout, Z-cite | P-pbticks, Z-cite | The record's typed values are tick 36480 as an offered list row and the clock text "m20 · 0:25.123", which is exactly P-pbticks (fill ticks and their clock text); no Burnout score is typed. |
| L-R20 | records/window-WN6.md:188 | P-burnout, Z-cite | P-pbticks, Z-cite | Same: the typed values are the ticks kFirstFillTick and kSecondFillTick and the clock text "0:51.421 / 2:10.088". |
| L-R20 | records/window-WN6.md:220 | P-burnout, Z-cite | P-pbticks, Z-cite | Same: ticks for `click_fill` and `choice_ref` and the clock text "0:51.421 / 2:10.088". |
| L-R20 | records/window-WN6.md:236 | P-burnout, Z-cite | P-burnout, P-pbticks, Z-cite | The record types the pending row and step row (Burnout) and also `kEarliestTick` 36480. |
| L-R20 | records/window-WN6.md:252 | P-other, Z-cite | P-pbticks, Z-cite | The typed values are the tick 162240 (`kLateFillTick`, named in X2) and the readout "2:28 - 5:28 of 5:59"; nothing else is typed. |
| L-R20 | records/window-WN8.md:145 | P-other, Z-cite | P-other, S-snapshot, Z-cite | The record calls 9956 a number "read from one run" of the first corpus chart with no working, the same fault the scout marked S-snapshot at WN7:266. |

Effect on the ends tables: L-R16's F-result row (plainest and least-fitting D9.md:270) becomes F-schedule on the same row. L-R20 needs a P-pbticks line (plainest records/window-WN6.md:172, least-fitting records/window-WN6.md:236), and P-other's plainest row stays WN6:92 while its least-fitting is WN8:145. S-snapshot gains WN8:145 as a second row (plainest WN7:266, least-fitting WN8:145). P-burnout's plainest stays WN6:204.

## Rows I checked and kept that the brief asked about

- No C key for the fake stall module and fake delete rule in `test_progress.py`: kept. The entries at E2.md:434-871 do not name the fakes; only the file's group note (E2.md:398, "delete_rule, stall, media_rule ... are faked (lines 66-114)") does. Rows that route through `percall.body` are E2.md:563 and :584; their own entries name no fake. See the first side note.
- No O-payload on any E1 to E3 row: kept, and kept both ways. No row in L-R16 carries O-payload or C-gate, and no entry names a hook payload as its fault. The one hand-typed input an entry names as a fault is the transcript at E1.md:101, which has O-transcript.
- records/E3.md:526 `no key`: agreed. The fault (a loose "__" check that can fail on a random temp name) fits no key.
- records/E3.md:821 K-cantfail: kept, but it is the weakest fit. The check can fail; the record's fault is that one assertion after a loop hides which of ten bad records slipped.

## Side notes

- records/E2.md:398 (group note for all of `test_progress.py`) says the tests fake `delete_rule`, `stall`, `media_rule` and the transcript read, and says every payload is hand-typed; E2.md:5 says the same for the four E2 test files. If the main session reads a group note as part of each record, rows E2.md:563 and E2.md:584 (and the other `percall.body`-routed progress rows) would take C-gate and O-payload. I kept the stricter reading in the brief. That is a rule choice for the main session, not mine.
- records/E1.md:7-8 and records/E3.md:35 say the same of E1 and E3 payloads ("hand-typed ... claim not checked"). The same choice applies to them.
- records/D9.md:361 and :465 type the posted caps (156.0, 36.0). The record says it is the same rule as :342 and :494; the keys agree across the four rows (P-other).
- records/D9.md:123 states a timing risk (0.05 s waits can flake). No key fits flakiness; the same fault is the reason records/E3.md:526 is `no key`. Two rows now carry a flakiness fault with no key between them.
- records/window-WN6.md:76 and later: every figure the records call movable is also fixed by whether Burnout's fills are offered at 60 ms (the setup `click_fill`). That dependence is what P-pbticks names; the scout of L-R20 wrote the same thing as a side note without a key.
- records/window-WN7.md:198 and records/window-WN4c.md record 1 type the same Burnout figures in two test files (`test_path_list.cpp:268`, `test_activation_rows.cpp:582`); the scout noted it, and I confirm it from record lines 190 and 211.
