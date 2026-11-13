"""prep_inputs.py makes a chart's dump, score and schedule files with
hydra_replay and dry-loads the schedule the way ch_run_chart does.

No test runs the real hydra_replay (agent rule 14). Each test writes a stub:
a small Python script, behind a .cmd file so the script's own subprocess call
runs it as it would run the exe. The stub records its arguments and writes
canned files: the Bomber fixtures for a chart that loads, or a schedule with
no chords for one the loader refuses.
"""

from __future__ import annotations

import io
import json
import os
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import game_tools  # noqa: E402
from tools.ch_probe import prep_inputs  # noqa: E402
from tools.ch_probe.tests.test_play_chart import _fixture_schedule  # noqa: E402

BOMBER_DUMP = os.path.join(_HERE, "bomber_refine_dump.json")
BOMBER_SCORE = os.path.join(_HERE, "bomber_refine_score.json")
# test_play_chart._fixture_schedule's copy, until task T4 remakes the fixture
# with the end fields.
BOMBER_SCHEDULE = _fixture_schedule("bomber_refine_schedule.json")

# A schedule with no refusal and no chords, which chords_to_play refuses.
_EMPTY_SCHEDULE = {"refusals": [], "activations": [], "chords": [], "presses": [],
                   "song_length_ms": None}

_STUB = r'''
import json, shutil, sys
args = sys.argv[1:]
with open({calls!r}, "a", encoding="utf-8") as f:
    f.write(json.dumps(args) + "\n")
if {exit_code!r}:
    sys.stderr.write("stub failed on purpose\n")
    sys.exit({exit_code!r})
out = args[args.index("--out") + 1]
source = {{"dump": {dump!r}, "score": {score!r}, "schedule": {schedule!r}}}[args[0]]
if source is None:
    with open(out, "w", encoding="utf-8") as f:
        json.dump({empty!r}, f)
else:
    shutil.copyfile(source, out)
'''


class PrepInputsTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.tmp = tmp.name
        self.chart_dir = os.path.join(self.tmp, "Some Song")
        os.makedirs(self.chart_dir)
        self.chart = os.path.join(self.chart_dir, "notes.chart")
        with open(self.chart, "w", encoding="utf-8") as f:
            f.write("[Song]\n")
        self.out_dir = os.path.join(self.tmp, "prep")
        self.calls = os.path.join(self.tmp, "calls.log")

    def _stub(self, dump=BOMBER_DUMP, score=BOMBER_SCORE, schedule=BOMBER_SCHEDULE,
              exit_code=0) -> str:
        script = os.path.join(self.tmp, "stub_replay.py")
        with open(script, "w", encoding="utf-8") as f:
            f.write(_STUB.format(calls=self.calls, dump=dump, score=score, schedule=schedule,
                                 empty=_EMPTY_SCHEDULE, exit_code=exit_code))
        exe = os.path.join(self.tmp, "stub_replay.cmd")
        with open(exe, "w", encoding="utf-8") as f:
            f.write(f'@"{sys.executable}" -I "{script}" %*\r\n')
        return exe

    def _main(self, exe, *extra):
        out = io.StringIO()
        code = prep_inputs.main(["--chart", self.chart, "--out", self.out_dir, "--exe", exe,
                                 *extra], out=out)
        return code, out.getvalue().splitlines()

    def _calls(self):
        with open(self.calls, encoding="utf-8") as f:
            return [json.loads(line) for line in f]

    def test_the_defaults_are_profile_39s_cap_4_and_ms_10(self):
        self.assertEqual((prep_inputs.DEFAULT_CAP, prep_inputs.DEFAULT_MS), ("4", "10"))

    def test_each_hydra_replay_call_is_capped_at_the_users_120_s(self):
        self.assertEqual(prep_inputs.REPLAY_CAP_S, 120.0)

    def test_prep_inputs_runs_three_commands_and_dry_loads_the_schedule(self):
        exe = self._stub()
        code, lines = self._main(exe)
        dump, score, schedule = (os.path.join(self.out_dir, "Some Song-dump.json"),
                                 os.path.join(self.out_dir, "Some Song-score.json"),
                                 os.path.join(self.out_dir, "Some Song-schedule.json"))
        # The schedule command takes the chart, the path, its index and the
        # out file, and nothing else.
        self.assertEqual(self._calls(), [
            ["dump", "--chart", self.chart, "--cap", "4", "--ms", "10", "--out", dump],
            ["score", "--chart", self.chart, "--path", dump, "--index", "0", "--out", score],
            ["schedule", "--chart", self.chart, "--path", dump, "--index", "0",
             "--out", schedule]])
        self.assertEqual(code, 0)
        self.assertTrue(lines[0].startswith(f"hydra_replay: {exe}, file date "), lines[0])
        self.assertEqual(lines[1:4], [f"dump: {dump}", f"score: {score}",
                                      f"schedule: {schedule}"])
        # The dry load is chords_to_play's on the schedule file: its chord
        # count (the Bomber schedule fixture's 1,170) and its lines.
        chords, expected = game_tools.chords_to_play(schedule)
        self.assertEqual(lines[4], "loads: 1170 chords")
        self.assertEqual(len(chords), 1170)
        self.assertEqual(lines[5:], expected)
        self.assertTrue(expected[0].startswith("declined: chords 624 ("), expected[0])

    def test_file_names_puts_all_three_in_the_out_folder(self):
        self.assertEqual(prep_inputs.file_names(self.chart, self.out_dir), (
            os.path.join(self.out_dir, "Some Song-dump.json"),
            os.path.join(self.out_dir, "Some Song-score.json"),
            os.path.join(self.out_dir, "Some Song-schedule.json")))

    def test_a_refusal_is_printed_and_exits_1(self):
        code, lines = self._main(self._stub(dump=None, score=None, schedule=None))
        _dump, _score, schedule = prep_inputs.file_names(self.chart, self.out_dir)
        self.assertEqual(code, 1)
        self.assertEqual(lines[-1], f"refused: {schedule} has no chords to play")
        self.assertFalse([line for line in lines if line.startswith("loads:")])

    def test_a_failing_schedule_command_exits_2_before_any_load(self):
        # A stub that fails only on its third call: a schedule hydra_replay
        # refuses is never loaded.
        exe = self._stub()
        script = os.path.join(self.tmp, "stub_replay.py")
        with open(script, encoding="utf-8") as f:
            text = f.read()
        with open(script, "w", encoding="utf-8") as f:
            f.write(text.replace('out = args[args.index("--out") + 1]',
                                 'if args[0] == "schedule":\n'
                                 '    sys.stderr.write("error: no path 0\\n")\n'
                                 '    sys.exit(1)\n'
                                 'out = args[args.index("--out") + 1]'))
        code, lines = self._main(exe)
        self.assertEqual(code, 2)
        self.assertIn("hydra_replay schedule failed with exit code 1:", lines)
        self.assertIn("error: no path 0", lines)
        self.assertFalse([line for line in lines if line.startswith(("loads:", "schedule:"))])

    def test_a_failing_exe_exits_2_with_its_error(self):
        code, lines = self._main(self._stub(exit_code=3))
        self.assertEqual(code, 2)
        self.assertIn("hydra_replay dump failed with exit code 3:", lines)
        self.assertIn("stub failed on purpose", lines)
        self.assertEqual(len(self._calls()), 1)

    def test_a_missing_exe_exits_2_before_anything_runs(self):
        missing = os.path.join(self.tmp, "nowhere", "hydra_replay.exe")
        code, lines = self._main(missing)
        self.assertEqual((code, lines), (2, [f"hydra_replay not found: {missing}"]))
        self.assertFalse(os.path.exists(self.calls))

    def test_the_default_exe_is_this_checkouts_release_build(self):
        self.assertEqual(prep_inputs.DEFAULT_EXE,
                         os.path.join(_REPO_ROOT, "build-cpp", "Release", "hydra_replay.exe"))


if __name__ == "__main__":
    unittest.main()
