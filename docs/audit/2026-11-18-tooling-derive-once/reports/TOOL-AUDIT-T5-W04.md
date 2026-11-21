# TOOL-AUDIT-T5-W04 (second-round verifier; ledger L02; rows L-F05-17, L-F06-06, L-F06-12, L-F06-17, L-F06-19)

Short paths: H = C:\Users\Patrick\Downloads\Hydra\hydra-test, G = C:\Users\Patrick\.claude\gates, MP = C:\Users\Patrick\.claude\model-postures.md, PRE = H\docs\agents\brief-preamble.md. Every claim below is "by reading"; I ran nothing.

### L-F05-17
**Verdict:** CONFIRMED (the ledger's claims all hold; it adds a few pointers I could not check).
**Homes as found:**
- H\tools\replay_schedule.cpp:25 `constexpr double kFrameMs = 3.0;` (comment: "The user set it: question 1 of docs/superpowers/plans/2026-11-08-player-freak-fixes.md"; that plan file exists).
- :31 `kJudgeEarlyMs = 1 * kFrameMs;` (comment: "the user chose 'One frame', D319 item 3").
- :34 `kPressLeadMs = 2.0` ("(D51)").
- :41 `kSpEndLeadMs = 15.0` (comment cites "question 3 of the optimal-score run plan, which has left the tree (removed; git show d7e47a20:docs/superpowers/plans/2026-11-06-optimal-score-run.md)"; the plan is no longer in the tree, as the ledger says).
- :47 `kSpOutLagMs = 15.0` (cites question 2 of 2026-11-07-player-refinements.md; file exists).
- :55 `kChordOrderGapMs = 8.0` (cites the amendment after LS-R1b run 1 in the same refinements plan).
- :61 `kRoomMarginMs = 3.0` (cites question 2 of player-freak-fixes.md).
- :20 comment "The window's clamps and formula constants are core/note_window.h's"; :61-62 of the constants object use `kWindowFrontMs` and `kWindowBackMs` by name, no local copy.
- constants_json() writes all of them to the file's "constants" object. H\tests\test_replay_schedule.cpp:272-278 pins frame_ms 3.0, press_lead_ms 2.0, sp_end_lead_ms 15.0, chord_order_gap_ms 8.0 as literals.
- H\docs\agents\derive-once-review.md:64 holds the worked example "3.00 ms is the one frame of judging the tool counts". A copy of the value in prose; no test holds it to kFrameMs.
- Other 3 ms: H\src\core\rules.h:32 `double backend_leeway_ms = 3.0;` (comment: "A backend note less than this many ms after the SP end still scores under SP ... (decision D29, 2026-10-03)"); read at src\app\path_view.cpp:262,383-403.
**Splits:**
- One number, 3.0, with three questions: kFrameMs (how long the game's frame is), kRoomMarginMs (how far inside its window a squeeze press must stay), rules.h backend_leeway_ms (how late after the SP end a backend note still counts). They agree in value today. They differ in the situation where one is changed: a changed frame period (say 60 fps instead of 330 cap) moves kFrameMs and kJudgeEarlyMs together but not the other two, and the margin and leeway are separate user choices. Each is its own named constant. Not a split in value.
- Not found a second home for 15 ms/8 ms/2 ms in tools\*.cpp, src, tests (I grepped replay_schedule, frame_ms, press_lead_ms, sp_end_lead in tools, src, tests; the only hits besides the tool are ch_probe schedule JSON files and test_kit_tools.py fixtures that use `frame_ms` as a test knob, not the 3.0 value).
**Ruling:** Confirmed lines searched in H\docs\agents\rulings\clone-hero.md for "D319" and "One frame": the only Confirmed hit is line 56 (2026-11-11, "Keep them (Recommended)" to a question about removing the hit-by-any-rise rule, D319 item 1). That is D319 item 1, not item 3 ("One frame"). So the code comments quote the user's words from plan questions and the decision log (H\docs\audit\2026-10-03-fix-decisions.md:1837, D319), and I found no Confirmed line for the 3, 15, 15, 8, 3 values. None names both sides of any split.
**Open:** Whether the 15 ms, 8 ms and 3 ms values are in a Confirmed line outside clone-hero.md (I searched only that page for D319/One frame, and a grep across all rulings pages for "Only where it matters", "8 ms", "15 ms" found no Confirmed hit). The decision log D319, D51, D29 entries and the plans' question answers were not opened.

### L-F06-06
**Verdict:** CONFIRMED (three id rules, as found; two details added).
**Homes as found:**
- G\progress.py:23-27 `_STATUS_CALL` regex: the id is `[A-Za-z0-9_-]+`, bare or in double quotes. progress.py:111-115 refuses a status call whose id is not the caller's own (`status_id != agent`), quoting the right line. progress.py:80-83 appends to the caller's record only when `status_id == agent`.
- G\common.py:113-116 `check_id`: refuses empty, ".", "..", and any id containing `/`, `\` or `:`. It guards path building: used by common.py:120,124,128 (agents_dir, status path, record path), stall.py:51,180, posture.py:22, agent_stop.py:41.
- G\status_append.ps1:25-39: if the target resolves to a file directly in the status folder, its name must have a record `<name>.json` under some session folder of the agents folder, else "refused: ... is not an agent id". The comment says progress.py makes the record before the script runs. A target ending `.md` or a path outside the status folder skips this check (line 22 `$target -match '\.md$'`, and the folder test at line 29).
**Splits:**
- progress.py asks "is this call's named id the caller's own, in the allowed character set?" It answers per call.
- common.check_id asks "is this part safe to put in a path?" Accepts an id with a space or `*`; the progress regex would not accept it as a status id.
- status_append.ps1 asks "does any agent, in any session, have a record by this name?" It does not ask that the writer be that agent. Situation where they differ: a name with a record from another session passes the script's check but is refused by progress.py when the caller's id is different (progress.py runs first as a hook). Without the hook (script run by hand) the script alone lets any recorded name be written. By reading only.
**Ruling:** None found under Confirmed (I searched the rulings for "agent id", "status file"; the status requirement itself is agents-and-workflow.md "no status updates is unacceptable" per the ledger; it names no id rule).
**Open:** Whether any real agent id contains a character that check_id accepts and the regex refuses; only a run settles it.

### L-F06-12
**Verdict:** CONFIRMED-CHANGED. Three transcript-location homes exist, but the ledger's list is off in two places: check.py does not locate transcripts, and the card count and the disk count are different quantities.
**Homes as found:**
- C:\Users\Patrick\.claude\tools\session_cards\prepass.py:143-170 `workflow_tally`: folder = `<dirname(path)>/<sid>/subagents/workflows/<run>`; counts files named `agent-*.jsonl` directly in it ("folder"), else the largest task-notice count ("notice"), else None. prepass.py:250-251 counts a call named "Agent" or "Task" as a direct subagent; :278 `direct = len(meta["subagents"])`; :290 `"subagents_launched": direct + workflow_agents`.
- session_cards\common.py:68-70 holds the notice regexes (TASK_NOTICE, NOTICE_TOOL_USE, NOTICE_AGENT_COUNT `<agent_count>(\d+)</agent_count>`).
- session_cards\check.py:80-100 `refresh_agents`: does not look for transcripts. It copies `subagents_launched` from the session's meta.json into the card's `agents:` field. (The ledger's "check.py:82-100" as a transcript-location home is wrong; it reads prepass's output.)
- G\stall.py:169-171 `_session_folder` (transcript path without `.jsonl`); :174-187 `_agent_transcript`: looks for `<folder>/subagents/agent-<id>.jsonl`, then walks `subagents/workflows` for it; :303-307 `_open_workflow_target` walks the same workflows folder.
- H\tools\agent_cost.py:158-162 `agent_paths`: `(folder / "subagents").rglob("agent-*.jsonl")`, any depth.
- G\agent_stop.py:51 `_TASK_ENDED` regex on `<task-notification>...<task-id>...<status>` (a notice reader; a different question: when does a background task end).
- Tool names "Agent": G\review_dispatch.py:141 `data.get("tool_name") != "Agent"`; settings.fragment.json:158 `"matcher": "Agent"`; prepass accepts "Agent" and "Task".
- Test: session_cards\test_session_cards.py:743-750 pins (1, 3, 7, 1, 8) from its own fixture (SID and workflow folders it builds at :738-741).
**Splits:**
- The card count (prepass) = Agent/Task tool calls seen in the main transcript + workflow agents counted from the run folder or notices. The disk count (agent_cost.rglob) = every `agent-*.jsonl` anywhere under `subagents`, nested folders included. prepass counts only direct children of a run folder. Situation where they differ: an Agent call that was refused or never started has no transcript but is counted as launched by prepass; a transcript in a deeper or other subfolder is counted by agent_cost and not by prepass.
- The notice count (a notice counts only its own launch; a run stopped early sends none, prepass docstring) versus the folder count (all launches). The code states the order (folder first, then largest notice).
- Layout `subagents/[workflows/<run>/]agent-<id>.jsonl` is written three times (prepass, stall, agent_cost), each in its own words; none imports another.
**Ruling:** None found (I searched rulings pages for "agents:" and "subagent"; none under Confirmed).
**Open:** Which of the three layouts matches Claude Code's real layout for nested workflows; only a real transcript folder settles it. Whether any card's `agents:` count differs from the disk count on a real session (a run over actual folders).

### L-F06-17
**Verdict:** CONFIRMED-CHANGED. The named code owner `judged_folder` does not exist in the live gates folder; and the prose homes disagree in words with the code.
**Homes as found:**
- G\review_dispatch.py (live, the file the settings point at): functions are keys_in, brief_lines, _same_path, _folder_repo, _key_problem, _branch_at, _review_problem, body; no `judged_folder`. A grep of all .py/.md/.ps1/.json under C:\Users\Patrick\.claude for "judged_folder" finds it only in worktrees\displays-read-engine and worktrees\remove-old-hooks (older copies of review_dispatch.py :65,156,165) and a comment in G\tests\replay.py:9 ("the judged folder's state"). So the ledger's owner is stale.
- The live owner of the folder question: review_dispatch.py:22 `_REPO_LINE = re.compile(r"Review repo: (?P<folder>\S.*)")` and :99-119 `_review_problem`: the folder must be an absolute existing path, and the top folder of a git worktree (`_same_path(fr[0], folder)`, error text "is not the top folder of a git worktree"); then the key must be a commit there; the folder's checked-out branch must not be main or the start branch (error "has main checked out ... not the branch under review"). The `_INSTEAD` text (:24-26) says "<folder> is the top folder of the worktree that has the branch under review checked out".
- G\README.md:39: "The folder is the top folder of the worktree that has the branch under review checked out, in this repo or the other one."
- MP:5 (line 5 of model-postures.md): "A brief may name a commit in the other repo if it writes that repo's root folder."
- H\docs\agents\derive-once-review.md:200: "The reviewer works in the branch's own worktree, which the brief names ... Put the key ... in the brief."
**Splits:**
- MP:5 says "that repo's root folder"; code, the refusal text and README:39 say "the top folder of the worktree that has the branch checked out". For the main checkout these are the same folder. For a branch checked out in a separate worktree they differ, and the code refuses the main checkout's root if it has main checked out. Situation: a hooks-repo branch checked out in a worktree under C:\Users\Patrick\.claude\worktrees (the case the other repo's branches are in): MP:5's wording points at the repo root, which the code refuses; the worktree's own top folder is accepted.
- derive-once-review.md:200 agrees with the code (the branch's own worktree) and does not use the word "root".
**Ruling:** None found under Confirmed (I searched the Confirmed sections of all H\docs\agents\rulings\*.md for "Review repo", "other repo", "root folder", "top folder": no hit).
**Open:** None that a run is needed for; the live file was read. Whether MP:5's wording was written before the "Review repo:" line existed is a history question I did not check.

### L-F06-19
**Verdict:** CONFIRMED (as the general claim; each cited page checked).
**Homes as found:**
- PRE:33 (Shells on Windows): lists the five refused Bash shapes and the 1,000-character limit in full, then says "`bash_windows_rule.py` in the gates folder owns the rule and the number." The code agrees: G\bash_windows_rule.py:4 "owns the length limit and the five refused shapes", :16 `MAX_CHARS = 1000`.
- PRE:43 (The delete rule): spells out the delete rule at length: first root the scratch folder, second root any worktree directly inside a `worktrees` folder of either repo with the two exact paths, never the worktree's own folder, refuses variables, wildcards, `..`, quotes, spaces. G\delete_rule.py:17-18 `WORKTREE_FOLDERS` holds those same two paths; its comment (:14-16) says any worktree there counts and never a worktree's own folder.
- G\tests\test_docs_numbers.py:192-204: two tests hold PRE "## Shells on Windows" to `bash_windows_rule.MAX_CHARS` (as "1,000") and `shell_output.CAP` (as "4,000"). Lines 183-190 hold the preamble's budget numbers. A grep of test_docs_numbers.py for "worktree" and "delete" finds nothing outside a settings comment (line 47) and a skip message (line 144): no test holds PRE:43 to delete_rule.py.
- C:\Users\Patrick\.claude\tools\session_cards\writer-brief.md:101 "At 20 minutes or 100 tool calls, stop starting new cards" (typed copy; not read by any test, see L-F04-01).
- G\tests\shell_output_sizes.py:20 `SHELL_TOOLS = ("Bash", "PowerShell")` is a typed copy of progress.py:20; G\tests\test_progress.py:520 `assertEqual(percall.KEY_TOOLS, ("mcp__ch__ch_key", "mcp__ch__ch_press"))` is a typed copy of percall.py:16. Both are the test copies of hook values; the second pins a typed list on purpose.
**Splits:** None in value (PRE:43's two folders match WORKTREE_FOLDERS today). A word-level difference: MP:5 and G\README.md:136 describe the trusted folders as "a `worktrees` folder" (as the ledger L-F06-04 records); PRE:43 names both exact paths; the code has two exact folders. The test does not hold PRE:43's text, so a change to WORKTREE_FOLDERS would not fail a docs test.
**Ruling:** None found under Confirmed on the wording of PRE:33 or PRE:43.
**Open:** Whether every other sentence in PRE:43 (variable, wildcard, `..`, space) matches delete_rule.py; I read only the roots and worktree folders in the code.
**Hook gap:** yes: no test ties the preamble's delete-rule text to the delete rule's code (checked by reading test_docs_numbers.py).

## Ids not reached
None. All five rows were opened.
