# Note Shuffle Design Plan (2026-10-08)

**Status:** draft for the user's review. No code is written and nothing is dispatched until the user answers the questions at the end and the four open questions in Part 4 are settled.

**Goal:** Hydra can score a chart the way Clone Hero 1.1 plays it with the Note Shuffle modifier on. Each drum setting gets its own shuffled chart, its own best path and its own stored result. With the switch off, every result reads exactly as it does today.

**Sources:** the memory note `ch-note-shuffle-algorithm` and the two Python references in session 6c19d0ef's scratchpad (`scout-strings\shuffle_ref.py`, `scout-flag\shuffle_ref.py`). Both still exist as of 2026-10-08. Clone Hero code facts come from GameAssembly.dll v1.1.0.6142. The Il2CppDumper output is the folder named in `docs/audit/ch-evidence.md` line 7.

---

## What the game does, in plain words

Note Shuffle moves each drum note to a new pad. Kicks never move. It looks random but isn't: the same chart always shuffles the same way.

Think of it as a deck of cards shuffled with a seed written on the box. The seed is built from the chart's first four notes: for each one, multiply its lane by its tick and add them up. The shuffler is a standard random-number generator called xorshift128+, started from that seed.

For each pad note, the game draws a random pad. It throws the draw away and redraws if the pad's colour is already taken. "Taken" means used earlier in the same chord. For the first note of a chord, it means used by the whole previous chord. Colour counts in both forms: a yellow tom blocks the yellow cymbal too. With Pro Drums on, the draw can land on any of the seven tom and cymbal pads. With it off, only the four toms.

There is one exception. If a chord has the same shape as an earlier one, it copies that chord's output instead of drawing. "Shape" means the same set of pads. The first note of a chord compares with the chord two back. Later notes in the chord compare with the chord one back. That is why a run of single notes on one pad comes out as an alternation between two pads.

The shuffle runs after the game has already applied flams, dropped 2x kicks (when 2x kick is off), applied disco flip, and folded cymbals into toms (when Pro Drums is off). So the input to the shuffle differs per drum setting, and so does the output.

Ghost, accent, Star Power and solo marks stay on the note as it moves. Scoring reads the new pads. A note that lands on a cymbal is worth +15, so with Pro Drums on the score changes.

## What the evidence already shows

Two scouts read the game code independently and wrote the two references. Both reproduce the user's gameplay video ("turkish man yelling meow at an egg", a 480-resolution .mid) at 22 of 22 ticks. The extra 180 points in that video came only from toms that turned into cymbals.

On 2026-10-08 I ran three more checks against that same video, using the references. The scripts are in this session's scratchpad (`order_probe.py`, `edge_probe.py`).

- **Note order inside a chord.** I fed each chord's notes in lane order, reversed, and kick-first with the pads reversed. All three match 22 of 22. So the video can't tell us the order. It doesn't disprove the decode either.
- **Ticks.** Using the file's own ticks matches 22 of 22. Scaling them to 192 resolution drops it to 11 of 22. So the game doesn't rescale a 480 .mid down to 192. It says nothing about files at other resolutions.
- **The "previous chord's colours" rule.** I removed it from one reference: the first note of a chord no longer avoids the previous chord's colours. The match fell to 5 of 22. So the video backs that rule, and the freeze below follows from it.
- **Freezes.** On 2,000 made-up pro grooves with no four-colour chords, nothing went wrong. With a four-colour chord on 2% of steps, 1,833 of 2,000 charts never finished shuffling. A chart whose first four notes all sit on tick 0 never finished either. Part 4 explains why.
- **Same-colour clashes and empty lanes.** On paper, the copy rule could give two notes in one chord the same colour, or give a note no pad at all. Neither happened in 4,000 made-up charts.

---

## Part 1: where the shuffle goes in Hydra's pipeline

Hydra already does all four of the game's earlier steps while it reads the chart, one chord at a time. They all live in `emit_chord_timestamp` ([song.cpp:323](src/parse/song.cpp:323)). Flam runs first ([model.cpp:228](src/core/model.cpp:228)), for .mid only. Disco flip runs next ([model.cpp:211](src/core/model.cpp:211)), and only with Pro Drums on. 2x kicks are never added when 2x Bass is off ([song.cpp:836](src/parse/song.cpp:836) for .mid, [song.cpp:1509](src/parse/song.cpp:1509) for .chart). There is no separate cymbal-fold step: with Pro Drums off, Hydra never creates cymbals, so they read as toms from the start ([song.cpp:779](src/parse/song.cpp:779), [song.cpp:1520](src/parse/song.cpp:1520)). So once the parser has built the song's chord list, all four steps are already done for that setting.

The shuffle can't live inside that per-chord function. It needs the whole song: the seed comes from the first four notes, and the copy rule looks two chords back. So it becomes one pass over the finished chord list, in a new file `src/parse/note_shuffle.{h,cpp}`. That's one function with one job: take a chord list plus the drum setting, and return the shuffled chord list.

It's called once, at the end of the two loaders every format passes through: `load_songbytes_mid` and `load_songbytes_chart` ([song.cpp:1696](src/parse/song.cpp:1696) says every loader ends there). .sng and .srb unpack to one of those two, so they get the shuffle for free. The loaders gain one input, the shuffle switch, alongside `pro` and `bass2x`. Analysis and the Preview both load through these functions, so both see the same shuffled chart.

The pass has to turn Hydra's chords into the game's note list and back. Hydra keeps a chord as a set of five lanes (kick, red, yellow, blue, green), each with a cymbal flag ([model.h:176](src/core/model.h:176)). The game keeps one note object per lane, with lane bits: kick 1, red 2, yellow, blue and green toms 4, 8 and 16, yellow, blue and green cymbals 32, 64 and 128. The pass builds that list, runs the game's rule on it, then writes each note's new colour and cymbal flag back. The note keeps its ghost or accent mark ([model.h:156](src/core/model.h:156)). Pro Drums on is the game's ProDrums instrument (seven pads to draw from). Off is the plain Drums instrument (four toms).

Two things have to be confirmed during the build. First, Hydra's fill, solo and Star Power passes run inside the parsers before this point ([song.cpp:1022](src/parse/song.cpp:1022) to 1044 for .mid, 1670 to 1686 for .chart). They must not read pad colours, so running after them changes nothing. The executor checks with a search, and the batch diff below proves it. Second, the order of notes inside a chord. Hydra throws away the file's order and always walks lanes kick, red, yellow, blue, green ([model.h:215](src/core/model.h:215)). Whether that matches the game is open question 2.

Clone Hero also has a second modifier that runs right after the shuffle (modifier bit 0x20, function 0x20F1900, called at 0x20D3329). Hydra models no other modifiers, so combining them is out of scope.

## Part 2: how the result is stored

Each drum setting gets one new stored result: difficulty × Pro Drums × 2x Bass, now also × Note Shuffle. Stored results are keyed by a text "mode" string built by `Settings::chartmode_key()` ([config.cpp:265](src/app/config.cpp:265)), for example "Expert Pro Drums, 2x Bass". The simplest design adds ", Note Shuffle" to the end of that string when the switch is on. With it off, the string is byte-for-byte what it is today. So every saved row keeps its key and stays Ready. The table, its unique key and its columns don't change, and there is no schema upgrade. This follows the precedent of Pro Drums and 2x Bass, which have always lived in this string.

The places that read the mode string back must learn the new ending. `Settings::with_chartmode` ([config.cpp:276](src/app/config.cpp:276)) rebuilds settings from a string by trying every mode, so it gains a loop over shuffle on and off. Without that, clicking a shuffled row in the path report would throw. `settings_for_mode` ([report.cpp:69](src/app/report.cpp:69)) uses it. The mode string is mentioned 114 times across 22 files, including the fill report and `fillcompare`. The executor goes through each one.

The switch is not a `hydra_rules.ini` field. Rules are judgment calls, and the rules fingerprint text is frozen ([rules.cpp:29](src/core/rules.cpp:29)). Adding a field would change every fingerprint and mark every saved row Stale. A rule would also let only one value exist at a time, but we want both results side by side.

**Version stamps (ADR 0018, [stored_versions.h](src/store/stored_versions.h)).** The results stamp rule says to bump it "whenever analysis output changes in a way the hydra_rules.ini fingerprint doesn't already catch". With the switch off, nothing changes. Shuffled rows use new keys, so no older row can hold a wrong value. So this feature on its own needs no bump of `kResultsStamp` ("2.1.0+allzero"). If it ships in a release that bumps the stamp for another reason, it shares that bump (D23). The scanned-chart stamp `kChartMetaStamp` (2) doesn't change: the shuffle doesn't touch identity, names, length or delay. Question 4 asks whether shuffled rows should get their own stamp.

Only summaries are stored (ADR 0026). Clicking a chart re-runs the engine, so the shuffled path's details need nothing new on disk.

`hydra_batch` analyzes one setting per run, taken from the settings file ([batch.cpp:117](src/cli/batch.cpp:117)). Once the switch is saved in `hydra.ini`, a batch analyzes whichever value is set. No batch code changes beyond reading the new key.

## Part 3: how the GUI shows it

A "Note Shuffle" checkbox sits beside "2x Bass" in the settings bar ([settings_bar.cpp:80](src/ui/settings_bar.cpp:80)). It's saved in `hydra.ini` as `view_noteshuffle`, off by default ([config.cpp:122](src/app/config.cpp:122)). It's locked during a batch like the others. Toggling it goes through `commit_settings` like every other setting ([app_state.cpp:800](src/ui/app_state.cpp:800)): the mode string changes, so the library re-reads its results and an open song re-analyzes. Nothing is marked Stale. The other result is still stored and comes back when the switch flips back.

What changes on screen once it's on:

- **Library.** The "Best path" column shows the shuffled result for the current setting. With nothing stored, it reads "Not analyzed", as today.
- **Preview.** The highway shows the shuffled gems, because it loads through the same loaders. Its load key `PreviewSongKey` ([preview_controller.h:49](src/ui/preview_controller.h:49)) gains the switch, or toggling wouldn't reload it.
- **Stars and Dynamics tabs.** These follow the engine. The star base comes from the engine's own totals ([stars.cpp:14](src/core/stars.cpp:14), [model.cpp:815](src/core/model.cpp:815)), so cymbals added by the shuffle raise the cutoffs automatically. Whether that matches the game is open question 3. The Dynamics tab's cymbal rows change as notes move.
- **Path report.** The "Mode" column prints the mode string ([report.cpp:55](src/app/report.cpp:55)), so it shows ", Note Shuffle" with no extra code.
- **Batch confirm.** The settings line ([library_dialogs.cpp:72](src/ui/library_dialogs.cpp:72)) gains "· Note Shuffle".
- **dmleaderboards.** Board scores don't record which modifiers were on ([dmbot_client.h:51](src/net/dmbot_client.h:51)). So Compare has to be refused while the switch is on. That uses the existing gate `why_not_comparable` ([dm_report.cpp:33](src/app/dm_report.cpp:33)), which greys out the button with a sentence.
- **User Guide.** A paragraph joins the settings section ([UserGuide.md:53](docs/UserGuide.md:53)).

GUI checks run through `hydra_uitest`, never by screenshots.

---

## Part 4: open questions to settle before any code

Each one has a way to settle it and a stop condition. None needs a whole-library run.

### Open 1: are ticks rescaled before the seed is taken?

Hydra keeps the file's own ticks: a .mid at its header resolution, a .chart at its `Resolution` (usually 192) ([song.cpp:978](src/parse/song.cpp:978), [song.cpp:1611](src/parse/song.cpp:1611)). The seed is lane × tick, so if the game stores ticks at another scale, every shuffled chart comes out different. The video proves raw ticks only for a 480 .mid.

**How to settle.** First, read the game's .mid and .chart readers in the dump, looking for any tick multiply on the way in. Then one game test by the user: a tiny groove saved as a 192-resolution .chart and a 960-resolution .mid, played with Note Shuffle on, recorded. Compare what's on screen with the reference fed raw ticks, then ticks scaled to 480.

**Stop condition.** If neither version matches, stop and report. Don't try a third scale by guessing.

### Open 2: what order are the notes in inside a chord?

Order matters in three places. It decides the seed when the four-note cut falls inside a chord. It decides which note counts as a chord's "first" (compared with two chords back) and which are "later" (compared with one back). And it decides which original note, carrying its ghost or accent, lands on which new pad. Hydra keeps no file order. The video can't tell the orders apart (see the evidence above). The same question covers where the game places the extra flam note, and a 2x kick sharing a tick with a normal kick. Hydra keeps only the first of those two ([song.cpp:927](src/parse/song.cpp:927)); that is the open question in the memory `derivation-audit-2026-10-03`.

**How to settle.** Read the game's per-player note builder (0x20D2EA0) and the chord builder it calls, to see whether notes in a chord are sorted by lane bit or kept in file order. If they're sorted, Hydra rebuilds the order from the chord and nothing else changes. If they're in file order, Hydra's .mid parser has to remember each note's position inside its tick, which is a parser change. Then one game test: use the reference to build a short chart where lane order and file order give visibly different pads. The user plays it with Note Shuffle on.

**Stop condition.** One reading pass and one game test. If they disagree, stop and report.

### Open 3: are the star cutoffs taken before or after the shuffle?

In Hydra, if the shuffle runs in the loader, the cutoffs follow the shuffled cymbals automatically. In the game, the star base is computed by 0x20F4680 and kept at engine+0x1D0. That function is not called inside the note builder: I checked the old listing of 0x20D2EA0, and no call reaches it. So whether the base sees shuffled cymbals depends on when the engine calls it.

**How to settle.** Find every caller of 0x20F4680 in the dump and check that it runs after the builder returns, on the same note list. If the user can find a results screen with stars from a shuffled run, that checks it too. The reference predicts the base and the cutoffs for that chart.

**What it changes.** If the base comes after the shuffle, there's nothing extra to do. If it comes before, the Stars tab needs the unshuffled base next to the shuffled score. That's a second number from a second chart, so it needs its own stored field under the derive-once rule. That design comes back to the user before any code.

### Open 4: the two cases where the game never finishes shuffling

**Seed zero.** If the first four notes all sit on tick 0, every lane × tick is 0, so the seed is 0. The generator then returns 0 forever, so every draw lands on red. The first pad note gets red. The next one must avoid red, never can, and the loop never ends. For example, a chart opening with a kick + snare + hi-hat + tom chord on tick 0 freezes. A chart whose first note is at tick 0 but whose fourth note is later doesn't.

**Four-colour chord.** A chord that uses all four colours (red, yellow, blue, green, in any tom or cymbal form) leaves nothing for the next chord's first note. That note must avoid every colour the previous chord used, so no pad is left and the loop never ends. The only way out is the copy rule: if the next chord has the same shape as the chord two back, its first note copies instead of drawing. For example, red + yellow cymbal + blue tom + green cymbal, then a lone snare, freezes. Two identical four-colour chords in a row behind a matching earlier chord don't.

**How to settle.** First, two game tests by the user, each a three-chord chart: one with a four-colour chord then a snare, one with four notes on tick 0. Does the game freeze, crash, or play? Then a count over the checked-in corpus (`testdata/input`, about 115 charts), run with the reference for every drum setting. That gives how many charts hit each freeze case, and confirms that same-colour clashes and empty lanes really never happen.

**Stop condition.** If the game plays either test chart normally, the decode is wrong somewhere. Stop and go back to the code reading before anything else.

---

## Questions for you

Each one quotes the rule as the code would apply it.

**Q1. The switch.** Rule: a checkbox labelled "Note Shuffle" beside "2x Bass", off by default, saved as `view_noteshuffle`. When it's on, every analysis, the Preview, the Stars and Dynamics tabs, and batches use the shuffled chart for the current difficulty, Pro Drums and 2x Bass. Proposed tooltip: "Score the chart as Clone Hero 1.1's Note Shuffle modifier rearranges it. The rearrangement is the same every time for a given chart and drum setting." Do you want this label and tooltip, or different words?

**Q2. The mode name.** Rule: when the switch is on, the mode string gains ", Note Shuffle" at the end. When it's off, it doesn't change. At one end, "Expert Pro Drums, 2x Bass" stays exactly that, and every saved row keeps working. At the other, "Easy Drums, 1x Bass, Note Shuffle" is a new key. This string is what the path report's Mode column, the batch confirm line and the dmleaderboards subtitle show. Is this name right?

**Q3. Pro Drums off.** With Pro Drums off there are no cymbals, so the shuffle only changes which tom each note lands on. The score, path and stars should come out identical to the unshuffled chart; only the Preview looks different. A batch diff on `testdata/input` will prove it. Rule I recommend: still store a separate row for "… Drums, … Note Shuffle", with one rule for every setting and no special case. The cost is one extra analysis per chart and setting. The alternative is to reuse the unshuffled row and shuffle only the Preview. Which do you want?

**Q4. A separate stamp for shuffled rows?** The results stamp rule today: bump `kResultsStamp` "whenever analysis output changes in a way the hydra_rules.ini fingerprint doesn't already catch", and every saved row reads Stale. This feature adds new keys only, so it needs no bump. Later, though, a fix to the shuffle alone, say after a game test, would mark every row Stale under that rule, unshuffled ones too. Rows re-analyze on click, so that's cheap. My recommendation is to keep the one stamp. The alternative is a second stamp in `stored_versions.h`, written only on shuffled rows, so a shuffle fix marks only those Stale. Keep one stamp, or add a second?

**Q5. Charts the game freezes on.** This only applies if Open 4's game tests show a real freeze. Rule: when the shuffle can't finish for a chart and setting, Hydra finds no path, and the library and details show a sentence like "Clone Hero freezes loading this chart with Note Shuffle on." It's shown the way an unreadable chart is today. Under Pro Drums off, a four-colour chord (red, yellow, blue and green toms together) followed by a different chord hits it. Under Pro Drums on, red + yellow cymbal + blue tom + green cymbal followed by a lone snare hits it. A normal two-pad groove never does. I don't recommend the other options: carrying on with a capped loop invents a chart nobody can play, and falling back to the unshuffled chart is silently wrong. Is the sentence right, and is "no path" the right outcome?

**Q6. dmleaderboards.** Rule: while Note Shuffle is on, "Compare with dmleaderboards…" is greyed out with "dmleaderboards scores don't say whether Note Shuffle was on, so they can't be compared with a shuffled path." With the switch off, nothing changes. Is that the behaviour and wording you want?

**Q7. If the order can't be settled.** If Open 2 stays unsettled after one reading pass and one game test, I recommend not shipping Note Shuffle until it is, because some charts would shuffle wrongly without anything saying so. The other choice is to ship with a warning in the tooltip. Which?

---

## The work, once the questions are answered

**Step 0, before any code (main session plus the user).** Copy the two references, the video check and the assembly listings out of the temp scratchpad into `docs/audit/note-shuffle/`, so they don't vanish with the temp folder. Settle Open 1 to 4: the code reading goes to one read-only scout on Fable, and the game tests are the user's. Run the corpus count on `testdata/input`. Record the answers as decision D104 in `docs/audit/2026-10-03-fix-decisions.md`, plus an ADR for where the shuffle lives and how it's keyed.

**One wave of three Opus executors in parallel, each in its own worktree.** Each owns its own files.

- **T1, the shuffle.** `src/parse/note_shuffle.{h,cpp}`, the call at the loaders' shared end, and a new `tests/test_note_shuffle.cpp`. The tests are fixed vectors produced by the reference: the video chart's 22 ticks, made-up charts covering the copy rule and the colour-blocking rule, and each freeze case once its outcome is known.
- **T2, the setting and key.** `config.{h,cpp}` (the ini key, the mode string, `with_chartmode`), the loaders' new input, `AnalysisSettings`, and the Preview key.
- **T3, the GUI text.** The settings bar, the dmleaderboards gate, the batch confirm line and the User Guide. T3 uses T2's setting, so it forks from T2's tip once T2 has committed the field. Until then, it builds against an agreed declaration.

**The join (main session).** Run the full test suite once. Then batch-diff `testdata/input`. With the switch off, every summary must match main byte for byte. With it on and Pro Drums off, summaries must match the unshuffled ones, if Q3 says separate rows. With it on and Pro Drums on, the reference predicts each chart's cymbal count, and Hydra's must agree. `hydra_uitest` covers the checkbox, the Preview reload and the greyed Compare button. Then a derive-once review before the merge.

**Not in this plan:** Clone Hero's other modifiers (including the one that runs right after the shuffle), guitar shuffle, and any whole-library run.
