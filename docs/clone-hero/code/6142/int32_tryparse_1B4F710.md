# int32_tryparse_1B4F710.asm  (build 6142, RVA 0x1B4F710, System.Int32.TryParse(string, out int))
Made with tools/ch_mcp/chdis.py (py -m tools.ch_mcp.chdis 1B4F710 400) on 2026-11-12, session c61c1986 (task T1-MID-NO-PRO). Method start per dump.cs line 89424.
Plain words: it is .NET's own TryParse. It passes the number style 7 and the current culture's number format to .NET's integer parser.
Lines 11-12 and 68-73: a null string sets the result to 0 and returns false.
Line 28: NumberFormatInfo.get_CurrentInfo, the current culture's number format (its sign symbols).
Lines 56-62: Number.TryParseInt32IntegerStyle(span, 7, format, out result), with the style in edx at line 60 and the call at line 62. Style 7 is NumberStyles.Integer: AllowLeadingWhite (1), AllowTrailingWhite (2) and AllowLeadingSign (4). It has no AllowDecimalPoint and no AllowThousands.
What style 7 accepts is .NET's rule, inside TryParseInt32IntegerStyle, which was not listed: blanks before and after the digits, one leading sign, and digits only. A decimal point, a thousands separator or a value outside the 32-bit range makes it fail.
