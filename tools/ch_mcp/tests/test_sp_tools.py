"""Tests for the Star Power tools (tools/ch_mcp/sp_tools.py).

No game: SpToolsOnARecordedRunTest runs on the real GameTools, the real watch
and the real clock, with only the game's reads played back from a real
recorded run (RecordedRun). GapLineOnARecordedRunTest reads a real watch's
rows.
"""

from __future__ import annotations

import csv
import dataclasses
import functools
import json
import math
import os
import shutil
import sys
import tempfile
import threading
import time
import unittest
import uuid
from unittest import mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import bot_run, game_files, game_tools, hydra_end, sp_tools  # noqa: E402
from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import engine  # noqa: E402
from tools.ch_probe import process  # noqa: E402
from tools.ch_probe.experiments import sp_compare, sp_watch  # noqa: E402
from tools.ch_probe.experiments.overlay_logs import WATCH_COLUMNS  # noqa: E402
from tools.ch_probe.experiments import play_chart  # noqa: E402
from tools.ch_probe.experiments.score_compare import stop_check  # noqa: E402
from tools.ch_mcp.tests.test_hydra_end import FAKE  # noqa: E402
# A real watch piece of a corpus chart's run, and Hydra's files for that chart,
# from the test file that owns them.
from tools.ch_probe.tests.test_sp_compare import (  # noqa: E402
    _PITCH_DARK_CHART, _RISE_CSV, _midi_resolution, _replay_file, _within, write_press_log)


def setUpModule():
    # The song ends come from fake dumps, because no test runs hydra_replay.
    FAKE.start()


def tearDownModule():
    FAKE.stop()


WAIT_S = 5.0
# A watch cap short enough to run out well inside WAIT_S. It is a test input,
# not a value the tools decide.
SHORT_CAP_S = WAIT_S / 25


# -- the watch on a recorded run ---------------------------------------------------------

_CH_RUNS = os.path.join(_REPO_ROOT, "testdata", "ch_runs")
# A real Star Power watch, cut to the rows these tests need: the first row of
# LS-G3's watch, then the three reads before the run's first activation, the
# read that shows it and the two after it. Copied from lines 2 and 20842-20847
# of hydra-data\ch-mcp-runs\2026-11-12\ls-g3-run1\sp_watch-1.csv.
_LS_G3_WATCH = os.path.join(_CH_RUNS, "ls-g3-run1", "sp_watch-1.csv")
# The reply line in which LS-D2B's watch named the engine its search found.
# Copied from line 13 of hydra-data\ch-mcp-runs\2026-11-06\ls-d2b-run1\replies.txt.
_LS_D2B_REPLIES = os.path.join(_CH_RUNS, "ls-d2b-run1", "replies.txt")
# Hydra's dump of a corpus chart, the file hydra_end.make_dump writes for it.
# The main session makes it with:
#   build-cpp\Release\hydra_replay.exe dump --chart testdata\input\test_pitch_dark_ratio\notes.mid
#     --cap 4 --ms 10 --out testdata\replay\test_pitch_dark_ratio-dump.json
_PITCH_DARK_FOLDER = os.path.join(_REPO_ROOT, "testdata", "input", "test_pitch_dark_ratio")
_PITCH_DARK_DUMP = os.path.join(_REPO_ROOT, "testdata", "replay", "test_pitch_dark_ratio-dump.json")
# Hydra's schedule of the same chart, beside its dump.
_PITCH_DARK_SCHEDULE = os.path.join(_REPO_ROOT, "testdata", "replay",
                                    "test_pitch_dark_ratio-schedule.json")


def recorded_snapshots():
    """Each row of the LS-G3 recording as the Snapshot the watch read then:
    every field the watch logs, as sp_compare.load_watch_rows reads it, and
    the fields it does not log left zero."""
    base = engine.decode_snapshot(bytes(engine.SNAPSHOT_SIZE))
    names = {f.name for f in dataclasses.fields(engine.Snapshot)}
    return [dataclasses.replace(base, **{c: v for c, v in row.items() if c in names})
            for row in sp_compare.load_watch_rows(_LS_G3_WATCH)]


def recorded_engine() -> int:
    """The engine address LS-D2B's reply named."""
    with open(_LS_D2B_REPLIES, encoding="utf-8") as f:
        return int(f.readline().split("engine: ", 1)[1], 16)


def logged(rows):
    """Watch rows without their wall time, which differs between two runs of
    the same reads."""
    return [{c: v for c, v in row.items() if c != sp_watch.WALL_COLUMN} for row in rows]


def csv_named_in(reply: str) -> str:
    """The watch CSV a reply names on its csv line."""
    line = next(line for line in reply.splitlines() if line.startswith("csv: "))
    return line[len("csv: "):].split(" (not written", 1)[0]


def wait_until(done, cap_s: float) -> bool:
    """Whether `done()` came true within `cap_s` seconds of wall time."""
    deadline = time.monotonic() + cap_s
    while not done():
        if time.monotonic() >= deadline:
            return False
        time.sleep(0.001)
    return True


class RecordedRun:
    """The game side of a watch, played back from the LS-G3 recording. The
    opener hands this back as the open process, its search answers with the
    engine address LS-D2B recorded, and its reader serves the recorded reads
    in order, then the last one again, as a game whose song has stopped does.
    `done` is set on the first repeat, so the watch has read every recorded
    row by then. No game memory is read."""

    def __init__(self):
        self.snaps = recorded_snapshots()
        self.engine_ptr = recorded_engine()
        self.served = 0
        self.done = threading.Event()
        self.closed = False

    def verify_targets(self):
        # The recording was taken on a build that passed this check.
        pass

    def close(self):
        self.closed = True

    def find_engine(self, proc):
        return self.engine_ptr

    def reader(self, proc, engine_ptr):
        def serve():
            i = min(self.served, len(self.snaps) - 1)
            self.served += 1
            if self.served > len(self.snaps):
                self.done.set()
            return self.snaps[i]
        return serve


class SpToolsOnARecordedRunTest(unittest.TestCase):
    """SpTools on the real GameTools, over a session folder in a temp folder,
    with the real watch on the real clock. Only the game's reads are played
    back, from RecordedRun."""

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.lock = threading.RLock()
        # run_reg=None keeps the Rewired key out of the backup (bot_run.Env).
        # The driver is never pressed: no tool here plays a chart.
        self.game = game_tools.GameTools(bot_run.Env(run_reg=None), self.lock,
                                         self.tmp.name, driver=object())
        self.run = RecordedRun()
        self.at_open = None

    def tearDown(self):
        self.game.ch_stop()
        self.tmp.cleanup()

    def opener(self, **kwargs):
        # The backup folders under the temp root when the game was opened.
        # It walks the folder rather than ask GameTools, whose lock
        # watch_for_run holds while this runs.
        self.at_open = (kwargs, sorted(
            os.path.join(parent, name) for parent, names, _files in os.walk(self.tmp.name)
            for name in names if name.startswith(game_files.BACKUP_PREFIX)))
        return self.run

    def tools(self, **kwargs):
        kwargs.setdefault("find_engine", self.run.find_engine)
        return sp_tools.SpTools(self.game, self.lock, opener=self.opener,
                                reader=self.run.reader, **kwargs)

    def watched(self, tools, *args):
        """Start a watch, wait for it to read every recorded row, and stop it.
        Returns the start's reply and the stop's."""
        reply = tools.ch_sp_watch_start(*args)
        self.assertTrue(self.run.done.wait(WAIT_S),
                        f"the watch did not read past the recording in {WAIT_S:g} s:\n{reply}")
        return reply, tools.ch_sp_watch_stop()

    def assert_names_the_recorded_flip(self, text):
        """`text` holds the activation line of LS-G3's first activation: its
        flip clock, the multiplier before it and on it, and the end time the
        flip row reads."""
        rows = sp_compare.load_watch_rows(_LS_G3_WATCH)
        flip = next(i for i, row in enumerate(rows) if row["sp_active_byte"])
        clock = sp_compare.format_ms(C.s_to_ms(rows[flip]["clock_s"]))
        end = sp_compare.format_ms(C.s_to_ms(rows[flip]["sp_end_s"]))
        line = next((line for line in text.splitlines()
                     if line.startswith(f"  activation 1 at {clock}:")), None)
        self.assertIsNotNone(line, f"no activation at {clock}:\n{text}")
        self.assertIn(f"multiplier {rows[flip - 1]['sp_mult_u32']} before, "
                      f"{rows[flip]['sp_mult_u32']} on the flip row", line)
        self.assertIn(f"end time at the flip {end}", line)

    # -- start and stop ----------------------------------------------------------------

    def test_start_then_stop_ends_with_stopped_and_writes_the_csv(self):
        reply, stop = self.watched(self.tools())
        csv_path = csv_named_in(reply)
        self.assertEqual(csv_named_in(stop), csv_path)
        with open(csv_path, newline="", encoding="utf-8") as f:
            self.assertEqual(tuple(next(csv.reader(f))), WATCH_COLUMNS)
        written = sp_compare.load_watch_rows(csv_path)
        recorded = sp_compare.load_watch_rows(_LS_G3_WATCH)
        # Every recorded read differs from the one before it, so each is
        # written once, then the last read the watch made is written again.
        self.assertEqual(logged(written), logged(recorded) + logged(recorded[-1:]))
        self.assertIn(f"watcher ended: {sp_watch.STOPPED_REASON}; rows written {len(written)};",
                      stop)
        self.assertTrue(self.run.closed)

    def test_the_backup_comes_before_the_game_is_opened(self):
        reply, _stop = self.watched(self.tools())
        kwargs, backups = self.at_open
        self.assertEqual(kwargs, {"read_only": True})
        self.assertEqual(len(backups), 1, backups)
        # The reply's first line names the backup the open found already made.
        self.assertIn(backups[0], reply.splitlines()[0])

    def test_a_second_start_while_running_is_refused(self):
        tools = self.tools()
        tools.ch_sp_watch_start()
        self.assertTrue(self.run.done.wait(WAIT_S))
        self.assertEqual(tools.ch_sp_watch_start(), sp_tools.WATCH_RUNNING)
        tools.ch_sp_watch_stop()

    def test_watch_for_run_is_refused_while_a_watch_runs(self):
        tools = self.tools()
        tools.ch_sp_watch_start()
        self.assertTrue(self.run.done.wait(WAIT_S))
        self.assertTrue(tools.watch_running())
        opened = self.at_open
        self.assertEqual(tools.watch_for_run(SHORT_CAP_S, self.run.find_engine),
                         (None, sp_tools.WATCH_RUNNING))
        # The refusal started no second watch: the game was opened once, by
        # the start, and that watch still runs.
        self.assertIs(self.at_open, opened)
        self.assertTrue(tools.watch_running())
        tools.ch_sp_watch_stop()

    def test_ch_stop_stops_the_watch(self):
        tools = self.tools()
        tools.ch_sp_watch_start()
        self.assertTrue(self.run.done.wait(WAIT_S))
        said = self.game.ch_stop()
        self.assertIn(f"watcher ended: {sp_watch.STOPPED_REASON}", said)
        self.assertFalse(tools.watch_running())
        self.assertIn(f"watcher ended: {sp_watch.STOPPED_REASON}", tools.ch_sp_watch_status())
        # A stopped watch says nothing at the next ch_stop.
        self.assertNotIn("watcher ended", self.game.ch_stop())

    # -- the run cap -------------------------------------------------------------------

    def test_a_song_folder_takes_its_cap_from_hydras_end(self):
        if not os.path.exists(_PITCH_DARK_DUMP):
            self.fail(f"{_PITCH_DARK_DUMP} is missing; make it with the hydra_replay "
                      "command beside _PITCH_DARK_DUMP")
        dumps = []

        def checked_in_dump(chart, dump):
            dumps.append(chart)
            shutil.copyfile(_PITCH_DARK_DUMP, dump)

        with mock.patch.object(hydra_end, "make_dump", checked_in_dump):
            reply, _stop = self.watched(self.tools(), _PITCH_DARK_FOLDER)
        cap = game_tools.cap_for_end(hydra_end.song_length_ms(_PITCH_DARK_DUMP))
        cap_line = next(line for line in reply.splitlines() if line.startswith("run cap: "))
        self.assertTrue(cap_line.startswith(f"run cap: {cap:g} s "), cap_line)
        self.assertIn(_PITCH_DARK_FOLDER, cap_line)
        # One dump of the folder's chart gives both the cap and the gap
        # line's end.
        self.assertEqual(len(dumps), 1)

    def test_no_song_folder_takes_the_watcher_cap(self):
        caps = []

        def watch(*args, **kwargs):
            caps.append(kwargs["cap_s"])
            return sp_watch.watch(*args, **kwargs)

        with mock.patch.object(self.game, "cap_for_end",
                               wraps=self.game.cap_for_end) as cap_for_end:
            reply, _stop = self.watched(self.tools(watch=watch))
        self.assertIn(f"run cap: {sp_watch.RUN_CAP_S:g} s "
                      "(sp_watch.RUN_CAP_S; no song folder given)", reply.splitlines())
        # The game is not asked for a cap, and the watch ran with the default.
        cap_for_end.assert_not_called()
        self.assertEqual(caps, [sp_watch.RUN_CAP_S])

    def test_run_cap_line_says_default_when_no_song_length(self):
        # A folder with no chart file: bot_run.song_length_ms names no end,
        # so game_tools.cap_for_end gives play_chart's default cap.
        folder = os.path.join(self.tmp.name, "no-length")
        os.makedirs(folder)
        reply, _stop = self.watched(self.tools(), folder)
        self.assertIn(f"run cap: {play_chart.RUN_CAP_S:g} s (default; no song length for {folder})",
                      reply.splitlines())

    # -- ch_sp -------------------------------------------------------------------------

    def test_ch_sp_with_no_watch(self):
        self.assertEqual(self.tools().ch_sp(), sp_tools.NO_WATCH)

    def test_ch_sp_prints_the_latest_read(self):
        tools = self.tools()
        self.watched(tools)
        reply = tools.ch_sp().splitlines()
        # The latest read is the recording's last, which the watch read again
        # after the song stopped.
        snap = self.run.snaps[-1]
        row = dict(zip(WATCH_COLUMNS, sp_watch.watch_row(None, snap)))
        amount = getattr(snap, sp_tools.AMOUNT_COL)
        sp_max = getattr(snap, sp_tools.MAX_COL)
        self.assertGreater(sp_max, 0, "the recording's last read should have a maximum")

        def ms(s):
            return sp_compare.format_ms(C.s_to_ms(s))

        self.assertEqual(reply[:5], [
            f"engine clock: {ms(snap.clock_s)}",
            f"amount: {amount} ({sp_tools.AMOUNT_COL})",
            f"max: {sp_max} ({sp_tools.MAX_COL})",
            f"game's half: {getattr(snap, sp_tools.HALF_COL)} ({sp_tools.HALF_COL})",
            f"share of max: {amount / sp_max:.3f}",
        ])
        self.assertTrue(reply[5].startswith(
            "ready: yes" if sp_tools.game_ready(row) else "ready: no"), reply[5])
        self.assertEqual(reply[6:10], [
            f"active flag: {snap.sp_active_byte} "
            f"({'on' if sp_compare.is_active(row) else 'off'})",
            f"end time: {ms(snap.sp_end_s)}, {ms(snap.sp_end_s - snap.clock_s)} ahead of the clock",
            f"multiplier: {snap.sp_mult_u32}",
            f"overflow flag: {snap.sp_overflow_u32}",
        ])

    def test_sp_lines_says_ready_and_the_flag_as_the_game_reads_them(self):
        readies, flags = set(), set()
        for snap in recorded_snapshots():
            row = dict(zip(WATCH_COLUMNS, sp_watch.watch_row(None, snap)))
            lines = sp_tools.sp_lines(snap)
            ready = sp_tools.game_ready(row)
            on = sp_compare.is_active(row)
            readies.add(ready)
            flags.add(on)
            ready_line = next(line for line in lines if line.startswith("ready: "))
            self.assertTrue(ready_line.startswith("ready: yes" if ready else "ready: no"),
                            ready_line)
            flag_line = next(line for line in lines if line.startswith("active flag: "))
            self.assertTrue(flag_line.endswith("(on)" if on else "(off)"), flag_line)
        # The recording holds reads on both sides of each question.
        self.assertEqual(readies, {True, False})
        self.assertEqual(flags, {True, False})

    # -- the row questions -------------------------------------------------------------

    def test_ch_sp_between_reports_the_recorded_flip_inside_the_window(self):
        tools = self.tools()
        self.watched(tools)
        rows = sp_compare.load_watch_rows(_LS_G3_WATCH)
        flip = next(i for i, row in enumerate(rows) if row["sp_active_byte"])
        before = rows[flip - 1]
        # From the read before the flip, ready with the flag clear, to the flip.
        inside = tools.ch_sp_between(C.s_to_ms(before["clock_s"]),
                                     C.s_to_ms(rows[flip]["clock_s"]))
        self.assertIn("activations inside: 1", inside)
        self.assert_names_the_recorded_flip(inside)
        self.assertIn(f"first row at the game's half with the flag clear: "
                      f"{sp_compare.format_ms(C.s_to_ms(before['clock_s']))}", inside)

    def test_the_status_activation_line_reads_end_time_at_the_flip(self):
        tools = self.tools()
        self.watched(tools)
        self.assert_names_the_recorded_flip(tools.ch_sp_watch_status())

    # -- the watch of a chained run ----------------------------------------------------

    def test_watch_for_run_searches_with_the_given_search_and_returns_its_engine(self):
        # The tools' own search is the real one, which cannot read a recorded
        # run: had the watch used it, the watch would have failed.
        tools = sp_tools.SpTools(self.game, self.lock, opener=self.opener,
                                 reader=self.run.reader)
        cap_s = SHORT_CAP_S
        engine_ptr, text = tools.watch_for_run(cap_s, self.run.find_engine)
        self.assertEqual(engine_ptr, self.run.engine_ptr, text)
        self.assertTrue(os.path.isfile(csv_named_in(text)), text)
        # The cap given is the one the watch keeps: it ends on its own, long
        # before the stopped clock would end it.
        self.assertTrue(wait_until(lambda: not tools.watch_running(), WAIT_S),
                        f"the watch was still running {WAIT_S:g} s after a {cap_s:g} s cap")
        status = tools.ch_sp_watch_status()
        self.assertNotIn(f"watcher ended: {sp_watch.STOPPED_REASON}", status)
        self.assertIn(f"{cap_s:g} s", status.splitlines()[0])

    def test_watch_for_run_stops_a_search_that_outlasts_its_cap_and_says_so(self):
        release = threading.Event()

        def stuck(proc):
            # A search still running when the watch's wait runs out. It
            # returns once the test releases it.
            release.wait(WAIT_S)
            return self.run.engine_ptr

        tools = self.tools()
        try:
            with mock.patch.object(sp_tools.engine_finder, "ENGINE_SEARCH_CAP_S", SHORT_CAP_S), \
                    mock.patch.object(sp_tools, "READ_CAP_S", SHORT_CAP_S):
                engine_ptr, text = tools.watch_for_run(WAIT_S, stuck)
        finally:
            release.set()
        self.assertIsNone(engine_ptr)
        self.assertIn("did not settle in ", text)
        self.assertIn("the watch was stopped", text)
        # The stop took: once the search returns, the watch ends as stopped
        # at its first poll, not at its run cap.
        self.assertTrue(wait_until(lambda: not tools.watch_running(), WAIT_S),
                        f"the watch still ran {WAIT_S:g} s after its search returned")
        self.assertIn(f"watcher ended: {sp_watch.STOPPED_REASON}", tools.ch_sp_watch_status())

    def test_ch_sp_watch_status_lists_activations_and_rises(self):
        tools = self.tools()
        self.watched(tools)
        status = tools.ch_sp_watch_status().splitlines()
        self.assertIn(f"engine: {hex(self.run.engine_ptr)}", status)
        # The recording's steps up of the amount and its flag's turns on,
        # read row by row.
        rows = sp_compare.load_watch_rows(_LS_G3_WATCH)
        col = sp_tools.AMOUNT_COL
        rises = [(C.s_to_ms(rows[i]["clock_s"]), rows[i - 1][col], rows[i][col])
                 for i in range(1, len(rows)) if rows[i][col] > rows[i - 1][col]]
        flips = [C.s_to_ms(rows[i]["clock_s"]) for i in range(1, len(rows))
                 if rows[i]["sp_active_byte"] and not rows[i - 1]["sp_active_byte"]]
        self.assertEqual(len(rises), 1, "the recording holds one rise")
        self.assertEqual(len(flips), 1, "the recording holds one activation")
        rise_ms, before, after = rises[0]
        self.assertIn(f"activations from 0 ms: {len(flips)}", status)
        self.assertIn(f"rises of {col} from 0 ms: 1", status)
        self.assertIn(f"  at {sp_compare.format_ms(rise_ms)}: +{after - before} "
                      f"({before} -> {after})", status)
        # From just past the rise, which comes before the flip: the rise is
        # left out and the activation kept.
        self.assertLess(rise_ms, flips[0])
        since = math.nextafter(rise_ms, math.inf)
        later = tools.ch_sp_watch_status(since_ms=since).splitlines()
        self.assertIn(f"rises of {col} from {since:g} ms: 0", later)
        self.assertIn(f"activations from {since:g} ms: 1", later)

    def test_watch_for_run_says_why_when_the_game_cannot_be_opened(self):
        # The real opener, asked for a process that is not running: the game's
        # own name with a suffix no running program has.
        missing = f"{C.PROCESS_NAME}-{uuid.uuid4().hex}.exe"
        searched = []
        tools = sp_tools.SpTools(self.game, self.lock,
                                 opener=functools.partial(process.open_process, missing),
                                 reader=self.run.reader)
        engine_ptr, text = tools.watch_for_run(
            SHORT_CAP_S, lambda proc: searched.append(proc) or self.run.engine_ptr)
        self.assertIsNone(engine_ptr)
        self.assertIn(f"watcher ended: failed: could not open the game: "
                      f"{process.ProcessNotFoundError.__name__}: process {missing!r} "
                      "is not running", text)
        # Nothing past the open ran: no search, and no CSV.
        self.assertEqual(searched, [])
        self.assertFalse(os.path.exists(csv_named_in(text)), text)
        self.assertFalse(tools.watch_running())

    def test_watch_for_run_fails_when_the_csv_cannot_be_made_after_the_search(self):
        # A folder in the session folder where the watch's next CSV goes, so
        # the real open of that file fails after the engine search.
        tools = self.tools()
        blocked = os.path.join(self.game.session_dir(),
                               f"{sp_tools.CSV_PREFIX}{tools._count + 1}.csv")
        os.makedirs(blocked)
        engine_ptr, text = tools.watch_for_run(SHORT_CAP_S, self.run.find_engine)
        self.assertIsNone(engine_ptr)
        failed = next((line for line in text.splitlines()
                       if line.startswith("watcher ended: failed: ")), None)
        self.assertIsNotNone(failed, text)
        # The error names the file it could not open.
        self.assertIn(os.path.basename(blocked), failed)
        self.assertEqual(csv_named_in(text), blocked)
        self.assertEqual(self.run.served, 0, "the watch read the game with no CSV to write")
        self.assertTrue(self.run.closed)

    def test_ch_sp_between_with_no_flip_says_so(self):
        tools = self.tools()
        self.watched(tools)
        rows = sp_compare.load_watch_rows(_LS_G3_WATCH)
        flip = next(i for i, row in enumerate(rows) if row["sp_active_byte"])
        # From the first read to the last read before the flip.
        before = tools.ch_sp_between(C.s_to_ms(rows[0]["clock_s"]),
                                     C.s_to_ms(rows[flip - 1]["clock_s"]))
        self.assertIn("no activation inside", before.splitlines())
        ready = next((row for row in rows[:flip]
                      if sp_tools.game_ready(row) and not sp_compare.is_active(row)), None)
        self.assertIsNotNone(ready, "the recording has no ready read before its flip")
        self.assertIn(f"first row at the game's half with the flag clear: "
                      f"{sp_compare.format_ms(C.s_to_ms(ready['clock_s']))} "
                      f"({sp_tools.AMOUNT_COL} {ready[sp_tools.AMOUNT_COL]}, "
                      f"{sp_tools.HALF_COL} {ready[sp_tools.HALF_COL]})", before)
        # Everything before the first read: no rows at all.
        first_ms = C.s_to_ms(rows[0]["clock_s"])
        empty = tools.ch_sp_between(0.0, math.nextafter(first_ms, 0.0)).splitlines()
        self.assertIn("no activation inside", empty)
        self.assertEqual(empty[-1], "no rows inside the window")

    # -- the song end of the gap line --------------------------------------------------

    def gap_line(self, tools):
        """The gap line of the watch's status, and the rows the watch wrote."""
        status = tools.ch_sp_watch_status()
        line = next(line for line in status.splitlines() if line.startswith("gaps over"))
        return line, sp_compare.load_watch_rows(csv_named_in(status))

    def test_the_song_end_comes_from_the_folder_the_watch_was_given(self):
        def checked_in_dump(chart, dump):
            shutil.copyfile(_PITCH_DARK_DUMP, dump)

        with mock.patch.object(hydra_end, "make_dump", checked_in_dump):
            tools = self.tools()
            self.watched(tools, _PITCH_DARK_FOLDER)
        line, rows = self.gap_line(tools)
        self.assertEqual(line, sp_tools._gap_line(
            rows, hydra_end.song_length_ms(_PITCH_DARK_DUMP)))

    def test_a_folder_with_no_length_leaves_the_song_end_unknown(self):
        # A folder with no chart file: bot_run.song_length_ms names no end.
        folder = os.path.join(self.tmp.name, "no-chart")
        os.makedirs(folder)
        tools = self.tools()
        self.watched(tools, folder)
        line, rows = self.gap_line(tools)
        self.assertEqual(line, sp_tools._gap_line(rows, None))

    def test_ch_run_charts_watch_takes_the_song_end_it_was_given(self):
        # ch_run_chart's end, read from its schedule with no new dump.
        _cap_s, song_end_ms = game_tools.schedule_run_cap_s(_PITCH_DARK_SCHEDULE)
        dumps = []
        with mock.patch.object(hydra_end, "make_dump",
                               lambda chart, dump: dumps.append(chart)):
            tools = self.tools()
            engine_ptr, text = tools.watch_for_run(WAIT_S, self.run.find_engine,
                                                   song_end_ms)
        self.assertEqual(engine_ptr, self.run.engine_ptr, text)
        # The wait ends as soon as the watch has read past the recording or
        # has ended, since an ended watch reads no more.
        wait_until(lambda: self.run.done.is_set() or not tools.watch_running(), WAIT_S)
        self.assertTrue(self.run.done.is_set(), tools.ch_sp_watch_status())
        tools.ch_sp_watch_stop()
        self.assertEqual(dumps, [])
        line, rows = self.gap_line(tools)
        self.assertEqual(line, sp_tools._gap_line(rows, song_end_ms))

    def test_a_watch_with_no_open_run_leaves_the_song_end_unknown(self):
        # No ch_start ran, so GameTools holds no run to read a length from.
        self.assertIsNone(self.game.song_length_ms())
        tools = self.tools()
        self.watched(tools)
        line, rows = self.gap_line(tools)
        self.assertEqual(line, sp_tools._gap_line(rows, None))

    def test_a_watch_that_wrote_no_row_counts_no_gap(self):
        # The real opener, asked for a process that is not running, so the
        # watch ends before its first read.
        missing = f"{C.PROCESS_NAME}-{uuid.uuid4().hex}.exe"
        tools = sp_tools.SpTools(self.game, self.lock,
                                 opener=functools.partial(process.open_process, missing),
                                 reader=self.run.reader)
        engine_ptr, text = tools.watch_for_run(SHORT_CAP_S, self.run.find_engine)
        self.assertIsNone(engine_ptr, text)
        status = tools.ch_sp_watch_status().splitlines()
        self.assertIn("rows: 0", status)
        line = next(line for line in status if line.startswith("gaps over"))
        self.assertTrue(line.startswith(
            f"gaps over {sp_tools.GAP_MS:g} ms between samples: 0"), line)
        self.assertNotIn("longest", line)

    def test_since_ms_does_not_narrow_the_gap_line(self):
        # Each poll sleeps twice GAP_MS on the real clock, so every recorded
        # read the watch writes comes after a gap over GAP_MS.
        poll_s = C.ms_to_s(2 * sp_tools.GAP_MS)
        tools = self.tools(sleep=lambda _s: time.sleep(poll_s))
        self.watched(tools)
        status = tools.ch_sp_watch_status()
        rows = sp_compare.load_watch_rows(csv_named_in(status))
        self.assertGreater(sp_compare.gap_summary(rows, sp_tools.GAP_MS, None).count, 0)
        # From just past the last row's clock: nothing is left to list.
        after_ms = math.nextafter(max(C.s_to_ms(row["clock_s"]) for row in rows), math.inf)
        later = tools.ch_sp_watch_status(since_ms=after_ms)

        def gap(text):
            return next(line for line in text.splitlines() if line.startswith("gaps over"))

        def activations(text):
            line = next(line for line in text.splitlines()
                        if line.startswith("activations from "))
            return int(line.rsplit(": ", 1)[1])

        # since_ms narrows the activation list, but not the gap line.
        self.assertGreater(activations(status), 0, status)
        self.assertEqual(activations(later), 0, later)
        self.assertEqual(gap(later), gap(status))

    def test_the_gap_line_comes_before_the_lists_a_cut_reply_can_lose(self):
        tools = self.tools()
        self.watched(tools)
        lines = tools.ch_sp_watch_status().splitlines()

        def first(prefix):
            return next(i for i, line in enumerate(lines) if line.startswith(prefix))

        gap = first("gaps over")
        self.assertLess(gap, first("activations from "))
        self.assertLess(gap, first(f"rises of {sp_tools.AMOUNT_COL} from "))

    # -- ch_sp_compare on a recorded run -----------------------------------------------

    def rise_files(self):
        """pitch-dark-ratios-run1's rise piece (test_sp_compare's _RISE_CSV),
        Hydra's path-0 score of its chart held to the piece's clock span
        (test_sp_compare._within) and written to the temp folder, and the
        path's schedule. Returns (rows, score, score_json, schedule_json)."""
        rows = sp_compare.load_watch_rows(_RISE_CSV)
        score = _within(sp_compare.load_score(_replay_file("test_pitch_dark_ratio-score.json")),
                        rows)
        score_json = os.path.join(self.tmp.name, "rise-score.json")
        with open(score_json, "w", encoding="utf-8") as f:
            json.dump(score, f)
        return rows, score, score_json, _replay_file("test_pitch_dark_ratio-schedule.json")

    def test_ch_sp_compare_refuses_without_a_resolution(self):
        rows, score, score_json, schedule_json = self.rise_files()
        tools = self.tools()
        refused = tools.ch_sp_compare(_RISE_CSV, score_json, schedule_json)
        self.assertEqual(len(refused.splitlines()), 1, refused)
        self.assertTrue(refused.startswith("compare refused: "), refused)
        self.assertIn("resolution", refused)
        # The same files with the chart's resolution are compared.
        resolution = _midi_resolution(_PITCH_DARK_CHART)
        want = sp_compare.compare(rows, score, resolution=resolution,
                                  reach=sp_compare.press_reach(schedule_json)).text()
        reply = tools.ch_sp_compare(_RISE_CSV, score_json, schedule_json, resolution)
        self.assertTrue(reply.startswith(want), reply)

    def test_ch_sp_compare_passes_the_last_plays_stop(self):
        _rows, _score, score_json, schedule_json = self.rise_files()
        resolution = _midi_resolution(_PITCH_DARK_CHART)
        # No play session has run: the report says no stop was given.
        self.assertIsNone(self.game.play_snapshot())
        reply = self.tools().ch_sp_compare(_RISE_CSV, score_json, schedule_json, resolution)
        self.assertIn(stop_check(None)[1], reply.splitlines())
        # A real play session on the keys route, whose real player asks the
        # real opener for a process that is not running, so it ends at once
        # before any press.
        missing = f"{C.PROCESS_NAME}-{uuid.uuid4().hex}.exe"
        game = game_tools.GameTools(
            bot_run.Env(run_reg=None), self.lock, os.path.join(self.tmp.name, "play"),
            driver=object(),
            live_play=functools.partial(
                play_chart.live_play,
                open_proc=functools.partial(process.open_process, missing)))
        self.addCleanup(game.ch_stop)
        started = game.ch_play_chart(_replay_file("test_pitch_dark_ratio-score.json"),
                                     schedule_json, route=game_tools.ROUTE_KEYS)
        self.assertIsNotNone(game.play_snapshot(), started)
        self.assertTrue(wait_until(lambda: game.play_snapshot()["stop"] is not None, WAIT_S),
                        f"the play session still ran {WAIT_S:g} s after it started")
        play = game.play_snapshot()
        self.assertEqual(play["press_count"], 0, play)
        self.assertIn(missing, play["stop"])
        reply = sp_tools.SpTools(game, self.lock).ch_sp_compare(
            _RISE_CSV, score_json, schedule_json, resolution)
        ended, line = stop_check(play["stop"], play["message"])
        self.assertFalse(ended)
        self.assertIn(line, reply.splitlines())
        self.assertNotIn(stop_check(None)[1], reply.splitlines())

    def test_ch_sp_compare_finds_the_recorded_rise_after_the_schedules_presses(self):
        rows, score, score_json, schedule_json = self.rise_files()
        ends = [c for c in score["chords"] if c["is_sp_phrase_end"]]
        self.assertEqual(len(ends), 1, "the rise piece should hold one phrase end")
        rises = sp_compare.phrase_rises(rows, sp_tools.AMOUNT_COL)
        self.assertTrue(rises, "the rise piece should hold the game's rise")
        # The rise comes after the schedule's first press of that chord.
        presses = sp_compare.press_reach(schedule_json)[ends[0]["index"]]
        self.assertGreaterEqual(rises[0].ms, presses.earliest_ms)
        reply = self.tools().ch_sp_compare(_RISE_CSV, score_json, schedule_json,
                                           _midi_resolution(_PITCH_DARK_CHART))
        line = [line for line in reply.splitlines() if line.startswith(
            f"PASS phrase rise after {sp_compare._chord_label(ends[0])}: ")]
        self.assertEqual(len(line), 1, reply)
        self.assertIn(f"+{rises[0].size} ({rises[0].before} -> {rises[0].after}) "
                      f"at {sp_compare.format_ms(rises[0].ms)}", line[0])

    def test_ch_sp_compare_replies_with_the_compare_report_then_the_ready_lines(self):
        rows, score, score_json, schedule_json = self.rise_files()
        resolution = _midi_resolution(_PITCH_DARK_CHART)
        report = sp_compare.compare(rows, score, resolution=resolution,
                                    reach=sp_compare.press_reach(schedule_json)).text()
        ready = sp_tools.ready_lines(rows, play_chart.load_schedule(schedule_json))
        self.assertTrue(ready, "the schedule should hold an activation")
        reply = self.tools().ch_sp_compare(_RISE_CSV, score_json, schedule_json, resolution)
        # Every line of the report, in order, then every ready line.
        self.assertEqual(reply.splitlines(), report.splitlines() + ready)

    def test_ch_sp_compare_reads_the_press_logs_judged_times(self):
        # A press log judging each chord of the piece 1 ms before its planned
        # press (a hand-picked offset): the reply is the report over
        # run_reach's judged reach and its header line, then the ready lines.
        rows, score, score_json, schedule_json = self.rise_files()
        resolution = _midi_resolution(_PITCH_DARK_CHART)
        planned = sp_compare.press_reach(schedule_json)
        press_csv = write_press_log(
            os.path.join(self.tmp.name, "press-1.csv"),
            [(c["index"], planned[c["index"]].earliest_ms - 1.0) for c in score["chords"]])
        reach, source = sp_compare.run_reach(schedule_json, press_csv)
        self.assertNotEqual(reach, planned)
        self.assertTrue(source.startswith(
            f"press times: the game's judged times (judged_ms) from {press_csv} "), source)
        report = sp_compare.compare(rows, score, resolution=resolution, reach=reach,
                                    reach_source=source).text()
        ready = sp_tools.ready_lines(rows, play_chart.load_schedule(schedule_json))
        reply = self.tools().ch_sp_compare(_RISE_CSV, score_json, schedule_json, resolution,
                                           press_csv=press_csv)
        self.assertEqual(reply.splitlines(), report.splitlines() + ready)
        self.assertIn(source, reply.splitlines())

    def test_ch_sp_compare_names_a_missing_press_log(self):
        _rows, _score, score_json, schedule_json = self.rise_files()
        missing = os.path.join(self.tmp.name, "no-press.csv")
        reply = self.tools().ch_sp_compare(_RISE_CSV, score_json, schedule_json,
                                           _midi_resolution(_PITCH_DARK_CHART),
                                           press_csv=missing)
        self.assertTrue(reply.startswith(f"could not read {missing}: FileNotFoundError: "),
                        reply)

    def test_ch_sp_compare_names_a_missing_file(self):
        missing = os.path.join(self.tmp.name, "none.csv")
        # The error the OS gives for opening that path.
        with self.assertRaises(OSError) as opened:
            open(missing, encoding="utf-8")
        reply = self.tools().ch_sp_compare(missing, missing, missing, 192)
        self.assertEqual(reply, f"compare failed: {type(opened.exception).__name__}: "
                                f"{opened.exception}")

    def test_ready_when_comes_from_the_schedule_and_whether_from_the_games_half(self):
        # pitch-dark-ratios-run1's real watch piece and Hydra's schedule of
        # the same chart's path.
        rows, _score, _score_json, schedule_json = self.rise_files()
        schedule = play_chart.load_schedule(schedule_json)
        lines = sp_tools.ready_lines(rows, schedule)
        self.assertEqual(len(lines), len(schedule.activations))
        ms_of = {chord.index: chord.ms for chord in schedule.chords}
        col, half = sp_tools.AMOUNT_COL, sp_tools.HALF_COL
        since_ms, found = 0.0, []
        for act, line in zip(schedule.activations, lines):
            # When: the schedule's own ready point.
            self.assertTrue(line.startswith(
                f"ready for activation {act.index}: Hydra ready at "
                f"{sp_compare.format_ms(act.sp_ready_ms)} (the schedule's sp_ready_ms); "), line)
            # Whether: the first row since the activation before, up to this
            # one's chord, whose flag is clear and whose amount has reached
            # the game's own half.
            until_ms = ms_of[act.chord]
            first = next((r for r in rows
                          if since_ms <= C.s_to_ms(r["clock_s"]) <= until_ms
                          and not sp_compare.is_active(r) and r[col] >= r[half]), None)
            if first is None:
                self.assertTrue(line.endswith("the game was not ready before its chord"), line)
            else:
                found.append(first)
                self.assertTrue(line.endswith(
                    f"the game first ready at {sp_compare.format_ms(C.s_to_ms(first['clock_s']))} "
                    f"({col} {first[col]}, {half} {first[half]})"), line)
            since_ms = until_ms
        self.assertTrue(found, "the piece should hold a ready row before a scheduled chord")
        # The game's half decides, not a share of the maximum: one above the
        # amount and the same row is not ready, at the amount it is.
        row = found[0]
        self.assertTrue(sp_tools.game_ready({**row, half: row[col]}))
        self.assertFalse(sp_tools.game_ready({**row, half: row[col] + 1}))

    def test_ch_sp_compare_refuses_a_schedule_that_refuses_its_path_naming_it(self):
        _rows, score, score_json, schedule_path = self.rise_files()
        with open(schedule_path, encoding="utf-8") as f:
            doc = json.load(f)
        # Hydra's schedule of the path with one line in its refusals list, in
        # the form tools/replay_schedule.cpp writes for a chord it cannot time.
        # The file is input that the real reader reads.
        chord = next(c for c in score["chords"] if c["is_sp_phrase_end"])["index"]
        doc["refusals"] = [f"chord {chord} is two squeezes; no rule times both"]
        schedule_json = os.path.join(self.tmp.name, "refusing-schedule.json")
        with open(schedule_json, "w", encoding="utf-8") as f:
            json.dump(doc, f)
        with self.assertRaises(ValueError) as refused:
            play_chart.load_schedule(schedule_json)
        reply = self.tools().ch_sp_compare(_RISE_CSV, score_json, schedule_json,
                                           _midi_resolution(_PITCH_DARK_CHART))
        # One line naming the file and the reader's refusal, and no report.
        self.assertEqual(reply, f"could not read {schedule_json}: ValueError: "
                                f"{refused.exception}")
        self.assertIn(doc["refusals"][0], reply)


# -- the gap line on a recorded run ------------------------------------------------------

# LS-O2's whole Star Power watch, its wall and clock columns, from
# hydra-data\ch-mcp-runs\2026-11-07\ls-o2-run1\sp_watch-1.csv.
_LS_O2_WATCH = os.path.join(_REPO_ROOT, "testdata", "ch_watch", "ls-o2-run1-sp_watch-1.csv")
# The song length ch_start printed for that run. Copied from line 9 of
# hydra-data\ch-mcp-runs\2026-11-07\ls-o2-run1\replies.txt.
_LS_O2_REPLIES = os.path.join(_CH_RUNS, "ls-o2-run1", "replies.txt")


def ls_o2_rows():
    with open(_LS_O2_WATCH, newline="", encoding="utf-8") as f:
        return [{c: float(r[c]) for c in ("wall_s", "clock_s")} for r in csv.DictReader(f)]


def ls_o2_song_end_ms() -> int:
    with open(_LS_O2_REPLIES, encoding="utf-8") as f:
        line = f.readline()
    return int(line.split("song length: ", 1)[1].split(" ms", 1)[0])


def wall_gap_ms(rows, i) -> float:
    return C.s_to_ms(rows[i]["wall_s"] - rows[i - 1]["wall_s"])


class GapLineOnARecordedRunTest(unittest.TestCase):
    """sp_tools._gap_line, the status's gap line, over LS-O2's real watch and
    the song length its run printed."""

    def setUp(self):
        self.rows = ls_o2_rows()
        self.end_ms = ls_o2_song_end_ms()

    def test_the_gap_line_leaves_out_a_gap_past_the_song_end(self):
        rows, end = self.rows, self.end_ms
        inside = [row for row in rows if C.s_to_ms(row["clock_s"]) <= end]
        # The rows past the song end, before the final one, hold gaps: with
        # every row counted, they change the line.
        self.assertNotEqual(sp_tools._gap_line(rows[:-1], None),
                            sp_tools._gap_line(inside, None))
        # With the song end, they change nothing: the line is the one of the
        # rows up to the end.
        self.assertEqual(sp_tools._gap_line(rows, end), sp_tools._gap_line(inside, end))

    def test_the_gap_line_leaves_out_a_final_frozen_clock_row(self):
        rows, end = self.rows, self.end_ms
        # The watch's final row repeats the stopped clock after a long wait.
        self.assertEqual(rows[-1]["clock_s"], rows[-2]["clock_s"])
        self.assertGreater(wall_gap_ms(rows, len(rows) - 1), sp_tools.GAP_MS)
        self.assertEqual(sp_tools._gap_line(rows, end), sp_tools._gap_line(rows[:-1], end))
        # With every row counted, that final wait is the longest gap.
        self.assertEqual(sp_compare.gap_summary(rows, sp_tools.GAP_MS, None).longest_ms,
                         wall_gap_ms(rows, len(rows) - 1))

    def test_an_unknown_song_length_is_said_on_the_gap_line(self):
        rows = self.rows
        unknown = sp_tools._gap_line(rows, None)
        known = sp_tools._gap_line(rows, self.end_ms)
        self.assertIn("song length not known", unknown)
        self.assertNotIn("song length not known", known)
        count = sp_compare.gap_summary(rows, sp_tools.GAP_MS, None).count
        self.assertGreater(count, 0)
        self.assertTrue(unknown.startswith(
            f"gaps over {sp_tools.GAP_MS:g} ms between samples: {count}, "), unknown)

    def test_an_unknown_song_length_is_said_when_no_gap_is_over(self):
        # The recording's first two reads, 4.5 ms apart on its wall clock.
        unknown = sp_tools._gap_line(self.rows[:2], None)
        self.assertTrue(unknown.startswith(
            f"gaps over {sp_tools.GAP_MS:g} ms between samples: 0 "), unknown)
        self.assertIn("song length not known", unknown)

    def inside_gaps(self):
        """sp_compare.gap_summary, the owner of the gap count, over the
        recording up to its song end."""
        return sp_compare.gap_summary(self.rows, sp_tools.GAP_MS, self.end_ms)

    def test_the_gap_line_counts_the_gaps_and_names_the_clocks_around_them(self):
        gaps = self.inside_gaps()
        self.assertGreater(gaps.count, 1, "the recording should hold gaps inside the song")
        self.assertEqual(
            sp_tools._gap_line(self.rows, self.end_ms),
            f"gaps over {sp_tools.GAP_MS:g} ms between samples: {gaps.count}, "
            f"longest {round(gaps.longest_ms, 1):g} ms, between "
            f"{sp_compare.format_ms(gaps.from_ms)} and {sp_compare.format_ms(gaps.to_ms)}")

    def test_the_longest_gap_keeps_its_tenth_of_a_ms(self):
        longest = self.inside_gaps().longest_ms
        # The recording's longest gap inside the song is not a whole ms at a
        # tenth's rounding, so a line rounded to the ms would differ.
        self.assertNotEqual(round(longest, 1), round(longest))
        line = sp_tools._gap_line(self.rows, self.end_ms)
        self.assertIn(f", longest {round(longest, 1):g} ms, ", line)
        self.assertNotIn(f", longest {round(longest):g} ms, ", line)

    def test_the_gap_line_says_0_when_no_gap_is_over(self):
        # The recording's first two reads, inside the song and closer together
        # on its wall clock than GAP_MS.
        rows = self.rows[:2]
        self.assertLessEqual(wall_gap_ms(rows, 1), sp_tools.GAP_MS)
        self.assertLessEqual(C.s_to_ms(rows[1]["clock_s"]), self.end_ms)
        self.assertEqual(sp_tools._gap_line(rows, self.end_ms),
                         f"gaps over {sp_tools.GAP_MS:g} ms between samples: 0")

    def test_the_real_ls_o2_gap_line(self):
        rows, end = self.rows, self.end_ms

        def ms(i):
            return C.s_to_ms(rows[i]["clock_s"])

        line = sp_tools._gap_line(rows, end)
        prefix = f"gaps over {sp_tools.GAP_MS:g} ms between samples: "
        self.assertTrue(line.startswith(prefix), line)
        count = int(line[len(prefix):].split(",", 1)[0])
        self.assertGreater(count, 1, "the recording should hold gaps inside the song")
        # Each pair of rows next to each other inside the song (both clocks at
        # or before its end), the widest wall gap first.
        widest = sorted((i for i in range(1, len(rows)) if max(ms(i - 1), ms(i)) <= end),
                        key=lambda i: wall_gap_ms(rows, i), reverse=True)
        # The pairs counted are the widest ones: each is over GAP_MS, and the
        # next widest is not.
        self.assertGreater(wall_gap_ms(rows, widest[count - 1]), sp_tools.GAP_MS)
        self.assertLessEqual(wall_gap_ms(rows, widest[count]), sp_tools.GAP_MS)
        counted = sorted(widest[:count])
        self.assertEqual(
            line,
            f"{prefix}{count}, longest {round(wall_gap_ms(rows, widest[0]), 1):g} ms, between "
            f"{sp_compare.format_ms(ms(counted[0] - 1))} and {sp_compare.format_ms(ms(counted[-1]))}")

    def test_a_gap_of_exactly_gap_ms_is_not_printed(self):
        # The recording's first two reads, inside the song, with their wall
        # times set GAP_MS apart, then just over it.
        first, second = self.rows[:2]

        def line(gap_ms):
            rows = [{**first, "wall_s": 0.0}, {**second, "wall_s": C.ms_to_s(gap_ms)}]
            return sp_tools._gap_line(rows, self.end_ms)

        self.assertEqual(line(sp_tools.GAP_MS),
                         f"gaps over {sp_tools.GAP_MS:g} ms between samples: 0")
        self.assertTrue(line(sp_tools.GAP_MS + 1).startswith(
            f"gaps over {sp_tools.GAP_MS:g} ms between samples: 1, "), line(sp_tools.GAP_MS + 1))


if __name__ == "__main__":
    unittest.main()
