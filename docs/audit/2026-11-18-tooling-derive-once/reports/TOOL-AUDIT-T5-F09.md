# TOOL-AUDIT-T5-F09 (finder F09, agent a2ad51c567921eb05)

Rows are grouped by question. Row ids are in brackets. All greps covered the brief's scope (gates incl. tests, tools\session_cards, agents, model-postures, both CLAUDE.md, skills, settings.json, Hydra docs\agents and rulings, handoff command, Hydra tools, memory notes not archive). Note: an early memory-notes grep used a glob that matched nothing, so I re-ran the memory greps by direct path; memory coverage is for status, worktrees, CH facts, repo name, ytdl, shared-code, plain English only.

## Q1. Where does an agent's status file live, and in what shape is the line? [I5-063, I6-005, I5-178 in part]
Homes:
- gates\common.py:10,123-124 `STATE_ROOT = <gates>\state`, `status_path()` = `<root>\status\<id>.md` (code owner; progress.py:90,150 and stall.py:119,140,249 call it).
- gates\status_append.ps1:11-14 `$StateRoot = 'C:\Users\Patrick\.claude\gates\state'`, `Join-Path $StateRoot 'status'` and `'agents'` (a second code home; its header comment says "progress.py owns how a status write is recognised").
- docs\agents\brief-preamble.md:13 `C:\Users\Patrick\.claude\gates\state\status\<your agent id>.md`.
- docs\agents\doc-review.md:9 and docs\agents\derive-once-review.md:9: same full path stated again.
- gates\README.md:75 (state folder layout list: `status\<agent_id>.md`).
- progress.py:22 `STATUS_PREFIX`, progress.py:58 prints the exact call with the real path to the agent at first call.
- tools\session_cards\writer-brief.md:100 states the helper call and "every 5 minutes" for card writers.
- memory every-brief-carries-status-rule.md:15 and MEMORY.md:45 (5 minutes + helper path; the note says the preamble owns the exact form).
What each tells its reader to do: all say append `HH:MM done ... | next: ...` every 5 minutes through the helper. Edges: exactly at the limit: README/preamble say nag at 5, refuse at 10; nothing else restates 10 except preamble:21. Main session vs agent: preamble binds agents only; memory note says "every brief", the main session has no status file. Card writers: skip the preamble (agents\card-writer.md:9) and get the call from writer-brief.md:100, with their own form `HH:MM done <n> of <m> cards | next:`. Worktree vs main: status_append.ps1 always writes the live main-checkout state folder, even from a worktree copy (its comment says so); common.STATE_ROOT is relative to the file, so a hook run from a worktree copy would look in the worktree's own state folder.
Kinds: K1 (path copied in 3 Hydra docs), K6 (common.py vs status_append.ps1 each decide the folder, plus `agents` record folder).
Split: none in value today. Possible split in the edge: status_append.ps1 fixed path vs common.STATE_ROOT file-relative path (in a worktree copy of gates).
Test holding homes equal: gates\tests\test_docs_numbers.py:182-190 holds preamble to `progress.STATUS_PREFIX`, "every 5 minutes" and "10 minutes". It does not check the folder path in the docs; it only forbids the old `hooks\state` (RETIRED, line 33). test_status_append.ps1 uses HOOKS_TEST_STATE_ROOT, not the live path.
Rulings: rulings\review-and-hooks.md:34 (2026-09-04) "demand status updates every 5 minutes of real time and reject any calls with no updates after 10 minutes". agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable".

## Q2. Which folders may a worktree sit in? [I6-056, I5-186]
Homes:
- gates\delete_rule.py:14-18 `WORKTREE_FOLDERS = (hydra-test\.claude\worktrees, C:\Users\Patrick\.claude\worktrees)` (the code owner for the delete rule).
- docs\agents\derive-once-review.md:197 states the same two paths.
- gates\README.md:136 states only `C:\Users\Patrick\.claude\worktrees\<task>` and "the delete rule trusts only those".
- gates\cleanup_sweep.ps1:174-178 (a second list for what is removable: `<root>\.claude\worktrees` in any repo, plus `<root>\worktrees` in the .claude repo).
- gates\session_cards_start.ps1:177 (scope test, "its .claude\worktrees too").
- Hydra tools\new_worktree.ps1:6,25-29 builds `<main>\.claude\worktrees\<TaskId>`; tools\effort_test_guard.ps1:17,105 holds its own check for `.claude\worktrees`.
- model-postures.md:5 "each in its own worktree under a `worktrees` folder (the delete rule trusts only those)".
- memory git-traps-in-shared-checkout.md:16 `git worktree add .claude\worktrees\<name>`.
- tools\session_cards\common.py:517-519 reads transcript folders named `<project>--claude-worktrees-*` (a fifth shape of the same location).
What each tells the reader: delete rule allows deletes only inside those two; sweep removes worktrees in a set that differs (it adds `<claude>\worktrees` which the delete rule also has, but allows Hydra's `<root>\worktrees` only under the .claude repo); README tells to cut under `~\.claude\worktrees`; derive-once-review lists both. Edges: main session vs agent: the delete rule judges subagents only; the main session is exempt (README:19), so the main session is not bound to these folders. Worktree vs main checkout: a path under the main checkout's `.claude\worktrees` passes; a worktree at `hydra-test\worktrees\<task>` (the very path this audit's own worktree uses, `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records`) is in neither delete_rule list, though cleanup_sweep.ps1:175 names `<root>\worktrees` for the .claude repo only.
Kinds: K1, K2 (README states one root, the code two), K6 (delete_rule, cleanup_sweep, new_worktree, effort_test_guard each decide where worktrees sit).
Split: delete_rule.WORKTREE_FOLDERS vs cleanup_sweep.ps1:174-178 vs README:136 (they do not list the same roots). Also this session's brief gives a worktree at `hydra-test\worktrees\...`, which delete_rule.py does not list.
Test: gates\tests\test_delete_rule.py:28-29,151-161 pins the delete rule's own roots as literals (a second copy of the list, not a link to the code). test_h52_cleanup_sweep.ps1 pins the sweep's. No test compares the two lists.
Ruling: rulings\review-and-hooks.md:17 (2026-10-25) chose "Keep rule, move worktrees (Recommended)" to a question saying "an agent recursively delete only inside its scratch folder or a worktree that sits in a `worktrees\` folder". It names the rule, not the two lists.

## Q3. Where does a learned Clone Hero fact go? [I5-009, I5-092]
Homes: Hydra CLAUDE.md:9 ("updates its page in the same commit ... never in a memory note"); brief-preamble.md:105 (adds: brief owns the page -> same commit; else put it in the report); docs\clone-hero\README.md:80 (adds: the main session adds it; a card's `write-back:` line says it); CLAUDE.md:29 (write-back card line, main session adds it); agents\card-writer.md:9 and tools\session_cards\writer-brief.md:88 (card writers record a `write-back:` line instead of reporting the fact).
Table: CLAUDE.md:9 - session or agent writes page in the commit. Preamble - agent without page ownership reports; main session adds. Card writers - write a `write-back:` header line, never the page. Main session - adds the fact, from reports or from `[write-back]` index marks. Edges: card writers skip the preamble, so for them the rule is their own brief; a worktree vs main: the page is in the Hydra repo, nothing says which checkout.
Kinds: K1 (three docs say it); K3 (card writers: same fact, different action and different actor); K5 none (no hook).
Splits: none in the wording. K3 difference noted above.
Test: none found (searched gates\tests and Hydra tests for "memory note" / "write-back").

## Q4. Where do the plain-English rules live? [I5-006]
Homes: ~\.claude\CLAUDE.md:1 (owner); Hydra CLAUDE.md:1 (pointer); brief-preamble.md:123 ("That file is the one home of those rules"); doc-review.md:81 (names the file as "the one home of the style rules" and then restates three of them: "jargon glossed on first use, sentences are short, ..."); model-postures.md:13 (restates "lead with the plain version, one idea per sentence, no bullet walls of file:line references"); writer-brief.md:94 ("Write in plain English, short sentences, no jargon without a gloss"); gates\stall.py:32 message ("Relay the claim ... in plain English"); model-postures.md:11; agents\scout/executor/planner.md:9 ("in plain English"); public-doc-style.md:3 (points at it); several memory notes.
Kinds: K1 (restated in doc-review, model-postures, writer-brief), K8-adjacent (preamble and doc-review each say "one home" while restating).
Split: none in content found. Test: none.

## Q5. Repo name, gh account/path/version [I5-010, I6-186, I6-224, I6-185]
Homes of `splax77/hydra-deluxe`: Hydra CLAUDE.md:13; docs\agents\this-machine.md:15; memory hydra-deluxe-rename.md:11 (says current home) and fc-video-workflow.md:15. issue-tracker.md:3 says "this repo" and :16 "Infer the repo from `git remote -v`" (tells the agent to read it from git, not from the name). publish.ps1 names no repo (checked "repo"/"Repo": no match). gh version/path/account: this-machine.md:13 only (one home; searched all scope for `patseguin1`, `2.97.0`). Edges: main session vs agent: the settings deny typed `gh` posts for both (README:73). Kinds: K1 (name in CLAUDE.md, this-machine, 2 memory notes). The git remote is the data owner (K4 risk: agents are given a name where `git remote -v` holds it). Split: this-machine.md:3 says the machine wins over the page. Test: none.

## Q6. Where is the version set? [I6-189]
Owner CMakeLists.txt:21 `project(Hydra VERSION 3.1.2 ...)`. Copy: this-machine.md:17 "reads VERSION 3.1.2 today" (K1, will drift on the next bump). installer\hydra.iss and build_installer.ps1 and src\core\version.h hold no literal (searched for x.y.z; none). Release naming `v` + version: this-machine.md:17 and docs\development.md (not read). Test: none for the this-machine line (the docs check is a Hydra test I did not run).

## Q7. Machine facts [I6-190, 191, 192, 193, 195, 196, 200, 201, 203, 205, 206]
Mostly one home: docs\agents\this-machine.md (lines 7-85), each a statement of a fact, with the page's own line 3 saying "the machine wins".
- I6-190 gates folder path: this-machine.md:19; also derive-once-review.md/doc-review.md/CLAUDE.md name `C:\Users\Patrick\.claude\gates\publish.ps1` and status_append path; code owner: gates folder location is file-relative (common.py:10); status_append.ps1:11 and progress.py:22,29 hard-code it. K1/K6.
- I6-191 Ghidra/Java: this-machine.md:23,25 only. skill reverse-engineer-anything\SKILL.md:37-79 repeats `rea-agents@6.5.0` (K1 of I6-192; owner is the registered MCP in `C:\Users\Patrick\.claude.json`, outside scope; I did not read it).
- I6-192 REA: this-machine.md:25 and SKILL.md (`npx -y rea-agents@6.5.0`). K1; they agree today.
- I6-193 CH decompile path: this-machine.md:27 points at docs\clone-hero\code\README.md (pointer, not read).
- I6-195 VS installs and I6-196 Find-CMake: this-machine.md:37-41 states versions and the Find-CMake story; owner code tools\find_cmake.ps1 (called from build_cpp.ps1:43 and tools\test_build_pass.ps1:93). Versions are copies of what vswhere returns (K1/K4: a typed table of values the tool reads).
- I6-200 CH log path: this-machine.md:53 only; no other home found in docs\agents, tools\*.py, tools\*.ps1.
- I6-201 cdb: this-machine.md:57-61 gives the PowerShell lookup "never from a typed path"; code copy tools\ch_probe\experiments\stall_harness.py:123-124 `_CDB_LOOKUP = "(Get-AppxPackage -Name Microsoft.WinDbg).InstallLocation"` (its comment says it is the lookup this-machine.md gives) and :175-185 builds the path. K6 (prose and code each hold the lookup; the code's comment names the page as source, which agent rule 15 says is not a source).
- I6-203 stall harness: this-machine.md:67; no other home.
- I6-205 hydra.db: this-machine.md:77 claims `db_path()` in src\app\config.cpp; I did not open config.cpp (row not verified; K8 check not done).
- I6-206 read a copy: not searched elsewhere (not reached).
Also this-machine.md:71 states ANTHROPIC_DEFAULT_OPUS_MODEL = `claude-opus-5-5`; live ~\.claude\settings.json:3 holds it (owner); tools\agent_cost.py:36-41 PRICES keys hold the model ids too and say "Source: the `claude-api` skill" (side note below).
Test: none holds this-machine.md values equal to the machine.

## Q8. Card tools: names, folders and paths [I4-042, 043, 044, 046, 059, 059b, 073, 079, 090, 092, 105]
- I4-042 decision log: common.py:44 owner; writer-brief.md:30 states the absolute path (K1). Also the docs say "D<n>". No test.
- I4-043 CLAUDE_REPO/HAND_TAGS: common.py:43,46; check.py:316 and prepass.py:461 each repeat the default list `[HYDRA_REPO, CLAUDE_REPO]` [I4-079] (K6 in code: two places build the same list; neither takes it from common). hand_tags: only common.py:46,554-561.
- I4-044 INDEX_FILE `index.md`: common.py:42; Hydra CLAUDE.md:25 states `docs/sessions/index.md` (K1); link_memory.py:24 has its own `INDEX_FILE = "MEMORY.md"` (different file, same constant name).
- I4-046 local-card rule: common.py:263-269 docstring quotes "The user ruled on 2026-10-31 ..."; writer-brief.md:16-18 quotes it again with the action for the writer ("add ` | local: yes`"); check.py:6-9,234-239; common.py:272-289 (`marked_local` = header says yes; `is_local_only` = card file sits in local-cards; check.keeps_local uses either). The comment says "Every script asks here, never by its own test" but check.py:239 combines two tests. I found no `## Confirmed` line for this ruling (grep for "locally", "local card", "not about the project" in rulings\ returned nothing), so the "user ruled" claim sits only in comments (a comment is a home, not the user's words). Kinds: K1, K3 (writer-brief tells the writer to decide it, common.py says the check decides), K7 (ruling not in rulings).
- I4-059/059b manifest.json, manifest.lock, .bak, condensed, staging: common.py:157-165,254-260 own them. Copies: writer-brief.md:26-31 states `DATA\batches`, `DATA\condensed`, `DATA\staging`, `DATA\local-cards`; gates\session_cards_start.ps1:259-271 builds `batches` and `staging` with its own `Join-Path` (K6: common.staging_dir owns staging but the hook rebuilds it). The hook does read repo/out/cards from common.py (session_cards_start.ps1:169-173), which is the right pattern.
- I4-073 where a passing card goes: check.py:242-260 owner; copies writer-brief.md:31 and common.py:263-269. Agree.
- I4-090 transcripts in scope: common.py:503-522 (one home; prepass.py:386 and prune_list.py call it). Comments say "plan decision 1/8" pointing at a plan (a comment, not a source).
- I4-092 CONTEXT.md path: prepass.py:456 only (default arg); common.build_tags takes the text path from the arg. One home. Searched session_cards, gates, Hydra docs\agents.
- I4-105 old card folder: migrate.py:98-99 only. One home.
Test: tools\session_cards\test_session_cards.py (builds all folders through `common.*` calls, e.g. lines 64,80,178; does not pin the writer-brief text). Nothing holds writer-brief.md's folder names equal to common.py.
Rulings: none found for card folders (see I4-046).

## Q9. What the settings sync and the shared-code list are [I5-037, I5-181 partly]
- sync after fragment change: Hydra agent-rules.md:7; model-postures.md:11; gates\README.md:73,108-112,154,160,168,172; code owner: gates\sync_settings.py (docstring per I5-181 row). Test: test_docs_numbers.py:142-146 `test_live_settings_carry_the_fragment` holds the live file equal to the fragment on main.
- "Shared code" list (`gates\common.py`, `gates\lib`, `gates\settings.fragment.json`, `gates\tests\lib`): agent-rules.md:7, model-postures.md:11, gates\README.md:156 (4 items, same words), memory merge-tests-touch-changed-files.md:15 (points at agent-rules rule 1). No code owner; hydra_precheck covers Hydra only. Kind: K1/K7 (stated 3 times, nothing checks it). Edge: main session runs the tests; agents never do.
- I5-186 marker: gates\README.md:138 says the `.derive-once-gate` marker names `gates/`, `agents/`, `tools/session_cards/`; the marker file itself (C:\Users\Patrick\.claude\.derive-once-gate:5-7) holds them; review_rules.py:157-185 reads them. Marker is the owner; README is a copy (K1).

## Q10. Smaller rows
- I5-141 scout write scope: agents\scout.md:9 "Change no files except your status line and files in your own scratch folder"; brief-preamble.md:41 (touch only owned files plus scratch) and :3; this brief (T5-finder.md:4) says no file in either repo. No hook enforces edit scope (grep of gates\*.py for "scout" finds only test_agent_stop.py); media_rule.py and delete_rule.py cover only media writes and deletes. K5/K7 (prose rule, no check). Main session vs agent: delete_rule applies to subagents only.
- I5-146 where agent rules live: gates\README.md:5 points at the preamble and says it does not repeat them; test_docs_numbers.py holds the preamble to progress.py numbers. agent-rules.md:5 also says details live in preamble + model-postures (I5-061). Both pointers, no values. Single home pattern; no split.
- I5-155 media guard: owner gates\media_rule.py:1-6,47-55 (`MEDIA_ROOT = r"C:\ytdl"`); README:21 names the constant and what the main session/subagent may do (pointer plus behaviour); audit-method.md:180 names D189. Hydra docs name no media rule; no `C:\ytdl` in docs\agents (searched). Memory ytdl-folders note and archived notes state `C:\ytdl` (K1). Edge: main session vs subagent is the whole rule. Test: gates\tests\test_media_rule.py (exists; not read).
- I5-176 pointer check: README:71 describes check_pointers; settings.fragment.json:101 registers it by full Hydra path; Hydra CLAUDE.md names nothing. Home of "what it checks": tools\check_pointers.py (not read) and README:71 (copy).
- I5-178 state layout: README:75 lists the files; code owners common.py (`STATE_ROOT`, `status_path`, `REVIEW_KINDS` at :11) and others per file; status_append.ps1 repeats `status` and `agents`. K1/K6.
- I5-192 shim scope: README:162; install_ref_gate.ps1:4 comment says the same about worktrees sharing hooks; no other home found in scope.
- I5-012, I5-023 (domain docs, this-machine pointer): CLAUDE.md pointers only; I5-010 above.
- I6-019, I6-020 (test-rubric, derivation audit): paths stated in agent docs: test-rubric.md by 5 pages (audit-method:5, derive-once-review:31,46,96, preamble:117, test-rubric-examples:3) as pointers; derivation-audit.md by audit-method:164,190 and derive-once-review:48. Pointers; no values to drift.
- I6-066, I6-074: doc-review.md:11 and :21,45 point at removed-file text with `git show` commands (correct form per CLAUDE.md). I6-074's claim "that page and the gates code win" matches README:5 pattern.
- I6-083: doc-review.md:81 says the global CLAUDE.md is "the one home of the style rules" and then restates some (see Q4).
- I6-089, I6-097 (handoff name and scratch rule): handoff.md:7,37; .claude\commands\handoff.md:11-12 states the name `docs/handoffs/<today>-<slug>-handoff.md` (K1). The skip for `docs/handoffs/` is in Hydra tests\test_docs_match_code.cpp:1233 (`kNotPermanentDirs`), tested at :1293-1308; agrees with handoff.md:37 (a test holds the claim to code).
- I6-172 run results folder: premise-map.md:44 `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\`; docs\agents\clone-hero-mcp.md not read, so other homes are unknown.

## Side notes
1. Hydra `.derive-once-gate:3-4` and `.doc-review-gate:2-3` say the gated paths are "decided by ... (hooks\lib\derive_once_rules.ps1 / doc_review_rules.ps1)". C:\Users\Patrick\.claude\hooks does not exist (checked), and `Test-DeriveOnceGatedPath` appears nowhere in gates. The decision is now in gates\review_rules.py (BUILTIN_CODE, `_is_code`, lines 157-185). K8, wrong owner claim, two lines.
2. tools\agent_cost.py:33-35 says prices come from "the `claude-api` skill" (a copy labelled as copy; also model ids at :37-40 repeat settings.json:3).
3. The audit brief's own worktree sits at `hydra-test\worktrees\...`; not in delete_rule.WORKTREE_FOLDERS (see Q2).
4. Hook gap: none found beyond the scout write scope not being enforced (Q10).

## Rows not reached or not verified in depth
I6-148 (only audit-method.md found), I6-205 (config.cpp `db_path()` not opened), I6-206, I6-215 (topic file list; no search), I6-193 (pointer target not read), I5-176 (check_pointers.py not read), I5-190-related python path, I4-092 and I4-105 (one home each, quick search only).

Counts: rows done about 49 of 56 (several as single-home rows). Candidates: Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9 groups. Splits: 3 (worktree roots; local-card test vs writer-brief and no confirmed ruling; Hydra gate markers naming a removed owner as a side note).
