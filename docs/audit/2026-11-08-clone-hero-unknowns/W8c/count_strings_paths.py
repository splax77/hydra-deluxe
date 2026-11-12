"""Recount IN-45's two numbers for build 6142: the game's string literals and
the source file paths its metadata names.

It counts, and prints each count with where it came from:
  1. entries in the dump's stringliteral.json, and how many distinct values;
  2. distinct `.cs` paths (any text ending in ".cs" that has a slash or
     backslash in it) in dump.cs, in stringliteral.json and in the raw bytes
     of the game's global-metadata.dat, plus how many of each start with
     "Assets".

Run from the repo root:
    py -I docs/audit/2026-11-08-clone-hero-unknowns/W8c/count_strings_paths.py
"""

import json
import os
import re
import sys

DUMPS = r"C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142"
PATH_TEXT = re.compile(rb"[A-Za-z0-9_\-. ]*(?:[\\/][A-Za-z0-9_\-. ]+)+\.cs(?![A-Za-z0-9])")


def paths_in(blob):
    # A match can start on the byte before the path (in global-metadata.dat
    # that byte is a length, not text), so each match is counted from its
    # first slash on.
    found = set()
    for m in PATH_TEXT.finditer(blob):
        text = m.group(0)
        found.add(text[re.search(rb"[\\/]", text).start():])
    return found


def show(label, found):
    assets = sum(1 for p in found if p.lstrip(b"\\/").startswith(b"Assets"))
    scripts = sum(1 for p in found if p.lstrip(b"\\/").startswith(b"Assets\\Scripts"))
    print(f"{label}: {len(found)} distinct .cs paths, {assets} starting with 'Assets', "
          f"{scripts} under 'Assets\\Scripts'")
    for p in sorted(found)[:3]:
        print(f"    e.g. {p.decode('utf-8', 'replace')}")


def main():
    with open(os.path.join(DUMPS, "stringliteral.json"), encoding="utf-8") as f:
        literals = json.load(f)
    print(f"stringliteral.json: {len(literals)} entries, "
          f"{len(set(e['value'] for e in literals))} distinct values")
    show("dump.cs", paths_in(open(os.path.join(DUMPS, "dump.cs"), "rb").read()))
    show("stringliteral.json values",
         paths_in("\n".join(e["value"] for e in literals).encode("utf-8")))
    sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", "..")))
    from tools.ch_mcp import game_files
    meta = os.path.join(game_files.GAME_DIR, "Clone Hero_Data", "il2cpp_data", "Metadata",
                        "global-metadata.dat")
    if os.path.isfile(meta):
        show("global-metadata.dat bytes", paths_in(open(meta, "rb").read()))
    else:
        print(f"global-metadata.dat: not found at the expected place under the game folder")


if __name__ == "__main__":
    main()
