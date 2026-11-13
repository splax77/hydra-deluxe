"""Clone Hero's files on this machine: where the game and its user files are,
how to back up the registry key a live step's tools change and put it back
after the step, and how to read a song folder's song.ini and change its
title.

This module is the one home of those paths, of the registry key the game's
input library saves its bindings under, and of the settings.ini line a run
changes (set_game_setting). No file in the game's folders is backed up,
restored, moved or deleted: the user's rulings of 2026-11-13 in
docs/agents/rulings/clone-hero.md. The server plan is
`docs/superpowers/plans/2026-10-28-ch-mcp-server.md` (removed; `git show 444631e6:docs/superpowers/plans/2026-10-28-ch-mcp-server.md`).

backup(), restore_all() and restore_registry() take their folders and their
reg.exe runner as arguments, so the tests run on temp folders and a fake
runner and never touch the real key.
"""

from __future__ import annotations

import json
import os
import re
import subprocess
import time
from typing import Callable, Iterator, Optional, Sequence

from tools.ch_probe.constants import PROCESS_NAME

# The game this machine has (the server plan's "What exists today"): its
# folder here, its file name from ch_probe.constants.PROCESS_NAME.
GAME_DIR = r"C:\Clone Hero\1.1 june\Clone Hero"
GAME_EXE = os.path.join(GAME_DIR, PROCESS_NAME)

_HOME = os.path.expanduser("~")
# Where the game keeps its settings and the stats of the last run.
DOCUMENTS_DIR = os.path.join(_HOME, "Documents", "Clone Hero")
# Where the game keeps its score store and user file.
LOCALLOW_DIR = os.path.join(_HOME, "AppData", "LocalLow", "srylain Inc_", "Clone Hero")

SCORESTATS = "scorestats.json"
SETTINGS = "settings.ini"
# What a restore line says for a file an older backup's manifest names: the
# game's copy is not touched.
LEFT_AS_SAVED = "left as the game saved it"

BACKUP_PREFIX = "backup-"
BACKUP_MANIFEST = "backup.json"
# The name of the key's export restore_registry takes before it puts the
# saved one back, so the game's version is kept.
AFTER_RUN_SUFFIX = ".after-run"

SONG_INI = "song.ini"
# The song.ini section read_song_ini reads, and its key for the listed title.
SONG_SECTION = "song"
SONG_TITLE_KEY = "name"

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
    """The registry key could not be put back. Carries the name of its
    backup entry and the error."""

    def __init__(self, name: str, error: object):
        super().__init__(f"restore failed for {name}: {error}")
        self.name = name
        self.error = error


def _stamp() -> str:
    return time.strftime("%Y%m%d-%H%M%S")


def _same_bytes(a: str, b: str) -> bool:
    with open(a, "rb") as fa, open(b, "rb") as fb:
        return fa.read() == fb.read()


def _export_registry(key: str, path: str, run_reg: RegRunner) -> None:
    run_reg(["export", key, path, "/y"])
    if not os.path.isfile(path):
        raise OSError(f"reg.exe export of {key} wrote no file at {path}")


def backup(backup_root: str, *,
           stamp: Callable[[], str] = _stamp,
           run_reg: Optional[RegRunner] = None) -> str:
    """Make a new dated folder under `backup_root` and return it. With a
    `run_reg`, REGISTRY_KEY is exported into it as REGISTRY_BACKUP and
    recorded in the manifest for restore_registry(); with none the manifest
    is empty. No file is copied. Any export error propagates, so a failed
    backup stops the step before the game starts."""
    folder = os.path.join(backup_root, BACKUP_PREFIX + stamp())
    os.makedirs(folder, exist_ok=False)
    entries = []
    if run_reg is not None:
        _export_registry(REGISTRY_KEY, os.path.join(folder, REGISTRY_BACKUP), run_reg)
        entries.append({"name": REGISTRY_BACKUP, "key": REGISTRY_KEY})
    with open(os.path.join(folder, BACKUP_MANIFEST), "w", encoding="utf-8") as f:
        json.dump({"files": entries}, f, indent=1)
    return folder


def _manifest_entries(backup_dir: str) -> list[dict]:
    with open(os.path.join(backup_dir, BACKUP_MANIFEST), encoding="utf-8") as f:
        return json.load(f)["files"]


def left_as_saved(backup_dir: str, *, out: Callable[[str], None] = print) -> list[str]:
    """One LEFT_AS_SAVED line for each file an older backup's manifest
    names. Touches no file: backups taken before the user's rulings of
    2026-11-13 hold copies of the game's files, and those copies are never
    put back."""
    lines = []
    for e in _manifest_entries(backup_dir):
        if "key" in e:
            continue
        line = f"{e['name']}: {LEFT_AS_SAVED}"
        out(line)
        lines.append(line)
    return lines


def restore_all(backup_dir: str, *, run_reg: Optional[RegRunner],
                out: Callable[[str], None] = print) -> list[str]:
    """left_as_saved(), then restore_registry(), for a backup that may hold
    the key. With the key in the manifest and no `run_reg` this raises
    RestoreError, so the key is never left out without a word. bot_run's
    restore is its only caller."""
    lines = left_as_saved(backup_dir, out=out)
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


def get_game_setting(path: str, key: str) -> str:
    """The value of the one `key = value` line in the [GAME_SECTION] section
    of the settings.ini at `path`, without the spacing around it. Raises
    SettingNotFound when the section has no such line, or more than one, and
    OSError when the file cannot be read."""
    with open(path, "rb") as f:
        lines = f.read().splitlines(keepends=True)
    m = _SETTING_LINE.match(lines[_game_setting_line(lines, key)])
    return m.group(2).decode("utf-8", "replace")


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


def _song_ini_lines(path: str) -> list[bytes]:
    with open(path, "rb") as f:
        return f.read().splitlines(keepends=True)


def _song_section_lines(lines: list[bytes]) -> Iterator[tuple[int, Optional[str], str]]:
    """The lines of a song.ini, split into lines, that belong to its song
    section, as (line index, key, value). The key comes lower-cased and is
    None for a `[song]` header. Keys before any section header count too. This
    is the one rule for which lines read_song_ini and set_song_title see."""
    section: Optional[str] = None
    for i, raw in enumerate(lines):
        line = raw.decode("utf-8-sig" if i == 0 else "utf-8", "replace").strip()
        if not line or line[0] in ";#":
            continue
        if line.startswith("[") and line.endswith("]"):
            section = line[1:-1].strip().lower()
            if section == SONG_SECTION:
                yield i, None, ""
            continue
        if section not in (None, SONG_SECTION) or "=" not in line:
            continue
        key, value = line.split("=", 1)
        yield i, key.strip().lower(), value.strip()


def read_song_ini(folder: str) -> dict[str, str]:
    """The keys of a song folder's song.ini song section, lower-cased; the
    first of two lines with one key wins. Empty when there is no song.ini."""
    path = os.path.join(folder, SONG_INI)
    if not os.path.isfile(path):
        return {}
    keys: dict[str, str] = {}
    for _i, key, value in _song_section_lines(_song_ini_lines(path)):
        if key is not None:
            keys.setdefault(key, value)
    return keys


def _add_song_title(lines: list[bytes], after: int, title: bytes) -> None:
    """Insert a title line after line `after`, ending it the way the file's
    first line ends."""
    found = re.search(rb"\r\n|\r|\n", b"".join(lines))
    ending = found.group() if found else b"\n"
    line = SONG_TITLE_KEY.encode() + b" = " + title
    ends_a_line = lines[after].endswith((b"\r", b"\n"))
    lines.insert(after + 1, line + ending if ends_a_line else ending + line)


def set_song_title(folder: str, title_for: Callable[[str], str]) -> None:
    """Replace the title in a song folder's song.ini with `title_for` of it.
    A title line that holds no title, or a file with no title line, gives
    `title_for` an empty string. Every title line in the song section changes,
    because the readers disagree on which of two wins: read_song_ini takes the
    first and Hydra's read_song_ini_keys the last. A file with no title line
    gets one, under its `[song]` header when it has one. Every other byte
    stays. Raises SettingNotFound when the file has no song section at all,
    and OSError when it cannot be read or written."""
    path = os.path.join(folder, SONG_INI)
    lines = _song_ini_lines(path)
    entries = list(_song_section_lines(lines))
    if not entries:
        raise SettingNotFound(f"no [{SONG_SECTION}] section in {path}")
    titles = [i for i, key, _value in entries if key == SONG_TITLE_KEY]
    for i in titles:
        m = _SETTING_LINE.match(lines[i])
        # surrogateescape carries bytes that are not UTF-8 through unchanged.
        title = title_for(m.group(2).decode("utf-8", "surrogateescape"))
        lines[i] = m.group(1) + title.encode("utf-8", "surrogateescape") + m.group(3)
    if not titles:
        headers = [i for i, key, _value in entries if key is None]
        _add_song_title(lines, headers[0] if headers else entries[0][0],
                        title_for("").encode("utf-8", "surrogateescape"))
    with open(path, "wb") as f:
        f.write(b"".join(lines))


def song_ini_pro_drums(folder: str) -> bool:
    """Whether song.ini says `pro_drums = True`."""
    return read_song_ini(folder).get("pro_drums", "").lower() in ("true", "1")
