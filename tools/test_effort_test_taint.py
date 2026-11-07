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

    def test_plan_acceptance_lists_first_and_third_only(self):
        first = bash("git show main:src/a.cpp")
        second = bash("git diff main...%s" % KEY)
        third = read(CHECKOUT + "\\src\\a.cpp")
        listed, summary = self.run_script([first, second, third])
        self.assertEqual(listed, [
            "abc123 | Bash | names main | git show main:src/a.cpp",
            "abc123 | Read | path outside the run's worktree | " + CHECKOUT + "\\src\\a.cpp",
        ])
        self.assertEqual(summary, "2 calls listed from 1 transcripts")

    def test_the_range_form_covers_only_the_runs_own_key(self):
        self.assert_listed(bash("git diff main...%s" % ("b" * 40)), "names main")
        self.assert_listed(bash("git diff origin/main...%s" % KEY), "names origin/main")
        self.assert_listed(bash("git diff main...%s" % KEY), "names main", key=None)

    def test_a_second_ref_beside_the_range_is_listed(self):
        self.assert_listed(bash("git diff main...%s; git show main:a.py" % KEY), "names main")

    def test_every_spelling_of_main_is_listed(self):
        for command in ("git log main", "git log HEAD..main", "git log main~2", "git checkout main",
                        "git show refs/heads/main:a.py", "git log origin/main"):
            self.assert_listed(bash(command), "names ")

    def test_file_names_that_hold_main_are_not_listed(self):
        for call in (read(WORKTREE + "\\src\\main.cpp"), bash("py tools/main_window.py"),
                     bash("py -c 'x == \"__main__\"'"), bash("ls my-main")):
            self.assert_clean(call)

    def test_other_branches_are_listed_and_the_runs_own_is_not(self):
        self.assert_listed(bash("git log claude/eff-rev-quiet-2"), "names branch claude/eff-rev-quiet-2")
        self.assert_listed(bash("git log claude/effort-test-results"), "names branch")
        self.assert_listed(bash("git show origin/claude/pbr4-a4d"), "names branch")
        self.assert_listed(bash("git log %s..claude/other" % KEY), "names branch claude/other")
        self.assert_listed(bash("git branch --list claude/*"), "names branch claude/*")
        self.assert_clean(bash("git log %s" % BRANCH))
        self.assert_clean(bash("git log %s:a.py" % BRANCH))

    def test_a_longer_branch_with_the_runs_name_as_a_start_is_listed(self):
        self.assert_listed(bash("git log %s0" % BRANCH), "names branch")

    def test_dot_claude_folders_are_not_branches(self):
        self.assert_clean(bash("ls C:/Users/Patrick/.claude/gates/x.py"))
        self.assert_clean(bash("ls .claude/gates"))

    def test_paths_outside_the_worktree_in_every_spelling(self):
        for path in ("C:/Users/Patrick/Downloads/Hydra/hydra-test/src/a.cpp",
                     "/c/Users/Patrick/Downloads/Hydra/hydra-test/src/a.cpp",
                     "c:\\users\\patrick\\downloads\\hydra\\hydra-test\\src\\a.cpp",
                     "C:\\\\Users\\\\Patrick\\\\Downloads\\\\Hydra\\\\hydra-test\\\\src"):
            self.assert_listed(bash("cat " + path), "path outside")

    def test_a_sibling_worktree_is_outside(self):
        self.assert_listed(read(WORKTREE[:-1] + "2\\src\\a.cpp"), "path outside")
        self.assert_listed(read(WORKTREE + "0\\src\\a.cpp"), "path outside")

    def test_paths_inside_the_worktree_are_not_listed(self):
        self.assert_clean(read(WORKTREE + "\\src\\a.cpp"))
        self.assert_clean(bash("git -C %s status" % WORKTREE.replace("\\", "/")))

    def test_a_reviewers_isolation_copy_counts_as_its_own(self):
        scratch = CHECKOUT + "\\.claude\\worktrees\\agent-1"
        self.assert_clean(read(scratch + "\\docs\\a.md"), scratch=scratch)
        self.assert_listed(read(scratch + "\\docs\\a.md"), "path outside")

    def test_a_run_with_no_worktree_has_every_checkout_path_listed(self):
        self.assert_listed(read(WORKTREE + "\\src\\a.cpp"), "path outside", worktree=None)

    def test_relative_climbs_out_of_the_worktree_are_listed(self):
        self.assert_listed(bash("cat ../../../src/a.cpp"), "climbs")
        self.assert_listed(bash("cat ..\\eff-rev-quiet-2\\src\\a.cpp"), "climbs")
        self.assert_clean(bash("cat ../a.cpp"))

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
