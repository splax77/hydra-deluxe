"""Print Clone Hero's session log as the game writes it, each line stamped with
the wall time it arrived.

Plain version: the game writes a log file per session, and the overlay spike
asks whether a line such as "Loading Song: ..." shows up while the song is
starting or only later. This script finds the newest session log, watches it
for new bytes, and prints every new line with the time it saw it, to the
millisecond. The main session notes the time it said "go"; the gap to the
Loading Song line is the answer.

It reads a file on disk only. It never opens, reads or sends input to the
game itself.

Usage (from the repo root):
    py tools\\ch_probe\\experiments\\log_tail.py [--logs <folder>]

It stops on Ctrl-C or after TAIL_CAP_S seconds, whichever comes first.
"""

from __future__ import annotations

import argparse
import datetime
import glob
import os
import sys
import time
from typing import Callable, Optional, TextIO, Tuple

# Where Clone Hero writes its session logs on this machine.
LOGS_DIR = r"C:\Users\Patrick\Documents\Clone Hero\Logs"
# The session logs' file names. The date in them is not zero-padded
# (Session-2026-9-25_... sorts after Session-2026-10-17_...), so the newest
# log is picked by its last write time, never by its name.
SESSION_GLOB = "Session-*.log"

# How often the file is checked for new bytes (the plan's Task D step 1).
POLL_S = 0.1
# How long a run lasts at most (the plan's Task D step 1).
TAIL_CAP_S = 300.0

# The line forms parse_log_line knows, as the 2026-10-17 session log writes
# them.
_LOADING = "[Log] Loading Song: "
_SEEK = "[Log] Seeking to song time:"
_COUNTDOWN_START = "[Log] OnCountdownStart"
_COUNTDOWN_END = "[Log] OnCountdownEnd"

Event = Tuple[str, object]


def parse_log_line(line: str) -> Optional[Event]:
    """Name the event one log line records, or None for any other line.

    Returns ("loading", "Artist - Title"), ("countdown_start", None),
    ("countdown_end", None) or ("seek", seconds as a float). A trailing line
    break (the game writes CRLF) is ignored."""
    text = line.rstrip("\r\n")
    if text.startswith(_LOADING):
        return ("loading", text[len(_LOADING):])
    if text == _COUNTDOWN_START:
        return ("countdown_start", None)
    if text == _COUNTDOWN_END:
        return ("countdown_end", None)
    if text.startswith(_SEEK):
        try:
            return ("seek", float(text[len(_SEEK):]))
        except ValueError:
            return None
    return None


def newest_session_log(folder: str) -> Optional[str]:
    """The session log in `folder` written most recently, or None when the
    folder holds none."""
    paths = glob.glob(os.path.join(folder, SESSION_GLOB))
    if not paths:
        return None
    return max(paths, key=os.path.getmtime)


def wall_stamp(moment: datetime.datetime) -> str:
    """A wall time as HH:MM:SS.mmm."""
    return moment.strftime("%H:%M:%S.") + f"{moment.microsecond // 1000:03d}"


def format_line(stamp: str, line: str) -> str:
    """One printed line: the wall time, the log line, and the event it
    records when parse_log_line knows it."""
    text = line.rstrip("\r\n")
    event = parse_log_line(text)
    if event is None:
        return f"{stamp}  {text}"
    kind, value = event
    tag = kind if value is None else f"{kind} {value!r}"
    return f"{stamp}  {text}    <= {tag}"


def tail(
    path: str,
    *,
    cap_s: float = TAIL_CAP_S,
    poll_s: float = POLL_S,
    start_at_end: bool = True,
    now: Callable[[], float] = time.monotonic,
    sleep: Callable[[float], None] = time.sleep,
    wall: Callable[[], datetime.datetime] = datetime.datetime.now,
    out: Optional[TextIO] = None,
) -> int:
    """Print each complete new line of `path` as it is written, stamped with
    wall(). Checks every `poll_s` seconds and returns after `cap_s` seconds;
    Ctrl-C stops it early. Returns the number of lines printed.

    A line the game has only half written waits until its line break arrives,
    so a line is never printed in two pieces. Bytes are split on the line
    break before decoding, so a character cut across two reads still decodes.
    """
    out = sys.stdout if out is None else out
    offset = os.path.getsize(path) if start_at_end else 0
    pending = b""
    printed = 0
    start = now()
    try:
        while True:
            with open(path, "rb") as f:
                f.seek(offset)
                chunk = f.read()
            if chunk:
                offset += len(chunk)
                stamp = wall_stamp(wall())
                pending += chunk
                *complete, pending = pending.split(b"\n")
                for raw in complete:
                    out.write(format_line(
                        stamp, raw.decode("utf-8", errors="replace")) + "\n")
                    printed += 1
                out.flush()
            if now() - start >= cap_s:
                out.write(f"Stopped at the {cap_s:g} s cap.\n")
                return printed
            sleep(poll_s)
    except KeyboardInterrupt:
        out.write("Stopped.\n")
        return printed


def main(argv: Optional[list] = None, *, out: Optional[TextIO] = None) -> int:
    out = sys.stdout if out is None else out
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--logs", default=LOGS_DIR,
                        help="the folder holding the session logs")
    args = parser.parse_args(argv)

    path = newest_session_log(args.logs)
    if path is None:
        out.write(f"No {SESSION_GLOB} in {args.logs}; start Clone Hero first.\n")
        return 2
    out.write(f"Watching {path} (from its current end, "
              f"{os.path.getsize(path)} bytes); cap {TAIL_CAP_S:g} s.\n")
    out.write(f"Started at {wall_stamp(datetime.datetime.now())}.\n")
    out.flush()
    tail(path, out=out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
