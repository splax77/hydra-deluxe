Listing: `midi_pump_21E420.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py` (`py -m tools.ch_mcp.chdis 21E420 830`). Function: MIDIInputManager's method at RVA 0x21E420 (`dump.cs` lines 52818 and 52819), which takes no argument. The register method subscribes it to Rewired's InputSourceUpdateEvent (`midi_register_21CDE0.asm` lines 238 to 253), so it runs on Rewired's input updates.

Plain version: on each input update the game turns every MIDI note a registered kit sent into an axis value on that kit's custom controller. A pad or the kick gets 0.5 plus half its velocity over 127, so a velocity of 127 gives 1.0 and a velocity of 1 gives about 0.504. Start and Select get 1.0 whatever their velocity.

The setup. Line 82 calls the manager's method at 0x220660 first; it has no listing, and what it writes was not read. Lines 86 to 88 load the three constants 127.0, 0.5 and 1.0 into xmm11, xmm10 and xmm9.

The loop. Lines 89 to 103 walk the manager's device list (+0x38) and skip a record whose enabled flag (+0x48) is false. Line 107 calls 0x21E100 on the record, which was not read. Lines 108 to 128 walk the record's message list (+0x38). For each message, lines 137 to 142 turn one byte of it into the value: `cvtdq2ps`, `divss xmm6, xmm11` (over 127), `mulss xmm6, xmm10` (times 0.5), `addss xmm6, xmm10` (plus 0.5). The byte is the message's second byte, at +0x9 of the struct #986 (`dump.cs` lines 52494 to 52500; lines 131 and 132 take it from the enumerator's copy). The method that fills a message was not read, so that this byte is the MIDI note's velocity is inferred: it is divided by 127, MIDI's top velocity, and the velocity helper reads it back with the inverse (`lane_velocity_337E10.asm` lines 79 to 103).

The writes. For each pad name the game asks the method at 0x21F840 whether the message is that pad's (it takes the record, the name and the message, `dump.cs` line 52780), and on true calls Rewired's CustomController.SetAxisValue (RVA 0x1D5DF10, `dump.cs` line 286520, which takes an element name and a float) on the record's controller at +0x30:

| Pad name the game tests | Lines | Element written | Value |
|---|---|---|---|
| Red Pad | 151 to 163 | Snare | the velocity value |
| Yellow Pad | 173 to 185 | Tom1 | the velocity value |
| Blue Pad | 195 to 207 | Tom2 | the velocity value |
| Green Pad | 217 to 229 | Tom3 | the velocity value |
| Yellow Cymbal | 239 to 251 | Cymbal1 | the velocity value |
| Blue Cymbal | 261 to 273 | Cymbal2 | the velocity value |
| Green Cymbal | 283 to 295 | Cymbal3 | the velocity value |
| Kick Pad | 305 to 317 | Kick | the velocity value |
| Start | 327 to 339 | StartPad | 1.0 (xmm9, line 337) |
| Select | 349 to 361 | SelectPad | 1.0 (xmm9, line 359) |

The pad names are the manager's string constants (`dump.cs` lines 52723 to 52732). The element names are definition 1's identifiers in `Clone Hero_Data\level1`. Lines 379 to 390 empty the record's message list once its messages are written.

What the listing does not show: how a message is matched to a pad (0x21F840), what 0x21E100 and 0x220660 do, and how often Rewired raises the event.
