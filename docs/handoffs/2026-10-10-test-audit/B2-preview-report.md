# B2 audit: preview, report, drawing/layout and audio tests

Scout: afc0d43f80ad193ca. Task TA-B2. Read-only. About 52 tool calls.

The short version: this area is in good shape. The tests mostly pin what the user sees, use hand-worked numbers with the arithmetic explained, and run fast (nothing in my files takes more than 0.7 s in the timing run; the one 18 s test in the Preview area lives in `tests/test_track_state.cpp`, which is outside my scope, so another scout should look at it). The real problems are a few weak spots, not a rotten core. The three that matter most: the golden-image test is too forgiving to catch a small drawing regression, the "skip a stem that will not open" rule is tested against a copy of the loop inside the test rather than the production code, and the Opus "cancel promptly" test can pass without ever reaching the code it names.

I read every file's test list. In depth I read: `test_preview_golden`, `test_audio_device`, `test_audio_player`, `test_audio_mixer`, `test_stream_mix`, most of `test_stem_reader` and `test_audio_decode`, `test_preview_load_progress` (all), `test_preview_clock`, `test_preview_config`, `test_theme`, `test_activation_row_layout`, much of `test_preview_transport`, `test_preview_controller` (about 40 percent), `test_overlay_layout` (first half), `test_highway_draw` (first 330 lines), `test_preview_view` (lines 1-1115 and the shape of the rest), `test_path_view` (about 35 percent), `test_report` (about 55 percent), `test_report_view` (the column and cell tests), the start of `test_preview_source`, and the helpers `display_fixtures.h` and a skim of `bank_check.h`.

---

## (a) Working well, keep

**The SP meter tests in `test_preview_view.cpp` (lines 916-1340).** Each one builds a tiny song by hand, states the arithmetic in a comment ("two bars is four measures: 8000 ms at 120 BPM 4/4"), and checks the meter at exact moments, including one microsecond either side of a jump. A real bug in how the meter banks or drains a bar would fail these immediately. They are the best tests in the area. The expected numbers are literals worked out by hand, not a re-run of the production formula. The regression cases (lines 1038 and 1091) say which real bug they came from.

**Base plus overlay equals the whole scene (`test_preview_view.cpp:838` and `:892`).** The Preview builds a scene in two halves for speed, and this test proves the two halves glue back to exactly the one-shot answer on hand-built songs and on every corpus chart and stored path. If someone changes one half and forgets the other, this fails and names the member that differs (`scene_difference`, line 714). Strong and cheap in spirit.

**Strings the user reads are pinned as literals.** `test_path_view.cpp` (for example the scale-warning wording at lines 313-423), `test_report_view.cpp` (cells), `test_preview_load_progress.cpp` (loading bar labels, "about 3:05 left") all pin the exact text. A wording regression shows up as a readable diff.

**Fake clocks instead of real time.** `test_preview_clock.cpp`, `test_preview_transport.cpp` and `ByteRateClock` in `test_preview_load_progress.cpp:240` all drive time with a fake `t` they advance by hand. No sleeps, exact values, no flakiness. This is the house style done right.

**The 300 MB Opus cancel test measures bytes, not milliseconds** (`test_preview_load_progress.cpp:289-294`). The comment explains a busy machine slows the stop but never moves where it happens. Good thinking (but see (b) for a hole in it).

**Controller tests never open a real sound device.** Every `PreviewController` test that loads audio sets a fake device factory (`no_device_factory`, `counting_factory`). I checked all 15 construction sites in `test_preview_controller.cpp`: the ones with no factory (lines 436-474, 674, 773, 788) either call `open_and_load` (which sets one) or load charts with no audio, so no device is opened. The only real-device test is the one CI excludes.

**Bit-for-bit and differential audio checks.** `test_stem_reader.cpp` and `test_stream_mix.cpp` compare the streaming readers to a whole-file decode and to the old mix, and `test_stream_mix.cpp:361` proves the audio callback never allocates memory. A seek bug, a resampler block-edge bug or a stem that stops early would all fail here.

**WCAG contrast tests** (`test_theme.cpp`, `test_report.cpp:1108`). They check a real, user-visible property (readable text) against the actual colour tokens. A colour tweak that hurts readability fails.

---

## (b) Broken or misleading

**1. The golden-image test is much too forgiving. Severity: high.**
`test_preview_golden.cpp:151` and `:212`, `testdata/preview/golden.json:10`. The test compares the whole 1372 by 714 frame and passes if the mean per-channel error is under 10 out of 255. The last full run measured 5.69 (`suite-durations.txt`, line 2842), and ADR 0008 says the baseline is about 5.7 "almost all of it the capture's gamma". So roughly 4.3 of headroom, which is 75 percent of the baseline error, is free for a regression to hide in. The error is averaged over about a million pixels, and a highway is mostly floor and background. A whole lane of gems drawn in the wrong colour, or every gem shifted a few pixels, changes only a small slice of the frame and moves the mean by well under 1. So the one test that claims "the Preview is a faithful port of Onyx" would pass most small drawing bugs. The bigger bugs (missing floor, wrong camera) it would catch. `test_preview_renderer.cpp` has a few pixel checks, but they are coarse too ("more than a quarter of the frame is highway").
Fix: keep the mean check but also compare per tile (for example 32 by 32 blocks) and fail if any unmasked block is far off, or correct for the known gamma offset and tighten the tolerance to around 2. At minimum, tighten 10 toward the real 5.7 plus a small margin.

**2. The "skip a stem that will not open" rule is tested against a copy of the loop, not the production code. Severity: medium-high.**
`test_audio_mixer.cpp:124-158` (and again at `:173-205`). The test opens each stem in its own `try/catch`, counts the failures, and then checks that the good ones mix. But the real rule lives in `open_song_stems` (`src/audio/song_audio.cpp:41`), which the Preview load job calls. I grepped the whole tests folder: `open_song_stems`, `map_song_stems` and `mix_song_stems` are never called by any test (only a text scan in `test_single_owner.cpp` mentions the names). The load-job tests I read (`test_preview_controller.cpp`, `test_preview_load_progress.cpp`) never feed the job a corrupt stem either (I grepped for "broken", "junk", "corrupt" in `test_preview_*.cpp`: no hits). So a bug in the real skip loop, for example letting one bad stem silence the whole song, would not fail any unit test. The test's own comment even says "the job's open loop", which is the loop it re-implements.
Fix: call `open_song_stems` with a mapped bad stem (via `map_song_stems`) and assert the good stems still open, and add one load-job case with a corrupt stem beside a good one.

**3. The Opus cancel test can pass without testing the cancel. Severity: medium.**
`test_preview_load_progress.cpp:271-299`. It sleeps 50 ms, checks the job is not finished, cancels, then checks the job stopped within 8 MB of progress. If the job is still in the "Reading chart" step at the 50 ms mark (slow disk, busy machine, antivirus), `bytes_done` is 0 at cancel and 0 at stop, so `at_stop - at_cancel <= 8 MB` is trivially true and the Opus open-loop cancel is never exercised. Nothing asserts that the job had reached the Opening step. It is also a Sleep-based "has the work started" guess, which the house rule says to avoid.
Fix: before cancelling, wait (with a cap) until `job.progress().bytes_done > 0`, and fail loudly if it never gets there. Also delete the 300 MB file in a guard object so a failed REQUIRE does not leave it in temp.

**4. The CI exclusion hides a weak test, and the device test proves less than its name says. Severity: medium.**
`.github/workflows/ci.yml:43` excludes "PreviewAudioDevice opens and starts the default output device" because hosted runners have no sound card. I checked the rest of the suite for the same hardware dependence and found none: the only other path that touches a real device is `PreviewController` without a factory, and I found no such test. So the exclusion is exactly one test, correctly. But that test (`test_audio_device.cpp:16-26`) only requires that the constructor does not throw. `start()` swallows a failed `ma_device_start` (`src/audio/device.cpp:76`: it only sets `started` when it succeeds, and `started` is private), so a broken WASAPI start still passes. The comment says "proves that backend still opens and starts" but it proves "still opens". Also it calls `set_headless(false)` and never puts the old value back (low risk today; nothing in `hydra_tests` sets headless to true, only the GUI harness does).
Fix: expose `started()` (or return the start result) and assert it; restore the previous headless value.

**5. The `suite` header comment in the golden test points to a procedure that does not exist. Severity: low.**
`test_preview_golden.cpp:5-6` says "captured by hand from Onyx, see docs/adr/0008 for the procedure". ADR 0008 (lines 21-29) says to "re-capture the golden" but has no steps (no Onyx version, window size, pause point or crop recipe beyond what `golden.json:2` says). Also the test's default tolerance in code is 20 (`:151`) while the file says 10, so a deleted `tolerance` key silently doubles the allowance.

**6. An escaped zero byte truncates a test input. Severity: low.**
`test_preview_source.cpp:90`: `bytes_of("OggS\x00\x02")` builds a `std::string` from a C string, which stops at the zero, so the input is just `"OggS"`. The check (`CHECK_FALSE`) still passes, but it is not the "Ogg page header with no codec name" it looks like.

---

## (c) Unnecessary: delete or merge candidates

**1. Two tests that are not tests.** `test_path_view.cpp:622` ("find a chart with an uncounted squeezed-out row") and `test_report.cpp:1028` ("report pages: write samples for the browser check") are marked `doctest::skip()` and exist only to be run by hand. They are developer tools living in the test binary. The first just prints a chart name; the second writes HTML. Severity: low. Move them to `tools/` (or delete, since the GUI test names the chart it needs). Likewise `test_preview_golden.cpp:215` "preview dump (dev aid)" runs on every suite pass, returns at once when its env var is unset, and shows as a passing test.

**2. The 600-frame cache timing test. Severity: medium.** `test_path_view.cpp:691`. It times 600 rebuilds against 600 cached reads with a wall clock and requires a 10x gap. The previous test (`:650`) already proves the cache builds once per input change by counting builds, which is the actual guarantee and is deterministic. The timing test adds only a flaky-under-load check on top. Delete it.

**3. `check_cap` with a parameter nobody varies. Severity: low.** `test_report.cpp:174` and `:285`: the helper takes a `cap` but has exactly one caller, with 4. It reads like a leftover from a multi-cap loop. Inline it.

**4. Overlap between audio tests. Severity: low.** `test_audio_mixer.cpp:173` ("StreamMix of real stems matches decoding every stem then mixing") repeats `test_stream_mix.cpp:162` and `:302`, which already compare StreamMix to decode-then-mix on the same fixtures. `test_stem_reader.cpp:403` and `:416` feed the same junk bytes to `open_stem_reader` and check two properties of the same throw; one case would do. `test_preview_transport.cpp:335` ("StreamMix's front pad") belongs in `test_stream_mix.cpp`, which already has a front pad case (mix case 4 at line 106).

**5. Preview-target release is tested twice. Severity: low.** `test_preview_renderer.cpp:256` ("released targets are freed and come back drawing the same") and `test_preview_controller.cpp:404` ("closing the Preview frees its render targets") both check the GPU targets are freed and come back. They test different layers (renderer vs controller), so keep both only if the controller one is about the controller calling release; otherwise keep the controller one and drop the renderer one.

**6. Tautology tests. Severity: low.** `test_path_view.cpp:425` (the hint string compared to the same sentence retyped), `test_theme.cpp:51` (`&kDisabledInputTextColor == &kDimTextColor`, which checks a variable is an alias of another) and `:121` (the gold colour compared to its own three numbers). They fail only when someone edits the constant on purpose. `test_theme.cpp:81-82` checks `>= 3.0` and then `>= 4.5` on the same value; the first line can never be the one that fails. `test_theme.cpp:42-45` checks only one colour component of each style slot (`.y` or `.x`), so a wrong red or blue would pass.

**7. Copy-text change detectors. Severity: low-medium.** `test_report_view.cpp:276-320` and `:391-440` pin every column tooltip sentence "word for word" (about 30 long strings). These test that a constant equals itself. The ids, order and numeric flags are real structure and worth pinning; the full tooltip paragraphs are not behaviour. Pin a couple, and let wording edits happen without touching tests. If the wording must be locked by decision, keep them but accept they will break on every reword.

---

## (d) Poorly designed

**1. Tests that read JavaScript and CSS as plain text. Severity: medium.** `test_report.cpp:892-927`, `:1128-1151`. For example `script.find("words.every(w => r.search.includes(w))")` and `css.find("thead { display: table-header-group; }")`. These prove a string is in the source, not that the page works. A harmless rewrite of the script (a different but equivalent expression) fails them, and a real bug in the script's logic passes them. The WCAG test at `:1108` is the good version of this idea (it reads the real colour values and measures contrast). Fix: where behaviour matters (search folding), pin it through a C++ function that builds the search text (as `path_search_text` at `:609` does) and keep the page-source checks to a minimum, or run the script in the GUI/browser check.

**2. Giant files and giant single tests. Severity: low-medium.** `test_preview_view.cpp` is 2328 lines with 86 cases, `test_path_view.cpp` 1337 lines with 49, `test_report.cpp` 1236 lines mixing the report data, the tiles, the fill page shell and CSS checks. Single cases are big too: `test_path_view.cpp:313` runs eleven separate scenarios in one case, and `test_highway_draw.cpp:175` is a 110-line "order and geometry" case. When the first scenario fails the later ones still run (CHECK, not REQUIRE), but the failure name is vague. Fix: split `test_preview_view.cpp` into scene, SP meter, boxes (time/score/drain/next-activation); use `SUBCASE` for the scenario lists; move the fill-page and CSS checks out of `test_report.cpp`.

**3. `scene_difference` is a hand-kept field list. Severity: low.** `test_preview_view.cpp:714-817` compares `PreviewScene` member by member. I checked the struct (`src/app/preview_view.h:226-263`) and today every top-level member is compared. But inside nested structs (`PreviewNote`, `PreviewFill`, `PreviewActivation`) only the listed fields are compared, so a field added later escapes the "base plus overlay equals whole" guarantee silently. Fix: give these structs `operator==` (defaulted) so the compiler keeps the list complete.

**4. The Opus fixture and the long-Opus helper hand-roll Ogg paging. Severity: low.** `test_preview_load_progress.cpp:76-158` writes its own Ogg CRC table and page splitter, while `test_stem_reader.cpp:190-221` already builds Ogg pages with libogg. Two ways of building the same thing in tests. Fix: put one libogg-based "repeat this Opus file to N bytes" helper in `audio_util.h`. Likewise `make_ramp` is copied between `test_audio_player.cpp:19` and `test_preview_transport.cpp:31` (its comment even says "as in test_audio_player"), and `flac_with_unknown_length` is declared by hand in `test_stream_mix.cpp:323` and defined in `test_stem_reader.cpp:528` instead of living in the shared header.

**5. Pixel and draw-order pinning in `test_highway_draw.cpp`. Severity: low-medium.** Cases like `:175` check the draw order (flats, then boxes, then beat lines) and exact box coordinates (`-1.09`, `-0.85`, `0.13`). That is on purpose here: the Preview is a port of Onyx, ADR 0008 says keep Onyx's numbers, and the coordinates come from the shipped config. So this is defensible. The cost is that retuning one config number breaks many tests at once (the config is loaded in `shipped_preview_config()`). Fix: keep `test_preview_config.cpp` as the one place that pins the config values; in the draw tests derive the box edges from `cfg` (`cfg.track.railing_x_width`) so a deliberate retune fails one test, not twelve.

**6. A global `operator new` replacement lives inside one test file. Severity: low.** `test_stream_mix.cpp:43-52` replaces the allocator for the whole `hydra_tests` binary to count allocations. It is a plain malloc pass-through, and the mimalloc redirect still applies underneath, so I found no real harm, but every other test now runs with it and nothing says so outside this file. Worth a one-line note in the test README or a dedicated allocation-counting test target.

**7. Formula restated in a test. Severity: low.** `test_overlay_layout.cpp:118-122` recomputes the room a box has as `highway_span_at(...).left - b.gap` and checks `b.left_w * s <= left_room`. That restates the rule `overlay_scale` is supposed to apply, using the same building block, so it can only catch a mistake in `overlay_scale`'s use of it, not a wrong rule. It is a property check (the box fits), which is acceptable, but it is the one place in my area where an expected value is derived instead of pinned. Also `test_overlay_layout.cpp:33` and `:39` compare `aspect` to `1920.0f / 1080.0f` and `600.0f / 700.0f`; those are the formula, retyped.

**8. Weak assertions in decode tests. Severity: low.** `test_audio_decode.cpp:110-114`, `:120-124`, `:152-156`: `channels >= 1`, `sample_rate >= 8000`, "at least 0.1 s" and a frequency within 7 percent of 220 Hz. The fixtures are known files (44.1 kHz, per the header of `test_stem_reader.cpp`), so exact channel count, rate and frame count can be pinned as literals. The Opus case already does this (`:162`) and it is the strongest of the four. The frequency tolerance of 7 percent would accept a 235 Hz tone, so a wrong-speed decode (resample bug) could slip through the Vorbis and MP3 cases.

---

## (e) Gaps

**1. `open_song_stems`, `map_song_stems`, `stems_total_bytes`, `mix_song_stems` have no unit test. Severity: medium-high.** See (b)-2. These are the production owner of "which stems play, in what mix, with what front pad" and the loading-bar byte total. They are exercised only indirectly through full load-job runs, where the failure path is never taken.

**2. Controller actions are covered by GUI tests only.** `PreviewController::jump_activation`, `seek_activation`, `step_ticks` and `set_scrubbing` appear in `tests/ui/uitest_preview.cpp` but in no unit test (`step_ticks`'s core, `step_tick_ms`, is unit tested in `test_preview_view.cpp:1389`; the controller wiring is not). Severity: low. The GUI suite does cover them, so this is only an observation about where the safety net is.

**3. The golden test covers one chart at one moment.** Only `notes.mid` at 36.913 s with Pro Drums and 2x Bass on. No check with Pro Drums off, an SP window active, a fill lane lit, or the SP end marks, all of which are Hydra's own additions (not Onyx's) and so have no Onyx reference anyway. `test_preview_renderer.cpp:136` does a coarse "pixels change" check for SP. Severity: medium, tied to (b)-1.

**4. `last_drawn_note`** (`src/app/preview_view.h:269`) appears to have no direct test, only use inside other functions. Severity: low.

**5. A hardware-dependent test is a single point.** Since the only real-device test is excluded in CI, CI never proves the WASAPI-only miniaudio build opens a device. That is inherent. A local run is the only check, and it only checks "does not throw" (see (b)-4). Severity: low-medium.

---

## (f) Files not reached

Not read in any depth, only their test lists: the second half of `test_path_view.cpp` (lines 730-1337: activation rows, squeeze sentences, badge, `PathsTabUi`), the second half of `test_preview_view.cpp` (lines 1115-2328: score box, drain box, next-activation box, time-box boundaries, SP meter corpus cases), `test_highway_draw.cpp` lines 330-813 (the SP phrase, fill lane and SP end mark cases), `test_report.cpp` lines 300-548 and 620-890 (the generate_report seam, cancel, reuse and failure cases), `test_report_view.cpp` search/sort cases (lines 1-270) and the dm comparison cells, `test_overlay_layout.cpp` lines 140-283 (word wrap, ellipsize), `test_label_layout.cpp`, `test_preview_source.cpp` lines 130-513 (container fixtures, delay/offset rules), `test_preview_controller.cpp` lines 140-380 and 696-817, the second half of `test_stem_reader.cpp` (FLAC and damaged-Opus cases, lines 530-621), `test_preview_renderer.cpp` lines 120-331, and the helpers `preview_config_util.h`, `audio_util.h`, `audio_chart_fixtures.h`, `report_samples.cpp/.h`, `wcag_util.h` and most of `bank_check.h`. Also not opened: `testdata/preview/notes.mid`, `golden_onyx.png` and the `testdata/audio` files (their size and contents were not checked).

Outside my scope but spotted: `tests/test_track_state.cpp:871` ("rebuild_overlay_fields: equals a full build on every corpus chart and stored path") takes 18.2 s, 80 percent of the whole suite's slowest tail, and its twin at `:823` takes 1.5 s. Whoever audits render or the engine tests should look at whether the corpus case can use a subset of charts.
