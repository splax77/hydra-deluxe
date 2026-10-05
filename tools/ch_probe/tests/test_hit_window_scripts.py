"""Tests for the pure parts of watch_window.py, walk_edges.py and live.py.

No game needed: synthetic samples, fake clocks, and the real probe-song layout
from probe_songs.py.
"""

from __future__ import annotations

import os
import struct
import sys
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import probe_songs as P  # noqa: E402
from tools.ch_probe.experiments import live  # noqa: E402
from tools.ch_probe.experiments import walk_edges as W  # noqa: E402
from tools.ch_probe.experiments import watch_window as WW  # noqa: E402


def window_map_notes() -> list[dict]:
    return P.manifest("x", P.window_map().notes)["notes"]


def samples_following(notes: list[dict], window_for) -> list[WW.Sample]:
    """A sample at every note time, window set by window_for(note)."""
    out = [WW.Sample(0.0, 0.0, 0, 0.0)]
    for n in notes:
        out.append(WW.Sample(float(n["time_ms"]), window_for(n), 0, 0.0))
    return out


class LiveSnapshotTest(unittest.TestCase):
    def test_decode_reads_each_field_at_its_offset(self):
        raw = bytearray(live.SNAPSHOT_SIZE)
        struct.pack_into("<d", raw, C.OFF_TOTAL_WINDOW, C.ms_to_s(C.WINDOW_CAP_MS))
        struct.pack_into("<I", raw, C.OFF_SCORE, 1234)
        struct.pack_into("<d", raw, C.OFF_SONG_CLOCK, 12.5)
        struct.pack_into("<I", raw, C.OFF_FLAGS, C.PRECISION_MODE_BIT)
        struct.pack_into("<d", raw, C.OFF_HIT_TIME, 12.49)
        s = live.decode_snapshot(bytes(raw))
        self.assertAlmostEqual(s.window_ms, C.WINDOW_CAP_MS)
        self.assertEqual(s.score, 1234)
        self.assertEqual(s.clock_s, 12.5)
        self.assertEqual(s.hit_time_s, 12.49)
        self.assertTrue(s.precision)

    def test_live_keeps_no_offsets_or_finder_of_its_own(self):
        for name in ("OFF_WINDOW", "OFF_SCORE", "OFF_CLOCK", "OFF_FLAGS",
                     "OFF_HIT_TIME", "find_live_engine"):
            self.assertFalse(hasattr(live, name), name)


class LiveHelpersTest(unittest.TestCase):
    """The one wait loop, start cursor, hit offset and window tolerance."""

    def test_wait_until_fires_at_the_target_or_its_lead(self):
        for lead_ms, fires_at_ms in ((0.0, 1100.0), (2.0, 1098.0)):
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

            clock = W.SongClock(read_raw, now=lambda: t[0])
            raw_ms, est_ms = live.wait_until(clock, 1100.0, lead_ms=lead_ms,
                                             now=lambda: t[0], sleep=sleep)
            self.assertAlmostEqual(est_ms, fires_at_ms)
            self.assertAlmostEqual(raw_ms, fires_at_ms)

    def test_wait_until_stops_after_five_seconds_frozen(self):
        self.assertEqual(live.STALL_S, 5.0)

        def run(step_s, raws):
            t = [0.0]
            script = list(raws)
            clock = W.SongClock(lambda: script.pop(0) if len(script) > 1 else script[0],
                                now=lambda: t[0])
            return live.wait_until(clock, 10000.0, now=lambda: t[0],
                                   sleep=lambda s: t.__setitem__(0, t[0] + step_s))

        with self.assertRaises(live.ClockFrozen):
            run(5.1, [1.0])                      # frozen past 5 s
        raw_ms, _ = run(4.9, [1.0, 1.0, 10.0])   # frozen 4.9 s, then it moves
        self.assertEqual(raw_ms, 10000.0)

    def test_wait_until_stops_differently_on_a_jump_back(self):
        t = [0.0]
        script = [3 * live.JUMP_BACK_S, 0.0]     # falls by more than the limit
        clock = W.SongClock(lambda: script.pop(0) if len(script) > 1 else script[0],
                            now=lambda: t[0])
        with self.assertRaises(live.ClockJumpedBack) as caught:
            live.wait_until(clock, 10000.0, now=lambda: t[0],
                            sleep=lambda s: t.__setitem__(0, t[0] + s))
        self.assertNotIsInstance(caught.exception, live.ClockFrozen)

    def test_first_note_index_skips_notes_less_than_150ms_ahead(self):
        self.assertEqual(live.START_LEAD_MS, 150.0)
        self.assertEqual(live.first_note_index([100.0, 149.0, 150.0, 200.0], 0.0), 2)

    def test_hit_offset_prefers_the_engine_hit_time(self):
        offset, from_engine = live.hit_offset_ms(10000.0, 10004.0, 10.000, 10.010)
        self.assertAlmostEqual(offset, 10.0)
        self.assertTrue(from_engine)
        offset, from_engine = live.hit_offset_ms(10000.0, 10004.0, 10.000, 10.000)
        self.assertEqual(offset, 4.0)
        self.assertFalse(from_engine)

    def test_window_changed_uses_the_one_tolerance(self):
        self.assertEqual(C.WINDOW_CHANGE_TOLERANCE_MS, 1e-6)
        self.assertTrue(live.window_changed(85.0, 85.0005))
        self.assertFalse(live.window_changed(85.0, 85.0))

    def test_old_diagnostics_are_gone(self):
        from tools.ch_probe import input_driver
        from tools.ch_probe.experiments import active_probe
        experiments = os.path.join(_REPO_ROOT, "tools", "ch_probe", "experiments")
        for script in ("hit_detect.py", "find_clock3.py", "poll_windows.py",
                       "find_engine.py", "milestone2.py", "test_attach.py"):
            self.assertFalse(os.path.exists(os.path.join(experiments, script)), script)
        for module in (W, active_probe, WW, input_driver, input_driver.InputDriver):
            for name in ("wait_until", "schedule_hit", "STALL_S"):
                self.assertFalse(hasattr(module, name), f"{module.__name__}.{name}")


class WatchWindowTest(unittest.TestCase):
    def test_value_at_takes_the_last_change_at_or_before(self):
        chg = [(0.0, 1.0), (100.0, 2.0), (200.0, 3.0)]
        self.assertIsNone(WW.value_at(chg, -1.0))
        self.assertEqual(WW.value_at(chg, 100.0), 2.0)
        self.assertEqual(WW.value_at(chg, 199.9), 2.0)

    def test_block_spans_split_in_the_silence_and_cover_every_note(self):
        notes = window_map_notes()
        spans = WW.block_spans(notes)
        self.assertEqual(len(spans), len({n["block"] for n in notes}))
        for n in notes:
            owner = [b for b, s, e in spans if s <= n["time_ms"] < e]
            self.assertEqual(owner, [n["block"]])

    def test_run_values_come_from_the_run_not_the_markers(self):
        notes = window_map_notes()
        # Window follows the gap after each note, markers read 100.
        samples = samples_following(
            notes, lambda n: float(n["gap_after_ms"] or 999))
        lines = WW.window_report(samples, notes)
        cap = [l for l in lines if l.strip().startswith("400 ms gap")]
        self.assertEqual(cap, [f"   400 ms gap: 400.00  DIFFERENT from {C.WINDOW_CAP_MS}"])
        floor = [l for l in lines if l.strip().startswith("30 ms gap")]
        self.assertEqual(floor, [f"    30 ms gap: 30.00  DIFFERENT from {C.WINDOW_FLOOR_MS}"])
        self.assertIn("Marker notes (100 ms gaps): 100.00", lines)

    def test_cap_verdict_passes_when_every_wide_run_reads_the_cap(self):
        notes = window_map_notes()
        samples = samples_following(
            # The gap edges typed as literals (170 and 75 ms), not the script's
            # own comparisons, so a wrong edge in watch_window fails here.
            notes, lambda n: C.WINDOW_CAP_MS if (n["gap_after_ms"] or 0) >= 170
            else (C.WINDOW_FLOOR_MS if (n["gap_after_ms"] or 999) <= 75
                  else 100.0))
        lines = WW.window_report(samples, notes)
        self.assertEqual(sum("matches the cap" in l for l in lines), 5)
        self.assertFalse(any("DIFFERENT" in l for l in lines))

    def test_frozen_window_says_so(self):
        notes = window_map_notes()
        samples = samples_following(notes, lambda n: 170.0)[1:]
        self.assertIn("The window never changed", WW.window_report(samples, notes)[0])

    def test_hit_time_report_matches_changes_to_notes(self):
        notes = [{"index": i, "time_ms": 3000 + 1000 * i} for i in range(3)]
        samples = [WW.Sample(2900.0, C.WINDOW_CAP_MS, 0, 0.0)]
        for i, n in enumerate(notes):
            samples.append(WW.Sample(n["time_ms"] + 8.0, C.WINDOW_CAP_MS, 100 * (i + 1),
                                     n["time_ms"] + 2.0))
        lines = WW.hit_time_report(samples, notes)
        self.assertIn("  +0x2e0 changed 3 times; the score rose 3 times.", lines)
        self.assertTrue(any("Looks like the hit time" in l for l in lines))


class WalkEdgesTest(unittest.TestCase):
    def test_default_plan_fits_edge_walk(self):
        plan = W.build_schedule((80, 92), (80, 92), 3, P.EDGE_WALK_NOTES)
        self.assertEqual(len(plan), 120)
        self.assertEqual(plan[:3], [0, 0, 0])
        self.assertEqual(plan[3:6], [80, 80, 80])
        self.assertEqual(plan[39:42], [92, 92, 92])
        self.assertEqual(plan[42:45], [-80, -80, -80])
        self.assertEqual(plan[78:81], [-92, -92, -92])
        self.assertEqual(set(plan[81:]), {0})

    def test_plan_that_does_not_fit_is_refused(self):
        with self.assertRaises(ValueError):
            W.build_schedule((0, 40), (0, 40), 3, 120)

    def test_parse_range(self):
        self.assertEqual(W.parse_range("68:80"), (68, 80))
        self.assertIsNone(W.parse_range("none"))
        with self.assertRaises(ValueError):
            W.parse_range("92:80")

    def test_song_clock_fills_in_after_a_fresh_change(self):
        t = [0.0]
        raw = [1.000]
        clock = W.SongClock(lambda: raw[0], now=lambda: t[0])
        self.assertEqual(clock.read(), (1.000, 1.000))   # first read: no anchor
        t[0] = 0.001; raw[0] = 1.016                      # change seen 1 ms later
        self.assertEqual(clock.read(), (1.016, 1.016))
        t[0] = 0.006
        r, est = clock.read()
        self.assertAlmostEqual(est, 1.021)
        t[0] = 1.0                                        # capped fill-in
        self.assertAlmostEqual(clock.read()[1], 1.016 + C.CLOCK_MAX_FILL_S)

    def test_song_clock_ignores_a_change_seen_late(self):
        t = [0.0]
        raw = [1.0]
        clock = W.SongClock(lambda: raw[0], now=lambda: t[0])
        clock.read()
        t[0] = 0.5; raw[0] = 1.5                          # slept through it
        clock.read()
        t[0] = 0.51
        self.assertEqual(clock.read(), (1.5, 1.5))

    def test_summary_brackets_the_edge(self):
        def row(p, m, hit):
            return W.Row(0, 0.0, p, m, m, m if hit else None, hit, m)
        rows = [row(0, 0.5, True), row(85, 85.2, True), row(85, 85.4, True),
                row(86, 86.1, False), row(-85, -85.3, True), row(-86, -86.2, False)]
        lines = W.summarize(rows)
        self.assertIn("On-time notes: 1/1 hit.", lines)
        # The edge analysis.find_window_edge gives for these rows, pinned.
        self.assertIn("  The late edge is 85.75 ms; 0 of 3 notes fall on the wrong"
                      " side of it.", lines)
        self.assertIn("  The early edge is 85.75 ms; 0 of 2 notes fall on the wrong"
                      " side of it.", lines)

    def test_summary_flags_overlap_and_all_miss(self):
        def row(p, m, hit):
            return W.Row(0, 0.0, p, m, m, None, hit, m)
        lines = W.summarize([row(85, 85.0, False), row(86, 86.0, True),
                             row(-80, -80.0, False)])
        self.assertTrue(any("overlap" in l for l in lines))
        self.assertTrue(any("early edge is below 80 ms" in l for l in lines))


if __name__ == "__main__":
    unittest.main()
