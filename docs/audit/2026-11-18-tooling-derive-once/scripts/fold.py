"""Fold the 115 T8 re-check rows: each split kind leaves by the user's answer that names it.

Reads rows.tsv (from parse.py, run over this audit folder) and writes the fold table to stdout as markdown.
"""
import sys
from collections import Counter

ANSWER = {
    "doc-vs-code": 'exit 3, "The code wins" (log.md, T6)',
    "memory-vs-other": 'exit 3, "Note is wrong" (log.md, T6)',
    "moved-pointer": 'exit 3, "Fix without asking" (log.md, T6)',
    "code-vs-code": 'exit 3, "Merge, I pick" (log.md, T8 batch 1); pick goes in the fix plan',
    "doc-vs-doc": 'exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan',
    "homeless-rule": 'exit 3, "Give each a home" (log.md, T8 batch 1)',
    "claimed-ruling": 'exit 3, "Drop the claim" (log.md, T8 batch 1)',
}

# Kinds the reports wrote inside the verdict line, which the parser does not read.
INLINE_KINDS = {
    "L-F29-08": "doc-vs-doc",
    "L-F16-08": "doc-vs-code;doc-vs-doc",
    "L-F29-03": "doc-vs-doc",
}

# The main session's own reading, each with its reason. A row here leaves as written, not by kind.
OVERRIDE = {
    "L-F22-24": ("REAL", "doc-vs-doc", None,
                 "R2 said GONE after looking in Hydra; the brief is in ~\\.claude\\tools\\session_cards (log.md, T8 re-check)"),
    "L-F06-16": ("REAL", "doc-vs-doc", None,
                 "R6 said doc-vs-code, but the stall notice is text a hook prints and enforces nothing, so both sides are instructions"),
    "L-F04-07": ("REAL", "ruling-vs-other", "exit 2, question: theme 17, whether any fixed line budget should exist", "no answer covers ruling-vs-other"),
    "L-F07-16": ("REAL", "ruling-vs-other", "exit 2, question: theme 17, whether any fixed line budget should exist", "no answer covers ruling-vs-other"),
    "L-F21-06": ("REAL", "ruling-vs-other", "exit 2, question: theme 17, whether any fixed line budget should exist", "no answer covers ruling-vs-other"),
    "L-F03-20": ("REAL", "homeless-rule;memory-vs-other;ruling-vs-other",
                 "exit 2, question: the render note's scrolling step against the Confirmed line at rulings/ui-and-display.md:27",
                 "no answer covers ruling-vs-other"),
    "L-F14-10": ("REAL", "homeless-rule;memory-vs-other;ruling-vs-other",
                 "exit 2, question: same split as L-F03-20", "no answer covers ruling-vs-other"),
    "L-F26-04": ("REAL", "homeless-rule;memory-vs-other",
                 "exit 2, question: whether status relays skip test_ agents",
                 '"Note is wrong" drops the skip and "Give each a home" keeps it; the two answers point opposite ways'),
    "L-F32-10": ("REAL", "claimed-ruling;memory-vs-other",
                 'claimed-ruling: exit 3, "Drop the claim"; the test_ skip: exit 2, question with L-F26-04',
                 "its memory half is the L-F26-04 skip"),
    "L-F13-08": ("REAL", "doc-vs-code;doc-vs-doc",
                 "exit 2, question: whether gh issue close --comment, which posts text with no deny rule, should pass the publish gate",
                 '"The code wins" keeps an ungated way to post text; "Merge, I pick" takes the safer reading; the two answers point opposite ways'),
    "L-F16-08": ("REAL", "doc-vs-code;doc-vs-doc",
                 "exit 2, question: same split as L-F13-08", "duplicate of L-F13-08"),
    "L-F21-09": ("REAL", "moved-pointer",
                 'D49\'s missing file name: exit 3, "Fix without asking"; the rest: open, sub-items no agent has read',
                 "T7 C14 split the row; R6 read only the D49 piece"),
}

CHECK_NOTE = {
    "L-F27-27": "closure checker: say whether SPEC.md's 0x0313ECB0 and constants.py's c1 answer the same question; they may be different constants",
}

rows = [l.rstrip("\n").split("\t") for l in open(sys.argv[1], encoding="utf-8") if l.strip()]
out, tally = [], Counter()
for rid, rep, base, kinds, _v, _k in rows:
    note = ""
    kinds = INLINE_KINDS.get(rid, kinds)
    exit_ = None
    if rid in OVERRIDE:
        base, kinds, exit_, note = OVERRIDE[rid]
    if exit_ is None:
        if base == "NOT A SPLIT":
            exit_ = "exit 4, not a problem: the report's sentence"
        elif base == "GONE":
            exit_ = "closure: GONE, the split is no longer in the files"
        else:
            ks = [k for k in kinds.split(";") if k]
            missing = [k for k in ks if k not in ANSWER]
            if not ks or missing:
                exit_ = "exit 2, question: no answer covers " + (",".join(missing) or "an unnamed kind")
            else:
                exit_ = "; ".join(f"{k}: {ANSWER[k]}" if len(ks) > 1 else ANSWER[k] for k in ks)
    if rid in CHECK_NOTE:
        note = (note + "; " if note else "") + CHECK_NOTE[rid]
    tally[(base, "open" if "exit 2" in exit_ or "open" in exit_ else exit_.split(",")[0].split(":")[0])] += 1
    out.append(f"| {rid} | {rep} | {base} | {kinds or '-'} | {exit_} | {note} |")

print("| Row | Report | Verdict | Kinds | Exit | Main session's note |")
print("|---|---|---|---|---|---|")
print("\n".join(sorted(out, key=lambda s: [int(x) for x in s.split("|")[1].strip()[3:].split("-")])))
print()
for k, v in sorted(tally.items()):
    print(f"- {k[0]}, {k[1]}: {v}", file=sys.stderr)
