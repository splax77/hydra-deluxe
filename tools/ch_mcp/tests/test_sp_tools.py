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

from tools.ch_mcp import frida_tools, sp_tools  # noqa: E402
from tools.ch_probe import engine  # noqa: E402
from tools.ch_probe import engine_finder as F  # noqa: E402
from tools.ch_probe.experiments import sp_watch  # noqa: E402
from tools.ch_probe.experiments.overlay_logs import WATCH_COLUMNS  # noqa: E402
from tools.ch_probe.tests import fakes  # noqa: E402

BACKUP_LINE = "backup folder: X (taken now)"
ENGINE = 0x2_0000_1000
WAIT_S = 5.0
NO_SONG = "no playing song found in 120 s; start the song and run again"


def snap_at(clock_s, **fields):
    """A snapshot of an all-zero object with its clock and any field set."""
    base = engine.decode_snapshot(bytes(engine.SNAPSHOT_SIZE))
    return dataclasses.replace(base, clock_s=clock_s, **fields)


class FakeGame:
    """The four GameTools methods SpTools calls, with every call recorded."""

    def __init__(self, folder, events):
        self.folder = folder
        self.events = events
        self.hooks = []
        self.cap_calls = []

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
        script = Script([snap_at(1.0)])
        tools = self.tools(script)
        reply = self.started(tools, song_folder="C:/songs/x")
        self.assertIn("run cap: 240 s (game_tools.run_cap_s for C:/songs/x)", reply)
        self.assertTrue(script.done.wait(WAIT_S))
        tools.ch_sp_watch_stop()
        self.assertEqual(self.game.cap_calls, ["C:/songs/x"])
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
            snap_at(1.0, sp_amount_u64=96, sp_max_u64=5760, sp_mult_u32=4),
            snap_at(12.5, sp_amount_u64=2880, sp_max_u64=5760, sp_active_byte=1,
                    sp_mult_u32=8, sp_end_s=61.25, sp_overflow_u32=1)])
        tools = self.tools(script)
        self.started(tools)
        self.assertTrue(script.done.wait(WAIT_S))
        reply = tools.ch_sp()
        tools.ch_sp_watch_stop()
        self.assertEqual(reply.splitlines()[:9], [
            "engine clock: 12500.0 ms",
            "amount: 2880 (sp_amount_u64)",
            "max: 5760 (sp_max_u64)",
            "share of max: 0.500",
            "ready: yes, at or over 0.5 of max (sp_compare.READY_FRACTION)",
            "active flag: 1 (on)",
            "end time: 61250.0 ms, 48750.0 ms ahead of the clock",
            "multiplier: 8",
            "overflow flag: 1",
        ])

    def test_sp_lines_below_the_ready_level_and_with_no_maximum(self):
        below = sp_tools.sp_lines(snap_at(2.0, sp_amount_u64=2879, sp_max_u64=5760))
        self.assertIn("ready: no, below 0.5 of max (sp_compare.READY_FRACTION)", below)
        self.assertIn("active flag: 0 (off)", below)
        empty = sp_tools.sp_lines(snap_at(2.0))
        self.assertIn("share of max: none (max is 0)", empty)
        self.assertIn("ready: unknown (max is 0)", empty)

    # -- the row questions -------------------------------------------------------------

    def flip_script(self):
        return Script([
            snap_at(1.0, sp_amount_u64=2880, sp_max_u64=5760, sp_mult_u32=4),
            snap_at(2.0, sp_amount_u64=2880, sp_max_u64=5760, sp_mult_u32=8,
                    sp_active_byte=1, sp_end_s=10.0)])

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
        self.assertIn("no row inside at the ready level with the flag clear", inside)

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
        self.assertIn("first row at the ready level with the flag clear: 1000.0 ms "
                      "(sp_amount_u64 2880)", before)
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

    def status_over(self, samples, since_ms=0):
        """ch_sp_watch_status's lines over a watch that made these (wall_s,
        clock_s) rows."""
        tools = self.tools()
        tools._watch = sp_tools._Watch(csv_path="x.csv", cap_s=1.0, rows=[
            dict(zip(WATCH_COLUMNS, sp_watch.watch_row(wall_s, snap_at(clock_s))))
            for wall_s, clock_s in samples])
        return tools.ch_sp_watch_status(since_ms).splitlines()

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
        reply = self.tools().ch_sp_compare(missing, missing)
        self.assertTrue(reply.startswith("compare failed: FileNotFoundError"))


if __name__ == "__main__":
    unittest.main()
