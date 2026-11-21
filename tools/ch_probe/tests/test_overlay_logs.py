"""Checks the overlay spike's two CSV column lists (overlay_logs) against real
logs from earlier runs, and checks that the writer and the readers agree on
them. No list is typed out here: overlay_logs is their one home.

The press and watch logs read here are parts of real runs, copied under
testdata/ch_runs/<run folder> from the run folders of the same name under
hydra-data/ch-mcp-runs.

Run from the repo root with:
    py -m unittest tools.ch_probe.tests.test_overlay_logs
"""

from __future__ import annotations

import csv
import os
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.experiments import (  # noqa: E402
    overlay_logs, play_chart, score_compare, sp_compare, watch_checks)

_CH_RUNS = os.path.join(_REPO_ROOT, "testdata", "ch_runs")

# One real press log of each age a run left on disk, each named by the
# overlay_logs list its run's play_chart wrote. No run left a log with
# PRESS_COLUMNS_WITHOUT_WALL or PRESS_COLUMNS_WITH_HIT; the prefix test below
# covers those two.
_RECORDED_PRESS_LOGS = (
    # hydra-data/ch-mcp-runs/2026-10-28/l3/keys/presses.csv, rows 1-3.
    ("KEYS_PRESS_COLUMNS", ("2026-10-28-l3", "keys-presses-rows-1-3.csv")),
    # hydra-data/ch-mcp-runs/2026-11-07/ls-t3-run-b/press-1.csv, chords 38-40.
    ("PRESS_COLUMNS_WITHOUT_PLAYER", ("ls-t3-run-b", "press-1-chords-38-40.csv")),
    # hydra-data/ch-mcp-runs/2026-11-07/ls-f1-run1/press-2.csv, rows 1-3.
    ("PRESS_COLUMNS_WITHOUT_RETIME", ("ls-f1-run1", "press-2-rows-1-3.csv")),
    # hydra-data/ch-mcp-runs/2026-11-11/ls-a2-run1/press-1.csv.
    ("PRESS_COLUMNS_BEFORE_TIMING", ("ls-a2-run1", "press-1-rows-1-3-1906.csv")),
    # hydra-data/ch-mcp-runs/2026-11-12/ls-p3-run1/press-1.csv, rows 1-3.
    ("PRESS_COLUMNS_BEFORE_IN_GAME", ("ls-p3-run1", "press-1-rows-1-3.csv")),
    # hydra-data/ch-mcp-runs/2026-11-12/ls-g3-run1/press-1.csv, its first rows.
    ("PRESS_LOG_COLUMNS", ("ls-g3-run1", "press-1-head.csv")),
)

# The press logs' column lists from the oldest to the newest, as the
# overlay_logs docstring tells their history.
_PRESS_HISTORY = (
    "KEYS_PRESS_COLUMNS", "PRESS_COLUMNS_WITHOUT_WALL", "PRESS_COLUMNS_WITHOUT_PLAYER",
    "PRESS_COLUMNS_WITHOUT_RETIME", "PRESS_COLUMNS_WITH_HIT",
    "PRESS_COLUMNS_BEFORE_TIMING", "PRESS_COLUMNS_BEFORE_IN_GAME", "PRESS_LOG_COLUMNS",
)

# A real watch log the current watcher wrote:
# hydra-data/ch-mcp-runs/2026-11-12/ls-g3-run1/sp_watch-1.csv, its first rows.
_CURRENT_WATCH_LOG = os.path.join(_CH_RUNS, "ls-g3-run1", "sp_watch-1.csv")


def _table(path: str) -> list:
    with open(path, encoding="utf-8", newline="") as f:
        return list(csv.reader(f))


def _recorded(name: str) -> str:
    return os.path.join(_CH_RUNS, *dict(_RECORDED_PRESS_LOGS)[name])


class WatchLogTest(unittest.TestCase):
    def test_the_readers_read_a_real_watch_log_back_column_by_column(self):
        # Both readers take the current watcher's log and give back each value
        # the file holds under the column the file names it by.
        table = _table(_CURRENT_WATCH_LOG)
        header, rows = overlay_logs.watch_header(table[0]), table[1:]
        self.assertTrue(rows, _CURRENT_WATCH_LOG)
        self.assertIn(header, watch_checks.accepted_headers())
        for reader in (watch_checks.load_watch_rows, sp_compare.load_watch_rows):
            got = reader(_CURRENT_WATCH_LOG)
            self.assertEqual(len(got), len(rows), reader.__module__)
            for row, read in zip(rows, got):
                self.assertEqual(set(read), set(overlay_logs.WATCH_COLUMNS), reader.__module__)
                for column, text in zip(header, row):
                    self.assertTrue(watch_checks.same_value(
                        read[column], watch_checks.parse_value(text)),
                        (reader.__module__, column, text, read[column]))

    def test_the_saved_watch_logs_headers_read_as_watch_columns(self):
        # The saved logs keep their own header, hit_time_s included; read
        # through watch_header, each must be one the readers accept.
        folder = os.path.join(_REPO_ROOT, "tools", "ch_probe", "experiments",
                              "results", "overlay-spike")
        names = sorted(n for n in os.listdir(folder) if n.endswith("_sp_watch.csv"))
        self.assertTrue(names, folder)
        for name in names:
            with open(os.path.join(folder, name), encoding="utf-8") as f:
                header = tuple(f.readline().strip().split(","))
            self.assertIn("hit_time_s", header, name)
            self.assertEqual(overlay_logs.watch_header(header),
                             overlay_logs.WATCH_COLUMNS_WITHOUT_LATE_GROUPS, name)


class PressLogTest(unittest.TestCase):
    def test_a_log_play_chart_writes_reads_back_through_score_compare(self):
        # play_chart's writer takes the rows of a real current log and writes
        # a file that score_compare reads to the same presses as the real log,
        # with every column the real log has, in its order.
        real = _recorded("PRESS_LOG_COLUMNS")
        with open(real, encoding="utf-8", newline="") as f:
            rows = list(csv.DictReader(f))
        self.assertTrue(rows, real)
        with tempfile.TemporaryDirectory() as tmp:
            written = os.path.join(tmp, "press.csv")
            play_chart.write_press_log(written, rows)
            header = _table(written)[0]
            self.assertEqual([c for c in header if c in rows[0]], list(rows[0]))
            self.assertEqual(score_compare.load_press_log(written),
                             score_compare.load_press_log(real))

    def test_each_real_older_press_log_has_its_named_columns_and_loads(self):
        # Each list overlay_logs names for an age of log is the header a real
        # log of that age has, and score_compare loads that log.
        for name, _ in _RECORDED_PRESS_LOGS:
            path = _recorded(name)
            table = _table(path)
            self.assertTrue(table[1:], path)
            self.assertEqual(tuple(table[0]), getattr(overlay_logs, name), name)
            presses = score_compare.load_press_log(path)
            self.assertEqual([p.index for p in presses],
                             [int(r[0]) for r in table[1:]], name)

    def test_each_press_header_starts_with_the_one_before_it(self):
        # A newer log only adds columns at the end, so each list in the
        # history starts with the one before it. The one rename, the older
        # verdict column to the send column, is read as overlay_logs names it.
        for older_name, newer_name in zip(_PRESS_HISTORY, _PRESS_HISTORY[1:]):
            older = getattr(overlay_logs, older_name)
            newer = getattr(overlay_logs, newer_name)
            # Two ages of log in a row never share one header.
            self.assertNotEqual(newer, older, (older_name, newer_name))
            if overlay_logs.SEND_COLUMN in newer and overlay_logs.HIT_COLUMN not in newer:
                older = tuple(overlay_logs.SEND_COLUMN if c == overlay_logs.HIT_COLUMN
                              else c for c in older)
            self.assertEqual(newer[:len(older)], older, (older_name, newer_name))

    def test_score_compare_loads_every_press_header_in_the_history(self):
        # The two ages no run left a log of load too: a real newer log cut to
        # that age's columns reads to the same presses as the real log.
        sources = {"PRESS_COLUMNS_WITHOUT_WALL": "PRESS_COLUMNS_WITHOUT_PLAYER",
                   "PRESS_COLUMNS_WITH_HIT": "PRESS_COLUMNS_WITHOUT_RETIME"}
        for name in _PRESS_HISTORY:
            if name in dict(_RECORDED_PRESS_LOGS):
                continue
            self.assertIn(name, sources, name)
            real = _recorded(sources[name])
            columns = getattr(overlay_logs, name)
            with open(real, encoding="utf-8", newline="") as f:
                rows = list(csv.DictReader(f))
            with tempfile.TemporaryDirectory() as tmp:
                cut = os.path.join(tmp, "press.csv")
                with open(cut, "w", encoding="utf-8", newline="") as f:
                    writer = csv.DictWriter(f, fieldnames=columns, extrasaction="ignore",
                                            restval="")
                    writer.writeheader()
                    writer.writerows(rows)
                got = score_compare.load_press_log(cut)
            want = score_compare.load_press_log(real)
            self.assertEqual(
                [(p.index, p.chart_ms, p.clock_ms_at_press, p.score_before, p.score_after)
                 for p in got],
                [(p.index, p.chart_ms, p.clock_ms_at_press, p.score_before, p.score_after)
                 for p in want], name)


if __name__ == "__main__":
    unittest.main()
