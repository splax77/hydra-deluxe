Hydra Deluxe 2.1.0 reads charts the way Clone Hero does in more places, so some scores and paths change. It also marks where Star Power ends in the Preview, and it says what went wrong when its database fails.

**Analyze your library once after installing.** Every saved result reads Stale, because the engine now stores more about each activation and several scoring rules changed. Click Analyze library once and everything is current again. The first scan after upgrading also reads every chart file once. Re-analyzing the whole library takes about as long as it did in 2.0.0.

**Scores that change, and why.**
- **Disco flip is read per difficulty**, as Clone Hero does. Expert never changes. Hard, Medium and Easy Pro Drums scores change on about 640 charts, mostly up, because charters often mark disco flip only on Expert.
- **2x kicks are read per difficulty too**, and the 2x Bass box works at every difficulty, like Clone Hero's Double Kick.
- **A Star Power phrase pays where Clone Hero pays it.** About 20 Expert charts lose points the game never gives. Muse - Feeling Good loses the most.
- **Chart dynamics count only when the chart turns them on the way Clone Hero reads it.** When the switch comes after some marked notes, the Dynamics tab says from what time on they count.
- **Drum fills land as in Clone Hero.** Each fill finds its chord on its own, with Clone Hero's landing window. A few charts' fill counts change, and two Thrice charts gain a few hundred points.
- **Squeeze scales are measured from the true Star Power end**, including when the SP cap clamps the meter. A path no longer shows a squeeze-in that can't happen, and Hydra no longer drops a path that gets a fill in favour of one that misses it. At cap 2, seven charts' best scores go up.
- **Hormone's Echo (Artificial Brain) now analyzes.** Its drum track has a second name, and Hydra now reads the one Clone Hero uses.

**The path list.** Paths that tie the best score always show, even over your Path limit. A required early fill counts against the Path limit like a squeeze. The "Best all-0 path" works as it did in 2.0: every squeeze and early fill on it is 0 ms or easier.

**The Preview.** A magenta edge and a small triangle beside each rail now mark where each Star Power window ends, so a gem can't hide it. The multiplier disc and the score box show the plain multiplier once Star Power stops paying. A chart that changed since it was analyzed opens with one line saying so, and its path hidden. Jumping back after the song ends brings the sound back.

**Song length comes from the chart.** The timeline and the Preview take a song's length from its song.ini, .sng or .srb, or from its last Expert note when none is stated. No audio is opened for it, and the timeline now includes the song's outro.

**Counts agree.** The batch, the library and the reports all count every copy of a chart, so "analyzed" plus "failed" adds up to the library's total. A chart in several folders is still analyzed once.

**Errors say what happened.** If hydra.db can't open at startup, Hydra shows a message instead of closing silently. A failed save during a batch lists that chart as failed and the batch carries on, instead of closing Hydra. A failed database read says so in the status line.

**Smaller changes.** Scan and Rescan wait while a batch runs. Leaderboard scores played at another scroll speed get their own "other speed" status and leave the Matched and Points-left totals. Report pages gain a "No paths" filter. A hand-edited setting outside its range is pulled to the nearest allowed value, and a `#` comment after a value is ignored.

**Opening your database in an older Hydra.** 2.1.0 adds a column to hydra.db. An older Hydra that opens it afterwards refuses to save, with an error, rather than writing a wrong row.

**Command line.**
- `hydra_batch` with no folder arguments now saves its scan as the library, like Scan library. Its "Store now holds" line is gone.
- `hydra_batch`, `hydra_report`, `hydra_fillcompare` and `hydra_bench` print the error and exit non-zero when the database fails.
- `hydra_replay` refuses a typed SqOut offset that sits exactly halfway between two chords, and names both.
