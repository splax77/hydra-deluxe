# fill_generator_20D2B30.asm (build 6142, RVA `0x20D2B30`)

Made 2026-10-30 with `tools/ch_mcp/chdis.py` (195 lines). Starts at the method's first instruction (`#8265.ʸˀʽʸʽʳʳʸʼʾʿ`, static: `(List<chord> notes, #8661 track, #8257 grid)`, returns `List<wrapper>`). 0x20D2D9D (the activation flag write) is line 135.

Plain words: when a track has no written fills, walk the measure lines of the grid. For each measure line, look for the chord nearest that line. If that chord is within one half beat plus one tick of the line, put the activation flag on it and record a fill that starts ON the line and ends at the chord. After a fill, jump four lines on; after a miss, step one line.

- Lines 1 to 30: stack set-up and the one-time generic-slot loads.
- Lines 31 to 39: `r13 = track`, `rdi = grid` (`r8`), `r14 = notes`. Null track or grid exits (line 193).
- Line 40: `rdi = [grid+0x20]`: the list of measure-line ticks (a `List<long>`). 0x20CF2B0 is the method that fills it: it adds a tick there only for a line of kind 0 (`grid_add_line_20CF2B0.md`).
- Lines 41 to 48: make the result list.
- Lines 49 to 52: when the chord list is empty (`[notes+0x18] == 0`), return the empty result list (lines 53 to 63).
- Lines 64 to 66: `rbx = [track+0x50]`: the chart object, whose `+0xb8` is the resolution (same field `20CFF60` reads).
- Lines 66 to 67: `xmm0 = resolution * 0.5`.
- Lines 70 to 72: `cvttsd2si rax` (truncate), `ebx = 1`, then `inc rax` at line 72. So the slop is `trunc(resolution / 2) + 1` ticks. This is the `+1` at 0x20D2C91. Line 74 saves it.
- Line 71 and 75 to 78: `ebx = 1` is the first line index (the second measure line; index 0 is never a candidate). The loop test at line 77 is `ebx < count(grid lines)`; when false, return the result list (lines 53 to 63).
- Lines 79 to 97: read the line ticks `L[ebx-1]` and `L[ebx]` twice each. `[rsp+0x28]` and `rdx` end up holding `L[ebx]`, the candidate line.
- Lines 98 to 103: call `0x5DDB50(notes, L[ebx], lastFoundIndex, method)`: the nearest-chord search (`nearest_chord_loop_5DDB50.md`). `ebp` carries the search start index; it is 0 at first (line 73) and is changed only when a fill is placed (line 128) and restored on every skip path (lines 180, 184, 189).
- Lines 104 to 106: result -1 (no chord): skip this line (lines 189 to 192: `ebx += 1`).
- Lines 107 to 115: fetch the chord at that index.
- Lines 116 to 126: `abs(chord.tick - L[ebx]) > slop` jumps to the skip path (lines 184 to 188, `ebx += 1`). `[chord+0x40]` is the chord's tick (line 117).
- Lines 127 to 135: in range: remember the chord index in `[rsp+0x88]` (line 128), call `0x20D51D0(chord, -1, 0)` to pick the note (line 131), then `or dword [note+0x7c], 0x800` (line 135): the activation flag.
- Lines 136 to 153: make a wrapper. Line 145 to 148: `rbp = L[ebx]` (the arithmetic `r12 - [rsp+0x38]` is `L[ebx-1] - L[ebx-1] = 0`, so the fill start is the line tick itself). Line 150 converts it to seconds with `0x212B9C0` into wrapper `+0x10`.
- Lines 154 to 161: wrapper `+0x18` from `0x20CFCE0`. Line 162 and 165: wrapper `+0x20` = the note's chord.
- Line 168: wrapper `+0x28` = `rbp` = the fill's start tick (the measure-line tick). Lines 169 to 173: wrapper `+0x30` = chord tick minus line tick (zero or negative when the chord is on or before the line), wrapper `+0x38` = 1.
- Lines 174 to 180: add the wrapper to the result. Lines 181 to 183: `ebx += 4` (line 182): four lines on after a placed fill.
- Lines 184 to 192: the two skip paths, `sub ebx, 3` then `add ebx, 4` = `ebx += 1`.

Who calls it: only `0x20D2EA0` (the chart builder) at 0x20D33A4, taken when the track's written-fill list is empty (`f20D2EA0.asm` lines 281 to 297, `cmp [list+0x18], r13d` with `r13d` = 0).
