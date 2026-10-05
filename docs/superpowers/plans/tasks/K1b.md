Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task K1b: the leaderboard and fill-comparison pages read from the owners

**Orchestrator answers (2026-10-04, binding; they override the open questions below).** Q1 (finding 312, a record on both sides but a score on one) and Q2 (the fill comparison's empty-page sentence): both PENDING the user's answer. Leave those two behaviours exactly as they are today and do everything else; a small follow-up applies the answers. Q3: yes, you own `src/app/fill_report.h`. Finding 309 (the "Under optimal" / "At optimal" / "Above optimal" chips, D48 Q29): you also own `src/app/dm_report.h` and the test files that pin the old "Matched" chip (`tests/test_s2_offspeed.cpp` and the dm uitest file), for that change only; no other task touches them. Q4: shared test files by case, as written. Q5: the page's average tile may take the mean of the numeric percentages on the page (the filter decides which rows count, so it must run in the page); that one rounding of the mean counts as the one rounding.

Base: the `claude/p3-d1` join commit (hash 47eafff (claude/p3-d1)). Branch: `claude/p3-k1b`. Worktree `.claude\worktrees\p3-k1b`.

## Goal

The leaderboard comparison page (`src/app/dm_report.cpp`) and the fill-spawn comparison page (`src/app/fill_report.cpp`, hydra_fillcompare) name their chips, percentages, fill rules and titles by hand. This task points them at the owners wave D1 built and fixes the two labels D48 Q29 decided. Nothing here changes a score or a stored record.

## What changes, finding by finding

**309, the "Matched" chip (D48 Q29).** Today `collect_dm_rows` names every joined score at or under optimal "matched", so a score 50,000 under optimal wears a Matched chip. Decision: the chips read "Under optimal", "At optimal" and "Above optimal", and the summary line matches. The status values in `collect_dm_rows`, the `<option>` list, `STATUS_CLASS`, the stats tiles, the Status column's help text, `tally_dm_rows` (its matched count splits into under and at), and `counts_phrase` all follow. "Not analyzed", "Not in library" and "Other speed" are unchanged.

**51, the percent rounded twice (Q30).** Today `collect_dm_rows` writes `pct` with four decimals and the page rounds again with `toFixed(2)`. Decision: round to nearest once. Owner: `format_percent` (O2, `src/app/display_format.h`). The row's percent cell is the owner's string, carried in the payload; the "Avg % of opt" tile is the one number the page still computes, from the full values, rounded once there. On screen: 198,010 of 198,020 reads 99.99%, not 100.00%.

**312, "Only in 1.1 db" for a chart with no score (Q29).** Today `collect_fill_rows` decides the status by score, so a chart whose 1.0 record has no paths falls through to "only 1.1". Decision: rows are labelled by which database holds a record. `tally_fill_rows` loses its catch-all else. What a chart with a record on both sides but a score on one reads is under Open questions.

**55, the fill rule's name (Q13).** Today "CH 1.0" and "CH 1.1" are typed in the fill page's title, heading and column names, and the footer's two sentences about each rule are prose typed here. Owner: `fill_rule_name` (short form) and `fill_rule_description` (O3a, `src/search/graph.h`). The option values "only 1.0" and "only 1.1" and the subtitle's "under 1.1" are version numbers, not rule names, and stay.

**99, commas (Q12).** Today the two page scripts call `toLocaleString()` bare in their stats tiles and delta cells. Every one goes through the shared `fmt` that K1a fixes in `html_page.cpp`. `report::counted` (two calls in `dm_report.cpp`) becomes the core `counted` (O1, `src/core/model.h`); K1a deletes the report copy.

**8, titles with tags (Q14).** Today both collectors clean names with `report::plain`, which knows only `<color>`. Owner: `display_title` (O3a, `src/parse/song.h`) for the song, which also gives "(unknown)". Artist and charter use the same stripping without the fallback; if O3a landed no fallback-free form, use `strip_rich_tags` (`src/app/library_query.h`) and say so. Every `report::plain` call in your three files goes, and the `fill_report.h` comment that names it.

**105, the empty comparison (Q28, the fillcompare half).** Today `src/cli/fillcompare.cpp` prints "No records to compare" whenever the row count is zero. The reason should come from `generate_fill_report`, as K1a does for the path report. The wording when records exist under other settings is under Open questions.

## Owned files

`src/app/dm_report.cpp`, `src/app/fill_report.cpp`, `src/app/fill_report.h`, `src/cli/fillcompare.cpp`, `tests/test_dm_report.cpp`, `tests/test_fill_report.cpp`, `tests/test_report.cpp` (only the case "comparison page explains its columns and splits the missing scores"), `tests/test_cli.cpp` (only the two hydra_fillcompare cases). The plan's table does not list `fill_report.h`; see Open questions.

## Tests

- "collect_dm_rows joins scores to records and labels them" — re-pin: a score under optimal is "under optimal", equal is "at optimal", over is "above optimal"; no row is "matched".
- "collect_dm_rows: no pct off 100% speed; store identity fallback" — unchanged, run it.
- "collect_dm_rows: a percent rounds once" (new) — 198,010 of 198,020 reads "99.99%"; the payload carries that string.
- "generate_dm_report: tally and framing behind one seam" — the counts phrase names under and at optimal.
- "comparison page explains its columns and splits the missing scores" — the option list and `STATUS_CLASS` carry the three new values and no "matched".
- "collect_fill_rows: a chart in one database only still gets a row" — unchanged.
- "collect_fill_rows: an old record with no score and no new record" (new) — the row is "only 1.0". The old record is a Ready record with no paths, so its summary has no score; H1's stored batch-result fixture writes exactly that record, so store it in the 1.0 database.
- "tally_fill_rows counts every status" — every status is counted by name; no else branch.
- "build_fill_html substitutes every placeholder" — the title and column names come from the short rule name.
- "collect_fill_rows: a blank stored song name reads (unknown)" and "collect_dm_rows: a blank stored song name reads (unknown)" — extend each with H1's tags-only title and a `<b>` title reading clean.
- "hydra_fillcompare compares a 1.0 and a 1.1 database" and "hydra_fillcompare compares both rules out of one database" — re-pin any changed text.

Run: `-sf=*test_dm_report.cpp`, `-sf=*test_fill_report.cpp`, `-tc="comparison page explains*"`, `-tc="hydra_fillcompare*"`. uitest script: `dm-compare-flow`.

## Scan rows

None. K1a's row for the page locale covers your files once the join deletes its two known-copy entries; O3a's fill-name row and O3a's tag row cover the names and titles.

## Done when

No "Matched" chip on a score under optimal. A one-sided chart is labelled by its record. 198,010 of 198,020 prints 99.99%. No `report::plain`, `report::counted` or bare `toLocaleString()` remains in your files.

## Open questions

1. Finding 312: when both databases hold a record but only one has a score (a Ready record with no paths), D48 names no label. Options: a fourth status such as "no score in 1.0", or treating the no-score side as absent. Ask before choosing.
2. Finding 105 for hydra_fillcompare: Q28's sentence names one cap and one fill rule, but this page compares two rules. Proposed: "Nothing is analyzed under these settings (SP cap 8, Expert Pro Drums) in either database." Ask before choosing; keep today's sentence until then.
3. `src/app/fill_report.h` must change (the reason on `GeneratedFillReport`, the stale comment) and the plan's K1b row does not list it.
4. `tests/test_report.cpp` and `tests/test_cli.cpp` are shared with K1a (different cases). The join merges them.
5. The payload's `pct` keeps a numeric form for the page's average tile beside the owner's string; confirm that the average rounding once on the page counts as "one rounding", or move the average into C++ too.
