"""Tests for tools/check_pointers.py, the pointer checker.

Each test builds a small Hydra-like git repo, a gates folder with an old hooks folder
beside it, and a memory folder, all in a temp folder. It writes notes that point at
files, runs the script the way a session does and reads its printed lines and exit code.
Run with: py -I tools/test_check_pointers.py
"""

import json
import os
import shutil
import stat
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
SCRIPT = os.path.join(HERE, "check_pointers.py")


def git(repo, *args):
    env = {k: v for k, v in os.environ.items() if not k.startswith("GIT_")}
    r = subprocess.run(["git", "-C", repo, "-c", "user.name=t", "-c", "user.email=t@t", *args],
                       capture_output=True, text=True, env=env)
    if r.returncode != 0:
        raise RuntimeError("git %s failed: %s" % (" ".join(args), r.stderr))
    return r.stdout.strip()


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", newline="\n", encoding="utf-8") as f:
        f.write(text)


class Base(unittest.TestCase):
    def setUp(self):
        self.tmp = os.path.realpath(tempfile.mkdtemp(prefix="check-pointers-"))
        self.repo = os.path.join(self.tmp, "repo")
        self.claude = os.path.join(self.tmp, "claude")
        self.gates = os.path.join(self.claude, "gates")
        self.memory = os.path.join(self.tmp, "memory")
        write(os.path.join(self.repo, "src", "real.cpp"), "int x;\n")
        write(os.path.join(self.repo, "CLAUDE.md"), "Project notes.\n")
        write(os.path.join(self.repo, "docs", "agents", "guide.md"), "A guide.\n")
        git(self.repo, "init", "-q", "-b", "main")
        git(self.repo, "add", ".")
        git(self.repo, "commit", "-q", "-m", "start")
        write(os.path.join(self.gates, "good_gate.py"), "")
        write(os.path.join(self.gates, "tests", "test_good_gate.py"), "")
        write(os.path.join(self.claude, "hooks", "old_gate.ps1"), "")
        write(os.path.join(self.memory, "MEMORY.md"), "index\n")

    def tearDown(self):
        def writable(func, path, _):  # git marks its object files read-only
            os.chmod(path, stat.S_IWRITE)
            func(path)
        shutil.rmtree(self.tmp, onexc=writable)

    def note(self, text, name="note.md"):
        write(os.path.join(self.memory, name), text)

    def run_check(self, *extra):
        r = subprocess.run([sys.executable, "-I", SCRIPT, "--repo", self.repo, "--memory", self.memory,
                            "--gates", self.gates, *extra], capture_output=True, text=True)
        return r.returncode, r.stdout, r.stderr

    def broken(self, out):
        """The broken-pointer lines, without the summary line."""
        return [line for line in out.splitlines() if line and not line.startswith("pointers:")]


class TestPointers(Base):
    def test_missing_path_reported(self):
        self.note("Title\n\nThe code is in `src/real.cpp`.\nThe old file was `src/gone.cpp`.\n")
        code, out, err = self.run_check()
        lines = self.broken(out)
        self.assertEqual(len(lines), 1, out + err)
        self.assertTrue(lines[0].endswith("note.md:4: src/gone.cpp (missing)"), lines[0])

    def test_hook_names(self):
        self.note("`good_gate.py` and `test_good_gate.py`\n`old_gate.ps1`\n`nowhere.sh`\n")
        code, out, err = self.run_check()
        lines = self.broken(out)
        self.assertFalse(any("good_gate.py" in line for line in lines), out)
        self.assertTrue(any(line.endswith(":2: old_gate.ps1 (retired)") for line in lines), out)
        self.assertTrue(any(line.endswith(":3: nowhere.sh (missing)") for line in lines), out)

    def test_absolute_path(self):
        real = os.path.join(self.repo, "src", "real.cpp")
        gone = os.path.join(self.tmp, "nothing", "here.txt")
        self.note("`%s`\n`%s`\n" % (real, gone))
        code, out, err = self.run_check()
        lines = self.broken(out)
        self.assertEqual(lines, [os.path.join(self.memory, "note.md") + ":2: %s (missing)" % gone], out + err)

    def test_sessions_line_skipped(self):
        self.note("`src/gone.cpp`\nSessions: `x/2026-10-25-13b9cb08.md`, `y/other.md`\n")
        code, out, err = self.run_check()
        lines = self.broken(out)
        self.assertEqual(len(lines), 1, out)
        self.assertIn(":1: src/gone.cpp", lines[0])
        self.assertIn("pointers: 1 checked, 1 broken", out)

    def test_exit_code(self):
        self.note("`src/real.cpp`\n")
        code, out, err = self.run_check()
        self.assertEqual(code, 0, out + err)
        self.assertIn("pointers: 1 checked, 0 broken", out)
        self.note("`src/gone.cpp`\n", name="second.md")
        code, out, err = self.run_check()
        self.assertEqual(code, 1, out + err)
        self.assertIn("pointers: 2 checked, 1 broken", out)

    def test_session_start_output(self):
        self.note("`src/real.cpp`\n")
        code, out, err = self.run_check("--session-start")
        self.assertEqual((code, out), (0, ""), err)
        self.note("`src/gone.cpp`\n", name="second.md")
        code, out, err = self.run_check("--session-start")
        self.assertEqual(code, 0, err)
        lines = out.splitlines()
        self.assertEqual(len(lines), 1, out)
        reply = json.loads(lines[0])
        self.assertEqual(list(reply), ["hookSpecificOutput"])
        self.assertEqual(reply["hookSpecificOutput"]["hookEventName"], "SessionStart")
        self.assertIn("src/gone.cpp (missing)", reply["hookSpecificOutput"]["additionalContext"])
        # A run that cannot check anything (here the repo given is not a git repo; the last
        # --repo wins) says so to the user and still exits 0, so a session never starts on a
        # failed hook.
        code, out, err = self.run_check("--session-start", "--repo", self.memory)
        self.assertEqual(code, 0, err)
        reply = json.loads(out.splitlines()[0])
        self.assertEqual(list(reply), ["systemMessage"])
        self.assertIn("check_pointers.py", reply["systemMessage"])

    def test_wrong_case_is_broken(self):
        self.note("`SRC/Real.cpp`\n")
        code, out, err = self.run_check()
        self.assertEqual(self.broken(out), [os.path.join(self.memory, "note.md") + ":1: SRC/Real.cpp (found at src/real.cpp)"],
                         out + err)

    def test_tail_match_is_never_found(self):
        # Only src/real.cpp exists. A pointer to another folder, or a bare name, that happens to
        # end the same way names a different place, so it is broken; the real file is a hint.
        self.note("`other/real.cpp`\n`real.cpp`\n")
        code, out, err = self.run_check()
        lines = self.broken(out)
        self.assertEqual(len(lines), 2, out)
        self.assertTrue(lines[0].endswith(":1: other/real.cpp (found at src/real.cpp)"), lines[0])
        self.assertTrue(lines[1].endswith(":2: real.cpp (found at src/real.cpp)"), lines[1])
        self.assertEqual(code, 1)

    def test_docs_read_at_any_depth_and_archive_left_out(self):
        write(os.path.join(self.repo, "docs", "agents", "rulings", "topic.md"), "`src/gone_ruling.cpp`\n")
        write(os.path.join(self.repo, "CLAUDE.md"), "`src/gone_claude.cpp`\n")
        write(os.path.join(self.memory, "archive", "old.md"), "`src/gone_archive.cpp`\n")
        code, out, err = self.run_check()
        self.assertIn(os.path.join("docs", "agents", "rulings", "topic.md") + ":1: src/gone_ruling.cpp (missing)", out)
        self.assertIn("CLAUDE.md:1: src/gone_claude.cpp (missing)", out)
        self.assertNotIn("gone_archive", out)

    def test_fenced_block_and_sibling_and_other_repo(self):
        # A bare path in a fenced block is read. A name beside the note, and a path written from
        # the hooks repo's root, both lead somewhere as written.
        self.note("```\nrun src/gone_fenced.cpp now\n```\n`MEMORY.md`\n`gates/good_gate.py`\n")
        code, out, err = self.run_check()
        self.assertEqual(self.broken(out), [os.path.join(self.memory, "note.md") + ":2: src/gone_fenced.cpp (missing)"],
                         out + err)

    def test_not_file_names_are_ignored(self):
        git(self.repo, "branch", "feature/x")
        git(self.repo, "remote", "add", "origin", "https://github.com/owner/proj.git")
        self.note("`feature/x` `HKCU:\\Software\\Thing` `yes` `https://example.com/a.md`\n"
                  "`/handoff` `//Hydra/Label` `C:\\Users\\...` `owner/proj`\n")
        code, out, err = self.run_check()
        self.assertIn("pointers: 0 checked, 0 broken", out, out + err)

    def test_bad_argument(self):
        r = subprocess.run([sys.executable, "-I", SCRIPT, "--memory", os.path.join(self.tmp, "no-such")],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 2, r.stdout + r.stderr)
        self.assertIn("usage", r.stderr)


if __name__ == "__main__":
    unittest.main()
