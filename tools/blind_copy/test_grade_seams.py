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

# Lines 1-24. A UI file: two runnable test functions listed in a TestEntry
# table, and a helper that also takes the context but is not listed.
UI_STYLE = """namespace {

void open_panel(ImGuiTestContext* ctx) {
    hydra::ui::set_view_analyzer_for_test(fake);
}

void test_close(ImGuiTestContext* ctx) {
    hydra::net::set_fetcher(fake);
    open_panel(ctx);
}

void test_plain(ImGuiTestContext* ctx) {
    IM_CHECK(1 == 1);
}

}

const std::vector<TestEntry>& area_tests() {
    static const std::vector<TestEntry> entries = {
        {"close", test_close},
        {"plain", test_plain},
    };
    return entries;
}
"""

# Lines 1-7. A name table whose function does not take the test context.
HANDLER_TABLE = """void on_click(int x) {
    hydra::net::set_fetcher(fake);
}

const Handler handlers[] = {
    {"click", on_click},
};
"""


# Lines 1-20. Test a is registered by a call. `decoy` only holds a commented-out
# call and a string that looks like an entry, so it registers nothing.
CALL_STYLE = """void test_a(ImGuiTestContext* ctx) {
    hydra::net::set_fetcher(fake);
}

void test_b(ImGuiTestContext* ctx) {
    hydra::net::set_fetcher(fake);
}

void test_c(ImGuiTestContext* ctx) {
    hydra::net::set_fetcher(fake);
}

void register_all(Harness& h) {
    uitest::register_test(h, "a", test_a);
}

void decoy(Harness& h) {
    // register_test(h, "b", test_b);
    const char* s = "{\\"c\\", test_c}";
    test_b(nullptr);
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

    def test_a_brace_default_argument_does_not_end_the_unit(self):
        text = ("size_t helper(int a,\n"
                "              const std::function<void()>& f = {}) {\n"
                "    return a;\n"
                "}\n\n"
                "void test_next(Ctx* c) {\n"
                "    run();\n"
                "}\n")
        got = [(u.name, u.start, u.end) for u in gs.top_units("t.cpp", text)]
        self.assertEqual(got, [("helper", 1, 4), ("test_next", 6, 8)])

    def test_namespace_and_test_case_names(self):
        self.write("code/tests/u.cpp", UPDATE_STYLE)
        facts = ('## offered; Later keeps a button\nlocation: code/tests/u.cpp:9\n'
                 f"input: reply | source: {ff.REPLACES} | trace: code/tests/u.cpp:10 -> code/tests/u.cpp:3-5\n")
        out = self.run_grader(facts, ranges="tests/u.cpp:1-12")
        self.assertIn("test: offered; Later keeps a button | code/tests/u.cpp:9-12 | facts section: yes", out)
        self.assertIn("hit: offered; Later keeps a button | code/tests/u.cpp:4 | set_fetcher, ScopedHook | found", out)
        self.assertNotIn("test: World", out)

    def test_a_registered_ui_function_is_a_test_and_its_table_is_not(self):
        self.write("code/tests/ui.cpp", UI_STYLE)
        out = self.run_grader(section("test_close", 7), ranges="tests/ui.cpp:1-24")
        self.assertIn("test: test_close | code/tests/ui.cpp:7-10", out)
        self.assertIn("test: test_plain | code/tests/ui.cpp:12-14", out)
        self.assertNotIn("test: area_tests", out)
        self.assertIn("miss: test_close | code/tests/ui.cpp:8 | set_fetcher", out)
        self.assertIn("miss: test_close | code/tests/ui.cpp:4 | for_test name", out)
        self.assertNotIn("area_tests | code/tests/ui.cpp", out)
        self.assertNotIn("test_plain | code/tests/ui.cpp:", out.split("test: test_plain")[1])

    def test_an_unlisted_context_helper_stays_a_helper(self):
        self.write("code/tests/ui.cpp", UI_STYLE)
        out = self.run_grader(section("test_close", 7), ranges="tests/ui.cpp:1-24")
        self.assertNotIn("test: open_panel", out)

    def test_a_register_test_call_lists_a_test_but_a_comment_or_string_does_not(self):
        self.write("code/tests/c.cpp", CALL_STYLE)
        out = self.run_grader(section("test_a", 1), ranges="tests/c.cpp:1-21")
        self.assertIn("test: test_a | code/tests/c.cpp:1-3", out)
        self.assertNotIn("test: register_all", out)
        self.assertIn("test: decoy | code/tests/c.cpp:17-21", out)

    def test_a_name_table_without_the_test_context_registers_no_test(self):
        self.write("code/tests/h.cpp", HANDLER_TABLE)
        out = self.run_grader(section("on_click", 1), ranges="tests/h.cpp:1-7")
        self.assertNotIn("test: on_click", out)
        self.assertIn("hit: code/tests/h.cpp:5 | code/tests/h.cpp:2 | set_fetcher", out)

    def outside_copy(self):
        self.write("code/tests/main_t.cpp",
                   '#include "help.h"\n\nvoid test_uses_outside() {\n    make_world();\n    SEAM_UP(x);\n'
                   '    Gate g;\n}\n')
        self.write("code/tests/help.h",
                   '#pragma once\n#define SEAM_UP(x) \\\n    hydra::net::set_fetcher(x)\n'
                   'void make_world();\nstruct Gate {\n    ScopedHook seam_;\n};\n')
        self.write("code/tests/help.cpp",
                   '#include "help.h"\n\nvoid make_world() {\n    hydra::app::set_other_thing([] { return 1; });\n}\n')
        return "tests/main_t.cpp:1-7"

    def test_seam_in_a_helper_defined_outside_the_slice(self):
        out = self.run_grader("", ranges=self.outside_copy())
        self.assertIn("miss: test_uses_outside | code/tests/help.cpp:4 | set_ call given a lambda", out)

    def test_seam_in_a_macro_and_a_wrapper_type_outside_the_slice(self):
        out = self.run_grader("", ranges=self.outside_copy())
        self.assertIn("miss: test_uses_outside | code/tests/help.h:3 | set_fetcher", out)
        self.assertIn("miss: test_uses_outside | code/tests/help.h:6 | ScopedHook", out)

    def test_outside_hit_is_found_when_the_facts_cite_it(self):
        facts = ("## test_uses_outside\nlocation: code/tests/main_t.cpp:3\n"
                 f"input: world | source: {ff.REPLACES} | trace: code/tests/help.cpp:4\n")
        out = self.run_grader(facts, ranges=self.outside_copy())
        self.assertIn("hit: test_uses_outside | code/tests/help.cpp:4 | set_ call given a lambda | found", out)
        self.assertIn("miss: test_uses_outside | code/tests/help.h:6 | ScopedHook", out)

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

    def test_a_lambda_given_to_any_set_call_is_a_hit(self):
        for line in ("hydra::app::set_other_thing([] { return 1; });",
                     "set_x(a, [&](int v) { use(v); });", "pc.set_y(", "set_z([=] {"):
            with self.subTest(line=line):
                self.assertTrue(self.labels(line), line)
        self.assertFalse(self.labels("set_sort(a, b);"))

    def test_compound_assignment_to_selected_is_a_hit(self):
        self.assertIn("selected-> assignment", self.labels("h.app->selected->plays += 1;"))
        self.assertNotIn("selected-> assignment", self.labels("if (h.app->selected->plays <= 1) {"))

    def test_the_scoped_hook_class_itself_is_not_a_hit(self):
        for line in ("class ScopedHook {", "    ~ScopedHook() { clear_(); }",
                     "    ScopedHook(const ScopedHook&) = delete;"):
            with self.subTest(line=line):
                self.assertNotIn("ScopedHook", self.labels(line))
        self.assertIn("ScopedHook", self.labels("    ScopedHook seam_;"))
        self.assertIn("ScopedHook", self.labels("    const ScopedHook seam(set, fn);"))

    def test_wrapper_line_skips_references_and_definitions(self):
        for line in ("std::unique_ptr<AppState> offered_app(UpdateWorld& w) {", "struct UpdateWorld {"):
            with self.subTest(line=line):
                self.assertNotIn("seam wrapper object", self.labels(line))


if __name__ == "__main__":
    unittest.main()
