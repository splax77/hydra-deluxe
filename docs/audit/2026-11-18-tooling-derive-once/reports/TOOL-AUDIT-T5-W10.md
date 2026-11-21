# TOOL-AUDIT-T5-W10 (second-round verifier; ledger TOOL-AUDIT-T5-L06; 20 rows)

All claims are "by reading". Nothing was run. gates = C:\Users\Patrick\.claude\gates. Hydra = C:\Users\Patrick\Downloads\Hydra\hydra-test. Several gates files have shifted a few lines since the ledger was written (ref_judge.py constants now :35-37, judge_push_lines :184, missing_trailers :242); I quote today's lines. Ruling checks: the ledger's `## Confirmed` quotes in review-and-hooks.md (Confirmed block :7-35, `## Unconfirmed` at :37) and tests-and-runs.md (`## Confirmed` :7, `## Unconfirmed` :31) were opened where a row needed them.

### L-F16-10
**Verdict:** CONFIRMED (values agree; no doc states the harness defaults).
**Homes as found:** gates/wake.py:22 `WAKE = 2`, :24 `UNHEARD = 1`, :26 `CRASH_SUFFIX = ".wake-crash.json"`, :69-71 `_crash` "WAKE the first time a crash text is seen for this session, UNHEARD after"; gates/README.md:31 is the wake paragraph. gates/tests/lib/harness.ps1:51 `[int]$TimeoutSec = 120`, :86 `WaitForExit($TimeoutSec * 1000)`, :95 `$script:HarnessDrainMs`, :106 "HOOK TIMED OUT after N s". Callers: tests/test_h52_cleanup_sweep.ps1:283 `-TimeoutSec 60`, :409 and :463 `-TimeoutSec 120`, :411 `WaitForExit(120000)`; tests/test_h53_session_cards_start.ps1:50 `time.sleep(120)` (a fake script that hangs on purpose, lines 46 and 206). settings.fragment.json hook timeouts 10, 15, 20, 30 and 660 (lines 81-235) answer what Claude Code enforces, a different question.
**Splits:** none found. (The 120 in the harness default and the 120 in the h52/h53 callers are separate numbers for separate jobs; nothing ties them.)
**Ruling:** none found.
**Open:** the exact README sentence "the same crash again exits 1" was read only as a trimmed line; wake.py's behaviour matches it. A crash drill is a run.

### L-F16-11
**Verdict:** CONFIRMED (the replay strings match the live texts today).
**Homes as found:** gates/tests/replay.py:44-62 OLD_RULES are regexes over the OLD hooks' refusal texts (comment :44-45 "Class obj1/obj2 are the objectives the new set must still enforce (list 1); obj3 is the budget, kept by progress.py"); :62 `Budget of \d+ tool calls reached` is the old budget wording. `tightening()` (:283-344) matches NEW live texts by substring, and each exists: "names the status helper but is not a status call" = progress.py:125; "whole song library" = percall.py:52; "is refused in an agent: it discards work" = delete_rule.py:157; "recursive or forced" and "is not a literal absolute path" = delete_rule.py:145,:148; "names no review key" = review_dispatch.py:158; "does not know the commit" / "already on main" = review_dispatch.py:107,:113; "no fresh agent has signed off" = publish_gate.py:103-104; "Your budget is spent" = progress.py:130.
**Splits:**
- SPLIT (wording only): replay's old-budget regex `Budget of N tool calls reached` (:62) against live progress.py:130 "Your budget is spent (...)". They differ for any recording made under the old hook; replay.py:315 handles the live text separately as "budget fired in replay". Not a live disagreement.
**Ruling:** none found.
**Open:** whether the plan that owns the objectives (2026-10-25-hooks-from-scratch.md) was not read.

### L-F16-15
**Verdict:** CONFIRMED-CHANGED (constants are real; the list-difference is a different question, not one split; one finder cite was off).
**Homes as found:** Hydra tools/check_pointers.py:32 `HOOK_EXTENSIONS = (".py", ".ps1", ".sh")`, used only at :82-83 `is_hook_name` ("a bare name of a hook script", no path separator). tools/doc_tokens.py:30-35 `KNOWN_EXTENSIONS` (a longer list: .md .py .ps1 .psm1 .sh .bat .cmd .cpp ... .png) is the path-shape test; docs/agents/handoff.md:59 says "A token with a path separator or a known extension is a path". Exit codes: check_pointers.py header "Exit 0 when nothing is broken, 1 when something is, 2 on a bad argument. With --session-start ... it always exits 0"; gates/README.md:100 "Either way the script exits 0" is the hooks-wide paragraph (the finder's :71 line holds a different paragraph; the "no onFailure" statement is at README:71 only inside a long line I could not isolate). settings.fragment.json:95-104 check_pointers `--session-start`, `"timeout": 15`, no `onFailure`. tools/doc_tokens.py:37 `BRANCH_PREFIXES = ("claude/", "hx-")`; tools/new_worktree.ps1:4,7,30 cuts only `claude/<TaskId>`. A Grep of Hydra tools for `hx-` finds only doc_tokens.py:37.
**Splits:**
- SPLIT (K1, shape): `.py/.ps1/.sh` is written once in HOOK_EXTENSIONS and again inside KNOWN_EXTENSIONS. They answer different questions (is a bare name a hook script / is a token a path), and differ for a bare `foo.md` (path by KNOWN, not a hook name).
- SPLIT (K8): `hx-` is a branch prefix doc_tokens treats as a branch; no tool in tools/ creates one (searched tools/ only).
**Ruling:** none found.
**Open:** check_handoff exit codes (0/1/2) vs handoff.md:57: tools/check_handoff.py has `sys.exit(main(...))` at :449 and `return 1` at :392; I did not read main() to confirm the 0/1/2 words.

### L-F16-16
**Verdict:** CONFIRMED (the -DryRun and check-mode text parts now read).
**Homes as found:** Hydra tools/build_slot.ps1:50 usage line `-DryRun -SlotDir <scratch folder>`, :60 `[switch]$DryRun`, :329-330 `if ($DryRun) { if (-not $SlotDir) { throw '-DryRun needs -SlotDir, so it never takes a real slot' }`; :326 "no claimed build pass for this build. $BuildPassHowTo"; Directory.Build.targets:13 `Text="This build has no claimed build pass; see the line above, from tools\build_slot.ps1."`.
**Splits:** SPLIT (wording): build_slot.ps1:326 and Directory.Build.targets:13 say the same fact in two sentences; they appear together (the second points at "the line above"), so no case where they disagree.
**Ruling:** agents-and-workflow.md:8-9 as the ledger quotes (tool name not spoken to).
**Open:** the `mcp__hydra__` prefix is a run question.

### L-F16-17
**Verdict:** CONFIRMED.
**Homes as found:** Hydra tools/hydra_ui_mcp.py:692-694 comment ("live replies carry no line number") and :708 `out.append(f"stopped at line {i} of {len(lines)}")`; docs/agents/ui-testing.md:229 "stops at the first failure with `stopped at line N of M`, counting the script's own lines"; tools/test_hydra_ui_mcp.py:773,:949,:956 (`"stopped at line 4 of 5"`, `"1 of 1"`, and a NotIn). hydra_ui_mcp.py:632 `"refused: one command is one line; this one holds a line break"`; test_hydra_ui_mcp.py:228 pins it. ui-testing.md does not state the one-command-is-one-line sentence (grep of that page: no hit). tools/doc_shots.ps1:8 `$exe = Join-Path $root 'build-cpp\Release\hydra_uitest.exe'`; :13-14 error text "ask the main session to build the hydra_uitest target" (restates rule 14); :31-35 reads `rect=(` lines from the output; tools/doc_shots/shots.txt top comment states the rect rule.
**Splits:** none found. Same words in doc and code for "stopped at line".
**Ruling:** none found.
**Open:** docs/agents/doc-screenshots.md:15-25 was not opened.

### L-F16-18
**Verdict:** CONFIRMED.
**Homes as found:** Hydra tools/hydra_ui_mcp.py:42 `REPLY_MARGIN_S = 15.0`, :43 `HELLO_TIMEOUT_S = 60.0`, :44 `REPLY_CAP_CHARS = 12_000`; tools/test_hydra_ui_mcp.py:288-296 states 12,000 and 1000 as literals; tools/fake_hydra_live.py:12 docstring "fake-big answer with 50,000 characters of text" against :64-65 `BIG_LINE = "x" * 45 + " row"` (49 characters with its newline, times 1000 lines, about 49,000); :61 `SESSION_TIMEOUT_S = 25` echoed as "timeout" at :105,:146. ui-testing.md grep for "12,000" finds no hit (the cap is not stated there).
**Splits:**
- SPLIT (rounding): the stand-in's docstring says 50,000; the real text is about 49,000. They differ only in the docstring; no test reads it.
**Ruling:** none found.
**Open:** none.

### L-F16-19
**Verdict:** CONFIRMED for the game folder (agree today) and for HUMAN_NEEDS_SONG; the other copies are read as listed.
**Homes as found:** tools/ch_mcp/game_files.py:31 `GAME_DIR = r"C:\Clone Hero\1.1 june\Clone Hero"`; tools/ch_mcp/README.md:12 and :58 repeat the path inside commands; README.md:27 gives the 1.0 path `C:\Clone Hero\1.0 old\Clone Hero`; README.md:27 says the `--version` for 1.0 is "passed by hand" because the script cannot read it. tools/ch_mcp/build_facts.py:28 `DUMPS_ROOT = r"C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps"`, :69 area reads the version from `Clone Hero_Data\version.json` (`_VERSION_REL`); dump_facts.py:27 documents `--version 1.1.0.6142-final` typed by hand. bot_run.py:63 `DEFAULT_PLAYER = "Drums,Expert"` and :12 usage text `[--player Drums,Expert]`. bot_run.py:109-112 `HUMAN_NEEDS_SONG` (reason stated once), used :120,:745. frida_tools.py:56-61 `NO_GAME_MANAGER`, `TRACE_RUNNING`, `NOT_ATTACHED = "not attached; call ch_attach first"`; frida_attach.py:439 `self.gone_reason or "not attached"` (own spelling). input_driver.py:90-106 `MENU_KEYS` (wiki URL read 2026-10-30, a comment); menu_steps.json line 2 contains "verified is still false".
**Splits:**
- SPLIT (K4, mild): the version of 1.1 is read from the game folder (build_facts.py) but typed by hand for dump_facts.py and for 1.0; they differ if the game updates and the typed value is not changed.
- SPLIT (K1 wording): `not attached` is spelled in frida_tools.py:61 and again in frida_attach.py:439 and (finder) kit_tools.py:1268; the first carries "call ch_attach first", the others do not.
**Ruling:** none found. Not opened: docs/agents/clone-hero-mcp.md:31,:45,:54,:70 and the plan answer at :70.
**Open:** none beyond the unopened docs.

### L-F16-22
**Verdict:** CONFIRMED-CHANGED (four rows opened; copies agree; one comparison was not a split).
**Homes as found:** tools/capture/steps.ps1:3 `param(... $Steps = @("start","min","restore","max"))`, :5 `$codes = @{ min = 6; restore = 9; max = 3 }`, :7 the unknown-step guard lists the same names in its message (second copy of the step names in one file). Hydra tools/doc_shots/shots.txt:12-20 puts panel settings back (`click ###analysissettings`, `click Reset to defaults`, `key Escape`, `expect-text Analysis settings: defaults`); tests/ui/uitest_library.cpp:254,:273,:322 check `on_screen(h, "Analysis settings: defaults")`; owner src/ui/library_dialogs.cpp:116-133 `settings_changes_summary` ("defaults" at :133) and :137 `"Analysis settings: " + settings_changes_summary(s) + (locked ? " (locked)" : "")`. tools/precheck/main.cpp:10,:41 `--range defaults to main...HEAD`; docs/agents/derive-once-review.md:17,:24 give `--range main...<key>` (explicit, so the two agree where used). tools/mutate/run.py:79 `MAX_COMMAND_LINE = 32767`, used :634 on `subprocess.list2cmdline(cmd)`; gates/bash_windows_rule.py:16 `MAX_CHARS = 1000`, used :174.
**Splits:**
- SPLIT (different limits): 32,767 (Windows CreateProcess limit, mutate tool) against 1,000 (the shell-command length a hook accepts). They answer different questions: how long a command line may be to start a process / how long a typed Bash command may be. Not a conflict; a pointer would only matter if one claims to be "the longest".
**Ruling:** none found; review-and-hooks.md:43 "length over 1000 and backslashes" is Unconfirmed (under :37).
**Open:** the doc-screenshots.md copy and the "five prose places" for "agents may run hydra_precheck.exe" were not counted.

### L-F17-08
**Verdict:** CONFIRMED-CHANGED (a real split the finder missed: two readers of "same document").
**Homes as found:** gates/review_rules.py:65-74 `normalized` and `doc_key` (sha256 over bytes with a BOM dropped and CRLF to LF; docstring says publish_gate's small-text count reads the same bytes); :19 `_DOC_MARKER_PATH`; :277-280 `_doc_review(path, key, clean, root)`: passes when `key in clean`, or when the path is not the doc-review marker and any CLEAN record has `r.get("path") == path` (exact string, case-sensitive, repo-relative); :214-217 `_clean_keys`; :264-275 `_code_review` and `_carries` (carry rule for code). gates/publish_gate.py:68-71 `document_path_id` (absolute, forward slashes, lower case; docstring "_signed_off is the one place in this file that compares a document's path with a recorded one"), :74-88 `_signed_off`: key in clean passes; else any CLEAN record's path joined to the repo top and compared by `document_path_id`; docstring :75-77 quotes the user's rule "once a fresh agent signed off a version of a document, later edits pass". ref_judge.py:419 comment "the same carry rule as review_rules._code_review". No other carry text read.
**Splits:**
- SPLIT: publish_gate._signed_off compares paths case-insensitively and by absolute path (after joining the repo top); review_rules._doc_review compares the recorded path as an exact repo-relative string. They differ when the recorded path differs only in case or slash form, or when the editing cwd is a worktree whose top differs from the recorded one.
- SPLIT: review_rules._doc_review excludes the doc-review marker file from the "later edits pass" path rule (`_DOC_MARKER_PATH`); publish_gate._signed_off has no such exclusion (it never sees the marker as a document).
- SPLIT (comment): publish_gate.py:75 says "once a fresh agent signed off a version of a document, later edits pass", and CLAUDE.md "Doc review" says "each document gets it once". The carry for code (`_carries`) is a different rule than the path rule for documents; the three are described in prose together.
**Ruling:** review-and-hooks.md:27 "every time claude publishes a document, it has to go through a review from a new agent with fresh context specifically briefed on these issues." Does not name the later-edits rule; the code docstring cites the user's rule without an id, which is a claim.
**Open:** a run with a case-different path or a worktree path.

### L-F17-12
**Verdict:** CONFIRMED-CHANGED (the shim strings are read and agree; a gate-marker split was found that the ledger did not list).
**Homes as found:** gates/ref_judge.py:35 `ALLOW, DENY = "ref-gate-verdict: allow", "ref-gate-verdict: deny"`, :36 `SELFTEST_OK = "ref-judge selftest: allow ok, deny ok"`, :37 `MAIN = "refs/heads/main"` (used :154,:171,:179,:188,:194,:205,:357-359). gates/ref_gate.sh:15-16 compares to the two verdict strings in full; install_ref_gate.ps1:13 `$selftestOk = 'ref-judge selftest: allow ok, deny ok'`, :32 compares the last selftest line to it. Other spellings of main: review_dispatch.py:83-88,:137 and publish_gate.py:47 write `refs/heads/main` literally; review_rules.py has more literals. publish_gate.py:12 `MARKER = ".doc-review-gate"`, :42-60 `gated_repo`; review_dispatch.py:140-145 `_gated` accepts `CODE_MARKER` or `DOC_MARKER` on main (and no local main = not gated, :83,:136-137); review_rules.py:18 `CODE_MARKER, DOC_MARKER = ".derive-once-gate", ".doc-review-gate"`.
**Splits:**
- SPLIT: publish_gate.gated_repo is true only when `.doc-review-gate` is in main's tree; review_dispatch._gated is true when either marker is. A repo with only `.derive-once-gate` is gated for reviewer dispatch and not gated for publishing.
- SPLIT (K3): publish_gate.gated_repo raises when git cannot read a repo that has a `.git` folder (:56) so the caller refuses; review_dispatch returns not-gated for no local main (:83-85,:136). Different failures for different damage.
- The shim's strings agree three ways; no split.
**Ruling:** review-and-hooks.md:57 is Unconfirmed (git-side hook "as the only judge of 'may main move'"). The ledger's "Git-side hook (Recommended)" at :30 was not re-read here.
**Open:** the shim run with a stub is a test run.

### L-F17-15
**Verdict:** CONFIRMED (copies agree today).
**Homes as found:** gates/ruling_rules.py:95-96 `CONFIRMED, UNCONFIRMED = "## Confirmed", "## Unconfirmed"`, :99 `INSTALL_DATE = "2026-11-14"` (comment cites the user's answer "Yes, no click", 2026-11-13, session ca29fda5), :100 `APPROVE, DECLINE = "File it", "Do not file"`, :101 `QUESTION = "File this ruling in {path}?"`. Hydra docs/agents/rulings/README.md:3 (`"File it"`), :41 ("Do not file" and "File this ruling in docs/agents/rulings/<topic file>?"), :51 (INSTALL_DATE and 2026-11-14); CLAUDE.md "Rulings and this machine" `"File it"`; docs/agents/agent-rules.md:33 `"File it"`; gates/README.md:59 `"File it"`. Rulings README.md:20 says Unconfirmed holds lines "copied from memory notes on 2026-10-31" (see L-F17-16 in the ledger for the 11-06 lines).
**Splits:** none in the words. The date is stated twice (code, README:51) with no test (test_docs_numbers.py:129-136 checks only the heading and an old phrase).
**Ruling:** no `## Confirmed` line for "File it" or the date. The user's words quoted in agent-rules.md:33 ("nothing should ever be recorded as a ruling without my explicit approval. so how do we prevent this?") do not name the label text.
**Open:** none.

### L-F18-05
**Verdict:** CONFIRMED (the markdown/DOC_SUFFIXES split); the rule 5 citation is read and is a loose fit, not a wrong cite.
**Homes as found:** gates/agent_stop.py:21-23 comment "The one whole line that holds the merge, and the one that says outright it does not. With neither, a finished reviewer has still signed off what it fixed (Hydra CLAUDE.md rule 5)." and `_BLOCKED, _CLEAN`; :118-120 `return "CLEAN", "its hand-back has no 'SIGN-OFF: CLEAN' or 'SIGN-OFF: BLOCKED' line, so it was recorded CLEAN"`; :163-165 comment and `DOC_SUFFIXES = (".md", ".html", ".htm")` ("Artifact pages are HTML, and doc-review.md says they are reviewed as committed files the same way as markdown"). Rule 5 (agent-rules.md:11; CLAUDE.md rule 5) says the reviewer "fixes every finding itself, commits its fixes and signs off", and only a reviewer out of time "signs off what it fixed and lists the rest in the Notes". The "no sign-off line is CLEAN" rule is not in rule 5; it is in reviewer.md:33 and doc-review.md:125 and derive-once-review.md:151 (per ledger).
**Splits:**
- SPLIT: reviewer.md:33, derive-once-review.md:151 and doc-review.md:29 say "markdown"; the hook records `.md`, `.html` and `.htm`. Differ when an HTML page is in the range.
- SPLIT (K8, mild): agent_stop.py:22 cites rule 5 for "no line = CLEAN"; rule 5 says it only for a reviewer that ran out of time and wrote nothing about a missing line.
**Ruling:** review-and-hooks.md:19 "Build option A as planned, with Q1 as recommended: a hand-back with no sign-off line records CLEAN and tells the main session." (as the ledger quotes). It names the CLEAN-with-note rule, not the DOC_SUFFIXES list or the rule 5 cite.
**Open:** gates/README.md:49 not opened here.

### L-F18-06
**Verdict:** CONFIRMED (no split changes a dispatch answer).
**Homes as found:** gates/review_dispatch.py:17-19 `_KEY` (a 40-hex hash standing alone; "a longer hex run, such as a 64-hex doc key, is not one"), :21 `_RANGE_LINE` with `[0-9a-f]{40}`; gates/review_rules.py:56 `_HEX = re.compile(r"^[0-9a-f]{40}([0-9a-f]{24})?$")` (used :423 for a main move's old/new), :25-28 `WAIVER_PHRASE` (7-40 derive-once and real-app; 7-64 doc-review). Docs: derive-once-review.md:17 "a **key** (a 40-character hash)"; doc-review.md:29 "The **key** is the 40-character hash of the branch's tip"; handoff.md:19,:35 "the full 40-character hash", :51 `missing` when not full 40; .claude/commands/handoff.md:15 "the commit's full hash"; model-postures.md:5 does not state a length. gates/review_rules.py:422-423 is the only reader of 64.
**Splits:**
- SPLIT (K1): three length rules, each for its own job: dispatch takes 40; a main move takes 40 or 64; a waiver takes 7-40 or 7-64. Docs say "40-character". Differ only for a waiver typed with a prefix (see L-F17-09) and for an old 64-hex doc key in a brief (ignored, review_dispatch.py:17-19).
**Ruling:** none found.
**Open:** none.

### L-F18-10
**Verdict:** CONFIRMED-CHANGED (the step 4 / step 9 difference is real but smaller than the finder wrote; the brief-quote wording split is real and larger).
**Homes as found:** Hydra docs/agents/derive-once-review.md:45 (step 4): a decision from "an ADR in `docs/adr/`, `CONTEXT.md`, a line under `## Confirmed` in `docs/agents/rulings/`, or the 'User decisions' header of a plan"; "A decision entry there (in the decision log, a plan's 'User decisions', `docs/adr/` or `CONTEXT.md`) counts only when it quotes the user's words as they are found under `## Confirmed` or in a session card." :52 (step 9): "quote the user's words that asked for it and say where they are: a D-record, a plan's 'User decisions', a line under `## Confirmed` ..., a session card, or the words your brief quotes." (no quote condition on the D-record/plan entry, but the instruction opens with "quote the user's words"). docs/agents/premise-map.md:45 `ruling`: "a line under `## Confirmed` ..., in a session card's quoted words under `docs/sessions/`, or in the user's message as the main session quotes it in the reviewer's brief and confirms." docs/agents/doc-review.md:63 (Plans): "a line under `## Confirmed` ..., a session card or the brief's quote". agent-rules.md:33-41 and rulings/README.md:34: Confirmed line, and "a decision entry there counts only when it quotes the user's words as found under `## Confirmed` or in a session card" (no brief quote listed). docs/sessions writer-brief (hooks repo copy, tools/session_cards/writer-brief.md:67) "The `## The user's words` section holds the user's exact words ... not filed rulings".
**Splits:**
- SPLIT: step 4 (:45) and agent-rules.md:41 say a decision entry counts only when it quotes Confirmed or a card; step 9 (:52) also accepts "the words your brief quotes". Differ when the only support is a brief's quote.
- SPLIT: the brief-quote rule has three wordings: premise-map.md:45 (needs the main session to quote it and "confirm"), doc-review.md:63 ("the brief's quote"), derive-once-review.md:52 ("the words your brief quotes"). Differ on whether the main session must confirm.
- SPLIT: step 4 lists ADR, CONTEXT.md, Confirmed line and plan "User decisions"; step 9 lists D-record and plan "User decisions"; neither lists the other's ADR / D-record. Differ for a visible change backed only by an ADR, or a new number backed only by a D-record.
**Ruling:** none found says which places count. A Confirmed line does not name both sides.
**Open:** none.

### L-F18-11
**Verdict:** CONFIRMED (Hydra side has a code owner; hooks side has prose only).
**Homes as found:** Hydra tools/precheck/test_pick.h:90-95 `shared_code` ("The one place shared code is decided"); tools/precheck/test_pick.cpp:786-791 pushes three lines `always: "-tc=docs match code*"`, `always: "-sf=*single_owner*"`, `always: "-sf=*test_precheck*"` with the comment "The three repo-wide checks of CLAUDE.md rule 1, run at every merge"; docs/agents/agent-rules.md:7 repeats the three commands and says "That tool decides what counts as shared code" for Hydra; for hooks it lists "`gates\common.py`, `gates\lib`, `gates\settings.fragment.json`, `gates\tests\lib`" (the user's ruling, 2026-10-14); C:\Users\Patrick\.claude\model-postures.md:11 "`common.py`, `lib\`, `settings.fragment.json` or `tests\lib\`"; gates/README.md:156 same four; docs/development.md:538-546 "decides first whether the whole suite runs: a build file, a test-harness file or a header with too many direct includers sends it there (`shared_code`)".
**Splits:**
- SPLIT (K7): the hooks shared-file list is written in three prose places (agent-rules.md:7, model-postures.md:11, gates/README.md:156) and no code owns it; a grep of gates/*.py for "whole suite|every test" finds none. They agree today.
**Ruling:** Confirmed, tests-and-runs.md:15 (2026-10-24) "Fix the doc line, and also add the repo-wide tests (docs-vs-code, single-owner scan, precheck self-test) to every merge run." (names the three always-run checks; does not speak to the hooks list). Confirmed :17 (2026-10-14) "why do we need to run the full suite every time a small change is made? this is a big waste of time. all the parts you didnt change will remain the same" (names the one side: run only what changed). The "whole suite only when shared code changes" half is the memory-note line at :34, under `## Unconfirmed`.
**Open:** none.

### L-F18-12
**Verdict:** CONFIRMED (both parts).
**Homes as found:** Hydra docs/agents/owner-map.md:3 "This page is the one home of the map's shape and of the two stops that go with it. Other pages poi[nt here]"; docs/agents/premise-map.md:5 "This page is the one home of the map's shape ... Other pages point here and do not copy it". Restated in: C:\Users\Patrick\.claude\agents\reviewer.md:17 ("When the plan has an owner map, open each owner and check that its own words claim the row's question. Then read the plan's steps with the map closed."), :13 (premise map check), planner.md:19 ("build only from its rows: use each ..."), planner.md:15 and :13, docs/agents/doc-review.md:63, owner-map.md:39-47 ("Who checks it": "opens each owner and reads its own words against the row's question ... Then reads the plan's steps with the map closed"). derive-once-review.md:21 "It finds kinds B, C, E and F mechanically, and kind D's new numbers." against :38 "The precheck's lines cover kinds B, C and E, kind D's new numbers, and kind F's candidates." tools/precheck/checks.cpp:8-12 labels (C, B, D, E, "Second homes in src/ (kind F)"); check_5.cpp:1-3 "Check 5: second homes in src/ (kind F) ... A type, a free function or a long string literal the range writes in one src/ file when another src/ file already has it"; items.h:50 `kLargeRangeLines = 400`; clones.cpp is under tests/single_owner/clones.cpp:90 `kCloneWindowLines = 8` (not in tools/precheck).
**Splits:**
- SPLIT (K2): derive-once-review.md:21 says kind F is found mechanically; :38 says the lines are "candidates". Differ for a reviewer deciding whether a precheck F line is a finding on its own.
- SPLIT (K1): the owner-map "Who checks it" and "What the planner may do with the map" are restated in reviewer.md:17, planner.md:19 and doc-review.md:63 although owner-map.md:3 says other pages point and do not copy.
- SPLIT: owner-map.md:39,:47 and a plan reviewer "signs off BLOCKED" on a contradiction; derive-once-review.md:106 says a code reviewer with an unfixed held finding signs CLEAN (different jobs; one reviewer covering both reads two instructions). (Not re-read in full.)
**Ruling:** owner-map.md:7 quotes the user ("Yes, write them"); this was not re-checked against a Confirmed line. Not opened: code-and-engine.md.
**Open:** whether the two copies differ in effect, not only in words (a reading).

### L-F18-13
**Verdict:** CONFIRMED.
**Homes as found:** C:\Users\Patrick\.claude\model-postures.md:7 "A task may start before its blocker's review is done only when the plan lists that blocker in the task's `earlyStart`." Hydra docs/superpowers/plans/2026-11-08-player-audit-fixes.md:124 and :140 use `earlyStart: F1`. A grep of gates/*.py for "earlyStart" finds no hit; no schema or checker for the key found (searched gates/*.py and model-postures.md only; ~/.claude/gates/state, workflows and session-cards condensed files also name it as prose). gates/agent_stop.py:20 `REVIEWER_TYPE = "reviewer"` and gates/review_dispatch.py:14 `REVIEWER_TYPE = "reviewer"` (two constants, same value). Cards: CLAUDE.md "Past sessions" ("Every past session of this project has a short card ... in the repo at `docs/sessions/`"); C:\Users\Patrick\.claude\tools\session_cards\writer-brief.md:18 "**A session not about the project is kept locally.** ... add ` | local: yes` ... files the card in a local folder that is never published, and the public index only counts it."; tools/session_cards/check.py:6 "every card in the cards root or the local-cards fold".
**Splits:**
- SPLIT (K7): `earlyStart` is a plan field a reviewer reads by eye; no hook or script reads it.
- SPLIT (K1): `REVIEWER_TYPE` is written twice (agent_stop.py:20, review_dispatch.py:14).
- SPLIT (K2, minor): CLAUDE.md says every session has a card in the repo; a local session's card is never in the repo.
**Ruling:** none found.
**Open:** audit-method.md "four exits" copies and the "staged on main" rule were not compared.

### L-F18-15
**Verdict:** CONFIRMED-CHANGED (the finder's split no longer exists: prose and code now agree).
**Homes as found:** gates/agent_stop.py:136-158 `signed_tip` docstring: the verdict is for this reviewer's newest commit on the recorded branch after the handed tip; with none, for the handed tip; "A branch that is gone or no longer descends from the handed tip, or reviewer commits on it that do not form one chain, leave no commit to sign" and the code checks `merge-base --is-ancestor tip head` and `merge-base --independent` (:118-158 area); `agent_commits` :123-135 reads the Agent trailer. C:\Users\Patrick\.claude\agents\reviewer.md:33 "It reads the one branch the dispatch recorded ... It checks that the branch still descends from the tip you were handed, and records your verdict for your newest commit on it, or for the handed tip when you committed no fix." Hydra derive-once-review.md:112 "reads that one branch from git, checks that it still descends from the key you were handed, and records your verdict for your newest commit on it, or for the key when you committed no fix." doc-review.md:95 (same branch, `Agent:` trailer). gates/README.md:45 "It signs the reviewer's own newest commit, not the branch head ... With none, the reviewer made no fix, and the verdict goes to the handed tip." ref_judge.py:242-251 `missing_trailers` skips merges (`len(parents) > 1`).
**Splits:** none found. The ledger's K3 split between "descends from the handed tip" and "newest commit found by the Agent trailer" is two halves of one rule: the descent is checked first, then the newest own commit is chosen.
**Ruling:** review-and-hooks.md:20 "i asked for the review agent to fix its findings directly, then we're done."
**Open:** integrate.md:27 and brief-preamble.md:65 not re-read for this row; the README:160 line about a reviewer merging main into a branch giving the merge its own `Agent:` trailer (a merge has no required trailer, ref_judge.py:244) is a prose request with no check.

### L-F18-17
**Verdict:** CONFIRMED.
**Homes as found:** gates/README.md:53-55 names owners: `missing_trailers` (ref_judge.py:242), `judge_push_lines` (ref_judge.py:184), `restore_checkout` (ref_judge.py:431), `_code_cause` (review_rules.py:231), `explain_main_move` (review_rules.py:419); all exist (lines shifted from the ledger). ref_judge.py:243-245 docstring "A merge commit ... needs none: the user's ruling named an agent's own commits, and git writes a merge's message itself." Hydra docs/agents/rulings/review-and-hooks.md:57 (Unconfirmed; under `## Unconfirmed` at :37): "User chose a git-side reference-transaction + pre-push hook as the only judge of 'may main move' and of trailers". brief-preamble.md:65,:67 gives the trailer shape. gates/README.md:160 "fast-forwards `main`: `git -C C:\Users\Patrick\.claude merge --ff-only <branch>`. Fast-forward only, so a live hook never holds a conflict marker."; :166 revert recipe. The hooks repo's marker `C:\Users\Patrick\.claude\.derive-once-gate` lists `gate: gates/` and `gate: agents/`, so main moves there are judged by the ref gate. A grep of ref_judge.py, review_rules.py and README.md for "ff-only|fast-forward|non-fast" finds only README lines; no hook refuses a non-fast-forward main move in the hooks repo (searched those two .py files only).
**Splits:**
- SPLIT (K5/K7): "fast-forward only" is a prose rule with no refusal found; a merge commit made on main directly would be judged as a code move (review or waiver), not refused for being a merge.
- SPLIT (K8, mild): ref_judge.py:244 names "the user's ruling" about an agent's own commits; the only matching line is the Unconfirmed note above (grep "trailer" in the rulings folder: review-and-hooks.md:57 only).
**Ruling:** none Confirmed. agent-rules.md:7 cites "the user's ruling, 2026-10-14" for the test list, and the "ff-only" is not in it.
**Open:** whether a real non-fast-forward merge on the hooks repo's main is refused (a run, not done).

### L-F18-21
**Verdict:** CONFIRMED for the step numbers and citations (all that were opened match today); a D13 example problem was found.
**Homes as found:** Hydra docs/agents/derive-once-review.md:42-53 numbered steps 1-10 (1 questions, 2 search, 3 truth tables, 4 new numbers, 5 tests, 6 docs, 7 known, 8 new homes, 9 what was asked for, 10 boundary). Citations: agent-rules.md:18 "step 9" (rule 12 match), :41 "step 8 ... `CONTEXT.md` word a second meaning" (step 8 at :49 names "CONTEXT.md term" so it fits); brief-preamble.md:115 "do the reviewer's first step yourself ... List each question" (step 1 fits); tools/precheck/check_5.cpp:32 "derive-once review, step 8" (second homes; step 8 fits); tools/precheck/changed_names.h:3 "docs/development.md, 'The merge test picker', step 3" (a different list; development.md:541-546 step 3 "takes the names each changed line defines"). Internal: derive-once-review.md :33,:36,:38,:57,:86,:106,:108 "step 10", :38 "step 8", :80 "step 3", :110 "step 5", :134 "step 9", :211 "step 4".
**Splits:**
- SPLIT (K2/K8, the D13 example): derive-once-review.md:52 says "D13 approved 'free', not the sentence 'no more than 0.0 ms late'". docs/audit/2026-10-03-fix-decisions.md:33 (D13, 2026-10-03) itself says "The sentence at offset 0 reads 'no more than 0.0 ms late'", so D13's own text approved that sentence. docs/audit/2026-10-03-fix-decisions.md:1680-1686 (D302, 2026-10-31) reverses D13, and records the user's words :1682 "who came up with this rule and when was it approved by me? ..." and "why does the squeeze in text say 'no more than 0.0ms late' instead of 'more than 0.0ms early' ??? who approved this text?? this makes no sense". So the example is right about what the user later said, but it is wrong about what D13 states; and D13 is the reversed record. Which of the two the example should rest on is not stated.
- SPLIT (K7): renumbering the ten steps would break the number cites silently (no check ties them).
**Ruling:** questions-and-audits.md:18 (Confirmed, 2026-10-31) chose "Yes, add it (Recommended)" for the "was this asked for?" check; :19 "Proposal 5" (the stray-edit rule cited at agent-rules.md:19 for rule 13). Neither names the D13 wording.
**Open:** the Pitch Dark example in derive-once-review.md:55-72 against docs/clone-hero/05-star-power.md:437 is row L-F18-22 (not mine).

## Ids not reached
None. All 20 rows were read. Parts left open and named in the rows: L-F16-15 check_handoff exit codes (main() not read), L-F16-17 and L-F16-19 doc pages (doc-screenshots.md, clone-hero-mcp.md), L-F18-12 owner-map.md:7 quote, L-F18-13 audit-method four-exits copies.
