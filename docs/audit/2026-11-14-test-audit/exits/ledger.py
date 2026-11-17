"""T8 E1 ledger: one row per record verdict and one row per recount row.

Usage: python -I ledger.py [AUD folder] [out.tsv]
Both default to this script's parent folder and exits/ledger.tsv in it.

It reads only the audit folder. It writes two kinds of row into one file:

  kind=record  one row per record in records/*.md, found by its "Verdict:" line.
  kind=count   one row per table row in counts/T5-*.md, with the fault marks
               after TOTALS.md section 3 and the user's answers 2 to 5 in the
               T8 plan (docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md).

Answer 4's rows are settled from the main session's exits/answer4.tsv
(settle_answer4); the script stops with an error when that file and the ledger
disagree. The held column marks a row whose answer is still to settle.

It also writes exits/notes.tsv (plan step E5): every side-note, split and
production-code block in the audit's notes folders (read_notes), unjudged.

Each count row gets the id of the one record row it belongs to (join_counts),
or an empty join and a reason. The by-reading and verifier verdicts reach
record rows through the same join (second_verdicts); the verifier reader knows
only one line shape, so most verify/ files are not read yet. The script is
not trusted to see everything: it prints each count file's row
count beside TOTALS.md section 1 and its joined and unjoined rows, and writes
every record title with no verdict it could read and every unjoined count row
to exits/not-in-ledger.md.

Each record row also carries the mutation run's evidence for its test, in the
mut_ columns (plan step E1c, task M3). The run's files in mutation/ are read
only through mutation_evidence.read_evidence(). tests.txt (report.py's) owns
each test's caught count; tools/mutate/first_use.py owns placing a record that
gives a line and no test name. This script writes those records to
mutation/line-records.txt and reads first_use.py's table back from
mutation/line-records.md when it exists. No mut_ column picks an exit. Every
record row that is not matched to one test, and every run test no record
matched, is written to mutation/mutation-unmatched.md. The script prints each
record row whose mut_ columns differ from the ledger it replaces.

Taken from repeats/repeats.py: nothing but the shape (plain regex over the
files, a TSV with a header line, a short printout). repeats.py reads test
source, not record files, so there was no record reader to reuse.
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import mutation_evidence  # noqa: E402  M1's reader; the run's files are read through it only

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
    if line.strip() and prev in ('', '---') and READ_LINE.match(nxt) and not VERDICT.search(line) \
            and not READ_LINE.match(line) and not re.match(r'^\s*(?:\d+|Q\d+)[ :]', line):
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
        heading, hline, cur, stack = '', 0, None, {}
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
                level = len(HEADING.match(line).group(1)) if kind == 'heading' else 9
                # The headings above this record, innermost first; the mutation join
                # reads a test file from them when the title names none.
                above = [(k, stack[k]) for k in sorted(stack, reverse=True) if k < level]
                if kind == 'heading':
                    heading, hline = title, i
                    stack = {k: v for k, v in stack.items() if k < level}
                    stack[level] = title
                vat = VERDICT.search(line)
                full = clean(re.split(r'(?:\*\*)?Verdict:', title)[0])
                cur = {'file': name, 'hline': i, 'heading': heading,
                       'record': full[:160], 'title': full, 'above': above,
                       'kind': kind, 'verdicts': [], 'vlines': [], 'level': level}
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
                         'reason': reason, 'text': ' '.join(cells), 'fix': '', 'answer': '',
                         'header': header, 'cells': cells})
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


TO_SETTLE = 'to settle'
ANSWER4_FILE = 'answer4.tsv'
ANSWER4_DONE = 'answer 4 (X9 removed: payload is input)'


class LedgerError(Exception):
    """A settle file that does not fit the ledger. main() prints it and exits 1."""


def read_answer4(path):
    """The main session's answer-4 rows, as dicts keyed by answer4.tsv's header."""
    lines = [ln for ln in lines_of(path) if ln.strip()]
    if not lines:
        raise LedgerError(f'{path} is empty')
    head = lines[0].split('\t')
    want = ['file', 'line', 'record', 'faults_before', 'faults_after', 'why']
    if head != want:
        raise LedgerError(f'{path}: header is {head}, expected {want}')
    out = []
    for n, ln in enumerate(lines[1:], 2):
        cells = ln.split('\t')
        if len(cells) != len(want) or not cells[1].isdigit():
            raise LedgerError(f'{path} line {n} is not a settle row: {ln[:100]}')
        out.append(dict(zip(want, cells), at=n))
    return out


def faults_text(faults):
    return ','.join(faults) or 'none'


def settle_answer4(rows, path=None):
    """Settle the rows apply_answers() left for answer 4 from the main session's file
    (exits/answer4.tsv). Each settle row must match one count row by file and line,
    and that row's faults must be its faults_before. Any row still to settle after the
    file is read is an error too. Returns [(row, faults before)] for the rows changed."""
    path = path or os.path.join(AUD, 'exits', ANSWER4_FILE)
    by_key = {}
    for r in rows:
        by_key.setdefault(('counts/' + r['cf'] + '.md', r['line']), []).append(r)
    changed, seen = [], set()
    for s in read_answer4(path):
        key = (s['file'], int(s['line']))
        where = f'{path} line {s["at"]} ({s["file"]}:{s["line"]} {s["record"]})'
        if key in seen:
            raise LedgerError(f'{where} settles a row an earlier line already settled')
        seen.add(key)
        hit = by_key.get(key, [])
        if len(hit) != 1:
            raise LedgerError(f'{where} matches {len(hit)} ledger rows, not one')
        r = hit[0]
        if faults_text(r['faults']) != s['faults_before']:
            raise LedgerError(f'{where}: its faults_before is {s["faults_before"]}, '
                              f'but the ledger row\'s faults are {faults_text(r["faults"])}')
        if TO_SETTLE not in r['answer']:
            raise LedgerError(f'{where} names a row that is not waiting for answer 4 '
                              f'(its answer is "{r["answer"]}")')
        before = list(r['faults'])
        r['faults'] = [] if s['faults_after'] == 'none' else s['faults_after'].split(',')
        r['answer'] = ANSWER4_DONE
        changed.append((r, before))
    left = [r for r in rows if TO_SETTLE in r['answer']]
    if left:
        raise LedgerError(f'{len(left)} rows wait for answer 4 and {path} has no line for them: '
                          + '; '.join(f'counts/{r["cf"]}.md:{r["line"]} {r["rec"]}' for r in left))
    return changed


def held(r):
    """'yes' while a row's answer is still for the main session to settle."""
    return 'yes' if TO_SETTLE in r['answer'] else ''


FILE_TOKEN = re.compile(r'\b((?:delta-|window-)?[A-Z]{1,2}\d+[a-z]?(?:-C\d+(?:-fuzz)?)?)(?:\.md)?\b')
SOURCE_FILE = re.compile(r'[\w./-]+\.(?:py|cpp|h|md|ps1|txt)')


def record_files(tokens_text):
    """Record files named in a text, by their id ("D10", "D8.md", "DL3", "WN4a")."""
    rec_dir = os.path.join(AUD, 'records')
    out = []
    for t in FILE_TOKEN.findall(tokens_text):
        for cand in (t, 'delta-' + t, 'window-' + t):
            if os.path.isfile(os.path.join(rec_dir, cand + '.md')) and cand + '.md' not in SUPERSEDED:
                if cand + '.md' not in out:
                    out.append(cand + '.md')
                break
    return out


def count_titles():
    """Each count file's first heading, which names the record files it counts."""
    out = {}
    cdir = os.path.join(AUD, 'counts')
    for name in os.listdir(cdir):
        if name.startswith('T5-') and name.endswith('.md'):
            for line in lines_of(os.path.join(cdir, name)):
                m = HEADING.match(line)
                if m:
                    out[name[:-3]] = clean(m.group(2))
                    break
    return out


def locators(r):
    """What a count row says about its record, each read from a named cell:
    test line, test name, record-file line, and record number (with how sure)."""
    h, cells = r['header'], r['cells']
    rec_i = next((k for k, x in enumerate(h) if x.startswith('rec') and 'file' not in x), None)
    rec = cells[rec_i] if rec_i is not None and rec_i < len(cells) else r['rec']
    rec_h = h[rec_i] if rec_i is not None else ''
    test = r['test'] or ''
    loc = {'T': None, 'name': None, 'L': None, 'N': None, 'N_sure': False}
    m = re.search(r'\.md:(\d+)', rec)
    if m or 'line' in rec_h and 'test' not in rec_h and ('.md' in rec_h or rec_h.startswith('d')):
        # "D9.md:24" (T5-IR5) or the "record (d2.md line)" column (T5-IR3): a record-file line.
        mm = m or re.match(r'^(\d+)\b', rec)
        loc['L'] = int(mm.group(1)) if mm else None
    elif re.match(r'^\d+\b', rec) and ('test line' in rec_h or 'tg line' in rec_h or 'file:line' in rec_h):
        loc['T'] = int(re.match(r'^(\d+)', rec).group(1))       # T5-IR8, T5-IR2
    t = re.search(r':\s*(\d+)', test) or re.search(r':\s*(\d+)', rec)
    if t and loc['T'] is None:
        loc['T'] = int(t.group(1))
    words = SOURCE_FILE.sub('', test + ' ' + rec)
    n = re.search(r'\btest_\w+', words) or re.search(r'\b[a-z][a-z0-9]*(?:_[a-z0-9]+){2,}\b', words)
    if n:
        loc['name'] = n.group(0)
    m = re.match(r'^(?:delta-|window-)?[A-Z]{1,2}\d+[a-z]?\s*[- ]\s*(\d+(?:\.\d+)?)\b', rec)
    if m:
        loc['N'], loc['N_sure'] = m.group(1), True                 # "A41 1", "C9 1.2", "C5-4"
    else:
        for k, x in enumerate(h):
            if x in ('no.', 'no', '#') or (x == 'record' and 'record file' in h):
                if k < len(cells) and re.fullmatch(r'\d+(?:\.\d+)?', cells[k]):
                    loc['N'], loc['N_sure'] = cells[k], x != '#'
    return loc


def name_hits(name, title):
    if name in title or name.replace('test_', '', 1) in title:
        return True
    toks = [w for w in name.replace('test_', '', 1).split('_') if w]
    if len(toks) < 3:
        return False
    rest = re.findall(r'[a-z0-9]+', title.lower())
    it = iter(rest)
    return all(any(w == x for x in it) for w in toks)     # words in order, others between


def join_counts(counts, recs):
    """Give each count row the id of the one record row it belongs to, or none.
    A join holds when one locator finds exactly one record row and no other locator
    finds a different one. Every row left out says why."""
    titles = count_titles()
    by_file = {}
    for x in recs:
        by_file.setdefault(x['file'], []).append(x)
    for r in counts:
        own = ' '.join(c for h, c in zip(r['header'], r['cells'])
                       if ('rec' in h or 'file' in h) and 'test' not in h)
        files = record_files(own) or record_files(r['section']) \
            or record_files(titles.get(r['cf'], ''))
        r['join'], r['why'] = '', ''
        if not files:
            r['why'] = 'names no record file'
            continue
        pool = [x for f in files for x in by_file.get(f, [])]
        loc = locators(r)
        sets = {}
        if loc['T'] is not None:
            pat = re.compile(r'(?::|\(|line |lines )\s*' + str(loc['T']) + r'(?!\d)')
            sets['test line'] = [x for x in pool if pat.search(x['record'])]
        if loc['name']:
            sets['test name'] = [x for x in pool if name_hits(loc['name'], x['record'])]
        if loc['L'] is not None:
            sets['record line'] = [x for x in pool if x['hline'] == loc['L'] or loc['L'] in x['vlines']]
        if loc['N'] is not None and len(files) == 1:
            pat = re.compile(r'^(?:Record\s+)?[A-Z]{0,3}' + re.escape(loc['N']) + r'[.: )]')
            sets['number'] = [x for x in pool if pat.match(x['record'])]
        trusted = [k for k in sets if k != 'number' or loc['N_sure']]
        uniq = {id(s[0]): s[0] for k, s in sets.items() if k in trusted and len(s) == 1}
        if len(uniq) != 1:
            r['why'] = ('locators disagree' if len(uniq) > 1 else
                        'no locator finds one record (' + ', '.join(f'{k} {len(s)}' for k, s in sets.items())
                        + ')' if sets else 'no locator in the row')
            continue
        hit = next(iter(uniq.values()))
        by = [k for k, s in sets.items() if k in trusted and s == [hit]]
        if by == ['number'] and 'test line' in sets and not sets['test line']:
            # The row's test line is in no record title, so the number alone cannot be checked.
            r['why'] = 'only the record number finds it; its test line is in no record title'
            continue
        clash = [k for k, s in sets.items() if s and hit not in s]
        if clash:
            r['why'] = 'locators disagree (' + ', '.join(clash) + ')'
            continue
        r['join'] = f"records/{hit['file']}:{hit['hline']}"
    # Two rows of one count file on one record: neither is trusted.
    seen = {}
    for r in counts:
        if r['join']:
            seen.setdefault((r['cf'], r['join']), []).append(r)
    for rs in seen.values():
        if len(rs) > 1:
            for r in rs:
                r['why'] = 'shares its record with ' + ', '.join(str(o['line']) for o in rs if o is not r)
                r['join'] = ''


BYREADING = re.compile(r'SHOWN BY RUN|HOLDS BY READING|CONTRADICTED|CANNOT TELL')
VERIFY_LINE = re.compile(r'^\s*(?:\*\*)?(\d+(?:\.\d+)?)\.\s+(.*?)\((?:line |:)?(\d+)\)')


def second_verdicts(recs):
    """The by-reading verdict (capture/byreading/*-verdicts.md tables) and the
    verifier's verdict (verify/*.md lines "17. name (683). ... Verdict: Rewrite.")
    for each record row, through the same checked join as the count rows. Rows that
    do not join are returned, not guessed."""
    rows = []
    bdir = os.path.join(AUD, 'capture', 'byreading')
    for name in sorted(os.listdir(bdir)):
        if name.endswith('-verdicts.md'):
            for section, header, cells, i, raw in table_rows(os.path.join(bdir, name)):
                v = BYREADING.search(' '.join(cells))
                if v and 'record' in header:
                    rows.append({'cf': 'byreading/' + name[:-3], 'line': i, 'section': '', 'kind2': 'byreading',
                                 'rec': col(header, cells, 'record'), 'test': col(header, cells, 'test'),
                                 'header': header, 'cells': cells, 'text': ' '.join(cells), 'v': v.group(0)})
    vdir = os.path.join(AUD, 'verify')
    for name in sorted(os.listdir(vdir)):
        if not name.endswith('.md'):
            continue
        # verify/A10.md, A26-resend.md, D10-resend.md and E1a.md check records/A10.md, A26.md, D10.md, E1.md.
        fid = re.sub(r'(-resend|-reread|-q3)$', '', name[:-3])
        fid = re.sub(r'^(E\d)[ab]$', r'\1', fid)
        if '-' in fid and not fid.startswith(('delta-', 'window-', 'C14-C15')):
            continue                                   # A3-A4-reread.md checks two files
        for i, line in enumerate(lines_of(os.path.join(vdir, name)), 1):
            m, vs = VERIFY_LINE.match(line), VERDICT.findall(line)
            if m and vs:
                rows.append({'cf': 'verify/' + name[:-3], 'line': i, 'section': '', 'kind2': 'verify',
                             'rec': f'{fid} {m.group(1)}', 'test': ':' + m.group(3),
                             'header': ['record', 'test'], 'cells': [f'{fid} {m.group(1)}', ':' + m.group(3)],
                             'text': clean(line)[:120], 'v': vs[-1]})
    join_counts(rows, recs)
    idx = {f"records/{r['file']}:{r['hline']}": r for r in recs}
    for r in rows:
        if r['join']:
            x = idx[r['join']]
            key = 'byreading' if r['kind2'] == 'byreading' else 'verifier'
            if key == 'verifier' and r['v'] == x['verdicts'][-1]:
                continue                               # the column holds only a verdict that differs
            x.setdefault(key, [])
            if r['v'] not in x[key]:
                x[key].append(r['v'])
    return rows


# The mutation run's evidence on each record row (plan step E1c, task M3).
MUT_COLS = ['mut_match', 'mut_test', 'mut_caught', 'mut_reaches', 'mut_caught_none', 'mut_caught_shared',
            'mut_running_at_limit', 'mut_red_in_timed_out', 'mut_crashed_running', 'mut_run', 'mut_limits']
# The columns that hold one test's evidence; empty unless the row is matched to one test.
MUT_EVIDENCE = MUT_COLS[1:9]
MATCHED = ('name', 'line', 'line, file changed since')
LINE_RECORDS_TXT = 'line-records.txt'
LINE_RECORDS_MD = 'line-records.md'
# Optional: test files that changed between the audit's reading and the run's build,
# one path per line, written by the main session at task M4 (plan P29).
CHANGED_SINCE = 'changed-since.txt'
UNMATCHED = 'mutation-unmatched.md'
OUTSIDE = 'outside every test case'      # first_use.py's words for a line in no test

FILE_WORD = re.compile(r'[\w./\\-]+\.(?:cpp|hpp|h|py|ps1)\b')
# first_use.py's table row: slice | record line | file:line | test | reaches | caught | exact.
PLACED_ROW = re.compile(r'^\| (\S+) \| (\d+) \| (\S+):(\d+) \| (.*) \|\s*\d*\s*\|[^|]*\|[^|]*\|$')


def run_index(ev):
    """The run's tests as tests.txt writes them, each split into file, line and name."""
    tests = {}
    for key, t in ev['tests'].items():
        loc, name = key.split(' ', 1)
        f, ln = loc.rsplit(':', 1)
        tests[key] = dict(t, key=key, file=f.replace('\\', '/'), line=int(ln), name=name)
    return tests


def per_test_counts(ev, tests):
    """Per test: catches through shared state, timed-out bugs it was running in or red in,
    and crashed bugs it was running in. Each is a count of the entries the run's files
    list for that test, nothing more."""
    from collections import Counter
    out = {
        'shared': Counter(c['test'] for c in ev['summary']['caught_through_shared_state']),
        'crashed': Counter(c['test'] for c in ev['summary']['bug_runs_crashed']),
        'running': Counter(b['running_at_limit'] for b in ev['timed_out'] if b['running_at_limit']),
        'red': Counter(x for b in ev['timed_out'] for x in set(b['red_in_file'])),
    }
    for what, c in out.items():
        stray = [k for k in c if k not in tests]
        if stray:
            raise mutation_evidence.EvidenceError(
                f'ledger: the run\'s {what} list names tests tests.txt does not hold: {stray[:5]}')
    return out


def test_files(text, by_base):
    """Test files a text names, as {base name: the run's path, or None when the run did not
    hold it}. A file counts when the run held it, or when its name or folder marks it as
    a test; a source file a title mentions in passing does not."""
    out = {}
    for w in FILE_WORD.findall(text):
        w = w.replace('\\', '/').strip('`./')
        base = os.path.basename(w)
        if base in by_base:
            out.setdefault(base, by_base[base])
        elif base.startswith(('test_', 'uitest_', 'pbtest_')) or '/tests/' in '/' + w or base == 'conftest.py':
            out.setdefault(base, None)
    return out


def title_lines(title, base):
    """The test-file lines a title gives: 'file:N', '(:N)' and '(N)', with both ends of
    'N-M' or 'N to M'. Only when it gives none of those, the first 'line N' or 'lines N-M':
    a "Read:" title names the test's lines first and other files' lines after them
    (records/C1.md line 104)."""
    rng = r'(\d+)(?:\s*(?:-|–|to)\s*(\d+))?'
    pats = [re.escape(base) + r'`?:' + rng, r'(?<![\w.])\(`?:' + rng, r'\((\d+)\)']
    found = set()
    for p in pats:
        for m in re.finditer(p, title):
            found.update(int(g) for g in m.groups() if g)
    if not found:
        m = re.search(r'\blines? ' + rng, title)
        if m:
            found.update(int(g) for g in m.groups() if g)
    return sorted(found)


def read_placed(aud):
    """first_use.py's table for line-records.txt, as {(slice, record line, file, line):
    test name or OUTSIDE}; None when the table has not been written."""
    path = os.path.join(aud, 'mutation', LINE_RECORDS_MD)
    if not os.path.exists(path):
        return None
    out = {}
    for ln in lines_of(path):
        m = PLACED_ROW.match(ln)
        if m and m.group(1) != 'Slice':
            out[(m.group(1), int(m.group(2)), m.group(3), int(m.group(4)))] = m.group(5)
    return out


def read_changed(aud):
    path = os.path.join(aud, 'mutation', CHANGED_SINCE)
    if not os.path.exists(path):
        return set()
    return {ln.strip().replace('\\', '/') for ln in lines_of(path) if ln.strip() and not ln.startswith('#')}


def slice_of(r):
    return r['file'][:-3]


def match_record(r, tests, by_base, names):
    """How one record row meets the run: (match, test key or None, reason, candidates,
    line entries). The rule is the plan's E1c, "How a record is matched to a test".

    A record that is a line under its own heading (records/window-WN3.md line 54, under
    "Record 1. <test> (<file>:<line>)") is titled by that heading, so when its own line
    names no test and no line, the heading just above it is read the same way."""
    res = match_text(r, r['title'], 'its title', r['above'], tests, by_base, names)
    if res[0] == 'none' and 'no test name' in res[2] and r['kind'] != 'heading' \
            and r['above'] and r['above'][0][0] >= 2:
        alt = match_text(r, r['above'][0][1], f'the heading "{r["above"][0][1][:80]}"', r['above'][1:],
                         tests, by_base, names)
        if alt[0] != 'none':
            return alt
    return res


def match_text(r, title, where, above, tests, by_base, names):
    fs, src = test_files(title, by_base), where
    if not fs:
        for _, h in above:
            fs = test_files(h, by_base)
            if fs:
                src = f'the heading "{h[:80]}"'
                break
    run_fs = {p for p in fs.values() if p}
    hits = [t for t in names if t['name'] in title]
    if fs and not run_fs:
        why = f'its test file {", ".join(sorted(fs))} (from {src}) is not one of the run\'s files'
        if hits:
            why += '; its title also holds the run test ' + ', '.join(t['key'] for t in hits[:3])
        return 'not in the run', None, why, [], []
    if run_fs:
        inside = [t for t in hits if t['file'] in run_fs]
        if hits and not inside:
            return ('none', None, f'its title holds the run test {", ".join(t["key"] for t in hits[:3])}, '
                    f'which is not in its test file {", ".join(sorted(fs))} (from {src})', [], [])
        hits = inside
    # A name inside a longer name that also sits in the title is that longer name's.
    hits = [t for t in hits if not any(u is not t and t['name'] in u['name'] for u in hits)]
    if len(hits) == 1:
        return 'name', hits[0]['key'], '', [], []
    if hits:
        return 'several', None, 'its title holds more than one test name', [t['key'] for t in hits], []
    if not fs:
        return 'none', None, 'names no test file and no test name the run holds', [], []
    if len(fs) > 1:
        return ('none', None, f'names no test name the run holds and several test files '
                f'({", ".join(sorted(fs))}, from {src})', [], [])
    f = next(iter(run_fs))
    lines = title_lines(title, os.path.basename(f))
    if not lines:
        return 'none', None, f'names {f} (from {src}) but no test name and no line', [], []
    return 'line', None, '', [], [(slice_of(r), r['hline'], f, ln, src) for ln in lines]


def place_lines(entries, placed, tests, changed):
    """The match for a record whose lines first_use.py placed: (match, key, reason, candidates)."""
    if placed is None:
        return ('none', None, 'its line is not placed yet: mutation/line-records.md does not exist '
                '(task M4 runs first_use.py on line-records.txt)', [])
    got = []
    for s, md, f, ln, _ in entries:
        p = placed.get((s, md, f, ln))
        if p is None:
            return 'none', None, f'mutation/line-records.md has no placement for {f}:{ln}', []
        got.append((ln, p))
    found = sorted({p for _, p in got})
    if found == [OUTSIDE]:
        return 'none', None, 'its line sits outside every test case (first_use.py)', []
    if len(found) > 1:
        return ('several', None, 'its lines fall in more than one test',
                [f'{entries[0][2]}:{ln} {p}' for ln, p in got])
    by_name = [t for t in tests.values() if t['name'] == found[0] and t['file'] == entries[0][2]]
    if len(by_name) != 1:
        return 'none', None, f'first_use.py placed it on "{found[0]}", which tests.txt does not hold', []
    match = 'line, file changed since' if entries[0][2] in changed else 'line'
    return match, by_name[0]['key'], '', []


def mutation_columns(recs, ev, placed, changed):
    """Fills r['mut'] (one value per MUT_COLS name) and r['mut_why'], r['mut_cands'] on
    every record row. Returns the line entries for line-records.txt."""
    tests = run_index(ev)
    by_base = {os.path.basename(t['file']): t['file'] for t in tests.values()}
    names = sorted(tests.values(), key=lambda t: -len(t['name']))
    counts = per_test_counts(ev, tests)
    all_entries = []
    for r in recs:
        match, key, why, cands, entries = match_record(r, tests, by_base, names)
        if entries:
            all_entries += entries
            match, key, why, cands = place_lines(entries, placed, tests, changed)
        mut = {c: '' for c in MUT_COLS}
        mut['mut_match'], mut['mut_run'], mut['mut_limits'] = match, ev['run'], mutation_evidence.LIMITS
        if key:
            t = tests[key]
            mut.update(mut_test=key, mut_caught=str(t['caught']), mut_reaches=str(t['reaches']),
                       mut_caught_none='yes' if t['caught_none'] else 'no',
                       mut_caught_shared=str(counts['shared'][key]),
                       mut_running_at_limit=str(counts['running'][key]),
                       mut_red_in_timed_out=str(counts['red'][key]),
                       mut_crashed_running=str(counts['crashed'][key]))
        r['mut'], r['mut_why'], r['mut_cands'] = mut, why, cands
    return all_entries


def write_line_records(path, entries):
    """The records that give a line and no test name, in tools/mutate/first_use_lines.txt's columns."""
    out = ['# T8 M3 line records: each ledger record row whose title gives a test file the run held',
           '# and a line, but no test name. Written by exits/ledger.py; do not edit by hand.',
           '# A title giving several lines, or a range, has one entry per line (both ends of a range).',
           '# Columns (tab-separated): record file, record line, test file, test line, where the file came from.',
           '']
    out += ['\t'.join([s, str(md), f, str(ln), src.replace('\t', ' ')]) for s, md, f, ln, src in entries]
    with open(path, 'w', encoding='utf-8', newline='\n') as fh:
        fh.write('\n'.join(out) + '\n')


def unmatched_tests(recs, ev):
    tests = run_index(ev)
    named = {r['mut']['mut_test'] for r in recs if r['mut']['mut_test']}
    return [k for k in tests if k not in named]


def write_unmatched(path, recs, ev):
    """Every record row not matched to one test, by record id with its reason, and every
    run test no record row matched, by name."""
    left = unmatched_tests(recs, ev)
    tests = run_index(ev)
    kinds = ['not in the run', 'several', 'none']
    rows = {k: [r for r in recs if r['mut']['mut_match'] == k] for k in kinds}
    out = [f'# Mutation run {ev["run"]}: records and tests the ledger did not match', '',
           'ledger.py writes this file; do not edit it by hand. The rule that matches a record row to a '
           'test is in the plan, docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md, step E1c. '
           'Nothing here is judged, and no line here picks an exit.', '',
           mutation_evidence.LIMITS, '',
           f'There are {sum(len(v) for v in rows.values())} record rows ('
           + ', '.join(f'{len(rows[k])} {k}' for k in kinds)
           + f') and {len(left)} run tests that no record row matched.', '']
    for k in kinds:
        out += [f'## Record rows: {k} ({len(rows[k])})', '']
        for r in rows[k]:
            line = f'- records/{r["file"]}:{r["hline"]} "{r["record"][:100]}": {r["mut_why"]}.'
            if r['mut_cands']:
                line += ' Candidates: ' + '; '.join(r['mut_cands']) + '.'
            out.append(line)
        out.append('')
    none_title = mutation_evidence.TESTS_NONE_TITLE
    for title, want in ((none_title, True), (mutation_evidence.TESTS_SOME_TITLE, False)):
        ks = [k for k in left if tests[k]['caught_none'] == want]
        out += [f'## Run tests no record row matched, under "{title}" in tests.txt ({len(ks)})', '']
        out += [f'- {k}' for k in ks] + ['']
    with open(path, 'w', encoding='utf-8', newline='\n') as fh:
        fh.write('\n'.join(out))
    return left


def read_old_mut(path):
    """The mut_ columns of the record rows of the ledger about to be replaced, by row id."""
    if not os.path.exists(path):
        return {}
    lines = lines_of(path)
    head = lines[0].split('\t') if lines else []
    if 'mut_match' not in head:
        return {}
    ix = {c: head.index(c) for c in MUT_COLS if c in head}
    out = {}
    for ln in lines[1:]:
        cells = ln.split('\t')
        if cells[0] == 'record':
            out[cells[head.index('id')]] = {c: cells[i] if i < len(cells) else '' for c, i in ix.items()}
    return out


def mut_changes(old, recs):
    """Row ids whose mut_ columns differ from the old ledger's, with the columns that differ."""
    if not old:
        return []
    out = []
    for r in recs:
        rid = f"records/{r['file']}:{r['hline']}"
        was = old.get(rid)
        diff = [c for c in MUT_COLS if was is None or was.get(c, '') != r['mut'][c]]
        if diff:
            out.append((rid, diff, was, r['mut']))
    return out


def write_not_in_ledger(path, unread, counts):
    """List what the script could not read, by file and line, for the scouts to fill."""
    unj = [r for r in counts if not r['join']]
    out = ['# Records the ledger script could not read', '',
           'ledger.py writes this file; do not edit it by hand. It lists two kinds of line, '
           'grouped by file. A record title is a line that looks like a record but has no '
           'verdict the script could read under it. A count row is a row of a recount table '
           'that the script could not tie to exactly one record row; the reason says why. '
           'Nothing here is judged. Scouts read each one and fill its ledger columns by hand.', '',
           f'There are {len(unread)} record titles and {len(unj)} count rows.', '']
    by = {}
    for f, ln, h in unread:
        by.setdefault('records/' + f, []).append(
            f'- Line {ln} looks like a record titled "{h[:120]}", but the script found no verdict under it.')
    for r in unj:
        by.setdefault(f"counts/{r['cf']}.md", []).append(
            f'- Line {r["line"]} is a count row for "{r["text"][:100]}". It is not joined: {r["why"]}.')
    for f in sorted(by):
        out += [f'## {f}', ''] + by[f] + ['']
    with open(path, 'w', encoding='utf-8', newline='\n') as fh:
        fh.write('\n'.join(out))
    return sum(len(v) for v in by.values())


def short_sections(rs):
    """Sections whose title states a record count ("D12 (40 Keep records)") that the
    table under it does not hold: rows the counter's file never wrote."""
    seen = {}
    for r in rs:
        seen[r['section']] = seen.get(r['section'], 0) + 1
    out = []
    for sec, have in seen.items():
        m = re.search(r'\((\d+) (?:Keep )?records\)', sec)
        if m and int(m.group(1)) != have:
            out.append((sec, int(m.group(1)), have))
    return out


def totals_section1():
    out = {}
    for line in lines_of(os.path.join(AUD, 'counts', 'TOTALS.md')):
        m = re.match(r'^\| (T5-\w+) \| (\d+) \| (\d+)', line)
        if m:
            out[m.group(1)] = (int(m.group(2)), int(m.group(3)))
    return out


# Plan step E5: the side notes, splits and production-code findings, pulled out as
# blocks with no judgment. The folders and the plan's rough file counts (E5's second
# paragraph) are printed beside what this reader finds.
NOTE_DIRS = ['records', 'verify', 'capture', 'sweep', 'counts']   # capture/ with all its subfolders
NOTE_KINDS = [
    ('side note', re.compile(r'side note', re.I)),
    # A line that starts "split", after list, bold, heading or number marks.
    ('split', re.compile(r'^[\s>#*_-]*(?:\d+\.\s*)?[\s*_]*split', re.I)),
    # The plan's phrase is "production-code finding". Its records also write
    # "Production code finding" and "Finding (production code)", so a line holding
    # both words in either order counts; the printout also gives the plan phrase alone.
    ('production-code finding', re.compile(r'production[- ]code.*finding|finding.*production[- ]code', re.I)),
]
PLAN_PHRASE = re.compile(r'production-code finding', re.I)
PLAN_FILES = {('records', 'side note'): 97, ('verify', 'side note'): 35, ('capture/byreading', 'side note'): 10,
              ('capture/rejudge', 'side note'): 3, ('sweep', 'side note'): 3, ('counts', 'side note'): 1,
              ('records', 'split'): 86, ('verify', 'split'): 116, ('records', 'production-code finding'): 53}
NOTE_COLS = ['kind', 'file', 'line', 'record_heading', 'ledger_id', 'text']


def note_files(aud):
    """Every .md file under the note folders, AUD-relative with forward slashes."""
    out = []
    for d in NOTE_DIRS:
        top = os.path.join(aud, d)
        if not os.path.isdir(top):
            continue
        for root, dirs, files in os.walk(top):
            if d != 'capture':
                dirs[:] = []
            dirs.sort()
            for f in sorted(files):
                if f.endswith('.md'):
                    out.append(os.path.relpath(os.path.join(root, f), aud).replace('\\', '/'))
    return out


def note_blocks(lines):
    """(kind, line number, nearest heading above or '', heading's line or 0, block text)
    for each hit line. A block is the hit line and the lines after it up to the next
    blank line or heading. Lines inside a code fence are never headings."""
    heads, fence = [], False
    for line in lines:
        if line.strip().startswith('```'):
            fence = not fence
        heads.append(not fence and bool(HEADING.match(line)))
    out, last, last_line = [], '', 0
    for i, line in enumerate(lines):
        for kind, pat in NOTE_KINDS:
            if pat.search(line):
                block = [line]
                for nxt, is_head in zip(lines[i + 1:], heads[i + 1:]):
                    if not nxt.strip() or is_head:
                        break
                    block.append(nxt)
                out.append((kind, i + 1, last, last_line, clean(' '.join(block))))
        if heads[i]:
            last, last_line = clean(HEADING.match(line).group(2)), i + 1
    return out


def read_notes(aud, recs):
    """One row per block in every note file. A block under a records/ heading gets the
    ledger id of the record row that starts at that heading, or else of the one record
    row of that file under that heading; none when neither finds exactly one."""
    by_start = {(r['file'], r['hline']): r for r in recs}
    by_head = {}
    for r in recs:
        by_head.setdefault((r['file'], r['heading']), []).append(r)
    rows, hits = [], {}
    for rel in note_files(aud):
        lines = lines_of(os.path.join(aud, rel))
        name = rel.split('/', 1)[1] if rel.startswith('records/') and rel.count('/') == 1 else None
        for kind, ln, head, hline, text in note_blocks(lines):
            rid = ''
            if name and head:
                r = by_start.get((name, hline))
                cands = [r] if r else by_head.get((name, head), [])
                if len(cands) == 1:
                    rid = f"records/{cands[0]['file']}:{cands[0]['hline']}"
            rows.append({'kind': kind, 'file': rel, 'line': ln, 'record_heading': head,
                         'ledger_id': rid, 'text': text})
        hits[rel] = any(PLAN_PHRASE.search(x) for x in lines)
    return rows, hits


def note_folder(rel):
    parts = rel.split('/')
    return '/'.join(parts[:2]) if parts[0] == 'capture' and len(parts) > 2 else parts[0]


def write_notes(path, rows):
    with open(path, 'w', encoding='utf-8', newline='\n') as fh:
        fh.write('\t'.join(NOTE_COLS) + '\n')
        for r in rows:
            fh.write('\t'.join(str(r[c]).replace('\t', ' ') for c in NOTE_COLS) + '\n')


def print_notes(rows, phrase_hits):
    print('\nnotes.tsv (plan step E5): per folder and kind, files hit / blocks, beside the plan\'s file count')
    folders = sorted({note_folder(r['file']) for r in rows} | {f for f, _ in PLAN_FILES})
    for f in folders:
        for kind, _ in NOTE_KINDS:
            rs = [r for r in rows if note_folder(r['file']) == f and r['kind'] == kind]
            nf = len({r['file'] for r in rs})
            plan = PLAN_FILES.get((f, kind))
            if not rs and plan is None:
                continue
            say = '' if plan is None else f'   plan {plan:4}' + ('' if plan == nf else f'   DIFFERS by {nf - plan:+}')
            print(f'  {f:20} {kind:24} {nf:4} / {len(rs):5}{say}')
    for f in ('records', 'verify'):
        n = sum(1 for rel, hit in phrase_hits.items() if hit and note_folder(rel) == f)
        print(f'  {f}: files holding the plan\'s exact phrase "production-code finding": {n}')
    print(f'  all blocks: {len(rows)}; with a ledger id: {sum(1 for r in rows if r["ledger_id"])}')


def main():
    recs, unread = read_records()
    counts, notable = read_counts()
    left = apply_corrections(counts, read_corrections())
    settle = apply_answers(counts)
    try:
        settled = settle_answer4(counts)
    except LedgerError as e:
        print('ledger: answer 4:', e, file=sys.stderr)
        sys.exit(1)
    join_counts(counts, recs)
    seconds = second_verdicts(recs)
    try:
        ev = mutation_evidence.read_evidence(AUD)
        entries = mutation_columns(recs, ev, read_placed(AUD), read_changed(AUD))
    except mutation_evidence.EvidenceError as e:
        print(e, file=sys.stderr)
        sys.exit(1)
    old_mut = read_old_mut(OUT)
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    cols = ['kind', 'file', 'line', 'heading_or_section', 'record', 'test', 'verdict', 'earlier_verdicts',
            'faults_counter', 'faults', 'correction', 'answer', 'held', 't6', 't7_reopened', 'reason',
            'id', 'joins_record', 'verifier_verdict_if_different', 'byreading_verdict'] + MUT_COLS
    with open(OUT, 'w', encoding='utf-8', newline='\n') as fh:
        fh.write('\t'.join(cols) + '\n')
        for r in recs:
            v = r['verdicts']
            t6 = 't6' if (r['file'], (re.match(r'(\d+)', r['record']) or [None, ''])[1]) in T6 else ''
            fh.write('\t'.join(['record', 'records/' + r['file'], str(r['line']), r['heading'], r['record'],
                                '', v[-1], ','.join(v[:-1]), '', '', '', '', '', t6,
                                'yes' if r['file'] in T7_REOPENED else '', '',
                                f"records/{r['file']}:{r['hline']}", '',
                                ','.join(r.get('verifier', [])), ','.join(r.get('byreading', []))]
                               + [r['mut'][c] for c in MUT_COLS]) + '\n')
        for r in counts:
            # A count row reaches its record's mut_ columns through joins_record; it has none of its own.
            fh.write('\t'.join(['count', 'counts/' + r['cf'] + '.md', str(r['line']), r['section'],
                                (r['rec'] + ' ' + r['num']).strip(), r['test'], '', '',
                                r['faults_raw'], faults_text(r['faults']), r['fix'], r['answer'], held(r),
                                '', '', r['reason'][:200], '', r['join'], '', '']
                               + [''] * len(MUT_COLS)) + '\n')
    mdir = os.path.join(AUD, 'mutation')
    write_line_records(os.path.join(mdir, LINE_RECORDS_TXT), entries)
    unm_path = os.path.join(mdir, UNMATCHED)
    no_record = write_unmatched(unm_path, recs, ev)

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
        if flag:
            for sec, want, have in short_sections(rs):
                print(f'           section "{sec}" says {want} records and its table holds {have}')
    rows_all = sum(1 for r in counts if r['cf'] in tot)
    faulty_all = sum(1 for r in counts if r['cf'] in tot and r['faults'])
    print(f'  all      {rows_all:4} / {faulty_all:4}   vs  {sum(v[0] for v in tot.values()):4} / '
          f'{sum(v[1] for v in tot.values()):4}   (TOTALS s1 sums; s1 faulty is before answers 2 and 3)')
    print('\nper count file: rows joined to one record row / unjoined')
    for cf in sorted({r['cf'] for r in counts}):
        rs = [r for r in counts if r['cf'] == cf]
        j = sum(1 for r in rs if r['join'])
        print(f'  {cf:8} {j:4} / {len(rs) - j:4}')
    unj = [r for r in counts if not r['join']]
    print(f'  all      {len(counts) - len(unj):4} / {len(unj):4}')
    print('\nunjoined count rows, by file and line:', len(unj))
    for r in unj:
        print(f'  counts/{r["cf"]}.md:{r["line"]} {r["why"]} | {r["text"][:70]}')
    for kind in ('byreading', 'verify'):
        rs = [r for r in seconds if r['kind2'] == kind]
        j = sum(1 for r in rs if r['join'])
        print(f'\n{kind} verdict lines read: {len(rs)}, joined to one record row: {j}, unjoined: {len(rs) - j}')
    print('record rows with a verifier verdict that differs:', sum(1 for r in recs if r.get('verifier')),
          '| with a by-reading verdict:', sum(1 for r in recs if r.get('byreading')))
    print('\ncorrections not applied (count of candidate rows):')
    for c, n in left:
        print(f'  {n} candidates: {c["line"]}')
    print('\nheadings that look like records but have no verdict line the script read:', len(unread))
    for f, ln, h in unread:
        print(f'  records/{f}:{ln} {h[:100]}')
    print('\ntables skipped:', len(notable))
    for cf, ln, why in notable:
        print(f'  {cf}:{ln} {why}')
    gone = sum(1 for r, before in settled if not r['faults'])
    print(f'\nanswer 4 changed {len(settled)} rows of the {len(settle)} it held ({ANSWER4_FILE}): '
          f'{gone} go from faulty to none, {len(settled) - gone} keep another fault; '
          f'rows still held: {sum(1 for r in counts if held(r))}')
    for r, before in settled:
        print(f'  counts/{r["cf"]}.md:{r["line"]} {r["rec"]} [{faults_text(before)}] to [{faults_text(r["faults"])}]')
    notes, phrase_hits = read_notes(AUD, recs)
    notes_path = os.path.join(os.path.dirname(OUT), 'notes.tsv')
    write_notes(notes_path, notes)
    print_notes(notes, phrase_hits)
    nil = os.path.join(os.path.dirname(OUT), 'not-in-ledger.md')
    print('\nnot-in-ledger.md lines (one per unread record title or unjoined count row):',
          write_not_in_ledger(nil, unread, counts))
    mc = Counter(r['mut']['mut_match'] for r in recs)
    print(f'\nmutation run {ev["run"]}: record rows by mut_match:', dict(mc), '| total', sum(mc.values()))
    nir = [r for r in recs if r['mut']['mut_match'] == 'not in the run']
    py = sum(1 for r in nir if '.py' in r['mut_why'].split(' (from ')[0])
    print(f'  not in the run: {py} name a Python file, {len(nir) - py} another file')
    print('  line records written to', os.path.join(mdir, LINE_RECORDS_TXT), ':', len(entries), 'entries from',
          len({(e[0], e[1]) for e in entries}), 'record rows;', LINE_RECORDS_MD,
          'read' if read_placed(AUD) is not None else 'not there yet')
    named = Counter(r['mut']['mut_test'] for r in recs if r['mut']['mut_test'])
    print(f'  run tests: {len(run_index(ev))}; matched by a record row {len(named)}, by two or more '
          f'{sum(1 for n in named.values() if n > 1)}; no record row {len(no_record)}')
    ch = mut_changes(old_mut, recs)
    print('\nrecord rows whose mut_ columns changed since the last ledger:',
          len(ch) if old_mut else 'none to compare (the last ledger had no mut_ columns)')
    for rid, diff, was, now in ch:
        print(f'  {rid}: ' + '; '.join(f'{c} {(was or {}).get(c, "")!r} to {now[c]!r}'
                                        for c in diff if c != 'mut_limits'))
    print('\nwrote', OUT, ',', nil, ',', notes_path, 'and', unm_path)


if __name__ == '__main__':
    main()
