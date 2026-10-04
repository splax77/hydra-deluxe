# Run blink_probe.py on every giant chart, a few at a time, one fresh process
# per chart. Giants: song.ini song_length >= --min-minutes, plus charts with no
# usable song_length whose notes file is >= 0.5 MB. In this library that second
# group is exactly the 54 .sng files (rank.py doesn't read their length, and
# their size includes audio), most of them full albums. Longest first. Resumable: a chart whose output exists is skipped.
#   python run_giants.py <ranked.json> <out_dir> --workers 4 -- <probe args...>
# Run from inside the public clone (blink_probe.py imports hydra.*).
import argparse, json, os, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

ap = argparse.ArgumentParser()
ap.add_argument('ranked')
ap.add_argument('out_dir')
ap.add_argument('--workers', type=int, default=4)
ap.add_argument('--min-minutes', type=float, default=30)
ap.add_argument('probe_args', nargs=argparse.REMAINDER)
a = ap.parse_args()
probe_args = [x for x in a.probe_args if x != '--']

rows = json.load(open(a.ranked, encoding='utf-8'))
giants = [r for r in rows if (r['len_ms'] or 0) >= a.min_minutes * 60000 or
          (not r['len_ms'] and r['size'] >= 500_000)]
giants.sort(key=lambda r: -(r['len_ms'] or 0))
os.makedirs(a.out_dir, exist_ok=True)
json.dump(giants, open(os.path.join(a.out_dir, 'giants.json'), 'w', encoding='utf-8'), indent=1)
print(f'{len(giants)} giants', flush=True)

def one(r):
    out = os.path.join(a.out_dir, f"r{r['raw']}.json")
    if os.path.exists(out):
        return
    t = time.perf_counter()
    p = subprocess.run([sys.executable, 'blink_probe.py', r['path'], out] + probe_args,
                       stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
    err = p.stderr.strip().splitlines()[-1][:150] if p.returncode not in (0, 3) and p.stderr else ''
    print(f"exit={p.returncode} {time.perf_counter() - t:7.1f}s  {r['artist'][:25]} - {r['title'][:45]} {err}",
          flush=True)
    if not os.path.exists(out):   # crashed before its first dump
        json.dump({'chart': r['path'], 'finished': False, 'stopped_because': 'crash: ' + err},
                  open(out, 'w'))

with ThreadPoolExecutor(a.workers) as ex:
    list(ex.map(one, giants))
print('done', flush=True)
