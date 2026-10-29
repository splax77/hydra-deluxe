"""Tests for compare_scorestats on a hand-written scorestats.json and
hydra_replay score JSON pair.
"""

from __future__ import annotations

import json
import os
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import compare_scorestats as S  # noqa: E402


def _chord(colors, phrase_end=False):
    return {"notes": [{"color": c, "cymbal": False, "sp_points": 0} for c in colors],
            "is_sp_phrase_end": phrase_end}


# Three chords holding four gems; the second chord ends an SP phrase.
HYDRA = {
    "chords": [_chord(["Kick", "Red"]), _chord(["Yellow"], phrase_end=True), _chord(["Kick"])],
    "final": {"notes": 200, "combo": 400, "sp": 0, "solo": 0, "accent": 10, "ghost": 5,
              "total": 615},
}


def _stats(total_notes, sp_phrases_total, base_score=215, note_score=200):
    return {"players": [{"is_bot": True, "total_notes": total_notes,
                         "sp_phrases_total": sp_phrases_total,
                         "base_score": base_score, "note_score": note_score}]}


class CompareTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.hydra = self._json("hydra.json", HYDRA)

    def tearDown(self):
        self._tmp.cleanup()

    def _json(self, name, data):
        path = os.path.join(self._tmp.name, name)
        with open(path, "w", encoding="utf-8") as f:
            json.dump(data, f)
        return path

    def _run(self, stats):
        path = self._json("stats.json", stats)
        lines = []
        ok = S.compare(path, self.hydra, out=lines.append)
        return ok, lines, path

    def test_matching_pair_prints_three_lines(self):
        ok, lines, st = self._run(_stats(4, 1))
        self.assertTrue(ok)
        g, h = f"({st}, players[0])", f"({self.hydra})"
        self.assertEqual(lines, [
            f"total_notes: pass; game 4 {g}; Hydra gems 4, chords 3 {h}; matches gems",
            f"sp_phrases_total: pass; game 1 {g}; Hydra chords with is_sp_phrase_end 1 {h}",
            f"base_score (reported): game base_score 215, note_score 200 {g}; "
            f"Hydra final.notes 200, Hydra final.notes + accent + ghost 215, "
            f"Hydra final.total 615 {h}; "
            "base_score equals final.notes + accent + ghost; note_score equals final.notes",
        ])

    def test_chord_count_match_is_named(self):
        ok, lines, _ = self._run(_stats(3, 1))
        self.assertTrue(ok)
        self.assertTrue(lines[0].startswith("total_notes: pass;"))
        self.assertTrue(lines[0].endswith("matches chords"))

    def test_mismatch_is_flagged(self):
        ok, lines, _ = self._run(_stats(5, 0, base_score=1, note_score=2))
        self.assertFalse(ok)
        self.assertTrue(lines[0].startswith("total_notes: FAIL; game 5"))
        self.assertTrue(lines[0].endswith("matches neither"))
        self.assertTrue(lines[1].startswith("sp_phrases_total: FAIL; game 0"))
        self.assertTrue(lines[2].endswith("base_score equals none; note_score equals none"))

    def test_one_failing_line_fails_the_run(self):
        ok, _, _ = self._run(_stats(4, 0))
        self.assertFalse(ok)

    def test_a_missing_player_is_refused(self):
        path = self._json("stats.json", {"players": []})
        with self.assertRaises(ValueError):
            S.compare(path, self.hydra, out=lambda _l: None)


if __name__ == "__main__":
    unittest.main()
