# clock_tick_16C650.asm  (build 6142, RVA 0x16C650, #707 per-frame update, called by GameManager.Update at 0xC0839)
Made with chdis.py, 2026-10-30, session b02ab3e6, scout R7. Method start per script.json.
Lines 2-18: prologue, once-only setup.
Lines 19-26: step = deltaTime x (+0x30 / +0x2C), where +0x30 is the practice speed copy and +0x2C the song speed ratio; +0x40 += step (line 26).
Lines 27-32: +0x38 (the raw clock) += the same step. Between audio reads the raw clock runs on the frame timer.
Lines 33-72: if the audio manager exists (a null throws at the stub near the end), its flag at +0x28 says a stream is loaded, and its flag at +0x78 is not set, read the audio position.
Lines 73-75: audio clock = BassAudioManager position (0x365F10, argument 0) / +0x2C + +0x58; stored at +0x20 (line 75).
Lines 76-80: go on only when that audio clock is later than +0x58 and earlier than +0x10 - 1.0 (the clock length minus one second): the song proper, not the lead-in or the tail.
Lines 81-90: the audio manager's flag at +0x38 must be set (playing).
Lines 91-108: difference = audio clock - raw clock. If the +0x28 flag is set and |difference| is above 0.003 s (the float at 0x30648CC, line 105), store the difference as the correction at +0x18 and clear the flag (107).
Lines 109-141: if |correction| is above 0.003, add deltaTime x correction to the raw clock this frame (+0x38, line 127) and to the applied total (+0x1C, line 125); once the applied total reaches the correction, reset it, set the flag again and subtract (135-141). The raw clock is pulled toward the audio clock gradually, not snapped.
Lines 142-152: epilogue and the null-throw stub.
