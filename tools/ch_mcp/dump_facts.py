"""Read an Il2CppDumper dump.cs and write facts.json beside it.

facts.json lists every class in the dump with its fields and methods, plus a
`signatures` block for the classes the Clone Hero MCP tests need by name. The
schema is described in plain sentences in tools/ch_mcp/README.md; that page is
its one description, so this file does not repeat it.

Run:
    py tools/ch_mcp/dump_facts.py <dump.cs> --version 1.1.0.6142-final
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys

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
_RVA_RE = re.compile(r"^\t// RVA: (?P<rva>-1|0x[0-9A-Fa-f]+) ")
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


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Write facts.json beside an Il2CppDumper dump.cs.")
    parser.add_argument("dump_cs", help="path to dump.cs")
    parser.add_argument("--version", required=True, help="the game's version string, stored as given")
    parser.add_argument(
        "--engine-ctor-rva",
        type=lambda s: int(s, 0),
        default=probe_constants.RVA_DRUMS_ENGINE_CTOR,
        help="RVA of the drum engine's constructor (default: ch_probe.constants.RVA_DRUMS_ENGINE_CTOR)",
    )
    args = parser.parse_args(argv)
    # Scrambled names are not in a Windows console's code page; escape them
    # rather than crash after the file is written.
    sys.stdout.reconfigure(errors="backslashreplace")
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
