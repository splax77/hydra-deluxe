# TOOL-AUDIT-T5-F08 (finder F08, session f6a384b5)

Read: brief; the preamble; line 108 of the hooks-from-scratch plan (read lines 100-112: "Each rule has one home"); Confirmed lines of review-and-hooks.md and agents-and-workflow.md (searched for worktree, scratch, ytdl, media, python, status, marker, posture, card words; one hit, below).
Searched, for every group: C:\Users\Patrick\.claude\gates (py, ps1, sh, json, README, tests), tools\session_cards, agents, model-postures.md, CLAUDE.md, skills (live, non-synced), live settings.json, Hydra CLAUDE.md, docs\agents (+rulings), .claude\commands, tools, memory notes (not archive). All 56 rows were taken; they fall into 14 questions (Q1-Q14). Each row id is listed under its question.
No test in gates\tests holds any of these path/name homes equal to each other, except where a test is named below. Tests that pin literal copies are named per question.
All "Q" quotes are from the files at the lines given.

Ruling found (Confirmed), speaks to Q2:
- docs\agents\rulings\review-and-hooks.md:17 (Confirmed section): "**2026-10-25** chose "Keep rule, move worktrees (Recommended)" to "The delete rule lets an agent recursively delete only inside its scratch folder or a worktree that sits in a `worktrees\` folder. Ten Hydra worktrees sit beside the repo as `C:\Users\Patrick\Downloads\Hydra\wt-*` instead. ... Which way?"" It names a `worktrees\` folder, not which one under Hydra.

## Q1. Where does an agent's status line go, and where is the state folder? (I1-020, I1-050, I1-053, I1-091, I2-017 part, I2-029, I1-055)
Owner code: common.py:10 STATE_ROOT; common.py:123-124 `status_path` = `<state>/status/<id>.md`; agents_dir common.py:119-120; check_id common.py:113-116.
Homes:
- status_append.ps1:11 `$StateRoot = 'C:\Users\Patrick\.claude\gates\state'` (full path, not derived), :13-14 `status` and `agents` subfolder names; :19-21 comment "Always the live state folder, even when this script runs from a copy: progress.py owns how a status write is recognised".
- progress.py:22 STATUS_PREFIX `& "C:/Users/Patrick/.claude/gates/status_append.ps1"` with comment "spelled as the allow rule in settings.fragment.json names it"; :28-29 `_HELPER_PATH` derived from it.
- settings.fragment.json:4 and live settings.json:10, the allow rule with the same text.
- progress.py:88-90 prints the status file via common.status_path (derived, fine).
- Preamble brief-preamble.md:13 (Hydra) "`C:\Users\Patrick\.claude\gates\state\status\<your agent id>.md`" and :15 the helper call; derive-once-review.md:9 and doc-review.md:9 (Hydra) repeat both.
- tools\session_cards\writer-brief.md:100 repeats the helper call and "One status line every 5 minutes".
- cleanup_sweep.ps1:61 and session_cards_start.ps1:53 re-derive state as `$PSScriptRoot\state`; cleanup_sweep.ps1:597-600 names subfolders `status` (`*.md`), `agents`, `stall`, `posture` and `*.json`; session_cards_start.ps1:54 names `session_cards`.
- tests\lib\harness.ps1:30 `$HOME\.claude\gates\state`; tests\replay.py:280 regex of the helper path (accepts `hooks|gates`).
- common.py:3 "The folder layout and the record formats are described in gates/README.md." README has no section that describes the layout; it names pieces in lines 9, 65, 69, 77 only.
- Memory note every-brief-carries-status-rule.md:13,15 (home, no path).
What each home tells its reader:
| Home | Tells |
|---|---|
| status_append.ps1 | agent writes to the live folder always, even from a copy |
| progress.py | main session: status call shape is judged against this one text |
| preamble / review pages | agent: type the path/call as written |
| writer-brief.md | card writer (skips the preamble): same call, same 5 min |
| cleanup_sweep.ps1 | sweep: prune `status\*.md` by age |
Edges: exactly at the limit — n/a (no number). Main session vs agent — main session has no status file rule here (progress.py applies to agents only). Card writers — own copy of the call in writer-brief.md:100. Worktree vs main — status_append.ps1 is fixed to the live path so a worktree copy still writes live; the other scripts derive from their own folder, so a worktree copy of cleanup_sweep/session_cards_start would use the worktree's state folder.
Kinds: K1 (preamble, two review pages, writer-brief, harness); K6 (status_append.ps1 and common.py both decide where the folder is; also the `*.md`/subfolder names in cleanup_sweep.ps1:597-600); K8 (common.py:3 says README describes the layout; it does not).
Splits: none on the value today. Possible: status_append.ps1 is fixed to the live path; cleanup_sweep.ps1:61 follows its own folder.
Tests: test_status_append.ps1 and test_progress.py:709,762 use STATUS_PREFIX; none holds status_append.ps1's folder equal to common.STATE_ROOT.
Also I1-053: reviews.jsonl, waivers.jsonl names are in common.py:213-251 and cleanup_sweep.ps1:27,551-584 (also named). I1-055: the id character rule also appears as `[A-Za-z0-9_-]+` in progress.py:24-25 (`_STATUS_CALL`) and the status_append.ps1 name/record check (:37-47): a third and fourth statement of what an agent id may be (K6).

## Q2. In which folders may an agent delete, and which worktrees count? (I1-032, I1-039 part, I1-033)
Owner: delete_rule.py:14-18 WORKTREE_FOLDERS (comment :14-16).
Homes:
- delete_rule.py:17-18 `...hydra-test\.claude\worktrees` and `C:\Users\Patrick\.claude\worktrees`; :62 refusal text lists them.
- Preamble :43 (Hydra): "any worktree directly inside a `worktrees` folder of either repo (`...\hydra-test\.claude\worktrees\` or `C:\Users\Patrick\.claude\worktrees\`)"; derive-once-review.md:197 same two paths; gates README.md:19 "the two repos' `worktrees` folders, listed at the top of `delete_rule.py`" (pointer), :136 "under `C:\Users\Patrick\.claude\worktrees\<task>`"; model-postures.md:5 "under a `worktrees` folder (the delete rule trusts only those)" (pointer); memory git-traps-in-shared-checkout.md:16 `.claude\worktrees\<name>`.
- cleanup_sweep.ps1:174-179 Get-SweepPrefixes: `<root>\.claude\worktrees` in any repo, plus `<root>\worktrees` for the repo the hook lives in (~/.claude only).
- tests\test_delete_rule.py:28-29 HOOKS_FOLDER / HYDRA_FOLDER literal copies; :160 `rm -rf C:/Users/Patrick/worktrees/x/y` must be refused.
- session_cards_start.ps1:177 comment "(its .claude\worktrees too)".
| Home | Tells |
|---|---|
| delete_rule.py | agent may delete only inside those two folders' worktrees; main session also gated by the same roots |
| cleanup_sweep.ps1 | removes idle worktrees under `.claude\worktrees` (Hydra) or `worktrees` and `.claude\worktrees` (~/.claude) |
| preamble | agent: cut worktree under the two named folders |
Edges: at the limit — a worktree's own folder never counts (delete_rule). Main vs agent — delete_rule runs for subagents (README:19), preamble says "hook checks every ... delete you run" (reader sees it as binding both). Card writers: skip preamble; they have no delete tool. Worktree vs main — Hydra worktrees actually in use sit in `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\` (git worktree list shows many, including this audit's own `...\worktrees\tooling-audit-records`), which is a third folder: not in delete_rule.WORKTREE_FOLDERS and not in cleanup_sweep's Hydra prefixes (cleanup_sweep adds `<root>\worktrees` only for ~/.claude).
Kinds: K6 (delete_rule and cleanup_sweep both decide "which worktree folders"), K2 (real use `hydra-test\worktrees\` vs the folder lists), K1 (preamble, review page, README, test literals), K3 possible (cleanup_sweep adds a folder delete_rule lacks).
Splits: delete_rule.py:17 / preamble:43 name `hydra-test\.claude\worktrees`; actual Hydra worktrees at `hydra-test\worktrees\...` match neither. cleanup_sweep.ps1:176-178 gives `worktrees` to ~/.claude only; delete_rule gives `C:\Users\Patrick\.claude\worktrees` to ~/.claude (the same folder as the sweep's `<root>\worktrees` for the .claude repo, so those agree).
Ruling: above (Confirmed, review-and-hooks.md:17).
I1-033 (scratch folder): `data["scratchpad_dir"]` is the hook input, read in delete_rule.py:48-55, media_rule.py:48 (via dr._roots), shell_output.py:90; one owner (the input). Homes that name it by words: preamble :43, README:19, shell_output.py comment (cleanup clears). Pointers; no value copy. I1-039: the "absolute path" test: delete_rule.py:93 `^([a-zA-Z]:[\\/]|/[a-zA-Z]/)`, also media_rule.py:10 PATH_SHAPE (does text look like a path, different question), review_dispatch.py:38 `_PATH` (path in a brief), bash_windows_rule.py:28 PATH_QUOTE, install_ref_gate.ps1:19 `^[A-Za-z]:/`: five spellings of "a drive path" for different questions (K6 candidate only for the first two; media_rule reuses delete_rule._inside/_roots/_norm, one owner there).

## Q3. Where does the main session write media freely? (I1-085, I1-090)
Owner: media_rule.py:6 MEDIA_ROOT `C:\ytdl` "(D189)".
Homes: only that line; the C:\ytdl folder is also in the user's additional dirs (not read, outside scope: the dispatch env). Memory ytdl-folders-not-a-project-matter.md names `C:\ytdl\videos` and `tracks` (a note, no rule). Decision log D189 (line 891) and D273 item 3 (line 1266): "writes ffmpeg or yt-dlp output under `C:\ytdl` freely; other such output outside scratch asks first in the main session and is refused in a subagent." (user's calls quoted in the log, not a `## Confirmed` file). No preamble/README line states the rule except README's media_rule paragraph (pointer, no value; media_rule listed in percall order README:11).
Single home for the value plus the user's log. Edges: main asks, agent is denied: media_rule.py:47-55 implements both; no prose elsewhere disagrees. Card writers: no shell tool for media.
Kinds: none found beyond one home. Tests: test_media_rule.py exists (not read; it calls the production function).

## Q4. Where is a long shell result saved, and how is it named? (I1-066, I1-067)
Owner: shell_output.py:26-28 SAVED_FOLDER / NAME_DIGITS, :85-96 saved_path.
Homes: README.md:23 "That file goes in a `shell-output` folder inside the session's scratch folder, which the cleanup sweep clears, or beside the transcript in a session with no scratch folder. `saved_path` owns where." (pointer plus folder name = value); test_shell_output.py:133 `"shell-output"`, :148,:322,:377 pin names `f8216bb3d50e`, `2c566dd885eb`, `372ea08cab33` (digit count 12 pinned by literal outputs).
Preamble :35 says only "the path of a file holding the full text"/"Read that file" (pointer). cleanup_sweep.ps1 does not name `shell-output` (it clears whole scratch folders).
Edges: main vs agent — same hook, same path. No scratch folder: falls back beside the transcript (FALLBACK_FOLDER `tool-results`, shell_output.py:27); only shell_output.py says so.
Kinds: K1 (README folder name; test literals of digit count and names, copied from one run's output).
Tests: tests pin literals; no test holds the README name equal.

## Q5. Which plan/measurement file is the source for a hook? (I1-071b, I1-082; also I4-001, I4-040 plan path)
shell_output.py:6-7 "docs/superpowers/plans/2026-11-07-shell-output-to-disk.md" (file exists in the Hydra tree); bash_windows_rule.py:6-7 `gates/tests/probe_results.md`; wake.py:5 also points there; README:23,25,27,73,178,182 point at `tests\probe_results.md`. All pointers; file exists. I4-001/I4-040: plan 2026-10-14-session-cards-backfill.md is spelled with "(removed; `git show 444631e6:...`)" in writer-brief.md:5, common.py:3-7, check.py, batch.py, index.py, link_memory.py, number_rulings.py, prepass.py (each its own header). Eight identical copies of one path-and-commit string (K1). Hydra docs rule: a finished plan leaves the repo and every line that names it says how to read it now (Hydra CLAUDE.md), so these removal notes are that rule's output, not a drift.
Kinds: K1 for the 8 card-tool copies; the others are pointers.

## Q6. Which folder holds rulings and its format page? (I3-010, I3-069)
Owner: ref_judge.py:41 RULINGS_DIR, with comment :39-40 "named here, not in ruling_rules.py ... ruling_rules.py reads it from here". ruling_rules.py:59 imports it; :94 FORMAT_PAGE = RULINGS_DIR + "README.md".
Other homes: ruling_rules.py:1 docstring "docs/agents/rulings/"; Hydra CLAUDE.md "Rulings and this machine" (docs/agents/rulings/, README.md); card tools common.py:53 comment and writer-brief.md:67 "`## Confirmed` in Hydra's `docs/agents/rulings/`"; Hydra tools (build_slot.ps1:74, compare_db.py:26, ch_mcp files) cite individual ruling files (pointers).
Kinds: K1 (docstring, comment and writer-brief copy the folder). Owner claim in ref_judge.py:39-40 is true (ruling_rules imports it).
Edges: card writers read the folder name from writer-brief.md:67 (a copy), not from code.

## Q7. Which files switch the review gates on, and which pages do refusals send people to? (I3-023, I3-024, I3-061)
Owner: review_rules.py:18 CODE_MARKER, DOC_MARKER; :20 CODE_BRIEF, DOC_BRIEF.
Homes: publish_gate.py:12 `MARKER = ".doc-review-gate"` (second code copy; used at :47); README.md:49,138 name `.derive-once-gate`/`.doc-review-gate` and which paths its `gate:` lines name; Hydra doc-review.md:17 ".doc-review-gate marker"; reviewer.md:33 names it; Hydra repo root holds both marker files; tests: test_agent_stop.py:32,57,61, test_publish.py:47, test_review_rules.py:37,286,316 use the literals; test_review_dispatch.py:158-171 use `review_rules.CODE_MARKER/DOC_MARKER` (by name).
Briefs: no other code home; CLAUDE.md (Hydra) names the two pages; reviewer.md and planner.md point at pages by path (pointers).
Kinds: K6/K1 (publish_gate.py:12 repeats review_rules.DOC_MARKER); tests pin literals in three files.
Splits: none (same text today).
Tests: none ties publish_gate.MARKER to review_rules.DOC_MARKER.

## Q8. Which repo is a review key judged in, and what does a folder in a brief look like? (I3-055, I3-056)
Owner: review_dispatch.py:38-39 `_PATH`, :65-99 judged_folder; comment :68-69 "with one exception the user approved on 2026-10-24".
Homes: no other code home of "folder named in a brief". The 2026-10-24 approval: not found as a Confirmed line in the two ruling files I read (my grep of their Confirmed sections for these words found none); it may be in another ruling file or the log; not checked. Docs that tell an agent/main session what to write in the brief: derive-once-review.md "For the orchestrator" (not read in full). README mentions the hook change dispatched from Hydra (README:138).
Kinds: single home for the code; the user-approval claim is unchecked (K8 possible if the claim has no Confirmed home; not graded).
Edges: main session vs agent: the dispatch hook runs on the main session's Agent call; worktree vs main checkout: the exception exists for a hook change in ~\.claude dispatched from a Hydra worktree.

## Q9. Which Python runs the gates? (I3-088, I3-092, I3-100)
Owner: none single. The path `C:\Users\Patrick\AppData\Local\Python\pythoncore-3.14-64\python.exe` is stated at: settings.fragment.json (9 times) and live settings.json (9 times; test_docs_numbers.py:142 checks the live file carries the fragment); publish.ps1:16; install_ref_gate.ps1:10 (default, `-Python` param); README.md:110,114,145; tests\test_shell_output.py:74; Hydra this-machine.md:7; derive-once-review.md:124; memory archive python-locations.md (excluded). Hydra preamble :97 tells agents `py -I tools/check_handoff.py`; session_cards_start.ps1 prints `py -I ...check.py` for the main session; this-machine.md:7 says `py` starts that same file, and bare `python` is a different install `C:\Python314\python.exe`.
| Home | Tells |
|---|---|
| fragment/live settings, README:114 | hooks start `python.exe -I -S <script>` by full path |
| publish.ps1, install_ref_gate.ps1 | scripts call the full path |
| this-machine.md, preamble | agents run `py` |
| derive-once-review.md:124 | agents run the full path for hook tests |
Edges: main vs agent — hooks always use the full path; agents are told `py`. Card writers: hook-printed commands use `py -I`. Worktree vs main — fragment is a copy; sync_settings.py (owner for fragment vs live) compares them; test_docs_numbers.py:142 tests the live file carries the fragment.
Kinds: K1 (many copies), K6 (installer default and publish.ps1 literal vs the settings registration). I3-092's other rows (LF/no BOM, quote rejection) have one home (install_ref_gate.ps1:53,57-58).
Splits: none on the value today. "py" and the full path both said to be the same file in this-machine.md:7; not checked against the machine.
Test: test_docs_numbers.py:142 (live settings carry the fragment); nothing ties publish.ps1 or install_ref_gate.ps1 to the fragment.

## Q10. How is the git shim recognised and installed? (I3-091, I3-093)
Owner: ref_judge.py:31-32 ALLOW/DENY/SELFTEST_OK for verdict strings.
Homes: install_ref_gate.ps1:12 `# git-ref-gate shim`, :13 `ref-judge selftest: allow ok, deny ok`; ref_gate.sh:2 (marker line), :15-16 `ref-gate-verdict: allow`/`deny` (copies of ref_judge.py:31); tests test_ref_judge.py:614,624,629,658 (literals; :658 uses a retired installer name `install_git_ref_gate.ps1`); ref_gate.sh:3 comment says the marker is the old shim's too. install_ref_gate.ps1:17-20 path form `C:\x` to `/c/x` (one home; a shell can't import).
Kinds: K1/K6 (selftest and verdict strings repeated in installer and shim; ref_judge.py is the producer). K3 not seen.
Splits: none today. Tests pin literals; at install time the installer runs the selftest and compares to its own constant (a runtime check of two homes, not a test).

## Q11. Live settings path and its sync state (I3-095)
Owner: sync_settings.py:37-43 `LIVE`, STATE_DIR, file names; comment ":37 Named in full, because a worktree has no settings.json of its own."
Homes: README.md:108 (live path), :170 (`state\settings_sync\settings.previous.json`), :110 (sync command), test_docs_numbers.py:142 and tests\test_sync_settings.py (use sync_settings by name). Hydra this-machine.md:71 names the same global settings.json for a different fact (model id).
Kinds: K1 (README states path and file name). No drift found.

## Q12. Where do stall/watch/hand-back/agents' record files live and how are an agent's transcript and the hooks repo found? (I2-017, I2-019, I2-026, I2-028, I2-043, I2-060)
Owners: stall.py:48-51 stall_path; wake.py:25-26 suffixes; agent_stop.py:39-41 handback_path (percall.py:104 uses it, one home); stall.py:54-68 `_session_agents`; stall.py:35 HOOKS_REPO.
Other homes: cleanup_sweep.ps1:597-600 (`stall`, `posture`, `agents` + patterns); status_append.ps1:45-46 (re-reads `agents\<session>\<id>.json` to decide if an id is an agent); common.py:119-128 agents_dir/_agent_path (the writer); progress.py:4 docstring "gates/state/agents/<session_id>/<agent_id>.json"; stall.py:174-187,307 builds `<session folder>\subagents\agent-<id>.jsonl` and `subagents\workflows\<run>\` while agent_stop.py:92,196,233 reads `agent_transcript_path` from the hook input (a value Claude Code gives); Hydra tools\agent_cost.py:160 `rglob("agent-*.jsonl")` under `subagents`; session-card prepass.py:162 and common.py:504 (subagents skipped); shell_output.py:95 uses `<session>\tool-results`.
Edges: main session vs agent — stall.py runs for the main session and guesses the agent's transcript folder; the SubagentStop input carries `agent_transcript_path` (agent_stop.py:92), so the layout is derived in stall.py and read in agent_stop.py (K4 if the PreToolUse input also carries it; not checked). HOOKS_REPO (stall.py:35) vs session cards common.CLAUDE_REPO (`~/.claude`), ruling_rules.CARD_TOOLS, sync_settings.GATES_DIR, tests\replay.py:42 REPOS: five places say where the hooks repo is.
Kinds: K6 (record-file shape judged in stall.py, status_append.ps1, cleanup_sweep.ps1), K1 (subagents layout in stall, prepass, agent_cost), K4 candidate (transcript location guessed).
Tests: none holds these equal.

## Q13. Where is the posture text and its last-send record? (I2-068, I2-071)
Owner: posture.py:14-15 POSTURE_PATH, :19-22 record `<STATE_ROOT>/posture/<session>.json`.
Homes: README.md:35 (names `..\model-postures.md`, pointer); cleanup_sweep.ps1:600 (`posture\*.json`); Hydra agent-rules.md:5 "model-postures" (pointer). Single code home. Kinds: K1 (cleanup_sweep re-names `posture`).

## Q14. Where do session cards, their data and tools live, and what do writers and the hook do with them? (I2-107, I2-108, I2-111, I2-112, I2-114, I3-081, I4-010, I4-011, I4-013, I4-014, I4-015, I4-016, I4-017, I4-036, I4-037, I4-038, I4-039, I4-040, I4-041)
Owner: tools\session_cards\common.py:31-46 (PROJECTS_ROOT, PROJECT_NAME, DATA_ROOT, DEFAULT_MEMORY, HYDRA_REPO, CARDS_ROOT, INDEX_FILE, DECISIONS_LOG) with its docstring :10-17 "this file owns both"; local_cards_dir/staging_dir common.py:258-269; card_name common.py:299.
Copies:
- writer-brief.md:25 DATA path, :29 memory index path (both spell `C--Users-Patrick-Downloads-Hydra-hydra-test`), :26 batch file shape, :28 tags.txt, :30 decision log absolute path and "entries start with `**D<n>`", :31 card name and where passing cards go.
- common.py:44 DECISIONS_LOG is relative; writer-brief.md:30 absolute; common.py:92 DECISION_ENTRY regex.
- Hydra tools\agent_cost.py:31 DEFAULT_ROOT `.claude/projects/C--Users-Patrick-Downloads-Hydra-hydra-test`; tools\check_pointers.py:30 MEMORY_DEFAULT `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory`: second and third code homes of the project folder name and memory path.
- session_cards_start.ps1:52,54 (ToolsDir `tools\session_cards`, state `session_cards`), :171-172 reads repo, data root, cards root from common.py (derived: pointer, good; comment ":168-169 read from the scripts' own common.py, which owns all three" is true); :274 `$rel = 'docs/sessions'` literal copy of CARDS_ROOT's tail; :257-263 batch names `wave-<n>-batch-<m>.txt` (batch.py:5,63 writes them: two homes); :243-253 reminder file; :21 comment "state_prune.ps1 leaves it alone" (pruner is now cleanup_sweep.ps1, cleanup_sweep.ps1:27 leaves `cleanup_sweep\`; K8 on a removed name, not checked whether state_prune.ps1 exists).
- ruling_rules.py:61 CARD_TOOLS built from its own folder and :80-92 loads common.py/prepass.py by path (derived).
- Hydra CLAUDE.md "Past sessions" states `docs/sessions/` and `docs/sessions/index.md` (home of the cards root and index name).
- prepass.py:437 writes `tags.txt` by literal while common.py:547 also names it (K6 inside the tool).
- check.py:38 CARD_FILE regex repeats card_name's shape; check.py:242-260 `publish()` and common.py:263-269 repeat the rule in writer-brief.md:31 "moves each card that passes into the repo's `docs\sessions\` folder, or into `DATA\local-cards\` when it carries `local: yes`".
| Home | Tells |
|---|---|
| common.py | tools: use these folders |
| writer-brief.md | card writer: read/write these absolute paths (the writer never runs code, so cannot read common.py) |
| hook text (session_cards_start.ps1) | main session: launch one writer per batch file, check.py moves passing cards into the cards root |
| Hydra tools (agent_cost, check_pointers) | read project/memory folders from their own literals |
Edges: card writers skip the preamble (card-writer.md:9) and carry their own paths; at the limit: writer-brief.md:101 "At 20 minutes or 100 tool calls" repeats progress.py's numbers (progress.py owns them). Main session vs agent: only the main session's check moves cards (writer-brief.md:33). Worktree vs main: writer-brief.md:5 and common.py:37-39 say writers cannot write into the main checkout from a worktree session.
Kinds: K1 (writer-brief paths and names; plan path x8), K6 (agent_cost.py:31, check_pointers.py:30, session_cards_start.ps1:274, prepass.py:437), K8 (session_cards_start.ps1:21 names state_prune.ps1).
Splits: none on today's values.
Tests: test_session_cards.py:45 swaps these constants for temp folders (it tests the tools against themselves); nothing holds writer-brief.md's paths equal to common.py.
I2-108: scope check "session folder is the Hydra repo or under it" session_cards_start.ps1:177-181,318; HYDRA_REPO is from common.py; other homes: none. I4-011 and I2-112: batch file name `wave-N-batch-M.txt` homes batch.py:5,63 and session_cards_start.ps1:257-263 (regex `^wave-(\d+)-batch-(\d+)\.txt$`); K6.

## Rows done / not reached
All 56 done: I1-020, I1-032, I1-033, I1-039, I1-050, I1-053, I1-055, I1-066, I1-067, I1-071b, I1-082, I1-085, I1-090, I1-091, I2-017, I2-019, I2-026, I2-028, I2-029, I2-043, I2-060, I2-068, I2-071, I2-107, I2-108, I2-111, I2-112, I2-114, I3-010, I3-023, I3-024, I3-055, I3-056, I3-061, I3-069, I3-081, I3-088, I3-091, I3-092, I3-093, I3-095, I3-100, I4-001, I4-010, I4-011, I4-013, I4-014, I4-015, I4-016, I4-017, I4-036, I4-037, I4-038, I4-039, I4-040, I4-041. Not reached: none. Limits: Q8's 2026-10-24 user approval was not traced; docs I did not open in full (derive-once-review.md body, test_media_rule.py) are noted per question.

## Side notes
- gates\README.md:19 quotes "The user ruled on 2026-10-31 that 'trying to ban specific command patterns is a losing battle ...'". That text is in docs\sessions\2026-10-31-6777b546.md and two plans, not in a `## Confirmed` file (grep of docs\agents\rulings\ found none).
- tests\test_ref_judge.py:658 writes a shim comment naming `install_git_ref_gate.ps1`, an installer name that no longer exists in gates\ (the file is install_ref_gate.ps1).
- Hook gap: a delete or media-output command written in a form the rule does not read passes; the README:19 already says this in general words.
- card-writer.md:9 and writer-brief.md:99-101 give card writers their own copy of the status rule (5 minutes; 20 minutes / 100 calls) in place of the preamble; progress.py owns the numbers.
