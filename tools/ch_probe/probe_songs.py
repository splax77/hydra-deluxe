"""Write the probe songs as ready-to-play Clone Hero song folders.

Three songs, each a folder with notes.chart, song.ini, a silent song.ogg and a
manifest.json:

  * Window Map  -- no inputs needed. Runs of evenly spaced notes at chosen gaps,
    plus notes with uneven gaps before and after. Watch the engine's window
    field while it plays. Answers: is the spacing capped at 170 ms, is there a
    floor, and which gap (before, after, or both) sets a note's window.
  * Edge Walk   -- isolated notes one second apart, for the input test that
    walks hits later/earlier in 1 ms steps until they turn into misses.
  * Early Fill Walk -- cases where Star Power comes ready a set number of ms
    around a drum fill's deadline, for the FL-24 live check. The layout is
    the EFW_ table below; its manifest also lists each case's times.

The chart uses Resolution 480 at 125 BPM, which makes one tick exactly one
millisecond. Every gap in the file is then the literal number of milliseconds,
with no rounding.

manifest.json lists every note with its time, block and the gap before and
after it, so a watcher script can line up window changes with notes. It holds
layout only -- no predicted window values.

The silent song.ogg is exactly as long as Hydra's song end for the chart
(hydra_end), and song.ini names no length. Finding that end runs hydra_replay
dump, so only a main session runs this script (agent rule 14).

    python -m tools.ch_probe.probe_songs                # installs into CH
    python -m tools.ch_probe.probe_songs --out <dir>    # somewhere else
"""

from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
from dataclasses import dataclass
from typing import Callable, Optional, Sequence

try:
    from . import constants as C
    from .probe_chart import chart_text as _chart_text, ms_to_ticks, ticks_to_ms
except ImportError:  # pragma: no cover - top-level import, ch_probe on sys.path
    import constants as C  # type: ignore[no-redef]
    from probe_chart import chart_text as _chart_text, ms_to_ticks, ticks_to_ms  # type: ignore[no-redef]

# hydra_end lives outside ch_probe, so a top-level import of this file still
# needs the repo root on sys.path for it.
_REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import hydra_end  # noqa: E402

RESOLUTION = 480
BPM = 125.0  # 480 ticks per beat at 125 BPM = 1 tick per ms

# Where the probe songs are installed, the songs' folder names, and the
# prefix that turns a folder name into the name the game lists. The runners
# read these; every probe song folder is written by write_song_folder.
DEFAULT_OUT = r"C:\Clone Hero\songs\Hydra Probe"
WINDOW_MAP = "Window Map"
EDGE_WALK = "Edge Walk"
EARLY_FILL_WALK = "Early Fill Walk"
NAME_PREFIX = "Hydra Probe - "
SONG_OGG = "song.ogg"

LEAD_IN_MS = 3000
SILENCE_MS = 3000  # between blocks; far wider than any window

# A short run at a fixed gap placed before each test run. It pulls the window
# to a known-different value first, so a test run that "doesn't move" the
# window is a real result rather than the engine never updating.
MARKER_GAP_MS = 100
MARKER_NOTES = 5

RUN_NOTES = 8  # notes per test run: the middle ones have the same gap both sides

# Step 1 (cap) and step 2 (floor) gaps, from the testing plan:
# docs/superpowers/plans/2026-09-25-hit-window-testing.md (removed; `git show 444631e6:docs/superpowers/plans/2026-09-25-hit-window-testing.md`)
CAP_GAPS_MS = (150, 170, 180, 200, 250, 400)
FLOOR_GAPS_MS = (30, 40, 50, 60)
# Step 3: (gap before, gap after) for the middle note of a three-note group.
# Each pair also appears reversed, so "before" and "after" can be told apart.
UNEVEN_GAPS_MS = ((60, 140), (140, 60), (90, 130), (130, 90))

EDGE_WALK_NOTES = 120
EDGE_WALK_GAP_MS = 1000

# The Early Fill Walk's layout table, in ms (one tick each). The plan that
# asked for this song gives every number here: "Which chart" in
# docs/superpowers/plans/2026-11-12-early-fill-live-check.md. Each case is a
# phrase note, the SP-ready note, the case fill ending on its last note, and
# the flush fill ending on its last note, then the rest.
#
# A case's distance is how many ms the SP-ready note comes after its fill's
# deadline; a negative distance puts the SP-ready note before the deadline.
EFW_CASES = (("A", -30), ("B", 30), ("C", 60), ("D", 70), ("E", 78))
EFW_LEAD_IN_MS = 20000      # the first case's phrase note
EFW_PHRASE_GAP_MS = 2000    # phrase note to SP-ready note
EFW_FILL_LENGTH_MS = 480    # the case fill and the flush fill
EFW_FLUSH_AFTER_MS = 6000   # case fill's last note to flush fill's last note
EFW_REST_MS = 12000         # flush fill's last note to the next case
# The fill deadline reaches four quarter notes past the tick at which Star
# Power comes ready (docs/clone-hero/04-fills.md, FL-19 and FL-22).
EFW_DEADLINE_BEATS = 4
# A phrase holds a note when start <= tick < start + length, and a phrase of
# length 0 pays nothing (docs/clone-hero/05-star-power.md, SP-01 and SP-02).
# Each phrase starts on its one note and is 10 ticks long, the length the
# corpus charts use most; every other note is over 1,000 ticks away, so the
# phrase holds only that note, which is its last chord and carries the award
# (SP-04).
EFW_PHRASE_LENGTH_TICKS = 10


@dataclass
class Note:
    time_ms: int
    block: str
    # "marker", "run", "first", "middle", "last", "isolated", or an Early
    # Fill Walk role: "phrase", "sp_ready", "fill_last", "flush_last"
    role: str


class Timeline:
    """Lays notes down left to right, in ms. At RESOLUTION and BPM a tick is
    one ms, so the chart's ticks equal these times; chart_text still turns
    each through probe_chart.ms_to_ticks. `phrases` and `fills` hold
    (start ms, length ms) pairs, empty for the songs that write none."""

    def __init__(self) -> None:
        self.notes: list[Note] = []
        self.phrases: list[tuple[int, int]] = []
        self.fills: list[tuple[int, int]] = []
        self.cases: list[dict] = []  # the Early Fill Walk's manifest rows
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


def early_fill_walk() -> Timeline:
    """The Early Fill Walk: one case per EFW_CASES row, laid out from the
    EFW_ table above. Two lone phrase notes each add a quarter of the meter
    (docs/clone-hero/05-star-power.md, SP-12), so hitting the second, the
    SP-ready note, makes Star Power ready. Each fill is written to end
    exactly on a lone kick, which is the chord the fill lands on
    (docs/clone-hero/04-fills.md, FL-03 and FL-05)."""
    t = Timeline()
    deadline_ms = int(ticks_to_ms(EFW_DEADLINE_BEATS * RESOLUTION, RESOLUTION, BPM))
    phrase_ms = int(ticks_to_ms(EFW_PHRASE_LENGTH_TICKS, RESOLUTION, BPM))
    t.cursor = EFW_LEAD_IN_MS
    for label, distance_ms in EFW_CASES:
        block = f"case_{label}"
        phrase_note_ms = t.cursor
        sp_ready_ms = phrase_note_ms + EFW_PHRASE_GAP_MS
        fill_deadline_ms = sp_ready_ms - distance_ms
        fill_start_ms = fill_deadline_ms + deadline_ms
        fill_last_ms = fill_start_ms + EFW_FILL_LENGTH_MS
        flush_last_ms = fill_last_ms + EFW_FLUSH_AFTER_MS
        flush_start_ms = flush_last_ms - EFW_FILL_LENGTH_MS
        for time_ms, role in ((phrase_note_ms, "phrase"), (sp_ready_ms, "sp_ready"),
                              (fill_last_ms, "fill_last"), (flush_last_ms, "flush_last")):
            t.notes.append(Note(time_ms, block, role))
        t.phrases.append((phrase_note_ms, phrase_ms))
        t.phrases.append((sp_ready_ms, phrase_ms))
        t.fills.append((fill_start_ms, EFW_FILL_LENGTH_MS))
        t.fills.append((flush_start_ms, EFW_FILL_LENGTH_MS))
        t.cases.append({
            "case": label,
            "distance_ms": distance_ms,
            "sp_ready_ms": sp_ready_ms,
            "fill_start_ms": fill_start_ms,
            "fill_deadline_ms": fill_deadline_ms,
            "fill_last_note_ms": fill_last_ms,
            "flush_last_note_ms": flush_last_ms,
        })
        t.cursor = flush_last_ms + EFW_REST_MS
    return t


def chart_text(name: str, notes: list[Note],
               phrases: Sequence[tuple[int, int]] = (),
               fills: Sequence[tuple[int, int]] = ()) -> str:
    """The song's notes.chart: every note a kick, through probe_chart's one
    writer, each note's ms turned into its tick at this song's resolution and
    tempo. `phrases` and `fills` are (start ms, length ms) pairs, turned into
    ticks the same way."""
    def ticks(ms: float) -> int:
        return ms_to_ticks(ms, RESOLUTION, BPM)

    return _chart_text(name, [ticks(n.time_ms) for n in notes],
                       resolution=RESOLUTION, bpm=BPM,
                       note=C.PROBE_CHART_NOTE_KICK, music_stream=SONG_OGG,
                       phrases=[(ticks(s), ticks(n)) for s, n in phrases],
                       fills=[(ticks(s), ticks(n)) for s, n in fills])


def song_ini(name: str) -> str:
    """The song's song.ini. It names no length: the song's end is Hydra's
    (write_song_folder), and the silent audio is exactly that long (D331)."""
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
    )


def manifest(name: str, notes: list[Note], cases: Sequence[dict] = ()) -> dict:
    """The song's manifest.json: every note's row, and a `cases` list when the
    song has one (the Early Fill Walk's per-case times)."""
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
    out = {
        "song": name,
        "resolution": RESOLUTION,
        "bpm": BPM,
        "note": "1 tick = 1 ms; chart Offset 0, song.ini delay 0; all kick",
        "notes": rows,
    }
    if cases:
        out["cases"] = [dict(c) for c in cases]
    return out


def write_silent_ogg(path: str, length_ms: float) -> None:
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


def write_song_folder(root: str, name: str, chart: str, *,
                      write_ogg: Callable[[str, float], None] = write_silent_ogg
                      ) -> tuple[str, float]:
    """Make <root>/<name> and write a playable song there: `chart` as
    notes.chart, a song.ini, and a silent song.ogg exactly as long as Hydra's
    song end for that chart (hydra_end.chart_song_length_ms, one hydra_replay
    dump). Returns the folder and that end in ms. Raises
    hydra_end.SongEndMissing, before any audio is written, when the dump
    names no end."""
    folder = os.path.join(root, name)
    os.makedirs(folder, exist_ok=True)
    notes_chart = os.path.join(folder, "notes.chart")
    with open(notes_chart, "w", encoding="utf-8", newline="\n") as f:
        f.write(chart)
    with open(os.path.join(folder, "song.ini"), "w", encoding="utf-8", newline="\n") as f:
        f.write(song_ini(full_name(name)))
    length_ms, _source = hydra_end.chart_song_length_ms(notes_chart)
    write_ogg(os.path.join(folder, SONG_OGG), length_ms)
    return folder, length_ms


def write_song(root: str, name: str, t: Timeline, *,
               write_ogg: Callable[[str, float], None] = write_silent_ogg) -> tuple[str, float]:
    """Write one probe song folder (write_song_folder) plus its manifest.json.
    Returns the folder and the song's length in ms, Hydra's end of its
    chart."""
    title = full_name(name)
    folder, length_ms = write_song_folder(
        root, name, chart_text(title, t.notes, t.phrases, t.fills), write_ogg=write_ogg)
    with open(os.path.join(folder, "manifest.json"), "w", encoding="utf-8", newline="\n") as f:
        json.dump(manifest(title, t.notes, t.cases), f, indent=1)
    return folder, length_ms


# Every probe song, by folder name, with the builder of its layout.
SONGS = ((WINDOW_MAP, window_map), (EDGE_WALK, edge_walk),
         (EARLY_FILL_WALK, early_fill_walk))


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--out", default=DEFAULT_OUT)
    args = ap.parse_args()
    for name, build in SONGS:
        t = build()
        folder, length_ms = write_song(args.out, name, t)
        print(f"{folder}: {len(t.notes)} notes, {C.ms_to_s(length_ms):.1f} s")


if __name__ == "__main__":
    main()
