"""List the checked-in charts and the in-test charts whose pinned values the
all-toms tail may move (plan
`docs/superpowers/plans/2026-11-12-mid-no-pro-toms.md` (removed; `git show e9fecb6f:docs/superpowers/plans/2026-11-12-mid-no-pro-toms.md`),
task T4, for MS2 and T6).

Usage:  py -I scout_corpus.py <repo root> [<output file>]

It reads only <repo root>/testdata and <repo root>/tests, and writes only the
output file (default: tail_charts_and_tests.txt beside this script).

Part 1 reads every .mid, .chart, .sng and .srb under testdata/. A loose chart
takes its keys from the song.ini in its own folder, as chart_drum_keys_beside
does. tail_rule.judge says whether it moves.

Part 2 reads the C++ under tests/. A test "builds a .mid in code" when its
body, or a helper function of its file that it calls, calls one of the file
writers in tests/midi_util.h (SMF_WRITER_RE). Each
such test is listed with what it mentions: a tom marker pitch, a fifth-pad
pitch, or one of the two song.ini keys, by literal or by a named constant of
its file. A test that mentions none of them is a candidate whose value may
move. This is a text scan, so it over-includes: a test that reads with Pro
Drums off, or that pins no value, is still listed. Tests whose text writes
either key as a number are listed apart, because the key reader changes for
them whatever the format.
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import tail_rule  # noqa: E402


# ---- Part 1: checked-in charts ------------------------------------------

def folder_keys(folder):
    for name in os.listdir(folder):
        if name.lower() == 'song.ini':
            with open(os.path.join(folder, name), 'rb') as f:
                return tail_rule.read_song_ini_keys(f.read())
    return {}


def chart_rows(testdata):
    rows = []
    for dirpath, _, files in os.walk(testdata):
        for name in sorted(files):
            path = os.path.join(dirpath, name)
            low = name.lower()
            try:
                if low.endswith('.mid'):
                    fmt, keys = 'mid', folder_keys(dirpath)
                    with open(path, 'rb') as f:
                        facts = tail_rule.mid_facts(f.read())
                elif low.endswith('.chart'):
                    fmt, keys = 'chart', folder_keys(dirpath)
                    with open(path, 'rb') as f:
                        facts = tail_rule.chart_facts(tail_rule.decode_text(f.read()))
                elif low.endswith('.sng'):
                    with open(path, 'rb') as f:
                        pairs, inner, data = tail_rule.read_sng(f.read())
                    if inner is None:
                        rows.append((path, 'sng', None, None, 'no notes file'))
                        continue
                    keys = tail_rule.sng_keys(pairs)
                    fmt = 'mid' if inner.lower() == 'notes.mid' else 'chart'
                    facts = (tail_rule.mid_facts(data) if fmt == 'mid'
                             else tail_rule.chart_facts(tail_rule.decode_text(data)))
                    fmt = 'sng/' + fmt
                elif low.endswith('.srb'):
                    with open(path, 'rb') as f:
                        _, data, is_mid = tail_rule.read_srb(f.read())
                    keys = {}
                    fmt = 'mid' if is_mid else 'chart'
                    facts = (tail_rule.mid_facts(data) if is_mid
                             else tail_rule.chart_facts(tail_rule.decode_text(data)))
                    fmt = 'srb/' + fmt
                else:
                    continue
            except Exception as e:  # a fuzz file that does not read: listed, not judged
                rows.append((path, low.rsplit('.', 1)[-1], None, None,
                             f'unreadable here ({type(e).__name__})'))
                continue
            if facts is None:
                rows.append((path, fmt, None, None, 'no drum track'))
                continue
            verdict = tail_rule.judge(fmt.split('/')[-1], facts, keys, in_sng=fmt.startswith('sng'))
            rows.append((path, fmt, facts, verdict, keys))
    return rows


# ---- Part 2: in-test builders -------------------------------------------

def blank_code(text):
    """`text` with comments and the insides of string and char literals turned
    to spaces, the same length, so braces can be counted on it."""
    out, i, n = list(text), 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith('//', i):
            j = text.find('\n', i)
            j = n if j < 0 else j
            out[i:j] = ' ' * (j - i)
            i = j
        elif text.startswith('/*', i):
            j = text.find('*/', i + 2)
            j = n if j < 0 else j + 2
            out[i:j] = ' ' * (j - i)
            i = j
        elif text.startswith('R"', i):
            m = re.match(r'R"([^(]*)\(', text[i:])
            if m:
                end = text.find(')' + m.group(1) + '"', i)
                end = n if end < 0 else end + len(m.group(1)) + 2
                out[i + 1:end] = ' ' * (end - i - 1)
                i = end
            else:
                i += 1
        elif c in '"\'':
            # A digit separator (1'000) is not a char literal.
            if c == "'" and i > 0 and text[i - 1].isalnum():
                i += 1
                continue
            j = i + 1
            while j < n and text[j] != c and text[j] != '\n':
                j += 2 if text[j] == '\\' else 1
            out[i + 1:j] = ' ' * (j - i - 1)
            i = j + 1
        else:
            i += 1
    return ''.join(out)


def block_end(clean, open_brace):
    depth = 0
    for i in range(open_brace, len(clean)):
        if clean[i] == '{':
            depth += 1
        elif clean[i] == '}':
            depth -= 1
            if depth == 0:
                return i + 1
    return len(clean)


TEST_RE = re.compile(r'^\s*TEST_CASE\s*\(\s*"((?:[^"\\]|\\.)*)"', re.M)
FUNC_RE = re.compile(r'^[A-Za-z_][\w:<>,\s\*&]*?\b([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?\{',
                     re.M)
CONST_RE = re.compile(r'\b(k\w+)\s*=\s*(\d+)\b')
CALL_RE = re.compile(r'\b([A-Za-z_]\w*)\s*\(')
# A call of one of tests/midi_util.h's file writers, with or without its
# testmidi:: prefix (a file may say `using namespace testmidi`).
SMF_WRITER_RE = re.compile(r'\b(?:testmidi::)?smf(?:_drums|_tracks)?\s*\(')
# The helpers of tests/corpus_util.h that walk the whole corpus, so a test
# calling one reads every chart, the ones that move too.
CORPUS_WIDE_RE = re.compile(r'\b(?:chart_paths|charts_with_paths|analyzed_with_paths)\s*\(')
KEY_NUMBER_RE = re.compile(r'(pro_drums|five_lane_drums)\s*=\s*[+-]?\d')


def scan_file(path):
    with open(path, encoding='utf-8', errors='replace') as f:
        raw = f.read()
    clean = blank_code(raw)
    tests = []
    for m in TEST_RE.finditer(raw):
        brace = clean.find('{', m.end())
        if brace < 0:
            continue
        tests.append((m.group(1), raw[m.start():block_end(clean, brace)]))
    funcs = {}
    for m in FUNC_RE.finditer(clean):
        name = m.group(1)
        if name in ('TEST_CASE', 'if', 'for', 'while', 'switch', 'return'):
            continue
        brace = m.end() - 1
        funcs.setdefault(name, raw[m.start():block_end(clean, brace)])
    consts = {k: int(v) for k, v in CONST_RE.findall(clean)}
    return tests, funcs, consts


def closure(body, funcs):
    """The body plus every helper it reaches, as one text."""
    seen, todo, parts = set(), [body], [body]
    while todo:
        text = todo.pop()
        for name in CALL_RE.findall(text):
            if name in funcs and name not in seen:
                seen.add(name)
                parts.append(funcs[name])
                todo.append(funcs[name])
    return '\n'.join(parts)


def mentions(text, consts):
    nums = {int(x) for x in re.findall(r'(?<![\w.])(\d{2,3})(?![\w.])', text)}
    nums |= {consts[name] for name in re.findall(r'\b(k\w+)\b', text) if name in consts}
    found = []
    if nums & tail_rule.TOM_MARKER_PITCHES:
        found.append('tom marker')
    if nums & tail_rule.FIFTH_PAD_PITCHES:
        found.append('fifth pad')
    for key in ('pro_drums', 'five_lane_drums'):
        if key in text:
            found.append(key)
    return found


def test_rows(tests_dir, moving_names):
    """`moving_names`: the file names of the checked-in charts that move, so a
    test that names one is listed."""
    files = []
    for dirpath, _, names in os.walk(tests_dir):
        for name in sorted(names):
            if name.endswith(('.cpp', '.h')):
                files.append(os.path.join(dirpath, name))
    builders, number_keys, corpus_wide, named = [], [], [], []
    scanned =[(path, scan_file(path)) for path in files]
    # A helper may live in a shared header; a name found in the test's own
    # file wins over one found elsewhere.
    shared = {}
    for _, (_, funcs, _) in scanned:
        for name, body in funcs.items():
            shared.setdefault(name, body)
    for path, (tests, funcs, consts) in scanned:
        rel = os.path.relpath(path, os.path.dirname(tests_dir)).replace('\\', '/')
        for name, body in tests:
            text = closure(body, {**shared, **funcs})
            # Only the test's own file here: every corpus helper in the shared
            # headers walks the corpus, so following them would list every test
            # that opens one chart by name.
            if CORPUS_WIDE_RE.search(closure(body, funcs)):
                corpus_wide.append((rel, name))
            hit = [n for n in moving_names if n in text]
            if hit:
                named.append((rel, name, hit))
            if KEY_NUMBER_RE.search(text):
                number_keys.append((rel, name, sorted(set(KEY_NUMBER_RE.findall(text)))))
            if not SMF_WRITER_RE.search(text):
                continue
            builders.append((rel, name, mentions(text, consts)))
    return builders, number_keys, corpus_wide, named


# ---- Output ---------------------------------------------------------------

def place(p):
    return ('five-lane' if p[0] else 'four-lane') + (' + tail' if p[1] else '')


def main(argv):
    if len(argv) < 2:
        sys.exit('usage: scout_corpus.py <repo root> [<output file>]')
    root = os.path.abspath(argv[1])
    out_path = argv[2] if len(argv) > 2 else os.path.join(
        os.path.dirname(os.path.abspath(__file__)), 'tail_charts_and_tests.txt')
    rows = chart_rows(os.path.join(root, 'testdata'))
    judged = [r for r in rows if r[3] is not None]
    moving = [r for r in judged if r[3]['moves']]
    builders, number_keys, corpus_wide, named = test_rows(
        os.path.join(root, 'tests'), sorted({os.path.basename(r[0]) for r in moving}))

    tail_no_pads = [r for r in judged if r[3]['new'][1] and not r[2]['cymbal_pads']]
    chart_tail = [r for r in judged if r[1].endswith('chart') and not r[2]['fifth_pad_note']
                  and not r[2]['pro_mark'] and not r[3]['new'][0]]
    candidates = [b for b in builders if not b[2]]

    def rel(p):
        return os.path.relpath(p, root).replace('\\', '/')

    lines = [
        f'{len(moving)} checked-in charts move and {len(candidates)} in-test .mid builders '
        f'mention no tom marker, fifth pad or key '
        f'(of {len(judged)} drum charts read under testdata/ and {len(builders)} tests '
        f'that build a .mid in code)',
        '',
        'Made by docs/audit/2026-11-12-mid-no-pro/scout_corpus.py. The rule it applies is',
        'tail_rule.py beside it; the plan is `docs/superpowers/plans/2026-11-12-mid-no-pro-toms.md` (removed; `git show e9fecb6f:docs/superpowers/plans/2026-11-12-mid-no-pro-toms.md`).',
        'A move shows only with Pro Drums on: with it off no pad is a cymbal.',
        '',
        '== Checked-in charts that move',
        'Each row: why it moves | placement today -> after | yellow, blue and fourth-pad notes',
        '(all difficulties / Expert) | keys written as a number | path.',
    ]
    for path, fmt, facts, v, keys in sorted(moving, key=lambda r: rel(r[0])):
        lines.append(f'{",".join(v["reasons"])} | {place(v["old"])} -> {place(v["new"])} | '
                     f'{facts["cymbal_pads"]}/{facts["expert_cymbal_pads"]} | '
                     f'{",".join(v["number_keys"]) or "-"} | {fmt} | {rel(path)}')
    lines += ['', f'== .mid charts that reach the tail but have no yellow, blue or fourth-pad '
                  f'note, so nothing moves ({len(tail_no_pads)})']
    lines += [f'{fmt} | {rel(p)}' for p, fmt, *_ in sorted(tail_no_pads, key=lambda r: rel(r[0]))]
    lines += ['', f'== .chart files with no N 5, no N 66 to 68 and no five_lane_drums '
                  f'({len(chart_tail)}). Hydra already reads these as all toms, so nothing moves.']
    lines += [f'{fmt} | {rel(p)}' for p, fmt, *_ in sorted(chart_tail, key=lambda r: rel(r[0]))]
    skipped = [r for r in rows if r[3] is None]
    lines += ['', f'== Files not judged ({len(skipped)})']
    lines += [f'{r[4]} | {r[1]} | {rel(r[0])}' for r in sorted(skipped, key=lambda r: rel(r[0]))]

    lines += ['', f'== Tests that name a checked-in chart that moves ({len(named)})',
              'Each row: chart | test file | test name.']
    lines += [f'{",".join(h)} | {f} | {n}' for f, n, h in named]
    lines += ['', f'== Tests that walk the whole corpus ({len(corpus_wide)}). They read the '
                  'charts that move without naming them. A test that reaches the corpus '
                  'through another helper is not caught here.',
              'Each row: test file | test name.']
    lines += [f'{f} | {n}' for f, n in corpus_wide]

    lines += ['', f'== In-test .mid builders that mention no tom marker, fifth pad or key '
                  f'({len(candidates)}). Their pinned values may move.',
              'Each row: test file | test name.']
    lines += [f'{f} | {n}' for f, n, _ in candidates]
    others = [b for b in builders if b[2]]
    lines += ['', f'== In-test .mid builders that mention one of them ({len(others)}). '
                  'A mention is not proof the test\'s chart carries it; T6 reads the test.',
              'Each row: what it mentions | test file | test name.']
    lines += [f'{",".join(m)} | {f} | {n}' for f, n, m in others]
    lines += ['', f'== Tests whose text writes a key as a number ({len(number_keys)}). The key '
                  'reader reads these differently after the plan.',
              'Each row: keys | test file | test name.']
    lines += [f'{",".join(k)} | {f} | {n}' for f, n, k in number_keys]

    with open(out_path, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(lines) + '\n')
    print(lines[0])
    print('wrote', out_path)


if __name__ == '__main__':
    main(sys.argv)
