# TOOL-AUDIT-T8-K2 closure check (18 ids)

H = C:\Users\Patrick\Downloads\Hydra\hydra-test, G = C:\Users\Patrick\.claude\gates, M = memory folder.
Result: 16 PASS, 2 REOPEN (L-F09-12, L-F13-17). Every id reached.

### L-F07-10
**Verdict**: PASS (exit 4)
**Homes read**: ~\.claude\skills\analyzing-sp-squeezes\SKILL.md:64 "Note whether the fleet hooks held: one background executor per task, no nested background workers."; H\docs\agents\brief-preamble.md:77 "Never use run_in_background ... You have no helper agents".
**Why**: The skill line is a note a main session makes about its own fleet. The preamble limits what an agent does in its own shell. They answer different questions.

### L-F07-17
**Verdict**: PASS (exit 3, "Note is wrong")
**Homes read**: M\archive\agent-worktree-base-is-origin-main.md:15 "Reviewers still use `isolation: worktree`"; H\docs\agents\derive-once-review.md:200 "leave isolation out ... the hook refuses a reviewer dispatch that sets isolation"; G\review_dispatch.py (isolation refusal present); G\README.md first-call paragraph on spawnedWithWorktree.
**Why**: The note says the opposite of the doc and the hook, and the rule it contradicts is held by both doc and hook, so pruning the note drops no rule. The note is archived and not in MEMORY.md, so it also barely matters. The other parts of the row were reported as agreeing; I did not re-open them.

### L-F08-09
**Verdict**: PASS (exit 3, "Merge, I pick")
**Homes read**: G\sync_settings.py:39 LIVE = r"C:\Users\Patrick\.claude\settings.json"; G\tests\test_docs_numbers.py:54 SETTINGS = os.path.join(os.path.dirname(GATES_DIR), "settings.json"), comment lines 52-53, and :157-161 test skips when the file is missing.
**Why**: Both are code. Two typed ways to name the live file; in a gates worktree they differ and the test skips. Kind is right. (Line numbers in the report were off by a few; the content matches.)

### L-F09-12
**Verdict**: REOPEN
**Homes read**: H\docs\agents\ui-testing.md:195 "`snapshot_library` is the only code that knows the library folder's path."; H\tools\hydra_ui_mcp.py:55 DEFAULT_LIBRARY_DIR and :140; H\tools\compare_db.py:51,87 (docstring only); H\tools\bench.cpp:974-991.
**Why**: The re-checker searched only .py and .ps1 files (and in a form that misses a C++ string with doubled backslashes). H\tools\bench.cpp:974-991, `installed_folders()`, is code that works out the installed Hydra's folder (ProgramFiles variables plus the literal "C:\\Program Files\\Hydra") and `refused_db` uses it to refuse --open on a file inside it. Its own comment says "The folders an installed Hydra keeps its library in". So the doc's claim "the only code that knows the library folder's path" is false today, and the row is a doc-vs-code split (and a second owner of "where is the installed library"). It should not be closed as not a split. Probably it goes to exit 3 "The code wins" for the doc sentence, or to a question if the user wants one owner.

### L-F09-13
**Verdict**: PASS (doc-vs-code: "The code wins"; doc-vs-doc: "Merge, I pick")
**Homes read**: H\docs\agents\this-machine.md:81 "copy hydra.db-wal and hydra.db-shm along with hydra.db"; H\docs\agents\ui-testing.md:195 backup API, no -wal in the copy; H\tools\hydra_ui_mcp.py:110-132 copy_database (sqlite3 backup through a read-only connection, plain copy only when no -wal); H\tools\compare_db.py:85-87 (docstring recipe).
**Why**: Real split. The deciding code is hydra_ui_mcp.copy_database, which is real code, not only text. Fixing the doc to match opens no guard gap.

### L-F10-23
**Verdict**: PASS (exit 3, "The code wins")
**Homes read**: H\docs\agents\ui-testing.md:294 "`report_window_flow_tests()`, which `register_tests` calls last"; H\tests\ui\uitest_tests.cpp:34-35 register_entries(report_window_flow_tests()); register_entries(command_tests()).
**Why**: The code registers command_tests after the flow tests, so "last" is wrong. Code is real, doc fix opens no gap. The memory-note 150 call cap matches G\progress.py:14.

### L-F11-04
**Verdict**: PASS (exit 3, "Fix without asking")
**Homes read**: G\tests\lib\harness.ps1:11 "Its proof is tests\test_h39_harness.ps1." and :21-23 HarnessShell/HarnessDrainMs; G\tests\ (no test_h39_harness.ps1); a find over ~\.claude finds that file only in ~\.claude\worktrees\{displays-read-engine,stop-hook-no-guess,stop-hook-range-plan}\hooks\tests\; grep of G for HarnessShell finds only harness.ps1.
**Why**: The named file is gone from the live gates, with nothing replacing it. Only old worktrees hold it. Gone with no replacement is a fair moved-pointer case.

### L-F13-06
**Verdict**: PASS (doc-vs-code: "The code wins"; doc-vs-doc: "Merge, I pick")
**Homes read**: H\docs\agents\brief-preamble.md:33 `python -I <file>`; H\docs\agents\this-machine.md:7-9 "Run Python as `py`", "A bare `python` reaches a different install"; G\ruling_rules.py:108 CHECK_COMMAND = "python.exe -I ..." and G\..\settings.json:12 allow rule `PowerShell(python.exe -I ... ruling_rules.py --check *)`; G\sync_settings.py:20 (docstring only); G\README.md:112,116,147 use the full path.
**Why**: Real split. The code side that decides is the settings.json allow rule and CHECK_COMMAND (a permission string a program matches), not only printed text. Matching the doc to it opens no guard gap.

### L-F13-09
**Verdict**: PASS (exit 3, "Merge, I pick")
**Homes read**: H\docs\agents\integrate.md:3 "its rules apply to you in full" and :27 "Hold the last task's merge uncommitted until the precheck in step 4 is clean"; H\docs\agents\brief-preamble.md:71 "Commit before your 100th tool call, and after each finished step."
**Why**: Two docs, two readings of when to commit. Kind is right.

### L-F13-17
**Verdict**: REOPEN
**Homes read**: H\docs\agents\issue-tracker.md:14 "**Close**: `gh issue close <number> --comment \"...\"`"; H\docs\agents\this-machine.md:15 "Text never goes to GitHub through a hand-typed `gh` command"; G\settings.fragment.json:12-13,26-27,40-41 deny `gh issue create*` and `gh issue comment*` but nothing for `gh issue close`; G\publish_gate.py has no close handling.
**Why**: The close split is the same one as L-F13-08, which the fold sent to a question (exit 2) because "The code wins" would keep an ungated way to post text. The brief says "The code wins" fails when making the doc match the code keeps a guard gap open. Here it would: this-machine.md says text goes only through publish.ps1, and `gh issue close --comment` does not. So the same row cannot take exit 3 for its doc-vs-code part. It should follow L-F13-08: a question to the user. The label edit part (issue-tracker.md:12) carries no prose and is fine.

### L-F13-20
**Verdict**: PASS (exit 4)
**Homes read**: H\docs\agents\brief-preamble.md:67 "Co-Authored-By: Claude Opus 5.5" and :69 "Use the model line your brief gives if it differs."; grep of G\*.py for "Co-Authored" finds nothing, so no hook checks the model name.
**Why**: One side is the harness's attribution reminder, a message and not a file. The preamble lets a brief set the model line. Agree it is framing, not two file texts.

### L-F13-23
**Verdict**: PASS (exit 3, "Merge, I pick"; the pwsh half is exit 4, see L-F14-02)
**Homes read**: G\README.md:17 "The hook tests on this page need no build, so agents still run them."; H\docs\agents\brief-preamble.md:47 "Never run all the hook tests ... Run only the tests your brief names."
**Why**: Two docs answer "may an agent run hook tests unprompted" two ways. Kind is right.

### L-F13-24
**Verdict**: PASS (exit 4)
**Homes read**: G\README.md:5 "live in ... brief-preamble.md. This page does not repeat them." and :19, :23, :25-27; H\docs\agents\brief-preamble.md:33-35,43; G\shell_output.py:15 CAP = 4000; G\bash_windows_rule.py:16 MAX_CHARS = 1000.
**Why**: The README describes how each hook works and points at the code for numbers; the preamble tells agents what they must do. I checked the numbers in the preamble (4,000 and 1,000) against the code and they match. I read the delete, trim and Bash-shape passages; I did not diff every sentence.

### L-F14-01
**Verdict**: PASS (doc-vs-code and doc-vs-doc: "The code wins" and "Merge, I pick"; memory part "Note is wrong" with a caveat)
**Homes read**: H\docs\agents\agent-rules.md:14 "(about 115 charts)"; H\docs\agents\ui-testing.md:48 "98 charts"; H\tests\test_engine_sweeps.cpp:656 CHECK(charts == 98); count now: 98 song.ini files under H\testdata\input; M\questions-describe-the-real-rule.md:24 "Strip the `[N/97]` prefix".
**Why**: 115 vs 98 is real. The memory part: I could not show the batch tool prints 98, because I may not run it and could not find the format string in the tools folder. If it prints 97, that third split is not real; the other two stand either way. Mark the memory part unchecked.

### L-F14-02
**Verdict**: PASS (exit 4, borderline)
**Homes read**: H\docs\agents\this-machine.md:51 "Run scripts with `pwsh` ... a plain call to a script [in powershell.exe] is refused"; G\README.md:154 "Run one with `powershell -NoProfile -ExecutionPolicy Bypass -File <...>`".
**Why**: The README passes the policy flag that the refusal needs, so both run. Note that it does name `powershell` where this-machine names `pwsh` for scripts; the main session may want to say so when it writes the fix, but it is not two contradictory rules.

### L-F14-03
**Verdict**: PASS (doc-vs-code "The code wins"; doc-vs-doc "Merge, I pick")
**Homes read**: same as L-F09-13, plus H\docs\agents\ui-testing.md:350 ("copy ... while the installed Hydra is closed", which agrees with this-machine because a closed Hydra leaves no -wal).
**Why**: Same split as L-F09-13, same code owner.

### L-F14-06
**Verdict**: PASS (exit 4)
**Homes read**: H\docs\agents\ui-testing.md:354 "The live window takes no input from the real mouse (the user, 2026-10-29: ...)"; H\docs\agents\rulings\ui-and-display.md:43 under ## Confirmed (section runs lines 7-101) "my mouse doesnt work in it"; grep of H\src, H\tools\hydra_ui_mcp.py and H\tests\ui for real mouse, NoMouse, WantCaptureMouse, MouseDrawCursor finds nothing that says otherwise.
**Why**: The doc repeats what the user saw. No file says the opposite. Not a split.

### L-F14-11
**Verdict**: PASS (exit 4)
**Homes read**: H\docs\agents\clone-hero-mcp.md:23; M\ch-runs-must-not-need-focus.md (body and Traps paragraph); H\docs\agents\rulings\clone-hero.md:34 (Confirmed section starts line 7, Unconfirmed at 67).
**Why**: Doc, note and ruling agree: brief focus is fine, a run must not depend on focus, never show the window.

## Ids not reached
None.
