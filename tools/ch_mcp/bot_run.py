"""Launch Clone Hero with a bot on one song and keep the stats file it writes.

The game's command line plays a song folder with bot players. This script
backs up the user files first, starts the game, waits (with a cap) for
scorestats.json to change, copies the new file out, watches whether the game
exits, and then puts the user files back. It is the one owner of the plan's
"Safety rules for every live step" for the launch.

    py tools/ch_mcp/bot_run.py --song <song folder> --out <folder>
        [--player Drums,Expert] [--profile <name>] [--stop] [--backup-root <folder>]
    py tools/ch_mcp/bot_run.py --song <song folder> --out <folder> --no-wait
    py tools/ch_mcp/bot_run.py --restore <backup folder>

--no-wait backs up and starts the game, then returns at once and restores
nothing; run --restore on the printed folder after the step. The game is
never closed unless --stop is given. No --profile means the game plays as
its default player.

A caller that needs to act between the steps (the MCP server) calls them one
at a time: start_run, wait_stats, exit_check and finish_run. run_bot is those
four in order. start_run with no song folder starts the game at its menu;
launch_command owns what the game is given either way.

start_run with human=True plays the song as a human player instead of the
bot: launch_human starts the game paused, attaches agent/launch.ts, arms it,
resumes the game and waits for the agent to say the loader's bot byte is
patched in memory (the direct-start plan,
docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md, "The launch").
Nothing here writes the game's files on disk.

The file paths and the backup list live in game_files.py; the caps are the
constants below.
"""

from __future__ import annotations

import argparse
import ctypes
import json
import os
import re
import shutil
import subprocess
import sys
import threading
import time
from dataclasses import dataclass, field
from typing import Any, Callable, List, Optional, Protocol

if __package__ in (None, ""):
    # Run as a script (py -I leaves the repo off sys.path).
    sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..")))

from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_mcp import game_files  # noqa: E402
from tools.ch_mcp.frida_tools import AGENT_DIR  # noqa: E402
from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import probe_songs  # noqa: E402

DEFAULT_PLAYER = "Drums,Expert"
# The user's caps for the live steps (the plan's question 5).
POLL_INTERVAL_S = 1.0
POLL_CAP_EXTRA_S = 60.0
EXIT_CHECK_S = 30.0

# What finish_run says when a stop it was asked for worked. The command line
# asks with --stop and says so; a caller with no such flag (the MCP server)
# passes STOPPED_LINE as finish_run's stop_line.
STOPPED_LINE = "stopped the game"
STOPPED_BY_FLAG_LINE = f"{STOPPED_LINE} (--stop)"

MANIFEST = "manifest.json"
PRO_DRUMS_INSTRUMENT = "ProDrums"

# Win32's ShowWindow value for "minimized, not activated"; subprocess names
# only SW_HIDE.
SW_SHOWMINNOACTIVE = 7
# Win32's process creation flag that starts the main thread suspended;
# subprocess has no name for it.
CREATE_SUSPENDED = 0x4

# The human launch's caps and poll, in seconds (the direct-start plan's
# question 1, answered by the user on 2026-10-31). The patch cap counts from
# the resume.
LAUNCH_ATTACH_CAP_S = 20.0
LAUNCH_ARM_CAP_S = 5.0
LAUNCH_PATCH_CAP_S = 30.0
LAUNCH_POLL_S = 0.5

# The agent the human launch loads, and the data file that tells it what to
# patch (both the direct-start plan's "The launch"; task D3 writes them).
LAUNCH_AGENT_FILE = os.path.join(AGENT_DIR, "launch.js")
LAUNCH_PATCH_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "launch_patch.json")
# launch_patch.json's `launch` values: create the process paused with Popen,
# or let Frida spawn it.
LAUNCH_SUSPENDED = "suspended"
LAUNCH_SPAWN = "spawn"
LAUNCH_FORMS = (LAUNCH_SUSPENDED, LAUNCH_SPAWN)
# The `state` words of launch.ts's arm and status replies. Idle means the
# agent holds no arm, which after an arm reply is a fault.
STATE_IDLE = "idle"
STATE_WAITING = "waiting"
STATE_PATCHED = "patched"
STATE_REFUSED = "refused"

# Why a human launch needs a song folder.
HUMAN_NEEDS_SONG = ("refused: a human launch needs a song folder, because the game's "
                    "command-line loader, whose bot byte the launch patches, runs only "
                    "with --song")


def human_refusal(human: bool, song_dir: Optional[str]) -> Optional[str]:
    """The line that refuses a launch asking for a human player with no song
    folder, or None when the launch may go on. start_run raises it as
    LaunchFailed; ch_start asks first, so it refuses before it stops an open
    run."""
    return HUMAN_NEEDS_SONG if human and song_dir is None else None


class SongLengthMissing(Exception):
    pass


class LaunchFailed(Exception):
    """A human launch that did not reach a patched loader. The game it started
    has been killed by the time this is raised; the message says why."""


class Game(Protocol):
    pid: int

    def is_running(self) -> bool: ...

    def stop(self) -> None: ...


class _Process:
    """The real game process."""

    def __init__(self, popen: subprocess.Popen):
        self._p = popen
        self.pid = popen.pid

    def is_running(self) -> bool:
        return self._p.poll() is None

    def stop(self) -> None:
        self._p.terminate()
        self._p.wait(timeout=EXIT_CHECK_S)


def _minimized_no_focus() -> "subprocess.STARTUPINFO":
    """The start-up info that asks the new process to open its window
    minimized and not take the focus (SW_SHOWMINNOACTIVE)."""
    info = subprocess.STARTUPINFO()
    info.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    info.wShowWindow = SW_SHOWMINNOACTIVE
    return info


def _popen_game(cmd: list[str], **extra: Any) -> Game:
    """Start the game in its own folder, minimized and without the focus, so
    the user's window stays in front (foreground.py checks it did). Its stdin
    and stdout go nowhere: the MCP server's stdout is its protocol channel
    and its stdin is where the protocol comes in, and a child that inherited
    them could write into one or read from the other. Its stderr stays the
    caller's. `extra` goes to Popen as well."""
    return _Process(subprocess.Popen(cmd, cwd=os.path.dirname(cmd[0]),
                                     stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
                                     startupinfo=_minimized_no_focus(), **extra))


def _real_launch(cmd: list[str]) -> Game:
    """The bot launch and the menu launch: _popen_game as it is."""
    return _popen_game(cmd)


def _real_start_suspended(cmd: list[str]) -> Game:
    """The human launch's paused start: _popen_game with the main thread
    suspended until resume_main_thread."""
    return _popen_game(cmd, creationflags=CREATE_SUSPENDED)


# Win32 access rights, wait results and Toolhelp flags the code below uses.
_SYNCHRONIZE = 0x00100000
_PROCESS_TERMINATE = 0x0001
_PROCESS_QUERY_LIMITED_INFORMATION = 0x1000
_THREAD_SUSPEND_RESUME = 0x0002
_THREAD_QUERY_LIMITED_INFORMATION = 0x0800
_TH32CS_SNAPTHREAD = 0x4
_WAIT_TIMEOUT = 0x102
_RESUME_FAILED = 0xFFFFFFFF


def _kernel32() -> Any:
    from ctypes import wintypes  # noqa: PLC0415  (Windows only)
    k32 = ctypes.WinDLL("kernel32", use_last_error=True)
    k32.OpenProcess.restype = wintypes.HANDLE
    k32.OpenProcess.argtypes = (wintypes.DWORD, wintypes.BOOL, wintypes.DWORD)
    k32.OpenThread.restype = wintypes.HANDLE
    k32.OpenThread.argtypes = (wintypes.DWORD, wintypes.BOOL, wintypes.DWORD)
    k32.CloseHandle.argtypes = (wintypes.HANDLE,)
    k32.WaitForSingleObject.restype = wintypes.DWORD
    k32.WaitForSingleObject.argtypes = (wintypes.HANDLE, wintypes.DWORD)
    k32.TerminateProcess.argtypes = (wintypes.HANDLE, wintypes.UINT)
    k32.CreateToolhelp32Snapshot.restype = wintypes.HANDLE
    k32.CreateToolhelp32Snapshot.argtypes = (wintypes.DWORD, wintypes.DWORD)
    k32.ResumeThread.restype = wintypes.DWORD
    k32.ResumeThread.argtypes = (wintypes.HANDLE,)
    entry = ctypes.POINTER(_THREADENTRY32)
    k32.Thread32First.argtypes = (wintypes.HANDLE, entry)
    k32.Thread32Next.argtypes = (wintypes.HANDLE, entry)
    times = ctypes.POINTER(_FILETIME)
    k32.GetThreadTimes.argtypes = (wintypes.HANDLE, times, times, times, times)
    return k32


class _THREADENTRY32(ctypes.Structure):
    _fields_ = [("dwSize", ctypes.c_uint32), ("cntUsage", ctypes.c_uint32),
                ("th32ThreadID", ctypes.c_uint32), ("th32OwnerProcessID", ctypes.c_uint32),
                ("tpBasePri", ctypes.c_int32), ("tpDeltaPri", ctypes.c_int32),
                ("dwFlags", ctypes.c_uint32)]


class _FILETIME(ctypes.Structure):
    _fields_ = [("low", ctypes.c_uint32), ("high", ctypes.c_uint32)]


def _thread_ids(k32: Any, pid: int) -> list[int]:
    """The ids of every thread process `pid` has now, from a Toolhelp
    snapshot."""
    snap = k32.CreateToolhelp32Snapshot(_TH32CS_SNAPTHREAD, 0)
    if snap is None or snap == ctypes.c_void_p(-1).value:
        raise OSError(f"CreateToolhelp32Snapshot failed: error {ctypes.get_last_error()}")
    try:
        entry = _THREADENTRY32()
        entry.dwSize = ctypes.sizeof(_THREADENTRY32)
        ids = []
        more = k32.Thread32First(snap, ctypes.byref(entry))
        while more:
            if entry.th32OwnerProcessID == pid:
                ids.append(entry.th32ThreadID)
            more = k32.Thread32Next(snap, ctypes.byref(entry))
        return ids
    finally:
        k32.CloseHandle(snap)


def resume_main_thread(pid: int) -> None:
    """Resume the main thread of a process created with CREATE_SUSPENDED.
    Popen closes its thread handle, so the thread is found again: of the
    process's threads (Frida's attach adds its own), the main thread is the
    one created first. Raises OSError when there is no thread, when the
    thread cannot be opened or resumed, or when it was not suspended."""
    k32 = _kernel32()
    oldest: Optional[tuple[int, int]] = None  # (creation time, thread id)
    for tid in _thread_ids(k32, pid):
        h = k32.OpenThread(_THREAD_QUERY_LIMITED_INFORMATION, False, tid)
        if not h:
            continue  # a thread that ended since the snapshot
        try:
            created, gone, kernel, user = (_FILETIME() for _ in range(4))
            if k32.GetThreadTimes(h, ctypes.byref(created), ctypes.byref(gone),
                                  ctypes.byref(kernel), ctypes.byref(user)):
                when = (created.high << 32) | created.low
                if oldest is None or when < oldest[0]:
                    oldest = (when, tid)
        finally:
            k32.CloseHandle(h)
    if oldest is None:
        raise OSError(f"process {pid} has no thread to resume")
    tid = oldest[1]
    h = k32.OpenThread(_THREAD_SUSPEND_RESUME, False, tid)
    if not h:
        raise OSError(f"OpenThread on thread {tid} of process {pid} failed: "
                      f"error {ctypes.get_last_error()}")
    try:
        before = k32.ResumeThread(h)
    finally:
        k32.CloseHandle(h)
    if before == _RESUME_FAILED:
        raise OSError(f"ResumeThread on thread {tid} of process {pid} failed: "
                      f"error {ctypes.get_last_error()}")
    if before == 0:
        raise OSError(f"thread {tid} of process {pid} was not suspended")


class _PidProcess:
    """A game Frida spawned, known by its process id: Windows' own process
    handle answers is_running and stop."""

    def __init__(self, pid: int):
        self.pid = pid
        self._k32 = _kernel32()
        self._h = self._k32.OpenProcess(
            _SYNCHRONIZE | _PROCESS_TERMINATE | _PROCESS_QUERY_LIMITED_INFORMATION, False, pid)
        if not self._h:
            raise OSError(f"OpenProcess on {pid} failed: error {ctypes.get_last_error()}")

    def is_running(self) -> bool:
        return self._k32.WaitForSingleObject(self._h, 0) == _WAIT_TIMEOUT

    def stop(self) -> None:
        self._k32.TerminateProcess(self._h, 1)
        if self._k32.WaitForSingleObject(self._h, int(EXIT_CHECK_S * 1000)) == _WAIT_TIMEOUT:
            raise subprocess.TimeoutExpired(C.PROCESS_NAME, EXIT_CHECK_S)


def _real_spawn(cmd: list[str]) -> Game:
    """The spawn form of the human launch: Frida creates the game paused in
    its own folder. Frida's spawn takes no show-window option, so the window
    is not asked to open minimized (the plan's question 2). Its output goes
    to a pipe for the reason _popen_game gives."""
    device = FA._import_frida().get_local_device()
    pid = device.spawn(cmd, cwd=os.path.dirname(cmd[0]), stdio="pipe")
    return _PidProcess(pid)


def _real_resume_spawned(pid: int) -> None:
    """Resume a game Frida spawned."""
    FA._import_frida().get_local_device().resume(pid)


def _real_attach_agent(pid: int, agent_path: str, log_path: str,
                       out: Callable[[str], None]) -> Any:
    """The LiveAgent that attaches to process `pid` (frida_attach.LiveAgent's
    `pid` argument). Its start() makes the attach."""
    return FA.LiveAgent(agent_path, log_path=log_path, pid=pid, out=out)


def _kill_game(game: Game) -> None:
    game.stop()


def join_thread(thread: threading.Thread, cap_s: float) -> None:
    """Wait for `thread` for at most `cap_s` seconds. The one real wait on a
    worker thread: Env.wait_thread and GameTools' wait_thread both default to
    it, and a test swaps either for a fake."""
    thread.join(cap_s)


@dataclass
class Env:
    """Everything the run touches outside its own arguments. The tests swap
    each piece for a fake."""
    exe: str = game_files.GAME_EXE
    documents_dir: str = game_files.DOCUMENTS_DIR
    locallow_dir: str = game_files.LOCALLOW_DIR
    launch: Callable[[list[str]], Game] = _real_launch
    now: Callable[[], float] = time.monotonic
    sleep: Callable[[float], None] = time.sleep
    stamp: Callable[[], str] = field(default=lambda: time.strftime("%Y%m%d-%H%M%S"))
    out: Callable[[str], None] = print
    # How the registry key is exported, deleted and imported. None leaves the
    # key out of the backup, which is what a test on temp folders needs.
    run_reg: Optional[game_files.RegRunner] = game_files.run_reg
    # The human launch (launch_human). start_process and spawn_process start
    # the game paused, one per launch form; attach_agent builds the LiveAgent
    # for a process id (pid, agent path, log path, out); resume_thread and
    # resume_spawned let the game go, one per form; kill ends a launch that
    # failed; wait_thread waits on the attach's thread for at most its cap.
    start_process: Callable[[list[str]], Game] = _real_start_suspended
    spawn_process: Callable[[list[str]], Game] = _real_spawn
    attach_agent: Callable[[int, str, str, Callable[[str], None]], Any] = _real_attach_agent
    resume_thread: Callable[[int], None] = resume_main_thread
    resume_spawned: Callable[[int], None] = _real_resume_spawned
    kill: Callable[[Game], None] = _kill_game
    wait_thread: Callable[[threading.Thread, float], None] = join_thread
    launch_agent_path: str = LAUNCH_AGENT_FILE
    launch_patch_path: str = LAUNCH_PATCH_FILE


def song_length_ms(song_dir: str) -> tuple[int, str]:
    """The song's length in ms and where it came from: song.ini's
    song_length, else the probe manifest's last note through
    probe_songs.song_length_ms."""
    ms = game_files.song_ini_length_ms(song_dir)
    if ms is not None:
        return ms, f"song_length in {os.path.join(song_dir, game_files.SONG_INI)}"
    path = os.path.join(song_dir, MANIFEST)
    if os.path.isfile(path):
        with open(path, encoding="utf-8") as f:
            notes = json.load(f).get("notes") or []
        if notes:
            return (probe_songs.song_length_ms(int(notes[-1]["time_ms"])),
                    f"last note in {path} plus probe_songs.SILENCE_MS")
    raise SongLengthMissing(
        f"{song_dir}: no song_length in song.ini and no notes in manifest.json; "
        "the stats wait has no cap, so nothing was started")


def run_cap_s(length_ms: int) -> float:
    """How long a run of a song `length_ms` long may take, in wall seconds:
    the song plus POLL_CAP_EXTRA_S. The stats wait and the server's other
    run caps use this one rule."""
    return C.ms_to_s(length_ms) + POLL_CAP_EXTRA_S


def launch_command(exe: str, song_dir: Optional[str], player: str,
                   profile: Optional[str] = None) -> list[str]:
    """The game's whole command line. With no song folder the game starts at
    its menu, the way the user starts it by hand, so it gets no flags."""
    if song_dir is None:
        return [exe]
    cmd = [exe, "--song", song_dir, "--player", player]
    if profile:
        cmd += ["--profile", profile]
    return cmd


def _file_state(path: str) -> Optional[tuple[int, int]]:
    try:
        st = os.stat(path)
    except FileNotFoundError:
        return None
    return st.st_size, st.st_mtime_ns


def _parses(path: str) -> bool:
    try:
        with open(path, encoding="utf-8-sig") as f:
            json.load(f)
    except (OSError, ValueError):
        return False
    return True


def _slug(text: str) -> str:
    return re.sub(r"[^A-Za-z0-9_-]+", "_", text).strip("_") or "song"


def _poll_stats(song_dir: str, out_dir: str, cap_s: float,
                before: Optional[tuple[int, int]], env: Env) -> Optional[str]:
    """Wait for scorestats.json to change and parse, then copy it into
    `out_dir`. Returns the copy, or None when the cap passes first."""
    path = os.path.join(env.documents_dir, game_files.SCORESTATS)
    start = env.now()
    changed = False
    while True:
        env.sleep(POLL_INTERVAL_S)
        elapsed = env.now() - start
        state = _file_state(path)
        if state is not None and state != before:
            changed = True
            if _parses(path):
                name = f"scorestats-{_slug(os.path.basename(os.path.normpath(song_dir)))}-{env.stamp()}.json"
                dest = os.path.join(out_dir, name)
                shutil.copy2(path, dest)
                env.out(f"scorestats.json changed after {elapsed:.0f} s; copied to {dest}")
                return dest
        if elapsed >= cap_s:
            if changed:
                env.out(f"scorestats.json changed but was not valid JSON after {elapsed:.0f} s; "
                        "nothing copied")
            else:
                env.out(f"scorestats.json unchanged after {elapsed:.0f} s; bot launch wrote no stats")
            return None


def _stats_state(env: Env) -> Optional[tuple[int, int]]:
    return _file_state(os.path.join(env.documents_dir, game_files.SCORESTATS))


@dataclass
class HumanLaunch:
    """A human launch that reached a patched loader. `agent` is the LiveAgent
    holding launch.ts, kept attached so ch_status can ask its status again
    and ch_stop can detach it; `patch` is launch_patch.json as it was sent
    with arm; `said` collects what the agent prints (its detach notes)."""
    game: Game
    agent: Any
    patch: dict
    form: str
    said: List[str] = field(default_factory=list)


@dataclass
class Run:
    """One launched game, between start_run and finish_run. song_dir, cap_s,
    length_ms and length_source are None when no song was launched from the
    command line. `launch` is the human launch, or None for a bot or menu
    launch."""
    song_dir: Optional[str]
    backup_dir: str
    command: list[str]
    game: Game
    launched_at: float  # on env.now
    cap_s: Optional[float]
    length_ms: Optional[int]
    length_source: Optional[str]
    before: Optional[tuple[int, int]]  # scorestats.json's state before the launch
    launch: Optional[HumanLaunch] = None


def _read_patch(path: str) -> dict:
    """launch_patch.json, with its `launch` form checked. Raises LaunchFailed
    naming the file and what is wrong; nothing has been started then."""
    try:
        with open(path, encoding="utf-8") as f:
            patch = json.load(f)
    except (OSError, ValueError) as e:
        raise LaunchFailed(f"could not read {path}: {e}; nothing was started") from e
    if not isinstance(patch, dict):
        raise LaunchFailed(f"{path} holds no JSON object; nothing was started")
    form = patch.get("launch")
    if form not in LAUNCH_FORMS:
        raise LaunchFailed(f"{path}: `launch` is {form!r}, not one of "
                           f"{', '.join(LAUNCH_FORMS)}; nothing was started")
    return patch


def _byte_values(value: Any) -> Optional[tuple[int, ...]]:
    """A byte string as the agent or the data file writes it, hex text such
    as "BA 00 00 00 00" or a list of numbers, as a tuple; None when it is
    neither, so it never equals anything."""
    if isinstance(value, str):
        try:
            return tuple(bytes.fromhex(value))
        except ValueError:
            return None
    if isinstance(value, (list, tuple)) and all(isinstance(v, int) for v in value):
        return tuple(value)
    return None


def _bytes_text(value: Any) -> str:
    """A byte string for a reply line: its hex text, or what came if it is
    not one."""
    values = _byte_values(value)
    if values is None:
        return "none" if value is None else repr(value)
    return " ".join(f"{v:02X}" for v in values)


# ch_status's loader line and its three words: the direct-start plan's
# LS-D0 pass line.
LOADER_LINE = "loader ran after patch: "
LOADER_YES = "yes"
LOADER_NO = "no"
LOADER_NOT_YET = "not yet"


def loader_ran(status: dict, patch: dict) -> str:
    """Whether the game's command-line loader ran after the patch landed,
    from launch.ts's status: LOADER_YES when it ran and the hook read the
    patch's `write` bytes at its first run, LOADER_NO when it ran and read
    other bytes, LOADER_NOT_YET when it has not run."""
    runs = status.get("loader_runs")
    if not isinstance(runs, int) or runs <= 0:
        return LOADER_NOT_YET
    seen = _byte_values(status.get("bytes_at_first_run"))
    return LOADER_YES if seen is not None and seen == _byte_values(patch.get("write")) else LOADER_NO


def launch_status_lines(status: dict, patch: dict) -> List[str]:
    """launch.ts's status reply as reply lines, ending with the loader line."""
    lines = [f"launch patch: {status.get('state')}; at the patch {_bytes_text(status.get('before'))}"
             f" before and {_bytes_text(status.get('after'))} after; at the loader's entry "
             f"{_bytes_text(status.get('entry_before'))}"]
    module = status.get("module")
    if isinstance(module, dict):
        lines.append(f"module: base {module.get('base')}, size {module.get('size')}")
    if status.get("state") == STATE_IDLE:
        lines.append("the launch agent holds no arm, so it will never patch")
    if status.get("patched_at_ms") is not None:
        lines.append(f"patched at: {status.get('patched_at_ms')} ms on the agent's own clock "
                     "(Date.now)")
    lines.append(f"loader runs since the patch: {status.get('loader_runs')}; bytes at its first "
                 f"run: {_bytes_text(status.get('bytes_at_first_run'))}")
    if status.get("reason"):
        lines.append(f"launch agent reason: {status.get('reason')}")
    lines.append(LOADER_LINE + loader_ran(status, patch))
    return lines


def request_launch_status(launch: HumanLaunch, cap_s: float) -> List[str]:
    """Ask the launch agent for its status, waiting at most `cap_s`, and
    return launch_status_lines of it, or the line naming the failure."""
    try:
        status = launch.agent.request({"type": "status"}, cap_s)
    except FA.ReadFailed as e:
        return [f"launch agent: status failed: {e}"]
    return launch_status_lines(status, launch.patch)


def _refused_text(status: dict, patch: dict) -> str:
    """A refusal: the bytes the agent found against the ones expected, then
    the agent's own reason (a byte check, a failed read-back, or a read,
    write or hook that threw)."""
    return (f"launch.ts refused the patch: at the loader's entry it found "
            f"{_bytes_text(status.get('entry_before'))} (launch_patch.json expects "
            f"{_bytes_text(patch.get('entry_expect'))}), and at the patch it found "
            f"{_bytes_text(status.get('before'))} (expects {_bytes_text(patch.get('expect'))}) "
            f"and {_bytes_text(status.get('after'))} after; the agent says: "
            f"{status.get('reason') or 'no reason given'}")


# What launch_human says when the agent answers idle after its arm reply.
IDLE_AFTER_ARM = ("the launch agent answered idle after its arm reply, so it holds no arm "
                  "and will never patch")


def launch_human(cmd: list[str], env: Env, log_path: str) -> HumanLaunch:
    """Start the game for a human player, in the order the direct-start
    plan's "The launch" sets: start it paused, attach launch.ts by process
    id, send arm with launch_patch.json, resume the main thread once the arm
    reply has come, then ask for the status every LAUNCH_POLL_S until it
    says patched. Each wait has its cap (LAUNCH_ATTACH_CAP_S,
    LAUNCH_ARM_CAP_S, LAUNCH_PATCH_CAP_S from the resume). A refusal, a
    failure or a cap kills the game, detaches the agent and raises
    LaunchFailed naming it. launch_patch.json's `launch` picks the form:
    LAUNCH_SUSPENDED (env.start_process, then env.resume_thread) or
    LAUNCH_SPAWN (env.spawn_process, then env.resume_spawned). The agent's
    log goes to `log_path`."""
    patch = _read_patch(env.launch_patch_path)
    form = patch["launch"]
    if not os.path.isfile(env.launch_agent_path):
        raise LaunchFailed(f"the launch agent is not built: no {env.launch_agent_path}; run "
                           "npm run build:launch in tools/ch_mcp/agent; nothing was started")
    said: List[str] = []
    start = env.spawn_process if form == LAUNCH_SPAWN else env.start_process
    game = start(cmd)
    env.out(f"started paused ({form}): process id {game.pid}")
    agent = None

    def fail(why: str) -> LaunchFailed:
        """Kill the game and detach the agent, then the error to raise."""
        notes = []
        try:
            env.kill(game)
            notes.append("the game was killed")
        except Exception as e:  # noqa: BLE001  (the reason above matters more)
            notes.append(f"killing the game failed: {type(e).__name__}: {e}")
        if agent is not None:
            notes.append(f"launch agent: {agent.detach()}")
        return LaunchFailed(f"{why}; " + "; ".join(notes))

    # The attach runs on its own thread, because Frida's attach to a process
    # that has not started yet has no cap of its own.
    box: dict = {}

    def attach() -> None:
        try:
            box["agent"] = env.attach_agent(game.pid, env.launch_agent_path, log_path, said.append)
            box["agent"].start(LAUNCH_ATTACH_CAP_S)
            box["started"] = True
        except BaseException as e:  # noqa: BLE001  (reported below)
            box["error"] = e

    worker = threading.Thread(target=attach, name="launch attach", daemon=True)
    worker.start()
    env.wait_thread(worker, LAUNCH_ATTACH_CAP_S)
    if worker.is_alive():
        # The agent is still inside its start, so it is left alone: killing
        # the game ends its session.
        raise fail(f"the attach to process {game.pid} had not finished after "
                   f"{LAUNCH_ATTACH_CAP_S:g} s (LAUNCH_ATTACH_CAP_S)")
    if "error" in box:
        e = box["error"]
        raise fail(f"the attach to process {game.pid} failed: {type(e).__name__}: {e}")
    agent = box["agent"]
    env.out(f"launch agent attached; log: {log_path}")

    try:
        armed = agent.request({"type": "arm", "patch": patch}, LAUNCH_ARM_CAP_S)
    except FA.ReadFailed as e:
        cap = " (LAUNCH_ARM_CAP_S)" if e.reason == FA.END_CAP else ""
        raise fail(f"the arm request failed: {e}{cap}") from None
    # The arm reply carries the agent's state at that moment: waiting on a
    # paused game, patched if the module was already mapped.
    if armed.get("state") == STATE_REFUSED:
        raise fail(_refused_text(armed, patch))
    if armed.get("state") == STATE_IDLE:
        raise fail(IDLE_AFTER_ARM)
    if armed.get("state") not in (STATE_WAITING, STATE_PATCHED):
        raise fail(f"the arm reply's state is {armed.get('state')!r}, not {STATE_WAITING}")
    modules = armed.get("modules") or []
    env.out(f"armed; modules loaded now ({len(modules)}): " + ", ".join(map(str, modules)))

    try:
        (env.resume_spawned if form == LAUNCH_SPAWN else env.resume_thread)(game.pid)
    except Exception as e:  # noqa: BLE001
        raise fail(f"resuming process {game.pid} failed: {type(e).__name__}: {e}") from None
    resumed_at = env.now()
    env.out("resumed the game")

    while True:
        remaining = LAUNCH_PATCH_CAP_S - (env.now() - resumed_at)
        if remaining <= 0:
            raise fail(f"the status still said {STATE_WAITING} {LAUNCH_PATCH_CAP_S:g} s after "
                       "the resume (LAUNCH_PATCH_CAP_S)")
        try:
            status = agent.request({"type": "status"}, remaining)
        except FA.ReadFailed as e:
            cap = " (LAUNCH_PATCH_CAP_S)" if e.reason == FA.END_CAP else ""
            raise fail(f"the status request failed: {e}{cap}") from None
        state = status.get("state")
        if state == STATE_PATCHED:
            env.out(f"patched {env.now() - resumed_at:.1f} s after the resume")
            for line in launch_status_lines(status, patch):
                env.out(line)
            return HumanLaunch(game=game, agent=agent, patch=patch, form=form, said=said)
        if state == STATE_REFUSED:
            raise fail(_refused_text(status, patch))
        if state == STATE_IDLE:
            raise fail(IDLE_AFTER_ARM)
        if state != STATE_WAITING:
            raise fail(f"the status reply's state is {state!r}"
                       + (f": {status.get('reason')}" if status.get("reason") else ""))
        env.sleep(LAUNCH_POLL_S)


def start_run(song_dir: Optional[str], *, player: str = DEFAULT_PLAYER,
              profile: Optional[str] = None, backup_root: str,
              env: Optional[Env] = None,
              before_launch: Optional[Callable[[], None]] = None,
              human: bool = False) -> Run:
    """Back up the user files, then start the game, and return at once. With
    a song folder the game plays it with a bot; a song with no known length
    raises SongLengthMissing before anything is backed up or started. With
    song_dir None the game starts at its menu and the run has no stats cap.
    With `human` the song is played by a human player instead: launch_human
    starts the game and returns once the loader's bot byte is patched, and
    a human launch with no song folder raises LaunchFailed(HUMAN_NEEDS_SONG)
    before anything is backed up. `before_launch`, when given, runs after
    the backup and before the launch, so a change it makes to the user files
    is undone by the restore. When it or the launch raises, the user files
    are put back before the error goes on, since no run comes back to
    finish."""
    env = env or Env()
    refusal = human_refusal(human, song_dir)
    if refusal is not None:
        raise LaunchFailed(refusal)
    length_ms = source = cap_s = None
    if song_dir is not None:
        # The game starts in its own folder, so a relative song path would be
        # read from there.
        song_dir = os.path.abspath(song_dir)
        length_ms, source = song_length_ms(song_dir)
        cap_s = run_cap_s(length_ms)
        if (game_files.song_ini_pro_drums(song_dir)
                and player.split(",")[0].strip() != PRO_DRUMS_INSTRUMENT):
            env.out(f"note: song.ini says pro_drums = True but the player is {player}; "
                    f"pass --player {PRO_DRUMS_INSTRUMENT},<difficulty> to play pro drums")

    backup_dir = game_files.backup(env.documents_dir, env.locallow_dir, backup_root,
                                   stamp=env.stamp, run_reg=env.run_reg)
    env.out(f"backup folder: {backup_dir}")
    before = _stats_state(env)

    cmd = launch_command(env.exe, song_dir, player, profile)
    human_launch = None
    try:
        if before_launch is not None:
            before_launch()
        env.out("starting: " + subprocess.list2cmdline(cmd))
        if human:
            log_path = os.path.join(backup_root, f"launch-{env.stamp()}.jsonl")
            human_launch = launch_human(cmd, env, log_path)
            game = human_launch.game
        else:
            game = env.launch(cmd)
    except BaseException:
        # No run comes back to finish, so put back what before_launch changed.
        _restore(backup_dir, env)
        raise
    launched_at = env.now()
    env.out(f"game process id: {game.pid}")
    return Run(song_dir=song_dir, backup_dir=backup_dir, command=cmd, game=game,
               launched_at=launched_at, cap_s=cap_s, length_ms=length_ms,
               length_source=source, before=before, launch=human_launch)


def wait_stats(run: Run, out_dir: str, env: Optional[Env] = None) -> Optional[str]:
    """Wait, up to the run's cap, for the game to write new stats, and copy
    them into `out_dir`. Returns the copy, or None when the cap passes first
    or the run launched no song."""
    env = env or Env()
    if run.cap_s is None:
        env.out("no song was launched from the command line; nothing to wait for")
        return None
    os.makedirs(out_dir, exist_ok=True)
    env.out(f"waiting up to {run.cap_s:.1f} s for scorestats.json to change "
            f"({run.length_ms} ms from {run.length_source}, plus {POLL_CAP_EXTRA_S:.0f} s)")
    return _poll_stats(run.song_dir, out_dir, run.cap_s, run.before, env)


def exit_check(run: Run, env: Optional[Env] = None) -> None:
    """Watch, up to EXIT_CHECK_S, whether the game exits on its own."""
    env = env or Env()
    game = run.game
    start = env.now()
    while game.is_running():
        if env.now() - start >= EXIT_CHECK_S:
            env.out(f"game still running after {EXIT_CHECK_S:.0f} s (menu or results screen)")
            return
        env.sleep(POLL_INTERVAL_S)
    env.out(f"game exited after {env.now() - start:.0f} s")


def _restore(backup_dir: str, env: Env) -> bool:
    """Put the user files back, then the registry key when the backup holds
    it. On a failure, print the file and the error and return False; the
    caller then stops."""
    env.out(f"restoring the user files from {backup_dir}")
    try:
        game_files.restore_all(backup_dir, out=env.out, run_reg=env.run_reg)
    except game_files.RestoreError as e:
        env.out(f"restore failed on {e.name}: {e.error}")
        env.out(f"stopped; the backup is still in {backup_dir}")
        return False
    return True


def _restore_hint(backup_dir: str) -> str:
    return f'py tools/ch_mcp/bot_run.py --restore "{backup_dir}"'


def late_write_hint(backup_dir: str) -> str:
    """What a user is told when a game that may still run could rewrite the
    files that were just restored. The caller says whether the game runs."""
    return ("may write these files again when it closes; after closing it, run "
            + _restore_hint(backup_dir))


def finish_run(run: Run, *, stop: bool, env: Optional[Env] = None,
               stop_line: str = STOPPED_BY_FLAG_LINE) -> bool:
    """Stop the game when asked, then put the user files back. A game that
    is still running after the stop's wait is reported and the restore still
    runs. A stop that worked prints `stop_line`. Returns False when the
    restore failed."""
    env = env or Env()
    game = run.game
    try:
        if stop and game.is_running():
            try:
                game.stop()
            except subprocess.TimeoutExpired:
                env.out(f"{C.PROCESS_NAME} (pid {game.pid}) was told to stop but was "
                        f"still running after {EXIT_CHECK_S:.0f} s")
            else:
                env.out(stop_line)
    except BaseException:
        _restore(run.backup_dir, env)
        raise
    if not _restore(run.backup_dir, env):
        return False
    if game.is_running():
        env.out("the game is still running and " + late_write_hint(run.backup_dir))
    return True


def run_bot(song_dir: str, out_dir: str, *, player: str = DEFAULT_PLAYER,
            profile: Optional[str] = None, stop: bool = False, no_wait: bool = False,
            backup_root: Optional[str] = None, env: Optional[Env] = None) -> int:
    """One bot launch: start_run, wait_stats, exit_check and finish_run in
    order. Returns 0 when the stats file was copied (or, with no_wait, when
    the game started), 1 when the cap passed with no stats, 2 when a restore
    failed."""
    env = env or Env()
    run = start_run(song_dir, player=player, profile=profile,
                    backup_root=backup_root or os.path.join(out_dir, "backups"), env=env)
    os.makedirs(out_dir, exist_ok=True)
    if no_wait:
        env.out("--no-wait: not waiting and not restoring; afterwards run "
                + _restore_hint(run.backup_dir))
        return 0

    try:
        copied = wait_stats(run, out_dir, env)
        if copied is not None:
            exit_check(run, env)
    except BaseException:
        _restore(run.backup_dir, env)
        raise
    if not finish_run(run, stop=stop, env=env):
        return 2
    return 0 if copied is not None else 1


def restore_only(backup_dir: str, env: Optional[Env] = None) -> int:
    env = env or Env()
    return 0 if _restore(backup_dir, env) else 2


def main(argv: Optional[list[str]] = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--song", help="the song folder to play")
    ap.add_argument("--out", help="where the stats copy and the backups go")
    ap.add_argument("--player", default=DEFAULT_PLAYER,
                    help=f"<Instrument>,<Difficulty> (default {DEFAULT_PLAYER})")
    ap.add_argument("--profile", help="a game profile name; omit to play as the default player")
    ap.add_argument("--stop", action="store_true", help="close the game when the run ends")
    ap.add_argument("--no-wait", action="store_true",
                    help="back up and start the game, then return without waiting or restoring")
    ap.add_argument("--backup-root", help="where the dated backup folder goes (default <out>\\backups)")
    ap.add_argument("--restore", metavar="BACKUP_FOLDER",
                    help="put the user files back from a backup folder, then stop")
    args = ap.parse_args(argv)

    if args.restore:
        return restore_only(args.restore)
    if not args.song or not args.out:
        ap.error("--song and --out are needed unless --restore is given")
    if not os.path.isfile(game_files.GAME_EXE):
        print(f"the game is not at {game_files.GAME_EXE}; nothing was started")
        return 2
    try:
        return run_bot(args.song, args.out, player=args.player, profile=args.profile,
                       stop=args.stop, no_wait=args.no_wait, backup_root=args.backup_root)
    except SongLengthMissing as e:
        print(e)
        return 2


if __name__ == "__main__":
    sys.exit(main())
