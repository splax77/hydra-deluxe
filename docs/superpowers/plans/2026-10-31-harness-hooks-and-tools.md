# Harness cleanup, part 2: hooks and tools

Date: 2026-10-31. Task HX-P2. Session 6777b546-e751-41ab-bb22-ba066e7b1412.

This is one of two plans. The other (HX-P1) covers the instructions and the memory notes. This one covers the hook code in `C:\Users\Patrick\.claude\gates\`, the two settings files, Hydra's GUI test tools, two new checking scripts and the old `hooks\` folder. The user approved planning all six recommendations of the harness review and added one rule: a handoff is an agent summary, so every claim in it is checked against the code before new work starts.

## What binds this plan

The hook rebuild rulings of 2026-10-25 still hold (memory `hook-design-audit-2026-10-25`, plan `docs/superpowers/plans/2026-10-25-hooks-from-scratch.md`). In plain words: a hook guards at the one place where the fact is known, and reads a structured field there. It never parses free text when a chokepoint exists. Style rules (sed -i, patch scripts, bullets at the end of a turn) are instructions, not hooks; the user said yes to that on 2026-10-25. Hook code has a hard line cap, `LINE_BUDGET` in `gates/tests/replay.py`, and only the user changes it. The old `hooks\` folder stays until 14 clean days after the 2026-10-25 cutover (task T13 of that plan), so not before 2026-11-08.

Every task below is sized for one executor on Opus inside 100 tool calls and 20 minutes. Hook tasks work in a worktree of `C:\Users\Patrick\.claude` under `C:\Users\Patrick\.claude\worktrees\<task>`. Hydra tasks work under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\<task>`. No two tasks own the same file, so every task without a blocker starts at once. There is one exception, named in T5: T3 and T5 each edit a different section of `docs/agents/ui-testing.md`, and the two sections share no line. T0 (the hook scrutiny) depends on nothing and runs first or alongside; its answers can change what T1, T2 and T4 should be, so the main session reads T0's results before it merges those three.

## What the files say (claims checked)

The harness review I planned from was an agent summary (a chat report, not a file). I checked each claim I built on against the files. These did not hold as written:

1. "The GUI tools have no scroll guard, only a memory note." False. Hydra's command channel already refuses a wildcard lookup of a library row's title while the search box is empty: `library_scroll_refusal` in `tests/ui/uitest_commands.cpp`, merged 2026-10-24 as SEARCH-GUARD, and the memory note `search-before-clicking-a-song` says so itself. What the guard misses is narrower: a ref that starts with `/` skips the check (`item_ref` returns it before the refusal runs), the `key` verb can send PageDown, PageUp, Home and End to the list, and a script that posts window messages from a shell bypasses the channel entirely. The 2026-10-30 incident (1,300 wheel notches) was the last kind, which no GUI tool can stop.
2. "Any new hook code must fit under the 3,100 cap." The code is already over it (the user later ruled that the exact count is not the point; see "The line count" below). `replay.line_total` counts 3,156 lines over 21 files today against `LINE_BUDGET = 3100`. The two key-route commits of 2026-10-30 (`73ee16c`, `078fba7`) took it over, and nothing said so.3. "42 one-off allow lines." 41 of the 46 allow entries in `.claude/settings.local.json` are one-off commands; the other five are three `Read` folders, `WebSearch` and one `WebFetch` domain.
4. "ExitPlanMode is a candidate chokepoint for approval." It is a real tool event, but this project almost never uses it: three of the saved transcripts hold an ExitPlanMode call, and one holds an approval ("User has approved your plan. You can now start coding."). The user's approvals are typed in chat ("yes plan all of this"). Section Q1 says what that means.
5. "The posture fired about 15 times in session 6777b546." The transcript holds 16 prompt records: 2 typed by the user (`turnOrigin: human`) and 14 agent hand-backs (`turnOrigin: peer`). The Claude Code hooks reference says `UserPromptSubmit` fires for "a background subagent reporting back to the session that started it", and its input carries no field that says where the prompt came from. So the count holds, and the fix cannot read the source from the input.

These held: the "CLEAN counts" rule is written twice (`publish_gate._signed_off`, lines 74 to 88, and `review_rules._clean_keys` with `_doc_review`, lines 160 to 183); `gates/README.md` line 98 still says an old `hooks\` gate refuses the word "patch"; `gates\state` holds no refusal log; `hooks\` holds 3,226 files on disk, 118 of them tracked; `.claude/settings.local.json` disables the `hydra` MCP server; `settings.json` registers posture, percall and waiver on every prompt.

## The line count (information only)

The user ruled on 2026-10-31, in these words: "im not too concerned about the exact number as much as efficiency and functionality. if a hook is doing something unnecessary or contains a lot of arbitrary rules that needs to scrutinized". So the hook line count is information here, not a limit that decides a cut, and Q7 of the earlier draft is answered. `LINE_BUDGET` in `gates/tests/replay.py` stays as it is, because only the user edits it, and `replay.py` keeps printing the count. No task raises it, and no task cuts a feature to meet it. What the ruling asks for instead is T0: a look at every registered check for what it protects and whether it earns its place.

For the record, `replay.line_total` counts `*.py`, `*.ps1` and `*.sh` in `gates\` and `gates\lib\*.ps1`, and not `gates\tests\`. I ran it: 3,156 lines over 21 files, 56 over `LINE_BUDGET = 3100`. The housekeeping scripts are 839 of them (`cleanup_sweep.ps1` 494, `session_cards_start.ps1` 345), so 2,317 without them.

By my estimate from reading the code, T1 adds about 14 counted lines and its two cuts remove about 17, T2 adds about 10 and T4 about 18, a net of about +25. These are estimates; each hook task puts the measured before and after in its commit body so the reader sees them. T1's two cuts stay in the plan for a different reason than the cap: each removes a second copy of a rule (derive-once).

## Tasks

### T0. Scrutinize every registered hook check (read-only; depends on nothing)

**Goal.** The user's ruling of 2026-10-31 ("if a hook is doing something unnecessary or contains a lot of arbitrary rules that needs to scrutinized") asks for a look at what each hook does and whether it earns its place. This task produces that look and changes no file.

**Files.** None. Each agent is read-only and returns its answer in its report; the main session turns the answers into questions for the user.

**Design.** One read-only scout (Sonnet) per hook family, so the four run in parallel: (1) `percall.py` with `progress.py`, `delete_rule.py`, `media_rule.py`, `stall.py`, `posture.py` and `waiver.py`; (2) `review_dispatch.py` and `agent_stop.py`; (3) `publish_gate.py` and `publish.ps1`; (4) `ref_gate.sh`, `ref_judge.py` and `review_rules.py`; (5) the two SessionStart scripts, `cleanup_sweep.ps1` and `session_cards_start.ps1`. The registered hooks are the ones in `settings.json`, and a check is one refusal, warning or notice a script can produce. For every check the scout states:
- what it protects, in one plain sentence;
- the user ruling behind it, quoted with its file and line, or the words "no ruling found" and where it looked;
- every number or pattern it hard-codes (a time, a count, a regex, a path list) and where that value came from, with a quote, or "no source found";
- what it costs per call (a process start, a git call, a file read), by reading the code;
- whether the refusal log of T1 can show it ever fires: the log carries the script name and the first 300 characters of the reason, so a check whose reason text is shared with a neighbour in the same script cannot be told apart, and the scout says so.

Each check leaves by one of the four exits of `docs/agents/audit-method.md`, with that page's evidence rules: keep (a ruling quoted, or it protects something the scout names and a fresh agent agrees), cut (no ruling found and nothing it protects), or a question to the user (a behaviour the user ruled on, or a number nobody chose). Only the main session picks the exit. Two things are already decided and are not reopened without a question: the `cleanup_sweep.ps1` and `session_cards_start.ps1` scripts "stay uncut" (memory `hook-design-audit-2026-10-25`), and the typed `allow library run` stays (the same note). A scout that sees a problem outside its family writes it down with file and line, as a side note.

**Acceptance criteria.**
- AC1: every check in the five families has a row with all five items above, or the words "not found" for an item, with where the scout looked.
- AC2: each ruling quote is greppable: the main session greps the quoted words in the cited file and line before it accepts a "keep".
- AC3: no scout edits a file (the report is the only output).

**Verify.** The main session spot-checks three rows per family against the code, as rule 9 asks.

### T1. Every refusal writes one log line (hooks)

**Goal.** A later review can count which gate refused what, and which gates never fire. One structured line per refusal, in one file, from the one place every hook's answer passes through.

**Files.** `gates/common.py`, `gates/ref_judge.py`, `gates/publish_gate.py`, `gates/percall.py`, `gates/tests/test_common.py`, `gates/tests/test_ref_judge.py`, `gates/tests/test_publish.py`, new `gates/tests/test_refusals.py`.

**Design.** Every Python hook returns its answer through `common.run`. That function sees the input (`session_id`, `agent_id`, `tool_name`, `hook_event_name`) and the output, so it is the one home of the log. When the output is a deny, or an ask, it appends one line to `gates/state/refusals.jsonl`: `{"time", "hook": <script file name>, "event", "tool", "session", "agent", "decision": "deny" | "ask", "reason": <first 300 characters>}`. The git-side judge does not go through `run`: `ref_judge.main` prints its one verdict line, so the same append goes beside the `DENY` print there, with `hook: "ref_judge.py"`, `event: "ref"`, `tool: <the ref name>`. `publish.ps1` asks `publish_gate.main --check`, which prints the reason and returns 1; that path logs too, with `event: "publish"`. A crash in the logging never changes the decision: the append sits in its own `try`, and a failed append adds "(refusal log failed: ...)" to the reason text so the loss is visible. `common.run` today calls `body(data)` and has no `root`, so `run` gains an optional `root=None` argument that it passes only to `log_refusal` (tests pass a temp folder; the hooks' own `__main__` lines pass nothing). A crash that `run` turns into a deny is a deny too, so it is logged the same way.

Two cuts pay for it. `publish_gate._signed_off` recomputes what `review_rules._clean_keys` and `review_rules._doc_review` already decide (the "latest verdict for the key is CLEAN" rule); it becomes one call into `_clean_keys` plus the path comparison it alone owns (about 7 to 11 lines cut). The two rules are not word for word the same today: `_signed_off` lets any earlier CLEAN verdict with a path count, while `_clean_keys` keeps only the latest verdict per key. After the cut, a CLEAN verdict that a later BLOCKED verdict for the same key overrode no longer counts for the path rule. That is the rule the merge gate already uses, and `test_publish.test_blocked_after_clean_path_does_not_count` (new) pins it. `percall._library_refusal` and `percall._key_refusal` share one shape (find the call, use an allow, else refuse); they become one `_allow_refusal(data, kind, is_allow, text)` with two short callers (about 6 lines cut).

No reader script. A refusal line is plain JSON, and the README gives the one PowerShell line that counts them by hook. A script would be more counted lines.

**Acceptance criteria.**
- AC1: a deny from any Python hook appends exactly one line with the eight fields above (`test_refusals.test_deny_writes_one_line`, run through `common.run` with a body that denies).
- AC2: an ask from the main session's media guard appends a line with `decision: "ask"` (`test_refusals.test_ask_writes_line`).
- AC3: an allow writes nothing (`test_refusals.test_allow_writes_nothing`).
- AC4: `ref_judge` writes a line on every refusal: the fixture deny of `--selftest`, a missing trailer, a non-descendant move and a refused move of main (`test_ref_judge.test_refusal_logged`, one assertion per case; the test fails if any case leaves the log unchanged).
- AC5: `publish_gate.main --check` on an unreviewed long text writes a line with `event: "publish"` (`test_publish.test_check_logs_refusal`).
- AC6: a failing append (the state folder made read-only, or a `root` that is a file) still returns the deny, and the reason ends with "(refusal log failed:" (`test_refusals.test_log_failure_keeps_deny`).
- AC7: the cut `_signed_off` gives the same answers as before on the existing `test_publish` cases (they pass unchanged), and the one deliberate difference above is pinned by `test_blocked_after_clean_path_does_not_count`.
- AC8: the merged `_allow_refusal` keeps every existing library and key-route test passing (the tests that import `percall`, found with `grep -l percall gates/tests/*.py`).
- AC9: the task's commit message states the line count from `replay.line_total` before and after, as information. The log has to stay small and simple, because the user asked for efficiency: one function, no reader script, no new state beyond the one file.

**Steps.** Cut a worktree `C:\Users\Patrick\.claude\worktrees\hx-refusal-log` on branch `hx-refusal-log`. Write the tests red first (AC1 to AC6). Add `common.log_refusal(data, out, root)` and call it from `run`; thread `root` the way the other state writers do. Add the two other call sites. Make the two cuts. Run the named test files. Count lines with `python -I -c "import sys; sys.path.insert(0,'gates/tests'); import replay; print(replay.line_total('gates'))"` and put both numbers in the commit body.

**Verify.** `python.exe -I -m unittest discover -t . -s gates/tests -p test_refusals.py`, then the same with `test_common.py`, `test_ref_judge.py`, `test_publish.py`, and each file `grep -l percall` names. All green, and the line count in the commit body. `common.py` is shared code, so the main session runs every hook test at the merge (`gates/README.md`, "Running the tests").

### T2. The posture goes out less often (hooks; starts after Q8 is answered)

**Goal.** `posture.py` stops re-sending the 550-word posture on every prompt, including the 14 agent hand-backs of a typical session.

**Files.** `gates/posture.py`, new `gates/tests/test_posture.py`.

**Design.** The input has no source field (the hooks reference lists `session_id`, `prompt_id`, `transcript_path`, `cwd`, `permission_mode`, `hook_event_name`, `prompt`), and the transcript record that says `turnOrigin: human` is written after the hook runs (`waiver.py` says the same for its own reason). So the hook cannot tell a typed prompt from a hand-back, and reading the prompt's text for a hand-back header would be free-text parsing. What it can read is time. The hook keeps `gates/state/posture/<session_id>.json` with the time it last sent the text. It sends on the first prompt of a session and again whenever the last send is older than `RESEND_MINUTES = 60`. Otherwise it returns nothing. The two numbers live at the top of `posture.py`, like `progress.py`'s. The 60 is my pick, not a user decision; the task brief names it as such, and the user settles it in Q8 before the build. The project `CLAUDE.md` says the main session gets the posture "each prompt", so this task changes what that sentence says; HX-P1 owns the sentence and has to follow Q8's answer. The posture text is also lost when a long session compacts, and a time rule cannot see that; Q8 says so.

**Acceptance criteria.**
- AC1: the first prompt of a session returns the posture text and writes the state file (`test_posture.test_first_prompt_sends`).
- AC2: a prompt 5 minutes later returns nothing and leaves the file as it was (`test_posture.test_recent_send_is_skipped`).
- AC3: a prompt 61 minutes later sends again and updates the file (`test_posture.test_resend_after_interval`).
- AC4: a missing or empty posture file still produces the systemMessage, every time, and writes no state (`test_posture.test_missing_file_is_loud`).
- AC5: a state file that is not a JSON object is replaced, not crashed on (`test_posture.test_bad_state_is_replaced`).
- AC6: `body` takes `now` and `root` so the tests never touch live state (checked by AC1 to AC5 running on a temp root).

**Steps.** Worktree `hx-posture`. Tests red. Add the state read and write with `common.read_json` and `common.write_json_atomic`. Keep the "loud" mode.

**Verify.** `python.exe -I -m unittest discover -t . -s gates/tests -p test_posture.py`.

### T3. The real-app check leaves a record (Hydra)

**Goal.** When Claude opens the real Hydra window on a snapshot of the user's library, the helper writes a structured record saying which commit it checked. That record is the fact the merge gate (T4) reads.

**Files.** `tools/hydra_ui_mcp.py`, `tools/test_hydra_ui_mcp.py`, `docs/agents/ui-testing.md` (the "Live mode and the MCP server" section only).

**Design.** `hydra_start('window')` is the one way the real window opens on the library snapshot (`snapshot_library` is the only code that knows the library folder). A window session ends in four places in the helper, and three of them write the record: `Helper._stop_session` (covers `hydra_stop`, `hydra_stop(keep_window=True)`, where the window stays open but the helper lets go of the session, and the stop that a new `hydra_start` makes first), `Helper._kill_report` when the window exited with code 0 without the helper sending quit (the user closed it), and `Helper.shutdown` at helper exit. A crash, a silent window the helper had to kill, or a nonzero exit writes nothing, because a session that died is not a check. The helper appends one line to `realapp.jsonl` in the repo's git common dir (`git rev-parse --git-common-dir` of the folder that holds `tools/hydra_ui_mcp.py`), so every worktree of the repo shares one file and git ignores it without a `.gitignore` entry. The line: `{"kind": "realapp", "commit": <HEAD of that repo>, "dirty": <true when git status --porcelain prints anything>, "commands": <count of commands run in the session>, "shots": <screenshots taken>, "started", "stopped"}`. Headless sessions write nothing. The reply of the stop names the file and the commit, so the main session can quote it.

Known weaknesses, said out loud. Hydra's exe carries no git stamp (`src/core/version.h` and `app_version.h` hold a version, not a commit), so the record names the tree's HEAD, not the build; a stale `Hydra.exe` would be recorded under the current commit. Stamping the exe is out of scope and listed below. Also, the record shows that a window was opened on a clean tree and that commands ran; it does not show what anyone looked at. And when the user checks the app themselves in a copy of the exe (memory `branch-window-check-recipe`, "When the user drives the check themself"), there is no helper session and so no record. That case uses the waiver in T4.

**Acceptance criteria.**
- AC1: a window session that ran two commands and one screenshot appends one line with the seven fields, `dirty` false, on a clean temp repo (`test_hydra_ui_mcp.test_window_stop_writes_realapp_record`, using the fake launcher and a temp git repo made in the test).
- AC2: a headless session appends nothing (`test_headless_writes_no_record`).
- AC3: a dirty tree records `dirty: true` (`test_dirty_tree_is_recorded`).
- AC4: a session the user closed (the `_kill_report` path: exit code 0, no quit sent) still writes the record when the helper notices (`test_user_close_writes_record`), and a crashed or killed window writes nothing (`test_crash_writes_no_record`). A `keep_window=True` stop writes the record (`test_keep_window_writes_record`).
- AC5: the stop reply names the file path and the commit (`test_stop_reply_names_record`).
- AC6: the docs section states the record's fields, where the file is and what `dirty` means, and `hydra_tests.exe "-tc=docs match code*"` stays green.

**Steps.** Worktree `hx-realapp-record`. Tests red. Add `realapp_record(session)` beside `snapshot_library`; count commands and shots in `Session`. Write it in `_stop_session` for window mode. Update the docs section.

**Verify.** `py -I tools/test_hydra_ui_mcp.py` (the helper's suite, fake transport only; `HYDRA_LIVE_WINDOW_TEST` unset), and `build-cpp\Release\hydra_tests.exe "-tc=docs match code*"`.

### T4. A UI change needs a real-app record before main moves (hooks)

**Goal.** The ref gate refuses a move of Hydra's `main` that changes `src/ui/` unless a real-app record (T3) names a commit in the range, with a clean tree. The user can waive it the way the two review gates are waived.

**Files.** `gates/review_rules.py`, `gates/tests/test_review_rules.py`.

**Design.** `explain_main_move` already lists the changed paths and knows `base` and `new`. Add `UI_CODE = re.compile(r"(?i)^src/ui/")` beside `BUILTIN_CODE`. When any changed or deleted path matches, read `<repo git common dir>/realapp.jsonl` with `common.records`, and accept when a record with `dirty` false names a commit in `git rev-list base..new` (for a created main, any record). Otherwise add a problem: "N file(s) under src/ui change and no real-app check is recorded for a commit on this branch. Instead: open the branch's build with hydra_start('window') on the library snapshot, look at the change, and stop the session; the helper writes the record. Only the user can skip it, by sending just this as a message: waive real-app <new[:10]>." Add `"real-app"` to `WAIVER_PHRASE` with the 40-hex form; `waiver.py` loops that dict, so it records the new kind with no edit, and `waived("real-app", new, root)` already does the typed-by-the-user check. The record check is one function, `realapp_checked(repo_dir, base, new)`, so the tests call it alone (`ref_judge.py` has no dry-run mode; its modes are `ref`, `push` and `--selftest`). The refusal text builds the phrase with the existing `waiver_phrase("real-app", new)`. The accepted record must also have `commands` of at least 1, because a window that was opened and closed with nothing run in it looked at nothing; the record proves a window was opened on that clean commit and used, not what anyone saw, and a later commit on the branch is not covered by an earlier check (the user can waive). The waiver is a whole-message typed phrase, matched the way the two review waivers and `allow library run` already are; no other text is read. T4 starts after T3 is merged and its field names are fixed, so the contract is read from the merged helper.

This gate reads a file the Hydra helper writes. The field names are the contract: T3 writes them, T4 reads them, and this plan is the one place both are written down.

**Acceptance criteria.**
- AC1: a move that changes `src/ui/x.cpp` with no record is refused, and the reason holds `waive real-app` and the ten-character key (`test_review_rules.test_ui_change_needs_realapp_record`).
- AC2: the same move with a clean record for a commit in `base..new` is allowed (`test_realapp_record_in_range_allows`).
- AC3: a record for a commit outside the range does not count (`test_realapp_record_outside_range_refused`).
- AC4: a record with `dirty: true` does not count (`test_dirty_realapp_record_refused`).
- AC5: a typed `waive real-app <prefix>` allows the move, and a prefix that does not match the new tip does not (`test_realapp_waiver`), both through a fixture transcript the way the other waiver tests do it.
- AC6: a move that changes `src/core/` only never reads the record file (`test_non_ui_change_skips_realapp`, with no file present).
- AC7: a refused move with a code review in place still names the real-app problem, and the review problem when both are missing (`test_realapp_and_review_problems_both_named`).
- AC8: the existing `test_review_rules` cases pass unchanged.

**Steps.** Worktree `hx-realapp-gate`. Tests red on a fixture repo with a `src/ui/` file. Add the regex, the phrase, `realapp_checked` and the problem text. Count lines before and after; put both in the commit body.

**Verify.** `python.exe -I -m unittest discover -t . -s gates/tests -p test_review_rules.py`, and `python.exe -I gates/ref_judge.py --selftest` prints `ref-judge selftest: allow ok, deny ok`.

### T5. Close the two gaps in the library scroll guard (Hydra)

**Goal.** The command channel refuses every way it has of scrolling the unfiltered library, with a reason that says to search first. Two ways slip through today.

**Files.** `tests/ui/uitest_commands.cpp`, `tests/ui/uitest_live.cpp` (its test list), `tests/ui/uitest_engine.h` and `tests/ui/uitest_harness.cpp` (a new `EngineHooks` entry that says whether the library is unfiltered, because the existing `is_library_row_title` takes a label and cannot answer that), and `docs/agents/ui-testing.md` (only the "Two guards" paragraph in "Command files", which becomes three guards, and the `hydra_key` row of the tools table). T3 edits a different section of that page, "Live mode and the MCP server"; the two sections share no line.

**Design.** Gap one: `item_ref` returns a ref that starts with `/` before `library_scroll_refusal` runs. Move the refusal ahead of that early return, so an absolute ref whose last label is a library row's title is refused the same way while the search box is empty. Gap two: the `key` verb sends any chord to the focused widget; PageDown, PageUp, Home and End scroll a focused list. When the focused window is the library's scrolling child (its name holds `##library`, the same test `library_scroll_refusal` uses) and the search box is empty, refuse those four keys with: "refused: <key> would scroll the whole library and the search box is empty; type into **/##search first, or ask the user". Arrow keys stay allowed: Up and Down move one row and are how the GUI tests step through results. The Python helper changes nothing, because Hydra owns every command rule (ADR 0030, `docs/agents/ui-testing.md`).

What this cannot cover, said in the task's docs line: a shell script that posts wheel messages to the window. That is a shell command, and the 2026-10-25 ruling says no gate parses free shell text. The instruction in `search-before-clicking-a-song` keeps binding there; HX-P1 owns that text.

**Acceptance criteria.**
- AC1: `click /<absolute path to a library row>` with the search empty is refused with the existing reason (`library-scroll-guard-absolute-ref` live test in `uitest_live.cpp`).
- AC2: the same click after `type **/##search <title>` goes ahead (same test, second half).
- AC3: `key PageDown` with the list focused and the search empty is refused; `key Down` is not; `key PageDown` with the search filled is not (`library-scroll-guard-keys` live test, three assertions).
- AC4: `key PageDown` with a report window focused is not refused (same test, fourth assertion), so the guard reads the focused window and not the verb alone.
- AC5: the existing live tests `command-row-click-refused` and `command-row-click-after-search` (`uitest_live.cpp`; there is no test called `library-search-guard`) still pass.

**Steps.** Worktree `hx-scroll-guard`. Build once with `.\build_cpp.ps1 -Target hydra_uitest`. Write the two live tests red. Move the refusal; add the key check with the four keys as one table beside `key_named`'s table (the table `parse_chord` reads). Rebuild (warm) and run only the named tests.

**Verify.** `build-cpp\Release\hydra_uitest.exe library-scroll-guard-absolute-ref`, `... library-scroll-guard-keys`, and the existing search-guard test by name. Never `--all`.

### T6. The handoff checker (Hydra)

**Goal.** A script that takes a handoff file and checks every claim of existence in it against the repo, so a session starting from a handoff sees what still holds before it trusts a word. This is the user's new rule, in their words: "before starting any new work, every claim in the handoff needs to be checked against what actually exists. the code is the source of truth."

**Files.** new `tools/check_handoff.py`, new `tools/test_check_handoff.py`, new `tools/doc_tokens.py` (the tokeniser and the shape classers; T7 imports it, so T7 starts after T6 is merged and no copy of it exists). It lives in Hydra's `tools/` and not in the hooks repo's `gates/`, because it is a checking script and not a hook; the `--repo` flag lets it check a hooks-repo handoff too.

**Two modes, one command.** The instructions plan (HX-P1) gives new handoffs a `## Claims` table with the columns Kind, Value, Claim, How to check, and the kinds `commit`, `path`, `test`, `ruling` and `state`. Every older handoff has no such table. So the checker has a table mode and a scrape mode, chosen by whether the file holds a heading that is exactly `## Claims`.

*Table mode* reads only the rows under that heading. It prints one line per row: a status word, then the kind, the value and the Claim text, and nothing else, so a session can paste the output into its first status line. A `commit` row is `missing` when `git cat-file -e` fails or the value is not a full 40-character hash, `not-on-main` when the commit is not an ancestor of `main`, `not-pushed` when it is on `main` but not an ancestor of `origin/main`, else `ok`. A `path` row is `ok` or `missing` (a repo-relative path against `--repo`, an absolute path as written). `test`, `ruling` and `state` rows print `manual`, because the session runs or reads them itself. Exit 0 only when every `commit` and `path` row is `ok`; exit 1 otherwise; exit 2 on a bad argument. This is exactly the interface HX-P1's section 3 asks for, with the script's name and place changed to `py -I tools/check_handoff.py <handoff.md> [--repo <root>]`.

*Scrape mode* is the rest of this section, for handoffs with no table. The script checks that things exist; it does not read what a claim means, so the session still reads the code behind each claim. Nothing forces a session to run it: a hook could only tell that a session started from a handoff by reading the prompt's words, which the 2026-10-25 ruling forbids, so HX-P1's instruction carries that.

**Command line.** `py -I tools/check_handoff.py <handoff.md> [--repo <root>] [--since <YYYY-MM-DD>]`. The repo defaults to the one that holds the script. The date defaults to the first `YYYY-MM-DD` in the file name, else the file's git author date, else today with a warning. A handoff that names the hooks repo's root folder (`C:\Users\Patrick\.claude`) on a line is also checked there for the hashes and paths on that line, the way `review_dispatch.judged_folder` lets a brief name the other repo.

**What it reads.** Every token in backticks, plus every bare 7-to-40-character hex word and every bare path with a `/` or `\` and a file extension. Each token is classed by shape: a hex word is a commit; `claude/...`, `hx-...` or any token git knows as a branch is a branch; a token with a path separator or a known extension is a path; `name()` or `Name::name` is a function. A token that is none of these is ignored, so prose in backticks never fails the check. HX-P1 writes the handoff shape; the only thing this checker asks of it is that names sit in backticks.

**What it prints.** Three sections, in this order, then one summary line. `EXISTS`: one line per token, with where it was found (the commit's subject and whether it is on `main`; the branch's tip; the path; the file and line of the function). `MISSING`: one line per token git or the tree does not know, with the nearest name when a case-only or extension-only match exists. `CHANGED SINCE <date>`: one line per existing path or function file with the count of commits touching it since the date and the newest subject. The summary line: `checked N claims: E exist, M missing, C changed since <date>`. Exit 0 when nothing is missing, 1 when anything is, 2 on a bad argument. A function is searched with `git grep -n` on the whole tree, so a name moved to another file shows as EXISTS with its new home.

**Acceptance criteria.**
- AC1: on a fixture handoff in a temp git repo with one real commit, one real branch, one real path and one real function, every one lands in EXISTS with its location (`test_check_handoff.test_existing_claims_listed`).
- AC2: a made-up hash, branch, path and function each land in MISSING, and the exit code is 1 (`test_missing_claims_fail`).
- AC3: a path committed again after the handoff's date lands in CHANGED SINCE with count 1 and the newer subject (`test_changed_since_date`).
- AC4: a path untouched since the date is not in CHANGED SINCE (`test_unchanged_is_not_listed`).
- AC5: a backticked word that is plain prose (`` `yes` ``) is in none of the three sections (`test_prose_is_ignored`).
- AC6: a line that names the hooks repo root is checked against that repo for the hash it holds (`test_other_repo_root_on_line`, with two temp repos).
- AC7: the date is read from the file name, then the git date, then today with a warning on stderr (`test_date_sources`).
- AC8: a missing file argument exits 2 with a usage line (`test_usage`).
- AC9: the tracked handoff `docs/handoffs/2026-10-31-audit-of-audits-rest-handoff.md` (present on `main`, so present in the executor's worktree; the redesign-2 handoff is untracked and would not be) runs end to end in scrape mode against the Hydra checkout, and the executor pastes the summary line into its report, so the output format is seen once on real data.
- AC10: table mode, on a fixture handoff in temp repos with a bare `origin`: one `commit` row on `main` and pushed prints `ok`; one on a side branch prints `not-on-main`; one on `main` but not pushed prints `not-pushed`; a made-up or abbreviated hash prints `missing`; a `path` row prints `ok` or `missing`; a `test`, `ruling` and `state` row each print `manual` (`test_claims_table_statuses`).
- AC11: table mode reads only the rows under `## Claims` (a backticked hash above the heading is not checked) and prints nothing but the row lines (`test_claims_table_only_rows`); exit 1 when a commit or path row is not `ok`, 0 when all are (`test_claims_exit_code`).

**Steps.** Worktree `hx-handoff-check`. Tests red first with the temp repos built in `setUp` by `git init`, one commit and one branch. Write the tokeniser, the four classers, the three sections. Keep every git call through one `git(repo, args)` helper that drops the `GIT_*` environment the way `review_rules.git` does.

**Verify.** `py -I tools/test_check_handoff.py`, then AC9 by hand.

### T7. The pointer checker (Hydra)

**Goal.** A script that checks every file path and hook name in the memory notes and `docs/agents/` against what exists, like the docs-vs-code test does for numbers. It finds the broken pointers the review listed (`tools/ch_mcp/start_method.json`, `tools/derive_once_precheck.ps1`, `gates/doc_review_sources.ps1`, `main_move_check.ps1` in the wrong folder) and the next ones.

**Files.** new `tools/check_pointers.py`, new `tools/test_check_pointers.py`. It imports `tools/doc_tokens.py`, which T6 creates, so T7 starts after T6 is merged.

**Where it lives and why.** In the Hydra repo under `tools/`, beside `check_handoff.py`, so the two share the one tokeniser. Not in `gates/`, because it is a checking script, not a hook, and `gates/` should hold hooks only. Not as a C++ test case, because the memory notes live outside the repo and the hook names live in the other repo; a Python script reads both with no build.

**Where it runs.** Two places. By hand: `py -I tools/check_pointers.py [--memory <folder>] [--gates <folder>]`, with the defaults being the Hydra memory folder `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory` and `C:\Users\Patrick\.claude\gates`. At session start: T8 registers it as a third SessionStart command, so the main session sees broken pointers the same way it sees missing cards, and fixes them alongside the user's request (CLAUDE.md "Past sessions" sets that pattern). It is not part of the merge test list: `hydra_precheck` owns that list and its three `always:` lines are `hydra_tests.exe` filters, so adding a Python command there is a precheck change this plan does not make.

**What it reads.** Every `*.md` in the memory folder and in `docs/agents/`, plus `CLAUDE.md`. From each, every backticked token that looks like a path (a separator or a known extension) and every token ending in `.py`, `.ps1` or `.sh` with no separator (a hook name). A repo-relative path is checked against the Hydra checkout; an absolute path against the disk; a bare hook name against `gates/` and `gates/tests/`, and a name found only in the old `hooks/` folder is reported as "retired". The script prints one line per broken pointer, `file:line: token (missing | retired | found at <other place>)`, then `pointers: N checked, B broken`, and exits 1 when B is not 0. A token inside a fenced code block is checked too; one inside a line that starts with `Sessions:` is not, because card ids are not paths.

**Acceptance criteria.**
- AC1: a note naming a path that exists is not reported; one naming a missing path is reported with its file and line (`test_check_pointers.test_missing_path_reported`).
- AC2: a hook name that exists in `gates/` passes; one that exists only in `hooks/` is reported as retired; one in neither is reported as missing (`test_hook_names`, three assertions, on temp folders).
- AC3: an absolute path is checked on disk (`test_absolute_path`).
- AC4: a `Sessions:` line is skipped (`test_sessions_line_skipped`).
- AC5: exit code 1 with broken pointers, 0 with none (`test_exit_code`).
- AC6: with the flag `--session-start` (the form T8 registers), it exits 0 always and prints one JSON line only when something is broken, else nothing. The line is `{"hookSpecificOutput": {"hookEventName": "SessionStart", "additionalContext": <the broken pointers>}}`, because `additionalContext` is the part the model reads (`session_cards_start.ps1` uses the same shape for the same reason), and a `systemMessage` alone would reach only the user. A crash inside prints a `systemMessage` saying the check did nothing, and still exits 0 (`test_session_start_output`).
- AC7: run against the real memory folder and `docs/agents/`, it reports at least the four known broken pointers above, and the executor pastes the summary line into its report. This is a read-only run; fixing the notes is HX-P1's work.

**Steps.** Worktree `hx-pointer-check`, cut after T6 is on `main`. Tests red on temp folders. Import the tokeniser from `tools/doc_tokens.py`; do not copy it.

**Verify.** `py -I tools/test_check_pointers.py`, then AC7 by hand.

### T8. Wiring and the README (hooks; blocked on T1, T2, T4, T7)

**Goal.** Register the pointer checker at session start, and make `gates/README.md` describe the hooks as they are after T1, T2 and T4.

**Files.** `settings.json`, `gates/settings.fragment.json`, `gates/README.md`, `gates/tests/test_docs_numbers.py`.

**Design.** A third SessionStart entry, `python.exe -I -S C:\Users\Patrick\Downloads\Hydra\hydra-test\tools\check_pointers.py --session-start`, timeout 15, no `onFailure` (it only informs). The fragment mirrors it. The README gets: the refusal log and its fields, with the one PowerShell line that counts lines by hook; the posture's resend rule and its two numbers' home; the real-app record and gate, with the waiver phrase; the pointer check registration; and the stale sentence at line 98 replaced (no old hook is live, so a test file may hold any name). `test_docs_numbers.RETIRED` and `DOCS` read the Hydra agent docs and never the gates README, so they cannot catch it; a new test, `test_readme_names_no_old_hook_as_live`, reads `gates/README.md` (found through `GATES_DIR`) and fails on the phrase "one of its live hooks".

**Acceptance criteria.**
- AC1: `settings.json` and the fragment parse and hold the same `hooks` block (`test_docs_numbers.test_fragment_matches_settings`, new).
- AC2: `test_docs_numbers.test_readme_names_no_old_hook_as_live` (new) fails on the old line 98 wording and passes on the new README (the executor runs it red on the old README first and keeps the red line). The existing `test_no_retired_words` does not read the README, so it is not the check.
- AC3: the README names `refusals.jsonl`, `realapp.jsonl`, `RESEND_MINUTES` and `check_pointers.py` (`test_docs_numbers.test_readme_names_new_records`, new, one assertion each).
- AC4: the executor pipes `{"hook_event_name": "SessionStart"}` into the exact command line that `settings.json` now holds. With no broken pointer it prints nothing; with a broken pointer planted in a temp folder passed as `--memory` (the settings line itself carries no `--memory`) it prints one `additionalContext` line. The executor pastes both outputs in its report.

**Steps.** Worktree `hx-wiring`, cut after T1, T2 and T4 are on `main` of the hooks repo and T7 on Hydra's. Edit, run the test file, do AC4.

**Verify.** `python.exe -I -m unittest discover -t . -s gates/tests -p test_docs_numbers.py`.

### T9. Remove the old `hooks\` folder (hooks; blocked until 2026-11-08 and T8)

**Goal.** Task T13 of the 2026-10-25 plan: after 14 clean days, delete `hooks\` in one reviewed commit. "Clean" means the cutover commit (`11b80b1` on the hooks repo's `main`) was never reverted and `settings.json` has pointed at `gates\` the whole time; the executor checks that with `git log --oneline 11b80b1..main -- settings.json` and reads every subject.

**Files.** `hooks/` (git rm of the 118 tracked files), `gates/README.md` (the sentences that name the folder: line 43, "carried over from the old `hooks\` folder", and the rollback paragraph at line 122; T8 will have rewritten line 98), `gates/tests/test_docs_numbers.py` (one new test) and a new `gates/tests/test_replay_old_state.py`. `gates/tests/replay.py` is not edited.

**Design.** `git rm -r hooks` removes the tracked files. The untracked rest (`hooks\state`, `hooks\_archive`, the retired `pipeclient.exe` copies) is outside any worktree, so the executor never deletes it; the task's report tells the main session to remove `C:\Users\Patrick\.claude\hooks` with `Remove-Item -Recurse -Force` after the merge, which the user's ask rule confirms. `replay.py` reads `hooks\state\doc_review\reviews.jsonl` through `OLD_STATE`, but it does so with `test_review_rules._jsonl`, which already returns an empty list when the file is missing, so the folder's removal needs no code change; the test below pins that. The README's rollback paragraph says the old folder is gone and rollback is the `git revert` of the gates commits alone.

**Acceptance criteria.**
- AC1: `git ls-files hooks` prints nothing on the branch.
- AC2: with `OLD_STATE` pointed at a folder that does not exist, `replay.convert_publish_records` returns 0 and does not raise (`test_replay_old_state.test_missing_old_state_carries_zero`, on a temp root).
- AC3: `test_docs_numbers.test_no_retired_words` passes, and the README holds no `hooks\` path other than git's own `.git\hooks\` (`test_docs_numbers.test_readme_names_no_old_folder`, new; the README line about the git shim names `.git\hooks\reference-transaction`, which stays).
- AC4: the executor's report quotes the `git log` line that proves the clean window, and the date of the commit is 2026-11-08 or later.

**Verify.** `python.exe -I -m unittest discover -t . -s gates/tests -p test_replay_old_state.py` and `-p test_docs_numbers.py`.

### T10. Conditional: the project settings file (Hydra; only on the user's answers to Q4 and Q5)

**Goal.** Apply what the user decides about the 41 one-off allow lines and the disabled `hydra` server.

**Files.** `.claude/settings.local.json`.

**Design.** If Q4 is yes: keep the five general entries, drop the 41 one-offs. If Q5 is "enable": remove `"disabledMcpjsonServers": ["hydra"]`. If Q5 is "keep disabled": no edit, and HX-P1's CLAUDE.md text has to stop telling agents to use that server (reported to the user as a cross-plan item). This file is not under git's `main` gate (it is ignored), so the main session edits it after the user's answer with no review; the task is one edit and needs no executor.

**Acceptance criteria.**
- AC1: the file parses, and `(Get-Content .claude/settings.local.json | ConvertFrom-Json).permissions.allow.Count` is 5 after a yes on Q4 (the main session runs it and pastes the number).
- AC2: with Q5 answered "enable", the `hydra` tools appear in a new session's tool list (the main session checks `hydra_start` is callable).

### T11. Conditional: approval before build at ExitPlanMode (hooks; only on a yes to Q1 option B)

**Goal.** An executor dispatch is refused until a plan was approved through plan mode in this session.

**Files.** new `gates/plan_approved.py`, `gates/review_dispatch.py`, new `gates/tests/test_plan_approved.py`, `settings.json` and the fragment (a `PostToolUse` entry for `ExitPlanMode`), `gates/README.md` (one paragraph; this task runs after T8 and T9 so the file is free).

**Design.** `PostToolUse` on `ExitPlanMode` carries `tool_response` with `plan` and `filePath` only when the user approved (a refusal is a tool error and never reaches `PostToolUse`). The hook writes `{"kind": "plan-approved", "session", "time", "plan_sha256", "path"}` to `gates/state/approvals.jsonl`. `review_dispatch.py` already runs on every `Agent` call; it refuses `subagent_type: executor` when the session has no approval line, with: "No plan was approved through ExitPlanMode in this session. Instead: show the plan with ExitPlanMode and wait for the user's approval, or ask the user to type: waive approval". `Workflow` launches are not read: their input is a script, which the ruling says not to parse. Planner, scout, reviewer and card-writer dispatches pass. About 35 counted lines, by estimate.

**Acceptance criteria.** AC1: an executor dispatch with no approval line is refused (`test_executor_needs_approval`). AC2: after one `PostToolUse` ExitPlanMode input, the same dispatch passes (`test_approval_allows_executor`). AC3: a scout dispatch passes with no approval (`test_scout_passes`). AC4: an approval from another session does not count (`test_other_session_ignored`). AC5: a `PostToolUse` input with no `tool_response.plan` writes nothing (`test_no_plan_no_record`).

**Verify.** `python.exe -I -m unittest discover -t . -s gates/tests -p test_plan_approved.py` and `-p test_review_dispatch.py`.

## Questions for the user

Each question states the rule on both sides in the words the files use, an example from each end, the options, a recommendation and the strongest case against it.

**Q1. Approval before build: is there a chokepoint?** CLAUDE.md rule 3: "No work starts without the user's approval. Every plan and every build waits for the user's yes before it begins." The 2026-10-25 ruling, as memory `hook-design-audit-2026-10-25` states it: "Never add a gate that parses free text when a chokepoint exists." The only tool event that means "the user approved" is `ExitPlanMode`: its `PostToolUse` fires with the plan only after the user approves. But this project does not use plan mode: 3 of the 119 saved transcripts hold an ExitPlanMode call (counted on 2026-10-31 by the tool's name in the call records), and plans are shown as committed files and approved in chat ("yes plan all of this", 2026-10-31). A typed yes is free text. So today there is no chokepoint for the approvals you actually give, and a hook that reads ExitPlanMode would refuse every executor you approve in chat. Example of the rule being set: the card `2026-10-04-1f13cd5d` records "if anything changes a display in the app or a record, run it by me". I found no card where an executor ran with no approval at all; the failures on record are builds that went past the approved words (memory `build-what-was-asked-not-a-nearby-mechanism`), and a gate on the approval event would not catch those. Example of what a plan-mode gate would do to you: this session's two typed approvals would both have been refused. Options: (A) keep rule 3 as an instruction, with the posture text (HX-P1) carrying it, and plan no hook; (B) build T11 and move to plan mode for every build, so every approval is an ExitPlanMode click; (C) a typed phrase like `approve build`, matched whole the way `allow library run` is, recorded by `waiver.py` and spent by the next executor dispatch. Recommendation: A now, and B if you are willing to approve through plan mode. The strongest case against A: rule 3 is an instruction, and instructions are what you have had to restate; I did not count how often rule 3 itself was broken. The case against C: a fixed phrase makes every "yes" you type today a refusal until you retype it, a misfire of the kind you called "this is just bad design" about exact-text matching (card `2026-10-25-4672d33d`).

**Q2. Bring back the bullet-wall Stop check?** Your global CLAUDE.md: "no dense nested-bullet walls" and "Before showing any plan ... check if it looks like the BAD example". The 2026-10-25 plan listed `stop_bullet_check` among the hooks "with no replacement and no objective behind them", and you said yes to style rules becoming instructions only. The cards show the check caught real bullet walls in 2026-09-06-d363bf9f and 2026-09-21-1136a257 (in both, a Stop hook blocked a reply for being mostly bullets). I did not count how often the rule was broken while no hook ran. A Stop hook reads the reply text, so it is free-text parsing by the ruling's words, though the pattern (a run of lines starting with `-` under a heading) is narrow. Options: (A) stay with the ruling: instruction only; (B) bring the check back as a Stop hook, about 40 counted lines by estimate, as a warning not a refusal. Recommendation: A, because you ruled it eight days ago, and a check that parses the reply text is the kind of arbitrary rule you asked to have scrutinized. The case against: the rule fails often, and a warning that names the offending block costs you nothing to ignore.

**Q3. Push after merge by default?** CLAUDE.md rule 10: "After a push, report done and move on." The push gate (`ref_judge.judge_push_lines`) trusts a push of gated `main`, and memory `push-gate-trusts-gated-main` says pushes were once refused wrongly. Of the 629 messages you typed that the 119 saved transcripts hold, 42 contain the word "push" (counted 2026-10-31). Options: (A) keep push as your word, every time; (B) the main session pushes `main` right after every merge; (C) it pushes once at the end of a session's merges. Recommendation: A. A push is the one step that leaves this machine, and about 42 typed messages over 119 sessions is cheap. The case against: the memory `hydra-mcp-server-2026-10` shows merged work sitting unpushed for days and a later session having to work out whether it was.

**Q4. Drop the 41 one-off allow lines?** `.claude/settings.local.json` holds 46 allow entries; 41 are whole commands from past sessions (a `Stop-Process -Id 22428`, a `mv raw_20D2440.asm ...`) that will never run again, and each one is a rule the permission system checks on every call. The five general ones (three `Read` folders, `WebSearch`, `WebFetch(domain:arxiv.org)`) do work. Options: (A) drop the 41 (T10); (B) leave the file alone. Recommendation: A. Three of the 41 are wildcards, not whole commands: `Bash(.venv/Scripts/python.exe -c ' *)` (any inline Python, and `.venv` does not exist in the checkout), `Bash(powershell.exe -NoProfile -NonInteractive -Command ' *)` (any quoted PowerShell) and one `Bash(SCRATCH_BASH=... *)`. Leaving those is more than clutter, because each allows any command that starts that way. The case against dropping them: a flow I did not find may still lean on the `powershell.exe -Command` wildcard, and dropping it would show a permission prompt the first time that flow runs.

**Q5. The disabled `hydra` MCP server.** `.claude/settings.local.json` has `"disabledMcpjsonServers": ["hydra"]`, while CLAUDE.md "GUI testing" tells every session to use `hydra_start` and the other `hydra_*` tools. The memory note `hydra-mcp-server-2026-10` records that a project server loads the helper once and cannot be reloaded after an edit, and that you said "you created the server, you figure it out"; no card I found records a ruling to disable it. Options: (A) enable it (T10), and sessions drive the real window through it (T3's record depends on it); (B) keep it disabled and have HX-P1 change CLAUDE.md to say the helper is driven through an `mcp.Client` driver instead. Recommendation: A. The case against: a server that starts with every session costs a Python start and holds the helper's code until the session ends, which bit the 2026-10-14 session.

**Q6. The overlapping Clone Hero tools.** `ch_press` and `ch_key` press keys through Windows after bringing the game to the front; `ch_kit_press` presses the virtual kit inside the game. `percall._key_call` refuses the first two without `allow key run`, and the memory `ch-runs-must-not-need-focus` says runs must never need focus. Four tools carry "status" in their name (`ch_status`, `ch_kit_status`, `ch_play_status`, `ch_sp_watch_status`), and `ch_state` and `ch_kit_watch` read nearby things (34 tools in all, counted from `tools/ch_mcp/`). Options: (A) leave the 34 tools as they are until the LS-K series ends; (B) retire `ch_press` and `ch_key` now that the kit route works. Recommendation: A, as the harness review's summary said "revisit the ch tools later": LS-K3 is still running and the key route is its fallback. The case against: every session loads 34 tool descriptions, and two of them exist to do what a rule forbids.

**Q7. The line cap. Answered by the user, 2026-10-31:** "im not too concerned about the exact number as much as efficiency and functionality. if a hook is doing something unnecessary or contains a lot of arbitrary rules that needs to scrutinized". So the count is information, no task cuts a feature to meet it, and `LINE_BUDGET` is left for the user to change or retire. The part of the answer that asks for work is T0. One thing stays open and is not decided here: whether `replay.py` should keep failing when the count is over `LINE_BUDGET`. It is a manual tool and no merge reads its exit code, so nothing breaks while it stays as it is.

**Q8. The posture resend interval.** T2 picks 60 minutes. Shorter means the main session sees the rules more often; longer means fewer tokens per hand-back wave. A session with 15 hand-backs in an hour sends once instead of fifteen times under either. Options: 30, 60, or once per session. Recommendation: 60. The case against: a long session on one plan may go two hours without a typed prompt, and the only resend lands on a hand-back. A second case against, and the stronger one: when a long session compacts (auto-compact runs near 500K tokens, `stall.py` `CTX_TEXT`), the earlier posture text is summarized away, and a time rule cannot see that, so the session could run up to an hour without the posture. A `SessionStart` entry with the `compact` matcher would be a structured signal for it, at the price of one more registered hook. Options therefore include: 30, 60, once per session, or 60 plus a reset on compaction. Whether the posture should be sent less often at all is also yours to say: CLAUDE.md describes it as what the main session "gets each prompt".

**Desktop connectors, for you to switch off in the app (no code planned).** The tool list this session carried offered connectors unrelated to Hydra: QuickBooks and payroll tools, Gmail, Google Drive, Google Sheets, Amplitude (two regions), Atlassian, BigQuery, Definite, Hex, Asana, Figma, Intercom, Linear, Notion, Slack, a Reddit browser and a browser-use plugin that failed to connect. They are app-level settings, not files in either repo.

## Out of scope (for the user to approve as such)

- Stamping `Hydra.exe` with its git commit, so T3's record could name the build and not the tree. Today the exe carries a version only.
- Any guard on a shell script that posts window messages to Hydra (the 2026-10-30 wheel incident). The ruling forbids free-shell-text gates; the instruction in `search-before-clicking-a-song` keeps it.
- A reader script for `refusals.jsonl`; the README's one-line count is the reader.
- Adding the pointer check to `hydra_precheck`'s `always:` lines.
- Timing the one-Python-process-per-call cost of `percall.py` (the findings noted it was never timed; no task here measures it).
- Any change to `cleanup_sweep.ps1` or `session_cards_start.ps1`.
- A `/handoff` command, CLAUDE.md procedures, memory notes and the posture text: HX-P1 owns them.

## Prior art

Two patterns here match what others built. Deterministic hooks at the tool-call boundary, with the runtime and not the model deciding whether the check runs, is the design argued in [Ran the Builder's post on agentic coding hooks](https://ranthebuilder.cloud/blog/agentic-coding-hooks-deterministic-ai-guardrails/) and the pure allow-or-deny policy function of [mcp-guardrails](https://pypi.org/project/mcp-guardrails/); [a DEV post on five deterministic rings](https://dev.to/pablets/guardrails-for-ai-agents-five-deterministic-rings-between-an-llm-and-real-money-2go6) puts it as "a guardrail is only a guardrail if the LLM cannot route around it". The handoff checker follows [Handover, a machine-checkable protocol for agent handoffs](https://brownlittlefish.substack.com/p/handover-a-machine-checkable-protocol) (a handoff pinned to a commit, with a linter that reports which claims no longer hold), the [doc-claim-validator skill](https://www.claudepluginhub.com/skills/nickcrew-nickcrew-claude-ctx-plugin/doc-claim-validator) (extract paths, commands and symbols from markdown, then check them, with a git staleness pass) and [TablePro PR 2175](https://github.com/TableProApp/TablePro/pull/2175), which made stale doc symbols fail lint. The Claude Code field facts come from the [hooks reference](https://code.claude.com/docs/en/hooks): `UserPromptSubmit` fires for a subagent reporting back and carries no source field; `ExitPlanMode`'s `PostToolUse` carries `tool_response.plan` and `filePath`.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| Every hook refusal writes one structured line | T1 Goal | T1 | `test_refusals.test_deny_writes_one_line`, `test_ref_judge.test_refusal_logged` (four cases), `test_publish.test_check_logs_refusal` |
| An ask is logged too | T1 Design | T1 | `test_refusals.test_ask_writes_line` |
| An allow writes nothing | T1 Design | T1 | `test_refusals.test_allow_writes_nothing` |
| A failed log never changes the decision, and is visible | T1 Design | T1 | `test_refusals.test_log_failure_keeps_deny` |
| The two cuts change no answers | T1 Design | T1 | existing `test_publish` and percall tests pass (AC7, AC8) |
| Every hook task reports its line count, as information | "The line count" section | T1, T4 | commit body numbers (T1 AC9, T4 Steps); T8 AC1 keeps settings in step |
| Every registered hook check is scrutinized for its protection, ruling, hard-coded numbers, cost and visibility in the log, and leaves by one of the four exits | T0 | T0 | AC1 (all five items per check), AC2 (each ruling quote grepped by the main session), AC3 (no file edited) |
| The posture goes out on the first prompt and then at most once per interval | T2 Goal | T2 | `test_posture.test_first_prompt_sends`, `test_recent_send_is_skipped`, `test_resend_after_interval` |
| A missing posture file is still said out loud | T2 AC4 | T2 | `test_posture.test_missing_file_is_loud` |
| Tests never touch live state | T2 AC6 | T2 | every test runs on a temp root |
| A window session writes a real-app record with seven fields | T3 Design | T3 | `test_window_stop_writes_realapp_record`, `test_user_close_writes_record` |
| Headless sessions write nothing | T3 AC2 | T3 | `test_headless_writes_no_record` |
| A dirty tree is recorded as dirty | T3 AC3 | T3 | `test_dirty_tree_is_recorded` |
| The docs state the record | T3 AC6 | T3 | `hydra_tests.exe "-tc=docs match code*"` |
| A `src/ui/` change needs a clean record in range, else refused with the waiver phrase | T4 Goal | T4 | `test_ui_change_needs_realapp_record`, `test_realapp_record_in_range_allows`, `test_realapp_record_outside_range_refused`, `test_dirty_realapp_record_refused` |
| The user can waive it with a typed phrase | T4 Design | T4 | `test_realapp_waiver` |
| Non-UI changes never read the record | T4 AC6 | T4 | `test_non_ui_change_skips_realapp` |
| Both problems are named when both are missing | T4 AC7 | T4 | `test_realapp_and_review_problems_both_named` |
| An absolute ref to a library row is refused while the search is empty | T5 Design | T5 | `library-scroll-guard-absolute-ref` |
| PageDown, PageUp, Home, End are refused in the unfiltered list; arrows are not; other windows are not | T5 Design | T5 | `library-scroll-guard-keys` (four assertions) |
| The existing search guard still holds | T5 AC5 | T5 | the existing live test by name |
| The handoff checker prints EXISTS, MISSING, CHANGED SINCE and a summary, exit 1 on missing (scrape mode) | T6 "What it prints" | T6 | `test_existing_claims_listed`, `test_missing_claims_fail`, `test_changed_since_date`, `test_unchanged_is_not_listed` |
| With a `## Claims` table the checker prints one status line per row (ok, missing, not-on-main, not-pushed, manual) and exits 0 only when every commit and path row is ok | T6 "Two modes" | T6 | `test_claims_table_statuses`, `test_claims_table_only_rows`, `test_claims_exit_code` |
| Prose in backticks never fails the check | T6 "What it reads" | T6 | `test_prose_is_ignored` |
| A line naming the hooks repo root is checked there | T6 Command line | T6 | `test_other_repo_root_on_line` |
| The date comes from the name, then git, then today with a warning | T6 Command line | T6 | `test_date_sources` |
| The checker is seen once on real data | T6 AC9 | T6 | summary line in the executor's report |
| Every path and hook name in memory and `docs/agents/` is checked | T7 Goal | T7 | `test_missing_path_reported`, `test_hook_names`, `test_absolute_path` |
| `Sessions:` lines are skipped | T7 "What it reads" | T7 | `test_sessions_line_skipped` |
| Exit 1 when anything is broken | T7 "What it reads" | T7 | `test_exit_code` |
| At session start it speaks only when something is broken | T7 "Where it runs" | T7, T8 | `test_session_start_output`; T8 AC4 by hand |
| The four known broken pointers are found | T7 Goal | T7 | AC7 run, summary line in the report |
| The README matches the hooks and names no old hook as live | T8 Goal | T8 | `test_docs_numbers.test_no_retired_words` (red then green), `test_readme_names_new_records` |
| settings.json and the fragment agree | T8 AC1 | T8 | `test_fragment_matches_settings` |
| `hooks\` goes only after 14 clean days, in one reviewed commit | T9 Goal | T9 | AC4 (the `git log` proof and the commit date), the derive-once gate on the hooks repo |
| `replay.py` survives the folder's removal | T9 Design | T9 | `test_replay_old_state.test_missing_old_state_carries_zero` |
| No `hooks\` path is left in the README | T9 AC3 | T9 | `test_readme_names_no_old_folder` |
| No task cuts a feature to meet the line count, and none raises `LINE_BUDGET` | "The line count" section, Q7 | all | no task owns `replay.py`; the reviewer checks the diff |
| No hook parses free text | What binds this plan | T1, T2, T4, T5, T7, T11 | every new check reads a field (`tool_response.plan`, `tool_name`, changed paths, the record file, the focused window's name) or a state file; the one typed text is the `waive real-app` phrase, matched whole like the two review waivers; the reviewer checks each new predicate against this line |
| Settings changes wait for the user's answer | T10 | T10 | no executor is dispatched for T10; the main session edits after Q4 and Q5 are answered |
| An executor dispatch needs a plan-mode approval (only if Q1 = B) | T11 | T11 | `test_executor_needs_approval`, `test_approval_allows_executor`, `test_scout_passes`, `test_other_session_ignored`, `test_no_plan_no_record` |
