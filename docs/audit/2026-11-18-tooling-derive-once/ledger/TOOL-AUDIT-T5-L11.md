# TOOL-AUDIT-T5-L11 (ledger compiler L11; finders F31, F32, F33 with re-sends F31b, F33b)

Compiled only from the finder and verifier reports. Nothing searched again. Where a verifier and a finder disagree, the verifier's reading is used and said so. Short names: HY = Hydra repo (hydra-test), AR = HY\docs\agents, G = C:\Users\Patrick\.claude\gates, MEM = the memory folder, SC = C:\Users\Patrick\.claude\tools\session_cards, MP = model-postures.md. Kind codes K1 to K8 are the finders' own. Quoted "on purpose / by design / kept" words are claims, not reasons.

Verifier reach: V31 and V32 re-opened only the splits and the K6/K8 candidates; their K1 and K7 rows are NOT VERIFIED. V33 and V33b reached most of theirs. V31b reached all of its rows.

---------------------------------------------------------------------
## F31 rows
---------------------------------------------------------------------

### L-F31-01: When no code answers a game question, who writes the new owner?
- **Source:** I7-176, I8-039, I8-041; F31 S1; V31 S1.
- **Kind:** K3 (K7: no check).
- **Verdict:** CONFIRMED. The verifier adds that rule 15 itself has a display case and a non-display case.
- **Homes:**
  - MEM\derive-display-from-engine-truth.md:18: "If none exists, propose one rather than copying."
  - AR\owner-map.md:37: "Nobody writes a new owner for it, because a missing answer to a game question is the user's call"
  - C:\Users\Patrick\.claude\agents\scout.md:11: "Never propose a new owner."
  - AR\agent-rules.md:21 (rule 15): "If none does, the new owner goes in core"; AR\agent-rules.md:23 says the line "never applies to a display"; :25 says under an owner map this waits for the user.
- **Splits:** One. A memory note and rule 15 say to propose or write a new owner in core. The owner-map page and the scout file say nobody proposes one. They differ when no existing code answers the question: by actor (planner vs scout) and by whether the value is a display.
- **Ruling quotes:** None found. V31: no Confirmed line read names both sides.
- **Tests:** none named.
- **Open:** none beyond the above.
- **Hook gap:** n/a.

### L-F31-02: What does a project word's meaning rest on, CONTEXT.md or the code's usage?
- **Source:** I7-218; F31 S2; V31 S2.
- **Kind:** K3.
- **Verdict:** CONFIRMED-CHANGED. The verifier: not a clean split. Order is: CONTEXT.md decides, code usage is checked before calling a term wrong, and a disagreement goes to the user with neither changed (brief-preamble.md:99 says this too). domain.md:43 only says "use the term as defined in CONTEXT.md" and does not conflict.
- **Homes:** AR\agent-rules.md:41 "CONTEXT.md decides what a project word means"; AR\agent-rules.md:43, AR\derive-once-review.md:51 and AR\brief-preamble.md:99 "The code's usage settles the meaning, not a guess drawn from one sentence of a doc"; AR\domain.md:43.
- **Splits:** None proven (wording tension only, per the verifier).
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** n/a.

### L-F31-03: Who folds a copy into its owner, and does a display change stop the fold?
- **Source:** I7-200, I7-202; F31 S3; V31 S3.
- **Kind:** K3.
- **Verdict:** CONFIRMED. The verifier calls the "no new feature code" vs fold point a tension only, since a fold of a copy the join made is inside "fix what the join breaks". Step 9 of the reviewer page was not re-read by the verifier.
- **Homes:** AR\integrate.md:5 "You write no new feature code. You only fix what the join itself breaks"; AR\integrate.md:21-23 "Fold it into the owner in the join ... If a fold would change a displayed text, a score or a stored record, stop and report it. Do not pick."; AR\derive-once-review.md:96 "Fold every copy into the owner." (no display-change stop on that line).
- **Splits:** One. Integrator stops on a fold that changes a display; reviewer's line has no such stop. They differ when a fold changes a displayed text, score or record.
- **Ruling quotes:** AR\rulings\agents-and-workflow.md:19 "if anything changes a display in the app or a record, run it by me. if it only changes code you can use your recommendation without input." Covers displays and records but does not name the fold step; it does not name both sides.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** n/a.

### L-F31-04: Which "boundary table" is the derive-once table?
- **Source:** I8-044; F31 S4; V31 S4.
- **Kind:** K3 (same word, two meanings).
- **Verdict:** CONFIRMED-CHANGED. The verifier: not one question with two answers. A planner/orchestrator writes a table before dispatch; a reviewer applies a four-column table after.
- **Homes:** MEM\derive-display-from-engine-truth.md:24 "write a table of each question and the side that owns it before dispatch"; AR\derive-once-review.md:53 (item 10) "a table with four columns" per value that crosses a boundary.
- **Splits:** None (shared word, different actors and shapes).
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** n/a.

### L-F31-05: Which code calls multiplier_step "for the multiplier squeeze"?
- **Source:** I7-136; F31 S5; V31 S5.
- **Kind:** K8 (light).
- **Verdict:** CONFIRMED. Owner is right, the caller claim is indirect.
- **Homes:** AR\test-rubric-examples.md:270-272 "for the multiplier squeeze (model.cpp:369)". Code: HY\src\core\model.cpp:366-369 (`MultSqueeze::applies`) calls `crosses_multiplier_step`; HY\src\core\timing.cpp:20-27 calls `multiplier_step`, which calls `to_multiplier` (timing.cpp:12-17). Finder: no file under src/app, src/ui or tools calls them.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** n/a.

### L-F31-06: Does a drum lane light every pad its notes land on after Note Shuffle?
- **Source:** I8-055, I8-056; F31 S6; V31 S6.
- **Kind:** K3.
- **Verdict:** CONFIRMED as "not proven". Both can hold if "the notes in the chart" means the shuffled notes. The D254 words sit under Unconfirmed.
- **Homes:** MEM\drum-lanes-d112-2026-10.md:13 "a lane lights every pad its notes land on, on purpose" (a claim); HY\src\parse\song.cpp:192-196 comment (cites D254, "the user ruled that a shuffled roll looks better with every pad lit"); HY\docs\clone-hero\02-note-list.md:627,632 (NL-57); :637 (NL-58, "Hydra applies no cut-off ... The user chose to record this difference and keep Hydra as it is"); song.cpp:202 comment.
- **Splits:** One, not proven. Memory/code say every pad lit; the Confirmed line says the lane must match the chart's notes. They could differ under Note Shuffle.
- **Ruling quotes:** AR\rulings\ui-and-display.md:56 (Confirmed) "why are you lying? the lane on the screen does not match the notes that are in the chart. i know its note shuffle, that doesnt excuse the preview being wrong". ui-and-display.md:117 ("clone hero only draws the blue ...") is under `## Unconfirmed`, so not the user's words. No Confirmed line names both sides.
- **Tests:** none named.
- **Open:** D254 and "lane starting late still counts" rest on the Unconfirmed line only.
- **Hook gap:** n/a.

### L-F31-07: Does Clone Hero "ignore" drums0dnoflip, or count it as a span start with a no-flip flag?
- **Source:** I8-015; F31; V31.
- **Kind:** K3 (wording).
- **Verdict:** CONFIRMED. Hydra's value (flip off) agrees in both.
- **Homes:** MEM\ch-disco-noflip-rule.md:11,15 (description says Clone Hero "ignoring drums0dnoflip"); HY\docs\clone-hero\02-note-list.md:253-260 (NL-21: counts it as a span start, adds only a no-flip flag, so an earlier drums0d keeps flipping through it); HY\src\parse\song.cpp:327-335.
- **Splits:** None in value.
- **Ruling quotes:** AR\rulings\clone-hero.md:53 "this is a bug in clone hero, not an intentional feature. it will be fixed in a future update." The verifier: it speaks to "bug", not to "ignores".
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** n/a.

### L-F31-08: Do the Hydra line numbers quoted on the Clone Hero lane page still match song.cpp?
- **Source:** I8-054; F31; V31.
- **Kind:** K2.
- **Verdict:** CONFIRMED (partly). Page lines 203/88/147 vs code 205/90/149 seen. The call line (page 2009 vs finder's 2204) was not seen by the verifier: unchecked.
- **Homes:** HY\docs\clone-hero\02-note-list.md:611 (`light_drum_lanes` line 203, `lights_lane_pad` 88, `game_lane_pads` 147) and :631 ("called at line 2009"); HY\src\parse\song.cpp: `lights_lane_pad` :90, `game_lane_pads` :149, `light_drum_lanes` :205. MEM\drum-lanes-d112-2026-10.md:11 says it is called once in `finish_loaded_song` (finder: call at song.cpp:2204).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** the call line 2204.
- **Hook gap:** n/a.

### L-F31-09: Do planner and reviewer briefs carry the edge-case rule the memory note says they carry?
- **Source:** I8-025; F31; V31.
- **Kind:** K8.
- **Verdict:** CONFIRMED in the scope searched. The memory note says "Planner and reviewer briefs say this too." grep of "edge case" over C:\Users\Patrick\.claude\agents finds no file. Briefs outside that folder were not searched.
- **Homes:** MEM\check-an-edge-case-can-happen.md:15; (claimed copies in planner.md and reviewer.md not found).
- **Splits:** None.
- **Ruling quotes:** AR\rulings\questions-and-audits.md:9 "i dont understand why you always try to invent insane edge cases, this one isnt even possible in clone hero..." (finder; not re-opened).
- **Tests:** none named.
- **Open:** briefs outside the agents folder.
- **Hook gap:** n/a.

### L-F31-10: Where do the two fc-video checklist rules ("report the headline", "never rewrite the video") live?
- **Source:** I8-071; F31, F31b; V31, V31b.
- **Kind:** K1, K3 (mild), K8 (first search).
- **Verdict:** CONFIRMED. F31/V31 did not find the README (searched only HY). F31b found it in the video-tools repo and V31b confirmed. The first reports' "not found" is superseded.
- **Homes:** MEM\fc-video-workflow.md:15 "Both are now README checklist items 0 and 5."; C:\Users\Patrick\Downloads\Hydra\video-tools\README.md:41-43 item 0 "Report the headline the moment you have it" and :56-57 item 5 "Never re-encode or rewrite the video for any reason." (README :38/:39 "Eleven things to check"). That repo is outside the audit scope list.
- **Splits:** None in value. The trigger wording differs ("when engine and evidence agree" vs "the moment you have it"; "rewrite" vs "re-encode").
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** n/a.

### L-F31-11: Skill points at a memory note that exists only in the archive
- **Source:** side note; F31; V31.
- **Kind:** K8.
- **Verdict:** CONFIRMED.
- **Homes:** C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:34 names memory note `gameplay-video-score-analysis`; it exists only as memory\archive\gameplay-video-score-analysis.md (not indexed).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** n/a.

### L-F31-12: A hook message that spells out rule 14
- **Source:** side note; F31; V31.
- **Kind:** K5 (close to a pointer, per the finder).
- **Verdict:** CONFIRMED.
- **Homes:** G\percall.py:108-110 "Agents never build, so hydra_build_pass is refused to every agent. ... the main session builds your branch and runs any test that needs a build". It restates rule 14 and names its own refusal. AR\agent-rules.md:27 says "nothing checks rule 14's line on tests" (K7 for the test half, finder).
- **Splits:** None.
- **Ruling quotes:** AR\rulings\agents-and-workflow.md:8-9 (finder, not re-opened by V31): "i want a limit of three builds at once ... only the main session" and "any tests that require a build are done by the main session, not agents."
- **Tests:** none named (finder: none in G\tests holds these homes equal).
- **Open:** none.
- **Hook gap:** n/a.

### L-F31-13: Who runs tests and builds (about ten agreeing homes)
- **Source:** I7-225, I8-099, I8-009; F31; V31 (not reached).
- **Kind:** K1 (K7 for the test half).
- **Verdict:** NOT VERIFIED (V31 took the finder's words).
- **Homes (finder only):** AR\doc-screenshots.md:19; MEM\merge-tests-touch-changed-files.md:11; AR\agent-rules.md:1,14,27; HY\CLAUDE.md rules 1 and 14; AR\brief-preamble.md:47,57,59; AR\ui-testing.md:17,27,151,348,362; AR\integrate.md:35,41,46; AR\derive-once-review.md:99,116,118; C:\Users\Patrick\.claude\agents\executor.md:3; reviewer.md:21; G\percall.py:109-110. Finder: all agree; card-writer agent file not read; brief-preamble says agents run hydra_precheck "from main" while ui-testing.md:364 says the hydra MCP runs the checkout its helper sits in. Also I8-009 (one build per wave): single home MEM\one-build-per-wave.md:11 (see L-F32-01).
- **Splits:** None found by the finder.
- **Ruling quotes:** AR\rulings\agents-and-workflow.md:8-9 (as above).
- **Tests:** none named.
- **Open:** all rows.
- **Hook gap:** n/a.

### L-F31-14: Where is the engine record read, and is an agent told to run what rule 14 forbids?
- **Source:** I7-108; F31; V31 (not reached).
- **Kind:** K3 (who acts), K6 candidate.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** AR\test-rubric.md:45 (hydra_replay dump then schedule; "ask the main session"); C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:18 ("hydra_replay dump"; scout "reconstructs each activation's SP ends engine-exactly", a second SP-end reconstruction in Python at C:\Users\Patrick\Downloads\Hydra\squeeze-tools, not read); MEM\sp-squeeze-mechanics.md; AR\clone-hero-mcp.md:48. The skill's invoker is not stated, so an agent loading it is told to run something agents may not run (rule 14).
- **Splits:** None named.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** all of it.
- **Hook gap:** n/a.

### L-F31-15: Where does SqOut get drawn and measured, the squeezed chord or the pre-collection end?
- **Source:** I8-043; F31; V31 (not reached).
- **Kind:** K3 candidate.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** MEM\sp-squeeze-mechanics.md / derive-display note :23 (drawn at the squeezed chord; "the search's bookkeeping (the nominal end ...) is not where the squeeze happens"); SKILL.md:22-23 (SqOut measured against the "pre-collection end"); HY\src\core\model.h:688 `nominal_end`, model.cpp:719,793 `sqout_tick`, HY\src\core\sqout_chord.h:49.
- **Splits:** Not proven (draw place vs gap measure); the finder says the main session should see both.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** all of it.
- **Hook gap:** n/a.

### L-F31-16: Rubric number rules, owner-map stops, planner owner-use and the other single-home prose rules
- **Source:** I7-104, 105, 107, 111, 115, 173, 177, 178, 190, 199, 200 (see L-F31-03), 211, 212, 217, 219; I8-009, 010, 013, 029, 032-035, 041, 042, 046, 053, 056; F31; V31 (not reached).
- **Kind:** K1 and K7 (mostly agreeing copies or single-home prose rules), K8-light (brief-preamble:111 says owner-map.md is "the one home of this rule" while spelling it out).
- **Verdict:** NOT VERIFIED (V31 took the finder's words).
- **Homes (finder only):** AR\test-rubric.md:21-25 and test-rubric-examples.md:186,190,265-267 copy the 10/20/30 multiplier steps (owner HY\src\core\timing.cpp:12-17; page HY\docs\clone-hero\06-scoring.md:55); AR\owner-map.md:17,37,39,43 against C:\Users\Patrick\.claude\agents\planner.md:19 and reviewer.md:17 (near word-for-word copies of "use each owner as it is, never combine owners ..."); AR\brief-preamble.md:111; MEM\counts-on-screen-must-agree.md:11-24 (counts rule) with AR\audit-method.md:75,97; MEM\derive-display-from-engine-truth.md:15,21,22 (names Chord::activation_note, kSpActivationBars, sp_ready_ms, app::song_length_ms, deact_tick; `app::song_length_ms` not grepped); MEM\dmleaderboards-comparison.md:47-50 with HY\src\net\dmbot_client.h:35-36; AR\issue-tracker.md:20,25; AR\domain.md:11,49 (finder: "proceed silently" against fail-loudly rules, K3 on tone); AR\handoff.md:45 and brief-preamble.md:97. Finder: one memory note (MEM\one-owner numbering, I8-038) has no owner named; see L-F31-27.
- **Splits:** None named by the finder.
- **Ruling quotes:** (finder, not re-opened) AR\rulings\questions-and-audits.md:11-13,15 "when did they diverge and why? did i ever ask for it?", "why are we even using two different systems...", "yes everything should use one rule.", "what are we already doing and why are you asking me to re derive it"; AR\rulings\review-and-hooks.md:8 "why did no reviewer ever catch this as a violation of derive once?".
- **Tests:** none named (finder: none in G\tests holds any of these).
- **Open:** all rows.
- **Hook gap:** n/a.

### L-F31-17: Does Hydra's test-rubric audit set-up agree with the audit method page?
- **Source:** I7-124; F31b (V31b CONFIRMED); F31 (shallow).
- **Kind:** K1; K3 (weak); K7 candidate.
- **Verdict:** CONFIRMED (V31b).
- **Homes:** AR\test-rubric.md:189 (calibration stage applies; bans `tests/test_sp_end.cpp`, `tests/test_sp_meter_curve.cpp` and any file the examples page quotes); AR\audit-method.md:44-46 (calibration; "If the calibration agent misses a known problem, stop and report the miss to the user"); AR\test-rubric-examples.md quotes the two banned files nine times (the ban list is a copy by name); AR\test-rubric.md:191-205 (order, corpus, plant bugs at :199, "No such test exists today (question 6)" at :201, unchecked). Finder: planting bugs is also done by HY\tools\mutate; test-rubric.md:199 does not point at it.
- **Splits:** None in value. test-rubric.md says nothing about a calibration miss; audit-method.md does.
- **Ruling quotes:** AR\rulings\agents-and-workflow.md:8-9 "any tests that require a build are done by the main session, not agents." (not re-opened by V31b; does not speak to the calibration homes).
- **Tests:** none named.
- **Open:** whether question 6's "no such test exists" holds.
- **Hook gap:** n/a.

### L-F31-18: Which code decides which side of the SP end a note sits on (owner-map counter-example)?
- **Source:** I7-180; F31, F31b; V31b.
- **Kind:** K1; K6 side line.
- **Verdict:** CONFIRMED. Owner claim holds.
- **Homes:** AR\owner-map.md:31; HY\src\core\model.h:428-432 "Which side of the SP end the note sits on is core::paid_by_sp_walk's question, not this one."; model.h:298; HY\src\core\backend_value.h:26-28 `paid_by_sp_walk(offset_ms)` ("The SP walk pays every chord at or before the SP end (the window is inclusive at the deactivation node), before any backend pricing happens"); backend_value.h:30-31 a tick form answering the same edge by tick, whose comment says it "agrees with paid_by_sp_walk wherever time runs forwards".
- **Splits:** None found. Two code places answer the edge (offset and tick); whether a test holds them equal is open (V31b did not read tests).
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** whether a test holds the offset and tick forms equal.
- **Hook gap:** n/a.

### L-F31-19: What must the "go" message before a live run hold?
- **Source:** I7-170; F31b; V31b.
- **Kind:** K1 (pointer), K5/K7 (prose, no hook checks the message).
- **Verdict:** CONFIRMED.
- **Homes:** AR\premise-map.md:78 (pointer); AR\clone-hero-mcp.md:58 (holds the content; "No hook checks this message; the plan reviewer checks that every step has its `Calls:` line."); planner.md:13; reviewer.md:13.
- **Splits:** None.
- **Ruling quotes:** None found (searched AR\rulings for "Calls:", "go request", "call list").
- **Tests:** G\tests\test_agent_files.py:16-21 holds only the phrase lists the planner and reviewer files must contain.
- **Open:** none.
- **Hook gap:** n/a (F31b: none noticed).

### L-F31-20: What must an integrator's report hold?
- **Source:** I7-208; F31, F31b; V31b.
- **Kind:** K1 (inside one page), K7.
- **Verdict:** CONFIRMED. V31b did not re-read the preamble default list.
- **Homes:** AR\integrate.md:56 (head commit hash and range); AR\integrate.md:60 (branch, worktree, head hash, each merge hash, owners gained, folds, conflicts, precheck output, stops); AR\brief-preamble.md "Your final report" default list (commit hashes, test commands with pass counts, ...). No integrator agent file exists; no hook reads the report.
- **Splits:** None in value. Which list applies is not stated.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** n/a.

### L-F31-21: What must an agent say before quoting a stored number, and is the pointer true?
- **Source:** I7-161; F31b; V31b.
- **Kind:** K1.
- **Verdict:** CONFIRMED. Pointer true: `build_activations` (HY\src\app\path_view.cpp:225) and `build_path_buttons` (:529) exist.
- **Homes:** AR\this-machine.md:95 (one text home).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** n/a.

### L-F31-22: Is a chosen mockup's look the design, with today's look never offered beside it?
- **Source:** I8-011; F31b; V31b.
- **Kind:** K7; K3 edge.
- **Verdict:** CONFIRMED.
- **Homes:** MEM\approved-work-is-not-reasked.md:15 "A chosen mock's values are the design; today's look is never an option beside it." (memory only; no page or hook); MEM\MEMORY.md:8 and MEM\new-screens-start-from-a-clickable-mockup.md:11,15,17 (related); AR\agent-rules.md:18 (rule 12, opposite direction).
- **Splits:** None proven. AR\rulings\ui-and-display.md:123 (under `## Unconfirmed`, starts :102) says row-layout consistency matters more than matching an approved mock: a possible exception, not a ruling.
- **Ruling quotes:** AR\rulings\ui-and-display.md:9 (Confirmed section runs :7-101) "yes i approved the mockup and asked for option B. so why are you offering what we have today as an option?" Names one side only. Finder also: :75, :77.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** n/a.

### L-F31-23: What to call and how to price a run that beats optimal
- **Source:** I8-016; F31b; V31b.
- **Kind:** K1 (SKILL.md copies the README), K2/K3, K7.
- **Verdict:** CONFIRMED-CHANGED (V31b): real; the two splits are different sources/causes but opposite verbs.
- **Homes:** MEM\ch-disco-noflip-rule.md:15 (grep the MIDI for `drums0dnoflip`, price against a scratch copy with the marker renamed `drums0dNOFLIP`; memory only); C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:55 and video-tools\README.md:41-43 (item 1) "a version problem or a bug, never a real score"; price 1.0 runs with `--legacy-fills`; video-tools\README.md:710-727 ("That is the whole recipe", :726); MEM\dmleaderboards-comparison.md:26-29 "Above-optimal scores are NORMAL, not a bug ... never flag as impossible/error"; AR\public-doc-style.md:17.
- **Splits:** (a) bug vs normal: a video run being priced against leaderboard rows. (b) the README's "whole recipe" never mentions the scratch-copy step that the memory note adds.
- **Ruling quotes:** AR\rulings\clone-hero.md:53 "this is a bug in clone hero, not an intentional feature. it will be fixed in a future update." Not about pricing; names neither side.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** n/a.

### L-F31-24: How many bars does an SP activation need, and how many remain when SP runs out?
- **Source:** I8-026; F31, F31b; V31b.
- **Kind:** K1.
- **Verdict:** CONFIRMED.
- **Homes:** MEM\check-an-edge-case-can-happen.md:11 (copy of the value, names `branch_activate` and `create_deactivated_path`, which drift if renamed); HY\src\core\timing.h:50-51 `kSpActivationBars = 2` ("Clone Hero's rule: an activation needs two banked bars of Star Power."); engine.cpp:982-983, :1001, :1106 `c.sp_bars = sq ? 1 : 0;`; model.cpp:703; path_view.cpp:539; pb_text.cpp:72-74; replay.h:338. Different fact, same number 2: bar length `kMeasuresPerSpBar = 2` (timing.h:268), stated in SKILL.md:22-23 and test-rubric.md:21.
- **Splits:** None.
- **Ruling quotes:** AR\rulings\questions-and-audits.md:9 (finder; not re-opened by V31b).
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** n/a.

### L-F31-25: May a plan carry an "Out of scope" list, and what does it need?
- **Source:** I8-036; F31, F31b; V31b.
- **Kind:** K3, K7.
- **Verdict:** CONFIRMED. The "to-do list" wording is in the Unconfirmed line (not re-opened by V31b).
- **Homes:** MEM\counts-on-screen-must-agree.md:24 (needs a recorded yes); C:\Users\Patrick\.claude\agents\planner.md:13 ("move it to an 'Out of scope' list for the user to approve"); MEM\search-web-before-finalizing-plans.md:11-15 ("Do not leave a 'left alone' list of related defects; either fold them into the plan or ask."); AR\rulings\docs-and-writing.md:19 (under `## Unconfirmed`).
- **Splits:** One. Planner default act (list for approval) vs the search-web note (no left-alone list). Compatible only if the user approves each item.
- **Ruling quotes:** None found Confirmed.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** n/a.

### L-F31-26: How many numberings may a tool reply have, and who owns each?
- **Source:** I8-038; F31, F31b; V31b.
- **Kind:** K8 (memory line names no owner), K6 by design (ADR text).
- **Verdict:** CONFIRMED.
- **Homes:** MEM\counts-on-screen-must-agree.md:18 "One owner per numbering." (names none); HY\docs\adr\0030-hydra-owns-every-live-command-rule.md:50 (script mode: the interpreter; live mode: only the helper numbers a `hydra_run` script's lines); HY\tools\hydra_ui_mcp.py:691-708 ("stopped at line N of M"); HY\tests\ui\uitest_commands.cpp:281-291, :432 ("!! failed at line %d"); AR\ui-testing.md:97,229,259; AR\doc-screenshots.md:13; HY\tools\test_hydra_ui_mcp.py:773,949,956 hold the helper's string.
- **Splits:** None in value; wording differs between the two modes.
- **Ruling quotes:** AR\rulings\code-and-engine.md:30 (Confirmed section runs :7-60) "you think giving claude conflicting numbers is a good idea? that is just going to confuse the model". Names the one-numbering side, not the two-modes side.
- **Tests:** HY\tools\test_hydra_ui_mcp.py (helper string only).
- **Open:** none.
- **Hook gap:** n/a.

### L-F31-27: Other F31 side notes
- **Source:** F31 side notes; V31 (not reached).
- **Kind:** K1, K8.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** SKILL.md:10 and MEM\sp-squeeze-mechanics.md:3,13 copy `kSqueezeWindowMs` = 500 ms (owner HY\src\core\model.h:121; agree today); MEM\one-build-per-wave.md shows a modified date after today's date (not touched).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** all.
- **Hook gap:** n/a.

---------------------------------------------------------------------
## F32 rows
---------------------------------------------------------------------

### L-F32-01: Build each executor's branch before review, or one combined build per wave?
- **Source:** I8-120, I10-038 and related; F32 S1; V32 S1.
- **Kind:** K3.
- **Verdict:** CONFIRMED.
- **Homes:** MEM\one-build-per-wave.md:11 "review each branch, then put the reviewed ones on one combined branch, build it once and run all their tests ... Never plan one build per branch."; C:\Users\Patrick\.claude\model-postures.md:11 "build its branch (`hydra_build_pass`, one build at a time) and run its tests, before review."; AR\agent-rules.md:20 (rule 14) "A failure before review goes to a fresh executor on the same branch ... the main session then builds again."
- **Splits:** One. They differ when a wave has many small branches (build count).
- **Ruling quotes:** AR\rulings\agents-and-workflow.md:8 "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." and :9 "any tests that require a build are done by the main session, not agents." Neither names a side. The note's own user quote ("One build per wave") has no card and no Confirmed line (V32: docs/sessions holds no card for the session).
- **Tests:** none for the build rule across docs. HY\tools\test_build_pass.ps1 and HY\tools\test_hydra_ui_mcp.py hold build_slot.ps1's reply text only.
- **Open:** none.
- **Hook gap:** n/a.

### L-F32-02: The build rule's numbers and text (3 builds, one per session, 120 s) in many homes
- **Source:** I10-038, 042, 043, 044, 045, 075, I10b-106, 112, I8-120; F32 Group A; V32 (partly: rule 14, MP:11, AR).
- **Kind:** K1; K5 (hook and script texts spell out the rule); K7 (the test half).
- **Verdict:** NOT VERIFIED beyond rule 14, MP:11 and the AR page (V32).
- **Homes (finder only unless stated):** HY\tools\build_slot.ps1:75 `$BuildSlotCount = 3`, :80 `$BuildPassUnclaimedSeconds = 120`, :87, :195, env var `HYDRA_BUILD_PASS` :21; HY\CLAUDE.md rule 14; AR\agent-rules.md:20 and :27; AR\brief-preamble.md:57; AR\derive-once-review.md:24,116; HY\docs\development.md:114-144 (:143 limit applies only when CLAUDECODE is 1; the CLAUDE.md and AR rule text does not carry that scope); executor.md:11, reviewer.md:21; G\percall.py:19,109; G\README.md:15; HY\tools\hydra_ui_mcp.py:22-23,874-877; HY\tools\new_worktree.ps1:7-8; HY\tools\compare_db.py:46; HY\tools\mutate\run.py:37; AR\rulings\agents-and-workflow.md:35 (Unconfirmed: "an unclaimed pass runs out after 2 minutes, 120 seconds"). card-writer.md carries no build rule.
- **Splits:** None beyond L-F32-01 (finder: no home states the edge at the fourth pass; worktrees not mentioned in AR/preamble).
- **Ruling quotes:** AR\rulings\agents-and-workflow.md:8-9 (above; verifier saw them as quoted).
- **Tests:** HY\tools\test_build_pass.ps1:4,117,129,225 and HY\tools\test_hydra_ui_mcp.py:667,683 hold build_slot.ps1's text, not the doc numbers. G\tests\test_docs_numbers.py holds budget and bash numbers, not builds.
- **Open:** all not re-opened.
- **Hook gap:** n/a.

### L-F32-03: Is there one answer for where SP ends, and what is the 3 ms leeway for?
- **Source:** I8-157, I8-159, I8-106 context; F32 S2; V32 S2.
- **Kind:** K6; K1 (3 ms in several homes).
- **Verdict:** CONFIRMED (and K6). The verifier: the row-type branch is the same file's code and the ruling does not settle whether the rating's "inside" question is a "counted/uncounted" or a "where SP ends" question; left to the main session.
- **Homes:** HY\src\core\squeeze_rating.cpp:185-188 `inside = row.squeezed_out ? core::paid_by_sp_walk(o) : core::counted_without_squeeze(o, backend_leeway_ms)`; HY\src\core\backend_value.h:28 `paid_by_sp_walk` = offset <= 0; :39-40 `counted_without_squeeze` = paid_by_sp_walk || offset < leeway. MEM\sp-end-is-one-point-leeway-is-delgars.md:11-17 (says no ruling answers it yet; does not cite the Confirmed line below); HY\CONTEXT.md:799-806 (leeway definition, default tied to `Rules::backend_leeway_ms` by marker comment); AR\audit-method.md:99,135; HY\src\app\path_view.cpp:262,383 (finder).
- **Splits:** One. A squeezed-out row is judged against the bare SP end; a plain backend row against the leeway. They differ for offsets from 0 up to the leeway (default 3 ms): a plain row is "inside", a squeezed-out row is not.
- **Ruling quotes:** AR\rulings\code-and-engine.md:12 (Confirmed, 2026-10-29) "there should never be two different answers for where star power ends. ever. the 3ms rule is for whether a backend is included in the optimal score or uncounted. it's an arbitrary number chosen by dragondelgar and i am fine with keeping it so our scores use the same methodology as delgar's hydra" (:13 is the same words without the first sentence). It states both sides (one SP end; 3 ms only for counted/uncounted); V32 says it does not settle which question the row-type branch asks. AR\rulings\questions-and-audits.md:27 sits in the Unconfirmed section (starts :24), not a ruling. A Confirmed badge line (ui-and-display.md, 2026-11-13, "the little badge showing the timing leeway is fine, but it needs to have a solid definition ...") covers badge text only.
- **Tests:** none holding the two rules equal; CONTEXT.md `<!-- default: Rules::backend_leeway_ms -->` markers are read by the docs check (finder).
- **Open:** whether a run shows a visible difference; not run.
- **Hook gap:** n/a.

### L-F32-04: Is MultSqueeze::gain only read by the display?
- **Source:** I8-106; F32; V32 (not reached).
- **Kind:** none found (single home plus code).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** MEM\min-path-delta-proof.md:22-23; HY\src\core\model.cpp:455 `MultSqueeze::gain()` calls `multsqueeze_gain` (scoring.cpp:148); only reader HY\src\app\path_view.cpp:141,144.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** all.
- **Hook gap:** n/a.

### L-F32-05: Which pitches and tick ends count for a phrase's completion, and who owns that?
- **Source:** I8-197, I8-198; F32; V32 (not reached).
- **Kind:** K1, K5; side note N1.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:32-33; C:\Users\Patrick\Downloads\Hydra\squeeze-tools\squeeze_scout.py:334-349,684 (outside the scope list); owner HY\src\parse\song.cpp:426-431 (`close_sp_phrase`) and :886-888 (`is_handled_note`, `kick2x`). The skill gives no pointer to song.cpp.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** all.
- **Hook gap:** n/a.

### L-F32-06: Where may a proper name in a deliverable come from?
- **Source:** I8-108, I8-211, I8-109; F32 Group C; V32 (not reached).
- **Kind:** K1, K2, K7.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** MEM\names-from-data-not-recall.md:15 ("Every proper name in a deliverable must trace to data in hand (files, DB, web lookup)"); SKILL.md:63 ("a file, the database or a cited lookup, or prints as `unknown`"); AR\derive-once-review.md:76 ("Every name comes from the repo, not memory."). Not in CLAUDE.md, the preamble or any agent file. I8-109 (Feeble Words is a full drum tacet): single home MEM\names-from-data-not-recall.md:17.
- **Splits:** Source lists differ (web lookup counted in two homes, not in the reviewer page); the finder calls it a K1/K2 difference, not a split.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** all.
- **Hook gap:** n/a.

### L-F32-07: When does a new screen need a clickable mockup, and what may it hold?
- **Source:** I8-110, I8-111; F32 Group D; V32 (not reached).
- **Kind:** K2 (within the memory note), K7.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** MEM\new-screens-start-from-a-clickable-mockup.md:3 ("before any plan or build"), :11 ("approves it before a plan is written"), :17 ("no option switches ... never as toggles"; the same line also says "The second mockup's four toggles were accepted; ten was too many"); MEM\MEMORY.md:8; MEM\approved-work-is-not-reasked.md:15; MEM\one-time-scale-per-screen.md:15. No home in CLAUDE.md, docs\agents or any agent file.
- **Splits:** Wording gap inside the note (description vs body; index "no option toggles" vs "four toggles were accepted"); not a split between homes.
- **Ruling quotes:** AR\rulings\ui-and-display.md:24 (2026-10-31) "this is way too many options. im not going to bother checking different combinations of 50 different boxes"; :9 (2026-11-14) "yes i approved the mockup and asked for option B. so why are you offering what we have today as an option?" (finder; the :9 line was re-read by V31b, see L-F31-22).
- **Tests:** none named.
- **Open:** all.
- **Hook gap:** n/a.

### L-F32-08: Memory note says no hook stops a whole-library run, but one does
- **Source:** I8-116; F32 S3; V32 S3.
- **Kind:** K2/K8 (stale note), K1.
- **Verdict:** CONFIRMED. Edge: the hook keys on the flag in shell text; whether a run without the flag slips past needs a run to settle.
- **Homes:** MEM\no-full-library-runs.md "No hook enforces it yet."; G\percall.py:45-55 `_library_refusal` refuses a shell call holding LIBRARY_FLAG with no typed allow; AR\agent-rules.md:14,27; AR\brief-preamble.md:49; MP:13 ("in chat" without the typed phrase); AR\this-machine.md:91. "How to ask" wording lives in the memory note and an Unconfirmed line (AR\rulings\tests-and-runs.md:36). "About 115 charts" in AR rule 8 and the note; finder counted 116.
- **Splits:** One (stale text vs hook); the note's "No hook enforces it yet" differs from the hook when any shell call holds the library flag.
- **Ruling quotes:** AR\rulings\review-and-hooks.md:16 and AR\rulings\tests-and-runs.md:14 (2026-10-25) "i want to be able to give explicit approval for a full library run."; tests-and-runs.md:18 "you dont need a full library run to check the rendering...". The "How to ask" form is in an Unconfirmed line only.
- **Tests:** none for the wording.
- **Open:** a call that reaches the whole library without the flag: needs a run.
- **Hook gap:** hook gap: possible, G\percall.py (V32 left it to a run to settle; no steps given).

### L-F32-09: Agent rules "11 to 13" hold the "You will see" list and the visible-change rule
- **Source:** I8-135; F32; V32.
- **Kind:** K8.
- **Verdict:** CONFIRMED-CHANGED. The range is one rule too wide: rule 11 is the list, rule 12 the visible-change rule, rule 13 is about uncommitted edits. Small; not a split.
- **Homes:** MEM\questions-describe-the-real-rule.md:11; AR\agent-rules.md rules 11-13 (line 17 for rule 11); HY\CLAUDE.md rules 11-13.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** n/a.

### L-F32-10: Status relay words, and the missing record of the user's "Claim, plus disagreement" pick
- **Source:** I8-141, I9-115; F32; V32 (relay-note ruling claim only).
- **Kind:** K1; K7 (test-fixture skip); K8 (ruling citation).
- **Verdict:** K8 part CONFIRMED (V32: searching docs/agents/rulings and docs/sessions for "Claim, plus" found nothing; the only 2026-11-16 ruling line is tests-and-runs.md:8, about tests failing instantly). The agreeing-homes part NOT VERIFIED.
- **Homes:** MEM\relay-status-in-plain-english.md:11,15 (cites `2026-11-16 ("Claim, plus disagreement")`; also "skip lines from test fixtures (names starting `test_`)" and "never paste the raw line"; neither is in stall.py or MP); MP:11; G\stall.py:26 `STATUS_MAX_LINES = 12`, :30-33 `STATUS_HEADER`; G\tests\test_stall.py:213-247,298-369 holds the header words.
- **Splits:** None.
- **Ruling quotes:** AR\rulings\agents-and-workflow.md:26 (Confirmed, 2026-09-05, finder) "the status updates should be brief but relayed in plain english".
- **Tests:** G\tests\test_stall.py holds the hook text to words, not to MP:11.
- **Open:** none for the K8 part.
- **Hook gap:** n/a.

### L-F32-11: "Same functionality" and "narrowing is its own question"
- **Source:** I8-137, I8-138; F32 Group H; V32 (not reached).
- **Kind:** K1, K7.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** MEM\questions-describe-the-real-rule.md:13,20; AR\audit-method.md:36,146; AR rule 12.
- **Splits:** None (different subjects: a question vs an audit's scope).
- **Ruling quotes:** AR\rulings\code-and-engine.md:47 (Confirmed, 2026-08-23) "hydra is expected to keep the same functionality it has today. your plan needs to ensure that." (:86 Unconfirmed twin.) Finder only.
- **Tests:** none named.
- **Open:** all.
- **Hook gap:** n/a.

### L-F32-12: Prior-art search and "left alone" lists in plans
- **Source:** I8-155, I8-156; F32; V32 (not reached).
- **Kind:** K7, K1.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** MEM\search-web-before-finalizing-plans.md:11,13,15; AR\audit-method.md:36 ("Leave nothing in a 'pre-existing, left alone' pile." D119); AR\rulings\docs-and-writing.md:19 (Unconfirmed). Not in planner.md, premise-map.md or owner-map.md.
- **Splits:** None (see also L-F31-25).
- **Ruling quotes:** None Confirmed found (D119's words are quoted in audit-method.md but not under `## Confirmed`).
- **Tests:** none named.
- **Open:** all.
- **Hook gap:** n/a.

### L-F32-13: Must a live step hold only one "never tried" call?
- **Source:** I8-173; F32 S4; V32 S4.
- **Kind:** K2/K3 (note silent on the cap).
- **Verdict:** CONFIRMED-CHANGED. The verifier: a gap, not a flat clash. The note sets no cap; it does not state that several are allowed (the finder's reading "allows several" is not what it says). tests\test_docs_match_code.cpp:1851-1878 was not opened by V32.
- **Homes:** MEM\trace-pass-inputs-before-live-runs.md:11,17-18 ("Mark each 'seen working on this exact target' or 'never tried'. Check each 'never tried' one offline ... and show the user that list."); AR\premise-map.md:64-68 "A step holds at most one `(never tried)` call. A step with two or more is a shape error."; AR\clone-hero-mcp.md:58.
- **Splits:** None proven (a gap; note silent on the cap).
- **Ruling quotes:** None found.
- **Tests:** HY\tests\test_docs_match_code.cpp:1851-1878 holds premise-map's shape against plan files, not against the memory note (finder; unopened by V32).
- **Open:** none.
- **Hook gap:** n/a.

### L-F32-14: Other method rules (search-nothing, forced repro, stored versions, open the source, find the question)
- **Source:** I8-160, I8-170, I8-175, I8-178, I8-129; F32 Group J; V32 (not reached).
- **Kind:** K1, K7.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** "a search that finds nothing proves nothing": AR\brief-preamble.md:103, AR\audit-method.md:124, AR\derive-once-review.md:76, AR\doc-review.md:65, MEM\verify-before-claiming-a-source-is-wrong.md:22,28 (five wordings; one page names the preamble as source); forced repro: brief-preamble.md:83, MEM\trace-pass-inputs-before-live-runs.md:19; stored versions: MEM\stored-versions-one-rule.md (not in public-doc-style.md, doc-review.md or CLAUDE.md; HY\src\store\stored_versions.h exists); MEM\open-the-source-before-offering-work.md:11,15; MEM\sp-end-is-one-point-leeway-is-delgars.md:16 with AR\audit-method.md:99 and AR\owner-map.md:43.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** all.
- **Hook gap:** n/a.

### L-F32-15: Where does kSqueezeWindowMs live, and the hit-window constant name in the scout tool
- **Source:** I8-213; F32 S5; V32 S5.
- **Kind:** K2, K8.
- **Verdict:** CONFIRMED for the skill's wrong file. V32 did not re-check that `kDefaultHitWindowMs` is absent from src.
- **Homes:** SKILL.md:10 "(`kSqueezeWindowMs`, src/search/graph.h)"; code HY\src\core\model.h:121 `constexpr double kSqueezeWindowMs = 500.0;` (src\search\graph.h:216 only mentions it in a comment; HY\src\core\model.h:106 `kHitWindowMs = 85.0`); MEM\sp-squeeze-mechanics.md:13-15 names src/core/model.h correctly and `within_squeeze_window` as the one check; C:\Users\Patrick\Downloads\Hydra\squeeze-tools\squeeze_scout.py:33-34 (outside scope) names "kDefaultHitWindowMs, src/core/model.h" and "kSqueezeWindowMs, src/search/graph.h". Values 500 and 85 agree across note, skill, tool and engine.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** `kDefaultHitWindowMs` absence (finder only).
- **Hook gap:** n/a.

### L-F32-16: Skill and setup rows (pinned package version, approval before setup, ask which app, fcvideo edit rule)
- **Source:** I8-218, I8-220, I8-210; F32 Group K, N5; V32 (not reached).
- **Kind:** K1; K7.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** skills\reverse-engineer-anything\SKILL.md:50-57 and references\connection-and-recovery.md:54-56 (same approval rule, near-identical words); `rea-agents@6.5.0` in SKILL.md:37,51,56,71,79 and AR\this-machine.md:25 (neither says which owns the version); SKILL.md:219-220 (single home); skills\analyzing-sp-squeezes\SKILL.md:62 (one session edits fcvideo.py at a time; single home) and :49.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** all.
- **Hook gap:** n/a.

### L-F32-17: Owner claims for hydra_use_mimalloc
- **Source:** I8-101; F32; V32 (not reached).
- **Kind:** K1 (pointers); side note N2.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** MEM\mimalloc-link-order.md:11; HY\CMakeLists.txt:109,266,417 and :1095 (function exists); HY\docs\development.md:236. Side note: MEM\mimalloc-link-order.md:15 pins a dumpbin path with MSVC version 14.44.35207 (a machine path in a memory note; this-machine.md is the page for machine paths).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** all.
- **Hook gap:** n/a.

### L-F32-18: Hook registration, prompt parts, git gate installer rows
- **Source:** I9-036, 037, 082, 160; F32 Group L; V32 (not reached).
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** G\settings.fragment.json:124-133,190-200 and the live C:\Users\Patrick\.claude\settings.json:134-143,200-210; G\README.md:23,33; G\prompt.py:1,18,20 (`FIELD_CAP = 10000`); G\install_ref_gate.ps1:12 (marker `# git-ref-gate shim`); README :53,83,162.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** G\tests\test_docs_numbers.py:148-163 and `test_live_settings_carry_the_fragment` (skipped in a worktree); G\tests\test_posture.py:139-273; G\tests\test_ref_judge.py:609-666.
- **Open:** all.
- **Hook gap:** n/a.

### L-F32-19: Session-card start note, claim window and reminder gap
- **Source:** I9-169, 170, 171; F32 Group M; V32 (not reached).
- **Kind:** K5 (note spells out commit steps held elsewhere), K1.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** G\session_cards_start.ps1:43 `$ClaimHours = 2`, :47 `$ReminderGapHours = 24`, :199-235, :275-292, :338; HY\CLAUDE.md "Past sessions"; card-writer.md and SC\writer-brief.md (card writer skips the preamble); the hook note says cards need no reviewer (stated only in the hook text).
- **Splits:** None.
- **Ruling quotes:** AR\rulings\agents-and-workflow.md:12 (2026-10-14) "use haiku for these simple transcript readers"; :15 "continue, and raise the cap to 60." (finder only). No Confirmed line found for the 2-hour claim window; the script comment cites "The user's choice (task SC-9, 2026-10-14)".
- **Tests:** G\tests\test_h53_session_cards_start.ps1 (comment: "The thresholds themselves live only in session_cards_start.ps1").
- **Open:** all.
- **Hook gap:** n/a.

### L-F32-20: Tool and MCP text rows (Hydra MCP instruction text, compiler process names)
- **Source:** I10-069, I10b-006, I10b-008; F32 Group N; V32 (not reached).
- **Kind:** K1; K7.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** HY\tools\hydra_ui_mcp.py:753-760,796 with AR\ui-testing.md:107,123,217,218 (refs below the toolbar need `**/`; search box `**/##search`); AR\ui-testing.md:99 (no hook reads shell text, so the scroll-the-library guard rests on the rule alone); HY\tools\ch_mcp\quiet.py:35 `COMPILER_NAMES`, machine_report.py:43 reuses it; quiet.py:49 `REPORTED_NAMES`.
- **Splits:** None.
- **Ruling quotes:** None found. Finder: AR\rulings\clone-hero.md:72 (30 percent CPU) sits after the Unconfirmed heading at :67.
- **Tests:** none named.
- **Open:** all.
- **Hook gap:** n/a.

### L-F32-21: quiet.py cites a ruling that is not filed
- **Source:** I10b-007; F32; V32.
- **Kind:** K8.
- **Verdict:** CONFIRMED. The date exists but the cited ruling is not filed.
- **Homes:** HY\tools\ch_mcp\quiet.py:38 "The user's ruling of 2026-11-07: an MSBuild started with this flag is a helper" (flag `/nodemode:1`, lines 38-41,122-127,149); rulings dated 2026-11-07 are AR\rulings\agents-and-workflow.md:8, :9, :34 (builds, merge), none about MSBuild helpers; HY\docs\sessions\2026-11-07-7d216054.md:13 is a card, not a ruling.
- **Splits:** None.
- **Ruling quotes:** None found (grep of AR\rulings for nodemode, MSBuild, compiler: none).
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** n/a.

### L-F32-22: Way past the build gate (side note)
- **Source:** F32 N4; V32 (not reached).
- **Kind:** K5/K7.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** G\README.md:15 says the pass check "reads the structured tool name only, never shell text"; a build run through another route is stopped only by build_cpp.ps1 and Directory.Build.targets (HY\docs\development.md:138-141).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** whether such a route exists: needs a run.
- **Hook gap:** hook gap: yes, G\README.md:15 (the build-pass check); no steps.

---------------------------------------------------------------------
## F33 rows
---------------------------------------------------------------------

### L-F33-01: Cleanup sweep ages and retry numbers copied into prose and a second script
- **Source:** I2-087 to 100, I5-175, I9-164 to 168, I10-071, I7-043; F33 Group A; V33 candidates 1 and 2.
- **Kind:** K1.
- **Verdict:** CONFIRMED.
- **Homes:** G\cleanup_sweep.ps1:49,51,54,56 (`$StartGapHours = 1`, `$WorktreeIdleHours = 24`, `$ScratchAgeDays = 7`, `$InfoLogDays = 30`); HY\tools\hydra_ui_mcp.py:600 "gone a week with no writes; $ScratchAgeDays in cleanup_sweep.ps1 owns" (names the owner); :770 "removes it after a week with" (no owner); HY\docs\agents\ui-testing.md:213 "removes it after a week with no writes" (no owner). If the number changes, three prose copies go stale. Retry copy: cleanup_sweep.ps1:457 `-ge 100`, `Start-Sleep -Milliseconds 10`, comment :454 "wait as common._retry"; G\common.py:14 `_RETRIES, _RETRY_SLEEP = 100, 0.01`. Agree today; nothing ties them.
- **Splits:** None.
- **Ruling quotes:** None found (finder searched HY\docs\agents\rulings for week, hour, idle).
- **Tests:** none holding homes equal. G\tests\test_h52_cleanup_sweep.ps1 uses its own values, says "The ages themselves live only in cleanup_sweep.ps1".
- **Open:** none.
- **Hook gap:** n/a.

### L-F33-02: Which folders hold task worktrees (sweep vs delete rule)?
- **Source:** I2-093, I9-164; F33 SPLIT-1; V33 SPLIT-1.
- **Kind:** K6.
- **Verdict:** CONFIRMED.
- **Homes:** G\cleanup_sweep.ps1:174-178 `Get-SweepPrefixes` (`<root>\.claude\worktrees` for any repo, plus `<root>\worktrees` only when the repo root equals the claude root); G\delete_rule.py:17-18 `WORKTREE_FOLDERS = (H\.claude\worktrees, C:\Users\Patrick\.claude\worktrees)`; prose: AR\brief-preamble.md:43, AR\derive-once-review.md:197, G\README.md:136. V33: the sweep covers H\.claude\worktrees, ~\.claude\.claude\worktrees, ~\.claude\worktrees; the delete rule covers H\.claude\worktrees and ~\.claude\worktrees. Neither covers H\worktrees\<task>, which exists on disk and holds this audit's own worktree.
- **Splits:** One. Sweep and delete rule decide the same folder list in different shapes. They differ for a task worktree under H\worktrees: never swept, and not under the delete rule's worktree allowance.
- **Ruling quotes:** None names both sides.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** hook gap: yes, G\delete_rule.py (worktree folder list); no steps.

### L-F33-03: Which sessions still count as running (sweep vs card tools)?
- **Source:** I2-094, I4-107; F33 SPLIT-2; V33 SPLIT-2.
- **Kind:** K6.
- **Verdict:** CONFIRMED.
- **Homes:** G\cleanup_sweep.ps1:355-389 (still running = registry pid alive, or transcript written within `$ScratchAgeDays`, 7 days; comment :52-53); SC\prepass.py:31 `RUNNING_CUTOFF_SECONDS = 2 * 3600` (comment "Plan decision 6 (user-approved)"), used at prepass.py:391 and prune_list.py:114.
- **Splits:** One. They differ for a session quiet for 3 hours to 7 days: the card tools call it finished, the sweep calls it running. The "user-approved" comment is a claim; no Confirmed line found.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** n/a.

### L-F33-04: Posture resend interval of 60 minutes: whose answer is it?
- **Source:** I2-069, 070, 071, I5-162, I9-081; F33 Group B; V33 candidate 3.
- **Kind:** K1, provenance gap.
- **Verdict:** CONFIRMED-CHANGED. The finder quoted the plan as "Recommendation: 60"; V33 read the plan line as "Q8. The posture resend interval. T2 picks 60 minutes." (options 30, 60, once per session, 60 plus reset). The verifier's reading wins.
- **Homes:** G\posture.py:16 `RESEND_MINUTES = 60  # the user's answer to Q8 of the hooks plan HX-P2` (a comment claim); G\README.md:35 (rule spelled out, points at posture.py for the number); G\settings.fragment.json:109; HY\docs\superpowers\plans\2026-10-31-harness-hooks-and-tools.md:306.
- **Splits:** None. The code comment calls 60 the user's answer; the plan shows a pick.
- **Ruling quotes:** None found (grep of docs\agents\rulings for posture and Q8: nothing).
- **Tests:** G\tests\test_posture.py:74 uses `posture.RESEND_MINUTES` by reference; nothing holds the plan or README equal.
- **Open:** none.
- **Hook gap:** n/a.

### L-F33-05: Clone Hero MCP backup, device and save-file text
- **Source:** I10b-021, I10b-037, I7-066, 067, 068, 071, 072, 101; F33 Group C; V33 candidates 4 and 7.
- **Kind:** K1; K6 (device names).
- **Verdict:** CONFIRMED for the device names (K6). The prose homes were NOT VERIFIED (V33 did not reopen them). V33 adds that the `ch` MCP server's own instructions shown in this session restate the backup and device text, a prose home the finder did not list.
- **Homes:** HY\tools\ch_mcp\kit_tools.py:129 `KIT_DEVICES = ("keyboard", "mouse")` (comment cites "the user's rulings of 2026-10-30"); HY\tools\ch_mcp\agent\kit.ts:199 `type Device = "keyboard" | "mouse"`, :203-205, :380 `setDeviceOn("keyboard", false)`. Prose (finder only): AR\clone-hero-mcp.md:15,19,21,25,60,54,70; HY\tools\ch_mcp\README.md:75,81,89; HY\tools\ch_mcp\server.py:130-135,160,181-184,311,504,579; owner code HY\tools\ch_mcp\game_files.py (REG_CAP_S 30.0 comment "the user's ruling of 2026-10-30: Cap at 30 s").
- **Splits:** None named.
- **Ruling quotes:** AR\rulings\clone-hero.md:8 (Confirmed, heading at :7) 2026-11-13 chose "Leave the line changed"; :9 "stop messing with the game's save file. just let it stay." They cover settings.ini and the save file only. The 2026-10-30 rulings ("Cap at 30 s", keyboard and mouse "Switch it off (Recommended)") and the 2026-10-31 menu-walk keep (question 4) are cited in code and docs; V33 grep of AR\rulings for "Cap at 30|Switch it off|Sources" found no hits.
- **Tests:** HY\tools\ch_mcp\tests\test_server.py:266-270 pins one refusal text; no test holds docs and code equal.
- **Open:** prose homes unverified.
- **Hook gap:** n/a.

### L-F33-06: "Is this a clean merge of the base and a reviewed branch commit?" (ref gate vs review rules)
- **Source:** I3-018, I9-159; F33 Group D; V33 candidate 6.
- **Kind:** K6, K3 (a copy that claims to be the same rule and is not).
- **Verdict:** CONFIRMED. The "same rule" docstring claim is false by reading.
- **Homes:** G\ref_judge.py:305-316 `made_from_existing` (true if another ref holds the commit, or the commit has exactly two parents, parent 0 equals the current value, and `merge-tree --write-tree <both parents>` gives the commit's tree; docstring "(the same carry rule as review_rules._code_review)"); G\review_rules.py:234-244 `_carries` (any commit k in base..new that `passes(k)` accepts, with merge-tree(base,k) equal to new's tree; no parent-count check, no parent-0 check, no other-ref shortcut; used at :251). G\README.md:53 names `restore_checkout` in ref_judge.py as owner and spells the rule.
- **Splits:** One. They differ for a commit already on another ref; a merge whose parents are in the other order; a merge of base with a branch commit `passes()` rejects.
- **Ruling quotes:** None names both sides.
- **Tests:** G\tests\test_ref_judge.py:451-605 test `restore_checkout` only; nothing holds the two merge rules equal.
- **Open:** behaviour on such a commit: needs a run; not run.
- **Hook gap:** hook gap: yes, G\ref_judge.py and G\review_rules.py (carry rule); no steps.

### L-F33-07: Session-card tool folder names and thresholds
- **Source:** I4-104, I4-106, I9-173; F33 Group E; V33 candidate 5.
- **Kind:** K1.
- **Verdict:** CONFIRMED for the folder name (two literals of one folder name). The test repeats were not opened. The other rows are NOT VERIFIED.
- **Homes:** SC\prune_list.py:23 `PROTECTED_FOLDERS = ("memory", "session-cards")`; SC\common.py:34 `DATA_ROOT = os.path.join(PROJECTS_ROOT, PROJECT_NAME, "session-cards")`; SC\test_session_cards.py:1095,1170 repeat both names; SC\writer-brief.md:25 states the data folder path. I4-104 (finder only): SC\migrate.py:26-27 `SOURCES`, common.py:49 comment "the user dropped it on 2026-10-31", test_session_cards.py:1173 `OLD_CARD_TAIL`. I9-173 (finder only): G\session_cards_start.ps1:45 `$RemovalReminderMb = 500`, :47 `$ReminderGapHours = 24`; the test uses 499.9, 500.0 and 25 h of its own.
- **Splits:** None.
- **Ruling quotes:** None found (grep Sources, dropped, 2026-10-31: only unrelated lines).
- **Tests:** SC\test_session_cards.py repeats the names.
- **Open:** I4-104 and I9-173 rows not reopened.
- **Hook gap:** n/a.

### L-F33-08: Which card folders does a bare check run count, and does the ruling judge read the same ones?
- **Source:** I4-078; F33, F33b; V33b.
- **Kind:** K6 candidate.
- **Verdict:** CONFIRMED.
- **Homes:** SC\check.py:213-226 `default_cards` (published, local, then staged; staged wins by name; index.md left out); G\ruling_rules.py:214-215 `cards_roots = [cards.CARDS_ROOT, cards.local_cards_dir(cards.DATA_ROOT)]` (no staging folder, does not use default_cards); SC\link_memory.py:70-71; SC\number_rulings.py:60 (different question).
- **Splits:** Only if both claim "the cards": ruling_rules reads no staging folder, check.py does. Situation: a card in staging only.
- **Ruling quotes:** None found.
- **Tests:** SC\test_session_cards.py:346,685-690; none ties ruling_rules.py:215 to `default_cards`.
- **Open:** none.
- **Hook gap:** n/a.

### L-F33-09: Which paths may be written to the prune list?
- **Source:** I4-108; F33b; V33b.
- **Kind:** K1; K6 weak.
- **Verdict:** CONFIRMED-CHANGED (weak). V33b: `check.py:38 CARD_FILE` (8-hex prefix of a card name) answers a different question from `prune_list.SESSION_ID` (full UUID transcript name), so the K6 is thin. The K1 claim about common.py path pieces was not re-opened.
- **Homes:** SC\prune_list.py:1-9,23-24,62-74,123; SC\common.py:34-35; SC\check.py:38; G\session_cards_start.ps1:16,346-350.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** SC\test_session_cards.py:685,788; none holds PROTECTED_FOLDERS equal to common.py.
- **Open:** common.py piece unverified.
- **Hook gap:** n/a.

### L-F33-10: kResultsStamp release rule lives only in a memory note
- **Source:** I8-169; F33; V33 candidate 8.
- **Kind:** K7.
- **Verdict:** CONFIRMED.
- **Homes:** MEM\stored-versions-one-rule.md:17 "at every release, if analysis output changed since the last bump, bump kResultsStamp" (and :3, MEM\MEMORY.md:27); owner HY\src\store\stored_versions.h; grep of HY\docs\agents and HY\.claude found no hit.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none; nothing checks it.
- **Open:** none.
- **Hook gap:** n/a.

### L-F33-11: Wrong owner name in session_cards_start.ps1
- **Source:** side note; F33, F33b; V33 candidate 10.
- **Kind:** K8.
- **Verdict:** CONFIRMED.
- **Homes:** G\session_cards_start.ps1:21 "so state_prune.ps1 leaves it alone"; no such file in G; G\cleanup_sweep.ps1:16,28 name `Invoke-StatePrune` inside cleanup_sweep.ps1.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** n/a.

### L-F33-12: Is the hand-back report saved before the budget check, and which tools count as report tools?
- **Source:** I1-012; F33, F33b; V33b.
- **Kind:** K6, K1.
- **Verdict:** CONFIRMED.
- **Homes:** G\percall.py:99-102 (comment "Saved first, so a report past the budget is kept"; `if data.get("tool_name") == "SubagentHandback"` saves the message); G\progress.py:18-19 `REPORT_TOOLS = ("SubagentHandback", "StructuredOutput")`, :119, percall.py:168; G\agent_stop.py:40; G\README.md:11,41,75,182; G\tests\probe_results.md:71; reviewer.md:25.
- **Splits:** One. A StructuredOutput report is exempt from the budget but never saved, so `agent_stop` has no saved hand-back for it. Situation: a subagent that hands back through StructuredOutput.
- **Ruling quotes:** None found.
- **Tests:** G\tests\test_progress.py:209,370 loop over both tool names for the exemption; :389-412 test the save for SubagentHandback only; G\tests\test_agent_stop.py:265,525-549. Nothing ties the two lists.
- **Open:** whether a live subagent ever uses StructuredOutput: needs a live run.
- **Hook gap:** hook gap: yes, G\percall.py and G\progress.py (report-tool lists); no steps.

### L-F33-13: How do wake watches avoid piling up?
- **Source:** I2-061; F33, F33b; V33b (not re-opened).
- **Kind:** K1.
- **Verdict:** NOT VERIFIED (V33b: finder says single home; not re-opened).
- **Homes (finder only):** G\wake.py:25,30-34 (token under `WATCH_SUFFIX = ".wake.json"`; "An older watch sees the new token on its next pass and ends quietly, so watches never pile up."); G\README.md:31.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** G\tests\test_wake.py:145-152 (behaviour, not the README text).
- **Open:** all.
- **Hook gap:** n/a.

### L-F33-14: When is a finding's triage row updated?
- **Source:** I6-143; F33, F33b; V33b.
- **Kind:** K7, K1.
- **Verdict:** CONFIRMED by the finder's quotes; V33b did not re-open and did not widen the search.
- **Homes:** AR\audit-method.md:110 "The merge that fixes a finding updates its row in the same step."; :168-169 ("192 of 396 findings open"); :108. The triage files (HY\docs\audit\2026-10-03-fix-triage.json/.md) are outside the finder's scope list.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** n/a.

### L-F33-15: Test-rubric examples page: commit hash, history text, G1 removal rule
- **Source:** I7-137, I7-138, I7-139; F33b; V33b.
- **Kind:** K1 (history told in two pages), K7.
- **Verdict:** I7-137 and I7-138 NOT VERIFIED (V33b: not re-opened). I7-139 CONFIRMED-in-part (the removal rule has no check; not re-opened).
- **Homes (finder only):** AR\test-rubric-examples.md:210 (commit 3b699581, finder: commit exists, subject "Checkpoint: complete C++ port through Phase 6 alongside the Python oracle"); :338-342, :5, :218 ("If a later reader finds an answer in that record that is wrong, G1 comes off this page; it does not stay with a note."); AR\test-rubric.md:13-15,25,84; HY\tests\test_timing.cpp:73 holds the quoted test name.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** nothing checks the page against the test (a rename drifts it).
- **Open:** all not re-opened.
- **Hook gap:** n/a.

### L-F33-16: "The machine wins; fix the line in the same commit"
- **Source:** I7-162; F33b; V33b.
- **Kind:** K1, K3 (different actors), K7.
- **Verdict:** CONFIRMED. V33b: different questions (this-machine lines vs Clone Hero pages), so the preamble difference is a K3-style wording gap only.
- **Homes:** AR\this-machine.md:3 "If a line here disagrees with what you see, the machine wins. Fix the line in the same commit as the work that found it."; HY\CLAUDE.md:9 (a session or agent updates a Clone Hero page in the same commit); AR\brief-preamble.md:105 (limits it to an agent whose brief owns the page, otherwise report it); AR\agent-rules.md:15; AR\brief-preamble.md:91-95.
- **Splits:** None proven.
- **Ruling quotes:** None found.
- **Tests:** the docs check does not test dates.
- **Open:** none.
- **Hook gap:** n/a.

### L-F33-17: The CI build folder rule and its test
- **Source:** I10b-113; F33b; V33b.
- **Kind:** K1.
- **Verdict:** CONFIRMED-CHANGED. The copy is real, but the finder's line numbers were off: `$script:OldTime = [datetime]::new(2000, 1, 1, ...)` is at ci_build_folder.ps1:51 (the :21-32 range is `$script:BuildFiles`).
- **Homes:** HY\tools\ci_build_folder.ps1:21-32 (`$script:BuildFiles`), :51 (`$script:OldTime`); HY\tools\test_ci_build_folder.ps1:1-15,61,66,99-105 (own copy of the 2000-01-01 date; one list entry); HY\docs\development.md:882-890 (outside scope); .github\workflows\ci.yml:466-469.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** HY\tools\test_ci_build_folder.ps1 pins the pair by behaviour (it fails if OldTime changes).
- **Open:** none.
- **Hook gap:** n/a.

### L-F33-18: How long hydra_stop waits: one number for two waits
- **Source:** I10-066; F33, F33b; V33b.
- **Kind:** K3.
- **Verdict:** CONFIRMED.
- **Homes:** HY\tools\hydra_ui_mcp.py:41-42 ("This is the one home of these three numbers."; `REPLY_MARGIN_S = 15.0`, comment "how much longer than the timeout in force to wait for a reply"); :459 reuses it as the process-exit wait in `_stop_session`; :566,:636 reply wait `s.timeout + reply_margin_s`; AR\ui-testing.md:214 (no length), :280 (names the constant as the wait for each reply).
- **Splits:** None in value. One number means two waits.
- **Ruling quotes:** None found.
- **Tests:** HY\tools\test_hydra_ui_mcp.py:489,605,1033,1039 use `hm.REPLY_MARGIN_S` (read the owner, no copy).
- **Open:** slow shutdown behaviour not run.
- **Hook gap:** n/a.

### L-F33-19: Who copies the Clone Hero stats file, and where?
- **Source:** I7-096; F33, F33b; V33b.
- **Kind:** K2, K1.
- **Verdict:** CONFIRMED.
- **Homes:** HY\tools\ch_mcp\README.md:83 ("copies ... only for a song `ch_start` was given a folder for, which the bot plays from the command line. When a person plays the song instead, the game's own ... scorestats.json still holds that run after `ch_stop`"); AR\clone-hero-mcp.md:60 ("whether the bot or a human player (`human=True`) plays it") and :15; HY\tools\ch_mcp\bot_run.py:755-761 sets `cap_s` whenever a song folder is given; :799 refuses only when `run.cap_s is None`; :752 `human_refusal`; HY\tools\ch_mcp\game_files.py:36,40 (path derived from `_HOME`); HY\docs\clone-hero\06-scoring.md:379.
- **Splits:** One. README:83 says a human-played folder launch is not copied; clone-hero-mcp.md:60 and the code say it is. Situation: `human=True` with a song folder. Also (K1): the literal path with the user's name appears at clone-hero-mcp.md:15 and :60 while game_files.py derives it from `_HOME`.
- **Ruling quotes:** AR\rulings\clone-hero.md:9 (Confirmed, heading verified at :7) "stop messing with the game's save file. just let it stay." Does not name either side (it covers leaving the file alone).
- **Tests:** none holding page and code equal.
- **Open:** none.
- **Hook gap:** n/a.

### L-F33-20: Other F33 single-home or pointer rows
- **Source:** I3-094, I3-099, I3-080, I5-030, I8-190, I8-224, I10-040, I10-066 (see L-F33-18); F33; V33 (not reached; item 9 "ytdl note" explicitly NOT REACHED).
- **Kind:** K1, K7.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** G\sync_settings.py:8-17 with test_sync_settings.py:13-180; G\settings.fragment.json and the live settings.json (G\tests\test_docs_numbers.py:142 `test_live_settings_carry_the_fragment`, skipped in a worktree); G\ruling_rules.py:108 `CHECK_COMMAND` with copies at settings.fragment.json:6 and live settings.json:12 (G\tests\test_docs_numbers.py:137 holds them equal); HY\CLAUDE.md:57 (finished plan or handoff leaves the repo; checked by the docs check); MEM\ytdl-folders-not-a-project-matter.md:11 (no hook, no doc); skills\reverse-engineer-anything\SKILL.md:341-342 with references\javascript-applications.md:173 and connection-and-recovery.md:84; HY\tools\build_slot.ps1 `Test-BuildSlotLive` :141-146 with HY\tools\test_build_pass.ps1:161-214.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** as named.
- **Open:** all.
- **Hook gap:** n/a.

### L-F33-21: Side note: slot lock folder lives apart from the gates state folder
- **Source:** F33 side notes; V33 (not reached).
- **Kind:** none stated.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** HY\tools\build_slot.ps1:10,78 keep slot locks in C:\Users\Patrick\.claude\hooks\state\build_slots while the gates state is G\state; both folders exist.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** all.
- **Hook gap:** n/a.

---------------------------------------------------------------------
## End list
---------------------------------------------------------------------
Candidates the verifier REFUTED: none (V31, V31b, V32, V33, V33b each report no refutation).

Rows no verifier reached (NOT VERIFIED): L-F31-13, L-F31-14, L-F31-15, L-F31-16, L-F31-27, L-F32-02, L-F32-04, L-F32-05, L-F32-06, L-F32-07, L-F32-11, L-F32-12, L-F32-14, L-F32-16, L-F32-17, L-F32-18, L-F32-19, L-F32-20, L-F32-22, L-F33-13, L-F33-15 (I7-137 and I7-138 parts), L-F33-20, L-F33-21. Parts not reached inside otherwise verified rows: the prose homes in L-F33-05; the I4-104 and I9-173 rows in L-F33-07; the agreeing-homes part of L-F32-10; the K1 common.py claim in L-F33-09.

Hook gaps named: L-F32-22, L-F33-02, L-F33-06, L-F33-12, and a possible one in L-F32-08.
