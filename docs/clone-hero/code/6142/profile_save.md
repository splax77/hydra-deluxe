# profile_save: where the profiles file lives and how the game loads it (build 6142)

Listing: `profile_save.asm` beside this file, made 2026-10-31 (session b15bb484) with `tools/ch_mcp/chdis.py` (its `Image` disassembler, printing a range) from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. The listing is the whole method at RVA `0x1C9CA0`, from its first instruction to the last byte before the next method (0x1CA120). It is a method of the profile list class #792 (`dump.cs` line 43116, method at line 43182). Despite the file name, this method loads the profiles; the writer of `profiles.ini` was looked for and not read through (see the end).

Plain version: the game keeps `profiles.ini` in the same folder as `settings.ini` and `scorestats.json`, which is `Documents\Clone Hero` on Windows. An older copy in Unity's own data folder (LocalLow) is only read: when the Documents copy is missing, the load copies the old file across. When the profiles are read back, a profile whose number is -1 is built and then dropped, never added to the list.

Where the path comes from, read outside this listing. The class's constructor `0x1CB350` (`dump.cs` line 43238) stores two paths: the folder from `0x307B40` joined with "profiles.bin" in its +0x10 (0x1CB3D6 to 0x1CB409), and the same folder joined with "profiles.ini" in its +0x18 (0x1CB417 to 0x1CB432). It then registers the ini name "profiles" with the +0x18 path through `0x2112BF0` (at 0x1CB46E), which stores the pair in a dictionary of the ini class #8513 (0x2112C79). The folder method `0x307B40` (`dump.cs` line 67903) returns .NET's Environment GetFolderPath with folder 5, joined with "Clone Hero" (0x307BC1 to 0x307BFB). When a static bool of `GlobalVariables` (its static fields' +0x10, tested at 0x307BBB) is set, it returns the parent of Unity's Application dataPath joined with "PlayerData" instead (0x307C17 to 0x307C6D). What sets that bool was not traced. Folder 5 is .NET's SpecialFolder Personal, the user's Documents folder. `EndOfSong.Start` takes the folder for `scorestats.json` from the same method (`scorestats_writer_15DC20.md`, lines 246 to 260).

Lines of the listing:

Lines 1 to 41: save registers and the one-time IL2CPP set-up; the strings "profiles.ini", "profiles" and "profiles.bin" are named here.

Line 42: a new empty list goes into the class's +0x20.

Lines 51 to 67: build two old paths, Unity's Application persistentDataPath joined with "profiles.ini" (line 60) and with "profiles.bin" (line 67).

Lines 71 to 81: when the new `profiles.ini` (+0x18) exists and the new `profiles.bin` (+0x10) also exists, delete the `.bin` (line 81).

Lines 85 to 90: when the new `profiles.ini` does not exist and the old one does, copy the old one to the new path (line 90). Lines 94 to 109: when neither new file exists and the old `profiles.bin` does, copy it to the new `.bin` path (line 109). The old files are never written or deleted.

Lines 112 to 121: when the new `profiles.ini` exists, ask the ini class's `0x21150E0` (the call at line 121) for the keys of the ini "profiles"; with no file, jump to line 244.

Lines 122 to 172: walk the keys. Line 173 takes each key from its eighth character (Substring with 7) and line 174 parses it as an int.

Lines 190 and 191: the ini name is "profiles", or "" when the number is -1. Line 199 builds the profile with the number and that ini name (`0xD8A90`). Line 200: when the number is -1, go back for the next key without adding it. Line 207 adds every other profile to the list, and the count at +0x28 goes up.

Lines 238 to 240: after the walk, call the ini class's `0x2114A80` with "profiles"; it was not read.

Lines 244 to 251: with no `profiles.ini`, call the class's own `0x1CB070` and then the ini class's `0x2112DB0` with no name. Neither was read; on 2026-10-31 neither folder held a `profiles.ini` or a `profiles.bin`, so whatever they do left no file on this machine.

The profile clone. The loader's clone `0xD8470` (called at line 253 of `cli_loader_3968D0.asm`) builds a new profile with number -1 and ini name "" (0xD84C9 to 0xD84DE: `mov r8` with the empty string, `mov edx, 0xffffffff`, then the call to `0xD8A90`) and copies the settings across with the setters. The profile constructor `0xD8A90` (`dump.cs` line 25699) keeps the ini name and the section "profile{0}" with the number (0xD8BE5 to 0xD8C1C, through the ini class's `0x2115820`) in the profile's +0xC0. So a profile the load reads belongs to the ini "profiles", and the clone belongs to an ini with an empty name, which no code found registers with a path.

The writer, looked for and not read through. The ini class #8513 (`dump.cs` line 541027) has two methods that call System.IO File.WriteAllText: `0x2113710` (one direct caller, in `GameInit` at 0x164A9B) and `0x2113C60`, which writes the one path held in the class's static +0x28. The save method `0x2112DB0` (`dump.cs` line 541097, taking a name) walks a dictionary and compares names (0x2112ED3 to 0x2112FA4); how far it goes for an empty name was not read. So whether the clone's settings are ever written to `profiles.ini` is not proven here.
