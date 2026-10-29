"""Clone Hero's files on this machine: where the game and its user files are,
how to back the user files up before a live step and put them back after it,
and how to read a song folder's song.ini.

This module is the one home of those paths and of the list of files a live
step backs up (the plan's "Safety rules for every live step").

backup() and restore() take their folders as arguments, so the tests run on
temp folders and never touch the real files.
"""

from __future__ import annotations

import json
import os
import shutil
import time
from dataclasses import dataclass
from typing import Callable, Optional

from tools.ch_probe.constants import PROCESS_NAME

# The game this machine has (the plan's "What exists today"): its folder here,
# its file name from ch_probe.constants.PROCESS_NAME.
GAME_DIR = r"C:\Clone Hero\1.1 june\Clone Hero"
GAME_EXE = os.path.join(GAME_DIR, PROCESS_NAME)

_HOME = os.path.expanduser("~")
# Where the game keeps its settings and the stats of the last run.
DOCUMENTS_DIR = os.path.join(_HOME, "Documents", "Clone Hero")
# Where the game keeps its score store and user file.
LOCALLOW_DIR = os.path.join(_HOME, "AppData", "LocalLow", "srylain Inc_", "Clone Hero")

SCORESTATS = "scorestats.json"
SETTINGS = "settings.ini"
# The five files a live step backs up, by the folder they live in.
DOCUMENTS_FILES = (SETTINGS, SCORESTATS)
LOCALLOW_FILES = ("scoredata.bin", "scoresext.bin", "user.bin")

BACKUP_PREFIX = "backup-"
BACKUP_MANIFEST = "backup.json"
# The name a file that did not exist before the run gets when restore() moves
# the game's new copy aside instead of deleting it.
AFTER_RUN_SUFFIX = ".after-run"

SONG_INI = "song.ini"


class RestoreError(Exception):
    """A file could not be put back. Carries the file's name and the error."""

    def __init__(self, name: str, error: object):
        super().__init__(f"restore failed for {name}: {error}")
        self.name = name
        self.error = error


@dataclass(frozen=True)
class _Entry:
    name: str
    source: str
    existed: bool


def _stamp() -> str:
    return time.strftime("%Y%m%d-%H%M%S")


def _same_bytes(a: str, b: str) -> bool:
    with open(a, "rb") as fa, open(b, "rb") as fb:
        return fa.read() == fb.read()


def backup(documents_dir: str, locallow_dir: str, backup_root: str, *,
           stamp: Callable[[], str] = _stamp) -> str:
    """Copy the five user files into a new dated folder under `backup_root`
    and return that folder. A file that does not exist is recorded as absent,
    so restore() can put that state back too. Any copy error propagates, so a
    failed backup stops the step before the game starts."""
    folder = os.path.join(backup_root, BACKUP_PREFIX + stamp())
    os.makedirs(folder, exist_ok=False)
    entries = []
    for src_dir, names in ((documents_dir, DOCUMENTS_FILES),
                           (locallow_dir, LOCALLOW_FILES)):
        for name in names:
            source = os.path.abspath(os.path.join(src_dir, name))
            existed = os.path.isfile(source)
            if existed:
                shutil.copy2(source, os.path.join(folder, name))
            entries.append({"name": name, "source": source, "existed": existed})
    with open(os.path.join(folder, BACKUP_MANIFEST), "w", encoding="utf-8") as f:
        json.dump({"files": entries}, f, indent=1)
    return folder


def _read_manifest(backup_dir: str) -> list[_Entry]:
    with open(os.path.join(backup_dir, BACKUP_MANIFEST), encoding="utf-8") as f:
        data = json.load(f)
    return [_Entry(e["name"], e["source"], bool(e["existed"])) for e in data["files"]]


def _restore_one(backup_dir: str, e: _Entry) -> str:
    saved = os.path.join(backup_dir, e.name)
    if e.existed:
        if os.path.isfile(e.source) and _same_bytes(saved, e.source):
            return f"{e.name}: unchanged"
        shutil.copy2(saved, e.source)
        if not _same_bytes(saved, e.source):
            raise OSError(f"{e.source} differs from the backup after the copy")
        return f"{e.name}: restored"
    if not os.path.exists(e.source):
        return f"{e.name}: unchanged (absent before and after the run)"
    aside = os.path.join(backup_dir, e.name + AFTER_RUN_SUFFIX)
    shutil.move(e.source, aside)
    return f"{e.name}: absent before the run; the game's new copy moved to {aside}"


def restore(backup_dir: str, *, out: Callable[[str], None] = print) -> list[str]:
    """Put every file named in `backup_dir`'s manifest back where it came
    from, byte for byte, printing one line per file. Stops at the first file
    that fails, raising RestoreError with its name; the files after it are
    left as they are."""
    lines = []
    for e in _read_manifest(backup_dir):
        try:
            line = _restore_one(backup_dir, e)
        except OSError as err:
            raise RestoreError(e.name, err) from err
        out(line)
        lines.append(line)
    return lines


def read_song_ini(folder: str) -> dict[str, str]:
    """The keys of a song folder's song.ini `[song]` section, lower-cased.
    Keys before any section header count too. Empty when there is no
    song.ini."""
    path = os.path.join(folder, SONG_INI)
    if not os.path.isfile(path):
        return {}
    keys: dict[str, str] = {}
    section: Optional[str] = None
    with open(path, encoding="utf-8-sig", errors="replace") as f:
        for raw in f:
            line = raw.strip()
            if not line or line[0] in ";#":
                continue
            if line.startswith("[") and line.endswith("]"):
                section = line[1:-1].strip().lower()
                continue
            if section not in (None, "song") or "=" not in line:
                continue
            key, value = line.split("=", 1)
            keys.setdefault(key.strip().lower(), value.strip())
    return keys


def song_ini_length_ms(folder: str) -> Optional[int]:
    """song.ini's `song_length` in ms, or None when it is missing, not a
    whole number, or not above zero."""
    value = read_song_ini(folder).get("song_length")
    try:
        ms = int(value) if value is not None else 0
    except ValueError:
        return None
    return ms if ms > 0 else None


def song_ini_pro_drums(folder: str) -> bool:
    """Whether song.ini says `pro_drums = True`."""
    return read_song_ini(folder).get("pro_drums", "").lower() in ("true", "1")
