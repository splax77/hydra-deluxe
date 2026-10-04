# Estimate each giant's untouched public time from a capped untouched run
# (blink_probe.py --trace --search-timeout S) plus its fixed run.
#   python estimate.py <fixed_dir> <untouched_dir> [--calibrate]
# For a run that finished, the time is exact. For one that stopped, the
# projection assumes the rest of the chart goes at the speed the run had at the
# end (progress gained per second over the last quarter of its trace). The
# untouched code slows down as a chart goes on, so this is a lower bound.
# --calibrate checks that rule on runs that did finish: cut their trace at
# 25% / 50% of their real search time, project, and compare with the real time.
import glob, json, os, sys

fixed_dir, un_dir = sys.argv[1], sys.argv[2]
giants = {f"r{r['raw']}": r for r in json.load(open(os.path.join(fixed_dir, 'giants.json'), encoding='utf-8'))}

def load(d, k):
    p = os.path.join(d, k + '.json')
    s = open(p).read() if os.path.exists(p) else ''
    return json.loads(s) if s.strip() else None   # empty: killed mid-write

def project(trace, upto=None):
    """Search seconds to reach progress 1.0, from trace points with t <= upto."""
    pts = [x for x in trace if upto is None or x[0] <= upto]
    if len(pts) < 2:
        return None, None
    t_last, p_last = pts[-1][0], pts[-1][1]
    tail = [x for x in pts if x[0] >= 0.75 * t_last] or pts[-2:]
    if len(tail) < 2:
        tail = pts[-2:]
    dt, dp = tail[-1][0] - tail[0][0], tail[-1][1] - tail[0][1]
    if dp <= 0:
        return None, None
    rate = dp / dt
    t_est = t_last + max(0.0, 1.0 - p_last) / rate
    dm = (tail[-1][2] - tail[0][2]) / dp          # MB per unit progress, recent
    mem_est = tail[-1][2] + max(0.0, dm) * max(0.0, 1.0 - p_last)
    return t_est, mem_est

if '--calibrate' in sys.argv:
    print('chart                                         real_s  cut25%_proj  cut50%_proj')
    for k in giants:
        u = load(un_dir, k)
        if not u or not u.get('finished') or u['search_s'] < 10 or len(u.get('trace', [])) < 4:
            continue
        real = u['search_s']
        p25, _ = project(u['trace'], 0.25 * real)
        p50, _ = project(u['trace'], 0.50 * real)
        f = lambda p: f"{p:8.1f} ({real / p:4.1f}x)" if p else '       n/a     '
        r = giants[k]
        print(f"{(r['artist'] + ' - ' + r['title'])[:45]:45} {real:7.1f}  {f(p25)}  {f(p50)}")
    sys.exit()

rows, tot_exact, tot_lb, nf = [], 0.0, 0.0, 0
for k, r in giants.items():
    fx, u = load(fixed_dir, k), load(un_dir, k)
    if fx and fx.get('finished'):
        pre, fsearch = fx['parse_s'] + fx['graph_s'], fx['search_s']   # identical code in both versions
    elif u and 'graph_s' in u:
        # No finished fixed run (Endless Setlist II/III: the final variant
        # step needs > 8 GB). The untouched run's own read+graph is the same code.
        pre, fsearch = u['parse_s'] + u['graph_s'], 0.0
        print('no finished fixed run for', giants[k]['title'], '- read+graph from the untouched run')
    else:
        print('no usable run for', k); continue
    if u and u.get('finished'):
        kind, search, mem = 'exact', u['search_s'], u['peak_mb']
    elif u and u.get('trace'):
        search, mem = project(u['trace'])
        kind = 'at least' if search else 'unknown'
        search = search or u['trace'][-1][0]
        nf += 1
    else:
        kind, search, mem = 'no run', None, None
    total = pre + search if search is not None else None
    rows.append((total or 0, kind, pre, fsearch, search, mem, u.get('progress') if u else None,
                 u.get('stopped_because') if u else None, r))
    if kind == 'exact':
        tot_exact += total
    elif total:
        tot_lb += total

rows.sort(key=lambda x: -x[0])
print(f"{'untouched total':>16} {'kind':8} {'read+graph':>10} {'fixed search':>12} {'untouched search':>16} "
      f"{'mem MB':>8} {'reached':>7}  chart")
for total, kind, pre, fsr, search, mem, prog, why, r in rows:
    print(f"{total:16.0f} {kind:8} {pre:10.1f} {fsr:12.1f} {search or 0:16.0f} {mem or 0:8.0f} "
          f"{prog or 0:7.2f}  {(r['artist'] + ' - ' + r['title'])[:50]} {why or ''}")
print(f"\nfinished untouched: {len(rows) - nf}, sum {tot_exact:.0f}s;  projected (lower bounds): {nf}, sum {tot_lb:.0f}s")
print(f"giants total, untouched, at least {(tot_exact + tot_lb) / 3600:.2f} h")
