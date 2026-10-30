"""Check every quote in PASS blocks against the files it cites.

For each block whose verdict is PASS, every double-quoted span of 15+ characters on the
"How it left" and "User's words" lines is searched in the corpus (the repo's markdown and
source, the session cards, the main checkout's handoffs). Matching ignores whitespace runs,
markdown emphasis, backticks and curly quotes. A quote that elides with "..." must have every
piece present in one file. Output, one line per quote:
  OK        found in a file the line names (by path, basename or the scout's abbreviation)
  ELSEWHERE found, but only in a file the line does not name
  MISS      found nowhere
"""
import os, re, sys, glob

REPO, OUT = sys.argv[1], sys.argv[2]
ROOTS = [
    (REPO, ["docs/**/*.md", "docs/**/*.json", "*.md", "tools/**/*.py", "src/**/*.h", "src/**/*.cpp", "tests/**/*.cpp"]),
    (r"C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\session-cards", ["**/*.md"]),
    (r"C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\handoffs", ["**/*.md"]),
    (r"C:\Users\Patrick\.claude\gates", ["**/*.md", "**/*.py", "**/*.ps1"]),
]


def norm(s):
    s = s.replace("\u2019", "'").replace("\u2018", "'").replace("\u201c", "").replace("\u201d", "").replace('"', "")
    s = re.sub(r"[*`_]", "", s)
    return re.sub(r"\s+", " ", s).strip().lower()


corpus = {}
for root, pats in ROOTS:
    for p in pats:
        for f in glob.glob(os.path.join(root, p), recursive=True):
            if f not in corpus and os.path.isfile(f):
                try:
                    corpus[f] = norm(open(f, encoding="utf-8", errors="replace").read())
                except OSError:
                    pass


def abbrevs(text):
    # "QF = `docs/...md`" style legends in the scout's header
    return {k: os.path.basename(v) for k, v in re.findall(r"\b([A-Z][A-Z0-9]{1,5}) = `([^`]+)`", text)}


for f in sorted(glob.glob(os.path.join(OUT, "*.md"))):
    text = open(f, encoding="utf-8").read()
    ab = abbrevs(text[:3000])
    for b in re.split(r"\n(?=### )", text):
        if not b.startswith("### "):
            continue
        m = re.search(r"\*\*Verdict:\*\*\s*\**\s*(PASS|REOPEN|GONE)", b)
        if not m or m.group(1) != "PASS":
            continue
        title = b.splitlines()[0][4:]
        lines = [l for l in b.splitlines() if "How it left" in l or "User's words" in l]
        n = 0
        for l in lines:
            named = set(os.path.basename(x) for x in re.findall(r"[\w./\\-]+\.(?:md|cpp|h|json|py|ps1)", l))
            named |= {ab[k] for k in ab if re.search(r"\b" + k + r"\b", l)}
            for q in l.split('"')[1::2]:
                nq = norm(q)
                pieces = [x for x in re.split(r"\s*(?:\.\.\.|\u2026)\s*", nq) if len(x) > 3]
                if len(nq) < 15 or not pieces:
                    continue
                n += 1
                hits = [p for p, t in corpus.items() if all(x in t for x in pieces)]
                if not hits:
                    st = "MISS"
                elif any(os.path.basename(h) in named for h in hits):
                    st = "OK"
                else:
                    st = "ELSEWHERE " + ",".join(sorted(set(os.path.basename(h) for h in hits))[:3])
                if not st.startswith("OK"):
                    print(f"{st} | {os.path.basename(f)} | {title} | {q[:100]}")
                else:
                    print(f"OK | {os.path.basename(f)} | {title}", file=sys.stderr)
        if n == 0:
            print(f"NOQUOTE | {os.path.basename(f)} | {title}")
