"""score_compare.py sets each chord's score step in a press log against
Hydra's points for it.

The tests read real press logs, or pieces of them, copied from Clone Hero
runs under hydra-data/ch-mcp-runs into testdata/ch_runs, beside the
hydra_replay score JSON of the chart the run played; each file's comment
names the run. The header tests also feed load_press_log headers no run
wrote, which it must refuse.
"""

from __future__ import annotations

import csv
import dataclasses
import json
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

_RUNS_DIR = os.path.join(_REPO_ROOT, "testdata", "ch_runs")
_REPLAY_DIR = os.path.join(_REPO_ROOT, "testdata", "replay")

# Hydra's score JSON for Window Map, path 0, the path the Window Map runs
# below played (an all-kick chart with no activation). From the repo root:
#   build-cpp\Release\hydra_replay.exe score
#       --chart "testdata/ch_runs/2026-10-28-l3/Window Map/notes.chart"
#       --path testdata/replay/window_map-dump.json --index 0
#       --out testdata/replay/window_map-score.json
_WINDOW_MAP_SCORE = os.path.join(_REPLAY_DIR, "window_map-score.json")
# Hydra's score JSON for Pitch Dark, path 0, the path pitch-dark-run1
# played; the comment above _REPLAY_DIR in test_sp_compare.py says how it
# is made.
_PITCH_DARK_SCORE = os.path.join(_REPLAY_DIR, "test_pitch_dark_ratio-score.json")

# hydra-data/ch-mcp-runs/2026-11-07/ls-t2-run-a/press-1.csv, whole: a kit
# run on Window Map that the game scored as Hydra did at every chord.
_FULL_RUN_LOG = os.path.join(_RUNS_DIR, "ls-t2-run-a", "press-1.csv")
# hydra-data/ch-mcp-runs/2026-11-07/ls-t3-run-b/press-1.csv, the rows of
# chords 38 to 40: a kit run on Window Map that fell behind the chart. At
# chord 39 the game's step is short of Hydra's points, and the read after
# chord 40 shows no step at all.
_BEHIND_LOG = os.path.join(_RUNS_DIR, "ls-t3-run-b", "press-1-chords-38-40.csv")
# hydra-data/ch-mcp-runs/2026-11-12/pitch-dark-run1/press-1.csv, the rows of
# chords 143 to 145; the run pressed chord 144 as two presses.
_SPLIT_LOG = os.path.join(_RUNS_DIR, "pitch-dark-run1", "press-1-chords-143-145.csv")
# hydra-data/ch-mcp-runs/2026-11-11/ls-a2-run1/press-1.csv, the rows of
# chords 137 and 138, and the chords 136 to 138 of the score JSON that run
# played (hydra-data/ch-mcp-runs/2026-11-11/ls-a2-prep/The Who - Won't Get
# Fooled Again (O)-score.json), each chord unchanged. Chord 137 holds a
# ghost or accent note, and the game's step for it is short of Hydra's
# points.
_GHOST_LOG = os.path.join(_RUNS_DIR, "ls-a2-run1", "press-1-chords-137-138.csv")
_GHOST_SCORE = os.path.join(_RUNS_DIR, "ls-a2-prep", "score-chords-136-138.json")
# The same run's press-1.csv, the rows of chords 194 to 196, and the chords
# 193 to 196 of the same score JSON with its "final", each unchanged. The
# send of chord 196 was dropped and the game's score did not step; chords
# 194 and 195 were sent and stepped by Hydra's points.
_DROPPED_LOG = os.path.join(_RUNS_DIR, "ls-a2-run1", "press-1-chords-194-196.csv")
_DROPPED_SCORE = os.path.join(_RUNS_DIR, "ls-a2-prep", "score-chords-193-196.json")
# hydra-data/ch-mcp-runs/2026-11-12/ls-p3-run1/press-1.csv, its first three
# rows: a log with the timing columns filled in and no in-game columns.
_TIMING_LOG = os.path.join(_RUNS_DIR, "ls-p3-run1", "press-1-rows-1-3.csv")
# hydra-data/ch-mcp-runs/2026-11-11/ls-a2-run1/press-1.csv, its first three
# rows and row 1906, the one row the run retimed.
_RETIMED_LOG = os.path.join(_RUNS_DIR, "ls-a2-run1", "press-1-rows-1-3-1906.csv")


def _data_file(path):
    """`path`, a file under testdata; a missing one fails the test at once."""
    if not os.path.isfile(path):
        raise AssertionError(
            f"{path} is missing; the comment beside its name in "
            "tools/ch_probe/tests/test_score_compare.py says how it is made")
    return path


def _load_score(path):
    with open(_data_file(path), encoding="utf-8") as f:
        return json.load(f)


def _real_report(presses, score, route=KIT, stop=play_chart.STOP_DONE):
    out = io.StringIO()
    passed = score_compare.report(presses, score, out, route=route, stop=stop)
    return passed, out.getvalue().splitlines()


def _steps(presses):
    """{chord index: the game's score step}, read straight from the rows: the
    score after a chord's last row minus the score before its first."""
    first, last = {}, {}
    for p in presses:
        first.setdefault(p.index, p.score_before)
        last[p.index] = p.score_after
    return {i: last[i] - first[i] for i in first}


def _hydra_points(score):
    """{chord index: Hydra's points}, read straight from the score JSON: a
    chord's cum.total minus the chord's before it."""
    chords = score["chords"]
    return {c["index"]: c["cum"]["total"] - (chords[k - 1]["cum"]["total"] if k else 0)
            for k, c in enumerate(chords)}


def _log_table(path):
    """The log at `path` as written: (its header, one {column: text} per row)."""
    with open(_data_file(path), encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        return tuple(reader.fieldnames), list(reader)


def _rewritten(path):
    """(the real log at `path` loaded, the same rows loaded again after
    play_chart.write_press_log wrote them to a file of its own)."""
    _, rows = _log_table(path)
    with tempfile.TemporaryDirectory() as d:
        out = os.path.join(d, "press.csv")
        play_chart.write_press_log(out, rows)
        return score_compare.load_press_log(path), score_compare.load_press_log(out)


class ScoreStepTest(unittest.TestCase):
    def test_a_matching_log_passes(self):
        presses = score_compare.load_press_log(_FULL_RUN_LOG)
        score = _load_score(_WINDOW_MAP_SCORE)
        # The run's every step equals Hydra's points, read from the two files.
        steps, points = _steps(presses), _hydra_points(score)
        self.assertEqual(sorted(steps), sorted(points))
        self.assertEqual(steps, points)
        passed, lines = _real_report(presses, score)
        self.assertTrue(passed)
        self.assertEqual([l for l in lines if l.startswith("FAIL")], [])
        # The final line sets the game's last score beside Hydra's total.
        final = [l for l in lines if l.startswith("Final:")]
        self.assertEqual(len(final), 1)
        self.assertIn(f"game {presses[-1].score_after} after chord index {presses[-1].index}",
                      final[0])
        self.assertIn(str(score["final"]["total"]), final[0])

    def test_a_chord_passes_only_when_the_step_equals_hydras_points(self):
        _, rows = _log_table(_DROPPED_LOG)
        self.assertEqual([(int(r["index"]), r["send"]) for r in rows],
                         [(194, play_chart.SENT), (195, play_chart.SENT),
                          (196, play_chart.DROPPED)])
        presses = score_compare.load_press_log(_DROPPED_LOG)
        score = _load_score(_DROPPED_SCORE)
        steps, points = _steps(presses), _hydra_points(score)
        verdicts = score_compare.compare_scores(
            presses, score_compare.chords_from_score(score), KIT)
        # Each chord's verdict follows from its step and Hydra's points
        # alone: equal where they match, FAIL where they do not.
        self.assertEqual({v.index: v.status for v in verdicts},
                         {i: "equal" if steps[i] == points[i] else "FAIL" for i in steps})
        self.assertEqual({v.index: v.status for v in verdicts},
                         {194: "equal", 195: "equal", 196: "FAIL"})
        passed, lines = _real_report(presses, score)
        self.assertFalse(passed)
        # The one score-step FAIL line names chord 196 and both steps.
        fails = [l for l in lines if l.startswith("FAIL score step")]
        self.assertEqual(len(fails), 1)
        self.assertIn(f"chord index 196: game {steps[196]:+d}, Hydra {points[196]:+d}",
                      fails[0])

    def test_a_short_step_and_a_read_with_no_step_fail(self):
        presses = score_compare.load_press_log(_BEHIND_LOG)
        score = _load_score(_WINDOW_MAP_SCORE)
        steps, points = _steps(presses), _hydra_points(score)
        # In the log: chord 38 steps by Hydra's points, chord 39 by fewer,
        # and the read after chord 40 shows the score unchanged.
        self.assertEqual(steps[38], points[38])
        self.assertLess(steps[39], points[39])
        self.assertEqual(steps[40], 0)
        self.assertGreater(points[40], 0)
        verdicts = {v.index: v for v in score_compare.compare_scores(
            presses, score_compare.chords_from_score(score), KIT)}
        self.assertEqual({i: v.status for i, v in verdicts.items()},
                         {38: "equal", 39: "FAIL", 40: "FAIL"})
        self.assertEqual({i: (v.game, v.hydra) for i, v in verdicts.items()},
                         {i: (steps[i], points[i]) for i in verdicts})

    def test_ghost_accent_forgiveness_applies_only_on_the_keys_route(self):
        presses = score_compare.load_press_log(_GHOST_LOG)
        score = _load_score(_GHOST_SCORE)
        steps, points = _steps(presses), _hydra_points(score)
        chord_137 = next(c for c in score["chords"] if c["index"] == 137)
        self.assertTrue(any(n["dynamic"] in ("ghost", "accent") for n in chord_137["notes"]))
        self.assertNotEqual(steps[137], points[137])
        chords = score_compare.chords_from_score(score)
        keys = {v.index: v.status for v in score_compare.compare_scores(presses, chords, KEYS)}
        kit = {v.index: v.status for v in score_compare.compare_scores(presses, chords, KIT)}
        self.assertEqual(keys[137], "ghost_accent")
        self.assertEqual(kit[137], "FAIL")
        # The kit route forgives nothing, and the keys route forgives only
        # a chord the kit route fails.
        self.assertNotIn("ghost_accent", kit.values())
        self.assertEqual({i: s for i, s in keys.items() if s != "ghost_accent"},
                         {i: kit[i] for i, s in keys.items() if s != "ghost_accent"})
        self.assertEqual({kit[i] for i, s in keys.items() if s == "ghost_accent"}, {"FAIL"})

    def test_a_split_chord_is_one_step(self):
        presses = score_compare.load_press_log(_SPLIT_LOG)
        score = _load_score(_PITCH_DARK_SCORE)
        self.assertEqual([p.index for p in presses], [143, 144, 144, 145])
        verdicts = score_compare.compare_scores(
            presses, score_compare.chords_from_score(score), KIT)
        self.assertEqual([v.index for v in verdicts], [143, 144, 145])
        split = verdicts[1]
        # The step runs from the first press's score before to the second's after.
        self.assertEqual(split.game, presses[2].score_after - presses[1].score_before)
        self.assertEqual(split.hydra, _hydra_points(score)[144])
        self.assertEqual(split.status, "equal")

    def test_a_chord_with_no_row_fails(self):
        # The full run's log without its last chord's row.
        presses = score_compare.load_press_log(_FULL_RUN_LOG)
        score = _load_score(_WINDOW_MAP_SCORE)
        missing = presses[-1].index
        self.assertEqual(missing, score["chords"][-1]["index"])
        passed, lines = _real_report(presses[:-1], score)
        self.assertFalse(passed)
        fails = [l for l in lines if l.startswith("FAIL")]
        self.assertEqual(len(fails), 1)
        self.assertIn(f"1 of {len(score['chords'])} (chord index {missing})", fails[0])

    def test_an_unknown_route_is_refused(self):
        # The full run's log and its score, which the kit route judges
        # without an error; "mouse" is a word neither route uses.
        presses = score_compare.load_press_log(_FULL_RUN_LOG)
        chords = score_compare.chords_from_score(_load_score(_WINDOW_MAP_SCORE))
        score_compare.compare_scores(presses, chords, KIT)
        self.assertNotIn("mouse", (KIT, KEYS))
        with self.assertRaisesRegex(ValueError, "'mouse'"):
            score_compare.compare_scores(presses, chords, "mouse")

    def test_the_route_words_are_game_tools(self):
        from tools.ch_mcp import game_tools
        self.assertEqual((KIT, KEYS), (game_tools.ROUTE_KIT, game_tools.ROUTE_KEYS))

    def test_a_press_not_in_the_score_json_is_an_error(self):
        # Pitch Dark's chords 143 to 145, against the score of chords 136 to
        # 138 of another song, which holds none of them.
        presses = score_compare.load_press_log(_SPLIT_LOG)
        score_compare.compare_scores(
            presses, score_compare.chords_from_score(_load_score(_PITCH_DARK_SCORE)), KIT)
        other = _load_score(_GHOST_SCORE)
        self.assertFalse({p.index for p in presses} & {c["index"] for c in other["chords"]})
        with self.assertRaisesRegex(ValueError, f"chord index {presses[0].index} "):
            score_compare.compare_scores(
                presses, score_compare.chords_from_score(other), KIT)


class RunEndTest(unittest.TestCase):
    def test_a_run_that_stopped_before_the_song_end_fails(self):
        # The full run's log, whose every chord passes, under each stop.
        presses = score_compare.load_press_log(_FULL_RUN_LOG)
        score = _load_score(_WINDOW_MAP_SCORE)
        passed, lines = _real_report(presses, score, stop=play_chart.STOP_DONE)
        self.assertTrue(passed)
        self.assertEqual(lines[0], score_compare.stop_check(play_chart.STOP_DONE)[1])
        for stop in (play_chart.STOP_NO_ENGINE, None):
            with self.subTest(stop=stop):
                passed, lines = _real_report(presses, score, stop=stop)
                self.assertFalse(passed)
                # The run-end verdict is the report's first line and fails it.
                self.assertTrue(lines[0].startswith("FAIL"))
                self.assertEqual(lines[0], score_compare.stop_check(stop)[1])
                if stop is not None:
                    self.assertIn(f"({stop})", lines[0])


class ClockLineTest(unittest.TestCase):
    def test_clock_line_wording(self):
        # The full run's report: its clock line carries the spread of the
        # run's press offsets and the number of presses.
        presses = score_compare.load_press_log(_FULL_RUN_LOG)
        _, lines = _real_report(presses, _load_score(_WINDOW_MAP_SCORE))
        clock = [l for l in lines if l.startswith("Clock at")]
        self.assertEqual(len(clock), 1)
        median, p5, p95 = score_compare.spread(score_compare.press_offsets_ms(presses))
        for part in (f"median {median:+.2f}", f"5th percentile {p5:+.2f}",
                     f"95th percentile {p95:+.2f}", f"({len(presses)} presses)"):
            self.assertIn(part, clock[0])


class PressTimeSpreadTest(unittest.TestCase):
    def test_median_and_percentiles(self):
        # 21 offsets, the last an outlier. spread's percentiles are
        # statistics.quantiles' inclusive cut points; for N values the k-th
        # of 20 cuts falls at sorted position (N - 1) * k / 20, so for 21
        # values the 5th percentile is sorted[1], the 95th sorted[19] and
        # the median sorted[10].
        offsets = [-2.0] * 10 + [-1.5] * 8 + [3.0, 5.0, 40.0]
        ordered = sorted(offsets)
        self.assertEqual(score_compare.spread(offsets), (ordered[10], ordered[1], ordered[19]))
        # The outlier stays out of the 95th percentile.
        self.assertNotEqual(ordered[19], max(offsets))

    def test_offsets_are_clock_minus_chart(self):
        # The full run's log: each offset is the row's clock_ms_at_press
        # minus its chart_ms, read from the file's text.
        _, rows = _log_table(_FULL_RUN_LOG)
        self.assertEqual(
            score_compare.press_offsets_ms(score_compare.load_press_log(_FULL_RUN_LOG)),
            [float(r["clock_ms_at_press"]) - float(r["chart_ms"]) for r in rows])

    def test_one_press(self):
        self.assertEqual(score_compare.spread([0.5]), (0.5, 0.5, 0.5))


class PressLogFileTest(unittest.TestCase):
    def test_reads_the_log_play_chart_writes(self):
        # Pitch Dark's rows, in the current header, written again by
        # play_chart: every row loads as it did from the run's own file.
        self.assertEqual(_log_table(_SPLIT_LOG)[0], overlay_logs.PRESS_LOG_COLUMNS)
        original, rewritten = _rewritten(_SPLIT_LOG)
        self.assertTrue(original)
        self.assertEqual(rewritten, original)

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


# The fields load_press_log turns from text into numbers, and how.
_NUMBER_FIELDS = {"index": int, "chart_ms": float, "clock_ms_at_press": float,
                  "score_before": int, "score_after": int}


class PressLogHeaderTest(unittest.TestCase):
    """Real press logs of each header load_press_log accepts, and headers it
    refuses."""

    def assert_loads_as_written(self, path, header):
        """load_press_log reads the real log at `path`, whose header is
        `header`: each Press field holds its column's text as written (the
        number fields as numbers), and a column the log lacks is blank.
        Returns the Press rows."""
        with open(path, encoding="utf-8", newline="") as f:
            table = list(csv.reader(f))
        self.assertEqual(tuple(table[0]), header)
        self.assertTrue(table[1:], f"{path} holds no rows")
        presses = score_compare.load_press_log(path)
        self.assertEqual(len(presses), len(table) - 1)
        for press, cells in zip(presses, table[1:]):
            written = dict(zip(table[0], cells))
            for field in dataclasses.fields(score_compare.Press):
                text = written.get(field.name, "")
                want = _NUMBER_FIELDS[field.name](text) if field.name in _NUMBER_FIELDS else text
                self.assertEqual(getattr(press, field.name), want, (path, field.name))
        return presses

    def test_a_six_column_log_loads_with_blank_kit_fields(self):
        # hydra-data/ch-mcp-runs/2026-10-28/l3/keys/presses.csv, its first
        # three rows: a keys-route run.
        presses = self.assert_loads_as_written(
            os.path.join(_RUNS_DIR, "2026-10-28-l3", "keys-presses-rows-1-3.csv"),
            overlay_logs.KEYS_PRESS_COLUMNS)
        self.assertEqual({p.down_frame for p in presses}, {""})

    def test_a_twelve_column_log_loads_with_the_wall_times(self):
        presses = self.assert_loads_as_written(
            _FULL_RUN_LOG, overlay_logs.PRESS_COLUMNS_WITHOUT_PLAYER)
        self.assertNotIn("", {p.wall_s_at_read for p in presses})

    def test_a_fifteen_column_log_from_before_the_retime_column_loads(self):
        # hydra-data/ch-mcp-runs/2026-11-07/ls-f1-run1/press-2.csv, its first
        # three rows.
        self.assert_loads_as_written(
            os.path.join(_RUNS_DIR, "ls-f1-run1", "press-2-rows-1-3.csv"),
            overlay_logs.PRESS_COLUMNS_WITHOUT_RETIME)

    def test_the_retime_word_play_chart_writes_reads_back(self):
        # LS-A2's rows, one of them retimed late, written again by play_chart.
        original, rewritten = _rewritten(_RETIMED_LOG)
        self.assertEqual([p.retimed for p in rewritten], [p.retimed for p in original])
        self.assertIn(play_chart.RETIMED_LATE, [p.retimed for p in rewritten])

    def test_press_has_a_field_for_every_log_column_but_the_send_column(self):
        self.assertEqual(
            tuple(f.name for f in dataclasses.fields(score_compare.Press)),
            tuple(c for c in overlay_logs.PRESS_LOG_COLUMNS
                  if c != overlay_logs.SEND_COLUMN))

    def test_a_log_with_the_header_before_timing_still_loads(self):
        # hydra-data/ch-mcp-runs/2026-11-11/ls-a2-run1/press-1.csv, its first
        # three rows and row 1906, the one row the run retimed.
        presses = self.assert_loads_as_written(
            _RETIMED_LOG, overlay_logs.PRESS_COLUMNS_BEFORE_TIMING)
        self.assertEqual([p.retimed for p in presses], ["", "", "", play_chart.RETIMED_LATE])

    def test_the_timing_columns_play_chart_writes_read_back(self):
        # LS-P3's rows, whose timing columns the run filled in, written
        # again by play_chart: each timing field keeps the run's text.
        original, rewritten = _rewritten(_TIMING_LOG)
        self.assertNotIn("", {p.press_took_ms for p in original})
        for column in overlay_logs.TIMING_PRESS_COLUMNS:
            with self.subTest(column=column):
                self.assertEqual([getattr(p, column) for p in rewritten],
                                 [getattr(p, column) for p in original])

    def test_a_log_with_the_header_before_the_in_game_columns_still_loads(self):
        # hydra-data/ch-mcp-runs/2026-11-12/ls-p3-run1/press-1.csv, its first
        # three rows.
        presses = self.assert_loads_as_written(
            _TIMING_LOG, overlay_logs.PRESS_COLUMNS_BEFORE_IN_GAME)
        self.assertNotIn("", {p.press_took_ms for p in presses})
        self.assertEqual({(p.frame_gap_ms, p.agent_note) for p in presses}, {("", "")})

    def test_the_in_game_columns_play_in_game_writes_read_back(self):
        # Pitch Dark's rows, whose frame gaps the in-game run filled in,
        # written again by play_chart: each in-game field keeps the run's text.
        original, rewritten = _rewritten(_SPLIT_LOG)
        self.assertNotIn("", {p.frame_gap_ms for p in original})
        for column in overlay_logs.IN_GAME_PRESS_COLUMNS:
            with self.subTest(column=column):
                self.assertEqual([getattr(p, column) for p in rewritten],
                                 [getattr(p, column) for p in original])

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
                with self.assertRaises(ValueError) as caught:
                    _load_text(header + "\n")
                # The message names the header the log has and the current
                # header it should have.
                message = str(caught.exception)
                self.assertIn(str(header.split(",")), message)
                self.assertIn(str(list(overlay_logs.PRESS_LOG_COLUMNS)), message)


if __name__ == "__main__":
    unittest.main()
