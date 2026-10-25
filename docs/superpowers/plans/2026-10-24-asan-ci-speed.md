# A faster AddressSanitizer job in CI

Plan written 2026-10-24 by planning agent a2ff272354964e54c (task ASAN-PLAN, session bc2849d9). Nothing here is built yet. Every task waits for the user's yes (CLAUDE.md rule 3). The user's words that shaped it: "create a plan with all of these and test real numbers before merging anything".

## The one-paragraph version

The nightly `asan` job takes about 26 minutes. Half of that, 13 to 14 minutes, is one test shard, `track_state`, and almost all of that shard is two test cases that call `TrackState::window` on an empty window hundreds of thousands of times. Each call rebuilds a sorted sweep structure from scratch, which is about sixty small heap allocations, and AddressSanitizer makes small allocations very slow. The fix is to build that sweep once, when the state's intervals change, and answer each lookup by binary search. That alone should bring the test step from about 14 minutes to about 4. After it, the shards get rebalanced so the four cores stay busy, the GUI tests move to their own runner so they stop adding four minutes to the end, and the compiler cache gets one measured trial on a branch (the numbers I have say it will not pay, and the plan says why). Each change is measured on real GitHub CI before it merges. Then the docs get the real numbers.

## Approving this plan approves these outward-facing steps

Each task is measured by pushing its branch to `origin` (repo `splax77/hydra-deluxe`) and starting the workflow on it by hand. That is a push and a manual CI run per task, and one task (the compiler cache) needs two runs. A manual run (`workflow_dispatch`) also runs every other job in `ci.yml`: "Build and test", "GUI tests", "Coverage report", "Fuzz corpus replay" and "Tool tests" all run on a manual start (only "Fuzz corpus replay" and "Tool tests" skip the nightly schedule, and "Build and test" and "GUI tests" run cold on a manual start unless the `warm` input is on). Hosted runners are free for a public repository. A manual run also saves a build folder cache for its branch (the "Save the build folder" step runs on `workflow_dispatch`, `ci.yml` lines 90 to 99), about 200 MB each per `ci.yml` line 101, and the prune step keeps the two newest per branch. Saying yes to this plan says yes to those pushes, runs and caches. Nothing merges to `main` until its numbers are in this plan's Results section, and a task that misses its pass number goes back to the user as a report, not into `main`.

## What I measured and read (every number's source)

### The baseline, from GitHub's step timestamps

`gh run view 40082627459 --json jobs` (the nightly run of 2026-10-24, main at fa19f5c9) gives the `asan` job's steps:

| Step | Time |
|---|---|
| Check out the code | 10 s |
| Set up the Visual Studio developer shell | 6 s |
| Build hydra_tests | 456 s (7.6 min) |
| Run hydra_tests (`ctest -j 4`) | 831 s (13.9 min) |
| Build hydra_uitest | 31 s |
| Run hydra_uitest | 227 s (3.8 min) |
| Job total | 26 min 8 s |

The three nightly runs before it (39937243620, 39761955257, 39583374199) took 23.5, 25.6 and 25.8 minutes (1410, 1537 and 1548 s), with the build between 6.5 and 7.3 minutes (388 to 437 s) and the test step between 12.4 and 14 minutes (741 to 837 s); the reviewer re-read all four runs with `gh run view --json jobs`. The run log (`gh run view 40082627459 --log`, saved in the planning scratch folder as `asan_AddressSanitizer.txt`) gives each ctest entry's time: `track_state` 829 s, `single_owner` 241 s, `rest` 210 s, `app_store` 158 s, `shards_cover_all` 4 s. ctest ran the four shards side by side, so the step's 831 s is the `track_state` shard alone; the other three were done by 241 s and three cores then sat idle for ten minutes.

For comparison, the normal "Build and test" job on push run 40142020665 built in 5 min 27 s, ran `hydra_tests` in 55 s and `hydra_uitest` in 2 min 13 s (from `docs/superpowers/plans/2026-10-24-ci-build-speed.md`, which read the same command). The ctest output of that same run (`normal.txt` in the scratch folder) shows the shards at 8, 8, 23 and 32 s, 32.4 s in all; the step is longer than ctest's own total because ctest takes about 20 s to start its first test. `track_state` takes 8 s there against 829 s under ASan, a factor of 100. The other shards are 5 to 30 times slower under ASan. That gap is the clue: something in `track_state` is pathological under ASan, not merely slow.

`docs/development.md` line 465 says the job's first run took 20m41s. Four nights of 23.5 to 26 minutes say that is stale.

### Local per-case times under ASan

A local ASan build sits in `build-asan\` (built from main b6703774). The main session ran each shard with doctest's `-d=true` and saved the per-case times in the scratch folder: `ts_durations.txt`, `dur_single_owner.txt`, `dur_app_store.txt`, `dur_rest.txt`. I read them and summed them by file.

The `track_state` shard takes 316 s locally. Two cases take 314 s of that: "rebuild_overlay_fields: equals a full build on random overlays" (161.4 s) and "rebuild_overlay_fields: equals a full build on every 8th corpus chart's stored paths" (153.2 s). Every other case in the shard is under 0.25 s. In the normal build (`build-cpp`) those two cases take 2.1 s and 2.7 s (the main session's reading). The shard's assertion count under doctest was 88,852.

The other shards locally: `single_owner` 64.4 s, `app_store` 37.4 s, `rest` 103.3 s. By file, the heavy ones are:

| File | Local ASan seconds | Shard today |
|---|---|---|
| `tests/single_owner/scan.cpp` | 22.3 (one case, the single-owner scan at 22.0) | single_owner |
| `tests/test_replay_windows.cpp` | 21.2 (the RapidCheck case is 11.1) | single_owner |
| `tests/test_replay.cpp` | 18.6 | single_owner |
| `tests/test_precheck.cpp` | 25.4 (fixture 11.4, two real ranges 7.8 and 6.1) | rest |
| `tests/test_preview_scene.cpp` | 13.6 (one corpus case, 13.2) | rest |
| `tests/test_perf_digest.cpp` | 9.2 | rest |
| `tests/test_precheck_git.cpp` | 6.8 | rest |
| `tests/test_engine_sweeps.cpp` | 13.4 | app_store |
| `tests/test_store_results.cpp` | 10.1 | app_store |
| `tests/test_cli.cpp` | 6.6 | app_store |

CI is slower than this machine: 829 against 316 s for `track_state` (2.6 times), 241 against 64 s for `single_owner` (3.8 times), 158 against 37 s for `app_store` (4.2 times), 210 against 103 s for `rest` (2.0 times; see the checkout-depth note below for why `rest` is lighter on CI). I use 2.5 to 4 as the CI factor in every prediction below.

### The diagnosis, checked in the code

Both slow cases call `check_rebuild` (`tests/test_track_state.cpp` line 813). For every pair of neighbouring instants it calls `got.window(w[i].t, w[i+1].t)` and `want.window(...)` (lines 825 to 829). Instants are the only things inside a window, and these two neighbours have nothing between them, so the window is always empty and `TrackState::window` (`src/render/track_state.cpp` line 430) falls back to `synthesize((near + far) / 2)`. `synthesize` (line 410) does `FieldSweep(*this).set(inst)`: it builds a fresh `FieldSweep`, which is six `SpanSweep`s, four more for the drum pads, and one `LaneSweep` (lines 219 to 249). Each `SpanSweep` copies its intervals into five vectors and sorts all five (lines 63 to 78); the `LaneSweep` copies and stable-sorts an index list (lines 110 to 119). That is about sixty allocations (ten `SpanSweep`s with five vectors each, plus the lane sweep's) and fifty-one sorts per call (five per `SpanSweep`, one in the `LaneSweep`), thrown away at once. The random-overlay case has about 1,500 notes, 400 beats and 300 span edges, so a few thousand instants, and calls `check_rebuild` 25 times, each doing two pro settings, two rebuild steps and two states: on the order of half a million `synthesize` calls, tens of millions of allocations. The sweeps' answers never change between calls, because the intervals do not change.

Under ASan every allocation and free is intercepted, padded, poisoned and quarantined, and a stack trace is captured for each. The [LLVM thread](https://lists.llvm.org/pipermail/cfe-dev/2017-December/056381.html) confirms the quarantine and stack depot exist and cost memory; it does not give a time cost per small allocation, so the claim that small allocations are the slow part is my reasoning. The measured 100-times gap between `track_state` on the two builds (8 s against 829 s) is the evidence, and the executor's timed run (Task 1) tests it. See also [Microsoft's MSVC ASan post](https://devblogs.microsoft.com/cppblog/addresssanitizer-asan-for-windows-with-msvc/). A function that is "sixty mallocs and fifty-one sorts" is the worst shape for it. The normal build hides this because the Windows heap is fast for small blocks (and the other exes use mimalloc, but `build-asan` runs on the Windows heap by design, `CMakeLists.txt` line 101).

Who else calls `window`: one place in `src/`, `build_highway_draws` in `src/render/highway_draw.cpp` line 243, once per drawn frame. Whenever the visible stretch of highway holds no instant (the song's lead-in, a quiet bar, or a short window at high speed) it pays the same rebuild every frame. Nobody else calls `synthesize`.

`TrackState` is copied in the app: `preview_load_job.cpp` line 405 (`render::TrackState track_state = base->track_state;`) and held in `std::optional` in `preview_controller.h` line 339. So whatever the fix stores must copy cleanly.

### What the earlier CI plan already measured about compiler caches

`docs/superpowers/plans/2026-10-24-ci-build-speed.md` (merged 556cd6df) measured the cache question for the normal build on this code, 4 CPUs pinned, cold `hydra_tests`: Visual Studio generator with `/Zi` 193 s; Ninja Multi-Config with `/Z7` 257 s. sccache needs Ninja (CMake's compiler-launcher setting does not work with the Visual Studio generator, [CMake discourse](https://discourse.cmake.org/t/using-a-compiler-cache-on-windows-with-msvc/13694)) and `/Z7` or `CMAKE_MSVC_DEBUG_INFORMATION_FORMAT=Embedded` (its MSVC parser refuses a shared `.pdb`, [sccache README](https://docs.rs/crate/sccache/0.17.0)). Its GitHub Actions cache backend makes one cache call per object and GitHub's rate limit turns some into misses ([Depot on sccache in Actions](https://depot.dev/blog/sccache-in-github-actions), [Mozilla's sccache-action](https://github.com/Mozilla-Actions/sccache-action)). ccache has supported MSVC since 4.6 but historically refused `/Zi` ([PostgreSQL list thread](https://www.postgrespro.com/list/thread-id/2602085)), and users report it saving nothing under CMake with MSVC on Actions ([ccache-action issue 112](https://github.com/hendrikmuhs/ccache-action/issues/112)); OpenMW swaps `/Zi` for `/Z7` when the launcher is set ([OpenMW commit](https://git.cijber.net/mirror/openmw/commit/240e2486ed5f9bc885d5d1150ff29fdd4a782ba8)). That plan chose a saved build folder (`.github/actions/warm-build-folder`) over a compiler cache, and the saved folder is live in `ci.yml` for the normal jobs. The project's generator-specific workarounds are in `CMakeLists.txt`: the `%` doubling that `/PDBALTPATH` avoids (CMake issue 15865, line 71) and plain `/LTCG` instead of the incremental kind (CMake issue 20484, line 55). `tests/test_build_flags.cpp` line 51 checks the stored `.pdb` name is bare; it reads the exe, so it holds under any generator that keeps the `/PDBALTPATH` line.

How warm a nightly cache could be: `git log --first-parent --since=14.days --format=%cd --date=short main` gives 1 to 72 commits a day on main (1, 69, 3, 72, 33, 27, 14, 10, 10, 3, 32, 21, 32 over the last thirteen days with commits). The earlier plan found 6 of 40 commits (15%) touch a header that recompiles 100 to 166 of the 481 objects. At even 10 commits a day, the chance that a day has no such commit is 0.85 to the tenth, about 20%. So on four nights in five the nightly build is mostly a cache miss anyway.

### The checkout depth

The `asan` job checks out with the default depth (`ci.yml` line 183; `actions/checkout` defaults to depth 1). "Build and test" uses `fetch-depth: 0` so `tests/test_precheck.cpp` can run its two real commit ranges (lines 457 and 477; it prints "step 1 range skipped" and "step 2 range skipped" without them). Under ASan those two cases take 7.8 and 6.1 s locally, so on CI the `rest` shard skips about 30 to 50 s of work, which is why `rest` is only 2.0 times its local time while the other shards are 2.6 to 4.2 times. Does it matter? The same git code runs under ASan anyway: `tests/test_precheck_git.cpp` builds its own repository (`git init` at line 43) and runs nine cases over it, 6.8 s locally. The two real ranges add a second sample on real history, not new code. Recommendation: keep the default depth. Making it `fetch-depth: 0` would add about 10 s of checkout and 30 to 50 s to `rest` for no new sanitizer coverage. Task 5 writes that sentence into the docs.

## The design, decided

### Item 1: synthesize answers by lookup

Today's `SpanSweep::at` walks five sorted vectors forward with cursors (`track_state.cpp` lines 80 to 98), so it must be asked in increasing time order and a one-off question needs a fresh object. The counts it keeps are exactly what binary search gives on the same sorted vectors: the number of starts strictly before `t` is `lower_bound(starts_, t)`, the number of ends at or before `t` is `upper_bound(ends_, t)`, the zero-length intervals at `t` are `equal_range(zeros_, t)`, and "some interval starts (ends) at `t`" is `binary_search` on `all_starts_` (`all_ends_`). The rule in the header comment (lines 113 to 119: inside wins, then Restart, Start, End, Empty) stays word for word.

So the change is: `SpanSweep::at` and `LaneSweep::at` become `const` lookups over the sorted data, with no cursors. `LaneSweep` picks "the first interval in list order that starts at or contains `t`, else the first that ends at `t`" by scanning its (small) interval list; fills with lanes are few per chart (my estimate, not counted; the executor counts them on one corpus chart), so a linear scan is cheaper than the active set it replaces. `FieldSweep` becomes a member of `TrackState`, built once at the end of `sweep_span_fields` (the one place the intervals are final) and owning its sorted copies, so `TrackState` stays a plain copyable value. Since the header forward-declares `FieldSweep` (line 197) and `TrackState` is copied in `preview_load_job.cpp`, the executor either moves `LaneSweep` and `FieldSweep` into the header as complete types or stores their sorted vectors directly as `TrackState` members; `std::unique_ptr` is out because it would silently delete the copy constructor the app relies on. `synthesize(t)` then reads the stored sweep. `sweep_span_fields` uses the same stored sweep for every instant, so there is exactly one implementation of "the state of a span at `t`": one owner, as CLAUDE.md's derive-once rule asks. Cost for the full sweep goes from amortised constant to one binary search per instant per field, about seven comparisons, which is nothing.

The tests keep their full coverage. `check_rebuild`'s loop over every neighbouring pair stays exactly as it is; only its cost changes. One new test pins the agreement the brief asks for: for every instant of the random scenes and of one corpus chart, `synthesize(inst.t)` must carry the same span fields as the stored instant (the lookup must agree with the sweep that filled the instants). The existing truth table (line 147), the one-pass sweep case (line 630) and every `check_pinned` string already pin the old sweep's answers, so a wrong binary search fails them.

What the app gains: `build_highway_draws` stops allocating and sorting on every frame whose window is empty. With a chart's few hundred intervals that is tens of microseconds a frame, not something a player sees. The real gain is one lookup structure instead of two ways of computing the same thing. I will not claim a visible speed-up.

Predicted times, with their basis. The two cases take 4.8 s in the normal build (2.1 + 2.7, the main session's measurement). If `synthesize` is roughly half of that, the rest is `build_track_state` four times per `check_rebuild` plus the instant comparison, which allocate per instant and run 5 to 15 times slower under ASan (the other shards' ratios). That gives 10 to 40 s locally for the two cases, so a `track_state` shard of 12 to 45 s locally and, at the 2.6 factor this very shard showed (829 against 316 s), 30 to 120 s on CI. A factor of 4 would give up to 180 s, above A6's 150 s; this is a prediction, not a measurement. The test step then becomes the slowest other shard, `single_owner` at about 241 s. So: shard from 829 s to 30 to 120 s; step from 831 s to about 245 s; job from 26 min to about 16.

### Item 2: rebalance the shards

After item 1 the shards' local ASan weights are about: `single_owner` 64 s, `rest` 103 s (about 90 on a shallow CI checkout), `app_store` 37 s, `track_state` 12 to 45 s. The ideal is total work over four cores: about 60 s locally. The heaviest single files are the ones in the table above; a shard is whole files (`tests/shards.txt` header), so the single-owner scan case (22 s) can never be split further and sets a floor of about 25 s local (about 90 s CI) for whichever shard holds it.

The executor computes the layout from the per-file numbers, but the shape I expect is four named shards plus `rest` (the `audio_device` entry stays, and CI skips it): `single_owner` keeps `*single_owner*` and `test_library_jobs.cpp` (about 25 s); a new `replay` shard takes `test_replay.cpp`, `test_replay_windows.cpp` and `test_engine_sweeps.cpp` (about 53 s); `track_state` takes `test_preview_scene.cpp` beside its two files (about 30 to 60 s); `app_store` loses `test_engine_sweeps.cpp` and gains `test_precheck.cpp` and `test_precheck_git.cpp` (about 56 s); `rest` drops to about 58 s. Local wall would fall from about 103 s (the `rest` shard alone) to about 55 to 60 s (the heaviest of those, `replay` or `app_store`). On CI I predict about 180 to 200 s, down from about 245 s. Basis: the `replay` shard's 53 s times 3.7, the factor its files showed inside `single_owner` (241 against 64.4 s), is about 196 s. The weakest part is `app_store`: its new files came from `rest`, whose factor was 2.0, while its old files had 4.2, so its 56 s could land anywhere from about 110 to 235 s, and 235 would miss S2's 220. It is a prediction, not a measurement. Six entries on four cores is fine: ctest starts the next entry as a slot frees, and `-j 4` stays, because the runner has 4 CPUs ([GitHub's runner reference](https://docs.github.com/en/actions/reference/runners/github-hosted-runners)) and more slots would only time-slice.

Balancing for ASan must not hurt the normal job. The normal shards took 8, 8, 23 and 32 s (`normal.txt`) and the "Run hydra_tests" step 30 to 70 s over six runs (30, 54, 55, 56, 63 and 70 s: runs 40140877018, 40158651019, 40142020665, 40157094452, 40157496862 and 40158194515; ctest alone took 32.4 s on 40142020665, and the rest is startup). Today's shards already exceed the 60 s line on two of those six, so Question 3 asks the user how to judge it. The executor measures each moved file in the normal build too and reports both layouts' predicted maxima. The pass rule is that the normal "Run hydra_tests" step stays at or under 60 s on the measuring run. `hydra_tests_shards_cover_all` (`tools/check_shards.ps1`) stays the proof that every case runs exactly once; it needs a configured build folder but reads the case list from any `hydra_tests.exe`, so the executor configures its worktree and points `-Exe` at the main checkout's exe.

Is any remaining slow case an ASan-only waste like item 1's? The shard ratios say `single_owner` is 30 times slower under ASan while `rest` and `app_store` are 5 to 9 times, so the scan (22 s) or the RapidCheck replay (11 s) may have an allocation-heavy loop. The executor runs the six heaviest cases in both `build-cpp` and `build-asan` with `-d=true` and reports the ratio for each; any case over 15 times (above the band the healthy shards sit in) is named in the report as a candidate for its own fix, with the loop it found. Fixing one is not in this plan; the user decides.

### Item 3: a compiler cache for the asan build

The evidence above says no, and the user wants it measured anyway. So the task is one measured trial on a branch that never merges unless the numbers pass.

The trial: a new configure preset `asan-ninja` that inherits `asan` but uses the Ninja Multi-Config generator, embeds debug info (`/Z7` instead of `/Zi`, through a `HYDRA_EMBEDDED_DEBUG` switch the preset turns on, applied in the `add_compile_options` line at `CMakeLists.txt` line 65), and sets `CMAKE_C_COMPILER_LAUNCHER` and `CMAKE_CXX_COMPILER_LAUNCHER` to `sccache`. Ninja ships with Visual Studio's CMake (`Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe`), and the asan job already runs `ilammy/msvc-dev-cmd`, which puts both on `PATH`. On the branch, the asan job's build steps install sccache with `Mozilla-Actions/sccache-action` using the GitHub Actions cache backend (`SCCACHE_GHA_ENABLED=true`), build with the new preset, and print `sccache --show-stats` after each build. The cache key is sccache's own, one entry per object keyed on the preprocessed source and compiler flags, under a version prefix `SCCACHE_GHA_VERSION=asan-1` so a flag change starts clean. GitHub's cache scoping decides who reads what ([dependency caching reference](https://docs.github.com/en/actions/reference/workflows-and-actions/dependency-caching)): a nightly run on main reads main's entries; a manual run on a branch reads that branch's and main's. Only the measuring branch writes during the trial. `/PDBALTPATH` stays, so `test_build_flags` still sees `hydra_tests.pdb`; `/Z7` leaves the linker's `.pdb` in place.

Expected numbers. The baseline build is 390 to 456 s over four nights. The earlier plan's cold Ninja `/Z7` build was 1.33 times the Visual Studio one (257 against 193 s), so a cold night on the new preset is predicted at 520 to 610 s, about two minutes worse. A fully warm run (every object a hit) leaves configure (about 33 s), the links, sccache's cache calls and the misses GitHub's rate limit creates: predicted 150 to 250 s. With 1 to 72 commits a day and a core header in about one commit in seven, the warm case is the exception. Expected nightly saving: negative to small. Risks: a second generator only CI uses, so the `%`-doubling and `/LTCG` workarounds get a path no developer runs; sccache's rate-limited backend dropping saves; a larger cache footprint beside the two saved build folders and the fuzz corpora.

Pass rule for merging: warm build at or under 250 s and cold build at or under 500 s (no more than about 45 s worse than the slowest baseline night). I predict the cold rule fails. If so the task's branch is deleted, the numbers go into this plan's Results, and nothing merges. A cheaper alternative, warming `build-asan` with the existing saved-folder action, is Question 1 below; it is not measured unless the user asks.

### Running the GUI tests beside the unit tests

Today the asan job builds and runs `hydra_uitest` after the unit tests: 31 + 227 s at the end. Inside one job they cannot overlap usefully: the ctest entry `hydra_uitest` books 4 processors (`CMakeLists.txt` line 640, from `kDefaultJobs = 4`), so `ctest -j 4` would run it alone, and raising `-j` to 8 would run eight processes on four cores and only time-slice; the GUI tests draw on the software GPU and are CPU-bound too. The clean move is the one the normal jobs already made: a second job, `asan-gui`, on its own runner, that checks out, sets up the developer shell, builds `hydra_uitest` with the asan preset and runs it. Its build is cold (no saved folder for ASan), predicted at 3.8 to 5.2 minutes (low end: the earlier plan read 2 min 17 s for a cold build up to `hydra_ui.lib` on run 40142020665 and guessed 25 s more for the harness, times 1.4; high end: the "GUI tests" job's cold "Build hydra_uitest" step took 3 min 43 s on run 40157496862, times 1.4. That run's conclusion is "cancelled" because its asan job was cut off; the GUI job finished. ASan compiles about 1.4 times slower, from the asan build's 456 s on run 40082627459 against the normal build's 327 s on run 40142020665), so the job is about 8 to 10 minutes, shorter than the asan job it runs beside. The asan job loses 258 s.

### ctest -j

Stays at 4. See item 2.

### D199 and pushes

D199 (`docs/audit/2026-10-03-fix-decisions.md` line 757): the user's words were "if its that much slower then no" about an ASan run on every run, and it moved to nightly. After every task here the asan job is still about 7.6 minutes of build plus 3 to 4 minutes of tests, about 11 minutes, against a warm push run of about 3½ minutes (the earlier plan's estimate). Moving it onto pushes would roughly triple what a push waits for. I do not propose it, and there is no question to ask.

## Expected times after the plan (estimates, with how I got them)

| Step | Today (run 40082627459) | After | Basis |
|---|---|---|---|
| Build hydra_tests | 456 s | 390 to 456 s | unchanged (the cache trial is predicted not to merge) |
| Run hydra_tests | 831 s | 180 to 200 s | item 1 removes the 829 s shard; item 2 lowers the slowest shard from 241 s to about 200 |
| Build + run hydra_uitest | 258 s | 0 s in this job | moved to `asan-gui`, which runs beside it in 8 to 10 min |
| asan job total | 26 min 8 s | about 10 to 12 min | the sum |

## The measurement protocol (the main session runs it; executors never push)

Each task is built on its own branch in its own worktree by one executor, who commits and stops. The main session then does the measuring, in this order, with these caps:

1. Push the task branch: `git push origin <branch>`. Do not push to it again until its run is read, because on a non-main branch a newer push cancels the run in progress (`ci.yml` lines 32 to 34).
2. Start the workflow on it: `& "C:\Program Files\GitHub CLI\gh.exe" workflow run ci.yml --ref <branch>` (add `-f warm=true` only where a task says so).
3. Find the run: `gh run list --workflow ci.yml --branch <branch> --limit 1 --json databaseId,status`.
4. Read it once it is done: `gh run view <id> --json jobs`, taking each step's `completedAt` minus `startedAt`, and `gh run view <id> --log` for the per-shard ctest lines ("Test #n: hydra_tests_<shard> ... Passed  N sec"). Check at most four times, fifteen minutes apart; if the run is still going at the fourth check, sixty minutes in (the job's own `timeout-minutes`), report that and stop.
5. Runner speed varies by two thirds (the same build took 5 min 27 s and 9 min 4 s on two days, per the earlier plan). If the run's "Build hydra_tests" step is over 540 s (20% above the slowest baseline night), start one more run before judging; two runs are the cap. Judge each pass rule on the better of the two. Task 3 is the exception: its cold run is predicted at 520 to 610 s, and its second run must stay warm, so it is judged on its two planned runs and step 5 does not add a third.
6. Write the run id and the numbers into the Results section of this plan, then merge or report.

Merges of build files and the test harness run the whole suite plus the three repo-wide tests (CLAUDE.md rule 1): `shards.txt` is read by `CMakeLists.txt`, and `ci.yml`, `CMakePresets.json` and `build_cpp.ps1` are build files, so Tasks 2, 3 and 4 get the whole suite at merge. Task 1 touches a header `highway_draw.cpp`, `preview_controller.cpp` and `preview_load_job.cpp` include, so its merge runs the tests that touch those files: `-sf=*test_track_state.cpp`, `-sf=*test_highway_draw.cpp`, `-sf=*test_preview_controller.cpp`, `-sf=*test_preview_scene.cpp`, plus the three repo-wide tests.

## Tasks

Each task is one executor (Opus, in its own worktree off `main`, under 100 tool calls and 20 minutes). Every brief carries the status rule (`hooks\state\status`). Tasks 1, 3 and 4 run in parallel in wave 1. Tasks 3 and 4 both touch the `asan` job in `ci.yml`: Task 3's edit lives only on its trial branch, and if the trial passes it merges after Task 4, with the main session resolving the overlap so that `asan-gui` takes the same preset. Task 2 waits for Task 1's merge because its numbers depend on the fixed shard. Task 5 waits for every CI number.

### Wave 1

#### Task 1 (ASAN-SYNTH): synthesize by lookup

Make `SpanSweep::at` and `LaneSweep::at` stateless lookups by binary search over the sorted vectors they already hold, store one `FieldSweep` in `TrackState` built at the end of `sweep_span_fields`, and have `synthesize` read it, as "Item 1" above decides. Keep `TrackState` copyable (it is copied at `preview_load_job.cpp` line 405). Keep `check_rebuild`'s loop over every neighbouring pair unchanged. Add the test "synthesize at an instant's own time agrees with the sweep that filled it" over the random scenes and one corpus chart. Comments name where the rule lives and do not restate it (brief-preamble "If you write code"). No new numeric threshold appears, so no user decision is needed.

Run the two slow cases red-then-timed: before the change, note the times from `ts_durations.txt` (161.4 s and 153.2 s); after it, rebuild `build-asan` for `hydra_tests` (warm; the folder exists at b6703774, one build at most) and run `build-asan\Release\hydra_tests.exe -d=true -sf=*test_track_state.cpp` from a Visual Studio developer shell (the ASan runtime DLL is on that shell's `PATH`, `docs/development.md` line 214). Report the two cases' times, the shard total and the assertion count, which must still be 88,852 plus the new test's assertions.

Owns: `src/render/track_state.h`, `src/render/track_state.cpp`, `tests/test_track_state.cpp`. Tests it may run: `build-cpp\Release\hydra_tests.exe -sf=*test_track_state.cpp`, `-sf=*test_highway_draw.cpp`, `-sf=*test_preview_controller.cpp`, and the ASan run above. About 60 calls.

Acceptance criteria:
- A1. The new agreement test passes, and the executor's report shows it fails when `synthesize` is made to return `Toggle::Empty` for one field (one deliberate break, reverted; the red line is kept for the reviewer).
- A2. `check_rebuild` lines 825 to 830 are unchanged (the reviewer diffs them), and the shard's assertion count is 88,852 plus the new test's count.
- A3. Local ASan time of the two cases together is at or under 40 s (predicted 10 to 40, from 314 s). Over 40 s is a miss: report, do not merge.
- A4. `hydra_tests.exe -sf=*test_preview_controller.cpp` passes, which compiles the copy at `preview_load_job.cpp` line 405 and so proves `TrackState` is still copyable.
- A5. The single-owner scan (`-sf=*single_owner*`, run at merge by the main session) passes with one `at` implementation per sweep class; the reviewer greps for a second walk and finds none.
- A6. CI, branch `asan-synth`: `hydra_tests_track_state` at or under 150 s (predicted 30 to 120, from 829) and the "Run hydra_tests" step at or under 300 s (predicted about 245, from 831). A miss goes to the user.

#### Task 3 (ASAN-CACHE): the compiler-cache trial

Add the `asan-ninja` configure and build presets to `CMakePresets.json`, the `HYDRA_EMBEDDED_DEBUG` switch in `CMakeLists.txt` that swaps `/Zi` for `/Z7` at line 65 when on, `asan-ninja` in `build_cpp.ps1`'s preset set, and on the branch only, the asan job's build steps in `ci.yml`: install sccache (`Mozilla-Actions/sccache-action`, GHA backend, `SCCACHE_GHA_VERSION=asan-1`), build `hydra_tests` and `hydra_uitest` with `-Preset asan-ninja`, and print `sccache --show-stats` after each build. Change nothing in the `asan` preset itself. Locally, prove the preset configures and builds `hydra_tests` once (one cold build, capped at 15 minutes; if it does not finish, report the state and stop) and that `hydra_tests.exe -sf=*test_build_flags.cpp` passes on it.

Owns: `CMakePresets.json`, `CMakeLists.txt` line 65 and the new option beside the `HYDRA_ASAN` block, `build_cpp.ps1`, the asan job's build steps in `ci.yml`. Tests it may run: `build-asan-ninja\Release\hydra_tests.exe -sf=*test_build_flags.cpp`, `-sf=*test_track_state.cpp`. About 50 calls.

Acceptance criteria:
- C1. `test_build_flags` passes on the Ninja `/Z7` build (the stored `.pdb` name is still bare).
- C2. CI, branch `asan-cache`, two manual runs in a row with no push between them: the first is cold (stats show hits near zero), the second warm (stats show hits for most compiles). Both step times and both stats blocks go in Results.
- C3. Merge rule: warm "Build hydra_tests" at or under 250 s (predicted 150 to 250) and cold at or under 500 s (predicted 520 to 610). If either fails, the branch is deleted, the numbers are recorded, and nothing merges. If both pass, the task merges as the asan job's build and Task 5 documents it.
- C4. The trial never changes the `asan` preset, so a failed trial leaves developers' `build-asan` as it is (the reviewer checks the preset diff).

#### Task 4 (ASAN-GUI-JOB): the GUI tests on their own runner

Add the job `asan-gui` ("AddressSanitizer GUI tests (Windows, Release)") to `ci.yml` with the same `if:` as `asan`, `timeout-minutes: 30`, steps: checkout (default depth), `ilammy/msvc-dev-cmd@v1` with `arch: x64`, `.\build_cpp.ps1 -Preset asan -Target hydra_uitest`, `.\build-asan\Release\hydra_uitest.exe --all`. Delete the two `hydra_uitest` steps from `asan`. Keep the comment above `asan` true (it should say the GUI tests run in `asan-gui`). YAML has no parser on this machine (memory rule 2026-10-24), so the executor reads every new line for an unquoted `: ` and quotes or blocks it.

Owns: the `asan` and `asan-gui` jobs in `ci.yml`. Tests: `hydra_tests.exe "-tc=docs match code*"`. About 25 calls.

Acceptance criteria:
- G1. CI, branch `asan-gui-job`: `asan-gui` passes with "109 of 109 passed" (the count the 2026-10-24 log prints) and takes at most as long as the `asan` job on the same run (predicted 8 to 10 min against about 11 to 16 min).
- G2. The `asan` job on that run has no `hydra_uitest` steps and is shorter than the baseline by at least 240 s once Task 1's numbers are accounted for (that is, compare it with Task 1's measured run if Task 1 merged first, else with 26 min 8 s). A miss goes to the user.

### Wave 2 (after Task 1 merges)

#### Task 2 (ASAN-SHARDS): rebalance the shards

Measure, then edit. Measure the normal-build time of every file that moves (`build-cpp\Release\hydra_tests.exe -d=true -sf=<file>` against the main checkout's exe, read-only; if that exe is missing or another session is rebuilding it, report and stop) and the ASan time of the post-fix `track_state` shard (from Task 1's report, or `build-asan` if the main checkout's was rebuilt after the merge). Then write the new `tests/shards.txt` so that, on the local ASan numbers, no shard is more than 1.3 times the mean of the named shards and `rest`, and on the normal numbers no shard exceeds 35 s (a little over the heaviest today, 32 s). Update the header's "Balanced on" sentence to name this plan and the scratch files' numbers, as data, not as a copy of the rule. Prove coverage: configure the worktree's `build-cpp` (`. tools\find_cmake.ps1; (Find-CMake) --preset default`, configure only) and run `pwsh -NoProfile -File tools\check_shards.ps1 -Ctest <ctest from that cmake folder> -BuildDir <worktree>\build-cpp -Config Release -Exe <main checkout>\build-cpp\Release\hydra_tests.exe`.

Also the ratio table: run the six heaviest cases (single-owner scan, the RapidCheck replay, "replay: on every corpus path...", the precheck fixture case, "base + overlay: equals build_preview_scene...", "a saved record reads back whole...") with `-d=true` and `-tc=<name>` on both exes and report normal seconds, ASan seconds and the ratio. Any case over 15 is named with the loop the executor believes allocates in it, for the user; no fix in this task.

Owns: `tests/shards.txt`. Tests: `check_shards.ps1` as above; the per-file timing runs. About 50 calls.

Acceptance criteria:
- S1. `check_shards.ps1` prints "every case runs in exactly one entry, and every test file has cases" on the new layout.
- S2. CI, branch `asan-shards`: "Run hydra_tests" in the `asan` job at or under 220 s (predicted 180 to 200, from about 245 after Task 1), and no shard over 200 s.
- S3. CI, same run: "Run hydra_tests" in "Build and test" at or under 60 s (today 30 to 70 s over six runs; see Question 3). The normal job must not get slower than that.
- S4. The ratio table is in the executor's report and copied into Results; each case over 15 is listed under Questions for the user.
- S5. `ctest -j 4` in `ci.yml` is unchanged (the reviewer reads line 198).
- Missing S2 or S3 goes to the user, not to `main`.

### Wave 3 (after every CI number is in Results)

#### Task 5 (ASAN-DOCS): the measured numbers in the docs

Rewrite the timing sentences of `docs/development.md`: in "Continuous integration", replace "its first run, on 2026-10-10, took 20m41s against 6m16s for the main job" with the measured asan and asan-gui job times and the run id; name the `asan-gui` job in the job list; add one sentence that the asan jobs keep the default checkout depth so the precheck self-test skips its two real ranges there and the normal job runs them. In "The AddressSanitizer build", keep the commands (they still hold) and add one sentence pointing at the sweep lookup only if the executor finds the section mentions test speed (it does not today; say nothing otherwise). Update the `asan` comment in `ci.yml` to point at the docs for timing, as it does, and the `asan-gui` comment to the same place. If Task 3 merged, add its preset to the build section; if it did not, add one sentence saying the compiler cache was measured and dropped, with the run ids. Every number names its run.

Owns: `docs/development.md` ("Continuous integration" and "The AddressSanitizer build"), the two job comments in `ci.yml`. Tests: `hydra_tests.exe "-tc=docs match code*"`. About 25 calls.

Acceptance criteria:
- D1. `docs match code` passes.
- D2. Every number in the changed paragraphs appears in this plan's Results with a run id (the reviewer checks each one).
- D3. The 20m41s sentence is gone.

## Questions for the user

**Question 1: should `build-asan` get a saved build folder, like `build-cpp`?** The rule today: `ci.yml` lines 70 to 78 restore `build-cpp` on pushes and pull requests and build cold on the nightly schedule and on a manual run (unless its `warm` input is on); lines 49 to 50 give the reason, "as a check that the saved build folder never hid a broken build". The asan job has no saved folder and builds cold every night (456 s). Reusing the `warm-build-folder` action for `build-asan` would make a quiet night's build about 1 to 2 minutes (the normal warm build was 25 s plus restore and save on run 40158194515; ASan links are slower), but a day with a core-header commit rebuilds most of it anyway (5 min 15 s on the normal build for `timing.h`, which is the earlier plan's estimate, 210 s local times 1.5, not a CI measurement), and with 1 to 72 commits a day most days have one. It also adds one more saved folder (about 200 MB each for `build-cpp` per `ci.yml` line 101; an ASan folder is probably larger, not measured) to the repository's 10 GB cache beside the two `build-cpp` folders and the fuzz corpora, and it means no ASan build is ever cold. Options: (a) no, leave the asan build cold (recommended: the saving lands on few nights and costs cache room); (b) yes, as a Task 3b with the same measure-then-merge rule (pass: warm build at or under 150 s).

**Question 2 (only if Task 2's ratio table names a case over 15 times slower under ASan):** should that case get its own fix, as a follow-up plan? The rule in this plan: no fix inside Task 2; the table goes to you with the loop named. Options: fix it next, or leave it since Task 2's rebalancing already hides it behind the other shards.

**Question 3: how should Task 2 judge that the normal job did not get slower?** The rule in this plan (S3): the "Run hydra_tests" step in "Build and test" must be at or under 60 s. Examples from the real runs: 55 s on run 40142020665 passes; 63 s on run 40157496862 and 70 s on run 40158194515 would fail, and both ran today's shard layout. So the line can fail on runner noise alone. Options: (a) keep 60 s on the step and rerun once on a miss; (b) judge the biggest normal shard in ctest's own log at or under 35 s (today 32 s on run 40142020665), which leaves out the startup noise (recommended). The strongest case for (a) is that the step is what the developer waits for.

## Out of scope (for the user to confirm)

- Moving the asan job onto pushes. D199 rules it out at the predicted 11 minutes; see "D199 and pushes".
- Making the asan job's checkout `fetch-depth: 0`. Decided against above; Task 5 records the reason.
- Running `hydra_uitest` inside the asan job at `ctest -j 8`. Decided against above (four cores, a four-slot booking); Task 4 does the clean version.
- Any fix found by Task 2's ratio table (Question 2).
- Shrinking the asan build itself (fewer includes of `timing.h`, `model.h`, `song.h`, `squeeze_rating.h`, `config.h`). The earlier plan noted it; it is its own work.

## Promise map

| Promise | Where in the plan | Task | Check that proves it |
|---|---|---|---|
| Every neighbouring pair of instants is still checked | Item 1, "The tests keep their full coverage" | 1 | A2: lines 825 to 830 unchanged; assertion count 88,852 plus the new test |
| A lookup agrees with the sweep | Item 1 | 1 | A1: the new test, shown red on one deliberate break |
| One owner for "state of a span at t" | Item 1, "exactly one implementation" | 1 | A5: single-owner scan at merge; reviewer finds no second walk |
| TrackState stays copyable | Item 1 | 1 | A4: `test_preview_controller` compiles and passes (copy at `preview_load_job.cpp` 405) |
| Two cases at or under 40 s locally under ASan | Item 1 predictions | 1 | A3, from the `-d=true` run |
| track_state shard at or under 150 s and step at or under 300 s on CI | Item 1 predictions | 1 | A6, from `gh run view --log` |
| Every case runs exactly once after rebalancing | Item 2 | 2 | S1: `check_shards.ps1` output line |
| ASan step at or under 220 s, no shard over 200 s | Item 2 | 2 | S2, from the CI log |
| The normal job's step stays at or under 60 s | Item 2, "must not hurt the normal job" | 2 | S3, from the same run's "Build and test" |
| The six heaviest cases get a normal/ASan ratio, over 15 is flagged | Item 2 last paragraph | 2 | S4: table in the report and Results; Question 2 |
| `ctest -j` stays 4 | "ctest -j" | 2 | S5: reviewer reads `ci.yml` line 198 |
| `test_build_flags` holds on the Ninja `/Z7` build | Item 3 | 3 | C1 |
| Cold and warm cache runs are both measured with stats | Item 3 | 3 | C2: two runs, stats blocks in Results |
| Cache merges only at warm at or under 250 s and cold at or under 500 s | Item 3 pass rule | 3 | C3; otherwise branch deleted, numbers recorded |
| The `asan` preset is untouched by the trial | Item 3 risks | 3 | C4: reviewer checks the preset diff |
| GUI tests run beside the unit tests, 109 of 109, no longer than the asan job | "Running the GUI tests beside the unit tests" | 4 | G1 |
| The asan job loses at least 240 s | same | 4 | G2 |
| Docs and the ci.yml comment carry measured numbers with run ids | Item 4 | 5 | D1 `docs match code`; D2 reviewer checks each number against Results; D3 |
| Checkout depth stays default and the reason is written down | "The checkout depth" | 5 | D2 covers the sentence; Out of scope lists the non-change |
| Nothing merges before its CI numbers are in | "Approving this plan approves..." and the protocol | main session | Results lists a run id above each task's merge commit; Task 5's reviewer checks |
| Executors never push; pushes and runs are the main session's and are approved by the plan's yes | protocol | main session | the briefs say "never push" (brief-preamble "Committing"); Results names who ran each dispatch |
| Waiting on a run is capped: four checks, sixty minutes, two runs (Task 3: its two planned runs) | protocol steps 4 and 5 | main session | Results records the number of checks and runs per task |
| Build-file and harness merges run the whole suite plus the three repo-wide tests | protocol last paragraph | main session | the merge commit's report names the commands (CLAUDE.md rule 1) |
| No move of the asan job onto pushes | "D199 and pushes" | none | Out of scope, for the user to confirm |

## Results

(Filled in by the main session as each run is read: run id, branch, step times, shard times, stats, pass or miss, merge commit.)

### Task 4 (ASAN-GUI-JOB): pass, merged f4c87ac8

Run 40160823788, branch `asan-gui-job` at 9715f06b, one run, read once. The new `asan-gui` job took 326 s (build hydra_uitest 136 s, run 172 s) and printed "119 of 119 passed" (the plan's 109 was the count at planning time; main has gained cases since). The `asan` job took 1,051 s (build 386 s, run hydra_tests 633 s), against 1,568 s on the baseline run 40082627459: 517 s shorter. G1 passes (326 s is under the asan job's 1,051 s) and G2 passes (517 s is over 240 s). Shards on this run, Task 1 not yet in: track_state 631 s, rest 261 s, app_store 212 s, single_owner 156 s. Every other job on the run passed. The task changed only `ci.yml`, so the merge gate needed no code review. After the merge the main session ran the three repo-wide tests on main: docs match code 5 of 5, single-owner scan 18 of 18, precheck self-test 36 of 36.

### Task 3 (ASAN-CACHE): warm build passes, cold misses; merged fa4c72ec by the user's ruling

The user said yes to the fix ("Yes, fix and rerun"). Commit 1c6cd7c1 spells the option `-d1trimfile:`; a local check first showed cl.exe still trims `__FILE__` with that spelling (a test file printed `t.cpp`, not its folder). Cold run 40195923058: "Build hydra_tests" 548 s (13:05:35 to 13:14:43), sccache 447 compile requests, 447 misses, 0 non-cacheable, so every compile was now cached. C3's cold rule (500 s) misses by 48 s; the build with no cache traffic had taken 514 s on run 40161564180. Warm run 40198152081, same commit, no push between: "Build hydra_tests" 95 s (13:33:49 to 13:35:24), 446 hits of 447. C3's warm rule (250 s) passes. The user ruled "Take it forward". Merging main into the branch (988ee03f) conflicted only in a ci.yml comment. The reviewer (derive-once, Scope: full, signed off on 6f9496f7) found that the merge had left `asan-gui` building with `asan-ninja` without installing sccache, and fixed it in 76757ca9; it also ignored `build-asan-ninja/` and named the preset in `docs/development.md` (6f9496f7). Run 40201169431 on 6f9496f7, with Tasks 1, 2, 4 and the slow-case follow-up (`docs/superpowers/plans/2026-10-25-asan-slow-cases.md`, tasks A to C merged ea681b2f, ad252451 and 81031b66) all in: `asan` job 226 s (14:12:47 to 14:16:33; build 81 s, 446 of 447 from the cache; run hydra_tests 126 s, shards track_state 104.68, single_owner 109.61, rest 114.39, app_store 125.61, shards_cover_all 2.15), against 1,568 s on the baseline run 40082627459. `asan-gui` 381 s (build 84 s, 268 of 282 from the cache; 119 of 119 passed). Every job passed. This run's cache was warm for today's code because the cancelled run 40200208384 on 988ee03f had compiled it. Merged fa4c72ec. After the merge, at d4ee798a (another session's TQA-B merge on top), the main session ran the whole suite (1,686 of 1,686), docs match code 5 of 5, the single-owner scan 18 of 18, the precheck self-test 54 of 54, and `check_shards.ps1` (every case in exactly one entry).

### Task 3 (ASAN-CACHE): cold run 1 of 2, sccache cached nothing

Run 40161564180, branch `asan-cache` at 195c77a6. "Build hydra_tests" took 514 s with the `asan-ninja` preset (the Visual Studio `asan` build took 386 s and 478 s on runs 40160823788 and 40161406164 the same day). `sccache --show-stats` after it: 447 compile requests, 0 executed, 0 hits, 0 misses, 447 "Non-cacheable calls", reason "multiple input files" for all 447; the same for the 13 compiles of the hydra_uitest build. So the build ran as a plain Ninja `/Z7` build and wrote nothing to the cache; a warm run would find nothing. The likely cause: `CMakeLists.txt` line 149 adds `/d1trimfile:<source folder>\` to every compile, a flag missing from sccache's MSVC argument table (the executor's note, from sccache's `msvc.rs`), so sccache reads its value as a second input file. Not yet confirmed by a run without the flag. The rest of the job: run hydra_tests 811 s (Task 1 not in this branch), GUI steps 278 s, job 1,628 s, all passed. C3's cold rule (500 s) already misses at 514 s without any cache traffic. Taken to the user before any further run.

The user asked for the cause rather than a stop ("dont just accept the failure. investigate and find out why their being refused"). Found, from sccache v0.19.0's own source (`src/compiler/args.rs` and `src/compiler/msvc.rs`, fetched at that tag): `ArgsIter::next` (args.rs lines 630 to 636) turns an argument with no table entry into `UnknownFlag` only when it starts with `-`; anything else becomes `Raw`, and `parse_arguments` (msvc.rs lines 728 to 734) counts every `Raw` as an input file, then refuses with "multiple input files" (lines 888 to 893). The MSVC table has `d1nodatetime` but no `d1trimfile`, so `/d1trimfile:C:\...\` from `CMakeLists.txt` line 149 is the second "input" on every compile. A scan of every `/` option in the trial's `build-asan-ninja` Ninja files against that table finds only one other unknown option, `/fo`, and it belongs to the resource-compiler rule (`cmcldeps.exe RC`), which has no launcher. Upstream knows the bug: [mozilla/sccache#1899](https://github.com/mozilla/sccache/issues/1899), open, where the reporter found the same flag behind the same misleading reason. The fix that keeps the trimming: spell the option `-d1trimfile:`. `cl.exe` accepts `-` and `/` alike, and sccache passes an unknown `-` option through to the compiler and into the cache key. The source folder is the same on every hosted runner, so the key stays stable between runs.

### Task 2 (ASAN-SHARDS): S2 passes, S3 misses; not merged

Layout: four entries (the executor's departure from the plan's five, because a fifth entry queues behind the first four on `ctest -j 4`): `test_replay.cpp` and `test_precheck.cpp` into `track_state`, `test_precheck_git.cpp` into `single_owner`, `test_perf_digest.cpp` into `app_store`. Reviewer sign-off at e403a0a4 (header comment only). Coverage: `check_shards.ps1` "every case runs in exactly one entry" (1,663 cases, 119 files).

Run 40163121679, branch `asan-shards` at 829b263e (the layout e403a0a4 keeps), one run, read once. The asan job: "Run hydra_tests" 187 s, shards single_owner 186 s, app_store 181 s, rest 180 s, track_state 163 s; job 712 s (build 504 s). S2 passes (220 s, no shard over 200 s). "Build and test": "Run hydra_tests" 99 s (55 s on run 40161406164), shards app_store 95.9, rest 91.6, single_owner 77.8, track_state 45.8, shards_cover_all 6.2 s, sum 317.3 s against S3's 112 s. S3 misses by almost three times. The moved files take about 16 s in a local normal build (the executor's measurement: test_precheck 9.06, test_precheck_git 5.77, test_replay 0.75, test_perf_digest 0.67), so moving them cannot add 224 s of work; the likely reading is that four equally heavy slices now contend for the 4-CPU runner (and the precheck's git calls for its disk) for the whole step, where before one slice ran long and the rest finished early. One run, so runner noise is not ruled out. Taken to the user; not merged at first.

The user asked to find where the time went before changing anything ("find out where all of that time is going and dig into the cause before changing anything"). A local scout run pinned to 4 CPUs could not reproduce it: the new layout ran in 19 s against 37 s for the old. A throwaway diagnostic branch, `diag-shards` at a43adacb (never merged), then ran in one CI job: ctest as CI runs it, and a script (`tools/diag/run_slices.ps1` on that branch) that timed the new layout four at a time and one at a time, the new layout with the two precheck files left in `rest`, the old layout, and the new layout again. Run 40196812678: the runner was an AMD EPYC 9V45 with 2 cores and 4 logical CPUs, and Defender's real-time protection was off (`RealTimeProtectionEnabled: False`). The ctest step took 30.7 s with shards track_state 17.19, shards_cover_all 1.25, single_owner 19.07, rest 30.09 and app_store 30.58, a sum of 98.2 s, so S3 passes on this run. The script's runs: new layout 30.4 s and 29.2 s, one slice at a time 63.9 s, old layout 38.7 s. The one outlier was the new layout with the precheck files in `rest`, at 84.6 s, where `app_store` took 73.7 s of wall time on 5.4 s of its own CPU, the same CPU as in every other run. Its slow cases were file and process heavy ("classify_library_file names each generation of library file" 14.26 s against 0.83 s in the new layout's run; the precheck self-test 12.34 s against 7.89 s in the old layout's run). That layout differs from the old one only by `test_replay.cpp` and `test_perf_digest.cpp`, so the reading is an intermittent stall on the runner, where wall time grows while CPU does not, and not the layout; the 96 s on run 40163121679 fits the same pattern. The user ruled "Merge on this run": merged to local `main` as 833944a5.

The executor's ratio table (Question 2) named four cases over 15 times slower under ASan: the two replay cases (24.8 and 23.1), "base + overlay: equals build_preview_scene on every corpus chart and stored path" (35) and "a saved record reads back whole across the corpus and config matrix" (9 to 17, moves with machine load).

### Task 1 (ASAN-SYNTH): pass, merged 8ac67b38

Local ASan (executor and reviewer, `-sf=*test_track_state.cpp -d=true`): the two slow cases took 2.6 s and 12.5 s, 15.1 s together against 314.6 s before; A3 passes (40 s). Reviewer sign-off on 32d0f23e (derive-once, Scope: full, one finding fixed: the single-owner rule data for the fill-lane pad named the old line).

Run 40161406164, branch `asan-synth` at 32d0f23e, one run, read once (this branch predates Task 4, so its asan job still ran the GUI tests). `hydra_tests_track_state` 59 s against 829 s on run 40082627459 (Question 3 answered by the user after this run: "Total test CPU", so S3 becomes the sum of the normal shards' times on the measuring run within 20% of this run's sum, 93.3 s, so at or under 112 s, because this run's normal `rest` shard took 54 s against 23 s on run 40142020665 with no change to its files); the "Run hydra_tests" step 222 s against 831 s. A6 passes (150 s and 300 s). The other shards: rest 221 s, single_owner 207 s, app_store 144 s, shards_cover_all 16 s. The asan job took 998 s with the GUI steps still in it. In "Build and test" the normal `track_state` shard took 2.0 s (8.3 s on run 40142020665) and the step 55 s. Every job passed. After the merge the main session built `build-cpp` and ran: test_track_state 22 of 22, test_highway_draw 27 of 27, test_preview_controller 26 of 26, test_preview_scene 30 of 30, docs match code 5 of 5, single-owner scan 18 of 18, precheck self-test 36 of 36.
