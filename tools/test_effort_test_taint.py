"""Tests for tools/effort_test_taint.py, which lists calls that may have read the solution.

Each test writes a fixture transcript, runs the script the way the main session does and reads
what it printed. Run with: py -I tools/test_effort_test_taint.py
"""

import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
SCRIPT = os.path.join(HERE, "effort_test_taint.py")

KEY = "a" * 40
CHECKOUT = "C:\\Users\\Patrick\\Downloads\\Hydra\\hydra-test"
WORKTREE = CHECKOUT + "\\.claude\\worktrees\\eff-rev-quiet-1"
BRANCH = "claude/eff-rev-quiet-1"


def bash(command, **extra):
    return "Bash", dict({"command": command}, **extra)


def read(path):
    return "Read", {"file_path": path}


class TaintTest(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, self.dir)

    def run_script(self, calls, **run):
        """Prints of the script over one transcript holding calls, one per record."""
        transcript = os.path.join(self.dir, "agent-abc123.jsonl")
        with open(transcript, "w", encoding="utf-8") as f:
            for i, (tool, tool_input) in enumerate(calls):
                record = {"type": "assistant", "agentId": "abc123", "message": {"content": [
                    {"type": "text", "text": "main"},
                    {"type": "tool_use", "id": "t%d" % i, "name": tool, "input": tool_input}]}}
                f.write(json.dumps(record) + "\n")
            f.write(json.dumps({"type": "user", "message": {"content": "git show main:src/a.cpp"}}) + "\n")
        wave = os.path.join(self.dir, "wave.json")
        with open(wave, "w", encoding="utf-8") as f:
            json.dump([dict({"transcript": transcript, "worktree": WORKTREE, "branch": BRANCH,
                             "key": KEY}, **run)], f)
        r = subprocess.run([sys.executable, "-I", SCRIPT, wave], capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stderr)
        lines = r.stdout.splitlines()
        return lines[:-1], lines[-1]

    def assert_listed(self, call, reason, **run):
        listed, _ = self.run_script([call], **run)
        self.assertEqual(len(listed), 1, listed)
        self.assertIn(reason, listed[0])

    def assert_clean(self, call, **run):
        listed, _ = self.run_script([call], **run)
        self.assertEqual(listed, [])

    def test_a_run_with_no_worktree_has_every_checkout_path_listed(self):
        self.assert_listed(read(WORKTREE + "\\src\\a.cpp"), "path outside", worktree=None)

    def test_relative_climbs_out_of_the_worktree_are_listed(self):
        self.assert_listed(bash("cat ../../../src/a.cpp"), "climbs")
        self.assert_listed(bash("cat ..\\eff-rev-quiet-2\\src\\a.cpp"), "climbs")
        self.assert_clean(bash("cat ../a.cpp"))

    def test_a_climb_with_nothing_after_it_and_a_results_worktree_are_listed(self):
        self.assert_listed(bash("cd ../../.."), "climbs")
        self.assert_listed(bash("ls ..\\..\\"), "climbs")
        self.assert_listed(bash("cat ../effort-test-results/a.md"), "climbs")
        self.assert_clean(bash("cat ../.hidden"))

    def test_home_spellings_of_the_checkout_are_outside(self):
        for path in ("~/Downloads/Hydra/hydra-test/src/a.cpp",
                     "$HOME/Downloads/Hydra/hydra-test/src/a.cpp",
                     "'~/Downloads/Hydra/hydra-test'",
                     "$env:USERPROFILE\\Downloads\\Hydra\\hydra-test\\src",
                     "%USERPROFILE%\\Downloads\\Hydra\\hydra-test\\src"):
            self.assert_listed(bash("cat " + path), "path outside")
        self.assert_clean(bash("git show HEAD~2"))

    def test_tool_calls_with_no_id_are_each_listed(self):
        transcript = os.path.join(self.dir, "agent-noid.jsonl")
        record = {"type": "assistant", "message": {"content": [
            {"type": "tool_use", "name": "Bash", "input": {"command": "git log main"}},
            {"type": "tool_use", "name": "Bash", "input": {"command": "git show main:a"}}]}}
        with open(transcript, "w", encoding="utf-8") as f:
            f.write(json.dumps(record) + "\n")
        wave = os.path.join(self.dir, "wave2.json")
        with open(wave, "w", encoding="utf-8") as f:
            json.dump([{"transcript": transcript}], f)
        r = subprocess.run([sys.executable, "-I", SCRIPT, wave], capture_output=True, text=True)
        self.assertEqual(r.stdout.splitlines()[-1], "2 calls listed from 1 transcripts")

    def test_git_all_forms_are_listed(self):
        self.assert_listed(bash("git log --all"), "git --all")
        self.assert_listed(bash("git log --oneline --all -5"), "git --all")
        self.assert_listed(bash("git -C x branch -a"), "git branch -a")
        self.assert_listed(bash("git branch -av"), "git branch -a")
        self.assert_listed(bash("git branch --all"), "git --all")
        self.assert_clean(bash("git log --oneline"))
        self.assert_clean(bash("git branch --show-current"))

    def test_each_merge_prefix_is_listed(self):
        for prefix in ("32973b96", "735d2daa", "8ad1fc65", "be54d7bc"):
            self.assert_listed(bash("git show %sdeadbeef" % prefix), "names merge " + prefix)

    def test_text_a_call_writes_or_describes_is_not_read(self):
        self.assert_clean(("Write", {"file_path": WORKTREE + "\\a.py", "content": "def main(): pass"}))
        self.assert_clean(("Edit", {"file_path": WORKTREE + "\\a.py", "old_string": "main",
                                    "new_string": "git log main"}))
        self.assert_clean(bash("ls", description="Compare with main"))
        self.assert_clean(("SubagentHandback", {"message": "Nothing touched main."}))

    def test_a_write_outside_the_worktree_is_listed(self):
        self.assert_listed(("Write", {"file_path": CHECKOUT + "\\a.py", "content": "x"}), "path outside")

    def test_other_tools_are_read_field_by_field(self):
        self.assert_listed(("Grep", {"pattern": "x", "path": CHECKOUT}), "path outside")
        self.assert_listed(("PowerShell", {"command": "git log main"}), "names main")
        self.assert_listed(("mcp__x", {"nested": {"args": ["git show main:a.py"]}}), "names main")

    def test_a_call_repeated_in_two_records_is_listed_once(self):
        transcript = os.path.join(self.dir, "agent-twice.jsonl")
        record = {"type": "assistant", "message": {"content": [
            {"type": "tool_use", "id": "same", "name": "Bash", "input": {"command": "git log main"}}]}}
        with open(transcript, "w", encoding="utf-8") as f:
            f.write(json.dumps(record) + "\n" + json.dumps(record) + "\n")
        wave = os.path.join(self.dir, "wave.json")
        with open(wave, "w", encoding="utf-8") as f:
            json.dump([{"transcript": transcript}], f)
        r = subprocess.run([sys.executable, "-I", SCRIPT, wave], capture_output=True, text=True)
        self.assertEqual(r.stdout.splitlines(), ["twice | Bash | names main | git log main",
                                                 "1 calls listed from 1 transcripts"])

    def test_an_unreadable_transcript_fails_loudly(self):
        wave = os.path.join(self.dir, "wave.json")
        with open(wave, "w", encoding="utf-8") as f:
            json.dump([{"transcript": os.path.join(self.dir, "gone.jsonl")}], f)
        r = subprocess.run([sys.executable, "-I", SCRIPT, wave], capture_output=True, text=True)
        self.assertEqual(r.returncode, 2)
        self.assertEqual(r.stdout, "")
        self.assertIn("cannot read the wave", r.stderr)

    def test_a_broken_transcript_line_fails_loudly(self):
        transcript = os.path.join(self.dir, "agent-bad.jsonl")
        with open(transcript, "w", encoding="utf-8") as f:
            f.write("{not json\n")
        wave = os.path.join(self.dir, "wave.json")
        with open(wave, "w", encoding="utf-8") as f:
            json.dump([{"transcript": transcript}], f)
        r = subprocess.run([sys.executable, "-I", SCRIPT, wave], capture_output=True, text=True)
        self.assertEqual(r.returncode, 2)


if __name__ == "__main__":
    unittest.main()
