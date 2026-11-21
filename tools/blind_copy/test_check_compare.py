"""Tests for tools/blind_copy/check_compare.py, the comparison format check.

Run with: py -I tools/blind_copy/test_check_compare.py
"""

import os
import shutil
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
SCRIPT = os.path.join(HERE, "check_compare.py")
sys.path.insert(0, HERE)

import check_compare as cc  # noqa: E402

SOURCE = [
    "void test_a() {",
    "    // the reply comes from a recorded server",
    '    CHECK(state == "Done");',
    "}",
    "void test_b() {",
    "}",
]
RECORD = [
    "# record",
    "11 Real: Yes, the comment says the reply is real.",
]
FACTS = [
    "## test_a",
    "location: code/tests/t.cpp:1",
    "input: reply | source: a production function the test replaces | trace: code/tests/t.cpp:2",
    "",
    "## test_b",
    "location: code/tests/t.cpp:5",
]

A_CLAIM = ('claim: "the reply comes from a recorded server" | from: comment tests/t.cpp:2'
           ' | rests on: nothing | facts: 3 | result: the facts contradict this')
REC_CLAIM = ('claim: "the comment says the reply is real" | from: record records/A1.md:2'
             ' | rests on: a comment | facts: none | result: no fact backs this')


def count(contra, rest, total):
    return ("count: records the facts contradict: %d | records resting on a comment, name or"
            " claim word: %d | total: %d" % (contra, rest, total))


def good_lines():
    return ["## test_a", A_CLAIM, REC_CLAIM,
            'open: "the comment says the reply is real" | settled by: grep testdata for a recorded reply',
            "", "## test_b", "claims: none", "", count(0, 1, 1)]


class Base(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp()
        self.root = os.path.join(self.tmp, "main")
        os.makedirs(os.path.join(self.root, "tests"))
        os.makedirs(os.path.join(self.root, "records"))
        self.write(os.path.join(self.root, "tests", "t.cpp"), SOURCE)
        self.write(os.path.join(self.root, "records", "A1.md"), RECORD)
        self.facts = os.path.join(self.tmp, "facts.md")
        self.write(self.facts, FACTS)
        self.compare = os.path.join(self.tmp, "compare.md")

    def tearDown(self):
        shutil.rmtree(self.tmp)

    @staticmethod
    def write(path, lines):
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write("\n".join(lines) + "\n")

    def run_check(self, lines):
        self.write(self.compare, lines)
        return cc.check(self.compare, self.facts, self.root)

    def assertRejects(self, lines, fragment):
        got = self.run_check(lines)
        reasons = [r for _, r in got]
        self.assertTrue(any(fragment in r for r in reasons),
                        "no rejection holding %r; got %r" % (fragment, reasons))

    def with_row(self, row, at=1):
        lines = good_lines()
        lines[at] = row
        return lines


class Clean(Base):
    def test_clean_file_passes(self):
        self.assertEqual(self.run_check(good_lines()), [])

    def test_cli_exit_codes(self):
        self.write(self.compare, good_lines())
        ok = subprocess.run([sys.executable, "-I", SCRIPT, self.compare, self.facts, self.root],
                            capture_output=True, text=True)
        self.assertEqual(ok.returncode, 0, ok.stdout + ok.stderr)
        self.write(self.compare, good_lines() + ["a prose aside"])
        bad = subprocess.run([sys.executable, "-I", SCRIPT, self.compare, self.facts, self.root],
                             capture_output=True, text=True)
        self.assertEqual(bad.returncode, 1)
        self.assertIn("compare.md:", bad.stdout)

    def test_cli_requires_main_root(self):
        self.write(self.compare, good_lines())
        r = subprocess.run([sys.executable, "-I", SCRIPT, self.compare, self.facts],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 2)


class Rows(Base):
    def test_missing_field(self):
        row = A_CLAIM.replace(" | rests on: nothing", "")
        self.assertRejects(self.with_row(row), "missing field 'rests on'")

    def test_missing_result_field(self):
        row = A_CLAIM.replace(" | result: the facts contradict this", "")
        self.assertRejects(self.with_row(row), "missing field 'result'")

    def test_result_outside_the_three_phrases(self):
        row = A_CLAIM.replace("the facts contradict this", "partly backed")
        self.assertRejects(self.with_row(row), "is not one of")

    def test_text_after_result_as_a_field(self):
        self.assertRejects(self.with_row(A_CLAIM + " | see the gate"), "text after the last field")

    def test_text_after_result_in_the_field(self):
        self.assertRejects(self.with_row(A_CLAIM + ", mostly"), "text after the result")

    def test_facts_line_past_the_end(self):
        row = A_CLAIM.replace("facts: 3", "facts: 99")
        self.assertRejects(self.with_row(row), "facts line 99 is not in the facts file")

    def test_facts_range_past_the_end(self):
        row = A_CLAIM.replace("facts: 3", "facts: 5-7")
        self.assertRejects(self.with_row(row), "facts line 7 is not in the facts file")

    def test_facts_zero(self):
        row = A_CLAIM.replace("facts: 3", "facts: 0")
        self.assertRejects(self.with_row(row), "facts line 0 is not in the facts file")

    def test_facts_list_and_range_pass(self):
        row = A_CLAIM.replace("facts: 3", "facts: 2, 5-6")
        self.assertEqual(self.run_check(self.with_row(row)), [])

    def test_quote_not_at_its_line(self):
        row = A_CLAIM.replace("tests/t.cpp:2", "tests/t.cpp:3")
        self.assertRejects(self.with_row(row), "quote not found at tests/t.cpp:3")

    def test_quote_reworded(self):
        row = A_CLAIM.replace("from a recorded server", "from the server")
        self.assertRejects(self.with_row(row), "quote not found")

    def test_quote_in_a_file_not_on_main(self):
        row = A_CLAIM.replace("tests/t.cpp:2", "tests/gone.cpp:2")
        self.assertRejects(self.with_row(row), "is not on main")

    def test_quote_past_the_end_of_its_file(self):
        row = A_CLAIM.replace("tests/t.cpp:2", "tests/t.cpp:40")
        self.assertRejects(self.with_row(row), "past the end of the file")

    def test_quote_path_leaving_main(self):
        row = A_CLAIM.replace("tests/t.cpp:2", "../facts.md:1")
        self.assertRejects(self.with_row(row), "leaves main's root")

    def test_quote_with_its_own_double_quotes(self):
        row = ('claim: "CHECK(state == "Done");" | from: message tests/t.cpp:3'
               ' | rests on: nothing | facts: 3 | result: the facts back this')
        self.assertEqual(self.run_check(self.with_row(row)), [])

    def test_from_kind_off_the_list(self):
        row = A_CLAIM.replace("from: comment", "from: guess")
        self.assertRejects(self.with_row(row), "from field")

    def test_rests_on_off_the_list(self):
        row = A_CLAIM.replace("rests on: nothing", "rests on: a decision")
        self.assertRejects(self.with_row(row), "rests on 'a decision'")

    def test_back_with_facts_none(self):
        row = A_CLAIM.replace("facts: 3 | result: the facts contradict this",
                              "facts: none | result: the facts back this")
        self.assertRejects(self.with_row(row), "with facts: none")

    def test_contradict_with_facts_none(self):
        row = A_CLAIM.replace("facts: 3", "facts: none")
        self.assertRejects(self.with_row(row), "with facts: none")

    def test_unbacked_with_facts_lines(self):
        row = REC_CLAIM.replace("facts: none", "facts: 3")
        self.assertRejects(self.with_row(row, at=2), "names facts lines")

    def test_unquoted_claim(self):
        row = A_CLAIM.replace('"the reply comes from a recorded server"',
                              "the reply comes from a recorded server")
        self.assertRejects(self.with_row(row), "double-quoted quote")


class Lines(Base):
    def test_prose_aside(self):
        lines = good_lines()
        lines.insert(2, "side note: the gate never opens")
        self.assertRejects(lines, "not one of the five line kinds")

    def test_title_line_before_sections(self):
        self.assertRejects(["# Pilot comparison"] + good_lines(), "outside any")

    def test_verdict_words_outside_quotes(self):
        for word in ("true", "holds", "consistent", "correct", "True"):
            with self.subTest(word=word):
                row = 'open: "the comment says the reply is real" | settled by: %s here' % word
                lines = good_lines()
                lines[3] = row
                self.assertRejects(lines, "the word '%s' outside a quote" % word)

    def test_verdict_word_inside_a_quote_passes(self):
        lines = list(good_lines())
        self.write(os.path.join(self.root, "tests", "t.cpp"),
                   SOURCE[:1] + ["    // the reply holds a recorded server"] + SOURCE[2:])
        lines[1] = A_CLAIM.replace("comes from", "holds")
        self.assertEqual(self.run_check(lines), [])

    def test_verdict_word_as_part_of_a_name_passes(self):
        self.assertNotRegex("test_true_case holdsx", cc.VERDICT_WORDS)

    def test_test_in_facts_with_no_section(self):
        lines = good_lines()
        del lines[5:7]
        self.assertRejects(lines, "test 'test_b' in the facts file has no section")

    def test_section_with_no_rows(self):
        lines = good_lines()
        del lines[6]
        self.assertRejects(lines, "has no claim row and no 'claims: none'")

    def test_claims_none_beside_a_claim(self):
        lines = good_lines()
        lines.insert(3, "claims: none")
        self.assertRejects(lines, "also has claim rows")

    def test_open_row_naming_no_claim(self):
        lines = good_lines()
        lines[3] = 'open: "some other claim" | settled by: a run'
        self.assertRejects(lines, "is not the quote of a claim row")

    def test_open_row_without_its_search(self):
        lines = good_lines()
        lines[3] = 'open: "the comment says the reply is real" | settled by: '
        self.assertRejects(lines, "open row")

    def test_test_not_in_facts(self):
        lines = good_lines()
        lines.insert(-1, "## test_c")
        lines.insert(-1, "claims: none")
        self.assertRejects(lines, "test 'test_c' is not in the facts file")


class Counts(Base):
    def test_wrong_contradict_count(self):
        lines = good_lines()
        lines[-1] = count(1, 1, 2)
        self.assertRejects(lines, "count of records the facts contradict is 1; the rows give 0")

    def test_wrong_rests_count(self):
        lines = good_lines()
        lines[-1] = count(0, 0, 0)
        self.assertRejects(lines, "resting on a comment, name or claim word is 0; the rows give 1")

    def test_wrong_total(self):
        lines = good_lines()
        lines[-1] = count(0, 1, 3)
        self.assertRejects(lines, "total is 3; the rows give 1")

    def test_recount_counts_a_contradicted_record(self):
        lines = good_lines()
        lines[2] = REC_CLAIM.replace("facts: none | result: no fact backs this",
                                     "facts: 3 | result: the facts contradict this")
        lines[-1] = count(1, 1, 2)
        self.assertEqual(self.run_check(lines), [])

    def test_no_count_line(self):
        self.assertRejects(good_lines()[:-1], "no count line")

    def test_count_line_missing_a_count(self):
        lines = good_lines()
        lines[-1] = "count: records the facts contradict: 0 | total: 1"
        self.assertRejects(lines, "count line: ")

    def test_line_after_the_count_line(self):
        lines = good_lines() + ["claims: none"]
        self.assertRejects(lines, "line after the count line")


if __name__ == "__main__":
    unittest.main()
