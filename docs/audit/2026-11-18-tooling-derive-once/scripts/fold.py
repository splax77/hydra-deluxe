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

# After the T8 closure check (reports/TOOL-AUDIT-T8-K1.md to K6.md, R7.md): rows a checker
# reopened, and the main session's refold of each, read against the files. Applied last.
BUILD_Q = "exit 2, question: whether a wave's build count needs the user's yes (the one-build-per-wave note against agent-rules.md rule 3)"
AFTER_CHECK = {
    "L-F09-12": ("REAL", "doc-vs-code;code-vs-code",
                 'doc-vs-code: exit 3, "The code wins"; code-vs-code: exit 3, "Merge, I pick"; pick goes in the fix plan',
                 "K2 REOPEN: tools/bench.cpp:986 also holds the library path, which R6's .py/.ps1 search missed; refold checked in K7"),
    "L-F13-17": ("REAL", "doc-vs-code;doc-vs-doc", "exit 2, question: same split as L-F13-08 (its label-edit half is not a split)",
                 "K2 REOPEN: its close-comment half is the L-F13-08 gap"),
    "L-F18-12": ("REAL", "doc-vs-doc", 'exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan',
                 "K3 REOPEN: the code side is check_5.cpp:30 kWhy, text that enforces nothing; the split is derive-once-review.md:21 against :38 and :49"),
    "L-F25-19": ("REAL", "code-vs-code", 'exit 3, "Merge, I pick" (log.md, T8 batch 1); pick goes in the fix plan',
                 "K4 REOPEN: dump_facts.py:381 DEFAULT_BUILD and test_docs_match_code.cpp:735 kBuild11 both say which build an unheaded span means; equal today, no test holds them equal"),
    "L-F27-27": ("REAL", "doc-vs-code",
                 "exit 2, question: rename constants.py's swapped formula tables to match the disassembly",
                 "K5 REOPEN: docs/clone-hero/03-timing.md:355 proves constants.py's normal and precision tables are swapped, so the code is the wrong side; SPEC.md's 0x0313ECB0 is in no 6142 listing"),
    "L-F29-01": ("REAL", "doc-vs-doc;memory-vs-other",
                 'doc-vs-doc: exit 3, "Merge, I pick"; memory-vs-other: ' + BUILD_Q,
                 'K5 REOPEN: "Note is wrong" would drop the note\'s build-count rule, which no doc holds; agent-rules.md:9 says compiling inside approved work needs no separate yes'),
    "L-F29-18": ("REAL", "doc-vs-doc;memory-vs-other",
                 'doc-vs-doc: exit 3, "Merge, I pick"; memory-vs-other: ' + BUILD_Q, "K5 REOPEN: as L-F29-01"),
    "L-F32-01": ("REAL", "doc-vs-doc;memory-vs-other",
                 'doc-vs-doc: exit 3, "Merge, I pick"; memory-vs-other: ' + BUILD_Q, "K6 REOPEN: as L-F29-01"),
    "L-F26-04": ("REAL", "homeless-rule;memory-vs-other", 'exit 3, "Note is wrong" (log.md, T6)',
                 "refold: the test_ skip filters nothing seen, since none of the 860 files in gates/state/status is named test_ (2026-11-19); checked in K7"),
    "L-F32-10": ("REAL", "claimed-ruling;memory-vs-other",
                 'claimed-ruling: exit 3, "Drop the claim" (K6 PASS); the test_ skip: exit 3, "Note is wrong", as L-F26-04',
                 "the skip half refolds with L-F26-04; checked in K7"),
    "L-F21-09": ("REAL", "moved-pointer",
                 'D49\'s missing file name: exit 3, "Fix without asking" (K4 PASS; deleted in db87b55c, so the line gets "(removed; git show ...)"); the other six pieces: WATCHDOG GONE, five exit 4 (reports/TOOL-AUDIT-T8-R7.md)',
                 "R7's six verdicts are checked in K7"),
    "L-F26-13": ("REAL", "memory-vs-other;homeless-rule",
                 'the "what a constant means" half: exit 3, "Note is wrong"; the "no history in comments" half: exit 3, "Give each a home"',
                 "K4 PASS with note: a Confirmed ruling backs the no-history half and no agent page or hook states it, so that half gets a home rather than being pruned"),
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
    if rid in AFTER_CHECK:
        base, kinds, exit_, note = AFTER_CHECK[rid]
    elif rid in CHECK_NOTE:
        note = (note + "; " if note else "") + CHECK_NOTE[rid]
    tally[(base, "open" if "exit 2" in exit_ or "open" in exit_ else exit_.split(",")[0].split(":")[0])] += 1
    out.append(f"| {rid} | {rep} | {base} | {kinds or '-'} | {exit_} | {note} |")

print("| Row | Report | Verdict | Kinds | Exit | Main session's note |")
print("|---|---|---|---|---|---|")
print("\n".join(sorted(out, key=lambda s: [int(x) for x in s.split("|")[1].strip()[3:].split("-")])))
print()
for k, v in sorted(tally.items()):
    print(f"- {k[0]}, {k[1]}: {v}", file=sys.stderr)
