# TOOL-AUDIT-T5-F07 (finder F07, agent aedfe3c91cbb68464)

All 46 rows were read in their T3 reports and searched. Several rows ask the same question, so they are grouped into 14 families below (F-A to F-N). Every row id is named in a family. Short paths: GR = C:\Users\Patrick\.claude\gates, AG = C:\Users\Patrick\.claude\agents, MP = C:\Users\Patrick\.claude\model-postures.md, H = C:\Users\Patrick\Downloads\Hydra\hydra-test, MEM = the memory folder (not archive\). Places searched for every family: GR (hooks, README, tests), the session_cards tool folder, AG, MP, global CLAUDE.md, live settings.json, H\CLAUDE.md, H\docs\agents (with rulings), H\.claude\commands, H\tools, MEM, and the text of the hook messages. Where I found only one home, I say so with what I searched.

Rows not reached: none. Depth is thin (one home plus a search) for I8-223, I8-232, I7-147 (REA rows, partly) and I10-059; see their entries.

## F-A. Which model runs which agent type (I6-001, I6-060, I6-064, I6-144, I6-204, I7-156, I10b-109)

Question: which model does a reviewer, doc reviewer, executor, scout, planner, card writer run on, and which model does the `opus` alias mean?

Homes (each quoted):
- H\CLAUDE.md:64 "Code reviewers run on Sonnet. Read-only scouts run on Sonnet or Haiku." (rule 2, full list of types).
- H\docs\agents\agent-rules.md:8 the same line.
- MP:5 "Executors ... on Opus ... Reviewers run on Sonnet. Read-only scouts run on Sonnet or Haiku. Card writers run on Haiku. Every dispatch names its agent type as `subagent_type`: planner (Fable), executor (Opus), reviewer (Sonnet), scout (Sonnet or Haiku) or card-writer (Haiku)."
- H\docs\agents\derive-once-review.md:5 "You run on Sonnet." and :202 "Dispatch one fresh Sonnet reviewer per change."
- H\docs\agents\doc-review.md:5 "You run on Sonnet. You use no helper agents and start no background jobs".
- H\docs\agents\integrate.md:56 "a fresh Sonnet reviewer".
- H\docs\agents\audit-method.md:114 "Fixes go to executors on Opus, one worktree each".
- AG\reviewer.md:4 `model: sonnet`; AG\scout.md:4 `model: sonnet`; AG\executor.md:4 `model: opus`; AG\planner.md:4 `model: fable`; AG\card-writer.md:4 `model: haiku`. These are the code that actually sets the model.
- H\docs\agents\rulings\agents-and-workflow.md:38 (Unconfirmed, a note's words) "Sonnet card writers ran ~150K tokens each ... the user switched the rest to Haiku."
- Opus alias: C:\Users\Patrick\.claude\settings.json:3 `"ANTHROPIC_DEFAULT_OPUS_MODEL": "claude-opus-5-5"` (live value); H\docs\agents\this-machine.md:71 "sets the ANTHROPIC_DEFAULT_OPUS_MODEL entry ... to `claude-opus-5-5` ... The user picks this model by hand."; H\docs\agents\brief-preamble.md:67 `--trailer "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"` (the model's name inside the commit form; the preamble line 69 adds "Use the model line your brief gives if it differs"); GR\tests\probe_results.md:175 "a desktop session in auto mode on Opus 5.5".
- Model names and prices: H\tools\agent_cost.py:37-40 PRICES keys `claude-fable-5-1`, `claude-opus-5-5`, `claude-sonnet-5-5`, `claude-haiku-5-5` ("Source: the `claude-api` skill", a text outside the scope); H\tools\test_agent_cost.py:21-24 and :99-120 repeat the four names and hand-worked dollar literals (docstring: "worked out by hand from the price table").

What each home tells its reader:

| Home | Reader | Tells them |
|---|---|---|
| CLAUDE.md rule 2, agent-rules:8, MP:5 | main session | pick the model per type; scout may be Sonnet or Haiku |
| AG\*.md `model:` | Claude Code | the type file pins one model; scout.md pins Sonnet only |
| DOR:5/:202, DR:5, integrate:56 | reviewer / main | reviewer is Sonnet |
| settings.json:3 | Claude Code | `opus` means claude-opus-5-5 |
| this-machine:71 | main | the user picks Opus by hand |
| agent_cost.py, test | cost tool | price per model name |

Edges: exactly at the limit: not applicable. Main session against agent: the main session is not in the model list in any home; MP:5 says "You are the main session" with no model. Card writers skip the preamble: AG\card-writer.md:9 says so, and is the only place the card writer's model is set beyond MP:5/CLAUDE.md; the card-writer type file is the one that sets it. Worktree vs main checkout: not applicable.

Kinds: K1 (reviewer-is-Sonnet is in 8+ places and they agree). K6-like: the model for each type is decided in the type file (code) and described in prose in CLAUDE.md/agent-rules/MP/DOR/DR/integrate. Opus name: K1 (settings.json owns; this-machine.md, preamble:67, agent_cost.py, test_agent_cost.py copy the name). K3: the scout line says "Sonnet or Haiku" while AG\scout.md pins `sonnet` only; the choice of Haiku is made only by a dispatch that overrides it. K5: DR:5/DOR:5 "use no helper agents and start no background jobs" spells out what AG\reviewer.md (`disallowedTools: Agent`) enforces for helpers (background jobs are not enforced by any hook I found; see F-I).

Splits: none in the value for Sonnet/Opus. One line for the scout: CLAUDE.md:64 and MP:5 say Sonnet or Haiku, AG\scout.md:4 has Sonnet only.

Confirmed ruling lines: H\docs\agents\rulings\agents-and-workflow.md:12 "use haiku for these simple transcript readers" (2026-10-14); H\docs\agents\rulings\review-and-hooks.md:22 "opus, retire, retire, minutes only, ..." (2026-10-10, mentions only the word opus).

Test holding homes equal: none found for models. GR\tests\test_agent_files.py checks only `maxTurns` in the agent files (searched it for sonnet/opus/model: no hit). GR\tests\test_docs_numbers.py holds the docs' budget numbers and retired words, not models. H\tools\test_agent_cost.py holds PRICES to literals, not to settings.json.

## F-B. What a reviewer dispatch is refused for, and when a sign-off counts (I6-009, I6-057, I6-058, I8-075, I6-144 in part)

Question: which reviewer dispatches does the hook refuse, what must the brief hold, and which reviews count?

Owner code: GR\review_dispatch.py (refusals at lines 149-169, handover record at 170); GR\agent_stop.py:200-203 ("no handover record matches its brief (was it dispatched through the Agent tool?)").

Homes:
- H\docs\agents\derive-once-review.md:17 "A hook refuses a reviewer dispatch with no key, a key already on `main`, a key git does not know or more than one commit".
- DOR:200 the same list plus "and records the tip it handed over and the branches pointing at it. A reviewer launched from a Workflow script, or as any other agent type, gets no such record, so its sign-off counts for nothing." Also "leave `isolation` out ... the hook refuses a reviewer dispatch that sets `isolation`" and "the full 40-character hash".
- GR\README.md:39 "The hook refuses a brief with no commit, a commit already on `main`, a commit git does not know, a commit in a repo that has no local `main` branch, or more than one commit ... It also refuses any reviewer dispatch that sets `isolation`".
- GR\README.md:188 "That is why a review counts only when it is dispatched through the `Agent` tool with `subagent_type: reviewer`; the user approved that (plan question Q2)."
- MP:5 "A review counts only when it goes through the `Agent` tool with `subagent_type: reviewer` and the key in the brief; a reviewer from a Workflow script leaves no verdict."
- MEM\git-traps-in-shared-checkout.md:17 "A reviewer never sets `isolation` either ... a hook refuses a reviewer dispatch that sets `isolation` (`docs/agents/derive-once-review.md`, step 4)." (MEM\MEMORY.md:12 repeats "reviewers never set isolation".)
- GR\review_dispatch.py:24-27 and :20-21 the refusal texts ("leave isolation out, and name the branch's own worktree"; "name in the brief the full 40-hex hash of the one branch tip to review, and write the base as the word main").
- AG\reviewer.md (tail): "When you stop, a hook records your verdict from git, not from your report."
- GR\tests\test_review_dispatch.py:79-92 holds the isolation refusal and the pass for other agent types.

What each says:

| Home | Reader | Tells them |
|---|---|---|
| DOR:17 | reviewer | the hook refuses 4 cases; a brief with the first two means the hook was bypassed |
| DOR:200 | main | omit isolation; one key; Workflow reviewer counts for nothing |
| README:39 | main/maintainer | 5 refusal cases incl. no local `main`; isolation refused whatever the brief holds |
| review_dispatch.py:155-159 | hook | a keyless brief is refused only when `_gated(...)` is true; else passes |
| MP:5 | main | Workflow reviewer leaves no verdict |
| MEM git-traps:17 | main | same as DOR:200 |

Edges: exactly at the limit: "more than one commit" is refused (code `len(keys) > 1`); a 64-hex run is ignored, not refused (README:39 says so; DOR does not). Main against agent: the hook runs on the main session's Agent call only. Card writers: not reviewers; no home. Worktree vs main checkout: README:39 and DOR:200 allow a key in the other repo if the brief names its root; DOR:17 says the key is "a commit on the change's own branch" with no mention of the other repo.

Kinds: K1 (the refusal list is in 3 prose places); K2 (split below); K5 (DOR:17 and DOR:200 spell out the hook's rule in a doc agents read, instead of pointing at review_dispatch.py; README:39 does the same).

Splits (own lines):
- SPLIT B1: GR\README.md:39 says the hook "refuses a brief with no commit" without condition. GR\review_dispatch.py:155-159 refuses a keyless brief only when the repo has a review marker on `main` (`_gated`); otherwise it returns None (passes). DOR:17 and DOR:200 also say "no key" is refused unconditionally.
- SPLIT B2: DOR:17 and DOR:200 list four refusals; README:39 and the code (`_key_problem`, line 108-110) also refuse a repo with no local `main`. DOR omits it.
- SPLIT B3 (older text, outside my scope list but cited by MEM): MEM\archive\agent-worktree-base-is-origin-main.md:15 says "Reviewers still use `isolation: worktree` as derive-once-review.md says"; DOR:200 and the code now refuse it. It is in archive, which the brief excludes, so I report only that it exists.

Rulings: H\docs\agents\rulings\review-and-hooks.md:20 "i asked for the review agent to fix its findings directly, then we're done." (2026-10-10); :19 "a hand-back with no sign-off line records CLEAN and tells the main session." Neither speaks to isolation or Workflow reviewers. The Workflow-reviewer limit rests on plan question Q2 (GR\README.md:188; hooks-from-scratch plan), which I found no `## Confirmed` line for (searched review-and-hooks.md and agents-and-workflow.md for "workflow": only agents-and-workflow.md:23 and :28, about parallel work and workflows as the default).

Test holding homes equal: GR\tests\test_review_dispatch.py pins the code's refusals, but nothing compares the three prose lists to the code.

## F-C. Hydra GUI MCP: registration, tool list, no-session message, per-tool rules (I7-032, I7-042, I10-059, I10-070)

Homes:
- Registration: H\.mcp.json (`"hydra"`: `py -I tools/hydra_ui_mcp.py`); H\docs\agents\ui-testing.md:181 "registered in `.mcp.json` at the repo root, and its code is `tools/hydra_ui_mcp.py`"; H\tools\hydra_ui_mcp.py:3 "see .mcp.json at the repo root".
- Tool list: H\tools\hydra_ui_mcp.py:763-879 (owner; line 15 "the helper owns ... the tool list"); H\docs\agents\ui-testing.md:209 "The tool list in `tools/hydra_ui_mcp.py` is the owner; this table is a copy of it." (table :213-228, 16 tools); H\tools\test_hydra_ui_mcp.py:51-55 TOOL_NAMES (17 names, including `hydra_build_pass`) with test at :715-724 "test_the_sdk_client_lists_every_tool" (assertEqual of the live list to TOOL_NAMES).
- No-session text: H\tools\hydra_ui_mcp.py:76 `NO_SESSION = "no session; call hydra_start"`; ui-testing.md:189 "every tool answers `no session; call hydra_start`" (quoted, partly cut in my view).
- Build pass tool: GR\percall.py:20 `BUILD_PASS_TOOL = "mcp__hydra__hydra_build_pass"` and :108-111 refusal text ("Agents never build, so hydra_build_pass is refused to every agent"); GR\README.md:15; H\CLAUDE.md:76 (rule 14); agent-rules.md:20 and :27; hydra_ui_mcp.py tool text "Main sessions only".

What each says: ui-testing.md table tells a reader 16 tools; the server and its test say 17; the percall hook names the 17th by its full name; CLAUDE.md rule 14 and README:15 tell main to use it.

Edges: main against agent: tool description says "Main sessions only" (prose) while percall.py:108 enforces it (K5 for the description). Worktree vs main checkout: ui-testing.md:364 "The `hydra` server runs the build of the checkout its helper file sits in".

Kinds: K1 (registration text in 3 places; tool list copied into the doc table; the no-session message quoted in the doc; the tool name copied into percall.py). K2: the ui-testing.md table lacks `hydra_build_pass` (16 vs 17); the doc says the table "is a copy" of the list. K5: the tool description "Main sessions only" duplicates a hook's rule.

Split C1: ui-testing.md:209-228 lists 16 tools and calls itself a copy of the owner list; the owner list and TOOL_NAMES hold 17.

Test holding homes equal: H\tools\test_hydra_ui_mcp.py:715-724 (code list vs the test's own TOOL_NAMES; not the doc table). No test compares ui-testing.md to the server. I found no test holding percall.py's BUILD_PASS_TOOL equal to the server's tool name (searched GR\tests for BUILD_PASS and hydra_build_pass: GR\tests\test_percall_rewrite.py may cover the refusal; I did not open it).

## F-D. Clone Hero MCP: process layout, tool list, front tools, game-method lists, kit player (I7-062, I7-065, I7-080, I7-087, I10b-032, I10b-046)

Homes:
- Process layout: H\.mcp.json (`py tools/ch_mcp/front.py -- py tools/ch_mcp/server.py`); H\tools\ch_mcp\README.md:77 same words; H\docs\agents\clone-hero-mcp.md:3 "Claude Code starts `tools/ch_mcp/front.py` from the `ch` entry in `.mcp.json`."
- Tool list: H\docs\agents\clone-hero-mcp.md:11 "The list of tools lives in one place: `build_server()` in `tools/ch_mcp/server.py` ... The front copies that list and adds two tools of its own, `ch_reload` and `ch_call`, from `FRONT_TOOLS` in `tools/ch_mcp/front.py`." (a pointer.) Copies of names: GR\percall.py:16-18 KEY_TOOLS `mcp__ch__ch_key`, `mcp__ch__ch_press`, KEY_ROUTE_TOOLS `{"mcp__ch__ch_play_chart": "keys"}`, :65 `mcp__ch__ch_call`; H\tools\ch_mcp\server.py:142-147 and :388-391 (tool text "a hook refuses them unless the user has typed allow key run"); H\tools\ch_mcp\front.py:36-38 FRONT_INSTRUCTIONS text and :55-60 names; the `ch` server instructions block shown to every session repeats the tool list in prose.
- "nothing open": front.py:60 `NOTHING_OPEN = "nothing open"`; server.py:205 `or "nothing open"` (the worker's own literal); tests test_front.py:123, test_server.py:74/:119. The front's comment says "Any other reply counts as something open, so a changed word refuses a reload."
- Game methods: H\tools\ch_mcp\agent\kit.ts:44 ALLOWED_CALLS (comment :18-19 "every name there is listed with its dump line in tools/ch_mcp/kit_methods.json; tests/test_frida_attach.py checks both"); agent\menu.ts:40 `ALLOWED_CALLS = ["get_activeInHierarchy", "get_gameObject"]`; kit_methods.json "about": "Every game method the kit agent may call". Test: H\tools\ch_mcp\tests\test_frida_attach.py:58-122 reads each agent's ALLOWED_CALLS and checks the kit's list against kit_methods.json (`kit_methods_findings`). The kit_methods.json is for the kit agent only; the menu agent's two methods are held by the test's per-agent ALLOWED_CALLS rule, not by the json (as far as I read: the test says "Every game method an agent calls is named in that file's own ALLOWED_CALLS").
- Kit player: H\tools\ch_mcp\kit_tools.py:92 `KIT_PLAYER = "Player1"` (owner); H\docs\clone-hero\07-internals.md:739 and :759 "The kit's player is `KIT_PLAYER` in tools/ch_mcp/kit_tools.py." (pointer, no value); test_kit_tools.py:278 and :629 pin "Player1"; ruling below.

Edges: main against agent: key route is refused for both by percall (README:13). Card writers: not applicable. Worktree vs main checkout: the ch front runs `server.py` from its own checkout path; the `ch_reload` text says merged code goes live without reconnect.

Kinds: K1 (names in percall.py, prose in server.py's tool text, front.py instructions, the harness-visible server text); K6 (the front's `NOTHING_OPEN` and the worker's literal in server.py:205 are two copies of one word; a test per side pins its own copy); K1 for kit_methods.json vs kit.ts (held equal by a test).

Splits: none found in values. (Hook gap, general words: the key-route lock matches tool names and the route string by exact text, so a rename on either side of the percall/server pair would not be noticed by a test I found; GR\tests\test_percall_key.py pins the hook's own constants.)

Confirmed ruling: H\docs\agents\rulings\clone-hero.md:79 "**2026-11-06** "Keep Player1" Context: LS-D1d passed with the kit on System ... So KIT_PLAYER stays Player1, and a human launch names System in ch_kit_setup". (I have not checked whether that line sits under `## Confirmed`; it is a ruling found, not a home. It speaks to I7-087.)

Tests holding homes equal: test_frida_attach.py (kit.ts/menu.ts lists vs kit_methods.json); GR\tests\test_percall_key.py:42-43 pins percall's `_key_call` names (no test compares them to the server's tool names).

## F-E. How a call is routed between main and subagent hooks (I9-066, I9-071, I9-090, I9-182)

Owner: GR\percall.py (`_subagent`, `_main_session`; docstring lines 1-6). The code order in a subagent: handback save, `progress.check`, build-pass refusal, then for shell calls `delete_rule`, library flag, `bash_windows_rule`, `media_rule`, then the key check (lines 99-123). In main: library flag, key check, `bash_windows_rule`, then media (ask), then `stall.run` (lines 126-139).

Homes:
- GR\percall.py docstring lines 1-6.
- GR\README.md:11 spells out the same routing in prose, both sides.
- GR\delete_rule.py:1-3 "percall.py calls check() for a subagent's Bash or PowerShell call only."
- GR\tests\test_delete_rule.py:1-10 (the I9-066 row) "The main session never reaches this code".
- GR\tests\test_media_rule.py:187-231 and the media_rule docstring: main gets `ask`, subagent gets `deny`.
- GR\tests\test_progress.py:295-341 routing test.
- GR\tests\replay.py:1-26 and :203-250 reimplements the routing by hand to replay old calls: "shell call in the main session: percall._library_refusal; main-session deletes are the fragment's ask rules (plan Q4)". It also says: "Every Bash call, main or sub, also goes through bash_windows_rule alone ... because the main-session route above does not reach that rule."
- H\docs\agents\brief-preamble.md:43 describes who the delete rule judges.

Splits:
- SPLIT E1: replay.py:23-24 says the main-session route "does not reach" `bash_windows_rule`; GR\percall.py:126-129 shows `_main_session` runs `bash_windows_rule.check` and GR\README.md:11 and :27 say it refuses in the main session and every agent. Replay's route for a main shell call is `_library_refusal` only; the real main route also runs the key check, bash_windows_rule and media_rule. Replay is therefore a second, older copy of the routing (K2/K6).
- SPLIT E2 (worktree edge): see F-J, the delete rule's roots.

Kinds: K1 (routing described in percall docstring, README:11, test docstrings); K6 (replay.py decides routing in its own code, with the old rules' text as OLD_RULES).

Tests holding homes equal: test_progress.py:295-341 and test_media_rule.py:187-231 call `percall.body` (they test the owner). I found none that holds replay.py's routing equal to percall's.

## F-F. First-call notice, agent id shape and who may write a status line (I9-019, I9-039, I9-077, I9-089, I8-065 in part)

Owner for the notice: GR\progress.py:87-90 "Your agent id is %s. Your status file is %s. Write a status line with this call, alone in its own call: %s". Id shape code: GR\common.py:113-116 `check_id` (refuses "", ".", "..", and any "/", "\\", ":"); GR\progress.py:26 `_STATUS_CALL` accepts only `[A-Za-z0-9_-]+` for the id in a status call; GR\status_append.ps1:26-41 accepts an id only if a record file exists under any session.

Homes:
- H\docs\agents\brief-preamble.md:9 "Its first reply to your first tool call names your id and your status file"; :17 "The name in the call must be your own agent id ... It refuses a name that has no agent record"; :19 "The status-call check in `progress.py` owns the exact shape."
- GR\status_append.ps1:5 "The call shape: the brief preamble's 'Status lines' section." and :9-10 "progress.py owns how a status write is recognised".
- GR\status_append.ps1:11 `$StateRoot = 'C:\Users\Patrick\.claude\gates\state'` (a copy of GR\common.py:10 STATE_ROOT, which is computed from the file's own folder).
- GR\progress.py:22 STATUS_PREFIX (path of the helper).
- Tests: test_common.py:167-171 (".." and "a/b" refused), test_agent_stop.py:28-32 (fixture ids `a0123456789abcdef`), test_percall_rewrite.py:26,:57 and test_progress.py:138-183 pin the notice text "Your agent id is a1." and the status path `<root>/status/<id>.md`.

Splits:
- SPLIT F1 (owner claim, K8): GR\status_append.ps1:5 says the preamble's Status lines section owns the call shape; preamble:19 says progress.py owns it. Both point at the other.
- SPLIT F2 (K6): three places decide what a legal agent id is: common.py:114 (anything without "/\\:" and not "." or ".."), progress.py:26 (`[A-Za-z0-9_-]+` only), status_append.ps1 (any name that has a record). An id outside `[A-Za-z0-9_-]` would pass check_id but could never write a status line.

Edges: main session against agent: the notice is for subagents only (percall routes a main call to stall; main is never counted, test_progress.py:295-341). Card writers: AG\card-writer.md:9 and tools\session_cards\writer-brief.md give their own status command (not the preamble). Worktree: not applicable.

Kinds: K8 (F1), K6 (F2), K1 (StateRoot copy; tests copy the notice wording; test_agent_stop.py:32 copies marker file names `.derive-once-gate`, `.doc-review-gate` as literals where GR\review_rules.py:18 owns them as CODE_MARKER and DOC_MARKER).

Tests holding homes equal: test_docs_numbers.py (holds docs' budget numbers and the "every N minutes" status interval to progress.py's); nothing holds status_append.ps1's StateRoot equal to common.STATE_ROOT.

## F-G. Status line every 5 minutes, and every brief carries it (I8-065, with the status rows it points at)

Homes: H\docs\agents\brief-preamble.md:13 "every 5 minutes. The form is `HH:MM done ... | next: ...`"; DOR:9 and DR:9 "every 5 minutes, form `HH:MM done ... | next: ...`"; GR\progress.py:16 `STATUS_NAG_MINUTES, STATUS_DENY_MINUTES = 5, 10` (owner); MEM\every-brief-carries-status-rule.md:11,:15 and MEM\MEMORY.md:45; MEM\relay-status-in-plain-english.md; MP:5 "briefed ... and the status rule" and "A brief names no status file and no id; it carries only the preamble's status rule."; H\CLAUDE.md rule 7 and agent-rules.md rule 7; GR\tests\test_docs_numbers.py:44-45 STATUS_EVERY regex holds docs' "every N minutes" equal to progress.py (the one test holding these homes equal).

What they say: MEM\every-brief-carries-status-rule.md:15 tells the brief writer to include a sentence about the status helper in every Agent prompt, "whatever its model or type", and says "No gate checks a brief for this sentence today". MP:5 says a brief "carries only the preamble's status rule" and names no file or id. This session's own brief (T5-finder.md) says the same through the preamble path.

Edges: Card writers skip the preamble but get the status call from the writer brief (AG\card-writer.md:9). Workflow-launched agents: the harness wraps the brief; the status rule must still be in it.

Kinds: K1 (the interval); K7 for "every brief carries the status-rule sentence": stated in MEM, MP:5 and rule 7, checked by nothing (the memory note says so itself), no home named as owner. K5: the memory note spells out the status call where the progress hook refuses an agent without status lines.

Confirmed rulings (quote): H\docs\agents\rulings\agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable" (2026-09-05); review-and-hooks.md:34 "demand status updates every 5 minutes of real time and reject any calls with no updates after 10 minutes" (2026-09-04).
Side split between two Confirmed lines: review-and-hooks.md:33 "demand status every 10 tool calls and reject the next call after 15" and :34 above are both under `## Confirmed`, dated the same day, and ask for different rules. The preamble and progress.py follow :34; GR\tests\test_docs_numbers.py:29 `RETIRED` lists `every 10 tool calls`.

## F-H. Commit trailers (I8-079, I9-137, I9-155)

Owner: GR\ref_judge.py:36 `TRAILER_KEYS = ("Task", "Agent", "Session")`; check in `missing_trailers` (lines 182-205): each value must contain an alphanumeric.

Homes:
- H\docs\agents\brief-preamble.md:65-67: the commit form with `--trailer "Task: <task id>" --trailer "Agent: <your agent id>" --trailer "Session: <session id>" --trailer "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"`; "A merge commit ... needs none".
- GR\ref_judge.py:224-231 the refusal text: `--trailer "Agent: <your agent id, or orchestrator>"`.
- GR\README.md:5 (points at the preamble, "This page does not repeat them"); :53 "A merge commit needs no trailers"; :45 stop hook uses the `Agent:` trailer to follow a reviewer's fixes.
- MEM\git-traps-in-shared-checkout.md:21 "The `Task:`, `Agent:` and `Session:` trailers must sit in the message's last paragraph, or the ref gate refuses the commit. Pass `Co-Authored-By` as a `--trailer` too"
- GR\session_cards_start.ps1:288 and a comment above it: "every commit needs the Task, Agent and Session trailers" (the message the hook prints to the main session for card commits).
- DOR:100 and DR:95 "the trailers from the preamble, with your own agent id in `Agent:`"; integrate.md:27 "the preamble's trailers".
- GR\stall.py (evidence line counts commits with the agent's `Agent:` trailer).
- Tests: GR\tests\test_review_rules.py:21 `TRAILERS = ["--trailer", "Task: T", "--trailer", "Agent: a1", "--trailer", "Session: s1"]`; GR\tests\test_ref_judge.py:22 `TRAILERS = ("Task: T-1", "Agent: a-1", "Session: s-1")`; ref_judge.py:357 selftest trailers.
- Unconfirmed ruling text: H\docs\agents\rulings\review-and-hooks.md:57 (under `## Unconfirmed`) "subagent Agent: trailer must be its own id". I found no code that checks the value is the committer's own id (ref_judge only checks non-empty alphanumeric).

What each says: preamble and ref_judge tell the committer to give the agent id; only ref_judge's refusal gives "or orchestrator" for the main session; the preamble has no word for the main session. MEM says "last paragraph", preamble says to use `--trailer`, the hook reads trailers by git's own parser.

Edges: exactly at the limit: a value with no alphanumeric character fails. Main against agent: only ref_judge says what the main session writes (`orchestrator`). Card writers: they commit nothing; the main session commits cards with the trailers (session_cards_start.ps1:288). Worktree vs main: the ref gate shim covers all worktrees of an install (README:162).

Kinds: K1 (the three keys in ~8 places, 2 test copies); K3 (the hook's `orchestrator` wording exists only in the refusal); K7/K8-ish for the "own id" rule (Unconfirmed ruling text, preamble says own id, no check).

Splits: none in the three keys. One line: preamble has "Co-Authored-By: Claude Opus 5.5" as a required trailer; ref_judge does not require it (only the three keys), and line 69 lets the brief override the model line.

Tests holding homes equal: none holds the preamble's trailer line or the tests' fixtures equal to TRAILER_KEYS (both test files hard-code their own lists).

## F-I. Parallel launch, serial only on a real dependency, comparison runs, fleet hooks (I8-092, I8-093, I8-094, I8-212, I8-223)

Homes for "independent approved tasks run at the same time":
- H\CLAUDE.md:68 rule 6 "Agents run in parallel, in workflows, while the main session merges. Never queue independent tasks one after another."
- H\docs\agents\agent-rules.md:12 the same plus "Waves of parallel agents are fine."
- MP:5 "Independent tasks run at the same time, each in its own worktree ... never one after another." and :7 (a dependent may start early only when the plan lists the blocker in `earlyStart`; cut from the blocker's executor branch; merge the blocker first).
- MEM\lean-agent-fanout.md:11 "Approved independent tasks run in parallel, each in its own worktree, launched in the same turn." and :16 "build the dependency graph and launch every task whose inputs exist"; the rows I8-092..094 sit in this note and cite "fork from X's join tip as soon as its code exists", "Serialize only on a real dependency ..., a timing run, or a merge that needs a gate or a yes", "Comparison runs launch together, each with its own scratch DB and output folder. Only a run that measures time stays alone."
- Rulings (Confirmed) H\docs\agents\rulings\agents-and-workflow.md:17 "run all the library runs in parallel instead of one after the other"; :20 "why is it so hard for you to understand parallel work is okay?"; :21 "you dont need to stay limited to 10. run as many as you can."; :23 "the plan should allow for everything to be run in parallel as a workflow with the main session merging the changes". The same file :47-48 (Unconfirmed? it follows line 32) holds "do we really need 54 merge agents?" (the batching side).
- MEM index line "~8 jobs per agent, 3-4 agents per round" (MEM\MEMORY.md:lean-agent-fanout entry).

What each says: rule 6 and MP:5: never queue independent tasks. MEM adds the timing-run exception, "launched in the same turn", "~8 jobs per agent, 3-4 agents per round". MP:7 adds the `earlyStart` rule. agent-rules.md:12 adds nothing on timing runs.

Edges: exactly at the limit: MP:5 "if a task is too big for one executor inside 100 tool calls and 20 minutes, it is two tasks" (numbers 100/20 in prose; owner is progress.py:14-15, and test_docs_numbers.py holds docs' call counts, MP is outside its list). Main vs agent: only the main session launches; agents have no helper agents. Card writers: card writer waves (tools\session_cards\writer-brief.md:33 "the main session checks every card after your wave, because parallel runs would clash on the manifest").

Kinds: K1 (parallel rule in 4 places); K7: the timing-run exception ("only a run that measures time stays alone") has one prose home (the memory note) and no check; "launch in the same turn", "~8 jobs per agent" likewise. K3: audit-method.md:114 and MP:5 both say "no two ... own the same file"/"one worktree each" (I6-144) while the owner of file ownership is each brief (preamble:41 "Touch only the files your brief owns").

I8-212 fleet hooks: H-scope search for "fleet hook" found only C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:64 "Note whether the fleet hooks held: one background executor per task, no nested background workers." No home defines "fleet hooks". The preamble (H\docs\agents\brief-preamble.md:77) says "Run every helper and long command in the foreground. Never use run_in_background", and DOR:5/DR:5 say "start no background jobs". GR\agent_stop.py:4-5 handles an agent that ends its turn waiting on background work ("parked"), so the hooks treat background work as something that happens. SPLIT I1: SKILL.md:64 treats a "background executor per task" as the held state; the preamble forbids run_in_background for every agent. (An executor dispatched in the background by the main session is a different thing from an agent's own `run_in_background`, but the skill's words do not say which; I report the two texts, not a verdict.) Kind: K3 (changed meaning of "background").

I8-223 parallel workers: C:\Users\Patrick\.claude\skills\reverse-engineer-anything\SKILL.md:310-313 "When parallel workers are available and the questions do not depend on one another, assign distinct scopes". Different skill, same idea as rule 6 but a different audience (the REA agent). One home for "distinct scopes"; I searched grep "distinct scopes", "own the same file" in skills, MP, docs\agents, AG: the only other near text is audit-method.md:114 (above). Kind: K1 loosely (same idea in a third wording).

Tests: none holds these homes equal.

## F-J. Where the delete rule lets an agent delete, and a worktree against the main checkout (I9-066 in part; found while searching)

Owner: GR\delete_rule.py:17-18 `WORKTREE_FOLDERS = (r"C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees", r"C:\Users\Patrick\.claude\worktrees")`.
Homes: preamble:43 repeats those two paths and "any worktree directly inside a `worktrees` folder of either repo"; GR\README.md:136 "Keep it under a `worktrees` folder: the delete rule trusts only those"; MP:5 "each in its own worktree under a `worktrees` folder (the delete rule trusts only those)".
SPLIT J1 (worktree against main checkout): the three prose homes say "a `worktrees` folder"; the code lists exactly two full paths. This session's worktree is C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records, a folder named `worktrees` that is in neither list (the Hydra root has `worktrees`, not `.claude\worktrees`). An agent briefed to delete inside it would be refused though the prose says it is allowed.
Confirmed ruling: H\docs\agents\rulings\review-and-hooks.md:17 chose "Keep rule, move worktrees (Recommended)" to "The delete rule lets an agent recursively delete only inside its scratch folder or a worktree that sits in a `worktrees\` folder."
Kinds: K2 (prose vs code), K1 (paths in preamble:43 copy the constant).
Test: GR\tests\test_delete_rule.py pins the code's roots; nothing compares the preamble's paths to the constant.

## F-K. CLAUDECODE as the gate for a build pass (I10-039)

Owner: GR\ref_gate.sh:8 `[ "$CLAUDECODE" = 1 ] || exit 0` (and its comment, line 5 "Outside Claude Code (CLAUDECODE is not 1) it does nothing").
Homes: H\tools\build_slot.ps1:89-90 `# True when this run is under the gate: the same test the git ref gate uses.` / `$env:CLAUDECODE -eq '1'`; H\docs\development.md:143 "The limit applies only when `CLAUDECODE` is 1" (outside the scope list; side note); GR\tests\test_ref_judge.py:49-51 sets and clears CLAUDECODE; H\tools\test_build_pass.ps1:238-242 tests values `$null` and `'0'` as off; GR\README.md:162 "guards every branch move made from Claude Code".
GR\README.md:186 says an agent's shell sees the same environment variables as the main session, so the test cannot tell a main session from an agent: build_slot's gate is "any Claude Code shell", while CLAUDE.md rule 14 and percall.py:108 say only the main session builds (the agent refusal is on the `hydra_build_pass` tool, not on the env var).
Kinds: K6 (two code places decide "is this run under Claude Code": the shell script in ref_gate.sh and the PowerShell function, each with its own test); K1.
Splits: none in value ('1').
Test holding homes equal: none; each is tested on its own.

## F-L. What the live probes found (I5-194, I9-187, I9-188)

Owner: GR\tests\probe_results.md (details and evidence).
Copies: GR\README.md:176-190 "What the probes found" restates the Claude Code versions (2.1.308, 2.1.310, 2.1.317), the `claude -p --setting-sources project` method, SubagentStop facts, maxTurns, the handback timing, `scratchpad_dir`, the env-var probe ("All 35 names and hashes matched"), the first-message hash fact, `onFailure: block`, ExitPlanMode fields and `git merge-tree --write-tree` on Git 2.55.0.windows.3. README:178 and :182 also say "The details ... are in `tests\probe_results.md`" (pointers that also state the values, so homes). Further copies: GR\README.md:128 (no agent file sets `maxTurns`), H\docs\agents\brief-preamble.md:73 ("no agent type file sets a turn cap" is in the budget paragraph), GR\tests\test_docs_numbers.py:34-36 (the test that fails when an agent file sets `maxTurns`), GR\delete_rule.py:52-59 and media_rule.py:48 and shell_output.py:89 (the code reads `scratchpad_dir`, which is absent under `claude -p`; probe_results says the rule "must refuse rather than crash when the field is missing"; delete_rule.py:59 prints "none: this call carries no scratchpad_dir"), GR\README.md:188 "the user approved that (plan question Q2)", H\docs\superpowers\plans\2026-10-25-hooks-from-scratch.md:108.
What each tells its reader: the README restates the facts as the reason for design choices; the plan line 108 (read as asked) says the numbers sit at the top of progress.py and the docs quote them, and "The user approved matching the code: 100 calls, minutes only."
Edges: main against agent: probe_results says the main session has no `agent_id`; percall.py:1-6 routes on exactly that. Card writers: not covered. Worktree: no.
Kinds: K1 (the probe facts appear twice, 7 facts); K7 for the claim "SubagentStop does not fire for a maxTurns stop" which only a probe supports (a test checks the consequence, no `maxTurns` in agent files).
Splits: none in values I could compare (Claude Code 2.1.308 vs 2.1.310 are for different probes, each stated as such).
Test holding homes equal: test_docs_numbers.py (budget numbers, retired words, README names); nothing holds README:176-190 to probe_results.md.

## F-M. Hand-off: stop launching work (I8-246)

Home: H\.claude\commands\handoff.md:9 "Stop launching work. Start no new agent and no new task until the handoff is written." One home. I searched CLAUDE.md, docs\agents (including handoff.md and agent-rules.md), MP, the gates, AG, MEM for "no new agent", "stop launching", "until the handoff": no other hit. No hook checks it. Kind: K7 only if the user counts a single prose rule as "stated in several places"; here it has one home and no check, so I report it as a single home. Edge: main session only; agents do not launch.

## F-N. REA registration and Codex approval (I7-147, I8-232)

I7-147 REA: live registration C:\Users\Patrick\.claude.json holds exactly one `rea-agents@6.5.0` (the real source); H\docs\agents\this-machine.md:25 "It starts as `npx -y rea-agents@6.5.0 mcp`" and "Its skill sits in `C:\Users\Patrick\.claude\skills\reverse-engineer-anything`"; C:\Users\Patrick\.claude\skills\reverse-engineer-anything\SKILL.md has `rea-agents@6.5.0` five times (lines 37, 51, 56, 66-71 region; and an instruction at :37 "Substitute the current supported client"). Kind: K1 (version string in 3 homes, 7 occurrences, all agree today). I found no test and no pointer-only home.
I8-232 Codex approval: single home, C:\Users\Patrick\.claude\skills\reverse-engineer-anything\references\connection-and-recovery.md:19 and :61-94. I searched this-machine.md and the gates for "approval_mode" and "approval policy": no other hit. One home.

## I10-059 and I8-065 note
I10-059 is in F-C (NO_SESSION appears in hydra_ui_mcp.py:76 and quoted in ui-testing.md:189; also a code path at hydra_ui_mcp.py:441 "no session is running", a second wording for a similar situation; K1/K6-light). I8-065 is in F-G.

## Confirmed ruling lines that speak to my rows (collected)
review-and-hooks.md:17 (delete rule/worktrees), :20 (review agent fixes directly), :24 (20 and 30 minute caps), :33/:34 (status; two different Confirmed rules), :22 (opus word); agents-and-workflow.md:8-9 (builds), :12 (Haiku readers), :17/:20/:21/:23 (parallel), :25 (status). None of them names both sides of a split I found.

## Side notes (outside my rows)
1. GR\tests\replay.py:41 `LINE_BUDGET = 3100  # the user's hard budget for hook code; only the user changes it`, while H\docs\agents\rulings\review-and-hooks.md:13 (Confirmed, 2026-10-25) says "Raise cap to 3,050". Values differ; I did not look for a later ruling.
2. H\docs\development.md:143 states the CLAUDECODE rule for builds in prose; it is outside the brief's scope list, so I did not search it for other homes.
3. GR\percall.py:18 KEY_ROUTE_TOOLS has the route string "keys" as a literal; H\tools\ch_mcp\game_tools.py has a route constant (ROUTES / ROUTE_KIT); I did not confirm a constant for "keys" or a test tying them.
4. GR\status_append.ps1:11 hard-codes the live state path; it can only be pointed elsewhere by the test variable HOOKS_TEST_STATE_ROOT (a hook gap in general terms: a status line is accepted for any id with a record under any session, not only the caller's session).
5. MEM\archive\agent-worktree-base-is-origin-main.md:15 still says reviewers use `isolation: worktree` (archive; not read as a home).
6. MP:5 says a brief "may name a commit in the other repo if it writes that repo's root folder"; README:39 and DOR:200 say the same; the code limit is `MAX_BRIEF_FOLDERS = 16` and "exactly one other repo" (review_dispatch.py:36, :94): not stated in prose.
