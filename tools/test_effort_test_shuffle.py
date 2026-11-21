"""Tests for tools/effort_test_shuffle.py and the scorer's brief it serves.

Each test builds a small git repo with six run branches, runs the script and reads the bundle
and key it wrote. Run with: py -I tools/test_effort_test_shuffle.py
"""

import importlib.util
import json
import os
import random
import re
import shutil
import stat
import subprocess
import sys
import tempfile
import types
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
SCRIPT = os.path.join(HERE, "effort_test_shuffle.py")
BRIEF = os.path.join(HERE, "effort_test_scorer_brief.md")

BRANCHES = ["claude/eff-quiet-%d" % n for n in range(1, 7)]


def read_brief():
    with open(BRIEF, encoding="utf-8") as f:
        return f.read()


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
        # The labels are the ones the script's own labelled() gives six branches.
        self.assertEqual(sorted(key["runs"]), sorted(load_script().labelled(BRANCHES, random.Random(0))))
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
        with open(self.key, encoding="utf-8") as f:
            labels = set(json.load(f)["runs"])
        names = os.listdir(self.out)
        files_of = {}
        for name in names:
            label, _, kind = name.partition(".")
            files_of.setdefault(label, set()).add(kind)
        # Every file belongs to a label in the key, every label has a file, and every label has
        # the same kinds of file, so nothing but the labelled runs' files is in the bundle.
        self.assertEqual(set(files_of), labels)
        kinds = files_of[min(labels)]
        self.assertTrue(kinds)
        for label in labels:
            self.assertEqual(files_of[label], kinds, label)
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

    def inputs(self, runs=None, key=None):
        """The script's parsed arguments for check_inputs, as run_script passes them."""
        return types.SimpleNamespace(run=[list(r) for r in (self.runs if runs is None else runs)],
                                     key=key or self.key, out=self.out)

    def assert_inputs_pass(self, runs=None, key=None):
        load_script().check_inputs(self.inputs(runs, key))

    def assert_refused(self, runs=None, key=None):
        """Runs the script and checks it exits 2 with the reason the script's check_inputs gives.

        Returns that reason, so a test can check it names the right thing."""
        result = self.run_script(runs=runs, key=key)
        self.assertEqual(result.returncode, 2)
        module = load_script()
        with self.assertRaises(module.CannotShuffle) as caught:
            module.check_inputs(self.inputs(runs, key))
        reason = str(caught.exception)
        self.assertIn(reason, result.stderr)
        return reason

    def test_a_count_other_than_six_is_refused(self):
        need = load_script().RUNS_PER_TASK
        self.assertEqual(len(self.runs), need)
        self.assert_inputs_pass()
        runs = self.runs[:need - 1]
        reason = self.assert_refused(runs=runs)
        self.assertIn(str(len(runs)), reason)
        self.assertIn(str(need), reason)
        self.assertFalse(os.path.exists(self.key))

    def test_a_repeated_branch_is_refused(self):
        self.assert_inputs_pass()
        self.assert_refused(runs=self.runs[:-1] + self.runs[:1])

    def test_an_existing_key_is_never_overwritten(self):
        self.assert_inputs_pass()
        with open(self.key, "w", encoding="utf-8") as f:
            f.write("first shuffle")
        self.assertIn(self.key, self.assert_refused())
        with open(self.key, encoding="utf-8") as f:
            self.assertEqual(f.read(), "first shuffle")

    def test_a_key_inside_the_bundle_is_refused(self):
        self.assert_inputs_pass()
        self.assert_refused(key=os.path.join(self.out, "key.json"))

    def test_a_key_in_a_folder_beside_the_bundle_is_allowed(self):
        folder = self.out + "-keys"
        os.mkdir(folder)
        self.assertEqual(self.run_script(key=os.path.join(folder, "key.json")).returncode, 0)

    def test_a_full_bundle_folder_is_refused(self):
        os.mkdir(self.out)
        self.assert_inputs_pass()
        with open(os.path.join(self.out, "old.diff"), "w", encoding="utf-8") as f:
            f.write("old")
        self.assertIn(self.out, self.assert_refused())

    def test_an_unknown_branch_writes_nothing(self):
        result = self.run_script(runs=self.runs[:5] + [("claude/gone", self.runs[5][1])])
        self.assertEqual(result.returncode, 2)
        self.assertFalse(os.path.exists(self.key))
        self.assertFalse(os.path.exists(self.out))

    def test_the_brief_names_every_label_and_bundle_file_the_script_writes(self):
        # The brief shows one attempt's files, under the first label, and names the label range.
        self.assertEqual(self.run_script().returncode, 0)
        brief = read_brief()
        labels = list(load_script().labelled(BRANCHES, random.Random(0)))
        first = labels[0]
        for label in (first, labels[-1]):
            self.assertRegex(brief, r"\b%s\b" % label)
        for named in re.findall(r"\bR\d+\b", brief):
            self.assertIn(named, labels)
        first_files = [name for name in os.listdir(self.out) if name.partition(".")[0] == first]
        self.assertTrue(first_files)
        for name in first_files:
            self.assertIn("`%s`" % name, brief)


class ScorerBriefTest(unittest.TestCase):
    def test_the_brief_holds_no_level_cost_or_agent_id(self):
        # The brief is a template: what the scorer learns beyond its fixed text comes only through
        # its <...> slots, so no slot may ask for an effort level, a cost or an agent id. The level
        # names belong to Claude Code, and no Hydra file owns a list of them, so they are matched
        # here as whole words, never as parts of words such as "follow".
        brief = read_brief().lower()
        for slot in re.findall(r"<([^<>]+)>", brief):
            for word in ("level", "effort", "cost", "agent"):
                self.assertNotIn(word, slot.split(), slot)
        for level in ("high", "medium", "low"):
            self.assertIsNone(re.search(r"\b%s\b" % level, brief), level)
        self.assertNotIn("$", brief)


if __name__ == "__main__":
    unittest.main()
