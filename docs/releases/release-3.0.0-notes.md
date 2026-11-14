Hydra Deluxe 3.0.0 adds the Path Builder, a second program where you build your own Star Power path and see what each choice costs. It also shows Clone Hero's own hit window on every squeeze and early fill, and reads some drum charts the way Clone Hero does.

**Your saved results read `Stale` once.** Some charts now read differently, so every result saved by 2.14.0 is marked `Stale` until you open the song or run `Analyze library...`. No scan starts, and your library and settings carry over. The first time 3.0.0 opens your library, the start box shows `Reading saved paths...` while it fills in a figure for your old results; it does this once.

**Update from the app.** If you run 2.12.0 or later, the bar at the top of the window offers this release the next time Hydra Deluxe starts; click `Update`. If you run 2.11.0 or 2.11.1 with its update check on, a line under the toolbar offers it once its daily check runs. From 2.10.2 or earlier, download the `-setup.exe` file below and run it.

## The Path Builder

The Path Builder opens a chart and lets you pick where to activate, one fill at a time. Click `Open in Path Builder` in the song panel to open the song you have open, or start it from the `Hydra Deluxe Path Builder` shortcut in the Start Menu. You can also open a chart with File > Open or by dragging it onto the window.

Three scores sit at the top: `Your score`, `Best you can still reach` and `Best possible`. Each fill is marked by what activating there costs: `Best place to activate`, `Costs under 1% of the best score`, `Costs more`, or `Can't activate here (hover to see why)`. `Show Hydra's best path` draws Hydra's own path faintly beside yours, and `Finish the rest for me` fills in the best ending from where you are. `Start over` clears your activations. You can copy, paste, save and open a path.

The Path Builder reads your `hydra_settings.ini` and `hydra_rules.ini` each time it opens a song and never changes them.

## Paths tab

**Every squeeze and early fill shows Clone Hero's own hit window.** The box for a squeeze in, a squeeze out or an early fill ends with a `Dynamic Hit Window:` row. It gives how early and how late Clone Hero accepts that note, with a tag like `5.4 ms spare` in yellow or a `short` tag in orange when the timing needs more than the window gives. The row is for reading only: badges, `Rating` words and the path report's tiers still use the fixed 85 ms window.

**Timings show one decimal.** Badges read like `squeeze out 163.0 ms` and `early fill 20.0 ms`, and the Preview's next-activation box and `Copy path` match. A squeeze exactly on the Star Power end now reads as needing 0.0 ms, for squeeze in and squeeze out alike. Before, a squeeze in on the end read as needing no timing.

**The `Rating` word in `Backend timings` follows the timing you need.** It uses the path report's tiers: `Normal`, `Hard`, `Extreme`, `Insane`, `Insane+` and `Beyond`. A note within 3 ms after the Star Power end reads `Normal`.

## Charts that read differently

These are why saved results read `Stale`.

A drum fill written in the chart that starts at or after the chart's first coda event never lands, as in Clone Hero. A skipped written fill also stops the generated fills.

Five-lane drum charts convert to four lanes the way Clone Hero converts them.

A drum chart with neither `five_lane_drums` nor `pro_drums` set reads every pad but the kick as a tom, as Clone Hero does. Those two keys read yes or no by the game's own rule, which differs by file type: in `song.ini`, any nonzero whole number or `true` means yes, while a `.sng` file accepts only `true` or `false`. So `pro_drums = 1` keeps cymbals in a `song.ini` and counts as unset in a `.sng`.

On a double drum lane, the second pad always comes from a later chord. A `.mid` section text holding `_ENDOFSONG` no longer makes a section.

## Library

The search box reads `Search song, artist, album, charter, or folder` while empty. The line under the table with `Clear search` and the `Matched on` note is gone; the `X` beside the box clears the search.

`squeeze<20` now leaves out a song at exactly 20 ms. Use `squeeze<=20` to keep it. Before, `<` meant the same as `<=`.

A song with no charter reads `(unknown)`, as a song with no artist does.

If a song folder can't be read, `Scan library` scans nothing and keeps your songs and results. The line under the toolbar names the folder and says to click `Scan library` when it is back. A folder listed twice in different spellings, such as different capital letters or slash directions, is scanned once, so its charts no longer show twice. A scan that hits an error lists it with the scan's problems instead of closing Hydra Deluxe.

While a batch runs, rows keep their order under the `Best path` sort and re-sort once when it ends. The song you clicked keeps its place when the song panel opens.

The start box reads `Opening your library...`, then `Reading your charts...` and `Reading your results...`, each with a bar.

## Path report and dmleaderboards

The path report window keeps its report when you close it. Reopening it shows the same report at once, with your search, timing choice, `Optimal paths only` and sort kept. The `Timing` column sorts in tier order, from `Normal` to `No timing needed`.

`Compare with dmleaderboards...` now works at any `SP cap`. It always compares against the 4-bar results, and its footer says to set `SP cap` to 4 and analyze first if you haven't. A score at another speed for a chart that isn't in your library reads `Not in your library`.

## Preview tab

The beat lines stop where the song ends instead of running to the end of the audio.

## Settings

**A `hydra_settings.ini` with an error is no longer read halfway.** A value Hydra Deluxe can't read, an unknown key or a line with no `=` counts as an error. Hydra Deluxe then runs on its default settings, saves nothing, and shows the file's path and the bad line in orange. Scans, analysis and the path report stay off until you fix the file and restart. No setting's default changed.

A number outside its range lands on the nearest edge. For `sp_cap`, a value of `0` now loads as 1, where it used to load as 4.

The `hit_window_ms` setting is gone. It had no control on screen. The hit window is a fixed 85 ms, and an old `hit_window_ms` line in the file is skipped and dropped on the next save.
