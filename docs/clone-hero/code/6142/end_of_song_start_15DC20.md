# end_of_song_start_15DC20.asm (build 6142, RVA `0x15DC20`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 15DC20 3000` (1596 lines). Starts at the method's first instruction: `EndOfSong.Start()` (`dump.cs` line 38794; class header at line 38671). Unity calls `Start` once, when the end-of-song screen's component first runs.

Only lines 721 to 743 were read, for the write to `GlobalVariables.scoreDisabled` (+0x7C, `dump.cs` line 55827).

- Lines 723 to 729: call `GlobalVariables` method `0x235A40` ("does any player play as a bot", `globals_any_bot_235A40.md`) on `GlobalVariables.instance`.
- Lines 730 and 731: false skips to line 743.
- Lines 732 to 742: true sets `scoreDisabled` to 2, which the enum #727 names Bot (`dump.cs` line 39732).

Who calls it: Unity, as a MonoBehaviour message. No caller search was run.
