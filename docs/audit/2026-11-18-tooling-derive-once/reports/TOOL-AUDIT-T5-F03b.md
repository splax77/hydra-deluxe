# TOOL-AUDIT-T5-F03b (re-send of F03)

Agent a376c0eb6941869e6. Hit the 100-call warning, so every row got one pass of targeted greps, not a full sweep. Abbreviations: M = C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory ; H = C:\Users\Patrick\Downloads\Hydra\hydra-test ; G = C:\Users\Patrick\.claude\gates.

Rows taken (exact ids from F03's "Rows not reached" list, plus the two it marked partial): I8-012, I8-050, I8-057, I8-080, I8-126, I8-148, I8-154, I8-163, I8-195, I8-196, I7-155, I7-189, I7-224, I7-226, I7-131, I7-133.

Not reached in depth: I8-196 (only the skill line and docs/clone-hero/05 greps; no home found for "collected phrase extends SP" besides the skill), I7-155 and I7-226 (single home, light search). Gates tests folder was searched only for KEY_HOLD_S / hold-equal tests by name pattern in this pass, not read file by file. The F03 note that gates tests know nothing about these numbers stands unverified by me.

Search method: Grep by question words and by number, over H (CLAUDE.md, docs/agents incl. rulings, docs/clone-hero, docs/adr, docs/audit decision log, .claude/commands, tools, tests, src), the ~/.claude files (CLAUDE.md, model-postures.md, agents, skills, gates .py/.md/lib), M (not archive), and the separate repo C:\Users\Patrick\Downloads\Hydra\squeeze-tools.

## I8-012 What numbers back the "approved work is not re-asked" cases (startup bar read about 130 ms vs a 0.15 s screen delay; T7 about 8 ms on 8 threads; floor tint 0.45 vs 0.2)
Homes:
- M\approved-work-is-not-reasked.md:11 "takes about 130 ms (under the 0.15 s screen delay)"; :13 "the threaded chart list, about 8 ms"; :15 "floor tint 0.45 ... or keep today's 0.2".
- H\docs\agents\rulings\code-and-engine.md:63 (under `## Unconfirmed`, line 61): "T7 (reading the chart list on 8 threads, measured at 5.4 ms against 13.8 on 1)". A home.
- H\docs\agents\rulings\ui-and-display.md:97 (`## Confirmed`): the user chose "Yes, counted bar" to a question quoting "takes over 0.15 s to open". Ruling found, not a home.
- Code owner of 0.15 s: H\src\ui\app_state.h:434 `kViewProgressDelaySeconds = 0.15` ("a fast open shows an empty window", :298). Second 0.15 in the same file :445 `kSearchThrottleSeconds = 0.15` (different question, same number).
- H\docs\agents\ui-testing.md:124 states 0.15 s for "Analyzing chart... / Cancel once it has run 0.15 s" (a copy of the same constant for the analysis screen; the code names it kViewProgressDelaySeconds).
Rulings (Confirmed): H\docs\agents\rulings\ui-and-display.md:9 "yes i approved the mockup and asked for option B. so why are you offering what we have today as an option?" ; H\docs\agents\rulings\code-and-engine.md:8 "why are you trying to convince me to skip something i already approved?".
Split: T7 timing: the note says "about 8 ms", the ruling-file line says 5.4 ms on 8 threads vs 13.8 ms on 1. Different figures for the same step.
Kinds: K1 (0.15 s in the note, the ruling, ui-testing.md:124, code), K2 (8 ms vs 5.4 ms).
Edges: the rule "after a yes, do not re-ask" is held only by the note and MEMORY.md:5. agent-rules.md rule 3 (H\docs\agents\agent-rules.md:9) says the opposite direction (work waits for a yes) and "when the user asks for one specific edit, the request is the approval"; no hook reads this. K7 candidate for "do not re-ask": the note, MEMORY.md index line, two rulings, no home named as owner. Test holding equal: none found.

## I8-050 dmleaderboards latency and fetch rules (warm ~650 ms, cold 30 s+, Cloudflare rejects empty User-Agent, background job, real UA, long timeout)
Homes:
- M\dmleaderboards-comparison.md:19-20 (the row).
- H\src\net\http.cpp:172-174 "A real User-Agent: Cloudflare ... rejects empty or missing ones with a 403" and :180-185 "Generous receive timeout: the render.com backend cold-starts ... tens of seconds", `WinHttpSetTimeouts(session_.h, 15000, 20000, 30000, 120000)`.
- H\src\net\dmbot_client.h:3-12 "Cloudflare (which 403s bot fetches...)", "first request after a lull can take tens of seconds", "off the render thread (see ui/dm_jobs.h) with a generous timeout and a cancel flag".
What each tells the reader: the note tells a session to fetch in the background with a real UA; the code does exactly that (it is the owner). The note says "30s+", the code says "tens of seconds" and sets 120 s receive; "warm ~650ms" appears only in the note.
Kinds: K1 (UA and cold-start rule in note and code); the 650 ms figure has one home. No G, ADR or docs/agents home found (searched G for dmleaderboards/Cloudflare/User-Agent: no files). Test holding equal: none found.

## I8-057 "In testdata, 4 of 50 lanes differ between the Onyx and CH pad rules"
One home: M\drum-lanes-d112-2026-10.md:17. Searched H docs/agents, docs/clone-hero, docs/adr, tools, tests names for lane/Onyx (no count of lanes in them) and G, M. The pad rule owner is `light_drum_lanes` H\src\parse\song.cpp:205. Side note: H\docs\clone-hero\02-note-list.md:611,631,641 and questions.md:270 point at "src/parse/song.cpp line 203" and "called at line 2009"; the code today has light_drum_lanes at line 205 and the call at line 2204 (song.cpp:2198-2204). A line-number copy that has drifted (K2). Ruling: H\docs\agents\rulings\ui-and-display.md:117 (look at its section: this is in the file; not confirmed by me which section).

## I8-080 "Four executors once started on a base 361 commits old"
One home for the figure: M\git-traps-in-shared-checkout.md:13. The rule behind it (cut an executor worktree from main by hand, executor checks its base commit first) is stated in M\git-traps-in-shared-checkout.md:16; G\model-postures.md:5 says "each in its own worktree"; :7 "Cut its branch from the blocker's executor branch". No hook or doc in G or H\docs\agents names the base of an executor worktree. Only reviewer isolation is a hook: G\review_dispatch.py:24-26,149-150 (tests G\tests\test_review_dispatch.py:79-92), H\docs\agents\derive-once-review.md:200. So "executor worktree base" has the note only (K7 candidate); I searched G and H\docs\agents for isolation, "cut from", "worktree add", "base commit".

## I8-126 "six review agents, ~781k tokens and two refused merges"
Homes: M\one-reviewer-rule-d107.md:13; H\docs\audit\2026-10-03-fix-decisions.md:599 "about 781k subagent tokens and two refused merges" (same figures, cites a removed handoff). Agree. Kinds: K1.
Related restatements of the rule itself (not the figures), as side info: G\model-postures.md:5,7,9; G\agents\reviewer.md:3; G\README.md:138; CLAUDE.md rule 5; brief-preamble.md:73,89.

## I8-148 "its test used 0.003, which the probe rule gives to KEY_HOLD_S" (key hold time)
Owner: H\tools\ch_probe\input_driver.py:41 `KEY_HOLD_S = 0.003`; users :195,:216; H\tools\ch_probe\interfaces.py:233 docstring points at it; H\tools\ch_mcp\tests\test_game_tools.py:750 uses the name.
Second homes: H\tests\single_owner\rules\probe.json:506-535 (question "How long is a key held?", pattern `\b0\.003\b`, owner_lines whitelist the owner line and the test pin); H\tools\ch_probe\tests\test_input_driver.py:117-118 `assertEqual(input_driver.KEY_HOLD_S, 0.003)` with comment "D54 records the 3 ms hold as it is" (a pinned value from a recorded decision).
Tests holding homes equal: the single-owner scan rule probe.json (forbids a loose 0.003 in tools); the pin at test_input_driver.py:118.
Same number, different questions (not twins): H\docs\clone-hero\03-timing.md:123,125 (the game's 0.003 s clock-correction gap), H\docs\clone-hero\05-star-power.md:467 (no 3 ms constant in the drum engine), H\tools\ch_probe\experiments\stall_harness.py:89 ("about 3 ms at 330 frames a second").
Kinds: K1 (pin in test plus scan whitelist); the note's wording ("its test used 0.003") is a pointer; the scan rule is the hook-like check.

## I8-154 "1,300 wheel notches"
One home for the figure: M\search-before-clicking-a-song.md:13 (with the user's quoted words there). The rule "never scroll the library unasked" homes: M\search-before-clicking-a-song.md:11 and :3; M\MEMORY.md:10; H\docs\agents\ui-testing.md:99 (guards: `item_ref`, `library_scroll_refusal`, `library_key_refusal`, `kPageKeys` in H\tests\ui\uitest_commands.cpp:239; says "No guard here can stop a shell script that sends wheel or key messages ... no hook reads shell text, so the rule ... is the only thing that binds there"). G: no hook reads wheel messages (searched PostMessage, WM_MOUSEWHEEL, mouse wheel, SendInput in G .py/.md/lib/tests: none).
Split (wording): M\render-checks-use-the-corpus.md:19 tells the reader "Scroll to the middle of the list, click a song, compare before and after" for open/close changes; M\search-before-clicking-a-song.md:11 says never scroll without asking in chat. One tells the step, the other forbids it.
Rulings quoted in the user's words (H\docs\agents\rulings\ui-and-display.md, `## Confirmed` runs 7-101): line 27 (2026-10-30) "ive had enough of this shit. something like 10 times now you waste my time by scrolling thorugh the entire library for minutes ... this should never happenw tihout my approval." and line 48 (2026-10-24) "why are you scrolling through the entire library??? why do you keep forgetting search exists?".
Kinds: K5/K7 (a rule in a memory note, guarded in code for one channel only, plus a stated gap), K3 candidate (render-checks scroll step).

## I8-163 "song_ms = video_ms + 19995.46 (constant, zero drift)"
One home for the value: M\sp-squeeze-mechanics.md:18-20. Searched H docs/agents, docs/clone-hero, docs/audit, tools, G, the skills, the squeeze-tools repo (README.md, squeeze_scout.py: no 19995). Pointer, no value: C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:34 "FC videos trim/pad intros; align first (see memory sp-squeeze-mechanics ...)". Note: the note says "may TRIM", the skill says "trim/pad". An archived note (M\archive\gameplay-video-score-analysis.md:21) also names video_ms; archive was out of scope.
Same row family, found while here, a split on the squeeze window's location (I8-161's neighbour): kSqueezeWindowMs is defined at H\src\core\model.h:121 (and `within_squeeze_window` :133, whose comment at :130-132 claims "The one window check: the search graph, the engine's tail rows, the stored rows ... and hydra_replay all ask it"). But:
- C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:10 says "`kSqueezeWindowMs`, src/search/graph.h".
- C:\Users\Patrick\Downloads\Hydra\squeeze-tools\squeeze_scout.py:34 `HYDRA_WINDOW_MS = 500.0     # kSqueezeWindowMs, src/search/graph.h` and line 4-5 same file. And squeeze_scout.py:33 `HIT_WINDOW_MS = 85.0        # kDefaultHitWindowMs, src/core/model.h`.
- The memory note M\sp-squeeze-mechanics.md:14 says src/core/model.h (correct).
So two homes say graph.h, the code owner is model.h (K2/K8 drift of an owner pointer), and squeeze_scout.py holds its own 500.0 and 85.0 (K1/K6 copies outside Hydra, in a tool). Also in H\src\core\replay.cpp:26-33 the phrase list goes through `core::squeeze_window_phrases`; I did not check that function calls `within_squeeze_window` (grep of model.cpp for squeeze_window_phrases found nothing, so it lives elsewhere; unchecked).

## I8-195 "A SqOut gap is measured against the pre-collection end, not the final deactivation node" (Enigma Machine 4750 ms wrong vs 2083 ms right)
Homes: C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:23 (rule and example) and :31 ("Use the pre-collection end for SqOut (tool does this)"). No other home with Enigma/4750/2083 in H docs/agents, docs/clone-hero, tools, src headers, G, M (searched). Code owners of the question: H\src\core\backend_value.h:14-29 (`sqout_position`, `paid_by_sp_walk`), H\src\core\model.h:276-295 (squeeze_difficulty, "Every row is measured from the one SP end (D269 call 4)"). The skill's phrase "the pre-collection end" versus model.h's "the one SP end": I did not find a source that defines "pre-collection end" in the code; the skill's tool squeeze_scout.py does the work ("tool does this"), so the rule has a second owner in that tool (K6 candidate, unchecked beyond that). The game's rule is on H\docs\clone-hero\05-star-power.md:403 (SP-37), which I did not read.

## I8-196 "A phrase whose completion note lands inside SP is collected -> SP extends +2 measures"
Homes found: the skill SKILL.md:22 only. Related value in H: kMeasuresPerSpBar (2 measures) src/core/timing.h:254 named at H\docs\clone-hero\05-star-power.md:109,129, :363 "pushes the end one quarter bar later (two measures in Hydra's units)". M\sp-squeeze-mechanics.md does not state it. Not searched further.

## I7-131 "CHECK(acts > 1000); // a floor the user approved (D43)"
Homes: H\docs\agents\test-rubric-examples.md:123; H\docs\agents\test-rubric.md:13 and :88 ("acts > 1000"); H\tests\test_sp_end.cpp:793 and :830 (the code); H\docs\audit\2026-10-03-fix-decisions.md:103 "D43 ... Approved as test limits, none an app setting or visible: `acts > 1000` and `skipped > 100` (test_search.cpp)". Split: the decision log says the check is in test_search.cpp; the code today has it in test_sp_end.cpp:793,830 (K2 file name). Also D43 appears in H\tests\test_replay_windows.cpp:392, test_fills_and_skips.cpp:305, test_fast_tempo.cpp:353 (other limits). The memory note M\labels-are-not-reasons.md:11 names "Approved under D43" as an example of a label that is not a check. Decision-log lines are the user's recorded decisions; I did not open lines 111-114.

## I7-133 bank rise {13440, 14400}, "src/search/engine.cpp line 2015"
Homes: H\docs\agents\test-rubric-examples.md:83-85 (the check) and :88 ("line 2015"); H\docs\agents\test-rubric.md:25,185; H\tests\test_sp_meter_curve.cpp:279,306,416-466 (the real test, 13440 and 14400 many times). The numbers are test data. The pointer is stale: H\src\search\engine.cpp:2015 is `const ScoreGraphNode* node = en.nodes[(size_t)oa.act_node];` inside the activation output loop, not a bank rise. No 13440 or 14400 in H\src. Also H\docs\clone-hero\05-star-power.md:117 states 14400 (the CH bar in ticks, a different question). K2 (a stale line pointer; I did not search whether 2015 was once a bank-rise line).

## I7-155 offline stall harness ("tools/ch_probe/experiments/stall_harness.py"; cdb attach at most once per run)
Homes: H\docs\agents\this-machine.md:67 (the row); H\tools\ch_mcp\kit_tools.py:23 names stall_harness; H\tools\ch_probe\experiments\stall_harness.py:89 (a 3 ms comment). H\docs\agents\this-machine.md:55-63 and H\docs\agents\rulings\review-and-hooks.md:32-33 speak to cdb (the latter is a ruling file; not read for section). One-line search only.

## I7-189 "The rules come from two web searches on 2026-10-10" (public-doc-style.md)
One home for that claim: H\docs\agents\public-doc-style.md:7. H\docs\clone-hero\06-scoring.md:127 has an unrelated "two web searches" (2026-11-11, about setlist combos). Same words, different facts, no split. M\search-web-before-finalizing-plans.md is a related rule, not a copy.

## I7-224 doc picture size limits
Homes: H\tools\doc_shots\shrink.py:18-21 `MAX_FILE_BYTES = 500 * 1024`, `MAX_FOLDER_BYTES = 4 * 1024 * 1024` ("The user's limits (D300 ...). tests/test_docs_match_code.cpp checks the same two at merge."); H\tests\test_docs_match_code.cpp:2020-2025 `kDocImageMaxBytes = 500 * 1024`, `kDocImagesMaxTotalBytes = 4 * 1024 * 1024` ("tools/doc_shots/shrink.py fails a capture run on the same limits"); H\docs\agents\doc-screenshots.md:17 (points at shrink.py constants, states no number, a pointer); decision log D300 in H\docs\audit\2026-10-03-fix-decisions.md. Agree today. Kinds: K6 (two code places hold the same two limits, each comment names the other); K1. Test holding them equal: none found (neither test reads shrink.py's constants; I did not read tools/test_* for one).

## I7-226 doc-screenshots "When to run" (before each release; after a label or layout change)
One home: H\docs\agents\doc-screenshots.md:23. tools\doc_shots.ps1:1-11 only points at the page. No hook or test checks it. K7 candidate (a prose rule nothing checks, one home).

## Side notes
- H\docs\clone-hero\02-note-list.md:611,631,641 and questions.md:270 quote `src/parse/song.cpp` line numbers (203, 2009) that no longer match the code (205, 2204).
- squeeze_scout.py (C:\Users\Patrick\Downloads\Hydra\squeeze-tools) is a tool outside both scope repos that holds 85.0 and 500.0 itself.
- H\docs\agents\ui-testing.md:99 says no hook reads shell text; this is a hook gap statement (a way past the scroll guard exists for messages posted from a shell, in general words).
