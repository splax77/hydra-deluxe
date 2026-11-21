"""Tests for check_traces.py, on a made-up copy in a temporary folder.

    py -I tools/blind_copy/test_check_traces.py
"""

import contextlib
import io
import os
import shutil
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import check_traces as ct  # noqa: E402

REPO = os.path.dirname(os.path.dirname(HERE))
BRIEF = os.path.join(REPO, "docs", "audit", "2026-11-20-script-first-recheck", "trace-brief.md")

KIND_A = "set_clock_for_test (tests/t.cpp)"
KIND_B = "ScopedHook (tests/u.cpp)"

KINDS_MD = f"""# Kinds

## {KIND_A}
hits: 1
tests: one
hit lines: tests/t.cpp:2

## {KIND_B}
hits: 1
tests: two
hit lines: tests/u.cpp:1
"""

TRACE_A = (f"## {KIND_A}\n"
           "returns: the content of a reply file | cites: code/tests/t.cpp:2 ; code/tests/t.cpp:3-4\n"
           "read from: code/tests/replies/r.json | cites: code/tests/t.cpp:3\n"
           "changes before use: the tag field | cites: code/tests/t.cpp:4\n")
NOT_TRACED_B = f"## {KIND_B}\nnot traced: reached my limit\n"


class CheckTracesTest(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp(prefix="check_traces_")
        self.copy = os.path.join(self.dir, "copy")
        for rel, text in (("code/tests/t.cpp", "a\nb\nc\nd\n"),
                          ("code/tests/u.cpp", "a\nb\n"),
                          ("code/tests/replies/r.json", "{}\n"),
                          ("code/testdata/song/notes.chart", "[Song]\n")):
            path = os.path.join(self.copy, *rel.split("/"))
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "w", encoding="utf-8") as f:
                f.write(text)
        self.kinds = self.write("kinds.md", KINDS_MD)

    def tearDown(self):
        shutil.rmtree(self.dir)

    def write(self, name, text):
        path = os.path.join(self.dir, name)
        with open(path, "w", encoding="utf-8") as f:
            f.write(text)
        return path

    def errors(self, *traces):
        paths = [self.write(f"traces-{i}.md", t) for i, t in enumerate(traces, 1)]
        errors, _, _ = ct.check(self.copy, self.kinds, paths)
        return errors

    def assertPasses(self, *traces):
        self.assertEqual(self.errors(*traces), [])

    def assertFails(self, needle, *traces):
        errors = self.errors(*traces)
        self.assertTrue(any(needle in e for e in errors),
                        f"no error holding {needle!r} in {errors!r}")

    def test_whole_traces_pass(self):
        self.assertPasses(TRACE_A + "\n" + NOT_TRACED_B)

    def test_traces_may_be_split_across_files(self):
        self.assertPasses(TRACE_A, NOT_TRACED_B)

    def test_two_cases_pass(self):
        second = ("returns: an error it throws | cites: code/tests/t.cpp:1\n"
                  "read from: none | cites: code/tests/t.cpp:1\n"
                  "changes before use: nothing | cites: code/tests/t.cpp:1\n")
        self.assertPasses(TRACE_A + second + NOT_TRACED_B)

    def test_cite_to_a_missing_line_fails(self):
        self.assertFails("is not in the file, which has 4 lines",
                         TRACE_A.replace("code/tests/t.cpp:4\n", "code/tests/t.cpp:5\n") + NOT_TRACED_B)

    def test_range_past_the_end_fails(self):
        self.assertFails("is not in the file",
                         TRACE_A.replace("t.cpp:3-4", "t.cpp:3-9") + NOT_TRACED_B)

    def test_line_zero_fails(self):
        self.assertFails("is not in the file",
                         TRACE_A.replace("code/tests/t.cpp:4\n", "code/tests/t.cpp:0\n") + NOT_TRACED_B)

    def test_cite_to_a_missing_file_fails(self):
        self.assertFails("is not in the copy",
                         TRACE_A.replace("code/tests/t.cpp:4\n", "code/tests/gone.cpp:1\n") + NOT_TRACED_B)

    def test_cite_without_code_prefix_fails(self):
        self.assertFails("is not a line cite",
                         TRACE_A.replace("code/tests/t.cpp:4\n", "tests/t.cpp:4\n") + NOT_TRACED_B)

    def test_bad_cite_in_the_answer_words_fails(self):
        self.assertFails("is not a line cite",
                         TRACE_A.replace("the tag field", "the tag field, see tests/t.cpp:4") + NOT_TRACED_B)

    def test_good_cite_in_the_answer_words_passes(self):
        self.assertPasses(TRACE_A.replace("the tag field", "the tag field, as code/tests/t.cpp:4 shows")
                          + NOT_TRACED_B)

    def test_range_wider_than_the_cap_fails(self):
        long_file = os.path.join(self.copy, "code", "tests", "t.cpp")
        with open(long_file, "w", encoding="utf-8") as f:
            f.write("x\n" * 40)
        self.assertFails("is wider than",
                         TRACE_A.replace("t.cpp:3-4", "t.cpp:1-40") + NOT_TRACED_B)

    def test_answer_with_no_cites_fails(self):
        self.assertFails("needs at least one cite",
                         TRACE_A.replace("| cites: code/tests/t.cpp:4\n", "| cites: \n") + NOT_TRACED_B)

    def test_answer_with_no_cites_field_fails(self):
        self.assertFails("ends with",
                         TRACE_A.replace(" | cites: code/tests/t.cpp:4\n", "\n") + NOT_TRACED_B)

    def test_kind_with_no_trace_fails(self):
        self.assertFails(f"the kind {KIND_B!r} has no trace", TRACE_A)

    def test_kind_heading_one_character_off_fails(self):
        off = NOT_TRACED_B.replace("ScopedHook", "Scopedhook")
        errors = self.errors(TRACE_A + off)
        self.assertTrue(any("is not a kind in kinds.md" in e for e in errors), errors)
        self.assertTrue(any(f"the kind {KIND_B!r} has no trace" in e for e in errors), errors)

    def test_kind_traced_twice_fails(self):
        self.assertFails("is traced twice", TRACE_A + NOT_TRACED_B, NOT_TRACED_B)

    def test_trace_holding_by_design_fails(self):
        self.assertFails("'by design'",
                         TRACE_A.replace("the tag field", "the tag field, by design") + NOT_TRACED_B)

    def test_every_status_word_fails_in_any_case(self):
        for word in ct.STATUS_WORDS:
            with self.subTest(word=word):
                self.assertFails("status or claim word",
                                 TRACE_A + NOT_TRACED_B.replace("reached my limit", word.upper()))

    def test_status_word_inside_a_longer_word_passes(self):
        self.assertPasses(TRACE_A.replace("the tag field", "the kept_count field") + NOT_TRACED_B)

    def test_status_line_fails(self):
        self.assertFails("a line the trace format does not have",
                         TRACE_A + "status: open\n" + NOT_TRACED_B)

    def test_line_before_first_heading_fails(self):
        self.assertFails("a line outside any kind's trace", "# Traces\n" + TRACE_A + NOT_TRACED_B)

    def test_case_out_of_order_fails(self):
        lines = TRACE_A.splitlines(keepends=True)
        swapped = lines[0] + lines[2] + lines[1] + lines[3]
        self.assertFails("is not whole cases", swapped + NOT_TRACED_B)

    def test_half_a_case_fails(self):
        lines = TRACE_A.splitlines(keepends=True)
        self.assertFails("is not whole cases", "".join(lines[:3]) + NOT_TRACED_B)

    def test_empty_trace_fails(self):
        self.assertFails("has no case", f"## {KIND_A}\n" + NOT_TRACED_B)

    def test_not_traced_beside_a_case_fails(self):
        self.assertFails("beside other lines", TRACE_A + "not traced: half done\n" + NOT_TRACED_B)

    def test_not_traced_with_no_reason_fails(self):
        self.assertFails("says what stopped", TRACE_A + f"## {KIND_B}\nnot traced: \n")

    def test_read_from_testdata_folder_passes(self):
        self.assertPasses(TRACE_A.replace("code/tests/replies/r.json", "code/testdata/song") + NOT_TRACED_B)

    def test_read_from_words_pass(self):
        for word in (ct.NONE_WORD, ct.CANNOT_TELL):
            with self.subTest(word=word):
                self.assertPasses(TRACE_A.replace("code/tests/replies/r.json", word) + NOT_TRACED_B)

    def test_read_from_missing_file_fails(self):
        self.assertFails("is not in the copy",
                         TRACE_A.replace("code/tests/replies/r.json", "code/tests/replies/gone.json") + NOT_TRACED_B)

    def test_read_from_the_testdata_folder_itself_fails(self):
        self.assertFails("is not a file or folder inside",
                         TRACE_A.replace("code/tests/replies/r.json", "code/testdata") + NOT_TRACED_B)

    def test_read_from_outside_tests_and_testdata_fails(self):
        src = os.path.join(self.copy, "code", "src", "net.cpp")
        os.makedirs(os.path.dirname(src))
        with open(src, "w", encoding="utf-8") as f:
            f.write("x\n")
        self.assertFails("is not a file or folder inside",
                         TRACE_A.replace("code/tests/replies/r.json", "code/src/net.cpp") + NOT_TRACED_B)

    def test_read_from_free_words_fail(self):
        self.assertFails("must be a file or folder cite",
                         TRACE_A.replace("code/tests/replies/r.json", "a reply file") + NOT_TRACED_B)

    def test_main_exit_codes(self):
        good = self.write("good.md", TRACE_A + NOT_TRACED_B)
        bad = self.write("bad.md", TRACE_A)
        with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(ct.main([self.copy, self.kinds, good]), 0)
            self.assertEqual(ct.main([self.copy, self.kinds, bad]), 1)
            self.assertEqual(ct.main([self.dir, self.kinds, good]), 2)
            self.assertEqual(ct.main([self.copy, self.kinds]), 2)

    def test_brief_carries_the_format_word_for_word(self):
        with open(BRIEF, encoding="utf-8") as f:
            brief = f.read()
        self.assertIn(ct.FORMAT_TEXT, brief)


if __name__ == "__main__":
    unittest.main(verbosity=1)
