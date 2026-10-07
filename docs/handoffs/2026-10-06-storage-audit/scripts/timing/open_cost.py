"""Process wall times (median of N) for the 2.1.0 hydra_replay commands that
stand in for 'open a song' options. Every command reads the tiny scratch DB."""
import subprocess, time, statistics, json, os, sys
A = os.path.dirname(os.path.abspath(__file__))
R = os.path.join(A, 'ship', 'hydra_replay.exe')
DB = os.path.join(A, 'tiny.db')
N = 5
charts = {
    'blink-182 Discography': r'C:\Clone Hero\songs\synchotic\Sync Charts\Misc\HopH2O Discography Charts\blink-182 - Discography\notes.mid',
    'Rise Against Discography': r'C:\Clone Hero\songs\synchotic\Sync Charts\Misc\HopH2O Discography Charts\Rise Against - Discography (2024)\notes.mid',
    'Hail The Sun Discography': r'C:\Clone Hero\songs\synchotic\Sync Charts\Misc\HopH2O Discography Charts\Hail The Sun - Discography (2025)\notes.mid',
    'Nirvana Endless Nameless (.chart)': r"C:\Clone Hero\songs\synchotic\Sync Charts\Drummer's Monthly Drive\highfine\the epic nirvana folder\3.- Full album charts\8.- Endless, Nameless Setlist (discog)\notes.chart",
}
def timed(args, expect=(0,)):
    ts = []
    for _ in range(N):
        t = time.perf_counter()
        r = subprocess.run(args, capture_output=True)
        ts.append((time.perf_counter() - t) * 1e3)
        if r.returncode not in expect:
            raise SystemExit(f'{args[1]} exit {r.returncode}: {r.stderr[:300]}')
    return statistics.median(ts)
out = {}
nul = os.path.join(A, 'scratch_out.json')
for name, chart in charts.items():
    dump = os.path.join(A, 'dump_' + name.split()[0] + '.json')
    subprocess.run([R, 'dump', '--chart', chart, '--db', DB, '--out', dump, '--no-analyze'], check=True)
    d = json.load(open(dump, encoding='utf-8'))
    ticks = ','.join(str(a['act_tick']) for a in d['paths'][0]['activations'])
    row = {}
    # Stored record: hash + snapshot + get_record (read+decode) + JSON out.
    row['dump_stored'] = timed([R, 'dump', '--chart', chart, '--db', DB, '--out', nul, '--no-analyze'])
    # Same, but a key with no row: hash + snapshot + an empty lookup.
    row['dump_nokey'] = timed([R, 'dump', '--chart', chart, '--db', DB, '--cap', '5', '--no-analyze'], expect=(1,))
    # Fresh analysis instead of the stored row (parse + analyze_chart + JSON).
    row['dump_fresh'] = timed([R, 'dump', '--chart', chart, '--db', DB, '--cap', '5', '--out', nul])
    # Parse + replay of no Star Power (the floor of score).
    row['score_nosp'] = timed([R, 'score', '--chart', chart, '--out', nul])
    # Parse + replay of the stored best path.
    row['score_path'] = timed([R, 'score', '--chart', chart, '--path', dump, '--out', nul])
    # Parse + full graph + search pinned to the stored path's activations.
    row['target'] = timed([R, 'target', '--chart', chart, '--ticks', ticks, '--out', nul])
    out[name] = row
    print(name, {k: round(v, 1) for k, v in row.items()}, flush=True)
json.dump(out, open(os.path.join(A, 'open_cost.json'), 'w'), indent=1)
