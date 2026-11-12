"""Make the auto-player's input files for one chart, and say whether a run
can start from them.

    py -I tools/ch_probe/prep_inputs.py --chart <notes.chart or notes.mid>
        --out <folder> [--exe <hydra_replay.exe>] [--cap 4] [--ms 10]

It runs `hydra_replay dump` on the chart, then `hydra_replay score --path
<dump> --index 0`, then `hydra_replay schedule --path <dump> --index 0`, and
writes `<name>-dump.json`, `<name>-score.json` and `<name>-schedule.json` into
the folder, where <name> is the chart's folder name. Then it loads the
schedule the way ch_run_chart does (game_tools.chords_to_play), and prints the
lines that run would print, or the loader's refusal. It prints the exe's file
date first, so a stale build shows.

Design point 10 of docs/superpowers/plans/2026-11-07-player-limits.md is why
it exists: the files were made by hand before, with scripts nobody kept.

It calls hydra_replay, so only a main session runs it (agent rule 14).
Exit codes: 0 the files load, 1 the loader refused them, 2 hydra_replay could
not be run or failed.
"""

from __future__ import annotations

import argparse
import datetime
import os
import subprocess
import sys
from typing import Callable, Optional, Sequence, TextIO

_REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

# game_tools is imported inside main: game_tools reaches this module's runner
# through hydra_end, so a top-level import here would be a cycle.

# Profile 39's cap and ms settings (Expert Pro Drums, 2x Bass, cap 4, ms 10),
# as the README of the 2026-11-07 limits-prep folder under
# hydra-data\ch-mcp-runs records them.
DEFAULT_CAP = "4"
DEFAULT_MS = "10"

# The build this checkout makes (build_cpp.ps1's Release folder).
DEFAULT_EXE = os.path.join(_REPO_ROOT, "build-cpp", "Release", "hydra_replay.exe")

# The longest one hydra_replay call may take before the script gives up on it.
# The user chose 120 s ("Use 120 s", 2026-11-07, session d0a3868c).
REPLAY_CAP_S = 120.0

EXIT_LOADS = 0
EXIT_REFUSED = 1
EXIT_REPLAY_FAILED = 2


def file_names(chart: str, out_dir: str):
    """The dump, score and schedule file paths for `chart` in `out_dir`."""
    name = os.path.basename(os.path.dirname(os.path.abspath(chart)))
    return (os.path.join(out_dir, f"{name}-dump.json"),
            os.path.join(out_dir, f"{name}-score.json"),
            os.path.join(out_dir, f"{name}-schedule.json"))


class ReplayFailed(Exception):
    """hydra_replay could not be run, did not finish in REPLAY_CAP_S or
    failed. The message is the lines that say which."""


def dump_command(exe: str, chart: str, dump: str, cap: str = DEFAULT_CAP,
                 ms: str = DEFAULT_MS) -> list:
    """The `hydra_replay dump` command line that writes `chart`'s dump to `dump`."""
    return [exe, "dump", "--chart", chart, "--cap", cap, "--ms", ms, "--out", dump]


def run_replay(command: Sequence[str], run: Callable = subprocess.run) -> None:
    """Run one hydra_replay command, capped at REPLAY_CAP_S. Raises
    ReplayFailed when it cannot be run, runs past the cap or exits non-zero.
    `run` stands in for subprocess.run in tests."""
    try:
        done = run(list(command), capture_output=True, text=True, timeout=REPLAY_CAP_S)
    except subprocess.TimeoutExpired:
        raise ReplayFailed(f"hydra_replay {command[1]} did not finish in {REPLAY_CAP_S:g} s "
                           f"(REPLAY_CAP_S)") from None
    except OSError as e:
        raise ReplayFailed(f"hydra_replay {command[1]} could not be run: {e}") from e
    if done.returncode != 0:
        raise ReplayFailed(f"hydra_replay {command[1]} failed with exit code {done.returncode}:\n"
                           + (done.stderr or done.stdout or "").strip())


def _exe_date(exe: str) -> str:
    stamp = datetime.datetime.fromtimestamp(os.path.getmtime(exe))
    return stamp.strftime("%Y-%m-%d %H:%M")


def main(argv: Optional[Sequence[str]] = None, *, out: Optional[TextIO] = None,
         run: Callable = subprocess.run) -> int:
    """Make the files, load them and print what a run would print. Returns
    the exit code. `run` stands in for subprocess.run in tests."""
    from tools.ch_mcp import game_tools  # noqa: PLC0415  (a cycle at the top; see above)
    out = sys.stdout if out is None else out
    parser = argparse.ArgumentParser(
        description="Make a chart's hydra_replay dump, score and schedule files and "
                    "dry-load the schedule.")
    parser.add_argument("--chart", required=True, help="the chart's notes.chart or notes.mid")
    parser.add_argument("--out", required=True, help="the folder the three files go in")
    parser.add_argument("--exe", default=DEFAULT_EXE, help="hydra_replay.exe to run")
    parser.add_argument("--cap", default=DEFAULT_CAP, help="dump's --cap")
    parser.add_argument("--ms", default=DEFAULT_MS, help="dump's --ms")
    args = parser.parse_args(argv)

    if not os.path.isfile(args.exe):
        print(f"hydra_replay not found: {args.exe}", file=out)
        return EXIT_REPLAY_FAILED
    print(f"hydra_replay: {args.exe}, file date {_exe_date(args.exe)}", file=out)
    os.makedirs(args.out, exist_ok=True)
    dump, score, schedule = file_names(args.chart, args.out)

    commands = [
        dump_command(args.exe, args.chart, dump, args.cap, args.ms),
        [args.exe, "score", "--chart", args.chart, "--path", dump, "--index", "0",
         "--out", score],
        [args.exe, "schedule", "--chart", args.chart, "--path", dump, "--index", "0",
         "--out", schedule],
    ]
    for command in commands:
        try:
            run_replay(command, run)
        except ReplayFailed as e:
            print(e, file=out)
            return EXIT_REPLAY_FAILED
    print(f"dump: {dump}", file=out)
    print(f"score: {score}", file=out)
    print(f"schedule: {schedule}", file=out)

    try:
        chords, lines = game_tools.chords_to_play(schedule)
    except game_tools.ChordsRefused as e:
        print(f"refused: {e}", file=out)
        return EXIT_REFUSED
    print(f"loads: {len(chords)} chords", file=out)
    for line in lines:
        print(line, file=out)
    return EXIT_LOADS


if __name__ == "__main__":
    sys.exit(main())
