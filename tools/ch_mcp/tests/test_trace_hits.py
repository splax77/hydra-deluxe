"""Tests for trace_hits: the once-per-note filter, the line-up with the
manifest, the 85 ms boundary, the settings the agent receives, and the
no-writes check over trace.ts.

No game and no Frida: the fake frida module and the fake wait come from
test_frida_attach, so both test files drive run_agent the same way.
"""

from __future__ import annotations

import json
import os
import sys
import tempfile
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
            json.dump({"song": "Window Map", "notes": notes_at(3000, 59080)},
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


if __name__ == "__main__":
    unittest.main()
