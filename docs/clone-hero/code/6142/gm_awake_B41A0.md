# gm_awake_B41A0.asm  (build 6142, RVA 0xB41A0, GameManager.Awake)
Made with chdis.py, 2026-10-30, session b02ab3e6. Method start per script.json.
Lines 2-146: unrelated start-up (scene check "Main Menu", background, shader id "_OutputHue"). Not read line by line. Line 126's label is one of several names script.json gives the shared code at 0x7452D0; the real type is the List<int> on line 124.
Lines 147-154: settings static +0x20 (the video setting) through 0x210C1F0 (value / 1000) is stored at GameManager+0x68.
Lines 155-165: settings static +0x18 (the audio setting) through the same getter: xmm0 = audio seconds; GameManager+0x6C = audio (line 164) and GameManager+0x68 = video - audio (line 165).
Lines 85-92: settings static +0x10 (`song_speed`) through the ratio getter 0x210C090 (value / 100) is stored at GameManager+0xD8, `practiceSpeed`. So outside practice mode practiceSpeed starts equal to the speed ratio. Read 2026-11-23.
Lines 93-103: the same setting's whole value minus 100, as a float, is stored at GameManager+0xDC. Its readers were not searched.
