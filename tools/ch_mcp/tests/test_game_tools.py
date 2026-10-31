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
from tools.ch_mcp import foreground as F  # noqa: E402
from tools.ch_mcp import game_files as G  # noqa: E402
from tools.ch_mcp import game_tools as GT  # noqa: E402
from tools.ch_mcp import install_chart as I  # noqa: E402
from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_mcp.tests.test_bot_run import (  # noqa: E402
    ORIGINAL, PATCHED, FakeLaunchAgent, Rig, StuckGame, _read, _write, wire_human)
from tools.ch_mcp.tests.test_game_files import FakeReg  # noqa: E402
from tools.ch_probe import probe_songs  # noqa: E402
from tools.ch_probe import input_driver  # noqa: E402
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
            sent=2, hit=1, max_streak=1, rows=list(ROWS), stop="stopped"))
        self.events = []
        self.watches = []

        def make_watch():
            self.watches.append(FakeWatch(self.events))
            return self.watches[-1]

        args = dict(driver=self.driver, live_play=self.live_play,
                    find_window=lambda: self.hwnd, make_watch=make_watch)
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

    def test_session_dir_is_the_scratch_folder_the_backups_go_in(self):
        t = ToolRig(self.tmp)
        folder = t.tools.session_dir()
        self.assertTrue(os.path.isdir(folder))
        self.assertTrue(folder.startswith(t.scratch_root))
        self.assertEqual(t.tools.session_dir(), folder)
        t.tools.ch_start(t.r.song)
        self.assertEqual(os.path.dirname(t.backup_dir()), t.tools.session_dir())

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
        _write(os.path.join(t.r.song, G.SONG_INI), b"[song]\nsong_length = 260504\n")
        self.assertEqual(GT.run_cap_s(t.r.song), 320.504)
        self.assertEqual(t.tools.run_cap_s(t.r.song), 320.504)
        no_length = os.path.join(self.tmp, "no length")
        os.makedirs(no_length)
        _write(os.path.join(no_length, G.SONG_INI), b"[song]\nname = x\n")
        self.assertEqual(GT.run_cap_s(no_length), 240.0)
        self.assertEqual(GT.run_cap_s(None), 240.0)

    def test_ch_play_chart_with_a_song_folder_passes_the_cap_to_live_play(self):
        seen = []

        def live_play(chords, **kw):
            seen.append(kw.get("cap_s"))
            return play_chart.RunResult(stop="done")

        t = ToolRig(self.tmp, live_play=live_play)
        _write(os.path.join(t.r.song, G.SONG_INI), b"[song]\nsong_length = 260504\n")
        reply = t.tools.ch_play_chart(self._score_json(), song_folder=t.r.song, route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertEqual(seen, [320.504])
        self.assertIn("run cap: 320.504 s", reply.splitlines())

    # -- the auto-player ------------------------------------------------------------------

    def _score_json(self) -> str:
        path = os.path.join(self.tmp, "score.json")
        _write(path, json.dumps(SCORE_JSON).encode())
        return path

    def test_a_second_ch_play_chart_is_refused_while_one_runs(self):
        t = ToolRig(self.tmp)
        score = self._score_json()
        first = t.tools.ch_play_chart(score, route="keys")
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
        t.tools.ch_play_chart(self._score_json(), log="press.csv", route="keys")
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
        t.tools.ch_play_chart(self._score_json(), route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        lines = t.tools.ch_play_status().splitlines()
        self.assertIn(f"ended: {play_chart.STOP_NO_ENGINE}", lines)
        self.assertIn("no song started within 120 s", lines)

    def test_ch_stop_tells_a_running_play_session_to_stop(self):
        t = ToolRig(self.tmp)
        t.tools.ch_play_chart(self._score_json(), route="keys")
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
        reply = t.tools.ch_play_chart(self._score_json(), route="kit")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertIs(seen[0]["press"], kit_press)
        self.assertNotIn("driver", seen[0])
        self.assertIn("route: kit (presses through the virtual kit inside the game)",
                      reply.splitlines())

    def test_no_route_picks_the_kit_once_it_is_set_up(self):
        def kit_press(lanes):
            return {}

        t, seen = self._kit_rig(kit_press)
        t.tools.ch_play_chart(self._score_json())
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertIs(seen[-1]["press"], kit_press)

    NO_ROUTE_REFUSED = ("refused: no route given and the kit is not set up; "
                        "call ch_kit_setup first")

    def test_no_route_is_refused_while_the_kit_is_not_set_up(self):
        t, seen = self._kit_rig(None)
        self.assertEqual(t.tools.ch_play_chart(self._score_json()), self.NO_ROUTE_REFUSED)
        self.assertIsNone(t.tools._play)
        self.assertEqual(seen, [])

    def test_no_route_is_refused_with_no_kit_tools_and_presses_no_key(self):
        t = ToolRig(self.tmp)
        self.assertEqual(t.tools.ch_play_chart(self._score_json()), self.NO_ROUTE_REFUSED)
        self.assertIsNone(t.tools._play)
        self.assertEqual(t.live_play.calls, [])
        self.assertEqual(t.driver.events, [])

    def test_route_keys_presses_keys_through_the_driver(self):
        t, seen = self._kit_rig(None)
        reply = t.tools.ch_play_chart(self._score_json(), route="keys")
        t.tools._play.thread.join(THREAD_CAP_S)
        self.assertIs(seen[-1]["driver"], t.driver)
        self.assertNotIn("press", seen[-1])
        self.assertIn("route: keys (keys to the focused game window)", reply.splitlines())

    def test_route_kit_without_a_kit_and_an_unknown_route_are_refused(self):
        t, seen = self._kit_rig(None)
        self.assertEqual(t.tools.ch_play_chart(self._score_json(), route="kit"),
                         'route kit: the kit is not set up; call ch_kit_setup first, '
                         'or pass route="keys"')
        self.assertEqual(t.tools.ch_play_chart(self._score_json(), route="pads"),
                         'unknown route "pads"; routes: kit, keys')
        self.assertIsNone(t.tools._play)
        self.assertEqual(seen, [])

    def test_the_status_rows_show_the_kit_columns_only_when_rows_carry_them(self):
        rows = [{**ROWS[0], "down_frame": 10, "up_frame": 12, "down_ms": 100, "up_ms": 113}]

        def live_play(chords, *, on_press, **kw):
            for row in rows:
                on_press(row)
            return play_chart.RunResult(stop="done", sent=1, hit=1, rows=rows)

        t = ToolRig(self.tmp, live_play=live_play)
        t.tools.use_kit_press(lambda: (lambda lanes: {}))
        t.tools.ch_play_chart(self._score_json())
        t.tools._play.thread.join(THREAD_CAP_S)
        lines = t.tools.ch_play_status().splitlines()
        self.assertEqual(lines[-2:], [
            "rows, newest first (index, chart_ms, clock_ms_at_press, score_before, score_after, "
            "hit, down_frame, up_frame, down_ms, up_ms):",
            "  0, 1000.0, 998.0, 0, 50, 1, 10, 12, 100, 113"])

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


if __name__ == "__main__":
    unittest.main()
