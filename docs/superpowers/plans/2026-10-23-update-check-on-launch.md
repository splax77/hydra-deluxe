# Every start checks for updates, and a version bar tops the window (2026-10-23)

Task ids UPD2-P1 (first draft) and UPD2-P2 (this revision). Planner: Fable. Nothing here is built yet; every task waits for the user's yes (agent rule 3). This plan changes what is on screen (a bar appears, a settings group goes, the update strip moves), so its words go to the user first, in "Words on screen" and "Questions for the user" below.

It follows D259 and `docs/superpowers/plans/2026-10-22-auto-updater.md`, which shipped the updater in 2.11.0 and 2.11.1. Every claim below about the shipped code was read in the code on `main` at c3fe00aa.

## What the user gets

Hydra Deluxe checks GitHub for a newer release every time it starts. There is no switch to turn that off, no `Check now` button, and no 24-hour wait. The `Analysis settings` panel goes back to its three analysis groups.

A new bar runs across the top of the window, above the toolbar. It is always there. On the left it says `Hydra Deluxe 2.11.1` in larger text. After that, a coloured dot and one sentence say where you stand: `Up to date`, `Checking for updates...`, or `Hydra Deluxe 2.12.0 is available.` with the `Update`, `Skip this version` and `Later` buttons right there in the bar. Everything the update strip under the toolbar used to show (the download with its MB count, bar and `Cancel`; `Checking the download...`; `Starting the installer...`; the installer-didn't-start and couldn't-verify lines) now shows in this bar instead. Nothing about updates stays under the toolbar. `Update` works exactly as in 2.11.x.

The user's rulings, in their words: "the app should automatically check for updates when it launches. it shouldnt need tyhe user to manually check. a version label is good too." Off switch: "No off switch". On the first draft's dimmed toolbar label: "why are you still trying ot make this part of analysis settings? the whole point is for version to be its own prominent thing". From three mocks: "Go with A, the version bar along the top of the window."

## What changes, in plain terms

The shipped rule for "should a check run now" lives in `app::update_check_due` (`src/app/config.cpp` line 484). It says: the `check_updates` setting is on, this is the installed exe (`installed_by_setup`), and the last finished check was more than `kUpdateCheckInterval` (24 hours, `config.h` line 355) ago, or there was none, or its time can't be read or is in the future. The new rule is one question: is this the installed exe. So `update_check_due`, `kUpdateCheckInterval`, `update_check_time_text`, `update_check_time`, `Settings::check_updates` and `Settings::last_update_check` all go. `AppState::check_updates_if_due` becomes `AppState::check_updates_on_start`, which asks `app::installed_by_setup()` and nothing else. The `Check now` path (`AppState::check_updates_now`, the `manual` flag in `start_update_check` and `collect_update_check`) goes with it, and with it the `UpToDate` strip phase, because the only thing that ever put the strip in `UpToDate` was `Check now` (`app_state.cpp` line 1189). Being up to date is now the bar's resting state, not a phase with an OK button.

The old INI keys. `check_updates` and `last_update_check` join `kRetiredSettingsKeys` in `src/app/config.h` (line 75), with `retired_in` set to the version that ships this (2.12.0 unless the main session numbers the release differently). Nothing else is needed for old files: `Settings::load_file` skips any key no setting has (`config.cpp` line 267), and `save_file` writes only the keys in the table, so a 2.11.x INI loads cleanly and loses the two lines at its next save. The existing test "each retired settings key loads silently and is never written back" (`tests/test_config.cpp` line 620) covers both keys the moment they are in the list. A user who unticked the box in 2.11.x (`check_updates=0`) gets checks again; that is the "No off switch" ruling, and the D-record and release note say so.

`skipped_update_version` and `last_run_version` stay as they are. Skip still writes the version and the start-up check still stays quiet for that version (`app_state.cpp` line 1180); only the `!manual` half of that line goes, since no manual check exists any more. `Later` still writes nothing; what the bar shows after it is question 1 below.

The strip moves. `render_update_strip` (`src/ui/library_toolbar.cpp` line 48) is deleted from the toolbar file, and its drawing becomes part of the new bar in `src/ui/version_bar.cpp`. The toolbar's `render_actions_row` keeps the status line and the bad-rules lines under it and loses only the `render_update_strip(app)` call (line 213).

## The version bar

Where it sits. The main window is begun by `begin_hydra_window` and drawn by `render_main_window`, both in `src/ui/library_view.cpp`; the toolbar is `detail::render_actions_row(app)` at line 186. The bar is a new call, `detail::render_version_bar(app)`, placed just before that line, so it is the first thing in the window. There is no `app_shell.cpp` toolbar; `app_shell.cpp` owns the OS window and the fonts, not this row. The startup screen (`render_startup_screen`, the library-file progress shown for a few seconds) keeps its own look and has no bar; the bar comes with the main window. That is listed under "Out of scope" for the user to confirm.

How it is drawn. The bar is a child window, `##versionbar`, with `ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysUseWindowPadding`, the same way the batch strip is built (`render_batch_strip`, `src/ui/library_dialogs.cpp` line 427). Its background is a new theme colour, `kVersionBarBg`, a shade darker than `kWindowBgColor` (37,37,38): the plan proposes (30,30,31), and the executor keeps whatever value passes the contrast tests below. Under the child, one hairline the full width of the window in the theme's separator colour (`ImGuiCol_Separator`, set to the border grey in `apply_theme`, `src/ui/theme.cpp` line 53), drawn with `ImGui::Separator()`. The bar's height is its one line of content plus the window padding; a narrow window that wraps the sentence makes it two lines, and the toolbar moves down with it.

The name. `Hydra Deluxe <version>` comes from `core::named_version(core::running_version())`, the one text of a version (see "Where the text comes from" below). It draws in a new text role, `TextRole::AppVersion`, default font at 20 px. A new role is needed because `src/ui/text_role.h` says a role is picked by asking "what is this text?", and no existing role is "the app's own name and version": `Label` draws at the row's size, so it would not be larger, and `DialogQuestion` (also default, 20 px) means a dialog's question, so borrowing it would be a font pick in disguise. Heavier is not on offer: `src/ui/fonts.h` loads one weight of the default font and the mono font, so a bold name would mean a new font file; the plan chooses larger. `kTextRoleCount` becomes 12 and the static_assert text changes with it; `tests/test_text_role.cpp` gets the role's row. The name is text, not a button: a click does nothing, and hovering it shows nothing.

The status, after the name on the same line. The sentence draws in `TextRole::Prose` at the normal size, on the name's baseline (`baseline_shift(TextRole::Prose, TextRole::AppVersion)`), with the dot before it. The dot is a filled circle the way `pad_dot` draws one in `src/ui/dynamics_tab.cpp` (line 61): radius `px(5)`, on the window draw list, with a `Dummy` reserving its room so the sentence never shifts. The dot slot is reserved in every state, so the sentence starts at the same x whether or not a dot is drawn.

One owner for the words. `AppState::version_bar_status()` returns a small struct: `dot` (an enum: `None`, `UpToDate`, `Available`), `text` (the sentence) and `dimmed` (a second, dimmed sentence, empty in most states). The bar only draws it. The rule, in order, for when no update phase is showing (`update.phase == None`):

1. `app::installed_by_setup()` is false: no dot; `Only the installed copy of Hydra Deluxe checks for updates.` in `kDimTextColor` (the sentence the greyed `Check now` showed; it moves from `settings_bar.cpp` line 216 to a named constant beside `kUpdateDeclinedText` in `app_state.cpp`, line 1077).
2. The check is running (`update_check_running()`): no dot; `Checking for updates...`
3. The check failed: no dot; `text` is `Hydra couldn't check for updates when it started.` and `dimmed` is the failure's `plain_error` sentence, for example `Hydra couldn't reach GitHub. Check your internet connection and try again.` The detail (`plain_error_detail`) is not shown. Both come from the job (`job->message()` is `plain_error`), never from `e.what()`, per the error-lines-derived-once rule. The next start checks again.
4. The check found a newer release that is not being offered right now (after `Later` or `Skip this version`): amber dot; `Hydra Deluxe 9.9.9 is available.` and, after a skip, `dimmed` reads `Skipped.` Hovering the sentence shows the release's notes, as the offer does. Question 1 asks the user to confirm this.
5. Otherwise: green dot; `Up to date`.

When an update phase is showing (`update.phase != None`), the bar draws the phase instead of rules 1 to 5, with the phase's own buttons, exactly as `render_update_strip` draws them today: `Offered` and `Declined` show the sentence with `Update`, `Skip this version` and `Later`; `Downloading` shows the sentence, the MB text, the progress bar and `Cancel`; `Verifying` and `Launching` show their sentence; `Failed` shows the sentence, `Update` when a release is known, and `Later`; `Updated` shows `Updated to Hydra Deluxe <version>.` with `OK`. The `Offered` sentence carries the amber dot; `Declined`, `Failed` and `Updated` carry none, so a dot always means a fact about versions and never a step in progress. The `detail` line (the installer's or Windows' reason) draws under the sentence with `error_detail_line`, as today.

The `Updated` note belongs in the bar too (recommended, question 3). It is the first start after a version change; the bar is where the version lives. While it shows, the start-up check still runs behind it. Today `collect_update_check` drops a found release when the strip is not empty (`app_state.cpp` line 1168, "The strip changes only from nothing"). With the found release kept in `UpdateState` (below), `dismiss_update_note` offers it the moment `OK` is clicked, if it is newer and not skipped; otherwise the bar falls to rule 5.

To answer rules 3 to 5 after the offer is gone, `UpdateState` grows: `last_check` (an enum: `NotRun`, `Failed`, `UpToDate`, `Newer`), `check_error` (the `plain_error` sentence when `Failed`) and `found` (the `app::UpdateRelease` the check returned when `Newer`, notes included, so the hover works after `Later`). `update.release` keeps its shipped meaning (the release offered or being installed) and is still cleared by `later()`.

Where the text comes from. The one home of the app's version is `HYDRA_VERSION` from `CMakeLists.txt`, checked by `src/core/version.h`, and read as numbers by `core::running_version()` in `src/core/app_version.h`. The strip already builds "Hydra Deluxe 9.9.9" in `named_release` (`app_state.cpp` line 1080, an anonymous-namespace helper: `HYDRA_APP_NAME + " " + version_text(version)`). That helper moves into `src/core/app_version.h` as `core::named_version(const AppVersion&)`; the phases call it and the bar's name calls it with `running_version()`. No file spells "Hydra Deluxe 2." or pastes `HYDRA_VERSION` into a sentence. A single-owner row pins that (T1a, criterion A6).

## Colours

Every colour has one owner in `src/ui/theme.h`. Three entries are new: `kVersionBarBg` for the bar's background, `kUpToDateDotColor` (a green) and `kUpdateAvailableDotColor` (an amber). The existing colours are reused for the rest: the name and the sentences in `kDefaultTextColor`, the dimmed sentences (`Skipped.`, the failure reason, the not-installed line) in `kDimTextColor`, the hairline in the theme's separator grey.

Why two new dot colours rather than borrowing. `kWarningColor` (255,127,0) is the orange of a problem line, and an available release is not a problem. The chip tier colours (`kChipRgb*` in `src/app/html_page.h`) mean timing tiers; a dot that shared one would carry that meaning into the bar. `kDoneStripBg` is a background, too dark for a dot. The executor picks the two values and `tests/test_theme.cpp` pins them: each dot reads at 3:1 or better on `kVersionBarBg` (WCAG's floor for a graphic), `kDefaultTextColor` and `kDimTextColor` read at 4.5:1 or better on `kVersionBarBg`, and `kVersionBarBg` is darker than `kWindowBgColor` in every channel. Starting values for the executor to test: green (80,190,120), amber (235,180,60), bar (30,30,31).

## Words on screen

Every sentence that is new or moves, in one place:

- Bar, left: `Hydra Deluxe <version>`, larger, from `core::named_version(core::running_version())`.
- Bar, resting states (the rule above): `Up to date` (green dot); `Checking for updates...`; `Hydra couldn't check for updates when it started.` with the reason dimmed after it; `Hydra Deluxe <new> is available.` (amber dot), with `Skipped.` dimmed after it when skipped; `Only the installed copy of Hydra Deluxe checks for updates.` dimmed.
- Bar, update phases: unchanged wording for Offered (amber dot), Downloading, Verifying, Launching, Declined, Failed and Updated, with their buttons, now in the bar. `UpToDate` is gone.
- Under the toolbar: nothing about updates.
- Settings panel: the `App` group, the `Check for updates` row, its help text and `Check now` are gone. The panel is `Chart`, `Clone Hero rules`, `Paths kept`, then `Reset to defaults`.

## Questions for the user

**Q1. What `Later` means now, and what the bar shows after it.** The shipped rule: `AppState::later()` (`app_state.cpp` line 1258) clears the strip and writes nothing, and the strip comes back when the next automatic check finds the release again. Today that next check is at the first start more than 24 hours after the last one (`update_check_due`). Example today: you click `Later` at 9:00, close Hydra, open it at 14:00 and at 18:00; the offer stays hidden both times and comes back the first start after 9:00 tomorrow. With every start checking, the same clicks bring the offer back at 14:00. Recommended: keep `later()` as it is, so `Later` means "put the buttons away until Hydra is next started", and the bar keeps the fact: amber dot, `Hydra Deluxe 9.9.9 is available.`, no buttons, notes on hover. The bar never says `Up to date` while a newer release is known, because that would be false. Alternative A: after `Later` the sentence keeps its `Update` button and loses only `Skip this version` and `Later`, so you can still update in this run without restarting. Alternative B: `Later` remembers the version and the time and hides it for 24 hours; that keeps a timestamp key and the time-parsing code this plan removes, for one button. Not recommended.

**Q2. `Skip this version` after this change.** The shipped rule: `skip_version()` writes `skipped_update_version=<version>` and the automatic check lands in `None` for that version (`app_state.cpp` lines 1180 to 1181), and only `Check now` ignored the skip (the user's fifth ruling during T3a). Example: skip 2.12.0, and no start offers 2.12.0 again; 2.12.1 is offered. With `Check now` gone, nothing ignores the skip any more. Recommended: keep it so, and have the bar say what it knows on every start while 2.12.0 is the latest: amber dot, `Hydra Deluxe 2.12.0 is available.` with `Skipped.` dimmed after it, no buttons. A skipped release is still out there, and the bar is the one place that tells you. Alternative: after a skip the bar shows the green `Up to date`; rejected because it is not true, and the bar's one job is to say where you stand. The only way to install a skipped release is the next release, or the installer from GitHub by hand (the guide says so today).

**Q3. The `Updated to Hydra Deluxe <version>.` note.** The shipped rule: the first start after a version change puts the strip in `Updated` with one `OK` (`start_updates`, `app_state.cpp` line 1095), and nothing else shows until `OK` is clicked. Example: install 2.12.0, start it, read `Updated to Hydra Deluxe 2.12.0.`, click `OK`. Recommended: it moves into the bar with everything else, in the sentence slot, and `OK` gives the slot back to the resting rule (or to the offer, if the start-up check found a newer release meanwhile). Alternative: keep it under the toolbar as the only update line there; rejected because the design says nothing about updates stays under the toolbar.

**Q4. Dots only for facts.** The design names a green dot for `Up to date` and an amber dot for an available release. The plan draws no dot for `Checking for updates...`, the failed check, the not-installed line, the download and installer steps, `Declined`, `Failed` and `Updated`, and reserves the dot's room so the sentence never moves. Example: on a start with no network the bar reads `Hydra couldn't check for updates when it started. Hydra couldn't reach GitHub. Check your internet connection and try again.` with no dot. Alternative: a grey dot in those states, so there is always a dot. Recommended: none; a dot then means one of two facts and nothing else.

**The user's answers (2026-10-23, in chat).** These replace the recommendations above wherever they differ, and the executors build these.

Q1: "Keep an Update button". This is alternative A. After `Later`, the bar keeps the amber dot and `Hydra Deluxe <new> is available.` with its `Update` button, and loses only `Skip this version` and `Later` until the next start. `later()` still writes nothing.

Q2: "Available, marked skipped". As recommended: amber dot, `Hydra Deluxe <new> is available.` with `Skipped.` dimmed after it, no buttons, on every start while the skipped release is the latest.

Q3: "In the top bar". As recommended: the `Updated to Hydra Deluxe <version>.` note takes the bar's sentence slot with its `OK`.

Q4: "Grey dot for the rest". The alternative: green means up to date, amber means a newer release is known, and every other state (checking, failed, not installed, the download and installer steps, `Declined`, `Failed`, `Updated`) shows a grey dot. So the bar always shows a dot. The grey comes from the theme with one owner, like the other two.

Everything else in this plan is a consequence of the rulings quoted at the top, or an exact wording for the user to read in "Words on screen".

## The build

Three executor tasks on Opus. T1a goes first from `main`. T1b and T2 fork from T1a's join and run in parallel. Each is under 100 tool calls and 20 minutes. T1a is the largest and its files are listed so the executor edits nothing else.

Interfaces fixed here so T1b and T2 agree with T1a without talking: `std::string core::named_version(const core::AppVersion&)` in `src/core/app_version.h` ("Hydra Deluxe 2.11.1"); `void AppState::check_updates_on_start()` (replaces `check_updates_if_due`; `check_updates_now` is removed); `struct VersionBarStatus { enum class Dot { None, UpToDate, Available } dot; std::string text; std::string dimmed; }` in `src/ui/update_job.h` beside `UpdateState`, and `VersionBarStatus AppState::version_bar_status() const`; `bool AppState::update_check_running() const` (unchanged); `UpdateState::Phase` without `UpToDate`; `UpdateState::last_check`, `check_error` and `found`; the constants `kOnlyInstalledChecksText`, `kCheckingForUpdatesText`, `kCouldNotCheckText`, `kUpToDateText` and `kSkippedText` beside `kUpdateDeclinedText` in `app_state.cpp` (T1b never reads them; it draws `version_bar_status()`); `TextRole::AppVersion` in `src/ui/text_role.h`; `kVersionBarBg`, `kUpToDateDotColor` and `kUpdateAvailableDotColor` in `src/ui/theme.h`; `void detail::render_version_bar(AppState&)` declared in `src/ui/library_parts.h`.

### T1a: the rule, the state and the version text (owner: `src/app/config.h`, `src/app/config.cpp`, `src/core/app_version.h`, `src/ui/app_state.h`, `src/ui/app_state.cpp`, `src/ui/update_job.h`, `tests/test_config.cpp`, `tests/test_update_state.cpp`, `tests/single_owner/rules/core.json` one new row; from `main`)

Remove `update_check_due`, `kUpdateCheckInterval`, `update_check_time_text`, `update_check_time`, `Settings::check_updates` and `Settings::last_update_check`, and add the two keys to `kRetiredSettingsKeys`. In `AppState`, replace `check_updates_if_due` with `check_updates_on_start` (gate: `installed_by_setup()` only), delete `check_updates_now` and the `manual` flag, delete the `UpToDate` phase and the branch that set it, add `UpdateState::last_check`, `check_error` and `found`, fill them in `collect_update_check` (the found release is kept even while a phase shows, so the `Updated` note no longer loses it), make `dismiss_update_note` offer the found release when it is newer and not skipped, and write `version_bar_status()` as "The version bar" says. Move `named_release` to `core::named_version`. The comments that named decisions 5, 6 and 8 of the old plan now point at this plan's D-record.

Acceptance criteria, each a test:
- A1 (`test_config.cpp`) the key-order test's expected list no longer has `check_updates`; the updater-keys test round-trips `skipped_update_version` and `last_run_version`, and `with_analysis_defaults` leaves both alone. The retired-keys test runs green with the two new rows, so an INI holding `check_updates=0` and `last_update_check=2026-10-22T13:45:00Z` loads and neither line is written back.
- A2 (`test_update_state.cpp`) with `set_installed_for_test(true)`, a new `AppState` has `update_check_running()` true before its first tick and `version_bar_status().text` is `Checking for updates...` with `dot == None`; with `false` it is never true, and after five ticks the status is `Only the installed copy of Hydra Deluxe checks for updates.` with `dot == None`. This replaces the C2 `update_check_due` case and the ISO-time case, which are deleted.
- A3 with the canned reply at v9.9.9, the start lands in `Offered` with text `Hydra Deluxe 9.9.9 is available.`; after `later()` the phase is `None` and the status is that sentence, `dot == Available`, `dimmed` empty, and `update.found->notes` is the fixture's notes; after `skip_version()` on a fresh start the phase is `None`, the INI says `skipped_update_version=9.9.9`, and the status is the sentence with `dimmed == "Skipped."` and `dot == Available`. A reply at v9.9.10 on the next start with 9.9.9 skipped lands in `Offered`.
- A4 with the reply at the running version, and again at v0.0.1, the start lands in `None` with empty strip text and the status `Up to date`, `dot == UpToDate`; no `UpToDate` enumerator exists (the test compiles against the enum).
- A5 with the fetcher throwing `NetHttpStatus` 403 and then `NetUnreachable` (the two shipped cases), the start lands in `None` with empty text and detail, and the status is `text == "Hydra couldn't check for updates when it started."`, `dimmed` equal to the matching `plain_error` sentence, `dot == None`. While the fetcher is held, the status is `Checking for updates...`.
- A6 `core::named_version(AppVersion{2, 11, 1})` is `Hydra Deluxe 2.11.1`, and the new single-owner row (question "What is the version text the user reads?", owner `named_version`) matches `std::string(HYDRA_APP_NAME) + " " + HYDRA_VERSION` and `"Hydra Deluxe " HYDRA_VERSION` and does not match `core::named_version(core::running_version())`; the scan runs green, which proves no second copy exists in `src/`.
- A7 the shipped cases for `Update`, the declined installer, the failed start, the bad signature, Cancel, the "updated" start and the leftover folder (`test_update_state.cpp` from line 341) still pass, edited only where they called `check_updates_now` or read `last_update_check`.
- A8 a start with `last_run_version` at 0.0.1 and the reply at v9.9.9 lands in `Updated`; after `dismiss_update_note()` the phase is `Offered` for 9.9.9. The same with 9.9.9 skipped: after `OK` the phase is `None` and the status names 9.9.9 with `Skipped.`. The same with the reply at the running version: after `OK` the status is `Up to date`.

Tests it may run: `hydra_tests.exe -sf=*test_update_state.cpp`, `-sf=*test_config.cpp`, `-sf=*test_update_release.cpp`, and the single-owner scan case `-tc=*single owner*` once.

### T1b: the bar, the toolbar and the settings panel (owner: new `src/ui/version_bar.cpp`, `src/ui/library_view.cpp` the one call, `src/ui/library_toolbar.cpp`, `src/ui/library_parts.h`, `src/ui/settings_bar.cpp`, `src/ui/text_role.h`, `src/ui/theme.h`, `CMakeLists.txt` the one source line, `tests/test_text_role.cpp`, `tests/test_theme.cpp`, `tests/ui/uitest_library.cpp`; forks from T1a's join)

Write `render_version_bar` in the new file as "The version bar" says: the child with `kVersionBarBg`, the name in `TextRole::AppVersion`, the dot and the status from `version_bar_status()`, or the phase with its buttons when a phase shows (the body of today's `render_update_strip`, moved, minus the `UpToDate` branch), then the hairline. Call it first in `render_main_window`. Delete `render_update_strip` and its call from `library_toolbar.cpp`. Add `TextRole::AppVersion` (default, 20 px), bump `kTextRoleCount` to 12 and add its test row. Add the three theme colours and their contrast checks. Delete `render_app_group` and its call in `render_panel`, and put the panel's comment back to three groups. Rewrite the `update-strip` uitest as `version-bar`: every place that clicked `Check now` now restarts the app with `restart_app` (already in the file, line 1336) so the start-up check runs, and every text is read in the bar.

Adding a source line to `CMakeLists.txt` is a build-file change, so at merge the main session runs the whole suite (agent rule 1); the executor runs only the tests named below.

Acceptance criteria, in the `version-bar` script unless named:
- B1 the child `##versionbar` exists; the text `Hydra Deluxe <running>` (from `core::named_version(core::running_version())`) is on screen inside it, its rectangle's left edge is at the window padding and its top is above the `Manage folders...` button's top; clicking it changes nothing (the phase stays, no popup opens, the status text is the same after the click).
- B2 with the reply at the running version, a restart lands with `Up to date` on screen in the bar and no update text anywhere under the toolbar (`Hydra Deluxe <running> is the latest version.` and `OK##update` are not on screen).
- B3 with the reply at v9.9.9, a restart shows `Hydra Deluxe 9.9.9 is available.` in the bar with `Update##update`, `Skip this version` and `Later`, and the notes on hover of the sentence (the shipped D1 mouse moves, with the sentence's rectangle taken from the bar); after `Later` the buttons are gone and the sentence is still on screen, with the notes still on hover; after `Skip this version` and a restart no buttons show, the sentence is still on screen and `Skipped.` is on screen.
- B4 with `set_installed_for_test(false)`, a restart shows `Only the installed copy of Hydra Deluxe checks for updates.` in the bar and `update_check_running()` is never true.
- B5 with the reply `this is not JSON`, a restart shows `Hydra couldn't check for updates when it started.` and the job's `plain_error` sentence in the bar, no `Later` and no `Update##update`.
- B6 the `Analysis settings` panel has no `App` group: `on_screen("Check for updates")` is false, `ItemExists("**/Check now")` is false, the three groups and `Reset to defaults` are still there, and with a batch holding the settings the lock line still reads `Stop the batch to change these.`
- B7 the shipped D3 (download with `Cancel##update`, the declined installer's sentence with the three buttons back) and D5 (`Updated to Hydra Deluxe <running>.` with `OK##update`, then nothing on the next start) still pass with the texts read in the bar; after `OK` with the reply at v9.9.9 the offer appears in the bar (A8 on screen).
- B8 (`test_text_role.cpp`) `font_for(TextRole::AppVersion)` is the default font at 20 px, and `kTextRoleCount == 12`; the font-role scan (`tests/single_owner/rules/ui/text_role.json`) runs green, which proves `version_bar.cpp` pushes no font itself.
- B9 (`test_theme.cpp`) `kUpToDateDotColor` and `kUpdateAvailableDotColor` each read at 3:1 or better on `kVersionBarBg`; `kDefaultTextColor` and `kDimTextColor` read at 4.5:1 or better on it; each channel of `kVersionBarBg` is below the same channel of `kWindowBgColor`; the dot colours differ from `kWarningColor`.
- B10 the existing `settings-reset` and `settings-and-reports` scripts still pass; `hydra_uitest update-strip` no longer exists (the name is `version-bar`).

Tests it may run: `hydra_uitest version-bar`, `hydra_uitest settings-reset`, `hydra_uitest settings-and-reports`, `hydra_tests.exe -sf=*test_text_role.cpp`, `-sf=*test_theme.cpp`, and the single-owner scan case `-tc=*single owner*` once.

Memory rule `ui-changes-need-real-app-check` applies to the bar: before merging T1b the main session captures the top of the real window at the user's window size, with the song panel closed and open, in the `Up to date` state and (through `hydra_start("window")` with the canned v9.9.9 reply if the harness allows it, otherwise the headless capture) in the offer state, and shows the user both.

### T2: the public docs, CONTEXT.md and the decision record (owner: `README.md`, `docs/UserGuide.md`, `docs/handoffs/release-next-notes.md`, `docs/audit/2026-10-03-fix-decisions.md`, `CONTEXT.md` the "Update check" paragraph, `docs/agents/ui-testing.md` the toolbar rows only; forks from T1a's join, in parallel with T1b)

README, install step 1 (line 28): "once a day when it starts and tells you under the toolbar" becomes "each time it starts and tells you in the bar at the top of the window, where the version you run is shown", and the last sentence ("To stop the check, untick `Check for updates` in `Analysis settings`.") goes.

User Guide. Line 9 ("The toolbar runs along the top") gains the bar: a sentence that the version bar runs along the top, with the toolbar under it. Line 17 (the `Updated to` note) says "in the version bar" instead of "under the toolbar" (per Q3's answer). Toolbar section (line 53): a paragraph before it, "The version bar", naming `Hydra Deluxe <version>` and the five resting sentences in plain words, with the dots. "When a new version is out" (lines 73 to 91): "once a day" becomes "each time it starts"; "under the toolbar" becomes "in the version bar" throughout; the sentences about 24 hours, the unreadable time and the future time go; "not even with `Check now`" goes; `Later` reads per Q1's answer; the `Check now` half of the skip paragraph goes and the skip paragraph says what the bar shows after a skip (Q2); the failure paragraph (line 89) says a failed start-up check shows its sentence and the reason in the bar with no buttons, and the next start tries again; the last paragraph (line 91, the `Check now` up-to-date line) becomes the `Up to date` sentence. Settings section: line 97 loses the `Check for updates` sentence; line 99 reads "The panel has three groups."; the table loses the `App` row (line 111); line 113's reset sentence loses "`Check for updates`,"; line 117's lock sentence loses the `Check for updates` sentence; the paragraph at line 137 goes.

`docs/differences-from-public-hydra.md`: its 2.11.0 row (line 143) is history and stays. The row for the release that ships this is added at release time as every row is, with the words "checks for a newer release each time it starts and shows the version, and whether a newer one is out, in a bar at the top of the window".

`release-next-notes.md`: one paragraph after the shipped one, saying the check now runs at every start, the `App` group and its box are gone (a box you had unticked no longer stops the check), the update line moved from under the toolbar into a new bar at the top of the window, and that bar always shows the version you run and whether you are up to date.

CONTEXT.md "Update check" (lines 415 to 426): three questions, not four. "Should a check run now" and `update_check_due` go; "Is this the installed exe" says it is the only gate on the start-up check; a sentence names `named_version` as the one text of a version, `version_bar_status` as the one answer the bar shows, and `render_version_bar` in `src/ui/version_bar.cpp` as the one place that draws it; "under the toolbar" becomes "in the version bar".

`docs/agents/ui-testing.md`: a new row above the toolbar row for the version bar (child `##versionbar`): the name text, the resting sentences, the update buttons by label (`Update##update`, `Skip this version`, `Later`, `Cancel##update`, `OK##update`), and a note that the name has no click. The toolbar row (line 106) stays as it is; any mention of the update strip under the toolbar goes.

The D-record, `D260`, after D259 in `docs/audit/2026-10-03-fix-decisions.md`: the rulings in the user's words, the answers to Q1 to Q4, the list of what was removed (the `App` group, `Check for updates`, `Check now`, the 24-hour wait, the up-to-date strip, the update strip under the toolbar, the two INI keys), the bar's sentences and dots, the new text role and theme colours, and the note that D259's calls 5, 6 and 8 and its rulings (2) and (5) are superseded.

Acceptance criteria:
- C1 `hydra_tests.exe -sf=*test_docs_match_code.cpp` runs green on the joined tree: it checks that every code name a doc uses exists, so a CONTEXT.md or guide that still named `update_check_due`, `check_updates`, `render_update_strip` or `Check now` as a code name would fail it once T1a and T1b have removed them.
- C2 a grep of `README.md`, `docs/UserGuide.md`, `CONTEXT.md`, `docs/handoffs/release-next-notes.md` (the new paragraph only) and `docs/agents/ui-testing.md` for `Check now`, `Check for updates`, `once a day`, `24 hours`, `check_updates` and `under the toolbar` next to an update sentence finds nothing; the reviewer runs it and the D-record quotes it. (`docs/handoffs/release-2.11.*-notes.md`, the shipped paragraph of `release-next-notes.md` and D259 are history and keep their words.)
- C3 the D-record exists, names D260, quotes the rulings and the Q1 to Q4 answers, and lists the removals.
- C4 every sentence in the guide and README matches "Words on screen" and the bar rule; the reviewer reads them side by side (`docs/agents/public-doc-style.md` applies: second person, every label checked against the code).

Tests it may run: `hydra_tests.exe -sf=*test_docs_match_code.cpp`.

## Testing without GitHub

Nothing touches the network. The unit tests keep `UpdateWorld` (`tests/test_update_state.cpp` line 82 on) and the uitest keeps `UpdateSeams` (`tests/ui/uitest_library.cpp` line 1290 on); both already serve the release reply, the download and the installer start through the shipped seams, and set `installed_by_setup` through `set_installed_for_test`. Where a test used `Check now` to run a second check, it now constructs or restarts the app, which is what a user does.

The user's real check, after the merge and before the release: start the installed 2.11.1 once from the new build's installer, and see the bar with `Hydra Deluxe 2.12.0` and the green `Up to date`; copy `Hydra.exe` to the desktop and start it to see the "only the installed copy" line; pull the network cable and start it to see the failed line with its reason. The offer itself is seen on the first release after the one that ships this, as D259 said.

## Out of scope (for the user to approve)

- A click on the version name (opening the release page, say). The ruling was "Text only" for the label, and the bar's name keeps it.
- A bar on the startup screen (the library-file progress screen); the bar comes with the main window.
- A bold weight of the default font; the name is larger, not heavier, because no bold font is loaded.
- Any change to `Update`, the download, the signature check or the installer launch.
- A time-based reminder for `Later` (Q1's alternative B).
- A new row in `docs/differences-from-public-hydra.md` before release day.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| Every start of the installed exe checks; no 24-hour wait | What changes | T1a | A2 (running before the first tick), A3 (a second start checks again) |
| A copy not put in place by the installer never checks | What changes; bar rule 1 | T1a, T1b | A2, B4 |
| No off switch: `check_updates` is not read | What changes | T1a | A1 (key list), A2 (checks with no setting) |
| Old `check_updates` and `last_update_check` lines load and are not written back | The old INI keys | T1a | A1 via the retired-keys test |
| No `Check now`, no `App` group, panel back to three groups | Words on screen | T1b | B6 |
| The up-to-date strip and its OK are gone; `Up to date` is the resting state | What changes; bar rule 5 | T1a, T1b | A4 (no `UpToDate` enumerator, status text), B2 |
| Version text derived once from `HYDRA_VERSION` through `named_version` | Where the text comes from | T1a | A6 single-owner row and scan |
| The bar is always shown, first in the window, above the toolbar | The version bar | T1b | B1 (child exists; name above `Manage folders...`) |
| The name is larger, in a role; no raw font pick | The name | T1b | B8 |
| The name has no click and no hover | The name | T1b | B1 |
| Bar words from one owner: only installed / checking / failed with plain_error / available (Skipped.) / Up to date | One owner for the words | T1a, T1b | A2 to A5 (the function), B2 to B5 (on screen) |
| Failure lines come from plain_error, never e.what() | Bar rule 3 | T1a | A5 (`dimmed` equals the plain_error sentence) |
| The bar names the newer version after Later or Skip, notes on hover | Bar rule 4; Q1, Q2 | T1a, T1b | A3, B3 |
| A skipped release says `Skipped.` | Q2 | T1a, T1b | A3, B3 |
| Later hides the buttons until the next start and writes nothing | Q1 | T1a, T1b | A3, B3 |
| Skip hides that version only; a newer one shows | Q2 | T1a | A3 |
| Nothing about updates stays under the toolbar | The strip moves | T1b | B2, B3, B7 (texts read in the bar; `render_update_strip` gone, C1) |
| The Updated note shows in the bar; OK gives way to the offer or the resting state | Q3 | T1a, T1b | A8, B7 |
| Dots only for the two facts; room reserved in every state | Q4 | T1a, T1b | A2, A4, A5 (`dot` values), the real-app capture for the layout |
| Three new theme colours with one owner each; contrast floors | Colours | T1b | B9 |
| Update, Cancel, declined start, bad signature, "updated" start unchanged | Out of scope; A7 | T1a, T1b | A7, B7 |
| Existing settings panel flows still pass | T1b | T1b | B10 |
| Public docs say every start, no box, the bar and its sentences | T2 | T2 | C1, C2, C4 |
| D260 records the rulings and the removals | T2 | T2 | C3 |
| CONTEXT.md names the owners that exist | T2 | T2 | C1 |
| No network in tests | Testing without GitHub | T1a, T1b | A3 and B3 run through the seams; the harness's fetcher throws for any other URL |
| Real-app capture of the bar before merge | T1b | main session | the user sees the captures |
| Whole suite at merge because `CMakeLists.txt` changes | T1b | main session | agent rule 1 |
