"""Launch Clone Hero with a bot on one song and keep the stats file it writes.

The game's command line plays a song folder with bot players. This script
backs up the user files first, starts the game, waits (with a cap) for
scorestats.json to change, copies the new file out, watches whether the game
exits, and then puts the user files back. It is the one owner of the plan's
"Safety rules for every live step" for the launch.

    py -I tools/ch_mcp/bot_run.py --song <song folder> --out <folder>
        [--player Drums,Expert] [--profile <name>] [--stop] [--backup-root <folder>]
    py -I tools/ch_mcp/bot_run.py --song <song folder> --out <folder> --no-wait
    py -I tools/ch_mcp/bot_run.py --restore <backup folder>

--no-wait backs up and starts the game, then returns at once and restores
nothing; run --restore on the printed folder after the step. The game is
never closed unless --stop is given. No --profile means the game plays as
its default player.

The file paths and the backup list live in game_files.py; the caps are the
constants below.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass, field
from typing import Callable, Optional, Protocol

if __package__ in (None, ""):
    # Run as a script (py -I leaves the repo off sys.path).
    sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..")))

from tools.ch_mcp import game_files  # noqa: E402
from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import probe_songs  # noqa: E402

DEFAULT_PLAYER = "Drums,Expert"
# The user's caps for the live steps (the plan's question 5).
POLL_INTERVAL_S = 1.0
POLL_CAP_EXTRA_S = 60.0
EXIT_CHECK_S = 30.0

MANIFEST = "manifest.json"
PRO_DRUMS_INSTRUMENT = "ProDrums"


class SongLengthMissing(Exception):
    pass


class Game(Protocol):
    pid: int

    def is_running(self) -> bool: ...

    def stop(self) -> None: ...


class _Process:
    """The real game process."""

    def __init__(self, popen: subprocess.Popen):
        self._p = popen
        self.pid = popen.pid

    def is_running(self) -> bool:
        return self._p.poll() is None

    def stop(self) -> None:
        self._p.terminate()
        self._p.wait(timeout=EXIT_CHECK_S)


def _real_launch(cmd: list[str]) -> Game:
    return _Process(subprocess.Popen(cmd, cwd=os.path.dirname(cmd[0])))


@dataclass
class Env:
    """Everything the run touches outside its own arguments. The tests swap
    each piece for a fake."""
    exe: str = game_files.GAME_EXE
    documents_dir: str = game_files.DOCUMENTS_DIR
    locallow_dir: str = game_files.LOCALLOW_DIR
    launch: Callable[[list[str]], Game] = _real_launch
    now: Callable[[], float] = time.monotonic
    sleep: Callable[[float], None] = time.sleep
    stamp: Callable[[], str] = field(default=lambda: time.strftime("%Y%m%d-%H%M%S"))
    out: Callable[[str], None] = print


def song_length_ms(song_dir: str) -> tuple[int, str]:
    """The song's length in ms and where it came from: song.ini's
    song_length, else the probe manifest's last note through
    probe_songs.song_length_ms."""
    ms = game_files.song_ini_length_ms(song_dir)
    if ms is not None:
        return ms, f"song_length in {os.path.join(song_dir, game_files.SONG_INI)}"
    path = os.path.join(song_dir, MANIFEST)
    if os.path.isfile(path):
        with open(path, encoding="utf-8") as f:
            notes = json.load(f).get("notes") or []
        if notes:
            return (probe_songs.song_length_ms(int(notes[-1]["time_ms"])),
                    f"last note in {path} plus probe_songs.SILENCE_MS")
    raise SongLengthMissing(
        f"{song_dir}: no song_length in song.ini and no notes in manifest.json; "
        "the stats wait has no cap, so nothing was started")


def launch_command(exe: str, song_dir: str, player: str,
                   profile: Optional[str] = None) -> list[str]:
    cmd = [exe, "--song", song_dir, "--player", player]
    if profile:
        cmd += ["--profile", profile]
    return cmd


def _file_state(path: str) -> Optional[tuple[int, int]]:
    try:
        st = os.stat(path)
    except FileNotFoundError:
        return None
    return st.st_size, st.st_mtime_ns


def _parses(path: str) -> bool:
    try:
        with open(path, encoding="utf-8-sig") as f:
            json.load(f)
    except (OSError, ValueError):
        return False
    return True


def _slug(text: str) -> str:
    return re.sub(r"[^A-Za-z0-9_-]+", "_", text).strip("_") or "song"


def _poll_stats(song_dir: str, out_dir: str, cap_s: float,
                before: Optional[tuple[int, int]], env: Env) -> Optional[str]:
    """Wait for scorestats.json to change and parse, then copy it into
    `out_dir`. Returns the copy, or None when the cap passes first."""
    path = os.path.join(env.documents_dir, game_files.SCORESTATS)
    start = env.now()
    changed = False
    while True:
        env.sleep(POLL_INTERVAL_S)
        elapsed = env.now() - start
        state = _file_state(path)
        if state is not None and state != before:
            changed = True
            if _parses(path):
                name = f"scorestats-{_slug(os.path.basename(os.path.normpath(song_dir)))}-{env.stamp()}.json"
                dest = os.path.join(out_dir, name)
                shutil.copy2(path, dest)
                env.out(f"scorestats.json changed after {elapsed:.0f} s; copied to {dest}")
                return dest
        if elapsed >= cap_s:
            if changed:
                env.out(f"scorestats.json changed but was not valid JSON after {elapsed:.0f} s; "
                        "nothing copied")
            else:
                env.out(f"scorestats.json unchanged after {elapsed:.0f} s; bot launch wrote no stats")
            return None


def _exit_check(game: Game, env: Env) -> None:
    start = env.now()
    while game.is_running():
        if env.now() - start >= EXIT_CHECK_S:
            env.out(f"game still running after {EXIT_CHECK_S:.0f} s (menu or results screen)")
            return
        env.sleep(POLL_INTERVAL_S)
    env.out(f"game exited after {env.now() - start:.0f} s")


def _restore(backup_dir: str, env: Env) -> bool:
    """Put the user files back. On a failure, print the file and the error
    and return False; the caller then stops."""
    env.out(f"restoring the user files from {backup_dir}")
    try:
        game_files.restore(backup_dir, out=env.out)
    except game_files.RestoreError as e:
        env.out(f"restore failed on {e.name}: {e.error}")
        env.out(f"stopped; the backup is still in {backup_dir}")
        return False
    return True


def _restore_hint(backup_dir: str) -> str:
    return f'py -I tools/ch_mcp/bot_run.py --restore "{backup_dir}"'


def run_bot(song_dir: str, out_dir: str, *, player: str = DEFAULT_PLAYER,
            profile: Optional[str] = None, stop: bool = False, no_wait: bool = False,
            backup_root: Optional[str] = None, env: Optional[Env] = None) -> int:
    """One bot launch. Returns 0 when the stats file was copied (or, with
    no_wait, when the game started), 1 when the cap passed with no stats,
    2 when a restore failed."""
    env = env or Env()
    # The game starts in its own folder, so a relative song path would be read
    # from there.
    song_dir = os.path.abspath(song_dir)
    length_ms, source = song_length_ms(song_dir)
    cap_s = C.ms_to_s(length_ms) + POLL_CAP_EXTRA_S
    if (game_files.song_ini_pro_drums(song_dir)
            and player.split(",")[0].strip() != PRO_DRUMS_INSTRUMENT):
        env.out(f"note: song.ini says pro_drums = True but the player is {player}; "
                f"pass --player {PRO_DRUMS_INSTRUMENT},<difficulty> to play pro drums")

    os.makedirs(out_dir, exist_ok=True)
    backup_dir = game_files.backup(env.documents_dir, env.locallow_dir,
                                   backup_root or os.path.join(out_dir, "backups"),
                                   stamp=env.stamp)
    env.out(f"backup folder: {backup_dir}")
    before = _file_state(os.path.join(env.documents_dir, game_files.SCORESTATS))

    cmd = launch_command(env.exe, song_dir, player, profile)
    env.out("starting: " + subprocess.list2cmdline(cmd))
    game = env.launch(cmd)
    env.out(f"game process id: {game.pid}")
    if no_wait:
        env.out("--no-wait: not waiting and not restoring; afterwards run " + _restore_hint(backup_dir))
        return 0

    try:
        env.out(f"waiting up to {cap_s:.1f} s for scorestats.json to change "
                f"({length_ms} ms from {source}, plus {POLL_CAP_EXTRA_S:.0f} s)")
        copied = _poll_stats(song_dir, out_dir, cap_s, before, env)
        if copied is not None:
            _exit_check(game, env)
        if stop and game.is_running():
            game.stop()
            env.out("stopped the game (--stop)")
    except BaseException:
        _restore(backup_dir, env)
        raise
    if not _restore(backup_dir, env):
        return 2
    if game.is_running():
        env.out("the game is still running and may write these files again when it closes; "
                "after closing it, run " + _restore_hint(backup_dir))
    return 0 if copied is not None else 1


def restore_only(backup_dir: str, env: Optional[Env] = None) -> int:
    env = env or Env()
    return 0 if _restore(backup_dir, env) else 2


def main(argv: Optional[list[str]] = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--song", help="the song folder to play")
    ap.add_argument("--out", help="where the stats copy and the backups go")
    ap.add_argument("--player", default=DEFAULT_PLAYER,
                    help=f"<Instrument>,<Difficulty> (default {DEFAULT_PLAYER})")
    ap.add_argument("--profile", help="a game profile name; omit to play as the default player")
    ap.add_argument("--stop", action="store_true", help="close the game when the run ends")
    ap.add_argument("--no-wait", action="store_true",
                    help="back up and start the game, then return without waiting or restoring")
    ap.add_argument("--backup-root", help="where the dated backup folder goes (default <out>\\backups)")
    ap.add_argument("--restore", metavar="BACKUP_FOLDER",
                    help="put the user files back from a backup folder, then stop")
    args = ap.parse_args(argv)

    if args.restore:
        return restore_only(args.restore)
    if not args.song or not args.out:
        ap.error("--song and --out are needed unless --restore is given")
    if not os.path.isfile(game_files.GAME_EXE):
        print(f"the game is not at {game_files.GAME_EXE}; nothing was started")
        return 2
    try:
        return run_bot(args.song, args.out, player=args.player, profile=args.profile,
                       stop=args.stop, no_wait=args.no_wait, backup_root=args.backup_root)
    except SongLengthMissing as e:
        print(e)
        return 2


if __name__ == "__main__":
    sys.exit(main())
