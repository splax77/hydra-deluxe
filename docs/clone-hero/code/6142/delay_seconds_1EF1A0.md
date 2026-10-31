# delay_seconds_1EF1A0.asm  (build 6142, RVA 0x1EF1A0, #828.DelaySeconds, the delay in seconds)
Made with chdis.py, 2026-10-30, session b02ab3e6, scout R7. Method start per script.json.
Lines 2-4: load the int at this+0x24 (the delay in ms that 0x1EFC60 or 0x35C980 stored), convert to double, divide by 1000.0 (the constant at 0x3064C28, which constants.py names RVA_FORMULA_DIVISOR). Line 5: return.
Lines 6-21: padding.
Note: the neighbour 0x1F0830 (#828.DelayOver1582) divides the same int by the double 1582.0 and nothing in the game's code section calls it directly.
