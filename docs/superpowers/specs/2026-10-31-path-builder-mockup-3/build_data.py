"""Gather one chart's engine data into the JS file the third mockup reads.

The third mockup uses "Divine Inner Tension (Album)" by Hail The Sun, a
42-minute chart, Expert Pro Drums, 2x kick, SP cap 4, 10 ms Path limit (the
user's options, read from their database).

Inputs, all in the scratch folder: score.json (hydra_replay score, no Star
Power), medusa_deep.json (the dump, cut to its best paths), own_totals.json
(own_totals.py), notes.mid and song.ini (the chart). own_totals.py lives in
2026-10-30-path-builder-mockup/ and keeps the old file names.

The album's dump holds 27,020 paths and 369,784 distinct window prefixes; one
hydra_replay target run per prefix is too slow for a mockup. So only the paths
within 20,000 points of the best are kept (878 paths, 12,260 prefixes). The
second mockup's gapfill.py step was dropped: on this chart it ran past a
200-second cap. Fills no kept path activates at show as "no engine numbers".

Every path prefix becomes one node: its own score (hydra_replay target over
exactly those windows), the best finish any kept path reaches from it, and each
next window a kept path takes from it. The mockup reads nothing else.

To rebuild, with notes.mid and song.ini copied into the scratch folder S and R
the hydra_replay exe (the dump takes about 8 seconds, the pricing about 76):
  R score --chart notes.mid --prodrums 1 --bass2x 1 --out score.json
  R dump --chart notes.mid --cap 4 --ms 10 --depth-mode points --depth 160000 --out deep.json
  py -c "import json; d=json.load(open('deep.json')); B=d['paths'][0]['total']; d['paths']=[p for p in d['paths'] if p['total']>=B-20000]; json.dump(d,open('medusa_deep.json','w'))"
  py ../2026-10-30-path-builder-mockup/own_totals.py S R notes.mid
  py build_data.py S pb-data.js
  py inline.py mockup_template.html pb-data.js path-builder-mockup-3.html
pb-data.js is not committed when it is over 5 MB; the inlined page holds it.
"""
import bisect, json, os, struct, sys

scratch, out_js = sys.argv[1], sys.argv[2]
L = lambda n: json.load(open(os.path.join(scratch, n), encoding="utf-8"))
score, deep, own = L("score.json"), L("medusa_deep.json"), L("own_totals.json")
chords = score["chords"]
NO_SP = score["final"]["total"]

# ---- the MIDI file: tempo map, time signatures, the drum track's SP phrases
def read_midi(path):
    data = open(path, "rb").read()
    assert data[:4] == b"MThd"
    hlen, _fmt, ntrk, res = struct.unpack(">IHHH", data[4:14])
    pos, tracks = 8 + hlen, []
    for _ in range(ntrk):
        assert data[pos:pos + 4] == b"MTrk"
        (tlen,) = struct.unpack(">I", data[pos + 4:pos + 8])
        tracks.append(data[pos + 8:pos + 8 + tlen]); pos += 8 + tlen
    def events(buf):
        i, t, status, out = 0, 0, 0, []
        def vlq():
            nonlocal i
            v = 0
            while True:
                b = buf[i]; i += 1; v = (v << 7) | (b & 0x7F)
                if not b & 0x80: return v
        while i < len(buf):
            t += vlq()
            b = buf[i]
            if b == 0xFF:
                typ = buf[i + 1]; i += 2; n = vlq(); out.append((t, "meta", typ, buf[i:i + n])); i += n
            elif b in (0xF0, 0xF7):
                i += 1; n = vlq(); i += n
            else:
                if b & 0x80: status = b; i += 1
                hi = status & 0xF0
                if hi in (0xC0, 0xD0): d = (buf[i],); i += 1
                else: d = (buf[i], buf[i + 1]); i += 2
                out.append((t, "ch", hi, d))
        return out
    return res, [events(t) for t in tracks]

res, tracks = read_midi(os.path.join(scratch, "notes.mid"))
tempos, sigs, drums = [], [], None
for evs in tracks:
    name = next((e[3].decode("latin-1") for e in evs if e[1] == "meta" and e[2] == 3), "")
    for t, kind, typ, d in evs:
        if kind == "meta" and typ == 0x51: tempos.append([t, (d[0] << 16) | (d[1] << 8) | d[2]])
        if kind == "meta" and typ == 0x58: sigs.append([t, d[0], 2 ** d[1]])
    if name.upper() == "PART DRUMS": drums = evs
assert drums is not None, "no PART DRUMS track"
tempos.sort(); sigs.sort()
if not sigs or sigs[0][0] != 0: sigs.insert(0, [0, 4, 4])

# Tick -> ms over the tempo map (the same rule every MIDI reader uses).
tms, acc = [], 0.0
for i, (t, us) in enumerate(tempos):
    if i: acc += (t - tempos[i - 1][0]) * tempos[i - 1][1] / res / 1000.0
    tms.append(acc)
def ms_of(t):
    i = bisect.bisect_right([x[0] for x in tempos], t) - 1
    return tms[i] + (t - tempos[i][0]) * tempos[i][1] / res / 1000.0

# Star Power phrases: note 116 on/off in the drum track.
on, phrases = {}, []
for t, kind, hi, d in drums:
    if kind != "ch" or len(d) < 2 or d[0] != 116: continue
    if hi == 0x90 and d[1] > 0: on[116] = t
    elif 116 in on: phrases.append([on.pop(116), t])

# The chart's own check: the score's phrase-end chords sit inside these spans.
ends = [c["tick"] for c in chords if c["is_sp_phrase_end"]]
assert len(ends) == len(phrases), (len(ends), len(phrases))
for e, (a, b) in zip(ends, phrases): assert a <= e < b, (e, a, b)
# The score's ms and the tempo map agree.
worst = max(abs(ms_of(c["tick"]) - c["ms"]) for c in chords)
assert worst < 0.01, worst

# ---- chords
BITS = {"Red": 1, "Yellow": 2, "Blue": 4, "Green": 8, "Kick": 16}
CYM = {"Yellow": 32, "Blue": 64, "Green": 128}
def bits(c):
    b = 0
    for n in c["notes"]:
        b |= BITS[n["color"]]
        if n["cymbal"]: b |= CYM.get(n["color"], 0)
        if n["dynamic"] == "ghost": b |= 256 << (BITS[n["color"]].bit_length() - 1)
        if n["dynamic"] == "accent": b |= 8192 << (BITS[n["color"]].bit_length() - 1)
    return b
def note_label(n):
    col = n["color"]
    if col == "Kick": return "Kick"
    if col == "Red": return "Red snare"
    return col + (" cymbal" if n["cymbal"] else " tom")
def rowstr(c):
    words = []
    for n in c["notes"]:
        w = note_label(n)
        if n["dynamic"] in ("ghost", "accent"): w += " (" + n["dynamic"] + ")"
        words.append(w)
    return "[" + " - ".join(words) + "]"
# The star base: per note 50, +15 on a cymbal, doubled on a ghost or accent
# (ChordNote::basescore, core/model.cpp).
base = sum((50 + (15 if n["cymbal"] else 0)) * (2 if n["dynamic"] in ("ghost", "accent") else 1)
           for c in chords for n in c["notes"])

ch_out = []
for c in chords:
    pts = sum(c["points"].values())
    ch_out.append([round(c["ms"], 3), c["tick"], bits(c), pts, c["multiplier"],
                   c["combo_before"] + len(c["notes"]), c["cum"]["total"]])
# The combo after a chord is the next chord's combo before (checked here).
for i in range(len(chords) - 1):
    assert chords[i + 1]["combo_before"] == ch_out[i][5], i

# ---- paths into a prefix tree
def wkey(a): return f"{a['act_tick']}:{a['deact_tick']}:{a['sqout_tick']}"
def sq_list(a):
    out = []
    for s in a["sqinouts"]:
        sqin = s["kind"] == "SqIn"
        off = s["offset_ms"]
        diff = off if sqin else -off + 0.0
        needs = (off > 0) if sqin else (off <= 0)
        out.append({"k": "in" if sqin else "out", "ms": round(diff, 1), "needs": needs})
    return out

nodes = {"": {"own": NO_SP, "best": 0, "end": False, "next": {}}}
missing_own = 0
for p in deep["paths"]:
    acts = p["activations"]
    toks = p["pathstring"].split(" ")
    key = ""
    for i, a in enumerate(acts):
        child = (key + ";" if key else "") + wkey(a)
        n = nodes[key]
        n["best"] = max(n["best"], p["total"])
        w = {"a": a["act_tick"], "d": a["deact_tick"], "q": a["sqout_tick"], "bars": a["sp_meter"],
             "sq": sq_list(a), "n": toks[i] if len(toks) == len(acts) else "",
             "skip": a["skipped_fill_ticks"]}
        n["next"].setdefault(wkey(a), {"w": w, "best": 0})
        n["next"][wkey(a)]["best"] = max(n["next"][wkey(a)]["best"], p["total"])
        if child not in nodes:
            o = own.get(child)
            if o is None: missing_own += 1
            nodes[child] = {"own": o, "best": 0, "end": False, "next": {}}
        key = child
    nodes[key]["best"] = max(nodes[key]["best"], p["total"])
    nodes[key]["end"] = True
    # A whole path's own score is its total.
    assert nodes[key]["own"] in (None, p["total"]), (key, nodes[key]["own"], p["total"])

best = deep["paths"][0]
ini = {}
for line in open(os.path.join(scratch, "song.ini"), encoding="utf-8-sig"):
    k, sep, v = line.partition("=")
    if sep: ini[k.strip().lower()] = v.strip()
data = {
    "song": ini["name"], "artist": ini["artist"], "charter": ini.get("charter", ""),
    "mode": score["chartmode"], "spCap": deep["sp_cap"], "noSp": NO_SP, "starBase": base,
    "res": res, "tempos": tempos, "sigs": sigs,
    "lengthMs": int(ini["song_length"]),   # song.ini song_length, the one song end (D255)
    "sections": [{"name": s["name"], "ms": round(s["ms"], 3), "tick": s["tick"]} for s in score["sections"]],
    "fills": [{"tick": c["tick"], "ms": round(c["ms"], 3), "startTick": c["fill_start"],
               "startMs": round(ms_of(c["fill_start"]), 3), "label": c["measure"]["label"],
               "chord": rowstr(c)} for c in chords if c.get("is_fill")],
    "phraseEnds": [c["tick"] for c in chords if c["is_sp_phrase_end"]],
    "phrases": [[a, b, round(ms_of(a), 3), round(ms_of(b), 3)] for a, b in phrases],
    "chords": ch_out, "chordRows": {str(c["tick"]): rowstr(c) for c in chords},
    "bestKey": ";".join(wkey(a) for a in best["activations"]),
    "nodes": {k: {"o": v["own"], "b": v["best"], "e": v["end"],
                  "x": [[c["w"], c["best"]] for c in v["next"].values()]} for k, v in nodes.items()},
    "pathLimitMs": 10,
}
open(out_js, "w", encoding="utf-8").write("window.PB = " + json.dumps(data, separators=(",", ":"), ensure_ascii=False) + ";\n")
print("paths", len(deep["paths"]), "nodes", len(nodes), "own missing", missing_own, "fills", len(data["fills"]),
      "phrases", len(phrases), "tempos", len(tempos), "sigs", len(sigs), "star base", base,
      "best", best["total"], "bytes", os.path.getsize(out_js))
