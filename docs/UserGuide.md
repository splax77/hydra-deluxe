# Hydra Deluxe User Guide

This guide walks through Hydra Deluxe's screen in the order you meet it: the toolbar, the analysis settings, the library and its search, the song panel with its four tabs, analyzing the whole library, and last the reports.

It covers Hydra Deluxe only. The public Hydra by DragonDelgar has a different screen; see [how the two differ](differences-from-public-hydra.md). For what the path notation and squeezes mean, see the [public Hydra wiki](https://github.com/DragonDelgar/hydra/wiki).

## The main screen

Hydra Deluxe has one main screen. The toolbar runs along the top, and the library fills the rest of the window. The toolbar's `Analysis settings` button opens the settings panel.

Click a song and the song panel opens beside the library. Drag the library's right edge to give either side more room. Hydra Deluxe remembers the width.

When something happens that you should know about, a message appears under the toolbar. A plain notice fades by itself. A problem stays, in orange, until you dismiss it with its `X`.

### The first start after updating

Your library and results live in a file called `hydra.db`, next to the program. A new version sometimes stores its results in a new layout. When it does, the first start after updating converts the file. This happens once.

A quick update shows nothing: the window opens and the library appears. If it takes a moment, a box in the middle of the window says:

> Updating your library file for this version of Hydra
>
> This happens once. Your charts and results are kept.

Under that, `Updating the results table...` comes with a bar that counts the results converted, like `12,345 / 38,009`. Then `Finishing...` shows while Hydra Deluxe reads your library. When the box goes away, your library is there as before. A result made by an older version reads `Stale` when the new version analyzes charts differently. Click the song or run a batch to refresh it.

On any start, if opening your library takes longer than usual, for example while a drive wakes from sleep, the window shows `Opening your library...` until the library appears.

You can close the window during the update. Hydra Deluxe stops and leaves `hydra.db` exactly as it was. The next start runs the update again from the beginning.

If the update fails, a message box shows this sentence, with the error from Windows or the database under it:

> Hydra couldn't update its library file (hydra.db) for this version. Your charts and results were not changed. Check that no other copy of Hydra or hydra_batch is running and that the disk isn't full, then start Hydra again.

Hydra Deluxe closes when you dismiss the box. Nothing in `hydra.db` has changed. Close any other copy of Hydra Deluxe or `hydra_batch`, free some disk space if the drive is full, and start Hydra Deluxe again.

A library file from a version older than 2.2.0 is too old to convert. Hydra Deluxe renames it to `hydra.db.pre-2.2.0`, replacing an earlier file of that name, and starts a fresh library. A box says:

> This library file is from a Hydra version older than 2.2.0, so Hydra set it aside as hydra.db.pre-2.2.0 and started a fresh library.

When you close the box, Hydra Deluxe scans your song folders, which it keeps in `hydra_settings.ini`, next to the program. Your songs come back as `Not analyzed`. This version doesn't read the old results in `hydra.db.pre-2.2.0`, so delete that file yourself if you don't want it.

If the old file can't be renamed, a message box says so, with the reason from Windows in the middle of the sentence, and Hydra Deluxe closes when you dismiss it:

> This library file is from a Hydra version older than 2.2.0, and Hydra could not set it aside: <reason>. Close any program using hydra.db and start Hydra again.

`hydra_batch` and `hydra_fillcompare` never rename the file. Given one older than 2.2.0, they stop and say that opening it in Hydra sets it aside and starts a fresh library.

### Toolbar

The toolbar holds, from left to right, `Manage folders... (N)`, `Scan library`, `Analyze library...`, `Compare with dmleaderboards...` and the `Analysis settings` button. `Open path report` sits at the right edge whenever your library has songs and `hydra_rules.ini` has no error.

`Manage folders... (N)` opens the `Song folders` window, where you choose which folders Hydra Deluxe reads. N is how many folders you have. Click `Add folder...` and pick your Clone Hero song folder, or any folder of charts. Hydra Deluxe finds every chart in its subfolders too. Each folder in the list has a red `X` that removes it, after you confirm. `Close` closes the window. After you add or remove a folder, and while at least one folder is left, a `Scan now` button appears beside `Close`. It closes the window and starts a scan, because a changed list only takes effect when you scan.

Removing a folder takes its songs out of the list at the next scan, and that scan also deletes the saved results of the songs it took out. If you add the folder back later, its songs come back as `Not analyzed`.

Hydra Deluxe reads `notes.mid` or `notes.chart` folders with a `song.ini`, `.sng` archives, and the `.srb` bundles that Clone Hero's built-in setlist ships with. Adding the game's own `Clone Hero_Data\StreamingAssets\songs` folder brings in the default songs too.

`Scan library` reads your folders and updates the song list. A song needs a valid ini file to show up. A song that fails is skipped, and the scan window lists the problems when it finishes. Re-scans are fast, because a chart whose file size and timestamp haven't changed is not read again. You can cancel a scan part-way, and the old library stays as it was. Songs stay in the list as they were until the next scan, even if you changed their files, so scan again after you add, remove or edit songs.

`Analyze library...` analyzes many songs at once, in the background. While a search or a chip other than `All` narrows the list, the button reads `Analyze shown (N)...` and analyzes only the N songs the table shows. A filter Hydra Deluxe can't read narrows nothing, so with only `stars:9` typed and the `All` chip on, the button still reads `Analyze library...`. See [Analyzing the whole library](#analyzing-the-whole-library).

`Compare with dmleaderboards...` compares a player's leaderboard scores with your stored results. It works only under Expert, an SP cap of 4, `1.0 fills` off and Note Shuffle off. With any other settings the button is greyed out, and hovering it says which setting is in the way. See [Reports](#reports).

`Analysis settings: ...` shows the settings your analyses run under, and opens the panel where you change them. See [Analysis settings](#analysis-settings).

`Open path report` opens the path report in its own window. When the report has to be built, Hydra Deluxe first analyzes every song that has no current result. The button appears whenever your library has songs and `hydra_rules.ini` has no error, and it stays while a report is open or building. While a report builds, it reads `Building path report...`. You can still click it, and the window shows that the report is building. See [Reports](#reports).

While a batch runs, `Scan library` and `Analyze library...` are greyed out, and hovering them says `A batch is running.` `Scan library` is also greyed out while a scan runs and when you have no folders. `Analyze library...` is also greyed out when there is nothing to analyze, and while `hydra_rules.ini` has an error.

## Analysis settings

The `Analysis settings` button sits on the toolbar, after `Compare with dmleaderboards...`. Click it to open the settings panel, which holds every setting that shapes an analysis.

The button's label names only the settings you have changed from their defaults, joined with ` · `, for example `Analysis settings: Hard · Note Shuffle`. With every setting at its default it reads `Analysis settings: defaults`. When the window is too narrow for the whole label, the label is cut short; hover over the button to read all of it.

The panel has three groups. A change applies the moment you make it, and there is no Apply button. Checking a box leaves the panel open. Click outside the panel or press `Escape` to close it.

| Group | Setting | Default |
|---|---|---|
| `Chart` | `Difficulty` | Expert |
| `Chart` | `Pro Drums` | on |
| `Chart` | `2x Bass` | on |
| `Chart` | `Note Shuffle` | off |
| `Clone Hero rules` | `SP cap` | 4 bars |
| `Clone Hero rules` | `1.0 fills` | off |
| `Paths kept` | `Score range` | 4<!-- default: kDefaultDepthValue --> scores |
| `Paths kept` | `Path limit` | on<!-- default: Settings::mslimit_enabled -->, 10 ms<!-- default: Settings::mslimit_value --> |

At the bottom of the panel, `Reset to defaults` puts every setting in the panel back to its default in one click, so the button reads `Analysis settings: defaults` again. It leaves your song folders and everything outside the panel alone. Hover over it to see what it will undo, for example `Resets: Pro Drums off · SP cap 5 bars`. It is greyed out when the settings are already the defaults.

Each setting applies to every song, not only the one you have open. A result is saved together with the settings it ran under. Change a setting and the library shows the results for the new settings; a combination you haven't analyzed yet shows its songs as `Not analyzed`. Change it back and the old results come back, with no batch to run. A song open in the panel is analyzed again under the new settings, the same as clicking it.

While a batch runs, the settings are locked. The button's label ends in ` (locked)`. The panel still opens, but its controls and `Reset to defaults` are greyed out and it says `Stop the batch to change these.` Opening a song never locks them. Changing a setting while the song is still analyzing starts it again under the new settings.

### What each setting does

`Difficulty` picks the difficulty to analyze and preview: Expert, Hard, Medium or Easy.

`Pro Drums` analyzes with cymbals and toms as separate notes, the way Clone Hero scores Pro Drums.

`2x Bass` includes the chart's 2x kicks, like Clone Hero's Double Kick modifier. It works at every difficulty. Each difficulty has its own 2x kicks, though few charts have any below Expert. For example, say you have a song open with 2x Bass on and want to see it with 1x bass. Uncheck `2x Bass` and the song is analyzed again without its 2x kicks. Check it again and the 2x result comes back.

`Note Shuffle` scores the chart the way Clone Hero 1.1's Note Shuffle modifier rearranges it. The game moves notes between pads, and the move is the same every time for a given chart and drum setting. So each Difficulty, Pro Drums and 2x Bass choice gets its own shuffled result, kept beside the unshuffled one. The Paths, Preview, Stars and Dynamics tabs and batches all use the shuffled chart while it's on. If Clone Hero would freeze trying to shuffle a chart, that chart shows no path, and a sentence saying so takes its place. The leaderboard comparison is off while Note Shuffle is on, because leaderboard scores don't say whether it was used.

`SP cap` is the most bars of Star Power the meter can hold during the analysis. 4 is Clone Hero's rule and the default. Leave it at 4 for paths you mean to play. Any other number is a what-if: its scores can't be reached in the game. At 1 bar no path can activate Star Power, and the Paths tab says so. Results are kept per cap, so a 4-bar result and a 16-bar result for the same song sit side by side. The leaderboard comparison only runs at 4 bars.

`1.0 fills` spawns drum fills by Clone Hero 1.0's rule instead of 1.1's. A fill only appears if your Star Power was ready in time. Clone Hero 1.1 wants it ready 4 beats before the fill. 1.0 wanted it about one fill-length before, so short fills were looser and long fills stricter. Leave it off for current Clone Hero, and check it to price a run played on 1.0. Results are kept per rule, so a song can hold a 1.1 result and a 1.0 result side by side. The leaderboard comparison needs it off. Everywhere else, Hydra Deluxe names the two rules `Clone Hero 1.0` and `Clone Hero 1.1`, or `CH 1.0` and `CH 1.1` in narrow columns.

`Score range` sets how many extra paths below optimal to keep, and its dropdown picks the unit. `scores` keeps the next few distinct scores under optimal. `points` keeps every path within that many points of optimal. These paths are a little worse, but handy when the optimal path is awkward to play. More paths make the analysis slower.

`Path limit`, when checked, keeps extra paths only when every squeeze and required early fill on them is within this many milliseconds. That keeps alternates you can realistically hit. A path that ties the best score is kept even when its timing is over the limit, and its figure shows in orange; below the best score, a path over the limit is dropped. Lower or negative values demand more slack. The limit compares raw milliseconds: for a squeeze, the gap at the SP end; for an early fill, the gap at the fill deadline. It does not account for frontend timing scaling (see the Paths tab), so a kept path can be a bit harder than its number suggests where an activation's scale line is orange.

## Searching the library

The library lists every song from the latest scan, one row per song. Click a song to open it. Its columns are `Title`, `Artist`, `Charter`, `Folder` and `Best path`.

Click a column heading to sort by it. Right-click the heading row to hide or show a column; every column but `Title` can be hidden. Hydra Deluxe remembers which columns are hidden and the sort. While the song panel is open, the `Charter` and `Folder` columns hide to save room, and they come back when you close it. The columns always stay in this order.

Song names show without Clone Hero's colour and style tags, here and in the report windows. A name that is nothing but tags reads `(unknown)`.

Columns that never cut their text, such as numbers, are as wide as their widest entry. The text columns split the room that is left in fixed shares, so their widths never follow their text. An entry longer than its column ends in `…`; hover over it to read all of it. No column gets narrower than its heading. When the window is too narrow even for the headings, the table scrolls sideways. Every table in Hydra Deluxe sizes its columns this way. You can drag a column's edge to resize it. In the Library, the width holds until a scan reloads the list or you change the search.

The `Best path` cell shows the song's state under the current analysis settings:

| The cell shows | It means |
|---|---|
| `Not analyzed` | There is no result yet. |
| A score and a path, like `378,315  3- 1 2` | The optimal path. |
| `Stale` | The result is out of date. Either another version of the app made it, or it was made under different rules in `hydra_rules.ini`. Click the song or run a batch to refresh it. If the cause was a rules edit, switching the rules back brings the result back. |
| Nothing | The song was analyzed, but the analysis kept no path. Its panel says `No paths found.` |
| `Can't analyze` (dim) | Hydra Deluxe tried, and something in the chart file itself stops the analysis, like `No Expert Pro Drums notes in this chart.` |

Hover a `Not analyzed` or `Stale` cell for the same explanation. Hovering a `Can't analyze` cell shows the reason, with a line of details under it when there are any.

Hydra Deluxe remembers four kinds of failure, because each comes from the chart file and happens again on every try: the chart has no notes for your difficulty and drum type, a timing line Hydra Deluxe refuses, a chart Clone Hero freezes on with Note Shuffle on, and a file Hydra Deluxe can't read or that is damaged. A failure is saved with the settings it happened under, like a result. Hydra Deluxe tries the song again on its own once its file changes, for example after you download it again, because a changed file counts as a new chart. Any other failure, such as a song file that was moved, isn't remembered, so the song stays `Not analyzed`. Clicking a `Can't analyze` song still analyzes it, and its panel shows the same reason.

The chips above the table filter by state: `All`, `Not analyzed`, `Stale`, `Analyzed` and `Can't analyze`, each with its count, like `Can't analyze (N)`. A chip with a count of 0 is greyed out.

### The search box

Type in the search box to narrow the list. `Ctrl+F` jumps to it. `Escape` or its `X` clears it. The heading above shows how many songs match, like `Library 5 of 97 charts`. The text that matched is highlighted in gold.

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

`stars:` and `squeeze<=` only look at current results. A song that is not analyzed, or whose result is `Stale`, never matches them. A filter Hydra Deluxe can't read, like `stars:9`, shows a short note under the search box and is left out of the search.

While you search for words and the `Folder` or `Charter` column is hidden, each row gets a second line under its title. That line shows the song's folder. When the search matched the charter instead, and the `Charter` column is hidden, the line reads `charted by` and the charter's name. The matched text is highlighted in gold there too.

Under the table, a search shows a `Clear search` button. When a match came from a column that is hidden, a note there says so, like `Matched on folder.`

## The song panel

Click a song to open its panel beside the library. Opening a song analyzes it under the current settings and shows its paths. The top shows the title, the artist and the charter. The `<` and `>` buttons step to the previous and next song in the list. The `X` button, or `Escape`, closes the panel.

`Hide library`, left of `<`, gives the song panel the whole window. It then reads `Show library`, which puts the library back at the width you left it. `<` and `>` still step through the list while it's hidden. Closing the panel always brings the library back. Hydra Deluxe remembers the choice, so the next song opens the same way, even after a restart.

Below the header is the headline: the best score, its path, and one line like `Optimal path · 7 stars`. Under it are four tabs: `Paths`, `Preview`, `Dynamics` and `Stars`.

Most songs take a few milliseconds to analyze, so the paths appear at once. A song that takes longer than 0.15 seconds shows `Analyzing chart...` with a progress bar and a `Cancel` button. Cancelling shows `Analysis cancelled.` with a `Try again` button, and nothing is saved for that song. Closing the panel or clicking another song cancels it the same way.

When the analysis finishes, the song's result is saved to the library if it was missing, `Stale` or different. So the library row and the panel always show the same numbers.

If the analysis fails, the panel says why in orange, with the error itself dimmed under it, and a `Continue` button. `Continue` clears the panel. A song the analysis kept no path for shows `No paths found.`

If the song's file has moved or been deleted since the last scan, the panel says `Song file not found.` and offers a `Rescan library` button. Its library row and result stay. If the chart was edited since the last scan, opening it reads the edited file, and the library row follows.

## Paths tab

The Paths tab lists the paths the analysis kept on the left. Click a path to show its activations on the right.

The list is grouped by score. Each score is a fold that shows the score and how many paths share it, like `378,315  1 path` on Burnout; click it to hide or show those paths. `Optimal` comes first. When several paths tie for the top score, all of them are optimal, because playing any of them earns the same score. Then come the extra paths under a heading like `Within 2 scores`.

Last comes `Best all-0 path`: the best path that activates at the first chance every time, with no skips. An early fill it lets pass reads `E1` and still counts as no skip: if you don't get that fill, the next one is your first chance anyway. Every squeeze and early fill on it is 0 ms or easier. It also shows how far it falls below optimal. This heading appears only when the all-0 path isn't already in the list above it, and it can hold several paths that tie.

Each path shows its own hardest timing right after it, like `3- 1 2   163.0 ms` under the `378,315` fold: the hardest squeeze or required early fill that path needs. It turns orange past the difficult limit. A path that needs no timing shows nothing there.

### The activations

The right side starts with a summary, like `Activations 3 · no SP left over`. Under it, a timeline runs from the first measure to the end of the song, with a gold mark for each activation. Hover the timeline to read which activation a mark is. A mark is outlined when its row has a timing badge, in orange only when that timing is orange too.

The timeline's length is the song's length as the chart's own files state it: the `song_length` line in song.ini, the same key in a .sng, or the length stored in a .srb. A chart that states none ends at its last Expert drum note. Hydra Deluxe never opens the audio to find it, so a chart with no audio still gets a timeline. A song with a long outro ends its last mark well before the right edge. A chart with no length at all, meaning no stated length and no Expert drum notes, shows no marks.

Each activation is one row. It shows the activation's number, its notation, its measure (like `m32.1.0`), the bars of SP banked when you activate (like `1 bar` or `2 bars`), and a badge for its hardest timing, like `squeeze out 163 ms` or `early fill 20 ms`. Click a row to open it, or use `Expand all` and `Collapse all`. The first row starts open.

An activation that skips fills keeps its early fill badge when its Star Power is ready no earlier than the fill's deadline, because that timing decides whether the first fill shows up. An early fill with time to spare has nothing to time, so it gets no badge. A squeeze that happens on its own still gets one when nothing else on the row needs timing, with a figure of 0 or below: `squeeze in -316 ms` means the phrase's last note already lands 316 ms before Star Power ends.

An open row shows these, in order:

1. The chord you activate on. On a multi-note chord, do a frontend squeeze: hit the activation note first, so the other notes score with the Star Power multiplier.
2. `Show in Preview >`, which jumps the Preview tab to this activation.
3. The early fill timing, when the activation has one (the `E` notation), like `Early fill: 0.0 ms (required)`. A positive number means you must hit that many ms early to make the fill show up. A negative number is slack, so the fill shows up with room to spare.
4. A plain sentence for each squeeze: which note to hit early or late, by how much, and what it's worth. Notes are named the way the Dynamics tab names them, like `Green tom` or `Yellow cymbal`.
5. `Backend timings`, a fold that starts open: the notes near the end of Star Power, with a short lead-in above the table.

### Backend timings

A backend squeeze means hitting the note at the SP end early, so it lands inside Star Power. In the `Backend timings` table, the note at `0.0 ms` is exactly where SP ends; the others are the notes just before and after it. If the last activation's SP runs past the end of the chart, the table lists the notes before that end, so every timing is negative.

The rating beside each note is a rough guide to how hard it is to fit into Star Power. A note the path doesn't count is tagged `(uncounted)`, squeeze-out notes included, like `Free SqOut (uncounted)`. For backend squeezes, look for notes from the backend leeway up to the hit window. The leeway is 3 ms<!-- default: Rules::backend_leeway_ms --> by default (`backend_leeway_ms` in `hydra_rules.ini`). The hit window is 85 ms by default (`hit_window_ms`, below).

### Frontend timing scale and overfill

Star Power length is measured in measures, not milliseconds: one bar of SP lasts two measures. If the SP end falls where measures last a different time than at the activation (a tempo or time signature change), frontend timing only partly carries to the SP end. Hitting the activation 50 ms late might move the SP end only 25 ms.

Whenever the scale isn't x1.00, the opened row shows it, early first: `Frontend timing scales x0.99 (early) / x4.45 (late) at the SP end.` Late and early hits can scale differently when the activation or the SP end sits right on a change. A side that is x1.00 is left out. If a SqIn moved the SP end, that earlier end gets its own clause. The line is orange when the scale changes a squeeze or backend figure, and those backend rows show an effective timing (`eff.`). Otherwise it is gray.

Sometimes a phrase collected during Star Power would push the meter past the SP cap, so the cap cuts the SP end short. A phrase that only brings the meter exactly to the cap doesn't count. When the cap cuts the end short, the note that filled the meter is the one whose timing moves the SP end, instead of the activation. The scale line and the eff. figures are measured from that note. The row shows an overfill warning, like `SP overfilled at m40.1.0`, when that matters: when the row lists a squeeze, or a backend note the path squeezes out or doesn't count.

### Below the activations

Under the activation rows are two folds, `Multiplier squeeze` and `Score breakdown`. Both start open.

`Multiplier squeeze` lists the multiplier squeezes in the song. When the combo multiplier goes up on a multi-note chord, hitting the more valuable notes (cymbals, dynamics) a little later scores them on the higher multiplier. This is why two FCs of the same song can differ by 15<!-- default: kCymbalBonusPoints --> points. `2x` means it happens as the multiplier reaches 2x (10<!-- default: to_multiplier:x2 --> notes in), and so on.

`Score breakdown` shows the score this path should get, in the same categories as Clone Hero's results screen. It includes multiplier, frontend and backend squeezes. `Notes`, `Accent Notes` and `Ghost Notes` together make the `Base score` on the Stars tab. The accent and ghost lines hold only the extra 50<!-- default: kNoteBasePoints --> points each of those notes adds, and a cymbal's extra points stay in `Notes`. A backend note that lands after the SP end still counts under Star Power only when it lands less than a small leeway after it, 3 ms<!-- default: Rules::backend_leeway_ms --> by default (`backend_leeway_ms` in `hydra_rules.ini`).

`Copy path` copies the selected path's notation. `Ctrl+C` does the same and flashes `Copied!`.

`Hide backend rows beyond` hides backend rows more than this many ms from the SP end. It starts off<!-- default: Settings::backendlimit_enabled -->, with 50 ms<!-- default: Settings::backendlimit_value --> in the box. A note the path squeezes out always shows. When it hides rows, the `Backend timings` line still counts every note and adds how many are hidden on each side of the SP end, like `8 notes near the SP end · 8 hidden: 4 before, 4 after`. The fold says the same with the limit, like `All 8 hidden: 4 before the SP end and 4 after, beyond 50 ms.` This only changes the display. It is not an analysis setting, so changing it never re-analyzes anything.

One more setting feeds these displays: `hit_window_ms` in `hydra_settings.ini` (default 85, the Clone Hero Pro Drums hit window per side). It has no control on screen yet.

## Preview tab

The Preview plays the chart as a 3D note highway, in time with the song's audio. It draws the chosen path on the highway: the Star Power windows as a teal floor, and the fills a player following the path would see.

Where Star Power runs out, a bright teal line crosses the floor and a small teal triangle sits just outside each railing, level with the line. A note just before the end can cover the line, but the triangles stay in view, so you can always see exactly where SP stops.

Where the chart marks a drum roll or trill, the highway lights a lane under it, in the pad's colour. A roll lights one pad's lane, and a trill lights up to two, the pads it moves between. Hydra Deluxe picks the pads the way Clone Hero does, from the first notes under the lane. When a roll's first chord holds a crash and a drum, the drum's lane lights, not the crash's. A lane shows on the same difficulties as in Clone Hero. Clone Hero drops a lane whose first note comes late, but nobody has read how late, so Hydra Deluxe draws every lane. In Clone Hero a lane also makes its notes easier to hit. Hydra Deluxe only draws the lanes: its scores and timings don't include that help.

### Choosing what to watch

`Showing` picks which path to draw. It lists the same paths as the Paths tab, in the same order. The all-0 path reads like `0 0 0 0  (best all-0)`.

`< Act` and `Act >` jump to the previous and next activation. `Show in Preview >` on the Paths tab lands here too. A jump lands on the activation chord with it already hit: the gem shows struck, and the score box counts it.

The transport buttons are `-5s`, `< 10 ms`, `Play`/`Pause`, `10 ms >` and `+5s`, with a volume slider beside them. Each has a key:

| Key | Does |
|---|---|
| `Space` | play and pause |
| `Left` and `Right` arrows | jump 5 seconds |
| `,` (comma) and `.` (period) | step 10 ms and pause |
| `[` and `]` | jump to the previous and next activation |

The same keys are drawn as keycaps in a bar under the highway, in the buttons' order. Each button's tooltip names its key too.

The scrubber under the highway shows where you are. Its gold marks are the path's activations, in the same gold as the next-activation box. The scrubber measures position against the song's length, like the Paths timeline, so an activation sits at the same spot on both. When the song's stated length runs past the last note and the audio, the scrubber ends where playback ends. A chart with no length still scrubs, up to where playback ends, but the Paths timeline shows no marks for it, so there is nothing to line up. Audio that runs past the song's length still plays.<!-- owners: scrub_end_ms (the scrubber's end), app::song_length_ms (the song's length) --> The beat lines run on to the end of the audio.

### When the settings or the chart change

The Preview follows the Analysis settings. Change the difficulty, Pro Drums, 2x Bass or Note Shuffle and that mode's notes appear in place. The song keeps playing, or stays paused, at the same spot. If the chart has no notes for that mode, the Preview pauses where it is and says so, like `No Hard Pro Drums notes in this chart.` Switch back and the notes return at that spot.

If the chart file changed since it was analyzed, the Preview draws no path and shows `This chart changed since it was analyzed. Click the song again to see its path.`

### The boxes on the highway

The time box, at the top left, shows the measure you're at and the measure where playback ends, the tempo and time signature, and the practice section when the chart has them. Playback ends at the end of the audio (or at the last note, when the audio stops first), and the clock beside the scrubber counts to that same point.

A position reads like `m27.2.450`: measure 27, beat 2, and 450 ticks into that beat. A tick is the chart's own smallest step of time, and the chart's resolution sets how many ticks make a quarter note. The beat number counts the beat lines on the highway, so a beat is a quarter note in 4/4 and an eighth note in 6/8 or 7/8. The Paths tab writes positions the same way.

The score box, under it, shows the running score in large type, then a row of seven stars, then the multiplier and combo, like `x4 · combo 212`. The stars you have earned so far are filled in gold. A last line says which star comes next and how many points it still needs, like `star 3 in 595`; once you have all seven, that line goes away. Stars count your score without the solo bonus, as Clone Hero does, so a solo's bonus raises the score but never the stars. The box appears once the song is analyzed. It reads `Score unavailable` when the path can't be replayed to its stored score.

The SP meter, at the right edge, is a gauge of banked Star Power, one line per bar, with the bars shown under it. It rises one bar at each phrase you collect and drains through each activation, reaching empty exactly where the path's SP ends. It holds as many bars as the result's SP cap. Before the song is analyzed, it uses the SP cap from the Analysis settings, the one the next analysis will run at. A chart with no Star Power still shows the meter, empty, and the drain box.

The drain box, at the top right beside the meter, shows how long one bar of SP lasts at the current tempo, like `1 bar / 2.5 s`. While SP is running on the path it reads `SP drain` and `empties in` the time left, in teal like the floor. Otherwise it reads `SP drain (if activated)` and `full meter`: how long a full meter would last at the current tempo.

The next activation box, at the top right under the drain box, starts with the next activation's number, like `Next: activation 2 of 3`. Under that is the activation in path notation with its timings, like `5+ (100 ms)`, and then where it is and its chord. After the last activation the box goes away. `Copy path` includes the same text for each activation.

The solo box appears right of the highway, a little below the middle, once you hit a solo's first note. It shows how much of the solo you have hit so far, like `85%`, and under that the notes, like `102 / 120 notes`. The percent is worked out the way Clone Hero works it out, so it can read one below the exact share. Hitting the solo's last note ends the solo. The box then shows `Perfect Solo!` for 1.5 seconds, then the solo bonus of 100 points per note, like `+12,000`, for 1.5 seconds, and then goes away. The path hits every note, so a solo always ends in `Perfect Solo!`. If a new solo starts while the box still shows the last one's result, the new solo takes over. The box works before the song is analyzed too.

### Other things the Preview can show

With no song selected, the tab says `Select a song to preview.` While a chart loads, it shows `Loading preview:` and the current step over a progress bar. If the chart can't be loaded, it shows `Preview failed:` and the reason in orange, with the error itself dimmed under it.

If Hydra Deluxe can't use your audio device, the Preview still draws and plays the chart, without sound. An orange line says `Hydra couldn't use your audio device; the preview is muted.`, and hovering it shows the device's own message.

On a computer where Hydra Deluxe can't get a graphics device to draw with, the tab shows only `Preview is unavailable (no render device).`

## Dynamics tab

This tab counts the chart's ghost and accent notes. Ghosts and accents are the soft and hard hits that score double in Clone Hero.

The `Pads` table shows, for each pad (and each cymbal separately under Pro Drums), how many notes are ghosts, accents and normal hits. `Kicks` does the same for kicks, with 2x kicks on their own row and a line saying how many kick notes are 2x. Percentages round to the nearest whole number. When 2x Bass is off, the 2x kick row stays visible but greyed out, and both All kicks and the totals leave it out. `Totals` adds them up.

The `Chart` box says whether the chart has dynamics turned on. A MIDI chart has to opt in with a tag. Without that tag Clone Hero ignores the velocity markings and plays every note as a normal hit. Hydra Deluxe reads the chart the same way, so the tab shows no ghost or accent counts. It says `This chart has no ghost or accent notes.`, and the Chart box says `Dynamics enabled: no (markings ignored by Clone Hero)`. When the tag only comes partway through the chart, the markings before it are ignored too, and the Chart box says how many there were.

Counts are worked out when you open the song, together with its paths. Nothing is saved, so they always match the chart file as it is now.

## Stars tab

This tab shows the score you need for each star. A star counts once your score, without the solo bonus, reaches its cutoff.

Above the table, `Base score` is every note hit once at 1x, with no Star Power. When the chart has a solo, `Solo bonus` is its full solo bonus. The solo bonus does not count toward stars, because Clone Hero compares your score without it.

The table has one row for each of stars 1 to 7:

| Column | What it shows |
|---|---|
| `Stars` | the star |
| `Average Multiplier` | the average multiplier a path needs for that star; the cutoff is the base score times this factor: 0.1<!-- default: kStarMultipliers[1] -->, 0.5<!-- default: kStarMultipliers[2] -->, 1.0<!-- default: kStarMultipliers[3] -->, 2.0<!-- default: kStarMultipliers[4] -->, 2.8<!-- default: kStarMultipliers[5] -->, 3.6<!-- default: kStarMultipliers[6] --> and 4.4<!-- default: kStarMultipliers[7] --> |
| `Cutoff` | the score that earns the star, rounded up |
| `With full solo bonus` | the cutoff plus the whole solo bonus: what the results screen shows at that cutoff if you also collect every solo bonus. This column shows only when the chart has a solo. |

## Analyzing the whole library

`Analyze library...` analyzes the songs in your library in one background run, called a batch. While a search or a chip other than `All` narrows the list, the button reads `Analyze shown (N)...` and analyzes only the N songs the table shows.

First a window titled `Analyze library` asks you to confirm. It says how many songs it will analyze and lists the settings it will use: `Difficulty`, `SP cap`, `Fills`, `Score range` and `Path limit`. To change those, cancel and edit the Analysis settings. Click `Start analyzing` to begin, or `Cancel`.

A batch skips songs that already have a current result under these settings. A `Stale` result isn't current, so a batch runs those songs again with the box unchecked. To analyze the songs with a current result again too, check `Also re-analyze charts that already have a result`. When every song already has a current result and the box is off, the window says `Every chart here already has a result.` and `Start analyzing` stays greyed out.

A `Can't analyze` song counts as one with a result, because its saved failure is its result. A batch skips it and counts it as skipped. When some of the songs in the batch can't be analyzed, the `Analyze library` window adds a dim line: `N of these can't be analyzed. Check the box to try them again.` Checking `Also re-analyze charts that already have a result` tries them again along with the rest.

The batch uses all but one of your CPU cores, and 8 at most. You can keep browsing and open songs while it runs. A strip under the toolbar shows its progress: how many songs are done, the song being analyzed, the time so far and, after a few songs, an estimate of the time left, like `about 1:30 left`. An open song's panel already shows the engine's result, so the batch only updates its library row.

`Pause` holds the batch, and `Resume` carries on. `Stop` ends it and keeps every result finished so far. The Analysis settings stay locked until the batch ends.

When the batch ends, the strip changes to a summary: how many songs were analyzed, failed and skipped, and how long it took. Songs that failed are listed under `N charts failed`, each with the reason. A finished batch then builds the [path report](#the-path-report). A stopped batch keeps its results but builds no report, because a report of part of the library would read as the whole of it.

Once the report is built, the summary strip offers these:

| Control | What it does |
| --- | --- |
| `Open report` | Opens the path report window. The strip's second line reads `The path report is ready.` Once you close the path report window, the report is gone, so the line and the button go too. The toolbar's `Open path report` still opens it. |
| `Open automatically` | Opens the path report as soon as it's built. It covers only the path report. The strip is the only place to change it, so it stays after you close the path report window. It hides while a report builds and after a build fails. |
| `X` | Dismisses the strip. |

If the database holds no results at all once a batch ends, as when every song in your first batch fails, the strip says `The path report could not be built.` and `Nothing is analyzed yet. Click a song or run a batch to analyze it.`

## Reports

### Report windows

Hydra Deluxe has two reports: the path report and the dmleaderboards comparison. Each one opens in its own window. It's a real Windows window with its own taskbar button, so you can drag it to another monitor and keep it open while you use the rest of Hydra Deluxe. It minimizes and restores with Hydra Deluxe and sits in front of the main window. Both windows can be open at once.

The first time a report window opens, it covers the main window, at the same size and place. After that it opens where you last left it, and maximized if you left it maximized, on the monitor it was on. While the main window is maximized, a report window opens maximized too. If the monitor a report was on is no longer connected, the window opens over the main window again, or maximized on the main window's monitor if it was maximized. A report window always fits on its monitor, its frame included. The placement is saved in `hydra_ui.ini`, next to the program.

The main window's place is saved in `hydra_ui.ini` too. Hydra Deluxe reopens where you closed it, at the same size, and maximized if you left it maximized. If that spot is on no monitor now, because a monitor was unplugged or moved, it opens at its default place and size.

Both report windows have the same layout. From top to bottom:

| Part | What it shows |
| --- | --- |
| Out-of-date strip | Only when the report is out of date (below). |
| Header | `Hydra` and the report's name, a line saying what the report covers, `Built HH:MM` (when it was built), `Refresh`, which builds it again, and `Close`, which closes the window like its `X`. `Close` is there whatever the report is doing, including while it builds or after it fails. |
| Tiles | Numbers that sum up the rows that pass the filters. They change as you filter. |
| Controls | A search box, a dropdown and, on the right, a count line saying how many rows pass. The path report also has `Optimal paths only`. |
| Table | One row per path or score. Hover a column heading to see what the column means. |
| Footer | A dim note under the table. |

Click a column heading to sort by it, and click it again to flip the order. Shift+click another heading to add a second sort. Empty values always sink to the bottom. Text columns sort the way the Library does. To hide or show a column, right-click a column heading and pick it from the menu. The row number column always stays. Columns are sized the way the Library's are, and number cells start at the left like the text. A width you drag holds until the report is built again or you change the search.

When no row passes the filters, the window says `Nothing matches those filters.` `Clear filters` then empties the search and sets the dropdown back to its first entry.

The window never changes its rows behind your back. After a batch, a report built before it shows a strip: `Your library changed since this report was built (a batch finished at HH:MM).` Opening a song that saves a new result shows the same strip with no time: `Your library changed since this report was built.` Changing a setting the report depends on shows `The settings changed since this report was built.` Each report's section below lists the settings that count for it. The old rows stay readable under the strip. `Refresh` builds the report again and clears the strip.

`Tab` moves through the controls and the table. The `Up` and `Down` arrows move the row selection, and each move selects the song, like a click. `Escape` or `Ctrl+W` closes the window, and so do its `X` and `Close`. While you're typing in the search box, `Escape` doesn't close the window: the first press empties the box, as in the Library's search. Closing a report window doesn't keep the report to show again. The path report is built again when you reopen it, and the comparison fetches the player's scores again when you pick the player again.

### The path report

The path report is one table of paths, squeeze timings and scores for the songs in your library, under your current Analysis settings: the difficulty, Pro Drums, 2x Bass, Note Shuffle, SP cap, `1.0 fills`, score range and path limit. Each chart shows its top 5 paths. A song that can't be analyzed has no rows, and neither has a song with no result. Lines above the table count both.

When you open the report and it needs building, Hydra Deluxe first analyzes every song with no current result under your settings, `Stale` ones included. This is an ordinary batch: the strip under the toolbar shows it, with `Pause` and `Stop`, and the Analysis settings stay locked while it runs. The window's first line reads `Analyzing n of N charts` over a progress bar, with the same counts as the strip. Its `Cancel` button stops the batch the way `Stop` does. A stopped batch builds no report, so the window then shows `Report cancelled.` with `Try again`. If a batch you started yourself was already running, the window shows that batch instead, and `Cancel` leaves it running. When more than 500 songs need analyzing, the report window asks first. Its first line reads `Analyze N charts that have no result yet?`, with `Cancel` and `Analyze` buttons. `Analyze` starts the batch. `Cancel` skips the analysis, and the report builds from the results already saved. At 500 or fewer, the analysis starts at once. When every song already has a result, the report builds straight away.

A finished batch builds the report too. `Open path report` on the toolbar opens it. Hydra Deluxe keeps the report in memory, not in a file. So the first time you open it after starting Hydra Deluxe, it builds a fresh one. Closing the window lets the report go, to save memory, and opening it again builds a fresh one too. The window keeps your search, timing choice, `Optimal paths only` and sort for it. `Refresh` and a batch's new report start them over. If a build is still running when you close the window, it stops. Closing the window during the analysis doesn't stop the batch; use `Cancel` or the strip's `Stop` for that. A report a batch builds while the window is closed waits for you to open it.

Once the analysis is done, the report builds. While it builds, the line under the header reads `Building the report from your library...`, with a `Cancel` button and grey placeholder rows. While `hydra_rules.ini` has an error, analysis is off. A window with no report to show then says so instead of building. Cancelling shows `Report cancelled.` with `Try again`. If the build fails, the window says `The path report could not be built.`, with the reason and `Try again`.

The window is titled `Path report — Hydra`, and its header reads `Hydra Path Index`. The line under it reads like `N charts — top 5 paths per chart — SP cap 4 bars`, and ends in `— Clone Hero 1.0 fills` when `1.0 fills` is on. A chart you have more than one copy of in your library shows its paths once, but each copy counts in that line and in the `Charts shown` tile.

A line above the table counts the songs that can't be analyzed. When they all have no notes for your difficulty and drum type, the line says so, like `59 charts have no Expert Pro Drums notes`. Otherwise it reads `N charts can't be analyzed`. `Show list` opens the list under the line, and `Hide list` closes it again. The list gives each song's title and artist, under a heading that gives the reason. There is one heading for each reason. A second line, `N charts have no result under these settings`, shows only when some songs have no result. Those songs were left unanalyzed, for example because you clicked `Cancel` when the report asked to analyze them, or they failed for a reason Hydra Deluxe doesn't remember, such as a song file that was moved. The batch strip's `N charts failed` list gives each reason of the second kind.

The five tiles sum up the paths that pass the filters:

| Tile | What it counts |
| --- | --- |
| `Charts shown` | The charts those paths belong to, out of every chart in your library. On a library of 19,512 songs where 59 have no Expert Pro Drums notes and the rest have results, it reads `19,453 of 19,512` with no filters on. The first number follows the filters. |
| `Paths shown` | The paths themselves. |
| `Hardest ms` | The hardest squeeze or required early fill any of those paths needs. |
| `Past N ms` | The paths in the `Beyond N ms` timing tier, the hardest one. |
| `Highest skip` | The most fills any one activation on those paths passes over. |

The controls are a search box, a timing dropdown and `Optimal paths only`. On the right, a count line says how many paths pass, like `120 of 4,512 paths`.

The timing dropdown starts at `All timing tiers`. Each other entry shows only the paths in one timing tier. A path's tier comes from its `Hardest ms`, measured in steps of your hit window:

| Entry | Paths it shows |
| --- | --- |
| `Normal`, `Hard`, `Extreme`, `Insane`, `Insane+` | Paths whose hardest timing falls in that band, from easiest to hardest. `Normal` takes timings up to 2.0 ms. |
| `Beyond N ms` | Paths whose hardest timing is past N ms, more than twice the hit window. At the default hit window of 85 ms, N is 170. |
| `No timing needed` | Paths that need no squeeze and no required early fill. |

`Optimal paths only` starts checked. It keeps just the optimal paths among each chart's 5 paths shown. When paths tie for the top score, it keeps every one of them, because each is optimal. `Clear filters` leaves it as it is.

The search box ignores case and accents. Every word you type must appear somewhere in the song, artist, charter or path the row shows, and the words can match different fields. Quotes and the Library's field prefixes, like `artist:`, are ordinary words here.

The table has one row per path and starts sorted by score, highest first. Its columns:

| Column | What it shows |
| --- | --- |
| `Song`, `Artist`, `Charter` | The chart's song, artist and charter. |
| `Path` | The path in path notation: one entry per activation, with its skip count and squeeze symbols. |
| `Score` | The total score the path reaches. |
| `Acts` | Activations: how many times the path uses Star Power. |
| `Max skip` | The most fills any one activation passes over before activating. |
| `Hardest ms` | The hardest squeeze or required early fill the path needs, in ms: the gap at the SP end, or at the fill deadline. It leaves out the frontend timing scale. A dash means it needs none. |
| `Timing` | The path's timing tier, the bands the timing dropdown lists. |
| `Early fill (ms)` | The hardest early fill on the path (written `E0` in the path): how many ms early you must hit to summon the fill. Negative means slack. A dash means the path has none. |
| `Average Multiplier` | The score without solo bonuses divided by the base score (every note at 1x). |
| `SqIn` | SP phrase notes squeezed into an active SP window (`+` in the path). |
| `SqOut` | SP phrase notes squeezed out of an active SP window (`-` in the path). |
| `Note count` | Notes in the chart. |

Click a row to select that song in Hydra Deluxe, the same as clicking it in the library. The report holds only results made under your current settings, so the Paths tab shows the same paths as the row.

When every song either can't be analyzed or has no result, the window shows the lines above the table and `Nothing to show`.

A report built before any song has a result under the current settings, while other results exist, says `The path report could not be built.` and names the settings it looked under, like `Nothing is analyzed under these settings (Expert Pro Drums, 2x Bass, SP cap 8, Clone Hero 1.1 fills). Analyze with these settings, or change them.` When nothing is analyzed at all, it says `Nothing is analyzed yet. Click a song or run a batch to analyze it.`

The path report depends on every Analysis setting: the difficulty, Pro Drums, 2x Bass, Note Shuffle, SP cap, `1.0 fills`, score range and path limit. Changing one marks the report out of date. `Refresh` then analyzes the songs that have no result under the new settings before it lists them. When more than 500 songs need it, the window asks first, the same way: `Analyze` runs the batch, and `Cancel` builds the report from the results already saved. Under settings you have never analyzed, that is every song in your library.

### Old report files

Older versions saved the reports as web pages, `hydra_paths.html` and `hydra_dmcompare.html`, in your **Documents\Hydra** folder. Hydra Deluxe no longer reads or writes them. It leaves any old ones where they are, because deleting your files needs your say. Delete them yourself if you don't want them.

### Compare with dmleaderboards

`Compare with dmleaderboards...` compares a [dmleaderboards.com](https://dmleaderboards.com) player's posted scores with your stored optimals. Pick a player from the searchable list in the `Compare dmleaderboards user` box; Hydra Deluxe remembers the last pick. Picking a player closes the box and opens the comparison in its own window, titled `dmleaderboards: <player> — Hydra`. Hydra Deluxe fetches the player's scores and matches them to your analyzed songs by chart hash.

The comparison needs Expert, an SP cap of 4, `1.0 fills` off and Note Shuffle off. The leaderboard only holds Expert scores played under Clone Hero's rules, and its scores don't say whether Note Shuffle was on. Only songs analyzed under the current settings can match, so analyze your library first for a full comparison.

The first request after a while can take tens of seconds, because the leaderboard's server has to wake up. While the scores load, the window says `Fetching scores and building the report...` and `The leaderboard server can take a moment to wake up.`, over a moving bar. The server gives no count, so the bar only shows that work is happening. `Cancel` closes the window and reopens the player list.

The comparison window has the same layout, keyboard and placement as the path report window (see [Report windows](#report-windows)). Its header reads `Hydra vs dmleaderboards`, and its line gives the player's name and total number of scores. Next to `Refresh`, which fetches the player's scores again and rebuilds the comparison, the header has `Compare another player...`, which reopens the box. There is one comparison window, so comparing another player replaces what it shows. Closing the window doesn't keep the comparison: to see it again, pick the player again, which fetches the scores again.

Every row gets one status. The status dropdown shows one status at a time, or `All charts`. These are its entries:

| Status | Which scores get it |
| --- | --- |
| `Under optimal` | Scores played at 100% speed that are below Hydra Deluxe's optimal for the chart. |
| `At optimal` | Scores played at 100% speed that equal it. |
| `Above optimal` | Scores played at 100% speed that are higher than it. |
| `No current result (in your library)` | Scores for a chart in your library that has no current result for this mode at SP cap 4. |
| `No paths (analyzed, none kept)` | Scores for a chart that was analyzed, but whose analysis kept no path. |
| `Not in your library` | Scores for a chart the last scan didn't find. |
| `Other speed` | Scores played at a speed other than 100%. |

A score played at another speed is always `Other speed`, whatever else is true of it. Hydra Deluxe's optimal is for normal speed. So `Other speed` rows show Hydra Deluxe's numbers when it has them, but aren't compared with its optimal.

Rows above optimal are expected, not errors. Hydra Deluxe's optimal leaves out several score backends on purpose. Many leaderboard scores were also set on older Clone Hero versions, whose fill rules allowed totals that are impossible now.

The tiles count only the rows that pass the filters:

| Tile | What it shows |
| --- | --- |
| `Scores shown` | Every row that passes, whatever its status. |
| `Under optimal`, `At optimal`, `Above optimal` | The rows with each of those statuses. |
| `No current result` | The `No current result (in your library)` rows. |
| `Not in your library` | The `Not in your library` rows. |
| `Other speed` | The `Other speed` rows. |
| `Avg % of optimal` | The average of the `% of opt` column, over the rows that have one. |
| `Points left on table` | The points left on the `Under optimal` rows, added up. |

`No paths (analyzed, none kept)` rows have no tile of their own, but `Scores shown` counts them.

The search box matches the song, artist and charter. The table starts sorted by points left, most first. Its columns:

| Column | What it shows |
| --- | --- |
| `Song`, `Artist`, `Charter` | The chart's song, artist and charter. |
| `Actual` | The score the player posted. |
| `Hydra opt` | The optimal score Hydra Deluxe found for the chart at SP cap 4, the Clone Hero rule. |
| `Points left` | `Hydra opt` minus `Actual`. A score above optimal reads how far over it is, as `+N over`. |
| `% of opt` | `Actual` as a percent of `Hydra opt`. Only for scores played at 100% speed. |
| `FC` | A check mark for a full combo: every note hit. |
| `Percent` | The percent of notes the player hit. |
| `Speed` | The playback speed the score was set at. 100% is normal speed. |
| `Rank` | The score's rank on this chart's leaderboard. |
| `Posted` | The date the score was posted. |
| `Status` | The row's status, written in lowercase in the table (`under optimal`, `no current result`, `not in library`). The dropdown spells out the same statuses, as in the table above. |

Clicking a row selects its song, as in the path report. A `Not in your library` row has no song to select, so clicking it does nothing, the `Up` and `Down` arrows pass over it, and hovering it says `Not in your library`.

The comparison goes out of date the same way the path report does, with the same strips. A batch marks it, and so does opening a song that saves a new result. So does a change to the difficulty, Pro Drums, 2x Bass, Note Shuffle, `1.0 fills`, the score range or the path limit, because those decide which of your results it compared against.

## Scoring rules (`hydra_rules.ini`)

A few of Hydra Deluxe's rules are judgment calls, not facts read from Clone Hero. You can change them in `hydra_rules.ini`, a plain text file next to Hydra.exe. The app and every command line tool read the same file.

The file is optional. A missing file, or a missing line, means the default below. Each line is `key = value`. A `#` starts a comment, either on a line of its own or after a value on the same line. There are no `[section]` headers.

```ini
# Hydra Deluxe's defaults
backend_leeway_ms = 3.0
sqout_rule = first_note
max_tied_paths = 4
fill_cooldown_measures = 4
fill_max_distance_beats = 0.5
fill_land_slop_beats = 0.03125
```

What each line does:

| Key | Default | Allowed values | What it sets |
| --- | --- | --- | --- |
| `backend_leeway_ms` | 3.0 ms<!-- default: Rules::backend_leeway_ms --> | 0 or more | A backend note that lands less than this far past the Star Power end still counts in the score. Exactly this far past it does not. |
| `sqout_rule` | `first_note` | `first_note` or `whole_chord` | What a squeeze-out costs. `first_note` removes the Star Power doubling from one note of the chord (the lowest-value one). `whole_chord` removes it from every note in the chord. |
| `max_tied_paths` | `4` | A whole number, 1 or more | How many paths Hydra Deluxe keeps at one score. It's one count per score, whichever side of the Path limit each path falls on, and paths inside the limit come first. More paths means longer lists and slower analysis. |
| `fill_cooldown_measures` | `4` | A whole number, 1 or more | For charts with no authored fills: how many measures must pass after an activation point before Hydra Deluxe places the next one. |
| `fill_max_distance_beats` | `0.5` | 0 or more | For charts with no authored fills: how far from a measure line a note can sit and still get a fill. Under Clone Hero 1.1's rule, Hydra Deluxe adds one tick to this, as Clone Hero does. |
| `fill_land_slop_beats` | `0.03125` (a 32nd of a beat) | 0 or more | How close after a fill's end a note must be for the fill to land on it. Hydra Deluxe adds one tick to this, as Clone Hero does, so even `0` lets a note one tick late take the fill. It applies only to fills written in the chart. |

Older files may still have `auto_cap_ladder`, `auto_budget_s` or `fill_length_measures` lines. Hydra Deluxe reads and ignores them, so those files keep working.

A value Hydra Deluxe can't read, a key it doesn't know, or a line with no `=` is an error. The error names the key, or quotes the line. The command line tools print the error and stop with exit code 2. The app still opens and shows the error, but analysis stays off until you fix the file and restart Hydra Deluxe. Hydra Deluxe never analyzes on the defaults behind your back.

Every result remembers the rules it was made with. After you change the file, results made under the old rules show `Stale` until you click the song or run a batch. Switching the rules back brings those results back.

## For power users: command line tools

You never need these; everything they do is in the app. Two console programs sit next to Hydra.exe and share its settings and library. `hydra_batch` analyzes the library, and `hydra_fillcompare` compares Clone Hero 1.0 and 1.1 fill results. The path report lives only in the app. The [developer page](development.md#command-line-tools) lists their flags.
