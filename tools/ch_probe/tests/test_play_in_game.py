"""play_chart.play_in_game against a fake kit agent.

Run from the repo root with:
    py -m pytest -q -p no:cacheprovider tools/ch_probe/tests/test_play_in_game.py
"""

from __future__ import annotations

import io
import os
import queue
import sys
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_mcp import kit_tools as K  # noqa: E402
from tools.ch_probe.experiments import play_chart as P  # noqa: E402
from tools.ch_probe.experiments.overlay_logs import SEND_COLUMN  # noqa: E402
from tools.ch_probe.input_driver import Lane  # noqa: E402
from tools.ch_probe.tests.test_play_chart import _unit_end_fields  # noqa: E402


def press(index, chart_ms, lanes, **kw):
    """A hand-built Press. One timed from an end read gets
    test_play_chart._unit_end_fields of its aim_end_ms for any end field the
    keywords leave out, as test_play_chart._fixture_schedule does."""
    fields = dict(index=index, chart_ms=chart_ms, aim_ms=0.0, lanes=list(lanes),
                  dynamics=["none"] * len(lanes), axes=[1.0] * len(lanes), declined=False,
                  last_send_ms=chart_ms + 50.0, rule="plain")
    fields.update(kw)
    if fields.get("end_read_after") is not None:
        for key, value in _unit_end_fields(fields["aim_end_ms"]).items():
            fields.setdefault(key, value)
    return P.Press(**fields)


def chord(index, ms, *presses):
    lanes = [l for p in presses for l in p.lanes]
    return P.Chord(index, index * 100, ms, lanes, ["none"] * len(lanes), presses=presses)


def sent_event(seq, chord_index, clock_ms, score_before, score_after, note=""):
    return {"kind": K.EVENT_SENT, "seq": seq, "index": chord_index,
            "clock_ms_at_press": clock_ms, "down_frame": 10 + seq,
            "up_frame": 12 + seq, "down_ms": 1_000_000.0 + seq, "up_ms": 1_000_006.0 + seq,
            "score_before": score_before, "score_after": score_after,
            "frame_gap_ms": 3.0, "agent_note": note}


def dropped_event(seq, chord_index, clock_ms, score):
    return {"kind": K.EVENT_DROPPED, "seq": seq, "index": chord_index,
            "clock_ms_at_press": clock_ms, "down_frame": None,
            "up_frame": None, "down_ms": None, "up_ms": None, "score_before": score,
            "score_after": score, "frame_gap_ms": 900.0, "agent_note": ""}


def started(seqs=()):
    return {"kind": K.EVENT_SKIPPED, "seqs": list(seqs), "count": len(seqs),
            "clock_ms": 0.0}


def stopped(reason):
    return {"kind": K.EVENT_STOP, "reason": reason, "frame": 1, "clock_ms": 0.0,
            "wall_ms": 0.0}


class FakePlayer:
    """kit_tools.GamePlayer's three parts. `script` maps a posted message's
    type to the events to queue after it, each list used once in order.
    `refuse` maps a message type to the error its post raises, as
    GamePlayer.post raises the agent's refusal."""

    def __init__(self, script, late=None, refuse=None):
        self.events = queue.Queue()
        self.posted = []
        self.script = {k: list(v) for k, v in script.items()}
        self._late = late
        self._refuse = refuse or {}

    def alive(self):
        return True

    def post(self, message):
        self.posted.append(message)
        batches = self.script.get(message["type"]) or []
        if batches:
            for event in batches.pop(0):
                if self._late is not None:
                    self._late()
                self.events.put(event)
        if message["type"] in self._refuse:
            raise self._refuse[message["type"]]
        return {"ok": True}


def run(chords, player, **kw):
    return P.play_in_game(chords, player, clock_addr=0x100, score_addr=0x200,
                          out=io.StringIO(), **kw)


class PlayInGameTest(unittest.TestCase):
    def test_the_play_message_carries_the_schedule_and_every_number(self):
        chords = [chord(0, 1000.0, press(0, 1000.0, [Lane.RED]))]
        player = FakePlayer({K.PLAY_TYPE: [[started(), sent_event(0, 0, 1001.0, 0, 50),
                                            stopped(K.STOP_REASON_DONE)]]})
        run(chords, player, cap_s=30.0)
        (message,) = player.posted
        self.assertEqual(message["type"], K.PLAY_TYPE)
        self.assertEqual((message["clock_addr"], message["score_addr"], message["cap_ms"],
                          message["score_delay_ms"], message["start_lead_ms"],
                          message["hold_frames"], message["stall_ms"],
                          message["jump_back_ms"]),
                         (0x100, 0x200, 30000.0, 5.0, 150.0, 2, 5000.0, 1000.0))
        self.assertEqual(message["entries"], [
            {"seq": 0, "index": 0, "at_ms": 1000.0, "last_send_ms": 1050.0,
             "elements": [{"id": 0, "value": 1.0}], "after_ms": None, "after_seq": None,
             "end_read_after": None}])

    def test_a_sent_row_and_a_dropped_row_fill_every_column_score_compare_reads(self):
        chords = [chord(0, 1000.0, press(0, 1000.0, [Lane.RED])),
                  chord(1, 1100.0, press(1, 1100.0, [Lane.BLUE]))]
        player = FakePlayer({K.PLAY_TYPE: [[started(), sent_event(0, 0, 1001.0, 0, 50),
                                            dropped_event(1, 1, 1200.0, 50),
                                            stopped(K.STOP_REASON_DONE)]]})
        result = run(chords, player)
        self.assertEqual(result.stop, P.STOP_DONE)
        self.assertEqual((result.sent, result.dropped), (1, 1))
        for row in result.rows:
            for column in ("index", "chart_ms", "clock_ms_at_press", "score_before",
                           "score_after", SEND_COLUMN):
                self.assertIsNotNone(row.get(column), column)
        self.assertEqual([r[SEND_COLUMN] for r in result.rows], [P.SENT, P.DROPPED])
        self.assertEqual(result.rows[1]["score_after"], 50)

    def test_a_sent_row_carries_frame_gap_ms_and_a_missed_retime_carries_agent_note(self):
        chords = [chord(0, 1000.0, press(0, 1000.0, [Lane.RED])),
                  chord(1, 1100.0, press(1, 1100.0, [Lane.BLUE], end_read_after=0,
                                         aim_end_ms=1500.0))]
        player = FakePlayer({K.PLAY_TYPE: [[
            started(), sent_event(0, 0, 1001.0, 0, 50),
            sent_event(1, 1, 1101.0, 50, 100, note=K.NOTE_RETIME_MISSED),
            stopped(K.STOP_REASON_DONE)]]})
        result = run(chords, player)
        self.assertEqual(result.rows[0]["frame_gap_ms"], 3.0)
        self.assertEqual(result.rows[0]["agent_note"], "")
        self.assertEqual(result.rows[1]["agent_note"], K.NOTE_RETIME_MISSED)
        self.assertIn(K.NOTE_RETIME_MISSED, result.message)
        self.assertIn("chords 1", result.message)

    def test_an_end_read_posts_one_retime_of_retime_from_ends_output(self):
        head = press(0, 1000.0, [Lane.RED], planned_end_ms=1500.0, seq=0)
        other = press(1, 1500.0, [Lane.RED], seq=1)
        timed = press(2, 2000.0, [Lane.BLUE], end_read_after=0, aim_end_ms=1500.0, seq=2)
        chords = [chord(0, 1000.0, head), chord(1, 1500.0, other), chord(2, 2000.0, timed)]
        player = FakePlayer({
            K.PLAY_TYPE: [[started(), sent_event(0, 0, 1001.0, 0, 50)]],
            K.RETIME_TYPE: [[sent_event(1, 1, 1501.0, 50, 100),
                             sent_event(2, 2, 2021.0, 100, 150),
                             stopped(K.STOP_REASON_DONE)]]})
        result = run(chords, player, read_sp_end=lambda: 1.52)
        retimes = [m for m in player.posted if m["type"] == K.RETIME_TYPE]
        self.assertEqual(len(retimes), 1)
        # Every entry not yet reported goes, the moved one under the seq it
        # had in the play message: a press that already fired, whose event is
        # still unread, is then ignored by the agent, not fired a second time.
        kept, moved = P.retime_from_end([other, timed], 0, 1520.0)
        self.assertIs(kept, other)
        self.assertEqual(moved.seq, 2)
        self.assertEqual(retimes[0], K.retime_message(
            0, [K.play_entry(1, kept, None), K.play_entry(2, moved, None)]))
        self.assertEqual(retimes[0]["entries"][1]["at_ms"], 2020.0)
        self.assertEqual([e["seq"] for e in player.posted[0]["entries"]], [0, 1, 2])
        self.assertEqual(result.rows[0]["end_read_ms"], 1520.0)
        self.assertEqual(result.stop, P.STOP_DONE)

    def test_a_later_group_in_a_retime_names_its_heads_seq(self):
        head = press(0, 1000.0, [Lane.RED], planned_end_ms=1500.0)
        timed = press(1, 2000.0, [Lane.BLUE], end_read_after=0, aim_end_ms=1500.0)
        later = press(1, 2000.0, [Lane.GREEN], end_read_after=0, aim_end_ms=1500.0,
                      aim_ms=5.0, after_ms=5.0)
        chords = [chord(0, 1000.0, head), chord(1, 2000.0, timed, later)]
        player = FakePlayer({
            K.PLAY_TYPE: [[started(), sent_event(0, 0, 1001.0, 0, 50)]],
            K.RETIME_TYPE: [[sent_event(1, 1, 2021.0, 50, 100),
                             sent_event(2, 1, 2026.0, 50, 100),
                             stopped(K.STOP_REASON_DONE)]]})
        result = run(chords, player, read_sp_end=lambda: 1.52)
        (retime,) = [m for m in player.posted if m["type"] == K.RETIME_TYPE]
        self.assertEqual([(e["seq"], e["after_seq"], e["after_ms"]) for e in retime["entries"]],
                         [(1, None, None), (2, 1, 5.0)])
        self.assertEqual(result.stop, P.STOP_DONE, result.message)

    def test_an_end_read_with_star_power_off_still_posts_a_retime(self):
        head = press(0, 1000.0, [Lane.RED], planned_end_ms=1500.0)
        timed = press(1, 2000.0, [Lane.BLUE], end_read_after=0, aim_end_ms=1500.0)
        chords = [chord(0, 1000.0, head), chord(1, 2000.0, timed)]
        player = FakePlayer({
            K.PLAY_TYPE: [[started(), sent_event(0, 0, 1001.0, 0, 50)]],
            K.RETIME_TYPE: [[sent_event(1, 1, 2001.0, 50, 100),
                             stopped(K.STOP_REASON_DONE)]]})
        result = run(chords, player, read_sp_end=lambda: None)
        retimes = [m for m in player.posted if m["type"] == K.RETIME_TYPE]
        self.assertEqual(retimes, [K.retime_message(
            0, [K.play_entry(1, P.replace(timed, seq=1), None)])])
        self.assertEqual(result.rows[0]["end_read_ms"], P.NO_ACTIVATION_READ)

    def test_a_retime_refused_after_the_play_ended_keeps_every_row_and_the_stop_word(self):
        # The end read after chord 0 was slow, so the agent played every
        # press and sent its stop before the retime reached it.
        head = press(0, 1000.0, [Lane.RED], planned_end_ms=1500.0)
        other = press(1, 1500.0, [Lane.RED])
        timed = press(2, 2000.0, [Lane.BLUE], end_read_after=0, aim_end_ms=1500.0)
        chords = [chord(0, 1000.0, head), chord(1, 1500.0, other), chord(2, 2000.0, timed)]
        error = "retime failed: Error: retime: no play is running"
        refusal = FA.ReadFailed(FA.END_AGENT_ERROR, f"the agent sent an error: {error}",
                                {"error": error})
        player = FakePlayer({K.PLAY_TYPE: [[
            started(), sent_event(0, 0, 1001.0, 0, 50), sent_event(1, 1, 1501.0, 50, 100),
            sent_event(2, 2, 2001.0, 100, 150), stopped(K.STOP_REASON_DONE)]]},
            refuse={K.RETIME_TYPE: refusal})
        rows = []
        result = run(chords, player, read_sp_end=lambda: 1.52, on_press=rows.append)
        self.assertEqual(result.stop, P.STOP_DONE, result.message)
        self.assertEqual([r["index"] for r in result.rows], [0, 1, 2])
        self.assertEqual(rows, result.rows)
        self.assertIn("retime after chord 0's end read was refused", result.message)
        self.assertIn("play had already ended", result.message)
        self.assertIn("retime: no play is running", result.message)
        self.assertNotIn(K.stop_play_message(), player.posted)

    def test_a_retime_refused_while_the_play_runs_stops_the_run_as_reply(self):
        head = press(0, 1000.0, [Lane.RED], planned_end_ms=1500.0)
        timed = press(1, 2000.0, [Lane.BLUE], end_read_after=0, aim_end_ms=1500.0)
        chords = [chord(0, 1000.0, head), chord(1, 2000.0, timed)]
        error = "retime failed: Error: an entry lacks its elements"
        refusal = FA.ReadFailed(FA.END_AGENT_ERROR, f"the agent sent an error: {error}",
                                {"error": error})
        player = FakePlayer({K.PLAY_TYPE: [[started(), sent_event(0, 0, 1001.0, 0, 50)]]},
                            refuse={K.RETIME_TYPE: refusal})
        result = run(chords, player, read_sp_end=lambda: 1.52)
        self.assertEqual(result.stop, "reply")
        self.assertIn("an entry lacks its elements", result.message)
        self.assertEqual([r["index"] for r in result.rows], [0])
        self.assertEqual(player.posted[-1], K.stop_play_message())

    def test_a_retime_post_with_no_reply_stops_the_run_as_reply(self):
        # Only the agent's own refusal can mean the play ended; a post that
        # got no reply stops the run even with the agent's stop queued.
        head = press(0, 1000.0, [Lane.RED], planned_end_ms=1500.0)
        timed = press(1, 2000.0, [Lane.BLUE], end_read_after=0, aim_end_ms=1500.0)
        chords = [chord(0, 1000.0, head), chord(1, 2000.0, timed)]
        player = FakePlayer({K.PLAY_TYPE: [[
            started(), sent_event(0, 0, 1001.0, 0, 50), sent_event(1, 1, 2001.0, 50, 100),
            stopped(K.STOP_REASON_DONE)]]},
            refuse={K.RETIME_TYPE: FA.ReadFailed(FA.END_CAP, "no reply to retime")})
        result = run(chords, player, read_sp_end=lambda: 1.52)
        self.assertEqual(result.stop, "reply")
        self.assertIn("no reply to retime", result.message)

    def test_an_unknown_seq_stops_the_run_as_reply(self):
        chords = [chord(0, 1000.0, press(0, 1000.0, [Lane.RED]))]
        player = FakePlayer({K.PLAY_TYPE: [[started(), sent_event(7, 0, 1001.0, 0, 50)]]})
        result = run(chords, player)
        self.assertEqual(result.stop, "reply")
        self.assertIn("seq 7", result.message)
        self.assertEqual(player.posted[-1], K.stop_play_message())

    def test_each_agent_stop_reason_gives_its_stop_word(self):
        words = {K.STOP_REASON_DONE: P.STOP_DONE, K.STOP_REASON_STOPPED: "stopped",
                 K.STOP_REASON_CAP: "cap", K.STOP_REASON_FROZEN: "frozen",
                 K.STOP_REASON_PAST_END: "past_end"}
        for reason, word in words.items():
            chords = [chord(0, 1000.0, press(0, 1000.0, [Lane.RED]))]
            batch = [started(), sent_event(0, 0, 1001.0, 0, 50), stopped(reason)]
            result = run(chords, FakePlayer({K.PLAY_TYPE: [batch]}))
            self.assertEqual(result.stop, word, reason)

    def test_a_jump_back_sends_the_plan_again_and_starts_the_counts_over(self):
        chords = [chord(0, 1000.0, press(0, 1000.0, [Lane.RED]))]
        player = FakePlayer({K.PLAY_TYPE: [
            [started(), sent_event(0, 0, 1001.0, 0, 50), stopped(K.STOP_REASON_JUMPED_BACK)],
            [started(), sent_event(0, 0, 1001.0, 0, 50), stopped(K.STOP_REASON_DONE)]]})
        result = run(chords, player)
        self.assertEqual([m["type"] for m in player.posted], [K.PLAY_TYPE, K.PLAY_TYPE])
        self.assertEqual(player.posted[0]["entries"], player.posted[1]["entries"])
        self.assertEqual((result.stop, result.sent, len(result.rows)), (P.STOP_DONE, 1, 1))

    def test_an_unknown_stop_reason_stops_the_run_as_reply(self):
        chords = [chord(0, 1000.0, press(0, 1000.0, [Lane.RED]))]
        result = run(chords, FakePlayer({K.PLAY_TYPE: [[started(), stopped("melted")]]}))
        self.assertEqual(result.stop, "reply")

    def test_events_that_arrive_900_ms_late_still_give_every_row_in_order(self):
        # The fake blocks 900 ms of a fake clock before each event; the
        # agent fired every press on time, so no row may say dropped.
        clock = [0.0]

        def late():
            clock[0] += 0.9

        chords = [chord(k, 1000.0 + 100 * k, press(k, 1000.0 + 100 * k, [Lane.RED]))
                  for k in range(4)]
        batch = [started()] + [sent_event(k, k, 1001.0 + 100 * k, 50 * k, 50 * k + 50)
                               for k in range(4)] + [stopped(K.STOP_REASON_DONE)]
        result = run(chords, FakePlayer({K.PLAY_TYPE: [batch]}, late=late),
                     now=lambda: clock[0])
        self.assertEqual([r["index"] for r in result.rows], [0, 1, 2, 3])
        self.assertEqual(result.dropped, 0)
        self.assertEqual(result.stop, P.STOP_DONE)

    def test_a_stop_events_cause_and_lift_error_are_in_the_message(self):
        chords = [chord(0, 1000.0, press(0, 1000.0, [Lane.RED]))]
        stop = dict(stopped(K.STOP_REASON_STOPPED), cause="no frame came for 5000 ms",
                    lift_error="the pad write failed")
        result = run(chords, FakePlayer({K.PLAY_TYPE: [[started(), stop]]}))
        self.assertEqual(result.stop, "stopped")
        self.assertIn("no frame came for 5000 ms", result.message)
        self.assertIn("the pad write failed", result.message)

    def test_a_done_that_carries_a_cause_is_not_a_clean_done(self):
        chords = [chord(0, 1000.0, press(0, 1000.0, [Lane.RED]))]
        stop = dict(stopped(K.STOP_REASON_DONE), cause="a kit write failed: boom")
        player = FakePlayer({K.PLAY_TYPE: [[started(), sent_event(0, 0, 1001.0, 0, 50), stop]]})
        result = run(chords, player)
        self.assertEqual(result.stop, "reply")
        self.assertIn("a kit write failed: boom", result.message)

    def test_an_error_the_agent_reports_stops_the_run_as_reply(self):
        chords = [chord(0, 1000.0, press(0, 1000.0, [Lane.RED]))]
        error = {"kind": K.EVENT_ERROR, "error": "the stall check failed: boom"}
        player = FakePlayer({K.PLAY_TYPE: [[started(), error]]})
        result = run(chords, player)
        self.assertEqual(result.stop, "reply")
        self.assertIn("the stall check failed: boom", result.message)
        self.assertEqual(player.posted[-1], K.stop_play_message())

    def test_a_split_chords_rows_match_play_chords_rows_but_the_timing_columns(self):
        first = press(0, 1000.0, [Lane.RED])
        later = press(0, 1000.0, [Lane.BLUE], aim_ms=5.0, after_ms=5.0)
        chords = [chord(0, 1000.0, first, later)]
        player = FakePlayer({K.PLAY_TYPE: [[
            started(), sent_event(0, 0, 1001.0, 0, 50), sent_event(1, 0, 1006.0, 0, 50),
            stopped(K.STOP_REASON_DONE)]]})
        rows = run(chords, player).rows
        self.assertEqual(player.posted[0]["entries"][1]["after_seq"], 0)
        self.assertEqual(player.posted[0]["entries"][1]["after_ms"], 5.0)

        class Clock:
            t = 0.0

            def read(self):
                Clock.t += 0.0005
                # Starts more than the start lead before the chord.
                return (0.8 + Clock.t, 0.8 + Clock.t)

        def kit_press(lanes, axes, then=()):
            return {"down_frame": 10, "up_frame": 12, "down_ms": 1.0, "up_ms": 2.0,
                    "groups": [{"down_frame": 10, "up_frame": 12, "down_ms": 1.0,
                                "up_ms": 2.0},
                               {"down_frame": 11, "up_frame": 13, "down_ms": 1.5,
                                "up_ms": 2.5}]}

        scores = iter([0, 0, 50])
        old_run = P.play_chords(chords, Clock(), lambda: next(scores), kit_press,
                                sleep=lambda s: None, out=io.StringIO(), kit=True)
        self.assertEqual(old_run.stop, P.STOP_DONE, old_run.message)
        old = old_run.rows
        compared = ("index", "chart_ms", "lead_ms", "score_before", "score_after",
                    SEND_COLUMN, "scored_between", "retimed", "end_read_ms")
        self.assertEqual([{c: r.get(c) for c in compared} for r in rows],
                         [{c: r.get(c) for c in compared} for r in old])


if __name__ == "__main__":
    unittest.main()
