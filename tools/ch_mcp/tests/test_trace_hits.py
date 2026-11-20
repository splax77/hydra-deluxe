"""Tests for trace_hits: the once-per-note filter, the line-up with the
manifest, the 85 ms boundary, the settings the agent receives, the by-name
check over trace.ts, the two hook RVAs, where a trace's notes come from, and
the background TraceSession.

No game and no Frida: the fake frida module and the fake wait come from
test_frida_attach, so both test files drive run_agent the same way. The
reading and command-line tests run on real traces of the Window Map probe
song, copied under testdata/ch_runs with the song folder the runs played.
"""

from __future__ import annotations

import contextlib
import json
import math
import os
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import unittest
import unittest.mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import build_stamp as BS  # noqa: E402
from tools.ch_mcp import hydra_end  # noqa: E402
from tools.ch_mcp import trace_hits as TH  # noqa: E402
from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_mcp.tests.test_frida_attach import (  # noqa: E402
    AGENT_DIR, CH_RUNS, FakeFrida, FakeSession, instant_wait, by_name_only_findings,
    recorded, sent)
from tools.ch_mcp.tests.test_build_stamp import make_built_agent  # noqa: E402
from tools.ch_mcp.tests.test_hydra_end import FAKE, _REAL_MAKE_DUMP  # noqa: E402
from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import prep_inputs  # noqa: E402
from tools.ch_probe.probe_songs import WINDOW_MAP  # noqa: E402
from tools.ch_probe.tests.test_sp_compare import _replay_file  # noqa: E402


def setUpModule():
    FAKE.start()


def tearDownModule():
    FAKE.stop()


# Real traces of the Window Map probe song (hydra-data/ch-mcp-runs/
# 2026-10-28/l3): a count run that played it to the end, a count run the
# agent's song cap ended early (from l3/keys), an args run of the two
# per-note methods, and that args run's own report.
L3 = "2026-10-28-l3"
# The song folder those runs played (C:\Clone Hero\songs\Hydra Probe\
# Window Map): its manifest, chart and song.ini.
WINDOW_MAP_SONG = os.path.join(CH_RUNS, L3, WINDOW_MAP)
WINDOW_MAP_CHART = os.path.join(WINDOW_MAP_SONG, "notes.chart")
# Hydra's dump of that chart, the file hydra_end.make_dump writes for it.
WINDOW_MAP_DUMP = os.path.join(_REPO_ROOT, "testdata", "replay", "window_map-dump.json")
# A later trace of the same song that caught one call before it was stopped
# (hydra-data/ch-mcp-runs/2026-11-06/ls-d1b-run1/trace-2.jsonl).
ONE_CALL_RUN = ("2026-11-06-ls-d1b-run1", "trace-2.jsonl")


def window_map_notes() -> list[dict]:
    return TH.load_manifest(WINDOW_MAP_SONG)[1]


def recorded_payloads(run: str, name: str, key: str) -> list[dict]:
    """The payloads of a recorded agent log that hold `key`."""
    return [m["payload"] for m in recorded(run, name) if key in m["payload"]]


def recorded_table(name: str) -> list[dict]:
    """The method table a recorded count run's agent sent last."""
    return recorded_payloads(L3, name, "methods")[-1]["methods"]


def recorded_report(name: str) -> list[str]:
    with open(os.path.join(CH_RUNS, L3, name), encoding="utf-8") as fh:
        return fh.read().splitlines()


def window_map_dump(test: unittest.TestCase) -> str:
    """WINDOW_MAP_DUMP, or a failure naming the command that makes it."""
    if not os.path.isfile(WINDOW_MAP_DUMP):
        cmd = prep_inputs.dump_command(prep_inputs.DEFAULT_EXE, WINDOW_MAP_CHART,
                                       WINDOW_MAP_DUMP)
        test.fail(f"{WINDOW_MAP_DUMP} is missing; make it with: "
                  f"{subprocess.list2cmdline(cmd)}")
    return WINDOW_MAP_DUMP


def copy_window_map(folder: str) -> None:
    for name in os.listdir(WINDOW_MAP_SONG):
        shutil.copyfile(os.path.join(WINDOW_MAP_SONG, name), os.path.join(folder, name))


@contextlib.contextmanager
def window_map_dumps(test: unittest.TestCase, made: list):
    """hydra_end.make_dump writing WINDOW_MAP_DUMP for each chart it is asked
    about, putting the chart into `made`. For a copy of the Window Map chart,
    whose hydra_replay dump that file is."""
    dump = window_map_dump(test)

    def make_dump(chart, out):
        made.append(chart)
        shutil.copyfile(dump, out)

    with unittest.mock.patch.object(hydra_end, "make_dump", make_dump):
        yield

ENGINE = {"name": "Eng", "image": "StrikeCore", "parent": "Base<Inputs>",
          "shape": {"sealed": True, "generic_base": True,
                    "round_start_double": True, "ctor_rva": True}}


def entry(names, count, owner="engine", rva="0x1000"):
    return {"names": names, "owner": owner, "rva": rva, "params": ["uint"],
            "count": count, "args": []}


def notes_at(*times_ms):
    return [{"index": i, "time_ms": t} for i, t in enumerate(times_ms)]


def call(name, song_time_s, args=()):
    return {"call": name, "names": [name], "songTime": song_time_s,
            "args": list(args)}


class OncePerNoteTest(unittest.TestCase):
    def test_a_full_run_finds_the_studys_per_note_methods(self):
        # The count run that played Window Map to its end.
        entries = recorded_table("trace-wm-count.jsonl")
        n = len(window_map_notes())
        exact, off = TH.once_per_note(entries, n)
        # The score and dynamics methods run once per note
        # (constants.py names both from the study).
        self.assertLessEqual({C.RVA_NOTE_SCORE_ADD, C.RVA_NOTE_DYNAMICS},
                             {int(e["rva"], 16) for e in exact})
        self.assertEqual(off, [])
        # Against a note count one more or one fewer, the same methods are
        # the off-by-one ones and none is exact.
        for other in (n - 1, n + 1):
            with self.subTest(note_count=other):
                self.assertEqual(TH.once_per_note(entries, other), ([], exact))

    def test_a_run_cut_short_finds_no_per_note_method(self):
        # The keys count run on the same song ended on the agent's song cap
        # before its last notes, so no method reached the manifest's count or
        # one off it.
        entries = recorded_table("trace-keys-count.jsonl")
        self.assertEqual(TH.once_per_note(entries, len(window_map_notes())),
                         ([], []))


# A call line of trace_hits' args report: the nearest note's index, its time
# and the call's difference from it.
NEAREST_IN_REPORT = re.compile(r"nearest note (\d+) at (\d+) ms, diff ([+-]\d+\.\d) ms$")


class LineUpTest(unittest.TestCase):
    def setUp(self):
        self.notes = window_map_notes()

    def test_each_call_gets_the_nearest_note_the_run_reported(self):
        # The args run's own report names each call's nearest note and
        # difference, as line_up worked them out during the run.
        calls = recorded_payloads(L3, "trace-wm-args.jsonl", "call")
        reported = [m.groups() for m in map(NEAREST_IN_REPORT.search,
                                            recorded_report("trace-wm-args.txt")) if m]
        self.assertEqual(len(reported), len(calls))
        lined = TH.line_up(calls, self.notes)
        self.assertEqual([(str(x.note_index), str(x.note_ms), f"{x.diff_ms:+.1f}")
                          for x in lined], reported)

    def test_a_call_at_the_limit_counts_and_just_inside_its_distance_does_not(self):
        calls = recorded_payloads(*ONE_CALL_RUN, "call")
        lined = TH.line_up(calls, self.notes)
        gap = abs(lined[0].diff_ms)
        self.assertEqual(TH.count_within(lined, gap), 1)
        self.assertEqual(TH.count_within(lined, math.nextafter(gap, 0.0)), 0)
        # The hit came inside EXPECT_NORMAL_BACK_MS.
        self.assertEqual(TH.count_within(lined, C.EXPECT_NORMAL_BACK_MS), 1)

    def test_notes_with_no_call(self):
        # The one call sits near one note; the notes either side of it are
        # farther than EXPECT_NORMAL_BACK_MS from it, so every other note has
        # no call near it.
        calls = recorded_payloads(*ONE_CALL_RUN, "call")
        hit = TH.line_up(calls, self.notes)[0]
        limit = C.EXPECT_NORMAL_BACK_MS
        i = hit.note_index
        self.assertEqual(self.notes[i]["index"], i)
        for side in (self.notes[i - 1], self.notes[i + 1]):
            self.assertGreater(abs(hit.call_ms - side["time_ms"]), limit)
        n = len(self.notes)
        self.assertEqual(TH.notes_without_call(calls, self.notes, limit), n - 1)
        # The limit itself is included.
        gap = abs(hit.diff_ms)
        self.assertEqual(TH.notes_without_call(calls, self.notes, gap), n - 1)
        self.assertEqual(TH.notes_without_call(calls, self.notes,
                                               math.nextafter(gap, 0.0)), n)


class CapTest(unittest.TestCase):
    # A song end of 60,080 ms, as a dump of the Window Map chart could name.
    END_MS = 60080.0

    def test_the_song_cap_is_hydras_end_plus_the_margin(self):
        self.assertAlmostEqual(TH.trace_cap_s(self.END_MS), 90.08)

    def test_the_wall_cap_is_the_song_cap_plus_the_pause_room(self):
        self.assertAlmostEqual(TH.wall_cap_s(self.END_MS), 210.08)


class SongEndTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.song = os.path.join(self.tmp.name, "Song")
        os.mkdir(self.song)

    def test_the_end_is_hydras_for_the_folders_chart(self):
        copy_window_map(self.song)
        chart = os.path.join(self.song, "notes.chart")
        made = []
        with window_map_dumps(self, made):
            got = TH.song_end_ms(self.song)
        self.assertEqual(got, (hydra_end.song_length_ms(WINDOW_MAP_DUMP),
                               hydra_end.chart_source_text(chart)))
        self.assertEqual(made, [chart])

    def test_a_folder_with_no_chart_is_refused_with_no_dump(self):
        before = len(FAKE.charts)
        with self.assertRaises(hydra_end.SongEndMissing) as caught:
            TH.song_end_ms(self.song)
        self.assertEqual(str(caught.exception),
                         f"{self.song} has no notes.chart or notes.mid, so Hydra has no "
                         "song end to cap the trace at")
        self.assertEqual(len(FAKE.charts), before)

    def test_a_failed_dump_is_refused(self):
        # hydra_end's own make_dump, run with an exe that is not there, so
        # the real runner fails before any hydra_replay could start.
        copy_window_map(self.song)
        chart = os.path.join(self.song, "notes.chart")
        missing_exe = os.path.join(self.tmp.name, "no-such-hydra_replay.exe")
        with unittest.mock.patch.object(hydra_end, "make_dump", _REAL_MAKE_DUMP), \
                unittest.mock.patch.object(prep_inputs, "DEFAULT_EXE", missing_exe):
            with self.assertRaises(hydra_end.SongEndMissing) as caught:
                TH.song_end_ms(self.song)
        refusal = str(caught.exception)
        cause = caught.exception.__cause__
        self.assertIsInstance(cause, prep_inputs.ReplayFailed)
        # The refusal names the chart and carries the runner's own reason.
        self.assertIn(chart, refusal)
        self.assertIn(str(cause), refusal)


class CommandLineTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.song = os.path.join(self.tmp.name, "Song")
        os.mkdir(self.song)
        self.manifest = os.path.join(self.song, "manifest.json")
        with open(self.manifest, "w", encoding="utf-8") as fh:
            json.dump({"song": "Song", "notes": notes_at(1000, 2000, 3000)}, fh)
        self.chart = os.path.join(self.song, "notes.chart")
        open(self.chart, "w").close()
        FAKE.set_end(self.chart, 6000.0)
        self.agent = make_built_agent(self.tmp.name, "trace")
        self.out_path = os.path.join(self.tmp.name, "trace.jsonl")
        self.lines = []

    def run_main(self, extra, session, engine="Eng", wait=instant_wait):
        return TH.main(["--song", self.song, "--engine-class", engine,
                        "--agent", self.agent, "--out", self.out_path] + extra,
                       frida_mod=FakeFrida(session), wait=wait,
                       out=self.lines.append)

    def use_window_map(self) -> float:
        """The song folder becomes a copy of the Window Map song the recorded
        runs played, and its chart's dump is Hydra's. Returns Hydra's end."""
        copy_window_map(self.song)
        self.enterContext(window_map_dumps(self, []))
        return hydra_end.song_length_ms(WINDOW_MAP_DUMP)

    def test_count_mode_posts_the_settings_and_prints_the_table(self):
        table = {"done": "song over", "mode": "count", "calls": 9,
                 "not_hooked": [], "no_code": ["base Abs"],
                 "methods": [entry(["Tick"], 6, rva="0x2000"),
                             entry(["Hit", "Hit2"], 3, owner="base")]}
        session = FakeSession([sent({"engine": ENGINE, "hooked": 2,
                                     "no_code": ["base Abs"]}), sent(table)])
        code = self.run_main(["--mode", "count"], session)
        self.assertEqual(code, 0)
        # The fake dump's song end of 6,000 ms + 30 s margin = 36 s song cap;
        # plus 120 s of pause room = 156 s wall-clock cap.
        self.assertEqual(session.script.posted, [{
            "type": "start", "cap_s": 156.0, "song_cap_s": 36.0,
            "mode": "count", "engine_class": "Eng",
            "engine_ctor_rva": C.RVA_DRUMS_ENGINE_CTOR,
            "round_start_offset": C.OFF_ROUND_START, "methods": []}])
        self.assertEqual(session.detach_calls, 1)
        self.assertEqual(self.lines, [
            "ended: done (agent: song over)",
            "engine: Eng in StrikeCore, base Base<Inputs>",
            "shape: sealed yes, generic_base yes, round_start_double yes, "
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

    def test_args_mode_reports_a_real_run_as_that_run_did(self):
        # The args run's messages, played through the real runner and wait
        # with the same song, method names and engine class.
        end_ms = self.use_window_map()
        msgs = recorded(L3, "trace-wm-args.jsonl")
        calls = [m["payload"] for m in msgs if "call" in m["payload"]]
        methods = list(dict.fromkeys(c["call"] for c in calls))
        engine = msgs[0]["payload"]["engine"]["name"]
        session = FakeSession(msgs)
        code = self.run_main(["--mode", "args"]
                             + [arg for m in methods for arg in ("--method", m)],
                             session, engine=engine, wait=FA._default_wait)
        self.assertEqual(code, 0)
        # The agent gets trace_settings for Hydra's end of the song, and the
        # wall cap for that end.
        self.assertEqual(session.script.posted, [{
            **TH.trace_settings(end_ms, TH.MODE_ARGS, engine,
                                C.RVA_DRUMS_ENGINE_CTOR, methods),
            "type": "start", "cap_s": TH.wall_cap_s(end_ms)}])
        self.assertEqual(session.detach_calls, 1)
        # Every line of the run's own report comes back, in its order, but
        # for the two lines that named that run's own files.
        report = [line for line in recorded_report("trace-wm-args.txt")
                  if "\\" not in line]
        wanted = set(report)
        self.assertEqual([line for line in self.lines if line in wanted], report)
        # The lines that name files name this run's manifest and log.
        self.assertEqual(len([line for line in self.lines if self.manifest in line]), 1)
        self.assertTrue(self.lines[-1].endswith(self.out_path), self.lines[-1])
        with open(self.out_path, encoding="utf-8") as fh:
            self.assertEqual([json.loads(line) for line in fh], msgs)

    def test_empty_manifest_prints_one_line(self):
        # A manifest with no notes is refused before any dump or attach.
        with open(self.manifest, "w", encoding="utf-8") as fh:
            json.dump({"song": "Song", "notes": []}, fh)
        dumps_before = len(FAKE.charts)
        session = FakeSession([])
        code = self.run_main(["--mode", "count"], session)
        self.assertEqual(code, 1)
        self.assertEqual(len(self.lines), 1, self.lines)
        self.assertIn(self.manifest, self.lines[0])
        self.assertIsNone(session.script)  # no agent was loaded
        self.assertEqual(len(FAKE.charts), dumps_before)  # and no dump made

    def test_a_folder_with_no_chart_prints_one_line_and_traces_nothing(self):
        os.remove(self.chart)
        session = FakeSession([])
        code = self.run_main(["--mode", "count"], session)
        self.assertEqual(code, 1)
        self.assertEqual(self.lines, [
            f"{self.song} has no notes.chart or notes.mid, so Hydra has no song end "
            "to cap the trace at; nothing traced"])
        self.assertIsNone(session.script)  # no agent was loaded

    def test_an_agent_error_is_printed_and_exits_1(self):
        # A real agent error reply (hydra-data/ch-mcp-runs/2026-10-31/
        # ls-d1a-run2/kit-1.jsonl). The kit agent sent it, and the runner
        # ends every agent's run on an error the same way (FA.ends_run).
        self.use_window_map()
        msgs = recorded("2026-10-31-ls-d1a-run2", "kit-1.jsonl")
        error = msgs[0]["payload"]["error"]
        session = FakeSession(msgs)
        code = self.run_main(["--mode", "count"], session, wait=FA._default_wait)
        self.assertEqual(code, 1)
        self.assertEqual(session.detach_calls, 1)
        # The report ends on the agent's own error.
        self.assertTrue(self.lines[-2].endswith(error), self.lines)

    def test_a_missing_agent_says_how_to_build_it(self):
        os.remove(self.agent)
        code = self.run_main(["--mode", "count"], FakeSession([]))
        self.assertEqual(code, 1)
        self.assertEqual(self.lines, [BS.check(self.agent)])
        self.assertTrue(self.lines[0].startswith(f"{self.agent} not found; "))

    def test_a_stale_agent_runs_nothing(self):
        with open(os.path.join(self.tmp.name, "trace.ts"), "a", encoding="utf-8") as fh:
            fh.write("// an edit after the build\n")
        code = self.run_main(["--mode", "count"], FakeSession([]))
        self.assertEqual(code, 1)
        self.assertEqual(self.lines, [
            f"{self.agent} is stale: trace.ts changed since it was built; "
            "run npm run build:trace in tools/ch_mcp/agent first"])

    def test_the_modes_refuse_the_wrong_method_flags(self):
        for extra in (["--mode", "args"], ["--mode", "count", "--method", "X"]):
            with self.subTest(extra=extra):
                with self.assertRaises(SystemExit):
                    with unittest.mock.patch("sys.stderr"):
                        self.run_main(extra, FakeSession([]))


class TraceAgentTest(unittest.TestCase):
    def test_trace_ts_reads_by_name_only(self):
        with open(os.path.join(AGENT_DIR, "trace.ts"), encoding="utf-8") as fh:
            self.assertEqual(by_name_only_findings(fh.read()), [])

    def test_only_gamemanager_ts_looks_up_the_game_manager(self):
        # One owner: gamemanager.ts holds the lookup. These two agents call its
        # findGameManager and neither repeats the lookup.
        for name in ("clock.ts", "trace.ts"):
            with self.subTest(agent=name):
                with open(os.path.join(AGENT_DIR, name), encoding="utf-8") as fh:
                    text = fh.read()
                self.assertIn("findGameManager()", text)
                self.assertNotIn("gc.choose", text)
                self.assertNotIn("tryAssembly", text)

    def test_ended_line_is_the_line_each_real_run_printed(self):
        # Two real runs of trace.ts and the first line of each one's own
        # report: the args run the song ended, and the keys count run the
        # agent's cap ended (testdata/ch_runs/2026-10-28-l3/
        # trace-keys-count.txt is that line, copied from hydra-data/
        # ch-mcp-runs/2026-10-28/l3/keys/trace-keys-count.txt). The result
        # carries the end ends_run reads from the run's last payload.
        for log, report in (("trace-wm-args.jsonl", "trace-wm-args.txt"),
                            ("trace-keys-count.jsonl", "trace-keys-count.txt")):
            with self.subTest(run=log):
                payloads = [m["payload"] for m in recorded(L3, log)]
                result = FA.RunResult(reason=FA.ends_run(payloads[-1]),
                                      cap_s=0.0, payloads=payloads)
                self.assertEqual(TH.ended_line(result), recorded_report(report)[0])

    def test_the_end_reasons_are_the_runners(self):
        # Every real trace.ts log: only its last payload, the agent's `done`,
        # ends the run, and ends_run reads it as the runner's done.
        for log in ("trace-wm-args.jsonl", "trace-wm-count.jsonl",
                    "trace-keys-count.jsonl", "trace-ew-count.jsonl"):
            with self.subTest(run=log):
                payloads = [m["payload"] for m in recorded(L3, log)]
                self.assertEqual([FA.ends_run(p) for p in payloads[:-1]],
                                 [None] * (len(payloads) - 1))
                self.assertEqual(FA.ends_run(payloads[-1]), FA.END_DONE)


class HookRvaTest(unittest.TestCase):
    # The literals are the study's: docs/research/
    # 2026-10-28-clone-hero-mcp-feasibility.md, "Test 3: the hit method".
    def test_the_two_engine_methods_are_the_studys_addresses(self):
        self.assertEqual(C.RVA_NOTE_SCORE_ADD, 0x20DEEC0)
        self.assertEqual(C.RVA_NOTE_DYNAMICS, 0x20DE770)

    def test_the_default_hooks_only_the_score_method(self):
        self.assertEqual(C.PER_NOTE_DEFAULT_RVAS, (C.RVA_NOTE_SCORE_ADD,))


class SongNotesTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.song = os.path.join(self.tmp.name, "Song")
        os.mkdir(self.song)
        # Hydra's own score JSON for Pitch Dark (testdata/input/
        # test_pitch_dark_ratio/notes.mid), made by hydra_replay score; the
        # comment above _REPLAY_DIR in tools/ch_probe/tests/test_sp_compare.py
        # gives the command.
        self.score = _replay_file("test_pitch_dark_ratio-score.json")
        with open(self.score, encoding="utf-8") as fh:
            self.chords = json.load(fh)["chords"]

    def test_a_score_json_becomes_notes_in_the_manifest_shape(self):
        notes = TH.notes_from_score(self.score)
        # One note per chord, each with only keys a real probe song's
        # manifest note also has, so line_up reads either the same way.
        self.assertEqual(len(notes), len(self.chords))
        manifest_keys = set.intersection(*(set(n) for n in window_map_notes()))
        for note in notes:
            self.assertLessEqual(set(note), manifest_keys, note)
        self.assertEqual([(n["index"], n["time_ms"]) for n in notes],
                         [(c["index"], c["ms"]) for c in self.chords])
        # A call at a chord's time lines up with that chord's note, no gap.
        chord = self.chords[len(self.chords) // 2]
        hit = TH.line_up([{"songTime": C.ms_to_s(chord["ms"])}], notes)[0]
        self.assertEqual(hit.note_index, chord["index"])
        self.assertAlmostEqual(hit.diff_ms, 0.0, places=6)

    def test_the_manifest_wins_when_the_folder_has_one(self):
        # The Window Map song folder has its real manifest; the Pitch Dark
        # score JSON holds other notes, and the manifest's are used.
        manifest, notes = TH.load_manifest(WINDOW_MAP_SONG)
        self.assertNotEqual(len(notes), len(self.chords))
        self.assertEqual(TH.song_notes(WINDOW_MAP_SONG, self.score),
                         (manifest, notes))

    def test_the_score_json_is_used_when_there_is_no_manifest(self):
        self.assertEqual(TH.song_notes(self.song, self.score),
                         (self.score, TH.notes_from_score(self.score)))

    def test_no_source_names_both(self):
        # The refusal names the manifest it looked for, and the score JSON
        # that was missing, or says none was given.
        missing = os.path.join(self.tmp.name, "nope.json")
        manifest = TH.manifest_path(self.song)
        with self.assertRaises(TH.NotesMissing) as given:
            TH.song_notes(self.song, missing)
        with self.assertRaises(TH.NotesMissing) as none:
            TH.song_notes(self.song)
        for refusal in (str(given.exception), str(none.exception)):
            with self.subTest(refusal=refusal):
                self.assertIn(manifest, refusal)
        self.assertIn(missing, str(given.exception))
        self.assertNotIn(missing, str(none.exception))
        self.assertNotEqual(str(given.exception), str(none.exception))


# How long a test waits for the session's thread. Only a hang reaches it.
THREAD_CAP_S = 5.0


class TraceSessionTest(unittest.TestCase):
    NOTES = notes_at(1000, 2000, 3000)
    END_MS = 6000.0

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.agent = make_built_agent(self.tmp.name, "trace")
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
                            song_end_ms=self.END_MS, agent_path=self.agent,
                            frida_mod=FakeFrida(fake_session), **kw)
        self.addCleanup(s.stop, THREAD_CAP_S)
        return s.start()

    def deliver(self, fake_session, payload):
        fake_session.script.handlers["message"](sent(payload), None)

    def test_the_agent_gets_mains_args_mode_settings(self):
        fake = FakeSession([sent({"done": "song over", "calls": 0})])
        s = self.session(fake)
        self.assertTrue(s.stop(THREAD_CAP_S))
        # The song end of 6,000 ms + 30 s margin = 36 s song cap; plus 120 s
        # of pause room = 156 s wall-clock cap.
        self.assertEqual(fake.script.posted, [{
            "type": "start", "cap_s": 156.0, "song_cap_s": 36.0,
            "mode": "args", "engine_class": "Eng",
            "engine_ctor_rva": C.RVA_DRUMS_ENGINE_CTOR,
            "round_start_offset": C.OFF_ROUND_START, "methods": ["Hit"]}])

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
                            song_end_ms=self.END_MS, agent_path=self.agent,
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
                            song_end_ms=self.END_MS, agent_path=self.agent)
        self.assertEqual(str(raised.exception), BS.check(self.agent))
        self.assertTrue(str(raised.exception).startswith(f"{self.agent} not found; "))

    def test_a_session_needs_a_method(self):
        with self.assertRaises(ValueError):
            TH.TraceSession(self.NOTES, "Eng", [], self.out_path,
                            song_end_ms=self.END_MS, agent_path=self.agent)


if __name__ == "__main__":
    unittest.main()
