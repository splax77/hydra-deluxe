# Questions for phase 7 (2026-10-04)

**Answered 2026-10-04: "all recommended".** Each Recommended line is the decision, recorded as D51 in `docs/audit/2026-10-03-fix-decisions.md` with the user's notes on 4- and 5-note chords and on the 60 ms early-fill window.

These are the decisions phase 7 needs from you before its code starts. Each one changes something you can see in the app, a stored record, or a score. Each question says what you see today, what you'd see after, and what I recommend. You can answer "all recommended", or "all recommended except 4 and 17", or reply to any number with your own call.

Code-only work goes ahead on my recommendation, and I'll report what changed. That covers refactors, scan rows, ADR and comment fixes, and dev-tool output (hydra_replay, hydra_bench, hydra_uitest, the Clone Hero probe scripts). Those calls are listed at the end, so you can see them.

Phase 7 is the audit's step 7: 75 findings. Rechecked at main (2ab1383), 6 are already fixed and 69 are open. Three questions (1, 3 and 4) change stored results. Those changes ride the re-analysis that "2.1.0" already forces, since no release carries 2.1.0 yet. Before those merges, I'll show you how many charts each one changes, as we did for D36.

## A. Which paths the search keeps, and their numbers

**1. With the tie limit at 4, may Hydra keep 8 tied top-score paths?** (finding 95, changes results)
Today the tie limit counts separately for paths inside your Path limit and paths over it. So "keep 4 tied paths" can keep 4 of each, which is 8.
After: one count per score. When some tied paths are inside the limit and some are over, the inside ones come first.
Recommended: one count per score, as the User Guide already promises. Fewer tied paths are stored, so the Optimal group can get shorter on charts with many ties.

**2. A path that ties the best score needs a squeeze over your Path limit. Is it shown?** (330)
Today it is shown, in orange, and the guide doesn't say so. Example: "I Am... All Of Me" keeps a second optimal path with a 78.9 ms squeeze at a 10 ms limit.
Recommended: keep showing it, and the guide gains one sentence. The best score is what you asked for. The orange figure already warns that it's hard.

**3. Multiplier squeezes on 4- and 5-note chords.** (49, 50, changes results)
A multiplier squeeze is a chord that crosses a multiplier step, so the order you hit its notes changes the score. Today Hydra lists only 2- and 3-note chords, plus 4-note chords at combos 7, 17 and 27. It never lists a 5-note chord. On a listed 4-note chord, the points shown are too low: "+15 pts" where hitting it right really gains +30, because two notes cross the step.
After: every chord that crosses a step is listed, whatever its size, and the points are the real gain. The how-to line names every note that crosses. Scores don't change. Only the Multiplier squeeze list and its total do.
Recommended: yes to both.

**4. A path whose squeezes are all free: does it show a timing figure?** (22, may change results)
A free squeeze is a squeeze-in at exactly the SP end; it needs no timing. Today such a path still shows a figure like "-163.0 ms" on its button, its badge and the report's Hardest column. The guide says nothing is shown.
After: no figure for a path with nothing to time. A squeeze-out at exactly 0.0 ms still counts as needing timing (D13 already decided that), and the all-0 list must follow D13 too. Today the all-0 list can keep a path whose only squeeze is a 0.0 ms squeeze-out.
Recommended: yes. Before merging, I'll show you which charts' all-0 lists change.

**4b. One duplicate hides the whole "Best all-0 path" section.** (324, from phase 6)
Today, if one all-0 variant matches a path already listed above it, the Paths tab hides the whole "Best all-0 path" section. Any other all-0 variants that aren't duplicates disappear with it.
Recommended: hide only the duplicate, and keep the other variants visible.

**5. Does a required early fill count against the Path limit like a squeeze?** (23)
Today it does. A path that needs a fill hit 30 ms early is treated like a 30 ms squeeze by the Path limit and the squeeze<= filter. But the guide and the Path limit tooltip say "hardest squeeze".
Recommended: keep the rule, since an early fill is still a timing you must hit. Fix the words to "hardest squeeze or required early fill".

**6. A phrase that fills the SP meter exactly to the cap: is that an overfill?** (307)
Today it isn't. The meter is full, nothing is lost, and no overfill note appears. No decision records this.
Recommended: keep, and record it.

**7. The early-fill window is 60 ms.** (313)
This is how early SP can be ready before a fill and still bring the fill. 60 ms comes from the library survey: the hardest early fill on 18,773 charts was 57.7 ms. No decision records it.
Recommended: keep 60 ms, and record it with that reason.

## B. Stored results and the library

**8. Results made under other rules: deleted, or kept for when you switch back?** (65, weighty)
Here's what happens today. You edit hydra_rules.ini, re-analyze three songs, then undo the edit. Those three songs read "Not analyzed", and their results at every other cap are gone too. ADR 0014 and the User Guide both promise the old results come back.
After: results under other rules stay in hydra.db and read Ready again when the rules match. The database keeps one more row per rules setup you've tried.
Recommended: keep them, as the docs already promise.

**9. Song length per difficulty.** (62)
Today Hydra stores one length per chart. If you analyze Expert and then Easy, the Expert timeline ends at Easy's last note, and late activation dots pile up on the right edge.
Recommended: store a length per difficulty, so each timeline ends at its own last note.

**10. One chart file in two folders.** (63)
Today the copy that names the chart flips. A rescan shows the first folder's song.ini names. An analysis shows the names of whichever copy was analyzed last. A batch also analyzes the chart once per folder.
Recommended: the first copy the scan lists always names it, and a batch analyzes each chart once. The batch confirm's count drops by the number of duplicates.

**11. A Ready chart that kept no paths.** (88)
Today it sits under the Analyzed chip, but `stars:` and `squeeze<=` never list it, and the leaderboard page calls it "not analyzed". Nobody has seen a real analysis produce one.
Recommended: it stays Analyzed, and the filters treat it as having no facts. The leaderboard and fill pages say "no paths".

**12. The stored tempo map and the scan cache follow reader changes.** (343, 345)
Today the tempo map is stored once, the first time a chart is analyzed, and is never refreshed. The rescan cache also reuses chart names and hashes forever. So after a reader change, old charts keep old measure labels, or old titles, until the file changes on disk.
After: each analysis rewrites the tempo map. The scan cache gets its own version stamp, so a reader change refreshes every chart on the next scan. Nothing you see changes today.
Recommended: yes to both.

**13. Record these as they are.** (129, 317, 321, 323, 325, 335)
Each of these is today's behaviour with no decision behind it:
- The scoring values Hydra took from Clone Hero: a cymbal is worth +15, a solo pays 100 per note, and the multiplier steps up at combos 10, 20 and 30.
- The library's star count comes from the stored summary, which a re-analysis refreshes.
- Old Auto results made with a custom cap ladder read Stale until re-analyzed. Only default-ladder ones are deleted at first start.
- The scan reads a .sng or .srb's title and artist from its first 1 MB.
- `squeeze<20` means "at most 20".
- The six lower bounds in hydra_rules.ini. One example: max_tied_paths must be at least 1. The User Guide will list all six.
Recommended: record all five as they are.

## C. Settings

**14. A hand-edited setting outside its range.** (322, 311, 31, 56, 72, 138)
Today a bad value in hydra_settings.ini is handled four different ways. Here's what each one does now:
- `depth_value=-1` crashes the search.
- `mslimit_value=900` searches at 900 ms, then snaps to 500 on your first click, and that's a different record.
- `backendlimit_value=-30` shows -30 and hides rows past 30 ms.
- A typed volume of 150% plays at full volume, then comes back as 40% next launch.
After: every number setting has one allowed range, the one its box already enforces. A value outside it is pulled to the nearest edge when the file loads, and when you type it.
Recommended: clamp, matching the boxes. The volume range becomes 0 to 100 everywhere, including Ctrl+click typing.

**15. Two small settings-file changes.** (66, 140)
A `# comment` after a value in hydra_settings.ini would be ignored. Today `view_difficulty=Hard # practice` silently becomes Expert. The hit window could also hold a decimal like 85.5, where today it's cut to 85.
Recommended: yes to both.

**16. May the SP cap be 1 bar?** (139)
A 1-bar cap can never activate SP, so every path comes back with no activations. Today it's allowed, and nothing explains why there are no activations.
Recommended: keep it allowed as a what-if, and add one line on the Paths tab: "A 1-bar cap can never activate Star Power."

## D. Preview

**17. Jumping back after the audio has ended.** (73)
Today, on a chart whose notes outlast the audio, jumping back while playing leaves the highway scrolling in silence with the play button lit.
Recommended: the sound comes back.

**18. The chart file changed since the library scan.** (126)
Today the Preview draws the stored path on the new notes, so activations can land on the wrong notes.
Recommended: open the chart, hide the path overlay, and show one line: "This chart changed since it was analyzed. Analyze it again to see its path."

**19. A FLAC whose header says its length is 0.** (R7.8)
The FLAC spec says 0 means "length unknown". Today such a stem is silent for the whole song, and if it's the only stem the Preview has no sound at all.
Recommended: read the file once on open to count its length, so it plays. Every other file keeps today's fast open.

**20. A damaged Opus stem after you scrub back.** (R7.9)
Today a damaged Opus stem goes quiet at the damage and stays quiet after any scrub back, until you reopen the chart. Every other format comes back when you scrub to before the damage.
Recommended: Opus behaves like the others.

**21. Which files count as song audio.** (74, 101)
Today a loose folder's preview.ogg is left out of the mix, but a .sng's preview.ogg is mixed into the song. A .srb with an unusual Ogg or WAV stream before its real audio plays silent. No library chart has either case.
Recommended: preview clips are left out everywhere, and both checks use the one "is this playable audio" rule.

**22. The SP gauge on the exact tick of a mid-measure time-signature change.** (58)
Today the gauge's drain line has a tiny kink there, because it measures that tick under the new time signature while the engine uses the old one. The end point doesn't move.
Recommended: the gauge uses the engine's side, so the kink goes.

## E. Library, batch and reports

**23. "Open automatically".** (R7.2)
Its hint says it opens finished batch reports, but it also opens every leaderboard comparison.
Recommended: keep the behaviour and fix the words to "Open each report in your browser as soon as it's built."

**24. Scanning during a batch.** (R7.3)
Today the toolbar's Scan button is off during a batch, but "Scan now" and "Rescan library" elsewhere still start a scan. That scan's window then covers the batch strip.
Recommended: block both during a batch, with the status line saying "A batch is running."

**25. Plain words for every error.** (193)
Today a missing or damaged Preview asset shows raw program text. Other errors become plain sentences only when their exact wording matches a list, and that list keeps falling behind.
Recommended: every error carries its kind, and each kind has one plain sentence. A Preview asset problem reads "Reinstall Hydra".

**26. Batch counts.** (142)
For one frame after a failed chart, "N analyzed" reads one too low.
Recommended: the batch reports its own analyzed, skipped and failed counts, so the numbers always arrive together.

## Code-only calls I'll make unless you say otherwise

These change only code or dev-tool output, so they go ahead on my recommendation:
- **hydra_replay:**
  - `dump --legacy-fills` reads the stored 1.0 result when there is one (103).
  - It takes its defaults from the app's settings (198).
  - Its on/off flags take 0 or 1 only, as its help says (68).
  - It keeps resolving an old dump's squeeze-out by its offset (38).
  - An empty record's best score reads null in its JSON and "-" in text (98). hydra_bench does the same.
- **hydra_bench:** `--dump-db` writes relative paths like `--scan --dump` does (108).
- **hydra_report:** `--paths 200000000` says "top 200,000,000 paths" and not "every path" (86). Its default output stays the current folder; the file name comes from one constant (202).
- **hydra_uitest:** `wait-idle` waits for every background job (109).
- **Leaderboard page:** its "SP cap 4" text comes from Clone Hero's cap constant (170).
- **Probe scripts:**
  - The six old diagnostic scripts are deleted: hit_detect, find_clock3, poll_windows, find_engine, milestone2 and test_attach. walk_edges and watch_window replaced them, and test_attach can kill Clone Hero (78, 83, 84, 174).
  - The runners share one wait loop that gives up after 5 s of a frozen clock. The edge runners fire at the target time. play_chart keeps its 2 ms early press as a named constant, so its playing doesn't change.
  - They share one start-cursor rule: skip notes less than 150 ms ahead.
  - They share one hit-offset formula and one window-edge finder (79 to 82).
  - The 2x kick stays on L, since either pedal hits either kick (85).
- **play_chart.py:** it reads the chart through `hydra_replay dump` instead of its own parser, so it plays exactly the notes Hydra analyzed (59).
