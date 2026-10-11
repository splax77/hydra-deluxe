import sys, os, bisect, sqlite3, json, time, multiprocessing as mp
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import lanelib as L
exec(open("simlib.py", encoding="utf-8").read())

def work(path):
    out = {"path": path, "lanes": [], "err": None}
    try:
        res, tempos, per = L.load_mid(path) if path.lower().endswith(".mid") else L.load_chart(path)
    except Exception as ex:
        out["err"] = repr(ex)[:80]; return out
    if per is None: return out
    for d, data in per.items():
        cs = sorted(data["chords"].items()); ticks = [t for t, _ in cs]
        for (s, e, dbl) in sorted(data["lanes"]):
            rec = {"diff": d, "dbl": bool(dbl), "start": s, "end": e}
            if dbl:
                lit = L.lit_set(cs, ticks, (s, e, dbl), True)
                lo = bisect.bisect_left(ticks, s); hi = bisect.bisect_right(ticks, e)
                seq = [(tk, col) for tk, ch in cs[lo:hi] for col in sorted(ch)]
                fl = [i for i, (tk, col) in enumerate(seq) if lit and col in lit]
                last = fl[-1] if fl else -1
                tp = [(L.secs(tempos, res, 0, tk), col, i in fl, i == last) for i, (tk, col) in enumerate(seq)]
                ev = simulate(tp, lit, True)
                gaps = []; lt = None
                for (t, col, f, la) in tp:
                    if f and lt is not None: gaps.append(t - lt)
                    if f: lt = t
                rec.update(lit="".join(sorted(L.NAME[c] for c in lit)) if lit else "", n=len(seq), flagged=len(fl),
                           maxgap=round(max(gaps), 3) if gaps else 0, breaks=[round(b[0], 3) for b in ev])
            out["lanes"].append(rec)
    return out

if __name__ == "__main__":
    c = sqlite3.connect("dbcopy/hydra.db")
    paths = [r[0] for r in c.execute("select path from charts")]
    sk = [p for p in paths if os.path.splitext(p)[1].lower() not in (".mid", ".chart")]
    todo = [p for p in paths if os.path.splitext(p)[1].lower() in (".mid", ".chart")]
    t0 = time.time(); results = []
    with mp.Pool(4) as pool:
        for i, r in enumerate(pool.imap_unordered(work, todo, chunksize=40), 1):
            results.append(r)
            if i % 2000 == 0: print(i, "of", len(todo), round(time.time() - t0), "s", flush=True)
    json.dump({"skipped_container": sk, "results": results}, open("libscan.json", "w"))
    print("done", len(results), round(time.time() - t0), "s")
