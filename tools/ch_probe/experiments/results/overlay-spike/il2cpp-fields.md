# Il2Cpp dump: the engine's fields and where the chart checksum lives (2026-10-28)

Task I of `docs/superpowers/plans/2026-10-28-overlay-spike.md`. This file holds the class and field lines that the spike uses, copied from a fresh Il2CppDumper run over the installed game. Nothing here was read from a running game. Every line inside a code block is copied from `dump.cs` as it stands, tab indents and all. A block shows only the lines the spike uses, in dump order, so lines between them are left out; the complete lists are the two engine classes under "The base engine class's full field list" and "The drums engine class's own fields". Where this file says "presumably" or "the dump does not say", the dump does not show it and it is not checked. Clone Hero's names are obfuscated, so most class and field names are strings of modifier letters (`ʿʶʴˀ...`). This file names each class by its `TypeDefIndex`, the number the dump prints after each class line. The numbers hold for this game build only; an update can renumber them, so search by the class line, not by the number alone.

## How the dump was made

Il2CppDumper v6.7.46 ran offline on the two game files (the first start failed at once, as item 1 below says, and the second made the dump): `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll` and `...\Clone Hero_Data\il2cpp_data\Metadata\global-metadata.dat`. It ran from its own folder with absolute paths, as the plan says. Output went to a new empty folder in the session scratch area, `scratchpad\il2cpp-dump`, and is not committed.

Two things needed handling, and neither changed any file:

1. The tool is built for .NET 7, and only .NET 8 and 10 are installed. The first run stopped at once with "You must install or update .NET to run this application". The second run set `DOTNET_ROLL_FORWARD=Major` for that one process. That is .NET's own setting for running an app on a newer major runtime, so it used .NET 8.
2. The tool's `config.json` has `"RequireAnyKey": true`, so it waits for a key press at the end. Standard input was redirected from a file holding one empty line. After it printed `Press any key to exit...`, `Console.ReadKey` threw "Cannot read keys when ... console input has been redirected", so the process exited with an error code. By then every output was written. The console's earlier lines were `Metadata Version: 31`, `Il2Cpp Version: 31`, `Dumping... Done!`, `Generate struct... Done!` and `Generate dummy dll... Done!`.

The run took 7.1 s. It wrote `dump.cs` (36,453,454 bytes), `il2cpp.h`, `script.json`, `stringliteral.json` and `DummyDll`.

The dump's header lists the game's code images. The two that matter here are `// Image 0: CloneHero.dll - 0` (the game and its menus, type indexes 0 to 1450) and `// Image 7: StrikeCore.dll - 8242` (the gameplay engine, type indexes 8242 to 8690).

## The engine class

The fields at `0x68`, `0x1B0`, `0x42`, `0x48`, `0x120` and `0x130` all belong to one class: the abstract base engine class, TypeDefIndex 8288, in `StrikeCore.dll`. Its class line is:

```
public abstract class ʿʶʴˀʴʾʵʵʶʳʲ // TypeDefIndex: 8288
```

The grep looked for a class with a field line ending in each of `// 0x68`, `// 0x1B0`, `// 0x42`, `// 0x48`, `// 0x120` and `// 0x130` (instance fields only), over the whole dump. In the game's own images (type indexes 0 to 1450 and 8242 to 8690) only class 8288 has all six. Six structs of the Rewired input library (type indexes 5060, 5063, 5072, 5164, 5169 and 5175) also have fields at all six offsets, so offsets alone would not settle it. The match rests on the method addresses as well. `docs/audit/ch-evidence.md` answers 37 and 222 name methods by address, and the dump prints each method's RVA (its address inside `GameAssembly.dll`). Every one of those addresses is a method of class 8288. ActivateStarPower is at 0x20F5640 (vtable slot 14). The per-update SP routine is at 0x20F6800 (slot 12). The end-time function is at 0x20F37E0. The score adders are at 0x20F5F70, 0x20F6A50 and 0x20F6EB0:

```
	// RVA: 0x20F5640 Offset: 0x20F4A40 VA: 0x1820F5640 Slot: 14
	public virtual void ʹʼʲʷʽʲʲʲʲʾʾ() { }
	// RVA: 0x20F6800 Offset: 0x20F5C00 VA: 0x1820F6800 Slot: 12
	protected virtual void ʾˀʷʶʲʳʸʽʻʵʳ() { }
	// RVA: 0x20F37E0 Offset: 0x20F2BE0 VA: 0x1820F37E0
	private double ʳʼʽʽʸʽʺʲʼʹʻ(long ʶʲʸʴʷʷʲʸˁʵʿ, double ʵʻʾʷʶʺʲʺʴʾʹ) { }
	// RVA: 0x20F5F70 Offset: 0x20F5370 VA: 0x1820F5F70
	protected void ʻʴʵʹʷʵˁˀʶˁʽ(int ʿʲʴʸʼˁʺʳʻʲʽ, ʽʿʸʸʾʶʶʾʶʹʲ ˀʹˁʸʷʽʲˀʽʾʹ) { }
	// RVA: 0x20F6A50 Offset: 0x20F5E50 VA: 0x1820F6A50 Slot: 8
	protected virtual void ʿʷʲʿʺʸʷʹʹʸʹ(double ʷʶʲʺʴʾʿʺˁʿʲ, bool ʶʽʴʼʿʿʻʼˀˀʸ = True) { }
	// RVA: 0x20F6EB0 Offset: 0x20F62B0 VA: 0x1820F6EB0 Slot: 6
	protected virtual void ʿʾʹʲˀʶʷʵʶʶˀ(ʽʿʸʸʾʶʶʾʶʹʲ ʼʽʶʽʴʳˀˁˁʼʸ, int ʷʾʲʶʼʿʴʶʲʸʳ = 50) { }
```

The end-time function takes a `long` and a `double` and returns a `double`. That fits answer 37's "end time from (amount, activation time)": a whole-number tick amount and a `double` activation time in, a `double` end time out. The dump gives no unit for the two times (the plan's `sp_end_s` column assumes seconds; that is checked live, not here).

### The live drums object is a subclass

Class 8288 is abstract, so the object the game creates is a subclass. One generic class sits in between, and the drums engine is the sealed class 8255 below it:

```
public abstract class ʿʺʸʺʼʴʸʼʻʳʼ<TInputs> : ʿʶʴˀʴʾʵʵʶʳʲ // TypeDefIndex: 8253
public sealed class ʺʺʸʶʿʵˀʸʲˀʲ : ʿʺʸʺʼʴʸʼʻʳʼ<ʾʸʸʻʷʸˁʾˁʽʸ> // TypeDefIndex: 8255
```

Class 8255 is the drums engine because the drum disassembly (`docs/audit/drum-lanes/FINDINGS-play-time.md`) names its press handler (0x20DEAA0), drum hit routine (0x20DE770, slot 11) and score function (0x20DEEC0, slot 6), and `tools/ch_probe/constants.py` names the drums engine constructor (0x20DF680, `RVA_DRUMS_ENGINE_CTOR`). All four are methods of 8255 in the dump:

```
	// RVA: 0x20DEEC0 Offset: 0x20DE2C0 VA: 0x1820DEEC0 Slot: 6
	// RVA: 0x20DEAA0 Offset: 0x20DDEA0 VA: 0x1820DEAA0
	// RVA: 0x20DF680 Offset: 0x20DEA80 VA: 0x1820DF680
	// RVA: 0x20DE770 Offset: 0x20DDB70 VA: 0x1820DE770 Slot: 11
```

A subclass keeps its base class's field offsets, so the base-class offsets below hold on the drums object unchanged. The only other concrete engines are classes 8289 and 8655. Both sit under the generic class 8544, and neither prints any field of its own. Class 8544 does (40 fields, all printed at `// 0x0`), so their real offsets are not in the dump. That does not touch the base-class offsets.

The generic class 8253 prints its own four fields at `// 0x0`, because their offsets depend on the type argument. The base class's last field is an `int` at `0x294`, and class 8255's first field is at `0x2C0`. The 40 bytes between them presumably hold 8253's four fields (the dump does not give their offsets). That gap does not touch any offset this spike reads.

### The six Star Power offsets

All six offsets agree with the plan and with answer 37. Here is each field line, with its type and width:

```
	public long ʵʼʿˁʷʹˀʵˁʼˀ; // 0x68
	public readonly long ˀʾʾʻʴʵʼʼʼʾʴ; // 0x1B0
	public bool ˁʸʴʵʳʾʽʿʹʷʿ; // 0x42
	protected int ʽʷʿʴˁˁʺʻʽʴʷ; // 0x48
	public double ʴʾˁʾʼʹʲʳʾʿʺ; // 0x120
	protected bool ʻʹʸʿʴʻʸʳʴʽˀ; // 0x130
```

In plain words:

- **SP amount, `+0x68`:** a `long`, 8 bytes, signed. This fits answer 37's "whole number of chart ticks".
- **SP maximum, `+0x1B0`:** a `readonly long`, 8 bytes. `readonly` means it is set when the engine is built and never changes after, which fits "the maximum".
- **SP active flag, `+0x42`:** a `bool`, 1 byte.
- **Multiplier, `+0x48`:** an `int`, 4 bytes. This fits answer 222's 32-bit `imul eax,[rsi+0x48]`.
- **SP end time, `+0x120`:** a `double`, 8 bytes, the same type the end-time function returns.
- **Overflow flag, `+0x130`:** a `bool`, 1 byte.

Task A records several widths for each field. The dump settles which width is right, and two of the wider reads take in bytes that belong to other fields or to no field. A 4-byte read at `+0x42` covers four separate one-byte `bool` fields, at `0x42`, `0x43`, `0x44` and `0x45`, so `sp_active_u32` mixes three other flags into the value. The byte read is the exact one. For `+0x130`, the next field starts at `0x138`. A 4-byte read there takes in bytes `0x131` to `0x133`, which belong to no field in the dump. For `+0x68` and `+0x1B0`, the 8-byte reads are the exact width; a 4-byte read gives the low half. For `+0x48`, the 4-byte read is exact; a byte read gives the low byte.

### Other engine fields the probe already reads

`engine.py` and `engine_finder.py` read other offsets on the same object. The dump agrees with each:

```
	protected double ʾʻʳʷʶʵʵʸʿʼʳ; // 0x20
	protected double ʲʾʾʸʶʾʺʺʵʳʼ; // 0x30
	protected double ʿʾʸˁʾˁˀˀʻʻʼ; // 0x38
	public int ʹʿʷʾʹʵʴʹʶʹʶ; // 0x8C
	public int ʼʼˁʴʿʸʿʲʺʳʵ; // 0x94
	protected ʴʻʽʻʲʾʸʵʺʶʲ ʸʶʸʵʵʳʾʺʸʷʼ; // 0x100
	public readonly ʷʴʼʴʴʷˁʵʺʴˁ ʿʹʸʷʸʽʻʿˀʳʴ; // 0x198
```

The window values at `0x20`, `0x30` and `0x38` are doubles. The note count at `0x8C` and the score at `0x94` are 4-byte ints. The field at `0x100` is a 16-byte struct (class 8496) whose first member is a double, so `engine.py`'s double clock at `+0x100` is that first member:

```
public struct ʴʻʽʻʲʾʸʵʺʶʲ // TypeDefIndex: 8496
	public readonly double ʸʲʿʽʴʴʹʴʶʵʼ; // 0x0
	public readonly long ʵʿʵʲʽʽʽʵʸˀʾ; // 0x8
```

The field at `0x198`, which `engine.py` calls "flags", has the modifiers flag enum as its type (class 8576, shown under "Song, difficulty, modifiers, combo and speed"). It is 4 bytes wide and `readonly`, which fits the modifiers the song was started with. The dump does not show when the field is set or whether it changes after.

The hit-time candidate at `+0x2E0` is a field of the drums class 8255, not of the base class. It is a double:

```
	private double ʷʷʽʹʷʹʼˁʻʴˁ; // 0x2E0
```

### The base engine class's full field list

Copied as the dump prints it, for Phase 1 and for the write-up:

```
public abstract class ʿʶʴˀʴʾʵʵʶʳʲ // TypeDefIndex: 8288
{
	// Fields
	protected double ʽʷʻʷʶʸʹʻʵʼʹ; // 0x10
	protected double ʲˁʺʴʸʲʵʹʻʸʻ; // 0x18
	protected double ʾʻʳʷʶʵʵʸʿʼʳ; // 0x20
	protected double ʴʴˀʼʷʵʶʶʲʲʷ; // 0x28
	protected double ʲʾʾʸʶʾʺʺʵʳʼ; // 0x30
	protected double ʿʾʸˁʾˁˀˀʻʻʼ; // 0x38
	protected bool ˀʲʲʺʵʹʽʶʽʶʹ; // 0x40
	protected bool ʲʳʶʹʶʸʳʾʾʳʶ; // 0x41
	public bool ˁʸʴʵʳʾʽʿʹʷʿ; // 0x42
	protected bool ʴʾʲʻʵʵʳʴʸʿʽ; // 0x43
	public bool ʷʽˁʳʽʵʶʸʸʲʿ; // 0x44
	public bool ˀʴʵʸʶʸʲʾʶʵʲ; // 0x45
	public bool ʿʷʶʶʾʾˀˀʻʲˁ; // 0x46
	protected int ʽʷʿʴˁˁʺʻʽʴʷ; // 0x48
	public int ʾʻʷʶʹʻʳʸʲʶʹ; // 0x4C
	protected int ʺʸʶʿʷʿʻʼʹʵʹ; // 0x50
	protected int ʺʶʾʴʼʿʳʼʴˁˀ; // 0x54
	public ʽʿʸʸʾʶʶʾʶʹʲ[] ʸʵʸʿʾʽʻʽʻʼʶ; // 0x58
	private long ʻʵˁʽʴʻʸʻʼʳʺ; // 0x60
	public long ʵʼʿˁʷʹˀʵˁʼˀ; // 0x68
	protected float ʺʼʻʳʼʳʲˀʼʸʺ; // 0x70
	protected float ʹʳˁʶʼʴˁʲʳʼˁ; // 0x74
	protected float ʽʲʹʸˀʴʿʿʷʿʳ; // 0x78
	protected float ʵʳʾʷʺʿʽʼʹˁʸ; // 0x7C
	public long ʲʺʷʽʼʻʿʴʹʻʲ; // 0x80
	public int ʲʺʺʿʳʲʾʲʽʼʿ; // 0x88
	public int ʹʿʷʾʹʵʴʹʶʹʶ; // 0x8C
	public int ʺʾˁʼʽˁʹˀʷʼʼ; // 0x90
	public int ʼʼˁʴʿʸʿʲʺʳʵ; // 0x94
	public int ʷˀʾʻʲʲʳʼʽʴʾ; // 0x98
	public int ʳʵˁʷˁˁʺʲʵʽʷ; // 0x9C
	public int ˁʴʻʳʳʻˀʴˁʻʾ; // 0xA0
	public int ʻˀʸʳʸʲʶʵʷʺʷ; // 0xA4
	public bool ʸʽʵʽʴʾʹʸˀʴʿ; // 0xA8
	public int ʴʳˀʵʿʿˁʿʵʵʳ; // 0xAC
	public int ʷʸʽˀʾʻʹʷʺˀʽ; // 0xB0
	public int ʸʾʵʷʿʹˁʲʸʶʿ; // 0xB4
	public double ʻʽʼʿʲʺʲʲˀʳʲ; // 0xB8
	public int ʽʼʷʶˁʶʿʵʺʿˁ; // 0xC0
	public int ʲʲˀʳʵʽʼʵʳʷʴ; // 0xC4
	public int ʹʷʶʾʼʲˁʿʶʺʽ; // 0xC8
	public int ʶʴʻʼʺʹʸʷʵˀˁ; // 0xCC
	public int ʽʹʳʿʹʾʼʴʹʻʾ; // 0xD0
	public int ʷˀʳʲʶʷʴʼʶʺˀ; // 0xD4
	public int ʲˀʵʻʿʴʹʵʿʼʻ; // 0xD8
	public int ʴˁʿʷʹˀˀʲˀʹˀ; // 0xDC
	public int ʳʹʸʿˁʹʲʼʵʻʻ; // 0xE0
	public int ʹʲʿʽʳʷʿʺʽʹʵ; // 0xE4
	public int ʳʾʿˀʵʹʸʴʽʸʲ; // 0xE8
	public int ʺʼʷʷʻʿʺˁʻʾˁ; // 0xEC
	public bool ʳˁʽʿʴʲʼʿʼˁʼ; // 0xF0
	public bool ʸʳʵʵˁʴʺʿʺʼʳ; // 0xF1
	public int[] ʴʵʻʳʺʾʾʴʽʽʸ; // 0xF8
	protected ʴʻʽʻʲʾʸʵʺʶʲ ʸʶʸʵʵʳʾʺʸʷʼ; // 0x100
	protected double ʽˀʽˁˀʿʷʿʲʴʹ; // 0x110
	protected long ʸʼʽʷʽʻʸʶʴʶʵ; // 0x118
	public double ʴʾˁʾʼʹʲʳʾʿʺ; // 0x120
	public double ʵˀʻʴʼʲʷʷˁʷʺ; // 0x128
	protected bool ʻʹʸʿʴʻʸʳʴʽˀ; // 0x130
	protected long ʸʷˁʹʶʳʿʳʹʷʵ; // 0x138
	protected long ʾʽˀʻʾʿʷˁʽʹʶ; // 0x140
	protected ʲʲʺʴʺʼʷʸʾʷʹ ʺʹʳʲʲʳʲʾʺʽˀ; // 0x148
	protected ushort ʻʴʿʸʲʳʲʷʴˀʴ; // 0x150
	protected ushort ʴʼˀʴʳʺʾˀˁʳʶ; // 0x152
	public bool ˀʽʾʴˀʾʵˁʼʿʶ; // 0x154
	public ʸʸʽˀʹʺʹʻʼʴʴ ˀʹʳʿʵʽʴʽʶʷˁ; // 0x158
	public int ʶʵʹʿʷʶʿʲʾʼʶ; // 0x160
	public int ʶʼʼʸʶʾˁˁʼʸʼ; // 0x164
	public int ʹʲˁˁʲʺʵʽʿʳʽ; // 0x168
	public int ʷʴʽʹʵʺˀˀʸʳʹ; // 0x16C
	public double ʸʻʹʶʷʹʵʹˀʾʾ; // 0x170
	protected int ʶʴʾʹʸʵʲʵʲʳˁ; // 0x178
	protected List<double> ˀʸʵʼʾʶʾʲʸʴʴ; // 0x180
	public List<double> ʵʲʷʸʹʶʴʸʸʷˀ; // 0x188
	public readonly ʳʴʾʷʸʼʷʹʴˁˀ ʵʸʹʽʺʾʻʹʲˁˀ; // 0x190
	public readonly bool ʿʻʲˀʸʻʹʵʼʺʿ; // 0x194
	public readonly bool ˀʾʽʳˀʹʵʿʼʼʸ; // 0x195
	public readonly bool ˀʺʷʿʾʴʽʼˁˀʷ; // 0x196
	public readonly ʷʴʼʴʴʷˁʵʺʴˁ ʿʹʸʷʸʽʻʿˀʳʴ; // 0x198
	private readonly bool ʲʹʲʴʲʺʲʺˁʺʵ; // 0x19C
	private readonly ʳʴʹʻʾʲʹʹʽʽʷ ʿʸʳˀʶʽʼʴʷʲʳ; // 0x1A0
	protected readonly int ˁʷʴʲʻʿʸʴʳʽʿ; // 0x1A8
	public readonly long ˀʾʾʻʴʵʼʼʼʾʴ; // 0x1B0
	public readonly long ʵˀʵʶˀʿʾʿʽʶʸ; // 0x1B8
	public readonly long ˀʴʴʲʲʶʹˁʶʴʼ; // 0x1C0
	public readonly long ʵʳˀʷʷʹʷʹʳʻʳ; // 0x1C8
	public readonly int ʸʳʳʲʷʲˀʶʺʾʼ; // 0x1D0
	public readonly ʹʷʽʶʶʸʻʿˁʾʵ ʵʹˁˀʷʽʾʲˁʲʶ; // 0x1D8
	public readonly ʲʽʶʳʺʹʳˀʾʴʹ.ʸʲʻʼʴʵʺʲʺˀʸ ʻʵˁʼˁʺʻʽʻʽʲ; // 0x1E0
	protected readonly ʿʻʾʿʳʷʿʷʼʽʲ.ʿʳʲʴʾʷʸʿʷʽʷ ʾʼʶʴʺʶˁʸʼʷʴ; // 0x280
	public Action ʹʹʳʻʻˀʴʾʷʾʷ; // 0x288
	public bool ʴʼʷʶˀʼʴʷʷʷʷ; // 0x290
	private int ʿʽʷʺʸʶʶʹʽʲʴ; // 0x294
```

### The drums engine class's own fields

```
public sealed class ʺʺʸʶʿʵˀʸʲˀʲ : ʿʺʸʺʼʴʸʼʻʳʼ<ʾʸʸʻʷʸˁʾˁʽʸ> // TypeDefIndex: 8255
{
	// Fields
	private const double ʵʴʺʹʶʹʸʷʷˁʹ = 0.25;
	private const double ʲˁʾʷʴʲˁʳʹʶʲ = 10;
	private const double ʵʶʿʺʼʹˀʵʴʶʲ = 0.075;
	private const double ʸʾʳʳˀʴʺʴʹʿʼ = 0.5;
	private const double ʹʺˀʺˀˁʲʹʿʿˀ = 0.75;
	private const uint ˁʳʲʵʽʳʿˁʹʵʲ = 2;
	private const uint ʷˀʷʻʾʿˀʳʶˀʵ = 4;
	public uint ˁʺʺʷʻʼʷʾʽʴʼ; // 0x2C0
	private ushort ʼʶʻʻʵʷʽˁˁˁʺ; // 0x2C4
	private ushort ʻʷʲʿʽʺˁʹʻʻʺ; // 0x2C6
	private bool ʸʾʸʲʽʷʽʽʵʵʳ; // 0x2C8
	private double ʺʲʷʻʵʸʶʼʽʺʼ; // 0x2D0
	private double ʽˁʳʳʲʸʼʶʾʺʸ; // 0x2D8
	private double ʷʷʽʹʷʹʼˁʻʴˁ; // 0x2E0
	private uint ʼˁʼʶʻʲʷʽʷʻʳ; // 0x2E8
	private uint ʻʺʳˁʷʿʾʶʿʸʲ; // 0x2EC
	private bool ʽˀʻʼʶʽʸˀʾʽʴ; // 0x2F0
	private bool ʺʲʽˀʹʲʷʷʺˁʾ; // 0x2F1
	private bool ʺʴʺʹʷʴʿʺˁʷʼ; // 0x2F2
	private bool ʿʷʺʶʼˁʵʺˀʿˁ; // 0x2F3
	private ushort ʿʼʴʿʾʼʼʴʴˁʷ; // 0x2F4
	public readonly bool ʲʲʻʷʿʵʴʷˁʷʿ; // 0x2F6
	private int ʶʵʼʸʽʲʼʵʸʶʺ; // 0x2F8
	private long ʺʾʾʿʷʹʳʺʺʽʿ; // 0x300
	private bool ʳʲʳˀʵʸʷʺʻʿʵ; // 0x308
	public ʾʿʴʹʻʹʸʳʼʲʵ ʻˀʺʽʽʶʼʼʷʷʽ; // 0x309
	private ʹʲʻʵʲʻˀʿʵʽˁ ʸʿʹʶʹʵʺʸʾʾʿ; // 0x318
```

The two `uint` counters at `0x2E8` and `0x2EC` are the ones the drum-lane findings say each lane-pad press zeroes with one 8-byte write.

## The chart checksum

### The strongest candidate: `SongEntry.checksum`, reached from `GlobalVariables.instance`

`SongEntry` (type 176, `CloneHero.dll`) is the game's record for one song in the library. Its class name and its checksum fields are not obfuscated:

```
public class SongEntry : ʴʲˁʹʸʹʷʶʽʶʷ // TypeDefIndex: 176
	public string folderPath; // 0x30
	public ˁʶʾʿʲʵʷʲʼʵʻ checksum; // 0x38
	private string checksumStringCached; // 0x48
	public string chartName; // 0x78
	public int songLength; // 0x80
	public string ChecksumString { get; }
```

The `checksum` field is a struct stored inline, 16 bytes from `0x38` to `0x47`; the next field starts at `0x48`. Its type is struct 8286 in `StrikeCore.dll`. That struct is sixteen single bytes and a constant 16:

```
public struct ˁʶʾʿʲʵʷʲʼʵʻ : IEquatable<ˁʶʾʿʲʵʷʲʼʵʻ>, IComparable<ˁʶʾʿʲʵʷʲʼʵʻ> // TypeDefIndex: 8286
	public const int ʵʹʿˀʼʸˀʷʽʺʿ = 16;
	private byte ʷʴʷʴʸʴˁʻʸˀʺ; // 0x0
	private byte ʷʲʸʺʴˀʶʵʵʿʻ; // 0xF
```

An MD5 is 16 bytes, so this fits the MD5 that `scorestats.json` records as `checksum`. The dump only shows the size. Whether these bytes equal that MD5 is the live check in run 3. `checksumStringCached` at `0x48` is a string pointer, presumably the hex text of the same value. It may be null until something asks for `ChecksumString`.

The song being played is presumably in `GlobalVariables.currentSongEntry`, going by its name; the dump does not show when the game sets it. `GlobalVariables` (type 1048) is a Unity object with a static `instance` field, so a reader may be able to follow pointers from a fixed address instead of searching memory:

```
public class GlobalVariables : MonoBehaviour // TypeDefIndex: 1048
	public ʴʲˁʹʸʹʷʶʽʶʷ selectedSong; // 0x20
	public SongEntry currentSongEntry; // 0x28
	public ʵʴʺˁʵʽʾʹʸʹˀ currentSongMetadata; // 0x30
	public ˁʻʽʷʽʻʾʵʷˀʶ[] playerList; // 0x38
	public int originalSongSpeed; // 0x5C
	public static GlobalVariables instance; // 0x8
```

So the chain is: GlobalVariables' static fields, then `+0x8` to `instance`, then `+0x28` to `currentSongEntry`, then `+0x38` to the 16 checksum bytes. The static `0x8` is an offset inside the class's static-fields block, not inside the object. The block is reached through the class's type-info pointer. `script.json` puts `GlobalVariables_TypeInfo` at address 63895696 (0x3CEF890) inside `GameAssembly.dll`. The dump does not say at which offset an Il2CppClass keeps its static-fields pointer for metadata version 31. That offset, whether `instance` holds the live object, and whether `currentSongEntry` holds the playing song and changes when a new song starts, are live checks. They are for the checksum work: Task C's finder (run 3 of the plan's sitting) reports where the hash really sits, and this route is the dump's candidate to compare with what it finds. `selectedSong` at `0x20` has a different, obfuscated type, the base class of `SongEntry`. `script.json` also lists `SongEntry_TypeInfo` at 63774712 (0x3CD1FF8).

### Other routes to the same `SongEntry` or engine

`GameManager` (type 449) also has a static field, at static offset `0x0`, of its own type. That field is presumably the running instance, but its name is obfuscated. `GameManager` holds `GlobalVariables` and the player objects, and `script.json` lists `GameManager_TypeInfo` at 63891280 (0x3CEE750):

```
public class GameManager : MonoBehaviour // TypeDefIndex: 449
	private static GameManager ʷʲʺʷʻʳʾʶˁˀʷ; // 0x0
	public GlobalVariables globalVariables; // 0x28
	public BasePlayer[] playerObjects; // 0x58
	public double songTime; // 0x70
	public bool isPaused; // 0x90
	public bool isSongOver; // 0x91
	public ScoreManager scoreManager; // 0xB8
	public bool isSongPlaying; // 0xC0
	internal ʿʶʴˀʴʾʵʵʶʳʲ[] ʽˁʶˀʽʷʽʳʺʴʾ; // 0x168
```

`BasePlayer` (type 901) holds the engine under a plain name:

```
public abstract class BasePlayer : MonoBehaviour, ʳʴʹʻʾʲʹʹʽʽʷ // TypeDefIndex: 901
	public GameManager gameManager; // 0x20
	public ComboColor comboCounter; // 0x38
	public ˁʻʽʷʽʻʾʵʷˀʶ player; // 0x88
	protected SPBar spBar; // 0x90
	public ʿʶʴˀʴʾʵʵʶʳʲ engine; // 0x120
	public ʲʽʶʳʺʹʳˀʾʴʹ.ʸʲʻʼʴʵʺʲʺˀʸ currentChartData; // 0x140
```

So a second pointer path to the engine object is GameManager's static instance, then `+0x58` (`playerObjects`, an array), then an element, then `+0x120`. Each step is a field line above; how to read a managed array's elements is not in the dump. Other classes that hold a `SongEntry` field are `SongSelect.currentSong` at `0x190` (the menu's selection), `BassAudioManager.menuSong` at `0x30`, `CliGameLauncher` at `0x40` and `PlayerSelectionMenu` at `0x238`.

### Every checksum-, hash- and MD5-named field in the game images

The grep was `(?i)(checksum|hash|md5)`, run over every class in `CloneHero.dll`, `StrikeCore.dll`, `acilAddOns.dll`, `CHNet.Core.dll` and `CHNet.Server.dll`. It also matched `HashSet<...>` collections, `GetHashCode()` overrides and `ComputeStringHash`. Those are .NET plumbing, not chart hashes, and are left out below. These are the rest, by class:

```
public class SongEntry : ʴʲˁʹʸʹʷʶʽʶʷ // TypeDefIndex: 176
	public ˁʶʾʿʲʵʷʲʼʵʻ checksum; // 0x38
	private string checksumStringCached; // 0x48
	public string ChecksumString { get; }
public class ʲʻʳʶʲʼʸʴʾʲʳ.ScoreData // TypeDefIndex: 297
	public string chartHash; // 0x18
	public string songHash; // 0x20
public class InGameLeaderboardManager : MonoBehaviour // TypeDefIndex: 197
	public string fakeHash; // 0x20
private sealed class InGameLeaderboardManager.ˁʲʹʽʹʴʾʻʷʽʴ // TypeDefIndex: 195
	public string trackHash; // 0x10
private sealed class InGameLeaderboardManager.ʵˀʹʹʾʹʸʹʵʹʵ : IEnumerator<object>, IEnumerator, IDisposable // TypeDefIndex: 196
	public string trackHash; // 0x20
private sealed class ʶʶʲʼʵˁʻʶʺʵʺ.ʾˀˀʲʶʺʿʷʴʸʸ<TInputs> // TypeDefIndex: 252
	public string songHash; // 0x0
private struct ʶʶʲʼʵˁʻʶʺʵʺ.ʻˀʾʲʼˀʽʶʺˁˀ<TInputs> : IAsyncStateMachine // TypeDefIndex: 265
	public string songHash; // 0x0
private struct ʲʻʳʶʲʼʸʴʾʲʳ.ʲʼʸʺˀʶʵʴʻʹʶ : IAsyncStateMachine // TypeDefIndex: 308
	public string chartHash; // 0x20
private struct ʲʻʳʶʲʼʸʴʾʲʳ.ʾʺʽʿʿʶʽʿʻʺʵ : IAsyncStateMachine // TypeDefIndex: 309
	public string trackHash; // 0x20
private struct GameManager.ʾʿʷʵʿʷʺˀʿʹˀ : IAsyncStateMachine // TypeDefIndex: 448
	private string <trackHash>5__3; // 0x30
private struct BasePlayer.ʵʷʸˀʻʾʾʹʸʲˁ<TInputs> : IAsyncStateMachine // TypeDefIndex: 995
	public string songHash; // 0x0
private sealed class ThumbnailCache.ʺʺʵʾˁʽʳʵˁʸʿ : IEnumerator<object>, IEnumerator, IDisposable // TypeDefIndex: 885
	private string <imageCacheHashPath>5__2; // 0x48
public interface ISongSessionService : IService<ISongSessionService>, IServiceMarker // TypeDefIndex: 1050
	public abstract UnaryResult<string> StartSongSession(string sessionToken, byte[] chartData, string songHash, int audioCalibration, int videoCalibration, string hardwareId);
public class ʵʹʿʳʷʻʴʽʺʿʶ // TypeDefIndex: 8516
	public Nullable<Hash> ʼʸʺʺʶˀʿʽʵʸʽ; // 0x30
private sealed class ˁʼˁʵˁˁʶˀʾʸʷ.ʽʴˁʺʶʼʵʽʻʸʽ // TypeDefIndex: 15013
	public ˀʽʵʶˀʵʶʷʹʴʸ hostChecksum; // 0x10
public enum ʶʻʳʽʹʳʻʴʻʸʸ // TypeDefIndex: 125
	public const ʶʻʳʽʹʳʻʴʻʸʸ OnlineHash = 7;
public enum ˀʷʵˀʹʾʴˀˀʿʿ // TypeDefIndex: 13480
	public const ˀʷʵˀʹʾʴˀˀʿʿ Checksum = 7;
	public const ˀʷʵˀʹʾʴˀˀʿʿ ClientHashes = 9;
```

The `songHash`, `chartHash` and `trackHash` strings sit in leaderboard and online-session code, and most are locals of async methods (the `IAsyncStateMachine` structs and `<...>5__3` fields). They live only while a leaderboard or session call runs, so none is a stable place to read the playing chart. `ScoreData` is the leaderboard score record; its `chartHash` and `songHash` are filled for a leaderboard request.

The `Hash` in class 8516 is Blake3's, a 32-byte hash, not an MD5:

```
// Namespace: Blake3
public struct Hash : IEquatable<Hash> // TypeDefIndex: 15773
	public const int Size = 32;
```

The grep also matched the methods that make or return a Blake3 `Hash`. These are those lines, with their classes:

```
public static class ʾˁʸˁˁʻʿˁˁʲʽ // TypeDefIndex: 8346
	public static Hash ʿʵʶʷʵʾʸʺʽʻʹ(ReadOnlySpan<byte> ʽʽʿʿʾʲʵʽʼʷˀ) { }
	public static string ʼʶʷʷʼʿʴʴʺʵʵ(Hash ʷˁʿʴʸʶʳʸʲʻʲ) { }
public class ˁʿʺʲʲʹˀʴʾʻʻ : IDisposable // TypeDefIndex: 8538
	public Hash ʻʾʳʳʻʷʺʶʲʵʵ() { }
	public Hash ʽʻʴʶʺˁʳʸʸʳʿ() { }
	public Hash ʷʻʿʽʲʾˀˀʸʹʾ() { }
public class ʷʿʽʽʵʻʶʹʼʼʺ : IDisposable // TypeDefIndex: 8661
	public Hash ʽʳʸʿʷʽʼˁʸʻʼ() { }
	public Hash ʼʿʼʺʹˁʲʾʷʿʶ() { }
```

Class 8346 is a static helper whose two methods turn bytes into a `Hash` and a `Hash` into a string. Class 8538 has methods that return a `Hash`, and `GameManager.song` at `0xA8` is of that class's type. So does class 8661, which is the type of a field inside the engine's chart-data struct at `+0x48` of that struct (class 8263; the same struct holds a class 8538 field at `+0x50`). That hints that the game also computes a Blake3 hash of the loaded chart, but the dump does not show what is hashed. Neither 8538 nor 8661 has a field of type `Hash`. The only field of that type in the game's images is the `Nullable<Hash>` at `0x30` of class 8516 above. Class 8516 also holds a class 8538 field at `0x10` and a class 8661 field at `0x18`, and three classes hold one of it: `CliGameLauncher` at `0x60`, the profile-settings class 451 at `0xE0` and class 8564 at `0x10`. Whether that hash is of the chart, and when it is filled in, are not shown by the dump; they are live checks.

Outside the game images, the same grep hit .NET's `MD5`, `HMACMD5` and `MD5CryptoServiceProvider` classes, the Mono TLS code, zstd's frame checksum fields, and TextMeshPro and XML hash codes. None of them holds game state.

## Song, difficulty, modifiers, combo and speed

These greps ran over the same game images with the patterns `(?i)(difficulty|modifier|songspeed|song_speed|playbackspeed|\bspeed\b|currentsong|currentchart|selectedsong|songentry|\bengine\b)`, `(?i)(combo|streak)`, `(?i)^\t.*speed\w*;` and `(?i)(sp_?bar|starpower|star_power)`. Reading these fields live is Phase 1.

**Current song.** `GlobalVariables.currentSongEntry` at `0x28`, as above.

**Difficulty.** The difficulty enum is class 8617, one signed byte:

```
public enum ˁʹʴʸʸʲʸˁʺʸʸ // TypeDefIndex: 8617
	public sbyte value__; // 0x0
	public const ˁʹʴʸʸʲʸˁʺʸʸ None = -1;
	public const ˁʹʴʸʸʲʸˁʺʸʸ Easy = 0;
	public const ˁʹʴʸʸʲʸˁʺʸʸ Medium = 1;
	public const ˁʹʴʸʸʲʸˁʺʸʸ Hard = 2;
	public const ˁʹʴʸʸʲʸˁʺʸʸ Expert = 3;
```

The engine holds it inside its chart-data struct at `+0x1E0` (type 8263). In that struct, the instrument is at `0x38` and the difficulty at `0x39`:

```
public struct ʲʽʶʳʺʹʳˀʾʴʹ.ʸʲʻʼʴʵʺʲʺˀʸ : IDisposable // TypeDefIndex: 8263
	public readonly ʼˁʿʷʶʸʴʼʹʾʶ ʹʼʺʽʷʹʹʾʶˁʹ; // 0x38
	public readonly ˁʹʴʸʸʲʸˁʺʸʸ ʿʸʽʺʾʽʴʻʼʲʾ; // 0x39
```

By the dump's layout that puts the instrument at engine `+0x218` and the difficulty at engine `+0x219`. That is arithmetic on the dump (0x1E0 + 0x38 and + 0x39), not a live read. The same struct is `BasePlayer.currentChartData` at `0x140`. The instrument enum is class 8326 (`Drums = 6`, `ProDrums = 9`, one signed byte).

The player's chosen difficulty, instrument and modifiers also sit in class 8526, the base of the profile-settings class 451. Class 451 is the type of the first field of the player class 1435, and the player class is the type of `GlobalVariables.playerList` and `BasePlayer.player`:

```
public class ʾˀʷˁʻˀʴʳʽʻʽ // TypeDefIndex: 8526
	public ʼˁʿʷʶʸʴʼʹʾʶ ʷʻʺʼʴʶʴʳʽʺʿ; // 0x10
	public ˁʹʴʸʸʲʸˁʺʸʸ ʵʽˀˁʿʿʷʿʶˁʺ; // 0x11
	public ʷʴʼʴʴʷˁʵʺʴˁ ʾʼʶˁʽʶʸʺʸʷʼ; // 0x14
public class ʹʺʿʻʲʹʻʹʹʵʸ : ʾˀʷˁʻˀʴʳʽʻʽ // TypeDefIndex: 451
public class ˁʻʽʷʽʻʾʵʷˀʶ // TypeDefIndex: 1435
	public ʹʺʿʻʲʹʻʹʹʵʸ ʲʼʽʺʿʹʽʻʵʺʶ; // 0x10
```

**Modifiers.** The modifiers are a flag enum, class 8576, 4 bytes. It is the type of the engine's `readonly` field at `+0x198` and of class 8526's field at `0x14`. Selected lines (`None`, `Shuffle` and the drum flags):

```
public enum ʷʴʼʴʴʷˁʵʺʴˁ // TypeDefIndex: 8576
	public int value__; // 0x0
	public const ʷʴʼʴʴʷˁʵʺʴˁ None = 1;
	public const ʷʴʼʴʴʷˁʵʺʴˁ Shuffle = 64;
	public const ʷʴʼʴʴʷˁʵʺʴˁ DoubleKick = 2;
	public const ʷʴʼʴʴʷˁʵʺʴˁ NoKick = 4;
	public const ʷʴʼʴʴʷˁʵʺʴˁ KickOnly = 8;
```

Some values repeat (for example `AllStrums = 2` and `DoubleKick = 2`), so a flag's meaning depends on the instrument. The full list is in the dump at the enum's class line.

**Song speed.** No field named like a live song speed turned up in the game images. The closest are `GlobalVariables.originalSongSpeed` at `0x5C` (an `int`), `GameManager.practiceSpeed` at `0xD8` (a `float`, practice mode) and `ScoreData.speed` at `0xA8` (the leaderboard record):

```
	public int originalSongSpeed; // 0x5C
	public float practiceSpeed; // 0xD8
	public int speed; // 0xA8
```

The other speed-named fields in the game images are leaderboard filters and locals (`speedMin`, `speedMax`, `minSpeed`, `maxSpeed` and a `speed` int in async leaderboard code), UI scrolling and animation, the Pong mini-game, `SongSelect.speedText` (a label), and `noteSpeed` (a `float`) in the generic note-renderer class `BaseNoteRenderer` at an offset the dump prints as `0x0` because the class is generic. That last one is the highway scroll speed by its name, not a song speed, and the dump does not show how it is set. The profile settings in class 8526 hold many fields of the obfuscated types `ʽʾʺʼʶʺʻʹʹʵʵ` and `ˁʼʼʼʷʳʹʲʿˀʻ`; the speed may be one of them, but the dump gives no name to tell which.

**Combo.** No combo-named field turned up on the engine or on any game class that holds live play state. The hits are `maxCombo` in the leaderboard records (`SlimScoreData` at `0x38`, `ScoreData` at `0xD8`), UI objects (`BasePlayer.comboCounter` at `0x38`, a `ComboColor` display; `ScoreManager.comboTransform` and `comboFont`), and three enum values named `RealDrums...Combo`. The engine's combo count is presumably one of the obfuscated `int` fields between `0x88` and `0xEC` of class 8288, but the dump does not say which, and this file does not guess.

**SP bar.** `SPBar` (type 676) is the on-screen bar, a Unity object whose one field is a sprite:

```
public class SPBar : MonoBehaviour, ˀˁʸʿʻʻʹʼʹʽʷ // TypeDefIndex: 676
	public SpriteRenderer starPowerBar; // 0x20
```

No field named like `sp_bar_ticks`, `spBar` in ticks or `starPowerBar` holds a number. The meter's size in ticks is presumably the engine's `readonly long` at `+0x1B0` (answer 37 calls it the maximum); the dump shows only its type. `StarPowerController` (type 678) and `SpNeckRenderer` (type 641) look like visual effects by their names. The other Star Power fields that hold numbers in the game images are these:

```
public class ʲʻʳʶʲʼʸʴʾʲʳ.ScoreData // TypeDefIndex: 297
	public int spPhraseCount; // 0xC0
	public int spPhrasesHit; // 0xC4
private struct ʲʽʶʳʺʹʳˀʾʴʹ.ˁʺʲʿʽʳʶʾʵʲʲ // TypeDefIndex: 8264
	public int currentStarpowerIndex; // 0x0
	public long spStart; // 0x10
	public long spEnd; // 0x18
```

`ScoreData` is the leaderboard record. Class 8264 is marked `[CompilerGenerated]` and is nested in the same outer class as the chart-data struct 8263, so it is a helper the compiler made for one method (its fields also include a list of Star Power phrases), not a stored meter.
