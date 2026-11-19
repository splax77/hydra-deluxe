# TOOL-AUDIT-T5-F11 (finder F11, agent a32ba82f9002e8a89)

Paths: G = C:\Users\Patrick\.claude\gates ; H = C:\Users\Patrick\Downloads\Hydra\hydra-test ; M = C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory

Read first: hooks-from-scratch plan line 108 (one home per rule; numbers at the top of progress.py; docs quote them; a test greps the docs) and the `## Confirmed` lines of review-and-hooks.md and agents-and-workflow.md. No Confirmed line speaks to any row below (they cover status lines, parallel work, hook arbitrariness). So "ruling found" is none for every row unless stated.

Searched for every row: G (py, ps1, sh, md, json, tests), tools/session_cards, agents, model-postures.md, ~/.claude/CLAUDE.md, skills, settings.json, H/CLAUDE.md, H/docs/agents, H/.claude/commands, H/tools, memory top level. Hook messages were read inside the hook files. Hydra src/ and tests/ were not in scope and not searched. The edge columns (limit, main vs agent, card writers, worktree vs main) are filled only where the question touches them. I ran no hook, no test and no build.

## Rows not reached
I8-255 (not a question; it is a note about unresolved memory links, the archive is out of scope). Thin searches are marked THIN in the row.

## Splits at a glance
- S1 worktree folders: delete rule vs cleanup sweep vs new_worktree vs the docs.
- S2 where a decision can live: derive-once-review.md:45 vs the precheck's own list.
- S3 build-slot and bench folder under the removed `.claude\hooks` folder.
- S4 HYDRA_MUTANT read two ways (C++ vs Python).
- S5 which Visual Studio install is picked (find_cmake.ps1 vs build_cpp.ps1 vs ci.yml).
- S6 replay.cpp says replay.h holds the scoring; replay.h says it owns none.
- S7 constants.py says it holds every address; other files hold addresses.

---
## I8-237 Where a handoff's shape and claim-checking are defined
Homes: H/docs/agents/handoff.md:3-39 (owner: shape, 40-character hashes at :19 and :35, no scratch path :37, `## Checker output` :27). H/.claude/commands/handoff.md:10 pointer "It owns the shape a handoff takes and how its claims are checked; follow it, not memory" (true). The same command also restates parts of the shape: :11 "the `## Claims` table with one row for every fact ... each in a `ruling` row. Write every hash in full and name no scratch path", :13 "under a `## Checker output` heading". H/tools/check_handoff.py:9 pointer. H/CLAUDE.md:57 and rule 9 point at handoff.md.
Reader table: command = main session follows 6 numbered steps; handoff.md = same facts as a guide; checker enforces hash/path/function rows only (handoff.md:39: it does not judge the Claim column or test/ruling/state rows). Agents and card writers: not addressed. Worktree vs main: command step 6 commits on its own branch.
Kinds: K1 (the command copies the table, hash and scratch rules), K5-like (the command spells out "every hash in full", which the checker enforces by marking short hashes missing). Splits: none. Test: tools/test_check_handoff.py tests the checker, not the command text. Ruling: none.

## I8-238 Handoff file path
Homes: handoff.md:7 `docs/handoffs/<YYYY-MM-DD>-<slug>-handoff.md` (owner; date is the writing day); command handoff.md:11 and :12 (same path, twice); H/tools/check_handoff.py:398-403 handoff_date (code reads the date from the file name, falls back to git date); G/../settings.json:14 `Edit(//c/Users/Patrick/Downloads/Hydra/hydra-test/docs/handoffs/**)` (a permission rule on the folder). The slug rule (2 to 6 lowercase words) is only in the command (:8), not in handoff.md.
Kinds: K1 (path in 3 places plus a permission rule), K7-ish for the slug rule (stated once, nothing checks it). No split. No test pins them equal (test_check_handoff.py uses names of the same shape but does not pin the shape).

## I8-255 NOT REACHED (see above).

## I9-001 Where the agent files (C:\Users\Patrick\.claude\agents) live
Homes: G/tests/test_docs_numbers.py:35 `AGENTS_DIR` (the definition, used at :40, :101-105). G/tests/test_agent_files.py:11 imports it with the comment "the one home of where the agent files live" (pointer; true). No code reads the folder (Claude Code owns it). README names `agents\` with no path. One home plus a correct pointer. K8: none. Searched `.claude.agents` and `AGENTS_DIR` in G, scope.

## I9-017 Where the state root is
Owner: G/common.py:10 `STATE_ROOT = dirname(abspath(__file__))/state`. Copies: G/tests/test_common.py:157-159 (pins it); G/status_append.ps1:11 `$StateRoot = 'C:\Users\Patrick\.claude\gates\state'` (typed second owner in PowerShell; comment :8-10 says progress.py owns how a state folder is laid out; override HOOKS_TEST_STATE_ROOT); G/tests/lib/harness.ps1:30 `$HOME\.claude\gates\state`; G/cleanup_sweep.ps1:61 `$PSScriptRoot\state`; G/README.md:75, :88, :134; H/docs/agents/brief-preamble.md:13, derive-once-review.md:9, doc-review.md:9 (typed `C:\Users\Patrick\.claude\gates\state\status\<id>.md`); H/tools/build_slot.ps1:10,78, bench_run.ps1:8-9,20, compare_db.py:43 (typed a different root, `.claude\hooks\state`).
Kinds: K1 (preamble and review docs, README), K6 (status_append.ps1 and cleanup_sweep.ps1 vs common.py), K2 (build_slot, bench_run, compare_db point at a folder that no longer exists: `ls C:\Users\Patrick\.claude\hooks` fails).
SPLIT S3: build_slot.ps1:78 default slot folder `...\.claude\hooks\state\build_slots` vs the real state root `...\.claude\gates\state`. The cleanup sweep prunes only gates\state, so nothing clears the hooks one. Test: test_build_pass.ps1:41 sets HYDRA_BUILD_SLOT_DIR to a temp folder, so no test holds the default equal to anything. Test holding status_append.ps1 equal to common.py: none (test_status_append.ps1 sets the env override).

## I9-018 Where agent records and locks are stored
Owner: G/common.py:120 `<root>/agents/<session>`, :124 `status/<agent>.md`, :200 `.json` becomes `.lock`. Copies: G/status_append.ps1:13-14 ($StatusDir, $AgentsDir: re-derives the subfolders); G/cleanup_sweep.ps1:597-600 prunes `'status' '*.md'`, `'agents' '*'`, `'stall' '*.json'`, `'posture' '*.json'` (folder names as strings); G/README.md:75 states the whole layout including `<agent_id>.lock`; G/progress.py:4 docstring; tests test_common.py:165,217, test_progress.py:301,813. Kinds: K1, K6 (PowerShell re-derives the layout). No test ties the PowerShell folders to common.py.

## I9-023 Docs and root the number-check reads
Home: G/tests/test_docs_numbers.py:22-26,70 (DEFAULT_HYDRA_ROOT, GATES_HYDRA_ROOT, DOCS = 5 files). The Hydra root path is typed in: delete_rule.py:17, tests/replay.py:42, settings.fragment.json:101, settings.json:13-15,111, tools/session_cards/common.py:36 (HYDRA_REPO), writer-brief.md:30,88, skills/analyzing-sp-squeezes/SKILL.md:49, H/tools/effort_test_taint.py:31. Other doc lists: test_docs_numbers.py:41 REVIEW_DOCS_HYDRA (3 files), check_pointers.py documents() (CLAUDE.md + all docs/agents/*.md + memory top level). README.md:154 says it in words with no list.
Kinds: K1/K6 for the Hydra root (about 9 homes), K1 for the three doc lists. This test is itself the one that holds docs equal to the code for the budget numbers (CALL_COUNT, STATUS_EVERY vs progress.py) and for names the README must carry (README_NAMES :54-55, check :179), but only for these 5 docs.

## I9-038 Settings fragment is the source; live settings location
Owner: G/sync_settings.py:36 FRAGMENT, :39 `LIVE = C:\Users\Patrick\.claude\settings.json`. Copies: test_docs_numbers.py:46-48 `SETTINGS = dirname(GATES_DIR)/settings.json` (second derivation of LIVE; FRAGMENT is imported, SETTINGS is not), comment "The fragment is the reviewed source of the registrations"; G/README.md:73,108,110,170; H/docs/agents/this-machine.md:71 (typed path). Kinds: K1, K6. Test holding live equal to fragment: test_docs_numbers.py:142-146 test_live_settings_carry_the_fragment. Main vs worktree: the test skips when no settings.json sits beside gates (a worktree), so the check runs on main only.

## I9-061 Where a subagent may delete recursively
Owner: G/delete_rule.py:17-18 WORKTREE_FOLDERS (`H\.claude\worktrees`, `C:\Users\Patrick\.claude\worktrees`); the scratch root comes from the hook input field `scratchpad_dir`.
Copies: H/docs/agents/brief-preamble.md:43 (both folders typed; "any worktree directly inside a `worktrees` folder of either repo"; also "a worktree's own folder and the `worktrees` folder itself never do", which delete_rule.py:16 also says); H/docs/agents/derive-once-review.md:197 (both typed); G/README.md:136 (only `C:\Users\Patrick\.claude\worktrees\<task>`; "Keep it under a `worktrees` folder"); G/model-postures.md:5 ("under a `worktrees` folder (the delete rule trusts only those)"); G/cleanup_sweep.ps1:174-178 (a different list: `<root>\.claude\worktrees` in any repo plus `<root>\worktrees` in the repo the hook lives in); H/tools/new_worktree.ps1:6,29-30 (creates `<main>\.claude\worktrees\<TaskId>`); H/tools/effort_test_guard.ps1:17,105 and effort_test_taint.py:43; H/tools/ch_mcp/quiet.py:53; G/session_cards_start.ps1:177; tests test_delete_rule.py:24-31 and test_media_rule.py:23-28 (their own constants).
Reader table: delete_rule.py = allow any worktree directly in those two exact folders; preamble:43 and derive-once-review:197 = same, typed; README:136 and model-postures:5 = "a `worktrees` folder" (looser); cleanup_sweep = removes worktrees under `.claude\worktrees` of any repo plus `<claude repo>\worktrees`; new_worktree = Hydra only. Main vs agent: the rule judges subagents; the preamble says the hook checks "you". Card writers read writer-brief.md, not the preamble; the hook still applies to them. Worktree vs main: preamble says cwd is the session's folder, not the worktree. At the limit: "strictly inside"; folder itself never passes.
SPLIT S1: (a) the folder sets differ among delete_rule.py, cleanup_sweep.ps1 (for the .claude repo it covers `C:\Users\Patrick\.claude\worktrees` and `C:\Users\Patrick\.claude\.claude\worktrees`) and new_worktree.ps1. (b) README:136 and model-postures:5 say "a `worktrees` folder"; only two exact folders pass. (c) This audit's own worktree is `H\worktrees\tooling-audit-records`, which is under neither folder delete_rule.py names (read from code; hook not run).
Kinds: K1, K2/K3 (looser wording), K6. Test: test_delete_rule.py pins the rule against its own constants, not against cleanup_sweep or the docs.

## I9-068 / I9-069 What the media guard does in the main session / in a subagent
Owner: G/media_rule.py (MEDIA_ROOT `C:\ytdl` :6, TOOLS :7, TEMPLATE :11). Copies: G/README.md:21 (restates: main may write under MEDIA_ROOT and its scratch folder with no question, elsewhere the user is asked; a subagent only under its scratch folder, else refused with a reason saying what to do instead; "same literal-path test `delete_rule.py` uses"; the value of MEDIA_ROOT is not typed, a pointer); G/percall.py:4,115-117 (main=False for agents); the decision numbers D189 and D273 appear in media_rule.py:1,53 and README:21 (pointers to the Hydra decision log). M/ytdl-folders-not-a-project-matter.md is not a rule. brief-preamble.md does not mention the media guard (searched `ytdl|media`).
Table: main session = free in C:\ytdl and scratch, else the user is asked; subagent = only scratch (a worktree is not a media root, test_media_rule.py:89-146); card writers/agents that skip the preamble get the hook but no prose, and the refusal text names the fix. Worktree vs main: see above. At the limit: unreadable paths count as outside.
Kinds: K1 (README restates who may write where), K5-ish (README spells out what the hook enforces). No split. Test: test_media_rule.py pins behaviour; `C:\ytdl` copied at test_media_rule.py:23-28. Ruling: none.

## I9-072 Fixed project, session and path values in fixtures
Copies: test_delete_rule.py:24-31, test_media_rule.py:23-28, test_ref_judge.py:670 `CARDS_PROJECT = "C--Users-Patrick-Downloads-Hydra-hydra-test"  # the card tools' PROJECT_NAME`. Owners: delete_rule.py:17-18, media_rule.py:6, tools/session_cards/common.py:33 PROJECT_NAME. The project folder name is also typed in check_pointers.py:30, writer-brief.md:25,29 and H/tools/agent_cost.py:31. Kind: K1 (tests copy the owners' values; test_ref_judge.py:670's comment names its owner but still types the value). No test imports the owner's constant (test_delete_rule.py does not read delete_rule.WORKTREE_FOLDERS).

## I9-080 Posture text and its send record
Owner: G/posture.py:14-15 POSTURE_PATH, :19 _state_path (`<root>/posture/<session>.json`, `{"sent": time}` at :50). Copies: G/README.md:35 (describes send timing; names `RESEND_MINUTES` without its value: pointer) and :75 (`posture\<session_id>.json holds the time posture.py last sent`); G/cleanup_sweep.ps1:600 `'posture' '*.json'`; tests test_posture.py:35-40, test_stall.py:616-657. test_docs_numbers.py:54 README_NAMES holds the README's name `RESEND_MINUTES` against the code. Kinds: K1, K6 (the sweep types the folder name and pattern). Main vs agent: README:35 says an agent's hand-back fires the prompt hook too and posture.py cannot tell it from a typed prompt. No split.

## I9-107 Default repo for gh in tests
Homes: H/CLAUDE.md:13 "Issues live as GitHub issues in `splax77/hydra-deluxe`"; H/docs/agents/this-machine.md:15 "The repo is `splax77/hydra-deluxe`"; M/hydra-deluxe-rename.md:11 (history of three repo names, ends at `splax77/hydra-deluxe`); G/tests/test_publish.py:341-393 (fixtures). H/docs/agents/issue-tracker.md:16 says "Infer the repo from `git remote -v`" and names no repo. publish.ps1 does not set a repo (only reads gh flags, :25). The git remote is the real owner; no tool holds the name. Kinds: K1 (three prose homes and a test), K7-ish (CLAUDE.md names it, issue-tracker.md says to infer it). No value split.

## I9-110 Other repos named in a brief
Owner: G/review_dispatch.py:36 MAX_BRIEF_FOLDERS = 16, :66-75 judged_folder docstring ("approved on 2026-10-24"). Copies: G/README.md:39 (describes the exception, ends "`judged_folder` in that file owns exactly when that is allowed"; does not state 16: pointer); G/agents/reviewer.md:23 ("When the brief names a commit in another repo, with that repo's root folder written in the brief, review and fix it there"); test_review_dispatch.py:224-286, whose docstring names "Get-JudgedFolder's rule" (a function that no longer exists: searched G for `Get-JudgedFolder`, only the test hit). Kinds: K1 (reviewer.md restates the rule), K8-ish (test docstring names a gone function as the rule). Comment "approved on 2026-10-24" is a comment, not a Confirmed line.

## I9-121 Posture file location and print
Same homes as I9-080: G/posture.py:15 POSTURE_PATH = `<gates parent>/model-postures.md`; README:35 `..\model-postures.md`; test_stall.py:616-657. Kind: K1. Main session only (additionalContext on UserPromptSubmit); a missing or empty file warns in a systemMessage (posture.py:45).

## I9-136 Real-app record file name
Homes: H/tools/hydra_ui_mcp.py:78-81 `REALAPP_FILE = "realapp.jsonl"` (comment: "The hooks repo's ref gate reads it, so its field names are a contract (realapp_record writes them)"); G/review_rules.py:44-49 `REALAPP_FILE = "realapp.jsonl"` (comment: "Hydra's tools/hydra_ui_mcp.py owns its name (REALAPP_FILE) and its fields (realapp_record); this file only reads them", yet it types the name itself); G/README.md:55; test_docs_numbers.py:54-55,179 (README must name `realapp.jsonl`); tests test_review_rules.py:410,528 (own constant), H/tools/test_hydra_ui_mcp.py:104,638 (own constant). Kinds: K1/K6 across repos, K8 (review_rules.py:48 says the other file owns the name, but this file has its own literal). Test holding the two equal: none found. Field names (kind, commit, dirty, n) are read at review_rules.py:385 with no cross-repo test.

## I9-151 Where the full text of a trimmed result is saved
Owner: G/shell_output.py:26-27 SAVED_FOLDER "shell-output", FALLBACK_FOLDER "tool-results", :89-95 saved_path. Copies: G/README.md:23 (pointer: "`saved_path` owns where"); brief-preamble.md trim paragraph (says the full text sits in a file at a path given and to read it with Read offset/limit; names no folder); cleanup_sweep clears the scratch folder (shell_output.py:26 comment); design plan H/docs/superpowers/plans/2026-11-07-shell-output-to-disk.md (named at shell_output.py:6); test_shell_output.py:133-134,374-389 pins it. Main vs agent: scratchpad_dir comes from hook input; with none it falls back to Claude Code's tool-results folder. Kind: one owner with pointers. No split.

## I9-161 Gate files copied by the tests
Home: G/tests/test_ref_judge.py:21 `GATE_FILES = ("ref_gate.sh", "install_ref_gate.ps1", "ref_judge.py", "common.py")`. What the gate actually loads: ref_judge.py:23 imports common; :236-240 loads review_rules lazily (the test writes its own stub, :64-70); ruling_rules per ref_judge.py:39-41. README:53 describes the shim in words. Kind: K1 (the test hand-lists the gate's files). If ref_judge gains an import the list goes stale; nothing holds it equal.

## I9-175 / I9-176 / I9-178 The PowerShell test harness
Home: G/tests/lib/harness.ps1:5 "This file is the one home of how a test runs a hook, reads its verdict, counts checks and prints its summary line". :11 "Its proof is tests\test_h39_harness.ps1" and :21 "Only test_h39_harness.ps1 changes them": that file does not exist (ls G/tests; grep for `h39` hits only these two lines). K8 (a proof claim naming a missing test). :18,211 hooks-folder default; :30 live state folder `$HOME\.claude\gates\state` (copy of common.STATE_ROOT: K6/K1); :167-175 Check-Decision requires a deny to end with a line starting "Instead: ". The same rule is asserted separately in test_bash_windows_rule.py:300, test_review_rules.py:119-124, test_delete_rule.py:126,259,297,345 and followed by hook messages; no prose page states it as a rule (K7). Users of the harness: test_h52_cleanup_sweep.ps1, test_h53_session_cards_start.ps1, test_status_append.ps1. I9-178 (New-HooksCopy copies named hooks plus lib\ into a temp folder): single home; THIN (no second home found, searched `hooks-copy`, `New-HooksCopy`). I9-175: single home (the harness) with its three users.

## I9-184 / I9-185 Replay: where output may be written / what it reads
Owner of the output rule: G/tests/replay_extract.py:11-14,31-45 (gates/state/replay/ or a scratch folder; tracked_path_refusal) and replay.py:24,360-373 (calls it); README:75 says state is gitignored. Reads: replay_extract.py:25-28 PROJECTS (~/.claude/projects), TEMP_CLAUDE (~/AppData/Local/Temp/claude), globs `*/*.jsonl`, `*/*/subagents/*.jsonl`, `subagents/workflows/*.jsonl`, `subagents/workflows/*/*.jsonl`; refusal-kind words :56-62. The same "where are the transcripts" question is answered by: tools/session_cards/common.py:32,503-513 (top-level transcripts only; subagents skipped, "plan decision 1"), H/tools/agent_cost.py:31,160 (project folder; `subagents` rglob `agent-*.jsonl`), G/shell_output.py:95 (transcript folder), G/cleanup_sweep.ps1:60 (`Temp\claude`), H/tools/hydra_ui_mcp.py:65 (`claude` under temp). The refusal-kind words have no second code home (searched `hook error`, `blocked by hook` in G py and session_cards; only a test fixture at test_session_cards.py:721). Kinds: K1/K6 (transcript layout in 4 code places with different depth rules; not a contradiction, since purposes differ), K1 (scratch layout).

## I9-192 Removed since the first read
Checked: `C:\Users\Patrick\.claude\hooks` no longer exists. Live references still pointing at it: H/tools/build_slot.ps1:10,78; H/tools/bench_run.ps1:8-9,20; H/tools/compare_db.py:43 (all S3); history/fixtures only: G/tests/probe_results.md:57, G/tests/replay.py:280 (regex accepts both `hooks|gates`), H/tools/test_check_pointers.py:53. G/tests/test_docs_numbers.py:25-27 RETIRED `hooks[/\\](state|tests)` forbids those words in the five Hydra docs only; it does not reach H/tools scripts. This is the test that holds the docs equal and it stops short of tools/. Kind: K2.

---
## I10-001 Where memory notes are read from by default
Homes: H/tools/check_pointers.py:30 MEMORY_DEFAULT (typed); tools/session_cards/common.py:35 DEFAULT_MEMORY (derived from PROJECTS_ROOT and PROJECT_NAME); writer-brief.md:29 (typed path to MEMORY.md). Kind: K6 (check_pointers types what session_cards derives), K1 (writer-brief). Ruling: none.

## I10-002 Gates folder default
Homes: check_pointers.py:31 GATES_DEFAULT; G/status_append.ps1:11; H/docs/agents/this-machine.md:19 "The hooks folder is `C:\Users\Patrick\.claude\gates`"; H/CLAUDE.md (publish.ps1 path); brief-preamble.md:13,17; derive-once-review.md:9; doc-review.md:9; settings.fragment.json:117 and settings.json:111 (command paths); harness.ps1:18 (derived). sync_settings.py derives GATES_DIR from its own file. Kinds: K1 (prose), K6 (check_pointers and status_append type it).

## I10-004 / I10-005 Which gates subfolders hold hook scripts / which folders the index skips
Home: check_pointers.py:33-35 HOOK_FOLDERS ("", "lib", "tests"), SKIP_FOLDERS {".git","__pycache__","state"}. G/README.md:134 describes the whitelist .gitignore; cleanup_sweep.ps1:597-600 names state subfolders. One home; searched G, H/tools, docs/agents for lists of hook subfolders.

## I10-009 Which pages quote the user word for word
Homes: check_pointers.py:42 RULINGS_FOLDER; G/ref_judge.py:41 RULINGS_DIR = "docs/agents/rulings/" (comment :39-40); G/ruling_rules.py:1; H/CLAUDE.md, docs/agents/rulings/README.md, derive-once-review.md:45, agent-rules.md:33 (prose). Kinds: K6 (two code constants in two repos, same folder), K1 (prose). No test holds the two equal. The brief lists other user-word homes (cards, decision log, plans); check_pointers treats only the rulings folder as quoted.

## I10-010 Which documents the pointer checker reads
Homes: check_pointers.py:63-79 documents(); G/README.md:71 (same list in words); test_docs_numbers.py DOCS (5) and REVIEW_DOCS_HYDRA (3) are other lists; the docs-match-code scan is in src/tests (not searched). M/MEMORY.md says finished notes go to archive/; documents() says archive "names retired things on purpose". Kind: K1/K6 (3 doc lists). Test: tools/test_check_pointers.py (checker behaviour only).

## I10-021 Which refs mean "on main" / "pushed"
Homes: H/tools/check_handoff.py:109,362-364 (`refs/heads/main`, `refs/remotes/origin/main`); handoff.md:19 (`git branch --contains`, `git log origin/main`). In gates the literal `refs/heads/main` is typed in ref_judge.py:33 MAIN (constant), review_dispatch.py:111,123,128,136, review_rules.py, agent_stop.py:170, publish_gate.py:47, cleanup_sweep.ps1:270, tests/replay.py. Kind: K6 for "what is main" (ref_judge has a constant; the others re-type it). check_handoff's "on main or pushed" is a different question from the gates' "a move of main". No value split.

## I10-034 Where slot locks live
Homes: H/tools/build_slot.ps1:10, :78 `'C:\Users\Patrick\.claude\hooks\state\build_slots'`, :94 HYDRA_BUILD_SLOT_DIR; hydra_ui_mcp.py:24,88 (pointer: "The rule is tools/build_slot.ps1's"); tests set the override (test_build_pass.ps1:41, test_hydra_ui_mcp.py:660,695). SPLIT S3 (see I9-017). Side: the limit "three" is stated in build_slot.ps1:73 (comment), CLAUDE.md rule 14, agent-rules.md:20, brief-preamble.md:57.

## I10-048 Build another worktree; exit codes
Home: build_slot.ps1:40-46,358. "put old-release worktrees on a short path" (260-character limit) appears only there (searched `260.char|short path`). One home.

## I10-055 Where the user's library is read from
Home: H/tools/hydra_ui_mcp.py:55-56,147 DEFAULT_LIBRARY_DIR `C:\Program Files\Hydra`, with claims at :17 ("snapshot_library, the only code that knows that folder's path") and :140 ("This is the only place that knows the library folder's path"). Other homes of that folder: H/tools/bench.cpp:979-986 (searches ProgramFiles, ProgramW6432 and ProgramFiles(x86) plus a typed `C:\Program Files\Hydra`); compare_db.py:51,87; ui-testing.md:193 (restates the order: library_dir, then HYDRA_LIBRARY_DIR, then DEFAULT_LIBRARY_DIR), :350; this-machine.md:77. Kinds: K8 (the "only code that knows" claim is wrong: bench.cpp knows it), K1 (docs). Whether src/ knows install folders was not searched.

## I10-056 Exe names and build folder
Home: hydra_ui_mcp.py:59-62. `build-cpp\Release` is typed in about 30 places: tools doc_shots.ps1:8, capture_window.ps1:7, check_shards.ps1:17, mutate/find.py:18,629, rewrite.py:516, test_hydra_ui_mcp.py:4; docs agent-rules.md:7, brief-preamble.md:47, derive-once-review.md:24,99, integrate.md:37,43,44,46, this-machine.md:63,77, ui-testing.md:24,170,185,237,349; G/session_cards_start.ps1:285 (a hook message that tells the main session to run `build-cpp\Release\hydra_tests.exe "-tc=docs match code*"`, the same text as H/CLAUDE.md); G/model-postures.md:11; skills/analyzing-sp-squeezes/SKILL.md:18; 6 tests. None points at an owner (build_cpp.ps1/CMakePresets.json were not read for their own definition: THIN). Kind: K1/K6.

## I10-057 Scratch folder for hydra_ui_mcp
Homes: hydra_ui_mcp.py:65-67,344 `<temp>/claude/hydra_live_<tag>`; ui-testing.md:213 ("`hydra_live_<tag>` in the `claude` folder of the Windows temp folder ... removes a kept folder after a week with no writes"); hydra_ui_mcp.py:770 ("after a week"); G/cleanup_sweep.ps1:54 `$ScratchAgeDays = 7` (owner of the age), :60 `Temp\claude`; ch_mcp/game_tools.py:47 "ch_live_"; replay_extract.py:26. The sweep does not name `hydra_live_`; it keys on running-session names under `Temp\claude` (:396-440, read headings only: THIN), so whether it clears these folders is not shown. Kinds: K1 ("a week" in 2 prose places vs 7 in code), K6 (`Temp\claude` in 4 places).

## I10-058 What a snapshot copies
Homes: hydra_ui_mcp.py:72 SNAPSHOT_FILES; ui-testing.md:193 and :346-350 (same three names); capture_window.ps1:8-10; this-machine.md:89. Kind: K1. No test holds the docs list equal.

## I10-072 Where cmake is looked for
Owner: H/tools/find_cmake.ps1:5-24 (PATH first, then vswhere -latest, else -all -prerelease, then the bundled path). Copies and pointers: build_cpp.ps1:12-13 pointer; this-machine.md:41 ("`cmake` is not on PowerShell's PATH ... Today that is the 2026 Build Tools' copy"); brief-preamble.md:31,57. Second finder: build_cpp.ps1:56-62 runs vswhere itself (`-all -prerelease ... Microsoft.VisualStudio.Component.VC.Tools.x86.x64`, newest by version, with its reason); .github/workflows/ci.yml:326 `vswhere -latest -all -products * -property installationPath`.
SPLIT S5: find_cmake.ps1 prefers `-latest`; build_cpp.ps1 takes the newest of `-all -prerelease` and says `-latest` skips an install with a pending update; ci.yml uses a third form. The cmake and the compiler can come from different installs. Kinds: K6, K3 (docs say cmake is not on PATH; find_cmake tries PATH first). "Today that is ..." is a dated fact in a doc.

## I10-073 Where task worktrees go and are named
Owner: new_worktree.ps1:6,25,29-30 (`<main>\.claude\worktrees\<TaskId>`, branch `claude/<TaskId>`, TaskId regex; claims "The main checkout owns .claude\worktrees"). Copies: the I9-061 list; branch prefix `claude/` in effort_test_taint.py:40 CLAUDE_BRANCH and fixtures; G/README.md:136 names `C:\Users\Patrick\.claude\worktrees\<task>`. M/git-traps-in-shared-checkout.md not read in full (THIN). Kinds: K1, S1.

## I10-074 Worktree starts with no build folder
One home: new_worktree.ps1:8-16. Searched `stale object|tracking.log|no build folder|build folder|own build`; cleanup_sweep.ps1:4 mentions "a build folder near 1 GB" (a separate fact).

## I10-078 What a capture -Dir must hold
Homes: capture_window.ps1:6-10; ui-testing.md:346-352 (same list in prose; "the two mimalloc DLLs" vs the script's two named dlls; "copy ... again before each run" = the script's sentence). Kind: K1. Minor K3: ui-testing says case 3 writes a settings.ini and copies only the ui ini, while the script says all three files are copied.

## I10-079 Doc picture inputs and outputs
Homes: doc_shots.ps1:2-11; H/docs/agents/doc-screenshots.md:3-29 (names the exe, shots.txt, docs/images, shrink.py: same values; the script says that page "explains the whole step"); tools/doc_shots/shots.txt:1-9. Kind: K1 (pointer plus values on both sides). No split.

## I10b-015 Where the il2cpp dumps live
Homes: H/tools/ch_mcp/build_facts.py:28 DUMPS_ROOT (used by chdis.py:218); kit_methods.json:3 "dump"; launch_patch.json:4 "dump"; README.md:7,12,20,58. No Python code reads the two JSON "dump" values (searched `["dump"]`, `get("dump")`; the TS agent files were not read for them: THIN). Kind: K1/K6.

## I10b-016 Dumper path and cap
Homes: dump_build.py:20,25. README.md:15 names DUMPER_CAP_S without its value (pointer). The dumper path appears nowhere else (searched `Il2CppDumper-net7`). One home.

## I10b-019 Where the game lives
Owner: game_files.py:31 GAME_DIR; chdis.py:214 and build_facts.py:33,69,108 use it (derived). Typed copies: dump_build.py:9, README.md:12,58, ch_probe/experiments/results/overlay-spike/il2cpp-fields.md:7. Kind: K1.

## I10b-020 Game save folders and files
Owner: game_files.py:36-41. Copies: H/docs/agents/clone-hero-mcp.md:15,60,62 (`C:\Users\Patrick\Documents\Clone Hero\scorestats.json`); this-machine.md:53 (`C:\Users\Patrick\AppData\locallow\srylain Inc_\Clone Hero\`). bot_run.py:448,458,473 use the constants. Kind: K1.

## I10b-025 Scratch and log name prefixes
Owner: game_tools.py:47-57. Copies: ch_probe/experiments/stall_harness.py:796,854 types `press-1.csv`/`press-{n}.csv` itself; server.py:423 docstring `builds-<n>.json`; clone-hero-mcp.md:52; rulings/clone-hero.md:29 quotes the user choosing "builds-<n>.json (Recommended)" (I did not check which heading it sits under, so I do not call it a Confirmed ruling). Kind: K1.

## I10b-058 Where addresses live
Home: ch_probe/constants.py:3-7 "This is the ONE place the numbers live. Every other module imports from here; nothing else hard-codes an address." Other addresses: ch_mcp/agent/menu.ts:54 `PLAYER_SCREEN_FRAME_RVA = 0x32F590` (comment: tests/test_frida_tools.py checks it against the dump's facts); launch_patch.json:8,11 loader_rva `0x3968D0`, patch_rva `0x396EC5`; start_candidates.json:19 rva `0x8D090`; kit_methods.json dump_line fields. None of those values appear in constants.py (grep for 32F590, 3968D0, 396EC5, 8D090). dump_facts.py:383 reads constants.py. SPLIT S7 (K8): claim vs reality. Test holding one equal: test_frida_tools.py for menu.ts (per its comment; not opened).

## I10b-064 Game process names
Owner: constants.py:270-273 (MODULE_NAME, PROCESS_NAME, WINDOW_TITLE). game_files.py:27,32 imports PROCESS_NAME, so the inventory note "game_files.py has PROCESS_NAME separately" does not match the file. Typed copies: dump_build.py:55,57 "GameAssembly.dll", launch_patch.json:7 "module", agent/launch.ts:2 (comment); game_files.py:36-38 "Clone Hero" folder names. Kind: K1 (module name x3).

## I10b-084 Where decisions are looked up
Owner: H/tools/precheck/decisions.cpp:317-338 and decisions.h:55-59 ("docs/adr/*.md and CONTEXT.md whole, the 'User decisions' block and 'Test limits' sections of docs/superpowers/plans/*.md, and the D-records of docs/audit/*decisions*.md"); check_3.cpp:2 pointer "decisions.h owns the docs the numbers are compared against". Others: derive-once-review.md:45 (ADR in docs/adr/, CONTEXT.md, a line under `## Confirmed` in docs/agents/rulings/, or the "User decisions" header of a plan; a later sentence adds the decision log); H/CLAUDE.md:21 and domain.md:9; tools/session_cards/common.py:44 DECISIONS_LOG and writer-brief.md:30,78.
SPLIT S2: derive-once-review.md:45 lists `## Confirmed` lines in docs/agents/rulings/ as a decision; the precheck reads no rulings folder (grep `rulings` in decisions.cpp/.h: no hit) and also reads "Test limits" sections the doc does not list. The doc says git does not check "those other places", so the reviewer does; that is a claim about intent, not a check. Kinds: K2/K6.

## I10b-100 Where scoring lives
Home: H/tools/replay.cpp:28-29 "The scoring itself lives in core/replay.h so this tool and tests/test_replay share one implementation." vs H/src/core/replay.h:1-12 "It owns no scoring rule", naming core/scoring.h chord_score_table, core::paid_by_sp and core::backend_row_value (core/backend_value.h), core::after_sp_end and core::sqout_position. tests/test_replay.cpp, test_replay_schedule.cpp, test_replay_windows.cpp include core/replay.h (the "share one implementation" part holds). tools/replay_schedule.h:2-10 names core/note_window.h for the hit window. SPLIT S6 (K8): replay.cpp says replay.h holds the scoring; replay.h says it holds none. src/core internals not read.

## I10b-105 Mutation tool: switch name, build folder, formats
Homes: tools/mutate/hydra_mutant.h:41-51 (C++ reader: unset gives -1; empty, negative or non-number stops the program), listener.cpp:42,97 (HYDRA_MUTANT, HYDRA_MUTANT_OUT), py_mutate.py:55-71 (`int(os.environ.get('HYDRA_MUTANT') or 0)`; HYDRA_MUTANT_REACH), run.py:7-9,629-631 (sets/clears it); MUTANTS_FORMAT = 1 at run.py:69 ("the mutants.json format rewrite.py writes (its FORMAT)") and FORMAT = 1 at rewrite.py:86 (docstring :52 "FORMAT 1"), find.py:77 (a separate spot format, also 1); "build-mutation" at run.py:7,32,864, rewrite.py:19,514,518, .gitignore:50, docs/development.md:589,611; physical-core count at run.py:596-609 only.
SPLIT S4: HYDRA_MUTANT unset means -1 in C++ and an empty string is refused there; Python treats unset and empty both as 0. Whether 0 means "no mutant" in Python's numbering I did not verify. K6. Format number: K1/K6 (rewrite.py writes, run.py re-types it; tests use run.MUTANTS_FORMAT; nothing ties it to rewrite.FORMAT). Default build folder: K1 x4.

## I10b-110 Fixture checkout, worktree and branch shapes
Homes: H/tools/test_effort_test_taint.py:18-21; effort_test_taint.py:31 MAIN_CHECKOUT, :40 CLAUDE_BRANCH, :43 climbs; effort_test_guard.ps1:30,105; plus the I9-061 list. Kind: K1/K6 (the checkout path is typed in about 9 files across both repos; effort_test_taint.py is code, not a test).

---
## Side notes
- G/tests/replay.py:24 holds LINE_BUDGET (hook code cap); the cap number is also in rulings (review-and-hooks.md, "Raise cap to 3,050"); not searched further.
- A way past the delete rule exists in general terms: it judges only literal absolute paths and only the two named worktree folders (see S1); I give no steps.
- The three-build limit is typed in build_slot.ps1:73, CLAUDE.md rule 14, agent-rules.md:20, brief-preamble.md:57 (not my row).
- G/session_cards_start.ps1:285 prints the docs-check command to the main session; the same command is in H/CLAUDE.md (K1).
- This audit's worktree `H\worktrees\tooling-audit-records` sits outside the folders the delete rule trusts.
