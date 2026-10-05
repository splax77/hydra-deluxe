Read `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md` first; it holds the shared rules. This brief adds the rest.

# Task SL1: a song's length comes from its chart metadata, with the last Expert drum note as the backup (D75)

Task id: SL1. Base: main at the commit that adds this brief (the main session names the hash at launch). Branch: `claude/sl1`. Worktree: `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\sl1`. Session for your trailers: e5c08fba-66a5-441a-a12a-bb4d1d11dc99. Model line: `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.

Decision: D75 in `docs/audit/2026-10-03-fix-decisions.md`. Read it, then D69 and D70 just above it, which it replaces in part. Every display change below is one D75 names. If you find another, stop and report it in game terms.

## Why

Analysis got 2.7 times slower than 2.0.0 because it opens every song's audio to read its length. The main session timed the whole library (19,906 charts, fresh database each run): 2.0.0 took 25.7 s to analyze, main 56-70 s, and main with the length read switched off 16-18 s. Almost every chart already states its length in its metadata. After this task, no length read opens an audio file.

## Worktree and builds

Make the worktree first: `git -C C:\Users\Patrick\Downloads\Hydra\hydra-test worktree add C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\sl1 -b claude/sl1 <base>`. Work, build and commit only there. Never edit the main checkout.

First build: `pwsh -NoProfile -File <worktree>\tools\build_slot.ps1 -Repo <worktree> -Target hydra_tests`. Let it wait for a slot. Later builds: `.\build_cpp.ps1 -Target <target>` from the worktree.

## The rule (D75)

1. **The metadata length.** A folder chart's song.ini `song_length`, a .sng's `song_length` metadata key, or a .srb's `song_length_ms` field. All three are milliseconds. A value that is missing, empty, zero, negative or not a number counts as not stated.
2. **The backup.** With no stated length, the length is the start time of the last note in the Expert drums chart, counting every note in the file, 2x kick included. It must not depend on the Pro or 2x settings, or on the difficulty the user analyzes at.
3. **Chart time.** A metadata length counts from the start of the audio. Move it into chart time with the chart's delay-or-Offset rule: `app::chart_audio_offset_ms` / `preview_audio_offset_ms` for the offset, and `chart_ms_of_audio_ms` in `src/audio/frames.h` for the conversion. Call those owners; don't restate the direction. The last-note backup is already in chart time.
4. **One length everywhere.** The Paths timeline, the Preview's scrub bar and the Preview's SP meter curve all read it. The Preview's playback range stays its own rule (D48: the later of the last note and the audio's end), so audio past the length still plays.
5. **No audio is opened for a length,** in analysis, `hydra_batch` or the backfill.

## What the code does today

**The length owner reads audio.** `audio::song_length_ms` (`src/audio/song_audio.cpp`) reads a .sng/.srb container whole, works out the offset (which lists the folder and reads song.ini again), maps every stem, opens a full stem reader on each and mixes them, then returns the mix's end in chart time. `app::song_length_found` and `read_song_length_or_keep` (`src/app/analysis.cpp`) wrap it. `run_batch` calls it on each worker through `BatchCallbacks::read_song_length`. `hydra_batch` (`src/cli/batch.cpp`) and the GUI's library analyze (`src/ui/library_jobs.cpp`, around lines 301 and 349) both pass `audio::song_length_ms`. `SongLengthJob` (`src/ui/song_length_job.cpp`) is the backfill for a stored song whose length stamp is old: it parses the chart and calls the same function.

**Where it's stored.** `songmeta.length_ms` with `songmeta.length_version`, stamped by `kSongLengthStamp` in `src/store/stored_versions.h` (now 1, "the audio's end in chart time"). `store::SongLength` carries "read, value or none". `RecordLookup::song_length_ms` hands it to the Paths tab (`src/ui/app_state.cpp`, `src/app/path_view.cpp`).

**The Preview reads its own audio end.** `build_preview_base` sets `PreviewScene::song_length_ms` from the audio end the load hands it, and `PreviewController::scrub_end_ms` passes `audio_end_ms_` to `app::scrub_end_ms` (AL1 open question 5). Under D75 both read the song's length instead.

**Metadata is already read once, at the scan.** `discover_charts` (`src/app/analysis.cpp`, around line 470) reads each chart's title, artist and charter: song.ini through `read_metadata_ini` (which calls `read_song_ini_keys`, the one song.ini reader), a .sng through `parse_sng_metadata` over the head bytes captured while hashing, and a .srb through `parse_srb_metadata`. Results are cached in the chart library cache and reused while a chart's signature is unchanged (`kChartMetaStamp`).

**The .srb metadata reader stops early.** `srb_parse_metadata` (`src/parse/srb.h`/`.cpp`) reads the eight strings and skips the rest. The fields after them, in order, all little-endian (from the user's .srb format reference, https://claude.ai/artifact/FPZtMi7XQ3R5iVfTASnK7n): `difficulty`, 12 signed bytes; `preview_start_ms`, i32; `icon`, a string (i32 byte count, then UTF-8); `playlist_track`, i32; `album_track`, i32; `song_length_ms`, i32; then a 16-byte checksum and the table of contents. Biology.srb stores 196,905 there. Two of the 30 shipped files store 0 (unknown). The version u32 (20210228) sits before the strings; check how the current reader treats the 4-byte prefix before relying on offsets.

## What changes

1. **One owner answers "how long is this song" (D75).** Given the metadata length (or none), the chart's offset and the parsed chart, it returns the length in chart time, or none. Suggested home: next to `song_length_found` in `src/app`, or a small new file if that fits the layers better. Its comment names D75. Delete `audio::song_length_ms` and the `read_song_length` callback plumbing once nothing calls them.
2. **The metadata length is read where metadata is read now.** Recommended: capture it at the scan, with title, artist and charter, through the same three readers, and carry it on `ScanItem` and in the chart library cache (bump `kChartMetaStamp` so cached entries are read once more). Then analysis, the batch and the backfill read no extra file. If that proves wrong, say why in your report and pick the next cheapest place; the hard rule is that no file is read twice for it and no audio is opened. Parse the number in one place for all three formats.
3. **The offset without a second song.ini read.** Today the offset lists the folder and reads song.ini again (40 s of thread time on the library). Read the delay where the metadata length is read, and feed both into the owner. `preview_audio_offset_ms` stays the formula.
4. **The backup needs Expert drum notes.** Analysis already parses the chart at the user's settings. When the metadata states no length and that parse is not Expert with 2x kick, parse once more at Expert with 2x. Only charts with no stated length pay for it (6 folder charts and 2 .srb in the user's library).
5. **The length stamp moves to 2** in `stored_versions.h`, with its comment saying what 2 means (D75). Old lengths then read as not read, and `SongLengthJob` fills them on first open through the new owner, with no audio read. This task is allowed to move this stamp and `kChartMetaStamp`. The results stamp does not move. No score, path or record changes.
6. **The Preview reads the song's length** for its scrub end and SP meter curve, not its audio end. `app::scrub_end_ms`'s fallback to the playback range stays for a song with no length.
7. **`hydra_batch` drops its `hydra_audio` link** if nothing else needs it (AL1 added it only for the length).
8. **Comments and docs.** Every comment that says the length is the audio's end (`song_audio.h`, `analysis.h`, `record_store.h`, `stored_versions.h`, `preview_view.h`, `preview_controller.h`, `preview_transport.h`, `song_length_job.h`, `path_view.h`, `library_jobs.h`) points at the new owner and D75 instead. `CONTEXT.md` and `docs/UserGuide.md` (around line 156) follow if they describe the length.

## Owned files

Anything this change needs under `src/`, `tests/`, `CMakeLists.txt`, `CONTEXT.md` and `docs/UserGuide.md`. No other task is running, so ownership is not split. Do not edit `docs/audit/*` or other briefs; report doc changes you think the audit files need. Your report lists every file you touched.

## Tests

Write each red first, keep the red line, then make it green. Pin literals from a run or call production; never work the expected value out again in the test. Reuse `tests/audio_chart_fixtures.h`, `tests/record_fixtures.h`, `tests/display_fixtures.h` and the existing temp-folder helpers; grep before writing a helper.

Cases to add (names are suggestions):
1. A folder chart whose song.ini says `song_length = 200000` reads 200,000 in chart time with no delay, and moves by the delay when song.ini has one (once positive, once negative; pin the literals).
2. `song_length =` empty, absent, `0`, `-5` and `abc` each fall back to the last Expert drum note. A chart whose last note is a 2x kick reads that note's time even when the settings have 2x off. A chart analyzed at Hard reads the Expert note.
3. A .sng with a `song_length` key reads it. A .srb's `song_length_ms` is parsed: pin Biology's 196,905 if a shipped .srb fixture exists in `testdata/`, or build the metadata block in the test; and 0 falls back.
4. No audio is opened: a folder chart with metadata and no audio file at all has a length, and so does one whose `song.ogg` is junk bytes. Its Paths timeline places activation dots (re-pin AL2's "no audio, no dots" cases to D75).
5. The Preview's scrub end and SP meter end equal the stored length for the same chart, while playback still runs to the audio's end. Re-pin AL1's cases that expect the audio's end.
6. `SongLengthJob` on a stamp-1 row returns the new length without reading audio.
7. Single-owner scan rows: re-point "How long is this song?" to the new owner, and flag any length worked out from a StreamMix or a stem reader.

Run only: `build-cpp\Release\hydra_tests.exe` with `-sf=` filters for each test file you add or change, plus `-tc="single-owner*"` and `-tc="fixtures:*"`. GUI scripts (build `hydra_uitest` first): `preview-controls`, `preview-activation-jumps`, `preview-drain-box`, `preview-path-overlay`, plus any Paths-tab script whose name contains `timeline` or `paths` (list them with `hydra_uitest --list` and run just those). Never the full suite; never `--all`. The main session runs the full suite and the library timing at merge.

## Done when

- No length read opens an audio file; `audio::song_length_ms` is gone.
- The length follows D75 items 1-4 for all three formats, from one owner.
- The stamps move as item 5 says, and nothing else stored changes.
- The cases above pass with their red lines recorded, and the named GUI scripts pass.

## Open questions (code-only; take the recommendation unless the code shows it's wrong, and say so in your report)

1. **Where the metadata length is captured.** Recommended: at the scan, as "What changes" item 2 says.
2. **A chart with no stated length and no Expert drum notes.** Recommended: no length, as today's "none" (`app::scrub_end_ms` and the Paths tab already handle none). Analysis fails such a chart anyway ("no Expert Pro Drums notes").
3. **Rows stored before the scan carried a length.** Recommended: the backfill reads the metadata itself through the same reader when the cache has no length, so an old library works without a rescan.

## Commits and report

Commit after each finished step, with `Task: SL1` and the preamble's other trailers. Report as the preamble says, and add: the owner's name and file, where the metadata length is read, the answer you took on each open question, and the files you touched.
