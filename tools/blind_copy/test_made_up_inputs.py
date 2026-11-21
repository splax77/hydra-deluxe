"""Tests for tools/blind_copy/made_up_inputs.py, the made-up-input list.

Run with: py -I tools/blind_copy/test_made_up_inputs.py
"""

import inspect
import os
import shutil
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
SCRIPT = os.path.join(HERE, "made_up_inputs.py")
sys.path.insert(0, HERE)

import facts_format as ff  # noqa: E402
import made_up_inputs as mu  # noqa: E402

GRADER = """slice file: code/tests/s.cpp | lines: 20 | range: 1-20
hits: 1
found: 0
missed: 1
miss: test_one | code/tests/s.cpp:7 | set_fetcher
"""


def facts_with(*sources):
    out = "## test_one\nlocation: code/tests/s.cpp:5\n"
    for i, src in enumerate(sources):
        if src == ff.REPLACES:
            out += (f"input: in{i} | source: {src} | replaces: hydra::net::fetch"
                    f" | stands in: throws code/tests/s.cpp:6 | trace: code/tests/s.cpp:{i + 1}\n")
        elif src == ff.TESTDATA:
            out += f"input: in{i} | source: {src} | trace: code/tests/s.cpp:{i + 1} -> code/testdata/x/real.chart\n"
        elif src == ff.NETWORK:
            out += f"input: in{i} | source: {src} | trace: code/tests/s.cpp:{i + 1} -> code/tests/r/reply.json\n"
        else:
            out += f"input: in{i} | source: {src} | trace: code/tests/s.cpp:{i + 1}\n"
    return out


class Rows(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp(prefix="made_up_")
        self.addCleanup(shutil.rmtree, self.dir, True)

    def write(self, name, text):
        path = os.path.join(self.dir, name)
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
        return path

    def table(self, facts_text, grader_text=""):
        facts = self.write("facts.md", facts_text)
        grader = self.write("grader.txt", grader_text)
        out = os.path.join(self.dir, "list.md")
        r = subprocess.run([sys.executable, "-I", SCRIPT, facts, grader, out], capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        with open(out, encoding="utf-8") as f:
            lines = f.read().splitlines()
        self.assertEqual(lines[0], "| " + " | ".join(mu.COLUMNS) + " |")
        return [[c.strip() for c in l.strip("|").split(" | ")] for l in lines[2:]]

    def test_each_made_up_source_gives_a_row(self):
        for src in ff.MADE_UP_SOURCES:
            with self.subTest(source=src):
                rows = self.table(facts_with(src))
                self.assertEqual(len(rows), 1)
                self.assertEqual(rows[0][2], src)
                self.assertEqual(rows[0][6], mu.BY_FACTS)

    def test_the_five_sources_are_the_planned_five(self):
        self.assertEqual(set(ff.MADE_UP_SOURCES), {
            ff.REPLACES, ff.PRODUCES_ITSELF, ff.BUILDS_IN_CODE, ff.WRITES_ITSELF, ff.CANNOT_TELL})

    def test_testdata_and_network_give_no_row(self):
        self.assertEqual(self.table(facts_with(ff.TESTDATA, ff.NETWORK)), [])

    def test_a_real_label_without_a_real_file_cite_is_listed(self):
        head = "## test_one\nlocation: code/tests/s.cpp:5\n"
        for label, trace in (
                (ff.TESTDATA, "code/tests/s.cpp:1 -> code/testdata"),
                (ff.TESTDATA, "code/tests/s.cpp:1"),
                (ff.TESTDATA, "code/tests/s.cpp:1 -> code/tests/r/reply.json"),
                (ff.NETWORK, "code/tests/s.cpp:1 -> code/tests/s.cpp"),
                (ff.NETWORK, "code/tests/s.cpp:1 -> code/tests"),
                (ff.NETWORK, "code/tests/s.cpp:1")):
            with self.subTest(label=label, trace=trace):
                rows = self.table(head + f"input: in | source: {label} | trace: {trace}\n")
                self.assertEqual(len(rows), 1)
                self.assertEqual(rows[0][2], label)

    def test_an_unknown_or_missing_source_is_listed(self):
        head = "## test_one\nlocation: code/tests/s.cpp:5\n"
        for line in ("input: in | source: a fixture file | trace: code/tests/s.cpp:1\n",
                     "input: in | trace: code/tests/s.cpp:1\n"):
            with self.subTest(line=line):
                self.assertEqual(len(self.table(head + line)), 1)

    def test_replaced_function_and_stand_in_are_carried(self):
        rows = self.table(facts_with(ff.REPLACES))
        self.assertEqual(rows[0][:6], ["test_one", "in0", ff.REPLACES, "hydra::net::fetch", "throws",
                                       "code/tests/s.cpp:1"])

    def test_grader_miss_gives_a_search_row(self):
        rows = self.table(facts_with(ff.TESTDATA), GRADER)
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0][0], "test_one")
        self.assertEqual(rows[0][5], "code/tests/s.cpp:7")
        self.assertEqual(rows[0][6], mu.BY_SEARCH)
        self.assertIn("found by search", rows[0][6])

    def test_every_row_is_open(self):
        rows = self.table(facts_with(*ff.SOURCE_WORDS), GRADER)
        self.assertEqual(len(rows), len(ff.MADE_UP_SOURCES) + 1)
        self.assertEqual({r[7] for r in rows}, {"open"})

    def test_no_row_can_carry_another_status(self):
        self.assertNotIn("status", inspect.signature(mu.row).parameters)
        self.assertEqual(mu.row("t", "i", "s", "f", "w", "c", "b")[-1], "open")
        self.assertEqual(mu.STATUS, "open")


if __name__ == "__main__":
    unittest.main()
