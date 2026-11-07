# Clone Hero 1.0 (build 4045): how the game makes and places drum fills

Source: `fill_generator.cs` beside this file. It holds eight methods copied unchanged from
`ilspycmd` output of `C:\Clone Hero\1.0 old\Clone Hero\Clone Hero_Data\Managed\Assembly-CSharp.dll`
(decompiled 2026-10-30, session b02ab3e6-76ac-4735-8576-04b87f32bb8d). Build 4045 is a Mono
build, so there are no addresses; a rule is cited as `Game code (4045): Class.Method`. The game's names
are scrambled, so every name below is our own readable name; `../names-4045.tsv` maps each one to the
scrambled name that `fill_generator.cs` uses. Line numbers are lines of `fill_generator.cs`. A banner comment
at the head of each method gives its line range in the full decompiled file.

Name key (role, readable name): the chord list of a drum track is a list of chord objects
`Chord`; a fill is an object `DrumFill` (start time, length in seconds, start tick, length in ticks,
and a "still on" flag); a written fill (one the chart file names) is `WrittenFill` (start tick, length in
ticks); the tempo map is `BeatGrid`; the chart is `Chart`; the per-track data is `TrackData`.

## Where the choice is made (lines 6 to 27)

Lines 7 to 17 are the head of `TrackBuilder.BuildPlayData`, the method that builds a drum track's play data.
Line 20 is the whole choice. If the track has no written fills at all (`WrittenFills.Count == 0`), the game
generates fills with `TrackBuilder.GenerateFills`. If the track has even one written fill, the game places only
the written fills with `TrackBuilder.PlaceWrittenFills` and generates none. The test is on the whole track, not on a
stretch of the song, and not per difficulty beyond what the track holds. This is the 1.0 answer to "does one
written fill stop generated fills".

## F1: the generator, `TrackBuilder.GenerateFills` (lines 30 to 77)

Lines 34 to 37: if the chord list exists and is empty, return an empty list.

Line 39: `num` is one 64th note in ticks, `12 * resolution / 192`, rounded. (`standardResolution` is 192.) At the
usual resolution of 192 that is 12 ticks.

Line 40: the loop walks the tempo map's list of measure lines `measureLines` (built by F5; lines are real-time
seconds) starting at index 1, the second measure line, and normally steps by 4. So the first candidate is the
second measure line, then every fourth line after it.

Lines 42 and 43: for the candidate line `i`, the fill's start time is the point halfway between measure line
`i-1` and measure line `i` (half a measure of real time before the line), and the fill's end time is line `i`.

Lines 44 to 49: find the chord nearest to line `i` (F4), searching from the last accepted chord onward. If no
chord comes back (-1), `i -= 3; continue;` makes the loop's `i += 4` net to `i + 1`: the game tries the very next
measure line instead of skipping four.

Lines 50 to 57: the nearest chord's time must be within 0.25 seconds of the measure line (`num5 > 0.25f`, absolute
difference). If not, the same `i -= 3; continue;` tries the next measure line. The tolerance is real time, not
beats.

Lines 58 to 74: accept the chord. Remember its index so the next search starts there. Pick the chord's
activation note with F3 (the highest drum pad in the chord). If F3 returns a note, set the `DRUM_ACTIVATION`
flag on it (1.0 value 0x100) and add a fill: start time (half a measure before the line), length in seconds
(time of the activation note's tick plus one 64th note, minus the start time), start tick (the start time turned
into a tick), length in ticks (note tick plus the 64th note minus the start tick), flag on. If F3 returns nothing,
no fill is added for this line and the loop still moves on by 4.

Line 76: return the list.

What this means: a generated 1.0 fill covers half a measure of real time up to the chosen chord and runs one
64th note past the chord's activation note. It is not the same shape as the 1.1 generator, which starts the fill
on the measure line (see `docs/adr/0010-legacy-fill-deadline-is-a-cli-only-mode.md`, 2026-10-15 note).

## F2: written fills, `TrackBuilder.PlaceWrittenFills` (lines 80 to 113)

Lines 84 and 85: `num` is `6 * resolution / 192` ticks (a 128th note at 192) and `num2` is `12 * resolution / 192`
ticks (a 64th note).

Lines 86 to 91: for each written fill in the track, skip it when the chart has a "coda" object and the fill starts
on exactly the coda's tick (the object `Coda` is created from an event named "coda" at
line 432 of the `Chart` class's file in the full decompile). That fill is the big rock ending, not a normal fill.

Lines 92 to 94: `num3` is the fill's end tick (start plus length). The method looks for the chord nearest to the end
tick with F4's rule but with limits: it must not lie before the fill's start tick and must not lie after the end
tick plus `num` (the call is `chords.NearestChordInRange(num3, fill.tick, num3 + num)`; its body is in the full decompile at
lines 50 to 88 of the `ChordListSearch` class's file and is not copied here). If both neighbours of the end tick fall outside the limits
it returns nothing and the fill is dropped. If only one is inside, that one is taken; if both are inside the nearer
wins and a tie goes to the later chord.

Lines 95 to 112: with a chord found, pick its activation note with F3, set the 0x100 flag, and add a fill whose
start is the written fill's start, whose length in seconds runs from the written start to the activation note's
tick plus a 64th note, and whose length in ticks is the note's tick minus the start plus a 64th note. Note that the
fill's length is set by the activation note, not by the length written in the file.

## F3: the activation note, `TrackBuilder.PickActivationNote` (lines 117 to 141)

The chord's lane mask is `LaneMask`. `num & 0x7F` is its pad lanes and `num & 0x180` is its two kick bits. If there
is any pad lane, the target lane is the highest set bit of the pad lanes (`BitMath.HighestSetBit`, the bit
smearing at lines 636 to 644 of the `BitMath` class's file). If there is none and the second argument is true (both callers pass
true), the target is the lowest set kick bit (`LowestSetBit`, `x & ~(x - 1)`, line 377). The method returns the first
note in the chord whose lane equals the target, or nothing. These two helpers are not copied.

## F4: the nearest chord, `ChordListSearch.NearestChord` (lines 144 to 166, helpers 170 to 193)

`FirstAfter` (lines 170 to 180) returns the index of the first chord whose tick is strictly after the target
tick, or 0 (not -1) when there is none. `LastAtOrBefore` (lines 184 to 193), called with its flag true, returns the index
of the last chord whose tick is at or before the target, or -1. `NearestChord` compares the two: with neither it
returns -1; with only one it returns that one; with both, it returns the earlier chord when the later one is farther
from the target than the earlier one is, else the later chord. A chord exactly on the target is the "at or before"
one at distance 0, so it wins. A tie goes to the later chord. Because `FirstAfter` returns 0 when no chord follows,
a target after the last chord compares chord 0 against the last chord and picks chord 0 (the far one); the 0.25 s test
in F1 then throws it out.

## F5: the measure lines, `BeatGrid.BuildLines` (lines 197 to 255)

Lines 199 to 204: start at tick 0 and run to the chart's last tick. Lines 205 to 253 walk one beat at a time; the step
is `resolution * 4 / denominator` ticks (line 251). Line 207 reads the time signature at the current tick and line 209
sets `flag` when it is the first beat or the numerator or denominator differs from the previous beat's. Lines 210 to 214:
on a change both counters (the beat within the measure and the beat since the change) restart at 0, so the measure grid
restarts at the first beat of a new time signature. Lines 215 to 218: when the beat within the measure reaches the
numerator it goes back to 0. Line 219: a beat that is first in its measure (or the first after a change) is a measure
start. Lines 221 to 231: the one exception is a numerator of 1 once at least one beat has passed since the change, where
the beat is stored as BEAT_STRONG or BEAT_WEAK instead. Lines 232 to 239: otherwise the beat is stored as MEASURE (line
238); if the previous beat's denominator was above 4 its stored kind is first overwritten with BEAT_WEAK (line 236), which
changes the kinds list only, not the list of measure times. Lines 241 to 248: every other beat is BEAT_STRONG or BEAT_WEAK
(strong when the denominator is 4 or less or the beat count divides evenly by denominator / 4). Each stored beat adds its
time in seconds to a list; a MEASURE also adds that time to the measure-line list `measureLines` that F1 walks (the
class `BeatGrid`, method `AddLine` at full-decompile lines 28 to 36). I read the field holding the numerator as
`numerator` and the denominator as `denominator` from how the step uses them; the chart parser's fields were not traced.

Limits of this reading: the obfuscator renames every field, so the role names above are my reading of how each is used.
The numbers 12, 6, 192, 0.25 and 4 are literals in live code. The dead twin `DrumsPlayer.UnusedFillCheckTwin` has scrambled
literals (491, 1618, 1219) and is never called anywhere in the four decompiled assemblies (Assembly-CSharp, firstpass, StrikeCore, CHNet.Core; grep of the whole output
found only its definition), so it is not part of any rule.
