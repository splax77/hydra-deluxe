Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task K1a: the path report page reads from the owners

**Orchestrator answers (2026-10-04, binding; they override the open questions below).** Q1 (footer sentence, finding 2): PENDING the user's answer. Leave the footer text exactly as it is today and do everything else; a small follow-up applies the answer. Q2: yes, you own `src/app/report.h` and the no-records mapping in `src/app/user_messages.cpp`. Q3, Q4: shared test files by case, as written. Also: re-pin the report's tier edges through O1's `past_difficult_floor` in `tests/test_report.cpp`, with 2.0 ms reading Normal and 170.0 ms reading Insane+ (D48 Q3; O1 pinned only the 2.0 ms floor in core). The squeezed-out backend row now reads "Free SqOut (uncounted) <-- squeezed out" (the D1 join made `summarystr` own the tag); if the report prints that row, it reads the same string.

Base: the `claude/p3-d1` join commit (hash 47eafff (claude/p3-d1)). Branch: `claude/p3-k1a`. Worktree `.claude\worktrees\p3-k1a`.

## Goal

The path report page (hydra_report and the app's Report button) spells several facts by hand: which paths are optimal, how a timing rounds, how a count sits next to its noun, which fill rule a page lists, how a title is cleaned, and why a report is empty. Wave D1 built one owner for each fact. This task points the report's body, footer, tiles, subtitle, empty reason and page numbers at those owners. Nothing here changes a score or a stored record.

## What changes, finding by finding

**7, which paths are optimal (D48 Q1).** Today `collect_rows` in `src/app/report.cpp` numbers each record's paths by sort position, and the page's "Best path only" box and the bold row keep only rank 1. The Paths tab calls every path tied at the top score Optimal. Decision: tied top-score paths are all optimal, in the report too. Owner: `HydraRecord::is_optimal` (O1, `src/core/model.h`). The payload carries an optimal flag read from that owner beside the rank; the page's filter and row class read the flag, not `rank === 1`. The subtitle's record count stays one per record. On screen: with "Best path only" ticked, a record with two tied paths shows both, both bold.

**36 and 2, how a timing prints and what the footer claims (Q2, Q5).** Today the page's `fmtMs` rounds with `toFixed(1)`, which can round an exact half the other way from the app. Decision: the report prints the app's own string. The payload carries the Hardest ms and Early fill columns as text from `format_ms_spaced` (`src/app/display_format.h`, the existing one-decimal owner), and the page prints that text; `fmtMs` goes. The tile over the table shows the text of the row with the largest Hardest ms. The footer today says "Timing tiers match Hydra's squeeze ratings", which is false (the Paths rows measure distance from the SP end, the tiers measure size). Decision: reword it. The exact sentence is under Open questions.

**35, the "Tightest squeeze" tile (Q7).** Today the tile can show an early fill's timing under the word "squeeze". Decision: the tile reads "Hardest ms", the column's name. Text only.

**14 and 99, counts and commas (Q12).** Today `report::counted` in `report.cpp` is a copy of the house rule, the subtitle types "bars" by hand, and the page scripts call `toLocaleString()` with no locale, so a German browser prints 1.234 under a subtitle saying 1,234. Owner: `counted(n, one, many)` beside `group_thousands` in `src/core/model.h` (O1). `report::counted` is deleted and its two subtitle calls and the cap label call the owner, so SP cap 1 reads "SP cap 1 bar". The page's shared `fmt` in `src/app/html_page.cpp` groups with a fixed en-US rule, and every bare `toLocaleString()` in `html_page.cpp` and `report.cpp` (the stats tiles, the # column, the footer count) goes through `fmt`. The CLI's "Wrote N path rows" line in `src/cli/report.cpp` uses the owner too.

**55, the fill rule's name (Q13).** Today `generate_report` appends the literal "Clone Hero 1.0 fills" to the cap label. Owner: `fill_rule_name` (O3a, `src/search/graph.h`), long form. The word "fills" after it stays.

**8, titles with Clone Hero tags (Q14).** Today `report::plain` in `report.cpp` strips only `<color>` tags, so a `<b>` title shows raw. Owner: `display_title` (O3a, `src/parse/song.h`) for the song name, which also gives "(unknown)" when nothing is left. Artist and charter use the same stripping without the fallback; if O3a landed no fallback-free form, use `strip_rich_tags` (`src/app/library_query.h`) and say so in your report. `report::plain` and its declaration are deleted. K1b removes the calls in its own files at the same time; the D2 join checks the build.

**105, why a report is empty (Q28).** Today `generate_report` returns `rows == 0` and every caller reads that as "nothing stored", even right after a batch stored records under another cap or fill rule. Owner: `generate_report` itself says why: nothing stored, or stored but nothing Ready under this cap and fill rule, or cancelled. hydra_report prints the reason; the app's `ReportJob` throws it and the strip shows it. The sentence when records exist is "Nothing is analyzed under these settings (SP cap 8, Clone Hero 1.1 fills). Analyze with these settings, or change them.", with the cap from the options and the rule from `fill_rule_name`. Today's "No records stored yet. Run hydra_batch first." stays for a truly empty database, and the app keeps its own sentence for that case.

**104 and 6, two phase 4 sentences that live in your file.** The subtitle today says "top N paths per chart"; the cut is per chart and mode. It reads "every mode at the current cap, top N paths per chart and mode" (the "every path" branch keeps its words). The Avg multiplier tooltip opens with "Points per note on average"; it opens with "Average multiplier: the score without solo bonuses divided by the base score (every note at 1x)".

## Owned files

`src/app/report.cpp`, `src/app/report.h`, `src/app/html_page.cpp`, `src/cli/report.cpp`, `src/ui/library_jobs.cpp` (only the empty-report throw in `ReportJob::run`), `src/app/user_messages.cpp` (only the no-records mapping and its sentence), `tests/test_report.cpp`, `tests/test_cli.cpp` (only the hydra_report case), `tests/test_single_owner.cpp` (one appended row). The plan's table lists neither `report.h` nor `user_messages.cpp`; see Open questions.

## Tests

Extend or add these cases in `tests/test_report.cpp` and `tests/test_cli.cpp`, using H1's record with a tied top-score variant (`tests/display_fixtures.h`):

- "report rows: a tied top-score variant is optimal too" — both tied paths carry the optimal flag, the third path does not, the record count is still 1.
- "report payload carries the hit window and the tier table" — extend: the ms field is the app's one-decimal string, and 12.25 ms prints the same text as `format_ms_spaced(12.25)`.
- "report lists only the wanted cap and names it" — extend: cap 1 reads "SP cap 1 bar", cap 1000 reads "SP cap 1,000 bars", a 1.0 page names the rule through the owner.
- "path report counts charts by hash in the tile and the subtitle" — re-pin the tile line to the `fmt` form.
- "path report explains and renames its columns" — the tile is "Hardest ms", the tooltip starts "Average multiplier:", the footer no longer says "match".
- "collect_rows: a blank or old-placeholder song name reads (unknown)" — extend: a `<b>` title reads clean, H1's tags-only title reads "(unknown)".
- "generate_report: one seam frames the page for every entry point" — extend: an empty store gives the nothing-stored reason; a store with a record at cap 4 asked at cap 8 gives the settings sentence naming "SP cap 8".
- "hydra_report writes a page for a filled database and says so for an empty one" — extend with the cap-8 run on the cap-4 database expecting "Nothing is analyzed under these settings".
- "the three report pages share one stylesheet and one script" — extend: no bare `toLocaleString()` in the page script (or rely on the scan row below).

Run: `-sf=*test_report.cpp`, `-tc="hydra_report*"`, `-tc="single-owner*"`. uitest scripts: `report-buttons`, `settings-and-reports`.

## Scan row

Question: "Which locale groups thousands on a report page?" Owner: `fmt` in `src/app/html_page.cpp`. Must match a line containing `.toLocaleString()`. Must not match a line containing `toLocaleString('en-US')`. Scope `src/app`. List the bare calls in `src/app/dm_report.cpp` and `src/app/fill_report.cpp` as known copies with removed_by "K1b"; the integrator deletes those entries at the D2 join.

## Done when

The report and the Paths tab agree on the optimal set and every ms string. An off-settings report names its reason. No bare `toLocaleString()` remains in your files. The plan's K1a row is satisfied and the corpus scores are untouched.

## Open questions

1. Footer wording (finding 2) is "reword" in D48 but no sentence is fixed. Proposed: "Timing tiers measure how big each squeeze is, in bands of your hit window. The Paths tab's row labels measure distance from the Star Power end and answer a different question. 'Beyond' is past the N ms window." Use it only once the main session confirms.
2. `src/app/report.h` must change (the reason field on `GeneratedReport`, the deleted `plain` and `counted` declarations) and `src/app/user_messages.cpp` must map the new reason to the strip, but the plan's K1a row owns neither. Both are proposed as K1a-owned here; O3b edits other lines of `user_messages.cpp` in D1, which lands before this fork, so there is no same-wave overlap.
3. `tests/test_report.cpp` is also touched by K1b (its comparison-page case at line 725) and `tests/test_cli.cpp` by K1b (the two hydra_fillcompare cases). Different cases; the join merges them.
4. `tests/test_single_owner.cpp` is shared with every D1 task and phase 5. Rows are appended; whoever joins second rebuilds the file.
