"""Hydra's song end, for every Clone Hero tool that needs a song's length.

Hydra has one song end, app::song_length_ms (D255), and hydra_replay writes
it into every dump and schedule as the top-level `song_length_ms` field
(tools/replay.cpp). song_length_ms below reads that field and refuses a file
that lacks it or holds null, so no tool falls back on a length of its own.
A tool that has a chart and no dump gets one through chart_song_length_ms,
which runs `hydra_replay dump` through prep_inputs' runner (the player audit
fix plan's question 3, D324).
"""

from __future__ import annotations

import json
import os
import tempfile

from tools.ch_probe import prep_inputs

FIELD = "song_length_ms"
# The prefix of the scratch folder a dump made for its song end goes in, and
# the folder those go under: None is the system temp folder, and tests point
# it at their own.
DUMP_FOLDER_PREFIX = "hydra_end_"
DUMP_ROOT = None


class SongEndMissing(ValueError):
    """The file names no song end; the message says why."""


def song_length_ms(json_path: str) -> float:
    """The `song_length_ms` field of the hydra_replay dump or schedule at
    `json_path`. Raises SongEndMissing when the file cannot be read, is not
    a JSON object, lacks the field, or holds null or anything but a number
    there."""
    try:
        with open(json_path, encoding="utf-8") as f:
            doc = json.load(f)
    except (OSError, ValueError) as e:
        raise SongEndMissing(f"could not read {json_path}: {type(e).__name__}: {e}") from e
    if not isinstance(doc, dict):
        raise SongEndMissing(f"{json_path} is not a JSON object")
    if FIELD not in doc:
        raise SongEndMissing(f"{json_path} has no {FIELD} field; it was written by a "
                             "hydra_replay older than the song end")
    value = doc[FIELD]
    if value is None:
        raise SongEndMissing(f"{json_path}: {FIELD} is null, so Hydra found no song end "
                             "for the chart")
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise SongEndMissing(f"{json_path}: {FIELD} is {value!r}, not a number")
    return value


def make_dump(chart: str, dump: str) -> None:
    """Write `chart`'s hydra_replay dump to `dump` with prep_inputs' runner
    and its default exe. Raises prep_inputs.ReplayFailed. Tests replace this
    function, because no test runs hydra_replay (agent rule 14)."""
    prep_inputs.run_replay(prep_inputs.dump_command(prep_inputs.DEFAULT_EXE, chart, dump))


def chart_song_length_ms(chart: str) -> tuple[float, str]:
    """Hydra's song end for the chart file `chart`, and the dump it was read
    from: make_dump writes the dump into a new scratch folder, then
    song_length_ms reads it. Raises SongEndMissing, with hydra_replay's own
    lines when the dump could not be made."""
    folder = tempfile.mkdtemp(prefix=DUMP_FOLDER_PREFIX, dir=DUMP_ROOT)
    dump = prep_inputs.file_names(chart, folder)[0]
    try:
        make_dump(chart, dump)
    except prep_inputs.ReplayFailed as e:
        raise SongEndMissing(f"no dump of {chart}, so no song end: {e}") from e
    return song_length_ms(dump), dump


def source_text(dump: str) -> str:
    """Where a length read by chart_song_length_ms came from, for a reply line."""
    return f"{FIELD} in {os.path.abspath(dump)}"
