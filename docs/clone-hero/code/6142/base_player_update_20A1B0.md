# Player update, RVA 0x20A1B0 (`BasePlayer` Update, slot 40)

Listing: `base_player_update_20A1B0.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. `dump.cs` line 48872 gives this RVA; script.json names it `BasePlayer$$Update`. It is the per-frame update of the player object, and it calls the engine's frame update `0x20F4550` three times.

- Lines 100-119: a loop. While virtual slot 0x378 of the player returns true, call the frame update on the engine at +0x120 with the time slot 0x378 wrote and `r8b = 1` (lines 111-114).
- Lines 120-143: a second loop of the same shape around `0x208F50`, calling the frame update with `r8b = 1` (lines 135-138).
- Lines 144-155: when neither loop ran, call the frame update once with the time at +0x70 of the object at +0x20 and `r8b = 1` (lines 152-155).

So every frame update from here gets `true` as its bool argument, the one the frame update stores at engine +0x290 (`base_frame_update_20F4550.asm` lines 5 and 14).
