# Every start checks for updates, and the toolbar shows the version (2026-10-23)

Task id UPD2-P1. Planner: Fable. Nothing here is built yet; every task waits for the user's yes (agent rule 3). This plan changes what is on screen (a label appears, a settings group goes), so its words go to the user first, in "Words on screen" and "Questions for the user" below.

It follows D259 and `docs/superpowers/plans/2026-10-22-auto-updater.md`, which shipped the updater in 2.11.0 and 2.11.1. Every claim below about the shipped code was read in the code on `main` at 0e34e2d8.

## What the user gets

Hydra Deluxe checks GitHub for a newer release every time it starts. There is no switch to turn that off, no `Check now` button, and no 24-hour wait. The `Analysis settings` panel goes back to its three analysis groups. A dimmed `Hydra Deluxe 2.11.1` label sits at the right end of the toolbar. Hover over it and it tells you whether you are up to date, or which newer version the start-up check found. Clicking it does nothing. The strip under the toolbar still offers a newer release with `Update`, `Skip this version` and `Later`, and `Update` still works exactly as in 2.11.x.

The user's rulings, in their words: "the app should automatically check for updates when it launches. it shouldnt need tyhe user to manually check. a version label is good too." Off switch: "No off switch". Label: "Text only".

## What changes, in plain terms

The shipped rule for "should a check run now" lives in `app::update_check_due` (`src/app/config.cpp` line 484). It says: the `check_updates` setting is on, this is the installed exe (`installed_by_setup`), and the last finished check was more than `kUpdateCheckInterval` (24 hours, `config.h` line 355) ago, or there was none, or its time can't be read or is in the future. The new rule is one question: is this the installed exe. So `update_check_due`, `kUpdateCheckInterval`, `update_check_time_text`, `update_check_time`, `Settings::check_updates` and `Settings::last_update_check` all go. `AppState::check_updates_if_due` becomes `AppState::check_updates_on_start`, which asks `app::installed_by_setup()` and nothing else. The `Check now` path (`AppState::check_updates_now`, the `manual` flag in `start_update_check` and `collect_update_check`) goes with it, and with it the `UpToDate` strip phase, because the only thing that ever put the strip in `UpToDate` was `Check now` (`app_state.cpp` line 1189). An up-to-date start now says nothing in the strip; the version label's hover says it instead.

The old INI keys. `check_updates` and `last_update_check` join `kRetiredSettingsKeys` in `src/app/config.h` (line 75), with `retired_in` set to the version that ships this (2.12.0 unless the main session numbers the release differently). Nothing else is needed for old files: `Settings::load_file` skips any key no setting has (`config.cpp` line 267), and `save_file` writes only the keys in the table, so a 2.11.x INI loads cleanly and loses the two lines at its next save. The existing test "each retired settings key loads silently and is never written back" (`tests/test_config.cpp` line 620) covers both keys the moment they are in the list. A user who unticked the box in 2.11.x (`check_updates=0`) gets checks again; that is the "No off switch" ruling, and the D-record and release note say so.

`skipped_update_version` and `last_run_version` stay as they are. Skip still writes the version and the start-up check still stays silent for that version (`app_state.cpp` line 1180); only the `!manual` half of that line goes, since no manual check exists any more. `Later` still writes nothing and hides the strip; what "until the next check" means now is question 1 below.

## The version label

Where the text comes from. The one home of the app's version is `HYDRA_VERSION` from `CMakeLists.txt`, checked by `src/core/version.h`, and read as numbers by `core::running_version()` in `src/core/app_version.h`. The strip already builds "Hydra Deluxe 9.9.9" in `named_release` (`app_state.cpp` line 1080, an anonymous-namespace helper: `HYDRA_APP_NAME + " " + version_text(version)`). That helper moves into `src/core/app_version.h` as `core::named_version(const AppVersion&)`, the strip calls it, and the label calls `core::named_version(core::running_version())`. No file spells "Hydra Deluxe 2." or pastes `HYDRA_VERSION` into a sentence. A single-owner row pins that (T1a, criterion A6).

Where it sits. At the right end of the toolbar's action row (`render_actions_row`, `src/ui/library_toolbar.cpp`), to the right of `Open path report` when that button shows. The row already reserves room at the right edge for the report button and passes it to the settings button as `room_after` (line 193); the label's width plus one item spacing is added to that room, so the settings label shortens first, as it does today for the report button. Drawn with `TextUnformatted` in `kDimTextColor` (`src/ui/theme.h` line 65), in the row's current font with no font push, so it is a plain dimmed label and the font-role scan has nothing to refuse. It is text, not a button: a click does nothing and nothing changes.

What the hover says. One function, `AppState::update_hover_text()`, answers it, and the toolbar only draws the answer through `wrapped_tooltip`. The rule, in order:

1. `app::installed_by_setup()` is false: `Only the installed copy of Hydra Deluxe checks for updates.` (the sentence the greyed `Check now` showed; it moves from `settings_bar.cpp` line 216 to a named constant beside `kUpdateDeclinedText` in `app_state.cpp`, line 1077).
2. The check is running (`update_check_running()`): `Checking for updates...`
3. The check failed: two lines, `Hydra couldn't check for updates when it started.` and under it the failure's `plain_error` sentence, for example `Hydra couldn't reach GitHub. Check your internet connection and try again.` The detail line (`plain_error_detail`) is not shown; the strip stays empty, as decision 8 of D259 says for an automatic check. The next start checks again.
4. The check found a newer release: `Hydra Deluxe 9.9.9 is available.` This holds whether or not the strip shows: after `Skip this version` or `Later` the strip is hidden, but the label still names the newer version. The strip is the offer; the label is the fact.
5. Otherwise: `Hydra Deluxe 2.11.1 is the latest version.` (the sentence the `UpToDate` strip used).

To answer 3 to 5 after the strip has been dismissed, `UpdateState` grows two fields that `collect_update_check` fills: `last_check` (an enum: `NotRun`, `Failed`, `UpToDate`, `Newer`) and `check_error` (the `plain_error` sentence when `Failed`), plus `newer` (the found `AppVersion` when `Newer`). `update.release` keeps its shipped meaning (the release offered or being installed) and is still cleared by `later()`.

## Words on screen

Every sentence that is new or moves, in one place:

- Toolbar label: `Hydra Deluxe <version>`, dimmed, from `core::named_version(core::running_version())`.
- Label hover: the five sentences in "The version label" above.
- Strip: unchanged wording for Offered, Downloading, Verifying, Launching, Declined, Failed and Updated. `UpToDate` is gone.
- Settings panel: the `App` group, the `Check for updates` row, its help text and `Check now` are gone. The panel is `Chart`, `Clone Hero rules`, `Paths kept`, then `Reset to defaults`.

## Questions for the user

**Q1. What `Later` means now.** The shipped rule: `AppState::later()` (`app_state.cpp` line 1258) clears the strip and writes nothing, and the strip comes back when the next automatic check finds the release again. Today the next automatic check is at the first start more than 24 hours after the last check (`update_check_due`). Example today: you click `Later` at 9:00, close Hydra, open it at 14:00 and at 18:00; the strip stays hidden both times and comes back the first start after 9:00 tomorrow. With every start checking, the same clicks bring the strip back at 14:00. Recommended: keep `later()` as it is, so `Later` means "hide until Hydra is next started". It is the smallest change, and `Skip this version` already covers "stop telling me about this one". Alternative: `Later` remembers the version and the time and hides it for 24 hours; that keeps a timestamp key and the time-parsing code this plan removes, for one button. Not recommended.

**Q2. `Skip this version` after this change.** The shipped rule: `skip_version()` writes `skipped_update_version=<version>` and the automatic check lands in `None` for that version (`app_state.cpp` lines 1180 to 1181), and only `Check now` ignored the skip (the user's fifth ruling during T3a). Example: skip 2.12.0, and no start offers 2.12.0 again; 2.12.1 is offered. With `Check now` gone, nothing ignores the skip any more. Recommended: keep it so; the label hover still names the skipped version (rule 4 above), so the fact is one hover away. No visible change to the strip, listed here only because the ruling that `Check now` re-offers a skipped release is now void.

Everything else in this plan is a consequence of the three rulings quoted at the top, or an exact wording for the user to read in "Words on screen".

## The build

Three executor tasks on Opus. T1a goes first from `main`. T1b and T2 fork from T1a's join and run in parallel. Each is under 100 tool calls and 20 minutes; T1a is the largest and its files are listed so the executor edits nothing else.

Interfaces fixed here so T1b and T2 agree with T1a without talking: `std::string core::named_version(const core::AppVersion&)` in `src/core/app_version.h` ("Hydra Deluxe 2.11.1"); `void AppState::check_updates_on_start()` (replaces `check_updates_if_due`; `check_updates_now` is removed); `std::string AppState::update_hover_text() const`; `bool AppState::update_check_running() const` (unchanged); `UpdateState::Phase` without `UpToDate`; the constants `kOnlyInstalledChecksText`, `kCheckingForUpdatesText` and `kCouldNotCheckText` beside `kUpdateDeclinedText` in `app_state.cpp` (T1b never reads them; it draws `update_hover_text()`).

### T1a: the rule, the state and the version text (owner: `src/app/config.h`, `src/app/config.cpp`, `src/core/app_version.h`, `src/ui/app_state.h`, `src/ui/app_state.cpp`, `src/ui/update_job.h`, `tests/test_config.cpp`, `tests/test_update_state.cpp`, `tests/single_owner/rules/core.json` one new row; from `main`)

Remove `update_check_due`, `kUpdateCheckInterval`, `update_check_time_text`, `update_check_time`, `Settings::check_updates` and `Settings::last_update_check`, and add the two keys to `kRetiredSettingsKeys`. In `AppState`, replace `check_updates_if_due` with `check_updates_on_start` (gate: `installed_by_setup()` only), delete `check_updates_now` and the `manual` flag, delete the `UpToDate` phase and the branch that set it, add `UpdateState::last_check`, `check_error` and `newer`, fill them in `collect_update_check`, and write `update_hover_text()` as "The version label" says. Move `named_release` to `core::named_version`. The comments that named decisions 5, 6 and 8 of the old plan now point at this plan's D-record.

Acceptance criteria, each a test:
- A1 (`test_config.cpp`) the key-order test's expected list no longer has `check_updates`; the updater-keys test round-trips `skipped_update_version` and `last_run_version`, and `with_analysis_defaults` leaves both alone. The retired-keys test runs green with the two new rows, so an INI holding `check_updates=0` and `last_update_check=2026-10-22T13:45:00Z` loads and neither line is written back.
- A2 (`test_update_state.cpp`) with `set_installed_for_test(true)`, a new `AppState` has `update_check_running()` true before its first tick; with `false` it is never true, and after five ticks the hover text is `Only the installed copy of Hydra Deluxe checks for updates.` This replaces the C2 `update_check_due` case and the ISO-time case, which are deleted.
- A3 with the canned reply at v9.9.9, the start lands in `Offered` with text `Hydra Deluxe 9.9.9 is available.` and the hover text is the same sentence; after `later()` the phase is `None` and the hover text still names 9.9.9; after `skip_version()` on a fresh start the phase is `None`, the INI says `skipped_update_version=9.9.9`, and the hover text still names 9.9.9. A reply at v9.9.10 on the next start with 9.9.9 skipped lands in `Offered`.
- A4 with the reply at the running version, and again at v0.0.1, the start lands in `None` with empty strip text and the hover text `Hydra Deluxe <running> is the latest version.`; no `UpToDate` enumerator exists (the test compiles against the enum).
- A5 with the fetcher throwing `NetHttpStatus` 403 and then `NetUnreachable` (the two shipped cases), the start lands in `None` with empty text and detail, and the hover text is `Hydra couldn't check for updates when it started.` followed by a newline and the matching `plain_error` sentence. While the fetcher is held, the hover text is `Checking for updates...`.
- A6 `core::named_version(AppVersion{2, 11, 1})` is `Hydra Deluxe 2.11.1`, and the new single-owner row (question "What is the version text the user reads?", owner `named_version`) matches `std::string(HYDRA_APP_NAME) + " " + HYDRA_VERSION` and `"Hydra Deluxe " HYDRA_VERSION` and does not match `core::named_version(core::running_version())`; the scan runs green, which proves no second copy exists in `src/`.
- A7 the shipped cases for `Update`, the declined installer, the failed start, the bad signature, Cancel, the "updated" start and the leftover folder (`test_update_state.cpp` from line 341) still pass, edited only where they called `check_updates_now` or read `last_update_check`.

Tests it may run: `hydra_tests.exe -sf=*test_update_state.cpp`, `-sf=*test_config.cpp`, `-sf=*test_update_release.cpp`, and the single-owner scan case `-tc=*single owner*` once.

### T1b: the toolbar label and the settings panel (owner: `src/ui/library_toolbar.cpp`, `src/ui/settings_bar.cpp`, `src/ui/library_parts.h`, `tests/ui/uitest_library.cpp`; forks from T1a's join)

Delete `render_app_group` and its call in `render_panel`, and put the panel's comment back to three groups. Draw the label at the right end of the action row as "The version label" says, add its width to the settings button's `room_after`, show `app.update_hover_text()` on hover, and remove the `UpToDate` branch from `render_update_strip`. Rewrite the `update-strip` uitest: every place that clicked `Check now` now restarts the app with `restart_app` (already in the file, line 1336) so the start-up check runs.

Acceptance criteria, in the `update-strip` script unless named:
- B1 the toolbar shows `Hydra Deluxe <running>` (the text from `core::named_version(core::running_version())`); its rectangle's right edge is within one item spacing of the row's right edge and lies to the right of `Open path report` when that button shows; its text colour is `kDimTextColor`.
- B2 with the reply at the running version, hovering the label shows `Hydra Deluxe <running> is the latest version.` and the strip shows nothing; clicking the label changes nothing (the phase stays `None`, no popup opens, the hover text is the same after the click).
- B3 with the reply at v9.9.9, a restart lands in the offer with its three buttons and the notes on hover (the shipped D1), and the label's hover reads `Hydra Deluxe 9.9.9 is available.`; after `Later` the strip is gone and the label's hover still reads it; after `Skip this version` and a restart the strip stays hidden and the label's hover still names 9.9.9.
- B4 with `set_installed_for_test(false)`, the label's hover reads `Only the installed copy of Hydra Deluxe checks for updates.` and no check runs.
- B5 with the reply `this is not JSON`, a restart shows no strip and the label's hover begins `Hydra couldn't check for updates when it started.`
- B6 the `Analysis settings` panel has no `App` group: `on_screen("Check for updates")` is false, `ItemExists("**/Check now")` is false, the three groups and `Reset to defaults` are still there, and with a batch holding the settings the lock line still reads `Stop the batch to change these.`
- B7 the shipped D3 (download, Cancel, declined installer) and D5 (the "updated" start) still pass.
- B8 the existing `settings-reset` and `settings-and-reports` scripts still pass.

Tests it may run: `hydra_uitest update-strip`, `hydra_uitest settings-reset`, `hydra_uitest settings-and-reports`.

Memory rule `ui-changes-need-real-app-check` applies to the label: before merging T1b the main session captures the toolbar in the real window at the user's window size, with the song panel closed and open (the report button present and absent), and shows the user.

### T2: the public docs, CONTEXT.md and the decision record (owner: `README.md`, `docs/UserGuide.md`, `docs/handoffs/release-next-notes.md`, `docs/audit/2026-10-03-fix-decisions.md`, `CONTEXT.md` the "Update check" paragraph, `docs/agents/ui-testing.md` the toolbar row only; forks from T1a's join, in parallel with T1b)

README, install step 1 (line 28): "once a day when it starts" becomes "each time it starts", and the last sentence ("To stop the check, untick `Check for updates` in `Analysis settings`.") goes. One sentence follows: "The toolbar's right end shows the version you run; hover over it to see whether a newer one is out."

User Guide. Toolbar section: a paragraph after `Open path report` (line 69) for the label, the five hover sentences in plain words. "When a new version is out" (lines 73 to 91): "once a day" becomes "each time it starts"; the sentences about 24 hours, the unreadable time and the future time go; "not even with `Check now`" goes; `Later` reads "hides the line until you next start Hydra Deluxe" (per Q1's answer); the `Check now` half of the skip paragraph goes; the failure paragraph says a failed start-up check shows nothing in the line and the label's hover says why; the last paragraph (the up-to-date strip) is replaced by the label's hover sentence. Settings section: line 97 loses the `Check for updates` sentence; line 99 reads "The panel has three groups."; the table loses the `App` row (line 111); line 113's reset sentence loses "`Check for updates`,"; line 117's lock sentence loses the `Check for updates` sentence; the paragraph at line 137 goes.

`docs/differences-from-public-hydra.md`: its 2.11.0 row (line 143) is history and stays. The row for the release that ships this is added at release time as every row is, with the words "checks for a newer release each time it starts and shows the version on the toolbar".

`release-next-notes.md`: one paragraph after the shipped one, saying the check now runs at every start, the `App` group and its box are gone (a box you had unticked no longer stops the check), and the toolbar shows the version with the hover.

CONTEXT.md "Update check" (lines 415 to 426): three questions, not four. "Should a check run now" and `update_check_due` go; "Is this the installed exe" says it is the only gate on the start-up check; a sentence names `named_version` as the one text of a version and `update_hover_text` as the one answer the label shows.

`docs/agents/ui-testing.md`: the toolbar row lists the label by its text and says it has no click.

The D-record, `D260`, after D259 in `docs/audit/2026-10-03-fix-decisions.md`: the three rulings in the user's words, the answers to Q1 and Q2, the list of what was removed (the `App` group, `Check for updates`, `Check now`, the 24-hour wait, the up-to-date strip, the two INI keys), the hover sentences, and the note that D259's calls 5, 6 and 8 and its rulings (2) and (5) are superseded.

Acceptance criteria:
- C1 `hydra_tests.exe -sf=*test_docs_match_code.cpp` runs green on the joined tree: it checks that every code name a doc uses exists, so a CONTEXT.md or guide that still named `update_check_due`, `check_updates` or `Check now` as a code name would fail it once T1a has removed them.
- C2 a grep of `README.md`, `docs/UserGuide.md`, `CONTEXT.md` and `docs/agents/ui-testing.md` for `Check now`, `Check for updates`, `once a day`, `24 hours` and `check_updates` finds nothing; the reviewer runs it and the D-record quotes it. (`docs/handoffs/release-2.11.*-notes.md` and D259 are history and keep their words.)
- C3 the D-record exists, names D260, quotes the three rulings and the Q1 and Q2 answers, and lists the removals.
- C4 every sentence in the guide and README matches "Words on screen" and the hover rule; the reviewer reads them side by side (`docs/agents/public-doc-style.md` applies: second person, every label checked against the code).

Tests it may run: `hydra_tests.exe -sf=*test_docs_match_code.cpp`.

## Testing without GitHub

Nothing touches the network. The unit tests keep `UpdateWorld` (`tests/test_update_state.cpp` line 82 on) and the uitest keeps `UpdateSeams` (`tests/ui/uitest_library.cpp` line 1290 on); both already serve the release reply, the download and the installer start through the shipped seams, and set `installed_by_setup` through `set_installed_for_test`. Where a test used `Check now` to run a second check, it now constructs or restarts the app, which is what a user does.

The user's real check, after the merge and before the release: start the installed 2.11.1 once from the new build's installer, and see the label at the right end of the toolbar with its up-to-date hover; copy `Hydra.exe` to the desktop and start it to see the "only the installed copy" hover; pull the network cable and start it to see the failed hover. The offer itself is seen on the first release after the one that ships this, as D259 said.

## Out of scope (for the user to approve)

- A click on the label (opening the release page, say). The ruling is "Text only".
- Any change to `Update`, the download, the signature check or the installer launch.
- A time-based reminder for `Later` (Q1's alternative).
- A new row in `docs/differences-from-public-hydra.md` before release day.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| Every start of the installed exe checks; no 24-hour wait | What changes | T1a | A2 (running before the first tick), A3 (a second start checks again) |
| A copy not put in place by the installer never checks | What changes; hover rule 1 | T1a, T1b | A2, B4 |
| No off switch: `check_updates` is not read | What changes | T1a | A1 (key list), A2 (checks with no setting) |
| Old `check_updates` and `last_update_check` lines load and are not written back | The old INI keys | T1a | A1 via the retired-keys test |
| No `Check now`, no `App` group, panel back to three groups | Words on screen | T1b | B6 |
| The up-to-date strip is gone | What changes | T1a, T1b | A4 (no `UpToDate` enumerator), B2 (no strip at the running version) |
| Version text derived once from `HYDRA_VERSION` through `named_version` | The version label | T1a | A6 single-owner row and scan |
| Label at the right end, dimmed, no click | The version label | T1b | B1, B2 |
| Hover: only installed copy / checking / failed with plain_error / newer / latest | The version label, hover rule | T1a, T1b | A2, A3, A4, A5 (the function), B2 to B5 (on screen) |
| Hover names the newer version after Later or Skip | Hover rule 4 | T1a, T1b | A3, B3 |
| A failed start-up check shows nothing in the strip | Hover rule 3 | T1a, T1b | A5, B5 |
| Skip hides that version only; a newer one shows | Q2 | T1a, T1b | A3, B3 |
| Later hides until the next start and writes nothing | Q1 | T1a, T1b | A3, B3 |
| Update, Cancel, declined start, bad signature, "updated" start unchanged | Out of scope; A7 | T1a, T1b | A7, B7 |
| Existing settings panel flows still pass | T1b | T1b | B8 |
| Public docs say every start, no box, label and hover | T2 | T2 | C1, C2, C4 |
| D260 records the rulings and the removals | T2 | T2 | C3 |
| CONTEXT.md names the owners that exist | T2 | T2 | C1 |
| No network in tests | Testing without GitHub | T1a, T1b | A3 and B3 run through the seams; the harness's fetcher throws for any other URL |
| Real-app capture of the label before merge | T1b | main session | the user sees the capture |
