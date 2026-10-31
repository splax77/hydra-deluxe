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

import dataclasses
import io
import json
import os
import tempfile
import threading
import time
from typing import Callable, List, Optional, Sequence, Union

from tools.ch_mcp import bot_run, compare_scorestats, foreground, game_files, install_chart
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

_NOT_RUNNING = f"{C.PROCESS_NAME} is not running"


def _join(thread: threading.Thread, cap_s: float) -> None:
    thread.join(cap_s)


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
    thread: Optional[threading.Thread] = None
    stop: threading.Event = dataclasses.field(default_factory=threading.Event)
    rows: list = dataclasses.field(default_factory=list)
    result: Optional[play_chart.RunResult] = None
    error: Optional[str] = None
    log_note: Optional[str] = None

    def running(self) -> bool:
        return self.thread is not None and self.thread.is_alive()


class GameTools:
    """The twelve game-side tools and the session state between them.

    Every outside seam is an argument, so the tests swap each for a fake:
    `env` (bot_run.Env: the exe, the user folders, the launch, the clock and
    the stamp), `driver` (the input driver), `live_play` and `live_press`
    (play_chart's), `install` (install_chart's), `find_window` (input_driver's),
    `compare` (compare_scorestats'), `wait_thread` (how ch_install_chart waits
    on its worker), `set_setting` (game_files.set_game_setting),
    `make_watch` (builds the foreground.ForegroundWatch ch_start starts; None
    builds the real one on `find_window`) and `scratch_root` (the temp root
    the session folders go under). `lock` is the server's one RLock; this
    class never makes its own."""

    def __init__(self, env: bot_run.Env, lock, scratch_root: Optional[str] = None, *,
                 driver: Optional[InputDriver] = None,
                 live_play: Callable = play_chart.live_play,
                 live_press: Callable = play_chart.live_press,
                 install: Callable = install_chart.install,
                 find_window: Callable[[], int] = find_game_window,
                 compare: Callable = compare_scorestats.compare,
                 wait_thread: Callable[[threading.Thread, float], None] = _join,
                 set_setting: Callable[[str, str, str], None] = game_files.set_game_setting,
                 make_watch: Optional[Callable[[], foreground.ForegroundWatch]] = None):
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

        self._run: Optional[bot_run.Run] = None
        # The foreground watch of the run the server started; None otherwise.
        self._watch: Optional[foreground.ForegroundWatch] = None
        # The backup taken for a game the server did not start (question 2).
        self._adopted_backup: Optional[str] = None
        self._scratch: Optional[str] = None
        self._count = 0
        self._last_stats: Optional[str] = None
        self._play: Optional[_PlaySession] = None
        self._stop_hooks: List[Callable[[], object]] = []
        # Where ch_play_chart gets the kit press: kit_tools.KitTools.kit_press,
        # handed in by use_kit_press. None until then.
        self._kit_press_source: Optional[Callable[[], Optional[Callable]]] = None

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
            return (f"backup folder: {self._adopted_backup} (taken now, because the server "
                    "did not start this game; ch_stop restores it)")

    def launched_at(self) -> Optional[float]:
        """When the server launched the open run's game, on env.now; None when
        the server did not start the game."""
        with self._lock:
            return self._run.launched_at if self._run is not None else None

    def session_dir(self) -> str:
        """The session's scratch folder, made on the first call. It holds the
        backups, the stats copies, the play logs and the Frida logs."""
        with self._lock:
            return self._scratch if self._scratch is not None else self._new_scratch()

    def run_cap_s(self, song_folder: Optional[str]) -> float:
        """The module's run_cap_s, for callers that hold only this object."""
        return run_cap_s(song_folder)

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
                 profile: Optional[str] = None) -> str:
        """Back up the user files, set settings.ini's pause_on_focus_lost
        line for the run (the backup puts the user's line back), then start
        the game minimized and start the foreground watch: with a song folder
        the bot plays it from the command line; with none the game opens at
        its menu. An open run is stopped and restored first."""
        with self._lock:
            lines: List[str] = []
            if self._run is not None or self._adopted_backup is not None:
                lines.append("a run was open; stopping it first:")
                if not self._stop_all(lines):
                    lines.append("the new run was not started")
                    return "\n".join(lines)
            scratch = self._new_scratch()
            env = self._env_into(lines)
            try:
                run = bot_run.start_run(song_folder, player=player, profile=profile,
                                        backup_root=scratch, env=env,
                                        before_launch=lambda: lines.append(self._pause_setting()))
            except bot_run.SongLengthMissing as e:
                lines.append(str(e))
                return "\n".join(lines)
            except OSError as e:
                lines.append(f"could not start the game: {e}")
                return "\n".join(lines)
            self._run = run
            self._watch = self._make_watch()
            self._watch.start()
            if run.length_ms is None:
                lines.append("song length: none (no song on the command line; pick one in "
                             "the game's menu)")
            else:
                lines.append(f"song length: {run.length_ms} ms, from {run.length_source}")
            return "\n".join(lines)

    def ch_stop(self) -> str:
        """Run the stop hooks, stop the play session, then stop the game the
        server started (bot_run.finish_run) and put the user files back, one
        line per file. A game the server did not start is left running and
        only the files are restored."""
        with self._lock:
            lines: List[str] = []
            self._stop_all(lines)
            return "\n".join(lines)

    def shutdown(self) -> str:
        """The server's exit (question 3): the same steps as ch_stop. A game
        the server started is terminated, then the files are restored; for a
        game it did not start, the files are restored and the game stays."""
        return self.ch_stop()

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
        """The game side of the session: the run, the backup folder, the
        scratch folder, the play session and the last stats copy. The Frida
        and trace lines are frida_tools'."""
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
            elif self._adopted_backup is not None:
                lines.append("game: not started by the server")
                lines.append(f"backup folder: {self._adopted_backup}")
            else:
                lines.append("game: no run open, and no backup taken")
            watch_line = self.foreground_line()
            if watch_line is not None:
                lines.append(watch_line)
            if self._scratch is not None:
                lines.append(f"scratch folder: {self._scratch}")
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
        song.ogg (install_chart.install), waiting at most INSTALL_CAP_S."""
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
        """compare_scorestats.compare's three lines for a stats copy against a
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

    def foreground_line(self) -> Optional[str]:
        """The foreground watch's count line, or None when no watch runs
        (the server did not start the game)."""
        with self._lock:
            return self._watch.line() if self._watch is not None else None

    def ch_play_chart(self, score_json: str, log: Optional[str] = None,
                      song_folder: Optional[str] = None, route: Optional[str] = None) -> str:
        """Start the auto-player (play_chart.live_play) on a hydra_replay score
        JSON in a background thread. `log`, when given, is where the press log
        is written when the run ends; a relative path goes in the session
        folder. `song_folder`, when given, sets the run cap through run_cap_s;
        with none the run keeps live_play's own cap. `route` is ROUTE_KIT
        (presses through the virtual kit inside the game) or ROUTE_KEYS (keys
        to the focused window); None means the kit, and is refused until the
        kit is set up, so no run takes the keyboard unless asked to."""
        with self._lock:
            if self._play is not None and self._play.running():
                return "a play session is already running; call ch_play_stop first"
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
                chords = play_chart.load_dump_chords(score_json)
            except (OSError, ValueError, KeyError, TypeError) as e:
                return f"could not read {score_json}: {type(e).__name__}: {e}"
            if not chords:
                return f"{score_json} has no chords to play"
            lines = [line for line in (self.ensure_backup(),) if line]
            log_path = None
            if log:
                log_path = log if os.path.isabs(log) else os.path.join(self.session_dir(), log)
            cap_s = None if song_folder is None else run_cap_s(song_folder)
            play = _PlaySession(score_json=score_json, chord_count=len(chords), log_path=log_path,
                                cap_s=cap_s, press=press)
            play.thread = threading.Thread(target=self._play_body, args=(play, chords),
                                           name="ch_play_chart", daemon=True)
            self._play = play
            play.thread.start()
            lines.append(f"play session started: {len(chords)} chords from {score_json}")
            lines.append(f"route: {route} ("
                         + ("presses through the virtual kit inside the game"
                            if route == ROUTE_KIT else "keys to the focused game window") + ")")
            lines.append(f"run cap: {play_chart.RUN_CAP_S if cap_s is None else cap_s:g} s")
            lines.append(f"waiting for a moving clock (cap {engine_finder.ENGINE_SEARCH_CAP_S:.0f} s): "
                         "start the song from the game's menu; a song launched with a bot "
                         "is played by the bot, not by keys")
            return "\n".join(lines)

    def ch_play_status(self) -> str:
        """The play session's counts, its stop reason when it ended, and every
        press row, newest first."""
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

    def _stop_all(self, lines: List[str]) -> bool:
        """The stop hooks, the play stop, the foreground watch's stop and its
        count, then the game and the restore. Returns False when the restore
        failed or the stop raised; the run stays open then, so a later
        ch_stop tries the restore again."""
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
            return True
        if self._adopted_backup is not None:
            lines.append("the server did not start the game, so it does not close it")
            if bot_run.restore_only(self._adopted_backup, env) != 0:
                return False
            lines.append("if the game is still running it "
                         + bot_run.late_write_hint(self._adopted_backup))
            self._adopted_backup = None
            return True
        lines.append("no run is open and no backup was taken; nothing to restore")
        return True

    def _play_body(self, play: _PlaySession, chords) -> None:
        """The play thread: live_play, then the press log. It never takes the
        lock, so a tool holding it never waits on the thread."""
        cap = {} if play.cap_s is None else {"cap_s": play.cap_s}
        # The kit route hands live_play its press and no driver, so no window
        # is looked up; the keys route keeps the driver.
        route = {"driver": self._driver} if play.press is None else {"press": play.press}
        try:
            play.result = self._live_play(chords, **route,
                                          on_press=play.rows.append,
                                          should_stop=play.stop.is_set,
                                          out=io.StringIO(), **cap)
        except Exception as e:  # reported by ch_play_status
            play.error = f"{type(e).__name__}: {e}"
        if play.log_path:
            try:
                play_chart.write_press_log(play.log_path, list(play.rows))
                play.log_note = f"press log: {play.log_path} ({len(play.rows)} rows)"
            except OSError as e:
                play.log_note = f"press log not written to {play.log_path}: {e}"

    @staticmethod
    def _stop_reason(play: _PlaySession) -> str:
        if play.error is not None:
            return f"failed: {play.error}"
        return play.result.stop if play.result is not None else "no result"

    def _play_lines(self, play: _PlaySession) -> List[str]:
        rows = list(play.rows)
        lines = [f"play session: {play.chord_count} chords from {play.score_json}"]
        if play.running():
            lines.append(f"running: {len(rows)} presses so far, "
                         f"{sum(1 for r in rows if r['hit'])} hit")
        else:
            lines.append(f"ended: {self._stop_reason(play)}")
            result = play.result
            if result is not None:
                if result.message:
                    lines.append(result.message)
                lines.append(f"sent {result.sent}, hit {result.hit}, "
                             f"misses {result.sent - result.hit}, best streak {result.max_streak}")
                if result.sent != len(rows):
                    lines.append(f"the song clock jumped back during the run: the counts above "
                                 f"cover the part after the jump; the rows below keep all "
                                 f"{len(rows)} presses")
            if play.log_note:
                lines.append(play.log_note)
        if rows:
            # The kit press's columns show only when the rows carry them.
            columns = [c for c in PRESS_LOG_COLUMNS if any(c in r for r in rows)]
            lines.append(f"rows, newest first ({', '.join(columns)}):")
            lines.extend("  " + ", ".join(str(r.get(c, "")) for c in columns)
                         for r in reversed(rows))
        return lines
