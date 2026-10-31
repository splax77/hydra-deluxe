"""Clone Hero's files on this machine: where the game and its user files are,
how to back the user files up before a live step and put them back after it,
and how to read a song folder's song.ini.

This module is the one home of those paths, of the list of files a live
step backs up (the plan's "Safety rules for every live step"), of the
registry key the game's input library saves its bindings under, and of the
settings.ini line a run changes (set_game_setting).

backup(), restore() and restore_registry() take their folders and their
reg.exe runner as arguments, so the tests run on temp folders and a fake
runner and never touch the real files or the real key.
"""

from __future__ import annotations

import json
import os
import re
import shutil
import subprocess
import time
from dataclasses import dataclass
from typing import Callable, Optional, Sequence

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

# Where the game's input library (Rewired) saves its bindings. The key has a
# dot in "Inc.", unlike the LocalLow folder above.
REGISTRY_KEY = r"HKCU\Software\srylain Inc.\Clone Hero"
# The key's export in the backup folder, and the name its manifest entry and
# any RestoreError carry.
REGISTRY_BACKUP = "rewired.reg"
# The export restore_registry takes after the import, to check it.
REGISTRY_CHECK_SUFFIX = ".check"
# How the restore lines name the key.
REGISTRY_LINE_NAME = "rewired registry"

# The settings.ini section set_game_setting edits, and the one line a run
# changes there: the game must keep running while it is not in front.
GAME_SECTION = "game"
PAUSE_ON_FOCUS_LOST = "pause_on_focus_lost"
PAUSE_ON_FOCUS_LOST_RUN_VALUE = "0"

# How long one reg.exe call may take, in seconds (the user's ruling of
# 2026-10-30: "Cap at 30 s"). A call that passes it fails like one that exits
# non-zero, so the backup stops the launch and the restore names the file.
REG_CAP_S = 30.0

# Runs reg.exe with the arguments after the program name; raises OSError
# when reg.exe fails.
RegRunner = Callable[[Sequence[str]], None]


def run_reg(args: Sequence[str]) -> None:
    """Run reg.exe with `args`, raising OSError with the command and what
    reg.exe said when it exits non-zero, and OSError when it has not
    finished after REG_CAP_S."""
    cmd = ["reg.exe", *args]
    try:
        done = subprocess.run(cmd, stdin=subprocess.DEVNULL, capture_output=True, text=True,
                              timeout=REG_CAP_S)
    except subprocess.TimeoutExpired as e:
        raise OSError(f"{subprocess.list2cmdline(cmd)} did not finish in {REG_CAP_S:.0f} s") from e
    if done.returncode != 0:
        said = (done.stderr or done.stdout or "").strip()
        raise OSError(f"{subprocess.list2cmdline(cmd)} exited {done.returncode}: {said}")


class SettingNotFound(Exception):
    """set_game_setting found no single line for the key in the section."""


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


def _export_registry(key: str, path: str, run_reg: RegRunner) -> None:
    run_reg(["export", key, path, "/y"])
    if not os.path.isfile(path):
        raise OSError(f"reg.exe export of {key} wrote no file at {path}")


def backup(documents_dir: str, locallow_dir: str, backup_root: str, *,
           stamp: Callable[[], str] = _stamp,
           run_reg: Optional[RegRunner] = None) -> str:
    """Copy the five user files into a new dated folder under `backup_root`
    and return that folder. A file that does not exist is recorded as absent,
    so restore() can put that state back too. With a `run_reg`, REGISTRY_KEY
    is exported beside them as REGISTRY_BACKUP and recorded in the manifest
    for restore_registry(); with none the key is left out. Any copy or export
    error propagates, so a failed backup stops the step before the game
    starts."""
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
    if run_reg is not None:
        _export_registry(REGISTRY_KEY, os.path.join(folder, REGISTRY_BACKUP), run_reg)
        entries.append({"name": REGISTRY_BACKUP, "key": REGISTRY_KEY})
    with open(os.path.join(folder, BACKUP_MANIFEST), "w", encoding="utf-8") as f:
        json.dump({"files": entries}, f, indent=1)
    return folder


def _manifest_entries(backup_dir: str) -> list[dict]:
    with open(os.path.join(backup_dir, BACKUP_MANIFEST), encoding="utf-8") as f:
        return json.load(f)["files"]


def _read_manifest(backup_dir: str) -> list[_Entry]:
    """The manifest's file entries; the registry entry is restore_registry's."""
    return [_Entry(e["name"], e["source"], bool(e["existed"]))
            for e in _manifest_entries(backup_dir) if "key" not in e]


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
    left as they are. The registry entry is not a file: restore_all() puts
    it back too, and bot_run's restore is its only caller."""
    lines = []
    for e in _read_manifest(backup_dir):
        try:
            line = _restore_one(backup_dir, e)
        except OSError as err:
            raise RestoreError(e.name, err) from err
        out(line)
        lines.append(line)
    return lines


def restore_all(backup_dir: str, *, run_reg: Optional[RegRunner],
                out: Callable[[str], None] = print) -> list[str]:
    """restore(), then restore_registry(), for a backup that may hold the
    key. With the key in the manifest and no `run_reg` this raises
    RestoreError, so the key is never left out without a word. Stops at the
    first failure."""
    lines = restore(backup_dir, out=out)
    return lines + restore_registry(backup_dir, run_reg=run_reg, out=out)


def _restore_key(backup_dir: str, key: str, run_reg: RegRunner) -> str:
    saved = os.path.join(backup_dir, REGISTRY_BACKUP)
    after_run = saved + AFTER_RUN_SUFFIX
    _export_registry(key, after_run, run_reg)
    if _same_bytes(saved, after_run):
        return f"{REGISTRY_LINE_NAME}: unchanged"
    run_reg(["delete", key, "/f"])
    run_reg(["import", saved])
    check = saved + REGISTRY_CHECK_SUFFIX
    _export_registry(key, check, run_reg)
    if not _same_bytes(saved, check):
        raise OSError(f"{key} still differs from {saved} after the import; the game's "
                      f"version is in {after_run}")
    return f"{REGISTRY_LINE_NAME}: restored"


def restore_registry(backup_dir: str, *, run_reg: Optional[RegRunner],
                     out: Callable[[str], None] = print) -> list[str]:
    """Put the registry key named in `backup_dir`'s manifest back, printing
    one line. The key is exported again and compared with the saved export;
    when they differ, the key is deleted, the saved export imported, and the
    key exported once more and compared. Raises RestoreError named
    REGISTRY_BACKUP when the texts still differ, when reg.exe fails, or when
    the manifest holds the key but no `run_reg` is given. A backup taken
    without the key prints nothing."""
    lines = []
    for e in _manifest_entries(backup_dir):
        if "key" not in e:
            continue
        try:
            if run_reg is None:
                raise OSError("the backup holds the registry key but no reg.exe runner was given")
            line = _restore_key(backup_dir, e["key"], run_reg)
        except OSError as err:
            raise RestoreError(e["name"], err) from err
        out(line)
        lines.append(line)
    return lines


def _game_setting_line(lines: list[bytes], key: str) -> int:
    """The index of the one `key = value` line in the [GAME_SECTION]
    section of a settings.ini split into lines. Raises SettingNotFound when
    there is none or more than one."""
    found = []
    section: Optional[str] = None
    want = key.strip().lower()
    for i, raw in enumerate(lines):
        line = raw.lstrip(b"\xef\xbb\xbf").strip()
        if line.startswith(b"[") and line.endswith(b"]"):
            section = line[1:-1].strip().lower().decode("utf-8", "replace")
            continue
        if section != GAME_SECTION or b"=" not in line:
            continue
        name = line.split(b"=", 1)[0].strip().lower().decode("utf-8", "replace")
        if name == want:
            found.append(i)
    if len(found) != 1:
        where = "no" if not found else f"{len(found)}"
        raise SettingNotFound(f"{where} `{key}` line in the [{GAME_SECTION}] section")
    return found[0]


# A settings line split into what comes before the value (the key, the "="
# and its spacing), the value, and the trailing spacing and line ending.
_SETTING_LINE = re.compile(rb"\A([^=]*=[ \t]*)(.*?)([ \t]*\r?\n?)\Z", re.S)


def set_game_setting(path: str, key: str, value: str) -> None:
    """Rewrite the value of the one `key = value` line in the [GAME_SECTION]
    section of the settings.ini at `path`. Every other byte of the file
    stays as it was, the line's own spacing and line ending included.
    Raises SettingNotFound when the section has no such line, or more than
    one, and OSError when the file cannot be read or written."""
    with open(path, "rb") as f:
        lines = f.read().splitlines(keepends=True)
    i = _game_setting_line(lines, key)
    m = _SETTING_LINE.match(lines[i])
    lines[i] = m.group(1) + value.encode("utf-8") + m.group(3)
    with open(path, "wb") as f:
        f.write(b"".join(lines))


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
