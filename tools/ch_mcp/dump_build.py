"""Run Il2CppDumper over a Clone Hero game folder into an output folder.

The dumper ends every run by waiting for a key, and with stdin redirected that
wait crashes, so a good run exits non-zero. This script therefore judges a run
by the dumper's three "Done!" lines and its output files, never by the exit
code. tools/ch_mcp/README.md says how to run it.

Run:
    py tools/ch_mcp/dump_build.py --game "C:\\Clone Hero\\1.1 june\\Clone Hero" --out <folder>
"""

from __future__ import annotations

import argparse
import glob
import os
import subprocess
import sys

DEFAULT_DUMPER = r"C:\Users\Patrick\Downloads\Il2CppDumper-net7-v6.7.46\Il2CppDumper.exe"

# The cap on one dumper run, which the user approved on 2026-10-28 as
# question 5 of the feasibility-tests plan (removed; read it with git show
# 444631e6:docs/superpowers/plans/2026-10-28-ch-mcp-feasibility-tests.md).
DUMPER_CAP_S = 600

# The stages the dumper announces, each closed by its own "Done!" line.
STAGES = ("Dumping...", "Generate struct...", "Generate dummy dll...")
DONE_LINE = "Done!"

# The files a good run must leave; the first three are the ones the plan
# requires, and il2cpp.h is reported with them.
REQUIRED_FILES = ("dump.cs", "script.json", "stringliteral.json")
REPORTED_FILES = ("dump.cs", "il2cpp.h", "script.json", "stringliteral.json")


class DumpError(Exception):
    """The dump could not be started, or the run did not count as good."""


def enclosing_repo(path: str) -> str | None:
    """The top folder of the git checkout that holds `path`, or None."""
    folder = os.path.abspath(path)
    while True:
        if os.path.exists(os.path.join(folder, ".git")):
            return folder
        parent = os.path.dirname(folder)
        if parent == folder:
            return None
        folder = parent


def game_inputs(game_dir: str) -> tuple[str, str]:
    """GameAssembly.dll and global-metadata.dat inside a Unity IL2CPP game folder."""
    dll = os.path.join(game_dir, "GameAssembly.dll")
    if not os.path.isfile(dll):
        raise DumpError(f"no GameAssembly.dll in {game_dir}")
    metas = glob.glob(os.path.join(glob.escape(game_dir), "*_Data", "il2cpp_data", "Metadata", "global-metadata.dat"))
    if len(metas) != 1:
        raise DumpError(f"expected one *_Data\\il2cpp_data\\Metadata\\global-metadata.dat in {game_dir}, found {len(metas)}")
    return dll, metas[0]


def missing_stages(stdout: str) -> list[str]:
    """The stages that never printed their "Done!" line, in run order."""
    done: set[str] = set()
    current = None
    for line in stdout.splitlines():
        line = line.strip()
        if line in STAGES:
            current = line
        elif line == DONE_LINE and current is not None:
            done.add(current)
            current = None
    return [s for s in STAGES if s not in done]


def judge(stdout: str, out_dir: str) -> list[str]:
    """Every reason the run does not count as good; empty means good."""
    problems = []
    done_count = sum(1 for line in stdout.splitlines() if line.strip() == DONE_LINE)
    if done_count < len(STAGES):
        problems.append(f"Il2CppDumper printed {done_count} of {len(STAGES)} {DONE_LINE} lines")
    for stage in missing_stages(stdout):
        problems.append(f"stage {stage!r} never printed {DONE_LINE}")
    for name in REQUIRED_FILES:
        if not os.path.isfile(os.path.join(out_dir, name)):
            problems.append(f"{name} is missing from {out_dir}")
    return problems


def run_dump(game_dir: str, out_dir: str, dumper: str = DEFAULT_DUMPER, runner=subprocess.run,
             cap_s: float = DUMPER_CAP_S) -> tuple[dict[str, int], int]:
    """Dump the game into out_dir; return each reported file's size in bytes
    and the dumper's exit code.

    Raises DumpError with the reason when the output folder is in a repo, the
    cap is hit, or the run does not count as good.
    """
    repo = enclosing_repo(out_dir)
    if repo is not None:
        raise DumpError(f"output folder {out_dir} is inside the git checkout {repo}; the dump is too big for a repo")
    dll, meta = game_inputs(game_dir)
    os.makedirs(out_dir, exist_ok=True)
    env = dict(os.environ, DOTNET_ROLL_FORWARD="Major")
    try:
        result = runner(
            [dumper, dll, meta, out_dir],
            stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            encoding="utf-8",
            errors="replace",
            env=env,
            timeout=cap_s,
        )
    except subprocess.TimeoutExpired:
        raise DumpError(f"Il2CppDumper did not finish in {cap_s:g} s; output folder left as is") from None
    stdout = result.stdout or ""
    problems = judge(stdout, out_dir)
    if problems:
        raise DumpError(
            "Il2CppDumper run failed (exit code {}):\n  {}\n--- stdout ---\n{}\n--- stderr ---\n{}".format(
                result.returncode, "\n  ".join(problems), stdout, result.stderr or ""
            )
        )
    sizes = {
        name: os.path.getsize(os.path.join(out_dir, name))
        for name in REPORTED_FILES
        if os.path.isfile(os.path.join(out_dir, name))
    }
    return sizes, result.returncode


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Run Il2CppDumper over a game folder.")
    parser.add_argument("--game", required=True, help="the game folder holding GameAssembly.dll")
    parser.add_argument("--out", required=True, help="output folder, outside any git checkout")
    parser.add_argument("--dumper", default=DEFAULT_DUMPER, help="path to Il2CppDumper.exe")
    args = parser.parse_args(argv)
    try:
        sizes, exit_code = run_dump(args.game, args.out, args.dumper)
    except DumpError as exc:
        print(f"dump_build: {exc}", file=sys.stderr)
        return 1
    print(f"dump good in {os.path.abspath(args.out)}: all {len(STAGES)} stages printed {DONE_LINE} "
          f"(dumper exit code {exit_code}, expected non-zero from its end-of-run key wait)")
    for name, size in sizes.items():
        print(f"  {name}: {size} bytes")
    return 0


if __name__ == "__main__":
    sys.exit(main())
