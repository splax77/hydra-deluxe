"""Compare the facts files of two Clone Hero builds.

This is test 4 of the feasibility study
(docs/research/2026-10-28-clone-hero-mcp-feasibility.md, "Test 4: a second
build", has its result), built as Task E of the feasibility-tests plan, which
has left the repo (removed; read it with git show
444631e6:docs/superpowers/plans/2026-10-28-ch-mcp-feasibility-tests.md). It
says whether the obfuscator's scrambled names survive from one build to the
next, and which readable classes moved a field.

Classes are matched by namespace and name, never by the image they came from,
because builds keep the game code in differently named images. The facts
schema is described in tools/ch_mcp/README.md; whether a name is readable is
decided by `dump_facts.is_readable`.

Run:
    py tools/ch_mcp/compare_dumps.py <older facts.json> <newer facts.json>
"""

from __future__ import annotations

import argparse
import json
import os
import sys

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp.dump_facts import is_readable  # noqa: E402


class CompareError(Exception):
    """A facts file could not be read or lacks the keys this script needs."""


def load_facts(path: str) -> dict:
    """Read one facts file, once, and check it has the keys used here."""
    try:
        with open(path, encoding="utf-8") as f:
            facts = json.load(f)
    except (OSError, ValueError) as exc:
        raise CompareError(f"{path}: {exc}") from exc
    if not isinstance(facts, dict) or not isinstance(facts.get("classes"), list):
        raise CompareError(f"{path}: no 'classes' list")
    return facts


def class_key(cls: dict) -> tuple[str, str]:
    return (cls["namespace"], cls["name"])


def display_name(key: tuple[str, str]) -> str:
    namespace, name = key
    return f"{namespace}.{name}" if namespace else name


def shape(cls: dict) -> tuple:
    """What a scrambled class is matched on besides its name: its field types
    (which also gives the field count) and its method count."""
    return (tuple(sorted(f["type"] for f in cls["fields"])), len(cls["methods"]))


def _index(classes: list[dict]) -> dict[tuple[str, str], list[dict]]:
    index: dict[tuple[str, str], list[dict]] = {}
    for cls in classes:
        index.setdefault(class_key(cls), []).append(cls)
    return index


def _fields_by_name(cls: dict) -> dict[str, dict]:
    """The class's fields by name. A name the class repeats is left out, since
    it cannot be paired with the other side."""
    by_name: dict[str, list[dict]] = {}
    for f in cls["fields"]:
        by_name.setdefault(f["name"], []).append(f)
    return {name: fs[0] for name, fs in by_name.items() if len(fs) == 1}


def _hex(offset: int) -> str:
    return f"0x{offset:X}"


def _name_counts(members: list[dict]) -> tuple[int, int]:
    """(scrambled, readable) name counts of a field or method list."""
    readable = sum(1 for m in members if is_readable(m["name"]))
    return len(members) - readable, readable


def _name_change(old: list[dict], new: list[dict]) -> str | None:
    """How the scrambled and readable name counts of one member list changed:
    'to readable', 'to scrambled', 'other', or None when the counts match."""
    old_s, old_r = _name_counts(old)
    new_s, new_r = _name_counts(new)
    if (old_s, old_r) == (new_s, new_r):
        return None
    if new_s < old_s and new_r > old_r:
        return "to readable"
    if new_s > old_s and new_r < old_r:
        return "to scrambled"
    return "other"


def compare(old: dict, new: dict) -> list[str]:
    """The report lines for two facts files, `old` first. The last line is the
    names verdict."""
    old_index = _index(old["classes"])
    new_index = _index(new["classes"])
    # Class names go through the same `is_readable` as member names, so the
    # two never disagree; the stored `readable` flag is not read.
    old_readable = {k for k in old_index if is_readable(k[1])}
    new_readable = {k for k in new_index if is_readable(k[1])}
    old_scrambled = set(old_index) - old_readable
    new_scrambled = set(new_index) - new_readable

    lines = [
        f"A: {old.get('game_version')} ({len(old['classes'])} classes)",
        f"B: {new.get('game_version')} ({len(new['classes'])} classes)",
    ]

    # Readable classes: present on both sides, or missing from one.
    both = sorted(old_readable & new_readable)
    single = [k for k in both if len(old_index[k]) == 1 and len(new_index[k]) == 1]
    repeated = len(both) - len(single)
    only_old = sorted(old_readable - new_readable)
    only_new = sorted(new_readable - old_readable)
    lines.append(
        f"readable classes in both: {len(both)} "
        f"({repeated} appear more than once in a file, so their fields are not compared)"
    )
    lines.append(f"readable classes only in A: {len(only_old)}")
    lines.extend(f"  {display_name(k)}" for k in only_old)
    lines.append(f"readable classes only in B: {len(only_new)}")
    lines.extend(f"  {display_name(k)}" for k in only_new)

    # Field offsets of the readable classes that appear once on each side.
    compared = 0
    not_comparable = 0
    moved: list[str] = []
    name_changes: list[tuple[tuple[str, str], str | None, str | None, dict, dict]] = []
    for key in single:
        a, b = old_index[key][0], new_index[key][0]
        a_fields, b_fields = _fields_by_name(a), _fields_by_name(b)
        for name, fa in a_fields.items():
            fb = b_fields.get(name)
            if fb is None:
                continue
            if fa["offset"] is None or fb["offset"] is None:
                not_comparable += 1
                continue
            compared += 1
            if fa["offset"] != fb["offset"]:
                moved.append(
                    f"  {display_name(key)}.{fa['name']}: {_hex(fa['offset'])} -> {_hex(fb['offset'])}"
                )
        field_change = _name_change(a["fields"], b["fields"])
        method_change = _name_change(a["methods"], b["methods"])
        if field_change or method_change:
            name_changes.append((key, field_change, method_change, a, b))
    lines.append(
        f"field offsets: {compared} fields compared, {len(moved)} moved, "
        f"{not_comparable} not comparable (offset null on a side)"
    )
    lines.extend(moved)

    # Member names in the readable classes: scrambled on one side, readable
    # on the other.
    def tally(index: int, label: str) -> str:
        changes = [c[index] for c in name_changes]
        return (
            f"{label} went scrambled to readable in {changes.count('to readable')}, "
            f"readable to scrambled in {changes.count('to scrambled')}, "
            f"other count changes in {changes.count('other')}"
        )

    lines.append(
        f"member names in readable classes: {len(name_changes)} classes changed; "
        f"{tally(1, 'field names')}; {tally(2, 'method names')}"
    )
    for key, _, _, a, b in name_changes:
        fa_s, fa_r = _name_counts(a["fields"])
        fb_s, fb_r = _name_counts(b["fields"])
        ma_s, ma_r = _name_counts(a["methods"])
        mb_s, mb_r = _name_counts(b["methods"])
        lines.append(
            f"  {display_name(key)}: fields scrambled {fa_s} -> {fb_s}, readable {fa_r} -> {fb_r}; "
            f"methods scrambled {ma_s} -> {mb_s}, readable {ma_r} -> {mb_r}"
        )

    # Scrambled classes: a name survives when it is on both sides with the
    # same shape. A name printed more than once in a file survives when every
    # copy's shape is matched on the other side.
    common = old_scrambled & new_scrambled
    kept = 0
    for key in common:
        if sorted(map(shape, old_index[key])) == sorted(map(shape, new_index[key])):
            kept += 1
    reshaped = len(common) - kept
    total = len(old_scrambled)
    lines.append(
        f"scrambled classes: {total} names in A, {len(new_scrambled)} in B; "
        f"same name and shape in both: {kept}; same name, other shape: {reshaped}; "
        f"only in A: {len(old_scrambled - new_scrambled)}; only in B: {len(new_scrambled - old_scrambled)}"
    )
    if kept == total:
        lines.append(f"names stable: {kept} of {total} scrambled classes keep their names")
    else:
        lines.append(f"names reshuffled: only {kept} of {total} scrambled names survive")
    return lines


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Compare the facts files of two Clone Hero builds.")
    parser.add_argument("old", help="facts.json of the older build (A)")
    parser.add_argument("new", help="facts.json of the newer build (B)")
    args = parser.parse_args(argv)
    # Scrambled names are not in a Windows console's code page; escape them
    # rather than crash partway through the report. A captured stdout (as in
    # the tests) has no reconfigure.
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(errors="backslashreplace")
    try:
        old = load_facts(args.old)
        new = load_facts(args.new)
        lines = compare(old, new)
    except CompareError as exc:
        print(f"compare_dumps: {exc}", file=sys.stderr)
        return 1
    except KeyError as exc:
        print(f"compare_dumps: a class, field or method record lacks the key {exc}", file=sys.stderr)
        return 1
    print(f"A file: {args.old}")
    print(f"B file: {args.new}")
    for line in lines:
        print(line)
    return 0


if __name__ == "__main__":
    sys.exit(main())
