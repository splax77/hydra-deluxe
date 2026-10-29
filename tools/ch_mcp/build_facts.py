"""Which Clone Hero build is installed, where its dumped facts live, and how
to name an engine method from its address in those facts.

This module is the one home of those three questions (the plan
2026-10-28-ch-mcp-server.md, task T2 and question 4). Every Frida tool asks
it before it attaches, and refuses on its answer: a build with no facts file,
or one whose facts describe another version, is unknown, and the first
version of the server only stops there.

The facts file is the one dump_facts.py writes (see this folder's README.md
for its layout). It is large, so a caller loads it once per build and keeps
the dict; nothing here caches it.
"""

from __future__ import annotations

import json
import os
import re
from typing import Iterable

from tools.ch_mcp import game_files

# Where dump_build.py and dump_facts.py put each build's dump, one folder per
# build number (the README's folder).
DUMPS_ROOT = r"C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps"
FACTS_FILE = "facts.json"

# The game's own version file, relative to the game folder.
_VERSION_REL = os.path.join("Clone Hero_Data", "version.json")
VERSION_FILE = os.path.join(game_files.GAME_DIR, _VERSION_REL)

# The build number is the fourth dotted number of a version string.
_BUILD_NUMBER = re.compile(r"^v?\d+\.\d+\.\d+\.(\d+)")


class BuildFactsError(Exception):
    """Base of the two refusals below, so a caller can catch either."""


class UnknownBuild(BuildFactsError):
    """No facts describe the installed build (question 4's refusal)."""

    def __init__(self, version: str, detail: str = ""):
        message = (f"unknown build {version}: no facts for it; "
                   f"run dump_build.py and dump_facts.py")
        if detail:
            message += f" ({detail})"
        super().__init__(message)
        self.version = version


class FactsLookupError(BuildFactsError):
    """The facts do not name exactly one class or method for a question."""


def version_file(game_dir: str) -> str:
    """The version file of the game in `game_dir`."""
    return os.path.join(game_dir, _VERSION_REL)


def current_build(game_dir: str = game_files.GAME_DIR) -> str:
    """The `version` the game in `game_dir` says it is, as written."""
    with open(version_file(game_dir), encoding="utf-8") as f:
        return json.load(f)["version"]


def build_number(version: str) -> str:
    """The build number of a version string, which names its dump folder.
    A string with no such number is an unknown build."""
    m = _BUILD_NUMBER.match(version)
    if not m:
        raise UnknownBuild(version, "no build number in the version string")
    return m.group(1)


def _without_v(version: str) -> str:
    return version.removeprefix("v")


def same_build(a: str, b: str) -> bool:
    """Whether two version strings name one build. version.json writes a
    leading `v` and dump_facts.py's facts do not, so one is dropped from each."""
    return _without_v(a) == _without_v(b)


def facts_path(version: str, dumps_root: str = DUMPS_ROOT) -> str:
    """Where the facts of the build `version` names would be."""
    return os.path.join(dumps_root, build_number(version), FACTS_FILE)


def load_facts(game_dir: str = game_files.GAME_DIR,
               dumps_root: str = DUMPS_ROOT) -> dict:
    """The facts of the installed build. Raises UnknownBuild when its facts
    file is missing or describes another version."""
    version = current_build(game_dir)
    path = facts_path(version, dumps_root)
    if not os.path.isfile(path):
        raise UnknownBuild(version, f"{path} does not exist")
    with open(path, encoding="utf-8") as f:
        facts = json.load(f)
    described = facts.get("game_version")
    if not isinstance(described, str) or not same_build(described, version):
        raise UnknownBuild(version, f"{path} describes {described}")
    return facts


def engine_class(facts: dict) -> tuple[str, int]:
    """The drum engine's scrambled class name and its constructor's RVA."""
    engine = facts["signatures"]["drums_engine"]
    return engine["name"], engine["ctor_rva"]


def _one_class(facts: dict, class_name: str) -> dict:
    found = [c for c in facts["classes"] if c.get("name") == class_name]
    if len(found) != 1:
        raise FactsLookupError(f"class {class_name} matched {len(found)} classes")
    return found[0]


def method_name_by_rva(facts: dict, class_name: str, rva: int) -> str:
    """The dump's name for the method of `class_name` at `rva`. The class and
    the method must each be the only match. A method the dump gives no
    address (a generic base class's) can never be named this way."""
    cls = _one_class(facts, class_name)
    found = [m for m in cls.get("methods", []) if m.get("rva") == rva]
    if len(found) != 1:
        raise FactsLookupError(
            f"rva 0x{rva:X} matched {len(found)} methods of {class_name}")
    return found[0]["name"]


def method_names_by_rva(facts: dict, class_name: str,
                        rvas: Iterable[int]) -> list[str]:
    """method_name_by_rva for each address in `rvas`, in order."""
    return [method_name_by_rva(facts, class_name, rva) for rva in rvas]
