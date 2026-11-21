Agent id: aaa7cf8a265a49201 (task T8-E3-V-L-R19, session f3bbb32b)

## Counts

L-R19: rows in the batch 92, lines in the label file 92 (one per row, no duplicates, none missing), rows I agree with 48, rows I change 44.

The ends table is sound for the scout's own keys: all 14 keys used appear in it, and every row it names is in the batch and carries that key. After my changes the P-other least-fitting row (`records/window-WN4b.md:362`) no longer carries P-other, and the K-duplicate and P-burnout rows still do.

I opened every record at its line (delta-DL5b, WN1, WN2, WN3, WN4a, WN4b, WN4c, WN5). Not checked: none.

## Rows I change

Three reasons repeat, so I give them once here.

- P-builtchart. `cross-findings.md:195` lists "Allies' best-path ticks typed as pins in test_path_costs.cpp" under P-builtchart, so the key covers typed engine answers (totals, ticks, path strings) on a named corpus or fixture chart, not only on a chart the test wrote itself. The scout used it that way for WN1 W3 to W9 and P-other for the same kind of value elsewhere. I made them agree.
- Z-cite. The key is a comment that names where the expected value came from. The records quote such comments ("pinned from one run", "Literals from T0's run") in many rows. The scout used Z-cite once. I added it only where the record quotes the comment.
- P-allister. The Allister "Overrated" offered-fill ticks are the key's own example.

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-R19 | records/delta-DL5b.md:171 | C-ownchecker | C-ownchecker, P-setting, P-message | The record says lines 499 and 503 type the default of `mslimit_value` and the checker's printed sentences, and its verdict rebuilds both from the owner's values. |
| L-R19 | records/window-WN1.md:133 | P-other | P-other, Z-cite | The record says line 609 stands on the comment "pinned from one run" (line 607). |
| L-R19 | records/window-WN1.md:149 | P-other | P-other, S-snapshot, Z-cite | The record calls the literal at 732 a snapshot "from one run (comment, line 730)". |
| L-R19 | records/window-WN1.md:331 | P-builtchart | P-builtchart, Z-cite | The record quotes the comment at line 95, "pinned as read from one run". |
| L-R19 | records/window-WN1.md:411 | P-other | P-builtchart, Z-cite | Typed ticks 49920 and 22272 are Allies' best-path ticks, the key's own example, "pinned as read from one run" (line 369). |
| L-R19 | records/window-WN2.md:240 | K-duplicate, P-burnout | K-duplicate, P-burnout, P-builtchart | The record also types Pitch Dark's path "3+ 2+ 1" and its ticks (128520, 48,783.075 ms), which are not Burnout figures. |
| L-R19 | records/window-WN2.md:307 | P-other | P-builtchart | The record names 5472, 13920 and 11616 as activation and end ticks from a run on a fixture chart. |
| L-R19 | records/window-WN2.md:324 | P-other | P-builtchart | The record names 8832 and 15648 as engine results on `node_before_phrase.chart`. |
| L-R19 | records/window-WN2.md:332 | P-other | P-builtchart, K-skip | The record names 10176, 13248 and 16224 as engine results and says the loop `continue`s silently when `window_at` finds nothing (line 307). |
| L-R19 | records/window-WN3.md:71 | K-duplicate | K-duplicate, K-floor | The scout's own note says `paths > 1` is a count the window can move, and the record names line 242 as the one floor. |
| L-R19 | records/window-WN3.md:140 | P-other, K-skip | P-builtchart, K-skip, Z-cite | Typed best path "3" and ticks on Hail The Sun are engine answers on a named chart, "pinned from one run" (comment 350). |
| L-R19 | records/window-WN3.md:191 | P-other | P-allister, Z-cite | The eight typed ticks are Allister - Overrated's offered fills, "typed from one run, comment 446". |
| L-R19 | records/window-WN3.md:260 | P-other | P-builtchart, Z-cite | Typed 233100, 150720, 82560 and the leftovers are engine answers on Don Broco - Actors, "typed from one run (audit finding 89, comment 597)". |
| L-R19 | records/window-WN3.md:292 | P-other, S-snapshot | P-builtchart, S-snapshot, Z-cite | The typed lists are Allies' best-path ticks and totals, the key's own example, "pinned from one run" (comment 74). |
| L-R19 | records/window-WN3.md:309 | P-other | P-builtchart | Typed 706180 and 5 are the engine's answer on White Washed in `test_path_costs.cpp`, the file the key's example names. |
| L-R19 | records/window-WN3.md:326 | K-cantfail, P-other | K-cantfail, P-builtchart, Z-cite | 407610 is an Allies engine total "pinned from one run". |
| L-R19 | records/window-WN3.md:343 | P-other | P-builtchart | Typed 451830 is an Allies engine total in `test_path_costs.cpp`. |
| L-R19 | records/window-WN4a.md:222 | K-floor | K-floor, Z-cite | The record quotes the file's comment: `min_paths` is "a floor pinned from one run". |
| L-R19 | records/window-WN4a.md:260 | K-floor, P-other | K-floor, P-other, S-snapshot | The record says the typed skip counts are, at 60 ms, "a snapshot". |
| L-R19 | records/window-WN4a.md:298 | S-snapshot | S-snapshot, P-builtchart | 228710 and "1 Allister best path" are the engine's answer on a named chart, typed twice (the record's Split). |
| L-R19 | records/window-WN4a.md:336 | S-snapshot, P-other | S-snapshot, P-builtchart, Z-cite | The typed figures are Pitch Dark engine values, and the file calls them "pinned from one run" (line 1437). |
| L-R19 | records/window-WN4a.md:355 | P-other | P-builtchart | 157545.2 and 158350.6 are Pitch Dark engine values "from the live run". |
| L-R19 | records/window-WN4a.md:393 | P-other, S-snapshot | P-other, S-snapshot, Z-cite | The record quotes the comment "pinned from one run" (896-897). |
| L-R19 | records/window-WN4a.md:412 | P-other, S-snapshot | P-builtchart, S-snapshot, Z-cite | Typed "3", 123838, 244800, 281759 are the same Hail The Sun engine answers as WN3:140, "pinned from one run" (943). |
| L-R19 | records/window-WN4a.md:431 | no key | K-floor | Line 263 `CHECK_FALSE(activations.empty())` is an existence check on search output that fails with no fault when the window moves, as the scout labelled WN3:88; item 9 calls it a stand-in. |
| L-R19 | records/window-WN4a.md:450 | P-other | P-other, S-snapshot | The old verdict calls the typed `{2, 7}` "a snapshot of 2 and 7", the same value the scout marked S-snapshot at :393. |
| L-R19 | records/window-WN4b.md:118 | P-other, S-snapshot | P-burnout, S-snapshot, Z-cite | The typed 13640, 364675, 378315 and "9 1" are Burnout's best path and score, "from one run (65 comment)". |
| L-R19 | records/window-WN4b.md:172 | P-message, P-other | P-message, P-burnout, Z-cite | Tick 36480 is a Burnout fill tick, "From one run only". |
| L-R19 | records/window-WN4b.md:189 | P-other, P-message | P-builtchart, P-message | Typed 257280 and "0 1 3 0+" are White Washed engine answers. |
| L-R19 | records/window-WN4b.md:223 | S-snapshot, P-other | S-snapshot, P-builtchart | Typed ticks, strings and finishes on White Washed are engine answers on a named chart. |
| L-R19 | records/window-WN4b.md:240 | S-snapshot, P-other | P-burnout, S-snapshot | Typed 334495 and "3-" are Burnout's first-window score and path. |
| L-R19 | records/window-WN4b.md:257 | S-snapshot | S-snapshot, P-burnout | Typed "3- 1" and 358235 are Burnout's best path and total. |
| L-R19 | records/window-WN4b.md:277 | K-duplicate | K-duplicate, P-burnout | Line 464 types Burnout's path "9 1". |
| L-R19 | records/window-WN4b.md:294 | P-other | P-builtchart | The typed table of offered fill ticks is an engine answer on White Washed. |
| L-R19 | records/window-WN4b.md:311 | P-other | P-builtchart | Typed 199680 and 165120 are fill ticks of one White Washed run. |
| L-R19 | records/window-WN4b.md:328 | P-other | P-builtchart | The typed six-tick list is an engine answer on White Washed. |
| L-R19 | records/window-WN4b.md:362 | P-other | P-allister | The eight real ticks are Allister - Overrated's offered fills typed at line 118. |
| L-R19 | records/window-WN4b.md:447 | P-message, P-other | P-message, P-builtchart | Typed 226,210, 228,710 and 2,500 are engine scores on Allister "Overrated". |
| L-R19 | records/window-WN4c.md:429 | P-other | P-message, P-other | The record calls the typed line "a report line from the tool", which is the widened P-message. |
| L-R19 | records/window-WN5.md:152 | P-burnout, S-snapshot | P-burnout, S-snapshot, Z-cite | The record quotes the test's comment (line 1062) "read off one run of this test". |
| L-R19 | records/window-WN5.md:187 | no key | K-floor | Line 516 `IM_CHECK(active_ms >= 0.0)` needs chart 0's best path to have an activation, the same existence check as WN4a:431; the alternative is `no key`. |
| L-R19 | records/window-WN5.md:381 | P-burnout | P-burnout, P-other, Z-cite | The record types the song length 130087.88 as a literal "one logged run", a value `song_length_ms` owns. |
| L-R19 | records/window-WN5.md:398 | P-other | P-other, Z-cite | The record quotes the comment "Literals from T0's run" (line 971). |
| L-R19 | records/window-WN5.md:415 | P-other | P-other, Z-cite | The record quotes the same comment, "Literals from T0's run". |

## Side notes

- `records/window-WN4a.md:431` and `records/window-WN5.md:187`: I gave both K-floor because the scout gave K-floor to the same pattern at `records/window-WN3.md:88` and `:123` (an existence check that fails with no engine fault). If the main session thinks that is a stretch, both are `no key` and form one question of their own.
- The scout read P-builtchart narrowly (charts the test wrote) in WN1 and P-other for the same values elsewhere. My reading follows `cross-findings.md:195`. If the main session reads the key narrowly, the 20-odd P-builtchart changes above fall back to P-other, and the other keys I added are unaffected.
- The scout's side note that `records/window-WN2.md:307`, `:324`, `:332` and `:340` hold only a verdict line is wrong: each has a claim line and a "Real chart" line naming the typed ticks (`records/window-WN2.md:307-345`).
- Z-cite is probably under-used outside this batch too: comments that say "pinned from one run" appear in most window-round records.
- `records/window-WN3.md:140` and `records/window-WN4a.md:412` type the same Hail The Sun values (123838, 244800, 281759) in two files; the record at `records/window-WN4a.md:524` names the same kind of repeat for `{2, 7}`, so these rows could share one question.
- `records/window-WN4a.md:527` records a possible split in the user's words (`code-and-engine.md` line 55, "use the hit window") against a typed 85.0; I did not judge it.
