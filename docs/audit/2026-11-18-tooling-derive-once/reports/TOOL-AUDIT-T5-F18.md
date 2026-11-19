# TOOL-AUDIT-T5-F18 (finder F18, agent af10fce7c60a7c18c)

Scope read: the gates folder (code, README, tests), agents/, model-postures.md, memory notes, session_cards tools, Hydra CLAUDE.md, docs/agents/ (rulings, owner-map, premise-map, derive-once-review, doc-review, agent-rules, brief-preamble), tools/precheck. Short names: DOR = derive-once-review.md, DR = doc-review.md, AR = agent-rules.md, BP = brief-preamble.md, MP = model-postures.md, GR = gates/README.md, RR = review_rules.py, RU = ruling_rules.py, PP = publish.ps1, PG = publish_gate.py, AS = agent_stop.py, PC = Hydra CLAUDE.md, RM = rulings/README.md, OM = owner-map.md, PM = premise-map.md.
I did the rows in question families (one search per question, so one row can sit under several ids). I hit the 100-call mark, so "not reached" lists rows I only touched on the surface.

## Family 1. One typed phrase lets one whole-library run through (I5-044, I5-077, I5-163 library part)
Homes: AR:14 ("sending exactly `allow library run`; each such message lets one command through"; "about 115 charts"); BP:49 (same, plus "`hydra_batch` given no chart folder refuses"); PC rule 8 (short); owner code RR:50-52 (`LIBRARY_ALLOW_TEXT`, `_LIBRARY_ALLOW`, `is_library_allow`), percall.py:45-55 (the refusal text prints the phrase and says "sending exactly"), GR:37; this-machine.md:91; audit-method.md:125; memory no-full-library-runs.md:11-15 ("about 115 charts"; "No hook enforces it yet"); Hydra src/cli/batch_scope.h:10 (`kWholeLibraryFlag`) for the refuse-without-flag half; percall.py:14 (`LIBRARY_FLAG`, a second spelling of the flag).
What each tells its reader:
| Home | Phrase | Match | Who acts |
| AR:14, BP:49 | exactly `allow library run` | stated as exact | every agent and main session; user types it |
| RR:50-52 | `allow library run` | whole message, case-ignored, trailing . or ! allowed, trimmed | the hook |
| percall.py:53 | "sending exactly: allow library run" | tells the agent "exactly" | the agent reads the hook message |
| memory note | no hook | says nothing enforces it | stale |
Edges: at the limit - "allow library run!" and " ALLOW Library Run " pass (tests test_review_rules.py:839-845 prove it); docs say "exactly". Main session vs agent - the hook judges both sides the same (percall.py:4); the allow must be typed in the main session transcript. Card writers skip the preamble, so BP:49 never reaches them; the hook still refuses their shell commands. Worktree vs main checkout - the flag test is on the command text only.
Kinds: K1 (phrase and "one command per message" in AR, BP, GR, percall message). K3 minor (docs say "exactly", code accepts case and one trailing . or !). K2/K8: memory note no-full-library-runs.md:15 says "No hook enforces it yet"; a hook does (percall.py `_library_refusal`). K2 minor: "about 115 charts" in AR:14 and the memory note is a count with no check I found.
Splits: (a) AR:14/BP:49 "exactly" vs RR:52 tolerant match. (b) memory note "no hook enforces it" vs percall.py:45-55.
Confirmed ruling: review-and-hooks.md:16 "i want to be able to give explicit approval for a full library run." (session dee9c61e); tests-and-runs.md:14 same words.
Tests holding homes equal: test_review_rules.py:839-845, test_progress.py:427-455, test_review_dispatch.py:309-336 test the code's match; nothing compares the docs' words with the code. test_docs_numbers.py does not cover it.

## Family 2. One review, then done; what goes back; what holds the merge (I5-041, I5-108, I5-089, I6-006 context, I5-131 budget part)
Homes: AR:11; PC rule 5 (short); DOR:90 ("nothing goes back to the user"), DOR:106 and :108 ("Holds the merge" exceptions), DOR:110, DOR:122-123, DOR:173-183 (Fix review), DOR:202; DR:98-100 ("does not hold the publish"); MP:5 ("Nothing goes back to the executor and no second round (D107)"); BP:89 ("Nothing comes back to you"); reviewer.md:11,19,31; memory one-reviewer-rule-d107.md:11,13,15; GR/AS code comment AS:21-22.
Differences:
| Home | Notes go to user | Does a Note hold the merge? | Who is read again after |
| AR:11 | yes, "the merge does not wait for them" | no | fix that fails build -> fresh executor + fresh reviewer (D314) |
| DOR:106 | yes | no, except three cases (not-asked visible change, unfixed kind H, unfixed kind D), prefixed "Holds the merge:"; sign-off stays CLEAN | same D314 |
| DOR:149, reviewer.md:31 | - | BLOCKED "when something you could not fix must hold the merge" | - |
| DR:98 | yes | "does not hold the publish" (no exception listed) | - |
| memory one-reviewer-rule-d107.md:11 | yes | "don't hold the merge" (no exceptions); "nothing goes to a fresh agent" | none (no D314) |
| MP:5 | - | - | "Nothing goes back to the executor" (AR/DOR say "to the user") |
Edges: out-of-time reviewer: AR:11, DOR:110, BP:73, reviewer.md:11, memory all agree it signs off what it fixed and lists the rest. Main session vs agent: the main session passes Notes on. Card writers: not reviewers. Worktree vs main: DOR:17 says a staged change on main stops the reviewer; no hook checks that.
Kinds: K1 (many homes), K2 (memory note: "don't hold the merge", no D314, no 2026-11-12 exceptions), K3 (DOR:90 and AR:11 "nothing goes back to the user" while DOR:106 sends questions to the user and holds the merge; MP:5 says "to the executor"), K6-like split inside DOR (below).
Splits: (a) DOR:106 says a reviewer with an unfixable kind H or D finding "still signs off CLEAN"; DOR:149 and reviewer.md:31 say write BLOCKED when something unfixed must hold the merge; AS:105-109 only reads the line, so the merge hold lives only in the Note prefix, which no hook reads. (b) memory note vs DOR:106. (c) DR:98 (publish never waits) vs DOR:106 (merge waits in three cases) when one reviewer reviews both.
Confirmed rulings: review-and-hooks.md:20 "i asked for the review agent to fix its findings directly, then we're done."; :21 "notes are fine, and yes do that for running out"; :9 "why are there findings a reviewer cant fix? give an example of one" (a question, not a rule). DOR:106 cites "alright approve b" (2026-11-12), which I did not find under Confirmed in review-and-hooks.md or agents-and-workflow.md (only the 2026-11-12 line at :8-9 appear). Not checked in session cards.
Tests: none holds the docs equal. AS tests cover the verdict word only.

## Family 3. The sign-off line and what the stop hook records (I5-138, I5-139, I5-166, I5-167, I5-168, I5-165)
Homes: owner code AS:21-23, :105-109 (`_BLOCKED`, `_CLEAN`; no line -> CLEAN with a notice), AS:132-158 `signed_tip`, AS:163 `DOC_SUFFIXES = (".md", ".html", ".htm")`, AS:64 `open_background_tasks`. Prose: reviewer.md:25-33; DOR:143-151; DR:117-125 (the two sign-off lines and the CLEAN-by-default sentence); GR:41-49; AR:11; the user's ruling at review-and-hooks.md:19.
Drift: reviewer.md:33 and DOR:151 say the hook fingerprints "every markdown file the range changes"; DR:29 says the documents are "every markdown file the range changes". The code takes .md, .html and .htm, and GR:49 says "a markdown file or an HTML page". Kind K2 (three docs vs code and GR). Also AS:22 cites "Hydra CLAUDE.md rule 5" for "a finished reviewer has still signed off what it fixed"; rule 5 says that only for a reviewer out of budget, not for a missing line (the missing-line rule is the 2026-10-25 ruling). K8 minor.
Edges: BLOCKED only on a whole line exactly `SIGN-OFF: BLOCKED` (all docs agree). Agent type that skips the preamble: a reviewer from a Workflow gets no record (MP:5, DOR:200, GR:39). Worktree: reviewer.md:19 and DOR:112 say fixes go on the branch.
Confirmed ruling: review-and-hooks.md:19 "Build option A as planned, with Q1 as recommended: a hand-back with no sign-off line records CLEAN and tells the main session."
Tests: test_agent_stop.py exercises the verdict word and the problem notice; test_docs_numbers.py `test_review_docs_name_no_typed_key` scans the review docs for typed-key words only. No test compares the docs' "markdown" with DOC_SUFFIXES.

## Family 4. The review key and the reviewer dispatch (I5-106, I5-164, I6-007, I6-008, I6-010)
Homes: review_dispatch.py:17-19 `_KEY` (40 hex alone; a longer run is ignored) and :20-21 the refusal message; RR:56 `_HEX` (40 or 64 hex); RR:26-28 waiver phrases accept 7-40 hex (derive-once, real-app) and 7-64 (doc-review); GR:39; DOR:17, :189, :200-202; DR:29; MP:5 ("the key in the brief"); reviewer.md:9; writer-brief.md:79 (a different 40-char use); Hydra docs/agents/handoff.md:19,35,51 and .claude/commands/handoff.md:15.
Differences: docs say "40-character hash" everywhere; code takes 40 for dispatch, 40 or 64 for verdict keys, and 7 to 40/64 for typed waivers. DOR:17 says the dispatch hook refuses "more than one commit"; GR:39 same. MP:5 says "the key in the brief" and does not give the length. Kinds: K1 (length), K3 minor (a "key" is 40 hex for dispatch, up to 64 for a doc waiver).
Split: none that changes an answer for a dispatch. Note for the waiver: DOR:189 says "type `waive derive-once <key>`"; RR:26 accepts short hex, docs do not say so.
DOR:17 "Until your review is written down ... you are read-only" and DOR:7 "You fix your own findings": one page, consistent. DOR:17 says stop on "a brief that tells you not to commit"; no hook checks the brief (K5-neutral; no hook claim made). reviewer.md:9 vs DOR:7: both point at the preamble.
Tests: test_review_dispatch.py covers the 40-hex rule; nothing ties the docs.

## Family 5. Which code and doc paths a move of main needs review for (I3-101 and the I5-025/I5-026 pointers)
Homes: owner code RR:18-24 (`CODE_MARKER`, `BUILTIN_CODE`, `PUBLIC_DOCS`, `_is_code` at :180-185); ~/.claude/.derive-once-gate (gate: gates/, agents/, tools/session_cards/); GR:138 (restates those three folders); the marker text itself; Hydra .derive-once-gate and .doc-review-gate; DR:17 (names the four public pages and the four steps); PC:53 (short); DOR:189.
Wrong owner claims (K8): Hydra `.derive-once-gate` says "Which paths count is decided by Test-DeriveOnceGatedPath in the user-scope hook (hooks\lib\derive_once_rules.ps1), not by this file." Hydra `.doc-review-gate` says "decided by the user-scope hooks (hooks\lib\doc_review_rules.ps1)". The live owner is RR:22 `BUILTIN_CODE` and RR:24 `PUBLIC_DOCS`; the .ps1 names are retired (test_docs_numbers.py RETIRED list does not check these two marker files).
Copies (K1): GR:138 repeats the ~/.claude marker's three gate: lines (owner is the marker file, read by RR:160-177). DR:17 repeats the four public page names that RR:24 owns. BUILTIN_CODE (src, tests, tools, third_party, installer, .github, CMake files) is stated nowhere else I found.
Edge: a hand edit of the marker file counts as code (RR:19, `_CODE_MARKER_PATH`); stated in DR:17 for the doc marker ("every change to it is reviewed").
Tests: none compares DR:17's four pages with PUBLIC_DOCS.

## Family 6. How text reaches GitHub and the short-text limit (I5-027, I5-026, I5-173, I5-174, I3-089)
Homes: PP:1-6,8 (ValidateSet of six kinds; "the small-text rule and the record check live there"); PG:5 ("so the small-text rule has this file as its one home") and :13-14 (`SMALL_LINES, SMALL_CHARS = 20, 2000`); GR:63 and :65; DR:17,19,21; PC:53; BP none.
Copies (K1): the six kinds list is written in PP:1, PP:8, GR:65 and DR:19. The limit values 20 and 2000 are stated only in PG:14 (DR:17 and GR:63 point at it with no value: pointers). Name drift: "small-text" (PP, PG, RR:67) vs "short-text" (GR:63) vs "A short text" (DR:17), one question, three names.
Edges: exactly at the limit: PG:31-33 counts UTF-16 units over 2000 and non-blank lines over 20 (both must hold). Main session vs agent: publish.ps1 is for both; DR:19 says a hook refuses hand-typed `gh` posts. Card writers: no GitHub posting.
Confirmed ruling: review-and-hooks.md:27 "every time claude publishes a document, it has to go through a review from a new agent with fresh context specifically briefed on these issues." (does not speak to the size limit).
Tests: none holds PP's kinds equal to GR/DR; test_status_append.ps1 is unrelated.

## Family 7. A ruling is filed only after the user clicks "File it" (I5-055, I5-172, I3-083, I3-084)
Homes: RU:100-101 (`APPROVE, DECLINE = "File it", "Do not file"`, `QUESTION = "File this ruling in {path}?"`), RU:456-457 (the by-hand check prints the question and both labels from those constants); RM:3, :41-43 (the question text and both labels written out), :40 (the by-hand command), :51 (backfill route: "INSTALL_DATE in ruling_rules.py, 2026-11-14"); AR:33; PC:41 (short); GR:57-61; RU:98-99 comment (the "Yes, no click" answer, date, session id).
Copies (K1): the question text and both labels in RM:41 duplicate RU:100-101; the install date 2026-11-14 in RM:51 duplicates RU:99. "One box may hold up to four lines" (RM:41) is a box-tool limit stated only there. RU:394 comment "a crash refuses, like every refusing gate" is a claim about other gates: common.py:53-70 owns the mode (`closed` denies, `loud` only prints); percall, delete_rule, publish_gate and review_dispatch run closed; waiver, posture, prompt, shell_output, agent_stop run loud (they do not refuse). The claim holds for the refusing gates; it is a claim, not a check. I3-083 and RJ:15 (missing review_rules.py or ruling_rules.py refuses) state the same stance.
Edges: a ruling "counts only once it is committed" (AR:33, RM:45) - no code. "Removing a line is always allowed" (RM:45) matches RU:10 ("Removed lines are never judged"). Main session vs agent: RM:43,47 and RU:27-30 say an agent's transcript never counts; both agree.
Kinds: K1 (question and labels, date). No split found.
Confirmed rulings: none quoted for "File it" itself in the files I read; the rule rests on AR:33's quote, not under Confirmed in these topic files as far as I looked (not exhaustively searched).
Tests: test_docs_numbers.py (:129-136) checks RM holds the approval heading and drops the old words; no test compares RM:41's question text with RU:101.

## Family 8. What counts as the user's words (I5-057, I5-059, I5-094)
Homes: AR:35-41; RM:18-22,34; DOR:45 and :52; DR:63; PM:45; BP:99; MEMORY.md header (context); writer-brief.md:67; planner.md:19 (pointer); reviewer.md:17.
Differences:
| Home | Counts as the user's words | Also counts |
| AR:41 | `## Confirmed` line | a decision entry only if it quotes Confirmed or a card |
| DOR:45 (step 4) | same condition as AR:41 | - |
| DOR:52 (step 9) | "a D-record, a plan's User decisions, a Confirmed line, a session card, or the words your brief quotes" - no quote condition | brief quote |
| PM:45 | Confirmed line; session card quoted words; "the user's message as the main session quotes it in the reviewer's brief and confirms" | brief quote with "confirms" |
| DR:63 | Confirmed, session card, "the brief's quote" | - |
| AR:37-39 | Unconfirmed, reading, memory note, handoff, "as ruled" never count | - |
Kinds: K1; K3: DOR:52 lets a D-record or a plan's "User decisions" stand as the place for step 9, while AR:41 and DOR:45 require such an entry to quote the Confirmed/card words; PM:45 adds a "confirms" clause the others lack. Split lines: (a) DOR:45 vs DOR:52 inside one file. (b) brief-quote rule worded three ways (DOR:52, PM:45, DR:63; AR does not list it).
Confirmed rulings: none found that says which places count; the rule is stated by AR (not a ruling line).
Tests: none.

## Family 9. Tests a merge runs (I5-035, I5-036, I5-113, I5-114, I5-190)
Homes: Hydra owner code tools/precheck/test_pick.h:90-95 ("The one place shared code is decided", `shared_code`) and test_pick.cpp:789-791 (three `always:` lines); AR:7 (writes the three commands; says the tool decides shared code); MP:11 (hooks list); GR:156 (hooks list); memory merge-tests-touch-changed-files.md:11-15; PC rule 1 pointer; docs/development.md:542 (not read in full). For hooks there is no code owner: `common.py`, `lib\`, `settings.fragment.json`, `tests\lib\` is written in AR:7, MP:11 and GR:156, with nothing that checks or applies it (K7), and no home names itself the owner. Three homes, same list today (K1).
Edges: a merge docs-only to Hydra main still runs the three always tests (AR:7; ruling at tests-and-runs.md:15, Confirmed). Hooks: "whole suite only when shared code" rests on tests-and-runs.md:34 which is Unconfirmed; the Confirmed line at :17 ("why do we need to run the full suite every time a small change is made? ...") does not name the list. AR:7 cites "the user's ruling, 2026-10-14" for it.
Kinds: K1 (Hydra always-three copied from test_pick.cpp into AR:7: the tool prints them and AR:7 also types them), K7 (hook shared list, three homes, no check), K1 for MP:11 vs GR:156.
Confirmed ruling: tests-and-runs.md:15 "Fix the doc line, and also add the repo-wide tests (docs-vs-code, single-owner scan, precheck self-test) to every merge run."; :17 "all the parts you didnt change will remain the same" (inside the "why do we need to run the full suite every time ..." line).
Tests: tests/test_precheck* (Hydra) cover test_pick; none compares AR:7's commands with test_pick.cpp:789-791 (docs-match-code test not checked by me).

## Family 10. Reviewer and planner checks of owner map, premise map and invented rules (I5-132, I5-133, I5-134, I6-006, I6-011, I6-014, I6-013/015/016/024/026 DOR side)
Homes: OM:3 and PM:5 each claim to be "the one home ... other pages point here and do not copy"; AR:23-25 (rule 15 paragraph; restates the owner-map stops and the brief rule); reviewer.md:13,15,17 (restates what the reviewer checks: try each unchecked row, open each checked row's source, every `### LS` step has `Calls:`, owner map: no row/combined owners/added condition is a finding, contradictions -> BLOCKED); planner.md:15-19 (restates "use each owner as it is, never combine owners ...", "an unchecked row a read can settle is settled"); DR:63 (plan-side checks); OM:43-47, PM:74.
Copies (K1): the reviewer's owner-map checks are in OM:47, reviewer.md:17; the planner's map rules in OM:43 and planner.md:19; the invented-rule paragraph in AR:23, reviewer.md:15, planner.md:17; premise-map reviewer checks in PM:74 and reviewer.md:13 (reviewer.md also adds the `Calls:` check and the "no unchecked premise used as settled" check, which PM:74 states differently: PM:74 lists three passes; reviewer.md:13 lists those plus Calls). K8: OM:3 and PM:5 claim sole home; AR:25, reviewer.md and planner.md restate the rules in their own words.
K3: OM:39 and OM:47 say a plan reviewer "signs off BLOCKED" on a contradiction; DOR:106 says a code reviewer with a held finding signs CLEAN. Different reviewer jobs, but one reviewer who covers both reads two instructions for "must hold the merge".
DOR-side rows: I6-006 (the one rule) restated at AR:15, PC rule 15, DOR:13, BP:117, reviewer.md:9. I6-011/I6-014 are pointers that name an owner with no value: items.h:50 `kLargeRangeLines = 400` (DOR:21 and :193 say "the precheck owns that threshold ... this page never restates them" - true); clones.cpp:90 `kCloneWindowLines = 8` (DOR:38 names it, states no value; ADR 0025:38 states "eight"). DOR:21 says precheck "finds kinds B, C, E and F mechanically"; DOR:38 says its lines cover "B, C and E ... and kind F's candidates" (K2 minor, same page; code emits E (check_4), F (check_5), D (check_3) per tools/precheck, and I did not find the B and C emitters by name).
Confirmed ruling: none quoted for these pages in my files; OM:7 quotes the user ("Yes, write them"; AR:25 same) but these are not under Confirmed that I opened (code-and-engine.md not read).
Tests: tests/test_docs_match_code.cpp checks the plan shapes (PM:76); nothing checks reviewer.md or planner.md against OM/PM.

## Family 11. Early start of a dependent task and what a review counts as (I5-109, I5-106)
Homes: MP:7 (earlyStart rule); AS:135-136 (comment: "early starts of dependent tasks" cut at the handed tip), so the hook code knows early starts exist; Hydra plan docs/superpowers/plans/2026-11-08-player-audit-fixes.md:140 uses `earlyStart: F1` in a heading. I found no schema, script or checker for `earlyStart` in the scope (searched ~/.claude agents, gates, skills, tools, memory; Hydra tools, docs/agents, CLAUDE.md, .claude/commands). One prose home plus one plan using the word: K7 (a rule nothing checks).
I5-106: "A review counts only when it goes through the `Agent` tool with `subagent_type: reviewer`" - homes MP:5, DOR:200, GR:39 (the hook records only those); owner code review_dispatch.py REVIEWER_TYPE (:16) and agent_stop.py REVIEWER_TYPE (:20), two constants with the same value (K6-like, copy of one string). Tests cover each.

## Family 12. Who reads the cards (I4-002) and the card rows in my list
Homes: writer-brief.md:9-10 "Cards are published with the Hydra repo ... so anyone can read them"; writer-brief.md:18 ("a session not about the project is kept locally ... `local: yes`"); check.py:10,216-226,245-259,281 (code that keeps local cards out of the published folder); common.py:15,37,267-293; PC:25 ("Every past session of this project has a short card ... live in the repo at docs/sessions/"). Not tested by me whether the repo is public.
Kinds: K2 minor: PC:25 says every session has a card in the repo; writer-brief.md:18 and check.py say a local-only session's card is never in the repo. K1: the public/local rule is in writer-brief and enforced by check.py. Edge (card writers skip the preamble): writer-brief.md is their only brief, so rules in BP never reach them.
Tests: tools/session_cards/test_session_cards.py (not opened).

## Pointer rows with nothing more found (I5-019, I5-020, I5-024, I5-025)
I5-019: PC:33 points at audit-method.md; no second statement of the four exits found in my files beyond audit-method itself (not read in full, so not proven single). I5-020: PC:37 points at ui-testing.md:5-13 (three cases); GR:55 states the real-app record rule for src/ui and ui-testing.md:214-251 repeats it (not compared line by line). I5-024: PC:45 points at public-doc-style.md (not opened; wording copied "second person, no first person ..." in PC only). I5-025: DOR:189-197, PC:49, DOR:17, MP:5 all say a code change goes on its own branch in its own worktree; the ref gate refuses a main move with no signed tip, nothing checks "staged on main" or "worktree" before then (K5/K7 for the worktree part; README:136 repeats it as advice).

## Rows only touched, not searched in full (not reached)
I5-135, I5-137 (reviewer: same-branch fix, cross-repo commit; one pass: GR:39 `judged_folder` and DOR:200 state the cross-repo rule; reviewer.md:9,23 restate it), I5-170, I5-171 (the "owns" claims in GR:53-55: I did not open `missing_trailers`, `_code_cause`, `judge_push_lines`, `restore_checkout`, `change_is_comments_only`, `_c_code`, `realapp_checked`, `_carries`, `tools/hydra_ui_mcp.py` to check each claim), I5-191 (GR:160 rollback and ff-only: no second home searched), I6-013, I6-015, I6-016, I6-024, I6-026 (DOR only; kinds list checked against precheck letters D, E, F only; steps 1-10 and the Pitch Dark example not compared to any other page), I5-131 (budget numbers: DOR:110, BP:73, AR:7, reviewer.md:11 all use 100 calls/20 minutes; progress.py:14-15 owns them; test_docs_numbers.py:110-127 checks the Hydra docs but not reviewer.md, which says "100 calls or 20 minutes" in words the test's regex `tool calls?` would not match).

## Side notes
- ~/.claude/gates/percall.py:14 `LIBRARY_FLAG = "--whole-library"` and Hydra src/cli/batch_scope.h:10 `kWholeLibraryFlag` are two spellings of one flag in two languages.
- ~/.claude/gates/publish.ps1:16 and install_ref_gate.ps1 hard-code the Python path (I3-100 already lists this).
- A gap in the hooks: the "Holds the merge:" Note prefix (DOR:106) holds a merge only by the main session's reading; no hook reads it.
- Hydra .derive-once-gate and .doc-review-gate name retired .ps1 hook files (Family 5).

## Summary line counts
Rows done in some depth: about 50 of 58. Rows not reached in depth: I5-135, I5-137, I5-170, I5-171, I5-191, I6-013, I6-015, I6-016, I6-024, I6-026 (and I5-131 only for numbers).
