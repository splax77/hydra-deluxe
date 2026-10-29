"""Tests for trace_hits: the once-per-note filter, the line-up with the
manifest, the 85 ms boundary, the settings the agent receives, the no-writes
check over trace.ts, the two hook RVAs, where a trace's notes come from, and
the background TraceSession.

No game and no Frida: the fake frida module and the fake wait come from
test_frida_attach, so both test files drive run_agent the same way.
"""

from __future__ import annotations

import json
import os
import sys
import tempfile
import threading
import unittest
import unittest.mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import trace_hits as TH  # noqa: E402
from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_mcp.tests.test_frida_attach import (  # noqa: E402
    AGENT_DIR, FakeFrida, FakeSession, instant_wait, no_write_findings, sent)
from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe.probe_songs import WINDOW_MAP  # noqa: E402

ENGINE = {"name": "Eng", "image": "StrikeCore", "parent": "Base<Inputs>",
          "shape": {"sealed": True, "generic_base": True,
                    "hit_time_double": True, "ctor_rva": True}}


def entry(names, count, owner="engine", rva="0x1000"):
    return {"names": names, "owner": owner, "rva": rva, "params": ["uint"],
            "count": count, "args": []}


def notes_at(*times_ms):
    return [{"index": i, "time_ms": t} for i, t in enumerate(times_ms)]


def call(name, song_time_s, args=()):
    return {"call": name, "names": [name], "songTime": song_time_s,
            "args": list(args)}


class OncePerNoteTest(unittest.TestCase):
    def test_picks_the_exact_count_and_the_off_by_one(self):
        table = [entry(["A"], 142), entry(["B", "B2"], 142),
                 entry(["C"], 141), entry(["D"], 143), entry(["E"], 140),
                 entry(["F"], 144), entry(["G"], 0), entry(["H"], 9000)]
        exact, off = TH.once_per_note(table, 142)
        self.assertEqual([e["names"] for e in exact], [["A"], ["B", "B2"]])
        self.assertEqual([e["names"] for e in off], [["C"], ["D"]])

    def test_no_match_gives_two_empty_lists(self):
        self.assertEqual(TH.once_per_note([entry(["A"], 7)], 120), ([], []))


class LineUpTest(unittest.TestCase):
    NOTES = notes_at(1000, 1100, 1300, 2000, 2500)

    def test_each_call_gets_its_nearest_note_and_the_difference(self):
        calls = [call("Hit", 0.99), call("Hit", 1.12), call("Hit", 1.25),
                 call("Hit", 2.0), call("Hit", 3.0)]
        lined = TH.line_up(calls, self.NOTES)
        self.assertEqual([x.note_index for x in lined], [0, 1, 2, 3, 4])
        self.assertEqual([x.note_ms for x in lined],
                         [1000, 1100, 1300, 2000, 2500])
        for got, want in zip([x.diff_ms for x in lined],
                             [-10.0, 20.0, -50.0, 0.0, 500.0]):
            self.assertAlmostEqual(got, want, places=6)

    def test_a_tie_goes_to_the_earlier_note(self):
        lined = TH.line_up([call("Hit", 1.05)], self.NOTES)
        self.assertEqual(lined[0].note_index, 0)

    def test_the_85_ms_boundary_counts_and_just_past_it_does_not(self):
        limit = C.EXPECT_NORMAL_BACK_MS
        exactly = TH.line_up([call("Hit", 1.0)], notes_at(915))
        before = TH.line_up([call("Hit", 1.0)], notes_at(1085))
        past = TH.line_up([call("Hit", 1.0)], notes_at(914.9))
        self.assertEqual(exactly[0].diff_ms, 85.0)
        self.assertEqual(before[0].diff_ms, -85.0)
        self.assertEqual(TH.count_within(exactly, limit), 1)
        self.assertEqual(TH.count_within(before, limit), 1)
        self.assertEqual(TH.count_within(past, limit), 0)


class CapTest(unittest.TestCase):
    # Window Map's last note is at 59,080 ms (plan 2026-10-28, question 5).
    WINDOW_MAP = notes_at(3000, 59080)

    def test_the_song_cap_is_the_song_length_plus_the_margin(self):
        self.assertAlmostEqual(TH.trace_cap_s(self.WINDOW_MAP), 92.08)

    def test_the_wall_cap_is_the_song_cap_plus_the_pause_room(self):
        self.assertAlmostEqual(TH.wall_cap_s(self.WINDOW_MAP), 212.08)


class CommandLineTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.song = os.path.join(self.tmp.name, "Song")
        os.mkdir(self.song)
        self.manifest = os.path.join(self.song, "manifest.json")
        with open(self.manifest, "w", encoding="utf-8") as fh:
            json.dump({"song": "Song", "notes": notes_at(1000, 2000, 3000)}, fh)
        self.agent = os.path.join(self.tmp.name, "trace.js")
        with open(self.agent, "w", encoding="utf-8") as fh:
            fh.write("// fake trace\n")
        self.out_path = os.path.join(self.tmp.name, "trace.jsonl")
        self.lines = []

    def run_main(self, extra, session):
        return TH.main(["--song", self.song, "--engine-class", "Eng",
                        "--agent", self.agent, "--out", self.out_path] + extra,
                       frida_mod=FakeFrida(session), wait=instant_wait,
                       out=self.lines.append)

    def test_count_mode_posts_the_settings_and_prints_the_table(self):
        table = {"done": "song over", "mode": "count", "calls": 9,
                 "not_hooked": [], "no_code": ["base Abs"],
                 "methods": [entry(["Tick"], 6, rva="0x2000"),
                             entry(["Hit", "Hit2"], 3, owner="base")]}
        session = FakeSession([sent({"engine": ENGINE, "hooked": 2,
                                     "no_code": ["base Abs"]}), sent(table)])
        code = self.run_main(["--mode", "count"], session)
        self.assertEqual(code, 0)
        # Last note 3,000 ms + SILENCE_MS 3,000 ms + 30 s margin = 36 s song
        # cap; plus 120 s of pause room = 156 s wall-clock cap.
        self.assertEqual(session.script.posted, [{
            "type": "start", "cap_s": 156.0, "song_cap_s": 36.0,
            "mode": "count", "engine_class": "Eng",
            "engine_ctor_rva": C.RVA_DRUMS_ENGINE_CTOR,
            "hit_time_offset": C.OFF_HIT_TIME, "methods": []}])
        self.assertEqual(session.detach_calls, 1)
        self.assertEqual(self.lines, [
            "ended: done (agent: song over)",
            "engine: Eng in StrikeCore, base Base<Inputs>",
            "shape: sealed yes, generic_base yes, hit_time_double yes, "
            "ctor_rva yes",
            f"notes in {self.manifest}: 3",
            "hooked: 2 code addresses, 9 calls; no code: 1 methods; "
            "not hooked: 0",
            "once per note (count 3): Hit / Hit2 (base 0x1000)",
            "off by one (count 2 or 4): none",
            "all methods by count:",
            "        6  Tick (engine 0x2000)  (uint)",
            "        3  Hit / Hit2 (base 0x1000)  (uint)",
            f"messages written to {self.out_path}",
        ])

    def test_args_mode_lines_calls_up_with_the_notes(self):
        session = FakeSession([
            sent({"engine": ENGINE, "hooked": 1, "no_code": []}),
            sent(call("Hit", 1.0, [7])), sent(call("Hit", 2.1, [8])),
            sent({"done": "song over", "mode": "args", "calls": 2,
                  "not_hooked": []})])
        code = self.run_main(["--mode", "args", "--method", "Hit",
                              "--engine-ctor-rva", "0x1234"], session)
        self.assertEqual(code, 0)
        posted = session.script.posted[0]
        self.assertEqual(posted["methods"], ["Hit"])
        self.assertEqual(posted["engine_ctor_rva"], 0x1234)
        self.assertEqual(self.lines[4:], [
            "calls: 2",
            "  call 1: Hit at 1000.0 ms, args [7]; nearest note 0 at 1000 ms, "
            "diff +0.0 ms",
            "  call 2: Hit at 2100.0 ms, args [8]; nearest note 1 at 2000 ms, "
            "diff +100.0 ms",
            "calls within 85 ms of their nearest note (EXPECT_NORMAL_BACK_MS): "
            "1 of 2",
            f"messages written to {self.out_path}",
        ])

    def test_a_trace_ended_by_the_runner_cap_exits_0_with_no_error(self):
        # Window Map's last note: 59,080 ms + SILENCE_MS 3,000 ms + 30 s
        # margin = 92.08 s song cap; plus 120 s of pause room = 212.08 s.
        with open(self.manifest, "w", encoding="utf-8") as fh:
            json.dump({"song": WINDOW_MAP, "notes": notes_at(3000, 59080)},
                      fh)
        session = FakeSession([sent({"engine": ENGINE, "hooked": 2,
                                     "no_code": []})])
        code = self.run_main(["--mode", "count"], session)
        self.assertEqual(code, 0)
        posted = session.script.posted[0]
        self.assertAlmostEqual(posted["cap_s"], 212.08)
        self.assertAlmostEqual(posted["song_cap_s"], 92.08)
        self.assertEqual(session.detach_calls, 1)
        self.assertEqual(self.lines[:2], [
            "cap of 212.08 s reached before the agent finished; 1 messages seen",
            "ended: cap"])
        self.assertIn("table: none received", self.lines)
        self.assertFalse(any(line.startswith("error") for line in self.lines))

    def test_an_agent_error_is_printed_and_exits_1(self):
        session = FakeSession([sent({"error": "engine shape check failed: "
                                              "ctor_rva", "engine": ENGINE})])
        code = self.run_main(["--mode", "count"], session)
        self.assertEqual(code, 1)
        self.assertIn("table: none received", self.lines)
        self.assertIn("error: engine shape check failed: ctor_rva", self.lines)
        self.assertEqual(session.detach_calls, 1)

    def test_not_running_says_so_and_stops(self):
        code = self.run_main(["--mode", "count"], None)
        self.assertEqual(code, 1)
        self.assertEqual(self.lines,
                         ["Clone Hero.exe is not running; nothing attached"])

    def test_a_missing_agent_says_how_to_build_it(self):
        os.remove(self.agent)
        code = self.run_main(["--mode", "count"], FakeSession([]))
        self.assertEqual(code, 1)
        self.assertEqual(self.lines, [
            f"{self.agent} not found; run npm run build:trace in "
            "tools/ch_mcp/agent first"])

    def test_the_modes_refuse_the_wrong_method_flags(self):
        for extra in (["--mode", "args"], ["--mode", "count", "--method", "X"]):
            with self.subTest(extra=extra):
                with self.assertRaises(SystemExit):
                    with unittest.mock.patch("sys.stderr"):
                        self.run_main(extra, FakeSession([]))


class TraceAgentTest(unittest.TestCase):
    def test_trace_ts_writes_nothing_to_the_game(self):
        with open(os.path.join(AGENT_DIR, "trace.ts"), encoding="utf-8") as fh:
            self.assertEqual(no_write_findings(fh.read()), [])

    def test_only_gamemanager_ts_looks_up_the_game_manager(self):
        # One owner: the agents call findGameManager; none repeats the lookup.
        for name in ("clock.ts", "trace.ts"):
            with self.subTest(agent=name):
                with open(os.path.join(AGENT_DIR, name), encoding="utf-8") as fh:
                    text = fh.read()
                self.assertIn("findGameManager()", text)
                self.assertNotIn("gc.choose", text)
                self.assertNotIn("tryAssembly", text)

    def test_ended_line_names_the_song_cap(self):
        result = FA.RunResult(reason=FA.END_DONE, cap_s=212.08,
                              payloads=[{"done": "song cap", "methods": []}])
        self.assertEqual(TH.ended_line(result), "ended: done (agent: song cap)")

    def test_the_end_reasons_are_the_runners(self):
        # trace.ts ends with a `done` payload, which ends_run reads.
        self.assertEqual(FA.ends_run({"done": "song over", "methods": []}),
                         FA.END_DONE)


class HookRvaTest(unittest.TestCase):
    # The literals are the study's: docs/research/
    # 2026-10-28-clone-hero-mcp-feasibility.md, "Test 3: the hit method".
    def test_the_two_engine_methods_are_the_studys_addresses(self):
        self.assertEqual(C.RVA_NOTE_SCORE_ADD, 0x20DEEC0)
        self.assertEqual(C.RVA_NOTE_DYNAMICS, 0x20DE770)

    def test_the_default_hooks_only_the_score_method(self):
        self.assertEqual(C.PER_NOTE_DEFAULT_RVAS, (C.RVA_NOTE_SCORE_ADD,))


def write_json(path, data):
    with open(path, "w", encoding="utf-8") as fh:
        json.dump(data, fh)


def score_chord(index, ms):
    return {"index": index, "ms": ms,
            "notes": [{"color": "Red", "cymbal": False}]}


class SongNotesTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.song = os.path.join(self.tmp.name, "Song")
        os.mkdir(self.song)
        self.score = os.path.join(self.tmp.name, "score.json")
        write_json(self.score, {"chords": [score_chord(0, 1000.0),
                                           score_chord(1, 1250.5),
                                           score_chord(2, 2000.0)]})

    def test_a_score_json_becomes_notes_in_the_manifest_shape(self):
        self.assertEqual(TH.notes_from_score(self.score), [
            {"index": 0, "time_ms": 1000.0},
            {"index": 1, "time_ms": 1250.5},
            {"index": 2, "time_ms": 2000.0}])

    def test_the_manifest_wins_when_the_folder_has_one(self):
        manifest = os.path.join(self.song, "manifest.json")
        write_json(manifest, {"song": "Song", "notes": notes_at(500, 900)})
        self.assertEqual(TH.song_notes(self.song, self.score),
                         (manifest, notes_at(500, 900)))

    def test_the_score_json_is_used_when_there_is_no_manifest(self):
        path, notes = TH.song_notes(self.song, self.score)
        self.assertEqual(path, self.score)
        self.assertEqual([n["time_ms"] for n in notes], [1000.0, 1250.5, 2000.0])

    def test_no_source_names_both(self):
        missing = os.path.join(self.tmp.name, "nope.json")
        manifest = os.path.join(self.song, "manifest.json")
        with self.assertRaises(TH.NotesMissing) as given:
            TH.song_notes(self.song, missing)
        self.assertEqual(str(given.exception),
                         "no notes to line the calls up with: no manifest at "
                         f"{manifest}, and no score JSON at {missing}")
        with self.assertRaises(TH.NotesMissing) as none:
            TH.song_notes(self.song)
        self.assertEqual(str(none.exception),
                         "no notes to line the calls up with: no manifest at "
                         f"{manifest}, and no score JSON was given")


# How long a test waits for the session's thread. Only a hang reaches it.
THREAD_CAP_S = 5.0


class TraceSessionTest(unittest.TestCase):
    NOTES = notes_at(1000, 2000, 3000)

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.agent = os.path.join(self.tmp.name, "trace.js")
        with open(self.agent, "w", encoding="utf-8") as fh:
            fh.write("// fake trace\n")
        self.out_path = os.path.join(self.tmp.name, "trace.jsonl")
        self.in_wait = threading.Event()

    def blocking_wait(self, event, timeout_s):
        """Sleeps on the session's stop event like the real wait, and says
        the session has reached its wait."""
        self.in_wait.set()
        return event.wait(timeout_s)

    def session(self, fake_session, **kw):
        kw.setdefault("wait", self.blocking_wait)
        s = TH.TraceSession(self.NOTES, "Eng", ["Hit"], self.out_path,
                            agent_path=self.agent,
                            frida_mod=FakeFrida(fake_session), **kw)
        self.addCleanup(s.stop, THREAD_CAP_S)
        return s.start()

    def deliver(self, fake_session, payload):
        fake_session.script.handlers["message"](sent(payload), None)

    def test_the_agent_gets_mains_args_mode_settings(self):
        fake = FakeSession([sent({"done": "song over", "calls": 0})])
        s = self.session(fake)
        self.assertTrue(s.stop(THREAD_CAP_S))
        # Last note 3,000 ms + SILENCE_MS 3,000 ms + 30 s margin = 36 s song
        # cap; plus 120 s of pause room = 156 s wall-clock cap.
        self.assertEqual(fake.script.posted, [{
            "type": "start", "cap_s": 156.0, "song_cap_s": 36.0,
            "mode": "args", "engine_class": "Eng",
            "engine_ctor_rva": C.RVA_DRUMS_ENGINE_CTOR,
            "hit_time_offset": C.OFF_HIT_TIME, "methods": ["Hit"]}])

    def test_stop_ends_a_run_that_never_ends_and_detaches_once(self):
        fake = FakeSession([sent({"engine": ENGINE, "hooked": 1,
                                  "no_code": []})])
        s = self.session(fake)
        self.assertTrue(self.in_wait.wait(THREAD_CAP_S))
        self.assertFalse(s.ended())
        self.assertEqual(s.ended_line(),
                         "ended: not yet; the trace is still running")
        self.assertTrue(s.stop(THREAD_CAP_S))
        self.assertEqual(fake.detach_calls, 1)
        self.assertEqual(s.result.reason, TH.END_STOPPED)
        self.assertIsNone(s.result.error)
        self.assertEqual(s.ended_line(), "ended: stopped")
        self.assertEqual(s.lines, [])

    def test_calls_grow_as_the_agent_sends_them(self):
        fake = FakeSession([sent({"engine": ENGINE, "hooked": 1,
                                  "no_code": []})])
        s = self.session(fake)
        self.assertTrue(self.in_wait.wait(THREAD_CAP_S))
        self.assertEqual(s.calls(), [])
        self.deliver(fake, call("Hit", 1.0, [7]))
        self.assertEqual(s.calls(), [call("Hit", 1.0, [7])])
        self.deliver(fake, call("Hit", 2.1, [8]))
        self.deliver(fake, call("Hit", 2.95, [9]))
        self.assertEqual(len(s.calls()), 3)
        self.assertEqual(len(s.payloads()), 4)

    def test_lined_up_starts_at_since(self):
        fake = FakeSession([sent({"engine": ENGINE, "hooked": 1,
                                  "no_code": []}),
                            sent(call("Hit", 1.0)), sent(call("Hit", 2.1)),
                            sent(call("Hit", 2.95))])
        s = self.session(fake)
        self.assertTrue(self.in_wait.wait(THREAD_CAP_S))
        lined = s.lined_up(since=2)
        self.assertEqual(len(lined), 1)
        self.assertEqual(lined[0].note_index, 2)
        self.assertAlmostEqual(lined[0].diff_ms, -50.0, places=6)
        self.assertEqual([x.note_index for x in s.lined_up()], [0, 1, 2])

    def test_a_run_the_agent_ends_keeps_the_agents_end(self):
        fake = FakeSession([sent(call("Hit", 1.0)),
                            sent({"done": "song over", "calls": 1})])
        s = self.session(fake)
        self.assertTrue(s.stop(THREAD_CAP_S))
        self.assertEqual(s.ended_line(), "ended: done (agent: song over)")
        self.assertEqual(fake.detach_calls, 1)
        self.assertEqual(len(s.calls()), 1)

    def test_an_agent_end_that_lands_as_stop_is_seen_keeps_its_reason(self):
        # The agent's end arrives between the wait's check of `ended` and its
        # check of the stop event: the run is already over as "done".
        fake = FakeSession([])
        s = TH.TraceSession(self.NOTES, "Eng", ["Hit"], self.out_path,
                            agent_path=self.agent,
                            frida_mod=FakeFrida(fake), wait=self.blocking_wait)
        real_is_set = s._stop.is_set
        delivered = []

        def is_set_after_agent_ends():
            flag = real_is_set()
            if flag and not delivered:
                delivered.append(True)
                self.deliver(fake, {"done": "song over", "calls": 0})
            return flag

        s._stop.is_set = is_set_after_agent_ends
        s.start()
        self.assertTrue(self.in_wait.wait(THREAD_CAP_S))
        self.assertTrue(s.stop(THREAD_CAP_S))
        self.assertEqual(s.ended_line(), "ended: done (agent: song over)")
        self.assertEqual(fake.detach_calls, 1)

    def test_the_wall_cap_still_ends_a_run_nobody_stops(self):
        # The clock reads 0 when the wait starts, then past the 156 s cap.
        clock = iter([0.0, 157.0])
        fake = FakeSession([])
        s = self.session(fake, now=lambda: next(clock))
        s._thread.join(THREAD_CAP_S)
        self.assertTrue(s.ended())
        self.assertEqual(s.ended_line(), "ended: cap")
        self.assertEqual(fake.detach_calls, 1)
        self.assertEqual(s.lines, [
            "cap of 156 s reached before the agent finished; 0 messages seen"])

    def test_not_running_is_the_end_and_its_line_is_kept(self):
        s = self.session(None)
        self.assertTrue(s.stop(THREAD_CAP_S))
        self.assertEqual(s.ended_line(), "ended: not running")
        self.assertEqual(s.lines,
                         ["Clone Hero.exe is not running; nothing attached"])

    def test_a_runner_that_raises_ends_as_failed(self):
        fake = FakeSession([], load_raises=RuntimeError("script broke"))
        s = self.session(fake)
        self.assertTrue(s.stop(THREAD_CAP_S))
        self.assertEqual(s.result.reason, TH.END_FAILED)
        self.assertEqual(s.result.error, "RuntimeError: script broke")
        self.assertEqual(fake.detach_calls, 1)

    def test_a_missing_agent_says_how_to_build_it(self):
        os.remove(self.agent)
        with self.assertRaises(TH.AgentMissing) as raised:
            TH.TraceSession(self.NOTES, "Eng", ["Hit"], self.out_path,
                            agent_path=self.agent)
        self.assertEqual(str(raised.exception), TH.agent_missing_line(self.agent))

    def test_the_missing_agent_line_names_the_agents_own_build_script(self):
        self.assertEqual(TH.agent_missing_line("a.js"),
                         "a.js not found; run npm run build:trace in tools/ch_mcp/agent first")
        self.assertEqual(TH.agent_missing_line("b.js", "state"),
                         "b.js not found; run npm run build:state in tools/ch_mcp/agent first")

    def test_a_session_needs_a_method(self):
        with self.assertRaises(ValueError):
            TH.TraceSession(self.NOTES, "Eng", [], self.out_path,
                            agent_path=self.agent)


if __name__ == "__main__":
    unittest.main()
