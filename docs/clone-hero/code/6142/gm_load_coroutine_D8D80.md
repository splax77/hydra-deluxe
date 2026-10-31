# gm_load_coroutine_D8D80.asm  (build 6142, RVA 0xD8D80, GameManager.ʻʵˀʸʷˀʿʸʳʲʲ.MoveNext, the song-load coroutine)
Made with chdis.py over the whole method (the next method starts at 0xD99E0), 2026-10-30, session b02ab3e6, scout R7. rsi = the coroutine state object, rdi = the GameManager.
Lines 20-85: first-call string setup and state dispatch. Lines 101-260: earlier states (network checks, log lines, audio length 0x36B700). Not read line by line.
Lines 272-282: call #828.ʻʸʳʶˁʷʵʴʻʾʹ (0x1F0160) with the GlobalVariables song metadata object and the SongEntry. That method reads the song.ini file and runs the [song] reader 0x1EFC60, so the delay lands at +0x24 first.
Lines 284-291: then call SongEntry.ʿʲʷʹʿʳʾʳʷˀʾ (0x35C110), which loads the chart and calls the [Song] scanner 0x35C980 (call at 0x35C484). So the chart's Offset is applied second, and only when +0x24 is still 0.
Lines 300-309: if the flag at [rsi+0x30] is 0, the length comes straight from the chart object and the code jumps to line 411.
Lines 310-329: otherwise get the speed ratio (call 0x210C090, line 316), the chart's last note end (0x212E130) turned into seconds through the tempo map (0x212B9C0).
Lines 330-352: audio length from the audio manager (+0x40), then Math.Max of it and (speed x last note end).
Lines 357-381: if the end event counts (0x212C7E0), take its time and Math.Min.
Lines 382-389: length + 1.0 (constant 0x3063DC0) is stored in the song object at +0x30.
Lines 411-434: a length of exactly 0 logs "Song length is 0!" and aborts.
Lines 435-443: call 0x1EF1A0, the delay in seconds, on the metadata object: xmm10 = delay.
Lines 444-451: speed ratio again (0x210C090): xmm8.
Lines 452-453: xmm7 = GameManager+0x6C (audio calibration), xmm6 = GameManager+0x68 (video minus audio).
Lines 454-464: new clock object (#707) and a call to its constructor 0x16C9A0 with (length + 1 in xmm1, delay seconds in xmm2, speed ratio in xmm3, audio calibration and video-minus-audio on the stack). Line 465 stores the clock at GameManager+0xB0.
Lines 472-473: the clock's float at +0x30 gets GameManager+0xD8 (practiceSpeed).
Lines 474-486: the clock length (+0x10) is copied to GameManager+0x50 (the song-end value) and the first clock read (0x16C480) goes to GameManager+0x70 and +0x88.
Lines 487-700: the rest of the load (lyrics, players, network). Not read.
