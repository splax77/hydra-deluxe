# ini_int_getter_21647A0.asm  (build 6142, RVA 0x21647A0, ini object method that returns an int for a key)
Made with chdis.py on 2026-10-30, session b02ab3e6, scout R7. Starts at a method start per script.json.
Lines 2-19: prologue; first-call string setup for the text ".".
Lines 20-31: ask the ini for the key's text through 0x1801BD150 (section in rdx, key in r8, text out). A missing key jumps to line 66 and returns the default (ebx = r9d, the caller's default).
Lines 32-36: if the text is null, throw; else test whether it contains the character ".".
Lines 37-45: no "." -> Int32.TryParse(text). A failure jumps to line 66 (default).
Lines 46-65: has "." -> Single.TryParse with NumberStyles 0xA7 (leading and trailing white space, leading sign, decimal point, exponent) and the invariant culture, multiply the float by 1000.0f (line 63), truncate toward zero (cvttss2si, line 64). So "delay = 0.5" gives 500 and "delay = 250.0" gives 250000.
Line 66: return the default. Lines 67-73: epilogue.
