# calibrator_update_2FB0B0: Calibrator.Update (build 6142)

Listing: `calibrator_update_2FB0B0.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. It runs from RVA `0x2FB0B0` to the last byte before the next method (0x2FB6E0).

Plain version: `Update` runs the calibration screen's steps. It reads `Time.realtimeSinceStartup` (lines 87, 200 and 285), shows the result text "Video Cal: {0} ms ... press green to return to the main menu, or red to try again!" with the value multiplied by 1000.0 (lines 106, 126 and 151 to 154), shows a large counter "<size=75>{0}</size>" (lines 217 and 259 to 262) worked from a value divided by 60.0 (line 236), loads the "Main Menu" scene on exit (lines 74 and 76), and reads any button through Rewired's `GetAnyButtonDown` (line 338).

What this does not prove: how the offset is worked out. The arithmetic sits in `Calibrator` helpers this method calls, `0x2FAA40` (lines 93 and 291), `0x2F9BC0` (line 298), `0x2FA910` (line 347), `0x2FAFD0` and `0x2F94F0`; none of them was listed. Nothing here sends a value to a leaderboard.
