# player_menu_on_ready_page_32F590.asm (build 6142, RVA `0x32F590`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 32F590 200` (4 lines). Starts at the method's first instruction: `PlayerSelectionMenu`'s public bool ʸʳʼʹʸʽʾˀʴʼʷ() (`dump.cs` lines 73000 and 73001; class header at line 72822). No other method in `dump.cs` has this RVA.

Plain words: it answers "is this player screen on its Ready page". It returns true when the page field at +0x120 holds 4, which is the page enum's Ready word (`dump.cs` lines 72794 to 72803, IN-56). It changes nothing.

- Line 1: compare the page field `[this+0x120]` with 4.
- Line 2: return 1 when equal, else 0.
- Line 3: return. Line 4 is padding.

Who calls it, direct callers only (byte scan with `docs/audit/2026-11-08-clone-hero-unknowns/W9a/callscan.py`, output kept beside it as `callers_32F590.txt`): six `call`s and no `jmp`. Four are in `PlayerSelectionCanvas` (TypeDefIndex 452): its `Update` at 0x0D0EFF (`player_canvas_update_D0E20.asm` line 54) and three unnamed methods at 0x0CFB1D, 0x0D00EF and 0x0D0B38. Two are in `PlayerProfileMenu`, at 0x1E2BE9 and 0x1E6F29. The only pointer to it is its entry in the `.data` method pointer table (file offset 0x3C59270), which every method has. The three unnamed canvas methods and the two profile-screen methods were not read.
