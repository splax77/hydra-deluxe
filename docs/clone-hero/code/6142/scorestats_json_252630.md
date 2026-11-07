Listing: `scorestats_json_252630.asm`, build 6142, made 2026-10-30 with `tools/ch_mcp/chdis.py`. Function: `#1037` method `ToJsonText`, RVA `0x252630`: turns the stats object into the JSON text that `EndOfSong.Start` writes.

Lines 1 to 28: prologue and one-time string-literal loads.
Lines 29 to 60: the twelve top-level key names are loaded as string literals: `band_score`, `playback_speed`, `band_stars`, `song_name`, `score_timestamp`, `charter_name`, `player_count`, and then `game_version`, `checksum`, `artist_name`, `game_mode`, `players` (the full set of `str:` lines is in the file). The literal `O` is a date format letter.
Lines 60 to 353: build a JSON object, one key per value from the stats object's fields, with the player blocks nested under `players`; the per-player keys are written by another class, not by this function.
