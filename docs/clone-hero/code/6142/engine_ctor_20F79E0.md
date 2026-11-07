# Base engine constructor, RVA 0x20F79E0 (build 6142)

Class `#8288`, `.ctor(this, struct #8263-sized argument block, settings object)` (script.json signature). Listing: `engine_ctor_20F79E0.asm`, 316 lines, ends at the padding after 0x20F8006. Decompiled lines: `ghidra_decompile_all.txt` 54155 onward (the function is in the file; the listing here is the instruction evidence). The drums engine's own constructor (0x20DF680, class 8255) calls the generic base 0x100EAE0 which calls this one (byte scan: callers at 0x100E9E3, 0x100EB63 and 0x100ECE3).

In plain words: the lines annotated here are the ones that set the Star Power sizes. The engine takes the chart record (stored at +0x230) and sets four numbers from it: a quarter bar of meter, a half bar, the full bar, and a few helpers. The quarter bar is the chart's double at +0xB8 times 7.5, rounded to a whole number. The full bar is four quarters. That 7.5 is a constant stored in the DLL at the address the listing names (the first of two doubles there, value 7.5, read 2026-10-30).

Not annotated line by line: lines 1 to 127 and 197 to 316. They copy the argument block to +0x1E0 through +0x27C, allocate two lists (+0x180 and +0x188), set defaults (+0x54 and +0xAC to -1, +0x30 to 0.07, and some floats), create the 32-slot note array at +0x58, and run the rest of start-up. Those lines were not read for any rule on the Star Power pages.

Lines 130 to 150: read a number from an earlier line (xmm6, truncated to an int); compare it with 25 (Int32.CompareTo at line 141); keep the larger of the two (line 144); divide by 25 (the `imul` by 0x51EB851F at lines 143 to 149 is a divide by 25 for a signed int). Store at +0x1A8. Meaning unsettled.
Lines 151 to 156: the chart's double at +0xB0 truncated to a whole number, divided by 4 rounding toward zero (the `and edx,3 / add / sar 2` at lines 153 to 155); stored at +0x1C8.
Lines 157 to 162: load the chart's double at +0xB8 and multiply it by 7.5.
Lines 163 to 166: round it with 0x182C49A60 (its first 25 instructions compare a value with 0.5 and add 0.5 otherwise, which reads like round-to-nearest; its tie rule was not read in full), truncate to a whole number, store at +0x1C0. This is a quarter bar.
Lines 167 to 168: +0x1B8 = quarter times 2, a half bar.
Lines 169 to 170: +0x1B0 = quarter times 4, the full bar the drain and clamp code reads.
Lines 171 to 182: build one helper object from the settings (+0x238) and store it at +0x1D8.
Lines 183 to 189: dl = 1 when the engine kind byte (+0x218) is 6 or 9, else 0.
Lines 190 to 192: chart record null jumps to line 261+ (null throw).
Lines 193 to 196: call the star base function 0x20F4680 with the flag and the chart's +0xB8; the result is stored at +0x1D0 (line 202).
Lines 197 to 207: +0x18 = minus +0x30; +0x20 = +0x30 doubled; +0x10 = +0x30; the multiplier +0x48 starts at 1 (line 206).

What is proven: the full bar is 4 times round(7.5 times the chart's +0xB8). What is not: what +0xB8 holds. Two live runs on 2026-10-28 read a full bar of 5,760 on a resolution-192 chart (`run1_cal0_scorestats.json` `sp_bar_ticks`), which is 4 times 1,440 and 1,440 is 7.5 times 192. So +0xB8 held 192 there. The chart's other double at +0xB0 is what the tick conversions use as their resolution; its value was not read.
