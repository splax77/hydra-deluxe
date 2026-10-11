# Test audit A: engine and chart-parsing tests

Scout TA-A (agent a66c0e2749393b6e0). Read-only. About 55 tool calls.
Paths are in `C:\Users\Patrick\Downloads\Hydra\hydra-test\tests\`. Line numbers are from the checkout as it stood at the start.

The short version: this area is in good shape. Most tests pin literals, drive the real parsers and engine, and avoid sleeps and the user's library. The findings below are mostly about one very slow test, copy-pasted blocks, a few misleading names, and a few tests that pin the old implementation instead of the behaviour.

One thing for the main session to see first: `test_search.cpp` (about 2,150 lines, 55 test cases about the engine's search and graph) is not on my list and I did not see it on anyone else's. Someone should audit it.

How I sampled: I read every test-case title in all my files. I read in depth: most of `test_replay` (head, helpers, 1293-1660, 1779-1944), `test_analysis` 190-430, `test_fast_tempo` 1-240, `test_stars` (all), `test_rules` 1-135 and 196-471, `test_midi` 1-100 and 300-440, `test_note_shuffle` (all), `test_fill_deadline` (all), `test_chord_code` (all), `test_s2_difficulty_table` (all), `test_s2_parser_owners` (all), `test_perf_digest` and `test_corpus_cache` (all), parts of `test_song`, `test_model`, `test_squeeze_rating`, `test_dm_report`, `test_fill_report`, `test_dynamics_breakdown`, `test_srb`, `test_track_state`, `test_s2_kick2x`, `test_s2_dynamics_tag` and `test_s2_deact_extension`.

---

## (a) Working well (keep this style)

**Literal tables with a named source.** `difficulty_literals.h` is one table of Clone Hero's pitches, quoted with the game's addresses in the header comment. `test_s2_difficulty_table.cpp:18-61` checks the parser against it, and other tests read their per-difficulty values from it instead of typing a second table. That is exactly the house rule. A real bug it would catch: the parser reading Hard's kick from Medium's pitch (`test_s2_difficulty_table.cpp:38` loads each difficulty from one file and checks each lands on its own tick).

**Note Shuffle against an outside oracle.** `test_note_shuffle.cpp:95-220` pins chords that came from a separate Python reference and from real game footage. It also has hand-built edge cases (seed wraps at 64 bits, tick 0, yellow tom blocks yellow cymbal). A wrong random-number step or a wrong "copy from two chords back" rule would fail it at once. The weakness is only layout (see (d)).

**Two independent paths must agree.** `test_replay.cpp:50` replays every stored path of every corpus chart and compares the replay total with the engine's total. The replay walks chords; the engine walks graph edges. A bug in either one's scoring breaks the match. `test_replay.cpp:1815-1873` goes further: it checks that the rows from many windows together equal the sum of replaying each window alone (`check_windows_add_up`, line 1614). That oracle is built from the production function, not a copy of it, which is the right way.

**Hand-built charts with "by hand" arithmetic in the comment.** `test_replay.cpp:1779-1809` and `1293-1369` build tiny songs and pin literal rows, with a sentence saying why each number is what it is. Good model for the rest.

**Real-bug regression tests with their story.** `test_analysis.cpp:201-241` (`run_work_pool: a slow consumer holds the workers back`) records the "bad allocation" incident, and asserts an upper bound on how many results pile up. Because it asserts an upper bound, a slow or busy machine can only make it pass more easily, never fail wrongly. `test_midi.cpp:306-329` pins the 2^32-length meta wraparound.

**Byte-count checks instead of timing.** `test_srb.cpp:159` and the sng equivalent check that the loader reads fewer than 2 MB of a 16 MB file, using a counting byte source. That is a behaviour check ("do not read the audio"), with no wall clock.

**Whole-corpus golden digests.** `test_perf_digest.cpp:90-102` pins one 64-bit digest of every corpus chart's engine result and parse. One change that moves any field of any answer fails it. Cheap (about 0.08 s).

**Property tests with fixed seeds.** `test_track_state.cpp:823` (random overlays vs full build, seed 20261004) and `test_fast_tempo.cpp:111` (fuzz charts on a fixed generator) are deterministic and explain their generator.

**No sleeps, no real audio, no network, no real %APPDATA% in this area**, except the loopback server noted in (b). I grepped for sleeps, clocks, threads, environment reads and the user's library path across my files. The only hits are listed below.

---

## (b) Broken or misleading

**1. `test_replay.cpp:1371` "replay multipliers agree with the combo on every corpus chord" checks one chart, not every chord of the corpus. Severity: medium.**
It loads `corpus::first_chart_with_suffix(".mid")` and replays it once (line 1372). The name says "every corpus chord". A multiplier bug that only shows on a chart with a time-signature change or a different resolution would pass. Fix: either loop `corpus::chart_paths()` like its neighbours do, or rename it "on the first .mid chart".

**2. `test_replay.cpp:1910` is a benchmark disguised as a test, skipped by default, aimed at the user's own library. Severity: medium.**
`doctest::skip()` keeps it from running, so it protects nothing. Its default input is `C:\Clone Hero\songs\Misc Downloads\blink-182 - Discography\notes.mid` (line 1912), a path on the user's machine. It has no timing assertion, only a MESSAGE, so even when run it can never fail on slowness. Fix: delete it; `hydra_bench` already exists for timing. Same shape: `test_s2_dynamics_tag.cpp:154` ("dev aid: print the Chart line for real charts") returns at once without the env var (line 156) and has no assertion when it does run. It shows up as a passing test while testing nothing. Move to a tool or delete.

**3. `test_perf_digest.cpp:64-76` swallows every exception without counting. Severity: low.**
If the engine started throwing on all 97 charts, `engine_digest` would hash the empty set. The test would fail only because the pinned literal no longer matches, which is fine today, but when someone repins the digest after a "meant" change they cannot tell that every chart now fails. Fix: count successes and `CHECK(analyzed == corpus size)` next to the pin, or fold failures into the digest on purpose (the parse digest already does, via `chart_parse_hash`).

**4. `test_squeeze_rating.cpp:702` "effective_backend_ms: the factor of two is the identity budget over the scaled budget" does not test that. Severity: low.**
The body is two checks at the identity scale (170.0 equals 170.0) and a comment saying the scaled case is pinned somewhere else (lines 707-708). A bug in the factor-of-two for a scaled row would not fail it. It also repeats `test_squeeze_rating.cpp:850` (`squeeze_budget_ms: identity scale is twice the hit window`). Delete or merge into 850.

**5. `test_rules.cpp:461` "a run is stamped with the rules fingerprint at every cap" has an unconditional `break` after the first chart (line 470). Severity: low.**
It looks like a loop over the corpus but is a one-chart test. Harmless, but the loop shape misleads. Fix: take `corpus::first_chart_with_notes()` and drop the loop.

**6. `test_dm_report.cpp:571-605` has a wall-clock threshold. Severity: low.**
It starts a real WinHTTP request against a loopback socket that never answers, sets cancel after a 300 ms sleep, and checks the call returns within 500 ms (line 604: "~50 ms expected"). A measured run returned in 63 ms, so there is room, but a very loaded machine could fail it. The thing it protects (cancel is polled inside WinHTTP) is real and cannot be tested without a real transport, so keep it. Fix if it ever flakes: widen to 2 s; the failure it guards against is a 120 s hang.

**7. Self-comparison checks that cannot fail on their own. Severity: low.**
`test_model.cpp:222` and `:224` check `a.is_e_critical() == is_e0(a.e_offset, 0)`, and `:187` checks `a.is_E0() == is_e0(10.0, 0)`. If one of the two functions is written as a call to the other, these are tautologies. The edge values just above them (`:178-179`, `:215-218`) are the real protection. Fine to keep as "the two halves agree" checks, but a reader should know they are not what pins the edge.

---

## (c) Unnecessary (delete or merge)

**1. Two test cases pin the same two fingerprint literals. Severity: low.**
`test_rules.cpp:415-427` and `:429-447` both check `default_rules().fingerprint() == 0x70d2e96669604cf2` and `retired_auto_fingerprint() == 0x5b610b430a43a4be`. Merge into one case.

**2. The same "(unknown) / tags removed / blank charter" block is pasted into three places. Severity: medium.**
`test_fill_report.cpp:477-523`, `test_dm_report.cpp:332-377` and `test_song.cpp:920-951` all walk the same five steps: blank name reads "(unknown)", tag-only title reads "(unknown)", bold title loses tags, tag-only artist reads "(unknown)", empty artist and the scan placeholder read "(unknown)" and a charter loses its end spaces. The rule has one owner (`display_title`, `display_artist`, `display_charter`), pinned in `test_song.cpp`. The two report copies only prove that each report calls the owner. One short case per report that feeds one tagged title and one blank artist would do that. Two more pairs repeat the same way: "a database with no chart library stops with the path report's sentence" (`test_fill_report.cpp:613` and `test_dm_report.cpp:790`) and "a result whose chart the library doesn't list is left out" (`test_fill_report.cpp:639` and `test_dm_report.cpp:809`). The long sentence "This database has no chart library. Run hydra_batch without folder arguments..." is typed out in full in both files, so one wording change breaks several tests. Fix: pin the sentence once at its owner and compare the reports' text to the owner's function.

**3. `test_squeeze_rating.cpp:680-700` repeats `:626-678`. Severity: low.**
Both check the Insane+ cutoff of 170.0 and that Beyond and None have no cutoff. The one new thing in 680 is that the cutoff equals `nominal_budget_ms(...)`. Fold that line into the first case. Also `:716` (`CHECK(kSqueezeWindowMs == 500.0)`) only pins a constant; the real protection is the rest of that case.

**4. `BackendSqueeze::summarystr` ladder is pinned in three files. Severity: low.**
`test_rules.cpp:168-218`, `test_model.cpp:733-746` and `test_squeeze_rating.cpp` all touch the label ladder (Standard, Hard, Easy, "(uncounted)"). The edges are legitimately different edges, but a single table-driven case (offset, is_sqout, expected label) would show the whole ladder in one place and make a future ladder change one edit.

**5. `test_corpus_cache.cpp` tests test helper code, not Hydra. Severity: low.**
It checks that `corpus::song` and `corpus::analyzed` (in `corpus_util.h`) return the same object twice. It is cheap (under 1 ms) and protects the speed of the corpus loops, so it is fine to keep. It cannot catch a product bug.

**6. Probable overlap, not verified in the bodies: `test_replay.cpp:147` "targeted search reproduces every corpus path" and `test_fast_tempo.cpp:642` "search_target: every corpus path pinned in full comes back alone". Severity: low.**
Both run a targeted search on every corpus path (0.14 s and 0.19 s). I read only the titles. They may check different things; worth a look when someone moves the `search_target` cases (see (d) item 4).

**7. Stale and numbered comments. Severity: low.**
`test_dynamics_breakdown.cpp:156` says "Test 3" and `:210` says "Test 4", and the cases around them are no longer in that order. `test_midi.cpp:307` says "Today it wraps to 0", describing the bug as present. These are small, but they teach the reader wrong things.

---

## (d) Poorly designed

**1. One test is 43 percent of the whole suite. Severity: high (speed).**
`test_track_state.cpp:871` "rebuild_overlay_fields: equals a full build on every corpus chart and stored path" took 18.2 s in the main session's run, where the whole suite took 42.1 s wall. It builds a scene for every stored path (tied variants included, through `rec.all_paths()`, line 882) of every chart, then for each neighbouring pair calls `check_rebuild` (line 755), which does four full builds (both Pro settings, there and back) plus a window check between every pair of instants. The random test just above it (`:823`) already checks the same property on dense synthetic data with five scenes. Fix: for each chart check only the base, the best path and the empty overlay (three scenes, two swaps), or cap at the first ten charts. That keeps real-chart coverage and should cut it to about a second or two. Nothing else in my area is slower than 0.4 s in that run; the next slowest are the loopback HTTP cancel case (0.37 s) and `dynamics_breakdown: the rules never change a chart's Dynamics count` (0.35 s).

**2. Files and tests named by plan step and audit number. Severity: medium.**
The whole `test_s2_*.cpp` family is named for "Step 2" of a past plan (`test_s2_parser_owners.cpp:1` says "Step 2, task 6"). Test names carry "finding 37" (`test_s2_deact_extension.cpp:97-316`), "(331)", "(R7.5)", "(258, 319)", "D105", "D34". A reader cannot tell what these protect without opening the audit docs. `test_s2_parser_owners.cpp` is a grab bag: chord modifier errors, number parsing, a song.ini delay, practice-section order plus the Preview's time box (`app::build_time_box`, line 141), the default meter, an .srb entry name, and blank artists through the scan and the rescan cache. Fix: rename by behaviour ("kick2x", "disco", "fill landing" already read well) and move each owner test into the file of the thing it owns (parsers into `test_song`, the scan one into `test_analysis`, the time box into the Preview tests). Keep the decision ID in a comment, not the test name.

**3. Giant files. Severity: medium.**
`test_replay.cpp` is 1,944 lines with 49 cases and its own 130-line helper block in the middle (1430-1650); `test_model.cpp` is 1,089 lines mixing number formatting (`group_thousands`, `counted`, `format_ms_whole`), Activation and Path rules, Chord strings and MultSqueeze; `test_song.cpp` 1,041; `test_analysis.cpp` 989 mixes the scan, the rescan cache, the work pool and the batch runner; `test_squeeze_rating.cpp` 878; `test_fast_tempo.cpp` 827. Fix: split along those seams. The helpers in `test_replay.cpp` (`first_row_difference`, `synthetic_windows`, `check_windows_add_up`) are a good candidate for a header.

**4. `search_target` tests live in `test_fast_tempo.cpp`. Severity: low.**
`test_fast_tempo.cpp:457-780` is eight cases about `search_target` and `keep_target_paths`, which have nothing to do with fast tempo. They belong beside the other search tests (`test_search.cpp`). The file's own header (lines 1-10) says it is about charts with SP bars shorter than 500 ms.

**5. Huge inline literal tables. Severity: low.**
`test_note_shuffle.cpp:100-123`: two lines hold 70-plus chords each, with no line breaks. A reviewer cannot check them by eye, and a diff after a repin is one unreadable line. Fix: put the vectors in a text file next to `gen_vectors.py` output (that file already exists as `vectors_output.txt`) and load it; this also removes the dependence on `docs/audit/...` paths (`test_note_shuffle.cpp:80`, `test_song.cpp:825`) staying where they are.

**6. A second copy of an old rule as the oracle. Severity: medium.**
`test_song.cpp:604-727` compares the hand-written disco-marker and section-header matchers against `std::regex` objects that are copies of the rules they replaced (`\[?mix.3.drums\d?d\]?`, lines 607 and 611). The house rule is that a test never recomputes the expected value with a copy of the production rule. This is a migration test: it was right when the hand matcher replaced the regex, and it is now a second definition of the rule that has to be edited with the first. It also runs about 4,500 parses per format. Fix: freeze the oracle into a table of (marker, on / off / unchanged) generated once from the regex and kept as literals; keep a smaller byte-by-byte loop only for "never crashes".

**7. Tests that pin vector capacity. Severity: low.**
`test_midi.cpp:384-411` (`capacity() == size()`, line 410) and, by its title, `test_song.cpp:87` ("keeps no spare room in its chord list", I did not read the body). These guard against memory waste, which is a fair goal, but they fail on a harmless refactor that builds the vector differently. If the goal is memory, a check on bytes allocated (as the srb test does with a byte counter) would survive refactors.

**8. Expected values re-derived by formula next to the literal. Severity: low.**
`test_fill_deadline.cpp:63-64` computes the expected lead as `3750.0 / bpm`; `:91-92` and `:104-105` as `fend - fill_len_ms - 250.0` / `10000.0`. The next line in the last two (`:93`, `:106`) does pin a literal, so the protection is there, but the file header (lines 8-9) says "the expected values are worked out from the formula". `:63-64` has no literal at all. Fix: pin one literal per tempo in a small table. Also `:80` is a negative check (`!= Approx(9564.0)`) that only adds to the positive one above it.

**9. Three ways to find the test data. Severity: low.**
`corpus::root()` (`test_stars.cpp:221`), `HYDRA_INPUT_DIR` (`test_fast_tempo.cpp:43`, `test_s2_disco.cpp:123`), `HYDRA_TESTDATA_DIR + "/input/..."` (`test_dynamics_breakdown.cpp:163`) and `sourcetree::root()` (`test_note_shuffle.cpp:80`). Several files each carry their own `#ifndef ... #error` guard. Fix: one `corpus::` accessor for each place and have the tests call it.

**10. Repeated setup that wants a table. Severity: low.**
`test_stars.cpp:121-153, 185-201, 267-292` build the same `Path` (1000 base, 100 ghost, 200 accent, 800 solo) four times for four cases. `test_replay.cpp` has eight copies of the "Song(192), 120 BPM, push chords at fixed ticks" setup; `make_chord_song` exists (line 809) but `squeeze_chart` and the inline loops at 1783 and 89-103 do not use it. `test_rules.cpp:46` and `test_dynamics_breakdown` and `test_preview_view` repeat the same two-line `bpm_changes[0] = 120.0` setup (18 hits in four files).

**11. Positional initialisers that hide meaning. Severity: low.**
`test_s2_kick2x.cpp:83-85` sets `rows[...] = {1, 2, 3}`, `{0, 1, 40}`, `{0, 0, 12}` with no field names; the reader has to know the order is ghost, accent, normal. Use designated names or a small helper.

**12. Settings built two ways in one file. Severity: low.**
`test_stars.cpp:222-226` builds `AnalysisSettings` by hand; `:299` uses `scratch_settings()`. Pick one.

---

## (e) Gaps

**1. The engine digest is pinned only at the default settings. Severity: medium.**
`test_perf_digest.cpp:90-102` pins the engine digest once (Expert, Pro Drums, 2x Bass, cap 4) but pins the parse digest at two settings. Nothing pins whole-corpus engine output under the Clone Hero 1.0 fill rule, a different leeway, 2x Bass off, or Hard. Those paths are exercised by spot tests (`test_rules.cpp:269` uses the 1.0 rule on one chart), not by a corpus-wide pin. A refactor that changed the 1.0-rule graph on charts other than Sugar would be caught only if some other test covers it.

**2. No test for `fill_rule` combined with the full corpus replay.** `test_replay.cpp:50` and `:147` use `Settings().to_analysis_settings()` only, so the replay-vs-engine agreement is checked only under the 1.1 fill rule and cap 4. Same reasoning as above. Severity: low to medium.

**3. `test_search.cpp` is unassigned.** About 2,150 lines, 55 cases, covering the graph, SP-end history, tied variants and banks (titles listed above in my read). It holds the engine's most intricate logic and the largest share of the "D34/D36/D38" regressions. Nobody on my list reads it. Severity: not a gap in tests, a gap in this audit.

**4. Nothing here tests the lean .chart reader on a truncated or binary file.** `test_midi.cpp` covers truncated and oversize .mid, `test_srb`/`test_sng` cover truncated containers, but the `.chart` malformed-line cases (`test_song.cpp:731-800`) are all well-formed-but-odd text. A chart with a NUL byte or a 2 GB line is not tried. I did not check whether another file does. Severity: low.

**5. Short-circuit in the "any_moved" leeway test.** `test_rules.cpp:220-247` stops at the first chart where the leeway moves a score (line 243). The monotonic checks `s_none <= s_default <= s_wide` therefore run on a handful of charts at most, not the corpus. If monotonicity broke on a later chart this would not see it. Severity: low.

---

## (f) Files and areas not reached

Read only by title (the TEST_CASE list), not in the body:
`test_timing.cpp`, `test_song_length.cpp`, `test_sng.cpp`, `test_fill_report.cpp` (except 477-523 and 613-652), `test_s2_offspeed.cpp`, `test_s2_phrase_end.cpp`, `test_s2_fill_landing.cpp`, `test_s2_disco.cpp`, most of `test_s2_dynamics_tag.cpp` and `test_s2_deact_extension.cpp` (first 130 lines read), `test_model.cpp` (about 40 percent read), `test_squeeze_rating.cpp` (lines 1-80 and 620-720 read; the "field fixture" cases at 208-380 and the `rate_activation` cases at 405-620 not read), `test_dm_report.cpp` (332-377, 496-630, 790-830 read), `test_srb.cpp` (155-265 read), `test_track_state.cpp` (640-890 read; the first 640 lines, including the helpers, not read), `test_analysis.cpp` (190-430 read; the scan and rescan cases at 53-175 and 440-989 by title only), `test_replay.cpp` 130-1290 (read by title only), `test_fast_tempo.cpp` 240-827 (by title).

Helpers: of `corpus_util.h`, `chart_text.h`, `midi_util.h`, `multidiff_chart.h`, `song_digest.h`, `song_equal.h`, `sng_util.h`, `srb_util.h`, `dm_fixture.h`, `warp_util.h`, `bytes_util.h` I read none in full (I read `difficulty_literals.h` in full). I did check by grep that the hand-built MIDI helpers (`smf`, `note_on`, `text_event`) live in `midi_util.h` and are used by all the MIDI-building tests, so there is no sign of duplicated MIDI builders. I did not check `chart_text.h` against the chart builders in `test_s2_parser_owners.cpp:33` (`chart_bytes` there is a local wrapper with a different signature than `testchart::chart_bytes`, same name).

Not on my list, not read: `test_search.cpp`, `test_batch_text.cpp`, `test_strutil.cpp`, `test_winstr.cpp`, `test_display_format.cpp`, `test_utf8_paths.cpp`. I do not know whether another scout has them.

Source under `src/`: I did not read production code except by the tests' includes. The gaps in (e) come from test titles and a few greps, not from a branch-by-branch comparison with the source.
