"""Tests for tools/blind_copy/check_facts.py, the facts format check.

Run with: py -I tools/blind_copy/test_check_facts.py
"""

import os
import shutil
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
SCRIPT = os.path.join(HERE, "check_facts.py")
sys.path.insert(0, HERE)

import check_facts as cf  # noqa: E402
import facts_format as ff  # noqa: E402

TEST_CPP = """void helper() {
    hydra::ui::set_view_analyzer_for_test(fake);
}
void test_one() {
    int got = compute(3);
    IM_CHECK(got == 7);
}
""" + "\n" * 50

SECTION = "## test_one\nlocation: code/tests/t.cpp:4\n"
CHECK = ("check: got | 7 | left made at: code/tests/t.cpp:5 | right made at: code/tests/t.cpp:6"
         " | same function: no\n")
BUILT = "input: argument three | source: a value the test builds in code | trace: code/tests/t.cpp:5\n"
REPLACED = (f"input: analyzer | source: {ff.REPLACES} | replaces: hydra::app::analyze"
            " | stands in: throws code/tests/t.cpp:2 | trace: code/tests/t.cpp:4 -> code/tests/t.cpp:1-3\n")
TESTDATA_GOOD = ("input: chart | source: a file under testdata"
                 " | trace: code/tests/t.cpp:5 -> code/testdata/song/notes.chart\n")
NETWORK_GOOD = ("input: reply | source: a real network reply"
                " | trace: code/tests/t.cpp:5 -> code/tests/replies/release.json\n")
CLEAN = SECTION + CHECK + BUILT + REPLACED + TESTDATA_GOOD + NETWORK_GOOD


class Base(unittest.TestCase):
    def setUp(self):
        self.copy = tempfile.mkdtemp(prefix="check_facts_")
        self.addCleanup(shutil.rmtree, self.copy, True)
        for rel, text in (("code/tests/t.cpp", TEST_CPP),
                          ("code/testdata/song/notes.chart", "[Song]\n"),
                          ("code/tests/replies/release.json", "{}\n")):
            path = os.path.join(self.copy, *rel.split("/"))
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "w", encoding="utf-8", newline="\n") as f:
                f.write(text)

    def errors(self, facts_text):
        path = os.path.join(self.copy, "facts.md")
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write(facts_text)
        checker = cf.Checker(self.copy)
        checker.run(path)
        return [why for _, why in checker.errors]

    def assertRejected(self, facts_text, words):
        errs = self.errors(facts_text)
        self.assertTrue(any(words in e for e in errs), f"no error holding {words!r}: {errs}")


class Clean(Base):
    def test_clean_file_passes(self):
        self.assertEqual(self.errors(CLEAN), [])

    def test_clean_file_exits_0_and_bad_exits_1(self):
        good = os.path.join(self.copy, "good.md")
        bad = os.path.join(self.copy, "bad.md")
        with open(good, "w", encoding="utf-8") as f:
            f.write(CLEAN)
        with open(bad, "w", encoding="utf-8") as f:
            f.write(CLEAN + "an aside\n")
        ok = subprocess.run([sys.executable, "-I", SCRIPT, self.copy, good], capture_output=True, text=True)
        self.assertEqual(ok.returncode, 0, ok.stdout + ok.stderr)
        self.assertIn("ok: 1 tests, 1 checks, 4 inputs", ok.stdout)
        no = subprocess.run([sys.executable, "-I", SCRIPT, self.copy, bad], capture_output=True, text=True)
        self.assertEqual(no.returncode, 1)
        self.assertIn("line 8: not one of the four line kinds", no.stdout)


class Testdata(Base):
    def test_good_path(self):
        self.assertEqual(self.errors(SECTION + TESTDATA_GOOD), [])

    def test_no_path(self):
        self.assertRejected(
            SECTION + "input: chart | source: a file under testdata | trace: code/tests/t.cpp:5\n",
            "must cite that path under code/testdata")

    def test_path_not_there(self):
        self.assertRejected(
            SECTION + "input: chart | source: a file under testdata"
            " | trace: code/tests/t.cpp:5 -> code/testdata/song/missing.chart\n",
            "is not in the copy")

    def test_a_tests_path_is_not_testdata(self):
        self.assertRejected(
            SECTION + "input: chart | source: a file under testdata"
            " | trace: code/tests/replies/release.json\n",
            "must cite that path under code/testdata")


class Network(Base):
    def test_good_path(self):
        self.assertEqual(self.errors(SECTION + NETWORK_GOOD), [])

    def test_no_path(self):
        self.assertRejected(
            SECTION + "input: reply | source: a real network reply | trace: code/tests/t.cpp:5\n",
            "must cite its recorded reply file")

    def test_path_not_there(self):
        self.assertRejected(
            SECTION + "input: reply | source: a real network reply"
            " | trace: code/tests/replies/gone.json\n",
            "is not in the copy")

    def test_a_folder_is_no_reply_file(self):
        self.assertRejected(
            SECTION + "input: reply | source: a real network reply | trace: code/tests/replies\n",
            "must cite its recorded reply file")


class Rejections(Base):
    def test_line_of_no_kind(self):
        self.assertRejected(CLEAN + "note: the comment says it is fine\n", "not one of the four line kinds")

    def test_field_with_parentheses(self):
        self.assertRejected(
            SECTION + "input: argument (the three) | source: a value the test builds in code"
            " | trace: code/tests/t.cpp:5\n", "parentheses or brackets")

    def test_field_with_brackets(self):
        self.assertRejected(
            SECTION + "input: argument [3] | source: a value the test builds in code"
            " | trace: code/tests/t.cpp:5\n", "parentheses or brackets")

    def test_aside_after_a_cite(self):
        self.assertRejected(
            SECTION + "input: argument | source: a value the test builds in code"
            " | trace: code/tests/t.cpp:5 (not opened)\n", "is not a cite")

    def test_text_after_last_field_of_input(self):
        self.assertRejected(SECTION + BUILT.rstrip("\n") + " | so it is fine\n", "text after the last field")

    def test_text_after_last_field_of_check(self):
        self.assertRejected(SECTION + CHECK.rstrip("\n") + " | by design\n", "text after the last field")

    def test_source_word_off_the_list(self):
        self.assertRejected(
            SECTION + "input: argument | source: a fake the comment explains | trace: code/tests/t.cpp:5\n",
            "source word is not on the list")

    def test_stand_in_word_off_the_list(self):
        self.assertRejected(
            SECTION + REPLACED.replace("stands in: throws", "stands in: behaves like the real one"),
            "stands in must be a stand-in word")

    def test_missing_same_function(self):
        self.assertRejected(
            SECTION + CHECK.replace(" | same function: no", ""), "missing the field 'same function'")

    def test_cite_file_not_in_copy(self):
        self.assertRejected(SECTION + BUILT.replace("code/tests/t.cpp:5", "code/tests/gone.cpp:5"),
                            "is not in the copy")

    def test_cite_line_not_in_file(self):
        self.assertRejected(SECTION + BUILT.replace("code/tests/t.cpp:5", "code/tests/t.cpp:9999"),
                            "is not in the file")

    def test_range_wider_than_the_limit(self):
        wide = f"code/tests/t.cpp:1-{ff.MAX_RANGE_LINES + 1}"
        self.assertRejected(SECTION + BUILT.replace("code/tests/t.cpp:5", wide), "is wider than")

    def test_range_at_the_limit_passes(self):
        at = f"code/tests/t.cpp:1-{ff.MAX_RANGE_LINES}"
        self.assertEqual(self.errors(SECTION + BUILT.replace("code/tests/t.cpp:5", at)), [])

    def test_check_side_that_is_not_code(self):
        self.assertRejected(SECTION + CHECK.replace("check: got |", "check: got, only when shown |"),
                            "left side is not code")

    def test_same_function_yes_needs_a_line(self):
        self.assertRejected(SECTION + CHECK.replace("same function: no", "same function: yes"),
                            "same function must be")

    def test_same_function_yes_with_a_line_passes(self):
        self.assertEqual(self.errors(SECTION + CHECK.replace(
            "same function: no", "same function: yes code/tests/t.cpp:5")), [])

    def test_section_without_location(self):
        self.assertRejected("## test_one\n" + BUILT, "has no location line")

    def test_bare_cite_outside_tests_and_testdata(self):
        path = os.path.join(self.copy, "code", "src")
        os.makedirs(path)
        self.assertRejected(SECTION + BUILT.replace("code/tests/t.cpp:5", "code/src"),
                            "must name a path under code/testdata or code/tests")

    def test_cite_that_escapes_the_copy(self):
        self.assertRejected(SECTION + BUILT.replace("code/tests/t.cpp:5", "code/tests/../../facts.md:1"),
                            "is not in the copy")


if __name__ == "__main__":
    unittest.main()
