Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task O3a: parse, search and timing owners (wave D1)

**Orchestrator answers (2026-10-04, binding; they override the open questions below).** Layering: move `strip_rich_tags` and its tag table down into `src/parse/song.h`/`song.cpp`; `src/app/library_query.h`/`.cpp` keep their callers working by calling the moved function (add those two files to your owned files; S1 edits other lines of `library_query.h` in a later wave). Its existing tests keep passing unchanged. `graph.cpp`: yes, you own it for the finding-41 comment only. `fill_rule_description`: reuse the fill report footer's two sentences, as written. Finding 8 also covers the artist: `display_title`'s rule (strip every tag, trim, "(unknown)" when empty) is the one rule; if a separate artist helper is needed, it calls the same owner. This last point waits on the user; build only the title owner unless the main session says otherwise.

Task id: O3a. Base: the `claude/p3-c` join commit (db97ea5 (claude/p3-c)). Branch: `claude/p3-<id lower case>`, so `claude/p3-o3a`.

## Goal

You add three owner functions and change no caller. One cleans a song title, one names a fill rule, one turns a playhead time into a tick. Wave D2 points the screens at them later; your job is the owner, its test and its scan row. You also fix two stale comments that phase 4 assigned to this task. Nothing you do changes anything on screen, in a score or in a stored record.

## Finding 8 and 111: one cleaned song title (D48 Q14)

Today two strippers exist. `strip_rich_tags` in `src/app/library_query.cpp` knows the eight Clone Hero rich-text tags. `plain` in `src/app/report.cpp` knows only `<color>`. The Library, the busy tooltip, the batch strip and `cli/batch.cpp` print titles raw or half-stripped. `title_or_unknown` in `src/parse/song.cpp` only turns an empty name or the old `<unknown title>` placeholder into "(unknown)".

D48 says: one cleaned title everywhere, and "(unknown)" when nothing is left.

The owner you add is `display_title` in `src/parse/song.h` and `song.cpp`, right beside `title_or_unknown`. It strips the tags through `strip_rich_tags`, trims leading and trailing spaces, then falls back the way `title_or_unknown` does. `title_or_unknown` stays, because stored names still reach it. On screen nothing moves in this task. In D2 a `<b>` title will read clean on every page, and a title made only of tags will read "(unknown)" in the Library too.

Test cases (in `tests/test_song.cpp`, which already includes `parse/song.h`; `tests/test_model.cpp` belongs to O1 this wave):
- "display_title: a title made only of Clone Hero tags reads (unknown)". Use H1's tag-only title fixture from `tests/display_fixtures.h` (read that file for its name). Also pin that a title of spaces alone reads "(unknown)".
- "display_title: tags go, words stay, spaces are trimmed". Pin the same inputs the existing `strip_rich_tags` test uses: `<b>Bold</b>` reads "Bold" and `<color=#e02222>Blood</color>line` reads "Bloodline". A title with a leading or trailing space reads without it.
- "display_title: a clean name and the old placeholder behave like title_or_unknown". "Some Song" passes through; `<unknown title>` reads "(unknown)" (both from the existing `title_or_unknown` case).

## Finding 55: one name per fill rule (D48 Q13)

Today the rule itself has one owner, `FillDeadlineRule` and `engine_mode_stamp` in `src/search/graph.h`. Its names do not. `cli/batch.cpp` says "Clone Hero 1.0" at one line and "Clone Hero 1.0 (legacy)" forty lines later. `library_dialogs.cpp` and `report.cpp` type the long name by hand. `fill_report.cpp` types "CH 1.0" and "CH 1.1" for its columns and heading, and its footer carries the one-sentence explanation of each rule.

D48 says: "Clone Hero 1.0" in sentences, "CH 1.0" in narrow columns, both from one place, and "(legacy)" goes. The checkbox label "1.0 fills" stays as it is (ADR 0010).

You add `fill_rule_name(rule, style)` and `fill_rule_description(rule)` in `src/search/graph.h` beside `engine_mode_stamp`. The name has a long form and a short form. The description is the sentence `fill_report.cpp` already prints for each rule in its footer: "Clone Hero 1.0 gave you until about one fill-length before the fill." and "Clone Hero 1.1 made it a flat 4 beats." Reuse those words; do not write new ones. On screen nothing changes now. In D2, hydra_batch loses "(legacy)" and every label reads the owner's text.

Test cases (in `tests/test_fill_deadline.cpp`, which includes `search/graph.h`):
- "fill rule names: one long and one short name per rule, and no (legacy)". Pin "Clone Hero 1.0", "Clone Hero 1.1", "CH 1.0", "CH 1.1", and that no name contains "(legacy)".
- "fill rule descriptions: one sentence per rule". Pin the two footer sentences above and that the two differ.

## Finding 57: which tick the Preview shows (D48 Q23)

Today `tick_at` in `src/app/preview_view.cpp` rounds `MsIndex::tick_at_ms` to the nearest tick and floors at 0. `build_activations` in `src/app/path_view.cpp` repeats that rounding by hand. `build_time_box` looks the tempo up by milliseconds but the meter and section up by that rounded tick, so for up to half a tick before a tempo change the BPM line lags the other lines.

D48 says: all four lines switch together. Only playback or scrubbing can land there.

You add `SongTiming::display_tick_at_ms(ms)` in `src/core/timing.h` and `timing.cpp`: round to the nearest tick, never below 0, the same answer `tick_at` gives today. You change no caller. In D2, K4a makes `tick_at` call it and looks the tempo up by that tick, and K2 replaces the hand rounding in `path_view.cpp`.

Test case (in `tests/test_timing.cpp`):
- "timing: display_tick_at_ms rounds to the nearest tick and never goes below 0". Use a flat 120 BPM song at resolution 480 like the file's existing cases. Pin that 999.9 ms gives tick 960 (the change's own tick, which is what "the half-tick case returns the new tick" means), that 1000.0 ms gives 960, and that a negative time gives 0. Those inputs come from the step 3a draft; add no other numbers.

## Phase 4 finding 41: two comments in your files

The handoff gives you two comment fixes because you own the files. Check each claim against the code before you reword it.

`src/search/graph.cpp`, the comment above the Clone Hero 1.0 branch of `activation_fill_deadline_ms` (around lines 76 to 78), says the legacy mode "never writes the database the GUI reads". That stopped being true when each result started carrying its rule in its key (`store::Lens::legacy_fills`, ADR 0010). Say instead that a 1.0 result is stored under its own key beside the 1.1 results. Keep the sentence that the legacy rule is off the bit-for-bit scoring surface on purpose.

`src/core/timing.h`, the comment above `ms_at_tick_f` and `tick_at_ms` (around lines 47 to 53), calls them helpers for "the display-layer exact squeeze solver" and says the engine's rebuild step calls `tick_at_ms` through `frontend_transfer_scales`. I checked main at dc7e582: `frontend_transfer_scales` in `src/core/squeeze_rating.cpp` uses only `ms_per_measure_at`, and no exact squeeze solver exists. The callers today are the Preview time box (`tick_at` in `preview_view.cpp`), the Paths timeline end (`path_view.cpp`), span ends in `render/track_state.cpp`, the legacy fill deadline in `graph.cpp`, and `SongTiming::sp_end_ms`, which only tests call. Reword the comment to name those, and add `display_tick_at_ms` once you write it. Keep the warning that the graph's scoring path must not use them. Re-verify with a grep before you commit.

## Owned files (only these may change)

`src/parse/song.h`, `src/parse/song.cpp`, `src/search/graph.h`, `src/search/graph.cpp` (the one comment only), `src/core/timing.h`, `src/core/timing.cpp`, `tests/test_song.cpp`, `tests/test_fill_deadline.cpp`, `tests/test_timing.cpp`, `tests/test_single_owner.cpp` (your two rows only).

`song.cpp` will include `app/library_query.h` for `strip_rich_tags`. Both files are in the same `hydra_core` library, so it builds; see the open question below.

## Tests you may run

`build-cpp\Release\hydra_tests.exe` with: `-tc="display_title*"`, `-tc="title_or_unknown*"`, `-tc="library query: rich-text tags*"`, `-sf=*test_fill_deadline*`, `-tc="timing: display_tick_at_ms*"`, `-tc="timing: continuous helpers*"`, `-tc="single-owner*"`. No `hydra_uitest` script; this task has no screen.

## Scan rows (tests/test_single_owner.cpp)

Row 1. Question: "Which rule names a fill deadline?" Owner: `fill_rule_name` in `src/search/graph.h`. It must flag the line `out.fills = s.legacy_fills ? "Clone Hero 1.0" : "Clone Hero 1.1";` (library_dialogs.cpp). It must not flag `const char* ch10 = hydra::engine_mode_stamp(hydra::FillDeadlineRule::Ch10);` (batch.cpp). List every line it flags today as a known copy, with `removed_by` naming the D2 task that owns the file: K5 for `cli/batch.cpp`, K3 for `library_dialogs.cpp`, `settings_bar.cpp` and `library_toolbar.cpp`, K1a for `report.cpp`, K1b for `fill_report.cpp` and `cli/fillcompare.cpp`. A sentence about the game version in help text is prose, not a label; shape the pattern so a quoted string that begins with the name is flagged and a sentence that merely mentions it is not.

Row 2. Question: "Which tags does Hydra strip from a song name?" Owner: `strip_rich_tags` in `src/app/library_query.cpp`, read through `display_title`. It must flag `static const char* kWord = "color";` (report.cpp, inside `plain`). It must not flag `row.title = app::strip_rich_tags(entry.title);` (library_model.cpp). Known copy: `plain` in `report.cpp`, `removed_by` K1a.

## Done when

A title made only of tags reads "(unknown)". `<b>` titles read clean. The half-tick case returns the new tick. Each owner has its test and its scan row, and the two comments say what the code does.

## Open questions

- Layering. The plan puts `display_title` in `src/parse/song.h`, but `strip_rich_tags` lives in `src/app/library_query.cpp`. Both are in `hydra_core`, so it compiles, yet parse would then depend on app. The alternative is to move `strip_rich_tags` and its tag table down to parse, which touches a file no D1 task owns (S1 owns `library_query.h` in D3). I kept the plan's placement; say if you want the move instead.
- The plan's D1 table lists `src/search/graph.h` for O3a but not `graph.cpp`; the handoff assigns the `graph.cpp` comment to O3a. This brief owns `graph.cpp` for that comment only.
- `fill_rule_description` has no decided sentence in D48. This brief reuses the two sentences the fill report's footer already prints, so no new wording appears. Say if you want different words.
