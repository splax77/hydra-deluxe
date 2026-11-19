# TOOL-AUDIT-T5-F29 (finder F29, agent a90e688b17b19b04b)

Short names: PC = Hydra CLAUDE.md. AR = docs/agents/agent-rules.md. BP = docs/agents/brief-preamble.md. DOR = docs/agents/derive-once-review.md. IN = docs/agents/integrate.md. AM = docs/agents/audit-method.md. RR = docs/agents/rulings/README.md. MP = C:\Users\Patrick\.claude\model-postures.md. GR = C:\Users\Patrick\.claude\gates\README.md. GC = C:\Users\Patrick\.claude\CLAUDE.md. AG-x = C:\Users\Patrick\.claude\agents\x.md. BS = tools/build_slot.ps1. I read the pre-reads: plan hooks-from-scratch line 108, and the `## Confirmed` lines of review-and-hooks.md and agents-and-workflow.md.

Rows are grouped by the question they ask. Every row id is listed under one group. Where a copy sits in a long line the grep tool omitted, I read the line itself.

## Group 1. Who builds, and who runs tests that need a build
Rows: I5-076, I5-079, I5-080, I5-110, I5-112, I5-122, I5-136, I5-152, I6-045, I6-113, I6-062.
Question: may an agent build, may it run a test that needs a build, which test programs may it run, and who builds and when?

Homes (file:line, quoted):
- PC rule 14 (CLAUDE.md, Agent rules): "At most three builds run at once. Agents never build. A main session builds, one at a time, through a pass from `hydra_build_pass`, and runs every test that needs a build."
- AR:7 (rule 1): "they run no test that needs a build; those are the main session's (rule 14)". AR:20 (rule 14): "Agents never build... Any test that needs a build is the main session's too... They may not run `hydra_tests.exe`, `hydra_uitest.exe` or `hydra_replay.exe`". It quotes the user's 2026-11-07 words and says "A build or test that fails before review goes to a fresh executor on the same branch". AR:27: "nothing checks rule 14's line on tests."
- BP:47: "Never run a test that needs a build... So you never run `hydra_tests.exe`, `hydra_uitest.exe` or `hydra_replay.exe`, not even the main checkout's copies". BP:53 (a failing-first test): "you cannot run it red, because that needs a build." BP:57-59: "At most three builds run at once on this machine, and only a main session starts one, through a pass from `hydra_build_pass`. A hook refuses that tool to every agent."
- MP:9: "Any build, and any test that needs one, is yours; of Hydra's test programs, agents run only `hydra_precheck.exe` from main." MP:11: "build its branch (`hydra_build_pass`, one build at a time) and run its tests, before review... After sign-off, rebuild if the reviewer changed code, run the merge's tests, then merge."
- AG-executor:11: "You never build, and you never run a test that needs a build. The user ruled: "any tests that require a build are done by the main session, not agents."" It lists the three programs and adds "a brief that asks you to build is wrong, so stop and say so."
- AG-reviewer:21: the same wording, plus "a code fix you commit is not compiled when you sign off: say in your report which fixes need the main session's build... A new test is run red only when a brief asks, and the main session does that."
- GR:15 (the build-pass refusal, with the user's 2026-11-07 words): "`percall.py` refuses that tool to every agent... Hydra's `tools/build_slot.ps1` owns the limits on it." GR:17: "No hook enforces that part... An agent may not run `hydra_tests.exe`, `hydra_uitest.exe` or `hydra_replay.exe`... The hook tests on this page need no build, so agents still run them."
- percall.py:20 and :109 (the code, and the message it prints to an agent): BUILD_PASS_TOOL = "mcp__hydra__hydra_build_pass". The message reads "Agents never build, so hydra_build_pass is refused to every agent. Instead: commit your work and hand back; the main session builds your branch and runs any test that needs a build. Say in your report what to build and which tests to run."
- DOR:24, :99, :116, :122, :175, :198, :202. DOR:116: "You never build, and you never run `hydra_tests.exe`, `hydra_uitest.exe` or `hydra_replay.exe`, from any checkout (agent rule 14)." DOR:198 (the main session's setup order): "Build the branch and run the merge's tests yourself... If the build or a test fails, send a fresh executor to the same branch". DOR:202: "If the reviewer's fixes touch C++, build that tip again."
- IN:35: "You do not build the joined branch or run its tests; the main session does both (agent rule 14)." IN:41: "the main session builds the joined branch once with the target your brief names", then runs the scan and docs tests. IN:46: the merge's tests run "once, after the review". IN:48: a failure goes to a fresh executor.
- docs/development.md:114-133 ("Building under Claude Code"): "agents never build, and at most three builds run at once (agent rule 14...)"; the ticket-booth pass description.
- BS:75 `$BuildSlotCount = 3` (comment: "the user's limit of three is in docs/agents/rulings/agents-and-workflow.md"); BS:80 `$BuildPassUnclaimedSeconds = 120`; BS:195 "One build at a time per session." This is the code home of the limits.
- tools/hydra_ui_mcp.py:22-23 (hydra_build_pass "lives here because a hook can then refuse that one tool"); docs/agents/ui-testing.md:185 ("the server is the main session's too (agent rule 14)"); docs/agents/test-rubric.md:45 ("Agents may not run `hydra_replay` (agent rule 14)").
- Memory note one-build-per-wave.md:11-15 (not in MEMORY.md's index): "review each branch, then put the reviewed ones on one combined branch, build it once and run all their tests... Never plan one build per branch." And: "before launching work that will need builds, say how many builds it needs and get a yes."

What each home tells its reader to do (edges in):
| Case | What the homes say |
|---|---|
| Agent starts a build | Refused. Code (percall.py) refuses only the pass tool. build_cpp.ps1 refuses a build with no pass (AR:27). Nothing reads shell text (GR:15). |
| Agent runs a test that needs a build | Prose only (AR:27, GR:17: "No hook enforces that part"). |
| Agent runs hydra_precheck.exe from main | Allowed (AR:20, BP:47, MP:9, AG-executor:11, AG-reviewer:21, DOR:116, GR:17). |
| Exactly at the limit: a fourth build | BS:75 owns the three. Every prose copy says "at most three" (PC, AR:20, BP:57, docs/development.md:114). Read from the code (BS:75), not from them, so they agree. |
| Main session against an agent | Main session builds one at a time, with a pass (AR:20, MP:11, BS:195). The agent hands back uncompiled. |
| Agent that skips the preamble (card writer) | AG-card-writer:9 says "you commit nothing and run no tests, so skip the preamble". The hook still refuses the pass tool. Nothing in the card-writer file states the build ban. |
| Worktree against main checkout | Agents may use the main checkout's hydra_precheck.exe on a worktree (DOR:24, IN:37). The scan and docs tests built from main read main's tree (memory repo-scans-read-main-checkout.md:3). |
| When to build in a wave | MP:11 and DOR:198: build each executor's branch before review. IN:41: build the joined branch once. Memory note one-build-per-wave: review each branch first, then build one combined branch. See the splits. |

Kinds: K1 (many agreeing copies of "agents never build", "at most three", the three exe names). K6/K7 for the test line (AR:27 and GR:17 both say no hook enforces it; the prose in five homes is its only owner). K3 for the word "build" (see Group 5).

Splits:
- SPLIT 1: when the main session builds in a wave. MP:11: "When an executor returns... build its branch... and run its tests, before review." DOR:198 says the same. The memory note one-build-per-wave.md:11 says "review each branch, then put the reviewed ones on one combined branch, build it once". IN:41 builds the joined wave branch once, before the review. The three differ on whether a build per executor branch happens before review.
- SPLIT 2: GR:17 says "The hook tests on this page need no build, so agents still run them." BP:47 says "Never run all the hook tests." and "Run only the tests your brief names." Running one hook test file is allowed in GR:17, and BP:47 allows it only when the brief names it. Not a conflict, but the two homes state different permissions for the same act (an agent running a hook test).
- SPLIT 3 (path): BS:78 `$BuildSlotDefaultDir = 'C:\Users\Patrick\.claude\hooks\state\build_slots'`. The folder `~\.claude\hooks\state\build_slots` exists. `~\.claude\gates\state\build_slots` does not. The hooks plan's T8 (plan line 170) says housekeeping paths moved "hooks\state to gates\state", and every other state path is under `gates\state`. Two state homes.

Confirmed rulings found:
- docs/agents/rulings/agents-and-workflow.md:8: "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session."
- docs/agents/rulings/agents-and-workflow.md:9: "any tests that require a build are done by the main session, not agents."
Neither speaks to when the build falls in a multi-branch wave. The one-build-per-wave quote is in a memory note, which is not the user's words.

Tests holding homes equal: none for this rule. test_docs_numbers.py pins only budget numbers, the shell cap and the status helper (lines 110-202). test_agent_files.py checks only the premise map and promise map phrases. No test holds PC, AR, BP, MP, the agent files and GR equal on rule 14. BS:75 (3) is held by test_build_pass.ps1 (tools/test_build_pass.ps1). I counted its matches but did not open it.

## Group 2. The code and source data are the truth; what a search that finds nothing proves; when blocked
Rows: I5-090 (code is source of truth), I5-091 (absence claims), I5-093 (when blocked), I6-033 (reading names).
Homes:
- PC:71 (rule 9), AR:15 (rule 9), BP:93-95 and MP:13: the same handoff sentence is word for word in four places: "A handoff is another agent's summary. Before any new work starts from a handoff, every claim in it is checked against what actually exists: the code is the source of truth, and a handoff claim that does not match the code is wrong." Also docs/agents/handoff.md:3, and AG-executor:9 ("check its Claims table against the code before you build on it").
- Absence: BP:103 ("A search that finds nothing proves nothing on its own. Before you write that something does not exist, is never called or is not used, widen the search past the first name you tried, and say where you looked."), DOR:76 ("a grep that finds nothing proves nothing, so say where you looked... Every name comes from the repo, not memory."), docs/agents/doc-review.md:65 ("A search that found nothing proves nothing on its own."), AM:124 (names it as "the preamble's rule"), memory verify-before-claiming-a-source-is-wrong.md (index line, MEMORY.md:32: "a grep that misses X ≠ X absent"), memory names-from-data-not-recall. Also two older audit docs (docs/audit/2026-09-24-derivation-audit.md:11, 2026-10-03-derivation-audit.md:24) and a handoff (docs/handoffs/2026-11-08-clone-hero-docs-gaps-handoff.md:37), outside the stated scope.
- When blocked: BP:109-111. BP:111 says docs/agents/owner-map.md "The stops" "is the one home of this rule". owner-map.md:3 does say "This page is the one home of the map's shape and of the two stops that go with it", and the section "The stops" exists (line 35), so the claim is true. AR:25, AG-planner:19 (return a report), AG-reviewer:17 (sign off BLOCKED), owner-map.md:47 state parts of it.
Table (edge: who acts, and what an agent type does on a contradiction):
| Agent type | Told to do |
|---|---|
| Executor, scout | stop and return a report (BP:111) |
| Planner | stop and return a report instead of a plan (AG-planner:19) |
| Reviewer | sign off `BLOCKED` and quote the words (AG-reviewer:17, owner-map.md:47) |
| Card writer | skips the preamble (AG-card-writer:9), so BP:111 never reaches it |
Kinds: K1 (the handoff sentence is four-times verbatim); K7 for absence (stated in BP:103, DOR:76, doc-review:65, AM:124, memory; no hook checks it; BP:103 is the only home with the full rule; AM:124 calls BP "the preamble's rule", so BP is named as its home).
Splits: none found. The wording differs (BP:103 asks for a widened search and "say where you looked"; DOR:76 asks only "say where you looked").
Confirmed rulings: agents-and-workflow.md:10 "so this whole time we've been accepting another agent's framing without verifying any of this is actually necessary." (speaks to rule 9).
Tests: none.

## Group 3. Comments name where a rule lives; tests and numbers; the owner check before committing
Rows: I5-095, I5-096, I5-097.
Homes:
- BP:115-119, one section. BP:115 (the owner check) points at AR rule 15 for the rule itself and restates the search method: "Search `src/core`, `src/app`, `src/ui` and `tools`". PC rule 15 and AR:21 (the full rule) say the same search list. A pointer that also states the value is a home: the four-folder list lives in PC rule 15, AR:21 and BP:115.
- BP:117 (tests and numbers): points at docs/agents/test-rubric.md. DOR:46 (step 5) restates: "A test that recomputes what production code computes, instead of calling it, is a copy." and "A number is fine only when it is the rule and a page or ruling gives the reason". DOR:45 (step 4) says new numbers need "an ADR in `docs/adr/`, `CONTEXT.md`, a line under `## Confirmed` in `docs/agents/rulings/`, or the "User decisions" header of a plan". BP:117 says a new number "needs a user decision your brief names and the reason for that value written beside it". The two homes name different places a decision may sit: DOR:45 lists four places, BP:117 says "your brief names". K3 candidate.
- Comments: BP:119 ("A comment names where a rule lives; it never restates the rule"), DOR:104 ("A comment that describes what a rule matches is a new copy too; name the owner instead."), memory comments-explain-current-code.md (MEMORY.md line: "RULE: no old values, dates or history in comments"). K1/K7: the comment rule has no hook behind it.
Table (edge): an executor with no named decision stops and asks (BP:117); a reviewer finds it and holds it as a kind D finding (DOR:106). An agent that skips the preamble (a card writer) writes no code, so none of it applies.
Splits: SPLIT 4: BP:117 and DOR:45 name different sources for "the user's decision behind a new number" (a decision "your brief names" against four listed places). Nothing says the brief's named decision must itself be in one of the four.
Confirmed rulings: none speak to comments. Tests: none.

## Group 4. Plain English
Rows: I5-097a (the plain-English home claim), I5-117.
Homes:
- GC:3-9 holds the rules (the one place that defines them).
- BP:123: "Write plain English... Lead with the plain version. One idea per sentence. Gloss any jargon in one line right after it. No bullet walls of file:line references; put sentences around them. See the "How to explain things" section of the user's global `CLAUDE.md`... That file is the one home of those rules." It claims GC as home, and GC does hold the rules. The line also states the rules, so it is a home of its own (a pointer that states the value).
- MP:13: "Write plain English: lead with the plain version, one idea per sentence, no bullet walls of file:line references."
- PC:1: "How to explain things lives in the user's global `~/.claude/CLAUDE.md`." (a pointer, no value).
- AG-scout:9, AG-executor:9: "Return your findings in your report, in plain English." (pointer-ish; states no rule).
- stall.py:30-33 (STATUS_HEADER, printed to the main session): "Relay the claim to the user briefly, in plain English." Also MP:11 and memory relay-status-in-plain-english.md:15 (the relay rule).
Differences (K3): GC says "no dense nested-bullet walls" and "No jargon without a one-line gloss". BP:123 and MP:13 change this to "No bullet walls of file:line references". That is a narrower rule than GC's. GC's own BAD example is an isolated file:line list inside bullets, so the two rules overlap but are not the same.
Table: the main session gets MP:13 and the global GC every session. An agent gets BP:123 (the preamble). An agent that skips the preamble (card writer) is told only AG-card-writer's own text; I did not find the plain-English rule in AG-card-writer:9. Worktree against main: no difference.
Splits: none found as an opposite answer. The narrower wording in BP/MP against GC is a K3 note.
Tests: test_stall.py:305 pins the phrases "briefly, in plain English" and "When the evidence disagrees" in the relay header. That holds the stall.py text equal to a test, and MP:11 and the memory note (which say the same thing in other words) are not held. Confirmed ruling: agents-and-workflow.md:26 "the status updates should be brief but relayed in plain english".

## Group 5. Approval, the main session's role, handoff, library runs
Rows: I5-101 (main session's role), I5-102 (approval gate), I5-115 (handoff and library claims), I6-103 (who runs the handoff checker).
Homes:
- Approval: PC rule 3 ("Every plan and every build waits for the user's yes before it begins."), AR:9 (the full rule, which says "In this rule a build means any piece of work: writing code, editing hooks, or launching agents to do either... The main session's compiling and test runs inside approved work need no separate yes (rule 14)."), MP:3 ("A code-only change needs no approval of its design, but its build still waits. When the user asks for one specific edit, the request is the approval."), AM:34 (an audit plan "goes to the user and waits for their yes (CLAUDE.md agent rule 3)"), memory approved-work-is-not-reasked, memory one-build-per-wave.md:15 ("before launching work that will need builds, say how many builds it needs and get a yes.").
- Main session's role: MP:1 only. PC and AR give no role sentence; MP:1 states it ("you own the requirements, the judgment, the integration, the builds and the final check").
- Handoff checker: handoff.md:65 and :67, BP:97 ("run the handoff checker on it first: `py -I tools/check_handoff.py <handoff.md>`"), .claude/commands/handoff.md:12, AG-executor:9 ("check its Claims table against the code"), AR:15, MP:13. The tool is tools/check_handoff.py.
- Library runs: PC rule 8, AR:14 ("sending exactly `allow library run`"), MP:13 ("Never run or brief a full library run without the user's permission in chat"), GR:13 (the whole-library check, which refuses a shell command holding `--whole-library` unless the user typed `allow library run`), BP:49. Code home: percall.py and review_rules.py (`is_library_allow`).
Differences:
- K3: the word "build" in rule 3 (any piece of work) and rule 14 (compiling). PC prints both rules with no gloss on rule 3's "build". Only AR:9 and AR:20 gloss it. MP:3 "its build still waits" uses the rule 3 meaning; MP:9 "Any build... is yours" uses the rule 14 meaning, in the same file.
- SPLIT 5: AR:9 says the main session's "compiling and test runs inside approved work need no separate yes (rule 14)". The memory note one-build-per-wave.md:15 says "before launching work that will need builds, say how many builds it needs and get a yes." The note quotes the user ("3 builds at once? what? i didnt accept doing lots of builds") but is a memory note, not a ruling. I did not find a session card for 8e6a2908 (not searched).
- BP:97 tells an agent with a handoff to run the checker first. AG-executor:9 tells it to "check its Claims table against the code" without naming the checker. AG-scout and AG-reviewer don't mention a handoff. The card writer skips the preamble.
Edges: handoff checker at the main session: handoff.md:67 ("The session that continues a handoff runs it again before any work starts") and MP:13 ("every claim in it is checked") both apply. The main session is not told the exact command in MP:13.
Confirmed rulings: review-and-hooks.md:16 "i want to be able to give explicit approval for a full library run." (library); agents-and-workflow.md:19 "if anything changes a display in the app or a record, run it by me. if it only changes code you can use your recommendation without input." (approval; it speaks to rule 3's "code-only"). agents-and-workflow.md:24: "you can't just go straight into writing code without confirming first."
Tests: test_docs_numbers.py has no test for these. Kind: K1, K3, K7 (the role sentence and the approval rule have no hook).

## Group 6. Which hooks enforce what; the one-registration rule; the docs test
Rows: I5-100, I5-145, I5-182, I5-189.
Homes:
- BP:5: enforced by hooks: "the status line, the call and time budget, the delete rule, the whole-library guard, the Bash shapes that fail on Windows, the trim of long shell output and the commit trailers." It omits: the build-pass refusal (BP:57 itself says "A hook refuses that tool to every agent"), the ref gate's one-review check, the key-route check, the media guard, the ruling check, the publish gate.
- AR:27: "Hooks check four of these. The progress hook keeps rule 7... The whole-library guard keeps rule 8. The ref gate keeps rule 5's one review before `main`. A hook keeps rule 14's 'agents never build'..." AR:27 does not list the delete rule, the Bash shapes, the trim or the trailers (they are not rules 1-15).
- GR:3: "six jobs and nothing else". The plan hooks-from-scratch line 11 says "The new system does five jobs and nothing else", with five numbered items (lines 13-17). SPLIT 6: five jobs (plan) against six jobs (GR:3). GR:3 says "where that plan and the code differ, the code wins", which is a claim, not a check; the plan line is still a home.
- GR:116: "`percall.py` is the only hook on PreToolUse `*`... `prompt.py` is its only registration... PostToolUse has one registration too, `shell_output.py`... Stop has one registration, `wake.py`... Its `timeout` is `stall.STALL_SECONDS` plus `stall.SCAN_EVERY`". Code home: settings.fragment.json (lines 124-236). stall.py:22 (STALL_SECONDS = 600) and :25 (SCAN_EVERY = 60); the fragment says `timeout: 660` for Stop; the live settings.json:245 says 660. The numbers agree today (K1). Registrations I read in the fragment and the live file: PreToolUse "*" (percall), "Agent" (review_dispatch), "Artifact|ExitPlanMode" (publish_gate); the live file also lists SubagentStop "*" (agent_stop.py). GR:116's "only hook on PreToolUse `*`" is true (the two others have narrower matchers).
- GR:154 / the docs test (test_docs_numbers.py): one test per property, as GR:154 says. I5-189 is one home (GR:154) plus the test itself. The test file's own list of checks (functions at lines 79-198) matches GR:154's description; I did not compare every item.
Table (edge): what a reader is told about hooks: BP:5 reader (every agent) is told the list; GR:3 reader (hook author) is told six jobs; MP and PC rule 14 tell the main session "a hook keeps rule 14's 'agents never build'" only through AR:27. The main session against an agent: percall.py runs different checks in each (GR:11).
Tests holding homes equal: tests/test_wake.py:212-218 (`test_registered_timeout_is_stall_seconds_plus_one_scan`: asserts the fragment's Stop timeout equals stall.STALL_SECONDS + stall.SCAN_EVERY) holds GR:116's number and the fragment equal to the code. test_docs_numbers.py:148 (`test_one_process_per_prompt`) and :155 (`test_one_hook_after_a_tool_call`) hold the one-registration rule for prompt and PostToolUse. test_docs_numbers.py:142 (`test_live_settings_carry_the_fragment`) holds the live file equal to the fragment. Nothing holds BP:5's list or AR:27's list equal to anything (K7: three lists, no home named; GR:3 and BP:5 each point at the other for detail).
Confirmed ruling: review-and-hooks.md:15 "yes that is good. make a new plan to get rid of every hook we have now and start over from scratch." and :11 "if a hook is doing something unnecessary or contains a lot of arbitrary rules that needs to scrutinized". Plan line 108 ("Each rule has one home. The numbers (100, 150, 20, 30, 5, 10) sit at the top of `progress.py` and nowhere else in code; the docs quote them") bears on BP:5 and AR:7's numbers. test_docs_numbers.py:110-122 enforces that for the status numbers (rule 7).

## Group 7. Who decides a project word's meaning; CONTEXT.md
Rows: I6-021, I6-022.
Homes (the sentence "CONTEXT.md decides what a project word means"):
- AR:41: "`CONTEXT.md` decides what a project word means. A ruling that uses a word does not redefine it. A change that gives a `CONTEXT.md` word a second meaning is a finding at step 8 of `docs/agents/derive-once-review.md`..."
- AR:43: "Before any agent calls a project term wrong or unclear, it checks how the code already uses the term. The code's usage settles the meaning, not a guess drawn from one sentence of a doc. If a definition's text and the code's usage disagree, that is a question for the user, in game terms with both readings, and the agent changes neither."
- DOR:51 (step 8), DOR:106 ("Never rewrite a `CONTEXT.md` definition, even to fix a finding"), BP:99, RR:22 ("`CONTEXT.md` decides that"), RR:34 ("If the reading and `CONTEXT.md` disagree, `CONTEXT.md` wins and the session asks the user."), PC:41 (rule summary under "Rulings and this machine": "Only the user changes what a `CONTEXT.md` term means; the full rule is in `docs/agents/agent-rules.md`, "Rulings and definitions""; that section exists and holds the rule at AR:41-43, so the pointer is correct), docs/agents/domain.md:43 (use the term as defined), CLAUDE.md:21.
Table (edge: when the definition text and the code's usage disagree):
| Home | Says |
|---|---|
| AR:41 | CONTEXT.md decides |
| AR:43 | the code's usage settles the meaning, but a disagreement is a question for the user; change neither |
| DOR:51 | same as AR:43 |
| BP:99 | a disagreement is a question for the user; change neither |
| RR:34 | a session's reading against CONTEXT.md: CONTEXT.md wins and the session asks |
SPLIT 7: inside AR:43 itself, "The code's usage settles the meaning" and "If a definition's text and the code's usage disagree... changes neither" answer the same disagreement two ways. DOR:51 repeats both sentences. BP:99 has only the second. AR:41 says CONTEXT.md decides, with no mention of the code.
Executor/reviewer against main session: both are told never to rewrite a definition (AR:43, DOR:106, BP:99). Card writers skip the preamble.
Confirmed ruling: none quoted for CONTEXT.md in docs/agents/rulings I read (the user's words quoted inside AR:43 are in AR, not a `## Confirmed` line). I did not grep every ruling file. Tests: none found for the sentence.
Kinds: K1 (same sentence in five places), K7 (the rule has no hook).

## Group 8. The squeeze rows in the review guide (worked example)
Rows: I6-028 (where the ratio between two ends is computed), I6-029 (where the bot shifts later presses).
Homes:
- Owner code: src/core/squeeze_rating.h:50-53: "The transfer scale between two ticks: mspm(end)/mspm(anchor)" for `transfer_scale_between`. squeeze_rating.h:57-73 names `step_transfer_scale` and `frontend_transfer_scales`. The file exists at this path, and the commit 707c285 holds `src/core/squeeze_rating.cpp`.
- DOR:57-66 (the Pitch Dark worked example): "The tool takes that squeeze's ratio from `SPSqueeze::transfer`... The finding is fixable: the fix makes the tool call `transfer_scale_between`... `retime_from_end` in `tools/ch_probe/experiments/play_chart.py` moves each later press by the read end minus the planned end, one for one."
- tools/replay_schedule.cpp:267-269 `end_conversion` (own.early/read.early, own.late/read.late) and :273-276 `read_at_own_ms` (comment: "The player's sum (retime_from_end in tools/ch_probe/experiments/play_chart.py), solved for the read."). tools/replay_schedule.h:139-141 says the same. replay_schedule.cpp:437-440 and :636 still read `SPSqueeze::transfer` and `Activation::transfer_post`; :656, :746, :775, :791 use `step_transfer_scale`.
- play_chart.py:361-369 `own_end_now_ms`: `own_stored_end_ms + conversion[side] * (read_end_ms - read_stored_end_ms)`, the same sum in the forward direction (K6: the conversion is computed in C++ and applied in Python). play_chart.py:390-442 `retime_from_end`: moves `aim_ms` by `own_now_ms - p.own_end_ms`.
Table (what each home does for a read end that differs from the planned one):
| Home | Move applied to a later press |
|---|---|
| DOR:66 | read end minus planned end, one for one |
| play_chart.py:441, :361-369 | `conversion[side] * (read - read_stored)`, then own_now minus own_end |
| replay_schedule.cpp:273-276 | inverse: `read_stored + (own - own_stored) / c` |
SPLIT 8: DOR:66 says "one for one"; play_chart.py:361-369 scales by the file's conversion. The docstring at play_chart.py:390-401 says "the move goes through the file's conversion". DOR:66 is a K2 drifted description of the Python rule (or the Python changed after DOR was written).
Possible K2 on DOR:57-64: the worked example says the tool takes the ratio from `SPSqueeze::transfer`. The tool's current code still reads it at replay_schedule.cpp:437 and :636. I did not trace whether :656 and :791 now replace it, so I cannot say whether the DOR text is stale.
Two homes of the sum: replay_schedule.cpp:273 (C++, inverse) and play_chart.py:361 (Python, forward). Tests: tests/test_replay_schedule.cpp:1716-1730 pins the C++ side with fixed numbers. tools/ch_probe/tests/test_play_chart.py:1692-1726 pins the Python side with fixed numbers. No test holds the two equal to each other.
Confirmed rulings: none found for the squeeze arithmetic in the files I read. docs/agents/rulings/code-and-engine.md:66 is about thresholds (not read in full).

## Group 9. Integrator
Rows: I6-105 (job), I6-106 (what the brief names), I6-108 (check a task branch calls owners), I6-109 (stop on a fold), I6-111 (appended scan-row files), I6-116 (never loosen a scan row), I6-119 (report).
One home each, all in IN (integrate.md: lines 3, 5, 21, 23, 29, 48, 60). I searched C:\Users\Patrick\.claude (agents, skills, gates, CLAUDE.md, model-postures.md, memory) for "integrator|wave branch|fork point|integrate.md": nothing. In the Hydra repo (docs/agents, CLAUDE.md, .claude/commands, tools/*.py) only BP:3 names integrators ("implementers, reviewers, integrators and read-only scouts"). Copies:
- I6-109: IN:23 ("If a fold would change a displayed text, a score or a stored record, stop and report it. Do not pick.") is a shorter copy of DOR:106 (the list of fixes that need the user) and of AR:11-12 (rules 11-12). Kind: K1 (copy), different reader (integrator against reviewer).
- I6-116: IN:48 "Never loosen a scan row to make it pass." Also in test-rubric.md? I did not search it.
- I6-113: IN:35, covered in Group 1.
- MP:5 lists agent types (planner, executor, reviewer, scout, card-writer). There is no integrator agent type in C:\Users\Patrick\.claude\agents (five files: card-writer, executor, planner, reviewer, scout). So the integrator is dispatched as some other type, or by brief only. Side note: K7-like gap.
Splits: IN:3 says "the main session's one suite run"; IN:46 says the merge's tests run "once, after the review"; IN:41 also builds once and runs two tests before the review. Not an opposite answer, but "one suite run" and the scan/docs tests before review are two runs.
Filters for the same tests are spelled differently: IN:43 `-tc="single-owner*"` and DOR:99 `-tc="single-owner*"`, against AR:7 `"-sf=*single_owner*"`; IN:44 `-sf=*docs_match_code*` against AR:7 and PC:57 `"-tc=docs match code*"`. SPLIT 9 (K1/K2): the two tests are named by different filters in different homes. AR:7 says the precheck prints them as `always:` lines, so the precheck would be the owner of the filter. I did not find the filter string in tools/precheck/ (grep for `single_owner|single-owner|always` there found no `always:` line text), so I cannot name the owner.

## Group 10. Reviewer and agents: no helpers, preamble first
Rows: I6-002, I6-003.
Homes for "no helper agents": DOR:5 ("You run on Sonnet. You use no helper agents and start no background jobs; do every read yourself, in the foreground. A wait on a helper reader uses up your review's time."), doc-review.md:5 (same sentence), BP:77 ("Run every helper and long command in the foreground. Never use run_in_background... You have no helper agents; do the work yourself."), AG-executor:9, AG-scout:9, AG-reviewer:9 ("You have no helper agents"), `disallowedTools: Agent` in the frontmatter of all five agent files (executor.md:6, reviewer.md:6, scout.md:6, planner.md:6, card-writer.md:6). The frontmatter is the code-level owner of "no helper agents"; the prose copies state it again. For "no background jobs": prose only (DOR:5, doc-review:5, BP:77). No hook refuses `run_in_background` for an agent (shell_output.py:75 only skips its rewrite for a background call; progress.py:77 has a comment about "an agent... parked on background work", which suggests agents can be parked on background work).
Homes for "read the preamble first": DOR:7 ("Read `docs/agents/brief-preamble.md` first."), IN:3, AG-executor:9, AG-reviewer:9, AG-scout:9, AG-card-writer:9 ("skip the preamble"), the workflow task text, MP:5 ("briefed with the preamble (`docs/agents/brief-preamble.md`) first"). BP:3 says "Where the brief and this page disagree, stop and report it". The card writer is the one agent type told to skip it.
Table: reviewer with DOR:5 and BP:77 agree; card writer: no helpers (frontmatter), no preamble. Main session against an agent: the main session starts agents and background jobs freely; nothing says so.
Tests: test_agent_files.py checks premise-map phrases only. Nothing holds the `disallowedTools` against the prose. Kinds: K1 (many copies), K5 (prose telling an agent not to run background jobs, with no hook), K7 for "no background jobs".

## Group 11. Audit method
Rows: I6-120 (what an audit is), I6-122 (the four exits), I6-124 (what "not a problem" needs), I6-125 (code-only), I6-126 (who the plan goes to), I6-127 (scope), I6-128 (decisions a plan must list), I6-130 (calibration), I6-132 (finders split work), I6-133 (a split must reach the user), I6-134 (verifier rules).
Homes: AM (audit-method.md: lines 3, 19-28, 34, 36, 38, 44-46, 52, 54, 58-64, 121-124) and PC:33 ("Every finding leaves by one of four exits. A split goes to the user unless a ruling quoted in the user's own words names both of its sides, and a fresh agent checks every closure."). The tooling audit's own task brief repeats the four exits and the claim words (the inventory report cites "the brief" as a home; I was told not to read it).
- Four exits: AM:19-26 and PC:33. The inventory row says the exits also appear at DOR:106; I read DOR:106 and it holds no four-exit list (it lists "A finding you should not fix yourself" and the "Holds the merge" notes). That cross-reference is wrong, not a home.
- "On purpose / by design / deliberately / kept": AM:26 and AM:123 (the list of what an audit brief carries). Also AM:144 and AM:180 (the old finding), and docs/agents/doc-review.md? I did not grep it there.
- "Code-only is not an exit": AM:28 only. Also AM:180 (history).
- Calibration: AM:44: "the 2026-10-03 audit did this with `src/core/squeeze_rating.cpp` at 707c285". I checked: commit 707c285 exists and holds src/core/squeeze_rating.cpp; the path agrees with the inventory row's comment. No other home.
- Split must reach the user: AM:54 and AM:24 (exit 2: "the default for every split"), PC:33 (with the ruling exception). The exception ("unless a ruling quoted in the user's own words names both of its sides") is in PC:33 and AM:25; AM:24 says the default is a question and AM:25 allows a ruling. Consistent.
- Scope and plan approval: AM:34-36 (the user's words quoted: "the first pass should cover everything", "Leave nothing in a "pre-existing, left alone" pile."). AM:36 says the words are in card `2026-10-11-8bffee2b` and D113/D119. I did not open the card.
Table (edge: "exactly at a limit" for an audit: the stop rule): AM:40 ("Every stage that repeats needs a cap") and BP:81 ("Any step that repeats or waits needs a cap") are the same rule in two homes (K1).
Main session against an agent: only the main session picks an exit (AM:66-68, and the tooling audit brief). A finder or verifier writes tables and side notes (AM:50, :58-62). Card writers don't take part.
Splits: none found. SPLIT-like wording note: AM:25 (exit 3) requires "a verbatim quote... naming both sides", while the task brief (and the user's CLAUDE.md "Audits" line, PC:33) says the same; the same text, so they agree.
Confirmed rulings: none quoted in AM I could match to `## Confirmed` lines; I did not search agents-and-workflow.md for an audit ruling beyond the lines I read. Tests: none for AM. docs match code (tests/test_docs_match_code.cpp) checks backticked names in docs, which would catch a renamed path (such as `src/core/squeeze_rating.cpp`) in AM:44, but I did not confirm it covers audit-method.md.
Kinds: K1 (four exits in AM and PC:33), K7 (AM:28 "code-only" and AM:123's claim-word list have no check).

## Rows with a single home (searched, nothing else found)
I6-105, I6-106, I6-108, I6-111, I6-116, I6-119 (IN only). I6-120, I6-125, I6-126, I6-127, I6-128, I6-130, I6-132, I6-133, I6-134 (AM only). Search covered: C:\Users\Patrick\.claude agents, skills, gates (md and py), CLAUDE.md, model-postures.md, the memory folder (not archive); the Hydra repo's CLAUDE.md, docs/agents, rulings README, .claude/commands, tools/*.py and tools/*.md. Searched by phrases ("four exits", "on purpose", "deliberately", "code-only", "calibration", "integrator", "wave branch", "fork point"). Phrases I did not search by words other than these: a copy that describes the rule in other words would be missed.

## Rows not reached in depth
None skipped, but depth varied. Weakest: I5-189 (one home, GR:154, and the test itself; I did not diff every item against the test), I6-116 (did not search test-rubric.md or tests for "loosen a scan row"), I6-127 and I6-130 (did not open the cited session card or the decision log).

## Side notes
1. tools/build_slot.ps1:78 keeps the pass folder in `~\.claude\hooks\state\build_slots`. Both `hooks\state\build_slots` and `hooks` exist; `gates\state\build_slots` does not. See split 3 in Group 1.
2. docs/agents/derive-once-review.md:57-66 is a worked example whose numbers (158,224.76 ms, 1.1067, 14.21 ms) are tied to code that has since changed; see Group 8.
3. The memory note one-build-per-wave.md is not listed in MEMORY.md (the index I was given); it is a home for the "when to build" rule.
4. A way past a hook exists: no hook checks `run_in_background` for an agent (GR and BP only say it in prose); and nothing hooks "agents run tests that need a build" (AR:27 says so).
5. MP:5's list of agent types has no integrator, and there is no integrator agent file.
6. docs/handoffs/2026-11-08-clone-hero-docs-gaps-handoff.md:37 and several docs/audit files restate "a search that finds nothing proves nothing"; they are outside the stated scope, so I did not list them as homes.
7. The inventory's I6-122 cross reference to DOR:106 for "four exits" does not match the file (see Group 11).
