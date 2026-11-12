"""score_compare.py sets each chord's score step in a press log against
Hydra's points for it.

Every fixture is hand-written: six chords whose Hydra totals rise by 100,
with a ghost note on the chord at index 2, and press logs that match them,
fall short, or differ at the ghost chord. The rule each verdict follows is
score_compare's; these tests pin its answers on these literals.
"""

from __future__ import annotations

import dataclasses
import io
import os
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.experiments import overlay_logs, play_chart, score_compare  # noqa: E402

KIT, KEYS = score_compare.ROUTE_KIT, score_compare.ROUTE_KEYS


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


def _presses(game_scores):
    """Press-log rows for the chords, each pressed 1.5 ms early, with the
    game's score after each press as given."""
    rows, before = [], 0
    for i, after in enumerate(game_scores):
        ms = 1000.0 + 500.0 * i
        rows.append(score_compare.Press(i, ms, ms - 1.5, before, after))
        before = after
    return rows


def _statuses(game_scores, route=KIT):
    verdicts = score_compare.compare_scores(
        _presses(game_scores), score_compare.chords_from_score(_SCORE), route)
    return [v.status for v in verdicts]


def _report(presses, route=KIT, stop=play_chart.STOP_DONE, message=""):
    out = io.StringIO()
    passed = score_compare.report(presses, _SCORE, out, route=route, stop=stop,
                                  stop_message=message)
    return passed, out.getvalue()


class ScoreStepTest(unittest.TestCase):
    def test_a_matching_log_passes(self):
        passed, text = _report(_presses([100, 200, 300, 400, 500, 600]))
        self.assertTrue(passed)
        self.assertIn("PASS score step: every chord's step equals Hydra's points", text)
        self.assertIn("Final: game 600 after chord index 5; Hydra's whole-chart total 600",
                      text)

    def test_a_chord_passes_only_when_the_step_equals_hydras_points(self):
        # Chord 3 steps by 99, one point under Hydra's 100.
        self.assertEqual(_statuses([100, 200, 300, 399, 499, 599]),
                         ["equal", "equal", "equal", "FAIL", "equal", "equal"])
        passed, text = _report(_presses([100, 200, 300, 399, 499, 599]))
        self.assertFalse(passed)
        self.assertIn("FAIL score step at chord index 3: game +99, Hydra +100", text)
        with open(score_compare.__file__, encoding="utf-8") as f:
            source = f.read()
        self.assertNotIn("MIN_HIT_PERCENT", source)
        # The judge reads neither the log's send column nor an older hit column.
        self.assertNotIn(".hit", source)
        self.assertNotIn('row["hit"]', source)

    def test_a_lagged_read_fails(self):
        # The read after chord 2 lands before the game credits it: its step is 0.
        self.assertEqual(_statuses([100, 200, 200, 400, 500, 600])[2], "FAIL")

    def test_ghost_accent_forgiveness_applies_only_on_the_keys_route(self):
        scores = [100, 200, 280, 380, 480, 580]
        self.assertEqual(_statuses(scores, KEYS)[2], "ghost_accent")
        self.assertEqual(_statuses(scores, KIT)[2], "FAIL")
        self.assertTrue(_report(_presses(scores), KEYS)[0])
        self.assertFalse(_report(_presses(scores), KIT)[0])

    def test_a_split_chord_is_one_step(self):
        p = _presses([100, 200, 300, 400, 500, 600])
        half = dataclasses.replace(p[3], score_after=350)
        rest = dataclasses.replace(p[3], score_before=350)
        rows = p[:3] + [half, rest] + p[4:]
        self.assertEqual(_statuses_of(rows), ["equal"] * 6)

    def test_a_chord_with_no_row_fails(self):
        p = _presses([100, 200, 300, 400, 500, 600])[:5]
        passed, text = _report(p)
        self.assertFalse(passed)
        self.assertIn("FAIL chords with no press-log row: 1 of 6 (chord index 5)", text)

    def test_an_unknown_route_is_refused(self):
        with self.assertRaises(ValueError):
            _statuses([100], "mouse")

    def test_the_route_words_are_game_tools(self):
        from tools.ch_mcp import game_tools
        self.assertEqual((KIT, KEYS), (game_tools.ROUTE_KIT, game_tools.ROUTE_KEYS))

    def test_a_press_not_in_the_score_json_is_an_error(self):
        with self.assertRaises(ValueError):
            score_compare.compare_scores(
                [score_compare.Press(9, 0.0, 0.0, 0, 0)],
                score_compare.chords_from_score(_SCORE), KIT)


def _statuses_of(rows):
    return [v.status for v in score_compare.compare_scores(
        rows, score_compare.chords_from_score(_SCORE), KIT)]


class RunEndTest(unittest.TestCase):
    def test_a_run_that_stopped_before_the_song_end_fails(self):
        full = _presses([100, 200, 300, 400, 500, 600])
        passed, text = _report(full, stop="cap", message="run cap of 240 s reached")
        self.assertFalse(passed)
        self.assertTrue(text.startswith(
            "FAIL run end: the run stopped before the song's end (cap): "
            "run cap of 240 s reached"))
        passed, text = _report(full, stop=None)
        self.assertFalse(passed)
        self.assertTrue(text.startswith("FAIL run end: the run's stop reason was not given"))
        passed, text = _report(full)
        self.assertTrue(passed)
        self.assertTrue(text.startswith("PASS run end: the run reached the song's end"))


class ClockLineTest(unittest.TestCase):
    def test_clock_line_wording(self):
        _, text = _report(_presses([100, 200, 300, 400, 500, 600]))
        self.assertIn("Clock at the read before the send minus chart ms: median -1.50, "
                      "5th percentile -1.50, 95th percentile -1.50 (6 presses)", text)


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
                score_compare.Press(2, 2000.0, 1998.5, 200, 280)])

    def test_a_wrong_header_is_refused(self):
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "press.csv")
            with open(path, "w", encoding="utf-8", newline="") as f:
                f.write("index,ms\n1,2\n")
            with self.assertRaises(ValueError):
                score_compare.load_press_log(path)


def _load_text(text):
    """load_press_log on a log holding exactly `text`."""
    with tempfile.TemporaryDirectory() as d:
        path = os.path.join(d, "press.csv")
        with open(path, "w", encoding="utf-8", newline="") as f:
            f.write(text)
        return score_compare.load_press_log(path)


class PressLogHeaderTest(unittest.TestCase):
    """Hand-written logs: each header load_press_log accepts, and headers it
    refuses."""

    def test_a_six_column_log_loads_with_blank_kit_fields(self):
        rows = _load_text(
            "index,chart_ms,clock_ms_at_press,score_before,score_after,hit\n"
            "0,3398.25,3398.75,0,115,1\n"
            "1,4616.5,4617.5,115,115,0\n")
        self.assertEqual(rows, [
            score_compare.Press(0, 3398.25, 3398.75, 0, 115, "", "", "", ""),
            score_compare.Press(1, 4616.5, 4617.5, 115, 115, "", "", "", ""),
        ])

    def test_a_ten_column_log_loads(self):
        rows = _load_text(
            "index,chart_ms,clock_ms_at_press,score_before,score_after,hit,"
            "down_frame,up_frame,down_ms,up_ms\n"
            "2,2000.0,1998.5,200,280,1,812,813,1998.25,2014.75\n"
            "3,2500.0,2501.0,280,380,1,,,,\n")
        self.assertEqual(rows, [
            score_compare.Press(2, 2000.0, 1998.5, 200, 280,
                                "812", "813", "1998.25", "2014.75"),
            score_compare.Press(3, 2500.0, 2501.0, 280, 380, "", "", "", ""),
        ])

    def test_a_twelve_column_log_loads_with_the_wall_times(self):
        rows = _load_text(
            ",".join(overlay_logs.PRESS_LOG_COLUMNS) + "\n"
            "2,2000.0,1998.5,200,280,1,812,813,1998.25,2014.75,1762500000.25,1762500000.5\n"
            "3,2500.0,2501.0,280,380,1,,,,,1762500000.75,1762500000.75\n")
        self.assertEqual(rows, [
            score_compare.Press(2, 2000.0, 1998.5, 200, 280,
                                "812", "813", "1998.25", "2014.75",
                                "1762500000.25", "1762500000.5"),
            score_compare.Press(3, 2500.0, 2501.0, 280, 380, "", "", "", "",
                                "1762500000.75", "1762500000.75"),
        ])

    def test_a_fifteen_column_log_from_before_the_retime_column_loads(self):
        rows = _load_text(
            "index,chart_ms,clock_ms_at_press,score_before,score_after,hit,"
            "down_frame,up_frame,down_ms,up_ms,wall_s_at_read,wall_s_at_reply,"
            "scored_between,end_read_ms,probe\n"
            "180,32583.33,32627.39,500,600,1,,,,,1.25,1.5,0,,\n")
        self.assertEqual(rows, [
            score_compare.Press(180, 32583.33, 32627.39, 500, 600, "", "", "", "",
                                "1.25", "1.5", "0", "", "", "")])

    def test_a_sixteen_column_log_keeps_the_retime_word(self):
        rows = _load_text(
            "index,chart_ms,clock_ms_at_press,score_before,score_after,hit,"
            "down_frame,up_frame,down_ms,up_ms,wall_s_at_read,wall_s_at_reply,"
            "scored_between,end_read_ms,probe,retimed\n"
            "180,32583.33,32624.39,500,600,1,,,,,1.25,1.5,0,,,inside\n"
            "181,32666.67,32665.39,600,700,1,,,,,1.75,2.0,0,,,\n")
        self.assertEqual([p.retimed for p in rows], ["inside", ""])

    def test_the_retime_word_play_chart_writes_reads_back(self):
        rows = [{"index": 180, "chart_ms": 32583.33, "clock_ms_at_press": 32627.39,
                 "score_before": 500, "score_after": 600, "hit": 1,
                 "retimed": play_chart.RETIMED_LATE}]
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "press.csv")
            play_chart.write_press_log(path, rows)
            (press,) = score_compare.load_press_log(path)
        self.assertEqual(press.retimed, "late")

    def test_press_has_a_field_for_every_log_column_but_the_send_column(self):
        self.assertEqual(
            tuple(f.name for f in dataclasses.fields(score_compare.Press)),
            tuple(c for c in overlay_logs.PRESS_LOG_COLUMNS
                  if c != overlay_logs.SEND_COLUMN))

    def test_a_log_with_the_header_before_timing_still_loads(self):
        # The header of LS-A2's press-1.csv (2026-11-11\ls-a2-run1), as written.
        rows = _load_text(
            "index,chart_ms,clock_ms_at_press,score_before,score_after,send,down_frame,"
            "up_frame,down_ms,up_ms,wall_s_at_read,wall_s_at_reply,scored_between,"
            "end_read_ms,probe,retimed\n"
            "180,32583.33,32624.39,500,600,sent,,,,,1.25,1.5,0,,,inside\n")
        self.assertEqual([(p.index, p.score_after, p.retimed, p.press_took_ms) for p in rows],
                         [(180, 600, "inside", "")])

    def test_the_timing_columns_play_chart_writes_read_back(self):
        rows = [{"index": 180, "chart_ms": 32583.33, "clock_ms_at_press": 32627.39,
                 "score_before": 500, "score_after": 600, "send": play_chart.SENT,
                 "wait_read_max_ms": 1.5, "press_took_ms": 212.4}]
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "press.csv")
            play_chart.write_press_log(path, rows)
            (press,) = score_compare.load_press_log(path)
        self.assertEqual((press.wait_read_max_ms, press.press_took_ms, press.end_read_took_ms),
                         ("1.5", "212.4", ""))

    def test_a_log_with_the_header_before_the_in_game_columns_still_loads(self):
        # The header play_chords wrote before task IG3 of
        # docs/superpowers/plans/2026-11-12-in-game-press-timing.md, LS-P3's
        # press-1.csv among them, as written.
        rows = _load_text(
            "index,chart_ms,clock_ms_at_press,score_before,score_after,send,down_frame,"
            "up_frame,down_ms,up_ms,wall_s_at_read,wall_s_at_reply,scored_between,"
            "end_read_ms,probe,retimed,wait_read_max_ms,wait_oversleep_max_ms,"
            "press_took_ms,reply_check_took_ms,end_read_took_ms,score_read_took_ms,"
            "prev_handoff_took_ms\n"
            "468,80000.0,80001.0,9000,9000,dropped,,,,,1.25,1.5,0,,,,87.3,,,,,,\n")
        self.assertEqual([(p.index, p.score_after, p.wait_read_max_ms, p.frame_gap_ms,
                           p.agent_note) for p in rows],
                         [(468, 9000, "87.3", "", "")])

    def test_the_in_game_columns_play_in_game_writes_read_back(self):
        rows = [{"index": 180, "chart_ms": 32583.33, "clock_ms_at_press": 32627.39,
                 "score_before": 500, "score_after": 600, "send": play_chart.SENT,
                 "frame_gap_ms": 3.25, "agent_note": "retime_missed"}]
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "press.csv")
            play_chart.write_press_log(path, rows)
            (press,) = score_compare.load_press_log(path)
        self.assertEqual((press.frame_gap_ms, press.agent_note, press.wait_read_max_ms),
                         ("3.25", "retime_missed", ""))

    def test_a_log_with_the_hit_column_still_loads(self):
        rows = _load_text(
            ",".join(overlay_logs.PRESS_COLUMNS_WITH_HIT) + "\n"
            "180,32583.33,32624.39,500,600,1,,,,,1.25,1.5,0,,,inside\n")
        self.assertEqual([(p.index, p.score_after, p.retimed) for p in rows],
                         [(180, 600, "inside")])

    def test_other_headers_are_refused(self):
        for header in (
            # the six-column header's names in another order
            "chart_ms,index,clock_ms_at_press,score_before,score_after,hit",
            # the six-column header plus only the first kit column
            "index,chart_ms,clock_ms_at_press,score_before,score_after,hit,down_frame",
            # the six-column header short one column
            "index,chart_ms,clock_ms_at_press,score_before,score_after",
            # the kit columns in another order
            "index,chart_ms,clock_ms_at_press,score_before,score_after,hit,"
            "up_frame,down_frame,down_ms,up_ms",
        ):
            with self.subTest(header=header):
                with self.assertRaisesRegex(ValueError, r"header \[.*\] is not \["):
                    _load_text(header + "\n")


if __name__ == "__main__":
    unittest.main()
