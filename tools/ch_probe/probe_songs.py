"""Write the hit-window probe songs as ready-to-play Clone Hero song folders.

Two songs, each a folder with notes.chart, song.ini, a silent song.ogg and a
manifest.json:

  * Window Map  -- no inputs needed. Runs of evenly spaced notes at chosen gaps,
    plus notes with uneven gaps before and after. Watch the engine's window
    field while it plays. Answers: is the spacing capped at 170 ms, is there a
    floor, and which gap (before, after, or both) sets a note's window.
  * Edge Walk   -- isolated notes one second apart, for the input test that
    walks hits later/earlier in 1 ms steps until they turn into misses.

The chart uses Resolution 480 at 125 BPM, which makes one tick exactly one
millisecond. Every gap in the file is then the literal number of milliseconds,
with no rounding.

manifest.json lists every note with its time, block and the gap before and
after it, so a watcher script can line up window changes with notes. It holds
layout only -- no predicted window values.

    python -m tools.ch_probe.probe_songs                # installs into CH
    python -m tools.ch_probe.probe_songs --out <dir>    # somewhere else
"""

from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
from dataclasses import dataclass
from typing import Callable, Optional

try:
    from . import constants as C
    from .probe_chart import chart_text as _chart_text, ms_to_ticks
except ImportError:  # pragma: no cover - top-level import, ch_probe on sys.path
    import constants as C  # type: ignore[no-redef]
    from probe_chart import chart_text as _chart_text, ms_to_ticks  # type: ignore[no-redef]

RESOLUTION = 480
BPM = 125.0  # 480 ticks per beat at 125 BPM = 1 tick per ms

# Where the probe songs are installed, the two songs' folder names, and the
# prefix that turns a folder name into the name the game lists. The runners
# read these; every probe song folder is written by write_song_folder.
DEFAULT_OUT = r"C:\Clone Hero\songs\Hydra Probe"
WINDOW_MAP = "Window Map"
EDGE_WALK = "Edge Walk"
NAME_PREFIX = "Hydra Probe - "

LEAD_IN_MS = 3000
SILENCE_MS = 3000  # between blocks; far wider than any window

# A short run at a fixed gap placed before each test run. It pulls the window
# to a known-different value first, so a test run that "doesn't move" the
# window is a real result rather than the engine never updating.
MARKER_GAP_MS = 100
MARKER_NOTES = 5

RUN_NOTES = 8  # notes per test run: the middle ones have the same gap both sides

# From the testing notes: step 1 (cap) and step 2 (floor) gaps.
CAP_GAPS_MS = (150, 170, 180, 200, 250, 400)
FLOOR_GAPS_MS = (30, 40, 50, 60)
# Step 3: (gap before, gap after) for the middle note of a three-note group.
# Each pair also appears reversed, so "before" and "after" can be told apart.
UNEVEN_GAPS_MS = ((60, 140), (140, 60), (90, 130), (130, 90))

EDGE_WALK_NOTES = 120
EDGE_WALK_GAP_MS = 1000


@dataclass
class Note:
    time_ms: int
    block: str
    role: str  # "marker", "run", "first", "middle", "last", "isolated"


class Timeline:
    """Lays notes down left to right, in ms. At RESOLUTION and BPM a tick is
    one ms, so the chart's ticks equal these times; chart_text still turns
    each through probe_chart.ms_to_ticks."""

    def __init__(self) -> None:
        self.notes: list[Note] = []
        self.cursor = LEAD_IN_MS  # where the next note goes

    def add(self, block: str, role: str) -> None:
        self.notes.append(Note(self.cursor, block, role))

    def run(self, block: str, role: str, gap_ms: int, count: int) -> None:
        """`count` notes, `gap_ms` apart. The first lands at the cursor."""
        for i in range(count):
            if i:
                self.cursor += gap_ms
            self.add(block, role)

    def step(self, gap_ms: int) -> None:
        self.cursor += gap_ms

    def silence(self) -> None:
        self.cursor += SILENCE_MS


def window_map() -> Timeline:
    t = Timeline()
    for label, gaps in (("cap", CAP_GAPS_MS), ("floor", FLOOR_GAPS_MS)):
        for gap in gaps:
            block = f"{label}_{gap}"
            t.run(block, "marker", MARKER_GAP_MS, MARKER_NOTES)
            t.step(gap)
            t.run(block, "run", gap, RUN_NOTES)
            t.silence()
    for before, after in UNEVEN_GAPS_MS:
        block = f"uneven_{before}_{after}"
        t.add(block, "first")
        t.step(before)
        t.add(block, "middle")
        t.step(after)
        t.add(block, "last")
        t.silence()
    return t


def edge_walk() -> Timeline:
    t = Timeline()
    t.run("isolated", "isolated", EDGE_WALK_GAP_MS, EDGE_WALK_NOTES)
    return t


def chart_text(name: str, notes: list[Note]) -> str:
    """The song's notes.chart: every note a kick, through probe_chart's one
    writer, each note's ms turned into its tick at this song's resolution and
    tempo."""
    ticks = [ms_to_ticks(n.time_ms, RESOLUTION, BPM) for n in notes]
    return _chart_text(name, ticks, resolution=RESOLUTION, bpm=BPM,
                       note=C.PROBE_CHART_NOTE_KICK, music_stream="song.ogg")


def song_ini(name: str, length_ms: int) -> str:
    return (
        "[song]\n"
        f"name = {name}\n"
        "artist = Hydra ch_probe\n"
        "charter = ch_probe\n"
        "genre = Test\n"
        "year = 2026\n"
        "diff_drums = 0\n"
        "pro_drums = False\n"
        "delay = 0\n"
        f"song_length = {length_ms}\n"
    )


def manifest(name: str, notes: list[Note]) -> dict:
    rows = []
    for i, n in enumerate(notes):
        before: Optional[int] = n.time_ms - notes[i - 1].time_ms if i else None
        after: Optional[int] = (
            notes[i + 1].time_ms - n.time_ms if i + 1 < len(notes) else None
        )
        rows.append({
            "index": i,
            "time_ms": n.time_ms,
            "block": n.block,
            "role": n.role,
            "gap_before_ms": before,
            "gap_after_ms": after,
        })
    return {
        "song": name,
        "resolution": RESOLUTION,
        "bpm": BPM,
        "note": "1 tick = 1 ms; chart Offset 0, song.ini delay 0; all kick",
        "notes": rows,
    }


def write_silent_ogg(path: str, length_ms: int) -> None:
    ffmpeg = shutil.which("ffmpeg")
    if ffmpeg is None:
        raise RuntimeError("ffmpeg not on PATH; needed to write song.ogg")
    subprocess.run(
        [ffmpeg, "-y", "-loglevel", "error", "-f", "lavfi",
         "-i", "anullsrc=r=44100:cl=stereo", "-t", f"{C.ms_to_s(length_ms):.3f}",
         "-c:a", "libvorbis", "-q:a", "0", path],
        check=True,
    )


def full_name(name: str) -> str:
    """The name the game lists for the probe song in folder `name`."""
    return NAME_PREFIX + name


def song_length_ms(last_note_ms: int) -> int:
    """A probe song's length: its last note, then SILENCE_MS of quiet."""
    return last_note_ms + SILENCE_MS


def write_song_folder(root: str, name: str, chart: str, length_ms: int, *,
                      write_ogg: Callable[[str, int], None] = write_silent_ogg) -> str:
    """Make <root>/<name> and write a playable song there: `chart` as
    notes.chart, a song.ini and a silent song.ogg of `length_ms`. Returns the
    folder."""
    folder = os.path.join(root, name)
    os.makedirs(folder, exist_ok=True)
    with open(os.path.join(folder, "notes.chart"), "w", encoding="utf-8", newline="\n") as f:
        f.write(chart)
    with open(os.path.join(folder, "song.ini"), "w", encoding="utf-8", newline="\n") as f:
        f.write(song_ini(full_name(name), length_ms))
    write_ogg(os.path.join(folder, "song.ogg"), length_ms)
    return folder


def write_song(root: str, name: str, t: Timeline) -> tuple[str, int]:
    """Write one probe song folder plus its manifest.json. Returns the folder
    and the song's length in ms."""
    length_ms = song_length_ms(t.notes[-1].time_ms)
    title = full_name(name)
    folder = write_song_folder(root, name, chart_text(title, t.notes), length_ms)
    with open(os.path.join(folder, "manifest.json"), "w", encoding="utf-8", newline="\n") as f:
        json.dump(manifest(title, t.notes), f, indent=1)
    return folder, length_ms


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--out", default=DEFAULT_OUT)
    args = ap.parse_args()
    for name, build in ((WINDOW_MAP, window_map), (EDGE_WALK, edge_walk)):
        t = build()
        folder, length_ms = write_song(args.out, name, t)
        print(f"{folder}: {len(t.notes)} notes, {C.ms_to_s(length_ms):.1f} s")


if __name__ == "__main__":
    main()
