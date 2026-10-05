Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task RP: reports (findings 86, 202, 170, 88 pages half, 54 CLI callers, 22 and 23 column words, 140 report default)

Task id: RP. Base: main after M7-2 and M6-J2 (the main session names the hash at launch). Branch: claude/p7-rp (worktree `.claude\worktrees\p7-rp`, made as the preamble says).

Plan row: `docs/superpowers/plans/2026-10-04-phase-7.md`, wave 3 table, "RP Reports". Decisions in `docs/audit/2026-10-03-fix-decisions.md`: D51's code-only calls for 86, 202 and 170 (the sheet's last section), D51 calls 5, 11 and 15, D58 item 3, and ST2's owners from D51 calls 11 and the 54 code-only call. Finding texts: `docs/audit/2026-10-03-derivation-audit.md`, headings `#### 86.`, `#### 202.`, `#### 170.`, `#### 88.`, `#### 54.`, `#### 22.`, `#### 23.` and `#### 140.`.

## Goal

Four questions get one owner each in the report code: when does a path report mean every path, what is each page's file name, which Clone Hero rules let the library be compared with dmleaderboards, and how do the two comparison pages treat a Ready record that kept no paths. The two console tools also stop reading the fill-rule stamp their own way and ask ST2's store function. The Hardest column's help text says "hardest squeeze or required early fill" (D51 call 5), and the report's hit-window default stops cutting a decimal to a whole number (D51 call 15). No score, path or stored record changes; the results stamp stays "2.1.0".

## What the code does today

Line numbers are from `claude/p6-j2-2` for `report.cpp`, `dm_report.cpp` and `fill_report.cpp` (phase 6's J2-2 rewrote parts of them; M6-J2 brings that text) and from main for every other file; the merged base moves them a little.

**86.** `report.h` holds two constants: `kEveryPathSentinel` (line 106, one billion, what `--all-paths` asks for in `src/cli/report.cpp` line 49) and `kEveryPathLabelThreshold` (line 108, one hundred million). `generate_report` (report.cpp line 420) prints "every path" whenever `max_paths` is above the threshold, so `hydra_report --paths 200000000` reads "every path" while listing at most that many.

**202.** `report_html_path` (`report_files.cpp` line 125) spells `hydra_paths.html`, and `dm_report_html_path` (line 127) spells `hydra_dmcompare.html`. `src/cli/report.cpp` line 42 spells `hydra_paths.html` again as its default `--out`, resolved against the current folder. `tests/ui/uitest_harness.cpp` lines 293 and 294 spell both again (LB's file).

**170.** `collect_dm_rows` (dm_report.cpp line 181) forces `kCloneHeroSpCap`, but the page types "SP cap 4" as text three times: the Hydra opt column help (line 83), the Status column help (line 91) and the footer (line 332). The three Clone Hero rules (Expert, the 4-bar cap, 1.1 fills) and their three tooltip sentences live only in `render_actions_row` (`src/ui/library_toolbar.cpp` lines 99 to 123, LB's file); `collect_dm_rows` passes the chart mode and lens through unchecked.

**88, pages half.** `collect_dm_rows` (lines 226 to 235) tests `rec->summary.score` and calls a Ready record with no paths "not analyzed". `collect_fill_rows` (fill_report.cpp lines 196 to 223) reads each side's `summary.score`, and the page script's `scoreText` (lines 78 to 80) writes "no score" for the empty side of an "in both" row. ST2 added the one answer, `PathSummary::has_scored_best_path()` (`record_store.h` line 84 at `claude/p7-w2b`), which M7-2 brings.

**54, CLI callers.** `src/cli/report.cpp` lines 85 to 87 read `engine_mode()` and compare `fill_rule_from_stamp` with Ch10 to force the 1.0 lens. `src/cli/fillcompare.cpp` lines 87 to 94 (`warn_if_not`) read the stamp and warn when it names the other side's rule, and say nothing for an unstamped file. ST2 added `RecordStore::stamped_fill_rule()` (record_store.h line 453 at `claude/p7-w2b`): Ch10 or Ch11 from the stamp, Ch11 for an unstamped file with results, nothing otherwise. `src/cli/batch.cpp` lines 157 to 161 still spell the unstamped rule themselves; that file is LB's (see Not in this task).

**22 and 23.** The 'ms' column's help (report.cpp line 106) reads "The hardest squeeze or early fill the path needs, in raw ms. A dash means it needs none." `row.ms` is `summarize_path`'s `hardest_ms`, which E3 made empty for a path with nothing to time (D58 item 3), so the dash already follows; only the words change here.

**140.** `ReportOptions::hit_window_ms` (report.h line 115) is an `int` set from `static_cast<int>(kDefaultHitWindowMs)`; `generate_report` (report.cpp line 386) casts it back to a double. `src/cli/report.cpp` line 77 copies `settings.hit_window_ms`, an `int` at main that LB turns into a double this wave.

## What changes

**86.** `kEveryPathLabelThreshold` is deleted. `generate_report` prints "every path" exactly when `max_paths` is at or above `kEveryPathSentinel`; the sentinel's comment names it as the owner of that question. `--paths 200000000` then reads "top 200,000,000 paths per chart and mode" (D51's code-only call; the thousands come from `hydra::counted`, which the subtitle already uses).

**202.** `report_files.h` gains one file-name constant per page, and `report_html_path` and `dm_report_html_path` build their paths from them. `src/cli/report.cpp` takes its default `--out` from the path report's constant; its default folder stays the current folder (D51). The harness literals are LB's to repoint.

**170.** `dm_report.h`/`.cpp` gain one owner of the gate, `why_not_comparable(Difficulty, int sp_cap, bool legacy_fills)`, returning an empty string when the comparison is allowed and otherwise the toolbar's sentence for the first rule that fails, moved word for word from `library_toolbar.cpp` lines 115 to 123 (the cap sentence formats `kCloneHeroSpCap` as the toolbar does). `collect_dm_rows` asks it for the one rule its inputs carry: a lens with 1.0 fills is refused with `std::invalid_argument` carrying that sentence; the cap stays forced to `kCloneHeroSpCap`; the comment says the difficulty rule reaches the same owner through the caller's settings (open question 3). The page's three "SP cap 4" texts are formatted from `kCloneHeroSpCap`: a `__SP_CAP__` placeholder filled in `page_template` the way `__BASE_SPEED__` is, and the footer built with the number. LB's toolbar calls `why_not_comparable` in wave 3.

**88.** `collect_dm_rows` reads `has_scored_best_path()`. A Ready record with none gets the status "no paths" (D51 call 11): a new status literal with its own dropdown option and a chip class, a `no_paths` count in `DmReportStats` that `tally_dm_rows` fills, and a ", N with no paths" clause in `counts_phrase` written only when N is above 0, so every existing phrase is unchanged (open question 1). `collect_fill_rows` reads the flag for each side, and the page's empty-side cell reads "no paths" instead of "no score" (open question 2). The "in both" status and D52's counting are untouched.

**54.** `src/cli/report.cpp` forces the 1.0 lens when `stamped_fill_rule()` is Ch10. `fillcompare.cpp`'s `warn_if_not` compares `stamped_fill_rule()` with the side's expected rule, so an unstamped file with results now warns when passed as `--old` (open question 4). Neither tool reads `engine_mode()` any more.

**22 and 23.** The 'ms' column help reads exactly: "The hardest squeeze or required early fill the path needs, in raw ms. A dash means it needs none." The guide's words are DOC's.

**140.** `ReportOptions::hit_window_ms` becomes a `double` defaulting to `kDefaultHitWindowMs`; `generate_report`'s cast goes. `src/cli/report.cpp` assigns the setting as it is, which compiles before and after LB's change.

## Owned files (only these may change)

- `src/app/report.h`, `src/app/report.cpp`, `src/app/report_files.h`, `src/app/report_files.cpp`
- `src/app/dm_report.h`, `src/app/dm_report.cpp`, `src/app/fill_report.h`, `src/app/fill_report.cpp`
- `src/cli/report.cpp`, `src/cli/fillcompare.cpp`
- `tests/test_report.cpp`, `tests/test_dm_report.cpp`, `tests/test_fill_report.cpp`, `tests/test_cli.cpp` (only its `hydra_report*` and `hydra_fillcompare*` cases)
- `tests/test_single_owner.cpp`: your own scan rows at the end of the file only.

Notes on the base. `report.cpp`, `dm_report.cpp`, `fill_report.cpp`, `test_report.cpp`, `test_dm_report.cpp`, `test_fill_report.cpp`, `tests/dm_fixture.h` and `tests/corpus_util.h` differ between main and `claude/p6-j2-2`; the base holds J2-2's text, and the line numbers above are J2-2's. `report.h`, `report_files.*`, `dm_report.h`, `fill_report.h`, `src/cli/report.cpp` and `src/cli/fillcompare.cpp` are main's text on every ref checked. `has_scored_best_path` and `stamped_fill_rule` exist only on `claude/p7-w2b` until M7-2 lands; build against the base, never against a wave branch. `src/cli/batch.cpp`, `library_toolbar.cpp`, `dm_jobs.cpp`, `app_state.cpp`, `library_jobs.h` and `uitest_harness.cpp` are not yours.

## Test cases to add or re-pin

Use the files' own helpers: `fill_store` and `store_tied` in test_report.cpp, `testdm::fill_store` and `make_score` from `tests/dm_fixture.h`, `put_ch10`/`put_ch11`/`compare` in test_fill_report.cpp, `test::store_batch_result` from `tests/display_fixtures.h`, `CliSandbox` and `run_exe` in test_cli.cpp. No new helper.

1. `generate_report: "every path" is the sentinel's, a huge --paths stays a count (86)` (new, test_report.cpp): the sentinel gives "every path"; 200,000,000 gives "top 200,000,000 paths per chart and mode"; `ReportOptions{}.hit_window_ms == kDefaultHitWindowMs` (140). Red line: "every path" at 200,000,000 today.
2. Re-pin `reports_dir is Documents\Hydra, made on first use`: the two paths end in the two constants. Add a scan row: `hydra_paths.html` and `hydra_dmcompare.html` each on one code line under `src/`, with the harness's two lines as known copies until LB repoints them.
3. `collect_dm_rows: a Ready record with no paths reads "no paths" (D51 call 11)` (new, test_dm_report.cpp): `fill_store`, then `store_batch_result` under a second hash the library lists (as `collect_dm_rows tells not analyzed from not in library` builds it). Pin: status "no paths", no `optimal`, `no_paths == 1`, `not_analyzed == 0`, `counts_phrase` ends ", 1 with no paths", the page has the option. Red line: "not analyzed" today.
4. `why_not_comparable names the missing Clone Hero rule (170)` (new): Expert, cap `kCloneHeroSpCap`, 1.1 gives ""; Hard gives "Needs Expert: the leaderboard only has Expert scores."; cap 8 gives the cap sentence; 1.0 gives the fills sentence; `collect_dm_rows` with a 1.0 lens throws `std::invalid_argument`. Red line: the function does not exist.
5. Re-pin `build_dm_html substitutes every placeholder`: no `__SP_CAP__` remains and the page holds "at SP cap " followed by the constant's digits. Add a scan row: the text "SP cap 4" appears on no code line under `src/`.
6. Re-pin `collect_fill_rows: a record on both sides with a score on one is in both`: the page holds `'no paths'` and not `'no score'`. `collect_fill_rows: an old record with no score and no new record` passes unchanged.
7. Re-pin `path report explains and renames its columns`: the 'ms' column line holds the exact sentence above. Red line: "or early fill the path needs".
8. `hydra_fillcompare warns when an unstamped database with results is passed as --old` (new, test_cli.cpp): build the unstamped file the way `hydra_fillcompare compares both rules out of one database` does, never calling `set_engine_mode`; pin that the output holds "Warning" and the file's name, and the exit code is 0. Red line: no warning today. Keep `hydra_report reports a --legacy-fills database under the 1.0 rule` and both existing `hydra_fillcompare*` cases passing unchanged; their "is stamped engine_mode=" pin stays.

Existing cases that must pass unchanged: `generate_report: one seam frames the page for every entry point`, `report payload carries the hit window and the tier table`, `report lists only the wanted cap and names it`, `collect_dm_rows joins scores to records and labels them`, `generate_dm_report: tally and framing behind one seam`, `generate_fill_report: a score on one side only is counted, and the parts add up`.

## Test filters you may run

- `build-cpp\Release\hydra_tests.exe -sf=*test_report*`, `-sf=*test_dm_report*`, `-sf=*test_fill_report*`
- `build-cpp\Release\hydra_tests.exe -tc="hydra_report*"`, `-tc="hydra_fillcompare*"` (build `hydra_batch`, `hydra_report` and `hydra_fillcompare` warm first)
- `build-cpp\Release\hydra_tests.exe -tc="single-owner*"`

Nothing else. Never the full suite.

## Stored results

None change, and the stamp stays "2.1.0". What a person can see change: the 'ms' column help words; the new "no paths" status, option and cell; a counts clause that appears only when a no-paths row exists; the hydra_report subtitle for `--paths` between one hundred million and one billion; and hydra_fillcompare's warning on an unstamped file. "SP cap 4" reads the same at the real cap.

## Not in this task

- `src/cli/batch.cpp` lines 157 to 161 (the unstamped-file rule), the toolbar's call to `why_not_comparable` and its tooltips, the harness's file-name literals, `ReportJob`'s and `Settings`' hit-window type: LB, which forks after your commit.
- The guide's and CONTEXT.md's words for 22 and 23: DOC.
- `fill_compare.html`, spelled once in `fillcompare.cpp`; `dm_jobs.cpp` and `app_state.cpp`.

## Done when

- `kEveryPathLabelThreshold` is gone and "every path" reads the sentinel; both page names are typed once under `src/`; no "SP cap 4" text remains in `dm_report.cpp`; `why_not_comparable` exists with the three sentences; both comparison pages read `has_scored_best_path`; neither CLI reads `engine_mode()`; `ReportOptions::hit_window_ms` is a double.
- The eight cases above pass with their red lines recorded; the named existing cases pass unchanged.
- `git diff --stat <base>..HEAD` lists only the owned files.

## Open questions (each with a recommended answer)

**Decided (D62):** the user took the recommended answer to every question below. Treat each recommendation as the decision.

1. **The "no paths" status's extra words (needs the user: display).** D51 call 11 gives only "no paths". Recommended: the dropdown option reads "No paths (analyzed, none kept)", the Status column help gains "No paths: analyzed, but the analysis kept no path.", the chip takes the not-analyzed colour, no new tile, and the counts clause appears only when the count is above 0. Nobody has seen a real analysis produce such a record.
2. **The fill page's empty-side cell (needs the user: display).** D50 item 2 chose "no score"; D51 call 11, later, says the fill page says "no paths". Recommended: "no paths", since it names the cause; D52's "with a score on one side only" counting words stay.
3. **The Expert rule inside `collect_dm_rows` (code-only).** It receives a chart-mode string, not a difficulty, and reading the difficulty back out of that string would copy `chartmode_key`'s format. Recommended: `collect_dm_rows` enforces the fills rule and the cap, the toolbar asks the same owner for all three, and the comment says so; passing the settings down to `generate_dm_report` would touch `dm_jobs.cpp` and `app_state.cpp`, which no wave 3 task owns, so the main session decides whether a later wave does that.
4. **The unstamped-file warning (code-only, dev-tool text).** Recommended: one plain sentence saying the file has no stamp and holds 1.1 results, so it is not the 1.0 side; the stamped-file sentence keeps its words and its test pin.
5. **The file-name constants' type (code-only).** Recommended: UTF-8 `const char*` constants in `report_files.h`, converted once in `report_files.cpp`, so the CLI's `std::string` default reads them as they are.

## Commits

One commit for the pages and one for the two CLIs if you prefer, trailers `Task: RP` plus the preamble's others. Report as the preamble says, with the exact warning sentence you chose for question 4.
