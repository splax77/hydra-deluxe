Agent id: af198acbde3e75293 (task T8-E3-V-L-R04-L-R05, session f3bbb32b)

I opened every record at the line named in `row`, for both batches. The scouts' agent ids (a7a7d28f2fd255b7b for L-R04, ae279cfb71339a90e for L-R05) differ from mine. Each fault key I add below is one the record's own text names and the scout left without a key. I never used the `run` column.

## Counts

- L-R04: 92 rows in the batch, 92 lines in the label file (each row exactly once), 83 rows I agree with, 9 rows I change.
- L-R05: 93 rows in the batch, 93 lines in the label file (each row exactly once), 76 rows I agree with, 17 rows I change.

Ends tables. L-R04: every row named in the ends table is in the batch and carries the key. L-R05: every row named carries its key, except F-db's least-fitting row `records/A32.md:95`, which loses F-db under my change below. My changes add keys to some rows, so the ends tables name rows that still carry their keys. The new `F-row` rows (`records/A33.md:149`, `:353`) are not in the L-R05 ends table, which has no F-row line.

## Rows I change

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-R04 | records/A24.md:267 | P-message, P-builtchart | P-message, P-builtchart, K-comment | Record: "raw results; each sum is worked in the comment, not on the line" (50, 100, 250). |
| L-R04 | records/A24.md:374 | K-duplicate, P-builtchart | K-duplicate, P-builtchart, K-comment | Record: "every figure is a raw result worked in the comments or in the setup". |
| L-R04 | records/A24.md:444 | K-skip, K-floor | K-skip, K-floor, K-cantfail | Record says line 1617 (bonus below SP points) "holds by construction ... cannot fail". |
| L-R04 | records/A25.md:80 | F-row, P-other | F-row, P-other, K-comment | Record: the typed tile numbers "have their working in a comment only". |
| L-R04 | records/A25.md:96 | F-row, P-other | F-row, P-other, K-comment | Record: "Raw results; the working (1200+2+1) is in the comment". |
| L-R04 | records/A25.md:341 | P-other | P-other, P-setting | The record types the subtitle "5 charts — " and the tile "5 of 5", screen words, as it does for A25.md:357. |
| L-R04 | records/A27.md:133 | K-duplicate | K-duplicate, K-namebody | Record: address equality "stands in for built once", and the test's own comment says it is "not a proof" (same shape as A26.md:474). |
| L-R04 | records/A28.md:80 | K-skip | K-skip, K-selfmade | Record: the expected side calls the owner the graph calls, "so only the push loop is guarded, not the window rule". |
| L-R04 | records/A28.md:192 | K-skip | K-skip, K-selfmade | Record: the first claim "is true by construction for engine-made records", because `set_sqout` stamps both together. |
| L-R05 | records/A29.md:67 | P-other, F-result, K-selfmade | P-other, F-result, K-selfmade, K-comment | Lines 113-114 are raw results whose working is "in a comment". |
| L-R05 | records/A29.md:85 | P-other, F-result, K-selfmade | P-other, F-result, K-selfmade, K-comment | Lines 143-152 are typed bars whose working is in comments ("0.25 bars per second"). |
| L-R05 | records/A29.md:316 | K-duplicate, P-other | K-duplicate, P-other, K-unreachable | Record 11: "Real: no", and the verdict says it catches "a state the engine does not produce". |
| L-R05 | records/A3.md:169 | P-setting | P-setting, K-comment | Record: "strings typed with the working only in a comment". |
| L-R05 | records/A3.md:302 | P-other, Z-cite | P-other, Z-cite, S-snapshot | Record: line 1448 "pins the search's pathstring '5- 2+ E2- E0 E0'" and "fails on any search change". |
| L-R05 | records/A3.md:369 | K-skip, K-floor, Z-cite | K-skip, K-floor, Z-cite, K-selfmade | Record 2: the check "compares the view with the function that fed it". |
| L-R05 | records/A30.md:370 | K-duplicate, K-cantfail | K-duplicate, K-cantfail, K-unreachable | Record 11: "Real: No; a stored scale that disagrees with the chart cannot be produced by the engine". |
| L-R05 | records/A30.md:493 | P-other | P-other, S-snapshot | Record: "'any change at all' marks a snapshot". |
| L-R05 | records/A31.md:121 | S-snapshot | S-snapshot, K-unreachable | Record: the `trim_end` option is set by no code in src or tools, "not a failure any player can meet". |
| L-R05 | records/A32.md:95 | K-cantfail, F-db | K-cantfail, K-unreachable | Record: the half-added state "cannot happen from this code" and no release wrote it; the file is one Hydra wrote, altered by one DROP COLUMN, not a database written in SQL by hand. |
| L-R05 | records/A32.md:359 | P-message | P-message, K-namebody | Record: line 955 checks one table only, so "leaves nothing behind" is promised and not checked. |
| L-R05 | records/A33.md:53 | K-selfmade | K-selfmade, K-cantfail | Record: "122 only fails if record_of keeps more than five, and the fixture has one path, so it cannot fail on this input". |
| L-R05 | records/A33.md:149 | P-other | P-other, F-row | Record 3: the library rows are "hand-made rows" from `library_row`. |
| L-R05 | records/A33.md:244 | P-message, K-namebody | P-message, K-namebody, K-cantfail | Record: line 378 "is true by construction: the test constructs the object it asks about". |
| L-R05 | records/A33.md:353 | P-other | P-other, F-row | Record 3: "two hand-made library rows (`library_row`)". |
| L-R05 | records/A33.md:431 | K-skip | K-skip, K-floor, K-cantfail | Record: `records > 0` is "a floor", and lines 654-685 "assert nothing" and only print. |
| L-R05 | records/A34.md:316 | K-cantfail | K-cantfail, K-namebody | Record: the name promises chunking but it "never crosses" the limit it names. |

## The scouts' own doubts

- L-R04 `records/A24.md:98` and `:120`: I agree with `no key`. The fault in both is the dependence on the first `.mid` in file order, which no key names.
- L-R04 `records/A25.md:535`: I agree with `no key`. The record's fault is that the test calls a wrapper no production code calls. Its sibling `:664` got K-selfmade because it compares the wrapper with the function it wraps; `:535` does not.
- L-R04 `records/A27.md:201`: I agree with `no key`. The `size() == 2` check is weak (it passes with the wrong two readers), but it is not a fake, a typed constant, a duplicate or any other listed fault.
- L-R05 F-db on `records/A32.md:28`: I keep it. The 1.6 table is typed in SQL (the test, lines 138-148) and the record says no real 1.6 file is available, so R-db's fix has no file to point at. See the side note.
- L-R05 F-db on `records/A32.md:95`: I take it off (see the table). That leaves F-db with one row in this batch.
- L-R05 F-result on the A29 sp-meter rows: I keep it. The records say the activations and `sp_end_steps` are hand-built stand-ins for the search's output (A29.md:67 reads "a hand-built activation", record 4 says the fixture "is not a record the engine can produce"). The rows that carry only `P-other` (`:140`, `:278`, `:401`, `:419`) read the steps from the real search or the display, so I leave them.

## Side notes

- `records/A32.md:28`: the 1.6 file the test types cannot be replaced by a real file (README names v1.8.4 as the oldest real one), so any question under F-db for this row has no R-db remedy. The main session should see this when F-db is asked.
- `records/A24.md:426` and `:339` carry a split the record calls SPLIT-A (SC-10 against SC-38, the chord pricing order); the scout already noted it.
- `records/A24.md:483` and `records/A25.md:535`/`:664`, `records/A30.md:370`, `records/A31.md:121`: each tests code with no caller in src or tools (`assign_score`, `report::tier_for`, `trim_end`, `decode_audio`). The scouts noted these.
- `records/A3.md:369`, `:285`, `:302`: the tag rule at `ui-and-display.md:107` is Unconfirmed and lines 107 and 108 contradict each other (the record's Split B); not a ruling.
- `records/A3.md:15`: its record says "no fill with time to spare is in the seven charts" and the fixture is a hand-built activation. I left F-result off because the record's verdict is Remove on duplication.
- `records/A32.md:95` and `records/A34.md:316`: both pass with the thing they name removed (rollback, chunking); I labelled K-cantfail as the scouts did. The records state this as their finding.
- Test names with commas (the doctest `-tc` filter splits on them) are noted in the records and were already recorded by the scouts.
