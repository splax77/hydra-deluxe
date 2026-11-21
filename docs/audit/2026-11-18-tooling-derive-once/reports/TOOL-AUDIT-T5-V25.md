# TOOL-AUDIT-T5-V25 (verifier of F25). All claims "by reading"; no run made. H = C:\Users\Patrick\Downloads\Hydra\hydra-test.

I checked the finder's candidates that carry a verdict-relevant claim: S1 to S5, the K6 and K7 items, the K8 owner-map claim, and the zero-lead and version side notes. Group-level K1 copies (G3 v+version, G7 crop rule, G12 kinds) I did not re-open one by one; they are not reached.

## S1 ruling source breadth: CONFIRMED (by reading)
- H docs/agents/premise-map.md:45: a `ruling` quote "must sit in a line under `## Confirmed` ..., in a session card's quoted words under `docs/sessions/`, or in the user's message as the main session quotes it in the reviewer's brief and confirms."
- H CLAUDE.md: "Only a line under `## Confirmed` may be cited as the user's ruling."
- H docs/agents/agent-rules.md:35: "A line under `## Confirmed` is the user's words." Line 41 lets a decision entry count when it quotes words "found under `## Confirmed` or in a session card." So agent-rules.md is itself on both sides, as the finder said. The CLAUDE.md "Past sessions" text also says a card quote "is not a filed ruling".
- Situation where they differ: a plan's `ruling` premise row that rests only on a card quote or a brief quote.
- Ruling check: I grepped the Confirmed lines of docs/agents/rulings/*.md for premise, session card, live step and heading words. Found no line that names either side. The grep was by keyword only.

## S2 live-step heading: CONFIRMED (by reading)
- Page H docs/agents/premise-map.md:56: "a heading that starts `### LS` followed by a `-` or a digit ... inside a section whose heading starts `## The live steps`. A struck heading `### ~~LS` is not a step."
- C agents/planner.md:13 says "a heading that starts `### LS`" with no dash/digit and no section condition; reviewer.md:13 not re-read in full (output trimmed), the finder's quote matches the planner line I saw.
- Test H tests/test_docs_match_code.cpp:1625-1631: is_live_step_id needs "LS" then a longer text; heading level 3.
- Differing situation: a heading like `### LSX` or one outside the live-steps section.

## S3 table list: CONFIRMED-CHANGED (by reading)
- H docs/agents/this-machine.md:85: "The tables today are `charts`, `meta`, `profiles`, `records` and `record_paths` (read from a copy of the development build's file)." It says the record tables come from `record_tables_sql()`.
- H tools/compare_db.py:130-136: TABLES = results, paths, path_refs, songmeta, dynamics, charts, meta, profiles, records, record_paths; SUMMARY_TABLES lists 6.
- Change: grep of H src finds CREATE TABLE only for profiles, records, record_paths (record_store.cpp:272,281,290) and charts, meta (853,866). So the five extra names in compare_db.py have no schema owner in src today; they look like an older or other database layout, not a live disagreement about the same file. The page says "today"; compare_db.py gives no date. Still two lists with different answers to "which tables does the database have". I did not open record_tables_sql() to see whether it creates more tables than the three; open question, by reading only.

## S4 ch_run_chart default player: CONFIRMED (by reading)
- H docs/agents/clone-hero-mcp.md:52 writes `kit_player="System"`.
- H tools/ch_mcp/server.py:414: `kit_player: str | None = None`. H tools/ch_mcp/kit_tools.py: `ch_kit_setup(player=None)` runs `self._setup(KIT_PLAYER if player is None else player)` and KIT_PLAYER = "Player1" (line 92). I read this path for ch_kit_setup; the finder said ch_run_chart reaches the same None-to-KIT_PLAYER (kit_tools.py:1050-1063); I did not trace the call from server.py into it. Open: a run of ch_run_chart with no kit_player would settle it.
- Differing situation: a caller who copies the doc signature and omits kit_player.

## S5 meaning of "kind A": CONFIRMED-CHANGED (by reading)
- H docs/agents/derive-once-review.md:30: "**A.** A rule written twice in production code."
- H docs/agents/test-rubric.md:39: a game rule only display code uses is reported "as a derive-once finding (kind A ...) against the production code"; line 148 also says "(derive-once, kind A)".
- Change: test-rubric.md calls it "a sign the display invented it" and the engine already has the answer, so it is a rule written twice in production (engine and display). That is arguably the same as kind A, not a second meaning. The finder's "even when nothing else answers" is wrong: test-rubric.md:39 says "the engine already has its answer". Verdict: the homes agree on the situation; I do not see a split. Marked not a split; K1 wording only. Ruling check: none found.

## K6 realapp.jsonl: CONFIRMED (by reading)
- H tools/hydra_ui_mcp.py:81 `REALAPP_FILE = "realapp.jsonl"` (comment at 80 "field names are a contract"); G review_rules.py:49 `REALAPP_FILE = "realapp.jsonl"`. Two literals, no test of mine seen that holds them equal (not searched in G tests beyond the finder's claim). No split in value today.

## K6 picture size limits: CONFIRMED (by reading)
- H tools/doc_shots/shrink.py:20-21 MAX_FILE_BYTES = 500*1024, MAX_FOLDER_BYTES = 4*1024*1024. H tests/test_docs_match_code.cpp:2024-2025 kDocImageMaxBytes = 500*1024, kDocImagesMaxTotalBytes = 4*1024*1024. Same values; two code homes.

## K6 Kind words: CONFIRMED (by reading)
- Test H tests/test_docs_match_code.cpp:1530 kKindWords = code, data, chart, run, ruling, unchecked; the page's Kind list was not re-read by me, so "a second list" rests on the finder's word for the page side. Live-step heading: see S2.

## K7 start dates: CONFIRMED (by reading)
- H tests/test_docs_match_code.cpp:1520 kPremiseMapSince = "2026-11-01", 1525 kDeferredPremiseSince = "2026-11-13". grep of H docs/agents/premise-map.md for both dates finds nothing.

## K8 "no test checks `## Owner map`": CONFIRMED (by reading)
- grep -i "owner map" over H tests/ and H tools/ and G *.py finds no match (grep for `Owner map` in tests and tools also none). I did not open row I7-174 to see its exact wording. The finder searched the same phrase only; a test could check it under another word (for example "owner-map").

## Side note: a lead of exactly 0 reads "early": CONFIRMED (by reading)
- H tools/ch_probe/experiments/edge_probe.py:172-173 and 198: `'early' if p.lead_ms >= 0 else 'late'`. H tools/ch_mcp/server.py:450-451: "a positive lead presses early, a negative one late"; says nothing of zero.

## Side note: "VERSION 3.1.2 today": CONFIRMED (by reading)
- H docs/agents/this-machine.md:17 reads "which reads VERSION 3.1.2 today". I did not re-open CMakeLists.txt:21.

## Splits (every split my tables show)
1. S1 (CONFIRMED): what may a plan cite as the user's ruling. premise-map.md:45 and agent-rules.md:41 admit card and brief quotes; CLAUDE.md and agent-rules.md:35 admit only `## Confirmed`. No ruling names both sides.
2. S2 (CONFIRMED): what a live-step heading is. Page and test vs planner.md/reviewer.md.
3. S3 (CONFIRMED-CHANGED): which tables the database has. this-machine.md 5 vs compare_db.py 10; the extra 5 have no CREATE TABLE in src.
4. S4 (CONFIRMED): ch_run_chart default kit_player. Doc "System" vs code None, which becomes "Player1" by the setup path.
5. Added by me: the same S1 sits inside agent-rules.md itself (line 35 vs line 41), so a fix to one doc alone leaves the split open. Not a new split, a second place for S1.
S5 removed as a split.

## Not reached
G3 build-number copy (I6-194), G6 label/wayfinder items, G7 crop-rule and reset-text copies, G8 commands>=1 gate versus page, G9 "Edge Walk Pair" path, G10 plan "1 to 11" vs rubric 12 questions, G12 skip-dir difference (kNotPermanentDirs), and the finder's six lightly searched rows. The finder's own unopened files (ADR 0028, test_live_protocol.cpp, /wayfinder, /triage) I also did not open.
