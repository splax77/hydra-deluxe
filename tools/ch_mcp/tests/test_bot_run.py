"""Tests for bot_run (the bot launcher) and install_chart.

No game, no real game files and no real registry key. The tests that use
_real_env run bot_run's real launcher, clock and sleep on temp folders: its
exe path names a file that is not there, so the real launch fails before any
game starts. _launching_env's exe is this Python, which exits at once. The
rest use the fakes below. The user folders are temp folders, and no Env here
has a reg.exe runner.
"""

from __future__ import annotations

import io
import json
import os
import re
import shlex
import shutil
import stat
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
from tools.ch_mcp import build_stamp as BS  # noqa: E402
from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_mcp import game_files as G  # noqa: E402
from tools.ch_mcp import hydra_end  # noqa: E402
from tools.ch_mcp import install_chart as I  # noqa: E402
from tools.ch_mcp.tests.test_build_stamp import make_built_agent  # noqa: E402
from tools.ch_mcp.tests.test_hydra_end import FAKE  # noqa: E402
from tools.ch_probe import prep_inputs, probe_songs  # noqa: E402
from tools.ch_probe.constants import PROCESS_NAME, ms_to_s  # noqa: E402

# The game's files, by folder; the run never backs up, restores, moves or
# deletes any of them.
GAME_SAVES_IN_DOCUMENTS = (G.SETTINGS, G.SCORESTATS)
GAME_SAVES_IN_LOCALLOW = ("scoredata.bin", "user.bin")
ORIGINAL = {name: f"original {name}".encode() for name in
            GAME_SAVES_IN_DOCUMENTS + GAME_SAVES_IN_LOCALLOW}
# The title prefix install_chart gives a copy, as song.ini bytes.
PREFIX = probe_songs.NAME_PREFIX.encode()
NEW_STATS = json.dumps({"players": [{"is_bot": True}]}).encode()
SONG_LENGTH_MS = 4000  # the fake song's song end in its fake dump

_TESTDATA = os.path.join(_REPO_ROOT, "testdata")
# A corpus chart folder: notes.mid, and a song.ini that says pro_drums = True
# and has its own song_length.
PITCH_DARK = os.path.join(_TESTDATA, "input", "test_pitch_dark_ratio")
# hydra_replay's dump of PITCH_DARK's chart, for its song end. The main session
# makes it with prep_inputs.dump_command's line (see _pitch_dark_dump).
PITCH_DARK_DUMP = os.path.join(_TESTDATA, "replay", "test_pitch_dark_ratio-dump.json")
_CH_RUNS = os.path.join(_TESTDATA, "ch_runs")
# Cut from hydra-data\ch-mcp-runs\2026-10-31\ls-d1a-run1\replies.txt: the
# command line start_run printed when the real game started, and the attach
# that found the song loaded from it.
RECORDED_REPLIES = os.path.join(_CH_RUNS, "ls-d1a-run1", "replies.txt")
# Copied from hydra-data\ch-mcp-runs\2026-11-12\ls-g1-run1\backup-20261112-182956:
# a real backup from before the rulings of 2026-11-13, so its manifest names
# the game's files as well as the registry key, with the copies it took of
# settings.ini and scorestats.json.
REAL_BACKUP = os.path.join(_CH_RUNS, "ls-g1-run1", "backup-20261112-182956")
# scorestats.json as it was before a run (REAL_BACKUP's copy), and as the game
# wrote it after one (copied from hydra-data\ch-mcp-runs\2026-11-12\
# pitch-dark-run1).
STATS_BEFORE_A_RUN = os.path.join(REAL_BACKUP, G.SCORESTATS)
STATS_AFTER_A_RUN = os.path.join(_CH_RUNS, "pitch-dark-run1",
                                 "scorestats-CHON_-_Pitch_Dark_B-20261112-230618.json")


def _real_env(tmp: str, lines: list) -> B.Env:
    """bot_run's own Env (real launcher, clock and sleep) on temp folders. Its
    exe is a path under `tmp` with no file there, so the real launch fails
    before any game starts, and it has no reg.exe runner, so the user's
    registry key is never read or written."""
    docs = os.path.join(tmp, "docs")
    os.makedirs(docs, exist_ok=True)
    return B.Env(exe=os.path.join(tmp, os.path.basename(G.GAME_EXE)), documents_dir=docs,
                 out=lines.append, run_reg=None)


# Windows' own whoami: with no arguments it prints the user's name (into
# _popen_game's DEVNULL stdout) and exits, and it refuses the game's flags
# and exits. It reads and writes nothing else.
_WHOAMI = os.path.join(os.environ.get("SystemRoot", r"C:\Windows"), "System32", "whoami.exe")


def _launching_env(tmp: str, lines: list) -> B.Env:
    """_real_env with whoami as its exe, so the real launch starts a real
    process that exits on its own at once. For a step that needs a launched
    run and does nothing with the game itself."""
    env = _real_env(tmp, lines)
    env.exe = _WHOAMI
    return env


def _reap(test: unittest.TestCase, game) -> None:
    """Wait for a process _launching_env started. It exits on its own at
    once, so the wait ends as soon as it does; THREAD_CAP_S is only the last
    stop."""
    try:
        game._p.wait(timeout=THREAD_CAP_S)
    except subprocess.TimeoutExpired:
        game._p.kill()
        test.fail(f"{_WHOAMI} was still running after {THREAD_CAP_S:g} s")


def _pitch_dark_dump(test: unittest.TestCase) -> str:
    """PITCH_DARK_DUMP, or a failure naming the command that makes it."""
    if not os.path.isfile(PITCH_DARK_DUMP):
        cmd = prep_inputs.dump_command(prep_inputs.DEFAULT_EXE,
                                       I.chart_file(PITCH_DARK), PITCH_DARK_DUMP)
        test.fail(f"{PITCH_DARK_DUMP} is missing; make it with: {subprocess.list2cmdline(cmd)}")
    return PITCH_DARK_DUMP


def _pitch_dark_end(test: unittest.TestCase) -> tuple[float, str]:
    """Hydra's song end for PITCH_DARK, read the way song_length_ms reads it,
    for start_run's song_end."""
    return (hydra_end.song_length_ms(_pitch_dark_dump(test)),
            hydra_end.chart_source_text(I.chart_file(PITCH_DARK)))


def _copy_real_backup(tmp: str) -> tuple[str, dict]:
    """REAL_BACKUP copied into `tmp`, with each file entry's source moved to a
    path under `tmp/sources`, so nothing a restore does can reach the user's
    own files. Returns the folder and each entry's new source by name."""
    folder = os.path.join(tmp, os.path.basename(REAL_BACKUP))
    shutil.copytree(REAL_BACKUP, folder)
    sources_dir = os.path.join(tmp, "sources")
    os.makedirs(sources_dir)
    manifest = os.path.join(folder, G.BACKUP_MANIFEST)
    with open(manifest, encoding="utf-8") as f:
        doc = json.load(f)
    sources = {}
    for entry in doc["files"]:
        if "source" in entry:
            entry["source"] = sources[entry["name"]] = os.path.join(sources_dir, entry["name"])
    with open(manifest, "w", encoding="utf-8") as f:
        json.dump(doc, f, indent=1)
    return folder, sources


def _folder_bytes(folder: str) -> dict:
    return {name: _read(os.path.join(folder, name)) for name in sorted(os.listdir(folder))}


def setUpModule():
    FAKE.start()


def tearDownModule():
    FAKE.stop()


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

    def __init__(self, tmp: str, *, exits_at=None, song_ini="[song]\nname = x\n",
                 length_ms=SONG_LENGTH_MS):
        self.docs = os.path.join(tmp, "docs")
        self.low = os.path.join(tmp, "low")
        self.out_dir = os.path.join(tmp, "out")
        self.song = os.path.join(tmp, "songs", probe_songs.WINDOW_MAP)
        for d in (self.docs, self.low, self.song):
            os.makedirs(d)
        for name, data in ORIGINAL.items():
            _write(self.path(name), data)
        _write(os.path.join(self.song, G.SONG_INI), song_ini.encode())
        # The song's chart, and the song end the fake dump of it holds.
        self.chart = os.path.join(self.song, I.CHART_FILES[0])
        _write(self.chart, b"[Song]\n")
        FAKE.set_end(self.chart, length_ms)
        self.clock = FakeClock()
        self.game = FakeGame(self.clock, exits_at)
        self.launch = FakeLauncher(self.game)
        self.lines = []
        self.env = B.Env(exe=r"C:\fake\Clone Hero.exe", documents_dir=self.docs,
                         launch=self.launch, now=self.clock.now,
                         sleep=self.clock.sleep, stamp=lambda: "20261028-101500",
                         out=self.lines.append, run_reg=None)

    def path(self, name: str) -> str:
        return os.path.join(self.low if name in GAME_SAVES_IN_LOCALLOW else self.docs, name)

    def game_writes_stats_at(self, t: float) -> None:
        def write():
            _write(self.path(G.SCORESTATS), NEW_STATS)
            _write(self.path("scoredata.bin"), b"game wrote this")
        self.clock.events.append((t, write))

    def backup_dir(self) -> str:
        return os.path.join(self.out_dir, "backups", "backup-20261028-101500")

    def assert_files_hold(self, test: unittest.TestCase, data: bytes) -> None:
        """Every game file holds `data`, the bytes the step left in it, and
        no restore line names one."""
        for name in ORIGINAL:
            test.assertEqual(_read(self.path(name)), data, name)
            test.assertFalse(any(line.startswith(f"{name}:") for line in self.lines), name)

    def assert_settings_untouched(self, test: unittest.TestCase) -> None:
        """settings.ini holds its bytes from before the run: nothing in
        bot_run writes it."""
        test.assertEqual(_read(self.path(G.SETTINGS)), ORIGINAL[G.SETTINGS])


class BotRunTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def test_the_command_line_has_song_and_player_only(self):
        # The command line the real game was started with, which loaded its
        # song (RECORDED_REPLIES), is launch_command's for that song and
        # player: the same flags, and nothing else.
        with open(RECORDED_REPLIES, encoding="utf-8") as f:
            text = f.read()
        self.assertIn("songLoaded: true", text)
        commands = []
        for line in text.splitlines():
            tokens = [t.strip('"') for t in shlex.split(line.partition(": ")[2], posix=False)]
            if tokens and os.path.basename(tokens[0]) == PROCESS_NAME:
                commands.append(tokens)
        self.assertEqual(len(commands), 1, text)
        recorded = commands[0]
        self.assertGreaterEqual(len(recorded), 5, recorded)
        self.assertEqual(recorded, B.launch_command(recorded[0], recorded[2], recorded[4]))

    def test_a_relative_song_path_reaches_the_game_absolute(self):
        # The game starts in its own folder, so it must get the full path.
        # The real launch fails (_real_env), after start_run printed the
        # command line it was given.
        lines = []
        env = _real_env(self.tmp, lines)
        song_end = _pitch_dark_end(self)
        old = os.getcwd()
        os.chdir(os.path.dirname(PITCH_DARK))
        try:
            with self.assertRaises(OSError):
                B.start_run(os.path.basename(PITCH_DARK), env=env, song_end=song_end,
                            backup_root=os.path.join(self.tmp, "backups"))
        finally:
            os.chdir(old)
        absolute = subprocess.list2cmdline(
            B.launch_command(env.exe, os.path.abspath(PITCH_DARK), B.DEFAULT_PLAYER))
        relative = subprocess.list2cmdline(
            B.launch_command(env.exe, os.path.basename(PITCH_DARK), B.DEFAULT_PLAYER))
        self.assertTrue(any(line.endswith(absolute) for line in lines), lines)
        self.assertFalse(any(line.endswith(relative) for line in lines), lines)

    def test_the_game_exe_name_is_the_probe_process_name(self):
        self.assertEqual(os.path.basename(G.GAME_EXE), PROCESS_NAME)

    def test_profile_appears_only_when_passed(self):
        # A profile adds one flag and its value after the song and the player,
        # and nothing else changes.
        plain = B.launch_command("exe", "song", B.DEFAULT_PLAYER)
        cmd = B.launch_command("exe", "song", B.DEFAULT_PLAYER, "Tester")
        self.assertEqual(cmd[:len(plain)], plain)
        self.assertEqual(len(cmd), len(plain) + 2)
        self.assertTrue(cmd[-2].startswith("--"), cmd)
        self.assertEqual(cmd[-1], "Tester")
        self.assertNotIn("Tester", plain)

    def test_the_poll_copies_a_changed_stats_file_and_leaves_the_games_own(self):
        # scorestats.json as it was before a real run, then as the game wrote
        # it after one; the poll runs on the real clock.
        lines = []
        env = _real_env(self.tmp, lines)
        stats = os.path.join(env.documents_dir, G.SCORESTATS)
        shutil.copyfile(STATS_BEFORE_A_RUN, stats)
        before = B._file_state(stats)
        shutil.copyfile(STATS_AFTER_A_RUN, stats)
        out_dir = os.path.join(self.tmp, "out")
        os.makedirs(out_dir)
        # A cap of 0 s makes _poll_stats poll once: the change is already
        # there, so a poll that misses it ends the wait at once with no copy.
        copy = B._poll_stats(PITCH_DARK, out_dir, 0, before, env)
        self.assertIsNotNone(copy, lines)
        self.assertEqual(os.path.dirname(copy), out_dir)
        self.assertEqual(_read(copy), _read(STATS_AFTER_A_RUN))
        # The game's own file stays as the game wrote it.
        self.assertEqual(_read(stats), _read(STATS_AFTER_A_RUN))
        # One line, and it names the file that changed and the copy.
        self.assertEqual(len(lines), 1, lines)
        self.assertIn(G.SCORESTATS, lines[0])
        self.assertIn(copy, lines[0])

    def test_run_bot_passes_stop_on_so_a_game_that_stays_is_closed(self):
        r = Rig(self.tmp, exits_at=None)
        r.game_writes_stats_at(5)
        B.run_bot(r.song, r.out_dir, stop=True, env=r.env)
        self.assertTrue(r.game.stopped)
        self.assertIn(B.STOPPED_BY_FLAG_LINE, r.lines)

    def test_the_cap_prints_its_line_runs_the_restore_and_leaves_settings_ini(self):
        r = Rig(self.tmp, exits_at=None)
        cap_s = B.run_cap_s(SONG_LENGTH_MS)
        # The game changes settings.ini during the run but never the stats.
        r.clock.events.append((10, lambda: _write(r.path("settings.ini"), b"the game changed this")))
        self.assertEqual(B.run_bot(r.song, r.out_dir, env=r.env), 1)
        self.assertIn(f"scorestats.json unchanged after {cap_s:.0f} s; bot launch wrote no stats",
                      r.lines)
        self.assertFalse(any("game exited" in line or "still running after" in line
                             for line in r.lines))
        self.assertEqual(r.clock.t, cap_s)
        self.assertFalse(os.path.exists(os.path.join(r.out_dir, "scorestats-Window_Map-20261028-101500.json")))
        self.assertEqual(_read(r.path(G.SETTINGS)), b"the game changed this")
        self.assertIn(f"restoring the backup from {r.backup_dir()}", r.lines)
        self.assertFalse(any(line.startswith(f"{G.SETTINGS}:") for line in r.lines))

    def test_a_restore_failure_stops_with_the_entry_name_and_the_backup_folder(self):
        # A real backup that holds the registry key, restored with no reg.exe
        # runner: restore_registry refuses it, as it does any key it cannot
        # put back.
        backup, _sources = _copy_real_backup(self.tmp)
        with open(os.path.join(backup, G.BACKUP_MANIFEST), encoding="utf-8") as f:
            key_entries = [e["name"] for e in json.load(f)["files"] if "key" in e]
        self.assertEqual(key_entries, [G.REGISTRY_BACKUP])
        lines = []
        self.assertNotEqual(B.restore_only(backup, env=B.Env(out=lines.append, run_reg=None)), 0)
        # One line names the entry that failed, and the last line the folder
        # the backup is still in.
        self.assertEqual(len([line for line in lines if G.REGISTRY_BACKUP in line]), 1, lines)
        self.assertNotIn(G.REGISTRY_BACKUP, lines[-1])
        self.assertIn(backup, lines[-1])
        self.assertFalse(any(G.REGISTRY_LINE_NAME in line for line in lines), lines)

    def test_a_restore_leaves_settings_ini_as_the_run_left_it(self):
        # A real backup that holds a copy of settings.ini; the file at its
        # source is that copy with the run's one setting changed, the way
        # set_game_setting changes it for a run.
        backup, sources = _copy_real_backup(self.tmp)
        settings = sources[G.SETTINGS]
        shutil.copyfile(os.path.join(backup, G.SETTINGS), settings)
        G.set_game_setting(settings, G.PAUSE_ON_FOCUS_LOST, G.PAUSE_ON_FOCUS_LOST_RUN_VALUE)
        self.assertNotEqual(_read(settings), _read(os.path.join(backup, G.SETTINGS)))
        run_left = _read(settings)
        backup_before = _folder_bytes(backup)
        lines = []
        B.restore_only(backup, env=B.Env(out=lines.append, run_reg=None))
        self.assertEqual(_read(settings), run_left)
        self.assertEqual(G.get_game_setting(settings, G.PAUSE_ON_FOCUS_LOST),
                         G.PAUSE_ON_FOCUS_LOST_RUN_VALUE)
        self.assertEqual(_folder_bytes(backup), backup_before)
        # Every other source is still absent: nothing was put back anywhere.
        self.assertEqual(os.listdir(os.path.dirname(settings)), [G.SETTINGS])
        settings_lines = [line for line in lines if G.SETTINGS in line]
        self.assertEqual(len(settings_lines), 1, lines)
        self.assertIn(G.LEFT_AS_SAVED, settings_lines[0])

    def test_the_length_is_hydras_end_from_a_dump_of_the_folders_chart(self):
        # song.ini's own song_length is never read for it: a copy of the
        # corpus folder whose song_length is doubled gets the dump's end.
        song = os.path.join(self.tmp, os.path.basename(PITCH_DARK))
        shutil.copytree(PITCH_DARK, song)
        ini = os.path.join(song, G.SONG_INI)
        with open(ini, encoding="utf-8") as f:
            text, n = re.subn(r"(?m)^(song_length\s*=\s*)(\d+)",
                              lambda m: f"{m.group(1)}{int(m.group(2)) * 2}", f.read())
        self.assertEqual(n, 1)
        with open(ini, "w", encoding="utf-8") as f:
            f.write(text)
        doubled = int(re.search(r"(?m)^song_length\s*=\s*(\d+)", text).group(1))
        dump = _pitch_dark_dump(self)
        made = []

        def dump_from_testdata(chart, out):
            made.append(chart)
            shutil.copyfile(dump, out)

        with mock.patch.object(hydra_end, "make_dump", dump_from_testdata):
            ms, source = B.song_length_ms(song)
        chart = I.chart_file(song)
        self.assertEqual(made, [chart])
        self.assertEqual(ms, hydra_end.song_length_ms(dump))
        self.assertNotEqual(ms, doubled)
        self.assertEqual(source, hydra_end.chart_source_text(chart))

    def test_a_folder_with_no_chart_starts_nothing(self):
        # A song folder holding only the corpus folder's song.ini.
        song = os.path.join(self.tmp, "songs", os.path.basename(PITCH_DARK))
        os.makedirs(song)
        shutil.copyfile(os.path.join(PITCH_DARK, G.SONG_INI), os.path.join(song, G.SONG_INI))
        lines = []
        env = _real_env(self.tmp, lines)
        out_dir = os.path.join(self.tmp, "out")
        with self.assertRaises(B.SongLengthMissing) as caught:
            B.run_bot(song, out_dir, env=env)
        # The message names the folder and every chart file it looked for.
        self.assertIn(song, str(caught.exception))
        for name in I.CHART_FILES:
            self.assertIn(name, str(caught.exception))
        # Nothing was backed up or started, and no line was printed.
        self.assertEqual(lines, [])
        self.assertFalse(os.path.exists(out_dir))

    def test_pro_drums_chart_with_a_drums_player_prints_a_note(self):
        # The corpus folder's song.ini says pro_drums = True. A drums player
        # gets one line before the backup's line, naming the player given
        # and the instrument to pass instead; a pro drums player gets none.
        # The real launch fails after both (_real_env).
        self.assertTrue(G.song_ini_pro_drums(PITCH_DARK))
        song_end = _pitch_dark_end(self)
        difficulty = B.DEFAULT_PLAYER.split(",", 1)[1]
        self.assertNotEqual(B.DEFAULT_PLAYER.split(",")[0], B.PRO_DRUMS_INSTRUMENT)
        for player, notes in ((B.DEFAULT_PLAYER, 1),
                              (f"{B.PRO_DRUMS_INSTRUMENT},{difficulty}", 0)):
            with self.subTest(player=player):
                tmp = os.path.join(self.tmp, str(notes))
                lines = []
                backup_root = os.path.join(tmp, "backups")
                with self.assertRaises(OSError):
                    B.start_run(PITCH_DARK, player=player, backup_root=backup_root,
                                env=_real_env(tmp, lines), song_end=song_end)
                backup_dir = os.path.join(backup_root, os.listdir(backup_root)[0])
                first_backup_line = next(i for i, line in enumerate(lines) if backup_dir in line)
                before_backup = lines[:first_backup_line]
                self.assertEqual(len(before_backup), notes, lines)
                for line in before_backup:
                    self.assertIn(player, line)
                    self.assertIn(B.PRO_DRUMS_INSTRUMENT, line)

    def test_a_drums_chart_with_a_drums_player_prints_no_note(self):
        r = Rig(self.tmp, exits_at=10)
        r.game_writes_stats_at(5)
        B.run_bot(r.song, r.out_dir, env=r.env)
        self.assertFalse(any(line.startswith("note: ") for line in r.lines), r.lines)


class RunStepsTest(unittest.TestCase):
    """The steps the server calls one at a time: start_run, wait_stats and
    finish_run."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def start(self, r: Rig, song_dir) -> B.Run:
        return B.start_run(song_dir, backup_root=os.path.join(r.out_dir, "backups"), env=r.env)

    def test_start_run_backs_up_before_the_launch(self):
        r = Rig(self.tmp, exits_at=None)
        run = self.start(r, r.song)
        self.assertEqual(run.backup_dir, r.backup_dir())
        self.assertEqual(os.listdir(run.backup_dir), [G.BACKUP_MANIFEST])
        self.assertEqual(r.lines[0], f"backup folder: {r.backup_dir()}")
        self.assertFalse(any(line.startswith("restoring") for line in r.lines))

    def test_start_run_keeps_the_stats_state_from_before_the_launch(self):
        r = Rig(self.tmp, exits_at=None)
        before = B._file_state(r.path(G.SCORESTATS))
        run = self.start(r, r.song)
        self.assertIsNotNone(run.before)
        self.assertEqual(run.before, before)

    def test_start_run_launches_the_command_and_records_the_game(self):
        r = Rig(self.tmp, exits_at=None)
        run = self.start(r, r.song)
        self.assertEqual(r.launch.commands, [run.command])
        self.assertEqual(run.command, B.launch_command(r.env.exe, r.song, B.DEFAULT_PLAYER))
        self.assertIs(run.game, r.game)
        self.assertEqual(run.length_ms, SONG_LENGTH_MS)
        self.assertEqual(run.cap_s, B.run_cap_s(run.length_ms))
        self.assertEqual(run.length_source, hydra_end.chart_source_text(r.chart))
        self.assertTrue(any(line.startswith("starting: ") for line in r.lines), r.lines)
        self.assertEqual(r.lines[-1], f"game process id: {r.game.pid}")

    def test_start_run_returns_before_any_sleep(self):
        r = Rig(self.tmp, exits_at=None)
        run = self.start(r, r.song)
        self.assertEqual(r.clock.t, 0.0)
        self.assertEqual(run.launched_at, 0.0)

    def test_run_cap_is_the_song_plus_the_extra_minute(self):
        # The song's seconds plus POLL_CAP_EXTRA_S, for any length.
        for length_ms in (0, 4000, 260504, 1):
            with self.subTest(length_ms=length_ms):
                self.assertAlmostEqual(B.run_cap_s(length_ms) - ms_to_s(length_ms),
                                       B.POLL_CAP_EXTRA_S)

    def test_bot_run_cap_is_run_cap_s_of_hydras_end(self):
        end_ms = 260504
        r = Rig(self.tmp, exits_at=None, length_ms=end_ms)
        run = self.start(r, r.song)
        self.assertEqual(run.length_ms, end_ms)
        self.assertEqual(run.cap_s, B.run_cap_s(end_ms))

    def test_start_run_takes_a_song_end_already_read_without_a_dump(self):
        # A song folder holding only PITCH_DARK's song.ini has no chart, so
        # song_length_ms cannot read an end there. Given Hydra's end for
        # PITCH_DARK's chart (its dump in testdata/replay), start_run reads
        # none and the run's cap and source come from the given end.
        song = os.path.join(self.tmp, "songs", os.path.basename(PITCH_DARK))
        os.makedirs(song)
        shutil.copyfile(os.path.join(PITCH_DARK, G.SONG_INI), os.path.join(song, G.SONG_INI))
        with self.assertRaises(B.SongLengthMissing):
            B.song_length_ms(song)
        given_ms, given_source = _pitch_dark_end(self)
        lines = []
        run = B.start_run(song, env=_launching_env(self.tmp, lines),
                          song_end=(given_ms, given_source),
                          backup_root=os.path.join(self.tmp, "backups"))
        self.addCleanup(_reap, self, run.game)
        self.assertEqual(run.length_ms, given_ms)
        self.assertEqual(run.length_source, given_source)
        self.assertEqual(run.cap_s, B.run_cap_s(given_ms))

    def test_start_run_with_no_song_launches_the_bare_exe_after_a_backup(self):
        # The real launch of a missing exe fails (_real_env) after start_run
        # printed the command it was given and made the backup.
        lines = []
        env = _real_env(self.tmp, lines)
        backup_root = os.path.join(self.tmp, "backups")
        with self.assertRaises(OSError):
            B.start_run(None, env=env, backup_root=backup_root)
        backups = os.listdir(backup_root)
        self.assertEqual(len(backups), 1, backups)
        backup_dir = os.path.join(backup_root, backups[0])
        backup_line = next(i for i, line in enumerate(lines) if backup_dir in line)
        # The one command line printed is the exe alone, after the backup.
        commands = [i for i, line in enumerate(lines)
                    if subprocess.list2cmdline([env.exe]) in line]
        self.assertEqual(len(commands), 1, lines)
        self.assertGreater(commands[0], backup_line, lines)
        self.assertEqual(shlex.split(lines[commands[0]].partition(": ")[2], posix=False),
                         [subprocess.list2cmdline([env.exe])])
        # No player or profile reaches a command with no song.
        for player in (B.DEFAULT_PLAYER, f"{B.PRO_DRUMS_INSTRUMENT},Expert"):
            with self.subTest(player=player):
                self.assertEqual(B.launch_command(env.exe, None, player, "Tester"), [env.exe])

    def test_wait_stats_on_a_no_song_run_says_so_without_sleeping(self):
        # A real launch with no song (_launching_env) gives a run with no cap.
        lines = []
        env = _launching_env(self.tmp, lines)
        run = B.start_run(None, env=env, backup_root=os.path.join(self.tmp, "backups"))
        self.addCleanup(_reap, self, run.game)
        self.assertIsNone(run.cap_s)
        printed = len(lines)
        out_dir = os.path.join(self.tmp, "out")
        start = env.now()
        self.assertIsNone(B.wait_stats(run, out_dir, env))
        # It returns before the poll's first sleep, makes no copy folder and
        # prints one line, which is not the wait's own line.
        self.assertLess(env.now() - start, B.POLL_INTERVAL_S)
        self.assertFalse(os.path.exists(out_dir))
        self.assertEqual(len(lines), printed + 1, lines)
        self.assertNotIn(G.SCORESTATS, lines[-1])

    def test_wait_stats_copies_a_changed_file_and_names_the_seconds(self):
        # scorestats.json as it was before a real run, then as the game wrote
        # it after one; the run is a real launch (_launching_env) with Hydra's
        # end for PITCH_DARK, and the wait runs on the real clock.
        lines = []
        env = _launching_env(self.tmp, lines)
        stats = os.path.join(env.documents_dir, G.SCORESTATS)
        shutil.copyfile(STATS_BEFORE_A_RUN, stats)
        run = B.start_run(PITCH_DARK, env=env, song_end=_pitch_dark_end(self),
                          backup_root=os.path.join(self.tmp, "backups"))
        self.addCleanup(_reap, self, run.game)
        shutil.copyfile(STATS_AFTER_A_RUN, stats)
        out_dir = os.path.join(self.tmp, "out")
        start = env.now()
        copy = B.wait_stats(run, out_dir, env)
        waited = env.now() - start
        self.assertIsNotNone(copy, lines)
        self.assertEqual(os.path.dirname(copy), out_dir)
        self.assertEqual(_read(copy), _read(STATS_AFTER_A_RUN))
        # One line names the file, the copy and the whole seconds the wait
        # took: at least one poll's sleep, and no more than the wall time.
        copied = [line for line in lines if copy in line]
        self.assertEqual(len(copied), 1, lines)
        self.assertIn(G.SCORESTATS, copied[0])
        seconds = [int(n) for n in re.findall(r"\b(\d+) s\b", copied[0])]
        self.assertEqual(len(seconds), 1, copied[0])
        self.assertGreaterEqual(seconds[0], round(B.POLL_INTERVAL_S))
        self.assertLessEqual(seconds[0], round(waited))


class LaunchStepsTest(unittest.TestCase):
    """What start_run does between the backup and the launch, and the
    registry key's backup and restore."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def test_the_servers_env_backs_up_the_key_with_reg_exe(self):
        self.assertIs(B.Env().run_reg, G.run_reg)

    def test_before_launch_runs_after_the_backup_and_before_the_launch(self):
        # The real launch of a missing exe raises (_real_env), and nothing
        # after it runs, so a before_launch that ran at all ran before it.
        lines = []
        backup_root = os.path.join(self.tmp, "backups")
        seen = []

        def before_launch():
            seen.append([os.path.join(backup_root, name) for name in os.listdir(backup_root)])

        with self.assertRaises(OSError):
            B.start_run(PITCH_DARK, env=_real_env(self.tmp, lines), backup_root=backup_root,
                        song_end=_pitch_dark_end(self), before_launch=before_launch)
        # It ran once, with the backup folder already made and named.
        self.assertEqual(len(seen), 1, seen)
        self.assertEqual(len(seen[0]), 1, seen)
        self.assertTrue(os.path.isfile(os.path.join(seen[0][0], G.BACKUP_MANIFEST)))
        self.assertTrue(any(seen[0][0] in line for line in lines), lines)

    def test_a_launch_that_raises_restores_the_backup_and_leaves_what_before_launch_changed(self):
        # A real settings.ini from a run's backup; before_launch changes its
        # run setting the way set_game_setting does for a run, and then the
        # real launch of a missing exe raises (_real_env).
        lines = []
        env = _real_env(self.tmp, lines)
        settings = os.path.join(env.documents_dir, G.SETTINGS)
        shutil.copyfile(os.path.join(REAL_BACKUP, G.SETTINGS), settings)
        backup_root = os.path.join(self.tmp, "backups")

        def before_launch():
            G.set_game_setting(settings, G.PAUSE_ON_FOCUS_LOST, G.PAUSE_ON_FOCUS_LOST_RUN_VALUE)

        with self.assertRaises(OSError):
            B.start_run(PITCH_DARK, env=env, backup_root=backup_root,
                        song_end=_pitch_dark_end(self), before_launch=before_launch)
        # The restore ran on the run's backup, after the launch's line...
        backup_dir = os.path.join(backup_root, os.listdir(backup_root)[0])
        launch_line = next(i for i, line in enumerate(lines)
                           if line.endswith(subprocess.list2cmdline(
                               B.launch_command(env.exe, PITCH_DARK, B.DEFAULT_PLAYER))))
        restores = [i for i, line in enumerate(lines) if backup_dir in line][1:]
        self.assertEqual(len(restores), 1, lines)
        self.assertGreater(restores[0], launch_line, lines)
        # ...and the setting before_launch changed stays changed.
        self.assertEqual(G.get_game_setting(settings, G.PAUSE_ON_FOCUS_LOST),
                         G.PAUSE_ON_FOCUS_LOST_RUN_VALUE)


# A small real program in place of the game: it writes what it was started
# with (its folder, its stdin, and the window flags of its start-up info) to
# the file named by its one argument, then exits.
_STARTED_WITH = r'''
import ctypes, json, os, sys
from ctypes import wintypes

class STARTUPINFOW(ctypes.Structure):
    _fields_ = [("cb", wintypes.DWORD), ("lpReserved", wintypes.LPWSTR),
                ("lpDesktop", wintypes.LPWSTR), ("lpTitle", wintypes.LPWSTR),
                ("dwX", wintypes.DWORD), ("dwY", wintypes.DWORD),
                ("dwXSize", wintypes.DWORD), ("dwYSize", wintypes.DWORD),
                ("dwXCountChars", wintypes.DWORD), ("dwYCountChars", wintypes.DWORD),
                ("dwFillAttribute", wintypes.DWORD), ("dwFlags", wintypes.DWORD),
                ("wShowWindow", wintypes.WORD), ("cbReserved2", wintypes.WORD),
                ("lpReserved2", ctypes.c_void_p), ("hStdInput", wintypes.HANDLE),
                ("hStdOutput", wintypes.HANDLE), ("hStdError", wintypes.HANDLE)]

info = STARTUPINFOW()
info.cb = ctypes.sizeof(info)
ctypes.windll.kernel32.GetStartupInfoW(ctypes.byref(info))
stdin = sys.stdin.read() if sys.stdin is not None else None
with open(sys.argv[1], "w", encoding="utf-8") as f:
    json.dump({"cwd": os.getcwd(), "stdin": stdin, "flags": info.dwFlags,
               "show": info.wShowWindow}, f)
'''


def _start_stand_in(tmp: str, start) -> tuple:
    """Start _STARTED_WITH through `start` (one of bot_run's real launch
    seams). Returns the game it gave back, the folder the program runs from
    and the file it writes."""
    script = os.path.join(tmp, "started_with.py")
    with open(script, "w", encoding="utf-8") as f:
        f.write(_STARTED_WITH)
    out = os.path.join(tmp, "started_with.json")
    return start([sys.executable, "-I", script, out]), os.path.dirname(sys.executable), out


def _started_with(test: unittest.TestCase, game, out: str) -> dict:
    """What the stand-in wrote, once it has exited. The wait ends as soon as
    the program exits, so a program that died early fails at once; THREAD_CAP_S
    is only the last stop."""
    try:
        game._p.wait(timeout=THREAD_CAP_S)
    except subprocess.TimeoutExpired:
        game._p.kill()
        test.fail(f"the stand-in program was still running after {THREAD_CAP_S:g} s")
    test.assertTrue(os.path.isfile(out), "the stand-in exited without writing what it saw")
    with open(out, encoding="utf-8") as f:
        return json.load(f)


class RealLaunchTest(unittest.TestCase):
    """bot_run's real launch seams, on a small real program."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def test_the_game_gets_no_stdin_and_keeps_its_folder(self):
        game, folder, out = _start_stand_in(self.tmp, B._real_launch)
        seen = _started_with(self, game, out)
        self.assertEqual(os.path.normcase(seen["cwd"]), os.path.normcase(folder))
        self.assertEqual(seen["stdin"], "")  # stdin is empty at once: nothing feeds it

    def test_the_game_opens_minimized_without_the_focus(self):
        game, _folder, out = _start_stand_in(self.tmp, B._real_launch)
        seen = _started_with(self, game, out)
        self.assertTrue(seen["flags"] & subprocess.STARTF_USESHOWWINDOW)
        self.assertEqual(seen["show"], B.SW_SHOWMINNOACTIVE)

    def test_the_paused_start_runs_nothing_until_the_resume(self):
        game, folder, out = _start_stand_in(self.tmp, B._real_start_suspended)
        try:
            # Paused: the program has written nothing. resume_main_thread
            # refuses a thread that was not suspended, so its success is the
            # proof the start was paused.
            self.assertFalse(os.path.exists(out))
            B.resume_main_thread(game.pid)
        except BaseException:
            game._p.kill()
            raise
        seen = _started_with(self, game, out)
        self.assertEqual(os.path.normcase(seen["cwd"]), os.path.normcase(folder))
        self.assertEqual(seen["stdin"], "")
        self.assertTrue(seen["flags"] & subprocess.STARTF_USESHOWWINDOW)
        self.assertEqual(seen["show"], B.SW_SHOWMINNOACTIVE)


# The real launch_patch.json, read by bot_run's own reader.
PATCH = B._read_patch(B.LAUNCH_PATCH_FILE)
# The launch agent's log from one real human launch, copied whole from
# hydra-data\ch-mcp-runs\2026-10-31\ls-d0-run1. Its replies in order: the
# start's idle, the arm reply naming the modules loaded, one waiting status,
# the patched status, then two later statuses after the loader ran.
LS_D0_LAUNCH_LOG = os.path.join(_TESTDATA, "ch_runs", "ls-d0-run1",
                                "launch-20261031-141438.jsonl")


def recorded_launch_replies() -> list:
    """The replies in LS_D0_LAUNCH_LOG, each without the id LiveAgent adds."""
    with open(LS_D0_LAUNCH_LOG, encoding="utf-8") as f:
        payloads = [json.loads(line)["payload"] for line in f if line.strip()]
    return [{k: v for k, v in p.items() if k != "id"} for p in payloads]
# How long a test waits on a fake's thread, or on the stand-in program, before
# it fails instead of hanging.
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
    r.env.launch_agent_path = make_built_agent(os.path.join(root, "agent"), "launch")
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

    def test_the_order_is_start_attach_load_arm_resume_status(self):
        # The real paused start, resume, kill and clock, on the stand-in
        # program in place of the game. Only the Frida attach cannot run
        # without the game, so the agent answers with the replies of a real
        # launch (LS_D0_LAUNCH_LOG). The start and resume seams are wrapped
        # only to record when they ran.
        _start, arm_reply, waiting, patched, *_later = recorded_launch_replies()
        agent = FakeLaunchAgent(self.events, statuses=[waiting, patched], arm_reply=arm_reply)
        lines, games, attaches = [], [], []

        def start(cmd):
            self.events.append("start")
            games.append(B._real_start_suspended(cmd))
            return games[-1]

        def resume(pid):
            if not agent.armed:
                raise AssertionError("resumed before the arm reply")
            self.events.append("resume")
            B.resume_main_thread(pid)

        def attach(pid, path, log, out):
            self.events.append("attach")
            attaches.append((pid, path, log))
            return agent

        env = B.Env(out=lines.append, start_process=start, resume_thread=resume,
                    attach_agent=attach,
                    launch_agent_path=make_built_agent(os.path.join(self.tmp, "agent"), "launch"))
        log = os.path.join(self.tmp, "launch.jsonl")
        try:
            launch, _folder, out = _start_stand_in(
                self.tmp, lambda cmd: B.launch_human(cmd, env, log))
        except BaseException:
            for game in games:
                game._p.kill()
            raise
        # The stand-in only runs, writes and exits once resumed.
        _started_with(self, launch.game, out)
        self.assertEqual(self.events, ["start", "attach", "load", "arm", "resume"]
                         + ["status"] * 2)
        self.assertIs(launch.agent, agent)
        self.assertIs(launch.game, games[0])
        self.assertEqual(launch.form, PATCH["launch"])
        self.assertEqual(attaches, [(games[0].pid, env.launch_agent_path, log)])
        self.assertEqual(agent.start_caps, [B.LAUNCH_ATTACH_CAP_S])
        # arm carries the data file's contents, with the arm cap.
        self.assertEqual(agent.requests[0], ({"type": "arm", "patch": PATCH}, B.LAUNCH_ARM_CAP_S))
        # One line names every module the arm reply listed.
        modules = arm_reply["modules"]
        self.assertEqual(len([line for line in lines if all(m in line for m in modules)]), 1, lines)
        # The patched status is reported through its one owner.
        for line in B.launch_status_lines(patched, PATCH):
            self.assertIn(line, lines)

    def test_waiting_statuses_are_polled_every_half_second_until_patched(self):
        agent = FakeLaunchAgent(self.events, statuses=[status("waiting")] * 3 + [PATCHED])
        self.launch(agent)
        self.assertEqual(self.events, ["start", "attach", "load", "arm", "resume"]
                         + ["status"] * 4)
        # Three waiting replies, one poll gap after each.
        waited_s = 3 * B.LAUNCH_POLL_S
        self.assertEqual(self.r.clock.t, waited_s)
        self.assertIn(f"patched {waited_s:.1f} s after the resume", self.r.lines)

    def test_a_patched_arm_reply_still_resumes_and_reads_the_status(self):
        agent = FakeLaunchAgent(self.events, arm_reply=status("patched", modules=["x.dll"]))
        launch, _ = self.launch(agent)
        self.assertEqual(self.events, ["start", "attach", "load", "arm", "resume", "status"])
        self.assertIs(launch.agent, agent)

    def test_a_status_that_stays_waiting_ends_at_the_patch_cap_from_the_resume(self):
        agent = FakeLaunchAgent(self.events, statuses=[status("waiting")])
        resumed = []
        wire_human(self.r, agent)
        fake_resume = self.r.env.resume_thread

        def resume(pid):
            fake_resume(pid)
            resumed.append(self.r.clock.t)

        self.r.env.resume_thread = resume
        resume_at = 7.0  # the resume comes later than time 0
        self.r.clock.t = resume_at
        with self.assertRaises(B.LaunchFailed) as cm:
            B.launch_human(["exe"], self.r.env, os.path.join(self.tmp, "l.jsonl"))
        said = str(cm.exception)
        self.assertIn(f"still said waiting {B.LAUNCH_PATCH_CAP_S:g} s after the resume", said)
        self.assertEqual(resumed, [resume_at])
        # The cap counts from the resume, not from the start.
        self.assertEqual(self.r.clock.t, resume_at + B.LAUNCH_PATCH_CAP_S)
        statuses = [cap for message, cap in agent.requests if message["type"] == "status"]
        self.assertEqual(len(statuses), round(B.LAUNCH_PATCH_CAP_S / B.LAUNCH_POLL_S))
        # Each status waits at most what is left of the cap.
        self.assertEqual(statuses[0], B.LAUNCH_PATCH_CAP_S)
        self.assertEqual(self.events[-2:], ["kill", "detach"])

    def test_the_spawn_form_takes_the_spawn_seam_and_resumes_through_the_device(self):
        agent = FakeLaunchAgent(self.events)
        launch, _ = self.launch(agent, {**PATCH, "launch": "spawn"})
        self.assertEqual(self.events,
                         ["spawn", "attach", "load", "arm", "resume spawned", "status"])
        self.assertEqual(launch.form, "spawn")

    def refused_before_the_start(self, env) -> str:
        """launch_human on the real Env `env`, which must refuse before its
        real start runs: a start would raise OSError on the missing "exe",
        not LaunchFailed. Returns the refusal."""
        with self.assertRaises(B.LaunchFailed) as cm:
            B.launch_human(["exe"], env, os.path.join(self.tmp, "l.jsonl"))
        return str(cm.exception)

    def test_a_bad_launch_form_or_a_missing_agent_starts_nothing(self):
        # A launch_patch.json is input the real reader reads: the real one,
        # with its form changed.
        bad = os.path.join(self.tmp, "bad_patch.json")
        _write(bad, json.dumps({**PATCH, "launch": "late"}).encode())
        agent_path = make_built_agent(os.path.join(self.tmp, "agent"), "launch")
        said = self.refused_before_the_start(
            B.Env(out=[].append, launch_patch_path=bad, launch_agent_path=agent_path))
        # The refusal names the bad form and every form the code takes.
        self.assertIn("'late'", said)
        for form in B.LAUNCH_FORMS:
            self.assertIn(form, said)
        os.remove(agent_path)
        said = self.refused_before_the_start(B.Env(out=[].append, launch_agent_path=agent_path))
        self.assertIn(BS.build_command(agent_path), said)

    def test_a_stale_stamp_refuses_naming_the_file_and_the_build_command(self):
        path = make_built_agent(os.path.join(self.tmp, "agent"), "launch")
        env = B.Env(out=[].append, launch_agent_path=path)
        source = os.path.join(os.path.dirname(path), BS.sources_of(path)[0])
        with open(source, "a", encoding="utf-8") as f:
            f.write("// an edit after the build\n")
        said = self.refused_before_the_start(env)
        # The refusal carries build_stamp's own reason, which names the
        # bundle, the changed source and the build command.
        self.assertIn(BS.check(path), said)
        self.assertIn(path, said)
        self.assertIn(os.path.basename(source), said)
        self.assertIn(BS.build_command(path), said)
        os.remove(BS.stamp_path(path))
        said = self.refused_before_the_start(env)
        self.assertIn(BS.check(path), said)
        self.assertIn(BS.build_command(path), said)

    def test_the_real_env_uses_the_real_seams_and_files(self):
        env = B.Env()
        self.assertIs(env.start_process, B._real_start_suspended)
        self.assertIs(env.resume_thread, B.resume_main_thread)
        self.assertEqual(env.launch_agent_path, B.LAUNCH_AGENT_FILE)
        self.assertEqual(env.launch_patch_path, B.LAUNCH_PATCH_FILE)
        # The patch file the real Env names is one the real reader takes.
        self.assertIn(B._read_patch(env.launch_patch_path)["launch"], B.LAUNCH_FORMS)

    def test_the_real_attach_builds_a_live_agent_for_the_pid(self):
        # The real LiveAgent; it attaches only on start(), which this test
        # never calls, so the pid needs no process behind it.
        out = [].append
        agent = B._real_attach_agent(1234, "launch.js", "log.jsonl", out)
        self.assertIsInstance(agent, FA.LiveAgent)
        self.assertEqual((agent.pid, agent.agent_path, agent.log_path), (1234, "launch.js", "log.jsonl"))
        self.assertFalse(agent.alive)

    def test_resume_main_thread_refuses_a_process_with_no_thread(self):
        # A Toolhelp snapshot only reads; no process id is a multiple of 4
        # this high.
        pid = 0xFFFFFFF0
        with self.assertRaises(OSError) as cm:
            B.resume_main_thread(pid)
        self.assertIn(f"process {pid} ", str(cm.exception))

    def test_the_toolhelp_and_thread_time_calls_find_this_processs_main_thread(self):
        # Read-only calls on the test's own process: they check the struct
        # layouts and argtypes against the real kernel32. ResumeThread itself
        # needs a suspended process, which only a launch makes (LS-D0).
        k32 = B._kernel32()
        mine = B._thread_ids(k32, os.getpid())
        self.assertIn(threading.get_native_id(), mine)
        self.assertEqual(B._oldest_thread_id(k32, os.getpid()), threading.main_thread().native_id)

    def test_a_spawned_games_process_handle_reports_a_running_process(self):
        # Read-only: asks about the test's own process and never stops it.
        process = B._PidProcess(os.getpid())
        self.assertEqual(process.pid, os.getpid())
        self.assertTrue(process.is_running())

    def test_a_run_leaves_the_game_exe_as_it_was(self):
        # A run on a game folder of its own: the exe's bytes and modified
        # time are the same after the run, its restore included.
        with tempfile.TemporaryDirectory() as tmp:
            r = Rig(tmp, exits_at=10)
            r.game_writes_stats_at(5)
            r.env.exe = os.path.join(tmp, "game", os.path.basename(G.GAME_EXE))
            os.makedirs(os.path.dirname(r.env.exe))
            _write(r.env.exe, b"MZ the game")
            before = (_read(r.env.exe), os.stat(r.env.exe).st_mtime_ns)
            self.assertEqual(B.run_bot(r.song, r.out_dir, env=r.env), 0)
            self.assertEqual(r.launch.commands[0][0], r.env.exe)
            self.assertEqual((_read(r.env.exe), os.stat(r.env.exe).st_mtime_ns), before)


class LoaderLineTest(unittest.TestCase):
    """The three words of ch_status's loader line."""

    def test_the_three_words(self):
        # The statuses of one real launch (LS_D0_LAUNCH_LOG): waiting, then
        # patched before the loader ran, then patched after it ran once.
        _start, _arm, waiting, patched, loader_ran, *_rest = recorded_launch_replies()
        self.assertEqual(B.loader_ran(loader_ran, PATCH), B.LOADER_YES)
        self.assertEqual(B.loader_ran(patched, PATCH), B.LOADER_NOT_YET)
        self.assertEqual(B.loader_ran(waiting, PATCH), B.LOADER_NOT_YET)
        # The same run read against a patch whose write bytes are the ones
        # the patch site held before (launch_patch.json's `expect`): the
        # loader read bytes other than that patch writes.
        other_write = {**PATCH, "write": PATCH["expect"]}
        self.assertEqual(B.loader_ran(loader_ran, other_write), B.LOADER_NO)

    def test_ch_status_warns_when_the_loader_read_no(self):
        def lines_for(bytes_seen):
            agent = FakeLaunchAgent([])
            agent.statuses = [{**PATCHED, "loader_runs": 1, "bytes_at_first_run": bytes_seen}]
            launch = B.HumanLaunch(game=None, agent=agent, patch=PATCH, form="suspended")
            return B.request_launch_status(launch, 1.0)

        said_no = lines_for("BA 01 00 00 00")
        self.assertIn(B.LOADER_LINE + "no", said_no)
        self.assertEqual(said_no[-1], B.LOADER_NO_WARNING)
        said_yes = lines_for(PATCH["write"])
        self.assertIn(B.LOADER_LINE + "yes", said_yes)
        self.assertEqual([line for line in said_yes if line.startswith("warning")], [])

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

    def test_a_failed_human_launch_restores_the_backup(self):
        agent = FakeLaunchAgent(self.events, statuses=[status("refused")])
        with self.assertRaises(B.LaunchFailed):
            self.start(self.r.song, agent)
        self.assertIn(f"restoring the backup from {self.r.backup_dir()}", self.r.lines)
        self.r.assert_settings_untouched(self)


class InstallChartTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name
        self.src = os.path.join(self.tmp, "input", "Some Song")
        self.dest_root = os.path.join(self.tmp, "Hydra Probe")
        os.makedirs(self.src)
        _write(os.path.join(self.src, G.SONG_INI), b"[song]\nsong_length = 90000\n")
        _write(os.path.join(self.src, "notes.mid"), b"MThd")
        # Hydra's song end for the chart, in its fake dump. song.ini's own
        # song_length lines in these tests are only bytes the copy keeps.
        FAKE.set_end(os.path.join(self.src, "notes.mid"), 90000)
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

    def test_the_silent_audio_is_hydras_end_not_song_ini(self):
        _write(os.path.join(self.src, G.SONG_INI), b"[song]\nsong_length = 9\n")
        I.install(self.src, dest_root=self.dest_root, write_ogg=self.fake_ogg)
        self.assertEqual([ms for _path, ms in self.oggs], [90000])

    def refused(self) -> str:
        """Install the source, which must be refused with nothing made."""
        with self.assertRaises(I.InstallRefused) as cm:
            I.install(self.src, dest_root=self.dest_root, write_ogg=self.fake_ogg)
        self.assertFalse(os.path.exists(self.dest_root))
        self.assertEqual(self.oggs, [])
        return str(cm.exception)

    def test_refuses_a_folder_whose_chart_has_no_song_end(self):
        FAKE.set_end(os.path.join(self.src, "notes.mid"), None)
        self.assertIn(f"{hydra_end.FIELD} is null", self.refused())

    def test_refuses_a_folder_without_a_chart(self):
        os.remove(os.path.join(self.src, "notes.mid"))
        self.assertIn(f"has no {' or '.join(I.CHART_FILES)}", self.refused())

    def test_refuses_a_folder_without_a_song_ini(self):
        os.remove(os.path.join(self.src, G.SONG_INI))
        self.assertIn(f"has no {G.SONG_INI}", self.refused())

    def test_install_refuses_a_name_that_is_not_a_plain_folder_name(self):
        outside = os.path.join(self.tmp, "outside")
        for name in ("..", os.path.join("..", "outside"), "a/b", "a\\b", "", ".",
                     os.path.join(self.tmp, "outside")):
            with self.subTest(name=name):
                with self.assertRaises(I.InstallRefused) as cm:
                    I.install(self.src, dest_root=self.dest_root, name=name,
                              write_ogg=self.fake_ogg)
                # The refusal names the name it refused.
                self.assertTrue(str(cm.exception).startswith(repr(name)), str(cm.exception))
        self.assertFalse(os.path.exists(self.dest_root))
        self.assertFalse(os.path.exists(outside))
        self.assertEqual(self.oggs, [])
        self.assertEqual(FAKE.charts.count(os.path.join(self.src, "notes.mid")), 0)

    def test_install_refuses_a_name_as_a_refused_line(self):
        out = io.StringIO()
        with redirect_stdout(out):
            code = I.main([self.src, "--dest", self.dest_root, "--name", ".."])
        self.assertEqual(code, 2)
        self.assertTrue(out.getvalue().startswith("refused: '..' is not a plain folder name"),
                        out.getvalue())

    def test_install_copies_no_other_audio(self):
        for name in (probe_songs.SONG_OGG, "drums.ogg", "guitar.opus", "song.mp3", "crowd.wav"):
            _write(os.path.join(self.src, name), b"audio")
        I.install(self.src, dest_root=self.dest_root, write_ogg=self.fake_ogg)
        dest = os.path.join(self.dest_root, "Some Song")
        # The song.ogg there is the silent one write_ogg makes, which the fake
        # here does not write; nothing of the source's audio is copied.
        self.assertEqual(sorted(os.listdir(dest)), sorted([G.SONG_INI, "notes.mid"]))
        self.assertEqual(self.oggs, [(os.path.join(dest, probe_songs.SONG_OGG), 90000)])

    # -- the copy's title -----------------------------------------------------------

    def installed_ini(self, source_ini: bytes, *, name=None) -> bytes:
        """The copy's song.ini after installing a source with `source_ini`."""
        _write(os.path.join(self.src, G.SONG_INI), source_ini)
        I.install(self.src, dest_root=self.dest_root, name=name, write_ogg=self.fake_ogg)
        return _read(os.path.join(self.dest_root, name or "Some Song", G.SONG_INI))

    def test_the_copys_title_gets_the_probe_prefix(self):
        ini = self.installed_ini(b"[song]\nname = Ladies Talk [Kiyo Sen] (Drumeo Performance)\n"
                                 b"song_length = 90000\n")
        title = "Ladies Talk [Kiyo Sen] (Drumeo Performance)"
        self.assertEqual(ini, b"[song]\nname = " + PREFIX + title.encode()
                         + b"\nsong_length = 90000\n")
        self.assertEqual(G.read_song_ini(os.path.join(self.dest_root, "Some Song"))["name"],
                         probe_songs.NAME_PREFIX + title)

    def test_every_other_byte_of_the_song_ini_stays(self):
        ini = self.installed_ini(b"\xef\xbb\xbf; name = a comment\r\n"
                                 b"[Song]\r\n"
                                 b"artist  =  Caf\xe9 \r\n"
                                 b"# name = another comment\r\n"
                                 b"NAME=Caf\xe9 Title  \r\n"
                                 b"\r\n"
                                 b"song_length = 90000\r\n"
                                 b"[other]\r\n"
                                 b"name = not the title\r\n"
                                 b"last line = no ending")
        self.assertEqual(ini, b"\xef\xbb\xbf; name = a comment\r\n"
                              b"[Song]\r\n"
                              b"artist  =  Caf\xe9 \r\n"
                              b"# name = another comment\r\n"
                              b"NAME=" + PREFIX + b"Caf\xe9 Title  \r\n"
                              b"\r\n"
                              b"song_length = 90000\r\n"
                              b"[other]\r\n"
                              b"name = not the title\r\n"
                              b"last line = no ending")

    def test_the_source_chart_is_never_touched(self):
        source = b"[song]\r\nname = Real Title\r\nsong_length = 90000\r\n"
        self.installed_ini(source)
        self.assertEqual(_read(os.path.join(self.src, G.SONG_INI)), source)

    def test_a_title_with_the_prefix_is_not_doubled(self):
        source = b"[song]\nname = Hydra Probe - Real Title\nsong_length = 90000\n"
        self.assertEqual(self.installed_ini(source), source)

    def test_a_name_before_any_section_is_the_title(self):
        self.assertEqual(self.installed_ini(b"name = Real Title\nsong_length = 90000\n"),
                         b"name = Hydra Probe - Real Title\nsong_length = 90000\n")

    def test_every_name_line_is_marked(self):
        # game_files reads the first of two name lines and Hydra the last.
        self.assertEqual(self.installed_ini(b"[song]\nname = First\nname = Last\n"
                                            b"song_length = 90000\n"),
                         b"[song]\nname = Hydra Probe - First\nname = Hydra Probe - Last\n"
                         b"song_length = 90000\n")

    def test_no_name_key_gets_one_from_the_folder_name(self):
        self.assertEqual(self.installed_ini(b"[song]\nsong_length = 90000\n"),
                         b"[song]\nname = Hydra Probe - Some Song\nsong_length = 90000\n")

    def test_no_name_key_uses_the_installed_folders_name(self):
        self.assertEqual(self.installed_ini(b"[song]\nsong_length = 90000\n", name="Other"),
                         b"[song]\nname = Hydra Probe - Other\nsong_length = 90000\n")

    def test_an_empty_name_counts_as_no_name(self):
        self.assertEqual(self.installed_ini(b"[song]\nname = \nsong_length = 90000\n"),
                         b"[song]\nname = Hydra Probe - Some Song\nsong_length = 90000\n")

    def test_a_new_name_line_uses_the_files_line_ending(self):
        self.assertEqual(self.installed_ini(b"\xef\xbb\xbf[song]\r\nsong_length = 90000\r\n"),
                         b"\xef\xbb\xbf[song]\r\nname = Hydra Probe - Some Song\r\n"
                         b"song_length = 90000\r\n")

    def test_a_new_name_line_goes_in_the_song_section_when_there_is_one(self):
        # Hydra reads only the [song] section.
        self.assertEqual(self.installed_ini(b"song_length = 90000\n[song]\nartist = x\n"),
                         b"song_length = 90000\n[song]\nname = Hydra Probe - Some Song\n"
                         b"artist = x\n")

    def test_a_new_name_line_after_a_last_line_with_no_ending(self):
        self.assertEqual(self.installed_ini(b"\xef\xbb\xbfsong_length = 90000"),
                         b"\xef\xbb\xbfsong_length = 90000\nname = Hydra Probe - Some Song")

    def test_a_read_only_song_ini_is_marked_in_the_copy_and_stays_read_only_at_the_source(self):
        path = os.path.join(self.src, G.SONG_INI)
        _write(path, b"[song]\nname = Real Title\nsong_length = 90000\n")
        os.chmod(path, stat.S_IREAD)
        try:
            I.install(self.src, dest_root=self.dest_root, write_ogg=self.fake_ogg)
            self.assertFalse(os.access(path, os.W_OK))
        finally:
            os.chmod(path, stat.S_IREAD | stat.S_IWRITE)
        self.assertEqual(_read(os.path.join(self.dest_root, "Some Song", G.SONG_INI)),
                         b"[song]\nname = " + PREFIX + b"Real Title\nsong_length = 90000\n")

    def test_a_failed_title_leaves_no_half_made_folder(self):
        with mock.patch.object(G, "set_song_title", side_effect=OSError("disk full")):
            with self.assertRaises(OSError):
                I.install(self.src, dest_root=self.dest_root, write_ogg=self.fake_ogg)
        self.assertFalse(os.path.exists(os.path.join(self.dest_root, "Some Song")))

    def test_a_song_ini_with_nowhere_for_a_title_is_refused_by_the_writer(self):
        _write(os.path.join(self.src, G.SONG_INI), b"[other]\nsong_length = 90000\n")
        with self.assertRaises(G.SettingNotFound):
            G.set_song_title(self.src, lambda title: title)


if __name__ == "__main__":
    unittest.main()
