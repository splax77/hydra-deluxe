Hydra Deluxe 2.11.0 can update itself. It checks GitHub for a newer release once a day and offers to install it.

**Your saved results carry over.** Nothing reads `Stale` after the update, and nothing is analyzed again. Scores and paths don't change.

**Install this release by hand.** The update check is new in this release, so 2.10.2 and earlier can't find it. Download the `-setup.exe` file below and run it. Later releases can come through the app.

**Hydra Deluxe tells you when a new version is out.** Once a day, when it starts, Hydra Deluxe asks GitHub for the latest release. When that release is newer than yours, a line under the toolbar reads `Hydra Deluxe <version> is available.` Hover over the version to read the release's notes. Click `Update` to download and install it. Nothing is downloaded or installed until you click. `Skip this version` hides the line until a newer release comes out, and `Later` hides it until the next check.

**Every update is checked before it runs.** Each release's installer comes with a signature file. Hydra Deluxe checks the download against it and refuses an installer that doesn't match. Windows then asks for permission to run the installer. Hydra Deluxe closes while it runs and opens again when it finishes. If you say no, nothing changes.

**You can check yourself, or turn the check off.** `Analysis settings` has a new `App` group with a `Check for updates` box and a `Check now` button. Untick the box to stop the daily check. `Check now` checks right away. Only the copy the installer put in place checks, so a `Hydra.exe` you copied to another folder never does. The first start after any update shows `Updated to Hydra Deluxe <version>.` once.

**An activation row shows every timing it has.** Before, a row showed only its hardest timing. Now a row with an early fill and a squeeze shows both badges, the fill first. When the right side is too narrow, the badges move under the row's measure and wrap if they need to.

**A tiny timing scale no longer shows as orange.** Some MIDI charts store two bars of the same length a few millionths apart, so the Paths tab showed an orange line such as `x1.000002 (late)` that changed nothing you could hit. A scale less than a thousandth from 1 now counts as no scale. A scale of exactly `x1.001` still shows.
