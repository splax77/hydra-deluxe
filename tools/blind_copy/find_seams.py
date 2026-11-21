"""List every seam hit in every test file of a blind copy, with its kind.

    py -I tools/blind_copy/find_seams.py <copy> --out <folder>

<copy> is the blind copy's root, the folder that holds `code/`. The search is
grade() in grade_seams.py, the one owner of which lines are seams and which
test owns them; this script adds no search pattern. It grades each code file
under code/tests and code/tools on its own, with the whole file as the slice
and an empty facts file, so every hit is listed and none is graded. Which
files count as code is facts_format.CODE_SUFFIXES; other files are counted by
folder and suffix as skipped, so none is left out silently.

It writes three files to <folder>:
  seams.tsv       one row per test and hit grade() gives, with the hit's kind
  coverage.tsv    each seam name a src/ header declares and tests/ names,
                  and whether any hit line names it
  file_kinds.tsv  per folder and suffix: files, files with no test, tests, hits
and prints the counts, the hits per kind, the file kinds with no test or no
hit, and every uncovered name. A hit's kind is kind(): kind_word() and the
hit's file, so the same word in two files is two kinds.
"""

import argparse
import os
import re
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import facts_format as ff  # noqa: E402
import grade_seams as gs  # noqa: E402

ROOTS = ("code/tests", "code/tools")
TESTS_ROOT = "code/tests"
SRC_ROOT = "code/src"
HEADER_SUFFIXES = (".h", ".hpp")
SEAM_WORD = re.compile(r"set_\w+|\w+_for_test")
WRAPPER_LABEL = "seam wrapper object"
WORD = re.compile(r"\w+")
CALLED = re.compile(r"\b\w+(?=\s*\()")
NO_TEST = "no test"


def shown(path):
    """A copy path as the records cite it, without the leading code/."""
    return path[len("code/"):] if path.startswith("code/") else path


def kind_word(line, labels, patterns):
    for tok in WORD.findall(line):
        if SEAM_WORD.fullmatch(tok):
            return tok
    if WRAPPER_LABEL in labels:
        wrapper = next(p for p in patterns if p.label == WRAPPER_LABEL)
        toks = WORD.findall(wrapper.regex.search(line).group(0))
        if toks and toks[0] == "const":
            toks = toks[1:]
        return toks[0]
    return labels[0]


def kind(path, line, labels, patterns):
    return f"{kind_word(line, labels, patterns)} ({shown(path)})"


def walk(copy, root):
    """Every file under root, as sorted code/... paths."""
    base = os.path.join(copy, *root.split("/"))
    if not os.path.isdir(base):
        raise FileNotFoundError(f"the copy has no {root} folder: {base}")
    out = []
    for dirpath, dirnames, files in os.walk(base):
        dirnames.sort()
        rel = os.path.relpath(dirpath, copy).replace("\\", "/")
        out += [rel + "/" + name for name in sorted(files)]
    return out


def is_code(path):
    return path.lower().endswith(ff.CODE_SUFFIXES)


def file_kind(path):
    folder, name = path.rsplit("/", 1)
    ext = os.path.splitext(name)[1].lower()
    return f"{shown(folder)} {ext or '(no suffix)'}"


class Lines:
    """Each copy file's lines, read once."""

    def __init__(self, copy):
        self.copy = copy
        self.cache = {}

    def of(self, path):
        if path not in self.cache:
            real = ff.copy_file(self.copy, path)
            if real is None or not os.path.isfile(real):
                raise FileNotFoundError(f"file is not in the copy: {path}")
            with open(real, encoding="utf-8", errors="replace") as f:
                self.cache[path] = f.read().split("\n")
        return self.cache[path]

    def text(self, path):
        return "\n".join(self.of(path))


def search(copy, patterns=None):
    """A dict: `rows` (one per test and hit), `files` (per file read: kind,
    tests, hits), `skipped` (paths not read) and `coverage`."""
    patterns = patterns if patterns is not None else gs.load_patterns()
    lines = Lines(copy)
    rows, files, skipped = [], [], []
    with tempfile.TemporaryDirectory(prefix="find_seams_") as tmp:
        facts = os.path.join(tmp, "facts.md")
        open(facts, "w", encoding="utf-8").close()
        for root in ROOTS:
            for path in walk(copy, root):
                if not is_code(path):
                    skipped.append(path)
                    continue
                ranges = [(path, 1, max(1, len(lines.of(path))))]
                try:
                    result = gs.grade(copy, ranges, facts, patterns)
                except Exception as e:
                    raise RuntimeError(f"grade() failed on {path}: {e}") from e
                tests = sum(1 for l in gs.render(result, ranges).splitlines() if l.startswith("test: "))
                mine = []
                for t, hp, hn, labels, _ in result["rows"]:
                    mine.append({
                        "test": t.label() if t else NO_TEST,
                        "test_file": shown(t.file) if t else "",
                        "test_line": str(t.start) if t else "",
                        "hit_file": shown(hp),
                        "hit_line": str(hn),
                        "patterns": ", ".join(labels),
                        "kind": kind(hp, lines.of(hp)[hn - 1], labels, patterns),
                        "_hit": (hp, hn),
                    })
                rows += mine
                files.append({"path": path, "kind": file_kind(path), "tests": tests, "hits": len(mine)})
    return {"rows": rows, "files": files, "skipped": skipped,
            "coverage": coverage(copy, lines, rows)}


def coverage(copy, lines, rows):
    """Each seam name a src/ header declares and a tests/ file names, with
    where, and whether any hit line names it."""
    declared = {}
    for path in walk(copy, SRC_ROOT):
        if not path.lower().endswith(HEADER_SUFFIXES):
            continue
        _, code = gs.code_marks(lines.text(path))
        for n, line in enumerate(code, 1):
            for m in CALLED.finditer(line):
                if SEAM_WORD.fullmatch(m.group(0)):
                    declared.setdefault(m.group(0), f"{shown(path)}:{n}")
    named = {}
    for path in walk(copy, TESTS_ROOT):
        if not is_code(path):
            continue
        _, code = gs.code_marks(lines.text(path))
        for n, line in enumerate(code, 1):
            for tok in sorted(set(WORD.findall(line)) & declared.keys()):
                named.setdefault(tok, []).append(f"{shown(path)}:{n}")
    hit_words = set()
    for hp, hn in {r["_hit"] for r in rows}:
        hit_words.update(WORD.findall(lines.of(hp)[hn - 1]))
    return [{"name": name, "state": "covered" if name in hit_words else "uncovered",
             "declared_at": declared[name], "named_at": ", ".join(named[name])}
            for name in sorted(named)]


def kind_counts(rows):
    counts = {}
    for r in rows:
        counts[r["kind"]] = counts.get(r["kind"], 0) + 1
    return counts


def file_kinds(found):
    out = {}
    for f in found["files"]:
        k = out.setdefault(f["kind"], {"state": "read", "files": 0, "no_test": 0, "tests": 0, "hits": 0})
        k["files"] += 1
        k["no_test"] += f["tests"] == 0
        k["tests"] += f["tests"]
        k["hits"] += f["hits"]
    for path in found["skipped"]:
        k = out.setdefault(file_kind(path), {"state": "skipped", "files": 0, "no_test": "", "tests": "", "hits": ""})
        k["files"] += 1
    return out


def write_tsv(path, header, rows):
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        for row in [header] + rows:
            cells = [str(c) for c in row]
            for c in cells:
                if "\t" in c or "\n" in c or "\r" in c:
                    raise ValueError(f"a cell holds a tab or line break, so {path} cannot hold it: {c!r}")
            f.write("\t".join(cells) + "\n")


SEAM_COLUMNS = ("test", "test_file", "test_line", "hit_file", "hit_line", "patterns", "kind")
COVERAGE_COLUMNS = ("name", "state", "declared_at", "named_at")
KIND_COLUMNS = ("file_kind", "state", "files", "no_test", "tests", "hits")


def write_outputs(found, out):
    os.makedirs(out, exist_ok=True)
    write_tsv(os.path.join(out, "seams.tsv"), SEAM_COLUMNS,
              [[r[c] for c in SEAM_COLUMNS] for r in found["rows"]])
    write_tsv(os.path.join(out, "coverage.tsv"), COVERAGE_COLUMNS,
              [[c[k] for k in COVERAGE_COLUMNS] for c in found["coverage"]])
    fk = file_kinds(found)
    write_tsv(os.path.join(out, "file_kinds.tsv"), KIND_COLUMNS,
              [[name] + [fk[name][k] for k in KIND_COLUMNS[1:]] for name in sorted(fk)])


def summary(found):
    out = []
    files = found["files"]
    out.append(f"files read: {len(files)} | no test: {sum(1 for f in files if f['tests'] == 0)} | "
               f"skipped: {len(found['skipped'])}")
    fk = file_kinds(found)
    for name in sorted(fk):
        k = fk[name]
        if k["state"] == "skipped":
            out.append(f"skipped: {name} | files: {k['files']}")
        else:
            out.append(f"file kind: {name} | files: {k['files']} | no test: {k['no_test']} | "
                       f"tests: {k['tests']} | hits: {k['hits']}")
    for name in sorted(fk):
        k = fk[name]
        if k["state"] == "read" and k["tests"] == 0:
            out.append(f"no tests: {name} | files: {k['files']}")
        elif k["state"] == "read" and k["hits"] == 0:
            out.append(f"no hits: {name} | files: {k['files']} | tests: {k['tests']}")
    rows = found["rows"]
    counts = kind_counts(rows)
    out.append(f"tests: {sum(f['tests'] for f in files)}")
    out.append(f"hits: {len(rows)}")
    out.append(f"no test hits: {sum(1 for r in rows if r['test'] == NO_TEST)}")
    out.append(f"kinds: {len(counts)}")
    for name in sorted(counts):
        out.append(f"kind: {name} | hits: {counts[name]}")
    uncovered = [c for c in found["coverage"] if c["state"] == "uncovered"]
    out.append(f"seam names declared and named: {len(found['coverage'])}")
    out.append(f"uncovered names: {len(uncovered)}")
    for c in uncovered:
        out.append(f"uncovered: {c['name']} | declared: {c['declared_at']} | named: {c['named_at']}")
    return "\n".join(out) + "\n"


def main(argv):
    parser = argparse.ArgumentParser(description="List every seam hit in every test file of a blind copy.")
    parser.add_argument("copy", help="the blind copy's root, the folder that holds code/")
    parser.add_argument("--out", required=True, help="folder for seams.tsv, coverage.tsv and file_kinds.tsv")
    args = parser.parse_args(argv[1:])
    try:
        found = search(args.copy)
        write_outputs(found, args.out)
    except (ValueError, FileNotFoundError, RuntimeError) as e:
        sys.stderr.write(f"find_seams: {e}\n")
        return 2
    sys.stdout.write(summary(found))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
