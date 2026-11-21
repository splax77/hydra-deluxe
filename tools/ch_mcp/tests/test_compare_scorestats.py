"""Tests for compare_scorestats. The pass and one-line-fail cases read a real
run: the game's scorestats.json of a full combo on a corpus chart, against
hydra_replay's score JSON of the same chart. The refusal and mismatch cases
use a small hand-written pair.
"""

from __future__ import annotations

import contextlib
import copy
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
from tools.ch_mcp.dump_facts import file_md5  # noqa: E402

_TESTDATA = os.path.join(_REPO_ROOT, "testdata")

# The chart LS-O3 run 1 played: the corpus copy of Ladies Talk.
LADIES_TALK_CHART = os.path.join(
    _TESTDATA, "input", "common", "IB24", "T6",
    "Senri Kawaguchi - Ladies Talk [Kiyo Sen] (Drumeo Performance)", "notes.mid")
# The game's stats file from that run, a full combo on the virtual kit,
# copied whole from ch-mcp-runs 2026-11-07/ls-o3-run1.
LADIES_TALK_STATS = os.path.join(
    _TESTDATA, "ch_runs", "ls-o3-run1", "scorestats-Ladies_Talk_fill_check-20261107-155904.json")
# Hydra's score of the same chart, path 0 of its dump, made with
#   hydra_replay dump --chart <LADIES_TALK_CHART> --out testdata/replay/ladies_talk-dump.json
#   hydra_replay score --chart <LADIES_TALK_CHART> --path testdata/replay/ladies_talk-dump.json
#       --index 0 --out testdata/replay/ladies_talk-score.json
LADIES_TALK_SCORE = os.path.join(_TESTDATA, "replay", "ladies_talk-score.json")


def _load(path):
    with open(path, encoding="utf-8-sig") as f:
        return json.load(f)


def _real_run():
    """The run's settings. The checksum is the song id, the MD5 of the chart
    file (CR-10 in docs/clone-hero/01-reading-a-chart.md). No file records
    the other three for the run, so they are the ones its stats file names."""
    stats = _load(LADIES_TALK_STATS)
    player = stats["players"][0]
    return S.RunSettings(file_md5(LADIES_TALK_CHART).upper(), int(stats["playback_speed"]),
                         bool(player["no_fail"]), tuple(player["modifiers"]))


REAL_RUN = _real_run()


def _run_args(run):
    args = ["--checksum", run.checksum, "--speed", str(run.playback_speed),
            "--no-fail", "on" if run.no_fail else "off"]
    for name in run.modifiers:
        args += ["--modifier", name]
    return args


# The top-level keys of a stats file; every other key changed is the player's.
_TOP_LEVEL = ("checksum", "playback_speed")


def _real_stats(**changes):
    """The real run's stats file with `changes` made to it."""
    stats = copy.deepcopy(_load(LADIES_TALK_STATS))
    for key, value in changes.items():
        if key in _TOP_LEVEL:
            stats[key] = value
        else:
            stats["players"][0][key] = value
    return stats


def _real_player():
    return _load(LADIES_TALK_STATS)["players"][0]


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


def _stats(total_notes=4, notes_hit=4, sp_phrases_total=1, sp_phrases_earned=1,
           base_score=215, note_score=200, score=615, accents=1, ghosts=1,
           activations=1, checksum=REAL_RUN.checksum, playback_speed=REAL_RUN.playback_speed,
           no_fail=REAL_RUN.no_fail, modifiers=REAL_RUN.modifiers):
    return {"checksum": checksum, "playback_speed": playback_speed,
            "players": [{"is_bot": True, "total_notes": total_notes,
                         "notes_hit": notes_hit,
                         "sp_phrases_total": sp_phrases_total,
                         "sp_phrases_earned": sp_phrases_earned,
                         "base_score": base_score, "note_score": note_score,
                         "score": score, "accent_notes_hit": accents,
                         "ghost_notes_hit": ghosts, "sp_activations": activations,
                         "no_fail": no_fail, "modifiers": list(modifiers)}]}


CHECKED = ("total_notes", "notes_hit", "sp_phrases_total", "sp_phrases_earned",
           "checksum", "playback_speed", "no_fail", "modifiers", "score",
           "accent_notes_hit, ghost_notes_hit", "sp_activations")


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

    def _real_score(self):
        """Hydra's score JSON of the real run's chart. Missing, the test
        fails: nothing else can stand in for it."""
        if not os.path.isfile(LADIES_TALK_SCORE):
            self.fail(f"{LADIES_TALK_SCORE} is missing; make it with the hydra_replay "
                      "commands above LADIES_TALK_SCORE")
        return LADIES_TALK_SCORE

    def _real_counts(self):
        return S.hydra_counts(_load(self._real_score()))

    def _run(self, stats, run=REAL_RUN):
        path = self._json("stats.json", stats)
        lines = []
        ok = S.compare(path, self.hydra, run=run, out=lines.append)
        return ok, lines, path

    def _main(self, stats, extra=None):
        """The script's exit code and its printed lines, against Hydra's
        score of the real run's chart."""
        if extra is None:
            extra = _run_args(REAL_RUN)
        score = self._real_score()
        path = self._json("stats.json", stats)
        printed = io.StringIO()
        with contextlib.redirect_stdout(printed):
            code = S.main([path, score] + list(extra))
        return code, printed.getvalue().splitlines()

    def _verdicts(self, lines):
        """Each checked line's name and word. The reported line has neither."""
        pairs = [line.split(";")[0].rsplit(": ", 1) for line in lines]
        return {name: word for name, word in pairs if word in ("pass", "FAIL")}

    def test_matching_pair_prints_every_line(self):
        score = self._real_score()
        h = S.hydra_counts(_load(score))
        stats = _load(LADIES_TALK_STATS)
        p = stats["players"][0]
        lines = []
        ok = S.compare(LADIES_TALK_STATS, score, run=REAL_RUN, out=lines.append)
        self.assertTrue(ok)
        g, hs = f"({LADIES_TALK_STATS}, players[0])", f"({score})"
        # Each line's game number and Hydra (or run) number, each beside the
        # file it came from.
        sides = {
            "total_notes": (f"game {p['total_notes']} {g}", f" {h.gems} {hs}"),
            "notes_hit": (f"game {p['notes_hit']} {g}", f" {h.gems} {hs}"),
            "sp_phrases_total": (f"game {p['sp_phrases_total']} {g}", f" {h.sp_phrase_ends} {hs}"),
            "sp_phrases_earned": (f"game {p['sp_phrases_earned']} {g}", f" {h.sp_phrase_ends} {hs}"),
            "checksum": (f"game {stats['checksum']} {g}", f"run {REAL_RUN.checksum}"),
            "playback_speed": (f"game {stats['playback_speed']} {g}", f"run {REAL_RUN.playback_speed}"),
            "no_fail": (f"game {p['no_fail']} {g}", f"run {REAL_RUN.no_fail}"),
            "modifiers": (f"game {tuple(p['modifiers'])} {g}", f"run {REAL_RUN.modifiers}"),
            "score": (f"game {p['score']} {g}", f" {h.total} {hs}"),
            "accent_notes_hit, ghost_notes_hit": (
                f"game {p['accent_notes_hit']} accents, {p['ghost_notes_hit']} ghosts {g}",
                f"accent {h.accent_gems}, ghost {h.ghost_gems} {hs}"),
            "sp_activations": (f"game {p['sp_activations']} {g}", f" {h.activations} {hs}"),
        }
        self.assertEqual(len(lines), len(CHECKED) + 1)
        for name, line in zip(CHECKED, lines):
            with self.subTest(name=name):
                self.assertTrue(line.startswith(f"{name}: pass; "), line)
                game, other = sides[name]
                self.assertIn(game, line)
                self.assertIn(other, line)
        reported = lines[-1]
        self.assertTrue(reported.startswith("base_score (reported): "), reported)
        self.assertIn(f"base_score {p['base_score']}, note_score {p['note_score']} {g}", reported)
        self.assertIn(hs, reported)

    def test_compare_scorestats_fails_on_notes_hit_phrases_score_checksum_speed_or_modifiers(self):
        # Each case is the real run's file with one value moved off what the
        # game and the run recorded.
        p = _real_player()
        cases = {"notes_hit": dict(notes_hit=p["notes_hit"] - 1),
                 "sp_phrases_earned": dict(sp_phrases_earned=p["sp_phrases_earned"] - 1),
                 "score": dict(score=p["score"] - 1),
                 "checksum": dict(checksum="0" * len(REAL_RUN.checksum)),
                 "playback_speed": dict(playback_speed=REAL_RUN.playback_speed - 10),
                 "no_fail": dict(no_fail=not REAL_RUN.no_fail),
                 "modifiers": dict(modifiers=[])}
        for name, change in cases.items():
            with self.subTest(name=name):
                self._assert_only_failure(_real_stats(**change), name)

    def test_total_notes_equal_to_the_chord_count_fails(self):
        # The chart has chords of more than one gem, so its chord count is
        # not its gem count; the game's total_notes counts gems.
        h = self._real_counts()
        self.assertLess(h.chords, h.gems)
        code, lines = self._main(_real_stats(total_notes=h.chords))
        self.assertEqual(code, 1)
        self.assertTrue(lines[0].startswith(f"total_notes: FAIL; game {h.chords} "), lines[0])
        self.assertEqual([n for n, w in self._verdicts(lines).items() if w == "FAIL"],
                         ["total_notes"])

    def test_no_run_settings_fails_on_one_line(self):
        ok, lines, _ = self._run(_stats(), run=None)
        self.assertFalse(ok)
        self.assertIn("checksum, playback_speed, no_fail, modifiers: FAIL; the run's "
                      "settings were not given, so the game's cannot be checked "
                      "against them", lines)
        code, _ = self._main(_real_stats(), extra=[])
        self.assertEqual(code, 1)

    def test_mismatch_is_flagged(self):
        ok, lines, _ = self._run(_stats(5, sp_phrases_total=0, base_score=1, note_score=2))
        self.assertFalse(ok)
        self.assertTrue(lines[0].startswith("total_notes: FAIL; game 5"))
        self.assertTrue(lines[2].startswith("sp_phrases_total: FAIL; game 0"))
        self.assertTrue(lines[-1].endswith("base_score equals none; note_score equals none"))

    def test_a_missing_player_is_refused(self):
        path = self._json("stats.json", {"players": []})
        with self.assertRaises(ValueError):
            S.compare(path, self.hydra, run=REAL_RUN, out=lambda _l: None)

    def test_a_stats_file_with_no_score_is_refused(self):
        stats = _stats()
        del stats["players"][0]["score"]
        with self.assertRaises(KeyError):
            self._run(stats)

    def test_the_full_combo_run_passes_every_line_and_exits_0(self):
        code, lines = self._main(_real_stats())
        self.assertEqual(code, 0)
        self.assertEqual(self._verdicts(lines), {name: "pass" for name in CHECKED})

    def _assert_only_failure(self, stats, line_name):
        code, lines = self._main(stats)
        self.assertEqual(code, 1)
        verdicts = self._verdicts(lines)
        self.assertEqual(len(verdicts), len(CHECKED))
        self.assertEqual([name for name, word in verdicts.items() if word == "FAIL"],
                         [line_name])

    def test_one_accent_short_fails_the_accent_line(self):
        self._assert_only_failure(_real_stats(accent_notes_hit=_real_player()["accent_notes_hit"] - 1),
                                  "accent_notes_hit, ghost_notes_hit")

    def test_one_ghost_short_fails_the_accent_line(self):
        self._assert_only_failure(_real_stats(ghost_notes_hit=_real_player()["ghost_notes_hit"] - 1),
                                  "accent_notes_hit, ghost_notes_hit")

    def test_one_activation_more_fails_the_activation_line(self):
        self._assert_only_failure(_real_stats(sp_activations=_real_player()["sp_activations"] + 1),
                                  "sp_activations")


if __name__ == "__main__":
    unittest.main()
