# gm_awake_B41A0.asm  (build 6142, RVA 0xB41A0, GameManager.Awake)
Made with chdis.py, 2026-10-30, session b02ab3e6, scout R7. Method start per script.json.
Lines 2-146: unrelated start-up (scene check "Main Menu", background, shader id "_OutputHue"). Not read line by line.
Lines 147-154: settings static +0x20 (the video setting) through 0x210C1F0 (value / 1000) is stored at GameManager+0x68.
Lines 155-165: settings static +0x18 (the audio setting) through the same getter: xmm0 = audio seconds; GameManager+0x6C = audio (line 164) and GameManager+0x68 = video - audio (line 165).
