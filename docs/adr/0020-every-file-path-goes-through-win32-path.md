# Every file path goes through win32_path

Windows refuses an ordinary file path longer than 260 characters. A machine
can opt out of that limit, but it is on by default, and the opt-out only helps
programs that declare themselves long-path aware. Chart packs nest deep enough
to hit it. On 2026-10-03 one chart in the user's library sat at 335 characters,
and the library scan didn't see it at all.

The fix lives in one place. `win32_path` in `src/core/winstr.cpp` takes a
path and hands back one Windows will accept at any length. A short path comes
back unchanged. A long one is made full first, because Windows stops resolving
`.`, `..` and `/` once the prefix is on. Then it gets the `\\?\` prefix
(`\\?\UNC\` for a network share). With that prefix Windows lifts the limit
whatever the machine setting is.

Everything else reaches the disk through it:

- The winstr helpers (`fopen_utf8`, `file_exists_utf8`, `is_directory_utf8`,
  `file_size_bytes`, `read_file_bytes`, `list_dir`) call it themselves.
  Most code only uses those.
- Code that must give a path to Windows or to `std::filesystem` itself wraps
  it at the call: `win32_path(p)` for a wide Win32 call, `os_path(p)` for a
  `std::filesystem` call or a file stream.
- SQLite opens go through `store::open_sqlite`. It applies `win32_path` and
  picks SQLite's own `win32-longpath` layer, whose path buffer holds 32,767
  characters instead of 260.
- Dear ImGui opens `hydra_ui.ini` and the fonts itself. `imconfig.h` turns off
  its built-in file functions, and `src/ui/imgui_files.cpp` supplies ones that
  call `fopen_utf8`.

Every exe except `hydra_tests` also carries a manifest that declares it
long-path aware (`src/app/long_paths.manifest`). That covers third-party code
on machines that have opted in. Hydra's own calls don't depend on it.
`hydra_tests` is left without it on purpose: Windows then refuses a plain long
path even on an opted-in machine, so the long-path tests prove Hydra's handling
and not the machine's setting.

A test keeps the rule from drifting. "every file call in src/ and tools/ goes
through win32_path" (tests/test_long_paths.cpp) reads every source file. It
fails on any raw Windows file call outside winstr.cpp whose line doesn't call
`win32_path`. It also fails on any `std::filesystem` call or file stream whose
line doesn't call `os_path`, and on `utf8_to_wide` used anywhere a path could
pass through it. The DMBot client is the one exception, because it converts a
URL.

Not covered: the Windows shell. Opening a report in the browser goes through
`ShellExecuteW`, which takes no `\\?\` path. The report folder is Documents\Hydra,
which is short.
