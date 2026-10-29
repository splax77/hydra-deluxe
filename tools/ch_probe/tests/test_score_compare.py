"""score_compare.py sets a press log's scores against Hydra's running total.

Every fixture is hand-written: six chords whose Hydra totals rise by 100,
with a ghost note on the chord at index 2, and press logs that match them,
lag them, fall short, or change at the ghost chord. The rule each verdict
follows is score_compare's; these tests pin its answers on these literals.
"""

from __future__ import annotations

import io
import os
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.experiments import play_chart, score_compare  # noqa: E402


def _note(dynamic="none"):
    return {"color": "Red", "cymbal": False, "dynamic": dynamic}


# Six chords, 500 ms apart; Hydra's total after each is 100, 200, ... 600.
# The chord at index 2 carries a ghost note.
_SCORE = {
    "chords": [
        {"index": i, "ms": 1000.0 + 500.0 * i, "cum": {"total": 100 * (i + 1)},
         "notes": [_note("ghost" if i == 2 else "none")]}
        for i in range(6)
    ],
    "final": {"total": 600},
}


def _presses(game_scores, hits=None):
    """Press-log rows for the six chords, each pressed 1.5 ms early, with the
    game's score after each press as given."""
    hits = [True] * len(game_scores) if hits is None else hits
    rows, before = [], 0
    for i, (after, hit) in enumerate(zip(game_scores, hits)):
        ms = 1000.0 + 500.0 * i
        rows.append(score_compare.Press(i, ms, ms - 1.5, before, after, hit))
        before = after
    return rows


def _statuses(game_scores):
    verdicts = score_compare.compare_scores(
        _presses(game_scores), score_compare.chords_from_score(_SCORE))
    return [v.status for v in verdicts]


def _report(presses):
    out = io.StringIO()
    passed = score_compare.report(presses, _SCORE, out)
    return passed, out.getvalue()


class ScoreCheckTest(unittest.TestCase):
    def test_a_matching_log_passes(self):
        self.assertEqual(_statuses([100, 200, 300, 400, 500, 600]), ["equal"] * 6)
        passed, text = _report(_presses([100, 200, 300, 400, 500, 600]))
        self.assertTrue(passed)
        self.assertIn("PASS score", text)
        self.assertIn("Final: game 600, Hydra 600 at chord index 5; "
                      "Hydra's whole-chart total 600", text)

    def test_a_log_one_chord_behind_passes(self):
        self.assertEqual(_statuses([0, 100, 200, 300, 400, 500]), ["lagged"] * 6)

    def test_fifty_short_from_chord_3_fails_there_once(self):
        self.assertEqual(_statuses([100, 200, 300, 350, 450, 550]),
                         ["equal", "equal", "equal", "FAIL", "equal", "equal"])
        passed, text = _report(_presses([100, 200, 300, 350, 450, 550]))
        self.assertFalse(passed)
        self.assertIn("FAIL score at chord index 3: game 350, Hydra 400 "
                      "(difference -50, was +0)", text)

    def test_a_change_at_the_ghost_chord_passes(self):
        self.assertEqual(_statuses([100, 200, 280, 380, 480, 580]),
                         ["equal", "equal", "ghost_accent", "equal", "equal", "equal"])
        passed, text = _report(_presses([100, 200, 280, 380, 480, 580]))
        self.assertTrue(passed)
        self.assertIn("chord index 2: -20 (was +0)", text)

    def test_a_change_at_a_plain_chord_after_the_ghost_fails_there(self):
        self.assertEqual(_statuses([100, 200, 280, 380, 430, 530]),
                         ["equal", "equal", "ghost_accent", "equal", "FAIL", "equal"])
        passed, text = _report(_presses([100, 200, 280, 380, 430, 530]))
        self.assertFalse(passed)
        self.assertIn("FAIL score at chord index 4: game 430, Hydra 500 "
                      "(difference -70, was -20)", text)

    def test_a_ghost_chord_read_late_may_change_the_next_chord(self):
        # The ghost chord's read lags; its -20 shows on the chord after it.
        self.assertEqual(_statuses([100, 200, 200, 380, 480, 580]),
                         ["equal", "equal", "lagged", "ghost_accent", "equal", "equal"])

    def test_the_late_shown_change_is_printed(self):
        passed, text = _report(_presses([100, 200, 200, 380, 480, 580]))
        self.assertTrue(passed)
        self.assertIn("chord index 2: +0 (was +0, read lagged)", text)
        self.assertIn("chord index 3: -20 (was +0, the chord before was read "
                      "lagged)", text)

    def test_a_change_after_a_lagged_plain_chord_fails(self):
        # Chord 4 is plain and read lagged; the -20 at chord 5 has no ghost.
        self.assertEqual(_statuses([100, 200, 300, 400, 400, 480]),
                         ["equal", "equal", "equal", "equal", "lagged", "FAIL"])

    def test_a_press_not_in_the_score_json_is_an_error(self):
        press = score_compare.Press(99, 0.0, 0.0, 0, 100, True)
        with self.assertRaises(ValueError):
            score_compare.compare_scores([press], score_compare.chords_from_score(_SCORE))


def _late_presses(first_index, game_after, game_before=0):
    """Press-log rows for the chords from `first_index` on, as if the run
    joined there with the game's score at `game_before`."""
    rows, before = [], game_before
    for i, after in enumerate(game_after, start=first_index):
        ms = 1000.0 + 500.0 * i
        rows.append(score_compare.Press(i, ms, ms - 1.5, before, after, after > before))
        before = after
    return rows


class MidSongJoinTest(unittest.TestCase):
    def test_a_log_that_starts_late_starts_the_difference_at_its_own_offset(self):
        # Joined at chord 3 with the game's score at 0; Hydra's total before
        # that chord is 300, so the difference starts at -300.
        presses = _late_presses(3, [100, 200, 300])
        chords = score_compare.chords_from_score(_SCORE)
        self.assertEqual(score_compare.starting_difference(presses, chords), (-300, True))
        self.assertEqual([v.status for v in score_compare.compare_scores(presses, chords)],
                         ["equal"] * 3)
        passed, text = _report(presses)
        self.assertTrue(passed)
        self.assertIn("The log starts at chord index 3, not the chart's first chord: "
                      "the running difference starts at -300", text)

    def test_a_late_log_still_fails_on_a_plain_change(self):
        presses = _late_presses(3, [100, 150, 250])
        chords = score_compare.chords_from_score(_SCORE)
        self.assertEqual([v.status for v in score_compare.compare_scores(presses, chords)],
                         ["equal", "FAIL", "equal"])

    def test_a_log_from_the_first_chord_starts_at_zero_and_says_nothing(self):
        presses = _presses([100, 200, 300, 400, 500, 600])
        chords = score_compare.chords_from_score(_SCORE)
        self.assertEqual(score_compare.starting_difference(presses, chords), (0, False))
        self.assertNotIn("not the chart's first chord", _report(presses)[1])


def _hundred_presses(misses):
    return [score_compare.Press(i, 10.0 * i, 10.0 * i, 0, 0, i not in misses)
            for i in range(100)]


class HitRateTest(unittest.TestCase):
    def test_min_hit_percent_is_98(self):
        self.assertEqual(score_compare.MIN_HIT_PERCENT, 98)

    def test_97_percent_fails_naming_the_first_miss(self):
        passed, line = score_compare.hit_rate_line(_hundred_presses({17, 40, 71}))
        self.assertFalse(passed)
        self.assertEqual(line, "FAIL hit rate 97.0% (97 of 100), under 98%; "
                               "first miss: chord index 17 at 170.0 ms")

    def test_exactly_98_percent_passes(self):
        passed, line = score_compare.hit_rate_line(_hundred_presses({17, 40}))
        self.assertTrue(passed)
        self.assertEqual(line, "PASS hit rate 98.0% (98 of 100)")


def _simulated_log(n, misses=(), lags=()):
    """A press log for `n` chords worth 100 points each and no ghost or accent,
    with the chords in `misses` never credited and the chords in `lags` read
    before the engine credited them (the credit lands before the next read).
    Returns the presses and the score JSON they were played against."""
    score = {"chords": [{"index": i, "ms": 1000.0 + 10.0 * i,
                         "cum": {"total": 100 * (i + 1)}, "notes": [_note()]}
                        for i in range(n)],
             "final": {"total": 100 * n}}
    presses, true_score, pending, before = [], 0, 0, 0
    for i in range(n):
        true_score += pending
        pending = 0
        own = 0 if i in misses else 100
        if i in lags:
            pending = own
        else:
            true_score += own
        ms = 1000.0 + 10.0 * i
        presses.append(score_compare.Press(i, ms, ms - 1.5, before, true_score,
                                           true_score > before))
        before = true_score
    return presses, score


class LaggedReadsTest(unittest.TestCase):
    def _hit_line(self, presses, score):
        out = io.StringIO()
        passed = score_compare.report(presses, score, out)
        line = next(l for l in out.getvalue().splitlines() if "hit rate" in l)
        return passed, line

    def test_lagged_reads_are_not_misses(self):
        # Three of 100 reads land before the credit, so the log's own hit
        # column says 97 of 100; the next press shows each was credited.
        presses, score = _simulated_log(100, lags={20, 50, 80})
        self.assertEqual(sum(p.hit for p in presses), 97)
        passed, line = self._hit_line(presses, score)
        self.assertTrue(passed)
        self.assertEqual(line, "PASS hit rate 100.0% (100 of 100; 3 lagged reads "
                               "counted as hits)")

    def test_real_misses_still_fail_the_hit_rate(self):
        presses, score = _simulated_log(100, misses={20, 50, 80})
        passed, line = self._hit_line(presses, score)
        self.assertFalse(passed)
        self.assertEqual(line, "FAIL hit rate 97.0% (97 of 100), under 98%; "
                               "first miss: chord index 20 at 1200.0 ms")

    def test_a_lag_and_a_miss_in_one_log(self):
        presses, score = _simulated_log(100, misses={50}, lags={20})
        passed, line = self._hit_line(presses, score)
        self.assertTrue(passed)
        self.assertEqual(line, "PASS hit rate 99.0% (99 of 100; 1 lagged reads "
                               "counted as hits)")

    def test_the_log_hit_column_is_read_unchanged(self):
        presses, _score = _simulated_log(10, lags={4})
        self.assertEqual([p.hit for p in presses],
                         [True] * 4 + [False] + [True] * 5)

    def test_a_lag_on_the_last_press_counts_as_a_miss(self):
        presses, score = _simulated_log(10, lags={9})
        passed, line = self._hit_line(presses, score)
        self.assertFalse(passed)
        self.assertEqual(line, "FAIL hit rate 90.0% (9 of 10), under 98%; "
                               "first miss: chord index 9 at 1090.0 ms")


class PressTimeSpreadTest(unittest.TestCase):
    def test_median_and_percentiles(self):
        offsets = [-2.0] * 10 + [-1.5] * 8 + [3.0, 5.0, 40.0]
        self.assertEqual(score_compare.spread(offsets), (-1.5, -2.0, 5.0))

    def test_offsets_are_clock_minus_chart(self):
        self.assertEqual(score_compare.press_offsets_ms(_presses([100, 200])),
                         [-1.5, -1.5])

    def test_one_press(self):
        self.assertEqual(score_compare.spread([0.5]), (0.5, 0.5, 0.5))


class PressLogFileTest(unittest.TestCase):
    def test_reads_the_log_play_chart_writes(self):
        rows = [{"index": 2, "chart_ms": 2000.0, "clock_ms_at_press": 1998.5,
                 "score_before": 200, "score_after": 280, "hit": 1}]
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "press.csv")
            play_chart.write_press_log(path, rows)
            self.assertEqual(score_compare.load_press_log(path), [
                score_compare.Press(2, 2000.0, 1998.5, 200, 280, True)])

    def test_a_wrong_header_is_refused(self):
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "press.csv")
            with open(path, "w", encoding="utf-8", newline="") as f:
                f.write("index,ms\n1,2\n")
            with self.assertRaises(ValueError):
                score_compare.load_press_log(path)


if __name__ == "__main__":
    unittest.main()
