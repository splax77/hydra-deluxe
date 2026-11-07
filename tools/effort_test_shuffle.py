"""Builds the blind scorer's bundle for one task of the effort test.

It gives the task's runs the labels R1 to R6 in a random order, writes each run's diff and test
output under its label, and records which label is which run in a key file. The scorer reads the
bundle and never the key, so the key must sit outside the bundle folder.

Run it with:
    py -I tools/effort_test_shuffle.py --task <task> --repo <repo> --base <sha>
        --key <key.json> --out <empty bundle folder>
        --run <branch> <test output file>      (once per run)

The bundle holds R1.diff and R1.test.txt, and so on. A run's branch and worktree carry its
number, and the launch order alternates levels, so a number left in a test's output would tell
the scorer the level. Every run's name is therefore replaced by its label in every bundle file.

The script exits 0 when the bundle and key are written, and 2 with nothing written when it
cannot do the job. It refuses a key file that already exists, because a second shuffle would
leave two orders for one set of scores.
"""

import argparse
import json
import os
import random
import re
import subprocess
import sys

RUNS_PER_TASK = 6


class CannotShuffle(Exception):
    pass


def git(repo, *args):
    r = subprocess.run(["git", "-C", repo, *args], capture_output=True)
    if r.returncode != 0:
        raise CannotShuffle("git %s failed: %s"
                            % (" ".join(args), r.stderr.decode("utf-8", "replace").strip()))
    return r.stdout.decode("utf-8", "replace")


def labelled(branches, rng):
    """A dict of R1, R2 and so on to branch names, in the order rng gave."""
    order = list(branches)
    rng.shuffle(order)
    return {"R%d" % (i + 1): branch for i, branch in enumerate(order)}


def hide_run_names(text, labels):
    for label, branch in labels.items():
        short = branch.split("/", 1)[-1]
        names = "|".join(re.escape(name) for name in (branch, short))
        text = re.sub(r"(?<![\w-])(?:%s)(?![\w-])" % names, label, text)
    return text


def check_inputs(args):
    if len(args.run) != RUNS_PER_TASK:
        raise CannotShuffle("got %d runs, need %d" % (len(args.run), RUNS_PER_TASK))
    if len({branch for branch, _ in args.run}) != len(args.run):
        raise CannotShuffle("a branch is named twice")
    if os.path.exists(args.key):
        raise CannotShuffle("the key file already exists: %s" % args.key)
    out = os.path.normcase(os.path.abspath(args.out))
    key_folder = os.path.normcase(os.path.dirname(os.path.abspath(args.key)))
    if key_folder == out or key_folder.startswith(out + os.sep):
        raise CannotShuffle("the key file must sit outside the bundle folder")
    if os.path.isdir(out) and os.listdir(out):
        raise CannotShuffle("the bundle folder is not empty: %s" % args.out)


def build(args):
    check_inputs(args)
    labels = labelled([branch for branch, _ in args.run], random.SystemRandom())
    outputs = dict(args.run)
    files = {}
    for label, branch in labels.items():
        diff = git(args.repo, "diff", "--no-color", "--no-ext-diff", args.base, branch)
        with open(outputs[branch], encoding="utf-8", errors="replace") as f:
            test_output = f.read()
        files[label + ".diff"] = hide_run_names(diff, labels)
        files[label + ".test.txt"] = hide_run_names(test_output, labels)
    os.makedirs(args.out, exist_ok=True)
    for name, text in files.items():
        with open(os.path.join(args.out, name), "w", encoding="utf-8", newline="") as f:
            f.write(text)
    with open(args.key, "w", encoding="utf-8") as f:
        json.dump({"task": args.task, "base": args.base, "runs": labels}, f, indent=2)
        f.write("\n")


def main():
    parser = argparse.ArgumentParser(description="Label six runs and build the scorer's bundle.")
    parser.add_argument("--task", required=True, help="the task's name")
    parser.add_argument("--repo", required=True, help="a checkout holding the runs' branches")
    parser.add_argument("--base", required=True, help="the commit the runs started from")
    parser.add_argument("--key", required=True, help="the key file to write, outside the bundle")
    parser.add_argument("--out", required=True, help="the bundle folder, empty or not yet made")
    parser.add_argument("--run", nargs=2, action="append", required=True,
                        metavar=("BRANCH", "TEST_OUTPUT"), help="one run; give it once per run")
    args = parser.parse_args()
    try:
        build(args)
    except (CannotShuffle, OSError) as e:
        print("cannot shuffle: %s" % e, file=sys.stderr)
        return 2
    print("wrote %d labelled runs to %s and the key to %s" % (len(args.run), args.out, args.key))
    return 0


if __name__ == "__main__":
    sys.exit(main())
