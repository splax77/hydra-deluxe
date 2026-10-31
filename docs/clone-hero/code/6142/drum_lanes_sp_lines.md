# Star Power lines in the committed drum-lanes listings

Annotations only; the committed `.asm` files under `docs/audit/drum-lanes/` are not edited. Made 2026-10-30 by scout R4. I searched every `.asm` there for the engine's Star Power fields (+0x42, +0x48, +0x68, +0x1B0, +0x1B8, +0x120, +0x130, +0xC8, +0xCC) and for calls to the Star Power functions, by `grep -E` over the folder. Most field hits are other classes' fields at the same offsets (for example `f20d12c0.asm` +0x68 is a stack slot). These are the ones that touch Star Power.

`docs/audit/drum-lanes/play/f20deec0.asm` is the drums score function, class 8255, RVA 0x20DEEC0.
- Lines 13 to 16: ask the note whether it is a cymbal (0x18214BE30, which tests three lane-mask bits); if yes the base value in esi gets +15 (`lea ebx,[rsi+0xf]`), else stays.
- Lines 19 to 24: ask the note a second yes/no question (0x18214BB40, not disassembled by this scout; session 1bae448b, 2026-10-11, read it as the ghost or accent test; that listing is not committed); if yes the value is doubled (`lea r8d,[rbx+rbx]`).
- Line 31: `jmp 0x1820f6eb0` tail-calls the base engine's AddScore (`add_note_score_20F6EB0.asm`). So the drums engine's whole Star Power split (multiplier halving, the Star Power share) is the base class's code, not drums code.

`docs/audit/drum-lanes/f20de770.asm` is the drums hit routine, RVA 0x20DE770, slot 11.
- Lines 117 and 125: calls to 0x20F5FF0 (sends event number 4 to the engine's listener when one exists; decompile 53142 to 53157) and to 0x20F63F0 (the base hit routine; its committed listing is `play/f20f63f0_base_hit.asm`).
- Lines 126 to 130: `mov r8d, 0x32` puts the base value 50 (0x32) in the third argument before the virtual score call through the vtable at offset 0x1A0. The 50 is the literal Hydra's `kNoteBasePoints` holds.

`docs/audit/drum-lanes/play/f20deaa0.asm` is the drums press handler, RVA 0x20DEAA0.
- Lines 115 to 120: if the player kind (+0x190) is not 1, call 0x20F57F0 (the "can Star Power be activated" test named in FINDINGS-play-time.md line 46; not read by this scout) and use its answer at line 123 to decide whether the fill-end note press activates Star Power first. The activation itself is `ActivateStarPower` (vtable slot 14, 0x20F5640; `sp_activate_20F5640.asm`).

`docs/audit/drum-lanes/f20dcad0.asm` (0x20DCAD0), line 7: `call 0x1820f57f0`, the same can-activate test, used for the automatic hit of a missed fill-end note (FINDINGS-play-time.md line 46).

`docs/audit/drum-lanes/play/f20f5490_deactivate_sp.asm` is the base engine's SP deactivate, RVA 0x20F5490, committed.
- Line 14: `[+0x42] = 0`, the active flag cleared.
- Lines 9 to 16: the multiplier at +0x48 is halved (signed divide by two): the reverse of the doubling at activation.
- Line 17: the meter at +0x68 is set to zero.
- Lines 18 to 21: the double at +0xB8 gets (clock minus activation time) added: running total of time spent in Star Power.
- Lines 22 to 26: +0x128 = clock; +0x138, +0x140 and +0x120 (the end time) cleared.
- Lines 27 to 33: if a listener exists, tail-call the event sender with event 0xE (14).
This confirms the effect of ending Star Power (multiplier halves, meter to zero, end time cleared). It does not say what triggers it.

`docs/audit/drum-lanes/play/f20f63f0_base_hit.asm` line 215: `mov r9, [rax + 0x1b0]` reads a vtable slot (offset 0x1B0 of the vtable), not the engine's +0x1B0 field. Do not read it as the full bar.
