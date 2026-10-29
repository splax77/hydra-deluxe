# Hydra Deluxe for developers and power users

This page holds everything the README leaves out: the command line tools in
full, how to build from source, and the developer tools. For using the app,
see the [User Guide](UserGuide.md).

## Command line tools

Two console tools ship next to the app. They share its settings, rules and
database: they read the same `hydra_settings.ini`, `hydra_rules.ini` and
`hydra.db` beside the executable. The path report has no command-line tool;
it lives only in the app (D103 item 5, ADR 0027).

```
hydra_batch --whole-library    Analyze every chart folder from the app's settings
hydra_batch <folder> [...]     ...or specific folders instead (the two don't mix)
hydra_batch --redo             Re-analyze charts already stored
hydra_batch --db <path>        Target a specific database
hydra_batch --rules <path>     Take the rule choices from this file, not hydra_rules.ini
hydra_batch --legacy-fills     Score fills by Clone Hero 1.0's rule

hydra_fillcompare              Compare Clone Hero 1.0 and 1.1 fill results, chart by chart
hydra_fillcompare --db <path>  Compare the results in a specific database
hydra_fillcompare ... --out fill_compare.html
hydra_fillcompare ... --rules <path>
hydra_fillcompare ... --no-open
```

`hydra_batch` with no folder and no `--whole-library` prints a sentence and
stops with exit code 2, so a bare run can't start a library-wide analysis by
accident. The other `hydra_batch` flags in the list go with either form.

Both read the app's settings file, so they work at the same SP cap, timing
limit, score range and chart mode the app is set to. The fill rule is the
exception, below. Both read the scoring rules from `hydra_rules.ini` next to
Hydra.exe, or from the file `--rules` names. If that file has an error, they
print it and stop with exit code 2.

Both refuse a library file from a Hydra version older than 2.2.0, print one
sentence saying so, and leave the file as it is. Opening such a file in the
app sets it aside and starts a fresh library. A file from 2.2.0 to 2.7.0 is
converted on its first open, by the app or by either tool. ADR 0031 has both
rules.

Clone Hero 1.1 changed when a drum fill appears. The app's **1.0 fills**
setting scores by the older 1.0 rule instead. Each result remembers which rule
made it, so a song's 1.0 and 1.1 results sit side by side in `hydra.db`.

`hydra_batch --legacy-fills` does the same from the command line. It ignores
the app's 1.0 fills setting and goes by the flag alone. Its results go into the
same database as any other run, the app's own `hydra.db` unless `--db` names
another, beside the 1.1 results.

To see what the rule change did, compare the two. `hydra_fillcompare` reads a
chart's 1.0 result and its 1.1 result from one database: the app's own
`hydra.db`, or the file `--db` names. A chart with a result under only one
rule still gets a row, labelled by that rule. Narrow columns name the rules
CH 1.0 and CH 1.1. Like the path report, it compares library charts only. A
database built by `hydra_batch` with folder arguments has no chart library,
so it stops with the path report's sentence. D92 and
`report::lacks_chart_library` own that rule, and ADR 0026 explains it. The
dmleaderboards comparison follows it too. To fill a database with both rules'
results, run `hydra_batch` once plain and once with `--legacy-fills`:

```
hydra_batch --whole-library
hydra_batch --whole-library --legacy-fills
hydra_fillcompare

hydra_batch --whole-library --db both.db
hydra_batch --whole-library --legacy-fills --db both.db
hydra_fillcompare --db both.db
```

## Building from source

Hydra Deluxe is a native Windows app: C++17, built with CMake and MSVC (Visual
Studio's "Desktop development with C++" workload is all it needs — the build
script finds the VS-bundled CMake itself). Third-party code is vendored under
`third_party/` and built from source. `THIRD_PARTY_NOTICES.txt` at the repo
root lists every library that goes into the shipped programs, with its
licence, and ships beside them. doctest, the test framework, is vendored for
the tests only.

```
.\build_cpp.ps1              # configure + build everything (Release), dev tools too
.\build_cpp.ps1 -Preset ship -Package   # zip a release without the GUI tests
.\build_cpp.ps1 -Target hydra_tests
ctest --test-dir build-cpp -C Release -j 4 -R hydra_tests --output-on-failure   # run the test suite in its slices (tests/shards.txt)
```

To build the Windows installer (needs Inno Setup 6:
`winget install -e --id JRSoftware.InnoSetup`):

```
.\installer\build_installer.ps1        # -> build-cpp\installer\HydraDeluxe-<ver>-setup.exe
```

It builds Release with the `ship` preset (in `build-ship\`, without the
attached GUI tests), stages the ship list via `cmake --install` (so stray user
data in the build tree can never leak into a release), downloads and caches
the VC++ redistributable, and compiles `installer\hydra.iss`.

### Libraries

Apart from the programs' own source files (src/ui/main.cpp and src/cli/), the
code under src/ is built as twelve static libraries, and each program links
only the ones it needs. `CMakeLists.txt` is the one place that says which library a
file is in (its source lists) and which libraries each one may use (its link
lines). Lowest first: `hydra_core` (the chart model, `Song`, scoring, replay
and timing) and `hydra_image` link nothing of Hydra's. `hydra_parse` and
`hydra_search` link `hydra_core`, and so does `hydra_store`, which also links
`hydra_search` privately. `hydra_app` links those three and is the top of the
logic, with no GUI in it; `hydra_batch` and `hydra_fillcompare` link it alone.
`hydra_audio`, `hydra_render` and `hydra_dm` link `hydra_app`. `hydra_preview`
links `hydra_audio` and `hydra_render`, and `hydra_uibase` (the GUI base:
ImGui set-up, theme, fonts, widgets) links them too. `hydra_ui`, Hydra's own
window, links `hydra_uibase`, `hydra_preview` and `hydra_dm`. Files stay in
their folders, so src/ui/ holds files of three libraries and src/app/ of two.

The layering test, tests/test_layering.cpp, reads those lists and fails when a
file includes a header from a library its own does not link. A wrong-way
include gets fixed, not parked: the test keeps tests/layering/known_gaps.json
empty. A third-party header needs one row in tests/layering/externals.json.
ADR 0032 records the design.

### What a Release build does

A Release build is tuned for speed in ways that never change a stored result.
The settings live in `CMakeLists.txt` and `build_cpp.ps1`; this says why.

`build_cpp.ps1` asks MSBuild to build independent projects at the same time
(`--parallel`, uncapped, decision D86 item 5). Each project also compiles its
own files in parallel, as before.

Link-time optimization is on only in the installer's build, the `ship`
preset in `build-ship\`. (Link-time optimization means the linker sees the whole
program at once, so it can inline across source files.) It makes the program a
little faster. It shipped only after a whole-library run stored byte-identical
results with it (see "Proving identical results"). The everyday build in
`build-cpp\` leaves it off, as every build did before 2026-10-06, and configure
prints which. So a timing run that should match what ships uses the exes from
`.\build_cpp.ps1 -Preset ship`, not from `build-cpp\`. The HYDRA_LTCG cache
switch in `CMakeLists.txt` decides it, and `build_cpp.ps1` always configures the
`ship` preset, so the installer's build can never miss that switch.

The everyday build leaves it off because both ways of doing it there cost too
much. The incremental kind (`/LTCG:incremental`) is what CMake's Visual Studio
generator gets by default: CMake only switches link-time optimization on, and
MSBuild picks that kind (CMake issue 20484). It keeps a cache beside each exe
(`.iobj` and `.ipdb` files) and redoes only what it thinks changed. On
2026-10-07, its first day on, that cache went stale about seven times: links
failed with C1001 or LNK1000, or `hydra_tests` crashed right after a build that
said it passed. Deleting the cache fixed it every time. The exact trigger was
not pinned down; changing a shared header, then building one target, then all
of them, did not break it in three tries. A garbage `.ipdb` did break it, with
C1301 ("invalid format, please delete and rebuild"). Plain `/LTCG` never reads
the cache, but it redoes the whole program on every link. Timed after a
one-file change in `src\core`, with no other compiler running: the incremental
kind rebuilt every exe in 18.8 s and `hydra_tests` alone in 17.7 s; plain
`/LTCG` took 90.5 s and 56.7 s; with link-time optimization off, 4.6 s and
2.9 s.

The installer's build uses plain `/LTCG`, which `CMakeLists.txt` names on
purpose. MSBuild still names the cache files, so they still appear, but a plain
link with garbage in both of them succeeded; it rewrote the `.iobj` and left
the `.ipdb` alone. Nothing stale can reach a shipped exe.

Each Release exe gets a symbol file (`.pdb`) beside it, for profilers and
crash dumps. The exe records only the symbol file's name, never the build
folder's path; `tests/test_build_flags.cpp` checks that name. Debuggers, the
ASan symbolizer and the coverage tool all find the symbol file by that name
beside the exe. No symbol file ships: the install rules name only the exes and
DLLs, and the installer refuses a staging folder that holds one.

The vendored SQLite is built without its memory-use counters and without
shared cache. Hydra uses neither, and leaving them out trims a little of
SQLite's own work; `tests/test_store_open.cpp` checks both. SQLite's default sync
level for WAL mode is not set at build time, because the store sets it itself
on every open (`record_store.cpp`). The store also caps the write-ahead log
(`hydra.db-wal`) there, at `kJournalSizeLimitBytes` (D93; ADR 0026).

Every exe runs on Microsoft's mimalloc memory allocator instead of the
Windows heap, the tests and benchmarks included (decision D88). It cut Hydra's
own CPU work about 10% on a whole-library run, at the cost of more peak
memory; `docs/handoffs/2026-10-07-mimalloc-trial.md` has the numbers. It
comes as two DLLs, `mimalloc.dll` and `mimalloc-redirect.dll`, and an exe
won't start without both beside it. `hydra_use_mimalloc` at the bottom of
`CMakeLists.txt` owns which exes link it and how, and puts both DLLs in the
build folder; the install rules stage them beside the shipped exes.

Four speed flags stay out on purpose. `/fp:fast` and `/fp:contract` let the
compiler reorder or fuse floating-point math, which can move a computed timing
or score in its last bit, and every stored result must stay byte-identical.
`/arch:AVX2` would stop Hydra from starting on a processor without AVX2.
Turning off `/GS`, the compiler's stack-overrun check, gives up a safety check
for a gain too small to measure.

The tests run against the checked-in chart corpus under `testdata/input/`;
nothing else is needed. `hydra_tests` asserts structural invariants and
lossless round-trips over that corpus. GUI changes are checked headlessly with
`hydra_uitest`; see [agents/ui-testing.md](agents/ui-testing.md).

The Python and PowerShell tools have their own tests. Run them from the
repository root:

    py -m pytest tools/ch_probe/tests tools/test_compare_db.py tools/test_hydra_ui_mcp.py -q
    pwsh -NoProfile -File tools/test_mutation_probe.ps1

The pytest runs need no C++ build. The MCP test also needs
`py -m pip install mcp`, and its real-exe cases skip until
`hydra_uitest.exe` is built. The `tool-tests` job in
`.github/workflows/ci.yml` runs the pytest paths and the two other PowerShell
self-tests (`tools/test_ci_build_folder.ps1`, `tools/test_release_sign.ps1`);
it is the owner of that list. The mutation probe's self-test needs a
built `hydra_precheck.exe` (`.\build_cpp.ps1 -Target hydra_precheck`),
because the probe masks comments and strings with it. The derive-once
precheck's own self-test runs inside `hydra_tests`:

    build-cpp\Release\hydra_tests.exe "-sf=*test_precheck*"

Name the pytest paths rather than `tools/` as a whole: the scripts in
`tools/ch_probe/experiments/` look like tests to pytest and drive the real
game.

The program's name on screen is "Hydra Deluxe", but its files keep their older
names: `Hydra.exe`, `hydra.db`, `hydra_settings.ini`, `hydra_ui.ini` and the
`C:\Program Files\Hydra` install folder. That way an upgrade from an earlier
Hydra finds the user's library where it was. A library file from before 2.2.0
is set aside as `hydra.db.pre-2.2.0` rather than converted (ADR 0031). Older versions also saved
report pages in `Documents\Hydra`. Nothing reads or writes that folder any
more, and Hydra leaves the old pages there (ADR 0027).

### The AddressSanitizer build

AddressSanitizer (ASan) is a compiler check for memory bugs. It stops the
program at the first overrun or use after free and prints where it happened.
The `asan` preset builds with it, in `build-asan\`. It is for tests only and
never ships.

```
.\build_cpp.ps1 -Preset asan -Target hydra_tests
ctest --test-dir build-asan -C Release -j 4 -R hydra_tests --output-on-failure
```

The `asan-ninja` preset is the same build through Ninja and the sccache
compiler cache, in `build-asan-ninja\`. CI uses it, so a night with few
changes rebuilds from the cache.

Run these from a Visual Studio developer shell ("Developer PowerShell for VS").
The ASan runtime DLL sits beside the compiler, not beside the exes, and that
shell puts it on `PATH`. The CLI tests start the command line tools as child
processes, and they need it too.

This build runs on the Windows heap instead of mimalloc, because mimalloc's
redirect would hide the heap from ASan. The `HYDRA_ASAN` switch in
`CMakeLists.txt` turns `HYDRA_MIMALLOC` off. The tests that check the mimalloc
redirect are left out of it, and so is the GUI runner's start check for it.
ASan keeps freed memory to itself during a run, so a test's memory use looks
higher than usual; that is how ASan works, not a leak.

### The coverage report

The coverage report is a map of which lines in `src\` the tests reach. It is
not a score: there is no threshold, and nothing fails when the number drops.

`.\build_cpp.ps1 -Preset coverage -Target hydra_tests` builds into
`build-coverage\`. That build links with `/PROFILE`, which
`Microsoft.CodeCoverage.Console` (shipped with Visual Studio, under
`Common7\IDE\Extensions\Microsoft\CodeCoverage.Console`) needs before it can
instrument a native exe. Instrumenting rewrites the exe in place so it records
each line it runs. The build never ships.

The `coverage-ninja` preset is the same build through Ninja and the sccache
compiler cache, in `build-coverage-ninja\`, the way `asan-ninja` is for the
ASan build. CI uses it, so a night with few changes rebuilds from the cache.
The tool still instruments an exe built this way: each object keeps its own
debug info, and the linker still writes the exe's `.pdb`.

The `coverage` job in `.github/workflows/ci.yml` holds the whole recipe and
runs nightly and on a manual run. It instruments the exes, then runs each test
step inside the tool's `collect` command (the unit-test slices one at a time).
`collect` records every instrumented process the command starts and ends when
the command ends, so no collector outlives its step. It merges the two results,
converts them to Cobertura (a common XML coverage format), and turns that into
an HTML report with ReportGenerator. The HTML is the run's
`coverage-report` artifact, and the line-coverage summary is on the run's
summary page.
`tools\coverage.settings.xml` leaves `third_party\` and `tests\` out of it. It
also turns off the tool's dynamic instrumentation: left on, it injects itself
into each test process ahead of mimalloc, and the mimalloc tests fail.

To instrument again after a test run, relink first (delete the exe and build).

## The report windows

The path report and the dmleaderboards comparison are Hydra windows, each its
own OS window (D103; ADR 0027 says why and what it costs). Each rule behind
them has one owner. This list points at each; read the owner for the rule.

- **The rows.** `report::generate_report` in `src/app/report.h` builds the
  path report's rows, and `dm_report::generate_dm_report` in
  `src/app/dm_report.h` builds the comparison's. Each result also carries the
  subtitle and footer the window shows.
- **The tiles.** `report::path_tiles` and `dm_report::dm_tiles`, beside the
  two builders. Both take the rows the window shows.
- **Search, filtering, sorting and the count line.** `TableView` in
  `src/app/report_view.h`. It is plain C++ with no ImGui, so the doctest
  suite pins it.
- **Each report's columns and keep-rules.** `src/app/path_report_view.{h,cpp}`
  and `src/app/dm_report_view.{h,cpp}`. A keep-rule is what a window's
  dropdown and checkbox let through.
- **When a report goes out of date.** `report::settings_change_touches` and
  `dm_report::settings_change_touches`, beside the two builders (D103 item
  22).
- **The window frame.** `src/ui/report_window.{h,cpp}` draws the header,
  strips, tiles, controls, table, footer and every state from a plain input
  struct. It never sorts, counts or filters by itself. The thin files
  `src/ui/path_report_window.cpp` and `src/ui/dm_report_window.cpp` fill it
  in for each report.
- **The OS window and its placement.** `report_window_class` in
  `src/ui/app_shell.{h,cpp}` makes each report its own OS window.
  `report_placement` beside it decides where a report opens and whether it
  opens maximized, and `place_report_window` applies that and notes where the
  window is (`maximize_due_report_windows` does the maximize itself, between
  frames, called from `main.cpp`), in the `[Hydra][Window:<key>]` sections of `hydra_ui.ini` (D103
  item 14; ADR 0027 says how it behaves). `main.cpp` measures the OS frame
  around a report and hands it over with `set_report_frame_insets`. The popup
  patch in `third_party/imgui/imgui.cpp` keeps popups inside their window
  (D103 item 15).
- **Column widths.** `src/ui/column_widths.h` holds the one rule every data table
  sizes its columns by. The two-column key-and-value forms (the settings bar,
  the batch dialog) are not data tables and keep ImGui's own fit. Its header
  comment says how a table uses it.
- **Chip colours.** `chip_color` in `src/ui/theme.{h,cpp}`.
- **The reports' state.** AppState holds both reports as `ReportSlot`s,
  `path_report` and `dm_report`, in `src/ui/app_state.h`, with the functions
  that build, cancel and show them. The jobs that build them are `ReportJob`
  in `src/ui/library_jobs.h` and `DmReportJob` in `src/ui/dm_jobs.h`.

The GUI tests for the windows are in `tests/ui/uitest_report_windows.cpp`;
[agents/ui-testing.md](agents/ui-testing.md) lists them.

## Developer tools

Two more console programs live in `tools/`. A plain `.\build_cpp.ps1` builds
them; they are not shipped. `hydra_bench` times the
analysis path — parse, search and database write, separately, per chart — so a
change to the engine can be measured instead of guessed at.

`hydra_replay` answers "what is *my* path worth?". Give it a chart and a list
of activation windows in ticks. It walks the chart chord by chord. For each
chord it prints the chord's own score, the running totals, and whether the
chord fell under Star Power — all as JSON, so it can be diffed or graphed.
`hydra_replay dump` analyzes the chart and prints the windows of every path the
engine found, so you can start from a path the app shows and change one
activation. It reads no database (docs/adr/0026).
`hydra_replay score --path <file>` prices a path straight out of the JSON
`dump` or `target` wrote, so nothing has to be retyped and nothing is lost on
the way — in particular the squeeze-out offsets, which a hand-typed window
list drops and which are worth real points. When a window ends on the note
that closes a Star Power phrase and carries no squeeze-out offset, `score`
says so instead of guessing: that score is right if the player did not squeeze
that note out, and a little high if they did.
`hydra_replay target` prices a path the search never kept. Give it the
activation ticks and the engine is made to activate at exactly those fills and
nowhere else; back come that path's squeeze variants with their windows, meter
and skips stamped the engine's way. The search folds equal-scoring paths into
one another, so a real player's path is often not in any record no matter how
deep the search; this is how you get its number anyway.
`hydra_replay selfcheck` is what keeps the numbers honest: it re-analyzes
every corpus chart, replays every path the engine found, and fails if the
replay's six score categories disagree with the engine's own by a single
point.

## Proving identical results

Some changes, such as speedups, must leave every stored result exactly as it
was. Three tools prove that, by comparing a run of the old build (A) with a
run of the new one (B) over the same charts.

`tools\compare_db.py A.db B.db` compares two databases table by table. It
prints one line per table, "N rows compared, M differ", lists the first few
keys that differ, and exits 1 if any table differs. It knows the record
tables (`profiles`, `records` and `record_paths`, ADR 0031) and matches their
rows across the two files by each profile's own columns. It leaves out the ids
each file numbers for itself, such as a profile's id or an old file's
`result_id`, which only records the order worker threads finished in; the
script's own header lists them.
A baseline made before the store kept summaries only (ADR 0026) has tables a
new file no longer has. Add `--summary-only` to compare just what the new
store keeps; the script's own header says which tables and columns that is.

`hydra_bench --engine <folder>` and `hydra_bench --parse <folder>` print one
digest over a whole folder: the first over every chart's whole engine result
plus the best path and summary of the record `prepare_record` makes from it
(`row_hash` in `tests\song_digest.h`), the second over every parsed chart and
its dynamics counts. They read the
same settings file `hydra_batch` reads, the one beside the exe.
`--out <file>` writes one line per chart, so two runs can be diffed to find
the chart that moved. The digests are defined in `tests\song_digest.h`, and
`tests\test_perf_digest.cpp` pins the corpus's values, so a change that moves
a result fails that test.

`tools\bench_run.ps1` runs a timing script under a machine-wide lock, so two
whole-library runs never share the machine. A caller that finds the lock
busy for too long gets exit code 3 and should retry later.

The full recipe, with the commands for a whole-library run, is the
"Proving identical results" section of
`docs\superpowers\plans\2026-10-06-perf-speedups.md`.

## Mutation probe

A passing test only helps if it would fail when the code breaks. The
mutation probe checks that. It plants one small deliberate bug at a time in
one source file, such as flipping `<` to `<=` or dropping a `+ 1`. After
each one it rebuilds `hydra_tests`, runs only the tests you name, and puts
the file back. A bug that makes a test fail is "killed". A bug no test
notices is a "survivor": a spot where a test is missing or too loose.

    pwsh -NoProfile -File tools\mutation_probe.ps1 -Source src\core\replay.cpp -Filter sf=*test_replay* -Max 20

The report gives the score (killed out of the edits that compiled) and each
survivor's line and edit, so a person can write a test for it. The script
edits source, so it refuses the main checkout; run it in a task worktree
made with `tools\new_worktree.ps1`. It holds one machine build slot for the
whole run. Each edit costs a warm rebuild and a test run, so twenty edits
take a few minutes. The script's header lists its options and the kinds of edit it
makes; `tools\test_mutation_probe.ps1` is its self-test.

Run it before a release, on the scoring and path code. It is not meant for
every merge: it is slow, and a survivor is a prompt to look, not a failure.

## Fuzzing the file parsers

A fuzzer feeds a parser random bytes, keeps the inputs that reach new code,
and mutates those. Hydra has one libFuzzer exe per kind of file it reads. The
list is `kTargets` in `tests/fuzz/harness.h`, which also holds the one body
every exe runs. A file the parser turns away with an exception is fine. A
crash, an AddressSanitizer report or a hang is a bug.

Build them with the `fuzz` preset, which sits on the AddressSanitizer build,
and write the seed folders (the checked-in files each fuzzer starts from).
`tools\fuzz_seeds.ps1` needs a built `hydra_tests`, which does the writing;
its header says why.

```
.\build_cpp.ps1 -Preset fuzz -Target hydra_fuzz
.\build_cpp.ps1 -Target hydra_tests
.\tools\fuzz_seeds.ps1
```

Run one target from a Visual Studio developer shell, for as long as you like.
The first folder is the corpus the fuzzer grows; the second holds the seeds.
The `.chart` target also takes a dictionary of the reader's keywords. Add the
`-timeout` and `-rss_limit_mb` limits `tools\fuzz_run.ps1` holds, so a hang
or a memory blowup stops the run the same way it does in the workflows. That
script can also do the run for you; its header shows how.

```
mkdir corpus\chart
.\build-fuzz\Release\fuzz_chart.exe corpus\chart build-fuzz\seeds\chart -dict=tests\fuzz\chart.dict -max_total_time=3600
```

The progress lines count coverage as `ft:` (features), not the `cov:` that
clang's libFuzzer prints. A rising `ft:` means the fuzzer is still finding new
code. On a finding it stops and writes `crash-<hash>` (or `timeout-`,
`oom-`) in the current folder; `-artifact_prefix=<folder>\` puts it elsewhere.

A finding becomes a permanent test by copying the file into
`testdata/fuzz/<target>/`, where `tests/test_fuzz_regressions.cpp` replays it
(see `testdata/fuzz/README.md`). Check it in with the parser fix, never
before, or the suite goes red. `.github/workflows/fuzz.yml` runs every target
for a fixed time once a week and uploads any finding as an artifact.

The weekly run also saves every input it kept, which the test suite never
sees. So CI replays them: the `fuzz-replay` job in `.github/workflows/ci.yml`
runs on every push and pull request. It builds the fuzz exes, restores the
newest combined corpus that fuzz.yml's `corpus-all` job saved, and runs each
input once, with no mutation, together with `testdata/fuzz`. A saved input
that now crashes, hangs or blows up memory fails the job within minutes, and
the input is uploaded as `fuzz-replay-crashes`. Before the first weekly run
saves a combined corpus, or where a pull request cannot read the cache, the
job replays `testdata/fuzz` alone and says so in a notice. To replay on your
own machine, run `tools\fuzz_run.ps1 -Mode replay` from a developer shell; its
header says how it finds the targets.

## Continuous integration

GitHub builds Hydra and runs its tests on every push to `main` and on every
pull request into `main`. The workflow is `.github/workflows/ci.yml`. It is a
backstop: the local hooks still gate commits first, and CI catches a commit
that went around them.

The jobs run on GitHub's Windows Server 2025 image with Visual Studio 2026,
the same generator the default CMake preset names. They build with
`build_cpp.ps1`, exactly as above, so a compiler warning fails the run. The
main job builds and runs `hydra_tests`. A second job, "GUI tests", builds
`hydra_uitest` and runs the headless GUI tests on its own machine at the same
time, so the run takes as long as the slower of the two, not both added up.
Both jobs also run once a night, from a cold build.

The `asan` job builds the same tests with the `asan-ninja` preset (the `asan`
build again, through Ninja and the sccache compiler cache) and runs them (see
"The AddressSanitizer build" above). Any memory error it finds fails the run.
A second job, `asan-gui`, builds `hydra_uitest` the same way and runs the GUI
tests on its own machine at the same time. Both run once a night and when
someone starts the workflow by hand, not on every push. On run 40201169431,
with the cache warm, the `asan` job took 226 s and `asan-gui` 381 s. A cold
build takes longer: 548 s for the `asan` build alone on run 40195923058.
Start them by hand from the Actions tab after a change to memory-heavy code.
Both keep the default shallow checkout, so the precheck self-test skips its
two real commit ranges there; the main job runs them.

Once a night, and when someone starts the workflow by hand, a `coverage` job
builds the tests with the `coverage-ninja` preset (the `coverage` build again,
through Ninja and the sccache compiler cache) and writes the coverage report
(see "The coverage report" above). It never fails a run over a low number. It
skips pushes because it repeats the whole build and both test sets. Its proving
run (run 38083092803, on 2026-10-10) took 11.5 minutes, before the cache. The
cache changes the build step's time. With an empty cache the step took
11 min 17 s (run 40503369133: 487 compiles, all misses). With a full cache it
took 70 s (run 40505915081: 485 hits, 2 misses), and the whole job 5 min 55 s.
The build step before the cache took 7 min 17 s (run 40313402500). The cold
run's coverage report matches the old build's: 158 files, 44,746 coverable lines,
84%.

Another job, `tool-tests`, runs the tool tests above with no C++ build. The main
job runs the mutation probe's self-test after it builds `hydra_tests`, because
that self-test needs `hydra_precheck.exe`. The main job and the coverage job
check out the whole history, because the precheck self-test in `hydra_tests` checks two real
commit ranges and skips them in a shallow clone.

The main job and the GUI tests job build on a saved build folder, so a push
recompiles only what it changed. Only the main job saves it: after its build,
a push to `main` or a manual run saves
`build-cpp` in GitHub's cache, and each branch keeps only its two newest
saved folders. Pushes and pull requests restore the newest one, then
`tools/ci_build_folder.ps1` decides whether the build can use it. That script
owns the rules and prints which way it went. In short, the build starts cold
(from nothing) when there is no saved folder, when the saved folder's commit
is not an ancestor of the one being built, or when a build file changed since
then; the script lists the build files. A new runner image also starts cold.
The nightly run always builds cold, so a broken build that a saved folder hid
shows up within a day. A run started by hand is cold too, unless you turn on its
`warm` input: tick it under "Run workflow" on the Actions tab, or run
`gh workflow run ci.yml --ref <branch> -f warm=true`. Leave it off for a cold
check of any branch.

One slice of the tests is left out on GitHub: the one that opens the real
sound output, because hosted runners have no audio device. `tests/shards.txt`
marks which slice that is. Run it locally.

Every run on `main` finishes. A newer push to `main` never cancels the run
before it, so a red run names the push that broke it. On a pull request or any
other branch, a newer push cancels the run it replaces.

## Releasing

Hydra's updater only runs an installer that was signed with the project's
private key. The check is `app::verify_update` in `src/app/update_verify.cpp`,
against the public key in `src/app/update_key.h`. So every release is built,
signed, uploaded as a draft and then published, in that order.

### Once: make the signing key

Run the key generator once, on the PC you release from:

```
pwsh -NoProfile -File tools\release_keygen.ps1
```

It writes the private key to `%USERPROFILE%\.hydra-release\update-signing.xml`.
The file is locked to your Windows account with DPAPI (Windows' per-user
encryption), so it is useless on another account or PC. It never goes in the
repo or in CI, and the script refuses to write it inside the repo. Back the
file up in your password store straight away.

The script also prints the public key as a C++ array. That array is already in
`kUpdatePublicKey` in `src/app/update_key.h`. The test "the shipped update key
is not the test key" in `tests/test_update_verify.cpp` fails if the test key
ever lands there, so a release can't go out with it.

If the key is ever lost, make a new one the same way and paste its array over
the old one in `update_key.h`. Installed copies only
trust the old key, so the first release with the new key can't update them:
players have to download and install that one release by hand.

### Every release: refresh the screenshots

The pictures in `docs/images/` are taken from the test charts by one command,
`pwsh -NoProfile -File tools\doc_shots.ps1`. Run it before each release, read
the pictures it saves, and commit the changed ones on a branch for review like
any other change. `docs/agents/doc-screenshots.md` explains the command, the
shot list and the size limits.

### Every release: build and sign

```
.\installer\build_installer.ps1 -Sign
```

This builds `build-cpp\installer\HydraDeluxe-<version>-setup.exe` and then signs
it, writing `HydraDeluxe-<version>-setup.exe.sig` (64 bytes) beside it. The
signing step is `tools\release_sign.ps1`, which you can also run by hand:

```
pwsh -NoProfile -File tools\release_sign.ps1 -Installer build-cpp\installer\HydraDeluxe-<version>-setup.exe
```

It checks the new signature before it finishes and prints the public key it
signed with; that key must match `src/app/update_key.h`.

### Every release: upload as a draft, then publish

Create the release as a draft with both files, then publish it:

```
gh release create v<version> --draft build-cpp\installer\HydraDeluxe-<version>-setup.exe build-cpp\installer\HydraDeluxe-<version>-setup.exe.sig --notes-file <notes>
gh release edit v<version> --draft=false
```

The draft step matters. Hydra asks GitHub for the latest published release,
and a draft is never that. Publishing only after both files are attached means
no player's Hydra ever sees a release with the signature missing. A release
without its `.sig` is refused, not installed.

`tools\test_release_sign.ps1` checks that the signing script and the C++
verifier agree on the signature format, using the committed test key in
`testdata/update/`. Run it after any change to either script.

## Other developer notes in this folder

`adr/` records design decisions. `agents/` holds instructions for coding
agents. `audit/`, `handoffs/` and `superpowers/` are working notes, plans and
specs from past changes. `srb-format.md` and
`cap-clamped-squeeze-frontend-anchor.md` are technical write-ups. None of these
are needed to use the app.
