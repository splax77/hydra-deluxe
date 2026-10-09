# Hydra Deluxe User Guide

This guide walks through Hydra Deluxe's screen in the order you meet it. First the toolbar, then the analysis settings, then the library and its search. After that comes the song panel with its four tabs, then analyzing the whole library, and last the reports.

It covers Hydra Deluxe. The public Hydra by DragonDelgar has a different screen; see [how the two differ](differences-from-public-hydra.md). For what the path notation and squeezes mean, see the [public Hydra wiki](https://github.com/DragonDelgar/hydra/wiki).

## The main screen

Hydra Deluxe has one main screen. The toolbar runs along the top. The **Analysis settings** bar sits under it. The library fills the rest of the window.

Click a song and the **song panel** opens beside the library. Drag the library's right edge to give either side more room. Hydra Deluxe remembers the width.

When something goes wrong, a message appears under the toolbar. A plain notice fades by itself. A problem stays in orange until you dismiss it with its `X`.

### The first start after updating

Your library and results live in a file called `hydra.db`, next to the program. A new version sometimes stores less in that file than an older one did. Then the first start after updating rewrites the file in the new layout. This happens once.

A quick update shows nothing: the window opens and the library appears. If it takes a moment, a box in the middle of the window says:

> Updating your library file for this version of Hydra
>
> This happens once. Your charts and results are kept.

Under that, a line says what Hydra Deluxe is doing. `Copying your library...` comes with a bar that counts the rows copied, like `12,345 / 38,009`. `Finishing...` shows with the bar full while the new file takes the old one's place. Coming from a 1.8.x version, the line reads `Updating the results table...` before the copy starts. On a slow disk, an estimate of the time left appears after a few seconds, the same way the Preview's does. When the box goes away, your library is there as before.

On any start, if opening your library takes longer than usual, for example while a drive wakes from sleep, the window shows `Opening your library...` until the library appears.

You can close the window during the update. Hydra Deluxe stops, removes the half-made new file and leaves `hydra.db` exactly as it was. The next start runs the update again from the beginning.

If the update fails, a message box shows this sentence, with the error from Windows or the database under it:

> Hydra couldn't update its library file (hydra.db) for this version. Your charts and results were not changed. Check that no other copy of Hydra or hydra_batch is running and that the disk isn't full, then start Hydra again.

Hydra Deluxe closes when you dismiss the box. Nothing in `hydra.db` has changed. Close any other copy of Hydra Deluxe or `hydra_batch`, free some disk space if the drive is full, and start Hydra Deluxe again.

### Toolbar

**`Manage folders... (N)`** opens the **Song folders** window. N is how many folders you have. Add your Clone Hero song folder (or any folder of charts) with `Add folder...`. Hydra Deluxe finds every chart in its subfolders. Each folder has a red `X` to remove it, with a confirmation, because removing a folder changes what the next scan finds.

Hydra Deluxe reads `notes.mid`/`notes.chart` + `song.ini` folders, `.sng` archives, and the `.srb` bundles Clone Hero's built-in setlist ships with. Adding the game's own `Clone Hero_Data\StreamingAssets\songs` folder brings in the default songs too.

**`Scan library`** reads your folders and updates the song list. A song needs a valid ini file to show up. A song that fails is skipped, and the scan window lists the problems when it finishes. Re-scans are fast: a chart whose file size and timestamp haven't changed is not read again. You can cancel a scan part-way; the old library stays as it was.

Songs stay in the list until the next scan, even if you changed their files. After you add, remove or edit songs, scan again.

**`Analyze library...`** analyzes many songs at once, in the background. While a search narrows the list, the button reads **`Analyze search (N)...`** and only analyzes the N songs your search shows. A filter Hydra Deluxe can't read narrows nothing, so with only `stars:9` typed the button still reads `Analyze library...`. See [Analyzing the whole library](#analyzing-the-whole-library).

**`Compare with dmleaderboards...`** compares a player's leaderboard scores with your stored results. See [Reports](#reports).

**`Open path report`** opens the path report in its own window. It appears once your library has analyzed songs, and it stays while a report is open or building. While a batch's report builds, it reads **`Building path report...`**. You can still click it, and the window shows the build's progress. See [Reports](#reports).

## Analysis settings

The settings bar holds every setting that shapes an analysis. Each one applies to every song, not just the one you have open. A result is saved together with the settings it ran under. Change a setting and the library shows the results for the new settings. Change it back and the old results come back, with no batch to run. A song open in the panel is analyzed again under the new settings, the same as clicking it.

While a batch runs, the bar is locked and says `Stop the batch to change these.` Opening a song never locks it. Changing a setting while the song is still analyzing starts it again under the new settings.

**Difficulty.** Which charted difficulty to analyze, path and preview: Expert, Hard, Medium or Easy.

**Pro Drums.** Analyze with cymbals and toms as separate notes, the way Clone Hero scores Pro Drums.

**2x Bass.** Include the chart's 2x kicks, like Clone Hero's Double Kick modifier. It works at every difficulty. Each difficulty has its own 2x kicks, though few charts have any below Expert.

For example, say you have a song open with 2x Bass on and want to see it with 1x bass. Untick 2x Bass and the song is analyzed again without its 2x kicks. Tick it again and the 2x result comes back.

**SP cap.** The most bars of Star Power the meter can hold during the analysis. 4 is Clone Hero's rule and the default. Leave it at 4 for paths you mean to play. Any other number is a what-if: its scores can't be reached in the game. At 1 bar no path can activate Star Power, and the Paths tab says so. Results are kept per cap, so a 4-bar result and a 16-bar result for the same song sit side by side. The leaderboard comparison only runs at 4 bars.

**1.0 fills.** Spawn drum fills by Clone Hero 1.0's rule instead of 1.1's. A fill only appears if your Star Power was ready in time. Clone Hero 1.1 wants it ready 4 beats before the fill. 1.0 wanted it about one fill-length before, so short fills were looser and long fills stricter. Leave it off for current Clone Hero. Tick it to price a run played on 1.0. Results are kept per rule, so a song can hold a 1.1 result and a 1.0 result side by side. The leaderboard comparison needs it off. Everywhere else, Hydra Deluxe names the two rules `Clone Hero 1.0` and `Clone Hero 1.1`, or `CH 1.0` and `CH 1.1` in narrow columns.

**Score range.** How many extra paths below optimal to keep. The dropdown picks the unit. `scores` keeps the next few distinct scores under optimal. `points` keeps every path within that many points of optimal. These paths are a little worse, but handy when the optimal path is awkward to play. More paths make the analysis slower.

**Path limit.** When ticked, an extra path is kept only when its hardest squeeze or required early fill is within this many milliseconds. That keeps alternates you can realistically hit. A path that ties the best score is kept even when its timing is over the limit, and its figure shows in orange; below the best score, a path over the limit is dropped. It starts on<!-- default: Settings::mslimit_enabled --> at 10 ms<!-- default: Settings::mslimit_value -->. Lower or negative values demand more slack. The limit compares raw milliseconds: for a squeeze, the gap at the SP end; for an early fill, the gap at the fill deadline. It does not account for frontend timing scaling (see the Paths tab), so a kept path can be a bit harder than its number suggests where an activation's scale line is orange.

## Searching the library

The library lists every song from the latest scan. Its columns are **Title**, **Artist**, **Charter**, **Folder** and **Best path**. Click a column heading to sort by it. Columns can be resized and hidden, and Hydra Deluxe remembers them, the sort included. The columns always stay in this order. While the song panel is open, the Charter and Folder columns step aside to save room, and the folder shows under the title. Song names show without Clone Hero's colour and style tags, here and in the report windows. A name that is nothing but tags reads `(unknown)`.

The **Best path** cell shows the song's state under the current analysis settings:

- `Not analyzed` means there is no result yet.
- A score and a path, like `378,315  3- 1 2`, is the optimal path.
- `Stale` means the result is out of date. Either another version of the app made it, or it was made under different rules in `hydra_rules.ini`. Click the song or run a batch to refresh it. If the cause was a rules edit, switching the rules back brings the result back.

Hover the cell for the same explanation.

The chips above the table filter by state: `All`, `Not analyzed`, `Stale` and `Analyzed`, each with its count. A chip with a count of 0 is greyed out.

### The search box

Type in the search box to narrow the list. `Ctrl+F` jumps to it. `Escape` or its `X` clears it. The heading above shows how many songs match, like `Library 5 of 97 charts`.

Searching ignores case and accents, so `ALLISTER` finds Allister and `beyonce` finds Beyoncé. Colour and style tags in song names are ignored too. The examples below come from Hydra Deluxe's test library.

| You type | What it matches | Example |
|---|---|---|
| words | songs whose title, artist, charter or folder contain every word, in any order | `day green` finds Burnout by Green Day |
| `"quoted text"` | that exact phrase | `"dance gavin dance"` finds the band's 2 songs |
| `artist:` | the word or phrase in the artist only | `artist:rush` finds YYZ |
| `charter:` | the charter only | `charter:onyxite` finds 8 songs |
| `folder:` | the folder only | `folder:"tier 4"` finds the 5 songs in Tier 4 |
| `title:` | the title only | `title:everlong` finds Everlong |
| `stars:N` | songs whose optimal path earns exactly N stars (0 to 7) | `stars:7` finds Burnout once it is analyzed |
| `squeeze<=N` | songs whose optimal path's hardest squeeze or required early fill is N ms or less | `squeeze<=200` keeps Burnout (163 ms); `squeeze<=20` drops it |

You can mix them: `charter:hoph2o stars:7`. `squeeze<N` means the same as `squeeze<=N`: at most N. A path that needs no timing passes any squeeze limit.

`stars:` and `squeeze<=` only look at current results. A song that is not analyzed, or whose result is Stale, never matches them. A filter Hydra Deluxe can't read, like `stars:9`, shows a short note under the search box and is left out of the search.

Under the table, a search shows a `Clear search` button. When a match came from a column that is hidden, a note there says so, like `Matched on folder.`

## The song panel

Click a song to open its panel beside the library. The top shows the title, the artist and the charter. The `<` and `>` buttons step to the previous and next song in the list. The `X` button, or `Escape`, closes the panel.

**`Hide library`**, left of `<`, gives the song panel the whole window. It then reads `Show library`, which puts the library back at the width you left it. `<` and `>` still step through the list while it's hidden. Closing the panel always brings the library back. Hydra Deluxe remembers the choice, so the next song opens the same way, even after a restart.

Opening a song analyzes it under the current settings. Most songs take a few milliseconds, so the paths simply appear. A song that takes longer than 0.15 seconds shows `Analyzing chart...` with a progress bar and a `Cancel` button. Cancelling shows `Analysis cancelled.` with a `Try again` button, and nothing is saved for that song. Closing the panel or clicking another song cancels it the same way.

When the analysis finishes, the song's result is saved to the library if it was missing, Stale or different. So the library row and the panel always show the same numbers.

Below the header is the headline: the best score, its path, and one line like `Optimal path · 7 stars`.

If the song's file has moved or been deleted since the last scan, the panel says `Song file not found.` and offers a `Rescan library` button. Its library row and result stay. If the chart was edited since the last scan, opening it reads the edited file, and the library row follows.

The panel has four tabs: **Paths**, **Preview**, **Dynamics** and **Stars**.

## Paths tab

The left side lists the paths the analysis kept, grouped by score. Each score is a fold that shows the score and how many paths share it, like `378,315  1 path` on Burnout; click it to hide or show those paths. **Optimal** comes first. When several paths tie for the top score, all of them are optimal, because playing any of them earns the same score. Then come the extra paths under a heading like **Within 2 scores**. Last is **Best all-0 path**: the best path that activates at the first chance every time, with no skips. An early fill it lets pass reads **E1** and still counts as no skip: if you don't get that fill, the next one is your first chance anyway. Every squeeze and early fill on it is 0 ms or easier. It also shows how far it falls below optimal. Click a path to show it on the right.

Each path shows its own hardest timing right after it, like `3- 1 2   163.0 ms` under the `378,315` fold: the hardest squeeze or required early fill that path needs. It turns orange past the difficult limit. A path that needs no timing shows nothing there.

The right side starts with a summary, like `Activations 3 · no SP left over`. Under it, a timeline runs from the first measure to the end of the song, with a mark for each activation. The song's length is the one the chart's own files state (the `song_length` line in song.ini, the same key in a .sng, or the length stored in a .srb). A chart that states none ends at its last Expert drum note. Hydra Deluxe never opens the audio to find it, so a chart with no audio still gets a timeline. A song with a long outro ends its last mark well before the right edge. A mark is outlined in orange only when its row's timing is orange too. A chart with no length at all, meaning no stated length and no Expert drum notes, shows no marks.

Each activation is one row. It shows the activation's number, its notation, its measure (like `m32.1.0`), the bars of SP banked when you activate (like `1 bar` or `2 bars`), and a badge for its hardest timing, like `squeeze out 163 ms` or `early fill 20 ms`. An activation that skips fills keeps its early fill badge when its Star Power is ready no earlier than the fill's deadline, because that timing decides whether the first fill shows up. An early fill with time to spare has nothing to time, so it gets no badge. A squeeze that happens on its own still gets one when nothing else on the row needs timing, with a figure of 0 or below: `squeeze in -316 ms` means the phrase's last note already lands 316 ms before Star Power ends. Click a row to open it, or use `Expand all` and `Collapse all`.

An open row shows:

- **Chord**: the chord you activate on. On a multi-note chord, do a frontend squeeze: hit the activation note first, so the other notes score with the Star Power multiplier.
- **`Show in Preview >`**: jumps the Preview tab to this activation.
- The early fill timing, when the activation has one (the `E` notation), like `Early fill: 0.0 ms (required)`. A positive number means you must hit that many ms early to make the fill show up. A negative number is slack, so the fill shows up with room to spare.
- A plain sentence for each squeeze: which note to hit early or late, by how much, and what it's worth. Notes are named the way the Dynamics tab names them, like `Green tom` or `Yellow cymbal`.
- **Backend timings**: the notes near the end of Star Power, with a short lead-in above the table.

A backend squeeze means hitting the note at the SP end early, so it lands inside Star Power. The note at `0.0 ms` is exactly where SP ends; the others are the notes just before and after it. If the last activation's SP runs past the end of the chart, the table lists the notes before that end, so every timing is negative. The rating beside each note is a rough guide to how hard it is to fit into Star Power. A note the path doesn't count is tagged `(uncounted)`, squeeze-out notes included, like `Free SqOut (uncounted)`. For double backend squeezes, look for notes from the backend leeway up to the hit window. The leeway is 3 ms<!-- default: Rules::backend_leeway_ms --> by default (`backend_leeway_ms` in `hydra_rules.ini`). The hit window is 85 ms by default (`hit_window_ms`, below).

Star Power length is measured in measures, not milliseconds. If the SP end falls where measures last a different time than at the activation (a tempo or time signature change), frontend timing only partly carries to the SP end. Hitting the activation 50 ms late might move the SP end only 25 ms. Whenever the scale isn't x1.00, the opened row shows it, early first: `Frontend timing scales x0.99 (early) / x4.45 (late) at the SP end.` Late and early hits can scale differently when the activation or the SP end sits right on a change. A side that is x1.00 is left out. If a SqIn moved the SP end, that earlier end gets its own clause. The line is orange when the scale changes a squeeze or backend figure, and those backend rows show an effective timing (`eff.`). Otherwise it is gray.

Sometimes a phrase collected during Star Power would push the meter past the SP cap, so the cap cuts the SP end short. A phrase that only brings the meter exactly to the cap doesn't count. When the cap cuts the end short, the note that filled the meter, not the activation, is now the one whose timing moves the SP end. The scale line and the eff. figures are measured from that note. The row shows an overfill warning, like `SP overfilled at m40.1.0`, when that matters: when the row lists a squeeze, or a backend note the path squeezes out or doesn't count.

Below the activations:

- **`Multiplier squeeze`** lists the multiplier squeezes in the song. When the combo multiplier goes up on a multi-note chord, hitting the more valuable notes (cymbals, dynamics) a little later scores them on the higher multiplier. It's why two FCs of the same song can differ by 15 points. `2x` means it happens as the multiplier reaches 2x (10 notes in), and so on.
- **`Score breakdown`** shows the score this path should get, in the same categories as Clone Hero's results screen. It includes multiplier, frontend and backend squeezes. A double squeeze only counts when the backend note lands within a small leeway past the SP end, 3 ms<!-- default: Rules::backend_leeway_ms --> by default (`backend_leeway_ms` in `hydra_rules.ini`).
- **`Copy path`** copies the selected path's notation. `Ctrl+C` does the same and flashes `Copied!`.
- **`Hide backend rows beyond`** hides backend rows more than this many ms from the SP end. It starts off<!-- default: Settings::backendlimit_enabled -->, with 50 ms<!-- default: Settings::backendlimit_value --> in the box. A note the path squeezes out always shows. This only changes the display. It is not an analysis setting, so changing it never re-analyzes anything.

One more setting feeds these displays: `hit_window_ms` in `hydra_settings.ini` (default 85, the Clone Hero Pro Drums hit window per side). It has no control on screen yet.

## Preview tab

The Preview plays the chart as a 3D note highway, in time with the song's audio. It draws the chosen path on the highway: the Star Power windows as a teal floor, and the fills a player following the path would see.

Where Star Power runs out, a bright teal line crosses the floor and a small teal triangle sits just outside each railing, level with the line. A note just before the end can cover the line, but the triangles stay in view, so you can always see exactly where SP stops.

The Preview follows the Analysis settings. Change the difficulty, Pro Drums or 2x Bass and it reloads with that mode's notes. If the chart has no notes for that mode, it says so, like `No Hard Pro Drums notes in this chart.` If the chart file changed since it was analyzed, the Preview draws no path and shows `This chart changed since it was analyzed. Click the song again to see its path.`

**`Showing`** picks which path to draw. It lists the same paths as the Paths tab, in the same order. The all-0 path reads like `0 0 0 0  (best all-0)`.

**`< Act`** and **`Act >`** jump to the previous and next activation. The `[` and `]` keys do the same. `Show in Preview >` on the Paths tab lands here too. A jump lands on the activation chord with it already hit: the gem shows struck, and the score box counts it.

The transport buttons are `-5s`, `< 5 Ticks`, `Play`/`Pause`, `5 Ticks >` and `+5s`, with a volume slider beside them. The keys:

- `Space` plays and pauses.
- `Left` and `Right` arrows jump 5 seconds.
- `,` (comma) and `.` (period) step 5 ticks.
- `[` and `]` jump between activations.

The same keys are drawn as keycaps in a bar under the highway, in the buttons' order. Each button's tooltip names its key too.

The scrubber under the highway shows where you are. Its gold marks are the path's activations, in the same gold as the next-activation box. The scrubber measures position against the song's length, like the Paths timeline, so an activation sits at the same spot on both. A chart with no length still scrubs, up to where playback ends, but the Paths timeline shows no marks for it, so there is nothing to line up. Audio that runs past the song's length still plays.<!-- owners: scrub_end_ms (the scrubber's end), app::song_length_ms (the song's length) --> The beat lines run on to the end of the audio.

The boxes on the highway:

- **Time box** (top left): the measure you're at and the measure where playback ends, the tempo and time signature, and the practice section when the chart has them. Playback ends at the end of the audio (or at the last note, when the audio stops first), and the clock beside the scrubber counts to that same point.
- **Score box** (under it): the running score in large type, then the multiplier and combo, like `x4 · combo 212`. It appears once the song is analyzed. It reads `Score unavailable` when the path can't be replayed to its stored score.
- **Next activation** (bottom left): the next activation's number, where it is, and its chord.
- **SP meter** (right edge): a gauge of banked Star Power, one line per bar, with the bars shown under it. It rises one bar at each phrase you collect and drains through each activation, reaching empty exactly where the path's SP ends. It holds as many bars as the result's SP cap. Before the song is analyzed, it uses the SP cap from the Analysis settings, the one the next analysis will run at.
- **Drain box** (top right, beside the meter): how long one bar of SP lasts at the current tempo, like `1 bar / 2.5 s`. While SP is running on the path it reads `SP drain` and `empties in` the time left, in teal like the floor. Otherwise it reads `SP drain (if activated)` and `full meter`: how long a full meter would last at the current tempo.

## Dynamics tab

This tab counts the chart's ghost and accent notes. Ghosts and accents are the soft and hard hits that score double in Clone Hero.

The **Pads** table shows, for each pad (and each cymbal separately under Pro Drums), how many notes are ghosts, accents and normal hits. **Kicks** does the same for kicks, with 2x kicks on their own row and a line saying how many kick notes are 2x. Percentages round to the nearest whole number. When 2x Bass is off, the 2x kick row stays visible but greyed out, and both All kicks and the totals leave it out. **Totals** adds them up.

The **Chart** box says whether the chart has dynamics turned on. A MIDI chart has to opt in with a tag. Without that tag Clone Hero ignores the velocity markings and plays every note as a normal hit. Hydra Deluxe reads the chart the same way, so the tab shows no ghost or accent counts. It says `This chart has no ghost or accent notes.`, and the Chart box says `Dynamics enabled: no (markings ignored by Clone Hero)`. When the tag only comes partway through the chart, the markings before it are ignored too, and the Chart box says how many there were.

Counts are worked out when you open the song, together with its paths. Nothing is saved, so they always match the chart file as it is now.

## Stars tab

This tab shows the score you need for each star.

**Base score** is every note hit once at 1x, with no Star Power. **Solo bonus** is the chart's full solo bonus. The solo bonus does not count toward stars: Clone Hero compares your score without it.

The table lists stars 1 to 7. **Multiplier** is what the base score is multiplied by (0.1, 0.5, 1.0, 2.0, 2.8, 3.6 and 4.4). **Cutoff** is the score that earns the star, rounded up. **With full solo bonus** is the cutoff plus the whole solo bonus: what the results screen shows at that cutoff if you also collect every solo bonus.

A star counts once your score, without the solo bonus, reaches its cutoff.

## Analyzing the whole library

`Analyze library...` analyzes every song in the library. While a search narrows the list, `Analyze search (N)...` analyzes only the N songs the search shows.

First a window titled **Analyze library** asks you to confirm. It says how many songs it will analyze and lists the settings it will use. To change those, cancel and edit the Analysis settings. Songs that already have a result are skipped, unless you tick `Also re-analyze charts that already have a result`. Click `Start analyzing` to begin, or `Cancel`.

The batch runs in the background, using all but one of your CPU cores. You can keep browsing and open songs while it runs. A strip under the toolbar shows its progress: how many songs are done, the song being analyzed, the time so far and, after a few songs, an estimate of the time left, like `about 1:30 left`. An open song's panel already shows the engine's result, so the batch only updates its library row.

- **`Pause`** holds the batch. **`Resume`** carries on.
- **`Stop`** ends it. Every result finished so far is kept.

The Analysis settings stay locked until the batch ends.

When it finishes, the strip changes to a summary: how many songs were analyzed, failed and skipped, and how long it took. Songs that failed are listed with the reason. A stopped batch keeps its results but builds no report.

## Reports

### The path report

Hydra Deluxe has two reports: the path report and the dmleaderboards comparison. Each one opens in its own window. It's a real Windows window with its own taskbar button, so you can drag it to another monitor and keep it open while you use the rest of Hydra Deluxe. It minimizes and restores with Hydra Deluxe and sits in front of the main window. Both windows can be open at once.

The first time a report window opens, it covers the main window, at the same size and place. After that it opens where you last left it. If that spot is on a monitor that's no longer connected, the window opens over the main window again. The placement is saved in `hydra_ui.ini`, next to the program.

### The path report

The **path report** is one table of every analyzed song's paths, squeeze timings and scores. It lists every chart mode (each difficulty, with or without Pro Drums and 2x Bass) analyzed at the current SP cap, fill rule, path limit and score range. Each chart and mode shows its top 5 paths.

A finished batch builds it. `Open path report` on the toolbar opens it at any time. Hydra Deluxe keeps the report in memory, not in a file. So the first time you open it after starting Hydra Deluxe, it builds a fresh one. Closing the window lets the report go, to save memory, and opening it again builds a fresh one too. The window keeps your search, timing choice, `Best path only` and sort for it. `Refresh` and a batch's new report start them over. If a build is still running when you close the window, it stops. A report a batch builds while the window is closed waits for you to open it.

Hydra Deluxe keeps only each song's summary, so building the report analyzes those songs again, on all cores. Songs a batch just analyzed are reused. On a large library that takes a few seconds. While it builds, the window shows a bar reading `Analyzing n of N records` and a `Cancel` button. A record is one song in one mode, so a song analyzed in two modes counts twice. When a batch already analyzed everything, the bar just moves, with no count. While `hydra_rules.ini` has an error, analysis is off. A window with no report to show then says so instead of building. Cancelling shows `Report cancelled.` with `Try again`. If the build fails, the window says `The path report could not be built.`, with the reason and `Try again`.

The window is titled `Path report — Hydra`. From top to bottom it has:

- **The header.** `Hydra Path Index`, a line saying what the report covers, `Built HH:MM` (when it was built), and `Refresh`, which builds it again.
- **Strips,** only when one applies. A song whose file can't be read any more is left out of the report. A strip then says `Left out: N charts whose file couldn't be read.`, and `Show files` lists their file paths. Another strip says when the report is out of date (below).
- **Tiles.** Five numbers that sum up the rows that pass the filters, like `Charts shown` and `Paths shown`. The **Hardest ms** tile is the hardest squeeze or required early fill any of those paths needs. The tiles change as you filter.
- **The controls.** A search box, a timing dropdown (`All timing tiers`, or one tier) and `Best path only`. On the right, a count line says how many paths pass, like `120 of 4,512 paths`.
- **The table.** One row per path. Hover a column heading to see what the column means.
- **The footer.** A dim note under the table.

`Best path only` starts ticked. It keeps just the optimal paths. When paths tie for the top score, it keeps every one of them, because each is optimal.

The search box ignores case and accents. Every word you type must appear somewhere in the song, artist, charter or path the row shows, and the words can match different fields. Quotes and the Library's field prefixes, like `artist:`, are ordinary words here. When no row passes the filters, the window says `Nothing matches those filters.` `Clear filters` then empties the search and sets the dropdown back to all. It leaves `Best path only` as it is.

The table starts sorted by score, highest first. Click a column heading to sort by it, and click it again to flip the order. Shift+click another heading to add a second sort. Empty values always sink to the bottom. Text columns sort the way the Library does. Columns can be resized and hidden, and Hydra Deluxe remembers their widths.

Click a row to select that song in Hydra Deluxe, the same as clicking it in the library. If the row is for another chart mode, say Hard instead of Expert, the settings bar switches to that mode first. That way the Paths tab shows the same path as the row. While a batch runs, the settings bar is locked. A row of another mode then selects nothing, and the status line says `A batch is running.`

If nothing is analyzed under the current settings but other results exist, the report says so and names the settings it looked under, like `Nothing is analyzed under these settings (SP cap 8, Clone Hero 1.1 fills).` It then suggests analyzing with these settings or changing them. Only an empty database says no records are stored yet.

**When the report is out of date.** The window never changes its rows behind your back. After a batch, a report built before it shows a strip: `Your library changed since this report was built (a batch finished at HH:MM).` Changing a setting the report depends on shows `The settings changed since this report was built.` For the path report those settings are the SP cap, `1.0 fills`, the score range and the path limit. Switching the difficulty, Pro Drums or 2x Bass doesn't count, because the report already lists every chart mode. The old rows stay readable under the strip. `Refresh` builds the report again and clears it.

**Keyboard.** `Tab` moves through the controls and the table. The `Up` and `Down` arrows move the row selection, and each move selects the song, like a click. `Escape` or `Ctrl+W` closes the window, and so does its `X`. While you're typing in the search box, `Escape` doesn't close it. Closing the path report lets it go, as described under the path report above. Closing the comparison keeps it in memory, so it opens again at once.

The finished batch strip offers:

- **`Open report`** opens the path report window. The strip's second line reads `The path report is ready.` Once you close the path report window, the report is gone, so the line, the button and the `Open automatically` box go too. The toolbar's `Open path report` still opens it.
- **`Open automatically`**: Open the path report as soon as it's built. It covers only the path report.
- Its `X` dismisses the strip.

### Old report files

Older versions saved the reports as web pages, `hydra_paths.html` and `hydra_dmcompare.html`, in your **Documents\Hydra** folder. Hydra Deluxe no longer reads or writes them. It leaves any old ones where they are, because deleting your files needs your say. Delete them yourself if you don't want them.

### Compare with dmleaderboards

`Compare with dmleaderboards...` compares a [dmleaderboards.com](https://dmleaderboards.com) player's posted scores with your stored optimals. Pick a player from the searchable list in the **Compare dmleaderboards user** box; Hydra Deluxe remembers the last pick. Picking a player closes the box and opens the comparison in its own window, titled `dmleaderboards: <player> — Hydra`. Hydra Deluxe fetches the player's scores and matches them to your analyzed songs by chart hash.

The comparison window works like the path report window. It has the same header, strips, tiles, controls, table, keyboard and placement. Its header reads `Hydra vs dmleaderboards` and also has `Compare another player...`, which reopens the box. There is one comparison window, so comparing another player replaces what it shows.

While the scores load, the window says `Fetching scores and building the report...` and `The leaderboard server can take a moment to wake up.`, over a moving bar. The server gives no count, so the bar only shows that work is happening. `Cancel` closes the window and reopens the player list. `Refresh` fetches the player's scores again and rebuilds the comparison.

The table lists each score, Hydra Deluxe's optimal, the points left, and a status per row. It starts sorted by points left, most first. A score played at normal speed that Hydra Deluxe has a result for reads `Under optimal`, `At optimal` or `Above optimal`. A score played at any other speed reads Other speed. The rest read Not analyzed or Not in your library. The status dropdown shows one status at a time, or `All charts`.

Clicking a row selects its song, as in the path report. A row whose status is `not in library` has no song to select, so clicking it does nothing, and hovering it says `Not in your library`.

The header line gives the player's total number of scores. The tiles count only the rows that pass the filters: `Scores shown`, then the scores under, at and above optimal, the ones for songs you haven't analyzed, and the ones for songs not in your library. Scores played at a speed other than 100% get their own status, Other speed. Clone Hero keeps a separate leaderboard for each speed, and Hydra Deluxe's optimal is for normal speed, so those rows show Hydra's numbers but aren't compared with its optimal.

Rows above optimal are expected, not errors. Hydra Deluxe's optimal leaves out several score backends on purpose. Many leaderboard scores were also set on older Clone Hero versions, whose fill rules allowed totals that are impossible now.

The comparison needs Expert, an SP cap of 4 and `1.0 fills` off, because the leaderboard only holds Expert scores played under Clone Hero's rules. Only songs analyzed under the current settings can match, so analyze your library first for a full comparison.

The comparison goes out of date the same way the path report does, with the same strips. A batch marks it. So does a change to the difficulty, Pro Drums, 2x Bass, `1.0 fills`, the score range or the path limit, because those decide which of your results it compared against.

The first request after a while can take tens of seconds, because the leaderboard's server has to wake up.

## Scoring rules (`hydra_rules.ini`)

A few of Hydra Deluxe's rules are judgment calls, not facts read from Clone Hero. You can change them in `hydra_rules.ini`, a plain text file next to Hydra.exe. The app and every command line tool read the same file.

The file is optional. A missing file, or a missing line, means the default below. Each line is `key = value`. A line starting with `#` is a comment. There are no `[section]` headers.

```ini
# Hydra Deluxe's defaults
backend_leeway_ms = 3.0
sqout_rule = first_note
max_tied_paths = 4
fill_cooldown_measures = 4
fill_max_distance_beats = 0.5
fill_length_measures = 0.5
fill_land_slop_beats = 0.03125
```

What each line does:

- **`backend_leeway_ms`** (default 3.0 ms<!-- default: Rules::backend_leeway_ms -->, must be 0 or more): a backend note that lands less than this far past the Star Power end still counts in the score. Exactly this far past it does not.
- **`sqout_rule`** (default `first_note`; `first_note` and `whole_chord` are the only values): what a squeeze-out costs. `first_note` removes the Star Power doubling from one note of the chord (the lowest-value one). `whole_chord` removes it from every note in the chord.
- **`max_tied_paths`** (default `4`, must be 1 or more): how many paths Hydra Deluxe keeps at one score: one count per score, whichever side of the Path limit each path falls on; paths inside the limit come first. More paths means longer lists and slower analysis.
- **`fill_cooldown_measures`** (default `4`, must be 1 or more): for charts with no authored fills, how many measures must pass after an activation point before Hydra Deluxe places the next one.
- **`fill_max_distance_beats`** (default `0.5`, must be 0 or more): for charts with no authored fills, how far from a measure line a note can sit and still get a fill.
- **`fill_length_measures`** (default `0.5`, must be above 0): for charts with no authored fills, how long each fill Hydra Deluxe places is, in measures.
- **`fill_land_slop_beats`** (default `0.03125`, a 32nd of a beat, must be 0 or more): how close after a fill's end a note must be for the fill to land on it. Hydra Deluxe adds one tick to this, as Clone Hero does, so even `0` lets a note one tick late take the fill. It applies only to fills written in the chart.

Older files may still have `auto_cap_ladder` or `auto_budget_s` lines. Hydra Deluxe reads and ignores them, so those files keep working.

A value Hydra Deluxe can't read, or a key it doesn't know, is an error that names the key. The command line tools print the error and stop with exit code 2. The app still opens and shows the error, but analysis stays off until you fix the file and restart Hydra Deluxe. Hydra Deluxe never analyzes on the defaults behind your back.

Every result remembers the rules it was made with. After you change the file, results made under the old rules show `Stale` until you click the song or run a batch. Switching the rules back brings those results back.

## For power users: command line tools

You never need these; everything they do is in the app. Two console programs sit next to Hydra.exe and share its settings and library. `hydra_batch` analyzes the library, and `hydra_fillcompare` compares Clone Hero 1.0 and 1.1 fill results. The path report lives only in the app. The [developer page](development.md#command-line-tools) lists their flags.
