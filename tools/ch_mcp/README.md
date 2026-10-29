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

Clone Hero 1.0 is a Mono build: its game code is plain .NET assemblies in `Clone Hero_Data\Managed\`, so there is no dump to make. `py tools/ch_mcp/mono_facts.py --game "C:\Clone Hero\1.0 old\Clone Hero" --version v1.0.0.4045-master --out "C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\4045"` reads the assemblies listed in `GAME_ASSEMBLIES` through `mono_reader.ps1` (PowerShell 7's own metadata reader, nothing installed) and writes the same `facts.json` shape. Mono metadata holds no addresses, so every field `offset` and method `rva` is null, and `signatures` has no `drums_engine`, because that entry is found by an address. `typedef_index` is the type's row number in its own assembly's TypeDef table; rows restart in each assembly, so two classes can share one. `dump_file` lists the assemblies read. The version string is the only one of its shape in `Clone Hero_Data\globalgamemanagers`; the exe's version info gives only the Unity version.

## What facts.json holds

`facts.json` is one JSON object with four keys, written as UTF-8 with the scrambled names kept as they are (not escaped), so read it with `encoding="utf-8"`. Any later reader of a game build, such as one for a Mono build, writes the same shape.

`game_version` is the version string the caller passed. `dump_file` is the name of the file the facts came from.

`classes` lists every class, struct, interface and enum, in the order the dump prints them. Each one has these keys:

- `name`: the name as the dump prints it. A nested class carries its outer class's name in front, joined by a dot.
- `namespace`: the namespace, or an empty string when it has none.
- `kind`: `class`, `struct`, `interface` or `enum`.
- `typedef_index`: the type's index in the game's metadata.
- `readable`: true when the name is plain ASCII, false when the obfuscator scrambled it.
- `base`: the base class as the dump writes it, generic arguments included, or null when there is none. The dump lists the base and the interfaces together with no marker. So the first listed type counts as the base unless every type of that name in the dump is an interface.
- `interfaces`: the other listed types.
- `fields`: one entry per field that has an offset. Constants have none and are left out.
- `methods`: one entry per method.

A field has a `name`, a `type`, an `offset` and a `static` flag. The offset is a number counted in bytes. For an instance field it is the distance from the start of the object; for a static field it is the distance into the class's static storage. A reader whose source gives no offsets writes null. The reader of an Il2CppDumper dump always writes a number.

A method has a `name`, a `return` type, a `params` list with each parameter's type (names and default values left out), a `static` flag and an `rva`. The RVA is the method's address counted from the start of `GameAssembly.dll`, as a number. It is -1 when the dump says the method has no compiled code of its own, as with abstract methods and open generic methods. A reader whose source gives no addresses writes null. The reader of an Il2CppDumper dump always writes a number. The per-instantiation addresses the dump lists under a generic method are not recorded.

`signatures` holds the short records the feasibility tests look up: `GameManager`, `ScoreManager` and `DrumsPlayer` by name, and `drums_engine` found by its constructor's address. Each has the class's `name`, `typedef_index` and `base`, its `field_count` and `method_count`, and `readable_fields`, which lists the fields whose names are readable in the same shape as above. `drums_engine` also has `ctor_rva`, the address it was found by. A name that matches no class or more than one stops the script with `class <name> matched N classes`.
