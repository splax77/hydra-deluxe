# TOOL-AUDIT-T5-W02 (second-round verifier, 12 rows)

All claims are by reading. H = C:\Users\Patrick\Downloads\Hydra\hydra-test; G = C:\Users\Patrick\.claude\gates; M = the memory folder. I ran nothing. For rulings: `## Unconfirmed` starts at review-and-hooks.md:37, agents-and-workflow.md:32, code-and-engine.md:61, clone-hero.md:67, questions-and-audits.md:24 (checked with a heading search).

### L-F02-11
**Verdict:** CONFIRMED.
**Homes as found:**
- H tools\replay_schedule.cpp:26 `constexpr double kFrameMs = 3.0;` (comment :21-25 "The user set it: question 1 of docs/superpowers/plans/2026-11-08-player-freak-fixes.md"); :31 `kJudgeEarlyMs = 1 * kFrameMs;` (comment :28-30 "the user chose "One frame", D319 item 3"); :67 `{"frame_ms", kFrameMs}` exported.
- H docs\agents\derive-once-review.md:64 "3.00 ms is the one frame of judging the tool counts" (a value copy inside a worked example).
- H src\core\rules.h:32 `double backend_leeway_ms = 3.0;`, read in src\core\backend_value.h:37-38 and model.h:98, :471-473. A different rule, same number. AM:13 and :135 "3" is this leeway, not the frame.
- Also noted: replay_schedule.cpp:57 `kRoomMarginMs = 3.0`, a third 3.0 with its own meaning.
**Splits:**
- Tool frame (3.0 ms, how long after a press the game judges it) against engine leeway (3.0 ms, how far past a Star Power end a back-end note still counts). Different questions; they match today only by number.
- derive-once-review.md:64 types "3.00" by hand beside `kFrameMs`.
**Ruling:** code-and-engine.md:13 (Confirmed, 2026-10-29): "the 3ms rule is for whether a backend is included in the optimal score or uncounted. it's an arbitrary number chosen by dragondelgar..." It speaks to the leeway only. I found no Confirmed line with "One frame": a search of rulings\*.md for "One frame" found nothing. So the code comments' "the user chose" claims have no Confirmed line (a search for the words finds none; the answer may sit in D319, which I did not open).
**Open:** whether D319 item 3 exists in docs\audit\2026-10-03-fix-decisions.md as quoted; not opened.

### L-F02-12
**Verdict:** CONFIRMED.
**Homes as found:**
- H tools\precheck\items.h:50 `constexpr long long kLargeRangeLines = 400;`, comment :44-49 "The user's decision is D82 item 2 ... a starting value to be tuned later. This is the one place it lives." Used at tools\precheck\items.cpp:102,104 (message built from the constant).
- H docs\agents\derive-once-review.md:21 "The precheck owns that threshold ... this page never restates them" and :193 "the threshold that `hydra_precheck.exe` owns" (pointers, no number).
- H tests\test_precheck_git.cpp:200-204 literal text "401 changed lines is over the 400-line threshold" and `RangeSize{400, 0}` (a literal copy of 400); tests\test_precheck.cpp:417 reads the constant.
- No 20-line threshold in code: searched G\ref_judge.py, review_rules.py, review_dispatch.py for "20 lines", "20-line", "400" and found nothing.
**Splits:** 
- Confirmed ruling "only large items over 20 lines should go for review" (review-and-hooks.md:22) against the code's 400-line warning (a warning, not a gate, per test text). Different questions: what goes to review at all against when a range is too big for one read. Nothing in G applies 20.
- The test literal 400 against the constant: second copy, equal today.
**Ruling:** review-and-hooks.md:22 (Confirmed): "opus, retire, retire, minutes only, only large items over 20 lines should go for review, retire. send agents to compile a full plan". It does not name 400. The ">20 lines" at :50 is under Unconfirmed.
**Open:** whether any gate applies the 20-line ruling; I searched three G files only (not the whole G), so "no gate" is partial.

### L-F02-13
**Verdict:** CONFIRMED-CHANGED. The row's kind K4 is weak: audit-method.md:104 itself says the number is the main session's own suggestion, so it is not "shown as settled" there.
**Homes as found:**
- H docs\agents\audit-method.md:104 "The user approved that rule on 2026-10-29 ("yes both"). The number is not the user's: "five or fewer per message" is the main session's own suggestion for what "small" means".
- H docs\superpowers\plans\2026-11-14-test-suite-audit.md:194 "five or fewer per message is the main session's own reading of "small"" (also says it is not the user's).
- docs\superpowers\plans\2026-11-08-player-audit.md:102 "Questions go out five or fewer per message" and 2026-11-08-clone-hero-docs-gaps-search.md:187 "Splits reach the user five or fewer per message" (both state it with no "own suggestion" note); docs\audit\2026-11-08-player-audit\questions.md:3.
- No hit in CLAUDE.md, the user's CLAUDE.md or the memory notes for "five or fewer" (searched those plus docs\agents).
**Splits:** the two plans (player-audit.md:102, docs-gaps-search.md:187) state the number as the plan's rule; audit-method.md:104 and test-suite-audit.md:194 say it is the main session's own suggestion. Same number, different standing.
**Ruling:** the "yes both" line is at questions-and-audits.md:28, below the Unconfirmed heading at :24, so it is not a Confirmed line.
**Open:** none.

### L-F02-14
**Verdict:** CONFIRMED.
**Homes as found:**
- H docs\agents\premise-map.md:68 "A step holds at most one `(never tried)` call. A step with two or more is a shape error".
- H tests\test_docs_match_code.cpp:1844-1851 `int never_tried`, `if (never_tried >= 2)` pushes "(never tried) calls; split it so each step tries one new piece" (the check holds the rule; `never_tried` counted at :1875, reset :1866).
- H docs\agents\clone-hero-mcp.md:58 names each `(never tried)` call to the user (a related rule about asking, not the one-per-step limit; the finder's use of it as a home is a stretch).
- Plans docs\superpowers\plans\2026-11-07-ch-run-timing.md:413 and :463 restate it ("at most one (never tried)"; "P31 ... at most one never tried").
**Splits:** none. The prose says "two or more is a shape error"; the test says `>= 2`. Same edge. premise-map.md does not name the test that enforces it (finder's point holds).
**Ruling:** none found (no search of the full rulings set beyond keyword greps for related words).
**Open:** none.

### L-F02-15
**Verdict:** CONFIRMED-CHANGED. The row says `kBackendInnerBandMs` and counts are "stated twice in the page". What I found: the constant is named at AM:135 only as deleted; no source file uses it; and the 3 / 10 ms ladder text is at AM:13 and AM:135 as history, not as a live rule.
**Homes as found:**
- H docs\agents\audit-method.md:13 "A plain row read "Standard" from -10 ms to under 3 ms ... "Standard SqOut" from -10 ms to under 10 ms" (past tense, "until D269 gave them one rule").
- audit-method.md:135 "where 10 was kBackendInnerBandMs, a constant D269 has since deleted along with both ladders"; commit c9c00cf7 named. A search of src, tests and tools for `kBackendInnerBandMs` found no hit.
- Live owner: src\core\rules.h:32 `backend_leeway_ms = 3.0`; src\core\backend_value.h:37-56 (`counted_without_squeeze` is `offset_ms < leeway_ms`, strict). AM:135 "3 is the default of the leeway setting" agrees.
- AM:190 names `BackendSqueeze::summarystr`; src\core\model.cpp:342 has it (taking `leeway_ms`). True.
- The counts at AM:140-178 ("33 questions", "219 closures", "about one in nine", "26, 14, 11 and 12") are present (grep found "219", "one in nine", "26, 14"). Their repeats in other files I did not open.
**Splits:** none. Historical text against current code: the page says the 10 ms band was deleted and the code agrees. K1 is only history.
**Ruling:** code-and-engine.md:12 and :13 (Confirmed, 2026-10-29): ":12 "there should never be two different answers for where star power ends. ever. the 3ms rule is for whether a backend is included in the optimal score or uncounted..."" It names the 3 ms side. The deleted 10 ms side is not named.
**Open:** the repeats of the counts in docs\audit\2026-10-29-audit-of-audits\blocks and others were not opened.

### L-F02-16
**Verdict:** CONFIRMED-CHANGED. The row says "nine versus ten sources"; I counted ten linked sources in public-doc-style.md:7 (Wikipedia, tropes.fyi, Google, Microsoft, digital.gov, Write the Docs, Diataxis, ripgrep, YARG, Moonscraper). The research file lists 74 `[S..]` tag lines/mentions (grep count; sources run [S1]..[S27] per the finder, not recounted). I found no place that states "nine".
**Homes as found:**
- H docs\agents\public-doc-style.md:7 "The rules come from two web searches on 2026-10-10. The main sources were ...", ten links. Line 9 area says tags like [S19] point to docs\research\ai-generated-docs-review.md (the file is where "this method comes from").
- H docs\agents\doc-review.md:11 "Tags like [S19] point to the numbered sources at the end of `docs/research/ai-generated-docs-review.md`. That file is where this method comes from". (This text is in doc-review.md line 11; public-doc-style.md names its own source list on line 7.)
- docs\research\ai-generated-docs-review.md header "Research scout write-up, 2026-10-08" (two days before the "web searches on 2026-10-10").
- H docs\clone-hero\06-scoring.md:127 "two web searches (one on a setlist combo ...)" on 2026-11-11: an unrelated fact with the same words.
**Splits:** the style page's "main sources" (ten, searched 2026-10-10) against the research file's own 27 numbered sources (dated 2026-10-08). Two lists feeding the "rules come from" claim; the page does not say whether the 10 are a subset of the 27. Dates differ by two days.
**Ruling:** none found.
**Open:** whether the ten are a subset of the research file's sources (not compared).

### L-F02-18
**Verdict:** CONFIRMED.
**Homes as found:**
- H docs\agents\clone-hero-mcp.md:48 "prints a line with the gaps over 50 ms between samples"; owner H tools\ch_mcp\sp_tools.py:62 `GAP_MS = 50`, line built at :499-500 `f"gaps over {GAP_MS:g} ms between samples: ..."` from the constant.
- clone-hero-mcp.md (the Star Power paragraph) "`sp_half_u64`, the game's field at engine `+0x1B8`"; owner H tools\ch_probe\constants.py:128 `OFF_SP_HALF = 0x1B8`. Same offset in game-fact pages docs\clone-hero\04-fills.md:214, 05-star-power.md:107 and :169, 07-internals.md:309 and :312.
- clone-hero-mcp.md "`sp_tools.ready_lines` owns the line"; sp_tools.py:516 `def ready_lines` exists, called at :329 and :355. True.
**Splits:** none. Value copies agree today. The CH-MCP page restates a game offset that belongs on the clone-hero pages (the project's rule is that game facts live on those pages).
**Ruling:** none found.
**Open:** no test ties the page's 50 to GAP_MS (I searched the docs check only by the finder's report, not opened).

### L-F02-19
**Verdict:** CONFIRMED-CHANGED. The row says CH:45 names the bot_run.py caps. I grep-searched clone-hero-mcp.md for `LAUNCH_` and found no hit, so CH:45 gives no launch cap names by that search.
**Homes as found:**
- H tools\ch_mcp\trace_hits.py:68-70 `TRACE_CAP_MARGIN_S = 30.0` ("question 5 of the feasibility-tests plan ... the end is D331's"); :74 `WALL_CAP_PAUSE_S = 120.0` ("approved by the user on 2026-10-28").
- H tools\ch_probe\prep_inputs.py:50 `REPLAY_CAP_S = 120.0` ("The user chose 120 s ("Use 120 s", 2026-11-07, session d0a3868c)").
- H tools\ch_probe\engine_finder.py:38 `ENGINE_SEARCH_CAP_S = 120.0` (a third 120.0; a different wait).
- H tools\ch_mcp\bot_run.py:88-90 `LAUNCH_ATTACH_CAP_S = 20.0`, `LAUNCH_ARM_CAP_S = 5.0`, `LAUNCH_PATCH_CAP_S = 30.0`.
- H docs\agents\clone-hero-mcp.md step 7 names `trace_hits.TRACE_CAP_MARGIN_S` and `prep_inputs.REPLAY_CAP_S`, and step 6 names `engine_finder.ENGINE_SEARCH_CAP_S`, all by name, no number (pointers). Step 1 text I read has no launch cap numbers.
- H tools\ch_mcp\tests\test_trace_hits.py:180-181 and :462-463 comments "6,000 ms + 30 s margin = 36 s song cap; plus 120 s of pause room = 156 s".
**Splits:** the three 120.0 values answer three different questions (pause room on the wall clock, the replay call, the engine search). Test comments copy 30, 120, 36 and 156 by hand beside the constants.
**Ruling:** clone-hero.md:25 (Confirmed per the finder; I did not re-open it): chose "Hydra's end of the chart (Recommended)". It names the cap's base, not the 120 or 30.
**Open:** CH:45's content differs from the row (not reached in full).

### L-F02-20
**Verdict:** CONFIRMED-CHANGED. The four homes exist and agree, but they are two different claims, not one claim four times.
**Homes as found:**
- G\README.md:118 "Registrations reload while a session runs. An edit to a watched settings file reaches every running session on its next call ... The probe saw this for a project settings file and infers it for the user's `settings.json`, which it did not touch."
- G\README.md:190 "An edit to a watched settings file reaches a running session on its next call; an edit to a `--settings` file does not."
- G\tests\probe_results.md:49-52 line "6 | A change to the `hooks` block reaches a running session | Yes for a watched settings file (the project `.claude\settings.json`). No for the file passed with `--settings`. The user's `settings.json` was not edited, so it is covered only by the same watcher, not seen directly" and the "Probe 6 (live reload)" paragraph.
**Splits:** README:118 says "reaches every running session" and "adding or removing a hook takes one merge and one sync" about the user's settings, which the probe did not test (README:118 itself says "infers"; probe_results says "not seen directly"). README:190 states it without that caveat. Same claim, different strength. By reading only; a run would settle the user settings.json case.
**Ruling:** none found.
**Open:** a run that edits the user's settings.json mid-session.

### L-F03-04
**Verdict:** CONFIRMED-CHANGED. The numbers are in the memory note, but the "K7 no hook" framing is partly wrong: the note is not a rule nothing checks stated in several places; it is one note plus its index line.
**Homes as found:**
- M\lean-agent-fanout.md frontmatter description "batch tiny uniform jobs about 8 per agent, 3-4 agents per discovery round" and body "Batch small uniform jobs (merges, rewrites) about 8 per agent." and "For repeated discovery rounds, use 3-4 agents ... More agents per round makes "stop when a round finds nothing" harder to reach".
- M\MEMORY.md:39 "~8 jobs per agent, 3-4 agents per round".
- Searches: docs\agents, G\*.py and *.md, the user's CLAUDE.md, model-postures.md and agents\*.md for "8 per agent", "3-4 agents", "jobs per agent": hits only in G\state\agents\...handback files (run records of this audit's own agents). No repo doc or hook states them.
**Splits:** none. Edge: the note says "3-4 agents" for discovery rounds only; the MEMORY.md index line says "3-4 agents per round" (shorter, loses "discovery").
**Ruling:** none under Confirmed carries the numbers. agents-and-workflow.md:48 (a line before the Unconfirmed heading at :32? No: :48 is after :32, so Unconfirmed) quotes "do we really need 54 merge agents?" and "was running 8 agents per round really necessary?" The user's "8 agents per round" was about too many agents, while the note's "about 8 jobs per agent" is a batching size: both use 8 for different things.
**Open:** none.

### L-F03-12
**Verdict:** CONFIRMED.
**Homes as found:**
- M\question-the-design-before-patching.md body: "When the same goal fails twice and each fix works around the last failure, stop and put the design itself to the user, with the alternatives, before planning another workaround." and description "2+ failed runs on one goal".
- M\MEMORY.md "2+ failed runs on one goal".
- H docs\agents\brief-preamble.md:85 "If your last two status lines show the same step and no new result, you are not making progress. Stop and report what you tried." (an agent's own stop-and-report rule; different actor: it reports to the main session, not the user, and counts status lines).
- Searched G\*.py, G\*.md, docs\agents\*.md, model-postures.md and agents\*.md for "failed twice", "fails twice", "same goal", "two failed", "failed runs": no hit for a rule in a hook or doc.
**Splits:** the note: same goal failing twice means the main session asks the user about the design. Preamble :85: an agent whose last two status lines repeat stops and reports. Same trigger shape (two), different actor and different recipient. "twice" and "2+" agree.
**Ruling:** none Confirmed found (keyword search).
**Open:** AR:9 as the finder cites it as a related home was not read by me.

### L-F03-16
**Verdict:** CONFIRMED-CHANGED. Pointers have drifted, and not by 2 only in two places: every song.cpp line number I checked on the drum-lane rows is off by about 2.
**Homes as found:**
- H docs\clone-hero\02-note-list.md:611 "`light_drum_lanes` in src/parse/song.cpp line 203 and `lights_lane_pad` line 88, with the pick in `game_lane_pads` line 147"; :631 "lines 203 to 213, called at line 2009; ... `every_lane_pad` (line 213)"; :641 "lines 203 to 213 ... its comment at lines 200 and 201"; docs\clone-hero\questions.md:270 "line 203".
- Code: src\parse\song.cpp:205 `light_drum_lanes(`; the call at :2204 `light_drum_lanes(std::move(song.drum_lanes), ...)`; `lights_lane_pad` at :90; `game_lane_pads` at :149; `every_lane_pad` defined :175, used :215. So 203 against 205, 2009 against 2204 (large), 88 against 90, 147 against 149, 213 against 215.
- docs\clone-hero\02-note-list.md was last changed in commit 4e57998f (2026-11-12). Other pointers in the same file (e.g. lines 15, 25, 35, 105: "lines 708 to 710", "1746 to 1773") I did not check.
- M\drum-lanes-d112-2026-10.md:17 holds the "4 of 50 lanes" claim (one home; not verified by me).
**Splits:** pointer lines in the pages against the code's real lines; the call-site gap (2009 against 2204) is not a small shift, so something between added about 190 lines before the call.
**Ruling:** none found (ui-and-display.md:117 not opened).
**Open:** whether the docs check reads line pointers at all (I only grep-searched test_docs_match_code.cpp for "song.cpp line" and found nothing; a name search proves little).

## Ids not reached
None. All 12 rows were read (some homes the finder named were not opened, listed in each Open line).
