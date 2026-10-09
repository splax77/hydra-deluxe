"""Build the Note Shuffle game-test songs and their predictions.

usage: py -I build.py <out dir>
Writes <out>/songs/<NS ...>/ (notes, song.ini, silent song.ogg) and <out>/predictions.json.
"""
import json, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ns_lib import *

OUT = sys.argv[1]
SONGS = os.path.join(OUT, "songs")

# ---- A: tick scale. Same groove as a 192 .chart and a 960 .mid (8th notes).
GROOVE_A = [[K, YC], [YC], [R], [YC], [K, YC], [K], [R], [YC],
            [K, GC], [BT], [R], [GT], [K, YT], [BT], [R], [K, GC]]
A_START_192 = 960  # beat 6 at 192 ticks per beat (found by design.py "A")
A192 = [(A_START_192 + 96 * i, list(c)) for i, c in enumerate(GROOVE_A)]
A960 = [(t * 5, c) for t, c in A192]

# ---- B: note order inside a tick (480 .mid). Note-ons written in this list order.
B480 = [(1920, [GT, K, YC]), (2160, [YC, K, GT]), (2400, [R, GC]), (2640, [YC, K, BT]),
        (2880, [BC, YT]), (3120, [YC, K]), (3360, [YC, K]), (3600, [GT, YC])]

# ---- C: lead-in, a four-colour chord, then a lone snare (480 .mid). C0 drops the snare.
LEAD = [[K, YC], [YC], [R], [YC], [K, YC], [YC], [R], [YC]]
C480 = [(1920 + 240 * i, list(c)) for i, c in enumerate(LEAD)]
C480 += [(1920 + 240 * 8, [R, YC, BT, GC]), (1920 + 240 * 10, [R])]
C0_480 = C480[:-1]

# ---- D: four notes on tick 0, then a groove (480 .mid). D0 is the same shifted 2 bars.
D_TAIL = [[YC], [R], [YC], [K, R], [YC], [BT], [K, YC], [R]]
D480 = [(0, [K, R, YC, BT])] + [(480 + 240 * i, list(c)) for i, c in enumerate(D_TAIL)]
D0_480 = [(t + 3840, c) for t, c in D480]

# ---- E: star cutoffs before vs after the shuffle (480 .mid, 110 BPM, no cymbals written).
# 82 chords picked by design_e.py: widest margin between the two hypotheses' 6-star cutoffs.
import design_e
E_BPM, E_CHORDS = 110, 82
E480 = design_e.original(E_CHORDS)

SONG_LIST = [
    ("NS A1 tick scale chart192", "chart", 192, A192, "A"),
    ("NS A2 tick scale mid960", "mid", 960, A960, "A"),
    ("NS B note order mid480", "mid", 480, B480, "B"),
    ("NS C1 four colour then snare", "mid", 480, C480, "C"),
    ("NS C2 control four colour last", "mid", 480, C0_480, "C"),
    ("NS D1 four notes on tick 0", "mid", 480, D480, "D"),
    ("NS D2 control same notes later", "mid", 480, D0_480, "D"),
    ("NS E star cutoffs", "mid", 480, E480, "E"),
]
BPM = {"NS E star cutoffs": E_BPM}


def e_stars(path_orig):
    """Score the shuffled E chart in Hydra (no Star Power) and give both hypotheses' cutoffs."""
    import subprocess
    sh = predict(E480, pro=True)
    p = os.path.join(os.path.dirname(os.path.abspath(__file__)), "e_shuffled_final.mid")
    with open(p, "wb") as f:
        f.write(mid_bytes("E shuffled", 480, sh, E_BPM))
    out = {}
    for label, chart, pro in (("pro drums on, shuffled", p, "1"), ("pro drums off (no cymbals either way)", path_orig, "0")):
        r = subprocess.run([REPO + "/build-cpp/Release/hydra_replay.exe", "score", "--chart", chart,
                            "--prodrums", pro, "--bass2x", "0"], capture_output=True, text=True, check=True)
        j = json.loads(r.stdout)
        notes = [n for c in j["chords"] for n in c["notes"]]
        gems, cym = len(notes), sum(n["cymbal"] for n in notes)
        score = j["chords"][-1]["cum"]["total"]
        b_before, b_after = 50 * gems, 50 * gems + 15 * cym
        out[label] = {"gems": gems, "cymbals_after_shuffle": cym, "full_combo_score_hydra": score,
                      "final": j.get("final"),
                      "base_before": b_before, "base_after": b_after,
                      "stars_if_base_before": design_e.stars(score, b_before),
                      "stars_if_base_after": design_e.stars(score, b_after),
                      "cutoffs_base_before": [design_e.cutoff(b_before, i) for i in range(7)],
                      "cutoffs_base_after": [design_e.cutoff(b_after, i) for i in range(7)]}
    return out


def rows(res, chords):
    if res == "HANG":
        return "HANG"
    return [{"tick": t, "pads": pads_text(ms)} for t, ms in res]


def main(extra=()):
    preds = {}
    for name, fmt, resol, chords, test in list(SONG_LIST) + list(extra):
        path = write_song(os.path.join(SONGS, name), name, resol, chords, fmt, BPM.get(name, 120))
        entry = {"file": os.path.relpath(path, OUT).replace("\\", "/"), "resolution": resol,
                 "written": [{"tick": t, "notes_in_file_order": [NAME[m] for m in c]} for t, c in chords],
                 "predictions": {}}
        hyps = {"raw ticks": (1, 1)}
        if test == "A":
            base = resol
            hyps = {"ticks as 192": (192, base), "ticks as 480": (480, base), "ticks as 960": (960, base)}
        hows = ("file", "lane", "bit", "reverse") if test == "B" else ("lane",)
        for pro in (True, False):
            for hname, (num, den) in hyps.items():
                for how in hows:
                    key = f"pro drums {'on' if pro else 'off'} | {hname} | order {how}"
                    entry["predictions"][key] = rows(predict(chords, how=how, num=num, den=den, pro=pro), chords)
        if test == "E":
            entry["stars"] = e_stars(path)
        preds[name] = entry
        print("built", name)
    with open(os.path.join(OUT, "predictions.json"), "w", encoding="utf-8", newline="\n") as f:
        json.dump(preds, f, indent=1)
    return preds


if __name__ == "__main__":
    main()
