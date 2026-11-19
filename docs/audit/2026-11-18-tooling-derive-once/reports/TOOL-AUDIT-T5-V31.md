# TOOL-AUDIT-T5-V31 (verifier of F31). All claims "by reading". Nothing run, nothing changed.

Checked: the 6 splits, the K8 rows, the noflip row, the line-drift side note, the percall.py note. Not reached (taken as the finder's words, unchecked): the "tests and builds" K1 row (about 10 homes), rubric rows I7-104..I7-161, owner-map rows I7-173..I7-180, memory rows I8-009/010/013/016/026/029/032-038/041-046/053/056, I7-199..I7-219, I7-190.

## Verdicts

S1 CONFIRMED. Memory derive-display-from-engine-truth.md:18 "If none exists, propose one rather than copying." owner-map.md:37 "Nobody writes a new owner for it, because a missing answer to a game question is the user's call". scout.md:11 "Never propose a new owner." agent-rules.md:21 (rule 15) says "the new owner goes in core"; agent-rules.md:23 says that line "never applies to a display". So rule 15 has a display case and a non-display case. Situation: a missing owner, by actor and by whether it is a display. No Confirmed line I read names both sides.

S2 CONFIRMED-CHANGED. Homes say what the finder says: agent-rules.md:41 "CONTEXT.md decides what a project word means"; agent-rules.md:43, derive-once-review.md:51, brief-preamble.md:99 "The code's usage settles the meaning, not a guess drawn from one sentence of a doc". But brief-preamble.md:99 also says that if the definition and the code's usage disagree, ask the user and change neither. So the order is: CONTEXT.md decides, code usage is checked before calling a term wrong, disagreement goes to the user. That is a wording tension, not a clean split. domain.md:43 only says "use the term as defined in CONTEXT.md" (no conflict).

S3 CONFIRMED. integrate.md:5 "You write no new feature code. You only fix what the join itself breaks"; integrate.md:21-23 "Fold it into the owner in the join ... If a fold would change a displayed text, a score or a stored record, stop and report it. Do not pick." derive-once-review.md:96 "Fold every copy into the owner." with no display-change stop on that line (finder says step 9 lists visible changes; not re-read by me). Situation: integrator vs reviewer, a fold that changes a display. The integrate.md:5 versus fold point is a tension only, since a fold is inside "fix what the join breaks" when the copy came from the join. Ruling agents-and-workflow.md:19 (as the finder quotes it) covers displays and records, but does not name the fold step, so it does not name both sides.

S4 CONFIRMED-CHANGED. Both homes are as quoted (memory :24 "write a table of each question and the side that owns it before dispatch"; derive-once-review.md:53, item 10, "a table with four columns" per value that crosses a boundary). They share the word "boundary" but differ in actor (planner/orchestrator before dispatch vs reviewer after) and in table shape. Kind K3 (same word, two meanings), not one question with two answers.

S5 CONFIRMED. test-rubric-examples.md:270-272 says multiplier_step is called "for the multiplier squeeze (model.cpp:369)". model.cpp:366-369 is MultSqueeze::applies, which calls crosses_multiplier_step; timing.cpp:20-27 shows crosses_multiplier_step calls multiplier_step, which calls to_multiplier. The owner is right; the caller claim is indirect. K8-light.

S6 CONFIRMED as "not proven". Memory drum-lanes-d112:13 "a lane lights every pad its notes land on, on purpose"; song.cpp:192-196 comment says the same and cites D254 ("the user ruled that a shuffled roll looks better with every pad lit"). Confirmed ui-and-display.md:56: "the lane on the screen does not match the notes that are in the chart. i know its note shuffle, that doesnt excuse the preview being wrong". Both can hold if "the notes in the chart" means the shuffled notes; by reading that point is unsettled. The D254 words ("clone hero only draws the blue ...", ui-and-display.md:117) are under Unconfirmed, so they are not the user's words. No ruling names both sides.

## Other rows

I8-015 (noflip wording) CONFIRMED. Memory description says Clone Hero "ignoring drums0dnoflip". NL-21 (02-note-list.md:253-255) says the game "counts as a span start ... adds only a no-flip flag ... so an earlier drums0d keeps flipping through it", under the heading "does not turn flip off". Hydra's value (flip off) agrees in both. K3, wording only. The quoted ruling ("this is a bug in clone hero") is the user's words on "bug", not on "ignores".

I8-054 drift CONFIRMED (partly). 02-note-list.md:611 says light_drum_lanes line 203, lights_lane_pad line 88, game_lane_pads line 147; :631 says "called at line 2009". Code in src/parse/song.cpp: lights_lane_pad :90, game_lane_pads :149, light_drum_lanes :205. The call line 2204 is the finder's number; I did not see it printed, so it is unchecked. K2.

I8-025 CONFIRMED in scope searched. Memory :15 says "Planner and reviewer briefs say this too." grep of "edge case" over C:\Users\Patrick\.claude\agents finds no file. Briefs outside that folder were not searched. K8.

I8-071 CONFIRMED as not found. Memory fc-video-workflow.md:15 says "Both are now README checklist items 0 and 5." grep for "checklist" and "never rewrite" in docs/clone-hero/README.md and docs/agents/*.md found none; no other README located. Stays open; a search that finds nothing proves nothing alone.

Skill pointer CONFIRMED. SKILL.md:34 names memory `gameplay-video-score-analysis`; it exists only as memory/archive/gameplay-video-score-analysis.md. K8.

percall.py:108-110 CONFIRMED. Deny message: "Agents never build, so hydra_build_pass is refused to every agent. ... the main session builds your branch and runs any test that needs a build". It restates rule 14 and names its own refusal.

## Splits added
None beyond S1 to S6. S2 and S4 are tensions or shared words, not proven splits; the main session should see both.

## Confirmed rulings
Re-read: ui-and-display.md:56 and :117. clone-hero.md:53 seen quoted in NL-21. None names both sides of any of S1 to S6.
