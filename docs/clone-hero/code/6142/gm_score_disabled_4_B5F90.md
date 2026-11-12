# gm_score_disabled_4_B5F90.asm (build 6142, RVA `0xB5F90`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis B5F90` (306 lines). Starts at the method's first instruction: `GameManager`'s public void ʵʿʲʾʺʵʽʾʼˀʽ(ˁʻʽʷʽʻʾʵʷˀʶ) (`dump.cs` line 25559; class header at line 25284). It loads `GetComponent<PauseMenu>` (line 12) and hands its player argument to the PauseMenu (lines 156 to 167), so it reads as the method that opens the pause menu; that name is a reading of the code, not the dump's.

Only lines 126 to 196 were read closely, for the write to `GlobalVariables.scoreDisabled` (+0x7C, `dump.cs` line 55827).

- Lines 126 to 136: when `scoreDisabled` is already not 0, skip to line 153.
- Lines 137 to 146: when `GlobalVariables.isPracticeEnabled` (+0x72, `dump.cs` line 55819) is set, skip to line 153.
- Lines 147 to 151: subtract GameManager's double at +0x118 from its double at +0x70 and compare with the constant 1185.0 (line 149). A difference under 1185.0 goes to line 182.
- Line 152: otherwise set `totalPauses` (+0x120, `dump.cs` line 25337) to 1.
- Lines 182 to 185: read `totalPauses` and store it back unchanged (no increment shows in these two lines); when it is 3 or less, go to line 153.
- Lines 186 to 196: otherwise set `scoreDisabled` to 4. The enum #727 names only None 0, Modifier 1, Bot 2 and Pause 3 (`dump.cs` lines 39726 to 39733), so 4 has no name in the dump.
- Line 174: the double at +0x118 is set from `0x16C550` on the object at +0xB0 after the pause menu opens.

Who calls it: not searched.
