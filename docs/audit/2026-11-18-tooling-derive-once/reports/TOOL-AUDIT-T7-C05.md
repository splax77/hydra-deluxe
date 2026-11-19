# TOOL-AUDIT-T7-C05 (closure checker, batch 4: S03 / L03)

H = C:\Users\Patrick\Downloads\Hydra\hydra-test; G = C:\Users\Patrick\.claude\gates; C = C:\Users\Patrick\.claude.
Counts: PASS 24, REOPEN 1 (L-F08-09), GONE 0. Not reached: none.

### L-F07-03
**Verdict:** PASS (A)
**Homes read:** H\docs\agents\ui-testing.md:209-229 (says "this table is a copy of" the tool list in tools/hydra_ui_mcp.py; 17 rows, hydra_start to hydra_run, no hydra_build_pass); H\tools\hydra_ui_mcp.py:874 (`def hydra_build_pass`), 18 `def hydra_` in all, :76 NO_SESSION `"no session; call hydra_start"` which ui-testing.md:189 quotes the same; H\tools\test_hydra_ui_mcp.py:51-55 (18 names); G\percall.py:20 and :142 `mcp__hydra__hydra_build_pass`; G\README.md:15; H\CLAUDE.md:76.
**Why:** The only split is a doc table missing a tool its own code owner defines. Name copies in percall and README agree. Doc against code.

### L-F07-04
**Verdict:** PASS (P)
**Homes read:** H\tools\ch_mcp\front.py:60 and server.py:205 (`"nothing open"`); G\percall.py:16-18 (KEY_TOOLS, KEY_ROUTE_TOOLS `{"mcp__ch__ch_play_chart": "keys"}`) against server.py:147 and :388 (route "kit" or "keys"); H\docs\agents\clone-hero-mcp.md:11; kit.ts ALLOWED_CALLS (41 names) against tools\ch_mcp\kit_methods.json (all 41 are in it; the 6 extra names there are field reads, which kit.ts:18-20 says it reads by name); H\tools\ch_mcp\kit_tools.py:92 `KIT_PLAYER = "Player1"` with server.py:320 and kit_tools.py:1061 pointing at it.
**Why:** Every copy holds the same value today. I found no second value anywhere. A pointer in place of the copies changes no behaviour.

### L-F07-05
**Verdict:** PASS (A)
**Homes read:** G\tests\replay.py:19-20 ("the main-session route above does not reach that rule"); G\percall.py:4 (docstring: both sides run bash_windows_rule), :149-150 (subagent route calls bash_windows_rule.check), :160-163 (`_main_session` calls bash_windows_rule.check).
**Why:** The replay.py comment says the main-session route does not reach the rule. The code does reach it. A comment against code; code is right.

### L-F07-07
**Verdict:** PASS (A)
**Homes read:** G\status_append.ps1:5 ("the call shape: the brief preamble's Status lines section"), :9 (progress.py owns how a status write is recognised); G\progress.py:22-26 (STATUS_PREFIX and _STATUS_CALL regex); H\docs\agents\brief-preamble.md:15 (the call as `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>"`).
**Why:** The sides are two comment lines naming different owners for the shape. The code (progress.py regex) decides the shape, and the preamble text matches it. Comment against code; the code wins.

### L-F07-12
**Verdict:** PASS (P)
**Homes read:** G\ref_gate.sh:6-8 (`[ "$CLAUDECODE" = 1 ] || exit 0`); H\tools\build_slot.ps1:88-90 (`$env:CLAUDECODE -eq '1'`, "the same test the git ref gate uses"); H\Directory.Build.targets:5-9 (`'$(CLAUDECODE)' == '1'`).
**Why:** All three test for "1". No disagreement. (build_slot gating any Claude Code shell, while the agent refusal is on the tool in percall.py:142, is two different layers, not a second value.)

### L-F07-13
**Verdict:** PASS (P)
**Homes read:** G\README.md:176-190 (2.1.308 probes; bash measurements 2.1.310; SubagentStop not fired for maxTurns); G\tests\probe_results.md:7, :25-34 (rows 1, 7, 9, 10), :53, :83, :108.
**Why:** README restates probe_results.md and every figure I compared (versions, maxTurns result, SubagentStop fields, reviewer agent_type) matches.

### L-F07-14
**Verdict:** PASS (P)
**Homes read:** H\.claude\commands\handoff.md:9. Searched H\docs\agents, H\CLAUDE.md and H\.claude\commands for "stop launching" and "no new agent": one hit.
**Why:** One home, nothing to disagree with.

### L-F07-15
**Verdict:** PASS (P)
**Homes read:** C:\Users\Patrick\.claude.json:4779 (`rea-agents@6.5.0`, the only occurrence); H\docs\agents\this-machine.md:25; C:\Users\Patrick\.claude\skills\reverse-engineer-anything\SKILL.md:37,51,56,71,79.
**Why:** Same pin everywhere. (Codex approval home references\connection-and-recovery.md was not opened; it was a single home and the row's split question is about the pin.)

### L-F08-01
**Verdict:** PASS (P)
**Homes read:** G\media_rule.py (MEDIA_ROOT C:\ytdl, per the ledger and the main/subagent code at :47-55). I searched G for "ytdl" in code; the code is the single owner and other places are pointers.
**Why:** No second value.

### L-F08-02
**Verdict:** PASS (P)
**Homes read:** G\shell_output.py:26-28 (SAVED_FOLDER "shell-output", FALLBACK_FOLDER "tool-results", NAME_DIGITS 12); G\README.md:23 ("shell-output" folder inside the session scratch folder).
**Why:** Constants and README agree; this session's own saved shell output is in a `shell-output` folder under the scratchpad, matching.

### L-F08-03
**Verdict:** PASS (P)
**Homes read:** The removed-plan string `git show 444631e6:docs/superpowers/plans/2026-10-14-session-cards-backfill.md` in C:\Users\Patrick\.claude\tools\session_cards\check.py:17, batch.py:11, common.py:5, index.py:12, link_memory.py:12, number_rulings.py:17, prepass.py:17, writer-brief.md:5 (8 copies, identical). `git cat-file -e 444631e6:<that path>` succeeds in H.
**Why:** All eight copies name the same commit and path and it resolves. A removal note is what the Hydra docs rule asks for, so nothing disagrees.

### L-F08-04
**Verdict:** PASS (P)
**Homes read:** G\ref_judge.py:43-45 (RULINGS_DIR = "docs/agents/rulings/", comment saying it is named here so ruling_rules.py reads it from here); G\ruling_rules.py:12, :59, :94 (FORMAT_PAGE = RULINGS_DIR + "README.md"); C:\Users\Patrick\.claude\tools\session_cards\common.py:53 and writer-brief.md:67 (`## Confirmed` in Hydra's docs/agents/rulings/).
**Why:** One code owner; the prose copies say the same folder.

### L-F08-05
**Verdict:** PASS (P)
**Homes read:** G\review_rules.py:18 (`".derive-once-gate", ".doc-review-gate"`); G\publish_gate.py:12 (`MARKER = ".doc-review-gate"`); G\README.md:49,138; H\docs\agents\doc-review.md:17.
**Why:** Same marker text everywhere.

### L-F08-07
**Verdict:** PASS (P)
**Homes read:** `...\Local\Python\pythoncore-3.14-64\python.exe` in G\install_ref_gate.ps1:10, G\publish.ps1:16, G\settings.fragment.json (9), C\settings.json (9), G\README.md (3); H\docs\agents\this-machine.md:7 (says `py` starts that exact file; `py -c "import sys;print(sys.executable)"` printed the same path); H\docs\agents\derive-once-review.md:124 (full path).
**Why:** Same file in every home. `py` and the full path are the same file on this machine. Only the spelling differs in prose.

### L-F08-08
**Verdict:** PASS (P)
**Homes read:** G\ref_judge.py:35 (ALLOW/DENY strings), :36 (SELFTEST_OK); G\ref_gate.sh:15-16; G\install_ref_gate.ps1:13 (`$selftestOk`).
**Why:** The verdict and self-test strings are identical in all three.

### L-F08-09
**Verdict:** REOPEN (should be a question, Q)
**Homes read:** G\sync_settings.py:39 (`LIVE = r"C:\Users\Patrick\.claude\settings.json"`); G\tests\test_docs_numbers.py:44-48 (SETTINGS = settings.json beside the gates folder), :142-146 (skips when there is none, "a worktree").
**Why:** On main both name one file. In a gates worktree they name different files and the test skips on purpose. Pointing the test at sync_settings.LIVE would make a worktree test read the live file instead of skipping, which changes behaviour and needs a choice. That is not P. (This is the sorting file's own reason, which admits a difference.)

### L-F08-12
**Verdict:** PASS (C)
**Homes read:** G\session_cards_start.ps1:21 ("state\session_cards\, a subfolder, so state_prune.ps1 leaves it alone"); G\cleanup_sweep.ps1:589-600 (`Invoke-StatePrune`: prunes only status, agents, stall, posture and logs; "Any other subfolder is left alone"), :16, :28. Searched C:\Users\Patrick\.claude for "state_prune": only that comment.
**Why:** Current place is clear: Invoke-StatePrune in cleanup_sweep.ps1:592, and its behaviour matches what the comment claims.

### L-F08-16
**Verdict:** PASS (P)
**Homes read:** C:\Users\Patrick\.claude\tools\session_cards\writer-brief.md:99-101 (5 minutes, 20 minutes, 100 tool calls); G\progress.py:13-16 (CALL_WARN 100, MINUTES_WARN 20, STATUS_NAG_MINUTES 5; comment "the one home"); agents\card-writer.md:9 (repeats no number).
**Why:** Equal values.

### L-F09-01
**Verdict:** PASS (P)
**Homes read:** H\CLAUDE.md:9, :29; H\docs\agents\brief-preamble.md:105; H\docs\clone-hero\README.md:78-80; writer-brief.md:64, :81, :88; agents\card-writer.md:9.
**Why:** All say the fact goes to its page, same commit, never a memory note; an agent without the page puts it in its report; card writers record a `write-back:` line and the main session adds it. Same rule, no second value. All docs; no code involved.

### L-F09-02
**Verdict:** PASS (P)
**Homes read:** H\docs\agents\doc-review.md:81; H\docs\agents\brief-preamble.md:123; writer-brief.md:94; C:\Users\Patrick\.claude\model-postures.md:11, :13; G\stall.py:32; H\docs\agents\public-doc-style.md:3; agents\scout.md:9.
**Why:** Each names the global CLAUDE.md as the home or restates it the same way. No content split.

### L-F09-03
**Verdict:** PASS (B)
**Homes read:** memory\hydra-deluxe-rename.md:3 (description says repo is patseguin1-commits/hydra-deluxe), :11 (body: moved to splax77/hydra-deluxe, current); H\CLAUDE.md:13 and H\docs\agents\this-machine.md:15 (splax77/hydra-deluxe); `git remote -v` in H (splax77/hydra-deluxe).
**Why:** The only wrong side is the memory note's own description line. this-machine.md:13 names `patseguin1-commits` as the signed-in gh account, which is not a repo name, so it is not a split.

### L-F09-04
**Verdict:** PASS (P)
**Homes read:** H\CMakeLists.txt:21 (`VERSION 3.1.2`); H\docs\agents\this-machine.md:17 ("reads VERSION 3.1.2 today", tag is v plus the version).
**Why:** Equal. The earlier "not found" note was wrong; the literal is there.

### L-F09-05
**Verdict:** PASS (P)
**Homes read:** this-machine.md:7, :19, :23, :25, :37, :39, :41, :53, :57, :67, :71; C\.claude.json:4779 and :4783 (GHIDRA_INSTALL_DIR equals this-machine.md:23's Ghidra folder); G\publish.ps1:16 and G\install_ref_gate.ps1:10; tools\ch_mcp\game_files.py:38 (LocalLow path, case only differs from :53); C\settings.json:3 (`claude-opus-5-5` equals :71) and tools\agent_cost.py:38; the Build Tools folders at :37 and :39 exist on disk; tools\find_cmake.ps1:9 uses vswhere as :41 says.
**Why:** No pair of homes says different things. Not checked: the version numbers in the VS rows and the cdb lookup against the machine (a run). Those have no second home to disagree.

### L-F09-07
**Verdict:** PASS (P)
**Homes read:** G\README.md:156; H\docs\agents\agent-rules.md:7; C\model-postures.md:11. All list `common.py`, `lib`, `settings.fragment.json`, `tests\lib`. G\lib holds hook_input.ps1 and G\tests\lib holds harness.ps1, so the paths exist.
**Why:** Same four items in all three. agent-rules.md:7 says precheck decides shared code for Hydra changes, then types the hook list by hand; that is not a second list. Nothing in code holds the hooks list, so a pointer changes no behaviour.

### L-F09-08
**Verdict:** PASS (C)
**Homes read:** H\.derive-once-gate (cites "Test-DeriveOnceGatedPath in the user-scope hook (hooks\lib\derive_once_rules.ps1)"); H\.doc-review-gate (cites "hooks\lib\doc_review_rules.ps1"). C:\Users\Patrick\.claude\hooks holds only `state`; no lib folder, and no `Test-DeriveOnceGatedPath` or those two filenames exist in G (searched). Current owners: G\review_rules.py:18 (markers), :22 BUILTIN_CODE, :157-185 (`_code_paths`, `_is_code`); for the doc gate, G\publish_gate.py:12 (MARKER) and G\review_rules.py:24 PUBLIC_DOCS (which pages count). C\.derive-once-gate itself says "gates/review_rules.py reads these lines".
**Why:** Both marker lines point at files that do not exist, and the current place is clear.

Ids not reached: none.
