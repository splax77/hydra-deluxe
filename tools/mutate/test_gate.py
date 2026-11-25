"""Tests for gate.py, on real git diffs and run folders the real runners wrote.

    py -I -m unittest discover -s tools/mutate -p test_gate.py

The Python cases build a throwaway git repository holding a small module and
three test modules, then run the real py_mutate.py on it, which plants the
bugs and runs the tests. So every catch and reach the gate reads is one a
real run produced; nothing in a run folder is typed here.

The C++ cases need a run folder the real run.py wrote from a mutation build,
which only the main session can make:

    set HYDRA_MUTATE_RUN=<run folder>
    set HYDRA_MUTATE_MUTANTS=<its mutants.json>   (when not two folders up)
    set HYDRA_MUTATE_SOURCE=<the checkout it was built from>   (default: this one)

With no run folder named they skip. They pick three of the run's tests by
what report.py says of them, copy each test's real source file into a
throwaway repository, and make the change a real `git diff -U0` shows.
"""

import contextlib
import io
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
import gate  # noqa: E402
import py_mutate  # noqa: E402
import report  # noqa: E402

RUN_VAR = "HYDRA_MUTATE_RUN"
MUTANTS_VAR = "HYDRA_MUTATE_MUTANTS"
SOURCE_VAR = "HYDRA_MUTATE_SOURCE"


def _git(repo, *args):
    done = subprocess.run(["git", "-C", str(repo)] + list(args), capture_output=True, text=True)
    if done.returncode != 0:
        raise AssertionError(f"git {' '.join(args)} failed: {done.stderr}")
    return done.stdout.strip()


def _new_repo(path):
    path.mkdir(parents=True)
    _git(path, "init", "-q")
    _git(path, "config", "user.email", "gate-test@example.invalid")
    _git(path, "config", "user.name", "gate test")
    _git(path, "config", "commit.gpgsign", "false")
    _git(path, "config", "core.autocrlf", "false")


def _commit(repo, files, message):
    """Writes {path: text} into the repository and commits it; returns the commit."""
    for rel, text in files.items():
        p = repo / rel
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(text, encoding="utf-8", newline="\n")
        _git(repo, "add", rel)
    _git(repo, "commit", "-q", "-m", message)
    return _git(repo, "rev-parse", "HEAD")


def _gate(argv):
    """gate.main's exit code and the lines it printed."""
    out = io.StringIO()
    with contextlib.redirect_stdout(out):
        code = gate.main(argv)
    return code, out.getvalue().splitlines()


def _rows(lines):
    """The printed lines that judge a test, as (word, test)."""
    out = []
    for line in lines:
        for word in (gate.PASSED, gate.REFUSED, gate.NOT_JUDGED):
            if line.startswith(word + ": "):
                out.append((word, line[len(word) + 2:].split(": ", 1)[0]))
    return out


# The module the bugs are planted in, and the test modules. Each test module
# holds one test, so each can be given to the gate alone.
CALC = "def is_small(n):\n    return n < 10\n"
TESTS = {
    # Requires the boundary's answer, so a moved boundary fails it.
    "test_value": ("import unittest\nimport calc\n\n\nclass Value(unittest.TestCase):\n"
                   "    def test_ten_is_not_small(self):\n        self.assertFalse(calc.is_small(10))\n"),
    # Calls the code and requires nothing of it.
    "test_runs": ("import unittest\nimport calc\n\n\nclass Runs(unittest.TestCase):\n"
                  "    def test_calls_it(self):\n        calc.is_small(3)\n"),
    # Never calls the module the bugs are in.
    "test_none": ("import unittest\n\n\nclass Other(unittest.TestCase):\n"
                  "    def test_strings(self):\n        self.assertEqual('ab' * 2, 'abab')\n"),
}
KEPT = ("import unittest\nimport calc\n\n\nclass Kept(unittest.TestCase):\n"
        "    def test_small(self):\n        self.assertTrue(calc.is_small(9))\n        self.assertFalse(calc.is_small(10))\n")


class PythonRun(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = Path(tempfile.mkdtemp(prefix="gate_test_"))
        cls.repo = cls.tmp / "repo"
        _new_repo(cls.repo)
        cls.base = _commit(cls.repo, {"pkg/calc.py": CALC, "pkg/test_kept.py": KEPT}, "base")
        cls.head = _commit(cls.repo, {f"pkg/{m}.py": t for m, t in TESTS.items()}, "three new tests")
        cls.run_dir = cls.tmp / "run"
        with contextlib.redirect_stdout(io.StringIO()):
            code = py_mutate.main(["--target", str(cls.repo / "pkg"), "--out", str(cls.run_dir),
                                   "--tests"] + list(TESTS) + ["test_kept"])
        if code != 0:
            raise AssertionError("py_mutate.py did not finish its run")
        lines = [json.loads(x) for x in (cls.run_dir / "map.jsonl").read_text(encoding="utf-8").splitlines()
                 if x.strip()]
        cls.tests, cls.reach, _, _ = py_mutate.build_map(lines)
        cls.ids = {x["module"]: x["test"] for x in cls.tests}

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tmp, ignore_errors=True)

    def _run(self, files, rng=None):
        out = self.tmp / "out"
        argv = ["py", str(self.run_dir), "--repo", str(self.repo), "--target", "pkg",
                "--range", rng or f"{self.base}..{self.head}", "--out", str(out), "--files"] + files
        return _gate(argv)

    def test_the_three_outcomes_in_order(self):
        code, lines = self._run([f"pkg/{m}.py" for m in TESTS])
        self.assertEqual(_rows(lines), [(gate.PASSED, self.ids["test_value"]),
                                        (gate.REFUSED, self.ids["test_runs"]),
                                        (gate.NOT_JUDGED, self.ids["test_none"])])
        self.assertEqual(code, gate.EXIT_REFUSED)
        self.assertIn(gate.PY_WHOLE_MODULE, lines)

    def test_each_alone_gives_its_exit_code(self):
        expected = {"test_value": gate.EXIT_NONE_REFUSED, "test_runs": gate.EXIT_REFUSED,
                    "test_none": gate.EXIT_NONE_REFUSED}
        for module, want in expected.items():
            with self.subTest(module=module):
                code, _ = self._run([f"pkg/{module}.py"])
                self.assertEqual(code, want)

    def test_a_test_not_judged_never_reads_passed(self):
        code, lines = self._run(["pkg/test_none.py"])
        self.assertEqual(_rows(lines), [(gate.NOT_JUDGED, self.ids["test_none"])])
        self.assertIn(gate.REACHES_NONE, "\n".join(lines))
        self.assertFalse([line for line in lines if line.startswith(gate.PASSED)])

    def test_a_refused_line_names_the_test_its_bugs_and_the_file_listing_them(self):
        tid = self.ids["test_runs"]
        code, lines = self._run(["pkg/test_runs.py"])
        refused = [line for line in lines if line.startswith(gate.REFUSED + ": ")]
        self.assertEqual(len(refused), 1)
        gate_file = self.tmp / "out" / gate.GATE_FILE
        self.assertTrue(refused[0].startswith(f"{gate.REFUSED}: {tid}: reached {len(self.reach[tid])} planted bugs"))
        self.assertIn(str(gate_file), refused[0])
        # The file lists each bug the test reaches with py_mutate's own line for it.
        summary = (self.run_dir / "summary.txt").read_text(encoding="utf-8").splitlines()
        listed = gate_file.read_text(encoding="utf-8").splitlines()
        for n in self.reach[tid]:
            own = [line.strip() for line in summary if line.startswith(f"  #{n} ")]
            self.assertEqual(len(own), 1)
            self.assertIn("  " + own[0], listed)

    def test_a_test_file_the_range_added_no_line_to_is_not_judged(self):
        code, lines = self._run(["pkg/test_kept.py"])
        self.assertEqual(_rows(lines), [])
        self.assertEqual(code, gate.EXIT_NONE_REFUSED)
        self.assertTrue(any(line.startswith("unchanged: pkg/test_kept.py") for line in lines))

    def test_a_line_added_to_an_old_test_module_judges_it(self):
        later = _commit(self.repo, {"pkg/test_kept.py": KEPT + "    # a second line of this test\n"}, "change")
        # The other cases name their range by commit, so this later commit leaves them alone.
        code, lines = self._run(["pkg/test_kept.py"], f"{self.head}..{later}")
        self.assertEqual(_rows(lines), [(gate.PASSED, self.ids["test_kept"])])
        self.assertEqual(code, gate.EXIT_NONE_REFUSED)


class AddedLines(unittest.TestCase):
    def test_a_real_diff_gives_each_files_added_lines(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo = Path(tmp) / "repo"
            _new_repo(repo)
            base = _commit(repo, {"a.txt": "one\ntwo\nthree\n", "b.txt": "x\n"}, "base")
            head = _commit(repo, {"a.txt": "one\nnew\ntwo\nthree\nlast\n", "b.txt": "y\n"}, "change")
            added = gate.added_lines(repo, f"{base}..{head}", ["a.txt", "b.txt"])
        self.assertEqual(added, {"a.txt": {2, 5}, "b.txt": {1}})


def _source_lines(path):
    return path.read_text(encoding="utf-8").split("\n")


@unittest.skipUnless(os.environ.get(RUN_VAR), f"set {RUN_VAR} to a run folder run.py wrote")
class CppRun(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.run_dir = Path(os.environ[RUN_VAR]).resolve()
        cls.mutants = (Path(os.environ[MUTANTS_VAR]) if os.environ.get(MUTANTS_VAR)
                       else cls.run_dir.parent.parent / "mutants.json").resolve()
        cls.source = Path(os.environ.get(SOURCE_VAR) or Path(HERE).parents[1]).resolve()
        cls.r = report.build_report(cls.run_dir, cls.mutants)
        cls.tmp = Path(tempfile.mkdtemp(prefix="gate_cpp_test_"))
        m = cls.r["map"]
        uncounted = set(cls.r["lists"]["incomplete"]) | set(cls.r["not_run"])
        red = set(m["red_with_no_bug"])

        def first(pick):
            return next((t for t in m["order"] if t not in red and pick(cls.r["tests"][t])), None)

        cls.picked = {
            gate.PASSED: first(lambda rec: rec["caught"]),
            gate.REFUSED: first(lambda rec: not rec["caught"] and set(rec["reaches"]) - uncounted),
            gate.NOT_JUDGED: first(lambda rec: not rec["caught"] and not rec["reaches"]),
        }

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tmp, ignore_errors=True)

    def _file_of(self, tid):
        rec = self.r["map"]["tests"][tid]
        rel = rec["file"].replace("\\", "/")
        lines = _source_lines(self.source / rel)
        if len(lines) <= rec["line"] or "TEST_CASE" not in lines[rec["line"] - 1]:
            self.skipTest(f"{self.source / rel} line {rec['line']} is not {tid}; the run is not of this checkout")
        return rel, lines, rec["line"]

    def _repo_without(self, name, rel, lines, drop):
        """A repository whose change adds line `drop` of the real file back."""
        repo = self.tmp / name
        _new_repo(repo)
        base = _commit(repo, {rel: "\n".join(lines[:drop - 1] + lines[drop:])}, "base")
        head = _commit(repo, {rel: "\n".join(lines)}, "the real file")
        return repo, f"{base}..{head}"

    def _gate(self, repo, rng, rel):
        return _gate(["cpp", str(self.run_dir), "--mutants", str(self.mutants), "--repo", str(repo),
                      "--range", rng, "--out", str(self.tmp / "out"), "--files", rel])

    def test_a_line_added_inside_one_test_judges_that_test(self):
        want_code = {gate.PASSED: gate.EXIT_NONE_REFUSED, gate.REFUSED: gate.EXIT_REFUSED,
                     gate.NOT_JUDGED: gate.EXIT_NONE_REFUSED}
        for word, tid in self.picked.items():
            with self.subTest(outcome=word):
                if tid is None:
                    self.skipTest(f"the run holds no test the gate would call {word}")
                rel, lines, line = self._file_of(tid)
                repo, rng = self._repo_without(f"inside{len(word)}", rel, lines, line + 1)
                code, printed = self._gate(repo, rng, rel)
                self.assertEqual(_rows(printed), [(word, tid)])
                self.assertEqual(code, want_code[word])
                if word == gate.NOT_JUDGED:
                    self.assertFalse([p for p in printed if p.startswith(gate.PASSED)])

    def test_a_line_added_above_a_files_first_test_judges_none(self):
        tid = next(t for t in self.picked.values() if t is not None)
        rel, lines, _ = self._file_of(tid)
        first = min(self.r["map"]["tests"][t]["line"] for t in self.r["map"]["order"]
                    if self.r["map"]["tests"][t]["file"].replace("\\", "/") == rel)
        if first < 2:
            self.skipTest(f"{rel}'s first test is on its first line")
        repo, rng = self._repo_without("above", rel, lines, first - 1)
        code, printed = self._gate(repo, rng, rel)
        self.assertEqual(_rows(printed), [])
        self.assertEqual(code, gate.EXIT_NONE_REFUSED)
        self.assertTrue(any(p.startswith(f"unchanged: {rel}") for p in printed))


if __name__ == "__main__":
    unittest.main()
