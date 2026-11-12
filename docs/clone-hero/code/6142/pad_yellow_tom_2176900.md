# pad_yellow_tom_2176900: the drums inputs record's yellow tom setter (build 6142)

Listing: `pad_yellow_tom_2176900.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. A method of the drums inputs record, called by `drums_input_source.asm` at its line 83 with the result of Rewired action 2 (line 78), the yellow tom action going by its place among the tom reads.

It takes one bool. When the bool is false it clears bit 2 of the record's first int (line 18, `btr eax, 2`); when true it sets bit 2 (line 24, `bts eax, 2`). The yellow cymbal setter (`pad_yellow_cymbal_21766F0.md`) uses bit 6, so the two keys of one colour set two different bits.
