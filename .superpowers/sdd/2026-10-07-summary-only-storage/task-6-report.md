# Task 6 report: the ADR and the docs

Agent a9371f8f04e8dec71, branch `worktree-agent-a9371f8f04e8dec71`, forked from main at 2337539 (fast-forwarded first). Docs only; no C++ changed.

## What I did

I wrote ADR 0026, "The store keeps summaries; the engine gives the details". It opens with a card-catalogue analogy, then gives the audit's numbers (338 MB on disk, the per-table breakdown, the engine's 1.8 / 12.8 / 294 ms timings, the 74-chart exact match, and the share-once scheme that never shared a path). It says what is stored and why each piece is (charts, one summary row per chart and settings, meta), where every detail now comes from (the click, the report, `hydra_replay dump`), how Ready moves to the `hyversion` and `rules_fp` columns, and the upgrade. The upgrade section says it lands with storage-T5 and quotes T0's numbers (0.41 s, 292,372,480 to 13,033,472 bytes, report pass about 2.4 s). The three rejected options each get their reason: compression, the trimmed codec, and share-once with integer ids.

Six older ADRs got a "Superseded in part by ADR 0026" blockquote under their title, linking to the new ADR, with their text otherwise left as written: 0009 (keying by settings stands; stored-once is history), 0014 (the fingerprint lives in `rules_fp`; `hydra_batch --reindex` is gone), 0017 and 0022 (layout statements are history), 0018 (the four removed stamps), 0021 (history and transfer scales are stamped at analysis, never stored).

ADR 0010 had two hits. Its 2026-09-29 note named `get_record`, which T5 deletes; I reworded that one clause to say the loader is gone and the pather sets `legacy_fills` from the run's settings (pather.cpp:308 is the only other setter). I left its original-decision mention of `--reindex` alone, as history, and added a short closing note saying the flag is gone.

CONTEXT.md now defines Record as the in-memory result and adds a "Summary row" term that carries the key and the Ready / Stale rule. Its Analysis settings entry drops the 16 parked lookups (T2 deletes `kParkedLookups`) and says a setting change re-analyzes the open song (D90 item 1). Its UI timings entry replaces the "Done!" half second with the 0.15 s box delay (D87 items 6 and 10).

The User Guide now describes the click: analysis on open, the box after 0.15 s, Cancel with "Analysis cancelled." and "Try again", closing or switching songs cancels, and the summary is saved when missing, Stale or different. The settings bar no longer locks for one song. The Stale advice reads "Click the song or run a batch to refresh it." The song-length "works it out once and remembers it" sentence is gone, and so is "the panel shows the batch's new result". The path report section says it analyzes again, reuses the batch's songs, and names the `Left out:` line (D89 item 1). An edited chart is read fresh on open (D87 item 3).

development.md loses `hydra_batch --reindex` from the flag list and its stamp sentence. `hydra_replay dump` is described as analyzing the chart and reading no database. `hydra_report`'s two lines no longer say "stored". hydra_batch and hydra_report keep `--db`, because both still take it.

docs/agents/ui-testing.md follows T2's harness: the script example drops `Analyze this song` and the `Done!` wait, the lock text is batch only, the song panel row names the progress box, Cancel, "Analysis cancelled." / Try again and Continue, and the template and helper list use `wait_song_analyzed`. A new rule of thumb names `ViewGate`, and the Dynamics note says the counts come from the click. **This file must merge after T2**, because `wait_song_analyzed` and `ViewGate` exist only on T2's branch.

D77's entry gained one line: item 2 was replaced by D87 item 4. No other decision entry changed.

## Writing for the state after T5

The docs test checks that every backticked code name in the ADRs, CONTEXT.md, the User Guide and development.md still exists in src/, tools/ or tests/. So no new text backticks a name T5 deletes or T2 adds. I also removed the backticks from the removed stamp names already in ADR 0017 (one) and ADR 0018 (three), and ADR 0018's blockquote says why. A scratch script mirrors the test's name rule and ran three ways: against main's code, against T2's branch, and against T5's planned deletion list (get_record, for_each_blob, the path_codec names, the four stamps, the dynamics and tempo-map functions, RecordLookup, record_bytes, reindex). All three come back clean apart from the test's own allow-list.

## Acceptance criteria

1. A grep of the User Guide for "Re-analyze", "--reindex" and "--no-analyze" finds nothing: PASS (case-sensitive count 0). The lower-case `Also re-analyze charts that already have a result` label stays, because it is the batch dialog's real checkbox.
2. Every ADR named in the brief links to the new ADR: PASS (0009, 0014, 0017, 0018, 0021, 0022).
3. The docs-match-code test passes: PASS.

## Verify

`.\build_cpp.ps1 -Target hydra_tests` built (a full link, no stale-link trap). Then `.\build-cpp\Release\hydra_tests.exe -sf=*docs_match_code*` gave:

```
[doctest] test cases:  5 |  5 passed | 0 failed | 1184 skipped
[doctest] assertions: 88 | 88 passed | 0 failed |
[doctest] Status: SUCCESS!
```

No other tests ran.

## Concerns

Three backticked names in ADR 0014's D51 call 8 amendment may not survive T5: `rules_fp_of`, `upgrade_results_key` and `row_readable_sql`. T5's plan doesn't list them, but they read the structure blob or the path format. If T5 deletes any of them, the docs test fails after the post-T5 merge, and the fix is to drop that name's backticks in ADR 0014.

The ADR and the docs describe T2's and T5's final state. On today's main the ADR's upgrade and Ready-check sentences are ahead of the code, as the brief asked.

CONTEXT.md gains a "Summary row" term. That is a new glossary entry, not new app text.
