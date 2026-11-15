agent a8512b21848207d76 (verifier T5-V-RP1, checks `repeats/RP1.md`, rows 1 to 69)

## Short answer

The sorter's work holds up. I agree with all seven candidates. I dispute two copies inside RP1-1 and I add one K2 part and several copies. No K3 or K4 row I opened hid a K1 or K2 part, except the two small ones named below. I read cross-findings.md first: X1 covers Burnout's score, path text and "163.0 ms"; X2 (fill ticks) does not touch any RP1 row.

## What I checked

Rows with more than 13 files are rows 1 to 7. The sorter judged their unopened files from a sample. I printed the asserted line of every place in those rows (about 350 lines) and read them all. In rows 2, 3, 4, 6 and 7 every place is a hand-built tick, a layout size, a wait in seconds for a UI test, or a count the test sets. One place in row 1 and one place in row 5 are product constants (below). Nothing in them is an engine answer copied from a real chart.

I also opened every place of every K1 and K2 row (16, 20, 24, 31, 37, 45, 57, 39) and the places of 9 K3 or K4 rows: 12, 18, 21, 25, 33, 42, 50, 52, 54. With rows 1 to 7 that is 16 of 69 rows, more than one in five, spread from the first to the last third.

## Disputes and additions, in plain words

**Dispute (copies in RP1-1).** The sorter lists `test_overlay_layout.cpp:342` and `:510` as copies of Burnout's best path. They are not. Line 340 passes the string `"  3- 1 2  (optimal)"` to a word-wrap function, and line 505 builds `"3- 1 2  (optimal)"` to test text trimming. The test makes the text up; it only happens to look like Burnout's path. That is K3. Changing the engine's answer would not turn these red. Remove them from the copy list (X1 also lists `test_overlay_layout`, so X1's file count of 10 is too high by one for the same reason).

**Weaker copy.** `test_sp_meter_curve.cpp:948` looks up a path from the engine by its text, `path_named(all_paths, "3- 1 2")`. It is a real use of the engine's answer, but as a lookup key. If the best path's text changes, the lookup fails. Keep it, marked as a key and not an assertion.

**Added copies in RP1-1.** `test_pb_coach.cpp:66` also types 364675 twice and the comment says "13,640" (Burnout's cost of skipping the first window), all from "one run". These are the same kind as 378315. Also, the sorter's row 16 K3 parts are right: `test_activation_rows.cpp:579` and `:755` build their own value (`format_ms(163.0)`, and a made-up `SPSqueeze` of -163.0), so they are K3.

**Added K2 part (row 5 and row 10).** `ui/uitest_paths.cpp:408` and `:410` assert `settings.backendlimit_value == 500` after typing 500 and 600. The comment on 406 says "The full engine window (500 ms) is reachable; beyond it clamps back." The 500 is the cap of that setting. Owner: `src/app/config.cpp:195` (`number("backendlimit_value", ..., 0, kWindowMs)`), where `kWindowMs` is `kSqueezeWindowMs` (`src/core/model.h:121`, copied at `config.cpp:158`). A change to the window turns this red once. One copy in the UI tests plus the pin at `test_replay_schedule.cpp:356` already in RP1-6. Add to RP1-6.

**Added K2 part (row 1).** `test_store_open.cpp:1284` and `:1292` assert `wal_autocheckpoint_for_test() == 1000`. Owner: the literal `PRAGMA wal_autocheckpoint=1000` at `src/store/record_store.cpp:1115`, with no named constant. Two copies in one file, so it is a single pin like those in RP1-6. Add to RP1-6. Note the owner is itself an unnamed literal.

**Added K2 part (row 5), copy outside `src/`.** `test_docs_match_code.cpp:2024` types `kDocImageMaxBytes = 500 * 1024`. Its comment says `tools/doc_shots/shrink.py` fails a capture run on the same limit (`shrink.py:20 MAX_FILE_BYTES = 500 * 1024`) and names the user's decision D300. Two copies, one in a tool and one in a test, each pointing at the other. Not a `src/` constant, so not K2 by the brief's definition. I list it so the main session sees it. It is not a repeat of 3 or more.

**Minor.** `test_config.cpp:414` (`clamp(early_fill_window_ms, 200) == 85`) and `:410` (`clamp(preview_volume, 150) == 100`) are pins of clamp results; the 85 and 100 belong to other rows. The 200 and 150 are the test's own inputs, K3, as the sorter said.

## Candidates

| Candidate | Verdict | Notes |
|---|---|---|
| RP1-1 Burnout score, path, squeeze | Agree, with two copies removed and one added (above) | Owner `Path::totalscore()` and `Path::pathstring()`, `src/core/model.h:890-891`, confirmed by `test_activation_rows.cpp:586-587` which reads them. X1 covers it. |
| RP1-2 "No Expert Pro Drums notes in this chart." | Agree | Owner `no_notes_message`, `src/parse/song.cpp:572`. I opened all 10 places. `test_user_messages.cpp:121` uses the text as both input and expected output, so it is also a pass-through test; it still pins the wording. Also builds "No Hard Drums notes..." at 124-125, the same owner. |
| RP1-3 "(unknown)" | Agree | Owner `kUnknownTitle`, `src/parse/song.h:101`. All 20 places in `r3` are assertions on that text. |
| RP1-4 "Normal" | Agree, copies added | Owner the tier table `src/core/squeeze_rating.cpp:246`. More copies: `test_rules.cpp:208, 217, 227, 241`, `test_squeeze_rating.cpp:917`, and `test_report_view.cpp:446` (`choices[1].label == "Normal"`, which reads the tier list through `tier_label`, so it is a K2 copy). `test_report_view.cpp:339` is a list of song names, not tiers: still unclear, I did not trace it. |
| RP1-5 "Expert" | Agree | `difficulty_name` and `Settings::view_difficulty = "Expert"` (`config.h:149`) confirmed. The three uitest copies assert the default. |
| RP1-6 single pins | Agree, two entries added (500 clamp in `uitest_paths`, WAL 1000) | Not repeats of 3 or more files. |
| RP1-7 48 kHz | Not settled | I did not trace the last three places (see "not reached"). `test_stream_mix.cpp:349` uses `bad_rate == 48000` and `:196` builds a 48000 tone: those are fixture inputs. `test_audio_decode.cpp:155` has the comment "Opus always decodes at 48 kHz", the one clear K2. |

## K3 and K4 rows checked and moved

Checked: 1 to 7 (all places), 12, 18, 21, 25, 33, 42, 50, 52, 54, plus the K3 parts of 16. None moved to K1 or K2. Two rows carry a K2 part the sorter missed (rows 1 and 5, above). The sorter's sample for rows 8 to 11 was not re-read.

Row 42 "Burnout" is K4: it is the song title read from the corpus chart, checked at all its places. Agree.

## Counts

Candidates agreed: 6 (RP1-1 to RP1-6), with changes on RP1-1, RP1-4 and RP1-6. Candidates disputed: 0 whole; 2 copies removed from RP1-1. Candidate not settled: RP1-7. Copies added: `test_pb_coach.cpp:66` (extra figures), `uitest_paths.cpp:408/410`, `test_store_open.cpp:1284/1292`, `test_rules.cpp:208/217/227/241`, `test_squeeze_rating.cpp:917`, `test_report_view.cpp:446`. Copies removed: `test_overlay_layout.cpp:342/510`.

## Rows not reached

Rows 8 to 11, 13 to 15, 17, 19, 22 to 23, 26 to 30, 32, 34 to 36, 38 to 41, 43 to 49, 51, 53, 55 to 56, 58 to 69 were not re-opened; they stand on the sorter's reading and the sample above. Row 39 (48 kHz): `test_preview_load_progress:341`, `test_preview_transport:62`, `test_stream_mix:168` not traced to whether the 48000 is the product rate or a fixture rate. Row 45's `test_report_view.cpp:339` not traced. I read no source of row 16's K3 `test_library_query` copies beyond the line text; `test_library_query.cpp:393` carries the comment "Burnout's real 163.0 ms" on a made-up summary, which supports the sorter's K3 reading (the test types it as an input) but also shows the author meant it as the real value.
