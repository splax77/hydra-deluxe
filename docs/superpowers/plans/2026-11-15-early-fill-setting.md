# Plan: the early-fill window becomes one adjustable setting

Task id: early-fill-setting-plan. Written on branch `claude/early-fill-setting-plan`, cut from main at `c10c7f54`. This plan changes no code.

## What the user asked

These lines are filed under `## Confirmed` in `docs/agents/rulings/code-and-engine.md` (lines 55 to 57).

- "make the early fill threshold an adjustable setting pinned to one number in both directions"
- "analysis settings panel, default 60ms, supports 0ms to 85ms"
- Earlier the same day: "so we should use the hit window. and stop inventing our own numbers."

In a question box the user also chose "Early fill only": the new setting moves the early-fill window and nothing else. Squeeze budgets, Rating words and timing tiers keep the fixed 85 ms hit window. That choice is quoted in the main session's brief and is not a filed line, so the Premise map marks it as the brief's quote.

## The short version

Today one fixed number, the hit window (85 ms), sets both sides of the early-fill window. After this work the user sets that number themselves, in a new `Early fill window` row of the Analysis settings panel. Call the number W. It starts at 60 ms and can go from 0 to 85 ms.

W does two jobs, as the hit window does today. A fill is dropped when Star Power came ready more than W after the fill's deadline. An activation gets the `E` mark when Star Power came ready less than W before the deadline. Both jobs read the same W, so the two sides can never drift apart.

Results are saved per W. A result analyzed at 60 and one analyzed at 85 sit side by side in the library file, and switching the box back brings the old result back, the way the SP cap and `1.0 fills` already work.

Nothing else moves. `kHitWindowMs` stays 85 and keeps its three other readers: the squeeze budget, the Rating bands and the report's hit-window text.

## Owner map

The owner map is the file committed beside this plan, unchanged: `docs/superpowers/plans/2026-11-15-early-fill-setting-owner-map.md`. Its table is not repeated here, so there is one copy. Tasks below name its rows by id (O1 to O19).

Its two Stops are settled by the user's lines above. Stop A (60 against "use the hit window"): the later lines make the window the user's own setting with a default of 60. Stop B ("never a setting" in `CONTEXT.md` and `model.h`): task T5 rewrites those texts, and the `CONTEXT.md` entry changes because the user changed the term.

## The design, one decision at a time

### How W reaches the engine (O1, O2, O3, O4, O5, O10)

The four early-fill questions in `src/core/model.h` take the window as an argument instead of reading `kHitWindowMs`. Those are `fill_refuses`, `is_e0`, `fill_within_limit` and `allzero_activation`. No other rule changes: the same strict compares, the same owners, one more argument.

W travels the route the owner map traced for `legacy_fills` (O10). It starts as `Settings::early_fill_window_ms`. `Settings::to_analysis_settings()` copies it onto `SearchSettings`. `src/search/pather.cpp` copies it onto every `EngineOptions` it builds (the main search and the all-0 pass). The engine keeps it and hands it to the four questions.

A drawn activation must answer "is this an E?" with the W its search ran under, not with whatever the box says now. So the engine writes W onto each `Activation` beside `e_offset`, in a new field `e_window_ms`. `Activation::is_e_critical`, `is_E0`, `within_ms_limit` and `Path::is_allzero` pass that field to the same four questions. This is a carry-out of a value the engine already uses, not a new rule.

The record gets the same value once, as `HydraRecord::early_fill_window_ms`, set in `pather.cpp` beside `record.legacy_fills`. The store uses it to refuse a mismatched profile (see below).

### Where the default lives

The user's 60 gets one home: `kDefaultEarlyFillWindowMs = 60` in `src/core/model.h`, beside `kHitWindowMs`, with the ruling named in its comment. `Settings`, `SearchSettings`, `EngineOptions`, `Activation`, `HydraRecord` and `Lens::from` all take their default from it. No file writes a second 60.

### The settings file (O11)

The new key is `early_fill_window_ms`. The name `hit_window_ms` is on the retired list (`src/app/config.h:90`) and stays there, because the hit window is still fixed.

The key is a `number(...)` row in the key table in `src/app/config.cpp`, with the range 0 to 85. The top edge is written as `static_cast<int>(kHitWindowMs)`, the way `kWindowMs` is already written from `kSqueezeWindowMs` on line 158, so no second 85 exists. A value outside the range loads at the nearest edge through the existing `pull_into_range`, and `Settings::clamp` serves the panel box and the tools. Text that is not a whole number is refused by the existing `read_value`. Nothing new is needed for any of that.

### The panel row (O12)

The row goes in the "Clone Hero rules" group in `src/ui/settings_bar.cpp`, under `1.0 fills`. It follows the panel's one row rule through `begin_row`: the name and its (?) on the left, only the control on the right.

- Name: `Early fill window`
- Control: a whole-number box (`ImGui::InputInt("##earlyfillwindow", ...)`), then the word `ms`. It is the same kind of box as Path limit's, with the same default step of 1 and the same `px(100)` width, so no new number is added. Each step goes through `Settings::clamp` and `app.edit_settings()`, as the SP cap box does.
- (?) tooltip: "How far from a fill's deadline your Star Power may come ready and still count as an early fill. Ready up to this many ms after the deadline, you can still summon the fill by hitting early; later than that, the fill never spawns. Ready less than this many ms before the deadline, the activation is marked E. At 0 no activation is an early fill. 85 ms, the hit window, is the most it allows."

Each tooltip sentence is checked against the code. "Later than that, the fill never spawns" is `fill_refuses` (`e_offset < -W`). "Less than this many ms before" is `is_e0` (`e_offset < W`). "At 0 no activation is an early fill" is O19. The 85 is printed through `hit_window_text()` (`src/app/report.cpp:92`), not typed.

`kAnalysisFields` gets one new row after `legacy_fills`: `{&Settings::early_fill_window_ms, AnalysisSetting::EarlyFillWindow, KeyPart::Lens}`. `AnalysisSetting` gets `EarlyFillWindow` after `Fills`. Reset to defaults and the summary then pick it up with no further code, because both walk the table.

### The toolbar button's words (O12)

`change_phrase` words a number setting as its name, a space, then the value from `describe_settings`: "SP cap 6 bars", "Path limit 10 ms". The new case follows it: `"Early fill window " + values.early_fill`. `SettingsText` gains `early_fill`, filled by `describe_settings` with the existing `format_ms_whole`.

So with the box at 85 and everything else default, the button reads `Analysis settings: Early fill window 85 ms`. At the default it reads `Analysis settings: defaults`. The Reset tooltip reads `Resets: Early fill window 85 ms`.

### How results are saved per W (O14, O15, O16)

W joins the lens, the existing owner of "which settings was this result made under" (O14). `Lens` gets a sixth field, `efill_window_ms`, and `kLensColumnList` gets a sixth name. `lens_match`, `bind_lens`, the `profiles` table's columns and its UNIQUE key are all built from that list, so they follow. `prepare_record` gains one check beside the fill-rule check: it throws when the profile's window differs from the record's.

I looked for a way to carry W without a new column and found none that fits. The chart mode and the SP cap are other key parts with other meanings. The rules fingerprint belongs to `hydra_rules.ini`, which the user turned down for this setting.

**The O15 gap.** No code adds a column to an existing file's `profiles` table. The scout's search, repeated here: `ALTER TABLE` and `ADD COLUMN` across `src/store`, `src/app` and `tools` find only `add_album_column` and `add_failure_columns`, and `record_tables_sql` makes `profiles` with `CREATE TABLE IF NOT EXISTS`, which leaves an old table alone. So T3 adds the step. This is a storage step, not a game rule.

A plain `ADD COLUMN` is not enough, because the table's UNIQUE key would still leave the new column out, and two profiles that differ only in W would collide. So the step rebuilds the table inside one transaction: rename the old table, create the new one from the same SQL a fresh file uses, copy every row across with its `id` kept, then drop the old table. `records` and `record_paths` point at profiles by `id`, so they need no change. The table is tiny (one row per settings combination), so the rebuild is quick. A failure rolls the whole step back and raises `DatabaseUpgrade`, as the two existing column steps do.

`kind_of` in `src/store/library_file.cpp` gets one more layout, `LacksEarlyFillColumn`: a `profiles` table without the new column. The step also runs for the two older in-place kinds, by the same pattern `add_album_column` uses. A Convertible file (2.2.0 to 2.7.0) gets a fresh `profiles` table anyway; its `convert_results` read of the old `results` table keeps its own list of the five old lens columns, because that table never had a sixth.

**What an old row is filed under.** No saved row records a window. Every row saved before this change is filed under the default, 60.

**The stamp moves, once.** `kResultsStamp` changes from `3.1.2+efillhitwindow` to `3.1.2+efillwindow`. It has to: rows stamped `3.1.2+efillhitwindow` were analyzed at 85 on both sides, and filed under 60 with a current stamp they would read Ready at 60, which is false. The stamp is unreleased, so this is still the one bump 3.1.2 ships (D23), not a second one: a 3.1.1 user sees one change of stamp either way.

**How old results read once this ships.**

- Results from 3.1.1 and earlier (late side 85, E mark 60) already read Stale under today's main. They still read Stale, at the default setting, where the user starts. Nothing new for them. `docs/releases/release-next-notes.md` already says so.
- Results saved on a dev build since merge `0bfa2dce` (2026-11-15, both sides 85) read Ready today. After this change they read Stale. That is a visible change nobody told the user about. It is question 1 below.
- No result vanishes at the default setting. At any other W a chart reads as not yet analyzed there until it is analyzed at that W, the same as changing the SP cap.

### The tools (O17)

`hydra_batch`, `hydra_bench --engine/--parse`, `hydra_fillcompare` and the Path Builder load `hydra_settings.ini`, so they get W with no code of their own. `hydra_bench`'s fixed yardstick uses bare `SearchSettings` defaults and so runs at 60.

`hydra_replay` starts from the app's defaults, not the file. It gets one flag, `--efill <ms>`, handled like `--ms`: read through `setting_flag_number`, then clamped by `Settings::clamp` in the loop at the end of `settings_from`. With no flag it runs at the default, 60.

### The 0 ms edge (O19)

At 0, `fill_refuses` is `e_offset < 0` and `is_e0` is `e_offset < 0 && skips == 0`. So a fill whose Star Power came ready any time after the deadline is refused, one ready exactly on the deadline still spawns, and no activation that spawns is ever an E. `allzero_activation` reduces to `skips == 0`. Nothing divides by W. T1 pins each of these with a test.

### What stays as it is

`kHitWindowMs`, `squeeze_rating.cpp`, `hit_window_text()` and the Dynamic Hit Window row (O7, O8) are not touched. `longest_activation_badge` (O6) keeps its code; only its comment changes, because an early-fill figure is now bounded by W, which is never wider than the hit window.

## What the user will see

Every screen below reads W through the owners in O2 to O4. The comparison is the new default, 60, against today's main, 85.

- **Paths tab, the `E` in a path's notation.** An activation whose Star Power came ready between 60 and 85 ms before the deadline loses its `E`. An activation whose Star Power came ready between 60 and 85 ms after the deadline is gone: the fill no longer spawns there, so the path activates on another fill or drops out, and a chart's best score can fall.
- **The early fill badge and the "Early fill" line.** They appear on the same activations as the `E`. The figure they show now lies inside 60 ms on either side, where today it lies inside 85.
- **The all-0 section.** A path may pass over one early fill and still be all-0. That allowance now covers fills with less than 60 ms to spare, not 85, so some charts' best all-0 path changes.
- **The path report's `Early fill (ms)` and `Hardest ms` columns.** No early-fill figure above 60 at the default.
- **The Analysis settings panel.** One new row, `Early fill window`, under `1.0 fills`.
- **The toolbar button.** `Analysis settings: Early fill window 85 ms` when the box is moved to 85, and so on for any value but 60.
- **Library status.** See "How old results read" above, and question 1.

Against release 3.1.1 (late side 85, E mark 60) the `E` marks at the default match again. The one difference is on the late side, where fills between 60 and 85 ms late no longer spawn.

Real chart examples need a build. Live step LS1 gets them from `testdata/input`: it runs `hydra_replay` on each chart with `--efill 85` and with `--efill 60` and keeps the charts whose printed paths differ. The main session shows the user those lines, before and after, with question 1 (agent rule 11).

## Questions

1. **Results saved today read Stale.** Any result you saved on a dev build since this morning's merge (the 85 ms window) reads Stale after this change and needs analyzing again. Results from 3.1.1 already read Stale and are not affected. Is that all right? If not, the store step can instead file those rows under 85, so they read Ready whenever the box is set to 85; that adds one case to the step, keyed on the old stamp's text.
2. **The batch confirm box and `hydra_batch`'s header.** They list the settings a batch runs under (Difficulty, SP cap, Fills, Score range, Path limit). Your words did not ask for a new line there. Should each gain an `Early fill window` line? Until you say yes, the plan leaves both as they are.

## Tasks

Each task is one Opus executor, in its own worktree, inside 100 tool calls and 20 minutes. Executors do not build; the main session builds each branch and takes its test list from `hydra_precheck --tests`. The tests each task names are the ones it writes or edits.

T1 and T5 start at once (`earlyStart`), in parallel. T3 starts from T1's tip. T2 starts from T3's tip. T4 and T6 start from T2's tip, in parallel. The main session joins the branches as they finish. One fresh reviewer reviews the joined set, once, because the tasks stack on each other and no branch builds alone past T1.

### T1: the engine reads W

Rows: O1, O2, O3, O4, O5, O6, O9, O10, O13, O19. `earlyStart`: yes. Depends on: nothing.

Owns `src/core/model.h`, `src/core/model.cpp`, `src/search/engine.h`, `src/search/engine.cpp`, `src/search/pather.h`, `src/search/pather.cpp`, `src/app/path_view.cpp` (comment at lines 79 to 85 only), `tests/test_model.cpp`, `tests/test_fills_and_skips.cpp`, `tests/song_digest.h`, and any other test under `tests/` that builds an `Activation` by hand and fails to compile or changes meaning (`record_fixtures.h`, `test_activation_rows.cpp`, `test_path_list.cpp`, `test_preview_boxes.cpp`, `test_report_rows.cpp`, `test_tied_variants.cpp`, `test_note_window.cpp`, `test_store_results.cpp`).

The work is the "How W reaches the engine" and "Where the default lives" sections above. `tests/song_digest.h`'s `digest_settings` states its window outright as the hit window, so the pinned digests keep describing the search they were pinned on (P17).

Acceptance criteria:

- `fill_refuses`, `is_e0`, `fill_within_limit` and `allzero_activation` take the window, and nothing in `src/` or `tools/` reads `kHitWindowMs` for an early-fill question. Check: `tests/test_model.cpp` boundary cases run at W = 60 and W = 85 (ready exactly W late still spawns; a hair later refuses; exactly W to spare is no E; a hair less is an E). They fail if either side reads a constant.
- Both sides read one W. Check: a `test_model.cpp` case walks several W values and asserts `is_e0(e, 0, W)` holds for every `e` where `fill_refuses(e, W)` holds (O13). It fails if the two sides take different numbers.
- The 0 ms edge. Check: `test_model.cpp` cases at W = 0: ready a hair after the deadline refuses, exactly on it spawns, no `e_offset` at or above 0 is an E, and `allzero_activation(e, 1, 0)` is false for every such `e`.
- The search follows the setting. Check: in `tests/test_fills_and_skips.cpp`, the existing 70 ms early-fill chart is analyzed twice through `analyze` with `SearchSettings::early_fill_window_ms` at 85 and at 60. At 85 the activation is found and is an E; at 60 it is not found on that fill. It fails if `pather.cpp` drops W on the way to either search.
- Every activation and record carries its W. Check: the same test asserts `Activation::e_window_ms` and `HydraRecord::early_fill_window_ms` equal the setting it ran under, for the main paths and the all-0 path.
- The default is 60 in one place. Check: a `test_model.cpp` case asserts a default `SearchSettings`, `EngineOptions` and `Activation` all hold `kDefaultEarlyFillWindowMs`.
- The squeeze side is untouched. Check: the diff holds no change to `src/core/squeeze_rating.cpp` or `src/app/report.cpp`; `tests/test_squeeze_rating.cpp` and `tests/test_replay_schedule.cpp:355` pass unedited.

### T3: the library file saves results per W

Rows: O14, O15, O16. Depends on: T1 (it reads `HydraRecord::early_fill_window_ms`). Starts from T1's tip.

Owns `src/store/record_store.h`, `src/store/record_store.cpp`, `src/store/library_file.h`, `src/store/library_file.cpp`, `src/store/stored_versions.h`, `testdata/store/README.md`, `tests/test_store_open.cpp`, `tests/test_store_records.cpp`, `tests/test_store_results.cpp`, `tests/test_library_jobs.cpp`.

The work is the "How results are saved per W" section above. First step: read what opening a `LacksAlbumColumn` file does beyond `set_up_schema` (any prompt or backup), and make the new kind follow it exactly (P16). If the new kind would show the user something the album step does not, stop and report.

Acceptance criteria:

- Two windows are two results. Check: `test_store_records.cpp` saves one chart under profiles that differ only in `efill_window_ms`, then looks each up and gets its own record back. It fails if the column is missing from the list or the key.
- A mismatched record is refused. Check: `prepare_record` with a profile at 85 and a record at 60 throws `std::invalid_argument`.
- An older file is rebuilt in place. Check: `test_store_open.cpp` opens a copy of `testdata/store/v2.12.1_library.db`; it classifies as `LacksEarlyFillColumn`; after opening it classifies as `Current`; every profile row keeps its `id` and its other columns; every record is still reachable under its old profile; each old row's window is `kDefaultEarlyFillWindowMs`; and two profiles differing only in W can then be saved.
- The rebuild is all or nothing. Check: a test forces a failure at each point the step names (after the rename, after the create, during the copy) and asserts the file reads exactly as before and the error kind is `DatabaseUpgrade`.
- The older kinds still open. Check: the existing Convertible, `LacksFailureColumns` and `LacksAlbumColumn` cases in `test_store_open.cpp` pass and end with the new column present.
- The stamp is `3.1.2+efillwindow` and accepts only itself. Check: `test_store_results.cpp` saves a row stamped `3.1.2+efillhitwindow` and asserts it reads Stale.
- No old result vanishes at the default. Check: the rebuilt fixture's rows are found by a lookup at the default lens and read Stale, not absent.

### T2: the setting, its file line and its words

Rows: O10, O11, O12, O14. Depends on: T1 and T3. Starts from T3's tip.

Owns `src/app/config.h`, `src/app/config.cpp`, `src/ui/library_dialogs.cpp`, `tests/test_config.cpp`.

The work is the "settings file" and "toolbar button's words" sections, plus `Settings::lens()` passing W to `Lens::from` and `to_analysis_settings()` passing it on.

Acceptance criteria:

- The key round-trips. Check: `test_config.cpp` saves and loads `early_fill_window_ms` and gets the same value; a missing line loads 60.
- The range is 0 to 85 and clamps. Check: the existing case "a value outside its range loads at the nearest edge" gains 200 (loads 85) and -5 (loads 0); "Settings::clamp is the one range every caller asks" gains the field. Text that is not a number is refused.
- `hit_window_ms` stays retired. Check: the existing retired-keys case passes unedited.
- Reset and the summary cover it. Check: "settings reset: puts back each panel setting and keeps the rest" walks `kAnalysisFields`, so it fails if the row or the phrase is missing; a new check asserts the exact text `Analysis settings: Early fill window 85 ms` and `Resets: Early fill window 85 ms`.
- The profile follows the setting. Check: `Settings::profile()` at 60 and at 85 differ in the lens and nowhere else; `batch_run()`'s search settings and profile hold the same W.

### T4: the panel row

Rows: O12. Depends on: T2. Starts from T2's tip.

Owns `src/ui/settings_bar.cpp`, `src/ui/library_parts.h` (if the tooltip text gets a function, as `legacy_fills_help_text` has), `tests/ui/uitest_library.cpp`.

The work is "The panel row" section above.

Acceptance criteria:

- The row is on screen with its exact name and tooltip. Check: a new `hydra_uitest` test, `settings-early-fill`, opens the panel, finds `Early fill window`, and compares the tooltip with the help-text function's return, which the test also checks contains `hit_window_text()`.
- The box changes the setting and clamps. Check: the test types 85 and reads the button label `Analysis settings: Early fill window 85 ms`; types 200 and reads 85 back; types -5 and reads 0 back.
- The row follows the one row rule. Check: the row is made by `begin_row`; the reviewer reads the diff for it, and LS3 shows it.
- Reset puts it back. Check: the existing `settings-reset` test gains the box.
- The box locks during a batch. Check: the existing `settings-lock` test gains the box.

### T6: hydra_replay's flag

Rows: O17. Depends on: T2. Starts from T2's tip.

Owns `tools/replay.cpp`, `tests/test_replay_schedule.cpp` (only if a flag test lives there; else the test file the executor finds for `--ms`), and the `hydra_replay` section of the page that lists its flags.

Acceptance criteria:

- `--efill <ms>` sets the window and clamps like `--ms`. Check: a test of `settings_from` with `--efill 85` holds 85, with `--efill 900` holds 85, with no flag holds 60, and with `--efill abc` throws the same error shape `--ms abc` does.
- The usage text names the flag. Check: the docs test that compares tool flags with their page passes.

### T5: the words in docs, comments and scans

Rows: O18, and Stops A and B. `earlyStart`: yes. Depends on: nothing for the text; its markers name `kDefaultEarlyFillWindowMs` and `Settings::early_fill_window_ms`, so it merges after T1 and T2.

Owns `CONTEXT.md` ("Early fill (E)" and "Hit window" entries), `docs/UserGuide.md`, `docs/differences-from-public-hydra.md`, `docs/clone-hero/04-fills.md` (FL-24), `docs/clone-hero/03-timing.md` (line 365), `docs/releases/release-next-notes.md`, `tests/test_docs_match_code.cpp` (the default-value table), `tests/single_owner/rules/core.json`, `tests/single_owner/rules/app.json`. The comments in `model.h` and `config.h` belong to T1 and T2.

The `CONTEXT.md` "Early fill (E)" entry becomes: "An activation timing (the `E` notation) near a fill's spawn deadline. The window is one number on both sides of the deadline: the `Early fill window` setting, 60 ms by default, from 0 to the hit window. On the orange side, Star Power comes ready after the deadline, so you must hit early to summon the fill. Later than the window (`fill_refuses`), the fill never spawns, and the search drops the activation. On the white side, Star Power comes ready before the deadline with less than the window to spare (`is_e0`). The activation still gets the `E` mark, but has nothing to time." The sentence "It is never a setting" goes. The "Hit window" entry keeps "fixed, not a setting" and drops the early fill from its list of readers.

The public pages follow `docs/agents/public-doc-style.md`. The User Guide's settings table gains an `Early fill window` row with a `default:` marker. Its early-fill paragraphs say "the `Early fill window` setting" where they say "85 ms, the hit window". The release-notes line becomes: "A new `Early fill window` setting in `Analysis settings` sets how far from a fill's deadline Star Power may come ready and still count as an early fill, on both sides. It starts at 60 ms and goes from 0 to 85 ms. Your saved results read Stale until you analyze them again."

Acceptance criteria:

- No doc says the early-fill window is fixed or is 85. Check: `hydra_tests.exe "-tc=docs match code*"` passes with the new `default:` markers in the table, so a doc number that drifts from `kDefaultEarlyFillWindowMs` fails.
- The single-owner scan agrees. Check: the scan's rows name the window argument as the owner of the early-fill compares, and still flag a bare `kHitWindowMs` compare on `e_offset`; the main session runs the scan on the joined branch.
- The public text follows the style page. Check: the reviewer reads the four changed public passages against `docs/agents/public-doc-style.md`.

## The live steps

The main session runs these, after the joined branch builds. None needs Clone Hero.

### LS1: real charts, before and after

Run `hydra_replay` on each of the 98 charts in `testdata/input`, once with `--efill 85` and once with `--efill 60`, printing each chart's paths. Keep the charts whose output differs, and show the user up to five, with the path lines before and after. Cap: one pass over the 98 charts, 10 minutes. If no chart differs, report that and name it as a gap in the corpus; do not reach for the whole library.

Calls: `hydra_replay --efill` (never tried)

### LS2: the panel's behaviour and text

Run the new and the two edited GUI tests. Cap: one run; a failure goes to a fresh executor.

Calls: `hydra_uitest --test settings-early-fill --test settings-reset --test settings-lock` (never tried)

### LS3: the panel in the real app

A panel row is a layout change, so case 2 of `docs/agents/ui-testing.md` applies. Open the real dev app the way "Seeing it in the real app" on that page says, with the user's `hydra_ui.ini` and a copy of their database, open the Analysis settings panel, and capture it. Show the user the capture. No analysis is started on the database copy. Cap: one capture; if the panel cannot be captured, report what failed. This step settles P18.

Calls: `hydra_start('window')` (never tried)

### LS4: the early-fill screens on the corpus

Case 3 of the same page. Start the app headless on `testdata/input`, open one chart LS1 named, and read the Paths tab text at 60 and at 85. Compare the `E` marks with LS1's lines for that chart.

Calls: `hydra_start('headless')` (seen: LS3's passing run, same launch kind), `hydra_open_song` (never tried)

## Out of scope, for the user to approve

- Opening a rebuilt library file with an older Hydra build. Nothing in this plan tests it.
- A line for the new setting in the batch confirm box and `hydra_batch`'s header, unless question 2 says yes.
- Any change to the squeeze budgets, Rating words, timing tiers or `kHitWindowMs`.

## Promise map

| Promise | Plan section | Task | The check that proves it |
|---|---|---|---|
| One number sets both sides of the window | The short version | T1 | `test_model.cpp`: for several W, every `e` that `fill_refuses` also satisfies `is_e0` |
| A fill is dropped when SP came ready more than W late; an E is marked with less than W to spare | How W reaches the engine | T1 | `test_model.cpp` boundary cases at W = 60 and W = 85 |
| The search, the all-0 pass and every drawn activation use the W the run was given | How W reaches the engine | T1 | `test_fills_and_skips.cpp`: the 70 ms chart at 85 and at 60; `e_window_ms` on main and all-0 paths |
| The default is 60, written once | Where the default lives | T1, T2, T5 | `test_model.cpp` default case; `test_config.cpp` missing-line case; docs `default:` markers |
| The range is 0 to 85; outside it lands on the nearest edge | The settings file | T2, T4, T6 | `test_config.cpp` 200 and -5; `settings-early-fill` 200 and -5; replay `--efill 900` |
| The key is `early_fill_window_ms`; `hit_window_ms` stays retired | The settings file | T2 | round-trip case; retired-keys case unedited |
| The row is named `Early fill window`, sits under `1.0 fills`, follows the one row rule, and shows the written tooltip | The panel row | T4 | `settings-early-fill`; LS3 capture |
| No new number: the box steps and is sized like Path limit's | The panel row | T4 | reviewer reads the diff: default `InputInt` step, `px(100)` |
| The button reads `Analysis settings: Early fill window 85 ms`; Reset names it | The toolbar button's words | T2, T4 | `test_config.cpp` exact text; `settings-early-fill` label read |
| Reset to defaults puts W back; a batch locks the box | The panel row | T2, T4 | reset case walks `kAnalysisFields`; `settings-reset`, `settings-lock` |
| Results are saved per W; switching back brings the old result back | How results are saved per W | T3 | `test_store_records.cpp` two-window case |
| A record never files under another window | How results are saved per W | T3 | `prepare_record` throws |
| An older file is rebuilt in place, ids kept, all or nothing | The O15 gap | T3 | `test_store_open.cpp` rebuild case; failure forced at each of the three points |
| Old rows are filed under 60 and none vanishes at the default | What an old row is filed under | T3 | rebuilt fixture read at the default lens: Stale, not absent |
| The stamp moves once, to `3.1.2+efillwindow` | The stamp moves, once | T3 | `test_store_results.cpp`: a `3.1.2+efillhitwindow` row reads Stale |
| The user is asked before today's results go Stale | Questions | question 1 | the main session's question, with LS1's lines |
| `hydra_replay` takes `--efill`, like `--ms` | The tools | T6 | `settings_from` flag cases |
| At 0 no activation is an early fill and nothing breaks | The 0 ms edge | T1 | `test_model.cpp` W = 0 cases |
| Squeeze budgets, Rating words, timing tiers and `kHitWindowMs` do not change | What stays as it is | T1 | no diff in `squeeze_rating.cpp` or `report.cpp`; their tests pass unedited |
| `CONTEXT.md`, the public pages and the scans say what the code does | T5 | T5 | docs-match-code test; single-owner scan; reviewer's style read |
| Real chart examples come from `testdata/input`, never the whole library | What the user will see | LS1 | LS1's command lines and cap |
| The panel change is seen in the real app before merge | LS3 | LS3 | the capture shown to the user |
| A new line in the batch confirm waits for the user's yes | Questions | question 2 | no diff to `batch_settings_summary` or `src/cli/batch.cpp` unless answered |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The user asked for one adjustable number on both sides | ruling | `docs/agents/rulings/code-and-engine.md` line 56 | checked 2026-11-15 |
| P2: The user asked for the panel, default 60, range 0 to 85 | ruling | `docs/agents/rulings/code-and-engine.md` line 57 | checked 2026-11-15 |
| P3: The user chose "Early fill only", so squeezes, Rating words and tiers keep 85 | unchecked | the main session's brief quotes the question box; no line in `docs/agents/rulings/` holds it | question 1 (the main session confirms the quote when it asks) |
| P4: Both early-fill compares read `kHitWindowMs` today and take only `e_offset` | code | `src/core/model.h:318` and `src/core/model.h:322-324` | checked 2026-11-15 |
| P5: The only early-fill readers of those functions are in `model.h`, `model.cpp` and `engine.cpp` | code | grep of `src` and `tools` for the function names: `src/core/model.cpp:473-855`, `src/search/engine.cpp:854-1930` | checked 2026-11-15 |
| P6: `kHitWindowMs` has three other readers, which this plan leaves alone | code | `src/core/squeeze_rating.cpp:133`, `src/core/squeeze_rating.cpp:242`, `src/app/report.cpp:92` | checked 2026-11-15 |
| P7: Only the engine fills `Activation::e_offset` in `src/` | code | grep of `src` for `e_offset`: hits only in `src/core/model.h`, `src/core/model.cpp`, `src/search/engine.cpp`, `src/app/path_view.cpp` | checked 2026-11-15 |
| P8: The library file saves per-path figures and the path text, not activations, so a saved figure already reflects the window it was analyzed under | code | `src/store/record_store.cpp:284-310` and `src/store/record_store.cpp:588-598` | checked 2026-11-15 |
| P9: A number key clamps to its range on load, and `Settings::clamp` serves boxes and tools | code | `src/app/config.cpp:211`, `src/app/config.cpp:235-238`, `src/app/config.cpp:305-311` | checked 2026-11-15 |
| P10: `hit_window_ms` is a retired key | code | `src/app/config.h:90` | checked 2026-11-15 |
| P11: Reset and the summary walk `kAnalysisFields`; a setting with no phrase throws | code | `src/app/config.cpp:359-377`, `src/ui/library_dialogs.cpp:88-105` | checked 2026-11-15 |
| P12: The button's phrase for a number setting is name, space, value | code | `src/ui/library_dialogs.cpp:97-100` and `src/ui/library_dialogs.cpp:129-131` | checked 2026-11-15 |
| P13: Every panel row is made by `begin_row`, and Path limit's box is a default-step `InputInt` at `px(100)` | code | `src/ui/settings_bar.cpp:63-70` and `src/ui/settings_bar.cpp:185-192` | checked 2026-11-15 |
| P14: The profile's columns, key, match and bind are built from `kLensColumnList`, and an existing `profiles` table is never altered | code | `src/store/record_store.cpp:183-208`, `src/store/record_store.cpp:247-257`, `src/store/record_store.cpp:383-393`, `src/store/record_store.cpp:848-876` | checked 2026-11-15 |
| P15: A lookup's candidates are the rows with the wanted lens; a row is Ready only with the current stamp and the same rules | code | `src/store/record_store.cpp:376-379` and `src/store/record_store.cpp:445-454` | checked 2026-11-15 |
| P16: Opening an older-kind file (`LacksAlbumColumn`) shows the user nothing beyond the open itself | unchecked | `src/store/record_store.cpp:836-841` shows the step; the callers of `classify_library_file` were not read | T3 |
| P17: The pinned engine digests do not move when `digest_settings` states the window as 85 | unchecked | `tests/song_digest.h` was not opened; the owner map row O18 only reports it | T1 |
| P18: The method under "Seeing it in the real app" can capture the open Analysis settings panel | unchecked | `docs/agents/ui-testing.md` lines 1 to 70 were read; that section was not | LS3; blocks nothing |
| P19: The stamp is `3.1.2+efillhitwindow`, compared as a whole string, and one bump covers a release | code | `src/store/stored_versions.h:42-55` | checked 2026-11-15 |
| P20: `hydra_replay` starts from default settings and clamps its number flags through `Settings::clamp` | code | `tools/replay.cpp:189-234` | checked 2026-11-15 |
| P21: `testdata/store/v2.12.1_library.db` holds a `profiles` table a release wrote | unchecked | `testdata/store` lists the file; it was not opened | T3 |
| P22: Some chart in `testdata/input` has an activation between 60 and 85 ms from a fill deadline | unchecked | no run exists yet | LS1; blocks nothing |
| P23: `hydra_replay` has a command that prints a chart's paths with their `E` marks | unchecked | `tools/replay.cpp` lines 186 to 234 were read; its commands were not | T6 |
| P24: The release notes already tell users their saved results read Stale in 3.1.2 | code | `docs/releases/release-next-notes.md:3` | checked 2026-11-15 |
| P25: `CONTEXT.md` says the early-fill window is never a setting | code | `CONTEXT.md:610-620` | checked 2026-11-15 |
