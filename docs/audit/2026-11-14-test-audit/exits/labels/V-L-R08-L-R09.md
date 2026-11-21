Agent id: abbf957f3d29877bd (task T8-E3-V-L-R08-L-R09, session f3bbb32b)

I opened every record at the line in `row` (A9, B1, B2 for L-R08; B3, B4, B5, B6, B7, C1, C10 for L-R09). I did not open any test file. I did not use the `run` column. Every row in both batches is a Rewrite or Remove record, so the record text was the only evidence. The row ids were diffed against the label files by script: exact match, no row missing, no row twice.

## Counts

- L-R08: rows in the batch 92, lines in the label file 92, rows I agree with 83, rows I change 9.
- L-R09: rows in the batch 87, lines in the label file 87, rows I agree with 68, rows I change 19.

Nine A9 rows the scout left `no key` (A9:79, 143, 207, 271, 399, 431, 558, 574, 590): I agree with all nine. Each record's only named fault is "no real settings file, real hydra.db keys or screen capture was used"; the inputs are the file's own text and no key in `keys.md` names that fault.

## Rows I change

Rule I applied: a key stays only if the record's own text shows the fault, and a fault the record names gets a key. Adds are keys the record's text shows that the scout left off. "working in a comment" means the record says the expected value's working sits only in a comment (K-comment). "a literal from one run" or "pinned from one run" in the test's comment is Z-cite.

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-R08 | records/A9.md:542 | K-duplicate, K-cantfail | K-duplicate, K-cantfail, K-unreachable | Its question 11 says "Real: no. A file cannot hold 7; only in-memory", the same fault the scout keyed on A9:239 and A9:606. |
| L-R08 | records/A9.md:606 | K-unreachable | K-unreachable, K-duplicate | Its verdict also removes it because `lens()` reads `search_depth_mode()`, which test 12 already covers (question 10 names tests 12 and 34). |
| L-R08 | records/B1.md:87 | P-message | P-message, K-duplicate | Its question 10 says test 6 "already runs both tools with a bogus flag and compares with the owner. Only 291 (nothing created) is new", and the verdict folds it into test 6. |
| L-R08 | records/B1.md:519 | P-setting | P-setting, S-snapshot, Z-cite | Its question 3 and 5 say the Acid Romance counts 418 and 37 were copied by hand from test 24's snapshot and "pinned from one run (comment at 303 says so)". |
| L-R08 | records/B1.md:682 | S-snapshot | S-snapshot, Z-cite | Its question 5 says "pinned from one run (comment at 434)", and the test's own name says "pinned". |
| L-R08 | records/B2.md:23 | P-builtchart | P-builtchart, K-comment | Its question 4 says the deadline 0.0 is a raw result and "the working ... lives in the comment". |
| L-R08 | records/B2.md:147 | P-builtchart | P-builtchart, K-comment | Its question 4 says the three raw ticks have "the working in the comment". |
| L-R08 | records/B2.md:159 | P-builtchart | P-builtchart, K-comment | Its question 4 says "the comment holds the grid (1680 ticks per 7/8 measure)". |
| L-R08 | records/B2.md:201 | S-snapshot | S-snapshot, Z-cite | Its question 5 quotes the file's comment "Pinned from one run of the code". |
| L-R09 | records/B3.md:139 | K-floor, P-other | K-floor, P-other, K-nocode | The test walks source files for header names and pins the text of two declarations (record questions 1 and 4), which is the source-text check K-nocode names. |
| L-R09 | records/B3.md:155 | P-builtchart, K-selfmade | P-builtchart, K-selfmade, K-comment | Its question 4 says the typed results have "the working in the comment". |
| L-R09 | records/B3.md:219 | P-builtchart, K-selfmade | P-builtchart, K-selfmade, K-comment | Its question 4 says 141607.57 is "a result with working in the comment". |
| L-R09 | records/B3.md:315 | P-builtchart, P-other | P-builtchart, P-other, K-comment | Its question 5 says -9.48 is worked "in comment". |
| L-R09 | records/B3.md:331 | P-builtchart | P-builtchart, K-comment | Its question 5 says "raw results; the working is in the comment". |
| L-R09 | records/B3.md:347 | P-builtchart | P-builtchart, K-comment | Its question 5 says the formula result is worked "by hand (comment 777-781)". |
| L-R09 | records/B4.md:90 | S-snapshot, P-builtchart, K-cantfail, K-skip | S-snapshot, P-builtchart, K-cantfail, K-skip, Z-cite | Its question 5 says the aims are "pinned" in the test's own comment (965-967, "pinned before F1"). |
| L-R09 | records/B4.md:126 | P-builtchart, K-nocode | P-builtchart, K-nocode, K-comment | Its question 4 says the figures are raw results with "the working only in the comment". |
| L-R09 | records/B5.md:28 | F-result, S-snapshot, K-comment | F-result, S-snapshot, K-comment, K-unreachable | Its question 11 says leeway -5 and 250 "cannot occur in a real run". |
| L-R09 | records/B7.md:179 | P-other, K-comment | P-other, K-comment, S-snapshot | Its question 9 says "typed 4-decimal floats are snapshots of one run". |
| L-R09 | records/C1.md:104 | P-other | P-other, Z-cite | Its question 4 says 34.0 is commented "a literal from one run". |
| L-R09 | records/C1.md:246 | K-selfmade | K-selfmade, K-unreachable | Its question 11 says "row width 470 cannot happen (below kMinPathDetailsW - 16)". |
| L-R09 | records/C1.md:273 | K-partial | K-partial, K-unreachable | Its question 11 says "470 cannot happen". |
| L-R09 | records/C1.md:338 | K-cantfail | K-cantfail, K-namebody | The name says every row of a path shares one x, but its question 2 says the test hands both calls the same measure width "so it never exercises the cross-row part". |
| L-R09 | records/C1.md:573 | P-setting, K-selfmade | K-floor, K-selfmade | The fault is the cap 8 "typed ... with no reason" (rubric question 5); it is no settings default or screen word, so P-setting does not fit. |
| L-R09 | records/C1.md:598 | K-selfmade | K-selfmade, K-cantfail | Its question 8 says `viewed.ready()` "cannot tell old from new", so the check passes with the old answer. |
| L-R09 | records/C1.md:706 | P-setting | K-floor | Same fault as C1.md:573: "8 is typed as the new cap with no reason". |
| L-R09 | records/C10.md:13 | P-other, Z-cite | P-other, Z-cite, S-snapshot | Its question 8 calls the typed JSON values and the key list "a snapshot" ("any change at all"). |
| L-R09 | records/C10.md:351 | P-other | P-other, Z-cite | Its question 4 says the comment says the row count is "read from the source file", which it is not; that is a comment naming a source. |

That is 9 L-R08 rows and 19 L-R09 rows (L-R09: B3 six, B4 two, B5 one, B7 one, C1 seven, C10 two). C1.md:573 and C1.md:706 swap a key; all other changes add keys.

## Ends tables

Every row named in both ends tables is in its batch and carries the key. One effect of my changes: L-R09's ends row for P-setting named C1.md:706 and C1.md:573, and after my change no L-R09 row carries P-setting, so that ends row goes away. K-floor in L-R09 then has C1.md:1038 and B3.md:139 as before, and could also show C1.md:573 as its least fitting row. I did not edit the scout's file.

## Side notes

- K-namebody is used for a name narrower than its body, not only for "a name promising a check the body never makes", which is how keys.md defines it. Rows: A9.md:526 (name says "maps the cap", body checks four things), B1.md:19 (name names only the stamp), B2.md:719 (name says "best path", helper checks every kept path). I left them as the scout labelled them; the main session should decide whether the key covers that reading. If it does not, those three rows have a named fault with no key.
- The fault "claim not checked on real input" (no real settings file, hydra.db keys, screen or Preview capture) is the record's named fault on most Rewrite rows in both batches, and keys.md has no key for it. It is why the nine A9 `no key` rows have no key; the same fault is simply not labelled on the many other rows that carry a real fault key.
- B5.md:28 and B5.md:262 record the disc check as "true by construction" and the scout used K-selfmade in places and K-cantfail in others (B5.md:213 has K-cantfail, B5.md:159 and B5.md:262 only K-selfmade). Both keys fit; I left each row as it was.
- records/B2.md:211 cites `docs/agents/rulings/code-and-engine.md` line 50 as the user's words, and records/B6.md:1094 quotes it ("we don't need golden anymore, remove it all"). I did not open that file to check the line is under `## Confirmed`.
- records/B3.md:360 and `tests/test_replay_schedule.cpp:774`: the comment "No chart in the corpus has a timed squeeze-in" is wrong by the record's own reading (Pitch Dark and `late_sqin_twice` have them). B3.md:274 says the comment's 24.82 lead disagrees with the 21.825 the split gives. Both are stale comments, not in my keys.
- records/B7.md:183 and :196: `ms_per_measure_at` is called "display-layer only" in `timing.h:291-292` but `src/core/squeeze_rating.cpp:15-18` calls it. A split noted in the record; not a key.
- records/B7.md:80 and :368: the "beat shorter than a tick" refusal is written twice (`apply_timesig` and `MeasureIndex`). A split in the record (B7:106 is Investigate, not in this batch).
- records/C1.md:490-505 (not in these batches): the window-tag colours test has a split with an `## Unconfirmed` ruling line; not a row here.
- The batch tsv for B2.md rows 23 to 829 points at "Read:" lines inside code fences, not headings; the heading for row N sits two lines above. This is why a scout reading by heading line can miss a row, but I found no row missed in the label files.

## Not checked

Nothing. I opened a record at each row of both batches. I read A9, B1, B2, B3, B4, B5, B7 whole or in ranges that cover every row, and read B6, C1 and C10 at the rows in the batch plus the range around each (for the B6 lane-group rows 466 to 636 and the .chart rows 725 to 817, which are not in the batch, I did not read them). I did not open any test source file, so a fault only the test shows is not covered.
