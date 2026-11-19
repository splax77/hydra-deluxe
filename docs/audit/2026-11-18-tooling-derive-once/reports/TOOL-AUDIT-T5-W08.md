# TOOL-AUDIT-T5-W08 (second-round verifier, 20 rows)

Short names as in the ledger: GATES = C:\Users\Patrick\.claude\gates, HY = C:\Users\Patrick\Downloads\Hydra\hydra-test, BP = HY\docs\agents\brief-preamble.md, TM = this-machine.md, UT = ui-testing.md, CH = clone-hero-mcp.md, SF = GATES\settings.fragment.json, RJ = GATES\ref_judge.py, README = GATES\README.md, MEM = the memory folder. Everything is "by reading"; I ran nothing. Where a ledger line number has moved I say so.

Tally: CONFIRMED 13, CONFIRMED-CHANGED 7, REFUTED 0, not reached 0.

### L-F13-04
**Verdict:** CONFIRMED, by reading.
**Homes as found:** BP:43 lists six commands "such as". GATES\delete_rule.py:32-35 holds GIT_DISCARD_WORDS (reset --hard/--merge; checkout --force/-f/.; restore .; switch --force/-f/--discard-changes; branch -D) and GIT_DISCARD_ACTIONS (stash drop/clear; worktree remove/prune); :120-128 add `git clean` and `branch -d --force`. README:19 spells the long list and says the two dicts "are the one list". SF:59-64 `ask`: `Bash(rm *)`, `Bash(git clean *)`, `Bash(git reset --hard*)`, `PowerShell(Remove-Item *)`; nothing else.
**Splits:** (1) BP:43 answers "which commands are refused" with six; the code refuses about a dozen forms; differ for `stash clear`, `worktree remove`, `branch -D`, `checkout .`, `restore .`, `switch`. (2) Agent is refused outright, main session only asked, and only for the four SF patterns; differ for the main session running `git reset --merge`, `checkout .`, `restore .`, `stash drop`, `worktree remove`, `branch -D` (SF has no ask for them). (3) README:19 repeats the list in prose beside the code (second copy).
**Ruling:** README:19 quotes a 2026-10-31 line; my grep of rulings\*.md for "losing battle" is not part of this check, and the quote names no side of any split. review-and-hooks.md:17 (Confirmed section) concerns worktree folders, not the list. None names both sides.
**Open:** None.

### L-F13-07
**Verdict:** CONFIRMED.
**Homes as found:** BP:71 "Never amend, rebase, reset or move a branch" and "The ref gate refuses a move of `main`, or of any branch origin also has, that is not a plain step forward". integrate.md:31 "Never rebase, amend, reset or force-move a branch." RJ:135-137 `shared = ref == MAIN or current("refs/remotes/origin/" + name) is not None`, then `shared and not is_ancestor(cur, new)` refuses (message :99-103). RJ:118-121 "Only the user deletes main." RJ:111-115 symbolic ref refused.
**Splits:** Instructions forbid any rewrite of any branch; the gate refuses only a shared branch's non-descendant move. Differ for a local branch origin lacks. Also: a move that IS a descendant (a plain step forward) is allowed on main even when a reset-based way produced it.
**Ruling:** none found.
**Open:** None.

### L-F13-11
**Verdict:** CONFIRMED.
**Homes as found:** TM:45-49 ("Medium Mandatory Level"; "`wpr -start` was refused ... `0xC5585011`"; "The user has ruled that no elevated window is opened."), TM:63 "`cdb` attaches from this unelevated shell, unlike `wpr`". Card HY\docs\sessions\2026-11-07-7d216054.md has "i dont accept that you need an elevated run. figure out another solutio." under Rulings. Plan 2026-11-07-ch-run-timing.md:27 widens to "No elevated window, no trace, no PresentMon, no install, no machine change". Plan 2026-11-12-player-stall-fixes.md:62 "No elevated window, so no `wpr` trace". machine_recorder.py:6 "needs no administrator rights". MEM check-long-captures-early.md:15 "Prefer non-invasive capture (ETW/WPR, read-only memory reads) over hooking hot functions in a live app."; MEMORY.md:18 "prefer ETW over hooks".
**Splits:** TM:49 says "The user has ruled"; I grepped docs\agents\rulings\ for elevated, administrator, admin window: no line (the "trace" hits are unrelated). The only user words are in a card. The memory note recommends ETW/WPR while TM says wpr is refused from agent shells. The plan's ban is wider than TM's.
**Ruling:** none in `## Confirmed`.
**Open:** none.

### L-F13-16
**Verdict:** CONFIRMED.
**Homes as found:** GATES\ruling_rules.py:100-101 `APPROVE, DECLINE = "File it", "Do not file"`, `QUESTION = "File this ruling in {path}?"` (:119 builds the text). HY\docs\agents\rulings\README.md:41 "The question reads `File this ruling in docs/agents/rulings/<topic file>?` ... The two labels are `File it` and `Do not file`. One box may hold up to four lines, one question each." CLAUDE.md (repo) says `"File it"`. agent-rules.md:33 mentions filing. GATES\tests\test_ref_judge.py:707,744 use the labels and question text.
**Splits:** None between text and code. "Up to four lines": I searched all GATES *.py and *.md for "four lines", "up to four", MAX and a bare 4 near questions: only the README line holds it. No owner in our code (probably the question tool's own limit); a number with no home.
**Ruling:** none.
**Open:** whether the four-line limit is a tool limit would need a run of the question tool.

### L-F13-17
**Verdict:** CONFIRMED-CHANGED.
**Homes as found:** publish.ps1:1 usage and :8 ValidateSet list six kinds; :60-61 require -Target for every kind except pr and issue, and -Title for pr, release, issue; :18-38 TextLetters `bFnfwetT`, ValueLetters `aBHlmrR`. doc-review.md:19 names the same six. README:65 names the same six. SF deny list (:8-57) covers pr create, release create, release edit, issue create, issue comment, pr comment and `gh api`, in bare, `.exe`, `&` and full-path spellings. test_publish.py pins the letters (:324-328 and :359-363; ledger's :326/:361 are inside those ranges).
**Splits:** (1) issue-tracker.md:30-32 "Create a GitHub issue." omits the route while :5 and :9 give it (weak). (2) New: issue-tracker.md:12-13 tells agents to run `gh issue edit` (labels) and `gh issue close` by hand; neither is in the SF deny list nor a publish kind, so label and close changes are the one GitHub write that has no route (see L-F13-08). (3) Six kinds appear as a ValidateSet, a doc-review sentence, a README sentence and 7 deny families, no test holds them equal.
**Ruling:** none.
**Open:** none.

### L-F13-19
**Verdict:** CONFIRMED (all copies agree today).
**Homes as found:** check_handoff.py:7 usage `py -I tools/check_handoff.py <handoff.md> [--repo <root>] [--since <YYYY-MM-DD>]`; handoff.md:45 and .claude\commands\handoff.md:12 carry the same line. check_handoff.py:32-33 MANUAL_KINDS ("test","ruling","state") and TABLE_STATUSES agree with handoff.md:39-55. Precheck: main.cpp:4-12 usage; derive-once-review.md:24 and :99, integrate.md:37 and :46, agent-rules.md:7 name `hydra_precheck.exe` forms. Hook tests: README:145 and derive-once-review.md:124 both `... python.exe -I -m unittest discover -t . -s gates/tests -p <file>`.
**Splits:** None. Note the shape differs but not the meaning: BP:47 names `hydra_precheck.exe --tests` for the main session, integrate.md:46 and derive-once-review.md:99 add `--repo <path>` and a range.
**Ruling:** none.
**Open:** none.

### L-F13-20
**Verdict:** CONFIRMED-CHANGED (the splits are real; some ledger homes are misplaced).
**Homes as found:** RJ:225-232 refusal advice `git commit -m "Your subject" --trailer "Task: <task>" --trailer "Agent: <your agent id, or orchestrator>" --trailer "Session: %s"` plus a revert/cherry-pick line and the merge/origin exemptions. BP:65-67 gives the agent form with `Co-Authored-By: Claude Opus 5.5`. README:53 (shim paragraph: owners of trailer rule and no-rewrite rule), README:160 and :166 (merge-commit exemption, rollback commit with trailers); README:162 is the shim paragraph for the installed hook, it does not hold the verdict strings. ref_gate.sh:12-17 and RJ:28-29 hold ALLOW/DENY strings; install_ref_gate.ps1:11,13 hold SELFTEST text. MEM git-traps-in-shared-checkout.md:21 "must sit in the message's last paragraph ... Pass Co-Authored-By as a --trailer". integrate.md:27 "separate `-m` arguments and the preamble's trailers, never on stdin".
**Splits:** (1) RJ names "orchestrator" as an Agent value; BP:65 says "your agent id". Differ for the main session. (2) BP:65 says never stdin; RJ's text does not. (3) BP:65-67 carries a fixed `Co-Authored-By: Claude Opus 5.5` line; the session's attribution reminder asks for a different model line; the gate does not check Co-Authored-By at all (RJ TRAILER_KEYS are Task/Agent/Session). Differ for any agent whose model is not Opus 5.5.
**Ruling:** none.
**Open:** none.

### L-F13-21
**Verdict:** CONFIRMED.
**Homes as found:** BP:27 "Never use `sed`, `head`, `tail` or `cat` to read part of a file."; BP:35 reads output files with Read and offset/limit; BP:77 "Never use run_in_background". Searched GATES percall.py, bash_windows_rule.py, shell_output.py, progress.py for head/tail/sed: the hits are about the exit-code "tail" only, no rule on those commands. For background: GATES\shell_output.py:75 skips its rewrite when `run_in_background` is set; agent_stop.py:47-51 recognises "moved to the background" results and treats the agent as parked; progress.py:77-79 un-parks on the next call. No refusal.
**Splits:** BP:77 forbids what the hooks treat as a normal state (parked). BP:27 is held by nothing. "writer-brief:99" was not found: my search of HY\docs and HY\.claude for the wording found only BP and an old audit page.
**Ruling:** none.
**Open:** none.

### L-F13-22
**Verdict:** CONFIRMED-CHANGED (a code home moved).
**Homes as found:** flag literal: GATES\percall.py:14 `LIBRARY_FLAG = "--whole-library"`; HY\src\cli\batch_scope.h:10 `kWholeLibraryFlag = "--whole-library"` (the ledger's batch.cpp:82 is not it; batch.cpp:3 is a usage comment); compare_db.py:29 says "src/cli/batch_scope.h owns that rule". Allow phrase: GATES\review_rules.py:51 `LIBRARY_ALLOW_TEXT = "allow library run"`, :52 `_LIBRARY_ALLOW` regex (also accepts a trailing . or ! and any letter case), :81 is_library_allow (whole message after strip). BP:49, agent-rules.md:14, audit-method.md:125, README:13,37, MEM no-full-library-runs.md:9-16 (says "the user has said yes in chat"). CLAUDE.md rule 8 "explicit permission, asked for in chat first".
**Splits:** (1) Flag spelled in percall.py and batch_scope.h with no test holding them equal. (2) Rule 8 says "explicit permission in chat"; the hook accepts only the exact phrase as the whole message. Differ when the user says "yes, run it". (3) TM:91 says a whole-library `--redo` on a copy of the real library needs the allow; percall keys only on the flag string, so a `--redo` on a copy without the flag passes. (4) percall's refusal text hands the exact phrase out to the agent, so the agent may ask for it.
**Ruling:** rulings\review-and-hooks.md:16, under `## Confirmed` (section starts :7; `## Unconfirmed` starts :37): "i want to be able to give explicit approval for a full library run." It says the user wants to approve explicitly; it does not name the exact-phrase side or the in-chat-only side. Line :47 is an Unconfirmed restatement.
**Open:** none.

### L-F13-23
**Verdict:** CONFIRMED.
**Homes as found:** README:11 percall order: in a subagent "saves SubagentHandback report, progress.py, build-pass refusal, delete_rule, whole-library check, bash_windows_rule, media_rule, then key-route"; code percall.py:132-157 does exactly this; main session README order whole-library, key, bash_windows, media, stall matches percall.py:160-173. README:116 "`percall.py` is the only hook on PreToolUse `*`": SF:140-188 has one `*` matcher (percall.py), plus `Agent` and `Artifact|ExitPlanMode`. README:108-110 and sync_settings.py:1-25 both describe key ownership (the docstring says it is the one home; README points at it). README:152 PowerShell tests and README:145 python tests. README:17 "The hook tests on this page need no build, so agents still run them" versus BP:47 "Never run all the hook tests"; README:148 "any test file may be named". README:166 and RJ:228-230 agree on trailers.
**Splits:** README:17 versus BP:47 are one file versus all, never stated together. README:152 uses `powershell`, TM says to use `pwsh` (see L-F14-02).
**Ruling:** none.
**Open:** none.

### L-F13-24
**Verdict:** CONFIRMED.
**Homes as found:** README:5 "This page does not repeat them" (the preamble's rules) while README:19, :23, :25, :27 restate the delete rule, the 4,000-character trim and the Bash refusal parts in full prose. test_docs_numbers.py:44 `CALL_COUNT = re.compile(r"\b(\d+)(?:st|nd|rd|th)? tool calls?\b")`. test_bash_windows_rule.py:53 `assertEqual(rule.MAX_CHARS, 1000)` (owner bash_windows_rule.py:16 `MAX_CHARS = 1000`); test_shell_output.py:172 `assertEqual(shell_output.ERROR_CHARS, 1000)` (owner shell_output.py:18 derives ERROR_CHARS as CAP - HEAD_CHARS - TAIL_CHARS, so the test pins a derived value); test_publish.py:247 pins a usage string; :359-363 letters.
**Splits:** None between homes; the README:5 claim is wrong as written.
**Ruling:** none.
**Open:** none.

### L-F14-10
**Verdict:** CONFIRMED-CHANGED.
**Homes as found:** MEM render-checks-use-the-corpus.md:19 "Scroll to the middle of the list, click a song, compare before and after, close, and compare again." MEM search-before-clicking-a-song.md: "Never scroll through the user's library to reach a song or a spot without asking in chat first and saying how it will be done ... every method". UT:99 three guards, owners in HY\tests\ui\uitest_commands.cpp: `kPageKeys` :76-77 {PageUp, PageDown, Home, End}; `library_key_refusal` ~:207-226 (refuses when the focused window is the library and `library_unfiltered()`); `item_ref` ~:230. uitest_live.cpp:481-505 pins the PageDown refusal. hydra_ui_mcp.py:757 "To open a song, use hydra_open_song with its exact title".
**Splits:** (1) The two memory notes differ on scrolling. But the ledger's explanation (corpus versus real window) does not hold in code: the guards look only at "library focused and search box empty" (uitest_commands.cpp:216-220), not at corpus versus real library, so the note's "scroll to the middle of the list" is refused by `key PageDown` on a headless unfiltered library too. (2) UT:99 says "PageDown or End"; the array also holds PageUp and Home; UT says "such as", so it is an example list, no split. (3) The user's words say "without my approval"; the note says "asking in chat"; a subagent has no chat channel (by reading BP; nothing says an agent can ask). (4) Guards are test-engine only; posted window messages from outside the command channel are covered by no hook (UT:99 says so itself).
**Ruling:** rulings\ui-and-display.md:27 (2026-10-30) "ive had enough of this shit. something like 10 times now you waste my time by scrolling thorugh the entire library for minutes to reach the song you want with no explanation. this should never happenw tihout my approval." and :48 (2026-10-24) "why are you scrolling through the entire library??? why do you keep forgetting search exists?". Both found in the file the ledger named; neither names corpus versus real library, so neither names both sides.
**Open:** whether a wheel scroll through the hydra MCP (not `key`) is refused would need a live run.
**Hook gap:** yes: no hook covers scrolling the real window; the guard lives only in the UI test engine.

### L-F14-11
**Verdict:** CONFIRMED-CHANGED.
**Homes as found:** CH:23 "The game window is never brought to the front. `ch_start` asks Windows to open the game minimized and not active ... The one launch that cannot ask is the spawn form of the human launch (`launch` set to `spawn` in launch_patch.json), because Frida's spawn takes no show-window option ... The file ships set to `suspended`"; then a watch counts front-window events; the count is "information". launch_patch.json:5 `"launch": "suspended"`. bot_run.py:80,157-160 SW_SHOWMINNOACTIVE = 7. MEM ch-runs-must-not-need-focus.md "TOP RULE" and its own text "Brief focus is not a failure: on 2026-10-31 the user asked ..." plus "so never show the window" (a trap about SW_SHOWNOACTIVATE).
**Splits:** (1) Memory "never show the window" versus the same note's "Brief focus is not a failure", the note holds both. (2) CH:23 "never brought to the front" versus a spawn launch that cannot ask (the page itself names it). (3) The memory note is stronger than the page's "count is information".
**Ruling:** rulings\clone-hero.md `## Confirmed` (starts :7; Unconfirmed starts :67): :34 "why does it matter if the game is focused briefly? it sounds to me like you're trying to over engineer a solution to a problem that isnt real"; :38 "yes we need to do autonomous runs. that waws the whole point of this"; :39 "why does the game need to be focused? hydra doesnt need to be focused when you test it"; :43 chose "No one needed (Rec.)"; :8-9 settings.ini line. Line :80 (Unconfirmed) says a brief grab at launch, before the kit exists, is acceptable; that is a note's reading, not a Confirmed line. :34 and :38/:39 together lean both ways (brief focus fine; autonomy needed) but no line names "never bring to front" against "brief grab at spawn".
**Open:** whether a brief grab at spawn is allowed is stated in no Confirmed line.
**Hook gap:** none.

### L-F14-12
**Verdict:** CONFIRMED.
**Homes as found:** HY\docs\agents\public-doc-style.md:27 puffery list (seamless, robust, ... streamline), :29 "serves as" rule, :31 throat-clearing: "It's worth noting", "Note that", "In this section", "Let's", "Simply", "just", "In order to", "To summarize", "In conclusion". C:\Users\Patrick\.claude\CLAUDE.md:5 "Cut throat-clearing ("It's worth noting", "In order to", "It's important to note")".
**Splits:** two phrases appear in both lists, "It's important to note" only in the user's, the rest only in the public-doc list. Different readers. I searched GATES for worth noting / in order to / important to note / delve / tapestry: no file. I did not find a Hydra test or tool that greps these (one search over source extensions hit only third-party headers).
**Ruling:** none.
**Open:** none.

### L-F14-13
**Verdict:** CONFIRMED.
**Homes as found:** doc-screenshots.md:5-7 and tools\doc_shots.ps1:2 and development.md:980: `pwsh -NoProfile -File tools\doc_shots.ps1` (same spelling in all three). Card 2026-10-26-93098c69.md:30 says "run tools\doc_shots.ps1" without the flags (a shortened form, same script). Label: library_dialogs.cpp:133 `return out.empty() ? "defaults" : out;`, :136-137 `settings_button_label` returns "Analysis settings: " + summary. test_batch_text.cpp:81-82 holds "Analysis settings: defaults" and "... (locked)". shots.txt:17 `click Reset to defaults`; owner settings_bar.cpp:226 `ImGui::Button("Reset to defaults")`.
**Splits:** none.
**Ruling:** none.
**Open:** shots.txt:20,83 and UserGuide.md:121-139 not opened.

### L-F14-14
**Verdict:** CONFIRMED-CHANGED (two gates, but they answer different questions).
**Homes as found:** tools\ch_mcp\build_facts.py:31-65: VERSION_FILE under "Clone Hero_Data\version.json", VersionFileError, UnknownBuild "unknown build ...: no facts for it; run dump_build.py and dump_facts.py" (asks: do facts exist for the installed game build). build_stamp.py:210 `check(js_path)` returns a stale reason when a built agent JS no longer matches its sources (asks: is the compiled agent stale); called from frida_tools.py:402 and method_watch.py:437. CH:78 describes only the version gate ("That check reads only the version string in the game's own version file") and does not mention the stale-agent check.
**Splits:** CH:78 names one refusal; the code has a second one that CH never lists. Differ for a stale agent build on an unchanged game version. They are not two owners of one question.
**Ruling:** none.
**Open:** none.

### L-F14-15
**Verdict:** CONFIRMED (partial: spot-checked, not all ~150).
**Homes as found:** UT:117-133 table. I checked these label strings against HY\src: Manage folders, Analyze library..., Compare with dmleaderboards (ui\library_toolbar.cpp), analyze_shown_label (library_toolbar.cpp:44; used :87), Open in Path Builder (ui\details_parts.h), Hide library, Expand all, Score breakdown, Show in Preview, Also re-analyze charts that already have a result, Start analyzing, Retry (ui\library_dialogs.cpp), Open automatically (app_state), Compare another player..., All timing tiers (app\path_report_view.cpp), Opening your library (ui\library_jobs.cpp), Checking for updates (ui\update_job.h), version_bar_status (app_state.cpp:1166), kViewProgressDelaySeconds (app_state.h). All exist. "Scan library" exists only in app\analysis.h and user_messages.cpp (not a ui file); the table says it is a toolbar button, so I did not confirm that owner in ui.
**Splits:** none found. No test holds the table equal to code.
**Ruling:** none.
**Open:** most labels unchecked.

### L-F14-16
**Verdict:** CONFIRMED (spot checks only).
**Homes as found:** UT:31-40 options (e.g. `--jobs` default "kDefaultJobs in tests/ui/uitest_main.cpp"; uitest_main.cpp:255 `kDefaultJobs = 4`). uitest_commands.cpp:106-120 split_bar splits at " | " (and " |" at line end); hydra_ui_mcp.py builds those lines (K6 weak: the bar is written in Python and read in C++). CH FRAME_PROBE_SECONDS: kit_tools.py:737 `= 5.0`; CH names the constant, not the number; test_kit_tools.py:1056 pins 5.0 and :1155-1157 pin the text "seconds is at most 5". .mcp.json: hydra starts with `py -I tools/hydra_ui_mcp.py`, ch with `py tools/ch_mcp/front.py -- py tools/ch_mcp/server.py` (no -I on ch; confirmed).
**Splits:** (1) .mcp.json runs hydra isolated and ch not. (2) The ch frame-probe error text copies the number 5 as a literal in a test message string (K1). No split of meaning.
**Ruling:** none.
**Open:** the other ~20 source ids not opened.

### L-F14-17
**Verdict:** CONFIRMED-CHANGED.
**Homes as found:** UT:44 "The full suite took about 90 s at the default job count, measured on 2026-10-10 at commit 4c037634 over 1 run, on a machine whose CPU other builds held at 100..." (dated number in a doc; confirmed). UT:48 canned dmleaderboards user `alice`, id `111`: tests\ui\uitest_batch_reports.cpp:194,246 use "alice"; I did not find the id 111 home. The stale issue-tracker copy: HY\.claude\worktrees\plan-agent-supervisor\docs\agents\issue-tracker.md exists, a registered worktree; a diff of its first 400 lines against HY\docs\agents\issue-tracker.md printed nothing, so it is NOT stale today; it is an identical checkout copy. TM:7 and README:114 both state the python path (L-F13-06).
**Splits:** None. The ledger's "stale copy" claim is not supported.
**Ruling:** none.
**Open:** id 111 home.

### L-F15-02
**Verdict:** CONFIRMED.
**Homes as found:** MEM git-traps-in-shared-checkout.md:19-20 "If a merge is refused, run `git merge --abort` at once and check `.git/MERGE_HEAD` is gone"; "Before any commit there, check `.git/MERGE_HEAD` and `git diff --cached --name-only`. If anything else is staged, do not commit there". Code: RJ now has `current("MERGE_HEAD")` at :448 (the ledger's :336 moved), used by restore_checkout to `git merge --quit` after a refused merge; it answers "put the checkout back after a refusal", a different question. I searched GATES *.py for `diff --cached`, `cached --name-only` and "staged": only tests hit; HY\docs\agents for MERGE_HEAD and diff --cached: no match. So the rule has one home (a memory note) and no hook check.
**Splits:** None.
**Ruling:** none.
**Open:** none.

## Ids not reached
None. All 20 rows were read against code.
