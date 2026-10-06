Read `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md` first; it holds the rules every agent follows.

# Task DB1: a database that fails at startup or at a batch's start says so (D72)

Task id: DB1. Base: main at the commit the main session names. Session for your trailers: 371d0f53-b961-4ade-b5ac-bea407e594cc.

Make your worktree and first build with `pwsh -NoProfile -File C:\Users\Patrick\Downloads\Hydra\hydra-test\tools\new_worktree.ps1 -TaskId db1 -Base <base> -Target hydra_tests`. It makes `.claude\worktrees\db1` on branch `claude/db1` and builds inside the shared build slot. Work, build and commit only there. Later builds are warm: `.\build_cpp.ps1 -Target <target>` from the worktree.

The decision is D72 in `docs/audit/2026-10-03-fix-decisions.md`. The background is `docs/handoffs/2026-10-05-database-failure-handoff.md`. Read both.

## Goal

When hydra.db can't be opened, Hydra says so instead of vanishing. At startup that is a Windows message box with the plain sentence and the raw error, and then Hydra closes. A batch whose database fails before it starts finishes as failed, with the sentence in the batch strip, and Hydra keeps running. The command-line tools print the sentence and the raw error and exit non-zero. No score, path, stored record or stamp changes.

## What the code does today

Line numbers are from main at a18118c.

**The store's open.** `RecordStore`'s constructor (`src/store/record_store.cpp`, line 689) throws DatabaseOpen only when `sqlite3_open_v2` itself fails (line 695). Everything after it can throw too: the two PRAGMAs, the schema setup, `upgrade_results_key`, `delete_auto_results` and `fill_missing_stars`. Those throw DatabaseWrite from `exec` (line 822) and friends. A locked file or a file that isn't a database usually fails there, at the first PRAGMA, so it reads "couldn't save". A throw from the constructor also skips the destructor, so `db_` is never closed. The file stays open, and a locked file stays locked by this process.

**The GUI's startup.** `main()` (`src/ui/main.cpp`) builds `AppState` at line 242 with no catch. `AppState()` (`src/ui/app_state.cpp`, line 32) opens the store through `app::open_store` and then runs `reload_library()`. Any throw leaves `main()` and the process ends. The window was already shown at line 194. Before that, `CreateDeviceD3D` failing returns 1 at line 182 with no message, and `CreateWindowW` (line 173), `ImGui_ImplWin32_Init` and `ImGui_ImplDX11_Init` (lines 204 and 205) aren't checked at all.

**A batch's start.** `BatchJob::run` (`src/ui/library_jobs.cpp`, line 243) already catches a failure while building its list (line 250): it pushes `plain_error(e)` to `failures`, a "Could not load the library: " detail, counts one failure and finishes. Its call to `app::run_batch` at line 303 has no catch. `run_batch` asks the store first (`charts_with_result` → `analyzed_hashes`, `src/app/analysis.cpp` line 631), so a database error there leaves the job's thread and Hydra closes. The Analyze-library click runs the same `charts_with_result` on the UI thread, uncaught, in `AppState::open_batch_confirm` (`app_state.cpp`, line 471). `start_batch` calls `open_batch_confirm` too, when no confirm is open (line 482).

**The command-line tools.** hydra_batch (`src/cli/batch.cpp`, line 143), hydra_report (`src/cli/report.cpp`, line 80) and hydra_fillcompare (`src/cli/fillcompare.cpp`, lines 79 and 81) call `open_store` with no catch. Each tool already exits 2 when it can't start (bad option, bad rules file, a refused run) and 1 when something fails while it runs. hydra_batch's `run_batch` call (line 252) has no catch either.

**The sentences.** `app::plain_error` (`src/app/user_messages.cpp`) picks the sentence from the error's kind. `plain_error_detail` gives the raw text. Nothing yet puts the two into one block of text, which a message box and stderr both need.

## What changes

1. **Every throw from opening the store reads DatabaseOpen (D72 item 2).** In `RecordStore`'s constructor, a throw from any step after `sqlite3_open_v2` closes the handle and rethrows as `KindedError(ErrorKind::DatabaseOpen, <the same raw text>)`. The raw text stays byte for byte. `std::bad_alloc` passes through unchanged, because `plain_error` answers it by type. The store stays the one owner of "what kind is a failed open"; no caller re-kinds it.
2. **One block of text for an error (new owner).** Add `app::plain_error_block(const std::exception&)` to `user_messages.h`/`.cpp`. It returns the sentence, a blank line, then the raw text, built from `plain_error` and `plain_error_detail`. No new words: no "Details:" label. The message box and all three tools use it. If `tests/test_single_owner.cpp` has a row this answers, extend it; otherwise add one row for "the sentence and the raw text of one error, as one block".
3. **The startup message box (D72 item 1).** In `main()`, catch a throw from building `AppState` and show `MessageBoxW` with `plain_error_block`, an error icon and the window title (`hydra::kWindowTitleW`) as its caption. Then tear down the way the normal exit does and return 1. Do the same for a failed `CreateWindowW`, `CreateDeviceD3D`, `ImGui_ImplWin32_Init` or `ImGui_ImplDX11_Init`: build an untyped `std::runtime_error` whose text names the call and its error code (`GetLastError` or the HRESULT), so it reads the existing fallback sentence (D71 item 4) with the raw text under it. `CreateDeviceD3D` returns a bool today; have it hand back the HRESULT so the text can name it. Pass the window as the box's owner once it exists. Keep `main()` readable: one small local function that shows the box is enough.
4. **No busy timeout (D72 item 3).** Don't add `sqlite3_busy_timeout`.
5. **A batch that can't start (D72 item 4).** `BatchJob::run` catches a throw from `run_batch` and finishes the batch as failed: the sentence in `failures`, the raw text in `failure_details`, one failure counted, the clock stopped, `finished` set. The list-building catch at line 250 already does exactly this, so move it into one private helper both catches call; don't copy it. `AppState::open_batch_confirm` catches a throw from `charts_with_result`, calls `set_problem(app::plain_error(e))`, and leaves the confirm closed. `start_batch` then starts nothing: it checks whether the confirm opened.
6. **The command-line tools (D72 item 5).** Each of the three wraps its `open_store` call. On a throw it prints `plain_error_block` to stderr and returns 2, the "can't start" code the tools already use. hydra_batch also wraps its `run_batch` call: it prints the same block and returns 1, the tools' existing "failed while running" code. If the three tools would each write the same catch, put the shared part in one place they all link (look at what they already share under `src/app/` first).

## Owned files (only these may change)

- `src/store/record_store.cpp` (the constructor only), and `src/store/record_store.h` only if the constructor needs a private helper declared
- `src/app/user_messages.h`, `src/app/user_messages.cpp`
- `src/ui/main.cpp`
- `src/ui/app_state.cpp` (`open_batch_confirm` and `start_batch` only)
- `src/ui/library_jobs.h`, `src/ui/library_jobs.cpp` (`BatchJob` only)
- `src/cli/batch.cpp`, `src/cli/report.cpp`, `src/cli/fillcompare.cpp`
- `tests/test_store.cpp`, `tests/test_user_messages.cpp`, `tests/test_app_state.cpp`, `tests/test_library_jobs.cpp`, `tests/test_cli.cpp`, `tests/test_single_owner.cpp` (your row only)
- `tests/ui/uitest_batch_reports.cpp` or `tests/ui/uitest_library.cpp` (one new script, whichever already holds the batch-strip scripts)
- `docs/agents/ui-testing.md` only if it lists scripts by name

If you need another file, stop and report which one and why.

## Test cases with red lines

Write each red first, then green, and keep the red line. Pin sentences as literals. Make a database fail with the techniques the tests already use: `test_store.cpp`'s trigger cases (line 1704), a file of junk bytes, or a second sqlite connection that holds an exclusive lock. To make a read fail after a good open, drop or rename the `results` table through a second connection. No new test helper if one already exists; grep `tests/` first.

In `tests/test_store.cpp`:
1. "a database file that isn't a database fails to open as DatabaseOpen". A file of junk bytes. The constructor throws a `KindedError` whose kind is DatabaseOpen and whose text is SQLite's own. Red line: the kind is DatabaseWrite today.
2. "a database another connection has locked fails to open as DatabaseOpen, and lets go of the file". A second connection holds an exclusive lock. The constructor throws DatabaseOpen. After the lock is released, a fresh `RecordStore` on the same file opens. Red line: the kind today.

In `tests/test_user_messages.cpp`:
3. "plain_error_block puts the sentence above the raw text". A DatabaseOpen `KindedError` gives the DatabaseOpen sentence literal, a blank line, then the raw text. An untyped error gives the fallback sentence the same way. Red line: it doesn't exist (a compile failure counts; quote the error line).

In `tests/test_app_state.cpp`:
4. "an AppState whose database can't open throws DatabaseOpen". Point `app::set_path_overrides` at a junk file and build `AppState`. It throws DatabaseOpen. Red line: DatabaseWrite today. (Restore the overrides at the end, the way the other cases that use them do.)
5. "Analyze library on a database that fails shows the sentence and opens no confirm". A good `AppState`, then break the results table and call `open_batch_confirm`. `status_message` holds the database sentence, `status_is_problem` is true, and `batch_confirm_pending` is false. Then `start_batch(false)` starts no batch. Red line: the exception escapes today.

In `tests/test_library_jobs.cpp`:
6. "a batch whose database fails before it starts finishes as failed". Break the results table, then start a `BatchJob` and wait for it. The snapshot is finished, `failed` is 1, `failures` holds the database sentence, and `failure_details` holds the raw text. Red line: today the exception leaves the thread and the test process ends; quote what the runner prints.

In `tests/test_cli.cpp`:
7. "hydra_batch, hydra_report and hydra_fillcompare say why a database won't open". Run each against a junk database file with `run_exe`. Each exits 2, and stderr holds the DatabaseOpen sentence and SQLite's text. Red line: today each ends with a crash exit code and nothing on stderr.

GUI script, through `hydra_uitest` (`docs/agents/ui-testing.md`):
8. A new script that breaks the scratch library's results table and clicks Analyze library. The status line shows the database sentence, and Hydra is still running. If the harness offers no way to break the table, stop and report rather than adding one outside your owned files.

The message box itself can't run headless. Check it once by hand: copy the built `Hydra.exe` (and anything it needs at startup) into your scratch folder beside a junk `hydra.db`, start it, read the dialog's text with PowerShell UI Automation, then close the dialog and confirm the process ends. Put the text you read in your report.

Existing tests that must pass unchanged: everything in `-sf=*test_store.cpp*` and `-sf=*test_user_messages*`, the `batch-done-strip`, `status-line` and `rules-error` GUI scripts, and `-tc="hydra_batch*"`.

## Test filters

- `build-cpp\Release\hydra_tests.exe` with `-sf=*test_store.cpp*`, `-sf=*test_user_messages*`, `-sf=*test_app_state*`, `-sf=*test_library_jobs*`, `-sf=*test_cli.cpp*` (build `hydra_batch`, `hydra_report` and `hydra_fillcompare` warm first), and `-tc="single-owner*"`.
- `build-cpp\Release\hydra_uitest.exe --test <name>` for your new script, `batch-done-strip`, `status-line` and `rules-error` (build with `.\build_cpp.ps1 -Target hydra_uitest` first).

Nothing else. Never the full suite, never `hydra_uitest --all`.

## Not in this task

- Database failures after startup other than the Analyze-library click and a batch's start (a scan's reload, the details panel's reads). They keep today's handling.
- `ReadNote::error` in the scan stage and the dimmed-details-line helper from the phase 7 closing handoff.
- Any new sentence. Every message here uses an existing one.

## Done when

- Every throw from `RecordStore`'s constructor reads DatabaseOpen and leaves no handle open.
- Hydra shows a message box for a database, window, graphics-device or ImGui-backend failure at startup, then closes.
- A batch whose database fails before it starts finishes as failed, and the Analyze-library click shows the sentence in the status line. Hydra keeps running in both.
- All three tools print the sentence and the raw error and exit 2 when the database won't open; hydra_batch exits 1 when its run fails.
- The cases above pass with their red lines recorded, and the named existing tests pass.
- `git diff --stat <base>..HEAD` lists only owned files.

## Commits

One commit per step is fine. Trailers: `Task: DB1`, your agent id, the session above, and `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`. Report as the preamble says, plus the message-box text you read by hand.
