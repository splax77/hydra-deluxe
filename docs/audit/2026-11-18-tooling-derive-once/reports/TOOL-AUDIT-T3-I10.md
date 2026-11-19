# TOOL-AUDIT-T3-I10: inventory of tools/ scripts that agents or hooks run

Repo: C:\Users\Patrick\Downloads\Hydra\hydra-test (worktree tooling-audit-records), commit ad1a9717c18d7865ee78681bfecf1235240a0c92. All paths below are under tools/ unless absolute. Read-only; no file changed.

## Which scripts, and how found
I grepped CLAUDE.md, docs/agents/, .claude/, the global CLAUDE.md, the memory notes, ~/.claude/settings.json and the gates folder (*.py, *.ps1, README.md) for each file name in tools/. I also grepped docs/ (outside audit, sessions, superpowers), .github and CMakeLists.txt for the build-ish scripts.

Found and read in full:
- check_pointers.py: run by the SessionStart hook in ~/.claude/settings.json (~line 111); named in the gates README.md and one gates script.
- check_handoff.py: brief-preamble.md, docs/agents/handoff.md and .claude/commands/handoff.md tell agents to run it.
- doc_tokens.py: imported by both checkers (no doc names it; read because the checkers' values live there).
- build_slot.ps1: named by gates percall.py, shell_output.py and README.md; run by hydra_build_pass and build_cpp.ps1.
- hydra_ui_mcp.py: the `hydra` MCP server (docs/agents/ui-testing.md; gates review_rules.py and README name it).
- find_cmake.ps1 (docs/agents/this-machine.md), new_worktree.ps1 (no doc names it; it makes task worktrees), capture_window.ps1 (ui-testing.md), doc_shots.ps1 (doc-screenshots.md), fake_hydra_live.py (ui-testing.md; first 60 lines only).

Partly read (constants only, by grep): tools/precheck/*.cpp (named in CLAUDE.md, agent-rules, brief-preamble, derive-once-review).

NOT reached: tools/ch_mcp/ (about 40 files; clone-hero-mcp.md and CLAUDE.md name it), tools/ch_probe/, tools/capture/*.ps1 (shot, steps, report_shot), tools/doc_shots/ (shots.txt, shrink.py), fake_hydra_live.py from line 60, the rest of tools/precheck code, tools/replay*.cpp, tools/mutate, tools/test_*.

Found with no agent, brief or hook reference, so not inventoried: agent_cost.py, bench_run.ps1 and compare_db.py, sccache_start.ps1, ci_build_folder.ps1 (docs/development.md or CI only), check_shards.ps1 (CMakeLists), cmake_presets.ps1, effort_test_*, gen_section_names.py (a clone-hero page), single_owner_rows.ps1 (an ADR), release_*.ps1, fuzz_*.ps1. This shows only what those greps covered.

## Rows
Format: id | question | file:line | value | owner claim | who acts | who is asked

### check_pointers.py
I10-001 | Where are memory notes read from by default | check_pointers.py:30 | `r"C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory"` | none | SessionStart hook, main session | no one
I10-002 | Where is the hooks (gates) folder by default | check_pointers.py:31 | `r"C:\Users\Patrick\.claude\gates"` | none | same | no one
I10-003 | Which extensions make a bare name a hook name | check_pointers.py:32 | `HOOK_EXTENSIONS = (".py", ".ps1", ".sh")` | none | the checker | no one
I10-004 | Which gates subfolders hold hook scripts | check_pointers.py:33-34 | `HOOK_FOLDERS = ("", "lib", "tests")` ("Subfolders of the gates folder that hold hook scripts and their tests.") | none | the checker | no one
I10-005 | Which folders the hook-file index skips | check_pointers.py:35 | `SKIP_FOLDERS = {".git", "__pycache__", "state"}` | none | the checker | no one
I10-006 | Which line starts a memory note's card pointers (never checked) | check_pointers.py:36 | `SESSIONS_LINE = "Sessions:"` | none | the checker | no one (memory notes end with it)
I10-007 | How a doc keeps naming a file that left the repo | check_pointers.py:38 | regex for `` `git show <7-40 hex>:<path>` `` | none in code; comment line 161 says CLAUDE.md "Doc review" gives the form | the checker, doc authors | doc authors write the line
I10-008 | Which words send a git show to the hooks repo | check_pointers.py:40 | `HOOKS_REPO_WORDS = "hooks repo"` | none | the checker, doc authors | doc authors
I10-009 | Which pages quote the user word for word (pointers in quotes unchecked) | check_pointers.py:42 | `docs/agents/rulings` | none | the checker | no one
I10-010 | Which documents are read | check_pointers.py:63-79 | CLAUDE.md, every .md under docs/agents, top-level .md of the memory folder (archive not read: "retired notes, which name retired things on purpose") | "documents() says which files are read" (line 9) | the checker | no one
I10-011 | What counts as a pointer | check_pointers.py:111-134 | path shape minus: colon past the drive, leading `/` `\` `[`, "...", "##", first part .git, slash-alternatives, git refs, remote owner/name | "is_pointer() which names count" (line 10) | the checker | no one
I10-012 | When a pointer is broken, retired, or found elsewhere | check_pointers.py:136-229 | roots tried: doc folder, repo root, hooks root, plus gates+HOOK_FOLDERS for bare hook names; exact case; "retired" if under `<hooks root>\hooks` or name in old hooks folder; else "found at ..." / "missing" | "Pointers.judge() how one pointer is judged" (line 9) | the checker | no one
I10-013 | Exit codes and hook reply | check_pointers.py:14-17, 282-314 | exit 0 clean, 1 broken, 2 bad argument; `--session-start` always exits 0; text "Broken file pointers in the agent docs and memory notes ... Fix them alongside the user's request:" | none | SessionStart hook; main session | main session is asked to fix broken pointers
I10-014 | Output line format | check_pointers.py:13-14, 267 | `"file:line: pointer (missing | retired | found at <other place>)"` and `"pointers: N checked, M broken"` | none | main session | no one
I10-015 | How the hook is called and its limit | ~/.claude/settings.json ~106-115 | `python.exe -I -S ...\tools\check_pointers.py --session-start`, `"timeout": 15` | none | Claude Code SessionStart | no one

### check_handoff.py
I10-016 | Which git settings are pinned / dropped | check_handoff.py:25-27 | `-c core.quotePath=false -c log.showSignature=false -c core.useReplaceRefs=false`; drops GIT_DIR, GIT_WORK_TREE, GIT_INDEX_FILE, GIT_COMMON_DIR, GIT_PREFIX, GIT_REPLACE_REF_BASE | none here; check_pointers.py:28 says "its git() and Repo are the one home of the git calls" | the checker | no one
I10-017 | What is a full commit hash in a Claims row | check_handoff.py:28 | `^[0-9a-fA-F]{40}$` | none | the checker | handoff authors
I10-018 | The Claims table heading | check_handoff.py:31 | `"## Claims"` | none | the checker | handoff authors
I10-019 | Which claim kinds are manual | check_handoff.py:32 | `("test", "ruling", "state")` | none | checker, then the reading agent | handoff authors write them; reader checks by hand
I10-020 | Row statuses | check_handoff.py:33 | `("ok","missing","not-on-main","not-pushed","manual")` plus "unknown" | none | checker; reading agent must report every row not ok/manual (brief-preamble.md:97) | no one
I10-021 | Which refs mean on main / pushed | check_handoff.py:109, 362-364 | `refs/heads/main`, `refs/remotes/origin/main` | none | the checker | no one
I10-022 | How a handoff's date is found | check_handoff.py:398-413 | date in file name, else last git commit date, else today with a warning | none | the checker | no one
I10-023 | Exit codes | check_handoff.py:11-12 | 0 all passed, 1 a check failed, 2 bad argument | none | agents per brief-preamble.md | no one
I10-024 | Table-mode pass rule | check_handoff.py:375-393 | no rows = error and exit 1; passes only if every status other than ok/manual has count 0 | none | the checker | no one
I10-025 | Scrape-mode output titles | check_handoff.py:310 | `EXISTS`, `MISSING`, `CHANGED SINCE <date>` | none | reading agent (preamble:97 says to report MISSING / CHANGED SINCE lines) | no one
I10-026 | Docs excluded when proving a function exists | check_handoff.py:90-97 | excludes `*.md` and `docs/`; "a doc that mentions a function is no proof the function exists" | none | the checker | no one

### doc_tokens.py
I10-027 | Which extensions make a bare name a path | doc_tokens.py:30-35 | `KNOWN_EXTENSIONS`: .md .py .ps1 .psm1 .sh .bat .cmd .cpp .cc .c .h .hpp .inl .json .jsonl .txt .yml .yaml .toml .cmake .iss .js .ts .html .css .xml .csv .svg .png (.exe, .db, .ini "left out on purpose") | "This is the one tokeniser for the scripts that check documents against the repo" (line 3) | both checkers | no one
I10-028 | Which prefixes make a branch name | doc_tokens.py:37 | `BRANCH_PREFIXES = ("claude/", "hx-")` | none | both checkers | no one
I10-029 | Token shapes | doc_tokens.py:39-56 | hex 7-40; call regex; line ref `:\d+(-\d+)?` or `#L\d+`; placeholder chars `<>*?{}$%|`; hex after "session"/"agent" (+id) is an id; trailing chars stripped `.,;:)]}'"` | none | both checkers | no one
I10-030 | Bare-hex rule | doc_tokens.py:49-50, 146-149 | 7-40 lowercase hex holding a digit and a letter a-f | none | both checkers | no one

### build_slot.ps1
I10-031 | How many builds may run at once | build_slot.ps1:75 | `$BuildSlotCount = 3` | "This file is the one place that knows the slot rule, the pass rule and their numbers." (line 3); cites D49 and rulings/agents-and-workflow.md | every build; main session; hydra_build_pass | no one. Also stated in CLAUDE.md rule 14 and brief-preamble.md "Builds" (not read as inventory).
I10-032 | How often a waiting build re-checks | build_slot.ps1:76 | `$BuildSlotPollSeconds = 15` | same | build callers | no one
I10-033 | How often it says it is waiting | build_slot.ps1:77 | `$BuildSlotMessageSeconds = 60` | same | same | no one
I10-034 | Where slot locks live | build_slot.ps1:78, 10 | `'C:\Users\Patrick\.claude\hooks\state\build_slots'` (slot1.lock and up); override `-SlotDir` or `HYDRA_BUILD_SLOT_DIR` | same | build scripts, tests | no one. The gates folder is `.claude\gates`; this path is under `.claude\hooks` (see I10-012 old hooks folder).
I10-035 | How long a pass may sit unclaimed | build_slot.ps1:80 | `$BuildPassUnclaimedSeconds = 120` ("The user's answer in the same rulings page") | same | main session (pass reply says "runs out if nothing claims it in 120 s") | no one
I10-036 | Wait for the slot folder's turn | build_slot.ps1:84 | `$BuildSlotFolderWaitMs = 5000` | same | the script | no one
I10-037 | How long an empty lock counts as being written | build_slot.ps1:86 | `$BuildSlotYoungSeconds = 5` | same | the script | no one
I10-038 | What the refusal tells people | build_slot.ps1:87 | `'Agents never build: hand back, and the main session builds. A main session gets a pass from hydra_build_pass.'` | none | any agent or main session refused | no one. Same rule is in agent-rule 14 and brief-preamble "Builds".
I10-039 | When a run is under the gate | build_slot.ps1:90 | `$env:CLAUDECODE -eq '1'`; comment "the same test the git ref gate uses" | says it matches the ref gate's test (a second copy by its own words) | the script | no one
I10-040 | When a lock still counts | build_slot.ps1:141-146 | unclaimed pass counts until `expires`; else while holder process lives | "Test-BuildSlotLive says when one still counts" (line 11) | the script | no one
I10-041 | Lock file format | build_slot.ps1:116-120, 204, 239 | line 1 process id, line 2 what is building, then `pass=`, `session=`, `expires=` (ISO "o") | "Read-BuildSlot says what a lock holds" | the script; old scripts read line 1 | no one
I10-042 | Who may get a pass; one per session | build_slot.ps1:188-196 | session id must match `^\S+$`; refused if that session holds a live slot ("One build at a time per session."); refused if all 3 slots taken | none | main session via hydra_build_pass | caller supplies -Session (hydra_ui_mcp.py:722 reads CLAUDE_CODE_SESSION_ID)
I10-043 | Pass reply text | build_slot.ps1:205-208 | `pass: <token>`; "It holds build slot N of 3 and runs out if nothing claims it in 120 s. Build with:"; `$env:HYDRA_BUILD_PASS='<token>'; .\build_cpp.ps1 -Target hydra_tests` | none | main session | main session is told to build with that line
I10-044 | A pass serves one claim; nested builds ride the parent's claim | build_slot.ps1:32-35, 229-243 | rule in text | "Enter-BuildPass owns that rule" | build_cpp.ps1, Directory.Build.targets | no one
I10-045 | Env var that carries the pass | build_slot.ps1:21, 230 | `HYDRA_BUILD_PASS` | none | main session sets, build scripts read | main session sets it
I10-046 | Retry count releasing a lock | build_slot.ps1:288-291 | 10 tries, 200 ms apart | none | the script | no one
I10-047 | -DryRun needs -SlotDir | build_slot.ps1:330 | throws '-DryRun needs -SlotDir, so it never takes a real slot' | none | tests | no one
I10-048 | Build another worktree; exit codes | build_slot.ps1:40-46, 358 | runs `<worktree>\build_cpp.ps1`; exit 0, its code, or 1 on throw; "put old-release worktrees on a short path" (MSBuild 260 chars) | none | main session | no one
I10-049 | Check mode | build_slot.ps1:37-38, 324-328 | exit 0 when not gated or pass claimed; else "no claimed build pass for this build." exit 1; Directory.Build.targets calls it | none | Directory.Build.targets | no one

### hydra_ui_mcp.py
I10-050 | Reply wait margin | hydra_ui_mcp.py:42 | `REPLY_MARGIN_S = 15.0` | "This is the one home of these three numbers." (line 41) | helper | no one
I10-051 | hydra_start wait for hello | hydra_ui_mcp.py:43 | `HELLO_TIMEOUT_S = 60.0` | same | helper | no one
I10-052 | Longest reply handed to Claude | hydra_ui_mcp.py:44, 223, 236 | `REPLY_CAP_CHARS = 12_000`; note `[cut at 12,000 characters: N of M lines dropped; contains= narrows hydra_text and hydra_dump]` | same | every agent using hydra tools | no one
I10-053 | Timeout in force | hydra_ui_mcp.py:46-47 | not kept here; "its owner is CommandSession in tests/ui/uitest_commands.h" | names another owner | helper | no one
I10-054 | Crash log tail; pipe retry | hydra_ui_mcp.py:52-53 | `LOG_TAIL_LINES = 20`, `CONNECT_RETRY_S = 0.05` | "Plumbing granularity, not behaviour" | helper | no one
I10-055 | Where the user's library is read from | hydra_ui_mcp.py:55-56, 147 | `DEFAULT_LIBRARY_DIR = r"C:\Program Files\Hydra"`, env `HYDRA_LIBRARY_DIR`; argument, then env, then default | "snapshot_library, the only code that knows that folder's path" (line 17); "This is the only place that knows the library folder's path" (line 140) | hydra_start in window mode | callers may pass library_dir
I10-056 | Exe names and build folder | hydra_ui_mcp.py:59-62 | `build-cpp/Release`, `hydra_uitest.exe`, `Hydra.exe`, targets `hydra_uitest` / `Hydra` | none | helper; tells main session which target to build | reply asks the main session to build the target
I10-057 | Scratch folder | hydra_ui_mcp.py:65-67, 301-306 | `<temp>/claude/hydra_live_<pid>_<n>`; files `window.log`, `stdout.log`; comments name cleanup_sweep.ps1 ($ScratchRoot) and is_under_temp in tests/temp_util.h | names owners elsewhere | helper, cleanup sweep hook | no one
I10-058 | What a snapshot copies | hydra_ui_mcp.py:72, 116-134 | `("hydra.db","hydra_settings.ini","hydra_ui.ini")`; only hydra.db must exist; plain copy when no -wal, else SQLite backup | none | helper | no one
I10-059 | Message when no session | hydra_ui_mcp.py:76 | `NO_SESSION = "no session; call hydra_start"` | none | agents | no one
I10-060 | Real-app record file and fields | hydra_ui_mcp.py:78-81, 194-199 | `realapp.jsonl` in the git common dir; fields `kind:"realapp", commit, dirty, commands, shots, started, stopped`; "The hooks repo's ref gate reads it, so its field names are a contract" | names a reader elsewhere | ref gate (hooks repo); helper | no one
I10-061 | When a window session counts as a real-app check | hydra_ui_mcp.py:454-486, 505-524 | record only for window mode with a hello line and a clean exit; "A window that crashed on quit, or had to be killed, is not a check." | none | ref gate; main session | no one
I10-062 | What counts as a screenshot | hydra_ui_mcp.py:84, 656 | `SHOT_REPLY = "screenshot: "`; "Its owner is the screenshot verb in tests/ui/uitest_commands.cpp" | names owner elsewhere | helper | no one
I10-063 | Git call limit | hydra_ui_mcp.py:87 | `GIT_TIMEOUT_S = 60` | none | helper | no one
I10-064 | Build-pass tool inputs and limits | hydra_ui_mcp.py:88-92, 719-739 | env `CLAUDE_CODE_SESSION_ID`; `BUILD_PASS_TIMEOUT_S = 30`; runs `pwsh -NoProfile -File tools/build_slot.ps1 -NewPass -Session <id> -Holder <pid>` | "The rule is tools/build_slot.ps1's" (line 24); "tools/build_slot.ps1 owns the rule" (line 878) | main session | no one
I10-065 | Why the build-pass tool lives here | hydra_ui_mcp.py:22-24 | "a hook can then refuse that one tool to agents by its name" | none | a gates hook | no one
I10-066 | Window-session stop | hydra_ui_mcp.py:453-468 | send `quit`, wait 15 s (REPLY_MARGIN_S), else kill; `keep_window=True` leaves it open | none | agents | no one
I10-067 | One command is one line | hydra_ui_mcp.py:629-632 | refused: "one command is one line; this one holds a line break" | comment "Transport" | agents | no one
I10-068 | Script run stops at first failure | hydra_ui_mcp.py:689-710 | "stopped at line N of M" | none | agents | no one
I10-069 | Server instruction text | hydra_ui_mcp.py:753-760 | "Drive Hydra's GUI by widget label. Start with hydra_start('headless') for the test corpus, or hydra_start('window') ... Refs below the toolbar need **/ (the search box is **/##search). See docs/agents/ui-testing.md." | none | every agent | no one
I10-070 | The tool list and per-tool rules | hydra_ui_mcp.py:763-879 | hydra_start, hydra_stop, hydra_reset, hydra_open_song ("the way to open a song"), hydra_click, hydra_type, hydra_check, hydra_key, hydra_window, hydra_wait_idle, hydra_wait_text, hydra_text, hydra_state, hydra_dump, hydra_rect, hydra_screenshot, hydra_run, hydra_build_pass ("Main sessions only") | helper "owns ... the tool list" (line 15) | agents | no one
I10-071 | Scratch cleanup age | hydra_ui_mcp.py:598-601, 768-771 | "removes it once it has gone a week with no writes; $ScratchAgeDays in cleanup_sweep.ps1 owns that age" | names owner elsewhere | cleanup_sweep.ps1 hook | no one

### find_cmake.ps1, new_worktree.ps1
I10-072 | Where cmake is looked for | find_cmake.ps1:5-24 | PATH first; else vswhere under `${ProgramFiles(x86)}\Microsoft Visual Studio\Installer` (`-latest`, component `Microsoft.VisualStudio.Component.VC.Tools.x86.x64`, else `-all -prerelease`), then `Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`; else throws "cmake.exe not found ..." | "Shared by build_cpp.ps1 and installer\build_installer.ps1" | build scripts | no one. brief-preamble.md:31 separately says "`cmake` is not on PowerShell's PATH".
I10-073 | Where task worktrees go and are named | new_worktree.ps1:6, 29-30 | `<main checkout>\.claude\worktrees\<TaskId>`, branch `claude/<TaskId>` from `-Base` (default `main`); TaskId must match `^[A-Za-z0-9._-]+$` | "The main checkout owns .claude\worktrees" (line 25) | whoever makes worktrees | caller supplies TaskId and optional Base. The delete rule in brief-preamble.md names the same folders.
I10-074 | Worktree starts with no build folder | new_worktree.ps1:10-16 | reasons given (stale objects, path #defines, tracking-log surgery) | none | main session | no one
I10-075 | Who builds in the worktree | new_worktree.ps1:7-8 | "The main session builds there later, under a build pass (tools\build_slot.ps1 owns that rule)" | names build_slot.ps1 | main session | no one

### capture_window.ps1, doc_shots.ps1, fake_hydra_live.py
I10-076 | Capture waits | capture_window.ps1:56, 66 | `$Wait = 12` s after launch; `WaitForExit(10000)` then kill | none | tools/capture scripts | callers may pass -Wait
I10-077 | Capture format | capture_window.ps1:49, 93-107 | physical pixels (SetProcessDPIAware); PrintWindow flag 2; PNG; empty rect skipped; type name `HydraCaptureV1`: "Bump it whenever the C# block below changes" | none | capture scripts | whoever edits the C# block bumps the version
I10-078 | What a capture -Dir must hold | capture_window.ps1:6-10 | copy of build-cpp\Release\Hydra.exe, mimalloc.dll, mimalloc-redirect.dll, assets\, resource\, hydra_settings.ini, hydra_ui.ini, hydra.db; "A run saves over the ini files and db, so copy them again before each run" | none | the caller (agent / main session) | the caller copies those files
I10-079 | Doc picture inputs and outputs | doc_shots.ps1:2-11 | exe `build-cpp\Release\hydra_uitest.exe`; list `tools\doc_shots\shots.txt`; output `docs\images`; `tools\doc_shots\shrink.py`; "never names a library file or a settings file" | "docs\agents\doc-screenshots.md explains the whole step" | main session | no one
I10-080 | doc_shots refusals | doc_shots.ps1:13-15, 20-24, 40-43 | exe missing: "ask the main session to build the hydra_uitest target", exit 1; hydra_uitest nonzero: same code; no rect before a screenshot: exit 1 | none | main session | no one
I10-081 | Crop rule: rect line before a bar-less screenshot keeps the frame below that rect | doc_shots.ps1:27-47 | `--below <file>=<y+height>` from ` rect=\((-?\d+),(-?\d+) (\d+)x(\d+)\)` | "(the comment at the top of shots.txt)" | the script | no one
I10-082 | Stand-in's commands and caps | fake_hydra_live.py:9-17, 38 | `fake-hang`, `fake-exit <code> [dump]`, `fake-big` (50,000 characters), `fake-close`, `quit`; `HANG_CAP_S = 600`; hello timeout "Deliberately not the real exe's starting value" (value not read) | "It knows no command" | MCP helper tests | no one

### tools/precheck (constants only, read by grep)
I10-083 | When a range is too big to review in one read | precheck/items.h:45-50 | `kLargeRangeLines = 400` (D82 item 2, "a starting value to be tuned later") | "This is the one place it lives." | hydra_precheck.exe; reviewers (derive-once-review.md:21 says the precheck owns the threshold) | no one
I10-084 | Test-pick limits | precheck/test_pick.h:22-30 | `kCommonNameLimit = 30`, `kPickSizeLimit = 40`, `kHeaderIncluderLimit = 20` (D299, "the user chose") | "They live here and nowhere else." | main session (gets the test list from `hydra_precheck.exe --tests`) | no one
I10-085 | Repeat thresholds | precheck/check_5.cpp:22-28 | `kMinRepeatedString = 12`, `kMinRecordFields = 3`, `kMinSharedFields = 4` (D113 item 8) | "This is the one place they live." | precheck, reviewers | no one
I10-086 | Body floor, namespace look-back | precheck/definitions.h:15-25 | `kBodyTokenFloor = 30` (D49), `kNamespaceLookBack = 200` | none | precheck | no one
I10-087 | Number canon limits | precheck/decisions.cpp:23-31 | `kWholeNumberLimit = 1e15`, `kMaxHexDigits = 16`, `kBoldLevel = 99` | none | precheck | no one
I10-088 | Item sort order; which docs numbers are compared against | precheck/items.h:39; check_3.cpp:2 | "The table in ranks() in items.cpp owns" the order; "decisions.h owns the docs the numbers are compared" against | names owners | precheck | no one

## Cross-references inside my own files
- Three builds at once: I10-031 (and CLAUDE.md rule 14, brief-preamble "Builds", outside my files).
- Pass rules: I10-035, I10-042, I10-043 in build_slot; I10-064 in hydra_ui_mcp defers to them.
- "Under the gate" test: I10-039 claims to match the ref gate's.
- Worktree folder: I10-073.
- Git calls: I10-016 (check_handoff) and I10-012 (check_pointers calls it).
- Extension lists: I10-003 (hook names) and I10-027 (path names) are separate lists.
- Slot folder path I10-034 sits under `.claude\hooks`, gates path I10-002 under `.claude\gates`.

## Hook gap noticed
check_pointers.py reads only CLAUDE.md, docs/agents/*.md and the top of the memory folder, so a script named only in another doc is never checked. General note only.
