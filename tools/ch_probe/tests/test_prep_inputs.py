"""prep_inputs.py makes a chart's dump and score files with hydra_replay and
dry-loads them the way ch_run_chart does.

No test runs the real hydra_replay (agent rule 14). Each test writes a stub:
a small Python script, behind a .cmd file so the script's own subprocess call
runs it as it would run the exe. The stub records its arguments and writes
canned files: the Bomber fixtures for a chart that loads, or a score with no
chords for one the loader refuses.
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
from tools.ch_probe.experiments import play_chart  # noqa: E402

BOMBER_DUMP = os.path.join(_HERE, "bomber_refine_dump.json")
BOMBER_SCORE = os.path.join(_HERE, "bomber_refine_score.json")

_STUB = r'''
import json, shutil, sys
args = sys.argv[1:]
with open({calls!r}, "a", encoding="utf-8") as f:
    f.write(json.dumps(args) + "\n")
if {exit_code!r}:
    sys.stderr.write("stub failed on purpose\n")
    sys.exit({exit_code!r})
out = args[args.index("--out") + 1]
source = {dump!r} if args[0] == "dump" else {score!r}
if source is None:
    with open(out, "w", encoding="utf-8") as f:
        json.dump({{"chords": []}} if args[0] == "score" else {{"paths": [{{"activations": []}}]}}, f)
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

    def _stub(self, dump=BOMBER_DUMP, score=BOMBER_SCORE, exit_code=0) -> str:
        script = os.path.join(self.tmp, "stub_replay.py")
        with open(script, "w", encoding="utf-8") as f:
            f.write(_STUB.format(calls=self.calls, dump=dump, score=score, exit_code=exit_code))
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

    def test_it_runs_dump_with_cap_4_and_ms_10_then_score_on_path_0_of_that_dump(self):
        self._main(self._stub())
        dump, score = (os.path.join(self.out_dir, "Some Song-dump.json"),
                       os.path.join(self.out_dir, "Some Song-score.json"))
        self.assertEqual(self._calls(), [
            ["dump", "--chart", self.chart, "--cap", "4", "--ms", "10", "--out", dump],
            ["score", "--chart", self.chart, "--path", dump, "--index", "0", "--out", score]])

    def test_a_chart_that_loads_names_both_files_and_prints_the_runs_lines(self):
        exe = self._stub()
        code, lines = self._main(exe)
        dump, score = prep_inputs.file_names(self.chart, self.out_dir)
        self.assertEqual(code, 0)
        self.assertTrue(lines[0].startswith(f"hydra_replay: {exe}, file date "), lines[0])
        self.assertIn(f"dump: {dump}", lines)
        self.assertIn(f"score: {score}", lines)
        chords, expected = game_tools.chords_to_play(
            score, play_chart.DECLINE_ALL_BUT_TOP_PAD, dump)
        start = lines.index(f"loads: {len(chords)} chords")
        self.assertEqual(lines[start + 1:], expected)
        self.assertTrue(expected[0].startswith("declined (all_but_top_pad): chords 624"))

    def test_a_refusal_is_printed_and_exits_1(self):
        code, lines = self._main(self._stub(dump=None, score=None))
        _dump, score = prep_inputs.file_names(self.chart, self.out_dir)
        self.assertEqual(code, 1)
        self.assertEqual(lines[-1], f"refused: {score} has no chords to play")
        self.assertFalse([line for line in lines if line.startswith("loads:")])

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
