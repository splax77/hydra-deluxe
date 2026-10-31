# Add-fill twin, `#8661.ʲʲʲʿʴʴʴʷʲʽʳ`, RVA `0x215B7C0` (build 6142)

Listing: `add_fill_twin_215B7C0.asm`, 71 lines, starts at the first instruction. `dump.cs` line 552108 area names it `public void ʲʲʲʿʴʴʴʷʲʽʳ(long, long, bool)` on class `#8661`; the decompile is `ghidra_decompile_all.txt` lines 47197-47244. Made with `tools/ch_mcp/chdis.py`, 2026-10-30.

Same line-by-line explanation as `add_fill_215BD50.md`: take a fill object from the pool at `[track+0x30]`, initialise it with `0x20FB9A0(object, tick, length, flag, 0)`, append it to the list at `[track+0x88]`, growing the list when full. The only differences from `0x215BD50` are the static flag (`[0x3f19754]` here, `[0x3f19752]` there) and three metadata slot addresses.

Unlike `0x215BD50`, this twin has one direct caller: 0x20E4B79, inside `0x20E49D0` (`fill_cache_caller_20E49D0.asm`).
