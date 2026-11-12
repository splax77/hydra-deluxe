"""Count the library charts the all-toms tail moves, for the plan's step MS5
(`docs/superpowers/plans/2026-11-12-mid-no-pro-toms.md` (removed; `git show e9fecb6f:docs/superpowers/plans/2026-11-12-mid-no-pro-toms.md`), question 4).

Usage:  py -I scan_library.py <songs folder> <output file>

Only the main session runs this, and only once the user has said yes to
question 4 in chat. It is a whole-library read. It reads every song folder's
notes file and song.ini, every .sng and every .srb under <songs folder>, and
writes only <output file>. It runs no analysis and changes nothing.

Both arguments are required: with no folder it refuses to run, so it never
falls back to a default library. It refuses an output file that exists.

The plan caps the run at 30 minutes (MS5). At that cap the script stops by
itself and writes what it has, marked unfinished, with the last path it read.

The rule it applies is tail_rule.py beside it. A loose folder's notes file is
notes.mid when there is one, else notes.chart (pick_notes_file in
src/parse/chart_files.cpp).
"""
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import tail_rule  # noqa: E402

CAP_SECONDS = 30 * 60  # the plan's MS5 cap
PROGRESS_EVERY = 2000  # charts between progress lines on stdout


def charts_under(root):
    """Each chart as (path, kind): a loose folder's notes file, or a container."""
    for dirpath, _, files in os.walk(root):
        by_low = {f.lower(): f for f in files}
        if 'notes.mid' in by_low:
            yield os.path.join(dirpath, by_low['notes.mid']), 'mid'
        elif 'notes.chart' in by_low:
            yield os.path.join(dirpath, by_low['notes.chart']), 'chart'
        for f in files:
            low = f.lower()
            if low.endswith('.sng'):
                yield os.path.join(dirpath, f), 'sng'
            elif low.endswith('.srb'):
                yield os.path.join(dirpath, f), 'srb'


def folder_keys(folder):
    for name in os.listdir(folder):
        if name.lower() == 'song.ini':
            with open(os.path.join(folder, name), 'rb') as f:
                return tail_rule.read_song_ini_keys(f.read())
    return {}


def read_chart(path, kind):
    """(format, facts, keys) of one chart; facts is None with no drum track."""
    with open(path, 'rb') as f:
        raw = f.read()
    if kind == 'mid':
        return 'mid', tail_rule.mid_facts(raw), folder_keys(os.path.dirname(path))
    if kind == 'chart':
        return ('chart', tail_rule.chart_facts(tail_rule.decode_text(raw)),
                folder_keys(os.path.dirname(path)))
    if kind == 'sng':
        pairs, inner, data = tail_rule.read_sng(raw)
        if inner is None:
            return 'sng', None, {}
        keys = tail_rule.sng_keys(pairs)
        if inner.lower() == 'notes.mid':
            return 'sng/mid', tail_rule.mid_facts(data), keys
        return 'sng/chart', tail_rule.chart_facts(tail_rule.decode_text(data)), keys
    _, data, is_mid = tail_rule.read_srb(raw)
    if is_mid:
        return 'srb/mid', tail_rule.mid_facts(data), {}
    return 'srb/chart', tail_rule.chart_facts(tail_rule.decode_text(data)), {}


def main(argv):
    if len(argv) != 3:
        sys.exit('refusing: give the songs folder and the output file '
                 '(scan_library.py <songs folder> <output file>); there is no default folder')
    root, out_path = argv[1], argv[2]
    if not os.path.isdir(root):
        sys.exit(f'refusing: {root} is not a folder')
    if os.path.exists(out_path):
        sys.exit(f'refusing: {out_path} exists')

    start = time.monotonic()
    counts = {}
    moving, numbered, unread = [], [], []
    seen, last, finished = 0, '', True
    for path, kind in charts_under(root):
        if time.monotonic() - start > CAP_SECONDS:
            finished = False
            break
        seen += 1
        last = path
        if seen % PROGRESS_EVERY == 0:
            print(f'{seen} charts read, {len(moving)} move, {time.monotonic() - start:.0f} s',
                  flush=True)
        try:
            fmt, facts, keys = read_chart(path, kind)
        except Exception as e:  # a damaged file is counted, not judged
            unread.append((path, type(e).__name__))
            continue
        if facts is None:
            counts[(fmt, 'no drum track')] = counts.get((fmt, 'no drum track'), 0) + 1
            continue
        v = tail_rule.judge(fmt.split('/')[-1], facts, keys, in_sng=fmt.startswith('sng'))
        counts[(fmt, 'drum chart')] = counts.get((fmt, 'drum chart'), 0) + 1
        if v['new'][1]:
            counts[(fmt, 'reaches the tail')] = counts.get((fmt, 'reaches the tail'), 0) + 1
        if v['moves']:
            moving.append((path, fmt, facts, v, keys))
        if v['number_keys']:
            numbered.append((path, fmt, v, keys))

    elapsed = time.monotonic() - start
    lines = [
        f'{len(moving)} charts move and {len(numbered)} write a key as a number, '
        f'of {seen} charts read in {elapsed:.0f} s'
        + ('' if finished else f' (UNFINISHED: stopped at the {CAP_SECONDS // 60}-minute cap; '
                               f'last path read: {last})'),
        '',
        f'Folder: {root}',
        'Made by docs/audit/2026-11-12-mid-no-pro/scan_library.py; the rule is tail_rule.py beside it.',
        'A move shows only with Pro Drums on.',
        '',
        '== Counts by format',
    ]
    for (fmt, what), n in sorted(counts.items()):
        lines.append(f'{fmt} | {what} | {n}')
    lines += ['', f'== Charts that move ({len(moving)})',
              'Each row: why | today -> after | yellow, blue and fourth-pad notes (all / Expert) | '
              'pro_drums | five_lane_drums | format | path']
    for path, fmt, facts, v, keys in moving:
        def place(p):
            return ('five-lane' if p[0] else 'four-lane') + (' + tail' if p[1] else '')
        lines.append(f'{",".join(v["reasons"])} | {place(v["old"])} -> {place(v["new"])} | '
                     f'{facts["cymbal_pads"]}/{facts["expert_cymbal_pads"]} | '
                     f'{keys.get("pro_drums", "(absent)")!r} | '
                     f'{keys.get("five_lane_drums", "(absent)")!r} | {fmt} | {path}')
    lines += ['', f'== Charts that write a key as a number ({len(numbered)})',
              'Each row: keys | moves? | pro_drums | five_lane_drums | format | path']
    for path, fmt, v, keys in numbered:
        lines.append(f'{",".join(v["number_keys"])} | {"moves" if v["moves"] else "same"} | '
                     f'{keys.get("pro_drums", "(absent)")!r} | '
                     f'{keys.get("five_lane_drums", "(absent)")!r} | {fmt} | {path}')
    lines += ['', f'== Files that did not read here ({len(unread)})']
    lines += [f'{err} | {path}' for path, err in unread]

    with open(out_path, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(lines) + '\n')
    print(lines[0])
    print('wrote', out_path)


if __name__ == '__main__':
    main(sys.argv)
