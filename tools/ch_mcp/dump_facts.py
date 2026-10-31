"""Read an Il2CppDumper dump.cs and write facts.json beside it, or write the
facts table that the Clone Hero pages cite.

facts.json lists every class in the dump with its fields and methods, plus a
`signatures` block for the classes the Clone Hero MCP tests need by name. The
schema is described in plain sentences in tools/ch_mcp/README.md; that page is
its one description, so this file does not repeat it.

The facts table (`--tsv`) holds one row for each game fact the pages under a
folder cite, and each fact `tools/ch_probe/constants.py` names, looked up in
one build's dump. The plan 2026-10-30-clone-hero-reference.md, question 2,
gives its columns. Which backticked spans count as game facts is decided by
`game_fact` alone; the C++ docs test (tests/test_docs_match_code.cpp) applies
the same forms, so compare the two when either changes. Which build a span
belongs to is decided by `span_builds`, which mirrors that test's rule too.
`--check` rebuilds the table and exits 1 when the file on disk differs.

Run:
    py tools/ch_mcp/dump_facts.py <dump.cs> --version 1.1.0.6142-final
    py tools/ch_mcp/dump_facts.py <dump.cs or facts.json> [--version V]
        --tsv docs/clone-hero --out <table.tsv> --dll <GameAssembly.dll> [--check]
"""

from __future__ import annotations

import argparse
import ast
import difflib
import hashlib
import json
import os
import re
import sys
from typing import NamedTuple

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as probe_constants  # noqa: E402

# The readable classes the signatures block names. The drum engine is
# scrambled, so it is found by its constructor's address instead.
NAMED_SIGNATURES = ("GameManager", "ScoreManager", "DrumsPlayer")
ENGINE_SIGNATURE = "drums_engine"

# Words that can precede a type in a field or method line of dump.cs.
_MODIFIERS = frozenset(
    "public private protected internal static readonly volatile const "
    "virtual override abstract sealed extern new unsafe".split()
)

_NAMESPACE_RE = re.compile(r"^// Namespace: ?(?P<ns>.*)$")
_HEADER_RE = re.compile(
    r"^(?P<mods>(?:[a-z]+ )*)(?P<kind>class|struct|interface|enum) "
    r"(?P<rest>.+) // TypeDefIndex: (?P<idx>\d+)$"
)
# A field line may carry a `/*Metadata offset 0x...*/` note before its `;`.
_FIELD_RE = re.compile(r"^\t(?P<decl>[^/]+?)(?: /\*[^*]*\*/)?; // 0x(?P<off>[0-9A-Fa-f]+)$")
# One hex address, as dump.cs prints an RVA and as a page cites one.
_HEX_ADDRESS = r"0x[0-9A-Fa-f]+"
_RVA_RE = re.compile(rf"^\t// RVA: (?P<rva>-1|{_HEX_ADDRESS}) ")
_SECTION_RE = re.compile(r"^\t// (?P<section>Fields|Properties|Methods)$")


class FactsError(Exception):
    """The dump could not be read, or a signature lookup did not match."""


def is_readable(name: str) -> bool:
    """A name is readable when it is plain ASCII; the obfuscator's names are not."""
    return name.isascii()


def _split_top_level(text: str) -> list[str]:
    """Split on commas that sit outside <>, [], () and string quotes."""
    parts: list[str] = []
    depth = 0
    quote = ""
    start = 0
    for i, ch in enumerate(text):
        if quote:
            if ch == quote and text[i - 1] != "\\":
                quote = ""
        elif ch in "\"'":
            quote = ch
        elif ch in "<[(":
            depth += 1
        elif ch in ">])":
            depth -= 1
        elif ch == "," and depth == 0:
            parts.append(text[start:i].strip())
            start = i + 1
    tail = text[start:].strip()
    if tail:
        parts.append(tail)
    return parts


def _bare_name(type_name: str) -> str:
    """Drop a trailing generic argument list: `Entry<TKey, TValue>` -> `Entry`."""
    if not type_name.endswith(">"):
        return type_name
    depth = 0
    for i in range(len(type_name) - 1, -1, -1):
        ch = type_name[i]
        if ch == ">":
            depth += 1
        elif ch == "<":
            depth -= 1
            if depth == 0:
                return type_name[:i]
    return type_name


def _strip_modifiers(tokens: list[str]) -> tuple[list[str], bool]:
    """Remove leading modifier words; say whether `static` was one of them."""
    is_static = False
    i = 0
    while i < len(tokens) and tokens[i] in _MODIFIERS:
        is_static = is_static or tokens[i] == "static"
        i += 1
    return tokens[i:], is_static


def _parse_field(decl: str) -> dict:
    tokens = decl.strip().split(" ")
    rest, is_static = _strip_modifiers(tokens)
    return {"name": rest[-1], "type": " ".join(rest[:-1]), "static": is_static}


def _method_name_start(head: str) -> int:
    """Index where the method name starts: the last space outside <>."""
    depth = 0
    for i in range(len(head) - 1, -1, -1):
        ch = head[i]
        if ch == ">":
            depth += 1
        elif ch == "<":
            depth -= 1
        elif ch == " " and depth == 0:
            return i + 1
    return 0


def _parse_param(param: str) -> str:
    """One parameter's type, with any default value and the name dropped."""
    eq = param.find(" = ")
    if eq >= 0:
        param = param[:eq]
    tokens = param.strip().split(" ")
    return " ".join(tokens[:-1]) if len(tokens) > 1 else tokens[0]


def _parse_method(line: str) -> dict:
    body = line.strip()
    open_paren = body.index("(")
    close_paren = body.rindex(")")
    head = body[:open_paren]
    start = _method_name_start(head)
    name = head[start:]
    ret_tokens, is_static = _strip_modifiers(head[:start].split())
    params = [_parse_param(p) for p in _split_top_level(body[open_paren + 1:close_paren])]
    return {"name": name, "return": " ".join(ret_tokens), "params": params, "static": is_static}


def _is_method_line(line: str) -> bool:
    """A method with a body ends `{ }`; one with no body (abstract) ends `);`."""
    return (
        line.startswith("\t")
        and not line.startswith(("\t//", "\t[", "\t/*", "\t|", "\t*/"))
        and line.rstrip().endswith(("{ }", ");"))
        and "(" in line
    )


def parse_dump(text: str) -> list[dict]:
    """Every class in a dump.cs text, in file order."""
    classes: list[dict] = []
    namespace = ""
    current: dict | None = None
    section = ""
    pending_rva: int | None = None
    for lineno, raw in enumerate(text.splitlines(), start=1):
        line = raw.rstrip("\r")
        if current is None:
            m = _NAMESPACE_RE.match(line)
            if m:
                namespace = m.group("ns").strip()
                continue
            m = _HEADER_RE.match(line)
            if m:
                name, _, listed = m.group("rest").partition(" : ")
                current = {
                    "name": name.strip(),
                    "namespace": namespace,
                    "kind": m.group("kind"),
                    "typedef_index": int(m.group("idx")),
                    "readable": is_readable(name.strip()),
                    "listed": _split_top_level(listed),
                    "fields": [],
                    "methods": [],
                }
                section = ""
                pending_rva = None
            continue
        if line in ("}", "{}"):  # an empty class prints its braces on one line
            classes.append(current)
            current = None
            continue
        m = _SECTION_RE.match(line)
        if m:
            section = m.group("section")
            continue
        if section == "Fields":
            m = _FIELD_RE.match(line)
            if m:
                field = _parse_field(m.group("decl"))
                field["offset"] = int(m.group("off"), 16)
                current["fields"].append(field)
        elif section == "Methods":
            m = _RVA_RE.match(line)
            if m:
                rva = m.group("rva")
                pending_rva = -1 if rva == "-1" else int(rva, 16)
            elif _is_method_line(line):
                if pending_rva is None:
                    raise FactsError(f"line {lineno}: method with no RVA line before it: {line.strip()}")
                method = _parse_method(line)
                method["rva"] = pending_rva
                current["methods"].append(method)
                pending_rva = None
    if current is not None:
        raise FactsError(f"class {current['name']} (TypeDefIndex {current['typedef_index']}) never closes")
    _resolve_bases(classes)
    return classes


def _resolve_bases(classes: list[dict]) -> None:
    """Split each class's listed types into its base class and its interfaces.

    dump.cs lists the base class first, then the interfaces, with no marker.
    The first listed type counts as the base unless every type in the dump with
    that name is an interface.
    """
    kinds: dict[str, set[str]] = {}
    for cls in classes:
        kinds.setdefault(_bare_name(cls["name"]), set()).add(cls["kind"])
    for cls in classes:
        listed = cls.pop("listed")
        base = None
        if cls["kind"] == "class" and listed:
            first_kinds = kinds.get(_bare_name(listed[0]), set())
            if first_kinds != {"interface"}:
                base = listed[0]
        cls["base"] = base
        cls["interfaces"] = listed[1:] if base is not None else listed


def find_by_ctor_rva(classes: list[dict], rva: int) -> dict:
    """The one class with a `.ctor` at `rva`; anything but one match refuses."""
    matches = [c for c in classes if any(m["name"] == ".ctor" and m["rva"] == rva for m in c["methods"])]
    if len(matches) != 1:
        raise FactsError(f"engine ctor RVA matched {len(matches)} classes")
    return matches[0]


def find_by_name(classes: list[dict], name: str) -> dict:
    """The one class with this exact name; anything but one match refuses."""
    matches = [c for c in classes if c["name"] == name]
    if len(matches) != 1:
        raise FactsError(f"class {name} matched {len(matches)} classes")
    return matches[0]


def signature(cls: dict) -> dict:
    """The short record of one class the MCP tests look up."""
    return {
        "name": cls["name"],
        "typedef_index": cls["typedef_index"],
        "base": cls["base"],
        "readable_fields": [f for f in cls["fields"] if is_readable(f["name"])],
        "field_count": len(cls["fields"]),
        "method_count": len(cls["methods"]),
    }


def build_facts(text: str, game_version: str, engine_ctor_rva: int, dump_file: str) -> dict:
    classes = parse_dump(text)
    signatures = {name: signature(find_by_name(classes, name)) for name in NAMED_SIGNATURES}
    engine = signature(find_by_ctor_rva(classes, engine_ctor_rva))
    engine["ctor_rva"] = engine_ctor_rva
    signatures[ENGINE_SIGNATURE] = engine
    return {
        "game_version": game_version,
        "dump_file": dump_file,
        "classes": classes,
        "signatures": signatures,
    }


def write_facts(dump_path: str, game_version: str, engine_ctor_rva: int) -> tuple[str, dict]:
    with open(dump_path, encoding="utf-8") as f:
        text = f.read()
    facts = build_facts(text, game_version, engine_ctor_rva, os.path.basename(dump_path))
    out_path = os.path.join(os.path.dirname(os.path.abspath(dump_path)), "facts.json")
    with open(out_path, "w", encoding="utf-8", newline="\n") as f:
        json.dump(facts, f, ensure_ascii=False, separators=(",", ":"))
    return out_path, facts


# ---- the facts table (--tsv) -------------------------------------------------

# The build a page fact belongs to when nothing on the page names another
# (the plan 2026-10-30-clone-hero-reference.md, task K5a). constants.py
# describes this build too (its docstring), so its facts are this build's.
DEFAULT_BUILD = "6142"

CONSTANTS_PATH = os.path.join(_REPO_ROOT, "tools", "ch_probe", "constants.py")
TOOLS_DIR = os.path.join(_REPO_ROOT, "tools")

# The `RVA_*` names in constants.py that are .rdata data addresses, not
# methods, so no dump line names them (the plan's question 2).
_DATA_ADDRESS_PREFIXES = ("RVA_CONST_", "RVA_FORMULA_")
_DATA_ADDRESS_NAMES = frozenset({"RVA_HITCHECK_THRESHOLD"})
# The constant that names the drum engine, whose object every OFF_* is in.
_ENGINE_CTOR_NAME = "RVA_DRUMS_ENGINE_CTOR"

# The page forms of a game fact. ASCII only, as the C++ test's std::regex
# reads `\w`. An RVA is at least five hex digits, so a short offset is prose.
_FACT_RVA_RE = re.compile(rf"^(?P<rva>{_HEX_ADDRESS})$")
_RVA_MIN_DIGITS = 5
_FACT_INDEX_OFFSET_RE = re.compile(rf"^#(?P<idx>\d+)\+(?P<off>{_HEX_ADDRESS})$")
_FACT_INDEX_MEMBER_RE = re.compile(r"^#(?P<idx>\d+)\.(?P<member>[A-Za-z_]\w*)$", re.ASCII)
_FACT_NAME_RE = re.compile(r"^(?P<left>[A-Za-z_]\w*)\.(?P<right>[A-Za-z_]\w*)$", re.ASCII)
# The C++ test's `file_ext` list (code_name_parts in
# tests/test_docs_match_code.cpp, with the extensions task K5a adds): a span
# ending in one of these names a file, not a game fact.
_FILE_EXT_RE = re.compile(
    r"\.(cpp|h|c|md|ini|json|py|ps1|exe|db|txt|mid|chart|sng|srb|opus|ogg|mp3|wav|flac|png|jpg|"
    r"cmake|hlsl|dll|asm|cs|tsv|bin|log|csv|xml|yml|yaml|dat|cfg)$",
    re.IGNORECASE,
)

_SPAN_RE = re.compile(r"`([^`\n]+)`")
_HEADING_RE = re.compile(r"^(?P<hashes>#{1,6}) (?P<text>.*)$")
_BLOCK_LEVEL = 3
_GAME_CODE_RE = re.compile(r"^Game code \((?P<build>\d+)\):")
_BUILD_HEADING_RE = re.compile(r"\bBuild \d+\.\d+\.\d+\.(?P<build>\d+)\b")

TABLE_COLUMNS = ("build", "kind", "typedef_index", "class", "member", "rva", "offset")


class Fact(NamedTuple):
    """One game fact a span names. `form` is "rva", "index_offset",
    "index_member" or "name"; the other fields hold that form's parts."""
    form: str
    index: int | None = None
    left: str = ""
    member: str = ""
    address: int | None = None


class Cited(NamedTuple):
    """A fact and where it was cited: `where` is `<file>:<line>`."""
    where: str
    span: str
    fact: Fact


def python_modules(tools_dir: str = TOOLS_DIR) -> frozenset[str]:
    """The name of every `.py` file under tools/, which a dotted span's left
    part may name instead of a game class."""
    names = set()
    for _dir, _subdirs, files in os.walk(tools_dir):
        names.update(f[:-3] for f in files if f.endswith(".py"))
    return frozenset(names)


def game_fact(span: str, modules: frozenset[str] | None = None) -> Fact | None:
    """The game fact a backticked span cites, or None when it cites none.

    This is the one place that decides which spans are game facts. A dotted
    span whose left part names a Python module under tools/ is a Python name,
    and a span the C++ test reads as a file name is a file.
    """
    span = span.strip()
    m = _FACT_RVA_RE.match(span)
    if m:
        if len(span) - 2 < _RVA_MIN_DIGITS:
            return None
        return Fact("rva", address=int(m.group("rva"), 16))
    m = _FACT_INDEX_OFFSET_RE.match(span)
    if m:
        return Fact("index_offset", index=int(m.group("idx")), address=int(m.group("off"), 16))
    m = _FACT_INDEX_MEMBER_RE.match(span)
    if m:
        return Fact("index_member", index=int(m.group("idx")), member=m.group("member"))
    m = _FACT_NAME_RE.match(span)
    if m and not _FILE_EXT_RE.search(span):
        if modules is None:
            modules = python_modules()
        if m.group("left") in modules:
            return None
        return Fact("name", left=m.group("left"), member=m.group("right"))
    return None


def span_builds(lines: list[str]) -> list[tuple[int, str, str]]:
    """Each backticked span of a page with its line number and its build.

    A span in a `###` block takes the build of the block's `Game code (<N>)`
    line; otherwise the build of the nearest heading above it that names
    `Build x.y.z.N`; otherwise DEFAULT_BUILD. Fenced code is not prose and is
    skipped.
    """
    block_build: dict[int, str] = {}
    block = None
    for i, line in enumerate(lines):
        h = _HEADING_RE.match(line)
        if h:
            block = i if len(h.group("hashes")) == _BLOCK_LEVEL else None
            continue
        g = _GAME_CODE_RE.match(line)
        if g and block is not None:
            block_build[block] = g.group("build")
    out: list[tuple[int, str, str]] = []
    sections: list[tuple[int, str]] = []
    block = None
    fenced = False
    for i, line in enumerate(lines):
        if line.strip().startswith("```"):
            fenced = not fenced
            continue
        if fenced:
            continue
        h = _HEADING_RE.match(line)
        if h:
            level = len(h.group("hashes"))
            while sections and sections[-1][0] >= level:
                sections.pop()
            b = _BUILD_HEADING_RE.search(h.group("text"))
            if b:
                sections.append((level, b.group("build")))
            block = i if level == _BLOCK_LEVEL else None
        build = block_build.get(block) if block is not None else None
        if build is None:
            build = sections[-1][1] if sections else DEFAULT_BUILD
        out.extend((i + 1, s, build) for s in _SPAN_RE.findall(line))
    return out


def _display_path(path: str) -> str:
    full = os.path.abspath(path)
    try:
        rel = os.path.relpath(full, _REPO_ROOT)
    except ValueError:  # another drive
        return full.replace("\\", "/")
    return (full if rel.startswith("..") else rel).replace("\\", "/")


def page_facts(pages_dir: str, build: str, modules: frozenset[str] | None = None) -> list[Cited]:
    """Every game fact the `.md` pages under `pages_dir` cite for `build`."""
    if modules is None:
        modules = python_modules()
    cited: list[Cited] = []
    paths = []
    for root, _subdirs, files in os.walk(pages_dir):
        paths.extend(os.path.join(root, f) for f in files if f.endswith(".md"))
    for path in sorted(paths):
        with open(path, encoding="utf-8") as f:
            lines = f.read().splitlines()
        shown = _display_path(path)
        for lineno, span, span_build in span_builds(lines):
            if span_build != build:
                continue
            fact = game_fact(span, modules)
            if fact is not None:
                cited.append(Cited(f"{shown}:{lineno}", span, fact))
    return cited


def _is_data_address(name: str) -> bool:
    return name.startswith(_DATA_ADDRESS_PREFIXES) or name in _DATA_ADDRESS_NAMES


def constants_facts(constants_path: str, classes: list[dict]) -> tuple[list[Cited], list[str]]:
    """The method addresses and engine offsets constants.py names, as facts.

    Every `RVA_*` but the data addresses is a method address; every `OFF_*`
    is an offset into the drum engine object, the class whose `.ctor` sits at
    the file's own engine-constructor RVA.
    """
    with open(constants_path, encoding="utf-8") as f:
        tree = ast.parse(f.read(), constants_path)
    values: list[tuple[str, int, int]] = []
    for node in tree.body:
        if (isinstance(node, ast.Assign) and len(node.targets) == 1
                and isinstance(node.targets[0], ast.Name)
                and isinstance(node.value, ast.Constant) and type(node.value.value) is int):
            values.append((node.targets[0].id, node.value.value, node.lineno))
    shown = _display_path(constants_path)
    by_name = {name: value for name, value, _ in values}
    cited: list[Cited] = []
    errors: list[str] = []
    engine_index = None
    if _ENGINE_CTOR_NAME not in by_name:
        errors.append(f"{shown}: no {_ENGINE_CTOR_NAME}, so the OFF_* offsets have no class")
    else:
        try:
            engine_index = find_by_ctor_rva(classes, by_name[_ENGINE_CTOR_NAME])["typedef_index"]
        except FactsError as exc:
            errors.append(f"{shown}: {_ENGINE_CTOR_NAME}: {exc}")
    for name, value, lineno in values:
        where = f"{shown}:{lineno}"
        if name.startswith("RVA_") and not _is_data_address(name):
            cited.append(Cited(where, name, Fact("rva", address=value)))
        elif name.startswith("OFF_") and engine_index is not None:
            cited.append(Cited(where, name, Fact("index_offset", index=engine_index, address=value)))
    return cited, errors


class _Index:
    """The dump's classes, indexed for the table's lookups."""

    def __init__(self, classes: list[dict]):
        self.by_index: dict[int, list[dict]] = {}
        self.by_name: dict[str, list[dict]] = {}
        self.by_bare: dict[str, list[dict]] = {}
        self.by_full: dict[str, list[dict]] = {}
        self.by_rva: dict[int, list[tuple[dict, dict]]] = {}
        for c in classes:
            self.by_index.setdefault(c["typedef_index"], []).append(c)
            self.by_name.setdefault(c["name"], []).append(c)
            self.by_bare.setdefault(_bare_name(c["name"]), []).append(c)
            if c.get("namespace"):
                self.by_full.setdefault(f"{c['namespace']}.{c['name']}", []).append(c)
            for m in c["methods"]:
                if isinstance(m.get("rva"), int) and m["rva"] >= 0:
                    self.by_rva.setdefault(m["rva"], []).append((c, m))

    def chain(self, cls: dict) -> list[dict]:
        """The class and its base classes, nearest first. A base the dump
        does not hold exactly once ends the chain."""
        out = [cls]
        while out[-1].get("base"):
            found = self.by_bare.get(_bare_name(out[-1]["base"]), [])
            if len(found) != 1 or found[0] in out:
                break
            out.append(found[0])
        return out


def _member_matches(index: _Index, cls: dict, test) -> list[tuple[str, dict]]:
    """The nearest class in `cls`'s chain with a member passing `test`, and
    every such member there: ("field" or "method", member)."""
    for c in index.chain(cls):
        found = [("field", f) for f in c["fields"] if test("field", f)]
        found += [("method", m) for m in c["methods"] if test("method", m)]
        if found:
            return found
    return []


def _row(build: str, kind: str, cls: dict, member: str = "", rva=None, offset=None) -> tuple:
    return (
        build,
        kind,
        str(cls["typedef_index"]),
        cls["name"] if cls.get("readable") else "",
        member,
        f"0x{rva:X}" if isinstance(rva, int) and rva >= 0 else "",
        f"0x{offset:X}" if isinstance(offset, int) else "",
    )


def _member_row(build: str, cls: dict, kind: str, member: dict) -> tuple:
    if kind == "field":
        return _row(build, kind, cls, member["name"], offset=member.get("offset"))
    return _row(build, kind, cls, member["name"], rva=member.get("rva"))


def _lookup(index: _Index, build: str, fact: Fact) -> tuple[list[tuple], str]:
    """The table rows for one fact, or an empty list and the reason."""
    if fact.form == "rva":
        found = index.by_rva.get(fact.address, [])
        if not found:
            return [], "matches no method address"
        # Two methods can share one address; each gets its row.
        return [_row(build, "method", c, m["name"], rva=m["rva"]) for c, m in found], ""
    if fact.form in ("index_offset", "index_member"):
        classes = index.by_index.get(fact.index, [])
        if len(classes) != 1:
            return [], f"TypeDefIndex {fact.index} matches {len(classes)} classes"
        cls = classes[0]
        if fact.form == "index_offset":
            found = _member_matches(index, cls, lambda kind, m: kind == "field" and not m["static"]
                                    and m.get("offset") == fact.address)
        else:
            found = _member_matches(index, cls, lambda kind, m: m["name"] == fact.member)
        if len(found) != 1:
            return [], f"matches {len(found)} members of TypeDefIndex {fact.index} and its bases"
        return [_member_row(build, cls, *found[0])], ""
    full = f"{fact.left}.{fact.member}"
    rows = [_row(build, "enum" if c["kind"] == "enum" else "class", c)
            for c in index.by_name.get(full, []) + index.by_full.get(full, [])]
    owners = index.by_name.get(fact.left, [])
    if len(owners) > 1:
        return [], f"class {fact.left} matches {len(owners)} classes"
    for cls in owners:
        rows += [_member_row(build, cls, kind, m) for kind, m in
                 _member_matches(index, cls, lambda kind, m: m["name"] == fact.member)]
    if len(rows) != 1:
        return [], f"matches {len(rows)} classes and members"
    return rows, ""


def table_rows(classes: list[dict], build: str, cited: list[Cited]) -> tuple[list[tuple], list[str]]:
    """One row per distinct fact, sorted, and an error for each fact that is
    not exactly one dump entry (an address several methods share excepted)."""
    index = _Index(classes)
    rows: set[tuple] = set()
    errors: list[str] = []
    for c in cited:
        found, reason = _lookup(index, build, c.fact)
        if found:
            rows.update(found)
        else:
            errors.append(f"{c.where}: `{c.span}` {reason} in build {build}")
    return sorted(rows), errors


def file_md5(path: str) -> str:
    """The MD5 of a file's bytes, read now."""
    digest = hashlib.md5()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def render_table(version: str, dll_path: str, rows: list[tuple]) -> str:
    lines = [
        "# Clone Hero game facts the pages cite; written by tools/ch_mcp/dump_facts.py --tsv",
        f"# version: {version}",
        f"# {os.path.basename(dll_path)} md5: {file_md5(dll_path)}",
        "\t".join(TABLE_COLUMNS),
    ]
    lines += ["\t".join(r) for r in rows]
    return "\n".join(lines) + "\n"


def load_classes(dump_path: str, version: str | None) -> tuple[list[dict], str]:
    """The classes of a dump.cs or a facts.json, and the build's version
    string. A facts.json carries its own version; a dump.cs needs one given."""
    with open(dump_path, encoding="utf-8") as f:
        text = f.read()
    if dump_path.lower().endswith(".json"):
        facts = json.loads(text)
        described = facts.get("game_version")
        if version is not None and version != described:
            raise FactsError(f"{dump_path} describes {described}, not {version}")
        return facts["classes"], described
    if version is None:
        raise FactsError("a dump.cs needs --version")
    return parse_dump(text), version


def build_table(dump_path: str, pages_dir: str, dll_path: str, version: str | None = None,
                constants_path: str = CONSTANTS_PATH) -> tuple[str, list[str]]:
    """The facts table text for one build, and every lookup error."""
    from tools.ch_mcp.build_facts import BuildFactsError, build_number

    classes, version = load_classes(dump_path, version)
    try:
        build = build_number(version)
    except BuildFactsError as exc:
        raise FactsError(str(exc)) from exc
    cited = page_facts(pages_dir, build)
    errors: list[str] = []
    if build == DEFAULT_BUILD:
        from_constants, errors = constants_facts(constants_path, classes)
        cited += from_constants
    rows, lookup_errors = table_rows(classes, build, cited)
    return render_table(version, dll_path, rows), errors + lookup_errors


def _table_main(args) -> int:
    if not args.out or not args.dll:
        print("dump_facts: --tsv needs --out and --dll", file=sys.stderr)
        return 1
    try:
        text, errors = build_table(args.dump_cs, args.tsv, args.dll, args.version, args.constants)
    except (FactsError, OSError) as exc:
        print(f"dump_facts: {exc}", file=sys.stderr)
        return 1
    for e in errors:
        print(f"dump_facts: {e}", file=sys.stderr)
    if errors:
        return 1
    if args.check:
        try:
            with open(args.out, encoding="utf-8") as f:
                old = f.read()
        except OSError as exc:
            print(f"dump_facts: {exc}", file=sys.stderr)
            return 1
        if old != text:
            sys.stdout.writelines(difflib.unified_diff(
                old.splitlines(keepends=True), text.splitlines(keepends=True), args.out, "dump"))
            print(f"dump_facts: {args.out} differs from the dump", file=sys.stderr)
            return 1
        print(f"{args.out} matches the dump")
        return 0
    with open(args.out, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)
    print(f"wrote {args.out}")
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Write facts.json beside an Il2CppDumper dump.cs, or the facts table the pages cite.")
    parser.add_argument("dump_cs", help="path to dump.cs (or, with --tsv, a facts.json)")
    parser.add_argument("--version", help="the game's version string, stored as given")
    parser.add_argument("--tsv", metavar="PAGES", help="write the facts table the .md pages under PAGES cite")
    parser.add_argument("--out", help="with --tsv: the table file")
    parser.add_argument("--dll", help="with --tsv: the game DLL whose MD5 the header carries")
    parser.add_argument("--constants", default=CONSTANTS_PATH, help="with --tsv: the constants.py to read")
    parser.add_argument("--check", action="store_true",
                        help="with --tsv: compare --out with the dump and exit 1 on a difference")
    parser.add_argument(
        "--engine-ctor-rva",
        type=lambda s: int(s, 0),
        default=probe_constants.RVA_DRUMS_ENGINE_CTOR,
        help="RVA of the drum engine's constructor (default: ch_probe.constants.RVA_DRUMS_ENGINE_CTOR)",
    )
    args = parser.parse_args(argv)
    # Scrambled names are not in a Windows console's code page; escape them
    # rather than crash after the file is written.
    for stream in (sys.stdout, sys.stderr):
        if hasattr(stream, "reconfigure"):
            stream.reconfigure(errors="backslashreplace")
    if args.tsv:
        return _table_main(args)
    if args.version is None:
        print("dump_facts: writing facts.json needs --version", file=sys.stderr)
        return 1
    try:
        out_path, facts = write_facts(args.dump_cs, args.version, args.engine_ctor_rva)
    except (FactsError, OSError) as exc:
        print(f"dump_facts: {exc}", file=sys.stderr)
        return 1
    engine = facts["signatures"][ENGINE_SIGNATURE]
    print(f"classes: {len(facts['classes'])}")
    print(
        f"drums engine: {engine['name']} (TypeDefIndex {engine['typedef_index']}), "
        f".ctor at RVA 0x{engine['ctor_rva']:X}, base {engine['base']}"
    )
    print(f"wrote {out_path} ({os.path.getsize(out_path)} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
