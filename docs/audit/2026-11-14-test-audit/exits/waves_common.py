"""What make_waves.py and check_waves.py share: which chunks are open, which have a file left, and
which repo a chunk runs in.

A chunk is done when its claude/tf-<chunk> branch is merged into main (C069's branch is
claude/tf-T08a). An open chunk has a file left when any file it names exists: a hook file under the
user's home, a Hydra file in the main checkout. A mop-up chunk names an audit record, not a test
file, so it always has one. A chunk runs in the hooks repo when every file it names is under
~/.claude/, and in hydra otherwise.
"""
import csv, os, subprocess

FIRST_WAVE, LAST_WAVE, MAX_PER_WAVE = 13, 18, 21


def read_tsv(path):
    return list(csv.DictReader(open(path, encoding='utf-8'), delimiter='\t'))


def is_done(repo, cid):
    b = 'claude/tf-' + ('T08a' if cid == 'C069' else cid)
    if subprocess.run(['git', '-C', repo, 'rev-parse', '--verify', '-q', b], capture_output=True).returncode:
        return False
    return subprocess.run(['git', '-C', repo, 'merge-base', '--is-ancestor', b, 'main']).returncode == 0


def path_of(repo, f):
    return os.path.join(os.path.expanduser('~'), f[2:]) if f.startswith('~/') else os.path.join(repo, f)


def has_file(repo, c):
    return c['kind'] == 'mop-up' or any(os.path.exists(path_of(repo, f)) for f in c['files'].split())


def repo_of(c):
    return 'hooks' if all(f.startswith('~/.claude/') for f in c['files'].split()) else 'hydra'


def part_no(c):
    return tuple(map(int, c['part'].split('/')))


def multi_part(chunks):
    """Each file split into parts across chunks, mapped to those chunks."""
    by = {}
    for c in chunks:
        if c['kind'] in ('file-part', 'mop-up') and part_no(c)[1] > 1:
            by.setdefault(c['files'], []).append(c)
    return by
