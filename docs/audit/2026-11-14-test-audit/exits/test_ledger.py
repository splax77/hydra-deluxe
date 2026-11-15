"""Checks ledger.py against the audit's real record and count files.

Run: python -I test_ledger.py   (from any folder; it finds the audit folder itself)
Every expected value below is read from the file it names, not typed from memory.
"""
import os
import re
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.argv = sys.argv[:1]
import ledger  # noqa: E402

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
        tot = ledger.totals_section1()
        for cf in ('T5-KR1', 'T5-IR4', 'T5-IR10', 'T5-IR21'):
            rows = [r for r in self.counts if r['cf'] == cf]
            self.assertEqual(len(rows), tot[cf][0], cf)
            self.assertEqual(sum(1 for r in rows if r['faults']), tot[cf][1], cf)

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

    def test_answer_4_rows_keep_x9_and_wait_for_the_main_session(self):
        self.assertTrue(self.settle)
        for r in self.settle:
            self.assertEqual(r['cf'], 'T5-IR4')
            self.assertIn('X9', r['faults'])
            self.assertTrue(r['rec'].startswith('RD') or r['text'].startswith('RD'))

    def test_answer_5_rows_keep_x9(self):
        rows = [r for r in self.counts if r['answer'].startswith('answer 5')]
        self.assertTrue(any('test_fits_screen' in r['text'] for r in rows))
        self.assertTrue(any(r['text'].startswith('616 ') for r in rows))
        for r in rows:
            self.assertIn('X9', r['faults'])


if __name__ == '__main__':
    unittest.main()
