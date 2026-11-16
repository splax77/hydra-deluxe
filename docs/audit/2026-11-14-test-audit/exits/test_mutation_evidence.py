"""Checks mutation_evidence.py against the copied run files and against a small made-up run.

Run: python -I test_mutation_evidence.py   (from any folder; it finds the audit folder itself)
Every expected value on the real files is read from the file it names, not typed from memory.
"""
import os
import re
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import mutation_evidence as me  # noqa: E402

AUD = os.path.dirname(HERE)
MUT = os.path.join(AUD, 'mutation')


def page_groups(text):
    """survivors-by-file.md as {file: (heading count, [lines in its block])}."""
    groups, cur, inside = {}, None, False
    for ln in text.splitlines():
        h = re.match(r'^## (.*) \((\d+)\)$', ln)
        if h:
            cur = h.group(1)
            groups[cur] = (int(h.group(2)), [])
        elif ln == '```':
            inside = not inside
        elif inside and cur is not None:
            groups[cur][1].append(ln)
    return groups


def read(path):
    with open(path, encoding='utf-8') as f:
        return f.read()


class OnTheCopiedRun(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.ev = me.read_evidence(AUD)
        cls.report = cls.ev['report']
        cls.page = read(os.path.join(MUT, me.SURVIVORS_FILE))
        cls.readme = read(os.path.join(MUT, me.README_FILE))

    def test_no_timed_out_block_is_lost(self):
        listed = me.timed_out_numbers(self.report)
        got = [b['number'] for b in self.ev['timed_out']]
        self.assertEqual(sorted(got), sorted(listed))
        self.assertEqual(len(got), self.report['by_state']['timed out'])

    def test_every_block_is_a_timed_out_bug_with_its_header_from_the_report(self):
        heads = {me.parse_bug_line(ln)['number']: ln for ln in self.report['sections'][me.TIMED_OUT_TITLE]['lines']}
        for b in self.ev['timed_out']:
            self.assertTrue(b['state'].startswith(me.TIMED_OUT_STATE), b['number'])
            self.assertEqual(b['header'], heads[b['number']])

    def test_no_survivor_is_lost_and_the_counts_add_up(self):
        groups = page_groups(self.page)
        on_page = [ln for _, lines in groups.values() for ln in lines]
        want = self.report['sections'][me.SURVIVORS_TITLE]['lines']
        self.assertEqual(sorted(on_page), sorted(want))
        self.assertEqual(len(on_page), self.report['by_state']['survived'])
        for f, (count, lines) in groups.items():
            self.assertEqual(count, len(lines), f)
            for ln in lines:
                self.assertEqual(me.parse_bug_line(ln)['file'], f)
        self.assertEqual(sum(c for c, _ in groups.values()), len(want))

    def test_the_survivors_page_opens_with_the_reports_limit_and_carries_the_limits(self):
        self.assertIn(me.SURVIVOR_LIMIT, self.report['limits'])
        body = self.page.split('\n## ', 1)[0]
        self.assertIn(me.SURVIVOR_LIMIT, body)
        self.assertIn(me.LIMITS, body)

    def test_the_readme_carries_the_limits_sentence_and_all_four_limits(self):
        self.assertIn(me.LIMITS, self.readme)
        for n in '1234':
            self.assertRegex(self.readme, rf'(?m)^{n}\. ')

    def test_the_readme_sizes_match_the_files(self):
        rows = dict(re.findall(r'^\| `([^`]+)` \| (\d+) \|', self.readme, re.M))
        names = me.WHOLE + (me.TIMED_OUT_FILE, me.SURVIVORS_FILE)
        self.assertEqual(sorted(rows), sorted(names))
        for name in names:
            self.assertEqual(int(rows[name]), os.path.getsize(os.path.join(MUT, name)), name)

    def test_the_tests_reader_holds_the_reports_count_of_tests_that_caught_none(self):
        none = [t for t, r in self.ev['tests'].items() if r['caught_none']]
        self.assertEqual(len(none), self.report['tests_caught_none'])
        self.assertTrue(all(self.ev['tests'][t]['caught'] == 0 for t in none))


# A made-up run, in report.py's shape, with Windows line ends so a copy that
# changes bytes is seen.
REPORT = """Mutation report for run R1.

Limits of this report.
A test that caught nothing has shown no way to fail against these bugs. That is a question to look at, not proof the test is empty.
No tool can say whether a survivor is an equivalent bug, one that changes nothing a user could see.

Counts.
Bug runs: 4: survived 2, timed out 2, crashed 0, incomplete 0, caught 0.
Tests in the mapping run: 2. Red with no bug: 0. Tests that caught no bug: 1.

== Survivors: bugs reached but caught by no test (2)
bug 1: src/a.cpp line 3, `>` becomes `>=`
bug 4: src/b.cpp line 9, `<` becomes `<=`

== Timed-out bugs, counted as caught but listed apart (2)
bug 2: src/a.cpp line 5, `==` becomes `!=`
bug 3: src/a.cpp line 7, `&&` becomes `||`
"""
BUGS = """Each bug run: its verdict and every test that went red in it.

bug 1: src/a.cpp line 3, `>` becomes `>=`
  survived its run, 1.0 s
  catches: 0

bug 2: src/a.cpp line 5, `==` becomes `!=`
  timed out in its run (counted as caught), 15.0 s
  catches: 0
  red in its file, not sorted because the run timed out: tests\\t.cpp:4 two
  running at the time limit: tests\\t.cpp:1 one

bug 3: src/a.cpp line 7, `&&` becomes `||`
  timed out in its run (counted as caught), 15.0 s
  catches: 0

bug 4: src/b.cpp line 9, `<` becomes `<=`
  survived its run, 1.0 s
  catches: 0
"""
TESTS = """Each test.

== Tests that caught no bug (1)
caught 0 | reaches 2 | tests\\t.cpp:1 one

== Tests that caught at least one bug (1)
caught 1 | reaches 1 | tests\\t.cpp:4 two
  bugs: 9
"""


class OnAMadeUpRun(unittest.TestCase):
    def make(self, report=REPORT, bugs=BUGS):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        run, aud = os.path.join(tmp.name, 'run'), os.path.join(tmp.name, 'aud')
        os.makedirs(run)
        for name, text in (('report.txt', report), ('bugs.txt', bugs), ('tests.txt', TESTS),
                           ('summary.json', '{"processes": 8}\n')):
            with open(os.path.join(run, name), 'wb') as f:
                f.write(text.replace('\n', '\r\n').encode('utf-8'))
        return run, aud

    def test_the_blocks_are_copied_byte_for_byte_and_the_whole_files_unchanged(self):
        run, aud = self.make()
        me.copy_run(run, aud)
        with open(os.path.join(me.mutation_dir(aud), me.TIMED_OUT_FILE), 'rb') as f:
            got = f.read()
        with open(os.path.join(run, 'bugs.txt'), 'rb') as f:
            src = f.read()
        b2 = src[src.index(b'bug 2:'):src.index(b'bug 3:')]
        b3 = src[src.index(b'bug 3:'):src.index(b'bug 4:')]
        self.assertEqual(got, b2 + b3)
        for name in me.WHOLE:
            with open(os.path.join(run, name), 'rb') as a, open(os.path.join(me.mutation_dir(aud), name), 'rb') as b:
                self.assertEqual(a.read(), b.read(), name)

    def test_the_reader_reads_back_what_the_blocks_say(self):
        run, aud = self.make()
        me.copy_run(run, aud)
        blocks = {b['number']: b for b in me.read_evidence(aud)['timed_out']}
        self.assertEqual(blocks[2]['running_at_limit'], 'tests\\t.cpp:1 one')
        self.assertEqual(blocks[2]['red_in_file'], ['tests\\t.cpp:4 two'])
        self.assertIsNone(blocks[3]['running_at_limit'])

    def test_a_lost_timed_out_block_stops_the_copy(self):
        lost = re.sub(r'bug 3:.*?\n\n', '', BUGS, flags=re.S)
        run, aud = self.make(bugs=lost)
        with self.assertRaises(me.EvidenceError):
            me.copy_run(run, aud)

    def test_a_report_whose_timed_out_count_differs_stops_the_copy(self):
        run, aud = self.make(report=REPORT.replace('timed out 2', 'timed out 3'))
        with self.assertRaises(me.EvidenceError):
            me.copy_run(run, aud)

    def test_a_lost_survivor_line_stops_the_copy(self):
        run, aud = self.make(report=REPORT.replace('bug 4: src/b.cpp line 9, `<` becomes `<=`\n', ''))
        with self.assertRaises(me.EvidenceError):
            me.copy_run(run, aud)

    def test_a_report_without_the_survivor_limit_stops_the_copy(self):
        run, aud = self.make(report=REPORT.replace(me.SURVIVOR_LIMIT + '\n', ''))
        with self.assertRaises(me.EvidenceError):
            me.copy_run(run, aud)

    def test_a_rewrite_keeps_the_run_folder_the_readme_names(self):
        run, aud = self.make()
        me.copy_run(run, aud)
        first = read(os.path.join(me.mutation_dir(aud), me.README_FILE))
        me.write_pages(aud)
        self.assertEqual(read(os.path.join(me.mutation_dir(aud), me.README_FILE)), first)


if __name__ == '__main__':
    unittest.main()
