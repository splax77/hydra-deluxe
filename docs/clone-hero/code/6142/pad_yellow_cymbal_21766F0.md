# pad_yellow_cymbal_21766F0: the drums inputs record's yellow cymbal setter (build 6142)

Listing: `pad_yellow_cymbal_21766F0.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. A method of the drums inputs record, called by `drums_input_source.asm` at its line 135 with the result of Rewired action 0x26 (line 130), one of the three actions 0x25 to 0x27 read after the four tom and kick actions.

It takes one bool. When the bool is false it clears bit 6 of the record's first int (line 18, `btr eax, 6`); when true it sets bit 6 (line 24, `bts eax, 6`). The yellow tom setter (`pad_yellow_tom_2176900.md`) uses bit 2.

That action 0x26 is the yellow cymbal is inferred from the order of the reads and the setter's bit, not from a Rewired action name; the action names were not read.
