# Hydra

Score optimizer and path viewer for Clone Hero drums: it scans a song library,
analyzes each chart, and shows the Star Power paths that give the best score.

## Language

### Library

**Chart**:
One playable song file (`.mid`, `.chart`, `.sng`, or `.srb`) plus its metadata.
_Avoid_: song file, track

**Library**:
The set of charts found by the latest scan of the user's folders.

**Scan**:
The pass that discovers charts in the configured folders and registers them in
the library. Distinct from analysis.

**Analyze**:
The search that computes a chart's paths and stores the result as a record.
Its progress bar moves in steps of at least half a percent of the chart
(src/search/engine.cpp), and the main search owns the first 90% of the bar,
the all-0 pass the rest (src/search/pather.cpp; D48, Q33).

**Record**:
The stored result of one analysis: the kept paths, their scores, and the
settings the analysis ran with. Its key is the chart and its analysis
settings: the chart mode, the SP cap, the fill rule, the path limit and the
score range. A chart
keeps one record per settings combination; records share their stored paths,
so a path found under several combinations is stored once. A record is stale
unless both hold: it carries a results stamp this build accepts (a stamp that
changes only when analysis output changes, not with every release; see
src/store/stored_versions.h), and its stored paths are in
this build's path-structure format under the rules in force. A lookup reports
it as one of three statuses: not
analyzed, stale, or ready. A listing returns only ready records, so a stale
record reads the same as no record at all.

**SP cap**:
The Star Power meter ceiling an analysis runs under, in bars. 4 is Clone
Hero's rule and the default. Other values answer what-if questions; their
scores are not achievable in game.
_Avoid_: edition, uncapped, SP meter

**Analysis settings**:
The seven settings that make up a record's identity, shown together in the
settings bar and applied to every song: difficulty, Pro Drums and 2x Bass
(together the chart mode), the SP cap, 1.0 fills (the fill spawn deadline),
the score range, and the path limit.
Changing one shows the records made under the new combination; changing it
back brings the old ones back without analyzing again; the app keeps the last
16 lookups the number boxes stepped away from, so stepping back does not ask
the store again (src/ui/app_state.h; D48, Q33). The backend limit is
not one of them: it only hides backend rows on screen and never re-analyzes.
The path limit starts on<!-- default: Settings::mslimit_enabled -->
at 10 ms<!-- default: Settings::mslimit_value -->. The backend limit ("Hide
backend rows beyond") starts off<!-- default: Settings::backendlimit_enabled -->
with 50 ms<!-- default: Settings::backendlimit_value --> in the box. The user
confirmed both defaults (D48, Q33); `Settings` in src/app/config.h owns them.
_Avoid_: view options

**Library search**:
What the user types to narrow the library. Every word must match the title,
artist, charter or folder, in any order; `"quotes"` match a phrase, and
`title:`, `artist:`, `charter:` and `folder:` limit a word or phrase to one
field. `stars:N` (exactly N stars) and `squeeze<=N` (hardest squeeze at most N
ms) test the stored best path and only ever match ready records. Matching
folds case and accents away, so `beyonce` finds "Beyoncé", and ignores Clone
Hero's rich-text tags.

**Batch time left**:
The batch strip's estimate of how long the rest of a batch will take: the
average time per finished chart so far, times the charts still to go. It
shows only once three charts have finished (`kEtaMinFinished` in
src/ui/library_jobs.h). Decided in the approved
docs/superpowers/specs/2026-09-27-ui-redesign-design.md ("The estimate appears
once three charts have finished").

**UI timings**:
How long the app's short-lived messages and refreshes wait. The Analyze
panel's "Done!" stays half a second, the "Copied!" flash after Copy path
lasts 2 seconds, the library search refilters at most every 0.15 s while
typing, and a running batch refreshes the library at most once a second
(src/ui/app_state.cpp, src/ui/paths_tab.cpp, src/ui/library_table.cpp;
confirmed as they are, D48, Q33).

**Leaderboard fetch**:
The download of real scores for the leaderboard report. It comes from the
DMBot API, the backend behind dmleaderboards.com, not from the site itself. It waits at most 15 s to resolve the host, 20 s to connect, 30 s to
send and 120 s to receive, because the backend cold-starts after idle; it
checks Cancel every 50 ms; and it accepts only HTTP status 200
(src/net/dmbot_client.cpp; D48, Q33).

### Paths

**Path**:
One way to play a chart's Star Power: which activations to take and what each
is worth. Every path that ties the record's top score is optimal, not only the
first one listed (D48). A path's average multiplier reads 0.000x when it has no
scoring notes, instead of dividing by zero (`Path::avg_mult`); real charts
never hit this (D48, Q33).

**Activation**:
One use of banked Star Power, written in path notation with its skip count and
squeeze symbols (e.g. `E2+-`).

**Skip**:
A fill an activation deliberately passes over before activating.

**Authored fill**:
A fill written in the chart. Each one is placed on its own once every chord
is read: on the last chord at or before its end but not before its start, or
on the first chord after its end within the landing window
(`fill_land_slop_beats`, plus one tick). The closer wins, a tie goes to the
later chord, and a fill with neither is dropped (docs/adr/0023).

**SP phrase**:
A chart section that awards a bar of Star Power when hit fully. It covers
the chords with start <= tick < end, and pays on the last of them; a phrase
of length zero pays nothing, and one that runs past the last note pays on
that note (Clone Hero's rule, docs/adr/0023).
_Avoid_: star power section

**All-0 path**:
The best path whose activations all record zero skips, found under a 0 ms
timing limit.

### Squeezes

**Squeeze**:
A deliberately early or late hit that moves points across a scoring boundary.

**Frontend squeeze**:
Hitting the activation chord's activation note first, so the chord's other
notes score under Star Power.

**Backend squeeze**:
Hitting a note near the SP end early, so it lands inside Star Power.

**SqIn / SqOut**:
Squeezing an SP phrase's note into (+) or out of (-) an active Star Power
window, written as the `+`/`-` symbols in path notation. Only a phrase after
the activation chord can be squeezed: one at or before it was banked before
SP started (core/sqout_chord.h, `activation_can_squeeze`). A phrase is
squeezed in only once (D34). An SP end offers a window at most one phrase
(`offered_phrase`, D36): its newest phrase, when collecting it is what moved
the window's end away from this SP end; or, when the window's end is this SP
end, the first phrase after it that the window has not squeezed in. An older
phrase can't be squeezed out while a newer one stays in: the newer one is hit
later, so it would be hit after Star Power ran out too.

**Multiplier squeeze**:
Ordering the hits of a multi-note chord on a combo-multiplier boundary so the
more valuable notes score on the higher multiplier.

**Star cutoff**:
The score a chart needs for 1 to 7 stars: its base score (every note at 1x,
no Star Power) times 0.1, 0.5, 1.0, 2.0, 2.8, 3.6 or 4.4, rounded up. Clone
Hero compares it against your score without the solo bonus. The product is
kept as a 32-bit float before it is rounded up, as the decompiled game does,
which can move a large cutoff by one point (`star_cutoff` in
src/core/stars.cpp; D48, Q33).

**Early fill (E)**:
An activation timing (the `E` notation) where the fill must be summoned by
hitting early; its window is fixed, not the hit-window setting.

**Fill spawn deadline (CH 1.1)**:
The latest your SP meter can fill up and still have a fill appear. Clone Hero
1.1 puts it a flat 4 beats before the fill starts. Hydra's default rule. A
beat here is a quarter note (the chart's resolution) in every meter, 6/8
included; that is not verified against Clone Hero (docs/adr/0023).

**Fill spawn deadline (CH 1.0)**:
The older rule: roughly one fill-length of lead time before the fill, clamped
to 250..10000 ms. Short fills got stricter in 1.1 and long fills got looser.
The "1.0 fills" analysis setting, or `hydra_batch --legacy-fills` into its own
database. Part of a record's key, so 1.0 and 1.1 records sit side by side;
`hydra_fillcompare` diffs the two. See docs/adr/0010.

**Generated fill**:
A fill Hydra places itself on a chart with no authored fills: on the chord
nearest a downbeat, if one sits within half a beat of it, half a measure
long, and at least 4 measures after the last one (hydra_rules.ini can change
these). With two or more meter changes
between two chords, its length reads the earlier meter; that is not
verified against Clone Hero (docs/adr/0023).

**Hit window**:
The per-side ms window Clone Hero registers a hit in. A setting; feeds the
squeeze budgets, ratings, and report tiers, never the search. The report's
timing tiers are Normal below 2 ms (`kDifficultMs`), then Hard below half a
hit window, Extreme below one window, Insane below one and a half, Insane+
below two, and Beyond from two windows up (`timing_tiers` in
src/core/squeeze_rating.cpp; D48, Q33).

**Transfer scale**:
How frontend timing error carries to the SP end. SP length is measured in
measures, so a hit `d` ms off moves the SP end `r*d` ms; early and late hits
can scale differently on a signature or tempo change. It is measured from the
note whose timing moves the SP end: the activation, or the cap's collecting
note. Each SqIn stores its own scale, at the SP end it was measured from
(docs/adr/0021).

**SP-end history**:
Every place an activation's SP end moved, in order: the activation, each
phrase collected, each cap clamp and each SqIn, with the end in force after
it. The engine stores it once per activation. The deact node, the clamp note
and the collected phrases are read from it (docs/adr/0021).

**Deact node**:
The exact chart position where an activation's Star Power ends; backend
squeeze timings are measured against it. The search stamps it onto the
record as it runs, and nothing downstream re-derives it.
_Avoid_: SP end tick (when the anchored search position is meant)

**Cap-clamped window**:
Normally an activation's Star Power window ends a fixed distance (measures)
past the activation. But if a phrase collected partway through Star Power
would overfill the meter past the SP cap (the max bars of SP you can hold),
the window's end gets pinned to the cap measured from that phrase's note
instead. A phrase that only fills the meter exactly to the cap is a tie, and a
tie does not clamp. The end is pinned only while the meter is full: as it
drains, a later phrase that fits under the cap extends the end again from
where it was pinned (`ScoreGraph::extend_deacts`, docs/adr/0013).
`clamp_tick()` names the note that pinned it, so later code doesn't have to
guess; a later unclamped extension keeps the earlier note.
The overfill warning in Song Details needs two things: the window is clamped,
and the activation lists a SqIn/SqOut or an uncounted or squeezed-out backend
row (`rate_activation` in squeeze_rating.cpp). A clamp with neither shows no
warning.

**Squeeze rating**:
The displayed difficulty judgement of a squeeze: its rating label, its
effective ms once the transfer scale is applied, and whether the scale is
material enough to warn about.

**Backend leeway**:
How long after the SP end a note still scores under Star Power without a
squeeze: less than `backend_leeway_ms`
(3 ms<!-- default: Rules::backend_leeway_ms --> by default). A note exactly
3.0 ms<!-- default: Rules::backend_leeway_ms --> after the SP end does not
score under SP. Hydra's own rule: no such constant was found in the Clone
Hero engine methods read; the 3 ms<!-- default: Rules::backend_leeway_ms -->
is Hydra's own setting.

**Difficulty**:
A path's or activation's hardest required squeeze, in raw gap ms — never
scaled by the transfer scale.

### Preview

**Preview**:
The rendered, playable view of a chart: its notes as a scrolling 3D highway,
synced to the song audio. Distinct from a Path (the scoring plan) and a Chart
(the song file).
_Avoid_: player, viewer

**Note highway**:
The 3D fretboard the Preview draws, with the drum notes scrolling toward the
strike line as the song plays.
_Avoid_: fretboard, track

**Gem**:
One drawn note on the note highway. Its model shows the drum type (tom, cymbal,
or kick), its texture shows the lane, the note's dynamics, and whether it sits
in an SP phrase. Dynamics are a velocity rule that applies to every lane, kick
included: velocity 1 is a ghost, velocity 127 an accent, and both score double
(see docs/adr/0012). A MIDI chart turns them on with the exact text
`ENABLE_CHART_DYNAMICS` or `[ENABLE_CHART_DYNAMICS]` in its drum track, and
only notes after the tag count; a `.chart` always has them on.
_Avoid_: note (the chart datum), block

**Lane**:
One column of the note highway, in KRYBG order. The four playable lanes
(Red..Green) spread across the highway; Kick is the full-width bar.

**Disco flip**:
A section where the chart swaps the red and yellow lanes, marked by text
events such as `[mix N drums0d]` (on) and `[mix N drums0]` (off), where N names the
difficulty: 0 Easy, 1 Medium, 2 Hard, 3 Expert. Each difficulty reads only
its own markers, and the flip happens only with Pro Drums on.
`drums0dnoflip` reads as off, unlike Clone Hero (docs/adr/0023).

**2x kick**:
A kick written for a double bass pedal: MIDI 59 Easy, 71 Medium, 83 Hard,
95 Expert, or `N 32` in a `.chart` difficulty section. Each difficulty reads
its own. The 2x Bass setting turns them on at every difficulty, like Clone
Hero's Double Kick modifier.

**Strike line**:
The fixed line on the note highway where a note is due to be hit. Notes scroll
down to it as the song plays; a gem flashes there when it passes.
_Avoid_: hit line, target line, target (the Onyx name)

**Beat line**:
A line across the note highway at a bar, a beat, or the half-beat before one,
drawn from the chart's timing.

**Active SP window**:
The stretch of the note highway from an activation to its deact node, where
Star Power is being spent. Comes from the path, not the chart; drawn as a
tinted floor.

**Path overlay**:
Hydra's own analysis drawn on the note highway: the active SP windows and the
fills as a player following the path would see them. A fill the path
activates on is *taken* (all four lanes lit, the activation note's lane
highlighted); a fill the path had enough SP for but passed over is *offered*
(lanes lit dimly); every other candidate fill is hidden, because the game would
not have shown it. Offered fills are the ones the engine stored on each
activation as passed over; nothing guesses them from a count. A tied variant's
activations carry its own passed-over fills and early-fill offset, the next one
after a fold between windows included (D38), and a fill the path took always
stays taken. Fills
after the last activation are hidden because the engine records nothing about
them.

**SP meter gauge**:
The vertical gauge on the note highway's right edge showing banked Star Power
at the playhead. With a path, every value is the record's. Between activations
it steps up one bar at each tick where the engine stamped a bar's arrival. In
an active window it shows the measures left until the SP end in force, two
measures to a bar, and the record lists every place that end moved. It empties
exactly at the deact node. The gauge counts no phrases and applies no cap. A
late squeeze-in refills at the old SP end, because the player hits that phrase
early and SP never stops. A squeezed-out phrase's bar arrives when the player
hits it: as SP ends for an early phrase, on its own note for a late one.
Without a path it fills one bar per phrase and pins at the cap, since nothing
spends it and there is no record to read.

**Stem**:
One of the several audio files a chart may ship instead of a single mix (e.g.
`drums`, `guitar`, `song`). The Preview mixes all of a chart's stems into one
output, unpacking each from its compressed file as it plays (docs/adr/0019).

**Mixer**:
The step that reads a chart's stems at one shared position, resamples them to
one common format, and sums them into a single signal to play. It reads a few
milliseconds at a time from each stem's compressed bytes; nothing is unpacked
up front. A stem it cannot open is skipped, so one broken stem does not
silence the rest. A stem that hits a decode error part way through goes
silent from that point.

**Transport**:
The Preview's play, pause, and seek control together with its clock. The clock
is the master: while playing it is the time at play plus the time since; the
note highway reads it to place the notes, and the audio follows it.
The audio is shifted by the chart's audio offset, so the notes land on the
music as they do in Clone Hero. A nonzero song.ini `delay` replaces the
.chart `Offset`; a delay of 0 counts as unset. A positive value makes the
notes come later than the music. `preview_audio_offset_ms` in
src/app/preview_source.cpp is the one owner. This was decided in
docs/superpowers/plans/2026-09-24-derivation-fixes.md (decision 9; Task 17's
gate was measured at the game) and extended to .sng and .srb by
docs/superpowers/plans/2026-09-26-codebase-audit-fixes.md (decision 4).
_Avoid_: player (the whole Preview), scrubber (the UI control only), playhead
(the audio follower only)
