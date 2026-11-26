# ini_save_2112DB0.asm (build 6142, RVA `0x2112DB0`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 2112DB0 550` (309 lines), from the method's first instruction to the last byte before the next method, `0x2113300`. It is the static save method of the ini class #8513 (`dump.cs` line 541097), taking one string, the ini name. The class keeps every setting in a dictionary (static +0x0) keyed by a two-string struct #8640 (`dump.cs` line 551077): +0x0 is the ini name and +0x8 is the section. The key constructor `0x2115820` takes the ini name first (its first string parameter has the same name as this method's), and its two-field store puts that first string at +0x0.

Plain version: the method walks every key in the settings dictionary. It skips a key whose ini name is the empty string. When it was given a name, it skips every key with another ini name; given null, it keeps them all. It skips a key whose ini name has no path registered in the class's second dictionary (static +0x8, filled by `0x2112BF0`). For each ini name left, it builds the file's text and writes it to the path that `0x2115760` looks up for that name. So a setting kept under the empty ini name is never written by this method.

What each line range does:

- Lines 1 to 37: stack set-up and the one-time loads of the generic slots and the empty string.
- Lines 38 to 52: `r13` is the name argument; `r15` starts as the empty string (the ini name of the file being built). A null settings dictionary (static +0x0) throws (line 294).
- Lines 53 to 69: start walking the dictionary's key-value pairs; when MoveNext returns false, go to the end at line 248.
- Lines 70 to 99: read the current pair. `rsi` is the key's +0x0 (the ini name), `rdi` its +0x8 (the section), `rbx` the value (the bag of entries).
- Lines 100 to 105: the empty-name test. `String.op_Equality(rsi, "")`; when it is true, jump back to the next pair (0x2112EF2). This is the test that drops every key with the empty ini name.
- Lines 106 to 133: walk the entries in the pair's bag.
- Lines 134 to 141: when the name argument is not null and the key's ini name differs from it, skip to the next entry.
- Lines 142 to 153: a type test on the entry; a failed test skips it.
- Lines 154 to 168: `ContainsKey(rsi)` on the static +0x8 dictionary (ini name to path); a name with no path is skipped.
- Lines 169 to 217: when the ini name changes from the one being built (`r15`), write out the one built so far (lines 182 to 189: `0x2115760` gives the path for `r15`, and a non-null path goes to the document writer `0x2165C50`), then start a new document for the new name with `0x2114600` (line 215).
- Lines 218 to 225: add the entry to the document (an interface call through the runtime helper at VA 0x182C44A20) and go to the next entry.
- Lines 226 to 284: the clean-up after each walk, then the last document's write (lines 268 to 282, the same lookup and write as lines 182 to 189).
- Lines 285 to 309: return, and the null-reference throws.

The path lookup `0x2115760` is in `ini_path_lookup_2115760.asm`: TryGetValue on the static +0x8 dictionary, returning null when the name is missing or its path is "%internal%".

Who calls it, direct callers only (byte scan with `docs/audit/2026-11-08-clone-hero-unknowns/W11b/callscan.py`, which reads rel32 `call` and `jmp` in the `il2cpp` and `.text` sections): 30 sites, 0x15DBD1, 0x15FBD1, 0x15FD51, 0x1661F9, 0x166636, 0x1C99CB, 0x1CA0DA, 0x1CA341, 0x1CA7BE, 0x1CAFD5, 0x1E16B2, 0x1E26EF, 0x1E588E, 0x1E7C5E, 0x2236F2, 0x2346E1, 0x26D89D, 0x37B816, 0x37BE46, 0x37C146, 0x37C3D6, 0x37C6B1, 0x37C991, 0x37CF49, 0x37D656, 0x2114C98, 0x213377B, 0x21337CB, 0x2138A9B and 0x213ACAB. Whatever name each passes, the empty-name test at lines 100 to 105 runs first.

Who registers an ini name with a path (`0x2112BF0`), direct callers only (byte scan): 9 sites. The string loaded just before each, read with `chdis` from 0x60 bytes before the call: 0xA16B4, 0x2AC157, 0x2AC35A and 0x2AC8BB build "GameColors_" plus a value; 0xA2117 builds "Rhythm Guitar" plus a value; 0x2ACDCB builds "setlistErrorDialog" plus a value; 0x1CB46E passes "profiles" with the `profiles.ini` path; 0x37539E passes "settings" with the `settings.ini` path; 0x235452 passes a joined name with a `config.ini` path. The name at 0x235452 is read in `ini_register_caller_235340.md`: it is 'highway_' plus a file name, so none of the nine names is empty.
