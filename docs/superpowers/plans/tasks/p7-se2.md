Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task SE2: the settings boxes call SE1's clamp (findings 31, 311, 322, 139 box callers)

Task id: SE2. Base: main after M_D and M7-1, then phase 6 J2-4's commit (the main session names both hashes at launch; J2-4 and this task both edit `tests/test_app_state.cpp`, so you fork from J2-4's commit). Branch: claude/p7-se2 (worktree `.claude\worktrees\p7-se2`, made as the preamble says).

Plan row: `docs/superpowers/plans/2026-10-04-phase-7.md`, wave 2 table, "SE2 Settings boxes". Decisions: D51 calls 14 and 16 in `docs/audit/2026-10-03-fix-decisions.md` (questions 14 and 16 of `docs/audit/2026-10-04-phase-7-questions.md`), plus the D51 addendum's SE1 lines. Finding texts: `docs/audit/2026-10-03-derivation-audit.md`, headings `#### 31.`, `#### 311.`, `#### 322.` and `#### 139.`.

## Goal

One question: what range may a typed number setting hold? SE1 (merged at M7-1) gave it one owner, `Settings::clamp` in `src/app/config.cpp`, backed by the key table there, and the file loader already asks it. The four number boxes in the GUI still carry their own copies of the same ranges. After this task each box hands the typed value to `Settings::clamp` and keeps nothing of its own, so a typed value and a hand-edited file value land on the same edge (D51 call 14), the cap box floors at 1 bar through the key table (call 16), and the three known-copy rows SE1 left for you in `tests/test_single_owner.cpp` come out. Nothing on screen changes for any value the boxes accept today.

## What the code does today

Checked at `claude/p3-d2`, which M_D brings.

**139 (cap box).** `render_sp_cap` in `src/ui/settings_bar.cpp` (about line 87) writes `std::max(1, cap)` into `app.settings.sp_cap`. The key table's `sp_cap` row floors at 1 as well.

**311 (depth box).** `render_score_range` (about line 122) turns a negative `depth_value` into 0 with its own `if`. The key table's `depth_value` row is 0 and up.

**322 (Path limit box).** `render_path_limit` (about line 150) clamps `mslimit_value` to plus or minus `kSqueezeWindowMs` itself, with a comment citing D41. The key table's `mslimit_value` row is the same window.

**31 (Backend limit box).** `render_path_footer` in `src/ui/paths_tab.cpp` (about line 505) clamps `backendlimit_value` to 0 up to `kSqueezeWindowMs` itself. The key table's `backendlimit_value` row is the same range, and `Settings::backend_limit` no longer takes an absolute value (SE1).

`Settings::clamp(int Settings::* field, int value)` (`src/app/config.h`) is the owner: `clamp(&Settings::mslimit_value, 900)` is 500, `clamp(&Settings::backendlimit_value, -30)` is 0, `clamp(&Settings::depth_value, -1)` is 0, `clamp(&Settings::sp_cap, 0)` is 1 (typed; only the file reads 0 as 4), all pinned in `tests/test_config.cpp` `Settings::clamp is the one range every caller asks`. `tests/test_single_owner.cpp` (from `claude/p7-w1`) carries a rule "What range may a number setting hold?" whose scan catches `std::max`, `std::min` and `std::clamp` writes to those fields, with three known copies marked "task SE2 (the boxes call Settings::clamp)": the `std::max(1, cap)` line and the `mslimit_value = std::clamp(...)` line in `settings_bar.cpp`, and the `backendlimit_value = std::clamp(...)` line in `paths_tab.cpp`.

The score-range combo (`depth_mode`) has two items and cannot produce a value outside 0 and 1, so it has no copy to remove; the Preview volume is PV's, and hydra_replay's `--cap` is T1's (done).

## What changes

Owner: `Settings::clamp` (SE1's; you do not edit it).

Each of the four boxes, when ImGui reports an edit, writes `Settings::clamp(&Settings::<field>, <typed value>)` into its field and then calls `app.edit_settings()` (or `app.commit_settings()` for the Backend limit, as today), with no `std::max`, `std::min`, `std::clamp` or `if (... < 0)` of its own. The local `window` variable and the D41 comment in `render_path_limit` go; the key table carries the window. The cap box keeps its local `cap` copy or drops it, your call, as long as the stored field only ever holds a clamped value. The box labels, widths, help text and the `##spcap`, `##depthvalue`, `##mslimitvalue` and `##backendlimitvalue` ids are untouched, because the GUI tests find the boxes by them. If `settings_bar.cpp` or `paths_tab.cpp` does not include `app/config.h` yet, it does now.

Remove the three known-copy rows for SE2 from `known_copies()` in `tests/test_single_owner.cpp`; the rule itself stays, and the scan then enforces that no box grows a range of its own again. Add no scan row of your own unless you find a fourth copy; if you do, report it.

## Owned files (only these may change)

- `src/ui/settings_bar.cpp`
- `src/ui/paths_tab.cpp`
- `tests/test_app_state.cpp`
- `tests/test_single_owner.cpp` (the three known-copy rows; any row of your own at the end of the file only).

Notes on the base. `settings_bar.cpp`, `paths_tab.cpp` and `test_app_state.cpp` changed only on `claude/p3-d2` (phase 3 wave D; the box lines above are that branch's text); `claude/p7-w1` left them alone and changed `test_single_owner.cpp` (the SE2 rows) and `config.h`/`.cpp` (the owner). J2-4 edits `test_app_state.cpp` and the uitest harness in this wave; your fork from its commit is what keeps the file to one writer. `tests/ui/uitest_batch_reports.cpp` (J2-4), `uitest_details.cpp` (J2-7) and `config.*` (J2-1) are not yours.

## Test cases

Write red first where a red exists.

1. `a number setting edited outside its range lands on the edge the file loader uses` (new, in `tests/test_app_state.cpp`): build the app the way `number boxes apply at once but write the INI only on flush` does (`ScratchPaths`, `app_on`). Set `mslimit_value` to `Settings::clamp(&Settings::mslimit_value, 900)`, call `edit_settings()` then `flush_settings()`, and pin that `Settings::load_file(paths.ini).mslimit_value` is `static_cast<int>(kSqueezeWindowMs)` and equals what loading an INI line `mslimit_value=900` gives (the loader's own answer, through `load_file` on a second scratch file, not retyped). Do the same for `sp_cap` typed as 0 landing on 1 and `depth_value` typed as -1 landing on 0. This case is green from the start; it pins that the box path and the file path share one answer, and it is why this file is yours.
2. The scan: with the three known-copy rows removed, `single-owner rules hold across src/, tools/ and tests/` must pass. Run it red first by removing the rows before you touch the boxes, and record the three lines it names.

Existing cases that must pass unchanged: `number boxes apply at once but write the INI only on flush`, `stepping a number box back reuses the lookup it already made`, `commit_settings refreshes when the ms limit or the score range changes`, `commit_settings refreshes the record and the library row on an SP cap change`.

## Test filters you may run

- `build-cpp\Release\hydra_tests.exe -sf=*test_app_state*`
- `build-cpp\Release\hydra_tests.exe -tc="number boxes apply at once*"`
- `build-cpp\Release\hydra_tests.exe -tc="single-owner*"`
- `build-cpp\Release\hydra_uitest.exe --test settings-and-reports` (build `hydra_uitest` with a warm `.\build_cpp.ps1 -Target hydra_uitest` first). Never `--all`.

Nothing else. Never the full suite.

## Stored results

None. Every value a box accepts today lands where it landed; only a typed value outside the range changes, and it lands where the file loader already puts it. Scores, paths and records are byte-identical. The stamp stays "2.1.0".

## Not in this task

- The owner, its key table and the loader (SE1, merged); `Settings::backend_limit` (SE1).
- The Preview's volume box, Ctrl+click typing and its cap floors (PV, this wave).
- The `depth_mode` combo and `within_label` (E3), and `batch_settings_summary` (LB, wave 3).
- A GUI test that types an out-of-range number into a box: see open question 1.

## Done when

- The four boxes write `Settings::clamp(...)` and nothing of their own; `git grep -n "std::clamp\|std::max(1, cap)" src/ui/settings_bar.cpp src/ui/paths_tab.cpp` shows no range on a settings field.
- The three SE2 known-copy rows are gone and `single-owner*` passes; case 1 passes; the named existing cases pass; `settings-and-reports` passes.
- `git diff --stat <base>..HEAD` lists only the owned files.

## Open questions (each with a recommended answer)

**Decided (D58):** the user took the recommended answer to every question below. Treat each recommendation as the decision.

1. **A GUI case that types 900 into the Path limit box (code-only).** The real proof of this task is a typed value landing on 500, and `uitest_details.cpp` already types into `##spcap` with `ItemInputValue`. Both GUI test files that could hold it belong to phase 6 in this wave (J2-4, J2-7). Recommended: the scan rows and case 1 are this task's proof, and the main session adds one typed-value step (900 in `##mslimitvalue` reads 500, 0 in `##spcap` reads 1) to `settings-and-reports` at M7-2, when `uitest_batch_reports.cpp` is free.
2. **The cap box's local copy (code-only).** `render_sp_cap` edits a local `cap` so the field never holds an unclamped value for a frame; the other three boxes bind the field directly. Recommended: keep each box's binding as it is and only change what it writes; making all four alike is a J4-3 widgets question.

## Commits

One commit, trailers `Task: SE2` plus the preamble's others. Report as the preamble says.
