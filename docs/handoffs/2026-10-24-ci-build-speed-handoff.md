# Handoff: plan faster CI runs, 2026-10-24

Status, 2026-10-24: nothing is planned or built. The user asked for a handoff "so another session can plan these fixes". Your job is the plan, not the build. Write the plan, then wait for the user's yes before any executor starts (agent rule 3).

## Why this exists

A CI run on GitHub takes about 9 to 10 minutes from push to green. Most of that time is the compiler, not the tests. Every job compiles the whole project from scratch on a fresh machine, three jobs do it separately, and nothing is cached between runs.

This session (Claude Code session 1666228b-aad0-4c3d-ac42-f2b0839c95cb) first removed the slowest test. The PowerShell precheck self-test took 5 min 57 s on CI. It is now a C++ test inside `hydra_tests` that runs in seconds (see memory `precheck-in-cpp-2026-10.md` and `docs/superpowers/plans/2026-10-24-precheck-in-cpp.md`). The user then asked why the runs still take so long. This handoff carries that answer.

## Where the time goes, measured

Every time below comes from GitHub's step timestamps (`gh run view <id> --json jobs`). Run 40142020665 (main 194661c3) is the last finished run before the precheck change. Run 40151605824 (main 5f357402) is the first run after it. It was cancelled during its test step when another session pushed: ci.yml has `cancel-in-progress: true` on a per-branch group.

"Build and test" is the longest job, so it sets how long a run takes:

| Step | Run 40142020665 | Run 40151605824 |
|---|---|---|
| Check out the code | 9 s | 8 s |
| Build `hydra_tests` (`build_cpp.ps1 -Target hydra_tests`, cold) | 5 min 27 s | 6 min 44 s |
| Run `hydra_tests` (ctest, 4 slices with `-j 4`) | 55 s | cancelled |
| Build `hydra_uitest` | 19 s | not reached |
| Run `hydra_uitest --all` | 2 min 13 s | not reached |
| Job total | 9 min 9 s | cancelled |

The build grew by about 1 min 17 s between the two runs. That is the new `hydra_precheck` library and exe, and its test files, which `hydra_tests` now depends on.

The other jobs run at the same time, and each compiles again:

| Job | Its build | Rest | Job total (40142020665) |
|---|---|---|---|
| Coverage report | 5 min 42 s (coverage preset, both exes) | instruments the exes, runs both test sets again | 9 min 53 s, finished after Build and test |
| Fuzz corpus replay | 2 min 2 s (fuzz preset) | replays the saved corpus, 3 min 33 s | 6 min 2 s |
| Tool tests (Python) | none | Python tests | 6 min 42 s before the change, 35 s after (run 40151605824) |
| AddressSanitizer | — | skipped on these runs | — |

## What causes it, from the code

The build uses the Visual Studio generator. `CMakePresets.json` line 8 sets `"generator": "Visual Studio 18 2026"` for the default preset. The CI jobs run `build_cpp.ps1`, which uses those presets, on `windows-2025` hosted runners. I believe those have 4 cores, but I didn't check that.

`.github/workflows/ci.yml` has no compiler cache and no saved build folder. The only cache in it is the fuzz corpus (`actions/cache/restore@v4`). So every push recompiles every file, three times over: the default, coverage and fuzz presets.

Not measured yet: which files take longest to compile. Third-party code (`third_party/`: imgui, sqlite, opus, ogg, miniaudio, miniz, json, doctest, stb, mimalloc, rapidcheck) is compiled in every job too. How much of the 5 to 6 minutes it takes is unknown.

## Fixes to weigh in the plan

These are candidates, ranked by the likely gain. None is measured on CI.

1. **Cache compiled output between runs.** A push would then recompile only the files it changed. The usual tools (sccache, ccache) hook in through `CMAKE_<LANG>_COMPILER_LAUNCHER`. That works with the Ninja and Makefile generators, not the Visual Studio generator. So CI would need a Ninja preset, with MSVC from the developer shell; the fuzz job already sets that shell up. The plan must check whether local builds keep the Visual Studio generator, and whether two generators cause drift. Search for prior art first (memory rule: plans cite GitHub/Stack Overflow prior art), for example sccache with MSVC on GitHub Actions, and Mozilla's sccache action.
2. **Build the third-party libraries once and reuse them.** First measure how much of the build they take: do a timed local cold build with per-project times (MSBuild's `-clp:PerformanceSummary`, or a binary log). If they're small, drop this one.
3. **Run Coverage less often.** Run it on a schedule or on demand instead of on every push. It doesn't lengthen a run, because it runs in parallel. But it costs a whole second build and test pass, and on run 40142020665 it finished 44 s after Build and test. Ask the user before changing when it runs: it's a policy call, not a code change.
4. **Split the GUI tests.** `hydra_uitest --all` takes 2 min 13 s, one script after another. The unit tests already run as 4 parallel slices from `tests/shards.txt`. Check whether the GUI scripts can share a machine without fighting over windows or audio. The ci.yml comment near line 27 says hosted runners have no audio device.
5. **Rethink the cancel rule.** This is small but cost a real run today. `cancel-in-progress: true` on main means a run is thrown away whenever another session pushes within its 10 minutes, and several sessions push to main. That isn't a speed fix. But it decides whether a push ever gets its own green run. Ask the user.

## Rules that bind the plan

- **Ask before any full library run.** Nothing in this plan needs one (memory `no-full-library-runs.md`).
- **The plan goes through the doc-review gate.** It's a plan shown with ExitPlanMode.
- **Repo-wide tests at each merge.** A change to ci.yml or CMakePresets.json is a change to build files. So its merge runs the whole suite plus the three repo-wide tests (CLAUDE.md rule 1). The third repo-wide test is now `hydra_tests.exe "-sf=*test_precheck*"`.
- **Check CI once after the push.** A change to CI itself is the one case where the main session reads the CI run after pushing (agent rule 10). Read one run, not a poll loop.
- **Measure before you build.** Name what each measurement decides before running it (memory `measure-small-explain-first.md`).

## State at handoff

Main is at 18a2c64f on origin. That's this session's precheck work (pushed as 5f357402), plus another session's four "CH10-SHARED-MEASURES" commits on top. CI run 40152118997 for 18a2c64f was in progress when this was written. It is the first run that can show the C++ precheck self-test and the moved mutation probe step passing on CI. If it failed, look there first.

Journal state, from the hook at handoff time:

Subagents active in the last 30 minutes and not finished:
  agent-a6edf0bdfc161e7d1: unfinished, last tool call SubagentHandback, touched 21:40
  agent-ae3a14670af05ff45: unfinished, last tool call SubagentHandback, touched 21:38

Both of those handed back their final reports: the PCX-9 reviewer and the PCX-9 executor. Their work is merged and pushed, so neither is still working.
