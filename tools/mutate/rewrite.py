"""The rewriter: write the switched copies the mutation build compiles.

The finder (find.py) says where a bug can be planted. This file takes its
spots and writes, for each .c or .cpp source, a copy in which every planted
spot sits behind its own numbered switch:

    HYDRA_MUT(17, (<original expression>), (<expression with the one change>))

The macro lives in hydra_mutant.h, which the mutation build force-includes
into every copy, so a copy gets no include line. Each side is passed in round
brackets, because the preprocessor splits macro arguments at any comma that
is not inside round brackets.

    py -I tools/mutate/rewrite.py src/core/stars.cpp [more files...]
    py -I tools/mutate/rewrite.py --out <folder> --root <repo> --precheck <exe> <files>

What it writes, all under --out (default: build-mutation in the checkout
holding this script, the mutation preset's build folder):

    mutated/<path relative to --root>   the copies; the mutation build's
                                        CMakeLists.txt block reads this folder
    mutants.json                        every switch and every skipped spot
    skipped.txt                         the skipped spots, one per line

The copies go only to the folder the mutation build reads. The tool refuses an
--out inside --root's src/ folder, and never writes over a source.

Nested spots. When several spots share one expression, or one spot's
expression holds another's, each spot wraps the whole of its expression once,
innermost first. The "on" side of a switch is always the original expression
with that switch's one change and nothing else; the "off" side holds the
wrappers inside it. So with every switch off the copy reads exactly as the
original, and with one switch on it differs only by that switch's change.

A compare spot's change is written as a call to a helper in hydra_mutant.h
on the spot's two operands (changed_span says the exact text), not as the
flipped operator itself: a type may define == and no !=, and the flipped
operator would then not build. Every other kind's change is its token's
replacement.
After writing each copy the tool reads it back through the same code mask and
checks both of those, byte for byte; a copy that fails stops the run.

Skipped spots. A spot the finder skipped keeps its reason. The rewriter adds
two reasons of its own (REWRITE_SKIP_REASONS): a spot in a file the mutation
build takes no copy of (a header, for one), and two spots whose expressions
cross, which no wrapping can hold both of. Both are listed, never planted.

mutants.json (FORMAT 1):

    {"format": 1,
     "complete": true, false while a run is still writing copies (or
                 stopped part way); a run that is not complete lists only
                 "copies", so the next run can clear them,
     "root": <absolute --root>,
     "kinds": {<kind>: <words>, ...},                  find.py's KINDS
     "skip_reasons": {<reason>: <words>, ...},         find.py's and this file's
     "copies": [<path under mutated/>, ...],
     "mutants": [{"number": <switch>, "file": <path relative to root>,
                  "copy": <path under mutated/>, "line": <line in the source>,
                  "copy_lines": [<first>, <last>], "kind": <kind>,
                  "change": <words>, "spot": <finder's number in the file>}, ...],
     "skipped": [{"file", "line", "kind", "change", "spot", "reason"}, ...]}

Switch numbers run 1, 2, 3 ... over the files in path order, then the
finder's order inside a file, so the same files always give the same numbers.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional

sys.path.insert(0, str(Path(__file__).resolve().parent))
import find  # noqa: E402

FORMAT = 1

# The sources the mutation build compiles from a copy; see the HYDRA_MUTATION
# block of CMakeLists.txt, which refuses a copy of anything else.
COPY_SUFFIXES = {".c", ".cpp"}

REWRITE_SKIP_REASONS = {
    "not_copied": "in a file the mutation build takes no copy of (it takes only .c and .cpp sources)",
    "crossing": "its expression crosses another spot's expression, so no wrapping holds both",
}
assert not set(REWRITE_SKIP_REASONS) & set(find.SKIP_REASONS), "a reason key has two meanings"

MACRO = "HYDRA_MUT"


class RewriteError(RuntimeError):
    pass


# -- wrapping -------------------------------------------------------------


def _crossing(spots: list[find.Spot]) -> set[int]:
    """Finder numbers of planted spots whose expressions cross another's."""
    bad: set[int] = set()
    for a in spots:
        for b in spots:
            if a is b:
                continue
            if a.expr_start < b.expr_start < a.expr_end < b.expr_end:
                bad.add(a.number)
                bad.add(b.number)
    return bad


# The helper in hydra_mutant.h that each compare replacement is written as.
# A type may define == but not !=, or < but not >=, so writing the flipped
# operator itself could fail to build; each helper uses the real operator when
# the operands have it and builds the same meaning from one they do have.
HELPERS = {"==": "eq", "!=": "ne", "<": "lt", "<=": "le", ">": "gt", ">=": "ge"}
assert set(HELPERS) == set(find.KINDS["compare"]["swap"].values()), "a compare replacement has no helper"


def changed_span(text: str, n: int, s: find.Spot) -> tuple[int, int, str]:
    """(start, end, new text): what switch n, on, writes in place of text[start:end].

    A compare spot becomes a call to its helper on its two operands, with the
    switch number as the helper's template argument, so a type with no usable
    operator fails to build naming the switch. Every other spot is its token's
    replacement."""
    if s.kind != "compare":
        return s.offset, s.offset + s.length, s.replacement
    helper = HELPERS[s.replacement]
    lhs = text[s.lhs_start:s.lhs_end]
    rhs = text[s.rhs_start:s.rhs_end]
    return s.lhs_start, s.rhs_end, f"::hydra_mutant::{helper}<{n}>(({lhs}), ({rhs}))"


def _changed(text: str, start: int, end: int, n: int, s: find.Spot) -> str:
    a, b, new = changed_span(text, n, s)
    return text[start:a] + new + text[b:end]


def wrap_text(text: str, numbered: list[tuple[int, find.Spot]]) -> str:
    """The text with each (switch number, spot) wrapped; spots must not cross."""
    by_interval: dict[tuple[int, int], list[tuple[int, find.Spot]]] = {}
    for n, s in numbered:
        if not (s.expr_start <= s.offset and s.offset + s.length <= s.expr_end):
            raise RewriteError(f"rewrite.py: spot {s.number} on line {s.line} lies outside its own expression")
        if text[s.offset:s.offset + s.length] != s.original:
            raise RewriteError(f"rewrite.py: spot {s.number} on line {s.line} does not match the file's text")
        if s.kind == "compare" and (None in (s.lhs_start, s.lhs_end, s.rhs_start, s.rhs_end) or not (
                                        s.expr_start <= s.lhs_start < s.lhs_end <= s.offset
                                        and s.offset + s.length <= s.rhs_start < s.rhs_end <= s.expr_end)):
            raise RewriteError(f"rewrite.py: compare spot {s.number} on line {s.line} has no operands "
                               "inside its expression")
        by_interval.setdefault((s.expr_start, s.expr_end), []).append((n, s))
    intervals = sorted(by_interval, key=lambda iv: (iv[0], -iv[1]))

    def render(a: int, b: int, skip: Optional[tuple[int, int]]) -> str:
        out = []
        pos = a
        for iv in intervals:
            if iv == skip or iv[0] < pos or iv[1] > b:
                continue
            out.append(text[pos:iv[0]])
            out.append(wrap(iv))
            pos = iv[1]
        out.append(text[pos:b])
        return "".join(out)

    def wrap(iv: tuple[int, int]) -> str:
        inner = render(iv[0], iv[1], iv)
        for n, s in sorted(by_interval[iv], key=lambda p: p[0]):
            inner = f"{MACRO}({n}, ({inner}), ({_changed(text, iv[0], iv[1], n, s)}))"
        return inner

    return render(0, len(text), None)


# -- reading a copy back ----------------------------------------------------


@dataclass
class Wrapper:
    number: int
    start: int  # offset of HYDRA_MUT in the copy
    end: int  # offset just past its closing bracket
    off: list = field(default_factory=list)
    on: list = field(default_factory=list)


_HEAD = re.compile(MACRO + r"\((\d+), \(")


def _close(mask: str, open_at: int) -> int:
    """Offset of the `)` that closes the `(` at open_at, by the code mask."""
    depth = 0
    for k in range(open_at, len(mask)):
        c = mask[k]
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                return k
    raise RewriteError(f"rewrite.py: the bracket at offset {open_at} of a copy never closes")


def parse_copy(copy: str, mask: str, a: int = 0, b: Optional[int] = None) -> list:
    """The copy as a list of plain text pieces and Wrappers, read through its code mask."""
    if b is None:
        b = len(copy)
    out: list = []
    pos = a
    while True:
        m = _HEAD.search(mask, pos, b)
        if not m:
            break
        off_open = m.end() - 1
        off_close = _close(mask, off_open)
        if mask[off_close:off_close + 4] != "), (":
            raise RewriteError(f"rewrite.py: a {MACRO} at offset {m.start()} of a copy has no second side")
        on_open = off_close + 3
        on_close = _close(mask, on_open)
        if mask[on_close + 1:on_close + 2] != ")":
            raise RewriteError(f"rewrite.py: a {MACRO} at offset {m.start()} of a copy does not close")
        out.append(copy[pos:m.start()])
        out.append(Wrapper(int(m.group(1)), m.start(), on_close + 2,
                           parse_copy(copy, mask, off_open + 1, off_close),
                           parse_copy(copy, mask, on_open + 1, on_close)))
        pos = on_close + 2
    out.append(copy[pos:b])
    return out


def take_sides(pieces: list, on: Optional[int] = None) -> str:
    """The text with switch `on` (or none) on: each wrapper replaced by the side it picks."""
    out = []
    for p in pieces:
        if isinstance(p, Wrapper):
            out.append(take_sides(p.on if p.number == on else p.off, on))
        else:
            out.append(p)
    return "".join(out)


def wrappers(pieces: list):
    """Every Wrapper in the pieces, outer ones first."""
    for p in pieces:
        if isinstance(p, Wrapper):
            yield p
            yield from wrappers(p.off)
            yield from wrappers(p.on)


# -- files ------------------------------------------------------------------


def _true_case(path: Path, root: Path) -> Path:
    """path relative to root, each part spelled as the folder lists it."""
    rel = path.resolve().relative_to(root.resolve())
    here = root.resolve()
    parts = []
    for part in rel.parts:
        names = {n.lower(): n for n in os.listdir(here)}
        name = names.get(part.lower(), part)
        parts.append(name)
        here = here / name
    return Path(*parts)


def read_text(path: Path) -> str:
    return find.read_source(path)


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="") as f:
        f.write(text)


def _inside(p: Path, folder: Path) -> bool:
    try:
        p.resolve().relative_to(folder.resolve())
        return True
    except ValueError:
        return False


def _clear_old_copies(out: Path) -> None:
    """Remove the copies the last run listed; stop on any file it did not write."""
    mutated = out / "mutated"
    if not mutated.exists():
        return
    old: set[str] = set()
    listing = out / "mutants.json"
    if listing.is_file():
        old = set(json.loads(listing.read_text(encoding="utf-8")).get("copies", []))
    present = {p.relative_to(mutated).as_posix() for p in mutated.rglob("*") if p.is_file()}
    unknown = sorted(present - old)
    if unknown:
        raise RewriteError(f"rewrite.py: {mutated} holds files no earlier run of this tool listed in "
                           f"{listing}, so it will not remove them: {', '.join(unknown)}. "
                           "Remove them by hand, or name another --out.")
    for rel in sorted(present):
        (mutated / rel).unlink()
    for d in sorted((p for p in mutated.rglob("*") if p.is_dir()), key=lambda p: len(p.parts), reverse=True):
        if not any(d.iterdir()):
            d.rmdir()


def rewrite(files, out: Path, root: Path, precheck: Optional[Path] = None) -> dict:
    """Write the copies, mutants.json and skipped.txt under out; return mutants.json's content."""
    root = Path(root).resolve()
    out = Path(out).resolve()
    precheck = Path(precheck) if precheck else find.default_precheck()
    if _inside(out, root / "src"):
        raise RewriteError(f"rewrite.py: --out {out} lies inside {root / 'src'}; copies never go there")
    rels: dict[str, Path] = {}
    for f in files:
        p = Path(f)
        if not p.is_absolute():
            p = root / p
        if not p.is_file():
            raise RewriteError(f"rewrite.py: no such file {p}")
        if not _inside(p, root):
            raise RewriteError(f"rewrite.py: {p} lies outside --root {root}")
        if _inside(p, out):
            raise RewriteError(f"rewrite.py: {p} lies inside --out {out}")
        rel = _true_case(p, root)
        rels[rel.as_posix()] = root / rel

    mutated = out / "mutated"
    _clear_old_copies(out)
    listing_path = out / "mutants.json"
    (out / "skipped.txt").unlink(missing_ok=True)

    mutants: list[dict] = []
    skipped: list[dict] = []
    copies: list[str] = []
    n = 0
    for rel in sorted(rels):
        src = rels[rel]
        text = read_text(src)
        spots = find.find_spots(src, precheck)
        planted = [s for s in spots if s.skip is None]
        reasons: dict[int, str] = {s.number: s.skip for s in spots if s.skip is not None}
        if src.suffix.lower() not in COPY_SUFFIXES:
            for s in planted:
                reasons[s.number] = "not_copied"
            planted = []
        for k in _crossing(planted):
            reasons[k] = "crossing"
        planted = [s for s in planted if s.number not in reasons]
        for s in spots:
            if s.number in reasons:
                skipped.append({"file": rel, "line": s.line, "kind": s.kind, "change": s.change,
                                "spot": s.number, "reason": reasons[s.number]})
        if not planted:
            continue
        numbered = []
        for s in planted:
            n += 1
            numbered.append((n, s))
        copy_path = mutated / rel
        if copy_path.resolve() == src.resolve():
            raise RewriteError(f"rewrite.py: the copy of {rel} would overwrite it")
        copy = wrap_text(text, numbered)
        # Listed before it is written, so a run that stops part way still
        # names every copy it left, and the next run can clear them.
        copies.append(rel)
        write_text(listing_path, json.dumps({"format": FORMAT, "complete": False, "copies": copies}) + "\n")
        write_text(copy_path, copy)
        pieces = check_copy(copy_path, text, numbered, precheck)
        spans = {w.number: (w.start, w.end) for w in wrappers(pieces)}
        for num, s in numbered:
            a, b = spans[num]
            mutants.append({"number": num, "file": rel, "copy": rel,
                            "line": s.line, "copy_lines": [copy.count("\n", 0, a) + 1, copy.count("\n", 0, b) + 1],
                            "kind": s.kind, "change": s.change, "spot": s.number})

    reasons_words = dict(find.SKIP_REASONS)
    reasons_words.update(REWRITE_SKIP_REASONS)
    listing = {
        "format": FORMAT,
        "complete": True,
        "root": root.as_posix(),
        "kinds": {k: v["words"] for k, v in find.KINDS.items()},
        "skip_reasons": reasons_words,
        "copies": copies,
        "mutants": mutants,
        "skipped": skipped,
    }
    out.mkdir(parents=True, exist_ok=True)
    lines = [f"{s['file']}:{s['line']}  {s['kind']}  {s['change']}  skipped: {reasons_words[s['reason']]}"
             for s in skipped]
    write_text(out / "skipped.txt", "".join(line + "\n" for line in lines))
    write_text(listing_path, json.dumps(listing, indent=1) + "\n")
    return listing


def check_copy(copy_path: Path, original: str, numbered: list[tuple[int, find.Spot]], precheck: Path) -> list:
    """Read a written copy back and stop unless every switch off gives the original
    and each switch on alone gives the original with that one change."""
    copy = read_text(copy_path)
    pieces = parse_copy(copy, find.code_mask(copy_path, precheck))
    if take_sides(pieces) != original:
        raise RewriteError(f"rewrite.py: {copy_path} with every switch off is not the original")
    found = sorted(w.number for w in wrappers(pieces))
    if found != sorted(n for n, _ in numbered):
        raise RewriteError(f"rewrite.py: {copy_path} holds switches {found}, not the ones it was written with")
    for n, s in numbered:
        if take_sides(pieces, n) != _changed(original, 0, len(original), n, s):
            raise RewriteError(f"rewrite.py: {copy_path} with switch {n} on differs from its one change")
    return pieces


def main(argv=None) -> int:
    here_root = Path(__file__).resolve().parents[2]
    ap = argparse.ArgumentParser(description="Write the switched copies the mutation build compiles.")
    ap.add_argument("files", nargs="+", type=Path)
    ap.add_argument("--root", type=Path, default=here_root,
                    help="the checkout the files belong to (default: the one holding this script)")
    ap.add_argument("--out", type=Path, default=None,
                    help="where the copies and mutants.json go (default: <root>/build-mutation)")
    ap.add_argument("--precheck", type=Path, default=None,
                    help="hydra_precheck.exe (default: the main checkout's build-cpp/Release copy)")
    a = ap.parse_args(argv)
    out = a.out or a.root / "build-mutation"
    try:
        listing = rewrite(a.files, out, a.root, a.precheck)
    except (RewriteError, find.FinderError) as e:
        print(e, file=sys.stderr)
        return 1
    counts: dict[str, int] = {}
    for s in listing["skipped"]:
        counts[s["reason"]] = counts.get(s["reason"], 0) + 1
    print(f"{len(listing['mutants'])} switches in {len(listing['copies'])} copies under "
          f"{Path(out).resolve() / 'mutated'}; {len(listing['skipped'])} spots skipped")
    for reason in sorted(counts):
        print(f"  skipped, {reason}: {counts[reason]}")
    print("Every copy reads back as its original with every switch off, and as one change with one on.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
