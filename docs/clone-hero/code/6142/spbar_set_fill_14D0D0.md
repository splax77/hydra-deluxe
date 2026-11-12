# SPBar fill update, RVA 0x14D0D0 (build 6142)

Made with `py -m tools.ch_mcp.chdis 14D0D0 2A0` from the worktree root (session 428cf079, task CH-UNK-W10b). Two short
callers are beside it: `baseplayer_spbar_update_208EA0.asm` (`py -m tools.ch_mcp.chdis 208EA0 23`) and
`baseplayer_spbar_reset_2093D0.asm` (`py -m tools.ch_mcp.chdis 2093D0 80`).

Function: `SPBar.ʸʿʲˁʴʾˁʴʻʳʼ(float value, bool active)`, `dump.cs` line 36569, in class `SPBar` (TypeDefIndex 676,
lines 36527 to 36619). The class's fields are `starPowerBar` (a SpriteRenderer, +0x20), `arrowTopPosition` (+0x28),
`arrowHalfPosition` (+0x34), `arrowBottomPosition` (+0x40), `topArrow` (+0x50), `bottomArrow` (+0x58) and an unnamed
float (+0x60).

## Plain version

This is a Star Power meter being drawn. Given the meter amount (0 to 1) and whether Star Power is active, it sets the
bar sprite's shader fill height to the amount, turns on an "electricity" effect when Star Power is active or the
amount is at least one half, shows or hides two arrow objects, and slides the top arrow up the bar with the amount.

## Lines

Lines 4 to 14 look up the two shader property names, `_uElectricityEffect` and `_uFillHeight`, once. Lines 15 to 17
return at once when the amount equals the last one stored at +0x60; line 22 stores the new one. Lines 21 to 33 call
`starPowerBar.material.SetFloat("_uFillHeight", amount)`. Lines 39 to 56 call
`SetFloat("_uElectricityEffect", x)` with x = 1 when `active` is true (lines 43 to 46) or when the amount is at least
0.5 (lines 47 to 50), else 0 (line 51). Lines 57 and 58 jump to lines 149 to 161 when the amount is 0 or less; that
branch hides both arrows. Otherwise lines 59 to 69 set `topArrow` active when `active` is true or the amount is at
least 0.5, and lines 70 to 76 set `bottomArrow` active when the amount is above 0. Lines 77 to 140 move the top arrow:
at an amount up to 0.5 it goes from the bottom position toward the half position by twice the amount, clamped to 0..1
(lines 87 to 106); above 0.5 it goes from the half position toward the top position by twice (amount minus 0.5)
(lines 108 to 131). Line 140 sets the arrow's local position. Line 162 is the shared call every null test jumps to;
it does not return (line 163 is `int3`).

## Callers

Direct callers only (byte scan), from `docs/audit/2026-11-08-clone-hero-unknowns/W10b/scan_callers.py`, output in
`spbar_callers.txt` beside it. Five sites jump to or call 0x14D0D0: SPBar.Start (0x14BA5B), SPBar's own method at
0x14BEF0 (0x14C358), and three BasePlayer methods. BasePlayer (TypeDefIndex 901, the base of DrumsPlayer and of the
guitar players) holds the bar as `protected SPBar spBar` at +0x90 (`dump.cs` line 48742).

`BasePlayer.ʷʽʶʺˀʿʺʽʲʸʶ(float, bool)` at 0x208EA0 (`dump.cs` line 48792, virtual slot 21) is a forwarder: it loads
`spBar` (line 2 of its listing), and when it is not null jumps to 0x14D0D0 with the same two arguments (lines 3 to 7);
when it is null it calls the failure call at line 8.

`BasePlayer.ʺʾʼʵˀʶʲʴʾʿʽ()` at 0x2093D0 (`dump.cs` line 48828, virtual slot 29) resets the HUD pieces: when the player is
active and enabled (lines 18 to 20) it calls 0x14D0D0 with an amount of 0 and `active` false (lines 21 to 27), then
jumps to `SpNeckRenderer` at 0x14E3B0 (line 34). A null `spBar` there also goes to the failure call (lines 22, 23 and
38).

The third BasePlayer site is 0x209C33 in `BasePlayer.ʾʴʸʽʺʵʳʳʳʴʿ` at 0x209A70 (`dump.cs` line 48882), the method that
takes a saved state object; it was not listed. Who calls 0x208EA0 was not scanned.
