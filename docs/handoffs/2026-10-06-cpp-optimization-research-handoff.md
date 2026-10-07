# C++ and compiler optimization research (2026-10-06)

This session researched C++ code and compiler speed advice on the web. It then checked our build setup and busiest code paths against that advice. It also read the notes from the other session's six `perf-*` experiments, so it would not repeat them. Nothing was edited, built or run. This file is the only output.

## The short version

The other session's experiments already cover almost all of the code-level advice. That includes grouping database writes into fewer transactions, caching prepared statements, cutting allocations in the parsers and the graph build, and the Windows segment heap.

What nobody has touched is the compiler. Our Release build uses CMake's stock flags (`/O2 /Ob2 /DNDEBUG`). It has no link-time optimization, no profile-guided optimization and no symbols for profiling.

The whole-library batch is limited by the database writer, not by analysis. So compiler flags that speed up analysis will barely change batch time until the writer fix lands. They would make single-chart analysis in the GUI faster sooner.

## What the build does today

The facts below come from reading `CMakeLists.txt`, `CMakePresets.json`, `build_cpp.ps1`, `installer/build_installer.ps1` and `.github/workflows/ci.yml`.

The build is MSVC only, C++17, with the Visual Studio 2026 generator. All three presets build Release. The installer builds the `ship` preset in Release and stages it with `cmake --install`.

The only global flags are `/MP` and `/d1trimfile` (`CMakeLists.txt:38`, `:45`). Our own targets add `/W4 /WX /EHsc /utf-8` (`:55-57`).

None of these appear anywhere in the repo: `/GL`, `/LTCG`, `INTERPROCEDURAL_OPTIMIZATION`, `/arch`, `/fp`, PGO, precompiled headers, unity build, `/Zi`, a runtime-library choice, `/guard:cf` or `/Qspectre`. That means the defaults apply: dynamic CRT (`/MD`), `/fp:precise` and the SSE2 baseline. The installer bundles the VC++ redistributable to match the dynamic CRT.

SQLite is the 3.46.0 amalgamation, built with `/W1` and no `SQLITE_*` options (`CMakeLists.txt:94-96`). At runtime it opens in WAL mode with `synchronous=NORMAL` (`record_store.cpp:725-726`). Each chart gets its own transaction (`record_store.cpp:1128`). Statements are prepared fresh on every call, apart from the per-node inserts inside one row.

## What the other session already found

These numbers are from its notes in `...\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\`. It checked each change against the current build and found the results identical.

- **Database writer:** a whole-library batch takes about 17 s, and it is limited by the one thread that writes to the database. Grouping 16 charts per transaction, caching prepared statements and changing how often SQLite checkpoints its log cut it from 16.3 s to 11.5 s.
- **Parsing:** went from 15.0 s to 8.85 s single-threaded, and the dynamics count from 3.85 s to 0.43 s.
- **Graph build:** went from 13.6 s to 2.74 s. Part of the fix stops `category_scores` creating a new array for every chord.
- **Folder scan:** went from 4.2 s to 2.06 s.
- **Search algorithm:** a shortcut that skips worse-looking paths gives wrong results (3 songs lost their best score). It was rejected.

Its CPU profile could not name much of the time: it fell in unnamed Windows code, probably the memory allocator and page faults.

## Recommendations, in order

**1. Link-time optimization (`/GL` + `/LTCG`).** Today the compiler optimizes each .cpp file on its own. With link-time optimization, the linker optimizes the whole program at once. That lets it inline a function from one file into another.

In CMake it's one setting, `CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE`, guarded by `check_ipo_supported()`. The costs are slower links and a slightly slower warm build. Microsoft gives no general gain figure.

The risk to identical results is low, because the floating-point rules stay `/fp:precise`. Prove it anyway with the whole-library hash and the database comparison the perf session already built. Source: https://learn.microsoft.com/en-us/cpp/build/reference/ltcg-link-time-code-generation

**2. Symbols in Release (`/Zi`, `/DEBUG /OPT:REF /OPT:ICF`).** These write a symbol file (a PDB) next to the exe and don't slow the program down. They let VSDiagnostics name the functions where time goes, instead of showing unnamed Windows code.

`/DEBUG` turns off the linker's dead-code and duplicate-code trimming by default. That's why `/OPT:REF /OPT:ICF` go back in by hand. The perf session's profiling worktree already does this as a throwaway experiment. The PDB need not ship.

**3. mimalloc, a faster memory allocator from Microsoft Research.** It replaces the standard memory allocator. Microsoft Research reports almost 4× the allocation throughput of the Windows allocator in a heavily multi-threaded test. That fits the unnamed Windows time in the profile. The perf session tried the segment heap but not mimalloc.

There are two catches. First, it is a new `third_party` dependency, which needs the user's OK. Second, the clean static route needs the static CRT (`/MT`), and we use `/MD`. The alternative route ships a redirect DLL beside the exe. Sources: https://github.com/microsoft/mimalloc and https://microsoft.com/en-us/research/blog/mimalloc-a-high-performance-scalable-memory-allocator-for-the-modern-era

**4. SQLite compile options.** sqlite.org says its recommended set saves about 5% of SQLite's CPU time and calls that "not a huge difference". Safe ones for us:
- `SQLITE_DEFAULT_MEMSTATUS=0` turns off SQLite's memory-usage tracking.
- `SQLITE_OMIT_SHARED_CACHE` drops a feature we don't use.
- `SQLITE_DEFAULT_WAL_SYNCHRONOUS=1` makes WAL-mode commits faster.

Do not use `SQLITE_THREADSAFE=0`, because the batch runs several threads. These options are cheap but small next to transaction grouping. Source: https://www.sqlite.org/compile.html

**5. Profile-guided optimization, later.** The build records a training run (the library batch), then rebuilds using that profile. Reported gains are 5–20%; Photoshop measured 20% using sample-based PGO together with link-time optimization. The cost is a more complicated build, plus profile files that go stale as the code changes. Only worth it after items 1–3 and the writer fix are in. Sources: https://learn.microsoft.com/en-us/cpp/build/profile-guided-optimizations and https://devblogs.microsoft.com/cppblog/boosting-adobe-photoshops-performance-with-msvc-and-spgo/

## What to leave alone, and why

`/fp:fast` and `/fp:contract` let the compiler reorder or fuse floating-point math. `core/timing.h:9-11` says stored records depend on that math being bit-for-bit identical, and the star cutoffs need exact float32 math. Either flag would silently move scores.

`/arch:AVX2` would crash the app on CPUs from before about 2013. Our hot code is integer and graph work, so it gains little from vector instructions. Microsoft also notes that AVX2 builds can round differently from the default build.

clang-cl is not installed. Lemire measured it up to 40% faster on one text-conversion test, and other reports go both ways. It is a possible later experiment, and it would need `-ffp-contract=off` to keep results identical.

Keep the stack buffer checks (`/GS`) on, because the app reads chart files downloaded from the internet. `/Qspectre` and `/guard:cf` are security features that slow code down. Iterator debugging is already off in Release.

A third-party hash map is not needed. The engine already has its own (StampMap in `engine.cpp:342`), and the `.chart` parser is being rewritten in the parse experiment.

## Small leads: checked, and not worth doing

This section first listed three small leads. A later session ("C++ optimization research leads") measured all three. None is worth doing as first written, but the check turned up a cheap build fix.

`read_song_ini_keys` (`analysis.cpp:285-325`) is not slow. Parsing all 19,352 song.ini files in the library takes 0.11 s of CPU in total, and a string_view version takes 0.06 s, with identical results. The perf profile's 1.8 s of "ini" worker time is spent opening and reading the files, not parsing them.

`word_stoi` (`song.cpp:1066`) is already handled. The perf session's parse patch adds `lean_stoi`, `lean_stoll` and `lean_try_parse_int`. The last one also skips the exception that `try_parse_int` throws for every named key in a `.chart` `[Song]` section, which this handoff had missed. The temporary string never allocated anyway, because tick numbers fit in the small-string buffer.

Precompiled headers would save little. Common headers cost about 0.75 s per file. `sqlite3.c` alone takes about 8 s to compile, and precompiled headers can't help it.

The cheap fix: `build_cpp.ps1:45` runs `cmake --build` without `--parallel`, and no preset sets a job count. So MSBuild builds one project at a time, with `/MP` only inside each project. Two cold builds of the full target list took 83 s as today and 60 s with `--parallel`, about 28% faster, with all 8 exes built both times. That is one run each, so treat the numbers as rough. The user has not decided yet whether to add it.

Those cold builds also mean the "3-6 minute cold build" in the brief preamble did not reproduce on an idle machine. That figure came from runs where several builds shared the machine.

One unconfirmed observation: the first read of all song.ini files took 44 s and the second took 0.9 s. It could be a cold disk cache or Defender scanning files on first open; the cause wasn't found. If it is real, a user's first scan is bound by the disk, and no compiler flag helps it.

The perf session's six worktrees have since been removed. Its changes now live only as patch files in its scratch folder, under `...\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\`.

## Suggested next steps

1. Merge the writer fix first, because the writer is the bottleneck.
2. Try link-time optimization and Release symbols together in one worktree. Measure with the perf session's harnesses on a copy of the real db, and prove the results are identical.
3. Take a named profile.
4. Decide on mimalloc from that profile. It needs the user's OK as a new dependency.

## Journal state at handoff

The hook's journal listed these lines. The completion notices for all three agents arrived before this file was written, and each one handed back its final report.

```
agent-a11872f0ebfba8374: unfinished, last tool call SubagentHandback, touched 20:41
agent-a52ede326a976f03c: unfinished, last tool call SubagentHandback, touched 20:40
agent-ac26cf06ee631d614: unfinished, last tool call SubagentHandback, touched 20:41
```

Nothing is in flight. No code was changed and nothing was committed.
