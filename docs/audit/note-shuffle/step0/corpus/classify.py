"""Steps 2-4: feed each extracted run to the capped Note Shuffle reference and classify it.

Reads chords.json (from extract.py). Writes results.csv and results.json next to this file.
The reference is shuffle_ref_capped.py, a byte-identical copy (same SHA-256) of
docs/audit/note-shuffle/probes/capped/shuffle_ref_capped.py.
"""
import csv, json, sys
from pathlib import Path
from collections import Counter

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.dont_write_bytecode = True
import shuffle_ref_capped as ref  # noqa: E402

LANE_ORDER = ["Kick", "Red", "Yellow", "Blue", "Green"]
TOM = {"Kick": 1, "Red": 2, "Yellow": 4, "Blue": 8, "Green": 16}
CYM = {"Yellow": 32, "Blue": 64, "Green": 128}
COLOUR = {2: "R", 4: "Y", 32: "Y", 8: "B", 64: "B", 16: "G", 128: "G"}


class Note:
    def __init__(self, tick, mask):
        self.tick, self.flags = tick, 0
        self.orig = mask
        self._mask = mask
        self.done = False

    @property
    def mask(self):
        return self._mask

    @mask.setter
    def mask(self, v):
        self._mask = v
        self.done = True


def build(chords, reverse):
    notes, by_tick = [], {}
    for tick, ns in chords:
        lanes = sorted(ns, key=lambda n: LANE_ORDER.index(n[0]))
        if len({n[0] for n in lanes}) != len(lanes):
            raise ValueError("two notes on one lane at tick %d" % tick)
        objs = [Note(tick, CYM[c] if cym else TOM[c]) for c, cym in lanes]
        if reverse:
            objs.reverse()
        notes += objs
        by_tick[tick] = objs
    return notes, by_tick


def seed_of(notes):
    s = 0
    for n in notes[:4]:
        s += n.orig * n.tick
    return s


def run(chords, instrument, reverse=False):
    notes, by_tick = build(chords, reverse)
    res = {"seed": seed_of(notes), "n_notes": len(notes),
           "cut_inside_chord": len(notes) > 4 and notes[3].tick == notes[4].tick}
    try:
        ref.shuffle(notes, instrument, lambda n: by_tick[n.tick])
        res["status"] = "finished"
    except RuntimeError as e:
        if str(e) != "hang":
            raise
        res["status"] = "froze_seed_zero" if res["seed"] == 0 else "froze_other"
        hang = next(n for n in notes if not (n.orig & 1) and not n.done)
        res["hang_tick"] = hang.tick
        pads_at = [n for n in by_tick[hang.tick] if not (n.orig & 1)]
        res["hang_inside_chord"] = any(n.done for n in pads_at)
        prev = [t for t, objs in by_tick.items() if t < hang.tick and any(not (o.orig & 1) for o in objs)]
        if prev:
            pt = max(prev)
            outs = [o.mask for o in by_tick[pt] if not (o.orig & 1)]
            res["block_tick"] = pt
            res["block_pad_notes"] = len(outs)
            res["block_out_colours"] = "".join(sorted({COLOUR.get(m, "?") for m in outs}))
            res["block_in_colours"] = "".join(sorted({COLOUR[o.orig] for o in by_tick[pt] if not (o.orig & 1)}))
    # checks over every note the reference reached
    clashes, empties = [], []
    for t, objs in by_tick.items():
        pads = [o for o in objs if not (o.orig & 1) and o.done]
        cols = [COLOUR.get(o.mask) for o in pads if o.mask]
        if len(cols) != len(set(cols)):
            clashes.append(t)
        empties += [t for o in pads if o.mask == 0]
    res["clash_ticks"] = clashes
    res["empty_ticks"] = empties
    res["out"] = {(n.tick, n.orig): n.mask for n in notes if n.done}
    return res


def main():
    recs = json.loads((HERE / "chords.json").read_text(encoding="utf-8"))
    rows = []
    for r in recs:
        row = {"chart": r["chart"].split("testdata\\input\\")[-1], "difficulty": r["difficulty"],
               "pro": r["pro"], "bass2x": r["bass2x"]}
        if r["exit"] != 0:
            row["status"] = "no_notes" if "has no notes" in r.get("stderr", "") else "extract_failed"
            row["detail"] = r.get("stderr", "")
            rows.append(row)
            continue
        chords = r["chords"]
        inst = 9 if r["pro"] else 6
        four = [t for t, ns in chords if sum(1 for c, _ in ns if c != "Kick") == 4]
        a = run(chords, inst)
        b = run(chords, inst, reverse=True)
        changed = (a["status"] != b["status"] or a.get("hang_tick") != b.get("hang_tick")
                   or a["out"] != b["out"])
        row.update({
            "status": a["status"], "chords": len(chords), "notes": a["n_notes"], "seed": a["seed"],
            "four_colour_chords": len(four), "first_four_colour_tick": four[0] if four else "",
            "hang_tick": a.get("hang_tick", ""), "hang_inside_chord": a.get("hang_inside_chord", ""),
            "block_tick": a.get("block_tick", ""), "block_pad_notes": a.get("block_pad_notes", ""),
            "block_in_colours": a.get("block_in_colours", ""), "block_out_colours": a.get("block_out_colours", ""),
            "clash_ticks": " ".join(map(str, a["clash_ticks"])), "empty_ticks": " ".join(map(str, a["empty_ticks"])),
            "seed_cut_inside_chord": a["cut_inside_chord"],
            "rev_status": b["status"], "rev_seed": b["seed"], "rev_hang_tick": b.get("hang_tick", ""),
            "rev_clash_ticks": " ".join(map(str, b["clash_ticks"])), "rev_empty_ticks": " ".join(map(str, b["empty_ticks"])),
            "order_changes_output": changed,
            "first_ticks": " ".join(str(t) for t, _ in chords[:4]),
        })
        rows.append(row)
    keys = []
    for row in rows:
        for k in row:
            if k not in keys:
                keys.append(k)
    with open(HERE / "results.csv", "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=keys)
        w.writeheader()
        w.writerows(rows)
    (HERE / "results.json").write_text(json.dumps(rows, indent=1), encoding="utf-8")
    print(Counter(r["status"] for r in rows))
    print("rev", Counter(r.get("rev_status") for r in rows))


if __name__ == "__main__":
    main()
