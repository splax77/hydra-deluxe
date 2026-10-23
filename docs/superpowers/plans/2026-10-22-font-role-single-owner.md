# Font roles: one place decides which text gets which font

Approved by the user on 2026-10-22: "Approved. Yes to all five recommendations (Q1-Q5)." So T6 runs, and every question below takes its recommended answer.

Plan only. Nothing here is built until the user says yes. Written 2026-10-22 by the planning agent for task FONT-ROLE-PLAN (session festive-shamir-6b64ea). Every fact below was read from the code in this worktree at commit d278eb3a, plus branch `badge2-join` at 16944a4d; the inventory in Appendix A was produced by a script that walked every file under `src/ui`.

## The plain version

Hydra loads two fonts. The default is Shippori Antique B1 and the other is Courier Prime, a monospace font (every letter the same width). Today each draw call decides for itself whether to use the mono font. There is no rule anyone can read, only a vague comment in `src/ui/fonts.h`. So two pieces of text on one row can disagree, and their baselines drift apart. The user called this a derive-once violation.

The fix is one small header, `src/ui/text_role.h`, that names the kinds of text Hydra draws (a role) and maps each role to a font and a size. Every draw call and every measurement names its role and asks the owner. A scan test refuses any raw font pick outside that header. The visible result is the same screens, with a few places changing font so that like text looks alike. Each of those visible changes is a question for the user at the end of this plan.

## What the code does today

The two fonts are loaded in `src/ui/app_shell.cpp` lines 461 and 462, both at `kFontSize`, which is 18 (`constexpr float kFontSize = 18.0f;`, pinned by the rule "What size do the UI fonts load at?" in `tests/single_owner/rules/ui.json`). Line 463 makes Shippori the default. Lines 470 to 474 merge a Japanese system font into the default font only. So any text drawn in the mono font cannot show Japanese, which matters for chart titles.

`g_mono_font` is declared in `src/ui/fonts.h` line 14. When the font file is missing it stays null; `ImGui::PushFont(nullptr, ...)` keeps the current font, so the app still runs, and only one place (`src/ui/preview_tab.cpp` line 624) spells that fallback out.

The inventory script (`inventory.ps1`, in the agent's scratch folder; its output is Appendix A) found 459 lines under `src/ui` that draw or measure text. By the font in force at that line: 413 use the default font at its normal size; 29 sit inside a `PushFont(g_mono_font)` scope; 6 pass `g_mono_font` as an argument; 5 use the Preview overlay's `font` variable, which is the mono font when it loaded; 2 are the helpers `text_at` and `wrapped_height` in `paths_tab.cpp`, which take a font from their caller; and 4 are fixed-size pushes of one font (the song title at 26 px, the headline score at 40 px, the headline path in mono at 24 px, the batch question at 20 px). `PushFont` appears 20 times in `src/ui` (`git grep -c`), of which 15 push the mono font.

The mix came from the September mockup. `docs/superpowers/specs/2026-09-27-ui-redesign-mockup/Main.dc.html` marks these as mono: the search box (line 90), the Best path column (112), the headline path at 24 px beside a 40 px score that is not mono (135, 136), the path buttons and the all-0 path (154 to 162), the timeline's `m1` and `m96` (190), the activation row's notation and measure but not its number, bars or badge (194), the chord (198), and a note name inside a squeeze sentence (202). `Preview.dc.html` marks the overlay boxes (31), the path picker (120), the clock (144) and the `[n]/4` key hint (185) as mono. Commit 7afb806e built the Paths tab from that mockup. No user ruling chose the mix; the user's ruling of 2026-10-22 ("All mono" for the activation row, recorded as D257) overrides the mockup for that row.

Branch `badge2-join` is ahead of this plan on the row. Its tip 16944a4d ("Paths rows: dropped badges go under the measure and wrap; all row text in mono") wraps the whole activation row in one `PushFont(g_mono_font)` and drops the per-call font arguments from `text_at`. This plan treats that row as decided and done. The task that touches `paths_tab.cpp` starts from the join tip once 16944a4d is on `main`, so the two never collide.

### Measurements that use one font and draws that use another

The brief asked for each width measured in one font but drawn in another. The script marks every measuring call (`CalcTextSize`, `text_width`, `measure_in_font`, `mono_width`, `wrapped_height`, `text_slot_width`, `button_slot_width`) with the font in force, and the pairs were checked by hand. Two real mismatches exist, both in table headers.

The Library table's `Best path` column is measured with the mono font (`library_table.cpp` line 248: `measure_in_font(g_mono_font)`, and `ColumnSpec::width_of` "measures this column's header and cells" per `column_widths.h` line 72). Its cells are drawn in mono (line 434), but its header is drawn by `TableHeadersRow()` at line 316 with no font pushed, so the header is measured in Courier Prime and drawn in Shippori.

The report windows do the same for any column whose `CellLook::mono` is set. `report_window.cpp` line 399 measures the header and cells in the mono font, line 359 pushes mono around each cell, and the header row is drawn with no push. Only the path report's `Path` column sets `mono` (`src/app/path_report_view.cpp` lines 64 and 74).

Every other pair agrees. The Paths tab's path buttons measure the title in mono and the detail in default and draw them the same way (lines 105 to 124). The backend table measures and draws in mono (342, 388 to 421). The activation row's number, badge and detail are measured and drawn in default today, and all-mono on `badge2-join`. The Preview's clock slot is measured and drawn in mono (563, 576), the path picker likewise (154 to 180), and the overlay measures with the same `font` it draws with (633, 283, 794, 798). The report cells' `fitted_text` measures with `text_width` inside the cell's push (37 to 44, 359 to 361). The headline path is cut inside its 24 px mono push (`details_panel.cpp` 115 to 121).

There is one fragile pattern that is not yet a bug. `text_at` and `wrapped_height` each take an `ImFont*`, so a caller must pass the same font to both, as `path_button` does at lines 107 and 116. Two arguments can drift. A role argument replaces both.

## The roles

A role is the kind of text, not the font. A future draw call picks a role by asking "what is this text?" and never "should this be mono?". The set is seven roles plus four fixed-size variants that already exist in the code.

**Notation** is a path string, an activation's notation or a chord name, such as `3- 1 2`, `E3+` or `[Kick - GreenCym]`. Mono.

**Figure** is a number, timing, measure position, clock or count that sits in a column, a fixed slot or a row of its kind, where digits must line up across rows or frames. Examples: `m32.1.0`, `163 ms`, `0:35.000`, the score breakdown lines. Mono.

**Label** is a short word that names a thing: a heading, a column header, a fold name, a button or tab label, the word `Chord`. Default font.

**Prose** is a sentence or a wrapped paragraph: hints, tooltips, notices, squeeze sentences, dialog text, "None.". Default font.

**Title** is a chart's title, artist, charter or folder wherever it is drawn. Default font, because only the default font carries the Japanese fallback (`app_shell.cpp` 470 to 474).

**Control** is the text inside an input field, combo, slider or checkbox. Default font. The Preview's path picker is Notation, not Control, because what it shows is a path.

**Overlay** is the Preview highway's text boxes and the SP gauge's labels, drawn on the draw list at a size the overlay scale sets. Mono, size supplied by the caller.

The four fixed-size variants are **SongTitle** (default, 26 px; `details_panel.cpp` 70), **HeadlineScore** (default, 40 px; 110), **HeadlinePath** (mono, 24 px; 115) and **DialogQuestion** (default, 20 px; `library_dialogs.cpp` 351). They match the mockup (Main.dc.html 135 and 136) and change nothing visible.

## The owner

One header, `src/ui/text_role.h`, in namespace `hydra::ui`. It is the only file that reads `g_mono_font`, calls `ImGui::PushFont`, or asks `ImGui::GetFont()` for a font to draw with. `fonts.h` keeps the handle and `app_shell.cpp` keeps the loader; neither picks a role.

```cpp
enum class TextRole { Notation, Figure, Label, Prose, Title, Control, Overlay,
                      SongTitle, HeadlineScore, HeadlinePath, DialogQuestion };

struct FontPick { ImFont* font; float size; };   // size 0 means the current size

// The one rule: which font and size a role draws in. A missing mono font
// falls back to the current font here and nowhere else.
FontPick font_for(TextRole role);

// Pushes the role's font for the scope, pops on exit.
class ScopedFont { public: explicit ScopedFont(TextRole r); ~ScopedFont(); };

// Measuring through the role, so a width is always taken in the font it is drawn in.
ImVec2 text_size(TextRole role, const char* text, float wrap_w = 0.0f);
WidthOf measure_in_role(TextRole role);   // replaces measure_in_font(g_mono_font)
```

`font_for` is a `switch` with no default branch and a `static_assert` on the role count, so a new role that forgets its mapping fails to compile. `measure_in_role` calls `measure_in_font(font_for(role).font, ...)` in `column_widths.cpp`; that function stays as the width primitive and is exempt from the scan because it only measures in the font it is handed. The Preview overlay keeps its `text_width(size, text)` lambda but takes its font from `font_for(TextRole::Overlay).font` and passes it to `draw_text_box` and `AddText`, so the draw-list calls still pick nothing themselves.

`text_at` and `wrapped_height` in `paths_tab.cpp` take a `TextRole` instead of an `ImFont*`. `CellLook::mono` in `src/app/report_view.h` (line 50) becomes `CellLook::kind`, an enum of Text, Notation or Figure, because a bool named after a font is a font pick in the app layer; `report_window.cpp` maps the kind to a role in one place.

### The scan rule

This row goes in a new file `tests/single_owner/rules/ui/text_role.json` in the `{"rules": [...]}` form ADR 0029 allows for a new row, so it conflicts with no other branch. The decision field names the D-number the user's yes gets (the next free one on `main` is D258; D257 is taken on `badge2-join`).

```json
{
  "rules": [
    {
      "question": "Which font does a kind of text draw in?",
      "owner": "font_for in src/ui/text_role.h",
      "owner_file": "src/ui/text_role.h",
      "owner_symbol": "font_for",
      "date": "2026-10-22",
      "decision": "D258 (the user's yes to docs/superpowers/plans/2026-10-22-font-role-single-owner.md)",
      "added_by": "FONT-ROLE T5",
      "pattern": "\\bg_mono_font\\b|ImGui::PushFont\\(|ImGui::GetFont\\(\\)|->CalcTextSizeA\\(|io\\.FontDefault",
      "owner_files": ["src/ui/text_role.h"],
      "exempt": [
        {"file": "src/ui/app_shell.cpp", "why": "loads the two font files and sets FontDefault; it picks no role"},
        {"file": "src/ui/fonts.h", "why": "declares the handle the loader fills; the owner is its only reader"},
        {"file": "src/ui/column_widths.cpp", "why": "measure_in_font is the width primitive; it measures in the font the owner handed it"}
      ],
      "must_match": [
        "ImGui::PushFont(g_mono_font, 0.0f);",
        "ImFont* font = g_mono_font ? g_mono_font : ImGui::GetFont();",
        "dl->AddText(ImGui::GetFont(), num_size, ImVec2(*lefts[i], top.y + px(2.0f)),",
        "ImGui::PushFont(nullptr, 40.0f);",
        "widths.push_back(ImGui::GetFont()->CalcTextSizeA(num_size, FLT_MAX, 0.0f, num.c_str()).x);"
      ],
      "must_not_match": [
        "ScopedFont font(TextRole::Figure);",
        "text_at(ImVec2(top.x + l.measure_x, text_y), text_color(), a.measure.c_str(), TextRole::Figure);",
        "const WidthOf mono = measure_in_role(TextRole::Notation);",
        "const float w = ImGui::CalcTextSize(text.c_str()).x;"
      ],
      "owner_lines": [
        {"file": "src/ui/text_role.h", "line_text": "ImFont* const mono = g_mono_font ? g_mono_font : ImGui::GetFont();", "why": "font_for's one fallback, the owner"}
      ],
      "scope": ["src"],
      "note": "A raw font pick is any of: reading the mono handle, pushing a font, asking ImGui for the current font to draw with, or measuring through a font pointer. ImGui::CalcTextSize with no font argument is allowed: it measures in whatever font a ScopedFont pushed, so it agrees with the draw by construction. No anchor: the alternatives share no plain text."
    }
  ]
}
```

The self-test "single-owner rules match their own examples" runs the `must_match` and `must_not_match` lines, so a slip in the pattern fails at once.

## What changes on screen

Under the roles above, most text keeps the font it has today. The headline, the path buttons, the score folds, the chord, the backend table cells, the score breakdown, the Best path column, the Preview path picker, the clock and the overlays all stay mono; every title, sentence, label, button and input stays default. The activation row is all mono by D257 already. These five places differ, and each is a question for the user below because it is visible.

On the report windows, the Dynamics tab and the Stars tab, the number columns (the path report's Score, Acts, Max skip and Hardest ms; the DM report's counts; the Dynamics and Stars tables' figures) switch from the default font to mono. Today only the path report's Path column is mono.

In the Library, the search box switches from mono to the default font, so a typed Japanese title shows instead of boxes.

On the Paths tab, the Multiplier squeeze fold's "how to" sentence switches from mono to the default font, and so does its "None." line shown when the chart has no squeezes (`paths_tab.cpp` 537, inside the same mono push). Its label line (the notation and the points gained) stays mono.

Every table header row draws in the default font. The one visible change is the backend table's `Timing`, `Chord`, `Points` and `Rating` headers, mono today; the Library and report headers already draw in default, and this also fixes the two measured-in-one-font, drawn-in-another headers.

On the Paths tab timeline, the small activation numbers above the strip switch from default to mono, matching the numbers in the rows under them (D257).

Nothing in wording changes. `hydra_uitest` reads text, not fonts, so a text dump of each screen before and after a task must be identical; that is each task's proof that only fonts moved.

## Tasks

Each task is sized for one Opus executor under 100 tool calls and 20 minutes. Each owns the files it names, runs only the tests it names, and ends with a window-mode look at its screens: `hydra_start('window')` from the hydra MCP server opens the real window on a snapshot copy of the user's library, and the executor captures each changed screen (the report capture scripts, or a window screenshot) for the user to see before merge. The main session shows those captures to the user; the memory rule "UI changes need a real-app check" is not satisfied by tests alone. Every task also runs `hydra_uitest --test <name>` for its screens before and after and keeps both text dumps.

**T0, the owner.** Serial, first; everything else includes its header. Adds `src/ui/text_role.h` with the enum, `font_for`, `ScopedFont`, `text_size` and `measure_in_role`; rewrites the comment at the top of `fonts.h` to point at the owner (no history, no restated rule); adds `tests/test_text_role.cpp` and its `CMakeLists.txt` line. The test creates an ImGui context the way `tests/test_app_shell.cpp` does, sets `g_mono_font` to a stand-in `ImFont`, and pins for every role which font pointer and size `font_for` returns (a table with eleven rows, one per role, and a check that the table's length equals the enum's count). A second case sets `g_mono_font` to null and checks that every mono role returns the current font and no role returns null. Runs `hydra_tests.exe -sf=test_text_role.cpp`. About 25 tool calls. No screen changes, so no window look.

**T1, the Paths tab.** Owns `src/ui/paths_tab.cpp` and `tests/ui/uitest_paths.cpp`. Starts from the join tip once `badge2-join` 16944a4d is on `main`; its branch must satisfy `git merge-base --is-ancestor 16944a4d HEAD`. Replaces every push and font argument with roles: path buttons and folds Notation, the detail line Prose, the timeline labels and numbers Figure, the activation row one `ScopedFont(TextRole::Figure)` around the whole row with the notation drawn in Notation inside it (both mono, so the D257 baseline holds), the chord Notation, the backend table cells Figure and Notation with headers drawn outside the scope as Label, the Multiplier squeeze label Notation and its sentence Prose, the breakdown Figure. `text_at` and `wrapped_height` take a role. The existing scan row "Which way is text aligned?" (`tests/single_owner/rules/ui/text-alignment.json`) quotes one `text_at(...)` line of the activation circle's number as an owner line; if T1 changes that line, T1 updates the row's `line_text` in the same commit, and that file is T1's to edit. Runs the Paths tests from `tests/ui/uitest_paths.cpp` (names from `hydra_uitest --list`). Window look: a chart with a squeeze and an early fill, rows open and closed, the backend table and both footer folds. About 60 tool calls.

**T2, the Preview tab.** Owns `src/ui/preview_tab.cpp`. The path picker Notation, the clock slot Figure, the overlay font from `font_for(TextRole::Overlay)` handed to `draw_text_box` and the gauge labels. Runs the `preview` test from `tests/ui/uitest_preview.cpp`. Window look: the Preview during playback with the gauge and a solo. About 30 tool calls. Parallel with T1, T3, T4.

**T3, the Library and the song panel header.** Owns `src/ui/library_table.cpp`, `src/ui/details_panel.cpp`, `src/ui/library_dialogs.cpp` and `tests/test_column_widths.cpp` (if that is where `library_column_specs` is tested; otherwise a new `tests/test_library_columns.cpp` with its CMake line). The Best path column is measured with `measure_in_role(TextRole::Notation)` for its cells and drawn in Notation; its header is Label. The search box is Control if the user says yes to question 2, else it keeps a Notation scope. The headline uses SongTitle, HeadlineScore and HeadlinePath; the batch question uses DialogQuestion. Adds a test that the Best path column spec's header width equals `text_size(TextRole::Label, "Best path").x`, which fails while the header is measured in mono. Runs that test and the `scan` and `difficulty` tests from `tests/ui/uitest_library.cpp` plus the `analyze` test from `uitest_details.cpp`. Window look: the Library with a Japanese title in the search box and in the list, and the song panel header. About 50 tool calls. Parallel with T1, T2, T4.

**T4, the report windows.** Owns `src/ui/report_window.cpp`, `src/ui/report_window.h`, `src/ui/path_report_window.cpp`, `src/ui/dm_report_window.cpp`, `src/app/report_view.h`, `src/app/path_report_view.cpp` and whichever `src/app` file builds the DM report's columns, plus `tests/ui/uitest_report_windows.cpp`. Replaces `CellLook::mono` with `CellLook::kind`; the Path column is Notation; if the user says yes to question 1, the count and ms columns are Figure. `column_spec` measures cells through `measure_in_role` and the header through Label; `cell` pushes the role. Adds a test that a Notation column spec's header width equals the Label width of its title. Runs the report window tests from `tests/ui/uitest_report_windows.cpp` and the app tests for `path_report_view`. Window look: the path report and the DM report. About 60 tool calls. Parallel with T1, T2, T3.

**T6, the Dynamics and Stars tabs.** Only if the user says yes to question 1. Owns `src/ui/dynamics_tab.cpp` and `src/ui/stars_tab.cpp`. Their figure columns measure and draw in Figure. Runs the `dynamics` and `stars` tests from `tests/ui/uitest_details.cpp`. Window look: both tabs. About 25 tool calls. Parallel with T1 to T4.

**T5, the join.** Serial, last, after every task above is merged. Adds `tests/single_owner/rules/ui/text_role.json` as shown, with the decision's D-number. Runs `hydra_tests.exe -tc="*single-owner*"`, which must pass with no baseline entry: a leftover raw pick anywhere under `src` fails it, and that failure names the file. Confirms `git grep -n "g_mono_font\|PushFont(" -- src` lists only `text_role.h`, `fonts.h` and `app_shell.cpp`. Writes the D258 entry in `docs/audit/2026-10-03-fix-decisions.md` quoting the user's answers. About 20 tool calls.

T1 depends on the `badge2-join` merge, not on T2 to T4. If that merge is late, T2, T3, T4 and T6 run first and T1 joins the wave when its base exists.

## Open questions for the user

Each question quotes the rule as the code would apply it and gives an example from both sides. A recommendation follows; nothing is decided here.

**Question 1. Do number columns in tables draw in mono?** The Figure rule reads: "a number, timing, measure position, clock or count that sits in a column, a fixed slot or a row of its kind, where digits must line up across rows or frames, draws in the mono font". Under it, the path report's Score (`378,315`), Acts, Max skip and Hardest ms (`163.0`) columns, the DM report's counts, and the Dynamics and Stars tables' figures switch to mono. Today they are default; only the Path column is mono (`src/app/path_report_view.cpp` 64 to 96). On the other side, the backend table's Timing and Points columns and the score breakdown lines are mono today, and the saved HTML report pages put every numeric cell in monospace (`src/app/html_page.cpp` 154 and 155: `.mono, td.num, .path, .stat-v { font-family: ... monospace }`), so the window and the page currently show the same number in different fonts. Recommendation: yes, mono, so the in-app report agrees with its own HTML page and with the backend table. If no, the Figure rule narrows to "outside a table" and T6 is dropped.

**Question 2. Does the Library search box draw in the default font?** The Control rule reads: "text inside an input field, combo, slider or checkbox draws in the default font; the Preview path picker is Notation because it shows a path". Under it the search box (`src/ui/library_table.cpp` 103 to 107) goes from mono to default. The mockup chose mono (Main.dc.html 90). On the other side, a Japanese title typed into the box today shows as boxes, because the Japanese fallback is merged into the default font only (`app_shell.cpp` 470 to 474). Recommendation: default, so what the user types looks like the titles it searches. If no, the box takes a Notation scope and Japanese stays unreadable in it.

**Question 3. Does the Multiplier squeeze fold's "how to" sentence draw in the default font?** The Prose rule reads: "a sentence or wrapped paragraph draws in the default font". `msq.howto` is a sentence from `Multsqueeze::howto` (`src/app/path_view.cpp` 179), drawn in mono today (`paths_tab.cpp` 550) under its label, which is the notation plus the points gained (177 to 178) and stays mono. On the other side, the open activation's squeeze sentences, the same kind of text, already draw in default inside their warm box (319 to 335). The same answer covers the fold's "None." line (537), which is prose too and is mono today. Recommendation: default, so every squeeze sentence on the tab reads the same. If no, the fold keeps all its lines mono as one block.

**Question 4. Do table header rows always draw in the default font?** The Label rule reads: "a column header draws in the default font". Under it the backend table's `Timing`, `Chord`, `Points` and `Rating` headers go from mono (drawn inside the push at `paths_tab.cpp` 388 to 421) to default. On the other side, the Library's `Best path` header and the path report's `Path` header already draw in default while being measured in mono (the two mismatches above); the rule fixes both by measuring them as Label too. Recommendation: yes. If no, headers take their column's role and the Library and report headers go mono instead.

**Question 5. Do the timeline's small activation numbers draw in mono?** The Figure rule applies to "a row of its kind where digits must line up". The numbers above the Paths tab timeline (`paths_tab.cpp` 235, drawn at 70 % size with `ImGui::GetFont()`) are default today; the same numbers in the row circles are mono on `badge2-join` (D257). Recommendation: mono, so the number above the mark matches the number in the row. If no, they take Label and stay default.

## Out of scope

The saved HTML report pages keep their own CSS fonts (`src/app/html_page.cpp` 154 and 155); they are drawn by the browser, not ImGui, and only question 1 brings the window in line with them. The highway renderer in `src/render` draws no text (`git grep` of `AddText`, `ImFont`, `PushFont` and `CalcTextSize` outside `src/ui` finds nothing). `hydra_uitest` cannot read fonts, so the proof that a font changed is the owner's unit test plus the window-mode look; no task promises a text-engine font assertion.

## Promise map

| Promise | Plan line | Task | Check that proves it |
|---|---|---|---|
| Every font pick goes through `font_for` in `text_role.h`; no raw pick elsewhere under `src` | "The owner", "The scan rule" | T5 | `hydra_tests.exe -tc="*single-owner*"` passes with the new row and no baseline entry; `git grep` of `g_mono_font` and `PushFont(` under `src` lists only the three exempt files |
| Each role maps to exactly one font and size, and a new role cannot be left unmapped | "The owner" | T0 | `test_text_role.cpp` table with one row per role, its length checked against the enum count; `switch` with no default plus `static_assert` |
| A missing mono font falls back in one place and never to null | "What the code does today", "The owner" | T0 | `test_text_role.cpp` case with `g_mono_font` null: every mono role returns the current font, none returns null |
| Text is measured in the font it is drawn in (the two header mismatches fixed) | "Measurements that use one font..." | T3, T4 | T3's test: Best path header width equals `text_size(Label, "Best path").x`; T4's test: a Notation column's header width equals its Label width; both fail while headers are measured in mono |
| The activation row stays all mono (D257) | "What the code does today", T1 | T1 | One `ScopedFont(Figure)` around the row; Notation drawn inside it is also mono by T0's test; window look shows one baseline |
| The plan does not collide with `badge2-join` | "What the code does today", T1 | T1 | `git merge-base --is-ancestor 16944a4d HEAD` is true on T1's branch before it edits `paths_tab.cpp` |
| No wording changes; only fonts move | "What changes on screen" | T1 to T4, T6 | Each task keeps the `hydra_uitest` text dump before and after; the dumps are identical |
| No visible change ships without the user's yes | "What changes on screen", "Open questions" | T1, T3, T4, T6 | Each task implements only the questions answered yes; the D258 entry written in T5 quotes the answers |
| Japanese titles stay readable (Title and, if approved, Control are default) | "The roles", question 2 | T0, T3 | T0's test pins Title and Control to the default font; T3's window look shows a Japanese title in the list and search box |
| The scan rule's examples really match and really miss | "The scan rule" | T5 | The self-test "single-owner rules match their own examples" passes on the new row |
| Comments point at the owner and restate no rule or history | "T0" | T0, T1 to T4 | The reviewer's comment check (brief preamble, "If you write code"); `fonts.h` names `text_role.h` and nothing else |
| Each task fits one executor (under 100 calls, 20 minutes) and runs only its own tests | "Tasks" | all | Each task's call estimate and named tests above; status lines every 10 calls |
| The real-app check happens before merge | "Tasks" | T1 to T4, T6 | Each task's window-mode capture of its changed screens, shown to the user by the main session |

## Appendix A: every text draw and measure under src/ui

Produced by the planning agent's inventory script (scratch `inventory.ps1`) over every `.cpp` and `.h` under `src/ui` at commit d278eb3a: 459 lines. The kind column says whether the line draws text, measures it, or both. The font column is the font in force at that line: `default` is Shippori Antique B1 at the loaded size; `mono` means inside a `PushFont(g_mono_font)` scope; `mono (explicit arg)` passes `g_mono_font` as an argument; `font var` is the Preview overlay's `font`, the mono font when it loaded; `param font` is a helper that takes the font from its caller; a size suffix is a fixed-size push. Lines in comments are skipped. The snippet is the code on that line, cut at 90 characters.

| Screen | Kind | What it draws or measures (code) | File:line | Font today |
|---|---|---|---|---|
| Column width rule (measures in the font it is handed) | measure | `WidthOf measure_in_font(ImFont* font, float size) {` | src/ui/column_widths.cpp:195 | default |
| Column width rule (measures in the font it is handed) | measure | `return font->CalcTextSizeA(size, FLT_MAX, 0.0f, s.data(), s.data() + s.size()).x;` | src/ui/column_widths.cpp:198 | font var (mono when loaded) |
| Column width rule (measures in the font it is handed) | measure | `WidthOf measure_in_font(ImFont* font) {` | src/ui/column_widths.cpp:202 | default |
| Column width rule (measures in the font it is handed) | measure | `return measure_in_font(font ? font : ImGui::GetFont(), ImGui::GetFontSize());` | src/ui/column_widths.cpp:203 | default |
| Column width rule | measure | `WidthOf measure_in_font(ImFont* font, float size);` | src/ui/column_widths.h:179 | default |
| Column width rule | measure | `WidthOf measure_in_font(ImFont* font = nullptr);` | src/ui/column_widths.h:180 | default |
| Song panel header and notices | measure | `const float library_w = std::max(button_slot_width("Hide library"),` | src/ui/details_panel.cpp:38 | default |
| Song panel header and notices | measure | `button_slot_width("Show library"));` | src/ui/details_panel.cpp:39 | default |
| Song panel header and notices | draw | `if (ImGui::Button(hidden ? "Show library" : "Hide library", ImVec2(library_w, button)))` | src/ui/details_panel.cpp:48 | default |
| Song panel header and notices | draw | `hint(hidden ? "Put the library back beside this song"` | src/ui/details_panel.cpp:50 | default |
| Song panel header and notices | draw | `if (ImGui::Button("<##prevsong", ImVec2(button, button))) app.select_relative(-1);` | src/ui/details_panel.cpp:55 | default |
| Song panel header and notices | draw | `hint("Previous song in the list");` | src/ui/details_panel.cpp:57 | default |
| Song panel header and notices | draw | `if (ImGui::Button(">##nextsong", ImVec2(button, button))) app.select_relative(1);` | src/ui/details_panel.cpp:61 | default |
| Song panel header and notices | draw | `hint("Next song in the list");` | src/ui/details_panel.cpp:63 | default |
| Song panel header and notices | draw | `if (ImGui::Button("X##closepanel", ImVec2(button, button))) app.show_details = false;` | src/ui/details_panel.cpp:65 | default |
| Song panel header and notices | draw | `hint("Close (Esc)");` | src/ui/details_panel.cpp:66 | default |
| Song panel header and notices | draw | `text_ellipsized(display_title(song.title).c_str(), text_w);` | src/ui/details_panel.cpp:71 | default 26.0px |
| Song panel header and notices | draw | `text_ellipsized(byline.c_str(), text_w);` | src/ui/details_panel.cpp:77 | default |
| Song panel header and notices | draw | `ImGui::TextUnformatted(app::kNoPathsFound);` | src/ui/details_panel.cpp:90 | default |
| Song panel header and notices | draw | `ImGui::TextUnformatted(group_thousands(optimal).c_str());` | src/ui/details_panel.cpp:112 | default 40.0px |
| Song panel header and notices | draw | `text_ellipsized(best.pathstring().c_str(), left_x + text_w - ImGui::GetCursorPosX());` | src/ui/details_panel.cpp:120 | mono 24.0px |
| Song panel header and notices | draw | `ImGui::TextUnformatted(facts.c_str());` | src/ui/details_panel.cpp:130 | default |
| Song panel header and notices | draw | `ImGui::TextColored(kWarningColor, "Song file not found.");` | src/ui/details_panel.cpp:139 | default |
| Song panel header and notices | draw | `if (ImGui::SmallButton("Rescan library")) {` | src/ui/details_panel.cpp:141 | default |
| Song panel header and notices | draw | `ImGui::TextColored(kWarningColor, "%s", app::kAnalysisOffSentence);` | src/ui/details_panel.cpp:150 | default |
| Song panel header and notices | draw | `ImGui::TextUnformatted("Cancelling...");` | src/ui/details_panel.cpp:165 | default |
| Song panel header and notices | draw | `ImGui::Text("Analyzing chart%.*s", dots, "...");` | src/ui/details_panel.cpp:169 | default |
| Song panel header and notices | draw | `if (ImGui::Button("Cancel")) app.cancel_view();` | src/ui/details_panel.cpp:176 | default |
| Song panel header and notices | draw | `ImGui::TextUnformatted("Analysis cancelled.");` | src/ui/details_panel.cpp:195 | default |
| Song panel header and notices | draw | `if (ImGui::SmallButton("Try again")) app.start_view();` | src/ui/details_panel.cpp:197 | default |
| Song panel header and notices | draw | `ImGui::TextWrapped("%s", app.viewed.message.c_str());` | src/ui/details_panel.cpp:201 | default |
| Song panel header and notices | draw | `if (ImGui::Button("Continue")) app.dismiss_view_error();` | src/ui/details_panel.cpp:204 | default |
| Song panel header and notices | draw | `if (ImGui::BeginTabItem("Paths")) {` | src/ui/details_panel.cpp:272 | default |
| Song panel header and notices | draw | `if (ImGui::BeginTabItem("Preview", nullptr, preview_flags)) {` | src/ui/details_panel.cpp:283 | default |
| Song panel header and notices | draw | `if (ImGui::BeginTabItem("Dynamics")) {` | src/ui/details_panel.cpp:290 | default |
| Song panel header and notices | draw | `if (ImGui::BeginTabItem("Stars")) {` | src/ui/details_panel.cpp:295 | default |
| Song panel, Dynamics tab | measure | `const WidthOf text = measure_in_font();` | src/ui/dynamics_tab.cpp:75 | default |
| Song panel, Dynamics tab | draw | `ImGui::TableHeadersRow();` | src/ui/dynamics_tab.cpp:91 | default |
| Song panel, Dynamics tab | draw | `text_ellipsized(row.cells[0].c_str());` | src/ui/dynamics_tab.cpp:100 | default |
| Song panel, Dynamics tab | draw | `ImGui::TextUnformatted(row.cells[c].c_str());` | src/ui/dynamics_tab.cpp:103 | default |
| Song panel, Dynamics tab | draw | `ImGui::TextUnformatted("Reading chart...");` | src/ui/dynamics_tab.cpp:129 | default |
| Song panel, Dynamics tab | draw | `ImGui::TextWrapped("Dynamics failed: %s", viewed.dynamics_message.c_str());` | src/ui/dynamics_tab.cpp:136 | default |
| Song panel, Dynamics tab | draw | `ImGui::TextWrapped("This chart has no ghost or accent notes.");` | src/ui/dynamics_tab.cpp:149 | default |
| Song panel, Dynamics tab | draw | `ImGui::SeparatorText("Pads");` | src/ui/dynamics_tab.cpp:160 | default |
| Song panel, Dynamics tab | draw | `ImGui::SeparatorText("Kicks");` | src/ui/dynamics_tab.cpp:179 | default |
| Song panel, Dynamics tab | draw | `ImGui::TextWrapped("%s", app::dynamics_kick2x_line(bd).c_str());` | src/ui/dynamics_tab.cpp:182 | default |
| Song panel, Dynamics tab | draw | `ImGui::SeparatorText("Totals");` | src/ui/dynamics_tab.cpp:211 | default |
| Song panel, Dynamics tab | draw | `ImGui::TextWrapped("Ghosts: %s", group_thousands(played.ghost).c_str());` | src/ui/dynamics_tab.cpp:212 | default |
| Song panel, Dynamics tab | draw | `ImGui::TextWrapped("Accents: %s", group_thousands(played.accent).c_str());` | src/ui/dynamics_tab.cpp:213 | default |
| Song panel, Dynamics tab | draw | `ImGui::TextWrapped("Dynamic notes: %s of %s (%s)",` | src/ui/dynamics_tab.cpp:217 | default |
| Song panel, Dynamics tab | draw | `ImGui::SeparatorText("Chart");` | src/ui/dynamics_tab.cpp:223 | default |
| Song panel, Dynamics tab | draw | `ImGui::TextWrapped("%s", dynamics_enabled_text(bd).c_str());` | src/ui/dynamics_tab.cpp:224 | default |
| Song panel, Dynamics tab | draw | `ImGui::TextWrapped("2x kicks: counted (2x Bass on)");` | src/ui/dynamics_tab.cpp:227 | default |
| Song panel, Dynamics tab | draw | `ImGui::TextWrapped("2x kicks: not counted (2x Bass off)");` | src/ui/dynamics_tab.cpp:229 | default |
| Library dialogs (scan, batch, folders) | draw | `if (!ImGui::BeginPopupModal(kFoldersModalId, nullptr,` | src/ui/library_dialogs.cpp:178 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextUnformatted("Hydra scans these folders (and their subfolders) for charts:");` | src/ui/library_dialogs.cpp:183 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextDisabled("(None.)");` | src/ui/library_dialogs.cpp:191 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::Button("X")) confirm_remove = i;` | src/ui/library_dialogs.cpp:198 | default |
| Library dialogs (scan, batch, folders) | draw | `hint("Remove this folder from the library");` | src/ui/library_dialogs.cpp:200 | default |
| Library dialogs (scan, batch, folders) | draw | `text_ellipsized(app.settings.chartfolders[i].c_str());` | src/ui/library_dialogs.cpp:202 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::Button("Add folder...")) {` | src/ui/library_dialogs.cpp:209 | default |
| Library dialogs (scan, batch, folders) | draw | `bool close = ImGui::Button("Close") \|\|` | src/ui/library_dialogs.cpp:229 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::Button("Scan now")) {` | src/ui/library_dialogs.cpp:234 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::BeginPopupModal("Remove folder?", nullptr,` | src/ui/library_dialogs.cpp:248 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextUnformatted("Remove this folder from the library?");` | src/ui/library_dialogs.cpp:254 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextDisabled("%s", app.settings.chartfolders[*confirm_remove].c_str());` | src/ui/library_dialogs.cpp:255 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextUnformatted("Its charts disappear from the list on the next scan.");` | src/ui/library_dialogs.cpp:256 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::Button("Remove") \|\| enter_pressed()) {` | src/ui/library_dialogs.cpp:258 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::Button("Cancel") \|\| ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {` | src/ui/library_dialogs.cpp:268 | default |
| Library dialogs (scan, batch, folders) | draw | `if (!ImGui::BeginPopupModal(kScanModalId, nullptr,` | src/ui/library_dialogs.cpp:281 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextUnformatted(scan_folders_found_text(p.folders_seen).c_str());` | src/ui/library_dialogs.cpp:287 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextUnformatted("Reading charts...");` | src/ui/library_dialogs.cpp:289 | default |
| Library dialogs (scan, batch, folders) | draw | `if (p.charts_cached > 0) ImGui::TextUnformatted(scan_reused_text(p.charts_cached).c_str())...` | src/ui/library_dialogs.cpp:291 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextUnformatted("Writing library...");` | src/ui/library_dialogs.cpp:295 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextUnformatted("Scan cancelled. The library was left unchanged.");` | src/ui/library_dialogs.cpp:299 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::Button("Continue") \|\| enter_pressed() \|\|` | src/ui/library_dialogs.cpp:300 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::Text("Done: %s found.", counted(p.charts_found, "chart", "charts").c_str());` | src/ui/library_dialogs.cpp:306 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextColored(kWarningColor, "%s during the scan:",` | src/ui/library_dialogs.cpp:310 | default |
| Library dialogs (scan, batch, folders) | draw | `for (const std::string& e : p.errors) ImGui::TextUnformatted(e.c_str());` | src/ui/library_dialogs.cpp:314 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::Button("Continue") \|\| enter_pressed() \|\|` | src/ui/library_dialogs.cpp:317 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::Button("Cancel") \|\| ImGui::IsKeyPressed(ImGuiKey_Escape, false))` | src/ui/library_dialogs.cpp:326 | default |
| Library dialogs (scan, batch, folders) | draw | `if (!ImGui::BeginPopupModal("Analyze library", &open, ImGuiWindowFlags_AlwaysAutoResize)) ...` | src/ui/library_dialogs.cpp:342 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextWrapped("%s", question.c_str());` | src/ui/library_dialogs.cpp:352 | default 20.0px |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextDisabled("%s", key);` | src/ui/library_dialogs.cpp:361 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextUnformatted(value.c_str());` | src/ui/library_dialogs.cpp:363 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextDisabled("To change these, cancel and edit Analysis settings on the main screen...` | src/ui/library_dialogs.cpp:372 | default |
| Library dialogs (scan, batch, folders) | draw | `begin_disabled_checkbox(with == 0);` | src/ui/library_dialogs.cpp:374 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::Checkbox("Also re-analyze charts that already have a result##redo", &app.batch_redo...` | src/ui/library_dialogs.cpp:375 | default |
| Library dialogs (scan, batch, folders) | draw | `end_disabled_checkbox(with == 0);` | src/ui/library_dialogs.cpp:376 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextDisabled("(%s)", counted(with, "chart", "charts").c_str());` | src/ui/library_dialogs.cpp:378 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextDisabled("%s", app::batch_cant_analyze_text(cant).c_str());` | src/ui/library_dialogs.cpp:382 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextUnformatted("It runs in the background, so you can keep browsing. Analysis "` | src/ui/library_dialogs.cpp:385 | default |
| Library dialogs (scan, batch, folders) | measure | `const float start_w = button_slot_width("Start analyzing");` | src/ui/library_dialogs.cpp:392 | default |
| Library dialogs (scan, batch, folders) | measure | `const float cancel_w = button_slot_width("Cancel");` | src/ui/library_dialogs.cpp:393 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::Button("Cancel")) cancel = true;` | src/ui/library_dialogs.cpp:397 | default |
| Library dialogs (scan, batch, folders) | draw | `bool start = ImGui::Button("Start analyzing");` | src/ui/library_dialogs.cpp:401 | default |
| Library dialogs (scan, batch, folders) | measure | `const float pause_w = std::max(button_slot_width("Pause"), button_slot_width("Resume"));` | src/ui/library_dialogs.cpp:431 | default |
| Library dialogs (scan, batch, folders) | measure | `const float stop_w = button_slot_width("Stop");` | src/ui/library_dialogs.cpp:432 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextUnformatted(app::kBatchPreparingText);` | src/ui/library_dialogs.cpp:440 | default |
| Library dialogs (scan, batch, folders) | draw | `text_ellipsized(line.c_str());` | src/ui/library_dialogs.cpp:450 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::ProgressBar(progress_fraction(s.completed, s.total), ImVec2(-1.0f, px(6.0f)), "");` | src/ui/library_dialogs.cpp:454 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextUnformatted(batch_counts(s).c_str());` | src/ui/library_dialogs.cpp:460 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextUnformatted(time.c_str());` | src/ui/library_dialogs.cpp:464 | default |
| Library dialogs (scan, batch, folders) | draw | `if (button_in_slot(s.paused ? "Resume" : "Pause", pause_w)) {` | src/ui/library_dialogs.cpp:473 | default |
| Library dialogs (scan, batch, folders) | draw | `if (button_in_slot("Stop", stop_w)) app.batch_job->stop();` | src/ui/library_dialogs.cpp:480 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::SetTooltip("Keeps the %s already finished.",` | src/ui/library_dialogs.cpp:483 | default |
| Library dialogs (scan, batch, folders) | measure | `const float open_w = button_slot_width("Open report");` | src/ui/library_dialogs.cpp:515 | default |
| Library dialogs (scan, batch, folders) | draw | `if (button_in_slot("Open report", open_w)) app.show_path_report();` | src/ui/library_dialogs.cpp:522 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::Button("X##dismissdone", ImVec2(close_w, close_w))) {` | src/ui/library_dialogs.cpp:525 | default |
| Library dialogs (scan, batch, folders) | draw | `hint("Dismiss");` | src/ui/library_dialogs.cpp:530 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextWrapped("%s: %s%stook %s", stopped ? "Stopped" : "Finished",` | src/ui/library_dialogs.cpp:534 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextWrapped("Every result finished before Stop is kept.");` | src/ui/library_dialogs.cpp:539 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextWrapped("Building the path report...");` | src/ui/library_dialogs.cpp:541 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextColored(kWarningColor, "%s", app::kPathReportFailed);` | src/ui/library_dialogs.cpp:543 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextWrapped("%s", app.path_report.message.c_str());` | src/ui/library_dialogs.cpp:544 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextWrapped("%s", app::kPathReportReady);` | src/ui/library_dialogs.cpp:547 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::Checkbox("Open automatically", &app.settings.auto_open_report))` | src/ui/library_dialogs.cpp:555 | default |
| Library dialogs (scan, batch, folders) | draw | `hint("Open the path report as soon as it's built.");` | src/ui/library_dialogs.cpp:557 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextColored(kWarningColor, "%s", s.run_error.c_str());` | src/ui/library_dialogs.cpp:561 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::TreeNode(head.c_str())) {` | src/ui/library_dialogs.cpp:566 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextWrapped("%s", s.failures[i].c_str());` | src/ui/library_dialogs.cpp:569 | default |
| Library dialogs (scan, batch, folders) | draw | `if (!ImGui::BeginPopupModal("Compare dmleaderboards user", nullptr,` | src/ui/library_dialogs.cpp:589 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextUnformatted("Loading the dmleaderboards user list...");` | src/ui/library_dialogs.cpp:597 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextDisabled("%s", app::kLeaderboardWaking);` | src/ui/library_dialogs.cpp:598 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::Button("Cancel") \|\| escape) {` | src/ui/library_dialogs.cpp:599 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextColored(kWarningColor, "Could not load the user list:");` | src/ui/library_dialogs.cpp:609 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextWrapped("%s", app.dm_fetch_job->message().c_str());` | src/ui/library_dialogs.cpp:610 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::Button("Retry")) app.start_dm_fetch();` | src/ui/library_dialogs.cpp:613 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::Button("Close") \|\| escape) {` | src/ui/library_dialogs.cpp:615 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextUnformatted("Pick a player to compare against your library:");` | src/ui/library_dialogs.cpp:630 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::InputTextWithHint("##dmfilter", "filter by name", filter, sizeof(filter));` | src/ui/library_dialogs.cpp:633 | default |
| Library dialogs (scan, batch, folders) | draw | `ImGui::TextDisabled("(No users found.)");` | src/ui/library_dialogs.cpp:639 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::Selectable(label, is_last)) {` | src/ui/library_dialogs.cpp:654 | default |
| Library dialogs (scan, batch, folders) | draw | `if (ImGui::Button("Close") \|\| (escape && !ImGui::GetIO().WantTextInput)) {` | src/ui/library_dialogs.cpp:663 | default |
| Library model | draw | `std::string library_cell_text(const LibraryRow& row, int column) {` | src/ui/library_model.cpp:135 | default |
| Library model | draw | `row.sort_keys[static_cast<size_t>(c)] = app::fold_for_search(library_cell_text(row, c));` | src/ui/library_model.cpp:172 | default |
| Library model | draw | `std::string library_cell_text(const LibraryRow& row, int column);` | src/ui/library_model.h:108 | default |
| Library table and search box | draw | `ImGui::TextUnformatted("Library");` | src/ui/library_table.cpp:73 | default |
| Library table and search box | draw | `ImGui::TextColored(kNewSongColor, "%s", count.c_str());` | src/ui/library_table.cpp:80 | default |
| Library table and search box | measure | `const float clear_w = button_slot_width("X##clearsearch");` | src/ui/library_table.cpp:91 | default |
| Library table and search box | draw | `ImGui::InputTextWithHint("##search", "Search title, artist, charter or folder", buf,` | src/ui/library_table.cpp:105 | mono |
| Library table and search box | draw | `sizeof(buf), ImGuiInputTextFlags_EscapeClearsAll);` | src/ui/library_table.cpp:106 | mono |
| Library table and search box | draw | `hint("Ctrl+F jumps here. Esc clears it.");` | src/ui/library_table.cpp:108 | default |
| Library table and search box | draw | `if (ImGui::Button("X##clearsearch")) clear_search(app);` | src/ui/library_table.cpp:122 | default |
| Library table and search box | draw | `hint("Clear search (Esc)");` | src/ui/library_table.cpp:123 | default |
| Library table and search box | draw | `ImGui::TextColored(kNewSongColor,` | src/ui/library_table.cpp:127 | default |
| Library table and search box | draw | `ImGui::TextColored(kWarningColor, "%s", error.c_str());` | src/ui/library_table.cpp:131 | default |
| Library table and search box | draw | `const bool clicked = ImGui::Button(label);` | src/ui/library_table.cpp:143 | default |
| Library table and search box | measure | `if (i > 0 && fits_on_line(button_slot_width(label.c_str()), style.ItemSpacing.x))` | src/ui/library_table.cpp:165 | default |
| Library table and search box | draw | `if (chip == StatusChip::Stale) hint(app::stale_chip_text().c_str());` | src/ui/library_table.cpp:169 | default |
| Library table and search box | measure | `const float x0 = pos.x + ImGui::CalcTextSize(text.data(), text.data() + s.begin).x;` | src/ui/library_table.cpp:182 | default |
| Library table and search box | measure | `const float x1 = pos.x + ImGui::CalcTextSize(text.data(), text.data() + s.end).x;` | src/ui/library_table.cpp:183 | default |
| Library table and search box | draw | `draw->AddText(ImVec2(x0, pos.y), kMatchTextColor, text.data() + s.begin,` | src/ui/library_table.cpp:186 | default |
| Library table and search box | draw | `void cell_text(const std::string& text, const std::vector<app::MatchSpan>& spans) {` | src/ui/library_table.cpp:193 | default |
| Library table and search box | draw | `text_ellipsized(text.c_str());` | src/ui/library_table.cpp:196 | default |
| Library table and search box | draw | `void draw_cut_title(ImVec2 pos, const std::string& shown) {` | src/ui/library_table.cpp:202 | default |
| Library table and search box | draw | `ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_Text), shown.c_str())...` | src/ui/library_table.cpp:203 | default |
| Library table and search box | draw | `line.text = library_cell_text(row, kColumnFolder);` | src/ui/library_table.cpp:223 | default |
| Library table and search box | draw | `const std::string charter = library_cell_text(row, kColumnCharter);` | src/ui/library_table.cpp:226 | default |
| Library table and search box | measure | `const WidthOf text = measure_in_font();` | src/ui/library_table.cpp:247 | default |
| Library table and search box | measure | `const WidthOf mono = measure_in_font(g_mono_font);` | src/ui/library_table.cpp:248 | mono (explicit arg) |
| Library table and search box | draw | `ImGui::TableHeadersRow();` | src/ui/library_table.cpp:316 | default |
| Library table and search box | draw | `const std::string title = library_cell_text(row, kColumnTitle);` | src/ui/library_table.cpp:384 | default |
| Library table and search box | draw | `const bool clicked = ImGui::Selectable(title.c_str(), selected, row_flags,` | src/ui/library_table.cpp:391 | default |
| Library table and search box | draw | `overflow_tooltip(title.c_str());` | src/ui/library_table.cpp:396 | default |
| Library table and search box | draw | `if (title_cut) draw_cut_title(title_pos, title_shown);` | src/ui/library_table.cpp:397 | default |
| Library table and search box | draw | `text_ellipsized(line.text.c_str());` | src/ui/library_table.cpp:410 | default |
| Library table and search box | draw | `const std::string text = library_cell_text(row, column);` | src/ui/library_table.cpp:421 | default |
| Library table and search box | draw | `cell_text(text, searching_words ? app::match_spans(q, field, text)` | src/ui/library_table.cpp:422 | default |
| Library table and search box | draw | `text_ellipsized(library_cell_text(row, kColumnBestPath).c_str());` | src/ui/library_table.cpp:435 | mono |
| Library table and search box | draw | `ImGui::SetTooltip("%s", app::kNotAnalyzedText);` | src/ui/library_table.cpp:441 | default |
| Library table and search box | draw | `ImGui::SetTooltip(` | src/ui/library_table.cpp:445 | default |
| Library table and search box | draw | `ImGui::SetTooltip("%s", app::stored_failure_text(*row.failure_kind,` | src/ui/library_table.cpp:452 | default |
| Library table and search box | draw | `ImGui::TextColored(kNewSongColor, "%s", why);` | src/ui/library_table.cpp:475 | default |
| Library table and search box | draw | `if (ImGui::SmallButton("Clear search")) clear_search(app);` | src/ui/library_table.cpp:478 | default |
| Library table and search box | draw | `ImGui::TextUnformatted("No charts match your search.");` | src/ui/library_table.cpp:497 | default |
| Library table and search box | draw | `if (ImGui::Button("Clear search")) clear_search(app);` | src/ui/library_table.cpp:498 | default |
| Library toolbar | draw | `if (ImGui::SmallButton("X##dismissstatus")) {` | src/ui/library_toolbar.cpp:30 | default |
| Library toolbar | draw | `hint("Dismiss");` | src/ui/library_toolbar.cpp:34 | default |
| Library toolbar | draw | `ImGui::TextUnformatted(app.status_message.c_str());` | src/ui/library_toolbar.cpp:39 | default |
| Library toolbar | draw | `ImGui::SetTooltip("%s", AppState::kBatchRunningStatus);  // D114 item 12` | src/ui/library_toolbar.cpp:54 | default |
| Library toolbar | measure | `"Manage folders... (" + widest_digits(digit_count(folder_count)) + ")";` | src/ui/library_toolbar.cpp:63 | default |
| Library toolbar | draw+measure | `if (button_in_slot(manage_label, button_slot_width(manage_widest.c_str())))` | src/ui/library_toolbar.cpp:64 | default |
| Library toolbar | draw | `hint("Add or remove the folders Hydra scans for charts");` | src/ui/library_toolbar.cpp:66 | default |
| Library toolbar | draw | `if (ImGui::Button("Scan library")) {` | src/ui/library_toolbar.cpp:71 | default |
| Library toolbar | measure | `const char widest = widest_digits(1)[0];` | src/ui/library_toolbar.cpp:91 | default |
| Library toolbar | measure | `std::max(button_slot_width("Analyze library..."), button_slot_width(sample.c_str()));` | src/ui/library_toolbar.cpp:95 | default |
| Library toolbar | draw | `if (button_in_slot(label.c_str(), analyze_w)) app.open_batch_confirm();` | src/ui/library_toolbar.cpp:99 | default |
| Library toolbar | draw | `if (ImGui::Button("Compare with dmleaderboards...")) {` | src/ui/library_toolbar.cpp:113 | default |
| Library toolbar | draw | `ImGui::SetTooltip("%s", refused.c_str());` | src/ui/library_toolbar.cpp:122 | default |
| Library toolbar | draw | `ImGui::SetTooltip("Compare a dmleaderboards player's scores against your library.");` | src/ui/library_toolbar.cpp:124 | default |
| Library toolbar | measure | `const float report_w = std::max(button_slot_width("Open path report"),` | src/ui/library_toolbar.cpp:133 | default |
| Library toolbar | measure | `button_slot_width("Building path report..."));` | src/ui/library_toolbar.cpp:134 | default |
| Library toolbar | draw | `if (ImGui::Button(building ? "Building path report..." : "Open path report",` | src/ui/library_toolbar.cpp:144 | default |
| Library toolbar | draw | `ImGui::TextColored(kWarningColor, "%s", app::kAnalysisOffSentence);` | src/ui/library_toolbar.cpp:154 | default |
| Library toolbar | draw | `ImGui::TextColored(kWarningColor, "%s", app.rules_error.c_str());` | src/ui/library_toolbar.cpp:155 | default |
| Library view | draw | `ImGui::TextUnformatted(detail::empty_library_message(app.settings));` | src/ui/library_view.cpp:27 | default |
| Library view | draw | `ImGui::Begin("Hydra", nullptr,` | src/ui/library_view.cpp:123 | default |
| Library view | draw | `ImGui::TextWrapped("Updating your library file for this version of Hydra");` | src/ui/library_view.cpp:152 | default |
| Library view | draw | `ImGui::TextWrapped("This happens once. Your charts and results are kept.");` | src/ui/library_view.cpp:153 | default |
| Library view | draw | `ImGui::TextUnformatted(p.label().c_str());` | src/ui/library_view.cpp:155 | default |
| Path report window | draw | `ImGui::TextUnformatted(r.cant_analyze_line.c_str());` | src/ui/path_report_window.cpp:118 | default |
| Path report window | draw | `ImGui::TextColored(kDimTextColor, "%s", group.sentence.c_str());` | src/ui/path_report_window.cpp:134 | default |
| Path report window | draw | `ImGui::TextWrapped("%s", line.c_str());` | src/ui/path_report_window.cpp:138 | default |
| Path report window | draw | `ImGui::TextUnformatted(r.no_result_line.c_str());` | src/ui/path_report_window.cpp:147 | default |
| Path report window | draw | `if (ImGui::Checkbox("Optimal paths only", &w.best_only)) apply_filters(w);` | src/ui/path_report_window.cpp:168 | default |
| Song panel, Paths tab | draw | `void text_at(const ImVec2& pos, const ImVec4& color, const char* text,` | src/ui/paths_tab.cpp:50 | default |
| Song panel, Paths tab | draw | `ImGui::TextColored(color, "%s", text);` | src/ui/paths_tab.cpp:55 | param font |
| Song panel, Paths tab | measure | `float wrapped_height(const char* text, float wrap_w, ImFont* font = nullptr) {` | src/ui/paths_tab.cpp:61 | default |
| Song panel, Paths tab | measure | `const float h = ImGui::CalcTextSize(text, nullptr, false, wrap_w).y;` | src/ui/paths_tab.cpp:63 | param font |
| Song panel, Paths tab | measure | `float mono_width(const std::string& text) {` | src/ui/paths_tab.cpp:89 | default |
| Song panel, Paths tab | measure | `const float w = ImGui::CalcTextSize(text.c_str()).x;` | src/ui/paths_tab.cpp:91 | mono |
| Song panel, Paths tab | measure | `const float timing_w = b.timing.empty() ? 0.0f : px(kTimingGap) + mono_width(b.timing);` | src/ui/paths_tab.cpp:105 | default |
| Song panel, Paths tab | measure | `const float title_h = wrapped_height(b.title.c_str(), title_wrap_w, g_mono_font);` | src/ui/paths_tab.cpp:107 | mono (explicit arg) |
| Song panel, Paths tab | measure | `const float detail_h = b.detail.empty() ? 0.0f : wrapped_height(b.detail.c_str(), wrap_w);` | src/ui/paths_tab.cpp:108 | default |
| Song panel, Paths tab | draw | `const bool clicked = ImGui::Selectable(id, selected, ImGuiSelectableFlags_None, ImVec2(0.0...` | src/ui/paths_tab.cpp:113 | default |
| Song panel, Paths tab | draw | `text_at(ImVec2(top.x + pad, top.y + pad), optimal ? kBestPathColor : text_color(),` | src/ui/paths_tab.cpp:115 | default |
| Song panel, Paths tab | measure | `const float title_w = std::min(mono_width(b.title), title_wrap_w);` | src/ui/paths_tab.cpp:118 | default |
| Song panel, Paths tab | draw | `text_at(ImVec2(top.x + pad + title_w + px(kTimingGap), top.y + pad),` | src/ui/paths_tab.cpp:119 | default |
| Song panel, Paths tab | draw | `text_at(ImVec2(top.x + pad, top.y + pad + title_h + gap),` | src/ui/paths_tab.cpp:123 | default |
| Song panel, Paths tab | draw | `const bool open = ImGui::TreeNodeEx(` | src/ui/paths_tab.cpp:140 | mono |
| Song panel, Paths tab | draw | `label.c_str(), ImGuiTreeNodeFlags_DefaultOpen \| ImGuiTreeNodeFlags_NoAutoOpenOnLog);` | src/ui/paths_tab.cpp:141 | mono |
| Song panel, Paths tab | draw | `ImGui::TextDisabled("%s", b.fold_count.c_str());` | src/ui/paths_tab.cpp:145 | default |
| Song panel, Paths tab | draw | `bool fold_open = false;  // a TreeNode is pushed and needs its TreePop` | src/ui/paths_tab.cpp:155 | default |
| Song panel, Paths tab | draw | `ImGui::TextDisabled("%s", heading);` | src/ui/paths_tab.cpp:170 | default |
| Song panel, Paths tab | draw | `hint("The best path that activates at the first chance every time "` | src/ui/paths_tab.cpp:172 | default |
| Song panel, Paths tab | measure | `widths.push_back(ImGui::GetFont()->CalcTextSizeA(num_size, FLT_MAX, 0.0f, num.c_str()).x);` | src/ui/paths_tab.cpp:228 | default |
| Song panel, Paths tab | draw | `dl->AddText(ImGui::GetFont(), num_size, ImVec2(*lefts[i], top.y + px(2.0f)),` | src/ui/paths_tab.cpp:235 | default |
| Song panel, Paths tab | draw | `if (!tip.empty()) ImGui::SetTooltip("%s", tip.c_str());` | src/ui/paths_tab.cpp:259 | default |
| Song panel, Paths tab | draw | `ImGui::TextDisabled("%s", view.timeline_start.c_str());` | src/ui/paths_tab.cpp:264 | mono |
| Song panel, Paths tab | measure | `flow_next(ImGui::CalcTextSize(view.timeline_end.c_str()).x, ImGui::GetStyle().ItemSpacing....` | src/ui/paths_tab.cpp:265 | mono |
| Song panel, Paths tab | draw | `ImGui::TextDisabled("%s", view.timeline_end.c_str());` | src/ui/paths_tab.cpp:266 | mono |
| Song panel, Paths tab | draw | `if (ImGui::Selectable(id, open, ImGuiSelectableFlags_None, ImVec2(0.0f, h))) ui.click_row(...` | src/ui/paths_tab.cpp:284 | default |
| Song panel, Paths tab | draw+measure | `text_at(ImVec2(dot.x - ImGui::CalcTextSize(num.c_str()).x * 0.5f, text_y), text_color(),` | src/ui/paths_tab.cpp:297 | default |
| Song panel, Paths tab | draw | `text_at(ImVec2(top.x + px(52.0f), text_y), kBestPathColor, a.notation.c_str(), g_mono_font...` | src/ui/paths_tab.cpp:299 | mono (explicit arg) |
| Song panel, Paths tab | measure | `a.badge.empty() ? ImVec2(0.0f, 0.0f) : ImGui::CalcTextSize(a.badge.c_str());` | src/ui/paths_tab.cpp:301 | default |
| Song panel, Paths tab | draw | `text_at(ImVec2(top.x + l.measure_x, text_y), text_color(), a.measure.c_str(), g_mono_font)...` | src/ui/paths_tab.cpp:303 | mono (explicit arg) |
| Song panel, Paths tab | draw | `text_at(ImVec2(top.x + l.bars_x, text_y), dim_color(), a.bars.c_str());` | src/ui/paths_tab.cpp:304 | default |
| Song panel, Paths tab | draw | `text_at(ImVec2(bx, text_y), warn_or_dim(a.difficult), a.badge.c_str());` | src/ui/paths_tab.cpp:312 | default |
| Song panel, Paths tab | draw | `ImGui::TextUnformatted(s.text.c_str());` | src/ui/paths_tab.cpp:331 | default |
| Song panel, Paths tab | measure | `const WidthOf mono = measure_in_font(g_mono_font);` | src/ui/paths_tab.cpp:342 | mono (explicit arg) |
| Song panel, Paths tab | measure | `ImGui::CalcTextSize(backend_cell(row, kRatingColumn).c_str(), nullptr, false,` | src/ui/paths_tab.cpp:367 | default |
| Song panel, Paths tab | draw | `ImGui::TextDisabled("%s", a.backends_hidden_line.c_str());` | src/ui/paths_tab.cpp:378 | default |
| Song panel, Paths tab | draw | `ImGui::TextDisabled("None.");` | src/ui/paths_tab.cpp:383 | default |
| Song panel, Paths tab | draw | `ImGui::TableHeadersRow();` | src/ui/paths_tab.cpp:398 | mono |
| Song panel, Paths tab | draw | `ImGui::TextUnformatted(backend_cell(row, 0).c_str());` | src/ui/paths_tab.cpp:403 | mono |
| Song panel, Paths tab | draw | `ImGui::SetTooltip("%s", row.tooltip.c_str());` | src/ui/paths_tab.cpp:405 | mono |
| Song panel, Paths tab | draw | `ImGui::TextUnformatted(backend_cell(row, 1).c_str());` | src/ui/paths_tab.cpp:407 | mono |
| Song panel, Paths tab | draw | `ImGui::TextUnformatted(backend_cell(row, 2).c_str());` | src/ui/paths_tab.cpp:409 | mono |
| Song panel, Paths tab | draw | `ImGui::TextUnformatted(backend_cell(row, kRatingColumn).c_str());` | src/ui/paths_tab.cpp:415 | mono |
| Song panel, Paths tab | draw | `ImGui::TextDisabled("Chord");` | src/ui/paths_tab.cpp:431 | default |
| Song panel, Paths tab | draw | `ImGui::TextUnformatted(a.chord.c_str());` | src/ui/paths_tab.cpp:434 | mono |
| Song panel, Paths tab | measure | `place_control_at_line_end(button_slot_width("Show in Preview >"));` | src/ui/paths_tab.cpp:438 | default |
| Song panel, Paths tab | draw | `if (ImGui::SmallButton(show)) ui.preview_jump = i;` | src/ui/paths_tab.cpp:439 | default |
| Song panel, Paths tab | draw | `if (!a.early_fill.empty()) ImGui::TextUnformatted(a.early_fill.c_str());` | src/ui/paths_tab.cpp:441 | default |
| Song panel, Paths tab | draw | `ImGui::TextUnformatted(a.scale_warning.c_str());` | src/ui/paths_tab.cpp:448 | default |
| Song panel, Paths tab | draw | `overflow_tooltip(app::kTransferScaleHint);` | src/ui/paths_tab.cpp:451 | default |
| Song panel, Paths tab | draw | `ImGui::TextUnformatted(a.overfill_warning.c_str());` | src/ui/paths_tab.cpp:457 | default |
| Song panel, Paths tab | draw | `overflow_tooltip(app::kOverfillHint);` | src/ui/paths_tab.cpp:460 | default |
| Song panel, Paths tab | draw | `const bool backends = ImGui::TreeNodeEx(` | src/ui/paths_tab.cpp:470 | default |
| Song panel, Paths tab | draw | `label, ImGuiTreeNodeFlags_NoTreePushOnOpen \| ImGuiTreeNodeFlags_NoAutoOpenOnLog);` | src/ui/paths_tab.cpp:471 | default |
| Song panel, Paths tab | draw | `ImGui::TextDisabled("\xC2\xB7 %s", a.backends_label.c_str());` | src/ui/paths_tab.cpp:476 | default |
| Song panel, Paths tab | draw | `ImGui::TextUnformatted("Activations");` | src/ui/paths_tab.cpp:485 | default |
| Song panel, Paths tab | measure | `flow_next(ImGui::CalcTextSize(view.summary.c_str()).x, ImGui::GetStyle().ItemSpacing.x);` | src/ui/paths_tab.cpp:487 | default |
| Song panel, Paths tab | draw | `ImGui::TextDisabled("%s", view.summary.c_str());` | src/ui/paths_tab.cpp:488 | default |
| Song panel, Paths tab | draw | `ImGui::TextDisabled("None.");` | src/ui/paths_tab.cpp:491 | default |
| Song panel, Paths tab | measure | `place_control_at_line_end(button_slot_width(toggle));` | src/ui/paths_tab.cpp:496 | default |
| Song panel, Paths tab | draw | `if (ImGui::SmallButton(toggle)) ui.set_all(!all);` | src/ui/paths_tab.cpp:497 | default |
| Song panel, Paths tab | measure | `widest_measure = std::max(widest_measure, ImGui::CalcTextSize(a.measure.c_str()).x);` | src/ui/paths_tab.cpp:504 | mono |
| Song panel, Paths tab | draw | `if (ImGui::Button(label)) open = !open;` | src/ui/paths_tab.cpp:519 | default |
| Song panel, Paths tab | measure | `flow_next(ImGui::CalcTextSize(summary.c_str()).x, spacing);` | src/ui/paths_tab.cpp:531 | default |
| Song panel, Paths tab | draw | `ImGui::TextDisabled("%s", summary.c_str());` | src/ui/paths_tab.cpp:532 | default |
| Song panel, Paths tab | measure | `flow_next(button_slot_width("Score breakdown##breakdown"), px(16.0f));` | src/ui/paths_tab.cpp:533 | default |
| Song panel, Paths tab | measure | `flow_next(button_slot_width("Copy path"), px(24.0f));` | src/ui/paths_tab.cpp:535 | default |
| Song panel, Paths tab | draw | `if (ImGui::Button("Copy path")) copy_selected_path(app);` | src/ui/paths_tab.cpp:536 | default |
| Song panel, Paths tab | draw | `hint("Ctrl+C also copies the selected path.");` | src/ui/paths_tab.cpp:537 | default |
| Song panel, Paths tab | measure | `flow_next(ImGui::CalcTextSize("Copied!").x, spacing);` | src/ui/paths_tab.cpp:540 | default |
| Song panel, Paths tab | draw | `ImGui::TextDisabled("Copied!");` | src/ui/paths_tab.cpp:541 | default |
| Song panel, Paths tab | draw | `if (d.squeezes.empty()) ImGui::TextDisabled("None.");` | src/ui/paths_tab.cpp:546 | mono |
| Song panel, Paths tab | draw | `ImGui::TextUnformatted(msq.label.c_str());` | src/ui/paths_tab.cpp:548 | mono |
| Song panel, Paths tab | draw | `ImGui::TextUnformatted(msq.howto.c_str());` | src/ui/paths_tab.cpp:550 | mono |
| Song panel, Paths tab | draw | `for (const std::string& line : d.breakdown) ImGui::TextUnformatted(line.c_str());` | src/ui/paths_tab.cpp:557 | mono |
| Song panel, Paths tab | draw | `if (ImGui::Checkbox("Hide backend rows beyond##backendlimit", &app.settings.backendlimit_e...` | src/ui/paths_tab.cpp:564 | default |
| Song panel, Paths tab | draw | `overflow_tooltip("Hides backend rows beyond +/- this many ms; squeezed-out notes always "` | src/ui/paths_tab.cpp:567 | default |
| Song panel, Paths tab | draw | `if (ImGui::InputInt("##backendlimitvalue", &app.settings.backendlimit_value)) {` | src/ui/paths_tab.cpp:573 | default |
| Song panel, Paths tab | draw | `ImGui::TextUnformatted("ms");` | src/ui/paths_tab.cpp:579 | default |
| Song panel, Paths tab | measure | `widest = std::max(widest, in + mono_width(b.title) +` | src/ui/paths_tab.cpp:607 | default |
| Song panel, Paths tab | measure | `: px(kTimingGap) + mono_width(b.timing)));` | src/ui/paths_tab.cpp:609 | default |
| Song panel, Paths tab | measure | `widest = std::max(widest, ImGui::CalcTextSize(b.detail.c_str()).x);` | src/ui/paths_tab.cpp:612 | default |
| Song panel, Preview tab | measure | `return std::max(cap_h, ImGui::CalcTextSize(key).x + pad_x * 2.0f);` | src/ui/preview_tab.cpp:81 | default |
| Song panel, Preview tab | measure | `float group_w = ImGui::CalcTextSize(kh.action).x + action_gap;` | src/ui/preview_tab.cpp:88 | default |
| Song panel, Preview tab | measure | `const float half_text = ImGui::CalcTextSize(key).x * 0.5f;` | src/ui/preview_tab.cpp:112 | default |
| Song panel, Preview tab | draw | `ImGui::TextColored(kDefaultTextColor, "%s", key);` | src/ui/preview_tab.cpp:115 | default |
| Song panel, Preview tab | draw | `ImGui::TextColored(kSubtleTextColor, "%s", kh.action);` | src/ui/preview_tab.cpp:119 | default |
| Song panel, Preview tab | draw | `ImGui::TextUnformatted("Showing");` | src/ui/preview_tab.cpp:152 | default |
| Song panel, Preview tab | measure | `ImGui::CalcTextSize(hydra::app::preview_path_label(b).c_str()).x);` | src/ui/preview_tab.cpp:163 | mono |
| Song panel, Preview tab | draw | `const bool open = ImGui::BeginCombo("##previewpath", shown.c_str());` | src/ui/preview_tab.cpp:168 | mono |
| Song panel, Preview tab | draw | `if (!open && shown != current) overflow_tooltip(current.c_str());` | src/ui/preview_tab.cpp:169 | mono |
| Song panel, Preview tab | draw | `if (ImGui::Selectable(item.c_str(), is_selected)) selected = b.path;` | src/ui/preview_tab.cpp:176 | mono |
| Song panel, Preview tab | draw | `ImGui::EndCombo();` | src/ui/preview_tab.cpp:179 | mono |
| Song panel, Preview tab | draw | `dl->AddText(font, l.size, ImVec2(at[i].x, at[i].y), l.color, l.text.c_str());` | src/ui/preview_tab.cpp:283 | font var (mono when loaded) |
| Song panel, Preview tab | measure | `l.width = text_width(size, text.c_str());` | src/ui/preview_tab.cpp:294 | default |
| Song panel, Preview tab | draw | `ImGui::TextUnformatted("Preview is unavailable (no render device).");` | src/ui/preview_tab.cpp:400 | default |
| Song panel, Preview tab | draw | `ImGui::TextUnformatted("Select a song to preview.");` | src/ui/preview_tab.cpp:404 | default |
| Song panel, Preview tab | draw | `ImGui::TextWrapped("Preview failed: %s", pc->error().c_str());` | src/ui/preview_tab.cpp:434 | default |
| Song panel, Preview tab | draw | `ImGui::Text("Loading preview: %s", lp.label.c_str());` | src/ui/preview_tab.cpp:445 | default |
| Song panel, Preview tab | draw | `ImGui::Text("Loading preview: %s, %s", lp.label.c_str(), lp.detail.c_str());` | src/ui/preview_tab.cpp:447 | default |
| Song panel, Preview tab | draw | `ImGui::TextColored(kWarningColor,` | src/ui/preview_tab.cpp:456 | default |
| Song panel, Preview tab | draw | `hint(pc->audio_warning().c_str());` | src/ui/preview_tab.cpp:458 | default |
| Song panel, Preview tab | draw | `ImGui::TextWrapped(` | src/ui/preview_tab.cpp:465 | default |
| Song panel, Preview tab | draw | `if (ImGui::Button("< Act##prevact")) pc->jump_activation(-1);` | src/ui/preview_tab.cpp:501 | default |
| Song panel, Preview tab | draw | `overflow_tooltip("Previous activation ([)");` | src/ui/preview_tab.cpp:503 | default |
| Song panel, Preview tab | draw | `if (ImGui::Button("Act >##nextact")) pc->jump_activation(+1);` | src/ui/preview_tab.cpp:506 | default |
| Song panel, Preview tab | draw | `overflow_tooltip("Next activation (])");` | src/ui/preview_tab.cpp:508 | default |
| Song panel, Preview tab | draw | `if (ImGui::Button(back_jump.c_str())) pc->jump_ms(-kJumpMs);` | src/ui/preview_tab.cpp:510 | default |
| Song panel, Preview tab | draw | `hint(back_jump_tip.c_str());` | src/ui/preview_tab.cpp:511 | default |
| Song panel, Preview tab | draw | `if (ImGui::Button(back_step.c_str())) pc->step_ms(-kStepMs);` | src/ui/preview_tab.cpp:513 | default |
| Song panel, Preview tab | draw | `hint(back_step_tip.c_str());` | src/ui/preview_tab.cpp:514 | default |
| Song panel, Preview tab | measure | `const float play_w = std::max(button_slot_width("Play"), button_slot_width("Pause"));` | src/ui/preview_tab.cpp:516 | default |
| Song panel, Preview tab | draw | `if (button_in_slot(pc->playing() ? "Pause" : "Play", play_w)) pc->toggle();` | src/ui/preview_tab.cpp:517 | default |
| Song panel, Preview tab | draw | `hint("Play or pause (Space)");` | src/ui/preview_tab.cpp:518 | default |
| Song panel, Preview tab | draw | `if (ImGui::Button(fwd_step.c_str())) pc->step_ms(kStepMs);` | src/ui/preview_tab.cpp:520 | default |
| Song panel, Preview tab | draw | `hint(fwd_step_tip.c_str());` | src/ui/preview_tab.cpp:521 | default |
| Song panel, Preview tab | draw | `if (ImGui::Button(fwd_jump.c_str())) pc->jump_ms(kJumpMs);` | src/ui/preview_tab.cpp:523 | default |
| Song panel, Preview tab | draw | `hint(fwd_jump_tip.c_str());` | src/ui/preview_tab.cpp:524 | default |
| Song panel, Preview tab | draw | `ImGui::TextUnformatted("Vol");` | src/ui/preview_tab.cpp:534 | default |
| Song panel, Preview tab | measure | `const char widest = widest_digits(1)[0];` | src/ui/preview_tab.cpp:559 | default |
| Song panel, Preview tab | measure | `const float readout_w = text_slot_width(readout_sample.c_str());` | src/ui/preview_tab.cpp:563 | mono |
| Song panel, Preview tab | draw | `if (ImGui::SliderFloat("##scrub", &pos_s, 0.0f, len_s, ""))` | src/ui/preview_tab.cpp:567 | default |
| Song panel, Preview tab | draw | `ImGui::SetTooltip("Gold marks are this path's activations.");` | src/ui/preview_tab.cpp:572 | default |
| Song panel, Preview tab | draw | `text_in_slot(box.timestamp.c_str(), readout_w);` | src/ui/preview_tab.cpp:576 | mono |
| Song panel, Preview tab | measure | `return font->CalcTextSizeA(sz, FLT_MAX, 0.0f, s).x;` | src/ui/preview_tab.cpp:633 | font var (mono when loaded) |
| Song panel, Preview tab | measure | `return [&text_width, sz](const std::string& s) { return text_width(sz, s.c_str()); };` | src/ui/preview_tab.cpp:694 | default |
| Song panel, Preview tab | draw | `dl->AddText(font, label_size, ImVec2(gauge_left, origin.y + v_margin),` | src/ui/preview_tab.cpp:794 | font var (mono when loaded) |
| Song panel, Preview tab | measure | `const float rw = text_width(label_size, readout.c_str());` | src/ui/preview_tab.cpp:797 | default |
| Song panel, Preview tab | draw | `dl->AddText(font, label_size,` | src/ui/preview_tab.cpp:798 | font var (mono when loaded) |
| Report windows (shared frame) | draw | `void fitted_text(const std::string& text, FittedTextCache& fits, size_t row, size_t column...` | src/ui/report_window.cpp:37 | default |
| Report windows (shared frame) | draw | `draw_cut_text(text.c_str(), fits.fitted(row, column, text, avail, fit), avail);` | src/ui/report_window.cpp:43 | default |
| Report windows (shared frame) | measure | `float button_w(const char* label) { return button_slot_width(label); }` | src/ui/report_window.cpp:46 | default |
| Report windows (shared frame) | draw | `ImGui::TextUnformatted("Hydra");` | src/ui/report_window.cpp:71 | default |
| Report windows (shared frame) | draw | `ImGui::TextColored(kAccentColor, "%s", f.heading);` | src/ui/report_window.cpp:73 | default |
| Report windows (shared frame) | draw | `ImGui::TextColored(kDimTextColor, "Built %s", clock_text(*f.built).c_str());` | src/ui/report_window.cpp:79 | default |
| Report windows (shared frame) | draw | `if (ImGui::Button("Compare another player...")) call(cb.compare_another);` | src/ui/report_window.cpp:87 | default |
| Report windows (shared frame) | draw | `if (ImGui::Button("Refresh")) call(cb.refresh);` | src/ui/report_window.cpp:90 | default |
| Report windows (shared frame) | draw | `if (ImGui::Button("Close")) *open = false;` | src/ui/report_window.cpp:93 | default |
| Report windows (shared frame) | draw | `ImGui::TextColored(kDimTextColor, "%s", subtitle);` | src/ui/report_window.cpp:99 | default |
| Report windows (shared frame) | draw | `ImGui::TextUnformatted(why.c_str());` | src/ui/report_window.cpp:127 | default |
| Report windows (shared frame) | draw | `if (ImGui::Button("Refresh##outofdate")) call(f.callbacks->refresh);` | src/ui/report_window.cpp:129 | default |
| Report windows (shared frame) | draw | `ImGui::TextColored(kWarningColor, "%s", f.analysis_off_sentence);` | src/ui/report_window.cpp:162 | default |
| Report windows (shared frame) | draw | `ImGui::TextColored(kWarningColor, "%s", f.analysis_off_error.c_str());` | src/ui/report_window.cpp:163 | default |
| Report windows (shared frame) | draw | `ImGui::TextUnformatted(f.building_lines[i].c_str());` | src/ui/report_window.cpp:172 | default |
| Report windows (shared frame) | draw | `ImGui::TextColored(kDimTextColor, "%s", f.building_lines[i].c_str());` | src/ui/report_window.cpp:174 | default |
| Report windows (shared frame) | draw | `ImGui::ProgressBar(progress_fraction(done, total), ImVec2(-1.0f, 0.0f), "");` | src/ui/report_window.cpp:179 | default |
| Report windows (shared frame) | draw | `ImGui::ProgressBar(-1.0f * static_cast<float>(ImGui::GetTime()),` | src/ui/report_window.cpp:182 | default |
| Report windows (shared frame) | draw | `if (ImGui::Button("Cancel")) call(cb.cancel);` | src/ui/report_window.cpp:188 | default |
| Report windows (shared frame) | draw | `if (ImGui::Button("Analyze") && f.analyze_enabled) call(cb.analyze);` | src/ui/report_window.cpp:191 | default |
| Report windows (shared frame) | draw | `if (ImGui::Button("Cancel")) call(cb.cancel);` | src/ui/report_window.cpp:195 | default |
| Report windows (shared frame) | draw | `ImGui::TextUnformatted("Report cancelled.");` | src/ui/report_window.cpp:201 | default |
| Report windows (shared frame) | draw | `if (ImGui::Button("Try again")) call(cb.try_again);` | src/ui/report_window.cpp:202 | default |
| Report windows (shared frame) | draw | `ImGui::TextColored(kWarningColor, "%s", f.failure_sentence);` | src/ui/report_window.cpp:205 | default |
| Report windows (shared frame) | draw | `ImGui::TextWrapped("%s", f.failure_message.c_str());` | src/ui/report_window.cpp:207 | default |
| Report windows (shared frame) | draw | `if (ImGui::Button("Try again")) call(cb.try_again);` | src/ui/report_window.cpp:211 | default |
| Report windows (shared frame) | draw | `if (ImGui::Button("Compare another player...")) call(cb.compare_another);` | src/ui/report_window.cpp:214 | default |
| Report windows (shared frame) | draw | `ImGui::TextWrapped("%s", f.empty_text.c_str());` | src/ui/report_window.cpp:220 | default |
| Report windows (shared frame) | draw | `m.visible = ImGui::Begin(f.window_name.c_str(), open,` | src/ui/report_window.cpp:246 | default |
| Report windows (shared frame) | draw | `ImGui::TextColored(kDimTextColor, "%s", f.footer.c_str());` | src/ui/report_window.cpp:267 | default |
| Report windows (shared frame) | measure | `const float w = (std::max)(ImGui::CalcTextSize(t.label.c_str()).x,` | src/ui/report_window.cpp:280 | default |
| Report windows (shared frame) | measure | `ImGui::CalcTextSize(t.value.c_str()).x) +` | src/ui/report_window.cpp:281 | default |
| Report windows (shared frame) | draw | `ImGui::TextColored(kDimTextColor, "%s", t.label.c_str());` | src/ui/report_window.cpp:288 | default |
| Report windows (shared frame) | draw | `ImGui::TextUnformatted(t.value.c_str());` | src/ui/report_window.cpp:289 | default |
| Report windows (shared frame) | measure | `ImGui::SetNextItemWidth(button_slot_width(hint_text));` | src/ui/report_window.cpp:298 | default |
| Report windows (shared frame) | draw | `const bool changed = ImGui::InputTextWithHint("##search", hint_text, m.search,` | src/ui/report_window.cpp:300 | default |
| Report windows (shared frame) | draw | `ImGuiInputTextFlags_EscapeClearsAll);` | src/ui/report_window.cpp:302 | default |
| Report windows (shared frame) | measure | `for (const std::string& l : labels) widest = (std::max)(widest, ImGui::CalcTextSize(l.c_st...` | src/ui/report_window.cpp:309 | default |
| Report windows (shared frame) | draw | `if (ImGui::BeginCombo(id, labels[static_cast<size_t>(chosen)].c_str())) {` | src/ui/report_window.cpp:314 | default |
| Report windows (shared frame) | draw | `if (ImGui::Selectable(labels[static_cast<size_t>(i)].c_str(), i == chosen) &&` | src/ui/report_window.cpp:316 | default |
| Report windows (shared frame) | draw | `ImGui::EndCombo();` | src/ui/report_window.cpp:322 | default |
| Report windows (shared frame) | measure | `const float w = ImGui::CalcTextSize(text.c_str()).x;` | src/ui/report_window.cpp:331 | default |
| Report windows (shared frame) | draw | `ImGui::TextColored(kDimTextColor, "%s", text.c_str());` | src/ui/report_window.cpp:335 | default |
| Report windows (shared frame) | draw | `ImGui::TextUnformatted(app::html::kNothingMatches);` | src/ui/report_window.cpp:340 | default |
| Report windows (shared frame) | draw | `return ImGui::Button("Clear filters");` | src/ui/report_window.cpp:341 | default |
| Report windows (shared frame) | measure | `return ImGui::CalcTextSize(f.footer.c_str(), nullptr, false, wrap).y +` | src/ui/report_window.cpp:347 | default |
| Report windows (shared frame) | draw | `fitted_text(text, fits, row, column);` | src/ui/report_window.cpp:360 | mono |
| Report windows (shared frame) | draw | `ImGui::TextUnformatted(text.c_str());` | src/ui/report_window.cpp:373 | default |
| Report windows (shared frame) | measure | `s.width_of = measure_in_font();` | src/ui/report_window.cpp:389 | default |
| Report windows (shared frame) | measure | `s.width_of = measure_in_font(look.mono ? g_mono_font : nullptr);` | src/ui/report_window.cpp:399 | mono (explicit arg) |
| Report windows (shared frame) | draw | `overflow_tooltip(columns[c - 1].definition.c_str());` | src/ui/report_window.h:458 | default |
| Report windows (shared frame) | draw | `const bool clicked = ImGui::Selectable(` | src/ui/report_window.h:478 | default |
| Report windows (shared frame) | draw | `if (!can_click(row)) hint(look.unclickable_hint);` | src/ui/report_window.h:487 | default |
| Settings bar | draw | `ImGui::SeparatorText(title);` | src/ui/settings_bar.cpp:55 | default |
| Settings bar | draw | `ImGui::TextUnformatted(name);` | src/ui/settings_bar.cpp:67 | default |
| Settings bar | draw | `bool setting_checkbox(const char* id, bool* value, bool locked) {` | src/ui/settings_bar.cpp:74 | default |
| Settings bar | draw | `begin_disabled_checkbox(locked);` | src/ui/settings_bar.cpp:75 | default |
| Settings bar | draw | `const bool changed = ImGui::Checkbox(id, value);` | src/ui/settings_bar.cpp:76 | default |
| Settings bar | draw | `end_disabled_checkbox(locked);` | src/ui/settings_bar.cpp:77 | default |
| Settings bar | draw | `if (ImGui::Combo("##difficulty", &idx, names, IM_ARRAYSIZE(names))) {` | src/ui/settings_bar.cpp:90 | default |
| Settings bar | draw | `if (setting_checkbox("##prodrums", &app.settings.view_prodrums, locked)) app.commit_settin...` | src/ui/settings_bar.cpp:101 | default |
| Settings bar | draw | `if (setting_checkbox("##bass2x", &bass2x_shown, locked)) {` | src/ui/settings_bar.cpp:107 | default |
| Settings bar | draw | `if (setting_checkbox("##noteshuffle", &app.settings.view_noteshuffle, locked))` | src/ui/settings_bar.cpp:116 | default |
| Settings bar | draw | `if (ImGui::InputInt("##spcap", &cap)) {` | src/ui/settings_bar.cpp:134 | default |
| Settings bar | draw | `ImGui::TextUnformatted("bars");` | src/ui/settings_bar.cpp:139 | default |
| Settings bar | draw | `if (setting_checkbox("##legacyfills", &app.settings.legacy_fills, locked))` | src/ui/settings_bar.cpp:143 | default |
| Settings bar | measure | `const float six_digits = ImGui::CalcTextSize(widest_digits(6).c_str()).x +` | src/ui/settings_bar.cpp:156 | default |
| Settings bar | draw | `if (ImGui::InputInt("##depthvalue", &app.settings.depth_value)) {` | src/ui/settings_bar.cpp:161 | default |
| Settings bar | draw | `if (ImGui::Combo("##depthmode", &mode_idx, modes, 2)) {` | src/ui/settings_bar.cpp:170 | default |
| Settings bar | draw | `if (setting_checkbox("##mslimit", &app.settings.mslimit_enabled, locked))` | src/ui/settings_bar.cpp:180 | default |
| Settings bar | draw | `if (ImGui::InputInt("##mslimitvalue", &app.settings.mslimit_value)) {` | src/ui/settings_bar.cpp:186 | default |
| Settings bar | draw | `ImGui::TextUnformatted("ms");` | src/ui/settings_bar.cpp:192 | default |
| Settings bar | draw | `if (ImGui::Button("Reset to defaults")) {` | src/ui/settings_bar.cpp:203 | default |
| Settings bar | draw | `overflow_tooltip(reset_settings_tooltip(app.settings, locked).c_str());` | src/ui/settings_bar.cpp:208 | default |
| Settings bar | draw | `ImGui::TextUnformatted(kSettingsLockedText);` | src/ui/settings_bar.cpp:222 | default |
| Settings bar | measure | `const float least = ImGui::CalcTextSize(kDefaultLabel.c_str()).x + chrome;` | src/ui/settings_bar.cpp:263 | default |
| Settings bar | measure | `const float text_room = std::min(ImGui::CalcTextSize(label.c_str()).x, room - chrome);` | src/ui/settings_bar.cpp:270 | default |
| Settings bar | draw | `ImGui::Button((shown + "###analysissettings").c_str(), ImVec2(text_room + chrome, 0.0f));` | src/ui/settings_bar.cpp:282 | default |
| Settings bar | draw | `if (shown != label) overflow_tooltip(label.c_str());` | src/ui/settings_bar.cpp:287 | default |
| Song panel, Stars tab | draw | `ImGui::Text("Base score: %s", group_thousands(sc.chart_base_score).c_str());` | src/ui/stars_tab.cpp:29 | default |
| Song panel, Stars tab | draw | `ImGui::Text("Solo bonus: %s (not counted toward stars)",` | src/ui/stars_tab.cpp:31 | default |
| Song panel, Stars tab | measure | `const WidthOf text = measure_in_font();` | src/ui/stars_tab.cpp:38 | default |
| Song panel, Stars tab | draw | `overflow_tooltip(` | src/ui/stars_tab.cpp:58 | default |
| Song panel, Stars tab | draw | `ImGui::TextUnformatted(cell.c_str());` | src/ui/stars_tab.cpp:66 | default |
| Song panel, Stars tab | draw | `ImGui::TextUnformatted(` | src/ui/stars_tab.cpp:73 | default |
| Theme | draw | `ImGui::PushStyleColor(ImGuiCol_Text, kDisabledInputTextColor);` | src/ui/theme.cpp:119 | default |
| Theme | draw | `void begin_disabled_checkbox(bool disabled) {` | src/ui/theme.cpp:136 | default |
| Theme | draw | `void end_disabled_checkbox(bool disabled) {` | src/ui/theme.cpp:147 | default |
| Theme (hint helpers) | draw | `inline const ImVec4& kDisabledInputTextColor = kDimTextColor;` | src/ui/theme.h:82 | default |
| Theme (hint helpers) | draw | `void begin_disabled_checkbox(bool disabled);` | src/ui/theme.h:122 | default |
| Theme (hint helpers) | draw | `void end_disabled_checkbox(bool disabled);` | src/ui/theme.h:123 | default |
| Shared widget helpers (font: the caller's current one) | draw | `ImGui::TextWrapped("%s", detail.c_str());` | src/ui/widgets.h:41 | default |
| Shared widget helpers (font: the caller's current one) | draw | `inline void hint(const char* text) {` | src/ui/widgets.h:48 | default |
| Shared widget helpers (font: the caller's current one) | draw | `ImGui::SetTooltip("%s", text);` | src/ui/widgets.h:50 | default |
| Shared widget helpers (font: the caller's current one) | draw | `inline void wrapped_tooltip(const char* text) {` | src/ui/widgets.h:56 | default |
| Shared widget helpers (font: the caller's current one) | draw | `ImGui::TextUnformatted(text);` | src/ui/widgets.h:59 | default |
| Shared widget helpers (font: the caller's current one) | draw | `ImGui::TextDisabled("(?)");` | src/ui/widgets.h:69 | default |
| Shared widget helpers (font: the caller's current one) | draw | `wrapped_tooltip(text);` | src/ui/widgets.h:71 | default |
| Shared widget helpers (font: the caller's current one) | draw | `ImGui::ProgressBar(frac, ImVec2(-1, 0), overlay.c_str());` | src/ui/widgets.h:96 | default |
| Shared widget helpers (font: the caller's current one) | draw | `ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f), overlay);` | src/ui/widgets.h:104 | default |
| Shared widget helpers (font: the caller's current one) | measure | `inline std::string widest_digits(int count) {` | src/ui/widgets.h:117 | default |
| Shared widget helpers (font: the caller's current one) | measure | `float w = ImGui::CalcTextSize(one).x;` | src/ui/widgets.h:122 | default |
| Shared widget helpers (font: the caller's current one) | measure | `inline float text_slot_width(const char* sample) { return ImGui::CalcTextSize(sample).x; }` | src/ui/widgets.h:135 | default |
| Shared widget helpers (font: the caller's current one) | draw | `inline void text_in_slot(const char* text, float slot_w) {` | src/ui/widgets.h:140 | default |
| Shared widget helpers (font: the caller's current one) | draw | `ImGui::TextUnformatted(text);` | src/ui/widgets.h:142 | default |
| Shared widget helpers (font: the caller's current one) | measure | `inline float button_slot_width(const char* widest_label) {` | src/ui/widgets.h:148 | default |
| Shared widget helpers (font: the caller's current one) | measure | `return ImGui::CalcTextSize(widest_label, nullptr, true).x +` | src/ui/widgets.h:149 | default |
| Shared widget helpers (font: the caller's current one) | draw | `inline bool button_in_slot(const char* label, float slot_w) {` | src/ui/widgets.h:152 | default |
| Shared widget helpers (font: the caller's current one) | draw | `return ImGui::Button(label, ImVec2(slot_w, 0.0f));` | src/ui/widgets.h:153 | default |
| Shared widget helpers (font: the caller's current one) | draw | `inline void overflow_tooltip(const char* text) {` | src/ui/widgets.h:217 | default |
| Shared widget helpers (font: the caller's current one) | draw | `wrapped_tooltip(text);` | src/ui/widgets.h:221 | default |
| Shared widget helpers (font: the caller's current one) | measure | `inline float text_width(const std::string& s) { return ImGui::CalcTextSize(s.c_str()).x; }` | src/ui/widgets.h:226 | default |
| Shared widget helpers (font: the caller's current one) | draw | `inline void draw_cut_text(const char* text, const std::string& shown, float avail) {` | src/ui/widgets.h:233 | default |
| Shared widget helpers (font: the caller's current one) | draw | `ImGui::TextUnformatted(text);` | src/ui/widgets.h:235 | default |
| Shared widget helpers (font: the caller's current one) | draw | `window->DrawList->AddText(window->DC.CursorPos, ImGui::GetColorU32(ImGuiCol_Text),` | src/ui/widgets.h:240 | default |
| Shared widget helpers (font: the caller's current one) | measure | `ImGui::Dummy(ImVec2(avail, ImGui::CalcTextSize(text).y));` | src/ui/widgets.h:244 | default |
| Shared widget helpers (font: the caller's current one) | draw | `overflow_tooltip(text);` | src/ui/widgets.h:245 | default |
| Shared widget helpers (font: the caller's current one) | draw | `inline void text_ellipsized(const char* text, float max_width = FLT_MAX) {` | src/ui/widgets.h:255 | default |
| Shared widget helpers (font: the caller's current one) | draw | `draw_cut_text(text, render::ellipsize(text, avail, text_width), avail);` | src/ui/widgets.h:259 | default |
| Shared widget helpers (font: the caller's current one) | draw | `inline bool row_selectable(const char* text, bool selected) {` | src/ui/widgets.h:266 | default |
| Shared widget helpers (font: the caller's current one) | draw | `ImGui::Selectable(text, selected, ImGuiSelectableFlags_SpanAllColumns);` | src/ui/widgets.h:269 | default |
| Shared widget helpers (font: the caller's current one) | measure | `ImGui::CalcTextSize(text).x > avail)` | src/ui/widgets.h:271 | default |
| Shared widget helpers (font: the caller's current one) | draw | `overflow_tooltip(text);` | src/ui/widgets.h:272 | default |
