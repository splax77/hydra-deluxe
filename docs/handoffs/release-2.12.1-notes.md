Hydra Deluxe 2.12.1 fixes the frontend timing scale line on the Paths tab, which could name a scale nothing on the activation used.

**Your saved results carry over.** Nothing reads `Stale` after the update, and nothing is analyzed again. Scores and paths don't change.

**Update from the app.** If you run 2.12.0, the bar at the top of the window offers this release the next time Hydra Deluxe starts; click `Update`. If you run 2.11.0 or 2.11.1 with its update check on, a line under the toolbar offers it once its daily check runs. From 2.10.2 or earlier, download the `-setup.exe` file below and run it.

**The scale line names only the scales the row uses.** When you open an activation on the Paths tab, a line such as `Frontend timing scales x0.83 (early) / x1.20 (late) at the SP end.` can show under its chord. A backend note inside Star Power uses the early scale, and a note past the SP end uses the late one. The line used to name both scales even when every note on the row used only one of them. Now it names only the scales the row's backend notes and squeezes use, and when none is left there's no line. For example, an activation whose backend notes all sit inside Star Power now shows only `Frontend timing scales x0.83 (early) at the SP end.` The `eff.` figures in the `Backend timings` table were already right and don't change.
