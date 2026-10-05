Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task C4b: named UI timing constants and colour names, values unchanged

Task id: C4b. Base: main at dc7e582. Branch: `claude/p3-c4b`. Worktree: `.claude/worktrees/p3-c4b`.

## Goal

Four short UI timings and three colours are typed as bare numbers today. This task gives each one a name in the file that owns it, and points the existing uses at that name. Nothing on screen changes: every value stays exactly what it is, and a scan row in the single-owner test keeps it that way.

## Finding R7.33: four UI timings are bare numbers

Today the "Done!" flash lasts half a second, written as `0.5` in `AppState::update_analyze_job` (`src/ui/app_state.cpp`). The "Copied!" flash lasts two seconds, written as `2.0` in `render_path_footer` (`src/ui/paths_tab.cpp`). The library search re-filters at most every 0.15 s, which is already named `kSearchThrottleSeconds`, but in the anonymous namespace of `src/ui/library_table.cpp`, read by `render_search_box`. Batch results refresh at most once a second, written as `1.0` in `AppState::tick_library` (`src/ui/app_state.cpp`). The two sibling timings beside them, `kFileCheckSeconds` and `kReportCheckSeconds`, are already named members of `AppState` in `src/ui/app_state.h`.

The owner is `AppState` in `src/ui/app_state.h`. Add four `static constexpr double` members beside `kFileCheckSeconds`: `kDoneFlashSeconds` (0.5), `kCopiedSeconds` (2.0), `kSearchThrottleSeconds` (0.15, moved here from `library_table.cpp`), and `kBatchRefreshSeconds` (1.0). Each gets one plain comment saying what it times. The user confirmed all four values in D48, Q33, so they are decided numbers, not new limits.

Then the four uses read the names: `update_analyze_job` and `tick_library` in `app_state.cpp`, `render_path_footer` in `paths_tab.cpp`, and `render_search_box` in `library_table.cpp`. The local constant in `library_table.cpp` goes away. The comment on `search_applied_at` in `app_state.h` that says "150 ms" may name the constant instead.

The 6.0 s status-line fade in `src/ui/library_toolbar.cpp` (finding 219) is the fifth timing of this kind. You do not own that file. See Open questions.

## Finding 218, the code half: a grey written twice and a hover teal written twice

Today `kDimTextColor` and `kDisabledInputTextColor` in `src/ui/theme.h` both spell the same grey, (160,160,160). They answer the same question, "which grey is dimmed or disabled text", in two places. In `apply_theme` (`src/ui/theme.cpp`), the `FrameBgHovered` and `HeaderHovered` slots each spell (0,100,100) as a literal. No name exists for that hover teal.

The owner of both is `src/ui/theme.h`. `kDisabledInputTextColor` becomes an alias of `kDimTextColor`, so the grey is written once. A new `kFrameHoveredColor` holds (0,100,100), and `apply_theme` assigns it to both hover slots. The value stays (0,100,100). Whether it should match the button hover teal (0,104,104) is the display half of 218; it is not in this plan and you do not touch it. `kDisabledInputTextColor` keeps its name, because `begin_disabled_input` in `theme.cpp` and `tests/test_theme.cpp` read it.

## Finding 136, the naming half: the Star Power gold has no name

Today `IM_COL32(255, 204, 51, ...)` is typed four times in `render_preview_panel` (`src/ui/preview_tab.cpp`): the next-activation header, the "SP" label, the gauge fill and the drain-box accent. The scrubber marks in `draw_scrub_marks` use `kBestPathColor`, a different gold (250,210,0). No named SP gold exists.

The owner is `src/ui/theme.h`. Add `kStarPowerColor` as (255,204,51) beside `kBestPathColor`, with a one-line comment saying it is the Star Power gold. That is all this task does for 136. Task K4a owns `preview_tab.cpp` and makes the four uses read the right names under D48, Q26 (marks and header take the best-path gold, the drain box turns teal). So the four literals stay in `preview_tab.cpp` for now. Your scan row lists them as known copies, removed by K4a.

## Owned files (only these may change)

- `src/ui/theme.h`
- `src/ui/theme.cpp`
- `src/ui/app_state.h`
- `src/ui/app_state.cpp` (the two timing lines only)
- `src/ui/paths_tab.cpp` (the one timing line only)
- `src/ui/library_table.cpp` (remove the local constant, point the one use at `AppState`)
- `tests/test_theme.cpp` (new cases)
- `tests/test_app_state.cpp` (one new case)
- `tests/test_single_owner.cpp` (new rows and their known copies only; phase 5 edits other rows in this file, so keep your hunks to your rows)

The plan's C4b row lists only the six source files. The three test files are added here because the plan's done-when needs scan rows and the preamble needs a failing test first.

## Test cases to add

Write each one first, run it red, then make it green.

In `tests/test_theme.cpp`:
- "theme: disabled input text is the dimmed grey". Pins `kDisabledInputTextColor` equal to `kDimTextColor` in all four components. No colour number is pinned; D48 names none for this grey.
- "theme: frames and headers share one hover teal". After `apply_theme`, pins `FrameBgHovered` and `HeaderHovered` both equal to `kFrameHoveredColor`. Follow the pattern of the existing "theme: apply_theme uses the readable shades" case.
- "theme: the Star Power gold is named". Pins `kStarPowerColor` at 255, 204, 51 (the numbers D48, Q26 names), full alpha.

In `tests/test_app_state.cpp`:
- "UI timings: Done!, Copied!, search re-filter and batch refresh keep their seconds". Pins `AppState::kDoneFlashSeconds` 0.5, `kCopiedSeconds` 2.0, `kSearchThrottleSeconds` 0.15, `kBatchRefreshSeconds` 1.0 (the numbers D48, Q33 names).

In `tests/test_single_owner.cpp`, four rows, each with a plain question, the owner, one must-match line and one must-not-match line:
- "How long does a UI confirmation stay, and how often does the UI re-check?" Owner: `AppState` in `src/ui/app_state.h`. The pattern is a time-since (`now - x` or `GetTime() - x`) compared against a bare number. Must match today's `0.5` and `2.0` lines; must not match a compare against `kFileCheckSeconds`.
- "Which grey is dimmed or disabled text?" Owner: `kDimTextColor` in `src/ui/theme.h`. Pattern: the (160,160,160) spelling in `ImVec4` or `IM_COL32` form. Owner line: the `kDimTextColor` definition.
- "Which teal is the hover colour for frames and headers?" Owner: `kFrameHoveredColor` in `src/ui/theme.h`. Pattern: the (0,100,100) spelling. Owner line: the `kFrameHoveredColor` definition.
- "Which gold is Star Power?" Owner: `kStarPowerColor` in `src/ui/theme.h`. Pattern: the (255,204,51) spelling. Owner line: the `kStarPowerColor` definition. Known copies: the four `preview_tab.cpp` lines, each with `removed_by` naming task K4a.

Known-copy and owner-line texts must be the exact trimmed line, so take them from the file, not from this brief.

## Tests you may run

- `-sf=*test_theme*`
- `-tc="the chart-file check*"`
- `-tc="UI timings*"`
- `-tc="single-owner rules match their own examples"` and `-tc="single-owner rules hold across src/, tools/ and tests/"`

The plan's row names only the first two. The last three are added because the new cases and scan rows cannot be checked without them. Never the full suite. No `hydra_uitest`.

## Done when

- No bare 0.5, 2.0, 0.15 or 1.0 timing literal remains outside `app_state.h`.
- No repeated colour literal remains outside `theme.h`, except the four Star Power gold lines in `preview_tab.cpp`, which the scan lists as known copies for K4a.
- A scan row guards each of the four facts, and both single-owner cases pass.
- `test_theme` and the two `test_app_state` cases pass, and every value is unchanged.
- `git diff --stat dc7e582..HEAD` lists only the owned files.

## Open questions

**Orchestrator answers (2026-10-04, binding):** Q1: the timing row lists the 6.0 s fade line in `library_toolbar.cpp` as a known copy with `removed_by` "finding 219, not yet scheduled"; do not edit that file. Q2: known copies for K4a, as the brief says; do not edit `preview_tab.cpp`.

1. The 6.0 s status-line fade in `library_toolbar.cpp` (finding 219) will match the timing scan row. 219 is not in the plan, and K3 owns that file. Either the row lists that line as a known copy with `removed_by` "finding 219, not yet scheduled", or the orchestrator gives 219 to K3 as a fifth named constant beside these four. Say which before the implementer writes the row; the brief does not choose.
2. The plan's done-when says no repeated colour literal outside `theme.h`, but C4b does not own `preview_tab.cpp`, where the gold is typed four times. This brief resolves it with known copies for K4a. If the orchestrator prefers C4b to edit those four lines, `preview_tab.cpp` must be added to the owned files and K4a told.
