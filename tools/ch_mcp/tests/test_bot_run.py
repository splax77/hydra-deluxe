"""Tests for bot_run (the bot launcher) and install_chart.

No game, no real user files and no real registry key: the launch, the clock
and the process check are the fakes below, the user folders are temp
folders, and Rig's Env has no reg.exe runner unless a test hands it
test_game_files' FakeReg.
"""

from __future__ import annotations

import io
import json
import os
import re
import subprocess
import sys
import tempfile
import threading
import unittest
from contextlib import redirect_stdout
from unittest import mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import bot_run as B  # noqa: E402
from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_mcp import game_files as G  # noqa: E402
from tools.ch_mcp import install_chart as I  # noqa: E402
from tools.ch_mcp.tests.test_game_files import FakeReg  # noqa: E402
from tools.ch_probe import probe_songs  # noqa: E402
from tools.ch_probe.constants import PROCESS_NAME  # noqa: E402

ORIGINAL = {name: f"original {name}".encode() for name in G.DOCUMENTS_FILES + G.LOCALLOW_FILES}
NEW_STATS = json.dumps({"players": [{"is_bot": True}]}).encode()
SONG_LENGTH_MS = 4000  # the fake song's song.ini song_length; the cap tests pin 64 s


def _write(path: str, data: bytes) -> None:
    with open(path, "wb") as f:
        f.write(data)


def _read(path: str) -> bytes:
    with open(path, "rb") as f:
        return f.read()


class FakeClock:
    """now() and sleep() for the run. sleep() moves time on and fires any
    event due by then."""

    def __init__(self):
        self.t = 0.0
        self.events = []  # (time, callable)

    def now(self) -> float:
        return self.t

    def sleep(self, s: float) -> None:
        self.t += s
        due = [e for e in self.events if e[0] <= self.t]
        self.events = [e for e in self.events if e[0] > self.t]
        for _, fn in due:
            fn()


class FakeGame:
    def __init__(self, clock: FakeClock, exits_at=None):
        self.clock = clock
        self.exits_at = exits_at
        self.pid = 4242
        self.stopped = False

    def is_running(self) -> bool:
        if self.stopped:
            return False
        return self.exits_at is None or self.clock.t < self.exits_at

    def stop(self) -> None:
        self.stopped = True


class FakeLauncher:
    def __init__(self, game: FakeGame):
        self.game = game
        self.commands = []

    def __call__(self, cmd):
        self.commands.append(cmd)
        return self.game


class Rig:
    """Temp user folders, a song folder and an Env wired to the fakes."""

    def __init__(self, tmp: str, *, exits_at=None, song_ini=None):
        self.docs = os.path.join(tmp, "docs")
        self.low = os.path.join(tmp, "low")
        self.out_dir = os.path.join(tmp, "out")
        self.song = os.path.join(tmp, "songs", probe_songs.WINDOW_MAP)
        for d in (self.docs, self.low, self.song):
            os.makedirs(d)
        for name, data in ORIGINAL.items():
            _write(self.path(name), data)
        ini = song_ini if song_ini is not None else f"[song]\nsong_length = {SONG_LENGTH_MS}\n"
        _write(os.path.join(self.song, G.SONG_INI), ini.encode())
        self.clock = FakeClock()
        self.game = FakeGame(self.clock, exits_at)
        self.launch = FakeLauncher(self.game)
        self.lines = []
        self.env = B.Env(exe=r"C:\fake\Clone Hero.exe", documents_dir=self.docs,
                         locallow_dir=self.low, launch=self.launch, now=self.clock.now,
                         sleep=self.clock.sleep, stamp=lambda: "20261028-101500",
                         out=self.lines.append, run_reg=None)

    def path(self, name: str) -> str:
        return os.path.join(self.docs if name in G.DOCUMENTS_FILES else self.low, name)

    def game_writes_stats_at(self, t: float) -> None:
        def write():
            _write(self.path(G.SCORESTATS), NEW_STATS)
            _write(self.path("scoredata.bin"), b"game wrote this")
        self.clock.events.append((t, write))

    def backup_dir(self) -> str:
        return os.path.join(self.out_dir, "backups", "backup-20261028-101500")

    def assert_all_restored(self, test: unittest.TestCase) -> None:
        for name, data in ORIGINAL.items():
            test.assertEqual(_read(self.path(name)), data, name)


class BotRunTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def test_the_command_line_has_song_and_player_only(self):
        r = Rig(self.tmp, exits_at=10)
        r.game_writes_stats_at(5)
        self.assertEqual(B.run_bot(r.song, r.out_dir, env=r.env), 0)
        self.assertEqual(r.launch.commands,
                         [[r"C:\fake\Clone Hero.exe", "--song", r.song, "--player", "Drums,Expert"]])

    def test_a_relative_song_path_reaches_the_game_absolute(self):
        # The game starts in its own folder, so it must get the full path.
        r = Rig(self.tmp, exits_at=10)
        r.game_writes_stats_at(5)
        old = os.getcwd()
        os.chdir(os.path.dirname(r.song))
        try:
            B.run_bot(probe_songs.WINDOW_MAP, r.out_dir, env=r.env)
        finally:
            os.chdir(old)
        song_arg = r.launch.commands[0][2]
        self.assertTrue(os.path.isabs(song_arg))
        self.assertEqual(os.path.basename(song_arg), probe_songs.WINDOW_MAP)

    def test_the_game_exe_name_is_the_probe_process_name(self):
        self.assertEqual(os.path.basename(G.GAME_EXE), PROCESS_NAME)

    def test_profile_appears_only_when_passed(self):
        cmd = B.launch_command("exe", "song", "ProDrums,Expert", "Tester")
        self.assertEqual(cmd, ["exe", "--song", "song", "--player", "ProDrums,Expert",
                               "--profile", "Tester"])
        self.assertNotIn("--profile", B.launch_command("exe", "song", "Drums,Expert"))

    def test_poll_notices_the_change_copies_it_and_restores(self):
        r = Rig(self.tmp, exits_at=12)
        r.game_writes_stats_at(5)
        self.assertEqual(B.run_bot(r.song, r.out_dir, env=r.env), 0)
        copy = os.path.join(r.out_dir, "scorestats-Window_Map-20261028-101500.json")
        self.assertEqual(_read(copy), NEW_STATS)
        self.assertIn(f"scorestats.json changed after 5 s; copied to {copy}", r.lines)
        self.assertIn("game exited after 7 s", r.lines)
        self.assertIn(f"backup folder: {r.backup_dir()}", r.lines)
        r.assert_all_restored(self)
        self.assertFalse(r.game.stopped)

    def test_the_exit_check_reports_a_game_that_stays(self):
        r = Rig(self.tmp, exits_at=None)
        r.game_writes_stats_at(5)
        self.assertEqual(B.run_bot(r.song, r.out_dir, env=r.env), 0)
        self.assertIn("game still running after 30 s (menu or results screen)", r.lines)
        self.assertFalse(r.game.stopped)  # never closed without --stop
        self.assertTrue(any("--restore" in line for line in r.lines))
        r.assert_all_restored(self)

    def test_stop_closes_a_game_that_stays(self):
        r = Rig(self.tmp, exits_at=None)
        r.game_writes_stats_at(5)
        B.run_bot(r.song, r.out_dir, stop=True, env=r.env)
        self.assertTrue(r.game.stopped)
        self.assertIn("stopped the game (--stop)", r.lines)

    def test_the_cap_prints_its_line_and_still_restores(self):
        r = Rig(self.tmp, exits_at=None)
        # The game changes settings.ini during the run but never the stats.
        r.clock.events.append((10, lambda: _write(r.path("settings.ini"), b"the game changed this")))
        self.assertEqual(B.run_bot(r.song, r.out_dir, env=r.env), 1)
        self.assertIn("scorestats.json unchanged after 64 s; bot launch wrote no stats", r.lines)
        self.assertFalse(any("game exited" in line or "still running after" in line
                             for line in r.lines))
        self.assertEqual(r.clock.t, 64)
        self.assertFalse(os.path.exists(os.path.join(r.out_dir, "scorestats-Window_Map-20261028-101500.json")))
        self.assertIn("settings.ini: restored", r.lines)
        r.assert_all_restored(self)

    def test_a_restore_failure_stops_the_run_with_the_file_name(self):
        r = Rig(self.tmp, exits_at=None)
        r.game_writes_stats_at(5)

        def fail(backup_dir, out):
            raise G.RestoreError("scoredata.bin", "locked")

        with mock.patch.object(B.game_files, "restore", fail):
            self.assertEqual(B.run_bot(r.song, r.out_dir, env=r.env), 2)
        self.assertIn("restore failed on scoredata.bin: locked", r.lines)
        self.assertEqual(r.lines[-1], f"stopped; the backup is still in {r.backup_dir()}")

    def test_a_restore_failure_on_the_cap_path_runs_no_exit_check(self):
        r = Rig(self.tmp, exits_at=None)

        def fail(backup_dir, out):
            raise G.RestoreError("user.bin", "locked")

        with mock.patch.object(B.game_files, "restore", fail):
            self.assertEqual(B.run_bot(r.song, r.out_dir, env=r.env), 2)
        self.assertIn("restore failed on user.bin: locked", r.lines)
        self.assertFalse(any("game exited" in line or "still running after" in line
                             for line in r.lines))

    def test_no_wait_starts_the_game_polls_nothing_restores_nothing(self):
        r = Rig(self.tmp, exits_at=None)
        self.assertEqual(B.run_bot(r.song, r.out_dir, no_wait=True, env=r.env), 0)
        self.assertEqual(len(r.launch.commands), 1)
        self.assertEqual(r.clock.t, 0.0)  # never slept, so never polled
        self.assertIn(f"backup folder: {r.backup_dir()}", r.lines)
        self.assertIn("game process id: 4242", r.lines)
        self.assertEqual(r.lines[-1],
                         "--no-wait: not waiting and not restoring; afterwards run "
                         f'py tools/ch_mcp/bot_run.py --restore "{r.backup_dir()}"')
        self.assertFalse(any(line.startswith("restoring") for line in r.lines))

        # The step: the game rewrites every backed-up file.
        for name in ORIGINAL:
            _write(r.path(name), b"changed by the game")
        restore_lines = []
        env = B.Env(out=restore_lines.append)
        self.assertEqual(B.restore_only(r.backup_dir(), env=env), 0)
        r.assert_all_restored(self)
        self.assertEqual(restore_lines[1:], [f"{name}: restored" for name in
                                             G.DOCUMENTS_FILES + G.LOCALLOW_FILES])

    def test_restore_command_line_restores(self):
        r = Rig(self.tmp, exits_at=None)
        B.run_bot(r.song, r.out_dir, no_wait=True, env=r.env)
        _write(r.path("user.bin"), b"changed")
        buf = io.StringIO()
        with redirect_stdout(buf):
            self.assertEqual(B.main(["--restore", r.backup_dir()]), 0)
        self.assertIn("user.bin: restored", buf.getvalue())
        r.assert_all_restored(self)

    def test_the_manifest_sets_the_cap_when_song_ini_has_no_length(self):
        r = Rig(self.tmp, song_ini="[song]\nname = x\n")
        with open(os.path.join(r.song, B.MANIFEST), "w", encoding="utf-8") as f:
            json.dump({"notes": [{"time_ms": 500}, {"time_ms": 1000}]}, f)
        ms, source = B.song_length_ms(r.song)
        self.assertEqual(ms, probe_songs.song_length_ms(1000))
        self.assertIn("manifest.json", source)

    def test_no_length_anywhere_starts_nothing(self):
        r = Rig(self.tmp, song_ini="[song]\nname = x\n")
        with self.assertRaises(B.SongLengthMissing):
            B.run_bot(r.song, r.out_dir, env=r.env)
        self.assertEqual(r.launch.commands, [])
        self.assertFalse(os.path.exists(r.out_dir))

    def test_pro_drums_chart_with_a_drums_player_prints_a_note(self):
        r = Rig(self.tmp, exits_at=10,
                song_ini=f"[song]\nsong_length = {SONG_LENGTH_MS}\npro_drums = True\n")
        r.game_writes_stats_at(5)
        B.run_bot(r.song, r.out_dir, env=r.env)
        self.assertTrue(r.lines[0].startswith("note: song.ini says pro_drums = True"))


class StuckGame(FakeGame):
    """A game that does not exit within the stop's wait."""

    def stop(self) -> None:
        raise subprocess.TimeoutExpired(PROCESS_NAME, B.EXIT_CHECK_S)


class RunStepsTest(unittest.TestCase):
    """The four steps the server calls one at a time."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def start(self, r: Rig, song_dir) -> B.Run:
        return B.start_run(song_dir, backup_root=os.path.join(r.out_dir, "backups"), env=r.env)

    def test_start_run_backs_up_launches_and_returns_before_any_sleep(self):
        r = Rig(self.tmp, exits_at=None)
        before = B._file_state(r.path(G.SCORESTATS))
        run = self.start(r, r.song)
        self.assertEqual(r.clock.t, 0.0)
        self.assertEqual(run.backup_dir, r.backup_dir())
        self.assertTrue(os.path.isfile(os.path.join(run.backup_dir, G.SCORESTATS)))
        self.assertIsNotNone(run.before)
        self.assertEqual(run.before, before)
        self.assertEqual(r.launch.commands, [run.command])
        self.assertEqual(run.command, B.launch_command(r.env.exe, r.song, B.DEFAULT_PLAYER))
        self.assertIs(run.game, r.game)
        self.assertEqual(run.launched_at, 0.0)
        self.assertEqual(run.length_ms, SONG_LENGTH_MS)
        self.assertEqual(run.cap_s, 64)
        self.assertIn("song.ini", run.length_source)
        self.assertEqual(len(r.lines), 3)
        self.assertEqual(r.lines[0], f"backup folder: {r.backup_dir()}")
        self.assertTrue(r.lines[1].startswith("starting: "))
        self.assertEqual(r.lines[2], "game process id: 4242")
        self.assertFalse(any(line.startswith("restoring") for line in r.lines))

    def test_run_cap_is_the_song_plus_the_extra_minute(self):
        # The Senri fill-check chart's song_length (ruling 3 of the 2026-10-30
        # ch_mcp Star Power plan).
        self.assertEqual(B.run_cap_s(260504), 320.504)

    def test_start_runs_cap_is_run_cap_s_of_the_song_length(self):
        r = Rig(self.tmp, exits_at=None, song_ini="[song]\nsong_length = 260504\n")
        run = self.start(r, r.song)
        self.assertEqual(run.cap_s, 320.504)
        self.assertEqual(run.cap_s, B.run_cap_s(run.length_ms))

    def test_start_run_with_no_song_launches_the_bare_exe_after_a_backup(self):
        r = Rig(self.tmp, exits_at=None)
        run = self.start(r, None)
        self.assertEqual(r.launch.commands, [[r"C:\fake\Clone Hero.exe"]])
        self.assertTrue(os.path.isdir(run.backup_dir))
        self.assertIsNone(run.song_dir)
        self.assertIsNone(run.cap_s)
        self.assertEqual(B.launch_command("exe", None, "Drums,Expert"), ["exe"])
        self.assertEqual(B.launch_command("exe", None, "ProDrums,Expert", "Tester"), ["exe"])

    def test_wait_stats_on_a_no_song_run_says_so_without_sleeping(self):
        r = Rig(self.tmp, exits_at=None)
        run = self.start(r, None)
        self.assertIsNone(B.wait_stats(run, r.out_dir, r.env))
        self.assertEqual(r.lines[-1], "no song was launched from the command line; nothing to wait for")
        self.assertEqual(r.clock.t, 0.0)

    def test_wait_stats_copies_a_changed_file_and_names_the_seconds(self):
        r = Rig(self.tmp, exits_at=None)
        run = self.start(r, r.song)
        r.game_writes_stats_at(5)
        copy = B.wait_stats(run, r.out_dir, r.env)
        self.assertEqual(copy, os.path.join(r.out_dir, "scorestats-Window_Map-20261028-101500.json"))
        self.assertEqual(_read(copy), NEW_STATS)
        self.assertIn(f"scorestats.json changed after 5 s; copied to {copy}", r.lines)

    def test_finish_run_with_stop_stops_the_game_and_restores_all_five_files(self):
        r = Rig(self.tmp, exits_at=None)
        run = self.start(r, r.song)
        for name in ORIGINAL:
            _write(r.path(name), b"changed by the game")
        self.assertTrue(B.finish_run(run, stop=True, env=r.env))
        self.assertTrue(r.game.stopped)
        self.assertIn("stopped the game (--stop)", r.lines)
        for name in G.DOCUMENTS_FILES + G.LOCALLOW_FILES:
            self.assertIn(f"{name}: restored", r.lines)
        r.assert_all_restored(self)
        self.assertFalse(any("--restore" in line for line in r.lines))

    def test_finish_run_says_a_caller_given_stop_line_instead_of_the_flag_one(self):
        r = Rig(self.tmp, exits_at=None)
        run = self.start(r, r.song)
        self.assertTrue(B.finish_run(run, stop=True, env=r.env, stop_line=B.STOPPED_LINE))
        self.assertIn("stopped the game", r.lines)
        self.assertNotIn("stopped the game (--stop)", r.lines)

    def test_finish_run_reports_a_game_that_outlives_the_stop_and_still_restores(self):
        r = Rig(self.tmp, exits_at=None)
        r.game = r.launch.game = StuckGame(r.clock)
        run = self.start(r, r.song)
        for name in ORIGINAL:
            _write(r.path(name), b"changed by the game")
        self.assertTrue(B.finish_run(run, stop=True, env=r.env))
        self.assertIn(f"{PROCESS_NAME} (pid 4242) was told to stop but was still running "
                      "after 30 s", r.lines)
        self.assertNotIn("stopped the game (--stop)", r.lines)
        r.assert_all_restored(self)
        self.assertEqual(r.lines[-1],
                         "the game is still running and may write these files again when it "
                         f'closes; after closing it, run py tools/ch_mcp/bot_run.py --restore '
                         f'"{r.backup_dir()}"')

    def test_finish_run_with_a_failed_restore_names_the_file_and_returns_false(self):
        r = Rig(self.tmp, exits_at=None)
        run = self.start(r, r.song)

        def fail(backup_dir, out):
            raise G.RestoreError("scoredata.bin", "locked")

        with mock.patch.object(B.game_files, "restore", fail):
            self.assertFalse(B.finish_run(run, stop=False, env=r.env))
        self.assertIn("restore failed on scoredata.bin: locked", r.lines)
        self.assertEqual(r.lines[-1], f"stopped; the backup is still in {r.backup_dir()}")


class LaunchStepsTest(unittest.TestCase):
    """What start_run does between the backup and the launch, and the
    registry key riding along with the files."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def start(self, r: Rig, **kw) -> B.Run:
        return B.start_run(r.song, backup_root=os.path.join(r.out_dir, "backups"), env=r.env, **kw)

    def test_the_servers_env_backs_up_the_key_with_reg_exe(self):
        self.assertIs(B.Env().run_reg, G.run_reg)

    def test_before_launch_runs_after_the_backup_and_before_the_launch(self):
        r = Rig(self.tmp)
        seen = []
        real_launch = r.launch

        def launch(cmd):
            seen.append("launch")
            return real_launch(cmd)

        r.env.launch = launch
        self.start(r, before_launch=lambda: seen.append(
            ("before_launch", os.path.isdir(r.backup_dir()))))
        self.assertEqual(seen, [("before_launch", True), "launch"])

    def test_a_launch_that_raises_puts_back_what_before_launch_changed(self):
        r = Rig(self.tmp)

        def launch(cmd):
            raise OSError("no such file")

        r.env.launch = launch
        with self.assertRaises(OSError):
            self.start(r, before_launch=lambda: _write(r.path(G.SETTINGS), b"changed"))
        r.assert_all_restored(self)
        self.assertIn(f"{G.SETTINGS}: restored", r.lines)

    def test_the_key_is_backed_up_and_restored_with_the_envs_runner(self):
        r = Rig(self.tmp)
        r.env.run_reg = reg = FakeReg()
        original = reg.text
        run = self.start(r)
        self.assertTrue(os.path.isfile(os.path.join(run.backup_dir, G.REGISTRY_BACKUP)))
        reg.text = b"the game changed this"
        self.assertTrue(B.finish_run(run, stop=True, env=r.env))
        self.assertEqual(r.lines[-1], "rewired registry: restored")
        self.assertEqual(reg.text, original)

    def test_a_registry_restore_failure_stops_like_a_file_one(self):
        r = Rig(self.tmp)
        r.env.run_reg = reg = FakeReg()
        run = self.start(r)
        reg.text = b"the game changed this"
        reg.import_works = False
        self.assertFalse(B.finish_run(run, stop=True, env=r.env))
        self.assertTrue(any(line.startswith("restore failed on rewired.reg: ")
                            for line in r.lines), r.lines)
        self.assertEqual(r.lines[-1], f"stopped; the backup is still in {r.backup_dir()}")


class FakePopen:
    """Records how subprocess.Popen was called."""
    calls: list = []

    def __init__(self, cmd, **kw):
        self.pid = 4321
        FakePopen.calls.append((cmd, kw))


class RealLaunchTest(unittest.TestCase):
    def test_the_game_gets_no_stdin_and_no_stdout_and_keeps_its_folder(self):
        FakePopen.calls = []
        cmd = [os.path.join("games", "ch", "Clone Hero.exe"), "--song", "x"]
        with mock.patch.object(B.subprocess, "Popen", FakePopen):
            game = B._real_launch(cmd)
        self.assertEqual(game.pid, 4321)
        [(seen_cmd, kw)] = FakePopen.calls
        kw = dict(kw)
        kw.pop("startupinfo")  # test_the_game_opens_minimized_without_the_focus checks it
        self.assertEqual([(seen_cmd, kw)], [(cmd, {
            "cwd": os.path.dirname(cmd[0]),
            "stdin": subprocess.DEVNULL,
            "stdout": subprocess.DEVNULL})])

    def test_the_game_opens_minimized_without_the_focus(self):
        FakePopen.calls = []
        with mock.patch.object(B.subprocess, "Popen", FakePopen):
            B._real_launch([os.path.join("games", "ch", "Clone Hero.exe")])
        info = FakePopen.calls[0][1]["startupinfo"]
        self.assertTrue(info.dwFlags & subprocess.STARTF_USESHOWWINDOW)
        # SW_SHOWMINNOACTIVE: minimized, not activated.
        self.assertEqual(info.wShowWindow, 7)
        self.assertEqual(B.SW_SHOWMINNOACTIVE, 7)


# launch_patch.json's fields that bot_run reads or names, with the values the
# direct-start plan's "The launch" gives for build 6142.
PATCH = {"launch": "suspended", "defer": False,
         "entry_expect": "40 56 57 41 55 48 81 EC C0 00 00 00",
         "expect": "BA 01 00 00 00", "write": "BA 00 00 00 00"}
# How long a test waits on a fake's thread before it fails instead of hanging.
THREAD_CAP_S = 5.0


def status(state, **kw):
    """A launch.ts status reply."""
    return {"state": state, **kw}


PATCHED = status("patched", before="BA 01 00 00 00", after="BA 00 00 00 00",
                 entry_before=PATCH["entry_expect"], module={"base": "0x7ff800000000", "size": 1},
                 loader_runs=0, bytes_at_first_run=None, patched_at_ms=1200, reason=None)


class FakeLaunchAgent:
    """launch.ts behind a LiveAgent: start, arm and status answer from the
    lists given, recording each call into the rig's `events`. `statuses`
    replies in turn and repeats its last one."""

    def __init__(self, events, *, statuses=(PATCHED,), arm_reply=None, arm_error=None,
                 start_error=None, start_block=None):
        self.events = events
        self.statuses = list(statuses)
        self.arm_reply = arm_reply or status("waiting", modules=["Clone Hero.exe", "ntdll.dll"])
        self.arm_error = arm_error
        self.start_error = start_error
        self.start_block = start_block
        self.armed = False
        self.requests = []
        self.start_caps = []

    def start(self, cap_s):
        self.events.append("load")
        self.start_caps.append(cap_s)
        if self.start_block is not None:
            self.start_block.wait(THREAD_CAP_S)
        if self.start_error is not None:
            raise self.start_error
        return {"state": "idle"}

    def request(self, message, cap_s):
        self.events.append(message["type"])
        self.requests.append((message, cap_s))
        if message["type"] == "arm":
            if self.arm_error is not None:
                raise self.arm_error
            self.armed = True
            return self.arm_reply
        reply = self.statuses[0]
        if len(self.statuses) > 1:
            self.statuses.pop(0)
        return reply

    def detach(self):
        self.events.append("detach")
        return "detached"


def wire_human(r: Rig, agent: FakeLaunchAgent, patch: dict = PATCH) -> list:
    """Give Rig `r`'s Env the human launch's fakes, a launch_patch.json
    holding `patch` and a built launch agent, all in temp files. Returns
    the attach calls (pid, agent path, log path)."""
    root = os.path.dirname(r.docs)
    r.env.launch_patch_path = os.path.join(root, "launch_patch.json")
    _write(r.env.launch_patch_path, json.dumps(patch).encode())
    r.env.launch_agent_path = os.path.join(root, "launch.js")
    _write(r.env.launch_agent_path, b"// the built agent")
    events = agent.events
    attaches = []

    def start(form):
        def go(cmd):
            events.append(form)
            r.launch.commands.append(cmd)
            return r.game
        return go

    def attach(pid, path, log, out):
        events.append("attach")
        attaches.append((pid, path, log))
        return agent

    def resume(name):
        def go(pid):
            assert agent.armed, "resumed before the arm reply"
            events.append(name)
        return go

    def kill(game):
        events.append("kill")
        game.stopped = True

    r.env.start_process = start("start")
    r.env.spawn_process = start("spawn")
    r.env.attach_agent = attach
    r.env.resume_thread = resume("resume")
    r.env.resume_spawned = resume("resume spawned")
    r.env.kill = kill
    return attaches


class HumanLaunchTest(unittest.TestCase):
    """launch_human's order, caps and refusals, on fakes."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name
        self.r = Rig(self.tmp, exits_at=None)
        self.events = []

    def tearDown(self):
        self._tmp.cleanup()

    def launch(self, agent, patch=PATCH):
        attaches = wire_human(self.r, agent, patch)
        log = os.path.join(self.tmp, "launch.jsonl")
        return B.launch_human(["exe", "--song", "s"], self.r.env, log), attaches

    def fails(self, agent, patch=PATCH) -> str:
        with self.assertRaises(B.LaunchFailed) as cm:
            self.launch(agent, patch)
        return str(cm.exception)

    def test_the_caps_and_the_poll_are_the_users_answers(self):
        self.assertEqual((B.LAUNCH_ATTACH_CAP_S, B.LAUNCH_ARM_CAP_S, B.LAUNCH_PATCH_CAP_S,
                          B.LAUNCH_POLL_S), (20.0, 5.0, 30.0, 0.5))
        self.assertEqual(B.CREATE_SUSPENDED, 4)

    def test_the_paused_start_passes_create_suspended_and_todays_startupinfo(self):
        FakePopen.calls = []
        cmd = [os.path.join("games", "ch", "Clone Hero.exe"), "--song", "x"]
        with mock.patch.object(B.subprocess, "Popen", FakePopen):
            game = B._real_start_suspended(cmd)
        self.assertEqual(game.pid, 4321)
        [(seen_cmd, kw)] = FakePopen.calls
        info = kw.pop("startupinfo")
        self.assertTrue(info.dwFlags & subprocess.STARTF_USESHOWWINDOW)
        self.assertEqual(info.wShowWindow, 7)
        self.assertEqual((seen_cmd, kw), (cmd, {
            "cwd": os.path.dirname(cmd[0]), "stdin": subprocess.DEVNULL,
            "stdout": subprocess.DEVNULL, "creationflags": 4}))

    def test_the_order_is_start_attach_load_arm_resume_status(self):
        agent = FakeLaunchAgent(self.events)
        launch, attaches = self.launch(agent)
        self.assertEqual(self.events, ["start", "attach", "load", "arm", "resume", "status"])
        self.assertIs(launch.agent, agent)
        self.assertIs(launch.game, self.r.game)
        self.assertEqual(launch.form, "suspended")
        self.assertEqual(attaches, [(4242, self.r.env.launch_agent_path,
                                     os.path.join(self.tmp, "launch.jsonl"))])
        self.assertEqual(agent.start_caps, [20.0])
        # arm carries the data file's contents, with the arm cap.
        self.assertEqual(agent.requests[0], ({"type": "arm", "patch": PATCH}, 5.0))
        self.assertIn("armed; modules loaded now (2): Clone Hero.exe, ntdll.dll", self.r.lines)
        self.assertIn("patched 0.0 s after the resume", self.r.lines)
        self.assertFalse(self.r.game.stopped)

    def test_waiting_statuses_are_polled_every_half_second_until_patched(self):
        agent = FakeLaunchAgent(self.events, statuses=[status("waiting")] * 3 + [PATCHED])
        self.launch(agent)
        self.assertEqual(self.events, ["start", "attach", "load", "arm", "resume"]
                         + ["status"] * 4)
        self.assertEqual(self.r.clock.t, 1.5)
        self.assertIn("patched 1.5 s after the resume", self.r.lines)

    def test_a_refused_status_kills_the_game_and_names_the_bytes(self):
        refused = status("refused", before="BA 02 00 00 00", entry_before="CC CC CC CC")
        agent = FakeLaunchAgent(self.events, statuses=[status("waiting"), refused])
        said = self.fails(agent)
        self.assertIn("at the loader's entry it found CC CC CC CC (launch_patch.json expects "
                      "40 56 57 41 55 48 81 EC C0 00 00 00)", said)
        self.assertIn("at the patch it found BA 02 00 00 00 (expects BA 01 00 00 00)", said)
        self.assertIn("the game was killed", said)
        self.assertEqual(self.events[-2:], ["kill", "detach"])
        self.assertTrue(self.r.game.stopped)

    def test_an_attach_that_never_returns_ends_at_its_cap_and_kills_the_game(self):
        block = threading.Event()
        agent = FakeLaunchAgent(self.events, start_block=block)
        waited = []
        self.r.env.wait_thread = lambda thread, cap_s: waited.append(cap_s)
        try:
            said = self.fails(agent)
        finally:
            block.set()
        self.assertEqual(waited, [20.0])
        self.assertIn("had not finished after 20 s (LAUNCH_ATTACH_CAP_S)", said)
        self.assertIn("kill", self.events)
        self.assertNotIn("resume", self.events)
        self.assertNotIn("detach", self.events)  # the start is still running
        self.assertTrue(self.r.game.stopped)

    def test_an_attach_that_fails_kills_the_game(self):
        agent = FakeLaunchAgent(self.events, start_error=FA.AttachFailed(
            FA.END_SCRIPT_ERROR, "script error: loading the agent failed: boom"))
        said = self.fails(agent)
        self.assertIn("the attach to process 4242 failed: AttachFailed: script error", said)
        # A failed LiveAgent.start leaves nothing attached, so nothing is detached.
        self.assertEqual(self.events, ["start", "attach", "load", "kill"])

    def test_an_arm_reply_that_never_comes_ends_at_its_cap_without_a_resume(self):
        agent = FakeLaunchAgent(self.events, arm_error=FA.ReadFailed(
            FA.END_CAP, "no reply to the arm request within the cap of 5 s"))
        said = self.fails(agent)
        self.assertEqual(agent.requests[0][1], 5.0)
        self.assertIn("within the cap of 5 s (LAUNCH_ARM_CAP_S)", said)
        self.assertEqual(self.events, ["start", "attach", "load", "arm", "kill", "detach"])
        self.assertTrue(self.r.game.stopped)

    def test_a_refused_arm_reply_kills_the_game_without_a_resume(self):
        agent = FakeLaunchAgent(self.events, arm_reply=status(
            "refused", before="00 00 00 00 00", entry_before=PATCH["entry_expect"]))
        said = self.fails(agent)
        self.assertIn("at the patch it found 00 00 00 00 00", said)
        self.assertNotIn("resume", self.events)

    def test_an_idle_arm_reply_is_a_fault_that_kills_the_game_without_a_resume(self):
        agent = FakeLaunchAgent(self.events, arm_reply=status("idle"))
        said = self.fails(agent)
        self.assertIn(B.IDLE_AFTER_ARM, said)
        self.assertEqual(self.events, ["start", "attach", "load", "arm", "kill", "detach"])

    def test_a_patched_arm_reply_still_resumes_and_reads_the_status(self):
        agent = FakeLaunchAgent(self.events, arm_reply=status("patched", modules=["x.dll"]))
        launch, _ = self.launch(agent)
        self.assertEqual(self.events, ["start", "attach", "load", "arm", "resume", "status"])
        self.assertIs(launch.agent, agent)

    def test_an_idle_status_after_the_arm_is_a_fault_named_at_once(self):
        agent = FakeLaunchAgent(self.events, statuses=[status("waiting"), status("idle")])
        said = self.fails(agent)
        self.assertIn(B.IDLE_AFTER_ARM, said)
        self.assertEqual(self.r.clock.t, 0.5)  # one poll, not the patch cap
        self.assertEqual(self.events[-2:], ["kill", "detach"])

    def test_a_refusal_for_a_failed_read_back_carries_the_agents_reason(self):
        refused = status("refused", before="BA 01 00 00 00", after="BA 01 00 00 00",
                         entry_before=PATCH["entry_expect"],
                         reason="the write did not take: the patch site at 0x396EC5 reads "
                               "BA 01 00 00 00 after writing BA 00 00 00 00")
        said = self.fails(FakeLaunchAgent(self.events, statuses=[refused]))
        self.assertIn("and BA 01 00 00 00 after; the agent says: the write did not take", said)
        self.assertIn("the game was killed", said)

    def test_a_status_that_stays_waiting_ends_at_the_patch_cap_from_the_resume(self):
        agent = FakeLaunchAgent(self.events, statuses=[status("waiting")])
        resumed = []
        wire_human(self.r, agent)
        fake_resume = self.r.env.resume_thread

        def resume(pid):
            fake_resume(pid)
            resumed.append(self.r.clock.t)

        self.r.env.resume_thread = resume
        self.r.clock.t = 7.0  # the resume comes later than time 0
        with self.assertRaises(B.LaunchFailed) as cm:
            B.launch_human(["exe"], self.r.env, os.path.join(self.tmp, "l.jsonl"))
        said = str(cm.exception)
        self.assertIn("still said waiting 30 s after the resume (LAUNCH_PATCH_CAP_S)", said)
        self.assertEqual(resumed, [7.0])
        self.assertEqual(self.r.clock.t, 37.0)
        statuses = [cap for message, cap in agent.requests if message["type"] == "status"]
        self.assertEqual(len(statuses), 60)
        self.assertEqual(statuses[0], 30.0)  # each status waits at most what is left
        self.assertEqual(self.events[-2:], ["kill", "detach"])

    def test_the_spawn_form_takes_the_spawn_seam_and_resumes_through_the_device(self):
        agent = FakeLaunchAgent(self.events)
        launch, _ = self.launch(agent, {**PATCH, "launch": "spawn"})
        self.assertEqual(self.events,
                         ["spawn", "attach", "load", "arm", "resume spawned", "status"])
        self.assertEqual(launch.form, "spawn")

    def test_a_bad_launch_form_or_a_missing_agent_starts_nothing(self):
        agent = FakeLaunchAgent(self.events)
        said = self.fails(agent, {**PATCH, "launch": "late"})
        self.assertIn("`launch` is 'late', not one of suspended, spawn; nothing was started", said)
        wire_human(self.r, agent)
        os.remove(self.r.env.launch_agent_path)
        with self.assertRaises(B.LaunchFailed) as cm:
            B.launch_human(["exe"], self.r.env, os.path.join(self.tmp, "l.jsonl"))
        self.assertIn("npm run build:launch", str(cm.exception))
        self.assertEqual(self.events, [])

    def test_the_real_env_uses_the_real_seams_and_files(self):
        env = B.Env()
        self.assertIs(env.start_process, B._real_start_suspended)
        self.assertIs(env.resume_thread, B.resume_main_thread)
        self.assertEqual(os.path.basename(env.launch_agent_path), "launch.js")
        self.assertEqual(os.path.basename(env.launch_patch_path), "launch_patch.json")

    def test_the_real_attach_builds_a_live_agent_for_the_pid(self):
        seen = []

        class FakeLiveAgent:
            def __init__(self, agent_path, **kw):
                seen.append((agent_path, kw))

        out = [].append
        with mock.patch.object(B.FA, "LiveAgent", FakeLiveAgent):
            B._real_attach_agent(1234, "launch.js", "log.jsonl", out)
        self.assertEqual(seen, [("launch.js", {"log_path": "log.jsonl", "pid": 1234, "out": out})])

    def test_resume_main_thread_refuses_a_process_with_no_thread(self):
        # A Toolhelp snapshot only reads; no process id is a multiple of 4
        # this high.
        with self.assertRaises(OSError) as cm:
            B.resume_main_thread(0xFFFFFFF0)
        self.assertIn("has no thread to resume", str(cm.exception))

    def test_bot_run_never_opens_the_game_exe_for_writing(self):
        with open(B.__file__, encoding="utf-8") as f:
            source = f.read().splitlines()
        write_mode = re.compile(r"""open\([^)]*["'][rbt]*[wax+]""")
        self.assertEqual([line for line in source if write_mode.search(line)], [])
        for line in source:
            if "GAME_EXE" in line:
                self.assertNotIn("open(", line)


class LoaderLineTest(unittest.TestCase):
    """The three words of ch_status's loader line."""

    def test_the_three_words(self):
        self.assertEqual((B.LOADER_YES, B.LOADER_NO, B.LOADER_NOT_YET), ("yes", "no", "not yet"))
        self.assertEqual(B.loader_ran(status("patched", loader_runs=1,
                                             bytes_at_first_run="BA 00 00 00 00"), PATCH), "yes")
        self.assertEqual(B.loader_ran(status("patched", loader_runs=2,
                                             bytes_at_first_run="BA 01 00 00 00"), PATCH), "no")
        self.assertEqual(B.loader_ran(status("patched", loader_runs=0), PATCH), "not yet")
        self.assertEqual(B.loader_ran(status("waiting"), PATCH), "not yet")

    def test_a_list_of_numbers_reads_like_the_same_hex_text(self):
        self.assertEqual(B.loader_ran(status("patched", loader_runs=1,
                                             bytes_at_first_run=[0xBA, 0, 0, 0, 0]), PATCH), "yes")


class HumanStartRunTest(unittest.TestCase):
    """start_run with human=True."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name
        self.r = Rig(self.tmp, exits_at=None)
        self.events = []

    def tearDown(self):
        self._tmp.cleanup()

    def start(self, song_dir, agent):
        wire_human(self.r, agent)
        return B.start_run(song_dir, player="ProDrums,Expert",
                           backup_root=os.path.join(self.r.out_dir, "backups"),
                           env=self.r.env, human=True)

    def test_no_song_folder_is_refused_before_the_backup(self):
        with self.assertRaises(B.LaunchFailed) as cm:
            self.start(None, FakeLaunchAgent(self.events))
        self.assertEqual(str(cm.exception), B.HUMAN_NEEDS_SONG)
        self.assertIn("runs only with --song", B.HUMAN_NEEDS_SONG)
        self.assertEqual(B.human_refusal(True, None), B.HUMAN_NEEDS_SONG)
        self.assertIsNone(B.human_refusal(True, self.r.song))
        self.assertIsNone(B.human_refusal(False, None))
        self.assertFalse(os.path.exists(self.r.out_dir))
        self.assertEqual(self.events, [])

    def test_the_backup_comes_before_the_paused_start_and_the_run_keeps_the_launch(self):
        agent = FakeLaunchAgent(self.events)
        seen = []
        attaches = wire_human(self.r, agent)
        real_start = self.r.env.start_process
        self.r.env.start_process = lambda cmd: (seen.append(os.path.isdir(self.r.backup_dir())),
                                                real_start(cmd))[1]
        run = B.start_run(self.r.song, player="ProDrums,Expert",
                          backup_root=os.path.join(self.r.out_dir, "backups"),
                          env=self.r.env, human=True)
        self.assertEqual(seen, [True])
        self.assertIs(run.launch.agent, agent)
        self.assertIs(run.game, self.r.game)
        self.assertEqual(run.command, B.launch_command(self.r.env.exe, self.r.song,
                                                       "ProDrums,Expert"))
        # The agent's log goes beside the backup folder.
        self.assertEqual(attaches[0][2], os.path.join(self.r.out_dir, "backups",
                                                      "launch-20261028-101500.jsonl"))
        self.assertEqual(self.events[0], "start")

    def test_a_failed_human_launch_restores_the_files(self):
        agent = FakeLaunchAgent(self.events, statuses=[status("refused")])
        with self.assertRaises(B.LaunchFailed):
            self.start(self.r.song, agent)
        self.assertIn(f"restoring the user files from {self.r.backup_dir()}", self.r.lines)
        self.r.assert_all_restored(self)


class InstallChartTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name
        self.src = os.path.join(self.tmp, "input", "Some Song")
        self.dest_root = os.path.join(self.tmp, "Hydra Probe")
        os.makedirs(self.src)
        _write(os.path.join(self.src, G.SONG_INI), b"[song]\nsong_length = 90000\n")
        _write(os.path.join(self.src, "notes.mid"), b"MThd")
        self.oggs = []

    def tearDown(self):
        self._tmp.cleanup()

    def fake_ogg(self, path, length_ms):
        self.oggs.append((path, length_ms))

    def test_copies_the_folder_and_writes_the_silent_ogg(self):
        chart = I.install(self.src, dest_root=self.dest_root, write_ogg=self.fake_ogg)
        dest = os.path.join(self.dest_root, "Some Song")
        self.assertEqual(chart, os.path.join(dest, "notes.mid"))
        self.assertEqual(self.oggs, [(os.path.join(dest, probe_songs.SONG_OGG), 90000)])

    def test_refuses_an_existing_folder(self):
        dest = os.path.join(self.dest_root, "Some Song")
        os.makedirs(dest)
        _write(os.path.join(dest, "keep.txt"), b"mine")
        with self.assertRaises(I.InstallRefused) as cm:
            I.install(self.src, dest_root=self.dest_root, write_ogg=self.fake_ogg)
        self.assertIn("already exists; not overwritten", str(cm.exception))
        self.assertEqual(os.listdir(dest), ["keep.txt"])
        self.assertEqual(self.oggs, [])

    def test_a_failed_ogg_leaves_no_half_made_folder(self):
        def broken_ogg(path, length_ms):
            raise RuntimeError("ffmpeg not on PATH")

        with self.assertRaises(RuntimeError):
            I.install(self.src, dest_root=self.dest_root, write_ogg=broken_ogg)
        self.assertFalse(os.path.exists(os.path.join(self.dest_root, "Some Song")))
        # The next try is not refused as "already exists".
        I.install(self.src, dest_root=self.dest_root, write_ogg=self.fake_ogg)

    def test_refuses_a_folder_without_song_length_or_chart(self):
        _write(os.path.join(self.src, G.SONG_INI), b"[song]\nname = x\n")
        with self.assertRaises(I.InstallRefused):
            I.install(self.src, dest_root=self.dest_root, write_ogg=self.fake_ogg)
        _write(os.path.join(self.src, G.SONG_INI), b"[song]\nsong_length = 9\n")
        os.remove(os.path.join(self.src, "notes.mid"))
        with self.assertRaises(I.InstallRefused):
            I.install(self.src, dest_root=self.dest_root, write_ogg=self.fake_ogg)
        self.assertFalse(os.path.exists(self.dest_root))


if __name__ == "__main__":
    unittest.main()
