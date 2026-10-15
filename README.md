# Hydra Deluxe
Hydra Deluxe finds the best Star Power paths for Clone Hero drums and shows you how to play them.

Its song browser keeps the information easy to reach, even in a large song library.

<br><p align="center"><img src="/resource/icon_app.png" width="200"></p>

Hydra Deluxe grew out of [DragonDelgar's Hydra](https://github.com/DragonDelgar/hydra). It started from the public v1.3.1 in August 2026, was rewritten as a native Windows app, and has grown a lot since. [How Hydra Deluxe differs from the public Hydra](docs/differences-from-public-hydra.md) covers every change, and why scores can differ between the two.

## What Hydra Deluxe does

Hydra Deluxe reads your Clone Hero drum charts and finds the Star Power paths that give the best score. It also keeps a few paths just below optimal, in case the best one is awkward to play.

For each path it tells you how to play it. Each activation says where it is, which chord to activate on, and which squeezes it needs. Each squeeze is a sentence saying which note to hit early or late, by how much, and what it's worth. A score breakdown matches the categories on Clone Hero's results screen.

The **Preview** plays the chart as a 3D note highway with the song's audio. It draws the path on the highway, with a running score, a Star Power meter, the stars you've earned so far and the next activation in path notation. The **Dynamics** tab counts ghost and accent notes. The **Stars** tab shows the score each star needs.

One `Analysis settings` button holds every setting an analysis runs under: difficulty, Pro Drums, 2x Bass, Clone Hero 1.1's Note Shuffle modifier, the SP cap and more. Each result is saved with its settings, so switching back brings the old results straight back. The search box takes filters such as `artist:`, `charter:`, `stars:7` and `squeeze<=20`. A few of Hydra Deluxe's own judgment calls, such as how much a squeeze-out costs, can be changed in `hydra_rules.ini`.

Hydra Deluxe can analyze your whole library in the background. It then builds a path report of every song's paths, which opens in its own window, where you can sort and search it. It can also compare a player's [dmleaderboards](https://dmleaderboards.com) scores with your optimals.

It reads `.mid`, `.chart`, `.sng` and `.srb` charts, on any difficulty, with or without Pro Drums and 2x Bass.

## Getting started

Hydra Deluxe runs on 64-bit Windows. The installer asks for admin rights, because it installs into `C:\Program Files`.

1. Download the installer (the `-setup.exe` file) from the [latest release](https://github.com/splax77/hydra-deluxe/releases/latest) and run it. It installs to `C:\Program Files\Hydra`, adds a Start Menu shortcut, and installs the Microsoft VC++ runtime if your PC doesn't have it. The installer isn't code-signed, so Windows SmartScreen may warn you. If it does, choose "More info", then "Run anyway".
2. Run Hydra Deluxe from the Start Menu.
3. Click `Manage folders...`, then `Add folder...`, and pick your Clone Hero songs folder (or whichever folder holds the songs you want to add).
4. Click `Scan library`.
5. When the scan finishes, your songs appear in a table. Type in the search box to find the song you want a path for, then click the song. Its panel opens beside the library, and Hydra Deluxe analyzes the song.
6. When the analysis finishes, the paths appear. The first path is optimal. Other paths tied for optimal are listed under the same score. Below them are some of the next-highest scores and their paths, which come in handy if the optimal path is too annoying or difficult.
7. Click a path on the left side of the panel to show its activations on the right side.
8. Use `<` and `>` to step to the previous or next song. To go back to browsing, close the panel with its `X` (or `Escape`).

The [User Guide](docs/UserGuide.md) explains every button and tab, in the order you meet them.

## Learning the mechanics

The public Hydra wiki explains the playing mechanics. They apply to Hydra Deluxe too:

* [Path Notation Quick Reference](https://github.com/DragonDelgar/hydra/wiki/Path-Notation-Quick-Reference)
* [Optimal Checklist](https://github.com/DragonDelgar/hydra/wiki/Optimal-Checklist)
* [Squeezes and early fills](https://github.com/DragonDelgar/hydra/wiki): the wiki's "Detailed Mechanics" pages.

The wiki's own user guide describes the public app's older screen, not this one.

## Good to know

**Paths at an SP cap other than 4 can't be played.** Clone Hero's Star Power meter holds 4 bars. The `SP cap` setting lets you raise that to see what the cap costs a chart. Scores at any other cap are a what-if. You can't reach them in the game.

**Your data stays in the install folder.** Your library, results and settings live next to Hydra.exe in `C:\Program Files\Hydra`. The reports live in their windows and aren't saved as files. Versions before 2.4.0 saved them as web pages in your `Documents\Hydra` folder; the [User Guide](docs/UserGuide.md#old-report-files) says what to do with those.

**Upgrading keeps your library and settings.** Your saved results carry over too. A release that changes how charts are analyzed marks them `Stale` until each song is analyzed again, as 2.1.0 and 2.4.0 did. Open a song or run a batch to bring its result up to date. Version 2.4.0 also dropped the saved results of `.sng` and `.srb` songs, which read `Not analyzed` until you do the same. The first start after an upgrade may take a moment to update your library file. That happens once, and your charts and results are kept. A library file from a version older than 2.2.0 is too old to update: Hydra Deluxe sets it aside as `hydra.db.pre-2.2.0`, starts a fresh library and scans your song folders again. The [User Guide](docs/UserGuide.md#the-first-start-after-updating) shows what you'll see. Uninstalling keeps your data too. To remove it, delete the folder yourself.

**Hydra Deluxe is the app that was called Hydra up to version 1.9.7.** It is the same program under a new name. It installs over that copy and keeps your settings and song folders. Its library file is older than 2.2.0, so it is set aside as described above, and your songs come back as `Not analyzed`. If you're coming from DragonDelgar's public Hydra instead, see [moving from the public version](docs/differences-from-public-hydra.md#moving-from-the-public-version).

## Acknowledgements
- DragonDelgar, who wrote Hydra and whose public version this one grew from
- Onyx, whose drum previewer the Preview's highway is ported from
- Boddy, Beud, and Nick (BongOfDestiny) for active beta testing
- Boddy for reference footage / images shown in the Hydra Wiki
- Drummer's Monthly for being an awesome CH drums community, and for testing some of the first paths ever made by Hydra
- Smidge for all the love and support
