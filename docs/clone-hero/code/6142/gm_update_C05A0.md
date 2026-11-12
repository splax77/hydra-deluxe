# gm_update_C05A0.asm  (build 6142, RVA 0xC05A0, GameManager.Update)
Made with tools/ch_mcp/chdis.py on 2026-11-08, session 77057df2 (task CH-UNK-W2). Method start per dump.cs.
Lines 17-18: when the byte at GameManager+0x158 is 0, the whole body is skipped (jump to the epilogue).
Lines 68-79: when isPaused (+0x90) is set, a CHNetManager check runs; if it returns false, the clock block below is skipped (jump to 0xC0B89, line 337).
Lines 140-159: the clock block. Line 144 calls the clock method 0x16C650 (clock_tick_16C650.md) on the clock at GameManager+0xB0. Lines 146-147 copy songTime (+0x70) to lastSongTime (+0x80). Lines 151-153 call 0x16C480 (clock_raw_minus_audio_16C480.md) and store the result in songTime (+0x70). Lines 157-159 call 0x16C490 (clock_raw_plus_video_16C490.md) and store it in songTimeVideoTime (+0x78).
Lines 164-170: when songTime is past farthestTimeReached (+0x88), it is stored there and the byte at +0xC0 is set.
Lines 178-180: the song-end test. songTime (+0x70) is compared with songLength (+0x50); a smaller songTime jumps past the end block, so the end block runs when songTime >= songLength.
Lines 181-209: the end block. If the GlobalVariables byte +0x72 is set, line 209 calls the GameManager method 0xBF510 with dl = 1. Otherwise, unless the FadeBehaviour check at line 199 returns true, line 204 calls the GameManager method 0xBD640 (gm_song_over_BD640.md), which sets isSongOver.
Field names from dump.cs lines 25298 to 25311. Lines 210 to 351 (input and practice handling) were not read line by line.
Line 157 calls the video-time getter `0x16C490`; line 159 stores the result at +0x78, which is `GameManager.songTimeVideoTime` (`dump.cs` line 25305). (Read in task CH-UNK-W4.)
