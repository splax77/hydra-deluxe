# TOOL-AUDIT-T7-C22 (closure checker, batch item 21: sorting S11, ledger L11)

Short names: HY = C:\Users\Patrick\Downloads\Hydra\hydra-test (main checkout, current files). AR = HY\docs\agents. MEM = C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory. G = C:\Users\Patrick\.claude\gates. Line numbers are today's.

Totals: PASS 24, REOPEN 1 (L-F32-10), GONE 0. Not reached: none.

### L-F31-02
**Verdict:** PASS (P)
**Homes read:** AR\agent-rules.md:41 ("CONTEXT.md decides what a project word means"), :43 ("Before any agent calls a project term wrong or unclear, it checks how the code already uses the term... If a definition's text and the code's usage disagree, that is a question for the user... changes neither"); AR\derive-once-review.md:51; AR\brief-preamble.md:99; AR\domain.md:40-45 ("use the term as defined in CONTEXT.md").
**Why:** All four say the same order: CONTEXT.md decides, check the code's usage first, a disagreement goes to the user with neither side changed. domain.md says less but does not conflict. Replacing the copies with a pointer to agent-rules.md:41-43 changes no behaviour.

### L-F31-04
**Verdict:** PASS (P)
**Homes read:** MEM\derive-display-from-engine-truth.md:24 ("write a table of each question and the side that owns it before dispatch"); AR\derive-once-review.md:53 (item 10, "a table with four columns" per value crossing a boundary).
**Why:** One is a planner's table before dispatch, the other a reviewer's table after. Different actors and shapes, no clash of values or rules.

### L-F31-05
**Verdict:** PASS (P)
**Homes read:** AR\test-rubric-examples.md:270-272 ("multiplier_step calls it for category_scores (:46) and for the multiplier squeeze (model.cpp:369)"); HY\src\core\model.cpp:366-369 (MultSqueeze::applies calls crosses_multiplier_step); HY\src\core\timing.cpp:12-27 (crosses_multiplier_step calls multiplier_step, which calls to_multiplier); HY\src\core\scoring.cpp:46 (multiplier_step), :56 (to_multiplier). Searched src/app, src/ui, tools for multiplier_step and to_multiplier: no hits.
**Why:** The claim is true by one indirect hop and the "no caller outside core" claim holds. Side note for whoever fixes the page: the cited scoring.cpp:57 and "combo += 1 on :56" are now :56 and :55, a small stale line number, not a disagreement.

### L-F31-07
**Verdict:** PASS (B)
**Homes read:** MEM\ch-disco-noflip-rule.md (description and body: Clone Hero "ignoring drums0dnoflip", "Hydra's reading (flip off)"); HY\docs\clone-hero\02-note-list.md:253-260 (NL-21: counts as a span start, adds only a no-flip flag, an earlier drums0d keeps flipping); HY\src\parse\song.cpp:326-335 (comment: "Clone Hero turns flip on for it... Hydra keeps it off on purpose (D19)"; `is_disco_off_marker` returns true for "dnoflip").
**Why:** The only difference is the memory note's loose wording against the page and the code. Hydra's value (flip off) agrees in all three. Every split is a memory note against a doc and code.

### L-F31-08
**Verdict:** PASS (C)
**Homes read:** HY\docs\clone-hero\02-note-list.md:611 (light_drum_lanes 203, lights_lane_pad 88, game_lane_pads 147; "lines 159 to 165", "line 166"), :631 ("lines 203 to 213, called at line 2009", "line 213"), :641 ("lines 203 to 213", "lines 200 and 201"); HY\src\parse\song.cpp: lights_lane_pad :90, game_lane_pads :149, light_drum_lanes :205 (body to ~:220, `every_lane_pad` use at :215), call at :2204 inside finish_loaded_song area; MEM\drum-lanes-d112-2026-10.md:11 (called once in the loader tail).
**Why:** Pure moved line numbers and the current ones are clear. The ledger lists only three function lines and the call line; the page has more shifted numbers in the same entries (159-165, 166, 213, 200-201 each moved by about two). The fix must re-read all of them, but they are all moved-line citations.

### L-F31-09
**Verdict:** PASS (B)
**Homes read:** MEM\check-an-edge-case-can-happen.md:11,15 ("Planner and reviewer briefs say this too"); C:\Users\Patrick\.claude\agents\ (card-writer, executor, planner, reviewer, scout); AR\*.md. Searched all of those for "edge case", "edge-case", "can happen", "cannot happen", "impossible", "real chart that/can": only public-doc-style.md, test-rubric.md:29 and two ruling lines match, none state the rule.
**Why:** The note claims copies that do not exist in the agent files or the agents docs. A memory note against agent files.

### L-F31-10
**Verdict:** PASS (P)
**Homes read:** MEM\fc-video-workflow.md:11,15; C:\Users\Patrick\Downloads\Hydra\video-tools\README.md:39 ("Eleven things to check"), :41-43 (item 0 "Report the headline the moment you have it"), :56 (item 5 "Never re-encode or rewrite the video for any reason").
**Why:** The note's pointer is true, the README holds both rules and no home says anything different. Only trigger wording differs and nothing chooses between them.

### L-F31-11
**Verdict:** PASS (C)
**Homes read:** C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:34 (names memory `gameplay-video-score-analysis`); MEM listing (no such file); MEM\archive\gameplay-video-score-analysis.md (exists, header: "this method is now a tool, see fc-video-workflow", step 1 is the alignment method).
**Why:** The note moved to memory\archive. The current place is nameable: memory\archive\gameplay-video-score-analysis.md (with MEM\fc-video-workflow.md for the tool that replaced it). Caveat for the fixer: which of those two the skill should point at is a wording pick, not a rule.

### L-F31-12
**Verdict:** PASS (P)
**Homes read:** G\percall.py:143-144 ("Agents never build, so hydra_build_pass is refused to every agent... the main session builds your branch and runs any test that needs a build"); AR\agent-rules.md:20 (rule 14) and :27 ("nothing checks rule 14's line on tests").
**Why:** The hook text restates rule 14 and agrees. Only the line numbers moved (ledger said 108-110).

### L-F31-13
**Verdict:** PASS (P)
**Homes read:** AR\doc-screenshots.md:19; AR\ui-testing.md:17,27,151,348,362,364; AR\agent-rules.md:7,20; HY\CLAUDE.md rule 14; AR\brief-preamble.md:57; C:\Users\Patrick\.claude\agents\executor.md:11, reviewer.md:21; MEM\merge-tests-touch-changed-files.md:15; G\percall.py:143-144. Searched card-writer.md for build/test rules: none.
**Why:** Every home says the main session builds and runs tests that need a build, agents do neither, and agents may run hydra_precheck from the main checkout only. ui-testing.md:364 is about which checkout the hydra MCP runs, a different subject, not a clash.

### L-F31-16
**Verdict:** PASS (P)
**Homes read:** AR\test-rubric-examples.md:265-266,280 and HY\docs\clone-hero\06-scoring.md:53-57 (10/20/30 steps agree) with HY\src\core\timing.cpp:12-17 (to_multiplier: <10 ->1, <20 ->2, <30 ->3, else 4); AR\test-rubric.md:17-25 (rule number vs snapshot number, no 10/20/30 copy); AR\owner-map.md:35-45; planner.md:19; reviewer.md:17; AR\brief-preamble.md:111; AR\domain.md:11,47-50.
**Why:** The multiplier steps agree with the code. planner.md, reviewer.md and owner-map.md say the same about using each owner as it is. brief-preamble.md:111 calls owner-map.md "the one home" while restating the rule, a copy, not a conflict. domain.md's "proceed silently" is about missing CONTEXT/ADR files, not about problems, so it does not clash with fail-loudly rules. I did not re-read every remaining home in the ledger row (memory dmleaderboards note, issue-tracker.md, handoff.md); those were not opened, so this PASS rests on the homes above.

### L-F31-17
**Verdict:** PASS (P)
**Homes read:** AR\test-rubric.md:189 (calibration stage applies, ban on the two files); AR\audit-method.md:44,46 (calibration; stop and report a miss to the user); AR\test-rubric-examples.md; HY\tools\mutate exists.
**Why:** test-rubric.md defers to audit-method.md's calibration stage and says nothing different. No split.

### L-F31-18
**Verdict:** PASS (P)
**Homes read:** AR\owner-map.md:31 (names SPSqueeze::needs_no_timing as the counter-example, quoting "not this one"); HY\src\core\model.h:430 ("end the note sits on is core::paid_by_sp_walk's question, not this one"), :290, :301; HY\src\core\backend_value.h:26-28 (`paid_by_sp_walk`, offset <= 0), :30-34 (`after_sp_end` by tick, "agrees with paid_by_sp_walk wherever time runs forwards").
**Why:** The owner claim holds. The offset form and the tick form give the same answer by their own comments. No disagreement in value.

### L-F31-19
**Verdict:** PASS (P)
**Homes read:** AR\premise-map.md:78 (pointer to clone-hero-mcp.md); AR\clone-hero-mcp.md:58 (holds the go-request content and "No hook checks this message"); planner.md:13; reviewer.md:13.
**Why:** The pointer is true and the content has one text home. No disagreement.

### L-F31-20
**Verdict:** PASS (P)
**Homes read:** AR\integrate.md:56 (head hash and range) and :58-60 ("Your report": branch, worktree, head hash, each merge hash, owners gained, folds, conflicts, precheck output, stops); AR\brief-preamble.md:125-127 ("Your brief says what to return. If it does not, return: ...").
**Why:** The preamble list is the default only where the brief gives none, and integrate.md names its own list. The two lists do not contradict (integrate's is a specialised version). Pointer fix changes no behaviour.

### L-F31-21
**Verdict:** PASS (P)
**Homes read:** AR\this-machine.md:95 (one text home, names `build_path_buttons()` and `build_activations()`); HY\src\app\path_view.cpp:225 (`build_activations`), :529 (`build_path_buttons`). Searched AR for other "stored number" rules: none.
**Why:** One text home, pointer true.

### L-F31-24
**Verdict:** PASS (P)
**Homes read:** MEM\check-an-edge-case-can-happen.md:11 ("activation needs 2 bars and SP running out leaves 0 or 1", names `branch_activate` and `create_deactivated_path`); HY\src\core\timing.h:51 (`kSpActivationBars = 2`), :268 (`kMeasuresPerSpBar = 2`, a different fact); HY\src\search\engine.cpp:601,605 (both names exist), :1106 (`c.sp_bars = sq ? 1 : 0;`).
**Why:** The memory copy of the value agrees with the owner and both named functions still exist.

### L-F31-26
**Verdict:** PASS (P)
**Homes read:** MEM\counts-on-screen-must-agree.md:18 ("One owner per numbering"); HY\docs\adr\0030-hydra-owns-every-live-command-rule.md:50 (script mode: the interpreter; live mode: only the helper numbers a hydra_run script's lines; no numbering from Hydra) and :98; HY\tools\hydra_ui_mcp.py:694,708 ("stopped at line N of M"); HY\tests\ui\uitest_commands.cpp:432 ("!! failed at line %d"); AR\ui-testing.md:97,229; AR\doc-screenshots.md:13.
**Why:** The ADR states which side numbers in each mode and the code and docs match it. The memory line names no owner but contradicts nothing.

### L-F31-27
**Verdict:** PASS (C)
**Homes read:** C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:10 (`kSqueezeWindowMs`, src/search/graph.h, 500 ms); HY\src\core\model.h:121 (`constexpr double kSqueezeWindowMs = 500.0;`), :130-134 (strictly inside the window); HY\src\search\graph.h:216 (mentions the name in a comment only); MEM\sp-squeeze-mechanics.md:3,13 (500 ms).
**Why:** The skill names the wrong file; the definition is in src/core/model.h:121 and the value 500 agrees everywhere. Current place is clear.

### L-F32-02
**Verdict:** PASS (P)
**Homes read:** HY\tools\build_slot.ps1:75 (`$BuildSlotCount = 3`), :80 (`$BuildPassUnclaimedSeconds = 120`); AR\agent-rules.md:20 (rule 14, "at most three builds", cites the 2026-11-07 ruling); AR\brief-preamble.md:57; HY\CLAUDE.md:76; HY\docs\development.md:114-144 (three builds, one pass per session, limit only when CLAUDECODE is 1); AR\rulings\agents-and-workflow.md:8, :35 (120 s, in an Unconfirmed line).
**Why:** Every number agrees with build_slot.ps1. The CLAUDECODE scope appears only in development.md, but rule 14 opens "these hold for the main session and every agent it sends out", so it is not a contradiction, only a missing scope note. I read these homes; I did not re-open percall.py:19, hydra_ui_mcp.py, new_worktree.ps1, compare_db.py or mutate/run.py (not reached), so those copies are unchecked.

### L-F32-04
**Verdict:** PASS (P)
**Homes read:** MEM\min-path-delta-proof.md:22-23 ("MultSqueeze::gain() is display-only; its one consumer is multsqueeze_views in src/app/path_view.cpp"); HY\src\core\model.cpp:455; HY\src\core\scoring.cpp:148; HY\src\app\path_view.cpp:136,141,144,161. Searched src, tools, tests for `.gain()` / `gain()`: only path_view.cpp:141,144 call MultSqueeze::gain (the others are audio and transport gains).
**Why:** The note matches the code.

### L-F32-05
**Verdict:** PASS (P)
**Homes read:** C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:32-33 (2x kick pitch 95 counts in 2x Bass mode; a note at the phrase-off tick is excluded); HY\src\parse\song.cpp:422-439 (`close_sp_phrase`, "start <= t < end", closes before that tick's chord), :888-892 (`is_handled_note`, kick2x), :974; C:\Users\Patrick\Downloads\Hydra\squeeze-tools\squeeze_scout.py:330-349 (pitches 96-100, 95 when two_x; half-open phrase, "NOT part of the phrase").
**Why:** Skill, tool and the owner in song.cpp say the same. The skill only lacks a pointer to the owner.

### L-F32-08
**Verdict:** PASS (B)
**Homes read:** MEM\no-full-library-runs.md:15 ("No hook enforces it yet."); G\percall.py:14 (`LIBRARY_FLAG = "--whole-library"`), :45-55 (`_library_refusal`: a shell call holding the flag with no unused typed allow is refused).
**Why:** The note is stale against the hook. Hook gap, for the main session: percall.py keys only on the flag text in the shell command, so a run that reaches the whole library without that flag may not be refused; I did not run anything to settle it.

### L-F32-09
**Verdict:** PASS (B)
**Homes read:** MEM\questions-describe-the-real-rule.md:11 ("Agent rules 11 to 13 hold the 'You will see' list and the visible-change rule"); AR\agent-rules.md:17 (rule 11, the "will see" list), :18 (rule 12, the visible-change rule), :19 (rule 13, uncommitted edits).
**Why:** The note names one rule too many; the content it cites is in rules 11 and 12. A memory note against a doc.

### L-F32-10
**Verdict:** REOPEN (should be a question, category Q)
**Homes read:** MEM\relay-status-in-plain-english.md:11,15; HY\docs\sessions\2026-11-16-e816e44f.md:24; AR\rulings\*.md (searched "Claim, plus" and "disagree"/"relay": only agents-and-workflow.md:26 and unrelated lines); C:\Users\Patrick\.claude\model-postures.md:11; G\stall.py:26-33 (STATUS_MAX_LINES, STATUS_HEADER); searched stall.py, progress.py, common.py for a "test_" or fixture skip: none.
**Why:** "Note is wrong" does not hold for the whole row. The note says the user chose "Claim, plus disagreement" on 2026-11-16, and that is true: the session card records `chose "Claim, plus disagreement" to "Relay wording: your filed words are ..."` (23:19 UTC). So the note is not wrong about the user's choice; the choice is only not filed under ## Confirmed, and model-postures.md:11 and stall.py's STATUS_HEADER already carry the same wording. Whether to file it, or to drop the claim, is the user's call, not a note correction. Only the second half is plainly a note against code: "Skip lines from test fixtures (names starting `test_`)" and "never paste the raw line" have no home in stall.py or model-postures.md. Suggested split: the unfiled choice goes to the user; the `test_` skip line is the note-wrong part.

Ids not reached: none.
