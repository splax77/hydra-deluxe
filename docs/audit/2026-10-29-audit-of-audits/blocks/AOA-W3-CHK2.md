# AOA-W3-CHK2: closure checks for AOA-W3d and AOA-W3e

Paths are relative to the aoa-w3 worktree (main 76ffb2a8) unless absolute.

### V7-SN1
- **Scout said:** PASS
- **Checked:** card `docs/sessions/2026-10-11-8eb32145.md` (line 20 says claim 4 held; the report text it came from was not kept); `docs/audit/note-shuffle/probes/capped/shuffle_ref_capped.py` (lines 1-5, 70, 81); the raw listing `docs/audit/note-shuffle/step0/open34-refute/shuffle_20f0c30.asm` (lines 40, 154, 207-210, 271-272, 318-319); `docs/clone-hero/02-note-list.md` NL-47; `src/parse/note_shuffle.cpp:140-152`.
- **Verdict:** PASS
- **Why:** The card alone is not enough: its evidence is the Python reference, whose own header says "Not verified against the running game", and its third argument is wrong (the D1 freeze comes from a zero seed, NL-47, and does not depend on `prev_tick`). But the claim does rest on the game's real code. The disassembly of the game's shuffle function zeroes r13 at 0x20f0cf3 (`xor r13d, r13d`), stores it to the prev-tick slot at 0x20f0eb7 (`mov [rsp+0x58], r13`), and the first-pass same-chord test at 0x20f0fa4 (`cmp r13, [r14+0x40]`, guarded by the drums flag r12b) runs before r13 is reloaded with a note tick at 0x20f1159. So the game's prev tick starts at 0, and Hydra's `int64_t prev_tick = 0` with `nt.tick == prev_tick` does the same. A live run would only be a second witness, not a missing one. Small side point: NL-47 cites `step0/open34/` for the listing; a copy also sits in `open34-refute/`.

## Blocks from AOA-W3d.md

### V1-SN5
- **Scout said:** GONE
- **Checked:** `git show --stat 35273613` (exists, on main); `src/app/fill_report.h:40-46`, `src/app/fill_report.cpp:197-201, 422`; decisions file line 555 (D114 item 2) and `docs/audit/2026-10-11-concept-audit/questions.md:37`.
- **Verdict:** GONE
- **Why:** The tile and subtitle now read "in both, not both scored" from one constant (`kInBothPhrase`), and the header says the status covers "one side scored, or neither kept a path". That removes the old "a score on one side only" wording that was wrong for a chart where neither side kept a path. The question's Recommended line quoted the reword and the user said "go with recommended".

### V1-SN6
- **Scout said:** PASS
- **Checked:** `src/app/path_report_view.cpp:106-107`, `src/core/squeeze_rating.cpp:76-82, 182-193`, `src/core/model.h:104`, `src/core/squeeze_rating.h:72`, decisions file lines 1062 and 1148.
- **Verdict:** PASS
- **Why:** The sentence says "more than twice the hit window". The Beyond edge is the Insane+ cutoff, `nominal_budget_ms()` = 85 x (1 + 1.0) = 170 ms, with 85 a fixed constant and the scale a constexpr 1.0. Both sentence and edge answer "how far is twice the window", and D270 call 8 and D278 item 4 (quoted correctly) fix the 85.

### V1-SN7
- **Scout said:** GONE
- **Checked:** `git show --stat fa2c16d8`; `src/ui/theme.cpp:22-24`; `src/app/html_page.cpp:125-141, 276-284, 305-307`.
- **Verdict:** GONE
- **Why:** The comment now says the tiers are `kChipRgb*` and that the fill page's dark scheme reads them for --t0, --t3 and --tn. That matches the dark block (`__CHIP_T0__`, `__CHIP_T3__`, `__CHIP_TN__`); the light scheme has its own literals, which the comment does not claim.

### V1-SN8
- **Scout said:** GONE
- **Checked:** `git show e10f83b9` (pickaxe on `has_record` in `src/store/record_store.h` names it); grep of `has_record` over all .cpp/.h in the worktree finds nothing.
- **Verdict:** GONE
- **Why:** The unused function is gone in e10f83b9 (2026-10-15).

### V2-SN1
- **Scout said:** GONE
- **Checked:** `git show --stat 81bc953c`; `tests/corpus_util.h:179, 245`; `src/app/config.cpp:333-342`.
- **Verdict:** GONE
- **Why:** The test cache key now uses `chartmode_key(reading)`, which appends ", Note Shuffle" when the setting is on.

### V2-SN2
- **Scout said:** GONE
- **Checked:** `src/app/dm_report.cpp:26-34`; `src/app/config.cpp:398-403`; pickaxe on `mode &&` in `dm_report.cpp` names e6140776.
- **Verdict:** GONE
- **Why:** `why_not_comparable_profile` calls `with_known_chartmode`, which throws for an unspellable mode text instead of treating it as "off".

### V2-SN3
- **Scout said:** GONE
- **Checked:** `src/app/dm_report.cpp:53-84`; `src/app/dm_report.h:80-90`.
- **Verdict:** GONE
- **Why:** The toolbar overload and `collect_dm_rows` both go through `why_not_comparable_profile`, so the comment is true. Commit e6140776 is the one the pickaxe ties to this code.

### V2-SN5
- **Scout said:** GONE
- **Checked:** same as V1-SN8.
- **Verdict:** GONE
- **Why:** Same item as V1-SN8, removed by e10f83b9.

### V3-SN2
- **Scout said:** GONE
- **Checked:** `src/ui/app_state.cpp:539-552`; `has_record` grep (nothing).
- **Verdict:** GONE
- **Why:** `save_view_summary` now saves the prepared record and returns its summary; the hand-spelled lookup and `has_record` are gone (e10f83b9).

### V3-SN3
- **Scout said:** PASS
- **Checked:** callers of `list_chart_library` (`grep` over src, tests, tools); `src/store/record_store.cpp:817-818`.
- **Verdict:** PASS
- **Why:** Library callers all pass (0, -1) and the sort is served by the `charts_by_name` index; the paging arguments have a test user (`tests/test_analysis.cpp:376` passes 10). It is an unneeded argument, not a place two parts answer one question differently, and no screen or record changes.

### V3-SN4
- **Scout said:** PASS
- **Checked:** `src/ui/app_state.cpp:276-290`; `src/app/analysis.h:220`.
- **Verdict:** PASS
- **Why:** `completed` counts analyzed plus failed, so a failed chart triggers one extra summary read, rate-limited by the refresh interval. The rows shown are the same either way.

### V3-SN5
- **Scout said:** PASS
- **Checked:** `src/app/analysis.cpp:506-520, 596-614`; `src/store/record_store.cpp:799-811, 1477-1490`; `src/ui/library_dialogs.cpp:199-212`; `src/app/config.cpp:207`.
- **Verdict:** REOPEN
- **Why:** The scout's own text admits a repeated root "could list its charts twice" and calls it a gap it could not confirm. Reading confirms it: a folder listed twice becomes two root nodes, both are popped in the serial walk (`for idx : roots`), and `save_scan_as_library` inserts every item with no dedupe, so the library would show each chart twice. Reaching it takes a hand-edited `chartfolder` line (the dialog refuses an exact repeat but compares text exactly, so case or slash variants of one folder pass it). A fix (dedupe roots in `discover_charts`) would change what the user sees only in that case: duplicate rows disappear. No score or stored record for normal libraries changes.

### V4-SN1
- **Scout said:** PASS
- **Checked:** grep of `dynamics_bonus` over src, tools, tests; V4.md lines 45-53 and 183.
- **Verdict:** PASS
- **Why:** The field is filled for the replay tool's per-note JSON and pinned by tests; no screen reads it. It does not answer a question twice, and the user's visible Accent/Ghost totals come from the category scores.

### V4-SN3
- **Scout said:** GONE
- **Checked:** `git show --stat 0d6fa00e -- src/core/replay.h`; `src/core/replay.h:240-247`.
- **Verdict:** GONE
- **Why:** The header now says the two functions are the only Path-to-ReplayScore pairing and lists the other copies by name, so the sentence is true.

### V4-SN5
- **Scout said:** PASS
- **Checked:** decisions file lines 1067 and 1090 (D271 item 1, D273 item 2); V4.md line 187; `src/core/song.cpp:19-48`.
- **Verdict:** PASS
- **Why:** D273 item 2 drops Q16 for a tempo near 1e14 BPM "which no real chart has", and D271 item 1 sets the rule: not asked again "unless a real chart shows one". The side note is the same absurd-tempo edge (V4.md says only a hand-made .chart reaches it). A refuse-at-parse change would be new, but nothing real is behind it, and the user's stated bar is a real chart.

### V5-SN1
- **Scout said:** GONE
- **Checked:** `git show ddaa8416` (M45, 2026-10-11); `src/app/path_view.cpp:116-130`.
- **Verdict:** GONE
- **Why:** The scout did not pin the commit; it is ddaa8416, which removed the dead "It costs no points" branch for a counted row and kept the uncounted sentence. The text a player can see did not change, because the removed branch never ran.

### V5-SN2
- **Scout said:** GONE
- **Checked:** `git show 250b47ce -- CONTEXT.md` (diff replaces "below 2 ms" and "below half" with "up to"); `CONTEXT.md:674-678`; `src/core/squeeze_rating.cpp:182-213`.
- **Verdict:** GONE
- **Why:** CONTEXT.md now says "up to" with the edge belonging to the band below, which matches `tier_for` (`*ms <= *t.cutoff`). Commit 250b47ce (2026-10-31) is on main.

### V5-SN4
- **Scout said:** PASS
- **Checked:** `src/search/engine.cpp:1174-1197`; `src/core/model.cpp:528-532`.
- **Verdict:** PASS
- **Why:** Both loops ask the same two leaf functions (`SPSqueeze::within_limit` and `fill_within_limit`); the engine's walk only adds the pinned-fill exemption, which is about which activation is judged. Same answer on every input the model walk judges.

### V7-SN1
- (written above, first block of this file)
- **Verdict:** PASS

### V9-SN3
- **Scout said:** GONE
- **Checked:** grep of `kRulesFile` word over all .cpp/.h (nothing); `src/app/rules_file.h:21`; pickaxe names f36c044a ("M72: one home for what a bad hydra_rules.ini reads as", 2026-10-11); decisions file line 564 (D114 item 11).
- **Verdict:** GONE
- **Why:** The dead text is deleted, as D114 item 11 says. The scout did not name the commit; it is f36c044a.

## Blocks from AOA-W3e.md

### V5-SN3
- **Scout said:** PASS
- **Checked:** `git show de332fbc^:src/app/report.cpp` lines 228-237 (`*ms < *t.cutoff`); `docs/audit/2026-10-03-derivation-audit.md:70-74`; `src/core/squeeze_rating.cpp:206-212`; decisions file line 121.
- **Verdict:** PASS
- **Why:** At the audit's date the edge test was `<`, so -85 was Insane. D48 Q3 later moved the edge ("A timing on a tier edge counts as inside") and today 85 is Extreme. The audit file is a dated record, and the two sides are different dates of one rule, not two live answers.

### V6-SN1
- **Scout said:** GONE
- **Checked:** `git show --stat d0d91262 -- tests/test_preview_transport.cpp`; `tests/test_preview_transport.cpp:290-303`.
- **Verdict:** GONE
- **Why:** The test is now named for a song length past the audio's end and sets 9000 ms against a 5000 ms playhead, so it does reach the case the note said was missing.

### V6-SN2
- **Scout said:** GONE
- **Checked:** `git show --stat dfbc31ff -- src/app/analysis.h`; `src/app/analysis.h:110-121`; `src/ui/library_jobs.cpp:486-489`.
- **Verdict:** GONE
- **Why:** The comment now says the Preview treats an empty md5 as no change while the click's ViewJob throws "could not read <path>", which is what the code at library_jobs.cpp:488-489 does.

### V6-SN5
- **Scout said:** PASS
- **Checked:** `src/ui/preview_controller.cpp:88-100`; `src/ui/app_state.cpp:30-42`; grep for writers of `rules` over src.
- **Verdict:** PASS
- **Why:** `settings.rules` is read once at start-up (`load_startup_settings`) and the only other write is a copy in `config.cpp:413`, so nothing can change the rules while the Preview is open. The key lacking a rules member answers nothing today; it would on the day a live rules reload exists.

### V7-SN3
- **Scout said:** GONE
- **Checked:** pickaxe on "restarts the measure" in `src/app/preview_view.cpp` names 801db20c; `src/app/preview_view.cpp:551-555`; `src/core/timing.h:112-126`.
- **Verdict:** GONE
- **Why:** The old comment was removed in 801db20c, and the time signature now follows the grid, whose spans do restart at the first beat on or after a change.

### V7-SN4
- **Scout said:** GONE
- **Checked:** `git show 5677f8d3 -- src/parse/song.cpp` (deletes `tpm_pos`, `pre_tpm`, the plusmeasure lines); `git show 6a98db3b`; grep of `tpm_pos|pre_tpm|bpm_pos` over src and tools (nothing); `src/core/song.cpp:100-135`; AOA-D2.md line 20.
- **Verdict:** GONE
- **Why:** The own meter walk is gone and `place_fills_ch10` reads the one `MeasureIndex`. On "Side picked without a ruling": true for the two-meter-changes case. The user did say, on 2026-10-24, to move the 1.0 generator onto MeasureIndex and merge after seeing which charts moved, but his words do not name that case. AOA-D2 already carries that question, so it is not a new leak here.

### V8-SN2
- **Scout said:** PASS
- **Checked:** `src/audio/stem_reader.h:174-195`; the scout's readers of `pos_` in `src/audio/ma_reader.cpp`.
- **Verdict:** PASS
- **Why:** After a failed restart the code sets `at_end` and leaves `pos` stale. While `at_end` is true every read returns 0 frames and the next seek restarts and resets `pos`, so the stale value is never read. No sound or count depends on it.

### V9-SN2
- **Scout said:** PASS
- **Checked:** `docs/audit/2026-10-11-concept-audit/questions.md:199-203`; decisions file lines 553 and 574; `src/ui/library_dialogs.cpp:487-490`; `src/ui/app_state.cpp:755`.
- **Verdict:** PASS
- **Why:** Q21's own text describes the batch edge with both outcomes ("a Stop pressed on the frame the last chart finishes marks the run Stopped and builds no path report, though every chart ran") and says the edge "stays" under the recommendation the user took. Caution: the listed options are about the two landing orders, and the batch edge appears only in the explanation and the case-against, so this is a thin rather than a strong cover.

### V9-SN4
- **Scout said:** GONE
- **Checked:** pickaxe on `ByteRateClock` in `src/ui/library_jobs.h` names 59b77bc7; grep over src shows it only in `preview_load_job.h/.cpp`.
- **Verdict:** GONE
- **Why:** The forward declaration and clock member left `library_jobs.h` in 59b77bc7 (2026-10-14).

### V9-SN5
- **Scout said:** PASS
- **Checked:** `src/ui/app_state.cpp:722-740, 912-940`; `src/ui/library_dialogs.cpp:380-402`; grep of `start_batch(` callers.
- **Verdict:** PASS
- **Why:** The only two callers are the confirm dialog (button enabled only when `to_run > 0`) and the report-window path (returns before `start_batch` when `todo_rows() == 0`). A direct call over an empty plan is reachable only from tests.

## Not reached

None. All 30 PASS and GONE blocks across the two files were checked.

## Side notes

- V7-SN1: the card is not enough alone. Its Python reference says "Not verified against the running game", and its argument that the D1 freeze proves tick 0 counts as same chord is wrong (that freeze is the zero seed, NL-47). The raw game disassembly in `docs/audit/note-shuffle/step0/open34-refute/shuffle_20f0c30.asm` is what carries the claim. W2a's REOPEN for "needs a live run" could be lifted on that basis, but a card citing only the reference should not be relied on again.
- NL-47 in `docs/clone-hero/02-note-list.md` cites the listing under `step0/open34/`; a copy also exists under `step0/open34-refute/`. Both files exist, so nothing is broken.
- Scouts W3d for V5-SN1 and V9-SN3 named no commit; the commits I found are ddaa8416 and f36c044a.

