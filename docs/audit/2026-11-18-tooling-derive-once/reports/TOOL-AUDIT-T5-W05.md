# TOOL-AUDIT-T5-W05 (second-round verifier, ledger L03)

Everything below is by reading; I ran nothing. Short paths: GR = C:\Users\Patrick\.claude\gates; H = C:\Users\Patrick\Downloads\Hydra\hydra-test; R = H\docs\agents\rulings; DOR = H\docs\agents\derive-once-review.md; MP = C:\Users\Patrick\.claude\model-postures.md.

**Warning for the main session: the gates repo changed while I worked.** GR\review_dispatch.py was rewritten (commit 4e37bbb, merged by 091ef7a on 2026-11-18, file saved 22:01), and GR\README.md line 39 was rewritten with it. Line numbers in the ledger for GR files are therefore stale in places (for example ref_judge.RULINGS_DIR is now line 45, not 41; the test's old-shim fixture is now line 674, not 658). I quote the file as it is now. The Hydra main-checkout docs (DOR, MP) have not moved, so some splits below are new.

### L-F07-01
**Verdict:** CONFIRMED.
**Homes as found:** Agent files: GR/../agents scout.md:4 `model: sonnet`, reviewer.md:4 `model: sonnet`, executor.md:4 `model: opus`, planner.md:4 `model: fable`, card-writer.md:4 `model: haiku`. H\CLAUDE.md rule 2 and H\docs\agents\agent-rules.md:8 hold the same sentence: "Read-only scouts run on Sonnet or Haiku." MP:5: "Executors on Opus ... Reviewers run on Sonnet. Read-only scouts run on Sonnet or Haiku. Card writers run on Haiku." DOR:5 and doc-review.md:5 "You run on Sonnet."; DOR:202 "Dispatch one fresh Sonnet reviewer per change."; integrate.md:56 "fresh Sonnet reviewer"; audit-method.md:114 "executors on Opus". C:\Users\Patrick\.claude\settings.json:3 `"ANTHROPIC_DEFAULT_OPUS_MODEL": "claude-opus-5-5"`; H\docs\agents\this-machine.md:71 "sets the ANTHROPIC_DEFAULT_OPUS_MODEL entry ... to `claude-opus-5-5` ... The user picks this model by hand." H\docs\agents\brief-preamble.md:67 `--trailer "Co-Authored-By: Claude Opus 5.5 ..."` and :69 "Use the model line your brief gives if it differs." H\tools\agent_cost.py:34-39 PRICES keys claude-fable-5-1, claude-opus-5-5, claude-sonnet-5-5, claude-haiku-5-5, comment "Source: the `claude-api` skill".
**Splits:**
- Scout: prose (CLAUDE.md rule 2, MP:5) allows "Sonnet or Haiku"; scout.md:4 pins `sonnet`. They differ when a scout is meant to run on Haiku.
- Opus alias: this-machine.md:71 and settings.json:3 name `claude-opus-5-5`; agent_cost.py PRICES holds the same literal and the preamble trailer holds "Opus 5.5" as text. Same value today; they differ when the user changes the env entry.
- Preamble:67 hardcodes the Opus trailer for every agent, including Sonnet scouts; :69 says a brief may override. They differ for any non-Opus agent whose brief does not give the line.
**Ruling:** Not looked up beyond the ledger (R\agents-and-workflow.md:12 and R\review-and-hooks.md:22 are the finder's quotes; I did not open them). Neither is said to name both sides of the scout split.
**Open:** Whether a dispatch can override a scout's pinned model: a run would settle it.

### L-F07-02
**Verdict:** CONFIRMED-CHANGED. The code was rewritten after the ledger; the README now agrees with the code, but the Hydra docs now disagree with both in new ways.
**Homes as found (live files):** GR\review_dispatch.py:152-186 `body`: an isolated dispatch is refused (:161); line 169 `if not keys and not labelled and not _gated(cwd): return None` (a brief with no 40-hex key and neither labelled line passes with no record unless the repo has a review marker on its local main, `_gated` :140-145); otherwise `brief_lines` (:41-57) requires exactly one `Review range: <start>...<key>` line and one `Review repo: <folder>` line; a second commit anywhere in the brief is refused (:175-178); `_review_problem` (:99-133) refuses a non-top folder, a key git does not know or is on main, a repo with no local main (`_key_problem` :83-85), a detached HEAD, a checked-out branch that is main or the start branch or does not point at the key, a missing start branch, and a start with zero or more than one merge base. The record it writes holds tip, session, prompt hash, time, folder plus base, base_branch, branch and git_dir (:183-185). GR\README.md:39 (new) states all this, including "A brief with no commit and neither line passes with no record, but only from a session folder whose repo has no review marker on `main`." DOR:17 (main checkout) "A hook refuses a reviewer dispatch with no key, a key already on `main`, a key git does not know or more than one commit". DOR:200 "A hook refuses a reviewer dispatch with no key ... and records the tip it handed over and the branches pointing at it. A reviewer launched from a Workflow script, or as any other agent type, gets no such record". DOR:17 says "The range is always `main...<key>`". MP:5: "A brief may name a commit in the other repo if it writes that repo's root folder."
**Splits:**
- B1: DOR:17/:200 say a brief with no key is refused; the code (:169) lets it pass when the repo is not gated. They differ in a repo with no review marker on main. README:39 now states the pass, so README and DOR disagree.
- B2: DOR lists four refusals; code also refuses a missing `Review range:` / `Review repo:` line, a folder that is not a worktree top, a wrong checked-out branch, a missing start branch, a repo with no local main, and no single merge base. DOR does not mention the two lines at all. The two differ for every dispatch: a brief written only to DOR's description is refused.
- B2b: DOR:200 says the hook records "the branches pointing at it"; the code now records one branch, a base and a start branch.
- B2c: DOR:17 says the range is always `main...<key>`; the code takes any local start branch.
- B2d: MP:5 says a brief names a commit with the other repo's root folder; the code reads the `Review repo:` line (the old folder rule is gone).
- B3: archived note C:\Users\Patrick\.claude\projects\...\memory\archive\agent-worktree-base-is-origin-main.md:15 says "Reviewers still use `isolation: worktree`"; DOR:200 and review_dispatch.py:161-162 refuse isolation. The note is archived; it still tells a future reader the opposite.
**Ruling:** Not found. R\review-and-hooks.md has `## Confirmed` at :7 and `## Unconfirmed` at :37. I did not find a Confirmed line naming either side (workflow-reviewer limit rests on README:188 "plan question Q2"; I did not search the rulings for it again).
**Open:** none beyond reading; the hook's behaviour on a real dispatch is a run.

### L-F07-03
**Verdict:** CONFIRMED-CHANGED (counts as V07 corrected).
**Homes as found:** H\docs\agents\ui-testing.md:209 "The tool list in `tools/hydra_ui_mcp.py` is the owner; this table is a copy of it." The table (:213-229) has 17 rows, hydra_start to hydra_run, and no `hydra_build_pass`. H\tools\test_hydra_ui_mcp.py:51-55 TOOL_NAMES holds those 17 plus `hydra_build_pass` = 18. The SDK test (:715-724) holds the live server's list to TOOL_NAMES. hydra_ui_mcp.py:874 `def hydra_build_pass`, :22 docstring "One tool has nothing to do with the GUI". NO_SESSION hydra_ui_mcp.py:76 `"no session; call hydra_start"`; UT:189 mentions the same words. GR\percall.py:20 `BUILD_PASS_TOOL = "mcp__hydra__hydra_build_pass"`, used :108. H\.mcp.json registers `hydra` (`py -I tools/hydra_ui_mcp.py`) and `ch`.
**Splits:**
- UT table (17 tools) vs server and test (18, with hydra_build_pass). They differ for the one tool that gives a pass: a reader of the table does not learn it exists.
- percall.py:20 spells the tool name `mcp__hydra__hydra_build_pass` by hand; the server defines `hydra_build_pass`; I found no test that holds them equal (not exhaustive; I did not open test_percall_*.py).
**Ruling:** none found. **Open:** none.

### L-F07-04
**Verdict:** CONFIRMED.
**Homes as found:** H\docs\agents\clone-hero-mcp.md:11 "The list of tools lives in one place: `build_server()` in `tools/ch_mcp/server.py`". GR\percall.py:16-18 `KEY_TOOLS = ("mcp__ch__ch_key", "mcp__ch__ch_press")`, `KEY_ROUTE_TOOLS = {"mcp__ch__ch_play_chart": "keys"}`; percall.py:61 `mcp__ch__ch_call` unwrapped. H\tools\ch_mcp\game_tools.py:70-72 `ROUTE_KIT = "kit"`, `ROUTE_KEYS = "keys"`, `ROUTES`. H\tools\ch_mcp\server.py defines `ch_press`, `ch_key`, `ch_play_chart` (:383, :463, :472). front.py:60 `NOTHING_OPEN = "nothing open"`; server.py:205 `h.open_items()) or "nothing open"` (literal, not the constant); tests test_server.py:74,:119 literal "nothing open". kit_tools.py:92 `KIT_PLAYER = "Player1"`; ch_kit_setup uses it (:1063). agent\kit.ts:44 `ALLOWED_CALLS` vs kit_methods.json (pinned by tests\test_frida_attach.py: "kit.ts's to KIT_METHODS"); agent\menu.ts:40 own `["get_activeInHierarchy","get_gameObject"]`, pinned to its names by the same test.
**Splits:**
- front.py:60 NOTHING_OPEN vs server.py:205 literal: front.py says "Any other reply counts as something open"; they differ the day one word changes (a changed word refuses a reload).
- percall.py:18 route literal "keys" vs game_tools.ROUTE_KEYS: same value today; they differ if the route is renamed (the lock would stop matching). test_percall_key.py pins the literal, not the server constant.
- percall's mcp__ch__ tool names vs server tool names: same today; no test holds them equal that I found.
**Ruling:** The finder's quote "Keep Player1" is at R\clone-hero.md:79, which is AFTER `## Unconfirmed` (:67). It is not the user's ruling and cannot be cited as one. Nothing Confirmed found for Player1.
**Open:** whether menu.ts's two names are held by any list beyond that test (the test's per-agent rule only).
**Hook gap:** yes, in general words: GR\percall.py key-route lock matches tool names and the route string by exact text.

### L-F07-06
**Verdict:** CONFIRMED.
**Homes as found:** GR\common.py:10 `STATE_ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "state")`; :113-116 `check_id` refuses "", ".", "..", and any of `/ \ :`; :119-124 `agents_dir`, `status_path`. GR\status_append.ps1:11 `$StateRoot = 'C:\Users\Patrick\.claude\gates\state'` with comment "Always the live state folder, even when this script runs from a copy"; override only `HOOKS_TEST_STATE_ROOT`; accepts a name only when some `agents\<session>\<name>.json` exists. GR\progress.py:22 `STATUS_PREFIX`; :26 `_STATUS_CALL` accepts the id as `[A-Za-z0-9_-]+`. brief-preamble.md:13 and :19, DOR:9, doc-review.md:9 repeat the full path. cleanup_sweep.ps1:596-600 lists `status`, `agents`, `stall`, `posture`. session_cards_start.ps1:53 `Join-Path $PSScriptRoot 'state'`. stall.py:48 builds the `stall` folder; posture.py builds `posture`. GR\common.py:3 "The folder layout and the record formats are described in gates/README.md"; README has `state\` described in one paragraph (:75) and no section named layout.
**Splits:**
- Folder: status_append.ps1:11 fixed live path vs common.py:10 path from its own file; same today, they differ when the gates folder runs from a copy.
- Id shape: check_id passes any id without `/ \ :`; progress.py:26 accepts only `[A-Za-z0-9_-]+` in a status call; status_append.ps1 accepts any name that has a record. An id such as `a.b` passes the first and third and cannot be written through a status call. Whether real ids ever contain other characters is not settled by reading.
- Who may write: status_append.ps1 accepts a line for any id with a record in any session; the preamble says "must be your own agent id"; only progress.py refuses another id (for the call text). A different-id call through some other route is not refused by the helper (hook gap, general words).
- README:75 lists `agents`, `status`, `stall`, `reviews.jsonl`, `waivers.jsonl` but I did not find `posture\` named there; the sweep names it and posture.py writes it.
**Ruling:** none naming these sides. **Open:** gates-copy behaviour and real id shapes (runs); whether "layout" counts as README:75 (reading calls it partial).
**Hook gap:** yes, in general words: GR\status_append.ps1 accepts a status line for any agent id that has a record, not only the caller's.

### L-F07-08
**Verdict:** CONFIRMED.
**Homes as found:** R\review-and-hooks.md:33 "demand status every 10 tool calls and reject the next call after 15" and :34 "demand status updates every 5 minutes of real time and reject any calls with no updates after 10 minutes" (both 2026-09-04, session 48cdbb34; `## Confirmed` :7, `## Unconfirmed` :37). R\agents-and-workflow.md:25 (Confirmed section :7-:31) "i don't care if an agent is read only, no status updates is unacceptable". GR\progress.py:16 `STATUS_NAG_MINUTES, STATUS_DENY_MINUTES = 5, 10`; preamble:13, DOR:9, doc-review.md:9, CLAUDE.md rule 7, agent-rules.md:13 all say "every 5 minutes". GR\tests\test_docs_numbers.py:29 RETIRED holds `every 10 tool calls`; :45 STATUS_EVERY regex holds docs' "every N minutes" to progress.py. Memory note every-brief-carries-status-rule.md:15 "No gate checks a brief for this sentence today".
**Splits:**
- Rulings :33 (10 tool calls / 15) and :34 (5 minutes / 10 minutes): both Confirmed, same day, different rules. Code and docs follow :34; the test retires :33's wording. Neither line names the other, and I found no Confirmed line saying one replaced the other.
- "Every brief carries the status sentence" is an instruction with no check (K7).
**Ruling:** as above; neither names both sides. **Open:** none.

### L-F07-09
**Verdict:** CONFIRMED.
**Homes as found:** GR\ref_judge.py:40 `TRAILER_KEYS = ("Task", "Agent", "Session")`; `missing_trailers` :242-252 requires each value to hold an alphanumeric character (`any(c.isalnum() ...)`) and skips merge commits; refusal text (:339) `--trailer "Agent: <your agent id, or orchestrator>"`. brief-preamble.md:67 gives four trailers including `Co-Authored-By: Claude Opus 5.5` and :69 "Use the model line your brief gives". The memory note git-traps-in-shared-checkout.md:21 says the three keys must sit in the last paragraph and pass `Co-Authored-By` as a `--trailer` too. DOR:100 and doc-review.md:95 say "the preamble's trailers and your own agent id". GR\tests\test_review_rules.py:21 and test_ref_judge.py:22 hold own trailer lists as literals.
**Splits:**
- Preamble gives Co-Authored-By as a trailer to write; ref_judge requires only the three keys. They differ for a commit with no Co-Authored-By (accepted).
- Preamble says give your own agent id; ref_judge accepts any value with an alphanumeric, and its own refusal also offers `orchestrator`. They differ for a commit whose Agent value is not the committer's id.
- The refusal text offers `orchestrator`; the preamble does not mention it.
**Ruling:** none Confirmed found; R\review-and-hooks.md:57 (finder's "subagent Agent: trailer must be its own id") is below :37, so Unconfirmed. **Open:** none.

### L-F07-10
**Verdict:** CONFIRMED-CHANGED (agrees with V07's reading of I1).
**Homes as found:** C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:64 "Note whether the fleet hooks held: one background executor per task, no nested background workers." (The SKILL.md in the .claude repo is also shown modified in `git status`, so line 64 may move.) preamble:77 "Never use run_in_background ... You have no helper agents." H\CLAUDE.md rule 6 "Agents run in parallel, in workflows, while the main session merges." R\agents-and-workflow.md Confirmed lines 17, 20, 21, 23 (parallel, e.g. :17 "run all the library runs in parallel instead of one after the other"). MP:7 `earlyStart` rule; GR\agent_stop.py:4-5 treats an agent that ended its turn to wait on its own background work as parked; GR\stall.py:2, :39-42 define an "empty fleet" notice (a Workflow open with no agent call for 300 seconds).
**Splits:**
- SKILL.md:64 asks the reader to check "one background executor per task"; the preamble forbids an agent's own run_in_background. They may or may not differ: the skill does not say whether it means a main-session background dispatch.
- "Fleet hooks": the phrase has no definition I found; stall.py uses "fleet" only for its empty-fleet notice. I searched gates/*.py, gates/README.md, model-postures.md and the skill.
**Ruling:** the Confirmed lines are about running in parallel; none names a split. **Open:** the meaning of "fleet hooks" and "background executor"; reading cannot tell.

### L-F07-12
**Verdict:** CONFIRMED-CHANGED (a third home was missed).
**Homes as found:** GR\ref_gate.sh:8 `[ "$CLAUDECODE" = 1 ] || exit 0` (comment :5 "Outside Claude Code (CLAUDECODE is not 1) it does nothing"). H\tools\build_slot.ps1:89-90 "True when this run is under the gate: the same test the git ref gate uses" `function Test-BuildPassGated { $env:CLAUDECODE -eq '1' }`. H\Directory.Build.targets:9 `<Target Name="HydraBuildPassCheck" Condition="'$(CLAUDECODE)' == '1'">` (third home; the row did not list it; test_build_pass.ps1:396 pins that condition string). H\docs\development.md:143 "The limit applies only when `CLAUDECODE` is 1." GR\README.md:162 and :186 (an agent's shell sees the same variables as the main session).
**Splits:** none in value today (all three test for "1"). They differ only if the variable's convention changes. build_slot's gate is "any Claude Code shell"; CLAUDE.md rule 14 and percall.py:108 say only the main session builds (the agent refusal sits on the tool, not the variable). Three places would each need the change.
**Ruling:** none found. **Open:** none.

### L-F07-13
**Verdict:** CONFIRMED.
**Homes as found:** GR\tests\probe_results.md rows 1, 3, 7, 10 and the sections "The Bash tool on Windows (2026-11-06)" and "Shell output probes (2026-11-07)"; GR\README.md:176-190 restates them: "SubagentStop fires for a command hook ... it does not fire at all for an agent that `maxTurns` stops" (probe_results row 1 and 7), "`scratchpad_dir` appears in desktop-hosted sessions ... A plain `claude -p` session leaves it out" (row 3; delete_rule.py:52, media_rule.py:48, shell_output.py:89 read `scratchpad_dir`), the brief SHA-256 sentence (row 4), "2.1.308 ... 2.1.310" for different probes, and README:186 "2026-11-07 ... 2.1.317 ... All 35 names and hashes matched". brief-preamble.md:73 repeats budget numbers (owner progress.py).
**Splits:** none found; the three Claude Code versions belong to different probes. README restates probe_results, so a changed probe result would leave README stale; test_docs_numbers.py holds budget numbers and retired words, not README:176-190.
**Ruling:** none. **Open:** the maxTurns claim rests on a probe only; a run would be needed to re-prove it.

### L-F07-14
**Verdict:** CONFIRMED.
**Homes as found:** H\.claude\commands\handoff.md:9 "1. Stop launching work. Start no new agent and no new task until the handoff is written." I searched H\CLAUDE.md, H\docs\agents\*.md, H\.claude\commands, MP, GR\agents, GR\*.py for "stop launching", "no new agent" and "start no new": one hit, that line.
**Splits:** none. **Ruling:** none found. **Open:** none. No hook checks it (an instruction with no check).

### L-F07-15
**Verdict:** CONFIRMED.
**Homes as found:** C:\Users\Patrick\.claude.json:4771-4780 holds the `rea` server with `npx -y rea-agents@6.5.0 mcp` (one occurrence of the pin). H\docs\agents\this-machine.md:25 "It starts as `npx -y rea-agents@6.5.0 mcp`" (so the page does carry the pin; this answers the open point in L-F09-05). The skill C:\Users\Patrick\.claude\skills\reverse-engineer-anything\SKILL.md has `rea-agents@6.5.0` at lines 37, 51, 56, 71, 79 (line 66 says "version-pinned" with no literal). The Codex approval text lives in references\connection-and-recovery.md:61 "Codex tool approval is separate from REA readiness" and :19.
**Splits:** none today (same version everywhere). Seven homes hold the version; changing it means editing .claude.json, this-machine.md and five skill lines. **Ruling:** none. **Open:** none.

### L-F07-17
**Verdict:** CONFIRMED-CHANGED. Item 5 is REFUTED by the current code.
**Homes as found:**
- Item 2 (CLAUDECODE in development.md:143): confirmed, see L-F07-12 for the full list of homes.
- Item 3 (route literal): confirmed. GR\percall.py:18 `{"mcp__ch__ch_play_chart": "keys"}` against H\tools\ch_mcp\game_tools.py:71 `ROUTE_KEYS = "keys"`; no import links them (test_percall_key.py pins the literal).
- Item 5 (MAX_BRIEF_FOLDERS): I searched GR\**\*.py for `MAX_BRIEF_FOLDERS` and `MAX_BRIEF` and found nothing; `git log -S MAX_BRIEF_FOLDERS` names commit 4e37bbb, which removed it from review_dispatch.py. The "exactly one other repo" limit is gone. The prose side (MP:5, DOR:200) still describes the old way: "A brief may name a commit in the other repo if it writes that repo's root folder." The current code reads the `Review repo:` line instead. See L-F07-02 B2d.
- Item 6 (archive): MEM\archive\agent-worktree-base-is-origin-main.md:15 says reviewers use `isolation: worktree`; DOR:200 and review_dispatch.py:161 refuse it. Confirmed.
**Splits:** the item 5 prose vs code split, and the archived note vs the refusal. **Ruling:** none. **Open:** none.

### L-F08-04
**Verdict:** CONFIRMED.
**Homes as found:** GR\ref_judge.py:43-45 "The folder whose changes wake the ruling check. It is named here, not in ruling_rules.py, so a move that touches no file in it never imports that file; ruling_rules.py reads it from here." `RULINGS_DIR = "docs/agents/rulings/"`. GR\ruling_rules.py:59 `from ref_judge import RULINGS_DIR`; :94 `FORMAT_PAGE = RULINGS_DIR + "README.md"`. Text copies: ruling_rules.py:1 docstring "docs/agents/rulings/", C:\Users\Patrick\.claude\tools\session_cards\common.py:53 comment, writer-brief.md:67 "`## Confirmed` in Hydra's `docs/agents/rulings/`". Other Hydra mentions: H\tools\compare_db.py:26 and build_slot.ps1:74 name rulings files in comments. No other code string for the folder in GR\ or the card tools.
**Splits:** none (one code owner, the rest are prose/comments). **Ruling:** none. **Open:** none.

### L-F08-09
**Verdict:** CONFIRMED.
**Homes as found:** GR\sync_settings.py:37-39 "The live file every Claude Code session on this machine reads. Named in full, because a worktree has no settings.json of its own." `LIVE = r"C:\Users\Patrick\.claude\settings.json"`; :40 STATE_DIR from `GATES_DIR`. GR\tests\test_docs_numbers.py:48 `SETTINGS = os.path.join(os.path.dirname(GATES_DIR), "settings.json")`; its test at :142-146 skips in a worktree ("no settings.json beside gates"). GR\README.md:108, :110, :170 name the path. H\docs\agents\this-machine.md:71 names the file for a different fact. A search of GR\*.py, *.ps1, *.sh and tools for the path string found only sync_settings.py:39 as code.
**Splits:** sync_settings.LIVE (always the live file) vs test_docs_numbers.SETTINGS (the file beside the gates folder): same file when the gates run from ~\.claude; they differ in a gates worktree, where the test skips instead of checking. **Ruling:** none. **Open:** none.

### L-F08-10
**Verdict:** CONFIRMED (K1/K4/K6, with one more split than the row names).
**Homes as found:** Folder names: common.py:119-124 `agents`, `status`; stall.py:48-51 `_stall_path` builds `stall\<session><suffix>` (exposed as `stall_path` :70 for wake.py:33,38,75); posture.py:19-22 `posture\<session>.json`; agent_stop.py:41-43 `handback_path` = `agents\<session>\<agent>.handback`; cleanup_sweep.ps1:596-600 `status *.md`, `agents *`, `stall *.json`, `posture *.json`. Agent transcript: stall.py:169-188 `_session_folder` (main transcript path minus `.jsonl`) and `_agent_transcript` (builds `<session folder>\subagents\agent-<id>.jsonl`, else walks `subagents\workflows`); percall.py:102-112 calls it with the main session's `transcript_path`; agent_stop.py:94,235,285 reads `data.get("agent_transcript_path")` from SubagentStop input. Hooks repo location: stall.py:35 `HOOKS_REPO` (dirname of gates), ruling_rules.py:61 `CARD_TOOLS` (relative to gates), session_cards common.py:43 `CLAUDE_REPO` (from HOME), sync_settings.py:35 `GATES_DIR`, tests\replay.py:42 `REPOS` (full literals).
**Splits:**
- Agent transcript: two ways to find it. stall.py and percall.py build it from the main session's path and the folder layout `subagents\agent-<id>.jsonl`; agent_stop.py takes the path Claude Code gives the stop hook. They agree while Claude Code lays the files out that way; they differ if it moves them. Whether the PreToolUse input carries the agent's own path I did not check.
- Folder names live in four writers (common.py, stall.py, posture.py, agent_stop.py) and one pruner (cleanup_sweep.ps1:596-600). The sweep prunes `*.json` in `stall` and `posture` but I did not check every suffix stall.py writes (a `LAUNCH_SUFFIX`, watch and crash files exist: wake.py:33,75, stall.py:324); suffixes that do not match `*.json` would stay behind. A run (or reading the suffix constants) would settle it.
- Hooks repo: five spellings of its location (above); same today, differ in a gates worktree.
**Ruling:** none. **Open:** the suffix question above; PreToolUse transcript field.

### L-F08-11
**Verdict:** CONFIRMED-CHANGED (a new split with the Hydra docs).
**Homes as found:** GR\posture.py:14-15 `POSTURE_PATH` (the file beside the gates folder, `model-postures.md`); :19-22 last-send record `<STATE_ROOT>/posture/<session>.json`; :16 `RESEND_MINUTES = 60  # the user's answer to Q8 of the hooks plan HX-P2`. GR\README.md:35 "`posture.py` hands the main session `..\model-postures.md`, but not on every prompt ... the posture goes out on a session's first prompt, then again once `RESEND_MINUTES` have passed since the last send, and again at once after a compaction". settings.fragment.json:117 also registers posture.py on SessionStart `compact`. cleanup_sweep.ps1:600 `posture *.json`. H\docs\agents\agent-rules.md:5 "the model-postures file (what the main session gets each prompt)".
**Splits:**
- agent-rules.md:5 says the main session gets the postures file each prompt; README:35 and posture.py send it on the first prompt, after 60 minutes, and after a compaction. They differ on every prompt between sends.
- README:75's list of state folders does not name `posture\` (the sweep and posture.py do).
- The "user's answer to Q8 of HX-P2" claim: I found no Confirmed ruling about posture resend in R\*.md (grep for "posture", "every prompt", "60 min", "hour"). A plan answer is not a Confirmed line.
**Ruling:** none. **Open:** none.

### L-F08-12
**Verdict:** CONFIRMED.
**Homes as found:** C:\Users\Patrick\.claude\tools\session_cards\common.py:30-46 `PROJECT_NAME = "C--Users-Patrick-Downloads-Hydra-hydra-test"`, `DATA_ROOT`, `DEFAULT_MEMORY`, `HYDRA_REPO`, `CARDS_ROOT = ...docs\sessions`, `CLAUDE_REPO`, `DECISIONS_LOG` (relative path), `HAND_TAGS`. Same project folder name respelled in H\tools\agent_cost.py:31 and H\tools\check_pointers.py:30. gates\session_cards_start.ps1:171 reads the folders from common.py (one owner) but :274 spells `$rel = 'docs/sessions'` and :53 `Join-Path $PSScriptRoot 'state'`; :21 comment names `state_prune.ps1`, a file that is not in the gates folder (the pruner is cleanup_sweep.ps1 `Invoke-StatePrune`, :591). writer-brief.md:25-31: DATA path spelled out, batch file as `DATA\batches\<name>`, `DATA\tags.txt`, memory index path, decision-log absolute path, card staging name; :100-101 repeat "every 5 minutes" and "20 minutes or 100 tool calls" (progress.py owns those). batch.py:5,65,71 writes `wave-<n>-batch-<m>.txt`; session_cards_start.ps1:261-262 reads them by that name; prepass.py:437 writes `tags.txt`, common.py:547 also names it, check.py:138 reads tags.
**Splits:**
- Project folder name: common.py owns it; agent_cost.py and check_pointers.py re-spell it. Same today.
- Batch file name: batch.py writes the pattern, session_cards_start.ps1 matches it by its own regex. Same today; they differ if the pattern changes.
- DECISIONS_LOG: common.py:47 is relative (`docs\audit\...`) while writer-brief.md:30 is absolute under H.
- state_prune.ps1 in session_cards_start.ps1:21 names a file that does not exist.
**Ruling:** none. **Open:** I did not open check.py's CARD_FILE regex.

### L-F08-13
**Verdict:** CONFIRMED.
**Homes as found:** GR\README.md:19 "The user ruled on 2026-10-31 that \"trying to ban specific command patterns is a losing battle because there will always be another pattern nobody thought to check.\"" The same words are in H\docs\sessions\2026-10-31-6777b546.md:25 (under the card's words section; "(15:56 UTC, asked what Anthropic advises; the delete rule was cut back)") and in H\docs\superpowers\plans\2026-11-07-build-limit.md and 2026-11-07-ch-run-timing.md. I searched R\*.md for "losing battle", "banning", "ban specific" and "command patterns": no hit.
**Splits:** README says "The user ruled"; there is no `## Confirmed` line, only a card quote. Under the card rule, "a card quote is not a filed ruling." **Ruling:** none Confirmed. **Open:** none.

### L-F08-14
**Verdict:** REFUTED as a stale name. The old installer name is deliberate fixture text for the old shim.
**Homes as found:** GR\tests\test_ref_judge.py:670-674, test `test_installer_overwrites_its_own_and_the_old_shim` writes `# git-ref-gate shim -- installed by install_git_ref_gate.ps1.` into a shim file, then checks the current installer overwrites it. GR\ref_gate.sh:2-3 "# git-ref-gate shim -- installed by __INSTALLER__. (The marker above is the old shim's too, so either installer may overwrite the other's shim.)" GR\install_ref_gate.ps1:12 `$marker = '# git-ref-gate shim'`; the only file in GR is install_ref_gate.ps1 (`ls | grep install`). The old name appears in no other file in GR (grep over py, ps1, md, sh outside state).
**Splits:** none. The test's old name is a stand-in for the old shim and the test's name says so; the installer matches only the marker, not the installer name. (Stated as my reading of the test, not run.) **Ruling:** none. **Open:** none.

## Ledger ids not reached
None. All 20 ids were read: L-F07-01, 02, 03, 04, 06, 08, 09, 10, 12, 13, 14, 15, 17 and L-F08-04, 09, 10, 11, 12, 13, 14.
