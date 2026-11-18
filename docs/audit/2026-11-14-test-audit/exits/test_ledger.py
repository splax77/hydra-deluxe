"""Checks ledger.py against the audit's real record and count files.

Run: python -I test_ledger.py   (from any folder; it finds the audit folder itself)
Every expected value below is read from the file it names, not typed from memory.
"""
import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.argv = sys.argv[:1]
import ledger  # noqa: E402
import mutation_evidence  # noqa: E402

AUD = os.path.dirname(HERE)


def find(rows, cf, pattern):
    return [r for r in rows if r['cf'] == cf and re.search(pattern, r['text'] + ' ' + r['section'])]


class LedgerOnRealFiles(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.recs, cls.unread = ledger.read_records()
        cls.counts, _ = ledger.read_counts()
        cls.left = ledger.apply_corrections(cls.counts, ledger.read_corrections())
        cls.settle = ledger.apply_answers(cls.counts)
        cls.held_before = {id(r): list(r['faults']) for r in cls.settle}
        cls.settled = ledger.settle_answer4(cls.counts)
        ledger.join_counts(cls.counts, cls.recs)

    def test_superseded_wn4_is_not_read(self):
        self.assertFalse([r for r in self.recs if r['file'] == 'window-WN4.md'])
        self.assertTrue([r for r in self.recs if r['file'] == 'window-WN4a.md'])

    def test_every_verdict_line_of_a_record_file_becomes_a_row(self):
        # D8.md: count its "Verdict:" lines directly and compare with the script's rows.
        text = ledger.lines_of(os.path.join(AUD, 'records', 'D8.md'))
        lines_with_verdict = sum(1 for ln in text if ledger.VERDICT.search(ln))
        rows = [r for r in self.recs if r['file'] == 'D8.md']
        self.assertEqual(sum(len(r['verdicts']) for r in rows), lines_with_verdict)

    def test_every_verdict_line_in_every_record_file_lands_in_a_row(self):
        # No verdict is dropped or counted twice, whatever shape its record has.
        for name in sorted(os.listdir(os.path.join(AUD, 'records'))):
            if not name.endswith('.md') or name in ledger.SUPERSEDED:
                continue
            text = ledger.lines_of(os.path.join(AUD, 'records', name))
            found = sum(len(ledger.VERDICT.findall(ln)) for ln in text)
            rows = [r for r in self.recs if r['file'] == name and not r.get('bare')]
            self.assertEqual(sum(len(r['verdicts']) for r in rows), found, name)

    def by_start(self, name, hline):
        rows = [r for r in self.recs if r['file'] == name and r['hline'] == hline]
        self.assertEqual(len(rows), 1, (name, hline))
        return rows[0]

    def test_a_question_line_with_a_verdict_is_not_its_own_record(self):
        # records/D15.md: the bold title at line 88 (BP-8) owns the verdict on line 90,
        # which starts "- 3: real pid liveness". Line 90 starts no row.
        r = self.by_start('D15.md', 88)
        self.assertTrue(r['record'].startswith('BP-8'))
        self.assertEqual(r['vlines'], [90])
        self.assertFalse([x for x in self.recs if x['file'] == 'D15.md' and x['hline'] == 90])

    def test_records_written_as_bold_lines_are_one_row_each(self):
        # records/E3.md lines 864 to 866: one bold test name per line, each with its verdict.
        for ln in (864, 865, 866):
            r = self.by_start('E3.md', ln)
            self.assertEqual(r['vlines'], [ln])
            self.assertTrue(r['record'].startswith('test_comment_'))

    def test_a_record_read_line_does_not_steal_its_heading(self):
        # records/A10.md: the heading at line 24 is the record; line 26 is its "Read:" line.
        r = self.by_start('A10.md', 24)
        self.assertTrue(r['record'].startswith('1. collect_dm_rows'))
        self.assertFalse([x for x in self.recs if x['file'] == 'A10.md' and x['hline'] == 26])

    def test_a_heading_record_with_a_fenced_body_keeps_its_verdict(self):
        # records/A37.md: heading at line 20, body fenced from line 22, verdict on line 35.
        self.assertEqual(self.by_start('A37.md', 20)['vlines'], [35])

    def test_a_paragraph_record_followed_by_its_read_line(self):
        # records/A39.md line 29 is a record title; its next line is "Read:".
        self.assertTrue(self.by_start('A39.md', 29)['record'].startswith('test_cap_switch'))

    def joined(self, cf, line):
        rows = [r for r in self.counts if r['cf'] == cf and r['line'] == line]
        self.assertEqual(len(rows), 1, (cf, line))
        return rows[0]

    def test_join_by_test_line_in_parentheses(self):
        # counts/T5-IR2.md line 94 "2653 worker exit leaves backup" is the record at
        # records/D5.md line 471, "test_the_workers_exit_leaves_a_marker_backup_and_its_marker (2653)".
        self.assertEqual(self.joined('T5-IR2', 94)['join'], 'records/D5.md:471')

    def test_join_where_number_and_test_line_agree(self):
        # counts/T5-IR7.md line 26 "D11 21 | TP:400" is records/D11.md line 315,
        # "21. test_a_caller_given_cap_ends_the_run_there (:400)".
        self.assertEqual(self.joined('T5-IR7', 26)['join'], 'records/D11.md:315')

    def test_join_by_record_file_column(self):
        # counts/T5-KR4.md line 20 "delta-DL1 | 430 | :430" is records/delta-DL1.md line 287,
        # "430. a hold open at a batch's end loses nothing ...".
        self.assertEqual(self.joined('T5-KR4', 20)['join'], 'records/delta-DL1.md:287')

    def test_a_number_alone_does_not_join(self):
        # counts/T5-IR15.md line 80 "A18-24 | :779": A18's record 24 cites test_model.cpp:689,
        # and no A18 record title cites 779, so the row cannot be checked.
        r = self.joined('T5-IR15', 80)
        self.assertEqual(r['join'], '')
        self.assertIn('record number', r['why'])

    def test_a_row_with_no_record_file_is_unjoined(self):
        # counts/T5-IR1.md line 20 names only a test, under "test_check_handoff.py (20 records)".
        r = self.joined('T5-IR1', 20)
        self.assertEqual(r['join'], '')
        self.assertEqual(r['why'], 'names no record file')

    def test_every_join_names_a_real_record_row_once_per_count_file(self):
        ids = {f"records/{r['file']}:{r['hline']}" for r in self.recs}
        seen = set()
        for r in self.counts:
            if r['join']:
                self.assertIn(r['join'], ids)
                self.assertNotIn((r['cf'], r['join']), seen)
                seen.add((r['cf'], r['join']))
            else:
                self.assertTrue(r['why'])

    def test_a_byreading_verdict_reaches_its_record(self):
        # capture/byreading/B1-verdicts.md line 13: "A17 17 | test_library_query.cpp:313 | ... |
        # HOLDS BY READING" is records/A17.md line 316, "17. library query: unknown field names
        # are words ... (tests/test_library_query.cpp:313)".
        rows = ledger.second_verdicts(self.recs)
        hit = [r for r in rows if r['cf'] == 'byreading/B1-verdicts' and r['line'] == 13]
        self.assertEqual(len(hit), 1)
        self.assertEqual(hit[0]['join'], 'records/A17.md:316')
        self.assertEqual(self.by_start('A17.md', 316)['byreading'], ['HOLDS BY READING'])

    def test_count_rows_match_totals_where_totals_reports_no_gap(self):
        # TOTALS.md section 4 names the files with a gap; these four are not among them.
        # TOTALS is from before answer 4, so the rows answer4.tsv takes to none are added back.
        tot = ledger.totals_section1()
        cleared = sum(1 for s in ledger.read_answer4(ANSWER4) if s['faults_after'] == 'none')
        for cf in ('T5-KR1', 'T5-IR4', 'T5-IR10', 'T5-IR21'):
            rows = [r for r in self.counts if r['cf'] == cf]
            back = cleared if cf == 'T5-IR4' else 0
            self.assertEqual(len(rows), tot[cf][0], cf)
            self.assertEqual(sum(1 for r in rows if r['faults']) + back, tot[cf][1], cf)

    def test_a_totals_removal_is_applied(self):
        # TOTALS.md section 3: "T5-KR2, record 2 test_review_dispatch.py:211: X9 to none".
        rows = find(self.counts, 'T5-KR2', r'test_review_dispatch\.py:211\b')
        self.assertEqual(len(rows), 1)
        self.assertNotIn('X9', rows[0]['faults'])
        self.assertTrue(rows[0]['fix'].startswith('TOTALS s3'))

    def test_answer_2_puts_x9_back_on_kr3(self):
        rows = find(self.counts, 'T5-KR3', r'test_frida_attach\.py:976\b')
        self.assertEqual(len(rows), 1)
        self.assertIn('X9', rows[0]['faults'])
        self.assertEqual(rows[0]['answer'], 'answer 2')

    def test_answer_3_puts_x9_back_on_ir1(self):
        rows = find(self.counts, 'T5-IR1', r'test_result_id_order_alone_compares_clean')
        self.assertEqual(len(rows), 1)
        self.assertIn('X9', rows[0]['faults'])
        self.assertEqual(rows[0]['answer'], 'answer 3')

    def test_answer_4_held_ir4_rd_rows_with_x9_until_settled(self):
        self.assertTrue(self.settle)
        for r in self.settle:
            self.assertEqual(r['cf'], 'T5-IR4')
            self.assertIn('X9', self.held_before[id(r)])
            self.assertTrue(r['rec'].startswith('RD') or r['text'].startswith('RD'))

    def test_answer_4_rows_carry_the_settled_faults_and_are_not_held(self):
        settles = ledger.read_answer4(ANSWER4)
        self.assertEqual(len(settles), len(self.settle))
        self.assertEqual({id(r) for r, _ in self.settled}, {id(r) for r in self.settle})
        for s in settles:
            r = self.joined('T5-IR4', int(s['line']))
            self.assertEqual(s['file'], 'counts/T5-IR4.md')
            self.assertEqual(ledger.faults_text(self.held_before[id(r)]), s['faults_before'])
            self.assertEqual(ledger.faults_text(r['faults']), s['faults_after'])
            self.assertEqual(r['answer'], ledger.ANSWER4_DONE)
            self.assertEqual(ledger.held(r), '')
        self.assertFalse([r for r in self.counts if ledger.held(r)])


ANSWER4 = os.path.join(AUD, 'exits', ledger.ANSWER4_FILE)


class Answer4SettleChecks(unittest.TestCase):
    """A settle file that does not fit the ledger stops the script, naming the row."""

    def setUp(self):
        self.counts, _ = ledger.read_counts()
        ledger.apply_corrections(self.counts, ledger.read_corrections())
        ledger.apply_answers(self.counts)
        self.lines = ledger.lines_of(ANSWER4)
        self.tmp = tempfile.mkdtemp()

    def tearDown(self):
        shutil.rmtree(self.tmp)

    def settle_with(self, lines):
        path = os.path.join(self.tmp, 'answer4.tsv')
        with open(path, 'w', encoding='utf-8', newline='\n') as fh:
            fh.write('\n'.join(lines) + '\n')
        return ledger.settle_answer4(self.counts, path)

    def edit_first(self, k, value):
        cells = self.lines[1].split('\t')
        cells[k] = value
        return [self.lines[0], '\t'.join(cells)] + self.lines[2:], cells

    def test_the_real_file_settles(self):
        self.assertEqual(len(self.settle_with(self.lines)), len(self.lines) - 1)

    def test_a_row_matching_no_ledger_row_fails(self):
        lines, cells = self.edit_first(1, '99999')
        with self.assertRaisesRegex(ledger.LedgerError, r'counts/T5-IR4\.md:99999 .*matches 0 ledger rows'):
            self.settle_with(lines)

    def test_a_row_whose_faults_before_differ_fails(self):
        lines, cells = self.edit_first(3, 'X12')
        with self.assertRaisesRegex(ledger.LedgerError, re.escape(f'{cells[0]}:{cells[1]} ') + r'.*faults_before is X12'):
            self.settle_with(lines)

    def test_a_held_row_with_no_settle_line_fails(self):
        cells = self.lines[-1].split('\t')
        with self.assertRaisesRegex(ledger.LedgerError, r'^1 rows wait for answer 4 .*' + re.escape(f'{cells[0]}:{cells[1]} ')):
            self.settle_with(self.lines[:-1])
        self.assertTrue([r for r in self.counts if ledger.held(r)])

    def test_the_script_exits_non_zero_on_a_bad_settle_file(self):
        # A copy of the folders the script reads before it settles answer 4, with one
        # settle row broken. The script must stop before it writes the ledger.
        aud2 = os.path.join(self.tmp, 'aud')
        for d in ('records', 'counts'):
            shutil.copytree(os.path.join(AUD, d), os.path.join(aud2, d))
        os.makedirs(os.path.join(aud2, 'exits'))
        lines, _ = self.edit_first(1, '99999')
        with open(os.path.join(aud2, 'exits', ledger.ANSWER4_FILE), 'w', encoding='utf-8', newline='\n') as fh:
            fh.write('\n'.join(lines) + '\n')
        p = subprocess.run([sys.executable, '-I', os.path.join(HERE, 'ledger.py'), aud2,
                            os.path.join(aud2, 'exits', 'ledger.tsv')], capture_output=True, encoding='utf-8')
        self.assertNotEqual(p.returncode, 0)
        self.assertIn('counts/T5-IR4.md:99999', p.stderr)
        self.assertFalse(os.path.exists(os.path.join(aud2, 'exits', 'ledger.tsv')))

    def test_answer_5_rows_keep_x9(self):
        rows = [r for r in self.counts if r['answer'].startswith('answer 5')]
        self.assertTrue(any('test_fits_screen' in r['text'] for r in rows))
        self.assertTrue(any(r['text'].startswith('616 ') for r in rows))
        for r in rows:
            self.assertIn('X9', r['faults'])


MATCH_VALUES = ('name', 'line', 'line, file changed since', 'not in the run', 'none', 'several')
# The E1 ledger, before the mut_ columns: the commit the plan's task M4 names for it.
E1_COMMIT = '891ab424'
LEDGER_IN_REPO = 'docs/audit/2026-11-14-test-audit/exits/ledger.tsv'


def git_show(spec):
    root = os.path.dirname(os.path.dirname(os.path.dirname(AUD)))
    return subprocess.run(['git', 'show', spec], cwd=root, capture_output=True, check=True,
                          encoding='utf-8').stdout


class MutationColumnsOnRealFiles(unittest.TestCase):
    """Plan step E1c, task M3's acceptance, on the real records and the copied run files."""

    @classmethod
    def setUpClass(cls):
        cls.recs, _ = ledger.read_records()
        cls.ev = mutation_evidence.read_evidence(AUD)
        cls.placed = ledger.read_placed(AUD)
        cls.entries = ledger.mutation_columns(cls.recs, cls.ev, cls.placed, ledger.read_changed(AUD))
        cls.tmp = tempfile.mkdtemp()
        path = os.path.join(cls.tmp, ledger.UNMATCHED)
        cls.left = ledger.write_unmatched(path, cls.recs, cls.ev)
        with open(path, encoding='utf-8') as fh:
            cls.unmatched = fh.read()
        cls.ledger_lines = ledger.lines_of(os.path.join(AUD, 'exits', 'ledger.tsv'))

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tmp)

    def matched(self):
        return [r for r in self.recs if r['mut']['mut_match'] in ledger.MATCHED]

    def test_every_record_row_has_one_match_and_every_unmatched_row_is_listed(self):
        from collections import Counter
        c = Counter(r['mut']['mut_match'] for r in self.recs)
        self.assertLessEqual(set(c), set(MATCH_VALUES))
        self.assertEqual(sum(c.values()), len(self.recs))
        for r in self.recs:
            if r['mut']['mut_match'] not in ledger.MATCHED:
                self.assertIn(f'- records/{r["file"]}:{r["hline"]} ', self.unmatched, r['title'][:80])
                self.assertTrue(r['mut_why'])

    def test_every_run_test_no_record_names_is_listed(self):
        named = {r['mut']['mut_test'] for r in self.matched()}
        for k in self.ev['tests']:
            if k not in named:
                self.assertIn(f'\n- {k}\n', self.unmatched)
        self.assertEqual(len(self.left), sum(1 for k in self.ev['tests'] if k not in named))

    def test_the_committed_unmatched_list_and_ledger_are_current(self):
        with open(os.path.join(AUD, 'mutation', ledger.UNMATCHED), encoding='utf-8') as fh:
            self.assertEqual(fh.read(), self.unmatched)
        old = ledger.read_old_mut(os.path.join(AUD, 'exits', 'ledger.tsv'))
        self.assertEqual(ledger.mut_changes(old, self.recs), [])

    def test_matched_rows_carry_their_tests_counts(self):
        self.assertTrue(self.matched())
        for r in self.matched():
            t = self.ev['tests'][r['mut']['mut_test']]
            self.assertEqual(r['mut']['mut_caught'], str(t['caught']))
            self.assertEqual(r['mut']['mut_reaches'], str(t['reaches']))
            self.assertEqual(r['mut']['mut_caught_none'], 'yes' if t['caught_none'] else 'no')

    def test_caught_none_tests_marked_plus_unmatched_come_to_the_reports_count(self):
        yes = {r['mut']['mut_test'] for r in self.matched() if r['mut']['mut_caught_none'] == 'yes'}
        left_none = [k for k in self.left if self.ev['tests'][k]['caught_none']]
        self.assertEqual(len(yes) + len(left_none), self.ev['report']['tests_caught_none'])

    def test_rows_not_matched_to_one_test_hold_no_evidence(self):
        for r in self.recs:
            if r['mut']['mut_match'] not in ledger.MATCHED:
                for c in ledger.MUT_EVIDENCE:
                    self.assertEqual(r['mut'][c], '', (r['file'], r['hline'], c))

    def test_every_row_with_a_test_holds_the_limits_sentence(self):
        for r in self.matched():
            self.assertTrue(r['mut']['mut_test'])
            self.assertEqual(r['mut']['mut_limits'], mutation_evidence.LIMITS)
            self.assertEqual(r['mut']['mut_run'], self.ev['run'])
        for n in ('(1)', '(2)', '(3)', '(4)'):
            self.assertIn(n, mutation_evidence.LIMITS)

    def test_running_at_limit_over_all_tests_comes_to_the_blocks_that_name_one(self):
        per = {r['mut']['mut_test']: int(r['mut']['mut_running_at_limit']) for r in self.matched()}
        counts = ledger.per_test_counts(self.ev, ledger.run_index(self.ev))
        total = sum(per.values()) + sum(counts['running'][k] for k in self.left)
        self.assertEqual(total, sum(1 for b in self.ev['timed_out'] if b['running_at_limit']))

    def test_no_timed_out_bug_is_credited_as_a_catch(self):
        timed_out = {b['number'] for b in self.ev['timed_out']}
        for k, t in self.ev['tests'].items():
            self.assertFalse(timed_out & set(t['bugs']), k)

    def test_a_changed_timed_out_file_changes_exactly_its_tests_rows(self):
        # Move one block's running test to another test, in a copy of mutation/.
        src = os.path.join(AUD, 'mutation')
        aud2 = os.path.join(self.tmp, 'aud')
        os.makedirs(os.path.join(aud2, 'mutation'))
        for name in mutation_evidence.WHOLE:
            shutil.copyfile(os.path.join(src, name), os.path.join(aud2, 'mutation', name))
        block = next(b for b in self.ev['timed_out'] if b['running_at_limit']
                     and any(r['mut']['mut_test'] == b['running_at_limit'] for r in self.matched()))
        was = block['running_at_limit']
        other = next(r['mut']['mut_test'] for r in self.matched() if r['mut']['mut_test'] != was)
        with open(os.path.join(src, mutation_evidence.TIMED_OUT_FILE), 'rb') as fh:
            raw = fh.read().decode('utf-8')
        head = block['header']
        at = raw.index(head + '\n') if head + '\n' in raw else raw.index(head + '\r\n')
        end = raw.index('running at the time limit: ' + was, at)
        raw = raw[:end] + raw[end:].replace(was, other, 1)
        with open(os.path.join(aud2, 'mutation', mutation_evidence.TIMED_OUT_FILE), 'wb') as fh:
            fh.write(raw.encode('utf-8'))
        ev2 = mutation_evidence.read_evidence(aud2)
        recs2, _ = ledger.read_records()
        ledger.mutation_columns(recs2, ev2, self.placed, ledger.read_changed(AUD))
        old = {f"records/{r['file']}:{r['hline']}": r['mut'] for r in self.recs}
        changed = ledger.mut_changes(old, recs2)
        want = {f"records/{r['file']}:{r['hline']}" for r in self.recs if r['mut']['mut_test'] in (was, other)}
        self.assertEqual({c[0] for c in changed}, want)
        for _, diff, _, _ in changed:
            self.assertEqual(diff, ['mut_running_at_limit'])

    def test_count_rows_hold_no_mut_value_of_their_own(self):
        head = self.ledger_lines[0].split('\t')
        ix = [head.index(c) for c in ledger.MUT_COLS]
        kinds = set()
        for ln in self.ledger_lines[1:]:
            cells = ln.split('\t')
            kinds.add(cells[0])
            if cells[0] != 'record':
                self.assertEqual([cells[i] for i in ix], [''] * len(ix), ln[:80])
        self.assertEqual(kinds, {'record', 'count'})

    def test_the_e1_columns_and_rows_are_unchanged(self):
        # Two later steps are allowed for: the held column (E2), read past by name, and
        # answer 4's rows, whose faults and answer come from answer4.tsv.
        before = git_show(f'{E1_COMMIT}:{LEDGER_IN_REPO}').splitlines()
        self.assertEqual(len(before), len(self.ledger_lines))
        head_b = before[0].split('\t')
        head_n = self.ledger_lines[0].split('\t')
        keep = [head_n.index(c) for c in head_b]
        settled = {(s['file'], s['line']): s for s in ledger.read_answer4(ANSWER4)}
        changed = 0
        for b, n in zip(before, self.ledger_lines):
            bc, nc = b.split('\t'), n.split('\t')
            got = [nc[i] for i in keep]
            s = settled.get((bc[1], bc[2]))
            if s:
                changed += 1
                self.assertEqual(bc[head_b.index('faults')], s['faults_before'])
                self.assertEqual(got[head_b.index('faults')], s['faults_after'])
                self.assertEqual(got[head_b.index('answer')], ledger.ANSWER4_DONE)
                for c in ('faults', 'answer'):
                    got[head_b.index(c)] = bc[head_b.index(c)]
            self.assertEqual(got, bc)
        self.assertEqual(changed, len(settled))

    def test_every_ledger_row_has_a_held_column_right_after_answer_and_none_is_held(self):
        head = self.ledger_lines[0].split('\t')
        self.assertEqual(head.index('held'), head.index('answer') + 1)
        for ln in self.ledger_lines[1:]:
            cells = ln.split('\t')
            self.assertEqual(len(cells), len(head), ln[:80])
            self.assertEqual(cells[head.index('held')], '', ln[:80])

    def test_line_records_go_out_in_first_use_columns(self):
        path = os.path.join(AUD, 'mutation', ledger.LINE_RECORDS_TXT)
        rows = [ln.split('\t') for ln in ledger.lines_of(path) if ln and not ln.startswith('#')]
        self.assertEqual(rows, [[s, str(md), f, str(ln), src] for s, md, f, ln, src in self.entries])
        for row in rows:
            self.assertTrue(row[3].isdigit())          # first_use.py reads the fourth column as the line
        self.assertNotIn('not placed', ''.join(ledger.lines_of(path)).lower())   # its section switch

    def test_first_use_tables_rows_are_read(self):
        # F1's table, written by first_use.py (merge 1acf6a8f), in the shape line-records.md takes.
        table = git_show('1acf6a8f:tools/mutate/first_use_table.md').splitlines()
        rows = [ln for ln in table if ln.startswith('| ') and not ln.startswith('| Slice')]
        self.assertTrue(rows)
        os.makedirs(os.path.join(self.tmp, 'placed', 'mutation'))
        with open(os.path.join(self.tmp, 'placed', 'mutation', ledger.LINE_RECORDS_MD), 'w', encoding='utf-8') as fh:
            fh.write('\n'.join(table) + '\n')
        placed = ledger.read_placed(os.path.join(self.tmp, 'placed'))
        for ln in rows:
            self.assertTrue(ledger.PLACED_ROW.match(ln), ln)
            cells = [c.strip() for c in ln.strip('|').split('|')]
            f, n = cells[2].rsplit(':', 1)
            self.assertEqual(placed[(cells[0], int(cells[1]), f, int(n))], cells[3])

    def test_a_placed_line_matches_one_test_or_says_why_not(self):
        rec_entries = {}
        for e in self.entries:
            rec_entries.setdefault((e[0], e[1]), []).append(e)
        es = next(v for v in rec_entries.values() if len(v) == 1)
        f = es[0][2]
        in_file = [t for t in ledger.run_index(self.ev).values() if t['file'] == f]
        key = lambda e: (e[0], e[1], e[2], e[3])
        tests = ledger.run_index(self.ev)
        one = ledger.place_lines(es, {key(es[0]): in_file[0]['name']}, tests, set())
        self.assertEqual(one[:2], ('line', in_file[0]['key']))
        self.assertEqual(ledger.place_lines(es, {key(es[0]): in_file[0]['name']}, tests, {f})[0],
                         'line, file changed since')
        self.assertEqual(ledger.place_lines(es, {key(es[0]): ledger.OUTSIDE}, tests, set())[0], 'none')
        self.assertEqual(ledger.place_lines(es, {}, tests, set())[0], 'none')
        self.assertEqual(ledger.place_lines(es, None, tests, set())[0], 'none')
        two = next(v for v in rec_entries.values() if len(v) == 2)
        f2 = [t for t in tests.values() if t['file'] == two[0][2]]
        if len(f2) > 1:
            got = ledger.place_lines(two, {key(two[0]): f2[0]['name'], key(two[1]): f2[1]['name']}, tests, set())
            self.assertEqual(got[0], 'several')
            self.assertEqual(len(got[3]), 2)


class NotesOnRealFiles(unittest.TestCase):
    """Plan step E5: notes.tsv holds every side-note, split and production-code block."""

    @classmethod
    def setUpClass(cls):
        cls.recs, _ = ledger.read_records()
        cls.rows, _ = ledger.read_notes(AUD, cls.recs)
        cls.committed = ledger.lines_of(os.path.join(AUD, 'exits', 'notes.tsv'))

    def at(self, kind, rel, line):
        rows = [r for r in self.rows if r['kind'] == kind and r['file'] == rel and r['line'] == line]
        self.assertEqual(len(rows), 1, (kind, rel, line))
        return rows[0]

    def source_line(self, rel, line):
        return ledger.lines_of(os.path.join(AUD, rel))[line - 1]

    def test_a_side_note_has_a_row_under_its_record(self):
        # records/A10.md line 66: "Splits: none. Side notes: the name has a semicolon, no comma.",
        # in record 2, whose heading is line 49.
        r = self.at('side note', 'records/A10.md', 66)
        self.assertEqual(r['text'], ledger.clean(self.source_line('records/A10.md', 66)))
        self.assertTrue(r['record_heading'].startswith('2. collect_dm_rows'))
        self.assertEqual(r['ledger_id'], 'records/A10.md:49')

    def test_a_split_has_a_row(self):
        # records/A1.md line 61: "Split: the sample row's `acts`, `skip` and `sqin` disagree ...".
        r = self.at('split', 'records/A1.md', 61)
        self.assertTrue(r['text'].startswith('Split: the sample row'))

    def test_a_production_code_finding_has_a_row(self):
        # records/A1.md line 59: "Finding (production code): none. ...", a heading form the
        # plan's phrase alone would miss.
        r = self.at('production-code finding', 'records/A1.md', 59)
        self.assertTrue(r['text'].startswith('Finding (production code): none.'))

    def test_a_block_stops_at_the_blank_line(self):
        # records/A1.md line 60 is blank, so line 59's block does not reach line 61's split.
        self.assertEqual(self.source_line('records/A1.md', 60).strip(), '')
        r = self.at('production-code finding', 'records/A1.md', 59)
        self.assertEqual(r['text'], ledger.clean(self.source_line('records/A1.md', 59)))
        self.assertNotIn('Split:', r['text'])

    def test_a_block_runs_on_to_the_blank_line_or_heading(self):
        lines = ['## R1', 'Side note: one', 'two\tthree', '', 'four', 'Split: a', 'b', '### R2', 'c']
        got = ledger.note_blocks(lines)
        self.assertEqual(got, [('side note', 2, 'R1', 1, 'Side note: one two three'),
                               ('split', 6, 'R1', 1, 'Split: a b')])

    def test_a_note_written_as_a_heading_takes_its_body_to_the_next_heading(self):
        lines = ['## R1', '', '## Side notes', '', 'a', '', 'b', '## R2', 'c']
        got = ledger.note_blocks(lines)
        self.assertEqual(got, [('side note', 3, 'R1', 1, '## Side notes a b')])

    def test_production_finding_without_the_word_code_is_a_hit(self):
        pat = dict(ledger.NOTE_KINDS)['production-code finding']
        for text in ('Production finding: none.', '## Production findings and splits',
                     'Finding (production, own line): no caller', 'Production-code finding: x',
                     'Finding (production code): none.'):
            self.assertTrue(pat.search(text), text)
        self.assertFalse(pat.search('Finding: none against src.'))
        r = self.at('production-code finding', 'records/A1.md', 59)
        self.assertEqual(r['kind'], 'production-code finding')
        self.assertTrue([x for x in self.rows if x['kind'] == 'production-code finding'
                         and x['text'].startswith('Production finding:')])

    def test_every_hit_line_in_the_note_folders_has_a_row(self):
        hits = 0
        for rel in ledger.note_files(AUD):
            for n, ln in enumerate(ledger.lines_of(os.path.join(AUD, rel)), 1):
                for kind, pat in ledger.NOTE_KINDS:
                    if pat.search(ln):
                        hits += 1
                        self.at(kind, rel, n)
        self.assertEqual(hits, len(self.rows))

    def test_capture_subfolders_are_read(self):
        self.assertTrue([r for r in self.rows if r['file'].startswith('capture/byreading/')])
        self.assertTrue([r for r in self.rows if r['file'].startswith('capture/rejudge/')])

    def test_the_committed_notes_file_is_current(self):
        self.assertEqual(self.committed[0].split('\t'), ledger.NOTE_COLS)
        self.assertEqual(self.committed[1:], ['\t'.join(str(r[c]) for c in ledger.NOTE_COLS) for r in self.rows])


class ExitRulesOnBuiltInput(unittest.TestCase):
    """Plan step E7: ledger.py reads the rule tables of a rules.md and applies them. Every
    rules.md here is built in the test; the real one is never edited."""

    # Exit cells for the built tables. A rule not named here gives exit 2.
    EXITS = {'R6': 'none', 'R3': '4', 'R7': '`no rule`', 'C2': "its record's exit", 'N0': 'not a row',
             'N1': 'not a row of its own', 'Xc': '4 only with a Closures sentence; otherwise `no rule`',
             'L1': '`no rule`'}

    def rules_text(self, ledger_order=None, extra='', closures='', quotes='', takes=None):
        order = ledger_order or list(ledger.LEDGER_RULES)
        takes = takes or {}

        def table(ids):
            out = ['| Rule | Takes | Exit | Group | Required by |', '|---|---|---|---|---|']
            for i in ids:
                out.append(f'| {i} | {takes.get(i, "rows")} | {self.EXITS.get(i, "2")} | g | test |')
            return out
        lines = ['# rules', '', f'## {ledger.LEDGER_SECTION}', ''] + table(order) + [extra, '',
                 f'## {ledger.OUTSIDE_SECTION}', ''] + table(list(ledger.OUTSIDE_RULES)) + [
                 '', '## Closures', '', 'The main session writes one line here.', closures, '',
                 '## Exit 3 quotes', '', 'The main session pastes one block here.', quotes, '']
        return '\n'.join(lines).split('\n')

    COLS = ['kind', 'file', 'line', 'record', 'verdict', 'faults', 'held', 't7_reopened', 'id',
            'joins_record', 'verifier_verdict_if_different', 'byreading_verdict']

    def rec(self, file, line, verdict, **kw):
        r = dict({c: '' for c in self.COLS}, kind='record', file=file, line=str(line), verdict=verdict,
                 id=f'{file}:{line}')
        r.update(kw)
        return r

    def count(self, file, line, faults, joins=''):
        return dict({c: '' for c in self.COLS}, kind='count', file=file, line=str(line), faults=faults,
                    joins_record=joins)

    def label(self, row, keys='', no_key=''):
        return {row: {'row': row, 'keys': keys, 'no_key': no_key}}

    def run_rules(self, rows, labels=None, inv=None, outside=(), **text):
        rules = ledger.parse_rules(self.rules_text(**text))
        led, out = ledger.apply_exits(rules, rows, labels or {}, inv or {}, list(outside))
        return {d['id']: d for d in led + out}

    def test_a_rewrite_record_takes_exit_2_in_the_group_of_its_key(self):
        r = self.rec('records/A.md', 5, 'Rewrite')
        d = self.run_rules([r], labels=self.label(r['id'], 'C-stub'))[r['id']]
        self.assertEqual((d['exit'], d['rule'], d['groups']), ('2', 'R1a', ['C-stub']))

    def test_a_label_that_says_no_key_is_a_group_of_its_own(self):
        r = self.rec('records/A.md', 5, 'Remove')
        d = self.run_rules([r], labels=self.label(r['id'], '', 'yes'))[r['id']]
        self.assertEqual(d['groups'], ['NK:records/A.md:5'])

    def test_the_first_rule_in_table_order_decides(self):
        # A Rewrite record the verifier disagrees with: R4 and R1a both take it.
        r = self.rec('records/A.md', 5, 'Rewrite', verifier_verdict_if_different='Keep')
        lab = self.label(r['id'], 'C-stub')
        order = list(ledger.LEDGER_RULES)
        self.assertLess(order.index('R4'), order.index('R1a'))
        self.assertEqual(self.run_rules([r], labels=lab)[r['id']]['rule'], 'R4')
        order.remove('R1a')
        order.insert(order.index('R4'), 'R1a')
        self.assertEqual(self.run_rules([r], labels=lab, ledger_order=order)[r['id']]['rule'], 'R1a')

    def test_a_held_row_gets_no_exit(self):
        r = self.rec('records/A.md', 5, 'Rewrite', held='yes')
        d = self.run_rules([r], labels=self.label(r['id'], 'C-stub'))[r['id']]
        self.assertEqual((d['exit'], d['rule'], d['groups']), ('none', 'R6', []))

    def test_a_clean_keep_record_and_its_clean_count_row_take_exit_4(self):
        r = self.rec('records/A.md', 5, 'Keep')
        c = self.count('counts/T5-KR1.md', 9, 'none', r['id'])
        ds = self.run_rules([r, c])
        self.assertEqual((ds[r['id']]['exit'], ds[r['id']]['rule']), ('4', 'R3'))
        self.assertEqual((ds['counts/T5-KR1.md:9']['exit'], ds['counts/T5-KR1.md:9']['rule']), ('4', 'C2'))

    def test_a_faulty_count_row_sends_its_keep_record_to_its_keys(self):
        r = self.rec('records/A.md', 5, 'Keep')
        c = self.count('counts/T5-KR1.md', 9, 'X9', r['id'])
        ds = self.run_rules([r, c], labels=self.label('counts/T5-KR1.md:9', 'P-message'))
        self.assertEqual((ds[r['id']]['exit'], ds[r['id']]['rule'], ds[r['id']]['groups']),
                         ('2', 'R2', ['P-message']))
        self.assertEqual(ds['counts/T5-KR1.md:9']['rule'], 'C1')

    def test_a_clean_count_row_that_joins_no_record_is_no_rule(self):
        c = self.count('counts/T5-KR1.md', 9, 'none')
        d = self.run_rules([c])['counts/T5-KR1.md:9']
        self.assertEqual((d['exit'], d['rule']), (ledger.NO_RULE, 'R7'))

    def test_a_same_as_record_takes_its_targets_group(self):
        target = self.rec('records/D.md', 90, 'Investigate', record='test_a (:98)')
        ptr = self.rec('records/D.md', 120, 'Investigate', record='test_b (:112)')
        inv = {target['id']: {'group': 'I-split', 'how': 'read', 'verdict': 'Verdict: Investigate.'},
               ptr['id']: {'group': 'I-pointer', 'how': 'read', 'verdict': 'Verdict: Investigate: same as :98.'}}
        d = self.run_rules([target, ptr], inv=inv)[ptr['id']]
        self.assertEqual((d['rule'], d['groups'], d['points_at']), ('R1p', ['I-split'], target['id']))

    def test_a_same_as_record_with_no_target_is_its_own_group(self):
        ptr = self.rec('records/D.md', 120, 'Investigate', record='test_b (:112)')
        inv = {ptr['id']: {'group': 'I-pointer', 'how': 'read', 'verdict': 'same as :98.'}}
        self.assertEqual(self.run_rules([ptr], inv=inv)[ptr['id']]['groups'], ['NK:records/D.md:120'])

    def cross(self, route):
        return {'id': 'X1.c', 'source': 'cross-rows.md', 'where': 'cross-rows.md line 24', 'route': route,
                'finding': 'X1', 'kind': 'sound'}

    def test_a_closure_row_is_no_rule_until_closures_holds_its_sentence(self):
        d = self.run_rules([], outside=[self.cross('closure?')])['X1.c']
        self.assertEqual((d['exit'], d['rule']), (ledger.NO_RULE, 'Xc'))
        d = self.run_rules([], outside=[self.cross('closure?')],
                           closures='- X1.c: the key only looks up the engine\'s own path.')['X1.c']
        self.assertEqual((d['exit'], d['rule']), ('4', 'Xc'))

    def test_a_closure_line_for_a_row_not_routed_closure_is_refused(self):
        with self.assertRaises(ledger.RulesError):
            self.run_rules([], outside=[self.cross('2')], closures='- X1.c: a sentence.')

    def test_a_cross_row_routed_2_takes_its_finding_as_its_group(self):
        d = self.run_rules([], outside=[self.cross('2')])['X1.c']
        self.assertEqual((d['exit'], d['groups']), ('2', ['X1']))

    def test_a_note_marked_empty_is_not_a_row_and_a_split_note_is_a_question(self):
        rows = [{'id': 'notes/marks.tsv:2', 'source': 'notes/marks.tsv', 'where': 'records/A.md:3', 'mark': 'empty'},
                {'id': 'notes/marks.tsv:3', 'source': 'notes/marks.tsv', 'where': 'records/A.md:4', 'mark': 'split'}]
        ds = self.run_rules([], outside=rows)
        self.assertEqual(ds['notes/marks.tsv:2']['exit'], ledger.NOT_A_ROW)
        self.assertEqual((ds['notes/marks.tsv:3']['exit'], ds['notes/marks.tsv:3']['groups']), ('2', ['N-split']))

    def test_a_repeat_note_is_a_row_of_its_record_only_when_it_names_a_ledger_row(self):
        r = self.rec('records/A.md', 5, 'Rewrite')
        rows = [{'id': 'notes/marks.tsv:2', 'source': 'notes/marks.tsv', 'where': 'a', 'mark': 'repeat',
                 'ledger_id': r['id']},
                {'id': 'notes/marks.tsv:3', 'source': 'notes/marks.tsv', 'where': 'b', 'mark': 'repeat',
                 'ledger_id': ''},
                {'id': 'notes/new-rows.tsv:2', 'source': 'notes/new-rows.tsv', 'where': 'c', 'mark': 'repeat',
                 'ledger_id': 'records/A.md:99'}]
        ds = self.run_rules([r], labels=self.label(r['id'], 'C-stub'), outside=rows)
        self.assertEqual((ds['notes/marks.tsv:2']['exit'], ds['notes/marks.tsv:2']['points_at']),
                         (ledger.NOT_A_ROW, r['id']))
        for nid in ('notes/marks.tsv:3', 'notes/new-rows.tsv:2'):
            self.assertEqual((ds[nid]['exit'], ds[nid]['rule']), (ledger.NO_RULE, 'R7'))

    def test_an_exit_3_ruling_outside_confirmed_is_refused(self):
        tmp = tempfile.mkdtemp()
        try:
            os.makedirs(os.path.join(tmp, 'docs'))
            with open(os.path.join(tmp, 'docs', 'r.md'), 'w', encoding='utf-8') as fh:
                fh.write('# R\n\n## Unconfirmed\n\n- words\n\n## Confirmed\n\n- user words\n')
            r = self.rec('records/A.md', 5, 'Rewrite')
            lab = self.label(r['id'], 'C-stub')
            ledger.apply_exits(ledger.parse_rules(self.rules_text(
                quotes='- records/A.md:5\n  - quote: q\n  - ruling: docs/r.md:9\n  - sides: a | b')),
                [r], lab, {}, [], repo=tmp)
            with self.assertRaisesRegex(ledger.RulesError, 'Confirmed'):
                ledger.apply_exits(ledger.parse_rules(self.rules_text(
                    quotes='- records/A.md:5\n  - quote: q\n  - ruling: docs/r.md:5\n  - sides: a | b')),
                    [r], lab, {}, [], repo=tmp)
        finally:
            shutil.rmtree(tmp)

    def test_an_outside_row_no_rule_takes_is_no_rule(self):
        row = {'id': 'notes/marks.tsv:2', 'source': 'notes/marks.tsv', 'where': 'records/A.md:3', 'mark': 'not read'}
        d = self.run_rules([], outside=[row])['notes/marks.tsv:2']
        self.assertEqual((d['exit'], d['rule']), (ledger.NO_RULE, 'R7'))

    def test_a_rule_id_the_code_does_not_know_is_refused(self):
        with self.assertRaisesRegex(ledger.RulesError, 'R99'):
            ledger.parse_rules(self.rules_text(ledger_order=list(ledger.LEDGER_RULES) + ['R99']))

    def test_a_rule_the_code_knows_and_the_table_lacks_is_refused(self):
        order = [r for r in ledger.LEDGER_RULES if r != 'R5']
        with self.assertRaisesRegex(ledger.RulesError, 'R5'):
            ledger.parse_rules(self.rules_text(ledger_order=order))

    def test_a_rule_naming_a_mut_column_may_only_give_exit_2(self):
        ok = self.rules_text(takes={'R1a': 'a Rewrite record with `mut_caught_none` yes'})
        ledger.parse_rules(ok)
        bad = self.rules_text(takes={'R3': 'a clean Keep record whose `mut_caught` is above zero'})
        with self.assertRaisesRegex(ledger.RulesError, r'line \d+: rule R3 names a mut_ column'):
            ledger.parse_rules(bad)

    def test_a_mut_rule_giving_exit_4_writes_no_exits_file(self):
        tmp = tempfile.mkdtemp()
        try:
            os.makedirs(os.path.join(tmp, 'exits'))
            text = self.rules_text(takes={'R3': 'a clean Keep record whose `mut_caught` is above zero'})
            with open(os.path.join(tmp, 'exits', ledger.RULES_FILE), 'w', encoding='utf-8') as fh:
                fh.write('\n'.join(text))
            with self.assertRaises(SystemExit) as stop:
                ledger.exits_main(tmp)
            self.assertEqual(stop.exception.code, 1)
            self.assertFalse(os.path.exists(os.path.join(tmp, ledger.EXITS_FILE)))
        finally:
            shutil.rmtree(tmp)

    def test_an_exit_3_block_without_its_sides_is_refused(self):
        r = self.rec('records/A.md', 5, 'Rewrite')
        with self.assertRaisesRegex(ledger.RulesError, 'sides'):
            self.run_rules([r], labels=self.label(r['id'], 'C-stub'),
                           quotes='- records/A.md:5\n  - quote: the words\n  - ruling: docs/x.md:4')

    def test_an_exit_3_block_closes_a_question_row(self):
        r = self.rec('records/A.md', 5, 'Rewrite')
        d = self.run_rules([r], labels=self.label(r['id'], 'C-stub'),
                           quotes='- records/A.md:5\n  - quote: the words\n  - ruling: docs/x.md:4\n'
                                  '  - sides: one side | the other')[r['id']]
        self.assertEqual((d['exit'], d['groups']), ('3', []))


if __name__ == '__main__':
    unittest.main()
