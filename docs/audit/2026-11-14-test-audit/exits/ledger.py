"""T8 E1 ledger: one row per record verdict and one row per recount row.

Usage: python -I ledger.py [AUD folder] [out.tsv]
Both default to this script's parent folder and exits/ledger.tsv in it.

It reads only the audit folder. It writes two kinds of row into one file:

  kind=record  one row per record in records/*.md, found by its "Verdict:" line.
  kind=count   one row per table row in counts/T5-*.md, with the fault marks
               after TOTALS.md section 3 and the user's answers 2 to 5 in the
               T8 plan (docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md).

The script does not join the two kinds: the count files name records in more
than ten different ways, and a join it cannot check would hide misses. It does
not read verify/ or capture/byreading/ either; those columns are left for the
scouts. It is not trusted to see everything: it prints each count file's row
count beside TOTALS.md section 1, and lists every heading that looks like a
record but has no verdict line it could read.

Taken from repeats/repeats.py: nothing but the shape (plain regex over the
files, a TSV with a header line, a short printout). repeats.py reads test
source, not record files, so there was no record reader to reuse.
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
AUD = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(HERE)
OUT = sys.argv[2] if len(sys.argv) > 2 else os.path.join(AUD, 'exits', 'ledger.tsv')

VERDICT = re.compile(r'(?:\*\*)?Verdict:(?:\*\*)?\s*(?:\*\*)?\s*(Keep|Investigate|Rewrite|Remove)\b')
VERDICT_BARE = re.compile(r'\b(Keep|Investigate|Rewrite|Remove) \([A-Z]{1,4}\)\.')
HEADING = re.compile(r'^(#{1,6})\s+(.*)$')
# A heading that names one test or one numbered item: "### 12. ...", "### test_x",
# "## R3 ...". Used only to list headings with no verdict line.
RECORDISH = re.compile(r'^(\d+(\.\d+)?[.:)]|[A-Z]{1,3}\d+\b|test_\w+|`)')
FAULT = re.compile(r'X9|X10|X11|X12|snapshot', re.I)
SUPERSEDED = {'window-WN4.md'}  # by WN4a, WN4b, WN4c (AUD/log.md line 358)

# T6 reds, from mechanical/corpus.md (commit 6aa6d684): record file and record number.
T6 = {('A4.md', '3'), ('A4.md', '45'), ('A13.md', '10'), ('delta-DL5.md', '18')}
# T7 slices whose planted bug stayed green or did not run (mechanical/planted.md, commit a4ad3306).
T7_REOPENED = {'A3.md', 'A9.md', 'A30.md', 'C7.md', 'window-WN3.md', 'window-WN4a.md',
               'window-WN4b.md', 'window-WN8.md'}


def lines_of(path):
    with open(path, encoding='utf-8', errors='replace') as fh:
        return fh.read().splitlines()


def clean(s):
    return re.sub(r'\s+', ' ', s.replace('\t', ' ')).strip()


# A record's title line, when it is not a Markdown heading: "**BP-2 a session ...**"
# (records/D15.md), "**2. test_summary_is_the_first_line** (155)" (records/D13.md).
BOLD_START = re.compile(r'^\s*(?:- )?\*\*(?!Verdict)([^*]{6,}?)\*\*(.*)$')
# A bold lead that is a question label inside a record ("**Read:**", "**Q4**"), not a title.
BOLD_LABEL = re.compile(r'^(Q?\d+[a-z]?|[A-Za-z /]{1,20}):?$')


# A numbered or bulleted list line that names a test: "1. `test_x` (`:179`) ... Verdict:
# Keep" (records/D15.md line 296).
LIST_START = re.compile(r'^\s*(?:- )?(?:\d+\.\s+)?(?:\*\*)?`?test_\w+|^\s*(?:- )?\d+\.\s+`')
READ_LINE = re.compile(r'^\s*(?:Read:|1 Claim:)')


def record_start(lines, k, in_fence, fence_first, open_record):
    """The title text if lines[k] starts a record, else None. Five shapes start one:
    a Markdown heading; a bold lead line; a list line that names a test; the first line
    inside a code fence (records/A22.md, B6.md, T3.md); and a paragraph whose next line
    is "Read:" or "1 Claim:" (records/A39.md line 29, C3.md line 667). The last three, and a bold lead,
    may carry the record's own verdict on the same line (records/E3.md line 864).

    Any other line with text before "Verdict:" is not a record: that text is the
    record's own question lines (records/E1.md line 30, E4.md line 15, D15.md line 90),
    so the verdict belongs to the record above it. For the same reason, while a record
    title is open (it has no verdict yet) only a heading starts a new one:
    a record's "Read:" line, its question lines and the tests it lists under question
    10 are part of it (records/A10.md line 26 is the "Read:" line of the record at 24)."""
    line = lines[k]
    if in_fence:
        # A heading record may hold its body in a fence (records/A37.md lines 20 to 36).
        return clean(line) if fence_first and line.strip() and not open_record else None
    m = HEADING.match(line)
    if m:
        return clean(m.group(2))
    if open_record:
        return None
    m = BOLD_START.match(line)
    if m and not BOLD_LABEL.match(m.group(1).strip()):
        return clean(m.group(1) + m.group(2))
    if LIST_START.match(line):
        return clean(line).lstrip('- ')
    prev = lines[k - 1].strip() if k else ''
    nxt = lines[k + 1] if k + 1 < len(lines) else ''
    if line.strip() and prev in ('', '---') and READ_LINE.match(nxt) and not VERDICT.search(line):
        return clean(line)
    return None


def read_records():
    """One row per record start that carries a verdict. Every verdict line belongs to
    the record start above it; a second verdict line under one start is a re-judge and
    is kept on the same row (its verdicts list, earliest first)."""
    rows, unread = [], []
    rec_dir = os.path.join(AUD, 'records')
    for name in sorted(os.listdir(rec_dir)):
        if not name.endswith('.md') or name in SUPERSEDED:
            continue
        lines = lines_of(os.path.join(rec_dir, name))
        heading, hline, cur = '', 0, None
        in_fence, fence_first = False, False

        def close(c):
            if c and not c['verdicts'] and RECORDISH.match(c['record']):
                unread.append((name, c['hline'], c['record']))
            elif c and c['verdicts']:
                rows.append(c)

        for i, line in enumerate(lines, 1):
            if line.strip().startswith('```'):
                in_fence, fence_first = not in_fence, not in_fence
                continue
            # A level 1 heading is the file's title ("# A39: tests/ui/..."), so it never
            # holds a record open. Records do sit under level 2 (records/A15.md line 30).
            open_record = bool(cur and not cur['verdicts'] and RECORDISH.match(cur['record'])
                               and cur.get('level', 9) > 1)
            title = record_start(lines, i - 1, in_fence, fence_first, open_record)
            if in_fence and line.strip():
                fence_first = False
            if title is not None:
                close(cur)
                kind = 'fence' if in_fence else ('heading' if HEADING.match(line) else 'other')
                if kind == 'heading':
                    heading, hline = title, i
                vat = VERDICT.search(line)
                cur = {'file': name, 'hline': i, 'heading': heading,
                       'record': clean(re.split(r'(?:\*\*)?Verdict:', title)[0])[:160],
                       'kind': kind, 'verdicts': [], 'vlines': [],
                       'level': len(HEADING.match(line).group(1)) if kind == 'heading' else 9}
                if not vat:
                    continue
            if cur is None:
                continue
            vs = VERDICT.findall(line)
            if not vs and not cur['verdicts'] and RECORDISH.match(cur['record']):
                # records/E1.md writes "... Investigate (RC)." with no "Verdict:".
                b = VERDICT_BARE.findall(line)
                if b:
                    cur['verdicts'].append(b[-1])
                    cur['vlines'].append(i)
                    cur['bare'] = True
                continue
            if vs:
                cur['verdicts'] += vs
                cur['vlines'] += [i] * len(vs)
        close(cur)
    for r in rows:
        r['line'] = r['vlines'][0]
    return rows, unread


def table_rows(path):
    """Yield (section, cells dict, raw line) for every data row of every table that
    has a faults column. IR8 writes rows as '- a | b | c' under a Format: line."""
    lines = lines_of(path)
    section, header, bullet_hdr = '', None, None
    for i, line in enumerate(lines, 1):
        m = HEADING.match(line)
        if m:
            section, header = clean(m.group(2)), None
            if not section.startswith('Table'):
                # IR8 gives its Format: line once, under the first "## Table:".
                bullet_hdr = None
            continue
        s = line.strip()
        if s.lower().startswith('format:'):
            bullet_hdr = [clean(c).lower() for c in s.split(':', 1)[1].split('|')]
            continue
        if s.startswith('|'):
            cells = [clean(c) for c in s.strip('|').split('|')]
            if all(re.fullmatch(r':?-+:?', c) for c in cells if c):
                continue
            nxt = lines[i].strip() if i < len(lines) else ''
            if re.match(r'^\|[ :|-]+$', nxt):
                header = [c.lower() for c in cells]
                continue
            if header is not None:
                yield section, header, cells, i, s
        elif bullet_hdr and s.startswith('- ') and '|' in s:
            yield section, bullet_hdr, [clean(c) for c in s[2:].split('|')], i, s
        elif not s:
            header = None


def col(header, cells, *names):
    for n in names:
        for k, h in enumerate(header):
            if n in h and k < len(cells):
                return cells[k]
    return ''


def marks(text):
    out = []
    for m in FAULT.findall(text or ''):
        m = 'snapshot' if m.lower() == 'snapshot' else m.upper()
        if m not in out:
            out.append(m)
    return out


def read_counts():
    rows, notable = [], []
    cdir = os.path.join(AUD, 'counts')
    for name in sorted(os.listdir(cdir)):
        if not name.startswith('T5-') or not name.endswith('.md'):
            continue
        cf = name[:-3]
        for section, header, cells, i, raw in table_rows(os.path.join(cdir, name)):
            if not any('fault' in h for h in header):
                notable.append((cf, i, 'table with no faults column skipped: ' + raw[:80]))
                continue
            faults = col(header, cells, 'faults', 'fault')
            rec = col(header, cells, 'record file', 'rec', 'record', '#', 'file')
            num = col(header, cells, 'no.', '#') if 'record file' in ' '.join(header) else ''
            if 'record file' in header and 'record' in header:
                num = cells[header.index('record')]
            test = col(header, cells, 'test')
            reason = col(header, cells, 'reason')
            if not test and rec and re.search(r'\w\.\w+:\d+|:\d+', rec):
                test = rec
            rows.append({'cf': cf, 'line': i, 'section': section, 'rec': rec, 'num': num,
                         'test': test, 'faults_raw': faults, 'faults': marks(faults),
                         'reason': reason, 'text': ' '.join(cells), 'fix': '', 'answer': ''})
    return rows, notable


CORR = re.compile(r'^- (T5-\w+), record (.*): (.+?) to (.+)$')


def read_corrections():
    out, part = [], False
    for line in lines_of(os.path.join(AUD, 'counts', 'TOTALS.md')):
        if line.startswith('## 3.'):
            part = True
            continue
        if part and line.startswith('## '):
            break
        m = CORR.match(line) if part else None
        if m:
            cf, rid, old, new = m.groups()
            out.append({'cf': cf, 'id': rid, 'old': old, 'new': marks(new), 'line': line})
    return out


def matches(row, rid):
    """A count row matches a correction id when every file name, record-file id and
    line number in the id appears in the row (its cells or its section)."""
    hay = row['text'] + ' ' + row['section']
    files = re.findall(r'[\w-]+\.(?:py|cpp|h|md)', rid)
    ids = re.findall(r'\b(?:[A-Z]{1,2}\d+(?:-C\d+)?|D\d+)\b', rid)
    nums = re.findall(r'(?<![\w.-])(\d+)(?![\w])', re.sub(r'[\w-]+\.(?:py|cpp|h|md)', '', rid))
    names = re.findall(r'\b(test_\w+|[a-z]+_[a-z_]+)\b', rid)
    for f in files:
        if f not in hay and f.split('.')[0] not in hay:
            return False
    for d in ids:
        if not re.search(r'\b' + re.escape(d) + r'\b', hay):
            return False
    for n in nums:
        if not re.search(r'(?<![\d])' + n + r'(?![\d])', hay):
            return False
    for n in names:
        if n not in hay:
            return False
    return bool(files or ids or nums or names)


ANSWER_BACK = {'T5-KR3': 'answer 2', 'T5-IR1': 'answer 3'}


def apply_corrections(rows, corrs):
    unapplied, used = [], set()
    by_cf = {}
    for r in rows:
        by_cf.setdefault(r['cf'], []).append(r)
    pending = []
    for c in corrs:
        cands = [r for r in by_cf.get(c['cf'], []) if matches(r, c['id']) and id(r) not in used]
        if len(cands) == 1:
            pending.append((c, cands[0]))
            used.add(id(cands[0]))
        else:
            unapplied.append((c, len(cands)))
    # The nine IR17 "D3 (test)" lines name no test. TOTALS gives their reason:
    # "hand-built ini and song.ini text". Apply them only if exactly nine D3 X9 rows say ini.
    left = []
    d3 = [c for c, n in unapplied if c['cf'] == 'T5-IR17' and c['id'] == 'D3 (test)']
    if d3:
        cands = [r for r in by_cf.get('T5-IR17', []) if r['rec'] == 'D3' and 'X9' in r['faults']
                 and re.search(r'\bini\b|song\.ini', r['reason'])]
        if len(cands) == len(d3):
            pending += list(zip(d3, cands))
        else:
            left += [(c, len(cands)) for c in d3]
    left += [(c, n) for c, n in unapplied if not (c['cf'] == 'T5-IR17' and c['id'] == 'D3 (test)')]
    for c, r in pending:
        ans = ANSWER_BACK.get(c['cf'])
        if ans:
            # Answers 2 and 3: the removal is undone, X9 stays.
            r['answer'] = ans
            r['fix'] = 'TOTALS s3 removal undone by ' + ans
            if 'X9' not in r['faults']:
                r['faults'].insert(0, 'X9')
        else:
            r['fix'] = 'TOTALS s3: ' + c['old'] + ' to ' + (', '.join(c['new']) or 'none')
            r['faults'] = c['new']
    return left


def apply_answers(rows):
    settle = []
    for r in rows:
        if r['cf'] == 'T5-IR4' and re.match(r'RD\b', r['rec'] or r['text']) and 'X9' in r['faults']:
            # Answer 4: X9 goes only where the payload is the one thing marked. The
            # reason text cannot show that, so the main session settles each row.
            r['answer'] = 'answer 4, main session to settle'
            settle.append(r)
        if r['cf'] == 'T5-IR8':
            t = r['text'] + ' ' + r['section']
            n = re.match(r'(\d+)', r['rec'] or r['text'])
            if ('D4' in r['section'] and n and 616 <= int(n.group(1)) <= 674) or 'test_fits_screen' in t:
                r['answer'] = 'answer 5 (keeps X9)'
    return settle


def totals_section1():
    out = {}
    for line in lines_of(os.path.join(AUD, 'counts', 'TOTALS.md')):
        m = re.match(r'^\| (T5-\w+) \| (\d+) \| (\d+)', line)
        if m:
            out[m.group(1)] = (int(m.group(2)), int(m.group(3)))
    return out


def main():
    recs, unread = read_records()
    counts, notable = read_counts()
    left = apply_corrections(counts, read_corrections())
    settle = apply_answers(counts)
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    cols = ['kind', 'file', 'line', 'heading_or_section', 'record', 'test', 'verdict', 'earlier_verdicts',
            'faults_counter', 'faults', 'correction', 'answer', 't6', 't7_reopened', 'reason']
    with open(OUT, 'w', encoding='utf-8', newline='\n') as fh:
        fh.write('\t'.join(cols) + '\n')
        for r in recs:
            v = r['verdicts']
            t6 = 't6' if (r['file'], (re.match(r'(\d+)', r['record']) or [None, ''])[1]) in T6 else ''
            fh.write('\t'.join(['record', 'records/' + r['file'], str(r['line']), r['heading'], r['record'],
                                '', v[-1], ','.join(v[:-1]), '', '', '', '', t6,
                                'yes' if r['file'] in T7_REOPENED else '', '']) + '\n')
        for r in counts:
            fh.write('\t'.join(['count', 'counts/' + r['cf'] + '.md', str(r['line']), r['section'],
                                (r['rec'] + ' ' + r['num']).strip(), r['test'], '', '',
                                r['faults_raw'], ','.join(r['faults']) or 'none', r['fix'], r['answer'],
                                '', '', r['reason'][:200]]) + '\n')

    from collections import Counter
    vc = Counter(r['verdicts'][-1] for r in recs)
    print('record rows:', len(recs), dict(vc), '| re-judged (2+ verdicts):',
          sum(1 for r in recs if len(r['verdicts']) > 1))
    print('\nper record file: rows by last verdict (Keep/Investigate/Rewrite/Remove), bare-form rows')
    for f in sorted({r['file'] for r in recs}):
        c = Counter(r['verdicts'][-1] for r in recs if r['file'] == f)
        bare = sum(1 for r in recs if r['file'] == f and r.get('bare'))
        multi = sum(1 for r in recs if r['file'] == f and len(r['verdicts']) > 1)
        print(f'  {f:22} {c["Keep"]:4} {c["Investigate"]:4} {c["Rewrite"]:4} {c["Remove"]:4}'
              + (f'  bare {bare}' if bare else '') + (f'  2+ verdicts {multi}' if multi else ''))
    print('count rows:', len(counts))
    print('\nper count file: table rows / faulty rows (after fixes)  vs  TOTALS s1 Read / Faulty')
    tot = totals_section1()
    for cf in sorted(tot):
        rs = [r for r in counts if r['cf'] == cf]
        faulty = sum(1 for r in rs if r['faults'])
        rd, fy = tot[cf]
        flag = '' if (len(rs), faulty) == (rd, fy) else '   MISMATCH'
        print(f'  {cf:8} {len(rs):4} / {faulty:4}   vs  {rd:4} / {fy:4}{flag}')
    print('\ncorrections not applied (count of candidate rows):')
    for c, n in left:
        print(f'  {n} candidates: {c["line"]}')
    print('\nheadings that look like records but have no verdict line the script read:', len(unread))
    for f, ln, h in unread:
        print(f'  records/{f}:{ln} {h[:100]}')
    print('\ntables skipped:', len(notable))
    for cf, ln, why in notable:
        print(f'  {cf}:{ln} {why}')
    print('\nrows left for answer 4, main session to settle:', len(settle))
    for r in settle:
        print(f'  counts/{r["cf"]}.md:{r["line"]} {r["rec"]} [{",".join(r["faults"])}] {r["reason"][:90]}')
    print('\nwrote', OUT)


if __name__ == '__main__':
    main()
