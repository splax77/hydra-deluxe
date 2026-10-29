"""Read a Mono build's game assemblies and write facts.json in the same shape
dump_facts.py writes for an IL2CPP build.

A Mono build (Clone Hero 1.0) ships its game code as ordinary .NET assemblies,
so there is no dump.cs. mono_reader.ps1 reads their metadata; this file turns
what it prints into facts.json. The schema, and what this reader cannot fill
in, are described in tools/ch_mcp/README.md.

Run:
    py tools/ch_mcp/mono_facts.py --game "C:\\Clone Hero\\1.0 old\\Clone Hero"
        --version v1.0.0.4045-master --out <folder>
"""

from __future__ import annotations

import argparse
import glob
import json
import os
import shutil
import subprocess
import sys
import tempfile

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import dump_facts  # noqa: E402
from tools.ch_mcp.dump_facts import FactsError  # noqa: E402

# The 1.0 files that hold the code 1.1 ships as CloneHero.dll, StrikeCore.dll,
# CHNet.Core.dll and CHNet.Server.dll, plus Unity's firstpass assembly.
GAME_ASSEMBLIES = (
    "Assembly-CSharp.dll",
    "Assembly-CSharp-firstpass.dll",
    "StrikeCore.dll",
    "CHNet.Core.dll",
    "CHNet.Server.dll",
)

READER_SCRIPT = os.path.join(_HERE, "mono_reader.ps1")
# The time cap the task brief set for one reader run (5 minutes).
READER_CAP_S = 300.0


def managed_folder(game_dir: str) -> str:
    """The one `<name>_Data\\Managed` folder of a Unity Mono build."""
    found = [p for p in glob.glob(os.path.join(game_dir, "*_Data", "Managed")) if os.path.isdir(p)]
    if len(found) != 1:
        raise FactsError(f"{game_dir}: expected one *_Data\\Managed folder, found {len(found)}")
    return found[0]


def assembly_paths(managed_dir: str) -> list[str]:
    paths = [os.path.join(managed_dir, name) for name in GAME_ASSEMBLIES]
    missing = [os.path.basename(p) for p in paths if not os.path.isfile(p)]
    if missing:
        raise FactsError(f"{managed_dir}: missing {', '.join(missing)}")
    return paths


def run_reader(paths: list[str], runner=subprocess.run) -> dict:
    """Run mono_reader.ps1 over `paths` and return the JSON it wrote."""
    pwsh = shutil.which("pwsh")
    if pwsh is None:
        raise FactsError("pwsh (PowerShell 7) is not on PATH; mono_reader.ps1 needs it")
    with tempfile.TemporaryDirectory(prefix="ch_mcp_mono_") as tmp:
        out = os.path.join(tmp, "reader.json")
        cmd = [pwsh, "-NoProfile", "-NonInteractive", "-File", READER_SCRIPT, "-Out", out, *paths]
        try:
            proc = runner(cmd, capture_output=True, text=True, timeout=READER_CAP_S)
        except subprocess.TimeoutExpired:
            raise FactsError(f"mono_reader.ps1 did not finish in {READER_CAP_S:.0f} s") from None
        if proc.returncode != 0 or not os.path.isfile(out):
            raise FactsError(
                f"mono_reader.ps1 failed (exit {proc.returncode}): {(proc.stderr or proc.stdout).strip()}"
            )
        with open(out, encoding="utf-8") as f:
            return json.load(f)


def class_kind(raw: dict) -> str:
    """class, struct, interface or enum, from the metadata flags and base type."""
    if raw["is_interface"]:
        return "interface"
    if raw["base_full"] == "System.Enum":
        return "enum"
    if raw["base_full"] == "System.ValueType":
        return "struct"
    return "class"


def to_class(raw: dict) -> dict:
    """One reader type in the facts.json class shape.

    dump.cs prints no base for a struct, an enum, an interface or a class that
    derives straight from System.Object, so neither does this.
    """
    kind = class_kind(raw)
    base = raw["base"] if kind == "class" and raw["base_full"] != "System.Object" else None
    return {
        "name": raw["name"],
        "namespace": raw["namespace"],
        "kind": kind,
        "typedef_index": raw["row"],
        "readable": dump_facts.is_readable(raw["name"]),
        "base": base,
        "interfaces": list(raw["interfaces"]),
        "fields": [
            {"name": f["name"], "type": f["type"], "offset": None, "static": f["static"]}
            for f in raw["fields"]
            if not f["literal"]
        ],
        "methods": [
            {
                "name": m["name"],
                "return": m["return"],
                "params": list(m["params"]),
                "static": m["static"],
                "rva": None,
            }
            for m in raw["methods"]
        ],
    }


def build_facts(reader_output: dict, game_version: str) -> dict:
    assemblies = reader_output["assemblies"]
    classes = [to_class(t) for a in assemblies for t in a["types"]]
    signatures = {
        name: dump_facts.signature(dump_facts.find_by_name(classes, name))
        for name in dump_facts.NAMED_SIGNATURES
    }
    return {
        "game_version": game_version,
        "dump_file": ", ".join(a["file"] for a in assemblies),
        "classes": classes,
        "signatures": signatures,
    }


def write_facts(game_dir: str, game_version: str, out_dir: str, runner=subprocess.run) -> tuple[str, dict]:
    paths = assembly_paths(managed_folder(game_dir))
    facts = build_facts(run_reader(paths, runner), game_version)
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, "facts.json")
    with open(out_path, "w", encoding="utf-8", newline="\n") as f:
        json.dump(facts, f, ensure_ascii=False, separators=(",", ":"))
    return out_path, facts


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Write facts.json for a Mono build of Clone Hero.")
    parser.add_argument("--game", required=True, help="the game folder, which holds <name>_Data\\Managed")
    parser.add_argument("--version", required=True, help="the game's version string, stored as given")
    parser.add_argument("--out", required=True, help="folder to write facts.json into")
    args = parser.parse_args(argv)
    sys.stdout.reconfigure(errors="backslashreplace")
    try:
        out_path, facts = write_facts(args.game, args.version, args.out)
    except (FactsError, OSError) as exc:
        print(f"mono_facts: {exc}", file=sys.stderr)
        return 1
    gm = facts["signatures"]["GameManager"]
    print(f"classes: {len(facts['classes'])}")
    print(f"GameManager: {gm['field_count']} fields, {len(gm['readable_fields'])} readable")
    print(f"wrote {out_path} ({os.path.getsize(out_path)} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
