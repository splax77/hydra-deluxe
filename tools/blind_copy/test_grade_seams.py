"""Tests for tools/blind_copy/grade_seams.py and seam_patterns.txt.

Run with: py -I tools/blind_copy/test_grade_seams.py
"""

import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
SCRIPT = os.path.join(HERE, "grade_seams.py")
sys.path.insert(0, HERE)

import facts_format as ff  # noqa: E402
import grade_seams as gs  # noqa: E402

# Lines 1-17. helper() holds a seam on line 2; test_body has its own on
# line 7; test_via_helper reaches line 2 through helper(); test_plain has none.
SLICE = """void helper() {
    hydra::ui::set_view_analyzer_for_test(fake);
}

void test_body() {
    int x = 0;
    hydra::net::set_fetcher(fake);
    IM_CHECK(x == 0);
}

void test_via_helper() {
    helper();
}

void test_plain() {
    IM_CHECK(1 == 1);
}
"""

UPDATE_STYLE = """namespace {

struct World {
    ScopedHook fetcher{hydra::net::set_fetcher, fake};
};

}

TEST_CASE("offered; Later keeps a button") {
    World w;
    CHECK(w.ok());
}
"""


def section(name, line, *inputs):
    out = f"## {name}\nlocation: code/tests/s.cpp:{line}\n"
    for src, cites in inputs:
        out += f"input: x | source: {src} | trace: {cites}\n"
    return out


class Base(unittest.TestCase):
    def setUp(self):
        self.copy = tempfile.mkdtemp(prefix="grade_seams_")
        self.addCleanup(shutil.rmtree, self.copy, True)
        self.write("code/tests/s.cpp", SLICE)

    def write(self, rel, text):
        path = os.path.join(self.copy, *rel.split("/"))
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
        return path

    def run_grader(self, facts_text, ranges="tests/s.cpp:1-17"):
        facts = self.write("facts.md", facts_text)
        r = subprocess.run([sys.executable, "-I", SCRIPT, self.copy, ranges, facts],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        return r.stdout

    def rows(self, out):
        return [l for l in out.splitlines() if l.startswith("hit: ")]


class Grading(Base):
    def test_hit_in_the_body_found(self):
        out = self.run_grader(section("test_body", 5, (ff.REPLACES, "code/tests/s.cpp:7")))
        self.assertIn("hit: test_body | code/tests/s.cpp:7 | set_fetcher | found", out)

    def test_hit_in_the_body_missed_without_a_cite(self):
        out = self.run_grader(section("test_body", 5, (ff.REPLACES, "code/tests/s.cpp:6")))
        self.assertIn("miss: test_body | code/tests/s.cpp:7 | set_fetcher", out)

    def test_hit_reached_through_a_helper(self):
        found = self.run_grader(section("test_via_helper", 11, (ff.PRODUCES_ITSELF, "code/tests/s.cpp:1-3")))
        self.assertIn("hit: test_via_helper | code/tests/s.cpp:2 | for_test name | found", found)
        missed = self.run_grader(section("test_via_helper", 11, (ff.PRODUCES_ITSELF, "code/tests/s.cpp:12")))
        self.assertIn("miss: test_via_helper | code/tests/s.cpp:2 | for_test name", missed)

    def test_a_helper_no_test_names_is_not_owned_by_the_others(self):
        out = self.run_grader(section("test_body", 5, (ff.REPLACES, "code/tests/s.cpp:7")))
        self.assertNotIn("hit: test_body | code/tests/s.cpp:2", out)
        self.assertNotIn("hit: test_plain", out)
        self.assertIn("hit: test_via_helper | code/tests/s.cpp:2", out)

    def test_hit_cited_as_a_testdata_file_is_a_miss(self):
        out = self.run_grader(section("test_body", 5, (ff.TESTDATA, "code/tests/s.cpp:7")))
        self.assertIn("miss: test_body | code/tests/s.cpp:7 | set_fetcher", out)

    def test_each_seam_source_word_counts(self):
        for word in ff.SEAM_SOURCES:
            with self.subTest(word=word):
                out = self.run_grader(section("test_body", 5, (word, "code/tests/s.cpp:7")))
                self.assertIn("code/tests/s.cpp:7 | set_fetcher | found", out)

    def test_another_tests_cite_does_not_count(self):
        out = self.run_grader(section("test_plain", 15, (ff.REPLACES, "code/tests/s.cpp:7")))
        self.assertIn("miss: test_body | code/tests/s.cpp:7 | set_fetcher", out)

    def test_zero_hits_prints_zero(self):
        out = self.run_grader(section("test_plain", 15), ranges="tests/s.cpp:15-17")
        self.assertIn("hits: 0\nfound: 0\nmissed: 0\n", out)

    def test_counts_and_line_count(self):
        out = self.run_grader(section("test_body", 5, (ff.REPLACES, "code/tests/s.cpp:7")))
        self.assertIn(f"slice file: code/tests/s.cpp | lines: {SLICE.count(chr(10))} | range: 1-17", out)
        self.assertIn("hits: 2\nfound: 1\nmissed: 1\n", out)

    def test_older_facts_with_asides_are_read(self):
        old = ("## test_body\nlocation: code/tests/s.cpp:5\n"
               f"input: fetcher | source: {ff.REPLACES} | trace: code/tests/s.cpp:5 -> "
               "code/tests/s.cpp:7 (set_fetcher replaced by a lambda)\n")
        out = self.run_grader(old)
        self.assertIn("code/tests/s.cpp:7 | set_fetcher | found", out)

    def test_namespace_and_test_case_names(self):
        self.write("code/tests/u.cpp", UPDATE_STYLE)
        facts = ('## offered; Later keeps a button\nlocation: code/tests/u.cpp:9\n'
                 f"input: reply | source: {ff.REPLACES} | trace: code/tests/u.cpp:10 -> code/tests/u.cpp:3-5\n")
        out = self.run_grader(facts, ranges="tests/u.cpp:1-12")
        self.assertIn("test: offered; Later keeps a button | code/tests/u.cpp:9-12 | facts section: yes", out)
        self.assertIn("hit: offered; Later keeps a button | code/tests/u.cpp:4 | set_fetcher, ScopedHook | found", out)
        self.assertNotIn("test: World", out)

    def test_bad_range_is_an_error(self):
        facts = self.write("facts.md", "")
        r = subprocess.run([sys.executable, "-I", SCRIPT, self.copy, "tests/s.cpp", facts],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 2)


def repo_lines(rel):
    with open(os.path.join(REPO, *rel.split("/")), encoding="utf-8") as f:
        return [l.strip() for l in f.read().splitlines()]


class Patterns(unittest.TestCase):
    def setUp(self):
        self.patterns = gs.load_patterns()

    def labels(self, line):
        return [p.label for p in self.patterns if p.regex.search(line)]

    def test_each_line_matches_its_real_example(self):
        for p in self.patterns:
            with self.subTest(label=p.label):
                self.assertIn(p.example, repo_lines(p.example_file))
                self.assertTrue(p.regex.search(p.example), p.example)

    def test_forms_the_plan_names_are_present(self):
        for line, label in (("set_report_hold_for_test(x);", "for_test name"),
                            ("hydra::net::set_fetcher(f);", "set_fetcher"),
                            ("hydra::net::set_downloader(f);", "set_downloader"),
                            ("ScopedHook off(a, b);", "ScopedHook"),
                            ('h.app->selected->charter = "Hoph2o";', "selected-> assignment")):
            with self.subTest(line=line):
                self.assertIn(label, self.labels(line))

    def test_selected_comparisons_are_not_hits(self):
        compares = [l for l in repo_lines("tests/ui/uitest_details.cpp") if "selected->notespath ==" in l]
        self.assertTrue(compares)
        for line in compares + ["if (selected->x != y)", "a = selected->x <= y;", "selected->x >= y"]:
            with self.subTest(line=line):
                self.assertNotIn("selected-> assignment", self.labels(line))

    def test_every_scoped_hook_holder_is_a_wrapper_line(self):
        wrapper = next(p for p in self.patterns if p.label == "seam wrapper object")
        member = re.compile(r"^\s+(?:std::optional<\s*)?ScopedHook\s*>?\s+\w+\s*;")
        head = re.compile(r"^(?:class|struct)\s+(\w+)")
        holders = set()
        for root, _, files in os.walk(os.path.join(REPO, "tests")):
            for name in files:
                if not name.endswith((".cpp", ".h")):
                    continue
                with open(os.path.join(root, name), encoding="utf-8", errors="replace") as f:
                    owner = None
                    for line in f:
                        m = head.match(line)
                        if m:
                            owner = m.group(1)
                        if member.match(line) and owner:
                            holders.add(owner)
        self.assertTrue(holders)
        for name in sorted(holders):
            with self.subTest(holder=name):
                self.assertTrue(wrapper.regex.search(f"    {name} gate;"), name)

    def test_wrapper_line_skips_references_and_definitions(self):
        for line in ("std::unique_ptr<AppState> offered_app(UpdateWorld& w) {", "struct UpdateWorld {"):
            with self.subTest(line=line):
                self.assertNotIn("seam wrapper object", self.labels(line))


if __name__ == "__main__":
    unittest.main()
