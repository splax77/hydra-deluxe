"""The quiet-machine check ch_start runs before every launch: is a compiler
running, is the CPU busy, was a build folder written to just now. It answers
with the facts as text lines and one verdict, and it changes nothing on the
machine.

ch_start is the one caller. ch_run_chart must not call it: a sample there would
wait before the first press, and the running game would read as load. A check
that could not look (a reader failed, the scan hit its cap) never says quiet.

The design is point 1 of docs/superpowers/plans/2026-11-07-ch-run-timing.md.
"""

from __future__ import annotations

import ctypes
import dataclasses
import json
import os
import re
import subprocess
import time
from typing import Callable, List, Optional, Sequence, Tuple

from tools.ch_mcp import game_files

# The CPU share at or under which the machine is quiet, the seconds it is
# averaged over, and how young a build folder's newest file must be to count as
# a build at work.
QUIET_CPU_PCT = 30
QUIET_SAMPLE_S = 5
BUILD_RECENT_S = 60

# Question 2 of the plan. node and python stay out on purpose: Claude Code is
# Node and the ch server is Python, so either would refuse every launch.
COMPILER_NAMES = ("cl", "link", "MSBuild", "cmake", "ninja", "clang-cl", "lld-link", "rc", "mt",
                  "tsc", "esbuild")

# The user's ruling of 2026-11-07: an MSBuild started with this flag is a helper
# a build left waiting for work, so it is reported and is no running compiler.
_MSBUILD_NAME = "msbuild"
_MSBUILD_HELPER_FLAG = re.compile(r"\s/nodemode:1(\s|$)", re.IGNORECASE)

# The user's ruling of 2026-11-07: how long the folder scan may run. It is twice
# the slowest scan measured on this machine (1.5 s for 31 folders), and a
# refusal at the cap leaves the next scan reading from the disk cache.
SCAN_CAP_S = 3.0

# Reported for the trace's factors table, and never a reason to refuse.
REPORTED_NAMES = ("Spotify.exe", "MsMpEng.exe")

_HOME = os.path.expanduser("~")
# The main checkout under the home folder. Its own build folder and each of its
# worktrees' are the ones scanned. The home folder's .claude\worktrees stays out
# by the same ruling: hooks write there, and no worktree there has a build folder.
_CHECKOUT = ("Downloads", "Hydra", "hydra-test")
# The checkout's two worktree folders, each under the checkout: the one
# Claude Code cuts worktrees in, and the one sessions cut them in by hand.
_WORKTREE_FOLDERS = ((".claude", "worktrees"), ("worktrees",))
_BUILD_FOLDER = "build-cpp"

_READ_ERRORS = (OSError, ValueError, subprocess.SubprocessError)

_CPU_COUNTER = r"\Processor(_Total)\% Processor Time"
_PDH_FMT_DOUBLE = 0x200

_LIST_PROCESSES = ("[Console]::OutputEncoding = [Text.Encoding]::UTF8; Get-CimInstance "
                   "Win32_Process | Select-Object Name,ProcessId,CommandLine | "
                   "ConvertTo-Json -Compress")

_PROGRAM = re.compile(r'\s*("[^"]*"|\S+)')
_WORKTREE = re.compile(r'[A-Za-z]:[\\/][^"\s]*?[\\/]worktrees[\\/][^\\/"\s]+')
_ANY_PATH = re.compile(r'[A-Za-z]:[\\/][^"\s]+')
_BRACKETED = re.compile(r"\(([^()]+)\)\s*$")


@dataclasses.dataclass(frozen=True)
class Proc:
    """One running process; `command_line` is None when Windows hides it."""
    name: str
    pid: int
    command_line: Optional[str]


@dataclasses.dataclass(frozen=True)
class FolderAge:
    """One folder and the age in seconds of the newest file the scan saw in
    it; None when it saw no file. A recent folder's scan stops at its first
    recent file, so the age answers is_recent and may not be the very newest."""
    path: str
    age_s: Optional[float]


@dataclasses.dataclass(frozen=True)
class FolderScan:
    """What one scan read. `unread` counts the folders its cap left, and
    `errors` says which folders it could not read."""
    folders: Tuple[FolderAge, ...]
    entries: int
    took_s: float
    unread: int = 0
    errors: Tuple[str, ...] = ()


@dataclasses.dataclass(frozen=True)
class QuietEnv:
    """The check's readers, handed in so the tests pass fakes."""
    list_processes: Callable[[], Sequence[Proc]]
    sample_cpu: Callable[[float], float]
    scan_folders: Callable[[], FolderScan]
    free_memory_bytes: Callable[[], int]
    power_scheme: Callable[[], str]


@dataclasses.dataclass(frozen=True)
class QuietReport:
    quiet: bool
    lines: Tuple[str, ...]


def is_compiler(process_name: str) -> bool:
    stem = os.path.splitext(process_name)[0].lower()
    return stem in {name.lower() for name in COMPILER_NAMES}


def is_msbuild_helper(process: Proc) -> bool:
    """Is this an idle-capable MSBuild helper. A hidden command line is not,
    because a check that could not look never says quiet."""
    return (os.path.splitext(process.name)[0].lower() == _MSBUILD_NAME
            and process.command_line is not None
            and _MSBUILD_HELPER_FLAG.search(process.command_line) is not None)


def is_recent(age_s: Optional[float]) -> bool:
    """Is a folder whose newest file has this age a build at work."""
    return age_s is not None and age_s < BUILD_RECENT_S


def folder_named(command_line: Optional[str]) -> Optional[str]:
    """The worktree a command line's arguments name, or else the first path in
    them. The program's own path is left out: it says where the compiler is
    installed, not whose build it is."""
    if command_line is None:
        return None
    arguments = _PROGRAM.sub("", command_line, count=1)
    found = _WORKTREE.search(arguments) or _ANY_PATH.search(arguments)
    return found.group(0) if found else None


def _compiler_lines(processes: Sequence[Proc]) -> List[str]:
    lines = []
    for process in processes:
        if is_compiler(process.name) and not is_msbuild_helper(process):
            folder = folder_named(process.command_line)
            where = f"folder {folder}" if folder else "no folder in its command line"
            lines.append(f"compiler running: {process.name}, pid {process.pid}, {where}")
    return lines


def _folder_lines(scan: FolderScan) -> Tuple[List[str], bool]:
    """The scan's lines, and whether the scan lets the machine be quiet."""
    recent = [f for f in scan.folders if is_recent(f.age_s)]
    lines = [f"build folder written to in the last {BUILD_RECENT_S} s: {f.path} "
             f"(a file {f.age_s:.0f} s old)" for f in recent]
    lines.extend(f"build folders: could not read {error}" for error in scan.errors)
    if scan.unread:
        total = len(scan.folders) + scan.unread
        lines.append(f"build folders: the scan hit its cap of {SCAN_CAP_S:g} s and left "
                     f"{scan.unread} of {total} folders unread, so a build may be writing there")
    clear = not lines
    if clear:
        lines.append(f"no build folder written to in the last {BUILD_RECENT_S} s")
    lines.append(f"build folders: {len(scan.folders)} read, {scan.entries} entries, "
                 f"{scan.took_s:.2f} s")
    return lines, clear


def quiet_report(env: QuietEnv) -> QuietReport:
    """Read the machine once and say whether a launch may go ahead. The CPU
    sample comes before the folder scan, so the scan's own work is not counted
    as load."""
    lines: List[str] = []
    quiet = True

    names = None
    try:
        processes = env.list_processes()
    except _READ_ERRORS as e:
        quiet = False
        lines.append(f"process list: could not read it ({e}), so a compiler may be running")
    else:
        names = {process.name.lower() for process in processes}
        compilers = _compiler_lines(processes)
        quiet = quiet and not compilers
        lines.extend(compilers or ["no compiler running"])
        helpers = sum(1 for process in processes if is_msbuild_helper(process))
        if helpers:
            lines.append(f"MSBuild helpers left from a build: {helpers}")

    try:
        cpu = env.sample_cpu(QUIET_SAMPLE_S)
    except _READ_ERRORS as e:
        quiet = False
        lines.append(f"CPU: could not read it ({e})")
    else:
        quiet = quiet and cpu <= QUIET_CPU_PCT
        lines.append(f"CPU: {cpu:.2f} percent averaged over {QUIET_SAMPLE_S} s "
                     f"(limit {QUIET_CPU_PCT} percent)")

    try:
        scan = env.scan_folders()
    except _READ_ERRORS as e:
        quiet = False
        lines.append(f"build folders: could not read them ({e})")
    else:
        folder_lines, clear = _folder_lines(scan)
        quiet = quiet and clear
        lines.extend(folder_lines)

    try:
        lines.append(f"free memory: {env.free_memory_bytes() / 1024 ** 3:.1f} GB")
    except _READ_ERRORS as e:
        lines.append(f"free memory: could not read it ({e})")
    try:
        lines.append(f"power scheme: {env.power_scheme()}")
    except _READ_ERRORS as e:
        lines.append(f"power scheme: could not read it ({e})")

    for name in REPORTED_NAMES:
        state = "unknown" if names is None else (
            "running" if name.lower() in names else "not running")
        lines.append(f"{name}: {state}")
    return QuietReport(quiet, tuple(lines))


def refusal_text(report: QuietReport) -> str:
    return (f"refused: the machine is not quiet: {'; '.join(report.lines)}; "
            "hold the other sessions' builds and call ch_start again")


def _build_folders(home: str) -> List[str]:
    """The build folders that exist: the main checkout's, then its worktrees',
    one worktree folder at a time in _WORKTREE_FOLDERS order."""
    checkout = os.path.join(home, *_CHECKOUT)
    worktrees: List[str] = []
    for parts in _WORKTREE_FOLDERS:
        try:
            with os.scandir(os.path.join(checkout, *parts)) as listing:
                worktrees.extend(sorted(e.path for e in listing if e.is_dir()))
        except FileNotFoundError:
            pass
    folders = (os.path.join(tree, _BUILD_FOLDER) for tree in [checkout, *worktrees])
    return [folder for folder in folders if os.path.isdir(folder)]


def scan_folders(home: str = _HOME, wall: Callable[[], float] = time.time,
                 clock: Callable[[], float] = time.monotonic) -> FolderScan:
    """Find each folder's newest file. os.scandir gets a whole directory's
    file times in one read of the directory, so no file is opened. A folder
    stops at its first recent file, and the whole scan stops at SCAN_CAP_S.
    Directory times cannot stand in for the walk: a file rewritten in place
    leaves its directory's time alone."""
    began = clock()
    now = wall()
    folders = _build_folders(home)
    read: List[FolderAge] = []
    errors: List[str] = []
    entries = 0

    for index, folder in enumerate(folders):
        newest: Optional[float] = None
        waiting = [folder]
        while waiting:
            if clock() - began >= SCAN_CAP_S:
                return FolderScan(tuple(read), entries, clock() - began,
                                  unread=len(folders) - index, errors=tuple(errors))
            directory = waiting.pop()
            try:
                with os.scandir(directory) as listing:
                    for entry in listing:
                        entries += 1
                        if entry.is_dir(follow_symlinks=False):
                            waiting.append(entry.path)
                            continue
                        written = entry.stat(follow_symlinks=False).st_mtime
                        newest = written if newest is None else max(newest, written)
            except FileNotFoundError:
                # A build removed it mid-scan, so there is nothing left to read.
                continue
            except OSError as e:
                errors.append(f"{directory}: {e}")
                continue
            if newest is not None and is_recent(now - newest):
                break
        read.append(FolderAge(folder, None if newest is None else now - newest))
    return FolderScan(tuple(read), entries, clock() - began, errors=tuple(errors))


def run_command(cmd: Sequence[str], runner=subprocess.run) -> str:
    """A command's output, raising OSError when it fails or overruns. It has
    no window, because a run must never take the user's focus. The cap is
    game_files.REG_CAP_S, the user's cap on a call to a Windows tool."""
    try:
        done = runner(cmd, stdin=subprocess.DEVNULL, capture_output=True, encoding="utf-8",
                      errors="replace", timeout=game_files.REG_CAP_S,
                      creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    except subprocess.TimeoutExpired as e:
        raise OSError(f"{subprocess.list2cmdline(cmd)} did not finish in "
                      f"{game_files.REG_CAP_S:.0f} s") from e
    if done.returncode != 0:
        said = (done.stderr or done.stdout or "").strip()
        raise OSError(f"{subprocess.list2cmdline(cmd)} exited {done.returncode}: {said}")
    return done.stdout


def list_processes(run: Callable[[Sequence[str]], str] = run_command) -> List[Proc]:
    """Every running process with its command line, in one read."""
    said = run(["powershell.exe", "-NoProfile", "-NonInteractive", "-Command", _LIST_PROCESSES])
    return [Proc(row["Name"], row["ProcessId"], row["CommandLine"]) for row in json.loads(said)]


def power_scheme(run: Callable[[Sequence[str]], str] = run_command) -> str:
    """The active power scheme's name, as powercfg prints it."""
    said = run(["powercfg", "/getactivescheme"]).strip()
    found = _BRACKETED.search(said)
    return found.group(1) if found else said


class _CounterValue(ctypes.Structure):
    """PDH_FMT_COUNTERVALUE holding a double."""
    _fields_ = [("status", ctypes.c_uint32), ("value", ctypes.c_double)]


def sample_cpu(seconds: float) -> float:
    """LIVE-ONLY: the processor counter's share across all processors,
    averaged between two reads `seconds` apart."""
    pdh = ctypes.WinDLL("pdh")
    handle = ctypes.c_void_p
    calls = {
        "PdhOpenQueryW": (ctypes.c_wchar_p, ctypes.c_size_t, ctypes.POINTER(handle)),
        "PdhAddEnglishCounterW": (handle, ctypes.c_wchar_p, ctypes.c_size_t,
                                  ctypes.POINTER(handle)),
        "PdhCollectQueryData": (handle,),
        "PdhGetFormattedCounterValue": (handle, ctypes.c_uint32, ctypes.c_void_p,
                                        ctypes.POINTER(_CounterValue)),
        "PdhCloseQuery": (handle,),
    }
    for name, argtypes in calls.items():
        getattr(pdh, name).argtypes = argtypes
        getattr(pdh, name).restype = ctypes.c_uint32

    def checked(name: str, *args) -> None:
        status = getattr(pdh, name)(*args)
        if status != 0:
            raise OSError(f"{name} failed: {status:#x}")

    query, counter, value = handle(), handle(), _CounterValue()
    checked("PdhOpenQueryW", None, 0, ctypes.byref(query))
    try:
        checked("PdhAddEnglishCounterW", query, _CPU_COUNTER, 0, ctypes.byref(counter))
        checked("PdhCollectQueryData", query)
        time.sleep(seconds)
        checked("PdhCollectQueryData", query)
        checked("PdhGetFormattedCounterValue", counter, _PDH_FMT_DOUBLE, None,
                ctypes.byref(value))
    finally:
        pdh.PdhCloseQuery(query)
    return value.value


class _MemoryStatus(ctypes.Structure):
    """MEMORYSTATUSEX."""
    _fields_ = [("length", ctypes.c_uint32), ("load", ctypes.c_uint32),
                ("total_phys", ctypes.c_uint64), ("avail_phys", ctypes.c_uint64),
                ("total_page", ctypes.c_uint64), ("avail_page", ctypes.c_uint64),
                ("total_virtual", ctypes.c_uint64), ("avail_virtual", ctypes.c_uint64),
                ("avail_extended", ctypes.c_uint64)]


def free_memory_bytes() -> int:
    """LIVE-ONLY: the free physical memory."""
    status = _MemoryStatus()
    status.length = ctypes.sizeof(_MemoryStatus)
    k32 = ctypes.WinDLL("kernel32", use_last_error=True)
    if not k32.GlobalMemoryStatusEx(ctypes.byref(status)):
        raise OSError(ctypes.get_last_error(), "GlobalMemoryStatusEx failed")
    return status.avail_phys


LIVE_ENV = QuietEnv(list_processes=list_processes, sample_cpu=sample_cpu,
                    scan_folders=scan_folders, free_memory_bytes=free_memory_bytes,
                    power_scheme=power_scheme)
