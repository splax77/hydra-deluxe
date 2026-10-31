Listing: `scorestats_writer_15DC20.asm`, build 6142, made 2026-10-30 with `tools/ch_mcp/chdis.py`. Function: `EndOfSong.Start` (#711), RVA `0x15DC20`, the whole method (1,596 lines). The part that matters for `scorestats.json` is lines 171 to 300.

Lines 1 to 120: one-time loads of every string literal the method uses; line 87 loads `scorestats.json` (slot 0x3CA8B78). The same block loads `botnosave`, `spectatornosave`, `savewarning`, `ScoreDisabledModifier` and `ScoreDisabledPause` (lines 79 to 102); how those are used was not traced.
Lines 121 to 170: set up the screen (scene-change hook, title text, fade).
Lines 171 to 181: allocate the stats object (#1037) and run its constructor with a value read from `GlobalVariables` at `+0x54` (the difficulty or mode, not traced); keep it in `this+0xE8`.
Lines 182 to 207: call `0x252200`, which fills the song fields (song entry, and two ints from the score screen's state).
Lines 208 to 245: loop over the player list in `GlobalVariables`; for each player that passes the test at `0x337540`, get its data (`0x337D00`) and call `0x251A70` to add a per-player block to the stats object.
Lines 246 to 260: get a folder from `0x307B40` and join it with the literal `scorestats.json` (`System.IO.Path.Combine`, line 260).
Lines 261 to 270: call the stats object's text writer `0x252630` and write the text to that path with `System.IO.File.WriteAllText`.
Line 272: on success, jump past the handler to line 302.
Lines 273 to 300: the error handler: build the text `Failed to write scorestats.json: ` plus the error's text and call Debug.LogError.
Lines 302 to 350: update the Discord presence (a second error handler logs `Failed to update Discord stats presence: `).
Lines 350 to 1596: the rest of the screen (song title, artist, charter, score text with `{0:n0}`, stars background, save warning); not part of the file writing and not annotated here.
