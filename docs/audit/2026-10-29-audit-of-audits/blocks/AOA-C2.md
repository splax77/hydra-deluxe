# AOA-C2: the concept audit's code-only findings (second half)

Common closure for every item below: the paragraph "Code-only findings, with the owner each verifier proposed" in `docs/audit/2026-10-11-concept-audit/questions.md:215-219` ("These proceed on your yes to the fix wave. Veto by id."), and D114 item 22 in `docs/audit/2026-10-03-fix-decisions.md:573`: "**The code-only findings.** Every finding the questions file lists as code-only is fixed with the owner its verifier proposed; none was vetoed. That includes M52's visible side effect: the Preview's jump buttons grey during a load, where today they are live and do nothing." The user's reply recorded at `fix-decisions.md:551`: "go with recommended". Commit hashes are on `main` (1fc55723).

### concept audit M06: small view helpers copied between the two window views

- **Splits:** None: a code copy. V1 writes "Disagree on: none today" (findings/V1.md:203), table rows at V1.md:193-199 all give the same answer for like-typed values (the `posted` empty-sinks rule is DM-only and recorded as D103 items 16-17).
- **How it left:** code-only. Closure's words: "M06 (view helpers): `report_view.h` gains one `optional_key` and one `text_column`." (questions.md:219).
- **User's words:** the batch reply "go with recommended" (fix-decisions.md:551) and D114 item 22 (fix-decisions.md:573). No split to name.
- **Code today:** `optional_key` at src/app/report_view.h:97 and `text_column` at report_view.h:104; both views `using report_view::optional_key` / `text_column` (src/app/path_report_view.cpp:13-15, dm_report_view.cpp:13-15); `ms_key` has no grep hit. By reading.
- **Verdict:** GONE
- **Why:** The copy is removed by commit d9442213 ("M06: one optional_key and one text_column, in report_view.h") and no input gave two answers before.

### concept audit M08: "keep listed records whose chart the library lists", SQL join against the in-memory test

- **Splits:** None for the two report loops (V1.md:255, :264 table: same answer). One rare input in the SQL pair: a stored hash differing from the library's md5 only in letter case would be counted by the report (both lower-cased in `records_by_hash`) and deleted by the next rescan purge (V1.md:266). V1 itself says the scan writes lower-case hex and it found no path that writes an upper-case `results.hyhash`; so no input the user can reach.
- **How it left:** code-only. Closure's words: "M08 (listed-records loops): one "listings the library lists" helper for the two loops; the SQL stays." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** the helper `listings_the_library_lists` came in with 11014fe9, then D115 commits b19db916 and 79fcbb9e rewrote report.cpp: `grep library_lists|library_copies_by_hash|list_records` finds nothing in src/. The page now reads `store::ChartIndex` (src/app/report.cpp:101, :153, :238). By reading.
- **Verdict:** GONE
- **Why:** Both loops are gone (11014fe9 merged them, D115 b19db916 replaced the whole listing path), and the store's results table that could hold an upper-case hash is replaced by `records` / `profiles` (src/store/record_store.cpp:244-262).

### concept audit M10: what "Clear filters" resets, and what "no filter" means

- **Splits:** None: a literal 0 for "keep everything" in seven places (V1.md:303, :320 "Disagree on: none today").
- **How it left:** code-only. Closure's words: "M10 ("all" choice): the choice lists own their "all" entry as a named constant." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `app::report_view::kEveryRowChoice` used at src/ui/path_report_window.cpp:39,82,108,175 and src/ui/dm_report_window.cpp:34,56,89. By reading.
- **Verdict:** GONE
- **Why:** Commit 14885849 ("M10: the report dropdowns' keep-everything choice is one named index"); the sides always agreed.

### concept audit M11: which settings each report page reads, and which make an open report out of date

- **Splits:** The path report reads `hit_window_ms` but `settings_change_touches` did not list it (V2.md:21, :28). V2.md:25-27 settles that no control writes `hit_window_ms` (only the INI loader at start-up), so no reachable input differs: "the gap is real in the list and dormant in use".
- **How it left:** code-only. Closure's words: "M11 (settings a page reads): each generator states its settings once; smallest step adds `hit_window_ms` to the path report's list." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `PageSettings{cap, lens, hit_window_ms}` and `settings_change_touches` comparing `PageSettings::of(before) != PageSettings::of(after)`, src/app/report.cpp:357-359. By reading.
- **Verdict:** GONE
- **Why:** Commit 221a7c00 ("M11: the path report states once which settings it reads"). Nothing the user can reach differed before, and nothing visible changes now.

### concept audit M12: chart mode, cap and lens as three loose parts

- **Splits:** None (V2.md:43 "Disagree on: none today").
- **How it left:** code-only. Closure's words: "M12 (chart mode, cap, lens): one struct for the record key, per its block." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** commit 9875abad adds `store::ResultSettings`; D115 later replaced the results table by `profiles` / `records` (src/store/record_store.cpp:244-262). The commit says "No stored row changes". By reading the commit body.
- **Verdict:** GONE
- **Why:** The loose triple was joined into one value in 9875abad, with no change a player sees.

### concept audit M13: the Analysis panel's settings spelled by hand in several lists

- **Splits:** None. V2.md:62 names two edges (an uncleaned difficulty string; `depth_mode` 2) and says "Neither changes what the user sees"; V2.md:64 "Disagree on: none today".
- **How it left:** code-only. Closure's words: "M13 (the panel's settings list): one list of the analysis fields." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `kAnalysisFields` (10 entries) at src/app/config.h:336, referenced by the reset comments at config.h:115, :279, :284. By reading.
- **Verdict:** GONE
- **Why:** Commit c4217be5 ("M13: the analysis settings are listed once, in kAnalysisFields"). No reachable disagreement before.

### concept audit M14: what 0 and 1 mean for the score-range unit

- **Splits:** None reachable. An int other than 0 or 1 gave scores in the search but a different stored lens (V2.md:77-78), yet "unreachable in the app": the INI loader clamps to the default 0 and the combo writes 0 or 1.
- **How it left:** code-only. Closure's words: "M14 (score-range unit): `Settings::lens()` passes `search_depth_mode() == Points`." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `Settings::lens()` passes `search_depth_mode() == DepthMode::Points ? 1 : 0` (src/app/config.cpp:423-428). By reading.
- **Verdict:** GONE
- **Why:** Commit 1d3fdc8e ("M14: the lens files the score-range unit the search ran with"). Stored rows for 0 and 1 are unchanged.

### concept audit M18: `settings_key` against `RecordKey`

- **Splits:** None: `settings_key` had no caller in `src/` (V2.md:164, :167); the two lists differ on rules and parse choices by design of the types.
- **How it left:** code-only. Closure's words: "M18 (`settings_key`): moves to the tests, its only user." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `grep settings_key src/` finds nothing. By grep.
- **Verdict:** GONE
- **Why:** Commit ed5bd9e6 ("M18: move settings_key out of src/ into the test cache that uses it").

### concept audit M19: what makes a Preview "another song", and the four parse choices

- **Splits:** None today (V2.md:195). V2.md:193 names a dormant rules edge: a rules-only change builds nothing in `PreviewController::open`. It is unreachable because `settings.rules` is set once at start-up.
- **How it left:** code-only. Closure's words: "M19 (Preview song key): one `Settings::reading()` for the four parse choices." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `ChartReading` (src/parse/chart_reading.h:22), `Settings::reading()` (src/app/config.h:259), `PreviewSongKey(md5, const ChartReading&, FillDeadlineRule)` (src/ui/preview_controller.h:60). `rules` is still assigned only at src/ui/app_state.cpp:32. By reading.
- **Verdict:** GONE
- **Why:** Commits f356febe, 4c1a5772, b9e7cff1, 3bec6c59 and later (merge abfbf1eb, 5de3c1b4, 64f3baa2) put one value through key and jobs; the rules edge stays dormant.

### concept audit M20: which columns make one result row's identity

- **Splits:** None (V2.md:210 "Disagree on: none today").
- **How it left:** code-only. Closure's words: "M20 (row identity): one list of identity columns in the store." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** commit 913c1299 built the columns from one list; D115 later replaced the `results` table with `profiles` and `records` (src/store/record_store.cpp:244-262), so the lens_match / replace-purge spellings in V2.md:203-208 no longer exist. By reading.
- **Verdict:** GONE
- **Why:** 913c1299, then D115's new tables.

### concept audit M21: what one scanned chart holds, and the eight hand-copied fields

- **Splits:** None reachable. V3.md:29 lists `sig` blanked in one direction by `scan_item_of`, then says "The `sig` blank is invisible today ... no code does that"; the empty-sig row and stale-stamp case are "deliberate and owned once".
- **How it left:** code-only. Closure's words: "M21 (scanned chart's fields): one struct, per its block." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `using ScanItem = store::ChartLibraryEntry;` (src/app/analysis.h:33); `grep scan_item_of|to_library_entry src/` finds nothing. By grep.
- **Verdict:** GONE
- **Why:** Commits e8ffe8df (store half) and f8199781 (ScanItem alias); the `sig` blanking converter no longer exists.

### concept audit M22: why a result is Stale, and the library's "did it change" test

- **Splits:** None (V3.md:49 "Disagree on: none today"). V3 notes the two "did it change" tests answer different questions.
- **How it left:** code-only. Closure's words: "M22 (why Stale): `LibraryRow::shows_same_as(lookup)` beside `facts_of`." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `bool LibraryRow::shows_same_as(const store::RecordLookup& lookup) const` (src/ui/library_model.cpp:57, declared library_model.h:108), called at library_model.cpp:77. By reading.
- **Verdict:** GONE
- **Why:** Commit adfe63ae ("M22: LibraryRow::shows_same_as owns which summary fields the library reads"); nothing differed before.

### concept audit M25: what a command-line tool does to start

- **Splits:** One. The question "what does a command-line tool print for an unknown option": `hydra_batch` and `hydra_fillcompare` print `Unknown option: %s` (src/cli/batch.cpp:86, src/cli/fillcompare.cpp:65) and exit 2; `hydra_replay` prints `unknown option %s` then the usage (tools/replay.cpp:768) and exits 2. V3.md:108 names it: "Disagree on: replay's lower-case unknown-option text (a different tool, and its wording is user-visible)". The rules-file frame itself (four copies, same answer) is not a split.
- **How it left:** code-only. Closure's words: "M25 (tool start-up): `app::load_tool_rules`, replay's lower-case text left alone." (questions.md:219).
- **User's words:** None found for the unknown-option wording. Searched fix-decisions.md for "unknown option" and "replay" under D114 and found none, and the question files of the concept audit. The batch reply was "go with recommended" (fix-decisions.md:551) on a paragraph that says only "replay's lower-case text left alone".
- **Code today:** `load_tool_rules` is called by all four tools (src/cli/batch.cpp:102, src/cli/fillcompare.cpp:74, tools/bench.cpp:518, tools/replay.cpp:775), commit 3f77b74b. The unknown-option lines still differ as above. By reading and grep.
- **Verdict:** REOPEN
- **Why:** The rules frame is fixed, but the item also held a wording difference the user can see on the command line, and it was left in place with no ruling. It is low weight (replay is a developer tool).
- **Examples:** `hydra_batch --bogus` prints `Unknown option: --bogus`; `hydra_replay --bogus` prints `unknown option --bogus` then the usage text; both exit 2 (the gap is the capital letter, the colon and the usage block). `tools/precheck/main.cpp:77` is a third spelling (`unknown option `) in yet another tool.

### concept audit M26: scan against the cache, then save as the library

- **Splits:** None of any size: V3.md:127 "Disagree on: bench's missing try (a per-tool choice). Nothing else."
- **How it left:** code-only. Closure's words: "M26 (scan against cache): `discover_charts` takes the cache by reference, `app::load_scan_cache` owns the swallowed failure." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `store::ChartLibraryCache load_scan_cache(store::RecordStore&)` swallows a read failure (src/app/analysis.cpp:430-436); used at src/cli/batch.cpp:163 and src/ui/library_jobs.cpp:174. `grep "cache.empty() ? nullptr"` finds nothing. tools/bench.cpp:165 and :270 pass a `cache` variable to `discover_charts` (not read to confirm whether they use `load_scan_cache`). By grep.
- **Verdict:** GONE
- **Why:** Commit ab00c8e7 ("M26: one rescan-cache read, and discover_charts takes the cache by reference"). The bench tool's missing try was a per-tool choice that changes nothing a player sees.

### concept audit M28: begin, do the work, commit, roll back, written five times

- **Splits:** None (V3.md:144 "Disagree on: none today").
- **How it left:** code-only. Closure's words: "M28 (transactions): `RecordStore::in_transaction`." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `RecordStore::in_transaction(F work)` (src/store/record_store.h:602, record_store.cpp:549) with one `exec("BEGIN")` (record_store.cpp:550); the save paths keep `ctl("BEGIN")` (:971, :1057) as V3 said they should. By grep.
- **Verdict:** GONE
- **Why:** Commit b305cca5 ("M28: one transaction frame in the store, RecordStore::in_transaction").

### concept audit M30: the six score categories and the engine's running total

- **Splits:** None (V4.md:18 "Disagree on: none today"; the list of six is a per-module fact; `emit_path` throws if the sums differ).
- **How it left:** code-only. Closure's words: "M30 (running total): the engine reads `score_total`." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `p.score += score_total(e.notes_points, e.comboscore, e.spscore, e.soloscore, ...` (src/search/engine.cpp:870), with the check at :1758. By reading.
- **Verdict:** GONE
- **Why:** Commit 0d6fa00e ("M30: the engine's running score uses score_total"); the hand sum agreed with the owner before.

### concept audit M33 part A: the engine asks `after_sp_end` again on a choice that carries `late`

- **Splits:** None for part A. V4.md:81: "`sq->late` and `after_sp_end(sq->chord, sp_end)` are the same two integers compared, on every input." Part B (the tick test against the ms test, which split on an absurd tempo) is not mine and was left untouched on purpose by the same line of the paragraph.
- **How it left:** code-only. Closure's words: "M33 part A: the engine reads `sq->late` instead of re-asking `after_sp_end`; part B untouched." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** the only `after_sp_end` mention in src/search/engine.cpp is a comment at :1075 saying the choice's `late` is that answer. By grep.
- **Verdict:** GONE
- **Why:** Commit 43b129d5 ("M33 part A: the squeeze-out bank reads the choice's late flag").

### concept audit M35: how the search graph is built

- **Splits:** None (V4.md:124 "Disagree on: none today"; the unset `build_cap` branch was never taken).
- **How it left:** code-only. Closure's words: "M35 (graph builders): one builder expression, per its block." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `ScoreGraph search_graph(const Song&, const SearchSettings&)` (src/search/pather.cpp:261) called at :390, :403, :440, :535. By grep.
- **Verdict:** GONE
- **Why:** Commit 3cc17b9b ("M35: one search graph builder for analysis and target searches").

### concept audit M38: the fill-deadline "4 beats" rule written as a calculation and as a sentence

- **Splits:** None (V4.md:175 "Disagree on: none today"; only the Ch11 "4" is a copy). The user-facing sentence stays word for word.
- **How it left:** code-only. Closure's words: "M38 (the "4 beats" sentence): the number comes from the constant, sentence unchanged." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `inline constexpr int kCh11FillLeadBeats = 4;` (src/search/graph.h:71), used at graph.cpp:69 and in the sentence at graph.h:79. By reading.
- **Verdict:** GONE
- **Why:** Commit 57c5da7b ("M38: one constant for Clone Hero 1.1's fill lead in beats").

### concept audit M42: what each path group is called on screen

- **Splits:** None: V5.md:111 "Disagree on: none today (same meaning, per-view spelling)". Each screen words the one `Group` enum for itself: heading "Optimal", picker "<notation>  (optimal)", headline "Optimal path" (V5.md:109). That is per-screen text on one meaning, not two rules.
- **How it left:** code-only, deferred. Closure's words: "M42 (group names): a `group_name(Group)` table only with the next rename." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573). No change was made, so nothing was decided beyond "later".
- **Code today:** `grep group_name src/` finds nothing; the copy is still there and still agrees (the all-0 and optimal words come from `src/ui/paths_tab.cpp`, `src/app/path_view.cpp`, `src/app/preview_view.cpp`). No commit mentions M42. By grep.
- **Verdict:** PASS
- **Why:** Nothing was fixed and nothing needed to be: the three spellings answer "what do we call this group on this screen", which each screen answers in its own words. A rename would touch all three, which is why the paragraph defers it.

### concept audit M43: the engine's SQ_IN / SQ_OUT ints against `SqueezeKind`

- **Splits:** None reachable: V5.md:122-123 "They agree for the only two values ever pushed ... would split on a third value or a flipped enum order."
- **How it left:** code-only. Closure's words: "M43 (SQ_IN/SQ_OUT): `SqueezeKind` used directly." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `grep "SQ_IN\b" src/` finds nothing. By grep.
- **Verdict:** GONE
- **Why:** Commit 61d85ba0 ("M43: the engine's squeeze records hold SqueezeKind").

### concept audit M44: how an activation's deactivation tick is read

- **Splits:** None: `activation_deact_tick` returned `act.deact_tick()` and nothing else (V5.md:132).
- **How it left:** code-only. Closure's words: "M44 (deact tick): `Activation::deact_tick`, the pass-through deleted." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** the free function is gone; src/core/squeeze_rating.cpp:25 reads `act.deact_tick()`. By reading.
- **Verdict:** GONE
- **Why:** Commit b7b034c9 ("M44: read the deactivation tick through Activation::deact_tick only").

### concept audit M45: does squeezing out this row cost points

- **Splits:** None: a branch that could never run (V5.md:141 "the branch ... cannot run").
- **How it left:** code-only. Closure's words: "M45 (squeeze-out cost): delete the dead branch." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** the "It costs no points, and its SP phrase banks for later." sentence is gone; only the uncounted row's "costs no points, because Hydra's score never counted" sentence remains (src/app/path_view.cpp:128). A new loop test in tests/test_stars.cpp checks `sqout_reduction > 0` for every pad, flag and combo band (commit body of ddaa8416). By reading the commit.
- **Verdict:** GONE
- **Why:** Commit ddaa8416 ("M45: drop the squeeze-out sentence that can never show"). No sentence a player could see was removed.

### concept audit M46: the " · " separator between facts on a line

- **Splits:** One. The question "how is the dot between two facts on a line spaced" is answered by `kFactSeparator = " \xC2\xB7 "` (one space each side, src/app/display_format.h:16) almost everywhere, and by two spaces each side on the batch strip's "Now:" dot (src/ui/library_dialogs.cpp:442, `"  \xC2\xB7  Now: "`), which sits in the same line as a one-space `kFactSeparator` before the artist (library_dialogs.cpp:443). A second odd spelling is the leading bullet at src/ui/paths_tab.cpp:499. V5.md:148 and :151-152 name both and say "needs the user if those two are made to match" and "put them to the user".
- **How it left:** code-only. Closure's words: "M46 (the " · " separator): one `kFactSeparator`; the two odd spellings stay." (questions.md:219).
- **User's words:** None found for the spacing. The paragraph names that the two spellings "stay", and "go with recommended" (fix-decisions.md:551) answered it; but the text does not say what the two spellings are or that they differ in spacing on one line. The code comment at library_dialogs.cpp:440-441 ("this one dot has two spaces each side, like the wide gap before the count, and stays as the strip shows it") is a claim, not a ruling.
- **Code today:** line "Analyzing...  3 of 10  ·  Now: Title · Artist" mixes two- and one-space dots on the batch strip (library_dialogs.cpp:438-443). By reading.
- **Verdict:** REOPEN
- **Why:** The paragraph says the odd spellings "stay" and never tells the user that one line of the strip uses two different dot gaps; the verifier itself said this part needs the user. The effect is cosmetic.
- **Examples:** Row one: the Paths tab summary `"3 acts · 2 left"` uses `kFactSeparator` (one space each side, path_view.cpp:407). Other end: the batch strip's first dot, `"  ·  Now: Song"` (two spaces each side, library_dialogs.cpp:442). Gap: on that same strip line the second dot, `" · Artist"`, is the one-space form (library_dialogs.cpp:443).

### concept audit M51: is this length one the timeline can use

- **Splits:** None reachable: V6.md:104 "Disagree on: none today"; the tab's `len_s > 0.0f` was `has_song_length` re-typed on a float.
- **How it left:** code-only. Closure's words: "M51 (usable length): the tab asks `has_song_length`." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** src/ui/preview_tab.cpp:553 `const float len_s = hydra::app::has_song_length(scrub_len_ms) ...`. By reading.
- **Verdict:** GONE
- **Why:** Commit 5178b821 ("M47, M51: the transport owns the playhead's range; the scrubber ends where playback can reach").

### concept audit M52: does the drawn path have activations

- **Splits:** One, in the loading state. "Can I jump to an activation now": the "< Act" and "Act >" buttons said yes (enabled, because the old scene's scrub marks were non-empty, preview_tab.cpp `no_acts = scrub_marks.empty()` per V6.md:112, :119), while `jump_activation` / `seek_activation` said no (`job_` set, returns false, no effect; V6.md:119). A second row, buttons greyed but jumps working when the playback range is 0 with activations (V6.md:118), V6 could not construct and calls unreachable.
- **How it left:** code-only, with the visible change disclosed in the paragraph. Closure's words: "M52 (has activations): `PreviewController::has_activations()`; note the verifier says the jump buttons would grey during a load where today they are live and do nothing, a visible change it calls a dead click." (questions.md:219). D114 item 22: "That includes M52's visible side effect: the Preview's jump buttons grey during a load, where today they are live and do nothing." (fix-decisions.md:573).
- **User's words:** the paragraph above, with "These proceed on your yes to the fix wave. Veto by id." (questions.md:217), answered "go with recommended" (fix-decisions.md:551). The text names both sides (greyed during a load vs live and do nothing), but the user's reply was to the 21 questions; the paragraph is not one of them and has no Recommended line.
- **Code today:** `bool PreviewController::has_activations() const { return active_ && !job_ && !scene_.activations.empty(); }` (src/ui/preview_controller.cpp:515-517); the tab greys from `!pc->has_activations()` (src/ui/preview_tab.cpp:495); `jump_activation` and `seek_activation` ask the same function (preview_controller.cpp:520, :529). By reading.
- **Verdict:** GONE
- **Why:** The button and the jump now ask one function (commits 161059a3 for `seek_activation`, then F5 merge 06d39ee5).
- **Side picked without a ruling:** the "grey during a load" side won. The paragraph at questions.md:219 and D114 item 22 both disclose it and the user said "go with recommended" to the file, but the line was in a "code-only, veto by id" paragraph, not a question with a Recommended line. The main session decides whether that counts as the user's word; the other option is to leave the buttons live during a load.

### concept audit M53: where each text box sits on the Preview image

- **Splits:** None: V6.md:138 "Disagree on: none today"; the stacking order was written as height sums in the fit pass and as tops in the draw pass.
- **How it left:** code-only. Closure's words: "M53 (box tops): one `overlay_tops`." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573). A separate visible change, one helper and one 10 px inset for every Preview box, was asked later and recorded as D114 item 27 (fix-decisions.md:578).
- **Code today:** `OverlayTops overlay_tops(const OverlayHeights& sizes, bool drain_drawn, bool score_shown);` (src/render/overlay_layout.h:93). By reading.
- **Verdict:** GONE
- **Why:** Commits c31a0d9b and 64d5c63b; the layout side is covered by D114 item 27, which the user was asked.

### concept audit M54: has the chart file changed since its record was analyzed

- **Splits:** The shared rule has one owner (`chart_changed_since`). The tail (the md5 comparison typed twice) had no disagreement. V6.md:156 lists two outcome differences: an unreadable file (Preview says "not changed", the click throws "could not read <path>") and a rewritten file with identical content (Preview not changed, click sets `files_changed` and saves the new sig). They answer different questions, "is the drawn path still valid" (Preview) against "does the library row need its signature refreshed" (click), and the unreadable file ends in an error line on both screens, so they are not two answers to one question.
- **How it left:** code-only. Closure's words: "M54 (chart changed): `ChartNow::differs_from(md5)`." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `ChartNow::differs_from` (src/app/analysis.h:106) called at src/ui/library_jobs.cpp:480 and src/ui/preview_load_job.cpp:179. By reading.
- **Verdict:** GONE
- **Why:** Commit dfbc31ff ("M54: ChartNow::differs_from answers both callers' md5 comparison"). The per-caller outcomes were left as V6 recommended; I judged them different questions, but that rests on V6's "defensible per caller" and my reading, not on a ruling.

### concept audit M55: two ways to assemble a Preview scene and its highway timeline

- **Splits:** None: V6.md:168 "Disagree on: none today"; the three-way assembly is parallel on purpose and only a two-line tail was copied.
- **How it left:** code-only. Closure's words: "M55 (scene assembly): `build_track_state` ends with `rebuild_overlay_fields`." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** src/render/track_state.cpp:326 `rebuild_overlay_fields(st, scene);`. By reading.
- **Verdict:** GONE
- **Why:** Commit fa85948b ("M55: build_track_state adds the overlay through rebuild_overlay_fields").

### concept audit M56: `PreviewNote::solo`, a dead copy of the solo flag

- **Splits:** None: no reader existed (V6.md:177, :180).
- **How it left:** code-only. Closure's words: "M56 (`PreviewNote::solo`): dropped as a dead copy." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `grep "\.solo = |bool solo" src/app/preview_view.*` finds nothing. By grep.
- **Verdict:** GONE
- **Why:** Commit d0ac8e43 ("M56: drop PreviewNote::solo, a dead copy of the solo flag").

### concept audit M57: the order the game lists a drum pad's lane bits

- **Splits:** None: V7.md:12 "Disagree on: none today" (the odd input, a Red note with a cymbal flag, cannot be made).
- **How it left:** code-only. Closure's words: "M57 (lane bits): `lane_bit_rank` read off `lane_bit`." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** src/parse/song.cpp:91-93 `int lane_bit_rank(...)` loops `for (uint32_t bit = lane_bit(note); (bit & 1u) == 0; bit >>= 1) ++rank;` over the note_shuffle.cpp:26 table. By reading.
- **Verdict:** GONE
- **Why:** Commit b8595172 ("M57: read the drum lane order off the game's lane bits").

### concept audit M58: when a drum note is a cymbal

- **Splits:** None: V7.md:25 "Disagree on: none today"; it would split only if a Pro-on Song reached a Pro-off renderer, and no path does.
- **How it left:** code-only. Closure's words: "M58 (cymbal pads): the `.chart` parser asks `allows_cymbals`." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** src/parse/song.cpp:1687-1691 asks `allows_cymbals(*pad)`. By reading.
- **Verdict:** GONE
- **Why:** Commit 3b896f1f ("M58: the .chart cymbal markers ask allows_cymbals"), with a pinning test ad396b1d.

### concept audit M59: what happens to one tick's events when one is malformed

- **Splits:** None: V7.md:35 "Disagree on: none today"; both parsers skip a malformed op and run the rest of the tick.
- **How it left:** code-only, optional. Closure's words: "M59, M62, M63, M65: small helpers (one "run one op" helper, one MIDI-header check, `is_container(ChartFormat)`, one `fits` pair in core)." (questions.md:219). The fix-wave brief made it conditional: "only if it removes more lines than it adds; otherwise leave it and say so" (fix-wave.md:147).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** no commit mentions M59 (`git log --grep "\bM59\b"` is empty). The two `catch (const ChartFileError&)` sites are still at src/parse/song.cpp:1019 (MIDI) and :1746 (.chart). By grep.
- **Verdict:** PASS
- **Why:** The copy is still there and still gives the same answer, so there is nothing the player sees. I did not find where the "leave it and say so" report was written.

### concept audit M62: how Hydra decides a chart's notes format

- **Splits:** None between like questions. V7.md:100 lists "song.mid" in a loose folder (path rule says Mid, name rule says None) and an .sng with only "chart.chart" against an .srb entry named "chart.chart". They ask different questions: "what format is this file I was handed" against "which file in this folder is the chart" against "which stream in this .srb is the notes". V7.md:100 itself says neither path "is a normal library" path. I did not check whether any command takes a bare `song.mid` path.
- **How it left:** code-only, optional. Closure's words: see M59's line (questions.md:219), "one MIDI-header check".
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** commit daf0f45d "M62: one test for a MIDI file's header tag" (src/parse/midi.cpp, merged in bf9145ed). By reading the commit stat only.
- **Verdict:** GONE
- **Why:** The "MThd" magic has one spelling; the three format tests were left apart as V7 recommended.

### concept audit M63: is this path a container (.sng or .srb) or a plain chart file

- **Splits:** None: V7.md:110 "Disagree on: none today".
- **How it left:** code-only. Closure's words: see M59's line (questions.md:219), "`is_container(ChartFormat)`".
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `bool is_container(ChartFormat format)` (src/parse/chart_files.cpp:17, declared chart_files.h:30); `is_container_path` returns `is_container(chart_format_of(notespath))` (src/app/preview_source.cpp:337-338). By reading.
- **Verdict:** GONE
- **Why:** Commit 990b287e ("M63: one list of container chart formats (is_container)").

### concept audit M64: is this entry a playable song stem

- **Splits:** Yes, three, all named in V7.md:121. (a) A ".ogg" entry whose bytes are not Vorbis or Opus: the name test keeps it, the byte test says Unknown, so it is dropped at open. (b) A "preview.wav" inside an .srb: kept, because the byte test is the only test for an .srb entry (src/app/preview_source.cpp:224, `looks_like_audio(stream)`), while a loose folder or .sng drops any "preview.*" by name (preview_source.cpp:149-151 `is_song_stem`). (c) Audio by bytes with no audio extension (".bin") in a folder: dropped by name. In (a) and (c) the stem is silently absent; in (b) a preview clip may be mixed into the song in an .srb but not in a folder.
- **How it left:** code-only. Closure's words: "M64 (stem pre-filter): one table of the five formats; no byte sniff for loose files." (questions.md:219). V7.md:122: "Do not make the folder/.sng path sniff bytes unless the user wants it (a choice between behaviours)."
- **User's words:** None found for the sniff choice. "go with recommended" (fix-decisions.md:551) on a paragraph that says "no byte sniff for loose files" without naming the three outcomes above. Searched fix-decisions.md for "sniff", "stem" and "preview.".
- **Code today:** `kAudioExtensions` table (src/core/audio_sniff.h:54) read by `is_audio_filename` (src/app/preview_source.cpp:143-145); commit 746de0b6. The name-versus-bytes split is unchanged. By reading.
- **Verdict:** REOPEN
- **Why:** The five-format list is now one table, but the item also held a behaviour choice (which stems play) that was decided as "no byte sniff" with no ruling. Whether case (b) is reachable depends on whether real .srb files hold a "preview" stream; I did not check.
- **Examples:** One end: a folder with `guitar.ogg` (real Vorbis) plays. Other end: a folder with `notes.ogg` holding garbage bytes is kept by name and dropped at open, so it is silently missing (preview_source.cpp `is_song_stem` then the decoder's `AudioDecode`). Gap: `preview.ogg` in a folder is excluded by the name test; a `preview.wav` stream inside an .srb passes `looks_like_audio` and is mixed in.

### concept audit M65: do n bytes fit at this position in the buffer

- **Splits:** None: V7.md:132 "Disagree on: none on the yes/no for any input a real file can give"; each caller's "no" policy is deliberately its own.
- **How it left:** code-only, optional. Closure's words: see M59's line (questions.md:219), "one `fits` pair in core".
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `constexpr bool fits(uint64_t size, uint64_t pos, uint64_t n)` in src/core/bounds.h:20; `using core::fits;` (src/parse/sng.cpp:16); `core::fits(meta, pos, ...)` (src/parse/srb.cpp:146). By reading.
- **Verdict:** GONE
- **Why:** Commit 40c409e6 ("M65: one bounds test for the binary readers (core::fits)").

### concept audit M75: where a path gets cut at its last separator

- **Splits:** None reachable: V8.md:193 "a path that ends in a separator (`base_name` returns empty, the other two trim first). No caller passes one."
- **How it left:** code-only. Closure's words: "M75 (path cuts): `trim_trailing_separators` in `core/winstr`." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `std::string trim_trailing_separators(std::string path)` (src/core/winstr.cpp:127), used by `parent_folder` (winstr.cpp:133) and `relpath` (src/app/analysis.cpp:51). `base_name` keeps its own cut at src/app/preview_source.cpp:34, as V8 allowed. By reading.
- **Verdict:** GONE
- **Why:** Commit 332a5e3e ("M75: one trailing-separator trim and one separator test in core/winstr").

### concept audit M76: the jobs' "seconds since start", the time-left clock and the 3-second gate

- **Splits:** V9.md:35 names one: the batch strip's time left comes from the mean time per finished chart with a 3-chart gate (`batch_eta_s`), the Preview load and the startup copy from the last second's byte rate with a 3-second gate (`ByteRateClock`). V9 says "The user sees "about 1:30 left" from both, from two different rules, by design and documented." The user ruled the startup copy follows the Preview's rule (D-record: "A time-left line, by the Song Preview's rule ... It reuses the Preview's rate clock (`ByteRateClock`), its words (`detail::time_left_text`) and its 3 s wait", fix-decisions.md:443). I read the two estimators as answering different questions (charts of unknown length against a known byte total), but no ruling covers the batch strip's rule against the Preview's.
- **How it left:** code-only, optional. Closure's words: "M76 (job clocks): optional `elapsed_since`." (questions.md:219); the fix-wave brief says "Optional; skip if it adds more than it removes" (fix-wave.md:251).
- **User's words:** None found for the batch strip's rule against the Preview's rule. Searched fix-decisions.md and the derivation audit (R7.1 at derivation-audit.md:6383 raised the two wordings; its wording half is now one function, `app::time_left_text`, used at src/ui/library_dialogs.cpp:4).
- **Code today:** `grep elapsed_since src/` finds nothing; `batch_eta_s` is still in src/ui/library_jobs.cpp. By grep.
- **Verdict:** PASS
- **Why:** The optional tidy was not done and the two expressions still agree. The two time-left rules are different quantities (finished charts against bytes moved), and both now show the same words; if you want the "by design" claim itself ruled on, that is a separate question for the user, not part of M76's code copy.

### concept audit M78: what Cancel does to a report build, and what the slot says afterwards

- **Splits:** One screen difference V9 calls on purpose: after Cancel the DM comparison window closes and reopens the player list at once, while the path report stays open with "Report cancelled." and Try again after the current chart finishes (V9.md:91-93). The final slot state is the same ("Final slot state is the same"). The DM side is D103 item 10: "**Cancel while fetching closes the window and reopens the player list,** as Cancel in the box goes back to the list today." (fix-decisions.md:462). I found no D-record for the path report's Cancel screen; D103's mock boards 1 to 5 are cited as the approved design (fix-decisions.md:452) and I did not open the mock.
- **How it left:** code-only. Closure's words: "M78 (Cancel on a report): optional `ReportSlot::mark_cancelled()`." (questions.md:219).
- **User's words:** D103 item 10 for the DM side (above); "go with recommended" (fix-decisions.md:551) for the tidy.
- **Code today:** `void mark_cancelled()` in src/ui/app_state.h:230, called at app_state.cpp:741, :781, :854, :945. By reading.
- **Verdict:** GONE
- **Why:** Commit 9208bcaf ("M78: ReportSlot::mark_cancelled does Cancel's two slot writes") removed the copied writes. The screen difference stays and rests on D103 item 10 for one side only; the two windows have different steps before them (the DM window has a picker to go back to), but that is my reading.

### concept audit M79: how "this job was cancelled" is thrown

- **Splits:** None: V9.md:118 "Disagree on: none today for the kind; the four raw texts differ but are never shown."
- **How it left:** code-only for the throw. Closure's words: "M79 (cancelled throw): one `Cancelled` error type in core." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `class Cancelled : public KindedError` (src/core/error_kind.h:150); the one hand-built `KindedError(ErrorKind::Cancelled, what)` is inside it (error_kind.h:153). By grep.
- **Verdict:** GONE
- **Why:** Commit ef2247e9 ("M79: one Cancelled error type in core builds every stop").

### concept audit M80: may a batch (or analysis) start now

- **Splits:** None reachable: V9.md:145 "Disagree on: nothing reachable". The zero tests ("matched rows" on the toolbar, "to-do rows" in the confirm) answer different questions, and the count differs only when a library reload failed with a status message.
- **How it left:** code-only. Closure's words: "M80 (may a batch start): `AppState::can_analyze()`." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `bool AppState::can_analyze() const { return !analysis_blocked() && !batch_running(); }` (src/ui/app_state.cpp:568), called at library_toolbar.cpp:97, library_dialogs.cpp:386, app_state.cpp:696. By reading.
- **Verdict:** GONE
- **Why:** Commit fe86522d ("M81, M84, M80: busy tooltip words, modal ids, can_analyze"); the confirm's Start now also asks "no batch running", which V9 showed cannot happen while it is open.

### concept audit M84: how a modal is named, and who opens it

- **Splits:** None: V9.md "Disagree on: none today".
- **How it left:** code-only. Closure's words: "M84 (modal names): two named ids in `library_parts.h`." (questions.md:219).
- **User's words:** "go with recommended" (fix-decisions.md:551); D114 item 22 (fix-decisions.md:573).
- **Code today:** `inline constexpr const char* kScanModalId = "Scanning charts";` (src/ui/library_parts.h:87) and the folders id beside it; strings unchanged. By reading.
- **Verdict:** GONE
- **Why:** Commit fe86522d.

## Not reached

Not covered because they are in the other scout's list or were not in the paragraph's code-only set: M16, M32, M34, M36, M37, M40, M48, M49, M67, M68, M70, M71. Every id in my list was reached. Two checks I did only partly: M26 (I did not confirm that tools/bench.cpp uses `load_scan_cache`, only that it passes a `cache` variable to `discover_charts`), and M62 (whether any command takes a bare `song.mid` path).

## Side notes

- **M33 part B (not mine, still open by reading):** V4.md:77-80 shows `after_sp_end` (tick test) and `paid_by_sp_walk` (ms test) disagree on a hand-made `.chart` with a tempo like `B 100000000000000000`; V4 says "needs the user" to merge them. The paragraph says "part B untouched", and I found no question that put the two answers to the user. A hand-made chart is the only way in.
- **M25 sibling:** `src/cli/batch.cpp` loads settings with `Settings::load_for_command_line(legacy_fills)` and `fillcompare` with `Settings::load()` (V3.md:108, "on purpose"); I did not re-read either today.
- **M54:** `ChartNow`'s header comment "an empty md5 means the file is unreadable, which is no change" was to be fixed per V6.md:158; src/app/analysis.h:116 now reads about `differs_from`, which I take as done but did not read in full.
- **R7.1 / M76 area:** `kEtaMinFinished = 3` charts (batch) and the Preview's 3 s gate are two thresholds for the same "when do we first show time left"; the "by design" claim in V9.md:35 has no ruling I could find.
- **V5 side note, still true by reading:** CONTEXT.md's Hit window entry says "Normal below 2 ms ... Hard below half a hit window" while the code is inclusive (V5.md:160); I did not recheck.

<!-- next -->
