# Clone Hero 1.0 (build 4045): the bot test, and why the 1.0 bot earns no Star Power

Source: `bot_test.cs` beside this file: lines copied unchanged from `ilspycmd` 11.0.0.9375 output of
`C:\Clone Hero\1.0 old\Clone Hero\Clone Hero_Data\Managed\Assembly-CSharp.dll` (MD5 `a5e7064d6c770e010a83b83d56d99c31`),
decompiled 2026-11-11, session 428cf079 (task CH-UNK-W10b), with `ilspycmd -p -o <empty folder> <that dll>`. The line
numbers in each section head are lines of the per-class files that command writes; they match the line numbers the
reference build's decompile gave for the same call sites (BasePlayer.cs 636, 755 and 1104, GameManager.cs 355).
Build 4045 has no addresses; cite a rule as `Game code (4045): Class.Method`. The names on this page are our own, given
here; they are not yet in `../names-4045.tsv`.

## Plain version

The 1.0 game asks one small helper "is this player the bot?" before it adds Star Power. The helper reads the player
profile's `isBot` setting and nothing else. When it says yes, the gain method does nothing at all. A command-line start
sets `isBot` to 1. So in 1.0 the bot never fills its Star Power meter, from phrases or from anything else that goes
through the gain method.

## B1: the bot test, `PlayerExtensions.IsBot` (whole file, 7 lines)

The scrambled static class `ʳʼʽʼˀʴʽʵʵʳʵ` holds one extension method, `ˀʵʲʲʻʸʸʽʹʽˁ`, called here `IsBot`. It takes a
player slot (class `ˁʻʽʷʽʻʾʵʷˀʶ`) and returns that slot's profile's `isBot` setting (line 5 of the original file). The
profile field `ʲʼʽʺʿʹʽʻʵʺʶ` holds a `ʹʺʿʻʲʹʻʹʹʵʸ`, which derives from `BasePlayerProfile`; `isBot` is not declared in
Assembly-CSharp, so it comes from that base class, which lives in another assembly that was not decompiled for this page.
The `(bool)` cast at BasePlayer.cs line 319 and the `CurrentValue = 1` write below show it is a setting with a
number value that reads as a bool.

## B2: the gain method's test, `BasePlayer.AddStarPower` (BasePlayer.cs lines 753 to 756)

This is the same method as F8 in `fill_deadline.md`. Its whole body sits inside one `if`: the player is not a remote
player and `IsBot` is false (line 755). So for a bot the method returns without touching the meter.

## B3: the command line makes a bot (CliGameLauncher.cs lines 680 to 685)

The command-line launcher copies a profile into the player slot and sets `isBot.CurrentValue = 1` on it (line 684).
That is the start the tools use for 1.0 bot runs.

## B4: nothing else adds to the meter

A search of every per-class file for an assignment to the meter field `ʹˀʼʵʳˀʻʼʵʿʵ` (`=`, `+=`, `-=`) found five
lines, all in BasePlayer.cs. Only line 759 adds, and it is inside B2's test. Line 283 drains it during Star Power,
line 533 resets it, line 771 caps it at 1 inside B2, and line 859 copies it back from a saved state whose only write
(line 596) stored this same meter. The callers of the gain method are BaseGuitarPlayer.cs line 337 (an amount from a method not read here),
BasePlayer.cs line 915 (0.25 with the award flag, after the STAR_POWER_END flag is cleared) and the drums override in DrumsPlayer.cs line 876, which ends by calling the
base method, so every path passes B2's test.

## Other callers of the bot test

BasePlayer.cs lines 636 and 1104, GameManager.cs lines 355 and 359, GlobalVariables.cs lines 436 and 1170,
PlayerSelectionMenu.cs lines 763 and 768, PlayerStatsBox.cs lines 264, 269, 459 and 464, and the scrambled class file
`ʹʵʽˁʼʺʷʿʷʲʻ.cs` line 134. They were found by a text search for the method name over the whole decompile and were not
read for this page, except that the name lines in PlayerSelectionMenu and PlayerStatsBox colour a bot's name cyan.
