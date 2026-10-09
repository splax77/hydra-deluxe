"""Does the Note Shuffle generator reach every pad from any non-zero state?

Generator: the step in docs/audit/note-shuffle/probes/capped/shuffle_ref_capped.py (lane_index).
Part 1 proves full period (state cycle covers all 2^128-1 non-zero states) via the GF(2) order test.
Part 2 measures, from real-style seeds, how many draws it takes to see every lane.
"""
import random, sys, time
M64 = (1 << 64) - 1

def step(s0, s1):
    x, y = s0, s1
    x ^= (x << 23) & M64
    n1 = x ^ y ^ (x >> 17) ^ (y >> 26)
    return y, n1

# ---- Part 1: linear map on 128-bit state (low 64 = s0, high 64 = s1)
def T(v):
    a, b = step(v & M64, v >> 64)
    return a | (b << 64)

N = 128
cols = [T(1 << i) for i in range(N)]

def apply(A, v):
    r = 0
    i = 0
    while v:
        if v & 1:
            r ^= A[i]
        v >>= 1
        i += 1
    return r

def compose(A, B):  # A after B
    return [apply(A, c) for c in B]

I = [1 << i for i in range(N)]

def mpow(A, e):
    R = I
    while e:
        if e & 1:
            R = compose(A, R)
        A = compose(A, A)
        e >>= 1
    return R

order = (1 << 128) - 1
primes = [3, 5, 17, 257, 641, 65537, 274177, 6700417, 67280421310721]
p = 1
for q in primes:
    p *= q
assert p == order, "factorization wrong"
t = time.time()
full = mpow(cols, order) == I
print("M^(2^128-1) == I:", full, f"({time.time()-t:.1f}s)")
ok = full
for q in primes:
    r = mpow(cols, order // q) != I
    print(f"  M^((2^128-1)/{q}) != I: {r}")
    ok = ok and r
print("FULL PERIOD (every non-zero state on one cycle):", ok)
sys.stdout.flush()

# ---- Part 2: draws needed to see every lane, per instrument
def lanes_until_all(seed, n_lanes, cap=100000):
    s0, s1 = (seed << 3) & M64, seed >> 3
    want = set(range(1, n_lanes))
    seen_at = {}
    k = 0
    while len(seen_at) < len(want) and k < cap:
        s0, s1 = step(s0, s1)
        k += 1
        r = ((s0 + s1) & M64) & 0x7FFFFFFF
        lane = ((r * (n_lanes - 1)) >> 31) + 1
        seen_at.setdefault(lane, k)
    return k if len(seen_at) == len(want) else None, seen_at

seeds = list(range(1, 300001))
seeds += [1 << i for i in range(64)]
seeds += [(1 << 64) - 1, (1 << 61), 3 << 61, 7 << 61]
rng = random.Random(1)
seeds += [rng.getrandbits(64) for _ in range(200000)]
seeds += [rng.getrandbits(40) for _ in range(200000)]
for name, nl in (("ProDrums (7 pads)", 8), ("Drums (4 toms)", 5)):
    worst, worst_seed, fails = 0, None, 0
    for sd in seeds:
        k, _ = lanes_until_all(sd, nl)
        if k is None:
            fails += 1
            continue
        if k > worst:
            worst, worst_seed = k, sd
    print(f"{name}: seeds={len(seeds)} never-all={fails} worst draws to see every lane={worst} (seed {worst_seed})")
