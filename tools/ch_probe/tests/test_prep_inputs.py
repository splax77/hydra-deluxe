"""prep_inputs.py makes a chart's dump, score and schedule files with
hydra_replay and dry-loads the schedule the way ch_run_chart does.

No test runs hydra_replay itself (agent rule 14). Where a step has to
succeed, main's `run` hands back the file hydra_replay really wrote for that
command: testdata/replay holds Pitch Dark's dump, score and schedule, made by
this script on testdata/input/test_pitch_dark_ratio/notes.mid. Where a step
has to fail, a real program fails for real: this Python, given hydra_replay's
arguments, cannot open the file the first argument names and exits with its
own error.
"""

from __future__ import annotations

import io
import json
import os
import re
import subprocess
import sys
import tempfile
import time
import unittest
from unittest import mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import game_tools  # noqa: E402
from tools.ch_probe import prep_inputs  # noqa: E402

# The corpus chart whose prep_inputs output is in REPLAY_DIR.
PITCH_DARK_CHART = os.path.join(_REPO_ROOT, "testdata", "input", "test_pitch_dark_ratio",
                                "notes.mid")
REPLAY_DIR = os.path.join(_REPO_ROOT, "testdata", "replay")

# Lines 2 and 4 of the README of hydra-data\ch-mcp-runs\2026-11-07\limits-prep,
# the record prep_inputs' DEFAULT_CAP and DEFAULT_MS name as their source.
LIMITS_PREP_README = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "2026-11-07-limits-prep",
                                  "README.txt")

# A schedule hydra_replay (build Nov 12 2026) wrote for Pitch Dark, copied
# from hydra-data\ch-mcp-runs\2026-11-12\pitch-dark-prep\schedule.json with
# path_source's file cut to its file name, and cut down to the one press
# (entry 244) that today's loader refuses and the one chord (243) it names.
# That press lacks a key later builds write; every other key is as recorded.
STALE_SCHEDULE = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "2026-11-12-pitch-dark-prep",
                              "schedule.json")


def _recorded_outputs(test: unittest.TestCase) -> dict:
    """Pitch Dark's dump, score and schedule in REPLAY_DIR, by the
    hydra_replay command that wrote each, or a failure naming the one that is
    missing."""
    outputs = dict(zip(("dump", "score", "schedule"),
                       prep_inputs.file_names(PITCH_DARK_CHART, REPLAY_DIR)))
    for path in outputs.values():
        if not os.path.isfile(path):
            test.fail(f"{path} is missing; make it with: py -I tools/ch_probe/prep_inputs.py "
                      f"--chart {PITCH_DARK_CHART} --out {REPLAY_DIR}")
    return outputs


def _recorded_run(test: unittest.TestCase, calls: list, *, schedule: str | None = None,
                  real: tuple = ()):
    """A `run` for prep_inputs.main. It notes each command's arguments in
    `calls`. A command named in `real` goes to subprocess.run as it is; any
    other gets the file hydra_replay wrote for it (`schedule` in place of the
    recorded schedule when given), copied to the command's --out."""
    outputs = _recorded_outputs(test)
    if schedule is not None:
        outputs["schedule"] = schedule

    def run(command, **kwargs):
        calls.append(command[1:])
        if command[1] in real:
            return subprocess.run(command, **kwargs)
        out = command[command.index("--out") + 1]
        with open(outputs[command[1]], "rb") as src, open(out, "wb") as dst:
            dst.write(src.read())
        return subprocess.CompletedProcess(command, 0, "", "")

    return run


def _replay_failure(command) -> str:
    """The message prep_inputs.run_replay gives for `command`, run for real."""
    try:
        prep_inputs.run_replay(command)
    except prep_inputs.ReplayFailed as e:
        return str(e)
    raise AssertionError(f"{command} succeeded; it was meant to fail")


class PrepInputsTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.tmp = tmp.name
        self.out_dir = os.path.join(self.tmp, "prep")
        # An exe path main can find and date. `run` answers in its place, so
        # nothing ever runs it.
        self.exe = os.path.join(self.tmp, "hydra_replay.exe")
        open(self.exe, "wb").close()
        # The failing runs start this Python with hydra_replay's arguments, so
        # it looks for a file named "dump" or "schedule" in the working
        # folder. An empty folder makes sure there is none.
        self.cwd = os.path.join(self.tmp, "cwd")
        os.makedirs(self.cwd)
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(self.cwd)
        self.calls = []

    def _main(self, exe, run=subprocess.run):
        out = io.StringIO()
        code = prep_inputs.main(["--chart", PITCH_DARK_CHART, "--out", self.out_dir,
                                 "--exe", exe], out=out, run=run)
        return code, out.getvalue().splitlines()

    def test_the_defaults_are_the_cap_and_ms_of_the_recorded_profile_39(self):
        with open(LIMITS_PREP_README, encoding="utf-8") as f:
            text = f.read()
        profile = re.search(r"profile 39 = [^;\n]*?cap (\d+), ms (\d+)", text)
        command = re.search(r"dump --chart \S+ --cap (\S+) --ms (\S+)", text)
        self.assertIsNotNone(profile, text)
        self.assertIsNotNone(command, text)
        self.assertEqual((prep_inputs.DEFAULT_CAP, prep_inputs.DEFAULT_MS), profile.groups())
        self.assertEqual((prep_inputs.DEFAULT_CAP, prep_inputs.DEFAULT_MS), command.groups())

    def test_a_hydra_replay_call_past_REPLAY_CAP_S_is_stopped(self):
        # A real process that runs well past the cap. The cap is lowered for
        # the test so the wait is short; run_replay reads it at each call.
        sleeper = [sys.executable, "-I", "-c", "import time; time.sleep(20)"]
        with mock.patch.object(prep_inputs, "REPLAY_CAP_S", 0.5):
            started = time.monotonic()
            with self.assertRaises(prep_inputs.ReplayFailed) as cm:
                prep_inputs.run_replay(sleeper)
            took = time.monotonic() - started
            self.assertIn(f"{prep_inputs.REPLAY_CAP_S:g} s", str(cm.exception))
        self.assertLess(took, 10.0, "the call was not stopped at the cap")

    def test_prep_inputs_runs_three_commands_and_dry_loads_the_schedule(self):
        code, lines = self._main(self.exe, _recorded_run(self, self.calls))
        dump, score, schedule = prep_inputs.file_names(PITCH_DARK_CHART, self.out_dir)
        # The dump command is dump_command's with the default cap and ms; the
        # score and schedule commands take the chart, the path, its index and
        # the out file, and nothing else.
        self.assertEqual(self.calls, [
            prep_inputs.dump_command(self.exe, PITCH_DARK_CHART, dump)[1:],
            ["score", "--chart", PITCH_DARK_CHART, "--path", dump, "--index", "0",
             "--out", score],
            ["schedule", "--chart", PITCH_DARK_CHART, "--path", dump, "--index", "0",
             "--out", schedule]])
        self.assertEqual(code, prep_inputs.EXIT_LOADS)
        self.assertTrue(lines[0].startswith(f"hydra_replay: {self.exe}, file date "), lines[0])
        self.assertEqual(lines[1:4], [f"dump: {dump}", f"score: {score}",
                                      f"schedule: {schedule}"])
        # The dry load is chords_to_play's on the schedule file: its chord
        # count and its lines.
        chords, expected = game_tools.chords_to_play(schedule)
        self.assertTrue(chords)
        self.assertEqual(lines[4], f"loads: {len(chords)} chords")
        self.assertEqual(lines[5:], expected)

    def test_file_names_are_the_files_prep_inputs_wrote_for_a_chart(self):
        # REPLAY_DIR's Pitch Dark files were written by this script, so
        # file_names must name them, each holding what its command writes.
        dump, score, schedule = prep_inputs.file_names(PITCH_DARK_CHART, REPLAY_DIR)
        docs = []
        for path in (dump, score, schedule):
            self.assertEqual(os.path.dirname(path), REPLAY_DIR)
            self.assertTrue(os.path.isfile(path), path)
            with open(path, encoding="utf-8") as f:
                docs.append(json.load(f))
        self.assertIn("paths", docs[0])
        self.assertIn("final", docs[1])
        self.assertIn("presses", docs[2])

    def test_a_refusal_is_printed_and_exits_1(self):
        code, lines = self._main(self.exe, _recorded_run(self, self.calls,
                                                         schedule=STALE_SCHEDULE))
        _dump, _score, schedule = prep_inputs.file_names(PITCH_DARK_CHART, self.out_dir)
        with self.assertRaises(game_tools.ChordsRefused) as cm:
            game_tools.chords_to_play(schedule)
        self.assertEqual(code, prep_inputs.EXIT_REFUSED)
        # The refusal, in the loader's words, is the last line: nothing loaded
        # is printed after the three file lines.
        self.assertEqual(len(lines), 5, lines)
        self.assertTrue(lines[4].endswith(str(cm.exception)), lines[4])
        self.assertIn(schedule, lines[4])

    def test_a_failing_schedule_command_exits_2_before_any_load(self):
        # The dump and score steps get their recorded files; the schedule step
        # runs this Python, which fails for real.
        code, lines = self._main(sys.executable,
                                 _recorded_run(self, self.calls, real=("schedule",)))
        dump, _score, schedule = prep_inputs.file_names(PITCH_DARK_CHART, self.out_dir)
        command = [sys.executable, "schedule", "--chart", PITCH_DARK_CHART, "--path", dump,
                   "--index", "0", "--out", schedule]
        failure = _replay_failure(command)
        self.assertEqual(code, prep_inputs.EXIT_REPLAY_FAILED)
        self.assertEqual(lines[1:], failure.splitlines())
        self.assertIn(" schedule ", failure.splitlines()[0])
        self.assertFalse(os.path.exists(schedule))

    def test_a_failing_exe_exits_2_with_its_error(self):
        code, lines = self._main(sys.executable)
        dump, score, _schedule = prep_inputs.file_names(PITCH_DARK_CHART, self.out_dir)
        command = prep_inputs.dump_command(sys.executable, PITCH_DARK_CHART, dump)
        done = subprocess.run(command, capture_output=True, text=True)
        failure = _replay_failure(command)
        self.assertEqual(code, prep_inputs.EXIT_REPLAY_FAILED)
        self.assertEqual(lines[1:], failure.splitlines())
        self.assertIn(str(done.returncode), failure.splitlines()[0])
        self.assertTrue(failure.endswith(done.stderr.strip()), failure)
        # The first failure ends the run: no step after it wrote anything.
        self.assertEqual(os.listdir(self.out_dir), [])

    def test_a_missing_exe_exits_2_before_anything_runs(self):
        missing = os.path.join(self.tmp, "nowhere", "hydra_replay.exe")
        code, lines = self._main(missing)
        self.assertEqual(code, prep_inputs.EXIT_REPLAY_FAILED)
        self.assertEqual(len(lines), 1, lines)
        self.assertTrue(lines[0].endswith(missing), lines[0])
        self.assertFalse(os.path.exists(self.out_dir))

    def test_the_default_exe_is_where_the_default_build_puts_it(self):
        # build_cpp.ps1's default preset, and that preset's binary folder and
        # configuration in CMakePresets.json; the Visual Studio generator puts
        # each exe in <binary folder>/<configuration>.
        with open(os.path.join(_REPO_ROOT, "build_cpp.ps1"), encoding="utf-8") as f:
            preset = re.search(r'\[string\]\$Preset = "([^"]+)"', f.read())
        self.assertIsNotNone(preset)
        with open(os.path.join(_REPO_ROOT, "CMakePresets.json"), encoding="utf-8") as f:
            presets = json.load(f)
        configure = next(p for p in presets["configurePresets"]
                         if p["name"] == preset.group(1))
        build = next(p for p in presets["buildPresets"] if p["name"] == preset.group(1))
        self.assertTrue(configure["generator"].startswith("Visual Studio"), configure)
        binary_dir = configure["binaryDir"].replace("${sourceDir}", _REPO_ROOT)
        self.assertEqual(os.path.normpath(prep_inputs.DEFAULT_EXE),
                         os.path.normpath(os.path.join(binary_dir, build["configuration"],
                                                       "hydra_replay.exe")))


if __name__ == "__main__":
    unittest.main()
