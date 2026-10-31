# Star Power per-update step, RVA 0x20F6800 (build 6142), vtable slot 12

Class `#8288`. Listing: `sp_update_20F6800.asm` (chdis, 2026-10-30). Decompiled lines: `ghidra_decompile_all.txt` 53491 to 53577. Fields (dump.cs lines 529952 to 530047): +0x42 SP active flag, +0x68 the meter, +0x80 a running long, +0x110 activation time (double), +0x118 activation tick, +0x120 SP end time (double), +0x130 "meter was clamped" flag, +0x138 and +0x140 ticks since activation (previous and now), +0x190 the player kind (enum `ʳʴʾʷʸʼʷʹʴˁˀ`, dump.cs line 546294: Player 0, RemotePlayer 1, Bot 2, Replay 3), +0x1A0 listener, +0x1B0 the full bar.

In plain words: once per update the game does one of two things. If Star Power is off, it only makes sure the meter is not above a full bar. If Star Power is on, it drains the meter by the ticks that have passed since the last update, caps the meter at a full bar, and, only when the meter was capped, works the end time out again.

Lines 5 to 8: branch on the active flag (+0x42). Zero falls through; non-zero jumps to line 24.
Lines 9 to 15 (not active): excess = meter (+0x68) minus the full bar (+0x1B0). If it is above zero, subtract it from +0x80 and set the meter to the full bar. The meter never stays above a full bar.
Lines 16 to 23: copy the meter to +0x60 and return.
Lines 24 to 26 (active): previous = old +0x140, stored at +0x138.
Line 27: call 0x20F3750: the current song tick (clock to ticks via the same conversion as above; 0 when the clock is negative; read 2026-10-30).
Lines 28 to 32: +0x140 = current tick minus the activation tick (+0x118).
Lines 33 to 36: one-time init of a type-info slot.
Lines 37 to 38: if the player kind is 1 (RemotePlayer) skip the drain and go to line 69. Player, Bot and Replay all drain.
Lines 39 to 40: delta = new +0x140 minus the previous one; non-negative goes to line 50.
Lines 41 to 49 (negative delta): if the object at +0x228 is null, throw (line 107). Otherwise compare +0x108 with the value 0x215FED0 returns; if +0x108 is not above it, go to line 109 (throws "Star power drain amount must be greater than 0", lines 109 to 129); if it is above, delta becomes 0 (line 49).
Lines 50 to 52: delta is capped at the meter, so the meter cannot go below zero.
Lines 53 to 56: meter = meter minus delta (line 56).
Lines 57 to 68: if a listener exists, send event 0xF with meter divided by the full bar (a float).
Lines 69 to 76: after the drain, if meter is above the full bar: +0x80 minus the excess, meter = full bar (line 75), and +0x130 = 1 (line 76). This is the clamp when a phrase overfilled the bar during Star Power.
Lines 77 to 78: if +0x130 is zero, skip to line 16.
Lines 79 to 86: recompute the end time: 0x20F37E0 with (ticks since activation + meter) and the activation time (+0x110); store at +0x120 (line 85).
Lines 87 to 104: convert the new end time to a tick position and call 0x20F3750; neither result is stored (the decompile at 53563 to 53571 shows both results dropped).
Line 105: clear +0x130.
Lines 107 to 108: null-reference throw. Lines 109 to 129: the exception above.

What it does not do: it never clears the active flag and never writes zero to the meter on purpose. Star Power is ended by the deactivate method at 0x20F5490 (committed listing `docs/audit/drum-lanes/play/f20f5490_deactivate_sp.asm`), which sets the flag to 0, halves the multiplier and zeroes the meter. What calls the deactivate method was not found: a byte scan for direct calls found only jumps from three generic engine wrappers (RVAs 0xFAD470, 0xFBBFD0 and 0xFBC0B0), none from the drums class. The end-time check in 0x20F6A50 (see `advance_events_20F6A50.md`) sends a virtual event at the end time; that this event runs the deactivate method is not proven here.
