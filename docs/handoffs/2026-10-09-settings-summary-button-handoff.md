# Replace the settings row with one summary button (handoff, 2026-10-09)

The "Analysis settings" bar under the toolbar is crowded. It holds a two-line caption and the controls for eight settings (ten widgets, since Score range and Path limit take two each) with their help markers, and on a narrow window its four blocks wrap onto extra lines. On 2026-10-09 the user picked a fix and asked for this handoff. They said "yes that all looks good. create a handoff for the next session to build". So the next session builds this as written. The design is settled; don't reopen it. If something in the code makes part of it impossible, stop and ask the user rather than picking a new design.

## What the user approved

**One button replaces the row.** The settings row goes away. A button at the end of the action row, after "Compare with dmleaderboards...", shows the current settings. With every setting at its default it reads "Analysis settings: defaults". When some differ, it lists only those, joined with " · ", for example "Analysis settings: Hard · Note Shuffle". If the window is too narrow for the whole label, hovering shows the full text.

**The phrase for each changed setting.** Each phrase is the control's own name plus its new value, so it matches what the panel shows. The user approved this exact wording. The defaults are what a default-constructed `app::Settings` holds (`src/app/config.h`, from line 68).

| Setting | Default | Phrase when it differs |
|---|---|---|
| Difficulty | Expert | the difficulty's name, e.g. "Hard" |
| Pro Drums | on | "Pro Drums off" |
| 2x Bass | on | "2x Bass off" |
| Note Shuffle | off | "Note Shuffle" |
| SP cap | 4 bars | "SP cap 5 bars" (the new value) |
| 1.0 fills | off | "1.0 fills" |
| Score range | 4 scores | "Score range 100 points" (the new value and unit) |
| Path limit | on, 10 ms | "Path limit 20 ms" (the new value), or "Path limit off" |

The phrases appear in the table's order. Score range counts as changed when the number or the unit differs. Path limit counts as changed when the tick or the ms value differs.

**The panel.** Clicking the button opens a popup with every control that is on the bar today. It has the same help markers and tooltips, laid out as a two-column form in three groups headed "Chart" (Difficulty, Pro Drums, 2x Bass, Note Shuffle), "Clone Hero rules" (SP cap, 1.0 fills) and "Paths kept" (Score range, Path limit). These are the existing controls, moved. Changes still apply the moment you make them and save to the INI exactly as now, through `commit_settings` and `edit_settings`. There is no Apply button. Ticking a box doesn't close the panel. Clicking outside it or pressing Esc does.

**While a batch runs.** The panel still opens. Its controls are greyed out by the same `begin_disabled_*` helpers as today. "Stop the batch to change these." sits at the bottom of the panel, and the button's label ends in " (locked)". One function reads `app.settings_lock()` once per frame and passes the result to both the button and the panel. `render_actions_row` already has its own `batch_busy = app.batch_running()` (`src/ui/library_toolbar.cpp` line 49). That value must not feed the button or the panel. A second lock source is what the scan test "What locks the analysis settings?" in `tests/test_single_owner.cpp` (near line 3409) exists to stop. Its comment says the decision is read once per frame so a batch ending mid-frame cannot send the bar to the one-song message with no analyze job behind it. With two readers, a batch ending mid-frame could likewise leave the button saying "(locked)" over enabled controls. If the read moves to another file, move that entry's file list and its required lines with it.

**The mock.** The user chose option A from `docs/handoffs/2026-10-09-settings-bar-mocks/settings_bar_mocks.html` (tab "A", with "only changes from default" selected). Build the layout and the look from that mock, but take every piece of text from this handoff. The phrase table and the label rules above win wherever the mock's text differs. The mock writes "SP 5 bars", "≤ 20 ms", "no path limit" and "100 points" without "Score range". Its label has no colon after "Analysis settings", and it shows a lock as the word "locked" in place of the arrow instead of " (locked)" at the end of the label. Its footer sentence says changing settings marks songs Stale, and that is wrong. Leave it out; the panel has no footer except the lock line. `CONTEXT.md` ("Summary row") says results are filed under the settings they ran with. A new setting combination shows charts as Not analyzed, and Stale means an older build or older rules.

## How to build it

**Say each phrase once.** The batch confirm already turns settings into text: `batch_settings_summary` in `src/ui/library_dialogs.cpp` (line 72). Its strings differ from the button's, for example "4 bars (Clone Hero's rule)" and "off". So don't force them to match. Share the pieces underneath instead: the counted "5 bars", "100 points" and "20 ms" values, and the difficulty name. Both summaries should call one small formatter per value. The ms value isn't made by `counted()` today; batch_settings_summary builds it inline, so that inline code becomes the shared formatter. Put the new changes-only function next to `batch_settings_summary` and declare it in `src/ui/library_parts.h`. Compare against `app::Settings{}`; never write a default down a second time. Read 2x Bass through `effective_bass2x()`, as the bar and the batch summary already do.

**Move the controls.** The four block functions in `src/ui/settings_bar.cpp` (`render_difficulty`, `render_sp_cap`, `render_score_range`, `render_path_limit`) already take `(AppState&, bool locked)`. They sit in an anonymous namespace in that file, so draw the popup from settings_bar.cpp too, or move them out. They can be called from inside the popup with small layout changes. How the label is cut on a narrow window, and where, is the builder's choice, as long as hovering shows the full text. The hand-built line wrapping goes away, together with `fits_in_row`'s use there, the divider drawing and the width cache `settings_block_w` in `src/ui/app_state.h` (line 120). `render_main_window` in `src/ui/library_view.cpp` (line 193) stops calling `render_settings_bar`. The button is drawn from `render_actions_row` in `src/ui/library_toolbar.cpp`, or from a function that one calls.

**Two scan-test entries name the old code.** `tests/test_single_owner.cpp` line 5527 lists settings_bar.cpp's right-align line as a known copy. Remove that entry if the line goes. The lock entry near line 3420 names settings_bar.cpp as the file that reads `settings_lock`. Keep it true wherever the read ends up.

**Dear ImGui facts, checked against `third_party/imgui/imgui.h` (1.93.0 WIP, docking branch).** Open the popup with `OpenPopup` once on the click, and draw it with `BeginPopup` at the same ID-stack level. Place it under the button with `SetNextWindowPos` right after the button, before any other item. Only `Selectable` and `MenuItem` close their popup when clicked (imgui.h line 866). Checkboxes, `InputInt` and `Combo` don't, so the panel stays open without extra flags. Write widths in terms of `GetFontSize()` or `GetFrameHeight()`, or the existing `px()`, never raw pixels.

## Tests

**GUI tests to update.** Four GUI test files click these controls today, and each must open the panel first:

- `tests/ui/uitest_library.cpp`, around lines 104-133, 187-202, 291 and 505-580;
- `tests/ui/uitest_details.cpp`, around lines 68-120 and 1002-1060;
- `tests/ui/uitest_batch_reports.cpp`, around lines 158-162, 215-222 and 331-345;
- `tests/ui/uitest_preview.cpp`, around lines 677-687.

Several of them use `SetRef(WindowInfo("//Hydra/##settingsbar"))`, and `uitest_details.cpp` reaches the controls by wildcard paths under the main window, such as `//Hydra/**/##spcap` and `**/Pro Drums`. Both break. The child window goes away, and a popup is a top-level window of its own, outside `//Hydra`. A research scout read that ImGui names it `##Popup_%08x` (imgui.cpp line 13355). It also read that the test engine's `//$FOCUSED` ref reaches whatever window has focus, such as an open popup. Neither has been tried in `hydra_uitest`, so prove one working path first and use it everywhere. A small helper that opens the panel and sets the ref would keep this in one place.

**Part of the layout test goes.** `test_library_layout` in `uitest_library.cpp` (lines 474-589) checks more than the bar, so don't delete it whole. Its bar-geometry checks go, which are the wrapping and caption-centring checks at about lines 502-519 and 570-588. The checks on the library chips and the Title column stay. The six-digit Score range check (lines 521-534, audit finding 112) moves into the open panel. Then add tests that check:

- the button reads "Analysis settings: defaults" on a default start;
- the label changes when a box is ticked;
- the panel stays open after ticking a box;
- the controls are disabled and the button says "(locked)" while a batch runs.

**A unit test for the wording.** Add a unit test for the changes-only function that pins each phrase in the table above as a literal.

**Docs to update.** Update the label cheat sheet in `docs/agents/ui-testing.md` (around line 98). Also update the User Guide paragraphs that describe the bar (`docs/UserGuide.md`).

## How to run it

1. **Build.** One Opus agent builds everything in its own worktree, because every edit touches the same few files. Its brief points at `docs/agents/brief-preamble.md` and names the status rule. It runs only the tests above, by filter, never the whole suite and never a library run.
2. **Review.** A fresh Sonnet agent does the derive-once review (`docs/agents/derive-once-review.md`). The summary text and the defaults are the obvious places for a second copy of a rule to sneak in.
3. **Screenshot.** Before merging, take a `hydra_uitest` screenshot of the button and the open panel, and send it to the user. Get their OK on how it looks.
4. **Merge.** Run the full suite once, then merge. The doc changes go through the doc-review gate when `main` moves.

## State when this was written

The journal state when this was written:

- agent-a10eec45ac881e8cb: unfinished, last tool call SubagentHandback, touched 12:26
- agent-aaebbe76d3990e5c3: unfinished, last tool call SubagentHandback, touched 12:27
- agent-aed3b40989bd3bfde: unfinished, last tool call SubagentHandback, touched 12:27

All three are the read-only research scouts. They show as unfinished only because their last tool call was the hand-back itself. Each one delivered its final report to the session that sent it, so nothing is still running. None of them edited a file.

No code has changed. The mock and this handoff are committed together.
