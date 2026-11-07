"""Tests for compare_scorestats on a hand-written scorestats.json and
hydra_replay score JSON pair.
"""

from __future__ import annotations

import contextlib
import io
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


def _chord(gems, phrase_end=False):
    """`gems` is a list of (color, dynamic) pairs."""
    return {"notes": [{"color": c, "cymbal": False, "sp_points": 0, "dynamic": d}
                      for c, d in gems],
            "is_sp_phrase_end": phrase_end}


# Three chords holding four gems, one ghost and one accent among them; the
# second chord ends an SP phrase. One activation.
HYDRA = {
    "chords": [_chord([("Kick", "none"), ("Red", "ghost")]),
               _chord([("Yellow", "accent")], phrase_end=True),
               _chord([("Kick", "none")])],
    "final": {"notes": 200, "combo": 400, "sp": 0, "solo": 0, "accent": 10, "ghost": 5,
              "total": 615},
    "activations": [{"act_tick": 100, "deact_tick": 200}],
}

# The Senri target's shape: the numbers LS-O5's pass line names, in a file of
# 368 accents, 726 ghosts, one plain kick and 8 activations.
TARGET = {
    "chords": ([_chord([("Yellow", "accent")]) for _ in range(368)]
               + [_chord([("Red", "ghost")]) for _ in range(726)]
               + [_chord([("Kick", "none")], phrase_end=True)]),
    "final": {"notes": 168235, "combo": 665075, "sp": 283720, "solo": 58000, "accent": 18400,
              "ghost": 36300, "total": 1229730},
    "activations": [{"act_tick": t, "deact_tick": t + 10} for t in range(8)],
}


def _stats(total_notes=4, sp_phrases_total=1, base_score=215, note_score=200, score=615,
           accents=1, ghosts=1, activations=1):
    return {"players": [{"is_bot": True, "total_notes": total_notes,
                         "sp_phrases_total": sp_phrases_total,
                         "base_score": base_score, "note_score": note_score,
                         "score": score, "accent_notes_hit": accents,
                         "ghost_notes_hit": ghosts, "sp_activations": activations}]}


def _target_stats(**changes):
    values = dict(total_notes=1095, sp_phrases_total=1, base_score=222935, note_score=164565,
                  score=1229730, accents=368, ghosts=726, activations=8)
    return _stats(**{**values, **changes})


class CompareTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.hydra = self._json("hydra.json", HYDRA)
        self.target = self._json("target.json", TARGET)

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

    def _main(self, stats):
        """The script's exit code and its printed lines, against TARGET."""
        path = self._json("stats.json", stats)
        printed = io.StringIO()
        with contextlib.redirect_stdout(printed):
            code = S.main([path, self.target])
        return code, printed.getvalue().splitlines()

    def _verdicts(self, lines):
        """Each checked line's name and word. The reported line has neither."""
        pairs = [line.split(";")[0].rsplit(": ", 1) for line in lines]
        return {name: word for name, word in pairs if word in ("pass", "FAIL")}

    def test_matching_pair_prints_six_lines(self):
        ok, lines, st = self._run(_stats())
        self.assertTrue(ok)
        g, h = f"({st}, players[0])", f"({self.hydra})"
        self.assertEqual(lines, [
            f"total_notes: pass; game 4 {g}; Hydra gems 4, chords 3 {h}; matches gems",
            f"sp_phrases_total: pass; game 1 {g}; Hydra chords with is_sp_phrase_end 1 {h}",
            f"score: pass; game 615 {g}; Hydra final.total 615 {h}",
            f"accent_notes_hit, ghost_notes_hit: pass; game 1 accents, 1 ghosts {g}; "
            f"Hydra gems with dynamic accent 1, ghost 1 {h}",
            f"sp_activations: pass; game 1 {g}; Hydra activations 1 {h}",
            f"base_score (reported): game base_score 215, note_score 200 {g}; "
            f"Hydra final.notes 200, Hydra final.notes + accent + ghost 215, "
            f"Hydra final.total 615 {h}; "
            "base_score equals final.notes + accent + ghost; note_score equals final.notes",
        ])

    def test_chord_count_match_is_named(self):
        ok, lines, _ = self._run(_stats(total_notes=3))
        self.assertTrue(ok)
        self.assertTrue(lines[0].startswith("total_notes: pass;"))
        self.assertTrue(lines[0].endswith("matches chords"))

    def test_mismatch_is_flagged(self):
        ok, lines, _ = self._run(_stats(5, 0, base_score=1, note_score=2))
        self.assertFalse(ok)
        self.assertTrue(lines[0].startswith("total_notes: FAIL; game 5"))
        self.assertTrue(lines[0].endswith("matches neither"))
        self.assertTrue(lines[1].startswith("sp_phrases_total: FAIL; game 0"))
        self.assertTrue(lines[5].endswith("base_score equals none; note_score equals none"))

    def test_one_failing_line_fails_the_run(self):
        ok, _, _ = self._run(_stats(sp_phrases_total=0))
        self.assertFalse(ok)

    def test_a_missing_player_is_refused(self):
        path = self._json("stats.json", {"players": []})
        with self.assertRaises(ValueError):
            S.compare(path, self.hydra, out=lambda _l: None)

    def test_a_stats_file_with_no_score_is_refused(self):
        stats = _stats()
        del stats["players"][0]["score"]
        with self.assertRaises(KeyError):
            self._run(stats)

    def test_the_target_run_passes_all_five_lines_and_exits_0(self):
        code, lines = self._main(_target_stats())
        self.assertEqual(code, 0)
        self.assertEqual(self._verdicts(lines), {
            "total_notes": "pass", "sp_phrases_total": "pass", "score": "pass",
            "accent_notes_hit, ghost_notes_hit": "pass", "sp_activations": "pass"})

    def _assert_only_failure(self, stats, line_name):
        code, lines = self._main(stats)
        self.assertEqual(code, 1)
        verdicts = self._verdicts(lines)
        self.assertEqual(len(verdicts), 5)
        self.assertEqual([name for name, word in verdicts.items() if word == "FAIL"],
                         [line_name])

    def test_one_point_short_fails_the_score_line_and_exits_1(self):
        self._assert_only_failure(_target_stats(score=1229729), "score")

    def test_one_accent_short_fails_the_accent_line(self):
        self._assert_only_failure(_target_stats(accents=367),
                                  "accent_notes_hit, ghost_notes_hit")

    def test_one_ghost_short_fails_the_accent_line(self):
        self._assert_only_failure(_target_stats(ghosts=725),
                                  "accent_notes_hit, ghost_notes_hit")

    def test_a_ninth_activation_fails_the_activation_line(self):
        self._assert_only_failure(_target_stats(activations=9), "sp_activations")


if __name__ == "__main__":
    unittest.main()
