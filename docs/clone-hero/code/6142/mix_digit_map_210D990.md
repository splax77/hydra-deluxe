# mix_digit_map_210D990 (build 6142, RVA `0x210D990`)

Made 2026-10-30 with tools/ch_mcp/chdis.py. It maps the number in a mix event to a difficulty byte. Lines 1-2: 0 returns 0 (Easy, line 17). Lines 3-4: 1 returns 1 (Medium). Lines 5-6: 2 returns 2 (Hard). Lines 7-8: 3 returns 3 (Expert, line 12). Lines 9-10: anything else returns 0xFF (-1, "none"), which the mix parser treats as a failure. Enum values from dump.cs 550049-550059: Easy 0, Medium 1, Hard 2, Expert 3, None -1.
