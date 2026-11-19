# TOOL-AUDIT-T5-F13b (re-send of F13; agent a95fe06eff300745e)

Short names: GATES = C:\Users\Patrick\.claude\gates; HY = C:\Users\Patrick\Downloads\Hydra\hydra-test; TM = HY\docs\agents\this-machine.md; M = the memory folder.

Ids taken: I6-197, I6-198 (the two rows F13 did not reach), plus the homes F13 says it did not open: percall.py, delete_rule.py, bash_windows_rule.py (_too_long), tools\build_slot.ps1, sync_settings.py, test_percall_key.py, test_review_rules.py, docs\agents\clone-hero-mcp.md, docs\agents\rulings\README.md, and a grep of rulings\, the decision log, plans and cards for the "no elevated window" line. For the homes I searched the question each one answers (listed in D below). test_review_rules.py (910 lines) I did not read in full; I grepped tests\ for the values below only.

## A. I6-197: what to do when CMake refuses a Visual Studio install
Row: TM:43 "-DCMAKE_GENERATOR_INSTANCE=\"<install folder>,version=<its version>\"" and "The lasting fix is a repair in the Visual Studio Installer, which is the user's to run."
Homes found:
- TM:35-39 install table (3 installs, versions 18.9.12120.119 / 18.9.12112.369 / 17.14.37614.0, folders). Today vswhere -all prints 17.14.37614.0, 18.9.12112.369, 18.9.12120.119: the table agrees (K1 copy of machine state, no check holds it equal).
- TM:41 "the `build-cpp` cache names the 2026 Build Tools as its generator instance". HY\build-cpp\CMakeCache.txt:773 `CMAKE_GENERATOR_INSTANCE:INTERNAL=C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools`: agrees today (K1).
- TM:41-43 says `Find-CMake` "falls back to `vswhere -all`". HY\tools\find_cmake.ps1:12-18 does exactly that (-latest with the VC component, then -all -prerelease). TM is a prose copy of the script's rule (K1; the script is the owner of the fallback).
- Nothing else: grep of HY\CLAUDE.md, docs\agents, docs\clone-hero, docs\superpowers\plans, .claude\commands, GATES, ~\.claude agents/CLAUDE.md/model-postures, M\*.md for GENERATOR_INSTANCE, vswhere, "Visual Studio Installer", "no 'version=' field" found only TM and find_cmake.ps1 (plus Frida's node_modules, not ours).
Table (what each home tells its reader): TM:43 tells a main session or agent configuring CMake to pass the flag with folder+version from the table; find_cmake.ps1 tells nobody, it just picks the first cmake it finds. Edges: exactly at limit n/a. Main session vs agent: agents never build (rule 14), so TM:43 can only be acted on by a main session; the row's "main session" owner matches. Card writers: skip preamble, no build. Worktree vs main: TM table lists install folders, not per-worktree; the cache named is HY\build-cpp only (a worktree's own build folder is not checked).
Kinds: K1 (table versions and cache instance copied from the machine; script rule copied into prose).
Splits: none today. Risk noted, not graded: the table's versions go stale at the next VS update and nothing holds them to vswhere.
Ruling: none (rulings\ grep for "Visual Studio", "vswhere" found nothing under ## Confirmed).
Test holding homes equal: none (GATES\tests has no test touching TM or find_cmake).

## B. I6-198: the agent shell is not elevated; no elevated window
Row: TM:47-49 "`Medium Mandatory Level`"; "`wpr -start` was refused ... with error `0xC5585011`"; "The user has ruled that no elevated window is opened."
Homes:
- TM:45-49 (the owner-claim home, states the level, the error code, the date 2026-11-07, "has ruled").
- TM:63 "`cdb` attaches from this unelevated shell, unlike `wpr`." (restates the fact; K1).
- HY\docs\superpowers\plans\2026-11-07-ch-run-timing.md:12 "it needs no administrator rights"; :27 quotes the user "i dont accept that you need an elevated run. figure out another solutio." and says "No elevated window, no trace, no PresentMon, no install, no machine change, and nothing that works around the Windows administrator prompt. Row P37."; :161 "no elevated window"; :220 "Elevation for WPR. Overridden ... no elevated window is opened now and the trace script is removed (T9)". The plan's Premise text says the account is in Administrators at Medium Mandatory Level and not in Performance Log Users (line found by grep in the same plan).
- HY\docs\superpowers\plans\2026-11-12-player-stall-fixes.md:62 "No elevated window, so no `wpr` trace (`docs/agents/this-machine.md`)." (pointer plus the rule restated).
- HY\tools\ch_mcp\machine_recorder.py:6 "It needs no administrator rights" (code comment; K1 of the same rule).
- HY\docs\sessions\2026-11-07-7d216054.md:11 and :21 (card; quotes the user's words "i dont accept that you need an elevated run. figure out another solutio."). Card, a home like any text, not a ruling.
- M\check-long-captures-early.md:15 "Prefer non-invasive capture (ETW/WPR, read-only memory reads) over ho..." tells the reader to prefer WPR; TM:49 says wpr is refused from agent shells. This is a split in what each tells the reader (K3/K2 flavour): the memory note recommends WPR, TM says WPR cannot run from an agent shell and the user ruled no elevated window.
- docs\audit\2026-10-03-fix-decisions.md:1115 is about elevating Hydra's installer (the `runas` verb), a different question, not a home.
Ruling check: grep of docs\agents\rulings\ (all files) for elevated/administrator/admin window found no line. The only words are the user's in the card and plan (unfiled). So TM:49 "The user has ruled that no elevated window is opened" cites a ruling that is not under any ## Confirmed. That is K8-style: the page claims a ruling and rulings\ holds none; the words exist only in the card (card:21) and plan (:27). CLAUDE.md says a card quote is not a filed ruling.
Table: TM:49 tells every agent: no Windows trace (wpr) from an agent shell; plan :27 tells the plan's executors: no elevated window, no trace, no PresentMon, nothing that works around the prompt (wider than TM, which says only elevated window); memory note tells: prefer ETW/WPR. Edges: main session vs agent: TM says "the shell agents run in", the plan says no elevated window at all (so a main-session elevated window is also out by the plan, TM silent). Card writers: not applicable. Worktree vs main: n/a.
Kinds: K1 (fact restated 4 places), K3 (plan widens it to PresentMon/install/machine change; TM keeps only wpr), K8 (ruling claimed, none filed), K2 against the memory note.
Hook enforcing it: none found (GATES grep for elevated/wpr/administrator found nothing). A prose rule nothing checks, in 5 places, no home: also K7.
Test: none.

## C. Homes F13 did not open, by the question each answers

C1. How many builds may run at once, and who may start one (percall.py:19-20 BUILD_PASS_TOOL and :107-111 refusal text; tools\build_slot.ps1).
- HY\tools\build_slot.ps1:1-3 "This file is the one place that knows the slot rule, the pass rule and their numbers"; :75 `$BuildSlotCount = 3` (owner); :73 comment "the user's limit of three is in" the decision log.
- HY\CLAUDE.md:76 rule 14 "At most three builds run at once."; docs\agents\agent-rules.md:20 "At most three builds run at once on this machine."; docs\agents\brief-preamble.md:57 "At most three builds run at once on this machine"; the user's Confirmed line docs\agents\rulings\agents-and-workflow.md:8 "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." (ruling found, not a home).
- HY\tools\test_build_pass.ps1:117 expects the text 'all 3 build slots are taken' (test holding the count in a literal, K1; it runs against the real script's slot count, so it does fail if 3 changes).
- GATES\percall.py:109 "Agents never build, so hydra_build_pass is refused to every agent." (code owner of the agent refusal).
- Differences: the ruling says "one build per session"; CLAUDE.md says "A main session builds, one at a time"; build_slot.ps1 gives each session a pass. Split worth a look (K3): the ruling's "limit of one build per session" vs the slot script, which takes `-Holder`/session id and refuses a fourth pass; whether one session can hold two passes was not tested (I did not run it).
- Path split (side finding): build_slot.ps1:10 and :78 and bench_run.ps1:8-20 put state under C:\Users\Patrick\.claude\hooks\state\ (build_slots, bench), while every gates script uses C:\Users\Patrick\.claude\gates\state\. Both folders exist (hooks\state holds agents, cleanup_sweep, posture, refusals.jsonl, reviews.jsonl, rulings.jsonl, session_cards, stall, status, waivers.jsonl, build_slots; gates\state holds nearly the same names). Two state homes for hooks (K6 candidate); I did not check which one the live code reads.
Kinds: K1 (count in 5 places), K3 (one-per-session), K6/K1 (two state folders).

C2. Which worktree folders may an agent delete inside (delete_rule.py:14-18 WORKTREE_FOLDERS).
- GATES\delete_rule.py:14-18 lists exactly two: HY\.claude\worktrees and C:\Users\Patrick\.claude\worktrees.
- GATES\tests\test_delete_rule.py:28-29 (HOOKS_FOLDER, HYDRA_FOLDER literals) and :159-161 pin the same two (copy by literal).
- HY\docs\agents\derive-once-review.md:197 "cut under a `worktrees\` folder (HY\.claude\worktrees\ or C:\Users\Patrick\.claude\worktrees\): the delete rule lets an agent delete only inside a worktree there."
- HY\tools\new_worktree.ps1:6,29 makes <main>\.claude\worktrees\<TaskId>.
- GATES\cleanup_sweep.ps1:174-178 removable worktrees: <root>\.claude\worktrees in any repo, and <root>\worktrees in the repo the hook lives in ("where the main session makes its task worktrees").
- Real state: `git worktree list` shows 98 worktrees under HY\worktrees\ (e.g. HY\worktrees\batch-rows-br1, and this audit's own HY\worktrees\tooling-audit-records) and a few under HY\.claude\worktrees\. HY\worktrees\ is named by cleanup_sweep only for the hooks repo, and by delete_rule and derive-once-review not at all.
- Confirmed ruling: docs\agents\rulings\review-and-hooks.md:17 (2026-10-25, "Keep rule, move worktrees (Recommended)") to "The delete rule lets an agent recursively delete only inside its scratch folder or a worktree that sits in a `worktrees\` folder. Ten Hydra worktrees sit beside the repo as `C:\Users\Patrick\Downloads\Hydra\wt-*` instead. An agent in `wt-batch-whole-library` today could not run `rm -rf` on its own `build-cpp` folder. Which way?" It says "a `worktrees\` folder" generally, not which path.
Table: delete_rule: an agent may rm -rf inside the two folders; HY\worktrees\<x>\... is not allowed unless inside the scratch folder (edge: a worktree at HY\worktrees\ is a worktree the user's ruling covers by words but the code does not list). cleanup_sweep: HY\worktrees\ is not a removable folder for the Hydra repo (only .claude\worktrees); for the hooks repo it is. new_worktree.ps1: makes only .claude\worktrees, so who makes HY\worktrees\<x>? Not found in scope (the main session by hand, per cleanup_sweep's comment). Main vs agent: delete_rule applies to agents only. Worktree vs main checkout: the rule reads paths, not repos.
Split (own line): the folder where Hydra task worktrees actually live today (HY\worktrees\, 98 of them) is on none of delete_rule's, derive-once-review's or new_worktree's lists, which all say HY\.claude\worktrees. Kinds: K2/K6 (two code owners of "which worktrees folders count": delete_rule.WORKTREE_FOLDERS and cleanup_sweep.ps1:174-178), K1 (test literal, doc).

C3. The command-length limit (bash_windows_rule.py:16 MAX_CHARS = 1000, :172-174 _too_long, :211-214 refusal text).
- Owner: bash_windows_rule.MAX_CHARS ("the user's number"). _too_long measures len(scan.text) minus the commit text (git commit messages are exempt).
- Copies: docs\agents\brief-preamble.md:33 "A command over 1,000 characters"; GATES\README.md:27 (the rule's page); test_bash_windows_rule.py:53 `assertEqual(rule.MAX_CHARS, 1000)`; test_docs_numbers.py:192-196 checks the preamble holds "1,000". Confirmed ruling: docs\agents\rulings\review-and-hooks.md:40 (2026-11-06 "Exempt git commit", about commits staying under the 1,000-character limit).
- Table: exactly at limit: 1000 characters passes, 1001 is refused (test lines 49-50); the preamble says "over 1,000" (agrees). Main vs agent: README says the rule refuses in both; delete_rule/percall order: for an agent the Bash length rule is judged after delete and library checks, for main it is first among shell rules (percall.py docstring). Card writers: skip the preamble, so the limit reaches them only through the hook's message. Worktree: n/a.
- Kinds: K1; the exemption for `git commit` is in code and ruling, and the preamble line (read only first words) may not carry it; I did not check line 33's tail.
- Test holding homes equal: test_docs_numbers.py:192-196 (preamble vs MAX_CHARS) and test_bash_windows_rule.py:53 (MAX_CHARS literal).

C4. Who may use the Clone Hero key route (percall.py:16-18 KEY_TOOLS, KEY_ROUTE_TOOLS; review_rules.py:54-55 KEY_ALLOW_TEXT; test_percall_key.py).
- Owner: percall.py (_key_call) and review_rules.py:54-55 (`KEY_ALLOW_TEXT = "allow key run"`, `_KEY_ALLOW` matches with optional trailing . or ! and ignores case).
- Homes: HY\docs\agents\clone-hero-mcp.md:29 "refuses each such call unless the user has typed exactly `allow key run` in chat in this session, and each such message lets one call through"; GATES\README.md:13 and :37 ("An allow must be the whole message"); the ch MCP server's own instructions (outside scope) say "a hook refuses them unless the user has typed allow key run"; M\ch-runs-must-not-need-focus.md:17 "Never start a key-driven (SendInput) run while the user may be using the machine; ask first." percall.py:96 the refusal message uses review_rules.KEY_ALLOW_TEXT (no copy).
- Table: clone-hero-mcp.md says "exactly `allow key run`"; code accepts "Allow key run." and "allow key run!" (edge at the limit). clone-hero-mcp.md names three tools and does not mention that `ch_call` to ch_key, to ch_play_chart with a keys route, to an unreadable ch_play_chart, or to ch_call is also refused (README:13 does, percall owns "which calls count"). The memory note says "ask first"; the hook says an allow typed by the user. Main vs agent: the key check runs on both sides (percall docstring); clone-hero-mcp.md addresses "you" without saying which. Card writers: skip preamble. Worktree: n/a.
- Kinds: K3 (clone-hero-mcp says "exactly", code is looser; doc names three tools, code also refuses ch_call forms), K5 (clone-hero-mcp.md:29 spells out the hook's rule), K1.
- Tests: test_percall_key.py:42 and 147-162 pin the tool names and that KEY_ALLOW_TEXT appears in the reason (by reference to review_rules.KEY_ALLOW_TEXT, so the message and owner are held equal; the doc line is not held).

C5. When may a ruling be filed (docs\agents\rulings\README.md:18-20, 38-43; GATES\ruling_rules.py).
- Owner: ruling_rules.py:100-101 (`APPROVE, DECLINE = "File it", "Do not file"`, `QUESTION = "File this ruling in {path}?"`) and its checks (README GATES:59).
- Copies: rulings\README.md:41 states the question text and labels and "One box may hold up to four lines, one question each" (the four-line limit is not in ruling_rules.py: grep for 4/four found nothing; it is AskUserQuestion's own limit, so it has no owner in our code, K7/K1); CLAUDE.md "Rulings and this machine" (`clicks "File it"`); agent-rules.md:33 ("Nothing is recorded as the user's ruling without the user's explicit approval", with the user's 2026-11-13 words); rulings\README.md:51 the "Yes, no click" route for old Unconfirmed lines.
- Kinds: K1 (labels, question text, in README, CLAUDE.md and code), K5 (README steps 2-4 spell out what ruling_rules enforces), K7 for "up to four lines". Test: test_ruling_rules.py exercises question_for(...) and the answers (line 231), so QUESTION and labels are held by reference in the test; the README text is not held.

C6. What sync_settings.py owns (sync_settings.py:1-25).
- It says "Which part of the live file the fragment owns ... is decided in `merged` and `first_difference` below, and nowhere else", the fragment is "the reviewed source", and the script runs "from the main checkout" after a merge. Homes of the same claims: GATES\README.md:110 (two steps, command with the python path); settings.fragment.json (the data). I did not read `merged`/`first_difference` bodies to see whether the claim "nowhere else" is true; test_sync_settings.py exists and was not read.
- Python path: sync_settings.py:20 uses `python.exe -I C:\Users\Patrick\.claude\gates\sync_settings.py`; README:110 uses C:\Users\Patrick\AppData\Local\Python\pythoncore-3.14-64\python.exe. A bare `python.exe` in the script docstring vs a full path in README (K1/K2 on how to launch). Not checked against TM's tool paths (this-machine.md).

C7. percall.py's routing claim (docstring :1-6): subagent = agent_id present; main = absent; "Plan section 2 says why." Same routing is restated in GATES\README.md (many lines) and HY\docs\agents\agent-rules.md:27 (hooks check four rules). Not read further.

## D. What I searched (so a nothing counts)
Greps over HY\CLAUDE.md, docs\agents (with rulings), docs\superpowers\plans, docs\sessions, .claude\commands, tools\*.ps1/*.py and tools\ch_mcp\*.py/*.md (not node_modules), GATES\*.py/*.md/*.sh/*.ps1/*.json and tests\, ~\.claude\CLAUDE.md, agents\, model-postures.md, session_cards tool, M\*.md, and the live settings.json (no elevated, wpr, admin or key-tool entries there) for: elevated/administrator/wpr/0xC5585011/Mandatory; GENERATOR_INSTANCE/vswhere; three builds/slots/HYDRA_BUILD_PASS; hooks\state; worktrees; 1,000/MAX_CHARS; allow key run/ch_key/ch_press; File it/four lines.

## Summary of splits (each one line)
1. TM:49 says "The user has ruled that no elevated window is opened"; no line under any ## Confirmed holds it (only the card and plan quote the user).
2. The memory note check-long-captures-early.md:15 prefers ETW/WPR; TM:49 says wpr is refused from agent shells and no elevated window is allowed.
3. delete_rule.WORKTREE_FOLDERS, derive-once-review.md:197 and new_worktree.ps1 name HY\.claude\worktrees; 98 Hydra worktrees live in HY\worktrees\, which cleanup_sweep names only for the hooks repo.
4. build_slot.ps1 and bench_run.ps1 keep state in ~\.claude\hooks\state; the gates scripts use ~\.claude\gates\state; both folders exist.
5. clone-hero-mcp.md:29 says "exactly `allow key run`"; review_rules accepts "Allow key run." and "allow key run!"; the doc also names three tools while the hook also refuses ch_call forms (README:13).
6. The ruling at agents-and-workflow.md:8 says one build per session; CLAUDE.md says "one at a time"; the slot script's per-session behaviour was not checked.

## Side notes
- GATES\sync_settings.py:20 launches with bare python.exe; README:110 with a full path.
- rulings\README.md:41 "up to four lines" has no owner in our code.
