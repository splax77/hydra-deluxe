# Survey: game-side facts and the reverse-engineering tooling (2026-10-30)

Written by the planning agent for task `ch-reference-plan`, slice "game facts and RE tooling". Everything below was read from the files named; nothing was run against the game, and no analysis was started. Where a search found nothing, the search is named.

## 1. Where the game facts sit today

### tools/ch_probe/constants.py is the one executable home, and it holds 37 addresses

`constants.py` says of itself (lines 1 to 8) that it is "the ONE place the numbers live" and that every other module imports from it. The file holds three kinds of game fact, all for build 1.1.0.6142 (line 22):

- Seven function RVAs (lines 38 to 73): the window formula `0x20DDDA0`, the drums engine constructor `0x20DF680`, the base engine constructor `0x100E960`, note processing `0x20DCC90`, the hit check `0x100DAB0`, the score adder `0x20DEEC0` and the dynamics handler `0x20DE770`.
- Fourteen engine-object byte offsets plus one flag bit (lines 87 to 147): total/back/front window `0x20/0x30/0x38`, song clock `0x100`, score `0x94`, hit time `0x2E0`, the six Star Power fields `0x68/0x1B0/0x42/0x48/0x120/0x130`, flags `0x198` with `PRECISION_MODE_BIT 0x1000`, note count `0x8C`.
- Fifteen `.rdata` constant RVAs (lines 159 to 191): four per-side window constants, four normal-formula and four precision-formula constants, the divisor, the exponent, and the hit-check threshold `0x31406E8`.

It also holds measured values that are not addresses: the window cap and floor `171.4313` / `96.2932` ms (lines 242 to 243, sourced to `experiments/results/poll_windows.csv` and ch-evidence section 77), the expected constants `0.085`, `0.0375`, `0.040` s (lines 218 to 220), and the drums-engine's own constant `0.075` s appears only in the dump note, not here.

Who reads it: `engine.py`, `engine_finder.py`, `process.py`, the experiments, and in `tools/ch_mcp` `trace_hits.py` (lines 12, 257, 288, 417), `frida_tools.py` (line 233) and `dump_facts.py` (line 302, the default `--engine-ctor-rva`). The grep `0x[0-9A-Fa-f]{5,8}\b|RVA_|OFF_|constants\.` over `tools/ch_probe/*.py` shows no other module writing an address of its own; `engine_finder.py` line 27 has `MODULE_SPAN = 0x4000000`, a search span, not a game fact.

### Second copies of the same addresses

Three addresses are repeated as literals in tests: `0x20DEEC0` and `0x20DE770` in `tools/ch_mcp/tests/test_trace_hits.py` lines 263 to 264 (an assertion that the constants have those values, which is a deliberate pin), and `0x20DF680` in `tests/test_dump_facts.py` line 25 and `tests/test_build_facts.py` lines 37, 46, 134 (fake facts). Those are test fixtures, not homes.

The Frida agents hold no addresses at all. `tools/ch_mcp/agent/gamemanager.ts`, `state.ts`, `clock.ts` and `trace.ts` read `GameManager.songTime` and `isSongOver` by field name through the Il2Cpp bridge (`gm.field<number>("songTime")`, `state.ts` line 38, `clock.ts` line 19, `trace.ts` lines 288 to 289). The only hex in `trace.ts` is `TYPE_ATTRIBUTE_SEALED = 0x100` and byte masks (lines 44, 112 to 121). So the agents' "home" for field names is the live game's own metadata, read at run time, which is a fourth kind of home.

### The dump note: tools/ch_probe/experiments/results/overlay-spike/il2cpp-fields.md

This 33,687-byte file (size from the folder listing) is the largest second home. It copies from `dump.cs`:

- The six Star Power field lines with their widths (lines 71 to 78), and the other probe offsets `0x20/0x30/0x38/0x8C/0x94/0x100/0x198/0x2E0` (lines 95 to 118). These duplicate constants.py's offsets, which the file says it checked (line 93: "The dump agrees with each").
- The full field list of the base engine class 8288 (lines 125 to 222, 70 field lines) and the drums engine class 8255 (lines 227 to 256, including seven `const` values: `0.25`, `10`, `0.075`, `0.5`, `0.75`, `2`, `4`).
- Six method RVAs that constants.py does not hold: `0x20F5640` (ActivateStarPower, slot 14), `0x20F6800` (per-update SP, slot 12), `0x20F37E0` (end time), `0x20F5F70`, `0x20F6A50`, `0x20F6EB0` (score adders), plus `0x20DEAA0` (press handler) at line 58.
- About 25 class identities by TypeDefIndex (8288, 8253, 8255, 8496, 8576, 8617, 8326, 8263, 8264, 176, 1048, 449, 901, 676, 678, 641, 297, 8286, 8516, 8538, 8661, 8346, 8526, 451, 1435), with the readable field lines of `SongEntry`, `GlobalVariables`, `GameManager`, `BasePlayer`, `SPBar`, `ScoreData`.
- Three enums with values: difficulty (`None=-1, Easy=0 ... Expert=3`, lines 416 to 423), instrument (`Drums=6, ProDrums=9`, line 433), modifiers (`None=1, Shuffle=64, DoubleKick=2, NoKick=4, KickOnly=8`, lines 450 to 457).
- Three `script.json` TypeInfo addresses: `GlobalVariables_TypeInfo 0x3CEF890`, `SongEntry_TypeInfo 0x3CD1FF8`, `GameManager_TypeInfo 0x3CEE750` (lines 300, 304).

### Addresses in docs

A grep for six- or seven-digit hex literals (`\b0x[0-9A-Fa-f]{6,7}\b`) over `docs/**/*.md` finds 327 occurrences in 42 files. The heavy files: `docs/superpowers/plans/2026-10-03-step2-parser-rules.md` (64 lines), `docs/audit/ch-evidence.md` (41), `docs/audit/drum-lanes/FINDINGS-play-time.md` (35), `docs/handoffs/2026-10-18-song-length-end-event-handoff.md` (31), `docs/adr/0023-chart-rules-follow-clone-hero-code.md` (20), `docs/audit/note-shuffle/step0/open2/README.md` (20). The count includes a few non-RVA hex values and many repeats, so the number of distinct functions is lower; a scout should dedupe it. None of these docs is checked by any test: `tests/test_docs_match_code.cpp` reads only `docs/adr`, `docs/agents`, README, CONTEXT, UserGuide, development.md and one note (lines 70 to 81), and it checks backticked Hydra names and `<!-- default: -->` markers, not game addresses.

Count, in short: 37 addresses in constants.py (the executable home); about 13 more RVAs and 25 class identities and 3 enums in il2cpp-fields.md with no executable home; 327 address mentions across 42 docs with no home at all; field names used by the Frida agents whose home is the live game.

## 2. The dumps in hydra-data

### Files and sizes (folder listing, 2026-10-28 timestamps)

`6142`: `dump.cs` 36,453,454 bytes; `facts.json` 19,792,675; `il2cpp.h` 57,781,836; `script.json` 108,796,190; `stringliteral.json` 1,977,914; `DummyDll\` with 97 files (`CloneHero.dll` 3,021,824, `StrikeCore.dll` 850,432, `Il2CppDummyDll.dll`, `__Generated`, and the Unity, .NET, Rewired, MessagePack and other library stubs).

`4045`: only `facts.json`, 1,463,133 bytes. No dump.cs, no DummyDll, because 1.0 is a Mono build (README line 27).

### What each file is

- `dump.cs`: Il2CppDumper's text listing, one C#-shaped block per class with `// TypeDefIndex: N` after the class line, each field with `// 0xOFF`, each method preceded by `// RVA: 0x... Offset: 0x... VA: 0x... [Slot: n]` (the regexes in `dump_facts.py` lines 41 to 45; samples in il2cpp-fields.md lines 29 to 40). Class names are obfuscated strings of modifier letters; `readable` in facts.json means plain ASCII.
- `facts.json`: one object with four keys `game_version`, `dump_file`, `classes`, `signatures` (README lines 31 to 51; confirmed by loading it: keys `['game_version', 'dump_file', 'classes', 'signatures']`). Each class has `name`, `namespace`, `kind`, `typedef_index`, `readable`, `fields`, `methods`, `base`, `interfaces`. A field is `{name, type, offset, static}`; a method is `{name, return, params, static, rva}` with `rva = -1` for methods without compiled code. `signatures` holds `GameManager`, `ScoreManager`, `DrumsPlayer` (by name) and `drums_engine` (by constructor RVA, `ctor_rva`). Counts from loading 6142: 16,065 classes, 55,622 fields, 122,605 methods, 104,484 methods with an RVA of 0 or more, 14,471 classes with readable names. 4045: 1,020 classes, 481 readable, 8,990 methods, every `rva` null, `signatures` has no `drums_engine`, `dump_file` = "Assembly-CSharp.dll, Assembly-CSharp-firstpass.dll, StrikeCore.dll, CHNet.Core.dll, CHNet.Server.dll", `game_version` = `v1.0.0.4045-master`.
- `il2cpp.h`: the C header of Il2Cpp runtime structs (`Il2CppObject`, `Il2CppClass`, `VirtualInvokeData`, ...) and every class layout, meant to be loaded into Ghidra or IDA so decompiled code shows field names. Its first 400 bytes were read to confirm; nothing in the repo loads it today (grep `il2cpp.h` over tools and docs/audit finds only the README and the dump note).
- `script.json`: a JSON object whose first key is `ScriptMethod`, a list of `{Address, Name, Signature}` per compiled method (first 300 bytes read). Il2CppDumper also writes `ScriptString`, `ScriptMetadata` and `Addresses` lists in this file, which is how the dump note got the `_TypeInfo` addresses, but I read only the head, so take the other keys as unverified. Ghidra's `ghidra_with_struct.py` from the Il2CppDumper package reads this file and `il2cpp.h` to name functions; the 2026-09-17 session tried that and hit a file-picker problem (card `2026-09-17-3ee237e9.md` line 13).
- `stringliteral.json`: a list of `{value, address}` for every string literal (first 200 bytes read). This is how the dynamics tag and "combo: {0}" references were found (ch-evidence line 260; feasibility doc line 158).
- `DummyDll\`: metadata-only stub assemblies (signatures, no method bodies) that a managed decompiler can open for names and types. The README does not describe them; this is Il2CppDumper's documented output, not something checked here.

### How they were made

`tools/ch_mcp/README.md` lines 9 to 23: `py tools/ch_mcp/dump_build.py --game "C:\Clone Hero\1.1 june\Clone Hero" --out ...\6142` runs Il2CppDumper (the copy in `C:\Users\Patrick\Downloads\Il2CppDumper-net7-v6.7.46`, folder listing) over `GameAssembly.dll` and `global-metadata.dat`; then `py tools/ch_mcp/dump_facts.py ...\6142\dump.cs --version 1.1.0.6142-final` writes `facts.json`, finding the drums engine by `RVA_DRUMS_ENGINE_CTOR`. The dump note records the run took 7.1 s (il2cpp-fields.md line 14). For 1.0, `mono_facts.py --game "C:\Clone Hero\1.0 old\Clone Hero" --version v1.0.0.4045-master` reads the Mono assemblies through `mono_reader.ps1` (README line 27).

### What "4045 is a Mono build" means for decompiling 1.0

The game code for 1.0 is ordinary .NET IL, not native x64. It is on disk: `C:\Clone Hero\1.0 old\Clone Hero\Clone Hero_Data\Managed\Assembly-CSharp.dll` (1,529,856 bytes), `Assembly-CSharp-firstpass.dll` (87,040) and `StrikeCore.dll` (67,584), folder listing dated 2026-10-28. A managed decompiler turns IL back into near-original C#, so 1.0 rules can be read as source with method names, not as assembly. `ilspycmd.exe` is installed at `C:\Users\Patrick\.dotnet\tools\ilspycmd.exe` (`Get-Command` found it). The REA skill also routes a "Managed PE/CLI assembly" to `inspect_managed_artifact` (SKILL.md line 99). The costs of this: there are no RVAs or offsets for 1.0 (facts.json has them all null), so a 1.0 rule's "address" in the reference is a class and method name, and its annotated file under `docs/clone-hero/code/` is decompiled C#, not `.asm`. Also, 481 of 1,020 classes in 4045 are readable, so 1.0 is partly obfuscated too; the drum engine must be found by a signature, not by name, which `mono_facts.py` does not do (README: "`signatures` has no `drums_engine`").

## 3. The RE tooling

### The rea MCP server

This agent cannot see any rea tool. ToolSearch returned "No matching deferred tools found" for five queries: `rea binary`, `+rea`, `open_binary decompile ghidra`, `select:mcp__rea__open_binary,mcp__rea__binary_overview,mcp__rea__analyze_function,mcp__rea__binary_session`, and `inspect_managed_artifact analyze_function pseudocode`. The only deferred tools listed to this agent are WebFetch and WebSearch. The `rea` server did deliver its instruction block ("REA provides reverse-engineering tools for local artifacts, native binaries, managed code ..."), so the server is connected to the session; the tools are just not in this planning agent's list. So I could not read the tool descriptions, and a scout brief must not assume a tool name beyond what SKILL.md names.

`C:\Users\Patrick\.claude\skills\reverse-engineer-anything\SKILL.md` (version 42, tool_count 207, lines 5 to 6) says: route a native DLL through `open_binary` then `analyze_function`, pseudocode and function/string searches, with `binary_overview` for inventory (lines 203 to 205); route a managed PE through `inspect_managed_artifact` (line 99); reuse an existing session or saved analysis database rather than reopening (lines 295 to 297); every conclusion cites Evidence IDs and separates observation from inference (lines 277 to 280); close with `close_binary` (lines 341 to 342). Whether REA keeps a Ghidra project between calls, and where, the skill does not say; `binary_session` with `{}` reports it (lines 227 to 229). The session card `2026-10-17 ecd07c4b` says REA was registered for this project "with Ghidra set".

### Ghidra projects on disk: none survive

Three headless Ghidra projects have been made, all in session scratch folders, and none exists now:

1. 2026-09-17, `C:\Users\Patrick\AppData\Local\Temp\claude\ghidra_project\CloneHero` (application.log line 39). This was the one full auto-analysis of `GameAssembly.dll`: "ANALYZING all memory and code" at 19:54:52 and "Analysis succeeded" at 20:31:05 (log lines 163 and 233), so 36 minutes 13 seconds of analysis, of which "Decompiler Switch Analysis" alone took 235.678 s (line 206). Its output survives as `C:\Users\Patrick\Downloads\Hydra\hydra-data\scratch_ms\ghidra_decompile_all.txt` (2,341,603 bytes, 987 functions per the session card) and `ghidra_decompile_output.txt` (59,506 bytes). The project folder: `Test-Path` is False.
2. 2026-10-18, `...\c433b17f-...\scratchpad\ch-song-end\proj\chp` (log line 80015). Import only, no auto-analysis: import at 09:13:01, saved at 09:13:26, and a script `DecompRvas.java <list> <out.c>` decompiled a list of RVAs on demand (line 80145), repeated nine times that morning. `Test-Path` on `...\ch-song-end` is False.
3. 2026-10-28, `...\6a549de0-...\scratchpad\ch-dynamics\proj\chp` (log line 81479). Same pattern: import 21:13:20, script `DecompAt.java <addrs> <out.c>` at 21:13:48 (line 81609), three batches. `Test-Path` on its `proj` folder is False.

The one stray `C:\Users\Patrick\AppData\Local\Temp\3wqfbpci.gpr` (2026-10-18) is an empty directory with one item, not a project. No `.gpr` or `.rep` exists under Downloads, Documents, `C:\Clone Hero`, `AppData\Local\Temp\claude`, `.claude`, Roaming or Local (recursive search, depth 5).

What this says about time: a full auto-analysis is about 36 minutes on this machine (one measurement, 2026-09-17). An import with no analysis plus per-function decompile is under a minute per batch (25 to 28 s from import to save on 10-18 and 10-28), and that is how the song-end and dynamics work got decompiled C. The session card's "about 20 minutes" (card `2026-09-17-3ee237e9.md` line 13) undercounts the log. A fresh project for the reference should run the full analysis once (about 36 minutes, run in the background by one scout) and then be kept somewhere that survives the session, because every project so far was lost with its scratchpad. The `GameAssembly.dll` identity is MD5 `106e15e5bf799ea62f916e8ceaf38bc6` (log lines 63 and 80037).

### How the committed .asm files were produced

Not with Ghidra. `docs/audit/note-shuffle/step0/open2/README.md` line 3: "`dis.py` is the capstone wrapper used"; `open2-refute/README.md` line 3: "produced fresh from GameAssembly.dll with `mydis.py` (a thin wrapper over the il2cpp folder's chdis.py, run with `py -I`)". The scout scripts import `chdis` (`scout-flag/getters.py` line 7, `tools.py` lines 12 to 13, `open34-refute/dis.py` line 12) and use `capstone.Cs(CS_ARCH_X86, CS_MODE_64)` (`scout-flag/getters.py` line 9). `chdis.py` lived in session 9168b711's scratchpad `il2cpp\` folder (ch-evidence.md line 7); that folder no longer exists (`Test-Path` False). Capstone 5.0.7 and frida 17.18.0 are installed (`py -c "import capstone, frida"`). So the repo's `dis.py` wrappers are broken until `chdis.py` is rewritten; it was a small module (PE section table, `rva2off`, `secs`, `names` from script.json, a `dis(rva, maxbytes)` printer). The 2026-10-09 FINDINGS.md line 7 says three Fable scouts plus skeptics read the DLL this way, with the listings committed under `open1/`, `open2/`, `open34/` and `-refute/` folders. The `re\` folder that ch-evidence.md cites (`re\f215C750.asm` and so on) was never committed and is gone with that scratchpad.

Committed listings today (folder counts and listings, this survey): 179 `.asm` files under `docs/audit/note-shuffle/`, and 35 `.asm` files under `docs/audit/drum-lanes/`, which hold more than lane code: `f20f5490_deactivate_sp.asm`, `f20f5540_slot9_base_and_activate_sp.asm`, `f20f63f0_base_hit.asm`, `f20f5810_base_update.asm`, `f20f3610_base_miss.asm`, `f20f6a50_advance_schedule.asm`, `f20f4b60_combo_break.asm`, `f20deec0.asm`, `f20deaa0.asm`, `f20de770.asm`, `spec_213e0c0.asm`, `f2155200_mid_reader_lane_case.asm` and the rest. So the Star Power activation, deactivation, base hit and update routines already have committed listings, and the reference's `code/` folder can start by moving or pointing to these rather than re-listing them.

## 4. A single home and a stated check for game facts

The constraint that shapes every option: the dumps live in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\`, outside the repo, so CI cannot read them, and `facts.json` for 6142 is 19.8 MB, too big to commit. Also, 1.0 has no addresses at all, so its "facts" are names only.

### Option A: a committed extract, checked two ways (recommended)

Make one small committed file the home for every game fact the pages and tools cite: `docs/clone-hero/code/game-facts.txt` (or `.json`), one line per fact: build, kind (class, field, method, rdata, enum), TypeDefIndex, dump name, offset or RVA, and our plain label. A script `tools/ch_mcp/extract_facts.py` writes it from the dumps and records each build's `GameAssembly.dll` MD5 and `game_version` in the header; it refuses a label whose dump entry is missing or ambiguous (as `build_facts.method_name_by_rva` already does). The extract is small (the pages will cite on the order of a hundred to two hundred facts, going by the 327 doc mentions with repeats).

Check 1, at every merge and in CI: extend `tests/test_docs_match_code.cpp` (which the precheck already prints as an `always:` line) to read every page under `docs/clone-hero/` and, for each marker like `<!-- game: 6142 method 0x20DEEC0 -->` or a backticked label, confirm the extract has that exact row. A page address with no extract row, or one that disagrees, fails. The same test asserts every `RVA_` and `OFF_` value in `constants.py` equals its extract row, so constants.py stays the executable copy but can no longer drift from the home. If Hydra links no JSON reader, use a tab-separated text file; the parse is a few lines of C++.

Check 2, local only: `py tools/ch_mcp/extract_facts.py --check` re-derives the extract from the dumps and diffs. It runs when the dumps are rebuilt (a new game build) and whenever a writer adds rows. It cannot run in CI. Its result is one line in the commit message of the change that touched the extract.

Cost: one Python script with tests (the parsing is already in `dump_facts.py`), one addition to the C++ docs test, markers on the pages. Roughly one executor each for the script and the test.

Strongest case against: it adds a third copy (dump, extract, page), and the extract can be stale against the dump without CI noticing, because only the local check reads the dump. The MD5 header makes staleness detectable but not automatic. It also does nothing for the Frida agents, whose field names are resolved live; a renamed field would only show as a run-time failure.

### Option B: constants.py becomes the home for everything

Grow `constants.py` to hold every RVA, offset and class index the pages cite, and have a Python test compare the pages to it. Fewer new files. Against it: the probe's constants file would hold hundreds of values nothing executes, the C++ docs test cannot read a Python module so the check moves out of the `always:` line into pytest (which the precheck does not run), and constants.py is still a copy of the dump with no check against it. 1.0 names would not fit its shape at all.

### Option C: pages cite the dump directly, checked by a local script

Each page fact carries a `<!-- dump: 6142 TypeDefIndex 8288 field 0x68 -->` marker and `tools/ch_mcp/check_pages.py` verifies it against `facts.json` in hydra-data. Least new code and no third copy. Against it: the check runs only on this machine and only when someone runs it; CI and the merge `always:` line cannot see it, which is exactly the "unchecked without saying so" the request forbids unless the user accepts it.

Recommendation: A, with the extract as a text table and the two checks named above. The user should be told the residual gap in plain words: the home is checked against the pages and constants.py at every merge, and against the game's dump only by hand on this machine.

## 5. Rules with code already found, and rows reachable but never decompiled

"Found" means a repo document cites the RVA (and, where noted, a committed listing). The listings cited by ch-evidence.md (`re\...`) are not committed, so those rows are "address known, listing lost".

### Area 1, finding and reading a chart

Found: the `.chart` line parser `0x213AFC0`, dispatcher `0x213D2D0`, `N` handler `0x213D620` with modifier map `0x213D160`, `E` events `0x213CAF0` with exact string compare `0x20FE510` (solo, soloend, disco, quoted events), `S 2` phrases and `S 64` fills `0x213E0C0`, the `[mix N drums]` parser `0x215C750`, the `.mid` drum-track reader `0x2155050`, track-name matching `0x2155FDF/0x2156134/0x21564A8/0x21564D7/0x2155F8D`, the events track `0x2155F36/0x2155F3D`, MThd `0x2155C00`, MTrk `0x214F6D0`, resolution setter `0x212C0B0`, chart length `0x212E130/0x215FED0`, end event `0x212C7E0/0x212E310`, load coroutine `0xD8D80`, audio length `0x36B700`, `song_length` write-back `0x358850`, the binary song-cache reader `0x20E5E70/0x215B7C0` (ch-evidence, ADR 0023, step0/open1, the song-end handoff). Committed listings exist for the open1 chain.
Reachable, not decompiled: `.sng` and `.srb` readers (no RVA cited in my slice; `docs/srb-format.md` was decoded from data per memory `srb-format-support`), `song.ini` key handling beyond `song_length` (`SongEntry` class 176 is readable, so its loader is findable by field writes), `.mid` solo markers (note 103) and the `SoloCounter` class 123, section events, `Offset` versus `delay` (chart class 8538 field `0xB0` is the offset per the song-end handoff, but the delay read is uncited).

### Area 2, building the note list

Found: the note builder `0x20D2EA0` (order: flam, 2x-kick drop, 5-lane remap, disco flip and cymbal fold, then shuffle), lane map `0x20D36A0`, play-note list `0x20D4BE0`, game-note ctor `0x214BF10`, shuffle `0x20F0C30` and lane draw `0x20F0B30`, drum finish step `0x215E280/0x215DBD0` (cymbal, tom, flam, ghost, accent with no tag check in `.chart`), MIDI dynamics tag `0x21555F1/0x21557A5/0x21557BB`, 2x kick on every difficulty `0x21555CD`, disco-flip per difficulty `0x213D076`. Committed `.asm` listings exist for most of these under note-shuffle.
Reachable, not decompiled: the 2x-kick merge "clean-up step" (FINDINGS says it exists, no RVA in the prose), `0x215CD50` (how `drums0dnoflip` is read; ch-evidence line 37 "I did not re-trace"), what plain Drums drops (only the "5-lane remap" step and the star-base note are cited).

### Area 3, timing

Found: window formula `0x20DDDA0`, per-note window `0x20F3A90`, half-gap clamp `0x20F7210`, hit check `0x100DAB0`, note processing `0x20DCC90`, miss walk `0x20F5810` with the strict and inclusive compares `0x20F59E7/0x20F59FB`, the `.rdata` constants, and the no-speed-factor finding in the score adders (answer 222).
Reachable, not decompiled: calibration (classes `CalibrationMenu` 992, `CalibrationWindow` 1166, `Calibrator` 1281 are readable in dump.cs, grep of class lines), playback speed's live field (il2cpp-fields.md line 461: none found by name; `originalSongSpeed 0x5C`, `practiceSpeed 0xD8` are the closest), the song clock's source (`GameManager.songTime 0x70` is read live but its writer is uncited), precision-mode cap (constants.py line 241: "nobody has read precision mode's cap").

### Area 4, fills

Found: fill creation sites `0x20CEDC0` (.mid 120), `0x213E0C0` (.chart), cache `0x20E5E70/0x215B7C0`, add-fill `0x215BD50` (no direct caller), fill-to-activation `0x20CFF60` with nearest-chord `0x5DE030` and the slop `0x20D0061` to `0x20D008D`, fill-end flag `0x20D015E/0x20D2D9D`, its getter `0x214BB50`, the lenient fill-end miss in `0x20DCAD0`.
Reachable, not decompiled: every generated-fill rule in ADR 0023's "not verified" list (lines 87 to 104: length across meter changes, 6/8 beats, one authored fill disabling generation, search-position carry, equal-distance ties, downbeat distance), the fill deadline and when a fill shows up, and the caller of `0x215BD50` (ch-evidence line 108 proposes one breakpoint; the `ch` server's Frida tracer could do it). The 1.0 fill rule: nothing decompiled; `Assembly-CSharp.dll` is on disk for it.

### Area 5, Star Power

Found: activate `0x20F5640`, deactivate `0x20F5490`, per-update SP `0x20F6800` with the overflow clamp `0x20F6932` to `0x20F6986` and `0x20F681D` to `0x20F683B`, end-time `0x20F37E0`, phrase assignment `0x20D2440/0x20D0330` and cleanup `0x5DB320` from `0x215C320`, the SP-window decision in the base hit `0x20F654F` to `0x20F65FB`, phrase credit `0x20F6719` to `0x20F6738`, SP-ready `0x20F57F0`, stored times `+0x110/+0x120/+0x128`.
Reachable, not decompiled: the drain per tick and the full-bar value at `+0x1B0` (readonly, set in the base engine ctor `0x20F79E0`, listing committed under open34 but the SP lines are not annotated), activation on drums beyond the fill-end press, squeeze in and out as a rule (the hit-time comparison is found; the squeeze window the memory note calls 500 ms is not cited to code), the 3 ms leeway (Hydra's own, ADR 0023 line 78: no such constant found).

### Area 6, scoring

Found: score adders `0x20F5F70/0x20F6A50/0x20F6EB0`, drum score `0x20DEEC0` (base 50, +15 cymbal, doubled for the second-note flag), hit routine `0x20DE770`, combo increments `0x20F6524/0x20F660E`, star base `0x20F4680` from the engine ctor, overhit `0x20DE9A0`.
Reachable, not decompiled: multiplier steps (field `+0x48`, incrementer uncited), accent and ghost values inside `0x20DE770`, the solo bonus (memory `preview-solo-box-10ms-step-2026-10` says "formula decoded", source not in my slice; `SoloCounter` 123 and `StarProgress` 1169 are readable), the star multiplier table, and the `scorestats.json` writer (`ScoreData` 297 is readable with `spPhraseCount`, `spPhrasesHit`, `maxCombo`, `speed`; the literal `scorestats` can be found in `stringliteral.json`).

### Area 7, internals

Found: `GameManager` 449, `BasePlayer` 901, `GlobalVariables` 1048, `SongEntry` 176, `SPBar` 676, both engine classes with full field lists, the per-note hooks `0x20DEEC0/0x20DE770/0x100E6E0`, the press handler `0x20DEAA0` and its callers `0x20DDC80/0x20DE070`, the lane code.
Reachable, not decompiled: the bot (a grep of `dump.cs` class lines for a name containing `Bot` found none, so it is obfuscated or named otherwise and must be found through a string or through `BasePlayer`), the song-select menu (`SongSelect` 347, `SongSelectSearch` 237, `SongSelectPanel` 144, `PracticeSectionMenu` 552 are readable), `ScoreManager` 592.

### Sizing for the RE waves

The committed, annotated code is concentrated in three areas (note list and chart reading under note-shuffle; lanes, hits, misses and Star Power activation under drum-lanes), and the rest, chiefly ch-evidence's fills, phrases, dynamics and `.chart` event rows, is "address known, listing lost". So the first RE wave is mostly re-listing known addresses into `docs/clone-hero/code/` and annotating them, which needs no search; a scout with the `dis.py` wrapper restored and a Ghidra project can do about eight functions each (memory `lean-agent-fanout`). The second wave is the genuinely new rows above, of which the fills area is the largest and the bot and solo bonus the most valuable to the request's own example.
