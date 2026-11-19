# TOOL-AUDIT-T8-R5 (theme 20, second half): 23 ids, all reached

Paths: H = C:\Users\Patrick\Downloads\Hydra\hydra-test, G = C:\Users\Patrick\.claude\gates, M = C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory.
Tally: REAL 8 (L-F20-01, L-F20-07, L-F22-13, L-F26-02, L-F26-15, L-F28-06, L-F30-10, L-F31-14); GONE 2 (L-F20-06, L-F30-23); NOT A SPLIT 13.

### L-F19-12
**Verdict**: NOT A SPLIT.
**Homes read**: M/plans-reviewed-before-shown.md:15 "the gate only catches ExitPlanMode and Artifact, so a plan pasted into chat slips past it. The review rule still applies." H/docs/agents/doc-review.md:17 "Internal docs (... committed plans) and questions to the user are not gated." G/publish_gate.py:112 `if tool == "ExitPlanMode" and inp.get("plan")` is the only plan check.
The doc says which plans a hook checks. The memory says a plan must be reviewed whatever the hook checks, and names the hook gap itself. CLAUDE.md "Only what outsiders read, and plans, get a review" agrees with the memory. Hook gap: a plan pasted in chat has no hook behind it (publish_gate.py:112 only).

### L-F19-14
**Verdict**: NOT A SPLIT.
**Homes read**: H/docs/agents/integrate.md:43-44 `-tc="single-owner*"` and `-sf=*docs_match_code*`; H/docs/agents/agent-rules.md:7 `"-tc=docs match code*"` and `"-sf=*single_owner*"`. I searched the tests: tests/single_owner/*.cpp cases all start `single-owner`; tests/test_docs_match_code.cpp has 20 cases, 19 start `docs match code`, one is `code lexer: ...` (line 2174).
Each spelling names real tests and picks nearly the same set (the docs file filter also takes the one lexer case). Nobody would act differently in practice.

### L-F20-01
**Verdict**: REAL (weak).
**Homes read**: H/docs/agents/ui-testing.md:46 "The 98 test charts"; :48 "testdata/input, 98 charts". H/docs/agents/agent-rules.md:14 "testdata/input (about 115 charts)". Code: tests/test_engine_sweeps.cpp:656 `CHECK(charts == 98)`; tests/ui/uitest_library.cpp:589 `shown() == 98`. Files today: 116 .chart/.mid files in 102 folders under testdata/input.
**Kind**: doc-vs-doc (98 is code-backed; "about 115" counts files).
**What changes**: a reader sizing the corpus gets 98 or about 115 for "charts"; only the unit differs, so little else changes.

### L-F20-04
**Verdict**: NOT A SPLIT.
**Homes read**: C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:55 "a run that beats the engine's optimal is a version problem or a bug, never a real score". H/docs/agents/public-doc-style.md:17 "Rows above optimal are expected: many leaderboard scores were set on older Clone Hero versions".
The skill is a checklist for judging one video run whose game version is read off the results screen; it names "version problem" as a cause. The style page words a leaderboard row for a player. Same cause named, different jobs.

### L-F20-06
**Verdict**: GONE.
**Homes read**: G/agent_stop.py:172-174 `documents(git_dir, base, tip)` "base, the start the dispatch recorded"; G/README.md:49 "from the `base` the handover recorded to the new tip; it never asks git about `main`". M/hooks-never-guess.md:13 names the old merge-base as the fault. Code, README and memory now agree on the recorded base. (The remaining `merge-base` calls, agent_stop.py:152 and :159, are ancestry checks on the branch, not the document start.)

### L-F20-07
**Verdict**: REAL.
**Homes read**: H/docs/agents/test-rubric-examples.md:9 "The Preview fix those four asked for has since landed. The Preview's own refill-tick function is gone"; same file :330-336 "It cannot hold one today ... the gauge disagrees with the record today ... the test to write in the same change as the Preview fix ... Today it would fail on Pitch Dark". Code: `refill_tick` is gone from src/ and tests/ (grep; it survives only in docs and audit files).
**Kind**: doc-vs-doc (same page); also doc-vs-code for the :330-336 text.
**What changes**: whether the display test is still to be written alongside a fix to come, or the fix has landed and the "would fail today" text is stale.

### L-F22-13
**Verdict**: REAL.
**Homes read**: H/.derive-once-gate:2-3 "Which paths count is decided by Test-DeriveOnceGatedPath in the user-scope hook (hooks\lib\derive_once_rules.ps1)". C:\Users\Patrick\.claude\hooks holds only `state`; a grep of the hooks repo and Hydra finds that name nowhere else. G/review_rules.py:21-22 BUILTIN_CODE, used by `_is_code` (:182-187) when the marker has no `gate:` lines.
**Kind**: moved-pointer (marker names a missing function; the deciding code is `_is_code` in review_rules.py).
**What changes**: someone sent to the marker's pointer for the gated-path rule finds nothing; the real rule is the regex in review_rules.py.

### L-F24-11
**Verdict**: NOT A SPLIT.
**Homes read**: H/docs/agents/audit-method.md:50 finder reports "a table of inputs and answers for each home, with the edges in". worktrees\tooling-audit-records\docs\audit\2026-11-18-tooling-derive-once\briefs\T5-finder.md:33-41 "a table of what each home tells its reader to do, with these edges in".
The method page is the general rule for code audits (values in, answers out). The T5 brief is this one tooling audit's finder brief, built for homes that are docs and hooks.

### L-F25-19
**Verdict**: NOT A SPLIT.
**Homes read**: tools/ch_mcp/build_facts.py:69-72 `current_build` "The `version` the game ... says it is"; tools/ch_mcp/dump_facts.py:381 `DEFAULT_BUILD = "6142"` (used :643 for a page with no build heading); tests/test_docs_match_code.cpp:719 `kBuild11 = "6142"`; tools/ch_mcp/launch_patch.json:3 `"build": "1.1.0.6142"`. All name 6142 today.
They answer different questions: which build is installed, which build an unlabeled facts page means, which build the patch fits. No two answer one question with different values today. (Repeated literals exist; they do not disagree.)

### L-F26-02
**Verdict**: REAL.
**Homes read**: H/docs/agents/audit-method.md:97 "Give one example from each end, and one from the gap where the two sides disagree." H/docs/agents/doc-review.md:61 reviewer checks "Each option has a concrete example from each end." M/questions-describe-the-real-rule.md:16 "one example from each end of its range".
**Kind**: doc-vs-doc; memory-vs-other.
**What changes**: a reviewer following doc-review.md passes a decision question with no gap example; one following audit-method.md sends it back.

### L-F26-15
**Verdict**: REAL (the memory side only).
**Homes read**: M/use-source-facts-not-proxies.md:13 "Song length now comes from the chart's own metadata ... The last Expert drum note is only a backup when the metadata is missing" (D75). H/src/app/song_length.h:1-12 and :50-55: one owner, "D75, as amended by D255", three candidates (stated length, end event, "the last note's end (Song::last_note_end_tick)" over every instrument) plus `kSongEndPadMs`. The tool side is now one path: tools/ch_mcp/bot_run.py:387-398 and install_chart.py:99 call `hydra_end.chart_song_length_ms`, so the first-line against last-line `song_length` difference and trace_hits.py:68-69 no longer exist as the row describes.
**Kind**: memory-vs-other (stale note against code); doc-vs-code.
**What changes**: the note tells a session the fallback is the last Expert drum note and no end event or pad exists; the code uses the end event, the any-instrument last note and a 1000 ms pad.

### L-F27-23
**Verdict**: NOT A SPLIT.
Same as L-F25-19: build_facts.py:69-72 (installed version), dump_facts.py:381 "6142", launch_patch.json:3-4 "1.1.0.6142", test_docs_match_code.cpp:719 "6142". Same build, three different questions.

### L-F28-06
**Verdict**: REAL.
**Homes read**: G/wake.py:5-6 "The design is the Hydra repo's docs/superpowers/plans/2026-11-16-agent-supervisor.md (`git show 80b310a9:docs/superpowers/plans/2026-11-16-agent-supervisor.md`), task W1"; G/stall.py:11 cites it too. H/docs/superpowers/plans/ on main has no such file (listing checked; `git ls-tree -r main` finds none). The file exists only on branch `plan/agent-supervisor` and in worktree H/.claude/worktrees/plan-agent-supervisor/docs/superpowers/plans/2026-11-16-agent-supervisor.md. I did not find check.py:34-35 (no check.py in G).
**Kind**: moved-pointer (the plan lives on an unmerged branch).
**What changes**: a reader opening the cited path on main finds nothing.

### L-F29-11
**Verdict**: NOT A SPLIT.
**Homes read**: H/docs/superpowers/plans/2026-10-25-hooks-from-scratch.md:11 "The new system does five jobs"; G/README.md:3 "The hooks do six jobs ... where that plan and the code differ, the code wins." The README lists six (it adds long shell output). One is the earlier design record, the other the current description, and the README says which wins.

### L-F30-10
**Verdict**: REAL.
**Homes read**: H/docs/agents/doc-screenshots.md:23 "Run it before each release ... Also run it after any change to a label or a layout that one of the pictures shows." H/docs/development.md:977-983 "Every release: refresh the screenshots ... Run it before each release".
**Kind**: doc-vs-doc.
**What changes**: after a label or layout change between releases, one page says refresh the pictures now, the other only at release.

### L-F30-11
**Verdict**: NOT A SPLIT.
**Homes read**: H/docs/agents/public-doc-style.md:5 "check every name and number against the code before you write it"; :77 "A release note describes the app as it was at that release ... names of things at the time". Line 5 covers writing for what ships; :77 covers rewriting an old note. Different cases on one page.

### L-F30-13
**Verdict**: NOT A SPLIT.
**Homes read**: H/docs/agents/audit-method.md:98 "give a recommendation and the strongest case against it"; M/counts-on-screen-must-agree.md:16 "recommend one, then fix it" and :21 "Never recommend keeping one side 'unchanged' or 'as today'". Both recommend; the note only bars recommending "keep a side as is" when code already disagrees with itself.

### L-F30-23
**Verdict**: GONE.
**Homes read**: H/docs/agents/issue-tracker.md:20,40 name `/triage` and `/wayfinder`. They exist: C:\Users\Patrick\.claude\plugins\cache\claude-plugins-official\mattpocock-skills\1.3.1\skills\engineering\triage and ...\wayfinder. Only ~/.claude/skills lacks them; they come from the plugin.

### L-F31-14
**Verdict**: REAL.
**Homes read**: H/docs/agents/test-rubric.md:45 "Agents may not run `hydra_replay` (agent rule 14), so an auditor asks the main session for the output". C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:18 the tool "gets the path from `hydra_replay dump` ... locate the asked-about moment from that alone"; it does not say who may run it. H/docs/agents/agent-rules.md rule 14 bars agents from `hydra_replay.exe`.
**Kind**: doc-vs-doc (skill against rule 14 and the rubric).
**What changes**: an agent that follows the skill runs squeeze_scout, which runs hydra_replay; an agent that follows rule 14 asks the main session.

### L-F31-15
**Verdict**: NOT A SPLIT.
**Homes read**: M/derive-display-from-engine-truth.md:23 "every SP event tied to a squeeze is drawn at that chord ... the nominal end ... is not where the squeeze happens"; SKILL.md:22-23 "A SqOut gap is measured against the pre-collection end, not the final deactivation node". One answers where a squeeze is drawn on a highway; the other which SP end a gap is measured from.

### L-F31-23
**Verdict**: NOT A SPLIT. Same finding as L-F20-04.
**Homes read**: video-tools README.md:41-43 and SKILL.md:55 "a version problem or a bug, never a real score" (judging one video, version read off the results screen); M/dmleaderboards-comparison.md:26-29 "Above-optimal scores are NORMAL ... older Clone Hero version ... Just label them". Both name the older game version as the cause. One is a check on a video run, the other how to label leaderboard rows.

### L-F31-25
**Verdict**: NOT A SPLIT.
**Homes read**: C:\Users\Patrick\.claude\agents\planner.md:13 "A promise with no task ... give it a task or move it to an 'Out of scope' list for the user to approve" (this file is in the hooks repo, not H/docs/agents); M/search-web-before-finalizing-plans.md:15 "Do not leave a 'left alone' list of related defects; either fold them into the plan or ask." Both end at the user: the planner's list is the ask; the note bars quiet lists of related defects. Different items (unproven promises against related defects).

### L-F32-06
**Verdict**: NOT A SPLIT.
**Homes read**: M/names-from-data-not-recall.md:15 "Every proper name in a deliverable must trace to data in hand (files, DB, web lookup)"; SKILL.md:63 same; H/docs/agents/derive-once-review.md:76 "Every name comes from the repo, not memory" (inside the reviewer's rules about code and ground truth). One covers song and band names in deliverables, the other the names a code reviewer cites.

## Not reached
None.
