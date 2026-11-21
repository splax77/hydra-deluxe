"""Check the script-first recheck's pass bar, B1 to B8, and print one line per bar.

    py -I tools/blind_copy/check_bars.py --folder <folder> --copy <copy>
        [--bars B1,B2,...] [--seams ...] [--coverage ...] [--hits ...] [--joined ...]
        [--kinds-md ...] [--matched ...] [--unmatched ...] [--questions ...]
        [--answers ...] [--records <folder>] [--repo <repo>]

Each file defaults to its name inside --folder. The bars are "The pass bar"
in the script-first recheck plan (docs/superpowers/plans/
2026-11-20-script-first-recheck.md). The outcome and answer words are
kinds.py's; the record states are match_records.py's. It exits 1 when any
bar it checks fails, and 2 when it cannot run.

questions.md, which B4 and B6 read, is plain text in this shape:

  One section per kind of flaw, headed `## ` and the kind's name exactly as
  kinds.OUTCOMES spells it. There are four such sections and no other `## `
  heading. Inside each section:
    tests: <the kind's count of tests, as kinds.md gives it>
    flagged records: <the kind's count of flagged records, as kinds.md gives it>
    example code: <path under code/>:<line> | <text quoted from that line>
    example record: <records/NAME.md>:<line> | <text quoted from that line>
      (two or three of each, an example's code line followed by its record line)
    unsorted: <path>:<line>
      (one line per unsorted hit nearest this kind; every unsorted hit of
      hits.tsv is named in exactly one section)
  Anywhere in the file, one line per seam name coverage.tsv marks uncovered:
    uncovered: <name>
  Any `status:` line says `open`.

A quote is found when its text, with runs of spaces made one, is inside the
cited line. A code line is read from <copy>/code; a record line from
--records/<NAME>.md, or else from git at match_records.RECORDS_COMMIT in --repo.
"""

import argparse
import collections
import csv
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import kinds as k  # noqa: E402
import match_records as mr  # noqa: E402

BARS = ("B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8")
JOIN_STATES = (mr.FLAGGED, mr.SOMETHING_ELSE, k.NO_RECORD)
CLAIM_WORDS = ("by design", "on purpose", "deliberately", "kept")
# The plan's B2: the seven update tests of records/C13.md lines 551-664.
B2_FILE, B2_FIRST, B2_LAST, B2_COUNT = "C13.md", 551, 664, 7
FILES = {"seams": "seams.tsv", "coverage": "coverage.tsv", "hits": "hits.tsv", "joined": "joined.tsv",
         "kinds_md": "kinds.md", "matched": "matched.tsv", "unmatched": "unmatched.tsv",
         "questions": "questions.md", "answers": "answers.tsv"}


class Bad(Exception):
    pass


def table(path, need):
    if not path or not os.path.isfile(path):
        raise Bad(f"no file {path}")
    with open(path, encoding="utf-8", newline="") as f:
        rows = list(csv.DictReader(f, delimiter="\t", quoting=csv.QUOTE_NONE))
        f.seek(0)
        cols = rows[0].keys() if rows else f.readline().rstrip("\r\n").split("\t")
    missing = [c for c in need if c not in cols]
    if missing:
        raise Bad(f"{os.path.basename(path)} has no column {', '.join(missing)}")
    return rows


def text(path):
    if not path or not os.path.isfile(path):
        raise Bad(f"no file {path}")
    with open(path, encoding="utf-8", errors="replace") as f:
        return f.read()


def sections(md):
    """`## ` heading -> its lines."""
    out, cur = collections.OrderedDict(), None
    for line in md.split("\n"):
        if line.startswith("## "):
            cur = line[3:].strip()
            if cur in out:
                out[cur + " (again)"] = []
                cur = cur + " (again)"
            else:
                out[cur] = []
        elif cur is not None:
            out[cur].append(line)
    return out


def field(lines, name):
    vals = [ln.split(":", 1)[1].strip() for ln in lines if ln.startswith(name + ":")]
    return vals


def tests_with_hits(seams):
    return {(s["test"], s["test_file"]) for s in seams if s["test_case"] == "yes"}


def b1(p):
    seams = table(p["seams"], ("test", "test_file", "test_case"))
    joined = table(p["joined"], ("test", "test file", "state"))
    count = collections.Counter((j["test"], j["test file"]) for j in joined)
    bad = []
    for t in sorted(tests_with_hits(seams)):
        if count[t] != 1:
            bad.append(f"{t[0]} ({t[1]}) has {count[t]} rows")
    for j in joined:
        if j["state"] not in JOIN_STATES:
            bad.append(f"{j['test']} is in the state {j['state']!r}")
    if bad:
        return False, "; ".join(bad[:10]) + (f"; and {len(bad) - 10} more" if len(bad) > 10 else "")
    return True, f"{len(tests_with_hits(seams))} tests with a hit, one row each"


def b2(p):
    matched = table(p["matched"], ("record", "test name", "test file"))
    joined = table(p["joined"], ("test", "test file", "state"))
    seven = []
    for m in matched:
        name, _, line = m["record"].rpartition(":")
        if os.path.basename(name) == B2_FILE and line.isdigit() and B2_FIRST <= int(line) <= B2_LAST \
                and m["test name"]:
            t = (m["test name"], m["test file"].removeprefix("code/"))
            if t not in seven:
                seven.append(t)
    if len(seven) != B2_COUNT:
        return False, f"{B2_FILE} lines {B2_FIRST}-{B2_LAST} name {len(seven)} tests, not {B2_COUNT}"
    state = {(j["test"], j["test file"]): j["state"] for j in joined}
    bad = [f"{t[0]} is {state.get(t, 'not joined')}" for t in seven if state.get(t) != mr.FLAGGED]
    if bad:
        return False, "; ".join(bad)
    return True, f"all {B2_COUNT} update tests are flagged"


def b3(p):
    seams = table(p["seams"], ("hit_file", "hit_line"))
    hits = table(p["hits"], ("hit", "via", "outcome"))
    bad = []
    keys = collections.Counter((h["hit"], h["via"]) for h in hits)
    bad += [f"{h} via {v or 'nothing'} has {n} outcomes" for (h, v), n in keys.items() if n > 1]
    bad += [f"{h['hit']} has the outcome {h['outcome']!r}, which is not one of the six"
            for h in hits if h["outcome"] not in k.OUTCOMES]
    have = {h["hit"] for h in hits}
    lines = {f"{s['hit_file']}:{s['hit_line']}" for s in seams}
    bad += [f"{h} has no outcome" for h in sorted(lines - have)]
    counts = collections.Counter(h["outcome"] for h in hits)
    total = sum(counts[o] for o in k.OUTCOMES)
    if total != len(hits):
        bad.append(f"the six counts add up to {total}, not the {len(hits)} rows")
    if p.get("kinds_md") and os.path.isfile(p["kinds_md"]):
        heads = list(sections(text(p["kinds_md"])))
        extra = [h for h in heads if h not in k.OUTCOMES]
        bad += [f"kinds.md has a section {h!r}, which splits or renames a kind" for h in extra]
    if bad:
        return False, "; ".join(bad[:10]) + (f"; and {len(bad) - 10} more" if len(bad) > 10 else "")
    return True, (f"{len(lines)} hit lines in {len(hits)} rows: " +
                  ", ".join(f"{o} {counts[o]}" for o in k.OUTCOMES))


class Lines:
    def __init__(self, copy, records, repo):
        self.copy, self.records, self.repo = copy, records, repo
        self.cache = {}

    def get(self, path, line):
        if path not in self.cache:
            self.cache[path] = self.read(path)
        got = self.cache[path]
        if got is None or not (1 <= line <= len(got)):
            return None
        return got[line - 1]

    def read(self, path):
        base = os.path.basename(path)
        if path.startswith("records/") or (path.endswith(".md") and "/" not in path):
            if self.records:
                real = os.path.join(self.records, base)
                return text(real).split("\n") if os.path.isfile(real) else None
            if self.repo:
                out = subprocess.run(["git", "-C", self.repo, "show",
                                      f"{mr.RECORDS_COMMIT}:{mr.RECORDS_DIR}{base}"],
                                     capture_output=True, text=True, encoding="utf-8")
                return out.stdout.split("\n") if out.returncode == 0 else None
            return None
        real = os.path.join(self.copy or "", "code", *path.removeprefix("code/").split("/"))
        return text(real).split("\n") if os.path.isfile(real) else None


def squash(s):
    return " ".join(s.split())


def b4(p):
    q = sections(text(p["questions"]))
    km = sections(text(p["kinds_md"])) if p.get("kinds_md") and os.path.isfile(p["kinds_md"]) else {}
    bad = []
    if list(q) != [x for x in q if x in k.KINDS] or sorted(q) != sorted(k.KINDS):
        bad.append(f"the questions are {list(q)}, not one per kind of flaw, four in all")
    lines = Lines(p.get("copy"), p.get("records"), p.get("repo"))
    for name, body in q.items():
        if name not in k.KINDS:
            continue
        for f in ("tests", "flagged records"):
            got = field(body, f)
            if len(got) != 1 or not got[0].isdigit():
                bad.append(f"{name} gives no single count of {f}")
            elif name in km and field(km[name], f) and field(km[name], f)[0] != got[0]:
                bad.append(f"{name} gives {f} {got[0]}, but kinds.md gives {field(km[name], f)[0]}")
        code, rec = field(body, "example code"), field(body, "example record")
        if not (2 <= len(code) <= 3) or len(rec) != len(code):
            bad.append(f"{name} has {len(code)} code examples and {len(rec)} record examples, "
                       f"not two or three of each")
        for ex in code + rec:
            where, _, quote = ex.partition("|")
            path, _, ln = where.strip().rpartition(":")
            got = lines.get(path, int(ln)) if ln.isdigit() else None
            if got is None or not quote.strip() or squash(quote) not in squash(got):
                bad.append(f"{name}: the quote {quote.strip()[:60]!r} is not at {where.strip()}")
    if bad:
        return False, "; ".join(bad[:10]) + (f"; and {len(bad) - 10} more" if len(bad) > 10 else "")
    return True, "four questions, each with its counts and quoted examples"


def b5(p):
    bad = []
    for key in ("hits", "joined"):
        for r in table(p[key], ("status",)):
            s = r["status"].strip()
            if s != k.OPEN:
                word = next((w for w in CLAIM_WORDS if w in s.lower()), None)
                bad.append(f"{FILES[key]} has the status {s!r}" + (f", a claim word ({word})" if word else ""))
    for key in ("kinds_md", "questions"):
        if p.get(key) and os.path.isfile(p[key]):
            for n, line in enumerate(text(p[key]).split("\n"), 1):
                if line.lower().startswith("status:") and line.split(":", 1)[1].strip() != k.OPEN:
                    bad.append(f"{FILES[key]}:{n} has the status {line.split(':', 1)[1].strip()!r}")
    if bad:
        return False, "; ".join(bad[:10])
    return True, "every row is open"


def b6(p):
    cov = table(p["coverage"], ("name", "state"))
    hits = table(p["hits"], ("hit", "outcome"))
    md = text(p["questions"])
    listed = set(field(md.split("\n"), "uncovered"))
    bad = [f"the uncovered seam name {c['name']} is not listed" for c in cov
           if c["state"] == "uncovered" and c["name"] not in listed]
    per = collections.Counter()
    for name, body in sections(md).items():
        for h in set(field(body, "unsorted")):
            per[h] += 1
    for h in sorted({r["hit"] for r in hits if r["outcome"] == k.UNSORTED}):
        if per[h] != 1:
            bad.append(f"the unsorted hit {h} is named in {per[h]} questions, not one")
    if bad:
        return False, "; ".join(bad[:10]) + (f"; and {len(bad) - 10} more" if len(bad) > 10 else "")
    return True, "every uncovered name and unsorted hit is named"


def b7(p):
    matched = table(p["matched"], ("record", "match"))
    unmatched = table(p["unmatched"], ("record", "match"))
    joined_states = (mr.SAME_PLACE, mr.MOVED)
    un = collections.Counter(u["record"] for u in unmatched)
    bad = []
    joined = [m for m in matched if m["match"] in joined_states]
    for m in matched:
        if m["match"] in joined_states and un[m["record"]]:
            bad.append(f"{m['record']} is both joined and unmatched")
        if m["match"] not in joined_states and un[m["record"]] != 1:
            bad.append(f"{m['record']} is in neither file")
    for u in unmatched:
        if not u["match"]:
            bad.append(f"{u['record']} is unmatched with no state")
    if len(joined) + len(unmatched) != len(matched):
        bad.append(f"{len(joined)} joined and {len(unmatched)} unmatched do not add up to {len(matched)} records")
    if bad:
        return False, "; ".join(bad[:10])
    return True, f"{len(joined)} joined and {len(unmatched)} unmatched make {len(matched)} records"


def b8(p):
    joined = table(p["joined"], ("test", "state", "result", "kinds"))
    answers = k.read_answers(p["answers"]) if p.get("answers") and os.path.isfile(p["answers"]) else {}
    bad = []
    for j in joined:
        kinds = [x for x in j["kinds"].split(k.SEP.strip()) if x.strip()]
        kinds = [x.strip() for x in kinds]
        want = k.result(j["state"], kinds, j["test"], answers)
        if j["result"] == k.CONTRADICTED and want != k.CONTRADICTED:
            bad.append(f"{j['test']} is contradicted with no answer that makes it so")
        elif want == k.CONTRADICTED and j["result"] != k.CONTRADICTED:
            bad.append(f"{j['test']} is {j['result']}, but its answers make it contradicted")
    if bad:
        return False, "; ".join(bad[:10])
    c = sum(1 for j in joined if j["result"] == k.CONTRADICTED)
    return True, f"{c} contradicted, each through an answer"


CHECKS = {"B1": b1, "B2": b2, "B3": b3, "B4": b4, "B5": b5, "B6": b6, "B7": b7, "B8": b8}


def check(paths, bars=BARS):
    """{bar: (passed, line)}."""
    out = {}
    for b in bars:
        try:
            out[b] = CHECKS[b](paths)
        except (Bad, ValueError, KeyError, OSError) as e:
            out[b] = (False, str(e))
    return out


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter, epilog=__doc__)
    ap.add_argument("--folder", default=".")
    ap.add_argument("--copy")
    ap.add_argument("--records")
    ap.add_argument("--repo")
    ap.add_argument("--bars", default=",".join(BARS))
    for key in FILES:
        ap.add_argument("--" + key.replace("_", "-"))
    a = ap.parse_args(argv)
    paths = {key: getattr(a, key) or os.path.join(a.folder, name) for key, name in FILES.items()}
    paths.update(copy=a.copy, records=a.records, repo=a.repo)
    bars = [b.strip() for b in a.bars.split(",") if b.strip()]
    if any(b not in CHECKS for b in bars):
        sys.stderr.write(f"check_bars: bars are {', '.join(BARS)}\n")
        return 2
    got = check(paths, bars)
    for b in bars:
        ok, line = got[b]
        print(f"{b} {'pass' if ok else 'FAIL'}: {line}")
    return 0 if all(ok for ok, _ in got.values()) else 1


if __name__ == "__main__":
    sys.exit(main())
