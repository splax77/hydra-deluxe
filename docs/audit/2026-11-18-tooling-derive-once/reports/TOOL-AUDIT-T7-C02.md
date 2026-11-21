# TOOL-AUDIT-T7-C02 closure check (item 1: L-F02-17 to L-F03-22, 18 ids)

H = C:\Users\Patrick\Downloads\Hydra\hydra-test (main checkout). M = the memory folder under C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory. G = C:\Users\Patrick\.claude\gates. All read as they are now.

Counts: PASS 18, REOPEN 0, GONE 0. Not reached: none.

### L-F02-17
**Verdict:** PASS (P)
**Homes read:** H src/ui/app_state.h:434 `kViewProgressDelaySeconds = 0.15`; :445 `kSearchThrottleSeconds = 0.15` (different question, same number); src/ui/app_state.h:296-299 and src/ui/details_panel.cpp:197-199 (comments name the constant, no number); H docs/agents/ui-testing.md:124 "once it has run 0.15 s"; M approved-work-is-not-reasked.md:11 "under the 0.15 s screen delay".
**Why:** Every home says 0.15. A pointer in place of the doc and note copies changes no behaviour and needs no choice.

### L-F02-18
**Verdict:** PASS (P)
**Homes read:** H docs/agents/clone-hero-mcp.md:48 "gaps over 50 ms" and "`+0x1B8`"; H tools/ch_mcp/sp_tools.py:62 `GAP_MS = 50`; H tools/ch_probe/constants.py:128 `OFF_SP_HALF = 0x1B8`.
**Why:** The page copies match the two constants exactly. No disagreement.

### L-F02-19
**Verdict:** PASS (P)
**Homes read:** H clone-hero-mcp.md:55 and :68 (they name `TRACE_CAP_MARGIN_S`, `REPLAY_CAP_S`, `ENGINE_SEARCH_CAP_S`; no numbers); H tools/ch_mcp/trace_hits.py:70 `= 30.0`, :74 `WALL_CAP_PAUSE_S = 120.0`; tools/ch_probe/prep_inputs.py:50 `REPLAY_CAP_S = 120.0`; tools/ch_probe/engine_finder.py:38 `= 120.0`; tools/ch_mcp/bot_run.py:88-90 (20.0, 5.0, 30.0); tools/ch_mcp/tests/test_trace_hits.py:180-181, 462-463 (6,000 ms + 30 s = 36 s; + 120 s = 156 s).
**Why:** The 120.0 values answer different questions and each sits once in code. The page names constants only. The test comments' arithmetic (36 s, 156 s) agrees with the constants. Nothing disagrees.

### L-F03-01
**Verdict:** PASS (B)
**Homes read:** M early-fill-orange-85-d250.md:3 and :11 (says white slack is a fixed `kEarlyFillSlackMs` (60 ms) read by `is_e0`); H src/core/model.h:101-106 (`kHitWindowMs = 85`, "also the widest early-fill window the setting allows"), :108-112 (`kDefaultEarlyFillWindowMs = 60`, "Every default of the setting ... is this one constant"), :115, :342 `is_e0(e_offset, skips, window_ms)`.
**Why:** The one split is the memory note against model.h. `is_e0` now takes `window_ms`, so no fixed slack constant exists. `grep kEarlyFillSlackMs` in H src, tests and tools finds nothing (it appears only in old plans, handoffs and audit docs).

(L-F03-01 also: M MEMORY.md:22 "reach kHitWindowMs 85 / slack 60" is a second memory-note line of the same claim; still a memory note, so B holds.)

### L-F03-03
**Verdict:** PASS (P)
**Homes read:** H src/core/rules.h:32 `backend_leeway_ms = 3.0`, :38 `max_tied_paths = 4`; M sp-end-is-one-point-leeway-is-delgars.md (quotes "3ms"), MEMORY.md:7 "3 ms"; M engine-tie-merge-prune.md:14 "default 4"; M every-brief-carries-status-rule.md:15 "every 5 minutes".
**Why:** Notes and code say the same numbers. I did not find a note with a different value.

### L-F03-05
**Verdict:** PASS (B)
**Homes read:** M measure-small-explain-first.md:11 (the quoted hook message); G/*.py, *.md, *.json grep for "differently", "since the last run", "what changed"; whole of C:\Users\Patrick\.claude grep for "end differently" (excluding transcripts).
**Why:** No hook carries the text. The only hits are G tests/test_agent_stop.py:362 ("fixed differently") and an unrelated plugin comment. So the split is a memory note against the hooks' code, and the note is the wrong side. (Whether a hook once carried it is unknown, but that does not change the note being wrong today.)

### L-F03-06
**Verdict:** PASS (B)
**Homes read:** M every-brief-carries-status-rule.md:3 ("the dispatch gate now denies briefs that lack it") and :15 ("No gate checks a brief for this sentence today"); G review_dispatch.py:1-6 (it records a review handover; nothing about status); G progress.py, prompt.py, percall.py, common.py, review_dispatch.py, review_rules.py searched for a brief check on the status sentence (none); G tests/test_docs_numbers.py:31 lists `reviewer_dispatch_gate` as a retired hook word.
**Why:** The split is the note against itself and against the gates' code. The code agrees with line 15. Note on shape: both sides are memory-note text, with code deciding, so it is in B's spirit. I am passing it, with this flagged for the main session in case it wants a strict reading of "memory note against docs or code".

### L-F03-07
**Verdict:** PASS (P)
**Homes read:** H tools/build_slot.ps1:75 `$BuildSlotCount = 3`; H tools/test_build_pass.ps1:115, 117, 119, 141, 199, 207 (literal 3; does not read the owner); H CLAUDE.md rule 14 "At most three builds run at once"; H docs/agents/agent-rules.md:20.
**Why:** All say three. One caution for the fixer: the literal 3 in test_build_pass.ps1 is a test's own stated expectation, so it should stay a literal rather than be "replaced by a pointer" (the test rubric forbids a test computing its expected value from the owner). That is not a disagreement and needs no choice.

### L-F03-09
**Verdict:** PASS (P)
**Homes read:** H docs/agents/test-rubric.md:15, :25, :48-49, :130, :175, :185; H docs/agents/test-rubric-examples.md:44, :186, :340.
**Why:** 243 / 128520 / 128400 and 13440 agree wherever they appear. The Blight and fill-reach figures the row listed in test-rubric-examples.md are no longer in the file (grep for 228, 64800, 65280, 7285, 7200, 14400 finds nothing), so there is nothing left to split with. I did not check the numbers against the engine's record; P only needs the homes to agree.

### L-F03-10
**Verdict:** PASS (C)
**Homes read:** H docs/agents/test-rubric-examples.md:88 ("`bank_rise_ticks` (`src/search/engine.cpp` line 2015)"); H src/search/engine.cpp:2015 (`const ScoreGraphNode* node = en.nodes[...]`, not the stamp) and :2022 (`act.bank_rise_ticks.assign(...)`).
**Why:** A moved line. The current place is src/search/engine.cpp:2022 and can be found by the name `bank_rise_ticks`. Same page :147 cites "line 2099" for `frontend_transfer_scales`; not in this row.

### L-F03-11
**Verdict:** PASS (C)
**Homes read:** H docs/audit/2026-10-03-fix-decisions.md:103 ("`acts > 1000` and `skipped > 100` (test_search.cpp)"); H tests/test_sp_end.cpp:793, :830 (`acts > 1000`); H tests/test_fills_and_skips.cpp:305 (`skipped > 100`); `ls tests` shows no test_search.cpp (only test_search_target.cpp).
**Why:** A moved or missing file name. The current files are clear: test_sp_end.cpp for `acts > 1000`, test_fills_and_skips.cpp for `skipped > 100`. test-rubric-examples.md:123 and test-rubric.md:13 carry the same D43 comment text as the code does.

### L-F03-13
**Verdict:** PASS (B)
**Homes read:** M approved-work-is-not-reasked.md:13 "the threaded chart list, about 8 ms"; H docs/agents/rulings/code-and-engine.md:63 "measured at 5.4 ms against 13.8 on 1" (under `## Unconfirmed`, which starts at :61; `## Confirmed` is at :7).
**Why:** The one split is the memory note against a doc line. The doc line is not a ruling (it is under Unconfirmed), so no ruling sits on either side. Caution: the 5.4 ms figure has no code or measurement file behind it that I could find (searched docs, src, tests, tools for "5.4 ms" / "13.8 ms"), so "note is wrong" rests on a doc line only.

### L-F03-15
**Verdict:** PASS (P)
**Homes read:** M dmleaderboards-comparison.md:19 "warm ~650ms, cold 30s+"; H src/net/http.cpp:172-185 (comment "tens of seconds"; `WinHttpSetTimeouts(..., 15000, 20000, 30000, 120000)`); H src/net/dmbot_client.h:10 "tens of seconds".
**Why:** "30s+" and "tens of seconds" agree, and the 120 s receive timeout covers both. "650 ms" has one home. No contradiction.

### L-F03-16
**Verdict:** PASS (C)
**Homes read:** H docs/clone-hero/02-note-list.md:611, :631, :641 ("line 203", "lines 203 to 213", "called at line 2009"); H docs/clone-hero/questions.md:270 ("line 203"); H src/parse/song.cpp:205 `light_drum_lanes`, :2204 its call, :90 `lights_lane_pad`, :149 `game_lane_pads`, :175 `every_lane_pad`, :215 the Note Shuffle branch.
**Why:** Pure line drift (definition at 205 not 203; call at 2204 not 2009). The current place of each named function is clear by name. The same doc lines also carry other drifted numbers (88, 147, 159 to 165, 200 to 201, 213), all moved by about 2 lines and all findable by name, so the fix should cover them too.

### L-F03-18
**Verdict:** PASS (P)
**Homes read:** M one-reviewer-rule-d107.md:13 "~781k tokens and two refused merges"; H docs/audit/2026-10-03-fix-decisions.md:599 "about 781k subagent tokens and two refused merges".
**Why:** Same figures in both.

### L-F03-19
**Verdict:** PASS (P)
**Homes read:** H tools/ch_probe/input_driver.py:41 `KEY_HOLD_S = 0.003` (used :195, :216); H tools/ch_probe/tests/test_input_driver.py:112, :118 (literal pin 0.003); H tests/single_owner/rules/probe.json:506-535 (owner KEY_HOLD_S).
**Why:** One owner, plus a scan rule and a test pin that hold it. No split. The test's literal is its stated expectation and should stay.

### L-F03-21
**Verdict:** PASS (P)
**Homes read:** ~/.claude/skills/analyzing-sp-squeezes/SKILL.md:22-23, :31 ("pre-collection end"); H src/core/model.h:276-295 ("Every row is measured from the one SP end (D269 call 4)"); H src/core/backend_value.h:14-29; H CONTEXT.md:571-600 (SqIn / SqOut: the window offers "its newest phrase, when collecting it is what moved the window's end away from this SP end"; :720-728 "Every place an activation's SP end moved"); H docs/clone-hero/05-star-power.md:361 (SP-34, phrase collected during SP moves the end later); M sp-end-is-one-point-leeway-is-delgars.md.
**Why:** The skill's "pre-collection end" is the SP end the window belongs to, before the collected phrase moves the end, which is the "this SP end" the code and CONTEXT.md measure SqOut from. "Pre-collection" appears in no code (searched src, docs/clone-hero, docs/agents, CONTEXT.md), so it is only a different phrase for the same point. I read no disagreement. This one rests on my reading of CONTEXT.md and model.h; if the main session wants a firmer check, it is the one in this batch with the most judgment in it.

### L-F03-22
**Verdict:** PASS (P)
**Homes read:** H docs/agents/doc-screenshots.md:21-23 ("Run it before each release ... Also run it after any change to a label or a layout that one of the pictures shows"); H docs/development.md:976-982 ("Run it before each release", points to doc-screenshots.md); H tools/doc_shots.ps1 (only points at the page); grep of docs, tools and G for the two phrases.
**Why:** The row said one home; there are two. development.md:980 repeats the first half ("before each release") and points to the owner page. They agree, so P holds. A pointer fix is harmless.

## Ids not reached
None.
