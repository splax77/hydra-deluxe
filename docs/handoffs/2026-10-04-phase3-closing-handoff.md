# Phase 3 closing handoff (2026-10-04)

Phase 3 is done. Every displayed fact it covered now has one owner function, and the screens read that owner. It merged to main in two steps: M_C (wave C) as 16c836c, and M_D (everything else) as 57dcf23. Neither is pushed. The plan (`docs/superpowers/plans/2026-10-04-phases-3-5.md`) records both. The triage marks 47 findings fixed (2278a6a).

M_D passed five derive-once review rounds and ended CLEAN at 66e6af7. On that tip the full suite passed 989 of 989, and `hydra_uitest --all` passed 63 of 63. The 97 corpus charts score byte-identically to main. Fifteen charts from the user's library store byte-identical rows on main and on M_D, and the results stamp stays 2.1.0, so nothing needs re-analysis.

The user made four sets of display calls along the way: D50, D52, D56 and D57 in `docs/audit/2026-10-03-fix-decisions.md`. The ones later work is most likely to meet: one-decimal ms always reads "163.5 ms" through one `format_ms`, and a missing artist reads "(unknown)" everywhere. Report-page search folds accents and matches every word against the shown text, and nothing more. The Preview scrubber ends at the last note while the clock keeps the audio length.

## What phase 3 left open, for steps 6 and 7

These findings were touched but not closed. Each is still open in the triage. They are questions for the step that owns them, not phase 3 debt to finish first.

- **8:** the leaderboard page shows DMBot's artist and song name uncleaned, while every stored name goes through `display_title` or `display_artist`.
- **13 and 172:** dead code in `build_record_status` still carries the old "No paths found." copy and the "Paths kept" count.
- **14:** `count_label` is still a forwarder that `dynamics_tab.cpp` calls instead of `counted`.
- **51 and 155:** the report pages' script still works out where Beyond starts and which rows are past it. The cells and tiles now read C++ text.
- **55 (ledger fills-34):** the settings bar's help text restates the 1.0 and 1.1 fill deadlines in its own words.
- **107:** the settings words need a core owner. Phase 4 deferred it until phase 3 merged, and that has now happened.
- **218:** only the code half was done. Whether the hover teal should be 0,104,104 is a display question nobody has asked yet.
- **261:** the all-0 cost against optimal is still worked out twice in `path_view.cpp`.
- **312:** the label half is done. The fill page's JS status tally is still a separate count from `tally_fill_rows`.
- **219:** the Preview's 6.0 s fade has no triage entry and stays as it is.

The phase 3 ledger (`.superpowers/sdd/2026-10-04-phase3/progress.md`) lists three smaller items. Three UTF-8 readers still read lead bytes their own way: `json_escape_into` in `html_page.cpp`, `clip_utf8` in `batch.cpp`, and the continuation test in `overlay_layout.cpp` (`utf8_length` in `library_query.cpp` is the likely owner). Two colour values still repeat (58,58,62 twice in `paths_tab`, 0,0,0,128 five times in `preview_tab`). And `tools/bench.cpp` crashes on a chart with no notes.

One migration line is a deliberate known copy. `record_store.cpp`'s schema 2 migration compares the stamp to "ch10" by hand, because it reads text old files hold and store never includes search. `tests/test_single_owner.cpp` records it.

## Lessons worth keeping

- **Run the full suite after every main merge into a long branch, not only at the end.** Three tests broke only where main's new `prepare_row` check met a fixture phase 3 added, and filtered runs never saw it.
- **Never SendMessage a workflow agent that is still running.** It forks a second copy into the same worktree. Put late answers in the brief file instead.
- **The gate refuses a review the main session submits.** When several area reviewers split one review, a fresh agent must file the combined text.

## Clean-up

The phase 3 worktrees under `.claude\worktrees\` (`p3-*`, plus `p3-mainbase` for the baseline build) can go once nobody needs them. Phase 6's J2 launched from the `p3-d2` join, so check with that session before removing `p3-d2`.
