# This machine: where the tools are

This page says where the tools live on the user's Windows machine, and how to look inside Hydra's database here. Every path and version on it was checked on the machine on 2026-10-31, with `Test-Path`, a version call or a read of the file named. Things on a machine drift: an update moves a tool, or fixes a problem. If a line here disagrees with what you see, the machine wins. Fix the line in the same commit as the work that found it.

## Python

Run Python as `py`. It starts the system Python 3.14.5 at `C:\Users\Patrick\AppData\Local\Python\pythoncore-3.14-64\python.exe`, which the user named for general use. It has the `sqlite3` module (SQLite 3.50.4). The hooks' `publish.ps1` calls that same file by its full path.

A bare `python` reaches a different install. In both shells it runs `C:\Python314\python.exe`, which is Python 3.14.7. The Microsoft Store alias (`C:\Users\Patrick\AppData\Local\Microsoft\WindowsApps\python.exe`) comes after it on the PATH. Use `py`, so every script runs on the same Python.

## GitHub CLI

`gh` is version 2.97.0, at `C:\Program Files\GitHub CLI\gh.exe`. A bare `gh` finds it from both PowerShell and Bash. It is already signed in, as the account `patseguin1-commits`, with its token in the Windows keyring and the scopes gist, read:org, repo and workflow. It needs no install and no login.

The repo is `splax77/hydra-deluxe`. Text never goes to GitHub through a hand-typed `gh` command. It goes through `C:\Users\Patrick\.claude\gates\publish.ps1`, as the project CLAUDE.md's "Doc review" section says.

How a release is built, signed and uploaded is in `docs/development.md`, in its "Every release" sections. A release's tag and title are both `v` plus the version, such as `v2.14.0`, and it carries two files: the installer and its `.sig` signature. The version number is set in one place, the project line of `CMakeLists.txt`, which reads VERSION 2.14.0 today.

## Ghidra and REA

Ghidra 12.1.3 is at `C:\Users\Patrick\Downloads\ghidra_12.1.3_PUBLIC_20260817\ghidra_12.1.3_PUBLIC`. Its headless runner is `C:\Users\Patrick\Downloads\ghidra_12.1.3_PUBLIC_20260817\ghidra_12.1.3_PUBLIC\support\analyzeHeadless.bat`. Java for it is Eclipse Temurin JDK 25 (25.0.4.1), at `C:\Program Files\Eclipse Adoptium\jdk-25.0.4.101-hotspot\bin\java.exe`. No GHIDRA_INSTALL_DIR variable is set for the machine or the user.

REA is a reverse-engineering toolkit that Claude Code reaches as the MCP server `rea` (an MCP server is a helper program that hands an agent extra tools). It is registered for this project only, at Claude Code's local scope, in `C:\Users\Patrick\.claude.json`. It starts as `npx -y rea-agents@6.5.0 mcp`, with its own GHIDRA_INSTALL_DIR set to the Ghidra folder above. Its skill sits in `C:\Users\Patrick\.claude\skills\reverse-engineer-anything`.

Ghidra's full decompile of Clone Hero build 6142 lives outside the repo. `docs/clone-hero/code/README.md` names its path and says what it holds.

A tool that reports "not configured" says nothing about what is on disk. An unset variable was once read as "Ghidra is not installed", and the user was told so wrongly. Search the disk and the session cards before you say a tool is missing.

## Visual Studio and CMake

Three Visual Studio installs are on the machine. All three report themselves complete and launchable:

| Install | Version | Folder |
|---|---|---|
| Build Tools 2026 | 18.9.12120.119 | `C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools` |
| Community 2026 | 18.9.12112.369 | `C:\Program Files\Microsoft Visual Studio\18\Community` |
| Build Tools 2022 | 17.14.37614.0 | `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools` |

`cmake` is not on PowerShell's PATH. `build_cpp.ps1` and `installer\build_installer.ps1` both find it through `Find-CMake` in `tools\find_cmake.ps1`, which asks Visual Studio's `vswhere` and uses the CMake bundled with Visual Studio. Today that is the 2026 Build Tools' copy, and the `build-cpp` cache names the 2026 Build Tools as its generator instance.

In August 2026 the Community install was left in an incomplete state, most likely by an interrupted update. `vswhere -latest` then listed nothing, and CMake refused to configure with "instance is not known to the Visual Studio Installer, and no 'version=' field was given". `Find-CMake` falls back to `vswhere -all` for that case. If CMake refuses that way again, configure with `-DCMAKE_GENERATOR_INSTANCE="<install folder>,version=<its version>"`, taking both from the table above. The lasting fix is a repair in the Visual Studio Installer, which is the user's to run.

## The Opus model behind agents

The global `C:\Users\Patrick\.claude\settings.json` sets the ANTHROPIC_DEFAULT_OPUS_MODEL entry of its `env` block to `claude-opus-5-5`. That entry decides which model the `opus` alias runs, so it decides the model of every agent sent out on Opus. As the note records it, the user found Opus 5 a big regression from Opus 4.6 and had the entry pinned to Opus 4.6. On 2026-10-05 they judged Opus 5.5 a return to form and had it set to `claude-opus-5-5`. If agents seem worse after a model release, check this entry first. Do not assume the newest model is the one wanted.

## Looking at the database

### Which file

Hydra keeps its library in `hydra.db`, next to the program that opened it (`db_path()` in `src/app/config.cpp`). So there are two on this machine. The development build's is `C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra.db`. The installed app's is `C:\Program Files\Hydra\hydra.db`. A screen the user shows you, in a video or a report, may come from the installed app. Check which file before you compare a number with it.

### Read a copy, never the live file

Copy the database to your scratch folder and read the copy. The store runs SQLite in WAL mode (write-ahead logging: recent writes sit in a side file until SQLite folds them into the main one). So copy `hydra.db-wal` and `hydra.db-shm` along with `hydra.db` whenever they are there. The bare `hydra.db` alone can miss the newest writes.

Read the copy with `py` and its `sqlite3` module. If Python ever breaks, Windows has its own SQLite in `C:\Windows\System32\winsqlite3.dll`, which PowerShell can call through `Add-Type`. There is no `sqlite3` command-line tool here.

The tables today are `charts`, `meta`, `profiles`, `records` and `record_paths` (read from a copy of the development build's file). Their columns are defined once, in `src/store/record_store.cpp`: the `charts` and `meta` tables in `RecordStore::set_up_schema`, and the result tables in `record_tables_sql()`. Read the columns there, not from an old note. The record layout has changed more than once. One join has held through it: a result row's `song_id` is the chart's `md5` (the hyhash, the chart's id; `docs/adr/0028-a-chart-id-is-clone-heros-song-id.md`). A join on those two columns works on a copy of the development build's file.

### Re-analyze one chart without touching the library

`hydra_batch.exe --db <copy> --redo <folder holding one chart>` analyzes that chart again into the copy. It reads the same `hydra_settings.ini` as the app beside it (`load_tool_settings()`). That is the clean way to test what the analysis does on one chart.

A timing that depends on what the database already holds needs a database that holds rows. A fresh `--db` hides any cost that grows with the library. On 2026-10-05 a query that scanned every library row on each save made a whole-library run take 92.4 seconds on a copy of the real library, while the same build took 16.9 seconds on a fresh database. The `charts_by_md5` index fixed it (D76), and the run on the real copy fell to under 20 seconds. But a whole-library `--redo` on a copy of the real library is a full library run. It needs the user's `allow library run` first (CLAUDE.md rule 8). Try the chart corpus in `testdata/input` before you ask.

### Quote the numbers the user can see

The user checks every number in the app. A number read straight out of the database can be one the app never shows. It may belong to another path than the one on screen, or the Paths tab may hide it. So before you quote a stored number, say which path it comes from (the optimal path, a path within the limit, or the best all-0 path) and whether the app displays it. What the Paths tab shows is built by `build_path_buttons()` and `build_activations()` in `src/app/path_view.cpp`. To see it as the user does, read the app as text with the `hydra` MCP server or `hydra_uitest`; `docs/agents/ui-testing.md` says how.
