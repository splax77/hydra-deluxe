"""Tests for the OCR text parser.

ocr.py holds only the pure parser, so that is all these tests cover. No saved
screen frame or OCR output exists, so the text these tests parse is the text the
game itself prints: its own display strings, taken from the build 6161 string
dump, filled in with hit offsets from a recorded run.

Run from the repo root:  python -m pytest tools/ch_probe/tests/test_ocr.py -q
Falls back to:           python -m unittest tools.ch_probe.tests.test_ocr
"""

from __future__ import annotations

import json
import os
import re
import statistics
import sys
import unittest

# Make the repo root importable so "from tools.ch_probe ..." works no matter
# where the test runner is launched from.
_REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.experiments.score_compare import load_press_log, press_offsets_ms
from tools.ch_probe.ocr import parse_accuracy_text

# Three entries copied unchanged from the game's string dump,
# hydra-data/ch-dumps/6161/stringliteral.json.
_GAME_STRINGS = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ch-dumps-6161",
                             "stringliteral-accuracy.json")
# The hit display's format string, as the game holds it.
_HIT_DISPLAY = "Accuracy: {0:0.0} ms\nAverage: {1:0.0} ms"
# A second accuracy line in the game, which prints the number with no unit.
_BARE_LINE = "Accuracy: {0}\n"
# A settings label that names accuracy but carries no number.
_LABEL = "Show Accuracy"
# The press log of the recorded run ls-e-run1 (copied from
# hydra-data/ch-mcp-runs/2026-11-07/ls-e-run1).
_PRESS_LOG = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-e-run1", "press-1.csv")


def _game_string(value: str) -> str:
    """The dumped game string equal to `value`; fails when the dump lacks it."""
    with open(_GAME_STRINGS, encoding="utf-8") as f:
        values = [e["value"] for e in json.load(f)]
    if value not in values:
        raise AssertionError(f"{_GAME_STRINGS} has no string {value!r}")
    return value


def _fill(fmt: str, *values: float) -> str:
    """A .NET composite format string filled the way the game fills it. A
    "{n:0.0}" slot prints one decimal; a bare "{n}" slot prints the number in
    its shortest form. Those are the only slot shapes the strings above use, so
    any other shape fails."""
    def slot(m: re.Match) -> str:
        value = values[int(m.group(1))]
        if m.group(2) is None:
            return repr(value)
        if m.group(2) == "0.0":
            return format(value, ".1f")
        raise AssertionError(f"slot format {m.group(2)!r} is not handled here")
    return re.sub(r"\{(\d+)(?::([^}]*))?\}", slot, fmt)


def _recorded_offsets() -> list:
    offsets = press_offsets_ms(load_press_log(_PRESS_LOG))
    if not offsets:
        raise AssertionError(f"{_PRESS_LOG} has no presses")
    return offsets


class ParseAccuracyTextTests(unittest.TestCase):

    def test_reads_the_accuracy_line_of_the_hit_display(self):
        # The hit display shows this hit's offset above the running average.
        # The parser must take the first, for every recorded press: late ones,
        # early ones, and the ones that print as 0.0 or -0.0.
        fmt = _game_string(_HIT_DISPLAY)
        offsets = _recorded_offsets()
        printed = [format(v, ".1f") for v in offsets]
        self.assertTrue(any(v < 0 for v in offsets), "the run has no early press")
        self.assertTrue(any(v > 0 for v in offsets), "the run has no late press")
        self.assertTrue(any(float(p) == 0.0 for p in printed),
                        "the run has no press that prints as zero")
        average = statistics.fmean(offsets)
        for offset, shown in zip(offsets, printed):
            text = _fill(fmt, offset, average)
            with self.subTest(text=text):
                self.assertEqual(parse_accuracy_text(text), float(shown))

    def test_a_number_with_no_unit_is_none(self):
        # The "ms" unit is required, so the game's bare accuracy line is not read.
        fmt = _game_string(_BARE_LINE)
        for offset in _recorded_offsets():
            text = _fill(fmt, offset)
            with self.subTest(text=text):
                self.assertIsNone(parse_accuracy_text(text))

    def test_text_with_no_number_is_none(self):
        # A label, and the display's format string before the game fills it.
        for value in (_LABEL, _HIT_DISPLAY):
            with self.subTest(text=value):
                self.assertIsNone(parse_accuracy_text(_game_string(value)))

    def test_empty_is_none(self):
        self.assertIsNone(parse_accuracy_text(""))

    def test_none_input_is_none(self):
        # The empty check catches None too; make sure it does not crash.
        self.assertIsNone(parse_accuracy_text(None))  # type: ignore[arg-type]


if __name__ == "__main__":
    unittest.main()
