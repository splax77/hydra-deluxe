"""Tests for tools/effort_test_scout.py, which checks scout answers against the key.

Run with: py -I tools/test_effort_test_scout.py
"""

import importlib.util
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
SCRIPT = os.path.join(HERE, "effort_test_scout.py")

_spec = importlib.util.spec_from_file_location("effort_test_scout", SCRIPT)
scout = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(scout)

PATH = "tools/check_pointers.py"
LINE = "    MAX_CALLS = 150"


class ScoutCheckTest(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, self.dir)
        self.questions = os.path.join(self.dir, "questions.json")
        with open(self.questions, "w", encoding="utf-8") as f:
            json.dump([{"id": "Q1", "question": "where?", "path": PATH, "line_text": LINE},
                       {"id": "Q2", "question": "empty", "path": PATH, "line_text": "  "}], f)

    def run_script(self, *answers, question="Q1"):
        paths = []
        for i, text in enumerate(answers):
            paths.append(os.path.join(self.dir, "a%d.txt" % i))
            with open(paths[-1], "w", encoding="utf-8") as f:
                f.write(text)
        r = subprocess.run([sys.executable, "-I", SCRIPT, self.questions, question, *paths],
                           capture_output=True, text=True)
        return r.returncode, [line.split(": ", 1)[-1] for line in r.stdout.splitlines()], r.stderr

    def result(self, text):
        code, lines, err = self.run_script(text)
        self.assertEqual(code, 0, err)
        return lines[0]

    def test_path_and_quoted_line_pass(self):
        self.assertEqual(self.result("It is in `%s`:\n\n`MAX_CALLS = 150`\n" % PATH),
                         "pass (path found, line found)")

    def test_an_absolute_windows_path_passes(self):
        self.assertEqual(self.result("C:\\repo\\tools\\check_pointers.py has MAX_CALLS = 150"),
                         "pass (path found, line found)")

    def test_a_wrong_path_fails(self):
        self.assertEqual(self.result("tools/check_handoff.py has MAX_CALLS = 150"),
                         "fail (path not found, line found)")

    def test_a_wrong_line_fails(self):
        self.assertEqual(self.result("%s has MAX_CALLS = 151" % PATH), "fail (path found, line not found)")

    def test_a_longer_file_name_is_not_the_path(self):
        for path in ("mytools/check_pointers.py", "tools/check_pointers.pyc", "tools/check_pointers.py.bak"):
            self.assertEqual(self.result("%s has MAX_CALLS = 150" % path), "fail (path not found, line found)")

    def test_the_match_is_case_exact(self):
        self.assertEqual(self.result("%s has max_calls = 150" % PATH), "fail (path found, line not found)")
        self.assertEqual(self.result("TOOLS/check_pointers.py has MAX_CALLS = 150"),
                         "fail (path not found, line found)")

    def test_each_answer_file_gets_a_line(self):
        code, lines, _ = self.run_script("%s MAX_CALLS = 150" % PATH, "no idea")
        self.assertEqual(code, 0)
        self.assertEqual(lines, ["pass (path found, line found)", "fail (path not found, line not found)"])

    def refusal(self, question):
        """The message find_question() refuses question with; it owns the wording."""
        with self.assertRaises(ValueError) as caught:
            scout.find_question(self.questions, question)
        return str(caught.exception)

    def test_an_unknown_question_fails_loudly(self):
        code, lines, err = self.run_script("x", question="Q9")
        self.assertEqual((code, lines), (2, []))
        message = self.refusal("Q9")
        self.assertIn("Q9", message)  # names the id that was asked for
        self.assertIn(self.questions, message)  # and the file it was looked for in
        self.assertIn(message, err)

    def test_an_empty_key_fails_loudly(self):
        code, lines, err = self.run_script("x", question="Q2")
        self.assertEqual((code, lines), (2, []))
        message = self.refusal("Q2")
        self.assertIn("Q2", message)  # names the question whose key is empty
        self.assertIn(message, err)
        # The refusal is for the empty key: Q1, whose key is filled, is found.
        self.assertEqual(scout.find_question(self.questions, "Q1")["id"], "Q1")

    def test_a_missing_answer_file_fails_loudly(self):
        r = subprocess.run([sys.executable, "-I", SCRIPT, self.questions, "Q1",
                            os.path.join(self.dir, "gone.txt")], capture_output=True, text=True)
        self.assertEqual(r.returncode, 2)


if __name__ == "__main__":
    unittest.main()
