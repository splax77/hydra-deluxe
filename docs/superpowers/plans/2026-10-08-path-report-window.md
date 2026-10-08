# Report Windows Implementation Plan (2026-10-08)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers-extended-cc:subagent-driven-development (recommended) or superpowers-extended-cc:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** the path report and the dmleaderboards comparison each open in their own Hydra window, a real OS window, instead of a web page. Both share one window frame and one table view. The two HTML pages, the GUI's file and browser plumbing, and `hydra_report` are gone. Every row reads as it does today.

**Spec:** `docs/superpowers/specs/2026-10-08-path-report-window-design.md`. **Mock:** `docs/superpowers/specs/2026-10-08-path-report-window-mockup/report-window.html`, boards 1 to 5.

**Tech stack:** C++17, Dear ImGui docking branch 1.93.0 WIP (vendored) with the Win32 + DX11 backends, doctest (`hydra_tests`), `hydra_uitest`.

**Status:** spec approved by the user 2026-10-08; wave 1 dispatched the same day. Base: main at d7b8001. Wave 1's briefs are `docs/superpowers/plans/tasks/rw-t1.md`, `rw-t2.md` and `rw-t3.md`, after `_rw-preamble.md`.

## Global constraints

These bind every task and every reviewer.

- **Rows don't change.** `generate_report`'s rows and `collect_dm_rows`' rows keep their exact values. No stored version stamp moves, so nothing needs re-analyzing. The join proves it on `testdata/input` and a recorded set of dm scores.
- **One owner per rule.** Each rule has exactly one home:
  - tiles: `path_tiles` and `dm_tiles`, beside each report's data
  - filtering, sorting and the count line: the shared `TableView`
  - columns and keep-rules: the two `*_report_view` files
  - chip colours: `theme`
  - search: `app/library_query`

  The windows draw what they're given. Nothing is written again that already exists; it moves.
- **New words are fixed by the spec.** Its "New words on screen" list is complete. Any other new user-visible text stops and goes to the user.
- **Fail loudly.** A failed build shows today's failure sentence, message and error inside the window. Nothing falls back silently.
- **Agent rules** (CLAUDE.md):
  - Opus executors, Sonnet reviewers, Fable planners.
  - Every brief starts from `docs/agents/brief-preamble.md` and names the status file, `C:\Users\Patrick\.claude\hooks\state\status\<agent id>.md`.
  - Agents run only their own tests.
  - Wrap up at 100 tool calls; stop at 150.
  - Every merge to main gets a derive-once review (`docs/agents/derive-once-review.md`), one round at most.
  - No whole-library run without the user's yes in chat.

**User decisions:** D103, items 1 to 13 in the spec, all made on 2026-10-08.

---

## The shape of the work

Think of it as building the new rooms before taking down the old ones. Each wave runs as one Workflow run of parallel agents in their own worktrees. Between waves the main session merges, which is a natural stop.

**Wave 1 lays three independent foundations.**

- T1 is the OS window plumbing, as a spike with an empty window.
- T2 teaches each report's data layer to hand over its tiles and text instead of a page.
- T3 is the shared table view and both reports' columns.

None of them touch the GUI's flow, so Hydra keeps working exactly as today after each merge.

**Wave 2 builds the two halves that meet in the middle.**

- T4 rewires the app state and jobs: results live in memory, nothing writes or opens a file, and the picker hands over to a window.
- T5 builds the window frame and both windows, drawn from a plain input struct and tested through a hook, without touching app state.

**Wave 3 joins them and clears the old.**

- T6 wires the windows into the app and adds the end-to-end UI tests.
- T7 deletes the pages, the plumbing and the CLI.
- T8 writes the docs and the ADR.

**The order:**

1. **Before dispatch (main session).** Done 2026-10-08: D103 is recorded in `docs/audit/2026-10-03-fix-decisions.md`, and the spec, mock, plan and D103 were committed with `docs/handoffs/2026-10-08-report-windows-handoff.md`. What's left is the user's review of the written spec.
2. **Wave 1:** T1, T2 and T3 in parallel, each forked from main.
3. **Merge wave 1** in the order T2, T3, T1. T1 merges only after the main session's hands-on check of the spike with the user.
4. **Wave 2:** T4 and T5 in parallel, forked from the wave-1 tip.
5. **Merge wave 2.**
6. **Wave 3:** T6, T7 and T8 in parallel, forked from the wave-2 tip.
7. **Merge wave 3,** in the order T8, T7, T6.
8. **The join** (main session), described at the end.

**Who owns which files,** so two writers never meet in one wave:

| Wave | Task | Owns |
|---|---|---|
| 1 | T1 | `src/ui/main.cpp`, `src/ui/app_shell.{h,cpp}` |
| 1 | T2 | `src/app/report.{h,cpp}`, `src/app/dm_report.{h,cpp}`, new cases in `tests/test_report.cpp` and `tests/test_dm_report.cpp` |
| 1 | T3 | new `src/app/report_view.{h,cpp}`, `src/app/path_report_view.{h,cpp}`, `src/app/dm_report_view.{h,cpp}`, new `tests/test_report_view.cpp` |
| 2 | T4 | `src/ui/app_state.{h,cpp}`, `src/ui/library_jobs.{h,cpp}`, `src/ui/dm_jobs.{h,cpp}`, `src/ui/library_toolbar.cpp`, `src/ui/library_dialogs.cpp`, `tests/test_app_state.cpp`, `tests/test_library_jobs.cpp`, `tests/ui/uitest_batch_reports.cpp` |
| 2 | T5 | new `src/ui/report_window.{h,cpp}`, `src/ui/path_report_window.cpp`, `src/ui/dm_report_window.cpp`, `src/ui/theme.{h,cpp}`, new `tests/ui/uitest_report_windows.cpp` |
| 3 | T6 | `src/ui/library_view.cpp`, the end-to-end cases in `tests/ui/uitest_report_windows.cpp` |
| 3 | T7 | `src/app/report_files.{h,cpp}`, `src/app/html_page.{h,cpp}`, `src/ui/report_outcome.h`, `src/ui/win32_dialogs.{h,cpp}`, `src/cli/report.cpp`, the page lines in `src/app/report.cpp` and `src/app/dm_report.cpp`, the old page tests, `tests/test_single_owner.cpp`, `tests/ui/uitest_harness.cpp`'s browser and folder recorders |
| 3 | T8 | `docs/**` |

`CMakeLists.txt` is touched by T3 and T5 (new sources and tests) and by T7 (removing `hydra_report`). Each change is a separate line, so the merges are mechanical.

---

### Task 1: Viewport spike (wave 1)

**Goal:** Hydra can show a report window in its own OS window, owned by the main window, and nothing else in the app changes.

**Files:** `src/ui/main.cpp` and `src/ui/app_shell.{h,cpp}`. The spike also adds an empty "Report spike" window behind a dev-only flag, which T5 replaces.

**Steps:**

- [ ] Turn on `ImGuiConfigFlags_ViewportsEnable`. Init the Win32 and DX11 backends for viewports, and call `UpdatePlatformWindows` and `RenderPlatformWindowsDefault` after `Render`. Follow the docking-branch Win32 + DX11 example in `third_party/imgui/examples`.
- [ ] Rewrite the comment at `app_shell.cpp:230` to say the report windows opt in, and why.
- [ ] Add `report_window_class()`, which returns the shared window class:
  - never merge into the main window (`NoAutoMerge`)
  - `NoDecoration` and `NoTaskBarIcon` cleared for this class only
  - the main viewport as its parent
- [ ] Add `place_report_window(name)`. On first open it centres the window over the main window, capped to that monitor's work area. On every open it checks the saved rectangle through `placement_on_screen` and falls back to first-open placement when the check fails.
- [ ] Change the occluded-skip at `main.cpp:326-332` to skip a frame only when every platform window is minimized or covered.
- [ ] Run the existing uitest scripts that open the five modals, and nothing more. Those are "Song folders", "Remove folder?", "Scanning charts", "Analyze library" and "Compare dmleaderboards user".

**Acceptance:**

- The five modals and the tooltips behave as before at normal window sizes.
- Under `hydra_uitest` the spike window opens by its name.
- The final report lists every place a popup or tooltip can now become its own OS window. (Done; after the user's answer, D103 item 15, task T1b keeps them all inside their window instead.)

**Hands-on check before merge (main session, with the user).** Build the spike and launch Hydra. Open the spike window, drag it to the second monitor, minimize and restore Hydra, cover Hydra with another app, then restart and reopen. Send the user screenshots. If anything misbehaves, stop. The ImHex-style off switch then goes to the user as a question.

### Task 2: The data layer hands over tiles and text (wave 1)

**Goal:** each report's result carries everything a window needs. The tiles get their one C++ home. The pages still build, so nothing breaks yet.

**Files:** `src/app/report.{h,cpp}` and `src/app/dm_report.{h,cpp}`. New cases go in `tests/test_report.cpp` and `tests/test_dm_report.cpp`.

**Steps (failing test first for each):**

- [ ] `GeneratedReport` gains `subtitle` and `footer`, the strings `generate_report` already builds. `GeneratedDmReport` gains `rows`, `subtitle`, `footer`, `username` and `chartmode`. `html` stays until T7.
- [ ] Add `path_tiles(rows)`, which returns five label and value pairs: Charts shown, Paths shown, Hardest ms, Past N ms and Highest skip. Charts shown takes over `page_charts`' copies-once-per-chart rule; `page_charts` then calls it or is folded in, so the rule exists once. Hardest ms shows that row's `ms_text`. Past N ms reads `beyond_edge_ms`.
- [ ] Add `dm_tiles(rows)`, which returns the nine tiles. The counts call `tally_dm_rows`. Average % of optimal is the mean of `pct_h`, rounded half up and written by the existing `format_percent`. This takes over the page script's version. Points left adds up `delta` over under-optimal rows.
- [ ] Add `status_token(status)`. It takes over the page's `STATUS_CLASS` table, which maps each status to a chip-colour token.
- [ ] Pin `path_tiles` on the six sample path rows from the samples test, and `dm_tiles` on the four sample scores. The dm literals are 4, 1, 0, 1, 1, 1, 0, "98.80%" and "3,456".

**Tests:** `-sf=*test_report.cpp*` and `-sf=*test_dm_report*`, new cases only.

### Task 3: The shared table view and both reports' columns (wave 1)

**Goal:** everything the pages' script decided about which rows show and in what order is now C++, pinned by doctest.

**Files:** create `src/app/report_view.{h,cpp}`, `src/app/path_report_view.{h,cpp}`, `src/app/dm_report_view.{h,cpp}` and `tests/test_report_view.cpp`. Add them to CMake.

**Interface:**

- `struct Column` has `id`, `title`, `numeric`, `definition`, `sort_key(row)` and `cell(row)`. The sort key returns a number, text or empty.
- `template <class Row> class TableView` is built from the rows, their search texts and the column table. It has `set_search`, `set_keep(predicate)`, `set_sort` (up to two column ids, each with a direction), `first_direction(column)`, `visible()`, `count_line(noun)` and `clear_search()`.
- `path_report_view` gives `path_columns()` (fifteen, in today's order) and `path_keep(tier, best_only)`.
- `dm_report_view` gives `dm_columns()` (thirteen) and `dm_keep(status)`.

**Steps (failing test first for each):**

- [ ] Search keeps D56 item 1: it folds with `app/library_query`'s `fold_for_search` and matches every word in any order, with no query language (no `parse_library_query`). Each row's search text comes from `make_searchable`, through today's `html::search_field`. The search text covers the fields the page searches today: song, artist, charter and path for the path report; song, artist and charter for the comparison. No new fold table.
- [ ] The sort works on an index array. Empty values sink in both directions. A numeric column starts high-to-low and a text column starts A-to-Z.
- [ ] Cells read exactly as the pages write them. Take each rule from today's `cells` in `report.cpp:154-170` and `dm_report.cpp:103-123`, calling the existing C++ text functions; nothing is re-derived. The path report has em dashes, thousands separators, `ms_text`, `efill_text`, `mult_text`, "Beyond N ms" and "No squeezes". The comparison has "+N over", "✓", "#N", "N%" and the date's first ten characters.
- [ ] Definitions move word for word from the pages' `cols`. `__BASE_SPEED__` and `__SP_CAP__` are filled from `net::kBaseSpeedPercent` and `kCloneHeroSpCap`.
- [ ] Pin both column orders, each keep-rule, the two-key sort, empty values, `clear_search`, and the count line ("4 of 4 scores").

**Tests:** `-sf=*test_report_view*` only.

---

### Task 4: App state and jobs (wave 2, from the wave-1 tip)

**Goal:** both results live in memory and nothing writes or opens a file. The picker hands over to the comparison window. There are no windows yet; open flags stand in for them.

**Files:** see the ownership table.

**Steps:**

- [ ] `ReportJob` and `DmReportJob` stop calling `publish_report`. They keep their results, and `DmReportJob` loses `open_when_done`.
- [ ] AppState gains, for each report:
  - its result, as a `shared_ptr` to a const result
  - its built time
  - an out-of-date reason (none, library or settings), set when a batch finishes or the committed settings change after the build
  - its window's open flag
  - a request function that starts the job when none is running
- [ ] Add `select_chart(hyhash, chartmode)`. It switches the settings bar first when the mode differs, then selects the first library copy.
- [ ] Toolbar: "Open path report" shows once the library has analyzed charts. It stays enabled while it reads "Building path report...", and it sets the path window's open flag.
- [ ] Batch-done strip: drop "Show in folder". The second line reads "The path report is ready." "Open report" sets the open flag, and so does "Open automatically" when the job finishes. The hint reads "Open the path report as soon as it's built."
- [ ] The dm box: picking a player starts the job, closes the box and sets the comparison window's open flag. The box's last step goes. Add `reopen_dm_picker()` for the window's Cancel and "Compare another player..." buttons.

**Tests:**

- the touched cases in `test_app_state.cpp` and `test_library_jobs.cpp`
- `uitest_batch_reports.cpp`'s `batch-done-strip`, `report-buttons`, `settings-and-reports` and `batch-open-failure`, now checking the open flags instead of `opened_urls`
- the existing dm picker scripts

### Task 5: The window frame and both windows (wave 2)

**Goal:** the windows from the mock, drawn from a plain input struct. They don't touch AppState yet.

**Files:** see the ownership table.

**Steps:**

- [ ] Add `chip_color(token)` in theme, with the seven tier colours from `html_page.cpp:146-147` and the dim colour. Add a test that each passes 3:1 against #252526 and the chip text passes 4.5:1. If one fails, stop and report it; don't pick a colour.
- [ ] Add `struct ReportWindowInput`. It holds the title, heading words, subtitle, built time, state, progress, failure text, notice strip, tiles, controls, footer and button callbacks.
- [ ] Add `draw_report_window(input, view, columns)`. It uses T1's `report_window_class()` and `place_report_window`, and lays out the frame as on mock boards 1 and 4. The table uses the flags from `library_table.cpp`, including its imgui#9519 workaround. Freeze the header and the "#" column, use the list clipper, and make one Selectable span each row. Header hover text comes from the column definitions.
- [ ] Draw every state on mock boards 2 and 5.
- [ ] Esc and Ctrl+W close a window with focus. Arrow keys move the selection and fire the row callback.
- [ ] Add `path_report_window.cpp` and `dm_report_window.cpp`, which build the input and the view for each report. A "not in library" row gets no click callback; its hover text is "Not in your library".
- [ ] Add a test-only hook so `uitest_report_windows.cpp` can open either window on the sample rows. Cover sort, filters, Clear filters, each state, keyboard close, and both windows open at once.

**Tests:** `hydra_uitest` on `uitest_report_windows.cpp` only, plus the theme contrast test.

---

### Task 6: Wire the windows into Hydra (wave 3, from the wave-2 tip)

**Goal:** the open flags show the windows, and the windows' callbacks reach AppState.

**Steps:**

- [ ] `library_view.cpp` draws both windows each frame from AppState. The callbacks are row click to `select_chart`, Refresh to the request functions, Cancel to cancelling the job (the comparison's Cancel also calls `reopen_dm_picker`), and "Compare another player..." to `reopen_dm_picker`.
- [ ] Add end-to-end scripts for every item under "The windows" in the spec's Testing section that needs AppState: toolbar, strip and auto-open; the picker hand-over and Cancel; row click with a mode switch; the out-of-date strip and Refresh.

**Tests:** `hydra_uitest` on `uitest_report_windows.cpp` and `uitest_batch_reports.cpp` only.

### Task 7: Remove the pages, the plumbing and the CLI (wave 3)

**Goal:** nothing writes, opens or reads `hydra_paths.html` or `hydra_dmcompare.html`, and `hydra_report` is gone.

**Steps:**

- [ ] Delete each symbol in the spec's "What gets deleted", after grepping for every reader.
- [ ] Keep `open_in_browser`, `copy_to_short_temp` and `write_report_file` for `hydra_fillcompare`.
- [ ] Run the samples test and render `fill.html` in headless Edge before and after (ADR 0016's check). It must not change visibly. That settles which `html_page.cpp` pieces go.
- [ ] Remove the result structs' `html` field.
- [ ] Remove the uitest harness's browser and folder recorders.
- [ ] Remove the single-owner and clone-allowlist rows that name deleted code.

**Tests:**

- `-sf=*test_report.cpp*`
- `-sf=*test_dm_report*`
- `-sf=*test_s2_offspeed*`
- `-sf=*test_single_owner*`
- `-sf=*test_fill_report*`, if it exists; otherwise the fill cases in `test_report.cpp`

### Task 8: Docs and ADR (wave 3)

**Files:**

- `docs/UserGuide.md`: the toolbar, the Reports section, the dmleaderboards section and the `hydra_report` line.
- `docs/development.md`: the CLI flags and the report owners.
- `docs/agents/ui-testing.md`: the new windows' references and scripts.
- ADR 0016: a dated note that only the fill page still uses the shared page.
- A new ADR 0027, "Reports are Hydra windows": the decision, why multi-viewports over a second context, and the costs (mixed DPI, screen readers, the spike's popup findings).

**Rule:** docs point at owners and never restate a rule. Plain English.

---

## The join (main session)

1. **Row comparison.** On `testdata/input` and a recorded dm score set, dump the old build's page payloads and the new views' rows with a scratch tool. Compare them field for field and expect zero differences.
2. **Full suite once:** `hydra_tests` and `hydra_uitest --all`.
3. **Hands-on OS window check list** from the spec's Testing section, run against both windows, with screenshots sent to the user.
4. **Memory.** Measure path rows and memory on the corpus and scale the numbers to the library's 18,811 charts. Ask the user whether to run the one real-library check. If the number is large, ask whether rows should be freed when the window closes.
5. **Release notes line:** the path report and the dmleaderboards comparison now open in Hydra, and `hydra_report` is gone.

## Running it as a Workflow

Each wave is one Workflow run. Each task's agent runs with `isolation: "worktree"` and `model: "opus"`, and its Sonnet derive-once review is pipelined after it, so reviews start as soon as each task finishes. Every brief carries:

- the preamble path
- the status-file rule
- its row of the ownership table
- its test filters
- the spec path

Wave 1 has three agents and three reviewers, wave 2 two and two, wave 3 three and three. That keeps each run within the medium size guideline. The main session merges between runs and does the T1 hands-on check before wave 2 starts.
