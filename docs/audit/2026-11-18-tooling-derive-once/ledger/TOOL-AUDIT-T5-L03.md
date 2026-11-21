# TOOL-AUDIT-T5-L03 ledger (finders F07, F08, F09; verifiers V07, V08, V09; re-send F09b/V09b)

Short paths: GR = C:\Users\Patrick\.claude\gates; H = C:\Users\Patrick\Downloads\Hydra\hydra-test; DOR = H\docs\agents\derive-once-review.md; DR = H\docs\agents\doc-review.md; MP = C:\Users\Patrick\.claude\model-postures.md; AG = C:\Users\Patrick\.claude\agents; SC = C:\Users\Patrick\.claude\tools\session_cards; TM = H\docs\agents\this-machine.md; UT = H\docs\agents\ui-testing.md; R = H\docs\agents\rulings. All verifier work was by reading; nothing was run. Rows with no verifier reading say "NOT VERIFIED" and quote the finder only ("finder only"). Where verifier and finder disagree, the verifier's reading is used and flagged.

=====================================================================
## Rows from finder F07
=====================================================================

### L-F07-01: Which model runs each agent type (reviewer, scout, executor, planner, card writer), and what the `opus` alias means
- **Source:** F07 family F-A; inventory rows I6-001, I6-060, I6-064, I6-144, I6-204, I7-156, I10b-109. Finder F07; verifier V07 (scout line only).
- **Kind:** K1, K3, K5 (F07: K6-like for type file vs prose).
- **Verdict:** CONFIRMED for the scout line (V07: "Scout model CONFIRMED (K3)"); the rest NOT VERIFIED (V07 "Not reached": "F-A model copies and Opus alias").
- **Homes:** V07 confirmed: AG\scout.md:4 `model: sonnet`; H\CLAUDE.md rule 2 "Read-only scouts run on Sonnet or Haiku." Finder only: H\CLAUDE.md:64 and agent-rules.md:8 (same line); MP:5 "Executors ... on Opus ... Reviewers run on Sonnet. Read-only scouts run on Sonnet or Haiku. Card writers run on Haiku."; DOR:5 "You run on Sonnet." and :202 "Dispatch one fresh Sonnet reviewer per change."; DR:5 "You run on Sonnet."; integrate.md:56; audit-method.md:114 "Fixes go to executors on Opus, one worktree each"; AG\reviewer.md:4 `model: sonnet`, AG\executor.md:4 `model: opus`, AG\planner.md:4 `model: fable`, AG\card-writer.md:4 `model: haiku`; C:\Users\Patrick\.claude\settings.json:3 `"ANTHROPIC_DEFAULT_OPUS_MODEL": "claude-opus-5-5"`; TM:71 "sets the ANTHROPIC_DEFAULT_OPUS_MODEL entry ... to `claude-opus-5-5` ... The user picks this model by hand."; H\docs\agents\brief-preamble.md:67 `Co-Authored-By: Claude Opus 5.5` and :69 "Use the model line your brief gives if it differs"; GR\tests\probe_results.md:175; H\tools\agent_cost.py:37-40 PRICES keys (comment "Source: the `claude-api` skill"; F09 side note 2 says the same, finder only); H\tools\test_agent_cost.py:21-24, 99-120 (four names and hand-worked dollar literals).
- **Splits:**
  - CLAUDE.md:64 and MP:5 say scouts run on "Sonnet or Haiku"; AG\scout.md:4 pins `sonnet` only. They differ when a scout is dispatched on Haiku (needs a dispatch-time override; V07: whether the harness allows one is open).
- **Ruling quotes:** Finder only: R\agents-and-workflow.md:12 "use haiku for these simple transcript readers" (2026-10-14); R\review-and-hooks.md:22 "opus, retire, retire, minutes only, ..." (2026-10-10). Neither names both sides of the scout split.
- **Tests:** None holds models equal. GR\tests\test_agent_files.py checks only `maxTurns`; H\tools\test_agent_cost.py holds PRICES to literals, not to settings.json (finder only).
- **Open:** Whether the harness lets a dispatch override the scout's model (a run would settle it). All other homes not reached by V07.
- **Hook gap:** none stated.

### L-F07-02: What a reviewer dispatch is refused for, and what a review must carry to count
- **Source:** F-B; I6-009, I6-057, I6-058, I8-075, I6-144 (part). F07, V07.
- **Kind:** K1, K2, K5.
- **Verdict:** B1 CONFIRMED; B2 CONFIRMED; B3 NOT VERIFIED (archive excluded, V07 "Not reached").
- **Homes:** V07 confirmed: GR\review_dispatch.py:155-159 `if not keys: if not _gated(...): return None`, then deny; `_gated` (:132-137) true only when the repo has a local main and a review marker on it; `_key_problem` (:108-110) refuses "no local main branch". README:39 "The hook refuses a brief with no commit". DOR:200 "A hook refuses a reviewer dispatch with no key". DOR:17 lists no key, key on main, unknown key, more than one commit (no "no local main" case). Finder only: DOR:200 also "records the tip it handed over and the branches pointing at it. A reviewer launched from a Workflow script, or as any other agent type, gets no such record, so its sign-off counts for nothing."; GR\README.md:188 "a review counts only when it is dispatched through the `Agent` tool with `subagent_type: reviewer`; the user approved that (plan question Q2)"; MP:5; MEM git-traps-in-shared-checkout.md:17; review_dispatch.py:20-27 refusal texts; GR\agent_stop.py:200-203.
- **Splits:**
  - B1: README:39 and DOR:17/:200 say a brief with no key is refused without condition; review_dispatch.py:155-159 refuses it only when the repo has a review marker on its local main (`_gated`), otherwise lets it pass. They differ in a repo with no review marker on main.
  - B2: DOR:17/:200 list four refusals; README:39 and `_key_problem` also refuse a repo with no local main. They differ in a repo with no local `main`.
  - B3 (finder only, archive): MEM\archive\agent-worktree-base-is-origin-main.md:15 says reviewers use `isolation: worktree`; DOR:200 and the code refuse it.
- **Ruling quotes:** No Confirmed ruling names both sides (V07: R\review-and-hooks.md:19 and :20 are about sign-off lines and fixes). Finder only: Workflow-reviewer limit rests on plan question Q2 (README:188), no `## Confirmed` line found (searched review-and-hooks.md and agents-and-workflow.md for "workflow").
- **Tests:** GR\tests\test_review_dispatch.py:79-92 pins the isolation refusal and pass for other agent types; nothing compares the three prose lists to the code (finder).
- **Open:** None beyond reading.
- **Hook gap:** none stated (B1 is a doc/code split, the keyless pass in an un-gated repo is the finder's own description of the code).

### L-F07-03: Hydra GUI MCP tool list (doc table vs server vs test), no-session text, build-pass tool name
- **Source:** F-C; I7-032, I7-042, I10-059, I10-070. F07, V07.
- **Kind:** K1, K2, K5.
- **Verdict:** C1 CONFIRMED-CHANGED (counts corrected by V07); the rest (registration text, NO_SESSION, BUILD_PASS_TOOL copies) NOT VERIFIED.
- **Homes:** V07 confirmed: UT table (:213-229) has 17 rows (hydra_start through hydra_run), not 16; TOOL_NAMES in H\tools\test_hydra_ui_mcp.py:51-55 holds those 17 plus hydra_build_pass = 18, not 17; hydra_ui_mcp.py:874 defines hydra_build_pass; UT:209 says the table "is a copy" of the owner list; test :724 holds the live list to TOOL_NAMES only, not to the doc. Finder only: H\.mcp.json registration; UT:181; hydra_ui_mcp.py:3; NO_SESSION at hydra_ui_mcp.py:76 `"no session; call hydra_start"` quoted at UT:189; hydra_ui_mcp.py:441 "no session is running"; GR\percall.py:20 `BUILD_PASS_TOOL = "mcp__hydra__hydra_build_pass"` and :108-111; GR\README.md:15; CLAUDE.md rule 14; tool text "Main sessions only".
- **Splits:**
  - C1: UT:209-228 calls its table a copy of the owner list and lacks `hydra_build_pass`; owner list and TOOL_NAMES hold it. (Finder said 16 vs 17; V07: 17 in the doc vs 18 in server and test. The verifier's counts win.)
- **Ruling quotes:** None found (finder: none listed for these rows).
- **Tests:** H\tools\test_hydra_ui_mcp.py:715-724 holds the live list to TOOL_NAMES, not to the doc. No test found holding percall's BUILD_PASS_TOOL equal to the server (GR\tests\test_percall_rewrite.py "may cover the refusal; I did not open it", finder).
- **Open:** none.
- **Hook gap:** none stated.

### L-F07-04: Clone Hero MCP: process layout, tool names, "nothing open" word, game-method lists, kit player
- **Source:** F-D; I7-062, I7-065, I7-080, I7-087, I10b-032, I10b-046. F07 only.
- **Kind:** K1, K6.
- **Verdict:** NOT VERIFIED (V07 "Not reached": "F-D (CH MCP, NOTHING_OPEN, KIT_PLAYER)").
- **Homes (finder only):** H\.mcp.json; H\tools\ch_mcp\README.md:77; H\docs\agents\clone-hero-mcp.md:3 and :11 "The list of tools lives in one place: `build_server()` in `tools/ch_mcp/server.py`"; GR\percall.py:16-18 KEY_TOOLS `mcp__ch__ch_key`, `mcp__ch__ch_press`, KEY_ROUTE_TOOLS `{"mcp__ch__ch_play_chart": "keys"}`, :65 `mcp__ch__ch_call`; H\tools\ch_mcp\front.py:60 `NOTHING_OPEN = "nothing open"` ("Any other reply counts as something open, so a changed word refuses a reload.") and server.py:205 `or "nothing open"`; tests test_front.py:123, test_server.py:74/:119; agent\kit.ts:44 ALLOWED_CALLS vs kit_methods.json (held by tests\test_frida_attach.py:58-122); agent\menu.ts:40 own two-name list; kit_tools.py:92 `KIT_PLAYER = "Player1"`; 07-internals.md:739,759 (pointer); test_kit_tools.py:278,:629 pin "Player1".
- **Splits:** None found by the finder.
- **Ruling quotes:** Finder only: R\clone-hero.md:79 "**2026-11-06** "Keep Player1" Context: LS-D1d passed with the kit on System ... So KIT_PLAYER stays Player1, and a human launch names System in ch_kit_setup" (finder did not check it sits under `## Confirmed`). No split to name.
- **Tests:** test_frida_attach.py (kit.ts vs kit_methods.json); GR\tests\test_percall_key.py:42-43 pins percall names only; no test compares percall names to server tool names.
- **Open:** Whether the menu agent's two methods are held by any list file (finder: held by the test's per-agent rule only).
- **Hook gap:** yes (finder only, general words): GR\percall.py. The key-route lock matches tool names and the route string by exact text, and no test ties them to the server's names.

### L-F07-05: How a call is routed between main-session and subagent hooks (replay vs percall)
- **Source:** F-E; I9-066, I9-071, I9-090, I9-182. F07, V07.
- **Kind:** K1, K2, K6.
- **Verdict:** E1 CONFIRMED (header read only; V07: "a replay run would settle whether the code matches the header (not run)").
- **Homes:** V07 confirmed: GR\tests\replay.py:19-20 "because the main-session route above does not reach that rule"; GR\percall.py:126-129 `_main_session` runs `bash_windows_rule.check(data)`; GR\README.md:11 says the same. Finder only: percall docstring :1-6; delete_rule.py:1-3; test_delete_rule.py:1-10; test_media_rule.py:187-231; test_progress.py:295-341; replay.py:203-250 reimplements routing (replay's main-session shell route is `_library_refusal` only); brief-preamble.md:43.
- **Splits:**
  - E1: replay.py says the main-session route does not reach `bash_windows_rule`; percall.py and README say it does. They differ for a main-session shell call.
  - E2 (finder): worktree edge, see L-F07-11.
- **Ruling quotes:** None found.
- **Tests:** test_progress.py:295-341 and test_media_rule.py:187-231 test percall; none holds replay's routing equal to percall's (finder).
- **Open:** I read replay's header only, not its routing code (V07).
- **Hook gap:** none stated.

### L-F07-06: Where the status folder is, what an agent id may look like, and who may write a status line
- **Source:** F-F, I9-019, I9-039, I9-077, I9-089, I8-065 (part); F08 Q1 (I1-020, I1-050, I1-053, I1-091, I2-017 part, I2-029, I1-055); F09 Q1 (I5-063, I6-005, I5-178 part). Finders F07, F08, F09; verifiers V07, V08, V09.
- **Kind:** K1, K6, K8.
- **Verdict:** F2 CONFIRMED (V07); status-folder split CONFIRMED (V08, V09); I1-055 CONFIRMED-CHANGED (V08: status_append.ps1 holds no charset, it checks an agent record exists); K8 about common.py:3 not confirmed (V08: "did not find a layout section in the README parts I read (not the whole README)"). Remaining copies NOT VERIFIED.
- **Homes:** V07/V08/V09 confirmed: GR\common.py:10 `STATE_ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "state")`, :113-116 `check_id` refuses "", ".", "..", and any of / \ :, :119-120 agents_dir, :123-124 `status_path`; GR\status_append.ps1:11 `$StateRoot = 'C:\Users\Patrick\.claude\gates\state'` (:11-14 also `status` and `agents` names; override only HOOKS_TEST_STATE_ROOT, :12); status_append.ps1:36-41 accepts a name only if an agent-folder record `<name>.json` exists; GR\progress.py:22 STATUS_PREFIX (same call text as preamble :15), :26 `_STATUS_CALL` accepts only `[A-Za-z0-9_-]+`; brief-preamble.md:13 repeats the full path; derive-once-review.md:9 and doc-review.md:9 repeat it; SC\writer-brief.md:100 repeats the call (V08, V09); GR\cleanup_sweep.ps1:597-600 names `stall`, `posture`, `*.json` (V08). Finder only: preamble :17, :19; status_append.ps1:19-21 comment "Always the live state folder, even when this script runs from a copy"; settings.fragment.json:4 and live settings.json:10 allow rule; cleanup_sweep.ps1:61 and session_cards_start.ps1:53 re-derive `$PSScriptRoot\state`; tests\lib\harness.ps1:30; tests\replay.py:280 regex (`hooks|gates`); GR\README.md:75; common.py:3 "The folder layout and the record formats are described in gates/README.md." (README has no layout section, finder, V08 could not confirm); common.py:213-251 and cleanup_sweep.ps1:27,551-584 reviews.jsonl/waivers.jsonl names; preamble card writers' own copy at writer-brief.md:100.
- **Splits:**
  - Folder: status_append.ps1:11 uses a fixed live path; common.py:10 derives the folder from its own file. Same value today; they differ when a copy of the gates folder runs elsewhere (a worktree copy). Not run.
  - Id shape (F2): common.check_id passes anything without "/\\:" and not "." or ".."; progress.py:26 accepts only `[A-Za-z0-9_-]+` in a status call; status_append.ps1 accepts any name with an agent record. An id like "a.b" passes check_id and cannot be written as a status call. Whether real ids ever have that shape is not settled by reading (V07).
- **Ruling quotes:** None naming these sides. (R\review-and-hooks.md:34 and R\agents-and-workflow.md:25 are about the status duty, see L-F07-08.)
- **Tests:** none hold status_append.ps1's folder equal to common.STATE_ROOT. test_common.py:167-171, test_agent_stop.py:28-32, test_percall_rewrite.py:26,:57, test_progress.py:138-183 pin notice wording and path shape; test_docs_numbers.py:182-190 holds preamble to `progress.STATUS_PREFIX`, "every 5 minutes", "10 minutes", forbids the old `hooks\state` (finder F09). test_status_append.ps1 uses HOOKS_TEST_STATE_ROOT (finder).
- **Open:** Whether real agent ids ever fall outside `[A-Za-z0-9_-]`; behaviour of a gates copy (a run). Whether the README has a layout section.
- **Hook gap:** yes (finder only, F07 side note 4): C:\Users\Patrick\.claude\gates\status_append.ps1 (a status line is accepted for any id with a record under any session, not only the caller's).

### L-F07-07: Who owns the status-call shape, the preamble or progress.py
- **Source:** F-F split F1; I9-039. F07, V07.
- **Kind:** K8.
- **Verdict:** CONFIRMED with a caveat (V07 F1).
- **Homes:** V07 confirmed: GR\status_append.ps1:5 "The call shape: the brief preamble's 'Status lines' section."; :9 says progress.py owns how a write is recognised. Preamble:19 (cut off in V07's view). The finder's sentence "The status-call check in `progress.py` owns the exact shape" was not seen by V07 ("that half is the finder's word").
- **Splits:**
  - status_append.ps1:5 says the preamble owns the shape; the preamble (per the finder) says progress.py owns it. Each points at the other. They differ for anyone asking where the shape is decided.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** The exact preamble sentence at :19 was not seen by V07.
- **Hook gap:** none stated.

### L-F07-08: The status-line duty (every 5 minutes, every brief carries it) and two Confirmed status rulings
- **Source:** F-G; I8-065. F07, V07.
- **Kind:** K1, K2 (between rulings), K5, K7.
- **Verdict:** The two Confirmed lines CONFIRMED (V07); the rest NOT VERIFIED.
- **Homes:** V07 confirmed: R\review-and-hooks.md:33 "demand status every 10 tool calls and reject the next call after 15" and :34 "demand status updates every 5 minutes of real time and reject any calls with no updates after 10 minutes" (both 2026-09-04, session 48cdbb34, under `## Confirmed`, which ends before `## Unconfirmed` at :37); progress.py:16 follows :34. Finder only: preamble:13; DOR:9 and DR:9; GR\progress.py:16 `STATUS_NAG_MINUTES, STATUS_DENY_MINUTES = 5, 10`; MEM every-brief-carries-status-rule.md:11,:15 ("No gate checks a brief for this sentence today"); MP:5 "A brief names no status file and no id; it carries only the preamble's status rule."; CLAUDE.md rule 7; agent-rules.md rule 7.
- **Splits:**
  - R\review-and-hooks.md:33 (status every 10 tool calls, reject after 15) and :34 (status every 5 minutes, reject after 10 minutes): both Confirmed, same day, different rules. The preamble and progress.py follow :34; GR\tests\test_docs_numbers.py:29 RETIRED lists `every 10 tool calls` (finder). Neither line names the other (V07).
- **Ruling quotes:** R\review-and-hooks.md:33 and :34 as above; neither names both sides of the other. Also R\agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable" (2026-09-05, finder only).
- **Tests:** GR\tests\test_docs_numbers.py:44-45 STATUS_EVERY regex holds docs' "every N minutes" equal to progress.py (finder). "Every brief carries the sentence": none.
- **Open:** none.
- **Hook gap:** none stated (the finder notes no gate checks a brief for the status sentence; that is a rule with no check, K7).

### L-F07-09: Commit trailers: keys, own-id rule, "orchestrator" wording, Co-Authored-By
- **Source:** F-H; I8-079, I9-137, I9-155. F07 only.
- **Kind:** K1, K3, K7.
- **Verdict:** NOT VERIFIED (V07 "Not reached": "F-H trailers").
- **Homes (finder only):** GR\ref_judge.py:36 `TRAILER_KEYS = ("Task", "Agent", "Session")`, `missing_trailers` :182-205, refusal text :224-231 `--trailer "Agent: <your agent id, or orchestrator>"`; preamble:65-67 (adds `Co-Authored-By: Claude Opus 5.5`), :69; GR\README.md:5,:45,:53; MEM git-traps-in-shared-checkout.md:21 "must sit in the message's last paragraph"; GR\session_cards_start.ps1:288; DOR:100, DR:95, integrate.md:27; tests test_review_rules.py:21, test_ref_judge.py:22 hard-code their own lists; R\review-and-hooks.md:57 (Unconfirmed) "subagent Agent: trailer must be its own id".
- **Splits:**
  - Preamble lists Co-Authored-By as a trailer to give; ref_judge requires only the three keys (finder).
  - Preamble says give your own agent id; ref_judge checks only that the value has an alphanumeric (no check it is the committer's own id).
- **Ruling quotes:** none Confirmed found; the Unconfirmed line is a note's words.
- **Tests:** none holds the preamble's trailer line or test fixtures equal to TRAILER_KEYS.
- **Open:** none.
- **Hook gap:** none stated.

### L-F07-10: Parallel launch rule, serial only on a real dependency, fleet hooks, parallel workers
- **Source:** F-I; I8-092, I8-093, I8-094, I8-212, I8-223. F07, V07 (I1 only).
- **Kind:** K1, K3, K7.
- **Verdict:** I1 CONFIRMED-CHANGED (V07: the skill line is a check-and-note, not an instruction to launch background executors, so "treats as expected" overstates it; what is real is that "fleet hooks" has no definition anywhere V07 saw, and "background executor" may mean a main-session background dispatch or an agent's own run_in_background, which reading cannot tell). All other homes NOT VERIFIED.
- **Homes:** V07 confirmed: C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:64 "Note whether the fleet hooks held: one background executor per task, no nested background workers."; preamble:77 "Never use run_in_background... You have no helper agents." Finder only: H\CLAUDE.md:68 rule 6; agent-rules.md:12; MP:5 and :7 (`earlyStart`); MEM lean-agent-fanout.md:11,:16; R\agents-and-workflow.md:17,:20,:21,:23 (Confirmed per finder), :47-48; DR:5/DOR:5 "start no background jobs"; GR\agent_stop.py:4-5; reverse-engineer-anything SKILL.md:310-313 "distinct scopes"; audit-method.md:114; MP:5 "100 tool calls and 20 minutes" (progress.py:14-15 owns the numbers).
- **Splits:**
  - SKILL.md:64 names "one background executor per task" as the thing to check; the preamble forbids an agent's run_in_background. They may differ, or may not: the skill's words do not say whether it means a main-session background dispatch (V07).
- **Ruling quotes:** Finder only: R\agents-and-workflow.md:17, :20, :21, :23 (parallel); none names a split.
- **Tests:** none.
- **Open:** What "fleet hooks" means; whether "background executor" is main-session dispatch or agent run_in_background (reading cannot tell; V07).
- **Hook gap:** none stated.

### L-F07-11: Where an agent may delete, and which worktree folders count (delete rule, preamble, cleanup sweep, real worktrees)
- **Source:** F-J (I9-066 part); F08 Q2 (I1-032, I1-039 part, I1-033); F09 Q2 (I5-186-adjacent, I6-056, I5-186). Finders F07, F08, F09; verifiers V07 (J1), V08 (Q2), V09 (Q2).
- **Kind:** K1, K2, K6 (F08 also K3).
- **Verdict:** CONFIRMED by all three verifiers (V08: "CONFIRMED, with a stronger split").
- **Homes:** V07/V08/V09 confirmed: GR\delete_rule.py:14-18 `WORKTREE_FOLDERS` = `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees` and `C:\Users\Patrick\.claude\worktrees`; `_roots` (:48-55) and `_instead` use only those; brief-preamble.md:43 gives those two paths and also "any worktree directly inside a `worktrees` folder of either repo"; MP:5 "under a `worktrees` folder (the delete rule trusts only those)"; derive-once-review.md:197 names the same two paths; GR\README.md:136 names only `C:\Users\Patrick\.claude\worktrees\<task>`; GR\cleanup_sweep.ps1:174-178 `Get-SweepPrefixes`: `<root>\.claude\worktrees` for any repo plus `<root>\worktrees` only for the repo the hook lives in (~/.claude); H\tools\new_worktree.ps1:6,29 builds `<main>\.claude\worktrees\<TaskId>` (V09); `git worktree list` shows Hydra worktrees under both `hydra-test\.claude\worktrees\` and `hydra-test\worktrees\` (V08, for example batch-rows-br1). Finder only: README:19 "the two repos' `worktrees` folders, listed at the top of `delete_rule.py`"; delete_rule.py:62 refusal text; MEM git-traps-in-shared-checkout.md:16; test_delete_rule.py:28-29,151-161,160; session_cards_start.ps1:177; tools\effort_test_guard.ps1:17,105; SC\common.py:517-519 (`<project>--claude-worktrees-*`); delete_rule.py:93 and four other drive-path spellings (I1-039); scratchpad_dir read at delete_rule.py:48-55, media_rule.py:48, shell_output.py:90 (one owner, the hook input).
- **Splits:**
  - Delete rule vs worktrees in use: this audit's worktree `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records` is in neither delete_rule's two roots nor cleanup_sweep's Hydra prefixes. Preamble:43 says "A worktree cut anywhere else is not a root" (V08), so text and code agree; the split is between code and practice. An agent in a worktree under `hydra-test\worktrees` cannot rm -rf its build folder, and the sweep never removes that worktree (V08). Both forms are in use today (V08 corrects the finder's "sit in worktrees\" as only half right).
  - Prose says "a `worktrees` folder" (preamble, MP:5, README:19); code lists exactly two full paths.
  - delete_rule accepts exactly two roots; cleanup_sweep accepts `.claude\worktrees` in any repo (V09) and `<root>\worktrees` only for ~/.claude; README:136 names only the ~/.claude one.
- **Ruling quotes:** R\review-and-hooks.md:17 (under `## Confirmed`, which starts :7), 2026-10-25, chose "Keep rule, move worktrees (Recommended)" to "The delete rule lets an agent recursively delete only inside its scratch folder or a worktree that sits in a `worktrees\` folder. Ten Hydra worktrees sit beside the repo as `C:\Users\Patrick\Downloads\Hydra\wt-*` instead. ... Which way?" It names a `worktrees\` folder in general. It does not name `hydra-test\worktrees` against `hydra-test\.claude\worktrees` or the two lists, so it names neither side of these splits (V07, V08, V09).
- **Tests:** test_delete_rule.py pins the delete rule's own two roots as literals (not linked to the code); test_h52_cleanup_sweep.ps1 pins the sweep's; no test compares the two lists or the preamble's paths to the constant (finders).
- **Open:** none beyond reading.
- **Hook gap:** none stated (the refusal is the hook working as coded).

### L-F07-12: What decides "this run is under Claude Code" (CLAUDECODE) for ref gate and build pass
- **Source:** F-K; I10-039. F07 only.
- **Kind:** K1, K6.
- **Verdict:** NOT VERIFIED (V07 "Not reached": "F-K CLAUDECODE").
- **Homes (finder only):** GR\ref_gate.sh:8 `[ "$CLAUDECODE" = 1 ] || exit 0` and :5 comment; H\tools\build_slot.ps1:89-90 "the same test the git ref gate uses" `$env:CLAUDECODE -eq '1'`; H\docs\development.md:143 "The limit applies only when `CLAUDECODE` is 1"; GR\tests\test_ref_judge.py:49-51; H\tools\test_build_pass.ps1:238-242; GR\README.md:162, :186 (an agent's shell sees the same env vars as the main session).
- **Splits:** None in value ('1'). Finder notes build_slot's gate is "any Claude Code shell" while CLAUDE.md rule 14 and percall.py:108 say only the main session builds (the agent refusal is on the tool, not the env var).
- **Ruling quotes:** None found.
- **Tests:** each side tested on its own; none holds them equal.
- **Open:** none.
- **Hook gap:** none stated.

### L-F07-13: What the live probes found (README restates probe_results.md)
- **Source:** F-L; I5-194, I9-187, I9-188. F07 only.
- **Kind:** K1, K7.
- **Verdict:** NOT VERIFIED (V07 "Not reached": "F-L probes").
- **Homes (finder only):** GR\tests\probe_results.md (owner of the details); GR\README.md:176-190, :178, :182, :128, :188; preamble:73; GR\tests\test_docs_numbers.py:34-36; GR\delete_rule.py:52-59, media_rule.py:48, shell_output.py:89 (`scratchpad_dir` absent under `claude -p`); H\docs\superpowers\plans\2026-10-25-hooks-from-scratch.md:108.
- **Splits:** None in values the finder could compare (Claude Code 2.1.308 vs 2.1.310 are for different probes).
- **Ruling quotes:** None found.
- **Tests:** test_docs_numbers.py holds budget numbers and retired words; nothing holds README:176-190 to probe_results.md.
- **Open:** "SubagentStop does not fire for a maxTurns stop" only a probe supports.
- **Hook gap:** none stated.

### L-F07-14: Hand-off: "stop launching work" rule
- **Source:** F-M; I8-246. F07 only.
- **Kind:** K7 (only if a single prose rule counts; finder reports one home).
- **Verdict:** NOT VERIFIED (V07 "Not reached": "F-M").
- **Homes (finder only):** H\.claude\commands\handoff.md:9 "Stop launching work. Start no new agent and no new task until the handoff is written." One home; no other hit in CLAUDE.md, docs\agents, MP, gates, AG, MEM. No hook checks it.
- **Splits:** None. **Ruling quotes:** None found. **Tests:** none named. **Open:** none. **Hook gap:** none stated.

### L-F07-15: REA registration and Codex approval
- **Source:** F-N; I7-147, I8-232. F07 only.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED (V07 "Not reached": "F-N").
- **Homes (finder only):** C:\Users\Patrick\.claude.json one `rea-agents@6.5.0`; TM:25 "It starts as `npx -y rea-agents@6.5.0 mcp`"; reverse-engineer-anything SKILL.md (five occurrences, lines 37, 51, 56, 66-71); Codex approval single home references\connection-and-recovery.md:19 and :61-94.
- **Splits:** None. (V09 did not find the `rea-agents@6.5.0` pin in TM:25's neighbourhood by grep of this-machine.md: see L-F09-05.)
- **Ruling quotes:** None found. **Tests:** none found. **Open:** none. **Hook gap:** none stated.

### L-F07-16: Hook code line budget (replay.py LINE_BUDGET vs ruling)
- **Source:** F07 side note 1. F07, V07.
- **Kind:** K2 (between a code constant and a ruling).
- **Verdict:** CONFIRMED (V07: values differ; "I looked for no later ruling").
- **Homes:** V07 confirmed: GR\tests\replay.py:41 `LINE_BUDGET = 3100` (finder: comment "the user's hard budget for hook code; only the user changes it"); R\review-and-hooks.md:13 (Confirmed, 2026-10-25) "Raise cap to 3,050".
- **Splits:**
  - replay.py holds 3100; the Confirmed ruling line says 3,050. They differ in what the budget is.
- **Ruling quotes:** R\review-and-hooks.md:13 "Raise cap to 3,050" (Confirmed); names one side only. No later ruling looked for.
- **Tests:** none named. **Open:** whether a later ruling changed the number. **Hook gap:** none stated.

### L-F07-17: Smaller side notes of F07 (CLAUDECODE in development.md, route literal "keys", MAX_BRIEF_FOLDERS, archived isolation note)
- **Source:** F07 side notes 2, 3, 5, 6. F07 only.
- **Kind:** K1, K7.
- **Verdict:** NOT VERIFIED (V07 "Not reached": side notes 2-6).
- **Homes (finder only):** H\docs\development.md:143 states the CLAUDECODE rule (not searched for other homes); GR\percall.py:18 KEY_ROUTE_TOOLS route string "keys" as a literal vs H\tools\ch_mcp\game_tools.py ROUTES / ROUTE_KIT (no constant for "keys" confirmed, no test tying them); MEM\archive\agent-worktree-base-is-origin-main.md:15 (see B3 in L-F07-02); MP:5, README:39 and DOR:200 say a brief may name a commit in the other repo with that repo's root, while the code limit is `MAX_BRIEF_FOLDERS = 16` and "exactly one other repo" (review_dispatch.py:36, :94), not stated in prose.
- **Splits:** None named. **Ruling quotes:** None found. **Tests:** none named. **Open:** all unreached. **Hook gap:** none stated.

=====================================================================
## Rows from finder F08
=====================================================================

### L-F08-01: Where does the main session write media freely (C:\ytdl)
- **Source:** Q3; I1-085, I1-090. F08, V08.
- **Kind:** none (single home).
- **Verdict:** CONFIRMED (single code home; V08 searched gates py, ps1, README by name).
- **Homes:** V08 confirmed: GR\media_rule.py:6 `MEDIA_ROOT = r"C:\ytdl"`. Finder only: decision log D189 (line 891) and D273 item 3 (line 1266) "writes ffmpeg or yt-dlp output under `C:\ytdl` freely; other such output outside scratch asks first in the main session and is refused in a subagent." (log, not a `## Confirmed` file); MEM ytdl-folders-not-a-project-matter.md (note). F09 (finder only): media_rule.py:47-55 implements main asks / agent denied; README:21 pointer; audit-method.md:180 names D189.
- **Splits:** None. **Ruling quotes:** None found under `## Confirmed` (finder). **Tests:** GR\tests\test_media_rule.py exists (not read). **Open:** none. **Hook gap:** none stated.

### L-F08-02: Where is a long shell result saved and how is it named
- **Source:** Q4; I1-066, I1-067. F08, V08.
- **Kind:** K1.
- **Verdict:** CONFIRMED (V08; test literals not re-opened).
- **Homes:** V08 confirmed: GR\shell_output.py:26-28 SAVED_FOLDER "shell-output", FALLBACK_FOLDER "tool-results", NAME_DIGITS 12; GR\README.md:23 repeats "a `shell-output` folder inside the session's scratch folder". Finder only: test_shell_output.py:133,148,322,377 pin names/literals; preamble:35 pointer.
- **Splits:** None. **Ruling quotes:** None found. **Tests:** tests pin literals copied from one run's output; none holds the README name equal. **Open:** none. **Hook gap:** none stated.

### L-F08-03: Which plan/measurement file a hook points at; removed card-plan path repeated 8 times
- **Source:** Q5; I1-071b, I1-082, I4-001, I4-040. F08, V08.
- **Kind:** K1 (the 8 card-tool copies only).
- **Verdict:** CONFIRMED-CHANGED (V08: pointers are not homes; only the 8 card-tool copies of the removed-plan string would be K1, and V08 did not recount the eight; a removal note is what the Hydra docs rule asks for, so it is not drift by itself).
- **Homes (finder only):** shell_output.py:6-7 plan path; bash_windows_rule.py:6-7, wake.py:5, README:23,25,27,73,178,182 point at `tests\probe_results.md`; the removed-plan string "(removed; `git show 444631e6:...`)" in writer-brief.md:5, common.py:3-7, check.py, batch.py, index.py, link_memory.py, number_rulings.py, prepass.py.
- **Splits:** None. **Ruling quotes:** None. **Tests:** none. **Open:** the count of eight. **Hook gap:** none stated.

### L-F08-04: Which folder holds rulings, and its format page
- **Source:** Q6; I3-010, I3-069. F08 only.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED (V08 "Not reached": Q6).
- **Homes (finder only):** GR\ref_judge.py:41 RULINGS_DIR (comment :39-40 "named here, not in ruling_rules.py ... ruling_rules.py reads it from here"); ruling_rules.py:59 imports it, :94 FORMAT_PAGE; ruling_rules.py:1 docstring; SC\common.py:53 comment; SC\writer-brief.md:67 "`## Confirmed` in Hydra's `docs/agents/rulings/`".
- **Splits:** None. **Ruling quotes:** None. **Tests:** none named. **Open:** all. **Hook gap:** none stated.

### L-F08-05: Which files switch the review gates on (marker names)
- **Source:** Q7; I3-023, I3-024, I3-061. F08, V08.
- **Kind:** K1, K6.
- **Verdict:** CONFIRMED (V08: same text today; no split).
- **Homes:** V08 confirmed: GR\review_rules.py:18 `CODE_MARKER, DOC_MARKER = ".derive-once-gate", ".doc-review-gate"`; GR\publish_gate.py:12 `MARKER = ".doc-review-gate"` (used :47). Finder only: README:49,138; DR:17; reviewer.md:33; test_agent_stop.py:32,57,61, test_publish.py:47, test_review_rules.py:37,286,316 use literals; test_review_dispatch.py:158-171 use `review_rules.CODE_MARKER/DOC_MARKER`.
- **Splits:** None (same text today).
- **Ruling quotes:** None. **Tests:** none ties publish_gate.MARKER to review_rules.DOC_MARKER; tests pin literals in three files. **Open:** none. **Hook gap:** none stated.

### L-F08-06: Which repo a review key is judged in, and the "approved on 2026-10-24" claim
- **Source:** Q8; I3-055, I3-056. F08, V08.
- **Kind:** K8 (possible), single code home.
- **Verdict:** CONFIRMED single code home; the approval claim is "not backed" (V08).
- **Homes:** V08 confirmed: GR\review_dispatch.py:68-69 "one exception the user approved on 2026-10-24". Confirmed lines dated 2026-10-24 V08 found: R\review-and-hooks.md:48 ("just move everything to c++"), R\tests-and-runs.md:15,16,33, R\ui-and-display.md:47-51; none is about a review key judged in another repo. Finder only: review_dispatch.py:38-39 `_PATH`, :65-99 judged_folder; DOR "For the orchestrator" (not read in full); README:138.
- **Splits:** None.
- **Ruling quotes:** None found under `## Confirmed` for the 2026-10-24 approval (V08 searched the lines above; decision log and cards not searched).
- **Tests:** none named. **Open:** the decision log and session cards were not searched for the approval. **Hook gap:** none stated.

### L-F08-07: Which Python runs the gates, and the spelling of its path (hooks vs `py`)
- **Source:** Q9; I3-088, I3-092, I3-100; F09b I5-190 part B. Finders F08, F09b; verifiers V08, V09b (V09b did not re-read the python-path claim).
- **Kind:** K1, K3 (weak), K6.
- **Verdict:** CONFIRMED (V08: counts equal, no split today); F09b's part B NOT re-checked by V09b ("I did not re-read gates README:110,114,145 or derive-once-review.md:124 for the python path claim").
- **Homes:** V08 confirmed: `pythoncore-3.14-64\python.exe` is in GR\install_ref_gate.ps1:10, GR\publish.ps1:16, GR\README.md, GR\settings.fragment.json (9) and C:\Users\Patrick\.claude\settings.json (9), GR\tests\test_shell_output.py. V08 did not re-open this-machine.md and derive-once-review.md:124. Finder only: README:110,114,145; TM:7 "Run Python as `py`. It starts the system Python 3.14.5 at `...python.exe`", and bare `python` is `C:\Python314\python.exe`; DOR:124 "with `C:\...\python.exe -I -m unittest discover...`"; sync_settings.py owns fragment-to-live; test_docs_numbers.py:142 checks live settings carry the fragment.
- **Splits:** None on the value today. Instruction spelling (finder only): TM:7 tells agents `py`; README:114,145 and DOR:124 give the full path (`py` without `-I` loads the script folder, the reason README:114 gives for `-I -S`).
- **Ruling quotes:** None. **Tests:** test_docs_numbers.py:142 (live settings carry the fragment); nothing ties publish.ps1 or install_ref_gate.ps1 to the fragment. **Open:** whether `py` and the full path are the same file on this machine (finder: not checked). **Hook gap:** none stated.

### L-F08-08: How the git shim is recognised and installed (verdict strings)
- **Source:** Q10; I3-091, I3-093. F08, V08.
- **Kind:** K1, K6.
- **Verdict:** CONFIRMED (V08).
- **Homes:** V08 confirmed: GR\ref_judge.py:31-32 ALLOW/DENY/SELFTEST_OK; GR\ref_gate.sh:15-16 repeat `ref-gate-verdict: allow` / `deny`; GR\install_ref_gate.ps1:12-13 repeat `# git-ref-gate shim` and `ref-judge selftest: allow ok, deny ok`. Finder only: tests test_ref_judge.py:614,624,629,658 (literals; :658 uses a retired installer name); install_ref_gate.ps1:17-20 path form.
- **Splits:** None today (same text).
- **Ruling quotes:** None. **Tests:** tests pin literals; the installer compares to its own constant at install time (finder). **Open:** none. **Hook gap:** none stated.

### L-F08-09: Live settings path and its sync state
- **Source:** Q11; I3-095. F08 only.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED (V08 "Not reached": Q11).
- **Homes (finder only):** GR\sync_settings.py:37-43 LIVE, STATE_DIR (comment ":37 Named in full, because a worktree has no settings.json of its own"); README.md:108,110,170; test_docs_numbers.py:142; tests\test_sync_settings.py; TM:71 names the same file for a different fact.
- **Splits:** None found. **Ruling quotes:** None. **Tests:** test_docs_numbers.py:142 and test_sync_settings.py. **Open:** all. **Hook gap:** none stated.

### L-F08-10: Where stall/watch/hand-back/agent record files live and how an agent's transcript and the hooks repo are found
- **Source:** Q12; I2-017, I2-019, I2-026, I2-028, I2-043, I2-060. F08, V08 (part).
- **Kind:** K1, K4 (candidate), K6.
- **Verdict:** partly CONFIRMED: cleanup_sweep.ps1:597-600 names `stall`, `posture`, `*.json` subfolders CONFIRMED (K1/K6, V08); the K4 claim (stall.py builds the transcript path, agent_stop.py reads it from input) NOT VERIFIED (V08: "not opened").
- **Homes:** V08 confirmed: GR\cleanup_sweep.ps1:597-600. Finder only: stall.py:48-51 stall_path, :54-68 `_session_agents`, :35 HOOKS_REPO, :174-187,307 builds `<session folder>\subagents\agent-<id>.jsonl` and `subagents\workflows\<run>\`; wake.py:25-26; agent_stop.py:39-41 handback_path, :92,196,233 reads `agent_transcript_path` from hook input; percall.py:104; status_append.ps1:45-46; common.py:119-128; progress.py:4; H\tools\agent_cost.py:160 `rglob("agent-*.jsonl")`; SC\prepass.py:162, common.py:504; shell_output.py:95. Five places for the hooks repo location: stall.py:35, SC common.CLAUDE_REPO, ruling_rules.CARD_TOOLS, sync_settings.GATES_DIR, tests\replay.py:42 REPOS.
- **Splits:** None named by the finder.
- **Ruling quotes:** None. **Tests:** none holds these equal (finder). **Open:** whether the PreToolUse input also carries the transcript path (finder: not checked). **Hook gap:** none stated.

### L-F08-11: Where the posture text and its last-send record live
- **Source:** Q13; I2-068, I2-071. F08 only.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED (V08 "Not reached": posture.py, Q13).
- **Homes (finder only):** GR\posture.py:14-15 POSTURE_PATH, :19-22 record `<STATE_ROOT>/posture/<session>.json`; README.md:35; cleanup_sweep.ps1:600 `posture\*.json`; agent-rules.md:5 pointer.
- **Splits:** None. **Ruling quotes:** None. **Tests:** none named. **Open:** all. **Hook gap:** none stated.

### L-F08-12: Session-card project folder, data folder and tool names (project name, cards root, batch names, state_prune)
- **Source:** Q14; I2-107, I2-108, I2-111, I2-112, I2-114, I3-081, I4-010, I4-011, I4-013 to I4-017, I4-036 to I4-041. F08, V08 (part).
- **Kind:** K1, K6, K8.
- **Verdict:** partly CONFIRMED. CONFIRMED (V08): SC\common.py:33-40 owns PROJECT_NAME, DATA_ROOT, DEFAULT_MEMORY, CARDS_ROOT; H\tools\agent_cost.py:31 and H\tools\check_pointers.py:30 spell the same project folder name again (K6); session_cards_start.ps1:274 `$rel = 'docs/sessions'` vs common.py:40 CARDS_ROOT (K6, same value); session_cards_start.ps1:21 names state_prune.ps1 and the file does not exist in gates (K8). NOT VERIFIED (V08): batch names, tags.txt, check.py regex, writer-brief paths.
- **Homes:** as above (V08). Finder only: writer-brief.md:25,26,28,29,30,31 (DATA path, batch shape, tags.txt, memory index path, absolute decision-log path, card name); common.py:44 DECISIONS_LOG relative vs writer-brief.md:30 absolute; common.py:92 DECISION_ENTRY; session_cards_start.ps1:52,54,171-172,177-181,243-263,318; batch.py:5,63 writes `wave-<n>-batch-<m>.txt`; prepass.py:437 writes `tags.txt` while common.py:547 also names it; check.py:38 CARD_FILE regex; check.py:242-260 publish(); Hydra CLAUDE.md "Past sessions" `docs/sessions/`; writer-brief.md:101 repeats progress.py's "20 minutes or 100 tool calls".
- **Splits:** None on today's values (finder and verifier).
- **Ruling quotes:** None. **Tests:** SC\test_session_cards.py:45 swaps these constants for temp folders; nothing holds writer-brief.md's paths equal to common.py. **Open:** the unreached rows. **Hook gap:** none stated.

### L-F08-13: README quotes a "user ruled 2026-10-31" line that is not under `## Confirmed`
- **Source:** F08 side note 1. F08 only.
- **Kind:** K7/K8 (a quoted user ruling with no Confirmed home).
- **Verdict:** NOT VERIFIED (side notes not covered by V08).
- **Homes (finder only):** GR\README.md:19 "The user ruled on 2026-10-31 that 'trying to ban specific command patterns is a losing battle ...'"; the text sits in H\docs\sessions\2026-10-31-6777b546.md and two plans; a grep of docs\agents\rulings\ found none.
- **Splits:** None. **Ruling quotes:** None found under `## Confirmed`. **Tests:** none named. **Open:** all. **Hook gap:** none stated.

### L-F08-14: Old installer name in a test's shim comment
- **Source:** F08 side note 2; also Q10 I3-093. F08 only.
- **Kind:** K8.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** GR\tests\test_ref_judge.py:658 writes a shim comment naming `install_git_ref_gate.ps1`; the file is install_ref_gate.ps1.
- **Splits:** None. **Ruling quotes:** None. **Tests:** n/a. **Open:** all. **Hook gap:** none stated.

### L-F08-15: Delete or media-output command written in a form the rule does not read
- **Source:** F08 side note 3. F08 only.
- **Kind:** hook gap.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** GR\README.md:19 already says this in general words.
- **Splits:** None. **Ruling quotes:** None found. **Tests:** none named. **Open:** n/a.
- **Hook gap:** hook gap: yes. GR\delete_rule.py and GR\media_rule.py.

### L-F08-16: Card writers carry their own copy of the status rule (5 minutes; 20 minutes / 100 calls)
- **Source:** F08 side note 4. F08 only.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** AG\card-writer.md:9; SC\writer-brief.md:99-101; owner of numbers GR\progress.py (finder says progress.py owns them).
- **Splits:** None. **Ruling quotes:** None. **Tests:** none named. **Open:** all. **Hook gap:** none stated.

=====================================================================
## Rows from finder F09 (and its re-send F09b)
=====================================================================

### L-F09-01: Where a learned Clone Hero fact goes
- **Source:** Q3; I5-009, I5-092. F09, V09.
- **Kind:** K1, K3.
- **Verdict:** CONFIRMED (V09; preamble:105 and clone-hero README:80 not re-opened).
- **Homes:** V09 confirmed: H\CLAUDE.md "updates its page in the same commit ... never in a memory note"; SC\writer-brief.md:88 "The `write-back:` line"; AG\card-writer.md:9 says the writer brief replaces the repo preamble. Finder only: preamble:105 (brief owns the page: same commit; else put it in the report); H\docs\clone-hero\README.md:80; CLAUDE.md:29.
- **Splits:** None in wording (K3: card writers write a `write-back:` header line instead of the page; main session adds it).
- **Ruling quotes:** None found. **Tests:** none found (finder searched gates\tests and Hydra tests for "memory note" / "write-back"). **Open:** none. **Hook gap:** none stated.

### L-F09-02: Where the plain-English rules live (one home, then restated)
- **Source:** Q4; I5-006, I6-083. F09, V09.
- **Kind:** K1, K8-adjacent.
- **Verdict:** CONFIRMED (V09; MP:13 not opened).
- **Homes:** V09 confirmed: DR:81 "That file is the one home of the style rules; read it ... In short, check that jargon is glossed on first use, sentences are short, and no wall of file:line bullets ..." (restates); preamble:123 states the rule too. Finder only: C:\Users\Patrick\.claude\CLAUDE.md:1 (owner); H\CLAUDE.md:1 pointer; MP:13 and :11 restate; writer-brief.md:94; GR\stall.py:32; agents scout/executor/planner.md:9; public-doc-style.md:3.
- **Splits:** None in content found. **Ruling quotes:** None. **Tests:** none. **Open:** none. **Hook gap:** none stated.

### L-F09-03: Repo name, gh account, path and version home (splax77/hydra-deluxe)
- **Source:** Q5; I5-010, I6-186, I6-224, I6-185. F09 only.
- **Kind:** K1, K4 risk (agents given a name where `git remote -v` holds it).
- **Verdict:** NOT VERIFIED (V09 Q5/Q6/Q7 "partly CONFIRMED"; the repo-name part was not addressed).
- **Homes (finder only):** H\CLAUDE.md:13; TM:15; MEM hydra-deluxe-rename.md:11 and fc-video-workflow.md:15; issue-tracker.md:3,16 "Infer the repo from `git remote -v`"; gh version/path/account TM:13 only; publish.ps1 names no repo.
- **Splits:** TM:3 says the machine wins over the page (finder). **Ruling quotes:** None. **Tests:** none. **Open:** all. **Hook gap:** none stated.

### L-F09-04: Where the Hydra version is set
- **Source:** Q6; I6-189. F09, V09.
- **Kind:** K1.
- **Verdict:** CONFIRMED for the owner; the this-machine copy NOT CONFIRMED (V09: "I did not see the literal 3.1.2 in my cut of line 17, so the copy is unconfirmed").
- **Homes:** V09 confirmed: H\CMakeLists.txt:21 `project(Hydra VERSION 3.1.2`; TM:17 carries the release-tag rule "v plus the version". Finder only: TM:17 "reads VERSION 3.1.2 today"; installer\hydra.iss, build_installer.ps1, src\core\version.h hold no literal; docs\development.md not read.
- **Splits:** None. **Ruling quotes:** None. **Tests:** none (the Hydra docs check was not run). **Open:** whether TM:17 holds the literal. **Hook gap:** none stated.

### L-F09-05: Machine facts in this-machine.md and where code repeats them (gates path, REA pin, cdb lookup, VS, CH log, others)
- **Source:** Q7; I6-190, I6-191, I6-192, I6-195, I6-196, I6-200, I6-201, I6-203; also I6-205/I6-206/I6-193 (see rows 12, 13, 15). F09, V09, V09b.
- **Kind:** K1, K4, K6, K8.
- **Verdict:** partly CONFIRMED. CONFIRMED (V09): stall_harness.py:122-124 `# The PowerShell lookup docs/agents/this-machine.md gives for cdb's folder.` then `_CDB_LOOKUP = "(Get-AppxPackage -Name Microsoft.WinDbg).InstallLocation"` (a code copy naming a page as its source, K6/K1). NOT CONFIRMED (V09): the `rea-agents@6.5.0` pin was not found by grep of this-machine.md (line 25 mentions REA only); the skill copy was not opened. Not reached: I6-148 etc. (V09 "Not reached" list).
- **Homes:** V09 confirmed as above. Finder only: TM:19, :23, :25, :27, :37-41, :53, :57-61, :67, :71; GR\common.py:10 vs status_append.ps1:11 / progress.py:22,29 for the gates folder; tools\find_cmake.ps1 called from build_cpp.ps1:43 and tools\test_build_pass.ps1:93; SKILL.md:37-79 `rea-agents@6.5.0`; H\tools\agent_cost.py:36-41.
- **Splits:** None found (TM:3 says the machine wins over the page, finder).
- **Ruling quotes:** None. **Tests:** none holds this-machine.md values equal to the machine. **Open:** whether TM:25 carries the pin; all unreached rows. **Hook gap:** none stated.

### L-F09-06: Card tools: the "local card" rule, the default repo list, and where batches/staging are built
- **Source:** Q8; I4-042, I4-043, I4-044, I4-046, I4-059, I4-059b, I4-073, I4-079, I4-090. F09, V09.
- **Kind:** K1, K3, K6, K7.
- **Verdict:** CONFIRMED (V09), with a third home the finder missed.
- **Homes:** V09 confirmed: SC\check.py:316 and SC\prepass.py:461 both `repos = a.repo or [common.HYDRA_REPO, common.CLAUDE_REPO]`; SC\common.py:258-260 owns staging; GR\session_cards_start.ps1:259,270-271 rebuild `Join-Path $cardData 'batches'` and `'staging'`; SC\batch.py:63 also builds `os.path.join(a.out, "batches")` (third home; V09 found no batches function in common.py); common.py:272-277 `marked_local` (header says yes); :280-289 `is_local_only` docstring "Every script asks here, never by its own test."; check.py:232-239 `keeps_local` = `common.marked_local(header) or common.is_local_only(out, entry)`; common.py:263-269 "The user ruled on 2026-10-31 ..."; writer-brief.md:16 "The user ruled on four kinds of content on 2026-10-31". Finder only: writer-brief.md:16-18,:26-31; common.py:43,46; link_memory.py:24 own `INDEX_FILE = "MEMORY.md"`; common.py:157-165; check.py:242-260; SC\prepass.py:386, prune_list.py.
- **Splits:**
  - S2 (V09): the local-card ruling: no `## Confirmed` line found for the "user ruled 2026-10-31" claim.
  - S3 (V09): check.py combines two local tests (`marked_local` or `is_local_only`) while common.py:280-289 says every script asks one place. They differ when a card writer marks `local: yes` versus a card already sitting in local-cards (the check keeps it local either way).
  - (Finder only) writer-brief tells the writer to decide local (K3); common.py says the check decides.
  - The default repo list `[HYDRA_REPO, CLAUDE_REPO]` is built in check.py:316 and prepass.py:461, not taken from common.
- **Ruling quotes:** None found. V09 grepped docs/agents/rulings/ only for "locally", "local card", "not about the project" and found nothing (search limited to those words).
- **Tests:** SC\test_session_cards.py builds folders through `common.*` and does not pin the writer-brief text; nothing holds writer-brief.md's folder names equal to common.py.
- **Open:** none beyond reading. **Hook gap:** none stated.

### L-F09-07: Which hook tests a merge runs, and the "shared code" list (README vs agent-rules vs precheck)
- **Source:** Q9 and F09b I5-190 part A; I5-037, I5-181 (part), I5-190. Finders F09, F09b; verifiers V09, V09b.
- **Kind:** K1, K7, K8.
- **Verdict:** CONFIRMED (V09b: "I5-190 ... CONFIRMED: the hook shared-code list is typed in gates README:156 and agent-rules.md:7. agent-rules.md:7 says precheck decides it, but tools\precheck holds nothing about gates or hooks"). V09 had called it "partly CONFIRMED" and had not seen the list in agent-rules.md:7 or MP:11 (unverified); V09b's reading supersedes that for agent-rules.md:7.
- **Homes:** V09 confirmed: GR\README.md:156 "It runs every test only when it touches what every hook shares: `common.py`, `lib\`, `settings.fragment.json` or `tests\lib\`." V09b confirmed: agent-rules.md:7 holds the same list and says `hydra_precheck.exe --tests` decides what counts as shared code; tools\precheck holds nothing about gates or hooks. Not verified (V09): MP:11. Finder only: MEM merge-tests-touch-changed-files.md (points at rule 1, no list); GR\tests\test_docs_numbers.py:142-146 `test_live_settings_carry_the_fragment`; `tools\precheck\test_pick.cpp:25-35,64-65,178` picks only tests\*.cpp.
- **Splits:**
  - agent-rules.md:7 says precheck decides the shared-code list; GR\README.md:156 states the list itself and precheck code holds none. They differ for a hook change.
- **Ruling quotes:** None naming the list. R\tests-and-runs.md:17 (quoted from the finder, not re-opened by V09b, Confirmed per finder) "why do we need to run the full suite every time a small change is made? ... all the parts you didnt change will remain the same" (2026-10-14) names no list, so it names neither side.
- **Tests:** none holds the list equal; test_docs_numbers.py checks budget numbers and retired hook names, not this list.
- **Open:** none. **Hook gap:** none stated.

### L-F09-08: Wrong owner claim in the two review-gate marker files (hooks\lib rules), and the marker's path list vs README
- **Source:** F09 side note 1; I5-186 (Q9). F09, V09.
- **Kind:** K8, K1.
- **Verdict:** CONFIRMED-CHANGED (V09). The finder said C:\Users\Patrick\.claude\hooks does not exist; V09: wrong, it exists, holding only `state\build_slots`; `hooks\lib` does not exist. The wrong-owner claim stands.
- **Homes:** V09 confirmed: H\.derive-once-gate: "Test-DeriveOnceGatedPath in the user-scope hook (hooks\lib\derive_once_rules.ps1)"; H\.doc-review-gate: "hooks\lib\doc_review_rules.ps1"; real owners GR\review_rules.py (BUILTIN_CODE line 22, markers line 18, `_is_code` line 180) and GR\publish_gate.py:12 `MARKER = ".doc-review-gate"`. V09 also confirmed: GR\README.md:138 names `gates/`, `agents/`, `tools/session_cards/` and the marker file holds the same three `gate:` lines (K1). Finder only: `Test-DeriveOnceGatedPath` appears nowhere in gates; review_rules.py:157-185 reads the marker.
- **Splits:** None between values; the claim names an owner that does not exist.
- **Ruling quotes:** None. **Tests:** none named. **Open:** none. **Hook gap:** none stated.

### L-F09-09: Scout write scope (prose only, no check)
- **Source:** Q10; I5-141. F09 only.
- **Kind:** K5/K7.
- **Verdict:** NOT VERIFIED (V09 "Not reached": "Q10 smaller rows (scout scope ...)").
- **Homes (finder only):** AG\scout.md:9 "Change no files except your status line and files in your own scratch folder"; brief-preamble.md:41 and :3; the finder's own T5 brief; no hook enforces edit scope (grep of gates\*.py for "scout" finds only test_agent_stop.py); media_rule.py and delete_rule.py cover only media writes and deletes.
- **Splits:** None. **Ruling quotes:** None. **Tests:** none found. **Open:** all.
- **Hook gap:** yes: no file named (finder: the scout write scope is not enforced by any hook it found; delete_rule.py and media_rule.py cover only deletes and media).

### L-F09-10: Smaller rows (agent-rules home, media guard, pointer check README, state layout, shim scope, domain docs, test-rubric/derivation-audit pointers, removed-file pointers, handoff name and scratch rule, run-results folder)
- **Source:** Q10; I5-146, I5-155, I5-176 (see L-F09-16), I5-178, I5-192, I5-012, I5-023, I6-019, I6-020, I6-066, I6-074, I6-083, I6-089, I6-097, I6-172. F09 only.
- **Kind:** K1 (pointers), K5.
- **Verdict:** NOT VERIFIED (V09 "Not reached": Q10 smaller rows).
- **Homes (finder only):** GR\README.md:5 and agent-rules.md:5 (pointers); media_rule.py:1-6,47-55 and README:21, audit-method.md:180; README:75 state layout vs common.py `STATE_ROOT`, `REVIEW_KINDS` :11 and status_append.ps1; README:162 and install_ref_gate.ps1:4 (shim scope); test-rubric.md named by five pages, derivation-audit.md by audit-method:164,190 and DOR:48 (pointers); DR:11,:21,:45 removed-file `git show` text; handoff.md:7,37, .claude\commands\handoff.md:11-12 and H tests\test_docs_match_code.cpp:1233 `kNotPermanentDirs` (tested at :1293-1308, agrees); premise-map.md:44 `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\`.
- **Splits:** None. **Ruling quotes:** None found. **Tests:** test_docs_match_code.cpp:1293-1308 holds the handoff skip to code; others none. **Open:** all (docs\agents\clone-hero-mcp.md not read for the run-results folder). **Hook gap:** none stated.

### L-F09-11: Where the audit's own sources (decision log, old audits, question files) are named
- **Source:** F09b I6-148. F09b, V09b.
- **Kind:** K1, K7 (weak).
- **Verdict:** CONFIRMED but only spot-checked (V09b).
- **Homes (finder only; V09b: "only spot-checked"):** audit-method.md:190, :142, :164, :176, :108, :38; agent-rules.md:17-18, :20; brief-preamble.md:89, DOR:90, DR:91, test-rubric.md:65 (fix-decisions.md); SC\writer-brief.md:30 full absolute path of the decision log; tools\doc_shots\shrink.py:18, tools\mutate\run.py:91.
- **Splits:** None. **Ruling quotes:** None found (V09b: no Confirmed line speaks to I6-148). **Tests:** none (the pointer check does not read writer-brief.md or tools\ comments). **Open:** full check not done. **Hook gap:** none stated.

### L-F09-12: Where hydra.db lives, and "snapshot_library is the only code that knows the library folder's path"
- **Source:** F09b I6-205. F09b, V09b.
- **Kind:** K1, K8, K3 (weak).
- **Verdict:** CONFIRMED (V09b).
- **Homes:** V09b confirmed: UT:195 "snapshot_library is the only code that knows the library folder's path" is contradicted by H\tools\compare_db.py:51,87 and TM:77, which also name `C:\Program Files\Hydra`. Finder only: H\src\app\config.cpp:96-98 `db_path()` (finder: TM:77's claim is true for the default path, omits the `--db` override that UT:199 mentions); TM:77; UT:346, :350, :193; hydra_ui_mcp.py:55 `DEFAULT_LIBRARY_DIR = r"C:\Program Files\Hydra"`, :72 `SNAPSHOT_FILES`; compare_db.py:51,:87.
- **Splits:**
  - UT:195 says only snapshot_library knows the library folder; compare_db.py and TM:77/UT:350 also name `C:\Program Files\Hydra`. They differ for anything reading the installed library by another route.
- **Ruling quotes:** None found. **Tests:** tools\test_hydra_ui_mcp.py exercises hydra_ui_mcp.py's own constants only; nothing holds TM and the code equal. **Open:** none. **Hook gap:** none stated.

### L-F09-13: How to read a copy of the database (hand copy of three files vs SQLite backup)
- **Source:** F09b I6-206. F09b, V09b.
- **Kind:** K2, K3, K1.
- **Verdict:** CONFIRMED (V09b: "a split on method").
- **Homes:** V09b confirmed: TM:81 says copy hydra.db plus -wal and -shm by hand; UT:195 and compare_db.py:87 use SQLite's online backup and end with one file. Finder only: TM:81 "The bare `hydra.db` alone can miss the newest writes."; tools\hydra_ui_mcp.py `copy_database` (~:108-132) (plain `shutil.copy2` when no -wal, else `sqlite3 ... ?mode=ro` `.backup()`); compare_db.py:82-88; src\store\record_store.cpp:838-847 `PRAGMA journal_mode=WAL`, library_file.cpp:139.
- **Splits:**
  - TM:81 tells an agent to copy the three files by hand; UT:195 and compare_db.py tell the tool/agent to use the backup API and end with one file. They differ when the installed Hydra is open and writing: UT:195's own words call a plain copy unsafe then.
- **Ruling quotes:** None found. **Tests:** tools\test_hydra_ui_mcp.py:375-424 tests the helper; nothing tests TM:81.
- **Open:** V09b: a run that copies the db while Hydra writes would show whether a three-file copy is ever torn. **Hook gap:** none stated.

### L-F09-14: The eight topic files under docs\agents\rulings\
- **Source:** F09b I6-215. F09b, V09b.
- **Kind:** K7 (weak), K1.
- **Verdict:** CONFIRMED, no split (V09b).
- **Homes (finder only; V09b: no split):** R\README.md:7-14 index lists agents-and-workflow, review-and-hooks, ui-and-display, code-and-engine, clone-hero, questions-and-audits, docs-and-writing, tests-and-runs; actual folder agrees; compare_db.py:26; test-rubric.md:78; ruling_rules.py takes `--topic` as an argument (no list of names); tests use `ui-and-display.md` as an example.
- **Splits:** None. **Ruling quotes:** None found. **Tests:** none holds the README list equal to the folder; tools\check_pointers.py checks pointers exist, not a list is complete. **Open:** none. **Hook gap:** none stated.

### L-F09-15: Where the Clone Hero decompile and dump root are
- **Source:** F09b I6-193. F09b, V09b.
- **Kind:** K1, K6 (weak).
- **Verdict:** CONFIRMED, no split (V09b: "I6-193 and I6-215 CONFIRMED, no split"; the ch-dumps root copies were not re-opened).
- **Homes (finder only):** H\docs\clone-hero\code\README.md:237 (decompile at `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\ghidra\ghidra_decompile_all.txt`; file exists, per finder); TM:27 pointer; H\tools\ch_mcp\build_facts.py:28 `DUMPS_ROOT`; kit_methods.json:3 and launch_patch.json:4 full dump path; tools\ch_mcp\README.md:7,12,20,58; tools\ch_mcp\tests\test_dump_facts.py:282.
- **Splits:** None. **Ruling quotes:** None found. **Tests:** none. **Open:** none. **Hook gap:** none stated.

### L-F09-16: Pointer check at session start: "always exits 0" vs the by-hand exit codes
- **Source:** F09b I5-176. F09b, V09b.
- **Kind:** K2.
- **Verdict:** CONFIRMED (V09b: "gates README:71 'always exits 0' holds only for --session-start (check_pointers.py docstring line 16)").
- **Homes:** V09b confirmed: GR\README.md:71 "always exits 0"; H\tools\check_pointers.py docstring :14-16 (by hand exits 0, 1 or 2; `--session-start` always exits 0, :293-305). Finder only: CP:63-79 `documents()`; GATES_DEFAULT/MEMORY_DEFAULT :30-31; settings.json:111 and settings.fragment.json:101.
- **Splits:**
  - README:71 says the check always exits 0; check_pointers.py exits 0 only under `--session-start` and by hand exits 1 or 2. They differ when run by hand with something broken (the form README:71 itself tells the reader to run).
- **Ruling quotes:** None found. **Tests:** no test_check_pointers found under tools\ (finder). **Open:** none. **Hook gap:** none stated.

### L-F09-17: CONTEXT.md path used by the card pre-pass, and where term words are parsed
- **Source:** F09b I4-092. F09b, V09b.
- **Kind:** K6 (weak).
- **Verdict:** CONFIRMED, only spot-checked (V09b).
- **Homes (finder only):** SC\common.py:36 `HYDRA_REPO`; SC\prepass.py:456 `--context` default `os.path.join(common.HYDRA_REPO, "CONTEXT.md")`; common.py:554-568 `build_tags` parses `^\*\*([^*]+)\*\*:`; H\tools\precheck\decisions.cpp:317-321 and check_3.cpp:534 and tests\test_docs_match_code.cpp:128 read CONTEXT.md without parsing bold terms; gates\delete_rule.py:17 spells the repo path again.
- **Splits:** None. **Ruling quotes:** None. **Tests:** SC\test_session_cards.py covers `build_tags` (not read in depth). **Open:** none. **Hook gap:** none stated.

### L-F09-18: The old card folder migrate.py reads
- **Source:** F09b I4-105. F09b, V09b.
- **Kind:** K1, K7 (weak).
- **Verdict:** CONFIRMED, only spot-checked (V09b).
- **Homes (finder only):** SC\migrate.py:98-99 (`--old-cards` default `<DATA_ROOT>\cards`, `--old-index`); common.py:34 DATA_ROOT, :42 INDEX_FILE; SC\writer-brief.md:25 full DATA path (K1); ruling_rules.py:215; check.py:225,252,282; `DATA\cards` still exists on disk.
- **Splits:** None. **Ruling quotes:** None. **Tests:** SC\test_session_cards.py not read in depth for migrate cases. **Open:** none. **Hook gap:** none stated.

### L-F09-19: Where the Opus model name is repeated (agent_cost.py price table)
- **Source:** F09 side note 2; see also L-F07-01. F09 only.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED (side note).
- **Homes (finder only):** H\tools\agent_cost.py:33-35 "the `claude-api` skill" as price source; :37-40 model ids repeat settings.json:3.
- **Splits:** None. **Ruling quotes:** None. **Tests:** none. **Open:** all. **Hook gap:** none stated.

=====================================================================
## Totals and lists
=====================================================================

- **Candidates the verifier REFUTED:** none. No verifier refuted any candidate.
- **Rows no verifier reached (NOT VERIFIED):** L-F07-04, L-F07-09, L-F07-12, L-F07-13, L-F07-14, L-F07-15, L-F07-17, L-F08-04, L-F08-09, L-F08-11, L-F08-13, L-F08-14, L-F08-15, L-F08-16, L-F09-03, L-F09-09, L-F09-10, L-F09-19. (Parts of other rows are also unreached; each row's Verdict line says which.)
- **Rows where a verifier changed the finder's reading:** L-F07-03 (17 rows in doc table, 18 in server and test, not 16 and 17), L-F07-10 (skill line is a check-and-note), L-F07-11 (both worktree folder forms in use), L-F08-03 (pointers are not homes), L-F09-04 (this-machine literal not seen), L-F09-05 (REA pin not found), L-F09-08 (hooks folder exists, hooks\lib does not), L-F07-06 (charset is not a fourth copy).
