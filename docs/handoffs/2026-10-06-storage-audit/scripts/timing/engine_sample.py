"""Per-chart engine timing: each chart alone in a scratch folder, hydra_bench --engine.

Charts are copied (song.ini + notes file) or, for .sng/.srb archives, hard-linked
into scratch, so discovery sees exactly one chart and the user's files are untouched.
"""
import os, re, shutil, subprocess, sys, random, json, time

A = os.path.dirname(os.path.abspath(__file__))
BENCH = os.path.join(A, 'bin', 'hydra_bench.exe')
work = os.path.join(A, 'one')
reps = 5
lists = sys.argv[1:-2]
nrandom = int(sys.argv[-2])
out_json = sys.argv[-1]

charts = []
for l in lists:
    for line in open(l, encoding='utf-8'):
        p = line.strip()
        if p and p not in charts:
            charts.append(p)
ok_paths = [line.split('\t')[0] for line in open(os.path.join(A, 'parse.tsv'), encoding='utf-8')
            if len(line.rstrip('\n').split('\t')) == 3]
random.seed(20261006)
for p in random.sample(ok_paths, nrandom):
    if p not in charts:
        charts.append(p)

rx = re.compile(r'charts (\d+) failed (\d+) \| parse ([\d.]+)s \| graph ([\d.]+)s \| analyze x(\d+) ([\d.]+)s \| prepare ([\d.]+)s')
results = []
for i, p in enumerate(charts):
    d = os.path.join(work, f'c{i}')
    if os.path.exists(d):
        shutil.rmtree(d)
    os.makedirs(d)
    src = os.path.dirname(p)
    ext = os.path.splitext(p)[1].lower()
    try:
        if ext in ('.sng', '.srb'):
            os.link(p, os.path.join(d, os.path.basename(p)))
        else:
            shutil.copy2(p, d)
            ini = os.path.join(src, 'song.ini')
            if os.path.exists(ini):
                shutil.copy2(ini, d)
    except OSError as e:
        print('skip', p, e)
        continue
    t = time.perf_counter()
    r = subprocess.run([BENCH, '--engine', d, '--reps', str(reps)], capture_output=True, text=True,
                       encoding='utf-8', errors='replace')
    wall = time.perf_counter() - t
    m = rx.search(r.stdout)
    shutil.rmtree(d, ignore_errors=True)
    if not m or m.group(1) != '1':
        print('no single-chart result', p, r.stdout.strip()[:200], r.stderr.strip()[:200])
        continue
    rec = dict(path=p, parse=float(m.group(3)) * 1e3, graph=float(m.group(4)) * 1e3,
               analyze=float(m.group(6)) * 1e3 / reps, prepare=float(m.group(7)) * 1e3, wall=wall * 1e3)
    results.append(rec)
    if i % 25 == 0:
        print(i, len(charts), rec, flush=True)
json.dump(results, open(out_json, 'w', encoding='utf-8'), indent=0)
print('done', len(results))
