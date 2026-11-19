# TOOL-AUDIT-T5-W07 (second-round verifier, ledger L04, 21 rows)

All claims are by reading. Nothing was run. H = C:\Users\Patrick\Downloads\Hydra\hydra-test (rows read in the worktree tooling-audit-records unless a path says main), G = C:\Users\Patrick\.claude\gates, M = the memory folder C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory. Kinds K1 to K8 as in the finder brief. No row was REFUTED whole; L-F11-15 is partly refuted (two homes the finder named are gone). Verified rows I read only in part say so under Open.

## Counts
CONFIRMED 10 (L-F10-16, 17, 19; L-F11-09, 10, 11, 14; L-F12-15, 16, 18), CONFIRMED-CHANGED 10 (L-F10-15, 18, 20, 21, 22, 23; L-F11-13, 15, 16, 17), REFUTED 0 (L-F11-15 is partly refuted inside its CONFIRMED-CHANGED), not reached: L-F12-17. L-F10-23, L-F11-16, L-F11-17, L-F12-16 and L-F12-18 were checked only in part (see Open on each).

### L-F10-15
**Verdict:** CONFIRMED-CHANGED. The typed Python path is real, but counts and one side differ from the row.
**Homes as found:** G/publish.ps1:16 `$Python = 'C:\Users\Patrick\AppData\Local\Python\pythoncore-3.14-64\python.exe'`; G/install_ref_gate.ps1:10 same path as the `-Python` default; G/README.md:145 same path in a test command (lines 110 and 114 matched but the grep dropped their long text, so unread); G/settings.fragment.json has the path on nine `command` lines (97, 113, 129, 146, 162, 178, 196, 213, 229), not six; live C:\Users\Patrick\.claude\settings.json has nine matches (count only); G/tests/test_shell_output.py has one. H/docs/agents/this-machine.md:7 "Run Python as `py`. It starts the system Python 3.14.5 at `C:\Users\Patrick\AppData\Local\Python\pythoncore-3.14-64\python.exe`". H/docs/agents/derive-once-review.md has one more (count only). The Hydra tree also has one in H/.claude/settings.local.json and in old plans and a session card. this-machine.md:13: gh 2.97.0 at `C:\Program Files\GitHub CLI\gh.exe`, account `patseguin1-commits`. tools/ch_mcp/game_files.py:38 builds `LOCALLOW_DIR` from `~\AppData\LocalLow\srylain Inc_\Clone Hero`; this-machine.md:53 types it `AppData\locallow` (Windows ignores the case; text differs).
**Splits:**
- Interpreter: G/settings.fragment.json:6 allow rule runs bare `python.exe -I ...ruling_rules.py --check`. this-machine.md:9 says a bare `python` reaches `C:\Python314\python.exe` (3.14.7), a different install from the 3.14.5 the hooks name. The two differ whenever that allow rule is the command that actually runs. Whether the allow rule is a pattern only or a command that executes is a run question.
- The typed path in nine plus nine settings lines, publish.ps1 and install_ref_gate.ps1 each names the interpreter on its own; this-machine.md names it as the page of record. They differ if the interpreter moves.
- The Python path is held by tests only for fragment versus live settings (finder; I saw test_docs_numbers.py:140-146 check the fragment against live settings).
**Ruling:** none found in the files I read. Not searched in rulings folder for Python.
**Open:** gh scopes and Visual Studio build numbers: not read. Live version of Python: needs a run (`py --version`).

### L-F10-16
**Verdict:** CONFIRMED.
**Homes as found:** H/docs/agents/domain.md:7-9 "**`CONTEXT.md`** at the repo root, or", "**CONTEXT-MAP.md** at the repo root if it exists", "**`docs/adr/`**"; H/CLAUDE.md "Single-context: `CONTEXT.md` and `docs/adr/` at the repo root". Glob finds CONTEXT.md and docs/adr/0028-a-chart-id-is-clone-heros-song-id.md and no CONTEXT-MAP.md.
**Splits:** None between files. Mild: domain.md:7 says "or" between CONTEXT.md and CONTEXT-MAP.md while its own file-structure section (lines 15-39) treats them as single-context versus multi-context; the existing repo is single-context, so they agree today.
**Ruling:** none found.
**Open:** none.

### L-F10-17
**Verdict:** CONFIRMED.
**Homes as found:** H/docs/agents/doc-screenshots.md:29 "`tools/doc_shots/shots.txt` is the one list of the pictures... Nothing else in `docs/images/` should exist"; H/tools/doc_shots/shots.txt:1-3 "This file is the one list of those pictures"; screenshot lines at 43, 51, 63, 69, 77, 84, 92, 101 name main-screen, paths-tab, preview-tab, stars-tab, dynamics-tab, analysis-settings, search-tips, path-report (all `.png`). Glob of docs/images holds exactly those eight files.
**Splits:** None today (eight lines, eight files, same names). Nothing found that holds them equal in a test (not searched in tests; open).
**Ruling:** none found.
**Open:** a test tying docs/images to shots.txt: not searched. K8 candidate not borne out: the "one list" claim holds by reading.

### L-F10-18
**Verdict:** CONFIRMED-CHANGED. The resolution order agrees in all homes; the finder's capture_window.ps1 claim was misread.
**Homes as found:** H/tools/hydra_ui_mcp.py:55,56,72,140-141,147 (`DEFAULT_LIBRARY_DIR`, `HYDRA_LIBRARY_DIR`, `SNAPSHOT_FILES = ("hydra.db", "hydra_settings.ini", "hydra_ui.ini")`, "`library_dir` if given, else the HYDRA_LIBRARY_DIR variable, else the installed folder"); :766-768 same order; H/docs/agents/ui-testing.md:193 same order and same three names. H/tools/capture_window.ps1:5-10 says "They copy nothing themselves"; their `-Dir` "holding a copy of... hydra_settings.ini, hydra_ui.ini and hydra.db". So capture_window.ps1 copies nothing; the finder's "copies all three files" is wrong. ui-testing.md:350 case 2 copies the three files from `C:\Program Files\Hydra`; case 3 copies only `hydra_ui.ini` and writes a `hydra_settings.ini`.
**Splits:**
- ui-testing.md:350 (case 3 copies only the ui ini, writes its own settings) versus capture_window.ps1:8-10 (a scratch folder holds all three files, copy them again before each run): they differ in case 3 on what the scratch folder holds. The script header says "copy them again" and does not name case 3, so it is a K3 wording gap, not a conflict.
- The three file names are typed in hydra_ui_mcp.py:72, ui-testing.md:193,350 and capture_window.ps1:8-10; the file names are read in C++ in src/app/config.cpp (`db_path()` joins `exe_dir()` and "hydra.db" at :96-98; settings via `kSettingsFileName` at :103). No test holds the lists equal (not searched).
**Ruling:** none found.
**Open:** src/ui ini name not opened.

### L-F10-19
**Verdict:** CONFIRMED.
**Homes as found:** H/docs/agents/ui-testing.md:346-352 manual steps (copy Hydra.exe, mimalloc DLLs, assets, resource from `build-cpp\Release`; case 2 copies the three files "while the installed Hydra is closed"); ui-testing.md:353 "Window mode of the `hydra` server makes its own copy of the library, so use it when Claude drives the check"; H/tools/hydra_ui_mcp.py:59-62 `EXE_DIR = REPO / "build-cpp" / "Release"`, `WINDOW_EXE = "Hydra.exe"`, `BUILD_TARGETS`. this-machine.md:77 and ui-testing.md:346 both say Hydra reads `hydra.db` next to the exe (`db_path()`/`exe_dir` in config.cpp:31,96-98); config.cpp confirms.
**Splits:**
- Manual step 3 copies the plain files while the app is closed; the helper snapshots with SQLite online backup when a -wal sits beside it (same split as L-F10-01). They differ when the installed app is open. The manual step carries its own condition (closed), so it does not name `-wal`/`-shm`; this-machine.md:81 says to copy `-wal` and `-shm` whenever present. ui-testing.md:350 and this-machine.md:81 give two different copy recipes for the same file.
- Hydra.exe runs from a scratch copy in both paths, so the build folder typed in ui-testing.md:349 and hydra_ui_mcp.py:59 are two typed copies (see L-F11-10).
**Ruling:** none found.
**Open:** none.

### L-F10-20
**Verdict:** CONFIRMED-CHANGED. Path shape agrees in the three homes; the finder's claim about a non-matching folder is a main-versus-worktree difference.
**Homes as found:** H/docs/agents/handoff.md:7 "`docs/handoffs/<YYYY-MM-DD>-<slug>-handoff.md`"; H/.claude/commands/handoff.md:8 (slug rule "lowercase words joined by dashes, 2 to 6 words") and :11 `docs/handoffs/<today>-<slug>-handoff.md`; H/tools/check_handoff.py:29 `_DATE = re.compile(r"\d{4}-\d{2}-\d{2}")`, :398-403 `handoff_date` takes the first date found in the file name. The slug rule (2 to 6 words) is stated only in the command file; handoff.md says `<slug>` only. In the main checkout docs/handoffs holds eight flat `*-handoff.md` files and one folder `2026-10-25-test-audit` (holding report.md), which does not match the name shape; in this worktree only six flat files are present.
**Splits:**
- The command file states a slug rule (2 to 6 words) that handoff.md and the checker do not; the checker accepts any date-bearing name. They differ for a name like `2026-11-18-x-handoff.md` (one word is allowed by the checker, the command rule demands two).
- The test-audit folder holds a report, not a handoff, under a name with the same date prefix; the checker would take its date but handoff.md names only flat files.
**Ruling:** none found.
**Open:** the checker's filename filter (does it require the `-handoff.md` ending): not read.

### L-F10-21
**Verdict:** CONFIRMED-CHANGED. The figures are different measurements, not one contradictory number, but the note's headline numbers are not in the README.
**Homes as found:** M/video-decode-ceilings.md:11 "Decode-only ceilings here: h264 ~3,000 fps CPU, AV1 (dav1d) ~1,900 fps; NVDEC ~2,300-2,700 fps for both. Today's fcvideo main pass runs 2,200-2,700 fps" and :19 "(2,557 vs 2,566 fps)" with "Evidence tables live in the video-tools README, Decoder section". C:\Users\Patrick\Downloads\Hydra\video-tools\README.md (the README lives there, not under hydra-test): :522-527 table "CPU decode + crop 2,617 / 1,787; CPU `-threads 16` 2,678 / 1,834; NVDEC, full frame copied back 894 / 669; NVDEC with the decoder's `-crop` 2,798 / 2,122"; :582 "about 894 fps against 2,557 on the processor".
**Splits:**
- Note's "decode-only ceiling 3,000 and 1,900" appears nowhere in the README (grep of "fps" shows only lines 522, 582, 632, 686). The README measures decode plus crop, so the two answer different questions.
- Inside the README, h264 CPU is 2,617 (table :524) and 2,557 (:582): two figures for the same case in one file (different runs; the text does not say).
- Note says NVDEC "~2,300-2,700"; README table shows 2,798 with `-crop` and 894 without: not the same measurement.
**Ruling:** none found.
**Open:** whether :582 is a second run of the table's row: a read of README 575-585 settles it; not done.

### L-F10-22
**Verdict:** CONFIRMED-CHANGED. The unmet needs are real, but the finder put them in the wrong files and "repeated twice" is wrong.
**Homes as found:** C:\Users\Patrick\.claude\skills\reverse-engineer-anything\SKILL.md:131 `REA_TSHARK_COMMAND`, :147-148 "sigrok-cli 0.7.2 through `REA_SIGROK_COMMAND`", :153-154 "WABT 1.0.42 through `REA_WABT_BIN_DIRECTORY`", :183 "Node.js 24.x >=24.15 or 26+" (once). references/android-applications.md:17 "Java 17 or newer. `REA_JADX_MCP_JAR`", :27 `REA_JADX_HEAP_MIB`. references/native-and-artifacts.md has none of these (grep found none). H/docs/agents/this-machine.md grep for REA_, Java 17, sigrok, WABT, Node.js found nothing; its REA lines 23-25 give Ghidra 12.1.3, Temurin JDK 25, and the `rea-agents@6.5.0` start command.
**Splits:** none between files. The skill needs tool paths and versions (jadx jar, tshark, sigrok, WABT, Node) that the machine page does not hold. Java 25 on the page meets "Java 17 or newer". SKILL.md:131 and :147 say the sigrok and tshark paths need Linux with util-linux `prlimit`, so on this Windows machine those needs may never apply (reading only).
**Ruling:** none found.
**Open:** whether the skill asks the user for these (K4) was not read past the lines above; the skill's own text says "supply" them.

### L-F10-23
**Verdict:** CONFIRMED-CHANGED (items checked below; the rest unreached).
**Homes as found (checked):**
- H/docs/agents/ui-testing.md:294-296 versus the code: `register_paths_tests` at tests/ui/uitest_paths.cpp:1054; `register_report_window_tests` at uitest_report_windows.cpp:1634; `report_window_flow_tests()` at :1618; `register_tests` at tests/ui/uitest_tests.cpp:17. Names match. But ui-testing.md:294 says `report_window_flow_tests()` is the one "which `register_tests` calls last"; uitest_tests.cpp:34-35 call `register_entries(report_window_flow_tests())` and then `register_entries(command_tests())` (from uitest_live.cpp). So "last" is wrong by one call.
- M/dmleaderboards-comparison.md:13 `https://dmbot-kb5j.onrender.com/api` equals H/src/net/dmbot_client.h:30 `kDefaultApiBase = "https://dmbot-kb5j.onrender.com/api"`. Same string, two typed copies.
- M/one-reviewer-rule-d107.md:15 "the reviewer cap is 150 calls like everyone else" and "(fix-round.md retired)"; G/progress.py:14 `CALL_WARN, CALL_STOP = 100, 150`. Glob finds no fix-round file under H/docs. The cap in the note is a typed copy of progress.py; the 20-minute / 30-minute limit also appears in the note's line 11 (20 minutes) with the limit named MINUTES_STOP in progress.py:129 (value not read).
- M/fc-video-workflow.md:11 "eleven-step checklist": the README checklist (video-tools\README.md:37-66) holds items 0 to 10, which is eleven. Correct today; item numbers "0 and 5" in line 15 match README lines 41 and 56.
- Search lists: M/verify-before-claiming-a-source-is-wrong.md:36-37 lists "Get-Command, winget list, Get-AppxPackage"; H/docs/agents/this-machine.md:29 says only "Search the disk and the session cards before you say a tool is missing" (no list). So the finder's "two search lists" is one list and one general sentence.
- C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:49 "the tool is standalone by design" is a by-design claim with no ruling.
**Splits:** the typed cap 150 and URL are copies with no test tying them. The doc "calls last" claim differs from the code by `command_tests()`.
**Ruling:** none found.
**Open:** not reached: CLAUDE.md:27 versus the sessions-first note, `sourcetree::root()` owner claim (tests/scan.cpp etc.), `test_docs_match_code.cpp`.

### L-F11-09
**Verdict:** CONFIRMED.
**Homes as found:** G/tests/lib/harness.ps1:5-7 "This file is the one home of how a test runs a hook, reads its verdict, counts checks and prints its summary line. A test that needs something different adds its own helper beside these"; :210-238 `New-HooksCopy` (copies named hooks plus all of `lib\` into `hooks-copy-<guid>` with empty `state\`; refuses a temp folder inside the source). Users: tests/test_h52_cleanup_sweep.ps1:17,473; tests/test_h53_session_cards_start.ps1:19. Python tests copy on their own with `shutil`: tests/test_publish.py:205,257-258,313,408; test_ref_judge.py:67,449,718,726,874; test_delete_rule.py:380; test_review_dispatch.py:343. harness.ps1 runs hooks with `$script:HarnessShell = 'powershell.exe'` (:22), so it serves PowerShell hooks only.
**Splits:**
- harness.ps1 copy: all of `lib\` and an empty `state\`, refuses an in-source temp. Python copies: per-test file lists (`GATE_FILES` etc.), `lib\` and `state\` handled per test (test_publish.py:313 copies `state` in). They differ in which files reach the copy and whether a refusal exists.
- The header says "one home of how a test runs a hook", yet the Python tests run Python hooks without it. The header scope is PowerShell tests, so the claim holds only for them.
**Ruling:** none found.
**Open:** whether any Python copy refuses an in-source temp folder: not read.

### L-F11-10
**Verdict:** CONFIRMED (more typed homes than the finder listed).
**Homes as found:** H/CMakePresets.json:10 `"binaryDir": "${sourceDir}/build-cpp"` (preset `default`), :96-97 buildPreset `default`, `"configuration": "Release"`; the file also holds seven other binaryDirs (build-cpp-vs2022, build-ship, build-asan, build-asan-ninja, build-coverage, build-coverage-ninja, build-fuzz, build-mutation). H/tools/cmake_presets.ps1:1-2 "A preset's build folder and configuration, read from CMakePresets.json (their one owner). Shared by build_cpp.ps1 and installer\build_installer.ps1." Typed copies found: tools/doc_shots.ps1:8, hydra_ui_mcp.py:59, check_shards.ps1:17, ci_build_folder.ps1:6,7,30, fuzz_seeds.ps1:22, mutate/find.py:18,235,629, mutate/rewrite.py:516, ch_probe/prep_inputs.py:46, ch_probe/tests/test_prep_inputs.py:166 and test_tick_rate_vs_hydra.py:28, ch_mcp/quiet.py:56, capture_window.ps1:7; .github has 20 hits (ci.yml 18, fuzz.yml 1, warm-build-folder action.yml 1). prep_inputs.py:46, quiet.py:56, ci_build_folder.ps1:30 and fuzz_seeds.ps1:22 were not on the finder's list.
**Splits:** none today (all say build-cpp\Release). They differ if the default preset's folder or config changes: only build_cpp.ps1 and build_installer.ps1 read the preset.
**Ruling:** none found.
**Open:** none.

### L-F11-11
**Verdict:** CONFIRMED (stays as V11b found).
**Homes as found:** H/tools/ch_mcp/game_tools.py:47 `SCRATCH_PREFIX = "ch_live_"`, :599 `scratch_root or tempfile.gettempdir()`, :1402 `f"{SCRATCH_PREFIX}{os.getpid()}_{self._count}"`. A search of all of G for `ch_live_` and `hydra_live_` returns nothing, so the sweep's code does not name either prefix.
**Splits:** hydra_live_ folders under `Temp\claude` (swept by age per V11b) versus ch_live_ folders under `Temp` directly. They differ when a ch_live_ folder is left behind: nothing in G names it. Prose "a week" versus 7 days: agrees today.
**Ruling:** none found.
**Open:** whether anything removes ch_live_ folders: this search covers G only; the C++ side and game_tools.py's own cleanup (I saw no `rmtree`/`shutil` match on the lines my grep returned) were not read in full.

### L-F11-13
**Verdict:** CONFIRMED-CHANGED (line numbers shifted; claims hold).
**Homes as found:** Hydra root typed in G/delete_rule.py:17 (`WORKTREE_FOLDERS`), G/tests/replay.py:42, G/tests/test_docs_numbers.py:22 `DEFAULT_HYDRA_ROOT`, G/tests/test_delete_rule.py:29,32, test_media_rule.py:24,27,28. Session-card tools live in C:\Users\Patrick\.claude\tools\session_cards\common.py (the hooks repo, not Hydra's tools/): :33 `PROJECT_NAME = "C--Users-Patrick-Downloads-Hydra-hydra-test"`, :35 `DEFAULT_MEMORY` derived, :36 `HYDRA_REPO` derived from HOME. H/tools/check_pointers.py:30 `MEMORY_DEFAULT = r"C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory"` typed; H/tools/agent_cost.py:31 `DEFAULT_ROOT` builds the same folder name; G/tests/test_ref_judge.py:686 `CARDS_PROJECT` typed with the comment "the card tools' PROJECT_NAME". H/tools/check_pointers.py:34 `HOOK_FOLDERS = ("", "lib", "tests")`, :35 `SKIP_FOLDERS`, :42 `RULINGS_FOLDER = os.path.join("docs","agents","rulings")`; G/ref_judge.py:45 `RULINGS_DIR = "docs/agents/rulings/"` (the finder said :41), and G/ruling_rules.py:59 imports it (owner used).
**Splits:**
- Project folder name: owner session_cards/common.py:33; typed in check_pointers.py:30, agent_cost.py:31, test_ref_judge.py:686, test_delete_rule.py:25, test_media_rule.py:24.
- The rulings folder: ref_judge.py:45 (imported by ruling_rules.py) versus check_pointers.py:42 (own copy, written as a path join). Same words; two homes; no test found tying them.
**Ruling:** none found.
**Open:** the three doc lists, `AGENTS_DIR` pointer, settings.json:13-15 typed copies: not read.

### L-F11-14
**Verdict:** CONFIRMED.
**Homes as found:** G/common.py:10 `STATE_ROOT`, :120-129 `agents_dir`, `status_path` ("status" and "agents" joined under `STATE_ROOT`); G/status_append.ps1:11-14 `$StateRoot = 'C:\Users\Patrick\.claude\gates\state'`, `$StatusDir = Join-Path $StateRoot 'status'`, `$AgentsDir = Join-Path $StateRoot 'agents'`; G/cleanup_sweep.ps1:597-600 `Remove-OldStateEntries $root 'status'`, `'agents'`, `'posture'`; G/posture.py:22 joins `"posture"` under `common.STATE_ROOT`; G/posture.py:14-15 `POSTURE_PATH` beside the gates folder. G/sync_settings.py:36,39 `FRAGMENT` (derived from GATES_DIR) and `LIVE = r"C:\Users\Patrick\.claude\settings.json"` (typed); G/tests/test_docs_numbers.py:48 `SETTINGS = os.path.join(os.path.dirname(GATES_DIR), "settings.json")` (a second derivation of LIVE) while :49 imports `FRAGMENT` from sync_settings; test :143-146 skips when no settings.json exists. H/docs/agents/this-machine.md:71 names `C:\Users\Patrick\.claude\settings.json`.
**Splits:**
- Folder names "status", "agents", "posture" are each typed in at least two places (common.py/posture.py and status_append.ps1/cleanup_sweep.ps1). status_append.ps1:11 also types the whole state root. They differ if the layout moves; the ps1 files have an override variable for tests only (`HOOKS_TEST_STATE_ROOT`, status_append.ps1:12).
- live-settings path: sync_settings.py:39 types it absolute, the test derives it. They agree if the gates folder sits in `.claude`.
**Ruling:** none found.
**Open:** none.

### L-F11-15
**Verdict:** CONFIRMED-CHANGED (partly refuted). The media-guard half holds; the review_dispatch half names homes that are gone.
**Homes as found:** G/media_rule.py:6 `MEDIA_ROOT = r"C:\ytdl"  # the main session's free media folder (D189)`, :7 `TOOLS = {"ffmpeg", "yt-dlp", "yt_dlp"}`, :11 `TEMPLATE`; media_rule.py:1 says "called by percall.py for every shell call"; percall.py:4,149-151,166-167 call `media_rule.check` (main=False for agents, main=True for the main session). G/README.md:11,21 name `media_rule.py` ("is the media guard (D189, brought back by D273 in the Hydra repo's decision log)"); H/docs/agents/brief-preamble.md has no mention of media/ffmpeg/yt-dlp (count 0). `splax77/hydra-deluxe` is typed in H/CLAUDE.md, this-machine.md:15, M/hydra-deluxe-rename.md:11 and four times in G/tests/test_publish.py; H/docs/agents/issue-tracker.md:16 "Infer the repo from `git remote -v`". G/review_dispatch.py: no `MAX_BRIEF_FOLDERS`, no `judged_folder`, no "approved on 2026-10-24"; the file now reads two labelled brief lines (lines 3-26, 41-57) and refuses `isolation` (:161-162). A search of all of G for `JudgedFolder|judged_folder` finds nothing.
**Splits:**
- The finder's rows at review_dispatch.py:36 and :66-75, and the stale "Get-JudgedFolder" docstring in test_review_dispatch.py, no longer exist in G: that part is REFUTED (the file was rewritten; the finder's report was stale). I searched G for the names; I did not read test_review_dispatch.py:224-286.
- Media guard prose: README:21 names it; the preamble says nothing, so an agent reading only the preamble does not learn that ffmpeg/yt-dlp writes outside the scratch folder or C:\ytdl are judged. That is a K7 gap (a hook with prose in one home).
- Repo name typed in four homes; issue-tracker.md says infer it from the remote; the remote is splax77 (L-F10-03).
**Ruling:** none found. D189/D273 live in the decision log, not under `## Confirmed`; I did not open it.
**Open:** reviewer.md:23 text not read.

### L-F11-16
**Verdict:** CONFIRMED-CHANGED for the parts I read (line number shifted).
**Homes as found:** G/tests/test_ref_judge.py:24 `GATE_FILES = ("ref_gate.sh", "install_ref_gate.ps1", "ref_judge.py", "common.py")` (the finder said :21); used at :66-67 to copy into a temp folder. G/ref_judge.py imports `common` at :26 and imports `review_rules` lazily at :93 and :350 and `ruling_rules` at :389. G/shell_output.py:27 `FALLBACK_FOLDER = "tool-results"` and :85-96 `saved_path`; comment :95 "Claude Code's folder for <session id>.jsonl is <session id>\tool-results, next to it."
**Splits:** the test's file list omits review_rules.py and ruling_rules.py, which ref_judge.py loads at run time; tests that need them copy them separately (test_ref_judge.py:718, 874). So the list is hand-kept and differs from what the judge imports; a new import would not appear in it. Transcript-layout readers (replay_extract.py, session_cards/common.py, agent_cost.py, cleanup_sweep.ps1:60, hydra_ui_mcp.py:65): not read.
**Ruling:** none found.
**Open:** the transcript readers and the trim save path pin test: not read.

### L-F11-17
**Verdict:** CONFIRMED-CHANGED for the items read.
**Homes as found:** H/tools/mutate/run.py:69 `MUTANTS_FORMAT = 1  # the mutants.json format rewrite.py writes (its FORMAT)`; rewrite.py:86 `FORMAT = 1`; and a third `FORMAT = 1` at tools/mutate/find.py:77 (the spot format; find.py:23 "Kept stable: the rewriter reads it"). run.py:111-112 refuses a listing whose `format` differs; tests use `run.MUTANTS_FORMAT` (test_run.py:26, test_first_use.py:57). G/check_pointers (H/tools/check_pointers.py):34-35,42 as in L-F11-13. G has `refs/heads/main` typed as a literal in review_dispatch.py:86,116,137,144, publish_gate.py:47 and review_rules.py:3,420 beside ref_judge.py:37 `MAIN = "refs/heads/main"`. tools/ch_mcp/game_files.py:31 `GAME_DIR = r"C:\Clone Hero\1.1 june\Clone Hero"`, :32 `GAME_EXE` built from `ch_probe.constants.PROCESS_NAME` (an owner used), :36-38 Documents and LocalLow dirs. H/tools/build_slot.ps1:10,78 type `C:\Users\Patrick\.claude\hooks\state\build_slots` (see L-F11-01).
**Splits:**
- Format version: run.py:69 versus rewrite.py:86 versus find.py:77. A comment ties the first to the second; nothing in code does. They differ the day rewrite.py bumps FORMAT.
- The word "main" ref: ref_judge.py:37 `MAIN` versus literals in three other gate files.
**Ruling:** none found.
**Open:** not reached: game_tools.py:47-57 versus stall_harness.py:796,854, constants.py:270-273 process names, dump_build.py:9,20,25,55,57, launch_patch.json:7, build_slot.ps1:40-46 and new_worktree.ps1:8-16 single-home rows, check_pointers' "names retired things on purpose" claim.

### L-F12-15
**Verdict:** CONFIRMED.
**Homes as found:** G/bash_windows_rule.py:173 "A commit's own text does not count (the user's ruling), and it has no limit of its own." and :178 "A pair in double quotes passes: bash's own reading of it is usually what was meant (the user's ruling)." G/README.md:19 "The user ruled on 2026-10-31 that "trying to ban specific command patterns is a losing battle because there will always be another pattern nobody thought to check."" H/docs/agents/rulings/review-and-hooks.md: `## Confirmed` begins at line 7; line 11 is the 2026-10-31 Confirmed line "im not too concerned about the exact number as much as efficiency and functionality. if a hook is doing something unnecessary or contains a lot of arbitrary rules that needs to scrutinized" (session 6777b546); the "Exempt git commit" line is at :40 under `## Unconfirmed` (:37). A search of docs/agents/** finds "losing battle" nowhere; it is in H/docs/sessions/2026-10-31-6777b546.md:25 only.
**Splits:**
- bash_windows_rule.py:173 says a commit's text is "the user's ruling"; the only filed line for it is under Unconfirmed (review-and-hooks.md:40). So the code names a ruling that no Confirmed line carries.
- bash_windows_rule.py:178 "pair in double quotes passes (the user's ruling)": no Confirmed line found (grep of "ruling" in the file returned only these two; I did not search the rulings folder for the double-quote pass).
- README:19 quotes words that the Confirmed line 11 does not hold. Line 11 and README:19 both come from the same day and session but say different things.
**Ruling:** none of the Confirmed lines I read names the commit-text exemption, the double-quote pass or the "losing battle" wording. Quote of the Confirmed line is above.
**Open:** a card quote is not a filed ruling; whether other rulings files (not review-and-hooks.md) hold these: only that file was searched for the three phrases.

### L-F12-16
**Verdict:** CONFIRMED for the literal-path list; the rest unreached.
**Homes as found:** G/delete_rule.py:26 `NOT_LITERAL = ("$", "`", "%", "*", "?", ",", "..", '"', "'")` (nine entries); delete_rule.py:91 tests `any(bad in path for bad in NOT_LITERAL)`. G/bash_windows_rule.py:17 `PATHCONV_OFF = "MSYS_NO_PATHCONV=1"`. H/docs/agents/brief-preamble.md:43 lists "a variable, a wildcard or a `..`, a path with a quote left inside it, and a path with a space in it". The preamble does not name `%`, `?`, `,` or backtick; the code refuses them.
**Splits:** the preamble's list (variables, wildcard, `..`, quote, space) versus the code's nine strings: `%`, `?`, `,` and the backtick are refused by code and named in no prose I read. Space is refused by word splitting, not by NOT_LITERAL.
**Ruling:** none found.
**Open:** not reached: the `ref:.dotpath` letter rule, the five-shape list versus bash_windows_rule.py:211-228, the refusal texts, find tokens, media TOOLS prose copies.

### L-F12-17
**Verdict:** NOT REACHED beyond one reading: G/README.md was not opened for these rows and progress.py:80-86,111-128 and status_append.ps1:38 were not read by me.
**Homes as found:** none read.
**Splits:** none confirmed.
**Ruling:** none.
**Open:** whole row.

### L-F12-18
**Verdict:** CONFIRMED for the header only; the whole file stays unchecked.
**Homes as found:** H/tools/build_slot.ps1:10 "Each slot is a lock file, C:\Users\Patrick\.claude\hooks\state\build_slots\"; :16 `-NewPass -Session <session id> [-Holder <process id>]`; :22-24 `-Claim`, `-Hold`, `-Release`; :30 "-Claim is what build_cpp.ps1 uses"; :73 "the user's limit of three is in" (the line continues into a comment I did not read in full); :78 `$BuildSlotDefaultDir = 'C:\Users\Patrick\.claude\hooks\state\build_slots'`; :224 "Claims the pass in HYDRA_BUILD_PASS for this process"; :247 `Test-BuildPassClaimed`; :295-325 the two ways in. G/percall.py:19 and G/README.md:15 say build_slot.ps1 owns what a pass allows (text from the ledger; not re-read by me).
**Splits:** the default slot folder in build_slot.ps1:10,78 is under `hooks\state`, not the gates state root (the L-F11-01 split); the limit of three is typed in the script's comment, CLAUDE.md rule 14, agent-rules.md and brief-preamble.md:57 (preamble: "At most three builds run at once").
**Ruling:** none found.
**Open:** the script's body from :100 to :325 not read for ownership of every pass rule.

## Ids not reached or only partly reached
Not reached: L-F12-17 (whole). Partly: L-F10-23 (CLAUDE.md:27 note, sourcetree::root owner), L-F11-16 (transcript readers), L-F11-17 (nine of the fourteen sub-items), L-F12-16 (all but the literal-path list), L-F12-18 (header only).
