# Review catches borrowed values: closing the three ways the replay schedule plan kept Star Power rules outside Hydra

Task PL-REVIEW-GAP, session bed936f0-797e-4a36-bbbe-07673ff356e4. Written 2026-11-12. This plan changes how plans are written and how changes are reviewed. It changes no Hydra code a player runs. The fix to the schedule code itself is a separate plan and is not part of this one.

## The plain version

On 2026-11-08 you told the planner of the replay schedule plan: "the whole point of this plan is to hand off all the starpower planning to hydra which already knows what to do". The plan said it did that. Two Star Power rules stayed outside Hydra anyway, and three reviews read that exact code without seeing them.

There were three ways out, and each one is a gap in the written method, not one agent's slip.

The first is a borrowed value. The schedule tool took a ratio from the engine, so a reviewer asking "does this come from the engine?" got yes. But the engine had worked that ratio out for a different Star Power end than the one the tool applied it to. The review method only asks whether a fact is derived twice. It never asks whether a value read from the owner is the owner's value for this exact thing.

The second is a rule hidden in arithmetic. The Python player moves every later press by the read end minus the planned end, one for one. That subtraction is a Star Power rule: it says a later end moves exactly as far as an earlier one. Nobody ruled on it. The review page files "a rule with no user decision" under kind D, then tells the reviewer to quote the precheck (the script that scans a change before review) for kind D and not look again. The precheck's kind D check finds new numbers. A subtraction has no number in it.

The third is a deferred premise. The planner saw the risk and wrote it down as an unchecked fact, to be settled by a live run. The tasks that rested on it were built before that run. Part of the fact could have been settled the same day by reading Hydra's own code.

This plan closes each one with a rule on the page that owns it, and proves the new rules by replaying the old reviews. Three fresh readers get the old plan and the old code ranges with the new pages and no hint. They must flag both items, or the wording is not good enough.

Three numbers to hold on to. The schedule planned the end read after chord 466 at 158,254.77 ms. The game stored 158,268.99 ms. That is 14.21 ms apart. The press log shows the anchor press itself landed about 1.8 ms later than its aim, so that accounts for a small part of it.

## What happened, checked against the repo

Every line here was read today in the worktree or in the run folder. The Premise map at the end names each source.

**The borrowed value.** In `tools/replay_schedule.cpp`, lines 661 to 662 pick the ratio for a squeeze-in from `SPSqueeze::transfer`. Line 692 keeps the first squeeze's ratio as the anchor chord's ratio. Line 756 then uses that ratio to price the end the player reads after the anchor chord. (The brief said lines 692 and 756; the first pick is at 661.) The engine stamps `SPSqueeze::transfer` in `frontend_transfer_scales` (`src/core/squeeze_rating.cpp`, lines 34 to 42), and it measures it at that squeeze's own end.

On Pitch Dark these are two different ends. The schedule file from the 2026-11-12 run shows activation 1 with four end steps. After chord 466 the end is at tick 198570 (158,224.76 ms). The squeeze-in at chord 644 is measured against the end at tick 202440 (161,356.44 ms), with ratio 1.1067. The tool priced the read of the first end with the second end's ratio: 158,224.76 + 1.1067 × (24.12 + 3.00) = 158,254.77 ms. Here 24.12 ms is the anchor's planned lead and 3.00 ms is the one frame of judging the plan counts (plan line 101). The press log's row 466 shows the game stored 158,268.99 ms.

**The hidden rule.** `retime_from_end` in `tools/ch_probe/experiments/play_chart.py` (lines 326 to 355) moves each later press by `read_end_ms - p.aim_end_ms`. The plan's owner table gives that row to Python (old plan line 138 on main, line 137 at the tip 4f6aa52b: "Shifting the later presses by read minus planned | Python | `retime_from_end` (one subtraction per press)"). The same plan quotes your ruling at line 29 and says "The owner table has one C++ owner for every Star Power question".

**The deferred premise.** The old plan's row P15 (line 340 on main) is marked `unchecked`, with Status "LS-S2 second read". Its Source names SP-24 on the Star Power page. SP-24 is labelled "Proven in the game's code", and `transfer_scale_between` (`src/core/squeeze_rating.cpp`, lines 12 to 22) takes an anchor tick and an end tick. A read of those two would have shown that a ratio belongs to one end. It would not have settled how the game moves a clamped end when a press is early; that part did need the live run.

**The three reviews.** The plan review (commit 413e396c, reviewer adae7fe267bcdb8b0) fixed a 3 ms double count inside the shift and added a premise row for `retime_from_end`. The TC2 code review (commit 05d243ae, reviewer aeee01a9b3e942e14) folded the end's shift into one function, `anchor_end_shift_ms`. That is a correct "derived twice" fix on the exact line that holds the borrowed ratio. The player audit's L12 (`docs/audit/2026-11-08-player-audit/ledger.md`, lines 134 to 142) is titled "retime_from_end moves presses by any delta and never rechecks the window", and its "Ruling quotes found" line says "none found specific to retiming". Each one was doing what its page asked.

**What I could not confirm.** The brief says the read end's true ratio is 1.39, that it sits in a 5/4 measure, and that the squeeze-in at chord 644 failed. The chart is a `.mid` file and I did not parse it. The press log shows chord 644 scored 200 points, which is what I read; I did not prove what a hit under Star Power would have scored. The brief says live step LS-S2 never ran. I found no folder named `ls-s*` under the run folder, which proves little on its own. None of these four carries into a task as settled (rows P21 to P24).

## Rulings that bind this plan

"the whole point of this plan is to hand off all the starpower planning to hydra which already knows what to do" (2026-11-08, card `docs/sessions/2026-11-08-6341c618.md`).

"why did the planner violate my explicit request to hand off planning to hydra? why did no reviewer ever catch this as a violation of derive once?", then "Yes, plan both" (today, as this plan's brief quotes them).

One review per change, with nothing going back (D107, agent rule 5). This plan adds steps to that one review. It adds no second review and no second round.

A reviewer's finding that needs you goes in its Notes, and the merge does not wait for it (`docs/agents/rulings/review-and-hooks.md`, line 19: "notes are fine"). Question 1 asks whether that should change for the two new kinds of finding. Until you answer, this plan keeps the ruling as it is.

Under `tools/`, a comment worded differently from correct code is a note, not a finding (D59). A borrowed value is not wording. The page already says a copy that gives two answers on a real input is still a finding there, and RG1 says so for the new kind in the same place.

I grepped the session index for review cards and read the review rulings page. I found no ruling that limits what a reviewer looks for, or that makes the precheck the only judge of kind D.

## Questions

1. **Should a review hold the merge when it finds an engine value used for the wrong thing, or an unruled rule in arithmetic, and the reviewer cannot fix it?**

   The rule today, from `docs/agents/derive-once-review.md` line 83: "Do not make those changes. Write the finding under Notes as a question for the user, in game terms, and sign off CLEAN on the rest. The orchestrator passes your Notes to the user. It does not hold the merge for them, except a visible change marked "not asked"".

   Most findings of the new kinds are fixable: the reviewer makes the tool call the engine function that owns the right value. The question is only about the ones that are not, because the engine has no such value yet or the rule needs your ruling.

   One end: Pitch Dark. The reviewer of the schedule tool finds that the read after chord 466 is priced with chord 644's ratio, and the engine stores no ratio for the end at tick 198570. Under (a) the tool merges, and you get a note. Under (b) the tool does not merge until you answer.

   The other end: a tool prints an engine time in seconds by dividing milliseconds by 1,000. That is a change of units. It is the same on every chart, so it is not a rule and neither option touches it.

   In the gap: the player's one-for-one shift. It is right when the two ends sit in measures of equal length and wrong when they do not. Under (a) it merges with a note. Under (b) it waits.

   - (a) Keep today's rule. The finding goes in Notes and the merge goes ahead.
   - (b) Hold the merge for these two kinds only, the way a "not asked" visible change already holds it.

   I recommend (b). A note did not stop any of the three reviews' merges, and a tool that plays a wrong rule costs a live run to find. The strongest case against: you ruled "i'm done with multiple rounds" and "notes are fine", and (b) lets a reviewer stall a merge on developer tools, where D59 says to go easy.

   You will see: nothing changes on any Hydra screen. Under (b), a merge can wait on a question it would not have waited on before.

## Tasks

RG1, RG2 and RG3 own different files and start together. RG4 runs on RG1's branch as soon as RG1 is committed, before RG1's review. Executors run on Opus, the replay readers on Sonnet. Every brief carries the status rule.

### RG1: the three repo pages (docs only; branch `claude/review-gap-docs`)

Files: `docs/agents/derive-once-review.md`, `docs/agents/premise-map.md`, `docs/agents/doc-review.md`, `docs/agents/rulings/review-and-hooks.md`.

**On the derive-once page.** Add kind H to the list of kinds: a value read from its owner that is the owner's answer for a different thing. Add step 10 to "How to review", titled "Check each value that crosses a boundary". A boundary is where a value leaves the code that owns it: engine to tool, C++ to Python, a stored record to a display. For each value the change reads across one, the reviewer writes one table row: the value, the exact thing its owner worked it out for (which tick, end, chord or activation, read from the owner's code and not from the reader's variable name), the exact thing the reader applies it to, and whether the two are the same. A row where they differ is a kind H finding. The fix calls the owner for the right thing.

The same step covers arithmetic. For each sum, difference, product or comparison the change does on an engine-owned quantity after it crosses the boundary, the reviewer asks one question: would a different chart, tempo or time signature make a different arithmetic correct? If yes, the arithmetic is a rule. A rule needs an owner in the engine or a ruling quoted in your words. With neither, it is a kind D finding. A change of units that is the same on every chart is not a rule.

Change the sentence that hands kind D to the precheck. It will say the precheck finds kind D's new numbers only, and that a rule with no number in it is the reviewer's to find in step 10. Kinds B, C and E stay quoted from the precheck as today.

Add the Pitch Dark case as the worked example under step 10, with both tables filled in. It uses only the numbers in this plan's "What happened" section. It does not use 1.39, the 5/4 measure or the word "failed", and it does not say LS-S2 never ran, unless the main session's brief gives a source for each (rows P21 to P24).

In "Developer tools: wording is a note", add that a kind H finding and a kind D rule are never wording. In "A finding you should not fix yourself", add the two new cases. If question 1 is answered (b), add them to the list of what holds the merge, and to the orchestrator section's matching sentence.

**On the premise map page.** Add three rules.

A source must be for the exact thing. A `code` row that says "the ratio this end moves by" names code that works the ratio out for that end. Code that works it out for another end, chord or tick is not a source for the row.

Settle what a read can settle. Before a plan is shown, the planner tries each `unchecked` row against the code and the Clone Hero pages. A row a read can settle is settled. If a read settles part of it, the row is split, as the page already requires. What is left names a task, a question or a live step. The page's Status sentence, which today allows only a task or a question number, gains the live step as a third allowed name.

A deferred premise blocks. When an `unchecked` row's Status names a live step, the Status also says which tasks rest on the row, as `blocks RG1, RG2` or `blocks nothing`. Each named task holds a line that starts `Waits for:` and names the step. The main session does not dispatch such a task until the step's result folder exists.

Add to "Who checks a map": the plan reviewer tries each `unchecked` row the same way, and a row a read could have settled is a finding.

**On the doc review page.** Add a "Plans" paragraph to Step 2. A ruling the plan quotes binds every row of its owner table. The reviewer reads every row whose owner is not the owner the ruling names. For each, it asks step 10's question: does this row move or convert a quantity the ruling gave to the other owner? A step that moves or converts an engine-owned quantity is not a "live part", whatever language runs it and whenever it runs. Sending a press and reading a value are live parts. Deciding how far a read moves a later press is not. Such a row is a finding. The fix moves the rule to the ruling's owner, or turns it into a question.

**On the rulings page.** Add one line dated 2026-11-12 with today's two quotes, in the line format `docs/agents/rulings/README.md` gives, backing the derive-once page.

Acceptance criteria:

- The derive-once page lists kind H and holds step 10. Proven by RG4's replays B and C.
- The page no longer tells the reviewer to quote the precheck for kind D and stop. Proven by a grep for "Quote its lines for B to E" that finds nothing, and by replay C.
- The doc review page holds the owner-table check. Proven by replay A flagging the owner table's "Shifting the later presses" row (line 137 at the replayed tip).
- The premise map page holds the three rules. Proven by replay A flagging the old plan's row P15, and by RG3's test for the Status shape.
- The worked example's numbers match this plan's. Proven by the reviewer recomputing 158,224.76 + 1.1067 × 27.12 = 158,254.77 and opening rows P4 and P5.
- No page states a rule another page owns. Proven by the reviewer's kind D read, and by the docs check `hydra_tests.exe "-tc=docs match code*"`, which the main session runs at the merge.

### RG2: the planner and reviewer definitions (hooks repo; branch `review-gap-agents`)

Files: `C:\Users\Patrick\.claude\agents\planner.md` and `C:\Users\Patrick\.claude\agents\reviewer.md`. The work happens in a worktree at `C:\Users\Patrick\.claude\worktrees\review-gap-agents`, as `C:\Users\Patrick\.claude\gates\README.md` requires. That repo's gate marker names `agents/`, so the change gets one fresh reviewer before it reaches that repo's `main`.

The planner's definition gains one paragraph. A ruling the plan quotes binds every row of its owner table. A step that moves or converts a quantity the engine owns is never a live part. An `unchecked` row that a code or doc read can settle is settled before the plan is returned. A row deferred to a live step says which tasks it blocks. The paragraph points at the premise map page and the doc review page for the details and repeats none of them.

The reviewer's definition gains one sentence after "When you review a plan, check its Premise map too": try each `unchecked` row against the code, and check the owner table against the plan's quoted rulings, as the doc review page says.

Acceptance criteria:

- Both files point at the repo pages by name and restate no rule. Proven by that repo's reviewer reading them against RG1's pages.
- A planner started after the merge is given the new paragraph. Proven by the main session reading the merged `planner.md` in the live folder and quoting the paragraph's first sentence in its report.

### RG3: the docs check reads the new Status shape (C++ test; branch `claude/review-gap-docs-test`)

File: `tests/test_docs_match_code.cpp`. The check `check_premise_maps` gains one rule. An `unchecked` row whose Status holds a live step's id (a token starting `LS-` or `LS` and a digit, the same shape the live-step heading check already reads) must also hold `blocks` followed by `nothing` or by task ids the plan has. Each task named must hold a `Waits for:` line that names that step. A plan dated before the rule's own start date is left alone, the same way `kPremiseMapSince` leaves older plans alone. The start date is the day the executor writes the change, and the comment beside it says so. The existing self-test case ("the premise map check catches each form") gains one failing example per new problem line and one passing example.

The executor does not build. The main session builds the branch and runs `hydra_tests.exe "-tc=docs match code*"`.

Acceptance criteria:

- A row deferred to a live step with no `blocks` word fails. A row naming a task with no `Waits for:` line fails. A row naming a task the plan lacks fails. Proven by the three new failing examples in the self-test.
- `blocks nothing` passes, and every plan already in the repo passes. Proven by the main session's run of the docs check on the branch.
- The rule's start date has one home. Proven by the reviewer's kind A read of the test file.

### RG4: replay the old reviews with the new pages (main session and three read-only scouts on Sonnet)

This is the proof that RG1's wording works. It follows the Calibrate stage of `docs/agents/audit-method.md`: a reader gets material with known problems and no hint of what they are.

The scouts are read-only and are not dispatched as reviewers. The reviewer dispatch hook refuses a key already on `main`, and every commit below is on `main`. Each scout gets the preamble, RG1's pages from the branch's worktree, and one target. Its brief says "review this as the page says and report every finding". The brief does not name Pitch Dark, the ratio, the shift, P15 or this plan. The worked example on the page does name Pitch Dark, so each scout reads RG1's pages with the worked example cut out; the main session writes those copies to the scratch folder.

- Replay A, the plan review: the file `docs/superpowers/plans/2026-11-08-replay-press-schedule.md` as it stood at commit 4f6aa52b27561a38d9183b711bdcd8f9b6759800, the tip the first reviewer was handed.
- Replay B, the TC2 code review: the range `c026e8cb4267d3d0e9302ffadcc865efbba37c4f...753ec20a8ac139fac1ef734969a2a70a14aa8033`.
- Replay C, the TP2 code review: commit ea4d247e31d20cc117c678b3d098f1c20d09f78b against its parent.

The gates README says the dispatch hook checks only `reviewer` dispatches (row P25), so a scout passes, but no scout has been sent a historic range. The precheck on a historic range has never been tried (row P26), and the scouts do not need it. If a hook refuses a scout, or the precheck refuses the range, the main session stops and reports the exact message. It does not route around it.

Cap: one run per replay. If a replay misses its item, a fresh executor rewords the step on RG1's branch once, and that one replay runs once more with a fresh scout. A second miss stops the work, and the main session brings you the design and its alternatives.

Acceptance criteria:

- Replay A reports the owner table's "Shifting the later presses" row as a rule outside the ruling's owner. It also reports the old plan's row P15 (line 341 at that tip) as a premise a read could have settled in part, or as one that blocks tasks built before its step.
- Replay B reports that the end read after an anchor chord is priced with a ratio the engine worked out for another end, as kind H.
- Replay C reports the one-for-one shift in `retime_from_end` as a rule with no ruling, as kind D.
- Each is proven by the scout's report quoting the file and line. The main session pastes those sentences into RG1's reviewer brief as the author's report.
- A replay that also reports things that are not problems does not fail the proof. The main session lists them for you (see "Out of scope").

## Order and merges

RG1 is committed first on its branch. RG4's three scouts run at once against it. When all three pass, RG1 goes to its one reviewer with the replay reports, then merges. RG3 builds and merges on its own. RG2 merges in the hooks repo after RG1, so the pages it points at exist. This plan file leaves the repo in the last of those merges.

## Out of scope, for you to approve

The fix to `tools/replay_schedule.cpp` and to the player's shift is the other plan.

No hook enforces the `Waits for:` line. RG3 makes a plan state the block, and the main session keeps it. A hook that refuses an executor dispatch for a waiting task would be a new hook, and this plan does not build one.

No past plan or review is re-read for the same gap, apart from the three replays. An audit of earlier owner tables would be its own plan.

The replays do not measure false alarms. If step 10 makes reviewers flag harmless arithmetic, that shows up in later reviews, and the wording gets tightened then.

## Promise map

| Promise | Plan line | Task | The check that proves it |
|---|---|---|---|
| A review asks, for each value read across a boundary, whether it is the owner's value for this exact thing | RG1, derive-once page, step 10 | RG1 | Replay B reports the ratio as kind H, quoting file and line |
| Kind D is no longer left wholly to the precheck; arithmetic across a boundary is judged as a possible rule | RG1, derive-once page | RG1 | The grep for "Quote its lines for B to E" finds nothing; replay C reports the shift as kind D |
| A ruling quoted in a plan binds every row of its owner table, and moving or converting an engine-owned quantity is never a live part | RG1, doc review page; RG2 | RG1, RG2 | Replay A reports the owner table's "Shifting the later presses" row; the hooks repo's reviewer reads `planner.md` against the page |
| An unchecked premise a read can settle is settled before the plan is shown | RG1, premise map page; RG2 | RG1, RG2 | Replay A reports the old plan's row P15 |
| A premise deferred to a live step blocks every task that rests on it | RG1, premise map page; RG3 | RG1, RG3 | RG3's three failing self-test examples; the docs check on every plan in the repo |
| The worked example is the Pitch Dark case, with only sourced numbers | RG1 | RG1 | The reviewer recomputes 158,254.77 and opens rows P4 and P5; rows P21 to P24 stay out |
| The replay briefs give no hint of the known problems | RG4 | RG4 | RG1's reviewer reads the three scout briefs and the cut pages in the scratch folder |
| A replay runs once, one reword is allowed, and a second miss goes to you | RG4 | RG4 | The main session's report names each replay's run count |
| A refused scout or precheck is reported, never routed around | RG4 | RG4 | The main session's report quotes the message, or says none came |
| The hooks repo change goes through a worktree and one reviewer | RG2 | RG2 | That repo's ref gate refuses the merge without a sign-off |
| No page restates a rule another page owns | RG1, RG2 | RG1, RG2 | Each reviewer's kind D read; the docs check at the merge |
| The new Status rule's start date has one home and leaves older plans alone | RG3 | RG3 | The reviewer's kind A read; the docs check passes on the branch |
| One review per change stays the rule | Rulings that bind this plan | RG1 | The reviewer confirms step 10 sits inside "How to review" and adds no second dispatch |
| Notes do not hold a merge unless you answer question 1 with (b) | Rulings that bind this plan | RG1 | The reviewer compares the page's "should not fix yourself" text with your answer |
| This plan leaves the repo in the merge that finishes it | Order and merges | RG2's merge step, main session | The docs check at that merge finds no line still pointing at it |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: the user's 2026-11-08 words are "the whole point of this plan is to hand off all the starpower planning to hydra which already knows what to do" | ruling | `docs/sessions/2026-11-08-6341c618.md` | checked 2026-11-12, by grep for the phrase |
| P2: the user's words today are the two quotes under "Rulings that bind this plan" | ruling | this plan's brief from session `bed936f0-797e-4a36-bbbe-07673ff356e4`; the main session repeats them in the reviewer's brief | checked 2026-11-12 |
| P3: the schedule tool takes a squeeze-in's ratio from `SPSqueeze::transfer`, keeps the first squeeze's ratio per anchor, and prices the read after the anchor with it | code | `tools/replay_schedule.cpp` lines 661 to 662, 692 and 753 to 767 | checked 2026-11-12 |
| P4: on Pitch Dark, activation 1's end after chord 466 is tick 198570 at 158,224.76 ms, the squeeze at chord 644 is measured at tick 202440 with ratio 1.1067 and anchor lead 24.12 ms, and the read is planned at 158,254.77 ms | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\pitch-dark-prep` (`schedule.json`) | checked 2026-11-12 |
| P5: the game's end read after chord 466 was 158,268.99 ms, chord 466's press went out at clock 143,250.17 ms against a chart time of 143,224.27 ms, and chord 644's row shows the score going from 283,065 to 283,265 | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\pitch-dark-run1` (`press-1.csv`, rows 466 and 644) | checked 2026-11-12 |
| P6: `retime_from_end` moves each later press by the read end minus the press's aimed end | code | `tools/ch_probe/experiments/play_chart.py` lines 326 to 355 | checked 2026-11-12 |
| P7: the old plan quotes the ruling at line 29, gives the shift to Python at line 138, and marks P15 `unchecked` with Status "LS-S2 second read" at line 340 | code | `docs/superpowers/plans/2026-11-08-replay-press-schedule.md` lines 29, 138 and 340 on main (29, 137 and 341 at commit 4f6aa52b, where the old P15 sits one line below P14) | checked 2026-11-12 |
| P8: the engine measures a squeeze-in's ratio at that squeeze's own end, with a function that takes an anchor tick and an end tick | code | `src/core/squeeze_rating.cpp` lines 12 to 22 and 34 to 42 | checked 2026-11-12 |
| P9: SP-24 is labelled "Proven in the game's code" | data | `docs/clone-hero/05-star-power.md` lines 247 to 250 | checked 2026-11-12 |
| P10: the plan review fixed a 3 ms double count and added a row for `retime_from_end`; the TC2 review folded the shift into `anchor_end_shift_ms` | code | commits `413e396c` and `05d243ae` (their messages and trailers) | checked 2026-11-12 |
| P11: audit finding L12 checked only the window recheck and found no ruling on retiming | code | `docs/audit/2026-11-08-player-audit/ledger.md` lines 134 to 142 | checked 2026-11-12 |
| P12: the derive-once page lists kinds A to F and G, says "Quote its lines for B to E; do not hunt for them again", and sends unfixable findings to Notes without holding the merge | code | `docs/agents/derive-once-review.md` lines 30 to 37, 51 and 83 | checked 2026-11-12 |
| P13: the precheck's kind D check is for new numbers | code | `tools/precheck/check_3.cpp` line 1 | checked 2026-11-12, the header comment only |
| P14: the premise map page lets an `unchecked` row name a task or a question, and the docs check accepts any heading of the form `### <id>:`, which a live step's heading is | code | `docs/agents/premise-map.md` lines 9 and 29; `tests/test_docs_match_code.cpp` lines 1703 to 1723 | checked 2026-11-12 |
| P15: the planner's definition holds the Promise map and Premise map rules and nothing on owner tables; the reviewer's holds the sentence "When you review a plan, check its Premise map too" | code | `C:\Users\Patrick\.claude\agents\planner.md`; `C:\Users\Patrick\.claude\agents\reviewer.md` line 13 | checked 2026-11-12 |
| P16: hooks repo work happens in a worktree under its `worktrees` folder, and its gate marker names `agents/` | code | `C:\Users\Patrick\.claude\gates\README.md` lines 120 and 122 | checked 2026-11-12 |
| P17: the user ruled that findings needing them go in Notes and do not hold the merge | ruling | `docs/agents/rulings/review-and-hooks.md` line 19 | checked 2026-11-12 |
| P18: the reviewer dispatch hook refuses a key already on `main` | code | `docs/agents/derive-once-review.md` line 17 | checked 2026-11-12, the page's statement, not the hook's code |
| P19: the first plan reviewer was handed tip 4f6aa52b, the TC2 reviewer tip 753ec20a, and TP2's code is commit ea4d247e | code | `git log` of `docs/superpowers/plans/2026-11-08-replay-press-schedule.md`; `git log fd93c684^1..fd93c684^2`; `git log 44200a63^1..44200a63^2` | checked 2026-11-12 |
| P20: no ruling limits what a reviewer looks for or makes the precheck the only judge of kind D | unchecked | `docs/agents/rulings/review-and-hooks.md` and a grep of `docs/sessions/index.md` for review cards found none; a search that finds nothing proves little | question 1 |
| P21: the read end's true ratio on Pitch Dark is 1.39 | unchecked | this plan's brief only; the chart is `notes.mid` and was not parsed | RG1 |
| P22: the read end sits in a 5/4 measure and the squeeze end in 4/4 | unchecked | this plan's brief only; `schedule.json` in the prep folder holds ticks and times but no time signatures | RG1 |
| P23: the squeeze-in at chord 644 failed | unchecked | this plan's brief; `press-1.csv` row 644 shows 200 points, which was not compared with a Star Power hit | RG1 |
| P24: live step LS-S2 of the old plan never ran | unchecked | no folder named `ls-s*` under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs`; the folder name is a guess | RG1 |
| P25: a scout dispatch on a historic range passes the dispatch hook | code | `C:\Users\Patrick\.claude\gates\README.md` line 37: the hook checks an `Agent` call with `subagent_type: reviewer`, so a read-only scout is not checked | checked 2026-11-12, the README's statement, not the hook's code |
| P26: the precheck runs on a historic range written as `<base>...<tip>` | unchecked | `tools/precheck/range.cpp` line 31 takes a merge-base of the two names; never run on these commits. The scouts do not need it, so only a main session that wants the lines tries it | RG4 |
