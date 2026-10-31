# Live checks

This page lists the rows only a run of the real game can settle.
It describes build 1.1 (v1.1.0.6142) unless a check names build 1.0.
Last write-back: 2026-10-31.

Each check gives its row id, the question in one line, what a pass and a fail look like, and whether a bot run is enough or the run needs Star Power activation and a menu start. The Star Power plan, `docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md`, takes its live steps from this list. Where one entry covers two or three rows, they share one run.

A "bot run" is a song the game's own bot plays from a command-line start; the bot never gains or spends Star Power. A "menu start" is a song started from the game's menus. The entries below say when they need one, which is mostly for a modifier or for key-press play.

## First: the Ladies Talk fill check (FL-27)

**FL-27.** Question: does Clone Hero offer a fill inside the second drum solo of "Ladies Talk", where no fill is written? The run that takes it is step LS3 of the Star Power plan.

The chart is `testdata/input/common/IB24/T6/Senri Kawaguchi - Ladies Talk [Kiyo Sen] (Drumeo Performance)/notes.mid`, at 480 ticks a beat. Both sources agree on the ticks. Wave 1 scout H4 read the chart: the last written fill before the gap runs from tick 131,520 to 132,960, the next from 191,760 to 193,200, and the solo from 140,400 to 186,480. LS3 reads them from Hydra's score file: the fill before the solo ends on chord 1019 at tick 132,960 (114,628.9 ms, measure m76), and the fill after ends on chord 1514 at tick 193,200 (166,546.0 ms, m108). LS3's window is those two times moved inward, 114,778.9 to 166,544.0 ms.

Pass: no Star Power flip inside the window, although Star Power is ready there with the flag clear. Fail: a flip inside it. Needs Star Power and a menu start: the bot never activates, and the flip comes from the game's press handler at a flagged note.

## Page 1: reading a chart

**CR-01.** Which notes file wins when a folder holds both? Pass: the game loads notes.mid (ch_notes shows its chart). Fail: it loads notes.chart. A bot run is enough.

**CR-02.** Does the game find NOTES.MID in upper case? Pass: the song loads. Fail: no chart. A bot run is enough.

**CR-08.** Does the game read a .srb whose notes entry has an odd name? Pass: it loads by sniffing. Fail: it refuses. Needs a crafted .srb started in the game; no Star Power.

**CR-16.** Does a track named "notes" then "PART DRUMS" at tick 0 load as drums (Hormone's Echo)? Pass: the drum highway has notes. Fail: no drum track. A bot run is enough.

**CR-23.** Two .chart notes written in reverse tick order. Pass: the game hits them in tick order (ch_notes). Fail: file order. A bot run is enough.

**CR-24.** Does [coda] or a text holding _ENDOFSONG change the song length or score? Pass: no change. Fail: a change. A bot start is enough.

**CR-29.** Does "[sectionX]" make a section? Pass: it shows in the practice section list. Fail: it does not. Needs a menu start (the practice menu), not only the bot.

**CR-31.** Does a wrong-case key such as "[section intro]" show a different name from "[section Intro]"? Pass: the names differ as Hydra says. Needs a menu start (the practice menu).

**CR-34.** Nested .chart solos (solo 0, solo 100, soloend 200, soloend 300, notes at 150 and 250). Pass: both notes earn the solo bonus (solo_bonus_total in scorestats.json). Fail: only the 150 note. A bot run is enough.

**CR-44.** Vocals running past the last drum note, no end event, short audio. Pass: songLength follows the drums (the game's code). Fail: it follows the vocals. A bot start is enough.

**CR-46 and CR-47.** songLength against Min(end, Max(audio, last note)) plus 1 s, on a chart with known numbers and real audio. Pass: equal. Fail: any other value. A bot start is enough.

## Page 2: building the note list

**NL-12.** The dynamics tag with a trailing space. Pass: ghost_notes_total is 0. Fail: it counts. A bot run on Pro Drums is enough.

**NL-13.** A note and the dynamics tag on one tick, tag second. Pass: the note scores plain. Fail: it scores its mark. A bot run is enough.

**NL-14 and NL-15.** Velocity-1 and velocity-127 kicks under the tag (I Am Ahab has 33 accent kicks). Pass: ghost_notes_total or accent_notes_total counts them and the score equals Hydra's with them priced. Fail: either is missed. A bot run on Pro Drums is enough.

**NL-16.** A merged kick carrying both a ghost and an accent. Pass: it counts as an accent only. Fail: a ghost, or both. A bot run is enough only with the per-note hook on; the score does not show it.

**NL-19.** `E [mix 3 drums0d]` with spaces in a .chart. Pass: the pads flip in the game (compare lanes in ch_notes). Fail: no flip. A bot run is enough.

**NL-22.** Does plain Drums skip the disco flip? Pass: the plain-Drums total equals Hydra's with no flip. Fail: it equals the flipped total. A bot run is enough.

**NL-30.** A 2x kick and a normal kick on one tick in both file orders, 2x off and on. Pass: total_notes and score equal Hydra's for both orders. A bot run is enough.

**NL-32.** Two red note-ons on one tick, velocity 127 then 1, under the tag. Pass: one red note with both marks counted (accent and ghost totals). Fail: only the first mark. A bot run is enough.

**NL-34.** A .chart chord written against colour order, Note Shuffle on. Pass: the shuffled pads match the colour-order row. Fail: the file-order row. Needs a menu start (Note Shuffle is a modifier).

**NL-37, NL-38 and NL-39.** Flam cases: a .mid flam on a single pad and on a two-pad chord, a green cymbal flam, and a .chart single-pad chord. Pass: total_notes rises by one for the single pad only, the cymbal gems show as Hydra predicts, and the .chart gets no copy. A bot run is enough.

**NL-42.** Does plain Drums draw only red and the toms under Note Shuffle? Pass: shuffled chords of song A1 with Pro Drums off match the "Pro Drums off" rows in docs/audit/note-shuffle/game-tests/predictions.json. Fail: any cymbal. Needs a menu start (Note Shuffle); a bot run once started.

**NL-57.** How the game draws a lane under Note Shuffle (Glide at 1:51.110). Pass or fail is what a human sees: only blue, or both columns. Needs a menu start with the user's song (search to reach it).

**NL-58.** A .mid lane whose first note starts late. Needs a human looking at the highway; a bot cannot show it.

**NL-65.** A .mid with the tag, velocity-1 and -127 notes, plain Drums then Pro Drums. Pass: the plain-Drums totals are 0 and the score gap equals the dynamics notes' points. Fail: no gap. A bot run is enough.

**NL-69.** What modifier bits 0x20, 4 and 8 do to the notes. Compare the note list with the modifier on and off. Needs a menu start (modifier selection).

## Page 3: timing

**TM-02.** Question: does `delay = 0.5` read as 500 ms? Pass: `+0x24` of the song metadata reads 500. Bot run, menu start only.

**TM-03.** Question: does `Offset = 0.2504` read as 251 ms? Pass: 251 at `+0x24`. Bot run.

**TM-14.** Question: does anything write engine+0x2E0 at a hit? Pass: it changes once per hit to within a few ms of the clock. Key-played stretch (the bot run read 0.0 throughout).

**TM-22 and TM-27.** Question: where is the hit/miss boundary on notes with gaps of 75, 120 and 170 ms? Pass: at half the stored window (48.1, about 60, 85.7 ms). Fail: 85 ms for all. Bot run with a custom press offset is enough; no Star Power.

**TM-26.** Question: are the early and late sides of a note's window the same? Pass: an early and a late press at the same offset get the same verdict. Bot run with custom offsets.

**TM-32.** Question: does the menu's Precision Mode set bit 0x1000, and does the window poll read 50.44 to 84.89 ms? Needs a menu start; bot run; no Star Power.

**TM-36.** Question: does video calibration move anything that judges a hit? Needs a human or an audio and video capture; a bot cannot tell.

**TM-40 and TM-43.** Question: at song_speed 50 and 200, do the window values and clock+0x38 against the audio position match the 100% run? Bot run with a menu start; no Star Power.

## Page 4: fills

FL-27, the Ladies Talk check, is the first entry on this page.

**FL-18.** Question: does the game skip a fill on the song's last chord a few ticks before the final measure line? Pass: no 0x800 flag on that chord. Fail: flagged. Chart load and a snapshot.

**FL-24.** Question: how far early can a fill-end hit be and still summon the fill when Star Power comes ready just after the deadline? Pass: the fill is offered up to 85 ms. Fail: it stops earlier. Needs Star Power activation and a menu start, key-played.

**FL-25.** Question: why did the game not offer Trippin path 3's E0 fill at tick 34944? Pass: a repeat with the fill watched shows the flag cleared and why. Bot run is enough to watch the flag; activation not needed.

**FL-28.** Question: does a chart whose only written fill lands on no chord get generated fills? Pass: none appear (the listing). Fail: generated fills appear. A probe chart, a chart load and a snapshot of the 0x800 flags are enough; no Star Power activation.

**FL-29.** Question: does a fill written on Expert only stop generated fills on Hard in a `.mid`? Pass/fail: read the 0x800 flags on Hard. Chart load and a snapshot; no activation.

**FL-32.** Question: when does a fill appear on screen? Pass: a recorded frame time for the fill's first drawn frame against its record's tick. Bot run with a capture.

## Page 5: Star Power

**SP-06.** Question: does a phrase nested inside another (for example 0 to 100 and 20 to 40, notes at 10, 30, 50) award in the game? Pass: the game flags only the outer phrase's last chord, so Hydra's extra award is a difference to raise; fail: both award, as in Hydra. A chart load and a read of note flags is enough; no Star Power activation, no menu start.

**SP-14.** Question: why did run 1's first activation start at 2,879 and not 2,880? Pass: a repeat run shows a whole-bar meter at every activation (noise); fail: the one-tick shortfall repeats on the first activation. Needs Star Power activation and a menu start.

**SP-15.** Question: does a game SP end follow the time signature (a 3/4, 6/8 or 7/4 section, or a change inside an activation)? Pass: the game's end (+0x120) is within one frame of Hydra's end; fail: off by about a beat or the part-measure share. Needs Star Power activation and a menu start.

**SP-18.** Question: can drums activate Star Power without a fill note? Pass: pressing every pad with banked Star Power and no fill never activates; fail: Star Power turns on. Needs Star Power banked and a menu start (key-press run).

**SP-21.** Question: is the activating note scored x8 or x4? Pass: x8 (the score jump on that chord is double); fail: x4. Run 1's `run1_cal0_sp_watch.csv` (score and multiplier columns around chord 162) may answer it with no new run; otherwise needs Star Power activation.

**SP-24.** Question: when a phrase overfills the meter during Star Power, does the end move to one full bar after the collecting moment? Pass: meter stops at +0x1B0 and the end equals collecting time plus a full bar; fail: the end keeps extending or does not move. Needs Star Power activation with four phrases banked and a fifth collected, and a menu start.

**SP-29.** Question: does a hit just past the SP end, inside the frame that first reaches the end, score doubled? Pass: a backend note a few ms after the end, hit on time, pays doubled while the frame is that long; fail: it pays single. Needs Star Power activation and a menu start, with the clock and score logged per hit.

**SP-30.** Question: is a hit ever judged at a clock between two frame clocks? Pass: a hit trace shows only frame clocks; fail: a hit's clock lies strictly between two frames. A bot run with the hit trace is enough.

**SP-31.** Question: does Hydra's end match the game's on more activations and other charts? Pass: game end minus Hydra's end under 5 ms on at least two more activations, one not in 4/4; fail: a larger gap. Needs Star Power activation and a menu start.

**SP-33.** Question: is a note exactly on an SP end, hit exactly on time, doubled? Pass: doubled (Hydra's reading); fail: single. Needs Star Power activation and a menu start.

**SP-35.** Question: which hit credits a phrase, and with which Star Power state? Pass: the meter rises on the hit of the phrase's last chord, using that hit's state; fail: it rises elsewhere. A key-press run (the bot gains no meter); no activation needed.

**SP-36.** Question: do squeezed_notes and squeeze_note_losses count the game's +0xE8 and +0xEC counters, and do they match Hydra's squeezes on the chords hit? Pass: equal counts; fail: they differ. Needs Star Power activation and a menu start.

**SP-37.** Question: do game and Hydra agree on which early or late hits collect a phrase at the SP end? Pass: the meter rises only on the side Hydra's SqIn or SqOut rule says; fail: otherwise. Needs Star Power activation and a menu start.

**SP-46.** Question: during Star Power, is the multiplier disc the only thing on the 1.1 screen that changes, with no Star Power meter drawn? Pass: a screenshot before and during Star Power differs only at the disc (and the highway's colours); fail: a meter or bar shows the Star Power amount. Needs Star Power activation and a menu start.

## Page 6: scoring

**SC-07 and SC-08.** Question: on a run that activates Star Power, do sp_score and combo_score equal Hydra's for the same activation path? Pass: equal; fail: they differ. Needs Star Power activation and a menu start (the bot never gains Star Power).

**SC-10.** Question: does the press order inside a chord that crosses a multiplier step change the score by Hydra's hit-order gain? Pass: the score gap equals it; fail: no gap. A key-press run with a chosen order; no Star Power.

**SC-11.** Question: does the combo carry into the next song of a setlist? Pass: the streak carries; fail: it resets. Needs a menu start and two songs; no Star Power.

**SC-12 and SC-14.** Question: what do the lane bytes (+0x309 to +0x310) and the threshold (+0x2C0) hold for a pad hit hard, a pad hit soft and a key press? Pass: keys give a value that never marks a dynamic, and the marks follow SC-13; fail: otherwise. The key side needs a key-press run; the pad side needs a kit; no Star Power.

**SC-16.** Question: is bit 0x10 of the flags word (+0x198) clear in a normal Quickplay run? Pass: clear; fail: set. A bot run with one snapshot is enough.

**SC-19.** Question: does an accent kick (velocity 127) pay double? Pass: accent_notes_hit counts it and base_score includes the doubling; fail: it does not. A bot run on a .mid with velocity-127 kicks is enough.

**SC-20 and SC-21.** Question: what is the solo bonus for a full solo and for a partly hit one, and does N count gems or chords? Pass: a full solo pays 100 per gem, a partial one follows the SC-20 formula; fail: any other number. The full case is a bot run on a chart with a solo marker and multi-gem chords; the partial case needs a key-press run that skips notes; no Star Power.

**SC-29 and SC-33.** Question: does band_stars ignore the solo bonus, and is avg_multiplier times base_score equal to score less solo_bonus_total? Pass: both; fail: either differs. A bot run on a chart with a solo, scored near a cutoff; no Star Power.

**SC-30.** Question: which HUD star-bar copy runs (slot 0 at 0.1 times base, or slot 1 at 0.5)? Pass: the StarProgress cutoff array's slot 0 is ceil(0.1 x base); fail: slot 0 is 0. A bot run read after the first frame.

## Page 7: internals

**IN-19.** Question: does `0x20DEEC0` fire only for hit notes? Pass: on a run with misses its calls equal the game's hit-note count in every section. Fail: calls exceed it. A bot never misses, so it needs a key-press run with misses and a menu start; no Star Power.

**IN-20.** Question: does `0x20DE770` fire for a missed note? Pass (hit-only): its count equals the hit-note count. Fail: it also counts misses. Needs a key-press run with misses and a menu start; no Star Power.

**IN-21.** Question: do key presses carry a lane value that fails the dynamics threshold? Pass: the lane byte (+0x309 to +0x310) on a key press stays on the failing side of the threshold, and a bot hit passes it. Fail: key presses register accents or ghosts. Needs one bot run and one key-press run with a menu start; no Star Power.

**IN-27.** Question: with Star Power off, does anything clear the flag at +0x130 after a gain sets it? The per-update method clears it only while Star Power is on (GD line 53572); read the other writers and clearers first (xrefs to +0x130). Pass: the flag turns on at every gain and at a clamp, and off where the clearing code says. Fail: it follows some other pattern. A bot never gains meter, so it needs Star Power gains: a key-press run with a phrase that overfills the meter and an activation, from a menu start.

**IN-29.** Question: does `GlobalVariables.currentSongEntry` hold the playing song during a song? Pass: its checksum equals the song's checksum in `scorestats.json`. Fail: null or another song. A bot run is enough; no Star Power.

**IN-36.** Question: does the results screen ignore keys after every command-line bot run, and take them after a menu-started run? Pass: keys work after a menu start and not after a bot run. Fail: keys fail after both, or work after both. Needs one bot run and one menu-started run; no Star Power.

**IN-40.** Question: does the 1.0 bot earn Star Power? Pass: after a 1.0 (build 4045) bot run on a chart with phrases, scorestats or the meter shows phrases earned. Fail: 0 earned. Needs a 1.0 run; a command-line bot start works in 1.0; no activation needed.

**IN-44.** Question: is there a setting or flag that fills the HitDetection logs? Pass: a song writes lines into `HitDetection-Standard.log` or `HitDetection-Validation.log`. Fail: none found. A bot run is enough once the setting is known; no Star Power.
