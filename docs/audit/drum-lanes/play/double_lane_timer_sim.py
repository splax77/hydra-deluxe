import sys, os, bisect
src = open(r"C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\audit\drum-lanes\play\lanecount.py", encoding="utf-8").read()
src = src.rsplit("\nmain()", 1)[0]
g = {"__name__": "lc", "__file__": r"C:\Users\Patrick\AppData\Local\Temp\x\lanecount.py"}
exec(compile(src, "lanecount", "exec"), g)
ROOT = g["ROOT"]; NAME = g["NAME"]
sys.stdout.reconfigure(encoding="utf-8", errors="replace")

def simulate(times_pads, lit, dbl_flag_note, T_BREAK=0.75, T_END=0.5):
    """times_pads: list of (t, colour, is_flagged, is_last). Perfect play: every note pressed at its time.
    returns list of (break_time, reason)."""
    mask = set(); mask2 = set(); dbl = False; fin = False
    t2e0 = 0.0; t2d8 = 0.0; c2ec = 0
    events = []
    for (t, col, flagged, last) in times_pads:
        # continuous timeouts before this note
        if mask and fin and t - t2d8 > T_END:
            mask = set(); mask2 = set(); dbl = False; fin = False; c2ec = 0
        if mask and dbl and not fin and t - t2e0 > T_BREAK:
            events.append((t2e0 + T_BREAK, "0.75s no full round"))
            mask = set(); mask2 = set(); dbl = False; c2ec = 0
        if flagged:
            c2ec = 0  # the press handler's qword write at 0x20DECF3 zeroes 0x2E8 and 0x2EC together
            mask2.discard(col)
            if mask and not mask2:
                t2e0 = t; mask2 = set(mask)
            elif not mask:
                # mask empty: and-result 0 -> ZF -> round reset path (mask2=mask=empty)
                t2e0 = t
            if col not in mask:
                dbl = dbl_flag_note
                mask.add(col); mask2.add(col); t2e0 = t
            if last:
                fin = True; t2d8 = t
        else:
            if mask:
                c2ec += 1
                if c2ec >= 3:
                    mask = set(); mask2 = set(); dbl = False; fin = False; c2ec = 0
    return events

rows = []
for dp, _, fs in os.walk(ROOT):
    for f in sorted(fs):
        if not (f.endswith(".mid") or f.endswith(".chart")): continue
        p = os.path.join(dp, f)
        try:
            res, tempos, per = g["load_mid"](p) if f.endswith(".mid") else g["load_chart"](p)
        except Exception as ex:
            continue
        if per is None: continue
        for d, data in per.items():
            cs = sorted(data["chords"].items()); ticks = [t for t, _ in cs]
            for (s, e, dbl) in sorted(data["lanes"]):
                if not dbl: continue
                lit = g["lit_set"](cs, ticks, (s, e, dbl), True)
                lo = bisect.bisect_left(ticks, s); hi = bisect.bisect_right(ticks, e)
                seq = []
                for tk, ch in cs[lo:hi]:
                    for col in sorted(ch):
                        seq.append((tk, col))
                flagged_idx = [i for i, (tk, col) in enumerate(seq) if lit and col in lit]
                last_i = flagged_idx[-1] if flagged_idx else -1
                tp = []
                for i, (tk, col) in enumerate(seq):
                    t = g["secs"](tempos, res, 0, tk)
                    tp.append((t, col, i in flagged_idx, i == last_i))
                ev = simulate(tp, lit, True)
                # also gaps between consecutive pad-completion info
                gaps = []
                last_t = None
                for (t, col, fl, la) in tp:
                    if fl and last_t is not None: gaps.append(t - last_t)
                    if fl: last_t = t
                rel = os.path.relpath(p, ROOT).replace("\\", "/")
                rows.append((rel, d, ''.join(sorted(NAME[c] for c in lit)) if lit else '', len(seq), len(flagged_idx),
                             round(max(gaps), 3) if gaps else 0, ev))
for r in rows:
    print(r[0].split("/")[-2][:40], r[1], "lit", r[2], "notes", r[3], "flagged", r[4], "maxgap", r[5], "BREAKS" if r[6] else "ok", r[6][:2])
