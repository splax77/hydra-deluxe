"""Tests for tools/effort_test_faults.py, which scores a reviewer against a fault key.

Each test builds a small git repo shaped like a fault branch (a base, a reference commit, then
one commit per fault), makes a reviewer's commits on top, runs the script and reads its lines.
Run with: py -I tools/test_effort_test_faults.py
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
SCRIPT = os.path.join(HERE, "effort_test_faults.py")

BASE_CALC = "".join("line %d\n" % n for n in range(1, 11))
BOUNDARY = "    return n <= limit  # planted boundary\n"
COMMENT = "# Files named test_*.py are tests.\n# Any other name is not a test.\n"
COPY = "def is_test(n):\n    return n.startswith('test_')\n"


def script():
    """The script as a module, so a test reads a message from the code that writes it."""
    if HERE not in sys.path:
        sys.path.insert(0, HERE)
    import effort_test_faults
    return effort_test_faults


def remove_repo(path):
    """Git marks its object files read-only, which Windows will not delete."""
    def make_writable(remove, target, _):
        os.chmod(target, stat.S_IWRITE)
        remove(target)
    shutil.rmtree(path, onexc=make_writable)


class FaultCheckTest(unittest.TestCase):
    def setUp(self):
        self.repo = tempfile.mkdtemp()
        self.addCleanup(remove_repo, self.repo)
        self.git("init", "-q", "-b", "fault")
        self.base = self.commit({"src/calc.py": BASE_CALC, "src/other.py": "other\n"}, "base")
        calc = BASE_CALC.replace("line 5\n", "line 5\nreference\n")
        self.commit({"src/calc.py": calc}, "reference")
        calc = calc.replace("line 8\n", BOUNDARY)
        f1 = self.commit({"src/calc.py": calc}, "fault one")
        self.calc_after_f1 = calc
        calc = calc.replace("line 2\n", "line 2\n" + COMMENT)
        f2 = self.commit({"src/calc.py": calc}, "fault two")
        f3 = self.commit({"src/other.py": "other\n" + COPY}, "fault three")
        self.calc_at_tip = calc
        self.tip = self.git("rev-parse", "HEAD")
        self.key = os.path.join(self.repo, "fault-key.json")
        self.faults = [
            self.fault("F1", "boundary", "src/calc.py", 9, f1, BOUNDARY.rstrip("\n"), "off by one at the limit"),
            self.fault("F2", "comment", "src/calc.py", 3, f2, "# Any other name is not a test.",
                       "a comment restates the test file rule"),
            self.fault("F3", "copy", "src/other.py", 2, f3, "    return n.startswith('test_')",
                       "a second copy of the test file rule"),
        ]
        self.write_key()
        self.git("switch", "-q", "-c", "review")

    def fault(self, fault_id, kind, path, line, commit, planted_text, what):
        return {"id": fault_id, "kind": kind, "file": path, "line": line, "commit": commit,
                "planted_text": planted_text, "what": what}

    def write_key(self):
        with open(self.key, "w", encoding="utf-8") as f:
            json.dump({"task": "t", "base": self.base, "tip": self.tip, "faults": self.faults}, f)

    def git(self, *args):
        r = subprocess.run(["git", "-C", self.repo, "-c", "user.name=t", "-c", "user.email=t@t", *args],
                           capture_output=True, text=True)
        if r.returncode != 0:
            raise RuntimeError("git %s failed: %s" % (" ".join(args), r.stderr))
        return r.stdout.strip()

    def commit(self, files, subject):
        for name, text in files.items():
            path = os.path.join(self.repo, name)
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "w", newline="\n") as f:
                f.write(text)
            self.git("add", name)
        self.git("commit", "-q", "-m", subject)
        return self.git("rev-parse", "HEAD")

    def score(self, report="", tip="review"):
        path = os.path.join(self.repo, "report.txt")
        with open(path, "w", encoding="utf-8") as f:
            f.write(report)
        r = subprocess.run([sys.executable, "-I", SCRIPT, self.key, self.repo, tip, path],
                           capture_output=True, text=True)
        return r.returncode, r.stdout.splitlines(), r.stderr

    def lines(self, report="", tip="review"):
        code, lines, err = self.score(report, tip)
        self.assertEqual(code, 0, err)
        return lines

    def test_a_hand_fix_of_one_of_three_faults_is_one_fixed_two_missed(self):
        fixed = self.calc_at_tip.replace(BOUNDARY, "    return n < limit\n")
        self.commit({"src/calc.py": fixed}, "fix the boundary")
        self.assertEqual(self.lines(), [
            "F1 fixed: src/calc.py line 9",
            "F2 missed: src/calc.py line 3",
            "F3 missed: src/other.py line 2",
            "1 fixed, 0 named, 2 missed, 0 false findings",
            "run: fail",
        ])

    def test_no_commits_and_an_empty_report_misses_all_three(self):
        self.assertEqual(self.lines()[-2:], ["0 fixed, 0 named, 3 missed, 0 false findings", "run: fail"])

    def test_every_fault_caught_passes_the_run(self):
        self.commit({"src/calc.py": self.calc_at_tip.replace(BOUNDARY, "x\n").replace(COMMENT, ""),
                     "src/other.py": "other\n"}, "fix all")
        report = "other.py line 2 holds the copy."
        self.assertEqual(self.lines(report)[-2:], ["3 fixed, 0 named, 0 missed, 0 false findings", "run: pass"])

    def test_named_by_file_and_key_line(self):
        for report in ("src/calc.py:9 is off by one.", "In src\\calc.py, line 9 is wrong.",
                       "calc.py L9 is wrong.", "SRC/CALC.PY at 9."):
            self.assertEqual(self.lines(report)[0], "F1 named: src/calc.py line 9", report)

    def test_named_by_the_line_the_fault_has_at_the_tip(self):
        # F2 pushed F1 down two lines, so a reviewer reading the tip sees it at 11, not 9.
        self.assertEqual(self.lines("calc.py:11 is off by one.")[0], "F1 named: src/calc.py line 9")

    def test_named_by_the_words_of_the_fault(self):
        self.assertEqual(self.lines("In calc.py there is an\nOff by one at the LIMIT.")[0],
                         "F1 named: src/calc.py line 9")

    def test_named_by_the_planted_text(self):
        self.assertEqual(self.lines("calc.py has `return n <= limit  # planted boundary`.")[0],
                         "F1 named: src/calc.py line 9")

    def test_a_file_alone_does_not_name_a_fault(self):
        # A miss whose file the report mentions carries score_fault's note, so the main session
        # reads that paragraph; a miss with no mention of the file carries none.
        report = "calc.py looks fine."
        result, note = script().score_fault(self.faults[0], self.calc_at_tip, self.calc_at_tip, report)
        self.assertEqual(result, "missed")
        self.assertNotEqual(note, "")
        self.assertEqual(script().score_fault(self.faults[0], self.calc_at_tip, self.calc_at_tip, "")[1], "")
        self.assertEqual(self.lines(report)[0], "F1 missed: src/calc.py line 9" + note)

    def test_a_line_with_no_file_does_not_name_a_fault(self):
        self.assertEqual(self.lines("Line 9 is off by one at the limit.")[0], "F1 missed: src/calc.py line 9")

    def test_a_paragraph_about_one_fault_does_not_name_another_in_the_same_file(self):
        lines = self.lines("calc.py line 3 restates a rule.\n\nThe number 9 is fine.\n")
        self.assertEqual(lines[0][:9], "F1 missed")
        self.assertEqual(lines[1], "F2 named: src/calc.py line 3")

    def test_a_longer_file_name_is_not_the_faults_file(self):
        self.assertEqual(self.lines("mycalc.py line 9 is wrong.")[0], "F1 missed: src/calc.py line 9")

    def test_a_number_inside_a_longer_number_is_not_the_line(self):
        for report in ("calc.py line 19.", "calc.py took 9ms.", "calc.py 1.9"):
            self.assertEqual(self.lines(report)[0][:9], "F1 missed", report)

    def test_fixed_wins_over_named(self):
        self.commit({"src/calc.py": self.calc_at_tip.replace(BOUNDARY, "x\n")}, "fix")
        self.assertEqual(self.lines("calc.py:9 was off by one.")[0], "F1 fixed: src/calc.py line 9")

    def test_fixing_the_planted_copy_counts_when_the_same_text_stands_elsewhere(self):
        effort_test_faults = script()
        fault = dict(self.faults[0], planted_text="return n <= limit")
        twice = "a\nreturn n <= limit\nb\nreturn n <= limit\n"
        once = "a\nreturn n < limit\nb\nreturn n <= limit\n"
        self.assertEqual(effort_test_faults.score_fault(fault, once, twice, "")[0], "fixed")
        self.assertEqual(effort_test_faults.score_fault(fault, twice, twice, "")[0], "missed")

    def test_removing_the_faults_file_counts_as_fixed(self):
        self.git("rm", "-q", "src/other.py")
        self.git("commit", "-q", "-m", "remove")
        lines = self.lines()
        self.assertEqual(lines[2], "F3 fixed: src/other.py line 2")

    def test_an_edit_away_from_planted_lines_is_a_false_finding(self):
        self.commit({"src/calc.py": self.calc_at_tip.replace("line 4\n", "changed\n")}, "stray")
        lines = self.lines()
        self.assertIn("false finding: src/calc.py line 6", lines)
        self.assertEqual(lines[-2], "0 fixed, 0 named, 3 missed, 1 false findings")

    def test_neighbouring_changed_lines_are_one_false_finding(self):
        text = self.calc_at_tip.replace("line 3\nline 4\n", "changed\nchanged too\n")
        self.commit({"src/calc.py": text}, "stray")
        self.assertEqual(self.lines()[-2], "0 fixed, 0 named, 3 missed, 1 false findings")

    def test_an_edit_on_a_reference_line_is_a_false_finding(self):
        self.commit({"src/calc.py": self.calc_at_tip.replace("reference\n", "changed\n")}, "stray")
        self.assertEqual(self.lines()[-2], "0 fixed, 0 named, 3 missed, 1 false findings")

    def test_an_edit_in_a_file_with_no_fault_is_a_false_finding(self):
        self.commit({"src/new.py": "x\n"}, "new file")
        self.assertIn("false finding: src/new.py line 0", self.lines())

    def test_removing_a_whole_multi_line_fault_is_no_false_finding(self):
        self.commit({"src/calc.py": self.calc_at_tip.replace(COMMENT, ""),
                     "src/other.py": "other\n"}, "remove both")
        self.assertEqual(self.lines()[-2], "2 fixed, 0 named, 1 missed, 0 false findings")

    def test_a_line_added_beside_a_planted_line_is_no_false_finding(self):
        text = self.calc_at_tip.replace(BOUNDARY, "# why\n" + BOUNDARY)
        self.commit({"src/calc.py": text}, "note the fault")
        self.assertEqual(self.lines()[-2], "0 fixed, 0 named, 3 missed, 0 false findings")

    def test_removed_lines_that_look_like_diff_headers_are_not_read_as_headers(self):
        self.git("switch", "-q", "fault")
        self.tip = self.commit({"src/other.py": "other\n" + COPY + "-- a/x\n++ b/y\n"}, "odd lines")
        self.write_key()
        self.git("switch", "-q", "-C", "review")
        self.commit({"src/other.py": "other\n" + COPY}, "drop them")
        lines = self.lines()
        self.assertEqual(lines[-3:-2], ["false finding: src/other.py line 4"])
        self.assertEqual(lines[-2], "0 fixed, 0 named, 3 missed, 1 false findings")

    def assert_cannot_score(self, tip, *names):
        """The run exits 2 with no score and prints the script's own refusal, which names `names`."""
        faults = script()
        with self.assertRaises(faults.CannotScore) as refused:
            faults.score(faults.load_key(self.key), self.repo, tip, "")
        message = str(refused.exception)
        for name in names:
            self.assertIn(name, message)
        code, lines, err = self.score(tip=tip)
        self.assertEqual(code, 2)
        self.assertEqual(lines, [])
        self.assertIn(message, err)

    def test_a_planted_text_missing_at_the_tip_fails_loudly(self):
        self.faults[0]["planted_text"] = "not in the file"
        self.write_key()
        self.assert_cannot_score("review", "F1", "planted_text", "src/calc.py")

    def test_an_empty_key_field_fails_loudly(self):
        self.faults[1]["what"] = ""
        self.write_key()
        self.assert_cannot_score("review", "F2", "what")

    def test_a_tip_that_does_not_descend_from_the_key_fails_loudly(self):
        self.assert_cannot_score(self.base, self.base, self.tip)

    def test_an_unknown_tip_fails_loudly(self):
        self.assert_cannot_score("no-such-branch", "no-such-branch")


if __name__ == "__main__":
    unittest.main()
