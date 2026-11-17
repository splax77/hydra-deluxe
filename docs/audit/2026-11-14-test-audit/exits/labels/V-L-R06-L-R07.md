Agent id: a932d1b09621275ce (task T8-E3-V-L-R06-L-R07, session f3bbb32b)

I checked the scouts' ids: L-R06.md was labelled by a5d47a0382cd79df1 and L-R07.md by a9b30b2d9029bd881. Neither is mine.

## Counts

- L-R06: 90 rows in the batch, 90 lines in the label file (each batch row exactly once, no extra, no duplicate), I agree with 86 rows, I change 4.
- L-R07: 89 rows in the batch, 89 lines in the label file (each batch row exactly once, no extra, no duplicate), I agree with 82 rows, I change 7.

The ends tables of both files are sound. Every plainest and least-fitting row named is in its batch and carries that key, and every key used in a table has an ends line. My changes do not move any end row.

I opened the record at the line in `row` for every row of both batches. Nothing is left unchecked.

## Rows I change

| batch | row | scout's keys | your keys | why |
|---|---|---|---|---|
| L-R06 | records/A35.md:208 | P-builtchart | P-builtchart, S-snapshot | The record says 49.479 "has no source beyond one run", a pinned printed number. |
| L-R06 | records/A36.md:553 | K-comment | K-comment, K-unreachable | The record says the no-timing scene "cannot be hit from the app" and is "not reachable from the Preview". |
| L-R06 | records/A36.md:773 | K-skip, K-partial | K-skip, K-partial, K-floor | The record names lines 1046-1047 as floors ("1047 is a floor standing for paths were walked"). |
| L-R06 | records/A39.md:369 | S-snapshot, Z-cite | S-snapshot, Z-cite, P-burnout | It types Burnout's score 378,315, path "3- 1 2" and 163.0 ms; P-burnout is the key for exactly this, and the R07 scouts used it for the same figures. |
| L-R07 | records/A5.md:405 | P-other | P-other, K-comment | The record says "the sum is only in a comment" (5321 = 4321 + pad), a raw result whose meaning lives in a comment. |
| L-R07 | records/A5.md:421 | P-other | P-other, K-comment | The record says 1100 = 100 (last note, "only in a comment") + 1000. |
| L-R07 | records/A6.md:40 | K-namebody, P-other | P-other | The name says "leaves the query empty" and line 69 checks that, so the name keeps its promise. The proxy the record names (rows shown standing for the button's wording) fits no key; see side notes. |
| L-R07 | records/A7.md:478 | P-other, K-selfmade | P-other, K-selfmade, K-comment | The record says the rectangle "is a raw result; its working is in the comment". |
| L-R07 | records/A7.md:496 | P-other | P-other, K-comment | The record says "raw rectangles; comment gives the working". |
| L-R07 | records/A7.md:513 | P-other | P-other, K-comment | The record says "corner rectangle raw; working in comment 167-169". |
| L-R07 | records/A7.md:616 | P-other, K-selfmade | P-other, K-selfmade, K-comment | The record says "raw rectangles with a long working in comments". |

## The four `no key` rows

All four stay `no key`. I tried the whole list against each record.

- records/A39.md:149: the test sets the button's two flags by hand and reads `active()` and `playing()` as a stand-in for the audio device. No key names a hand-copied button body or a flag standing for a device. K-selfmade and K-nocode do not fit, because the test checks real Hydra code.
- records/A4.md:145: a wait with no cap, so a stuck pool hangs the suite instead of failing. K-cantfail is the nearest key, but it names a check that cannot fail while passing. This test never passes when stuck, it never returns. K-skip does not fit either. No key.
- records/A46.md:150: counts vertices of two chips as a proxy for "an outline is drawn". The test can fail, types nothing and fakes nothing, so no key fits.
- records/A7.md:420: reads the `report_started` flag where the claim is about the strip's drawn words. The record names it a stand-in (Q6). No key fits.

## Side notes

- Four of the `no key` or proxy faults share one shape: a field or a count stands for what the claim says is on screen. They are records/A39.md:149, A46.md:150, A7.md:420 and A6.md:40 (rows shown standing for the button's wording). Together they could be one question of their own, since the key list is closed after revision 1.
- records/A6.md:63 and :84 (rows 3 and 4, not in these batches): a split between D48 Q16 ("it shows the new result straight away") and the tests, which pin that the open panel keeps its result. Rows 6 to 10 of A6 (all in L-R07) rest on the same commit.
- records/A6.md:207 (row 11, not in the batch): the test comment quotes "the user's wording" and the record found no such quote. A6 rows 6 to 13 in L-R07 cite the same hold-the-order rule.
- records/A4.md:404 and :425: comments cite "the user's rule (2026-10-15)" and "user answers 1 and 6 of 2026-10-15" and the record found neither. This touches A4 rows 25 and 26 (L-R06).
- records/A8.md:102 and :120: a split inside the settings text. The confirm's Fills line says "Clone Hero 1.0/1.1" and the settings button says "1.0 fills" for the same setting. Rows 5 and 6 are not in the batch.
- records/A8.md:438: the rule "lost pixels go to the last column" has no decision behind it (D245 settles only "re-split"). Row 23 is in L-R07.
- records/A36.md:229 to :232: the screen reads 0.026 ms past the engine's SP end on Pitch Dark. Row 5 is not in the batch.
- records/A36.md:547 to :549 (row 14, in L-R06): the record raises a possible derive-once case. The display works out which chord is inside an SP phrase by time interval, and it did not search `src/core` for an engine per-chord flag.
- records/A44.md:221 and records/A45.md:200 and :251: the splits the scouts listed are real as read (A44 row 11, Unconfirmed lines on both sides; A45 row 12, "one second after" against D255). Nothing to add.
- records/A39.md:251 and :263: "remembered hidden" for Hide library has no decision. Row test_panel_hide_library is not in the batch.
- records/A47.md:174 (row in L-R07): the fixture stands in for AppState. I kept the scout's K-cantfail. C-rig might also fit, but the key list does not say whether a test fixture is a "rig", so I left it.
