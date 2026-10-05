Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task ER1: every failure that has a sentence is thrown with its kind (finding 193 and R7.20, part 1 of 2)

Task id: ER1. Base: **main after AL2 merges** (phase 6 finished at e4833e0; the main session names the hash at launch). Branch: claude/p7-er1 (worktree `.claude\worktrees\p7-er1`, made as the preamble says). Session for your trailers: 6fc5f64c-7b23-4264-912b-0b78bf5cec9d.

Plan row: `docs/superpowers/plans/2026-10-04-phase-7.md`, "Wave 4: errors carry their kind". The change is about 850 lines in all, so it is split by folder under D59 item 4. ER1 builds the owner and gives a kind to every thrower below `src/app` (core, parse, store, search, net, audio, render). ER2 (`p7-er2.md`) forks from your commit and moves the screens, the batch and the app-folder throwers onto it, then deletes the text matcher. No file is in both tasks, except that ER2 later deletes the two text helpers you leave in place. Decision: D51 call 25 in `docs/audit/2026-10-03-fix-decisions.md` ("Errors carry their kind, and each kind has one plain sentence. A Preview asset problem reads 'Reinstall Hydra'"). Finding texts: `docs/audit/2026-10-03-derivation-audit.md`, headings `#### 193.` and `#### R7.20`.

## Goal

Today Hydra picks the plain sentence for a failure by reading the exception's words. ER1 makes the thrower say what kind of failure it is. Each kind maps to its sentence in one switch, in `app::plain_error`. The thrown text stays byte for byte, so every details line and every CLI line reads as before. The text matcher stays for one merge, only for errors that still arrive as text (the batch) or are still untyped (the app-folder throwers); ER2 removes both. No score, path or stored record changes; the results stamp stays "2.1.0".

## What the code does today

Line numbers are from main at e4833e0. AL2 moves lines in `record_store.cpp`.

**The matcher.** `plain_error_text` (`src/app/user_messages.cpp`, lines 80 to 156) compares the raw text against about 45 copied literals, by exact text or prefix. `plain_error` (line 158) checks two types first (`std::bad_alloc`, `RulesFileError`), then the matcher, then three more types (`ChartFileError`, `MidiError`, `store::SerializeError`) whose every message reads the same. `is_no_notes_message` (line 74) copies the shape of `no_notes_message`'s sentence. `is_timing_refusal` (`src/parse/song.cpp`, line 23) matches the three refusal prefixes from `song.h`.

**The throwers, by kind.** Each of these is matched by its words today:
- Song file missing: `read_file_bytes` and `file_size_bytes` in `src/core/winstr.cpp` ("cannot open file: ", "cannot read file size: "), and `src/audio/mapped_file.cpp` (four "cannot open file: ").
- Unreadable chart: `src/parse/srb.cpp` (six throws), `src/parse/song.cpp` ("No chart files found in SNG file.", "Truncated SNG file.", "unexpected chart type: "), and the typed `ChartFileError` (`src/core/model.h`) and `MidiError` (`src/parse/midi.h`).
- Refused timing: `check_timing_maps` and the time-signature throw in `song.cpp` (lines 38 to 52 and 187). The sentence quotes the raw text.
- Already plain: `NoNotesError` (`src/parse/song.h`, line 88).
- Database: `RecordStore`'s constructor ("failed to open database "), and the write throws in `src/store/record_store.cpp` ("prepare failed: ", "sqlite exec failed: ", "meta_set failed: ", "put_dynamics failed: ", "add_song failed: ", "add_row ", "reindex failed: ", "rebuild_chart_library failed: ").
- Stored result: the typed `store::SerializeError` (`serialize.cpp`, `path_codec.cpp`).
- Broken search: "search reached a broken state" (`src/search/engine.cpp`, line 2038).
- Network: `fail` in `src/net/dmbot_client.cpp` (line 38) appends " (error N)", and the matcher spots 12002 (`ERROR_WINHTTP_TIMEOUT`) inside the text. Also "leaderboard returned HTTP N", the two bad-reply throws, and five "cancelled" throws.
- Damaged audio: every "decode_audio: " throw (`decode.cpp`, `stem_reader.cpp`, `opus_reader.cpp`, `vorbis_reader.cpp`, `ma_reader.cpp`) and the three "StreamMix: " throws in `stream_mix.cpp`.
- Preview assets: "PreviewRenderer: missing ..." (`src/render/preview_renderer.cpp`, lines 162, 191, 294 and 302) and "3d-config.json: " (`preview_config.cpp`, lines 62 and 97).
- Cancelled: `audio::OpenCancelled` (`stem_reader.h`) reads "cancelled".

**Throwers the matcher misses** (finding 193 and R7.20). These fall to "Something went wrong" today:
- Database writes "fill_missing_stars failed: ", "deleting Auto results failed: ", "Auto cleanup path gc failed: ", and AL2's new "saving the song's length failed: ".
- Preview assets "PreviewRenderer: undecodable texture", "PreviewRenderer: shader ..." (line 98, a shader that won't compile), and `src/render/obj_loader.cpp`'s "obj: ... index out of range" and "obj: no faces".

**Third-party text.** None reaches the matcher on its own. sqlite's text is always inside Hydra's own prefix. The leaderboard's JSON errors are rethrown as Hydra's words (`dmbot_client.cpp`, line 281). `preview_config.cpp` wraps the JSON library's errors (line 96). The SRB reader words its own miniz failures. `std::bad_alloc` is matched by type.

**Not a sentence.** The Direct3D calls through `check` (`preview_renderer.cpp`, line 34) are a graphics-card failure, not a missing file. They have no sentence today and get none here (ER2's open question 3 asks about the screen).

## What changes

1. **The kind.** A new header `src/core/error_kind.h` holds `enum class ErrorKind` and one exception type that carries a kind and keeps its text as `what()`. Code below `src/app` never includes `app/` (checked: no file in core, parse, store, search, audio, render or net does), so the kind lives in core (open question 1). The kinds are the sentences `user_messages.cpp` already has: Cancelled, DatabaseOpen, DatabaseWrite, SongFileMissing, HashFailed, ChartUnreadable, ChartTimingRefused, AlreadyPlain, SearchBroken, NetUnreachable, NetTimeout, NetHttpStatus, NetBadReply, NoScores, NoRecords, ReportWrite, RulesFile, StoredResult, AudioDecode and PreviewAssets. An HTTP failure carries its status code as a number, which only NetHttpStatus reads (open question 3). Out of memory stays a type check on `std::bad_alloc`, because the standard library throws it.
2. **The typed errors join it.** `ChartFileError` carries ChartUnreadable, or ChartTimingRefused when `check_timing_maps` or the time-signature check throws it. `NoNotesError` carries AlreadyPlain. `MidiError` carries ChartUnreadable. `store::SerializeError` carries StoredResult. `audio::OpenCancelled` carries Cancelled. Their names, their catch sites and their texts stay.
3. **The throwers.** Each thrower listed above throws its kind with its old words, byte for byte. That covers the four misses too: the database ones become DatabaseWrite, and the renderer and `.obj` ones become PreviewAssets (open question 4). `fail` picks NetTimeout when the WinHTTP code is 12002 and NetUnreachable otherwise. That choice now happens at the call that has the code, so nothing reads the number back out of text. A file-level chart throw in `song.cpp` or `srb.cpp` that is a plain `std::runtime_error` today becomes the kinded type, **not** a `ChartFileError`. The two per-note catches in `song.cpp` (lines 877 and 1392) swallow `ChartFileError`, and widening what they swallow would change which charts load.
4. **The switch.** `plain_error` asks the kind first: one `switch` returns each kind's sentence (the constants already in `user_messages.cpp`, word for word). ChartTimingRefused wraps the raw text in today's "Hydra can't analyze this chart because ... Fix that line ..." sentence. AlreadyPlain returns the text as it is. Only an exception with no kind goes on to the old type checks and `plain_error_text`. Comments name the switch as the owner. They never list what the matcher matches.
5. **What stays for ER2.** `plain_error_text`, `is_no_notes_message` and `is_timing_refusal` stay as they are, because the batch still hands failures over as text and the app-folder throwers are still untyped. Each line the new scan row flags goes in `known_copies` with "task ER2" as its remover.

## Owned files (only these may change)

- `src/core/error_kind.h` (new), `src/core/model.h` (`ChartFileError` only), `src/core/winstr.cpp` (the three file throws)
- `src/app/user_messages.h`, `src/app/user_messages.cpp`
- `src/parse/song.h` (`NoNotesError`'s kind only), `src/parse/song.cpp` (the throw sites only), `src/parse/srb.cpp`, `src/parse/midi.h`
- `src/store/serialize.h`, `src/store/record_store.cpp` (the throw sites only)
- `src/search/engine.cpp` (line 2038 only)
- `src/net/dmbot_client.cpp`
- `src/audio/stem_reader.h` (`OpenCancelled`), `decode.cpp`, `stem_reader.cpp`, `opus_reader.cpp`, `vorbis_reader.cpp`, `ma_reader.cpp`, `stream_mix.cpp`, `mapped_file.cpp`
- `src/render/preview_renderer.cpp`, `preview_config.cpp`, `obj_loader.cpp`
- `tests/test_user_messages.cpp`, `tests/test_store.cpp`, `tests/test_winstr.cpp`, `tests/test_srb.cpp`, `tests/test_obj_loader.cpp`, `tests/test_preview_renderer.cpp`, `tests/test_preview_config.cpp`, `tests/test_stem_reader.cpp`
- `tests/test_single_owner.cpp`: your row at the end of `rules()`, and your `known_copies` lines.

Not yours: everything under `src/app` except `user_messages.*`, everything under `src/ui` and `src/cli` (ER2). `src/audio/mixer.cpp`'s "mix_stems:" throws are a test reference with no user (R7.20), so leave them untyped.

Notes on the base. AL2 rewrote the song-length lines of `record_store.cpp`; re-read the file at your fork for its new throw. Phase 6's folds have all landed, so every other owned file is main's text.

## Test cases with red lines

Write each red first, then green, and keep the red line. Pin sentences as literals, the way `test_user_messages.cpp` does today. Never compute one again. Use the store's trigger technique from `a save_analysis that fails leaves nothing behind` (`exec_on_file`, `testtemp::temp_path`). No new helper.

In `tests/test_user_messages.cpp`:
1. New: "user_messages: a kinded error reads its kind's sentence, whatever its words". For every kind, an error of that kind with the text "x" reads the kind's literal sentence. NetHttpStatus with 503 reads "dmleaderboards returned an error (HTTP 503). Try again later.", ChartTimingRefused quotes "x", and AlreadyPlain returns "x". Red line: `ErrorKind` does not exist.
2. Keep every existing case green unchanged. They throw untyped `std::runtime_error`s, which still reach the matcher until ER2.

In the throwers' own files, each case calls the real code and pins `plain_error` of what it throws:
3. `test_store.cpp`, new: "a database write the old matcher missed reads as a database error". A trigger refuses AL2's song-length write inside `save_analysis` (find its statement at your fork). The caught error reads the database-write sentence. Red line: "Something went wrong. Try again, and if it keeps happening, report it with the details below."
4. `test_obj_loader.cpp`, new: "an .obj with no faces reads as a Preview asset problem". `load_obj` of text with no faces reads "Some of Hydra's Preview files are missing. Reinstall Hydra to restore them." Red line: the "Something went wrong" fallback.
5. `test_preview_renderer.cpp`: extend `PreviewRenderer: a missing asset dir is a clear error` to pin the Preview-assets sentence. This is green on the base through the text; your report says so.
6. `test_winstr.cpp`, new: "a missing file reads as a moved song file". `read_file_bytes` of a path that does not exist reads the song-file-missing sentence, and `what()` still starts "cannot open file: ". Green on the base through the text; it pins that the words stayed.
7. `test_srb.cpp` and `test_stem_reader.cpp`: one case each, pinning the chart-unreadable and audio-decode sentences on a real truncated `.srb` and an unrecognized container. Green on the base; it pins the kind.

Existing cases that must pass unchanged: all of `test_user_messages.cpp`, `user_messages: a real refused load shows the tick sentence`, `the no-notes error carries no_notes_message's sentence` (`test_song.cpp`), the `3d-config.json: missing key` cases in `test_preview_config.cpp`, and `jobs: a failed leaderboard fetch says what to do` (run it with `-sf=*test_library_jobs*`).

Scan row (end of `rules()` in `tests/test_single_owner.cpp`, scope `src`): "Which plain sentence does this failure show?", owner `plain_error`'s switch on `ErrorKind` in `src/app/user_messages.cpp`. It flags exception text compared against a literal: a `what` or `error` string, or `what()` or `error()`, met with `==` or `!=`, with `starts_with`, `ends_with` or `starts_with_any`, with `.find(` or `.compare(`, or through a `substr` compared to a prefix. Give it `must_match` examples from today's lines: `if (what == "cancelled") return kStopped;`, `if (starts_with(what, "cannot write ")) return kReportWrite;`, `if (what.find("(error 12002)") != std::string_view::npos) return kNetTimeout;` and `if (what.substr(0, prefix.size()) == prefix) return true;`. Give it `must_not_match` lines like `error_ = e.what();` and `if (job.error().empty()) return;`. Every line it flags in `user_messages.cpp`, and the one in `song.cpp`'s `is_timing_refusal`, goes in `known_copies` with "task ER2 (the text matcher is deleted once the batch and the screens carry kinds)".

## Test filters

- `build-cpp\Release\hydra_tests.exe` with `-sf=*test_user_messages*`, `-sf=*test_store*`, `-sf=*test_winstr*`, `-sf=*test_srb*`, `-sf=*test_sng*`, `-sf=*test_midi*`, `-sf=*test_song.cpp*`, `-sf=*test_path_codec*`, `-sf=*test_stem_reader*`, `-sf=*test_stream_mix*`, `-sf=*test_audio_mixer*`, `-sf=*test_obj_loader*`, `-sf=*test_preview_config*`, `-sf=*test_preview_renderer*`, `-sf=*test_dm_report*`, `-sf=*test_library_jobs*` (run only), `-tc="single-owner*"`

Nothing else. Never the full suite, never `hydra_uitest --all`.

## Stored results

None change. No thrown text changes either: details lines, the batch's raw lines and hydra_batch's output read the same.

## Not in this task

- The screens (`preview_tab.cpp`, `preview_controller`, `dynamics_tab.cpp`, the batch strip), the batch's text hand-off in `run_batch`, the app-folder throwers (`analysis.cpp`'s hashing, `report_files.cpp`, `rules_file.h`, `dm_report.cpp`), the UI throwers (`job_base.h`'s `JobCancelled`, `dm_jobs.cpp`, `library_jobs.cpp`) and deleting the matcher: ER2.
- The Direct3D failures in `check`: they get no kind. ER2's open question 3 asks how the Preview shows them.

## Done when

- `ErrorKind` exists in core. Every thrower listed under "What the code does today" throws its kind with its old words. `plain_error` answers a kinded error from one switch.
- The four misses read their real sentences.
- The cases above pass with their red lines recorded, and the named existing cases pass unchanged.
- The scan row is in place, with today's matcher lines listed for ER2.
- `git diff --stat <base>..HEAD` lists only the owned files.

## Open questions (each with a recommended answer)

1. **Where the kind lives** (code-only). Recommended: `src/core/error_kind.h`. Nothing below `src/app` includes `app/`, and the throwers live in six of those folders. The sentences stay in `user_messages.cpp`, the owner the plan names.
2. **Missed database writes now read as database errors** (needs the user: displayed text). AL2's song-length write runs inside `save_analysis`, so a failure there shows after "Analyzed, but saving failed." That line now reads "Hydra couldn't save to its database (hydra.db). Check that the disk isn't full and that no other copy of Hydra is running, then try again." instead of "Something went wrong. Try again, and if it keeps happening, report it with the details below." The other three misses run while the store opens, where no screen shows a sentence today (see ER2's "Not in this task"). Recommended: yes. D51 call 25 asks for exactly this.
3. **The HTTP code rides on the error** (code-only). Recommended: the error type holds one optional number, read only by NetHttpStatus. The alternative, one kind per status code, has no end.
4. **Damaged Preview files share the "missing" sentence** (needs the user: displayed text). An undecodable texture, a shader that won't compile and a broken `.obj` are damaged, not missing. Today they show raw text, or nothing specific. Recommended: they read the existing sentence word for word, "Some of Hydra's Preview files are missing. Reinstall Hydra to restore them." D51 call 25 names "Reinstall Hydra", and reinstalling fixes both. The alternative rewords the one sentence to "missing or damaged", which changes the words for a missing file too. ER2 makes this visible on the Preview tab.
5. **Thrown text stays byte for byte** (code-only). Recommended: yes. Details lines, hydra_batch's output and the existing pins keep reading the same, and the kind carries the meaning.

## Commits

One commit, trailers `Task: ER1` plus the preamble's others, with the session above. Report as the preamble says, plus the list of kinds with one thrower each, and each `known_copies` line you added for ER2.
