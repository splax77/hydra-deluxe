Read docs/superpowers/plans/tasks/_perf-preamble.md first; it holds the rules.

# Task B1: build settings

Task id: B1. Base: H1's tip, which your prompt gives. Branch: `claude/perf-b1`, worktree `.claude\worktrees\perf-b1`; make it as the preamble says.

Your spec is the plan's "Task B1" section, word for word. This brief adds only what the plan leaves to dispatch time.

## What this brief adds

On the base, `build_cpp.ps1` builds its argument list at line 45 (`$buildArgs = @("--build", "--preset", $Preset, "--config", $Config)`); `--parallel` with no number goes into that list.

The plan's verify line starts with `-Configure`. Your first build is still a cold one, so make it through the slot helper as the preamble says, then reconfigure. For the "cold build before and after" times, the "before" time is your first slot build (say if the slot helper made you wait, and how long, so the number is the build alone). The "after" cold build means deleting your own worktree's `build-cpp` folder and building again; only your own worktree's build folder, through the slot helper.

Correctness compares against the baseline's saved `fresh.db`, `engine_rows.txt`, `parse_expert.tsv`; you only run your own side. If any differs with IPO on, IPO comes out, as the plan says.

The installer guard runs with `-SkipBuild` against your worktree's build. Inno Setup may be absent; the stage guards run before `ISCC`. Stop at the first step that needs Inno Setup and say how far it got.

## Owned files

The plan's B1 list: `build_cpp.ps1`, `CMakeLists.txt`, `installer/build_installer.ps1`, `docs/development.md`, `tests/test_store.cpp` (one appended case).

Owned-file check: every B1 acceptance criterion is met inside the files above; build timings and the scratch install prefix live in your scratchpad.

## Preflight

Command: `grep -n "\-\-build\|--config" build_cpp.ps1`, run by the orchestrator on the base's code on 2026-10-06.
Output: `45:    $buildArgs = @("--build", "--preset", $Preset, "--config", $Config)`. `git apply --check` of `patches/prof.patch` (the model for the Release-scoped lines) passes cleanly on this code.

## Return

`complete`, `branch`, `worktree`, `tip`, `ipo_kept` (true or false), `report`, `questions`, `handoff`.
