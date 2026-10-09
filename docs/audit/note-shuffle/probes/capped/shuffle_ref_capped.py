"""Reference re-implementation of Clone Hero 1.1.0.6142 Note Shuffle, read from GameAssembly.dll.

Source: 0x20F0C30 (shuffle), 0x20F0B30 (lane draw), 0x214BCD0 (chord pattern),
0x20FC610/0x20FC5F0 (bit enumerator), jump tables at 0x20F11CC / 0x20F1208.
Not verified against the running game.

A note here is one play note: tick (chart ticks, note+0x40), mask (lane bits, note+0x80),
flags (note+0x7C), chord (list of every note object chained to it via +0x10/+0x18,
including itself). Drum lane bits: kick 0x1, red 0x2, Y tom 0x4, B tom 0x8, G tom 0x10,
Y cym 0x20, B cym 0x40, G cym 0x80. Each drum note object holds one lane.
"""
M64 = (1 << 64) - 1

# instrument enum (dump.cs 533188): Guitar 0, Bass 1, Rhythm 2, GuitarCoop 3, SixFretGuitar 4,
# SixFretBass 5, Drums 6, Keys 7, Band 8, ProDrums 9, SixFretRhythm 10, SixFretGuitarCoop 11,
# Crowd 12, Vocals 13, SixFretKeys 14
LANES_N = {0: 6, 1: 6, 2: 6, 3: 6, 7: 6, 4: 7, 5: 7, 10: 7, 11: 7, 14: 7, 6: 5, 9: 8}  # else 1
EXCLUDED = {8: 0, 12: 0, 13: 0}  # else 1


def popcount(x):
    return bin(x).count("1")


def bits_low_to_high(x):
    while x:
        low = x & -x
        yield low
        x ^= low


def pads(m):
    """Lanes blocked by the lanes in m, in both tom and cymbal form (0x20F1106-0x20F112E)."""
    cx = ((((m >> 3) & 0xFFFD) | m) & 0x1E) & 0xFFFF
    return (cx | ((cx & 0x1C) << 3)) & 0xFFFF


def shuffle(notes, instrument, chord_of):
    """notes: list in game order (mutated). chord_of(note) -> list of notes in its chord."""
    is_drums = instrument in (6, 9)
    n_lanes = LANES_N.get(instrument, 1)
    excluded = EXCLUDED.get(instrument, 1)
    full = (((1 << n_lanes) - 1) & 0xFFFF) ^ excluded

    seed = 0
    for nt in notes[:min(len(notes), 4)]:
        seed = (seed + nt.mask * nt.tick) & M64
    s = [(seed << 3) & M64, seed >> 3]  # rng+0x10, rng+0x18

    def lane_index():  # xorshift128+ step; btr edx,31 keeps the low 31 bits of the sum
        x, y = s[0], s[1]
        x ^= (x << 23) & M64
        s1 = x ^ y ^ (x >> 17) ^ (y >> 26)
        s[0], s[1] = y, s1
        r = ((y + s1) & M64) & 0x7FFFFFFF
        return ((r * (n_lanes - 1)) >> 31) + 1      # == trunc(r * 2^-31 * (N-1)) + 1, exact

    def draw(count):
        m, k = 0, 0
        guard = 0
        while k < count:
            guard += 1
            if guard > 100000: raise RuntimeError('hang')
            b = (1 << (lane_index() & 31)) & 0xFFFF
            if not (m & b):
                m |= b
                k += 1
        return m

    prev_tick = 0      # r13 / [rsp+0x58], both start at 0
    cur_pat = 0        # [rsp+0x24]
    prev_pat = 0       # [rsp+0x28]
    cur_out = 0        # [rsp+0xd0]
    prev_out = 0       # [rsp+0xe8]
    for nt in notes:
        if nt.mask & excluded:
            continue
        if not is_drums and ((nt.flags & 8) or nt.mask == full):
            continue
        cnt = popcount(nt.mask)
        if is_drums and nt.tick == prev_tick:
            pat = cur_pat
        else:
            pat = 0
            for c in chord_of(nt):
                pat |= c.mask
            pat &= 0xFE
        if not is_drums:
            while True:
                out = draw(cnt)
                if out != prev_out:
                    break
            prev_out = out
            nt.mask = out
            prev_tick = nt.tick
            continue
        if prev_pat != 0 and prev_pat == pat:
            out = 0
            idx = next((i for i, b in enumerate(bits_low_to_high(pat)) if b == nt.mask), None)
            if idx is not None:
                src = list(bits_low_to_high(prev_out))
                out = src[idx] if idx < len(src) else 0
        else:
            used = pads(cur_out)
            tries = 0
            while True:  # never ends if every lane is blocked (see report)
                tries += 1
                if tries > 10000: raise RuntimeError('hang')
                out = draw(cnt)
                if not (out & used):
                    break
        if nt.tick == prev_tick:
            cur_out |= out
        else:
            prev_out, cur_out = cur_out, out
            prev_pat, cur_pat = cur_pat, pat
        nt.mask = out
        prev_tick = nt.tick


if __name__ == "__main__":
    class N:
        def __init__(s, tick, mask):
            s.tick, s.mask, s.flags = tick, mask, 0
    ns = [N(480, 1), N(480, 2), N(600, 2), N(720, 2), N(840, 2), N(960, 4), N(960, 0x40)]
    chords = {}
    for n in ns:
        chords.setdefault(n.tick, []).append(n)
    shuffle(ns, 9, lambda n: chords[n.tick])
    print([(n.tick, hex(n.mask)) for n in ns])
