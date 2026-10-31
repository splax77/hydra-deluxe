# sp_gain_20F4FA0: the drum engine's Star Power gain, and why a bot never earns any (build 6142)

Listing: `sp_gain_20F4FA0.asm` beside this file, made 2026-10-30 (session b02ab3e6-76ac-4735-8576-04b87f32bb8d)
with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. The listing starts at the function's
first instruction (RVA `0x20F4FA0` is in `script.json` as a method start). Our readable name for it is `#8288.AddStarPower`
(class `BaseEngine`, TypeDefIndex 8288, the engine base class that `BasePlayer.engine` holds; `dump.cs` line 529952 is
the class and the method is the one declared as a `public virtual void` method, under its scrambled name, with the parameters `(long ..., bool ... = False)`).
Its only Ghidra caller in the surviving decompile is `FUN_1820ddb70` (the deadline function, `ghidra_decompile_all.txt` lines
670 to 681, call at line 681).

Plain version: when the engine is asked to add Star Power meter, it first looks at which kind of player the engine belongs
to. If the kind is "Bot" or "RemotePlayer" it returns at once and adds nothing. Only a local human player or a replay
reaches the code that adds meter.

Lines of the listing:

Lines 1 to 12 are the usual start: save registers, and the one-time IL2CPP class set-up check (the byte at `[rip + ...]`).

Lines 13 and 14: compare the engine's player-kind field, the 4-byte value at offset `+0x190`, with 2. The values are the enum
`#8553` (`dump.cs` line 546294): Player 0, RemotePlayer 1, Bot 2, Replay 3. If it equals 2 (Bot) the code jumps to line 58, the
closing code that restores registers and returns (the ret is line 62). Nothing was added.

Lines 15 and 16: the same compare with 1 (RemotePlayer), same jump.

Lines 17 and 18: if the gain amount (the `long` argument in `rdx`) is negative, jump to the throw path, which builds an
`ArgumentOutOfRangeException` with the text 'Star power gain amount must be greater than 0' (the lines after 62).

Lines 19 to 22: add the amount to the meter at `+0x68` and to a running total at `+0x80`. The unit is ticks (see page 5, the
overlay spike's "30 x resolution" row).

Lines 23 and 24: when the second argument (the "award by phrase" flag, `r8b`) is true, jump to line 37, which raises the phrase
counter at `+0x4C` and then notifies the listener at `+0x1A0` (event 1).

Lines 25 to 36: when the flag is false: if Star Power is already active (`+0x42` set) skip the notice. Otherwise, if the
meter has just crossed the half-full mark (the half is the long at `+0x1B8`: before the add it was below it, after it is at or
above it) notify the listener with event `0xD` ("Star Power available").

Lines 44 to 57: set the "meter changed" byte at `+0x130` and send the listener the new meter ratio (meter divided by the long at
`+0x1B0`, the full bar) and the active flag (event `0xF`).

Line 62 is the `ret`; the lines after it are the exception path.

What this proves and what it does not. It proves that the bot (engine kind 2) can never add meter by any route that goes
through this method. It does not prove that no other method adds meter, so the claim "the bot never uses Star Power" stays
backed by the live run as well (feasibility study lines 117 to 129: 11 phrases offered, 0 earned, 0 activations). That the
bot's engine really holds kind 2 was not traced in this pass: the field at `+0x190` is written once in the constructor at
0x20F7BCB (`mov dword ptr [rsi + 0x190], eax`, found by a scan of the class's methods) and which caller passes 2 for a
bot player was not followed.
