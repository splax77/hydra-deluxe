Hydra Deluxe 2.13.0 adds an `Album` column and album search to the library, adds two squeeze search filters, and fixes where Clone Hero 1.0 mode places generated fills near a time signature change.

**Your saved results read `Stale` after the update.** A Clone Hero 1.0 mode fill can move, so every saved result is marked out of date. Click a song, or click `Analyze library...`, to refresh them. Only a result in Clone Hero 1.0 mode, on a chart with a generated fill near a time signature change, can come back different.

**Click `Scan library` once to fill in albums.** Until you do, the `Album` column stays empty. That first scan reads every chart again, so it takes longer than a usual scan.

**Update from the app.** If you run 2.12.0 or 2.12.1, the bar at the top of the window offers this release the next time Hydra Deluxe starts; click `Update`. If you run 2.11.0 or 2.11.1 with its update check on, a line under the toolbar offers it once its daily check runs. From 2.10.2 or earlier, download the `-setup.exe` file below and run it.

**An `Album` column in the library.** It sits between `Artist` and `Charter`. The album comes from the chart's song.ini, or from the file itself for a `.sng` or `.srb` chart. A chart that names no album shows an empty cell. Like `Charter` and `Folder`, the column hides while the song panel is open; a word search then shows the album on a second line under the artist.

**Search by album.** Type `album:dookie` to list songs with that word in the album. Plain words in the search box now match the album too.

**Two more squeeze filters.** `squeeze>20` lists songs whose best path has a squeeze or required early fill of more than 20 ms. `squeeze>=20` includes 20 ms. Pair them with `squeeze<=` for a range, such as `squeeze>=10 squeeze<=200`. A squeeze filter with any other sign, such as `squeeze=20`, now shows `squeeze takes <=, <, >= or >, like squeeze<=20` under the box. Before, Hydra Deluxe searched for it as a plain word.

**Search tips.** The long hint under the search box is now a short line, `Type any words, or narrow it down.`, and a `Search tips` link. Click it for every filter with an example.

**Clone Hero 1.0 fills near a time signature change.** In Clone Hero 1.0 mode, on a chart with no fills of its own, Hydra Deluxe places fills the way the game does. It counted measures differently from the game around a time signature change, so a fill near one could land in the wrong place. Now it counts measures the same way as the rest of the app. The fill rule itself doesn't change.

**Every early fill shows a badge.** An activation that needs an early fill now always shows its `early fill` badge, even with time to spare. A fill with time to spare shows a figure below 0, such as `early fill -56 ms`. This holds while the margin is under 60 ms, the same fills that carry the `E` in the notation. A badge with time to spare is never orange and never counts as a path's hardest timing.

**Badges line up.** Every badge on the Paths tab is now the same width, with its text centred, so the badges in each row line up.

**Report windows no longer cover Hydra Deluxe.** A path report or leaderboard comparison window used to stay in front of the main window, so a maximized report hid Hydra Deluxe until you closed it. Now each report window minimizes, restores and comes to the front on its own, like any other window.

**A plain sentence for a chart with a broken tempo or resolution.** A chart with no tempo at its start, or with a missing or unreadable `Resolution`, now shows a sentence such as `Hydra can't analyze this chart because it has no tempo at tick 0. Fix that line in the chart file or download the song again.` It used to show a raw program error.

**`hydra_batch` asks for `--whole-library`.** Run with no chart folders, the command-line tool `hydra_batch` used to analyze every folder in your settings. It now stops and says `A run over the whole library needs --whole-library. Name chart folders to analyze just those.` Add the flag to analyze every folder, or name the folders you want. The flag and folder names together stop with a message too.
