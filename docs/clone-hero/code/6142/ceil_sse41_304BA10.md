Listing: `ceil_sse41_304BA10.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: the fast route of the rounding helper 0x3031130 (`ceil_helper_3031130.md`), RVA 0x304BA10, the whole routine.

Line 1: `roundsd xmm0, xmm0, 0xa`. The immediate 0xA sets the rounding mode bits to 10 (toward plus infinity) and bit 3 (do not raise the inexact exception), so this is a round up.
Line 2: return.
