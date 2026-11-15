a893c0571fafbeba3

# Verify: window round WN4 (T5-V-WN4)

Task T5-V-WN4, session e633f7f6-f313-4787-b622-8d901b577b64. Code read at the worktree HEAD (`5ec682c7`'s code). Nothing was run. Every claim below is "by reading" unless it names a run.

The filed rulings on the window, under `## Confirmed` in `docs/agents/rulings/code-and-engine.md` lines 55 to 57:

- "so we should use the hit window. and stop inventing our own numbers."
- "make the early fill threshold an adjustable setting pinned to one number in both directions"
- "analysis settings panel, default 60ms, supports 0ms to 85ms"

## The short version

The window facts in the auditor's file hold. The set of 473 test cases is complete: every `TEST_CASE` in the 18 files has a row (I counted cases per file and matched line numbers). The auditor's sorting does not hold. Thirteen B rows are wrong, because the case runs the search on the default window and asserts a typed value taken from that run. One A row is wrong. The 45 C records are not full records and understate the typed numbers.

## 1. The window facts (by reading)

All hold: `kDefaultEarlyFillWindowMs = 60.0` (`src/core/model.h:112`); `app::Settings::early_fill_window_ms` starts at it (`src/app/config.h:195`); `SearchSettings::early_fill_window_ms` (`src/search/pather.h:45`); the Activation default (`src/core/model.h:606`); `default_cfg()` is `Settings().to_analysis_settings()` (`tests/corpus_default_cfg.h:17`). `fill_refuses` and `is_e0` are at `src/core/model.h:331,335`, and the engine reads them at `src/search/engine.cpp:859,862,1007`. The window applies under both fill rules (the engine code has no rule branch there), so the "1.0 fill rule" tests are not exempt.

The window is set in two places in this group, as the auditor said: `tests/song_digest.h:332` (`digest_settings`, `kMaxEarlyFillWindowMs`, which is 85, `src/core/model.h:115`) and `tests/test_activation_rows.cpp:1228` (`kHitWindowMs`, 85, `src/core/model.h:106`). One more consequence the auditor missed: `edge_run` at `tests/test_song_parse.cpp:1354` passes `digest_settings`, so the edge-file cases at 1387 and 1390 also run on 85. They read parse hashes only, so B stands, but the table calls them default-window.

## 2. Scope: cases with no row

None. Case counts match the rows per file (48, 19, 8, 10, 48, 30, 33, 10, 30, 40, 10, 40, 7, 21, 37, 2, 52, 28 = 473). The auditor's "470" is a miscount of three. The helpers table matches the code (`app_state_fixtures.h` passes caller settings, `song_details_fixtures.h` `burnout()` runs the search at the scratch default, `dm_fixture.h` has no search).

The auditor's own limit paragraph says every B row in a "token scan" file is unproven. I found that the scan missed searches that sit behind a helper struct or function (`OverratedCard`, `player_path`, `pitch_dark()`, `save_old` through `fill_saved_hardest`). That is where the disputes below come from.

## 3. A rows

| Row | Verdict | Why |
|---|---|---|
| `tests/test_song_parse.cpp:1292` Note Shuffle switch: the Dynamics pads follow... | agree | Runs `batch_run` from `digest_settings`, which sets 85 (`tests/song_digest.h:332`). |
| `tests/test_activation_rows.cpp:1186` the chord names its notes in the Pro Drums setting's words | dispute, it is B | The case body (1186 to 1215) is a hand-built `Activation` and `build_activations` with `HydraRecord rec`; no search runs. The scan took the helper `analyzed_like_the_gui` (1221), which sits after it in the same text span, as part of it. The 85 helper is read by `tapped_out`, `focus`, `epidermis` and one direct call at 1446, used only by cases in the delta round (1308, 1356 and 1371 inside it, 1396, 1439, 1469, 1504 and 1524 inside it) and by 1544 and 1558. |
| `tests/song_digest.h` (helper) | agree | |

## 4. B rows

I read the body of every B row that holds a corpus run, a store save, a fixture app click, a path-builder helper or a Preview load, and spot-read the hand-built ones. B rows that hold: `tests/test_replay.cpp` 179, 360, 550, 649 (property of the same record; `REQUIRE(!p0["activations"].empty())` at one chart, not window-sensitive in a way I can show), 871, 917, 994, 1595, 1649 and the hand-built ones; `tests/test_replay_schedule.cpp` all hand-built geometry cases (419, 473 and 697 and the rest, which type Freak and Rescued numbers into `split_squeeze` with no search), and 208, 822, 851, 964, 1130, 1378 (both sides from the same stored path); `tests/test_replay_windows.cpp` all 7; `tests/test_report_rows.cpp` (counts and wording from one stored record, and `store_tied`'s hand-built record); `tests/test_app_click.cpp` (row against viewed record, or against a fresh `analyze_chart_file` on the same settings); `tests/test_app_reports.cpp` and `tests/test_analysis.cpp` (store, counts, and `run_batch` where `paths.front().path == engine.best_path().pathstring()` compares two reads of the same search); `tests/test_cli.cpp` 429 and 529 (they assert "0 only under 1.0, 0 only under 1.1", which is about which rule has a record, not a score; the chart at `small_chart()`, `tests/test_cli.cpp:121`, is the smallest corpus chart with two or more phrases); `tests/test_pathstring_read.cpp` 82, 126, 135, 175 (round trip: the path the search found at W is read back by a graph built at W); `tests/test_preview_scene.cpp` all but 898; `tests/test_song_parse.cpp` 1232, 1253 (the freeze error is thrown at load, before any search).

### Disputes: these B rows should be C (13)

Each runs the search at the default window and asserts a value taken from that run.

1. `tests/test_replay_schedule.cpp:1514` "schedule: Pitch Dark prices each read at its own step's scale". `pitch_dark()` (1448) takes `corpus::analyzed(...)` at `Settings().to_analysis_settings()`, then `paths.front()`. Lines 1531 to 1540 type `steps[0].tick == 178560`, 182280, 198570, 194080, 202440, 202480, 206280, and `pd.acts.size() == 3`, `steps.size() == 4` ("row P3"). `kPitchDark243OwnEndMs = 100949.4546` (1437) is "pinned from one run". The case names a real chart and a stored path from the search; under 60 ms the first path may not be the one at 85.
2. `tests/test_replay_schedule.cpp:1621` "schedule: Pitch Dark's read range is its read end ...". Same `pitch_dark()`.
3. `tests/test_preview_scene.cpp:898` "a corpus best path as the ghost lists its taken fills". `CHECK(scene.ghost_fills == std::vector<size_t>{2, 7})` on `corpus::analyzed_by_ten_scores().record.best_path()`. A typed list taken from a search run.
4. `tests/test_preview_controller.cpp:614` "set_ghost_path: the ghost's fills land on the scene ...". The same `{2, 7}` at (614 + 95 =) about line 643, on the same best path.
5. `tests/test_activation_rows.cpp:1331` "dynamic hit window: Burnout's squeeze out has 5.4 ms safe". `burnout()` runs the search at the scratch default (60). It requires `best.pathstring() == "3- 1 2"` and types "frontend 84.5 ms [K   G]   backend 84.3 ms [  Y  ]" and "5.4 ms safe". The sister cases (1308, 1354, 1396 and others) are in the delta round because their lines changed; this one did not, and the auditor's B reason ("burnout(,early_fill_window") is a token hit, not a reason.
6. `tests/test_saved_figure_fill.cpp:93` "a saved path with a squeeze-in on the SP end gets its figure back". `fill_saved_hardest` builds the graph with `search_graph(song, settings)` (`src/app/saved_figure_fill.cpp:100`) and replays a stored path text on it. The test types `paths[0].path == "1 0+ 0 0"` and the figures `{0.0, 0.0, 0.0, nullopt, 0.0}`. Whether a path text places at all, and the figure it gets, comes from the graph at 60.
7. `tests/test_saved_figure_fill.cpp:133` "a row that disagrees with its replay keeps its empty figure": `after[1] == 0.0` from the same graph.
8. `tests/test_saved_figure_fill.cpp:147` "a stop leaves the marker unset": `figures_of(...)[0] == 0.0` after the run.
9. `tests/test_saved_figure_fill.cpp:286` "an inside-Star-Power squeeze-in fills a negative figure ...". `saved.record.paths[0].path == "1 2 3+"` and `*figure == -189.83750000002328` on Everlong. The sharpest one: a typed double out of the replay on the 60 ms graph.
10. `tests/test_saved_figure_fill.cpp:308` "the fill hands every row of one chart the same graph": the figures `{0.0, 0.0, 0.0, nullopt, 0.0}` at 331 to 332.
11. `tests/test_pb_text.cpp:112` "choice order: cheapest first, ties in chart order, pending last". `player_path(song, {})` is `search_target(song, default_cfg(), ...)`; line 118 types the offered fill ticks `{55680, 71040, 90240, 105600, ...}` of Allister - Overrated as `REQUIRE(offered == ...)`.
12. `tests/test_pb_text.cpp:298` "fill card: an offered fill reads the heading, three rows and two grey lines". `OverratedCard` (262) runs `best_finish(song, default_cfg(), ...)` and `corpus::analyzed(... default_cfg())`. It types "m30 · 0:45.789", "Phrases saved=2", "Best finish here=226,210", "Cost=-2,500" (the file's own comment: "Pinned from one run").
13. `tests/test_pb_text.cpp:313` "fill card: a costless fill reads best ...". Same card: "m38 · 0:58.421", "Best finish here=228,710", "Cost=best".

### Weaker, not disputed

- `tests/test_replay_schedule.cpp:241` ("declined chords are the skipped fill ticks") ends on `CHECK(declined_rows > 0)`, and 208, 851, 1130 end on `rows > 0` or `acts_seen > 0`. Those floors count search output. I found no way for 60 vs 85 to make them zero, but the auditor's B reason does not say why either.
- `tests/test_preview_controller.cpp:575` `REQUIRE_FALSE(other_key.empty())` needs a second path from the search at that cap.

## 5. C records

Each record has the title, the old record pointer, the window line, a typed-number line, and a hedged verdict ("Investigate (claim not checked only), unless the old verdict was Rewrite or Remove ..."). None answers question 1 to 12, and none says "proven by" or "by reading".

Findings across all 45:

- **The old record was located for only 8 of 45.** The other 37 say "heading not found by title match" and send the verifier to `coverage.md`. They exist. `coverage.md` maps the files: pb_coach to `records/C7.md` (records 7 to 25: titles match, for example "### 3. coach click: a refusal is current only ..." and "### 13. coach finish ..."), pb_text to `records/A19.md` (headings PB2 to PB6 and the refusal and card records, verdicts "Rewrite" for 26 to 31), preview_scene to `records/A20.md` (289: "Investigate, claim not checked only"; 925: "Rewrite ..."), pathstring_read to `records/B2.md`, app_click to `records/A5.md`, saved_figure_fill to `records/A27.md` (SF9: "Investigate: claim not checked only"; SF10: "Rewrite"), activation_rows 582 to `records/A2.md` ("## 13. activation rows: Burnout's three activations"). The auditor's rule "the old verdict stays unless a fault in the test itself" cannot be applied without them. Records 7 to 25, 26 to 31, 32, 33, 34 to 37, 38 to 42 and 44 therefore give no old verdict and no new one.
- **The verdict is not a verdict.** "Investigate ... unless the old verdict was Rewrite or Remove" leaves both open.
- **Typed numbers are understated** (the line says "none found by the scan" where the assertion lines hold typed engine values):
  - Record 1 (`test_replay.cpp:104`): `min_paths` 382, 382 and 294 at lines 121 to 123, compared at 142 (`run.paths >= c.min_paths`). The test's own comment calls it "a floor pinned from one run". The path count is a search count that can move at 60.
  - Record 3 (`test_replay.cpp:311`): `Setting{4, 4, 0, 10}, Setting{4, 40, 0, 200}, Setting{2, 40, 4, 200}` (skipped 0, 0, 4; min_variants 10, 200, 200), with `CHECK(n.skipped() == s.skipped)` at 331, and the comment's "today the three settings list 12, 295 and 303 variants". Both are counts of a search's variants.
  - Record 5 (`test_replay.cpp:611`): `"17360"` at 635.
  - Record 6 (`test_replay.cpp:888`): `{19200}` at 909 to 910.
  - Records 10 and 11 (`test_pb_coach.cpp:133,178`): `"needs 130.4 ms, your limit is 10 ms"` at 163 and 174.
  - Record 12 (`test_pb_coach.cpp:202`): `kFillTick = 36480` at 209.
  - Record 14 (`test_pb_coach.cpp:276`): `"130.4 ms"` at 273.
  - Record 7: the ladder `{378315, 364675, 364675}` at 66 is only partly listed.
  - Record 23 (`test_pb_coach.cpp:572`): `{706180, 706180, 706180}`, 199680 and the tick list `{149760, 165120, 180480, 199680, 245760, 272640}` at 582 to 607; 24 and 25 list some.
  - Records 26, 27, 28, 29, 31 (`test_pb_text.cpp:56,131,146,165,326`): "2:51.485 · 2 phrases · ends m135 · squeeze out 130.4 ms", "m108 → m116", "m132 → m135", "squeeze out 130.4 ms", "ends m135 ...": all typed from the search's record.
  - Record 30 lists 72480 and 195360; it also types the kinds.
- **Record 35** (`test_pathstring_read.cpp:99`) refuses "   " and "99": refusal is a parse check, so it is closer to B (the first chart's graph is built but the text never places). I do not dispute it. I note the auditor gave no reason.
- Question 8's named bug at a line appears in none of the 45. Question 3's engine dump at 60 ms is named in general terms ("the chart the test uses") and not by chart for records 1 to 6 and 7 to 25 (the tests do name their charts: Burnout / White Washed / Allister - Overrated / Pitch Dark).
- Verdict agreement: I cannot agree with any of the 45 as written. For records 1 to 6, 43 (`B1` record 18, "Rewrite ... state the sum as a property") and 45 (`A3` record 22, "Rewrite ...") the old verdict is quoted correctly (by reading A23, A24, B1, A3), and keeping them is reasonable; they are still missing the typed numbers above. Records 5 and 6 and 32 are old "Investigate, claim not checked only", which fits.
- Not found as a typed value, but a likely split with the window: `tests/test_preview_scene.cpp:289`'s record types 19200 (matches line 289's body).

## 6. Splits

- Test of Pitch Dark's SP end steps (`test_replay_schedule.cpp:1531 to 1540`) types `steps[1].end_tick == 198570` and `kPitchDark243OwnEndMs`. The engine's own record is `records-engine/` at 85 ms. The window moves which path is `paths.front()`, so the test and the engine record (at 85) may answer the same chart differently.
- `tests/test_activation_rows.cpp:1228` sets 85 for the "dynamic hit window" cases and `burnout()` (default 60) feeds 582, 1331, 1543 and 1556 in the same file. One file runs two windows. The cases at 582 and 1331 type the same family of numbers ("84.5 ms", "5.4 ms safe") against the second.
- `tests/song_digest.h:332` sets 85 for the digest and edge cases while the digest's own `to_analysis_settings` default is 60: the digest tests pin parse and prepared rows only, so no value moves, but the helper's 85 is a second window in the same corpus runs.

## 7. Not reached

I did not read line by line: the hand-built B rows in `test_replay_schedule.cpp` (106 to 773, 905 to 939, 1005 to 1098, 1272 to 1345, 1514 onward other than the two named), `test_song_parse.cpp` (50 B rows, only 1232, 1253, 1292, 1387 and 1390 read), `test_analysis.cpp` (48 B rows; read 173, 198 and the token list), `test_report_rows.cpp` (read 204, 214, 636 to 700 and typed values by grep), `test_preview_scene.cpp` rows 37 to 448 and 690 to 744 (token and typed-number scan only), and `test_app_reports.cpp` (assertion lines by grep). I read no engine dump; none exists at 60.
