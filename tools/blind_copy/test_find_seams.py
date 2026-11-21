"""Tests for tools/blind_copy/find_seams.py.

Run with: py -I tools/blind_copy/test_find_seams.py
"""

import os
import shutil
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
SCRIPT = os.path.join(HERE, "find_seams.py")
sys.path.insert(0, HERE)

import find_seams as fs  # noqa: E402
import grade_seams as gs  # noqa: E402

# A made-up copy. src/ declares four seam names. tests/ names three of them:
# set_fetcher through the wrapper type in world.h, set_audio_device_factory
# in a line only its own pattern matches, and set_clock in a plain call no
# pattern matches. set_lookup_hold_for_test is declared and never named.
TREE = {
    "code/src/net/net.h": (
        "#pragma once\n"
        "namespace hydra::net {\n"
        "void set_fetcher(Fetcher f);\n"
        "void set_clock(Clock c);\n"
        "}\n"),
    "code/src/app/jobs.h": (
        "#pragma once\n"
        "struct Job {\n"
        "    void set_lookup_hold_for_test(Fn f);\n"
        "    void set_audio_device_factory(Factory f);\n"
        "};\n"),
    # Lines 1-4.
    "code/tests/world.h": (
        "#pragma once\n"
        "struct UpdateWorld {\n"
        "    ScopedHook fetcher{hydra::net::set_fetcher, fake};\n"
        "};\n"),
    # Line 4 is the wrapper object, line 9 the uncovered call, line 14 the
    # one-pattern call, line 18 a hit no test owns.
    "code/tests/test_update.cpp": (
        '#include "world.h"\n'
        "\n"
        'TEST_CASE("update offered") {\n'
        "    UpdateWorld w;\n"
        "    CHECK(w.ok());\n"
        "}\n"
        "\n"
        'TEST_CASE("clock set plainly") {\n'
        "    hydra::net::set_clock(fake_clock);\n"
        "    CHECK(true);\n"
        "}\n"
        "\n"
        'TEST_CASE("no audio device") {\n'
        "    pc.set_audio_device_factory(no_device_factory());\n"
        "    CHECK(true);\n"
        "}\n"
        "\n"
        "#define FAKE_NET hydra::net::set_fetcher(nullptr)\n"),
    "code/tests/fuzz/fuzz_a.cpp": "int fuzz_one(const char* d) {\n    return 0;\n}\n",
    "code/tests/reply.json": '{"tag": "v1"}\n',
    "code/tools/test_tool.py": "def test_a():\n    assert 1 == 1\n",
}


class Run(unittest.TestCase):
    def setUp(self):
        self.copy = tempfile.mkdtemp(prefix="find_seams_test_")
        self.addCleanup(shutil.rmtree, self.copy, True)
        for rel, text in TREE.items():
            path = os.path.join(self.copy, *rel.split("/"))
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "w", encoding="utf-8", newline="\n") as f:
                f.write(text)
        self.out = os.path.join(self.copy, "out")

    def run_script(self):
        r = subprocess.run([sys.executable, "-I", SCRIPT, self.copy, "--out", self.out],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        return r.stdout

    def tsv(self, name):
        with open(os.path.join(self.out, name), encoding="utf-8") as f:
            lines = f.read().splitlines()
        header = lines[0].split("\t")
        return [dict(zip(header, l.split("\t"))) for l in lines[1:]]

    def seams(self, test=None):
        rows = self.tsv("seams.tsv")
        return [r for r in rows if test is None or r["test"] == test]

    def test_a_wrapper_type_gives_its_test_the_inner_hits(self):
        self.run_script()
        got = {(r["hit_file"], r["hit_line"], r["kind"]) for r in self.seams("update offered")}
        self.assertIn(("tests/test_update.cpp", "4", "UpdateWorld (tests/test_update.cpp)"), got)
        self.assertIn(("tests/world.h", "3", "set_fetcher (tests/world.h)"), got)
        row = next(r for r in self.seams("update offered") if r["hit_file"] == "tests/world.h")
        self.assertEqual((row["test_file"], row["test_line"]), ("tests/test_update.cpp", "3"))
        self.assertEqual(row["patterns"], "set_fetcher, ScopedHook")

    def test_a_declared_name_no_pattern_matches_is_uncovered(self):
        out = self.run_script()
        cov = {c["name"]: c for c in self.tsv("coverage.tsv")}
        self.assertEqual(cov["set_clock"]["state"], "uncovered")
        self.assertEqual(cov["set_clock"]["declared_at"], "src/net/net.h:4")
        self.assertEqual(cov["set_clock"]["named_at"], "tests/test_update.cpp:9")
        self.assertIn("uncovered: set_clock | declared: src/net/net.h:4 | named: tests/test_update.cpp:9", out)

    def test_a_name_its_pattern_matches_is_covered(self):
        # Fails when the set_audio_device_factory line leaves seam_patterns.txt:
        # no other form matches the call on test_update.cpp:14.
        out = self.run_script()
        cov = {c["name"]: c["state"] for c in self.tsv("coverage.tsv")}
        self.assertEqual(cov["set_audio_device_factory"], "covered")
        self.assertEqual(cov["set_fetcher"], "covered")
        self.assertNotIn("uncovered: set_audio_device_factory", out)

    def test_removing_the_pattern_makes_the_name_uncovered(self):
        patterns = [p for p in gs.load_patterns() if p.label != "set_audio_device_factory"]
        cov = {c["name"]: c["state"] for c in fs.search(self.copy, patterns)["coverage"]}
        self.assertEqual(cov["set_audio_device_factory"], "uncovered")

    def test_a_name_tests_never_use_is_not_listed(self):
        self.run_script()
        self.assertNotIn("set_lookup_hold_for_test", {c["name"] for c in self.tsv("coverage.tsv")})

    def test_a_header_struct_listed_as_a_test_is_marked_not_a_test_case(self):
        self.run_script()
        self.assertEqual({r["test_case"] for r in self.seams("update offered")}, {"yes"})
        header = [r for r in self.seams("UpdateWorld")]
        self.assertTrue(header)
        self.assertEqual({r["test_case"] for r in header}, {"no"})
        self.assertEqual({r["test_case"] for r in self.seams("no test")}, {""})

    def test_a_hit_no_test_owns_is_kept_as_no_test(self):
        out = self.run_script()
        rows = self.seams("no test")
        self.assertEqual([(r["hit_file"], r["hit_line"], r["test_file"], r["test_line"]) for r in rows],
                         [("tests/test_update.cpp", "18", "", "")])
        self.assertEqual(rows[0]["kind"], "set_fetcher (tests/test_update.cpp)")
        self.assertIn("no test hits: 1\n", out)

    def test_kind_counts_add_up_to_the_hit_total(self):
        out = self.run_script()
        lines = out.splitlines()
        total = int(next(l for l in lines if l.startswith("hits: ")).split(": ")[1])
        per_kind = [int(l.rsplit("| hits: ", 1)[1]) for l in lines if l.startswith("kind: ")]
        kinds = int(next(l for l in lines if l.startswith("kinds: ")).split(": ")[1])
        rows = self.seams()
        self.assertTrue(rows)
        self.assertEqual(sum(per_kind), total)
        self.assertEqual(total, len(rows))
        self.assertEqual(kinds, len(per_kind))
        self.assertEqual(kinds, len({r["kind"] for r in rows}))
        self.assertTrue(all(r["kind"] for r in rows))

    def test_file_kinds_are_counted_and_named(self):
        out = self.run_script()
        self.assertIn("no tests: tools .py | files: 1", out)
        self.assertIn("no hits: tests/fuzz .cpp | files: 1 | tests: 1", out)
        self.assertIn("skipped: tests .json | files: 1", out)
        kinds = {k["file_kind"]: k for k in self.tsv("file_kinds.tsv")}
        self.assertEqual(kinds["tools .py"]["no_test"], "1")
        self.assertEqual(kinds["tests .json"]["state"], "skipped")
        self.assertIn("files read: 4 | no test: 1 | skipped: 1", out)

    def test_a_copy_without_tests_is_an_error(self):
        empty = tempfile.mkdtemp(prefix="find_seams_empty_")
        self.addCleanup(shutil.rmtree, empty, True)
        r = subprocess.run([sys.executable, "-I", SCRIPT, empty, "--out", os.path.join(empty, "out")],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 2)
        self.assertIn("code/tests", r.stderr)


class Kinds(unittest.TestCase):
    """The plan's own examples of the kind rule, from both ends."""

    def setUp(self):
        self.patterns = gs.load_patterns()

    def kind(self, line, path):
        labels = [p.label for p in self.patterns if p.regex.search(line)]
        self.assertTrue(labels, line)
        return fs.kind(path, line, labels, self.patterns)

    def test_examples(self):
        for line, path, want in (
                ("fetcher(hydra::net::set_fetcher,", "code/tests/test_update_state.cpp",
                 "set_fetcher (tests/test_update_state.cpp)"),
                ("hydra::net::set_fetcher(", "code/tests/ui/uitest_harness.cpp",
                 "set_fetcher (tests/ui/uitest_harness.cpp)"),
                ("job.set_lookup_hold_for_test([&lookups] {", "code/tests/test_library_jobs.cpp",
                 "set_lookup_hold_for_test (tests/test_library_jobs.cpp)"),
                ("ScopedHook fetcher;", "code/tests/a.cpp", "ScopedHook (tests/a.cpp)"),
                ('h.app->selected->artist = "Green Day";', "code/tests/ui/uitest_details.cpp",
                 "selected-> assignment (tests/ui/uitest_details.cpp)"),
                ("    ViewGate gate;", "code/tests/ui/uitest_details.cpp", "ViewGate (tests/ui/uitest_details.cpp)"),
                ("    const UpdateWorld w;", "code/tests/a.cpp", "UpdateWorld (tests/a.cpp)")):
            with self.subTest(line=line):
                self.assertEqual(self.kind(line, path), want)

    def test_the_same_word_in_two_files_is_two_kinds(self):
        a = self.kind("fetcher(hydra::net::set_fetcher,", "code/tests/test_update_state.cpp")
        b = self.kind("hydra::net::set_fetcher(", "code/tests/ui/uitest_harness.cpp")
        self.assertNotEqual(a, b)


if __name__ == "__main__":
    unittest.main()
