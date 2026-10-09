# Report windows: design (2026-10-08)

**Status:** design approved by the user on 2026-10-08 (mock boards 1 to 5), and this written spec approved the same day.
**Mock:** `docs/superpowers/specs/2026-10-08-path-report-window-mockup/report-window.html`.
**Plan:** `docs/superpowers/plans/2026-10-08-path-report-window.md`.
**Base:** main at d7b8001 (Hydra Deluxe 2.3.0).

## What we're building

Today Hydra's two GUI reports are web pages. After a batch, Hydra writes the path report to `Documents\Hydra\hydra_paths.html`, which is 28 MB on a real library, and can open it in the browser. The dmleaderboards comparison writes `hydra_dmcompare.html` and opens it the same way.

This change moves both reports into Hydra. Each one opens in its own Hydra window, which is a real Windows window, so you can drag it to another monitor. Both windows share one frame: the header, tiles, search, one dropdown, count line, table and states. They differ only in their rows and columns. The HTML pages, the code that writes them and the `hydra_report` command-line tool all go away.

The rows don't change. The same code builds them from the same analysis and the same leaderboard join, so every number reads as it does today.

The fill-spawn comparison stays an HTML page. Only the `hydra_fillcompare` command-line tool makes it, and it never appears in the GUI.

## Decisions

The user made these on 2026-10-08. They become D103 in `docs/audit/2026-10-03-fix-decisions.md` before any agent starts.

**Both windows:**

1. Headers sort. The "Sort by" dropdown and its up/down button go. Clicking a header sorts by it, clicking again flips it, and Shift+click adds a second sort.
2. No export. The windows only show the report.
3. Clicking a row selects that chart in Hydra. If the row's mode isn't the one on the settings bar, the settings bar switches to the row's mode first, so the Paths tab and the row always agree.
4. The column legend goes. Each column's definition shows as the header's hover text. The footer sentence each page has today stays, as a dim note under the table.

**The path report:**

5. `hydra_report` is removed.
6. The Charts tile is renamed "Charts shown". It counts the charts among the rows that pass the filters, matching "Paths shown" beside it. The subtitle keeps counting every chart.

**The dmleaderboards comparison:**

7. It moves into its own window.
8. Picking a player in the "Compare dmleaderboards user" box closes the box and opens the window. The box's last step (Done, Open report, Compare another, Close) goes away.
9. There's one comparison window. Comparing another player replaces its contents.
10. Cancel while scores are fetching closes the window and reopens the player list.
11. Refresh fetches the player's scores again and rebuilds the comparison.
12. Clicking a "not in library" row does nothing, because there's no chart to select. Hovering it says "Not in your library".
13. "Open automatically" now covers only the path report. The comparison always opens its window because you asked for it.

## What the research says

Three scouts read the code, Dear ImGui's documentation and issues, and desktop design guidance. The parts that shaped this design are below.

**Getting a second OS window.** Dear ImGui's docking branch, which Hydra already vendors (1.93.0 WIP), can put any ImGui window in its own OS window. ImGui calls each extra OS window a "viewport". The DX11 backend says this is "STRONGLY preferred" over running a second ImGui context in a hand-made second window ([imgui_impl_dx11.cpp](https://codebrowser.dev/imgui/imgui/backends/imgui_impl_dx11.cpp.html), [Multi-Viewports wiki](https://github.com/ocornut/imgui/wiki/Multi-Viewports)). A single window can opt in through `ImGuiWindowClass` overrides; the maintainer suggests exactly that in [ocornut/imgui#3152](https://github.com/ocornut/imgui/issues/3152). The rest of Hydra then stays in the main window as today.

**Known traps.** Four came up:

- Hydra's main loop stops drawing while its window is minimized or covered (`src/ui/main.cpp:326-332`), so a report window on another monitor would freeze.
- A saved position on a monitor that's since been unplugged can put a window off-screen ([ocornut/imgui#4236](https://github.com/ocornut/imgui/issues/4236)).
- ImGui marks its per-monitor DPI options experimental ([imgui.h](https://raw.githubusercontent.com/ocornut/imgui/docking/imgui.h), [FAQ](https://github.com/ocornut/imgui/blob/docking/docs/FAQ.md)).
- ImHex, a large ImGui app, ships a setting to turn its extra OS windows off because some systems handle them badly ([ImHex v1.24.2](https://github.com/WerWolv/ImHex/releases/tag/v1.24.2)).

**Testing.** This is verified in our copy of ImGui. When the backend has no viewport support, ImGui turns the feature off by itself and keeps every window inside the main one (`third_party/imgui/imgui.cpp:11823-11843`). `hydra_uitest` runs with no OS backend, so there each report is an ordinary in-app window. The test engine finds windows by name across all viewports, so the same references work in both setups.

**Windows guidance.** Microsoft's window-management guide ([win-window-mgt](https://learn.microsoft.com/en-us/windows/win32/uxguide/win-window-mgt)) covers long-lived modeless windows like these:

- They get Minimize and Close.
- They first open centred on their owner, a little above centre, sized for their content but not maximized.
- They have a minimum size below which the content stops working.
- They save their placement on close and check it against the current monitors on reopen.
- Restoring the main window restores them too.

Esc closes a window ([keyboard guide](https://learn.microsoft.com/en-us/windows/win32/uxguide/inter-keyboard)). An extra window should be a convenience, not a requirement ([multiple views](https://learn.microsoft.com/en-us/windows/uwp/ui-input/show-multiple-views)).

**Tables and filters.** NN/g says to freeze the header, pair light striping with row hover, and make an active filter obvious ([data tables](https://www.nngroup.com/articles/data-tables/)). GOV.UK right-aligns numbers ([table](https://design-system.service.gov.uk/components/table/)). Carbon shows a sort arrow only on the sorted column ([data table](https://carbondesignsystem.com/components/data-table/usage/)). A filter that matches nothing should say so and offer a way out, which here is Clear filters.

**Progress.** Show progress after about a second, with a count for record-by-record work and a Cancel for anything long ([progress indicators](https://www.nngroup.com/articles/progress-indicators/), [response times](https://www.nngroup.com/articles/response-times-3-important-limits/)). When outside events change the data, say so rather than changing it silently ([visibility of system status](https://www.nngroup.com/articles/visibility-system-status/)).

**Contrast.** Text needs 4.5:1, and controls and focus marks need 3:1 (WCAG 2.2 [contrast](https://www.w3.org/WAI/WCAG22/Understanding/contrast-minimum.html), [non-text contrast](https://www.w3.org/WAI/WCAG22/Understanding/non-text-contrast.html)). The chip colours are the HTML pages' dark-scheme colours, which were checked against the page background. They need checking again against Hydra's #252526.

## How the windows work

**One ImGui context, extra OS windows.** Hydra turns on multi-viewports and calls the two platform-window functions after each frame, as the upstream Win32 + DX11 example does. Both report windows carry one shared window class, and that class does four things:

- It always gets its own OS window and never merges into the main window.
- It gets a real Windows title bar with Minimize, Maximize and Close.
- It gets its own taskbar button.
- The main window is its OS owner, so it minimizes and restores with Hydra and sits in front of it.

Every other ImGui window keeps today's settings, so nothing else tears off. The comment in `setup_imgui` that says viewports are deliberately off is rewritten to say why the report windows opt in.

**Drawing while Hydra is covered.** The main loop's skip-while-occluded check changes. It now skips a frame only when every viewport is minimized or covered. Because the report windows are owned, minimizing Hydra minimizes them too, so the idle case costs what it does today.

**Placement.** On first open, a report window is centred over the main window and capped to that monitor's work area. After that, `hydra_ui.ini` remembers each window's position and size. On every open, the saved rectangle goes through the existing `placement_on_screen` (`src/ui/app_shell.cpp:118`), the same check the main window uses. If the rectangle fails it, the window falls back to first-open placement. Each window has a minimum size where the controls row and the first five columns still fit.

**Opening and closing.** Each window has one instance. Opening it again brings it to the front. The title-bar X, Esc or Ctrl+W hides the window. Its rows stay in memory, so reopening is instant.

**The path report's entry points.** "Open path report" shows once the library has analyzed charts; today it needs a report file to exist. While a batch's report builds it reads "Building path report..." as today, but it stays clickable and opens the window in its building state. The batch-done strip keeps "Open report" and "Open automatically" but loses "Show in folder". Its second line reads "The path report is ready." instead of naming a file.

**The comparison's entry point.** The toolbar's "Compare dmleaderboards user" button and its rules check are unchanged. The box still loads the user list and lets you pick a player. Picking one closes the box and opens the window, which shows today's two waiting sentences and a Cancel button. "Compare another player..." in the window reopens the box.

## What the windows show

The mock's board 1 is the path report and board 4 is the comparison. Both use the same frame. From top to bottom it has:

- **Title bar:** "Path report — Hydra", or "dmleaderboards: <player> — Hydra".
- **Header:** "Hydra Path Index" or "Hydra vs dmleaderboards", each page's subtitle as today, "Built HH:MM", and Refresh. The comparison also has "Compare another player...".
- **Notice strip,** only when one applies. For the path report that's the left-out charts: today's sentence, "Left out: N charts whose file couldn't be read.", with the files folded under "Show files".
- **Tiles:** five for the path report and nine for the comparison. They're worked out from the rows that pass the filters, as today, and wrap onto a second line in a narrow window.
- **Controls:** search, plus one dropdown each. The path report's is Timing ("All timing tiers" and each tier), with Best path only beside it, ticked by default. The comparison's is Status, today's eight choices. The count line, "X of Y paths" or "X of Y scores", sits at the right.
- **Table:** the "#" row number, then each page's columns in today's order. That's fifteen for the path report and thirteen for the comparison.
- **Footer note:** each page's footer sentence today, word for word, dim.

**Table behaviour.** It uses the same table flags as the library table (`src/ui/library_table.cpp`):

- The header row and the "#" column are frozen.
- Columns can be resized and hidden, and their widths are saved in `hydra_ui.ini`.
- Rows are striped and highlight on hover, and the selected row stays highlighted.
- The list clipper draws only the visible rows, so 40,000+ rows scroll smoothly.

Each window's first sort is today's: Score, highest first, for the path report, and Points left, most first, for the comparison. Empty values sink to the bottom in both directions. A new column starts high-to-low if it holds numbers and A-to-Z if it holds text. Both rules come from the HTML pages.

**Cells.** Every cell reads exactly as the HTML page writes it today. That includes:

- ellipsis truncation with the full text on hover, using `text_ellipsized` and `overflow_tooltip` in `src/ui/widgets.h`
- the mono font for Path
- right-aligned numbers, with thousands separators where the page has them
- the C++-made text for Hardest ms, Early fill, Avg multiplier and % of opt
- a dash for no value
- the gold bar on a best path
- "+N over" in red for a score above optimal
- "✓" for a full combo, "#N" for rank, and the posted date's first ten characters

Timing and Status are outlined chips in their colour. A path chip reads "Beyond N ms" or "No squeezes". A comparison chip reads the status itself, coloured as today: green for under and at optimal, yellow for above, grey for not in library, and the dim colour for not analyzed, no paths and other speed.

**States.** These come from the mock's boards 2 and 5. Every state except "nothing to report" is shared by both windows.

- **Building (path report):** a progress bar reading "Analyzing n of N records" (D103 item 25; with nothing left to analyze it moves with no count, item 26), Cancel, and greyed placeholder rows. Cancel shows "Report cancelled." with Try again.
- **Building (comparison):** today's sentences, "Fetching scores and building the report..." and "The leaderboard server can take a moment to wake up.", with a moving bar and Cancel. The server gives no count, so the bar only shows that work is happening.
- **Failed:** today's sentence for each report ("The path report could not be built." or "Could not build the report."), its message, its error and Try again. The comparison also offers "Compare another player...".
- **Out of date:** a strip saying the library or the settings changed since the report was built, with Refresh. The old rows stay readable under it. It shows after a batch finishes or the committed settings change.
- **No rows pass the filters:** "Nothing matches those filters." and Clear filters. Clear filters empties the search and resets the dropdown to all. Best path only is left as it is.
- **Nothing to report:** for the path report, its existing empty-reason text. For the comparison, a player with no scores shows today's empty result.

**Keyboard.** Tab moves through the controls and the table. Arrow keys move the row selection, and each move selects the chart in Hydra, like a click. Esc and Ctrl+W close the window.

**New words on screen.** These are the only new user-visible strings, and they're all in the mock:

- "Built HH:MM"
- "Refresh"
- "Report cancelled." and "Try again"
- "Clear filters"
- "Show files"
- "Your library changed since this report was built."
- "The settings changed since this report was built."
- "The path report is ready."
- "Analyzing n of N records" (D103 item 25)
- "Compare another player..."
- "Not in your library", as the hover text on a "not in library" row
- "Charts shown"
- "Open the path report as soon as it's built.", the new hint on "Open automatically"

Everything else reuses today's words.

## How the code is arranged

Every Hydra rule is worked out once. Today the pages' JavaScript computes tiles, filters, the search match and the sort, which are second copies of rules C++ also needs. The dm page even carries a mirror of `tally_dm_rows`, with a test whose only job is to check the two agree. Each of these rules moves to one C++ home.

**The data layer stays.** `report::generate_report` still builds `ReportRow`s, reuses the batch's charts through `ReportSeed`, and lists left-out files. `dm_report::generate_dm_report` still joins scores to records. Both result structs lose their `html` string. `GeneratedReport` gains `subtitle` and `footer`, the strings it builds today and passes to the page. `GeneratedDmReport` gains `rows`, `subtitle`, `footer`, `username` and `chartmode`.

**The tiles live with each report's data.** `report::path_tiles(rows)` takes over the page's five tiles. The "copies once per chart" rule behind Charts shown moves there from `page_charts`. Hardest ms shows that row's `ms_text`, and "Past N ms" reads `beyond_edge_ms`, the one `tier_for` uses. `dm_report::dm_tiles(rows)` takes over the nine. It calls `tally_dm_rows` for the counts. It also takes over two numbers that only the JavaScript computes today: the average % of optimal (a mean of `pct_h`, rounded half up and written as `format_percent` writes it) and the points left on the table. The status-to-chip-colour table moves next to `tally_dm_rows`.

**A shared table view, `app/report_view.{h,cpp}`.** This is plain C++ with no ImGui, so doctest can pin it. A `TableView<Row>` holds the rows, each row's search text, a keep-rule from the window's controls, and up to two sort keys. It answers `visible()` with the row indices to draw, in order, and the count line. Search folds text with the library's own `fold_for_search` and builds each row's search text with `make_searchable` (`src/app/library_query.h`), so accents fold the same way in all three search boxes. As D56 item 1 decided, a report's search matches words only; the Library alone reads the full query language.

**Each report's columns, `app/path_report_view.{h,cpp}` and `app/dm_report_view.{h,cpp}`.** These hold each report's column table: id, title, numeric or text, definition, sort key and cell text. The definitions move word for word from the pages, with `__BASE_SPEED__` and `__SP_CAP__` filled from their constants as today. They also hold each report's keep-rule: Best path and Timing for the path report, Status for the comparison.

**The window frame, `ui/report_window.{h,cpp}`.** This is all the shared ImGui code. It draws the title, header, notice strips, tiles, controls, count line, table, footer note and every state from a plain input struct. That struct holds the result, the state, the built time, the out-of-date reason and the button callbacks. The frame never sorts, counts or filters by itself. Two thin files fill it in: `ui/path_report_window.cpp` and `ui/dm_report_window.cpp`.

**Chip colours.** One owner, `chip_color(token)` next to the theme colours in `src/ui/theme.{h,cpp}`. It holds the seven dark-scheme tier colours and the dim colour.

**Row click.** A new `AppState::select_chart(hyhash, chartmode)` switches the settings bar to `chartmode` when it differs. It then finds the chart's library entry by hash and calls the existing `select`. A chart with several library copies selects the first in the library's current order.

**The jobs.** `ReportJob` and `DmReportJob` stop writing and opening files. Each hands its result to AppState, which owns both results, their built times, their out-of-date reasons and the two windows' open flags. A window can start its own job from Refresh, or when it opens with nothing in memory.

## What gets deleted

**The pages.** `src/app/report.cpp` loses `build_html`, `kBody`, `kPageJs`, `kTitle`, `py_repr`, `ms_text_into` and `beyond_edge_text`, each after checking nothing else reads it. `page_charts` moves into `path_tiles`. `src/app/dm_report.cpp` loses `build_dm_html`, its `kBody`, `kPageJs`, `kTitle` and `page_template`.

**The file plumbing the GUI used.** `src/app/report_files.{h,cpp}` loses:

- `reports_dir` and its Documents seam
- both file names and both path functions
- `report_file_exists`
- `open_report_in_browser` and `open_dm_report_in_browser`

`open_in_browser`, `copy_to_short_temp` and `write_report_file` stay, because `hydra_fillcompare` uses them. `src/ui/report_outcome.h` goes. So do `show_in_folder` and its seam in `src/ui/win32_dialogs.{h,cpp}`, whose only caller was the strip's Show in folder.

**Shared page code.** `src/app/html_page.cpp` keeps what the fill page needs. The CSS and script pieces only the path and dm pages used go. These include the tier and status chip classes, `.toggle`, `tr.best`, `.neg`, the `.dm` sizing rules and the `search_field` path argument. `--t*` variables stay only if the fill page reads them. T7 settles each piece by looking at the fill sample before and after.

**The CLI and the dialog.** `src/cli/report.cpp` goes, with its CMake target, its install line and its place in `hydra_tests`' dependencies. In `library_dialogs.cpp`, the dm box's last step and the strip's Show in folder go.

**Tests.** Tests that pin page HTML are deleted or rewritten against the views and tile functions:

- `tests/test_report.cpp`, the page cases and the `reports_dir` cases
- `tests/test_dm_report.cpp`
- `tests/test_s2_offspeed.cpp`, the page cases. "The page counts the same statuses" goes, because there's no second count left.
- `tests/ui/uitest_batch_reports.cpp`, the browser and folder recorders
- the single-owner scans and clone-allowlist rows in `tests/test_single_owner.cpp` that name deleted symbols

The samples test keeps writing the fill page only.

**Docs.** These need updating:

- `docs/UserGuide.md`: the toolbar, the Reports section, the dmleaderboards section and the `hydra_report` line
- `docs/development.md`
- `docs/agents/ui-testing.md`
- ADR 0016, which gets a dated note that only the fill page still uses the shared page

A new ADR 0027, "Reports are Hydra windows", records the decision.

**Old files on disk.** A user upgrading keeps their old `hydra_paths.html` and `hydra_dmcompare.html` in `Documents\Hydra`. Hydra never reads them again, and it doesn't delete them, because deleting a user's files needs their say.

## Testing

**Data layer and views.** doctest cases pin literals or call the production function:

- `path_tiles` on the six sample path rows and `dm_tiles` on the four sample scores. The comparison's literals are 4 scores, 1 under, 0 at, 1 above, 1 not analyzed, 1 not in library, 0 other speed, 98.80% and 3,456.
- Charts shown counting copies once per chart, the case `test_report.cpp` pins today.
- The shared view's search, keep-rule, two-key sort, empty values sinking, and count line.
- Each report's keep-rule, column order and definition text.
- Clear filters.

**The windows.** `hydra_uitest` scripts run headless, where each window lives inside the main viewport:

- the path report: open from the toolbar, open from the strip, auto-open after a batch
- the comparison: picking a player opens the window, Cancel reopens the list, "Compare another player..." reopens the box
- sort by header click
- filter to nothing, then Clear filters
- a row click selects the chart and switches the settings bar's mode when needed
- a "not in library" row click does nothing
- Esc closes a window, and reopening is instant
- both windows can be open at once
- the out-of-date strip shows after a batch and clears after Refresh

**Before and after.** On the `testdata/input` corpus, the rows the path window shows must match the rows the old page embedded, field for field. The same goes for a recorded set of dm scores. A small scratch tool dumps both, and the join compares them once.

**The real OS windows.** The main session runs one hands-on check list after the merge and sends the user screenshots:

- drag each window to a second monitor
- minimize and restore Hydra
- cover Hydra with another app and check the report keeps drawing
- check that a window saved on a missing monitor comes back on screen
- restart Hydra and check placement is kept

No whole-library run is planned. The one real-library check, which shows row count and memory on the user's 18,811 charts, waits for the user's yes in chat.

## Risks

**Turning on viewports reaches the whole app.** Upstream ImGui lets a tooltip or popup near a window's edge become its own OS window. The spike showed this, and D103 item 15 keeps every popup and tooltip inside the window that opened it, through one marked block in our copy of `imgui.cpp`, because ImGui has no setting for it.

**Memory.** Keeping 40,000+ path rows in memory after the window closes costs something. The 28 MB page suggests tens of MB. The plan measures it on the corpus. If it looks large, the alternative is freeing rows on close and rebuilding on open, which takes a few seconds. That choice goes to the user with the number in hand.

**Mixed DPI.** On two monitors with different scaling, the report windows keep the main window's scale. Text may look too large or too small on the second monitor. Per-monitor DPI stays out of scope while ImGui marks it experimental.

**Screen readers** can't read ImGui windows. The HTML pages could be read by one. That's a loss for anyone who used a screen reader on the reports, and it's named here so it's a decision, not an accident.

## Out of scope

- **The fill-spawn comparison.** It's command-line only and stays HTML.
- **Export and copy.** Decision 2.
- **A setting to keep reports inside the main window,** the ImHex-style off switch. Not added unless the spike finds a machine where the OS windows misbehave.
- **Per-monitor DPI scaling.** It's experimental in ImGui.
- **Saving filters and sort between runs.** Column widths and order persist through ImGui. Search and the dropdowns start fresh, as on the pages.
- **Tiles that act as filters.** Not asked for, and no source recommends it.
