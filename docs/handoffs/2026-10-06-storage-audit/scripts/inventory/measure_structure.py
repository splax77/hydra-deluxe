"""Measure the structure blob and a sample of node payloads in the installed DB copy.

Read-only. Decodes path format 7 by hand (src/store/path_codec.cpp).
"""
import sqlite3, struct, sys, collections

DB = r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\3dbe8199-05ae-4b84-8477-f6bbc98854d5\scratchpad\db\hydra.db"


class R:
    def __init__(self, b):
        self.b = b
        self.p = 0

    def u8(self):
        v = self.b[self.p]; self.p += 1; return v

    def u32(self):
        v = struct.unpack_from("<I", self.b, self.p)[0]; self.p += 4; return v

    def i32(self):
        v = struct.unpack_from("<i", self.b, self.p)[0]; self.p += 4; return v

    def u64(self):
        v = struct.unpack_from("<Q", self.b, self.p)[0]; self.p += 8; return v

    def i64(self):
        v = struct.unpack_from("<q", self.b, self.p)[0]; self.p += 8; return v

    def f64(self):
        v = struct.unpack_from("<d", self.b, self.p)[0]; self.p += 8; return v

    def s(self):
        n = self.u32(); v = self.b[self.p:self.p + n].decode("latin1"); self.p += n; return v

    def opt(self, fn):
        if self.u8():
            return fn()
        return None


S = collections.Counter()  # structure byte buckets
N = collections.Counter()  # counts


def read_tree_entry(r, depth):
    r.p += 16  # hash
    S["hash"] += 16
    nvar = r.u32(); S["nvar_count"] += 4
    N["nodes_referenced"] += 1
    if depth > 0:
        N["variants"] += 1
    for _ in range(nvar):
        before = r.p
        r.opt(r.i32)
        ntrail = r.u32()
        for _ in range(ntrail):
            r.i64()
        S["variant_varpoint_trail"] += r.p - before
        N["variant_trailing_ticks"] += ntrail
        read_tree_entry(r, depth + 1)


def read_root_totals(r):
    before = r.p
    for _ in range(6):
        r.i64()
    r.i32()
    S["root_scores_notecount"] += r.p - before
    before = r.p
    ntrail = r.u32()
    for _ in range(ntrail):
        r.i64()
    S["root_trailing_bank"] += r.p - before
    N["root_trailing_ticks"] += ntrail


def structure(blob):
    r = R(blob)
    fmt = r.u32(); fp = r.u64()
    S["head_format_rulesfp"] += 12
    if fmt != 7:
        N["structure_not_format_7"] += 1
        S["non7_rest"] += len(blob) - 12
        return fmt, fp, None
    before = r.p
    ms = r.opt(r.f64); cap = r.opt(r.i32); conv = r.u8()
    S["ms_limit_sp_cap_converged"] += r.p - before
    N["ms_limit_set"] += ms is not None
    N["converged_true"] += conv
    before = r.p
    nmsq = r.u32()
    for _ in range(nmsq):
        r.s(); r.i32()
    S["multsqueezes"] += r.p - before
    N["multsqueezes"] += nmsq
    nroots = r.u32(); S["root_count"] += 4
    N["roots"] += nroots
    for _ in range(nroots):
        read_tree_entry(r, 0)
        read_root_totals(r)
    nzero = r.u32(); S["allzero_count"] += 4
    N["allzero_roots"] += nzero
    for _ in range(nzero):
        read_tree_entry(r, 0)
        read_root_totals(r)
    assert r.p == len(blob), (r.p, len(blob))
    return fmt, fp, cap


PN = collections.Counter()


def node(payload):
    r = R(payload)
    fmt = r.u32()
    if fmt != 7:
        PN["node_not_format_7"] += 1
        return
    nact = r.u32()
    PN["activations"] += nact
    for _ in range(nact):
        r.i64(); r.s(); r.i32()
        nb = r.u32()
        PN["backends"] += nb
        for _ in range(nb):
            r.i64(); r.s(); r.i32(); r.i32()
            off = r.opt(r.f64)
            if off is None:
                PN["backend_offset_missing"] += 1
        nsq = r.u32()
        PN["sqins"] += nsq
        for _ in range(nsq):
            r.f64()
            sc = r.opt(lambda: (r.f64(), r.f64()))
            if sc is None:
                PN["sqin_scale_unknown"] += 1
            elif sc == (1.0, 1.0):
                PN["sqin_scale_identity"] += 1
        e = r.f64()
        if e > 60.0:
            PN["e_offset_over_window(no E)"] += 1
        post = r.opt(lambda: (r.f64(), r.f64()))
        if post is None:
            PN["post_scale_unknown"] += 1
        elif post == (1.0, 1.0):
            PN["post_scale_identity"] += 1
        sq = r.opt(r.i64)
        if sq is not None:
            PN["sqout_ticks"] += 1
        nst = r.u32()
        PN["sp_end_steps"] += nst
        kinds = []
        for _ in range(nst):
            r.i64(); r.i64(); kinds.append(r.u8())
        for k in kinds:
            PN["step_kind_%d" % k] += 1
        if nst == 1:
            PN["acts_with_only_activation_step"] += 1
        nbank = r.u32(); PN["bank_ticks"] += nbank
        for _ in range(nbank):
            r.i64()
        nf = r.u32(); PN["skipped_fill_ticks"] += nf
        for _ in range(nf):
            r.i64()
        if nf == 0:
            PN["acts_with_zero_skips"] += 1
    assert r.p == len(payload)


con = sqlite3.connect("file:%s?mode=ro" % DB.replace("\\", "/"), uri=True)
cur = con.cursor()
rows = cur.execute("SELECT structure, bestpath, hyversion, score, sp_cap, legacy_fills FROM results").fetchall()
tot_struct = sum(len(r[0]) for r in rows)
tot_best = sum(len(r[1]) for r in rows)
vers = collections.Counter(r[2] for r in rows)
nopaths = sum(1 for r in rows if r[3] is None)
caps = collections.Counter(r[4] for r in rows)
fmts = collections.Counter(); fps = collections.Counter()
for r in rows:
    fmt, fp, cap = structure(r[0])
    fmts[fmt] += 1; fps[fp] += 1
print("results rows", len(rows))
print("structure bytes total", tot_struct, "avg", tot_struct / len(rows))
print("bestpath bytes total", tot_best)
print("hyversion", dict(vers))
print("formats", dict(fmts), "rules_fp distinct", len(fps))
print("rows with no paths (score NULL)", nopaths)
print("sp_cap", dict(caps))
print("STRUCTURE BUCKETS", dict(S))
print("STRUCTURE COUNTS", dict(N))

# summary columns' bytes: estimate via SQLite typeof/length
cols = ["score", "actcount", "maxskip", "hardest_ms", "avgmult", "notecount", "sqin_count", "sqout_count", "pathcount", "stars", "rules_fp", "hyversion", "hyhash", "chartmode"]
for c in cols:
    n = cur.execute("SELECT SUM(LENGTH(%s)), COUNT(%s) FROM results" % (c, c)).fetchone()
    print("col", c, n)

# node payload stats over all nodes
cnt = 0
for (payload,) in cur.execute("SELECT payload FROM paths"):
    node(payload); cnt += 1
print("nodes decoded", cnt)
print("NODE COUNTS", dict(PN))
