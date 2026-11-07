Listing: `scorestats_json_252630.asm`, build 6142, made 2026-10-30 with `tools/ch_mcp/chdis.py`. Function: `#1037` method `ToJsonText`, RVA `0x252630`: turns the stats object into the JSON text that `EndOfSong.Start` writes.

Lines 1 to 28: prologue and one-time loads of type and method information.
Lines 29 to 54: the twelve top-level key names are loaded as string literals: `band_score`, `playback_speed`, `band_stars`, `song_name`, `score_timestamp`, `charter_name`, `player_count`, `artist_name`, `checksum`, `game_mode`, `players` and `game_version`. The literal `O` (line 41) is a date format letter.
Lines 55 to 97: set up the writer: indented JSON options (line 68), a memory stream (line 75) and a `Utf8JsonWriter` (line 89).
Lines 98 to 258: write a JSON object, one key per value from the stats object's fields, with the player blocks nested under `players`; the per-player keys are written by another class, not by this function.
Lines 259 to 323: close the writer, get the UTF-8 encoding (line 279), make two calls through method tables on the stream and the encoding (lines 286 and 293), and return that result.
Lines 324 to 353: the throw stubs.
