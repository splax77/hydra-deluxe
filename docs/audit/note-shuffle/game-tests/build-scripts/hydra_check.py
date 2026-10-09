"""Load each built song in Hydra (hydra_replay score, no Star Power) and compare Hydra's chord
list with what build.py meant to write, under Pro Drums on and off.

usage: py -I hydra_check.py <out dir>
"""
import json, os, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ns_lib import *
import build

EXE = REPO + "/build-cpp/Release/hydra_replay.exe"
OUT = sys.argv[1]
MASK = {("Kick", False): K, ("Red", False): R, ("Yellow", False): YT, ("Blue", False): BT,
        ("Green", False): GT, ("Yellow", True): YC, ("Blue", True): BC, ("Green", True): GC}


def hydra_chords(path, pro):
    r = subprocess.run([EXE, "score", "--chart", path, "--prodrums", "1" if pro else "0",
                        "--bass2x", "0"], capture_output=True, text=True)
    if r.returncode != 0:
        return None, r.stderr.strip()
    j = json.loads(r.stdout)
    chords = [(c["tick"], frozenset(MASK[(n["color"], n["cymbal"])] for n in c["notes"])) for c in j["chords"]]
    extra = {k: v for k, v in j.items() if k not in ("chords", "sections", "chart")}
    return chords, extra


def check(songs):
    bad = 0
    for name, fmt, resol, chords, test in songs:
        path = os.path.join(OUT, "songs", name, "notes." + fmt)
        for pro in (True, False):
            want = [(t, frozenset(m if pro else FOLD.get(m, m) for m in c)) for t, c in chords]
            got, extra = hydra_chords(path, pro)
            ok = got == want
            bad += not ok
            short = {k: extra[k] for k in extra if not isinstance(extra[k], (list, dict))} if isinstance(extra, dict) else extra
            print(f"{'OK ' if ok else 'BAD'} {name} pro={'on' if pro else 'off'}: {len(got or [])} chords; {short}")
            if not ok:
                print("   want", want)
                print("   got ", got)
    return bad


if __name__ == "__main__":
    sys.exit(check(build.SONG_LIST))
