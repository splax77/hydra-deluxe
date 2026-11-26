"""An offline dry run of the kit agent's frame picker: it plays a schedule
through PressSchedule.step (press_schedule.ts) on a list of frames, with no
game, and counts what the picker reports for each press.

It builds the `play` message exactly as a live kit run does, by calling
play_chart.play_in_game with a stand-in player that keeps the message it is
posted and stops the run there. So the presses, their seqs, their later
groups, the hold, the start lead, the score delay, the stall, the jump back
and the run cap (game_tools.schedule_run_cap_s, as ch_run_chart takes it) are
the live run's own. It works out no window, band or drop rule: the picker
decides every press, and this only adds up the events it returns.

Plain Node runs press_schedule.ts (Node erases its type annotations), as
tools/ch_mcp/tests/test_press_schedule.py does; no frida-compile step and no
package is needed.

The frames:
  --frames CSV      a frame clock log a run wrote (play_chart.FRAME_CLOCK_COLUMNS).
                    Its steps, frame to frame, are played from the start clock
                    on, and played again from its first step when the schedule
                    outlasts them.
  --even-step-ms    one clock step on every frame.
Both lists run when both are given. The first frame's clock is start_clock_ms.
The wall time moves with the clock, one for one; the picker reads the wall only
for its run cap, its stall check and when it reports a press, never to pick a
frame. The score it reads is 0 on every frame, since no game is scoring.

No SP end is read, so no `retime` reaches the picker: a press that waits on one
plays as the schedule planned it, and its event carries the note retime_missed,
which the report counts.

Before it runs, it requires settings.ini's framerate and frame multiplier to be
the values given (game_files owns where the file is and how its lines are read).

    py -I tools/ch_mcp/agent/frame_dry_run.py <schedule.json> --frames <frame_clock.csv>
        --even-step-ms <ms> --framerate <n> --multiplier <n> [--json <out.json>]
"""

from __future__ import annotations

import argparse
import io
import json
import os
import pathlib
import shutil
import subprocess
import sys
from typing import Optional, Sequence

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import game_files, game_tools  # noqa: E402
from tools.ch_probe.experiments import play_chart  # noqa: E402

SCHEDULER = os.path.join(_HERE, "press_schedule.ts")
# The longest one dry run may take before it is called stuck. A full 5000%
# schedule is a few million frames.
NODE_CAP_S = 900

# The driver: reads {"play", "start_clock_ms", "frames"} from stdin, steps one
# PressSchedule frame by frame until it stops, and prints the tally.
DRIVER = r"""
import { readFileSync } from "node:fs";
const { PressSchedule } = await import(%(url)s);
const c = JSON.parse(readFileSync(0, "utf8"));

// One pass of the frame list: each frame's clock and frame number, relative
// to its first.
let relClock, relFrame;
if (c.frames.kind === "csv") {
    const lines = readFileSync(c.frames.path, "utf8").split(/\r?\n/).filter((l) => l.length !== 0);
    const header = lines[0].split(",");
    const fi = header.indexOf(c.frames.frame_column);
    const ci = header.indexOf(c.frames.clock_column);
    if (fi < 0 || ci < 0) {
        throw new Error(`${c.frames.path} has no ${c.frames.frame_column} or ${c.frames.clock_column} column`);
    }
    relClock = new Float64Array(lines.length - 1);
    relFrame = new Float64Array(lines.length - 1);
    let clock0 = null, frame0 = null;
    for (let k = 1; k < lines.length; k++) {
        const cells = lines[k].split(",");
        const clock = Number(cells[ci]), frame = Number(cells[fi]);
        if (!Number.isFinite(clock) || !Number.isInteger(frame)) {
            throw new Error(`${c.frames.path} line ${k + 1} is not a frame and a clock: ${lines[k]}`);
        }
        if (clock0 === null) { clock0 = clock; frame0 = frame; }
        relClock[k - 1] = clock - clock0;
        relFrame[k - 1] = frame - frame0;
        // The wall follows the clock, so a step that does not move it forward
        // would hold the wall still or turn it back.
        if (k > 1 && !(relClock[k - 1] > relClock[k - 2] && relFrame[k - 1] > relFrame[k - 2])) {
            throw new Error(`${c.frames.path} line ${k + 1} does not move the clock and the frame forward`);
        }
    }
    if (relClock.length < 2) {
        throw new Error(`${c.frames.path} holds fewer than two frames, so no step`);
    }
} else {
    if (!(c.frames.step_ms > 0)) {
        throw new Error(`an even step must be above 0: ${c.frames.step_ms}`);
    }
    relClock = Float64Array.of(0, c.frames.step_ms);
    relFrame = Float64Array.of(0, 1);
}
const period = relClock.length - 1;
const span = relClock[period], frameSpan = relFrame[period];

const sched = new PressSchedule(c.play, () => 0);
const t = {
    entries: sched.slots.size, frames: 0, loops: 0, sent: 0, sent_no_frame_in_band: 0,
    sent_judged_outside_band: 0, dropped: 0, dropped_no_frame_in_band: 0, skipped: 0,
    retime_missed: 0, stop: null, max_examined: 0, max_examined_frame: null,
    max_examined_clock_ms: null, dropped_seqs: [], no_frame_in_band_seqs: [],
    judged_outside_band_seqs: [],
};
const tally = (events) => {
    for (const e of events) {
        if (e.kind === "skipped") {
            t.skipped += e.count;
        } else if (e.kind === "stop") {
            t.stop = e.reason;
        } else if (e.kind === "sent" || e.kind === "dropped") {
            const seen = { seq: e.seq, index: e.index, clock_ms: e.clock_ms_at_press };
            if (e.kind === "sent") {
                t.sent += 1;
                if (e.no_frame_in_band) { t.sent_no_frame_in_band += 1; }
                if (e.judged_in_band === false) {
                    t.sent_judged_outside_band += 1;
                    t.judged_outside_band_seqs.push({ ...seen, judged_ms: e.judged_ms });
                }
            } else {
                t.dropped += 1;
                t.dropped_seqs.push(seen);
                if (e.no_frame_in_band) { t.dropped_no_frame_in_band += 1; }
            }
            if (e.no_frame_in_band) { t.no_frame_in_band_seqs.push(seen); }
            if (e.agent_note === "retime_missed") { t.retime_missed += 1; }
        }
    }
};
for (let k = 0; sched.stopReason === null; k++) {
    const loop = Math.floor(k / period), j = k %% period;
    const rel = loop * span + relClock[j];
    const frame = loop * frameSpan + relFrame[j];
    const clock = c.start_clock_ms + rel;
    const wall = rel;
    const s = sched.step(frame, clock, wall);
    t.frames += 1;
    t.loops = loop;
    if (sched.examined > t.max_examined) {
        t.max_examined = sched.examined;
        t.max_examined_frame = k;
        t.max_examined_clock_ms = clock;
    }
    tally(s.events);
    // The picker stops itself at its cap (stopFor); a frame past it with no
    // stop is a fault in the picker, not a long run.
    if (sched.stopReason === null && wall >= c.play.cap_ms) {
        throw new Error(`frame ${k} is past the play's cap_ms ${c.play.cap_ms} and the picker did not stop`);
    }
}
if (t.sent + t.dropped + t.skipped !== t.entries) {
    throw new Error(`the picker reported ${t.sent + t.dropped + t.skipped} of ${t.entries} entries`);
}
process.stdout.write(JSON.stringify(t));
"""


class SettingsMismatch(RuntimeError):
    """settings.ini does not hold the framerate or multiplier the run needs."""


class _Captured(Exception):
    pass


class _CapturePlayer:
    """Stands in for kit_tools.GamePlayer for one post: it keeps the `play`
    message and stops play_in_game there, before any event is waited for."""

    def __init__(self) -> None:
        self.message: Optional[dict] = None

    def post(self, message: dict) -> None:
        self.message = message
        raise _Captured()


def play_message(schedule_json: str) -> dict:
    """The `play` message a live kit run of `schedule_json` posts, built by
    play_in_game itself, with ch_run_chart's run cap. The clock and score
    addresses are 0: the picker does not read them."""
    chords = play_chart.load_schedule(schedule_json).chords
    cap_s = game_tools.schedule_run_cap_s(schedule_json)[0]
    player = _CapturePlayer()
    try:
        play_chart.play_in_game(chords, player, clock_addr=0, score_addr=0, cap_s=cap_s,
                                out=io.StringIO())
    except _Captured:
        return player.message
    raise RuntimeError("play_in_game returned without posting its play message")


def start_clock_ms(play: dict) -> float:
    """The first frame's clock: two start leads before the earliest entry.
    One lead would put that entry on the picker's skip edge, where rounding
    decides; the second keeps it clear and gives the picker frames to measure
    its step on before any press comes due."""
    return min(e["at_ms"] for e in play["entries"]) - 2 * play["start_lead_ms"]


def run_frames(play: dict, *, csv_path: Optional[str] = None,
               even_step_ms: Optional[float] = None) -> dict:
    """Step the picker over one frame list (exactly one of `csv_path` and
    `even_step_ms`) and return the driver's tally."""
    if (csv_path is None) == (even_step_ms is None):
        raise ValueError("give one frame list: a frame clock CSV or an even step")
    node = shutil.which("node")
    if node is None:
        raise RuntimeError("node is not on PATH, so press_schedule.ts cannot run")
    if csv_path is not None:
        frame_col, clock_col = play_chart.FRAME_CLOCK_COLUMNS
        frames = {"kind": "csv", "path": os.path.abspath(csv_path),
                  "frame_column": frame_col, "clock_column": clock_col}
    else:
        frames = {"kind": "even", "step_ms": even_step_ms}
    driver = DRIVER % {"url": json.dumps(pathlib.Path(SCHEDULER).as_uri())}
    done = subprocess.run(
        [node, "--input-type=module", "-e", driver],
        input=json.dumps({"play": play, "start_clock_ms": start_clock_ms(play), "frames": frames}),
        capture_output=True, text=True, timeout=NODE_CAP_S, check=False)
    if done.returncode != 0:
        raise RuntimeError(f"node exited {done.returncode}: {done.stderr}")
    return json.loads(done.stdout)


def require_settings(framerate: str, multiplier: str,
                     documents_dir: str = game_files.DOCUMENTS_DIR) -> None:
    """Raise SettingsMismatch unless settings.ini holds `framerate` and
    `multiplier`, compared as the file writes them."""
    path = os.path.join(documents_dir, game_files.SETTINGS)
    have = (game_files.get_video_setting(path, game_files.FRAMERATE),
            game_files.get_game_setting(path, game_files.FRAME_MULTIPLIER))
    if have != (framerate, multiplier):
        raise SettingsMismatch(
            f"{path} holds {game_files.FRAMERATE} {have[0]} and {game_files.FRAME_MULTIPLIER} "
            f"{have[1]}; this run needs {framerate} and {multiplier}")


def report_line(name: str, t: dict) -> str:
    return (f"{name}: {t['frames']} frames, stop {t['stop']}; "
            f"{t['entries']} presses: sent {t['sent']} ({t['sent_no_frame_in_band']} "
            f"no_frame_in_band, {t['sent_judged_outside_band']} judged outside their band), "
            f"dropped {t['dropped']} ({t['dropped_no_frame_in_band']} no_frame_in_band), "
            f"skipped {t['skipped']}, retime_missed {t['retime_missed']}; largest examined in "
            f"one frame {t['max_examined']} (frame {t['max_examined_frame']}, clock "
            f"{t['max_examined_clock_ms']} ms)")


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(
        description="Play a schedule through the kit agent's frame picker offline.")
    parser.add_argument("schedule_json", help="a hydra_replay schedule JSON")
    parser.add_argument("--frames", help="a frame clock CSV a run wrote")
    parser.add_argument("--even-step-ms", type=float, help="an even clock step, in ms")
    parser.add_argument("--framerate", required=True,
                        help="the framerate settings.ini must hold")
    parser.add_argument("--multiplier", required=True,
                        help="the frame multiplier settings.ini must hold")
    parser.add_argument("--json", help="write every tally, with its seq lists, here")
    args = parser.parse_args(argv)
    if args.frames is None and args.even_step_ms is None:
        parser.error("give --frames, --even-step-ms or both")
    try:
        require_settings(args.framerate, args.multiplier)
    except (SettingsMismatch, game_files.SettingNotFound, OSError) as e:
        print(f"Refused: {e}")
        return 1
    play = play_message(args.schedule_json)
    print(f"{os.path.basename(args.schedule_json)}: {len(play['entries'])} presses, first "
          f"frame clock {start_clock_ms(play)} ms, cap {play['cap_ms']} ms")
    tallies = {}
    if args.frames is not None:
        tallies["recorded"] = run_frames(play, csv_path=args.frames)
        print(report_line(f"recorded steps of {args.frames} (played again "
                          f"{tallies['recorded']['loops']} times)", tallies["recorded"]))
    if args.even_step_ms is not None:
        tallies["even"] = run_frames(play, even_step_ms=args.even_step_ms)
        print(report_line(f"even {args.even_step_ms} ms steps", tallies["even"]))
    if args.json is not None:
        with open(args.json, "w", encoding="utf-8") as f:
            json.dump(tallies, f, indent=1)
    clean = all(t["dropped"] == 0 and not t["no_frame_in_band_seqs"] for t in tallies.values())
    print("Clean: no press dropped or marked no_frame_in_band." if clean
          else "Not clean: a press was dropped or marked no_frame_in_band.")
    return 0 if clean else 2


if __name__ == "__main__":
    sys.exit(main())
