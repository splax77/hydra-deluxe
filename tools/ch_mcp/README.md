# tools/ch_mcp

This folder holds the code for the Clone Hero MCP feasibility tests. Those tests check whether an MCP server (a tool server an agent can call) can be built on top of a running Clone Hero. The plan `docs/superpowers/plans/2026-10-28-ch-mcp-feasibility-tests.md` (removed; `git show 444631e6:docs/superpowers/plans/2026-10-28-ch-mcp-feasibility-tests.md`) was the spec for those tests, and `docs/research/2026-10-28-clone-hero-mcp-feasibility.md` holds their results. The server built on what they proved is described under "The server" at the end of this page.

## Making the dump and its facts file

Il2CppDumper reads the game's compiled code and its metadata file and writes out every class it finds. The dump is too big for the repo, so it lives outside it, in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\<build>\`.

First run the dumper over the game folder:

```
py tools/ch_mcp/dump_build.py --game "C:\Clone Hero\1.1 june\Clone Hero" --out "C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142"
```

It refuses an output folder inside a git checkout. It counts a run as good from the dumper's own output and files, not from its exit code, because the dumper crashes at its final "press any key" prompt; the reasons are in the docstring of `dump_build.py`. When the run is good it prints the sizes of `dump.cs`, `il2cpp.h`, `script.json` and `stringliteral.json`. When it is not, it prints what was missing and the dumper's output. A run that hits the time cap (`DUMPER_CAP_S` in `dump_build.py`) stops and leaves the folder as it is. `--dumper` points at another `Il2CppDumper.exe`.

Then turn `dump.cs` into `facts.json` beside it:

```
py tools/ch_mcp/dump_facts.py "C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs" --version 1.1.0.6142-final
```

`--version` is stored as given. The drum engine's class name is scrambled, so the script finds it by its constructor's address, which defaults to `RVA_DRUMS_ENGINE_CTOR` in `tools/ch_probe/constants.py`; `--engine-ctor-rva` passes another. If that address belongs to no class's constructor, or to more than one, the script stops with `engine ctor RVA matched N classes`. It prints the engine class it found and the size of the file it wrote.

The tests run with `py -m pytest tools/ch_mcp/tests/test_dump_facts.py -q`. They use a small hand-written `dump.cs` and a fake dumper, so they never touch the game.

Clone Hero 1.0 is a Mono build: its game code is plain .NET assemblies in `Clone Hero_Data\Managed\`, so there is no dump to make. `py tools/ch_mcp/mono_facts.py --game "C:\Clone Hero\1.0 old\Clone Hero" --version v1.0.0.4045-master --out "C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\4045"` reads the assemblies listed in `GAME_ASSEMBLIES` through `mono_reader.ps1` (PowerShell 7's own metadata reader, nothing installed) and writes the same `facts.json` shape. Mono metadata holds no addresses, so every field `offset` and method `rva` is null, and `signatures` has no `drums_engine`, because that entry is found by an address. `typedef_index` is the type's row number in its own assembly's TypeDef table; rows restart in each assembly, so two classes can share one. `dump_file` lists the assemblies read. The version string `v1.0.0.4045-master` came from a byte search of `Clone Hero_Data\globalgamemanagers`, where it is the only string of its shape (the exe's version info gives only the Unity version). The script cannot read it, so you pass it by hand with `--version`; `4045` is the build number in it, and the output folder is named after that, as the 1.1 folder is named `6142`.

## What facts.json holds

`facts.json` is one JSON object with four keys, written as UTF-8 with the scrambled names kept as they are (not escaped), so read it with `encoding="utf-8"`. Every reader of a game build writes the same shape, `dump_facts.py` for an Il2CppDumper dump and `mono_facts.py` for a Mono build.

`game_version` is the version string the caller passed. `dump_file` is the name of the file the facts came from.

`classes` lists every class, struct, interface and enum, in the order the dump prints them. Each one has these keys:

- `name`: the name as the dump prints it. A nested class carries its outer class's name in front, joined by a dot.
- `namespace`: the namespace, or an empty string when it has none.
- `kind`: `class`, `struct`, `interface` or `enum`.
- `typedef_index`: the type's index in the game's metadata. In a Mono build it is a row number inside one assembly, so it repeats across assemblies and is not a key; use `name` and `namespace` to find a class.
- `readable`: true when the name is plain ASCII, false when the obfuscator scrambled it.
- `base`: the base class as the dump writes it, generic arguments included, or null when there is none. The dump lists the base and the interfaces together with no marker. So the first listed type counts as the base unless every type of that name in the dump is an interface.
- `interfaces`: the other listed types.
- `fields`: one entry per field that has an offset. Constants have none and are left out.
- `methods`: one entry per method.

A field has a `name`, a `type`, an `offset` and a `static` flag. The offset is a number counted in bytes. For an instance field it is the distance from the start of the object; for a static field it is the distance into the class's static storage. A reader whose source gives no offsets writes null. The reader of an Il2CppDumper dump always writes a number.

A method has a `name`, a `return` type, a `params` list with each parameter's type (names and default values left out; a parameter keeps the `out`, `in`, `ref`, `[In]` or `[Out]` marker the dump prints before its type), a `static` flag and an `rva`. The RVA is the method's address counted from the start of `GameAssembly.dll`, as a number. It is -1 when the dump says the method has no compiled code of its own, as with abstract methods and open generic methods. A reader whose source gives no addresses writes null. The reader of an Il2CppDumper dump always writes a number. The per-instantiation addresses the dump lists under a generic method are not recorded.

`signatures` holds the short records the feasibility tests look up: `GameManager`, `ScoreManager` and `DrumsPlayer` by name, and `drums_engine` found by its constructor's address. Each has the class's `name`, `typedef_index` and `base`, its `field_count` and `method_count`, and `readable_fields`, which lists the fields whose names are readable in the same shape as above. `drums_engine` also has `ctor_rva`, the address it was found by. A name that matches no class or more than one stops the script with `class <name> matched N classes`.

## The facts tables and the names map

The Clone Hero reference pages under `docs/clone-hero/` cite game facts: method addresses, fields by offset, and classes and members by name. `dump_facts.py --tsv` looks each one up in one build's dump and writes the facts table the C++ docs test reads, `docs/clone-hero/code/facts-<build>.tsv`:

```
py tools/ch_mcp/dump_facts.py "C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs" --version 1.1.0.6142-final --tsv docs/clone-hero --out docs/clone-hero/code/facts-6142.tsv --dll "C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll"
```

For build 4045, pass that build's `facts.json` instead of a `dump.cs`. Add `--check` to compare the table on disk with the dump without writing it; it exits 1 on any difference. Which spans count as facts, and which build a span belongs to, are decided in `dump_facts.py` (`game_fact`, `place_build` and `span_builds`). A file under `docs/clone-hero/code/<build>/` is one build's notes, so every span in it belongs to that build, whatever its headings say. The table's columns are `TABLE_COLUMNS` there. Its last column, `cited`, holds the span exactly as a page writes it, one row for each distinct span. The docs test finds a page's span by that column alone, so the tool is the only place a span is resolved.

The game scrambles most of its names, and a page never writes a scrambled name: any backticked span holding a non-ASCII character is an error, from the tool and from the docs test. A page writes our own readable name instead, taken from the build's names map, `docs/clone-hero/code/names-<build>.tsv`. Its lines that start with `#` are comments. Then comes the column line (`NAMES_COLUMNS` in `dump_facts.py`) and one row per name. A class row names a class and leaves `member` empty; a member row names one member of a class. In build 6142 a row finds its class by TypeDefIndex. In build 4045 the TypeDefIndex column stays empty and the class is found by its name, because a Mono build's indexes repeat across assemblies. When the tool resolves a span, a class part or member part that is a name in the map stands for the dump's real name. So `#8255.spAmount`, `DrumsPlayer.spAmount` and a renamed class's member all resolve. A missing map file is an empty map. Build 4045's `facts.json` lists no properties, so a span naming a property there (a map row whose member the dump does not list) resolves through the map alone: its table row has kind `property-unlisted`, and the run prints a warning naming the span.

Each run also checks the map against the dump (`check_names` in `dump_facts.py` lists what it checks). A map error makes the run exit 1. A map row that no page cites is printed as a warning and does not change the exit code.

## The server

`server.py` is an MCP server for Clone Hero: a helper program Claude Code starts, which gives an agent tools to start the game, read it and press its pads and buttons through a virtual drum kit inside the game. The plans are `docs/superpowers/plans/2026-10-28-ch-mcp-server.md` (removed; `git show 444631e6:docs/superpowers/plans/2026-10-28-ch-mcp-server.md`); `docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md` for Star Power and the menus; `docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md` for the virtual kit; and `docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md` for starting a song for a human player from the command line. The agent's page is `docs/agents/clone-hero-mcp.md`. The list of tools lives in one place, `build_server()` in `server.py`; read it there, with each tool's one-paragraph description.

The tools live in six modules, and `server.py` builds one object of each around a shared lock. The game-side tools are in `game_tools.py`: the launch, the stop, the auto-player and the locked key route. The Frida tools, the menu read included, are in `frida_tools.py`. `sp_tools.py` holds the Star Power watcher; it reads the game through `tools/ch_probe` and never reads memory itself. `kit_tools.py` holds the virtual kit: its setup, presses, status, watch and teardown, and the press it hands to the auto-player. `menu_tools.py` is the menu driver behind `ch_open_song`, and it presses through the kit. `method_watch.py` runs the method watch. Each one calls the module that already owns its work.

Three more modules hold what the tools lean on. `game_files.py` owns the game's paths, the user files a run backs up, the registry key and the one `settings.ini` line a run changes. `bot_run.py` owns the launch, which asks Windows for a minimized window that does not take focus. It also owns the human launch, `launch_human`: start the game paused, attach the launch agent by process id, arm it, resume the game, and wait for the agent to say the patch is made, each wait with its cap. `foreground.py` watches the front window from `ch_start` to `ch_stop` and counts each time the game's window came to the front.

Six data files sit beside the code, each with one owner. `launch_patch.json` is the one-byte patch the human launch makes in memory: the module, the loader's address and its first twelve bytes, the patch site, the bytes it must hold and the bytes written there, and two switches (`launch`, how the game is started paused, and `defer`, whether the patch waits a step) that the direct-start plan's live steps change between runs. `bot_run.py` reads it and sends it whole in the arm message, `agent/launch.ts` writes nothing that is not in it, and `tests/test_launch_patch.py` checks it against the facts table and the loader's listing in `docs/clone-hero/code/6142/`. `menu_steps.json` is the kit steps `ch_open_song` presses; `menu_tools.py` reads it and checks it with `load_steps`. It is kept for a day the direct route breaks (the direct-start plan's question 4), so no live step corrects it now. `menu_fields.json` names every field `ch_menu` reads on the player screen (its page, cursor, rows, controller type, instrument, difficulty and a few flags), each with its dump line, how it is read, and the enum words where it has them; `agent/menu.ts` reads them and `frida_tools.py` prints them, with the highlighted row worked out there. The build bundles that file into `state.js`, so a change to it needs `build:state` again. `kit_methods.json` lists every game method the kit agent may call, with the dump line it was read from; `agent/kit.ts` holds the same names in its own list, and `tests/test_frida_attach.py` checks the two agree. `start_candidates.json` is the list of game methods the method watch hooks; `method_watch.py` reads it and `agent/menuhooks.ts` hooks what it names. The method watch is kept for the same day. `start_method.json` and `ch_start_song` (task T8 of the Star Power and song start plan) were never built, and that plan now strikes T8 in favour of the direct route.

It is registered as `ch` in `.mcp.json` at the repo root, which runs `py tools/ch_mcp/server.py`. Claude Code asks the user to approve it the first time. The entry has no `-I`, unlike the Hydra helper's, because the frida module lives in the user's site-packages and `-I` skips them. JSON has no comments, so this README is where that reason is written down.

The server runs the compiled Frida agents (`agent/state.js`, `agent/trace.js`, `agent/menuhooks.js`, `agent/kit.js` and `agent/launch.js`) and never builds them. Build them from their TypeScript in `tools/ch_mcp/agent` with `npm run build:state`, `npm run build:trace`, `npm run build:menuhooks`, `npm run build:kit` and `npm run build:launch`. The menu read in `agent/menu.ts` is part of `state.js`, so `build:state` builds it too. `kit.ts` is its own agent, loaded beside the state agent by `ch_kit_setup` and built by `build:kit`. `launch.ts` is the launch agent `ch_start(human=True)` loads into the paused game, built by `build:launch`. It is the glue between Frida and `agent/launch_core.ts`, which holds the patch decision (`applyPatch`) with no Frida in it, so `tests/test_launch_core.py` runs it in plain Node. Git ignores the compiled files and `node_modules`. A tool that needs an agent that is not built says which command to run.

The server takes a backup of the user's game files before it touches the game, and puts them back when it stops. `ch_start` backs up before it launches. The first tool that touches a game the server did not start takes the backup then. The backup also exports the registry key the game's input library, Rewired, saves its bindings under, as `rewired.reg` beside the files. `ch_stop` restores the files, one line per file, then the key, with one line that says it was unchanged or restored. When the server exits, it closes a game it started and then restores; a game it did not start stays open and its files are restored. The backups, the stats copies, the Frida logs, the kit logs, the launch agent's logs, the Star Power CSVs and the method-watch logs go in one session folder per run under the temp folder; `SCRATCH_PREFIX` in `game_tools.py` starts its name. The server never deletes that folder, because it holds the backups.

Keep a human run's stats before you stop. `ch_wait_stats` copies the game's new `scorestats.json` only for a song `ch_start` was given a folder for, which the bot plays from the command line. When a person plays the song instead, copy `Documents\Clone Hero\scorestats.json` yourself before `ch_stop`, because `ch_stop` puts the old file back and the new one is lost.

A read is fast. Three `ch_state` calls sent back to back read the song clock 0.087 s and 0.072 s apart, so one read, round trip included, takes under 0.1 s, far inside the 5 s cap, `READ_CAP_S` in `frida_tools.py` (one burst of three on one song, 2026-10-29; `docs/clone-hero/07-internals.md` holds that live result).

Clone Hero's end-user licence forbids reverse-engineering the game. Its License Restrictions (https://clonehero.net/eula) say you agree not to "disassemble, decrypt, reverse compile or reverse engineer any part of the Application". The user chose on 2026-10-28 to build these tools anyway; `docs/research/2026-10-28-clone-hero-mcp-feasibility.md` (its EULA section, line 115) records that choice and what the tools touch. Check any claim about the licence against that page, never against a summary.

While the kit is set up, the user's own keyboard and mouse reach no player in the game. `ch_kit_setup` takes them away from every Rewired player and switches Rewired's keyboard device and mouse device off for the run, as the user ruled on 2026-10-30. If either does not read back as off, setup refuses and takes the kit away again. `ch_kit_teardown`, which `ch_stop` also runs, puts each device back to what setup recorded and checks it. `kit_tools.py` and `agent/kit.ts` own how this is done and checked.

The tests run with `py -m pytest tools/ch_mcp/tests -q`. They use fakes for the game, Frida, the kit agent, the registry and the keys, so they never start the game.
