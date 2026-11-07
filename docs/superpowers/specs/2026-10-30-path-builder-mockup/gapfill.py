"""Fill gaps in the mockup's path set.

A gap is a window prefix P and a fill f after P's last window where the player
has two or more Star Power phrases saved (so f can be activated), but no kept
path continues P with f. For each gap, every kept path whose activation ticks
start with P's ticks then f is re-priced by hydra_replay target with P's exact
windows and the candidate's later windows pinned. Realized results are real
engine prices of real paths and join the set. Repeats until no gap closes.
"""
import bisect, json, os, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

scratch, replay, chart = sys.argv[1], sys.argv[2], sys.argv[3]
deep_path = os.path.join(scratch, "medusa_deep.json")
deep = json.load(open(deep_path))
score = json.load(open(os.path.join(scratch, "medusa_score.json")))
chords = score["chords"]
ticks = [c["tick"] for c in chords]; ms = [c["ms"] for c in chords]
def ms_at(t):
    i = bisect.bisect_right(ticks, t) - 1
    if i < 0: return ms[0]
    if i >= len(ticks) - 1: return ms[-1]
    return ms[i] + (t - ticks[i]) / (ticks[i + 1] - ticks[i]) * (ms[i + 1] - ms[i])
fills = [(c["tick"], ms_at(c["fill_start"])) for c in chords if c.get("is_fill")]
phrases = [c["ms"] for c in chords if c["is_sp_phrase_end"]]
W = lambda a: (a["act_tick"], a["deact_tick"], a["sqout_tick"])
shell = {k: v for k, v in deep.items() if k != "paths"}
work = os.path.join(scratch, "gap_work"); os.makedirs(work, exist_ok=True)

def price(item):
    i, acts = item
    src, out = os.path.join(work, f"g{i}.json"), os.path.join(work, f"go{i}.json")
    json.dump({**shell, "paths": [{"activations": acts, "index": 0, "pathstring": "", "total": 0}]}, open(src, "w"))
    r = subprocess.run([replay, "target", "--chart", chart, "--path", src, "--out", out], capture_output=True, text=True)
    os.remove(src)
    if r.returncode != 0: return None
    o = json.load(open(out)); os.remove(out)
    return o["paths"][0] if o.get("realized", True) and o["paths"] else None

for round_no in range(4):
    paths = deep["paths"]
    keys = {tuple(W(a) for a in p["activations"]) for p in paths}
    prefixes = {}
    for p in paths:
        acts = p["activations"]
        for k in range(len(acts) + 1):
            prefixes.setdefault(tuple(W(a) for a in acts[:k]), acts[:k])
    nexts = {}
    for p in paths:
        acts = p["activations"]
        for k in range(len(acts)):
            nexts.setdefault(tuple(W(a) for a in acts[:k]), set()).add(acts[k]["act_tick"])
    jobs, seen = [], set()
    gaps = 0
    for pk, pacts in prefixes.items():
        last_t = pk[-1][1] if pk else -1
        last_ms = ms_at(pk[-1][1]) if pk else -1
        for ft, fstart in fills:
            if ft <= last_t: continue
            if sum(1 for m in phrases if last_ms < m < fstart) < 2: continue
            if ft in nexts.get(pk, ()): continue
            gaps += 1
            # Every known way of going on from fill ft: P's windows, then any
            # kept path's windows from its window at ft onward (and that
            # window alone).
            for p in paths:
                acts = p["activations"]
                for j, a in enumerate(acts):
                    if a["act_tick"] != ft: continue
                    for cand in (list(pacts) + acts[j:], list(pacts) + [a]):
                        ck = tuple(W(x) for x in cand)
                        if ck in keys or ck in seen: continue
                        seen.add(ck); jobs.append(cand)
    print(f"round {round_no}: gaps {gaps}, candidates {len(jobs)}", flush=True)
    if not jobs: break
    with ThreadPoolExecutor(max_workers=8) as ex:
        found = [p for p in ex.map(price, enumerate(jobs)) if p]
    added = 0
    for p in found:
        k = tuple(W(a) for a in p["activations"])
        if k not in keys:
            keys.add(k); deep["paths"].append(p); added += 1
    print(f"round {round_no}: realized {len(found)}, added {added}", flush=True)
    if not added: break

deep["paths"].sort(key=lambda p: -p["total"])
json.dump(deep, open(deep_path, "w"))
print("paths now", len(deep["paths"]))
