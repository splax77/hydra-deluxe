"""Copy one chart folder into the game's probe song folder with a silent
song.ogg and a title marked as a probe, so the bot can play it from the
command line.

    py -I tools/ch_mcp/install_chart.py <chart folder> [--name <folder name>]
        [--dest <songs folder>]

The chart folder needs a song.ini with a song_length (game_files reads it)
and a notes.chart or notes.mid. The copy goes into probe_songs.DEFAULT_OUT
unless --dest says otherwise, and the silent audio comes from
probe_songs.write_silent_ogg (ffmpeg on PATH). The copy's song.ini title takes
the mark from probe_songs.full_name, so nobody mistakes a song with no audio
for a real one in the game's or Hydra's list; the source folder keeps its
title. An existing folder is never overwritten. Only the one named folder is
copied.
"""

from __future__ import annotations

import argparse
import os
import shutil
import stat
import sys
from typing import Callable, Optional

if __package__ in (None, ""):
    # Run as a script (py -I leaves the repo off sys.path).
    sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..")))

from tools.ch_mcp import game_files  # noqa: E402
from tools.ch_probe import probe_songs  # noqa: E402

CHART_FILES = ("notes.chart", "notes.mid")


class InstallRefused(Exception):
    pass


def chart_file(folder: str) -> Optional[str]:
    """The folder's chart file, or None when it has neither kind."""
    for name in CHART_FILES:
        path = os.path.join(folder, name)
        if os.path.isfile(path):
            return path
    return None


def probe_title(title: str, folder_name: str) -> str:
    """The title a copy in `folder_name` lists: its own `title`, or the folder
    name when it has none, marked as a probe once."""
    title = title or folder_name
    return title if title.startswith(probe_songs.NAME_PREFIX) else probe_songs.full_name(title)


def mark_copy(dest: str) -> None:
    """Mark the title in the song.ini of the copy at `dest`."""
    # The copy keeps the source's file mode, and a read-only file can't be edited.
    os.chmod(os.path.join(dest, game_files.SONG_INI), stat.S_IREAD | stat.S_IWRITE)
    game_files.set_song_title(dest, lambda title: probe_title(title, os.path.basename(dest)))


def install(src: str, *, dest_root: str = probe_songs.DEFAULT_OUT, name: Optional[str] = None,
            write_ogg: Callable[[str, int], None] = probe_songs.write_silent_ogg) -> str:
    """Copy `src` to `dest_root`/`name`, mark the copy's title and write its
    silent song.ogg. Returns the new folder's chart file. Raises
    InstallRefused, with the reason, before anything is written."""
    src = os.path.abspath(src)
    if not os.path.isdir(src):
        raise InstallRefused(f"{src} is not a folder")
    length_ms = game_files.song_ini_length_ms(src)
    if length_ms is None:
        raise InstallRefused(f"{src} has no song.ini with a song_length; the silent audio "
                             "would have no length")
    if chart_file(src) is None:
        raise InstallRefused(f"{src} has no {' or '.join(CHART_FILES)}")
    dest = os.path.join(dest_root, name or os.path.basename(src))
    if os.path.exists(dest):
        raise InstallRefused(f"{dest} already exists; not overwritten")
    try:
        shutil.copytree(src, dest)
        mark_copy(dest)
        write_ogg(os.path.join(dest, probe_songs.SONG_OGG), length_ms)
    except BaseException:
        # `dest` did not exist a moment ago, so it is ours: a half-made folder
        # would show in the game's list and could not be played, and the next
        # try would refuse it as already existing.
        shutil.rmtree(dest, ignore_errors=True)
        raise
    return chart_file(dest)


def main(argv: Optional[list[str]] = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("chart_folder", help="one chart folder, for example under testdata/input")
    ap.add_argument("--name", help="the folder name in the game (default: the source folder's name)")
    ap.add_argument("--dest", default=probe_songs.DEFAULT_OUT,
                    help=f"the songs folder to copy into (default {probe_songs.DEFAULT_OUT})")
    args = ap.parse_args(argv)
    try:
        chart = install(args.chart_folder, dest_root=args.dest, name=args.name)
    except InstallRefused as e:
        print(f"refused: {e}")
        return 2
    print(f"installed; its chart file is {chart}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
