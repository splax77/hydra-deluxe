"""Poll the engine's total_window field while you play a song.

Finds the engine object, then polls total_window at ~200 Hz for up to
60 seconds, recording every value change with a timestamp. At the end,
writes the collected data and prints a summary of the window range.

This is the passive probe without the debugger — it sees the STORED
window (after any clamp the engine applies), not the raw formula output.
Clone Hero caps the whole window at a measured 171.43 ms (constants.WINDOW_CAP_MS).
A stored value above that means no cap; values that top out there mean the cap held.

Start a song, then run:
    python tools\\ch_probe\\experiments\\poll_windows.py

Press Ctrl+C to stop early.
"""

from __future__ import annotations

import ctypes
import csv
import os
import struct
import sys
import time

_REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants
from tools.ch_probe.process import open_process

from tools.ch_probe.engine_finder import scan_for_engine

RESULTS_DIR = os.path.join(os.path.dirname(__file__), "results")
POLL_HZ = 200
DURATION_S = 60


def window_verdict(windows: list[float], back_ms: float) -> list[str]:
    """Plain-English verdict lines for a run's stored windows, in ms.

    Judged against the measured normal-mode cap and floor (constants.py).
    `back_ms` is the engine's live back constant; when it is not the normal
    85 ms the game is in precision mode, whose cap nobody has read.
    """
    tol = constants.WINDOW_MATCH_TOLERANCE_MS
    w_min, w_max = min(windows), max(windows)
    if abs(back_ms - constants.EXPECT_NORMAL_BACK_MS) > constants.CONST_MATCH_TOLERANCE_MS:
        return [f"  Back window is {back_ms:.1f} ms, not the normal "
                f"{constants.EXPECT_NORMAL_BACK_MS:.0f}: precision mode, "
                "which has no measured cap yet."]
    lines = [f"  Measured cap {constants.WINDOW_CAP_MS} ms, floor {constants.WINDOW_FLOOR_MS} ms."]
    if w_max > constants.WINDOW_CAP_MS + tol:
        lines.append(f"  *** Window EXCEEDED the measured cap: max {w_max:.3f} ms. NO CLAMP. ***")
    elif abs(w_max - constants.WINDOW_CAP_MS) <= tol:
        lines.append("  The window reached the measured cap and never passed it.")
    elif len(set(round(w, 2) for w in windows)) == 1:
        lines.append("  Window never changed: either notes were uniform or no notes hit.")
    else:
        lines.append("  The window stayed below the cap: the song may have had no gaps of "
                     f"{constants.CAP_FROM_GAP_MS:.0f} ms or more.")
    if abs(w_min - constants.WINDOW_FLOOR_MS) <= tol:
        lines.append("  It also reached the measured floor (gaps of "
                     f"{constants.FLOOR_UP_TO_GAP_MS:.0f} ms or less).")
    return lines


def main() -> None:
    print(f"Looking for {constants.PROCESS_NAME}...")
    proc = open_process()
    proc.verify_targets()
    print(f"  PID {proc.pid}, addresses verified.")

    # Read exact bytes for the signature
    back_bytes = proc.read(proc.resolve(constants.RVA_CONST_NORMAL_BACK), 8)
    front_bytes = proc.read(proc.resolve(constants.RVA_CONST_NORMAL_FRONT), 8)

    print("  Scanning for engine object...")
    hits, _, _ = scan_for_engine(proc, back_bytes, front_bytes)
    module_end = proc.module_base + 0x4000000
    heap_hits = [h for h in hits if not (proc.module_base <= h < module_end)]

    if not heap_hits:
        print("  No engine object found. Start a song first.")
        proc.close()
        return

    engine_ptr = heap_hits[0]
    print(f"  Engine at {engine_ptr:#x}")

    tw = proc.read_double(engine_ptr + 0x20)
    bw = proc.read_double(engine_ptr + 0x30)
    print(f"  total_window = {tw*1000:.2f} ms, back_window = {bw*1000:.1f} ms")

    # Poll
    print(f"\n  Polling at ~{POLL_HZ} Hz for up to {DURATION_S}s...")
    print("  Play a song with varied note spacings. Ctrl+C to stop.\n")

    interval = 1.0 / POLL_HZ
    rows = []  # (elapsed_s, total_window_ms)
    t0 = time.perf_counter()
    last_w = None
    changes = 0

    try:
        while True:
            now = time.perf_counter()
            elapsed = now - t0
            if elapsed >= DURATION_S:
                break

            try:
                w = proc.read_double(engine_ptr + 0x20)
            except OSError:
                print("  Lost the process (song ended or game closed).")
                break

            w_ms = w * 1000.0

            if last_w is None or abs(w_ms - last_w) > 0.001:
                rows.append((elapsed, w_ms))
                changes += 1
                if changes <= 50 or changes % 20 == 0:
                    print(f"  {elapsed:7.3f}s  window = {w_ms:8.3f} ms")
                last_w = w_ms

            time.sleep(interval)

    except KeyboardInterrupt:
        print("\n  Stopped by user.")

    elapsed_total = time.perf_counter() - t0
    print(f"\n  Collected {len(rows)} distinct values in {elapsed_total:.1f}s")

    if not rows:
        print("  No data. Was a song playing?")
        proc.close()
        return

    windows = [r[1] for r in rows]
    w_min = min(windows)
    w_max = max(windows)
    back_ms = bw * 1000.0

    print(f"  Window range: {w_min:.3f} — {w_max:.3f} ms")
    print(f"  Back window (constant): {back_ms:.1f} ms")
    for line in window_verdict(windows, back_ms):
        print(line)

    # Write results
    os.makedirs(RESULTS_DIR, exist_ok=True)
    csv_path = os.path.join(RESULTS_DIR, "poll_windows.csv")
    with open(csv_path, "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(["elapsed_s", "total_window_ms"])
        for elapsed, w_ms in rows:
            writer.writerow([f"{elapsed:.6f}", f"{w_ms:.6f}"])
    print(f"\n  Results written to {csv_path}")

    proc.close()
    print("Done.")


if __name__ == "__main__":
    main()
