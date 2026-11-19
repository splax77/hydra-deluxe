# TOOL-AUDIT-T5-V04 (verifier of F04). All claims by reading; nothing run.
Checked against files, not the finder's words.

## Splits
1. LINE_BUDGET: CONFIRMED. gates\tests\replay.py:41 `LINE_BUDGET = 3100  # the user's hard budget for hook code; only the user changes it`. review-and-hooks.md has `## Confirmed` at line 7 and `## Unconfirmed` at line 37. Line 13 (Confirmed): chose "Raise cap to 3,050". 3,100 appears only at line 44, under Unconfirmed ("rase cap to 3100 ... backs: no page yet"). No Confirmed line names both 3,050 and 3,100.
2. Hard budget vs "not too concerned": CONFIRMED. Line 11 (Confirmed, 2026-10-31): "im not too concerned about the exact number as much as efficiency and functionality..." vs replay.py:41 "hard budget ... only the user changes it". The Confirmed line does not name both sides. (replay.py:489 exit 1 and the 5,572 line count not rechecked.)
3. /handoff command vs handoff.md: CONFIRMED. .claude\commands\handoff.md:12 "If a row is still `missing` after that second run, leave it, and say which row and why in your report." docs\agents\handoff.md:65 "fixes every `missing` row it finds". I found no ruling on it.
4. build_slot.ps1:79 "The user's answer in the same rulings page" (120 s): CONFIRMED. The only 120 s text is agents-and-workflow.md line 35, under Unconfirmed (header line 32): "Yes, all recommended" to seven questions, including "(an unclaimed pass runs out after 2 minutes, 120 seconds)". Confirmed line 8 covers the three-builds limit only.
5. stall.py:36 "Auto-compact should have fired at 500K (50% of the window)": CONFIRMED that settings.json and settings.fragment.json have no AUTOCOMPACT setting (grep). I did not search the README or Claude Code's own docs; the 50% may be a Claude Code default, which only its docs or a run could settle.
6. Library chart counts in memory notes: CONFIRMED-CHANGED. Counts seen: 18,610 (min-path-delta-proof.md:29), 18,811 (no-full-library-runs.md:17), 18,755 (stored-versions-one-rule.md:11), 19,000 (measure-small-explain-first.md:11). search-before-clicking-a-song.md:17 holds only the prefix "of 19", a check string, not a count. So four counts, each a dated example. The ruling-file figures 18,848 and 19,610 were not rechecked.
Missed splits: none.

## Candidates
- K1 budget numbers in many docs: CONFIRMED. progress.py:14-16 holds `100, 150`, `20, 30`, `5, 10`. Copies seen: writer-brief.md:100-101, reviewer.md:11 ("at 100 calls or 20 minutes"), model-postures.md:5 (line read), CLAUDE.md:69. Which docs the test covers rests on F04's reading of test_docs_numbers.py; I did not reopen it.
- K3 writer-brief.md:101 and model-postures.md:5: CONFIRMED. writer-brief says finish the card in hand and report; the preamble says commit and report; writer-brief is silent on 150 calls / 30 minutes.
- K1 660 wake timeout: CONFIRMED (settings.fragment.json:235 and settings.json:245, both `"timeout": 660`).
- K1 500K typed in stall.py twice: CONFIRMED (stall.py:27 `CTX_THRESHOLD = 500000`; :36 text says "500K"). model-postures.md:15 not rechecked.
- K1 three builds in 4 docs: CONFIRMED-CHANGED. CLAUDE.md:76, agent-rules.md:20 and development.md:114 say "at most three builds run at once". brief-preamble.md:57 says "You never build ... (agent rule 14)" and points to rule 14 instead of stating three on that line.
- K7 turn cap: CONFIRMED for README.md:128 ("No agent file in ..\agents\ sets `maxTurns`"). The preamble wording was not matched by my grep.
- K8 posture.py:16 comment "the user's answer to Q8 of the hooks plan HX-P2": the comment exists (line 16). Whether any ruling backs it I did not search; F04's claim stands unchecked.
- K8 build_slot.ps1:79: see split 4. K8 replay.py:41: see splits 1 and 2.

## Side notes
- REFUTED: F04 says `C:\Users\Patrick\.claude\hooks` does not exist. It exists and holds `state\build_slots` (ls). So build_slot.ps1:78's path is real, and the side note about it (and about bench_run.ps1:8-9) is wrong.
- CONFIRMED: 05-star-power.md:417 says "src/core/model.h line 111"; the constant is at model.h:121.
- CONFIRMED: analyzing-sp-squeezes SKILL.md:10 names src/search/graph.h; the constant is in src/core/model.h:121.
- Not rechecked: test_progress.py:856-858 literal tuple; the .py-only scan.

## Not reached
Groups B, C, I, J, L (SP leeway), M; the 15 s / 60 s build-slot values; test-file claims in groups A, B, D.
