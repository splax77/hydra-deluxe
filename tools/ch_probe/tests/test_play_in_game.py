"""play_chart.play_in_game against a real run's recorded kit agent events,
and, where no run recorded the case, against a fake kit agent.

Run from the repo root with:
    py -m pytest -q -p no:cacheprovider tools/ch_probe/tests/test_play_in_game.py
"""

from __future__ import annotations

import csv
import io
import json
import os
import queue
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import kit_tools as K  # noqa: E402
from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe.experiments import live  # noqa: E402
from tools.ch_probe.experiments import play_chart as P  # noqa: E402
from tools.ch_probe.experiments import score_compare  # noqa: E402
from tools.ch_probe.experiments.overlay_logs import (  # noqa: E402
    FRAME_PICK_PRESS_COLUMNS, KIT_PRESS_COLUMNS)
from tools.ch_probe.input_driver import Lane  # noqa: E402
from tools.ch_probe.tests.test_play_chart import _unit_end_fields  # noqa: E402

# Files copied from hydra-data/ch-mcp-runs/2026-11-12/ls-g3-run1 (each test
# names which lines). The run played its chords on the kit agent and wrote
# every event the agent sent to kit-1.jsonl.
_G3 = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-g3-run1")

# The addresses a run hands the agent; nothing reads them in a test.
CLOCK_ADDR, SCORE_ADDR = 0x100, 0x200

def press(index, chart_ms, lanes, **kw):
    """A hand-built Press. One timed from an end read gets
    test_play_chart._unit_end_fields of its aim_end_ms for any end field the
    keywords leave out."""
    fields = dict(index=index, chart_ms=chart_ms, aim_ms=0.0, lanes=list(lanes),
                  dynamics=["none"] * len(lanes), axes=[1.0] * len(lanes), declined=False,
                  rule="plain")
    fields.update(kw)
    if fields.get("end_read_after") is not None:
        for key, value in _unit_end_fields(fields["aim_end_ms"]).items():
            fields.setdefault(key, value)
    return P.Press(**fields)


def chord(index, ms, *presses):
    lanes = [l for p in presses for l in p.lanes]
    return P.Chord(index, index * 100, ms, lanes, ["none"] * len(lanes), presses=presses)


def sent_event(seq, chord_index, clock_ms, score_before, score_after):
    """A sent event in the shape the kit agent sends (K.PRESS_EVENT_FIELDS),
    the frame-pick fields hand-picked test inputs."""
    event = {"kind": K.EVENT_SENT, "seq": seq, "index": chord_index,
             "clock_ms_at_press": clock_ms, "down_frame": 10 + seq,
             "up_frame": 12 + seq, "down_ms": 1_000_000.0 + seq, "up_ms": 1_000_006.0 + seq,
             "score_before": score_before, "score_after": score_after,
             "frame_gap_ms": 3.0, "agent_note": "", "step_ms": 3.0,
             "predicted_judge_ms": clock_ms, "judged_ms": clock_ms,
             "judged_in_band": True, "no_frame_in_band": False}
    assert set(K.PRESS_EVENT_FIELDS) <= set(event)
    return event


def started(seqs=()):
    return {"kind": K.EVENT_SKIPPED, "seqs": list(seqs), "count": len(seqs),
            "clock_ms": 0.0}


def stopped(reason):
    return {"kind": K.EVENT_STOP, "reason": reason, "frame": 1, "clock_ms": 0.0,
            "wall_ms": 0.0}


class FakePlayer:
    """kit_tools.GamePlayer's three parts. `script` maps a posted message's
    type to the events to queue after it, each list used once in order."""

    def __init__(self, script):
        self.events = queue.Queue()
        self.posted = []
        self.script = {k: list(v) for k, v in script.items()}

    def alive(self):
        return True

    def post(self, message):
        self.posted.append(message)
        batches = self.script.get(message["type"]) or []
        if batches:
            for event in batches.pop(0):
                self.events.put(event)
        return {"ok": True}


# The g3 recording predates the agent's frame pick, so its sent and dropped
# events lack the frame-pick fields K.PRESS_EVENT_FIELDS asks of every press
# event. These values are the test's own, not the run's: with_frame_pick adds
# them to a recorded event and leaves every field the run recorded as it was.
# A press with no frame in its band would change the row's agent_note, a
# column the run wrote, so no_frame_in_band is False.
RECORDING_FRAME_PICK = {"step_ms": 3.0, "predicted_judge_ms": 1000.0, "judged_ms": 1001.0,
                        "judged_in_band": True, "no_frame_in_band": False}


def with_frame_pick(event):
    """A recorded press event with RECORDING_FRAME_PICK's fields added; any
    other event as recorded."""
    if event["kind"] not in (K.EVENT_SENT, K.EVENT_DROPPED):
        return event
    return {**RECORDING_FRAME_PICK, **event}


def assert_rows_are_the_runs(test, written_lines, recorded_lines):
    """`written_lines`, a press log play_in_game's rows gave, holds the
    recorded log's rows in every column the run wrote, and in each frame-pick
    column the run's log lacks, RECORDING_FRAME_PICK's value."""
    written = list(csv.DictReader(written_lines))
    recorded = list(csv.DictReader(recorded_lines))
    recorded_columns = next(csv.reader(recorded_lines[:1]))
    added = [c for c in next(csv.reader(written_lines[:1])) if c not in recorded_columns]
    test.assertEqual(added, list(FRAME_PICK_PRESS_COLUMNS))
    test.assertEqual(len(written), len(recorded))
    for row, run_row in zip(written, recorded):
        test.assertEqual({c: row[c] for c in recorded_columns}, run_row)
        test.assertEqual({c: row[c] for c in added},
                         {c: str(RECORDING_FRAME_PICK[c]) for c in added})


class RecordedPlayer:
    """kit_tools.GamePlayer's three parts over a kit agent's recorded
    messages (a run's kit-N.jsonl). The first play message posted queues
    every play_events event the file holds, in its order, keeping the sent
    and dropped events of `seqs` only, each with_frame_pick; a later post
    queues nothing. Once the queue is empty the session reads as gone, so a
    run that waits for an event the recording lacks ends at once as "lost"."""

    def __init__(self, path, seqs):
        keep = set(seqs)
        self.recorded = []
        with open(path, encoding="utf-8") as f:
            for line in f:
                payload = json.loads(line)["payload"]
                if payload.get("type") != "play_events":
                    continue
                self.recorded.extend(
                    with_frame_pick(e) for e in payload["events"]
                    if e["kind"] not in (K.EVENT_SENT, K.EVENT_DROPPED) or e["seq"] in keep)
        self.events = queue.Queue()
        self.posted = []

    def alive(self):
        return not self.events.empty()

    def post(self, message):
        self.posted.append(message)
        if message["type"] == K.PLAY_TYPE and len(self.posted) == 1:
            for event in self.recorded:
                self.events.put(event)
        return {"ok": True}


def run(chords, player, **kw):
    return P.play_in_game(chords, player, clock_addr=CLOCK_ADDR, score_addr=SCORE_ADDR,
                          out=io.StringIO(), **kw)


def recorded_log(name):
    """The lines of a press log copied from the g3 run."""
    with open(os.path.join(_G3, name), encoding="utf-8") as f:
        return f.read().splitlines()


def recorded_chords(log_lines):
    """The chords of a recorded press log, at the run's own chart times. A
    chord's first row is its first order group, group 0; a further row of
    the same index is a later group, group 1, which follows group 0 at the
    same judge target. Lanes reach only the play message, which no row
    reads."""
    presses = {}
    for r in csv.DictReader(log_lines):
        index, ms = int(r["index"]), float(r["chart_ms"])
        if index not in presses:
            presses[index] = [press(index, ms, [Lane.RED])]
        else:
            presses[index].append(press(index, ms, [Lane.BLUE], group=1, follows=0))
    return [chord(index, group[0].chart_ms, *group) for index, group in presses.items()]


def repressed(one_press_chord, lanes, **kw):
    """A one-press chord's press made again on `lanes` with `kw`, and the
    chord holding it."""
    p = one_press_chord.presses[0]
    q = press(p.index, p.chart_ms, lanes, **kw)
    return q, chord(p.index, p.chart_ms, q)


def watched_sp_end(active):
    """The Star Power end the g3 run's sp_watch-1.csv recorded on its first
    row whose active byte is set (`active`) or clear, read the way
    engine.EngineModel.sp_end reads the same two fields. The file is the
    watch's header, its first row and six rows around its first activation."""
    with open(os.path.join(_G3, "sp_watch-1.csv"), encoding="utf-8") as f:
        for row in csv.DictReader(f):
            if (int(row["sp_active_byte"]) != 0) == active:
                return float(row["sp_end_s"]) if active else None
    raise AssertionError(f"sp_watch-1.csv holds no row with the active byte "
                         f"{'set' if active else 'clear'}")


class PlayInGameTest(unittest.TestCase):
    def test_the_play_message_carries_the_schedule_and_every_number(self):
        # The agent's side is the g3 run's recorded events (kit-1-head.jsonl,
        # see test_a_real_runs_agent_events_write_the_press_log_that_run_wrote).
        chords = recorded_chords(recorded_log("press-1-head.csv"))
        player = RecordedPlayer(os.path.join(_G3, "kit-1-head.jsonl"), seqs=range(len(chords)))
        cap_s = 30.0
        result = run(chords, player, cap_s=cap_s)
        self.assertEqual(result.stop, P.STOP_DONE, result.message)
        (message,) = player.posted
        self.assertEqual(message["type"], K.PLAY_TYPE)
        # Each number is the one its owner names: the caller's addresses and
        # cap, play_chart's score delay, live's start lead and stop checks,
        # and kit_tools' hold.
        self.assertEqual(
            {key: message[key] for key in ("clock_addr", "score_addr", "cap_ms",
                                           "score_delay_ms", "start_lead_ms", "hold_frames",
                                           "stall_ms", "jump_back_ms")},
            {"clock_addr": CLOCK_ADDR, "score_addr": SCORE_ADDR, "cap_ms": C.s_to_ms(cap_s),
             "score_delay_ms": C.s_to_ms(P.SCORE_READ_DELAY_S),
             "start_lead_ms": live.START_LEAD_MS, "hold_frames": K.KIT_HOLD_FRAMES,
             "stall_ms": C.s_to_ms(live.STALL_S), "jump_back_ms": C.s_to_ms(live.JUMP_BACK_S)})
        # One entry per press, in schedule order, each carrying its press's
        # chord, judge target and lanes, and no send time.
        presses = [p for c in chords for p in c.presses]
        self.assertEqual(len(message["entries"]), len(presses))
        for seq, (entry, p) in enumerate(zip(message["entries"], presses)):
            self.assertEqual((entry["seq"], entry["index"], entry["at_ms"],
                              entry["after_seq"], entry["end_read_after"]),
                             (seq, p.index, p.at_ms, None, None))
            self.assertNotIn("last_send_ms", entry)
            self.assertEqual([e["id"] for e in entry["elements"]],
                             [K.lane_element(lane).id for lane in p.lanes])

    def test_a_real_runs_rows_load_into_score_compare_with_every_kit_field(self):
        # The g3 run's events (kit-1-head.jsonl) give rows whose press log
        # score_compare.load_press_log, the comparer's own reader, reads back:
        # one Press per row, with the score the agent read and every kit
        # column filled.
        chords = recorded_chords(recorded_log("press-1-head.csv"))
        player = RecordedPlayer(os.path.join(_G3, "kit-1-head.jsonl"), seqs=range(len(chords)))
        result = run(chords, player)
        self.assertEqual(result.stop, P.STOP_DONE, result.message)
        scratch = tempfile.TemporaryDirectory()
        self.addCleanup(scratch.cleanup)
        written = os.path.join(scratch.name, "press.csv")
        P.write_press_log(written, result.rows)
        loaded = score_compare.load_press_log(written)
        events = [e for e in player.recorded if e["kind"] == K.EVENT_SENT]
        self.assertEqual(len(loaded), len(events))
        for row, event in zip(loaded, events):
            self.assertEqual((row.index, row.clock_ms_at_press, row.score_before,
                              row.score_after),
                             (event["index"], event["clock_ms_at_press"],
                              event["score_before"], event["score_after"]))
            for column in KIT_PRESS_COLUMNS:
                self.assertNotEqual(getattr(row, column), "", column)

    def test_a_real_runs_agent_events_write_the_press_log_that_run_wrote(self):
        # The kit agent's events of the first three chords of a real run, and
        # the run's stop, read back through play_in_game and write_press_log,
        # give the press log rows that run wrote. Both files are copied from
        # hydra-data/ch-mcp-runs/2026-11-12/ls-g3-run1: kit-1-head.jsonl is
        # kit-1.jsonl's lines 4 to 7 (the skipped event and seqs 0 to 2) and
        # its line holding the stop; press-1-head.csv is press-1.csv's header
        # and first three rows.
        folder = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-g3-run1")
        with open(os.path.join(folder, "press-1-head.csv"), encoding="utf-8") as f:
            recorded_log = f.read().splitlines()
        reader = csv.DictReader(recorded_log)
        recorded_rows = list(reader)
        # The schedule is the run's own chart times; a chord's lanes reach
        # only the play message, which no row reads.
        chords = [chord(int(r["index"]), float(r["chart_ms"]),
                        press(int(r["index"]), float(r["chart_ms"]), [Lane.RED]))
                  for r in recorded_rows]
        player = RecordedPlayer(os.path.join(folder, "kit-1-head.jsonl"),
                                seqs=range(len(chords)))
        result = run(chords, player)
        self.assertEqual(result.stop, P.STOP_DONE, result.message)
        scratch = tempfile.TemporaryDirectory()
        self.addCleanup(scratch.cleanup)
        written = os.path.join(scratch.name, "press.csv")
        P.write_press_log(written, result.rows)
        with open(written, encoding="utf-8") as f:
            assert_rows_are_the_runs(self, f.read().splitlines(), recorded_log)

    def test_a_press_event_lacking_a_field_stops_the_run_as_reply(self):
        # No recorded run sent such an event, so it is typed here: a sent
        # event missing any one of K.PRESS_EVENT_FIELDS ends the run as
        # reply, naming the field, and writes no row for it.
        for field in K.PRESS_EVENT_FIELDS:
            chords = [chord(0, 1000.0, press(0, 1000.0, [Lane.RED]))]
            event = sent_event(0, 0, 1001.0, 0, 50)
            del event[field]
            batch = [started(), event, stopped(K.STOP_REASON_DONE)]
            player = FakePlayer({K.PLAY_TYPE: [batch]})
            result = run(chords, player)
            self.assertEqual(result.stop, "reply", field)
            self.assertIn(field, result.message)
            self.assertEqual(result.rows, [], field)

    def test_an_end_read_retimes_the_timed_press_to_the_band_the_read_moves_it_to(self):
        # The agent's side is the g3 run's recorded events of its first three
        # chords (kit-1-head.jsonl); the end read is the end the game stored
        # at that run's first activation (sp_watch-1.csv). The plan's end,
        # planned_end_ms, and the timed press's window, band and own end are
        # hand-picked schedule inputs: a window side_ms either side of its
        # chart time, its own end own_offset_ms into it, and a band from the
        # window's front edge to that end, aimed at its middle. The own end
        # sits far enough inside the window that the read's move keeps it
        # there, so the band really moves.
        planned_end_ms, side_ms, own_offset_ms = 104_800.0, 50.0, 10.0
        (c0, c1, c2) = recorded_chords(recorded_log("press-1-head.csv"))
        _, c0 = repressed(c0, [Lane.RED], planned_end_ms=planned_end_ms)
        other = c1.presses[0]
        chart_ms = c2.presses[0].chart_ms
        window_from, window_to = chart_ms - side_ms, chart_ms + side_ms
        own_end_ms = chart_ms + own_offset_ms
        band = P.end_band(window_from, own_end_ms, False)
        timed, c2 = repressed(
            c2, [Lane.BLUE], end_read_after=c0.index, aim_end_ms=planned_end_ms,
            aim_ms=P.band_middle_ms(band) - chart_ms, band=band,
            window_from_ms=window_from, window_to_ms=window_to, own_end_ms=own_end_ms,
            read_stored_end_ms=planned_end_ms, own_stored_end_ms=own_end_ms,
            conversion={"early": 1.0, "late": 1.0}, read_range_ms=None)
        chords = [c0, c1, c2]
        player = RecordedPlayer(os.path.join(_G3, "kit-1-head.jsonl"), seqs=range(3))
        read_s = watched_sp_end(active=True)
        result = run(chords, player, read_sp_end=lambda: read_s)
        self.assertEqual(result.stop, P.STOP_DONE, result.message)
        read_ms = C.s_to_ms(read_s)
        self.assertEqual(result.rows[0]["end_read_ms"], read_ms)
        (retime,) = [m for m in player.posted if m["type"] == K.RETIME_TYPE]
        self.assertEqual(retime["read_after"], c0.index)
        # Every entry not yet reported goes, each under the seq it had in the
        # play message: a press that already fired, whose event is still
        # unread, is then ignored by the agent, not fired a second time. The
        # untimed press keeps its judge target; the timed one goes where
        # play_chart.retime_from_end, the move's owner, puts it for this read,
        # which is not where the schedule put it.
        (moved,) = P.retime_from_end([timed], c0.index, read_ms)
        self.assertNotEqual(moved.at_ms, timed.at_ms)
        play_seqs = {e["index"]: e["seq"] for e in player.posted[0]["entries"]}
        self.assertEqual([(e["seq"], e["at_ms"]) for e in retime["entries"]],
                         [(play_seqs[other.index], other.at_ms),
                          (play_seqs[timed.index], moved.at_ms)])
        self.assertEqual(retime["entries"][1]["band"], moved.band)

    def test_an_end_read_with_star_power_off_still_posts_a_retime(self):
        # The agent's side is the g3 run's recorded events of its first two
        # chords (kit-1-head.jsonl); the end read is a row of the run's
        # sp_watch-1.csv taken before its first activation.
        (c0, c1, _) = recorded_chords(recorded_log("press-1-head.csv"))
        planned_end_ms = c1.presses[0].chart_ms
        _, c0 = repressed(c0, [Lane.RED], planned_end_ms=planned_end_ms)
        timed, c1 = repressed(c1, [Lane.BLUE], end_read_after=c0.index,
                              aim_end_ms=planned_end_ms)
        chords = [c0, c1]
        player = RecordedPlayer(os.path.join(_G3, "kit-1-head.jsonl"), seqs=range(2))
        read_s = watched_sp_end(active=False)
        result = run(chords, player, read_sp_end=lambda: read_s)
        self.assertEqual(result.stop, P.STOP_DONE, result.message)
        (retime,) = [m for m in player.posted if m["type"] == K.RETIME_TYPE]
        # The timed press goes again under its seq, where the schedule put it.
        self.assertEqual([(e["seq"], e["at_ms"]) for e in retime["entries"]],
                         [(player.posted[0]["entries"][1]["seq"], timed.at_ms)])
        self.assertEqual(result.rows[0]["end_read_ms"], P.NO_ACTIVATION_READ)

    def test_an_event_for_a_seq_no_entry_holds_stops_the_run_as_reply(self):
        # The g3 run's recorded events of its first three chords
        # (kit-1-head.jsonl) played against a schedule of only the first two:
        # the third event names a seq the play message never gave.
        chords = recorded_chords(recorded_log("press-1-head.csv"))[:2]
        player = RecordedPlayer(os.path.join(_G3, "kit-1-head.jsonl"), seqs=range(3))
        unknown = player.recorded[-2]["seq"]
        self.assertNotIn(unknown, range(len(chords)))
        result = run(chords, player)
        self.assertEqual(result.stop, "reply")
        self.assertIn(str(unknown), result.message)
        self.assertEqual(len(result.rows), len(chords))
        self.assertEqual(player.posted[-1], K.stop_play_message())

    def test_each_agent_stop_reason_gives_its_stop_word(self):
        # No recorded run stopped for a reason but done, so the agent's stop
        # is typed here. play_in_game's rule: done is STOP_DONE, and every
        # other reason that ends a run keeps its own word.
        for reason in (K.STOP_REASON_DONE, K.STOP_REASON_STOPPED, K.STOP_REASON_CAP,
                       K.STOP_REASON_FROZEN, K.STOP_REASON_PAST_END):
            chords = [chord(0, 1000.0, press(0, 1000.0, [Lane.RED]))]
            batch = [started(), sent_event(0, 0, 1001.0, 0, 50), stopped(reason)]
            result = run(chords, FakePlayer({K.PLAY_TYPE: [batch]}))
            word = P.STOP_DONE if reason == K.STOP_REASON_DONE else reason
            self.assertEqual(result.stop, word, reason)

    def test_a_split_chords_later_group_names_its_head_and_no_delay(self):
        # The g3 run's chord 3 went down in two order groups. Its schedule is
        # press-1-split.csv (press-1.csv's header and first five rows) and the
        # agent's side kit-1-split.jsonl (kit-1.jsonl's lines 4 to 9, the
        # skipped event and seqs 0 to 4, and line 396, which holds the stop).
        chords = recorded_chords(recorded_log("press-1-split.csv"))
        player = RecordedPlayer(os.path.join(_G3, "kit-1-split.jsonl"), seqs=range(5))
        result = run(chords, player)
        self.assertEqual(result.stop, P.STOP_DONE, result.message)
        entries = player.posted[0]["entries"]
        (split,) = [c for c in chords if len(c.presses) == 2]
        first, later = [e for e in entries if e["index"] == split.index]
        # The later group names its head's seq and carries no gap: the agent
        # sends it on the frame after its head (press_schedule.ts).
        self.assertIsNone(first["after_seq"])
        self.assertEqual(later["after_seq"], first["seq"])
        self.assertEqual(later["at_ms"], split.presses[1].at_ms)
        for entry in (first, later):
            self.assertNotIn("after_ms", entry)

    def test_a_real_split_chords_rows_are_the_rows_that_run_wrote(self):
        # The same g3 files as the test above: a chord played in two order
        # groups gives one row per group, as the run's own press log has them.
        lines = recorded_log("press-1-split.csv")
        chords = recorded_chords(lines)
        player = RecordedPlayer(os.path.join(_G3, "kit-1-split.jsonl"), seqs=range(5))
        result = run(chords, player)
        self.assertEqual(result.stop, P.STOP_DONE, result.message)
        scratch = tempfile.TemporaryDirectory()
        self.addCleanup(scratch.cleanup)
        written = os.path.join(scratch.name, "press.csv")
        P.write_press_log(written, result.rows)
        with open(written, encoding="utf-8") as f:
            assert_rows_are_the_runs(self, f.read().splitlines(), lines)


if __name__ == "__main__":
    unittest.main()
