"""Tests for check_bars.py: a made-up set of outputs that meets every bar,
then one broken input per bar that must fail it, B8 from both ends for all
four kinds on real tests of the copy, and B1 to B4 on pilot 2's files.
Run: py -I tools/blind_copy/test_check_bars.py"""

import os
import shutil
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import check_bars as cb  # noqa: E402
import find_seams as fs  # noqa: E402
import kinds as k  # noqa: E402
import match_records as mr  # noqa: E402

PILOT2 = os.path.join(HERE, "fixtures", "pilot2")
REAL_COPY = mr.COPY
UPDATE = "tests/test_update_state.cpp"
PANEL_FILE = "tests/ui/uitest_details.cpp"
PANEL = "test_panel_close_cancels"
SEVEN = [f"update test {i}" for i in range(1, 8)]


def write(path, header, rows):
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\t".join(header) + "\n")
        for r in rows:
            f.write("\t".join(str(c) for c in r) + "\n")


def joined_column(name):
    return k.JOINED_COLUMNS.index(name)


def matched_row(record, name, file, state, match=mr.SAME_PLACE):
    row = {c: "" for c in mr.COLUMNS}
    row.update({"record": record, "test name": name, "test file": file, "match": match, "state": state,
                "newest copy": "yes", "test state": state,
                "line 9": f"{record}: 9 Stand-in: None.", "line 11": f"{record}: 11 Real: Yes."})
    return [row[c] for c in mr.COLUMNS]


class Good:
    """A folder of outputs that meets every bar."""

    def __init__(self, root):
        self.root = root
        self.copy = os.path.join(root, "copy")
        self.records = os.path.join(root, "records")
        self.out = os.path.join(root, "out")
        for d in (os.path.join(self.copy, "code", "tests", "ui"), self.records, self.out):
            os.makedirs(d, exist_ok=True)
        with open(os.path.join(self.copy, "code", *UPDATE.split("/")), "w", encoding="utf-8") as f:
            f.write("\n".join(f"    UpdateWorld w(\"update_{n}\");" for n in range(1, 21)) + "\n")
        with open(os.path.join(self.copy, "code", *PANEL_FILE.split("/")), "w", encoding="utf-8") as f:
            f.write("\n".join(f"    ViewGate gate_{n};" for n in range(1, 11)) + "\n")
        with open(os.path.join(self.records, "C13.md"), "w", encoding="utf-8") as f:
            f.write("\n".join(f"9 Stand-in: None, row {n}." for n in range(1, 700)) + "\n")
        self.answers = [[k.FAKE, k.CONTRADICTED, ""]]
        self.build()

    def build(self):
        seams = [[name, UPDATE, "1", "yes", UPDATE, str(10 + i), "for_test name", "x"]
                 for i, name in enumerate(SEVEN)]
        seams += [[PANEL, PANEL_FILE, "1", "yes", PANEL_FILE, "5", "ScopedHook", "x"],
                  [PANEL, PANEL_FILE, "1", "yes", PANEL_FILE, "6", "ScopedHook", "x"]]
        self.seams = seams
        self.rows = [k.Row(f"{UPDATE}:{10 + i}", outcome=k.FAKE) for i in range(7)]
        self.rows += [k.Row(f"{PANEL_FILE}:5", outcome=k.FAIL), k.Row(f"{PANEL_FILE}:6", note="x")]
        self.matched = [matched_row(f"C13.md:{551 + 10 * i}", n, UPDATE, mr.FLAGGED) for i, n in enumerate(SEVEN)]
        self.matched += [matched_row("A39.md:389", PANEL, PANEL_FILE, mr.FLAGGED),
                         matched_row("B1.md:5", "", "", mr.SOMETHING_ELSE, mr.NO_TEST)]
        self.unmatched = [self.matched[-1]]
        self.coverage = [["set_fetcher", "covered", "src/net/http.h:56", ""],
                         ["set_all", "uncovered", "src/app/path_view.h:298", ""]]

    def write(self, questions=None, joined=None):
        o = self.out
        write(os.path.join(o, "seams.tsv"), fs.SEAM_COLUMNS, self.seams)
        write(os.path.join(o, "coverage.tsv"), fs.COVERAGE_COLUMNS, self.coverage)
        write(os.path.join(o, "matched.tsv"), mr.COLUMNS, self.matched)
        write(os.path.join(o, "unmatched.tsv"), mr.COLUMNS, self.unmatched)
        write(os.path.join(o, "answers.tsv"), ("kind", "answer", "exceptions"), self.answers)
        answers = k.read_answers(os.path.join(o, "answers.tsv"))
        self.joined = joined if joined is not None else \
            k.join([dict(zip(fs.SEAM_COLUMNS, s)) for s in self.seams], self.rows, [dict(zip(mr.COLUMNS, m)) for m in self.matched], set(), answers)
        k.write_outputs(o, self.rows, self.joined)
        with open(os.path.join(o, "questions.md"), "w", encoding="utf-8") as f:
            f.write(questions if questions is not None else self.questions())

    def questions(self):
        km = cb.sections(cb.text(os.path.join(self.out, "kinds.md")))
        out = []
        for kind in k.KINDS:
            out += [f"## {kind}", "", "status: open"]
            out += [f"tests: {cb.field(km[kind], 'tests')[0]}",
                    f"flagged records: {cb.field(km[kind], 'flagged records')[0]}"]
            for n in (3, 4):
                out.append(f"example code: {UPDATE}:{n} | UpdateWorld w(\"update_{n}\");")
                out.append(f"example record: records/C13.md:{n + 600} | 9 Stand-in: None, row {n + 600}.")
            if kind == k.FAIL:
                out.append(f"unsorted: {PANEL_FILE}:6")
            out.append("")
        out.append("uncovered: set_all")
        return "\n".join(out) + "\n"

    def paths(self, **over):
        p = {key: os.path.join(self.out, name) for key, name in cb.FILES.items()}
        p.update(copy=self.copy, records=self.records, repo=None)
        p.update(over)
        return p


class Bars(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp()
        self.g = Good(self.tmp)
        self.g.write()

    def tearDown(self):
        shutil.rmtree(self.tmp)

    def bar(self, b, **over):
        return cb.check(self.g.paths(**over), [b])[b]

    def fails(self, b, needle=None, **over):
        ok, line = self.bar(b, **over)
        self.assertFalse(ok, line)
        if needle:
            self.assertIn(needle, line)

    def test_good_folder_passes_every_bar(self):
        got = cb.check(self.g.paths())
        for b in cb.BARS:
            self.assertTrue(got[b][0], f"{b}: {got[b][1]}")

    def test_b1_test_with_a_hit_and_no_row(self):
        self.g.write(joined=self.g.joined[1:])
        self.fails("B1", SEVEN[0])

    def test_b2_one_of_seven_in_another_state(self):
        self.g.matched[2] = matched_row("C13.md:571", SEVEN[2], UPDATE, mr.SOMETHING_ELSE)
        self.g.write()
        self.fails("B2", SEVEN[2])

    def test_b2_flagged_test_with_no_hit_has_no_joined_row(self):
        # The seventh update test builds its own value and has no hit.
        self.g.seams = [s for s in self.g.seams if s[0] != SEVEN[6]]
        self.g.rows = self.g.rows[:6] + self.g.rows[7:]
        self.g.write()
        self.assertTrue(self.bar("B2")[0], self.bar("B2")[1])

    def test_b2_joined_row_that_disagrees_with_matched(self):
        joined = [list(j) for j in self.g.joined]
        joined[3][3] = mr.SOMETHING_ELSE
        self.g.write(joined=joined)
        self.fails("B2", SEVEN[3])

    def test_b3_hit_with_two_outcomes(self):
        self.g.rows.append(k.Row(f"{UPDATE}:10", outcome=k.WAIT))
        self.g.write()
        self.fails("B3", "2 outcomes")

    def test_b3_hit_with_no_outcome(self):
        self.g.rows = self.g.rows[1:]
        self.g.write()
        self.fails("B3", f"{UPDATE}:10 has no outcome")

    def test_b3_kind_split_by_hook_name(self):
        self.g.rows[0].outcome = "fake data (set_fetcher)"
        self.g.write()
        self.fails("B3", "not one of the six")

    def test_b3_outcome_for_a_line_the_search_never_hit(self):
        self.g.rows.append(k.Row(f"{UPDATE}:99", outcome=k.WAIT))
        self.g.write()
        self.fails("B3", "not a hit line of seams.tsv")

    def test_b4_five_questions(self):
        self.g.write(questions=self.g.questions() + "## fake data, part two\ntests: 1\n")
        self.fails("B4", "four in all")

    def test_b4_question_with_no_count(self):
        q = self.g.questions().replace("tests: 7\n", "", 1)
        self.g.write(questions=q)
        self.fails("B4", "no single count of tests")

    def test_b4_count_that_kinds_md_does_not_give(self):
        self.g.write(questions=self.g.questions().replace("tests: 7\n", "tests: 9\n", 1))
        self.fails("B4", "kinds.md gives 7")

    def test_b4_quote_not_at_the_cited_line(self):
        q = self.g.questions().replace(f"{UPDATE}:3 |", f"{UPDATE}:5 |", 1)
        self.g.write(questions=q)
        self.fails("B4", f"not at {UPDATE}:5")

    def test_b5_status_other_than_open(self):
        self.g.write(questions=self.g.questions().replace("status: open", "status: later", 1))
        self.fails("B5", "later")

    def test_b5_each_claim_word(self):
        for word in cb.CLAIM_WORDS:
            with self.subTest(word=word):
                self.g.write(questions=self.g.questions().replace("status: open", f"status: {word}", 1))
                self.fails("B5", word)
                hits = os.path.join(self.g.out, "hits.tsv")
                text = cb.text(hits)
                with open(hits, "w", encoding="utf-8") as f:
                    f.write(text.replace("\topen\n", f"\t{word}\n", 1))
                self.fails("B5", f"a claim word ({word})")
                self.g.write()

    def test_b6_uncovered_name_missing(self):
        self.g.write(questions=self.g.questions().replace("uncovered: set_all\n", ""))
        self.fails("B6", "set_all")

    def test_b6_unsorted_hit_missing(self):
        self.g.write(questions=self.g.questions().replace(f"unsorted: {PANEL_FILE}:6\n", ""))
        self.fails("B6", f"{PANEL_FILE}:6 is named in 0")

    def test_b7_record_in_neither_file(self):
        self.g.unmatched = []
        self.g.write()
        self.fails("B7", "B1.md:5 is in neither file")

    def test_b7_joined_record_missing_from_its_tests_row(self):
        joined = [list(j) for j in self.g.joined]
        joined[0][joined_column("records")] = ""
        self.g.write(joined=joined)
        self.fails("B7", "C13.md:551 is joined to")

    def test_b8_contradicted_with_no_answer(self):
        self.g.answers = [[k.FAKE, k.NOT_ANSWERED, ""]]
        joined = [list(j) for j in self.g.joined]
        self.g.write(joined=joined)
        for j in joined:
            j[4] = k.CONTRADICTED
        self.g.write(joined=joined)
        self.fails("B8", "with no answer")


class B8RealTests(unittest.TestCase):
    """B8 for all four kinds, each on a real test of the copy whose hits the
    rule sorts into that kind, read through kinds.py."""

    @classmethod
    def setUpClass(cls):
        if not os.path.isdir(os.path.join(REAL_COPY, "code")):
            raise unittest.SkipTest(f"the blind copy is not at {REAL_COPY}")
        lines = fs.Lines(REAL_COPY)
        cov = {c["name"]: c["declared_at"] for c in fs.coverage(REAL_COPY, lines, [])}
        cls.real = {
            # P33: test_panel_close_cancels's hits.
            PANEL: [(PANEL_FILE, 1084), ("tests/ui/uitest_harness.cpp", 206),
                    ("tests/ui/uitest_harness.cpp", 213), ("tests/ui/uitest_harness.cpp", 353),
                    ("tests/ui/uitest_harness.h", 196)],
            # P5: the first C13 update test's UpdateWorld.
            "every start of the installed exe checks, and a copy never does": [(UPDATE, 181)],
            # The between-reads hold of the plan's "test code run mid-step".
            "lookup hold": [("tests/test_app_library.cpp", 694)],
        }
        hit_lines = {h for hs in cls.real.values() for h in hs}
        hit_lines |= {(UPDATE, n) for n in (86, 95, 96, 105, 111)}
        sorter = k.Sorter(REAL_COPY, cov, hit_lines)
        cls.kinds = {t: sorted({r.outcome for p, n in hs for r in sorter.sort(p, n)} & set(k.KINDS))
                     for t, hs in cls.real.items()}

    def test_each_kind_from_both_ends(self):
        have = {kind: t for t, ks in self.kinds.items() for kind in ks}
        self.assertEqual(sorted(have), sorted(k.KINDS), self.kinds)
        for kind, test in have.items():
            ks = self.kinds[test]
            with self.subTest(kind=kind, test=test):
                stands = {kind: (k.STANDS, set())}
                self.assertNotEqual(k.result(mr.FLAGGED, ks, test, stands), k.CONTRADICTED)
                said = {kind: (k.CONTRADICTED, set())}
                self.assertEqual(k.result(mr.FLAGGED, ks, test, said), k.CONTRADICTED)
                named = {kind: (k.CONTRADICTED, {test})}
                self.assertNotEqual(k.result(mr.FLAGGED, ks, test, named), k.CONTRADICTED)

    def test_panel_close_cancels_is_not_contradicted_by_fake_data(self):
        answers = {k.FAKE: (k.CONTRADICTED, set())}
        self.assertNotIn(k.FAKE, self.kinds[PANEL])
        self.assertEqual(k.result(mr.FLAGGED, self.kinds[PANEL], PANEL, answers), mr.FLAGGED)
        first = "every start of the installed exe checks, and a copy never does"
        self.assertEqual(k.result(mr.FLAGGED, self.kinds[first], first, answers), k.CONTRADICTED)


def pilot2_seams(path):
    """Pilot 2's search output (ls4-grade.txt) in seams.tsv's columns: its
    `test:` lines give each test's file, its `hit:` lines each hit."""
    files, rows = {}, []
    for line in cb.text(path).split("\n"):
        parts = [x.strip() for x in line.split("|")]
        if line.startswith("test: "):
            files[parts[0][6:]] = parts[1].removeprefix("code/").split(":")[0]
        elif line.startswith("hit: "):
            name = parts[0][5:]
            hp, hn = parts[1].removeprefix("code/").split(":")
            rows.append([name, files.get(name, ""), "1", "yes", hp, hn, parts[2], "x"])
    return rows


class Pilot2(unittest.TestCase):
    def test_b1_to_b4_fail_on_pilot2(self):
        tmp = tempfile.mkdtemp()
        try:
            g = Good(tmp)
            g.seams = pilot2_seams(os.path.join(PILOT2, "ls4-grade.txt"))
            g.write()
            p = g.paths(joined=os.path.join(PILOT2, "pilot2-compare.md"),
                        hits=os.path.join(PILOT2, "pilot2-made-up.md"),
                        questions=os.path.join(PILOT2, "pilot2-compare.md"),
                        kinds_md=os.path.join(PILOT2, "pilot2-compare.md"))
            got = cb.check(p, ["B1", "B2", "B3", "B4"])
            for b in ("B1", "B2", "B3", "B4"):
                self.assertFalse(got[b][0], f"{b}: {got[b][1]}")
            # Each fails because pilot 2 wrote no such table, or no such
            # four questions, not because a file was missing.
            for b in ("B1", "B2", "B3"):
                self.assertIn("has no column", got[b][1], b)
            self.assertIn("not one per kind of flaw, four in all", got["B4"][1])
        finally:
            shutil.rmtree(tmp)


if __name__ == "__main__":
    unittest.main()
