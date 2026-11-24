# Clone Hero 1.0 (build 4045): the song clock, and the Star Power sizes, at a song speed

Source: `song_clock.cs` beside this file: lines copied unchanged from `ilspycmd` 11.0.0.9375 output of
`C:\Clone Hero\1.0 old\Clone Hero\Clone Hero_Data\Managed\Assembly-CSharp.dll` (MD5 `a5e7064d6c770e010a83b83d56d99c31`),
decompiled 2026-11-23, session 46353d99 (task song-speed-r1), with `ilspycmd -p -o <empty folder> <that dll>`. The line
numbers in each section head are lines of the per-class files that command writes. Build 4045 has no addresses; cite a
rule as `Game code (4045): Class.Method`. The game's names are scrambled. The names on this page are our own, given here;
the ones not already in `../names-4045.tsv` are not added to it. Line numbers below are lines of `song_clock.cs`.

The song speed setting is the same object `fill_generator.cs` F9 multiplies the timing resolution by (its
`GetFloatPercent`, 1.0 at 100%). "Song time" is `GameManager.songTime`, the clock the fill deadline is handed.

## Plain version

The 1.0 game's song clock counts the audio's position divided by the song speed. Note times are counted the same way,
because the timing resolution is the file's resolution times the speed. So the clock and the notes agree, and both run at
one second per real second. The fill deadline is handed this clock, so its 0.25 s, its 10 s and the fill's length are all
real seconds.

Star Power sizes do not change with the speed. A phrase adds a quarter of the meter. The drain takes the whole meter over
eight measures, and the sustain gain fills it over 30 beats, both counted in the file's own ticks. The clock's tick comes
back to the file's own tick because both the seconds and the resolution carry the same factor.

## C1 to C4: what the deadline is handed, and where it comes from

C1 (lines 6 to 10) is the drums player's Star Power gain, the same lines as `fill_deadline.cs` lines 9 to 11: when the
meter is about to cross one half, it calls the deadline check with the game manager's song time (line 9). That call is
the deadline's only caller in the decompile. C2 (line 13) is the song time field, a double.

C3 (lines 16 to 23) is from the song-load coroutine. Line 21 sets the clock's divisor to the song speed. Line 22 then reads
song time from the clock. C4 (lines 26 to 46) is `GameManager.Update`, every frame: line 26 keeps last frame's chart tick,
line 38 ticks the clock, line 39 reads song time from it again, and line 46 turns song time into this frame's chart tick
with the seconds-to-tick method (`fill_generator.cs` F9, lines 600 to 617).

## C5 to C9: the clock

C5 (lines 49 to 69) holds the clock's divisor (line 49), its rate, 1 by default (line 51), the divisor's setter (lines 54
to 60) and its readouts. Song time is the clock's running value plus one offset minus another (lines 65 and 67). Both
offsets are set at song load (C3 lines 18 and 19); this page does not say what they hold.

C6 (lines 72 to 105) is the tick C4 calls. Line 74 takes the audio position and divides it by the divisor, the song speed.
Line 75 moves the running value on by the frame's Time.deltaTime times the rate. Lines 76 to 104 then nudge the running
value toward line 74's target whenever the two differ by more than 0.003 s (a threshold field at line 12 of the clock class's file, not
copied here). So the clock follows the audio position over the speed.

C7 (lines 108 to 116) is the audio position: a value read through an audio wrapper class that lives outside
Assembly-CSharp, which was not decompiled, so its unit is not read here. C8 (line 119) sets the song's end to the audio's
length over the song speed, the same division as C6 line 74.

C9 (lines 122 to 150) is the only place the rate changes. Two buttons step it by 0.05. Line 123 leaves the loop before
either button is read whenever the song speed is not 100%, so at any other speed the rate stays 1.

## S1 to S8: the Star Power sizes

S1 (lines 153 to 160) runs every frame from C4 line 42. It sets the drain's base to the file's resolution
(`resolution`, F9's second field) times 4 over the meter's bottom number, times its top number, times 8: eight measures
of the file's own ticks. S2 (lines 163 to 170) is the drain for one frame: this frame's chart tick minus last frame's
(C4 lines 46 and 26), cut to a whole number, over S1's base. S3 (lines 173 to 177) applies it while Star Power is active,
and S4 (lines 180 to 191) takes it off the meter, which runs from 0 to 1.

S5 (lines 194 to 198) is a finished phrase: it adds a fixed 0.25, a quarter of the meter.

S6 (lines 201 to 203), at chart load, sets the sustain gain's base to 30 times the file's resolution. S7 (lines 206 to
213) is the gain for one frame: the same chart-tick difference as S2 over that base. S8 (line 216) is its only caller,
in the guitar player; drums do not use it. Line 202 of S6 sets a guitar-only value from the timing resolution; it is not
a Star Power size.

## T1: why the clock's tick is the file's own tick

T1 (lines 219 to 234) adds a tempo to the chart. Its start second (line 232; line 227 is the first tempo at tick 0) is the previous tempo's start second plus the
ticks between them turned into seconds with the timing resolution, the file's resolution times the speed (`fill_generator.cs`
F9, lines 541 to 545). So at speed s every tempo starts at its 100% second divided by s. Song time is also the 100% audio
second divided by s (C6 line 74). The seconds-to-tick method (`fill_generator.cs` lines 600 to 623) takes the gap from the
tempo's start second, multiplies it by the tempo and by the timing resolution, so the two factors of s cancel and the
result is the file's own tick. S2 and S7 count those ticks, so the drain and the sustain gain are the same number of
beats at every speed.
