"""Read an Il2CppDumper dump.cs and write facts.json beside it, or write the
facts table that the Clone Hero pages cite.

facts.json lists every class in the dump with its fields and methods, plus a
`signatures` block for the classes the Clone Hero MCP tests need by name. The
schema is described in plain sentences in tools/ch_mcp/README.md; that page is
its one description, so this file does not repeat it.

The facts table (`--tsv`) holds one row for each game fact the pages under a
folder cite, and each fact `tools/ch_probe/constants.py` names, looked up in
one build's dump. The plan 2026-10-30-clone-hero-reference.md, question 2,
gives its columns, and TABLE_COLUMNS adds `cited`, the span as the page
writes it, which is what the C++ test matches a page span by. A page writes a
scrambled game name by our readable name for it, from that build's names map
(`names_path`, columns NAMES_COLUMNS); `_lookup` resolves those names, and
`check_names` checks the map against the dump. Which backticked spans count as game facts is decided by
`game_fact` alone; the C++ docs test (tests/test_docs_match_code.cpp) applies
the same forms, so compare the two when either changes. Which build a span
belongs to is decided by `span_builds`, which mirrors that test's rule too.
A method address that dump.cs prints only inside a generic method's
`GenericInstMethod` block (`parse_dump` collects those as `instances`) also
gets a method row; facts.json does not carry them, so a facts.json holds no
such rows.
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
# A generic method's `/* GenericInstMethod : ... */` block lists each address its
# instantiations were compiled at (`|-RVA:`), and under it the instantiations
# (`|-<class>.<method>`).
_INSTANCE_RVA_RE = re.compile(rf"^\t\|-RVA: (?P<rva>-1|{_HEX_ADDRESS}) ")
_INSTANCE_NAME_RE = re.compile(r"^\t\|-(?P<name>.+)$")
_INSTANCE_END = "\t*/"


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


def _split_instance(name: str, method: str) -> tuple[str, str]:
    """Split an instantiation line into its class and its method as written.

    `method` is the generic method the block follows. The line is that name,
    with its own `<...>` arguments, after the class's last top-level dot; when
    no such dot is found, the first top-level dot divides them.
    """
    dots: list[int] = []
    depth = 0
    for i, ch in enumerate(name):
        if ch == "<":
            depth += 1
        elif ch == ">":
            depth -= 1
        elif ch == "." and depth == 0:
            dots.append(i)
    for i in reversed(dots):
        rest = name[i + 1:]
        if rest.startswith(method) and (len(rest) == len(method) or rest[len(method)] == "<"):
            return name[:i], rest
    if dots:
        return name[:dots[0]], name[dots[0] + 1:]
    return "", name


def parse_dump(text: str, instances: list[dict] | None = None) -> list[dict]:
    """Every class in a dump.cs text, in file order.

    The methods a generic method's `GenericInstMethod` block lists are not
    class members, so they stay out of the result. Pass a list as `instances`
    to collect them: one record per instantiation with the TypeDefIndex of the
    class holding the generic method, the instantiated `class` and `method` as
    the dump writes them, and the `rva`. facts.json does not carry them.
    """
    classes: list[dict] = []
    namespace = ""
    current: dict | None = None
    section = ""
    pending_rva: int | None = None
    instance_rva: int | None = None
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
        if line == _INSTANCE_END:
            instance_rva = None
            continue
        m = _INSTANCE_RVA_RE.match(line)
        if m:
            rva = m.group("rva")
            instance_rva = None if rva == "-1" else int(rva, 16)  # -1: an abstract method has no address
            continue
        m = _INSTANCE_NAME_RE.match(line)
        if m:
            if instances is not None and instance_rva is not None and current["methods"]:
                owner, method = _split_instance(m.group("name"), current["methods"][-1]["name"])
                instances.append({"typedef_index": current["typedef_index"], "class": owner,
                                  "method": method, "rva": instance_rva})
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
# A file under `code/<build>/` in the pages folder is one build's notes, so
# its place gives the build of every span in it.
_PLACE_BUILD_RE = re.compile(r"^code/(?P<build>\d+)/")

# `cited` is the span as the page writes it (a constants.py name for a row
# that file gives), so the C++ test matches a page span by that column alone
# and resolution stays here.
TABLE_COLUMNS = ("build", "kind", "typedef_index", "class", "member", "rva", "offset", "cited")

# The names map: our readable names for the game's scrambled classes and
# members, one file per build beside the facts tables. The pages write these
# names, never the scrambled ones.
NAMES_COLUMNS = ("build", "typedef_index", "class", "member", "name", "what")
# A Mono build's TypeDefIndex restarts in each assembly (tools/ch_mcp/README.md),
# so that build's map names its classes by name alone.
_BUILDS_KEYED_BY_NAME = frozenset({"4045"})
# The builds whose facts.json lists no properties (mono_facts.py reads only
# fields and methods), so a map row naming a property cannot be checked there.
# A page span that resolves to one gets a table row of this kind, from the
# map alone.
_BUILDS_WITHOUT_PROPERTIES = frozenset({"4045"})
UNLISTED_PROPERTY_KIND = "property-unlisted"
_IDENTIFIER_RE = re.compile(r"^[A-Za-z_]\w*$", re.ASCII)


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


class NameRow(NamedTuple):
    """One row of a names map. `where` is `<file>:<line>`. `typedef_index` is
    None in a build keyed by name, and `member` is empty on a class row."""
    where: str
    typedef_index: int | None
    cls: str
    member: str
    name: str
    what: str


def names_path(pages_dir: str, build: str) -> str:
    """Where one build's names map sits under the pages folder."""
    return os.path.join(pages_dir, "code", f"names-{build}.tsv")


def load_names(path: str, build: str) -> tuple[list[NameRow], list[str]]:
    """The rows of one build's names map, and an error for each malformed
    line. A missing file is an empty map."""
    if not os.path.isfile(path):
        return [], []
    with open(path, encoding="utf-8") as f:
        lines = f.read().splitlines()
    shown = _display_path(path)
    keyed_by_index = build not in _BUILDS_KEYED_BY_NAME
    rows: list[NameRow] = []
    errors: list[str] = []
    columns_seen = False
    for lineno, line in enumerate(lines, start=1):
        if not line.strip() or line.startswith("#"):
            continue
        where = f"{shown}:{lineno}"
        if not columns_seen:
            columns_seen = True
            if line != "\t".join(NAMES_COLUMNS):
                errors.append(f"{where}: the first line after the # comments is not the column line")
            continue
        cols = line.split("\t")
        if len(cols) != len(NAMES_COLUMNS):
            errors.append(f"{where}: {len(cols)} tab-separated columns, not {len(NAMES_COLUMNS)}")
            continue
        row_build, idx, cls, member, name, what = cols
        problems = []
        if row_build != build:
            problems.append(f"the build is {row_build!r}, not {build}")
        if keyed_by_index and not re.fullmatch(r"\d+", idx, re.ASCII):
            problems.append(f"build {build} names a class by its TypeDefIndex, and {idx!r} is not one")
        if not keyed_by_index and idx:
            problems.append(f"build {build} names a class by name alone, so typedef_index stays empty")
        if not cls:
            problems.append("the class column is empty")
        if not _IDENTIFIER_RE.match(name):
            problems.append(f"the name {name!r} is not an ASCII identifier")
        if not what.strip():
            problems.append("the what column is empty")
        if problems:
            errors.extend(f"{where}: {p}" for p in problems)
            continue
        rows.append(NameRow(where, int(idx) if keyed_by_index else None, cls, member, name, what))
    return rows, errors


class NamesMap:
    """One build's names map, indexed for resolution. `used` collects the
    rows a page span resolved through."""

    def __init__(self, rows: list[NameRow], build: str):
        self.rows = rows
        self.keyed_by_index = build not in _BUILDS_KEYED_BY_NAME
        self.has_properties = build not in _BUILDS_WITHOUT_PROPERTIES
        self.class_rows: dict[str, list[NameRow]] = {}
        self.member_rows: dict[object, dict[str, list[NameRow]]] = {}
        self.used: set[NameRow] = set()
        for r in rows:
            if r.member:
                self.member_rows.setdefault(self.row_key(r), {}).setdefault(r.name, []).append(r)
            else:
                self.class_rows.setdefault(r.name, []).append(r)

    def row_key(self, row: NameRow):
        return row.typedef_index if self.keyed_by_index else row.cls

    def class_key(self, cls: dict):
        return cls["typedef_index"] if self.keyed_by_index else cls["name"]

    def dump_classes(self, index: "_Index", row: NameRow) -> list[dict]:
        """The dump classes a map row's class names."""
        if self.keyed_by_index:
            return index.by_index.get(row.typedef_index, [])
        return index.by_name.get(row.cls, [])

    def renamed_classes(self, index: "_Index", name: str) -> list[tuple[dict, NameRow]]:
        """The dump classes our class name `name` stands for, each with its row."""
        return [(c, r) for r in self.class_rows.get(name, []) for c in self.dump_classes(index, r)]

    def member_rows_named(self, cls: dict, name: str) -> list[NameRow]:
        """The map rows that give a member of `cls` our name `name`."""
        return self.member_rows.get(self.class_key(cls), {}).get(name, [])


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


def place_build(path: str, pages_dir: str) -> str | None:
    """The build a page's place under `pages_dir` gives, or None. A file under
    `code/<build>/` takes that build for every span in it."""
    rel = os.path.relpath(os.path.abspath(path), os.path.abspath(pages_dir)).replace("\\", "/")
    m = _PLACE_BUILD_RE.match(rel)
    return m.group("build") if m else None


def span_builds(lines: list[str], place: str | None = None) -> list[tuple[int, str, str]]:
    """Each backticked span of a page with its line number and its build.

    A page whose place gives a build (`place`, from `place_build`) puts every
    span in that build. Otherwise a span in a `###` block takes the build of
    the block's `Game code (<N>)` line; otherwise the build of the nearest
    heading above it that names `Build x.y.z.N`; otherwise DEFAULT_BUILD.
    Fenced code is not prose and is skipped.
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
        build = place or build
        out.extend((i + 1, s, build) for s in _SPAN_RE.findall(line))
    return out


def _display_path(path: str) -> str:
    full = os.path.abspath(path)
    try:
        rel = os.path.relpath(full, _REPO_ROOT)
    except ValueError:  # another drive
        return full.replace("\\", "/")
    return (full if rel.startswith("..") else rel).replace("\\", "/")


def page_facts(pages_dir: str, build: str,
               modules: frozenset[str] | None = None) -> tuple[list[Cited], list[str]]:
    """Every game fact the `.md` pages under `pages_dir` cite for `build`, and
    an error for each of that build's spans holding a non-ASCII character: the
    game's scrambled names are written on a page by their names-map name."""
    if modules is None:
        modules = python_modules()
    cited: list[Cited] = []
    errors: list[str] = []
    shown_map = _display_path(names_path(pages_dir, build))
    paths = []
    for root, _subdirs, files in os.walk(pages_dir):
        paths.extend(os.path.join(root, f) for f in files if f.endswith(".md"))
    for path in sorted(paths):
        with open(path, encoding="utf-8") as f:
            lines = f.read().splitlines()
        shown = _display_path(path)
        for lineno, span, span_build in span_builds(lines, place_build(path, pages_dir)):
            if span_build != build:
                continue
            if not span.isascii():
                errors.append(f"{shown}:{lineno}: `{span}` holds a non-ASCII letter; write the readable "
                              f"name from {shown_map} instead")
                continue
            fact = game_fact(span, modules)
            if fact is not None:
                cited.append(Cited(f"{shown}:{lineno}", span.strip(), fact))
    return cited, errors


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
    """The dump's classes, indexed for the table's lookups. `instances` are
    the generic instantiations `parse_dump` collects; each is a method at its
    address, in the instantiated class the dump names."""

    def __init__(self, classes: list[dict], instances: list[dict] = ()):
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
        for inst in instances:
            owner = {"typedef_index": inst["typedef_index"], "name": inst["class"],
                     "readable": is_readable(inst["class"])}
            method = {"name": inst["method"], "rva": inst["rva"]}
            self.by_rva.setdefault(inst["rva"], []).append((owner, method))

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
    every such member there: ("field" or "method", member). `test` gets the
    class in the chain, the kind and the member."""
    for c in index.chain(cls):
        found = [("field", f) for f in c["fields"] if test(c, "field", f)]
        found += [("method", m) for m in c["methods"] if test(c, "method", m)]
        if found:
            return found
    return []


def _named(names: NamesMap, member: str, via: list[NameRow]):
    """A `_member_matches` test for a member a page names: by the dump's name,
    or by our name for it in the map. A map row it matches through goes in `via`."""
    def test(c: dict, _kind: str, m: dict) -> bool:
        if m["name"] == member:
            return True
        rows = [r for r in names.member_rows_named(c, member) if r.member == m["name"]]
        via.extend(rows)
        return bool(rows)
    return test


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


def _unlisted_property_rows(index: _Index, build: str, cls: dict, member: str, names: NamesMap,
                            via: list[NameRow]) -> list[tuple]:
    """The rows for a member our map names that the dump does not list. In a
    build whose facts.json lists no properties, a map row whose member is no
    field or method of its class is a property; the nearest class in `cls`'s
    chain with such a row gives it. A build that lists its properties has none."""
    if names.has_properties:
        return []
    for c in index.chain(cls):
        listed = {m["name"] for m in c["fields"] + c["methods"]}
        found = [r for r in names.member_rows_named(c, member) if r.member not in listed]
        if found:
            via.extend(found)
            return [_row(build, UNLISTED_PROPERTY_KIND, c, r.member) for r in found]
    return []


def _lookup(index: _Index, build: str, fact: Fact,
            names: NamesMap) -> tuple[list[tuple], str, list[NameRow]]:
    """The table rows for one fact, or an empty list and the reason, and the
    names-map rows the lookup went through. A class or member part may be the
    dump's name or our name for it in `names`."""
    via: list[NameRow] = []
    if fact.form == "rva":
        found = index.by_rva.get(fact.address, [])
        if not found:
            return [], "matches no method address", via
        # Two methods can share one address; each gets its row.
        return [_row(build, "method", c, m["name"], rva=m["rva"]) for c, m in found], "", via
    if fact.form in ("index_offset", "index_member"):
        classes = index.by_index.get(fact.index, [])
        if len(classes) != 1:
            return [], f"TypeDefIndex {fact.index} matches {len(classes)} classes", via
        cls = classes[0]
        if fact.form == "index_offset":
            found = _member_matches(index, cls, lambda _c, kind, m: kind == "field" and not m["static"]
                                    and m.get("offset") == fact.address)
        else:
            found = _member_matches(index, cls, _named(names, fact.member, via))
            if not found:
                unlisted = _unlisted_property_rows(index, build, cls, fact.member, names, via)
                if len(unlisted) == 1:
                    return unlisted, "", via
        if len(found) != 1:
            return [], f"matches {len(found)} members of TypeDefIndex {fact.index} and its bases", via
        return [_member_row(build, cls, *found[0])], "", via
    full = f"{fact.left}.{fact.member}"
    rows = [_row(build, "enum" if c["kind"] == "enum" else "class", c)
            for c in index.by_name.get(full, []) + index.by_full.get(full, [])]
    owners = list(index.by_name.get(fact.left, []))
    for c, r in names.renamed_classes(index, fact.left):
        via.append(r)
        if not any(c is o for o in owners):
            owners.append(c)
    if len(owners) > 1:
        return [], f"class {fact.left} matches {len(owners)} classes", via
    for cls in owners:
        listed = _member_matches(index, cls, _named(names, fact.member, via))
        rows += ([_member_row(build, cls, kind, m) for kind, m in listed] if listed
                 else _unlisted_property_rows(index, build, cls, fact.member, names, via))
    if len(rows) != 1:
        return [], f"matches {len(rows)} classes and members", via
    return rows, "", via


def table_rows(classes: list[dict], build: str, cited: list[Cited], instances: list[dict] = (),
               names: NamesMap | None = None,
               warnings: list[str] | None = None) -> tuple[list[tuple], list[str]]:
    """One row per distinct fact and span, sorted, and an error for each fact
    that is not exactly one dump entry (an address several methods share
    excepted). The rows `names` resolved through are added to `names.used`. A
    span that resolves to a property the dump does not list gets a row of
    kind UNLISTED_PROPERTY_KIND and a line in `warnings` when a list is given."""
    if names is None:
        names = NamesMap([], build)
    index = _Index(classes, instances)
    rows: set[tuple] = set()
    errors: list[str] = []
    for c in cited:
        found, reason, via = _lookup(index, build, c.fact, names)
        if found:
            rows.update(r + (c.span,) for r in found)
            names.used.update(via)
            if warnings is not None and any(r[1] == UNLISTED_PROPERTY_KIND for r in found):
                warnings.append(f"{c.where}: `{c.span}` is a property that build {build}'s facts.json does "
                                "not list, so its table row comes from the names map alone")
        else:
            errors.append(f"{c.where}: `{c.span}` {reason} in build {build}")
    return sorted(rows), errors


def check_names(classes: list[dict], names: NamesMap) -> tuple[list[str], list[str]]:
    """The errors and warnings in a names map, read against the dump.

    Each row's class and member exist in the dump. Our class name is unique
    and is no class's real name; our member name is unique within its class
    and is none of that class's real member names; a member has one name. A
    row no page span resolved through (`names.used`) is a warning.
    """
    index = _Index(classes)
    errors: list[str] = []
    warnings: list[str] = []
    class_named: dict[str, NameRow] = {}
    class_row: dict[object, NameRow] = {}
    member_named: dict[tuple, NameRow] = {}
    member_row: dict[tuple, NameRow] = {}
    for r in names.rows:
        found = names.dump_classes(index, r)
        if names.keyed_by_index:
            if len(found) != 1:
                errors.append(f"{r.where}: TypeDefIndex {r.typedef_index} matches {len(found)} classes")
                continue
            if found[0]["name"] != r.cls:
                errors.append(f"{r.where}: TypeDefIndex {r.typedef_index} is the class `{found[0]['name']}` "
                              f"in the dump, not `{r.cls}`")
                continue
        elif len(found) != 1:
            errors.append(f"{r.where}: the class `{r.cls}` matches {len(found)} classes")
            continue
        cls = found[0]
        key = names.class_key(cls)
        real_members = {m["name"] for m in cls["fields"] + cls["methods"]}
        if not r.member:
            if r.name in index.by_name:
                errors.append(f"{r.where}: `{r.name}` is a real class name in the dump")
            if r.name in class_named:
                errors.append(f"{r.where}: `{r.name}` names another class too, at {class_named[r.name].where}")
            if key in class_row:
                errors.append(f"{r.where}: the class has another name too, at {class_row[key].where}")
            class_named.setdefault(r.name, r)
            class_row.setdefault(key, r)
        else:
            if r.member not in real_members:
                if names.has_properties:
                    errors.append(f"{r.where}: the class `{r.cls}` has no member `{r.member}`")
                else:
                    warnings.append(f"{r.where}: the class `{r.cls}` has no field or method `{r.member}`, "
                                    "and facts.json has no properties to check it against")
            if r.name in real_members:
                errors.append(f"{r.where}: `{r.name}` is a real member name of the class `{r.cls}`")
            if (key, r.name) in member_named:
                errors.append(f"{r.where}: `{r.name}` names another member of the class too, at "
                              f"{member_named[(key, r.name)].where}")
            if (key, r.member) in member_row:
                errors.append(f"{r.where}: the member has another name too, at {member_row[(key, r.member)].where}")
            member_named.setdefault((key, r.name), r)
            member_row.setdefault((key, r.member), r)
        if r not in names.used:
            warnings.append(f"{r.where}: `{r.name}` is cited by no page")
    return errors, warnings


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


def load_classes(dump_path: str, version: str | None) -> tuple[list[dict], list[dict], str]:
    """The classes of a dump.cs or a facts.json, the generic instantiations
    (only a dump.cs holds them), and the build's version string. A facts.json
    carries its own version; a dump.cs needs one given."""
    with open(dump_path, encoding="utf-8") as f:
        text = f.read()
    if dump_path.lower().endswith(".json"):
        facts = json.loads(text)
        described = facts.get("game_version")
        if version is not None and version != described:
            raise FactsError(f"{dump_path} describes {described}, not {version}")
        return facts["classes"], [], described
    if version is None:
        raise FactsError("a dump.cs needs --version")
    instances: list[dict] = []
    return parse_dump(text, instances), instances, version


def build_table(dump_path: str, pages_dir: str, dll_path: str, version: str | None = None,
                constants_path: str = CONSTANTS_PATH,
                warnings: list[str] | None = None) -> tuple[str, list[str]]:
    """The facts table text for one build, and every error: in the pages, in
    the lookups and in the build's names map (`names_path`). The map's
    warnings go in `warnings` when a list is given."""
    from tools.ch_mcp.build_facts import BuildFactsError, build_number

    classes, instances, version = load_classes(dump_path, version)
    try:
        build = build_number(version)
    except BuildFactsError as exc:
        raise FactsError(str(exc)) from exc
    name_rows, errors = load_names(names_path(pages_dir, build), build)
    names = NamesMap(name_rows, build)
    cited, page_errors = page_facts(pages_dir, build)
    errors += page_errors
    if build == DEFAULT_BUILD:
        from_constants, constants_errors = constants_facts(constants_path, classes)
        cited += from_constants
        errors += constants_errors
    lookup_warnings: list[str] = []
    rows, lookup_errors = table_rows(classes, build, cited, instances, names, lookup_warnings)
    map_errors, map_warnings = check_names(classes, names)
    if warnings is not None:
        warnings.extend(lookup_warnings + map_warnings)
    return render_table(version, dll_path, rows), errors + lookup_errors + map_errors


def _table_main(args) -> int:
    if not args.out or not args.dll:
        print("dump_facts: --tsv needs --out and --dll", file=sys.stderr)
        return 1
    warnings: list[str] = []
    try:
        text, errors = build_table(args.dump_cs, args.tsv, args.dll, args.version, args.constants, warnings)
    except (FactsError, OSError) as exc:
        print(f"dump_facts: {exc}", file=sys.stderr)
        return 1
    for w in warnings:
        print(f"dump_facts: warning: {w}", file=sys.stderr)
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
