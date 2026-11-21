Agent id: adb15cdcbc7b74f6c (task T8-E3-V-L-R02-L-R03, session f3bbb32b)

## Counts

- L-R02: rows in the batch 90, lines in the label file 90 (one line per row, none missing, none doubled), rows I agree with 90, rows I change 0.
- L-R03: rows in the batch 93, lines in the label file 93 (one line per row, none missing, none doubled), rows I agree with 91, rows I change 2 (3 keys added in all; see the table).

Both ends tables are sound: every row they name is in its batch and carries the key it is named for. Every key used in each batch has an ends line.

## Rows I change

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-R03 | records/A19.md:440 | S-snapshot, P-other | S-snapshot, P-other, Z-cite, P-message | The record's own reason (line 446) quotes the test comment "its line is pinned from one run" as the source of 86, and line 447 says the tool's printed text is compared with typed text. |
| L-R03 | records/A23.md:11 | K-floor, S-snapshot | K-floor, S-snapshot, Z-cite | The record (line 21) quotes the test comment "a floor pinned from one run on the corpus" as the only source of 382 and 294, which is a comment naming the value's source. |

## The two questions in the dispatch

FittedTextCache rows (L-R02, records/A14.md:586, 603, 620, 637, 654, 671). I agree with `no key` for all six. Rows 586 to 654 hand a counting function to the cache in place of the real cut (`CountingFit`, tests/test_fitted_text_cache.cpp:16-24, whose header comment says it stands in for the real cut). The C keys name a fake analyzer, reader, worker, rig, clock, session, program, gate module or self-defined checker. None covers a small fake function handed to Hydra's own cache, and C-stub is for stub programs. No X10 to X12, K or P key fits their fault either. Row 671 stands a moving reset key in for "a dragged column lets go" (record line 682). No key names that stand-in, so it stays `no key`. I did not stretch any key to them.

Z-cite and K-comment (L-R03). They are not swapped. Z-cite is on rows whose records quote a test comment as the value's source ("Pinned from one run", "These literals pin one run"), for example A22.md:646 to 1089, A19.md:21, 285, 337, 386 and 403, A20.md:504 and 521, A23.md:53. K-comment is on rows whose raw numbers have their working only in a comment, for example A21.md:30, A23.md:370 to 433, A2.md:127, A20.md:104. A23.md:53 has Z-cite but not K-comment, which is right: its record says the working is "in no line", not in a comment. The only slips were the two missing Z-cites in the table above.

## Side notes

- records/A18.md:285 (L-R02): K-nocode is used for a function (`Activation::needs_timing`) that is Hydra code with no production caller (record line 289). The key's definition covers helpers in the test file, language behaviour or a signature, so the fit is loose. No key from the list fits better. Leave it for the main session to see when it reads the K-nocode group.
- records/A2.md:267 (L-R03): the record says lines 749 and 751 cannot fail without 745 failing. The scout used K-duplicate. K-cantfail could also be argued, but the record says "All can fail", so I left it.
- records/A19.md:285 and the scout's own side note: the record shows "-0.0 ms" in one place and "0.0 ms" in another for the same case (two outputs for one value). I read A19.md:286 to 301 only through the scout's note and did not check it.
- records/A14.md:524 (L-R02 scout's note, record 29): the record names a possible split between the Preview's `sp_meter_bars_at` and the engine's trailing-fill list. Not checked by me; it is a finding for the main session.
- records/A22.md:818 and 869 (L-R03 scout's note): the record flags `saved_enough` in `src/ui/preview_timeline.cpp:493-515` as the display working out enough Star Power itself beside `sp_ready_ms`. Possible split; not checked by me.

## Not checked

I read every row's scout keys against the batch file and the ends tables. I opened the record text for these rows only: records/A14.md:21, 38, 586 to 671; records/A18.md:285; records/A2.md:267; records/A19.md:440; records/A23.md:11, 96. The other rows I judged from the scout's note against the batch title, the counter reason and the record greps for "pinned from one run". I did not open the test source files except tests/test_fitted_text_cache.cpp lines 1 to 48.
