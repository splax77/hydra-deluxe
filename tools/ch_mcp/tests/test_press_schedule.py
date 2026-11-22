"""Tests for agent/press_schedule.ts, the frame scheduler. The in-game press
timing plan's task IG1 gave its first tests; the plan is
docs/superpowers/plans/2026-11-12-in-game-press-timing.md (removed; git show b971b235:docs/superpowers/plans/2026-11-12-in-game-press-timing.md).
The frame pick (the measured step, the two-frame judge time, bands, windows
and the no-frame fallback) has one test per acceptance line of task T2 of the
squeeze timing fix; the scheduler's header is the contract they test.

No game and no Frida. Plain Node runs press_schedule.ts (Node erases its type
annotations), as test_launch_core.py runs launch_core.ts: each case is a
list of steps, retimes and stops sent to a small driver on stdin, and the
driver prints what the scheduler returned for each. The score it reads is
whatever the case's latest step says. The tests skip, naming the reason, when
`node` is not on PATH.

Every clock in these cases is a whole number of steps of 3.0, 0.75, 6.0 or
another value a double holds exactly, so a prediction equal to an edge is
equal, not off by a rounding.
"""

from __future__ import annotations

import csv
import json
import os
import pathlib
import shutil
import subprocess
import sys
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.constants import s_to_ms  # noqa: E402

SCHEDULER = os.path.join(_REPO_ROOT, "tools", "ch_mcp", "agent", "press_schedule.ts")
NODE = shutil.which("node")
# One driver run is a fresh Node start; this is its stop.
NODE_CAP_S = 30

# The driver: reads {"play": ..., "ops": [...]} from stdin, runs each op on
# one PressSchedule and prints a list with one result per op.
DRIVER = """
import { readFileSync } from "node:fs";
const { PressSchedule } = await import(%(url)s);
const c = JSON.parse(readFileSync(0, "utf8"));
let score = 0;
const out = [];
try {
    const sched = new PressSchedule(c.play, () => score);
    for (const op of c.ops) {
        if (op.op === "step") {
            if (op.score !== undefined) { score = op.score; }
            out.push(sched.step(op.frame, op.clock_ms, op.wall_ms));
        } else if (op.op === "retime") {
            out.push(sched.retime(op.read_after, op.entries));
        } else if (op.op === "stop") {
            out.push(sched.stop(op.wall_ms));
        }
    }
} catch (e) {
    out.push({ error: e.message });
}
process.stdout.write(JSON.stringify(out));
"""

# The pad ids the cases press: any distinct whole numbers will do.
SNARE, TOM, KICK, CYMBAL = 1, 2, 3, 4


def band(from_ms, to_ms, from_strict=False):
    return {"from_ms": from_ms, "to_ms": to_ms, "from_strict": from_strict}


def entry(seq, at_ms, pad=SNARE, *, index=None, band=None, window=None,
          no_frame_aim_ms=None, after_seq=None, end_read_after=None):
    """One schedule entry. Its chord index is its seq unless given. With no
    `window` (a (front, back) pair) it has no window, as a probe entry."""
    e = {
        "seq": seq,
        "index": seq if index is None else index,
        "at_ms": at_ms,
        "elements": [{"id": pad, "value": True}],
        "after_seq": after_seq,
        "end_read_after": end_read_after,
    }
    if band is not None:
        e["band"] = band
    if window is not None:
        e["window_from_ms"], e["window_to_ms"] = window
    if no_frame_aim_ms is not None:
        e["no_frame_aim_ms"] = no_frame_aim_ms
    return e


def play(entries, **numbers):
    """A play message. Its numbers are the cases' own, picked apart from one
    another so a mix-up shows."""
    msg = {"entries": entries, "hold_frames": 2, "start_lead_ms": 150.0,
           "score_delay_ms": 5.0, "cap_ms": 600000.0, "stall_ms": 5000.0,
           "jump_back_ms": 1000.0}
    msg.update(numbers)
    return msg


def step(frame, clock_ms, wall_ms, score=None):
    op = {"op": "step", "frame": frame, "clock_ms": clock_ms, "wall_ms": wall_ms}
    if score is not None:
        op["score"] = score
    return op


def ticks(first_clock, step_ms, count, *, first_frame=10, wall0=5000.0, wall_step=3.0):
    """`count` frames, each `step_ms` of clock after the one before."""
    return [step(first_frame + k, first_clock + k * step_ms, wall0 + k * wall_step)
            for k in range(count)]


def run_raw(play_msg, ops):
    driver = DRIVER % {"url": json.dumps(pathlib.Path(SCHEDULER).as_uri())}
    done = subprocess.run(
        [NODE, "--input-type=module", "-e", driver],
        input=json.dumps({"play": play_msg, "ops": ops}), capture_output=True,
        text=True, timeout=NODE_CAP_S, check=False)
    if done.returncode != 0:
        raise AssertionError(f"node exited {done.returncode}: {done.stderr}")
    return json.loads(done.stdout)


def run(play_msg, ops, *, stop_after=False):
    """Each op's result. With `stop_after`, a stop follows the last op, so
    every press written is reported; its result is the last one."""
    if stop_after:
        last_wall = max((op["wall_ms"] for op in ops if "wall_ms" in op), default=0.0)
        ops = ops + [{"op": "stop", "wall_ms": last_wall + 1.0}]
    out = run_raw(play_msg, ops)
    if out and "error" in out[-1]:
        raise AssertionError(f"the scheduler threw: {out[-1]['error']}")
    return out


# Seven readings of the Star Power watch, copied unchanged from lines
# 19125-19131 of hydra-data/ch-mcp-runs/2026-11-13/pitch-dark-ratios-run1/
# sp_watch-1.csv. They span press 144 of that run's press-1.csv, where the
# score went 45965, 46485, 46885.
SCORE_WINDOW_CSV = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "pitch-dark-ratios-run1",
                                "sp_watch-1-chord144.csv")


def score_window():
    """The window's wall times and song clocks in ms, and its scores."""
    with open(SCORE_WINDOW_CSV, newline="", encoding="utf-8") as f:
        rows = list(csv.DictReader(f))
    return ([s_to_ms(float(r["wall_s"])) for r in rows],
            [s_to_ms(float(r["clock_s"])) for r in rows],
            [int(r["score"]) for r in rows])


def written(result):
    return [w["seq"] for w in result.get("write", [])]


def released(result):
    return [w["seq"] for w in result.get("release", [])]


def kinds(result):
    return [(e["kind"], e.get("seq", e.get("reason"))) for e in result["events"]]


def stop_event(result):
    return [e for e in result["events"] if e["kind"] == "stop"]


def press_events(out):
    """Every sent and dropped event of a run, in order."""
    return [e for r in out for e in r.get("events", []) if e["kind"] in ("sent", "dropped")]


def press_event(out, seq):
    found = [e for e in press_events(out) if e["seq"] == seq]
    if len(found) != 1:
        raise AssertionError(f"seq {seq} has {len(found)} press events: {found}")
    return found[0]


def down_op(out, seq):
    """The index of the op whose step wrote `seq`, or None."""
    for i, r in enumerate(out):
        if seq in written(r):
            return i
    return None


@unittest.skipIf(NODE is None, "node is not on PATH, so press_schedule.ts cannot run")
class FramePickTest(unittest.TestCase):
    """Task T2's acceptance lines, one test each, in its order."""

    def fire_clock(self, step_ms, target_ms):
        """The clock a plain entry at `target_ms` fires on when frames come
        `step_ms` apart from 960.0, and its event."""
        ops = ticks(960.0, step_ms, int(60.0 / step_ms))
        out = run(play([entry(1, target_ms, window=(900.0, 1100.0))], start_lead_ms=1.0), ops,
                  stop_after=True)
        i = down_op(out, 1)
        self.assertIsNotNone(i)
        return ops[i]["clock_ms"], press_event(out, 1)

    def test_a_plain_entry_fires_on_the_frame_whose_judge_time_is_nearest_its_target(self):
        # At 3.0 ms frames a press written on clock 993.0 is judged at 999.0
        # and one on 996.0 at 1002.0; 999.0 is nearer 1000.0, and 1002.0 is
        # the first judge time to reach it.
        clock, ev = self.fire_clock(3.0, 1000.0)
        self.assertEqual((clock, ev["step_ms"], ev["predicted_judge_ms"]), (993.0, 3.0, 999.0))
        # At 0.75 ms frames clock 998.25 is judged at 999.75 and 999.0 at
        # 1000.5; 999.75 is nearer. A fixed 3 ms frame would fire on 994.0.
        clock, ev = self.fire_clock(0.75, 1000.0)
        self.assertEqual((clock, ev["step_ms"], ev["predicted_judge_ms"]), (998.25, 0.75, 999.75))

    def test_a_plain_entry_never_fires_outside_its_window(self):
        # The target is the window's back edge. Judge times 999.0 (clock
        # 993.0) and 1002.0 (clock 996.0) come; 1002.0 is nearer but outside.
        ops = ticks(960.0, 3.0, 20)
        out = run(play([entry(1, 1001.0, window=(990.0, 1001.0))], start_lead_ms=1.0), ops,
                  stop_after=True)
        self.assertEqual(ops[down_op(out, 1)]["clock_ms"], 993.0)
        ev = press_event(out, 1)
        self.assertEqual((ev["predicted_judge_ms"], ev["no_frame_in_band"]), (999.0, False))

    def test_a_plain_entry_with_a_step_wider_than_its_window_is_not_sent(self):
        # 6.0 ms frames from 961.0 judge at 997.0 (clock 985.0) and then
        # 1003.0 (clock 991.0): the window 998.0 to 1002.0 gets no frame.
        ops = ticks(961.0, 6.0, 8)
        out = run(play([entry(1, 1000.0, window=(998.0, 1002.0))], start_lead_ms=1.0), ops)
        self.assertIsNone(down_op(out, 1))
        self.assertEqual(out[5]["events"], [
            {"kind": "dropped", "seq": 1, "index": 1, "clock_ms_at_press": 991.0,
             "down_frame": None, "up_frame": None, "down_ms": None, "up_ms": None,
             "score_before": 0, "score_after": 0, "frame_gap_ms": 3.0, "agent_note": "",
             "step_ms": 6.0, "predicted_judge_ms": 1003.0, "judged_ms": None,
             "judged_in_band": None, "no_frame_in_band": True},
            {"kind": "stop", "reason": "done", "frame": 15, "clock_ms": 991.0,
             "wall_ms": 5015.0},
        ])

    def straddled(self, e, first_clock, step_ms):
        ops = ticks(first_clock, step_ms, 16)
        out = run(play([e], start_lead_ms=1.0), ops, stop_after=True)
        i = down_op(out, 1)
        return (None if i is None else ops[i]["clock_ms"]), press_event(out, 1)

    def test_entries_the_frames_straddle_take_their_kinds_frame_inside_the_window(self):
        window = (900.0, 1100.0)
        # A late gem, band after 1050.0 to the window's back edge, at 60.0 ms
        # frames: clock 925.0 judges at 1045.0, before the band, and the next
        # at 1105.0, past the window. It fires on the frame before the band.
        clock, ev = self.straddled(entry(1, 1075.0, band=band(1050.0, 1100.0, True),
                                         window=window), 805.0, 60.0)
        self.assertEqual((clock, ev["predicted_judge_ms"], ev["no_frame_in_band"]),
                         (925.0, 1045.0, True))
        # An end-band from the window's front edge to an end at 920.0: clock
        # 765.0 judges at 885.0, before the window, and 825.0 at 945.0, past
        # the end. It fires on the first frame after the end.
        clock, ev = self.straddled(entry(1, 910.0, band=band(900.0, 920.0), window=window),
                                   705.0, 60.0)
        self.assertEqual((clock, ev["predicted_judge_ms"], ev["no_frame_in_band"]),
                         (825.0, 945.0, True))
        # A squeeze-in, end-band to 905.0 at 10.0 ms frames, falls back to its
        # chord's time 1000.0: clock 976.0 judges at 996.0, the nearest. The
        # first frame after the end, 906.0, is the end-band's pick, not this.
        clock, ev = self.straddled(entry(1, 902.5, band=band(900.0, 905.0), window=window,
                                         no_frame_aim_ms=1000.0), 856.0, 10.0)
        self.assertEqual((clock, ev["predicted_judge_ms"], ev["no_frame_in_band"]),
                         (976.0, 996.0, True))
        for judged in (1045.0, 945.0, 996.0):
            self.assertTrue(window[0] <= judged <= window[1])

    def test_a_banded_entry_with_no_frame_in_its_window_is_not_sent(self):
        # 30.0 ms frames judge at 985.0 (clock 925.0) and 1015.0 (clock
        # 955.0); the window 990.0 to 1010.0 gets neither.
        clock, ev = self.straddled(entry(1, 995.0, band=band(990.0, 1000.0),
                                         window=(990.0, 1010.0)), 895.0, 30.0)
        self.assertIsNone(clock)
        self.assertEqual((ev["kind"], ev["clock_ms_at_press"], ev["no_frame_in_band"]),
                         ("dropped", 955.0, True))

    def test_one_long_frame_after_a_press_shows_as_the_real_judge_times_excess(self):
        # 3.0 ms frames; the press goes down on clock 993.0, judged at 999.0
        # by prediction. The frame after it runs 13.0 ms, 10.0 ms over.
        ops = ticks(960.0, 3.0, 12) + [step(22, 1006.0, 5036.0), step(23, 1009.0, 5039.0),
                                       step(24, 1012.0, 5042.0), step(25, 1015.0, 5045.0)]
        out = run(play([entry(1, 1000.0, band=band(990.0, 1030.0), window=(900.0, 1100.0))],
                       start_lead_ms=1.0), ops, stop_after=True)
        self.assertEqual(ops[down_op(out, 1)]["clock_ms"], 993.0)
        ev = press_event(out, 1)
        self.assertEqual(ev["predicted_judge_ms"], 999.0)
        self.assertEqual(ev["judged_ms"] - ev["predicted_judge_ms"], 10.0)
        self.assertTrue(ev["judged_in_band"])

    def test_frames_that_turn_twice_as_long_show_on_each_later_events_real_judge_time(self):
        # Twenty 3.0 ms frames to clock 1000.0, then 6.0 ms frames. The
        # median stays 3.0, so each press is judged later than predicted.
        ops = ticks(940.0, 3.0, 21) + ticks(1006.0, 6.0, 10, first_frame=31, wall0=5063.0,
                                            wall_step=6.0)
        out = run(play([entry(1, 1018.0, SNARE, band=band(1016.0, 1020.0), window=(900.0, 1100.0)),
                        entry(2, 1036.0, TOM, band=band(1030.0, 1045.0), window=(900.0, 1100.0))],
                       start_lead_ms=1.0), ops, stop_after=True)
        one, two = press_event(out, 1), press_event(out, 2)
        # Entry 1 goes down on clock 1012.0 for 1018.0; the clock two frames
        # on is 1024.0, outside its band.
        self.assertEqual((one["clock_ms_at_press"], one["step_ms"], one["predicted_judge_ms"]),
                         (1012.0, 3.0, 1018.0))
        self.assertEqual((one["judged_ms"], one["judged_in_band"]), (1024.0, False))
        # Entry 2 goes down on clock 1030.0 for 1036.0 and is judged at
        # 1042.0, still inside its band.
        self.assertEqual((two["clock_ms_at_press"], two["predicted_judge_ms"]), (1030.0, 1036.0))
        self.assertEqual((two["judged_ms"], two["judged_in_band"]), (1042.0, True))

    def test_a_probe_entry_with_no_band_fires_on_the_frame_nearest_its_target(self):
        # Judge times 999.0 (clock 993.0) and 1002.0 (clock 996.0): 1002.0 is
        # nearer 1000.6.
        ops = ticks(960.0, 3.0, 20)
        out = run(play([entry(1, 1000.6)], start_lead_ms=1.0), ops, stop_after=True)
        self.assertEqual(ops[down_op(out, 1)]["clock_ms"], 996.0)
        self.assertEqual(press_event(out, 1)["predicted_judge_ms"], 1002.0)

    def test_one_long_frame_among_many_does_not_move_the_next_press(self):
        # Ten 3.0 ms frames, one of 50.0 ms, then 3.0 ms frames again.
        ops = ticks(900.0, 3.0, 11) + ticks(980.0, 3.0, 12, first_frame=21, wall0=5080.0)
        out = run(play([entry(1, 1010.0)], start_lead_ms=1.0), ops, stop_after=True)
        ev = press_event(out, 1)
        self.assertEqual((ev["clock_ms_at_press"], ev["step_ms"], ev["predicted_judge_ms"]),
                         (1004.0, 3.0, 1010.0))

    def test_a_banded_entry_with_several_frames_in_its_band_fires_on_the_one_nearest_at_ms(self):
        # The band 990.0 to 1030.0 holds judge times 990.0 to 1029.0; 1011.0
        # (clock 1005.0) is nearest 1012.0, not 990.0, the first inside.
        ops = ticks(960.0, 3.0, 30)
        out = run(play([entry(1, 1012.0, band=band(990.0, 1030.0), window=(900.0, 1100.0))],
                       start_lead_ms=1.0), ops, stop_after=True)
        self.assertEqual(ops[down_op(out, 1)]["clock_ms"], 1005.0)

    def test_a_band_narrower_than_the_step_takes_the_one_frame_inside_it(self):
        # 3.0 ms frames from 961.5 judge at 1000.5 (clock 994.5), the one
        # judge time inside the 2.0 ms band 1000.0 to 1002.0.
        ops = ticks(961.5, 3.0, 20)
        out = run(play([entry(1, 1001.0, band=band(1000.0, 1002.0), window=(900.0, 1100.0))],
                       start_lead_ms=1.0), ops, stop_after=True)
        self.assertEqual(ops[down_op(out, 1)]["clock_ms"], 994.5)
        ev = press_event(out, 1)
        self.assertEqual((ev["predicted_judge_ms"], ev["no_frame_in_band"]), (1000.5, False))

    def test_no_entry_fires_past_its_windows_back_edge(self):
        # A head: 3.0 ms frames to clock 984.0 (judged 990.0, but 993.0 is
        # nearer 995.0), then one long frame to clock 1000.0, judged 1006.0,
        # past the window's back edge 1000.0.
        ops = ticks(960.0, 3.0, 9) + [step(19, 1000.0, 5027.0)]
        out = run(play([entry(1, 995.0, window=(990.0, 1000.0))], start_lead_ms=1.0), ops,
                  stop_after=True)
        self.assertIsNone(down_op(out, 1))
        self.assertEqual(press_event(out, 1)["kind"], "dropped")
        # A later group: its head goes down on clock 993.0, judged 999.0; on
        # the next frame the group would be judged 1002.0, past 1000.0.
        ops = ticks(960.0, 3.0, 16)
        out = run(play([entry(1, 999.0, SNARE, window=(990.0, 1000.0)),
                        entry(2, 999.0, KICK, index=1, window=(990.0, 1000.0), after_seq=1)],
                       start_lead_ms=1.0), ops, stop_after=True)
        self.assertEqual(ops[down_op(out, 1)]["clock_ms"], 993.0)
        self.assertIsNone(down_op(out, 2))
        ev = press_event(out, 2)
        self.assertEqual((ev["kind"], ev["predicted_judge_ms"]), ("dropped", 1002.0))

    def test_a_judge_time_equal_to_a_strict_from_ms_is_not_inside_the_band(self):
        # Judge times 1006.0 (clock 1000.0) and 1009.0 (clock 1003.0); 1006.0
        # is nearer 1007.0. With from_strict it is outside, so the strict
        # entry takes 1009.0; the same band not strict takes 1006.0.
        ops = ticks(961.0, 3.0, 20)
        out = run(play([entry(1, 1007.0, SNARE, band=band(1006.0, 1030.0, True),
                              window=(900.0, 1100.0)),
                        entry(2, 1007.0, TOM, band=band(1006.0, 1030.0, False),
                              window=(900.0, 1100.0))], start_lead_ms=1.0), ops)
        self.assertEqual(ops[down_op(out, 1)]["clock_ms"], 1003.0)
        self.assertEqual(ops[down_op(out, 2)]["clock_ms"], 1000.0)
        self.assertEqual([press_event(out, s)["no_frame_in_band"] for s in (1, 2)],
                         [False, False])

    def later_group_frames(self, step_ms):
        ops = ticks(960.0, step_ms, int(60.0 / step_ms))
        out = run(play([entry(1, 1000.0, SNARE, window=(900.0, 1100.0)),
                        entry(2, 1000.0, KICK, index=1, window=(900.0, 1100.0), after_seq=1)],
                       start_lead_ms=1.0), ops, stop_after=True)
        return down_op(out, 1), down_op(out, 2)

    def test_a_later_group_fires_on_the_frame_right_after_its_heads(self):
        for step_ms in (3.0, 0.75):
            head, later = self.later_group_frames(step_ms)
            self.assertIsNotNone(head)
            self.assertEqual(later, head + 1, step_ms)

    def test_a_later_group_listed_before_its_head_still_waits_for_the_frame_after(self):
        # The group's seq comes first, so it is looked at before its head on
        # the head's own frame.
        ops = ticks(960.0, 3.0, 20)
        out = run(play([entry(2, 1000.0, SNARE, window=(900.0, 1100.0)),
                        entry(1, 1000.0, KICK, index=2, window=(900.0, 1100.0), after_seq=2)],
                       start_lead_ms=1.0), ops, stop_after=True)
        self.assertEqual(down_op(out, 1), down_op(out, 2) + 1)

    def test_a_judge_time_equal_to_a_star_power_end_is_inside_its_end_band(self):
        # Judge times 1009.0, 1012.0 (clock 1006.0) and 1015.0; the end-band
        # stops at the end 1012.0, and 1012.0 is nearest 1011.5.
        ops = ticks(961.0, 3.0, 25)
        out = run(play([entry(1, 1011.5, band=band(900.0, 1012.0), window=(900.0, 1100.0))],
                       start_lead_ms=1.0), ops, stop_after=True)
        self.assertEqual(ops[down_op(out, 1)]["clock_ms"], 1006.0)
        ev = press_event(out, 1)
        self.assertEqual((ev["predicted_judge_ms"], ev["no_frame_in_band"]), (1012.0, False))

    def test_no_entry_fires_before_one_step_is_measured(self):
        # On the first frame any judge time is past 1000.5, but no step is
        # known yet.
        out = run(play([entry(1, 1000.5)], start_lead_ms=0.0),
                  [step(10, 1000.0, 5000.0), step(11, 1003.0, 5003.0)], stop_after=True)
        self.assertEqual([written(r) for r in out[:2]], [[], [1]])
        self.assertEqual(press_event(out, 1)["step_ms"], 3.0)

    def test_every_press_event_carries_the_step_and_the_predicted_judge_time(self):
        ops = ticks(960.0, 3.0, 30)
        out = run(play([entry(1, 1000.0, SNARE, window=(900.0, 1100.0)),
                        entry(2, 1000.0, KICK, window=(900.0, 1100.0), after_seq=1),
                        entry(3, 1001.0, TOM, window=(990.0, 1001.0))],
                       start_lead_ms=1.0), ops + [step(40, 2000.0, 5090.0)])
        events = press_events(out)
        self.assertEqual(sorted(e["kind"] for e in events), ["sent", "sent", "sent"])
        dropped = run(play([entry(1, 1000.0, window=(998.0, 1002.0))], start_lead_ms=1.0),
                      ticks(961.0, 6.0, 8))
        events.append(press_event(dropped, 1))
        self.assertEqual(events[-1]["kind"], "dropped")
        for e in events:
            for key in ("step_ms", "predicted_judge_ms"):
                self.assertIsInstance(e[key], (int, float), (key, e))
                self.assertNotIsInstance(e[key], bool, (key, e))

    def test_an_entry_with_a_retired_key_is_refused(self):
        for key in ("last_send_ms", "after_ms"):
            e = {**entry(1, 1000.0), key: 1000.0}
            out = run_raw(play([e]), [])
            self.assertIn(f'gives "{key}"', out[-1]["error"])


@unittest.skipIf(NODE is None, "node is not on PATH, so press_schedule.ts cannot run")
class PressScheduleTest(unittest.TestCase):
    """Holds, skips, retimes and stops, from the in-game timing plan."""

    def test_a_press_is_released_exactly_hold_frames_frames_after_its_down(self):
        # Each step is one real reading of the game's wall time, song clock and
        # score, in order, from the Star Power watch of a real run (see
        # SCORE_WINDOW_CSV). The game scored chord 144 there, so the score
        # moves twice inside these seven readings.
        walls, clocks, scores = score_window()
        # The entry is aimed at the judge time of a press on the second
        # reading, the first with a step. The start lead is shorter than the
        # gap from the first reading, so the entry is not skipped at load.
        step1 = clocks[1] - clocks[0]
        out = run(play([entry(1, clocks[1] + 2 * step1)], hold_frames=3, start_lead_ms=1.0), [
            step(10 + k, clocks[k], walls[k], score=scores[k]) for k in range(len(walls))
        ])
        self.assertEqual(written(out[1]), [1])
        self.assertEqual([released(r) for r in out], [[], [], [], [], [1], [], []])
        # The score after is read on the first reading at least the score delay
        # after the release: the sixth reading is too soon, the seventh is not.
        delay = play([])["score_delay_ms"]
        self.assertLess(walls[5], walls[4] + delay)
        self.assertGreaterEqual(walls[6], walls[4] + delay)
        # The real score differs on the down, the sixth and the seventh
        # readings, so a read on the wrong one shows.
        self.assertEqual(len({scores[1], scores[5], scores[6]}), 3)
        self.assertEqual(out[5]["events"], [])
        ev = out[6]["events"][0]
        self.assertEqual({k: ev[k] for k in ("kind", "seq", "index", "clock_ms_at_press",
                                             "down_frame", "up_frame", "down_ms", "up_ms",
                                             "score_before", "score_after", "frame_gap_ms",
                                             "agent_note")}, {
            "kind": "sent", "seq": 1, "index": 1, "clock_ms_at_press": clocks[1],
            "down_frame": 11, "up_frame": 14, "down_ms": walls[1], "up_ms": walls[4],
            "score_before": scores[1], "score_after": scores[6],
            "frame_gap_ms": walls[1] - walls[0], "agent_note": ""})
        # Its judge time is the clock read two frames after its down.
        self.assertEqual(ev["judged_ms"], clocks[3])

    def test_a_held_pad_delays_its_next_press_to_the_frame_after_the_release(self):
        out = run(play([entry(1, 1006.0, SNARE), entry(2, 1009.0, SNARE)], start_lead_ms=1.0), [
            step(10, 997.0, 5000.0),
            step(11, 1000.0, 5003.0),
            step(12, 1003.0, 5006.0),
            step(13, 1006.0, 5009.0),
            step(14, 1009.0, 5012.0),
        ])
        self.assertEqual([written(r) for r in out], [[], [1], [], [], [2]])
        self.assertEqual([released(r) for r in out], [[], [], [], [1], []])

    def test_entries_inside_the_start_lead_are_skipped_and_counted(self):
        # The first clock is 900 and the lead 150, so heads before 1050 are
        # skipped, with the later group of the head at 1040.
        out = run(play([entry(1, 1000.0, SNARE),
                        entry(2, 1040.0, TOM),
                        entry(3, 1060.0, KICK, index=2, after_seq=2),
                        entry(4, 1050.0, CYMBAL)]), [
            step(10, 900.0, 5000.0),
        ])
        self.assertEqual(out[0]["events"], [
            {"kind": "skipped", "seqs": [1, 2, 3], "count": 3, "clock_ms": 900.0}])
        self.assertEqual(written(out[0]), [])

    def test_a_first_frame_that_skips_every_entry_stops_as_past_end(self):
        out = run(play([entry(1, 1000.0), entry(2, 1020.0, KICK, after_seq=1)]),
                  [step(10, 900.0, 5000.0)])
        self.assertEqual(kinds(out[0]), [("skipped", None), ("stop", "past_end")])
        self.assertEqual(out[0]["events"][0]["seqs"], [1, 2])

    def test_a_dropped_head_drops_its_later_group_on_the_same_frame(self):
        out = run(play([entry(1, 1000.0, SNARE, window=(990.0, 1010.0)),
                        entry(2, 1000.0, KICK, index=1, window=(990.0, 1010.0), after_seq=1)]), [
            step(10, 800.0, 5000.0),
            step(11, 1500.0, 5900.0),
        ])
        self.assertEqual(written(out[1]), [])
        self.assertEqual(kinds(out[1]), [("dropped", 1), ("dropped", 2), ("stop", "done")])

    def test_a_retime_replaces_only_entries_not_yet_fired(self):
        # Entry 1 goes down on clock 1000.0, judged 1006.0. The retime moves
        # entry 2 to 1156.0, which a press on clock 1150.0 meets, and drops 3.
        ops = [step(10, 997.0, 5000.0), step(11, 1000.0, 5003.0)]
        later = ticks(1003.0, 3.0, 70, first_frame=12, wall0=5006.0)
        out = run(play([entry(1, 1006.0, SNARE), entry(2, 1106.0, TOM),
                        entry(3, 1206.0, KICK)], start_lead_ms=1.0),
                  ops + [{"op": "retime", "read_after": 9,
                          "entries": [entry(1, 2000.0, SNARE), entry(2, 1156.0, TOM)]}] + later)
        self.assertEqual(out[2], {"removed": [3], "ignored": [1]})
        self.assertEqual(down_op(out, 1), 1)
        self.assertEqual(later[down_op(out, 2) - 3]["clock_ms"], 1150.0)
        self.assertEqual(sorted(e["seq"] for e in press_events(out)), [1, 2])

    def test_an_end_read_after_entry_with_no_retime_fires_at_its_planned_time_as_retime_missed(self):
        # Entry 1 waits on the read after chord 5, whose retime never comes;
        # entry 2 waits on the read after chord 6, whose retime arrives with
        # it unmoved. A retime lists every entry not yet reported, so it lists
        # entry 1 too.
        one = entry(1, 1006.0, SNARE, end_read_after=5)
        two = entry(2, 1006.0, TOM, end_read_after=6)
        out = run(play([one, two], start_lead_ms=1.0), [
            step(10, 997.0, 5000.0),
            {"op": "retime", "read_after": 6, "entries": [one, two]},
            step(11, 1000.0, 5003.0),
            step(12, 1003.0, 5006.0),
            step(13, 1006.0, 5009.0),
            step(14, 1009.0, 5014.0),
        ])
        self.assertEqual(written(out[2]), [1, 2])
        notes = {e["seq"]: e["agent_note"] for e in out[5]["events"] if e["kind"] == "sent"}
        self.assertEqual(notes, {1: "retime_missed", 2: ""})

    # The four stops: each lifts every pad down and reports its reason.

    def held_two(self, hold_frames=5):
        """The first three frames of the stop cases: two pads go down on
        frame 11, judged at 1006.0, and are held past frame 12."""
        return play([entry(1, 1006.0, SNARE), entry(2, 1006.0, TOM),
                     entry(3, 1500.0, KICK)], hold_frames=hold_frames, stall_ms=100.0,
                    cap_ms=1000.0, start_lead_ms=1.0), [
            step(10, 997.0, 5000.0),
            step(11, 1000.0, 5003.0),
            step(12, 1003.0, 5006.0),
        ]

    def check_stop(self, result, reason, frame, clock_ms, wall_ms):
        self.assertEqual(released(result), [1, 2])
        self.assertEqual(written(result), [])
        self.assertEqual(kinds(result), [("sent", 1), ("sent", 2), ("stop", reason)])
        self.assertEqual([e["score_after"] for e in result["events"][:2]], [None, None])
        self.assertEqual([e["up_frame"] for e in result["events"][:2]], [frame, frame])
        self.assertEqual(stop_event(result), [
            {"kind": "stop", "reason": reason, "frame": frame, "clock_ms": clock_ms,
             "wall_ms": wall_ms}])

    def test_a_clock_that_jumps_back_releases_every_pad_and_reports_jumped_back(self):
        msg, ops = self.held_two()
        out = run(msg, ops + [step(13, 2.0, 5009.0), step(14, 5.0, 5012.0)])
        self.check_stop(out[3], "jumped_back", 13, 2.0, 5009.0)
        self.assertEqual(out[4], {"write": [], "release": [], "events": []})

    def test_a_clock_that_stops_releases_every_pad_and_reports_frozen(self):
        # The clock holds at 1003 from wall 5006; the stall is 100 ms, so
        # wall 5106 still plays and 5107 stops.
        msg, ops = self.held_two(hold_frames=50)
        out = run(msg, ops + [step(13, 1003.0, 5106.0), step(14, 1003.0, 5107.0)])
        self.assertEqual(out[3]["events"], [])
        self.check_stop(out[4], "frozen", 14, 1003.0, 5107.0)

    def test_stop_play_releases_every_pad_and_reports_stopped(self):
        msg, ops = self.held_two()
        out = run(msg, ops + [{"op": "stop", "wall_ms": 5007.5}, step(13, 1006.0, 5009.0)])
        self.check_stop(out[3], "stopped", None, 1003.0, 5007.5)
        self.assertEqual(out[4], {"write": [], "release": [], "events": []})

    def test_the_cap_releases_every_pad_and_reports_cap(self):
        # The cap is 1000 ms of wall time from the first frame at 5000.
        msg, ops = self.held_two(hold_frames=500)
        out = run(msg, ops + [step(13, 1006.0, 5999.0), step(14, 1009.0, 6000.0)])
        self.assertEqual(stop_event(out[3]), [])
        self.check_stop(out[4], "cap", 14, 1009.0, 6000.0)

    def test_a_900_ms_frame_gap_drops_exactly_the_entries_whose_window_it_passed(self):
        # Ten 3.0 ms frames to clock 987.0, then one of 900.0 ms to 1887.0,
        # judged 1893.0: the windows of entries 1 and 2 are past, those of 3
        # and 4 are not.
        ops = ticks(960.0, 3.0, 10) + [step(20, 1887.0, 5927.0)]
        out = run(play([entry(1, 1000.0, SNARE, window=(990.0, 1010.0)),
                        entry(2, 1500.0, TOM, window=(1490.0, 1510.0)),
                        entry(3, 1900.0, KICK, window=(1880.0, 1920.0)),
                        entry(4, 1950.0, CYMBAL, window=(1890.0, 2010.0)),
                        entry(5, 2500.0, SNARE)], start_lead_ms=1.0), ops)
        self.assertEqual(kinds(out[10]), [("dropped", 1), ("dropped", 2)])
        self.assertEqual([(e["clock_ms_at_press"], e["frame_gap_ms"]) for e in out[10]["events"]],
                         [(1887.0, 900.0), (1887.0, 900.0)])


if __name__ == "__main__":
    unittest.main()
