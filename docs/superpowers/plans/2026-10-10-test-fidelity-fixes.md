# Test fidelity fixes: oracles that share production's mistakes, or prove too little

Planner TP-2, 2026-10-10. Source: the test-suite audit (`docs/audit/2026-10-10-test-suite-audit.md`) and the scout reports A-engine, B1-store-app and B2-preview-report. Every claim below was checked in the code before it went into a task; where the scouts were wrong or imprecise, the task says so.

## What is wrong, in one paragraph

Seven places in the suite look like protection but are not. The old-database upgrade tests build their "old" files from a column list that the upgrade itself uses, so both can be wrong together. The golden-image test averages colour error over a million pixels and passes at under 10/255 when the real error is 5.7, so a wrongly coloured lane would slip through; it also quietly passes if its fixture goes missing. The "skip a stem that won't open" rule is tested on a copy of the loop, not on `open_song_stems`. Two tests recompute their expected answers from the old rule (regexes in `test_song.cpp`, a formula in `test_fill_deadline.cpp`). The whole-corpus engine digest is pinned at one setting. Three tests fail on a slow machine instead of on a bug. And the one real-sound-card test only proves a constructor does not throw. This plan replaces each with a test that pins a literal from one run, or calls the real function, or both.

Every new number in this plan is a user decision; the questions are collected at the end with my recommendation and where each number comes from.

## How the work splits

Seven tasks. Tasks 2 to 7 are independent and can run in parallel in worktrees. Task 1 has a serial first half (building old releases, one cold build at a time) and a test-writing second half that can start as soon as the first fixture file exists. Each task is one Opus executor; one fresh Sonnet reviewer reviews the merge (rule 5). Nothing here runs the user's library; everything runs on `testdata/input` or hand-built files.

---

## Task 1: frozen real old databases for the upgrade tests

**What is wrong.** `downgrade_to_schema3` and `downgrade_to_schema2` in `tests/test_store.cpp` (lines 1168 to 1202) rebuild the old results table from `kSchema2ResultsColumns` and `kSchema2ResultsTableSql`, which `src/store/record_store.cpp` exports (lines 253 to 269). `upgrade_results_key` (line 770) copies rows across with the same `kSchema2ResultsColumns`. So the three "a schema N database keeps its results" cases (lines 1236, 1263, 1282) prove that the upgrade agrees with itself. There is no `.db` file anywhere under `testdata/` (checked with a glob). The one honest old fixture is `tests/old_layout_fixture.h`, whose table text is a literal, but it covers only the last layout before summary-only storage (D87), not schema 2 or 3.

I checked the one thing the synthetic fixture could be hiding: whether an old file's structure blob really starts with a 4-byte path format and then the 8-byte rules fingerprint, which `rules_fp_of("structure")` reads at byte 5. In v1.8.4's `record_store.cpp` the comment and `read_le(structure_head, 4, 8)` say yes, and the same in v2.0.0. So today's upgrade is right. The point of the fixture is that nobody has to re-derive that the next time the upgrade changes.

**Which old versions.** The schema history, from `git merge-base --is-ancestor` on the commits that changed the store:

| Schema | What it is | Landed in | First release tag |
|---|---|---|---|
| 2 | results keyed by the analysis settings, `PRAGMA user_version = 2` | 133b52e6 (2026-08-28) | v1.7.0 |
| 3 | `legacy_fills` joins the key, `user_version = 3` | 1f3efdf6 (2026-09-29) | v1.9.x; v2.0.0 holds it (its `record_store.cpp` has the column and `user_version = 3`) |
| 4 | `rules_fp` joins the key, the structure blob still stored | e56e2569 (2026-10-04) | v2.1.0 |
| summary-only | the blob and the four detail tables dropped (D87) | 6bddaa84 (2026-10-07) | v2.2.0 |

Recommended fixture set, four files: **v1.8.4** (schema 2, the release the user's own library ran on when the 1.8.3 Stale incident happened; see `stored-versions-one-rule`), **v1.8.4 with `--legacy-fills`** (schema 2 whose file-level fill stamp says Clone Hero 1.0, which `stamped_fill_rule()` reads during the upgrade), **v2.0.0** (schema 3), and **v2.1.0** (schema 4 with the detail tables, which also replaces the hand-written `old_layout_fixture.h` as the proof for the D87 upgrade; keep the header for the crash-mid-upgrade tests, which need a file they can seed and stop).

**How to produce them honestly.** For each tag: `git worktree add <scratch>\wt-v1.8.4 v1.8.4`, then from that worktree `.\build_cpp.ps1 -Target hydra_batch`. A cold build takes three to six minutes and the preamble allows one at a time, so the four builds run one after another (about 20 minutes; this is the serial half). Make a folder holding three charts copied from `testdata/input` (pick one `.chart`, one `.mid` and one `.sng`; name them in the README). Run the old `hydra_batch` from its own build folder, so its mimalloc DLLs are beside it: `build-cpp\Release\hydra_batch.exe --db <scratch>\v1.8.4.db <that folder>` (v1.7.10 and v1.8.4 both take `--db`; checked in `git show v1.8.4:src/cli/batch.cpp`). For the legacy file add `--legacy-fills`. Then fold the WAL into the file and shrink it with Python's sqlite3: `VACUUM INTO 'testdata/store/v1.8.4-schema2.db'` (VACUUM keeps `user_version`). Commit the file with a `README.md` beside it that records the tag, its commit hash, the exact command, the three charts, the file's byte size and the numbers the tests pin (below), so anyone can regenerate and compare. Old tags may not configure under VS 2026 (memory `vs2026-incomplete-instance`); the generator-instance workaround there applies. If a tag will not build in 10 minutes of trying, stop and report which one.

**What each new test asserts.** Copy the fixture to a temp path first (the store opens in WAL mode and would write `-wal`/`-shm` beside a checked-in file). Before opening: `PRAGMA user_version` equals the pinned literal (2, 3 or 4) and the columns that schema lacks are absent (`legacy_fills` for schema 2, `rules_fp` for 2 and 3). Open with today's `RecordStore` and a progress log, then check: the results row count equals the pinned literal (3, or 1 per chart that analysed; the README records it); the set of `result_id`s is unchanged; `rules_fp` on every row equals the fingerprint literal read out of the fixture's blob bytes 5 to 12 at fixture time (the README records the hex); `legacy_fills` is 0, or 1 for the `--legacy-fills` file; every result reads **Stale**, because those files stamp `hyversion` "1.8.2" or "2.1.0" and `kResultsStamp` today accepts only "2.4.0" (`src/store/stored_versions.h` line 62), and that is the honest answer, not Ready; the `charts` rows (3) keep their names; for v2.1.0 the detail-table count (`kDetailTablesCountSql`) is 0 and the `structure` column is gone; the open reported `UpdatingResultsKey` for schemas 2 and 3; and a second open of the same file reports no upgrade step and the same counts (stability, which B1 noted nobody checks).

**What goes away.** The three synthetic schema cases and `downgrade_to_schema2/3` are replaced by the fixture cases. `kSchema2ResultsTableSql` then has no reader (its own comment says the store test is its only one), so it leaves `record_store.cpp` and `record_store.h`. `kSchema2ResultsColumns` stays: the upgrade uses it, and `old_layout_fixture.h` does too.

**Failing first.** The new test with the fixture path wrong fails on its `REQUIRE(file exists)`, which is the loud failure the golden test lacks. Keep that red line for the review.

**Files.** `testdata/store/*.db` and `README.md` (new), `tests/test_store.cpp`, `src/store/record_store.{h,cpp}` (one constant removed), `CMakeLists.txt` only if the fixture folder needs a define (the tests already have `HYDRA_TESTDATA_DIR`). Size: half a day, most of it waiting for builds.

---

## Task 2: a golden-image check that would notice a wrong lane

**What is wrong, checked.** `tests/test_preview_golden.cpp` downsamples both images by 2 (line 196), sums absolute per-channel error over every unmasked pixel, and passes when the mean is under `tolerance` (line 212). `golden.json` sets 10; the code's fallback if the key is missing is 20 (line 151). The last full run measured 5.69 (`scratchpad\suite-durations.txt`). The frame is 1372 by 714; a drum lane is about a fifth of the width and the gems in view cover a few percent of the area, so recolouring one lane's gems moves the mean by well under 1. At line 140 a missing `golden.json` prints "skipped" and returns, and the test counts as passed. ADR 0008 says "re-capture the golden" and gives no steps; `golden.json`'s `_comment` is the only record (Onyx 20251011, the blink-182 chart, paused at 0:36.913, whole window, crop `[0, 126, 1372, 714]`).

**What the field does.** Skia Gold, which Chromium and PDFium use, matches exactly by default and offers a "fuzzy" mode with two knobs, a maximum number of differing pixels and a per-pixel channel-delta threshold, plus a Sobel variant that blanks out edges before comparing ([Chromium's wrapper](https://chromium.googlesource.com/chromium/src/+/main/ui/base/test/skia_gold_matching_algorithm.h), [goldctl's test](https://skia.googlesource.com/buildbot/+/6bb2146c9a8b8bfbf84f6fa415320a3ff522d222/gold-client/cmd/goldctl/cmd_match_test.go)); Gold also keeps several approved images per test rather than loosening thresholds ([Chromium GPU pixel testing](https://chromium.googlesource.com/chromium/src/+/da492f144feba8874ce6d25a9da170228763b21e/docs/gpu/gpu_pixel_testing_with_gold.md)). pixelmatch counts mismatched pixels above a per-pixel colour-distance threshold (default 0.1 in OKLab), skips anti-aliased edge pixels by default, and its `windowSize` option returns the worst count in any N by N window, which is the per-tile idea ([mapbox/pixelmatch](https://github.com/mapbox/pixelmatch)). jest-image-snapshot offers SSIM as a structural alternative to per-pixel counts ([its README](https://app.unpkg.com/jest-image-snapshot@6.4.0/files/README.md)). The common thread: a per-pixel delta threshold and a count or window budget, never a whole-image mean alone.

**The plan.** Keep the mean as a sanity line but add the check that matters: at half resolution, count pixels whose largest channel delta exceeds D, and fail if the worst 16 by 16 tile (32 by 32 at full size) holds more than P percent of such pixels, or if more than F percent of all unmasked pixels do. That is Gold's two knobs plus pixelmatch's window. A recoloured lane makes every gem tile in that lane blow past P; a few-pixel shift of all gems does the same at every gem edge. Gamma stays out of the metric (correcting the capture's gamma is a bigger change; offered as option C below).

The first step is a measurement, because D, P and F cannot be chosen blind. Add a dev aid under the existing `HYDRA_PREVIEW_GOLDEN_DUMP` env var that writes the per-tile error map as a CSV and the histogram of per-pixel max-channel deltas for the current golden. From that one run the executor reports: the current worst tile, the 99th-percentile delta, and what the three numbers would be with a 50 percent margin. The user picks from those. The test then pins them in `golden.json`.

Then: the fixture is checked in, so a missing `golden.json` or `golden_onyx.png` becomes `REQUIRE`, and the `tolerance` fallback of 20 goes (every key `REQUIRE`d present, so a deleted key cannot double the allowance). Add a `pixel_delta`, `tile_percent` and `frame_percent` key beside `tolerance`.

Recapture procedure: write `testdata/preview/README.md` with what is known today from `golden.json` (Onyx build, chart, Expert Pro Drums, the pause time, that the screenshot is the whole window and `crop` names the highway) and a numbered list of the steps that have to be true for the crop to be 1372 by 714 again. The window size and how the pause landed on exactly 36.913 are not recorded anywhere; the README marks them "to confirm at the next recapture", and ADR 0008 points at the README instead of saying "re-capture". The question of whether the user remembers those two steps is below.

**Failing first.** With D, P, F set, recolour nothing: the test is green. The executor proves the metric bites by rendering with `pro=false` (the test's own `render_chart` takes `pro`) and showing the tile check goes red while the mean stays under 10; keep that line for the review.

**Files.** `tests/test_preview_golden.cpp`, `testdata/preview/golden.json`, `testdata/preview/README.md` (new), `docs/adr/0008-...md` (one sentence). Size: half a day. Parallel with everything else.

---

## Task 3: stems through the real functions

**What is wrong, checked.** `tests/test_audio_mixer.cpp` line 139 opens each stem in its own try/catch and counts failures; `src/audio/song_audio.cpp` line 47 is the loop that does it for real, and nothing in `tests/` calls `map_song_stems`, `stems_total_bytes`, `open_song_stems` or `mix_song_stems` (grep over `tests/`: only `test_single_owner.cpp` names them, as text). The load-job tests in `test_preview_load_progress.cpp` and `test_preview_controller.cpp` feed only good audio.

**The plan.** A new `tests/test_song_audio.cpp` with four cases that call the real functions, on the five existing `testdata/audio/sine220.*` fixtures and junk bytes typed in the test; no new binary fixtures.

One: `map_song_stems` on a file stem that exists, a file stem whose path does not exist, and a bytes stem, returns mapped, empty and owned in that order, and `stems_total_bytes` equals the sum of the two real sizes, pinned as literals (the executor reads the two fixtures' byte sizes once with `Get-Item` and pins them; the README-style comment says which file each number is).

Two: `open_song_stems` on [ogg file, junk bytes "not audio", mp3 bytes] returns two readers; the progress callback's last call is `(total, total)`; and a callback that returns false on its first call makes it throw `OpenCancelled`.

Three: `mix_song_stems` with offset -250 ms gives `audio_offset_ms` 0, a mix 12,000 frames longer than the one-stem mix (250 ms at 48 kHz; the number the load-job test at line 338 already pins), and an `end_chart_ms`; with no readers at all, `end_chart_ms` is empty.

Four, the load-job case: a temp chart folder built with `audiochart` helpers holding `song.ogg` (the sine) and `drums.ogg` whose bytes are junk. The job must finish ok and its audio length must equal a plain one-stem `StreamMix` of the sine, which proves the junk stem was skipped and the good one survived. A second folder holding only the junk stem: the job finishes ok with no audio end.

The copy-loop case at line 124 is deleted. The one at line 173 (StreamMix vs decode-then-mix) stays; it is about mixing, not skipping.

**Failing first.** Case two red by temporarily making the junk stem a `REQUIRE`d third reader; keep the line. **Files.** `tests/test_song_audio.cpp` (new), `tests/test_audio_mixer.cpp`, `CMakeLists.txt` (add the file). Size: a few hours.

---

## Task 4: pinned literals instead of recomputed oracles

**test_song.cpp lines 604 to 727.** Two disco cases compare the hand-written matcher against `std::regex` copies of the rule it replaced, over nine marker shapes times 256 bytes times two prior states (about 4,600 parses each). The section-header case does the same with `\[.*\]` over 21 lines. These were right as migration tests and are now a second home for the rule.

Replace the regex oracles with tables produced once from them. For the disco cases: for each of the nine shapes in `disco_candidates`, the set of bytes that make it "on", the set that make it "off", stated as byte ranges in a small table (the executor generates it with a throwaway script run against the two regexes, records the date in the comment, and deletes the script). The 256-byte parse loop stays; it reads the expected answer from the table. For the header case: a three-column table of line, outcome (throws ChartFileError, parses with resolution 192, or throws out_of_range), taken from one run of today's code. The regexes are deleted. Each literal's source is the comment: "pinned from the regex oracle on 2026-10-10, commit X".

**test_fill_deadline.cpp.** Line 63 checks the CH 1.0 lead equals `3750.0 / bpm`, the formula. Replace with a table of the five tempos and the lead each gives, pinned from one run (they are 53.571428571428571, 37.5, 26.785714285714285, 15.625 and 9.375 ms for 70, 100, 140, 240 and 400 BPM; the executor confirms by running, not by arithmetic). Lines 91 and 104 restate `fend - fill_len_ms - 250` and `- 10000`; the literals on lines 93 and 106 already pin the answer, so the formula lines go. The header's "worked out from the formula" sentence changes to say the literals came from one run.

**Files.** `tests/test_song.cpp`, `tests/test_fill_deadline.cpp`. Size: a few hours. Failing first: flip one table entry, show red, flip back.

---

## Task 5: the engine digest at the settings that change the engine

**Checked.** `tests/test_perf_digest.cpp` pins the engine digest once, at Expert, Pro, 2x, cap 4, scores depth 4, 10 ms limit; the parse digest is pinned there and at Hard, no Pro, no 2x. The user's own `hydra_settings.ini` (`C:\Program Files\Hydra\hydra_settings.ini`) is Expert, Pro, 2x, scores 4, ms limit 10, cap 4, legacy fills off, backend limit 100, hit window 85, so the user runs the defaults; backend limit and hit window are display-side and do not reach the prepared row.

The settings that change what the engine computes (from `app::Settings` fields and `batch_run`): difficulty, Pro Drums, 2x Bass, Note Shuffle, SP cap, depth mode and value, the ms limit, and the CH 1.0 fill rule (`legacy_fills`, CLI only, ADR 0010). Recommended additional pins, three engine digests: **Hard, no Pro, no 2x** (so the engine and parse pins share a second setting); **Expert Pro 2x with the CH 1.0 fill rule** (`test_rules.cpp` covers that rule on one chart today); and **Expert Pro 2x with Note Shuffle on and the ms limit off** (Note Shuffle is new, merged 2026-10-09, and the limit-off path has no corpus pin). Each run costs about what the existing one does (0.08 s in the duration file). Also count successes inside `engine_digest` and `CHECK(analyzed == corpus size)` beside each pin, so a repin can never hide that every chart started failing (A-engine finding 3; same file, small).

**Files.** `tests/test_perf_digest.cpp`. Failing first: the executor pins each new digest from one run, then changes one literal's last digit to show the red message, then restores it. Size: an hour plus run time.

---

## Task 6: no more wall-clock assertions

**Checked.** `tests/test_library_model.cpp` line 393 fails if the best of five filter passes takes 20 ms or more; line 392 already prints `m.order().size()`. `tests/test_library_query.cpp` line 379 fails at 20 ms; line 378 checks only `hits > 0`. `tests/test_path_view.cpp` line 718 requires 600 cached frames to be ten times faster than 600 rebuilds, and the case at line 650 already proves the cache builds once per input change by counting.

Both 20 ms checks become count checks: pin the row count each of the nine queries returns against the 20,000 synthetic rows (literal per query, from one run) and the hit count of `artist 12 "tier 3"` (one literal). The timing stays as a `MESSAGE`, the way `test_store.cpp` line 1895 already does. The ratio test is deleted; if the user wants the cache speed watched, `hydra_bench` is where a timing belongs, so the option below offers moving it there instead.

**Files.** the three test files. Size: an hour. Failing first: a wrong literal.

---

## Task 7: the real-device test should prove the device played

**Checked.** `tests/test_audio_device.cpp` constructs the device, calls `start()`, and resets it. `start()` in `src/audio/device.cpp` sets `started` only when `ma_device_start` returns success and swallows failure (line 76); `started` is private. So a WASAPI that opens but will not start passes. CI excludes this one case by name (`.github/workflows/ci.yml` line 43), correctly: it is the only hardware test. `set_headless(false)` is never put back.

Recommended: keep it and make it prove playback. `PreviewAudioDevice` gains `bool started() const`, the test asserts it after `start()`, and the test's source callback counts calls in an atomic; the test waits up to a cap for the count to pass zero (the device thread pulled audio) and fails loudly if it never does. The old headless value is restored by a guard. The alternative, deleting it, loses the only proof that the WASAPI-only miniaudio build works; since every dev machine has a sound card and CI already skips it, keeping it costs nothing.

Flag, not in scope: `start()` hiding a failed `ma_device_start` from the user is a production behaviour. The Preview would show Play with no sound. Whether that should surface is a separate question for the user.

**Files.** `tests/test_audio_device.cpp`, `src/audio/device.{h,cpp}` (one accessor). Size: an hour. Failing first: assert `started()` before calling `start()`.

---

## User decisions

1. **Which old releases become fixtures (Task 1).** Options: (A) v1.8.4, v1.8.4 `--legacy-fills`, v2.0.0, v2.1.0 (recommended; one per schema, plus the one file-level fill stamp the upgrade reads); (B) v1.8.4 and v2.1.0 only; (C) also v1.6.2 to pin "a pre-schema-2 file opens empty and is left alone" (`test_store.cpp` line 712 does this synthetically today).
2. **Fixture size limit (Task 1).** Three charts per file. Old layouts stored about 15 KB per chart (D87: 292 MB for 18.8k charts), so each file should be well under 256 KB after `VACUUM INTO`. Recommended cap: 256 KB per file, 1 MB for the set; if a file comes out bigger, drop to two charts.
3. **Golden thresholds (Task 2).** Decided after the measurement step reports the current worst tile and delta percentiles. The shape I recommend: per-pixel delta D (Gold's channel-delta knob), worst-tile budget P percent, whole-frame budget F percent, and the mean tightened from 10 to the measured 5.69 plus a margin (7 would give 23 percent headroom). Option C: correct the capture's gamma first and then set a tighter mean near 2 (bigger change, nicer numbers).
   **Decided by the user on 2026-10-10:** D = 32 (a pixel is over when its largest channel delta, at half resolution, exceeds 32/255); worst 16 by 16 half-res tile at most 9.4 percent over; whole frame at most 0.08 percent over; mean tolerance tightened from 10 to 7. Evidence (TF-T2a measurement at 2c8ea2c7): today's render worst tile 6.25 percent, frame 0.0525 percent, mean 5.688; the pro=false render worst tile 32.8 percent, frame 0.125 percent, mean 5.727. The limits are today's numbers plus 50 percent, rounded.
4a. **Recapture steps:** the user does not remember the Onyx window size or how the pause landed on 0:36.913 (2026-10-10); the README marks both "to confirm at the next recapture".
8a. **A failed `ma_device_start`:** the user said yes, show it (2026-10-10); a separate follow-up session designs the message.
4. **Recapture steps (Task 2).** Do you remember the Onyx window size and how the pause landed on 0:36.913? If yes, they go in the README now; if not, the README says "confirm at the next recapture".
5. **Which settings to pin the engine digest at (Task 5).** Recommended three above. Option: add cap 8 or points depth as a fourth. Each pin adds about 0.1 s to the suite.
6. **Timing tests (Task 6).** (A) counts replace both 20 ms checks and the ratio test is deleted (recommended); (B) same, but the ratio check moves into `hydra_bench` as a printed line.
7. **Device test (Task 7).** (A) keep and strengthen with a wait cap of 2 s for the first callback (recommended; a WASAPI device calls back within tens of ms, so 2 s is a hang detector, not a timing test); (B) delete it.
8. **Surface a failed `ma_device_start` to the user?** Not in this plan; yes or no decides a follow-up.

## Risks

Old tags may not configure under the installed VS 2026 instance; the known workaround is in memory, and the task has a 10-minute stop per tag. The old `hydra_batch` is run from its own build folder so mimalloc's DLLs are found (memory `mimalloc-link-order`). The golden thresholds depend on one measurement on one machine (WARP, so no GPU variance; the capture itself is fixed). The disco table is big (nine shapes by 256 bytes); stating it as ranges keeps it readable, and the 256-byte loop still runs so nothing is lost. Task 1 removes a production constant; the single-owner scan may have a row naming it, which the executor updates.

## Rough size

Task 1 about half a day (mostly builds); Task 2 half a day; Tasks 3 and 4 a few hours each; Tasks 5, 6 and 7 about an hour each. Tasks 2 to 7 run in parallel; Task 1's fixture half runs alone, then its tests join the wave.

## What done looks like

Four real old `.db` files live under `testdata/store/` with a README that says how they were made, and the upgrade tests open them and pin counts, ids, fingerprints and Stale status; `kSchema2ResultsTableSql` is gone from production. The golden test fails loudly without its fixture, checks worst-tile and pixel-count budgets the user chose, and the recapture steps are written down. `open_song_stems`, `map_song_stems`, `stems_total_bytes` and `mix_song_stems` each have a test, and a load job with a junk stem beside a good one plays the good one. No test in the suite recomputes its expected value from a regex or formula in these two files. The engine digest is pinned at four settings and counts its charts. No test fails on wall-clock time except the one loopback-HTTP cancel case the audit chose to keep. The device test proves the device started and pulled audio.
