"""Tests for game_tools, the Clone Hero server's game-side tools.

No game, no keys and no real user files: the launch, the clock and the game
are test_bot_run's fakes, the user folders are temp folders, the input driver
records its keys, and live_play is a fake that delivers rows.
"""

from __future__ import annotations

import csv
import inspect
import json
import os
import re
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
from tools.ch_mcp import build_stamp  # noqa: E402
from tools.ch_mcp import menu_tools  # noqa: E402
from tools.ch_mcp.frida_tools import MENU_BLOCK, StateUnread  # noqa: E402
from tools.ch_mcp.tests.test_recorder_link import FakePopen  # noqa: E402
from tools.ch_mcp.tests.test_bot_run import (  # noqa: E402
    ORIGINAL, PATCHED, FakeLaunchAgent, Rig, StuckGame, _read, _write, wire_human)
from tools.ch_mcp.tests.test_game_files import FakeReg  # noqa: E402
from tools.ch_mcp.tests.test_hydra_end import FAKE  # noqa: E402
from tools.ch_probe import probe_songs  # noqa: E402
from tools.ch_probe import input_driver  # noqa: E402
from tools.ch_probe.experiments import play_chart  # noqa: E402
from tools.ch_probe.experiments.overlay_logs import PRESS_LOG_COLUMNS  # noqa: E402
from tools.ch_probe.tests.test_play_chart import (  # noqa: E402
    _FakeGame, _FakeSongClock, _FakeTime, _schedule_of)
from tools.ch_probe.input_driver import DEFAULT_BINDINGS, InputDriver, Lane  # noqa: E402

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

# The schedule fixtures under tools/ch_probe/tests (hydra_replay schedule of
# each chart, path 0) and the reply lines they give. Every number below is
# read off the fixture file: a declined chord's `kept_gems`, a squeeze's
# `anchor_chord`, `anchor_aim_ms`, `planned_end_ms`, `ratio`, `anchor_room_ms`
# and `squeeze_room_ms`, and a squeeze press's `aim_ms` against its
# `aim_end_ms` (15 ms either side of the end on every timed squeeze here).
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

    def __call__(self, chords, *, driver, on_press, should_stop, out):
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


class ToolRig:
    """A GameTools wired to test_bot_run's Rig and the fakes above."""

    def __init__(self, tmp: str, **tool_args):
        self.r = Rig(tmp, exits_at=None)
        self.scratch_root = os.path.join(tmp, "scratch")
        self.driver = RecordingDriver()
        self.hwnd = WINDOW
        self.live_play = FakeLivePlay(play_chart.RunResult(
            sent=2, rows=list(ROWS), stop="stopped"))
        self.events = []
        self.watches = []
        self.popen = FakePopen()

        def make_watch():
            self.watches.append(FakeWatch(self.events))
            return self.watches[-1]

        args = dict(driver=self.driver, live_play=self.live_play,
                    find_window=lambda: self.hwnd, make_watch=make_watch,
                    make_recorder=self.make_recorder)
        args.update(tool_args)
        self.tools = GT.GameTools(self.r.env, threading.RLock(), self.scratch_root, **args)

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

    def test_session_dir_is_the_scratch_folder_the_backups_go_in(self):
        t = ToolRig(self.tmp)
        folder = t.tools.session_dir()
        self.assertTrue(os.path.isdir(folder))
        self.assertTrue(folder.startswith(t.scratch_root))
        self.assertEqual(t.tools.session_dir(), folder)
        t.tools.ch_start(t.r.song)
        self.assertEqual(os.path.dirname(t.backup_dir()), t.tools.session_dir())

    # -- the quiet check ----------------------------------------------------------

    def test_a_machine_that_is_not_quiet_refuses_with_no_folder_no_backup_no_launch(self):
        t = ToolRig(self.tmp)
        with mock.patch.object(Q, "LIVE_ENV", FakeQuiet(cpu=40.0).env()):
            reply = t.tools.ch_start(t.r.song)
        self.assertTrue(reply.startswith("refused: the machine is not quiet: "), reply)
        self.assertIn("CPU: 40.00 percent", reply)
        self.assertTrue(reply.endswith("; hold the other sessions' builds and call ch_start again"))
        self.assertFalse(os.path.exists(t.scratch_root))
        self.assertIsNone(t.tools._scratch)
        self.assertIsNone(t.tools._run)
        self.assertEqual(t.r.launch.commands, [])
        self.assertEqual(t.tools.ch_stop(),
                         "no run is open and no backup was taken; nothing to restore")

    def test_a_quiet_machine_puts_the_report_before_the_launch_lines(self):
        t = ToolRig(self.tmp)
        lines = t.tools.ch_start(t.r.song).splitlines()
        cpu = lines.index("CPU: 3.00 percent averaged over 5 s (limit 15 percent)")
        backup = lines.index(f"backup folder: {t.backup_dir()}")
        starting = next(i for i, line in enumerate(lines) if line.startswith("starting: "))
        self.assertEqual(lines[0], "no compiler running")
        self.assertLess(cpu, backup)
        self.assertLess(backup, starting)
        self.assertIn("free memory: 1.0 GB", lines)
        self.assertIn("power scheme: Balanced", lines)

    def test_the_check_runs_once_and_waits_quiet_sample_s(self):
        fake = FakeQuiet()
        t = ToolRig(self.tmp)
        with mock.patch.object(Q, "LIVE_ENV", fake.env()):
            t.tools.ch_start(t.r.song)
        self.assertEqual(fake.calls.count(("sample", Q.QUIET_SAMPLE_S)), 1)

    def test_an_open_run_is_stopped_first_so_the_game_is_not_read_as_load(self):
        t = ToolRig(self.tmp)
        t.tools.ch_start(t.r.song)
        scratch = t.tools._scratch
        in_scratch = os.listdir(scratch)
        with mock.patch.object(Q, "LIVE_ENV", FakeQuiet(cpu=40.0).env()):
            lines = t.tools.ch_start(t.r.song).splitlines()
        self.assertEqual(lines[0], "a run was open; stopping it first:")
        self.assertIn(f"{G.SETTINGS}: unchanged", lines)
        self.assertTrue(lines[-1].startswith("refused: the machine is not quiet: "))
        self.assertIsNone(t.tools._run)
        self.assertEqual(t.tools._scratch, scratch)
        self.assertEqual(os.listdir(scratch), in_scratch)
        self.assertEqual([d for d in os.listdir(t.scratch_root)
                          if d.startswith(GT.SCRATCH_PREFIX)], [os.path.basename(scratch)])
        self.assertFalse(os.path.exists(os.path.join(t.scratch_root, GT.OPEN_RUN_MARKER)))

    def test_a_refusal_with_no_need_of_the_machine_takes_no_sample(self):
        fake = FakeQuiet()
        t = ToolRig(self.tmp)
        with mock.patch.object(Q, "LIVE_ENV", fake.env()):
            self.assertEqual(t.tools.ch_start(human=True), B.HUMAN_NEEDS_SONG)
        self.assertEqual(fake.calls, [])

    def test_ch_start_has_no_argument_that_skips_the_check(self):
        self.assertEqual(list(inspect.signature(GT.GameTools.ch_start).parameters),
                         ["self", "song_folder", "player", "profile", "human"])

    # -- ch_start ----------------------------------------------------------------

    def test_ch_start_with_a_song_backs_up_before_the_launch_and_names_it_all(self):
        t = ToolRig(self.tmp)
        seen_at_launch = []
        real_launch = t.r.launch

        def launch(cmd):
            seen_at_launch.append([d for d in os.listdir(t.tools._scratch)
                                   if d.startswith(G.BACKUP_PREFIX)])
            return real_launch(cmd)

        t.r.env.launch = launch
        dumps_before = len(FAKE.charts)
        reply = t.tools.ch_start(t.r.song)
        # One dump of the song's chart serves the recorder's cap and the run's.
        self.assertEqual(FAKE.charts[dumps_before:], [t.r.chart])
        backup = t.backup_dir()
        self.assertEqual(seen_at_launch, [[os.path.basename(backup)]])
        self.assertTrue(backup.startswith(t.scratch_root))
        lines = reply.splitlines()
        self.assertIn(f"backup folder: {backup}", lines)
        self.assertTrue(any(line.startswith("starting: ") and t.r.song in line for line in lines))
        self.assertIn("game process id: 4242", lines)
        # Hydra's end, from the dump made for the song's chart.
        self.assertEqual(lines[-1], "song length: 4000 ms, from "
                         + hydra_end.chart_source_text(t.r.chart))
        self.assertEqual(t.tools.launched_at(), 0.0)

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
        t = ToolRig(self.tmp)
        t.tools.ch_start(t.r.song)
        first_backup = t.backup_dir()
        _write(t.r.path(G.SETTINGS), b"changed by the game")
        lines = t.tools.ch_start(t.r.song).splitlines()
        self.assertEqual(lines[0], "a run was open; stopping it first:")
        restored = lines.index(f"{G.SETTINGS}: restored")
        second_start = [i for i, line in enumerate(lines) if line.startswith("starting: ")]
        self.assertEqual(len(second_start), 1)
        self.assertLess(restored, second_start[0])
        self.assertIn(f"restoring the user files from {first_backup}", lines)
        self.assertTrue(t.r.game.stopped)
        self.assertEqual(len(t.r.launch.commands), 2)
        self.assertNotEqual(t.backup_dir(), first_backup)
        t.r.assert_all_restored(self)

    # -- ch_stop and shutdown -----------------------------------------------------

    def test_ch_stop_stops_the_game_and_restores_all_five_files(self):
        t = ToolRig(self.tmp)
        t.tools.ch_start(t.r.song)
        for name in ORIGINAL:
            _write(t.r.path(name), b"changed by the game")
        lines = t.tools.ch_stop().splitlines()
        for name in G.DOCUMENTS_FILES + G.LOCALLOW_FILES:
            self.assertIn(f"{name}: restored", lines)
        self.assertIn("stopped the game", lines)
        self.assertFalse(any("--stop" in line for line in lines))
        self.assertTrue(t.r.game.stopped)
        t.r.assert_all_restored(self)
        self.assertIsNone(t.tools.launched_at())

    def test_ch_stop_runs_the_stop_hooks_first_in_order(self):
        t = ToolRig(self.tmp)
        t.tools.ch_start(t.r.song)
        calls = []
        t.tools.add_stop_hook(lambda: calls.append(("first", t.r.game.stopped)) or "trace stopped")
        t.tools.add_stop_hook(lambda: calls.append(("second", t.r.game.stopped)))
        lines = t.tools.ch_stop().splitlines()
        self.assertEqual(calls, [("first", False), ("second", False)])
        self.assertLess(lines.index("trace stopped"), lines.index("stopped the game"))

    def test_ch_stop_says_when_the_game_outlives_the_wait_and_still_restores(self):
        t = ToolRig(self.tmp)
        t.r.game = t.r.launch.game = StuckGame(t.r.clock)
        t.tools.ch_start(t.r.song)
        for name in ORIGINAL:
            _write(t.r.path(name), b"changed by the game")
        reply = t.tools.ch_stop()
        self.assertIn("(pid 4242) was told to stop but was still running after 30 s", reply)
        self.assertNotIn("stopped the game\n", reply)
        t.r.assert_all_restored(self)

    def test_a_failed_restore_names_the_file_and_folder_and_nothing_further_runs(self):
        t = ToolRig(self.tmp)
        t.tools.ch_start(t.r.song)
        backup = t.backup_dir()

        def fail(backup_dir, out, leave=()):
            raise G.RestoreError("scoredata.bin", "locked")

        with mock.patch.object(G, "restore", fail):
            lines = t.tools.ch_start(t.r.song).splitlines()
        self.assertIn("restore failed on scoredata.bin: locked", lines)
        self.assertIn(f"stopped; the backup is still in {backup}", lines)
        self.assertEqual(lines[-1], "the new run was not started")
        self.assertEqual(len(t.r.launch.commands), 1)
        # The run stays open, so the next ch_stop tries the restore again.
        self.assertEqual(t.backup_dir(), backup)
        self.assertIn("scoredata.bin: unchanged", t.tools.ch_stop())

    def test_shutdown_terminates_a_server_started_game_and_restores(self):
        t = ToolRig(self.tmp)
        t.tools.ch_start(t.r.song)
        _write(t.r.path("user.bin"), b"changed by the game")
        reply = t.tools.shutdown()
        self.assertTrue(t.r.game.stopped)
        self.assertIn("user.bin: restored", reply)
        t.r.assert_all_restored(self)

    def test_shutdown_with_an_adopted_game_restores_only(self):
        t = ToolRig(self.tmp)
        t.tools.ensure_backup()
        backup = t.backup_dir()
        _write(t.r.path("user.bin"), b"changed by the game")
        reply = t.tools.shutdown()
        self.assertEqual(t.r.launch.commands, [])
        self.assertFalse(t.r.game.stopped)
        self.assertIn("user.bin: restored", reply)
        self.assertIn("the server did not start the game, so it does not close it", reply)
        self.assertEqual(reply.splitlines()[-1],
                         "if the game is still running it may write these files again when it "
                         "closes; after closing it, run py tools/ch_mcp/bot_run.py --restore "
                         f'"{backup}"')
        t.r.assert_all_restored(self)

    # -- ensure_backup and launched_at ----------------------------------------------

    def test_ensure_backup_on_an_adopted_game_takes_one_backup(self):
        t = ToolRig(self.tmp)
        first = t.tools.ensure_backup()
        backup = t.backup_dir()
        self.assertTrue(first.startswith(f"backup folder: {backup} "))
        self.assertTrue(os.path.isfile(os.path.join(backup, G.BACKUP_MANIFEST)))
        self.assertIsNone(t.tools.launched_at())
        self.assertEqual(t.tools.ensure_backup(), "")
        self.assertEqual(sorted(os.listdir(t.scratch_root)),
                         sorted([os.path.basename(t.tools._scratch), GT.OPEN_RUN_MARKER]))

    def test_ensure_backup_takes_none_when_a_run_holds_one(self):
        t = ToolRig(self.tmp)
        t.tools.ch_start(t.r.song)
        self.assertEqual(t.tools.ensure_backup(), "")

    # -- the stats files ----------------------------------------------------------

    def test_install_cap_is_the_approved_sixty_seconds(self):
        self.assertEqual(GT.INSTALL_CAP_S, 60.0)

    def test_ch_install_chart_that_outlasts_the_cap_says_so(self):
        release = threading.Event()

        def slow_install(src, *, name=None):
            release.wait(THREAD_CAP_S)
            return "never read"

        t = ToolRig(self.tmp, install=slow_install, wait_thread=lambda thread, cap_s: None)
        try:
            reply = t.tools.ch_install_chart("chart folder")
        finally:
            release.set()
        self.assertTrue(reply.startswith("not finished after 60 s (INSTALL_CAP_S)"), reply)

    def test_ch_install_chart_names_the_chart_or_the_refusal(self):
        def install(src, *, name=None):
            if name == "taken":
                raise I.InstallRefused("taken already exists; not overwritten")
            return os.path.join(probe_songs.DEFAULT_OUT, name, "notes.mid")

        t = ToolRig(self.tmp, install=install)
        self.assertEqual(t.tools.ch_install_chart("src", name="new"),
                         "installed; its chart file is "
                         + os.path.join(probe_songs.DEFAULT_OUT, "new", "notes.mid"))
        self.assertEqual(t.tools.ch_install_chart("src", name="taken"),
                         "refused: taken already exists; not overwritten")

    def test_ch_wait_stats_copies_the_stats_and_ch_stats_reads_the_copy(self):
        t = ToolRig(self.tmp)
        self.assertEqual(t.tools.ch_wait_stats(),
                         "no run is open; start one with ch_start(song_folder)")
        t.tools.ch_start(t.r.song)
        t.r.game_writes_stats_at(5)
        reply = t.tools.ch_wait_stats()
        self.assertIn("scorestats.json changed after 5 s; copied to ", reply)
        copy = t.tools._last_stats
        self.assertTrue(copy.startswith(t.tools._scratch))
        self.assertIn(f"last stats copy: {copy}", t.tools.ch_status())
        self.assertEqual(t.tools.ch_stats().splitlines(),
                         [f"stats: {copy}", "players[0]:", "  is_bot: true"])

    def test_ch_stats_prints_every_key_in_the_files_order(self):
        path = os.path.join(self.tmp, "stats.json")
        _write(path, json.dumps({
            "game_version": "v1", "band_score": 10,
            "players": [{"instrument": "Drums", "modifiers": ["DoubleKick"], "no_fail": True,
                         "score": 10}]}).encode())
        t = ToolRig(self.tmp)
        self.assertEqual(t.tools.ch_stats(path).splitlines(), [
            f"stats: {path}", "game_version: v1", "band_score: 10", "players[0]:",
            "  instrument: Drums", '  modifiers: ["DoubleKick"]', "  no_fail: true",
            "  score: 10"])

    # The run's settings the ch_compare tests pass, and the stats file holds.
    COMPARE_RUN = compare_scorestats.RunSettings(
        "AF44D5C8D941BB33444D18E7871CDC77", 100, True, ("DoubleKick",))

    def _compare_files(self):
        stats = os.path.join(self.tmp, "stats.json")
        score = os.path.join(self.tmp, "score.json")
        run = self.COMPARE_RUN
        _write(stats, json.dumps({
            "checksum": run.checksum, "playback_speed": run.playback_speed,
            "players": [{"total_notes": 2, "notes_hit": 2, "sp_phrases_total": 0,
                         "sp_phrases_earned": 0, "base_score": 100, "note_score": 100,
                         "score": 100, "accent_notes_hit": 0, "ghost_notes_hit": 0,
                         "sp_activations": 0, "no_fail": run.no_fail,
                         "modifiers": list(run.modifiers)}]}).encode())
        _write(score, json.dumps({
            "chords": [{"notes": [{"dynamic": "none"}], "is_sp_phrase_end": False}] * 2,
            "final": {"notes": 100, "accent": 0, "ghost": 0, "total": 100},
            "activations": []}).encode())
        return stats, score

    def test_ch_compare_prints_compares_lines_then_the_verdict(self):
        stats, score = self._compare_files()
        run = self.COMPARE_RUN
        expected = []
        self.assertTrue(compare_scorestats.compare(stats, score, run=run, out=expected.append))
        t = ToolRig(self.tmp)
        reply = t.tools.ch_compare(stats, score, run.checksum, run.playback_speed, run.no_fail,
                                   list(run.modifiers))
        self.assertEqual(reply.splitlines(), expected + ["pass"])

    def test_ch_compare_without_the_run_settings_fails_on_the_settings_line(self):
        stats, score = self._compare_files()
        expected = []
        self.assertFalse(compare_scorestats.compare(stats, score, out=expected.append))
        t = ToolRig(self.tmp)
        self.assertEqual(t.tools.ch_compare(stats, score).splitlines(), expected + ["fail"])

    def test_ch_compare_refuses_some_but_not_all_of_the_run_settings(self):
        stats, score = self._compare_files()
        t = ToolRig(self.tmp)
        self.assertEqual(
            t.tools.ch_compare(stats, score, checksum=self.COMPARE_RUN.checksum, no_fail=True),
            "refused: give all four of the run's settings, or none; missing: "
            "playback_speed, modifiers")

    def test_ch_compare_takes_an_empty_modifier_list_as_given(self):
        stats, score = self._compare_files()
        run = self.COMPARE_RUN
        reply = ToolRig(self.tmp).tools.ch_compare(stats, score, run.checksum,
                                                   run.playback_speed, run.no_fail, [])
        self.assertIn("modifiers: FAIL; ", reply)
        self.assertEqual(reply.splitlines()[-1], "fail")

    # -- keys -------------------------------------------------------------------------

    def test_ch_press_focuses_the_window_and_presses_the_bound_keys(self):
        t = ToolRig(self.tmp)
        focused = []
        with mock.patch.object(play_chart, "focus_window", focused.append):
            reply = t.tools.ch_press("red,kick")
        red, kick = DEFAULT_BINDINGS[Lane.RED], DEFAULT_BINDINGS[Lane.KICK]
        self.assertEqual(focused, [WINDOW])
        self.assertEqual(t.driver.events, [(red, False), (kick, False), (red, True), (kick, True)])
        self.assertEqual(reply.splitlines()[-1], "pressed red, kick; keys sent: S, L")
        # No run was open, so the press took the adopted game's backup first.
        self.assertTrue(reply.startswith(f"backup folder: {t.backup_dir()} "))

    def test_ch_press_refuses_an_unknown_lane_with_the_list(self):
        t = ToolRig(self.tmp)
        reply = t.tools.ch_press("purple")
        self.assertEqual(reply, 'unknown lane "purple"; valid lanes: green, red, yellow, blue, '
                                "kick, yellow_cymbal, blue_cymbal, green_cymbal")
        self.assertEqual(t.driver.events, [])
        self.assertIsNone(t.backup_dir())

    def test_ch_press_with_no_window_says_so(self):
        t = ToolRig(self.tmp)
        t.hwnd = 0
        self.assertTrue(t.tools.ch_press("red").startswith("no game window found"))
        self.assertEqual(t.driver.events, [])

    # -- menu keys ------------------------------------------------------------------------

    def _key_rig(self):
        """A ToolRig whose driver records keys, typed characters and sleeps
        into one list, beside the backup and the focus."""
        events = []

        class KeyDriver(InputDriver):
            def send_key(self, vk, key_up):
                events.append(("key", vk, key_up))

            def send_unicode(self, unit, key_up):
                events.append(("char", chr(unit), key_up))

            def send_input(self, event):
                raise AssertionError("a test reached the real SendInput")

        t = ToolRig(self.tmp, driver=KeyDriver())
        real_backup = t.tools.ensure_backup

        def backup():
            events.append(("backup",))
            return real_backup()

        t.tools.ensure_backup = backup
        real_sleep = t.r.env.sleep
        t.r.env.sleep = lambda s: (events.append(("sleep", s)), real_sleep(s))
        return t, events

    def _ch_key(self, keys):
        t, events = self._key_rig()
        with mock.patch.object(play_chart, "focus_window",
                               lambda hwnd: events.append(("focus", hwnd))):
            reply = t.tools.ch_key(keys)
        return t, events, reply

    def test_ch_key_backs_up_focuses_once_then_presses_and_types_in_order(self):
        song = probe_songs.WINDOW_MAP
        t, events, reply = self._ch_key(f"enter,down,text:{song}")
        gap = ("sleep", input_driver.MENU_KEY_GAP_S)  # pinned in test_input_driver
        hold = ("sleep", input_driver.KEY_HOLD_S)
        enter, down = input_driver.MENU_KEYS["enter"], input_driver.MENU_KEYS["down"]
        typed = []
        for i, char in enumerate(song):
            if i:
                typed.append(gap)
            typed += [("char", char, False), ("char", char, True)]
        self.assertEqual(events, [("backup",), ("focus", WINDOW),
                                  ("key", enter, False), hold, ("key", enter, True), gap,
                                  ("key", down, False), hold, ("key", down, True), gap]
                         + typed)
        self.assertEqual(reply.splitlines()[-1],
                         f'keys sent: enter, down, text "{song}" (10 characters)')
        self.assertTrue(reply.startswith(f"backup folder: {t.backup_dir()} "))

    def test_ch_key_types_a_comma_inside_the_text(self):
        _t, events, reply = self._ch_key("text:a,b")
        self.assertEqual([e[1] for e in events if e[0] == "char" and not e[2]], ["a", ",", "b"])
        self.assertEqual(reply.splitlines()[-1], 'keys sent: text "a,b" (3 characters)')

    def test_ch_key_refuses_an_unknown_key_with_the_list_and_sends_nothing(self):
        t, events, reply = self._ch_key("f1")
        self.assertEqual(reply, 'unknown key "f1"; valid keys: enter, escape, space, backspace, '
                                "tab, up, down, left, right, and text:<string>")
        self.assertEqual(events, [])
        self.assertIsNone(t.backup_dir())

    def test_ch_key_checks_every_key_before_sending_any(self):
        _t, events, reply = self._ch_key("enter,purple")
        self.assertTrue(reply.startswith('unknown key "purple"'))
        self.assertEqual(events, [])

    def test_ch_key_with_no_window_says_so(self):
        t, events = self._key_rig()
        t.hwnd = 0
        self.assertTrue(t.tools.ch_key("enter").startswith("no game window found"))
        self.assertEqual(events, [])

    def test_press_keys_takes_the_list_already_split(self):
        t, events = self._key_rig()
        with mock.patch.object(play_chart, "focus_window", lambda hwnd: None):
            result = t.tools.press_keys(["escape", "text:x,y"])
        self.assertTrue(result.sent)
        self.assertEqual(result.text.splitlines()[-1],
                         'keys sent: escape, text "x,y" (3 characters)')

    def test_press_keys_says_sent_false_for_each_refusal(self):
        t, events = self._key_rig()
        unknown = t.tools.press_keys(["enter", "purple"])
        self.assertFalse(unknown.sent)
        self.assertTrue(unknown.text.startswith('unknown key "purple"'))
        self.assertFalse(t.tools.press_keys(["text:"]).sent)
        self.assertFalse(t.tools.press_keys([]).sent)
        t.hwnd = 0
        no_window = t.tools.press_keys(["enter"])
        self.assertFalse(no_window.sent)
        self.assertTrue(no_window.text.startswith("no game window found"))
        self.assertEqual(events, [])

    def test_press_lanes_says_sent_false_for_each_refusal(self):
        t = ToolRig(self.tmp)
        unknown = t.tools.press_lanes("purple")
        self.assertFalse(unknown.sent)
        self.assertTrue(unknown.text.startswith('unknown lane "purple"'))
        self.assertFalse(t.tools.press_lanes("").sent)
        t.hwnd = 0
        self.assertFalse(t.tools.press_lanes("red").sent)

    def test_press_lanes_says_sent_true_and_ch_press_prints_the_same_text(self):
        t = ToolRig(self.tmp)
        with mock.patch.object(play_chart, "focus_window", lambda hwnd: None):
            result = t.tools.press_lanes("red")
            self.assertTrue(result.sent)
            self.assertEqual(t.tools.ch_press("red").splitlines()[-1],
                             result.text.splitlines()[-1])

    def test_split_keys_keeps_the_text_whole(self):
        self.assertEqual(GT.split_keys("enter, down ,text:Ladies Talk, fill"),
                         ["enter", "down", "text:Ladies Talk, fill"])
        self.assertEqual(GT.split_keys(" text: a "), ["text: a "])

    # -- the run cap ----------------------------------------------------------------------

    def test_run_cap_is_the_song_plus_the_extra_minute_else_240_s(self):
        t = ToolRig(self.tmp)
        self.assertEqual(t.tools.cap_for_end(260504), 320.504)
        self.assertEqual(GT.cap_for_end(260504), 320.504)
        self.assertEqual(GT.cap_for_end(None), 240.0)

    def test_the_schedules_run_cap_reads_its_song_end_and_refuses_null(self):
        schedule = self._schedule_json({**SCHEDULE_JSON, "song_length_ms": 260504})
        self.assertEqual(GT.schedule_run_cap_s(schedule), (320.504, 260504))
        null = self._schedule_json({**SCHEDULE_JSON, "song_length_ms": None})
        with self.assertRaises(hydra_end.SongEndMissing):
            GT.schedule_run_cap_s(null)

    def test_ch_play_chart_with_a_song_folder_passes_the_cap_to_live_play(self):
        seen = []

        def live_play(chords, **kw):
            seen.append(kw.get("cap_s"))
            return play_chart.RunResult(stop="done")

        t = ToolRig(self.tmp, live_play=live_play)
        # The schedule's song end sets the cap, not a dump of the folder's
        # chart, whose fake end differs.
        FAKE.set_end(t.r.chart, 9999)
        schedule = self._schedule_json({**SCHEDULE_JSON, "song_length_ms": 260504})
        dumps_before = len(FAKE.charts)
        reply = t.tools.ch_play_chart(self._score_json(), schedule, song_folder=t.r.song,
                                      route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertEqual(len(FAKE.charts), dumps_before)
        self.assertEqual(seen, [320.504])
        self.assertIn("run cap: 320.504 s", reply.splitlines())

    def test_the_status_prints_the_threshold_the_player_read(self):
        def live_play(chords, **kw):
            return play_chart.RunResult(stop="done", dyn_threshold=64)

        t = ToolRig(self.tmp, live_play=live_play)
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(),route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertEqual(t.tools.ch_play_status().splitlines()[2], "dynamics threshold: 64")

    def test_the_status_prints_no_threshold_when_none_was_read(self):
        t = ToolRig(self.tmp, live_play=lambda chords, **kw: play_chart.RunResult(stop="done"))
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(),route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertNotIn("threshold", t.tools.ch_play_status())

    def test_the_status_names_the_first_chord_pressed_from_the_oldest_row(self):
        def live_play(chords, **kw):
            for row in ({**ROWS[0], "index": 7, "chart_ms": 5398.0},
                        {**ROWS[1], "index": 8, "chart_ms": 5600.0}):
                kw["on_press"](row)
            return play_chart.RunResult(stop="done", sent=2)

        t = ToolRig(self.tmp, live_play=live_play)
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(),route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertIn("first press chord index 7 at 5398.0 ms",
                      t.tools.ch_play_status().splitlines())

    def test_the_status_names_the_first_chord_while_the_run_goes_on(self):
        t = ToolRig(self.tmp)
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(),route="keys")
        self.assertTrue(t.live_play.delivered.wait(THREAD_CAP_S))
        self.assertIn("first press chord index 0 at 1000.0 ms",
                      t.tools.ch_play_status().splitlines())
        t.tools.ch_play_stop()
        t.tools._play.thread.join(THREAD_CAP_S)

    def test_the_status_says_when_no_chord_was_pressed(self):
        t = ToolRig(self.tmp, live_play=lambda chords, **kw: play_chart.RunResult(stop="done"))
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(),route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertIn("first press: none, no chord was pressed",
                      t.tools.ch_play_status().splitlines())

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
        t = ToolRig(self.tmp)
        score = self._score_json()
        first = t.tools.ch_play_chart(score, self._schedule_json(), route="keys")
        self.assertIn("play session started: 2 chords", first)
        self.assertIn("waiting for a moving clock (cap 120 s)", first)
        self.assertTrue(t.live_play.delivered.wait(THREAD_CAP_S))
        self.assertEqual(t.tools.ch_play_chart(score, self._schedule_json()),
                         "a play session is already running; call ch_play_stop first")
        self.assertEqual(len(t.live_play.calls), 1)
        t.tools.ch_play_stop()
        t.tools._play.thread.join(THREAD_CAP_S)

    def test_ch_play_stop_sets_the_flag_and_ch_play_status_reports_the_result(self):
        t = ToolRig(self.tmp)
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(),log="press.csv", route="keys")
        self.assertTrue(t.live_play.delivered.wait(THREAD_CAP_S))
        snapshot = t.tools.play_snapshot()
        self.assertEqual(snapshot, {"last_row": ROWS[-1], "press_count": 2, "stop": None,
                                    "message": ""})
        self.assertIn("running: 2 presses so far", t.tools.ch_play_status())

        stop_reply = t.tools.ch_play_stop()
        self.assertTrue(t.tools._play.stop.is_set())
        self.assertEqual(stop_reply.splitlines()[-1], "stopped")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertFalse(t.tools._play.running())

        lines = t.tools.ch_play_status().splitlines()
        self.assertIn("ended: stopped", lines)
        self.assertIn("sent 2", lines)
        log = os.path.join(t.tools._scratch, "press.csv")
        self.assertIn(f"press log: {log} (2 rows)", lines)
        self.assertEqual(lines[-2:], ["  1, 1500.0, 1499.0, 50, 50, sent",
                                      "  0, 1000.0, 998.0, 0, 50, sent"])
        self.assertTrue(os.path.isfile(log))
        self.assertEqual(t.tools.play_snapshot(),
                         {"last_row": ROWS[-1], "press_count": 2, "stop": "stopped",
                          "message": ""})

    def test_play_snapshot_is_none_with_no_play_session(self):
        t = ToolRig(self.tmp)
        self.assertIsNone(t.tools.play_snapshot())
        self.assertEqual(t.tools.ch_play_status(), "no play session has run")

    def test_a_run_with_no_engine_reports_the_finders_message(self):
        t = ToolRig(self.tmp, live_play=lambda chords, **kw: play_chart.RunResult(
            stop=play_chart.STOP_NO_ENGINE, message="no song started within 120 s"))
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(),route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        lines = t.tools.ch_play_status().splitlines()
        self.assertIn(f"ended: {play_chart.STOP_NO_ENGINE}", lines)
        self.assertIn("no song started within 120 s", lines)
        self.assertEqual(t.tools.play_snapshot()["message"], "no song started within 120 s")

    def test_ch_stop_tells_a_running_play_session_to_stop(self):
        t = ToolRig(self.tmp)
        t.tools.ch_play_chart(self._score_json(), self._schedule_json(),route="keys")
        self.assertTrue(t.live_play.delivered.wait(THREAD_CAP_S))
        reply = t.tools.ch_stop()
        self.assertIn("play session told to stop", reply)
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertEqual(t.tools.play_snapshot()["stop"], "stopped")
        # The play run took the adopted game's backup, and ch_stop restored it.
        self.assertIn("settings.ini: unchanged", reply)

    # -- the kit route ----------------------------------------------------------------------

    def _kit_rig(self, kit_press):
        seen = []

        def live_play(chords, **kw):
            seen.append(kw)
            return play_chart.RunResult(stop="done", rows=[])

        t = ToolRig(self.tmp, live_play=live_play)
        t.tools.use_kit_press(lambda: kit_press)
        return t, seen

    def test_route_kit_hands_live_play_the_kit_press_and_no_driver(self):
        def kit_press(lanes):
            return {}

        t, seen = self._kit_rig(kit_press)
        reply = t.tools.ch_play_chart(self._score_json(), self._schedule_json(),route="kit")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertIs(seen[0]["press"], kit_press)
        self.assertNotIn("driver", seen[0])
        self.assertIn("route: kit (presses through the virtual kit inside the game)",
                      reply.splitlines())

    def test_no_route_picks_the_kit_once_it_is_set_up(self):
        def kit_press(lanes):
            return {}

        t, seen = self._kit_rig(kit_press)
        t.tools.ch_play_chart(self._score_json(), self._schedule_json())
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertIs(seen[-1]["press"], kit_press)

    NO_ROUTE_REFUSED = ("refused: no route given and the kit is not set up; "
                        "call ch_kit_setup first")

    def test_no_route_is_refused_while_the_kit_is_not_set_up(self):
        t, seen = self._kit_rig(None)
        self.assertEqual(t.tools.ch_play_chart(self._score_json(), self._schedule_json()), self.NO_ROUTE_REFUSED)
        self.assertIsNone(t.tools._play)
        self.assertEqual(seen, [])

    def test_no_route_is_refused_with_no_kit_tools_and_presses_no_key(self):
        t = ToolRig(self.tmp)
        self.assertEqual(t.tools.ch_play_chart(self._score_json(), self._schedule_json()), self.NO_ROUTE_REFUSED)
        self.assertIsNone(t.tools._play)
        self.assertEqual(t.live_play.calls, [])
        self.assertEqual(t.driver.events, [])

    def test_route_keys_presses_keys_through_the_driver(self):
        t, seen = self._kit_rig(None)
        reply = t.tools.ch_play_chart(self._score_json(), self._schedule_json(),route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertIs(seen[-1]["driver"], t.driver)
        self.assertNotIn("press", seen[-1])
        self.assertIn("route: keys (keys to the focused game window)", reply.splitlines())

    def test_route_kit_without_a_kit_and_an_unknown_route_are_refused(self):
        t, seen = self._kit_rig(None)
        self.assertEqual(t.tools.ch_play_chart(self._score_json(), self._schedule_json(),route="kit"),
                         'route kit: the kit is not set up; call ch_kit_setup first, '
                         'or pass route="keys"')
        self.assertEqual(t.tools.ch_play_chart(self._score_json(), self._schedule_json(),route="pads"),
                         'unknown route "pads"; routes: kit, keys')
        self.assertIsNone(t.tools._play)
        self.assertEqual(seen, [])

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

    def test_a_declined_chord_sends_its_kept_gems_and_the_reply_names_them(self):
        t, seen = self._decline_rig()
        score, schedule = self._decline_files(kept=(0,))
        reply = t.tools.ch_play_chart(score, schedule, route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertEqual([c.declined for c in seen[0]], [False, True])
        self.assertEqual([p.lanes for p in seen[0][1].presses], [[Lane.KICK]])
        self.assertIn("declined: chords 1 (kick)", reply.splitlines())

    def test_a_declined_chord_that_keeps_no_gem_says_nothing_sent(self):
        t, seen = self._decline_rig()
        score, schedule = self._decline_files(kept=())
        reply = t.tools.ch_play_chart(score, schedule, route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertEqual([p.lanes for p in seen[0][1].presses], [[]])
        self.assertIn("declined: chords 1 (nothing sent)", reply.splitlines())

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

        t = ToolRig(self.tmp, live_play=live_play)
        t.tools.use_kit_press(lambda: (lambda lanes: {}))
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

    class ChainKit:
        def __init__(self, events, press):
            self.events, self.press = events, press

        def setup_for_run(self, player):
            self.events.append(("setup", player))
            return self.press, "setup reply"

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
                   loaded=None):
        """A chained-start rig whose game has `loaded` loaded; None means the
        rig's own song folder."""
        events = []
        seen = {}

        def played(chords, *, on_press, **kw):
            events.append("play")
            seen.update(kw, chords=chords, engine=kw["find_engine"]("a proc"))
            on_press(ROWS[0])
            return play_chart.RunResult(stop="done")

        press = press if press is not None else (lambda lanes, axes=None: {})
        t = ToolRig(self.tmp, live_play=live_play or played)
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

    def test_the_players_press_is_the_kit_press_setup_gave_and_it_has_no_driver(self):
        def press(lanes, axes=None):
            return {}

        t, _events, seen = self._chain_rig(press=press)
        self._run(t)
        self.assertIs(seen["press"], press)
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

    def test_the_call_returns_at_the_first_press_while_the_run_goes_on(self):
        release = threading.Event()

        def live_play(chords, *, on_press, **kw):
            on_press(ROWS[0])
            release.wait(THREAD_CAP_S)
            return play_chart.RunResult(stop="done")

        t, _events, _seen = self._chain_rig(live_play)
        reply = t.tools.ch_run_chart(t.r.song, self._score_json(), self._schedule_json(), "System")
        self.assertIn("first press chord index 0", reply)
        self.assertTrue(t.tools._play.running())
        release.set()
        t.tools._play.thread.join(THREAD_CAP_S)

    def test_a_decline_reaches_the_chords_and_the_reply_names_them(self):
        t, _events, seen = self._chain_rig()
        score, schedule = self._decline_files(kept=(0,))
        reply = self._run(t, score, schedule)
        self.assertEqual([c.declined for c in seen["chords"]], [False, True])
        self.assertEqual([p.lanes for p in seen["chords"][1].presses], [[Lane.KICK]])
        self.assertIn("declined: chords 1 (kick)", reply.splitlines())

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

    def test_a_failed_attach_stops_there_and_tears_nothing_down(self):
        t, events, _seen = self._chain_rig(attaches=False)
        lines = self._run(t).splitlines()
        self.assertEqual(events, ["attach"])
        self.assertRegex(lines[0], r"^attach: FAILED after .* s; the run stopped at attach$")
        self.assertEqual(lines[1], "the game is not running")

    def test_a_failed_setup_stops_before_the_watch_and_the_reply_names_setup(self):
        t, events, _seen = self._chain_rig(press="none")
        lines = self._run(t).splitlines()
        self.assertEqual(events, ["attach", ("setup", "System")])
        self.assertRegex(lines[2], r"^kit setup: FAILED after .* s; the run stopped at kit setup$")
        self.assertEqual(lines[3], "setup reply")
        self.assertIsNone(t.tools._play)

    def test_a_failed_watch_tears_the_kit_down_and_starts_no_player(self):
        t, events, _seen = self._chain_rig(engine=None)
        lines = self._run(t).splitlines()
        self.assertEqual(events[-1], "teardown")
        self.assertNotIn("play", events)
        self.assertRegex(lines[3], r"^watch: FAILED after .* s; the run stopped at watch$")
        self.assertEqual(lines[4:], ["watcher ended: no song", "kit teardown: all checks ok"])
        self.assertIsNone(t.tools._play)

    def test_a_player_that_ends_with_no_press_tears_the_kit_down(self):
        def live_play(chords, **kw):
            return play_chart.RunResult(stop=play_chart.STOP_NO_ENGINE, message="no song")

        t, events, _seen = self._chain_rig(live_play)
        reply = self._run(t)
        self.assertEqual(events[-2], "teardown")
        self.assertRegex(reply, r"play: FAILED after .* s; the run stopped at play")
        self.assertIn("first press: none, no chord was pressed", reply)
        self.assertIn("no song", reply)

    def test_a_player_that_ends_with_no_press_stops_the_watch_too(self):
        def live_play(chords, **kw):
            return play_chart.RunResult(stop=play_chart.STOP_NO_ENGINE, message="no song")

        t, events, _seen = self._chain_rig(live_play)
        reply = self._run(t)
        self.assertEqual(events[-2:], ["teardown", "watch stop"])
        self.assertTrue(reply.endswith("watcher ended: stopped"), reply)

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

    def test_the_search_timeout_names_the_engines_it_searched(self):
        def times_out(proc, patterns, **kw):
            raise GT.engine_finder.EngineSearchTimeout("no playing song found in 120 s")

        with mock.patch.object(GT.engine_finder, "find_live_engine_capped", times_out), \
                mock.patch.object(GT.engine_finder, "normal_pattern", return_value=(b"", b"")):
            with self.assertRaises(GT.engine_finder.EngineSearchTimeout) as caught:
                GT.find_run_engine("a proc")
        self.assertIn("no playing song found in 120 s", str(caught.exception))
        self.assertIn("normal-mode engines only", str(caught.exception))

    def test_a_player_that_raises_is_named_and_tears_the_kit_down(self):
        def live_play(chords, **kw):
            raise ValueError("no ghost press is possible")

        t, events, _seen = self._chain_rig(live_play)
        reply = self._run(t)
        self.assertEqual(events[-2], "teardown")
        self.assertIn("ended: failed: ValueError: no ghost press is possible", reply)

    def test_the_chain_is_refused_while_a_play_session_runs(self):
        release = threading.Event()

        def live_play(chords, *, on_press, **kw):
            on_press(ROWS[0])
            release.wait(THREAD_CAP_S)
            return play_chart.RunResult(stop="done")

        t, events, _seen = self._chain_rig(live_play)
        t.tools.ch_run_chart(t.r.song, self._score_json(), self._schedule_json(), "System")
        events.clear()
        self.assertEqual(t.tools.ch_run_chart(t.r.song, self._score_json(), self._schedule_json(), "System"),
                         GT.PLAY_ALREADY_RUNNING)
        self.assertEqual(events, [])
        release.set()
        t.tools._play.thread.join(THREAD_CAP_S)

    def test_the_chain_makes_no_call_into_the_quiet_check_and_no_sampler_wait(self):
        t, _events, _seen = self._chain_rig()
        fake = FakeQuiet()
        with mock.patch.object(Q, "LIVE_ENV", fake.env()), \
                mock.patch.object(Q, "quiet_report") as check:
            self._run(t)
        check.assert_not_called()
        self.assertEqual(fake.calls, [])

    def test_the_tools_take_schedule_json_and_no_dump_json(self):
        # Question 6 (a) of docs/superpowers/plans/2026-11-08-replay-press-
        # schedule.md and D324 item 2: schedule_json replaces dump_json, and
        # no decline mode or lead is an argument. D318 item 4: ch_play_chart
        # and ch_run_chart take path_index.
        signatures = {name: list(inspect.signature(getattr(GT.GameTools, name)).parameters)
                      for name in ("ch_play_chart", "ch_run_chart", "ch_edge_probe")}
        self.assertEqual(signatures, {
            "ch_play_chart": ["self", "score_json", "schedule_json", "log", "song_folder",
                              "route", "path_index"],
            "ch_run_chart": ["self", "song_folder", "score_json", "schedule_json",
                             "kit_player", "path_index"],
            "ch_edge_probe": ["self", "song_folder", "score_json", "schedule_json",
                              "probes_json"]})

    # -- the right song and path (D318 item 4) --------------------------------------------

    def test_run_chart_refuses_when_the_loaded_folder_is_not_the_schedules_chart(self):
        other = os.path.join(self.tmp, "songs", "Another Song")
        t, events, _seen = self._chain_rig(loaded=other)
        lines = self._run(t).splitlines()
        # The attach ran; no kit was built, no watch started, nothing pressed.
        self.assertEqual(events, ["attach"])
        self.assertIsNone(t.tools._play)
        self.assertRegex(lines[1], r"^song check: FAILED after .* s; the run stopped at song check$")
        self.assertEqual(lines[2], f"the game has the song folder {json.dumps(other)} loaded, "
                                   f"not the schedule's chart folder {t.r.song}; nothing pressed")

    def test_run_chart_refuses_when_no_song_is_loaded_or_the_read_fails(self):
        t, events, _seen = self._chain_rig()
        for loaded, said in ((None, "the game has the song folder null loaded"),
                             (StateUnread("not attached"), "no read came back: not attached")):
            with self.subTest(loaded=loaded):
                t.tools._run_parts.frida.loaded = loaded
                events.clear()
                lines = self._run(t).splitlines()
                self.assertEqual(events, ["attach"])
                self.assertTrue(lines[2].startswith(said), lines[2])

    def test_run_chart_refuses_a_song_folder_that_is_not_the_schedules_before_the_attach(self):
        t, events, _seen = self._chain_rig()
        schedule = os.path.join(self.tmp, "x schedule.json")
        _write(schedule, json.dumps(for_song(SCHEDULE_JSON, os.path.join(self.tmp, "x"))).encode())
        reply = t.tools.ch_run_chart(t.r.song, self._score_json(), schedule, "System")
        self.assertEqual(reply, f"refused: song_folder {t.r.song} is not the folder of the "
                                f"schedule's chart, {os.path.join(self.tmp, 'x')}; nothing started")
        self.assertEqual(events, [])

    def test_run_chart_refuses_a_schedule_with_no_chart_or_a_relative_one(self):
        t, events, _seen = self._chain_rig()
        for chart, said in ((None, "names no chart"),
                            ("songs/x/notes.chart", "names its chart by the relative path")):
            with self.subTest(chart=chart):
                path = os.path.join(self.tmp, "chart schedule.json")
                doc = dict(SCHEDULE_JSON)
                if chart is not None:
                    doc["chart"] = chart
                _write(path, json.dumps(doc).encode())
                reply = t.tools.ch_run_chart(t.r.song, self._score_json(), path, "System")
                self.assertTrue(reply.startswith(f"refused: {path} {said}"), reply)
                self.assertEqual(events, [])

    def test_path_index_must_match_path_source(self):
        dump = PATH_SOURCE["file"]
        cases = (
            (dict(SCHEDULE_JSON, path_source={"file": dump, "index": 2}), 0,
             f"was made from path 2 of {dump}, but path_index is 0; nothing started"),
            ({k: v for k, v in SCHEDULE_JSON.items() if k != "path_source"}, 0,
             "has no path_source index, so the path it plays is unknown"),
            (SCHEDULE_JSON, 1, f"was made from path 0 of {dump}, but path_index is 1"))
        t, events, _seen = self._chain_rig()
        for doc, index, said in cases:
            with self.subTest(said=said):
                path = self._schedule_json(doc)
                reply = t.tools.ch_run_chart(t.r.song, self._score_json(), path, "System",
                                             path_index=index)
                self.assertTrue(reply.startswith(f"refused: {path} {said}"), reply)
                self.assertTrue(reply.endswith("; nothing started"), reply)
                self.assertEqual(events, [])
                reply = t.tools.ch_play_chart(self._score_json(), path, route="keys",
                                              path_index=index)
                self.assertTrue(reply.startswith(f"refused: {path} {said}"), reply)
                self.assertIsNone(t.tools._play)
        # A schedule made from path 2, asked for path 2, plays.
        path = self._schedule_json(dict(SCHEDULE_JSON, path_source={"file": dump, "index": 2}))
        reply = t.tools.ch_run_chart(t.r.song, self._score_json(), path, "System", path_index=2)
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertIn("first press chord index 0", reply)

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

    def test_with_no_parts_handed_in_it_refuses_and_touches_nothing(self):
        t = ToolRig(self.tmp)
        self.assertTrue(t.tools.ch_run_chart(t.r.song, self._score_json(), self._schedule_json()).startswith("refused:"))
        self.assertIsNone(t.tools._play)

    def test_foreground_line_is_the_watchs_line_or_none(self):
        t = ToolRig(self.tmp)
        self.assertIsNone(t.tools.foreground_line())
        t.tools.ch_start(t.r.song)
        self.assertEqual(t.tools.foreground_line(), "foreground went to the game 0 times so far")
        t.tools.ch_stop()


# A settings.ini with the run's line in its [game] section.
USER_SETTINGS = b"[audio]\r\nvolume = 1\r\n\r\n[game]\r\npause_on_focus_lost = 1\r\nnote_speed = 7\r\n"


class RunWithoutFocusTest(unittest.TestCase):
    """ch_start's settings line and foreground watch, and the registry key
    riding along with the files."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def test_ch_start_sets_the_line_after_the_backup_then_launches_then_watches(self):
        def set_setting(path, key, value):
            backups = [d for d in os.listdir(t.tools._scratch) if d.startswith(G.BACKUP_PREFIX)]
            t.events.append(("setting", path, key, value, len(backups)))

        t = ToolRig(self.tmp, set_setting=set_setting)
        real_launch = t.r.launch

        def launch(cmd):
            t.events.append("launch")
            return real_launch(cmd)

        t.r.env.launch = launch
        lines = t.tools.ch_start(t.r.song).splitlines()
        self.assertEqual(t.events, [
            ("setting", t.r.path(G.SETTINGS), "pause_on_focus_lost", "0", 1),
            "launch", "watch start"])
        said = "settings.ini: pause_on_focus_lost set to 0 for this run (restored by ch_stop)"
        self.assertIn(said, lines)
        self.assertLess(lines.index(f"backup folder: {t.backup_dir()}"), lines.index(said))
        starting = [i for i, line in enumerate(lines) if line.startswith("starting: ")]
        self.assertLess(lines.index(said), starting[0])

    def test_the_users_line_is_set_for_the_run_and_put_back_by_ch_stop(self):
        t = ToolRig(self.tmp)
        _write(t.r.path(G.SETTINGS), USER_SETTINGS)
        t.tools.ch_start(t.r.song)
        self.assertEqual(_read(t.r.path(G.SETTINGS)),
                         USER_SETTINGS.replace(b"pause_on_focus_lost = 1", b"pause_on_focus_lost = 0"))
        lines = t.tools.ch_stop().splitlines()
        self.assertIn("settings.ini: restored", lines)
        self.assertEqual(_read(t.r.path(G.SETTINGS)), USER_SETTINGS)

    def test_a_settings_file_without_the_line_is_left_alone_and_named(self):
        t = ToolRig(self.tmp)
        reply = t.tools.ch_start(t.r.song)
        self.assertIn("settings.ini: pause_on_focus_lost not set to 0 (no `pause_on_focus_lost` "
                      "line in the [game] section); the game may pause while it is not in front",
                      reply.splitlines())
        self.assertEqual(_read(t.r.path(G.SETTINGS)), ORIGINAL[G.SETTINGS])
        self.assertEqual(len(t.r.launch.commands), 1)

    def test_ch_status_says_the_count_so_far_and_ch_stop_the_final_count(self):
        t = ToolRig(self.tmp)
        self.assertNotIn("foreground", t.tools.ch_status())
        t.tools.ch_start(t.r.song)
        watch = t.watches[0]
        watch.count = 2
        self.assertIn("foreground went to the game 2 times so far", t.tools.ch_status().splitlines())
        lines = t.tools.ch_stop().splitlines()
        self.assertTrue(watch.stopped)
        self.assertIn("foreground went to the game 2 times", lines)
        self.assertLess(lines.index("foreground went to the game 2 times"),
                        lines.index("stopped the game"))
        self.assertNotIn("foreground", t.tools.ch_status())

    def test_an_adopted_game_starts_no_watch(self):
        t = ToolRig(self.tmp)
        t.tools.ensure_backup()
        self.assertEqual(t.watches, [])
        self.assertNotIn("foreground", t.tools.shutdown())

    def test_both_backups_carry_the_registry_key_with_the_envs_runner(self):
        for start in ("ch_start", "ensure_backup"):
            with self.subTest(start=start):
                t = ToolRig(os.path.join(self.tmp, start))
                t.r.env.run_reg = reg = FakeReg()
                getattr(t.tools, start)(*([t.r.song] if start == "ch_start" else []))
                self.assertTrue(os.path.isfile(os.path.join(t.backup_dir(), G.REGISTRY_BACKUP)))
                reg.text = b"the game changed this"
                self.assertIn("rewired registry: restored", t.tools.ch_stop().splitlines())


class HumanStartTest(unittest.TestCase):
    """ch_start(human=True), ch_status's loader line and ch_stop's detach,
    on test_bot_run's launch fakes."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name
        self.events = []

    def tearDown(self):
        self._tmp.cleanup()

    def rig(self, agent=None, folder=""):
        t = ToolRig(os.path.join(self.tmp, folder))
        agent = agent or FakeLaunchAgent(self.events)
        wire_human(t.r, agent)
        return t, agent

    def test_human_with_no_song_folder_is_refused_naming_the_reason(self):
        t, _agent = self.rig()
        reply = t.tools.ch_start(human=True)
        self.assertEqual(reply, B.HUMAN_NEEDS_SONG)
        self.assertIn("runs only with --song", reply)
        self.assertIsNone(t.backup_dir())
        self.assertEqual(self.events, [])
        self.assertEqual(t.r.launch.commands, [])

    def test_human_takes_the_backup_before_the_paused_start(self):
        t, agent = self.rig()
        seen = []
        fake_start = t.r.env.start_process

        def start(cmd):
            seen.append([d for d in os.listdir(t.tools._scratch) if d.startswith(G.BACKUP_PREFIX)])
            return fake_start(cmd)

        t.r.env.start_process = start
        lines = t.tools.ch_start(t.r.song, player="ProDrums,Expert", human=True).splitlines()
        self.assertEqual(seen, [[os.path.basename(t.backup_dir())]])
        self.assertEqual(self.events, ["start", "attach", "load", "arm", "resume", "status"])
        self.assertIn("game process id: 4242", lines)
        self.assertIs(t.tools._run.launch.agent, agent)
        self.assertEqual(t.watches[0].started, True)

    def test_ch_status_prints_the_loader_line_all_three_ways(self):
        cases = [(dict(loader_runs=1, bytes_at_first_run="BA 00 00 00 00"), "yes"),
                 (dict(loader_runs=1, bytes_at_first_run="BA 01 00 00 00"), "no"),
                 (dict(loader_runs=0), "not yet")]
        for fields, word in cases:
            with self.subTest(word=word):
                self.events.clear()
                later = {**PATCHED, **fields}
                t, _agent = self.rig(FakeLaunchAgent(self.events, statuses=[PATCHED, later]),
                                     folder=word)
                t.tools.ch_start(t.r.song, human=True)
                lines = t.tools.ch_status().splitlines()
                self.assertIn("player: human (suspended launch)", lines)
                self.assertIn(f"loader ran after patch: {word}", lines)
                self.assertTrue(any(line.startswith("launch patch: patched; at the patch "
                                                    "BA 01 00 00 00 before and BA 00 00 00 00 after")
                                    for line in lines), lines)
                t.tools.ch_stop()

    def test_ch_status_prints_each_of_the_four_states(self):
        cases = {
            "idle": ({"state": "idle"}, "the launch agent holds no arm, so it will never patch"),
            "waiting": ({"state": "waiting", "loader_runs": 0, "reason": None},
                        "loader ran after patch: not yet"),
            "patched": (PATCHED, "patched at: 1200 ms on the agent's own clock (Date.now)"),
            "refused": ({"state": "refused", "before": "BA 01 00 00 00", "after": None,
                         "loader_runs": 0, "reason": "patching GameAssembly.dll failed: boom"},
                        "launch agent reason: patching GameAssembly.dll failed: boom"),
        }
        for state, (later, want) in cases.items():
            with self.subTest(state=state):
                self.events.clear()
                t, _agent = self.rig(FakeLaunchAgent(self.events, statuses=[PATCHED, later]),
                                     folder="state-" + state)
                t.tools.ch_start(t.r.song, human=True)
                lines = t.tools.ch_status().splitlines()
                self.assertTrue(any(line.startswith(f"launch patch: {state};") for line in lines),
                                lines)
                self.assertIn(want, lines)
                t.tools.ch_stop()

    def test_ch_status_names_a_status_that_fails(self):
        t, agent = self.rig()
        t.tools.ch_start(t.r.song, human=True)

        def failing(message, cap_s):
            raise FA.ReadFailed(FA.END_DETACHED, "session detached: process-terminated")

        agent.request = failing
        self.assertIn("launch agent: status failed: session detached: process-terminated",
                      t.tools.ch_status().splitlines())

    def test_ch_stop_detaches_the_launch_agent_before_it_stops_the_game(self):
        t, agent = self.rig()
        t.tools.ch_start(t.r.song, human=True)
        agent.detach = lambda: (self.events.append(("detach", t.r.game.stopped)), "detached")[1]
        lines = t.tools.ch_stop().splitlines()
        self.assertIn(("detach", False), self.events)
        self.assertLess(lines.index("launch agent: detached"), lines.index("stopped the game"))
        self.assertTrue(t.r.game.stopped)
        t.r.assert_all_restored(self)

    def test_a_failed_human_launch_says_so_and_restores_the_files(self):
        t, _agent = self.rig(FakeLaunchAgent(self.events, statuses=[{"state": "refused"}]))
        lines = t.tools.ch_start(t.r.song, human=True).splitlines()
        self.assertTrue(lines[-1].startswith("the human launch failed: launch.ts refused the patch"),
                        lines)
        self.assertIn(f"{G.SETTINGS}: unchanged", lines)
        self.assertIsNone(t.tools._run)
        self.assertEqual(t.watches, [])
        t.r.assert_all_restored(self)


def next_worker(t: ToolRig, running: bool = False, **tool_args) -> GT.GameTools:
    """A second GameTools on the same temp root and user folders, the way a
    fresh worker comes up after `t`'s was killed: `t.tools` is simply never
    stopped. `running` is what the fake pid test answers for every pid. Both
    workers share this test's pid, so a later stamp keeps their backups apart."""
    t.r.env.stamp = lambda: "20261028-101501"
    return GT.GameTools(t.r.env, threading.RLock(), t.scratch_root, driver=t.driver,
                        find_window=lambda: t.hwnd, is_pid_running=lambda pid: running,
                        make_recorder=t.make_recorder, **tool_args)


class MarkerTest(unittest.TestCase):
    """The open-run marker: a killed worker's way back to the user's files."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name
        self.t = ToolRig(self.tmp)
        self.path = os.path.join(self.t.scratch_root, GT.OPEN_RUN_MARKER)

    def tearDown(self):
        self._tmp.cleanup()

    def assert_marker_names(self, folder):
        self.assertEqual(GT.read_marker(self.path), (os.getpid(), folder))

    def killed_run(self):
        """A run left open with every user file changed; returns its backup."""
        self.t.tools.ch_start(self.t.r.song)
        for name in ORIGINAL:
            _write(self.t.r.path(name), b"changed by the game")
        return self.t.backup_dir()

    # -- writing and deleting -----------------------------------------------------

    def test_the_marker_sits_under_the_scratch_root_and_never_the_real_temp_folder(self):
        self.assertEqual(self.t.tools._marker_path(), self.path)

    def test_ch_start_names_the_pid_and_folder_and_ch_stop_deletes_the_marker(self):
        self.t.tools.ch_start(self.t.r.song)
        self.assert_marker_names(self.t.backup_dir())
        self.t.tools.ch_stop()
        self.assertFalse(os.path.exists(self.path))

    def test_the_marker_is_there_before_the_settings_change_and_the_launch(self):
        seen = []

        def set_setting(path, key, value):
            seen.append(("setting", GT.read_marker(self.path)))

        t = self.t
        t.tools._set_setting = set_setting
        real_launch = t.r.launch
        t.r.env.launch = lambda cmd: seen.append(("launch", GT.read_marker(self.path))) or real_launch(cmd)
        t.tools.ch_start(t.r.song)
        said = (os.getpid(), t.backup_dir())
        self.assertEqual(seen, [("setting", said), ("launch", said)])

    def test_a_human_start_has_its_marker_before_the_game_starts(self):
        wire_human(self.t.r, FakeLaunchAgent([]))
        seen = []
        fake_start = self.t.r.env.start_process

        def start(cmd):
            seen.append(GT.read_marker(self.path))
            return fake_start(cmd)

        self.t.r.env.start_process = start
        self.t.tools.ch_start(self.t.r.song, human=True)
        self.assertEqual(seen, [(os.getpid(), self.t.backup_dir())])

    def test_ensure_backup_writes_the_marker_and_shutdown_deletes_it(self):
        self.t.tools.ensure_backup()
        self.assert_marker_names(self.t.backup_dir())
        self.t.tools.shutdown()
        self.assertFalse(os.path.exists(self.path))

    def test_a_failed_restore_keeps_the_marker_and_the_next_good_one_deletes_it(self):
        self.t.tools.ch_start(self.t.r.song)
        backup = self.t.backup_dir()

        def fail(backup_dir, out, leave=()):
            raise G.RestoreError("scoredata.bin", "locked")

        with mock.patch.object(G, "restore", fail):
            self.t.tools.ch_stop()
        self.assert_marker_names(backup)
        self.t.tools.ch_stop()
        self.assertFalse(os.path.exists(self.path))

    def test_a_stop_that_raises_keeps_the_marker(self):
        self.t.tools.ch_start(self.t.r.song)
        backup = self.t.backup_dir()
        with mock.patch.object(B, "finish_run", side_effect=RuntimeError("boom")):
            self.t.tools.ch_stop()
        self.assert_marker_names(backup)

    def test_a_second_ch_start_leaves_a_marker_naming_the_new_backup(self):
        self.t.tools.ch_start(self.t.r.song)
        first = self.t.backup_dir()
        self.t.tools.ch_start(self.t.r.song)
        self.assertNotEqual(self.t.backup_dir(), first)
        self.assert_marker_names(self.t.backup_dir())

    def test_a_marker_that_cannot_be_deleted_is_named_with_what_it_would_do(self):
        self.t.tools.ch_start(self.t.r.song)
        backup = self.t.backup_dir()
        with mock.patch.object(GT.os, "remove", side_effect=PermissionError("locked")):
            reply = self.t.tools.ch_stop()
        self.assertIn(f"open-run marker {self.path} was not deleted (locked); delete it by hand, "
                      f"or the next worker adopts {backup}, which is already restored",
                      reply.splitlines())

    def test_a_marker_that_cannot_be_written_is_said_and_the_run_goes_on(self):
        with mock.patch.object(GT.os, "rename", side_effect=PermissionError("locked")):
            reply = self.t.tools.ch_start(self.t.r.song)
        self.assertIn("open-run marker not written (PermissionError: locked)", reply)
        self.assertIn(B._restore_hint(self.t.backup_dir()), reply)
        self.assertEqual(os.listdir(self.t.scratch_root), [os.path.basename(self.t.tools._scratch)])
        self.assertEqual(len(self.t.r.launch.commands), 1)

    # -- a ch_start that fails ----------------------------------------------------

    def test_a_ch_start_that_fails_after_its_backup_leaves_no_marker(self):
        def boom(cmd):
            self.assertTrue(os.path.isfile(self.path))
            raise OSError("no such game")

        self.t.r.env.launch = boom
        lines = self.t.tools.ch_start(self.t.r.song).splitlines()
        self.assertEqual(lines[-1], "could not start the game: no such game")
        self.assertFalse(os.path.exists(self.path))
        self.assertIsNone(self.t.backup_dir())

    def test_a_failed_human_launch_leaves_no_marker(self):
        wire_human(self.t.r, FakeLaunchAgent([], statuses=[{"state": "refused"}]))
        self.t.tools.ch_start(self.t.r.song, human=True)
        self.assertFalse(os.path.exists(self.path))

    def test_a_song_with_no_length_never_takes_a_backup_or_a_marker(self):
        with mock.patch.object(B, "song_length_ms", side_effect=B.SongLengthMissing("no length")):
            self.assertEqual(self.t.tools.ch_start(self.t.r.song).splitlines()[-1], "no length")
        self.assertFalse(os.path.exists(self.path))

    def test_a_failed_start_whose_restore_fails_keeps_the_marker_for_ch_stop(self):
        def boom(cmd):
            _write(self.t.r.path(G.SETTINGS), b"changed before the launch failed")
            raise OSError("no such game")

        self.t.r.env.launch = boom

        def fail(backup_dir, out, leave=()):
            raise G.RestoreError("scoredata.bin", "locked")

        with mock.patch.object(G, "restore", fail):
            lines = self.t.tools.ch_start(self.t.r.song).splitlines()
        backup = self.t.backup_dir()
        self.assertIsNotNone(backup)
        self.assert_marker_names(backup)
        self.assertIn(f"the user files may not be back; ch_stop tries again from {backup}", lines)
        self.assertEqual(lines[-1], "could not start the game: no such game")
        self.t.tools.ch_stop()
        self.t.r.assert_all_restored(self)
        self.assertFalse(os.path.exists(self.path))

    # -- the next worker adopts ---------------------------------------------------

    KEPT = ("profiles.ini", "profiles.bin", "scoredata.bin", "scoresext.bin", "user.bin")

    def age_folder(self, folder, hours):
        then = time.time() - hours * 3600
        os.utime(folder, (then, then))

    def assert_partly_restored(self):
        """settings.ini and scorestats.json are back; the five others still
        hold what the game wrote."""
        for name, data in ORIGINAL.items():
            want = b"changed by the game" if name in self.KEPT else data
            self.assertEqual(_read(self.t.r.path(name)), want, name)

    def test_a_restore_from_a_marker_leaves_each_score_file_and_says_so(self):
        self.killed_run()
        lines = next_worker(self.t).ch_stop().splitlines()
        for name in self.KEPT:
            self.assertIn(f"{name}: left as it is, because the backup may be older than your "
                          "play", lines)
        self.assertEqual(sum("left as it is" in line for line in lines), 5)

    def test_a_restore_from_a_marker_keeps_a_score_file_changed_after_the_backup(self):
        self.t.tools.ch_start(self.t.r.song)
        _write(self.t.r.path("scoredata.bin"), b"the user played after the backup")
        next_worker(self.t).ch_stop()
        self.assertEqual(_read(self.t.r.path("scoredata.bin")), b"the user played after the backup")

    def test_a_live_runs_ch_stop_still_restores_every_file(self):
        self.t.tools.ch_start(self.t.r.song)
        for name in ORIGINAL:
            _write(self.t.r.path(name), b"changed by the game")
        lines = self.t.tools.ch_stop().splitlines()
        self.t.r.assert_all_restored(self)
        self.assertFalse(any("left as it is" in line for line in lines))

    def test_an_ordinary_adopted_backup_is_restored_whole_at_shutdown(self):
        self.t.tools.ensure_backup()
        for name in ORIGINAL:
            _write(self.t.r.path(name), b"changed by the game")
        self.t.tools.shutdown()
        self.t.r.assert_all_restored(self)
        self.assertFalse(os.path.exists(self.path))

    def test_the_workers_exit_leaves_a_marker_backup_and_its_marker(self):
        backup = self.killed_run()
        tools = next_worker(self.t)
        reply = tools.shutdown()
        self.assertIn(backup, reply)
        self.assertIn(self.path, reply)
        for name in ORIGINAL:
            self.assertEqual(_read(self.t.r.path(name)), b"changed by the game", name)
        self.assert_marker_names(backup)
        self.assertEqual(tools.open_lines(), [f"adopted backup: {backup}"])
        tools.ch_stop()
        self.assert_partly_restored()
        self.assertFalse(os.path.exists(self.path))

    def test_the_next_worker_adopts_and_ch_stop_restores_and_deletes_the_marker(self):
        backup = self.killed_run()
        self.age_folder(backup, 3)
        tools = next_worker(self.t)
        said = (f"a previous worker (pid {os.getpid()}) left a backup open: {backup}, 3 hours old; "
                "ch_stop restores settings.ini, scorestats.json and the Rewired key from it and "
                "leaves profiles.ini, profiles.bin, scoredata.bin, scoresext.bin, user.bin as "
                "they are")
        self.assertIn(said, tools.ch_status().splitlines())
        self.assertIn(said, self.t.r.lines)
        self.assertEqual(tools.open_lines(), [f"adopted backup: {backup}"])
        with mock.patch.object(B, "restore_only", wraps=B.restore_only) as restore_only:
            lines = tools.ch_stop().splitlines()
        self.assertEqual(restore_only.call_args.args[0], backup)
        self.assertIn(f"{G.SETTINGS}: restored", lines)
        self.assertIn(f"{G.SCORESTATS}: restored", lines)
        self.assert_partly_restored()
        self.assertFalse(self.t.r.game.stopped)
        self.assertFalse(os.path.exists(self.path))
        self.assertNotIn(said, tools.ch_status().splitlines())
        self.assertEqual(tools.open_lines(), [])

    def test_adopting_restores_nothing_until_ch_stop(self):
        self.killed_run()
        tools = next_worker(self.t)
        tools.ch_status()
        for name in ORIGINAL:
            self.assertEqual(_read(self.t.r.path(name)), b"changed by the game", name)

    def test_an_adopted_backup_puts_the_registry_key_back(self):
        self.t.r.env.run_reg = reg = FakeReg()
        self.t.tools.ch_start(self.t.r.song)
        reg.text = b"the game changed this"
        self.assertIn("rewired registry: restored", next_worker(self.t).ch_stop().splitlines())
        self.assertFalse(os.path.exists(self.path))

    def test_a_ch_start_on_a_marker_backup_refuses_naming_the_folder_and_its_age(self):
        backup = self.killed_run()
        self.age_folder(backup, 5)
        tools = next_worker(self.t, make_watch=lambda: FakeWatch([]))
        reply = tools.ch_start(self.t.r.song)
        self.assertEqual(reply, f"refused: a previous worker left a backup open: {backup}, "
                                "5 hours old; call ch_stop to restore it, then ch_start")
        self.assertIsNone(tools._run)
        for name in ORIGINAL:
            self.assertEqual(_read(self.t.r.path(name)), b"changed by the game", name)
        self.assert_marker_names(backup)
        tools.ch_stop()
        self.assertIn("song length", tools.ch_start(self.t.r.song))

    def test_an_adopted_restore_that_fails_keeps_the_marker_and_the_adoption(self):
        backup = self.killed_run()
        tools = next_worker(self.t)

        def fail(backup_dir, out, leave=()):
            raise G.RestoreError("scoredata.bin", "locked")

        with mock.patch.object(G, "restore", fail):
            lines = tools.ch_stop().splitlines()
        self.assertIn("restore failed on scoredata.bin: locked", lines)
        self.assert_marker_names(backup)
        self.assertEqual(tools._adopted_backup, backup)
        self.assertIn("refused", tools.ch_start(self.t.r.song))
        tools.ch_stop()
        self.assert_partly_restored()
        self.assertFalse(os.path.exists(self.path))

    def test_a_backup_that_vanishes_after_adoption_fails_the_stop_without_raising(self):
        backup = self.killed_run()
        tools = next_worker(self.t)
        os.remove(os.path.join(backup, G.BACKUP_MANIFEST))
        reply = tools.ch_stop()
        self.assertIn("restoring failed: FileNotFoundError", reply)
        self.assert_marker_names(backup)

    def test_a_folder_with_no_manifest_is_not_adopted_and_the_marker_stays(self):
        backup = self.killed_run()
        os.remove(os.path.join(backup, G.BACKUP_MANIFEST))
        tools = next_worker(self.t)
        self.assertIsNone(tools._adopted_backup)
        self.assertIn(f"holds no {G.BACKUP_MANIFEST}", tools.ch_status())
        self.assertIn("nothing to restore", tools.ch_stop())
        self.assert_marker_names(backup)

    # -- a marker whose pid still runs --------------------------------------------

    def test_a_running_pid_is_not_adopted_and_the_line_gives_the_by_hand_restore(self):
        backup = self.killed_run()
        tools = next_worker(self.t, running=True)
        self.assertIsNone(tools._adopted_backup)
        status = tools.ch_status()
        self.assertIn(f"process {os.getpid()} was running", status)
        self.assertIn(backup, status)
        self.assertIn(f'py tools/ch_mcp/bot_run.py --restore "{backup}"', status)
        self.assertIn(self.path, status)
        self.assertEqual(tools.open_lines(), [])

    def test_a_worker_that_adopted_nothing_never_deletes_anothers_marker(self):
        backup = self.killed_run()
        tools = next_worker(self.t, running=True)
        self.assertIn("nothing to restore", tools.ch_stop())
        self.assertIn("nothing to restore", tools.shutdown())
        self.assert_marker_names(backup)
        self.assertEqual(_read(self.t.r.path(G.SETTINGS)), b"changed by the game")

    def test_a_second_workers_run_leaves_the_first_workers_marker_alone(self):
        backup = self.killed_run()
        tools = next_worker(self.t, running=True, make_watch=lambda: FakeWatch([]))
        reply = tools.ch_start(self.t.r.song)
        self.assertIn("open-run marker not written (FileExistsError", reply)
        self.assert_marker_names(backup)
        tools.ch_stop()
        self.assert_marker_names(backup)
        self.assertEqual([n for n in os.listdir(self.t.scratch_root) if n.endswith(".tmp")], [])

    # -- a marker whose folder is gone --------------------------------------------

    def test_a_gone_folder_deletes_the_marker_and_ch_stop_has_nothing_to_restore(self):
        for running in (False, True):
            with self.subTest(running=running):
                gone = os.path.join(self.tmp, "gone")
                os.makedirs(self.t.scratch_root, exist_ok=True)
                _write(self.path, json.dumps({"pid": 77, "folder": gone}).encode())
                tools = next_worker(self.t, running=running)
                self.assertFalse(os.path.exists(self.path))
                self.assertIsNone(tools._adopted_backup)
                self.assertIn(f"a previous worker (pid 77) left a marker for a backup that is "
                              f"gone: {gone}; the marker was deleted",
                              tools.ch_status().splitlines())
                self.assertIn("nothing to restore", tools.ch_stop())

    # -- a marker that cannot be read ---------------------------------------------

    def test_an_unreadable_marker_starts_the_worker_and_stays_byte_for_byte(self):
        cases = {"garbage": b"{not json", "empty": b"", "not utf-8": b"\xff\xfe\x00",
                 "a list": b"[1, 2]", "no folder": b'{"pid": 5}', "no pid": b'{"folder": "x"}',
                 "a true pid": b'{"pid": true, "folder": "x"}',
                 "a text pid": b'{"pid": "5", "folder": "x"}',
                 "a zero pid": b'{"pid": 0, "folder": "x"}',
                 "an empty folder": b'{"pid": 5, "folder": ""}',
                 "a numeric folder": b'{"pid": 5, "folder": 9}',
                 "deep nesting": b"[" * 200000}
        os.makedirs(self.t.scratch_root, exist_ok=True)
        for name, data in cases.items():
            with self.subTest(case=name):
                _write(self.path, data)
                self.t.r.lines.clear()
                tools = next_worker(self.t)
                self.assertIsNone(tools._adopted_backup)
                said = [line for line in self.t.r.lines if self.path in line]
                self.assertEqual(len(said), 1, self.t.r.lines)
                self.assertIn("was left in place and nothing was adopted", said[0])
                self.assertIn(said[0], tools.ch_status().splitlines())
                tools.ch_stop()
                self.assertEqual(_read(self.path), data)

    def test_a_folder_at_the_markers_path_starts_the_worker_and_stays(self):
        os.makedirs(self.path)
        tools = next_worker(self.t)
        self.assertIn(self.path, tools.ch_status())
        tools.ch_stop()
        self.assertTrue(os.path.isdir(self.path))

    def test_an_unreadable_marker_is_never_replaced_by_a_new_run(self):
        os.makedirs(self.t.scratch_root, exist_ok=True)
        _write(self.path, b"{not json")
        tools = next_worker(self.t, make_watch=lambda: FakeWatch([]))
        self.assertIn("open-run marker not written", tools.ch_start(self.t.r.song))
        tools.ch_stop()
        self.assertEqual(_read(self.path), b"{not json")

    def test_a_pid_test_that_raises_leaves_the_marker(self):
        backup = self.killed_run()

        def broken(pid):
            raise OSError("no answer")

        tools = GT.GameTools(self.t.r.env, threading.RLock(), self.t.scratch_root,
                             driver=self.t.driver, is_pid_running=broken)
        self.assertIsNone(tools._adopted_backup)
        self.assertIn("was left in place and nothing was adopted: OSError: no answer",
                      tools.ch_status())
        self.assert_marker_names(backup)


class RecorderTest(unittest.TestCase):
    """How ch_start, ch_run_chart and the stops drive the machine recorder.
    test_recorder_link.py covers the link itself."""

    ENGINE = 0x2_0000_1000

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)
        self.t = ToolRig(self._tmp.name)
        self.popen = self.t.popen

    def folder(self, n: int = 1) -> str:
        return os.path.join(self.t.tools._scratch, f"machine-{n}")

    def running_line(self, pid: int = 777) -> str:
        return f"machine recorder: running, pid {pid}"

    def stdin_lines(self) -> list:
        return [e[1] for e in self.popen.events if isinstance(e, tuple) and e[0] == "stdin"]

    def chain(self, events: list) -> GT.GameTools:
        """Set up ch_run_chart's fakes, with the parts' calls going into `events`."""
        def played(chords, *, on_press, **_kw):
            events.append("play")
            on_press(ROWS[0])
            return play_chart.RunResult(stop="done")

        self.t.tools._live_play = played
        self.t.tools.use_run_parts(GT.RunParts(
            GameToolsTest.ChainFrida(events, loaded=self.t.r.song),
            GameToolsTest.ChainKit(events, lambda lanes, axes=None: {}),
            GameToolsTest.ChainSp(events, self.ENGINE)))
        return self.t.tools

    def run_chart(self) -> str:
        score = os.path.join(self._tmp.name, "score.json")
        with open(score, "w", encoding="utf-8") as f:
            json.dump(SCORE_JSON, f)
        schedule = os.path.join(self._tmp.name, "schedule.json")
        with open(schedule, "w", encoding="utf-8") as f:
            json.dump(for_song(SCHEDULE_JSON, self.t.r.song), f)
        reply = self.t.tools.ch_run_chart(self.t.r.song, score, schedule, "System")
        self.t.tools._play.thread.join(THREAD_CAP_S)
        return reply

    # -- ch_start -------------------------------------------------------------------

    def test_ch_start_runs_the_check_the_folder_the_recorder_the_launch_then_the_pid(self):
        t = self.t
        events = self.popen.events
        quiet = FakeQuiet()
        quiet.calls = events
        real_scratch, real_start = t.tools._new_scratch, B.start_run
        began = t.r.clock.t

        def new_scratch():
            events.append("scratch")
            return real_scratch()

        def start_run(*args, **kwargs):
            events.append("start_run")
            self.assertEqual(t.r.clock.t, began)
            return real_start(*args, **kwargs)

        t.tools._new_scratch = new_scratch
        with mock.patch.object(Q, "LIVE_ENV", quiet.env()), \
                mock.patch.object(B, "start_run", start_run), \
                mock.patch("time.sleep") as sleep:
            t.tools.ch_start(t.r.song)
        sleep.assert_not_called()
        self.assertEqual([e for e in events if e not in ("processes", "scan", "memory", "power")],
                         [("sample", Q.QUIET_SAMPLE_S), "scratch", "recorder start", "start_run",
                          ("stdin", b"pid 4242\n")])

    def test_a_machine_that_is_not_quiet_starts_no_recorder(self):
        with mock.patch.object(Q, "LIVE_ENV", FakeQuiet(cpu=Q.QUIET_CPU_PCT + 1).env()):
            self.t.tools.ch_start(self.t.r.song)
        self.assertEqual(self.popen.calls, [])

    def test_the_reply_names_the_recorder_and_the_worker_and_its_parent(self):
        lines = self.t.tools.ch_start(self.t.r.song).splitlines()
        self.assertIn(f"machine recorder: started, pid 777, folder {self.folder()}", lines)
        self.assertIn(f"worker pid {os.getpid()}, its parent pid {os.getppid()}", lines)

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

    def test_a_recorder_that_fails_to_start_is_one_line_and_the_run_opens(self):
        self.popen.error = OSError("no python")
        said = R.start_failed_line(self.popen.error)
        lines = self.t.tools.ch_start(self.t.r.song).splitlines()
        self.assertEqual(lines.count(said), 1)
        self.assertIsNotNone(self.t.tools._run)
        self.assertEqual(len(self.t.tools.open_lines()), 1)
        self.assertIn(said, self.t.tools.ch_status().splitlines())

    def test_a_fake_recorder_whose_start_raises_still_lets_the_run_open(self):
        class Broken(R.RecorderLink):
            def start(self, *_args):
                raise RuntimeError("broken")

        self.t = ToolRig(self._tmp.name + "-broken", make_recorder=Broken)
        reply = self.t.tools.ch_start(self.t.r.song)
        self.assertIn("machine recorder: not started (RuntimeError: broken)", reply)
        self.assertIsNotNone(self.t.tools._run)

    def test_a_pid_line_that_cannot_be_written_is_one_line_and_the_run_opens(self):
        def failing(*args, **kwargs):
            proc = FakePopen.__call__(self.popen, *args, **kwargs)
            proc.stdin.fail = True
            return proc

        self.t.tools._recorder = R.RecorderLink(popen=failing)
        reply = self.t.tools.ch_start(self.t.r.song)
        self.assertIn('machine recorder: "pid 4242" not sent (OSError: the pipe is closed)', reply)
        self.assertIsNotNone(self.t.tools._run)

    def test_a_launch_that_fails_stops_the_recorder_before_ch_start_returns(self):
        for error in (B.SongLengthMissing("no length"), B.LaunchFailed("refused"),
                      OSError("no exe")):
            with self.subTest(error=error):
                before = len(self.popen.procs)
                with mock.patch.object(B, "start_run", side_effect=error):
                    reply = self.t.tools.ch_start(self.t.r.song)
                proc = self.popen.procs[before]
                self.assertTrue(proc.stdin.closed.is_set())
                self.assertEqual(proc.returncode, 0)
                self.assertIn(f"machine recorder: stopped, pid {proc.pid}", reply)
                self.assertEqual(self.t.tools.open_lines(), [])

    def test_a_second_ch_start_ends_the_first_recorder(self):
        self.t.tools.ch_start(self.t.r.song)
        self.t.tools.ch_start(self.t.r.song)
        first, second = self.popen.procs
        self.assertEqual(first.returncode, 0)
        self.assertIsNone(second.returncode)

    # -- ch_run_chart ---------------------------------------------------------------

    def test_the_engine_goes_out_after_the_watch_and_before_the_play(self):
        events = self.popen.events
        self.chain(events)
        self.t.tools.ch_start(self.t.r.song)
        self.run_chart()
        steps = [e[0] if isinstance(e, tuple) and e[0] != "stdin" else e for e in events]
        engine = ("stdin", b"engine 0x200001000\n")
        self.assertLess(steps.index("watch"), steps.index(engine))
        self.assertLess(steps.index(engine), steps.index("play"))

    def test_the_reply_holds_the_samples_line_after_the_watch_line(self):
        self.chain([])
        self.t.tools.ch_start(self.t.r.song)
        with open(os.path.join(self.folder(), MR.SAMPLES_FILE), "wb") as f:
            f.write(b"sample,wall_s\n0,1000.25\n1,1000.27\n")
        lines = self.run_chart().splitlines()
        at = lines.index("machine recorder: 2 samples so far, last at 1000.27")
        self.assertTrue(lines[at - 1].startswith("watch: ok"), lines)

    def test_a_recorder_that_wrote_nothing_is_a_line_and_the_play_goes_on(self):
        events = []
        self.chain(events)
        self.t.tools.ch_start(self.t.r.song)
        reply = self.run_chart()
        self.assertIn("machine recorder: no samples yet in", reply)
        self.assertIn("play", events)

    def test_a_recorder_that_never_started_is_a_line_and_the_play_goes_on(self):
        events = []
        self.chain(events)
        self.popen.error = OSError("no python")
        self.t.tools.ch_start(self.t.r.song)
        lines = self.run_chart().splitlines()
        self.assertIn(R.start_failed_line(self.popen.error), lines)
        self.assertIn("play", events)

    def test_a_failed_engine_write_is_one_line_and_the_play_still_starts(self):
        events = []
        self.chain(events)
        self.t.tools.ch_start(self.t.r.song)
        self.popen.procs[0].stdin.fail = True
        reply = self.run_chart()
        self.assertIn('machine recorder: "engine 0x200001000" not sent (OSError: the pipe is closed)',
                      reply)
        self.assertIn("play", events)
        self.assertIn(GT.first_press_line(ROWS), reply)

    # -- the stops ------------------------------------------------------------------

    def test_ch_stop_and_shutdown_close_stdin_and_wait_the_cap(self):
        for stop in ("ch_stop", "shutdown"):
            with self.subTest(stop=stop):
                self.t.tools.ch_start(self.t.r.song)
                proc = self.popen.procs[-1]
                folder = self.folder(len(self.popen.procs))
                self.popen.events.clear()
                lines = getattr(self.t.tools, stop)().splitlines()
                self.assertTrue(proc.stdin.closed.is_set())
                self.assertIn(("wait", R.RECORDER_EXIT_CAP_S), self.popen.events)
                self.assertNotIn("terminate", self.popen.events)
                self.assertEqual(lines[0], f"machine recorder: stopped, pid {proc.pid}, "
                                           f"exit code 0, folder {folder}")

    def test_ch_stop_and_shutdown_terminate_one_that_does_not_exit_and_say_so(self):
        self.popen.hangs = True
        for stop in ("ch_stop", "shutdown"):
            with self.subTest(stop=stop):
                self.t.tools.ch_start(self.t.r.song)
                proc = self.popen.procs[-1]
                self.popen.events.clear()
                reply = getattr(self.t.tools, stop)()
                self.assertIn(("wait", R.RECORDER_EXIT_CAP_S), self.popen.events)
                self.assertEqual(self.popen.events.count("terminate"), 1)
                self.assertIn(f"machine recorder: pid {proc.pid} did not exit within "
                              f"RECORDER_EXIT_CAP_S ({R.RECORDER_EXIT_CAP_S} s) of its stdin "
                              "closing; terminated", reply)

    def test_the_stop_ends_the_recorder_before_the_hooks_run(self):
        self.t.tools.ch_start(self.t.r.song)
        self.t.tools.add_stop_hook(lambda: self.popen.events.append("hook"))
        self.t.tools.ch_stop()
        self.assertLess(self.popen.events.index("stdin closed"), self.popen.events.index("hook"))

    def test_a_stop_that_raises_still_restores_the_files(self):
        self.t.tools.ch_start(self.t.r.song)

        def broken():
            raise RuntimeError("broken")

        self.t.tools._recorder.stop = broken
        reply = self.t.tools.ch_stop()
        self.assertIn("stopping the machine recorder failed: RuntimeError: broken", reply)
        self.t.r.assert_all_restored(self)

    def test_open_lines_list_a_running_recorder_and_not_a_stopped_one(self):
        self.t.tools.ch_start(self.t.r.song)
        self.assertIn(self.running_line(), self.t.tools.open_lines())
        self.t.tools.ch_stop()
        self.assertEqual(self.t.tools.open_lines(), [])

    def test_ch_status_prints_the_recorders_line(self):
        self.assertIn("machine recorder: none", self.t.tools.ch_status().splitlines())
        self.t.tools.ch_start(self.t.r.song)
        self.assertIn(self.running_line(), self.t.tools.ch_status().splitlines())

    def test_one_that_exited_by_itself_leaves_open_lines_and_is_said_once_more(self):
        self.t.tools.ch_start(self.t.r.song)
        folder = self.folder()
        self.popen.procs[0].returncode = 3
        said = (f"machine recorder: exited by itself, pid 777, exit code 3, folder {folder}, "
                f"log {os.path.join(folder, R.LOG_NAME)}")
        self.assertNotIn(said, self.t.tools.open_lines())
        self.assertEqual(len(self.t.tools.open_lines()), 1)
        self.assertIn(said, self.t.tools.ch_status().splitlines())
        self.assertIn(said, self.t.tools.ch_stop().splitlines())
        self.assertIsNone(self.t.tools._recorder.stop())
        self.assertNotIn("terminate", self.popen.events)

    def test_a_stop_with_no_recorder_adds_no_line(self):
        self.assertEqual(self.t.tools.ch_stop(),
                         "no run is open and no backup was taken; nothing to restore")


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

    # -- the activation lead line -------------------------------------------------------

    # -- the anchor line -----------------------------------------------------------------

    def _squeeze(self, **fields):
        # A hand-built timed squeeze of the schedule's shape; the numbers are
        # test inputs.
        return {"chord": 931, "kind": "SqOut", "needs_no_timing": False, "feasible": True,
                "anchor_chord": 820, "anchor_aim_ms": -58.82, "planned_end_ms": 186348.9,
                "ratio": {"early": 1.0045, "late": 1.0045},
                "anchor_room_ms": 20.04, "squeeze_room_ms": 20.04, **fields}

    def test_the_anchor_line_reads_every_number_off_the_squeeze(self):
        doc = {"activations": [{"squeezes": [self._squeeze()]}]}
        self.assertEqual(GT.anchor_lines(doc), [
            "chord 820 pressed 58.8 ms early for the squeeze-out at chord 931 "
            "(end planned 186,348.9 ms, ratio 1.0045, room 20.0 ms both presses)"])

    def test_a_late_anchor_names_the_squeeze_in_and_each_ratio_and_room(self):
        doc = {"activations": [{"squeezes": [self._squeeze(
            chord=77, kind="SqIn", anchor_chord=70, anchor_aim_ms=33.4, planned_end_ms=4952.8,
            ratio={"early": 0.99, "late": 1.0}, anchor_room_ms=40.0, squeeze_room_ms=12.5)]}]}
        self.assertEqual(GT.anchor_lines(doc), [
            "chord 70 pressed 33.4 ms late for the squeeze-in at chord 77 "
            "(end planned 4,952.8 ms, ratio 0.9900 early, 1.0000 late, "
            "anchor room 40.0 ms, squeeze room 12.5 ms)"])

    def test_a_squeeze_that_needs_no_timing_has_no_anchor_line(self):
        doc = {"activations": [{"squeezes": [{"chord": 717, "kind": "SqIn",
                                              "needs_no_timing": True, "feasible": True}]}]}
        self.assertEqual(GT.anchor_lines(doc), [])

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

    def test_the_end_line_sets_the_games_end_against_the_planned_one(self):
        rows = [{**ROWS[0], "index": 820, "end_read_ms": 186352.1},
                {**ROWS[1], "index": 820, "end_read_ms": 186352.1},
                {**ROWS[1], "index": 900, "end_read_ms": ""},
                {**ROWS[1], "index": 1000, "end_read_ms": "no activation read"}]
        self.assertEqual(GT.activation_end_lines(rows, {820: 186349.0, 1000: 200000.0}), [
            "end read after chord 820: end planned 186,349.0 ms, game 186,352.1 ms (3.1 later)",
            "end read after chord 1000: end planned 200,000.0 ms, game end not read "
            "(no activation read)"])

    def test_rows_without_the_new_columns_give_no_lines(self):
        self.assertEqual(GT.activation_end_lines(ROWS, {0: 1000.0}), [])
        self.assertEqual(GT.scored_between_lines(ROWS), [])

    def test_points_between_presses_go_to_the_declined_chord_before_them(self):
        # Bomber's chords 854 (declined) and 855, from press-2.csv of the
        # ls-r2ab-run1 run as the plan reads it, with the column T3 adds.
        rows = [{"index": 853, "send": play_chart.SENT, "scored_between": 0},
                {"index": 854, "send": play_chart.DECLINED, "scored_between": 0},
                {"index": 855, "send": play_chart.SENT, "scored_between": 200},
                {"index": 856, "send": play_chart.SENT, "scored_between": "50"}]
        self.assertEqual(GT.scored_between_lines(rows), [
            "scored between presses: chord 854 granted 200 before chord 855, "
            "50 unexplained before chord 856"])

    def test_a_split_declined_chord_waits_once(self):
        rows = [{"index": 801, "send": play_chart.DECLINED},
                {"index": 801, "send": play_chart.DECLINED, "scored_between": 0},
                {"index": 802, "send": play_chart.SENT, "scored_between": 200},
                {"index": 803, "send": play_chart.SENT, "scored_between": 200}]
        self.assertEqual(GT.scored_between_lines(rows), [
            "scored between presses: chord 801 granted 200 before chord 802, "
            "200 unexplained before chord 803"])

    def test_the_play_status_carries_the_end_and_between_lines(self):
        rows = [{**ROWS[0], "end_read_ms": 1003.1, "scored_between": 0},
                {**ROWS[1], "scored_between": 40}]

        def live_play(chords, *, on_press, **kw):
            for row in rows:
                on_press(row)
            return play_chart.RunResult(stop="done", sent=2)

        t = ToolRig(self.tmp, live_play=live_play)
        # The game's end is read after chord 0, planned at a hand-picked 1000.
        schedule = schedule_of(SCORE_JSON)
        schedule["chords"][0]["reads"] = [{"activation": 0, "planned_end_ms": 1000.0,
                                           "stored_end_ms": 1000.0}]
        t.tools.ch_play_chart(self._json("score.json", SCORE_JSON),
                              self._json("schedule.json", schedule), route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        lines = t.tools.ch_play_status().splitlines()
        self.assertIn("end read after chord 0: end planned 1,000.0 ms, game 1,003.1 ms (3.1 later)",
                      lines)
        self.assertIn("scored between presses: 40 unexplained before chord 1", lines)

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

        t = ToolRig(self.tmp, live_play=live_play)
        if kit:
            t.tools.use_kit_press(lambda: (lambda lanes, axes=None: {}))
        return t, seen

    def _probe(self, t, probes):
        reply = t.tools.ch_edge_probe(t.r.song, self._json("score.json", SCORE_JSON),
                                      self._json("schedule.json", SCHEDULE_JSON),
                                      self._json("probes.json", probes))
        if t.tools._play is not None:
            t.tools._play.thread.join(THREAD_CAP_S)
        return reply

    def test_the_probe_presses_its_chord_at_its_lead_and_marks_only_that_row(self):
        t, seen = self._probe_rig()
        reply = self._probe(t, [{"index": 1, "lead_ms": -60}])
        self.assertIn("probes: chord 1 60 ms late", reply.splitlines())
        self.assertIn("press", seen)
        rows = {row["index"]: row for row in t.tools._play.rows}
        self.assertEqual(rows[1]["chart_ms"], 1500.0)
        self.assertAlmostEqual(rows[1]["chart_ms"] - rows[1]["clock_ms_at_press"], -60.0)
        self.assertEqual(rows[1]["probe"], -60.0)
        self.assertNotIn("probe", rows[0])
        # SCHEDULE_JSON's hand-picked aim of 2 ms early (_schedule_of).
        self.assertAlmostEqual(rows[0]["chart_ms"] - rows[0]["clock_ms_at_press"], 2.0)
        self.assertIn("probe chord 1: asked 60.00 ms late, pressed 60.00 ms late, score step 50",
                      t.tools.ch_play_status().splitlines())

    def test_the_probe_writes_a_press_log_in_the_session_folder(self):
        t, _seen = self._probe_rig()
        reply = self._probe(t, [{"index": 0, "lead_ms": 80}])
        log = os.path.join(t.tools.session_dir(), "press-1.csv")
        self.assertEqual(t.tools._play.log_path, log)
        self.assertIn(f"press log: {log}", [line.split(" (")[0] for line in reply.splitlines()])

    def test_a_probe_chord_not_in_the_file_is_refused_and_nothing_runs(self):
        t, seen = self._probe_rig()
        reply = self._probe(t, [{"index": 7, "lead_ms": 80}])
        self.assertEqual(reply, "refused: ValueError: probe chords 7 are not in the score file")
        self.assertIsNone(t.tools._play)
        self.assertEqual(seen, {})

    def test_the_probe_needs_the_kit(self):
        t, _seen = self._probe_rig(kit=False)
        self.assertEqual(self._probe(t, [{"index": 1, "lead_ms": 80}]),
                         "refused: the kit is not set up; call ch_kit_setup first")
        self.assertIsNone(t.tools._play)

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
        rows = [{**ROWS[0], "index": 152},
                {**ROWS[1], "index": 153, "score_after": 50, "send": play_chart.DROPPED}]
        lines = self._status(rows, play_chart.RunResult(
            stop="done", sent=1, dropped=1, rows=list(rows)))
        self.assertIn("sent 1", lines)
        self.assertFalse([line for line in lines if "jumped back" in line])
        self.assertIn("dropped: chords 153", lines)
        # The dropped line comes right after the counts.
        self.assertEqual(lines[lines.index("sent 1") + 1],
                         "dropped: chords 153")

    def test_a_result_with_no_rows_of_its_own_counts_the_drop_as_logged(self):
        rows = [{**ROWS[0], "index": 152},
                {**ROWS[1], "index": 153, "send": play_chart.DROPPED}]
        lines = self._status(rows, play_chart.RunResult(
            stop="done", sent=1, dropped=1))
        self.assertFalse([line for line in lines if "jumped back" in line])

    def test_a_split_chord_dropped_is_named_once(self):
        rows = [{**ROWS[0], "index": 152},
                {**ROWS[1], "index": 153, "send": play_chart.DROPPED},
                {**ROWS[1], "index": 153, "send": play_chart.DROPPED}]
        lines = self._status(rows, play_chart.RunResult(
            stop="done", sent=1, dropped=1, rows=list(rows)))
        self.assertIn("sent 1", lines)
        self.assertIn("dropped: chords 153", lines)

    def test_no_dropped_row_says_none(self):
        lines = self._status(list(ROWS), play_chart.RunResult(
            stop="done", sent=2, rows=list(ROWS)))
        self.assertIn("dropped: none", lines)

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

    def test_a_log_write_that_blocks_900_ms_drops_no_chord(self):
        result = self._play()
        self.assertEqual(result.stop, play_chart.STOP_DONE)
        self.assertEqual(result.dropped, 0)
        self.assertEqual([r["send"] for r in result.rows],
                         [play_chart.SENT] * len(SCORE_JSON["chords"]))

    def test_the_same_block_on_the_player_thread_still_drops(self):
        result = self._play(wire_into_on_press=True)
        self.assertEqual(result.stop, play_chart.STOP_DONE)
        self.assertGreater(result.dropped, 0)


if __name__ == "__main__":
    unittest.main()
