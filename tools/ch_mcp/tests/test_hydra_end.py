"""Tests for hydra_end, the one reader of Hydra's song end, and the fake
hydra_replay dump the other tool tests use.

No test runs hydra_replay (agent rule 14). FakeDumps stands in for
hydra_end.make_dump: a test names a chart's song end with FakeDumps.set_end,
and the fake writes a dump that holds only that field, which the real
hydra_end.song_length_ms then reads.
"""

from __future__ import annotations

import json
import os
import re
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


# The five files whose song length used to come from song.ini or the probe
# formula (the player audit fix plan's task F10).
READER_FILES = ("bot_run.py", "game_tools.py", "sp_tools.py", "install_chart.py",
                "game_files.py")
_CH_MCP = os.path.join(_REPO_ROOT, "tools", "ch_mcp")


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

    def test_hydra_end_refuses_null_or_missing(self):
        for text, said in (('{"song_length_ms": null}', "song_length_ms is null"),
                           ('{"paths": []}', "has no song_length_ms field"),
                           ('{"song_length_ms": "4000"}', "not a number"),
                           ('{"song_length_ms": true}', "not a number"),
                           ('[1]', "is not a JSON object"),
                           ('{', "could not read")):
            with self.subTest(text=text):
                with self.assertRaises(hydra_end.SongEndMissing) as caught:
                    hydra_end.song_length_ms(self._file(text))
                self.assertIn(said, str(caught.exception))
        with self.assertRaises(hydra_end.SongEndMissing):
            hydra_end.song_length_ms(os.path.join(self.tmp, "nowhere.json"))

    def test_the_end_is_the_number_as_written(self):
        # The Senri schedule fixture's song end.
        self.assertEqual(hydra_end.song_length_ms(self._file('{"song_length_ms": 260504.0}')),
                         260504.0)

    def test_make_dump_runs_prep_inputs_dump_command_with_the_default_exe(self):
        # The module's own make_dump, kept before FakeDumps patches over it.
        with mock.patch.object(prep_inputs, "run_replay") as run:
            _REAL_MAKE_DUMP("C:/songs/x/notes.chart", "C:/out/x-dump.json")
        run.assert_called_once_with(prep_inputs.dump_command(
            prep_inputs.DEFAULT_EXE, "C:/songs/x/notes.chart", "C:/out/x-dump.json"))

    def test_chart_song_length_ms_reads_the_dump_it_made(self):
        chart = os.path.join(self.tmp, "Song", "notes.chart")
        FAKE.set_end(chart, 193934.0)
        ms, dump = hydra_end.chart_song_length_ms(chart)
        self.assertEqual(ms, 193934.0)
        self.assertEqual(os.path.basename(dump), "Song-dump.json")
        self.assertTrue(os.path.isfile(dump))
        self.assertEqual(hydra_end.source_text(dump), f"song_length_ms in {os.path.abspath(dump)}")

    def test_a_failed_dump_is_no_song_end_with_hydra_replays_lines(self):
        chart = os.path.join(self.tmp, "Unset", "notes.chart")
        with self.assertRaises(hydra_end.SongEndMissing) as caught:
            hydra_end.chart_song_length_ms(chart)
        self.assertIn("hydra_replay dump failed with exit code 1", str(caught.exception))

    def test_every_length_reader_calls_hydra_end(self):
        texts = {}
        for name in READER_FILES:
            with open(os.path.join(_CH_MCP, name), encoding="utf-8") as f:
                texts[name] = f.read()
        for name, text in texts.items():
            with self.subTest(file=name):
                self.assertNotIn("song_ini_length_ms", text)
                self.assertNotIn("probe_songs.song_length_ms", text)
                # A read of song.ini's own length key, quoted.
                self.assertIsNone(re.search(r"""["']song_length["']""", text))
        # The two that read a length themselves read it from hydra_end; the
        # other two take it from bot_run.song_length_ms, whose source the
        # behaviour tests in test_bot_run and test_game_tools pin.
        for name in ("bot_run.py", "install_chart.py"):
            with self.subTest(file=name):
                self.assertIn("hydra_end.chart_song_length_ms(", texts[name])
        for name in ("game_tools.py", "sp_tools.py"):
            with self.subTest(file=name):
                self.assertIn("bot_run.song_length_ms(", texts[name])


if __name__ == "__main__":
    unittest.main()
