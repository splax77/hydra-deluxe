"""Tests for game_tools, the Clone Hero server's game-side tools.

No game, no keys and no real game files: the launch, the clock and the game
are test_bot_run's fakes, the user folders are temp folders, the input driver
records its keys, and live_play is a fake that delivers rows. Tests on the
real tools run the real play thread only while Clone Hero is not running, so
its open of the game fails before any press; such a test fails at once when
the game is running.
"""

from __future__ import annotations

import contextlib
import csv
import dataclasses
import functools
import glob
import inspect
import io
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import unittest
from unittest import mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import bot_run as B  # noqa: E402
from tools.ch_mcp import compare_scorestats  # noqa: E402
from tools.ch_mcp import foreground as F  # noqa: E402
from tools.ch_mcp import game_files as G  # noqa: E402
from tools.ch_mcp import game_tools as GT  # noqa: E402
from tools.ch_mcp import hydra_end  # noqa: E402
from tools.ch_mcp import install_chart as I  # noqa: E402
from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_mcp import quiet as Q  # noqa: E402
from tools.ch_mcp import machine_recorder as MR  # noqa: E402
from tools.ch_mcp import recorder_link as R  # noqa: E402
from tools.ch_mcp import server  # noqa: E402
from tools.ch_mcp import build_stamp  # noqa: E402
from tools.ch_mcp import menu_tools  # noqa: E402
from tools.ch_mcp import frida_tools as FT  # noqa: E402
from tools.ch_mcp.frida_tools import MENU_BLOCK, StateUnread  # noqa: E402
from tools.ch_mcp.tests.test_recorder_link import FakePopen  # noqa: E402
from tools.ch_mcp.tests.test_machine_recorder import complete_rows  # noqa: E402
from tools.ch_mcp.tests.test_bot_run import (  # noqa: E402
    ORIGINAL, REAL_BACKUP, STATS_AFTER_A_RUN, FakeGame, Rig, _read, _write)
from tools.ch_probe.constants import PROCESS_NAME  # noqa: E402
from tools.ch_mcp.tests.test_game_files import FakeReg  # noqa: E402
from tools.ch_mcp.tests.test_hydra_end import FAKE  # noqa: E402
from tools.ch_probe import probe_songs  # noqa: E402
from tools.ch_probe import input_driver  # noqa: E402
from tools.ch_probe.process import ProcessNotFoundError, open_process  # noqa: E402
from tools.ch_probe.experiments import edge_probe, play_chart  # noqa: E402
from tools.ch_probe.experiments.overlay_logs import PRESS_LOG_COLUMNS  # noqa: E402
from tools.ch_probe.tests.test_play_chart import (  # noqa: E402
    _GHOST_SCHEDULE, _FakeGame, _FakeSongClock, _FakeTime, _RecordedThresholdEngine,
    _schedule_of)
from tools.ch_probe.input_driver import DEFAULT_BINDINGS, MENU_KEYS, InputDriver, Lane  # noqa: E402


class StuckGame(FakeGame):
    """A game that does not exit within the stop's wait."""

    def stop(self) -> None:
        raise subprocess.TimeoutExpired(PROCESS_NAME, B.EXIT_CHECK_S)


WINDOW = 0x1234
# How long a test waits on a fake's thread before it fails instead of hanging.
THREAD_CAP_S = 5.0
# How long a test waits on a real child process before it fails instead of
# hanging.
CHILD_CAP_S = 30.0

ROWS = [
    {"index": 0, "chart_ms": 1000.0, "clock_ms_at_press": 998.0,
     "score_before": 0, "score_after": 50, "send": play_chart.SENT},
    {"index": 1, "chart_ms": 1500.0, "clock_ms_at_press": 1499.0,
     "score_before": 50, "score_after": 50, "send": play_chart.SENT},
]
SCORE_JSON = {"chords": [
    {"index": 0, "ms": 1000.0,
     "notes": [{"color": "Red", "cymbal": False, "dynamic": "none"}]},
    {"index": 1, "ms": 1500.0,
     "notes": [{"color": "Kick", "cymbal": False, "dynamic": "none"}]},
]}
# The path_source every hand-built schedule here carries: path 0 of a dump,
# as hydra_replay schedule writes it.
PATH_SOURCE = {"file": "dump.json", "index": 0}


def schedule_of(score):
    """test_play_chart._schedule_of's hand-built schedule of `score`, with
    PATH_SOURCE, so the play tools take it for path 0."""
    return {**_schedule_of(score), "path_source": dict(PATH_SOURCE)}


def for_song(schedule, song_folder):
    """`schedule` as made from the notes.chart in `song_folder`, the song
    ch_run_chart checks the game has loaded."""
    return {**schedule, "chart": os.path.join(song_folder, "notes.chart")}


# A hand-built schedule of SCORE_JSON's two chords, one plain press each
# (test_play_chart._schedule_of).
SCHEDULE_JSON = schedule_of(SCORE_JSON)

# A corpus chart folder (song.ini and notes.mid) for ch_install_chart.
CORPUS_CHART_FOLDER = os.path.join(_REPO_ROOT, "testdata", "input", "test_pitch_dark_ratio")
# The game's scorestats.json from an in-game run of the Window Map probe song
# (copied from hydra-data/ch-mcp-runs/2026-11-07/ls-o2-run1), and Hydra's
# score JSON of that song, path 0 (the command that makes it is in the
# comment above _WINDOW_MAP_SCORE in tools/ch_probe/tests/test_score_compare.py).
WINDOW_MAP_STATS = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-o2-run1",
                                "scorestats-Window_Map-20261107-093442.json")
WINDOW_MAP_SCORE = os.path.join(_REPO_ROOT, "testdata", "replay", "window_map-score.json")

# The schedule fixtures under tools/ch_probe/tests (hydra_replay schedule of
# each chart, path 0) and the reply lines they give. Every number below is
# read off the fixture file: a declined chord's `kept_gems`, a squeeze's
# `anchor_chord`, `anchor_aim_ms`, `planned_end_ms`, `ratio`, `anchor_room_ms`
# and `squeeze_room_ms`, and a squeeze press's firing time (`at_ms`, from its
# `aim_ms`) against its `own_end_ms` (15 ms either side of the end on every
# timed squeeze here).
_PROBE_TESTS = os.path.join(_HERE, "..", "..", "ch_probe", "tests")
FREAK_DECLINED = ("declined: chords 51 (kick), 103 (kick), 134 (nothing sent), "
                  "162 (kick, blue cymbal), 196 (nothing sent), 548 (kick, blue cymbal), "
                  "582 (kick), 959 (nothing sent), 1001 (kick)")
FREAK_SQUEEZES = ("squeezes: chord 387 out (red 15 ms after the end), "
                  "chord 717 in (needs no timing), "
                  "chord 1144 out (yellow tom 15 ms after the end)")
# Chord 263's anchor_aim_ms is -21.824, its planned_end_ms 60,554.527, both
# rooms 26.322 (freak_limits_schedule.json).
FREAK_ANCHORS = [
    "chord 263 pressed 21.8 ms early for the squeeze-out at chord 387 "
    "(end planned 60,554.5 ms, ratio 1.0000, room 26.3 ms both presses)",
    "chord 1062 pressed 62.7 ms early for the squeeze-out at chord 1144 "
    "(end planned 141,596.5 ms, ratio 1.0000, room 11.0 ms both presses)"]
MOUTHS_DECLINED = "declined: chords 221 (nothing sent), 801 (kick, yellow tom), 845 (nothing sent)"
MOUTHS_SQUEEZES = ("squeezes: chord 431 in (15 ms before the end), chord 740 out "
                   "(yellow cymbal, green cymbal 15 ms before the end; kick 15 ms after), "
                   "chord 1240 in (15 ms before the end)")
# Mouths's ratios differ early and late, and chord 340's two rooms differ.
MOUTHS_ANCHORS = [
    "chord 340 pressed 16.6 ms late for the squeeze-in at chord 431 "
    "(end planned 68,991.0 ms, ratio 1.0148 early, 1.0262 late, "
    "anchor room 66.1 ms, squeeze room 52.9 ms)",
    "chord 704 pressed 10.4 ms early for the squeeze-out at chord 740 "
    "(end planned 118,418.3 ms, ratio 1.0287 early, 0.6207 late, room 75.3 ms both presses)",
    "chord 1208 pressed 4.5 ms late for the squeeze-in at chord 1240 "
    "(end planned 209,577.9 ms, ratio 0.9976 early, 0.9989 late, room 78.2 ms both presses)"]
BOMBER_DECLINED = ("declined: chords 624 (kick), 684 (kick), 854 (nothing sent), "
                   "874 (kick), 919 (kick)")
BOMBER_SQUEEZES = ("squeezes: chord 180 out (red 15 ms after the end), "
                   "chord 448 in (needs no timing)")
BOMBER_ANCHOR = ("chord 147 pressed 62.4 ms early for the squeeze-out at chord 180 "
                 "(end planned 32,607.2 ms, ratio 1.0000, room 11.2 ms both presses)")
# The start of each order line: the first split chords of the schedule's
# presses (`after_ms` 8 on the later group). The whole line names every split
# chord of the chart, too long to pin here.
MOUTHS_ORDER = ("order: chords 4 (kick, then green cymbal 8 ms later), "
                "340 (green cymbal, then kick, yellow cymbal 8 ms later), "
                "704 (green cymbal, then kick, yellow cymbal 8 ms later), ")
BOMBER_ORDER = ("order: chords 147 (yellow cymbal, then kick 8 ms later), "
                "265 (yellow cymbal, then kick 8 ms later), "
                "412 (blue cymbal, then kick 8 ms later), "
                "716 (blue cymbal, then kick 8 ms later), ")


class FakeQuiet:
    """What ch_start's quiet check reads, with a record of every reader called."""

    def __init__(self, cpu=3.0):
        self.cpu = cpu
        self.calls = []

    def env(self):
        def called(name, value):
            self.calls.append(name)
            return value

        return Q.QuietEnv(
            list_processes=lambda: called("processes", []),
            sample_cpu=lambda seconds: called(("sample", seconds), self.cpu),
            scan_folders=lambda: called("scan", Q.FolderScan(folders=(), entries=0, took_s=0.0)),
            free_memory_bytes=lambda: called("memory", 1024 ** 3),
            power_scheme=lambda: called("power", "Balanced"))


_quiet_patch = mock.patch.object(Q, "LIVE_ENV", FakeQuiet().env())


def setUpModule():
    # ch_start's check would otherwise read this machine and wait out its sample.
    _quiet_patch.start()
    # The song ends come from fake dumps, because no test runs hydra_replay.
    FAKE.start()


def tearDownModule():
    FAKE.stop()
    _quiet_patch.stop()


class RecordingDriver(InputDriver):
    def __init__(self):
        super().__init__()
        self.events = []

    def send_key(self, vk: int, key_up: bool) -> None:
        self.events.append((vk, key_up))


class FakeLivePlay:
    """Delivers ROWS through on_press, then runs until should_stop says so and
    returns `result`. If THREAD_CAP_S passes first, it returns a "test cap"
    stop instead."""

    def __init__(self, result: play_chart.RunResult):
        self.result = result
        self.delivered = threading.Event()
        self.calls = []

    def __call__(self, chords, *, on_press, should_stop, out, driver=None):
        self.calls.append((list(chords), driver))
        for row in ROWS:
            on_press(row)
        self.delivered.set()
        deadline = time.monotonic() + THREAD_CAP_S
        while not should_stop():
            if time.monotonic() > deadline:
                return play_chart.RunResult(stop="test cap")
            time.sleep(0.01)
        return self.result


class FakeWatch:
    """A foreground watch that records start and stop and holds a set count."""

    def __init__(self, events: list, count: int = 0):
        self._events = events
        self.count = count
        self.started = self.stopped = False

    def start(self):
        self.started = True
        self._events.append("watch start")

    def stop(self):
        self.stopped = True

    def line(self):
        return F.count_line(self.count) + ("" if self.stopped else " so far")


class FakeGamePlayer:
    """kit_tools.GamePlayer as the play thread sees it: it is closed once the
    run ends, or when a refusal leaves it unused."""

    def __init__(self):
        self.closed = False

    def close(self):
        self.closed = True


class FakeLiveInGame:
    """play_chart.live_in_game: records each player it is handed, then plays
    through `play`, a fake that takes live_play's keywords with no driver."""

    def __init__(self, play):
        self.play = play
        self.players = []

    def __call__(self, chords, *, player, **kw):
        self.players.append(player)
        return self.play(chords, **kw)


def live_play_never(*_args, **_kw):
    """A live_play for a kit run, which must never reach it."""
    raise AssertionError("a kit run reached live_play")


class FakeKitPart:
    """The kit part of the run parts, as ch_play_chart and ch_edge_probe
    read it: game_player hands out `player`, None while no kit is set up."""

    def __init__(self, player):
        self.player = player

    def game_player(self):
        return self.player


class ToolRig:
    """A GameTools wired to test_bot_run's Rig and the fakes above. With
    `kit`, the kit is set up: the rig's play fake (the `live_play` argument,
    or FakeLivePlay) plays through FakeLiveInGame on `player`, and a
    live_play that raises stands in for the Python loop."""

    def __init__(self, tmp: str, *, kit: bool = False, **tool_args):
        self.r = Rig(tmp, exits_at=None)
        # The registry key the backup holds and ch_stop puts back.
        self.r.env.run_reg = self.reg = FakeReg()
        self.scratch_root = os.path.join(tmp, "scratch")
        self.driver = RecordingDriver()
        self.hwnd = WINDOW
        self.live_play = FakeLivePlay(play_chart.RunResult(
            sent=2, rows=list(ROWS), stop="stopped"))
        self.player = FakeGamePlayer()
        self.live_in_game = FakeLiveInGame(tool_args.pop("live_play", self.live_play)
                                           if kit else self.live_play)
        self.events = []
        self.watches = []
        self.popen = FakePopen()

        def make_watch():
            self.watches.append(FakeWatch(self.events))
            return self.watches[-1]

        args = dict(driver=self.driver, live_play=live_play_never if kit else self.live_play,
                    live_in_game=self.live_in_game,
                    find_window=lambda: self.hwnd, make_watch=make_watch,
                    make_recorder=self.make_recorder)
        args.update(tool_args)
        self.tools = GT.GameTools(self.r.env, threading.RLock(), self.scratch_root, **args)
        if kit:
            self.tools.use_run_parts(GT.RunParts(None, FakeKitPart(self.player), None))

    def make_recorder(self) -> R.RecorderLink:
        return R.RecorderLink(popen=self.popen)

    def backup_dir(self) -> str:
        run = self.tools._run
        return run.backup_dir if run is not None else self.tools._adopted_backup


class GameToolsTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def real_tools(self) -> GT.GameTools:
        """real_game_tools under this test's folder: a real Env with no
        registry runner and a game exe that is not there, so a launch fails
        for real and nothing can start the game."""
        return real_game_tools(self.tmp, os.path.join(self.tmp, "real scratch"))

    def test_session_dir_is_the_scratch_folder_the_backups_go_in(self):
        tools = self.real_tools()
        scratch_root = os.path.join(self.tmp, "real scratch")
        folder = tools.session_dir()
        self.assertTrue(os.path.isdir(folder))
        self.assertTrue(folder.startswith(scratch_root))
        self.assertEqual(tools.session_dir(), folder)
        tools.ensure_backup()
        backup = tools._adopted_backup
        self.assertTrue(os.path.isfile(os.path.join(backup, G.BACKUP_MANIFEST)), backup)
        self.assertEqual(os.path.dirname(backup), tools.session_dir())
        tools.ch_stop()

    # -- the quiet check ----------------------------------------------------------

    def test_a_machine_that_is_not_quiet_refuses_with_no_folder_no_backup_no_launch(self):
        # A real process list read while a build was linking (BUILD_PROCESSES),
        # and this machine's own other readings.
        processes = build_processes()
        compilers = Q._compiler_lines(processes)
        self.assertEqual(len(compilers), 1)
        tools = self.real_tools()
        with mock.patch.object(Q, "LIVE_ENV", real_quiet_env(processes)):
            reply = tools.ch_start()
        prefix, suffix = refusal_parts()
        self.assertTrue(reply.startswith(prefix), reply)
        self.assertTrue(reply.endswith(suffix), reply)
        self.assertIn(compilers[0], reply)
        self.assertFalse(os.path.exists(os.path.join(self.tmp, "real scratch")))
        self.assertIsNone(tools._scratch)
        self.assertIsNone(tools._run)
        self.assertIsNone(tools._adopted_backup)
        self.assertEqual(tools.open_lines(), [])
        # A ch_stop after the refusal finds nothing to put back and makes no folder.
        tools.ch_stop()
        self.assertFalse(os.path.exists(os.path.join(self.tmp, "real scratch")))

    def test_a_quiet_machine_puts_the_report_before_the_launch_lines(self):
        t = ToolRig(self.tmp)
        lines = t.tools.ch_start(t.r.song).splitlines()
        cpu = lines.index("CPU: 3.00 percent averaged over 5 s (limit 30 percent)")
        backup = lines.index(f"backup folder: {t.backup_dir()}")
        starting = next(i for i, line in enumerate(lines) if line.startswith("starting: "))
        self.assertEqual(lines[0], "no compiler running")
        self.assertLess(cpu, backup)
        self.assertLess(backup, starting)
        self.assertIn("free memory: 1.0 GB", lines)
        self.assertIn("power scheme: Balanced", lines)

    def test_the_check_runs_once_and_waits_quiet_sample_s(self):
        # The real readers, on the recorded build-time process list, so the
        # check refuses and nothing after it runs.
        tools = self.real_tools()
        with mock.patch.object(Q, "LIVE_ENV", real_quiet_env(build_processes())):
            began = time.monotonic()
            reply = tools.ch_start()
            took = time.monotonic() - began
        # The real CPU sample waits its whole QUIET_SAMPLE_S, and the reply
        # holds the one CPU reading it took.
        # A timer may wake a few ms early on the monotonic clock.
        self.assertGreaterEqual(took, Q.QUIET_SAMPLE_S - 0.1)
        self.assertEqual(reply.count(f"averaged over {Q.QUIET_SAMPLE_S} s"), 1, reply)

    def test_an_open_run_is_stopped_first_so_the_game_is_not_read_as_load(self):
        # The open run is a real adopted backup, with its marker.
        tools = self.real_tools()
        scratch_root = os.path.join(self.tmp, "real scratch")
        tools.ensure_backup()
        scratch = tools._scratch
        in_scratch = os.listdir(scratch)
        self.assertTrue(os.path.exists(os.path.join(scratch_root, GT.OPEN_RUN_MARKER)))
        with mock.patch.object(Q, "LIVE_ENV", real_quiet_env(build_processes())):
            lines = tools.ch_start().splitlines()
        self.assertEqual(lines[0], "a run was open; stopping it first:")
        prefix, _suffix = refusal_parts()
        self.assertTrue(lines[-1].startswith(prefix), lines)
        # The stop restored the backup before the check refused.
        self.assertIsNone(tools._adopted_backup)
        self.assertIsNone(tools._run)
        self.assertEqual(tools._scratch, scratch)
        self.assertEqual(os.listdir(scratch), in_scratch)
        self.assertEqual([d for d in os.listdir(scratch_root)
                          if d.startswith(GT.SCRATCH_PREFIX)], [os.path.basename(scratch)])
        self.assertFalse(os.path.exists(os.path.join(scratch_root, GT.OPEN_RUN_MARKER)))

    def test_a_refusal_with_no_need_of_the_machine_takes_no_sample(self):
        # The real readers: a sample would hold the reply QUIET_SAMPLE_S.
        tools = self.real_tools()
        with mock.patch.object(Q, "LIVE_ENV", real_quiet_env()):
            began = time.monotonic()
            reply = tools.ch_start(human=True)
            took = time.monotonic() - began
        self.assertEqual(reply, B.human_refusal(True, None))
        self.assertLess(took, Q.QUIET_SAMPLE_S)

    def test_ch_start_has_no_argument_that_skips_the_check(self):
        self.assertEqual(list(inspect.signature(GT.GameTools.ch_start).parameters),
                         ["self", "song_folder", "player", "profile", "human"])

    # -- ch_start ----------------------------------------------------------------

    def test_ch_start_backs_up_before_the_launch_and_names_the_folder(self):
        # The launch fails for real (the game exe is not there), after the
        # backup and before any game.
        tools = self.real_tools()
        exe = tools._env.exe
        lines = tools.ch_start().splitlines()
        backups = [line for line in lines if line.startswith("backup folder: ")]
        self.assertEqual(len(backups), 1, lines)
        backup = backups[0][len("backup folder: "):]
        self.assertTrue(backup.startswith(os.path.join(self.tmp, "real scratch")), backup)
        self.assertTrue(os.path.isfile(os.path.join(backup, G.BACKUP_MANIFEST)), backup)
        starting = lines.index("starting: " + subprocess.list2cmdline(
            B.launch_command(exe, None, B.DEFAULT_PLAYER)))
        self.assertLess(lines.index(backups[0]), starting)
        self.assertTrue(lines[-1].startswith("could not start the game: "), lines)
        self.assertIsNone(tools._run)
        self.assertIsNone(tools.launched_at())

    def test_ch_start_with_no_song_launches_launch_commands_no_song_line(self):
        t = ToolRig(self.tmp)
        reply = t.tools.ch_start()
        self.assertEqual(t.r.launch.commands,
                         [B.launch_command(t.r.env.exe, None, B.DEFAULT_PLAYER)])
        self.assertIn("song length: none", reply)

    def test_the_command_line_comes_from_launch_command_with_known_flags_only(self):
        t = ToolRig(self.tmp)
        t.tools.ch_start(t.r.song, profile="Tester")
        cmd = t.r.launch.commands[0]
        self.assertEqual(cmd, B.launch_command(t.r.env.exe, t.r.song, B.DEFAULT_PLAYER, "Tester"))
        self.assertEqual({a for a in cmd if a.startswith("--")},
                         {"--song", "--player", "--profile"})

    def test_a_second_ch_start_stops_and_restores_the_first_before_launching(self):
        # The first run is a real adopted backup; the second start's launch
        # fails for real, after its own backup.
        tools = self.real_tools()
        tools.ensure_backup()
        first_backup = tools._adopted_backup
        lines = tools.ch_start().splitlines()
        self.assertEqual(lines[0], "a run was open; stopping it first:")
        restored = [i for i, line in enumerate(lines) if first_backup in line]
        self.assertTrue(restored, lines)
        second_start = [i for i, line in enumerate(lines) if line.startswith("starting: ")]
        self.assertEqual(len(second_start), 1, lines)
        self.assertLess(max(restored), second_start[0])
        self.assertIsNone(tools._adopted_backup)
        backups = [line for line in lines if line.startswith("backup folder: ")]
        self.assertEqual(len(backups), 1, lines)
        self.assertNotIn(first_backup, backups[0])

    # -- ch_stop and shutdown -----------------------------------------------------

    def put_game_files(self) -> dict:
        """The game's files in real_tools' documents folder, holding what the
        real game wrote: scorestats.json after a run (pitch-dark-run1) and
        the settings.ini of ls-g1-run1's backup. Returns each name's bytes."""
        _exe, documents = real_game_paths(self.tmp)
        written = {G.SCORESTATS: _read(STATS_AFTER_A_RUN),
                   G.SETTINGS: _read(os.path.join(REAL_BACKUP, G.SETTINGS))}
        for name, data in written.items():
            _write(os.path.join(documents, name), data)
        return written

    def assert_game_files_kept(self, written: dict) -> None:
        _exe, documents = real_game_paths(self.tmp)
        for name, data in written.items():
            self.assertEqual(_read(os.path.join(documents, name)), data, name)

    def test_ch_stop_restores_the_backup_and_leaves_every_file(self):
        tools = self.real_tools()
        tools.ensure_backup()
        backup = tools._adopted_backup
        # The files as a run left them, written after the backup.
        written = self.put_game_files()
        lines = tools.ch_stop().splitlines()
        self.assertTrue([line for line in lines if backup in line], lines)
        self.assertIsNone(tools._adopted_backup)
        self.assertEqual(tools.open_lines(), [])
        # Every file, settings.ini and scorestats.json among them, stays as
        # the run left it.
        self.assert_game_files_kept(written)
        self.assertIsNone(tools.launched_at())

    def test_ch_stop_runs_the_stop_hooks_first_in_order(self):
        tools = self.real_tools()
        tools.ensure_backup()
        backup = tools._adopted_backup
        marker = os.path.join(self.tmp, "real scratch", GT.OPEN_RUN_MARKER)
        calls = []
        # Each hook notes whether the backup was still unrestored: its marker
        # goes only once the restore is done.
        tools.add_stop_hook(lambda: calls.append(("first", os.path.exists(marker)))
                            or "first hook's line")
        tools.add_stop_hook(lambda: calls.append(("second", os.path.exists(marker))))
        lines = tools.ch_stop().splitlines()
        self.assertEqual(calls, [("first", True), ("second", True)])
        self.assertFalse(os.path.exists(marker))
        # The hook's line comes before the restore's lines, which name the backup.
        restore = [i for i, line in enumerate(lines) if backup in line]
        self.assertTrue(restore, lines)
        self.assertLess(lines.index("first hook's line"), min(restore))

    def test_ch_stop_says_when_the_game_outlives_the_wait_and_still_restores(self):
        t = ToolRig(self.tmp)
        t.r.game = t.r.launch.game = StuckGame(t.r.clock)
        t.tools.ch_start(t.r.song)
        for name in ORIGINAL:
            _write(t.r.path(name), b"changed by the game")
        reply = t.tools.ch_stop()
        self.assertIn("(pid 4242) was told to stop but was still running after 30 s", reply)
        self.assertNotIn("stopped the game\n", reply)
        self.assertIn("rewired registry: unchanged", reply)
        t.r.assert_files_hold(self, b"changed by the game")

    def test_a_failed_restore_names_it_and_the_folder_and_nothing_further_runs(self):
        tools = self.real_tools()
        tools.ensure_backup()
        backup = tools._adopted_backup
        scratch_root = os.path.join(self.tmp, "real scratch")
        before = sorted(os.listdir(scratch_root))
        # A real failure: the backup folder is moved away, so the restore
        # cannot read its manifest.
        moved = backup + " moved"
        os.rename(backup, moved)
        lines = tools.ch_start().splitlines()
        self.assertEqual(lines[0], "a run was open; stopping it first:")
        self.assertTrue([line for line in lines if backup in line], lines)
        self.assertEqual(lines[-1], "the new run was not started")
        self.assertFalse([line for line in lines if line.startswith("starting: ")], lines)
        # No new session folder or backup was made.
        self.assertEqual(sorted(os.listdir(scratch_root)), before)
        # The backup stays held, so the next ch_stop tries the restore again.
        self.assertEqual(tools._adopted_backup, backup)
        os.rename(moved, backup)
        tools.ch_stop()
        self.assertIsNone(tools._adopted_backup)

    def test_shutdown_with_an_adopted_game_restores_only(self):
        tools = self.real_tools()
        tools.ensure_backup()
        backup = tools._adopted_backup
        written = self.put_game_files()
        reply = tools.shutdown()
        self.assertIsNone(tools._adopted_backup)
        self.assert_game_files_kept(written)
        self.assertIn("the server did not start the game, so it does not close it", reply)
        self.assertEqual(reply.splitlines()[-1],
                         "if the game is still running it " + B.late_write_hint(backup))

    # -- ensure_backup and launched_at ----------------------------------------------

    def test_ensure_backup_on_an_adopted_game_takes_one_backup(self):
        tools = self.real_tools()
        scratch_root = os.path.join(self.tmp, "real scratch")
        first = tools.ensure_backup()
        backup = tools._adopted_backup
        self.assertTrue(first.startswith(f"backup folder: {backup} "))
        self.assertTrue(os.path.isfile(os.path.join(backup, G.BACKUP_MANIFEST)))
        self.assertIsNone(tools.launched_at())
        self.assertEqual(tools.ensure_backup(), "")
        self.assertEqual(sorted(os.listdir(scratch_root)),
                         sorted([os.path.basename(tools._scratch), GT.OPEN_RUN_MARKER]))
        tools.ch_stop()

    # -- the stats files ----------------------------------------------------------

    def test_ch_install_chart_names_the_refusal_install_gives(self):
        # The real install into a songs folder under this test's folder, on a
        # corpus chart folder. Both refusals come before anything is read or
        # written, so neither needs hydra_replay or ffmpeg.
        dest_root = os.path.join(self.tmp, "songs")
        os.makedirs(os.path.join(dest_root, os.path.basename(CORPUS_CHART_FOLDER)))
        tools = GT.GameTools(B.Env(*real_game_paths(self.tmp), run_reg=None), threading.RLock(),
                             os.path.join(self.tmp, "real scratch"),
                             install=functools.partial(I.install, dest_root=dest_root))
        taken = tools.ch_install_chart(CORPUS_CHART_FOLDER)
        self.assertTrue(taken.startswith("refused: "), taken)
        self.assertIn(os.path.join(dest_root, os.path.basename(CORPUS_CHART_FOLDER)), taken)
        missing = os.path.join(self.tmp, "no such folder")
        not_a_folder = tools.ch_install_chart(missing)
        self.assertTrue(not_a_folder.startswith("refused: "), not_a_folder)
        self.assertIn(missing, not_a_folder)
        # Neither refusal wrote anything into the songs folder.
        self.assertEqual(os.listdir(dest_root), [os.path.basename(CORPUS_CHART_FOLDER)])

    def test_ch_wait_stats_and_ch_stats_with_no_run_and_no_copy_name_the_next_tool(self):
        tools = self.real_tools()
        wait = tools.ch_wait_stats()
        self.assertIn("no run is open", wait)
        self.assertIn("ch_start", wait)
        stats = tools.ch_stats()
        self.assertIn("no stats copy yet", stats)
        self.assertIn("ch_wait_stats", stats)
        # Neither took a backup or made a session folder.
        self.assertIsNone(tools._adopted_backup)
        self.assertFalse(os.path.exists(os.path.join(self.tmp, "real scratch")))

    def test_ch_stats_prints_every_key_in_the_files_order(self):
        with open(WINDOW_MAP_STATS, encoding="utf-8-sig") as f:
            stats = json.load(f)
        lines = self.real_tools().ch_stats(WINDOW_MAP_STATS).splitlines()
        self.assertEqual(lines[0], f"stats: {WINDOW_MAP_STATS}")
        # Each line is `key: value`; the value reads back as the file's own,
        # a string as itself and anything else as JSON.
        def parsed(line, indent):
            self.assertTrue(line.startswith(indent), line)
            key, _sep, text = line[len(indent):].partition(": ")
            return key, text

        top = [key for key in stats if key != "players"]
        self.assertEqual([parsed(line, "")[0] for line in lines[1:1 + len(top)]], top)
        for line in lines[1:1 + len(top)]:
            key, text = parsed(line, "")
            value = stats[key]
            self.assertEqual(text if isinstance(value, str) else json.loads(text), value, key)
        rest = lines[1 + len(top):]
        for i, player in enumerate(stats["players"]):
            self.assertEqual(rest[0], f"players[{i}]:")
            body, rest = rest[1:1 + len(player)], rest[1 + len(player):]
            self.assertEqual([parsed(line, "  ")[0] for line in body], list(player))
            for line in body:
                key, text = parsed(line, "  ")
                value = player[key]
                self.assertEqual(text if isinstance(value, str) else json.loads(text), value, key)
        self.assertEqual(rest, [])

    def _window_map_run(self) -> compare_scorestats.RunSettings:
        """The settings the Window Map stats file says the game played with,
        given back as the run's."""
        with open(WINDOW_MAP_STATS, encoding="utf-8-sig") as f:
            stats = json.load(f)
        player = stats["players"][0]
        return compare_scorestats.RunSettings(stats["checksum"], stats["playback_speed"],
                                              player["no_fail"], tuple(player["modifiers"]))

    def test_ch_compare_prints_compares_lines_then_the_verdict(self):
        run = self._window_map_run()
        expected = []
        ok = compare_scorestats.compare(WINDOW_MAP_STATS, WINDOW_MAP_SCORE, run=run,
                                        out=expected.append)
        reply = self.real_tools().ch_compare(WINDOW_MAP_STATS, WINDOW_MAP_SCORE, run.checksum,
                                             run.playback_speed, run.no_fail, list(run.modifiers))
        self.assertEqual(reply.splitlines(), expected + ["pass" if ok else "fail"])

    def test_ch_compare_without_the_run_settings_fails_on_the_settings_line(self):
        expected = []
        self.assertFalse(compare_scorestats.compare(WINDOW_MAP_STATS, WINDOW_MAP_SCORE,
                                                    out=expected.append))
        # With the run's settings given, the same files give other lines: the
        # settings line is where the two differ.
        given = []
        compare_scorestats.compare(WINDOW_MAP_STATS, WINDOW_MAP_SCORE,
                                   run=self._window_map_run(), out=given.append)
        self.assertNotEqual(expected, given)
        reply = self.real_tools().ch_compare(WINDOW_MAP_STATS, WINDOW_MAP_SCORE)
        self.assertEqual(reply.splitlines(), expected + ["fail"])

    def test_ch_compare_refuses_some_but_not_all_of_the_run_settings(self):
        run = self._window_map_run()
        # A stats path that is not there: the refusal comes before any read.
        missing = os.path.join(self.tmp, "no stats.json")
        reply = self.real_tools().ch_compare(missing, WINDOW_MAP_SCORE, checksum=run.checksum,
                                             no_fail=run.no_fail)
        self.assertTrue(reply.startswith("refused: "), reply)
        self.assertNotIn(missing, reply)
        named = reply.partition("missing: ")[2].split(", ")
        self.assertEqual(named, ["playback_speed", "modifiers"])
        # Every name it gives is one of ch_compare's own arguments.
        self.assertLessEqual(set(named),
                             set(inspect.signature(GT.GameTools.ch_compare).parameters))

    def test_ch_compare_takes_an_empty_modifier_list_as_given(self):
        run = self._window_map_run()
        self.assertTrue(run.modifiers)
        none_played = []
        compare_scorestats.compare(WINDOW_MAP_STATS, WINDOW_MAP_SCORE,
                                   run=dataclasses.replace(run, modifiers=()), out=none_played.append)
        as_played = []
        compare_scorestats.compare(WINDOW_MAP_STATS, WINDOW_MAP_SCORE, run=run,
                                   out=as_played.append)
        reply = self.real_tools().ch_compare(WINDOW_MAP_STATS, WINDOW_MAP_SCORE, run.checksum,
                                             run.playback_speed, run.no_fail, [])
        # Taken as an empty list, not as a missing setting: compare's lines
        # for no modifiers, which differ from the ones for the modifiers the
        # game played with.
        self.assertEqual(reply.splitlines()[:-1], none_played)
        self.assertNotEqual(none_played, as_played)
        self.assertEqual(reply.splitlines()[-1], "fail")

    # -- keys -------------------------------------------------------------------------

    def test_ch_press_refuses_an_unknown_lane_with_the_list(self):
        tools = self.real_tools()
        reply = tools.ch_press("purple")
        self.assertIn('"purple"', reply)
        listed = reply.partition("valid lanes: ")[2].split(", ")
        self.assertEqual(listed, [lane.name.lower() for lane in Lane])
        # Refused before the window lookup: no backup, no session folder.
        self.assertIsNone(tools._adopted_backup)
        self.assertFalse(os.path.exists(os.path.join(self.tmp, "real scratch")))

    def test_ch_press_with_no_window_says_so(self):
        # The real window lookup on this desktop. With the game open the
        # press would go to it, so the test fails at once rather than press.
        if input_driver.find_game_window():
            self.fail("a Clone Hero window is open; close it to run this test")
        tools = self.real_tools()
        reply = tools.ch_press("red")
        self.assertTrue(reply.startswith("no game window found"), reply)
        self.assertIsNone(tools._adopted_backup)
        self.assertFalse(os.path.exists(os.path.join(self.tmp, "real scratch")))

    # -- menu keys ------------------------------------------------------------------------

    def _fail_if_game_open(self):
        """The real window lookup on this desktop. With the game open a key
        that passes the check would go to it, so the test fails at once
        rather than press."""
        if input_driver.find_game_window():
            self.fail("a Clone Hero window is open; close it to run this test")

    def assert_nothing_held(self, tools):
        """No backup and no session folder: the refusal came before the
        backup, which comes before the first key goes out."""
        self.assertIsNone(tools._adopted_backup)
        self.assertFalse(os.path.exists(os.path.join(self.tmp, "real scratch")))

    def test_ch_key_refuses_an_unknown_key_with_the_list_and_sends_nothing(self):
        tools = self.real_tools()
        reply = tools.ch_key("f1")
        self.assertTrue(reply.startswith('unknown key "f1"'), reply)
        # The list is input_driver's menu keys in its order, then the text item.
        keys, _and, text = reply.partition("valid keys: ")[2].rpartition(", and ")
        self.assertEqual(keys.split(", "), list(MENU_KEYS))
        self.assertEqual(text, GT.TEXT_PREFIX + "<string>")
        self.assert_nothing_held(tools)

    def test_ch_key_checks_every_key_before_sending_any(self):
        tools = self.real_tools()
        reply = tools.ch_key("enter,purple")
        self.assertTrue(reply.startswith('unknown key "purple"'), reply)
        self.assert_nothing_held(tools)

    def test_ch_key_with_no_window_says_so(self):
        self._fail_if_game_open()
        tools = self.real_tools()
        reply = tools.ch_key("enter")
        self.assertTrue(reply.startswith("no game window found"), reply)
        self.assert_nothing_held(tools)

    def test_press_keys_takes_each_item_whole(self):
        self._fail_if_game_open()
        tools = self.real_tools()
        # ch_key splits "enter,escape" into two keys, which pass the check and
        # reach the window lookup; press_keys takes the same text as one item,
        # which is no key.
        self.assertTrue(tools.ch_key("enter,escape").startswith("no game window found"))
        whole = tools.press_keys(["enter,escape"])
        self.assertFalse(whole.sent)
        self.assertTrue(whole.text.startswith('unknown key "enter,escape"'), whole.text)
        # A text item with a comma in it passes the check as it stands.
        typed = tools.press_keys(["escape", "text:x,y"])
        self.assertTrue(typed.text.startswith("no game window found"), typed.text)
        self.assert_nothing_held(tools)

    def test_press_keys_says_sent_false_for_each_refusal(self):
        self._fail_if_game_open()
        tools = self.real_tools()
        unknown = tools.press_keys(["enter", "purple"])
        self.assertFalse(unknown.sent)
        self.assertTrue(unknown.text.startswith('unknown key "purple"'), unknown.text)
        empty_text = tools.press_keys([GT.TEXT_PREFIX])
        self.assertFalse(empty_text.sent)
        self.assertTrue(empty_text.text.startswith(f'"{GT.TEXT_PREFIX}" has no text to type'),
                        empty_text.text)
        none_given = tools.press_keys([])
        self.assertFalse(none_given.sent)
        self.assertTrue(none_given.text.startswith("no keys given"), none_given.text)
        no_window = tools.press_keys(["enter"])
        self.assertFalse(no_window.sent)
        self.assertTrue(no_window.text.startswith("no game window found"), no_window.text)
        self.assert_nothing_held(tools)

    def test_press_lanes_says_sent_false_for_each_refusal(self):
        self._fail_if_game_open()
        tools = self.real_tools()
        unknown = tools.press_lanes("purple")
        self.assertFalse(unknown.sent)
        self.assertTrue(unknown.text.startswith('unknown lane "purple"'), unknown.text)
        none_given = tools.press_lanes("")
        self.assertFalse(none_given.sent)
        self.assertTrue(none_given.text.startswith("no lanes given"), none_given.text)
        no_window = tools.press_lanes("red")
        self.assertFalse(no_window.sent)
        self.assertTrue(no_window.text.startswith("no game window found"), no_window.text)
        self.assert_nothing_held(tools)

    def test_ch_press_prints_the_text_press_lanes_returns(self):
        # Each case press_lanes answers without the game: an unknown lane, no
        # lanes, and no window.
        self._fail_if_game_open()
        tools = self.real_tools()
        for lanes in ("purple", "", "red"):
            with self.subTest(lanes=lanes):
                self.assertEqual(tools.ch_press(lanes), tools.press_lanes(lanes).text)

    def test_ch_key_keeps_a_text_item_whole_to_the_end(self):
        self._fail_if_game_open()
        tools = self.real_tools()
        # Split at its comma, the text item would leave " fill", which the key
        # check refuses; kept whole, the list passes the check and reaches the
        # window lookup.
        self.assertTrue(tools.ch_key("enter, fill").startswith('unknown key "fill"'))
        self.assertTrue(tools.ch_key("enter, down ,text:Ladies Talk, fill")
                        .startswith("no game window found"))
        # Spaces before the text item do not hide it, and spaces inside it are
        # text to type, so it is not empty.
        self.assertTrue(tools.ch_key(" text: a ").startswith("no game window found"))
        self.assert_nothing_held(tools)

    # -- the run cap ----------------------------------------------------------------------

    def test_run_cap_is_bot_runs_cap_of_the_song_end_else_play_charts_cap(self):
        # The song end of a real hydra_replay schedule (PITCH_DARK_SCHEDULE).
        end_ms = _json_field(PITCH_DARK_SCHEDULE, hydra_end.FIELD)
        tools = self.real_tools()
        self.assertEqual(tools.cap_for_end(end_ms), B.run_cap_s(end_ms))
        self.assertEqual(GT.cap_for_end(end_ms), B.run_cap_s(end_ms))
        self.assertEqual(GT.cap_for_end(None), play_chart.RUN_CAP_S)

    def test_the_schedules_run_cap_reads_its_song_end_and_refuses_a_file_with_none(self):
        # PITCH_DARK_SCHEDULE is a hydra_replay schedule with a song end.
        end_ms = _json_field(PITCH_DARK_SCHEDULE, hydra_end.FIELD)
        self.assertEqual(GT.schedule_run_cap_s(PITCH_DARK_SCHEDULE),
                         (GT.cap_for_end(end_ms), end_ms))
        # NO_SONG_END_DUMP was written by a hydra_replay older than the song
        # end, so it has no song_length_ms.
        with open(NO_SONG_END_DUMP, encoding="utf-8") as f:
            self.assertNotIn(hydra_end.FIELD, json.load(f))
        with self.assertRaises(hydra_end.SongEndMissing):
            GT.schedule_run_cap_s(NO_SONG_END_DUMP)

    # -- the play status --------------------------------------------------------------------

    def _ended_status(self, rows, result):
        """ch_play_status of a real play session that ended with `result`
        after the press rows `rows`."""
        tools = self.real_tools()
        tools._play = GT._PlaySession(score_json="score.json", chord_count=len(rows),
                                      log_path=None, rows=list(rows), result=result)
        return tools.ch_play_status().splitlines()

    def test_the_status_prints_the_threshold_the_player_read(self):
        # with_axes on a ghost chart reads the threshold one real run read
        # (test_play_chart's _RecordedThresholdEngine, from ls-o1-run1).
        engine = _RecordedThresholdEngine()
        _chords, threshold = play_chart.with_axes(
            play_chart.load_schedule(_GHOST_SCHEDULE).chords, engine)
        self.assertEqual((engine.reads, threshold), (1, engine.value))
        lines = self._ended_status([], play_chart.RunResult(stop="done",
                                                            dyn_threshold=threshold))
        self.assertIn(f"dynamics threshold: {engine.value}", lines)

    def test_the_status_prints_no_threshold_when_none_was_read(self):
        # Pitch Dark's schedule presses no ghost, so with_axes reads no
        # threshold.
        engine = _RecordedThresholdEngine()
        _chords, threshold = play_chart.with_axes(
            play_chart.load_schedule(PITCH_DARK_SCHEDULE).chords, engine)
        self.assertEqual((engine.reads, threshold), (0, None))
        lines = self._ended_status([], play_chart.RunResult(stop="done",
                                                            dyn_threshold=threshold))
        self.assertFalse([line for line in lines if "threshold" in line], lines)

    def test_the_status_names_the_first_chord_pressed_from_the_oldest_row(self):
        # Four rows of a real run's press log (PITCH_DARK_RUN), oldest first.
        rows = _press_rows(*PITCH_DARK_RUN, "press-1-chords-61-64.csv")
        lines = self._ended_status(rows, play_chart.RunResult(
            stop="done", sent=len(rows), rows=list(rows)))
        self.assertIn(GT.first_press_line(rows), lines)
        self.assertNotIn(GT.first_press_line(rows[-1:]), lines)
        self.assertNotEqual(rows[0]["index"], rows[-1]["index"])

    def test_the_status_names_the_first_chord_while_the_run_goes_on(self):
        rows = _press_rows(*PITCH_DARK_RUN, "press-1-chords-61-64.csv")
        tools = self.real_tools()
        play = GT._PlaySession(score_json="score.json", chord_count=len(rows),
                               log_path=None, rows=list(rows))
        # The run's thread runs until the session's stop is set, which
        # ch_play_stop does.
        play.thread = threading.Thread(target=play.stop.wait, args=(THREAD_CAP_S,),
                                       daemon=True)
        play.thread.start()
        tools._play = play
        lines = tools.ch_play_status().splitlines()
        self.assertIn(f"running: {len(rows)} presses so far", lines)
        self.assertIn(GT.first_press_line(rows), lines)
        self.assertEqual(tools.ch_play_stop().splitlines()[-1], "stopped")
        play.thread.join(THREAD_CAP_S)
        self.assertFalse(play.running())

    def _fail_if_game_runs(self) -> str:
        """The real open_process, which a keys-route run calls first. With
        Clone Hero running, a run here would read its memory and press keys
        into it, so the test fails at once. With it not running, returns the
        open's refusal, the words the run's thread reports."""
        try:
            open_process(read_only=True)
        except ProcessNotFoundError as e:
            return str(e)
        self.fail("Clone Hero is running; close it to run this test")

    def _run_with_no_game(self, score=None, schedule=None, log=None):
        """ch_play_chart on the keys route of real tools, with Clone Hero not
        running: the real play thread runs the real live_play, whose open of
        the game fails, so the run ends before any press. `score` and
        `schedule` default to Pitch Dark's hydra_replay files
        (PITCH_DARK_SCORE, PITCH_DARK_SCHEDULE). Returns the tools, the start
        reply and the open's refusal."""
        refusal = self._fail_if_game_runs()
        tools = self.real_tools()
        reply = tools.ch_play_chart(score or PITCH_DARK_SCORE, schedule or PITCH_DARK_SCHEDULE,
                                    log=log, route=GT.ROUTE_KEYS)
        self.assertIsNotNone(tools._play, reply)
        tools._play.thread.join(THREAD_CAP_S)
        self.assertFalse(tools._play.running())
        return tools, reply, refusal

    def _running_session(self, tools, rows):
        """A real play session on `tools` holding the press rows `rows`,
        whose thread runs until the session's stop is set, as a run's thread
        does once ch_play_stop or ch_stop sets it."""
        play = GT._PlaySession(score_json=PITCH_DARK_SCORE, chord_count=len(rows),
                               log_path=None, rows=list(rows))
        play.thread = threading.Thread(target=play.stop.wait, args=(THREAD_CAP_S,),
                                       daemon=True)
        play.thread.start()
        tools._play = play
        return play

    def test_the_status_says_when_no_chord_was_pressed(self):
        tools, reply, _refusal = self._run_with_no_game()
        # The start reply names the chords the schedule plays and the score.
        chords, _plan = GT.chords_to_play(PITCH_DARK_SCHEDULE)
        started = [line for line in reply.splitlines()
                   if line.startswith("play session started:")]
        self.assertEqual(len(started), 1, reply)
        self.assertIn(f" {len(chords)} chords ", started[0])
        self.assertTrue(started[0].endswith(PITCH_DARK_SCORE), started[0])
        self.assertEqual(tools._play.rows, [])
        self.assertIn("first press: none, no chord was pressed",
                      tools.ch_play_status().splitlines())

    def test_a_run_that_cannot_open_the_game_reports_why(self):
        tools, _reply, refusal = self._run_with_no_game()
        lines = tools.ch_play_status().splitlines()
        ended = [line for line in lines if line.startswith("ended: ")]
        self.assertEqual(len(ended), 1, lines)
        self.assertIn(ProcessNotFoundError.__name__, ended[0])
        self.assertIn(refusal, ended[0])
        snapshot = tools.play_snapshot()
        self.assertIn(refusal, snapshot["stop"])
        self.assertEqual((snapshot["last_row"], snapshot["press_count"], snapshot["message"]),
                         (None, 0, ""))

    def test_the_press_log_of_a_run_with_no_press_is_its_header(self):
        tools, _reply, _refusal = self._run_with_no_game(log="press.csv")
        log = os.path.join(tools.session_dir(), "press.csv")
        first = tools.ch_play_status().splitlines()[0]
        self.assertTrue(first.startswith(f"press log: {log} "), first)
        self.assertIn("0 rows", first)
        with open(log, encoding="utf-8", newline="") as f:
            self.assertEqual(list(csv.reader(f)), [list(PRESS_LOG_COLUMNS)])

    # -- the auto-player ------------------------------------------------------------------

    def _score_json(self) -> str:
        path = os.path.join(self.tmp, "score.json")
        _write(path, json.dumps(SCORE_JSON).encode())
        return path

    def _song(self) -> str:
        """The song folder of every ToolRig made on self.tmp (test_bot_run's Rig)."""
        return os.path.join(self.tmp, "songs", probe_songs.WINDOW_MAP)

    def _schedule_json(self, schedule=None) -> str:
        path = os.path.join(self.tmp, "schedule.json")
        _write(path, json.dumps(for_song(schedule or SCHEDULE_JSON, self._song())).encode())
        return path

    def test_a_second_ch_play_chart_is_refused_while_one_runs(self):
        tools = self.real_tools()
        play = self._running_session(
            tools, _press_rows(*PITCH_DARK_RUN, "press-1-chords-61-64.csv"))
        self.assertEqual(tools.ch_play_chart(PITCH_DARK_SCORE, PITCH_DARK_SCHEDULE,
                                             route=GT.ROUTE_KEYS),
                         GT.PLAY_ALREADY_RUNNING)
        # The running session is kept, and the refusal came before the backup.
        self.assertIs(tools._play, play)
        self.assert_nothing_held(tools)
        tools.ch_play_stop()
        play.thread.join(THREAD_CAP_S)
        self.assertFalse(play.running())

    def test_ch_play_stop_sets_the_flag_and_the_session_ends(self):
        tools = self.real_tools()
        # Four rows of a real run's press log (PITCH_DARK_RUN).
        rows = _press_rows(*PITCH_DARK_RUN, "press-1-chords-61-64.csv")
        play = self._running_session(tools, rows)
        self.assertEqual(tools.play_snapshot(), {"last_row": rows[-1],
                                                 "press_count": len(rows), "stop": None,
                                                 "message": ""})
        self.assertEqual(tools.ch_play_stop().splitlines()[-1], "stopped")
        self.assertTrue(play.stop.is_set())
        play.thread.join(THREAD_CAP_S)
        self.assertFalse(play.running())
        snapshot = tools.play_snapshot()
        self.assertIsNotNone(snapshot["stop"])
        self.assertEqual((snapshot["last_row"], snapshot["press_count"]),
                         (rows[-1], len(rows)))

    def test_the_status_of_an_ended_run_lists_its_rows_newest_first(self):
        rows = _press_rows(*PITCH_DARK_RUN, "press-1-chords-61-64.csv")
        result = play_chart.RunResult(stop="stopped", sent=len(rows), rows=list(rows))
        lines = self._ended_status(rows, result)
        self.assertIn(f"ended: {result.stop}", lines)
        header = next(i for i, line in enumerate(lines) if line.startswith("rows, newest first"))
        listed = lines[header + 1:]
        self.assertEqual([line.split(",")[0].strip() for line in listed],
                         [str(row["index"]) for row in reversed(rows)])

    def test_play_snapshot_is_none_with_no_play_session(self):
        tools = self.real_tools()
        self.assertIsNone(tools.play_snapshot())
        self.assertEqual(tools.ch_play_status(), "no play session has run")

    def test_ch_stop_tells_a_running_play_session_to_stop(self):
        tools = self.real_tools()
        play = self._running_session(
            tools, _press_rows(*PITCH_DARK_RUN, "press-1-chords-61-64.csv"))
        reply = tools.ch_stop()
        self.assertIn("play session told to stop", reply.splitlines())
        self.assertTrue(play.stop.is_set())
        play.thread.join(THREAD_CAP_S)
        self.assertFalse(play.running())

    # -- the route ----------------------------------------------------------------------

    def test_no_route_is_refused_while_the_kit_is_not_set_up(self):
        # The server's own tools, whose real kit tools have no kit set up.
        tools = self._server_tools()
        self.assertEqual(tools.ch_play_chart(PITCH_DARK_SCORE, PITCH_DARK_SCHEDULE),
                         GT.NO_ROUTE_NO_KIT)
        self.assertIsNone(tools._play)

    def test_no_route_is_refused_with_no_kit_tools_and_presses_no_key(self):
        # Real tools with no run parts handed in.
        tools = self.real_tools()
        self.assertEqual(tools.ch_play_chart(PITCH_DARK_SCORE, PITCH_DARK_SCHEDULE),
                         GT.NO_ROUTE_NO_KIT)
        self.assertIsNone(tools._play)
        self.assert_nothing_held(tools)

    def test_route_keys_plays_on_the_key_driver_with_no_kit_player(self):
        tools, reply, _refusal = self._run_with_no_game()
        routes = [line for line in reply.splitlines() if line.startswith("route: ")]
        self.assertEqual(len(routes), 1, reply)
        self.assertTrue(routes[0].startswith(f"route: {GT.ROUTE_KEYS} ("), routes[0])
        # With no kit player, the play thread presses through the key driver.
        self.assertIsNone(tools._play.player)

    def test_route_kit_without_a_kit_and_an_unknown_route_are_refused(self):
        tools = self._server_tools()
        kit = tools.ch_play_chart(PITCH_DARK_SCORE, PITCH_DARK_SCHEDULE, route=GT.ROUTE_KIT)
        self.assertTrue(kit.startswith(f"route {GT.ROUTE_KIT}: "), kit)
        self.assertIn(f'route="{GT.ROUTE_KEYS}"', kit)
        unknown = tools.ch_play_chart(PITCH_DARK_SCORE, PITCH_DARK_SCHEDULE, route="pads")
        self.assertIn('"pads"', unknown)
        self.assertTrue(unknown.endswith(", ".join(GT.ROUTES)), unknown)
        self.assertIsNone(tools._play)

    # -- declining a fill ---------------------------------------------------------------

    # Which gems a declined chord keeps is the schedule's (tools/
    # replay_schedule.cpp); these tests only carry its file through the tool.
    # A chord 1 of a kick and a green cymbal, declined in the schedule with
    # `kept` as its kept gems.
    DECLINE_SCORE = {"chords": [
        {"index": 0, "ms": 1000.0, "tick": 480,
         "notes": [{"color": "Red", "cymbal": False, "dynamic": "none"}]},
        {"index": 1, "ms": 1500.0, "tick": 960,
         "notes": [{"color": "Kick", "cymbal": False, "dynamic": "none"},
                   {"color": "Green", "cymbal": True, "dynamic": "none"}]},
    ]}

    def _decline_files(self, kept=(0,)):
        schedule = schedule_of(self.DECLINE_SCORE)
        if kept is not None:
            schedule["chords"][1].update(declined=True, kept_gems=list(kept))
            schedule["presses"][1]["gems"] = list(kept)
        score = os.path.join(self.tmp, "decline-score.json")
        path = os.path.join(self.tmp, "decline-schedule.json")
        _write(score, json.dumps(self.DECLINE_SCORE).encode())
        _write(path, json.dumps(for_song(schedule, self._song())).encode())
        return score, path

    def _decline_rig(self):
        seen = []

        def live_play(chords, **kw):
            seen.append(chords)
            return play_chart.RunResult(stop="done")

        return ToolRig(self.tmp, live_play=live_play), seen

    def _freak_declines(self):
        """Freak's hydra_replay schedule (FREAK_SCHEDULE): the chords the
        play tools read from it, each declined chord by index, its kept gems
        by index, and the reply of a keys-route run on it."""
        with open(self.FREAK_SCHEDULE, encoding="utf-8") as f:
            doc = json.load(f)
        kept = {row["index"]: row["kept_gems"] for row in doc["chords"] if row["declined"]}
        chords, _plan = GT.chords_to_play(self.FREAK_SCHEDULE)
        declined = {chord.index: chord for chord in chords if chord.declined}
        self.assertEqual(set(declined), set(kept))
        _tools, reply, _refusal = self._run_with_no_game(
            os.path.join(_PROBE_TESTS, "freak_limits_score.json"), self.FREAK_SCHEDULE)
        line = GT.declined_line(chords, doc)
        self.assertIn(line, reply.splitlines())
        return declined, kept, line

    def test_a_declined_chord_sends_its_kept_gems_and_the_reply_names_them(self):
        declined, kept, line = self._freak_declines()
        sending = [index for index in kept if kept[index]]
        self.assertTrue(sending, kept)
        for index in sending:
            chord = declined[index]
            with self.subTest(chord=index):
                self.assertEqual(sorted(lane for p in chord.presses for lane in p.lanes),
                                 sorted(chord.lanes[k] for k in kept[index]))
                self.assertNotIn(f"{index} (nothing sent)", line)

    def test_a_declined_chord_that_keeps_no_gem_says_nothing_sent(self):
        declined, kept, line = self._freak_declines()
        silent = [index for index in kept if not kept[index]]
        self.assertTrue(silent, kept)
        for index in silent:
            with self.subTest(chord=index):
                self.assertEqual([lane for p in declined[index].presses for lane in p.lanes], [])
                self.assertIn(f"{index} (nothing sent)", line)

    def test_a_schedule_with_no_declined_chord_says_none(self):
        t, seen = self._decline_rig()
        score, schedule = self._decline_files(kept=None)
        reply = t.tools.ch_play_chart(score, schedule, route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertEqual([c.declined for c in seen[0]], [False, False])
        self.assertEqual([len(p.lanes) for c in seen[0] for p in c.presses], [1, 2])
        self.assertIn("declined: chords none", reply.splitlines())
        self.assertNotIn("squeezes", reply)

    def test_decline_mode_and_dump_json_are_no_longer_arguments(self):
        # D324 item 2 of docs/audit/2026-10-03-fix-decisions.md and question 6
        # of docs/superpowers/plans/2026-11-08-replay-press-schedule.md: a
        # call that still passes either fails at once.
        t, seen = self._decline_rig()
        score, schedule = self._decline_files()
        for extra in ({"decline_mode": "nothing"}, {"dump_json": schedule}):
            with self.assertRaises(TypeError):
                t.tools.ch_play_chart(score, schedule, route="keys", **extra)
        self.assertIsNone(t.tools._play)
        self.assertEqual(seen, [])

    def test_a_schedule_with_a_refusal_is_refused_in_its_words(self):
        t, seen = self._decline_rig()
        score, path = self._decline_files()
        schedule = schedule_of(self.DECLINE_SCORE)
        schedule["refusals"] = ["chord 1 is two squeezes; no rule times both"]
        _write(path, json.dumps(schedule).encode())
        self.assertEqual(t.tools.ch_play_chart(score, path, route="keys"),
                         f"{path} refuses to play this path: "
                         "chord 1 is two squeezes; no rule times both")
        self.assertIsNone(t.tools._play)
        self.assertEqual(seen, [])

    def test_a_missing_schedule_is_refused(self):
        t, seen = self._decline_rig()
        score, _schedule = self._decline_files()
        missing = os.path.join(self.tmp, "no such schedule.json")
        reply = t.tools.ch_play_chart(score, missing, route="keys")
        self.assertTrue(reply.startswith(f"could not read {missing}: FileNotFoundError"), reply)
        self.assertIsNone(t.tools._play)

    # -- the squeezes, on the two fixtures of the 2026-11-07 player plan --------------

    def _refine_play(self, name):
        score = os.path.join(_PROBE_TESTS, f"{name}_score.json")
        schedule = os.path.join(_PROBE_TESTS, f"{name}_schedule.json")
        t, seen = self._decline_rig()
        reply = t.tools.ch_play_chart(score, schedule, route="keys")
        if t.tools._play is not None:
            t.tools._play.thread.join(THREAD_CAP_S)
        return reply, t

    def test_the_freak_reply_names_both_anchors_off_the_schedule(self):
        lines = self._refine_play("freak_limits")[0].splitlines()
        start = lines.index(FREAK_DECLINED)
        self.assertEqual(lines[start + 1:start + 4], [FREAK_SQUEEZES] + FREAK_ANCHORS)

    def test_the_mouths_reply_names_the_squeezes_and_the_declined_fills(self):
        lines = self._refine_play("mouths_refine")[0].splitlines()
        start = lines.index(MOUTHS_DECLINED)
        self.assertEqual(lines[start + 1:start + 5], [MOUTHS_SQUEEZES] + MOUTHS_ANCHORS)

    def test_the_bomber_reply_names_the_early_anchor(self):
        lines = self._refine_play("bomber_refine")[0].splitlines()
        start = lines.index(BOMBER_DECLINED)
        self.assertEqual(lines[start + 1:start + 3], [BOMBER_SQUEEZES, BOMBER_ANCHOR])

    def test_the_mouths_order_line_follows_the_anchor_lines(self):
        lines = self._refine_play("mouths_refine")[0].splitlines()
        line = lines[lines.index(MOUTHS_ANCHORS[-1]) + 1]
        self.assertTrue(line.startswith(MOUTHS_ORDER), line[:200])

    def test_the_bomber_order_line_follows_the_anchor_line(self):
        lines = self._refine_play("bomber_refine")[0].splitlines()
        line = lines[lines.index(BOMBER_ANCHOR) + 1]
        self.assertTrue(line.startswith(BOMBER_ORDER), line[:200])

    def test_a_long_order_line_names_every_split_chord(self):
        # Twelve made-up chords, a second apart, each a green cymbal then a
        # kick 8 ms later: two presses per chord, hand-built.
        schedule = schedule_of({"chords": [
            {"index": i, "ms": 1000.0 * (i + 1), "notes": [
                {"color": "Kick", "cymbal": False, "dynamic": "none"},
                {"color": "Green", "cymbal": True, "dynamic": "none"}]}
            for i in range(12)]})
        presses = []
        for press in schedule["presses"]:
            presses.append({**press, "gems": [1]})
            presses.append({**press, "gems": [0], "after_ms": 8.0, "aim_ms": press["aim_ms"] + 8.0})
        schedule["presses"] = presses
        chords, lines = GT.chords_to_play(self._schedule_file(schedule))
        self.assertEqual(GT.order_lines(chords, schedule),
                         ["order: chords "
                          + ", ".join(f"{i} (green cymbal, then kick 8 ms later)"
                                      for i in range(12))])
        self.assertEqual(lines, ["declined: chords none"] + GT.order_lines(chords, schedule))

    def test_a_chart_with_no_split_chord_gives_no_order_line(self):
        schedule = schedule_of({"chords": [
            {"index": 0, "ms": 1000.0, "notes": [
                {"color": "Kick", "cymbal": False, "dynamic": "none"},
                {"color": "Green", "cymbal": True, "dynamic": "none"}]}]})
        chords, _lines = GT.chords_to_play(self._schedule_file(schedule))
        self.assertEqual(GT.order_lines(chords, schedule), [])

    def _schedule_file(self, schedule):
        path = os.path.join(self.tmp, "order schedule.json")
        with open(path, "w", encoding="utf-8") as f:
            json.dump(schedule, f)
        return path

    def test_an_infeasible_squeeze_is_refused_naming_it(self):
        # Freak's schedule with its first squeeze marked not feasible:
        # load_schedule's refusal comes back whole, and no run starts.
        with open(os.path.join(_PROBE_TESTS, "freak_limits_schedule.json"),
                  encoding="utf-8") as f:
            schedule = json.load(f)
        schedule["activations"][0]["squeezes"][0]["feasible"] = False
        path = self._schedule_file(schedule)
        t, seen = self._decline_rig()
        reply = t.tools.ch_play_chart(os.path.join(_PROBE_TESTS, "freak_limits_score.json"),
                                      path, route="keys")
        self.assertTrue(reply.startswith(
            f"{path} holds a squeeze the game cannot make at 100% speed: chord 387 "
            "(activation 0, SqOut, "), reply)
        self.assertIsNone(t.tools._play)
        self.assertEqual(seen, [])

    def test_the_status_says_presses_and_chords_when_a_chord_is_split(self):
        # Two press rows of chord 740, as press_plan splits it.
        rows = [{**ROWS[0], "index": 740}, {**ROWS[1], "index": 740, "send": play_chart.SENT}]

        def live_play(chords, *, on_press, **kw):
            for row in rows:
                on_press(row)
            return play_chart.RunResult(stop="done", sent=2)

        t = ToolRig(self.tmp, live_play=live_play)
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(),route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertIn("sent 2 presses for 1 chord",
                      t.tools.ch_play_status().splitlines())

    def test_the_status_of_a_chord_sent_as_one_message_is_no_clock_jump(self):
        # play_chords sends a split chord's two order groups as one message:
        # two rows, one `sent`.
        rows = [{**ROWS[0], "index": 740, "send": play_chart.SENT}, {**ROWS[1], "index": 740, "send": play_chart.SENT}]

        def live_play(chords, *, on_press, **kw):
            for row in rows:
                on_press(row)
            return play_chart.RunResult(stop="done", sent=1,
                                        rows=list(rows))

        t = ToolRig(self.tmp, live_play=live_play)
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(),route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        lines = t.tools.ch_play_status().splitlines()
        self.assertIn("sent 1 messages (2 presses) for 1 chord",
                      lines)
        self.assertFalse([line for line in lines if "jumped back" in line])

    def test_every_lane_has_a_word_in_the_reply(self):
        self.assertEqual(set(GT._LANE_WORDS), set(Lane))

    def test_the_status_counts_a_declined_chord_apart_from_the_misses(self):
        row = {**ROWS[1], "send": play_chart.DECLINED}

        def live_play(chords, *, on_press, **kw):
            on_press(ROWS[0])
            on_press(row)
            return play_chart.RunResult(stop="done", sent=1, declined=1)

        t = ToolRig(self.tmp, live_play=live_play)
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(),route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        lines = t.tools.ch_play_status().splitlines()
        self.assertIn("sent 1, declined 1", lines)
        self.assertIn("  1, 1500.0, 1499.0, 50, 50, declined", lines)
        self.assertFalse([line for line in lines if "jumped back" in line])

    def test_the_running_status_does_not_count_a_declined_row_as_a_hit(self):
        release = threading.Event()

        def live_play(chords, *, on_press, **kw):
            on_press({**ROWS[1], "send": play_chart.DECLINED})
            delivered.set()
            release.wait(THREAD_CAP_S)
            return play_chart.RunResult(stop="done")

        delivered = threading.Event()
        t = ToolRig(self.tmp, live_play=live_play)
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(),route="keys")
        self.assertTrue(delivered.wait(THREAD_CAP_S))
        self.assertIn("running: 1 presses so far", t.tools.ch_play_status())
        release.set()
        t.tools._play.thread.join(THREAD_CAP_S)

    def test_the_status_rows_show_the_kit_columns_only_when_rows_carry_them(self):
        rows = [{**ROWS[0], "down_frame": 10, "up_frame": 12, "down_ms": 100, "up_ms": 113}]

        def live_play(chords, *, on_press, **kw):
            for row in rows:
                on_press(row)
            return play_chart.RunResult(stop="done", sent=1, rows=rows)

        t = ToolRig(self.tmp, kit=True, live_play=live_play)
        t.tools.ch_play_chart(self._score_json(), self._schedule_json())
        t.tools._play.thread.join(THREAD_CAP_S)
        lines = t.tools.ch_play_status().splitlines()
        self.assertEqual(lines[-2:], [
            "rows, newest first (index, chart_ms, clock_ms_at_press, score_before, score_after, "
            "send, down_frame, up_frame, down_ms, up_ms):",
            "  0, 1000.0, 998.0, 0, 50, sent, 10, 12, 100, 113"])

    def test_the_status_rows_show_the_timing_columns(self):
        # D335 item 2: the timing columns go into the status rows as well as
        # the file. No row reads an end, so end_read_took_ms is left out, as
        # every column no row carries is.
        rows = [{**ROWS[0], "wait_read_max_ms": 0.4, "wait_oversleep_max_ms": 1.2,
                 "press_took_ms": 7.6, "reply_check_took_ms": 0.1,
                 "score_read_took_ms": 0.3, "prev_handoff_took_ms": 0.05}]

        def live_play(chords, *, on_press, **kw):
            for row in rows:
                on_press(row)
            return play_chart.RunResult(stop="done", sent=1, rows=rows)

        t = ToolRig(self.tmp, live_play=live_play)
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(), route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        lines = t.tools.ch_play_status().splitlines()
        self.assertEqual(lines[-2:], [
            "rows, newest first (index, chart_ms, clock_ms_at_press, score_before, score_after, "
            "send, wait_read_max_ms, wait_oversleep_max_ms, press_took_ms, reply_check_took_ms, "
            "score_read_took_ms, prev_handoff_took_ms):",
            "  0, 1000.0, 998.0, 0, 50, sent, 0.4, 1.2, 7.6, 0.1, 0.3, 0.05"])

    # -- the chained start --------------------------------------------------------------

    ENGINE = 0x2_0000_1000

    class ChainFrida:
        """`loaded` is the song folder the game's GlobalVariables reads, as
        agent/menu.ts sends it; a StateUnread is raised by the read instead."""

        def __init__(self, events, attaches=True, loaded=None):
            self.events, self.attaches, self.loaded = events, attaches, loaded

        def ch_attach(self):
            self.events.append("attach")
            return "attached" if self.attaches else "the game is not running"

        def is_attached(self):
            return self.attaches

        def read_state(self):
            if isinstance(self.loaded, StateUnread):
                raise self.loaded
            return {MENU_BLOCK: {"globals": {"found": True, "currentSongFolder": self.loaded,
                                             "gameMode": 0, "playerCount": 1}}}

    class ChainPlayer:
        """kit_tools.GamePlayer as the play thread sees it: it is closed once
        the run ends."""

        def __init__(self):
            self.closed = False

        def close(self):
            self.closed = True

    class ChainKit:
        def __init__(self, events, press):
            self.events, self.press = events, press
            self.player = GameToolsTest.ChainPlayer()

        def setup_for_run(self, player):
            self.events.append(("setup", player))
            return (self.player if self.press is not None else None), "setup reply"

        def game_player(self):
            self.events.append("game player")
            return self.player

        def ch_kit_teardown(self):
            self.events.append("teardown")
            return "kit teardown: all checks ok"

    class ChainSp:
        def __init__(self, events, engine):
            self.events, self.engine = events, engine

        def watch_for_run(self, cap_s, find_engine, song_end_ms=None):
            self.events.append(("watch", cap_s, find_engine, song_end_ms))
            return self.engine, "csv: w.csv" if self.engine else "watcher ended: no song"

        def ch_sp_watch_stop(self):
            self.events.append("watch stop")
            return "watcher ended: stopped"

    def _chain_rig(self, live_play=None, *, attaches=True, press=None, engine=ENGINE,
                   loaded=None, **tool_args):
        """A chained-start rig whose game has `loaded` loaded; None means the
        rig's own song folder. `tool_args` go to GameTools as they are."""
        events = []
        seen = {}

        def played(chords, *, on_press, **kw):
            events.append("play")
            seen.update(kw, chords=chords, engine=kw["find_engine"]("a proc"))
            on_press(ROWS[0])
            return play_chart.RunResult(stop="done")

        press = press if press is not None else (lambda lanes, axes=None: {})
        t = ToolRig(self.tmp, kit=True, live_play=live_play or played, **tool_args)
        t.tools.use_run_parts(GT.RunParts(
            self.ChainFrida(events, attaches, t.r.song if loaded is None else loaded),
            self.ChainKit(events, press if press != "none" else None),
            self.ChainSp(events, engine)))
        return t, events, seen

    def _song_copy(self, schedule_path):
        """A copy of the schedule file at `schedule_path` made from this
        test's song, so a fixture schedule passes ch_run_chart's song checks.
        A path with no file stays as it is."""
        if not os.path.isfile(schedule_path):
            return schedule_path
        with open(schedule_path, encoding="utf-8") as f:
            schedule = json.load(f)
        path = os.path.join(self.tmp, "song " + os.path.basename(schedule_path))
        _write(path, json.dumps(for_song(schedule, self._song())).encode())
        return path

    def _run(self, t, score=None, schedule=None, **kw):
        reply = t.tools.ch_run_chart(t.r.song, score or self._score_json(),
                                     self._song_copy(schedule) if schedule else
                                     self._schedule_json(), "System", **kw)
        if t.tools._play is not None:
            t.tools._play.thread.join(THREAD_CAP_S)
        return reply

    def test_the_chain_runs_attach_setup_watch_play_in_that_order(self):
        t, events, _seen = self._chain_rig()
        self._run(t)
        self.assertEqual([e if isinstance(e, str) else e[0] for e in events],
                         ["attach", "setup", "watch", "play"])
        self.assertEqual(events[1], ("setup", "System"))

    def test_the_watch_gets_the_one_search_and_the_player_its_engine(self):
        t, events, seen = self._chain_rig()
        self._run(t)
        self.assertIs(events[2][2], GT.find_run_engine)
        self.assertEqual(seen["engine"], self.ENGINE)

    def test_the_run_plays_on_the_player_setup_gave_and_has_no_driver(self):
        t, _events, seen = self._chain_rig()
        self._run(t)
        self.assertEqual(t.live_in_game.players, [t.tools._run_parts.kit.player])
        self.assertNotIn("driver", seen)

    def test_the_watch_and_the_player_share_the_songs_run_cap(self):
        t, events, seen = self._chain_rig()
        schedule = self._schedule_json()
        cap_s, end_ms = GT.schedule_run_cap_s(schedule)
        dumps_before = len(FAKE.charts)
        t.tools.ch_run_chart(t.r.song, self._score_json(), schedule, "System")
        t.tools._play.thread.join(THREAD_CAP_S)
        # The cap and the watch's song end come from the run's schedule, so
        # the run makes no dump of its own.
        self.assertEqual(len(FAKE.charts), dumps_before)
        self.assertEqual(events[2][1], cap_s)
        self.assertEqual(events[2][3], end_ms)
        self.assertEqual(seen["cap_s"], cap_s)

    def test_the_search_is_the_auto_players_with_its_dots_kept_off_stdout(self):
        with mock.patch.object(play_chart, "find_normal_engine", return_value=self.ENGINE) as find:
            self.assertEqual(GT.find_run_engine("a proc"), self.ENGINE)
        self.assertEqual(find.call_args.args, ("a proc",))
        self.assertIsNot(find.call_args.kwargs["out"], None)

    def test_the_reply_has_a_line_per_part_and_names_the_first_press(self):
        t, _events, _seen = self._chain_rig()
        lines = self._run(t).splitlines()
        self.assertEqual([line.split(":")[0] for line in lines[:6]],
                         ["attach", "song check", "kit setup", "watch", "machine recorder",
                          "play"])
        self.assertRegex(lines[1], r"^song check: ok, .* s, " + re.escape(t.r.song) + " is loaded$")
        self.assertIn(f"engine {self.ENGINE:#x}, csv: w.csv", lines[3])
        self.assertEqual(lines[-1], "first press chord index 0 at 1000.0 ms")
        # The six parts, the schedule's declined line, the builds record's
        # line, the press log's line and the first press.
        self.assertEqual(lines[6], "declined: chords none")
        self.assertEqual(lines[7], "builds record: "
                         + os.path.join(t.tools.session_dir(), "builds-1.json"))
        self.assertEqual(len(lines), 10)
        self.assertEqual(lines[4], "machine recorder: none; this run is not recorded")

    # -- the press log file -------------------------------------------------------------

    @staticmethod
    def _log_table(path):
        with open(path, encoding="utf-8", newline="") as f:
            return list(csv.reader(f))

    def _rows_on_disk(self, path, count):
        """The data rows of the log at `path` once it holds `count` of them,
        read while the run goes on; fails after THREAD_CAP_S."""
        deadline = time.monotonic() + THREAD_CAP_S
        while True:
            rows = self._log_table(path)[1:] if os.path.exists(path) else []
            if len(rows) >= count or time.monotonic() > deadline:
                return rows
            time.sleep(0.005)

    def test_the_rows_reach_the_file_in_order_and_only_once(self):
        def live_play(chords, *, on_press, **kw):
            for i in range(200):
                on_press({**ROWS[0], "index": i})
            return play_chart.RunResult(stop="done", sent=200)

        t, _events, _seen = self._chain_rig(live_play)
        self._run(t)
        table = self._log_table(t.tools._play.log_path)
        self.assertEqual(table[0], list(PRESS_LOG_COLUMNS))
        self.assertEqual([r[0] for r in table[1:]], [str(i) for i in range(200)])

    def test_no_file_call_runs_on_the_player_thread(self):
        calls = []
        player = {}
        real_open = open
        real_writer = play_chart._press_log_writer

        class Recorded:
            """A file whose write and flush say which thread called them."""

            def __init__(self, f):
                self._f = f

            def __getattr__(self, name):
                return getattr(self._f, name)

            def __enter__(self):
                return self

            def __exit__(self, *exc):
                calls.append(("close", threading.get_ident()))
                return self._f.__exit__(*exc)

            def write(self, text):
                calls.append(("write", threading.get_ident()))
                return self._f.write(text)

            def flush(self):
                calls.append(("flush", threading.get_ident()))
                return self._f.flush()

        def recorded_open(*args, **kwargs):
            calls.append(("open", threading.get_ident()))
            return Recorded(real_open(*args, **kwargs))

        def recorded_writer(f):
            calls.append(("_press_log_writer", threading.get_ident()))
            return real_writer(f)

        def live_play(chords, *, on_press, **kw):
            player["ident"] = threading.get_ident()
            for row in ROWS:
                on_press(row)
            return play_chart.RunResult(stop="done", sent=2)

        t = ToolRig(self.tmp, live_play=live_play)
        path = os.path.join(self.tmp, "named.csv")
        with mock.patch.object(play_chart, "open", create=True, new=recorded_open), \
                mock.patch.object(play_chart, "_press_log_writer", new=recorded_writer):
            t.tools.ch_play_chart(self._score_json(), self._schedule_json(), log=path,
                                  route="keys")
            t.tools._play.thread.join(THREAD_CAP_S)
        self.assertEqual({name for name, _ident in calls},
                         {"open", "_press_log_writer", "write", "flush", "close"})
        self.assertNotIn(player["ident"], {ident for _name, ident in calls})
        self.assertEqual([r[0] for r in self._log_table(path)[1:]], ["0", "1"])

    def test_the_reply_names_the_press_file_under_the_session_folder(self):
        t, _events, _seen = self._chain_rig()
        lines = self._run(t).splitlines()
        log = os.path.join(t.tools.session_dir(), "press-1.csv")
        self.assertEqual(t.tools._play.log_path, log)
        # The fake's run may already have ended, and then the line counts the rows.
        self.assertEqual(lines[-2].split(" (")[0], f"press log: {log}")

    def test_each_run_of_a_session_gets_its_own_press_file(self):
        t, _events, _seen = self._chain_rig()
        self._run(t)
        first = t.tools._play.log_path
        self._run(t)
        self.assertEqual(os.path.dirname(t.tools._play.log_path), os.path.dirname(first))
        self.assertEqual(os.path.basename(t.tools._play.log_path), "press-2.csv")
        self.assertEqual(len(self._log_table(first)), 2)

    def test_the_status_names_the_file_first_while_running_and_after(self):
        release = threading.Event()

        def live_play(chords, *, on_press, **kw):
            on_press(ROWS[0])
            release.wait(THREAD_CAP_S)
            return play_chart.RunResult(stop="done", sent=1)

        t, _events, _seen = self._chain_rig(live_play)
        t.tools.ch_run_chart(t.r.song, self._score_json(), self._schedule_json(), "System")
        log = t.tools._play.log_path
        self.assertEqual(t.tools.ch_play_status().splitlines()[0], f"press log: {log}")
        release.set()
        t.tools._play.thread.join(THREAD_CAP_S)
        lines = t.tools.ch_play_status().splitlines()
        self.assertEqual(lines[0], f"press log: {log} (1 rows)")
        self.assertEqual(len([line for line in lines if log in line]), 1)

    def test_a_named_log_is_written_row_by_row_too(self):
        seen = []
        path = os.path.join(self.tmp, "named.csv")

        def live_play(chords, *, on_press, **kw):
            on_press(ROWS[0])
            seen.append(len(self._rows_on_disk(path, 1)))
            return play_chart.RunResult(stop="done", sent=1)

        t = ToolRig(self.tmp, live_play=live_play)
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(),log=path, route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertEqual(seen, [1])

    def test_a_run_reusing_a_log_name_starts_the_file_over(self):
        def live_play(chords, *, on_press, **kw):
            for row in ROWS:
                on_press(row)
            return play_chart.RunResult(stop="done", sent=2)

        t = ToolRig(self.tmp, live_play=live_play)
        path = os.path.join(self.tmp, "named.csv")
        for _ in range(2):
            t.tools.ch_play_chart(self._score_json(), self._schedule_json(),log=path, route="keys")
            t.tools._play.thread.join(THREAD_CAP_S)
        self.assertEqual(len(self._log_table(path)), 3)

    def test_a_run_with_no_press_leaves_a_file_with_the_header(self):
        t = ToolRig(self.tmp, live_play=lambda chords, **kw: play_chart.RunResult(stop="done"))
        path = os.path.join(self.tmp, "named.csv")
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(),log=path, route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertEqual(self._log_table(path), [list(PRESS_LOG_COLUMNS)])

    def test_a_failed_write_goes_on_and_the_end_writes_every_row_once(self):
        path = os.path.join(self.tmp, "named.csv")
        said = []
        tried = []
        real_writer = play_chart._press_log_writer

        class Failing:
            """The real writer, whose rows fail as a file held open would."""

            def __init__(self, f):
                self._real = real_writer(f)
                self.writeheader, self.writerows = self._real.writeheader, self._real.writerows

            def writerow(self, row):
                tried.append(row["index"])
                raise PermissionError("held open")

        def live_play(chords, *, on_press, **kw):
            for row in ROWS:
                on_press(row)
            deadline = time.monotonic() + THREAD_CAP_S
            while t.tools._play.writer.error is None and time.monotonic() < deadline:
                time.sleep(0.005)
            said.append(t.tools.ch_play_status().splitlines()[0])
            return play_chart.RunResult(stop="done", sent=2)

        t = ToolRig(self.tmp, live_play=live_play)
        with mock.patch.object(play_chart, "_press_log_writer", new=Failing):
            t.tools.ch_play_chart(self._score_json(), self._schedule_json(),log=path, route="keys")
            t.tools._play.thread.join(THREAD_CAP_S)
        self.assertEqual(tried, [0])
        self.assertEqual(said, [f"press log not written to {path}: held open"])
        self.assertEqual([r[0] for r in self._log_table(path)[1:]], ["0", "1"])
        self.assertEqual(t.tools.ch_play_status().splitlines()[0],
                         f"press log: {path} (2 rows)")

    def test_the_end_waits_for_the_writer_and_says_so_when_the_cap_passes(self):
        path = os.path.join(self.tmp, "named.csv")
        held = threading.Event()
        self.addCleanup(held.set)
        real_writer = play_chart._press_log_writer

        class Held:
            """The real writer, whose rows wait until `held` is set."""

            def __init__(self, f):
                self._real = real_writer(f)
                self.writeheader, self.writerows = self._real.writeheader, self._real.writerows

            def writerow(self, row):
                held.wait(THREAD_CAP_S)
                return self._real.writerow(row)

        def live_play(chords, *, on_press, **kw):
            for row in ROWS:
                on_press(row)
            return play_chart.RunResult(stop="done", sent=2)

        t = ToolRig(self.tmp, live_play=live_play)
        with mock.patch.object(play_chart, "_press_log_writer", new=Held), \
                mock.patch.object(GT.C, "LOG_WRITER_CAP_S", 0.05):
            t.tools.ch_play_chart(self._score_json(), self._schedule_json(), log=path,
                                  route="keys")
            t.tools._play.thread.join(THREAD_CAP_S)
            lines = t.tools.ch_play_status().splitlines()
        self.assertEqual(lines[0], f"press log: {path} may lack its last rows: its writer "
                                   f"did not finish within 0.05 s; this status holds all "
                                   f"2 rows")
        # The rows stay in memory for the status reply.
        self.assertIn("  0, 1000.0, 998.0, 0, 50, sent", lines)

        # A writer that finishes inside the cap leaves every row in the file.
        late = t.tools._play.writer
        held.set()
        self.assertTrue(late.stop(THREAD_CAP_S))
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(), log=path,
                              route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertEqual(t.tools.ch_play_status().splitlines()[0],
                         f"press log: {path} (2 rows)")
        self.assertEqual([r[0] for r in self._log_table(path)[1:]], ["0", "1"])

    def test_a_log_nowhere_writable_is_said_and_the_run_keeps_its_rows(self):
        path = os.path.join(self.tmp, "no such folder", "named.csv")

        def live_play(chords, *, on_press, **kw):
            on_press(ROWS[0])
            return play_chart.RunResult(stop="done", sent=1)

        t = ToolRig(self.tmp, live_play=live_play)
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(),log=path, route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        lines = t.tools.ch_play_status().splitlines()
        self.assertTrue(lines[0].startswith(f"press log not written to {path}: "), lines[0])
        self.assertIn("  0, 1000.0, 998.0, 0, 50, sent", lines)

    def test_with_no_log_the_status_opens_with_the_session_line(self):
        t = ToolRig(self.tmp, live_play=lambda chords, **kw: play_chart.RunResult(stop="done"))
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(),route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertTrue(t.tools.ch_play_status().startswith("play session: "))

    def test_a_later_first_press_is_named(self):
        def live_play(chords, *, on_press, **kw):
            on_press(ROWS[1])
            return play_chart.RunResult(stop="done")

        t, _events, _seen = self._chain_rig(live_play)
        self.assertEqual(self._run(t).splitlines()[-1],
                         "first press chord index 1 at 1500.0 ms")

    def test_a_decline_reaches_the_chords_and_the_reply_names_them(self):
        # chords_to_play is what ch_run_chart loads its chords and reply lines
        # through. The schedule is hydra_replay's (see _PROBE_TESTS above).
        path = os.path.join(_PROBE_TESTS, "mouths_refine_schedule.json")
        with open(path, encoding="utf-8") as f:
            doc = json.load(f)
        chords, lines = GT.chords_to_play(path)
        rows = {row["index"]: row for row in doc["chords"]}
        declined = [c for c in chords if c.declined]
        self.assertTrue(declined, "the fixture declines at least one chord")
        self.assertEqual([c.index for c in declined],
                         [i for i, row in rows.items() if row["declined"]])
        for chord in declined:
            kept_lanes = sorted(chord.lanes[k] for k in rows[chord.index]["kept_gems"])
            sent_lanes = sorted(lane for p in chord.presses for lane in p.lanes)
            self.assertEqual(sent_lanes, kept_lanes, chord.index)
        said = next(line for line in lines if line.startswith("declined: chords "))
        named = re.findall(r"(\d+) \(", said)
        self.assertEqual([int(i) for i in named], [c.index for c in declined])

    def _refine_run_lines(self, name):
        """The reply lines after the play's line of a chained run of a fixture."""
        t, _events, _seen = self._chain_rig()
        lines = self._run(t, os.path.join(_PROBE_TESTS, f"{name}_score.json"),
                          os.path.join(_PROBE_TESTS, f"{name}_schedule.json"))
        lines = lines.splitlines()
        start = next(i for i, line in enumerate(lines) if line.startswith("play: ok"))
        return lines[start + 1:]

    def test_the_mouths_run_reply_names_its_squeezes_after_the_declined_line(self):
        lines = self._refine_run_lines("mouths_refine")
        self.assertEqual(lines[:5], [MOUTHS_DECLINED, MOUTHS_SQUEEZES] + MOUTHS_ANCHORS)
        self.assertTrue(lines[5].startswith(MOUTHS_ORDER), lines[5][:200])
        self.assertTrue(lines[6].startswith("builds record: "), lines[6])
        self.assertTrue(lines[7].startswith("press log: "), lines[7])

    def test_the_freak_run_reply_names_its_anchors(self):
        lines = self._refine_run_lines("freak_limits")
        self.assertEqual(lines[:4], [FREAK_DECLINED, FREAK_SQUEEZES] + FREAK_ANCHORS)

    def test_the_bomber_run_reply_names_its_early_anchor(self):
        lines = self._refine_run_lines("bomber_refine")
        self.assertEqual(lines[:3], [BOMBER_DECLINED, BOMBER_SQUEEZES, BOMBER_ANCHOR])
        self.assertTrue(lines[3].startswith(BOMBER_ORDER), lines[3][:200])
        self.assertTrue(lines[4].startswith("builds record: "), lines[4])
        self.assertTrue(lines[5].startswith("press log: "), lines[5])

    def test_decline_mode_and_dump_json_are_not_run_arguments(self):
        t, events, _seen = self._chain_rig()
        for extra in ({"decline_mode": "nothing"}, {"dump_json": "d.json"}):
            with self.assertRaises(TypeError):
                self._run(t, **extra)
        self.assertEqual(events, [])

    def test_a_schedule_it_cannot_read_stops_before_the_attach(self):
        t, events, _seen = self._chain_rig()
        missing = os.path.join(self.tmp, "none.json")
        reply = self._run(t, schedule=missing)
        self.assertTrue(reply.startswith(f"could not read {missing}: FileNotFoundError"), reply)
        self.assertEqual(events, [])

    def test_a_run_that_never_presses_ends_at_the_run_cap_and_says_so(self):
        def live_play(chords, *, should_stop, **kw):
            while not should_stop():
                time.sleep(0.005)
            return play_chart.RunResult(stop="stopped")

        t, events, _seen = self._chain_rig(live_play)
        with mock.patch.object(GT, "cap_for_end", return_value=0.05):
            reply = t.tools.ch_run_chart(t.r.song, self._score_json(), self._schedule_json(), "System")
        self.assertIn("no press inside the run cap of 0.05 s; the call stopped the run", reply)
        self.assertEqual(events[-2:], ["teardown", "watch stop"])
        self.assertFalse(t.tools._play.running())

    def test_the_tools_take_schedule_json_and_no_dump_json(self):
        # Question 6 (a) of docs/superpowers/plans/2026-11-08-replay-press-
        # schedule.md and D324 item 2: schedule_json replaces dump_json, and
        # no decline mode or lead is an argument. D318 item 4: ch_play_chart
        # and ch_run_chart take path_index. D342: neither takes timing.
        signatures = {name: list(inspect.signature(getattr(GT.GameTools, name)).parameters)
                      for name in ("ch_play_chart", "ch_run_chart", "ch_edge_probe")}
        self.assertEqual(signatures, {
            "ch_play_chart": ["self", "score_json", "schedule_json", "log", "song_folder",
                              "route", "path_index"],
            "ch_run_chart": ["self", "song_folder", "score_json", "schedule_json",
                             "kit_player", "path_index", "probes_json"],
            "ch_edge_probe": ["self", "song_folder", "score_json", "schedule_json",
                              "probes_json"]})

    # -- in-game timing

    def test_game_tools_names_no_kit_press_source_and_no_timing(self):
        # D342: the kit route is timed by the game only.
        with open(GT.__file__, encoding="utf-8") as f:
            source = f.read()
        for gone in ("use_kit_press", "_kit_press_source", "TIMING_", "timing:", "timing="):
            self.assertNotIn(gone, source)

    def test_the_status_rows_show_frame_gap_ms_and_agent_note(self):
        # In-game rows carry frame_gap_ms and agent_note, and ch_play_status
        # prints both for each row. The rows are the first three of a real
        # in-game run's press log (testdata/ch_runs/ls-g2-run1/press-1.csv,
        # copied from hydra-data/ch-mcp-runs/2026-11-12/ls-g2-run1).
        path = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-g2-run1", "press-1.csv")
        with open(path, encoding="utf-8", newline="") as f:
            rows = list(csv.DictReader(f))
        self.assertTrue(rows)
        tools = GT.GameTools(None, threading.RLock(), os.path.join(self.tmp, "scratch"))
        tools._play = GT._PlaySession(score_json="score.json", chord_count=len(rows),
                                      log_path=None, rows=rows)
        lines = tools.ch_play_status().splitlines()
        start = next(i for i, line in enumerate(lines) if line.startswith("rows, newest first ("))
        columns = lines[start][len("rows, newest first ("):-len("):")].split(", ")
        self.assertIn("frame_gap_ms", columns)
        self.assertIn("agent_note", columns)
        printed = [line[2:].split(", ") for line in lines[start + 1:]]
        self.assertEqual(len(printed), len(rows))
        for cells, row in zip(printed, reversed(rows)):
            for column in ("index", "frame_gap_ms", "agent_note"):
                self.assertEqual(cells[columns.index(column)], row[column], column)

    # -- the right song and path (D318 item 4) --------------------------------------------

    def test_run_chart_refuses_when_the_loaded_folder_is_not_the_schedules_chart(self):
        # The game's read is a real one: the first state read of a run
        # (testdata/ch_runs/ls-g2-run1/state-2.jsonl, copied from
        # hydra-data/ch-mcp-runs/2026-11-12/ls-g2-run1), with another song
        # loaded than this test's.
        path = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-g2-run1", "state-2.jsonl")
        with open(path, encoding="utf-8") as f:
            state = json.loads(f.readline())["payload"]
        loaded = menu_tools.loaded_folder(state)
        self.assertTrue(loaded)
        t, events, _seen = self._chain_rig()
        t.tools._run_parts.frida.read_state = lambda: state
        lines = self._run(t).splitlines()
        # The attach ran; no kit was built, no watch started, nothing pressed.
        self.assertEqual(events, ["attach"])
        self.assertIsNone(t.tools._play)
        self.assertTrue(lines[1].startswith("song check: FAILED"), lines[1])
        self.assertIn(json.dumps(loaded), lines[2])
        self.assertIn(t.r.song, lines[2])
        self.assertTrue(lines[2].endswith("nothing pressed"), lines[2])

    def test_run_chart_refuses_when_the_read_fails(self):
        # The failed read is FridaTools' own, from one that never attached.
        t, events, _seen = self._chain_rig()
        frida = FT.FridaTools(t.tools, threading.RLock())
        with self.assertRaises(StateUnread) as unread:
            frida.read_state()
        t.tools._run_parts.frida.read_state = frida.read_state
        lines = self._run(t).splitlines()
        self.assertEqual(events, ["attach"])
        self.assertIsNone(t.tools._play)
        self.assertTrue(lines[1].startswith("song check: FAILED"), lines[1])
        self.assertTrue(lines[2].startswith(f"no read came back: {unread.exception}"), lines[2])

    # The refusals below come before the attach, so they run on the server's
    # own tools (server.Helper, the wiring the MCP server uses) and on real
    # hydra_replay schedules. EFL_* are the early-fill live check's prep
    # files, copied from hydra-data/ch-mcp-runs/2026-11-12/efl-prep: its
    # schedule was made from path 1 of its dump.
    EFL_DIR = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "efl-prep")
    EFL_SCHEDULE = os.path.join(EFL_DIR, "schedule.json")
    EFL_SCORE = os.path.join(EFL_DIR, "score.json")

    def _server_tools(self) -> GT.GameTools:
        """The GameTools of a real server.Helper on this test's temp folder,
        with its real run parts handed in, except the attach, which fails the
        test: ch_run_chart reaches the game only through it, and a unit test
        must never attach to the user's running Clone Hero."""
        helper = server.Helper(scratch_root=os.path.join(self.tmp, "server"))

        def never_attach(*_args, **_kwargs):
            raise AssertionError("a refusal stopped refusing: ch_run_chart reached the "
                                 "attach, which would go to a running Clone Hero")
        helper.frida.ch_attach = never_attach
        return helper.game

    def _assert_started_nothing(self, tools: GT.GameTools):
        """No play, no press log and no builds record: the run never reached
        the attach."""
        self.assertIsNone(tools._play)
        self.assertEqual(tools._press_log_count, 0)
        self.assertEqual(glob.glob(os.path.join(tools.session_dir(), GT.BUILDS_PREFIX + "*")),
                         [])

    @staticmethod
    def _schedule_source(path):
        """The path index and dump file the schedule at `path` says it was
        made from (its path_source, as hydra_replay schedule writes it)."""
        source = GT.schedule_doc(path)["path_source"]
        return source["index"], source["file"]

    def test_run_chart_refuses_a_song_folder_that_is_not_the_schedules_before_the_attach(self):
        tools = self._server_tools()
        index, _dump = self._schedule_source(self.EFL_SCHEDULE)
        chart_folder = GT.schedule_chart_folder(GT.schedule_doc(self.EFL_SCHEDULE),
                                                self.EFL_SCHEDULE)
        other = os.path.join(self.tmp, "another song")
        reply = tools.ch_run_chart(other, self.EFL_SCORE, self.EFL_SCHEDULE, "System",
                                   path_index=index)
        self.assertTrue(reply.startswith("refused: "), reply)
        self.assertIn(f"song_folder {other} ", reply)
        self.assertIn(chart_folder, reply)
        self.assertTrue(reply.endswith("nothing started"), reply)
        self._assert_started_nothing(tools)

    def test_run_chart_refuses_a_schedule_that_names_its_chart_by_a_relative_path(self):
        # Pitch Dark's schedule in testdata/replay was made by prep_inputs.py
        # with a relative --chart (the command is in the comment above
        # _REPLAY_DIR in tools/ch_probe/tests/test_sp_compare.py), so its
        # chart says nothing about where the song folder is.
        path = os.path.join(_REPO_ROOT, "testdata", "replay",
                            "test_pitch_dark_ratio-schedule.json")
        chart = GT.schedule_doc(path)["chart"]
        self.assertFalse(os.path.isabs(chart), chart)
        index, _dump = self._schedule_source(path)
        tools = self._server_tools()
        reply = tools.ch_run_chart(os.path.join(self.tmp, "song"), self.EFL_SCORE, path,
                                   "System", path_index=index)
        self.assertTrue(reply.startswith(f"refused: {path} "), reply)
        self.assertIn(chart, reply)
        self.assertTrue(reply.endswith("nothing started"), reply)
        self._assert_started_nothing(tools)

    def test_path_index_must_match_path_source(self):
        index, dump = self._schedule_source(self.EFL_SCHEDULE)
        other = 0 if index != 0 else 1
        tools = self._server_tools()
        song = os.path.join(self.tmp, "another song")
        # Asked for another path, both tools refuse, naming the schedule's
        # own path and the one asked for, and start nothing.
        for reply in (tools.ch_run_chart(song, self.EFL_SCORE, self.EFL_SCHEDULE, "System",
                                         path_index=other),
                      tools.ch_play_chart(self.EFL_SCORE, self.EFL_SCHEDULE, route="keys",
                                          path_index=other)):
            with self.subTest(reply=reply):
                self.assertTrue(reply.startswith(f"refused: {self.EFL_SCHEDULE} "), reply)
                self.assertIn(f"path {index} of {dump}", reply)
                self.assertIn(f"path_index is {other}", reply)
                self.assertTrue(reply.endswith("nothing started"), reply)
                self._assert_started_nothing(tools)
        # Asked for its own path, the path check passes: the next check, the
        # song folder's, is the one that refuses.
        reply = tools.ch_run_chart(song, self.EFL_SCORE, self.EFL_SCHEDULE, "System",
                                   path_index=index)
        self.assertNotIn("path_index", reply)
        self.assertTrue(reply.startswith(f"refused: song_folder {song} "), reply)
        self._assert_started_nothing(tools)

    def test_builds_json_in_the_run_folder(self):
        t, _events, _seen = self._chain_rig()
        path = self._schedule_json(dict(SCHEDULE_JSON, hydra_replay_build="Nov  8 2026 17:56:03"))
        self._run(t, schedule=path)
        with open(os.path.join(t.tools.session_dir(), "builds-1.json"), encoding="utf-8") as f:
            record = json.load(f)
        # The chained start loads the state agent and the kit agent; the
        # server did not launch this game for a human player, so no launch agent.
        self.assertEqual([a["file"] for a in record["agents"]], ["state.js", "kit.js"])
        self.assertEqual(record["hydra_replay_build"], "Nov  8 2026 17:56:03")
        # A schedule with no build field says so in the next run's file.
        self._run(t)
        with open(os.path.join(t.tools.session_dir(), "builds-2.json"), encoding="utf-8") as f:
            self.assertEqual(json.load(f)["hydra_replay_build"], build_stamp.NO_REPLAY_BUILD)

    def test_a_human_launch_adds_the_launch_agent_to_the_builds_record(self):
        self.assertEqual([os.path.basename(p) for p in GT.run_agent_paths("x/launch.js")],
                         ["launch.js", "state.js", "kit.js"])

    # -- probes (task T2 of docs/superpowers/plans/2026-11-12-early-fill-live-check.md) ---

    FREAK_SCHEDULE = os.path.join(_PROBE_TESTS, "freak_limits_schedule.json")

    def _probe_chain_rig(self):
        """A chained-start rig whose player gives one row per press, at
        exactly the time press_plan gives it, and records the chords it got."""
        seen = {}

        def live_play(chords, *, on_press, **kw):
            seen["chords"] = list(chords)
            for press in play_chart.press_plan(chords):
                on_press({"index": press.index, "chart_ms": press.chart_ms,
                          "clock_ms_at_press": press.at_ms, "lead_ms": press.lead_ms,
                          "score_before": 0, "score_after": 50, "send": play_chart.SENT})
            return play_chart.RunResult(stop="done", sent=len(chords))

        t, events, _seen = self._chain_rig(live_play)
        return t, events, seen

    def _probes_file(self, probes):
        path = os.path.join(self.tmp, "probes.json")
        _write(path, json.dumps(probes).encode())
        return path

    def _probe_refusal(self, schedule_path, probes):
        """ch_run_chart's refusal of `probes` on the schedule at
        `schedule_path`, in edge_probe.probe_chords' own words."""
        chords, _lines = GT.chords_to_play(schedule_path)
        with self.assertRaises(ValueError) as caught:
            edge_probe.probe_chords(chords, [edge_probe.Probe(p["index"], float(p["lead_ms"]))
                                             for p in probes])
        return f"refused: ValueError: {caught.exception}; nothing started"

    def test_run_chart_with_probes_says_the_probes_line_after_the_plays(self):
        t, _events, _seen = self._probe_chain_rig()
        probes = self._probes_file([{"index": 1, "lead_ms": 84}])
        lines = self._run(t, probes_json=probes).splitlines()
        play = [i for i, line in enumerate(lines) if line.startswith("play: ok")][0]
        self.assertEqual(lines[play + 1:play + 2],
                         edge_probe.probe_lines(edge_probe.load_probes(probes)))

    def _run_own_song(self, tools, schedule, score, probes_json):
        """ch_run_chart on `schedule`'s own song folder and path, so its song
        and path checks pass and the probes file is what it reads next."""
        index, _dump = self._schedule_source(schedule)
        song = GT.schedule_chart_folder(GT.schedule_doc(schedule), schedule)
        return tools.ch_run_chart(song, score, schedule, "System", path_index=index,
                                  probes_json=probes_json)

    def test_run_chart_refuses_a_probe_chord_not_in_the_schedule_before_the_attach(self):
        chords, _lines = GT.chords_to_play(self.EFL_SCHEDULE)
        absent = max(chord.index for chord in chords) + 1
        probes = [{"index": absent, "lead_ms": 84}]
        tools = self._server_tools()
        reply = self._run_own_song(tools, self.EFL_SCHEDULE, self.EFL_SCORE,
                                   self._probes_file(probes))
        self.assertEqual(reply, self._probe_refusal(self.EFL_SCHEDULE, probes))
        self.assertIn(str(absent), reply)
        self._assert_started_nothing(tools)

    def test_run_chart_refuses_a_probe_on_a_chord_timed_from_a_star_power_end(self):
        # The chord is the first one Freak's schedule times from a Star Power
        # end (Press.timed_from_end); the schedule holds several.
        chords, _lines = GT.chords_to_play(self.FREAK_SCHEDULE)
        from_end = [chord.index for chord in chords
                    if any(press.timed_from_end is not None for press in chord.presses)]
        self.assertTrue(from_end, "Freak's schedule times no press from a Star Power end")
        probes = [{"index": from_end[0], "lead_ms": 84}]
        tools = self._server_tools()
        reply = self._run_own_song(tools, self.FREAK_SCHEDULE,
                                   os.path.join(_PROBE_TESTS, "freak_limits_score.json"),
                                   self._probes_file(probes))
        self.assertEqual(reply, self._probe_refusal(self.FREAK_SCHEDULE, probes))
        self._assert_started_nothing(tools)

    def test_run_chart_refuses_a_probes_file_load_probes_refuses(self):
        # One probe object, not a list of them.
        path = self._probes_file({"index": 5, "lead_ms": 84})
        with self.assertRaises(ValueError) as caught:
            edge_probe.load_probes(path)
        tools = self._server_tools()
        reply = self._run_own_song(tools, self.EFL_SCHEDULE, self.EFL_SCORE, path)
        self.assertEqual(reply, f"refused: ValueError: {caught.exception}; nothing started")
        self._assert_started_nothing(tools)

    def test_with_no_parts_handed_in_it_refuses_and_touches_nothing(self):
        # A GameTools on its own, before the server hands in the run's parts.
        tools = GT.GameTools(B.Env(), threading.RLock(), os.path.join(self.tmp, "scratch"))
        index, _dump = self._schedule_source(self.EFL_SCHEDULE)
        song = GT.schedule_chart_folder(GT.schedule_doc(self.EFL_SCHEDULE), self.EFL_SCHEDULE)
        reply = tools.ch_run_chart(song, self.EFL_SCORE, self.EFL_SCHEDULE, path_index=index)
        self.assertTrue(reply.startswith("refused: "), reply)
        self.assertIn(GT.GameTools.use_run_parts.__name__, reply)
        self._assert_started_nothing(tools)

    def test_foreground_line_is_none_when_the_server_did_not_start_the_game(self):
        tools = self._server_tools()
        self.assertIsNone(tools.foreground_line())


# A settings.ini with the run's line in its [game] section.
USER_SETTINGS = b"[audio]\r\nvolume = 1\r\n\r\n[game]\r\npause_on_focus_lost = 1\r\nnote_speed = 7\r\n"


class RunWithoutFocusTest(unittest.TestCase):
    """ch_start's settings step, and the registry key's backup and restore."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def _pause_setting(self, settings: bytes):
        """ch_start's settings step (GameTools._pause_setting) on a real Env
        whose documents folder holds `settings` as its settings.ini: its
        reply line and the file afterwards."""
        documents = os.path.join(self.tmp, "documents")
        path = os.path.join(documents, G.SETTINGS)
        os.makedirs(documents)
        _write(path, settings)
        tools = GT.GameTools(B.Env(documents_dir=documents), threading.RLock(),
                             os.path.join(self.tmp, "scratch"))
        return tools._pause_setting(), path

    def test_the_settings_line_is_set_to_its_run_value_and_nothing_else_changes(self):
        key, value = G.PAUSE_ON_FOCUS_LOST, G.PAUSE_ON_FOCUS_LOST_RUN_VALUE
        check = os.path.join(self.tmp, "before " + G.SETTINGS)
        _write(check, USER_SETTINGS)
        self.assertNotEqual(G.get_game_setting(check, key), value)
        said, path = self._pause_setting(USER_SETTINGS)
        self.assertEqual(G.get_game_setting(path, key), value)
        self.assertTrue(said.startswith(f"{G.SETTINGS}: {key} set to {value} "), said)
        # Only the line's value changed: the other lines and sections stay.
        before = [line for line in USER_SETTINGS.splitlines() if not line.startswith(key.encode())]
        after = [line for line in _read(path).splitlines() if not line.startswith(key.encode())]
        self.assertEqual(after, before)

    def test_a_settings_file_without_the_line_is_left_alone_and_named(self):
        key, value = G.PAUSE_ON_FOCUS_LOST, G.PAUSE_ON_FOCUS_LOST_RUN_VALUE
        settings = b"[audio]\r\nvolume = 1\r\n\r\n[game]\r\nnote_speed = 7\r\n"
        said, path = self._pause_setting(settings)
        self.assertEqual(_read(path), settings)
        self.assertTrue(said.startswith(f"{G.SETTINGS}: {key} not set to {value} "), said)
        with self.assertRaises(G.SettingNotFound) as missing:
            G.get_game_setting(path, key)
        self.assertIn(f"({missing.exception})", said)


# The launch agent's status replies from a real human launch, copied from
# hydra-data/ch-mcp-runs/2026-11-06/ls-d2a-run1/launch-20261106-194201.jsonl:
# its idle, waiting and two patched replies, the second after the game's
# command-line loader ran once. The waiting reply that lists the modules is
# left out.
# Sample 615 of the process list a machine recorder took in ls-g3-run1, while
# a build was linking (link.exe). Copied from hydra-data/ch-mcp-runs/
# 2026-11-12/ls-g3-run1/machine-1/processes.csv, header and that sample only.
BUILD_PROCESSES = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-g3-run1",
                               "machine-1-processes-sample-615.csv")


def build_processes() -> list:
    """BUILD_PROCESSES as the quiet check's process list."""
    with open(BUILD_PROCESSES, encoding="utf-8", newline="") as f:
        return [Q.Proc(row["name"], int(row["pid"]), None) for row in csv.DictReader(f)]


def real_quiet_env(processes=None) -> Q.QuietEnv:
    """The quiet check's real readers, which setUpModule's stand-in in
    Q.LIVE_ENV hides; with `processes`, that list in place of this machine's."""
    listed = Q.list_processes if processes is None else (lambda: processes)
    return Q.QuietEnv(list_processes=listed, sample_cpu=Q.sample_cpu,
                      scan_folders=Q.scan_folders, free_memory_bytes=Q.free_memory_bytes,
                      power_scheme=Q.power_scheme)


def refusal_parts() -> tuple:
    """quiet.refusal_text's wording before and after a report's lines."""
    prefix, suffix = Q.refusal_text(Q.QuietReport(False, ("\0",))).split("\0")
    return prefix, suffix


D2A_LAUNCH_LOG = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "2026-11-06-ls-d2a-run1",
                              "launch-20261106-194201.jsonl")


def _recorded_statuses() -> list:
    with open(D2A_LAUNCH_LOG, encoding="utf-8") as f:
        return [json.loads(line)["payload"] for line in f if line.strip()]


class HumanStartTest(unittest.TestCase):
    """ch_start(human=True)'s refusal on real tools, and ch_status's launch
    lines from the launch agent's recorded replies."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def test_human_with_no_song_folder_is_refused_naming_the_reason(self):
        # A real Env with no registry runner and a game exe that is not there,
        # so a refusal that broke could not start the game or touch the key.
        documents = os.path.join(self.tmp, "documents")
        os.makedirs(documents)
        scratch_root = os.path.join(self.tmp, "scratch")
        tools = GT.GameTools(B.Env(exe=os.path.join(self.tmp, "no game.exe"),
                                   documents_dir=documents, run_reg=None),
                             threading.RLock(), scratch_root)
        reply = tools.ch_start(human=True)
        self.assertIsNotNone(B.human_refusal(True, None))
        self.assertEqual(reply, B.human_refusal(True, None))
        # Nothing ran: no backup folder and no marker under the scratch root,
        # and nothing is held open.
        left = os.listdir(scratch_root) if os.path.isdir(scratch_root) else []
        self.assertEqual(left, [])
        self.assertEqual(tools.open_lines(), [])

    def test_ch_status_launch_lines_from_the_recorded_replies(self):
        patch = B._read_patch(B.LAUNCH_PATCH_FILE)
        statuses = _recorded_statuses()
        self.assertEqual([s["state"] for s in statuses],
                         [B.STATE_IDLE, B.STATE_WAITING, B.STATE_PATCHED, B.STATE_PATCHED])
        # The loader had not run in the first three replies. In the last it
        # had run once and read the bytes launch_patch.json writes.
        words = [B.LOADER_NOT_YET, B.LOADER_NOT_YET, B.LOADER_NOT_YET, B.LOADER_YES]
        for status, word in zip(statuses, words):
            with self.subTest(id=status["id"]):
                lines = B.launch_status_lines(status, patch)
                self.assertTrue(lines[0].startswith(f"launch patch: {status['state']};"), lines)
                self.assertEqual(lines[-1], B.LOADER_LINE + word)
                # Only the idle reply says the agent will never patch.
                never = [line for line in lines if "never patch" in line]
                self.assertEqual(len(never), 1 if status["state"] == B.STATE_IDLE else 0, lines)


# A worker on real GameTools: argv gives the repo root, the game exe, the
# documents folder and the scratch root. It takes its backup, which writes
# the marker, prints the backup folder, then stays alive until its stdin ends,
# so the test can end it from outside.
KILLED_WORKER_CHILD = """
import sys, threading
sys.path.insert(0, sys.argv[1])
from tools.ch_mcp import bot_run as B, game_tools as GT
tools = GT.GameTools(B.Env(exe=sys.argv[2], documents_dir=sys.argv[3], run_reg=None),
                     threading.RLock(), sys.argv[4])
tools.ensure_backup()
print(tools._adopted_backup, flush=True)
sys.stdin.read()
"""


def real_game_paths(tmp: str) -> tuple[str, str]:
    """A game exe under `tmp` that is not there, and a documents folder under
    `tmp`, made."""
    documents = os.path.join(tmp, "real documents")
    os.makedirs(documents, exist_ok=True)
    return os.path.join(tmp, "no game.exe"), documents


def real_game_tools(tmp: str, scratch_root: str) -> GT.GameTools:
    """A GameTools on a real Env on real_game_paths(tmp), with `scratch_root`
    as its temp root. It has no registry runner, so its backups are the code's
    own temp-folder form, and its real recorder and real pid test. Nothing it
    does can start the game."""
    exe, documents = real_game_paths(tmp)
    return GT.GameTools(B.Env(exe=exe, documents_dir=documents, run_reg=None),
                        threading.RLock(), scratch_root)


class MarkerTest(unittest.TestCase):
    """The open-run marker: a killed worker's way to put its backup back."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name
        self.t = ToolRig(self.tmp)
        self.path = os.path.join(self.t.scratch_root, GT.OPEN_RUN_MARKER)
        # A cleanup, not tearDown, so a live worker's cleanup, added later,
        # runs first and frees the folder.
        self.addCleanup(self._tmp.cleanup)

    def assert_marker_names(self, folder):
        self.assertEqual(GT.read_marker(self.path), (os.getpid(), folder))

    # -- writing and deleting -----------------------------------------------------

    def test_the_marker_sits_under_the_scratch_root_and_never_the_real_temp_folder(self):
        self.assertEqual(self.t.tools._marker_path(), self.path)

    def real_tools(self) -> GT.GameTools:
        """real_game_tools on this test's marker folder."""
        return real_game_tools(self.tmp, self.t.scratch_root)

    def real_paths(self) -> tuple[str, str]:
        return real_game_paths(self.tmp)

    def live_worker(self) -> tuple[subprocess.Popen, str]:
        """A worker that took its backup and wrote its marker on real tools
        and still runs, as a second session's worker does. Returns its
        process and its backup folder, which the marker names; the test's
        cleanup ends it."""
        exe, documents = self.real_paths()
        child = subprocess.Popen([sys.executable, "-I", "-c", KILLED_WORKER_CHILD, _REPO_ROOT,
                                  exe, documents, self.t.scratch_root],
                                 stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
        self.addCleanup(self.end_worker, child)
        folder = child.stdout.readline().strip()
        self.assertTrue(os.path.isdir(folder), folder)
        self.assertEqual(GT.read_marker(self.path), (child.pid, folder))
        self.assertTrue(GT.pid_running(child.pid))
        return child, folder

    def end_worker(self, child: subprocess.Popen) -> None:
        """End a live worker from outside, as a reload ends one."""
        if child.poll() is None:
            child.terminate()
        child.wait(CHILD_CAP_S)
        child.stdout.close()
        child.stdin.close()

    def killed_worker(self) -> tuple[int, str]:
        """A live worker, then ended from outside. Returns its pid, no longer
        running, and its backup folder, which the marker names."""
        child, folder = self.live_worker()
        self.end_worker(child)
        self.assertFalse(GT.pid_running(child.pid))
        return child.pid, folder

    def put_game_files(self) -> dict:
        """The game's files in the documents folder, holding what the real
        game wrote: scorestats.json after a run (pitch-dark-run1) and the
        settings.ini of ls-g1-run1's backup. Returns each name's bytes."""
        _exe, documents = self.real_paths()
        written = {G.SCORESTATS: _read(STATS_AFTER_A_RUN),
                   G.SETTINGS: _read(os.path.join(REAL_BACKUP, G.SETTINGS))}
        for name, data in written.items():
            _write(os.path.join(documents, name), data)
        return written

    def assert_game_files_kept(self, written: dict) -> None:
        _exe, documents = self.real_paths()
        for name, data in written.items():
            self.assertEqual(_read(os.path.join(documents, name)), data, name)

    def test_ensure_backup_writes_the_marker_and_shutdown_deletes_it(self):
        tools = self.real_tools()
        self.assertFalse(os.path.exists(self.path))
        tools.ensure_backup()
        backup = tools._adopted_backup
        self.assertTrue(os.path.isdir(backup), backup)
        self.assert_marker_names(backup)
        tools.shutdown()
        self.assertFalse(os.path.exists(self.path))

    def test_a_failed_restore_keeps_the_marker_and_the_next_good_one_deletes_it(self):
        tools = self.real_tools()
        tools.ensure_backup()
        backup = tools._adopted_backup
        # A real failure: the backup folder is moved away, so the restore
        # cannot read its manifest.
        moved = backup + " moved"
        os.rename(backup, moved)
        tools.ch_stop()
        self.assert_marker_names(backup)
        os.rename(moved, backup)
        tools.ch_stop()
        self.assertFalse(os.path.exists(self.path))

    def test_a_marker_that_cannot_be_deleted_is_named_with_what_it_would_do(self):
        tools = self.real_tools()
        tools.ensure_backup()
        backup = tools._adopted_backup
        # An open handle shares no delete, so Windows refuses the delete for
        # real, as it does while another process has the marker open.
        with open(self.path, "rb"):
            with self.assertRaises(PermissionError) as refused:
                os.remove(self.path)
            lines = tools.ch_stop().splitlines()
        said = [line for line in lines if self.path in line]
        self.assertEqual(len(said), 1, lines)
        self.assertIn(f"({refused.exception})", said[0])
        self.assertIn(backup, said[0])
        self.assert_marker_names(backup)
        self.assertEqual(tools.open_lines(), [])

    def test_a_marker_that_cannot_be_written_is_said_and_the_run_goes_on(self):
        # A folder stands where the marker goes, so the marker's write fails
        # for real; the worker starts all the same.
        os.makedirs(self.path)
        probe = os.path.join(self.tmp, "probe")
        _write(probe, b"")
        with self.assertRaises(OSError) as refused:
            os.rename(probe, self.path)
        tools = self.real_tools()
        lines = tools.ensure_backup().splitlines()
        backup = tools._adopted_backup
        self.assertTrue(os.path.isdir(backup), backup)
        said = [line for line in lines if type(refused.exception).__name__ in line]
        self.assertEqual(len(said), 1, lines)
        self.assertIn(B._restore_hint(backup), said[0])
        self.assertTrue(os.path.isdir(self.path))
        self.assertEqual([n for n in os.listdir(self.t.scratch_root) if n.endswith(".tmp")], [])
        # The run goes on: its backup is held, and ch_stop restores it.
        self.assertEqual(tools.open_lines(), [f"adopted backup: {backup}"])
        tools.ch_stop()
        self.assertEqual(tools.open_lines(), [])

    # -- a ch_start that fails ----------------------------------------------------

    def test_a_ch_start_that_fails_after_its_backup_leaves_no_marker(self):
        tools = self.real_tools()
        exe, _documents = self.real_paths()
        # The game exe is not there, so the real launch fails after the backup.
        with self.assertRaises(OSError) as refused:
            B._real_launch([exe])
        lines = tools.ch_start().splitlines()
        self.assertTrue(lines[-1].startswith("could not start the game: "), lines)
        self.assertTrue(lines[-1].endswith(str(refused.exception)), lines)
        self.assertFalse(os.path.exists(self.path))
        self.assertIsNone(tools._run)
        self.assertIsNone(tools._adopted_backup)
        self.assertEqual(tools.open_lines(), [])

    def test_a_song_with_no_length_never_takes_a_backup_or_a_marker(self):
        with mock.patch.object(B, "song_length_ms", side_effect=B.SongLengthMissing("no length")):
            self.assertEqual(self.t.tools.ch_start(self.t.r.song).splitlines()[-1], "no length")
        self.assertFalse(os.path.exists(self.path))

    # -- the next worker adopts ---------------------------------------------------

    def age_folder(self, folder, hours):
        then = time.time() - hours * 3600
        os.utime(folder, (then, then))

    def assert_files_left(self):
        """Every game file, settings.ini among them, still holds what the
        game wrote."""
        for name in ORIGINAL:
            self.assertEqual(_read(self.t.r.path(name)), b"changed by the game", name)

    def test_an_older_marker_backup_that_names_the_games_files_leaves_them(self):
        written = self.put_game_files()
        pid, backup = self.killed_worker()
        # The killed worker's backup becomes REAL_BACKUP, a real backup from
        # before the rulings of 2026-11-13: its manifest names the game's files
        # and the registry key. Each source is moved into this test's
        # documents folder, so a restore that broke could only write there.
        _exe, documents = self.real_paths()
        with open(os.path.join(REAL_BACKUP, G.BACKUP_MANIFEST), encoding="utf-8") as f:
            entries = json.load(f)["files"]
        for e in entries:
            if "source" in e:
                e["source"] = os.path.join(documents, e["name"])
        for name in os.listdir(REAL_BACKUP):
            _write(os.path.join(backup, name), _read(os.path.join(REAL_BACKUP, name)))
        with open(os.path.join(backup, G.BACKUP_MANIFEST), "w", encoding="utf-8") as f:
            json.dump({"files": entries}, f)
        tools = self.real_tools()
        self.assertEqual(tools._adopted_backup, backup)
        lines = tools.ch_stop().splitlines()
        left = G.left_as_saved(backup, out=lambda _line: None)
        self.assertEqual(len(left), sum("source" in e for e in entries))
        for line in left:
            self.assertIn(line, lines)
        self.assert_game_files_kept(written)
        # The key is in the manifest and these tools have no reg.exe runner,
        # so the key's restore fails for real and the backup stays adopted.
        self.assertTrue(any(line.startswith(f"restore failed on {G.REGISTRY_BACKUP}: ")
                            for line in lines), lines)
        self.assertEqual(GT.read_marker(self.path), (pid, backup))
        self.assertEqual(tools._adopted_backup, backup)

    def test_a_restore_from_a_marker_keeps_a_score_file_changed_after_the_backup(self):
        pid, backup = self.killed_worker()
        # The game saved its files after the killed worker's backup.
        written = self.put_game_files()
        tools = self.real_tools()
        self.assertEqual(tools._adopted_backup, backup)
        tools.ch_stop()
        self.assertIsNone(tools._adopted_backup)
        self.assert_game_files_kept(written)

    def test_an_ordinary_adopted_backup_is_restored_at_shutdown(self):
        self.t.tools.ensure_backup()
        for name in ORIGINAL:
            _write(self.t.r.path(name), b"changed by the game")
        self.t.tools.shutdown()
        self.assert_files_left()
        self.assertFalse(os.path.exists(self.path))

    def test_the_workers_exit_leaves_a_marker_backup_and_its_marker(self):
        pid, backup = self.killed_worker()
        written = self.put_game_files()
        tools = self.real_tools()
        reply = tools.shutdown()
        self.assertIn(backup, reply)
        self.assertIn(self.path, reply)
        self.assert_game_files_kept(written)
        self.assertEqual(GT.read_marker(self.path), (pid, backup))
        self.assertEqual(tools.open_lines(), [f"adopted backup: {backup}"])
        tools.ch_stop()
        self.assert_game_files_kept(written)
        self.assertFalse(os.path.exists(self.path))

    def test_the_next_worker_adopts_and_ch_stop_restores_and_deletes_the_marker(self):
        pid, backup = self.killed_worker()
        hours = 3
        self.age_folder(backup, hours)
        written = self.put_game_files()
        printed = io.StringIO()
        with contextlib.redirect_stdout(printed):
            tools = self.real_tools()
        said = [line for line in tools.ch_status().splitlines()
                if backup in line and f"(pid {pid})" in line]
        self.assertEqual(len(said), 1)
        self.assertIn(GT.age_text(hours * 3600), said[0])
        self.assertEqual(printed.getvalue().splitlines(), said)
        self.assertEqual(tools.open_lines(), [f"adopted backup: {backup}"])
        with mock.patch.object(B, "restore_only", wraps=B.restore_only) as restore_only:
            lines = tools.ch_stop().splitlines()
        self.assertEqual(restore_only.call_args.args[0], backup)
        self.assertIn("if the game is still running it " + B.late_write_hint(backup), lines)
        self.assert_game_files_kept(written)
        self.assertFalse(os.path.exists(self.path))
        self.assertNotIn(said[0], tools.ch_status().splitlines())
        self.assertEqual(tools.open_lines(), [])

    def test_adopting_restores_nothing_until_ch_stop(self):
        _pid, backup = self.killed_worker()
        written = self.put_game_files()
        tools = self.real_tools()
        tools.ch_status()
        self.assert_game_files_kept(written)
        self.assertEqual(tools._adopted_backup, backup)
        self.assertTrue(os.path.isfile(os.path.join(backup, G.BACKUP_MANIFEST)))

    def test_a_ch_start_on_a_marker_backup_refuses_naming_the_folder_and_its_age(self):
        pid, backup = self.killed_worker()
        hours = 5
        self.age_folder(backup, hours)
        written = self.put_game_files()
        tools = self.real_tools()
        reply = tools.ch_start()
        self.assertTrue(reply.startswith("refused: "), reply)
        self.assertIn(backup, reply)
        self.assertIn(GT.age_text(hours * 3600), reply)
        self.assertIsNone(tools._run)
        self.assertEqual(os.listdir(os.path.dirname(backup)), [os.path.basename(backup)])
        self.assert_game_files_kept(written)
        self.assertEqual(GT.read_marker(self.path), (pid, backup))
        tools.ch_stop()
        # Once restored, ch_start goes on to the launch, which fails for real
        # here because the game exe is not there.
        self.assertFalse(tools.ch_start().startswith("refused: "))

    def test_an_adopted_restore_that_fails_keeps_the_marker_and_the_adoption(self):
        pid, backup = self.killed_worker()
        written = self.put_game_files()
        tools = self.real_tools()
        # A real failure: the backup folder is moved away, so the restore
        # cannot read its manifest.
        moved = backup + " moved"
        os.rename(backup, moved)
        lines = tools.ch_stop().splitlines()
        self.assertTrue(any(line.startswith("restoring failed: ") and backup in line
                            for line in lines), lines)
        self.assertEqual(GT.read_marker(self.path), (pid, backup))
        self.assertEqual(tools._adopted_backup, backup)
        self.assertTrue(tools.ch_start().startswith("refused: "))
        os.rename(moved, backup)
        tools.ch_stop()
        self.assert_game_files_kept(written)
        self.assertFalse(os.path.exists(self.path))

    def test_a_backup_that_vanishes_after_adoption_fails_the_stop_without_raising(self):
        pid, backup = self.killed_worker()
        tools = self.real_tools()
        manifest = os.path.join(backup, G.BACKUP_MANIFEST)
        os.remove(manifest)
        with self.assertRaises(FileNotFoundError) as gone:
            G.restore_registry(backup, run_reg=None)
        said = [line for line in tools.ch_stop().splitlines()
                if line.startswith("restoring failed: ")]
        self.assertEqual(len(said), 1)
        self.assertIn(f"{type(gone.exception).__name__}: {gone.exception}", said[0])
        self.assertIn(backup, said[0])
        self.assertEqual(GT.read_marker(self.path), (pid, backup))

    def marker_lines(self, tools: GT.GameTools, pid: int, folder: str) -> list:
        """ch_status's lines that name both a previous worker's pid and its
        backup folder."""
        return [line for line in tools.ch_status().splitlines()
                if folder in line and re.search(rf"\b{pid}\b", line)]

    def test_a_folder_with_no_manifest_is_not_adopted_and_the_marker_stays(self):
        pid, backup = self.killed_worker()
        written = self.put_game_files()
        os.remove(os.path.join(backup, G.BACKUP_MANIFEST))
        tools = self.real_tools()
        self.assertIsNone(tools._adopted_backup)
        said = self.marker_lines(tools, pid, backup)
        self.assertEqual(len(said), 1)
        self.assertIn(G.BACKUP_MANIFEST, said[0])
        self.assertIn(self.path, said[0])
        self.assertEqual(tools.open_lines(), [])
        tools.ch_stop()
        self.assert_game_files_kept(written)
        self.assertEqual(GT.read_marker(self.path), (pid, backup))

    # -- a marker whose pid still runs --------------------------------------------

    def test_a_running_pid_is_not_adopted_and_the_line_gives_the_by_hand_restore(self):
        child, backup = self.live_worker()
        tools = self.real_tools()
        self.assertIsNone(tools._adopted_backup)
        said = self.marker_lines(tools, child.pid, backup)
        self.assertEqual(len(said), 1)
        self.assertIn(B._restore_hint(backup), said[0])
        self.assertIn(self.path, said[0])
        self.assertEqual(tools.open_lines(), [])
        # The running pid is what held the backup back: once that worker has
        # ended, the next worker adopts the same marker.
        self.end_worker(child)
        self.assertEqual(self.real_tools()._adopted_backup, backup)

    def test_a_worker_that_adopted_nothing_never_deletes_anothers_marker(self):
        child, backup = self.live_worker()
        written = self.put_game_files()
        tools = self.real_tools()
        for stop in (tools.ch_stop, tools.shutdown):
            with self.subTest(stop=stop.__name__):
                stop()
                self.assertEqual(GT.read_marker(self.path), (child.pid, backup))
                self.assert_game_files_kept(written)

    def test_a_second_workers_backup_leaves_the_first_workers_marker_alone(self):
        child, backup = self.live_worker()
        tools = self.real_tools()
        lines = tools.ensure_backup().splitlines()
        mine = tools._adopted_backup
        self.assertNotEqual(mine, backup)
        # The marker is written by renaming a temp file onto its path, and
        # Windows refuses a rename onto a file that is there.
        other = os.path.join(self.tmp, "other marker")
        _write(other, b"")
        with self.assertRaises(OSError) as refused:
            os.rename(other, self.path)
        said = [line for line in lines if B._restore_hint(mine) in line]
        self.assertEqual(len(said), 1, lines)
        self.assertIn(f"({type(refused.exception).__name__}: ", said[0])
        self.assertEqual(GT.read_marker(self.path), (child.pid, backup))
        tools.ch_stop()
        self.assertEqual(GT.read_marker(self.path), (child.pid, backup))
        self.assertEqual([n for n in os.listdir(self.t.scratch_root) if n.endswith(".tmp")], [])

    # -- a marker whose folder is gone --------------------------------------------

    def test_a_gone_folder_deletes_the_marker_and_ch_stop_has_nothing_to_restore(self):
        for running in (False, True):
            with self.subTest(running=running):
                if running:
                    child, backup = self.live_worker()
                    pid = child.pid
                else:
                    pid, backup = self.killed_worker()
                shutil.rmtree(backup)
                tools = self.real_tools()
                self.assertFalse(os.path.exists(self.path))
                self.assertIsNone(tools._adopted_backup)
                self.assertEqual(len(self.marker_lines(tools, pid, backup)), 1)
                self.assertEqual(tools.open_lines(), [])
                tools.ch_stop()
                self.assertFalse(os.path.exists(self.path))

    # -- a marker that cannot be read ---------------------------------------------

    def assert_unread_marker_said(self, tools: GT.GameTools, said: list) -> None:
        """`said` is the one line naming the marker, ending with what
        read_marker raises on it, and ch_status repeats it."""
        with self.assertRaises(Exception) as unread:
            GT.read_marker(self.path)
        self.assertEqual(len(said), 1, said)
        self.assertIn(self.path, said[0])
        self.assertTrue(said[0].endswith(f"{type(unread.exception).__name__}: "
                                         f"{unread.exception}"), said[0])
        self.assertIn(said[0], tools.ch_status().splitlines())

    def test_an_unreadable_marker_starts_the_worker_and_stays_byte_for_byte(self):
        # Each case is the killed worker's real marker, damaged one way.
        pid, _backup = self.killed_worker()
        real = _read(self.path)
        marker = json.loads(real)

        def dump(value) -> bytes:
            return json.dumps(value).encode()

        cases = {"cut short": real[:len(real) // 2], "empty": b"", "not utf-8": b"\xff" + real,
                 "a list": dump(list(marker.values())),
                 "no folder": dump({"pid": marker["pid"]}),
                 "no pid": dump({"folder": marker["folder"]}),
                 "a true pid": dump({**marker, "pid": True}),
                 "a text pid": dump({**marker, "pid": str(pid)}),
                 "a zero pid": dump({**marker, "pid": 0}),
                 "an empty folder": dump({**marker, "folder": ""}),
                 "a numeric folder": dump({**marker, "folder": pid}),
                 "deep nesting": b"[" * 200000}
        for name, data in cases.items():
            with self.subTest(case=name):
                _write(self.path, data)
                printed = io.StringIO()
                with contextlib.redirect_stdout(printed):
                    tools = self.real_tools()
                self.assertIsNone(tools._adopted_backup)
                self.assert_unread_marker_said(tools, printed.getvalue().splitlines())
                tools.ch_stop()
                self.assertEqual(_read(self.path), data)

    def test_a_folder_at_the_markers_path_starts_the_worker_and_stays(self):
        # A folder where the marker file goes: opening it to read fails.
        os.makedirs(self.path)
        printed = io.StringIO()
        with contextlib.redirect_stdout(printed):
            tools = self.real_tools()
        self.assertIsNone(tools._adopted_backup)
        self.assert_unread_marker_said(tools, printed.getvalue().splitlines())
        tools.ch_stop()
        self.assertTrue(os.path.isdir(self.path))

    def test_an_unreadable_marker_is_never_replaced_by_a_new_run(self):
        # The killed worker's real marker, cut short.
        self.killed_worker()
        data = _read(self.path)
        data = data[:len(data) // 2]
        _write(self.path, data)
        with contextlib.redirect_stdout(io.StringIO()):
            tools = self.real_tools()
        lines = tools.ensure_backup().splitlines()
        said = [line for line in lines if B._restore_hint(tools._adopted_backup) in line]
        self.assertEqual(len(said), 1, lines)
        tools.ch_stop()
        self.assertEqual(_read(self.path), data)


class RecorderTest(unittest.TestCase):
    """How ch_start and the stops drive the machine recorder.
    test_recorder_link.py covers the link itself."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)
        self.t = ToolRig(self._tmp.name)
        self.popen = self.t.popen

    def folder(self, n: int = 1) -> str:
        return os.path.join(self.t.tools._scratch, f"machine-{n}")

    # -- ch_start -------------------------------------------------------------------

    # -- the real recorder program ----------------------------------------------------

    def real_tools(self) -> GT.GameTools:
        return real_game_tools(self._tmp.name, os.path.join(self._tmp.name, "real scratch"))

    def start_real_recorder(self, tools: GT.GameTools) -> list:
        """Start the real machine recorder through ch_start's own step, on the
        session folder, and return its lines; the test's cleanup stops it."""
        lines: list = []
        tools._start_recorder(tools.session_dir(), GT.cap_for_end(None), lines)
        self.addCleanup(tools._stop_recorder, [])
        return lines

    def test_a_machine_that_is_not_quiet_starts_no_recorder(self):
        # The process list is a real one, read by a recorder while a build was
        # linking: sample 615 of ls-g3-run1's machine-1 processes.csv, from
        # hydra-data/ch-mcp-runs/2026-11-12/ls-g3-run1. The machine's other
        # readings are this machine's own.
        processes = build_processes()
        compilers = Q._compiler_lines(processes)
        self.assertEqual(len(compilers), 1)
        tools = self.real_tools()
        with mock.patch.object(Q, "LIVE_ENV", real_quiet_env(processes)):
            reply = tools.ch_start()
        self.assertIn(compilers[0], reply)
        self.assertFalse(tools._recorder.running())
        self.assertFalse(os.path.exists(os.path.join(self._tmp.name, "real scratch")))
        self.assertEqual(tools.open_lines(), [])

    def test_the_reply_names_the_recorder_and_the_worker_and_its_parent(self):
        tools = self.real_tools()
        lines = self.start_real_recorder(tools)
        proc = tools._recorder._proc
        self.assertTrue(GT.pid_running(proc.pid))
        folder = proc.args[proc.args.index("--out") + 1]
        self.assertTrue(os.path.isfile(os.path.join(folder, R.LOG_NAME)), folder)
        said = [line for line in lines if re.search(rf"\b{proc.pid}\b", line) and folder in line]
        self.assertEqual(len(said), 1, lines)
        worker = proc.args[proc.args.index("--parent") + 1]
        said = [line for line in lines if re.search(rf"\b{worker}\b", line)
                and re.search(rf"\b{os.getppid()}\b", line)]
        self.assertEqual(len(said), 1, lines)
        self.assertEqual(tools.open_lines(), [tools._recorder.line()])

    def test_the_command_carries_this_worker_and_the_songs_run_cap(self):
        self.t.tools.ch_start(self.t.r.song)
        command = self.popen.calls[0][0]
        self.assertEqual(command[:2], [sys.executable, "-I"])
        self.assertEqual(command[command.index("--parent") + 1], str(os.getpid()))
        self.assertEqual(command[command.index("--run-cap-s") + 1],
                         "64.0")
        self.assertEqual(command[command.index("--out") + 1], self.folder())
        self.assertNotIn("--rate", command)
        self.assertNotIn("--cap-s", command)

    def test_a_run_with_no_song_gets_the_cap_with_no_folder(self):
        self.t.tools.ch_start()
        command = self.popen.calls[0][0]
        self.assertEqual(command[command.index("--run-cap-s") + 1], str(play_chart.RUN_CAP_S))

    def test_a_recorder_that_fails_to_start_is_one_line_and_ch_status_keeps_it(self):
        tools = self.real_tools()
        # A file where the session folder goes, so the real recorder's folder
        # cannot be made.
        session = os.path.join(self._tmp.name, "a file")
        _write(session, b"")
        with self.assertRaises(OSError) as refused:
            os.makedirs(os.path.join(session, f"{R.FOLDER_PREFIX}1"))
        lines: list = []
        tools._start_recorder(session, GT.cap_for_end(None), lines)
        said = [line for line in lines
                if f"({type(refused.exception).__name__}: {refused.exception})" in line]
        self.assertEqual(len(said), 1, lines)
        self.assertFalse(tools._recorder.running())
        self.assertEqual(tools.open_lines(), [])
        self.assertIn(said[0], tools.ch_status().splitlines())

    def test_a_pid_line_that_cannot_be_sent_is_one_line_and_raises_nothing(self):
        tools = self.real_tools()
        self.start_real_recorder(tools)
        proc = tools._recorder._proc
        # The real recorder ends, so the pipe to it is closed for real.
        proc.terminate()
        proc.wait(CHILD_CAP_S)
        sent = f"pid {os.getpid()}"
        lines: list = []
        tools._tell_recorder(sent, lines)
        with self.assertRaises(OSError) as closed:
            proc.stdin.write(f"{sent}\n".encode("ascii"))
            proc.stdin.flush()
        self.assertEqual(len(lines), 1, lines)
        self.assertIn(f'"{sent}"', lines[0])
        self.assertIn(f"({type(closed.exception).__name__}: {closed.exception})", lines[0])

    def real_tools_keeping_procs(self) -> tuple:
        """real_tools whose recorder is the real program, started by the real
        Popen; the processes it started are kept in the list it returns, so a
        test can read one after the tools have let it go."""
        procs: list = []

        def popen(*args, **kwargs):
            procs.append(subprocess.Popen(*args, **kwargs))
            return procs[-1]

        self.addCleanup(lambda: [p.kill() for p in procs if p.poll() is None])
        exe, documents = real_game_paths(self._tmp.name)
        tools = GT.GameTools(B.Env(exe=exe, documents_dir=documents, run_reg=None),
                             threading.RLock(), os.path.join(self._tmp.name, "real scratch"),
                             make_recorder=lambda: R.RecorderLink(popen=popen))
        return tools, procs

    def recorder_lines(self, lines: list, proc: subprocess.Popen) -> list:
        """The lines of `lines` that name `proc`'s pid as a whole word and its folder."""
        folder = proc.args[proc.args.index("--out") + 1]
        return [line for line in lines if re.search(rf"\b{proc.pid}\b", line) and folder in line]

    def test_a_launch_that_fails_stops_the_recorder_before_ch_start_returns(self):
        tools, procs = self.real_tools_keeping_procs()
        # The launch fails for real: the game exe is not there.
        reply = tools.ch_start()
        self.assertIsNone(tools._run)
        self.assertEqual(len(procs), 1)
        proc = procs[0]
        # It ended through its stdin closing, not a terminate, before the reply.
        self.assertEqual(proc.poll(), 0)
        said = [line for line in self.recorder_lines(reply.splitlines(), proc)
                if f"exit code {proc.returncode}" in line]
        self.assertEqual(len(said), 1, reply)
        self.assertEqual(tools.open_lines(), [])

    def test_a_second_start_ends_the_first_recorder(self):
        tools = self.real_tools()
        self.start_real_recorder(tools)
        first = tools._recorder._proc
        lines = self.start_real_recorder(tools)
        second = tools._recorder._proc
        self.assertIsNot(first, second)
        self.assertEqual(first.poll(), 0)
        self.assertIsNone(second.poll())
        said = [line for line in self.recorder_lines(lines, first)
                if f"exit code {first.returncode}" in line]
        self.assertEqual(len(said), 1, lines)

    def test_the_samples_line_counts_a_real_recordings_rows_and_names_its_last_time(self):
        tools = self.real_tools()
        self.start_real_recorder(tools)
        proc = tools._recorder._proc
        folder = proc.args[proc.args.index("--out") + 1]
        deadline = time.monotonic() + CHILD_CAP_S
        while len(complete_rows(folder, MR.SAMPLES_FILE)) < 2:
            if proc.poll() is not None:
                self.fail(f"the recorder ended before two samples, exit code {proc.returncode}")
            if time.monotonic() > deadline:
                self.fail(f"no two samples within {CHILD_CAP_S} s")
            time.sleep(1 / MR.RATE_PER_S)
        # Ended, so its file no longer grows while the line and the rows are read.
        proc.terminate()
        proc.wait(CHILD_CAP_S)
        rows = complete_rows(folder, MR.SAMPLES_FILE)
        line = tools._recorder.samples_line()
        self.assertRegex(line, rf"\b{len(rows)} samples\b")
        self.assertTrue(line.endswith(rows[-1][MR.WALL_COLUMN]), line)

    def test_a_recorder_that_never_started_says_so_in_the_samples_line_and_is_told_nothing(self):
        tools = self.real_tools()
        # A file where the session folder goes, so the real recorder's folder
        # cannot be made.
        session = os.path.join(self._tmp.name, "a file")
        _write(session, b"")
        lines: list = []
        tools._start_recorder(session, GT.cap_for_end(None), lines)
        self.assertIn(tools._recorder.samples_line(), lines)
        told: list = []
        tools._tell_recorder(f"pid {os.getpid()}", told)
        self.assertEqual(told, [])

    # -- the stops ------------------------------------------------------------------

    def test_ch_stop_and_shutdown_end_the_recorder_by_closing_its_stdin(self):
        tools = self.real_tools()
        for stop in ("ch_stop", "shutdown"):
            with self.subTest(stop=stop):
                self.start_real_recorder(tools)
                proc = tools._recorder._proc
                folder = proc.args[proc.args.index("--out") + 1]
                lines = getattr(tools, stop)().splitlines()
                self.assertEqual(proc.poll(), 0)
                # The recorder's own last row names why it ended.
                rows = complete_rows(folder, MR.SAMPLES_FILE)
                self.assertEqual(rows[-1]["stop_reason"], MR.STOP_STDIN)
                self.assertEqual(self.recorder_lines(lines[:1], proc), lines[:1], lines)
                self.assertIn(f"exit code {proc.returncode}", lines[0])

    def test_the_stop_ends_the_recorder_before_the_hooks_run(self):
        tools = self.real_tools()
        self.start_real_recorder(tools)
        proc = tools._recorder._proc
        seen: list = []
        tools.add_stop_hook(lambda: seen.append(proc.poll()))
        tools.ch_stop()
        self.assertEqual(seen, [0])

    def test_open_lines_list_a_running_recorder_and_not_a_stopped_one(self):
        tools = self.real_tools()
        self.start_real_recorder(tools)
        proc = tools._recorder._proc
        opened = tools.open_lines()
        self.assertEqual(opened, [tools._recorder.line()])
        self.assertRegex(opened[0], rf"\b{proc.pid}\b")
        tools.ch_stop()
        self.assertEqual(tools.open_lines(), [])

    def test_ch_status_prints_the_recorders_line(self):
        tools = self.real_tools()
        self.assertIn(tools._recorder.line(), tools.ch_status().splitlines())
        self.start_real_recorder(tools)
        proc = tools._recorder._proc
        status = tools.ch_status().splitlines()
        self.assertIn(tools._recorder.line(), status)
        self.assertEqual(len([line for line in status if re.search(rf"\b{proc.pid}\b", line)]), 1,
                         status)

    def test_one_that_exited_by_itself_leaves_open_lines_and_is_said_once_more(self):
        tools = self.real_tools()
        self.start_real_recorder(tools)
        proc = tools._recorder._proc
        folder = proc.args[proc.args.index("--out") + 1]
        # It ends with no word from the tools, as a recorder that dies does.
        proc.terminate()
        proc.wait(CHILD_CAP_S)
        self.assertEqual(tools.open_lines(), [])

        def exited(lines: list) -> list:
            return [line for line in self.recorder_lines(lines, proc)
                    if f"exit code {proc.returncode}" in line
                    and os.path.join(folder, R.LOG_NAME) in line]

        status = tools.ch_status().splitlines()
        self.assertEqual(len(exited(status)), 1, status)
        stopped = tools.ch_stop().splitlines()
        self.assertEqual(exited(stopped), exited(status), stopped)
        self.assertIsNone(tools._recorder.stop())

    def test_a_stop_with_no_recorder_adds_no_line(self):
        tools = self.real_tools()
        reply = tools.ch_stop()
        self.assertEqual([line for line in reply.splitlines() if line.startswith(R._LINE_PREFIX)],
                         [], reply)
        self.assertIsNone(tools._recorder.stop())


class PidRunningTest(unittest.TestCase):
    def test_this_process_runs(self):
        self.assertTrue(GT.pid_running(os.getpid()))

    def test_a_process_that_exited_does_not(self):
        child = subprocess.Popen([sys.executable, "-c", ""])
        child.wait(CHILD_CAP_S)
        self.assertFalse(GT.pid_running(child.pid))

    def test_a_pid_no_process_ever_held_does_not(self):
        self.assertFalse(GT.pid_running(0x7FFFFFFC))


# A child that registers an atexit handler writing `sys.argv[1]`, says so on
# stdout, then stays alive until its stdin ends, so the test holds it open.
ATEXIT_CHILD = """
import atexit, sys
atexit.register(lambda: open(sys.argv[1], "w").close())
print("ready", flush=True)
sys.stdin.read()
"""


class AtexitFactTest(unittest.TestCase):
    """The reload plan's row P5, the fact the marker exists for: a process
    ended from outside runs no atexit handler, and one that exits normally
    does."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)
        self.flag = os.path.join(self._tmp.name, "atexit-ran")
        self.child = subprocess.Popen([sys.executable, "-c", ATEXIT_CHILD, self.flag],
                                      stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
        self.addCleanup(self.child.stdout.close)
        self.addCleanup(self.child.stdin.close)
        self.assertEqual(self.child.stdout.readline().strip(), "ready")

    def test_a_terminated_process_runs_no_atexit_handler(self):
        self.child.terminate()
        self.child.wait(CHILD_CAP_S)
        self.assertFalse(os.path.exists(self.flag))

    def test_a_normal_exit_runs_the_atexit_handler(self):
        self.child.stdin.close()
        self.child.wait(CHILD_CAP_S)
        self.assertTrue(os.path.exists(self.flag))


# Hydra's schedule of the corpus chart testdata/input/test_pitch_dark_ratio
# (the command is in the comment above _REPLAY_DIR in
# tools/ch_probe/tests/test_sp_compare.py), and the folder under
# testdata/ch_runs of an in-game run of that chart's path 0 (copied from
# hydra-data/ch-mcp-runs/2026-11-13/pitch-dark-ratios-run1). An older build
# made the run's own schedule; its reads and declined chords are this one's.
PITCH_DARK_SCHEDULE = os.path.join(_REPO_ROOT, "testdata", "replay",
                                   "test_pitch_dark_ratio-schedule.json")
PITCH_DARK_RUN = ("pitch-dark-ratios-run1",)
# Hydra's score JSON of the same chart, path 0, beside its schedule.
PITCH_DARK_SCORE = os.path.join(_REPO_ROOT, "testdata", "replay",
                                "test_pitch_dark_ratio-score.json")
# A hydra_replay dump of the Bomber refine chart under tools/ch_probe/tests,
# written by a hydra_replay older than the song end: it has no song_length_ms.
NO_SONG_END_DUMP = os.path.join(_PROBE_TESTS, "bomber_refine_dump.json")


def _json_field(path, field):
    """The top-level `field` of the JSON file at `path`."""
    with open(path, encoding="utf-8") as f:
        return json.load(f)[field]


def _press_rows(*parts):
    """A press log under testdata/ch_runs as a play session holds its rows:
    the csv's text, with `index` the int play_chords writes."""
    with open(os.path.join(_REPO_ROOT, "testdata", "ch_runs", *parts),
              encoding="utf-8", newline="") as f:
        return [{**row, "index": int(row["index"])} for row in csv.DictReader(f)]


class PlayerLimitsLinesTest(unittest.TestCase):
    """The reply lines and the edge probe of the 2026-11-07 player-limits
    plan, task T4: design points 1, 8 and 9."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)
        self.tmp = self._tmp.name

    def _json(self, name, value):
        path = os.path.join(self.tmp, name)
        _write(path, json.dumps(value).encode())
        return path

    # -- the anchor line -----------------------------------------------------------------

    def _schedule(self, name):
        """A hydra_replay schedule fixture under tools/ch_probe/tests, read."""
        with open(os.path.join(_PROBE_TESTS, name), encoding="utf-8") as f:
            return json.load(f)

    @staticmethod
    def _squeeze_at(doc, chord):
        return next(q for q in GT._squeezes(doc) if q["chord"] == chord)

    def _anchor_line_of(self, doc, q):
        lines = [line for line in GT.anchor_lines(doc)
                 if f" at chord {q['chord']} " in line]
        self.assertEqual(len(lines), 1, GT.anchor_lines(doc))
        return lines[0]

    def test_the_anchor_line_reads_every_number_off_the_squeeze(self):
        # Freak's squeeze-out at chord 387 (freak_limits_schedule.json): its
        # anchor is pressed early, and its two ratios and two rooms are the
        # same, so the line gives each once.
        doc = self._schedule("freak_limits_schedule.json")
        q = self._squeeze_at(doc, 387)
        self.assertEqual((q["kind"], q["needs_no_timing"]), ("SqOut", False))
        self.assertLess(q["anchor_aim_ms"], 0)
        self.assertEqual(q["ratio"]["early"], q["ratio"]["late"])
        line = self._anchor_line_of(doc, q)
        self.assertTrue(line.startswith(
            f"chord {q['anchor_chord']} pressed {abs(q['anchor_aim_ms']):.1f} ms early for the "
            f"{GT._SQUEEZE_WORDS['SqOut'][1]} at chord 387 "), line)
        self.assertIn(f"end planned {q['planned_end_ms']:,.1f} ms", line)
        self.assertIn(f"ratio {q['ratio']['early']:.4f},", line)
        self.assertIn(f"room {q['anchor_room_ms']:.1f} ms both presses", line)

    def test_a_late_anchor_names_the_squeeze_in_and_each_ratio_and_room(self):
        # Mouths' squeeze-in at chord 431 (mouths_refine_schedule.json): its
        # anchor is pressed late, and its early and late ratios differ, as
        # do its two rooms.
        doc = self._schedule("mouths_refine_schedule.json")
        q = self._squeeze_at(doc, 431)
        self.assertEqual((q["kind"], q["needs_no_timing"]), ("SqIn", False))
        self.assertGreater(q["anchor_aim_ms"], 0)
        self.assertNotEqual(f"{q['ratio']['early']:.4f}", f"{q['ratio']['late']:.4f}")
        self.assertNotEqual(f"{q['anchor_room_ms']:.1f}", f"{q['squeeze_room_ms']:.1f}")
        line = self._anchor_line_of(doc, q)
        self.assertTrue(line.startswith(
            f"chord {q['anchor_chord']} pressed {q['anchor_aim_ms']:.1f} ms late for the "
            f"{GT._SQUEEZE_WORDS['SqIn'][1]} at chord 431 "), line)
        self.assertIn(f"ratio {q['ratio']['early']:.4f} early, {q['ratio']['late']:.4f} late",
                      line)
        self.assertIn(f"anchor room {q['anchor_room_ms']:.1f} ms, "
                      f"squeeze room {q['squeeze_room_ms']:.1f} ms", line)

    def test_the_squeeze_line_times_a_press_from_its_own_end(self):
        # Pitch Dark's squeeze-in at chord 644 is timed from the end the
        # player reads after chord 466 (aim_end_ms), which is not its own
        # planned end (own_end_ms). The line gives the gap to its own end.
        path = os.path.join(_REPO_ROOT, "testdata", "replay",
                            "test_pitch_dark_ratio-schedule.json")
        doc = GT.schedule_doc(path)
        row = next(p for p in doc["presses"] if p["chord"] == 644)
        self.assertEqual((row["rule"], row["timed_from_end"]), ("squeeze_in", 1))
        chords = play_chart.load_schedule(path).chords
        press = next(p for p in play_chart.press_plan(chords) if p.index == 644)
        own_gap = row["own_end_ms"] - press.at_ms
        read_gap = row["aim_end_ms"] - press.at_ms
        self.assertGreater(own_gap, 0)
        self.assertNotEqual(GT._ms_text(abs(own_gap)), GT._ms_text(abs(read_gap)))
        line = GT.squeeze_lines(chords, doc)[0]
        self.assertIn(f"chord 644 in ({GT._ms_text(own_gap)} ms before the end)", line)

    def test_a_squeeze_that_needs_no_timing_has_no_anchor_line(self):
        # Freak's squeeze-in at chord 717 needs no timing
        # (freak_limits_schedule.json): it gets no anchor line, while the
        # file's timed squeezes each get one.
        doc = self._schedule("freak_limits_schedule.json")
        self.assertTrue(self._squeeze_at(doc, 717)["needs_no_timing"])
        lines = GT.anchor_lines(doc)
        self.assertFalse([line for line in lines if " at chord 717 " in line], lines)
        timed = [q for q in GT._squeezes(doc) if not q["needs_no_timing"]]
        self.assertTrue(timed)
        self.assertEqual(len(lines), len(timed), lines)

    def test_the_planned_ends_are_the_schedules_on_each_chord_read_after(self):
        # Freak reads its first activation's end after chords 232 and 263
        # (each chord's `reads` in freak_limits_schedule.json).
        chords, _lines = GT.chords_to_play(os.path.join(_PROBE_TESTS,
                                                       "freak_limits_schedule.json"))
        t = ToolRig(self.tmp, live_play=lambda chords, **kw: play_chart.RunResult(stop="done"))
        play = GT._PlaySession(score_json="s.json", chord_count=len(chords), log_path=None)
        t.tools._start_play(play, chords)
        play.thread.join(THREAD_CAP_S)
        self.assertEqual(play.planned_ends[232], 57933.88726341159)
        self.assertEqual(play.planned_ends[263], 60554.52732830211)

    # -- the read end and the points between presses ---------------------------------------

    def _planned_ends(self, schedule_path):
        """The planned ends _start_play keeps for the schedule's chords."""
        chords, _lines = GT.chords_to_play(schedule_path)
        t = ToolRig(self.tmp, live_play=lambda chords, **kw: play_chart.RunResult(stop="done"))
        play = GT._PlaySession(score_json="s.json", chord_count=len(chords), log_path=None)
        t.tools._start_play(play, chords)
        play.thread.join(THREAD_CAP_S)
        self.assertFalse(play.running(), "the play thread did not end")
        return play.planned_ends

    def test_the_end_line_sets_the_games_end_against_the_planned_one(self):
        # Every row of PITCH_DARK_RUN that carries the game's end, against the
        # ends Hydra's schedule of the chart plans for the same chords.
        rows = _press_rows(*PITCH_DARK_RUN, "press-1-end-reads.csv")
        ends = self._planned_ends(PITCH_DARK_SCHEDULE)
        lines = GT.activation_end_lines(rows, ends)
        self.assertEqual(len(lines), len(rows), lines)
        for row, line in zip(rows, lines):
            planned, game = ends[row["index"]], float(row["end_read_ms"])
            gap = game - planned
            self.assertEqual(line.split(": ")[0], f"end read after chord {row['index']}")
            self.assertIn(f"end planned {planned:,.1f} ms, game {game:,.1f} ms", line)
            self.assertTrue(line.endswith(
                f"({abs(gap):.1f} {'later' if gap >= 0 else 'earlier'})"), line)

    def test_rows_without_the_new_columns_give_no_lines(self):
        # A press log written before play_chart had the end_read_ms and
        # scored_between columns.
        rows = _press_rows("ls-t2-run-a", "press-1.csv")
        self.assertTrue(rows)
        self.assertNotIn("end_read_ms", rows[0])
        self.assertNotIn("scored_between", rows[0])
        ends = {row["index"]: float(row["chart_ms"]) for row in rows}
        self.assertEqual(GT.activation_end_lines(rows, ends), [])
        self.assertEqual(GT.scored_between_lines(rows), [])

    def test_points_between_presses_go_to_the_declined_chord_before_them(self):
        # PITCH_DARK_RUN declined chord 62, and the game's score rose between
        # the presses of chords 62 and 63 (scored_between on 63's row).
        rows = _press_rows(*PITCH_DARK_RUN, "press-1-chords-61-64.csv")
        declined = [r["index"] for r in rows if r["send"] == play_chart.DECLINED]
        between = [r for r in rows if float(r["scored_between"] or 0)]
        self.assertEqual(declined, [62])
        self.assertEqual([r["index"] for r in between], [63])
        self.assertEqual(GT.scored_between_lines(rows), [
            f"scored between presses: chord 62 granted "
            f"{float(between[0]['scored_between']):g} before chord 63"])

    def test_the_play_status_carries_the_end_and_between_lines(self):
        # PITCH_DARK_RUN's rows of chords 61 to 64 and its end-read rows, in
        # the order the run logged them, as a play session's rows.
        rows = (_press_rows(*PITCH_DARK_RUN, "press-1-chords-61-64.csv")
                + _press_rows(*PITCH_DARK_RUN, "press-1-end-reads.csv"))
        ends = self._planned_ends(PITCH_DARK_SCHEDULE)
        tools = GT.GameTools(None, threading.RLock(), os.path.join(self.tmp, "scratch"))
        tools._play = GT._PlaySession(score_json="score.json", chord_count=len(rows),
                                      log_path=None, rows=rows, planned_ends=ends)
        lines = tools.ch_play_status().splitlines()
        for line in (GT.activation_end_lines(rows, ends) + GT.scored_between_lines(rows)):
            self.assertIn(line, lines)
        self.assertTrue([line for line in lines if line.startswith("end read after chord 817: ")])
        self.assertTrue([line for line in lines if line.startswith("scored between presses: ")])

    # -- the edge probe ------------------------------------------------------------------

    def _probe_rig(self, kit=True):
        seen = {}

        def live_play(chords, *, on_press, **kw):
            # One row per press, at exactly the time press_plan gives it.
            seen.update(kw, chords=list(chords))
            for press in play_chart.press_plan(chords):
                on_press({"index": press.index, "chart_ms": press.chart_ms,
                          "clock_ms_at_press": press.at_ms, "lead_ms": press.lead_ms,
                          "score_before": 0, "score_after": 50, "send": play_chart.SENT})
            return play_chart.RunResult(stop="done", sent=2)

        t = ToolRig(self.tmp, kit=kit, live_play=live_play)
        return t, seen

    def _probe(self, t, probes):
        reply = t.tools.ch_edge_probe(t.r.song, self._json("score.json", SCORE_JSON),
                                      self._json("schedule.json", SCHEDULE_JSON),
                                      self._json("probes.json", probes))
        if t.tools._play is not None:
            t.tools._play.thread.join(THREAD_CAP_S)
        return reply

    def test_the_play_status_names_each_probe_row_with_its_asked_lead(self):
        # The press log and probes file of a real probe run
        # (testdata/ch_runs/ls-e-run1, copied from
        # hydra-data/ch-mcp-runs/2026-11-07/ls-e-run1): each probe chord's row
        # carries the lead probes.json asked for, and the status gives one
        # probe line per such row, in the probes file's order.
        rows = _press_rows("ls-e-run1", "press-1.csv")
        with open(os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-e-run1", "probes.json"),
                  encoding="utf-8") as f:
            probes = json.load(f)
        marked = {r["index"]: float(r[edge_probe.PROBE_COLUMN]) for r in rows
                  if r[edge_probe.PROBE_COLUMN]}
        self.assertEqual(marked, {p["index"]: p["lead_ms"] for p in probes})
        tools = GT.GameTools(None, threading.RLock(), os.path.join(self.tmp, "scratch"))
        tools._play = GT._PlaySession(score_json="score.json", chord_count=len(rows),
                                      log_path=None, rows=rows)
        probe_lines = [line for line in tools.ch_play_status().splitlines()
                       if line.startswith("probe chord ")]
        self.assertEqual([line.split(":")[0] for line in probe_lines],
                         [f"probe chord {p['index']}" for p in probes])
        for p, line in zip(probes, probe_lines):
            asked = p["lead_ms"]
            self.assertIn(f"asked {abs(asked):.2f} ms {'early' if asked >= 0 else 'late'}, ",
                          line)

    def test_the_probe_moved_chords_reach_the_agent_as_play_entries_at_literal_times(self):
        # live_in_game builds the kit agent's `play` entries from the chords
        # the probe handed it. Chord 1 is at 1500 ms with an aim of -2 ms, so
        # unmoved it fires at 1498 and last sends at 1550; a probe 60 ms late
        # fires it at 1560, and its last send moves by the 62 ms the press moved.
        from tools.ch_mcp import kit_tools
        t, seen = self._probe_rig()
        self._probe(t, [{"index": 1, "lead_ms": -60}])
        entries = [kit_tools.play_entry(seq, press, None)
                   for seq, press in enumerate(play_chart.press_plan(seen["chords"]))]
        self.assertEqual([(e["index"], e["at_ms"], e["last_send_ms"]) for e in entries],
                         [(0, 998.0, 1050.0), (1, 1560.0, 1612.0)])

    def test_the_probe_needs_the_kit(self):
        # A GameTools with no kit set up, as the server starts.
        tools = GT.GameTools(None, threading.RLock(), os.path.join(self.tmp, "scratch"))
        reply = tools.ch_edge_probe(os.path.join(self.tmp, "song"), "score.json",
                                    os.path.join(_PROBE_TESTS, "edge_walk_pair_schedule.json"),
                                    os.path.join(_REPO_ROOT, "testdata", "ch_runs",
                                                 "ls-e-run1", "probes.json"))
        self.assertTrue(reply.startswith("refused: "), reply)
        self.assertIn("ch_kit_setup", reply)
        self.assertIsNone(tools._play)

    def test_no_play_tool_takes_a_lead_argument(self):
        # The lead is press_plan's (and a probe file's), never a tool argument.
        for tool in (GT.GameTools.ch_run_chart, GT.GameTools.ch_play_chart,
                     GT.GameTools.ch_edge_probe):
            with self.subTest(tool=tool.__name__):
                self.assertFalse([name for name in inspect.signature(tool).parameters
                                  if "lead" in name])


class DroppedStatusTest(unittest.TestCase):
    """ch_play_status with a dropped press: design point 5 and question 4 of
    docs/superpowers/plans/2026-11-08-player-freak-fixes.md, task T1."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)
        self.tmp = self._tmp.name

    def _status(self, rows, result):
        score = os.path.join(self.tmp, "score.json")
        _write(score, json.dumps(SCORE_JSON).encode())

        def live_play(chords, *, on_press, **kw):
            for row in rows:
                on_press(row)
            return result

        schedule = os.path.join(self.tmp, "schedule.json")
        _write(schedule, json.dumps(SCHEDULE_JSON).encode())
        t = ToolRig(self.tmp, live_play=live_play)
        t.tools.ch_play_chart(score, schedule, route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        return t.tools.ch_play_status().splitlines()

    def test_a_dropped_row_is_out_of_the_sent_counts_and_no_clock_jump(self):
        # ls-p3-run1's rows of chords 467 to 469 (testdata/ch_runs/ls-p3-run1,
        # copied from hydra-data/ch-mcp-runs/2026-11-12/ls-p3-run1): the
        # player dropped chord 468. The result counts the rows as play_chords
        # counts them, a message per chord.
        rows = _press_rows("ls-p3-run1", "press-1-chords-467-469.csv")
        sent = [r["index"] for r in rows if r["send"] == play_chart.SENT]
        dropped = [r["index"] for r in rows if r["send"] == play_chart.DROPPED]
        self.assertEqual(dropped, [468])
        tools = GT.GameTools(None, threading.RLock(), os.path.join(self.tmp, "scratch"))
        tools._play = GT._PlaySession(
            score_json="score.json", chord_count=len(rows), log_path=None, rows=rows,
            result=play_chart.RunResult(stop="done", sent=len(sent), dropped=len(dropped),
                                        rows=list(rows)))
        lines = tools.ch_play_status().splitlines()
        self.assertFalse([line for line in lines if "jumped back" in line])
        counts = lines.index(f"sent {len(sent)}")
        self.assertEqual(lines[counts + 1], "dropped: chords 468")

    def test_a_result_with_no_rows_of_its_own_counts_the_drop_as_logged(self):
        rows = [{**ROWS[0], "index": 152},
                {**ROWS[1], "index": 153, "send": play_chart.DROPPED}]
        lines = self._status(rows, play_chart.RunResult(
            stop="done", sent=1, dropped=1))
        self.assertFalse([line for line in lines if "jumped back" in line])

    def test_no_dropped_row_says_none(self):
        # ls-p3-run1's first three rows (testdata/ch_runs/ls-p3-run1, copied
        # from hydra-data/ch-mcp-runs/2026-11-12/ls-p3-run1): every chord sent.
        rows = _press_rows("ls-p3-run1", "press-1-rows-1-3.csv")
        self.assertEqual({r["send"] for r in rows}, {play_chart.SENT})
        tools = GT.GameTools(None, threading.RLock(), os.path.join(self.tmp, "scratch"))
        tools._play = GT._PlaySession(
            score_json="score.json", chord_count=len(rows), log_path=None, rows=rows,
            result=play_chart.RunResult(stop="done", sent=len(rows), rows=list(rows)))
        lines = tools.ch_play_status().splitlines()
        # The no-drop wording comes from its owner, dropped_line, given no rows.
        counts = lines.index(f"sent {len(rows)}")
        self.assertEqual(lines[counts + 1], GT.dropped_line([]))

    def test_the_dropped_line_from_rows(self):
        self.assertEqual(GT.dropped_line(ROWS), "dropped: none")
        self.assertEqual(GT.dropped_line([{**ROWS[0], "index": 153,
                                           "send": play_chart.DROPPED}]),
                         "dropped: chords 153")


# How long the forced press-log write blocks: the over-time of LS-A2 run 1's
# first stall, before chord 128 (the table in "What was checked" of
# docs/superpowers/plans/2026-11-12-player-stall-fixes.md).
BLOCKED_WRITE_S = 0.9


class _BlockingRowWriter:
    """The real press-log writer, whose first row costs BLOCKED_WRITE_S of
    the fake wall clock when, and only when, the player's thread writes it.
    That is what a write Windows holds does: it costs the player time only
    if the player is the one writing."""

    def __init__(self, real, wall: _FakeTime, player: dict):
        self._real, self._wall, self._player = real, wall, player

    def writeheader(self):
        return self._real.writeheader()

    def writerow(self, row):
        first = not self._player.get("blocked")
        self._player["blocked"] = True
        if first and threading.get_ident() == self._player.get("ident"):
            self._wall.t += BLOCKED_WRITE_S
        return self._real.writerow(row)

    def writerows(self, rows):
        return self._real.writerows(rows)


class BlockedPressLogTest(unittest.TestCase):
    """PS1 of docs/superpowers/plans/2026-11-12-player-stall-fixes.md: the
    real _play_body and the real play_chords on a fake song clock, with a
    press-log write that blocks 900 ms."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name
        self.addCleanup(self._tmp.cleanup)
        self.wall = _FakeTime()
        self.game = _FakeGame()
        # The player's thread, once live_play starts on it, and whether the
        # one blocked row has been written.
        self.player = {}
        real = play_chart._press_log_writer
        patch = mock.patch.object(
            play_chart, "_press_log_writer",
            new=lambda f: _BlockingRowWriter(real(f), self.wall, self.player))
        patch.start()
        self.addCleanup(patch.stop)

    def _play(self, wire_into_on_press: bool = False) -> play_chart.RunResult:
        blocking = _BlockingRowWriter(mock.Mock(), self.wall, self.player)

        def live_play(chords, *, on_press, should_stop, out, **kw):
            self.player["ident"] = threading.get_ident()
            if wire_into_on_press:
                given = on_press

                def on_press(row):
                    blocking.writerow(row)
                    given(row)
            return play_chart.play_chords(
                chords, _FakeSongClock(self.wall, start_s=0.5), self.game.read_score,
                self.game.press, now=self.wall.now, sleep=self.wall.sleep,
                on_press=on_press, should_stop=should_stop, out=out)

        t = ToolRig(self.tmp, live_play=live_play)
        score = os.path.join(self.tmp, "score.json")
        _write(score, json.dumps(SCORE_JSON).encode())
        schedule = os.path.join(self.tmp, "schedule.json")
        song = os.path.join(self.tmp, "songs", probe_songs.WINDOW_MAP)
        _write(schedule, json.dumps(for_song(SCHEDULE_JSON, song)).encode())
        t.tools.ch_play_chart(score, schedule, log=os.path.join(self.tmp, "press.csv"),
                              route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertFalse(t.tools._play.running())
        self.assertIsNone(t.tools._play.error)
        return t.tools._play.result

    def test_the_same_block_on_the_player_thread_still_drops(self):
        result = self._play(wire_into_on_press=True)
        self.assertEqual(result.stop, play_chart.STOP_DONE)
        self.assertGreater(result.dropped, 0)


if __name__ == "__main__":
    unittest.main()
