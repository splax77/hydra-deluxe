# speed_ratio_210C090.asm  (build 6142, RVA 0x210C090, #(IntSetting = int setting).AsRatio, value / 100)
Made with chdis.py, 2026-10-30, session b02ab3e6. Method start per script.json.
Lines 2-9: prologue and a once-only metadata-method touch. Lines 10-12: load the int setting value at this+0x10, convert to float, divide by 100.0f (constant at 0x3063FFC). Line 13-15: return. The load coroutine calls it on the settings object at GlobalVariables-settings static +0x10, which the settings loader built as the "song_speed" setting, so the result is the song speed as a ratio (100 -> 1.0).
