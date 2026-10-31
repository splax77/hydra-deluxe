# Feasibility study: a full MCP server for Clone Hero

Written 2026-10-28 for Clone Hero v1.1.0.6142-final (`C:\Clone Hero\1.1 june\Clone Hero\`). An MCP server is a helper that gives an agent a set of tools to call, the way `tools/hydra_ui_mcp.py` lets an agent drive Hydra. The goal here is the same for Clone Hero. An agent should be able to start the game, pick a song and its settings, play it with exact inputs, and read back what the game saw. That means the song clock, score, combo, multiplier, Star Power, each note's hit or miss with its timing offset, and the end-of-song breakdown.

## The answer

It is feasible. Most of it can be built with tools already on this machine, and no game file has to change.

The plan has three layers. The first is free: the game already launches songs from the command line with bot players, and it writes a detailed end-of-song file. The second reads the game's live state from inside the process with Frida. Frida is a tool that attaches to a running program and runs our script inside it, and an earlier session already used it on this game install. The third is per-note results and exact-time inputs, which means finding and hooking the game's hit-detection code. That third layer is the real unknown. The hit code is the most heavily scrambled part of the game, so it needs some reverse-engineering work before anyone can promise it.

The scrambling (obfuscation) turned out to be much less of a blocker than it looked. The game's on-screen components keep their real names, and those are exactly the objects an MCP wants to read first. The details are in "The obfuscation, measured" below.

One thing is the user's call, not a technical question. Clone Hero's EULA forbids reverse-engineering or modifying the game. Every layer past the first does some of that, and so does the existing `tools/ch_probe` work. See "Risks". The user made that call on 2026-10-28 and chose to go ahead; "Results of the four tests" below says how.

The four tests ran on 2026-10-28. All four settled their question: layers A and C both work, and the per-note hook exists. The results are at the end.

## What exists today

`tools/ch_probe` is a Python tool built in September for one question: how wide is the drum hit window? It reads the game's memory from outside the process and presses keys with Windows' `SendInput`. It finds the drum engine object by scanning memory for two hit-window constants. It then reads three fields from that object: the song clock (offset 0x100), the score (0x94), and a value it guesses is the last hit time (0x2e0, never confirmed live). `experiments/play_chart.py` auto-played the chart "Slipping" on 2026-09-25 and hit its notes. It counts a hit when the score goes up. `experiments/results/` holds the two logs from those live runs.

It cannot do much else. It has no menu or song-select control, so a person starts each song by hand. It has no per-note result and no combo, multiplier or Star Power, and it only knows the drums engine. Every offset is tied to build 6142, and the tool refuses to run on any other build. Keys only arrive when the game window has focus. And its engine search waits forever if no song is playing.

The project's Hydra MCP server is the shape to copy. It is a Python stdio server using the `mcp` package. It owns the launch, the stop and the crash report, and returns every answer as text. It works because Hydra has a command channel built in. Clone Hero has none, so a Clone Hero server has to supply its own way in.

## What the game gives for free

Four things need no reverse engineering at all.

**Command-line song launch with bots.** The official manual documents `--song <folder>` and `--player <Instrument>,<Difficulty>` (up to four times), plus `--profile`, `-i` for instrument names and `-v` for versus mode. The game then plays that chart with bot players. The manual does not say whether the game exits afterwards or writes a score. This is still useful on its own: the bots play the chart exactly as Clone Hero reads it.

**The end-of-song file.** The game writes `Documents\Clone Hero\scorestats.json` at the end of each song. Each player gets about 45 fields. They include score, base_score, note_score, sustain_score, sp_score, combo_score, solo_bonus_total, squeezed_score, notes_hit, total_notes, max_streak, sp_phrases_earned and sp_phrases_total, sp_activations, time_in_sp, avg_multiplier, the ghost and accent counts, is_fc, is_bot, and hit counts for each tenth of the song. That covers the whole end-of-song breakdown. The catch is that each song overwrites the file, so the MCP must copy it after every run.

**The session log.** Each launch writes `Documents\Clone Hero\Logs\Session-<date>_<time>.log`. It records lines such as "Loading Song: ...", "Seeking to song time:...", "OnCountdownStart" and "OnCountdownEnd". Following that log tells the MCP which state the game is in without reading memory. Two empty files, `HitDetection-Standard.log` and `HitDetection-Validation.log`, sit next to it. They look like a hit log that is switched off. Nobody has found the switch yet. If one exists, it might hand over per-note results for free.

**Settings as a plain file.** `settings.ini`, together with `settings_schema.json` (every setting with its default, minimum and maximum), lets the MCP set no-fail, speed and the rest before it launches the game.

Put together with bot launches, those files make a cheap first win for Hydra. Have Clone Hero's bot play each chart in `testdata/input`, then compare its total_notes, sp_phrases_total and base_score with what Hydra computes for the same chart. That checks Hydra's chart reading against the game itself at scale, with zero reverse engineering. Whether the bot also spends Star Power the way a player would is unknown, so its total score may not be comparable.

## The obfuscation, measured

The game was scrambled with Beebyte Obfuscator 3.13.0; the dump contains a marker class named `__BB_OBFUSCATOR_VERSION_3_13_0`. Beebyte replaces names with strings of odd Unicode letters such as `ʺʻʻˁˀʹʿʲʻʴʿ`. On 2026-10-28 a scout ran Il2CppDumper v6.7.46, a tool that rebuilds a Unity game's class list from its files, and counted the result. Il2CppDumper was already in `C:\Users\Patrick\Downloads\Il2CppDumper-net7-v6.7.46\`. It needs `DOTNET_ROLL_FORWARD=Major` to run on the installed .NET 8 and 10. The durable copy is in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\`, made by `tools/ch_mcp/dump_build.py`.

Only 4 of the 95 assemblies the scout counted (the DLLs that hold the game's code) are scrambled:

| Assembly | What it holds | Types scrambled | Methods scrambled |
|---|---|---|---|
| CloneHero.dll | the game's scripts | 985 of 1,451 (68%) | 7,941 of 11,926 (67%) |
| StrikeCore.dll | the chart and play engine | 427 of 449 (95%) | 3,095 of 3,577 (87%) |
| CHNet.Core.dll | online play | 167 of 176 | 1,426 of 1,614 |
| CHNet.Server.dll | online server code | 15 of 22 | 288 of 303 |

Everything else keeps its real names. That includes Unity, TextMeshPro and Rewired (the input library the game uses). The scout counted these with a regex, so treat them as close, not exact. CloneHero.dll's 68% is flattered by about 400 types of Rewired menu code compiled into it, so the real game logic is more scrambled than that.

The objects an MCP wants first kept their names. `GameManager` keeps 40 of its 60 fields. They include `songTime`, `songLength`, `isPaused`, `isSongOver`, `isSongPlaying`, `songLoaded`, `practiceSpeed`, `currentSection`, `scoreManager`, `playerObjects` and `starProgress`. `ScoreManager`, `SongSelect`, `SongEntry`, `PlayerStatsBox`, `BasePlayer` and `DrumsPlayer` all keep their class names, and many of their fields. These are Unity components (classes attached to objects in a scene). Beebyte has to leave their names alone, because Unity looks them up by name.

What is scrambled is the logic behind them. The score, combo and multiplier counters inside `ScoreManager` are scrambled. So are the play engine class and almost all of StrikeCore. There are still handholds. The game's 23,972 text strings are not encrypted, for example "Perfect Full Combo", "combo: {0}" and "Practice Star Power". Every call into Unity, Rewired or the BASS audio library keeps its name. Some compiler-made helper classes keep readable fields, such as `spPhrases`, `spStart` and `spEnd` in StrikeCore. And the metadata file holds a plain list of the original source file paths and class names (776 paths, for example `Assets\Scripts\EndOfSong\EndOfSong.cs`). That list does not say which scrambled class is which.

The known tool that unscrambles Beebyte is Beebyte-Deobfuscator, a plugin for Il2CppInspector. It needs an unscrambled build of the same game to compare against, and there is none on this machine. So the plan below does not depend on it.

The open question was whether the scrambled names change with each game update. Beebyte can reuse a saved name table between builds, or draw new names each time. Test 4 below compared v1.0 with v1.1 and found the names mostly redrawn. So the plan assumes the worst: scrambled names and all memory offsets change with every update.

## Four ways to build it

**A. Files and command line only.** Launch with flags, edit settings, follow the session log, read `scorestats.json`. There is no reverse engineering and no update risk. There is also no live state, no per-note results and no human-style play. This is the first layer of every option below.

**B. Read memory from outside the process (extend ch_probe).** Il2CppDumper's `script.json` gives the address where each class's description lives in `GameAssembly.dll`. From there a reader can follow pointers to the one `GameManager` object and read its fields at the offsets the dump gives. This needs nothing new installed, and it already works for the clock and score. But it can only poll, so a poll that runs a few milliseconds apart can miss a single note's result. It also cannot call the game's code, so menus have to be driven by keypresses. And every address has to be re-derived after each update.

**C. Frida inside the process.** The MCP server attaches Frida to the running game and loads a small script. That script uses `frida-il2cpp-bridge`, a library that finds Unity classes, objects and methods at run time. It can find readable classes by name, which survives updates. It can hook a method, so each note's hit or miss is pushed to the MCP as an event instead of polled. And it can call game methods, which opens a path to driving menus directly. Frida 17.18.0, its Python bindings and Node 26.8.1 (needed to compile the bridge script) are all installed. Frida already ran against this game install on 2026-09-15 to capture decrypted song audio. That session also installed `frida-il2cpp-bridge` and wrote a scan script for it, but its card records no result from running the bridge, so whether the bridge itself works on this build is still untested. The costs are the reverse-engineering work to find the hit code, and the bridge library not saying anything about scrambled names. That is only a problem for the scrambled classes, which have to be found by their shape (field count and types, the strings they use, the readable calls they make).

**D. A mod loader (MelonLoader or BepInEx 6).** A mod loader puts a C# plugin inside the game. That gives the richest access: plain C# calls into the game, and a named pipe for the MCP. The current loaders claim support for this metadata version (31), but only through maintainer comments and pre-release builds. Nobody has published a working mod for Clone Hero v1. The only Clone Hero mod found, "My Clone Hero Tweaks", targets the old v0.23 build. This option also writes files into the game folder, which is the most direct clash with the EULA's "modify" clause. And one open BepInEx issue shows its interop generator crashing on another game with this same metadata version.

## Recommendation

Build A first, then C. Keep B as the fallback for the fields it already reads, and skip D unless C runs into a wall.

C beats B because per-note results need hooks, not polling, and menus are far easier to drive through the game's own methods than through keypresses. C beats D because it changes no game file, Frida has already run in this game, and its tools are installed (the bridge itself is untested here). D's support for this game is untested.

The server would follow the shape of `tools/hydra_ui_mcp.py`: a Python stdio server, one tool per action, answers as text. The tools would fall into four groups.

Lifecycle: start the game with chosen settings and window size, stop it, report a crash, and return the state ("menu", "song select", "countdown", "playing", "paused", "results") from the session log and `GameManager`.

Song control: launch a chart with bots through the command line; later, pick a song and difficulty for a human-style player through `SongSelect`.

Play: press lanes at chosen song times. Start with `SendInput`, which is proven but needs window focus and has a few milliseconds of jitter. Move to injecting inside the process once the input path is understood. Rewired is readable, and the game polls input on its own clock (`poll_rate = 656` in `settings.ini`). One line of the existing design carries over: trust the time the engine recorded, not the time the key was sent.

Observe: a snapshot of clock, pause and end state, score, combo, multiplier and Star Power; a stream of per-note events (note, hit or miss, offset in ms); and the end-of-song breakdown copied from `scorestats.json`.

To keep it working across updates, store the class map as signatures rather than offsets. A readable class is found by name. A scrambled class is found by its shape and the strings it uses. After an update, the MCP regenerates the dump and finds each class again. If a signature matches anything other than exactly one class, it stops with an error, the same way `ch_probe` refuses a build it does not know.

## Risks

**Online play and leaderboards.** The game carries a large online stack (CHNet, MagicOnion, Socket.IO). The MCP should run only offline, on a test profile. It is unknown whether scores from an instrumented session would upload anywhere.

**The hit code.** Before the tests this layer had the lowest confidence: the scout put it at about 55%, against about 85% for the clock, state, song select and results layer. Test 3 found the per-note methods, so it is no longer the blocker. What is left is reading a note's result and offset from the hook's arguments.

**Updates.** Test 4 shows the scrambled names change across a major update. Plan for every update to need a re-derive step.

**Input.** `SendInput` needs focus and has no timing promise. The wiki warns that keyboards can drop keys in chords. Exact-time testing needs input inside the process, which is still to be proven.

## Next steps: four small tests that settle the unknowns

Each test is short and needs the game running with the user present. They ran on 2026-10-28; "Results of the four tests" below has what each found.

1. **Bot launch.** Run `"Clone Hero.exe" --song "C:\Clone Hero\songs\Hydra Probe\Window Map" --player Drums,Expert`. Check whether `scorestats.json` is rewritten, whether `is_bot` is true, whether the bot uses Star Power, and whether the game exits or returns to a menu. This settles how much layer A can do.
2. **Frida attach.** Attach with `frida-il2cpp-bridge`, find `GameManager`, and print `songTime` and `isSongOver` while a song plays. This proves layer C's foundation.
3. **Find the hit method.** Start from the drum engine object `ch_probe` already finds. Hook its methods during a short song, and keep the one that fires once per note. Use the "combo: {0}" string and the readable calls to confirm. This settles the per-note layer.
4. **A second build.** If the user has, or can get, any other Clone Hero build, dump it and compare the scrambled names. This settles the update question. It would also open the Beebyte-Deobfuscator route if that build were unscrambled.

One lead is worth a look along the way: the empty `HitDetection-*.log` files may have a setting that fills them. (An earlier draft also named a `RemoteControl` class. The dump holds no such class. The word is only the value `RemoteControl = 100` of Rewired's `JoystickType` enum, so it is not a lead.)

## Results of the four tests

The tools are in `tools/ch_mcp/`, built and reviewed from the plan `docs/superpowers/plans/2026-10-28-ch-mcp-feasibility-tests.md` (removed; `git show 444631e6:docs/superpowers/plans/2026-10-28-ch-mcp-feasibility-tests.md`). The raw outputs (stats copies, clock samples, trace tables, backups) were written to that session's scratch folder, under `l1`, `l2`, `l3` and `l4`. That folder was not kept. Every run backed up the five user files first and restored them afterwards, byte for byte.

**The EULA.** The reviewed plan's first question says the EULA forbids reverse-engineering. Two later edits said it allows it: the session handoff (`docs/handoffs/2026-10-28-ch-mcp-feasibility-tests-handoff.md` (removed; `git show 444631e6:docs/handoffs/2026-10-28-ch-mcp-feasibility-tests-handoff.md`), committed on 2026-10-30 after it was corrected; it records that it said "allows" until 2026-10-28 22:00), and an uncommitted one-word change to the plan's old worktree copy (`.claude/worktrees/ch-mcp-tests-plan`, edited at 20:17; that worktree has since been removed). Neither says who made it. The main session read that worktree copy, asked the user's approval with the word "allows", and the user approved tests 2 and 3 on that wrong premise. The EULA's License Restrictions (https://clonehero.net/eula, last updated 4 February 2021) say you agree not to "Modify, make derivative works of, disassemble, decrypt, reverse compile or reverse engineer any part of the Application". When the mistake came out, after the tests had run, the user chose to go ahead and take the risk ("Proceed, my call"). The tools changed no value in the game and no file in its folder. They did read its code and fields, and the hit tracer's hooks (Frida `Interceptor.attach`) patch the start of each traced method in the running game's memory while the tracer is attached.

### Test 1: the bot launch (layer A)

Passed. The launcher played Window Map and Trippin SP Spike with the bot from the command line, on ProDrums and on plain Drums, and closed the game with `--stop` afterwards. The game does not exit when the song ends: the launcher found it still running 30 s later (on a menu or the results screen; it cannot tell which). The game numbers came from its `scorestats.json`, and the Hydra numbers from `hydra_replay score` on the same `notes.chart`, with `--prodrums 0` for the plain Drums run. One key-press run on the same chart, kept at `tools/ch_probe/experiments/results/overlay-spike/run1_cal0_scorestats.json`, was compared with them.

The findings this test listed now live in docs/clone-hero/07-internals.md (IN-34 and IN-35 for the scores and the dynamics, IN-37 for the stats file, IN-53 for the game staying open, IN-31 and IN-21), 05-star-power.md (SP-44), 06-scoring.md (SC-14, SC-15, SC-37) and 02-note-list.md (NL-61 to NL-65, NL-67).

Two rulings came out of this test. The user ruled that keyboard input never registers dynamics. The user also ruled the plain-Drums loss of dynamics a Clone Hero bug, so Hydra keeps scoring dynamics in its normal-drums mode, and charts with dynamics are compared as ProDrums. The test settled that bot runs can check how Hydra reads a chart, and its score with no Star Power, but nothing about Star Power paths.

### Test 2: the Frida attach (layer C's foundation)

Passed. The agent found `GameManager` by name and read its fields over one Window Map run, with no errors. The plan's step waited for `OnCountdownStart` in the session log before attaching, which did not work, so the working run attached 8 s after launch instead. The findings now live in docs/clone-hero/07-internals.md (IN-02, IN-43).

Two readings from that run have no row on the pages yet, so they stay here. `songTime` advanced 59.894 s over 59.898 s of wall time before the song ended. After `isSongOver` turned true (at `songTime` 63.21 s), the clock froze at 63.56 s for the rest of the run.

### Test 3: the hit method (layer C's per-note layer)

Passed for the two bot runs (Window Map, 142 notes, and Edge Walk, 120 notes). In both bot runs no other method came within one of the note count (the next lowest was 2,402 and 2,627 calls). The key-press run on Window Map (`play_chart.py`) counted 139 of 142, so the "once per note" test did not pass there: the trace ended on its cap (song length plus 30 s, the tracer's `ended: done (agent: cap)` line) while the song sat paused. The three missing notes were at 58.86 s, 58.99 s and 59.08 s; the trace's last counted note was at 55.86 s. The pause as the cause is inferred from `play_chart.py` waiting for the song to start. `play_chart.py` itself hit all 142 notes and read 25,550 from the game after the last note, the same as Hydra's score. A second bot run logged each call of the two engine methods with the game clock. All 284 calls on Window Map landed within 3.0 ms after their nearest note (the tool's own limit is 85 ms, and the median was 1.75 ms). A bot hits on time, so this shows where the hook fires, not how a late or early hit looks. Reading the hit's result and offset from the note object was the next piece of work.

The findings now live in docs/clone-hero/07-internals.md (IN-17 for the three per-note methods, IN-18 for when they fire and the 139-of-142 trace, IN-45 for the "combo: {0}" string).

### Test 4: a second build

The user had v1.0.0.4045 at `C:\Clone Hero\1.0 old\`. It was read with .NET's metadata reader instead of Il2CppDumper, and `compare_dumps.py` compared the two builds. Both builds' class lists are in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\` (`4045` and `6142`). The findings now live in docs/clone-hero/07-internals.md (IN-48 to IN-51). One fact has no row on the pages yet, so it stays here: the study read 1.0 as built with Unity 2021.3.45f2. The study also said two things that IN-50 holds as unchecked claims: the drum engine's 1.1 name is not in 1.0 at all, and only 13 `GameManager` field names appear in both builds (a one-off check; `compare_dumps.py` prints the 63 to 20 scrambled and 0 to 40 readable counts).

The test's conclusion for the tools: the MCP must find scrambled classes again by shape after an update. Whether a small patch keeps more names, this test cannot say, because 1.0 to 1.1 also changed the Unity version and the scripting backend.

### What to fix in the tools

Three small faults showed up. When its cap passed, `frida_attach.py` printed an error ("session detached: application-requested") and exited 1, although all its samples were good (seen in the late-attach run, which had a 15 s cap). `trace_hits.py` runs through the same code (`run_agent`), so it should do the same when its runner cap fires; that was not seen, because the traces ended on the agent's own cap line. `bot_run.py --no-wait` prints its restore command with `py -I`; that runs, but is the form the Frida tools reject. And the cap of a trace that waits on a paused song should start when the song plays, not at attach.

## Sources

The local facts come from three read-only scouts on 2026-10-28, and the main session spot-checked them against the files. The checked facts: the game version (`Clone Hero_Data\version.json`), the assembly list (`ScriptingAssemblies.json`), the `scorestats.json` fields, the class names in `global-metadata.dat`, and the `GameManager` fields and Beebyte marker in the regenerated `dump.cs`. The Unity version, 2022.3.62f2, comes from the scout's reading of `globalgamemanagers` and `UnityPlayer.dll`. The September Frida work is in the session card for session 450488b5 (2026-09-15). The `ch_probe` facts come from its own README and `constants.py`.

Web sources: the Clone Hero EULA (https://clonehero.net/eula), the command-line manual page (https://wiki.clonehero.net/books/clone-hero-manual/page/chart-previews-via-the-command-line), and the data-locations page (https://wiki.clonehero.net/books/clone-hero-manual/page/data-locations). The main session re-read the first two. The rest: BepInEx metadata-31 issues #1115, #1120 and #1231 (https://github.com/BepInEx/BepInEx/issues); MelonLoader and Cpp2IL releases (https://github.com/LavaGang/MelonLoader, https://github.com/SamboyCoding/Cpp2IL); Il2CppDumper's v31 issue #797 (https://github.com/Perfare/Il2CppDumper/issues/797); frida-il2cpp-bridge (https://github.com/vfsfitvnm/frida-il2cpp-bridge); Beebyte-Deobfuscator (https://github.com/OsOmE1/Beebyte-Deobfuscator); "My Clone Hero Tweaks" (https://github.com/Biendeo/My-Clone-Hero-Tweaks); ViGEmBus's retirement notice (https://github.com/nefarius/ViGEmBus); and the MCP Python SDK (https://github.com/modelcontextprotocol/python-sdk; `mcp` 2.3.0 is installed here). Reddit and the Clone Hero Discord could not be searched, so "nobody has modded v1" means nothing was found, not that nothing exists.
