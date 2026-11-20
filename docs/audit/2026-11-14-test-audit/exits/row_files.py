"""Map each labels.tsv row to the test file it lives in. Usage: rowfiles.py <repo> <out.tsv>"""
import csv, re, subprocess, sys, collections, os
repo, out = sys.argv[1], sys.argv[2]
A = os.path.join(repo, 'docs/audit/2026-11-14-test-audit')
files = subprocess.run(['git', '-C', repo, 'ls-files'], capture_output=True, text=True).stdout.split('\n')
by_base = collections.defaultdict(list)
for f in files:
    by_base[f.rsplit('/', 1)[-1]].append(f)
PAT = re.compile(r'([\w./\\-]+\.(?:cpp|h|hpp|py|ps1|psm1|js|json))')

def resolve(tok):
    tok = tok.replace('\\', '/')
    if tok in by_base.get(tok.rsplit('/', 1)[-1], []):
        return tok, 'exact'
    c = [f for f in by_base.get(tok.rsplit('/', 1)[-1], []) if f.endswith(tok)]
    if len(c) == 1:
        return c[0], 'base'
    if len(c) > 1:
        t = [f for f in c if '/tests' in '/' + f or f.startswith('tests')]
        return ('|'.join(c), 'ambiguous')
    return tok, 'notinrepo'

led = {}
with open(os.path.join(A, 'exits/ledger.tsv'), encoding='utf-8') as fh:
    for r in csv.DictReader(fh, delimiter='\t'):
        if r['id']:
            led[r['id']] = r

cache = {}
def count_line(path, n):
    if path not in cache:
        with open(os.path.join(A, path), encoding='utf-8') as fh:
            cache[path] = fh.read().split('\n')
    return cache[path], cache[path][n - 1]

# A legend code is a short upper-case name: letters (PS, MIDI), one letter (Q), or letters and digits (S2).
CODE = r'[A-Z][A-Z0-9]{0,4}'

# legend: lines like "`PG` = tests/x.py" or "PG is test_x.py" near the top
def legend(lines):
    m = {}
    for ln in lines[:80]:
        for code, f in re.findall(r'`?\b(%s)\b`?\s*(?:=|is|means|for|:)\s*`?([\w./-]+\.(?:py|cpp|ps1))' % CODE, ln):
            m[code] = f
        for f, code in re.findall(r'`?([\w./-]+\.(?:py|cpp|ps1))`?\s*\(`?(%s)`?\)' % CODE, ln):
            m[code] = f
    return m

HOOKS = 'C:/Users/Patrick/.claude'
hook_files = subprocess.run(['git', '-C', HOOKS, 'ls-files'], capture_output=True, text=True).stdout.split('\n')
for f in hook_files:
    if f:
        by_base[f.rsplit('/', 1)[-1]].append('~/.claude/' + f)
names = collections.defaultdict(set)
for root, fl in ((repo, files), (HOOKS, hook_files)):
    for f in fl:
        if not f or not re.search(r'(^|/)(tests?|tools|gates)/|test_', f) or not f.endswith(('.py', '.cpp', '.ps1', '.h')):
            continue
        try:
            txt = open(os.path.join(root, f), encoding='utf-8', errors='replace').read()
        except OSError:
            continue
        key = f if root == repo else '~/.claude/' + f
        for nm in re.findall(r'def\s+(test_\w+)|function\s+(Test-\w+|test_\w+)', txt):
            names[nm[0] or nm[1]].add(key)
        for nm in re.findall(r'TEST_CASE\("([^"]+)"', txt):
            names[nm].add(key)

prefix = collections.defaultdict(set)
for nm, fs in list(names.items()):
    if not nm.startswith('test_') and len(nm) >= 12:
        prefix[nm[:30].lower()] |= fs

def by_name(text, scope_file):
    t = re.sub(r'^[\W\d]+\.?\s*|`', '', text).lower()
    for k in (t[:30],):
        hit = prefix.get(k)
        if hit and len(hit) == 1:
            return next(iter(hit)), 'name'
    for mm in re.finditer(r'"([^"]{12,})"|`([^`]{12,})`', text):
        s = (mm.group(1) or mm.group(2)).lower()[:30]
        hit = prefix.get(s)
        if hit and len(hit) == 1:
            return next(iter(hit)), 'name'
    cands = re.findall(r'\b(test_\w+)', text)
    for c in cands:
        hit = names.get(c, set())
        if len(hit) == 1:
            return next(iter(hit)), 'name'
        if len(hit) > 1 and scope_file:
            hs = [h for h in hit if h.endswith(scope_file)]
            if len(hs) == 1:
                return hs[0], 'name'
    return None

def legend2(lines):
    m = legend(lines)
    for ln in lines[:80]:
        for f, code in re.findall(r'`([\w./-]+\.(?:py|cpp|ps1))`\s*\(`?(%s)\b' % CODE, ln):
            m[code] = f
        for code, f in re.findall(r'`?\b(%s)\b`?\s*(?:is|=|:|for|means)\s*`([\w./-]+\.(?:py|cpp|ps1))`' % CODE, ln):
            m[code] = f
        for code, f in re.findall(r'\|\s*`?(%s)`?\s*\|\s*`([\w./-]+\.(?:py|cpp|ps1))`' % CODE, ln):
            m[code] = f
        # A file the legend lists with no code is cited by its stem: test_process.py as "process:186".
        for f, stem in re.findall(r'\b(test_(\w+)\.(?:py|cpp|ps1))\b', ln):
            m.setdefault(stem, f)
    return m

def header_files(lines):
    found = []
    for ln in lines[:40]:
        found += PAT.findall(ln)
    return list(dict.fromkeys(found))

def is_test(f):
    # The same rule as is_test in chunks.py: the path of a test file, not production code.
    return ('/tests/' in f or f.startswith('tests/') or '/test_' in f or f.startswith('test_')
            or f.rsplit('/', 1)[-1].startswith('test_') or '/ui/uitest' in f or 'report_samples' in f)

def heading_test(path, n):
    """A record sits under a `###` heading that names its test, as (tests/<file>:<line>).
    That heading beats the record's Read: line, which names the production code it read.
    Returns (file, 'heading-exact' or 'heading-base'), or None."""
    if not os.path.exists(os.path.join(A, path)):
        return None
    lines, _ = count_line(path, n)
    j = next((k for k in range(n - 1, -1, -1) if lines[k].startswith('#')), None)
    if j is None or not lines[j].startswith('###'):
        return None
    for tok in PAT.findall(lines[j]):
        f, how = resolve(tok.split(':')[0])
        if how in ('exact', 'base') and is_test(f):
            return f, 'heading-' + how
    return None

def table_file(lines, k, sep, col):
    """The file of the nearest row above line index k, down to the table's separator at index sep,
    whose test cell names one: a path, or a legend code such as "Q :87" or "process:186".
    Rows whose cell is only ":N" are passed over. Returns (file, 'table-<how>') or None."""
    lg = legend2(lines)
    for j in range(k, sep, -1):
        cells = [c.strip() for c in lines[j].strip().strip('|').split('|')]
        cell = cells[col] if len(cells) > col else ''
        if re.match(r'^:\s*\d', cell):
            continue
        mm = PAT.search(cell)
        if mm:
            f, how = resolve(mm.group(1).split(':')[0])
            return f, 'table-' + how
        cm = re.match(r'(%s|[a-z]\w*)\s*:\s*\d' % CODE, cell)
        if cm and cm.group(1) in lg:
            f, how = resolve(lg[cm.group(1)])
            return f, 'table-' + how
        return None
    return None

stats = collections.Counter()
rows_out = []
with open(os.path.join(A, 'exits/labels/labels.tsv'), encoding='utf-8') as fh:
    for r in csv.DictReader(fh, delimiter='\t'):
        row = r['row']; how = ''; tf = ''
        path, _, n = row.rpartition(':')
        if r['kind'] == 'count' and path.startswith('counts/'):
            lines, ln = count_line(path, int(n))
            cells = [c.strip() for c in ln.strip().strip('|').split('|')]
            i = int(n) - 1
            while i > 0 and not re.match(r'^\|\s*:?-{3}', lines[i]):
                i -= 1
            hdr = [c.strip().lower() for c in lines[i - 1].strip().strip('|').split('|')] if i > 0 else []
            col = next((k for k, h in enumerate(hdr) if h in ('test', 'test case', 'test name', 'case', 'file:line', 'test (file:line)')), None)
            if col is None:
                col = next((k for k, h in enumerate(hdr) if 'test' in h or 'file' in h), 1)
            test = cells[col] if len(cells) > col else ''
            # A test cell that is only ":154" continues the file of the nearest row above it in the
            # same table that names one. A file named elsewhere on its line is its test only when
            # it is a test file: "scan.cpp:538-548" in the faulty-check column is, but "rows from
            # gen_vectors.py" in the reason column is not. With no such row above, the table's
            # heading or the file's legend decides, as for any row whose cell names no file.
            cont = re.match(r'^:\s*\d', test) is not None
            m = PAT.search(test) or PAT.search(ln)
            if cont and m and not PAT.search(test) and not is_test(resolve(m.group(1).split(':')[0])[0]):
                m = None
            above = table_file(lines, int(n) - 2, i, col) if cont and not m else None
            test = test if test else ln
            if above:
                tf, how = above
            elif m:
                tf, how = resolve(m.group(1).split(':')[0])
            else:
                lg = legend2(lines)
                code = next((mm for mm in re.finditer(r'\b(%s|[a-z]\w*)\s*:\s*\d' % CODE, ln) if mm.group(1) in lg), None)
                hf = header_files(lines)
                nb = by_name(ln, None)
                if code and code.group(1) in lg:
                    tf, how = resolve(lg[code.group(1)])
                    how = 'legend-' + how
                elif nb:
                    tf, how = nb
                elif any(lines[j].startswith('#') and PAT.search(lines[j]) for j in range(int(n) - 2, max(int(n) - 400, 0), -1)):
                    j = next(j for j in range(int(n) - 2, max(int(n) - 400, 0), -1) if lines[j].startswith('#') and PAT.search(lines[j]))
                    if any(lines[k].startswith('#') and not PAT.search(lines[k]) for k in range(j + 1, int(n) - 1)):
                        tf, how = test, 'unresolved'
                    else:
                        tf, how = resolve(PAT.search(lines[j]).group(1))
                        how = 'header-' + how
                elif len(hf) == 1:
                    tf, how = resolve(hf[0])
                    how = 'header-' + how
                else:
                    tf, how = test, 'unresolved'
        else:
            lr = led.get(row)
            text = (lr['record'] + ' ' + lr['test']) if lr else ''
            m = PAT.search(text)
            nb = by_name(text, None)
            ht = heading_test(path, int(n))
            if ht:
                tf, how = ht
            elif m:
                tf, how = resolve(m.group(1).split(':')[0])
            elif nb:
                tf, how = nb
            else:
                rl, _ = count_line(path, int(n)) if os.path.exists(os.path.join(A, path)) else ([], '')
                hf = header_files(rl)
                if len(hf) == 1:
                    tf, how = resolve(hf[0]); how = 'header-' + how
                else:
                    tf, how = text[:60], 'unresolved'
        stats[how.split('-')[-1] if how.startswith('legend') else how] += 1
        stats['legend' if how.startswith('legend') else 'direct'] += 1
        rows_out.append((r['batch'], r['kind'], row, r['keys'], tf, how))

# Last resort: a row takes the file of the nearest resolved row above it in the same audit file.
GOOD = ('exact', 'base', 'name', 'legend-exact', 'legend-base', 'header-exact', 'header-base',
        'heading-exact', 'heading-base', 'table-exact', 'table-base', 'notinrepo')
resolved = collections.defaultdict(list)
for b, k, row, keys, tf, how in rows_out:
    if how in GOOD:
        p, _, n = row.rpartition(':')
        resolved[p].append((int(n), tf))
for p in resolved:
    resolved[p].sort()
for i, (b, k, row, keys, tf, how) in enumerate(rows_out):
    if how == 'unresolved':
        p, _, n = row.rpartition(':')
        above = [f for (ln, f) in resolved.get(p, []) if ln < int(n)]
        if above:
            rows_out[i] = (b, k, row, keys, above[-1], 'inherited')
            stats['inherited'] += 1
            stats['unresolved'] -= 1

with open(out, 'w', encoding='utf-8', newline='') as fh:
    w = csv.writer(fh, delimiter='\t', lineterminator='\n')
    w.writerow(['batch', 'kind', 'row', 'keys', 'test_file', 'how'])
    w.writerows(rows_out)
print(len(rows_out), dict(stats))
