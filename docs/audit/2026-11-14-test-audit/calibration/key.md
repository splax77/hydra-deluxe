# T2 calibration key (never in the repo, never in a brief until T3 is scored)

Written 2026-11-15 by the main session, before any calibration agent exists.
File: tests/test_preview_boxes.cpp at main a19b9cb2 (the plan's fallback; see below).

## Why not tests/test_preview_scene.cpp

Read whole. No test there has a claim that is the fault. The fill states come
from the engine's lists (src/app/preview_view.cpp 465-495: skipped_fill_ticks,
the activation's own fill, trailing_skipped_fill_ticks). The SP end is
Activation::window_end_tick. The solo bonus is Path::score_solo. It does hold
checks that are the fault (ghost_fills == {2, 7} "pinned from one run" at 903;
the "157922:H 180960:H 244800:O 281759:O " string at 953; the silent skips and
floors at 749/774-775 and 973/991), but the plan needs both kinds in one file.

## Claim is the fault

K1. "score box: the multiplier is the replay's, doubled on chords Star Power pays" (line 332).
  Fails Q2 and Q3. Lines 354-357 and 366 say the box reads the plain x2 from
  6500 ms on, though the chord at 6500 ms was paid by SP and the replay's
  multiplier_shown for it ("What the game's disc shows once this chord is hit",
  src/core/replay.h, doubled when in_sp) is doubled. So the title's claim and its
  checks disagree. The box works the disc out itself: build_score_box,
  src/app/preview_view.cpp 631-635, shows at.multiplier only while
  running_activation(scene, now) is set, else at.multiplier_plain. Two owners for
  "what the disc shows at this moment": the replay's shown_multiplier and the box.
  Engine record: pitch_dark_ratio.schedule.json, chord 676, tick 206280,
  ms 164341.3, ends_sp true, paid_by activation 1 (act_tick 178560). The replay
  pays it doubled; the box's rule shows plain from 164341.3 ms.
  D11 (docs/audit/2026-10-03-fix-decisions.md line 29) is a decision for the box's
  side; Q3 says a decision does not answer it. Expected verdict: Investigate
  (decision vs record), with a production-code finding against build_score_box.

K2. "score box: a leeway chord past the SP end is paid doubled, the box stays plain" (line 401).
  Fails Q3, same rule as K1 (Q10: one verdict). Lines 434-437 confirm the replay
  pays the chord doubled (in_sp, multiplier_shown doubled); lines 440-442 require
  the box to read plain at that chord's own instant. The claim rests on D11 alone.
  Engine record: as K1 (pitch_dark_ratio chord 676 is the on-the-end case; no
  dumped chart was searched for a chord inside the 3 ms leeway).
  Expected verdict: Investigate.

## Checks are the fault

K3. "score box: a solo's bonus lands on its last note" (line 280).
  Fails Q4/Q5 at line 294: CHECK(scene.score.steps[1].total == 180), "pinned from
  one run as a fixed oracle" (its own comment, 291-292). A snapshot. Lines 302-303
  and 307-308 also type the box's text ("x1 · combo 2") rather than reading the
  replay row (Q6).

K4. "score box stars: before the first chord no star is earned" (line 485).
  Fails Q4/Q5 at line 495: "star 1 in 20", "Pinned from one run" (494). A snapshot
  beside the line above it (493) that already takes it from star_cutoffs.

K5. "score box stars: the analyzed chart ends on the path's own star count" (line 498).
  Fails Q4/Q5/Q9 at line 509: end.stars == 7, "Pinned from one run" (507-508), on
  the corpus chart corpus::analyzed_by_ten_scores() picks, so it leans on which
  charts are in testdata/input.

K6. "score box stars: a solo's bonus raises the score but not the stars" (line 513).
  Fails Q4/Q5: "The literals below were pinned from one run" (516). Lines 527,
  529, 535-537, 546, 548-549 type "180", "star 2 in 10", "480", 2, "star 3 in 100",
  "580", 3, "star 4 in 380". Line 540-541 is a good property; line 547 compares
  with path_stars, which is fine.

## Scoring (T3)

Pass: the agent reports K1 to K6 as not Keep, and reports K1 and K2 as failing
Q2 or Q3 (the claim), not only Q4. A miss of any one stops the audit (T3).
Key count: 6.
