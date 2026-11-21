"""T8 E1c (task M1): the mutation run's evidence, copied into the audit, and its one reader.

Usage:
  python -I mutation_evidence.py --copy-run <run folder> [AUD folder]
  python -I mutation_evidence.py [AUD folder]

With --copy-run it copies report.txt, tests.txt and summary.json from the run
folder into AUD/mutation/ byte for byte, writes timed-out-bugs.txt (the
timed-out bugs' blocks of the run's bugs.txt, byte for byte), then writes
survivors-by-file.md and README.md. Without it, it reads the copies already in
AUD/mutation/ and writes survivors-by-file.md and README.md again. The AUD
folder defaults to this script's parent folder.

The plan is docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md, step
E1c and task M1. read_evidence() is the one reader of the four copied files;
ledger.py (task M3) calls it and reads the run through nothing else.

The text format of report.txt, bugs.txt and tests.txt is owned by
tools/mutate/report.py on main (report_text, bugs_text, tests_text,
STATE_LISTS, STATE_WORDS). That tool only writes the files and has no reader,
and this script reads frozen copies, so it does not import it. Each reader
stops with a message when a heading it needs is missing or a list's length
differs from the count its heading states, so a changed format fails loudly
instead of reading short.
"""
import argparse
import json
import os
import re
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))

# The files this step keeps, by name. The first three are copied whole.
WHOLE = ('report.txt', 'tests.txt', 'summary.json')
TIMED_OUT_FILE = 'timed-out-bugs.txt'
SURVIVORS_FILE = 'survivors-by-file.md'
README_FILE = 'README.md'

# report.txt's titles for the two lists this step reads (report.py, STATE_LISTS).
SURVIVORS_TITLE = 'Survivors: bugs reached but caught by no test'
TIMED_OUT_TITLE = 'Timed-out bugs, counted as caught but listed apart'
# How bugs.txt starts a timed-out bug's state line (report.py, STATE_WORDS).
TIMED_OUT_STATE = 'timed out in its run'
# tests.txt's two lists (report.py, tests_text).
TESTS_NONE_TITLE = 'Tests that caught no bug'
TESTS_SOME_TITLE = 'Tests that caught at least one bug'

# The report's own limit for survivors, quoted by the plan's E1c, "The survivors
# list". The script stops if report.txt's limits do not hold it word for word.
SURVIVOR_LIMIT = ('No tool can say whether a survivor is an equivalent bug, '
                  'one that changes nothing a user could see.')

# The one fixed sentence naming the run's four limits (E1c, "The limits").
# M3 puts it on every ledger row with a mut_test; README.md spells each limit out.
LIMITS = ('Mutation run evidence, with four limits: (1) the tool plants its own kinds of bug, '
          'not the change an auditor named, so caught nothing is not proof a test is empty; '
          '(2) the run went several bugs at once, so a timed-out bug lists its red tests '
          'but credits none of them as catches; (3) each verdict comes from one run of its bug; '
          '(4) the run held only the C++ tests in hydra_tests, so a test outside it has no '
          'evidence here, which is not the same as catching nothing. '
          'See docs/audit/2026-11-14-test-audit/mutation/README.md.')

SECTION = re.compile(r'^== (.*) \((\d+)\)$')
BUG_LINE = re.compile(r'^bug (\d+): (.+?) line (\d+), (.*)$')
BUG_HEAD = re.compile(r'^bug (\d+): ')
TEST_ROW = re.compile(r'^caught (\d+) \| reaches (\d+) \| (.+)$')
RUN_ID = re.compile(r'^Mutation report for run (\S+)\.$')
BUG_RUNS = re.compile(r'^Bug runs: (\d+): (.*)\.$')
TESTS_NONE_COUNT = re.compile(r'Tests that caught no bug: (\d+)\.')


class EvidenceError(RuntimeError):
    pass


def _text_lines(path):
    """The file's lines with their own line ends kept, so a copy can be byte for byte."""
    with open(path, 'rb') as f:
        return f.read().decode('utf-8').splitlines(keepends=True)


def _bare(line):
    return line.rstrip('\r\n')


def parse_bug_line(line):
    """One 'bug N: <file> line <L>, <change>' line of report.txt as its parts."""
    m = BUG_LINE.match(line)
    if not m:
        raise EvidenceError(f'mutation_evidence: not a bug line: {line!r}')
    return {'number': int(m.group(1)), 'file': m.group(2), 'line': int(m.group(3)),
            'change': m.group(4), 'text': line}


def read_report(path):
    """report.txt: its run id, its limits, its count lines and every '== ' list.

    Each list's length is checked against the count in its heading.
    """
    lines = [_bare(x) for x in _text_lines(path)]
    m = RUN_ID.match(lines[0]) if lines else None
    if not m:
        raise EvidenceError(f'mutation_evidence: {path} does not open with its run id')
    out = {'run': m.group(1), 'limits': [], 'counts': [], 'sections': {}, 'by_state': {}}
    try:
        start = lines.index('Limits of this report.') + 1
        counts_at = lines.index('Counts.') + 1
    except ValueError:
        raise EvidenceError(f'mutation_evidence: {path} has no limits or counts block')
    for block, at in (('limits', start), ('counts', counts_at)):
        while at < len(lines) and lines[at]:
            out[block].append(lines[at])
            at += 1
    title = None
    for ln in lines:
        s = SECTION.match(ln)
        if s:
            title = s.group(1)
            if title in out['sections']:
                raise EvidenceError(f'mutation_evidence: {path} has the list {title!r} twice')
            out['sections'][title] = {'count': int(s.group(2)), 'lines': []}
        elif title is not None and ln:
            out['sections'][title]['lines'].append(ln)
    for t, sec in out['sections'].items():
        if len(sec['lines']) != sec['count']:
            raise EvidenceError(f'mutation_evidence: {path}, list {t!r} says {sec["count"]} '
                                f'and holds {len(sec["lines"])} lines')
    for ln in out['counts']:
        b = BUG_RUNS.match(ln)
        if b:
            for part in b.group(2).split(', '):
                word, _, n = part.rpartition(' ')
                out['by_state'][word] = int(n)
        z = TESTS_NONE_COUNT.search(ln)
        if z:
            out['tests_caught_none'] = int(z.group(1))
    for t in (SURVIVORS_TITLE, TIMED_OUT_TITLE):
        if t not in out['sections']:
            raise EvidenceError(f'mutation_evidence: {path} has no list {t!r}')
    return out


def survivors(report):
    """The survivors of report.txt, each line parsed, in the report's order."""
    return [parse_bug_line(ln) for ln in report['sections'][SURVIVORS_TITLE]['lines']]


def timed_out_numbers(report):
    return [parse_bug_line(ln)['number'] for ln in report['sections'][TIMED_OUT_TITLE]['lines']]


def split_bug_blocks(raw_lines):
    """bugs.txt's lines as {bug number: its block}, each block its raw lines.

    A block runs from its 'bug N: ' line to the blank line after it, which it keeps.
    """
    blocks, cur = {}, None
    for ln in raw_lines:
        h = BUG_HEAD.match(ln)
        if h:
            cur = int(h.group(1))
            if cur in blocks:
                raise EvidenceError(f'mutation_evidence: bug {cur} has two blocks')
            blocks[cur] = [ln]
        elif cur is not None:
            blocks[cur].append(ln)
            if not _bare(ln):
                cur = None
    return blocks


def parse_block(raw):
    """One bugs.txt block as the facts M3 reads from it."""
    lines = [_bare(x) for x in raw if _bare(x)]
    head = parse_bug_line(lines[0])
    out = {'number': head['number'], 'header': lines[0], 'state': lines[1].strip() if len(lines) > 1 else '',
           'red_in_file': [], 'running_at_limit': None, 'lines': lines}
    for ln in lines[2:]:
        s = ln.strip()
        if s.startswith('red in its file,'):
            out['red_in_file'].append(s.split(': ', 1)[1])
        elif s.startswith('running at the time limit: '):
            out['running_at_limit'] = s[len('running at the time limit: '):]
    return out


def read_timed_out(path):
    """timed-out-bugs.txt as a list of parsed blocks, in file order."""
    blocks = split_bug_blocks(_text_lines(path))
    out = [parse_block(b) for b in blocks.values()]
    for b in out:
        if not b['state'].startswith(TIMED_OUT_STATE):
            raise EvidenceError(f'mutation_evidence: {path} holds bug {b["number"]}, '
                                f'which did not time out: {b["state"]!r}')
    return out


def read_tests(path):
    """tests.txt as {test: {caught, reaches, caught_none, bugs}}, the test as tests.txt writes it."""
    tests, title, last, counts = {}, None, None, {}
    for ln in (_bare(x) for x in _text_lines(path)):
        s = SECTION.match(ln)
        if s:
            title = s.group(1)
            counts[title] = [int(s.group(2)), 0]
            continue
        r = TEST_ROW.match(ln)
        if r and title in (TESTS_NONE_TITLE, TESTS_SOME_TITLE):
            name = r.group(3)
            if name in tests:
                raise EvidenceError(f'mutation_evidence: {path} names {name} twice')
            tests[name] = {'caught': int(r.group(1)), 'reaches': int(r.group(2)),
                           'caught_none': title == TESTS_NONE_TITLE, 'bugs': []}
            counts[title][1] += 1
            last = name
        elif ln.startswith('  bugs: ') and last is not None:
            tests[last]['bugs'] = [int(n) for n in ln[len('  bugs: '):].split(', ')]
    for t in (TESTS_NONE_TITLE, TESTS_SOME_TITLE):
        if t not in counts:
            raise EvidenceError(f'mutation_evidence: {path} has no list {t!r}')
        if counts[t][0] != counts[t][1]:
            raise EvidenceError(f'mutation_evidence: {path}, list {t!r} says {counts[t][0]} '
                                f'and holds {counts[t][1]} tests')
    return tests


def read_summary(path):
    with open(path, encoding='utf-8') as f:
        return json.load(f)


def mutation_dir(aud):
    return os.path.join(aud, 'mutation')


def read_evidence(aud):
    """The one reader of AUD/mutation/: report, tests, summary and timed-out blocks."""
    d = mutation_dir(aud)
    report = read_report(os.path.join(d, 'report.txt'))
    ev = {'run': report['run'], 'report': report,
          'tests': read_tests(os.path.join(d, 'tests.txt')),
          'summary': read_summary(os.path.join(d, 'summary.json')),
          'timed_out': read_timed_out(os.path.join(d, TIMED_OUT_FILE))}
    check_timed_out(report, [b['number'] for b in ev['timed_out']])
    return ev


def check_timed_out(report, numbers):
    """Stops unless the blocks are exactly the bugs report.txt lists as timed out, once each."""
    want = timed_out_numbers(report)
    stated = report['by_state'].get('timed out')
    if len(numbers) != len(want) or sorted(numbers) != sorted(want) or stated != len(want):
        missing = sorted(set(want) - set(numbers))
        extra = sorted(set(numbers) - set(want))
        raise EvidenceError(f'mutation_evidence: {len(numbers)} timed-out blocks, and report.txt states '
                            f'{stated} (its list holds {len(want)}); missing {missing[:20]}, '
                            f'not in its list {extra[:20]}')


def survivors_page(report):
    sv = survivors(report)
    by_file = {}
    for s in sv:
        by_file.setdefault(s['file'], []).append(s['text'])
    out = [f'# Survivors of mutation run {report["run"]}, by source file', '',
           SURVIVOR_LIMIT, '',
           LIMITS, '',
           f'A survivor is a bug some test reached and no test caught. These are the '
           f'{len(sv)} lines of report.txt\'s list "{SURVIVORS_TITLE}", copied word for word '
           f'and grouped by the source file each names, in {len(by_file)} files. The files are in '
           f'name order and the bugs in the report\'s order. This page holds no verdict and '
           f'ranks nothing. It is an input for the missing-coverage pass that comes after the '
           f'audit, and T8 judges none of it.', '']
    for f in sorted(by_file):
        out += [f'## {f} ({len(by_file[f])})', '', '```'] + by_file[f] + ['```', '']
    return '\n'.join(out), len(sv), {f: len(v) for f, v in by_file.items()}


def _write(path, text):
    # The platform's line ends, as report.py writes the run's files, so the
    # sizes the README states hold for a checkout on the machine that wrote them.
    with open(path, 'w', encoding='utf-8') as f:
        f.write(text)


def readme_text(report, source, sizes, timed_out, procs):
    silent = sum(1 for b in timed_out if not b['running_at_limit'])
    limit1 = next((ln for ln in report['limits'] if 'not proof the test is empty' in ln), None)
    if limit1 is None or SURVIVOR_LIMIT not in report['limits']:
        raise EvidenceError('mutation_evidence: report.txt\'s limits do not hold the sentences '
                            'the plan quotes')
    out = [f'# Mutation run {report["run"]}: the evidence T8 keeps', '',
           LIMITS, '',
           f'These files come from the run folder `{source}`. That folder sits in a worktree\'s '
           f'build folder and goes when the worktree does, so the text files T8 reads are kept here. '
           f'`exits/mutation_evidence.py` copied them and is their one reader. The plan is '
           f'`docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md`, step E1c.', '',
           '| File | Bytes | What it is |', '|---|---|---|']
    what = {'report.txt': 'copied whole: the run\'s limits, counts and lists of bugs by state',
            'tests.txt': 'copied whole: each test\'s caught and reaches counts, the tests that caught none first',
            'summary.json': 'copied whole: the run\'s counts, its crashes and its catches through shared state',
            TIMED_OUT_FILE: f'the {len(timed_out)} timed-out bugs\' blocks of the run\'s bugs.txt, byte for byte',
            SURVIVORS_FILE: 'written here: report.txt\'s survivors grouped by source file'}
    for name in WHOLE + (TIMED_OUT_FILE, SURVIVORS_FILE):
        out.append(f'| `{name}` | {sizes[name]} | {what[name]} |')
    out += ['', 'The rest of bugs.txt is not kept. It repeats what tests.txt lists per test. '
            'The bugs folder and map.json are too big to keep.', '',
            '## The four limits', '',
            '1. The tool plants its own kinds of bug, not the change an auditor named. So caught '
            f'nothing is not proof that a test is empty. The report\'s own words: "{limit1}"',
            f'2. The run went {procs}, so a slow test can be counted as caught on a '
            'time-out. A timed-out bug\'s red tests are listed but never credited as catches.',
            f'3. Each verdict comes from one run of its bug. {silent} of the {len(timed_out)} '
            'timed-out blocks name no test running at the time limit.',
            '4. The run held only the C++ tests in `hydra_tests`. A Python test, a `hydra_uitest` '
            'test or a helper function has no evidence here at all. Having no evidence is not the '
            'same as catching nothing.', '']
    return '\n'.join(out)


README_SOURCE = re.compile(r'from the run folder `([^`]*)`')


def write_pages(aud, source=None):
    """Writes survivors-by-file.md and README.md from the copies in AUD/mutation/.

    source names the run folder in the README; without it, the name the README
    already holds is kept.
    """
    d = mutation_dir(aud)
    ev = read_evidence(aud)
    page, n_survivors, per_file = survivors_page(ev['report'])
    stated = ev['report']['by_state'].get('survived')
    if stated != n_survivors or sum(per_file.values()) != n_survivors:
        raise EvidenceError(f'mutation_evidence: {n_survivors} survivors listed, report.txt states '
                            f'{stated}, per-file counts add to {sum(per_file.values())}')
    _write(os.path.join(d, SURVIVORS_FILE), page)
    if source is None:
        old = os.path.join(d, README_FILE)
        m = None
        if os.path.exists(old):
            with open(old, encoding='utf-8') as f:
                m = README_SOURCE.search(f.read())
        if not m:
            raise EvidenceError(f'mutation_evidence: {old} names no run folder; run with --copy-run')
        source = m.group(1)
    procs = ev['summary'].get('processes')
    procs = f'{procs} bugs at once (summary.json, "processes")' if procs is not None else 'several bugs at once'
    sizes = {n: os.path.getsize(os.path.join(d, n)) for n in WHOLE + (TIMED_OUT_FILE, SURVIVORS_FILE)}
    _write(os.path.join(d, README_FILE), readme_text(ev['report'], source, sizes, ev['timed_out'], procs))
    return ev, n_survivors, per_file, sizes


def _source_name(run_dir):
    """The run folder from the repo root when it sits in a worktree, else its full path."""
    p = os.path.abspath(run_dir).replace('\\', '/')
    i = p.find('/worktrees/')
    return p[i + 1:] if i >= 0 else p


def copy_run(run_dir, aud):
    """Copies the run's evidence into AUD/mutation/ and writes the two pages."""
    d = mutation_dir(aud)
    report = read_report(os.path.join(run_dir, 'report.txt'))
    want = timed_out_numbers(report)
    blocks = split_bug_blocks(_text_lines(os.path.join(run_dir, 'bugs.txt')))
    in_bugs = [n for n, b in blocks.items() if parse_block(b)['state'].startswith(TIMED_OUT_STATE)]
    check_timed_out(report, in_bugs)
    os.makedirs(d, exist_ok=True)
    for name in WHOLE:
        shutil.copyfile(os.path.join(run_dir, name), os.path.join(d, name))
    with open(os.path.join(d, TIMED_OUT_FILE), 'wb') as f:
        for n in sorted(want):
            f.write(''.join(blocks[n]).encode('utf-8'))
    return write_pages(aud, _source_name(run_dir))


def main(argv=None):
    ap = argparse.ArgumentParser(description='Copy a mutation run\'s evidence into the T8 audit.')
    ap.add_argument('aud', nargs='?', default=os.path.dirname(HERE), help='the audit folder')
    ap.add_argument('--copy-run', default=None, help='the run folder to copy from')
    a = ap.parse_args(argv)
    try:
        if a.copy_run:
            ev, n, per_file, sizes = copy_run(a.copy_run, a.aud)
        else:
            ev, n, per_file, sizes = write_pages(a.aud)
    except EvidenceError as e:
        print(e, file=sys.stderr)
        return 1
    print(f'mutation_evidence: run {ev["run"]}: {len(ev["timed_out"])} timed-out blocks; '
          f'{n} survivors in {len(per_file)} files, per-file counts add to {sum(per_file.values())}')
    for name, size in sizes.items():
        print(f'  {name}: {size} bytes')
    return 0


if __name__ == '__main__':
    sys.exit(main())
