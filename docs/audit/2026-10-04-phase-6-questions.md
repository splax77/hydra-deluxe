# Questions for phase 6 (2026-10-04)

These are the four decisions phase 6 needs from you before it starts. Phase 6 is the duplicate folds: the same fact worked out in two places gets one owner. Almost all of it is code-only and changes nothing you can see, so it goes ahead on my recommendation and I report what changed. Only these four touch something stored or shown, or ask you to leave a known gap alone.

Each question says what you see today, what you would see after, and what I recommend. You can answer "all recommended", or "all recommended except 3", or reply to any number with your own call. Your answers get the next free D numbers in `docs/audit/2026-10-03-fix-decisions.md` (D52 is already taken by phase 3). The code-only design calls are listed in the plan under "Recorded as recommended" and get one D number with them.

Three questions the drafts raised are no longer ours. Whether the "Best all-0 path" section should hide only duplicates (243, 324) and whether multiplier squeezes should be listed on 4- and 5-note chords (335) now belong to phase 7's E3 and E2, which own those rules and asked you in their own sheet.

## 1. The database's schema number (finding 298; record)

**Today:** every time Hydra opens its database it writes the number 3 into a slot in the file called `user_version`. Nothing in Hydra ever reads that slot back. When the database needs upgrading, Hydra decides what to do by asking which columns the tables have, not by reading the number. Two tests check that the number is 3.

**After, option A:** Hydra stops writing the slot. A new database carries 0 there, an existing one keeps its 3, and nothing on any screen, report or stored result changes. The two tests and the "schema user_version 3" wording in the file header go.

**After, option B:** the number becomes the real gate. It moves into `stored_versions.h` beside the other stamps, the column checks are deleted, and upgrades run by comparing numbers.

**Recommended: A.** The column checks already own the question and have run every upgrade so far. Making a number the gate would skip the checks on any file whose number is wrong, and nothing outside Hydra reads the slot. Phase 6 touches no other stored field.

## 2. The Score range box's width (finding 112; display)

**Today:** the settings bar sizes the Score range box to fit the text "000000", six zeros. In the shipped font an 8 is about one percent wider than a 0, so six 8s do not quite fit by the same rule. Frame padding hides the difference, and you cannot see it.

**After:** the box is sized by `widest_digits(6)`, the same rule the Analyze button and the Preview already use. It grows by about half a pixel, times your DPI scale. You will not be able to tell.

**Recommended: fold it.** One rule for "how wide can N digits be" means a font change can never clip the box, and it removes one of the two width formulas the audit found outside the widgets file.

## 3. A taken fill's lane colour when two taken fills touch (findings 76 and R7.14; display)

**Today:** in the Preview, each instant along the highway decides which pad colour lights the fill lane. The draw code then colours a whole merged span with the colour of the first instant inside it. The two agree whenever a span holds one fill, which is every case anyone has been able to show. If two taken fills ever touched or overlapped (a fill starting on a half tick, or a second activation while the first's Star Power still runs), the second fill's lane would light in the first fill's colour.

**After:** each span carries its own pad, so the second fill would light its own lane colour. In the one-fill case, which is every chart in your library, nothing changes.

**Recommended: take the owner's answer.** It is what the engine says the second fill is, no library record reaches the case, and the fold removes the last place the draw code works out a rule the track state already knows.

## 4. The Preview's xN disc at the Star Power end (finding 1's second half; a gap to leave)

This one is not a fold. It is a known gap I propose to record and leave, and you can override that.

**Today:** the Preview's multiplier disc shows the multiplier the last hit chord earned until the next chord comes. So right after Star Power ends, the disc still reads doubled for a moment while the drain box already says idle. D2 settled when the disc doubles (only when Star Power paid the chord); it did not settle this gap.

**After, if left:** the same as today. The gap gets one sentence in the decision record so the next audit stops asking.

**After, if changed:** the disc would drop to the plain multiplier at the Star Power end, before the next chord. That needs a new "time inside the window" rule for the disc, not a fold of an existing one, and the game's own disc has never been checked against video for this moment.

**Recommended: leave it, as a known gap.** Changing it would invent a rule nobody has verified against Clone Hero. If you want it checked, it is a video question first, then a phase 7-style task.

## Nothing else changes

No other finding in phase 6 changes any text, number, colour, layout, stored field, stamp, score or path. The folds that name an existing value (two Star Power bars to activate, the default depth of 4) keep the value and only give it a name. The two developer tools whose words change, `hydra_bench`'s header and `hydra_replay`'s raw MIDI error, count as code under your rule, and the plan lists them.
