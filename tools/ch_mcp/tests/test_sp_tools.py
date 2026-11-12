"""Tests for the Star Power tools (tools/ch_mcp/sp_tools.py).

No game: a fake opener hands back a fake process, a fake finder stands in
for the engine search, and a scripted reader serves Snapshots in order, then
repeats the last one until the watch is told to stop. The watch itself is the
real sp_watch.watch, on a wall clock that never moves (so neither the frozen
clock check nor the run cap ends a run) and a sleep that yields for a real
millisecond.
"""

from __future__ import annotations

import contextlib
import csv
import dataclasses
import io
import json
import os
import sys
import tempfile
import threading
import time
import types
import unittest
from unittest import mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import frida_tools, game_tools, sp_tools  # noqa: E402
from tools.ch_probe import engine  # noqa: E402
from tools.ch_probe import engine_finder as F  # noqa: E402
from tools.ch_probe.experiments import sp_compare, sp_watch  # noqa: E402
from tools.ch_probe.experiments.overlay_logs import WATCH_COLUMNS  # noqa: E402
from tools.ch_probe.experiments import play_chart  # noqa: E402
from tools.ch_probe.tests import fakes  # noqa: E402
from tools.ch_mcp import install_chart  # noqa: E402
from tools.ch_mcp.tests.test_hydra_end import FAKE  # noqa: E402


def setUpModule():
    # The song ends come from fake dumps, because no test runs hydra_replay.
    FAKE.start()


def tearDownModule():
    FAKE.stop()


def song_with_end(folder: str, length_ms) -> None:
    """Give the song folder `folder` a chart whose fake dump holds `length_ms`
    as Hydra's song end."""
    chart = os.path.join(folder, install_chart.CHART_FILES[0])
    with open(chart, "w", encoding="utf-8") as f:
        f.write("[Song]\n")
    FAKE.set_end(chart, length_ms)


_PROBE_TESTS = os.path.join(_REPO_ROOT, "tools", "ch_probe", "tests")
# Mouths path 0's hydra_replay schedule, beside its score file.
_MOUTHS_SCHEDULE = os.path.join(_PROBE_TESTS, "mouths_refine_schedule.json")

BACKUP_LINE = "backup folder: X (taken now)"
ENGINE = 0x2_0000_1000
WAIT_S = 5.0
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

    def run_cap_s(self, song_folder):
        self.cap_calls.append(song_folder)
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

    # -- start and stop ----------------------------------------------------------------

    def test_start_then_stop_ends_with_stopped_and_writes_the_csv(self):
        script = Script([snap_at(1.0), snap_at(1.01, sp_amount_u64=2880)])
        tools = self.tools(script)
        reply = self.started(tools)
        csv_path = os.path.join(self.tmp.name, "sp_watch-1.csv")
        self.assertIn(f"csv: {csv_path}", reply)
        self.assertTrue(script.done.wait(WAIT_S))
        stop = tools.ch_sp_watch_stop()
        self.assertIn("watcher ended: stopped; rows written 3", stop)
        self.assertIn(f"csv: {csv_path}", stop)
        self.assertEqual(script.engine_ptr, ENGINE)
        with open(csv_path, newline="", encoding="utf-8") as f:
            table = list(csv.reader(f))
        self.assertEqual(tuple(table[0]), WATCH_COLUMNS)
        # The first read, the change, and the final row for the last read.
        self.assertEqual([r[1] for r in table[1:]], ["1.0", "1.01", "1.01"])
        self.assertEqual([r[4] for r in table[1:]], ["0", "2880", "2880"])
        self.assertEqual(self.events[-1], "close")

    def test_the_backup_comes_before_the_game_is_opened(self):
        script = Script([snap_at(1.0)])
        tools = self.tools(script)
        reply = self.started(tools)
        self.assertTrue(reply.startswith(BACKUP_LINE))
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        self.assertEqual(self.events[:3],
                         ["backup", ("open", {"read_only": True}), "verify"])

    def test_a_second_start_while_running_is_refused(self):
        entered, release = threading.Event(), threading.Event()

        def slow_finder(proc):
            entered.set()
            release.wait(WAIT_S)
            raise F.EngineSearchTimeout(NO_SONG)

        tools = self.tools(find_engine=slow_finder)
        self.started(tools)
        self.assertTrue(entered.wait(WAIT_S))
        self.assertEqual(tools.ch_sp_watch_start(),
                         "a Star Power watch is already running; "
                         "call ch_sp_watch_stop first")
        release.set()
        tools.ch_sp_watch_stop()

    def test_the_stop_hook_stops_the_thread(self):
        script = Script([snap_at(1.0)])
        tools = self.tools(script)
        self.started(tools)
        self.assertTrue(script.done.wait(WAIT_S))
        self.assertEqual(len(self.game.hooks), 1)
        said = self.game.hooks[0]()
        self.assertIn("sp watch: watcher ended: stopped", said)
        self.assertIn("watcher ended: stopped", tools.ch_sp_watch_status())
        self.assertIsNone(self.game.hooks[0]())

    def test_the_default_engine_search_writes_nothing_to_stdout(self):
        # The server's stdout is its protocol channel, so the search's
        # progress dots must not reach it.
        stdout = io.StringIO()
        with fakes.searching_for_no_song(F), contextlib.redirect_stdout(stdout):
            with self.assertRaises(F.EngineSearchTimeout):
                frida_tools.find_engine_quietly(types.SimpleNamespace(module_base=0x1000))
        self.assertEqual(stdout.getvalue(), "")

    def test_a_finder_timeout_is_the_reason_in_the_status(self):
        def no_song(proc):
            raise F.EngineSearchTimeout(NO_SONG)

        tools = self.tools(find_engine=no_song)
        self.started(tools)
        stop = tools.ch_sp_watch_stop()
        self.assertIn(f"watcher ended: {NO_SONG}", stop)
        status = tools.ch_sp_watch_status()
        self.assertIn(f"watcher ended: {NO_SONG}", status)
        self.assertIn("(not written: the engine was never found)", status)
        self.assertIn("engine: not found yet", status)
        self.assertEqual(self.events[-1], "close")

    # -- the run cap -------------------------------------------------------------------

    def test_a_song_folder_takes_its_cap_from_the_game(self):
        folder = os.path.join(self.tmp.name, "x")
        os.makedirs(folder)
        song_with_end(folder, 4000)
        script = Script([snap_at(1.0)])
        tools = self.tools(script)
        reply = self.started(tools, song_folder=folder)
        self.assertIn(f"run cap: 240 s (game_tools.run_cap_s for {folder})", reply)
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        self.assertEqual(self.game.cap_calls, [folder])
        self.assertEqual(self.watch_kw["cap_s"], 240.0)

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

    def test_ch_sp_with_no_watch(self):
        self.assertEqual(self.tools().ch_sp(),
                         "no watcher has run; call ch_sp_watch_start")

    def test_ch_sp_before_the_engine_is_found_says_so(self):
        entered, release = threading.Event(), threading.Event()

        def slow_finder(proc):
            entered.set()
            release.wait(WAIT_S)
            raise F.EngineSearchTimeout(NO_SONG)

        tools = self.tools(find_engine=slow_finder)
        self.started(tools)
        self.assertTrue(entered.wait(WAIT_S))
        reply = tools.ch_sp()
        self.assertTrue(reply.startswith(
            "no engine read yet (the watcher is waiting for a moving clock)"))
        self.assertIn("watcher running: 0 rows so far", reply)
        release.set()
        tools.ch_sp_watch_stop()

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

    def test_sp_lines_below_the_ready_level_and_with_no_maximum(self):
        below = sp_tools.sp_lines(snap_at(2.0, sp_amount_u64=2879,
                                          sp_max_u64=SPIKE_FULL_METER, sp_half_u64=2880))
        self.assertIn("ready: no, the amount is below the game's half", below)
        self.assertIn("active flag: 0 (off)", below)
        empty = sp_tools.sp_lines(snap_at(2.0))
        self.assertIn("share of max: none (max is 0)", empty)
        self.assertIn("ready: unknown (max is 0)", empty)

    # -- the row questions -------------------------------------------------------------

    def flip_script(self):
        return Script([
            snap_at(1.0, sp_amount_u64=2880, sp_max_u64=SPIKE_FULL_METER, sp_half_u64=2880,
                    sp_mult_u32=4),
            snap_at(2.0, sp_amount_u64=2880, sp_max_u64=SPIKE_FULL_METER, sp_half_u64=2880,
                    sp_mult_u32=8, sp_active_byte=1, sp_end_s=10.0)])

    def test_ch_sp_between_reports_a_flip_inside_the_window(self):
        script = self.flip_script()
        tools = self.tools(script)
        self.started(tools)
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        inside = tools.ch_sp_between(1500.0, 2500.0)
        self.assertIn("activations inside: 1", inside)
        self.assertIn("  activation 1 at 2000.0 ms: multiplier 4 before, 8 on the flip "
                      "row; end time at the flip 10000.0 ms; flag still set at the last row",
                      inside)
        self.assertIn("no row inside at the game's half with the flag clear", inside)

    def test_the_status_activation_line_reads_end_time_at_the_flip(self):
        script = self.flip_script()
        tools = self.tools(script)
        self.started(tools)
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        self.assertIn("  activation 1 at 2000.0 ms: multiplier 4 before, 8 on the flip row; "
                      "end time at the flip 10000.0 ms; flag still set at the last row",
                      tools.ch_sp_watch_status().splitlines())

    # -- the watch of a chained run ----------------------------------------------------

    def test_watch_for_run_searches_with_the_given_search_and_returns_its_engine(self):
        script = Script([snap_at(1.0)])
        own = []
        tools = self.tools(script, find_engine=lambda proc: own.append(proc) or 0x1)
        searched = []
        engine_ptr, text = tools.watch_for_run(300.0, lambda proc: searched.append(proc) or ENGINE)
        self.assertEqual(engine_ptr, ENGINE)
        self.assertEqual(text, f"csv: {os.path.join(self.tmp.name, 'sp_watch-1.csv')}")
        self.assertEqual(searched, [self.proc])
        self.assertEqual(own, [])
        self.assertTrue(script.done.wait(WAIT_S))
        self.assertEqual(script.engine_ptr, ENGINE)
        self.assertEqual(self.watch_kw["cap_s"], 300.0)
        tools.ch_sp_watch_stop()

    def test_watch_for_run_says_why_when_no_engine_is_found(self):
        def no_song(proc):
            raise F.EngineSearchTimeout(NO_SONG)

        engine_ptr, text = self.tools().watch_for_run(300.0, no_song)
        self.assertIsNone(engine_ptr)
        self.assertIn(f"watcher ended: {NO_SONG}", text)

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

    def test_watch_for_run_says_why_when_the_game_cannot_be_opened(self):
        tools = self.tools()
        tools._opener = lambda **kw: (_ for _ in ()).throw(OSError("no game"))
        engine_ptr, text = tools.watch_for_run(300.0, lambda proc: ENGINE)
        self.assertIsNone(engine_ptr)
        self.assertIn("could not open the game: OSError: no game", text)

    def test_watch_for_run_fails_when_the_csv_cannot_be_made_after_the_search(self):
        tools = self.tools()
        tools._game.session_dir = lambda: os.path.join(self.tmp.name, "a file")
        with open(os.path.join(self.tmp.name, "a file"), "w", encoding="utf-8"):
            pass
        engine_ptr, text = tools.watch_for_run(300.0, lambda proc: ENGINE)
        self.assertIsNone(engine_ptr)
        self.assertIn("watcher ended: failed:", text)

    def test_watch_for_run_is_refused_while_a_watch_runs(self):
        script = Script([snap_at(1.0)])
        tools = self.tools(script)
        self.started(tools)
        self.assertEqual(tools.watch_for_run(300.0, lambda proc: ENGINE),
                         (None, sp_tools.WATCH_RUNNING))
        tools.ch_sp_watch_stop()

    def test_ch_sp_between_with_no_flip_says_so(self):
        script = self.flip_script()
        tools = self.tools(script)
        self.started(tools)
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        before = tools.ch_sp_between(0.0, 1500.0)
        self.assertIn("no activation inside", before)
        self.assertIn("first row at the game's half with the flag clear: 1000.0 ms "
                      "(sp_amount_u64 2880, sp_half_u64 2880)", before)
        self.assertIn("no rows inside the window", tools.ch_sp_between(3000.0, 4000.0))

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

    def test_the_gap_line_leaves_out_a_gap_past_the_song_end(self):
        lines = self.status_over([(100.0, 3.9), (100.06, 3.96), (100.12, 4.02)],
                                 song_end_ms=4000)
        self.assertIn("gaps over 50 ms between samples: 1, longest 60 ms, "
                      "between 3900.0 ms and 3960.0 ms", lines)

    def test_the_gap_line_leaves_out_a_final_frozen_clock_row(self):
        lines = self.status_over([(100.0, 3.9), (100.004, 4.02), (105.1, 4.02)],
                                 song_end_ms=4000)
        self.assertIn("gaps over 50 ms between samples: 0", lines)

    def test_an_unknown_song_length_is_said_on_the_gap_line(self):
        lines = self.status_over([(100.0, 3.9), (100.06, 3.96)], song_end_ms=None)
        self.assertIn("gaps over 50 ms between samples: 1, longest 60 ms, between "
                      "3900.0 ms and 3960.0 ms (song length not known: every row counted)",
                      lines)

    def test_an_unknown_song_length_is_said_when_no_gap_is_over(self):
        lines = self.status_over([(100.0, 3.9), (100.004, 3.904)], song_end_ms=None)
        self.assertIn("gaps over 50 ms between samples: 0 "
                      "(song length not known: every row counted)", lines)

    def test_the_real_ls_o2_gap_line(self):
        path = os.path.join(_REPO_ROOT, "testdata", "ch_watch",
                            "ls-o2-run1-sp_watch-1.csv")
        with open(path, newline="", encoding="utf-8") as f:
            rows = [{c: float(r[c]) for c in ("wall_s", "clock_s")}
                    for r in csv.DictReader(f)]
        self.assertEqual(sp_tools._gap_line(rows, 62080),
                         "gaps over 50 ms between samples: 360, longest 243.7 ms, "
                         "between 2146.8 ms and 40566.2 ms")

    def test_the_song_end_comes_from_the_folder_the_watch_was_given(self):
        folder = os.path.join(self.tmp.name, "song")
        os.makedirs(folder)
        song_with_end(folder, 4000)
        script = Script([snap_at(1.0)])
        tools = self.tools(script)
        self.started(tools, folder)
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        self.assertEqual(self.gap_line_of_a_finished_watch(tools),
                         "gaps over 50 ms between samples: 1, longest 60 ms, "
                         "between 3900.0 ms and 3960.0 ms")

    def test_a_folder_with_no_length_leaves_the_song_end_unknown(self):
        folder = os.path.join(self.tmp.name, "no-length")
        os.makedirs(folder)
        self.game._run = types.SimpleNamespace(length_ms=4000)
        script = Script([snap_at(1.0)])
        tools = self.tools(script)
        self.started(tools, folder)
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        self.assertIn("(song length not known: every row counted)",
                      self.gap_line_of_a_finished_watch(tools))

    def test_with_no_folder_the_song_end_comes_from_the_open_run(self):
        self.game._run = types.SimpleNamespace(length_ms=4000)
        script = Script([snap_at(1.0)])
        tools = self.tools(script)
        self.started(tools)
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        self.assertEqual(self.gap_line_of_a_finished_watch(tools),
                         "gaps over 50 ms between samples: 1, longest 60 ms, "
                         "between 3900.0 ms and 3960.0 ms")

    def test_ch_run_charts_watch_takes_the_song_end_from_the_open_run(self):
        self.game._run = types.SimpleNamespace(length_ms=4000)
        script = Script([snap_at(1.0)])
        tools = self.tools(script)
        engine_ptr, _text = tools.watch_for_run(5.0, lambda proc: ENGINE)
        self.assertEqual(engine_ptr, ENGINE)
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        self.assertEqual(self.gap_line_of_a_finished_watch(tools),
                         "gaps over 50 ms between samples: 1, longest 60 ms, "
                         "between 3900.0 ms and 3960.0 ms")

    def test_ch_run_charts_watch_takes_the_song_end_from_the_folder_it_was_given(self):
        # ch_start with no song, then ch_run_chart(song_folder): no open-run length.
        folder = os.path.join(self.tmp.name, 'song2')
        os.makedirs(folder)
        song_with_end(folder, 4000)
        script = Script([snap_at(1.0)])
        tools = self.tools(script)
        engine_ptr, _text = tools.watch_for_run(5.0, lambda proc: ENGINE, folder)
        self.assertEqual(engine_ptr, ENGINE)
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        self.assertEqual(self.gap_line_of_a_finished_watch(tools),
                         'gaps over 50 ms between samples: 1, longest 60 ms, '
                         'between 3900.0 ms and 3960.0 ms')

    def test_the_song_end_outlives_the_run(self):
        # ch_stop clears the open run, and a status call after it still has
        # the watch's song end.
        self.game._run = types.SimpleNamespace(length_ms=4000)
        script = Script([snap_at(1.0)])
        tools = self.tools(script)
        self.started(tools)
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        self.game._run = None
        self.assertNotIn("not known", self.gap_line_of_a_finished_watch(tools))

    def test_a_menu_launch_leaves_the_song_end_unknown(self):
        # bot_run.Run has no length when no song was on the command line.
        self.game._run = types.SimpleNamespace(length_ms=None)
        script = Script([snap_at(1.0)])
        tools = self.tools(script)
        self.started(tools)
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        self.assertIn("(song length not known: every row counted)",
                      self.gap_line_of_a_finished_watch(tools))

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


if __name__ == "__main__":
    unittest.main()
