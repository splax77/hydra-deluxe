# Handoff: the early fill live check

Written 2026-11-12 by session 375e58cd. For a future session to plan and run. Nothing here has started.

## The goal

Find out, in a real run of Clone Hero 1.1, whether hitting the note that makes Star Power ready early can make a fill appear that would otherwise be just too late.

Hydra's whole "early fill" idea rests on that, and nobody has ever checked it in the game.

## What is known and what is not

The game's fill deadline is proven in its code (FL-19 on `docs/clone-hero/04-fills.md`). When a hit is about to bring the Star Power meter to half, the game removes every fill that starts within 4 beats of the current tick. The check is an exact tick compare with no millisecond allowance.

Hydra adds its own model on top (FL-24, labelled "Hydra assumes it, never checked"). It assumes the deadline is judged at the moment the player hits the SP-ready note, so an early hit makes Star Power ready earlier and can beat the deadline. Hydra allows up to 85 ms of that (`fill_refuses`, reach `kHitWindowMs`), and badges an activation E0 when it has less than 60 ms of room (`is_e0`, `kEarlyFillSlackMs`). The listing of the game's deadline method does not settle the question: it compares against a "current tick", and the reference does not say whether that tick follows the press or the note's place in the chart.

One live result points the other way (FL-25). On the probe chart Trippin SP Spike, the game never offered Hydra's path 3 E0 fill at tick 34944 in three bot runs; it activated at the next fill, tick 38016, each time. The bot hit notes a median 0.42 ms early, so those runs neither confirm nor rule out the early-hit model. Why the fill was not offered is not known.

The live-checks page already lists both rows. FL-24 asks how far early a hit can be and still summon the fill when Star Power comes ready just after the deadline (pass: the fill is offered up to 85 ms). FL-25 asks why Trippin path 3's fill was not offered (pass: a repeat with the fill watched shows the flag cleared and why).

## Why it matters now

The Dynamic Hit Window plan (`docs/superpowers/plans/2026-11-12-dynamic-hit-window.md`, merged to main) adds a row under the "Early fill" line that shows the game's window for the SP-ready note and how much room it leaves. Its question 1 asks whether that row should ship while FL-24 is unproven. The user asked what is unchecked about early fills, was told the above, and then asked for this handoff. Whether the early fill row waits for this check is the user's call on question 1; at the time of writing it was not yet answered.

## What was done

Nothing has been run. This session read FL-19, FL-24 and FL-25, the two live-check rows, and Hydra's early fill code, and explained the gap to the user.

## What is next

1. Read `docs/clone-hero/README.md`, `docs/agents/clone-hero-mcp.md` and `docs/agents/premise-map.md` before planning, as CLAUDE.md requires for anything that touches the game.
2. Plan one small live check, with a Premise map and a call list per live step. Live runs need the user's permission, and runs must not need the game window focused (the user's rulings in `docs/agents/rulings/clone-hero.md`).
3. The check needs a chart where Star Power comes ready a known number of ms after a fill's deadline, and a way to press the SP-ready note early by a chosen amount. Kit runs are now timed by the game itself (IG6a, D343), which may give that control; check it in the code first.
4. Pass and fail come from the FL-24 row on `docs/clone-hero/live-checks.md`. A FL-25 repeat with the fill watched could share the run.
5. Whatever the run shows goes on `docs/clone-hero/04-fills.md` (FL-24's label and evidence) in the same commit as the work, per the write-back rule.

## The user's words

Session 375e58cd, 2026-11-12: "what is unchecked about early fills?" and then "create a handoff for that and proceed with the plan for the nfew stuff".

## Claims

| Kind | Value | Claim | How to check |
|---|---|---|---|
| path | `docs/clone-hero/04-fills.md` | holds FL-19 (the deadline, proven in code), FL-24 (Hydra's early fill model, "Hydra assumes it, never checked") and FL-25 (Trippin path 3's fill not offered, proven in a live run) | the checker finds the file; read the three blocks |
| path | `docs/clone-hero/live-checks.md` | lists the FL-24 and FL-25 live checks with their pass lines | the checker finds the file; grep `FL-24` |
| path | `docs/superpowers/plans/2026-11-12-dynamic-hit-window.md` | the Dynamic Hit Window plan; its question 1 asks whether the early fill row ships while FL-24 is unproven | the checker finds the file; read its Questions section |
| path | `tools/ch_probe/experiments/results/overlay-spike/run1_cal0_sp_compare.txt` | one of FL-25's three result files; line 34 says activation 3 was not on the path's chord | the checker finds the file |
| path | `C:\Clone Hero\songs\Hydra Probe\Trippin SP Spike` | the probe chart FL-25 was run on | the folder exists |
| path | `docs/agents/clone-hero-mcp.md` | how live runs of the game are driven | the checker finds the file |
| function | `fill_refuses()` | refuses a fill when Star Power comes ready more than `kHitWindowMs` (85 ms) after the deadline; in `src/core/model.h` | the checker finds it |
| function | `is_e0()` | an E0 has less than `kEarlyFillSlackMs` (60 ms) to spare and no skipped fill; in `src/core/model.h` | the checker finds it |
| function | `activation_fill_deadline_ms()` | Hydra's copy of the deadline: fill start minus 4 beats; in `src/search/graph.cpp` | the checker finds it |
| commit | `bc33ea86abf6a0b549307a76b32fbbc935ce2ce4` | the merge that put the Dynamic Hit Window plan and mockup on `main`; on `main`, not pushed when this was written | the checker finds it on main |
| commit | `8901af45ae41f40094e03f309f937ad99d943da5` | the IG6a merge: every kit run is timed by the game (D343); on `main`, not pushed when this was written | the checker finds it on main |
| ruling | `375e58cd` | "create a handoff for that and proceed with the plan for the nfew stuff" | this session's card, once written, holds the words |
| state | question 1 of the Dynamic Hit Window plan was unanswered when this was written | the fact | ask the user, or read this session's card |
| state | no live run for FL-24 has been made | the fact | FL-24's label on `docs/clone-hero/04-fills.md` is still "Hydra assumes it, never checked" |

## Checker output

```text
ok | path | docs/clone-hero/04-fills.md | holds FL-19 (the deadline, proven in code), FL-24 (Hydra's early fill model, "Hydra assumes it, never checked") and FL-25 (Trippin path 3's fill not offered, proven in a live run)
ok | path | docs/clone-hero/live-checks.md | lists the FL-24 and FL-25 live checks with their pass lines
ok | path | docs/superpowers/plans/2026-11-12-dynamic-hit-window.md | the Dynamic Hit Window plan; its question 1 asks whether the early fill row ships while FL-24 is unproven
ok | path | tools/ch_probe/experiments/results/overlay-spike/run1_cal0_sp_compare.txt | one of FL-25's three result files; line 34 says activation 3 was not on the path's chord
ok | path | C:\Clone Hero\songs\Hydra Probe\Trippin SP Spike | the probe chart FL-25 was run on
ok | path | docs/agents/clone-hero-mcp.md | how live runs of the game are driven
ok | function | fill_refuses() | refuses a fill when Star Power comes ready more than `kHitWindowMs` (85 ms) after the deadline; in `src/core/model.h`
ok | function | is_e0() | an E0 has less than `kEarlyFillSlackMs` (60 ms) to spare and no skipped fill; in `src/core/model.h`
ok | function | activation_fill_deadline_ms() | Hydra's copy of the deadline: fill start minus 4 beats; in `src/search/graph.cpp`
not-pushed | commit | bc33ea86abf6a0b549307a76b32fbbc935ce2ce4 | the merge that put the Dynamic Hit Window plan and mockup on `main`; on `main`, not pushed when this was written
not-pushed | commit | 8901af45ae41f40094e03f309f937ad99d943da5 | the IG6a merge: every kit run is timed by the game (D343); on `main`, not pushed when this was written
manual | ruling | 375e58cd | "create a handoff for that and proceed with the plan for the nfew stuff"
manual | state | question 1 of the Dynamic Hit Window plan was unanswered when this was written | the fact
manual | state | no live run for FL-24 has been made | the fact
checked 14 rows: 9 ok, 0 missing, 0 not-on-main, 2 not-pushed, 3 manual
```
