# clock_raw_minus_audio_16C480.asm  (build 6142, RVA 0x16C480, #707, songTime getter)
Made with chdis.py, 2026-10-30, session b02ab3e6. Lines 2-4: return the raw clock (+0x38) minus the audio calibration (+0x48). The bytes are f2 0f 10 41 38 f2 0f 5c 41 48 c3, the same bytes the 2026-10-28 overlay handoff quotes.
