"""Lists the tool calls in one wave of effort-test runs that may have read the finished solution.

The plan's "Global constraints" says what counts. The plan was removed; read it with
git show 6804353f:docs/superpowers/plans/2026-11-07-agent-effort-test.md. This script only lists. The main session
reads each listed call and rules on it, so the script prints no verdict and always exits 0 once
it has read every transcript. It exits 2 when it cannot read one, because a skipped transcript
would look like a clean run.

Run it with: py -I tools/effort_test_taint.py <wave.json>

The wave file is a JSON list with one object per run:

    {"transcript": "<agent-*.jsonl>", "worktree": "<the run's own worktree>",
     "branch": "<the run's own branch>", "key": "<the tip a reviewer was handed>",
     "scratch": "<a reviewer's isolation copy>"}

Only "transcript" is required. A field left out allows less, so more calls are listed.

The script reads what a call asked for, not what it did, so two things go unseen. It skips the
text a call writes (the bodies of Write and Edit, the final report) and the plain-words
description of a shell call, because none of them reads anything. It also cannot see a command that reached the main checkout
without naming it, such as a bare `git log` run from a folder that happens to be that checkout.
"""

import argparse
import json
import os
import re
import sys

MAIN_CHECKOUT = "C:/Users/Patrick/Downloads/Hydra/hydra-test"

# The four task merges of the test, cut to the length the plan names.
MERGE_PREFIXES = ("32973b96", "735d2daa", "8ad1fc65", "be54d7bc")

UNSCANNED_FIELDS = {"description", "content", "old_string", "new_string", "message"}

MAIN_REF = re.compile(
    r"(?<![\w-])(?:refs/remotes/origin/|refs/heads/|origin/)?main(?![\w/-]|\.\w)")
CLAUDE_BRANCH = re.compile(r"claude/[\w*-]+(?:[./][\w*-]+)*")
ALL_REFS = re.compile(r"\bgit\b[^;&|\n]*\s--all(?![\w-])")
BRANCH_ALL = re.compile(r"\bgit\b[^;&|\n]*\sbranch\b[^;&|\n]*\s-[a-zA-Z]*a[a-zA-Z]*(?![\w-])")
# Two climbs reach the worktrees folder from a worktree's root, with or without a slash after
# them. One climb is listed only when it names a worktree of the test (eff-..., effort-...).
CLIMB = re.compile(r"(?<![\w.])\.\./\.\.(?![\w.])|\.\./eff")
HOME_FOLDER = "c:/users/patrick"
HOME_SPELLINGS = re.compile(r"(?<![\w:./-])(?:~|\$home|\$\{home\}|\$env:userprofile|%userprofile%)(?=/|\s|$|['\"])")


def fold_path(text):
    """One spelling for the path forms a tool call can use: Windows, Git Bash, home, any case."""
    folded = re.sub(r"/{2,}", "/", text.replace("\\", "/").lower())
    folded = HOME_SPELLINGS.sub(HOME_FOLDER, folded)
    return re.sub(r"(?<![\w:./-])/c/users/", "c:/users/", folded)


def scanned_strings(value, field=None):
    """Every string in a tool call's input that the call reads or runs."""
    if field in UNSCANNED_FIELDS:
        return []
    if isinstance(value, str):
        return [value]
    if isinstance(value, dict):
        return [s for k, v in value.items() for s in scanned_strings(v, k)]
    if isinstance(value, list):
        return [s for v in value for s in scanned_strings(v)]
    return []


def is_inside(folded_tail, folder):
    """True when a path that starts folded_tail is the folder or something in it."""
    if not folded_tail.startswith(folder):
        return False
    following = folded_tail[len(folder):len(folder) + 1]
    return not re.match(r"[\w.-]", following)


def stray_paths(text, own_folders):
    root = fold_path(MAIN_CHECKOUT)
    folded = fold_path(text)
    return [m.start() for m in re.finditer(re.escape(root) + r"(?![\w.-])", folded)
            if not any(is_inside(folded[m.start():], own) for own in own_folders)]


def other_branches(text, own_branch):
    found = []
    for m in CLAUDE_BRANCH.finditer(text):
        before = text[:m.start()]
        is_dot_claude_folder = before.endswith(".") and not before.endswith("..")
        if is_dot_claude_folder or re.search(r"[\w\\-]$", before):
            continue
        if m.group() != own_branch:
            found.append(m.group())
    return found


def reasons_for(text, run):
    """Why one string of a call is listed, in plain words; empty when it is not."""
    reasons = []
    unranged = text
    if run.get("key"):
        unranged = re.sub(r"(?<![\w/-])main\.\.\." + re.escape(run["key"]) + r"(?!\w)", "", text)
    for m in MAIN_REF.finditer(unranged):
        reasons.append("names " + m.group())
    for name in other_branches(text, run.get("branch")):
        reasons.append("names branch " + name)
    own_folders = [fold_path(run[f]).rstrip("/") for f in ("worktree", "scratch") if run.get(f)]
    if stray_paths(text, own_folders):
        reasons.append("path outside the run's worktree")
    if CLIMB.search(fold_path(text)):
        reasons.append("relative path climbs out of the worktree")
    if ALL_REFS.search(text):
        reasons.append("git --all")
    if BRANCH_ALL.search(text):
        reasons.append("git branch -a")
    for prefix in MERGE_PREFIXES:
        if prefix in text.lower():
            reasons.append("names merge " + prefix)
    return reasons


def tool_calls(transcript):
    """Yields (agent id, tool name, input) for each tool call, once each."""
    fallback_id = os.path.basename(transcript).removesuffix(".jsonl").removeprefix("agent-")
    seen = set()
    with open(transcript, encoding="utf-8") as f:
        for line in f:
            if not line.strip():
                continue
            record = json.loads(line)
            content = (record.get("message") or {}).get("content")
            if record.get("type") != "assistant" or not isinstance(content, list):
                continue
            for block in content:
                if not isinstance(block, dict) or block.get("type") != "tool_use":
                    continue
                if block.get("id"):
                    if block["id"] in seen:
                        continue
                    seen.add(block["id"])
                yield record.get("agentId") or fallback_id, block.get("name"), block.get("input")


def listed_calls(run):
    listed = []
    for agent, tool, tool_input in tool_calls(run["transcript"]):
        strings = scanned_strings(tool_input)
        reasons = [r for s in strings for r in reasons_for(s, run)]
        if reasons:
            call = " ; ".join(" ".join(s.split()) for s in strings)
            listed.append("%s | %s | %s | %s" % (agent, tool, ", ".join(dict.fromkeys(reasons)), call))
    return listed


def main():
    parser = argparse.ArgumentParser(description="List tool calls for the main session to rule on.")
    parser.add_argument("wave", help="JSON file listing the wave's runs (see the file's top comment)")
    args = parser.parse_args()
    try:
        with open(args.wave, encoding="utf-8") as f:
            runs = json.load(f)
        lines = [line for run in runs for line in listed_calls(run)]
    except (OSError, ValueError, KeyError, TypeError, AttributeError) as e:
        print("cannot read the wave: %s: %s" % (type(e).__name__, e), file=sys.stderr)
        return 2
    for line in lines:
        print(line)
    print("%d calls listed from %d transcripts" % (len(lines), len(runs)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
