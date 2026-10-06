# Handoff: when hydra.db fails, Hydra closes with no message (2026-10-05)

Written by session 6fc5f64c at the end of phase 7. Nothing is in flight for this; it is a new piece of work that needs the user's answers before any code.

## The problem in one paragraph

If Hydra can't open or read its database, it just disappears. No window, no sentence, nothing to tell the user to close the other copy of Hydra or free up disk space. The same happens if the database fails at the very start of an "Analyze library" batch. Since ER1 and ER2 (finding 193), Hydra already has the right sentences for these failures. Nothing on these paths catches the error to show them.

## What happens today, path by path

**Starting the GUI.** `main()` in `src/ui/main.cpp` builds `AppState` (line 242). Its constructor in `src/ui/app_state.cpp` loads the settings and the rules file. Then it opens the store through `app::open_store`, which builds `store::RecordStore` (`src/store/record_store.cpp`). Then it loads the library with `reload_library()`. Nothing on that path has a try/catch except the rules file's own. There is no process-wide handler either: no `set_terminate` and no `SetUnhandledExceptionFilter` in `src`. So any database error there leaves `main()` and the C++ runtime ends the process. The main window has been created by then, but no frame has been drawn.

**Which sentence a failure would get.** SQLite opens files lazily. So only a failure of `sqlite3_open_v2` itself throws `DatabaseOpen`. A locked file, a corrupt file or a full disk usually fails a moment later, at the first statement (`PRAGMA journal_mode=WAL` or the schema setup). That throws `DatabaseWrite`, whose sentence talks about saving. Nothing sets a busy timeout (`sqlite3_busy_timeout` appears nowhere in `src`), so a database locked by another copy of Hydra fails at once instead of waiting.

**Starting a library batch.** `BatchJob::start()` in `src/ui/library_jobs.cpp` runs `run()` on a plain `std::thread`. Unlike `AnalyzeJob` and `ReportJob`, it does not go through `run_guarded`. `run()` calls `app::run_batch` with no catch. The first thing `run_batch` does is ask the store which charts are already analyzed (`charts_with_result` → `store.analyzed_hashes`), and that can throw. ER2 made each chart's analysis and save failure a per-chart failure, so those are safe. But an error before the work pool starts, or one the pool rethrows, escapes the thread, and Hydra closes. The same `charts_with_result` call also runs on the UI thread, uncaught, when the user clicks Analyze library (`AppState::open_batch_confirm`, `app_state.cpp` line 471).

**The command-line tools.** `hydra_batch` (`src/cli/batch.cpp` line 143) calls `open_store` with no catch, so a failed open ends it with no message and no exit code. `hydra_report` and `hydra_fillcompare` call `open_store` the same way. Their surrounding code wasn't read.

**The sentences that already exist** (`src/app/user_messages.cpp`, picked by `plain_error`):
- DatabaseOpen: "Hydra couldn't open its database (hydra.db). Check that no other copy of Hydra is running and that the Hydra folder isn't read-only."
- DatabaseWrite: "Hydra couldn't save to its database (hydra.db). Check that the disk isn't full and that no other copy of Hydra is running, then try again."

## What the user needs to decide

Each of these changes what the user sees, so each is a question before any code.

1. **How the startup failure is shown.** Hydra has no message box anywhere today, and every in-app message (the status line, the rules-file strip) needs a built `AppState`, which needs the store. The choice is between:
   - **(a) A Windows message box.** It shows the plain sentence with the raw error underneath, then closes. This is the smallest change and works before any frame is drawn. *Recommended.*
   - **(b) Start anyway** with no library, showing the sentence in a strip, like the rules-file error does. This is much bigger, because every screen assumes a store exists.
2. **Which sentence a startup failure reads.** Any failure while opening should probably read the "couldn't open" sentence, even when SQLite only notices at the first statement. The other choice is to keep the "couldn't save" sentence for those cases. *Recommended: "couldn't open"*, by treating every throw from the `RecordStore` constructor as DatabaseOpen.
3. **Whether to wait for a lock.** A busy timeout would let a second copy of Hydra wait briefly for the first, instead of failing at once. That is a new number (how long to wait), so it needs a decision. *Recommended: no timeout.* Two copies of Hydra on one database shouldn't run together, and the open sentence already says so.
4. **How a batch that can't start is shown.** *Recommended:* catch it in `BatchJob::run` and finish the batch as failed, with the database sentence in the batch strip. Do the same for the Analyze-library click (status line). The process keeps running either way.
5. **The command-line tools.** *Recommended:* print the plain sentence and the raw error to stderr and exit with a non-zero code, the way `hydra_batch` already handles a bad rules file (exit 2).

## Related, found on the way, not part of this

- A failed Direct3D device makes `main()` return 1 with no message (`main.cpp` around line 177). The window creation and the two ImGui backend inits aren't checked either. If the user picks the message box in question 1, it could cover these too.
- In the scan stage, `ReadNote::error` decides "did it fail" by a non-empty error text, so an exception with an empty message is dropped (ER2 review note).

## How to start

1. Ask the five questions above (one sheet, recommendations first) and record the answers as the next D number in `docs/audit/2026-10-03-fix-decisions.md`.
2. Then write one brief, with failing-first tests for each path.
   - Startup: drive it with a database path the tests make unopenable. `app::set_path_overrides` exists for test paths.
   - Batch: a store whose `analyzed_hashes` throws.
   - CLI: an unopenable path.
   - Showing the sentence: check through `hydra_uitest` where that is possible.
3. Every code merge needs a derive-once review first (`docs/agents/derive-once-review.md`).
