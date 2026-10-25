# Faster CI runs

Plan written 2026-10-24 by Claude Code session 66725311 (started from session 94686408). Its starting point is `docs/handoffs/2026-10-24-ci-build-speed-handoff.md`. Nothing here is built yet. Every task waits for the user's yes (CLAUDE.md rule 3), and two policy questions at the end need the user's answer first.

## The short version

A CI run takes about 9 minutes because the longest job does four slow things one after another. It compiles `hydra_tests` from scratch, runs it, compiles `hydra_uitest`, and then runs the GUI tests. The plan cuts that two ways.

First, the GUI tests move to their own job, so they run beside the unit tests instead of after them. Second, each run starts from the previous run's build folder, saved in GitHub's cache. A push then recompiles only the files it changed, using the same Visual Studio build developers use.

For a typical push, I expect the run to drop from about 9 min 10 s to about 3½ minutes. A push that edits a core header such as `timing.h` or `model.h` still recompiles almost everything; that kind of run drops to about 7½ minutes. These are estimates from local timings and earlier CI logs. The proving runs in the last step measure the real numbers before anything merges to main.

The handoff's lead candidate was a compiler cache (sccache) with the Ninja build tool. I measured it and dropped it. On this code, a cold Ninja build is about a third slower than today's Visual Studio build, so the pushes that miss the cache would get slower. Building the third-party libraries once is dropped too: they are about a tenth of the compile work and are never what the build waits on.

## What I measured

### CI step times

All CI times come from GitHub's step timestamps (`gh run view <id> --json jobs`) and the run logs (`gh run view <id> --log`).

Run 40142020665 (main 194661c3, the last finished green run) spent its "Build and test" job like this:

| Step | Time |
|---|---|
| Check out the code | 9 s |
| Build `hydra_tests` (cold, includes 33 s of CMake configure) | 5 min 27 s |
| Run `hydra_tests` (ctest, 4 slices side by side) | 55 s |
| Build `hydra_uitest` (only what `hydra_tests` didn't build) | 19 s |
| Run `hydra_uitest --all` (118 tests, 4 at a time) | 2 min 13 s |
| Job total | 9 min 9 s |

The build log shows where the 5 min 27 s goes. Configure ended at 23:16:48. Every library, third-party and Hydra's own, was done by 23:18:31. `hydra_tests.exe` alone then took until 23:21:41. So 3 min 10 s of the step is the one test target, whose 124 test files can't start compiling until `hydra_ui.lib` is built.

The build step's time also swings a lot between runs. It took 5 min 27 s on run 40142020665, 6 min 44 s on run 40151605824, and 9 min 4 s on run 40152118997. Only part of that is new code (the `hydra_precheck` target, about 1 min 17 s per the handoff). The rest is runner speed. So every estimate below is a typical value, and a slow runner can stretch it by half again.

The hosted `windows-2025` runner has 4 CPUs and 16 GB for a public repository ([GitHub's runner reference](https://docs.github.com/en/actions/reference/runners/github-hosted-runners)). `splax77/hydra-deluxe` is public (`gh repo view --json visibility`).

### Local cold builds on 4 CPUs

I built `hydra_tests` from a clean copy of commit 44a8c082 (`git archive`) with each process pinned to 4 logical CPUs (`start /affinity F`), to match the runner. This machine's CPUs are faster than the runner's. On run 40142020665, CI took about 1.5 times as long as these local numbers (294 s of compile and link against 193 s), so I scale by 1.5 below. The scripts (`run.ps1` to `run4.ps1`, `push_misses.py`) and the raw times (`times.txt`) are in `C:\Users\Patrick\AppData\Roaming\Claude\side-session-notes\local_94686408-b50b-4d28-a8bf-a2ceecf5eeb1\ci-build-speed-measurements\`.

| Build | Cold `hydra_tests` |
|---|---|
| Visual Studio generator, `/Zi` (today's CI) | 193 s |
| Visual Studio generator, `/Z7` | 263 s |
| Ninja, `/Zi` | 312 s |
| Ninja Multi-Config, `/Z7` (what sccache needs) | 257 s |

`/Zi` writes debug info into one shared symbol file per target, through a helper process (`mspdbsrv`). `/Z7` keeps it inside each object file. sccache can only cache `/Z7` objects: its MSVC argument parser refuses a shared `.pdb` (`src/compiler/msvc.rs`, "shared pdb"), and its docs say to use embedded debug info.

Ninja did more total work: 1,048 CPU-seconds of compiling at `/Z7` and 1,274 at `/Zi`, read from its `.ninja_log`. The Visual Studio build used at most 772 (193 s on 4 CPUs). The likely reason is that MSBuild's `/MP` hands one compiler process a batch of files, so start-up and header loading are paid once per batch. Ninja starts one compiler per file. This goes against the usual report that Ninja is faster on Windows ([CMake discourse on batching cl](https://discourse.cmake.org/t/using-cmake-ninja-to-batch-cl-invocations/1853), [cmake-developers numbers](https://cmake.org/pipermail/cmake-developers/2012-February/015046.html)). Those reports gain mostly from Ninja's better scheduling across many projects. Here one target holds most of the work, so better scheduling gains little.

Third-party code is small. In the Ninja log it adds up to about 125 of 1,274 compile seconds: RapidCheck 31, Opus 30, SQLite 21, imgui 19, mimalloc 13, the rest a few each. In the Visual Studio build every third-party project was done 13.5 s into the build, while Hydra's own libraries were still compiling. Prebuilding them would save almost nothing, so that candidate is out.

### What a saved build folder would recompile

`ninja -t deps` lists, for every object file, the headers it read. I matched that list against the files changed by each of the last 40 commits on main's first-parent line, up to 44a8c082. The script is `push_misses.py` in the same folder. For each commit it gives the objects that change would recompile, and their compile time from the cold Ninja log.

Half of the 40 commits would recompile at most 7.4 CPU-seconds' worth of code. Many recompile nothing, because they only touch docs or plans. Six of the 40 (15%) recompile 100 or more of the 481 objects. Each of those six touched a widely included header: `timing.h` (166 objects), `model.h` (165), `song.h` (162), `squeeze_rating.h` (106) and `config.h` (101). For those, a saved folder saves little.

### A saved build folder, tested locally

Building on a saved folder only works if unchanged files look older than their compiled objects. A fresh checkout gives every file the current time. So the restore step gives every tracked file one fixed old time (2000-01-01), then gives "now" to just the files changed since the saved build's commit. I tested that on a Visual Studio build folder:

| After the restore | Build `hydra_tests` + `hydra_uitest` |
|---|---|
| Nothing changed (first build after the restore) | 45.5 s |
| Nothing changed (second and third build) | 7.4 s, 6.0 s |
| One test file changed | 7.8 s |
| `squeeze_rating.h` changed (106 objects) | 154.5 s |
| `timing.h` changed (166 objects) | 209.9 s |
| Cold, for comparison (this folder) | 238.0 s |

Setting every file's time took 0.4 s. The saved folder is 1,174 MB, and 214 MB once packed with zstd (12 s to pack). GitHub's cache uses the same tar-and-zstd packing.

My first run of this test was wrong, and the mistake matters for anyone repeating it. I first built in the Windows temp folder. MSBuild's file tracker ignores everything there (warning MSB8029), so header changes did not trigger rebuilds: `timing.h` "rebuilt" in 4.2 s. I repeated the test in a git-ignored folder under the repo, which gave the numbers in the table above. On CI the workspace is `D:\a\hydra-deluxe\hydra-deluxe`, outside the temp folder.

I don't know why the first no-change build took 45 s. Two likely causes are antivirus scanning the freshly written outputs, or MSBuild's first glob check. The estimates below budget for it.

## Prior art

Compiler caches can't be used with the Visual Studio generator. CMake's compiler-launcher setting works only with Ninja and Makefiles ([CMake discourse thread](https://discourse.cmake.org/t/using-a-compiler-cache-on-windows-with-msvc/13694)). Blender's Windows build refuses sccache unless it uses Ninja ([Blender's configure_msbuild.cmd](https://projects.blender.org/archive/blender-archive/src/commit/b9e1cc931ee901b153a0c3bb59fa36df6bc7b12d/build_files/windows/configure_msbuild.cmd)). OpenMW works around it by swapping a fake `cl.exe` into MSBuild ([OpenMW commit](https://git.cijber.net/mirror/openmw/commit/240e2486ed5f9bc885d5d1150ff29fdd4a782ba8)). That trick also turns off `/MP` batching, which is where this build's speed comes from.

sccache with MSVC needs embedded debug info, `CMAKE_MSVC_DEBUG_INFORMATION_FORMAT=Embedded` or `/Z7` ([sccache README](https://docs.rs/crate/sccache/0.17.0)). Mozilla's action stores the cache in GitHub's cache service ([sccache-action](https://github.com/Mozilla-Actions/sccache-action)). That backend makes one cache call per object, and GitHub's rate limit turns some of those calls into misses or dropped saves ([Depot on sccache in Actions](https://depot.dev/blog/sccache-in-github-actions), [sccache GHA docs](https://android.googlesource.com/toolchain/sccache/+/HEAD/docs/GHA.md)). ccache supports MSVC from 4.6, but users report it saving nothing under CMake with MSVC on Actions ([ccache-action issue 112](https://github.com/hendrikmuhs/ccache-action/issues/112)).

Saving a build folder in a CI cache depends on file times, and git doesn't store them. Two known fixes are `git-restore-mtime`, which sets each file to its last commit time ([write-up](https://zenn.dev/kt3k/articles/d557cc874961ab)), and `mtime_cache`, which restores times by content hash ([mtime_cache](https://github.com/iboB/mtime_cache)). Commit times aren't safe here: a commit made before the previous CI run finished would look older than that run's objects and be skipped. So the plan uses its own rule. Every file gets one old time, and only files changed since the saved build's commit get "now".

GitHub's cache rules decide the rest ([dependency caching reference](https://docs.github.com/en/actions/reference/workflows-and-actions/dependency-caching)). A repository gets 10 GB. Entries unused for 7 days are removed, and above 10 GB the least recently used go first. Runs on main read main's caches. Pull request runs also read their base branch's caches, and what a pull request saves is visible only to that pull request.

## Why not sccache, in one paragraph

sccache is the safer cache in one way: it keys each object on its preprocessed content, so it can't reuse a stale object. But it needs Ninja and `/Z7`, and that combination built `hydra_tests` cold in 257 s against 193 s today. About one push in seven edits a core header and recompiles most of the code, and those pushes would get about a third slower. It would also put CI on a different generator from every developer's build, and this project has generator-specific workarounds in `CMakeLists.txt`: the `%` doubling (CMake issue 15865) and the `/LTCG` note (CMake issue 20484). The saved folder keeps the exact local build, and the staleness risk is covered by the safeguards below.

## The design

### The saved build folder

A new script, `tools/ci_build_folder.ps1`, does two jobs. Before the build, `-Prepare` reads the commit the saved folder was built from (a small file, `build-cpp\.ci-built-commit`). After the build, `-Record` writes that file.

`-Prepare` decides between using the saved folder and starting cold:

- With no saved folder, or no record in it, there is nothing to do: the build is cold, as today.
- If the recorded commit isn't an ancestor of the commit being built (a force push or a rewritten branch), the script deletes the folder and the build is cold.
- If the push changes a build file, the script deletes the folder and the build is cold. The build files are `CMakeLists.txt`, `CMakePresets.json`, `build_cpp.ps1`, `tools/find_cmake.ps1`, `tools/cmake_presets.ps1` and anything under `third_party/`. Regenerating projects in place is MSBuild's riskiest incremental path, and these files change rarely, so a cold build costs little.
- Otherwise it sets every tracked file to 2000-01-01, then sets the files changed between the recorded commit and the current one to now.

In every case it prints one line saying which path it took and why.

The cache key holds the build folder's name, the runner image version (the `ImageVersion` variable hosted runners set) and the run id. A run restores the newest key with its folder name and image version. A new runner image ships a new compiler, so it starts cold instead of mixing objects from two compilers.

Restore and prepare become one small composite action, `.github/actions/warm-build-folder/action.yml`, so the build job and the GUI job share one copy of those steps.

### Saving and keeping the cache small

Only the "Build and test" job saves, and only on a push to main or a manual run. It saves right after its build, before the tests, so a failing test still leaves a warm folder for the fix. Pull request runs only restore.

Each save is about 214 MB. Main gets dozens of pushes a day, and GitHub only evicts above 10 GB. Left alone, old build folders would fill the cache within a day. Eviction would then remove the weekly fuzz corpus caches, which `fuzz.yml` reads only on Mondays. So after saving, the job deletes all but the two newest build-folder caches with `gh cache delete`. That needs `actions: write` on this job only. The two kept entries hold about 430 MB, and the fuzz caches stay safe.

### The safety net for a stale folder

The nightly scheduled run builds "Build and test" and the GUI tests cold, with no restore. Today "Build and test" skips the schedule (`if: github.event_name != 'schedule'`), so this adds one cold run a night. If a saved folder ever hid a broken build, the nightly run fails within a day. A manual run is cold too, which gives a one-click check on any branch.

### The GUI tests get their own job

A new job, "GUI tests (Windows, Release)", checks out the code, restores the saved folder, builds `hydra_uitest` and runs `hydra_uitest --all`. "Build and test" stops running the GUI tests. It still builds `hydra_uitest` in the same build step as `hydra_tests`, so the folder it saves holds everything the GUI job needs next time. That costs 11 to 19 s.

The GUI runner already runs 4 tests at once (`kDefaultJobs = 4`, `tests/ui/uitest_main.cpp` line 255), so splitting it further inside one machine gains nothing. Moving it to its own machine takes its 2 min 13 s off the longest job. Each job gets its own VM, so the GUI tests and unit tests can't fight over windows or sound.

`build_cpp.ps1 -Target` will take several target names, so one call builds both exes. Today the build job calls it twice, and each call re-checks every project.

## Expected CI time per job

These are estimates. Each line says how it was made. "Local × 1.5" means a local 4-CPU timing scaled by the CI-to-local ratio from run 40142020665 (294 s on CI against 193 s locally).

The restore time is the biggest unknown. 214 MB downloads in seconds, but unpacking 1,174 MB of small files on a Windows runner could take 20 to 60 s. I budget 45 s, and the first proving run measures it.

**A typical push** (one of the half of commits that recompile under 8 CPU-seconds):

| Job | Today | After | How the "after" was made |
|---|---|---|---|
| Build and test | 9 min 9 s | about 3 min | checkout 10 s (run 40142020665), restore 45 s (budget), prepare under 5 s (0.4 s locally), build 12 to 70 s (7.8 s to 45.5 s local × 1.5), save 30 s (12 s pack × 1.5, plus upload), unit tests 55 s (run 40142020665), mutation probe self-test (not yet timed on CI: it was skipped on the two runs since it moved) |
| GUI tests | (inside the job above) | about 3½ min | checkout 10 s, restore 45 s, build about 15 to 70 s (same scaling), GUI tests 2 min 13 s (run 40142020665) |
| Fuzz corpus replay | 3 min 44 s | unchanged | run 40152118997 |
| Tool tests | 30 s | unchanged | run 40152118997 |
| Whole run | about 9 min 10 s | **about 3½ min** | the longest job |

**A push that edits a core header** (about 15% of the last 40 commits):

| Job | After | How it was made |
|---|---|---|
| Build and test | about 7½ min | the `timing.h` rebuild, 210 s local × 1.5 = 5 min 15 s, plus checkout, restore, save and unit tests as above |
| GUI tests | about 4 min | the GUI exe's share of the rebuild (`hydra_core`, `hydra_ui` and the harness, about 120 CPU-seconds in the Ninja log, about a minute on CI), plus the GUI tests |
| Whole run | **about 7½ min** | today: about 9 min 10 s |

**A cold run** (the nightly run, a new runner image, or a push that changes a build file):

| Job | After | How it was made |
|---|---|---|
| Build and test | about 7 min 20 s | today's two build steps (5 min 27 s + 19 s), save 30 s, unit tests 55 s, plus the probe |
| GUI tests | about 5 min | the cold build up to `hydra_ui.lib` took 2 min 17 s on run 40142020665; plus about 25 s for the harness and the exe (my guess), plus the GUI tests |
| Whole run | **about 7 min 20 s** | today: about 9 min 10 s |

The Coverage job isn't in these tables. It runs beside the others and doesn't set the run's length. It does cost a second full build and test pass on every push to main: 9 min 53 s on run 40142020665. Question 1 below covers it.

## Tasks

Each task is one executor (Opus, in its own worktree, under 100 tool calls and 20 minutes). Tasks 1 and 2 run in parallel. Their changes to `ci.yml` sit in separate blocks, and the owned lines are spelled out below.

### Task 1 (CI-WARM): the saved build folder

This task writes `tools/ci_build_folder.ps1` with `-Prepare` and `-Record`, exactly as described under "The saved build folder". It also writes a self-test, `tools/test_ci_build_folder.ps1`. The self-test builds a small git repo and a fake build folder in a temp folder, then checks four cases: no record leaves the folder alone; an ancestor commit with one changed file gives that file "now" and every other file 2000-01-01; a range that changes `CMakeLists.txt` removes the folder; a recorded commit that isn't an ancestor removes the folder.

The task also writes the composite action `.github/actions/warm-build-folder/action.yml`. Its inputs are the build folder name, and whether to restore at all, which is off for the nightly and manual runs.

It adds a `warm` input to the workflow's manual trigger (`workflow_dispatch`). It is off by default, so a manual run is cold unless someone asks for a warm one.

It changes `build_cpp.ps1` so `-Target` takes several names.

In `ci.yml` it owns these parts of "Build and test": the checkout options, the new restore step, one build step for both exes, the record, save and prune steps, `permissions: actions: write` for this job, and the job's `if:` so it also runs on the nightly schedule. It doesn't touch the "Run hydra_uitest" step; task 2 removes that.

Finally, it updates the "Continuous integration" section of `docs/development.md`: how the saved folder works, when a build is cold, and how to force a cold run.

Owns: `tools/ci_build_folder.ps1`, `tools/test_ci_build_folder.ps1`, `.github/actions/warm-build-folder/action.yml`, `build_cpp.ps1`, the parts of `ci.yml` named above, and the saved-folder paragraph of `docs/development.md`. Tests: `pwsh -NoProfile -File tools/test_ci_build_folder.ps1`, plus one local `.\build_cpp.ps1 -Target hydra_tests,hydra_uitest` to show that several targets work. About 60 calls.

### Task 2 (CI-JOBS): the GUI tests' own job, and the two policy answers

This task adds the job "GUI tests (Windows, Release)" to `ci.yml`. Its steps are: check out with `fetch-depth: 0` (the prepare step needs history back to the saved build's commit), `uses: ./.github/actions/warm-build-folder`, `.\build_cpp.ps1 -Target hydra_uitest`, then `.\build-cpp\Release\hydra_uitest.exe --all`. The job runs wherever "Build and test" runs, nightly included. It deletes the "Run hydra_uitest" step from "Build and test".

It applies the user's answers to questions 1 and 2: the coverage job's `if:`, and the `concurrency` block with its comment. It updates the matching sentences in `docs/development.md`: the job list, the coverage paragraph and the cancel sentence.

The action path is fixed by this plan, so task 2 can write the `uses:` line before task 1's action exists. Task 2 merges after task 1.

Owns: the new job, the deleted step, the coverage `if:`, the `concurrency` block and those sentences of `docs/development.md`. Tests: none run locally beyond the docs-vs-code case, `hydra_tests.exe "-tc=docs match code*"`. The proving runs below test the workflow. About 30 calls.

### After both tasks: the merge and the proving runs (main session)

Each task gets its own fresh derive-once reviewer. The reviewers also review the `docs/development.md` text with `doc-review.md`. Task 1 merges into a join branch, `ci-speed`, cut from main. Then task 2 merges into it.

Both tasks change build files, so the join runs the whole `hydra_tests` suite plus the three repo-wide tests: docs-vs-code, the single-owner scan and the precheck self-test (CLAUDE.md rule 1). It also runs `tools/test_ci_build_folder.ps1`.

Then the proving runs. With the user's yes to this plan, which includes these pushes:

1. Push `ci-speed` to origin and start the workflow on it by hand (`gh workflow run ci.yml --ref ci-speed`). A manual run is cold by design, so this run measures the cold path and saves the first folder.
2. Push one commit to `ci-speed` that changes a single test file, and start a run with the `warm` input on (task 1 adds it). This run measures restore time, the warm build and both jobs' totals.

I read each run once (agent rule 10's exception for CI changes), with no polling loop. Then I write the measured times into this plan's results section.

If the warm "Build and test" job takes over 5 minutes, or the restore over 90 s, I bring the numbers back to the user instead of merging. Otherwise `ci-speed` merges to main, is pushed, and the remote branch is deleted.

## Questions for the user

Answered 2026-10-24. The user's words: "build with recommended". So Coverage runs nightly and on any manual run, and a newer push to main never cancels a run in progress; pull requests still cancel.

**Question 1: when should the Coverage job run?** Today it runs on every push to main. It never fails a run over a low number, and it isn't part of the run's length. But each push pays a second full build and both test sets again: 9 min 9 s to 9 min 53 s of runner time (runs 40142020665 and 40152118997). When a test fails, it fails a second time in the coverage job too, as it did on run 40152118997. The options:

- Nightly, plus any manual run. You get a fresh report every morning, without a second build on each push. (Recommended.)
- Every push to main, as now. The saved build folder would then need a third cache entry for the coverage build.
- Manual runs only.

**Question 2: should a newer push to main cancel the run in progress?** Today it does (`cancel-in-progress: true` on a group per branch). Several sessions push to main within minutes of each other, so a push often never gets its own finished run. Run 40151605824 was cancelled that way during its tests. The options:

- Never cancel on main; pull requests still cancel. Each push to main gets a finished run, and a red run names the push that broke it. On main the group gets the commit id. Otherwise GitHub still replaces a queued run with a newer one, even with cancelling turned off. Hosted runners are free for a public repository, so the cost is only queueing when many pushes land at once. (Recommended.)
- Keep cancelling, as now. CI then always shows the newest code, but a failure can't always be traced to one push.

## Out of scope, noticed while planning

Main is red right now. `hydra_tests_rest` fails on runs 40152118997 (18a2c64f) and 40152863600 (775ada53), at `tests/test_preview_load_progress.cpp` line 436, in "a Preview read under the 1.0 fills rule draws the 1.0 generator's fills" (`CHECK( ch10 != fill_spans(read_scene(hydra::FillDeadlineRule::Ch11)) )`). It started with the CH10-SHARED-MEASURES merge. This plan doesn't touch it; it's for the session that owns that work.

Five headers (`timing.h`, `model.h`, `song.h`, `squeeze_rating.h`, `config.h`) each pull in 100 to 166 of the 481 objects. Fewer includes of them would speed up both CI's heavy pushes and every local build. That is its own piece of work, with no plan yet.

## Results

(Filled in after the proving runs.)
