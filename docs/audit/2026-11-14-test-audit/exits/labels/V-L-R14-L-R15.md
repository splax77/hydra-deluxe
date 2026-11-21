Agent id: a85329f329d4427d1 (task T8-E3-V-L-R14-L-R15, session f3bbb32b)

I opened every row's record at the heading line in `row` (the claim, items 4, 5, 8, 9, 10 and the verdict) and judged the scouts' keys against `keys.md` and Revision 1. I did not use the `run` column. I did not open the test files, because the only evidence for these rows is the record's own text.

## Counts

- L-R14: 79 rows in the batch, 79 lines in the label file (same row set, no row missing, none twice), 76 rows I agree with, 3 rows I change.
- L-R15: 80 rows in the batch, 80 lines in the label file (same row set, no row missing, none twice), 80 rows I agree with, 0 rows I change.

Ends tables: every plainest and least-fitting row named in both tables is in its batch and carries the key it is listed under. One consequence of my change: L-R14's only `P-name` row is D3:235, which I move to P-other, so L-R14's ends table loses its P-name line and P-other gains a row (the plainest and least-fitting rows it already names for P-other still hold).

## Rows I change

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-R14 | records/D2.md:966 | K-namebody | K-namebody, K-cantfail | The record's item 8 says the sleep-the-whole-cap bug still passes, so "the line cannot fail for the bug named" is a check that cannot fail. |
| L-R14 | records/D3.md:235 | P-name, S-snapshot | P-other, S-snapshot, K-skip | The typed obfuscated method name belongs to the game's build, not a name Hydra writes (P-name), and the record's own verdict also asks to "report the skip" when the facts file is absent. |
| L-R14 | records/D3.md:453 | K-duplicate, P-message | K-duplicate, P-other | The typed 12,000 is a cap constant (record item 5: "12_000 has no source named"), not a message the code prints. |

## The eleven `no key` rows in L-R14: no key fits

I read each record and tried every key in `keys.md` and Revision 1. None fits, so they stay `no key`.

- D2:673, 731, 764, 775 and 1365 are the bundled-claims fault: one test holds several claims, so a failure does not name its claim. K-partial (a hand-picked list standing for "every") and K-namebody (a promise the body never makes) are about a different fault. D2:731 also says its joined substring checks can pass for the wrong row, but that is a weak check, not "cannot fail". D2:1365 also leans on a mid-test re-point of `out_path`; still no key.
- D3:484, 540, 546, 554 and 561 are fixed `time.sleep` calls that race the worker. C-clock is a fake clock; these sleeps are real. K-cantfail and K-floor do not describe a timing race. Stays `no key`.
- D4:761: the 200 rows are an unnamed free input and only column 0 is checked. P-other needs a constant the code owns, and 200 is the test's own number. K-cantfail would need a check that cannot fail, and this one can. Stays `no key`.

## The 38 P-other rows in L-R15: all stay P-other

I checked each against the narrower P keys. All 38 are typed constants or copies the code or a data file owns, and none is a better fit for P-burnout, P-pbticks, P-allister, P-tiers, P-message, P-placeholder, P-setting, P-name, P-onefile or P-builtchart. The rows are D5:142, 591, 786, 833; D6:156, 182, 195, 230, 254, 266, 278, 422; D7:48, 86, 92, 98, 155, 220, 267, 317, 341, 350, 431, 438, 935, 940; D8:93, 167, 174, 180, 187, 206, 276, 317, 330, 611, 735, 817. Two I weighed longest:

- D7:341 (typed 0.02 for the recorder's default rate): P-setting means a Hydra settings default or screen word. This is a tool's own constant, so P-other.
- D5:142 (whole parameter lists of three tool functions): P-name means a name the product writes. Parameter names are code shape, so P-other with K-nocode, as the scout labelled it.

## Side notes

- records/D2.md:1949 to 1951 (production findings): `tools/ch_mcp/dump_facts.py:141-144` names the field "0D58" by splitting at the last space; `tools/ch_mcp/frida_attach.py:182-183` prints "1 messages seen"; `foreground.py:20-21` and `dump_build.py:22-25` cite user answers the record could not find. These are not label matters; the main session should route them.
- records/D2.md:1955 to 1956 (Splits): "Is this backticked span a game fact?" is answered by `game_fact` in `tools/ch_mcp/dump_facts.py:566` and by the span reader in `tests/test_docs_match_code.cpp`; "Which agent file may call which game method" is answered by the test helper and by `kit_methods.json` plus kit.ts's `ALLOWED_CALLS`. Both are splits for the user.
- records/D3.md:612 (K-cantfail by the scout) also shares the fixed-sleep fault of the five `no key` D3 rows (record: "nothing proves the call was open"); the scout's side note counts it with the sleep rows, which is right, but its key stays K-cantfail.
- records/D4.md:584, 596, 604, 611 and 620 say the Freak, Mouths and Bomber fixture files were not opened, so the typed numbers' source (Z-cite) was never checked by the record. The same is true of records/D5.md:34, 43 and 49.
- records/D4.md:126 and 431 say the 30 s and 120 s constants were "not found" by the record; I did not search for them either.
- records/D5.md:55, 142, 158 and 858 (four tests, one removed-argument promise) and records/D6.md:803 to 832 (one-line records with no Read or reason lines) are as the L-R15 scout's side notes say; I confirmed the first set from the records and the second from the record text.
- records/D8.md:1004: the name says thirty-seven and the typed set holds 38; confirmed in the record.
- records/D7.md:235: the dead test replaced by a same-named function at :682, so Python never runs it; confirmed in the record. K-skip is the closest key, but it is not a skip in the usual sense. It may be worth a `no key` look at E4 if the main session wants the dead-test case separate.

## Not checked

None. All 79 L-R14 rows and all 80 L-R15 rows were opened at their record line.
