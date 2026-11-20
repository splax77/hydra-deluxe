"""Tests for agent/press_schedule.ts, the frame scheduler of the in-game press
timing plan, one test per line of the acceptance list of its task IG1. The
plan is docs/superpowers/plans/2026-11-12-in-game-press-timing.md (removed; git show b971b235:docs/superpowers/plans/2026-11-12-in-game-press-timing.md).

No game and no Frida. Plain Node runs press_schedule.ts (Node erases its type
annotations), as test_launch_core.py runs launch_core.ts: each case is a
list of steps, retimes and stops sent to a small driver on stdin, and the
driver prints what the scheduler returned for each. The score it reads is
whatever the case's latest step says. The tests skip, naming the reason, when
`node` is not on PATH.
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


def entry(seq, at_ms, pad=SNARE, *, index=None, last_send_ms=None, after_ms=None,
          after_seq=None, end_read_after=None):
    """One schedule entry. Its chord index is its seq unless given, and its
    last send is far past its fire time unless given."""
    return {
        "seq": seq,
        "index": seq if index is None else index,
        "at_ms": at_ms,
        "last_send_ms": at_ms + 500.0 if last_send_ms is None else last_send_ms,
        "elements": [{"id": pad, "value": True}],
        "after_ms": after_ms,
        "after_seq": after_seq,
        "end_read_after": end_read_after,
    }


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


def run(play_msg, ops):
    driver = DRIVER % {"url": json.dumps(pathlib.Path(SCHEDULER).as_uri())}
    done = subprocess.run(
        [NODE, "--input-type=module", "-e", driver],
        input=json.dumps({"play": play_msg, "ops": ops}), capture_output=True,
        text=True, timeout=NODE_CAP_S, check=False)
    if done.returncode != 0:
        raise AssertionError(f"node exited {done.returncode}: {done.stderr}")
    out = json.loads(done.stdout)
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
    return [w["seq"] for w in result["write"]]


def released(result):
    return [w["seq"] for w in result["release"]]


def kinds(result):
    return [(e["kind"], e.get("seq", e.get("reason"))) for e in result["events"]]


def stop_event(result):
    return [e for e in result["events"] if e["kind"] == "stop"]


@unittest.skipIf(NODE is None, "node is not on PATH, so press_schedule.ts cannot run")
class PressScheduleTest(unittest.TestCase):
    def test_an_entry_fires_on_the_first_frame_at_or_past_at_ms_and_not_the_frame_before(self):
        out = run(play([entry(1, 1000.0, SNARE), entry(2, 1001.5, TOM)]), [
            step(10, 800.0, 5000.0),
            step(11, 999.9, 5003.0),
            step(12, 1000.0, 5006.0),
            step(13, 1003.0, 5009.0),
        ])
        self.assertEqual([written(r) for r in out], [[], [], [1], [2]])

    def test_an_entry_past_last_send_ms_on_that_frame_is_dropped_and_nothing_is_written(self):
        out = run(play([entry(1, 1000.0, last_send_ms=1010.0)]), [
            step(10, 800.0, 5000.0, score=40),
            step(11, 990.0, 5003.0),
            step(12, 1012.0, 5006.0),
        ])
        self.assertEqual(written(out[2]), [])
        self.assertEqual(out[2]["events"], [
            {"kind": "dropped", "seq": 1, "index": 1, "clock_ms_at_press": 1012.0,
             "down_frame": None, "up_frame": None, "down_ms": None, "up_ms": None,
             "score_before": 40, "score_after": 40, "frame_gap_ms": 3.0, "agent_note": ""},
            {"kind": "stop", "reason": "done", "frame": 12, "clock_ms": 1012.0,
             "wall_ms": 5006.0},
        ])

    def test_a_later_group_fires_after_ms_after_its_heads_real_fire_clock(self):
        # The head is due at 1000 but its first frame is 1004, so the later
        # group, planned at 1010, is due at 1014.
        out = run(play([entry(1, 1000.0, SNARE),
                        entry(2, 1010.0, KICK, index=1, after_ms=10.0, after_seq=1)]), [
            step(10, 800.0, 5000.0),
            step(11, 1004.0, 5003.0),
            step(12, 1010.0, 5006.0),
            step(13, 1013.9, 5009.0),
            step(14, 1014.0, 5012.0),
        ])
        self.assertEqual([written(r) for r in out], [[], [1], [], [], [2]])

    def test_a_later_group_due_past_its_last_send_ms_still_goes_with_its_head(self):
        # The schedule gives a later group its head's last_send_ms. The head
        # fires at 1004, inside its limit of 1008; the later group falls due at
        # 1014, past that limit, and goes anyway, as play_chords sends it.
        out = run(play([entry(1, 1000.0, SNARE, last_send_ms=1008.0),
                        entry(2, 1010.0, KICK, index=1, last_send_ms=1008.0,
                              after_ms=10.0, after_seq=1)]), [
            step(10, 800.0, 5000.0),
            step(11, 1004.0, 5003.0),
            step(12, 1014.0, 5006.0),
        ])
        self.assertEqual([written(r) for r in out], [[], [1], [2]])
        self.assertNotIn("dropped", [k for r in out for k in kinds(r)])

    def test_a_press_is_released_exactly_hold_frames_frames_after_its_down(self):
        # Each step is one real reading of the game's wall time, song clock and
        # score, in order, from the Star Power watch of a real run (see
        # SCORE_WINDOW_CSV). The game scored chord 144 there, so the score
        # moves twice inside these seven readings.
        walls, clocks, scores = score_window()
        # The entry is due on the second reading's clock. The start lead is
        # shorter than the gap from the first reading, so the entry is not
        # skipped at load.
        out = run(play([entry(1, clocks[1])], hold_frames=3, start_lead_ms=1.0), [
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
        self.assertEqual(out[6]["events"][0], {
            "kind": "sent", "seq": 1, "index": 1, "clock_ms_at_press": clocks[1],
            "down_frame": 11, "up_frame": 14, "down_ms": walls[1], "up_ms": walls[4],
            "score_before": scores[1], "score_after": scores[6],
            "frame_gap_ms": walls[1] - walls[0], "agent_note": ""})

    def test_a_held_pad_delays_its_next_press_to_the_frame_after_the_release(self):
        out = run(play([entry(1, 1000.0, SNARE), entry(2, 1003.0, SNARE)]), [
            step(10, 800.0, 5000.0),
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
                        entry(3, 1060.0, KICK, index=2, after_ms=20.0, after_seq=2),
                        entry(4, 1050.0, CYMBAL)]), [
            step(10, 900.0, 5000.0),
        ])
        self.assertEqual(out[0]["events"], [
            {"kind": "skipped", "seqs": [1, 2, 3], "count": 3, "clock_ms": 900.0}])
        self.assertEqual(written(out[0]), [])

    def test_a_first_frame_that_skips_every_entry_stops_as_past_end(self):
        out = run(play([entry(1, 1000.0), entry(2, 1020.0, KICK, after_ms=20.0, after_seq=1)]),
                  [step(10, 900.0, 5000.0)])
        self.assertEqual(kinds(out[0]), [("skipped", None), ("stop", "past_end")])
        self.assertEqual(out[0]["events"][0]["seqs"], [1, 2])

    def test_a_dropped_head_drops_its_later_group_on_the_same_frame(self):
        out = run(play([entry(1, 1000.0, SNARE, last_send_ms=1010.0),
                        entry(2, 1010.0, KICK, index=1, after_ms=10.0, after_seq=1)]), [
            step(10, 800.0, 5000.0),
            step(11, 1500.0, 5900.0),
        ])
        self.assertEqual(written(out[1]), [])
        self.assertEqual(kinds(out[1]), [("dropped", 1), ("dropped", 2), ("stop", "done")])

    def test_a_retime_replaces_only_entries_not_yet_fired(self):
        out = run(play([entry(1, 1000.0, SNARE), entry(2, 1100.0, TOM),
                        entry(3, 1200.0, KICK)]), [
            step(10, 800.0, 5000.0),
            step(11, 1000.0, 5003.0),
            {"op": "retime", "read_after": 9,
             "entries": [entry(1, 2000.0, SNARE), entry(2, 1150.0, TOM)]},
            step(12, 1100.0, 5006.0),
            step(13, 1150.0, 5009.0),
            step(14, 1200.0, 5012.0),
            step(15, 2000.0, 5015.0),
        ])
        self.assertEqual(out[2], {"removed": [3], "ignored": [1]})
        self.assertEqual([written(r) for i, r in enumerate(out) if i != 2],
                         [[], [1], [], [2], [], []])

    def test_an_end_read_after_entry_with_no_retime_fires_at_its_planned_time_as_retime_missed(self):
        # Entry 1 waits on the read after chord 5, whose retime never comes;
        # entry 2 waits on the read after chord 6, whose retime arrives with
        # it unmoved. A retime lists every entry not yet reported, so it lists
        # entry 1 too.
        one = entry(1, 1000.0, SNARE, end_read_after=5)
        two = entry(2, 1000.0, TOM, end_read_after=6)
        out = run(play([one, two]), [
            step(10, 800.0, 5000.0),
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
        frame 11 and are held past frame 12."""
        return play([entry(1, 1000.0, SNARE), entry(2, 1000.0, TOM),
                     entry(3, 1500.0, KICK)], hold_frames=hold_frames, stall_ms=100.0,
                    cap_ms=1000.0), [
            step(10, 800.0, 5000.0),
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

    def test_a_900_ms_frame_gap_drops_exactly_the_entries_past_last_send_ms_and_fires_the_rest(self):
        out = run(play([entry(1, 1000.0, SNARE, last_send_ms=1100.0),
                        entry(2, 1500.0, TOM, last_send_ms=1600.0),
                        entry(3, 1800.0, KICK, last_send_ms=1890.0),
                        entry(4, 1850.0, CYMBAL, last_send_ms=1950.0),
                        entry(5, 2500.0, SNARE)]), [
            step(10, 800.0, 5000.0),
            step(11, 990.0, 5003.0),
            step(12, 1890.0, 5903.0),
        ])
        self.assertEqual(written(out[2]), [3, 4])
        self.assertEqual(kinds(out[2]), [("dropped", 1), ("dropped", 2)])
        self.assertEqual([(e["clock_ms_at_press"], e["frame_gap_ms"]) for e in out[2]["events"]],
                         [(1890.0, 900.0), (1890.0, 900.0)])


if __name__ == "__main__":
    unittest.main()
