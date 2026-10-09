Read docs/superpowers/plans/tasks/_rw-preamble.md first; it holds the rules.

# Task T8: the docs and ADR 0027

Task id: RW-T8. Base: `1a0bdd8` (main, the wave-2 tip plus the wave-2 handoff). Branch `claude/rw-t8-docs`, worktree `.claude\worktrees\rw-t8-docs`; make it as the preamble says.

Your spec is the plan's "Task 8" section and the spec's "What gets deleted" ("Docs"). The wave-2 handoff, `docs/handoffs/2026-10-08-report-windows-wave2-handoff.md`, says what the ADR must cover. You write docs only; no code, no builds, no tests.

## What T6 and T7 are doing at the same time

You document the finished state, not the base. On `1a0bdd8` the windows exist but nothing opens them, and the pages and `hydra_report` still exist. By the time your branch merges (first, before T7 and T6), T7 will have deleted the pages, the file plumbing and `hydra_report`, and T6 will have wired the windows in. Write as if both are done. Their briefs are `rw-t6.md` and `rw-t7.md` beside this one; read them for names. If a name you need isn't fixed in either brief, write it as the brief says and list it in your report so the main session checks it after the merge.

## What to write

**`docs/UserGuide.md`.** The toolbar's "Open path report" (shows once the library has analyzed charts, and also while a report is loaded or building, D103 item 20). The Reports section: the path report opens in its own Hydra window, what each part of the window is (header, strips, tiles, controls, table, footer), the states, and the keyboard. The dmleaderboards section: picking a player opens the comparison window; Cancel and "Compare another player..." reopen the picker. Remove the `hydra_report` line. Say that old `hydra_paths.html` and `hydra_dmcompare.html` files in `Documents\Hydra` are no longer read or written, and Hydra leaves them where they are (the spec's "Old files on disk"). Explain when things happen, not how the code decides them, and point at the window for the details.

**`docs/development.md`.** Remove `hydra_report`'s CLI flags. Name the owners from the spec's "How the code is arranged": tiles in `report.h` / `dm_report.h` (`path_tiles`, `dm_tiles`), the table view in `app/report_view.h`, columns and keep-rules in the two `*_report_view` files, the window frame in `ui/report_window.{h,cpp}`, chip colours in `theme`, and the reports' state in AppState's `ReportSlot`s. Point at each; don't restate any rule.

**`docs/agents/ui-testing.md`.** The report windows' references (window names, the test-only hook T5 added for sample rows) and their scripts. T5's are `report-window-sort`, `report-window-filters`, `report-window-states`, `report-window-keys` and `report-windows-both`. T6's are `report-window-open-path`, `report-window-dm-handover`, `report-window-row-click`, `report-window-reopen` and `report-window-out-of-date`. Remove any mention of the browser and folder recorders.

**ADR 0016.** A dated note (2026-10-08) that only the fill page still uses the shared page; the path and dm pages are gone (ADR 0027).

**ADR 0010 and ADR 0002.** Both mention `hydra_report` or the path report page. Add a dated note to each that says what changed and points at ADR 0027. Don't rewrite their decisions.

**New ADR 0027, "Reports are Hydra windows".** Follow the house ADR format (read two recent ADRs, 0025 and 0026). Cover:
- the decision: the path report and the dmleaderboards comparison are Hydra windows, each its own OS window, and the HTML pages and `hydra_report` are gone (D103);
- why ImGui multi-viewports rather than a second ImGui context (the spec's "What the research says");
- the `imgui.cpp` patch for D103 item 15. ImGui has no setting to keep popups and tooltips inside the window that opened them. GitHub issue ocornut/imgui#4624 asked for one and was closed without an answer. So it's one marked `// Hydra:` block in `third_party/imgui/imgui.cpp`, which must be kept when ImGui is updated;
- that `hydra_ui.ini` now carries a `ViewportPos` line per report window, and a saved spot that's off every monitor falls back to the main window's rectangle (D103 item 14);
- the costs: mixed DPI, screen readers, and what the spike found about popups (the spec's "Risks");
- D103 items 14 to 22, each named in one plain line.

Check the issue number and its state on GitHub (`C:\Program Files\GitHub CLI\gh.exe issue view 4624 --repo ocornut/imgui`) before you cite it; if it says something different, cite what it says.

**Not yours:** the release notes line (the main session writes it at the join), the plan and briefs under `docs/superpowers`, and `docs/handoffs`.

## Rules

Docs point at owners and never restate a rule. Plain English: one idea per sentence, short sentences, a one-line gloss for any jargon. Name D103 items by number where a reader would ask "who decided this?".

## Owned files

`docs/UserGuide.md`, `docs/development.md`, `docs/agents/ui-testing.md`, `docs/adr/0002-*.md`, `docs/adr/0010-*.md`, `docs/adr/0016-*.md`, new `docs/adr/0027-reports-are-hydra-windows.md`, and the ADR index if `docs/adr` has one.

## Tests

None. Before you return, grep your changed docs for `hydra_report`, `hydra_paths.html`, `hydra_dmcompare.html`, `Show in folder` and `browser`, and say in your report why each remaining hit is right.

## Return

`complete`, `branch`, `worktree`, `tip`, `report`, `questions`, `handoff`.
