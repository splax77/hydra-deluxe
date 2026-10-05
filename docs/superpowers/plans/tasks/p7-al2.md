Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task AL2: the store saves a song's audio length once per song, and no length comes from notes (D69, part 2 of 2; finding 62's leftover)

Task id: AL2. Base: **AL1's commit** (AL1 forks from main after phase 6's M6-J3; the main session names both hashes). You call AL1's song length owner. Branch: claude/p7-al2 (worktree `.claude\worktrees\p7-al2`, made as the preamble says). Session for your trailers: 6fc5f64c-7b23-4264-912b-0b78bf5cec9d.

Decision: D69 in `docs/audit/2026-10-03-fix-decisions.md`, which replaces D51 call 9 and D58 item 5. Read `p7-al1.md` first: it explains the split, the time base (chart time) and the owner you call. No file is in both tasks.

## Goal

When a chart is analyzed, Hydra saves its song's audio length once per song (D69 item 2). A result saved before this reads its audio once, on open, the way the old backfill read the chart. A chart with no readable audio has no length, so its Paths timeline places no dots and nothing falls back to the last note (item 3). Everything that works a length out from notes goes (item 4). No result row or blob changes; the results stamp stays "2.1.0".

## What the code does today

**The store.** `store::song_length_ms` (`record_store.cpp`) is the last timestamp's onset. `save_analysis` writes it to the analyzed difficulty's `songlength` row (`write_song_length`, replacing) and fills `songmeta.length_ms` when empty (the per-chart `set_song_length`). `add_song` writes it through `upsert_song`'s `COALESCE`. The `songlength` table (ST2, D51 call 9) is created in the constructor. `get_record` reads `read_song_length` for the key's chart mode and falls back to the `songmeta` value from `read_tempomap`, into `RecordLookup::song_length_ms`. `stored_versions.h` has no stamp for any of it, and every existing `songmeta.length_ms` holds a last-note value.

**The backfill.** `AppState::update_song_length` starts a `SongLengthJob` when the viewed Ready record has timing but no length. It remembers the md5 and chart mode it tried (`length_tried_`, `length_job_chartmode_`). The job parses the chart (`load_songpath`) and returns `store::song_length_ms`. The answer goes to the per-difficulty `set_song_length`, then to `viewed` and to parked lookups of that chart mode.

**The writers.** `run_batch` (`analysis.cpp`, in `hydra_core`) and `AppState::store_finished_analysis` both call `save_analysis` with the parsed `Song`. `hydra_core` cannot read audio; after AL1, `hydra_batch` links `hydra_audio`.

**The readers.** `paths_tab.cpp` hands `app.viewed.song_length_ms` to `build_activations` (`path_view.cpp`). That gives each activation its `song_fraction` and the view its `timeline_end`, only when `has_song_length` says the length is usable. With no length there are no dots and no end label, so the Paths tab needs no logic change.

**The docs.** `docs/UserGuide.md`'s Paths paragraph says the timeline "reads the chart once for its length". Its Preview paragraph says the scrubber measures against the last note, which AL1 changed. CONTEXT.md has no entry for a song's length.

## What changes

1. **One length per song, with a stamp.** `songmeta.length_ms` keeps its column and now holds AL1's song length (the audio's end in chart time). A new stamp `kSongLengthStamp` in `stored_versions.h` (a `StampRule<int, 1>`, written 1, accepting 1) carries a BUMP note naming AL1's owner, `open_stem_reader`'s lengths and `preview_audio_offset_ms`. It is stored in a new `songmeta` column, added by a `has_column` step with default 0. A row whose stamp is not current reads as "not read yet", so every last-note length in an existing file is ignored without being touched. A current row with no length means "read: this song has no readable audio".
2. **Schema steps.** `DROP TABLE IF EXISTS songlength` and the column add are idempotent steps in `RecordStore`'s constructor, beside the `has_column` steps already there. Nothing keys off `user_version`: under D53, J4-2 deletes its only write and nothing reads the slot. The order between J4-2 and AL2 does not matter; whichever lands second merges text only.
3. **Deleted.** `store::song_length_ms`, both `set_song_length` overloads, `read_song_length`, `write_song_length`, the `songlength` CREATE, and `upsert_song`'s length parameter with its `COALESCE`. `add_song` and `upsert_song` leave the length columns alone on update and write none on insert.
4. **Writers.** `save_analysis` is given the song's length and whether the audio was read. When it was read, the length and the stamp are written in the same transaction, on every analysis of any difficulty, because the audio belongs to the song (open question 5). When it was not read (an analysis with no audio reader, such as `hydra_bench` or a test), the stored length stays as it was. One backfill writer replaces both `set_song_length` overloads. It writes a length, or none, with the stamp, but only while the row's stamp is not current, so a slower backfill never overwrites an analysis. An unregistered song is left alone, as today.
5. **Reader.** `get_record` gives `RecordLookup::song_length_ms` the stored length only when the stamp is current, and says beside it whether the song was read, for the backfill. `SongMetaRead` carries the stamp. The comments on `RecordLookup::song_length_ms` and the header's table list say where the number comes from, and that nothing falls back.
6. **Analysis.** `BatchCallbacks` gains an optional song-length reader taking the notes path and the parsed `Song`; AL1's owner fits it as is. `run_batch`'s worker calls it after the analysis, on the pool's thread. No reader means "not read". `BatchJob` (`library_jobs.cpp`) and `hydra_batch` (`cli/batch.cpp`) pass AL1's owner. `AnalyzeJob` calls it on its own thread after `analyze_chart_file` and keeps the answer beside its result, and `store_finished_analysis` passes that answer to `save_analysis`.
7. **Backfill.** `SongLengthJob` keeps its one chart parse, now only for `chart_offset_s` (open question 2), and returns AL1's song length. A finished job that found no audio is still an answer: read, none. `update_song_length` keys `length_tried_` by md5 alone and drops `length_job_chartmode_`. It starts the job when the viewed Ready record was not read, writes through the backfill writer, and puts a found length on `viewed` and on every parked lookup of the same md5.
8. **`path_view`, comments only.** `build_activations`'s `song_length_ms` is the song's audio length (`RecordLookup::song_length_ms`), and none means no timeline.
9. **Docs.** UserGuide, Paths paragraph: the timeline runs to the end of the song's audio, so a long outro ends the last mark before the right edge (D69's closing line). A result saved before this version reads the song's audio once for its length, with nothing re-analyzed. A chart with no audio shows no marks. UserGuide, Preview paragraph: the scrubber measures against the end of the song's audio, like the Paths timeline. CONTEXT.md gains a **Song length** entry: how long the song's audio runs, in chart time, worked out by AL1's owner and stored once per song.

## Owned files (only these may change)

- `src/store/record_store.h`, `src/store/record_store.cpp` (the length lines only: J4-2 owns the SQL helpers and the `user_version` line, J4-3 the `DynamicsKey` operators)
- `src/store/stored_versions.h` (`kSongLengthStamp` only)
- `src/app/analysis.h`, `src/app/analysis.cpp` (the `BatchCallbacks` member and `run_batch`'s save)
- `src/ui/library_jobs.h`, `src/ui/library_jobs.cpp`, `src/cli/batch.cpp` (the reader they pass)
- `src/ui/app_state.h`, `src/ui/app_state.cpp` (`update_song_length`, `length_tried_`, `store_finished_analysis`; not J4-3's `dynamics_key` lines or J4-2's `list_chart_library` line)
- `src/ui/song_length_job.h`, `src/ui/song_length_job.cpp`
- `src/app/path_view.h`, `src/app/path_view.cpp` (comments only; J4-4 owns the id builder)
- `docs/UserGuide.md` (the two paragraphs), `CONTEXT.md` (the new entry)
- `tests/test_store.cpp`, `tests/test_app_state.cpp`, `tests/test_analysis.cpp`, `tests/ui/uitest_paths.cpp` (paths-rows's length setup and paths-length-backfill only)
- `tests/test_single_owner.cpp`: your rows at the end, plus removing `record_store.cpp`'s known-copy line from AL1's "When is the song's last note?" row.

Notes on the base. M6-J3 under AL1 holds J3-6's `record_store` and J3-3's `path_view` changes. Phase 6's J4 runs beside you on the same files. J4-2 owns `record_store.h/.cpp` and `test_store.cpp` except the length lines; it leaves `set_song_length`, the `songlength` table and the header comment near `set_song_length` to you. J4-3 changes `app_state.h`'s `dynamics_key` and `record_store.h`'s `DynamicsKey` operators. J4-4 changes `path_view`'s id builder and lines in `uitest_paths.cpp`. J4-6 changes temp-path lines in `test_app_state.cpp` and `test_analysis.cpp`. None touches `song_length_job` or `save_analysis`. Stay out of their lines; whichever lands second merges text only.

## Test cases to add or re-pin

Write each red first, then green, and keep the red line. Lengths you hand in are inputs; lengths read from audio come from calling AL1's owner. Use `temp_db`, `exec_on_file` and `scalar` in `test_store.cpp`, and `tests/audio_chart_fixtures.h` (AL1) for a folder with audio. No new helper.

In `tests/test_store.cpp`:
1. Re-pin `a stored song keeps its length, and an old songmeta row reads none` as "an analysis saves the song's length, and an unstamped length reads as not read". `save_analysis` with 4321 (read) reads 4321. `exec_on_file` zeroes the stamp column: it reads none, not read. The backfill writer gives 5000: it reads 5000. A second backfill write is ignored, and an unknown song is left alone. Red line: the new signature does not exist.
2. Replace `a song's length is stored per difficulty` with "one length per song: every difficulty reads the latest analysis". Save under "a" with 4321, then under "b" with 1234; both read 1234. Red line: "a" still reads its own.
3. Delete `an old database's one length shows until that difficulty is analyzed again` (D58 item 5 is replaced). In its place, "a file from before AL loses its songlength table and its last-note lengths": seed the table and a `length_ms` with `exec_on_file`, reopen, pin that `scalar` finds no `songlength` table and that `get_record` reads none, not read.
4. New: "an analysis with no audio reader leaves the song's length alone". Saved read with 4321, then saved not read: still 4321.
5. New: "a song with no readable audio reads as read, with no length".

In `tests/test_analysis.cpp`:
6. New: "run_batch saves the length its reader gives". Use a counting analyzer, as the dedupe case does, and a reader returning 4321: `get_record` reads 4321. With no reader: not read. Red line: the member does not exist.

In `tests/test_app_state.cpp`:
7. Re-pin `update_song_length stores the length under the viewed difficulty (D51 Q9)` as "the backfill reads a chart's audio once, and every difficulty shows it". Use AL1's short chart with long audio, plus a Ready record under a second chart mode. After the job, both lookups read AL1's owner called on that folder. Red line: the other chart mode reads none.
8. New: "a chart with no audio is read once and shows no length". On a corpus chart (no audio) the job finishes, `viewed` has no length and the store says read. Selecting it again starts no job. Red line: today the job finds a last-note length.
9. `open_chart_with_no_length` loses its `sequence.clear()` trick (`add_song` stores no length now). `any_job_running lists every background job` passes unchanged.

In `tests/ui/uitest_paths.cpp`:
10. `paths-length-backfill`: the GUI test library has no audio, so Burnout shows no "m96" end label and no marks. The backfill runs once, the record is not re-read, and no analyze job starts. Red line: "m96" is on screen today.
11. `paths-rows`: before each `check_marks`, set `app.viewed.song_length_ms` to a literal input past the chart's last activation (open question 4). The outline checks stay as they are.

Scan rows: "How long is this song?" gains the store's side: owner the backfill writer and `save_analysis`, flagging a `songmeta` length written anywhere else.

Existing cases that must pass unchanged: `activation timeline: onset over the song's length, and the end measure`, `a saved song's tempo map follows the latest analysis`, `save_analysis writes the song, the result and the count together`, `a save_analysis that fails leaves nothing behind`, `run_batch analyzes a chart found in two folders once`.

## Test filters you may run

- `build-cpp\Release\hydra_tests.exe` with `-sf=*test_store*`, `-sf=*test_app_state*`, `-sf=*test_analysis*`, `-sf=*test_path_view*` (run only), `-sf=*docs_match_code*`, `-tc="single-owner*"`, `-tc="hydra_batch reuses the GUI's scan cache"` (build `hydra_batch` warm first)
- `build-cpp\Release\hydra_uitest.exe --test <name>` for `paths-rows`, `paths-length-backfill`, `paths-list` and `preview-activation-jumps`

Nothing else. Never the full suite, never `hydra_uitest --all`.

## Stored results

No result row or blob changes, the results stamp stays "2.1.0", and nothing is re-analyzed. The `songlength` table is dropped. `songmeta` gains the stamp column. Every existing length reads as not read until its song is analyzed or opened. Say in your report how a database from the shipped 2.0.0 reads after this build opens it. A 2.0.0 build opening a file written after this reads `songmeta.length_ms` as its length, now the audio's; it has never heard of the stamp column.

## Not in this task

- The owner itself, the Preview and the sync rule (AL1). The `user_version` slot and the store's SQL helpers (J4-2).
- Re-reading a length when a loose folder's audio is swapped without the chart changing (open question 6).

## Done when

- No length is worked out from notes anywhere: `store::song_length_ms`, both `set_song_length` overloads and the `songlength` table are gone.
- An analysis saves the song's audio length once per song, with its stamp. A last-note length from an old file reads as not read, and the backfill reads the audio once.
- A chart with no readable audio shows no timeline dots.
- The cases above pass with their red lines recorded; the named existing cases and GUI scripts pass; the guide and CONTEXT.md say what the code does.
- `git diff --stat <base>..HEAD` lists only the owned files.

## Open questions (each with a recommended answer)

1. **How old last-note lengths are dropped** (needs the user: stored data). Recommended: the new stamp. Old values read as not read, and opening the song reads its audio once, the pattern ADR 0018 already uses for dynamics and the scan cache. The alternative clears the column once on open, which needs a marker so it runs only once: the same stamp by another name.
2. **The backfill still parses the chart, for its Offset only** (needs the user: D69 item 4 names that parse). Recommended: yes. The length is in chart time (AL1 question 2), and the Offset lives in the chart; what D69 removes is the length taken from notes. The cost matches today's (about 0.1 s on the largest chart), plus the audio open. The alternative stores audio time plus the offset, two stored numbers and a conversion in each reader.
3. **A silent chart is read once** (needs the user: how often Hydra re-reads). Recommended: the stamp records "read, none", so the backfill never re-reads it on later opens or sessions. A re-analysis reads again, so audio added later shows after the next analysis. The alternative retries once per session, as today. It picks up added audio without a re-analysis, but re-reads every silent chart each session, which is a whole-file read for a .sng or .srb.
4. **The GUI tests of timeline marks** (code-only). Recommended: `paths-rows` hands the open record a literal length, because the GUI test library has no audio and every Preview GUI test relies on that. `paths-length-backfill` pins the silent-chart behaviour. The alternative checks a silent audio file into `testdata/input` for Burnout and Beg, which changes what every Preview GUI test loads.
5. **Every analysis rewrites the song's length** (code-only). Recommended: yes. The audio belongs to the song, so there is no "last difficulty wins" problem, which was finding 62's complaint.
6. **Audio swapped after analysis** (code-only). A loose folder's hash covers the chart file, not its audio, so a replaced `song.ogg` keeps the old length until the next analysis. Recommended: accept, since D69 item 2 saves the length at analysis.

## Commits

One commit for the store and its writers, one for the backfill and the docs if you prefer. Trailers: `Task: AL2` plus the preamble's others, with the session above. Report as the preamble says, plus the 2.0.0 database note above.
