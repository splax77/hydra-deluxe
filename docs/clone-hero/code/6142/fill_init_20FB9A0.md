# fill_init_20FB9A0.asm (build 6142, RVA `0x20FB9A0`)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (5 lines). Starts at the method's first instruction. `dump.cs` lines 530466 to 530512 list sixteen scrambled methods of the written-fill class #8292 at this one address, each taking (`long`, `long`, `bool`); the class's only own field is the `bool` at `+0x20` (`dump.cs` line 530462), whose name is the same as the third parameter's.

Plain words: store the first `long` (the fill's start tick) at `+0x10`, the second (its length) at `+0x18`, and the `bool` at `+0x20`, then return the object. This `+0x20` byte is the one the landing step `0x20CFF60` tests to pass a fill over (`fill_to_activation_20CFF60.asm` line 86).

- Line 1: `+0x10` = `rdx`.
- Line 3: `+0x18` = `r8`.
- Line 4: `+0x20` = `r9b`.

Direct callers (byte scan, 2026-11-08): 0x20CEE3D in the `.mid` fill helper 0x20CEDC0, 0x20E6078 in `0x20E5E70`, 0x213E2A6 in the `.chart` reader `0x213E0C0`, 0x215B83D in `0x215B7C0` and 0x215BDCD in `0x215BD50`. The `.mid` reader passes 0 (`mid_drum_reader_2155050.asm` line 159, `xor r9d, r9d` before the helper call), and the `.chart` reader passes 0 (`docs/audit/drum-lanes/spec_213e0c0.asm` line 109). `0x20E5E70` and the reader behind `0x215B7C0` (`fill_cache_caller_20E49D0.asm`, the virtual call then `cmp eax, 1` / `sete`) pass a byte read back from a stream.
