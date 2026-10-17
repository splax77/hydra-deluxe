Hydra Deluxe 2.9.2 reads the tracks of a MIDI chart the way Clone Hero does. The Paths tab now opens with everything on its right side folded.

**Your saved results carry over.** Nothing reads `Stale` after the update. A `.mid` chart that this release reads differently keeps its old result until you click it, which analyzes it again, or until its file changes.

**MIDI charts read their tracks the way the game does.** Clone Hero never reads the first track of a `.mid` chart as an instrument, so a chart whose only drum track is its first track has no drums in the game. That is the usual case for a one-track `.mid` file. Hydra Deluxe used to read drums from that track. Such a chart now reads `Can't analyze`, with a reason like `No Expert Pro Drums notes in this chart.` Practice sections now come from the first track named `EVENTS`, in any mix of upper and lower case, and from no other track, as in the game.

**The Paths tab starts folded.** When you open a path, every activation row starts closed, and so does each row's `Backend timings` fold. `Multiplier squeeze` and `Score breakdown` start closed too. Click a row or a fold to open it. Those last two stay the way you left them when you open another song. The path list on the left still starts open.

**The backend table has no lead-in.** The gray paragraph above each `Backend timings` table is gone. The User Guide now says what the `Timing` and `Points` columns mean.
