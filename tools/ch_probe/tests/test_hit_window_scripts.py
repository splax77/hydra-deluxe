"""Tests for the pure parts of watch_window.py and live.py.

No game needed: real watcher and press logs copied from earlier Clone Hero
runs under testdata/ch_runs, read back on the real timer where a clock is
needed, the real probe-song layout from probe_songs.py, and synthetic samples
where no real run recorded the field.
"""

from __future__ import annotations

import os
import sys
import time
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import probe_songs as P  # noqa: E402
from tools.ch_probe.experiments import live  # noqa: E402
from tools.ch_probe.experiments import watch_window as WW  # noqa: E402
from tools.ch_probe.experiments.score_compare import load_press_log  # noqa: E402
from tools.ch_probe.experiments.watch_checks import load_watch_rows  # noqa: E402
from tools.ch_probe.tests import test_play_chart as _play_chart_tests  # noqa: E402

_CH_RUNS = os.path.join(_REPO_ROOT, "testdata", "ch_runs")

# The end of a real watch: the clock's last two moves, then the final read,
# when sp_watch's own live.StoppedCheck found the clock held too long. Copied
# unchanged from lines 1 and 50524-50526 of
# hydra-data\ch-mcp-runs\2026-11-07\ls-r1-run2\sp_watch-1.csv.
FROZEN_TAIL_CSV = os.path.join(_CH_RUNS, "ls-r1-run2", "sp_watch-1-tail.csv")

# 2.2 s of a real watch near the end of The Who's "Won't Get Fooled Again":
# three hits that moved +0x2E0, a 1.6 s gap, then three hits that did not
# (page 07, IN-12: the field is the start of a lane round, not a hit time).
# Copied unchanged from lines 1 and 113384-113874 of
# hydra-data\ch-mcp-runs\2026-11-08\ls-m2-run1\sp_watch-2.csv.
LANE_END_CSV = os.path.join(_CH_RUNS, "ls-m2-run1", "sp_watch-2-lanes-509.55-511.75.csv")

# The same run's press log rows for those six notes, indexes 2131 to 2136.
# Copied unchanged from lines 1 and 2146-2151 of
# hydra-data\ch-mcp-runs\2026-11-08\ls-m2-run1\press-2.csv.
LANE_END_PRESSES_CSV = os.path.join(_CH_RUNS, "ls-m2-run1", "press-2-rows-2131-2136.csv")


def recorded_samples(path: str) -> list[WW.Sample]:
    """A real watcher log as watch_window samples. sp_watch logs no hit
    window, so window_ms is NaN: no test here reads it from these samples."""
    return [WW.Sample(C.s_to_ms(r["clock_s"]), float("nan"), r["score"],
                      C.s_to_ms(r["round_start_s"]))
            for r in load_watch_rows(path)]


def recorded_notes(path: str) -> list[dict]:
    """A real press log's notes, in the shape round_start_report reads."""
    return [{"index": p.index, "time_ms": p.chart_ms} for p in load_press_log(path)]


def recorded_song_clock(path: str):
    """A live.SongClock over the song clock a real watcher logged, read back
    at the recording's own pace on the real timer (test_play_chart's
    _RecordedGame). A read past the recording's end fails the test at once."""
    game = _play_chart_tests._RecordedGame(path)
    return live.SongClock(lambda: game.read()[0]), game


def window_map_notes() -> list[dict]:
    return P.manifest("x", P.window_map().notes)["notes"]


def samples_following(notes: list[dict], window_for) -> list[WW.Sample]:
    """A sample at every note time, window set by window_for(note)."""
    out = [WW.Sample(0.0, 0.0, 0, 0.0)]
    for n in notes:
        out.append(WW.Sample(float(n["time_ms"]), window_for(n), 0, 0.0))
    return out


class LiveHelpersTest(unittest.TestCase):
    """The one wait loop, start cursor, hit offset and window tolerance."""

    def test_wait_until_fires_at_the_target_or_its_lead(self):
        target_ms = 1100.0
        for lead_ms in (0.0, 2.0):
            fires_at_ms = target_ms - lead_ms
            # The song clock moves 1 ms per read and the read is always fresh,
            # so the estimate equals the raw clock. A sleep moves both on.
            ms = [999]
            t = [0.0]

            def read_raw():
                ms[0] += 1
                t[0] += 0.001
                return C.ms_to_s(ms[0])

            def sleep(s):
                ms[0] += round(C.s_to_ms(s))
                t[0] += s

            clock = live.SongClock(read_raw, now=lambda: t[0])
            raw_ms, est_ms = live.wait_until(clock, target_ms, lead_ms=lead_ms,
                                             now=lambda: t[0], sleep=sleep)
            self.assertAlmostEqual(est_ms, fires_at_ms)
            self.assertAlmostEqual(raw_ms, fires_at_ms)

    def test_wait_until_stops_on_a_real_song_end_frozen_past_the_stall_limit(self):
        # The real watch held its clock for longer than live.STALL_S at the
        # end of the song, so a wait for a moment the clock never reached
        # must stop with ClockFrozen before the recording runs out. The wait
        # reads the recorded clock raw: SongClock's fill-in would carry the
        # estimate past a target this close. The target sits inside
        # wait_until's spin range, so it reads without sleeping and sees the
        # last move as soon as the recording makes it.
        rows = load_watch_rows(FROZEN_TAIL_CSV)
        last_move = max(i for i in range(1, len(rows))
                        if rows[i]["clock_s"] != rows[i - 1]["clock_s"])
        held_s = rows[-1]["wall_s"] - rows[last_move]["wall_s"]
        self.assertGreater(held_s, live.STALL_S)
        game = _play_chart_tests._RecordedGame(FROZEN_TAIL_CSV)
        never_reached_ms = C.s_to_ms(game.last_clock_s) + 1.0
        with self.assertRaises(live.ClockFrozen):
            live.wait_until(game, never_reached_ms)

    def test_wait_until_does_not_stop_while_a_real_clock_moves(self):
        # The real lane-end watch moves its clock every frame through a 1.6 s
        # gap with no hits, so a wait for the next note after the gap ends at
        # that note, not with ClockFrozen.
        notes = recorded_notes(LANE_END_PRESSES_CSV)
        gaps = [b["time_ms"] - a["time_ms"] for a, b in zip(notes, notes[1:])]
        after_gap = notes[gaps.index(max(gaps)) + 1]["time_ms"]
        clock, game = recorded_song_clock(LANE_END_CSV)
        raw_ms, est_ms = live.wait_until(clock, after_gap)
        self.assertGreaterEqual(est_ms, after_gap)
        self.assertLessEqual(raw_ms, C.s_to_ms(game.last_clock_s))

    def test_first_note_index_skips_notes_less_than_the_lead_ahead(self):
        # Real note times from the lane-end press log. A clock exactly the
        # lead before a note starts at that note; a clock a hair later has
        # passed it.
        notes_ms = [n["time_ms"] for n in recorded_notes(LANE_END_PRESSES_CSV)]
        self.assertEqual(notes_ms, sorted(set(notes_ms)))
        for k, note_ms in enumerate(notes_ms):
            self.assertEqual(live.first_note_index(notes_ms, note_ms - live.START_LEAD_MS), k)
            self.assertEqual(
                live.first_note_index(notes_ms, note_ms - live.START_LEAD_MS + 0.001), k + 1)

    def test_window_changed_uses_the_one_tolerance(self):
        # Just inside and just past the tolerance, both ways round.
        tol = C.WINDOW_CHANGE_TOLERANCE_MS
        self.assertFalse(live.window_changed(85.0, 85.0))
        self.assertFalse(live.window_changed(85.0, 85.0 + tol / 2))
        self.assertFalse(live.window_changed(85.0 + tol / 2, 85.0))
        self.assertTrue(live.window_changed(85.0, 85.0 + 2 * tol))
        self.assertTrue(live.window_changed(85.0 + 2 * tol, 85.0))


class WatchWindowTest(unittest.TestCase):
    def test_value_at_takes_the_last_change_at_or_before(self):
        # The score changes of a real watch: at every logged row's clock, and
        # halfway to the next row, value_at gives that row's score. A watcher
        # row is written only when a field changes, so a score holds until
        # the next row. Before the first row there is no value.
        samples = recorded_samples(LANE_END_CSV)
        chg = WW.changes(samples, "score")
        self.assertGreater(len(chg), 1)
        self.assertIsNone(WW.value_at(chg, samples[0].clock_ms - 1.0))
        for s, nxt in zip(samples, samples[1:]):
            if nxt.clock_ms == s.clock_ms:
                continue                          # the later row at this clock wins
            self.assertEqual(WW.value_at(chg, s.clock_ms), s.score)
            self.assertEqual(WW.value_at(chg, (s.clock_ms + nxt.clock_ms) / 2), s.score)

    def test_block_spans_split_in_the_silence_and_cover_every_note(self):
        notes = window_map_notes()
        spans = WW.block_spans(notes)
        self.assertEqual(len(spans), len({n["block"] for n in notes}))
        for n in notes:
            owner = [b for b, s, e in spans if s <= n["time_ms"] < e]
            self.assertEqual(owner, [n["block"]])

    def test_cap_verdict_passes_when_every_wide_run_reads_the_cap(self):
        # The gap edges are typed as literals, not the script's own
        # comparisons, so a wrong edge in watch_window fails here. Each
        # half-gap is held between 37.5 and 85 ms, so a note reads the cap
        # from a 170 ms gap and the floor up to a 75 ms gap
        # (docs/clone-hero/03-timing.md, TM-19 and TM-20).
        cap_from_gap_ms, floor_up_to_gap_ms = 170, 75
        notes = window_map_notes()
        samples = samples_following(
            notes, lambda n: C.WINDOW_CAP_MS if (n["gap_after_ms"] or 0) >= cap_from_gap_ms
            else (C.WINDOW_FLOOR_MS if (n["gap_after_ms"] or 999) <= floor_up_to_gap_ms
                  else 100.0))
        lines = WW.window_report(samples, notes)
        # One verdict per cap block of the real layout whose gap reaches the cap.
        wide_cap_blocks = {n["block"] for n in notes if n["block"].startswith("cap_")
                           and int(n["block"].split("_")[1]) >= cap_from_gap_ms}
        self.assertTrue(wide_cap_blocks)
        self.assertEqual(sum("matches the cap" in l for l in lines), len(wide_cap_blocks))
        self.assertFalse(any("DIFFERENT" in l for l in lines))

    def test_round_start_report_ties_each_real_change_to_its_note(self):
        # A real watch and press log of the same run. Each new +0x2E0 value
        # gets one row naming the press nearest it, and the count line holds
        # the changes the watch logged beside the scoring presses the press
        # log, a separate log, recorded.
        samples = recorded_samples(LANE_END_CSV)
        notes = recorded_notes(LANE_END_PRESSES_CSV)
        presses = load_press_log(LANE_END_PRESSES_CSV)
        new_values = []
        for a, b in zip(samples, samples[1:]):
            if b.round_start_ms != a.round_start_ms:
                new_values.append(b.round_start_ms)
        self.assertTrue(new_values)
        scored = sum(1 for p in presses if p.score_after > p.score_before)
        lines = WW.round_start_report(samples, notes)
        rows = [l for l in lines if l.startswith("  clock ")]
        self.assertEqual(len(rows), len(new_values))
        for line, v in zip(rows, new_values):
            nearest = min(notes, key=lambda n: abs(n["time_ms"] - v))
            self.assertIn(f"note #{nearest['index']} ", line)
        self.assertIn(f"  +0x2e0 changed {len(new_values)} times; "
                      f"the score rose {scored} times.", lines)

    def test_round_start_report_says_when_the_changes_do_not_fit(self):
        # In the real lane-end run the last three scoring presses left +0x2E0
        # alone (IN-12: it moves only when a hit completes a lane round), so
        # the field did not change once per hit and the report says so.
        samples = recorded_samples(LANE_END_CSV)
        presses = load_press_log(LANE_END_PRESSES_CSV)
        changed = sum(1 for a, b in zip(samples, samples[1:])
                      if b.round_start_ms != a.round_start_ms)
        scored = sum(1 for p in presses if p.score_after > p.score_before)
        self.assertNotEqual(changed, scored)
        last = WW.round_start_report(samples, recorded_notes(LANE_END_PRESSES_CSV))[-1]
        self.assertIn("+0x2e0 did NOT change once per hit", last)
        self.assertIn(f"within {C.ROUND_START_MAX_GAP_MS:g} ms of the note", last)


class SongClockTest(unittest.TestCase):
    def test_song_clock_fills_in_after_a_fresh_change(self):
        t = [0.0]
        raw = [1.000]
        clock = live.SongClock(lambda: raw[0], now=lambda: t[0])
        self.assertEqual(clock.read(), (1.000, 1.000))   # first read: no anchor
        changed_s, seen_t = 1.016, 0.001                  # change seen 1 ms later
        t[0] = seen_t; raw[0] = changed_s
        self.assertEqual(clock.read(), (changed_s, changed_s))
        later_t = 0.006
        t[0] = later_t
        r, est = clock.read()
        self.assertAlmostEqual(est, changed_s + (later_t - seen_t))
        t[0] = 1.0                                        # capped fill-in
        self.assertAlmostEqual(clock.read()[1], changed_s + C.CLOCK_MAX_FILL_S)

    def test_song_clock_ignores_a_change_seen_late(self):
        # A real watch's song clock, read back at the recording's own pace on
        # the real timer. After one read, the next change is left unread for
        # longer than CLOCK_FRESH_S, so its age is unknown: every read until
        # the clock moves again gives the raw clock with nothing filled in.
        # A read past the recording's end fails the test at once.
        clock, game = recorded_song_clock(LANE_END_CSV)
        first_raw, _ = clock.read()
        read_t = time.perf_counter()
        while game.read()[0] == first_raw or time.perf_counter() - read_t <= C.CLOCK_FRESH_S:
            pass
        late_raw, late_est = clock.read()
        self.assertEqual(late_est, late_raw)
        held = []
        while True:
            raw, est = clock.read()
            if raw != late_raw:
                break
            held.append(est)
        self.assertTrue(held)
        self.assertEqual(held, [late_raw] * len(held))


if __name__ == "__main__":
    unittest.main()
