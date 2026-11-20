// Namespace: System
internal interface ISpanFormattable // TypeDefIndex: 1706
{
	// Methods

	// RVA: -1 Offset: -1 Slot: 0
	public abstract bool TryFormat(Span<char> destination, out int charsWritten, ReadOnlySpan<char> format, IFormatProvider provider);
}

// Namespace: System
internal sealed class TypeNameParser // TypeDefIndex: 1880
{
	// Methods

	// RVA: 0x1BBA730 Offset: 0x1BB9B30 VA: 0x181BBA730
	internal static Type GetType(string typeName, Func<AssemblyName, Assembly> assemblyResolver, Func<Assembly, string, bool, Type> typeResolver, bool throwOnError, bool ignoreCase, ref StackCrawlMark stackMark) { }
}

// Namespace: 
public sealed class Application.MemoryUsageChangedCallback : MulticastDelegate // TypeDefIndex: 11297
{
	// Methods

	// RVA: 0xAEF80 Offset: 0xAE380 VA: 0x1800AEF80
	public void .ctor(object object, IntPtr method) { }

	// RVA: 0x5DC0 Offset: 0x51C0 VA: 0x180005DC0 Slot: 13
	public virtual void Invoke(in ApplicationMemoryUsageChange usage) { }
}

// Namespace: 
internal class Interop.User32 // TypeDefIndex: 1475
{
	// Methods

	// RVA: 0x196F120 Offset: 0x196E520 VA: 0x18196F120
	internal static extern int LoadString(SafeLibraryHandle handle, int id, [Out] StringBuilder buffer, int bufferLength) { }
}

// Namespace: System.IO
internal class CStreamReader : StreamReader // TypeDefIndex: 3212
{
	// Fields
	private TermInfoDriver driver; // 0x60

	// Methods

	// RVA: 0x1B109A0 Offset: 0x1B0FDA0 VA: 0x181B109A0
	public void .ctor(Stream stream, Encoding encoding) { }

	// RVA: 0x1B10710 Offset: 0x1B0FB10 VA: 0x181B10710 Slot: 9
	public override int Peek() { }

	// RVA: 0x1B10790 Offset: 0x1B0FB90 VA: 0x181B10790 Slot: 10
	public override int Read() { }

	// RVA: 0x1B107F0 Offset: 0x1B0FBF0 VA: 0x181B107F0 Slot: 11
	public override int Read([In] [Out] char[] dest, int index, int count) { }

	// RVA: 0x1B10730 Offset: 0x1B0FB30 VA: 0x181B10730 Slot: 14
	public override string ReadLine() { }

	// RVA: 0x1B10760 Offset: 0x1B0FB60 VA: 0x181B10760 Slot: 13
	public override string ReadToEnd() { }
}

// Namespace: System.IO
internal static class MonoIO // TypeDefIndex: 3205
{
	// Fields
	public static readonly IntPtr InvalidHandle; // 0x0
	private static bool dump_handles; // 0x8

	// Properties
	public static IntPtr ConsoleOutput { get; }
	public static IntPtr ConsoleInput { get; }
	public static IntPtr ConsoleError { get; }
	public static char VolumeSeparatorChar { get; }
	public static char DirectorySeparatorChar { get; }
	public static char AltDirectorySeparatorChar { get; }
	public static char PathSeparator { get; }

	// Methods

	// RVA: 0x1B1CF10 Offset: 0x1B1C310 VA: 0x181B1CF10
	public static Exception GetException(MonoIOError error) { }

	// RVA: 0x1B1C820 Offset: 0x1B1BC20 VA: 0x181B1C820
	public static Exception GetException(string path, MonoIOError error) { }

	// RVA: 0x19574B0 Offset: 0x19568B0 VA: 0x1819574B0
	public static string GetCurrentDirectory(out MonoIOError error) { }

	// RVA: 0x1957F60 Offset: 0x1957360 VA: 0x181957F60
	private static MonoFileType GetFileType(IntPtr handle, out MonoIOError error) { }

	// RVA: 0x1B1D020 Offset: 0x1B1C420 VA: 0x181B1D020
	public static MonoFileType GetFileType(SafeHandle safeHandle, out MonoIOError error) { }

	// RVA: 0x19572D0 Offset: 0x19566D0 VA: 0x1819572D0
	public static bool FindCloseFile(IntPtr hnd) { }

	// RVA: 0x19582E0 Offset: 0x19576E0 VA: 0x1819582E0
	private static IntPtr Open(char* filename, FileMode mode, FileAccess access, FileShare share, FileOptions options, out MonoIOError error) { }

	// RVA: 0x1B1D240 Offset: 0x1B1C640 VA: 0x181B1D240
	public static IntPtr Open(string filename, FileMode mode, FileAccess access, FileShare share, FileOptions options, out MonoIOError error) { }

	// RVA: 0x1956A00 Offset: 0x1955E00 VA: 0x181956A00
	private static bool Cancel_internal(IntPtr handle, out MonoIOError error) { }

	// RVA: 0x1B1C700 Offset: 0x1B1BB00 VA: 0x181B1C700
	internal static bool Cancel(SafeHandle safeHandle, out MonoIOError error) { }

	// RVA: 0x1956B30 Offset: 0x1955F30 VA: 0x181956B30
	public static bool Close(IntPtr handle, out MonoIOError error) { }

	// RVA: 0x19582F0 Offset: 0x19576F0 VA: 0x1819582F0
	private static int Read(IntPtr handle, byte[] dest, int dest_offset, int count, out MonoIOError error) { }

	// RVA: 0x1B1D2C0 Offset: 0x1B1C6C0 VA: 0x181B1D2C0
	public static int Read(SafeHandle safeHandle, byte[] dest, int dest_offset, int count, out MonoIOError error) { }

	// RVA: 0x1959180 Offset: 0x1958580 VA: 0x181959180
	private static int Write(IntPtr handle, [In] byte[] src, int src_offset, int count, out MonoIOError error) { }

	// RVA: 0x1B1D660 Offset: 0x1B1CA60 VA: 0x181B1D660
	public static int Write(SafeHandle safeHandle, byte[] src, int src_offset, int count, out MonoIOError error) { }

	// RVA: 0x19585A0 Offset: 0x19579A0 VA: 0x1819585A0
	private static long Seek(IntPtr handle, long offset, SeekOrigin origin, out MonoIOError error) { }

	// RVA: 0x1B1D400 Offset: 0x1B1C800 VA: 0x181B1D400
	public static long Seek(SafeHandle safeHandle, long offset, SeekOrigin origin, out MonoIOError error) { }

	// RVA: 0x1958080 Offset: 0x1957480 VA: 0x181958080
	private static long GetLength(IntPtr handle, out MonoIOError error) { }

	// RVA: 0x1B1D130 Offset: 0x1B1C530 VA: 0x181B1D130
	public static long GetLength(SafeHandle safeHandle, out MonoIOError error) { }

	// RVA: 0x1958F00 Offset: 0x1958300 VA: 0x181958F00
	private static bool SetLength(IntPtr handle, long length, out MonoIOError error) { }

	// RVA: 0x1B1D530 Offset: 0x1B1C930 VA: 0x181B1D530
	public static bool SetLength(SafeHandle safeHandle, long length, out MonoIOError error) { }

	// RVA: 0x1959240 Offset: 0x1958640 VA: 0x181959240
	public static IntPtr get_ConsoleOutput() { }

	// RVA: 0x1959230 Offset: 0x1958630 VA: 0x181959230
	public static IntPtr get_ConsoleInput() { }

	// RVA: 0x1959220 Offset: 0x1958620 VA: 0x181959220
	public static IntPtr get_ConsoleError() { }

	// RVA: 0x1956D30 Offset: 0x1956130 VA: 0x181956D30
	public static bool DuplicateHandle(IntPtr source_process_handle, IntPtr source_handle, IntPtr target_process_handle, out IntPtr target_handle, int access, int inherit, int options, out MonoIOError error) { }

	// RVA: 0x1959270 Offset: 0x1958670 VA: 0x181959270
	public static char get_VolumeSeparatorChar() { }

	// RVA: 0x1959250 Offset: 0x1958650 VA: 0x181959250
	public static char get_DirectorySeparatorChar() { }

	// RVA: 0x1959210 Offset: 0x1958610 VA: 0x181959210
	public static char get_AltDirectorySeparatorChar() { }

	// RVA: 0x1959260 Offset: 0x1958660 VA: 0x181959260
	public static char get_PathSeparator() { }

	// RVA: 0x1956D20 Offset: 0x1956120 VA: 0x181956D20
	private static void DumpHandles() { }

	// RVA: 0x1958440 Offset: 0x1957840 VA: 0x181958440
	public static bool RemapPath(string path, out string newPath) { }

	// RVA: 0x1B1D7A0 Offset: 0x1B1CBA0 VA: 0x181B1D7A0
	private static void .cctor() { }
}

