"""Price every distinct window prefix of the dumped paths with hydra_replay target.

target pins each window's activation, end and squeeze-out and declines every
other fill, so its total is what the player's own path (those windows and no
others) scores. Writes {prefix_key: total} to own_totals.json.
"""
import json, os, subprocess, sys, tempfile
from concurrent.futures import ThreadPoolExecutor

scratch, replay, chart = sys.argv[1], sys.argv[2], sys.argv[3]
dump = json.load(open(os.path.join(scratch, "medusa_deep.json")))

def key(acts):
    return ";".join(f"{a['act_tick']}:{a['deact_tick']}:{a['sqout_tick']}" for a in acts)

prefixes = {}
for p in dump["paths"]:
    a = p["activations"]
    for k in range(1, len(a) + 1):
        prefixes.setdefault(key(a[:k]), a[:k])

shell = {k: v for k, v in dump.items() if k != "paths"}
work = os.path.join(scratch, "prefix_work")
os.makedirs(work, exist_ok=True)

def run(item):
    i, (k, acts) = item
    src = os.path.join(work, f"p{i}.json")
    out = os.path.join(work, f"o{i}.json")
    json.dump({**shell, "paths": [{"activations": acts, "index": 0, "pathstring": "", "total": 0}]},
              open(src, "w"))
    r = subprocess.run([replay, "target", "--chart", chart, "--path", src, "--out", out],
                       capture_output=True, text=True)
    if r.returncode != 0:
        return k, None, r.stderr.strip()[:300]
    o = json.load(open(out))
    os.remove(src); os.remove(out)
    if not o.get("realized", True) or not o["paths"]:
        return k, None, "not realized: " + str(o.get("failed_reason"))
    return k, o["paths"][0]["total"], None

old_path = os.path.join(scratch, "own_totals.json")
old = json.load(open(old_path)) if os.path.exists(old_path) else {}
items = [(i, kv) for i, kv in enumerate(prefixes.items()) if kv[0] not in old]
print("known", len(old), "to price", len(items), flush=True)
totals, errors = dict(old), []
with ThreadPoolExecutor(max_workers=8) as ex:
    for n, (k, t, err) in enumerate(ex.map(run, items)):
        if err: errors.append((k, err))
        else: totals[k] = t
        if n % 500 == 0: print(n, "of", len(items), flush=True)

json.dump(totals, open(os.path.join(scratch, "own_totals.json"), "w"))
print("priced", len(totals), "errors", len(errors))
for e in errors[:10]: print(e)
