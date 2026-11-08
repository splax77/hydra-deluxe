"""The build stamp of a compiled Frida agent: written once by the agent's
build script, read by every loader before it loads the agent.

Why: a built agent (`tools/ch_mcp/agent/<name>.js`) is untracked and built
by hand, so a run could load a `.js` older than the `.ts` it came from and
nothing would say so (D318 item 1, "Refuse stale"). The build scripts in
`agent/package.json` run this file as a script after `frida-compile`, and
it writes the stamp (`stamp_path`). A loader calls `check` and refuses the
agent when it returns a line.

Which files the stamp covers is `sources_of`. It reads the list from the
bundle's own source map, so the list is the compiler's, not a second copy
of the import rules. A new import changes the text of the file that holds
it, so that file's hash no longer matches and the stamp reads stale until
the next build lists the new file too. Packages are left out, but the
files that pin them and set the compile (`BUILD_INPUTS`) are covered, so
a changed dependency or compiler setting reads stale too.

Only the standard library is imported here, so the script runs on its own.
"""

from __future__ import annotations

import hashlib
import json
import os
import re
import sys
from typing import Any, Dict, Iterable, List, Optional

# The stamp's version: a reader refuses a stamp of any other.
STAMP_FORMAT = 1
AGENT_FOLDER_TEXT = "tools/ch_mcp/agent"
PACKAGE_JSON = "package.json"
# frida-compile's bundle: a header of asset sizes and names, then the assets.
BUNDLE_MAGIC = "📦\n"
BUNDLE_SEPARATOR = "\n✄\n"
NODE_MODULES = "node_modules"
# Files in the agent folder that decide what the compile produces without
# being imported: the package versions and the compiler settings.
BUILD_INPUTS = ("package.json", "package-lock.json", "tsconfig.json")
# What builds_record writes when the schedule holds no build stamp.
NO_REPLAY_BUILD = ("not in the schedule: it has no hydra_replay_build field, so the "
                   "hydra_replay that wrote it is unknown")


class StampError(Exception):
    """A bundle or stamp this module cannot read; the message says which
    file and what is wrong."""


def stamp_path(js_path: str) -> str:
    """The stamp file beside a built agent: `kit.js` -> `kit.stamp.json`."""
    root, _ = os.path.splitext(js_path)
    return root + ".stamp.json"


def build_command(js_path: str) -> str:
    """The npm command that builds `js_path`: the script in the package.json
    beside it whose `frida-compile -o` names that file."""
    folder = os.path.dirname(os.path.abspath(js_path))
    name = os.path.basename(js_path)
    package = os.path.join(folder, PACKAGE_JSON)
    try:
        with open(package, encoding="utf-8") as f:
            scripts = json.load(f).get("scripts", {})
    except (OSError, ValueError) as e:
        raise StampError(f"{package}: cannot read the build scripts ({e})") from e
    for script, text in scripts.items():
        out = re.search(r"-o\s+(\S+)", text)
        if out is not None and out.group(1) == name:
            return f"npm run {script}"
    raise StampError(f"{package}: no build script writes {name}")


def _sha256_file(path: str) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 16), b""):
            h.update(block)
    return h.hexdigest()


def bundle_assets(js_path: str) -> Dict[str, bytes]:
    """The assets of a frida-compile bundle by name (`/kit.js`,
    `/kit.js.map`), read by the sizes its header gives."""
    with open(js_path, "rb") as f:
        data = f.read()
    magic = BUNDLE_MAGIC.encode("utf-8")
    sep = BUNDLE_SEPARATOR.encode("utf-8")
    if not data.startswith(magic):
        raise StampError(f"{js_path}: not a frida-compile bundle (no bundle header)")
    header_end = data.find(sep[1:], len(magic))
    if header_end < 0:
        raise StampError(f"{js_path}: the bundle header never ends")
    entries = []
    for line in data[len(magic):header_end].decode("utf-8").splitlines():
        size, _, name = line.partition(" ")
        if not size.isdigit() or not name:
            raise StampError(f"{js_path}: bad bundle header line {line!r}")
        entries.append((name, int(size)))
    at = header_end + len(sep) - 1
    assets: Dict[str, bytes] = {}
    for i, (name, size) in enumerate(entries):
        if i > 0:
            if data[at:at + len(sep)] != sep:
                raise StampError(f"{js_path}: no separator before bundle asset {name}")
            at += len(sep)
        assets[name] = data[at:at + size]
        if len(assets[name]) != size:
            raise StampError(f"{js_path}: bundle asset {name} is cut short")
        at += size
    return assets


def bundle_module(js_path: str) -> str:
    """The script asset of a built agent: the code Frida runs."""
    name = "/" + os.path.basename(js_path)
    assets = bundle_assets(js_path)
    if name not in assets:
        raise StampError(f"{js_path}: the bundle holds no {name}")
    return assets[name].decode("utf-8")


def sources_of(js_path: str) -> List[str]:
    """The local sources the bundle's source map names, as written there
    (relative to the agent folder), node_modules left out."""
    name = "/" + os.path.basename(js_path) + ".map"
    assets = bundle_assets(js_path)
    if name not in assets:
        raise StampError(f"{js_path}: the bundle holds no source map {name}, so the "
                         "files the compile read are unknown")
    try:
        source_map = json.loads(assets[name].decode("utf-8"))
    except ValueError as e:
        raise StampError(f"{js_path}: its source map is not JSON ({e})") from e
    if source_map.get("sourceRoot"):
        raise StampError(f"{js_path}: its source map has a sourceRoot "
                         f"({source_map['sourceRoot']!r}), which this reader does not resolve")
    sources = [s for s in source_map.get("sources", [])
               if NODE_MODULES not in s.replace("\\", "/").split("/")]
    if not sources:
        raise StampError(f"{js_path}: its source map names no local source")
    return sources


def make_stamp(js_path: str) -> Dict[str, Any]:
    """The stamp for a just-built agent: the hash of the `.js`, of each
    local source its source map names and of each BUILD_INPUTS file."""
    folder = os.path.dirname(os.path.abspath(js_path))
    hashes = {}
    for name in BUILD_INPUTS:
        if os.path.isfile(os.path.join(folder, name)):
            hashes[name] = _sha256_file(os.path.join(folder, name))
    for source in sources_of(js_path):
        path = os.path.normpath(os.path.join(folder, source))
        if not os.path.isfile(path):
            raise StampError(f"{js_path}: its source map names {source}, which is not there")
        hashes[source] = _sha256_file(path)
    return {"format": STAMP_FORMAT, "js": os.path.basename(js_path),
            "js_sha256": _sha256_file(js_path), "sources": hashes}


def write_stamp(js_path: str) -> str:
    """Write the stamp beside `js_path` and return its path."""
    stamp = make_stamp(js_path)
    path = stamp_path(js_path)
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        json.dump(stamp, f, indent=2, sort_keys=True)
        f.write("\n")
    return path


def read_stamp(js_path: str) -> Dict[str, Any]:
    """The stamp beside `js_path`, as written."""
    path = stamp_path(js_path)
    try:
        with open(path, encoding="utf-8") as f:
            stamp = json.load(f)
    except FileNotFoundError as e:
        raise StampError(f"{js_path} has no build stamp ({os.path.basename(path)})") from e
    except (OSError, ValueError) as e:
        raise StampError(f"{path}: cannot read the build stamp ({e})") from e
    if not isinstance(stamp, dict) or stamp.get("format") != STAMP_FORMAT \
            or not isinstance(stamp.get("sources"), dict):
        raise StampError(f"{path}: not a format {STAMP_FORMAT} build stamp")
    return stamp


def _stale_reason(js_path: str) -> Optional[str]:
    """Why `js_path` may not load, or None when its stamp matches."""
    if not os.path.isfile(js_path):
        return f"{js_path} not found"
    try:
        stamp = read_stamp(js_path)
    except StampError as e:
        return str(e)
    if stamp.get("js_sha256") != _sha256_file(js_path):
        return f"{js_path} is not the file its build stamp was written for"
    folder = os.path.dirname(os.path.abspath(js_path))
    for source, digest in sorted(stamp["sources"].items()):
        path = os.path.normpath(os.path.join(folder, source))
        if not os.path.isfile(path):
            return f"{js_path} is stale: {source}, which it was built from, is gone"
        if _sha256_file(path) != digest:
            return f"{js_path} is stale: {source} changed since it was built"
    return None


def check(js_path: str) -> Optional[str]:
    """None when the built agent matches its sources; otherwise the refusal
    line, naming the file and the command that rebuilds it."""
    reason = _stale_reason(js_path)
    if reason is None:
        return None
    try:
        command = build_command(js_path)
    except StampError as e:
        return f"{reason}; and its build command is unknown: {e}"
    return f"{reason}; run {command} in {AGENT_FOLDER_TEXT} first"


def stamp_digest(stamp: Dict[str, Any]) -> str:
    """One hash for a whole stamp, for the run's record."""
    text = json.dumps(stamp, sort_keys=True, separators=(",", ":"))
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def builds_record(agent_paths: Iterable[str], schedule: Dict[str, Any]) -> Dict[str, Any]:
    """The run's record of what it ran: one entry per loaded agent (its
    stamp and that stamp's digest, or why it has none) and the schedule's
    `hydra_replay_build`, or NO_REPLAY_BUILD when the schedule has none."""
    agents = []
    for path in agent_paths:
        entry: Dict[str, Any] = {"file": os.path.basename(path)}
        try:
            stamp = read_stamp(path)
        except StampError as e:
            entry["stamp"] = None
            entry["why_no_stamp"] = str(e)
        else:
            entry["stamp"] = stamp
            entry["digest"] = stamp_digest(stamp)
        stale = _stale_reason(path)
        entry["fresh"] = stale is None
        if stale is not None:
            entry["stale"] = stale
        agents.append(entry)
    build = schedule.get("hydra_replay_build")
    if not isinstance(build, str) or not build:
        build = NO_REPLAY_BUILD
    return {"agents": agents, "hydra_replay_build": build}


def main(argv: List[str]) -> int:
    if len(argv) != 1:
        print("usage: py -I ../build_stamp.py <built agent .js>", file=sys.stderr)
        return 2
    try:
        path = write_stamp(argv[0])
    except (StampError, OSError) as e:
        print(f"build stamp not written: {e}", file=sys.stderr)
        return 1
    print(f"build stamp: {path}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
