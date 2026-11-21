"""Tests for hydra_end, the one reader of Hydra's song end, and the fake
hydra_replay dump the other tool tests use.

No test runs hydra_replay (agent rule 14). The tests here read real dumps
and schedules hydra_replay wrote into testdata/replay and testdata/ch_runs.
FakeDumps stands in for hydra_end.make_dump in the other tool tests: a test
names a chart's song end with FakeDumps.set_end, and the fake writes a dump
that holds only that field, which the real hydra_end.song_length_ms then
reads.
"""

from __future__ import annotations

import glob
import json
import os
import shutil
import sys
import tempfile
import unittest
from unittest import mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import hydra_end  # noqa: E402
from tools.ch_probe import prep_inputs  # noqa: E402

_ABSENT = object()
_REAL_MAKE_DUMP = hydra_end.make_dump


def _key(chart: str) -> str:
    return os.path.normcase(os.path.abspath(chart))


class FakeDumps:
    """hydra_end.make_dump and hydra_end.DUMP_ROOT replaced for a test module:
    start() in setUpModule, stop() in tearDownModule. A chart with no end set
    fails the way a failed hydra_replay dump does."""

    def __init__(self):
        self.ends = {}
        self.charts = []  # every chart a dump was made for, in order
        self._root = None
        self._patches = []

    def set_end(self, chart: str, value=_ABSENT) -> None:
        """The dump of `chart` holds `value` as its song_length_ms; with no
        value it holds no such field."""
        self.ends[_key(chart)] = value

    def make_dump(self, chart: str, dump: str) -> None:
        self.charts.append(chart)
        if _key(chart) not in self.ends:
            raise prep_inputs.ReplayFailed(
                f"hydra_replay dump failed with exit code 1:\nno fake dump for {chart}")
        value = self.ends[_key(chart)]
        with open(dump, "w", encoding="utf-8") as f:
            json.dump({} if value is _ABSENT else {hydra_end.FIELD: value}, f)

    def start(self) -> "FakeDumps":
        self._root = tempfile.mkdtemp(prefix="fake_dumps_")
        self._patches = [mock.patch.object(hydra_end, "make_dump", self.make_dump),
                         mock.patch.object(hydra_end, "DUMP_ROOT", self._root)]
        for p in self._patches:
            p.start()
        return self

    def stop(self) -> None:
        for p in reversed(self._patches):
            p.stop()
        shutil.rmtree(self._root, ignore_errors=True)


FAKE = FakeDumps()


def setUpModule():
    FAKE.start()


def tearDownModule():
    FAKE.stop()


# Every dump and schedule hydra_replay wrote into the checked-in test data.
# Each folder's commit says which chart and command made its files.
REAL_HYDRA_FILES = sorted(
    glob.glob(os.path.join(_REPO_ROOT, "testdata", "replay", "*-dump.json"))
    + glob.glob(os.path.join(_REPO_ROOT, "testdata", "replay", "*-schedule.json"))
    + glob.glob(os.path.join(_REPO_ROOT, "testdata", "ch_runs", "*", "schedule.json")))
# One real dump, written by `hydra_replay dump` on the Window Map test chart
# (commit 77984463), for the tests that need a dump in place of make_dump's.
REAL_DUMP = os.path.join(_REPO_ROOT, "testdata", "replay", "window_map-dump.json")


class HydraEndTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.tmp = tmp.name

    def _file(self, text: str) -> str:
        path = os.path.join(self.tmp, "doc.json")
        with open(path, "w", encoding="utf-8") as f:
            f.write(text)
        return path

    def _real_dump_with(self, value=_ABSENT) -> dict:
        """The real dump REAL_DUMP, with its song end replaced by `value`, or
        dropped when no value is given."""
        with open(REAL_DUMP, encoding="utf-8") as f:
            doc = json.load(f)
        del doc[hydra_end.FIELD]
        if value is not _ABSENT:
            doc[hydra_end.FIELD] = value
        return doc

    def test_hydra_end_refuses_null_or_missing(self):
        # Each refused file is named in the refusal, and so is the field for
        # the files that are JSON objects.
        cases = (("null", json.dumps(self._real_dump_with(None)), True),
                 ("absent", json.dumps(self._real_dump_with()), True),
                 ("text", json.dumps(self._real_dump_with("4000")), True),
                 ("bool", json.dumps(self._real_dump_with(True)), True),
                 ("not an object", "[1]", False),
                 ("cut short", "{", False))
        for name, text, names_field in cases:
            with self.subTest(case=name):
                path = self._file(text)
                with self.assertRaises(hydra_end.SongEndMissing) as caught:
                    hydra_end.song_length_ms(path)
                self.assertIn(path, str(caught.exception))
                if names_field:
                    self.assertIn(hydra_end.FIELD, str(caught.exception))
        missing = os.path.join(self.tmp, "nowhere.json")
        with self.assertRaises(hydra_end.SongEndMissing) as caught:
            hydra_end.song_length_ms(missing)
        self.assertIn(missing, str(caught.exception))

    def test_the_end_is_the_number_hydra_replay_wrote(self):
        # Every real dump and schedule: the end comes back as written, with
        # its type kept, so a rounding, scaling or int() cast goes red even
        # on a whole-number end.
        self.assertTrue(REAL_HYDRA_FILES, "no hydra_replay files under testdata")
        for path in REAL_HYDRA_FILES:
            with self.subTest(file=os.path.relpath(path, _REPO_ROOT)):
                with open(path, encoding="utf-8") as f:
                    written = json.load(f)[hydra_end.FIELD]
                got = hydra_end.song_length_ms(path)
                self.assertIs(type(got), type(written))
                self.assertEqual(got, written)

    def _dump_root(self) -> str:
        """A fresh, empty folder for chart_song_length_ms's scratch folders."""
        root = os.path.join(self.tmp, "dumps")
        os.makedirs(root)
        patch = mock.patch.object(hydra_end, "DUMP_ROOT", root)
        patch.start()
        self.addCleanup(patch.stop)
        return root

    def test_chart_song_length_ms_reads_the_dump_it_made(self):
        chart = os.path.join(self.tmp, "Song", "notes.chart")
        FAKE.set_end(chart, 193934.0)
        seen = []
        real = hydra_end.song_length_ms

        def read(dump):
            seen.append(dump)
            return real(dump)

        with mock.patch.object(hydra_end, "song_length_ms", read):
            ms, source = hydra_end.chart_song_length_ms(chart)
        self.assertEqual(ms, 193934.0)
        self.assertEqual([os.path.basename(d) for d in seen], ["Song-dump.json"])
        self.assertEqual(source, hydra_end.chart_source_text(chart))
        self.assertEqual(source,
                         f"song_length_ms in a hydra_replay dump of {os.path.abspath(chart)}")

    def _missing_exe(self):
        """The real make_dump, run with prep_inputs' default exe pointed at a
        file that does not exist, so the real runner fails the way it does
        when hydra_replay was never built."""
        for patch in (mock.patch.object(hydra_end, "make_dump", _REAL_MAKE_DUMP),
                      mock.patch.object(prep_inputs, "DEFAULT_EXE",
                                        os.path.join(self.tmp, "no_build", "hydra_replay.exe"))):
            patch.start()
            self.addCleanup(patch.stop)

    @staticmethod
    def _dump_written(doc: dict):
        """A patch under which make_dump writes `doc` where the dump goes, in
        place of a hydra_replay run."""
        def write(chart, dump):
            with open(dump, "w", encoding="utf-8") as f:
                json.dump(doc, f)
        return mock.patch.object(hydra_end, "make_dump", write)

    def test_chart_song_length_ms_leaves_no_scratch_folder_behind(self):
        root = self._dump_root()
        chart = os.path.join(self.tmp, "Kept", "notes.chart")
        with open(REAL_DUMP, encoding="utf-8") as f:
            real = json.load(f)
        with self._dump_written(real):
            ms, _ = hydra_end.chart_song_length_ms(chart)
        self.assertEqual(ms, real[hydra_end.FIELD])
        self.assertEqual(os.listdir(root), [])
        # Nor when the dump it made names no end.
        for name, doc in (("Null", self._real_dump_with(None)),
                          ("Absent", self._real_dump_with())):
            with self.subTest(case=name):
                with self._dump_written(doc):
                    with self.assertRaises(hydra_end.SongEndMissing):
                        hydra_end.chart_song_length_ms(chart)
                self.assertEqual(os.listdir(root), [])
        # Nor when hydra_replay cannot be run at all.
        self._missing_exe()
        with self.assertRaises(hydra_end.SongEndMissing):
            hydra_end.chart_song_length_ms(chart)
        self.assertEqual(os.listdir(root), [])

    def test_a_failed_dump_is_no_song_end_with_hydra_replays_lines(self):
        chart = os.path.join(self.tmp, "Unset", "notes.chart")
        self._missing_exe()
        with self.assertRaises(hydra_end.SongEndMissing) as caught:
            hydra_end.chart_song_length_ms(chart)
        # The refusal names the chart and carries the runner's own lines.
        cause = caught.exception.__cause__
        self.assertIsInstance(cause, prep_inputs.ReplayFailed)
        self.assertIn(chart, str(caught.exception))
        self.assertIn(str(cause), str(caught.exception))


if __name__ == "__main__":
    unittest.main()
