# guitar_length_check_115BB30.asm (build 6142, RVA `0x115BB30`)

Made 2026-11-23 with `tools/ch_mcp/chdis.py` (73 lines). Starts at the method's first instruction. script.json names it as a method of `BaseGuitarPlayer<...>` (class #690, a guitar player; drums use `DrumsPlayer`). The same body is compiled three times, at `0x115BB30`, `0x115BC80` and `0x115BDD0`, one per generic instance.

Plain words: when a guitar note's long field is longer than the engine's quarter of the working resolution, play a sound through the audio manager.

- Lines 22 to 25: load the player's engine (`+0x120`) and its long at `+0x1C8`, which the base engine constructor sets to the chart's `+0xB0` truncated and divided by 4 (`engine_ctor_20F79E0.md`, lines 151 to 156).
- Lines 26 and 27: go on only when the note's `+0x48` is greater than that value.
- Lines 28 to 41: two virtual checks on the player.
- Lines 42 to 59: call a `BassAudioManager` method with -1.0, then set the player's byte at `+0xD0`.

Where readers of `+0x1C8` were looked for: every method of the engine classes #8288, #8253 and #8255 (only the constructor writes it), and a byte search of the whole code section for the 0x1C8 offset, kept to methods whose class name holds Player, Drum, Note, Track, Sustain, Star, Highway, GameManager, Lane or Fill. Only these three guitar methods read it from an engine. What the note's `+0x48` holds, and what the sound is, were not read.
