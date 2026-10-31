# AOA-W3d: concept audit verifier side notes

Code today = main 76ffb2a8 in `.claude/worktrees/aoa-w3`. Paths below are relative to it. "V1.md:330" means `docs/audit/2026-10-11-concept-audit/findings/V1.md` line 330, and so on.

### V1-SN1: the path report's Timing column sorts on the tier's name

- **Splits:** "In what order do the timing tiers go?": the Timing dropdown lists them in ladder order Normal, Hard, Extreme, Insane, Insane+, Beyond, None (src/app/path_report_view.cpp:55-57, from `timing_tiers()` at src/core/squeeze_rating.cpp:187-191) vs the Timing column header sorts them as text, which gives Beyond, Extreme, Hard, Insane, Insane+, None, Normal (src/app/path_report_view.cpp:108, `c.sort_key = [](const ReportRow& r) { return SortKey{r.tier}; };`).
- **How it left:** nothing at all. V1.md:330 says "by reading it looks like a bug or an unstated choice".
- **User's words:** D103 item 16 "Text columns sort the way the Library table does" (docs/audit/2026-10-03-fix-decisions.md:470) and item 23 "Song, Charter, Mode and Timing showed no change" (:477). Neither says whether the Timing column goes by name or by ladder; item 23 only says the Timing order did not move between the old page and the window. Searched the decisions file for "Timing" (seven hits, none about sort order).
- **Code today:** unchanged, by reading (src/app/path_report_view.cpp:108).
- **Verdict:** REOPEN
- **Why:** The user sees the order when clicking the header, and nobody asked whether "Normal" should come first or last. A name sort puts the ladder out of order (Extreme before Hard).
- **Examples:** Click Timing once (A to Z). Name order: Beyond (more than twice the hit window), Extreme (up to 85 ms), Hard (up to 42.5 ms), Insane, Insane+, None, Normal. Ladder order would be Normal (up to 2 ms), Hard, Extreme, Insane, Insane+, Beyond, None. The ends differ (Beyond first vs Normal first) and so does the middle (Extreme before Hard). The Hardest ms column already sorts by number, so ladder order is reachable through it today.

### V1-SN2: an off-speed score on a chart the library does not list is clickable

- **Splits:** "Does clicking a comparison row for a chart not in the library do nothing?": row status "not in library" is unclickable (src/ui/dm_report_window.cpp:107-109, `return r.status != app::dm_report::kStatusNotInLibrary;`) vs the same missing chart with an off-speed score gets status "other speed" because the last assignment wins (src/app/dm_report.cpp:145 sets not-in-library, :147 `if (!base) row.status = kStatusOtherSpeed;`), so it is clickable and calls `select_chart` (src/ui/dm_report_window.cpp:155), which may switch the Settings bar's chart mode (src/ui/app_state.cpp:627-630) and then finds no copy and selects nothing (:639-650).
- **How it left:** nothing at all.
- **User's words:** D103 item 12: "A \"not in library\" row's click does nothing. Its hover text says \"Not in your library\"." (docs/audit/2026-10-03-fix-decisions.md:466). D25 gave off-speed scores their own status (:61) and D103's later note keeps "+N over" on them (:275); neither says what a click on a missing chart does.
- **Code today:** as above, by reading. Whether the mode switch can happen depends on the window's mode differing from the bar's at click time (src/ui/app_state.cpp:618); not run.
- **Verdict:** REOPEN
- **Why:** D103 item 12 names one status; the off-speed status hides the same missing chart behind another word, so the ruling does not reach it.
- **Examples:** A score at normal speed on a chart not in the library: status "not in library", click does nothing, hover reads "Not in your library". The same chart with a score at 120% speed: status "other speed", click runs `select_chart`, hover has no "Not in your library" text. If the window's mode is not the bar's, that click also flips the Settings bar to the row's mode and then selects nothing. The fix changes what the user sees (hover text and click), so it is the user's call.

### V1-SN3: the HTML pages' sort returns 1 for two missing values

- **Splits:** None: a comparator that is not a strict order, with no input where the visible result is wrong.
- **How it left:** nothing at all. V1.md:332 says "Harmless in practice (both sink)".
- **User's words:** None needed; D103 item 17 says "An empty Posted date sinks to the bottom in both directions" (docs/audit/2026-10-03-fix-decisions.md:471), which the code meets.
- **Code today:** src/app/html_page.cpp:358-359 `if (x === null || x === undefined) return 1; if (y === null || y === undefined) return -1;` so two nulls return 1 in either order. By reading.
- **Verdict:** REOPEN
- **Why:** The comparator is still inconsistent; only the order among rows that are all missing is at stake.
- **Code-only fix:** return 0 when both are missing (check `y` first only when `x` is not missing).

### V1-SN4: the "if (!*open)" branch in both report windows is never reached

- **Splits:** None: dead code against a header that promises a different call pattern.
- **How it left:** nothing at all.
- **User's words:** D271 item 7 "Both report windows keep their rows when closed (Q13). The user chose \"Both keep\"." (docs/audit/2026-10-03-fix-decisions.md:1073). It covers keeping rows; it does not mention this branch.
- **Code today:** src/ui/library_view.cpp:206-209 calls each draw function only `if (app.path_report.window_open)` / `if (app.dm_report.window_open)`, while src/ui/report_window.h:107-109 says "Call every frame, open or not." The branch at src/ui/dm_report_window.cpp:120-126 and src/ui/path_report_window.cpp:195-201 (`if (!*open) { w.memory.was_open = false; ... drop_rows(w); return; }`) can only run on a call with `*open` false, which the caller never makes. By reading.
- **Verdict:** REOPEN
- **Why:** The comment inside says a replaced result "lets it go now, not at the reopen", but that never happens, so the old result stays held until the window reopens.
- **Code-only fix:** either call both draw functions every frame as the header says, or delete the branch and reword the header. No stored record or display changes.

### V1-SN5: "in both" wording for a chart where neither side kept a path

- **Splits:** None today.
- **How it left:** a question the user answered. D114 item 2: "The fill page's tile and subtitle for \"in both\" read \"In both, not both scored\"." (docs/audit/2026-10-03-fix-decisions.md:555).
- **User's words:** Q2 answered "go with recommended" (D114, docs/audit/2026-10-03-fix-decisions.md:553 per the wave brief).
- **Code today:** src/app/fill_report.h:40-46 "A record on each side but not a score on each: one side scored, or neither kept a path." and `kInBothPhrase = "in both, not both scored"`; used for the tile (src/app/fill_report.cpp:197) and subtitle (:422). By reading.
- **Verdict:** GONE
- **Why:** Commit 35273613 (2026-10-11, "M02 fill half") reworded it to cover both cases.

### V1-SN6: the report's "twice the hit window" and "past the N ms window" prose

- **Splits:** None: two sentences that restate the Beyond rule, and both give today's answer.
- **How it left:** nothing at all. V1.md:335 says "Agrees today".
- **User's words:** D270 call 8 "One hit window number everywhere, fixed at 85" (docs/audit/2026-10-03-fix-decisions.md:1062) and D278 item 4 "Say \"the 85 ms hit window\"" (:1148).
- **Code today:** src/app/path_report_view.cpp:106-107 builds "...hit window. Beyond means more than twice the hit window." from `report::hit_window_text()`; src/app/report.cpp:350-351 writes the edge from `beyond_edge_text()`. The Beyond edge is `nominal_budget_ms()` = `kHitWindowMs * (1 + kIdentityScale)` (src/core/squeeze_rating.cpp:61-62, 76-82, 189), and `kIdentityScale` is a constant 1.0 (src/core/squeeze_rating.h:72). So the edge is exactly twice the one fixed hit window. By reading.
- **Verdict:** PASS
- **Why:** The hit window is now one fixed constant and the scale is a constexpr, so no setting can move the edge away from "twice"; the sentence cannot disagree with the code without a code edit.

### V1-SN7: theme.cpp comment on the HTML tier tokens

- **Splits:** None: a stale comment.
- **How it left:** nothing at all.
- **Code today:** src/ui/theme.cpp:22-24 now reads "The tiers are html_page.h's kChipRgb* colours, which the fill page's dark scheme also reads for its --t0, --t3 and --tn", which matches src/app/html_page.cpp:139-140 (`--t0: __CHIP_T0__; --t3: __CHIP_T3__; --tn: __CHIP_TN__;`). By reading.
- **Verdict:** GONE
- **Why:** Commit fa2c16d8 ("M07 colours: the chip tier colours live once, read by chip_color and the fill page") rewrote the comment.

### V1-SN8: `RecordStore::has_record` has no production caller

- **Splits:** None: dead code.
- **How it left:** nothing at all (V1.md:337 "I did not re-verify").
- **Code today:** `grep has_record` over src, tools and tests finds nothing. By reading.
- **Verdict:** GONE
- **Why:** Commit e10f83b9 (2026-10-15, "Delete the old store API now every caller uses records") removed it. `AppState::save_view_summary` is also rewritten (src/ui/app_state.cpp:539-545) and spells no `get_summary` lookup.

### V2-SN1: the test cache key leaves Note Shuffle out

- **Splits:** None: a test helper's key.
- **How it left:** nothing at all.
- **Code today:** tests/corpus_util.h:179 and :245 now key on `hydra::app::chartmode_key(reading)`, and `chartmode_key` appends ", Note Shuffle" when `reading.noteshuffle` is set (src/app/config.cpp:341). By reading.
- **Verdict:** GONE
- **Why:** Commit 81bc953c (2026-10-11, "Test cache keys: spell a ChartReading with app::chartmode_key") fixed it.

### V2-SN2: a chart-mode text the Settings cannot parse passes the Note Shuffle test as "off"

- **Splits:** None: a gate that failed open.
- **How it left:** nothing at all.
- **Code today:** `why_not_comparable_profile` calls `Settings{}.with_known_chartmode(profile.chartmode)` (src/app/dm_report.cpp:31), which throws when no choice spells the text (src/app/config.h:281-283). By reading.
- **Verdict:** GONE
- **Why:** Commits e6140776 (2026-10-11, M17) and 79fcbb9e (2026-10-14) replaced the `mode &&` guard; a bad text now throws instead of reading as "off".

### V2-SN3: dm_report.h says the toolbar and `collect_dm_rows` ask the same question

- **Splits:** None: a comment.
- **How it left:** nothing at all.
- **Code today:** `collect_dm_rows` calls `why_not_comparable_profile(profile)` (src/app/dm_report.cpp:83), which checks difficulty, cap, fill rule and Note Shuffle (:53-69); the header says the toolbar's overload asks "what collect_dm_rows will ask" and that "the cap setting never refuses" (src/app/dm_report.h:87-89). Both sides ask the same function. By reading.
- **Verdict:** GONE
- **Why:** The comment and the code agree now (M17 in e6140776; D271 item 8 for the cap).

### V2-SN4: commit_settings is called "the single place that knows which settings change a record's identity"

- **Splits:** None: a comment that restates a rule instead of naming its owner.
- **How it left:** nothing at all.
- **User's words:** D103 item 22 says each report decides for itself whether a settings change reaches it: "The rule has one owner beside the two report builders." (docs/audit/2026-10-03-fix-decisions.md:476).
- **Code today:** src/ui/app_state.h:742-743 still says "This is the single place that knows which settings change a record's identity". The deciding is in `Settings::profile()` compared at src/ui/app_state.cpp:1125, and each report's own `settings_change_touches` at :1135-1136. By reading.
- **Verdict:** REOPEN
- **Why:** The comment names `commit_settings` as the knower, but it only compares `profile()` and asks the two reports; the rule lives in those.
- **Code-only fix:** reword the comment to point at `Settings::profile()` and the reports' `settings_change_touches`.

### V2-SN5: `RecordStore::has_record` has no caller in src/

- **Splits:** None. Same item as V1-SN8.
- **Verdict:** GONE
- **Why:** Removed by commit e10f83b9 (2026-10-15); no `has_record` remains in src, tools or tests.

### V3-SN1: `BatchProgress::current_title` has no reader

- **Splits:** None: a dead field. The live `Snapshot::current_title` (src/ui/library_jobs.h:276) is a different struct with the same name.
- **How it left:** nothing at all.
- **Code today:** `current_title` on `BatchProgress` is declared at src/app/analysis.h:225 and written at src/app/analysis.cpp:902 (`progress.current_title = wr.item.title;`). Every other hit for the name in src, tools and tests belongs to `Snapshot` (src/ui/library_dialogs.cpp:439-441, src/ui/library_jobs.cpp:357, 370) or its tests. By reading (grep over src, tools, tests).
- **Verdict:** REOPEN
- **Why:** Nothing reads the field; it is still written on every chart.
- **Code-only fix:** delete the field and the write at analysis.cpp:902.

### V3-SN2: `save_view_summary` tests `!found.empty()`, and `has_record` has no caller

- **Splits:** None.
- **Code today:** `AppState::save_view_summary` now saves the prepared record and returns `prepared.record.summary()` (src/ui/app_state.cpp:539-545); no `found.empty()` test. `has_record` no longer exists anywhere (see V1-SN8). By reading.
- **Verdict:** GONE
- **Why:** The save path was rewritten around records (store lookup at src/ui/app_state.cpp:549), and commit e10f83b9 (2026-10-15) deleted `has_record`.

### V3-SN3: `list_chart_library(offset, limit)` called with "no limit", and its ORDER BY name

- **Splits:** None.
- **Code today:** production calls pass `(0, -1)` with the comment "-1 = no limit" (src/ui/app_state.cpp:253, 912; src/ui/library_jobs.cpp:43). The function still sorts `ORDER BY name LIMIT ? OFFSET ?` (src/store/record_store.cpp:1548). Tests call it with `(0, 10)` (tests/test_analysis.cpp:376, tests/test_store_open.cpp:1068). The `charts_by_name` index exists for that sort (src/store/record_store.cpp:817-818). By reading.
- **Verdict:** PASS
- **Why:** Every caller gets the same rows in the same order as before; the paging arguments have test users and the sort is served by an index. A tidy-up at most, no wrong answer.

### V3-SN4: `tick_library` re-reads summaries when a chart fails

- **Splits:** None: a wasted read.
- **Code today:** the trigger is `snap.completed != batch_seen_completed_` (src/ui/app_state.cpp:282), and `completed` is "analyzed + failed" (src/app/analysis.h:220). By reading.
- **Verdict:** PASS
- **Why:** A failed chart causes one extra summary read at most once per refresh interval; the rows shown are the same.

### V3-SN5: the `charts` table has no UNIQUE on `path`

- **Splits:** None found. Not a split, a missing guard.
- **Code today:** the table is created without UNIQUE (src/store/record_store.cpp:799-811). The walk keeps a `visited` set so a nested folder is listed once (src/app/analysis.cpp:513-520, 557), and the Song folders dialog refuses an exact repeat ("That folder is already in the list.", src/ui/library_dialogs.cpp:203-206). The walk's comment says a root can repeat (src/app/analysis.cpp:511 "repeats included"), so a hand-edited settings file with the same folder twice could list its charts twice. I did not check a different spelling of one folder (case or slash), and I did not run a scan.
- **Verdict:** PASS
- **Why:** The two ways a user adds a folder both stop the usual duplicate; the hand-edit case is a narrow gap I could not confirm by reading.

### V4-SN1: `ReplayNote::dynamics_bonus` has no reader in src/

- **Splits:** None.
- **Code today:** the field is filled at src/core/replay.cpp:155 and read only by the replay tool's JSON (tools/replay.cpp:411) and tests (tests/test_replay.cpp, tests/test_stars.cpp, tests/replay_util.h:57). By reading (grep over all *.cpp and *.h).
- **Verdict:** PASS
- **Why:** It is the replay tool's per-note output, not a second answer to any question the app shows. The scout's claim that the Preview reads it was wrong, as V4.md:183 says.

### V4-SN2: engine comment names `add_deact_edge` for the offset rule

- **Splits:** None: a comment that is one name off.
- **Code today:** src/search/engine.cpp:1983-1984 says "the same offset rule add_deact_edge uses". The offset rule is `offset_from_sp_end` (src/core/model.h:114), reached through `edge_row_offset` (src/search/graph.h:185-186) and called directly at src/search/engine.cpp:1987. By reading.
- **Verdict:** REOPEN
- **Why:** Still the same stale wording.
- **Code-only fix:** name `offset_from_sp_end` in the comment.

### V4-SN3: replay.h says `score_of` / `assign_score` are the only spelling of the Path-to-ReplayScore pairing

- **Splits:** None.
- **Code today:** src/core/replay.h:243-246 now reads "the two are the only pairing between Path and ReplayScore. Other code copies the six between other shapes (the engine's copy-out, a variant taking its leader's scores)". By reading.
- **Verdict:** GONE
- **Why:** Commit 0d6fa00e (2026-10-11, "M30: the engine's running score uses score_total") narrowed the sentence to what is true.

### V4-SN4: `EdgeView v;` in `enumerate` has no initialisers

- **Splits:** None: a hazard for a future edit.
- **Code today:** src/search/engine.cpp:174 `EdgeView v;`, with `struct EdgeView` at :75-84 holding ten plain fields. Every field is assigned at :175-194 today. By reading.
- **Verdict:** REOPEN
- **Why:** The compiler would not flag a field added later and forgotten here.
- **Code-only fix:** write `EdgeView v{};` so a forgotten field reads as 0.

### V4-SN5: `check_timing_maps` does not check that ms still tell neighbouring ticks apart

- **Splits:** None today; this is the M33 part B edge.
- **How it left:** a question dropped. D273 item 2: "Q16 is dropped, for the reason Q5 and Q6 were (D271 item 1). The tick test (`after_sp_end`) and the millisecond test (`paid_by_sp_walk`) can only disagree at a tempo near 1e14 BPM, which no real chart has and a `.mid` cannot reach. The user chose \"Drop it (Recommended)\". Both stay as the code has them." (docs/audit/2026-10-03-fix-decisions.md:1090).
- **User's words:** the quote above; D271 item 1 has the user's "is there any real chart where this actually matters or are you just inventing situations to waste my time" (:1067).
- **Code today:** src/core/song.cpp:19-48 refuses a tempo at or below 0 or not finite, and does not refuse an absurdly high finite one. By reading.
- **Verdict:** PASS
- **Why:** The side note is the same absurd-tempo edge the user dropped; the ruling names the tempo (about 1e14 BPM) and says it is unreachable on a real chart. The ruling is about the two tests agreeing, not about refusing such a chart, so a "refuse it at parse" idea would still be new, but nothing real is behind it.

### V5-SN1: the "costs no points" branch for a counted row is dead

- **Splits:** None.
- **Code today:** src/app/path_view.cpp:121-130 now splits on `core::counted_without_squeeze(off, leeway_ms)`: a counted row says "It scores N fewer points", an uncounted row says "It costs no points, because Hydra's score never counted that note under Star Power". The comment at :117-120 says a counted row's cost is never 0. So the "costs no points" text is the uncounted case, which is reachable. By reading.
- **Verdict:** GONE
- **Why:** The branch was reworked so each sentence matches one case; I did not pin the commit.

### V5-SN2: CONTEXT.md says Normal "below 2 ms", Hard "below half a hit window"

- **Splits:** None.
- **Code today:** CONTEXT.md:674-677 now reads "Normal up to 2 ms (`kDifficultMs`), then Hard up to half a hit window ... each edge belongs to the band below it". It matches `timing_tiers` and `tier_for` (src/core/squeeze_rating.cpp:187-212, `*ms <= *t.cutoff`). By reading.
- **Verdict:** GONE
- **Why:** The doc now says "up to" and states the inclusive edge.

### V5-SN4: `Engine::act_within_limit` loops over the squeezes beside `Activation::within_ms_limit`

- **Splits:** None: a code copy that gives the same answer.
- **Code today:** the engine loop (src/search/engine.cpp:1184-1188) and the model loop (src/core/model.cpp:528-531) both call `SPSqueeze::within_limit` per squeeze and `fill_within_limit(e_offset, skips, limit)` for the fill, so the leaf rules are shared and the two loops agree on every input. By reading.
- **Verdict:** PASS
- **Why:** Same question, same two leaf functions; the only extra thing the engine adds is the pinned-fill exemption at :1182, which the model walk does not have, and that is about which activations are judged, not how.

### V7-SN1: `prev_tick` starts at 0 in Note Shuffle, so a first note at tick 0 reads as "same chord"

- **Splits:** None: Hydra and Clone Hero's decoded code agree.
- **How it left:** already checked. Session card 2026-10-11-8eb32145 (claim 4): "It holds, and it matches Clone Hero." with "The capped reference at `docs/audit/note-shuffle/probes/capped/shuffle_ref_capped.py:70` says both start at 0." (that report text was not kept; card `docs/sessions/2026-10-11-8eb32145.md`:20 says only "Claim 4 held and needed no fix"). W2a (docs/audit/2026-10-29-audit-of-audits/blocks/AOA-W2a.md:148) marked it REOPEN for lack of a live run, without reading that card's answer.
- **Code today:** src/parse/note_shuffle.cpp:140 `int64_t prev_tick = 0;`, :146 `const bool same_chord = nt.tick == prev_tick;`. The reference file's line 70 reads "prev_tick = 0      # r13 / [rsp+0x58], both start at 0" and its line 81 `if is_drums and nt.tick == prev_tick:`. By reading.
- **Verdict:** PASS
- **Why:** Hydra follows the game's decoded code at this point, which is the source of truth for Note Shuffle (D105: "go with the game's rule" per the memory note). The card's extra argument (the game froze on a chart that puts four notes on tick 0) I did not re-check.

### V7-SN2 and V8-SN5: audio headers say `std::runtime_error` where the code throws `KindedError`

- Not mine: the brief gives these to another scout. Card 2026-10-11-8eb32145 records them as fixed ("the comments now say KindedError and name its kind").

### V8-SN3: the rules file has a lower bound per key and no upper bound

- **Splits:** None between two parts; a range question nobody put to the user.
- **How it left:** nothing at all for the upper end. The lower end is ruled: D270 call 3 "The five `hydra_rules.ini` floors stay (Q3). After each key and its floor were explained: \"yes those floors are fine\"." (docs/audit/2026-10-03-fix-decisions.md:1057). D114 item 8 (Q8) covers `nan`, `inf` and hex: "The rules file reads numbers through `parse_finite_number`; `nan`, `inf` and hex are reported with the rules file's usual error" (:561).
- **Code today:** src/core/rules.cpp:76-81 gives each numeric key only a floor (`backend_leeway_ms` 0.0, `max_tied_paths` 1.0, `fill_cooldown_measures` 1.0, `fill_max_distance_beats` 0.0, `fill_land_slop_beats` 0.0). src/app/rules_file.cpp:43-45 reads numbers through `parse_finite_number`, so a huge finite value such as 1e300 passes. By reading; I did not run it.
- **Verdict:** REOPEN
- **Why:** The floors were ruled, but no one was asked whether a huge finite value should refuse. The rules file's own D270 call 4 says "refuse anything bad".
- **Examples:** `backend_leeway_ms=3` is the default; `backend_leeway_ms=0` is the floor and reads as written; `backend_leeway_ms=1e300` loads, and every note past the SP end would fall inside the leeway (I read the key's name and floor, not the code that uses it). The gap is any value above a sensible ceiling, which today has no edge to land on (D270 call 2 sends out-of-range values to the nearest edge, and there is no top edge). Only a hand-edited file reaches it. A fix would make Hydra refuse a file it loads today, so it is the user's call.

### V9-SN1: `ScanJob::run` has no try/catch around `discover_charts`

- **Splits:** None: a missing guard. The other jobs catch (`run_guarded`, src/ui/library_jobs.cpp:123, 476, 551), and `ScanJob::run` does not.
- **How it left:** nothing at all (V9.md:253 "I did not check whether `discover_charts` can throw").
- **Code today:** `ScanJob::start` runs `spawn([this] { run(); })` (src/ui/library_jobs.cpp:164), and `JobBase::spawn` is `thread_ = std::thread(std::move(fn))` with no catch (src/app/job_base.h:46). `ScanJob::run` calls `app::load_scan_cache`, `app::discover_charts` and `app::save_scan_as_library` with no try/catch (src/ui/library_jobs.cpp:171-216). `discover_charts` catches per-file and per-folder errors (src/app/analysis.cpp:544-548, 688-689), but an exception from anywhere else in it (an allocation failure, say) would end the process. I did not find a throw that a real folder reaches.
- **Verdict:** REOPEN
- **Why:** Rule 4 says fail loudly; the scan thread is the one job that cannot report its own failure.
- **Code-only fix:** run the body through `run_guarded` like the other jobs. The user would see something different only in a case that now ends the process.

### V9-SN3: `kRulesFile` is pinned by tests but has no GUI caller

- **Splits:** None.
- **How it left:** a question the user answered. D114 item 11: "The command-line tools print \"hydra_rules.ini has an error:\" and then the loader's line. The dead `kRulesFile` text is deleted, and the app's analysis-off sentence moves into `user_messages.cpp`." (docs/audit/2026-10-03-fix-decisions.md:564).
- **Code today:** no `kRulesFile` remains in src, tools or tests; the file name lives in `kRulesFileName` (src/app/rules_file.h:21), used by src/app/rules_file.cpp:105, 112. By reading (grep over all *.cpp and *.h).
- **Verdict:** GONE
- **Why:** The dead text was deleted as D114 item 11 says.

## Not reached

I hit the 100-call wrap-up before these. A fresh scout can take them in this order.

- V5-SN3 (V5.md:161): the derivation audit's finding 2 table says "-85 -> Insane"; today -85 is Extreme. A doc in `docs/audit/` about an older audit, so likely "history, not wrong"; not read.
- V6-SN1 to V6-SN5 (V6.md:187-191): the drag-to-right-end test name (tests/test_preview_transport.cpp), the `ChartNow` comment on an empty md5 (src/app/analysis.h:125 against src/ui/library_jobs.cpp:506), the "carrying a timecode" comment (src/app/preview_view.h:449), `note_in_span` used only by a test (src/render/track_state.cpp:404), and `rules` held by four objects.
- V7-SN3 (V7.md:154): the preview_view.cpp comment about a mid-measure meter change "restarts the measure" against `MeasureIndex`.
- V7-SN4 (V7.md:155): `Song::check_activations` calling `timing().plusmeasure` and its own tpm walk side by side (finding 53, "recorded as kept"; W2a's Not reached list says V7's finding 53 is covered by C1's M61 block).
- V8-SN1, V8-SN2 (V8.md:201-202): `MaReader::skip` marking an error only when `got != 0`, and `CountedLength::seek` leaving `pos` stale after a failed restart.
- V8-SN4 (V8.md:204): `write_report_file` always uses `<path>.tmp`.
- V9-SN2 (V9.md:254): the "stopped" read from the cancel flag after the batch finished (M77 edge).
- V9-SN4, V9-SN5 (V9.md:256-257): the `ByteRateClock` forward declaration; `start_batch` over an empty plan (V9.md itself says only tests reach it).
- Handled elsewhere, per the brief: the audio headers' "throws" wording (V7-SN2, V8-SN5), which card 2026-10-11-8eb32145 records as fixed. The `Playhead::read_frames` note is not a numbered item in the nine side-note sections; it is in side-notes.md, and another scout has it.

## Side notes

- W2a (docs/audit/2026-10-29-audit-of-audits/blocks/AOA-W2a.md:148) marked the `prev_tick` side note REOPEN as "needs a live game run", but session card 2026-10-11-8eb32145 had already checked it against the game's decoded code (reference py line 70) and found it matching. See V7-SN1.
- V1-SN2 may also be a count question: an off-speed score on a missing chart is counted under "other speed" (src/app/dm_report.cpp:147) and not under "not in library", so the "Not in your library" tile may leave those rows out. I did not read `count_status`.

