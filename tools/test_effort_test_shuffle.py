"""Tests for tools/effort_test_shuffle.py and the scorer's brief it serves.

Each test builds a small git repo with six run branches, runs the script and reads the bundle
and key it wrote. Run with: py -I tools/test_effort_test_shuffle.py
"""

import importlib.util
import json
import os
import random
import shutil
import stat
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
SCRIPT = os.path.join(HERE, "effort_test_shuffle.py")
BRIEF = os.path.join(HERE, "effort_test_scorer_brief.md")

BRANCHES = ["claude/eff-quiet-%d" % n for n in range(1, 7)]


def load_script():
    spec = importlib.util.spec_from_file_location("effort_test_shuffle", SCRIPT)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def remove_repo(path):
    """Git marks its object files read-only, which Windows will not delete."""
    def make_writable(remove, target, _):
        os.chmod(target, stat.S_IWRITE)
        remove(target)
    shutil.rmtree(path, onexc=make_writable)


class ShuffleTest(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp()
        self.addCleanup(remove_repo, self.dir)
        self.repo = os.path.join(self.dir, "repo")
        os.mkdir(self.repo)
        self.git("init", "-q", "-b", "base")
        self.write("a.txt", "base\n")
        self.git("add", "a.txt")
        self.git("commit", "-q", "-m", "base")
        self.base = self.git("rev-parse", "HEAD")
        self.runs = []
        for n, branch in enumerate(BRANCHES, 1):
            self.git("switch", "-q", "-c", branch, self.base)
            self.write("a.txt", "work of run %d\n" % n)
            self.git("commit", "-q", "-am", "work")
            output = os.path.join(self.dir, "test%d.txt" % n)
            with open(output, "w", encoding="utf-8") as f:
                f.write("C:\\x\\.claude\\worktrees\\eff-quiet-%d\\tools ok on %s, run %d\n" % (n, branch, n))
            self.runs.append((branch, output))
        self.key = os.path.join(self.dir, "key.json")
        self.out = os.path.join(self.dir, "bundle")

    def write(self, name, text):
        with open(os.path.join(self.repo, name), "w", newline="\n") as f:
            f.write(text)

    def git(self, *args):
        r = subprocess.run(["git", "-C", self.repo, "-c", "user.name=t", "-c", "user.email=t@t", *args],
                           capture_output=True, text=True)
        if r.returncode != 0:
            raise RuntimeError("git %s failed: %s" % (" ".join(args), r.stderr))
        return r.stdout.strip()

    def run_script(self, runs=None, key=None, out=None):
        args = ["--task", "quiet", "--repo", self.repo, "--base", self.base,
                "--key", key or self.key, "--out", out or self.out]
        for branch, output in self.runs if runs is None else runs:
            args += ["--run", branch, output]
        return subprocess.run([sys.executable, "-I", SCRIPT, *args], capture_output=True, text=True)

    def bundle_file(self, name):
        with open(os.path.join(self.out, name), encoding="utf-8") as f:
            return f.read()

    def test_key_file_records_each_label_once(self):
        r = self.run_script()
        self.assertEqual(r.returncode, 0, r.stderr)
        with open(self.key, encoding="utf-8") as f:
            key = json.load(f)
        self.assertEqual(key["task"], "quiet")
        self.assertEqual(key["base"], self.base)
        self.assertEqual(sorted(key["runs"]), ["R1", "R2", "R3", "R4", "R5", "R6"])
        self.assertEqual(sorted(key["runs"].values()), BRANCHES)

    def test_each_labelled_file_holds_the_run_the_key_says(self):
        self.assertEqual(self.run_script().returncode, 0)
        with open(self.key, encoding="utf-8") as f:
            runs = json.load(f)["runs"]
        for label, branch in runs.items():
            n = branch[-1]
            self.assertIn("+work of run %s\n" % n, self.bundle_file(label + ".diff"))
            self.assertTrue(self.bundle_file(label + ".test.txt").endswith(", run %s\n" % n))

    def test_the_bundle_holds_only_the_labelled_files_and_no_run_name(self):
        self.assertEqual(self.run_script().returncode, 0)
        names = sorted(os.listdir(self.out))
        self.assertEqual(names, sorted(["R%d.diff" % n for n in range(1, 7)] +
                                       ["R%d.test.txt" % n for n in range(1, 7)]))
        for name in names:
            self.assertNotIn("eff-quiet", self.bundle_file(name))

    def test_a_run_name_is_replaced_by_that_runs_label(self):
        module = load_script()
        labels = {"R1": "claude/eff-q-2", "R2": "claude/eff-q-1"}
        self.assertEqual(module.hide_run_names("claude/eff-q-1 and worktrees\\eff-q-2\\x, eff-q-10", labels),
                         "R2 and worktrees\\R1\\x, eff-q-10")

    def test_the_order_is_shuffled(self):
        module = load_script()
        rng = random.Random(1)
        orders = {tuple(module.labelled(BRANCHES, rng).values()) for _ in range(20)}
        self.assertGreater(len(orders), 1)

    def assert_refused(self, result, words):
        self.assertEqual(result.returncode, 2)
        self.assertIn(words, result.stderr)

    def test_a_count_other_than_six_is_refused(self):
        self.assert_refused(self.run_script(runs=self.runs[:5]), "got 5 runs, need 6")
        self.assertFalse(os.path.exists(self.key))

    def test_a_repeated_branch_is_refused(self):
        self.assert_refused(self.run_script(runs=self.runs[:5] + self.runs[:1]), "named twice")

    def test_an_existing_key_is_never_overwritten(self):
        with open(self.key, "w", encoding="utf-8") as f:
            f.write("first shuffle")
        self.assert_refused(self.run_script(), "already exists")
        with open(self.key, encoding="utf-8") as f:
            self.assertEqual(f.read(), "first shuffle")

    def test_a_key_inside_the_bundle_is_refused(self):
        self.assert_refused(self.run_script(key=os.path.join(self.out, "key.json")), "outside the bundle")

    def test_a_key_in_a_folder_beside_the_bundle_is_allowed(self):
        folder = self.out + "-keys"
        os.mkdir(folder)
        self.assertEqual(self.run_script(key=os.path.join(folder, "key.json")).returncode, 0)

    def test_a_full_bundle_folder_is_refused(self):
        os.mkdir(self.out)
        with open(os.path.join(self.out, "old.diff"), "w", encoding="utf-8") as f:
            f.write("old")
        self.assert_refused(self.run_script(), "not empty")

    def test_an_unknown_branch_writes_nothing(self):
        result = self.run_script(runs=self.runs[:5] + [("claude/gone", self.runs[5][1])])
        self.assertEqual(result.returncode, 2)
        self.assertFalse(os.path.exists(self.key))
        self.assertFalse(os.path.exists(self.out))


class ScorerBriefTest(unittest.TestCase):
    def test_the_brief_holds_no_level_cost_or_agent_id(self):
        with open(BRIEF, encoding="utf-8") as f:
            brief = f.read().lower()
        for word in ("high", "medium", "low", "$", "agent-"):
            self.assertNotIn(word, brief)

    def test_the_brief_names_the_three_inputs_and_the_labels(self):
        with open(BRIEF, encoding="utf-8") as f:
            brief = f.read()
        for words in ("<checklist file>", "<reference diff file>", "<bundle folder>", "R1 to R6"):
            self.assertIn(words, brief)


if __name__ == "__main__":
    unittest.main()
