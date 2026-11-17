# E5: the side notes and their marks

T8 step E5 (`docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md`). Written by the main session, session f3bbb32b, 2026-11-16.

The audit's agents wrote side notes, splits and production-code findings into their record, verify, capture and sweep files. A script (`../ledger.py`) pulled 1,664 of them into `../notes.tsv`. Scouts gave each one a mark: `repeat` (the record's own verdict covers it), `split` (two parts answer one question differently), `prod` (a fault in `src/` or `tools/`), `finding` (any other problem with a test, record, doc or citation) or `empty` (not a finding). A mark is a fact, not an exit. Only the main session picks exits, at E7.

## The files

`marks.tsv` holds one line per pulled note with its final mark, the batch that made it, and the scout's one-sentence reason. `new-rows.tsv` holds the notes the scouts found that the script missed. `merge_marks.py` builds both, and it fails if any scout's table does not equal its batch.

`extra-rows.tsv` holds 82 rows the plan asks for that the note pull does not make one by one, built by `extra_rows.py`. Seventeen are the tests and records `capture/byreading/SUMMARY.md` names under "Side notes that need an exit in T8" and "Rule slip"; `notes.tsv` holds that section only as two block rows. Sixty-five are run tests that no record names, from `mutation/mutation-unmatched.md` (plan step E1c). All 82 are marked `finding`.

`files-read.md` lists, by name, the 61 files a scout read whole and the 188 it did not reach.

Every one of the 241 files that the plan's searches hit ("side note", a line starting "split", "production-code finding") has at least one row in `notes.tsv`, checked by script.

| Mark | Pulled notes | New rows |
|---|---|---|
| finding | 634 | 191 |
| empty | 591 | 3 |
| split | 248 | 13 |
| prod | 114 | 26 |
| repeat | 77 | 11 |
| **all** | **1,664** | **244** |

## How the marks were made

The first round (batches `N01` to `N09`, brief `../../briefs/T8-E5-notes.md`) gave each scout about 200 notes and 13 to 82 files. That was more than a scout can read inside its caps. Most scouts marked rows from the batch text and searched their files instead of reading them. A spot check of six such marks found five wrong.

So every note not marked from its file went to a second round (`remark/R01` to `R09`, brief `../../briefs/T8-E5-remark.md`), chosen by `remark/make_remark.py`. That is 1,357 notes. Each was opened in its file and marked again; 407 marks changed. The other 307 kept their first mark, because their scout had opened them in the file: all of N09, N04's non-empty rows, and N07's rows outside the seven files it did not read through.

## What this does not cover

The second job of E5 was to find notes the script missed, by reading every file whole. The user chose the cheaper route (session f3bbb32b), which re-marks the pulled notes and does not re-read the files. So the search for missed notes stands only where a first-round scout read a file whole: the 61 files under "Read whole" in `files-read.md`. In the 188 files under "Not reached", a note written in a shape the script does not know may still be unread. The 244 new rows come from those whole reads and from the first round's searches, so they are not a complete list.
