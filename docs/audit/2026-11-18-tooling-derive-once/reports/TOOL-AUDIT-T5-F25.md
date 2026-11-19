# TOOL-AUDIT-T5-F25 (finder F25, session f6a384b5-9eed-4607-95cb-681ec635d569)

Rows are grouped by question. Paths: H = C:\Users\Patrick\Downloads\Hydra\hydra-test, G = C:\Users\Patrick\.claude\gates, C = C:\Users\Patrick\.claude.
Searched for every group: H docs\agents, H CLAUDE.md, H .claude\commands, H tools, H tests\test_docs_match_code.cpp, G *.py, README, tests, C agents\*.md, C CLAUDE.md, C settings.json, memory notes, session_cards. Skills folder holds only synced vendor skills; nothing there.
No `## Confirmed` line was found that speaks to any group below. I grepped the rulings topic files agents-and-workflow, review-and-hooks, docs-and-writing, tests-and-runs for handoff/premise/owner map/screenshot/release/label/source words. That grep was partial: several long lines were omitted from its output and I did not read the required plan line 108.

## G1. What counts as a source (I6-175, I7-167) and the ruling kind (premise map)
Homes:
- H docs\agents\premise-map.md:52 "A premise about the world ... is sourced by the thing itself. A file that only reports on it does not count. That means a handoff, another plan, a session card, a memory note, or a Clone Hero block with no proof label."
- H docs\agents\premise-map.md:42 (data kind: block labelled "Hydra assumes it, never checked" or "Unknown" is not a source).
- H docs\agents\agent-rules.md:41 and rule 15 text in H CLAUDE.md: "A rule copied from a tool, test or comment is never a source." (a second statement of the same idea, a different list).
- H docs\agents\agent-rules.md:39 "A reading, a memory note, a handoff or a plan's 'as ruled' never counts as the user's words."
- C agents\planner.md:13 and C agents\reviewer.md:13 restate the unchecked-row rule ("A row with no code, data, chart, run or ruling source, as that page defines them, is unchecked") and point at premise-map.md.
- H tests\test_docs_match_code.cpp (kKindWords at 1530; status rules ~1748-1790) reads the shape, not what counts as a source.
Edge table (what each home tells its reader):
- At the limit: a session card is a source for a `ruling` row only (premise-map:52), but for nothing else. agent-rules:41 allows a decision entry to count "when it quotes the user's words as found under ## Confirmed or in a session card".
- Main session vs agent: no home distinguishes them.
- Agent skipping the preamble (card writers): not addressed; card writer brief C tools\session_cards\writer-brief.md:67 says a card holds "the user's exact words ... not filed rulings".
- Worktree vs main: not addressed.
Kinds: K1 (two lists of what is not a source), K3 candidate (see split S1).
Split S1: H docs\agents\premise-map.md:45 says a `ruling` source may be "in a session card's quoted words under docs/sessions/, or in the user's message as the main session quotes it in the reviewer's brief and confirms". H CLAUDE.md "Rulings and this machine" and H docs\agents\agent-rules.md:35 say only a `## Confirmed` line "may be cited as the user's ruling". Different answers to "what may a plan cite as the user's ruling". (agent-rules.md:41 and the brief's own list also let a card quote count, so agent-rules is itself on both sides.)
Test holding homes equal: none found. G tests\test_agent_files.py holds planner/reviewer files to phrases ("## Premise map", "Calls:", "never tried", "unchecked", the page path) only.

## G2. Premise map shape and live-step call list (I6-176, I6-177, I6-178, I7-163, I7-164, I7-165, I7-166, I7-168)
Homes:
- H docs\agents\premise-map.md:5 "This page is the one home of the map's shape, and of the shape of a live step's call list." Lines 13-35, 41-46, 56-68.
- H tests\test_docs_match_code.cpp: is_live_step_id 1625-1628 (starts "LS", third char '-' or digit), kKindWords 1530 (the six words, a second list), kPremiseMapHeading 1527, kLiveStepsHeading 1529, kWaitsFor 1526, never-tried count check ~1851 ("split it so each step tries one new piece"), kPremiseMapSince "2026-11-01" at 1520, kDeferredPremiseSince "2026-11-13" at 1525.
- C agents\planner.md:13 "A step never chains two never-tried calls, so a failure names one piece; split such a step in two." and "(a heading that starts `### LS`)". States the rule while pointing at the page.
- C agents\reviewer.md:13 same "(a heading that starts `### LS`)" and "no row" checks.
- H docs\agents\clone-hero-mcp.md:58 "Before the main session asks the user for a run, its message shows that step's `Calls:` line exactly ... names each `(never tried)` call ... No hook checks this message".
- H CLAUDE.md "Every new plan carries a Premise map ... every live step lists its tool calls".
- G tests\test_agent_files.py:18-21 holds planner/reviewer files to contain "Calls:", "never tried".
Edge table:
- At the limit: docs say "at most one (never tried) call" (premise-map:68); the test counts "2 (never tried) calls" as the error (test 2003 expects "has 2 (never tried) calls"), consistent. The doc's "seen" definition (64-66) has two looser readings; the test only checks that each entry has "(seen: <run id>)" or "(never tried)" (test ~1878), so whether a "seen" is true is not checked anywhere.
- Main session vs agent: the go request (clone-hero-mcp.md:58) is the main session's; planners/reviewers hold the rest.
- Card writers: not covered.
- Worktree vs main: not covered.
Kinds: K6 (doc page and the C++ test each decide "is this a live step heading" and "which Kind words"), K1 (planner.md and reviewer.md restate the live-step heading and the never-tried rule), K7 (start dates 2026-11-01 and 2026-11-13 live only in the test; premise-map.md names no start date, only "the comment above it says which plans count" at 76).
Split: S2 — premise-map.md:56 says a step is "### LS followed by - or a digit ... inside a section whose heading starts `## The live steps`"; planner.md:13 and reviewer.md:13 say "a heading that starts `### LS`" with no dash/digit and no section condition. A heading such as `### LSX` counts by the agent files and not by the page or test.
Test holding homes equal: test_docs_match_code.cpp holds plans to the page's shape (one direction). No test holds the page's Kind list equal to kKindWords.

## G3. Release naming and signature file (I6-188) and build number (I6-194)
Homes (release):
- H docs\agents\this-machine.md:17 "A release's tag and title are both `v` plus the version, such as `v3.0.0`, and it carries two files: the installer and its `.sig` signature. ... project line of CMakeLists.txt, which reads VERSION 3.1.2 today."
- H docs\development.md:1011-1017 command lines use `-Target v<version> -Title v<version>` and `<setup.exe>.sig`; 928-933 says the app refuses "exactly one .sig asset ... a tag that parses".
- H src\app\update_release.cpp:17-18 kInstallerSuffix "-setup.exe", kSignatureSuffix "-setup.exe.sig" (code reads asset names); H installer\build_installer.ps1 and tools\release_sign.ps1 write "<installer>.sig".
- G publish.ps1 takes -Target and -Title as given (no rule about `v`+version).
- H CMakeLists.txt:21 VERSION 3.1.2 (the value this-machine.md copies as "3.1.2 today").
Edge: nothing says what an agent does if the tag differs from `v`+version; publish.ps1 does not check it.
Kinds: K1 (the "v"+version rule is in prose in two places and in no code; update_release.cpp only checks that the tag parses). K1/K2-risk: "reads VERSION 3.1.2 today" is a copied value that goes stale when the version bumps.
Split: none (value-level agreement today). Test holding equal: none found.
Build number (I6-194): H docs\agents\this-machine.md:27 "Clone Hero build 6142"; same value in H docs\clone-hero\README.md:4 (v1.1.0.6142), H tools\ch_probe\constants.py:22, H tools\ch_probe\SPEC.md:76, file names facts-6142.tsv / code\6142\. Docs test (README 88) holds constants.py addresses to facts-6142.tsv rows, not the build number. K1. Searched only light; not graded.

## G4. Database tables and the join key (I6-208, I6-209, I7-159)
Homes:
- H docs\agents\this-machine.md:85 "The tables today are charts, meta, profiles, records and record_paths ... Their columns are defined once, in src/store/record_store.cpp ... a result row's song_id is the chart's md5".
- H src\store\record_store.cpp:272,281,290 (profiles, records, record_paths) and 853,866 (charts, meta): the real owner of the CREATE TABLE text.
- H tools\compare_db.py:130-146 TABLES = results, paths, path_refs, songmeta, dynamics, charts, meta, profiles, records, record_paths; SUMMARY_TABLES, OWN_IDS, REFERENCES: a hand list of tables and key relations (columns come from PRAGMA).
- H tools\test_compare_db.py:141 hand-written CREATE TABLE record_paths copy for its fixture.
- H docs\development.md:272 (row counts for results, paths, path_refs, songmeta, dynamics) and :500 "tables (profiles, records and record_paths, ADR 0031)".
- H docs\adr\0028 named as the home of song_id == md5.
Edge: this-machine.md says "Read the columns there" (pointer, good). Its list of 5 tables disagrees with compare_db.py's list of 10 (results, paths, path_refs, songmeta, dynamics are not in the 5).
Kinds: K1 (table names), K6 candidate (compare_db.py decides keys and references by hand lists; record_store.cpp owns them), K8 candidate: "defined once" is true for record tables; the tables results/paths/path_refs/songmeta/dynamics are not in record_store.cpp (my grep of src\store found only the five CREATE TABLEs there), so the sentence "The tables today are ..." is a partial list.
Split S3: this-machine.md:85 lists 5 tables as "the tables today"; compare_db.py lists 10 as the database's tables.
Test holding equal: none found (tests mention record_paths in test_store_*.cpp only, none reads compare_db.py's lists).

## G5. Rulings: where filed, headings, shape, removal (I6-214, I6-216, I6-218, I6-220, I6-223)
Homes:
- H docs\agents\rulings\README.md:3,18-20,26-32,40-45,51,55-57 (the page).
- G ruling_rules.py:1-30 and 94-110: "LINE, SESSION_NOTE and parse() are the one home of that shape"; CONFIRMED/UNCONFIRMED headings, INSTALL_DATE = "2026-11-14", APPROVE "File it", DECLINE "Do not file", QUESTION "File this ruling in {path}?", SESSION_NOTE regex (8 hex or full uuid).
- H docs\agents\agent-rules.md:33-37, H CLAUDE.md "Rulings and this machine", G README.md:59, C tools\session_cards\common.py:53 and writer-brief.md:67 (WORDS_HEADING and the "not filed rulings" line), H docs\agents\premise-map.md:45, H handoff.md:13.
- H docs\agents\rulings\README.md:51 repeats "INSTALL_DATE in ruling_rules.py, 2026-11-14" and the user's "Yes, no click" date 2026-11-13; the code comment at ruling_rules.py:97-98 repeats both.
Edge table:
- At the limit: README:51 backfill needs date "before the day the gate was installed"; code `f["date"] < INSTALL_DATE` agrees. README:45 "Removing a line is always allowed"; docstring ruling_rules.py:10 "Removed lines are never judged" agrees; README:51 says moving/deleting the Unconfirmed heading counts as filing, code docstring 24-25 agrees.
- Main session vs agent: README:47 and ruling_rules.py:27-30 agree: an agent transcript never counts.
- Card writers: the card tools own the quote test (`is_users_words` in session_cards common.py); README and agent files only point at it.
- Worktree vs main: ruling_rules judges any branch against its merge base (docstring 34-38); README says "every branch a Claude Code session moves"; agree.
Kinds: K1 (INSTALL_DATE, the two labels and the question text are in README and code), K5 (README steps 1-4 spell out what the gate enforces), K3 candidate on S1 above.
Split: none among these homes. Tests holding homes equal: G tests\test_ruling_rules.py:25 reads README by path for the "format page" rule only; G tests\test_docs_numbers.py:54-56 holds the gates README to name ruling_rules.py/rulings.jsonl and "Hydra's rulings format page must hold the approval step". So a test does hold README to contain the approval step (not the labels).

## G6. Issue tracker, wayfinder, triage labels (I6-230, I6-232, I6-234, I6-235, I6-236, I7-213, I7-214, I7-215)
Homes:
- H docs\agents\issue-tracker.md:28 number space, 40-47 wayfinder conventions ("`gh api` is denied", `Blocked by:`, `wayfinder:map`, `wayfinder:<type>`, `gh issue edit <n> --add-assignee @me`).
- H docs\agents\triage-labels.md:7-13 five labels, each "equal to its name", plus their meanings; H CLAUDE.md "Triage labels: ... each label string equal to its name".
- C settings.json:25,39,53 and G settings.fragment.json:15,29,43: `gh api *` is in the deny list in every spelling, so the "denied" claim has a code home; G README.md:73 says "every gh api call is denied outright".
- H docs\agents\issue-tracker.md:5 and G publish.ps1 / settings deny rules for posting (gh issue create/comment denied; issue edit and close are not denied).
Edge: issue-tracker.md:46 claim step `gh issue edit <n> --add-assignee @me` is allowed by settings (only create, comment, pr create/comment, release create/edit, api are denied). The wayfinder "frontier query" is described in prose only; the /wayfinder skill is not in the scanned scope (not found in C skills or H .claude\commands).
Kinds: K1 (the five label strings appear in triage-labels.md left and right columns, same strings; CLAUDE.md states the rule), K7 candidate (labels exist on GitHub; nothing in scope checks that the repo's labels match; I did not call gh). The `/wayfinder` and `/triage` skills named as readers were not found in scope, so "who acts" is unverified.
Split: none. Test: none found.

## G7. Doc screenshots (I6-245, I6-247, I6-248, I6-252)
Homes:
- H docs\agents\doc-screenshots.md:11-29 and H tools\doc_shots\shots.txt:1-20 (file header repeats "one list of those pictures" and the crop rule); H tools\doc_shots.ps1:3,28; H docs\development.md:979-983 points at the page.
- Size limits (not my rows but the same page, noticed): H tools\doc_shots\shrink.py:20-21 (500 KB file, 4 MB folder) and H tests\test_docs_match_code.cpp:2024-2025 (kDocImageMaxBytes, kDocImagesMaxTotalBytes), both from decision D300 (decision log line 1666). The test comment says shrink.py "fails a capture run on the same limits". Two code homes, no test holds them equal.
- Crop rule: doc-screenshots.md:15 and shots.txt:6-9 and doc_shots.ps1:28 each state "a rect line directly before a screenshot with no ref keeps the frame below that rectangle".
- Reset step: doc-screenshots.md:25 and shots.txt:12-20 both say click `Reset to defaults`, expect `Analysis settings: defaults`. The words come from H src\ui\settings_bar.cpp:226 ("Reset to defaults") and H src\ui\library_dialogs.cpp:137 ("Analysis settings: " + summary).
- "The one list": H tests\test_docs_match_code.cpp:15,2032 holds shots.txt equal to the files in docs\images and to the pages that link them (a test that holds homes equal).
Kinds: K1 (text copies of button wording in shots.txt and the page; shots.txt expect-text breaks the run if the app text changes, so it is a checked copy), K6 (size limits in shrink.py and the C++ test), K1 for the crop rule in three texts.
Split: none. Test holding equal: test_docs_match_code.cpp check 4 (shot list vs images vs pages). Not holding: size limits.
Note: I6-247 (timeout caps and `!! failed at line N`) searched only in doc-screenshots/ui-testing; not graded.

## G8. Hydra UI test page: output, hydra_run, real-app record, protocol (I7-009, I7-044..049, I7-053)
Homes:
- H docs\agents\ui-testing.md:42, 229, 233-282.
- Pass/fail form: H tests\ui\uitest_main.cpp:23,81,222-244 prints "[PASS]/[FAIL]" and "%zu of %zu passed" (code owner). Page states the same.
- "stopped at line N of M": H tools\hydra_ui_mcp.py:694,708 (owner), page line 229.
- "Hydra exited with code X", "Hydra did not answer within N s", "no real-app record written": H tools\hydra_ui_mcp.py:460,479,485,584,588,643; page line 251,282; tests in H tools\test_hydra_ui_mcp.py:578-646 pin them.
- Real-app record: file name realapp.jsonl in H tools\hydra_ui_mcp.py:81 (REALAPP_FILE) AND G review_rules.py:49 (REALAPP_FILE = "realapp.jsonl"; comment 47-48 says hydra_ui_mcp.py owns the name and fields, "this file only reads them"), page line 237, G README.md. Fields: page 241-249 (seven fields), hydra_ui_mcp.py:194-196 writes them, G review_rules.py:383-386 reads kind, dirty (must be False), commands (int >= 1), commit (in branch). G test_review_rules.py:404 builds a record with literal fields; G test_docs_numbers.py:54 holds the gates README to name "realapp.jsonl".
- Protocol: hello line and reply JSON written by H tests\ui\live_protocol.h:64-89 (owner, pinned by tests\test_live_protocol.cpp per page line 278), read by H tools\hydra_ui_mcp.py:420-425; page 265, 271.
- Open-dialog note: owner H tests\ui\uitest_engine.cpp (open_modal_note), page 259 points at it.
Edge table (real-app record):
- At the limit: page 246 `commands` "how many command lines Hydra answered"; gate requires commands >= 1; the page does not say a record with 0 commands counts for nothing (the gate's rule is not on the page).
- Main session vs agent: the record is written by the helper in whichever checkout it sits in; the gate reads the repo's common dir, so a worktree session writes to the main checkout's .git; page 237 says the same.
- Card writers: not applicable.
- Worktree vs main: page 237 and review_rules.py:376 (`--git-common-dir`) agree.
Kinds: K6/K1 (realapp.jsonl name in two code files, comment names one as owner; no test holds the two constants equal), K3 candidate (gate adds conditions the page's "contract" does not list: dirty must be false, commands >= 1, commit on the branch).
Split: none in value. Test holding homes equal: none for REALAPP_FILE across the two repos. test_live_protocol.cpp pins protocol lines (named by the page; I did not open it).

## G9. Clone Hero MCP tool contracts (I7-091, I7-092, I7-093)
Homes:
- H docs\agents\clone-hero-mcp.md:52-54; H tools\ch_mcp\server.py:413-441 (tool docstring the agent reads) and 444-460 (ch_edge_probe); the ch MCP server instructions text in the session context also describes ch_run_chart and ch_edge_probe.
- Code owners named: H tools\ch_probe\engine_finder.py:38 ENGINE_SEARCH_CAP_S = 120.0; H tools\ch_probe\experiments\overlay_logs.py:158 PRESS_LOG_COLUMNS; H tools\ch_probe\experiments\edge_probe.py (probe parsing 73-87, lead sign 22, 46, 173, 198); H tools\ch_mcp\menu_tools.py:43 STEPS_FILE; H tools\ch_mcp\kit_tools.py:92 KIT_PLAYER = "Player1".
- Probe lead sign copied in doc (53), server.py:450-451, edge_probe.py:22,46.
Edge table:
- At the limit: the doc and server say "a positive lead presses early and a negative one late"; edge_probe.py:173 and :198 print 'early' when lead_ms >= 0, so a lead of exactly 0 reads "early" (doc says nothing about zero).
- Main session vs agent: ch_edge_probe/ch_run_chart refuse while a play session runs (server docstrings); doc agrees.
Kinds: K2 candidate.
Split S4 (K2): H docs\agents\clone-hero-mcp.md:52 writes the signature `ch_run_chart(song_folder, score_json, schedule_json, kit_player="System")`, but H tools\ch_mcp\server.py:414 has `kit_player: str | None = None`, and H tools\ch_mcp\kit_tools.py:1050-1063 turns None into KIT_PLAYER "Player1". The doc shows a default of "System" that the code does not have. (server.py:421 says use "System" for a human launch; that is advice, not the default.)
Kinds also: K1 (cap name, columns, builds-<n>.json / press-<n>.csv named in doc, server docstring and game_tools.py:1151), K7 for the "Edge Walk Pair" chart path C:\Clone Hero\songs\Hydra Probe\Edge Walk Pair (doc only; H tools\ch_probe\probe_songs.py:64,68 DEFAULT_OUT and NAME_PREFIX are the nearest owner; I did not find a name check). Test holding equal: none found for the doc signature.

## G10. Test rubric (I7-106, I7-109, I7-117, I7-121, I7-122)
Homes:
- H docs\agents\test-rubric.md:29, 47-51, 86, 107, 115-152.
- H docs\superpowers\plans\2026-11-14-test-suite-audit.md:167 ("Records whose claim was confirmed change reasoned to confirmed") and :216 ("answers 1 to 11, each a sentence or two naming files, lines and functions ... the two lines Engine: and Claim:"), a plan home of the answer shape.
- H docs\agents\derive-once-review.md:30-31 defines kind A ("A rule written twice in production code") and kind B; test-rubric.md:39,107,148 uses "kind A" for "a display works out a game fact" with no second copy.
Edge: the rubric lists 12 questions, the plan says answers "1 to 11" (the plan predates or differs from question 12).
Kinds: K1 (answer shape in rubric and in the plan), K3 candidate.
Split S5 (K3): derive-once-review.md:30 kind A = "a rule written twice in production code"; test-rubric.md:39 says a display rule that only display code uses is "kind A ... against the production code" even when nothing else answers the question. Different meanings of kind A.
Test: none found.

## G11. Owner map shape (I7-171, I7-174, I7-175)
Homes:
- H docs\agents\owner-map.md:3 (one-home claim), 21-29, 37.
- C agents\scout.md:11 spells out the statuses (`carry out`, `none`), "game words, never in the shape of an answer you picture", "Never propose a new owner" and points at the page.
- C agents\planner.md:15 and C agents\reviewer.md:17 restate the planner's and reviewer's duties and the stop ("sign off BLOCKED").
- H CLAUDE.md "Doc review" paragraph: "A row with no owner, or a result that contradicts the user's words, stops the work and goes to the user" (the two stops, stated in CLAUDE.md as well).
- H docs\agents\agent-rules.md rule 15 and H CLAUDE.md rule 15 ("Call the one owner, merge several into one first, or name the search that found none").
Edge: owner-map.md:13 says a plan that "changes hooks needs none"; CLAUDE.md says an owner map is for "a plan that changes what Hydra decides or shows"; consistent. The scout.md text has no start or stop rule on `split`; the page has `split`.
Kinds: K1 and K5 (CLAUDE.md, scout.md, planner.md, reviewer.md state the statuses and stops that the page owns).
K8 candidate: the row I7-174 says a docs test reads this shape; I searched H tests (all *.cpp), H tools, G *.py and *.md for "owner map" and found none. So no test checks `## Owner map` today (searched by that phrase only).
Split: none.

## G12. Handoff shape and checker (I7-192, I7-193, I7-194, I7-195, I7-196, I7-198)
Homes:
- H docs\agents\handoff.md:11-37, 49-59.
- H tools\check_handoff.py:28 _FULL_HASH {40}, 31 CLAIMS_HEADING, 32 MANUAL_KINDS = test, ruling, state (code owner of the kinds and status words).
- H .claude\commands\handoff.md:11 "Write every hash in full and name no scratch path", 13 "## Checker output" (K5 copy of the page's rules, with the page named at 10).
- H docs\agents\brief-preamble.md:95-97 and CLAUDE.md rule 9 (handoff is another agent's summary; run the checker).
- H tests\test_docs_match_code.cpp:1233 kNotPermanentDirs = {"docs/handoffs/", "docs/superpowers/"}: the page (handoff.md:37) states the skip as "skips docs/handoffs/" while the test skips both.
Edge: At the limit: a hash must be exactly 40 chars (page 35, code 28); scrape mode treats "bare hex words" as commits (page 59), so a short hash there is judged differently from table mode (page 51 says short = missing). Main vs agent: checker run by /handoff writer and by the continuing session; agents continue per preamble 97. Card writers: n/a.
Kinds: K1 (seven kinds listed in the page and in code), K5 (/handoff.md restates two rules), K1 for the scratch-path skip.
Split: none in value. Test holding equal: none holds handoff.md's kind list to check_handoff.py; test_docs_match_code.cpp holds the skip dirs.

## Summary of candidate splits found
S1 ruling source breadth (premise-map.md:45 vs CLAUDE.md/agent-rules.md:35).
S2 live step heading definition (premise-map.md:56 and the test vs planner.md:13/reviewer.md:13).
S3 table list (this-machine.md:85 vs tools\compare_db.py:130).
S4 ch_run_chart default player (clone-hero-mcp.md:52 vs server.py:414 / kit_tools.py:92).
S5 meaning of kind A (derive-once-review.md:30 vs test-rubric.md:39).

## Rows not reached in depth
Searched lightly only (a home list but no edge table): I6-194, I6-247, I7-053, I7-109, I7-122, I7-106. Every other row id in my list is covered by a group above, but I did not open ADR 0028, test_live_protocol.cpp, the /wayfinder or /triage skills (not found in scope) or the decision log's D300 text beyond line 1666.

## Side notes
- H docs\agents\this-machine.md:17 gives "VERSION 3.1.2 today" and the example "v3.0.0" together; the CMakeLists.txt project line is the owner (CMakeLists.txt:21).
- G README.md and settings: the `gh` deny list in C settings.json:18-67 covers `gh api` but not `gh issue edit/close`; issue-tracker.md:14,46 relies on those being allowed. Not a problem by itself; noted so the closers know.
- A way past a hook is not something I looked for; none noticed.
