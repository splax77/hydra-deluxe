Hydra Deluxe 2.10.2 shows accented letters in song names correctly. Before, some names showed a `�` and lost the characters after it.

**Your saved results carry over.** Nothing reads `Stale` after the update, and nothing is analyzed again. Scores and paths don't change.

**Names with accented letters read correctly.** Some charts save their `song.ini` in an older Windows text format instead of UTF-8, the format Hydra Deluxe expected. In those files, a letter such as `á` or `ö` showed as `�`, and up to three characters after it disappeared, so `Szamár Madár` read `Szam�Mad�`. Hydra Deluxe now checks whether a file is valid UTF-8. When it isn't, Hydra Deluxe reads the file in the older Windows format instead. Clone Hero itself is closed source, so the rule follows YARG, an open-source game that reads the same chart files. Song names in `.chart` files, MIDI files, `.sng` files and `.srb` bundles follow the same rule. A file saved as UTF-16 is now read too.

**Click `Scan library` once to fix the names you already have.** Your library keeps the names from your last scan. The first scan after the update reads every chart again, so it takes longer than a usual scan. Later scans are fast again. Search finds the fixed names too, so `aenema` finds `Ænema`.

**One rare chart reads differently.** A disco flip marker in a `.chart` file with an accented letter where its difficulty letter belongs no longer turns disco flip on. A MIDI chart already worked this way.
