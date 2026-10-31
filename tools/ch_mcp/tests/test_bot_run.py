"""Tests for bot_run (the bot launcher) and install_chart.

No game and no real user files: the launch, the clock and the process check
are the fakes below, and the user folders are temp folders.
"""

from __future__ import annotations

import io
import json
import os
import subprocess
import sys
import tempfile
import unittest
from contextlib import redirect_stdout
from unittest import mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import bot_run as B  # noqa: E402
from tools.ch_mcp import game_files as G  # noqa: E402
from tools.ch_mcp import install_chart as I  # noqa: E402
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
                         out=self.lines.append)

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

        # The step: the game rewrites all five files.
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
        self.assertEqual(FakePopen.calls, [(cmd, {
            "cwd": os.path.dirname(cmd[0]),
            "stdin": subprocess.DEVNULL,
            "stdout": subprocess.DEVNULL})])


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
