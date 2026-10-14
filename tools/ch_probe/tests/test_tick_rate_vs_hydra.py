"""The probe's tick-to-ms rule agrees with Hydra's own.

probe_chart.ticks_to_ms is a Python copy of the rate Hydra's MsIndex uses
(src/core/timing.cpp). This test writes a one-tempo probe chart, asks a built
hydra_replay.exe for each chord's ms, and checks the two agree. When no
hydra_replay.exe is built the test is skipped and says so, because the
Python tests run without a C++ build.

Written to run under pytest (`python -m pytest <path> -q`) or plain unittest.
"""

from __future__ import annotations

import json
import os
import subprocess
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.probe_chart import chart_text, ticks_to_ms  # noqa: E402

_REPLAY_EXE = os.path.join(_REPO_ROOT, "build-cpp", "Release", "hydra_replay.exe")

# A tempo and resolution that make the rate a fraction, so a rounding
# difference between the two sides would show.
_RESOLUTION = 192
_BPM = 137.5
_NOTE_TICKS = [768, 1000, 1536, 2111, 3072]
_RED_PAD = 1

# How long one hydra_replay score run may take on a five-note chart before
# the test gives up and says so.
_RUN_TIMEOUT_S = 60


@unittest.skipUnless(os.path.isfile(_REPLAY_EXE),
                     f"no hydra_replay.exe at {_REPLAY_EXE}; build it with "
                     "build_cpp.ps1 -Target hydra_replay to run this check")
class TestTickRateMatchesHydra(unittest.TestCase):
    def test_each_chord_ms_matches_ticks_to_ms(self):
        text = chart_text("tick rate check", _NOTE_TICKS, resolution=_RESOLUTION,
                          bpm=_BPM, note=_RED_PAD, music_stream=None)
        with tempfile.TemporaryDirectory() as d:
            chart = os.path.join(d, "notes.chart")
            with open(chart, "w", encoding="utf-8", newline="\n") as f:
                f.write(text)
            out = os.path.join(d, "score.json")
            run = subprocess.run([_REPLAY_EXE, "score", "--chart", chart, "--out", out],
                                 capture_output=True, text=True, timeout=_RUN_TIMEOUT_S)
            self.assertEqual(run.returncode, 0, f"hydra_replay failed: {run.stderr}")
            with open(out, encoding="utf-8") as f:
                chords = json.load(f)["chords"]

        self.assertEqual([c["tick"] for c in chords], _NOTE_TICKS)
        for c in chords:
            self.assertAlmostEqual(c["ms"], ticks_to_ms(c["tick"], _RESOLUTION, _BPM),
                                   delta=1e-6, msg=f"tick {c['tick']}")


if __name__ == "__main__":
    unittest.main()
