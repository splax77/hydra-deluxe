"""Gather Medusa's engine data into one JS file the mockup reads."""
import bisect, json, os, sys

scratch, out_js = sys.argv[1], sys.argv[2]
L = lambda n: json.load(open(os.path.join(scratch, n)))
deep, top, score, own = L("medusa_deep.json"), L("medusa_dump.json"), L("medusa_score.json"), L("own_totals.json")

chords = score["chords"]
ticks = [c["tick"] for c in chords]
ms = [c["ms"] for c in chords]
dec = [c["measure"]["decimal"] for c in chords]

def interp(t, ys):
    i = bisect.bisect_right(ticks, t) - 1
    if i < 0: return ys[0]
    if i >= len(ticks) - 1: return ys[-1]
    f = (t - ticks[i]) / (ticks[i + 1] - ticks[i])
    return ys[i] + f * (ys[i + 1] - ys[i])

def at(t):
    return {"ms": round(interp(t, ms)), "m": round(interp(t, dec), 2)}

def key(acts):
    return ";".join(f"{a['act_tick']}:{a['deact_tick']}:{a['sqout_tick']}" for a in acts)

def sq_list(a):
    # Each squeeze's figure as Hydra prints it: SPSqueeze::difficulty
    # (squeeze_difficulty in core/model.h) and needs_no_timing
    # (paid_by_sp_walk: offset <= 0) for SqIn, the opposite for SqOut.
    out = []
    for s in a["sqinouts"]:
        sqin = s["kind"] == "SqIn"
        off = s["offset_ms"]
        diff = off if sqin else -off + 0.0
        needs = (off > 0) if sqin else (off <= 0)
        out.append({"k": "in" if sqin else "out", "ms": round(diff, 1), "needs": needs})
    return out

paths = []
for p in deep["paths"]:
    acts = p["activations"]
    toks = p["pathstring"].split(" ")
    paths.append({
        "total": p["total"],
        "w": [{"a": a["act_tick"], "d": a["deact_tick"], "q": a["sqout_tick"], "bars": a["sp_meter"],
               "sq": sq_list(a), "n": toks[i] if len(toks) == len(acts) else "?"} for i, a in enumerate(acts)],
        "own": [own.get(key(acts[:k + 1])) for k in range(len(acts))],
    })
deep_keys = {key(p["activations"]) for p in deep["paths"]}
named = []
for i, p in enumerate(top["paths"]):
    named.append({"name": "Best path" if i == 0 else f"Path {i + 1}", "total": p["total"],
                  "notation": p["pathstring"], "key": key(p["activations"]),
                  "inDeep": key(p["activations"]) in deep_keys})

# The highway: every chord as [ms, lane bits], the drum track's Star Power
# phrases read from the chart, and where each measure starts.
BITS = {"Red": 1, "Yellow": 2, "Blue": 4, "Green": 8, "Kick": 16}
CYM = {"Yellow": 32, "Blue": 64, "Green": 128}
def chord_bits(c):
    b = 0
    for n in c["notes"]:
        b |= BITS[n["color"]]
        if n["cymbal"]: b |= CYM.get(n["color"], 0)
        if n["dynamic"] == "ghost": b |= 256
    return b
gems = [[round(c["ms"], 1), chord_bits(c)] for c in chords]

chart_path = sys.argv[3]
phrases_sp, in_drums = [], False
for line in open(chart_path, encoding="utf-8-sig"):
    line = line.strip()
    if line.startswith("["): in_drums = line == "[ExpertDrums]"
    elif in_drums and "= S 2 " in line:
        t, rest = line.split("=", 1)
        length = int(rest.split()[2])
        phrases_sp.append([round(interp(int(t), ms)), round(interp(int(t) + length, ms))])

measures = []
for n in range(1, int(dec[-1]) + 2):
    target = n - 1
    i = bisect.bisect_left(dec, target)
    if i >= len(dec): break
    if i == 0: measures.append(round(ms[0])); continue
    f = (target - dec[i - 1]) / (dec[i] - dec[i - 1]) if dec[i] != dec[i - 1] else 0
    measures.append(round(ms[i - 1] + f * (ms[i] - ms[i - 1])))

all_ticks = sorted({a for p in paths for w in p["w"] for a in (w["a"], w["d"], w["q"]) if a >= 0} |
                   {c["tick"] for c in chords if c.get("is_fill")})
data = {
    "song": "(I Used to Make Out With) Medusa", "artist": "Bring Me the Horizon",
    "mode": score["chartmode"], "spCap": deep["sp_cap"],
    "lengthMs": round(ms[-1]) + 1500,
    "sections": [{"name": s["name"], "ms": round(s["ms"])} for s in score["sections"]],
    "fills": [{"tick": c["tick"], "ms": round(c["ms"]), "startMs": round(interp(c["fill_start"], ms)),
               "label": c["measure"]["label"]} for c in chords if c.get("is_fill")],
    "phrases": [round(c["ms"]) for c in chords if c["is_sp_phrase_end"]],
    "notes": [round(m) for m in ms],
    "gems": gems, "spPhrases": phrases_sp, "measures": measures, "pathLimitMs": 10,
    "tickAt": {str(t): at(t) for t in all_ticks},
    "paths": paths, "named": named,
    "depthPoints": 145000,
}
missing = sum(1 for p in paths for o in p["own"] if o is None)
open(out_js, "w", encoding="utf-8").write("window.MEDUSA = " + json.dumps(data, separators=(",", ":")) + ";\n")
print("paths", len(paths), "fills", len(data["fills"]), "phrases", len(data["phrases"]),
      "own missing", missing, "named in deep", [n["inDeep"] for n in named],
      "bytes", os.path.getsize(out_js))
