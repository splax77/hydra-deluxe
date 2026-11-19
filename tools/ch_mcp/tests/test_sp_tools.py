"""Tests for the Star Power tools (tools/ch_mcp/sp_tools.py).

No game: a fake opener hands back a fake process, a fake finder stands in
for the engine search, and a scripted reader serves Snapshots in order, then
repeats the last one until the watch is told to stop. The watch itself is the
real sp_watch.watch, on a wall clock that never moves (so neither the frozen
clock check nor the run cap ends a run) and a sleep that yields for a real
millisecond.

SpToolsOnARecordedRunTest runs on the real GameTools and the real clock
instead, with only the game's reads played back from a real recorded run
(RecordedRun).
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
import types
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
from tools.ch_probe import engine_finder as F  # noqa: E402
from tools.ch_probe import process  # noqa: E402
from tools.ch_probe.experiments import sp_compare, sp_watch  # noqa: E402
from tools.ch_probe.experiments.overlay_logs import WATCH_COLUMNS  # noqa: E402
from tools.ch_probe.experiments import play_chart  # noqa: E402
from tools.ch_mcp.tests.test_hydra_end import FAKE  # noqa: E402


def setUpModule():
    # The song ends come from fake dumps, because no test runs hydra_replay.
    FAKE.start()


def tearDownModule():
    FAKE.stop()


_PROBE_TESTS = os.path.join(_REPO_ROOT, "tools", "ch_probe", "tests")
# Mouths path 0's hydra_replay schedule, the file beside its score file.
_MOUTHS_SCHEDULE = os.path.join(_PROBE_TESTS, "mouths_refine_schedule.json")

BACKUP_LINE = "backup folder: X (taken now)"
ENGINE = 0x2_0000_1000
WAIT_S = 5.0
# A watch cap short enough to run out well inside WAIT_S. It is a test input,
# not a value the tools decide.
SHORT_CAP_S = WAIT_S / 25
NO_SONG = "no playing song found in 120 s; start the song and run again"


# The full meter of a chart at resolution 192 (Trippin SP Spike's), from the one
# place that knows the game's full meter (SP-11), so no test types it again.
SPIKE_FULL_METER = sp_compare.game_full_meter(192)


def snap_at(clock_s, **fields):
    """A snapshot of an all-zero object with its clock and any field set."""
    base = engine.decode_snapshot(bytes(engine.SNAPSHOT_SIZE))
    return dataclasses.replace(base, clock_s=clock_s, **fields)


def _row(clock_s, amount, **fields):
    """A watch row as the watch writes it, from snap_at."""
    snap = snap_at(clock_s, sp_amount_u64=amount, sp_amount_u32=amount, **fields)
    return dict(zip(WATCH_COLUMNS, sp_watch.watch_row(100.0 + clock_s, snap)))


class FakeGame:
    """The four GameTools methods SpTools calls, with every call recorded, and
    the open run SpTools reads the song's length from."""

    def __init__(self, folder, events):
        self.folder = folder
        self.events = events
        self.hooks = []
        self.cap_calls = []
        self._run = None
        # What play_snapshot returns: None when no play session ran.
        self.play = None

    def play_snapshot(self):
        return self.play

    def song_length_ms(self):
        return self._run.length_ms if self._run is not None else None

    def ensure_backup(self):
        self.events.append("backup")
        return BACKUP_LINE if self.events.count("backup") == 1 else ""

    def session_dir(self):
        return self.folder

    def add_stop_hook(self, hook):
        self.hooks.append(hook)

    def cap_for_end(self, length_ms):
        self.cap_calls.append(length_ms)
        return 240.0


class FakeProc:
    def __init__(self, events):
        self.events = events

    def verify_targets(self):
        self.events.append("verify")

    def close(self):
        self.events.append("close")


class Script:
    """A reader serving `snaps` in order, then the last one again. `done` is
    set on the first repeat, so the watch has finished with every scripted
    snapshot by then."""

    def __init__(self, snaps):
        self.snaps = list(snaps)
        self.served = 0
        self.done = threading.Event()

    def reader(self, proc, engine_ptr):
        self.engine_ptr = engine_ptr

        def serve():
            i = min(self.served, len(self.snaps) - 1)
            self.served += 1
            if self.served > len(self.snaps):
                self.done.set()
            return self.snaps[i]
        return serve


class SpToolsTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.events = []
        self.game = FakeGame(self.tmp.name, self.events)
        self.proc = FakeProc(self.events)
        self.watch_kw = {}

    def tearDown(self):
        for hook in self.game.hooks:
            hook()
        self.tmp.cleanup()

    def opener(self, **kwargs):
        self.events.append(("open", kwargs))
        return self.proc

    def recording_watch(self, *args, **kwargs):
        self.watch_kw.update(kwargs)
        return sp_watch.watch(*args, **kwargs)

    def tools(self, script=None, find_engine=None):
        script = script or Script([snap_at(1.0)])
        return sp_tools.SpTools(
            self.game, threading.RLock(), opener=self.opener,
            find_engine=find_engine or (lambda proc: ENGINE),
            reader=script.reader, watch=self.recording_watch,
            now=lambda: 1000.0, sleep=lambda s: time.sleep(0.001))

    def started(self, tools, *args, **kwargs):
        reply = tools.ch_sp_watch_start(*args, **kwargs)
        self.assertIn("watching: waiting for a moving clock (cap 120 s", reply)
        return reply

    # -- the run cap -------------------------------------------------------------------

    def test_no_song_folder_takes_the_watcher_cap(self):
        script = Script([snap_at(1.0)])
        tools = self.tools(script)
        reply = self.started(tools)
        self.assertIn("run cap: 240 s (sp_watch.RUN_CAP_S; no song folder given)", reply)
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        self.assertEqual(self.game.cap_calls, [])
        self.assertEqual(self.watch_kw["cap_s"], sp_watch.RUN_CAP_S)

    # -- ch_sp -------------------------------------------------------------------------

    def test_ch_sp_prints_the_latest_read(self):
        script = Script([
            snap_at(1.0, sp_amount_u64=96, sp_max_u64=SPIKE_FULL_METER, sp_mult_u32=4),
            snap_at(12.5, sp_amount_u64=2880, sp_max_u64=SPIKE_FULL_METER, sp_half_u64=2880,
                    sp_active_byte=1, sp_mult_u32=8, sp_end_s=61.25, sp_overflow_u32=1)])
        tools = self.tools(script)
        self.started(tools)
        self.assertTrue(script.done.wait(WAIT_S))
        reply = tools.ch_sp()
        tools.ch_sp_watch_stop()
        self.assertEqual(reply.splitlines()[:10], [
            "engine clock: 12500.0 ms",
            "amount: 2880 (sp_amount_u64)",
            "max: 5760 (sp_max_u64)",
            "game's half: 2880 (sp_half_u64)",
            "share of max: 0.500",
            "ready: yes, the amount is at or over the game's half",
            "active flag: 1 (on)",
            "end time: 61250.0 ms, 48750.0 ms ahead of the clock",
            "multiplier: 8",
            "overflow flag: 1",
        ])

    # -- the watch of a chained run ----------------------------------------------------

    def test_watch_for_run_stops_a_search_that_outlasts_its_cap_and_says_so(self):
        release = threading.Event()

        def stuck(proc):
            release.wait(WAIT_S)
            raise F.EngineSearchTimeout(NO_SONG)

        tools = self.tools()
        with mock.patch.object(sp_tools.engine_finder, "ENGINE_SEARCH_CAP_S", 0.05), \
                mock.patch.object(sp_tools, "READ_CAP_S", 0.05):
            engine_ptr, text = tools.watch_for_run(300.0, stuck)
        release.set()
        self.assertIsNone(engine_ptr)
        self.assertIn("did not settle in 0.1 s", text)
        self.assertIn("the watch was stopped", text)

    def test_watch_for_run_is_refused_while_a_watch_runs(self):
        script = Script([snap_at(1.0)])
        tools = self.tools(script)
        self.started(tools)
        self.assertEqual(tools.watch_for_run(300.0, lambda proc: ENGINE),
                         (None, sp_tools.WATCH_RUNNING))
        tools.ch_sp_watch_stop()

    def test_ch_sp_watch_status_lists_activations_and_rises(self):
        script = Script([
            snap_at(1.0, sp_amount_u64=0, sp_max_u64=5760, sp_mult_u32=4),
            snap_at(1.5, sp_amount_u64=1440, sp_max_u64=5760, sp_mult_u32=4),
            snap_at(2.0, sp_amount_u64=1440, sp_max_u64=5760, sp_mult_u32=8,
                    sp_active_byte=1, sp_end_s=10.0)])
        tools = self.tools(script)
        self.started(tools)
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        status = tools.ch_sp_watch_status()
        self.assertIn("engine: 0x200001000", status)
        self.assertIn("activations from 0 ms: 1", status)
        self.assertIn("rises of sp_amount_u64 from 0 ms: 1", status)
        self.assertIn("  at 1500.0 ms: +1440 (0 -> 1440)", status)
        later = tools.ch_sp_watch_status(since_ms=1600)
        self.assertIn("rises of sp_amount_u64 from 1600 ms: 0", later)
        self.assertIn("activations from 1600 ms: 1", later)

    # -- the gap line ------------------------------------------------------------------

    def status_over(self, samples, since_ms=0, song_end_ms=60000):
        """ch_sp_watch_status's lines over a watch that made these (wall_s,
        clock_s) rows, of a song `song_end_ms` long."""
        tools = self.tools()
        tools._watch = sp_tools._Watch(csv_path="x.csv", cap_s=1.0, rows=[
            dict(zip(WATCH_COLUMNS, sp_watch.watch_row(wall_s, snap_at(clock_s))))
            for wall_s, clock_s in samples], song_end_ms=song_end_ms)
        return tools.ch_sp_watch_status(since_ms).splitlines()

    def gap_line_of_a_finished_watch(self, tools):
        """The gap line after a watch of three rows, the last two 60 ms apart
        on the fake wall clock and the last past 4000 ms."""
        tools._watch.rows[:] = [
            dict(zip(WATCH_COLUMNS, sp_watch.watch_row(wall_s, snap_at(clock_s))))
            for wall_s, clock_s in ((100.0, 3.9), (100.06, 3.96), (100.12, 4.02))]
        return next(line for line in tools.ch_sp_watch_status().splitlines()
                    if line.startswith("gaps over"))

    def test_the_real_ls_o2_gap_line(self):
        path = os.path.join(_REPO_ROOT, "testdata", "ch_watch",
                            "ls-o2-run1-sp_watch-1.csv")
        with open(path, newline="", encoding="utf-8") as f:
            rows = [{c: float(r[c]) for c in ("wall_s", "clock_s")}
                    for r in csv.DictReader(f)]
        self.assertEqual(sp_tools._gap_line(rows, 62080),
                         "gaps over 50 ms between samples: 360, longest 243.7 ms, "
                         "between 2146.8 ms and 40566.2 ms")

    def test_a_watch_with_no_open_run_leaves_the_song_end_unknown(self):
        script = Script([snap_at(1.0)])
        tools = self.tools(script)
        self.started(tools)
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        self.assertIn("(song length not known: every row counted)",
                      self.gap_line_of_a_finished_watch(tools))

    def test_the_real_game_tools_holds_the_run_the_song_end_reads(self):
        # The fake above stands in for this field, so this pins its real name.
        game = game_tools.GameTools(None, threading.RLock(), self.tmp.name, driver=object())
        tools = sp_tools.SpTools(game, threading.RLock())
        self.assertIsNone(game.song_length_ms())
        game._run = types.SimpleNamespace(length_ms=62080)
        self.assertEqual(game.song_length_ms(), 62080)
        self.assertEqual(tools._open_run_length_ms(), 62080)

    def test_the_status_prints_two_gaps_with_the_clocks_around_them(self):
        lines = self.status_over([(100.0, 1.0), (100.004, 1.004), (100.064, 1.064),
                                  (100.068, 1.068), (100.312, 1.312), (100.316, 1.316)])
        self.assertIn("gaps over 50 ms between samples: 2, longest 244 ms, "
                      "between 1004.0 ms and 1312.0 ms", lines)

    def test_the_longest_gap_keeps_its_tenth_of_a_ms(self):
        lines = self.status_over([(100.0, 1.0), (100.2437, 1.2437)])
        self.assertIn("gaps over 50 ms between samples: 1, longest 243.7 ms, "
                      "between 1000.0 ms and 1243.7 ms", lines)

    def test_the_status_prints_0_when_no_gap_is_over(self):
        lines = self.status_over([(100.0, 1.0), (100.004, 1.004), (100.008, 1.008)])
        self.assertIn("gaps over 50 ms between samples: 0", lines)

    def test_a_gap_of_exactly_gap_ms_is_not_printed(self):
        self.assertEqual(sp_tools.GAP_MS, 50)
        lines = self.status_over([(0.0, 1.0), (0.05, 1.05)])
        self.assertIn("gaps over 50 ms between samples: 0", lines)

    def test_a_watch_with_no_rows_prints_0(self):
        self.assertIn("gaps over 50 ms between samples: 0", self.status_over([]))

    def test_since_ms_does_not_narrow_the_gap_line(self):
        lines = self.status_over([(100.0, 1.0), (100.06, 1.06)], since_ms=5000)
        self.assertIn("gaps over 50 ms between samples: 1, longest 60 ms, "
                      "between 1000.0 ms and 1060.0 ms", lines)

    def test_the_gap_line_comes_before_the_lists_a_cut_reply_can_lose(self):
        lines = self.status_over([(100.0, 1.0)])
        gap = next(i for i, line in enumerate(lines) if line.startswith("gaps over"))
        self.assertLess(gap, next(i for i, line in enumerate(lines)
                                  if line.startswith("activations from")))

    def test_ch_sp_compare_names_a_missing_file(self):
        missing = os.path.join(self.tmp.name, "none.csv")
        reply = self.tools().ch_sp_compare(missing, missing, missing, 192)
        self.assertTrue(reply.startswith("compare failed: FileNotFoundError"))

    def test_ch_sp_compare_refuses_without_a_resolution(self):
        missing = os.path.join(self.tmp.name, "none.csv")
        self.assertTrue(self.tools().ch_sp_compare(missing, missing, missing).startswith(
            "compare refused: give resolution"))

    def test_ch_sp_compare_refuses_while_a_play_runs(self):
        self.game.play = {"last_row": None, "press_count": 0, "stop": None, "message": ""}
        missing = os.path.join(self.tmp.name, "none.csv")
        self.assertEqual(self.tools().ch_sp_compare(missing, missing, missing, 192),
                         "compare refused: the play session is still running; "
                         "call ch_play_stop first")

    def test_ch_sp_compare_passes_the_last_plays_stop(self):
        watch_csv, score_json = self._mouths_files()
        self.game.play = {"last_row": None, "press_count": 3, "stop": play_chart.STOP_DONE,
                          "message": ""}
        reply = self.tools().ch_sp_compare(watch_csv, score_json, _MOUTHS_SCHEDULE, 192)
        self.assertIn("PASS run end: the run reached the song's end", reply)
        self.game.play = None
        reply = self.tools().ch_sp_compare(watch_csv, score_json, _MOUTHS_SCHEDULE, 192)
        self.assertIn("FAIL run end: the run's stop reason was not given", reply)

    def test_ch_sp_compare_prints_the_last_plays_stop_message(self):
        watch_csv, score_json = self._mouths_files()
        self.game.play = {"last_row": None, "press_count": 0,
                          "stop": play_chart.STOP_NO_ENGINE,
                          "message": "no song started within 120 s"}
        reply = self.tools().ch_sp_compare(watch_csv, score_json, _MOUTHS_SCHEDULE, 192)
        self.assertIn(f"FAIL run end: the run stopped before the song's end "
                      f"({play_chart.STOP_NO_ENGINE}): no song started within 120 s",
                      reply.splitlines())

    def test_ready_when_comes_from_the_schedule_and_whether_from_the_games_half(self):
        self.assertTrue(sp_tools.game_ready(_row(1.0, 7201, sp_half_u64=7200)))
        self.assertFalse(sp_tools.game_ready(_row(1.0, 7200, sp_half_u64=7201)))
        schedule = play_chart.Schedule(
            chords=[play_chart.Chord(index=5, tick=0, ms=3000.0, lanes=[], dynamics=[])],
            activations=[play_chart.Activation(index=0, chord=5, read_end_after=None,
                                               sp_ready_ms=1500.0)],
            song_length_ms=60000.0)
        ready = sp_tools.ready_lines([_row(1.0, 7200, sp_half_u64=7201),
                                      _row(2.0, 7201, sp_half_u64=7200)], schedule)
        self.assertEqual(ready, [
            "ready for activation 0: Hydra ready at 1500.0 ms (the schedule's sp_ready_ms); "
            "the game first ready at 2000.0 ms (sp_amount_u64 7201, sp_half_u64 7200)"])
        not_ready = sp_tools.ready_lines([_row(2.0, 7200, sp_half_u64=7201)], schedule)
        self.assertEqual(not_ready, [
            "ready for activation 0: Hydra ready at 1500.0 ms (the schedule's sp_ready_ms); "
            "the game was not ready before its chord"])
        # No Python half: sp_tools computes no share of the maximum as a level.
        with open(sp_tools.__file__, encoding="utf-8") as f:
            source = f.read()
        for copy in ("READY_FRACTION", "at_ready_level", "ready_between",
                     "* sp_max", "sp_max /", "/ 2", "* 0.5"):
            self.assertNotIn(copy, source)

    def test_run_cap_line_says_default_when_no_song_length(self):
        folder = os.path.join(self.tmp.name, "no-length")
        os.makedirs(folder)
        script = Script([snap_at(1.0)])
        tools = self.tools(script)
        reply = self.started(tools, folder)
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        self.assertIn(f"run cap: 240 s (default; no song length for {folder})", reply)

    def _compare_files(self, score, rows):
        """A watcher CSV of `rows` and a copy of the score file `score`."""
        score_path = os.path.join(self.tmp.name, "score.json")
        with open(score_path, "w", encoding="utf-8") as f:
            json.dump(score, f)
        csv_path = os.path.join(self.tmp.name, "watch.csv")
        with open(csv_path, "w", newline="", encoding="utf-8") as f:
            w = csv.writer(f)
            w.writerow(WATCH_COLUMNS)
            for r in rows:
                w.writerow([r[c] for c in WATCH_COLUMNS])
        return csv_path, score_path

    def _mouths_files(self):
        with open(os.path.join(_PROBE_TESTS, "mouths_refine_score.json"), encoding="utf-8") as f:
            score = json.load(f)
        # test_sp_compare's rows for chord 740: its kick is pressed after the
        # squeeze-out's end, and the rise lands after that press.
        rows = [_row(118.300, 0), _row(118.412, 0), _row(118.441, 1536)]
        return self._compare_files(score, rows)

    def test_ch_sp_compare_on_mouths_reports_as_compare_does(self):
        watch_csv, score_json = self._mouths_files()
        want = sp_compare.compare(sp_compare.load_watch_rows(watch_csv),
                                  sp_compare.load_score(score_json), resolution=192,
                                  reach=sp_compare.press_reach(_MOUTHS_SCHEDULE)).text()
        self.assertIn("Star Power comparison: 3 watcher rows", want)
        reply = self.tools().ch_sp_compare(watch_csv, score_json, _MOUTHS_SCHEDULE, 192)
        self.assertTrue(reply.startswith(want + "\nready for activation "), reply)

    def test_ch_sp_compare_reads_the_mouths_schedule(self):
        watch_csv, score_json = self._mouths_files()
        reply = self.tools().ch_sp_compare(watch_csv, score_json, _MOUTHS_SCHEDULE, 192)
        self.assertIn("PASS phrase rise after chord 740 (tick 48480, 118426.0 ms): "
                      "+1536 (0 -> 1536) at 118441.0 ms, full", reply)

    def test_ch_sp_compare_refuses_a_schedule_that_refuses_its_path_naming_it(self):
        watch_csv, score_json = self._mouths_files()
        with open(_MOUTHS_SCHEDULE, encoding="utf-8") as f:
            doc = json.load(f)
        doc["refusals"] = ["chord 740 is a squeeze the schedule could not split, "
                           "so it is not pressed"]
        schedule_json = os.path.join(self.tmp.name, "schedule.json")
        with open(schedule_json, "w", encoding="utf-8") as f:
            json.dump(doc, f)
        reply = self.tools().ch_sp_compare(watch_csv, score_json, schedule_json, 192)
        self.assertEqual(reply, f"could not read {schedule_json}: ValueError: {schedule_json} "
                                f"refuses to play this path: chord 740 is a squeeze the "
                                f"schedule could not split, so it is not pressed")


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

    # -- ch_sp -------------------------------------------------------------------------

    def test_ch_sp_with_no_watch(self):
        self.assertEqual(self.tools().ch_sp(), sp_tools.NO_WATCH)

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


if __name__ == "__main__":
    unittest.main()
