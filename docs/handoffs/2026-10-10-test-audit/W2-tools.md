# W2: Testing tools survey for Hydra Deluxe (task TA-W2)

Date of survey: 2026-10-10. Read-only research. Nothing in the repo was changed.

## What Hydra uses today (confirmed from the repo)

- One doctest exe, `hydra_tests`. About 1,250 test cases in 78 files under `tests/` (Grep count of TEST_CASE-style macros; approximate).
- `CMakeLists.txt` calls `enable_testing()` and registers just two CTest entries: `hydra_tests` (whole exe) and `hydra_uitest --all`. There is no per-test discovery.
- CI (`.github/workflows/ci.yml`) builds `hydra_tests`, runs it once with one test excluded (`-tce=` the audio device test), then builds and runs `hydra_uitest --all --jobs 4`.
- doctest is vendored in `third_party/doctest`.

Release dates below come from the GitHub releases API (api.github.com), fetched on 2026-10-10. "Unconfirmed" means I could not find a source.

## Short version

The cheapest big win is not a new tool. It is to split the one doctest exe into several parallel runs. After that, an AddressSanitizer job and a coverage job are cheap and supported by Microsoft on Windows. Mutation testing for C++ does not work with MSVC today.

---

## 1. C++ test frameworks

doctest is already in place and is the fastest to compile. Switching costs a rewrite of about 1,250 test cases and buys little. Verdict for all of them: stay on doctest.

### doctest (current)
- What: single-header C++ test framework, tiny compile cost.
- Latest release: v2.5.3, 2026-07-06 (https://api.github.com/repos/doctest/doctest/releases/latest). Actively maintained.
- MSVC/Windows: yes. The 2.5.3 notes mention Windows build fixes (same URL). Hydra already builds with it.
- Speed claim: the author says including the header is "over 20 times lighter" than Catch's (https://raw.githubusercontent.com/doctest/doctest/master/doc/markdown/faq.md). This is the author's own claim, not an independent benchmark.
- Parallel: test cases run one after another inside one process. Asserts are thread-safe, but cases are not parallel (same FAQ).
- Subset options: `--first=N`, `--last=N` (run a numbered range of the filtered list), `--test-case`, `--source-file`, `--order-by`, `--list-test-cases` (https://raw.githubusercontent.com/doctest/doctest/master/doc/markdown/commandline.md). The FAQ says the intended way to go parallel is to start the exe several times with different ranges.
- CTest: `doctest_discover_tests` runs the exe with `--list-test-cases` and makes one CTest entry per test case, with options for prefix, labels (from test suites), JUnit output dir, extra args (https://raw.githubusercontent.com/doctest/doctest/master/scripts/cmake/doctestAddTests.cmake). I could not confirm a build-time vs test-time discovery switch (unconfirmed).
- Verdict: keep. See section 2 for how to get parallelism out of it.

### Catch2 v3
- What: the other popular header/library framework, richer features (generators, matchers, benchmarks).
- Latest release: v3.16.1, 2026-10-08 (https://api.github.com/repos/catchorg/Catch2/releases/latest).
- MSVC/Windows: yes (widely used; I did not fetch a specific MSVC statement, so treat as likely, not confirmed here).
- Parallel: has `--shard-count` and `--shard-index` built in, and a CMake helper `catch_add_sharded_tests` (marked proof of concept) (https://raw.githubusercontent.com/catchorg/Catch2/devel/docs/command-line.md, https://raw.githubusercontent.com/catchorg/Catch2/devel/docs/cmake-integration.md). `catch_discover_tests` has a discovery-mode option.
- Cost: rewrite of the test macros and assertion style across 78 files. Compile time is heavier than doctest (doctest author's claim above).
- Verdict: skip. Sharding is nice, but you can get the same with doctest's ranges.

### GoogleTest / GoogleMock
- What: the most widely used C++ framework, with mocking.
- Latest release: v1.18.0, 2026-08-10, needs C++17 (https://api.github.com/repos/google/googletest/releases/latest).
- MSVC/Windows: yes, supported platform.
- Parallel: sharding by environment variables `GTEST_TOTAL_SHARDS` and `GTEST_SHARD_INDEX` (https://raw.githubusercontent.com/google/googletest/main/docs/advanced.md). XML output via `--gtest_output`.
- Cost: full rewrite; mocking is a style Hydra does not seem to need (it tests pure logic and golden files).
- Verdict: skip.

### Boost.Test
- What: Boost's framework. Latest Boost release 1.92.0, 2026-08-12 (https://api.github.com/repos/boostorg/boost/releases/latest).
- MSVC/Windows: yes. But it pulls in Boost; heavy for a project that does not use it today (I did not check for Boost use; unconfirmed).
- Verdict: skip.

### Boost.UT (ut)
- What: modern C++20 single-header framework.
- Latest release: v2.3.1, 2025-04-02 (https://api.github.com/repos/boost-ext/ut/releases/latest). About 18 months old. Slowest release pace of the group.
- MSVC: unconfirmed.
- Verdict: skip.

---

## 2. Running the existing doctest exe faster

Plain version: one process runs ~1,250 cases one at a time. Run the same exe several times at once, each on a slice, and the wall clock shrinks. No library change needed.

| Option | What it is | Cost | Notes |
|---|---|---|---|
| A. `ctest -j` with a few slices | Register N CTest entries, each `hydra_tests --first=a --last=b` or `-sf=` for a file group; run `ctest -j N` | Low: a few CMake lines | Slices by number shift when tests are added. Slicing by source file (`-sf=`) is more stable. |
| B. `doctest_discover_tests` | One CTest entry per test case; `ctest -j` | Low to write; high at run time? | 1,250 process starts. Process start cost of `hydra_tests` is not measured (unconfirmed). Gives per-case timing and `ctest --rerun-failed` for free. |
| C. GitHub Actions matrix | Run slices on separate runner jobs | Medium | Each job pays a build; only worth it if tests are slower than the build. Cache or upload the built exe as an artifact. |

Sources: doctest command-line and FAQ (links above). CTest runs each registered test as its own process and `-j` runs processes in parallel (general CTest behaviour; my search could not find a primary source for doctest + `ctest -j` specifically, so treat the combination as unconfirmed until tried).

Risk to check first: tests that share a temp folder, a SQLite file, the audio device, or a fixed path will collide when run side by side. The CI line that excludes the audio-device test suggests at least one such case exists. Needs a scan before turning on parallel runs (inference from the repo, not confirmed).

Also useful for free: `ctest --rerun-failed --repeat until-pass:3` (CMake 3.17 and later) is used by Eigen's CI to retry failures (https://eigen.googlesource.com/mirror/+/c631f232700d6ea8b2c7a7d89b8270e2ac1ce71e/ci/scripts/test.linux.script.sh).

### Test impact analysis (run only tests affected by a change)
- CTest has no built-in way to do this. A 2017 CMake mailing-list question about it has no answer in the results (https://cmake.org/pipermail/cmake/2017-April/065383.html).
- Found tools: Ratchet (2011 research prototype, https://tdl-ir.tdl.org/handle/2152/ETD-UT-2011-05-3306), O3DE's in-house framework (proposal, https://github.com/o3de/sig-testing/issues/53), Parasoft (commercial, vendor claim of 70% less run time, https://www.parasoft.com/webinar/run-tests-on-modified-code-only-in-your-ci-cd-pipeline/). None is a drop-in for this project.
- DIY idea: once coverage exists (section 6), record which test files touch which source files, then pick tests from `git diff`. Always keep a full run on `main`.
- Verdict: skip for now. Revisit only if CI time (about 10 minutes) becomes a real problem after sharding.

---

## 3. Golden-file / approval testing

### ApprovalTests.cpp
- What: a library that compares output to a saved "approved" file and opens a diff tool when they differ.
- Latest release: v.10.13.0, 2024-03-12 (https://api.github.com/repos/approvals/ApprovalTests.cpp/releases/latest). No release in about 2.5 years.
- Frameworks supported include doctest, Catch2, GoogleTest, Boost.Test (https://github.com/approvals/ApprovalTests.cpp).
- Windows: listed as supported; MSVC by name not mentioned on the page, CI badges show Cygwin and MinGW (same URL). Partly confirmed.
- Buys: nicer diffs and an approve workflow. Hydra already has golden tests over `testdata/input` (I did not read how they are written).
- Cost: moderate; migrating existing golden tests gives little new coverage.
- Verdict: skip. Only worth a look if the current golden-test update flow is painful.

Snapshot libraries other than this: none found worth listing in my searches.

---

## 4. Property-based testing and fuzzing

Plain version: instead of checking a few hand-picked inputs, the machine invents thousands of inputs and checks a rule that must always hold.

### RapidCheck
- What: C++ property-based testing (QuickCheck style) with shrinking (cuts a failing input down to a small one) and stateful testing.
- Latest release: none published; repo is active, last commit 2026-10-05 (https://api.github.com/repos/emil-e/rapidcheck/commits?per_page=1, https://github.com/emil-e/rapidcheck).
- MSVC: README states CI on Visual Studio 2015 and says later versions should work (https://github.com/emil-e/rapidcheck). Not verified with VS 2026.
- Framework glue: README names Boost Test and GoogleTest/GoogleMock, not doctest. You can still call `rc::check(...)` inside a doctest case and assert its boolean result (this usage is my inference, unconfirmed).
- Buys: Hydra has rules that should hold for any chart, for example "the best path score is never below a valid path's score", "a parser round trip returns the same chart", "timing values stay in range". Good for scoring and path code and for parsers.
- Cost: low to medium; header and CMake add_subdirectory, a few test files.
- Verdict: try, on one or two invariants.

### libFuzzer through MSVC (`/fsanitize=fuzzer`)
- What: the compiler builds a fuzz target that feeds mutated bytes to a function, guided by code coverage. Microsoft marks it experimental.
- Support: from Visual Studio 2022 17.0; recommended together with `/fsanitize=address`; the two flags cannot be combined into one comma list (https://learn.microsoft.com/en-us/cpp/build/reference/fsanitize). Works with static and dynamic CRT.
- Buys: Hydra reads untrusted file formats: .chart, .mid, .sng, .srb, audio, images (tests exist for each in `tests/`). Fuzzing those parsers can find crashes and hangs that a corpus of 115 good files never will.
- Cost: low per target: one small `LLVMFuzzerTestOneInput(const uint8_t*, size_t)` function per parser, a CMake target, seed it from `testdata/input`. Run for a few minutes on a schedule, not on every push.
- Verdict: adopt for the parsers.

### Catch2 generators
Needs Catch2; skip (see section 1).

---

## 5. Mutation testing for C++ (does a tool exist that works with MSVC?)

Plain version: a mutation tool changes the code in small ways (for example `<` to `<=`) and checks that some test fails. If no test fails, the tests are weak there. Short answer: none works with an MSVC-only setup today.

| Tool | Latest | MSVC/Windows | Evidence |
|---|---|---|---|
| Mull | 0.34.1, 2026-09-13 (https://api.github.com/repos/mull-project/mull/releases/latest) | No. Release assets are Ubuntu, Debian, macOS, RHEL packages only (same URL). The Windows request (issue #1039) is open (https://github.com/mull-project/mull/issues?q=windows). Mull works on LLVM IR, so MSVC binaries are out. |
| Dredd | Active; last commit 2026-09-26 (https://api.github.com/repos/mc-imperial/dredd/commits?per_page=1) | Unconfirmed. It rewrites source with Clang tooling from a `compile_commands.json`, so it could in theory mutate source that MSVC then compiles. Docs and build steps are Linux (Ubuntu 24.04, LLVM 17) (https://github.com/mc-imperial/dredd). No Windows or clang-cl evidence found (https://github.com/mc-imperial/dredd). |
| Dextool mutate | Unconfirmed date (no releases listed) | Unconfirmed. Docs show Ubuntu and Fedora Docker files, needs LDC compiler and about 16 GB RAM to build (https://github.com/joakim-brannstrom/dextool). |
| mutate_cpp, pmat | Not fetched | pmat lists MSVC in prerequisites but all examples are Linux and macOS (https://docs.rs/crate/pmat/3.29.0/source/docs/features/CPP-MUTATION-TESTING.md). Unconfirmed. |

What would work: run Mull or Dredd in a Linux CI job (or WSL) on the pure-logic part of Hydra (scoring, path search, parsers) built with Clang. This needs those sources to build with Clang on Linux, which has not been checked; Hydra is Windows only today, so expect porting work. Verdict: skip for now. If you want the effect cheaply, try Dredd on a few core files in WSL as an experiment, not as CI.

---

## 6. Code coverage on Windows/MSVC

Plain version: coverage shows which lines the tests ran. It is a map of blind spots, not a quality score.

### Microsoft.CodeCoverage.Console (plus `dotnet-coverage` for conversion)
- What: Microsoft's command-line coverage collector. Handles native C++ by instrumenting the binary on disk. MSVC only.
- Docs updated 2026-10-08 (https://learn.microsoft.com/en-us/visualstudio/test/microsoft-code-coverage-console-tool).
- Edition: for VS 2022 it needed Enterprise; the page for the current VS says Enterprise, Community and Professional all have it (same URL). The GitHub Windows runner image has Visual Studio installed; which edition is on it: unconfirmed.
- Needs the exe linked with `/PROFILE` (same URL). Commands: `instrument`, `collect`.
- Output: native `.coverage` file. `dotnet-coverage merge -f cobertura` converts `.coverage` to Cobertura XML (https://learn.microsoft.com/en-us/dotnet/core/additional-tools/dotnet-coverage). Whether that merge handles native C++ `.coverage` files is unconfirmed; the doc shows it only for .NET. Test it before relying on it.
- Codecov accepts Cobertura XML (https://docs.codecov.com/docs/supported-report-formats).
- Cost: low to medium; a separate build config with `/PROFILE`, one CI job. Instrumented runs are slower (size not measured).
- Verdict: try.

### OpenCppCoverage
- What: the long-time free Windows coverage tool.
- Latest release: release-0.9.9.0, 2020-05-21 (https://api.github.com/repos/OpenCppCoverage/OpenCppCoverage/releases/latest). The repository is now archived: the owner says no more releases, bug fixes or compatibility updates (https://github.com/OpenCppCoverage/OpenCppCoverage). README names Visual Studio 2008 or later, no mention of VS 2022 or 2026.
- Verdict: skip (archived).

### clang-cl + llvm-cov
- What: Clang's source-based coverage, readable by `llvm-cov` and exportable as LCOV.
- Windows: clang-cl `--coverage` has worked since Clang 6 per a Mozilla write-up, and reports go through `grcov` to Codecov; the profile runtime library may be missing from official Windows binaries (https://andreasfertig.com/blog/2020/12/cpp-insights-code-coverage-on-windows/). I found no complete clang-cl + `-fprofile-instr-generate` + Codecov example (search result).
- Cost: high. It means a second compiler for the whole project and its dependencies (ImGui, SQLite, miniaudio, mimalloc).
- Verdict: skip.

---

## 7. Sanitizers on MSVC

### AddressSanitizer (`/fsanitize=address`)
- What: catches memory bugs at run time: out-of-bounds reads and writes, use after free, double free, stack use after scope, container overflow in the MSVC STL.
- Status: supported since VS 2019 16.9; x86 and x64 on Windows 10 and later (https://learn.microsoft.com/en-us/cpp/sanitizers/asan). Docs updated 2026-07-09 (same page). Works with `/MD` and `/MT`, any optimization level (same page).
- Not compatible with: `/RTC` runtime checks, incremental linking, edit-and-continue, OpenMP, coroutines (https://learn.microsoft.com/en-us/cpp/sanitizers/asan-known-issues). Thread-local variables are not protected (same page).
- Not available on MSVC: leak, undefined-behaviour, thread, memory sanitizers (the page lists these as requests only) (https://learn.microsoft.com/en-us/cpp/sanitizers/asan).
- Heap-replacing libraries: the docs warn that a library that overrides `operator new` and `delete` (their example is MFC) can make ASan miss or misclassify errors (https://learn.microsoft.com/en-us/cpp/sanitizers/asan-known-issues). Hydra links mimalloc, which redirects allocations (from the project notes, not a web source). Whether ASan and mimalloc clash: unconfirmed. Expect to turn mimalloc off in the sanitizer build.
- Runtime DLLs ship with the compiler; the build needs the "C++ AddressSanitizer" Visual Studio component (same page). The GitHub runner image having it: unconfirmed.
- Buys: the project has many parsers, a custom store, and an app with threads. This is the one tool most likely to find a real bug nobody has seen.
- Cost: low. One extra CMake config or option, one CI job that runs `hydra_tests` (the doctest exe) under ASan. Runs slower and uses more memory (overhead not measured).
- Verdict: adopt.

---

## 8. Benchmarks and performance regression

### Google Benchmark
- Latest release: v1.9.5, 2026-01-21 (https://api.github.com/repos/google/benchmark/releases/latest). Needs C++17 to build (https://github.com/google/benchmark).
- MSVC: the README does not list MSVC or a Windows CI by name; an `appveyor.yml` is in the repo (https://github.com/google/benchmark). Treat as likely but unconfirmed here.
- Compare tool: `compare.py` compares two runs and runs a statistical test (needs about 9 repeats). It has no pass or fail gate or CI mode (https://raw.githubusercontent.com/google/benchmark/main/docs/tools.md).
- Cost: medium.
- Verdict: skip for now. The project already has timing scripts and notes (project memory mentions `hydra_bench` and perf waves; I did not inspect them).

### nanobench
- Latest release: v4.6.0, 2026-08-14 (https://api.github.com/repos/martinus/nanobench/releases/latest). Single header, MIT licence (https://github.com/martinus/nanobench). MSVC support not stated on the page (unconfirmed).
- Verdict: skip. Only if you want in-test microbenchmarks.

### github-action-benchmark
- What: GitHub Action that stores benchmark results and alerts when a result gets worse by a set percent. Supports Google Benchmark and Catch2 output (https://github.com/benchmark-action/github-action-benchmark).
- Caveats: default alert threshold 200%; does not comment on PRs (listed as future work); all examples use Linux runners, Windows unconfirmed (same URL). Shared GitHub runners are noisy, so small regressions will not be reliable.
- Verdict: skip.

---

## 9. GUI testing

### Dear ImGui Test Engine (already in use)
- What: automation and test library for Dear ImGui, drives widgets by label.
- Latest activity: last commit 2026-10-08 (https://api.github.com/repos/ocornut/imgui_test_engine/commits?per_page=1).
- Licence (v1.04, https://raw.githubusercontent.com/ocornut/imgui_test_engine/main/imgui_test_engine/LICENSE.txt): free for individuals, non-legal entities and non-profits, educational use, software released under an OSI-approved open-source licence, and legal entities with turnover under 2 million USD. Otherwise a 45-day trial, then a paid licence from the seller named in the licence. Copyright notice and licence text must be kept with copies. Hydra is a personal repo (`splax77/hydra-deluxe`) and looks free-eligible. If it is ever sold or run by a company over the turnover line, re-read the licence.
- Features seen in the wiki: JUnit XML export via `-export-file` and `-export-format`, screen and video capture, a context API (https://github.com/ocornut/imgui_test_engine/wiki/Exporting-Results). Headless mode and the performance tool were not covered by the pages I could fetch (unconfirmed).
- Cost: nothing new.
- Verdict: keep. Possible small gain: export JUnit XML from `hydra_uitest` so the CI summary can list failures by name (needs a hand-written reporter step; no source for built-in GitHub support).

Alternatives (Windows UI Automation tools, WinAppDriver and similar) were not researched; they do not see inside an ImGui window anyway (my reasoning: ImGui draws everything on one surface with no native controls).

---

## 10. Python and PowerShell

### pytest + Hypothesis
- pytest is already used. Hypothesis (property-based testing for Python) latest release v6.168.5, 2026-10-05 (https://api.github.com/repos/HypothesisWorks/hypothesis/releases?per_page=2). Very active.
- Buys: stronger tests for the Python tools (parsers, scoring checkers). Cost: low, one pip package per tool test file.
- Verdict: try, on one tool.
- pytest-xdist (run tests in parallel): v3.8.0, 2025-07-01 (https://api.github.com/repos/pytest-dev/pytest-xdist/releases/latest). Only worth it if the Python test run is slow. Verdict: skip unless needed.

### Pester (PowerShell)
- What: the standard PowerShell test framework: `Describe`, `It`, `Mock`, JUnit output.
- Latest: 6.2.0, 2026-09-09; 5.9.1 is the last 5.x as shown on the PowerShell Gallery (https://api.github.com/repos/pester/Pester/releases?per_page=3, https://www.powershellgallery.com/packages/Pester/5.9.1). Pester 6 is newer and has a major version change; breaking changes not reviewed (unconfirmed). Pester 5 ships in the Windows runner image? Unconfirmed.
- Buys: the hooks are the safety system for the whole agent workflow. A custom harness works, but Pester gives `Mock` (fake commands such as `git`), tags, `-Output`, and JUnit for CI.
- Cost: medium. Hook tests would need porting. Only new tests need to use it; old ones can stay.
- Verdict: try on new hook tests only. Not a priority; the current harness works.

---

## 11. Flaky-test tooling

- Built-in CTest: `--repeat until-pass:N` (retry), `--repeat until-fail:N` (stress test), `--rerun-failed` (https://eigen.googlesource.com/mirror/+/c631f232700d6ea8b2c7a7d89b8270e2ac1ce71e/ci/scripts/test.linux.script.sh, https://gitlab.kitware.com/cmake/cmake/merge_requests/203). This works once tests are registered as separate CTest entries (section 2).
- Practice from the search: keep retries out of the main pipeline or make them visible; give quarantined tests an expiry date; log pass-on-retry as a warning (https://tenki.cloud/blog/flaky-test-quarantine-github-actions, https://testdino.com/blog/flaky-test-github-actions). These are vendor blogs; treat as opinion.
- SaaS flaky-test dashboards exist (Codecov Test Analytics and others). I did not research them (unconfirmed).
- Hydra rule fit: the project's own rule says to force the repro, not loop and hope (brief preamble). A retry-until-pass hides flakes, so use retries for reporting only.
- Verdict: adopt the free CTest options after sharding; skip paid tools.

---

## Ranked top 5

1. **Shard `hydra_tests` with CTest** (adopt). Register 4 to 8 slices by source file (or per-case via `doctest_discover_tests`), run `ctest -j`. Free, no new library. Check for shared temp/DB/audio state first.
2. **AddressSanitizer CI job** (adopt). Microsoft-supported on MSVC, catches memory bugs the tests do not see. Expect to disable mimalloc in that build.
3. **Fuzz the file parsers with `/fsanitize=fuzzer`** (adopt, scheduled run). Hydra reads untrusted chart, MIDI, SNG, SRB, audio and image files; a 115-chart corpus cannot cover bad input.
4. **Coverage with Microsoft.CodeCoverage.Console to Codecov** (try). Gives a map of blind spots; confirm that the native `.coverage` file converts to Cobertura before investing.
5. **Pester 5 or 6 for new hook tests, plus RapidCheck/Hypothesis on one invariant each** (try, small). Pester for the hook scripts; RapidCheck for scoring/path invariants; Hypothesis for one Python tool.

Rationale: the biggest, cheapest gain is turning the single serial doctest run into several parallel ones, because it needs no new dependency and also unlocks `ctest --rerun-failed` and per-test timing. The next two items add checks of a different kind from what exists: AddressSanitizer finds memory bugs in code that already passes its tests, and fuzzing hunts for crashes on malformed input files, which matters for an app that reads files users download. Coverage is last in the adopt/try block because it only informs; it does not find bugs. Switching framework (Catch2, GoogleTest), approval libraries, clang-cl coverage and mutation testing cost more than they return right now. Mutation testing in particular has no tool that works with MSVC (Mull has no Windows build; Dredd and Dextool are unconfirmed on Windows).

## Things I could not confirm
- Whether the GitHub Windows runner has the ASan component and the coverage tool's edition.
- Whether `dotnet-coverage merge -f cobertura` accepts native C++ `.coverage` files.
- doctest discovery mode (build-time vs test-time) and process start cost of `hydra_tests`.
- ASan with mimalloc.
- MSVC support statements for Catch2, Google Benchmark and nanobench (not fetched or not stated on the pages).
- ImGui Test Engine headless mode and perf-tool details (wiki pages not available to me).
