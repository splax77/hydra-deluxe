"""Reference re-implementation of Clone Hero v1.1.0.6142 Note Shuffle, transcribed from
GameAssembly.dll RVA 0x20F0C30 (shuffle), 0x20F0B30 (lane picker), 0x20EA810 (RNG seed ctor),
0x214BCD0 (chord mask). Unverified against the game; read from disassembly only.

A note here is a dict: {"tick": int, "lanes": int (bitmask), "ext_sustain": bool, "chord": [notes at same chord]}.
Drum lane bits: 1 kick, 2 R, 4 Y tom, 8 B tom, 16 G tom, 32 Y cym, 64 B cym, 128 G cym.
"""
M64 = (1 << 64) - 1

# instrument enum: 0 Guitar 1 Bass 2 Rhythm 3 GuitarCoop 4 SixFretGuitar 5 SixFretBass 6 Drums
# 7 Keys 8 Band 9 ProDrums 10 SixFretRhythm 11 SixFretGuitarCoop 12 Crowd 13 Vocals 14 SixFretKeys
RANGE = {0: 6, 1: 6, 2: 6, 3: 6, 7: 6, 4: 7, 5: 7, 10: 7, 11: 7, 14: 7, 6: 5, 9: 8, 8: 1, 12: 1, 13: 1}
SKIP = {8: 0, 12: 0, 13: 0}  # everything else listed in RANGE skips lane bit 0 (open / kick)


class Rng:
    def __init__(self, seed):
        self.a = (seed << 3) & M64
        self.b = seed >> 3

    def next31(self):
        a, b = self.a, self.b
        t = (a ^ (a << 23)) & M64
        nb = t ^ b ^ (t >> 17) ^ (b >> 26)
        self.a, self.b = b, nb
        return ((b + nb) & 0xFFFFFFFF) & 0x7FFFFFFF


def pick(rng, k, rng_range):
    """Lane picker lambda: k distinct single bits from 1..rng_range-1; duplicates consume draws."""
    mask = 0
    count = 0
    while count < k:
        v = rng.next31()
        bit = 1 << ((((v * (rng_range - 1)) >> 31) + 1) & 31)
        if not (mask & bit):
            count += 1
            mask |= bit
    return mask & 0xFFFF


def chord_mask(note):
    m = 0
    for n in note["chord"]:
        m |= n["lanes"]
    return m


def shuffle(notes, instrument):
    n = max(0, min(len(notes), 4))
    seed = 0
    for i in range(n):
        seed = (seed + notes[i]["lanes"] * notes[i]["tick"]) & M64
    rng = Rng(seed)
    drums = instrument in (6, 9)
    rng_range = RANGE.get(instrument, 1)
    skip = SKIP.get(instrument, 1) if instrument in RANGE else 0
    full = ((1 << rng_range) - 1) ^ skip
    prev_tick = 0
    used = 0       # lanes output so far at the current tick
    prev_out = 0   # drums: lanes output at the previous tick (see report for the one-tick lag)
    shape_cur = 0
    shape_prev = 0
    for note in notes:
        lanes = note["lanes"]
        if lanes & skip:
            continue
        if not drums:
            if note["ext_sustain"] or lanes == full:
                continue
        k = bin(lanes).count("1")
        if drums and note["tick"] == prev_tick:
            shape = shape_cur
        else:
            shape = chord_mask(note) & 0xFE
        out = None
        if drums and shape_prev != 0 and shape_prev == shape:
            # repeat path: same rank inside the old output
            bits = [1 << i for i in range(16) if shape & (1 << i)]
            idx = next((i for i, b in enumerate(bits) if b == lanes), None)
            out = 0
            if idx is not None:
                pbits = [1 << i for i in range(16) if prev_out & (1 << i)]
                out = pbits[idx] if idx < len(pbits) else 0
        else:
            while True:
                out = pick(rng, k, rng_range)
                if not drums:
                    if out == prev_out:
                        continue
                    break
                x = ((((used >> 3) & 0xFFFD) | used) & 0x1E)
                blocked = ((x & 0x1C) << 3) | x
                if out & blocked:
                    continue
                break
            if not drums:
                prev_out = out
                note["lanes"] = out
                prev_tick = note["tick"]
                continue
        if note["tick"] == prev_tick:
            used |= out
        else:
            prev_out = used
            used = out
            shape_prev = shape_cur
            shape_cur = shape
        note["lanes"] = out
        prev_tick = note["tick"]
    return notes


if __name__ == "__main__":
    # toy pro-drums chart: kick+snare, hihat, kick+snare, hihat ...
    notes = []
    for i in range(8):
        t = 480 + 240 * i
        group = [{"tick": t, "lanes": 1}, {"tick": t, "lanes": 2}] if i % 2 == 0 else [{"tick": t, "lanes": 32}]
        for g in group:
            g["ext_sustain"] = False
            g["chord"] = group
        notes += group
    shuffle(notes, 9)
    print([(n["tick"], n["lanes"]) for n in notes])
