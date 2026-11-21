# Owner map: Clone Hero 1.0 fills match the game

Written by a read-only scout on 2026-11-21 (session 63088e3d), from main at 38304a9c9; checked in part by the main session (CONTEXT.md lines 651 to 661, rules.h lines 44 to 45, song.h line 35, fill_generator.md lines 32 to 36, Q-37). The shape is `docs/agents/owner-map.md`.

## The user's words

- To whether Hydra's 1.0 mode should match old Clone Hero 1.0 or stay as it is, the user chose "Match the old game" (2026-11-21, session 63088e3d; recorded on `docs/clone-hero/questions.md`, Q-38).
- To whether the plan should add the two rules Hydra has no code for (a chord counts only within 0.25 s of real time of the line; the fill starts half way in real time between the previous line and the chosen line), the user chose "Yes, add them" (same session).
- To whether the plan may rewrite CONTEXT.md's "Generated fill" entry to say 1.0 mode follows old Clone Hero 1.0's own fill code, the user chose "Yes, rewrite it", with the exact wording shown to the user before merge (same session).

## Stops and how they were settled

1. O4 and O5 had Status `none`. The user answered "Yes, add them", so the plan adds those two owners, taken from the game's code, and nothing else new.
2. Game facts not yet read. The game's seconds-to-tick and tick-to-seconds converters (their rounding) are not in `docs/clone-hero/code/4045/fill_generator.cs` or `names-4045.tsv`. The helper `AddLine` is not copied: `fill_generator.md` lines 112 to 115 say only measure beats go into the list the generator walks, but the code file does not show it. The activation-note rule (highest pad bit, FL-37) is not compared with `Chord::activation_note` (src/core/model.cpp:309). The plan's first task reads these from the game, and a result that changes the rule stops the plan and goes to the user.
3. CONTEXT.md lines 651 to 661 say 1.0 "keeps Hydra's own model" and cite ADR 0023, which has no 1.0 text. The user approved a rewrite; the new wording goes to the user before merge.
4. Q-37 (a written fill that lands on no chord still stops generation in the game, not in Hydra) is a separate open split with no real chart known. It is out of scope; 1.0 keeps today's behaviour there.
6. Row O2 in a meter with 1 on top: the game makes only the first beat after the meter change a measure line (fill_generator.cs lines 215 to 231; FL-40), while `MeasureIndex` puts a line every beat (src/core/timing.cpp line 127 and `measure_line_tick`). Checked by the main session. The user's "Match the old game" covers where 1.0's fill candidates are, so the 1.0 generator follows the game's lines here. 1.1 must not change: the 1.1 build has the same exception (`docs/clone-hero/code/6142/grid_restart_20DACA0.md` line 16), but no user words ask for a 1.1 change and no chart in testdata/input has such a meter, so the plan records it as a finding, not a task and not a question.
5. Whether a sub-tick fill start matters (the game keeps a start second, Hydra only whole ticks, `SongTimestamp::activation_length`, src/core/song.h:35) is not a user question: the plan measures it.

## Map

| # | Question | Owner today | Game | Status |
|---|---|---|---|---|
| O1 | Which measure lines are candidates; first line, step, after a miss | `Song::place_fills_ch10` (src/core/song.cpp:138-170): each chord's own line and the next, then `rules.fill_cooldown_measures` lines after a fill. 1.1: `place_fills_ch11` (song.cpp:187-230). Both read `Rules::fill_cooldown_measures` (default 4, src/core/rules.h:44) | fill_generator.cs 38-57: `i = 1; i += 4; i -= 3` (second line, every fourth, next line after a miss); literal 4 | found, differs |
| O2 | Where the measure lines are, restart at a meter change | `MeasureIndex` (src/core/timing.h:129-178, timing.cpp ~154-200) | fill_generator.cs 197-255 (F5); lines only up to the last gem end | found; numerator-1 uncompared (FL-40); Hydra's 1.0 line map is chord-driven and can add a line past the last gem |
| O3 | Which chord a line takes (nearest, tie to later) | Three inline copies: `place_fills_ch10` (song.cpp:146-155), `place_fills_ch11` (song.cpp:204-214), `place_authored_fills` (src/parse/song.cpp:407-418) | `NearestChord` (F4), fill_generator.cs 144-193, searches on from the last chord used | split |
| O4 | How close the chord must be, in real time | Ticks only: `tick_resolution_ * fill_max_distance_beats` (song.cpp:158-159; rules.h:45, shared with 1.1). `MsIndex::at` (timing.h:78) gives ms of a tick | `num5 > 0.25f`, seconds, fill_generator.cs 51-57 | none; user: add it |
| O5 | Where the fill starts | `fill_start_tick(chord_tick, activation_length)` (src/core/model.h:324) reads a start; nothing computes the midpoint. `MsIndex::tick_at_ms` (timing.h:87-100, fenced: "never use them in the graph or the engine's scoring path, apart from the Clone Hero 1.0 fill deadline") | midpoint in seconds between line i-1 and line i, then to a tick (fill_generator.cs 42, 43, 63, 67) | none; user: add it |
| O6 | Where the fill ends | Engine treats the chord tick as the end (`add_act_edge`, src/search/graph.cpp:381-389). The 64th appears only as `res / 16.0` in the Ch10 deadline branch (graph.cpp:85-87, cited to Python Hydra 1.2). `Chord::activation_note` (model.cpp:309) | activation note tick plus `RoundToInt(12 * res / 192)` (fill_generator.cs 39, 59, 63-70) | found, with problems (inline constant, fractional vs whole ticks) |
| O7 | Fill length for the deadline | `activation_fill_deadline_ms` Ch10 branch (graph.cpp:62-91); 0.25 s and 10 s clamps inline at graph.cpp:89 | `CheckFillDeadlines`, fill_deadline.cs 17-31; FL-42 | found; agrees only at a steady tempo |
| O8 | When the game vets a fill, what "now" is | `fill_e_offset`, `fill_refuses` (model.h:336-338), engine.cpp:807, 1006, 1699 | fill_deadline.cs 7-14 | found; FL-41 unchecked |
| O9 | Across a tempo change | Generator: ticks only. Deadline: `MsIndex::ms_at_tick_f` (timing.cpp:95), inverse `tick_at_ms` | seconds throughout | found; reach and start covered by O4, O5 |
| O10 | A track with any written fill gets no generated fills | `Song::check_activations` (song.cpp:107-119); coda in `place_authored_fills` (src/parse/song.cpp:383-406) | fill_generator.cs 19-27, 88-91; FL-35, FL-39 | found; Q-37 out of scope |
| O11 | How the Preview draws a 1.0 generated fill | `drawn_fill_start_tick` (src/app/preview_view.cpp:75-82), scene 317-327; span min(start, chord) to max(start, chord) | n/a | found; comment "already that half measure long" goes stale; drawn span ends at the chord, not the 64th past it, and no ruling covers that |
| O12 | What picks the 1.0 rule | `fill_rule_for`, `legacy_fills_for` (src/search/graph.h:45, 51); generator choice in `Song::check_activations` | n/a | found |
| O13 | Texts that describe the 1.0 rule | `fill_rule_description` (graph.h:75-81), `legacy_fills_help_text` (src/ui/settings_bar.cpp:25-32), `kCh10FillLengthMeasures` comment (song.cpp:123-126), src/app/rules_file.cpp:26, rules.h:39-43, CONTEXT.md:641-669 | n/a | found; several go stale |
| O14 | Stored records and the stale stamp | Key `store::Lens::legacy_fills` (src/store/record_store.h:122-150; refusal record_store.cpp:687-693; written pather.cpp:547). Stamp `kResultsStamp` (src/store/stored_versions.h:55), one string for all records; its comment says bump it when analysis output changes. Reports: src/app/fill_report.cpp, report.cpp:293,338, dm_report.cpp:56-67, saved_figure_fill.cpp:99, src/cli/fillcompare.cpp, tools/replay.cpp:469, tools/bench.cpp. Fixtures `testdata/store/*-legacy-fills.db`, `testdata/replay/compare_db/*` | n/a | found; one stamp, so a bump marks 1.1 results stale too |

## Tests that pin today's 1.0 behaviour

- `tests/test_generated_fills.cpp`: helpers `ch10_reference`, `one_measure_share` (93-107); cases at 153-169, 254-273, 275-285; 197-215 pins only the Ch10 deadline.
- `tests/single_owner/rules/parse-fills.json` 65-136 pins the text of `place_fills_ch10`.
- `tests/test_fill_deadline.cpp` 34-142 pins the deadline formula (64th before the start, 250 ms floor, 10,000 ms ceiling).
- `tests/record_fixtures.h:108` `make_ch10_fill_song` (hand-set fills, deadlines 18968.75 and 18768.75 ms), used by `tests/test_fills_and_skips.cpp:87-119`, `tests/test_preview_scene.cpp:340-360`, `tests/test_replay.cpp:1046`.
- `tests/test_preview_scene.cpp` 177-197; `tests/test_preview_load_progress.cpp` 440-456; `tests/test_song_parse.cpp` 58-74 (1.1 only).
- Self-consistency under 1.0: `tests/test_replay.cpp` 103-130, `tests/test_perf_digest.cpp:133`.
- Use the Ch10 key, not read for pinned scores: `tests/test_fill_report.cpp`, `test_report_rows.cpp` (839-888), `test_cli.cpp`, `test_dm_report.cpp`, `test_store_open.cpp`, `test_store_results.cpp`, `test_engine_sweeps.cpp`, `test_batch_text.cpp`, `test_config.cpp`, `test_preview_controller.cpp`, `tests/ui/uitest_details.cpp`.
- Unchanged by this work: `tests/test_fill_landing.cpp` 417-421, 463-490 (1.0 coda for written fills); `tests/test_rules.cpp:259`, `tests/test_rules_file.cpp:60`.
