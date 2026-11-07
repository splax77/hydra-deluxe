"""Scores one reviewer of the effort test against its branch's fault key.

Each planted fault comes out fixed, named or missed, and every edit away from a planted line is
a false finding. The key's shape and what the test does with these results are in the plan,
docs/superpowers/plans/2026-11-07-agent-effort-test.md, task T3.

Run it with:
    py -I tools/effort_test_faults.py <fault-key.json> <repo> <reviewer tip> <report.txt>

<repo> is any checkout that holds the reviewer's commits. <reviewer tip> is its last commit or
branch name, and <report.txt> is the text of its report. The script exits 0 once it has scored
the run, whatever the score, and 2 when the inputs cannot be scored.

Three choices the key's format leaves open:

One edit is one hunk of `git diff -U0`, so a run of neighbouring changed lines is one false
finding, however long.

A planted line is any line `git blame` gives to a fault's commit at the key's tip, so a fault
that added several lines is whole, and a later fault that moved its lines changes nothing.

A report names a fault only when one paragraph holds both the file and the line or words, so a
paragraph about one fault cannot name another fault in the same file. The words must be the
key's `planted_text` or `what`, whole. A paraphrase does not match; the script says so on the
fault's line, so the main session can read that paragraph itself.
"""

import argparse
import json
import os
import re
import subprocess
import sys

REQUIRED_FAULT_FIELDS = ("id", "kind", "file", "line", "commit", "planted_text", "what")
HUNK = re.compile(r"@@ -(\d+)(?:,(\d+))? \+\d+(?:,(\d+))? @@")
BLAME_HEADER = re.compile(r"([0-9a-f]{40}) \d+ (\d+)")


class CannotScore(Exception):
    pass


def git(repo, *args):
    r = subprocess.run(["git", "-C", repo, *args], capture_output=True)
    return r.returncode, r.stdout.decode("utf-8", "replace"), r.stderr.decode("utf-8", "replace")


def must_git(repo, *args):
    code, out, err = git(repo, *args)
    if code != 0:
        raise CannotScore("git %s failed: %s" % (" ".join(args), err.strip()))
    return out


def commit_of(repo, ref):
    return must_git(repo, "rev-parse", "--verify", ref + "^{commit}").strip()


def load_key(path):
    with open(path, encoding="utf-8") as f:
        key = json.load(f)
    if not key.get("tip") or not key.get("faults"):
        raise CannotScore("the key has no tip or no faults")
    for fault in key["faults"]:
        empty = [name for name in REQUIRED_FAULT_FIELDS if not fault.get(name)]
        if empty:
            raise CannotScore("fault %s has no %s" % (fault.get("id"), ", ".join(empty)))
    return key


def file_at(repo, commit, path):
    """The file's text at that commit, or None when the commit does not hold it."""
    code, out, _ = git(repo, "show", "%s:%s" % (commit, path))
    return out if code == 0 else None


def planted_lines(repo, tip, faults):
    """Each fault file's line numbers at the tip that came from a fault's commit."""
    commits = {commit_of(repo, fault["commit"]) for fault in faults}
    planted = {}
    for path in {fault["file"] for fault in faults}:
        lines = planted.setdefault(path, set())
        for row in must_git(repo, "blame", "--porcelain", tip, "--", path).split("\n"):
            m = BLAME_HEADER.match(row)
            if m and m.group(1) in commits:
                lines.add(int(m.group(2)))
    return planted


def changed_hunks(repo, old, new):
    """(path, first old line, old line count) for each hunk from old to new.

    A hunk that adds lines and removes none has an old count of 0, and its first old line is
    the line it follows. A binary file counts as one hunk at line 0.
    """
    rows = must_git(repo, "-c", "core.quotePath=false", "diff", "-U0", "--no-renames",
                    "--no-ext-diff", "--no-color", "--src-prefix=a/", "--dst-prefix=b/",
                    old, new).split("\n")
    hunks, path, i = [], None, 0
    while i < len(rows):
        row = rows[i]
        i += 1
        if row.startswith("diff --git a/"):
            # With renames off the header is "a/<path> b/<path>", the same path twice.
            names = row[len("diff --git a/"):]
            path = names[:(len(names) - len(" b/")) // 2]
        elif row.startswith("Binary files "):
            hunks.append((path, 0, 0))
        else:
            m = HUNK.match(row)
            if m:
                old_count = 1 if m.group(2) is None else int(m.group(2))
                new_count = 1 if m.group(3) is None else int(m.group(3))
                hunks.append((path, int(m.group(1)), old_count))
                i += old_count + new_count
    return hunks


def false_findings(hunks, planted):
    false = []
    for path, first, count in hunks:
        touched = range(first, first + count) if count else (first, first + 1)
        if not planted.get(path, set()).intersection(touched):
            false.append((path, first))
    return false


def squash(text):
    return " ".join(text.split()).lower()


def mentions_file(paragraph, path):
    name = re.escape(os.path.basename(path).lower())
    return re.search(r"(?<![\w.-])" + name + r"(?![\w-])", paragraph.replace("\\", "/").lower())


def mentions_line(paragraph, numbers):
    alternatives = "|".join(str(n) for n in numbers)
    return re.search(r"(?<![\w.])L?(?:%s)(?!\w)" % alternatives, paragraph)


def mentions_words(paragraph, fault):
    text = squash(paragraph)
    return squash(fault["planted_text"]) in text or squash(fault["what"]) in text


def score_fault(fault, reviewed_text, tip_text, report):
    """The fault's result, and a note for a miss the main session should read."""
    if reviewed_text is None or fault["planted_text"] not in reviewed_text:
        return "fixed", ""
    numbers = {fault["line"]}
    numbers.update(n for n, row in enumerate(tip_text.split("\n"), 1) if fault["planted_text"] in row)
    with_file = [p for p in re.split(r"\n\s*\n", report.replace("\r\n", "\n"))
                 if mentions_file(p, fault["file"])]
    if any(mentions_line(p, numbers) or mentions_words(p, fault) for p in with_file):
        return "named", ""
    if with_file:
        return "missed", " (the report mentions this file with no line or words match; read it)"
    return "missed", ""


def score(key, repo, reviewer_ref, report):
    tip = commit_of(repo, key["tip"])
    reviewed = commit_of(repo, reviewer_ref)
    if git(repo, "merge-base", "--is-ancestor", tip, reviewed)[0] != 0:
        raise CannotScore("%s does not descend from the key's tip %s" % (reviewer_ref, key["tip"]))
    lines, counts = [], {"fixed": 0, "named": 0, "missed": 0}
    for fault in key["faults"]:
        tip_text = file_at(repo, tip, fault["file"])
        if tip_text is None or fault["planted_text"] not in tip_text:
            raise CannotScore("fault %s: planted_text is not in %s at the key's tip"
                              % (fault["id"], fault["file"]))
        result, note = score_fault(fault, file_at(repo, reviewed, fault["file"]), tip_text, report)
        counts[result] += 1
        lines.append("%s %s: %s line %d%s" % (fault["id"], result, fault["file"], fault["line"], note))
    false = false_findings(changed_hunks(repo, tip, reviewed), planted_lines(repo, tip, key["faults"]))
    lines.extend("false finding: %s line %d" % item for item in false)
    lines.append("%d fixed, %d named, %d missed, %d false findings"
                 % (counts["fixed"], counts["named"], counts["missed"], len(false)))
    lines.append("run: %s" % ("fail" if counts["missed"] else "pass"))
    return lines


def main():
    parser = argparse.ArgumentParser(description="Score one reviewer against a fault key.")
    parser.add_argument("key", help="the branch's fault-key.json")
    parser.add_argument("repo", help="a checkout holding the reviewer's commits")
    parser.add_argument("tip", help="the reviewer's last commit or branch")
    parser.add_argument("report", help="text file holding the reviewer's report")
    args = parser.parse_args()
    try:
        with open(args.report, encoding="utf-8") as f:
            report = f.read()
        lines = score(load_key(args.key), args.repo, args.tip, report)
    except (CannotScore, OSError, ValueError) as e:
        print("cannot score: %s" % e, file=sys.stderr)
        return 2
    for line in lines:
        print(line)
    return 0


if __name__ == "__main__":
    sys.exit(main())
