# Questions for phases 3, 4 and 5 (2026-10-04)

**Answered 2026-10-04: "all recommended".** Each question's Recommended line is the decision, recorded as D48 in `docs/audit/2026-10-03-fix-decisions.md`.

These are the decisions phases 3 to 5 need from you before the display work starts. Each one changes something you can see in the app, a report page or a command-line tool you use. Nothing here changes a stored record or a score.

Each question says what you see today, what you would see after, and what I recommend. You can answer "all recommended" or "all recommended except 4 and 17", or reply to any number with your own call.

Code-only work (refactors, scan rows, ADR and comment fixes, dev-tool output such as hydra_replay, hydra_bench and hydra_uitest) is not listed. It goes ahead on my recommendation and I'll report what changed.

All 49 display findings and 41 of the 45 docs findings are still open at main (50e4b0b). Four docs findings were already fixed by steps 1 and 2.

## A. Paths tab and squeeze numbers

**1. Two paths tie for the top score: are both "optimal"?** (finding 7, weighty)
Today the Paths tab and the Preview call both of them Optimal (gold). The report's "Best path only" view keeps only the first one and bolds only it.
After: one answer everywhere.
Recommended: both are optimal. A drummer gets the same score playing either, so the report lists the tied path too.

**2. How does a squeeze timing print?** (4, 18, 36, 154)
Today one squeeze can read "13 ms" on the activation badge, "12 ms" when you copy the path, and "12.6 ms" on the path button. The report page can round an exact half differently from the app.
After: whole-ms text rounds to nearest everywhere, so the badge and the copied text agree. One-decimal text is unchanged. The report prints the app's own string.
Recommended: yes. When a squeeze and an early fill tie exactly, the badge keeps naming the squeeze, as today.

**3. A timing exactly on a tier edge.** (34, 94)
Today a path at exactly 2.0 ms has no orange warning on the Paths tab but reads Hard in the report. A gap of exactly 170.0 ms reads Beyond in the report but has no "over budget" line in the details.
After: both places agree.
Recommended: a timing on the edge counts as inside, so 2.0 ms is not difficult and 170.0 ms is Insane+. That's how the Paths tab has always read it.

**4. The ±10 ms band on backend rows.** (306, weighty)
Today a backend row turns "Easy SqOut" at +10 ms and "Easy" at -10 ms, whatever your hit window or leeway.
After: either the 10 stays as a named number you chose, or it follows the leeway setting.
Recommended: keep 10 ms as a named number. Players compare these labels with Clone Hero, and nothing in the game ties them to the leeway.

**5. The report footer says its tiers match the Paths rows.** (2)
Today a 5 ms SqOut reads "Hard" in the report and "Standard SqOut" on its Paths row, under a footer saying the two match.
Recommended: reword the footer. The row label measures distance from the SP end, both early and late; the report tier measures size. They answer different questions.

**6. Uncounted Star Power notes in the backend rows.** (33)
Today a phrase note the path doesn't count shows 0 points and a label like "Free SqOut", with no "(uncounted)" tag.
After: "Free SqOut (uncounted)", like the plain rows.
Recommended: yes.

**7. The report's "Tightest squeeze" tile.** (35)
Today the tile can show an early fill's timing under the word "squeeze".
Recommended: rename it "Hardest ms" to match the column.

**8. Orange outline on the Paths timeline.** (3)
Today the timeline outlines any badged activation in orange, even a 1 ms squeeze-in or an early fill whose row badge is grey.
Recommended: orange only when the row itself is orange (difficult).

**9. "Bars" on an activation row.** (93)
Today a row says "2 bars" when you banked 2 bars at the activation. A phrase collected during Star Power can make it last as long as 3 would.
Recommended: keep the number and fix the words: "bars banked when you activate". That is what your meter shows when you hit the activation, and the gauge already shows the spend.

**10. Early-fill badge on an activation that skips fills.** (305)
Today an activation with no required squeeze can show a grey "early fill 20 ms" badge while its path button shows no timing. The 2026-09-27 plan said there should be no badge.
Recommended: keep the badge and record the choice. That timing decides whether the first fill appears, which is what the skip count means.

**11. Drum note names.** (17)
Today a green tom is "GreenTom" on the Paths tab and in the Preview box, but "Green tom" on the Dynamics tab. With Pro Drums off, the Paths tab says "YellowTom" where Dynamics says "Yellow".
Recommended: use the Dynamics wording everywhere ("Green tom", "Yellow cymbal", "2x kick", plain "Yellow" with Pro Drums off).

## B. Library, song panel and settings bar

**12. Counts next to nouns, and commas.** (14, 99)
Today SP cap 1 reads "1 bars" in the song panel, the report subtitle and hydra_batch. "Within 1000 scores" sits next to "1,000 scores", and the leaderboard picker can say "1 scores". In a German-language browser the report pages print 1.234 under a subtitle saying 1,234.
Recommended: one rule everywhere: singular at exactly 1, commas from 1,000, whatever the browser's language.

**13. One name for each fill rule.** (55)
Today the old rule is "Clone Hero 1.0" in the batch confirm, "Clone Hero 1.0 (legacy)" a few lines later, "Clone Hero 1.0 fills" in the report subtitle and "CH 1.0" in the fill comparison.
Recommended: "Clone Hero 1.0" in sentences and "CH 1.0" in narrow columns, both from one place. "(legacy)" goes. The checkbox stays "1.0 fills".

**14. Song titles with Clone Hero tags.** (8, 111)
Today a title with `<b>` or `<size>` tags shows the raw tags in all three report pages. A bare `<color>` tag shows raw in the Library, the busy tooltip and the batch strip. A title that is nothing but tags is blank in the Library and "(unknown)" in reports.
Recommended: one cleaned title everywhere, and "(unknown)" when nothing is left. This restores your earlier rule (D10, D22, D27).

**15. Analyze button with a filter that doesn't parse.** (19)
Today typing `stars:9` (not a valid filter) makes the button read "Analyze search (N)" with the whole library's count.
Recommended: it reads "Analyze library" until a search really narrows the list.

**16. Song panel after a batch.** (123)
Today, if the panel is open on a chart a batch just finished, it keeps saying "Not analyzed yet" or "Out of date" until you click away and back.
Recommended: it shows the new result straight away.

**17. "Out of date" wording, and the settings bar.** (13, R7.4)
Today "Out of date" is worded three ways and always blames both possible causes. The settings bar says "this song analyzes" even after you've moved to a different song.
Recommended: one sentence that names the real cause ("analyzed by another Hydra version" or "different rules in hydra_rules.ini"). The bar names the song that is analyzing.

**18. How a long label is cut.** (70)
Today a cut title in the Library ends in " …" with the space kept. It is cut one character earlier than the same text in the Preview's path picker, which ends in "…" with no space.
Recommended: the picker's rule everywhere (no trailing space, an exact fit allowed).

**19. A progress bar at 0 of 0.** (69)
Today the scan dialog draws 0 of 0 as full, the batch strip draws it empty, and the Preview loader sits at 8%.
Recommended: empty, meaning "nothing reported yet". The scan's Done line already says "0 charts found" in words.

**20. Time text.** (318, R7.1)
Today the Preview clock can read 0:60.000 for half a millisecond before each minute. The batch strip says "about 1:30 left" while the Preview loader says "about 2 min left".
Recommended: the clock reads 1:00.000. Both use the batch strip's "about m:ss left" form, and the Preview keeps its 3-second wait before showing it.

**21. Two error messages.** (R7.20)
Today a file-size read failure during Analyze says "Something went wrong.", and a Preview mixer failure prints raw text ("Preview failed: StreamMix: ...").
Recommended: the first says the file may have been moved, like other missing-file errors. The second uses the audio-decode sentence.

## C. Preview

**22. Reload the Preview when the chart mode changes.** (20)
Today, with a song open on the Preview tab, switching difficulty, Pro Drums or 2x Bass keeps the old mode's notes on the highway. The path, gauge and score box already come from the new mode.
Recommended: the Preview reloads with the new mode's notes, or says "No Hard Pro Drums notes in this chart." The settings-bar help already promises this.

**23. The time box half a tick before a tempo change.** (57)
Today, for up to half a tick before a change, the meter, section and drain box already show the new values while the BPM line still shows the old tempo.
Recommended: all four switch together. Only playback or scrubbing can land there.

**24. Gauge cap for a song with no result yet.** (308)
Today the gauge pins at 4 bars whatever your SP cap setting says.
Recommended: it uses your Settings cap, the one the next analysis will run at.

**25. Where the song ends.** (9, 329)
Today the Paths timeline ends at the last note. The Preview scrubber ends at the end of the audio. So an activation sits further right on the Paths timeline than on the scrubber whenever the music outlasts the notes. The beat lines stop two measures after the last note.
Recommended: both bars measure position against the last note, so an activation lines up in both. The scrubber still lets you play the audio tail, and the beat lines run to the end of the audio.

**26. Preview colours.** (16, 136, 218)
Today the highway floor is teal while Star Power runs, but the drain box text is gold at the same moment. The scrubber's activation marks use one gold (250,210,0) and the next-activation header another (255,204,51).
Recommended: teal means "SP running" everywhere, so the drain box text turns teal like the floor (the Onyx look, ADR 0008). Activation marks and the header both use the best-path gold.

**27. A note exactly on the playhead.** (15)
Today, after "< Act" or "Act >", the score box already counts the chord under the playhead while the highway shows it unlit.
Recommended: it counts as hit in both, so a jump lands on a struck gem.

## D. Report pages

**28. What an empty report says.** (105)
Today hydra_report and the app say "No records stored yet, run hydra_batch first", even right after a batch stored records under a different cap or fill rule.
Recommended: when records exist, it says "Nothing is analyzed under these settings (SP cap 8, Clone Hero 1.1 fills). Analyze with these settings, or change them." Today's sentence stays only for an empty database.

**29. Leaderboard and fill-comparison labels.** (309, 312)
Today a leaderboard score 50,000 under optimal wears a "Matched" chip (it means "found in your library", not "equals optimal"). In the fill comparison, a chart whose 1.0 record has no paths is listed under "Only in 1.1 db".
Recommended: "Under optimal", "At optimal" and "Above optimal" chips, with the summary line to match. Fill-comparison rows are labelled by which database holds a record.

**30. Percentages.** (302, 51)
Today the Dynamics tab cuts decimals off, so 12 of 453 kicks reads 2% and 999 of 1000 reads 99%. The leaderboard page can say 100.00% for a score 10 points short of optimal.
Recommended: round to nearest everywhere (3%, 100%, and 99.99% for the near miss).

**31. Searching on report pages.** (67)
Today typing "beyonce" on a report page misses "Beyoncé". "beyonce halo" misses a row that has one word in each field. The Library finds both.
Recommended: the pages search the way the Library does.

## E. Guide, tooltip and command-line wording (one batch)

**32. Make the text say what the code already does.** (6, 39, 40, 41, 42, 43, 91, 104, 106)
Each item fixes text that describes something the app doesn't do:
- The Avg multiplier tooltip says it's a ratio, not points per note.
- The guide's drain-box line says "at the current tempo".
- The early-fill line says a positive number means you must hit early.
- The backend line names the leeway setting instead of "3 ms".
- The dynamics line says untagged MIDI charts show no ghost or accent counts.
- The overfill line says the warning also needs a listed squeeze.
- The report pages say "every mode at the current cap, top N per chart and mode".
- hydra_batch's two refusals give the real reason for the guard.
- hydra_batch's closing line says "rows for every setting", so it stops looking like a different count from the report's.

Recommended: approve all nine.

## F. Numbers to keep as they are and write down

**33. Confirm today's numbers so they get recorded.** Nothing changes on screen if you say yes. Each gets one sentence in CONTEXT.md or its ADR, so the next audit stops asking about it.
- The path limit starts on at 10 ms, and "Hide backend rows beyond" starts off with 50 ms in the box (R7.31). Changing these would change which paths new analyses keep, so they're listed first.
- An average multiplier of 0.000x for a path with no scoring notes, which never happens in real charts (310).
- The report's tier ladder: 2 ms, half a hit window, one window, one and a half, two, then Beyond (333).
- UI timings: 0.5 s "Done!", 2.0 s "Copied!", 0.15 s search refilter, 1.0 s batch refresh (R7.33).
- Two path cut-offs that answer different questions: 248 characters, past which file calls need the long-path prefix, and 260, the most the Windows shell accepts. Both go in ADR 0020 (R7.12).
- A chained Opus file plays only the links that match its first link's channel count; the ADRs get corrected to say so (R7.39).
- Renderer, audio and leaderboard tuning values (327, 336, 337, 339, 348, 349, R7.32, R7.34, R7.36):
  - star cutoffs multiplied as 32-bit floats, as the game does;
  - Preview spans ending half a tick past their last note;
  - the half-millisecond "on an activation" slack;
  - the rules fingerprint format;
  - the loading bar's shares and pacing;
  - the audio reader limits;
  - 16 parked lookups;
  - the engine's progress step.

Recommended: confirm all.
