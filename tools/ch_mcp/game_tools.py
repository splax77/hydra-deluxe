"""The Clone Hero MCP server's game-side tools: start and stop the game, read
its stats files, press drum lanes and menu keys, and run the auto-player.

Each tool is a thin call into the module that owns the work (the server
plan's "The tools" and "Who owns which question"; the plan is removed, read it
with git show 444631e6:docs/superpowers/plans/2026-10-28-ch-mcp-server.md).
This module adds only the session state the server keeps
between tool calls: the open bot_run.Run, the backup folder, the play thread
and its press rows, and one scratch folder per session.

Every tool returns plain text and never prints: an MCP server's stdout is its
protocol channel. The server (server.py) builds the one RLock and hands it in;
every tool holds it, so the tools never run at the same time.

The Frida tools (frida_tools.py) build on five methods here: ensure_backup,
launched_at, session_dir, add_stop_hook and play_snapshot.
"""

from __future__ import annotations

import ctypes
import dataclasses
import io
import json
import os
import tempfile
import threading
import time
from typing import Any, Callable, List, Optional, Sequence, Tuple, Union

from tools.ch_mcp import (
    bot_run, compare_scorestats, foreground, game_files, install_chart, quiet, recorder_link)
from tools.ch_mcp.frida_tools import READ_CAP_S
from tools.ch_probe import constants as C
from tools.ch_probe import engine_finder
from tools.ch_probe.experiments import play_chart
from tools.ch_probe.experiments.overlay_logs import PRESS_LOG_COLUMNS
from tools.ch_probe.input_driver import (
    MENU_KEY_GAP_S, MENU_KEYS, InputDriver, Lane, find_game_window)

# How long ch_install_chart waits for install_chart.install, whose silent
# audio comes from an ffmpeg call with no timeout of its own (the plan's
# question 5d).
INSTALL_CAP_S = 60.0

SCRATCH_PREFIX = "ch_live_"
# ch_run_chart's press logs, numbered in the session folder.
PRESS_LOG_PREFIX = "press-"
# The file under the temp root that names an open backup and the worker holding
# it, so a worker killed before its restore leaves the next worker a way back to
# the user's files (docs/superpowers/plans/2026-11-06-ch-server-reload.md, design
# point 3). read_marker reads it, _write_marker writes it.
OPEN_RUN_MARKER = "ch_open_run.json"

# What OpenProcess and GetExitCodeProcess answer for pid_running. Both are
# Windows API values, not project thresholds: ERROR_INVALID_PARAMETER is 87 in
# winerror.h (OpenProcess sets it for a pid no process has) and STILL_ACTIVE is
# 259 in Microsoft's GetExitCodeProcess page (the exit code of a process that
# has not ended).
_ERROR_INVALID_PARAMETER = 87
_STILL_ACTIVE = 259

# ch_play_chart's two routes: the virtual kit inside the game, or keys sent to
# the focused game window (the plan 2026-10-30-ch-mcp-in-game-input.md, "The
# tools").
ROUTE_KIT = "kit"
ROUTE_KEYS = "keys"
ROUTES = (ROUTE_KIT, ROUTE_KEYS)
# ch_play_chart's reply when no route is given and the kit is not set up. It
# names the kit setup and not the keys route, so no run takes the keyboard
# unasked (the user's answer to the plan's question 6, 2026-10-30, "Lock it
# too").
NO_ROUTE_NO_KIT = ("refused: no route given and the kit is not set up; "
                   "call ch_kit_setup first")
PLAY_ALREADY_RUNNING = "a play session is already running; call ch_play_stop first"

_NOT_RUNNING = f"{C.PROCESS_NAME} is not running"


def pid_running(pid: int) -> bool:
    """LIVE-ONLY: whether process `pid` still runs. os.kill cannot ask this,
    because on Windows it ends the process. Only a plain no is False: a wrong
    yes costs a by-hand restore, and a wrong no restores under a live run."""
    from ctypes import wintypes  # noqa: PLC0415  (Windows only)
    k32 = bot_run._kernel32()
    k32.GetExitCodeProcess.argtypes = (wintypes.HANDLE, ctypes.POINTER(wintypes.DWORD))
    handle = k32.OpenProcess(bot_run._PROCESS_QUERY_LIMITED_INFORMATION, False, pid)
    if not handle:
        return ctypes.get_last_error() != _ERROR_INVALID_PARAMETER
    try:
        code = wintypes.DWORD()
        if not k32.GetExitCodeProcess(handle, ctypes.byref(code)):
            return True
        return code.value == _STILL_ACTIVE
    finally:
        k32.CloseHandle(handle)


def age_text(seconds: float) -> str:
    """How old something is, as the largest whole unit: minutes, hours or days."""
    minutes = max(0, int(seconds // 60))
    if minutes < 60:
        return f"{minutes} minute{'' if minutes == 1 else 's'}"
    hours = minutes // 60
    if hours < 48:
        return f"{hours} hour{'' if hours == 1 else 's'}"
    return f"{hours // 24} days"


def read_marker(path: str) -> Tuple[int, str]:
    """The pid and the backup folder a marker file holds. Raises OSError when
    the file cannot be read and ValueError when it holds anything else."""
    with open(path, encoding="utf-8") as f:
        marker = json.load(f)
    pid = marker.get("pid") if isinstance(marker, dict) else None
    folder = marker.get("folder") if isinstance(marker, dict) else None
    if type(pid) is not int or pid <= 0 or not isinstance(folder, str) or not folder:
        raise ValueError("it holds no pid and backup folder")
    return pid, folder


def _key_name(vk: int) -> str:
    """A virtual-key code as the letter on the key when it is one."""
    return chr(vk) if ord("A") <= vk <= ord("Z") else f"0x{vk:02X}"


def _lane_list() -> str:
    return ", ".join(lane.name.lower() for lane in Lane)


def _value_text(value) -> str:
    return value if isinstance(value, str) else json.dumps(value, ensure_ascii=False)


# The prefix that marks a key item as text to type rather than a MENU_KEYS name.
TEXT_PREFIX = "text:"


def _menu_key_list() -> str:
    return ", ".join(MENU_KEYS) + f", and {TEXT_PREFIX}<string>"


def split_keys(keys: str) -> List[str]:
    """ch_key's comma list as press_keys items. A text item runs to the end of
    the string, so a comma inside it is typed, not split."""
    items: List[str] = []
    rest = keys
    while rest:
        if rest.lstrip().startswith(TEXT_PREFIX):
            items.append(rest.lstrip())
            break
        head, _sep, rest = rest.partition(",")
        if head.strip():
            items.append(head.strip())
    return items


def decline_refusal(decline_mode: Optional[str], dump_json: Optional[str]) -> Optional[str]:
    """The refusal for a tool's decline arguments, or None when they can run.
    The list of chords to decline comes from `dump_json`, so a mode needs it."""
    if decline_mode is None:
        return None
    if decline_mode not in play_chart.DECLINE_MODES:
        return (f'unknown decline mode "{decline_mode}"; '
                f'modes: {", ".join(play_chart.DECLINE_MODES)}')
    if dump_json is None:
        return f"decline mode {decline_mode} needs dump_json, the path's hydra_replay dump"
    return None


def run_cap_s(song_folder: Optional[str]) -> float:
    """The wall-time cap for a run of the song in `song_folder`:
    bot_run.run_cap_s of its length, or play_chart.RUN_CAP_S when no folder
    is given or the folder has no length (bot_run.song_length_ms)."""
    if song_folder is None:
        return play_chart.RUN_CAP_S
    try:
        length_ms, _source = bot_run.song_length_ms(song_folder)
    except bot_run.SongLengthMissing:
        return play_chart.RUN_CAP_S
    return bot_run.run_cap_s(length_ms)


def find_run_engine(proc) -> int:
    """LIVE-ONLY: ch_run_chart's one engine search, with its progress dots
    dropped, because the server's stdout is its protocol channel. It is the
    auto-player's search, not the watch's: sp_watch.find_engine_live scans the
    game's memory once for each window pattern, which the first press cannot
    wait for, and the patterns only it scans for are ones the auto-player
    never plays on. A run with no normal-mode engine (no song playing, or a
    precision-mode one) ends here as the search's timeout, so the reply says
    which engines were searched."""
    try:
        return play_chart.find_normal_engine(proc, out=io.StringIO())
    except engine_finder.EngineSearchTimeout as e:
        raise engine_finder.EngineSearchTimeout(
            f"{e} (ch_run_chart searched normal-mode engines only, as ch_play_chart does; "
            "a precision-mode engine is not found here)") from e


def first_press_line(rows: Sequence[dict]) -> str:
    """Which chord a run's press rows start at; `rows` holds at least one."""
    return f"first press chord index {rows[0]['index']} at {rows[0]['chart_ms']} ms"


def declined_line(decline_mode: str, declined: Sequence[int]) -> str:
    """Which chords a run declines."""
    return f"declined ({decline_mode}): chords {', '.join(map(str, declined)) or 'none'}"


class ChordsRefused(Exception):
    """A score file or decline arguments no run can start from; the text says
    why."""


@dataclasses.dataclass(frozen=True)
class RunParts:
    """The tools ch_run_chart chains, which each import this module:
    frida_tools.FridaTools, kit_tools.KitTools and sp_tools.SpTools."""
    frida: Any
    kit: Any
    sp: Any


@dataclasses.dataclass(frozen=True)
class PressResult:
    """What one press call did. `sent` is True only when the keys went out;
    a refusal (an unknown key or lane, no game window) leaves it False.
    `text` is the reply the tool prints either way."""
    sent: bool
    text: str


@dataclasses.dataclass
class _PlaySession:
    """One ch_play_chart run. The thread appends to `rows` through on_press;
    `result` is set when live_play returns."""
    score_json: str
    chord_count: int
    log_path: Optional[str]
    # The run cap handed to live_play; None leaves live_play's own.
    cap_s: Optional[float] = None
    # The kit press on the kit route; None presses keys through the driver.
    press: Optional[Callable] = None
    # The engine search handed to live_play; None leaves live_play's own.
    find_engine: Optional[Callable[[object], int]] = None
    thread: Optional[threading.Thread] = None
    stop: threading.Event = dataclasses.field(default_factory=threading.Event)
    # Set at the first press row, or when the run ends with none.
    pressed: threading.Event = dataclasses.field(default_factory=threading.Event)
    rows: list = dataclasses.field(default_factory=list)
    result: Optional[play_chart.RunResult] = None
    error: Optional[str] = None
    # What became of the press log: set by a failed write, and when the run
    # ends. While it holds a failure the rows stop going to the file.
    log_note: Optional[str] = None

    def running(self) -> bool:
        return self.thread is not None and self.thread.is_alive()


class GameTools:
    """The thirteen game-side tools and the session state between them.

    Every outside seam is an argument, so the tests swap each for a fake:
    `env` (bot_run.Env: the exe, the user folders, the launch, the clock and
    the stamp), `driver` (the input driver), `live_play` and `live_press`
    (play_chart's), `install` (install_chart's), `find_window` (input_driver's),
    `compare` (compare_scorestats'), `wait_thread` (how ch_install_chart waits
    on its worker), `set_setting` (game_files.set_game_setting),
    `make_watch` (builds the foreground.ForegroundWatch ch_start starts; None
    builds the real one on `find_window`), `make_recorder` (builds the
    recorder_link.RecorderLink ch_start starts), `is_pid_running` (pid_running)
    and `scratch_root` (the temp root the session folders and OPEN_RUN_MARKER go
    under). `lock` is the server's one RLock; this class never makes its own.

    Building one reads a marker a previous worker left (_read_marker_at_start)
    and restores nothing: the user's answer to the reload plan's question 9."""

    def __init__(self, env: bot_run.Env, lock, scratch_root: Optional[str] = None, *,
                 driver: Optional[InputDriver] = None,
                 live_play: Callable = play_chart.live_play,
                 live_press: Callable = play_chart.live_press,
                 install: Callable = install_chart.install,
                 find_window: Callable[[], int] = find_game_window,
                 compare: Callable = compare_scorestats.compare,
                 wait_thread: Callable[[threading.Thread, float], None] = bot_run.join_thread,
                 set_setting: Callable[[str, str, str], None] = game_files.set_game_setting,
                 make_watch: Optional[Callable[[], foreground.ForegroundWatch]] = None,
                 make_recorder: Callable[[], recorder_link.RecorderLink] = recorder_link.RecorderLink,
                 is_pid_running: Callable[[int], bool] = pid_running):
        self._env = env
        self._lock = lock
        self._scratch_root = scratch_root or tempfile.gettempdir()
        self._driver = driver if driver is not None else InputDriver()
        self._live_play = live_play
        self._live_press = live_press
        self._install = install
        self._find_window = find_window
        self._compare = compare
        self._wait_thread = wait_thread
        self._set_setting = set_setting
        self._make_watch = make_watch or (lambda: foreground.ForegroundWatch(
            self._find_window, foreground.foreground_window, time.monotonic, time.sleep))
        self._is_pid_running = is_pid_running

        self._run: Optional[bot_run.Run] = None
        # The foreground watch of the run the server started; None otherwise.
        self._watch: Optional[foreground.ForegroundWatch] = None
        self._recorder = make_recorder()
        # The backup taken for a game the server did not start (question 2).
        self._adopted_backup: Optional[str] = None
        self._scratch: Optional[str] = None
        self._count = 0
        self._last_stats: Optional[str] = None
        self._play: Optional[_PlaySession] = None
        self._press_log_count = 0
        self._stop_hooks: List[Callable[[], object]] = []
        # Where ch_play_chart gets the kit press: kit_tools.KitTools.kit_press,
        # handed in by use_kit_press. None until then.
        self._kit_press_source: Optional[Callable[[], Optional[Callable]]] = None
        # What ch_run_chart chains, handed in by use_run_parts. None until then.
        self._run_parts: Optional[RunParts] = None
        # The backup folder of the marker this worker wrote or adopted, the only
        # marker it may delete. A marker it did not take stays another's.
        self._marked: Optional[str] = None
        # True while `_adopted_backup` came from a marker. Nothing restores it
        # without a ch_stop: the user's answer to the reload plan's question 9.
        self._from_marker = False
        # What ch_status says about the marker found at start. The adoption
        # line goes once its backup is restored; the other outcomes stay.
        self._adoption_line: Optional[str] = None
        self._marker_line: Optional[str] = None
        self._read_marker_at_start()

    # -- the interface frida_tools builds on ------------------------------------

    def ensure_backup(self) -> str:
        """Take the backup for a game the server did not start, once. Returns
        nothing when a run or an earlier call already holds one; otherwise
        the line naming the new backup folder."""
        with self._lock:
            if self._run is not None or self._adopted_backup is not None:
                return ""
            scratch = self._new_scratch()
            self._adopted_backup = game_files.backup(
                self._env.documents_dir, self._env.locallow_dir, scratch, stamp=self._env.stamp,
                run_reg=self._env.run_reg)
            lines = [f"backup folder: {self._adopted_backup} (taken now, because the server "
                     "did not start this game; ch_stop restores it)"]
            self._write_marker(self._adopted_backup, lines)
            return "\n".join(lines)

    def launched_at(self) -> Optional[float]:
        """When the server launched the open run's game, on env.now; None when
        the server did not start the game."""
        with self._lock:
            return self._run.launched_at if self._run is not None else None

    def song_length_ms(self) -> Optional[int]:
        """The length ch_start printed for the open run's song, in ms; None
        with no run or no song on its command line."""
        with self._lock:
            return self._run.length_ms if self._run is not None else None

    def session_dir(self) -> str:
        """The session's scratch folder, made on the first call. It holds the
        backups, the stats copies, the play logs and the Frida logs."""
        with self._lock:
            return self._scratch if self._scratch is not None else self._new_scratch()

    def run_cap_s(self, song_folder: Optional[str]) -> float:
        """The module's run_cap_s, for callers that hold only this object."""
        return run_cap_s(song_folder)

    def open_lines(self) -> List[str]:
        """What the game side holds open, one line each: the run with its
        backup folder, an adopted backup, a running play session, a running
        machine recorder. The server's Helper.open_items adds the other tools'
        lines."""
        with self._lock:
            lines: List[str] = []
            if self._run is not None:
                lines.append(f"run: game pid {self._run.game.pid}, "
                             f"backup folder {self._run.backup_dir}")
            elif self._adopted_backup is not None:
                lines.append(f"adopted backup: {self._adopted_backup}")
            if self._play is not None and self._play.running():
                lines.append(f"play session: running, {len(self._play.rows)} presses so far")
            if self._recorder.running():
                lines.append(self._recorder.line())
            return lines

    def add_stop_hook(self, hook: Callable[[], object]) -> None:
        """Run `hook` at the start of ch_stop and shutdown, before the game is
        stopped, in the order the hooks were added. A hook may return a line
        for the reply."""
        with self._lock:
            self._stop_hooks.append(hook)

    def play_snapshot(self) -> Optional[dict]:
        """While a play session exists: {"last_row": the last press row or
        None, "press_count": the rows delivered so far, "stop": the run's stop
        reason, or None while it runs}. None when no play session ever ran."""
        with self._lock:
            play = self._play
            if play is None:
                return None
            rows = list(play.rows)
            return {"last_row": rows[-1] if rows else None,
                    "press_count": len(rows),
                    "stop": None if play.running() else self._stop_reason(play)}

    # -- starting, stopping and status --------------------------------------------

    def ch_start(self, song_folder: Optional[str] = None, player: str = bot_run.DEFAULT_PLAYER,
                 profile: Optional[str] = None, human: bool = False) -> str:
        """Check the machine is quiet (quiet.quiet_report), back up the user
        files, set settings.ini's pause_on_focus_lost
        line for the run (the backup puts the user's line back), then start
        the game minimized and start the foreground watch: with a song folder
        the bot plays it from the command line; with none the game opens at
        its menu. With `human` and a song folder a human player plays it
        instead (bot_run.launch_human), and `human` with no song folder is
        refused before anything runs. An open run is stopped and restored
        first, so its game is not read as load. A machine that is not quiet
        refuses the launch before any backup or folder, and a quiet one puts
        the report's lines first in the reply. The machine recorder starts
        after the check and before the launch, with no wait, and hears the
        game's pid once the launch is back; a launch that fails stops it."""
        with self._lock:
            refusal = bot_run.human_refusal(human, song_folder)
            if refusal is not None:
                return refusal
            lines: List[str] = []
            if self._from_marker:
                return (f"refused: {self._pending_text()}; call ch_stop to restore it, then "
                        "ch_start")
            if self._run is not None or self._adopted_backup is not None:
                lines.append("a run was open; stopping it first:")
                if not self._stop_all(lines):
                    lines.append("the new run was not started")
                    return "\n".join(lines)
            report = quiet.quiet_report(quiet.LIVE_ENV)
            if not report.quiet:
                lines.append(quiet.refusal_text(report))
                return "\n".join(lines)
            lines.extend(report.lines)
            scratch = self._new_scratch()
            in_scratch = set(os.listdir(scratch))
            env = self._env_into(lines)
            self._start_recorder(scratch, song_folder, lines)

            def before_launch() -> None:
                # start_run takes the backup and launches in one call, so this
                # is the one moment between them: the backup is whole, and
                # nothing of the user's has changed yet.
                self._mark_new_backup(scratch, in_scratch, lines)
                lines.append(self._pause_setting())

            try:
                try:
                    run = bot_run.start_run(song_folder, player=player, profile=profile,
                                            backup_root=scratch, env=env,
                                            before_launch=before_launch, human=human)
                except BaseException:
                    self._stop_recorder(lines)
                    self._settle_failed_start(lines)
                    raise
            except bot_run.SongLengthMissing as e:
                lines.append(str(e))
                return "\n".join(lines)
            except bot_run.LaunchFailed as e:
                lines.append(f"the human launch failed: {e}")
                return "\n".join(lines)
            except OSError as e:
                lines.append(f"could not start the game: {e}")
                return "\n".join(lines)
            self._run = run
            self._tell_recorder(f"pid {run.game.pid}", lines)
            self._watch = self._make_watch()
            self._watch.start()
            if run.length_ms is None:
                lines.append("song length: none (no song on the command line; pick one in "
                             "the game's menu)")
            else:
                lines.append(f"song length: {run.length_ms} ms, from {run.length_source}")
            return "\n".join(lines)

    def ch_stop(self) -> str:
        """Stop the machine recorder, run the stop hooks, stop the play
        session, then stop the game the server started (bot_run.finish_run)
        and put the user files back, one line per file. A game the server did
        not start is left running and only the files are restored."""
        with self._lock:
            lines: List[str] = []
            self._stop_all(lines)
            return "\n".join(lines)

    def shutdown(self) -> str:
        """The server's exit (question 3): the same steps as ch_stop. A game
        the server started is terminated, then the files are restored; for a
        game it did not start, the files are restored and the game stays. A
        backup adopted from a marker is left, with its marker, for a ch_stop."""
        with self._lock:
            lines: List[str] = []
            self._stop_all(lines, at_exit=True)
            return "\n".join(lines)

    def _pause_setting(self) -> str:
        """Set settings.ini's pause_on_focus_lost line to its run value and
        say so. A file without the line is left as it is, and the line says
        the game may pause while it is not in front."""
        path = os.path.join(self._env.documents_dir, game_files.SETTINGS)
        key, value = game_files.PAUSE_ON_FOCUS_LOST, game_files.PAUSE_ON_FOCUS_LOST_RUN_VALUE
        try:
            self._set_setting(path, key, value)
        except (game_files.SettingNotFound, OSError) as e:
            return (f"{game_files.SETTINGS}: {key} not set to {value} ({e}); the game may "
                    "pause while it is not in front")
        return f"{game_files.SETTINGS}: {key} set to {value} for this run (restored by ch_stop)"

    def ch_status(self) -> str:
        """The game side of the session: the run, the backup folder, a
        human launch's patch status (bot_run.request_launch_status), the
        scratch folder, the machine recorder, the play session and the last
        stats copy. The Frida and trace lines are frida_tools'."""
        with self._lock:
            lines: List[str] = []
            run = self._run
            if run is not None:
                game = run.game
                lines.append(f"game: started by the server, pid {game.pid}, "
                             f"{'running' if game.is_running() else 'not running'}")
                lines.append("song: " + (run.song_dir if run.song_dir is not None
                                         else "none (started at the menu)"))
                lines.append(f"backup folder: {run.backup_dir}")
                if run.launch is not None:
                    lines.append(f"player: human ({run.launch.form} launch)")
                    lines.extend(bot_run.request_launch_status(run.launch, READ_CAP_S))
            elif self._adopted_backup is not None:
                lines.append("game: not started by the server")
                lines.append(f"backup folder: {self._adopted_backup}")
                if self._adoption_line is not None:
                    lines.append(self._adoption_line)
            else:
                lines.append("game: no run open, and no backup taken")
            if self._marker_line is not None:
                lines.append(self._marker_line)
            watch_line = self.foreground_line()
            if watch_line is not None:
                lines.append(watch_line)
            if self._scratch is not None:
                lines.append(f"scratch folder: {self._scratch}")
            lines.append(self._recorder.line())
            play = self._play
            if play is None:
                lines.append("play session: none")
            elif play.running():
                lines.append(f"play session: running, {len(play.rows)} presses so far")
            else:
                lines.append(f"play session: ended ({self._stop_reason(play)})")
            lines.append(f"last stats copy: {self._last_stats or 'none'}")
            return "\n".join(lines)

    # -- the stats files ----------------------------------------------------------

    def ch_install_chart(self, chart_folder: str, name: Optional[str] = None) -> str:
        """Copy one chart folder into the probe song folder with a silent
        song.ogg and a marked title (install_chart.install), waiting at most
        INSTALL_CAP_S."""
        with self._lock:
            box: dict = {}

            def work() -> None:
                try:
                    box["chart"] = self._install(chart_folder, name=name)
                except BaseException as e:  # reported below
                    box["error"] = e

            worker = threading.Thread(target=work, name="ch_install_chart", daemon=True)
            worker.start()
            self._wait_thread(worker, INSTALL_CAP_S)
            if worker.is_alive():
                return (f"not finished after {INSTALL_CAP_S:.0f} s (INSTALL_CAP_S); the copy "
                        "goes on in the background and may still finish")
            error = box.get("error")
            if isinstance(error, install_chart.InstallRefused):
                return f"refused: {error}"
            if error is not None:
                return f"install failed: {type(error).__name__}: {error}"
            return f"installed; its chart file is {box['chart']}"

    def ch_wait_stats(self) -> str:
        """Wait, up to the run's cap, for the game to write new stats, and copy
        them into the session folder (bot_run.wait_stats)."""
        with self._lock:
            if self._run is None:
                return "no run is open; start one with ch_start(song_folder)"
            lines: List[str] = []
            copy = bot_run.wait_stats(self._run, self._scratch, self._env_into(lines))
            if copy is not None:
                self._last_stats = copy
            return "\n".join(lines)

    def ch_stats(self, path: Optional[str] = None) -> str:
        """Every field of a stats copy as `name: value` lines, in the file's
        order, each player's record under its own heading. No path means the
        last copy ch_wait_stats made."""
        with self._lock:
            path = path or self._last_stats
            if path is None:
                return "no stats copy yet; call ch_wait_stats, or pass a path"
            try:
                with open(path, encoding="utf-8-sig") as f:
                    stats = json.load(f)
            except (OSError, ValueError) as e:
                return f"could not read {path}: {e}"
            lines = [f"stats: {path}"]
            for key, value in stats.items():
                if key != "players":
                    lines.append(f"{key}: {_value_text(value)}")
            for i, player in enumerate(stats.get("players") or []):
                lines.append(f"players[{i}]:")
                lines.extend(f"  {key}: {_value_text(value)}" for key, value in player.items())
            return "\n".join(lines)

    def ch_compare(self, stats_path: str, score_json: str) -> str:
        """compare_scorestats.compare's six lines for a stats copy against a
        hydra_replay score JSON, then pass or fail."""
        with self._lock:
            lines: List[str] = []
            try:
                ok = self._compare(stats_path, score_json, out=lines.append)
            except (OSError, ValueError, KeyError, TypeError) as e:
                lines.append(f"compare failed: {type(e).__name__}: {e}")
                return "\n".join(lines)
            lines.append("pass" if ok else "fail")
            return "\n".join(lines)

    # -- keys and the auto-player ---------------------------------------------------

    def ch_press(self, lanes: Union[str, Sequence[str]]) -> str:
        """Press one chord: lane names such as "red,kick". press_lanes does
        the work and this prints its text."""
        return self.press_lanes(lanes).text

    def press_lanes(self, lanes: Union[str, Sequence[str]]) -> PressResult:
        """Press one chord of lane names, parsed through input_driver.Lane and
        pressed through play_chart.live_press, which focuses the game window
        first. The result says whether anything was sent, for menu_tools."""
        with self._lock:
            names = lanes.split(",") if isinstance(lanes, str) else list(lanes)
            names = [n.strip() for n in names if n.strip()]
            if not names:
                return PressResult(False, f"no lanes given; valid lanes: {_lane_list()}")
            chord: List[Lane] = []
            for name in names:
                try:
                    chord.append(Lane[name.upper()])
                except KeyError:
                    return PressResult(False, f'unknown lane "{name}"; valid lanes: {_lane_list()}')
            hwnd = self._find_window()
            if not hwnd:
                return PressResult(False, f"no game window found ({_NOT_RUNNING}, or its window is closed)")
            lines = [line for line in (self.ensure_backup(),) if line]
            keys = self._live_press(self._driver, hwnd)(chord)
            lines.append("pressed " + ", ".join(lane.name.lower() for lane in chord)
                         + "; keys sent: " + (", ".join(_key_name(vk) for vk in keys) or "none"))
            return PressResult(True, "\n".join(lines))

    def ch_key(self, keys: str) -> str:
        """Press menu keys in order: a comma list of input_driver.MENU_KEYS
        names, with an optional last item text:<string> that types the string
        (split_keys says how the list is read). press_keys does the work."""
        return self.press_keys(split_keys(keys)).text

    def press_keys(self, items: Sequence[str]) -> PressResult:
        """Press `items` in order, each a MENU_KEYS name or text:<string>,
        with input_driver.MENU_KEY_GAP_S between two keys and between two
        typed characters. Every item is checked before anything is sent; then
        the backup is taken, the game window is focused once (the focus
        play_chart.live_press uses) and the keys go out. The result says
        whether anything was sent, for menu_tools."""
        with self._lock:
            steps: List[Union[int, str]] = []
            for item in items:
                if item.startswith(TEXT_PREFIX):
                    text = item[len(TEXT_PREFIX):]
                    if not text:
                        return PressResult(False, f'"{item}" has no text to type; '
                                                  f"valid keys: {_menu_key_list()}")
                    steps.append(text)
                    continue
                vk = MENU_KEYS.get(item.strip().lower())
                if vk is None:
                    return PressResult(False, f'unknown key "{item}"; valid keys: {_menu_key_list()}')
                steps.append(vk)
            if not steps:
                return PressResult(False, f"no keys given; valid keys: {_menu_key_list()}")
            hwnd = self._find_window()
            if not hwnd:
                return PressResult(False, f"no game window found ({_NOT_RUNNING}, or its window is closed)")
            lines = [line for line in (self.ensure_backup(),) if line]
            play_chart.focus_window(hwnd)
            sent: List[str] = []
            for i, step in enumerate(steps):
                if i:
                    self._env.sleep(MENU_KEY_GAP_S)
                if isinstance(step, str):
                    count = self._driver.type_text(step, gap_s=MENU_KEY_GAP_S,
                                                   sleep=self._env.sleep)
                    sent.append(f'text "{step}" ({count} characters)')
                else:
                    self._driver.press_key(step, sleep=self._env.sleep)
                    sent.append(next(name for name, vk in MENU_KEYS.items() if vk == step))
            lines.append("keys sent: " + ", ".join(sent))
            return PressResult(True, "\n".join(lines))

    def use_kit_press(self, source: Callable[[], Optional[Callable]]) -> None:
        """Hand in where the kit press comes from (kit_tools.KitTools.kit_press:
        the press, or None while the kit is not set up)."""
        with self._lock:
            self._kit_press_source = source

    def use_run_parts(self, parts: RunParts) -> None:
        """Hand in the tools ch_run_chart chains."""
        with self._lock:
            self._run_parts = parts

    def foreground_line(self) -> Optional[str]:
        """The foreground watch's count line, or None when no watch runs
        (the server did not start the game)."""
        with self._lock:
            return self._watch.line() if self._watch is not None else None

    def ch_play_chart(self, score_json: str, log: Optional[str] = None,
                      song_folder: Optional[str] = None, route: Optional[str] = None,
                      decline_mode: Optional[str] = None, dump_json: Optional[str] = None,
                      path_index: int = 0) -> str:
        """Start the auto-player (play_chart.live_play) on a hydra_replay score
        JSON in a background thread. `log`, when given, is where the press log
        is written, row by row as the run goes; a relative path goes in the
        session folder. `song_folder`, when given, sets the run cap through
        run_cap_s; with none the run keeps live_play's own cap. `route` is ROUTE_KIT
        (presses through the virtual kit inside the game) or ROUTE_KEYS (keys
        to the focused window); None means the kit, and is refused until the
        kit is set up, so no run takes the keyboard unless asked to.
        `decline_mode`, one of play_chart.DECLINE_MODES, says what to send for
        each fill path `path_index` of the dump file `dump_json` passes over
        (play_chart.declined_chord_indexes); None presses every chord."""
        with self._lock:
            if self._play is not None and self._play.running():
                return PLAY_ALREADY_RUNNING
            kit_press = self._kit_press_source() if self._kit_press_source is not None else None
            if route is None:
                if kit_press is None:
                    return NO_ROUTE_NO_KIT
                route = ROUTE_KIT
            if route not in ROUTES:
                return f'unknown route "{route}"; routes: {", ".join(ROUTES)}'
            if route == ROUTE_KIT and kit_press is None:
                return (f"route {ROUTE_KIT}: the kit is not set up; call ch_kit_setup first, "
                        f'or pass route="{ROUTE_KEYS}"')
            press = kit_press if route == ROUTE_KIT else None
            try:
                chords, declined = self._chords_to_play(score_json, decline_mode, dump_json,
                                                        path_index)
            except ChordsRefused as e:
                return str(e)
            lines = [line for line in (self.ensure_backup(),) if line]
            log_path = None
            if log:
                log_path = log if os.path.isabs(log) else os.path.join(self.session_dir(), log)
            cap_s = None if song_folder is None else run_cap_s(song_folder)
            self._start_play(_PlaySession(score_json=score_json, chord_count=len(chords),
                                          log_path=log_path, cap_s=cap_s, press=press), chords)
            lines.append(f"play session started: {len(chords)} chords from {score_json}")
            lines.append(f"route: {route} ("
                         + ("presses through the virtual kit inside the game"
                            if route == ROUTE_KIT else "keys to the focused game window") + ")")
            lines.append(f"run cap: {play_chart.RUN_CAP_S if cap_s is None else cap_s:g} s")
            if decline_mode is not None:
                lines.append(declined_line(decline_mode, declined))
            lines.append(f"waiting for a moving clock (cap {engine_finder.ENGINE_SEARCH_CAP_S:.0f} s): "
                         "start the song from the game's menu; a song launched with a bot "
                         "is played by the bot, not by keys")
            return "\n".join(lines)

    def ch_run_chart(self, song_folder: str, score_json: str, kit_player: Optional[str] = None,
                     decline_mode: Optional[str] = None,
                     dump_json: Optional[str] = None) -> str:
        """Start a run in one call: attach, kit setup on `kit_player`
        (ch_kit_setup's `player`), the Star Power watch and the auto-player on
        the kit, with one engine search that the watch and the player share.
        `decline_mode` and `dump_json` are ch_play_chart's. Each part keeps
        its owner, and this adds no wait of its own before the first press:
        the arguments are read before the attach, and the player starts the
        moment the search ends. The call returns at the first press. The
        press rows go to a press-<n>.csv in the session folder as they happen.

        The machine recorder hears the engine's address right after the watch
        step. The reply has one line per part with the time it took, the
        recorder's samples line after the watch's, the press log's line, then
        first_press_line. A part that fails stops the call
        there: the reply names it and gives its own reply, and a kit that part left standing
        is torn down. A failed attach built no kit, and a failed setup tears
        down its own."""
        with self._lock:
            parts = self._run_parts
            if parts is None:
                return "refused: the server did not hand in the run's parts (use_run_parts)"
            if self._play is not None and self._play.running():
                return PLAY_ALREADY_RUNNING
            try:
                chords, declined = self._chords_to_play(score_json, decline_mode, dump_json)
                cap_s = run_cap_s(song_folder)
            except ChordsRefused as e:
                return str(e)
            except (OSError, ValueError) as e:
                return f"could not pick the run cap for {song_folder}: {type(e).__name__}: {e}"
            lines: List[str] = []

            began = self._env.now()
            text = parts.frida.ch_attach()
            if not parts.frida.is_attached():
                return self._run_stopped(lines, "attach", began, text)
            lines.append(f"attach: ok, {self._seconds_since(began)}")

            began = self._env.now()
            press, text = parts.kit.setup_for_run(kit_player)
            if press is None:
                return self._run_stopped(lines, "kit setup", began, text)
            lines.append(f"kit setup: ok, {self._seconds_since(began)}")

            began = self._env.now()
            engine, text = parts.sp.watch_for_run(cap_s, find_run_engine, song_folder)
            if engine is None:
                return self._run_stopped(lines, "watch", began, text,
                                         parts.kit.ch_kit_teardown())
            lines.append(f"watch: ok, {self._seconds_since(began)}, engine {engine:#x}, {text}")
            self._tell_recorder(f"engine {engine:#x}", lines)
            samples = self._recorder.samples_line()
            if samples is not None:
                lines.append(samples)

            began = self._env.now()
            self._press_log_count += 1
            log_path = os.path.join(self.session_dir(),
                                    f"{PRESS_LOG_PREFIX}{self._press_log_count}.csv")
            play = _PlaySession(score_json=score_json, chord_count=len(chords), log_path=log_path,
                                cap_s=cap_s, press=press, find_engine=lambda _proc: engine)
            self._start_play(play, chords)
            first_press_in_time = play.pressed.wait(cap_s)
            rows = list(play.rows)
            if not rows:
                play.stop.set()
                self._wait_thread(play.thread, READ_CAP_S)
                said = self._play_lines(play)
                if not first_press_in_time:
                    said.insert(0, f"no press inside the run cap of {cap_s:g} s; the call "
                                   "stopped the run")
                # The watch goes with the kit, so a second ch_run_chart is not
                # refused for a watch still running.
                return self._run_stopped(lines, "play", began, "\n".join(said),
                                         parts.kit.ch_kit_teardown() + "\n"
                                         + parts.sp.ch_sp_watch_stop())
            lines.append(f"play: ok, {self._seconds_since(began)} to the first press, "
                         f"{len(chords)} chords from {score_json}, run cap {cap_s:g} s")
            if decline_mode is not None:
                lines.append(declined_line(decline_mode, declined))
            lines.append(self._press_log_line(play))
            lines.append(first_press_line(rows))
            return "\n".join(lines)

    def ch_play_status(self) -> str:
        """The press log's path when the session has one, the play session's
        counts, its stop reason when it ended, and every press row, newest
        first. The server cuts a long reply from its end, so the file is what
        holds every row."""
        with self._lock:
            play = self._play
            if play is None:
                return "no play session has run"
            return "\n".join(self._play_lines(play))

    def ch_play_stop(self) -> str:
        """Tell the play session to stop at its next clock read, then show the
        result so far."""
        with self._lock:
            play = self._play
            if play is None or not play.running():
                return "no play session is running" + (
                    "\n" + "\n".join(self._play_lines(play)) if play is not None else "")
            play.stop.set()
            return "\n".join(self._play_lines(play) + ["stopped"])

    # -- inside ---------------------------------------------------------------------

    @staticmethod
    def _chords_to_play(score_json: str, decline_mode: Optional[str], dump_json: Optional[str],
                        path_index: int = 0) -> Tuple[List[play_chart.Chord], List[int]]:
        """The chords of `score_json` with the decline arguments applied, and
        the indexes declined. Raises ChordsRefused saying why no run can
        start from them."""
        refusal = decline_refusal(decline_mode, dump_json)
        if refusal is not None:
            raise ChordsRefused(refusal)
        declined: List[int] = []
        if decline_mode is not None:
            try:
                declined = play_chart.declined_chord_indexes(score_json, dump_json, path_index)
            except (OSError, ValueError, KeyError, IndexError, TypeError) as e:
                raise ChordsRefused(
                    f"could not read the fills to decline: {type(e).__name__}: {e}") from e
        try:
            chords = play_chart.load_dump_chords(score_json, dict.fromkeys(declined, decline_mode))
        except (OSError, ValueError, KeyError, TypeError) as e:
            raise ChordsRefused(f"could not read {score_json}: {type(e).__name__}: {e}") from e
        if not chords:
            raise ChordsRefused(f"{score_json} has no chords to play")
        return chords, declined

    def _start_play(self, play: _PlaySession, chords: Sequence[play_chart.Chord]) -> None:
        """Make `play` the session and start its thread."""
        play.thread = threading.Thread(target=self._play_body, args=(play, chords),
                                       name="ch_play_chart", daemon=True)
        self._play = play
        play.thread.start()

    def _seconds_since(self, began: float) -> str:
        """How long a part of ch_run_chart took, on env.now."""
        return f"{self._env.now() - began:.2f} s"

    def _run_stopped(self, lines: List[str], part: str, began: float, text: str,
                     teardown: Optional[str] = None) -> str:
        """ch_run_chart's reply when `part` failed: the parts that passed,
        the line naming it, its own reply, then the teardown's lines."""
        lines = lines + [f"{part}: FAILED after {self._seconds_since(began)}; "
                         f"the run stopped at {part}", text]
        if teardown is not None:
            lines.append(teardown)
        return "\n".join(lines)

    def _new_scratch(self) -> str:
        """A new session folder under the temp root. It is never deleted: it
        holds the backups."""
        self._count += 1
        self._scratch = os.path.join(self._scratch_root,
                                     f"{SCRATCH_PREFIX}{os.getpid()}_{self._count}")
        os.makedirs(self._scratch, exist_ok=True)
        return self._scratch

    def _env_into(self, lines: List[str]) -> bot_run.Env:
        """The server's Env with its printed lines going into `lines`."""
        return dataclasses.replace(self._env, out=lines.append)

    # -- the machine recorder ---------------------------------------------------------

    def _start_recorder(self, scratch: str, song_folder: Optional[str],
                        lines: List[str]) -> None:
        """Start the recorder, with no wait, and name the pids the report needs
        to find the worker and the py launcher behind it. A recorder that does
        not start is one line, and the run goes on."""
        try:
            lines.extend(self._recorder.start(scratch, os.getpid(), run_cap_s(song_folder)))
        except Exception as e:  # a run never fails for its recording
            lines.append(recorder_link.start_failed_line(e))
        lines.append(f"worker pid {os.getpid()}, its parent pid {os.getppid()}")

    def _tell_recorder(self, line: str, lines: List[str]) -> None:
        """Send `line` to the recorder; a failed write adds one line to the reply."""
        said = self._recorder.send(line)
        if said is not None:
            lines.append(said)

    def _stop_recorder(self, lines: List[str]) -> None:
        """Stop the recorder, adding its line when there was one to stop."""
        try:
            said = self._recorder.stop()
        except Exception as e:  # the restore after this matters more
            said = f"stopping the machine recorder failed: {type(e).__name__}: {e}"
        if said is not None:
            lines.append(said)

    # -- the open-run marker ----------------------------------------------------------

    def _marker_path(self) -> str:
        return os.path.join(self._scratch_root, OPEN_RUN_MARKER)

    def _read_marker_at_start(self) -> None:
        """Act on a marker a previous worker left, saying what happened on
        env.out and in ch_status. Nothing in the file may stop this worker
        starting, so anything that goes wrong here leaves it in place."""
        path = self._marker_path()
        try:
            pid, folder = read_marker(path)
            self._settle_found_marker(path, pid, folder)
        except FileNotFoundError:
            return
        except Exception as e:  # the worker must start whatever the file holds
            self._marker_line = (f"open-run marker {path} was left in place and nothing was "
                                 f"adopted: {type(e).__name__}: {e}")
        said = self._adoption_line or self._marker_line
        if said is not None:
            self._env.out(said)

    def _settle_found_marker(self, path: str, pid: int, folder: str) -> None:
        """Design point 3's three outcomes for a marker holding `pid` and
        `folder`, and the one it leaves out: a folder restore_only cannot read
        would make every ch_stop fail, so it is not adopted."""
        if not os.path.isdir(folder):
            os.remove(path)
            self._marker_line = (f"a previous worker (pid {pid}) left a marker for a backup "
                                 f"that is gone: {folder}; the marker was deleted")
        elif self._is_pid_running(pid):
            self._marker_line = (f"process {pid} was running when this worker started, and its "
                                 f"marker holds a backup open: {folder}; it was not adopted, "
                                 "because that may be a second session's worker with a live "
                                 "run; if it is not, restore by hand with "
                                 f"{bot_run._restore_hint(folder)} and delete {path}")
        elif not os.path.isfile(os.path.join(folder, game_files.BACKUP_MANIFEST)):
            self._marker_line = (f"a previous worker (pid {pid}) left a marker, but {folder} "
                                 f"holds no {game_files.BACKUP_MANIFEST}; it was not adopted "
                                 f"and {path} was left in place")
        else:
            age = age_text(time.time() - os.path.getmtime(folder))
            restored = [n for n in game_files.DOCUMENTS_FILES + game_files.LOCALLOW_FILES
                        if n not in game_files.KEPT_FILES]
            self._adoption_line = (
                f"a previous worker (pid {pid}) left a backup open: {folder}, {age} old; "
                f"ch_stop restores {', '.join(restored)} and the Rewired key from it and "
                f"leaves {', '.join(game_files.KEPT_FILES)} as they are")
            self._adopted_backup = self._marked = folder
            self._from_marker = True

    def _pending_text(self) -> str:
        """What ch_start says about the backup adopted from a marker: its
        folder and its age from the folder's own time."""
        folder = self._adopted_backup
        try:
            age = f"{age_text(time.time() - os.path.getmtime(folder))} old"
        except OSError:
            age = "age unknown"
        return f"a previous worker left a backup open: {folder}, {age}"

    def _write_marker(self, folder: str, lines: List[str]) -> None:
        """Name `folder` in the marker, or add a line saying this backup has
        none. A marker already there is never replaced: it is another worker's
        open backup, or a file left for the user to read."""
        path = self._marker_path()
        temp = f"{path}.{os.getpid()}.tmp"
        try:
            with open(temp, "w", encoding="utf-8") as f:
                json.dump({"pid": os.getpid(), "folder": folder}, f)
            # On Windows os.rename refuses an existing target, so the check and
            # the write are one step, and no reader sees half a file.
            os.rename(temp, path)
        except OSError as e:
            try:
                os.remove(temp)
            except OSError:
                pass  # a stray temp file holds nothing a worker reads
            lines.append(f"open-run marker not written ({type(e).__name__}: {e}); if this worker "
                         f"is killed, restore by hand with {bot_run._restore_hint(folder)}")
            return
        self._marked = folder

    def _mark_new_backup(self, scratch: str, in_scratch: set, lines: List[str]) -> None:
        """Write the marker for the backup start_run just took in `scratch`,
        the one folder of its kind that `in_scratch` did not list."""
        name = next(n for n in os.listdir(scratch)
                    if n not in in_scratch and n.startswith(game_files.BACKUP_PREFIX))
        self._write_marker(os.path.join(scratch, name), lines)

    def _delete_marker(self, lines: List[str]) -> None:
        """Delete the marker this worker holds, after its backup was restored.
        A file that stays is no longer this worker's to touch, and the line
        says so, because the next worker would restore that old backup."""
        if self._marked is None:
            return
        folder, self._marked = self._marked, None
        path = self._marker_path()
        try:
            os.remove(path)
        except FileNotFoundError:
            pass
        except OSError as e:
            lines.append(f"open-run marker {path} was not deleted ({e}); delete it by hand, or "
                         f"the next worker adopts {folder}, which is already restored")

    def _settle_failed_start(self, lines: List[str]) -> None:
        """After start_run raised. It restores on the way out but does not say
        how that went, and the marker may go only after a restore that worked,
        so restore_only asks: restoring files already back changes nothing. A
        restore that fails keeps the marker and holds the backup as adopted,
        so ch_stop tries again and no new run takes a backup of the run's
        files."""
        folder = self._marked
        if folder is None:
            return
        said: List[str] = []
        try:
            restored = bot_run.restore_only(folder, self._env_into(said)) == 0
        except Exception as e:  # the failed start's own error goes on after this
            restored = False
            said.append(f"restoring failed: {type(e).__name__}: {e}")
        if restored:
            self._delete_marker(lines)
            return
        self._adopted_backup = folder
        lines.extend(said)
        lines.append(f"the user files may not be back; ch_stop tries again from {folder}")

    def _stop_all(self, lines: List[str], at_exit: bool = False) -> bool:
        """The recorder's stop, the stop hooks, the play stop, the foreground
        watch's stop and its count, then the game and the restore. Returns
        False when the restore failed or the stop raised; the run stays open
        then, so a later ch_stop tries the restore again. At the worker's exit
        a backup adopted from a marker is left alone, with its marker."""
        self._stop_recorder(lines)
        for hook in list(self._stop_hooks):
            try:
                said = hook()
            except Exception as e:  # the restore below matters more
                lines.append(f"a stop hook failed: {type(e).__name__}: {e}")
            else:
                if said:
                    lines.append(str(said))
        play = self._play
        if play is not None and play.running():
            play.stop.set()
            lines.append("play session told to stop")
        if self._watch is not None:
            self._watch.stop()
            lines.append(self._watch.line())
        env = self._env_into(lines)
        if self._run is not None and self._run.launch is not None:
            # The launch agent goes before the game does (the direct-start
            # plan's "The launch").
            try:
                lines.append("launch agent: " + self._run.launch.agent.detach())
            except Exception as e:  # the restore below matters more
                lines.append(f"detaching the launch agent failed: {type(e).__name__}: {e}")
        if self._run is not None:
            try:
                ok = bot_run.finish_run(self._run, stop=True, env=env,
                                        stop_line=bot_run.STOPPED_LINE)
            except Exception as e:
                lines.append(f"stopping the game failed: {type(e).__name__}: {e}; "
                             f"the backup is still in {self._run.backup_dir}")
                return False
            if not ok:
                return False
            self._run = None
            self._watch = None
            self._delete_marker(lines)
            return True
        if self._adopted_backup is not None:
            if self._from_marker and at_exit:
                lines.append(f"{self._pending_text()}; it and its marker {self._marker_path()} "
                             "stay, and ch_stop in the next session restores it")
                return True
            lines.append("the server did not start the game, so it does not close it")
            leave = game_files.KEPT_FILES if self._from_marker else ()
            try:
                if bot_run.restore_only(self._adopted_backup, env, leave) != 0:
                    return False
            except Exception as e:
                lines.append(f"restoring failed: {type(e).__name__}: {e}; "
                             f"the backup is still in {self._adopted_backup}")
                return False
            lines.append("if the game is still running it "
                         + bot_run.late_write_hint(self._adopted_backup))
            self._adopted_backup = None
            self._adoption_line = None
            self._from_marker = False
            self._delete_marker(lines)
            return True
        lines.append("no run is open and no backup was taken; nothing to restore")
        return True

    def _play_body(self, play: _PlaySession, chords) -> None:
        """The play thread: live_play, with each press row added to the press
        log as it is made. It never takes the lock, so a tool holding it never
        waits on the thread. A log that fails stops being written and the run
        goes on, because an error raised out of on_press would end the run."""
        cap = {} if play.cap_s is None else {"cap_s": play.cap_s}
        search = {} if play.find_engine is None else {"find_engine": play.find_engine}
        # The kit route hands live_play its press and no driver, so no window
        # is looked up; the keys route keeps the driver.
        route = {"driver": self._driver} if play.press is None else {"press": play.press}

        def log_failed(e: OSError) -> None:
            play.log_note = f"press log not written to {play.log_path}: {e}"

        def on_press(row: dict) -> None:
            play.rows.append(row)
            if play.log_path and play.log_note is None:
                try:
                    play_chart.append_press_row(play.log_path, row)
                except OSError as e:
                    log_failed(e)
            play.pressed.set()

        if play.log_path:
            # Starts the file over, so a reused log name holds this run's rows only.
            try:
                play_chart.write_press_log(play.log_path, [])
            except OSError as e:
                log_failed(e)
        try:
            play.result = self._live_play(chords, **route,
                                          on_press=on_press,
                                          should_stop=play.stop.is_set,
                                          out=io.StringIO(), **cap, **search)
        except Exception as e:  # reported by ch_play_status
            play.error = f"{type(e).__name__}: {e}"
        if play.log_path:
            try:
                # The rows are already in the file unless a write failed; then
                # the one whole write replaces what it holds.
                if play.log_note is not None:
                    play_chart.write_press_log(play.log_path, list(play.rows))
                play.log_note = f"press log: {play.log_path} ({len(play.rows)} rows)"
            except OSError as e:
                log_failed(e)
        play.pressed.set()

    @staticmethod
    def _stop_reason(play: _PlaySession) -> str:
        if play.error is not None:
            return f"failed: {play.error}"
        return play.result.stop if play.result is not None else "no result"

    @staticmethod
    def _press_log_line(play: _PlaySession) -> str:
        """The press log's line: what became of it, else its path."""
        return play.log_note or f"press log: {play.log_path}"

    def _play_lines(self, play: _PlaySession) -> List[str]:
        rows = list(play.rows)
        lines = [self._press_log_line(play)] if play.log_path else []
        lines.append(f"play session: {play.chord_count} chords from {play.score_json}")
        if play.running():
            lines.append(f"running: {len(rows)} presses so far, "
                         f"{sum(1 for r in rows if r['hit'] == 1)} hit")
        else:
            lines.append(f"ended: {self._stop_reason(play)}")
            result = play.result
            if result is not None:
                if result.dyn_threshold is not None:
                    lines.append(f"dynamics threshold: {result.dyn_threshold}")
                if result.message:
                    lines.append(result.message)
                lines.append(f"sent {result.sent}, hit {result.hit}, "
                             f"misses {result.sent - result.hit}, best streak {result.max_streak}"
                             + (f", declined {result.declined}" if result.declined else ""))
                if result.sent + result.declined != len(rows):
                    lines.append(f"the song clock jumped back during the run: the counts above "
                                 f"cover the part after the jump; the rows below keep all "
                                 f"{len(rows)} presses")
        if not rows:
            lines.append("first press: none, no chord was pressed")
        else:
            lines.append(first_press_line(rows))
            # The kit press's columns show only when the rows carry them.
            columns = [c for c in PRESS_LOG_COLUMNS if any(c in r for r in rows)]
            lines.append(f"rows, newest first ({', '.join(columns)}):")
            lines.extend("  " + ", ".join(str(r.get(c, "")) for c in columns)
                         for r in reversed(rows))
        return lines
