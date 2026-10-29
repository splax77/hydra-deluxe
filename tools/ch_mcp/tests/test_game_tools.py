"""Tests for game_tools, the Clone Hero server's game-side tools.

No game, no keys and no real user files: the launch, the clock and the game
are test_bot_run's fakes, the user folders are temp folders, the input driver
records its keys, and live_play is a fake that delivers rows.
"""

from __future__ import annotations

import json
import os
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
from tools.ch_mcp import game_files as G  # noqa: E402
from tools.ch_mcp import game_tools as GT  # noqa: E402
from tools.ch_mcp import install_chart as I  # noqa: E402
from tools.ch_mcp.tests.test_bot_run import ORIGINAL, Rig, StuckGame, _read, _write  # noqa: E402
from tools.ch_probe import probe_songs  # noqa: E402
from tools.ch_probe.experiments import play_chart  # noqa: E402
from tools.ch_probe.input_driver import DEFAULT_BINDINGS, InputDriver, Lane  # noqa: E402

WINDOW = 0x1234
# How long a test waits on a fake's thread before it fails instead of hanging.
THREAD_CAP_S = 5.0

ROWS = [
    {"index": 0, "chart_ms": 1000.0, "clock_ms_at_press": 998.0,
     "score_before": 0, "score_after": 50, "hit": 1},
    {"index": 1, "chart_ms": 1500.0, "clock_ms_at_press": 1499.0,
     "score_before": 50, "score_after": 50, "hit": 0},
]
SCORE_JSON = {"chords": [
    {"index": 0, "ms": 1000.0, "notes": [{"color": "Red", "cymbal": False}]},
    {"index": 1, "ms": 1500.0, "notes": [{"color": "Kick", "cymbal": False}]},
]}


class RecordingDriver(InputDriver):
    def __init__(self):
        super().__init__()
        self.events = []

    def send_key(self, vk: int, key_up: bool) -> None:
        self.events.append((vk, key_up))


class FakeLivePlay:
    """Delivers ROWS through on_press, then runs until should_stop says so
    (or THREAD_CAP_S passes) and returns `result`."""

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


class ToolRig:
    """A GameTools wired to test_bot_run's Rig and the fakes above."""

    def __init__(self, tmp: str, **tool_args):
        self.r = Rig(tmp, exits_at=None)
        self.scratch_root = os.path.join(tmp, "scratch")
        self.driver = RecordingDriver()
        self.hwnd = WINDOW
        self.live_play = FakeLivePlay(play_chart.RunResult(
            sent=2, hit=1, max_streak=1, rows=list(ROWS), stop="stopped"))
        args = dict(driver=self.driver, live_play=self.live_play,
                    find_window=lambda: self.hwnd)
        args.update(tool_args)
        self.tools = GT.GameTools(self.r.env, threading.RLock(), self.scratch_root, **args)

    def backup_dir(self) -> str:
        run = self.tools._run
        return run.backup_dir if run is not None else self.tools._adopted_backup


class GameToolsTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

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
        reply = t.tools.ch_start(t.r.song)
        backup = t.backup_dir()
        self.assertEqual(seen_at_launch, [[os.path.basename(backup)]])
        self.assertTrue(backup.startswith(t.scratch_root))
        lines = reply.splitlines()
        self.assertIn(f"backup folder: {backup}", lines)
        self.assertTrue(any(line.startswith("starting: ") and t.r.song in line for line in lines))
        self.assertIn("game process id: 4242", lines)
        self.assertEqual(lines[-1], f"song length: 4000 ms, from song_length in "
                                    f"{os.path.join(t.r.song, G.SONG_INI)}")
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

        def fail(backup_dir, out):
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
        _write(t.r.path("user.bin"), b"changed by the game")
        reply = t.tools.shutdown()
        self.assertEqual(t.r.launch.commands, [])
        self.assertFalse(t.r.game.stopped)
        self.assertIn("user.bin: restored", reply)
        self.assertIn("the server did not start the game, so it does not close it", reply)
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
        self.assertEqual(os.listdir(t.scratch_root), [os.path.basename(t.tools._scratch)])

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

    def test_ch_compare_prints_compares_lines_then_the_verdict(self):
        stats = os.path.join(self.tmp, "stats.json")
        score = os.path.join(self.tmp, "score.json")
        _write(stats, json.dumps({"players": [{"total_notes": 2, "sp_phrases_total": 0,
                                               "base_score": 100, "note_score": 100}]}).encode())
        _write(score, json.dumps({
            "chords": [{"notes": [{}], "is_sp_phrase_end": False}] * 2,
            "final": {"notes": 100, "accent": 0, "ghost": 0, "total": 100}}).encode())
        expected = []
        self.assertTrue(compare_scorestats.compare(stats, score, out=expected.append))
        t = ToolRig(self.tmp)
        self.assertEqual(t.tools.ch_compare(stats, score).splitlines(), expected + ["pass"])

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

    # -- the auto-player ------------------------------------------------------------------

    def _score_json(self) -> str:
        path = os.path.join(self.tmp, "score.json")
        _write(path, json.dumps(SCORE_JSON).encode())
        return path

    def test_a_second_ch_play_chart_is_refused_while_one_runs(self):
        t = ToolRig(self.tmp)
        score = self._score_json()
        first = t.tools.ch_play_chart(score)
        self.assertIn("play session started: 2 chords", first)
        self.assertIn("waiting for a moving clock (cap 120 s)", first)
        self.assertTrue(t.live_play.delivered.wait(THREAD_CAP_S))
        self.assertEqual(t.tools.ch_play_chart(score),
                         "a play session is already running; call ch_play_stop first")
        self.assertEqual(len(t.live_play.calls), 1)
        t.tools.ch_play_stop()
        t.tools._play.thread.join(THREAD_CAP_S)

    def test_ch_play_stop_sets_the_flag_and_ch_play_status_reports_the_result(self):
        t = ToolRig(self.tmp)
        t.tools.ch_play_chart(self._score_json(), log="press.csv")
        self.assertTrue(t.live_play.delivered.wait(THREAD_CAP_S))
        snapshot = t.tools.play_snapshot()
        self.assertEqual(snapshot, {"last_row": ROWS[-1], "press_count": 2, "stop": None})
        self.assertIn("running: 2 presses so far, 1 hit", t.tools.ch_play_status())

        stop_reply = t.tools.ch_play_stop()
        self.assertTrue(t.tools._play.stop.is_set())
        self.assertEqual(stop_reply.splitlines()[-1], "stopped")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertFalse(t.tools._play.running())

        lines = t.tools.ch_play_status().splitlines()
        self.assertIn("ended: stopped", lines)
        self.assertIn("sent 2, hit 1, misses 1, best streak 1", lines)
        log = os.path.join(t.tools._scratch, "press.csv")
        self.assertIn(f"press log: {log} (2 rows)", lines)
        self.assertEqual(lines[-2:], ["  1, 1500.0, 1499.0, 50, 50, 0",
                                      "  0, 1000.0, 998.0, 0, 50, 1"])
        self.assertTrue(os.path.isfile(log))
        self.assertEqual(t.tools.play_snapshot(),
                         {"last_row": ROWS[-1], "press_count": 2, "stop": "stopped"})

    def test_play_snapshot_is_none_with_no_play_session(self):
        t = ToolRig(self.tmp)
        self.assertIsNone(t.tools.play_snapshot())
        self.assertEqual(t.tools.ch_play_status(), "no play session has run")

    def test_a_run_with_no_engine_reports_the_finders_message(self):
        t = ToolRig(self.tmp, live_play=lambda chords, **kw: play_chart.RunResult(
            stop=play_chart.STOP_NO_ENGINE, message="no song started within 120 s"))
        t.tools.ch_play_chart(self._score_json())
        t.tools._play.thread.join(THREAD_CAP_S)
        lines = t.tools.ch_play_status().splitlines()
        self.assertIn(f"ended: {play_chart.STOP_NO_ENGINE}", lines)
        self.assertIn("no song started within 120 s", lines)

    def test_ch_stop_tells_a_running_play_session_to_stop(self):
        t = ToolRig(self.tmp)
        t.tools.ch_play_chart(self._score_json())
        self.assertTrue(t.live_play.delivered.wait(THREAD_CAP_S))
        reply = t.tools.ch_stop()
        self.assertIn("play session told to stop", reply)
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertEqual(t.tools.play_snapshot()["stop"], "stopped")
        # The play run took the adopted game's backup, and ch_stop restored it.
        self.assertIn("settings.ini: unchanged", reply)


if __name__ == "__main__":
    unittest.main()
