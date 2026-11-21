# TOOL-AUDIT-T5-V05 (verifier of F05). All claims "by reading"; I ran nothing. H = main checkout.

## 1. Quiet check CPU 15 vs 30 (K2): CONFIRMED-CHANGED
Homes: H\tools\ch_mcp\quiet.py:29 `QUIET_CPU_PCT = 30` (:30 `QUIET_SAMPLE_S = 5`). The old 15 is in two places in H\docs\superpowers\plans\2026-11-12-player-stall-fixes.md: line 74 (question 3 text) and line 303 (premise row P22, "CPU over 15 percent ... checked 2026-11-12"). The finder named only line 74. rulings\clone-hero.md has `## Confirmed` at line 7 and `## Unconfirmed` at 67, so line 72 ("raise the limit to 30%") is Unconfirmed: a home, not the user's words. Its own context says the old limit was 15.
Situation: reading a plan versus the code. The plan is a dated record, so the 15 is stale history, not a live reader.
SPLIT: 15 (plan lines 74 and 303) vs 30 (quiet.py, Unconfirmed line 72). No Confirmed line names either side.

## 2. "flat 85 ms" in the ch_probe README (K2/K3): CONFIRMED-CHANGED (finder partly wrong)
Homes: H\tools\ch_probe\README.md:101 "Hydra's model uses a flat 85 ms hit window"; H\tools\ch_probe\SPEC.md:302 "Hydra's model uses a flat `kHitWindowMs = 85.0`" (the finder missed this copy). H\src\core\model.h:106 `kHitWindowMs = 85.0`. H\src\core\note_window.h:8-11 says ratings, tiers, badges and search keep kHitWindowMs, "a separate number (D270 call 8): the user ruled 'just show it alongside.'" The clamps kWindowFrontMs 37.5 / kWindowBackMs 85.0 and the formula (note_window.h:30-45) are display only.
So the README is true of Hydra's rating model. The K2 drift is REFUTED. What stands: two engine homes for 85 ms (kHitWindowMs and kWindowBackMs), described in a header comment as on purpose, citing a ruling I did not find in a Confirmed line (D270 not opened; it is in the decision log). Python copy: tools\ch_probe\constants.py:89-92,227,241-243 holds 85, 37.5, 171.4313, 96.2932 (K1). I did not re-search tests for a link between constants.py and note_window.h.

## 3. Copies with no test between them (K1)
- Picture limits: CONFIRMED. tools\doc_shots\shrink.py:17-21 (500*1024, 4*1024*1024) and tests\test_docs_match_code.cpp:2022-2025 (same numbers). Each comment names the other; neither reads the other.
- "Clean time plus 5 s": CONFIRMED. tools\mutate\run.py:89-100 (rule and TIME_LIMIT_RULE), run.py:967 (a print-string literal "(each run's clean time plus 5 s)"), docs\development.md:732-734 (states the rule while saying its one home is run.py). I count three live homes, not the finder's four. Tests pinning 5.0 not opened.
- Three-builds rule: CONFIRMED. Owner H\tools\build_slot.ps1:75 `$BuildSlotCount = 3`. Prose restates it: gates\README.md:15, brief-preamble.md, CLAUDE.md and agent-rules rule 14 ("At most three builds"). No test between them seen.
- Capture 12 s wait: CONFIRMED. tools\capture_window.ps1:56 `$Wait = 12`; tools\capture\report_shot.ps1:10 and steps.ps1:8 pass a literal `12`.

## 4. Tests pin literals (K1): CONFIRMED
tests\test_precheck_git.cpp:200-202 pins `RangeSize{400,0}` and "over the 400-line threshold". tests\test_precheck_pick.cpp:75-77 pin `kCommonNameLimit == 30`, `kPickSizeLimit == 40`, `kHeaderIncluderLimit == 20`. Changing an owner means editing these tests. Not opened: test_precheck_pick_exe.cpp strings.

## 5. Two readers of settings.ini (K6): CONFIRMED (arguable)
machine_report.py:49-52 says the lines "sit in [video], so game_files.get_game_setting, which reads [game], cannot". Two code readers of one file with a stated reason; they read different sections. I did not open game_files.py.

## 6. SEARCH_NEXT_ITEM = Blue guess (K4): CONFIRMED
menu_tools.py:48 "So Blue is a guess: live step LS-K3...". Only a live run on the real menu settles it; stays open.

## 7. Prose doing a hook's job (K5): CONFIRMED
gates\README.md:15 explains the build-pass refusal in prose and quotes the ruling; brief-preamble.md also covers builds. The hook (percall.py) is the owner.

## Other lines
- Scan cap comment (quiet.py:43, "The user's ruling of 2026-11-07: how long the folder scan may run", SCAN_CAP_S = 3.0): I grepped "2026-11-07" in all docs\agents\rulings\*.md. Hits are only agents-and-workflow.md:8, :9, :34, :35, none about a scan cap. No Confirmed line backs the comment. Cards and question files not searched. Open.
- agents-and-workflow.md:8 (Confirmed): "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." It names the limit and who builds; no split involved.
- ui-and-display.md:27 (2026-10-30) is about scrolling Hydra's library for minutes; menu_tools.py:39 `SEARCH_DOWN_CAP = 10` scrolls Clone Hero's list. Whether it covers that is for the main session. I did not check that line 27 sits under Confirmed.

## Summary
Checked 7. CONFIRMED 5. CONFIRMED-CHANGED 2. REFUTED 0 whole (the README drift part is refuted). Splits added 2 (plan line 303; SPEC.md:302 copy). Not reached: none; homes not opened are listed above.
