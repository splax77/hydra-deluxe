# TOOL-AUDIT-T5-L09 ledger (finders F25, F26, F27; re-send F25b)

Compiled from the finder and verifier reports only. H = C:\Users\Patrick\Downloads\Hydra\hydra-test, G = C:\Users\Patrick\.claude\gates, M = memory folder, DOCS = H\docs\agents. All verifier verdicts are "by reading"; nothing was run. V25b verified nothing (the F25b file did not exist when it ran), so every F25b row is NOT VERIFIED. Where verifier and finder disagree, the verifier's reading is used and said so. No ruling quotes were found by any report that name both sides of any split. I add no opinion on exits.

# Part 1: F25 rows

### L-F25-01: What may a plan cite as "the user's ruling"?
- **Source:** I6-175, I7-167 (G1); F25 S1; V25 S1. Same question as L-F26-03.
- **Kind:** K1, K3
- **Verdict:** CONFIRMED
- **Homes:** DOCS\premise-map.md:45 a `ruling` quote "must sit in a line under `## Confirmed` ..., in a session card's quoted words under `docs/sessions/`, or in the user's message as the main session quotes it in the reviewer's brief and confirms." H CLAUDE.md "Only a line under `## Confirmed` may be cited as the user's ruling." DOCS\agent-rules.md:35 "A line under `## Confirmed` is the user's words."; agent-rules.md:41 lets a decision entry count when it quotes words "found under `## Confirmed` or in a session card". CLAUDE.md "Past sessions": a card quote "is not a filed ruling".
- **Splits:** (1) premise-map.md:45 and agent-rules.md:41 admit card and brief quotes; CLAUDE.md and agent-rules.md:35 admit only `## Confirmed`. They differ when a plan's `ruling` premise row rests only on a card quote or a brief quote. (2) The verifier adds that agent-rules.md is on both sides inside itself (line 35 vs 41), so a fix to one doc leaves the split open.
- **Ruling quotes:** None found. Verifier grepped Confirmed lines for premise, session card, live step, heading words (keyword only). Finder's grep was partial.
- **Tests:** none named. G\tests\test_agent_files.py pins phrases only.
- **Open:** keyword-only ruling search.
- **Hook gap:** no

### L-F25-02: What is a "live step" heading?
- **Source:** I6-176..178, I7-163..166, I7-168 (G2); F25 S2; V25 S2.
- **Kind:** K1, K6
- **Verdict:** CONFIRMED
- **Homes:** DOCS\premise-map.md:56 "a heading that starts `### LS` followed by a `-` or a digit ... inside a section whose heading starts `## The live steps`. A struck heading `### ~~LS` is not a step." H tests\test_docs_match_code.cpp:1625-1631 `is_live_step_id` ("LS" then longer text, heading level 3). C agents\planner.md:13 "a heading that starts `### LS`" (no dash/digit, no section condition); reviewer.md:13 same (finder's quote; verifier saw only the planner line).
- **Splits:** page and test vs planner.md/reviewer.md. They differ for a heading such as `### LSX`, or one outside the live-steps section.
- **Ruling quotes:** None found (same keyword grep as L-F25-01).
- **Tests:** test_docs_match_code.cpp holds plans to the page's shape (one direction). Nothing holds the agent files equal to the page.
- **Open:** reviewer.md:13 not re-read by verifier.
- **Hook gap:** no

### L-F25-03: Where do the Kind words and the plan start dates live?
- **Source:** I6-176..178 (G2); F25; V25 (K6, K7 items).
- **Kind:** K6, K7
- **Verdict:** CONFIRMED (test side). The page's own Kind list was not re-read by the verifier, so "a second list" rests on the finder.
- **Homes:** H tests\test_docs_match_code.cpp:1530 kKindWords = code, data, chart, run, ruling, unchecked; :1520 kPremiseMapSince "2026-11-01"; :1525 kDeferredPremiseSince "2026-11-13". Verifier: grep of premise-map.md for both dates finds nothing. Page Kind list at premise-map.md:41-46 (finder only).
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** none holds the page's Kind list equal to kKindWords.
- **Open:** page Kind list not re-read.
- **Hook gap:** no

### L-F25-04: What counts as "not a source" (two lists)?
- **Source:** I6-175, I7-167 (G1); F25 only.
- **Kind:** K1
- **Verdict:** NOT VERIFIED (verifier did not reach it)
- **Homes (finder only):** premise-map.md:52 "A file that only reports on it does not count. That means a handoff, another plan, a session card, a memory note, or a Clone Hero block with no proof label."; premise-map.md:42; agent-rules.md:41 and rule 15 "A rule copied from a tool, test or comment is never a source."; agent-rules.md:39; C agents\planner.md:13 and reviewer.md:13.
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** not reached.
- **Hook gap:** no

### L-F25-05: Release tag/title naming and the version number copy
- **Source:** I6-188 (G3); F25; V25 side note.
- **Kind:** K1, K2-risk
- **Verdict:** CONFIRMED for the copy text only ("VERSION 3.1.2 today" is in this-machine.md:17; CMakeLists.txt:21 not re-opened by verifier). The "v plus version" rule being in prose only was not reached.
- **Homes:** DOCS\this-machine.md:17 "A release's tag and title are both `v` plus the version ... which reads VERSION 3.1.2 today." H docs\development.md:1011-1017; H src\app\update_release.cpp:17-18 (suffixes only; checks the tag parses); G publish.ps1 takes -Target/-Title as given (finder only). H CMakeLists.txt:21 VERSION 3.1.2 (finder only).
- **Splits:** None (agree today).
- **Ruling quotes:** None found.
- **Tests:** none found.
- **Open:** nothing checks tag == `v`+version.
- **Hook gap:** no

### L-F25-06: Which tables does the database have?
- **Source:** I6-208, I6-209, I7-159 (G4); F25 S3; V25 S3.
- **Kind:** K1, K6, K8
- **Verdict:** CONFIRMED-CHANGED. Verifier: the five extra names in compare_db.py have no CREATE TABLE in src today, so they look like another or older layout, not a live disagreement about one file.
- **Homes:** DOCS\this-machine.md:85 "The tables today are `charts`, `meta`, `profiles`, `records` and `record_paths`". H tools\compare_db.py:130-136 TABLES = results, paths, path_refs, songmeta, dynamics, charts, meta, profiles, records, record_paths. H src\store\record_store.cpp:272,281,290 (profiles, records, record_paths) and 853,866 (charts, meta). H tools\test_compare_db.py:141 typed CREATE TABLE copy. F27 reached the same (I10b-115): no src owner found for results, paths, path_refs.
- **Splits:** this-machine.md lists 5 tables, compare_db.py lists 10. They differ for results, paths, path_refs, songmeta, dynamics.
- **Ruling quotes:** None found.
- **Tests:** none holds the lists equal.
- **Open:** verifier did not open `record_tables_sql()`.
- **Hook gap:** no

### L-F25-07: What is `ch_run_chart`'s default player?
- **Source:** I7-091..093 (G9); F25 S4; V25 S4.
- **Kind:** K2
- **Verdict:** CONFIRMED
- **Homes:** DOCS\clone-hero-mcp.md:52 `ch_run_chart(song_folder, score_json, schedule_json, kit_player="System")`. H tools\ch_mcp\server.py:414 `kit_player: str | None = None`. H tools\ch_mcp\kit_tools.py:92 KIT_PLAYER = "Player1"; `ch_kit_setup(player=None)` runs `self._setup(KIT_PLAYER if player is None else player)`. Finder: kit_tools.py:1050-1063 gives ch_run_chart the same None-to-KIT_PLAYER.
- **Splits:** doc says default "System", code default is None which becomes "Player1". They differ for a caller who copies the doc signature and omits kit_player.
- **Ruling quotes:** None found.
- **Tests:** none for the doc signature.
- **Open:** verifier did not trace server.py into kit_tools for ch_run_chart; a run with no kit_player would settle it. Finder only: "Edge Walk Pair" path in the doc has no name check (K7).
- **Hook gap:** no

### L-F25-08: What does "kind A" mean?
- **Source:** I7-106, I7-117, I7-121 (G10); F25 S5; V25 S5.
- **Kind:** K1
- **Verdict:** CONFIRMED-CHANGED. Verifier's reading wins: not a split. test-rubric.md:39 says "the engine already has its answer", so it is a rule written twice in production, the same as kind A. The finder's "even when nothing else answers" is wrong.
- **Homes:** DOCS\derive-once-review.md:30 "**A.** A rule written twice in production code." DOCS\test-rubric.md:39 and :148 "(derive-once, kind A)".
- **Splits:** None (verifier removed the split).
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** no

### L-F25-09: Real-app record file name (realapp.jsonl) in two code files
- **Source:** I7-044..049 (G8); F25; V25 (K6). Same question as L-F27-18.
- **Kind:** K6
- **Verdict:** CONFIRMED
- **Homes:** H tools\hydra_ui_mcp.py:81 `REALAPP_FILE = "realapp.jsonl"` (comment :80 "field names are a contract"); G review_rules.py:49 `REALAPP_FILE = "realapp.jsonl"` (comment says hydra_ui_mcp.py owns the name and fields). DOCS\ui-testing.md:237-251 field table (finder). Gate conditions (dirty False, commands >= 1, commit on branch, review_rules.py:383-386) are not on the page (finder only, not reached by verifier).
- **Splits:** None by value.
- **Ruling quotes:** None found.
- **Tests:** no test seen that holds the two constants equal (verifier did not search G tests beyond the finder). G test_docs_numbers.py:54 holds the gates README to name realapp.jsonl.
- **Open:** page vs gate conditions not reached.
- **Hook gap:** no

### L-F25-10: Doc picture size limits (500 KB per file, 4 MB folder)
- **Source:** I6-245..252 (G7); F25; V25; F25b (D300).
- **Kind:** K6
- **Verdict:** CONFIRMED
- **Homes:** H tools\doc_shots\shrink.py:20-21 MAX_FILE_BYTES = 500*1024, MAX_FOLDER_BYTES = 4*1024*1024. H tests\test_docs_match_code.cpp:2024-2025 kDocImageMaxBytes, kDocImagesMaxTotalBytes (same values). F25b (not verified): decision log D300 (docs\audit\2026-10-03-fix-decisions.md:1662-1666) holds the user's words 500 KB / 4 MB; each file's comment says it matches the other (test_docs_match_code.cpp:2022-2023, shrink.py:17-18).
- **Splits:** None by value.
- **Ruling quotes:** D300 line is a decision-log entry (F25b), not a `## Confirmed` line.
- **Tests:** none compares the two.
- **Open:** none.
- **Hook gap:** no

### L-F25-11: Does any test check the `## Owner map` shape?
- **Source:** I7-171, I7-174, I7-175 (G11); F25; V25.
- **Kind:** K8 (also K1, K5 for restated statuses in scout/planner/reviewer)
- **Verdict:** CONFIRMED (no match for "owner map" in H tests, H tools, G *.py). Verifier: a test could use another word ("owner-map"); I7-174 row wording not opened.
- **Homes:** DOCS\owner-map.md:3 (one-home claim), :21-29, :37. C agents\scout.md:11, planner.md:15, reviewer.md:17; H CLAUDE.md; rule 15 (finder).
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** none found.
- **Open:** wider search words.
- **Hook gap:** no

### L-F25-12: Does a probe lead of exactly 0 read "early" or "late"?
- **Source:** I7-091..093 (G9); F25 side; V25.
- **Kind:** K2 candidate
- **Verdict:** CONFIRMED
- **Homes:** H tools\ch_probe\experiments\edge_probe.py:172-173 and 198 `'early' if p.lead_ms >= 0 else 'late'`. H tools\ch_mcp\server.py:450-451 "a positive lead presses early, a negative one late" (says nothing of zero). DOCS\clone-hero-mcp.md:53.
- **Splits:** the docs state a sign rule for positive and negative; code calls 0 "early". They differ at lead exactly 0.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** no

### L-F25-13: Ruling line shape, labels, INSTALL_DATE (README vs ruling_rules.py)
- **Source:** I6-214..223 (G5); F25 only.
- **Kind:** K1, K5
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** G ruling_rules.py:1-30, 94-110 (CONFIRMED/UNCONFIRMED, INSTALL_DATE "2026-11-14", "File it", "Do not file", SESSION_NOTE); DOCS\rulings\README.md:3,18-20,26-32,40-45,51,55-57 (README:51 repeats INSTALL_DATE and 2026-11-13); H CLAUDE.md; agent-rules.md:33-37; G README.md:59.
- **Splits:** None among these homes (finder).
- **Ruling quotes:** None found.
- **Tests:** G tests\test_ruling_rules.py:25 (format page rule); test_docs_numbers.py:54-56 (README must hold the approval step, not the labels).
- **Open:** not reached by verifier. See also L-F27-11.
- **Hook gap:** no

### L-F25-14: Issue tracker, wayfinder and triage label wording
- **Source:** I6-230..236, I7-213..215 (G6); F25 only.
- **Kind:** K1, K7
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** DOCS\issue-tracker.md:28,40-47; DOCS\triage-labels.md:7-13; H CLAUDE.md "each label string equal to its name"; C settings.json:25,39,53 and G settings.fragment.json:15,29,43 (`gh api *` denied); G README.md:73. The /wayfinder and /triage skills were not found in scope.
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** none. Nothing checks that the repo's real labels match.
- **Open:** not reached; `gh` not called.
- **Hook gap:** no

### L-F25-15: Doc screenshot crop rule and Reset-to-defaults words
- **Source:** I6-245, I6-248, I6-252 (G7); F25 only. Same question as L-F26-12 and L-F27-21.
- **Kind:** K1
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** DOCS\doc-screenshots.md:15; H tools\doc_shots\shots.txt:6-9; H tools\doc_shots.ps1:28; reset step in doc-screenshots.md:25 and shots.txt:12-20 (words from src\ui\settings_bar.cpp:226 and library_dialogs.cpp:137).
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** test_docs_match_code.cpp:15,2032 holds shots.txt equal to the picture files and the pages (not the crop rule or reset words).
- **Open:** not reached.
- **Hook gap:** no

### L-F25-16: Real-app record gate conditions vs the ui-testing page
- **Source:** I7-044..049 (G8); F25 only.
- **Kind:** K3
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** DOCS\ui-testing.md:241-249 (seven fields; `commands` "how many command lines Hydra answered"); G review_rules.py:383-386 (dirty must be False, commands int >= 1, commit in branch).
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** G test_review_rules.py:404 uses literal fields; none holds page equal to gate.
- **Open:** not reached.
- **Hook gap:** no

### L-F25-17: Handoff kinds, statuses, scratch-path skip dirs
- **Source:** I7-192..198 (G12); F25 only.
- **Kind:** K1, K5
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** DOCS\handoff.md:11-37,49-59; H tools\check_handoff.py:28,31,32; H .claude\commands\handoff.md:11,13; H tests\test_docs_match_code.cpp:1233 kNotPermanentDirs = {"docs/handoffs/", "docs/superpowers/"} while handoff.md:37 says it "skips docs/handoffs/".
- **Splits:** None in value (finder).
- **Ruling quotes:** None found.
- **Tests:** none holds the kind list to the page.
- **Open:** not reached. See L-F26-08 and L-F27-16.
- **Hook gap:** no

### L-F25-18: "Edge Walk Pair" probe chart path with no name check
- **Source:** I7-093 (G9); F25 only.
- **Kind:** K7
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** DOCS\clone-hero-mcp.md (chart path C:\Clone Hero\songs\Hydra Probe\Edge Walk Pair); nearest owner H tools\ch_probe\probe_songs.py:64,68 (DEFAULT_OUT, NAME_PREFIX). Also builds-<n>.json / press-<n>.csv names in doc, server docstring, game_tools.py:1151.
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** not reached.
- **Hook gap:** no

### L-F25-19: Clone Hero build number 6142 typed in many places (F25b I6-194)
- **Source:** I6-194; F25, F25b; no verifier (V25b did nothing). Same question as L-F27-23.
- **Kind:** K1, K8
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** owner of "which build is installed": H tools\ch_mcp\build_facts.py:69-81 `current_build`. Copies: DOCS\this-machine.md:27 "Clone Hero build 6142"; docs\clone-hero\README.md:4,70,88; docs\clone-hero\code\README.md:6,12; tools\ch_mcp\dump_facts.py:381 `DEFAULT_BUILD="6142"`; tests\test_docs_match_code.cpp:719 `kBuild11="6142"`; launch_patch.json:3; menu_fields.json:3; start_candidates.json:3; ch_probe\constants.py:22; ch_probe\SPEC.md:76; facts-6142.tsv names; tools\ch_mcp\README.md:12,20 dump-folder path.
- **Splits:** None
- **Ruling quotes:** None found. Searched: rulings topic files by awk grep.
- **Tests:** docs test holds constants.py addresses to facts-6142.tsv, not the build number.
- **Open:** F25b unverified.
- **Hook gap:** no

### L-F25-20: Screenshot shot checks: `!! failed at line N` vs `line N: ...` (F25b I6-247)
- **Source:** I6-247; F25b; no verifier.
- **Kind:** K3, K6, K7
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** DOCS\doc-screenshots.md:13 a missing element "stops the run with `!! failed at line N`"; H tests\ui\uitest_commands.cpp:289-293,363-366,382-385 (expect-text and rect failures go through `refuse`: `line N: "<text>" is not on screen`, `line N: rect found no item`); :432 `!! failed at line N` only for engine actions; DOCS\ui-testing.md:97 states the split correctly. Timeout claim true: shots.txt:10 sets 120, code default `CommandSession::timeout = 30.0` (uitest_commands.h:19). shots.txt has 8 expect-text and 8 screenshot lines but no test checks each screenshot has one; test_docs_match_code.cpp:2032-2133 checks only picture files and names. doc_shots.ps1:27-41 is a third reader of the rect rule.
- **Splits (finder only):** doc-screenshots.md:13 says `!! failed at line N` for a missing element; the code says `line N: ...` for expect-text/rect. They differ for an expect-text or rect failure.
- **Ruling quotes:** None found.
- **Tests:** none holds these equal.
- **Open:** F25b unverified.
- **Hook gap:** no

### L-F25-21: Wording when Hydra dies or goes silent (F25b I7-053)
- **Source:** I7-053; F25b; no verifier.
- **Kind:** K1, K3
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** owner H tools\hydra_ui_mcp.py (`_kill_report` 557-601; lines 460, 643). DOCS\ui-testing.md:282 ("`Hydra exited with code X`"), :251; tools\test_hydra_ui_mcp.py (257, 281, 455, 478, 575, 584, 593) copy the wording. Code also has "Hydra's window closed and Hydra exited with code 0: the user closed it, or a quit command was sent" (584-586) and "closed its live channel but kept running for N s" (578-582, no doc home). Scratch-folder age "a week" copied in ui-testing.md:282 and hydra_ui_mcp.py:599-601; owner G\cleanup_sweep.ps1:54 `$ScratchAgeDays = 7`; plan 2026-11-07-shell-output-to-disk.md:38,148,270 copies "7".
- **Splits:** None stated.
- **Ruling quotes:** None found (ui-and-display.md:69,78 are about other matters).
- **Tests:** tools\test_hydra_ui_mcp.py pins the wording.
- **Open:** F25b unverified.
- **Hook gap:** no

### L-F25-22: Test-rubric record shape and counts (F25b I7-109, I7-122, I7-106)
- **Source:** I7-106, I7-109, I7-122; F25, F25b; no verifier.
- **Kind:** K1, K3
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** DOCS\test-rubric.md (29, 47-53, 113-152), DOCS\test-rubric-examples.md (246-329), plan docs\superpowers\plans\2026-11-14-test-suite-audit.md:167,210-225. Differences: rubric lists 12 questions (Q12 "what did you read"), record has `Read` plus answers 1-11, plan says "answers 1 to 11". test-rubric.md:47 says "two lines" (Engine, Claim) but the sample at 48-51 and the record at :130 add a third "They differ." line. "a sentence or two" vs Q4/Q8/Q2 needing more. Rubric record (:113) says `Activation::refill_tick` has only a Preview caller; test-rubric-examples.md:9 says that function is gone. Pitch Dark values 128520 and 128400 copied in test-rubric.md:15,48-49,130,175 and examples:23. Commit 003d76a3 exists.
- **Splits (finder only, unsettled):** test-rubric.md:113 says refill_tick has a Preview caller; test-rubric-examples.md:9 says it is gone.
- **Ruling quotes:** None found.
- **Tests:** none checks the record shape (searched test_docs_match_code.cpp, tools\check_*.py, G *.py, G tests).
- **Open:** F25b unverified.
- **Hook gap:** no

### L-F25-23: Is a result row's `song_id` "the chart's md5"? (F25b, ADR 0028)
- **Source:** I7-159 (G4); F25b; no verifier.
- **Kind:** K3
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** DOCS\this-machine.md:85 "a result row's `song_id` is the chart's `md5` (the hyhash ...)". H docs\adr\0028-a-chart-id-is-clone-heros-song-id.md names `song_id_read` in src\app\analysis.cpp as the one owner; a `.srb` id is a 16-byte checksum read from metadata, not an MD5 of the file; a `.sng` id is the MD5 of the inner notes file.
- **Splits:** None stated (loose wording only).
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** F25b unverified.
- **Hook gap:** no

### L-F25-24: Live protocol JSON written by C++ and by a Python fake (F25b)
- **Source:** I7-045..049 (G8); F25b; no verifier.
- **Kind:** K6, K1
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** H tests\ui\live_protocol.h:64-89 (owner, pinned by tests\test_live_protocol.cpp:14-57); H tools\fake_hydra_live.py:105,145 writes the same shape in Python (kept "for what the real exe cannot do"); DOCS\ui-testing.md:265,271. Values 25, 60 and reply margin 15 in Python (hydra_ui_mcp.py:42,43; fake_hydra_live.py SESSION_TIMEOUT_S); ui-testing.md:267 points at `testwait::kUitestProcessCap` (5 minutes, tests\wait_util.h:22) without stating it.
- **Splits:** None stated.
- **Ruling quotes:** None found.
- **Tests:** test_live_protocol.cpp pins the C++ side only.
- **Open:** F25b unverified.
- **Hook gap:** no

# Part 2: F26 rows

### L-F26-01: How many numbers make up the early-fill window, and what is orange?
- **Source:** I8-061; F26 SPLIT-1; V26 SPLIT-1.
- **Kind:** K1, K2
- **Verdict:** CONFIRMED
- **Homes:** M\early-fill-orange-85-d250.md:11 "The reach is the hit window, `kHitWindowMs` (85 ms...) ... The slack is `kEarlyFillSlackMs` (60 ms), read by `is_e0`"; M\MEMORY.md:22 repeats it. H CONTEXT.md:610-614 "The window is one number on both sides of the deadline: the `Early fill window` setting, 60 ms by default"; :615-617 orange meaning. H src\core\model.h:106 `kHitWindowMs = 85.0`, :112 `kDefaultEarlyFillWindowMs = 60.0`. Verifier: no `kEarlyFillSlackMs` in src or tests.
- **Splits:** the note names a two-sided window with a constant the code does not have; CONTEXT.md says one number. They differ on the white side (the E with nothing to time).
- **Ruling quotes:** H docs\agents\rulings\code-and-engine.md:23 "change the early fill search to allow orange timing up to 85ms." and :24 "Orange only (Recommended)" (finder's quotes, not reopened). Neither names the white-side number, so they do not name both sides.
- **Tests:** none in G\tests. H tests\test_activation_rows.cpp is cited (first_use_table.md:15) as holding "the row is difficult exactly when a badge is orange"; not opened.
- **Open:** none.
- **Hook gap:** no

### L-F26-02: How many examples does a decision question need?
- **Source:** I8-134; F26 SPLIT-2; V26 SPLIT-2.
- **Kind:** K1, K3
- **Verdict:** CONFIRMED-CHANGED. Verifier's reading wins: doc-review.md:61 is about each option's example, not the predicate, so it is a third wording, not a clean two-vs-three. The real difference is audit-method asks for a gap example and the other two do not.
- **Homes:** M\questions-describe-the-real-rule.md:16 "one example from each end of its range". DOCS\audit-method.md:97 "one example from each end, and one from the gap where the two sides disagree". DOCS\doc-review.md:61 "Each option has a concrete example from each end."
- **Splits:** audit-method.md asks for end examples plus a gap example; the note and doc-review.md ask for end examples only. They differ when a question has a gap where the two sides disagree.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** no

### L-F26-03: Does a card quote or a brief quote count as the user's ruling?
- **Source:** I8-001, I8-002, I8-006; F26 SPLIT-3; V26 SPLIT-3. Same question as L-F25-01.
- **Kind:** K1, K3
- **Verdict:** CONFIRMED
- **Homes:** DOCS\premise-map.md:45 (ruling row may quote Confirmed, "a session card's quoted words", or the brief's quote). DOCS\doc-review.md:63 "Confirmed ... a session card or the brief's quote". DOCS\agent-rules.md:35-37 (Confirmed is the user's words; "A session that needs the user's words for it looks for the quote in a session card"). H CLAUDE.md:25 "a card quote is not a filed ruling"; CLAUDE.md:41 and rulings\README.md:16-18 (only `## Confirmed`). M\MEMORY.md:1,49 also point at Confirmed only.
- **Splits:** plan rows and plan review accept a card quote; the plain word "ruling" elsewhere does not. They differ when a plan's ruling row rests on a card quote.
- **Ruling quotes:** None found naming either side.
- **Tests:** none holds the index equal to notes (searched G\tests for MEMORY and memory).
- **Open:** none.
- **Hook gap:** no

### L-F26-04: How does the main session relay agent status (and what is on the evidence line)?
- **Source:** I8-139, I8-140; F26 SPLIT-4 and K5 item; V26 SPLIT-4 and K5.
- **Kind:** K1, K2, K5
- **Verdict:** CONFIRMED
- **Homes:** M\relay-status-in-plain-english.md "Skip lines from test fixtures (names starting `test_`)"; M\MEMORY.md:46 "one plain sentence per agent"; G\stall.py:30-33 STATUS_HEADER "Relay the claim to the user briefly, in plain English. When the evidence disagrees, add one sentence"; G\stall.py:34 `TOOL_TAIL = 5`; C\model-postures.md:11 repeats the header's rule almost word for word and "last five tools"; DOCS\brief-preamble.md:23 ("five is TOOL_TAIL in stall.py"); G README.md:29. Verifier: grep of stall.py for `test_`/`fixture`: no hit, so the skip is prose only.
- **Splits:** the note adds the `test_` skip and the second sentence; the MEMORY.md index line says only one sentence; the hook text has no skip rule. They differ for a status line from a `test_`-named fixture agent and when evidence disagrees.
- **Ruling quotes:** agents-and-workflow.md:26 "the status updates should be brief but relayed in plain english" (finder's quote, not reopened; names neither the second sentence nor the skip).
- **Tests:** none holds the prose to TOOL_TAIL (searched G\tests for TOOL_TAIL and "last five").
- **Open:** none.
- **Hook gap:** no

### L-F26-05: Which file states the installer invariants (AppId)?
- **Source:** I8-186; F26 SPLIT-5; V26 SPLIT-5.
- **Kind:** K1, K8
- **Verdict:** CONFIRMED-CHANGED. Verifier: weaker than a split; a loose note wording. build_installer.ps1 states other invariants (:85, :94).
- **Homes:** M\windows-installer.md:13 "Invariants, all stated in hydra.iss and build_installer.ps1". H installer\hydra.iss:23-24 "AppId must never change", :52 the GUID. build_installer.ps1 has no AppId/GUID line. M\hydra-deluxe-rename.md:13; M\MEMORY.md:42.
- **Splits:** None firm. (Finder called it a split; the verifier did not.)
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** no

### L-F26-06: Splax layout numbers in the note vs layouts.json
- **Source:** I8-017; F26 SPLIT-6; V26 (NOT CHECKED).
- **Kind:** K2, K8
- **Verdict:** NOT VERIFIED (verifier did not open video-tools\layouts.json, outside the brief)
- **Homes (finder only):** M\ch-disco-noflip-rule.md:15 "score=596,298,124,34 disc=708,660,56,56 at 1080p" "isn't in layouts.json"; C\skills\analyzing-sp-squeezes\SKILL.md:60; C:\Users\Patrick\Downloads\Hydra\video-tools\layouts.json (several Splax 1080p layouts, e.g. splax-2026 score [736,418,326,84], splax-halfspeed [596,456,280,68]).
- **Splits:** None firm (finder: the note's numbers are not in the file, so its claim holds).
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** which layout the note means.
- **Hook gap:** no

### L-F26-07: Is the window title "Hydra" or "Hydra Deluxe"?
- **Source:** I8-087; F26 side note; V26.
- **Kind:** K2 (finder candidate)
- **Verdict:** REFUTED as a conflict. Verifier: ui-testing.md:118-120 calls the ImGui window `Hydra` and the version bar text `Hydra Deluxe <version>`; two different things. Whether the OS window title is the bare "Hydra Deluxe" stays open (verifier's grep of src found nothing).
- **Homes:** M\hydra-deluxe-rename.md:13; ui-and-display.md:127 (Unconfirmed) "The window title is now the bare name 'Hydra Deluxe' with no version"; DOCS\ui-testing.md:118-120,131.
- **Splits:** None
- **Ruling quotes:** none Confirmed; line 127 is under Unconfirmed.
- **Tests:** none.
- **Open:** read the app's window creation code.
- **Hook gap:** no

### L-F26-08: The /handoff command vs the handoff page and checker
- **Source:** I8-235..245; F26; V26 (partly opened). Same question as L-F25-17 and L-F27-16.
- **Kind:** K1, K3
- **Verdict:** CONFIRMED-CHANGED. K1 (slug, status words) confirmed. K3 changed: command step 4 says "Fix every row it marks `missing`"; the finder's "leave it" belongs to the second run, and the verifier did not reopen that line or handoff.md:33.
- **Homes:** H .claude\commands\handoff.md:2,8,11-13,15 (kebab-case slug of 2 to 6 words; status words `missing`, `not-on-main`, `not-pushed`, `manual`). DOCS\handoff.md:7 (`<slug>`, no word count), :13, :33 "A row whose check fails is wrong until someone fixes the handoff.", :51-55. H tools\check_handoff.py:31,33 (owner of the words). DOCS\brief-preamble.md:95-97.
- **Splits:** command text vs page on what to do with a still-`missing` row after the second run (verifier says the quoted "leave it" is from the second run only, unsettled). Differ for a row still missing after the second run.
- **Ruling quotes:** None found.
- **Tests:** H tools\test_check_handoff.py tests the checker, not the command text.
- **Open:** command line "leave it" vs handoff.md:33 not reopened.
- **Hook gap:** no

### L-F26-09: Is an agent still "parked" after a call, and who decides it?
- **Source:** I9-040, I9-041, I9-043, I9-052; F26; V26. Same question as L-F27-01.
- **Kind:** K6, K1
- **Verdict:** CONFIRMED
- **Homes:** G\agent_stop.py:84 `parked_on`, :256-276 (sets `parked`, `parked_since`). G\stall.py:238-240 shows "parked since" unless `not (_is_time(last) and last > since)`. G README.md:41 "marks the agent finished, unless the agent only stopped to wait ... it is parked". (progress.py:77-79 clears it on any call, per F27.)
- **Splits:** None by value; two code places answer "does a later call end parking" (see L-F27-01).
- **Ruling quotes:** review-and-hooks.md:19 "a hand-back with no sign-off line records CLEAN and tells the main session." (finder's quote, about the CLEAN rule, not about parking).
- **Tests:** test_agent_stop.py holds code and records; no test holds README:41 prose to the code.
- **Open:** none.
- **Hook gap:** no

### L-F26-10: Where is the waiver record form written down?
- **Source:** I9-021, I9-022; F26; V26.
- **Kind:** K8, K1
- **Verdict:** CONFIRMED
- **Homes:** G\waiver.py:3 "the record forms are in gates/README.md". G README.md names no waiver field list (verifier grep for prompt_id/prefix: no hit). G\waiver.py:22-26,43-44 writes kind, prefix, prompt_id, transcript_path, session_id, time. G\common.py:11-12 REVIEW_KINDS and G tests\test_common.py:224-252 (sample records; the waiver sample has no `time`).
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** test_common.py round-trips review records; a test of waiver.py's fields not searched.
- **Open:** none.
- **Hook gap:** no

### L-F26-11: Which docs does the retired-words test scan?
- **Source:** I9-024; F26; V26.
- **Kind:** K1, K3, K7
- **Verdict:** CONFIRMED
- **Homes:** G\tests\test_docs_numbers.py:25-26 scans five docs (CLAUDE.md, agent-rules.md, brief-preamble.md, derive-once-review.md, doc-review.md) for ten RETIRED patterns including `every 10 tool calls`; no rulings files are scanned. Plan hooks-from-scratch.md:108 "The numbers (100, 150, 20, 30, 5, 10) sit at the top of `progress.py` and nowhere else in code; the docs quote them, and a test in T11 greps the preamble and CLAUDE.md for `110` and `every 10 tool calls`" (two patterns, two docs). Confirmed line review-and-hooks.md:33 "demand status every 10 tool calls ..." holds a retired phrase (finder's quote, not reopened).
- **Splits:** plan says two patterns on two docs; test holds ten on five (finder's K3; verifier confirmed the test side only).
- **Ruling quotes:** review-and-hooks.md:33, :34 (finder), not reopened.
- **Tests:** this test.
- **Open:** none.
- **Hook gap:** no

### L-F26-12: Picture crop rule (a rect line before a ref-less screenshot)
- **Source:** I7-223; F26 only. Same question as L-F25-15 and L-F27-21.
- **Kind:** K1
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** DOCS\doc-screenshots.md:15; H tools\doc_shots\shots.txt:6-9 ("full-frame"); H tools\doc_shots.ps1:27-29 ("no bar") and the code at :37-38 (acts only when the screenshot line has exactly one argument).
- **Splits:** None; wording differs ("no ref" / "full-frame" / "no bar").
- **Ruling quotes:** None found.
- **Tests:** none found.
- **Open:** not reached.
- **Hook gap:** no

### L-F26-13: What may a code comment hold (history, restated rules)?
- **Source:** I8-030, I8-031; F26 only.
- **Kind:** K1, K7
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** M\comments-explain-current-code.md:11,15; M\MEMORY.md:23; DOCS\brief-preamble.md:119 "A comment names where a rule lives; it never restates the rule"; rulings code-and-engine.md:22 (Confirmed) "comments exist to explain the current code, not give a history lesson" (session c35a4b7d).
- **Splits:** None (the note and preamble answer two different rules).
- **Ruling quotes:** code-and-engine.md:22 as above (history rule only).
- **Tests:** none found (searched G\*.py for "comment").
- **Open:** not reached.
- **Hook gap:** no

### L-F26-14: Premise-map test pins phrases only
- **Source:** I9-002..006; F26 only. Related: L-F25-02.
- **Kind:** K1
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** G\tests\test_agent_files.py:13-23 (PLANNER_PHRASES, REVIEWER_PHRASES; nine phrases) vs DOCS\premise-map.md:13 (owner); C agents\planner.md:13,15, reviewer.md:13; H CLAUDE.md:55; doc-review.md:63.
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** this test pins words in agent files, not agreement with the page.
- **Open:** not reached.
- **Hook gap:** no

### L-F26-15: Song length: D75 or D255?
- **Source:** I8-177; F26 only.
- **Kind:** K1
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** M note (D75); H src\app\analysis.h:216 "(app::song_length_ms, D255)" (owner); H tools\ch_mcp\hydra_end.py:33-76 carries `song_length_ms` out of a hydra_replay dump. F27 (I10b-051): D255 amends D75 items 1 and 2 (fix-decisions.md:1052); trace_hits.py:67-69 says "D331".
- **Splits:** None by value; D-number differs between note and code comment.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** not reached.
- **Hook gap:** no

### L-F26-16: Hook record field lists copied into tests
- **Source:** I9-007..014, I9-021; F26 only.
- **Kind:** K1
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** owner G\common.py:36-48 `log_refusal` (G README.md:79 "the only writer"); G tests\test_refusals.py:16 FIELDS; common.py:21-33 output shapes and test_common.py:52-65; common.py:11-12 REVIEW_KINDS (G README.md:75 "the one place the review record fields are checked") and test_common.py:224-252.
- **Splits:** None (agree today).
- **Ruling quotes:** None found.
- **Tests:** the tests call production code.
- **Open:** not reached.
- **Hook gap:** no

### L-F26-17: Gates README names and rulings README heading pinned by a test
- **Source:** I9-031..033; F26 only.
- **Kind:** K1, K7
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** G tests\test_docs_numbers.py:51-59 (README_NAMES, APPROVAL_HEADING, OLD_HOOK_LIVE); G README.md; DOCS\rulings\README.md:36 heading `## How to file a ruling: the approval step`.
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** this test.
- **Open:** not reached.
- **Hook gap:** no

### L-F26-18: README prose on stop records (README:41) vs agent_stop.py
- **Source:** I9-040..052; F26 only.
- **Kind:** K1
- **Verdict:** NOT VERIFIED (the parked part is in L-F26-09)
- **Homes (finder only):** G README.md:41,43,45,51; G agent_stop.py:234-292 (outcomes "verdict recorded", "no verdict", "not a reviewer", "crashed"); G tests\test_agent_stop.py:203-298,593-643.
- **Splits:** None
- **Ruling quotes:** review-and-hooks.md:19 (finder).
- **Tests:** no test holds README:41 prose to the code.
- **Open:** not reached.
- **Hook gap:** no

### L-F26-19: Memory index lines are second statements of each note
- **Source:** I8-006 (I8-001/002); F26 only.
- **Kind:** K1
- **Verdict:** NOT VERIFIED (drift found in two index lines, both in L-F26-01 and L-F26-04)
- **Homes (finder only):** M\MEMORY.md index lines vs each note; MEMORY.md:22 (orange), :46 (relay).
- **Splits:** see L-F26-01 and L-F26-04.
- **Ruling quotes:** None found.
- **Tests:** none holds the index equal to the notes.
- **Open:** not reached.
- **Hook gap:** no

### L-F26-20: DMBot join key, routes and upload behaviour
- **Source:** I8-048, I8-049, I8-052; F26 only.
- **Kind:** K1
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** M\dmleaderboards-comparison.md:14-45; H src\app\dm_report.h:3 "leaderboard `identifier` == Hydra `hyhash`". Replace rule and routes: one home (the note); not found in docs\clone-hero\07-internals.md IN-55 or docs\adr.
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** none found.
- **Open:** not reached.
- **Hook gap:** no

# Part 3: F27 rows

### L-F27-01: Is an agent still "parked" after it makes a call?
- **Source:** I9-098; F27; V27. Same question as L-F26-09.
- **Kind:** K6
- **Verdict:** CONFIRMED
- **Homes:** G\progress.py:77-79 "A call means the agent is no longer parked on background work (agent_stop.py sets the flag)." then pops `parked`, `parked_since`. G\stall.py:~236-240 shows "parked since" only `if rec.get("parked") and _is_time(since) and not (_is_time(last) and last > since)`. G\agent_stop.py:256-276. G README.md:41.
- **Splits:** progress.py clears on any call; stall.py also guards with `last > since`. They differ only at `last == since` or a stale record where `parked` is set but `last` is not newer.
- **Ruling quotes:** None found.
- **Tests:** test_progress.py:158-177 (progress side); test_stall.py separate; none holds them equal.
- **Open:** none.
- **Hook gap:** no

### L-F27-02: How is a document key computed, and is the count the same bytes?
- **Source:** I9-131; F27; V27.
- **Kind:** K1, K6
- **Verdict:** CONFIRMED
- **Homes:** G\review_rules.py:65-71 `normalized` ("a UTF-8 byte-order mark dropped and CRLF made LF"; code `replace(b"\r\n", b"\n")` only); docstring "publish_gate.py's small-text count reads the same bytes." G\publish_gate.py:19-29 calls `normalized(raw)` then also `.replace("\r\n","\n").replace("\r","\n")` and decodes UTF-16 itself. G tests\test_review_dispatch.py:23 `DOC_KEY` and test_review_rules.py:333 (same hex typed twice). G test_docs_numbers.py:42 TYPED_KEY (finder).
- **Splits:** key path vs count path (found by the verifier): a lone CR and UTF-16 input are treated differently, which contradicts the "reads the same bytes" docstring. Also (finder, verifier confirmed the code side only): the test row text says "CRs removed" while code drops only CRLF pairs; the verifier did not open the test row text.
- **Ruling quotes:** None found.
- **Tests:** test_publish.py:26 binds `real_review_rules.doc_key` (same function).
- **Open:** test row wording not opened.
- **Hook gap:** no

### L-F27-03: Which calls count as a hand-back (budget exemption vs saved message)?
- **Source:** I9-092; F27 side note 3; V27.
- **Kind:** K6, K1
- **Verdict:** CONFIRMED (as a K6 pair)
- **Homes:** G\progress.py:19 `REPORT_TOOLS = ("SubagentHandback", "StructuredOutput")`; G\percall.py:168 uses `progress.REPORT_TOOLS`; percall.py:102 hard-codes `== "SubagentHandback"` for saving the message; percall.py:104 `write_json_atomic(agent_stop.handback_path(...))`; G agent_stop.py:39,88,194; G README.md:75,182; DOCS\derive-once-review.md:128, doc-review.md:106, C agents\reviewer.md:25.
- **Splits:** one list covers both tools, the save covers only SubagentHandback. They differ for an agent that ends with StructuredOutput: exempt from the budget but its message is not saved.
- **Ruling quotes:** review-and-hooks.md:19 (finder; CLEAN rule).
- **Tests:** test_progress.py:388-421; test_agent_stop.py read the same path through `handback_path`; none compares README to code.
- **Open:** none.
- **Hook gap:** no

### L-F27-04: How does a doc cite a file that left the repo?
- **Source:** I10-007; F27; V27.
- **Kind:** K6, K1
- **Verdict:** CONFIRMED
- **Homes:** H tools\check_pointers.py:38 `` `git show ([0-9a-fA-F]{7,40}):([^`\s]+)` `` (backticks required; upper or lower hex). H tests\test_docs_match_code.cpp:1334-1346 `is_removed_citation` (lowercase `[0-9a-f]{7,40}:` plus the docs path, no backticks needed; or "never committed", any case). H CLAUDE.md:57 "as `<path>` (removed; `git show <commit>:<path>`)". Verifier note: 07-internals.md IN-76 line 5 uses `(removed; git show 6804353f:...)` without backticks, which the C++ test accepts and the Python form would not.
- **Splits:** (1) backticks required in Python, not in C++. (2) uppercase hash accepted by Python, refused by C++. (3) "never committed" accepted by C++ only.
- **Ruling quotes:** None found.
- **Tests:** none holds them equal.
- **Open:** not run.
- **Hook gap:** no

### L-F27-05: What does a commit hash (or bare hex token) look like?
- **Source:** I10-017, I10-029, I10-030; F27; V27 (I10-017).
- **Kind:** K6
- **Verdict:** CONFIRMED (session_cards HEX and the digit rules not opened by the verifier)
- **Homes:** H tools\check_handoff.py:28 `_FULL_HASH` `^[0-9a-fA-F]{40}$`. G review_rules.py:56 `_HEX` `^[0-9a-f]{40}([0-9a-f]{24})?$`; review_rules.py:26-28 waiver patterns `{7,40}`, `{7,64}`, `{7,40}` with `(?i)`. H tools\doc_tokens.py:39 `_HEX` 7-40 either case; :50 `_BARE_HEX` 7-40 lowercase (finder: with a digit and a letter a-f). DOCS\handoff.md:19. Finder only: check_pointers.py:38; test_docs_match_code.cpp "shown" regex 7-40 lowercase; ST common.py:78 (digit required), :79, :541 (40-64).
- **Splits:** (a) upper-case 40-hex passes check_handoff and fails review_rules._HEX. (b) doc_tokens `_HEX` takes upper, `_BARE_HEX` does not. (c) doc-review waiver allows up to 64, the other two up to 40. (d, verifier's) waiver phrases are case-insensitive while `_HEX` in the same file is lowercase only.
- **Ruling quotes:** None found.
- **Tests:** none holds equal.
- **Open:** session_cards HEX rules not opened.
- **Hook gap:** no

### L-F27-06: Which song.ini lines count (section, pro_drums, chart file order)?
- **Source:** I10b-022, I10b-023; F27; V27.
- **Kind:** K2, K6
- **Verdict:** CONFIRMED (the `.mid`-before-`.chart` order was not checked)
- **Homes:** H tools\ch_mcp\game_files.py:276-290 `_song_section_lines` ("Keys before any section header count too"); :348-350 `.lower() in ("true","1")`. H src\parse\song.cpp:2217-2238 (`in_song_section = false`; keys before a `[song]` header are ignored) and :2274-2283 `ini_drum_key_is_true` (any non-zero int32 or trimmed case-insensitive "true"); src\parse\song.h:177-183. H install_chart.py:37 `CHART_FILES = ("notes.chart","notes.mid")`; src\parse\chart_files.cpp:28-41 prefers .mid.
- **Splits:** (1) keys before any section count in Python, not in the engine. (2) `pro_drums` values `2`, `-1` count in the engine only. (3, finder only) `.mid` over `.chart` in the engine; the Python tuple lists `.chart` first (what install_chart does with the order not read).
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** Python use is a printed note (bot_run.py:762-764, not opened); order split unchecked.
- **Hook gap:** no

### L-F27-07: The dynamics-stop flag bit 0x10: two homes in two formats
- **Source:** I10b-028; F27; V27.
- **Kind:** K1, K8
- **Verdict:** CONFIRMED
- **Homes:** H tools\ch_mcp\frida_tools.py:66-68 `DYNAMICS_STOP_BIT = 0x10` (comment names SC-16). H docs\clone-hero\06-scoring.md:175-177 "bit 0x10 of the engine flags word". H tools\ch_probe\constants.py has no 0x10 entry; its header (finder) says "This is the ONE place the numbers live".
- **Splits:** None by value.
- **Ruling quotes:** None found.
- **Tests:** the docs test (:996-1010) checks listed constants.py names, not this bit.
- **Open:** none.
- **Hook gap:** no

### L-F27-08: Where do the kit's definition, map category and layout facts live (IN-76 vs IN-82)?
- **Source:** I10b-033, I10b-034, I10b-035; F27; V27.
- **Kind:** K1, K8
- **Verdict:** CONFIRMED-CHANGED. The finder's "IN-76 does not hold the fact" is refuted for the map category and layout (the only values the kit_tools comment covers). IN-76 (07-internals.md:833-835) does hold them.
- **Homes:** H tools\ch_mcp\kit_tools.py:62-63 "The fact's home is block IN-76"; :59-96 and :153-181 `KIT_ELEMENTS`, `KIT_BINDINGS` ("The one table of what can be pressed on the kit"). H docs\clone-hero\07-internals.md IN-76 :833-835 ("category 1 ("Gameplay") with layouts 0 ("Default") and 2 ("Drums")") and IN-82 :904-908 (id 2, category 1, layout 2; "The action names are those of `KIT_BINDINGS` in tools/ch_mcp/kit_tools.py").
- **Splits:** None (same values, no disagreement found). Remaining: action names live in kit_tools.py while IN-82 points at the code; the code comment for definition id/name (`KIT_DEFINITION_ID`, "MIDI Drums") names no home.
- **Ruling quotes:** None found.
- **Tests:** the docs test does not read kit_tools.py's tables.
- **Open:** none.
- **Hook gap:** no

### L-F27-09: Handover record and brief hash computed by two routes
- **Source:** I9-109; F27 only.
- **Kind:** K6, K1
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** G\review_dispatch.py:170-172 hashes `prompt` from the tool input; G\agent_stop.py:199-201 hashes the transcript's first user message; G\common.py:11 REVIEW_KINDS; G README.md:43; G tests\test_review_dispatch.py:26,29 fixtures.
- **Splits:** None found today.
- **Ruling quotes:** None found.
- **Tests:** test_agent_stop.py round trip (not read line by line).
- **Open:** not reached.
- **Hook gap:** no

### L-F27-10: Who owns reading the `Agent:` trailer; which trailers are required?
- **Source:** I9-116; F27 only.
- **Kind:** K1, K6, K8
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** G\agent_stop.py:114 "The one reader of the Agent trailer"; G\stall.py:210-232; G\ref_judge.py:182-229 `missing_trailers` (Task:, Agent:, Session:; merge commits exempt); DOCS\brief-preamble.md:65-67; G\session_cards_start.ps1; G tests\test_ref_judge.py:22, test_review_rules.py:21.
- **Splits (finder only, "not graded"):** agent_stop.py:114 "The one reader" vs ref_judge.py:186 also reading trailers by key (different purpose: presence check).
- **Ruling quotes:** None found.
- **Tests:** none compares preamble to ref_judge.
- **Open:** not reached.
- **Hook gap:** no

### L-F27-11: What lines may a ruling-file commit add, and what passes free?
- **Source:** I9-140, I9-143; F27 only. Related: L-F25-13.
- **Kind:** K1, K3
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** G\ruling_rules.py:95-110, :177, :189, :345; DOCS\rulings\README.md:43 "refuses any added line that is not in the right shape"; G README.md:57-61; H CLAUDE.md:41; ST common.py:53. Pass-free list (removed, blank, headings, README prose, one title line) only in code and test_ruling_rules.py:296-329.
- **Splits (finder only):** README.md:43 "any added line" vs the code's exemptions. They differ for a README or heading edit.
- **Ruling quotes:** None found.
- **Tests:** test_ruling_rules.py:21-29,125-178,296-329 (code side only).
- **Open:** not reached.
- **Hook gap:** no

### L-F27-12: Which transcript records count as the user's words or the brief?
- **Source:** I9-146; F27 only.
- **Kind:** K6, K1
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** G\ruling_rules.py:220-235 via ST prepass.condense; G\review_rules.py:89-109 `human_prompt_text`; G\agent_stop.py:26-36 `first_user_message`, :70; ST common.py:384-395,474; ST prepass.py:89-140; G\stall.py:174-200; G tests\test_ruling_rules.py:1-8 (claims to copy tools/session_cards/fixtures shapes).
- **Splits:** None shown (filters differ by purpose; not compared line by line).
- **Ruling quotes:** None found.
- **Tests:** none holds the readers equal.
- **Open:** not reached.
- **Hook gap:** no

### L-F27-13: Status folder and call shape in status_append.ps1, common.py and the preamble
- **Source:** I9-174; F27 only.
- **Kind:** K1, K6
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** G\status_append.ps1:11 `$StateRoot` literal; G\common.py:10,124 STATE_ROOT and status path; G\progress.py:22 `STATUS_PREFIX`, :27-30 regex, `_helper_line`; G settings.fragment.json allow rule; DOCS\brief-preamble.md:13-15.
- **Splits:** None by value.
- **Ruling quotes:** review-and-hooks.md:34 "demand status updates every 5 minutes of real time and reject any calls with no updates after 10 minutes" and agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable" (finder's quotes, not reopened).
- **Tests:** test_status_append.ps1 uses HOOKS_TEST_STATE_ROOT; test_progress.py pins STATUS_PREFIX behaviour; none holds the ps1 literal equal to common.STATE_ROOT. Finder's side note: the script always writes the live folder, "by design in its comment; a claim, not evidence".
- **Open:** not reached.
- **Hook gap:** no

### L-F27-14: Which line starts a memory note's card pointers?
- **Source:** I10-006; F27 only.
- **Kind:** K6, K1
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** H tools\check_pointers.py:36 `SESSIONS_LINE = "Sessions:"`; ST link_memory.py:23 `SESSIONS_PREFIX = "Sessions: "`; H CLAUDE.md:27; tests ST test_session_cards.py:529,678,897 and H tools\test_check_pointers.py:100 type the literal.
- **Splits:** None by behaviour.
- **Ruling quotes:** None found.
- **Tests:** none compares writer and reader.
- **Open:** not reached.
- **Hook gap:** no

### L-F27-15: What counts as a pointer, and does the named file exist (Python vs C++ checkers)?
- **Source:** I10-011, I10-012, I10-014; F27 only.
- **Kind:** K6
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** H tools\check_pointers.py:108-134,136-229; H tools\doc_tokens.py:39-56; H tests\test_docs_match_code.cpp:1326-1330 `docs_path_re`, :1040, :232, :1420; G README.md:71. Finder's side finding: live folder `...\.claude\hooks\state\build_slots` still exists while hooks now live under `gates\`; check_pointers treats paths under `<hooks root>\hooks` as "retired".
- **Splits:** None by value.
- **Ruling quotes:** None found.
- **Tests:** test_check_pointers.py vs the C++ test; none holds equal.
- **Open:** not reached.
- **Hook gap:** no

### L-F27-16: Handoff Claims table heading, manual kinds, row statuses, scrape-mode words
- **Source:** I10-017, I10-018..020, I10-025, I10b-111; F27 only. Same question as L-F25-17 and L-F26-08.
- **Kind:** K1, K6
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** H tools\check_handoff.py:28,31 `CLAIMS_HEADING = "## Claims"`, :32 MANUAL_KINDS, :33 TABLE_STATUSES, :310; DOCS\handoff.md:15-27,39,47-51; DOCS\brief-preamble.md:97 (lists two of the three scrape words); H .claude\commands\handoff.md; H tools\test_check_handoff.py:226 TABLE_HEAD; H tests\test_docs_match_code.cpp:1517-1580 (`table_cells`, `is_table_row`, comments call themselves a copy of the Python rule).
- **Splits:** None by value.
- **Ruling quotes:** None found.
- **Tests:** none holds C++ and Python equal; handoff.md is not compared.
- **Open:** not reached.
- **Hook gap:** no

### L-F27-17: Build slot lock folder path and the three-build limit
- **Source:** I10-041; F27 only.
- **Kind:** K1, K2
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** H tools\build_slot.ps1:10,78 `...\.claude\hooks\state\build_slots` (live folder exists; gates state is `...\.claude\gates\state`); :75 `$BuildSlotCount = 3`; H CLAUDE.md:76; DOCS\agent-rules.md:20 (rule 14); G percall.py:20,109 refuse hydra_build_pass to agents; H tools\hydra_ui_mcp.py:24,720-737. No other reader of the lock fields found.
- **Splits:** None by value.
- **Ruling quotes:** G README.md:15 states the build-pass rule (finder: not checked against rulings files).
- **Tests:** none named.
- **Open:** not reached.
- **Hook gap:** no

### L-F27-18: Real-app record file and fields (F27's copy of the question)
- **Source:** I10-060; F27 only. Same question as L-F25-09.
- **Kind:** K1
- **Verdict:** NOT VERIFIED (the F25 side is in L-F25-09)
- **Homes (finder only):** H tools\hydra_ui_mcp.py:78-81,194-199; G review_rules.py:47-49,371-395; DOCS\ui-testing.md:237-251; G README.md:55.
- **Splits:** None by value.
- **Ruling quotes:** None found.
- **Tests:** tools\test_hydra_ui_mcp.py:519-586 pins the writer; none holds the reader's keys equal to the writer's.
- **Open:** not reached.
- **Hook gap:** no

### L-F27-19: The `screenshot: ` reply prefix
- **Source:** I10-062; F27 only.
- **Kind:** K1, K6
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** H tests\ui\uitest_commands.cpp:412 and uitest_engine.cpp:462 (the engine prints the prefix in two places); H tools\hydra_ui_mcp.py:84 `SHOT_REPLY`; H tools\doc_shots.ps1:25; DOCS\ui-testing.md:87; tools\test_hydra_ui_mcp.py:507,523,962.
- **Splits:** None by value.
- **Ruling quotes:** None found.
- **Tests:** typed copies only.
- **Open:** not reached.
- **Hook gap:** no

### L-F27-20: capture_window.ps1 version bump rule has no check
- **Source:** I10-077; F27 only.
- **Kind:** K1, K7
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** H tools\capture_window.ps1:23-50 ("Bump it whenever the C# block below changes"); DOCS\ui-testing.md:352,363.
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** none found.
- **Open:** not reached.
- **Hook gap:** no

### L-F27-21: Crop rule and the one list of doc pictures (F27's copy)
- **Source:** I10-081, I10b-072; F27 only. Same question as L-F25-15 and L-F26-12.
- **Kind:** K1
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** H tools\doc_shots.ps1:27-47; tools\doc_shots\shots.txt:1-10; DOCS\doc-screenshots.md:15,29; rect text produced by tests\ui\uitest_engine.cpp:409; tools\test_hydra_ui_mcp.py:904,973,988,996.
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** test_docs_match_code.cpp:2036 holds the picture list equal; none holds the crop rule's text equal.
- **Open:** not reached.
- **Hook gap:** no

### L-F27-22: What a written number means (precheck number canon vs docs test)
- **Source:** I10-087; F27 only.
- **Kind:** K6
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** H tools\precheck\decisions.cpp:22-31,88-92 (`kWholeNumberLimit = 1e15`, `kMaxHexDigits = 16`, `kBoldLevel = 99`); decisions.h:23-30; tools\precheck\definitions.cpp:86; H tests\test_docs_match_code.cpp:360,553; tests\test_precheck_check3.cpp:106-111.
- **Splits (finder only, "coverage, not a verified disagreement"):** precheck accepts separators `, ' _` and a trailing `f`/`u`; the docs-test regex does not.
- **Ruling quotes:** None found.
- **Tests:** none holds equal.
- **Open:** not reached.
- **Hook gap:** no

### L-F27-23: Build 6142, loader RVA, dump path and launch words typed in several files
- **Source:** I10b-012, I10b-014, I10b-017, I10b-018; F27 only. Same question as L-F25-19.
- **Kind:** K1, K8
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** H tools\ch_mcp\build_facts.py:28,64-103 (claims to be "the one home" of which build is installed); dump_facts.py:53-54,381; ch_probe\constants.py:24; launch_patch.json:3,4 (dump path repeats build_facts.py:28 `DUMPS_ROOT`); kit_methods.json:3; H tests\test_docs_match_code.cpp:719; loader RVA 0x3968D0 in docs\clone-hero\07-internals.md:666,688,698,738 (patch RVA 0x396EC5 only in the json); tools\ch_mcp\bot_run.py:99-107 words vs agent\launch_core.ts:44, launch.ts:107.
- **Splits:** None by value (all say 6142).
- **Ruling quotes:** None found.
- **Tests:** only constants.py addresses are compared (:998); none for the json.
- **Open:** not reached.
- **Hook gap:** no

### L-F27-24: Menu page words in menu_fields.json and frida_tools.py
- **Source:** I10b-030; F27 only.
- **Kind:** K1
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** H tools\ch_mcp\frida_tools.py:150-167; menu_fields.json:17-18; docs\clone-hero\07-internals.md IN-58.
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** tools\ch_mcp\tests\test_frida_tools.py:789 holds them equal (not opened by the verifier).
- **Open:** not reached.
- **Hook gap:** no

### L-F27-25: Play stop reasons and event words across TS and Python
- **Source:** I10b-039; F27 only.
- **Kind:** K1
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** H tools\ch_mcp\kit_tools.py:709-730; agent\press_schedule.ts:55-76 and :328,464,472; agent\kit.ts:1121,1156.
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** not read.
- **Open:** not reached.
- **Hook gap:** no

### L-F27-26: Dynamics words in C++, Python and the replay table
- **Source:** I10b-054; F27 only.
- **Kind:** K1, K6
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** H src\core\model.cpp `dynamic_str` (model.h:70); src\core\replay.h:140 (second C++ spelling); tools\ch_mcp\kit_tools.py:258-264; tools\ch_mcp\compare_scorestats.py:40-46,76 (copy "because this file runs on its own").
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** none holds the Python copies equal to model.cpp.
- **Open:** not reached.
- **Hook gap:** no

### L-F27-27: Function RVAs and field offsets: constants.py "ONE place" vs SPEC.md and doc pages
- **Source:** I10b-059, I10b-066; F27 only.
- **Kind:** K1, K8
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** H tools\ch_probe\constants.py:38-156,326-342 ("This is the ONE place the numbers live"); constants.py:3 names SPEC.md; ch_probe\README.md:17,61; SPEC.md; docs\clone-hero pages and code\6142 files; docs\clone-hero\code\facts-6142.tsv:955-967.
- **Splits:** None by value.
- **Ruling quotes:** None found.
- **Tests:** H tests\test_docs_match_code.cpp:996-1010 holds listed names (`kDataAddressNames`) to the 6142 table; coverage of every constant unchecked.
- **Open:** not reached.
- **Hook gap:** no

### L-F27-28: Scan row kind letters (code) vs kind letters A-H (doc)
- **Source:** I10b-090; F27 only.
- **Kind:** K1, K3
- **Verdict:** NOT VERIFIED
- **Homes (finder only):** H tools\precheck\check_rows.cpp:242-249 `row_kind`; check_rows.h:26-29; DOCS\derive-once-review.md:28-38 ("The precheck's lines cover kinds B, C and E").
- **Splits (finder only, "not graded"):** the code's letters classify where a scan hit sits; the doc's letters classify the finding.
- **Ruling quotes:** None found.
- **Tests:** tests\test_precheck*.cpp not read.
- **Open:** not reached.
- **Hook gap:** no

### L-F27-29: compare_db fixture layout vs table owner
- **Source:** I10b-115; F27 only. Same question as L-F25-06.
- **Kind:** K1, K8
- **Verdict:** NOT VERIFIED (the F25 side is in L-F25-06)
- **Homes (finder only):** H tools\test_compare_db.py:19-154; H tools\compare_db.py:96-146; H src\store\record_store.cpp:272-304; src\store\library_file.cpp:86-94 reads `results`. No src owner found for `results`, `paths`, `path_refs` (searched CREATE TABLE and path_refs in src).
- **Splits:** None
- **Ruling quotes:** None found.
- **Tests:** none compares compare_db.py to record_store.cpp.
- **Open:** owner of three tables not found.
- **Hook gap:** no

# Single-home or thin rows (no ledger row)
Finders reported one home and no second answer for these: F26 I8-020, I8-068, I8-073, I8-128, I8-130, I8-214, I8-222, I8-231, I8-168, I8-149/150; F27 I9-097, I9-154, I9-163, I10b-041, I10b-053, I10b-080, I10b-101, I10b-108. No verifier reached them.

# End lists
Candidates the verifier REFUTED (whole): L-F26-07 (as a conflict; the OS title question stays open). Parts refuted: L-F27-08 (the claim that IN-76 lacks the fact); L-F25-08 (the "split").
Verifier changed but kept: L-F25-06, L-F25-08, L-F26-02, L-F26-05, L-F26-08, L-F27-08.
Rows no verifier reached (NOT VERIFIED): L-F25-04, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 (19-24 are F25b; V25b verified nothing); L-F26-06, 12, 13, 14, 15, 16, 17, 18, 19, 20; L-F27-09 through L-F27-29.
