"""Milestone 1: verify the address pipeline.

Opens the running Clone Hero, reads two known constants from .rdata
(the normal-mode back and front window values), and prints whether
they match what Ghidra saw. If they do, every address in constants.py
is trustworthy. If they don't, the game updated and the addresses
are stale.

No debugger needed -- just ReadProcessMemory.

    python -m tools.ch_probe.experiments.milestone1
"""

from __future__ import annotations

import os
import sys

_REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants
from tools.ch_probe.engine import EngineModel
from tools.ch_probe.process import open_process


def main() -> None:
    print(f"Looking for {constants.PROCESS_NAME}...")
    proc = open_process()
    print(f"  PID {proc.pid}, GameAssembly.dll at {proc.module_base:#x}")

    back = proc.read_const_double(constants.RVA_CONST_NORMAL_BACK)
    front = proc.read_const_double(constants.RVA_CONST_NORMAL_FRONT)
    print(f"  Normal back  = {back:.6f} s  ({constants.s_to_ms(back):.1f} ms, expect {constants.EXPECT_NORMAL_BACK_S} s)")
    print(f"  Normal front = {front:.6f} s  ({constants.s_to_ms(front):.1f} ms, expect {constants.EXPECT_NORMAL_FRONT_S} s)")

    try:
        proc.verify_targets()
        print("PASS: address pipeline verified.")
    except Exception as e:
        print(f"FAIL: {e}")
        proc.close()
        return

    # Bonus: every constant EngineModel reads, so we can see the real numbers.
    print("\n  Every .rdata constant (EngineModel.constants):")
    for key, val in sorted(EngineModel(proc).constants().items()):
        print(f"    {key} = {val:.10f}")

    proc.close()
    print("\nDone. All constants above are live-read from the running game.")


if __name__ == "__main__":
    main()
