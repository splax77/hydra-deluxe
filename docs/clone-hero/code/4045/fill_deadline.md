# Clone Hero 1.0 (build 4045): the fill deadline, when a fill is thrown away

Source: `fill_deadline.cs` beside this file: three methods copied unchanged from `ilspycmd` output of
`C:\Clone Hero\1.0 old\Clone Hero\Clone Hero_Data\Managed\Assembly-CSharp.dll` (decompiled 2026-10-30, session
b02ab3e6-76ac-4735-8576-04b87f32bb8d, scout R8). Build 4045 has no addresses; cite a rule as
`Game code (4045): Class.Method`. The game's names are scrambled, so the names on this page are our own;
`../names-4045.tsv` maps each one to the scrambled name that `fill_deadline.cs` uses. Line numbers are lines of
`fill_deadline.cs`.

The fill objects are those `fill_generator.md` describes: each has a start time in seconds, a length in seconds,
the note it ends on, and an "on" flag. The song clock is the game manager's song time, `GameManager.songTime`, in seconds.

## F6: when the check runs, `DrumsPlayer.AddStarPower` (lines 7 to 14)

This is the drums player's version of the method that adds Star Power meter (the base version is F8). The meter
`starPowerAmount` runs from 0 to 1. Line 9: when the meter is below one half now, would be at or above one half after this
gain, and Star Power is not already active (`starPowerActive`), the game calls the deadline check, F7, with the current song
time (line 11). Line 13: then it adds the gain through the base method, with the "award" flag set to true.

So the deadline is decided once, at the moment the meter first crosses one half on the way up. It is not re-run on
later frames while the meter stays above one half. Each time the meter drops below one half and climbs back over
(after an activation, for example) the check runs again.

## F7: the check, `DrumsPlayer.CheckFillDeadlines` (lines 17 to 31)

`fills` is the track's list of fills, in time order. For each fill, with `now` the argument:

Lines 21 to 24: if the fill starts more than 10 seconds after now, stop. That fill and every later one are left
untouched (they stay available).

Lines 25 to 29: otherwise, if the fill starts earlier than `now + length` (its length in seconds, times 1) or earlier
than `now + 0.25`, the fill is thrown away. The game turns its "on" flag off (line 27) and clears the
activation flag (`DRUM_ACTIVATION`, 0x100 in this build) from the note it ends on (line 28), so that note no longer
offers a Star Power activation.

Put together, a fill survives when it starts at least `max(length, 0.25 s)` after now, with the length counted as
the fill's own length up to a cap of 10 s, because fills more than 10 s ahead are never examined and a fill whose
length is over 10 s is removed whenever it starts within 10 s. In plain words: Star Power must be half full about
one fill-length before the fill starts, at least 0.25 seconds before, and no more than 10 seconds before. All of this
is in real time. The length is the fill's own length from `fill_generator.md` (half a measure of real time plus one
64th note for generated fills; the written start to the activation note plus one 64th note for written fills), so the
64th note that earlier write-ups of this rule list is part of the length, not a separate term.

## F8: the meter gain, `BasePlayer.AddStarPower` (lines 34 to 57)

Read only to know what the meter holds. Line 36 skips a remote player and any player for which `PlayerSkipTest()` is true (what that tests was not found). Line 38
remembers whether the meter was below one half with no activation. Lines 40 to 49: the gain is added; when the
call came from a phrase award, the awards counter goes up and the game raises the "Star Power awarded" event; when it
did not and the meter just passed one half, it raises "Star Power available". Lines 50 to 53: the meter is capped
at 1. Line 54 adds the gain to a running total. The drums override (F6) always passes the award flag as true, so on
drums the "available" branch at line 46 never runs.

## Not part of the rule

`DrumsPlayer.UnusedFillCheckTwin` (full decompile lines 275 to 289) has the same shape with the literals 491, 1618 and 1219 and
clears chord flags rather than the activation flag. No call to it exists in any of the four decompiled assemblies
(Assembly-CSharp, Assembly-CSharp-firstpass, StrikeCore and CHNet.Core were decompiled and searched by name), so it is dead code and I did not
treat its numbers as a rule.
