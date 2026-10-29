# tools/ch_mcp

This folder holds the code for the Clone Hero MCP feasibility tests. Those tests check whether an MCP server (a tool server an agent can call) can be built on top of a running Clone Hero. The plan `docs/superpowers/plans/2026-10-28-ch-mcp-feasibility-tests.md` is the spec for everything here.

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

## The server

`server.py` is an MCP server for Clone Hero: a helper program Claude Code starts, which gives an agent tools to start the game, read it and press its keys. The plan is `docs/superpowers/plans/2026-10-28-ch-mcp-server.md`, and the agent's page is `docs/agents/clone-hero-mcp.md`. The list of tools lives in one place, `build_server()` in `server.py`; read it there, with each tool's one-paragraph description. The game-side tools are in `game_tools.py` and the Frida tools in `frida_tools.py`. Each one calls the module that already owns its work.

It is registered as `ch` in `.mcp.json` at the repo root, which runs `py tools/ch_mcp/server.py`. Claude Code asks the user to approve it the first time. The entry has no `-I`, unlike the Hydra helper's, because the frida module lives in the user's site-packages and `-I` skips them. JSON has no comments, so this README is where that reason is written down.

The server runs the compiled Frida agents (`agent/state.js` and `agent/trace.js`) and never builds them. Build them from their TypeScript in `tools/ch_mcp/agent` with `npm run build:state` and `npm run build:trace`. Git ignores the compiled files and `node_modules`. A tool that needs an agent that is not built says which command to run.

The server takes a backup of the user's game files before it touches the game, and puts them back when it stops. `ch_start` backs up before it launches. The first tool that touches a game the server did not start takes the backup then. `ch_stop` restores the files, one line per file. When the server exits, it closes a game it started and then restores; a game it did not start stays open and its files are restored. The backups, the stats copies and the Frida logs go in one session folder per run under the temp folder; `SCRATCH_PREFIX` in `game_tools.py` starts its name. The server never deletes that folder, because it holds the backups.

The tests run with `py -m pytest tools/ch_mcp/tests -q`. They use fakes for the game, Frida and the keys, so they never start the game.
