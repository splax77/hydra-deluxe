# Exits (T6)

Each ledger row with the exit the main session applied. A sorter proposed each row's category (`sorting/`, brief `briefs/T6-sort.md`); the main session applied the exit for that category by script and owns it. Every exit 3 and every Fix pending row is checked against the files by a fresh closure checker in T7 (`closures.md`) before it counts as closed. Rows whose category a checker rejects are reopened as questions.

| Category | Rows | Exit |
|---|---|---|
| A | 79 | exit 3: the user's answer "The code wins (Recommended)" (log.md, T6 section); the doc gets fixed to point at the code |
| Q | 238 | exit 2: a question to the user (questions.md) |
| B | 33 | exit 3: the user's answer "Note is wrong (Recommended)" (log.md, T6 section); the note gets fixed or pruned |
| C | 21 | exit 3: the user's answer "Fix without asking (Recommended)" (log.md, T6 section); the pointer gets fixed |
| P | 282 | Fix pending: no split; the copy becomes a pointer in the fix plan; open until that merge |
| R | 7 | none: refuted, not a finding |

Rows still NOT VERIFIED after both verifier rounds stay open whatever their category, and are listed in summary.md.

| Row | Verdict | Category | Exit | Theme | Sides |
|---|---|---|---|---|---|
| L-F01-01 | CONFIRMED | Q | exit 2 | calls versus minutes limit | CLAUDE.md:69 rule 7 'only time counts, not calls' vs model-postures.md:5 sizing by '100 tool calls' and progress.py:14-16 counting 100/150 calls |
| L-F01-02 | CONFIRMED-CHANGED | Q | exit 2 | what happens at the call warning | writer-brief.md:101 'stop starting new cards' vs progress.py:94-100 'commit and report' vs reviewer.md:11 'stop fixing, sign off' |
| L-F01-03 | CONFIRMED | Q | exit 2 | operators inside quotes | delete_rule.py:29,77 cuts at an operator inside quotes; bash_windows_rule.py:40 does not cut in quotes |
| L-F01-04 | CONFIRMED | Q | exit 2 | context 500K warning | stall.py:36-37 'ask the user to run /compact' vs model-postures.md:15 'end the phase and let compaction happen' |
| L-F01-05 | CONFIRMED | Q | exit 2 | cleanup sweep timeout | cleanup_sweep.ps1:58 GitTimeoutSec 600 vs settings.fragment.json:81 timeout 30 |
| L-F01-06 | CONFIRMED-CHANGED | Q | exit 2 | bash length limit ruling | bash_windows_rule.py:16 '1000 # the user's number' and README:27 claim a ruling; review-and-hooks.md:40,43 sit under Unconfirmed |
| L-F01-07 | CONFIRMED | A | exit 3 |  | session_cards_start.ps1:277 'batch.py sizes a wave to the at-once limit' vs batch.py:23-26,70 BATCHES_PER_WAVE 10 |
| L-F01-08 | CONFIRMED | Q | exit 2 | status by calls or minutes | review-and-hooks.md:33 'every 10 tool calls, reject after 15' vs review-and-hooks.md:34 'every 5 minutes ... reject after 10 minutes' |
| L-F01-09 | CONFIRMED | Q | exit 2 | git exemption chained commands | BP:21 'a command that starts with git' vs progress.py:119 startswith('git ') on the whole command text |
| L-F01-10 | CONFIRMED | P | Fix pending |  | none |
| L-F01-11 | CONFIRMED-CHANGED | A | exit 3 |  | BP:73 'counted from your first tool call' vs progress.py:85-87 record may be made by the status helper first |
| L-F01-12 | CONFIRMED | P | Fix pending |  | none |
| L-F01-13 | CONFIRMED | P | Fix pending |  | none |
| L-F01-14 | CONFIRMED | P | Fix pending |  | none |
| L-F01-15 | CONFIRMED | Q | exit 2 | claimed ruling in code comment | stall.py:8-9 'user's ruling of 2026-10-25' vs no Confirmed line found |
| L-F01-16 | CONFIRMED | P | Fix pending |  | none |
| L-F01-17 | CONFIRMED | P | Fix pending |  | none |
| L-F01-18 | CONFIRMED | P | Fix pending |  | none |
| L-F01-19 | CONFIRMED | P | Fix pending |  | none |
| L-F01-20 | CONFIRMED | P | Fix pending |  | none |
| L-F01-21 | CONFIRMED | P | Fix pending |  | none |
| L-F01-22 | CONFIRMED | P | Fix pending |  | none |
| L-F01-23 | CONFIRMED | P | Fix pending |  | none |
| L-F01-24 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F01-25 | CONFIRMED | P | Fix pending |  | none |
| L-F01-26 | CONFIRMED | Q | exit 2 | status line wording unchecked | BP:15-19 says what the line should say vs progress.py:26-27 accepts any text |
| L-F02-01 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F02-02 | CONFIRMED | Q | exit 2 | docs check reads main not branch | test_docs_numbers.py:22,70 reads docs from main or GATES_HYDRA_ROOT |
| L-F02-03 | CONFIRMED | P | Fix pending |  | none |
| L-F02-04 | CONFIRMED | P | Fix pending |  | none |
| L-F02-05 | CONFIRMED | Q | exit 2 | ch tools given library root | clone-hero-mcp.md:39 'Nothing runs over the user's song library' vs tools/ch_mcp/*.py no refusal of a library root |
| L-F02-06 | CONFIRMED-CHANGED | B | exit 3 |  | memory no-full-library-runs.md:11 'about 115' and questions-describe-the-real-rule.md:24 '[N/97]' vs ui-testing.md:48 98 and tests pinning 98 |
| L-F02-07 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F02-08 | CONFIRMED | Q | exit 2 | sp compare asks resolution | sp_tools.py:317,330-332 refuses without resolution vs game_tools.py:470-487 schedule names the chart |
| L-F02-09 | CONFIRMED | P | Fix pending |  | none |
| L-F02-10 | CONFIRMED | Q | exit 2 | replay tool numbers ruling | replay_schedule.cpp:36-40 comment 'the user set it' vs no Confirmed line for 15 ms (DOR:108 'no game rule sets that margin') |
| L-F02-11 | CONFIRMED | Q | exit 2 | replay tool numbers ruling | replay_schedule.cpp:21-31 'the user chose One frame' vs no Confirmed line (code-and-engine.md:13 speaks to the leeway only) |
| L-F02-12 | CONFIRMED | Q | exit 2 | large review threshold | review-and-hooks.md:22 Confirmed 'only large items over 20 lines should go for review' vs items.h:50 kLargeRangeLines 400 |
| L-F02-13 | CONFIRMED-CHANGED | Q | exit 2 | questions per message number | audit-method.md:104 'main session's own suggestion' vs player-audit.md:102 and docs-gaps-search.md:187 stating it as the rule |
| L-F02-14 | CONFIRMED | P | Fix pending |  | none |
| L-F02-15 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F02-16 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F02-17 | CONFIRMED | P | Fix pending |  | none |
| L-F02-18 | CONFIRMED | P | Fix pending |  | none |
| L-F02-19 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F02-20 | CONFIRMED-CHANGED | Q | exit 2 | settings reload claim strength | README.md:118 'reaches every running session' vs README.md:190 and probe_results.md:49-52 'not seen directly' |
| L-F03-01 | CONFIRMED | B | exit 3 |  | memory early-fill-orange-85-d250.md:3,11 fixed slack kEarlyFillSlackMs vs model.h:108-115 one setting default 60 up to 85 |
| L-F03-02 | CONFIRMED | Q | exit 2 | squeeze window pointer | SKILL.md:10 and squeeze_scout.py:34 say graph.h; model.h:117-121 owns it; note sp-squeeze-mechanics.md:13-15 narrows callers |
| L-F03-03 | CONFIRMED | P | Fix pending |  | none |
| L-F03-04 | CONFIRMED-CHANGED | Q | exit 2 | fan-out numbers no home | lean-agent-fanout.md:3,20-21 '8 per agent, 3-4 agents' vs no doc or hook |
| L-F03-05 | CONFIRMED | B | exit 3 |  | measure-small-explain-first.md:11 quotes a hook message vs no hook carries that text in the gates |
| L-F03-06 | CONFIRMED | B | exit 3 |  | every-brief-carries-status-rule.md:3 'dispatch gate now denies briefs' vs :15 'No gate checks' and review_dispatch.py |
| L-F03-07 | CONFIRMED | P | Fix pending |  | none |
| L-F03-08 | CONFIRMED-CHANGED | Q | exit 2 | library chart count | no-full-library-runs.md:17 '18,811 charts' vs min-path-delta-proof.md:29 '18,610 charts' |
| L-F03-09 | CONFIRMED | P | Fix pending |  | none |
| L-F03-10 | CONFIRMED | C | exit 3 |  | test-rubric-examples.md:88 'engine.cpp line 2015' vs engine.cpp:2015 not a bank-rise line |
| L-F03-11 | CONFIRMED | C | exit 3 |  | fix-decisions.md:103 says test_search.cpp vs tests/test_sp_end.cpp:793,830 |
| L-F03-12 | CONFIRMED | Q | exit 2 | same goal failed twice rule | question-the-design-before-patching.md:11 vs no hook or doc |
| L-F03-13 | CONFIRMED | B | exit 3 |  | approved-work-is-not-reasked.md:13 'about 8 ms' vs code-and-engine.md:63 '5.4 ms against 13.8' |
| L-F03-14 | CONFIRMED | Q | exit 2 | no re-ask rule home | approved-work-is-not-reasked.md and MEMORY.md:5 vs no hook or doc rule |
| L-F03-15 | CONFIRMED | P | Fix pending |  | none |
| L-F03-16 | CONFIRMED-CHANGED | C | exit 3 |  | 02-note-list.md:611,631,641 and questions.md:270 'line 203, 2009' vs song.cpp:205 and :2204 |
| L-F03-17 | CONFIRMED-CHANGED | Q | exit 2 | executor worktree base rule | git-traps-in-shared-checkout.md:13,16 gives the figure; the archived note gives the cause; no hook or doc |
| L-F03-18 | CONFIRMED | P | Fix pending |  | none |
| L-F03-19 | CONFIRMED | P | Fix pending |  | none |
| L-F03-20 | CONFIRMED | Q | exit 2 | scroll to middle of list | search-before-clicking-a-song.md:11,13 never scroll vs render-checks-use-the-corpus.md:19 'Scroll to the middle' |
| L-F03-21 | CONFIRMED | P | Fix pending |  | none |
| L-F03-22 | CONFIRMED | P | Fix pending |  | none |
| L-F04-01 | CONFIRMED | P | Fix pending |  | none |
| L-F04-02 | CONFIRMED | P | Fix pending |  | none |
| L-F04-03 | CONFIRMED | P | Fix pending |  | none |
| L-F04-04 | CONFIRMED | P | Fix pending |  | none |
| L-F04-05 | CONFIRMED | Q | exit 2 | auto-compact claim at 500K | stall.py:36 text says auto-compact fires at 500K (50%); settings.json and settings.fragment.json set no auto-compact |
| L-F04-06 | CONFIRMED-CHANGED | Q | exit 2 | posture resend interval | posture.py:16 says 60 is the user's answer to Q8; plan 2026-10-31-harness-hooks-and-tools.md:89,306 says 60 is a pick awaiting an answer |
| L-F04-07 | CONFIRMED | Q | exit 2 | hook line budget | replay.py:41 hard budget 3100; Confirmed review-and-hooks.md:13 says 3,050 and :11 says exact number does not matter |
| L-F04-08 | CONFIRMED | P | Fix pending |  | none |
| L-F04-09 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F04-10 | CONFIRMED | Q | exit 2 | unclaimed pass timeout ruling | build_slot.ps1:79 says user answered 120 s; agents-and-workflow.md:35 holds it only under Unconfirmed |
| L-F04-11 | REFUTED | R | none |  | none |
| L-F04-12 | CONFIRMED | P | Fix pending |  | none |
| L-F04-13 | CONFIRMED | P | Fix pending |  | none |
| L-F04-15 | CONFIRMED-CHANGED | A | exit 3 |  | handoff.md:7 states one step of the date rule; check_handoff.py:398-413 has three steps (name, git author date, today) |
| L-F04-16 | CONFIRMED | C | exit 3 |  | 05-star-power.md:417,419 says model.h line 111; model.h:121 holds the constant; analyzing-sp-squeezes SKILL.md:10 says src/search/graph.h |
| L-F04-17 | CONFIRMED-CHANGED | A | exit 3 |  | 05-star-power.md:453-459 says 3 ms flat; rules.h:32 and rules.cpp:76 make it a hydra_rules.ini default |
| L-F04-18 | CONFIRMED-CHANGED | B | exit 3 |  | no-full-library-runs.md:17 18,811 vs min-path-delta-proof.md:29 18,610 vs stored-versions-one-rule.md:11 18,755 |
| L-F04-19 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F04-20 | CONFIRMED | A | exit 3 |  | PRE:73 and README.md:128 say the budget hook is the only limit; settings.json:4 sets CLAUDE_CODE_MAX_SUBAGENT_SPAWN_DEPTH 1 |
| L-F05-01 | CONFIRMED-CHANGED | A | exit 3 |  | plan 2026-11-12-player-stall-fixes.md:74,303 says 15 percent; quiet.py:29 QUIET_CPU_PCT = 30 |
| L-F05-02 | CONFIRMED | A | exit 3 |  | server.py:128,158 says about 6 s; quiet.py:30,46 is 5 s sample plus up to 3 s scan |
| L-F05-03 | CONFIRMED | Q | exit 2 | tool caps claimed as user ruling | quiet.py:43 cites the user's ruling of 2026-11-07 for the 3 s scan cap; no Confirmed line holds it |
| L-F05-04 | CONFIRMED-CHANGED | Q | exit 2 | tool caps claimed as user ruling | bot_run.py:64-91 comments say user answered caps; the cited plans say recommended or a guess with no ruling |
| L-F05-05 | CONFIRMED | Q | exit 2 | tool caps claimed as user ruling | kit_tools.py:94-123, frida_tools.py:39-49, trace_hits.py:67-74 comments quote user answers; no Confirmed line for most caps |
| L-F05-06 | CONFIRMED | P | Fix pending |  | none |
| L-F05-07 | CONFIRMED | Q | exit 2 | menu step Blue guess | menu_tools.py:48 says Blue is a guess pending live step LS-K3 |
| L-F05-08 | CONFIRMED | Q | exit 2 | scroll ruling and game song list | ui-and-display.md:27 (Confirmed) bars scrolling the library unasked; menu_tools.py:39 SEARCH_DOWN_CAP = 10 scrolls the game's list |
| L-F05-09 | CONFIRMED | P | Fix pending |  | none |
| L-F05-10 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F05-11 | CONFIRMED | P | Fix pending |  | none |
| L-F05-12 | CONFIRMED | Q | exit 2 | capture script click offset | report_shot.ps1:5-8 hand-measured 80 px offset; widgets.h:197 and library_toolbar.cpp:140 compute the button position |
| L-F05-13 | CONFIRMED | Q | exit 2 | capture startup wait | capture_window.ps1:56 default 12; shot.ps1:2 default 10 (W03 extra finding) |
| L-F05-14 | CONFIRMED | P | Fix pending |  | none |
| L-F05-15 | CONFIRMED | P | Fix pending |  | none |
| L-F05-16 | CONFIRMED | P | Fix pending |  | none |
| L-F05-17 | CONFIRMED | P | Fix pending |  | none |
| L-F05-18 | CONFIRMED | P | Fix pending |  | none |
| L-F05-19 | CONFIRMED | P | Fix pending |  | none |
| L-F06-01 | CONFIRMED | P | Fix pending |  | none |
| L-F06-02 | CONFIRMED | P | Fix pending |  | none |
| L-F06-03 | CONFIRMED | P | Fix pending |  | none |
| L-F06-04 | CONFIRMED | A | exit 3 |  | delete_rule.py:17-18 trusts two exact folders; model-postures.md:5 and README.md:136 read as any worktrees folder |
| L-F06-05 | CONFIRMED | B | exit 3 |  | model-postures.md:11 says a brief carries no status file or id; memory note every-brief-carries-status-rule.md:3 says it must |
| L-F06-06 | CONFIRMED | Q | exit 2 | agent id check rules | progress.py:23-26 regex and own-id test; common.py:113-116 check_id; status_append.ps1:25-39 any recorded name |
| L-F06-07 | CONFIRMED | A | exit 3 |  | PRE:65 names merge commits only; ref_judge.py:225-232,188 also exempts commits origin or any local branch has |
| L-F06-08 | CONFIRMED-CHANGED | Q | exit 2 | main session detection | percall.py:163 tests only no agent_id; stall.py:421 and wake.py:97 test agent_id or no session_id |
| L-F06-09 | REFUTED | R | none |  | none |
| L-F06-10 | CONFIRMED | A | exit 3 |  | README.md:39 and derive-once-review.md:17,200 read as unconditional; review_dispatch.py:155-159 refuses only a gated repo |
| L-F06-11 | CONFIRMED | P | Fix pending |  | none |
| L-F06-12 | CONFIRMED-CHANGED | Q | exit 2 | agent transcript count | prepass.py:143-170 counts direct agent files; agent_cost.py:160 rglob counts all; stall.py:175-185 own layout |
| L-F06-13 | CONFIRMED | Q | exit 2 | scout model and tools | scout.md:4 model sonnet and :5 lists Edit, Write; rule 2 and model-postures.md:5 say Sonnet or Haiku, read-only |
| L-F06-14 | CONFIRMED | B | exit 3 |  | lean-agent-fanout.md:21 and MEMORY.md:39 say 3-4 agents; agents-and-workflow.md:21 Confirmed says run as many as you can |
| L-F06-15 | CONFIRMED | Q | exit 2 | what a brief carries | model-postures.md:5 omits worktree and scratch folder; PRE:3 omits the status rule |
| L-F06-16 | CONFIRMED-CHANGED | A | exit 3 |  | model-postures.md:11 says nudge once then stop; stall.py:124-125,143-144 says stop and re-dispatch |
| L-F06-17 | CONFIRMED-CHANGED | A | exit 3 |  | model-postures.md:5 says that repo's root folder; review_dispatch.py:99-119 needs the branch's worktree top folder; judged_folder no longer exists |
| L-F06-18 | CONFIRMED | P | Fix pending |  | none |
| L-F06-19 | CONFIRMED | P | Fix pending |  | none |
| L-F07-01 | CONFIRMED | Q | exit 2 | scout model Haiku or Sonnet | CLAUDE.md rule 2 and model-postures.md:5 allow scouts on Sonnet or Haiku; agents/scout.md:4 pins `model: sonnet`; preamble:67 hardcodes the Opus trailer. |
| L-F07-02 | CONFIRMED-CHANGED | Q | exit 2 | review dispatch refusal docs | derive-once-review.md:17,:200 and model-postures.md:5 describe fewer refusals and old rules; review_dispatch.py:152-186 refuses more; archived memory note:15 says isolation worktree. |
| L-F07-03 | CONFIRMED-CHANGED | A | exit 3 |  | ui-testing.md:209-229 table lists 17 tools and calls itself a copy; hydra_ui_mcp.py:874 and test_hydra_ui_mcp.py:51-55 hold 18 with hydra_build_pass. |
| L-F07-04 | CONFIRMED | P | Fix pending |  | none |
| L-F07-05 | CONFIRMED | A | exit 3 |  | tests/replay.py:19-20 says the main-session route does not reach bash_windows_rule; percall.py:126-129 runs bash_windows_rule.check in the main-session route. |
| L-F07-06 | CONFIRMED | Q | exit 2 | agent id shape and status writers | common.py:113-116 check_id passes "a.b"; progress.py:26 accepts only [A-Za-z0-9_-]+; status_append.ps1:36-41 accepts any id with a record; preamble says own id only. |
| L-F07-07 | CONFIRMED | A | exit 3 |  | status_append.ps1:5 says the preamble owns the call shape; progress.py:22-26 recognises the call and status_append.ps1:9 says progress.py owns recognition. |
| L-F07-08 | CONFIRMED | Q | exit 2 | status line timing | review-and-hooks.md:33 "every 10 tool calls ... reject after 15" vs :34 "every 5 minutes ... reject after 10 minutes", both under ## Confirmed. |
| L-F07-09 | CONFIRMED | Q | exit 2 | commit trailer own id check | brief-preamble.md:67-69 asks for Co-Authored-By and your own agent id; ref_judge.py:40,242-252 requires Task/Agent/Session with any alphanumeric value and offers "orchestrator". |
| L-F07-10 | CONFIRMED-CHANGED | Q | exit 2 | fleet hooks and background executor | analyzing-sp-squeezes/SKILL.md:64 names "one background executor per task" and "fleet hooks"; preamble:77 forbids an agent's run_in_background. |
| L-F07-11 | CONFIRMED | Q | exit 2 | worktree folder location | delete_rule.py:14-18 trusts two worktree roots; cleanup_sweep.ps1:174-178 trusts .claude\worktrees in any repo plus <root>\worktrees for ~/.claude only; hydra-test\worktrees is in neither. |
| L-F07-12 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F07-13 | CONFIRMED | P | Fix pending |  | none |
| L-F07-14 | CONFIRMED | P | Fix pending |  | none |
| L-F07-15 | CONFIRMED | P | Fix pending |  | none |
| L-F07-16 | CONFIRMED | Q | exit 2 | hook code line budget | tests/replay.py:41 LINE_BUDGET = 3100; rulings/review-and-hooks.md:13 (Confirmed) "Raise cap to 3,050". |
| L-F07-17 | CONFIRMED-CHANGED | Q | exit 2 | review dispatch refusal docs | MP:5 and DOR:200 describe the removed other-repo folder rule; review_dispatch.py now reads the Review repo line (MAX_BRIEF_FOLDERS gone); archived note:15 vs refusal. |
| L-F08-01 | CONFIRMED | P | Fix pending |  | none |
| L-F08-02 | CONFIRMED | P | Fix pending |  | none |
| L-F08-03 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F08-04 | CONFIRMED | P | Fix pending |  | none |
| L-F08-05 | CONFIRMED | P | Fix pending |  | none |
| L-F08-06 | CONFIRMED | Q | exit 2 | unbacked user approval claim | review_dispatch.py:68-69 comment says "one exception the user approved on 2026-10-24"; no Confirmed line in rulings matches. |
| L-F08-07 | CONFIRMED | P | Fix pending |  | none |
| L-F08-08 | CONFIRMED | P | Fix pending |  | none |
| L-F08-09 | CONFIRMED | P | Fix pending |  | none |
| L-F08-10 | CONFIRMED | Q | exit 2 | state folder names and transcript path | stall.py:169-188 and percall.py:102-112 build the agent transcript path from the layout; agent_stop.py:94 reads it from hook input; sweep prunes *.json only (cleanup_sweep.ps1:596-600). |
| L-F08-11 | CONFIRMED-CHANGED | Q | exit 2 | unbacked user approval claim | agent-rules.md:5 says the main session gets postures each prompt; README:35 and posture.py:14-22 send first prompt, 60 minutes, after compaction; posture.py:16 cites "user's answer to Q8" with no Confirmed line. |
| L-F08-12 | CONFIRMED | C | exit 3 |  | session_cards_start.ps1:21 names state_prune.ps1, which does not exist; cleanup_sweep.ps1:591 Invoke-StatePrune is the current place. |
| L-F08-13 | CONFIRMED | Q | exit 2 | unbacked user ruling claim | GR\README.md:19 "The user ruled on 2026-10-31 that ... losing battle"; no ## Confirmed line has it, only the card H\docs\sessions\2026-10-31-6777b546.md:25. |
| L-F08-14 | REFUTED | R | none |  | none |
| L-F08-15 | CONFIRMED | Q | exit 2 | commands hidden in wrappers | delete_rule.py:75-77,153-164 reads only the first word of a piece; media_rule.py:30-44 finds the tool word anywhere in a segment and skips quoted ones. |
| L-F08-16 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F09-01 | CONFIRMED | P | Fix pending |  | none |
| L-F09-02 | CONFIRMED | P | Fix pending |  | none |
| L-F09-03 | CONFIRMED-CHANGED | B | exit 3 |  | hydra-deluxe-rename.md:3 says patseguin1-commits/hydra-deluxe; its body :11, CLAUDE.md:13, this-machine.md:15 and git remote say splax77/hydra-deluxe. |
| L-F09-04 | CONFIRMED | P | Fix pending |  | none |
| L-F09-05 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F09-06 | CONFIRMED | Q | exit 2 | local card rule and ruling claim | check.py:232-239 keeps_local combines marked_local and is_local_only while common.py:280-289 says every script asks one place; "user ruled 2026-10-31" has no Confirmed line. |
| L-F09-07 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F09-08 | CONFIRMED-CHANGED | C | exit 3 |  | .derive-once-gate and .doc-review-gate cite hooks\lib\derive_once_rules.ps1 and doc_review_rules.ps1; the owners are gates\review_rules.py and publish_gate.py:12. |
| L-F09-09 | CONFIRMED | Q | exit 2 | scout write scope not enforced | agents/scout.md:9 says change no files except status and scratch; scout.md:5 grants Edit and Write and no hook (percall.py, agent_stop.py) limits it. |
| L-F09-10 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F09-11 | CONFIRMED | P | Fix pending |  | none |
| L-F09-12 | CONFIRMED | A | exit 3 |  | ui-testing.md:195 says snapshot_library is the only code that knows the library path; tools/compare_db.py:51,87 and this-machine.md:77 also name C:\Program Files\Hydra. |
| L-F09-13 | CONFIRMED | Q | exit 2 | how to copy the database | this-machine.md:81 says copy hydra.db plus -wal and -shm by hand; ui-testing.md:195 and compare_db.py:87 use SQLite backup to one file. |
| L-F09-14 | CONFIRMED | P | Fix pending |  | none |
| L-F09-15 | CONFIRMED | P | Fix pending |  | none |
| L-F09-16 | CONFIRMED | A | exit 3 |  | gates README.md:71 says the check always exits 0; tools/check_pointers.py:14-16,293-305 exits 0 only with --session-start and 1 or 2 by hand. |
| L-F09-17 | CONFIRMED | P | Fix pending |  | none |
| L-F09-18 | CONFIRMED | P | Fix pending |  | none |
| L-F09-19 | CONFIRMED | P | Fix pending |  | none |
| L-F10-01 | CONFIRMED | A | exit 3 |  | this-machine.md:81 says copy hydra.db with -wal/-shm as plain files; ui-testing.md:195 and hydra_ui_mcp.py:107-134 use SQLite online backup |
| L-F10-02 | CONFIRMED | C | exit 3 |  | this-machine.md:51 says PowerShell 7.6.9; the machine reports 7.6.10 |
| L-F10-03 | CONFIRMED | B | exit 3 |  | memory hydra-deluxe-rename.md:3 says patseguin1-commits; its line 11, git remote, CLAUDE.md:13 and this-machine.md:15 say splax77 |
| L-F10-04 | CONFIRMED | B | exit 3 |  | memory sp-squeeze-mechanics.md:4,11 says the game's SqIn/SqOut rule is at SP-37; 05-star-power.md:403-412 says SP-37 is Hydra's assumption, never checked against the game |
| L-F10-05 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F10-06 | CONFIRMED-CHANGED | Q | exit 2 | replay tool flag name | squeeze_scout.py:153 uses --replay; fcvideo.py:2906 and video-tools README:78 use --hydra-replay for the same job |
| L-F10-07 | CONFIRMED | C | exit 3 |  | skills/analyzing-sp-squeezes SKILL.md:34 points at memory gameplay-video-score-analysis, which exists only in memory archive/ |
| L-F10-08 | CONFIRMED | Q | exit 2 | install folder permissions decision | memory windows-installer.md:15 calls users-modify the user's design; no ruling, decision line or card found; installer/hydra.iss:73 sets it |
| L-F10-09 | CONFIRMED | Q | exit 2 | machine tool paths home | CLAUDE.md and MEMORY.md:49 say this-machine.md holds this machine's tool paths; it has no dumpbin or ffmpeg line, paths live in three memory notes |
| L-F10-10 | CONFIRMED | P | Fix pending |  | none |
| L-F10-11 | CONFIRMED | Q | exit 2 | worktree folder location | delete_rule.py:14-18 trusts only H\.claude\worktrees and the user-level folder; cleanup_sweep.ps1:174-178 adds <root>\worktrees; this audit's worktree sits at H\worktrees |
| L-F10-12 | CONFIRMED | P | Fix pending |  | none |
| L-F10-13 | CONFIRMED-CHANGED | A | exit 3 |  | hydra_ui_mcp.py:17,140 claim it is the only code that knows the library folder; bench.cpp:975-986 also pushes C:\Program Files\Hydra |
| L-F10-14 | CONFIRMED | P | Fix pending |  | none |
| L-F10-15 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F10-16 | CONFIRMED | P | Fix pending |  | none |
| L-F10-17 | CONFIRMED | P | Fix pending |  | none |
| L-F10-18 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F10-19 | CONFIRMED | A | exit 3 |  | ui-testing.md:350 copies plain files for the real-app stand-up; hydra_ui_mcp.py snapshot uses online backup; this-machine.md:81 gives a third recipe |
| L-F10-20 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F10-21 | CONFIRMED-CHANGED | B | exit 3 |  | memory video-decode-ceilings.md:11 gives 3,000/1,900 fps; video-tools README:522-527,582 gives different figures and not those |
| L-F10-22 | CONFIRMED-CHANGED | Q | exit 2 | REA tool paths home | REA skill SKILL.md:131-183 needs sigrok, WABT, Node paths and versions; this-machine.md holds none of them |
| L-F10-23 | CONFIRMED-CHANGED | Q | exit 2 | assorted small copy fixes | ui-testing.md:294 says report_window_flow_tests is called last but uitest_tests.cpp:34-35 calls command_tests after it; memory cap 150 copies progress.py:14 |
| L-F11-01 | CONFIRMED-CHANGED | Q | exit 2 | build slot folder location | build_slot.ps1:10,78 and bench_run.ps1 use ...\.claude\hooks\state\build_slots; common.py:10 STATE_ROOT is ...\gates\state |
| L-F11-02 | CONFIRMED-CHANGED | Q | exit 2 | which Visual Studio install | find_cmake.ps1:6-17 tries PATH then -latest; build_cpp.ps1:60-63 picks newest of -all -prerelease; ci.yml:326 uses -latest -all |
| L-F11-03 | CONFIRMED | Q | exit 2 | mutant switch unset meaning | hydra_mutant.h:41-53 unset is -1 and empty aborts; py_mutate.py:63 unset or empty is 0 |
| L-F11-04 | CONFIRMED | Q | exit 2 | missing harness proof test | harness.ps1:5,11,21 names tests\test_h39_harness.ps1 as its proof; that file does not exist |
| L-F11-05 | CONFIRMED | P | Fix pending |  | none |
| L-F11-06 | CONFIRMED | A | exit 3 |  | constants.py:3-7 says it is the ONE place for addresses; menu.ts:54, launch_patch.json:8,11 and start_candidates.json:19 hard-code others |
| L-F11-07 | CONFIRMED | A | exit 3 |  | derive-once-review.md:45 lists the rulings folder as a decision home; precheck decisions.cpp:317-338 and decisions.h:55-63 do not read it |
| L-F11-08 | CONFIRMED | A | exit 3 |  | replay.cpp:28-29 says scoring lives in core/replay.h; replay.h:1-12 says it owns no scoring rule |
| L-F11-09 | CONFIRMED | A | exit 3 |  | harness.ps1:5-7 says it is the one home of how a test runs a hook; Python tests (test_publish.py:205, test_ref_judge.py:67) copy and run hooks on their own |
| L-F11-10 | CONFIRMED | P | Fix pending |  | none |
| L-F11-11 | CONFIRMED | Q | exit 2 | leftover scratch folder cleanup | hydra_ui_mcp.py hydra_live_ folders sit under Temp\claude and are swept by age; game_tools.py:47,599 ch_live_ folders sit under Temp and nothing removes them |
| L-F11-12 | CONFIRMED | P | Fix pending |  | none |
| L-F11-13 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F11-14 | CONFIRMED | P | Fix pending |  | none |
| L-F11-15 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F11-16 | CONFIRMED-CHANGED | Q | exit 2 | test gate file list | test_ref_judge.py:24 GATE_FILES omits review_rules.py and ruling_rules.py that ref_judge.py:93,389 loads at run time |
| L-F11-17 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F12-01 | CONFIRMED | A | exit 3 |  | status_append.ps1:3-5 names the preamble as owner; brief-preamble.md:19 names progress.py; progress.py:21 names settings.fragment.json |
| L-F12-02 | CONFIRMED | Q | exit 2 | git options that take a value | delete_rule.py:30 lists seven value options; bash_windows_rule.py:25 lists two |
| L-F12-03 | CONFIRMED | Q | exit 2 | how shell commands are cut | delete_rule.py:75-77 pieces() is quote-blind; bash_windows_rule.py _scan is quote-aware; media_rule.py:18-27 has its own loop |
| L-F12-04 | CONFIRMED-CHANGED | Q | exit 2 | quoted command word | delete_rule.py:66-69 does not read a quoted first word; shell_output.py:44-52 and media_rule.py:14-15 strip quotes; bash_windows_rule.py:137-143 refuses it |
| L-F12-05 | CONFIRMED | Q | exit 2 | ch_call to ch_call | percall.py:69-70 counts it as a key call needing allow key run; front.py:491-504 refuses it outright |
| L-F12-06 | CONFIRMED | A | exit 3 |  | brief-preamble.md:33 says a doubled backslash outside double quotes is refused; bash_windows_rule.py:175-181 also passes one in a comment |
| L-F12-07 | CONFIRMED | Q | exit 2 | which deletes need a yes | settings.fragment.json:59-63 asks on four patterns; delete_rule.py:19,31-35 refuses a wider list for agents |
| L-F12-08 | CONFIRMED | P | Fix pending |  | none |
| L-F12-09 | CONFIRMED | P | Fix pending |  | none |
| L-F12-10 | CONFIRMED | Q | exit 2 | whole-library run detection | percall.py:14 checks only the --whole-library flag; agent-rules.md:14 and the memory note cover any run over the whole library |
| L-F12-11 | CONFIRMED | P | Fix pending |  | none |
| L-F12-12 | CONFIRMED | Q | exit 2 | exit-code tail command list | shell_output.py:20-22 lists mutation_probe.ps1 (gone) and hydra_precheck; rule texts split the same programs into may-run and may-not-run |
| L-F12-13 | CONFIRMED | P | Fix pending |  | none |
| L-F12-14 | CONFIRMED | P | Fix pending |  | none |
| L-F12-15 | CONFIRMED | Q | exit 2 | unconfirmed ruling claims | bash_windows_rule.py:173,178 and README:19,27 cite "the user's ruling"; no Confirmed line in review-and-hooks.md carries those words (the exempt-commit line is under Unconfirmed:40) |
| L-F12-16 | CONFIRMED | A | exit 3 |  | brief-preamble.md:43 and delete_rule.py:60-63/README:19 list some non-literal path characters; delete_rule.py:26 NOT_LITERAL also refuses %, ?, , and backtick |
| L-F12-17 | NOT VERIFIED | P | Fix pending (open: NOT VERIFIED) |  | none |
| L-F12-18 | CONFIRMED | P | Fix pending |  | none |
| L-F13-01 | CONFIRMED | Q | exit 2 | git pins in gates | ref_judge.py:34-35 pins four settings and drops config env; review_rules.py:60-62 and check_handoff.py:24-26 pin three and drop six others |
| L-F13-02 | CONFIRMED | A | exit 3 |  | status_append.ps1:1 says the preamble owns the call shape; brief-preamble.md:19 says progress.py:22-27 owns it; writer-brief.md:100 omits the quote rule |
| L-F13-03 | CONFIRMED | Q | exit 2 | 30,000 character ceiling | brief-preamble.md:35 says shell_output.py owns the number; shell_output.py:15 CAP = 4000 only, 30,000 is Claude Code's own limit |
| L-F13-04 | CONFIRMED | Q | exit 2 | destructive git commands asked or refused | delete_rule.py:32-35 refuses agents a longer list than brief-preamble.md:43; settings.fragment.json:60-63 only asks the main session for four patterns |
| L-F13-05 | CONFIRMED | Q | exit 2 | worktree folder location | delete_rule.py:14-18 and brief-preamble.md:43 name HY\.claude\worktrees; cleanup_sweep.ps1:172-179 and real task worktrees use HY\worktrees |
| L-F13-06 | CONFIRMED | Q | exit 2 | which python spelling | brief-preamble.md:33 python -I versus this-machine.md:4-8 use py; ruling_rules.py:108 and sync_settings.py:20 bare python.exe versus README:114 full path |
| L-F13-07 | CONFIRMED | Q | exit 2 | agents rewriting own branches | brief-preamble.md:71 and integrate.md:31 forbid any rewrite; ref_judge.py:135-137 refuses only main or branches origin has |
| L-F13-08 | CONFIRMED-CHANGED | Q | exit 2 | hand-typed gh issue close | issue-tracker.md:14 hand-types gh issue close --comment; this-machine.md:15 says text never goes through hand-typed gh |
| L-F13-09 | CONFIRMED | Q | exit 2 | integrator commit hold | integrate.md:27 hold last merge uncommitted; brief-preamble.md:71 commit before 100th call and after each step |
| L-F13-10 | CONFIRMED | B | exit 3 |  | every-brief-carries-status-rule.md front matter says dispatch gate denies briefs; its body says no gate checks; review_dispatch.py has no such check |
| L-F13-11 | CONFIRMED | Q | exit 2 | elevated window ruling | this-machine.md:49 says the user has ruled no elevated window; no Confirmed ruling line exists, only a card and a plan; memory note prefers ETW/WPR |
| L-F13-12 | CONFIRMED | P | Fix pending |  | none |
| L-F13-13 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F13-14 | CONFIRMED | P | Fix pending |  | none |
| L-F13-15 | CONFIRMED | A | exit 3 |  | clone-hero-mcp.md:29 says exactly allow key run; review_rules.py:54-55,86 accepts case and trailing . or ! |
| L-F13-16 | CONFIRMED | P | Fix pending |  | none |
| L-F13-17 | CONFIRMED-CHANGED | Q | exit 2 | hand-typed gh issue close | issue-tracker.md:12-13 hand-types gh issue edit/close; settings.fragment.json deny list and publish.ps1 kinds have no route for them |
| L-F13-18 | CONFIRMED | P | Fix pending |  | none |
| L-F13-19 | CONFIRMED | P | Fix pending |  | none |
| L-F13-20 | CONFIRMED-CHANGED | Q | exit 2 | commit trailer and co-author line | ref_judge.py:225-232 allows orchestrator as Agent; brief-preamble.md:65-67 fixes Co-Authored-By Opus 5.5 versus the session attribution reminder |
| L-F13-21 | CONFIRMED | Q | exit 2 | background commands and Read tool | brief-preamble.md:77 forbids background runs; agent_stop.py:44-51 treats them as parked; no hook refuses either rule |
| L-F13-22 | CONFIRMED-CHANGED | Q | exit 2 | library run allow phrase | CLAUDE.md rule 8 says asked for in chat; review_rules.py:51 wants the exact phrase; this-machine.md:91 versus percall.py:14 keying on the flag only |
| L-F13-23 | CONFIRMED | Q | exit 2 | hook tests run by agents | gates README:17 says agents run hook tests; brief-preamble.md:47 says never run all the hook tests; README:152 powershell versus this-machine.md:51 pwsh |
| L-F13-24 | CONFIRMED | Q | exit 2 | README repeats preamble rules | gates README:5 says it does not repeat the preamble; README:19,23,25,27 restate delete, trim and Bash rules |
| L-F14-01 | CONFIRMED-CHANGED | A | exit 3 |  | agent-rules.md:14 about 115 charts versus ui-testing.md:46 and test_engine_sweeps.cpp:656 98 |
| L-F14-02 | CONFIRMED | Q | exit 2 | pwsh or powershell | this-machine.md:51 use pwsh; gates README.md:152 powershell -ExecutionPolicy Bypass |
| L-F14-03 | CONFIRMED | Q | exit 2 | copy live hydra.db | this-machine.md:81 plain copy; ui-testing.md:195 online backup; ui-testing.md:350 copy while closed |
| L-F14-04 | CONFIRMED | A | exit 3 |  | this-machine.md:63 cdb with -y; stall_harness.py:121 CDB_COMMAND and launch_cdb pass no -y |
| L-F14-05 | CONFIRMED-CHANGED | A | exit 3 |  | ui-testing.md:18-20 bare build_cpp.ps1 -Target; build_cpp.ps1 header and build_slot.ps1:208 require a pass |
| L-F14-06 | CONFIRMED | P | Fix pending |  | none |
| L-F14-07 | CONFIRMED-CHANGED | A | exit 3 |  | clone-hero-mcp.md:31 names four menus no tool opens; menu_tools.py:191-234 enforces an online flag and kit-only items |
| L-F14-08 | CONFIRMED | Q | exit 2 | ch_compare settings required | game_tools.py:954-972 needs all four or none; compare_scorestats.py:202 keys on three |
| L-F14-09 | CONFIRMED | P | Fix pending |  | none |
| L-F14-10 | CONFIRMED-CHANGED | Q | exit 2 | scrolling the library rule | render-checks-use-the-corpus.md:19 scroll the middle of the list versus search-before-clicking-a-song.md:11 never scroll; guard only in uitest_commands.cpp |
| L-F14-11 | CONFIRMED-CHANGED | Q | exit 2 | game window focus at spawn | clone-hero-mcp.md:23 never front but one launch cannot ask hidden; ch-runs-must-not-need-focus.md:11 never show the window |
| L-F14-12 | CONFIRMED | Q | exit 2 | public doc style word lists | public-doc-style.md:27-31 versus user global CLAUDE.md:5 throat-clearing list overlap with different words |
| L-F14-13 | CONFIRMED | P | Fix pending |  | none |
| L-F14-14 | CONFIRMED-CHANGED | A | exit 3 |  | clone-hero-mcp.md:78 names one refusal; build_facts.py:31-65 and build_stamp.py:210 are two gates |
| L-F14-15 | CONFIRMED | P | Fix pending |  | none |
| L-F14-16 | CONFIRMED | P | Fix pending |  | none |
| L-F14-17 | CONFIRMED | P | Fix pending |  | none |
| L-F15-01 | CONFIRMED | Q | exit 2 | git add forms allowed | brief-preamble.md:63 bans add -A and add . ; git-traps-in-shared-checkout.md:19 bans add -A and add -u |
| L-F15-02 | CONFIRMED | P | Fix pending |  | none |
| L-F15-03 | CONFIRMED | P | Fix pending |  | none |
| L-F15-04 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F15-05 | CONFIRMED | P | Fix pending |  | none |
| L-F15-06 | CONFIRMED | P | Fix pending |  | none |
| L-F15-07 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F15-08 | CONFIRMED | P | Fix pending |  | none |
| L-F15-09 | CONFIRMED-CHANGED | B | exit 3 |  | windows-installer.md:19 says needs the full SHA; development.md:1011 passes --target main |
| L-F15-10 | CONFIRMED | B | exit 3 |  | no-full-library-runs.md:15 says No hook enforces it yet; percall.py:14 enforces it |
| L-F15-11 | CONFIRMED | C | exit 3 |  | SKILL.md:10 and squeeze-tools README:9 say src/search/graph.h; the constant is at src/core/model.h:121; 05-star-power.md says line 111 |
| L-F15-12 | CONFIRMED | A | exit 3 |  | brief-preamble.md:33 any git commit text is free; bash_windows_rule.py:172-174 charges --no-pager commit and quoted git commit |
| L-F15-13 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F15-14 | CONFIRMED | Q | exit 2 | background commands and Read tool | brief-preamble.md:77 forbids run_in_background; agent_stop.py:45-51 and README:41 plan for it and no hook refuses it |
| L-F15-15 | CONFIRMED-CHANGED | Q | exit 2 | REA client list | reverse-engineer-anything SKILL.md:37-42 lists 15 client ids; connection-and-recovery.md:46-50 lists 23 |
| L-F16-01 | CONFIRMED-CHANGED | Q | exit 2 | quoted first word reading | delete_rule.py:3-5 leaves a quoted delete unread; shell_output.py:46-47 and media_rule.py:15 read "cmake.exe" as cmake |
| L-F16-02 | CONFIRMED | Q | exit 2 | where commands are cut | delete_rule.py:75-77 cuts inside a word at a;b; media_rule.py:18-27 cuts only when an operator stands alone |
| L-F16-03 | CONFIRMED | Q | exit 2 | what main session is asked on deletes | settings.fragment.json:59-64 asks for four shapes; delete_rule.py:17-35 refuses a longer list to agents |
| L-F16-04 | CONFIRMED | P | Fix pending |  | none |
| L-F16-05 | CONFIRMED | P | Fix pending |  | none |
| L-F16-06 | CONFIRMED | A | exit 3 |  | brief-preamble.md:73 "At 150 tool calls ... it stops you"; progress.py:129 refuses the 150th call itself |
| L-F16-07 | CONFIRMED | Q | exit 2 | status call reader | replay.py:279-280 accepts hooks or gates folder in the status path; progress.py:22 accepts only gates |
| L-F16-08 | CONFIRMED | Q | exit 2 | gh commands that carry text | issue-tracker.md:14 gh issue close --comment has no deny rule; settings.fragment.json:8-57 denies gh pr comment typed by hand |
| L-F16-09 | CONFIRMED | P | Fix pending |  | none |
| L-F16-10 | CONFIRMED | P | Fix pending |  | none |
| L-F16-11 | CONFIRMED | P | Fix pending |  | replay.py:62 old budget regex Budget of N tool calls reached; progress.py:130 Your budget is spent |
| L-F16-12 | CONFIRMED | P | Fix pending |  | none |
| L-F16-13 | REFUTED | R | none |  | none |
| L-F16-14 | CONFIRMED | Q | exit 2 | git settings pinned | ref_judge.py:34-35 also pins trailer.separators; check_handoff.py:25-26 and review_rules.py:61-62 do not |
| L-F16-15 | CONFIRMED-CHANGED | P | Fix pending |  | check_pointers.py:32 HOOK_EXTENSIONS; doc_tokens.py:30-35 KNOWN_EXTENSIONS |
| L-F16-16 | CONFIRMED | P | Fix pending |  | none |
| L-F16-17 | CONFIRMED | P | Fix pending |  | none |
| L-F16-18 | CONFIRMED | A | exit 3 |  | fake_hydra_live.py:12 docstring "50,000 characters"; fake_hydra_live.py:64-65 BIG_LINE is about 49,000 |
| L-F16-19 | CONFIRMED | Q | exit 2 | ch tools copied values | build_facts.py:69 reads the version from the game folder; dump_facts.py:27 and README.md:27 type it by hand |
| L-F16-20 | CONFIRMED | Q | exit 2 | capture start wait | shot.ps1:2 Wait = 10; capture_window.ps1:56 Start-Hydra Wait = 12 |
| L-F16-21 | CONFIRMED-CHANGED | C | exit 3 |  | SKILL.md:10 says src/search/graph.h; 05-star-power.md:417 cites model.h line 111, code has it at model.h:121 |
| L-F16-22 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F16-23 | CONFIRMED-CHANGED | Q | exit 2 | unfiled rulings quoted in README | gates/README.md:19 and :27 quote the user; the words are not under ## Confirmed (review-and-hooks.md:40 is Unconfirmed) |
| L-F17-01 | CONFIRMED | Q | exit 2 | whole-library allow phrase | brief-preamble.md:49 "exactly allow library run"; review_rules.py:51-52 accepts any case and a trailing . or !; percall.py:47-48 judges only the flag |
| L-F17-02 | CONFIRMED | P | Fix pending |  | none |
| L-F17-03 | CONFIRMED | P | Fix pending |  | none |
| L-F17-04 | CONFIRMED | Q | exit 2 | message when a verdict has a problem | stall.py:280-281 says a move to a tip with no verdict is refused; agent_stop.py:243 raises a problem even when a verdict was recorded |
| L-F17-05 | CONFIRMED | P | Fix pending |  | none |
| L-F17-06 | CONFIRMED-CHANGED | A | exit 3 |  | derive-once-review.md:200 lists four causes; README.md:39 and review_dispatch.py:128 add a repo with no local main |
| L-F17-07 | CONFIRMED | A | exit 3 |  | reviewer.md:33, derive-once-review.md:151 and doc-review.md:29 say markdown; agent_stop.py:163 DOC_SUFFIXES adds .html and .htm |
| L-F17-08 | CONFIRMED-CHANGED | Q | exit 2 | same document path rule | publish_gate.py:68-88 compares paths absolute and lower case; review_rules.py:277-280 compares exact repo-relative strings |
| L-F17-09 | CONFIRMED | Q | exit 2 | waiver key length | doc-review.md:21 writes waive doc-review <key>; review_rules.py:25-28 accepts 7 to 40 or 64 hex |
| L-F17-10 | CONFIRMED | P | Fix pending |  | none |
| L-F17-11 | CONFIRMED | C | exit 3 |  | .derive-once-gate line 3 and .doc-review-gate line 3 name hooks\lib\*.ps1; the lists live in review_rules.py:21-24 |
| L-F17-12 | CONFIRMED-CHANGED | Q | exit 2 | which repos are gated | publish_gate.py:42-60 gated only by .doc-review-gate; review_dispatch.py:140-145 by either marker |
| L-F17-13 | CONFIRMED-CHANGED | Q | exit 2 | crash behaviour in prompt part | percall.py:176 closed when run alone; prompt.py:6-7,:17 says a deny is only said in a systemMessage |
| L-F17-14 | CONFIRMED | Q | exit 2 | what counts as user typed | review_rules.py:89-113 reads turnOrigin; session_cards common.py:372-435 reads origin.kind and strips reminders |
| L-F17-15 | CONFIRMED | P | Fix pending |  | none |
| L-F17-16 | CONFIRMED | Q | exit 2 | rulings README unconfirmed section | rulings/README.md:20 says copied from memory on 2026-10-31; review-and-hooks.md:40-43 holds 2026-11-06 lines |
| L-F17-17 | CONFIRMED | Q | exit 2 | small-text limit 2000 | publish_gate.py:13-14 SMALL_LINES, SMALL_CHARS = 20, 2000; review-and-hooks.md:22 names only the 20-line side |
| L-F17-18 | CONFIRMED | P | Fix pending |  | none |
| L-F17-19 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F17-20 | CONFIRMED | P | Fix pending |  | none |
| L-F18-01 | CONFIRMED | Q | exit 2 | whole-library allow phrase | agent-rules.md:14 "exactly"; review_rules.py:52 tolerant; memory no-full-library-runs.md:15 "No hook enforces it yet" against percall.py:45-55 |
| L-F18-02 | CONFIRMED-CHANGED | Q | exit 2 | finding that holds the merge | derive-once-review.md:106 sign CLEAN with a Holds the merge Note; derive-once-review.md:149 and reviewer.md:31 write BLOCKED |
| L-F18-03 | CONFIRMED-CHANGED | Q | exit 2 | unconfirmed line cited as ruling | derive-once-review.md:106 cites 2026-11-12 "alright approve b"; review-and-hooks.md:39 is under ## Unconfirmed |
| L-F18-04 | CONFIRMED | Q | exit 2 | finding that holds the merge | model-postures.md:5 nothing goes back to the executor; agent-rules.md:11 nothing goes back to the user |
| L-F18-05 | CONFIRMED | A | exit 3 |  | reviewer.md:33, derive-once-review.md:151 and doc-review.md:29 say markdown; agent_stop.py:163 includes html |
| L-F18-06 | CONFIRMED | P | Fix pending |  | none |
| L-F18-07 | CONFIRMED | C | exit 3 |  | .derive-once-gate line 3 names hooks\lib\*.ps1; review_rules.py:22-24 holds the lists |
| L-F18-08 | CONFIRMED | P | Fix pending |  | none |
| L-F18-09 | CONFIRMED | P | Fix pending |  | none |
| L-F18-10 | CONFIRMED-CHANGED | Q | exit 2 | what counts as users words | derive-once-review.md:45 and agent-rules.md:41 need a Confirmed or card quote; derive-once-review.md:52 also accepts the words your brief quotes |
| L-F18-11 | CONFIRMED | Q | exit 2 | hooks shared-file list | agent-rules.md:7, model-postures.md:11 and gates/README.md:156 list the shared hooks files; no code owns it |
| L-F18-12 | CONFIRMED | Q | exit 2 | precheck kind F wording | derive-once-review.md:21 kind F found mechanically; derive-once-review.md:38 kind F lines are candidates |
| L-F18-13 | CONFIRMED | Q | exit 2 | earlyStart has no checker | model-postures.md:7 earlyStart rule; no hook or script reads it; REVIEWER_TYPE in agent_stop.py:20 and review_dispatch.py:14 |
| L-F18-14 | CONFIRMED | A | exit 3 |  | writer-brief.md:101 stop starting new cards and report; progress.py:129 warns then refuses; reviewer.md:11 says 100 calls |
| L-F18-15 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F18-16 | CONFIRMED-CHANGED | Q | exit 2 | claimed approval with no ruling | review_dispatch.py:68 docstring "the one exception the user approved on 2026-10-24"; no Confirmed line found |
| L-F18-17 | CONFIRMED | Q | exit 2 | ff-only rule with no hook | README.md:160 fast-forward only prose; no hook refuses a non-fast-forward main move; ref_judge.py:244 cites a user ruling found only Unconfirmed |
| L-F18-18 | CONFIRMED | Q | exit 2 | realapp record name copy | review_rules.py:49 REALAPP_FILE literal; hydra_ui_mcp.py:78 REALAPP_FILE literal; ui-testing.md trigger against the src/ui path hook |
| L-F18-19 | CONFIRMED | P | Fix pending |  | none |
| L-F18-20 | CONFIRMED | Q | exit 2 | known_clones entries | derive-once-review.md:38 any add is a finding; ADR 0029:130-142 allows a dated entry |
| L-F18-21 | CONFIRMED | Q | exit 2 | D13 example in review doc | derive-once-review.md:52 names D13 as the example; fix-decisions.md:1680-1686 D302 reverses D13 |
| L-F18-22 | CONFIRMED | Q | exit 2 | Pitch Dark chord 466 example | derive-once-review.md:57-64 ratio 1.1067, 14.21 ms; 05-star-power.md:437 ratio 1.3926, 5.8 ms |
| L-F19-01 | CONFIRMED | Q | exit 2 | holding the merge on findings | one-reviewer-rule-d107.md:11 (memory) says do not hold; derive-once-review.md:106 holds for kind H, kind D, Holds-the-merge notes; doc-review.md:98 holds nothing |
| L-F19-02 | CONFIRMED | A | exit 3 |  | integrate.md:54 'keys a review on the exact tree, any later commit voids it' vs review_rules.py:233-246 _carries and derive-once-review.md:191 (clean merge of main passes) |
| L-F19-03 | CONFIRMED | A | exit 3 |  | ui-testing.md:145,147 list 12 report-window tests vs tests/ui/uitest_report_windows.cpp holding 24 names |
| L-F19-04 | CONFIRMED-CHANGED | B | exit 3 |  | one-reviewer-rule-d107.md:15 (memory) says cap is 150 with no 100/20 wrap-up point vs progress.py:12-14 and reviewer.md:11 |
| L-F19-05 | CONFIRMED | A | exit 3 |  | derive-once-review.md:151,202, doc-review.md:29, reviewer.md:33 'every markdown file' vs agent_stop.py:163 DOC_SUFFIXES (.md,.html,.htm) |
| L-F19-06 | CONFIRMED-CHANGED | Q | exit 2 | merge-of-main carry rule | review_rules.py:233-246 _carries (scans for a passing commit) vs ref_judge.py:305-316 made_from_existing (two parents, no pass check) |
| L-F19-07 | CONFIRMED | A | exit 3 |  | doc-review.md:21 'nobody computes or types a key' then 'waive doc-review <key>' vs publish_gate.py:107 and review_rules.py:26-28 which require a typed hex key |
| L-F19-08 | CONFIRMED | A | exit 3 |  | ui-testing.md:10-11 real-app check only for visual cases, cleared by 'still owed' vs review_rules.py:428-437 any non-comment src/ui change needs record or typed waiver |
| L-F19-09 | CONFIRMED | P | Fix pending |  | none |
| L-F19-10 | CONFIRMED | P | Fix pending |  | none |
| L-F19-11 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F19-12 | CONFIRMED-CHANGED | Q | exit 2 | plan review scope | plans-reviewed-before-shown.md (memory) 'every plan before user sees it, in chat too' vs doc-review.md:17 'committed plans are not gated' and publish_gate.py:110-113 |
| L-F19-13 | CONFIRMED | P | Fix pending |  | none |
| L-F19-14 | CONFIRMED-CHANGED | Q | exit 2 | merge test command spellings | integrate.md:43-44 '-tc=single-owner*', '-sf=*docs_match_code*' vs agent-rules.md:7 '-tc=docs match code*', '-sf=*single_owner*'; MEMORY.md:20 vs its note |
| L-F19-15 | CONFIRMED | P | Fix pending |  | none |
| L-F19-16 | CONFIRMED | P | Fix pending |  | none |
| L-F19-17 | CONFIRMED | C | exit 3 |  | H .derive-once-gate and .doc-review-gate name hooks\lib\*.ps1 vs live owner gates\review_rules.py:18-22 |
| L-F19-18 | CONFIRMED | P | Fix pending |  | none |
| L-F19-19 | CONFIRMED | P | Fix pending |  | none |
| L-F19-20 | NOT VERIFIED | P | Fix pending (open: NOT VERIFIED) |  | none |
| L-F19-21 | CONFIRMED | Q | exit 2 | banned phrase lists | public-doc-style.md:31 nine throat-clearing phrases vs ~/.claude/CLAUDE.md:5 three phrases |
| L-F20-01 | CONFIRMED | Q | exit 2 | chart count wording | ui-testing.md:46,48 '98' vs agent-rules.md:14 and no-full-library-runs.md:11 'about 115' |
| L-F20-02 | CONFIRMED | Q | exit 2 | what counts as full library run | no-full-library-runs.md:15 'No hook enforces it yet' vs percall.py:46-55; three lists in memory, agent-rules.md:14, brief-preamble.md:49 |
| L-F20-03 | CONFIRMED | B | exit 3 |  | early-fill-orange-85-d250.md:11 names kEarlyFillSlackMs vs src/core/model.h:106-115 kDefaultEarlyFillWindowMs |
| L-F20-04 | CONFIRMED | Q | exit 2 | above-optimal scores | analyzing-sp-squeezes SKILL.md:55 'bug, never a real score' vs public-doc-style.md:17 and dmleaderboards-comparison.md:24-29 'expected, normal' |
| L-F20-05 | CONFIRMED | B | exit 3 |  | hydra-deluxe-rename.md:3 description 'patseguin1-commits' vs its own line 11 and this-machine.md:15 'splax77' |
| L-F20-06 | CONFIRMED | Q | exit 2 | stop hook merge base | hooks-never-guess.md:13 calls merge base an assumption/fault vs agent_stop.py documents() and README:49 state it as the rule |
| L-F20-07 | CONFIRMED | Q | exit 2 | Preview fix tense | test-rubric-examples.md:9 'has since landed' vs :330-336 'the test to write in the same change as the Preview fix' |
| L-F20-08 | CONFIRMED | P | Fix pending |  | none |
| L-F20-09 | CONFIRMED | Q | exit 2 | video tools review gate | fc-video-workflow.md:17 'Keep the reviewer gate' vs review_rules.py:408 gate off with no marker in video-tools repo |
| L-F20-10 | CONFIRMED-CHANGED | B | exit 3 |  | hooks-never-guess.md:17 and one-reviewer-rule-d107.md:11 'no second review' vs agent-rules.md:11, derive-once-review.md:175 D314 exception |
| L-F20-11 | CONFIRMED | Q | exit 2 | plan GUI click test rule | new-screens-start-from-a-clickable-mockup.md:15 only; planner.md, premise-map.md, owner-map.md, ui-testing.md have no hit |
| L-F20-12 | CONFIRMED | Q | exit 2 | capture zoom step | render-checks-use-the-corpus.md:3,18 '2x zoom' vs ui-testing.md:10-11 no zoom step |
| L-F20-13 | NOT VERIFIED | B | exit 3 (open: NOT VERIFIED) |  | derive-display-from-engine-truth.md:20 'a row' vs derive-once-review.md:80 'row OR one-sentence reason' |
| L-F21-01 | CONFIRMED | P | Fix pending |  | none |
| L-F21-02 | CONFIRMED | Q | exit 2 | who typed this check | review_rules.py:103 human_prompt_text turnOrigin vs session_cards/common.py:397-398 classify_user origin.kind |
| L-F21-03 | CONFIRMED | Q | exit 2 | reviewer fingerprint twice | review_dispatch.py:172 sha256(prompt) vs agent_stop.py:199 sha256(first user message) |
| L-F21-04 | CONFIRMED | A | exit 3 |  | stall.py:280 'A move of main to a tip with no verdict is refused.' vs review_rules.py:408-411 'no-gate' allows it |
| L-F21-05 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F21-06 | CONFIRMED | Q | exit 2 | hooks line budget | replay.py:41 LINE_BUDGET = 3100 vs review-and-hooks.md:13 Confirmed 'Raise cap to 3,050' |
| L-F21-07 | CONFIRMED | C | exit 3 |  | inventory row I9-138 describes test_review_rules.py:911-1013 which no longer exists (replay.py:16,131 only) |
| L-F21-08 | CONFIRMED | P | Fix pending |  | none |
| L-F21-09 | CONFIRMED | P | Fix pending |  | none |
| L-F22-01 | CONFIRMED | P | Fix pending |  | none |
| L-F22-02 | CONFIRMED | Q | exit 2 | commit hash length and case | gates\ref_judge.py:195 accepts 40 or 64 lowercase; gates\review_dispatch.py:19 finds 40 hex only; waiver phrases in review_rules.py:26-28 take {7,40} or {7,64} |
| L-F22-03 | CONFIRMED | P | Fix pending |  | none |
| L-F22-04 | CONFIRMED | P | Fix pending |  | none |
| L-F22-05 | CONFIRMED | P | Fix pending |  | none |
| L-F22-06 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F22-07 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F22-08 | CONFIRMED | Q | exit 2 | ruling line tail | gates\ruling_rules.py:105 accepts only (session <id>); tools\session_cards\common.py:96-98 RULING_NOTE accepts any one-level parenthesised note |
| L-F22-09 | CONFIRMED | P | Fix pending |  | none |
| L-F22-10 | CONFIRMED | P | Fix pending |  | none |
| L-F22-11 | CONFIRMED | Q | exit 2 | card content rules | writer-brief.md:16-21 cites a 2026-10-31 user ruling on four content kinds; no Confirmed line in docs\agents\rulings holds it and check.py checks only the local value |
| L-F22-12 | CONFIRMED | A | exit 3 |  | writer-brief.md:75 forbids parentheses in the note; tools\session_cards\common.py:96-98 RULING_NOTE accepts one inner level |
| L-F22-13 | CONFIRMED | Q | exit 2 | who decides gated code paths | Hydra .derive-once-gate names Test-DeriveOnceGatedPath in hooks\lib that does not exist; gates\review_rules.py:21-22 BUILTIN_CODE is the finder's unconfirmed reading of the decider |
| L-F22-14 | CONFIRMED | B | exit 3 |  | memory every-brief-carries-status-rule.md line 3 says the dispatch gate denies briefs without the sentence; line 15 of the same note says no gate checks a brief |
| L-F22-15 | CONFIRMED | P | Fix pending |  | none |
| L-F22-16 | CONFIRMED-CHANGED | A | exit 3 |  | gates\README.md:75 says REVIEW_KINDS is the one place review record fields are checked; gates\agent_stop.py:211-214 writes an extra path field that common.py:11-12 does not list |
| L-F22-17 | CONFIRMED | P | Fix pending |  | none |
| L-F22-18 | CONFIRMED | P | Fix pending |  | none |
| L-F22-19 | CONFIRMED | P | Fix pending |  | none |
| L-F22-20 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F22-21 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F22-22 | CONFIRMED-CHANGED | Q | exit 2 | status helper id check | gates\status_append.ps1:27-37 accepts any existing agent record; gates\progress.py:46-49 ties the id to the caller only for subagent calls |
| L-F22-23 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F22-24 | CONFIRMED-CHANGED | Q | exit 2 | does a brief name the status helper | model-postures.md:11 says a brief names no status file or id; memory note and writer-brief.md:100 say it names the helper; progress.py:60-62 gives the id at first call |
| L-F23-01 | CONFIRMED | P | Fix pending |  | none |
| L-F23-02 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F23-03 | CONFIRMED | P | Fix pending |  | none |
| L-F23-04 | CONFIRMED | P | Fix pending |  | none |
| L-F23-05 | CONFIRMED | P | Fix pending |  | none |
| L-F23-06 | CONFIRMED | A | exit 3 |  | writer-brief.md:63 and :29 say memory names come from MEMORY.md; writer-brief.md:87 and link_memory.py:70-71 follow the folder listing |
| L-F23-07 | CONFIRMED | A | exit 3 |  | writer-brief.md:79 forbids every 7-to-40 hex string; check.py:176-180 flags only strings with a digit and a letter that are not session-id pieces |
| L-F23-08 | CONFIRMED | Q | exit 2 | what counts as user typing | gates\review_rules.py:89-113 decides by turnOrigin human; tools\session_cards\common.py:372-459 decides by origin.kind plus command args and queued prompts |
| L-F23-09 | CONFIRMED-CHANGED | Q | exit 2 | decision number ceiling | number_rulings.py:62-64 drops D-numbers of 1000 and above; session_cards\check.py:170 takes any D plus 1-4 digits and demands a log entry |
| L-F23-10 | CONFIRMED-CHANGED | B | exit 3 |  | tools\session_cards\batch.py:26 BATCHES_PER_WAVE = 10; memory lean-agent-fanout.md:21 says 3-4 agents per round |
| L-F23-11 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F23-12 | CONFIRMED | P | Fix pending |  | none |
| L-F23-13 | CONFIRMED-CHANGED | A | exit 3 |  | writer-brief.md:74-75 forbids quoting the unsent list; check.py:155-157 forbids only text found in no message; session_cards_start.ps1:44-48 hand-copies 30 minus 5 |
| L-F23-14 | REFUTED | R | none |  | none |
| L-F24-01 | CONFIRMED-CHANGED | Q | exit 2 | what counts as the user's words | derive-once-review.md:52 and doc-review.md:63 vs premise-map.md:45 and test-rubric.md:76 and audit-method.md:126 list different places the words may sit |
| L-F24-02 | CONFIRMED-CHANGED | Q | exit 2 | what counts as the user's words | CLAUDE.md:25 and writer-brief.md:67 say a card quote is not a filed ruling; doc-review.md:63 and derive-once-review.md:52 accept a card's quoted words |
| L-F24-03 | CONFIRMED | P | Fix pending |  | none |
| L-F24-04 | CONFIRMED | P | Fix pending |  | none |
| L-F24-05 | CONFIRMED | P | Fix pending |  | none |
| L-F24-06 | CONFIRMED | P | Fix pending |  | none |
| L-F24-07 | CONFIRMED | P | Fix pending |  | none |
| L-F24-08 | CONFIRMED | Q | exit 2 | sign-off line with added text | gates\agent_stop.py:102-109 records CLEAN when SIGN-OFF: BLOCKED has any text after it; docs only say write nothing with it |
| L-F24-09 | CONFIRMED-CHANGED | Q | exit 2 | combined review report sections | derive-once-review.md:126-151 eight sections vs doc-review.md:104-125 six sections; doc-review.md:123 says one line signs both |
| L-F24-10 | CONFIRMED-CHANGED | Q | exit 2 | handoff checker rules | check_handoff.py:28 table mode takes exactly 40 hex; doc_tokens.py:39 scrape mode takes 7-40; handoff.md:51-57 names five statuses but the code prints unknown |
| L-F24-11 | CONFIRMED | Q | exit 2 | what a finder reports | audit-method.md:50 asks inputs-and-answers tables with value edges; T5-finder.md:33-41 asks what each home tells its reader with situation edges |
| L-F24-12 | CONFIRMED-CHANGED | Q | exit 2 | what a brief carries | brief-preamble.md:3 vs audit-method.md:120 vs the every-brief memory note give three different lists |
| L-F24-13 | CONFIRMED | P | Fix pending |  | none |
| L-F24-14 | CONFIRMED | P | Fix pending |  | none |
| L-F24-15 | CONFIRMED-CHANGED | A | exit 3 |  | gates\README.md:93 says every Python hook uses common.run; gates\wake.py:90-110 and ref_judge.py:492-538 do not |
| L-F25-01 | CONFIRMED | Q | exit 2 | what counts as the user's ruling | premise-map.md:45 and agent-rules.md:41 admit card and brief quotes; CLAUDE.md and agent-rules.md:35 admit only ## Confirmed |
| L-F25-02 | CONFIRMED | A | exit 3 |  | premise-map.md:56 and test_docs_match_code.cpp:1625-1631 (LS then dash or digit, in live steps section) against agents/planner.md:13, reviewer.md:13 ("starts ### LS") |
| L-F25-03 | CONFIRMED | P | Fix pending |  | none |
| L-F25-04 | CONFIRMED-CHANGED | Q | exit 2 | what counts as a source | premise-map.md:52 (handoff, plan, card, memory note do not count) against CLAUDE.md rule 15 (tool, test, comment never a source); agent-rules.md:39 a third list |
| L-F25-05 | CONFIRMED | P | Fix pending |  | none |
| L-F25-06 | CONFIRMED-CHANGED | P | Fix pending |  | this-machine.md:85 lists five tables; tools/compare_db.py:130-136 lists ten (older layouts) |
| L-F25-07 | CONFIRMED | A | exit 3 |  | clone-hero-mcp.md:52 kit_player="System"; tools/ch_mcp/server.py:414 None then kit_tools.py:92 "Player1" |
| L-F25-08 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F25-09 | CONFIRMED | P | Fix pending |  | none |
| L-F25-10 | CONFIRMED | P | Fix pending |  | none |
| L-F25-11 | CONFIRMED | P | Fix pending |  | none |
| L-F25-12 | CONFIRMED | Q | exit 2 | probe lead of exactly zero | server.py:450-451 and clone-hero-mcp.md:53 give a sign rule only; edge_probe.py:172-173,198 calls lead 0 "early" |
| L-F25-13 | CONFIRMED-CHANGED | A | exit 3 |  | rulings/README.md:43 "any added line" and :51 date copy; ruling_rules.py:8-9,99,373 exempts headings, blanks, title line |
| L-F25-14 | CONFIRMED | P | Fix pending |  | none |
| L-F25-15 | CONFIRMED | A | exit 3 |  | doc-screenshots.md:15 "no ref"; tools/doc_shots.ps1:23-38 regex needs exactly one argument |
| L-F25-16 | NOT VERIFIED | P | Fix pending (open: NOT VERIFIED) |  | none |
| L-F25-17 | CONFIRMED-CHANGED | A | exit 3 |  | handoff.md:33-37 "skips docs/handoffs/" and "40 characters"; test_docs_match_code.cpp:1233 skips two dirs, check_handoff.py:28 accepts upper case |
| L-F25-18 | CONFIRMED | A | exit 3 |  | clone-hero-mcp.md:52-53 path "Edge Walk Pair"; tools/ch_probe/probe_songs.py:64-68 writes "Hydra Probe - Edge Walk" |
| L-F25-19 | CONFIRMED-CHANGED | Q | exit 2 | game build number home | build_facts.py:69-81 current_build (full version string) against dump_facts.py:381 DEFAULT_BUILD "6142" and test_docs_match_code.cpp:719 |
| L-F25-20 | CONFIRMED | A | exit 3 |  | doc-screenshots.md:13 "!! failed at line N"; uitest_commands.cpp:366,385 "line N: <reason>" |
| L-F25-21 | CONFIRMED | A | exit 3 |  | ui-testing.md:282 two messages and "a week"; hydra_ui_mcp.py:570-601 four messages, cleanup_sweep.ps1:54 age 7 |
| L-F25-22 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F25-23 | CONFIRMED | A | exit 3 |  | this-machine.md:85 "the chart's md5"; ADR 0028 and analysis.cpp song_id_read say a .srb id is a stored checksum |
| L-F25-24 | CONFIRMED | P | Fix pending |  | none |
| L-F26-01 | CONFIRMED | B | exit 3 |  | memory early-fill-orange-85-d250.md:11 two numbers (kEarlyFillSlackMs); CONTEXT.md:610-614 and model.h:106,112 one setting |
| L-F26-02 | CONFIRMED-CHANGED | Q | exit 2 | examples a decision question needs | audit-method.md:97 end examples plus a gap example; doc-review.md:61 and memory questions-describe-the-real-rule.md:16 end examples only |
| L-F26-03 | CONFIRMED | Q | exit 2 | what counts as the user's ruling | premise-map.md:45 and doc-review.md:63 accept a card quote; CLAUDE.md:25,41 and rulings README.md:16-18 only ## Confirmed |
| L-F26-04 | CONFIRMED | B | exit 3 |  | memory relay-status-in-plain-english.md ("test_" skip, second sentence); stall.py:30-34 header has no skip rule |
| L-F26-05 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F26-06 | CONFIRMED | P | Fix pending |  | none |
| L-F26-07 | REFUTED | R | none |  | none |
| L-F26-08 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F26-09 | CONFIRMED | Q | exit 2 | when an agent is parked | progress.py:77-79 clears parked on any call; stall.py:~236-240 also needs last > since |
| L-F26-10 | CONFIRMED | C | exit 3 |  | waiver.py:3 "the record forms are in gates/README.md"; README holds no waiver field list, fields are in waiver.py:22-26,43-44 |
| L-F26-11 | CONFIRMED | A | exit 3 |  | plan hooks-from-scratch.md:108 two patterns on two docs; test_docs_numbers.py:25-26 ten patterns on five docs |
| L-F26-12 | CONFIRMED | A | exit 3 |  | doc-screenshots.md:15 "no ref" and shots.txt:6-9 "full-frame"; doc_shots.ps1:27-38 one-argument regex |
| L-F26-13 | CONFIRMED | Q | exit 2 | may a comment state a value | memory comments-explain-current-code.md allows saying what a constant means; brief-preamble.md:119 "never restates the rule" |
| L-F26-14 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F26-15 | CONFIRMED-CHANGED | Q | exit 2 | song length source | memory use-source-facts-not-proxies.md (D75 metadata); analysis.h:215-216 D255 end event; trace_hits.py:68-69 D331; fix-decisions.md:1853 first vs last song_length line |
| L-F26-16 | CONFIRMED | P | Fix pending |  | none |
| L-F26-17 | CONFIRMED | P | Fix pending |  | none |
| L-F26-18 | CONFIRMED | P | Fix pending |  | none |
| L-F26-19 | CONFIRMED | B | exit 3 |  | MEMORY.md:22 and the note name slack 60 / kEarlyFillSlackMs; model.h:106,112 has one 60 ms default |
| L-F26-20 | CONFIRMED-CHANGED | B | exit 3 |  | memory dmleaderboards-comparison.md routes and md5; dmbot_client.h:6-7,50 other routes and Clone Hero song id |
| L-F27-01 | CONFIRMED | Q | exit 2 | when an agent is parked | progress.py:77-79 clears on any call; stall.py:236-240 also requires last > since |
| L-F27-02 | CONFIRMED | Q | exit 2 | document key byte handling | review_rules.py:65-71 normalized (CRLF only); publish_gate.py:19-29 also strips lone CR and reads UTF-16 |
| L-F27-03 | CONFIRMED | Q | exit 2 | StructuredOutput hand-back | progress.py:19 REPORT_TOOLS has two tools; percall.py:102 saves the message for SubagentHandback only |
| L-F27-04 | CONFIRMED | Q | exit 2 | removed file citation shape | check_pointers.py:38 needs backticks, accepts upper hex; test_docs_match_code.cpp:1334-1346 lowercase, "never committed" |
| L-F27-05 | CONFIRMED | Q | exit 2 | commit hash shape | check_handoff.py:28 upper case ok; review_rules.py:56 lowercase only; doc_tokens.py:39,50 either/lowercase; waiver 7-64 |
| L-F27-06 | CONFIRMED | Q | exit 2 | song.ini lines that count | game_files.py:276-290 keys before a header count; song.cpp:2217-2238,2274-2283 ignore them, pro_drums values differ |
| L-F27-07 | CONFIRMED | P | Fix pending |  | none |
| L-F27-08 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F27-09 | CONFIRMED | Q | exit 2 | brief hash two routes | review_dispatch.py:170-172 hashes the tool prompt; agent_stop.py:26-36,199-201 hashes first transcript user message |
| L-F27-10 | CONFIRMED-CHANGED | A | exit 3 |  | agent_stop.py:114 comment "The one reader"; ref_judge.py:224-253 also reads the trailer |
| L-F27-11 | CONFIRMED | A | exit 3 |  | rulings/README.md:43 "any added line"; ruling_rules.py:366-387 skips blanks, headings, title, README prose |
| L-F27-12 | CONFIRMED | Q | exit 2 | which record is the user's words | review_rules.py:89-109 human_prompt_text; ruling_rules.py:229-232 via prepass; agent_stop.py:26-36 first_user_message |
| L-F27-13 | CONFIRMED | P | Fix pending |  | none |
| L-F27-14 | CONFIRMED | P | Fix pending |  | none |
| L-F27-15 | CONFIRMED-CHANGED | Q | exit 2 | pointer checker rules | check_pointers.py:108-229 fuzzy any-file paths, "retired" under hooks; test_docs_match_code.cpp:1320-1329 docs/ paths exact only |
| L-F27-16 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F27-17 | CONFIRMED | P | Fix pending |  | none |
| L-F27-18 | CONFIRMED | Q | exit 2 | what counts as real-app use | ui-testing.md:241-249 says commands counts screenshots and timeout lines; review_rules.py:371-395 needs only dirty False and commands >= 1 |
| L-F27-19 | CONFIRMED | P | Fix pending |  | none |
| L-F27-20 | CONFIRMED | P | Fix pending |  | none |
| L-F27-21 | CONFIRMED | A | exit 3 |  | doc-screenshots.md:15 "no ref"; doc_shots.ps1:27-47 regex ^screenshot\s+(\S+)\s*$ |
| L-F27-22 | CONFIRMED | Q | exit 2 | number format in docs | precheck decisions.cpp:99,84-92 accepts separators and f/u suffix; test_docs_match_code.cpp:360 regex reads 1,000,000 as three numbers |
| L-F27-23 | CONFIRMED-CHANGED | Q | exit 2 | game build number home | build_facts.py:69-81 "1.1.0.6142" string; dump_facts.py:381 "6142"; launch_patch.json:3-4; test_docs_match_code.cpp:719 |
| L-F27-24 | CONFIRMED | P | Fix pending |  | none |
| L-F27-25 | CONFIRMED | P | Fix pending |  | none |
| L-F27-26 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F27-27 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F27-28 | CONFIRMED-CHANGED | A | exit 3 |  | derive-once-review.md:26-38 "precheck covers kinds B, C and E"; check_rows.cpp:241-265 also prints A and defines B differently |
| L-F27-29 | CONFIRMED | P | Fix pending |  | none |
| L-F28-01 | CONFIRMED | P | Fix pending |  | none |
| L-F28-02 | CONFIRMED | Q | exit 2 | stuck agent nudge or stop | model-postures.md:11 nudge once then TaskStop; stall.py:124,143 TaskStop and re-dispatch with no nudge |
| L-F28-03 | CONFIRMED | P | Fix pending |  | none |
| L-F28-04 | CONFIRMED | A | exit 3 |  | agent-rules.md:27 notice at 30 minutes; stall.py:134 only after more than 30 minutes |
| L-F28-05 | CONFIRMED | P | Fix pending |  | none |
| L-F28-06 | CONFIRMED | Q | exit 2 | missing plan file cited | wake.py:5 cites docs/superpowers/plans/2026-11-16-agent-supervisor.md (`git show 80b310a9:docs/superpowers/plans/2026-11-16-agent-supervisor.md`) which does not exist; check.py:34-35 cites it by name only |
| L-F28-07 | CONFIRMED | P | Fix pending |  | none |
| L-F28-08 | CONFIRMED | A | exit 3 |  | cleanup_sweep.ps1:33-46 and session_cards_start.ps1:26-32 header comments; the scripts' param blocks |
| L-F28-09 | CONFIRMED | Q | exit 2 | write-back reminder has no hook | CLAUDE.md Past sessions says next main session adds the fact; session_cards_start.ps1 has no write-back text |
| L-F28-10 | CONFIRMED | C | exit 3 |  | .derive-once-gate and .doc-review-gate name hooks\lib\*_rules.ps1; logic is in gates\review_rules.py |
| L-F28-11 | CONFIRMED | P | Fix pending |  | none |
| L-F28-12 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F28-13 | CONFIRMED-CHANGED | Q | exit 2 | plain English rule copies | brief-preamble.md:123 keeps the gloss; model-postures.md:13 drops it; both restate a different subset of ~/.claude/CLAUDE.md |
| L-F28-14 | CONFIRMED-CHANGED | B | exit 3 |  | Hydra CLAUDE.md:1 ask if unclear; memory ask-only-game-or-permission-questions.md limits to game or permission |
| L-F28-15 | CONFIRMED-CHANGED | Q | exit 2 | who defines a project word | agent-rules.md:41 CONTEXT.md decides; agent-rules.md:43 code usage settles the meaning |
| L-F28-16 | CONFIRMED | Q | exit 2 | meaning of build in rules 3 and 14 | agent-rules.md:9 build is any piece of work; agent-rules.md:20 build means compiling |
| L-F28-17 | CONFIRMED | Q | exit 2 | unfiled user quotes in rules | agent-rules.md:21,23 quote the user; rulings/agents-and-workflow.md Confirmed has no such lines |
| L-F28-18 | CONFIRMED-CHANGED | Q | exit 2 | who writes a missing owner | agent-rules.md:21 new owner goes in core; owner-map.md:37 nobody writes a new owner |
| L-F28-19 | CONFIRMED | Q | exit 2 | agent edit limits across worktrees | brief-preamble.md:41 never touch another worktree; brief-preamble.md:43 and gates README.md:19 delete hook passes any worktree path |
| L-F28-20 | CONFIRMED | Q | exit 2 | build pass slot folder | build_slot.ps1:78 hooks\state\build_slots; every other hook state in gates\state |
| L-F28-21 | CONFIRMED-CHANGED | Q | exit 2 | who updates Clone Hero page | Hydra CLAUDE.md:9 any session or agent updates the page; brief-preamble.md:105 only an agent whose brief owns the page |
| L-F28-22 | CONFIRMED | P | Fix pending |  | none |
| L-F29-01 | CONFIRMED-CHANGED | Q | exit 2 | when a wave is built | model-postures.md:11 and derive-once-review.md:198 build each branch before review; memory one-build-per-wave.md builds one combined branch |
| L-F29-02 | CONFIRMED | P | Fix pending |  | none |
| L-F29-03 | CONFIRMED-CHANGED | Q | exit 2 | hook tests agents may run | gates README.md:17 agents run hook tests; brief-preamble.md:47 only the ones the brief names |
| L-F29-04 | CONFIRMED | Q | exit 2 | build pass slot folder | build_slot.ps1:78 hooks\state\build_slots; gates\state holds all other hook state |
| L-F29-05 | CONFIRMED | P | Fix pending |  | none |
| L-F29-06 | CONFIRMED | Q | exit 2 | what backs a new number | brief-preamble.md:117 brief names the decision; derive-once-review.md:45 lists four places |
| L-F29-07 | CONFIRMED | Q | exit 2 | why comments versus no history | brief-preamble.md:119 allows why-a-rule-exists comments; rulings/code-and-engine.md:22 bans history in comments |
| L-F29-08 | CONFIRMED-CHANGED | Q | exit 2 | plain English rule copies | model-postures.md:13 drops the gloss; brief-preamble.md:123 and global CLAUDE.md differ in subsets |
| L-F29-09 | CONFIRMED | B | exit 3 |  | agent-rules.md:9 no separate yes for compiles; memory one-build-per-wave.md:15 get a yes before builds |
| L-F29-10 | CONFIRMED | Q | exit 2 | meaning of build in rules 3 and 14 | agent-rules.md:9 any piece of work; model-postures.md:3 versus :9,:11 compile meaning |
| L-F29-11 | CONFIRMED | Q | exit 2 | number of hook jobs | hooks-from-scratch plan line 11 five jobs; gates README.md:3 six jobs |
| L-F29-12 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F29-13 | CONFIRMED | Q | exit 2 | who defines a project word | agent-rules.md:41 CONTEXT.md decides; agent-rules.md:43 code usage settles meaning |
| L-F29-14 | CONFIRMED | A | exit 3 |  | derive-once-review.md:66 one for one; play_chart.py:361-369 scales by conversion |
| L-F29-15 | CONFIRMED-CHANGED | A | exit 3 |  | derive-once-review.md:57-66 transfer_scale_between; replay_schedule.cpp:656 step_transfer_scale |
| L-F29-16 | CONFIRMED | Q | exit 2 | end conversion two code homes | replay_schedule.cpp:267-276 C++ inverse; play_chart.py:361-369 Python forward |
| L-F29-17 | CONFIRMED-CHANGED | A | exit 3 |  | integrate.md:43-44 and derive-once-review.md:99 tc and sf forms; precheck test_pick.cpp:789-790 prints the reverse |
| L-F29-18 | CONFIRMED-CHANGED | Q | exit 2 | integrator role and build timing | integrate.md:3,41-46 integrator builds joined branch; model-postures.md:5,11 five types and per-branch build |
| L-F29-19 | CONFIRMED | A | exit 3 |  | brief-preamble.md:77 never; agent_stop.py and stall.py:239 tolerate and track parked agents |
| L-F29-20 | CONFIRMED | P | Fix pending |  | none |
| L-F30-01 | CONFIRMED | P | Fix pending |  | none |
| L-F30-02 | CONFIRMED-CHANGED | A | exit 3 |  | agent-rules.md:14 any whole-library run; percall.py:48 and review_rules.py:51 test only the flag text |
| L-F30-03 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F30-04 | CONFIRMED-CHANGED | Q | exit 2 | who writes a missing owner | agent-rules.md:21 new owner goes in core; owner-map.md:37 and scout.md:11 nobody writes one |
| L-F30-05 | CONFIRMED | P | Fix pending |  | none |
| L-F30-06 | REFUTED | R | none |  | none |
| L-F30-07 | CONFIRMED | P | Fix pending |  | none |
| L-F30-08 | CONFIRMED | P | Fix pending |  | none |
| L-F30-09 | CONFIRMED-CHANGED | Q | exit 2 | who defines a project word | README.md:34 and brief-preamble.md:99 and derive-once-review.md:51 differ; domain.md:11 lets a skill create terms |
| L-F30-10 | CONFIRMED | Q | exit 2 | when to refresh doc screenshots | doc-screenshots.md two triggers; development.md:977-983 release only |
| L-F30-11 | CONFIRMED | Q | exit 2 | old names in release notes | public-doc-style.md:77 keep old names; public-doc-style.md:5 check names against code |
| L-F30-12 | CONFIRMED | Q | exit 2 | where the user's words are searched | audit-method.md:126 omits rulings; CLAUDE.md:41 omits plans and cards; premise-map.md:45 adds chat |
| L-F30-13 | CONFIRMED-CHANGED | Q | exit 2 | question recommendation rule | audit-method.md:98 always recommend; memory counts-on-screen-must-agree.md:21 never recommend keeping a side |
| L-F30-14 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F30-15 | CONFIRMED | Q | exit 2 | who writes a missing owner | memory derive-display-from-engine-truth.md:18 propose an owner; scout.md:11 and owner-map.md stops say nobody writes one |
| L-F30-16 | CONFIRMED | P | Fix pending |  | none |
| L-F30-17 | CONFIRMED | Q | exit 2 | plain English rule copies | brief-preamble.md:123, doc-review.md:81 and writer-brief.md:94 are three subsets of the global file |
| L-F30-18 | CONFIRMED | P | Fix pending |  | none |
| L-F30-19 | CONFIRMED-CHANGED | A | exit 3 |  | clone-hero-mcp.md:49 late inside plain; play_chart.py:99,418-438 adds range |
| L-F30-20 | CONFIRMED-CHANGED | A | exit 3 |  | derive-once-review.md:66 one for one; play_chart.py:361-369 multiplies by conversion |
| L-F30-21 | CONFIRMED | P | Fix pending |  | none |
| L-F30-22 | CONFIRMED | P | Fix pending |  | none |
| L-F30-23 | CONFIRMED-CHANGED | Q | exit 2 | skills named but missing | issue-tracker.md:20,40 names /triage and /wayfinder; ~/.claude/skills holds neither |
| L-F31-01 | CONFIRMED | Q | exit 2 | who writes a missing owner | derive-display-from-engine-truth.md:18 and agent-rules.md:21 say propose or write a new owner in core; owner-map.md:37 and scout.md:11 say nobody proposes one |
| L-F31-02 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F31-03 | CONFIRMED | Q | exit 2 | display-changing fold stops or not | integrate.md:21-23 stops on a fold that changes a display or score; derive-once-review.md:96 says fold every copy with no stop |
| L-F31-04 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F31-05 | CONFIRMED | P | Fix pending |  | none |
| L-F31-06 | CONFIRMED | Q | exit 2 | drum lane lit after Note Shuffle | drum-lanes-d112 memory:13 and song.cpp:192-196 say every pad lit; ui-and-display.md:56 Confirmed says lane must match the chart notes |
| L-F31-07 | CONFIRMED | B | exit 3 |  | ch-disco-noflip-rule.md:11,15 says Clone Hero ignores drums0dnoflip; 02-note-list.md:253-260 says it counts as a span start with a no-flip flag |
| L-F31-08 | CONFIRMED | C | exit 3 |  | 02-note-list.md:611,631 cite song.cpp lines 203/88/147 and call at 2009; code is at 205/90/149 and 2204 |
| L-F31-09 | CONFIRMED | B | exit 3 |  | check-an-edge-case-can-happen.md:15 says planner and reviewer briefs carry the rule; no agent file in the searched folder does |
| L-F31-10 | CONFIRMED | P | Fix pending |  | none |
| L-F31-11 | CONFIRMED | C | exit 3 |  | analyzing-sp-squeezes SKILL.md:34 names memory note gameplay-video-score-analysis; it exists only in memory/archive |
| L-F31-12 | CONFIRMED | P | Fix pending |  | none |
| L-F31-13 | CONFIRMED | P | Fix pending |  | none |
| L-F31-14 | CONFIRMED | Q | exit 2 | who may run hydra_replay dump | test-rubric.md:45 says an agent asks the main session; analyzing-sp-squeezes SKILL.md:18 runs hydra_replay dump itself and rule 14 forbids agents |
| L-F31-15 | CONFIRMED | Q | exit 2 | SqOut draw and measure point | derive-display memory:23 says nominal end is not where a squeeze happens; SKILL.md:22-23 measures SqOut from pre-collection end |
| L-F31-16 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F31-17 | CONFIRMED | P | Fix pending |  | none |
| L-F31-18 | CONFIRMED | P | Fix pending |  | none |
| L-F31-19 | CONFIRMED | P | Fix pending |  | none |
| L-F31-20 | CONFIRMED | P | Fix pending |  | none |
| L-F31-21 | CONFIRMED | P | Fix pending |  | none |
| L-F31-22 | CONFIRMED | Q | exit 2 | chosen mockup is the design | approved-work-is-not-reasked.md:15 memory rule with no page or hook; ui-and-display.md:123 Unconfirmed hints an exception |
| L-F31-23 | CONFIRMED-CHANGED | Q | exit 2 | above-optimal run: bug or normal | video-tools README.md:41-43 and SKILL.md:55 say a bug, never a real score; dmleaderboards-comparison.md:26-29 says above-optimal is NORMAL |
| L-F31-24 | CONFIRMED | P | Fix pending |  | none |
| L-F31-25 | CONFIRMED | Q | exit 2 | out-of-scope lists in plans | planner.md:13 moves items to an Out of scope list for approval; search-web-before-finalizing-plans.md:11-15 says leave no left-alone list |
| L-F31-26 | CONFIRMED | P | Fix pending |  | none |
| L-F31-27 | CONFIRMED | C | exit 3 |  | SKILL.md:10 cites kSqueezeWindowMs in src/search/graph.h; it lives in src/core/model.h:121 |
| L-F32-01 | CONFIRMED | Q | exit 2 | builds per branch or per wave | one-build-per-wave.md:11 builds once on a combined branch; model-postures.md:11 and agent-rules.md:20 build each branch before review |
| L-F32-02 | CONFIRMED | P | Fix pending |  | none |
| L-F32-03 | CONFIRMED | Q | exit 2 | where SP ends and leeway | squeeze_rating.cpp:185-188 uses paid_by_sp_walk for squeezed-out rows, counted_without_squeeze with leeway for others |
| L-F32-04 | CONFIRMED | P | Fix pending |  | none |
| L-F32-05 | CONFIRMED | P | Fix pending |  | none |
| L-F32-06 | CONFIRMED-CHANGED | Q | exit 2 | where a proper name may come from | names-from-data-not-recall.md:15 and SKILL.md:63 accept web lookup; derive-once-review.md:76 says only the repo |
| L-F32-07 | CONFIRMED | Q | exit 2 | mockup rule has no home | new-screens-start-from-a-clickable-mockup.md:17 says no option switches but also four toggles were accepted |
| L-F32-08 | CONFIRMED | B | exit 3 |  | no-full-library-runs.md says no hook enforces it; percall.py:45-55 _library_refusal refuses it |
| L-F32-09 | CONFIRMED-CHANGED | B | exit 3 |  | questions-describe-the-real-rule.md:11 says rules 11 to 13; agent-rules.md rules 11-12 hold the cited content |
| L-F32-10 | CONFIRMED | B | exit 3 |  | relay-status-in-plain-english.md:11,15 cites an unfiled ruling and a test_ skip; stall.py and model-postures.md:11 have neither |
| L-F32-11 | CONFIRMED | P | Fix pending |  | none |
| L-F32-12 | CONFIRMED | Q | exit 2 | prior-art search rule has no home | search-web-before-finalizing-plans.md:11-15 only in memory; audit-method.md:36 covers left-alone for audits |
| L-F32-13 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F32-14 | CONFIRMED | P | Fix pending |  | none |
| L-F32-15 | CONFIRMED | C | exit 3 |  | SKILL.md:10 and squeeze_scout.py:33-34 cite kSqueezeWindowMs in src/search/graph.h; it lives in src/core/model.h:121 |
| L-F32-16 | CONFIRMED | P | Fix pending |  | none |
| L-F32-17 | CONFIRMED | P | Fix pending |  | none |
| L-F32-18 | CONFIRMED | P | Fix pending |  | none |
| L-F32-19 | CONFIRMED | P | Fix pending |  | none |
| L-F32-20 | CONFIRMED-CHANGED | A | exit 3 |  | ui-testing.md:99 says no hook reads shell text; brief-preamble.md:43 and percall.py library guard read shell text |
| L-F32-21 | CONFIRMED | Q | exit 2 | unfiled ruling cited in code | quiet.py:38 cites the user's ruling of 2026-11-07 on MSBuild helpers; no such line in rulings |
| L-F32-22 | CONFIRMED | P | Fix pending |  | none |
| L-F33-01 | CONFIRMED | P | Fix pending |  | none |
| L-F33-02 | CONFIRMED | Q | exit 2 | worktree folder location | cleanup_sweep.ps1:174-178 Get-SweepPrefixes vs delete_rule.py:17-18 WORKTREE_FOLDERS; neither covers H\worktrees |
| L-F33-03 | CONFIRMED | Q | exit 2 | when a session counts as running | cleanup_sweep.ps1:355-389 uses 7 days; prepass.py:31 RUNNING_CUTOFF_SECONDS is 2 hours |
| L-F33-04 | CONFIRMED-CHANGED | Q | exit 2 | posture resend interval | posture.py:16 comment calls 60 minutes the user's answer; plan line 306 shows T2 picks 60 |
| L-F33-05 | CONFIRMED | P | Fix pending |  | none |
| L-F33-06 | CONFIRMED | Q | exit 2 | ref gate clean merge rule | ref_judge.py:305-316 made_from_existing vs review_rules.py:234-244 _carries |
| L-F33-07 | CONFIRMED | P | Fix pending |  | none |
| L-F33-08 | CONFIRMED | Q | exit 2 | which card folders count | check.py:213-226 default_cards includes staging; ruling_rules.py:214-215 reads no staging |
| L-F33-09 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F33-10 | CONFIRMED | Q | exit 2 | kResultsStamp release rule has no home | stored-versions-one-rule.md:17 only; no doc, hook or test |
| L-F33-11 | CONFIRMED | C | exit 3 |  | session_cards_start.ps1:21 names state_prune.ps1; cleanup_sweep.ps1:16,28 has Invoke-StatePrune |
| L-F33-12 | CONFIRMED | Q | exit 2 | report tools saved before budget | percall.py:99-102 saves only SubagentHandback; progress.py:18-19 REPORT_TOOLS also has StructuredOutput |
| L-F33-13 | CONFIRMED | P | Fix pending |  | none |
| L-F33-14 | CONFIRMED | P | Fix pending |  | none |
| L-F33-15 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F33-16 | CONFIRMED | P | Fix pending |  | none |
| L-F33-17 | CONFIRMED-CHANGED | P | Fix pending |  | none |
| L-F33-18 | CONFIRMED | P | Fix pending |  | none |
| L-F33-19 | CONFIRMED | A | exit 3 |  | ch_mcp/README.md:83 says a human-played folder launch is not copied; bot_run.py:755-761 and clone-hero-mcp.md:60 say it is |
| L-F33-20 | CONFIRMED | P | Fix pending |  | none |
| L-F33-21 | CONFIRMED | P | Fix pending |  | none |
