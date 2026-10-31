# end_event_usable_212C7E0 (build 6142, made 2026-10-30 with tools/ch_mcp/chdis.py)

Function: `#8538.ʶˁʻʷʼʷʶʻʳʳʿ()` on the chart class (`dump.cs` line 543589), RVA `0x212C7E0`; the first instruction is the function start. Decompiled at `hydra-data\scratch_ms\ghidra_decompile_all.txt` lines 14779 to 14791 (`FUN_18212c7e0`). It says whether the chart's `[end]` event is allowed to end the song. The end event object is at chart+0x108, and its tick is the `long` at +0x10 of that object. The setter `0x212E310` (decompile lines 15845 to 15867) stores only the first end event: it fills +0x108 only when it is still null.

Lines 1 to 2: prologue; the end event pointer at chart+0x108 is tested.

Lines 3 to 6: no end event means the answer is false (`xor al, al; ret`).

Lines 7 to 10: the end event's tick is read into rbx; `ChartLength(addMargin = true)` (`chart_length_212E130`) is called (line 10, `dl = 1` at line 9).

Lines 11 to 13: the answer is true when `end tick >= ChartLength(true)` (`cmp rbx, rax; setge`), so a tick exactly equal counts. `ChartLength(true)` is the last gem end of every instrument track plus the truncated margin at chart+0xB0.

Lines 14 to 18: epilogue.
