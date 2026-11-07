Hydra Deluxe 2.12.0 puts your version in a bar along the top of the window and checks for a newer release every time it starts.

**Your saved results carry over.** Nothing reads `Stale` after the update, and nothing is analyzed again. Scores and paths don't change.

**Update from the app.** If you run 2.11.0 or 2.11.1 with its update check on, a line under the toolbar offers this release once its daily check runs; click `Update`. From 2.10.2 or earlier, download the `-setup.exe` file below and run it.

**A version bar runs along the top of the window.** It shows `Hydra Deluxe <version>` for the version you run, then a dot and a sentence that say where you stand. A green dot and `Up to date` mean you have the latest release. An amber dot and `Hydra Deluxe <version> is available.` mean a newer one is out. A grey dot shows in every other state: while the check runs, when it fails, in a copy the installer did not put in place, and while an update downloads, starts, or fails. Everything about updates now shows in this bar, and nothing shows under the toolbar any more.

**Every start checks.** Hydra Deluxe asks GitHub for the latest release each time it starts, so you never need to check yourself. Nothing is downloaded or installed until you click `Update`. If the check fails, for example with no internet connection, the bar says so with the reason, and the next start tries again. Only the copy the installer put in place checks; a `Hydra.exe` you copied to another folder says so in the bar.

**`Later` keeps an `Update` button.** When a newer release is out, the bar offers `Update`, `Skip this version` and `Later`. `Later` puts away `Skip this version` and `Later` until the next start, but the sentence and its `Update` button stay, so you can still update without restarting. `Skip this version` hides the buttons for that release; the bar still names it, with `Skipped.` after it, until a newer release comes out.

**The update setting is gone.** `Analysis settings` no longer has an `App` group, a `Check for updates` box or a `Check now` button, and holds only the settings that shape an analysis. If you unticked `Check for updates` in 2.11.x, the check runs again.
