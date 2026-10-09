"""Make the expected chords for tests/test_note_shuffle.cpp from the capped reference.

usage: py -I gen_vectors.py

Every literal in that test comes from one run of this script. Two kinds of input:

1. The game-test songs (../../game-tests/songs). Hydra reads each one through
   hydra_replay "score" (step0/corpus/extract.py does the reading), and the
   chords go to the reference in lane order, exactly as step0/corpus/classify.py
   builds them. The script checks every result against the row the game
   confirmed in game-tests/predictions.json.
2. Small made-up sequences, one per rule the test pins. For each, the script
   watches which branch of the reference fired (by wrapping its helpers), so a
   case that no longer exercises its rule fails here, not silently in C++.

Prints C++ initialisers to paste into the test, and writes the same text to
vectors_output.txt next to this file.
"""
import importlib.util
import io
import sys
from pathlib import Path

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
NS = HERE.parent.parent  # docs/audit/note-shuffle


def load(path, name):
    spec = importlib.util.spec_from_file_location(name, path)
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


ref = load(NS / "probes/capped/shuffle_ref_capped.py", "shuffle_ref_capped")
extract = load(NS / "step0/corpus/extract.py", "extract")

LANE_ORDER = ["Kick", "Red", "Yellow", "Blue", "Green"]
TOM = {"Kick": 1, "Red": 2, "Yellow": 4, "Blue": 8, "Green": 16}
CYM = {"Yellow": 32, "Blue": 64, "Green": 128}
BIT_TO_LANE = {1: ("Kick", False), 2: ("Red", False), 4: ("Yellow", False), 8: ("Blue", False),
               16: ("Green", False), 32: ("Yellow", True), 64: ("Blue", True), 128: ("Green", True)}
DYN_CH = {"none": "n", "ghost": "g", "accent": "a"}


class Note:
    def __init__(self, tick, colour, cym, dyn="none", is2x=False):
        self.tick, self.flags = tick, 0
        self.colour, self.dyn, self.is2x = colour, dyn, is2x
        self.mask = CYM[colour] if cym else TOM[colour]
        self.orig = self.mask


def build(chords):
    """chords: [(tick, [(colour, cym, dyn, is2x), ...])] -> notes in lane order, by_tick."""
    notes, by_tick = [], {}
    for tick, ns in chords:
        lanes = sorted(ns, key=lambda n: LANE_ORDER.index(n[0]))
        objs = [Note(tick, *n) for n in lanes]
        notes += objs
        by_tick.setdefault(tick, []).extend(objs)
    return notes, by_tick


def code_of(objs):
    """Chord::code() of the shuffled notes: KRYBG, n/g/a, upper case for a cymbal or 2x kick."""
    out = ["."] * 5
    for o in objs:
        colour, cym = BIT_TO_LANE[o.mask]
        i = LANE_ORDER.index(colour)
        if out[i] != ".":
            raise ValueError("two notes on one lane at tick %d" % o.tick)
        ch = DYN_CH[o.dyn]
        out[i] = ch.upper() if (cym or (colour == "Kick" and o.is2x)) else ch
    return "".join(out)


TRACE = []
_bits = ref.bits_low_to_high
_pads = ref.pads


def traced_bits(x):
    TRACE.append("copy")
    return _bits(x)


def traced_pads(m):
    TRACE.append("redraw")
    return _pads(m)


def run(chords, pro):
    """Shuffle; return (codes per tick or None on a freeze, branch per pad note)."""
    notes, by_tick = build(chords)
    ref.bits_low_to_high = traced_bits
    ref.pads = traced_pads
    TRACE.clear()
    branches = []

    # Record each pad note's branch: the trace entries since the last pad note.
    orig_setattr = Note.__setattr__

    def setattr_watch(self, k, v):
        if k == "mask" and hasattr(self, "orig"):
            branches.append((self.tick, self.colour, "copy" if "copy" in TRACE else "redraw"))
            TRACE.clear()
        orig_setattr(self, k, v)

    Note.__setattr__ = setattr_watch
    try:
        ref.shuffle(notes, 9 if pro else 6, lambda n: by_tick[n.tick])
        codes = [(t, code_of(by_tick[t])) for t, _ in chords]
    except RuntimeError as e:
        if str(e) != "hang":
            raise
        codes = None
    finally:
        Note.__setattr__ = orig_setattr
        ref.bits_low_to_high = _bits
        ref.pads = _pads
    return codes, branches


# ---- 1. game-test songs ---------------------------------------------------

SONGS = ["NS A1 tick scale chart192", "NS A2 tick scale mid960", "NS B note order mid480",
         "NS C1 four colour then snare", "NS C2 control four colour last",
         "NS D1 four notes on tick 0", "NS D2 control same notes later", "NS E star cutoffs"]

PAD_NAMES = {("Kick", False): "kick", ("Red", False): "red snare", ("Yellow", False): "yellow tom",
             ("Blue", False): "blue tom", ("Green", False): "green tom",
             ("Yellow", True): "yellow cymbal", ("Blue", True): "blue cymbal",
             ("Green", True): "green cymbal"}


def names_of_code(code, pro):
    out = set()
    for i, ch in enumerate(code):
        if ch == ".":
            continue
        colour = LANE_ORDER[i]
        cym = ch.isupper() and colour != "Kick"
        name = PAD_NAMES[(colour, cym)]
        if not pro and colour == "Red":
            name = "red snare"
        out.add(name)
    return out


def confirmed_row(pred, song, pro):
    p = pred[song]
    on = "on" if pro else "off"
    for k in (f"pro drums {on} | ticks as {p['resolution']} | order lane",
              f"pro drums {on} | raw ticks | order lane"):
        if k in p["predictions"]:
            return p["predictions"][k]
    raise KeyError(song)


def song_chords(song, pro):
    path = str(NS / "game-tests/songs" / song / ("notes.chart" if "chart192" in song else "notes.mid"))
    rec = extract.one((path, ("expert", pro, 0)))
    if rec["exit"] != 0:
        raise RuntimeError(rec)
    return [(t, [(c, cym, "none", False) for c, cym in ns]) for t, ns in rec["chords"]]


def emit_songs(w):
    import json
    pred = json.loads((NS / "game-tests/predictions.json").read_text(encoding="utf-8"))
    for song in SONGS:
        for pro in (1, 0):
            chords = song_chords(song, pro)
            if not pro and any(cym for _, ns in chords for _, cym, _, _ in ns):
                raise RuntimeError("cymbal in a Pro Drums off read of " + song)
            codes, _ = run(chords, pro)
            row = confirmed_row(pred, song, pro)
            if codes is None or row == "HANG":
                if not (codes is None and row == "HANG"):
                    raise RuntimeError(f"{song} pro={pro}: freeze disagrees with predictions.json")
                w(f'// {song}, Pro Drums {"on" if pro else "off"}: the game freezes')
                continue
            if len(row) != len(codes):
                raise RuntimeError(f"{song} pro={pro}: {len(codes)} chords, predictions {len(row)}")
            for (t, c), r in zip(codes, row):
                if t != r["tick"] or names_of_code(c, pro) != set(r["pads"].split(" + ")):
                    raise RuntimeError(f"{song} pro={pro} tick {t}: {c} vs prediction {r}")
            body = ", ".join(f'{{{t}, "{c}"}}' for t, c in codes)
            w(f'// {song}, Pro Drums {"on" if pro else "off"}: matches predictions.json')
            w(f"{{{body}}},")


# ---- 2. made-up sequences -------------------------------------------------

def N(colour, cym=False, dyn="none", is2x=False):
    return (colour, cym, dyn, is2x)


def emit_case(w, name, chords, pro, codes):
    w(f"// {name} (Pro Drums {'on' if pro else 'off'})")
    w("// input:  " + ", ".join(f'{{{t}, "{code_of(build([(t, ns)])[0])}"}}' for t, ns in chords))
    w("// output: " + ", ".join(f'{{{t}, "{c}"}}' for t, c in codes))


def emit_synthetic(w):
    # Copy rule. Chord 3's first note is checked against chord 1 (two back) and
    # copies; chord 4's first note is checked against chord 2 (two back) and
    # redraws, though chord 3 (one back) has its shape; chord 4's second note is
    # checked against chord 3 (one back) and copies.
    copy = [(480, [N("Red"), N("Yellow", True)]), (960, [N("Blue")]),
            (1440, [N("Red"), N("Yellow", True)]), (1920, [N("Red"), N("Yellow", True)])]
    codes, br = run(copy, 1)
    want = [(480, "Red", "redraw"), (480, "Yellow", "redraw"), (960, "Blue", "redraw"),
            (1440, "Red", "copy"), (1440, "Yellow", "redraw"),
            (1920, "Red", "redraw"), (1920, "Yellow", "copy")]
    assert br == want, br
    emit_case(w, "copy rule", copy, 1, codes)

    # Colour block: a chord whose first note lands on a yellow tom, where a
    # later draw for that chord is the yellow cymbal. The real reference refuses
    # it. The same reference blocking only the exact lane drawn (no tom and
    # cymbal pairing) takes it, and that chord is the first place the two runs
    # differ, so the pairing is what changed the result.
    def lanes_by_tick(seq, pads):
        notes, bt = build(seq)
        ref.pads = pads
        try:
            ref.shuffle(notes, 9, lambda n: bt[n.tick])
        finally:
            ref.pads = _pads
        return [sorted(o.mask for o in bt[t]) for t, _ in seq]

    found = None
    for base in range(480, 480 + 20000, 7):
        seq = [(base, [N("Red"), N("Blue")]), (base + 240, [N("Red"), N("Blue")]),
               (base + 480, [N("Red"), N("Green")]), (base + 720, [N("Red"), N("Blue")]),
               (base + 960, [N("Red"), N("Green")]), (base + 1200, [N("Red"), N("Blue")])]
        try:
            real = lanes_by_tick(seq, _pads)
            loose = lanes_by_tick(seq, lambda m: m & 0xFE)
        except RuntimeError:
            continue
        diff = next((i for i in range(len(seq)) if real[i] != loose[i]), None)
        if diff is not None and 4 in loose[diff] and 32 in loose[diff] and 4 in real[diff]:
            found = (seq, seq[diff][0])
            break
    assert found, "no colour-block case found"
    seq, t_block = found
    codes, _ = run(seq, 1)
    emit_case(w, f"colour block at tick {t_block} (exact-lane blocking would put a yellow tom and"
                 " a yellow cymbal there)", seq, 1, codes)

    # Pro Drums off draws only the four toms.
    toms = [(960 + 120 * i, [N(c) for c in cs]) for i, cs in enumerate(
        [["Kick", "Yellow"], ["Yellow"], ["Red"], ["Yellow"], ["Kick", "Yellow"], ["Kick"],
         ["Red"], ["Yellow"], ["Kick", "Green"], ["Blue"], ["Red", "Blue", "Green"], ["Green"],
         ["Kick", "Yellow"], ["Blue"], ["Red"], ["Kick", "Green"]])]
    codes, _ = run(toms, 0)
    assert all(not ch.isupper() for _, c in codes for ch in c[1:]), codes
    emit_case(w, "Pro Drums off", toms, 0, codes)

    # Ghost and accent ride along with each moved note; the 2x mark stays on the kick.
    dyn = [(480, [N("Kick", is2x=True), N("Red", dyn="ghost"), N("Yellow", True, "accent")]),
           (960, [N("Blue", dyn="accent")]), (1440, [N("Kick"), N("Green", dyn="ghost")]),
           (1920, [N("Red", dyn="accent"), N("Green", True)])]
    codes, _ = run(dyn, 1)
    emit_case(w, "dynamics ride along", dyn, 1, codes)

    # A seed that wraps 64 bits: the first four notes' lane x tick sums past 2^64.
    t0 = (1 << 62) + 12345
    wrap = [(t0, [N("Green", True)]), (t0 + 480, [N("Green", True)]),
            (t0 + 960, [N("Blue", True)]), (t0 + 1440, [N("Yellow", True)]),
            (t0 + 1920, [N("Red")]), (t0 + 2400, [N("Red"), N("Green", True)])]
    raw = sum(o.orig * o.tick for o in build(wrap)[0][:4])
    assert raw >= 1 << 64, raw
    codes, _ = run(wrap, 1)
    emit_case(w, f"seed wraps (raw sum {raw}, wrapped {raw & ref.M64})", wrap, 1, codes)

    # A first chord on tick 0 with a non-zero seed: the reference's tick
    # bookkeeping starts at 0, so tick 0's notes all take the same-chord path.
    zero = [(0, [N("Kick"), N("Red"), N("Blue")]), (480, [N("Yellow", True)]),
            (960, [N("Red")]), (1440, [N("Yellow", True)]), (1920, [N("Red"), N("Yellow", True)])]
    codes, _ = run(zero, 1)
    assert codes is not None
    emit_case(w, "first chord on tick 0, seed not zero", zero, 1, codes)

    # The gameplay video's chart. Its file is not in the repo; verify/check_video.py
    # holds its notes (from the video scout's MIDI dump) and what the video showed.
    # Loading that script runs it, so its printout is swallowed here.
    argv, out = sys.argv, sys.stdout
    sys.argv = ["check_video.py", str(NS / "scout-strings"), str(NS / "scout-flag")]
    sys.stdout = io.StringIO()
    try:
        cv = load(NS / "verify/check_video.py", "check_video")
    finally:
        sys.argv, sys.stdout = argv, out
    video = [(t, [(BIT_TO_LANE[m][0], BIT_TO_LANE[m][1], "none", False) for m in lanes])
             for t, lanes in cv.chart]
    codes, _ = run(video, 1)
    for t, c in codes:
        got = set()
        for i, ch in enumerate(c):
            if ch != ".":
                colour = LANE_ORDER[i]
                cym = ch.isupper() and colour != "Kick"
                got.add(CYM[colour] if cym else TOM[colour])
        assert got == cv.seen[t], (t, c, cv.seen[t])
    emit_case(w, "gameplay video chart (every tick matches the video)", video, 1, codes)


def main():
    buf = io.StringIO()

    def w(s):
        print(s)
        buf.write(s + "\n")

    w("// ---- game-test songs (hydra_replay score -> reference)")
    emit_songs(w)
    w("// ---- made-up sequences")
    emit_synthetic(w)
    (HERE / "vectors_output.txt").write_text(buf.getvalue(), encoding="utf-8")


if __name__ == "__main__":
    main()
