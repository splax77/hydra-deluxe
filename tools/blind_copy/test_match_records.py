"""Tests for match_records.py. Each builds a small git repo of records and a
small stripped copy, then runs the script on them.

    py -I tools/blind_copy/test_match_records.py
"""

import csv
import os
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import match_records as mr  # noqa: E402

FOLDER = "records/"

CPP = """\
void helper_rows() {
}

TEST_CASE("every start checks") {
}

TEST_CASE("update state: a full name with its own colon") {
}
"""

PY = """\
class LaunchTest:
    def test_one(self):
        pass
"""


def record(heading, nine=None, eleven=None):
    lines = [heading, "1 Claim: something."]
    if nine is not None:
        lines.append("9 Stand-in: " + nine)
    if eleven is not None:
        lines.append("11 Real:    " + eleven)
    return "\n".join(lines) + "\n\n"


class Fixture:
    def __init__(self, root):
        self.repo = os.path.join(root, "repo")
        self.copy = os.path.join(root, "copy")
        self.out = os.path.join(root, "out")
        os.makedirs(os.path.join(self.repo, FOLDER))
        os.makedirs(os.path.join(self.copy, "code", "tests"))
        os.makedirs(os.path.join(self.copy, "code", "tools"))
        with open(os.path.join(self.copy, "code", "tests", "t.cpp"), "w", encoding="utf-8") as f:
            f.write(CPP)
        with open(os.path.join(self.copy, "code", "tools", "test_x.py"), "w", encoding="utf-8") as f:
            f.write(PY)
        self.git("init", "-q")
        self.git("config", "user.email", "t@example.invalid")
        self.git("config", "user.name", "t")
        self.git("config", "core.autocrlf", "false")
        self.when = 1700000000

    def git(self, *args, env=None):
        subprocess.run(["git", "-C", self.repo, *args], check=True, capture_output=True, env=env)

    def commit(self, files):
        """Write {name: text} into the records folder and commit them at a
        committer time one hour after the last commit."""
        for name, text in files.items():
            with open(os.path.join(self.repo, FOLDER, name), "w", encoding="utf-8", newline="\n") as f:
                f.write(text)
        self.when += 3600
        env = dict(os.environ, GIT_COMMITTER_DATE=f"{self.when} +0000", GIT_AUTHOR_DATE=f"{self.when} +0000")
        self.git("add", "-A", env=env)
        self.git("commit", "-q", "-m", "records", env=env)

    def run(self):
        lines = []
        code = mr.run(self.out, copy=self.copy, repo=self.repo, records_commit="HEAD",
                      folder=FOLDER, say=lines.append)
        return code, lines

    def rows(self, name="matched.tsv"):
        with open(os.path.join(self.out, name), encoding="utf-8") as f:
            return list(csv.DictReader(f, delimiter="\t"))


def by_record(rows):
    return {r["record"]: r for r in rows}


class MatchRecordsTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.fx = Fixture(self.tmp.name)

    def tearDown(self):
        self.tmp.cleanup()

    def test_each_match_state(self):
        self.fx.commit({"A.md": "# A\n\n"
                        + record("## update state: every start checks (tests/t.cpp:4)", "x", "no")
                        + record("## update state: a full name with its own colon (tests/t.cpp:90)", "x", "no")
                        + record("## helper_rows (tests/t.cpp:1-2)", "x", "no")
                        + record("## LaunchTest (tools/test_x.py:1)", "x", "no")
                        + record("## a gone test (tests/gone.cpp:3)", "x", "no")
                        + record("## Records", "x", "no")
                        + record("## no such test (tests/t.cpp:4)", "x", "no")})
        code, lines = self.fx.run()
        self.assertEqual(code, 0, lines)
        rows = by_record(self.fx.rows())
        got = {k: r["match"] for k, r in rows.items()}
        self.assertEqual(got["A.md:3"], mr.SAME_PLACE)
        self.assertEqual(rows["A.md:3"]["test name"], "every start checks")
        self.assertEqual(got["A.md:8"], mr.MOVED)
        self.assertEqual(rows["A.md:8"]["test name"], "update state: a full name with its own colon")
        self.assertEqual(rows["A.md:8"]["test lines at " + mr.TESTS_COMMIT], "7")
        self.assertEqual(got["A.md:13"], mr.SAME_PLACE)
        self.assertEqual(got["A.md:18"], mr.SAME_PLACE)
        self.assertEqual(got["A.md:23"], mr.GONE)
        self.assertEqual(got["A.md:28"], mr.NO_TEST)
        self.assertEqual(got["A.md:33"], mr.NO_TEST)
        unmatched = {r["record"]: r["match"] for r in self.fx.rows("unmatched.tsv")}
        self.assertEqual(unmatched, {"A.md:23": mr.GONE, "A.md:28": mr.NO_TEST, "A.md:33": mr.NO_TEST})

    def test_flag_states_in_any_letter_case(self):
        self.fx.commit({"A.md": record("## every start checks (tests/t.cpp:4)", "a loop stands in", "YES, the corpus.")
                        + record("## helper_rows (tests/t.cpp:1)", "none.", "a made-up chart")
                        + record("## LaunchTest (tools/test_x.py:1)", "the fake fetcher", "no, typed")
                        + record("## Records", "NONE", None)})
        code, lines = self.fx.run()
        self.assertEqual(code, 0, lines)
        rows = by_record(self.fx.rows())
        self.assertEqual((rows["A.md:1"]["state"], rows["A.md:1"]["flagged by or missing"]), (mr.FLAGGED, "line 11"))
        self.assertEqual((rows["A.md:6"]["state"], rows["A.md:6"]["flagged by or missing"]), (mr.FLAGGED, "line 9"))
        self.assertEqual(rows["A.md:11"]["state"], mr.SOMETHING_ELSE)
        self.assertEqual((rows["A.md:16"]["state"], rows["A.md:16"]["flagged by or missing"]),
                         (mr.SOMETHING_ELSE, "no line 11"))
        self.assertEqual(rows["A.md:11"]["line 9"], "A.md:13: the fake fetcher")

    def test_newest_copy_is_the_last_committed_line_9(self):
        self.fx.commit({"A.md": record("## every start checks (tests/t.cpp:4)", "None.", "Yes.")})
        self.fx.commit({"W.md": record("## every start checks (tests/t.cpp:4)", "a loop", "typed")})
        code, lines = self.fx.run()
        self.assertEqual(code, 0, lines)
        rows = by_record(self.fx.rows())
        self.assertEqual(rows["A.md:1"]["newest copy"], "no")
        self.assertEqual(rows["W.md:1"]["newest copy"], "yes")
        self.assertEqual(rows["A.md:1"]["test state"], mr.SOMETHING_ELSE)

    def test_a_tie_keeps_both_and_takes_the_worse_state(self):
        self.fx.commit({"A.md": record("## helper_rows (tests/t.cpp:1)", "a loop", "typed")})
        self.fx.commit({"B.md": record("## helper_rows (tests/t.cpp:1)", "a loop", "typed")})
        self.fx.commit({"C.md": record("## every start checks (tests/t.cpp:4)", "a loop", "typed")
                        + record("## every start checks (tests/t.cpp:4)", "None.", "typed")})
        code, lines = self.fx.run()
        self.assertEqual(code, 0, lines)
        rows = by_record(self.fx.rows())
        self.assertEqual((rows["C.md:1"]["newest copy"], rows["C.md:6"]["newest copy"]), ("tie", "tie"))
        self.assertEqual(rows["C.md:1"]["test state"], mr.FLAGGED)

    def test_stops_when_blame_cannot_tell_any_copies_apart(self):
        self.fx.commit({"A.md": record("## every start checks (tests/t.cpp:4)", "a loop", "typed")
                        + record("## every start checks (tests/t.cpp:4)", "None.", "typed")})
        code, lines = self.fx.run()
        self.assertEqual(code, 1)
        self.assertTrue(any(line.startswith("STOP: blame gives one time") for line in lines), lines)
        self.assertFalse(os.path.exists(self.fx.out))

    def test_stops_on_a_line_9_no_record_reaches(self):
        self.fx.commit({"A.md": "## Records\n\n"
                        + record("### every start checks (tests/t.cpp:4)", "None.", "Yes.")
                        + record("### helper_rows (tests/t.cpp:1)", "None.", "Yes.")})
        code, lines = self.fx.run()
        self.assertEqual(code, 1)
        self.assertTrue(any("1 line 9 rows sit under no" in line for line in lines), lines)
        self.assertFalse(os.path.exists(self.fx.out))

    def test_counts_match_git_grep(self):
        self.fx.commit({"A.md": record("## every start checks (tests/t.cpp:4)", "None.", "Yes.")
                        + record("## Counts")})
        code, lines = self.fx.run()
        self.assertEqual(code, 0, lines)
        self.assertIn("headings: 2 matched or listed; git grep counts 2 `## ` headings", lines)
        self.assertTrue(any(line.startswith("line 9 rows: script 1, git grep 1;") for line in lines), lines)


if __name__ == "__main__":
    unittest.main()
