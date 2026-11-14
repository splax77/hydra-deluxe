Hydra Deluxe 3.0.1 makes the Paths tab's `Dynamic Hit Window:` row say which note is which. If you run 3.0.0, your saved results carry over as they are: nothing reads `Stale` and nothing is analyzed again. If you update from 2.14.0 or earlier, the 3.0.0 notes apply: your results read `Stale` once.

**Update from the app.** If you run 2.12.0 or later, the bar at the top of the window offers this release the next time Hydra Deluxe starts; click `Update`. If you run 2.11.0 or 2.11.1 with its update check on, a line under the toolbar offers it once its daily check runs. From 2.10.2 or earlier, download the `-setup.exe` file below and run it.

## Paths tab

**The `Dynamic Hit Window:` row names each note.** Each note reads `frontend` or `backend`, then its window, then its chord, written the way the `Backend timings` table writes chords. `frontend` means you hit that note early, on the front of its window, and `backend` means you hit it late, on the back of its window. The frontend note always comes first. On Burnout's optimal path in the test charts, activation 1's squeeze out reads `frontend 84.5 ms [K   G]   backend 84.3 ms [  Y  ]` with `5.4 ms safe`: you hit Kick and Green early and Yellow late.

**The row agrees with the sentence above it.** In 3.0.0 the row said `early` and `late` by squeeze type alone. On a squeeze whose badge figure is below 0, such as `squeeze in -83.3 ms`, the sentence says to hit the Star Power phrase's last note "no more than 83.3 ms late", while the row called that same note `early`. The row now calls it `backend`, and every row takes its words from the same rule as the sentence.

**The tag reads `safe` instead of `spare`.** A yellow tag like `5.4 ms safe` means the game's windows cover the timing with that much room, and room of exactly 0.0 ms reads `safe`. An orange tag still reads `short`. The tag's figure is the same sum as before, with one exception: when the note that moves the Star Power end sits where the measures change length, such as a tempo or time signature change, its window now counts on the side it is hit on. On such a squeeze the figure can move a little: on Dead on Arrival (O)'s optimal path, activation 3 went from `608.7 ms spare` to `609.3 ms safe`.

The squeeze sentences and the `Early fill:` line still say early and late. Badges, `Rating` words and the path report's tiers still use the fixed 85 ms hit window.
