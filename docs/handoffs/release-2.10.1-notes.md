Hydra Deluxe 2.10.1 shows drum fills in the Preview again on charts whose fills Hydra Deluxe places itself. It also fixes two ways a broken chart file could hang or crash the app.

**Your saved results carry over.** Nothing reads `Stale` after the update, and nothing is analyzed again. Scores and paths don't change.

**Generated fills light the lanes again.** When a chart has no drum fills of its own, Hydra Deluxe places them the way Clone Hero 1.1 does. Since 2.8.0, such a fill starts on its measure line, so a fill whose chord sits right on that line has a length of zero. The Preview drew nothing for those fills. On many charts the Preview showed no fill at any activation, and an activation looked like it happened on nothing. Now the Preview lights the lanes for the half measure before each generated fill's chord, as it did before 2.8.0. The fill timing that decides whether a fill appears is unchanged. Fills written into the chart, and fills placed under `1.0 fills`, draw as before.

**A chart with a note placed absurdly far into the song no longer hangs Hydra Deluxe.** Placing fills for such a chart used to step through every empty measure up to that note, which could take tens of millions of steps. It now skips empty measures. The fills placed are the same.

**A MIDI file cut off in the middle of a track header is read safely.** Hydra Deluxe used to read past the end of such a file, which could crash it. The cut-off track is now ignored, and the tracks before it still load.
