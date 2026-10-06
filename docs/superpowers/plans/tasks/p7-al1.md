Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task AL1: one rule for how long a song's audio runs, and the Preview reads it (D69, part 1 of 2)

Task id: AL1. Base: main after phase 6's M6-J3 (the main session names the hash at launch). Branch: claude/p7-al1 (worktree `.claude\worktrees\p7-al1`, made as the preamble says). Session for your trailers: 6fc5f64c-7b23-4264-912b-0b78bf5cec9d.

Decision: D69 in `docs/audit/2026-10-03-fix-decisions.md` (it replaces D51 call 9, D58 item 5 and D50 item 4's "the scrubber ends at the last note"). There is no plan row; D69 came after the plan. The whole change is about 750 lines, so it is split by file (D59 item 4). AL1 builds the length owner and points the Preview at it. AL2 (`p7-al2.md`) forks from your commit and moves the store, the analysis writers, the backfill and the Paths tab onto it. No file is in both.

## Goal

A song's length becomes how long its audio runs: the longest of its audio files, by the rule the Preview already uses for playback (D69 item 1). After this task one function answers "how long is this song" from a notes path, without decoding, and the Preview's scrubber, its gold marks and its SP meter read that answer. No score, path or stored record changes. The results stamp stays "2.1.0".

## What the code does today

**The audio end exists, under another name.** `PreviewLoadJob::open_audio` (`src/ui/preview_load_job.cpp`) gets the stems from `app::resolve_preview_stems` and opens each with `audio::open_stem_reader`, skipping a stem that fails. `PreviewLoadJob::run` then turns a negative chart offset into silence in front (`frames_of_ms` at `kOutRate`), builds an `audio::StreamMix` at `kOutRate`/`kOutChannels`, and asks `audio_end_chart_ms` for the mix's end in chart time. StreamMix's length is the front pad plus the longest stem after conversion (its constructor in `stream_mix.cpp`). That number, `audio_end_ms`, feeds `build_preview_scene` (where the beat lines stop) and `PreviewController::audio_end_ms_`.

**The song's length is the last note.** `build_preview_base` (`src/app/preview_view.cpp`) sets `PreviewScene::song_length_ms` from `store::song_length_ms(song)`, the last timestamp's onset. `close_curve` ends the SP meter curve there. `PreviewController::scrub_end_ms` hands `store::song_length_ms(*song_)` to `app::scrub_end_ms`, which falls back to the transport's playback range when the length is not usable (`has_song_length`). `PreviewController::poll` passes `scene_.song_length_ms` to `PreviewTransport::load` as `last_note_ms`, and the transport plays to the later of that and the audio's end (D48).

**The offset is locked inside a parse.** `resolve_preview_song` (`src/app/preview_source.cpp`) works out where chart time 0 sits in the audio, `preview_audio_offset_ms` of the song.ini or .sng delay and `Song::chart_offset_s`, as part of parsing the chart. Nothing can ask for it with a song already parsed.

**Layers.** `hydra_audio` links `hydra_core`, never the reverse, and `hydra_batch` links only `hydra_core` (CMakeLists.txt). The chart sync pair (`audio_ms_of_chart_ms`, `chart_ms_of_audio_ms`) and `audio_end_chart_ms` live in `src/ui/preview_transport.h`, so audio code cannot call them without reaching into `ui`.

**What an open costs.** `open_stem_reader` decodes nothing in the usual case. WAV and FLAC read their header. Vorbis reads its headers and its last page. Opus builds a page index by walking every page header, which reads the whole file. MP3 counts frames from their headers (dr_mp3) and builds seek points. Only a header that says 0 frames, a FLAC total of 0 or a Vorbis stream with no end page, is counted by decoding once (`CountedLength`, AU1, D51 call 19). A .sng's audio is unmasked from the container bytes (`sng_audio_from`) and a .srb's is decrypted from the section after the notes (`srb_audio_from`, loose files as its fallback); both need the container read once (`read_preview_container`). So the Preview's own open is already the cheapest correct read. Anything cheaper would be a second length rule per format.

## What changes

1. **The chart sync rule moves into `hydra_audio`.** The two sync functions and `audio_end_chart_ms` move from `preview_transport.h` to `src/audio/frames.h`, which already calls itself the one conversion every audio caller uses. `preview_transport.h` includes it, so its callers don't change. `kOutRate` and `kOutChannels` move beside the mix owner below, because the mix's length is counted in that format.
2. **One new `hydra_audio` file** (suggested `src/audio/song_audio.h`/`.cpp`) holds three owners. *Open a song's stems:* `open_stem_reader` on each, skipping one that fails, with an optional per-stem progress callback and `OpenCancelled` let through; the load job's loop calls it with its byte counter. *Mix them:* the negative-offset pad, the StreamMix and `audio_end_chart_ms`, returning the mix, the playhead's offset and the end in chart time; `PreviewLoadJob::run` calls it in place of its own lines. *The song's length:* from a notes path and its parsed `Song`, read the container once, find the stems, work out the offset (item 3), open and mix them, and return the end in chart time, or nothing when no stem opens. Its comment names it as the one answer to "how long is this song" (D69).
3. **`preview_source` answers "where does chart time 0 sit" for a parsed song** (notes path, container bytes, `chart_offset_s`). `resolve_preview_song` calls it, so the delay-or-Offset choice stays in one place and `preview_audio_offset_ms` stays the formula.
4. **The Preview reads the audio length.** `build_preview_base` sets `song_length_ms` from the `audio_end_ms` it is already given (0 with none), so the SP meter curve closes at the audio's end. `PreviewController::scrub_end_ms` passes `audio_end_ms_` to `app::scrub_end_ms`; its fallback to the playback range stays for a chart with no audio (open question 1). The transport keeps its playback rule (D48). Its last-note input comes from one new owner in `preview_view`, "when is the last drawn note" over a `PreviewScene`, which the beat-grid code calls too. After this nothing in `preview_view` or the controller calls `store::song_length_ms`; AL2 deletes it.
5. **Comments** on `PreviewScene::song_length_ms`, `scrub_end_ms`, `has_song_length`, `song_fraction`, `PreviewTransport::load` and the grid in `build_preview_base` say the length is the audio's end in chart time and name the owner from item 2.
6. **CMakeLists.txt:** the new file joins `hydra_audio`, the new test file joins `hydra_tests`, and `hydra_batch` links `hydra_audio`, so AL2 can save lengths in a batch (open question 3).

## Owned files (only these may change)

- `src/audio/song_audio.h`, `src/audio/song_audio.cpp` (new), `src/audio/frames.h`
- `src/ui/preview_transport.h` (the moved rule; `.cpp` only if an include needs it), `src/ui/preview_load_job.h`, `src/ui/preview_load_job.cpp`, `src/ui/preview_controller.h`, `src/ui/preview_controller.cpp`
- `src/app/preview_source.h`, `src/app/preview_source.cpp`, `src/app/preview_view.h`, `src/app/preview_view.cpp`
- `CMakeLists.txt` (the two new files and `hydra_batch`'s link line)
- `tests/audio_chart_fixtures.h` (new), `tests/test_song_audio.cpp` (new), `tests/test_preview_view.cpp`, `tests/test_preview_controller.cpp`, `tests/test_preview_transport.cpp`, `tests/display_fixtures.h`, `tests/test_preview_golden.cpp` (the dump's time box line only)
- `tests/test_single_owner.cpp`: your rows at the end, plus re-pointing the existing "When is the song's last note?" row.

Not yours: `src/store/*`, `src/app/analysis.*`, `src/app/path_view.*`, `src/ui/app_state.*`, `src/ui/song_length_job.*`, `src/ui/library_jobs.*`, `src/cli/*`, `docs/*` (all AL2's); `tests/ui/uitest_preview.cpp` (J4-4's this wave).

Notes on the base. AL1 forks from main after M6-J3, which holds J3-4's `preview_view` and J3-5's controller, load job and transport changes; read those files at your fork, not main's today. Phase 6's J4 runs beside you. J4-5 removes one `preview_view` field and owns lines in `test_preview_view.cpp`. J4-6 changes temp-path lines in `test_preview_controller.cpp`, `test_preview_load_progress.cpp` and `test_preview_source.cpp`. Stay out of their lines; whichever lands second merges text only.

## Test cases to add or re-pin

Write each red first, then green, and keep the red line. Pin the run's numbers as literals.

Move `test_preview_controller.cpp`'s `short_chart_with_long_audio` and `chart_with_audio`, with the copy and temp-folder helpers they need, into `tests/audio_chart_fixtures.h`, so AL1's and AL2's cases build one kind of folder one way. If J4-6's `tests/temp_util.h` is on your base, the moved helpers call it.

In `tests/test_song_audio.cpp` (new):
1. `song length: a loose chart's length is its audio's end in chart time`. The short chart with the 5 s sine reads about 5000 ms (pin the run's literal). Red line: the name does not exist.
2. `song length: the chart's offset moves the end`. The same folder with a song.ini delay, once positive and once negative. Each end moves by the delay, in `preview_audio_offset_ms`'s direction; pin both literals.
3. `song length: a chart with no readable audio has none`. No audio file: none. A `song.ogg` holding junk bytes: none.
4. `song length: the Preview's audio end and the song length agree`. Load the folder through `PreviewController` and compare its scrub end with case 1's call. Both call production.

In `tests/test_preview_controller.cpp`:
5. Re-pin `scrub marks: the Preview's scrubber ends at the last note while the audio plays on` as "...ends at the audio's end". `scrub_end_ms()` is 5000, not 100, and a drag to it seeks to 5000. Red line: 100.
6. Re-pin `a jump past the end of the Preview stops at the audio's end`: `scrub_end_ms()` reads 5000 too.

In `tests/test_preview_view.cpp`:
7. `build_preview_scene: notes carry lane and drum attributes`: with no audio end given, `song_length_ms` is 0, not 750.
8. Rewrite `build_preview_scene: the song length is the store's, even past the last drawn note` as "the song length is the audio's end". With an audio end it reads that end; without, 0. Its beat-line checks stay.
9. `an analyzed chart's overlay matches its path` and `score box: the analyzed chart ends on the path's total` stop reading `song_length_ms` as the last note; they ask the new last-note owner.
10. `scrub marks: an activation sits at the Paths timeline's fraction when the audio outlasts the notes`: both bars measure against `c.audio_end_ms`, and the mark is 0.125 (750 ms of 6000). Red line: 0.75.
11. `scrub marks: a playhead past the last note parks the thumb at the right end`: the right end is the audio's end, 6000, and the thumb follows the playhead to it.

In `tests/test_preview_transport.cpp`:
12. `scrubber: a drag to the right end seeks to the last note, not the audio end` becomes "...seeks to the audio's end": the drag lands at 6000. The no-length line stays.

`tests/display_fixtures.h`: `AudioTailChart::last_note_ms` is the literal 1000.0 its comment already states, not `store::song_length_ms`. Its sanity case follows.

Scan rows: re-point "When is the song's last note?" to the new `preview_view` owner. Record's `store::song_length_ms` line stays as a known copy that AL2 removes. Add "How long is this song?": owner the song length function, flagging a `StreamMix` built outside `song_audio.cpp`, with the load job's old `make_unique<audio::StreamMix>` line as its `must_match`. The transport's own `audio_end_chart_ms` call on its playhead is the playback range, not a song length; list it as allowed.

Existing cases that must pass unchanged: `a Preview load turns a negative chart offset into front silence`, every `sp meter curve*` case, `song_fraction: *`, and the stream mix and stem reader suites.

## Test filters you may run

- `build-cpp\Release\hydra_tests.exe` with `-sf=*test_song_audio*`, `-sf=*test_preview_view*`, `-sf=*preview_controller*`, `-sf=*preview_transport*`, `-sf=*preview_load_progress*`, `-sf=*preview_source*`, `-sf=*stream_mix*` and `-sf=*stem_reader*` (run only), `-tc="sp meter curve*"`, `-tc="fixtures:*"`, `-tc="single-owner*"`
- `build-cpp\Release\hydra_uitest.exe --test <name>` for `preview-activation-jumps`, `preview-controls`, `preview-path-overlay` and `preview-drain-box` (build `hydra_uitest` first)
- one warm `.\build_cpp.ps1 -Target hydra_batch`, to prove the new link

Nothing else. Never the full suite, never `hydra_uitest --all`.

## Stored results

None change. The store is untouched, and no stamp moves.

## Not in this task

- The stored length, its stamp, the `songlength` table, `store::song_length_ms`, `save_analysis`, the backfill, `run_batch`, the Paths tab and the docs (AL2).
- What a damaged stem does (AU1, merged) and the FLAC count (D51 call 19).

## Done when

- One function returns a song's length from its audio without decoding; the load job and the length share the opener and the mix step; the sync rule lives in `hydra_audio`; `preview_source` answers the offset for a parsed song.
- The Preview's scrubber, marks and SP meter read the audio's end; nothing in the Preview calls `store::song_length_ms`.
- The cases above pass with their red lines recorded, the named suites pass, and the four GUI scripts pass.
- `git diff --stat <base>..HEAD` lists only the owned files.

## Open questions (each with a recommended answer)

1. **A chart with no readable audio, in the Preview** (needs the user: what the Preview shows). Recommended: it still plays and scrubs to its last drawn note, with its gold marks on that range, as today. That range is the transport's playback rule (D48), not a song length; only the Paths timeline drops its dots (D69 item 3, AL2). The alternative hides the Preview's marks too. Its scrubber would still need the last note as its range. Every Preview GUI test waits for Burnout's three marks, and the GUI test library has no audio, so all of those would change.
2. **The time base** (code-only). Recommended: the length is the audio's end in chart time, `audio_end_chart_ms`'s answer. Every reader measures in chart time: the Paths timeline divides a chart-time onset by the length and turns it into a tick with `display_tick_at_ms`, and the Preview's clock runs in chart time. The writer converts once. The cost: the length depends on the chart's Offset and delay, so AL2's backfill still reads the chart for its Offset. The alternative stores audio time, which needs the offset stored too and a conversion in every reader.
3. **`hydra_batch` links `hydra_audio`** (code-only). Recommended: yes, so a batch saves lengths as D69 item 2 says. The libraries are static, so the installer ships no new file. The alternative saves none in a batch and leaves each chart to the GUI's backfill on first open.
4. **Where the sync rule lives** (code-only). Recommended: `audio/frames.h`. The alternative is a new `audio/chart_sync.h`; either way `preview_transport.h` includes it.
5. **The Preview reads its own audio end, not the stored length** (code-only). Recommended: yes. It is the same owner and the same answer, and it is what plays; the stored length serves the Paths tab without opening audio.

## Commits

One commit for the owner and the load job, one for the Preview readers if you prefer. Trailers: `Task: AL1` plus the preamble's others, with the session above. Report as the preamble says, plus the per-format open costs you saw if any differ from "What the code does today".
