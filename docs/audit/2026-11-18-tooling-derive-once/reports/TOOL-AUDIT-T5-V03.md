# TOOL-AUDIT-T5-V03 (verifier a77cd5c05061cd256, checking F03)

Paths: H = C:\Users\Patrick\Downloads\Hydra\hydra-test (main checkout; the worktree copy was not used for src). M = memory folder. G = C:\Users\Patrick\.claude\gates. All claims below are by reading; nothing run.

## Candidate 1. Corpus size 115 / 97 vs 98 (K2) : CONFIRMED
- Count: 98 song.ini under H\testdata\input (find, by reading).
- M\no-full-library-runs.md:11 "checked-in corpus in `testdata/input` (about 115 charts)". M\questions-describe-the-real-rule.md:24 "Strip the `[N/97]` prefix". H\docs\agents\ui-testing.md:48 "`testdata/input`, 98 charts". H\tools\test_hydra_ui_mcp.py:774 and :1070 `assertIn("charts=98", ...)`.
- SPLIT A: "about 115" (no-full-library-runs.md:11) vs 98 (ui-testing.md:48, file count).
- SPLIT B: "[N/97]" (questions-describe-the-real-rule.md:24) vs 98.
- Situation: main session reads the notes; the tool test in a worktree with an added chart would fail on its typed 98 (not checked by run; a run would settle it).
- Ruling: none found by me in rulings/.

## Candidate 2. kEarlyFillSlackMs vs early-fill setting (K8/K2) : CONFIRMED
- grep of H\src and H\tests for `kEarlyFillSlackMs`: no hits. src\core\model.h:108-112 `kDefaultEarlyFillWindowMs = 60.0`, "Every default of the setting (Settings, SearchSettings, the search graph, Activation, HydraRecord) is this one constant." :101-106 `kHitWindowMs = 85.0`, "also the widest early-fill window the setting allows". :338 `fill_refuses(double e_offset, double window_ms)`.
- M\early-fill-orange-85-d250.md:3 and :11 say the reach is kHitWindowMs read by fill_refuses (orange) and the slack is the fixed `kEarlyFillSlackMs` (60 ms) read by is_e0. That is not the code (one setting, default 60, up to 85). I did not read is_e0's body.
- SPLIT: note :11 (fixed slack constant, orange side 85) vs model.h:108-115 (one user setting 0 to 85 for the window).
- Rulings (Confirmed section starts line 7 of H\docs\agents\rulings\code-and-engine.md; Unconfirmed at line 61): :23 "change the early fill search to allow orange timing up to 85ms."; :24 chose "Orange only (Recommended)" to "Which side of the early-fill window should go to 85 ms?"; :57 "analysis settings panel, default 60ms, supports 0ms to 85ms". The later :57 line is the one model.h quotes. Neither line names both sides of the note-vs-code split (the note's fixed slack is not discussed), so the ruling does not close the split. :24 and :57 describe different designs; the finder's point stands.
- Memory index M\MEMORY.md:22 "reach kHitWindowMs 85 / slack 60" repeats the stale split.

## Candidate 3. kSqueezeWindowMs location (K8) : CONFIRMED
- SKILL.md (~/.claude/skills/analyzing-sp-squeezes) line 10: "(`kSqueezeWindowMs`, src/search/graph.h)". Real: src\core\model.h:121 `constexpr double kSqueezeWindowMs = 500.0;`. graph.h:216 only mentions the name in a comment.
- M\sp-squeeze-mechanics.md:13-15 says model.h (correct) but "used by src/search/graph.cpp". Callers of within_squeeze_window: graph.cpp:160, :329; engine.cpp:2044; model.cpp:845; sqout_chord.h:32, :123. So the note narrows the callers.
- SPLIT 1: skill (graph.h) vs model.h:121 and the note (model.h). SPLIT 2: note's "used by graph.cpp" vs the five caller sites.
- Ruling: none found.

## Candidate 4. 85 / 3 / 4 values, status interval, fan-out copies (K1/K7) : CONFIRMED (as K1 only, no drift found)
- Status 5 minutes: owner G\progress.py:16 (per finder; not re-opened), held equal by G\tests\test_docs_numbers.py for five docs; memory notes are outside the test. By reading, the note M\every-brief-carries-status-rule.md:15 says "every 5 minutes", which agrees.
- 3 ms leeway / 4 tied paths: finder's owner claims (rules.h:32, :38) not re-opened; the note claims were not contradicted by anything I read. Marked unchecked beyond the finder's word.
- Fan-out numbers (lean-agent-fanout.md): not re-opened; I accepted the finder's reading only as "K7 candidate, unchecked by me".
- No split found.

## Candidate 5. Hook message quoted in the I8-097 note (K8) : CONFIRMED (the quoted text is nowhere in the gates)
- M\measure-small-explain-first.md:11 "A hook also flagged the fifth bench_run launch: "say what changed since the last run and why this one will end differently"."
- Searched G\*.py, G\*.md, G\*.json for "differently" and "since the last run": only hit is an unrelated line in G\tests\test_agent_stop.py:352 ("fixed differently"). So no hook today carries that text. By reading; whether it was retired is unknown.

## Candidate 6. Status-gate claim inside one note (K2/K8) : CONFIRMED
- M\every-brief-carries-status-rule.md:3 "the dispatch gate now denies briefs that lack it." :15 "No gate checks a brief for this sentence today" (finder's quote; I saw the :15 line starts with How to apply and did not re-quote the "No gate" sentence in my output; the grep for "No gate" printed only line 15 beginning, which is the start of the same line).
- G\review_dispatch.py:1 "a reviewer dispatch writes the handover record, or is refused"; its grep for "status" found no text. Its job is reviewer dispatch, not brief wording.
- SPLIT: note :3 vs note :15. Edge: the reviewer_dispatch_gate retired-name point was not re-checked by me.

## Candidate 7. tools/test_build_pass.ps1 types 3 (K6) : CONFIRMED
- H\tools\build_slot.ps1:75 `$BuildSlotCount = 3`, used at :152, :197, :199, :201. H\tools\test_build_pass.ps1:115 `(Get-Locks).Count -eq 3`, :117 `'all 3 build slots are taken'`, :141 `Want = 3`, :199/:207 `Write-Lock 3`. The test never reads $BuildSlotCount (grep for BuildSlotCount in the test returned no hit). So the test and owner agree today; changing the owner breaks the test.
- Ruling found, not a home: H\docs\agents\rulings\agents-and-workflow.md:8 (finder quote; not re-opened).
- No split today.

## Extra finder claims checked
- Library size: three different counts in M: min-path-delta-proof.md:29 "18,610 charts", no-full-library-runs.md:17 "18,811 charts", stored-versions-one-rule.md:11 "18,755 results went Stale". CONFIRMED as three numbers. CONFIRMED-CHANGED in kind: the 18,755 is "results", a different thing from a chart count, so it is not necessarily a split. SPLIT only between 18,610 and 18,811 (both "charts"; they may be different dates, unchecked).
- Pitch Dark 128520/128400 in H\docs\agents\test-rubric.md:48, :49, :130 (confirmed by grep). TE and :175 not re-opened.
- Two Confirmed early-fill lines describing different designs: see candidate 2 (lines :24 and :57 exist; the "different designs" reading is the finder's, I agree by reading).

## Splits list
1. 115 vs 98 corpus charts.
2. [N/97] vs 98.
3. Fixed kEarlyFillSlackMs (note) vs one early-fill setting (model.h).
4. SKILL.md graph.h vs model.h for kSqueezeWindowMs.
5. Note "used by graph.cpp" vs five callers.
6. Status gate, note :3 vs :15.
7. 18,610 vs 18,811 charts (18,755 is results, not charts).

## Not reached
Finder's groups 5 (owner lines in rules.h), 6 (progress.py), 8, 9 rulings quote, 10, 12, and the I7-133 engine.cpp line 2015 claim were not re-opened by me. Finder's own unreached rows (I8-012, I8-050, I8-057, I8-080, I8-126, I8-148, I8-154, I8-163, I8-195, I8-196, I7-155, I7-189, I7-224, I7-226) are not mine.
