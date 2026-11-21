# C14-C15 fuzz records: six fuzz targets and the single-owner loader (tests/single_owner/engine.cpp)

Auditor: agent aa63f1dccb4ecf72b, task T5-FUZZ, session 16892967-2cf8-4437-a7cd-e3dad250f621. Nothing was run or built. C14 and C15 wrote no record for these files because they hold no `TEST_CASE`. This file judges each as a test anyway: a fuzz target is a test whose promise is "no input crashes, hangs or reads out of bounds".

## Reading common to every fuzz record

Read: `tests/fuzz/harness.h` whole (lines 1-84); the six `tests/fuzz/fuzz_*.cpp` files (each 6 lines, the same shape); `CMakeLists.txt` lines 126-141 and 952-982 (by grep and listing; the loop at 963-969 builds one exe per `fuzz_*.cpp` under `HYDRA_FUZZ`); `.github/workflows/fuzz.yml` whole; `.github/workflows/ci.yml` lines 385-444 (the `fuzz-replay` job); `tools/fuzz_run.ps1` and `tools/fuzz_seeds.ps1` whole; `tests/test_fuzz_regressions.cpp` lines 28-145; `testdata/fuzz/README.md`; the file list of `testdata/fuzz/`; `src/parse/song.cpp` lines 2335-2464; `src/audio/decode.cpp`; `src/image/decode.cpp`; `src/ui/preview_load_job.cpp` lines 100-135; `src/core/error_kind.h:129` (`KindedError` derives from `std::runtime_error`); `docs/agents/test-rubric.md`; `records/B1.md` lines 1-80 (the record for `tests/test_fuzz_regressions.cpp`); `records/C14.md` and `records/C15.md` (the "no test case" notes). Adding commit: `b93ca015` "Fuzz targets for the six file parsers, and a fuzz preset" (subject only; the body was not opened). `docs/development.md` "Fuzzing the file parsers": heading found, body not read. Not read: the parsers' own bodies (`ChartParser::parse`, `parse_mid_lean`, the sng and srb readers, `open_stem_reader`, stb), the fuzz runs' CI logs.

What every target promises. `run_one` (harness.h:71-79) copies the bytes into a vector of exactly their size, calls the parser inside `try`, swallows `std::exception`, and returns 0 on every path that returns. Its header comment (lines 3-6) says the rule: a parser that rejects bytes with an exception is fine; "a crash, an AddressSanitizer report, a hang, an abort" is a bug.

What turns it red. Nothing in `run_one` can: the return is always 0, so `CHECK(run_one(...) == 0)` in `tests/test_fuzz_regressions.cpp:57` can never fail by its value (B1 found the same). Red comes only from outside it. A crash or an exception that is not a `std::exception` ends the process (and so the doctest run). An ASan report aborts it, and ASan is on only in the fuzz and asan builds (`CMakeLists.txt:136` refuses `HYDRA_FUZZ` without `HYDRA_ASAN`); in the plain `hydra_tests` build the same replay catches a crash but not an out-of-bounds read that happens not to fault. A hang or a memory blow-up turns red only under libFuzzer, whose limits are `-timeout=10` and `-rss_limit_mb=2048` (`tools/fuzz_run.ps1:41`); under doctest a hang just hangs. A parser that throws `std::exception` on every input, even a valid chart, is green: the harness has no check that a good file parses. The seed step covers that for two targets only: `tests/test_fuzz_regressions.cpp:128,133` `CHECK_NOTHROW` on the sng and srb seeds.

How and where they run. Weekly, `fuzz.yml` (Mondays 06:17 UTC, or by hand) builds each exe with `-Preset fuzz`, seeds it from the checked-in corpus, and fuzzes 600 s with a cached grown corpus (`fuzz.yml:118`). On every push and pull request, `ci.yml`'s `fuzz-replay` job (line 391) replays the grown corpus plus all of `testdata/fuzz` once through each exe (`-runs=0`). The seeds are written by `hydra_tests -tc="fuzz seeds*"` (`tools/fuzz_seeds.ps1:36`), so building the seeds needs a built `hydra_tests`. The doctest case "fuzz regressions" replays `testdata/fuzz` through the same `run_one` in the ordinary test run, without ASan unless that build has it.

The claim cannot be checked by one run. "No input" is a promise only a long run can support, and the real thing is the real mechanism: the fuzz exe under libFuzzer and ASan on the real parser. I have no run output, so every record below says "claim not checked" for question 3 and names what it needs: the output of one `fuzz_run.ps1 -Mode replay` over `testdata/fuzz` on a fuzz-preset build (the main session builds and runs it), and the last weekly `fuzz.yml` run's result for that target. Under the brief's reading rule, a test whose only fault is question 3 gets Investigate, "claim not checked only". Where I found a second fault it is named in the record.

Shared findings, so each record can point here:

- Finding F1 (split, for the user): `run_one`'s `return 0` makes `tests/test_fuzz_regressions.cpp:57`'s `CHECK(... == 0)` unable to fail. That line is B1's. Nothing here proposes a new rule; the question is whether the check should exist at all.
- Finding F2: `run_one` catches `std::exception`, so every `KindedError` (`src/core/error_kind.h:129`) is "rejected properly". A target cannot tell a rejection of a good file from a rejection of a bad one. Two things only back the good-file side: the sng and srb `CHECK_NOTHROW` on their seeds, and the parsers' own tests (not read).
- Finding F3: `parse_song` builds the settings from `app::AnalysisSettings` defaults (harness.h:30-42), so the fuzzers read only the default difficulty, default fill rule (`fill_rule_for(s.legacy_fills)`) and default reading flags. A parser branch behind `prodrums`, `bass2x`, another difficulty or the Clone Hero 1.0 fill rule is not reached. `reading.difficulty` and `legacy_fills` are inputs a user can change, so those branches are real.

---

## tests/fuzz/fuzz_chart.cpp (line 4, `LLVMFuzzerTestOneInput`)

Read: common; the file; `load_songbytes_chart` (`src/parse/song.cpp:2345-2363`); its callers `load_songpath_chart` (2371), `load_container_sng` (2390), `load_container_srb` (2418), `load_songpath_reading` (2458); `tests/fuzz/chart.dict` (listed, not opened); `testdata/fuzz/chart/far-chord-fill-lines.chart` (listed). Adding commit `b93ca015`.
1 Claim: Whatever bytes a `.chart` file holds, loading it either returns a song or throws a normal error; it never crashes, hangs or reads out of bounds.
2 Owner: `load_songbytes_chart` (song.cpp:2345) is the one chart loader. Callers in `src/`: `load_songpath_chart`, `load_container_sng`, `load_container_srb`, `load_songpath_reading`, all in song.cpp. The target calls it with `ChartDrumKeys{}` (the overload at 2360), where production's loose-file path passes `chart_drum_keys_beside(path)` (2373) and the sng path passes keys from the container's metadata (2386). So it is the production parser, with the song.ini drum keys unset (F3).
3 Real chart: claim not checked. Engine: not read, no fuzz run output. Claim: no crash or hang on any input. Needs: the replay output over `testdata/fuzz/chart` and the grown corpus, and the last weekly run for `chart` (main session).
4 Direct: line 5 passes the bytes through; there is no assertion. The check is the process surviving (see common).
5 Reason: `-timeout=10`, `-rss_limit_mb=2048` are typed in `tools/fuzz_run.ps1:41` with no reason beside them. Ten seconds and 2 GB are a chosen budget, not a rule.
6 Display: not a display.
7 Failure: the saved inputs name them: a chart with a far chord and fill lines made the fill search blow up (`testdata/fuzz/chart/far-chord-fill-lines.chart`, 24,824 bytes). The adding commit's body was not opened. Still possible for a new input.
8 Deleted: deleting the target: a crash on a malformed chart goes unseen until a user hits it (reasoned). The only line is the call.
9 Stand-in: the promise is "no input", and the stand-in is a 10-minute weekly run plus a replay of what earlier runs found. That is the normal shape of fuzzing; it is not a proof. Seeds: one saved regression and, from the weekly seeds job, every corpus `.chart` (44 tracked files; `tests/test_fuzz_regressions.cpp:97-104`). A dictionary exists (`chart.dict`); the other five targets have none.
10 Same rule: `tests/test_fuzz_regressions.cpp:46` replays the same saved inputs through the same `run_one`; B1's record owns it.
11 Real: yes, a damaged `.chart` in a song folder.
Verdict: Investigate. Claim not checked only (replay output and the weekly run's result for `chart`). F3 applies: the fuzzer reaches only default settings.

## tests/fuzz/fuzz_mid.cpp (line 4)

Read: common; the file; `load_songbytes_mid` (`src/parse/song.cpp:2335-2358`); callers `load_songpath_mid` (2365), `load_container_sng` (2389), `load_container_srb` (2417), `load_songpath_reading` (2455). Saved inputs listed: `far-chord-fill-lines.mid`, `mtrk-tag-at-end.mid` (32 bytes), `no-tracks-after-long-header.mid`. Adding commit `b93ca015`.
1 Claim: Whatever bytes a `.mid` file holds, loading it returns a song or throws a normal error; never a crash, hang or out-of-bounds read.
2 Owner: `load_songbytes_mid`; same callers as above; the target uses `ChartDrumKeys{}`, as production does for an srb's notes (2417) and not for a loose or sng file (F3).
3 Real chart: claim not checked. Needs the replay output over `testdata/fuzz/mid` and the weekly run's result for `mid`.
4 Direct: no assertion; survival is the check.
5 Reason: the limits, as for chart.
6 Display: not a display.
7 Failure: three saved inputs name three: a file that ends on an `MTrk` tag, one with no tracks after a long header, and the far-chord fill case. Still possible.
8 Deleted: a crash on a truncated or hostile midi unseen (reasoned).
9 Stand-in: as chart. Seeds are every tracked corpus `.mid` (71 files by `git ls-files`), via the seeds job.
10 Same rule: the regression replay (B1).
11 Real: yes.
Verdict: Investigate. Claim not checked only; F3.

## tests/fuzz/fuzz_sng.cpp (line 4)

Read: common; the file; `load_songbytes_sng` (`src/parse/song.cpp:2423-2426`) and `load_container_sng` (2383-2391); `load_songpath_sng` (2436). Saved input listed: `far-chord-fill-lines.sng` (33,121 bytes). Seeds code: `tests/test_fuzz_regressions.cpp:117-135`. Adding commit `b93ca015`.
1 Claim: Whatever bytes a `.sng` container holds, reading its header, metadata and notes returns a song or throws a normal error, without a crash, hang or out-of-bounds read.
2 Owner: `load_container_sng` is the one reader. The target reaches it through `load_songbytes_sng`, which uses `memory_byte_source(buf)`. Production's library scan and Preview open a container with `file_byte_source(path)` (`load_songpath_sng`, 2436) or a shared buffer (the comment at 2379-2382). The container logic (`sng_read_head`, `sng_read_metadata`, `sng_read_notes`) is the same code; the byte source differs (memory against file reads in pieces). A bug in the file source's piecewise reads is not reached.
3 Real chart: claim not checked. Needs the replay output over `testdata/fuzz/sng` and the weekly run for `sng`.
4 Direct: no assertion.
5 Reason: the limits. The container size the real library holds is up to about 1 GB (ADR 0024, named in the comment at song.cpp:2433-2435); the fuzzer's inputs are far smaller, and the seed is a built container wrapping one corpus chart.
6 Display: not a display.
7 Failure: the far-chord fill case saved inside a container. Still possible.
8 Deleted: a crash on a corrupt `.sng` header unseen (reasoned).
9 Stand-in: the seeds are built by `testsng::make_sng` (test fixture code), the only builder of the container; the real-world `.sng` files have no seed. Two seeds only (`seed.mid.sng`, `seed.chart.sng`).
10 Same rule: the regression replay (B1). Sng reading tests elsewhere not searched.
11 Real: yes.
Verdict: Investigate. Claim not checked only. Second question for the user, not a rewrite: should the file-backed `ByteSource` path have a target? I cannot say whether that is wanted.

## tests/fuzz/fuzz_srb.cpp (line 4)

Read: common; the file; `load_songbytes_srb` (`src/parse/song.cpp:2428-2431`) and `load_container_srb` (2393-2419). Saved inputs listed: `endless-zeros.srb` (33 bytes), `far-chord-fill-lines.srb`, `mtrk-tag-at-end.srb`. Adding commit `b93ca015`.
1 Claim: Whatever bytes an `.srb` container holds, reading its metadata and inflating its notes stream returns a song or throws a normal error, without a crash, hang or memory blow-up.
2 Owner: `load_container_srb`; same memory-against-file byte-source difference as sng (`load_songpath_srb`, 2441). The inflate step is bounded by `kSrbMaxStream` (named at 2401; its value not read).
3 Real chart: claim not checked. Needs the replay output over `testdata/fuzz/srb` and the weekly run for `srb`. This is the target whose saved input `endless-zeros.srb` names a hang or memory case, so the replay's result matters most here.
4 Direct: no assertion.
5 Reason: the memory limit (2048 MB) is what judges an inflate bomb here; no page says why that number.
6 Display: not a display.
7 Failure: a tiny stream of zeros that inflates without end. Still possible if the bound is removed.
8 Deleted: an inflate bomb or a bad stream length going unseen (reasoned).
9 Stand-in: seeds are built by `testsrb::make_srb`; two seeds. No real `.srb` file seeds it.
10 Same rule: the regression replay (B1).
11 Real: yes.
Verdict: Investigate. Claim not checked only.

## tests/fuzz/fuzz_audio.cpp (line 4)

Read: common; the file; `src/audio/decode.cpp` whole; callers of `decode_audio` in `src/` (grep `decode_audio\(`, `audio::decode_audio`, `StemReader`, `open_stem_reader` over src, tools, tests); `src/ui/preview_load_job.cpp:105-135`; `testdata/audio` listing (`sine220.flac/.mp3/.ogg/.opus/.wav`). Saved inputs listed: `huge-comment-length.ogg` (11,410 bytes), `huge-flac-length.flac` (32,600 bytes). Adding commit `b93ca015`.
1 Claim: Whatever bytes an audio stem holds, finding its container kind and decoding it returns audio or throws a normal error, without a crash, a hang or a huge allocation.
2 Owner: `decode_audio` (`src/audio/decode.cpp:40`) opens `open_stem_reader(StemBytes)` and reads the whole stem. Callers in `src/`: none outside its own file and `decode_stem` (47); the Preview and the player never call it. They call `open_song_stems` (`src/ui/preview_load_job.cpp:125`) and read the stem reader in chunks, with seeks. So the target reaches the production reader's open and read-to-end path through a wrapper only the tests use; it does not reach a seek, a chunk size that differs, or the mapped-file source (`audio::map_song_stems`). The `read_all` comment (decode.cpp:13-19) says a fuzzed FLAC once asked for hundreds of gigabytes, which is why it grows the buffer by frames.
3 Real chart: claim not checked. Needs the replay output over `testdata/fuzz/audio` and the weekly run for `audio`.
4 Direct: no assertion.
5 Reason: the limits. Seeds are the five `sine220` fixtures only (`tests/test_fuzz_regressions.cpp:108-110`); no real song stem.
6 Display: not a display.
7 Failure: a fuzzed header promising a huge length; the two saved files. Fixed in `read_all`, still possible in a reader that sizes from a header (opus, vorbis, miniaudio readers not read).
8 Deleted: a decoder crash on a damaged stem unseen (reasoned).
9 Stand-in: the target is a stand-in for the Preview's reader in the sense above: a split between the fuzzed path (`decode_audio`, whole-file) and the played path (`StemReader` in chunks, with seek). `tests/test_stem_reader.cpp:249` checks that the two agree on good audio; that does not cover bad input. A decode of 5 fixture seeds also runs the sniffer, as harness.h:47 says.
10 Same rule: the regression replay (B1); `tests/test_stem_reader.cpp` for the equality on good audio (not read).
11 Real: yes; a corrupt stem in a song folder.
Verdict: Investigate. Claim not checked, and one question for the user: the target fuzzes `decode_audio`, which only tests call, not the reader the Preview plays; should it also fuzz seek and chunked reads?

## tests/fuzz/fuzz_image.cpp (line 4)

Read: common; the file; `decode_image` (`src/image/decode.cpp:13-27`); its callers `src/render/preview_renderer.cpp:166` and `src/ui/icons.cpp:31`. `testdata/fuzz/image` holds only `.gitkeep` (no saved input). `testdata/preview` seed: `golden_onyx.png`. Adding commit `b93ca015`.
1 Claim: Whatever bytes an image holds, decoding returns an empty image or pixels; it never crashes, hangs or reads out of bounds.
2 Owner: `decode_image` is the one decoder, and the target calls it directly, the same function both production callers use (PNG and JPEG only, `STBI_ONLY_PNG`, `STBI_ONLY_JPEG`). Failure is an empty image, not an exception (harness.h:52), so `run_one`'s `catch` is never the path here; red is a crash only.
3 Real chart: claim not checked. Needs the replay output over the grown corpus for `image` (there is no saved file) and the weekly run for `image`.
4 Direct: no assertion.
5 Reason: the limits.
6 Display: not a display (the decoder feeds one).
7 Failure: stb's known decoder faults on hostile PNG and JPEG (the reason to fuzz it). No saved case, so no failure was ever caught here; the adding commit's body was not opened.
8 Deleted: a decoder fault unseen (reasoned).
9 Stand-in: one PNG seed, no JPEG seed, although JPEG is decoded; the JPEG branch has no seed to start from. Cover art and the Preview's textures are what real users load.
10 Same rule: the regression replay has no image file to replay (`CHECK(files > 0)` at test_fuzz_regressions.cpp:63 is met by other targets).
11 Real: yes; a damaged album image.
Verdict: Investigate. Claim not checked only (replay output and weekly run). Side note: no JPEG seed, `tests/test_fuzz_regressions.cpp:111-113` takes `.png` only.

---

## tests/single_owner/engine.cpp: the functions the scan tests call

These are the loader and the row-scan engine. `scan.cpp` (audited as A49) calls them, `tools/precheck/check_rows.cpp` calls `select_rule_files` (44), `parse_rules_file` (52), `parse_baseline` (57), `listed_lines` (180), `compile_rule` (197), `start_scan` (198) and `scan_file` (207), `tests/test_layering.cpp` calls `parse_dates` (494), `today_iso` (610) and `baseline_due` (620), and `tests/test_precheck.cpp:323` calls `rule_files`. In `src/` and `tools/` outside precheck there is no other caller (grep over `src`, `tools`).

Reading common to the engine records: `tests/single_owner/engine.cpp` whole (1-383); `engine.h` whole; `listed.h` whole; `scan.cpp` lines 40-612 (every case that calls an engine function); `tests/source_tree.h` lines 28-105 (`walk_visits`, `source_files`); `tests/single_owner/baseline.json` (51 bytes; by grep: no `date`, `remove_by` or entry, so both lists are empty). Not read: `scan.cpp` lines 612-end, `clones.cpp`, `rows.h`, `special.cpp`, `tools/precheck/check_rows.cpp` beyond the grep lines, the rules files' contents, `tests/test_precheck.cpp`'s planted-row case. Adding commit for the split: `ad410b9a` "Pull the single-owner row scan's engine out of scan.cpp" (subject only).

What the scan tests rely on, in game words for a reviewer: "a source line that answers a question the owner already answers is found, and a line the owner lists is let through once". The real thing is the real source tree and the real rules files. The independent owner for "which files exist" is git (`source_files`), and for "does this regex flag this line" the rows' own `must_match` and `must_not_match` examples. A scan that finds nothing proves nothing on its own: the real tree passing says no flagged line was found, not that none exists. The made-up rows in `scan.cpp` (260, 351, 431) are the only miss-detection checks, and they are hand-built. `tests/test_precheck.cpp:308` plants a row in a fixture repo; I did not read it, so I do not count it.

### load_tables (engine.cpp:270)
Read: common; 270-275; `scan.cpp:52-63`. 
1 Claim: The scan reads every rules file and the baseline of the repo it is given, and a bad file fails with its name.
2 Owner: it composes `load_rules` and `parse_baseline`. Callers: `scan.cpp:53` only (and `check_4.cpp` reads `kRulesDir`/`kBaselineFile` itself). 
3 Real chart: Engine: the real `tests/single_owner/rules/` and `baseline.json` through `load_tables(sourcetree::root())`. Claim: all rows load. Checked on real input by every scan case, which would throw at load; `rules().size() > 100` at scan.cpp:479 proves the folder was found. Owner of "which rule files exist": `select_rule_files` below.
4 Direct: no assertion of its own; failure is a thrown `runtime_error`.
8 Deleted: loading `baseline.json` last (reasoned: a missing baseline throws via `read_text_file`).
9 Stand-in: none.
10 Same rule: `check_rows.cpp` loads the rules a second way, from git tip text (`parse_rules_file`), not from disk; the two share `parse_rule`.
11 Real: yes.
Verdict: Keep. Questions 5-7 and 12: no number; the failure is a rules file that fails to load silently, which `required_string`'s throws prevent; read as above.

### load_rules (engine.cpp:263) and rule_files (engine.cpp:253)
Read: common; 253-268; `scan.cpp:538-562`; `test_precheck.cpp:323` by grep.
1 Claim: The rules files under a folder, an area file or one folder down, are all read in name order, named by their path under `rules/`; a non-json file and a file two folders down are not read.
2 Owner: `select_rule_files` decides which; `rule_files` lists the disk and `load_rules` parses. Callers: `scan.cpp:550,559` (made-up folder), `load_tables`, `test_precheck.cpp:323`.
3 Real chart: Engine: the real `tests/single_owner/rules/` folder, and the planted-folder case at scan.cpp:538. Claim: real folder loads (>100 rows); made-up folder gives the rows in order named `rules/made_up.json` and `rules/made_up/topic.json`. The made-up folder is hand-built, the real folder is real; independent owner of the file list: `select_rule_files`, whose test is below. Checked.
4 Direct: scan.cpp:551-555 compares question and `rules_file` per row (properties); 559 the whole error text (the text names file and row: the claim).
8 Deleted: a sort removed (reasoned: the "area row" then "topic row" order at 552-555 depends on name order; on Windows directory order is already by name, so this line may not catch it on this machine); a missing `rules/` prefix.
9 Stand-in: none.
10 Same rule: `select_rule_files` has its own case (scan.cpp:375).
11 Real: yes.
Verdict: Keep.

### select_rule_files (engine.cpp:242)
Read: common; 242-251; `scan.cpp:375-385`; ADR 0029 (named in the comment, not opened).
1 Claim: Of a path list, the rules files are the `.json` files directly in the rules folder or one folder down; they come back as `rules/<path>` in byte order.
2 Owner: this is the one function; callers `rule_files` (259) and `check_rows.cpp:44`.
3 Real chart: Engine: the real rules folder gives >100 rows through it (scan.cpp:479), and the case's list is a hand-built list. Claim: the made-up list is filtered and sorted as stated. Checked on real input: the real folder's rows load. Independent owner of "which files": none; the area-folder shape is ADR 0029, not opened.
4 Direct: 381-383 the full expected list typed, which states the rule as an example (uppercase first, `.txt` and two-deep dropped). 384 empty in, empty out.
8 Deleted: dropping the `count > 1` test (the `app/deep/row.json` entry would appear: reasoned); dropping the sort (the order would differ).
9 Stand-in: none. ASCII sort of a name is the claim.
10 Same rule: the folder case (scan.cpp:538) uses it end to end.
11 Real: yes.
Verdict: Keep.

### parse_rule (engine.cpp:86), parse_rules_file (engine.cpp:140)
Read: common; 28-60, 86-149; `scan.cpp:516-536`, 545-560; `rows.h` not read.
1 Claim: A row with a missing, unknown or mistyped field fails with its rules file and its question (or its row number when it has no question); a good row loads; a rules file holds a `rules` array.
2 Owner: this function. Callers: `parse_rules_file`, `scan.cpp:519-534`, `check_rows.cpp:52` (via `parse_rules_file`). Other places that read rule rows: none found in `src`, `tools`.
3 Real chart: Engine: the real rules files, all parsed by it each run. Claim: a typo field fails. Hand-built rows (519-534) check the failures; the real rows check the success. Checked on real input for success; the failure messages are checked on made-up rows only, which is all a failure can be.
4 Direct: 519 question carried; 522-535 four whole messages typed (they are the claim: file, question, field).
8 Deleted: the `check_keys` call (a typo would pass: line 526 catches); `required_string` empty test (an empty `"owner"` would pass: not caught by a made-up row, reasoned); `scan_comments` mistyped as a string would throw a json type error, whose message is wrapped with the row name by the catch at 135.
9 Stand-in: none. Gap: `pattern`, `calls_owner`, `must_match` regexes are not compiled here; a bad regex surfaces in `compile_rule` as a `std::regex_error` with no rule name (side note below).
10 Same rule: none.
11 Real: yes.
Verdict: Keep.
Side note: engine.cpp:277-281 `compile_rule`: a bad `pattern` throws `std::regex_error` with no row named, unlike every other loader failure here. Reasoned, not run.

### parse_baseline (engine.cpp:204)
Read: common; 181-238; `scan.cpp` by grep (no direct call; reached through `load_tables`); `check_rows.cpp:57`.
1 Claim: The baseline's known copies and known clones load with their dates checked, and a missing or unknown field fails by name.
2 Owner: this function; callers `load_tables`, `check_rows.cpp:57`. Date checks are `parse_dates`.
3 Real chart: claim not checked: the real `baseline.json` holds no entry (51 bytes), so no real baseline entry has ever passed through it in this tree. Engine: not applicable on real input. Claim: entries with dates load. Needs: a real entry from an earlier commit of `baseline.json` (`git log -p`) fed through it, or the main session's answer that an empty baseline is the intended state.
4 Direct: no case calls it with an entry in `scan.cpp`; only `test_layering.cpp` (not read) and the precheck use entries.
8 Deleted: `check_keys` at 207 (reasoned: nothing in `scan.cpp` would catch a typo field on a baseline entry).
9 Stand-in: none.
10 Same rule: `test_layering.cpp:480-494` has its own gap list read by `parse_dates`.
11 Real: yes when entries exist.
Verdict: Investigate. Claim not checked only: needs a real baseline entry; and the scan's own tests never feed it an entry (that is a missing test, not a wrong one).

### parse_dates (engine.cpp:181)
Read: common; 181-193; `scan.cpp:579-604`; `test_layering.cpp:494` by grep; `kBaselineMaxDays` comment (engine.h:32-35).
1 Claim: A baseline entry's `remove_by` is a real date, no more than 14 days after its `date`; a missing or bad date fails with the entry's name.
2 Owner: this function; callers `parse_baseline` (220, 231), `test_layering.cpp:494`, `scan.cpp:593-602`.
3 Real chart: claim not checked on real input (the real baseline is empty). Calendar facts checked by hand: 2026-10-05 plus 14 is 2026-10-19; correct.
4 Direct: 593 loads and reads `remove_by`; 595-603 three whole messages typed (they are the claim).
5 Reason: 14 days: `kBaselineMaxDays` comment says "the suggestion in question 5, 'Baseline cap', approved with the rest of" a plan that is removed (`git show 444631e6:docs/superpowers/plans/2026-10-10-single-owner-redesign.md`). Not opened; an approval is not a reason. A number with no reason I can find.
8 Deleted: the cap test (595 catches); the real-date test (599 catches); the missing field (603 catches). A `remove_by` earlier than `date` is not rejected and no line tests it (reasoned).
9 Stand-in: none.
10 Same rule: `test_layering.cpp` applies it to its own list.
11 Real: yes.
Verdict: Investigate. Two causes: claim not checked on a real entry, and the 14 has no reason I can find (the plan that approved it is not opened).

### iso_plus_days (engine.cpp:151)
Read: common; 70-76, 151-163; `scan.cpp:580-583`.
1 Claim: An ISO date plus a number of days is the ISO date that many days later; a date that does not exist comes back as a different day; a date not in `YYYY-MM-DD` shape comes back empty.
2 Owner: this function. The only date-arithmetic in `src`, `tools`, `tests` (grep `mktime`, `strftime`, `%Y-%m-%d`; the others read a clock for display, not add days). Callers: `parse_dates`, `scan.cpp:580-583`.
3 Real chart: Engine: the calendar. Claim: 2026-10-05 plus 14 is 2026-10-19; 2026-12-25 plus 14 is 2027-01-08 (a year end); 2026-02-30 normalises to 2026-03-02. I checked these three by hand and they are right. Real input: no real baseline date exists.
4 Direct: 580-583 each states one case by its inputs.
8 Deleted: noon (`tm_hour = 12`): not caught on a machine with no daylight-saving change on those dates (reasoned); the shape test: 583 catches.
9 Stand-in: none.
10 Same rule: none.
11 Real: yes.
Verdict: Keep.

### today_iso (engine.cpp:165)
Read: common; 165-172; `scan.cpp:565`; `test_layering.cpp:610` by grep.
1 Claim: Today's date on this machine's calendar, as YYYY-MM-DD.
2 Owner: this function; callers `scan.cpp:565`, `test_layering.cpp:610`.
3 Real chart: Engine: the real clock. Claim: the case reads it and compares nothing to an independent date. Checked: no.
4 Direct: no assertion on its value; `baseline_due` receives it.
8 Deleted: returning an empty string would make every entry look not-due (`"" <= remove_by`, engine.cpp:176), a silent pass; no line checks the shape (reasoned).
9 Stand-in: none.
10 Same rule: none.
11 Real: yes.
Verdict: Investigate. Claim not checked only: nothing compares `today_iso()` with an independent date, or at least with `is_iso_shape`; and the empty result passes silently.

### baseline_due (engine.cpp:175)
Read: common; 175-179; `scan.cpp:564-604`.
1 Claim: An entry is due (fails) the day after its `remove_by`, not on it; the message names the entry, `remove_by`, who added it and when.
2 Owner: this function; callers `scan.cpp:568,572,587,588`, `test_layering.cpp:620`.
3 Real chart: claim not checked on real input: no real entry. Engine: none. Needs a real baseline entry.
4 Direct: 587 the last allowed day gives empty; 588-590 the next day gives the typed message, the claim.
8 Deleted: `<=` turned to `<` (587 catches); the message's pieces (590 catches).
9 Stand-in: none.
10 Same rule: `test_layering` shares it for its own list; they agree because both call it.
11 Real: yes.
Verdict: Investigate. Claim not checked only (empty real baseline).

### copy_entry_name (engine.cpp:195) and clone_entry_name (engine.cpp:199)
Read: common; 195-202; `scan.cpp:568,572,586,589`.
1 Claim: A baseline entry is named in failures by its question, file and text (copy) or both files, the line count and the first line (clone).
2 Owner: these two; callers `scan.cpp:568,572,586`, `test_layering.cpp` (not opened). Their text is also the naming `baseline_due` messages use.
3 Real chart: claim not checked on real input: no real entry. Only `clone_entry_name` is pinned, by a made-up entry at scan.cpp:589; `copy_entry_name` has no assertion on its text.
4 Direct: 589 types the clone name inside a larger message.
8 Deleted: the copy name's words: no line catches it (reasoned).
9 Stand-in: none.
10 Same rule: none.
11 Real: yes.
Verdict: Investigate. Claim not checked only; `copy_entry_name`'s text has no pinning line.

### compile_rule (engine.cpp:277) and flags_line (engine.cpp:283)
Read: common; 277-287; `scan.cpp:100-104, 170-200, 260-282, 396-405`; `rows.h` not read.
1 Claim: A line is flagged when it holds the row's anchor, matches its pattern, and does not call the row's owner.
2 Owner: `flags_line`, the one verdict (engine.h:103-105). Callers: `scan_file` (373), `scan.cpp:178,182,186,404`. Same question answered elsewhere: none found; `scan_single_file` in scan.cpp:394 calls it, as the header says.
3 Real chart: Engine: every real row's own examples (`must_match`, `must_not_match`, owner lines) run through it at scan.cpp:170-189; those examples are written by the row's author, not independent of the pattern. Claim: each example is or is not flagged. Checked on real rows against their own examples. Independent owner of "is this a real answer": none; this is a regex judging text.
4 Direct: 178, 182, 186 each states a property: this example is flagged or not. 187 `why` not empty.
8 Deleted: `calls_owner` test inverted (a row's owner-line example at 186 catches only if that line lacks a call to the owner); the anchor test removed (the anchor guard at 109 catches only that anchors are in examples, not that `flags_line` uses them: a deletion at 284 passes every example, reasoned).
9 Stand-in: the examples are a stand-in for "the row finds all real answers". The real tree test (202) can only show none are found.
10 Same rule: `scan_single_file` repeats the comment-and-flag loop of `scan_file` outside the engine (scan.cpp:396-406); that is a second copy of the row-scan loop, in the test file. Both call `flags_line`. Split on a copy: the single-file loop and `scan_file` agree only by reading the same helpers.
11 Real: yes.
Verdict: Investigate. Claim not checked: no independent check that a row finds every real answer; only own examples. Plus the anchor deletion at 284 passes every test I read.
Finding: the anchor shortcut at engine.cpp:284 is not exercised by any assertion (reasoned).

### lexer_language (engine.cpp:289) and skipped_comment_line (engine.cpp:295)
Read: common; 289-297, 78-82; `scan.cpp:396, 403, 467-470`; `code_lexer.h` not read.
1 Claim: A file's comments are read in the language its extension names, C++ when unknown; for C++ only a `//` line is a skipped comment, every other language skips what the lexer calls a whole-line comment.
2 Owner: `codelex::language_of_extension` and `codelex::split` own the languages (not read); these two choose. Callers: `walk_language` (316), `scan_file` (354), `scan.cpp:95, 396, 403, 467-470`.
3 Real chart: claim not checked on real input: the case at 467-470 types four lines. Real input would be a real `.ps1` and `.yml` from `tools/` and `.github/`; scan.cpp:441-443 names those paths but checks `walk_reads_for`, not the comment reading. Needs: the real workflow and script files' comment lines run through it, checked against `codelex`.
4 Direct: 467 `// x` skipped; 468 `/* x */` not skipped in C++ (this states a limit: a block-comment line is scanned, so a flagged line can be a false hit, never a missed one); 469 `# x` skipped in PowerShell; 470 `x # y` not skipped in YAML.
8 Deleted: the C++ special case at 296 (468 catches); the language lookup (469 catches).
9 Stand-in: none.
10 Same rule: the lexer is `codelex`'s; `scan.cpp:71-84` `names_outside_comments` uses it directly for owner symbols. The row scan and that guard treat comments by two rules (C++ `//` only; codelex pieces). Split: a symbol named only inside a `/* */` block counts as a comment in the owner-gone guard (509-514, `/* fits_shell */` is not a name) but a line in that block is scanned by `scan_file`.
11 Real: yes.
Verdict: Investigate. Claim not checked only; the split above goes to the user as a question about two comment rules.

### in_scope (engine.cpp:299)
Read: common; 299-304; used inside `scan_file` (359). No direct case in `scan.cpp`.
1 Claim: A row with no scope reads `src` and `tools`; a row with a scope reads its folders and files.
2 Owner: this function; one caller, `scan_file`. Callers outside the test code: none.
3 Real chart: claim not checked: no case calls it. The real rows' `scope` fields are exercised by 224-232 (which only checks scoped files exist) and 414-429 (single files).
4 Direct: none.
8 Deleted: the default `src`/`tools` line: nothing catches a swap in `scan.cpp` as I read it (reasoned): the made-up row's `src/a.cpp` case at 260 is in `src`, which passes either way.
9 Stand-in: none.
10 Same rule: `walk_visits` (`tests/source_tree.h:38`) names the walked folders `src`, `tools`, `tests`; the default scope of a row is `src` and `tools`. The two lists are separate and agree only by convention.
11 Real: yes.
Verdict: Investigate. Claim not checked: the default scope has no assertion; needs a made-up row scanned in a `tests/` file and a `tools/` file.

### walk_language (engine.cpp:308) and walk_reads_for (engine.cpp:320)
Read: common; 306-323; `scan.cpp:209,310,418,440-447`; `source_tree.h:38`.
1 Claim: The walk reads C++ files and Python files under `src`, `tools` and `tests`, never `tests/single_owner/` (it holds made-up rows as text), and a row reads only files in its own language; anything else is read by the single-file case.
2 Owner: `walk_visits` says which folders; these two add language and the exclusion. Callers: `scan.cpp:209,310,418,440-447`. A second place that decides the same "is this file read by the scan": `scan.cpp:418` (`walk_reads_for`) and `tools/precheck/check_rows.cpp` (not read for this).
3 Real chart: Engine: real paths named at 440-444: `src/core/winstr.cpp` read, `tools/fuzz_seeds.ps1` not, `.github/workflows/ci.yml` not. Claim: as typed. These are real tracked files; the independent owner of "is it a C++ file" is `is_cpp_source` (listed.h:62). Checked on real paths.
4 Direct: 440-447 each states one path and its answer (properties by name); 444 the excluded folder.
8 Deleted: the `tests/single_owner/` exclusion at 311 (444 catches); the `compare(0, 19, ...)` length (the literal 19 matches the prefix length: 6 + 12 + 1; a typo would be caught by 444).
9 Stand-in: none.
10 Same rule: see `in_scope`.
11 Real: yes.
Verdict: Keep.

### listed_lines (engine.cpp:327)
Read: common; 327-338; `scan.cpp:205,354`; `listed.h:38-59` (`take_listed_line`).
1 Claim: The lines a scan accepts by exact text are each row's owner lines, then every baseline copy, in that order.
2 Owner: this function; callers `scan.cpp:205,354`, `check_rows.cpp:180`.
3 Real chart: Engine: the real rows' owner lines, run in the whole-tree case (202) and the stale check in `check_scan` (not read). Claim: each owner line is used by a real source line, or the scan fails as stale. Checked on real input; the independent owner of the lines is the source file itself.
4 Direct: 354 builds the list from the made-up row and 358-372 reads the hits (the `listed_index == 0` at 364 pins the order for one entry); the baseline half has no assertion.
8 Deleted: the baseline loop (334): nothing catches it with an empty real baseline (reasoned).
9 Stand-in: none.
10 Same rule: `take_listed_line` has its own case (155).
11 Real: yes.
Verdict: Keep. The baseline half is unchecked because the real baseline is empty; that is the same open point as `parse_baseline`.

### start_scan (engine.cpp:340) and scan_file (engine.cpp:345)
Read: common; 340-381; `scan.cpp:128-151, 260-282, 351-373`; `check_rows.cpp:198,207` by grep.
1 Claim: Scanning a file's lines flags each line that answers a row's question without its owner, skips comment lines unless the row reads comments, limits a function-scoped row to one function's lines, skips owner and exempt files, and reports for each flagged line whether a listed line took it, was used up, or none listed it.
2 Owner: `scan_file` is the one scan (engine.h:3-6). Callers: `scan.cpp:140,358`, `check_rows.cpp:207`. The single-file case in `scan.cpp:394` repeats a part of the loop (see `flags_line`).
3 Real chart: Engine: the real source tree (202): no flagged line left. Claim: the same. A scan that finds nothing is consistent with a scan that cannot find anything; the made-up file at 255-256 and 431-476 is the only proof it flags; independent owner for "does this tree answer a question": none. Claim not checked on real input for the misses. Needs: a real known answer in the tree (a line from before a row's fix, from `git log`) run through `scan_file`, or the planted fixture of `test_precheck.cpp:308` (not read).
4 Direct: 270-280 two whole messages typed (the made-up file, five lines, each message names file, line, question and owner: this is the claim); 359-372 each hit's line, text, take, index (properties by name of the made-up file); 222 `functions_found == 1` per function-scoped row (a property); 233 `files > 100`.
5 Reason: `files > 100` is a floor that stands in for "the scan found the sources"; `walk_visits` and `source_files` already throw on zero files (`source_tree.h:101-103`), so the floor adds only a check for 100 against a tree with several hundred files; no reason found for 100. `rules().size() > 100` at 479 is the same shape.
6 Display: not a display; the messages are text the fixer reads, typed in the test and not compared with a second owner.
8 Deleted: the function-scope end at 368 (`line == "}"`): a nested or CRLF-ended function would run on; no line covers a function whose closing brace is not at column 0 (reasoned); the `skip` of owner and exempt files at 371-372: no made-up case lists an owner file or an exempt file (reasoned); the `comment && !scan_comments` skip at 360: line 475 catches it.
9 Stand-in: `functions_found == 1` guards that a function-scoped row finds its function; good. The floors at 233 and 479 are stand-ins for "nothing was skipped".
10 Same rule: the single-file loop (see `flags_line`); `check_rows.cpp` calls it too.
11 Real: yes.
Verdict: Investigate. Claim not checked for the misses, and the `owner_files` and `exempt` skips and the function-end rule have no made-up case. The two floors at 233 and 479 are for the A49 record to judge; this record only names them.
Finding for A49 (not production code): `scan.cpp:233` and `:479` use floors where `source_files` already refuses an empty walk.

---

Counts: 22 records. Six are fuzz targets. Sixteen are engine records; seven of those cover two functions that cannot be judged apart (`load_rules` with `rule_files`, `parse_rule` with `parse_rules_file`, `copy_entry_name` with `clone_entry_name`, `compile_rule` with `flags_line`, `lexer_language` with `skipped_comment_line`, `walk_language` with `walk_reads_for`, `start_scan` with `scan_file`), so all 23 functions are covered. Verdicts: 7 Keep, 15 Investigate, 0 Rewrite, 0 Remove.

Splits for the user, each its own line:
- S1: `run_one` always returns 0, and one test checks that value (`tests/test_fuzz_regressions.cpp:57`).
- S2: the row scan and the owner-gone guard read comments by two rules (`skipped_comment_line` for C++ `//` only, `names_outside_comments` by `codelex` pieces).
- S3: `scan_single_file` (scan.cpp:394) repeats the row-scan loop outside the engine.
- S4: `tests/fuzz/fuzz_audio.cpp` fuzzes `decode_audio`, which only tests call, while the Preview plays through `open_song_stems`.
