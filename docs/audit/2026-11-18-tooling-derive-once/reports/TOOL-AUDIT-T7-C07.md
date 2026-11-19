# TOOL-AUDIT-T7-C07 (closure checker, batch 6: sorting S04, ledger L04, F10-01 to F11-13)

H = C:\Users\Patrick\Downloads\Hydra\hydra-test, G = C:\Users\Patrick\.claude\gates, M = the memory folder. All 25 rows read against the current files. Totals: PASS 25, REOPEN 0, GONE 0. Two PASS rows carry a caution (F10-19, F11-07).

### L-F10-01
**Verdict:** PASS (A).
**Homes read:** H/docs/agents/this-machine.md:81 ("copy `hydra.db-wal` and `hydra.db-shm` along with `hydra.db`"); H/docs/agents/ui-testing.md:195 (online backup when a -wal sits beside it); H/tools/hydra_ui_mcp.py:107-134 `copy_database` (backup when -wal exists, plain copy otherwise).
**Why:** The split is this-machine.md (a doc) against copy_database (a program file). ui-testing.md:195 agrees with the code, so it is no separate side. The code gives one clear answer.

### L-F10-02
**Verdict:** PASS (C).
**Homes read:** H/docs/agents/this-machine.md:51 says "PowerShell 7.6.9"; the machine reports 7.6.10 (`$PSVersionTable`, path unchanged).
**Why:** A moved version number with a clear current value.

### L-F10-03
**Verdict:** PASS (B).
**Homes read:** M/hydra-deluxe-rename.md:3 (description says patseguin1-commits/hydra-deluxe) and its line 11 (splax77/hydra-deluxe); `git remote -v` origin is splax77/hydra-deluxe; H/CLAUDE.md:13 and this-machine.md:15 say splax77.
**Why:** Only a memory note is wrong, against the git remote and docs.

### L-F10-04
**Verdict:** PASS (B).
**Homes read:** M/sp-squeeze-mechanics.md:4,11 (game rule is at SP-37); H/docs/clone-hero/05-star-power.md:403-412 (SP-37, "Label: Hydra assumes it, never checked"; game code lines list only SP-36 and SP-29).
**Why:** The only split is a memory note against a reference page.

### L-F10-05
**Verdict:** PASS (P).
**Homes read:** M/render-checks-use-the-corpus.md (description and body); H/docs/agents/ui-testing.md:9-13 (case 2 real app on the user's db copy, case 3 corpus, "never a whole-library analysis"); H/docs/agents/this-machine.md:89-92.
**Why:** The note and ui-testing.md say the same thing, and the note adds only the user's quotes. this-machine.md:92 asks for a db copy only for timings, and says to try testdata/input first, so it does not disagree. A pointer changes no behaviour.

### L-F10-07
**Verdict:** PASS (C).
**Homes read:** C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:34 (cites memory `gameplay-video-score-analysis`); M/archive/gameplay-video-score-analysis.md exists (and not in M top level); M/MEMORY.md says archive notes are not indexed.
**Why:** The name moved to archive/, and the current place is clear: M/archive/gameplay-video-score-analysis.md.

### L-F10-10
**Verdict:** PASS (P).
**Homes read:** M/git-traps-in-shared-checkout.md:14-16 (`git worktree add .claude\worktrees\<name> -b claude/<name> main`); H/tools/new_worktree.ps1:1-8,36 (makes <main>\.claude\worktrees\<TaskId> on claude/<TaskId> from -Base, default main, using `git worktree add`).
**Why:** The note's command and the script do the same job with the same path and branch name. A pointer to the script changes no behaviour.

### L-F10-12
**Verdict:** PASS (P).
**Homes read:** H/installer/hydra.iss:65 `OutputBaseFilename=HydraDeluxe-{#HYDRA_VERSION}-setup`; H/installer/build_installer.ps1:7,128 `HydraDeluxe-$version-setup.exe`.
**Why:** The two names agree, so only a copy is left.

### L-F10-13
**Verdict:** PASS (A).
**Homes read:** H/tools/hydra_ui_mcp.py:17,140 ("the only code that knows"/"the only place that knows the library folder's path"), :55 `DEFAULT_LIBRARY_DIR`; H/tools/bench.cpp:975-986 `installed_folders()` reads the Program Files variables and pushes "C:\\Program Files\\Hydra".
**Why:** A comment claim against a program file that also knows the folder.

### L-F10-14
**Verdict:** PASS (P).
**Homes read:** H/docs/agents/test-rubric.md:11 and :23; ui-testing.md:46,48; test-rubric-examples.md:101,113,116; H/tests/ui/uitest_batch_reports.cpp:389,481,494,507,514. Counted `notes.*` files under H/testdata/input: 98.
**Why:** Every typed 98 agrees with the corpus count today. test-rubric.md:23 itself calls such a number a snapshot, so it is a copy and no split.

### L-F10-15
**Verdict:** PASS (P). Caution: gh scopes were confirmed but I did not check the Visual Studio build numbers beyond reading them on the page.
**Homes read:** this-machine.md:5-16 (Python at the pythoncore-3.14-64 path, gh 2.97.0, account patseguin1-commits, scopes gist/read:org/repo/workflow); `py` reports that interpreter at 3.14.5; `gh --version` 2.97.0, `gh auth status` matches the account and scopes. The same pythoncore-3.14-64\python.exe string appears in G/publish.ps1:16, G/install_ref_gate.ps1:10, G/README.md:110,114,145, G/settings.fragment.json (9 lines), C:\Users\Patrick\.claude\settings.json (9 lines), G/tests/test_shell_output.py:74, H/docs/agents/derive-once-review.md:124. tools/ch_mcp/game_files.py:38 LOCALLOW_DIR.
**Why:** All typed copies name the same path. The bare-python difference (C:\Python314, 3.14.7) is documented on the page and depends on which command a run uses, not a conflict between homes.

### L-F10-16
**Verdict:** PASS (P).
**Homes read:** H/docs/agents/domain.md:5-9; H/CLAUDE.md "Domain docs" section; H has CONTEXT.md and docs/adr/, no CONTEXT-MAP.md.
**Why:** domain.md and CLAUDE.md agree. The "or" against "if it exists" is wording only, with no value to choose.

### L-F10-17
**Verdict:** PASS (P).
**Homes read:** H/tools/doc_shots/shots.txt (8 `screenshot` lines: main-screen, paths-tab, preview-tab, stars-tab, dynamics-tab, analysis-settings, search-tips, path-report); H/docs/images (the same 8 files); H/docs/agents/doc-screenshots.md ("the one list of the pictures"); H/tools/doc_shots.ps1:1-11.
**Why:** The list, the folder and the page agree (eight and eight).

### L-F10-18
**Verdict:** PASS (P).
**Homes read:** H/tools/hydra_ui_mcp.py:56,68-72 (`SNAPSHOT_FILES` = hydra.db, hydra_settings.ini, hydra_ui.ini) and :137-147 (library_dir, then HYDRA_LIBRARY_DIR, then the installed folder); H/docs/agents/ui-testing.md:193,344-352; H/tools/capture_window.ps1:1-10; H/docs/agents/this-machine.md:89.
**Why:** Order and file names agree in all homes. ui-testing case 3 (copy only hydra_ui.ini, write a settings file) against capture_window.ps1's header is a wording gap, not a conflict about a value. Line numbers cited in the ledger have moved, but the content holds.

### L-F10-19
**Verdict:** PASS (A). Caution: the live difference is now only this-machine.md:81 against the helper.
**Homes read:** H/docs/agents/ui-testing.md:350 (copy the three files "while the installed Hydra is closed"); H/tools/hydra_ui_mcp.py:107-134 and :60-64 (backup when a -wal exists); this-machine.md:81 (copy -wal/-shm too).
**Why:** Every split is a doc against the helper program, and the helper's `copy_database` gives the answer. ui-testing.md:350 already says to copy with the app closed, which fits the helper's plain-copy branch. The two docs are not set against each other here, so the code decides each.

### L-F10-20
**Verdict:** PASS (P).
**Homes read:** H/docs/agents/handoff.md:5-9 (`docs/handoffs/<YYYY-MM-DD>-<slug>-handoff.md`); H/.claude/commands/handoff.md:6-14 (same path; slug rule in step 0 only); docs/handoffs holds `*-handoff.md` files plus one older folder.
**Why:** The path shape agrees. The slug rule sits in one place, so there is nothing to choose between.

### L-F10-21
**Verdict:** PASS (B).
**Homes read:** M/video-decode-ceilings.md:11 ("h264 ~3,000 fps CPU, AV1 ~1,900 fps"); C:\Users\Patrick\Downloads\Hydra\video-tools\README.md:503-530 (table: h264 2,617 / 2,678 / 894 / 2,798; AV1 1,787 / 1,834 / 669 / 2,122) and :582 (894 against 2,557). A grep for 3,000/1,900/3000/1900 in the README found nothing.
**Why:** The only split is a memory note against the README. The README table cells are now read.

### L-F11-05
**Verdict:** PASS (P).
**Homes read:** H/tools/hydra_ui_mcp.py:81 `REALAPP_FILE = "realapp.jsonl"`; G/review_rules.py:44-49 (comment says Hydra's hydra_ui_mcp.py owns the name; `REALAPP_FILE = "realapp.jsonl"` typed at line 49).
**Why:** Both values are the same string, so pointing one at the other changes nothing.

### L-F11-06
**Verdict:** PASS (A).
**Homes read:** H/tools/ch_probe/constants.py:3-8 ("This is the ONE place the numbers live ... nothing else hard-codes an address"); H/tools/ch_mcp/agent/menu.ts:54 `PLAYER_SCREEN_FRAME_RVA = 0x32F590`; H/tools/ch_mcp/launch_patch.json:8,11 (0x3968D0, 0x396EC5); H/tools/ch_mcp/start_candidates.json:19-20,29 (more RVAs).
**Why:** A comment claim against a ts file and two JSON files a program reads.

### L-F11-07
**Verdict:** PASS (A). Caution for the main session: the split is a rule list in a doc against the precheck program, and "the code wins" would drop the rulings folder from the list. I read the exit as the category defines it, so I did not reopen it.
**Homes read:** H/docs/agents/derive-once-review.md:45 (step 4, decision homes include a line under `## Confirmed` in docs/agents/rulings/); H/tools/precheck/decisions.h:55-63 and decisions.cpp (read ADRs, CONTEXT.md, plan "User decisions" and "Test limits", decision-log D-records); `grep -c rulings` gives 0 in both files. The doc says Git does not check those other places, so the review does.
**Why:** Doc against code.

### L-F11-08
**Verdict:** PASS (A).
**Homes read:** H/tools/replay.cpp:28-29 ("The scoring itself lives in core/replay.h"); H/src/core/replay.h:1-17 ("It owns no scoring rule"); H/src/core/replay.cpp:92,185-194 calls chord_score_table, core::after_sp_end, core::sqout_position, core::paid_by_sp, core::backend_row_value; those are defined in H/src/core/backend_value.h:18,34,63.
**Why:** A comment in a tool against the core code, which really holds the scoring in other files.

### L-F11-09
**Verdict:** PASS (A).
**Homes read:** G/tests/lib/harness.ps1:4-7 ("the one home of how a test runs a hook ..."); Python tests copy hooks themselves: G/tests/test_publish.py:205,257-258,313,408; test_ref_judge.py:67,449,718,874; test_delete_rule.py:380; test_review_dispatch.py:343. PowerShell tests use `New-HooksCopy` (test_h52_cleanup_sweep.ps1:17,473; test_h53_session_cards_start.ps1:19).
**Why:** A comment claim against Python test code, which does it separately.

### L-F11-10
**Verdict:** PASS (P).
**Homes read:** grep of H/tools, H/docs/agents, CMakePresets.json and the root scripts for `build-cpp`. Every Release path is `build-cpp\Release\...`. H/CMakePresets.json:10 has binaryDir `${sourceDir}/build-cpp`; the `build-cpp-vs2022` preset at :17 is a different preset and is named nowhere else.
**Why:** No home disagrees. The fix points them at the preset.

### L-F11-12
**Verdict:** PASS (P).
**Homes read:** H/tools/ch_mcp/build_facts.py:28 `DUMPS_ROOT`; kit_methods.json:3 and launch_patch.json:4 ("dump"); tools/ch_mcp/README.md:7,12,20,58; test_dump_facts.py:282; docs pages. All name C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps.
**Why:** Same path in every home. The two JSON "dump" values are the same folder, 6142\dump.cs.

### L-F11-13
**Verdict:** PASS (P). The row was NOT VERIFIED, so I read the homes myself.
**Homes read:** C:\Users\Patrick\.claude\tools\session_cards\common.py:33-36 (PROJECT_NAME, DEFAULT_MEMORY, HYDRA_REPO; this file has moved out of H/tools); H/tools/check_pointers.py:30 `MEMORY_DEFAULT` (same project folder, same memory path); H/tools/agent_cost.py:31; H/tools/effort_test_taint.py:31 (C:/Users/Patrick/Downloads/Hydra/hydra-test); G/delete_rule.py:17; G/tests/test_ref_judge.py:686 (same project folder name).
**Why:** All typed roots and names agree. The ledger's path for common.py (H/tools/session_cards) is stale, since it is now under C:\Users\Patrick\.claude\tools. That is a path move, not a disagreement.

## Not reached
None. All 25 ids were checked.
