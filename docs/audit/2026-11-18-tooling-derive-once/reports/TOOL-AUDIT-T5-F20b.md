# TOOL-AUDIT-T5-F20b (re-send finder, agent adcca20d2a7592723)

Rows taken (the ones F20 listed as not reached or partial): I8-072, I8-083, I8-112, I8-143. F20's other "thin" rows (I8-117, I8-204, I8-205) it called one-home answers from a grep; I did not retake them. F20 also said it did not search Hydra tools\ beyond a few greps; none of my four rows lives there (I grepped tools/hydra_ui_mcp.py for zoom/capture only). Abbreviations: MEM = C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory; AR = docs/agents/agent-rules.md; DO = docs/agents/derive-once-review.md; RR = C:\Users\Patrick\.claude\gates\review_rules.py. Searched for all rows: gates (*.py, *.md, lib, tests), agents, model-postures.md, both CLAUDE.md, skills, settings.json, docs/agents incl. rulings, handoff.md, memory notes.

## I8-072 Does tool work (the video tools) get a reviewer before it counts as done?
Row: MEM fc-video-workflow.md:17 "tool work ran as one Opus implementer plus one Sonnet reviewer that reproduced every done-when itself ... Keep the reviewer gate, and read its evidence, not the implementer's summary."
Homes:
- MEM fc-video-workflow.md:17 (above). Tells the main session to keep the reviewer and read its evidence.
- RR:22 `BUILTIN_CODE = ^((src|tests|tools|third_party|installer|\.github)/|CMakeLists.txt|...)`: the hook's own list of code paths. Hydra's `tools/` is in it, so a change there needs a clean verdict before main moves.
- DO:86 "Under `tools/` (developer tools, not shipped in Hydra) a comment, header or doc line that states a rule differently ... is a note, not a finding" (a softer standard for tools, user's D59 per that line).
- AR rule 5 (agent-rules.md:11) and CLAUDE.md:67: "One review per change", one fresh reviewer; no mention of "reproduce every done-when".
Edges:
| Home | Reviewer needed? | Notes |
|---|---|---|
| MEM:17 | yes, reads evidence not summary | names "Opus implementer + Sonnet reviewer", matches AR rule 2 |
| RR:22 | yes for Hydra `tools/`, by hook | enforced |
| video-tools repo | not enforced | see split S1 |
| main vs agent | MEM tells the main session; hook binds both | |
| card writers (skip preamble) | n/a | |
| worktree vs main | RR works on base..new in a repo with the marker | |
Kinds: K5 (prose "keep the reviewer gate" where a hook is the gate, for Hydra's tools/), K8-ish check: the note says "the reviewer gate" without saying that no gate exists in the video-tools repo.
Split S1: the video tools live in a separate git repo, C:\Users\Patrick\Downloads\Hydra\video-tools (has its own .git; `ls -a` shows no `.derive-once-gate` or `.doc-review-gate` file). RR keys its gate on that marker file (RR:15-22). So for that repo the "reviewer gate" in MEM:17 is prose only; the hook does not run there. I read the marker's absence from the directory listing only; I did not read how the hook treats a repo with no marker beyond RR:15-22. MEM:17 and RR:22 therefore answer "does tool work get a reviewer" differently for video-tools (a rule with no hook) versus Hydra tools/ (a hook).
Another note: the memory line says the reviewer "reproduced every done-when"; no page in docs/agents (grep "done-when", "done when") sets that as a reviewer duty. One home for that sub-rule.
Rulings found: none under `## Confirmed` that name this (I grepped docs/agents/rulings for "reviewer gate", "tool work": no hit). Tests holding homes equal: none (gates/tests/test_review_rules.py does not mention BUILTIN_CODE or tools/).

## I8-083 Does removing hook guesses ever reverse a user decision or add a second review (D107)?
Row: MEM hooks-never-guess.md:17 "removing guesses never reverses a user decision or adds a second review (D107)."
Homes (the D107 one-review rule itself, which this line restates by reference):
- MEM one-reviewer-rule-d107.md:11 "A change gets ONE fresh reviewer ... No separate fixer, no checker, no second round, no 'bring it back to the user' step".
- AR rule 5 (agent-rules.md:11) "One review per change, then done ... There is no separate fixer, no checker and no second round, and nothing goes back to the user (D107). There is one narrow exception (D314)" (C++ fix that fails the build gets a fresh executor and a fresh reviewer who reads only that fix).
- CLAUDE.md:67 same rule, shortened, same exception.
- DO:175 "That is the one case where a signed-off change gets another look. It is a narrow exception to D107 ... the user approved it in D314."
- MEM MEMORY.md:11 "TOP RULE: one fresh reviewer fixes, commits, signs off; nothing goes back."
- MEM hooks-never-guess.md:17 (the row).
- docs/agents/doc-review.md:17 "each document gets it once" (the doc-side form).
Ruling found: docs/agents/rulings/review-and-hooks.md:49 (under a section I confirmed only by header list: `## Confirmed` is at line 7, `## Unconfirmed` at line 37, so line 49 sits under Unconfirmed, not Confirmed) says "this has all been a massive waste of time." and "i asked for the review agent to fix its findings directly, then we're done." Because it is under `## Unconfirmed`, it is a home, not a ruling. I did not read the Confirmed lines (budget).
Table:
| Home | At the limit | Main vs agent | Card writers | Worktree |
|---|---|---|---|---|
| AR 5 | one review, C++ build-failure exception, reviewer out of time signs off what it fixed | rule holds for both | no review of cards stated | n/a |
| MEM hooks-never-guess:17 | "never adds a second review" with no exception named | memory, main session reads | | |
| DO:175 | the exception stated as a narrow one | | | |
Split S2 (K3, wording): MEM hooks-never-guess:17 and MEM one-reviewer-rule-d107 say "never a second review / nothing goes back" without the D314 exception; AR 5, CLAUDE.md:67 and DO:175 state the exception. Today no one acts differently, but the memory lines are the stricter copies.
Also: docs/agents/audit-method.md:81 "A fresh agent checks every closure" and :58 "Fresh agents check every candidate" are further reviews of audit work; not code review, so a different question, but none of the D107 lines says audits are outside it.
Hook side: RR (lines ~217-258) records a verdict per commit; a branch with no verdict is refused ("no reviewer verdict is recorded for %s or any commit before it on its branch", RR:222). That matches the memory line "a hook that can't find what it needs is a bad hook; fix the cause, never record 'no verdict'". No test I found holds the memory and the hook equal.
Side note: the T3 row flags the note's modified time as 2026-11-19; I did not judge it.

## I8-112 Does every plan for a screen carry one GUI test of the whole main task, done by clicking?
Row: MEM new-screens-start-from-a-clickable-mockup.md:15 "Every plan for a screen includes one GUI test that does the whole main task by clicking on screen, not by calling the engine."
Searched for: "GUI test", "main task", "whole task", "click", "clickable", "new screen", "plan for a screen" across all scope.
Homes:
- MEM new-screens-start-from-a-clickable-mockup.md:15 (the row).
- MEM same note, "before planning any new screen, build the mockup ... have the user do the screen's main task in it": a mockup step, a different rule (user clicks a mockup), same note.
- MEM MEMORY.md "New screens start from a clickable mockup ... user clicks through the main task in a mockup before any plan" (the mockup rule only; does not mention the GUI-test rule).
- docs/agents/ui-testing.md:3, 9-11: the three cases of which check a GUI change needs. Says `hydra_uitest` drives the UI by label and click. It does not say a screen plan must include a whole-main-task test. ui-testing.md:97-101 holds the click verbs.
- C:\Users\Patrick\.claude\agents\planner.md:17 "A plan for a screen invents no rule. Every value the screen shows is a value the engine already computes ..." (about values, not tests). planner.md has no line on a GUI test for the main task.
- docs/agents/premise-map.md and owner-map.md: no word of this rule (grep "click", "main task").
Result: the whole-main-task GUI test rule has one home, a memory note. The planner agent page and the plan-shape pages do not state it. That makes it K7 (a prose rule with no home among the pages plans are written from; nothing checks it) only if it is meant to bind plans; I only report that it appears once. No hook checks it (grep in gates/*.py for "GUI test"/"main task": none).
Edges: main session vs agent: the memory note is read by the main session; planner agents (agents\planner.md) never see it. Card writers: n/a. Rulings: none found; the note's "Why" attributes the origin to the Path Builder session (user quote not in my search).
Tests holding homes equal: none.

## I8-143 Look at a capture at 2x zoom before showing it
Row: MEM render-checks-use-the-corpus.md:18 "Zoom it 2x and check gaps, column alignment and shared baselines before showing it, and name any flaw yourself."
Homes:
- MEM render-checks-use-the-corpus.md:18 (row) and its description line 3 "look at the capture at 2x, check the song open and close transition". Same note, two lines, same figure 2x.
- docs/agents/ui-testing.md:11 (case 2): "Before merge the change is also seen in the real dev app ... and the captures are shown to the user." No zoom, no 2x, no baseline check; it tells the reader to show captures.
- docs/agents/doc-screenshots.md (the tool that makes the public-page pictures): grep for zoom/2x/inspect found nothing about checking the picture at 2x (I read matched lines only).
- tools/hydra_ui_mcp.py: a screenshot verb exists (SHOT_REPLY, line 84); no zoom or 2x step in it (grep: zoom, 2x: no hit).
Table:
| Home | Zoom before showing? | Who |
|---|---|---|
| MEM:18 and :3 | yes, 2x, name flaws | main session |
| ui-testing.md:11 | not stated; "captures are shown" | main session |
| doc-screenshots.md | not stated | doc pictures |
Split S3 (K3, partial): ui-testing.md:11 tells the main session to show the captures; MEM:18 adds a 2x look-first step. The page that is the check's home does not contain the step. The number 2x has one home (the memory note, twice).
Rulings: none found with these words. The user quote "ugly as hell" appears in MEM:18; it is inside a memory note, not a ruling.
Tests: none; no hook checks it.

## Side notes
N1. MEM render-checks-use-the-corpus.md and ui-testing.md:11 and :13 repeat the same user quote and story (column widths squeezed on the real Library) in three places (memory description, memory body, ui-testing.md case 2, plus case 3's "you dont need a full library run" in both). Copies of the same words; no value drift seen.
N2. Gap, in general words: the review gate keys on a marker file per repo, so a repo without the marker (video-tools) has no review hook.

## Summary
Rows taken: I8-072, I8-083, I8-112, I8-143. Splits: S1 (reviewer gate for the separate video-tools repo: prose only, no hook), S2 (D107 memory lines omit the D314 exception), S3 (2x-zoom step only in a memory note, not in ui-testing.md). I8-112 and the 2x rule have one home each in a memory note. No `## Confirmed` ruling found for any row, and no test holds any of these homes equal.
