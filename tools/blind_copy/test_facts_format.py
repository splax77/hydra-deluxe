"""Tests for tools/blind_copy/facts_format.py, the facts format's one home.

Run with: py -I tools/blind_copy/test_facts_format.py
"""

import os
import subprocess
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
SCRIPT = os.path.join(HERE, "facts_format.py")
sys.path.insert(0, HERE)

import facts_format as ff  # noqa: E402


class Format(unittest.TestCase):
    def test_seven_source_words_and_six_stand_in_words(self):
        self.assertEqual(len(ff.SOURCE_WORDS), 7)
        self.assertEqual(len(ff.STAND_IN_WORDS), 6)

    def test_printed_text_names_every_word(self):
        r = subprocess.run([sys.executable, "-I", SCRIPT], capture_output=True, text=True)
        self.assertEqual(r.returncode, 0)
        for word in ff.SOURCE_WORDS + ff.STAND_IN_WORDS:
            with self.subTest(word=word):
                self.assertIn(f"  {word}\n", r.stdout)
        for kind in ("## ", "location: ", "check: ", "input: ", "replaces: ", "stands in: ", "same function: "):
            self.assertIn(kind, r.stdout)

    def test_cite_forms(self):
        self.assertEqual(ff.parse_cite("code/tests/a.cpp:12").kind, "line")
        r = ff.parse_cite("code/tests/a.cpp:12-20")
        self.assertEqual((r.a, r.b), (12, 20))
        self.assertTrue(r.covers("code/tests/a.cpp", 15))
        self.assertFalse(r.covers("code/tests/a.cpp", 21))
        self.assertEqual(ff.parse_cite("code/testdata/x y (z)/notes.mid").kind, "bare")
        self.assertEqual(ff.parse_cite("library").kind, "word")
        self.assertEqual(ff.parse_cite("cannot tell").kind, "word")
        self.assertIsNone(ff.parse_cite("tests/a.cpp:12"))
        self.assertIsNone(ff.parse_cite("code/tests/a.cpp:12 (aside)"))

    def test_cites_anywhere_reads_older_lines(self):
        line = "input: x | source: y | trace: code/a.cpp:3 -> code/b.cpp:4-6 (an aside) ; code/c.cpp:9"
        self.assertEqual([c.text for c in ff.cites_anywhere(line)],
                         ["code/a.cpp:3", "code/b.cpp:4-6", "code/c.cpp:9"])


if __name__ == "__main__":
    unittest.main()
