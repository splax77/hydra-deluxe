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
hydra_batch                    Analyze every chart folder from the app's settings
hydra_batch <folder> [...]     ...or specific folders instead
hydra_batch --redo             Re-analyze charts already stored
hydra_batch --db <path>        Target a specific database
hydra_batch --rules <path>     Take the rule choices from this file, not hydra_rules.ini
hydra_batch --legacy-fills     Score fills by Clone Hero 1.0's rule (needs its own --db)

hydra_fillcompare --old <ch10.db> --new <ch11.db>
                               Compare Clone Hero 1.0 and 1.1 fill results, chart by chart
hydra_fillcompare ... --out fill_compare.html
hydra_fillcompare ... --rules <path>
hydra_fillcompare ... --no-open
```

Both read the app's settings file, so they work at the same SP cap, timing
limit, score range and chart mode the app is set to. The fill rule is the
exception, below. Both read the scoring rules from `hydra_rules.ini` next to
Hydra.exe, or from the file `--rules` names. If that file has an error, they
print it and stop with exit code 2.

Clone Hero 1.1 changed when a drum fill appears. The app's **1.0 fills**
setting scores by the older 1.0 rule instead. Each result remembers which rule
made it, so a song's 1.0 and 1.1 results sit side by side in `hydra.db`.

`hydra_batch --legacy-fills` does the same from the command line. It ignores
the app's 1.0 fills setting and goes by the flag alone. It still refuses to
write into the app's own `hydra.db`, so give it its own `--db`. Each database it
fills is stamped with the rule, and hydra_batch refuses (exit code 2) a run
whose rule disagrees with the stamp.

To see what the rule change did, compare the two. `hydra_fillcompare` reads
the 1.0 results from `--old` and the 1.1 results from `--new`. Each chart's
row is labelled by which database holds a result for it. Narrow columns name
the rules CH 1.0 and CH 1.1. Like the path report, it compares library charts
only. A database built by `hydra_batch` with folder arguments has no chart
library, so it stops with the path report's sentence. D92 and
`report::lacks_chart_library` own that rule, and ADR 0026 explains it. The
dmleaderboards comparison follows it too. The two databases can be two
files, or the app's own database twice:

```
hydra_batch --legacy-fills --db ch10.db
hydra_batch --db ch11.db
hydra_fillcompare --old ch10.db --new ch11.db

hydra_fillcompare --old hydra.db --new hydra.db
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
folder's path. No symbol file ships: the install rules name only the exes and
DLLs, and the installer refuses a staging folder that holds one.

The vendored SQLite is built without its memory-use counters and without
shared cache. Hydra uses neither, and leaving them out trims a little of
SQLite's own work; `tests/test_store.cpp` checks both. SQLite's default sync
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

To run the leak checks (`hydra::test::leak_checked` in `tests/leak_check.h`), build
with `.\build_cpp.ps1 -Target hydra_tests -Config Debug` and run
`build-cpp\Debug\hydra_tests.exe` with `MIMALLOC_DISABLE_REDIRECT=1` set.

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

The Python and PowerShell tools have their own tests, and none needs a C++
build. Run them from the repository root:

    py -m pytest tools/ch_probe/tests tools/test_compare_db.py -q
    pwsh -NoProfile -File tools/test_mutation_probe.ps1
    pwsh -NoProfile -File tools/test_derive_once_precheck.ps1

Name the two pytest paths rather than `tools/` as a whole: the scripts in
`tools/ch_probe/experiments/` look like tests to pytest and drive the real
game.

The program's name on screen is "Hydra Deluxe", but its files keep their older
names: `Hydra.exe`, `hydra.db`, `hydra_settings.ini`, `hydra_ui.ini` and the
`C:\Program Files\Hydra` install folder. That way an upgrade from an earlier
Hydra finds the user's records where they were. Older versions also saved
report pages in `Documents\Hydra`. Nothing reads or writes that folder any
more, and Hydra leaves the old pages there (ADR 0027).

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
- **The OS window and its placement.** `report_window_class` and
  `place_report_window` in `src/ui/app_shell.{h,cpp}` (D103 item 14). The
  popup patch in `third_party/imgui/imgui.cpp` keeps popups inside their
  window (D103 item 15).
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
keys that differ, and exits 1 if any table differs. It leaves out
`result_id`, which only records the order worker threads finished in.
A baseline made before the store kept summaries only (ADR 0026) has tables a
new file no longer has. Add `--summary-only` to compare just what the new
store keeps; the script's own header says which tables and columns that is.

`hydra_bench --engine <folder>` and `hydra_bench --parse <folder>` print one
digest over a whole folder: the first over every chart's stored result row,
the second over every parsed chart and its dynamics counts. They read the
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

## Continuous integration

GitHub builds Hydra and runs its tests on every push to `main` and on every
pull request into `main`. The workflow is `.github/workflows/ci.yml`. It is a
backstop: the local hooks still gate commits first, and CI catches a commit
that went around them.

The job runs on GitHub's Windows Server 2025 image with Visual Studio 2026,
the same generator the default CMake preset names. It builds `hydra_tests`
and `hydra_uitest` with `build_cpp.ps1`, exactly as above, so a compiler
warning fails the run. Then it runs `hydra_tests` and the headless GUI tests
in `hydra_uitest`.

A second job, beside it, runs the tool tests above with no C++ build. It
checks out the whole history, because the precheck self-test checks two real
commit ranges and skips them in a shallow clone.

One test is left out on GitHub: the one that opens the real sound output
(`tests/test_audio_device.cpp`), because hosted runners have no audio device.
Run it locally. A newer push to the same branch or pull request cancels the
run it replaces.

## Other developer notes in this folder

`adr/` records design decisions. `agents/` holds instructions for coding
agents. `audit/`, `handoffs/` and `superpowers/` are working notes, plans and
specs from past changes. `srb-format.md` and
`cap-clamped-squeeze-frontend-anchor.md` are technical write-ups. None of these
are needed to use the app.
