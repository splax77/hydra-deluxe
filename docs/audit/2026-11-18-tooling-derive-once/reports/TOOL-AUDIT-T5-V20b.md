# TOOL-AUDIT-T5-V20b (verifier of F20b). All claims "by reading".

## I8-072: CONFIRMED
- MEM fc-video-workflow.md:17 quoted as the finder says ("one Opus implementer plus one Sonnet reviewer that reproduced every done-when itself ... Keep the reviewer gate").
- review_rules.py:18 marker names `.derive-once-gate`; :21-22 BUILTIN_CODE includes `tools/`; :153-154 `_has_marker` checks `git ls-tree` for the marker; :408 the code gate turns on only if a rev has it.
- `ls` of C:\Users\Patrick\Downloads\Hydra\video-tools: has its own .git; `.derive-once-gate` does not exist. S1 holds: the hook gate is off there, the note's "reviewer gate" is prose only. Open (needs a run, not done): what the hook does on a push in that repo.
- Edge: Hydra `tools/` (hook) vs video-tools repo (no hook). "Reproduce every done-when": grep of docs/agents found no home; one home (the memory note).
- Rulings: no `## Confirmed` line mentions reviewer gate / tool work / video-tools (grep of rulings/*.md).

## I8-083: CONFIRMED-CHANGED
- Homes quoted: hooks-never-guess.md:17 "removing guesses never reverses a user decision or adds a second review (D107)"; one-reviewer-rule-d107.md:11 "ONE fresh reviewer ... no second round" (no D314/exception hit by grep); agent-rules.md:11 states "one narrow exception (D314)"; CLAUDE.md rule 5 same; derive-once-review.md:175 "narrow exception to D107, and the user approved it in D314". S2 holds (the memory notes omit the exception).
- Change: the finder said it read no Confirmed lines. Under `## Confirmed` in docs/agents/rulings/review-and-hooks.md: 2026-10-10 "i asked for the review agent to fix its findings directly, then we're done." (session 02851f21) names the one-review side. Also Confirmed: 2026-10-09 "1 - send the fixes to a fresh agent" and 2026-10-05 "make that fix and have a new review agent pick up what's left", which lean the other way. None names both sides of the D314 exception split. The finder's "massive waste of time" quote is only under `## Unconfirmed` (headers at lines 7 and 37), as it said.
- Situation: a C++ reviewer fix that fails the main session's build.

## I8-112: CONFIRMED
- new-screens-start-from-a-clickable-mockup.md:15: "Every plan for a screen includes one GUI test that does the whole main task by clicking on screen, not by calling the engine."
- grep "main task|GUI test|click" in agents/planner.md, premise-map.md, owner-map.md: no hit. ui-testing.md lines 3-16 do not say it. Rulings: no hit for "main task/whole task". I did not redo the finder's gates/*.py search (unchecked by me).
- One home, a memory note; no hook.

## I8-143: CONFIRMED
- render-checks-use-the-corpus.md:3 "look at the capture at 2x" and :18 "Zoom it 2x and check gaps, column alignment and shared baselines before showing it". ui-testing.md case 2 says only "the captures are shown to the user"; grep "zoom|2x|inspect" in ui-testing.md and doc-screenshots.md: no hit. S3 holds. Rulings: no hit for "2x|zoom".

## Splits
S1 video-tools repo: reviewer gate prose only. S2 memory notes omit D314 exception. S3 2x step only in memory. S4 (new): Confirmed rulings on review hold one-review words and fresh-agent-for-fixes words; none names the D314 exception.

## Not reached
None of the four candidates. I did not read docs/audit/2026-10-03-fix-decisions.md for D107/D314.
