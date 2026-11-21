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
                        + record("## Records (tests/t.cpp:9)", "x", "no")
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
                        + record("## Records (tests/t.cpp:9)", "NONE", None)})
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

    def test_stops_on_a_row_under_no_record_heading(self):
        self.fx.commit({"A.md": "# A\n\n## Shared answers\n9 Stand-in: None.\n11 Real: Yes.\n\n"
                        + record("### every start checks (tests/t.cpp:4)", "None.", "Yes.")})
        code, lines = self.fx.run()
        self.assertEqual(code, 1)
        self.assertTrue(any(line.startswith("STOP: 1 line 9 rows sit under no record heading")
                            for line in lines), lines)
        self.assertTrue(any(line.startswith("STOP: 1 line 11 rows sit under no record heading")
                            for line in lines), lines)
        self.assertFalse(os.path.exists(self.fx.out))

    def test_counts_match_git_grep(self):
        self.fx.commit({"A.md": record("## every start checks (tests/t.cpp:4)", "None.", "Yes.")
                        + record("## Counts")})
        code, lines = self.fx.run()
        self.assertEqual(code, 0, lines)
        self.assertIn("headings: 1 record headings and 1 section headings; git grep counts 2 "
                      "`##`, `###` and `####` headings", lines)
        self.assertTrue(any(line.startswith("line 9 rows: script 1 on 1 lines, git grep 1 lines;")
                            for line in lines), lines)

    # 1. Record headings at three levels; any other heading is a section.
    def test_record_headings_are_levels_two_to_four_with_a_place(self):
        self.fx.commit({"A.md": "## Records\n\n"
                        + record("### every start checks (tests/t.cpp:4)", "None.", "Yes.")
                        + record("#### helper_rows (tests/t.cpp:1)", "a loop", "no")
                        + "### Shared answers (S-EN)\n\n"})
        code, lines = self.fx.run()
        self.assertEqual(code, 0, lines)
        rows = by_record(self.fx.rows())
        self.assertEqual(rows["A.md:3"]["match"], mr.SAME_PLACE)
        self.assertEqual(rows["A.md:8"]["match"], mr.SAME_PLACE)
        self.assertEqual(set(rows), {"A.md:3", "A.md:8"})
        with open(os.path.join(self.fx.out, "sections.tsv"), encoding="utf-8") as f:
            sections = [line.split("\t")[0] for line in f.read().splitlines()[1:]]
        self.assertEqual(sections, ["A.md:1", "A.md:13"])

    # 2. A leading record number is dropped; a bare number with no full stop is not.
    def test_a_leading_record_number_is_dropped(self):
        self.fx.commit({"A.md": record("### 44. every start checks (tests/t.cpp:4)", "x", "no")
                        + record("### 44 every start checks (tests/t.cpp:4)", "x", "no")})
        code, lines = self.fx.run()
        self.assertEqual(code, 0, lines)
        rows = by_record(self.fx.rows())
        self.assertEqual((rows["A.md:1"]["match"], rows["A.md:1"]["test name"]),
                         (mr.SAME_PLACE, "every start checks"))
        self.assertEqual(rows["A.md:6"]["match"], mr.NO_TEST)

    # 3. A short place takes its file from the nearest heading above that names one.
    def test_a_short_place_takes_the_file_named_above(self):
        self.fx.commit({"A.md": record("### every start checks (:4)", "x", "no")
                        + "# File tests/t.cpp\n\n"
                        + record("### every start checks (:4)", "x", "no")
                        + record("### helper_rows (line 1)", "x", "no")})
        code, lines = self.fx.run()
        self.assertEqual(code, 0, lines)
        rows = by_record(self.fx.rows())
        self.assertEqual((rows["A.md:1"]["match"], rows["A.md:1"]["match note"]),
                         (mr.NO_TEST, "no heading above names a file"))
        self.assertEqual((rows["A.md:8"]["match"], rows["A.md:8"]["test file"]), (mr.SAME_PLACE, "tests/t.cpp"))
        self.assertEqual(rows["A.md:13"]["match"], mr.SAME_PLACE)

    # 4. Names inside the parenthesis, and bare file names.
    def test_bracketed_names_and_bare_file_names(self):
        self.fx.commit({"A.md": record("### paths-list: the list (`helper_rows`, tests/t.cpp:1)", "x", "no")
                        + record("### LaunchTest (test_x.py:1)", "x", "no")
                        + record("### LaunchTest (`test_y.py:1`)", "x", "no")})
        other = os.path.join(self.fx.copy, "code", "tests", "py")
        os.makedirs(other)
        for folder in ("tools", os.path.join("tests", "py")):
            with open(os.path.join(self.fx.copy, "code", folder, "test_y.py"), "w", encoding="utf-8") as f:
                f.write(PY)
        code, lines = self.fx.run()
        self.assertEqual(code, 0, lines)
        rows = by_record(self.fx.rows())
        self.assertEqual((rows["A.md:1"]["match"], rows["A.md:1"]["test name"]), (mr.SAME_PLACE, "helper_rows"))
        self.assertEqual((rows["A.md:6"]["match"], rows["A.md:6"]["test file"]), (mr.SAME_PLACE, "tools/test_x.py"))
        self.assertEqual((rows["A.md:11"]["match"], rows["A.md:11"]["match note"]),
                         (mr.NO_TEST, "bare file name test_y.py names 2 files in the copy"))

    # 5. Each line shape is read; a number inside prose or a numbered list is not.
    def test_line_shapes(self):
        self.fx.commit({"A.md": "### every start checks (tests/t.cpp:4)\n- 9 Stand-in: None.\n- **11 Real:** no\n\n"
                        "### helper_rows (tests/t.cpp:1)\n8 Deleted: 3. 9 none. 10 records 13, 16, 70. 11 yes.\n\n"
                        "### LaunchTest (tools/test_x.py:1)\nRead: 1-3. 1: a claim. 9: a loop. 11: yes, real.\n\n"
                        "### a gone test (tests/gone.cpp:3)\n5 Reason: 9 and 10 straddle the step.\n"
                        "9. Numbers on screen.\n11 No, typed.\n"})
        code, lines = self.fx.run()
        self.assertEqual(code, 0, lines)
        rows = by_record(self.fx.rows())
        self.assertEqual((rows["A.md:1"]["state"], rows["A.md:1"]["flagged by or missing"]), (mr.FLAGGED, "line 9"))
        self.assertEqual(rows["A.md:1"]["line 11"], "A.md:3: no")
        self.assertEqual((rows["A.md:5"]["state"], rows["A.md:5"]["flagged by or missing"]),
                         (mr.FLAGGED, "line 9 and line 11"))
        self.assertEqual(rows["A.md:5"]["line 9"], "A.md:6: none.")
        self.assertEqual((rows["A.md:8"]["state"], rows["A.md:8"]["flagged by or missing"]), (mr.FLAGGED, "line 11"))
        self.assertEqual((rows["A.md:11"]["state"], rows["A.md:11"]["flagged by or missing"]),
                         (mr.SOMETHING_ELSE, "no line 9"))
        self.assertIn("  shape labelled, after '- ', in bold: 1", lines)
        self.assertIn("  shape no label, after another answer on its line: 1", lines)
        self.assertIn("  shape no label, number then ':', inside a 'Read:' line: 1", lines)

    # 6. A row belongs to the nearest record heading above it; two rows make it something else.
    def test_rows_belong_to_the_nearest_record_heading(self):
        self.fx.commit({"A.md": "### every start checks (tests/t.cpp:4)\n9 Stand-in: None.\n"
                        "#### Notes\n9 none.\n11 Real: Yes.\n\n"
                        + record("### helper_rows (tests/t.cpp:1)", "a loop", None)
                        + "#### More notes\n11 Real: Yes.\n"})
        code, lines = self.fx.run()
        self.assertEqual(code, 0, lines)
        rows = by_record(self.fx.rows())
        self.assertEqual((rows["A.md:1"]["state"], rows["A.md:1"]["flagged by or missing"]),
                         (mr.SOMETHING_ELSE, "two or more rows for line 9"))
        self.assertEqual(rows["A.md:1"]["line 9"], "A.md:2: None. | A.md:4: none.")
        self.assertEqual((rows["A.md:7"]["state"], rows["A.md:7"]["flagged by or missing"]), (mr.FLAGGED, "line 11"))

    # 7. A line git grep finds and the reader does not read stops the run.
    def test_stops_when_the_reader_misses_a_line_git_grep_finds(self):
        self.fx.commit({"A.md": record("### every start checks (tests/t.cpp:4)", "None.", "Yes.")
                        + "8-10 as record 2. 9 none.\n"})
        code, lines = self.fx.run()
        self.assertEqual(code, 1)
        self.assertTrue(any(line.startswith("STOP: the script reads line 9 rows on 1 lines, git grep finds 2 lines. "
                                            "Found only by git grep (1): A.md:6") for line in lines), lines)
        self.assertIn("  A.md:6: 8-10 as record 2. 9 none.", lines)


if __name__ == "__main__":
    unittest.main()
