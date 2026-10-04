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

For file calls, "long" starts at 248 characters, not 260 (`kPlainPathLimit`
in `winstr.cpp`, which is `MAX_PATH` minus 12). The reason is
`CreateDirectoryW`, the tightest of the file calls: a new folder's path must
leave room for an 8.3 file name inside it, so a plain folder path of 248 or
more already fails. Prefixing from 248 keeps every file call safe with one
edge. The shell has its own edge, 260, covered below. Both numbers were kept
as they are by the user's decision D48 (2026-10-04).

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

A test keeps the rule from drifting. "single-owner rules hold across src/,
tools/ and tests/" (tests/test_single_owner.cpp) reads every source file in
`src` and `tools`. It fails on any raw Windows file call outside winstr.cpp
whose line doesn't call `win32_path`. It fails on a file stream, or a
`std::filesystem` call that takes a path, whose line doesn't call `os_path`.
That second check works from a list of call names, not every call there is.
The list covers making, removing, renaming, copying and testing files and
folders, sizes and times, `canonical`, both directory iterators, `absolute`,
`status`, `equivalent`, `space` and `current_path`. A new kind of call needs a
new name on the list. The test also fails on `utf8_to_wide` used anywhere a
path could pass through it. The DMBot client is the one exception, because it
converts a URL.

## The shell: Open report and Show in folder

The Windows shell (`ShellExecuteW`, Explorer) is the exception. It takes no
`\\?\` path and nothing of 260 characters or more. We measured this on
2026-10-03 with a page that pinged a local listener when it really loaded.
From a long-path-aware program like Hydra.exe, the shell failed on the long
path, the `\\?\` path, a long `file:///` link and even the short 8.3 name,
which it seems to expand back to the long path. A program without the manifest did
better only because the shell rewrote the path into a `\\?\` form that Firefox
accepts and Edge and Chrome don't.

What worked was the short 8.3 name (`C:\CLONEH~1\...`) handed to the program
directly. Firefox, Edge and Chrome all opened it, and Explorer opened the
folder with the file highlighted. A copy of the page at a short path also
opened normally through the shell.

The shell's edge is 260 characters, the plain `MAX_PATH`, and it never takes
a `\\?\` path. One function asks that question: `fits_shell` (winstr) says
whether the shell takes a path as it is. Nothing else compares a path's
length against 260 for the shell. `shell_path`, `open_in_browser`,
`copy_to_short_temp` and the tests all call it, and the source-scan test fails
on a second copy.

So `shell_path` (winstr) turns a path the shell won't take into its short
name, or gives back nothing when there isn't one. That happens when the file
is missing or the drive keeps no short names, which Windows turns off on most
drives other than C:.

- Open report (`app::open_in_browser`): a path the shell takes goes to the
  shell as before. Otherwise it goes to the program Windows opens `.html`
  files with, as its short name. The page is copied to `%TEMP%\Hydra` and
  that copy is opened through the shell in two cases. One is when there is
  no short name. The other is when there is a short name but the `.html`
  program won't launch (no program is set for `.html`, or starting it
  fails). A report is one self-contained file, so the copy shows the same
  page.
- Show in folder (`ui::show_in_folder`): Explorer gets the short name. With
  none, it returns false, and the app's message names the full path.

The source-scan test also fails on a shell launch anywhere but these two files.
