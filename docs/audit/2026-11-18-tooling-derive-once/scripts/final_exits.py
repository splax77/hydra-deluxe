"""Give every ledger row of this audit one final exit, from the records alone.

Run from anywhere: py -I final_exits.py <the audit folder>. It reads files and prints markdown;
it writes nothing.

The order, as the T9 brief (briefs/T9-record.md, task T9-S) sets it:
  1. exits.md: each row's T6 verdict, category and exit.
  2. T7's closure verdicts (reports/TOOL-AUDIT-T7-C*.md): PASS keeps the T6 exit, GONE closes the row,
     REOPEN makes it a question (those 19 rows were all re-checked in T8, so step 4 decides them).
  3. The user's answers by theme, for the question rows T8 did not re-check (themes 1 to 10 and 16 of
     reports/TOOL-AUDIT-T8-G1.md). The answers are in log.md; THEME_ANSWERS names the section of each.
  4. fold-T8.md for its 115 rows, which win over everything above.
The ledger files (ledger/TOOL-AUDIT-T5-L*.md) are read too, so a ledger row that exits.md lost shows as a gap.
"""
import glob
import os
import re
import sys
from collections import Counter, defaultdict

D = sys.argv[1]


def read(rel):
    return open(os.path.join(D, rel), encoding="utf-8").read()


# ---- the ledger: every row id and its kinds -------------------------------------------------------
ledger = {}  # id -> (kinds listed before any parenthesis, ledger file)
for f in sorted(glob.glob(os.path.join(D, "ledger", "TOOL-AUDIT-T5-L*.md"))):
    for sec in re.split(r"(?m)^### ", open(f, encoding="utf-8").read())[1:]:
        m = re.match(r"(L-F\d+-\d+)", sec)
        if not m:
            continue
        k = re.search(r"(?m)^- \*\*Kind:\*\*(.*)", sec)
        kinds = sorted(set(re.findall(r"K\d", k.group(1).split("(")[0]))) if k else []
        ledger[m.group(1)] = (kinds, os.path.basename(f))

# ---- exits.md: T6 ------------------------------------------------------------------------------------
t6 = {}  # id -> (verdict, category, exit text)
for line in read("exits.md").splitlines():
    c = [x.strip() for x in line.split("|")]
    if len(c) > 5 and re.fullmatch(r"L-F\d+-\d+", c[1]):
        t6[c[1]] = (c[2], c[3], c[4])

# ---- T7 closure verdicts ---------------------------------------------------------------------------
t7 = defaultdict(set)
for f in sorted(glob.glob(os.path.join(D, "reports", "TOOL-AUDIT-T7-C*.md"))):
    t = open(f, encoding="utf-8").read()
    for sec in re.split(r"(?m)^#{2,4} ", t)[1:]:
        ids = re.findall(r"L-F\d+-\d+", sec.splitlines()[0])
        m = re.search(r"\b(PASS|REOPEN|GONE)\b", sec)
        for i in ids:
            t7[i].add(m.group(1) if m else "?")
    for line in t.splitlines():
        if line.startswith("|"):
            ids = re.findall(r"L-F\d+-\d+", line)
            m = re.search(r"\b(PASS|REOPEN|GONE)\b", line)
            if ids and m:
                t7[ids[0]].add(m.group(1))

# ---- G1 themes -----------------------------------------------------------------------------------------
theme = {}
for sec in re.split(r"(?m)^## ", read("reports/TOOL-AUDIT-T8-G1.md"))[1:]:
    n = re.match(r"(\d+)\.", sec)
    ids = re.search(r"(?m)^Ids: \d+ ids: (.*)", sec)
    if n and ids:
        for i in ids.group(1).split(","):
            theme["L-" + i.strip()] = int(n.group(1))

# ---- fold-T8.md -----------------------------------------------------------------------------------
fold = {}  # id -> (verdict, exit text)
for line in read("fold-T8.md").splitlines():
    c = [x.strip() for x in line.split("|")]
    if len(c) > 6 and re.fullmatch(r"L-F\d+-\d+", c[1]):
        fold[c[1]] = (c[3], c[5])

# ---- the answers ------------------------------------------------------------------------------------
# Each answer: the label the summary groups by, the log.md section it is in, and the fix it asks for.
# "done" means log.md records the fix as made already.
ANSWERS = {
    "code": ('"The code wins"', "log.md, T6", "the doc is fixed to point at the code"),
    "note": ('"Note is wrong"', "log.md, T6", "the memory note is fixed or pruned"),
    "pointer": ('"Fix without asking"', "log.md, T6", "the pointer is moved to where the thing is now"),
    "claim": ('"Drop the claim"', "log.md, T8 batch 1", "the claim that the user ruled is dropped"),
    "merge": ('"Merge, I pick"', "log.md, T8 batches 1 and 3", "the pick is listed in the fix plan for the user's yes"),
    "home": ('"Give each a home"', "log.md, T8 batch 1", "the rule is given a home in agent-rules.md or the preamble"),
    "minutes": ('"Minutes (line 34)"', "log.md, T8 batch 1", "review-and-hooks.md line 33 is retired and the copies follow line 34"),
    "warning": ('"Finish, report the rest"', "log.md, T8 batch 2", "the warning texts are made to say: finish the item in hand, commit, list the rest"),
    "confirmed": ('"Confirmed only"', "log.md, T8 batch 2", "the reviewer and premise-map pages are fixed to say a card or chat quote closes nothing"),
    "trust": ('"Trust both"', "log.md, T8 batch 2", "the delete rule accepts both worktree folders"),
    "budget": ("the Confirmed line review-and-hooks.md:11", "log.md, T8 batch 4", "the \"user's hard budget\" comment is dropped"),
    "count": ('"Yes, state the count"', "log.md, T8 batch 5", "agent-rules.md rule 3 says the build count needs a yes, and the one-build-per-wave rule gets its home there"),
    "rename": ("\"if no value changes then why is this even a question?\"", "log.md, T8 batch 4", "constants.py's two formula tables are renamed to match the disassembly"),
    "scroll": ('"delete that memory note. search function exists for a reason."', "log.md, T8 batch 4", "done: the note was deleted the same day"),
    "issue": ('"Fix the doc line"', "log.md, T8 batch 4", "done: merge 6acfbed4"),
    "lib4": ("the library-run check (theme 4, runs half)", "log.md, T8 batch 2", "nothing: the flag is the only way to start such a run"),
}
DONE = {"scroll", "issue"}

# Themes 1 to 10 and 16, the question rows T8 did not re-check. Each gives (exit, answer) pairs.
THEME_ANSWERS = {
    1: [("exit 3", "claim")],
    2: [("exit 3", "home")],
    3: [("exit 3", "home")],
    6: [("exit 3", "merge")],
    7: [("exit 3", "merge")],
    8: [("exit 3", "merge")],
    9: [("exit 3", "merge")],
    10: [("exit 3", "confirmed")],
}
# Themes whose answer log.md gives in halves. Each row is placed by the half its Sides column in
# exits.md names; the comment gives the words in that column that place it.
ROW_ANSWERS = {
    # Theme 4. Runs half: exit 4 (log.md, batch 2). Phrase half ("accepts any case", "exact phrase"):
    # doc against code, "The code wins". A memory line "No hook enforces it yet" against percall.py is a
    # memory note against code, "Note is wrong" (log.md, T6).
    "L-F12-10": [("exit 4", "lib4")],                                        # flag vs "any run"
    "L-F13-22": [("exit 4", "lib4"), ("exit 3", "code")],                    # "wants the exact phrase"
    "L-F17-01": [("exit 4", "lib4"), ("exit 3", "code")],                    # "accepts any case"
    "L-F18-01": [("exit 4", "lib4"), ("exit 3", "code"), ("exit 3", "note")],  # "tolerant"; memory line
    "L-F20-02": [("exit 4", "lib4"), ("exit 3", "note")],                    # memory line vs percall.py
    # Theme 5. Worktree folders: "Trust both". Which deletes are asked or refused is two code homes
    # (settings.fragment.json against delete_rule.py), the "code half of 5": "Merge, I pick".
    "L-F07-11": [("exit 3", "trust")],
    "L-F10-11": [("exit 3", "trust")],
    "L-F13-05": [("exit 3", "trust")],
    "L-F33-02": [("exit 3", "trust")],
    "L-F12-07": [("exit 3", "merge")],
    "L-F13-04": [("exit 3", "merge"), ("exit 3", "code")],                   # also brief-preamble.md:43
    "L-F16-03": [("exit 3", "merge")],
    # Theme 16. Status by minutes or calls: "Minutes (line 34)". At the warning: "Finish, report the rest".
    "L-F01-01": [("exit 3", "minutes")],
    "L-F01-08": [("exit 3", "minutes")],
    "L-F07-08": [("exit 3", "minutes")],
    "L-F01-02": [("exit 3", "warning")],
    # The other four theme 16 rows are named by neither answer; the main session folded them under
    # earlier answers in late-folds.md.
    "L-F01-04": [("exit 3", "merge")],
    "L-F04-05": [("exit 3", "code")],
    "L-F01-09": [("exit 4", None)],
    "L-F28-02": [("exit 3", "merge")],
}
# Rows no stage gave an exit, folded in late-folds.md. L-F04-14 never reached exits.md.
LATE = {"L-F04-14": [("exit 3", "merge")]}

T6_ANSWER = {"A": "code", "B": "note", "C": "pointer"}

FOLD_TOKENS = [  # words in fold-T8.md's Exit column, and the answer they name
    ('"The code wins"', "code"), ('"Note is wrong"', "note"), ('"Fix without asking"', "pointer"),
    ('"Merge, I pick"', "merge"), ('"Drop the claim"', "claim"), ('"Give each a home"', "home"),
    ('"Yes, state the count"', "count"), ("review-and-hooks.md:11", "budget"),
    ("delete that memory note", "scroll"), ("Fix pending", "rename"),
]


def fold_exits(i, seen=()):
    text = fold[i][1]
    m = re.search(r"\bas (L-F\d+-\d+)", text)
    if m and m.group(1) not in seen and not re.search(r'"', text):
        return fold_exits(m.group(1), seen + (i,))
    out = []
    if text.startswith("exit 1"):
        return [("exit 1", "issue")]
    if "GONE" in text and "exit 3" not in text:
        return [("GONE", None)]
    for tok, a in FOLD_TOKENS:
        if tok in text:
            out.append(("Fix pending" if a == "rename" else "exit 3", a))
    if not out and "exit 4" in text:
        out = [("exit 4", None)]
    if "exit 4" in text and out and out[0][0] != "exit 4":
        out.append(("exit 4", None))
    return out


ORDER = ["still open", "exit 2", "Fix pending", "exit 3", "exit 1", "exit 4", "GONE", "refuted"]

final, parts, gaps, why = {}, {}, [], {}
for i in sorted(set(ledger) | set(t6)):
    if i not in t6:
        if i in LATE:
            parts[i] = LATE[i]
            final[i] = min((e for e, _ in LATE[i]), key=ORDER.index)
            why[i] = "late-folds.md"
            continue
        gaps.append((i, "in the ledger (%s) but not in exits.md or sorting/, so it never got a T6 exit" % ledger[i][1]))
        continue
    verdict, cat, _ = t6[i]
    p = []
    if i in fold:
        p = fold_exits(i)
        why[i] = "fold-T8.md"
    elif cat == "R":
        p = [("refuted", None)]
        why[i] = "exits.md"
    elif cat in "ABCP":
        v = t7.get(i, set())
        if v == {"PASS"}:
            p = [("Fix pending", "pending")] if cat == "P" else [("exit 3", T6_ANSWER[cat])]
            why[i] = "exits.md + T7 PASS"
        elif v == {"GONE"}:
            p = [("GONE", None)]
            why[i] = "T7 GONE"
        else:
            gaps.append((i, "T7 verdicts %s and not in fold-T8.md" % sorted(v)))
            continue
    elif cat == "Q":
        th = theme.get(i)
        if i in ROW_ANSWERS:
            p = ROW_ANSWERS[i]
        elif th in THEME_ANSWERS:
            p = THEME_ANSWERS[th]
        elif th == 16:
            p = [("exit 2", None)]
        else:
            gaps.append((i, "question row, theme %s, no answer and not in fold-T8.md" % th))
            continue
        why[i] = "theme %s" % th
    if verdict == "NOT VERIFIED" and i not in fold:
        p = [("still open", None)] + p
    if not p:
        gaps.append((i, "no exit found in its record (%s)" % why.get(i)))
        continue
    parts[i] = p
    final[i] = min((e for e, _ in p), key=ORDER.index)

# ---- output --------------------------------------------------------------------------------------
print("## Counts\n")
print("Ledger rows (ledger/): %d. Rows in exits.md: %d.\n" % (len(ledger), len(t6)))
print("By verdict (exits.md Verdict column): " + ", ".join("%s %d" % kv for kv in Counter(v for v, _, _ in t6.values()).most_common()) + ".\n")
kc = Counter(k for ks, _ in ledger.values() for k in ks)
print("By kind (ledger Kind lines; a row with several kinds counts once under each): " + ", ".join("%s %d" % (k, kc[k]) for k in sorted(kc)) + ".\n")
print("T6 categories (exits.md): " + ", ".join("%s %d" % kv for kv in sorted(Counter(c for _, c, _ in t6.values()).items())) + ".\n")
t7c = Counter(min(v) if len(v) == 1 else "mixed" for v in t7.values())
print("T7 verdicts by row (reports/TOOL-AUDIT-T7-C*.md): %d rows; " % len(t7) + ", ".join("%s %d" % kv for kv in sorted(t7c.items())) + ".\n")
fv = Counter(v for v, _ in fold.values())
print("fold-T8.md: %d rows; verdicts after the closure check: " % len(fold) + ", ".join("%s %d" % kv for kv in sorted(fv.items())) + ".\n")
fc = Counter(final.values())
print("Final exit by row: " + ", ".join("%s %d" % (e, fc[e]) for e in ORDER if fc[e]) + "; gaps %d; total %d.\n" % (len(gaps), sum(fc.values()) + len(gaps)))
print("Where each final exit came from: " + ", ".join("%s %d" % kv for kv in sorted(Counter(why[i] for i in final).items())) + ".\n")

if "--theme-closures" in sys.argv:
    # Closures that left by an answer by theme or a late fold, for a fresh closure check.
    print("## Closures by theme answer\n")
    print("| Row | Source | Exits |")
    print("|---|---|---|")
    for i in sorted(i for i in final if why[i].startswith("theme") or why[i] == "late-folds.md"):
        ex = "; ".join(e + (", " + ANSWERS[a][0] if a in ANSWERS else "") for e, a in parts[i])
        print("| %s | %s | %s |" % (i, why[i], ex))
    sys.exit(0)

print("## Gaps\n")
for i, w in gaps:
    print("- %s: %s" % (i, w))
print()
print("## Rows by final exit\n")
for e in ORDER:
    ids = sorted(i for i in final if final[i] == e)
    if ids:
        print("**%s** (%d): %s\n" % (e, len(ids), ", ".join(ids)))

print("## Fix pending and exit 3 rows by answer\n")
groups = defaultdict(list)
for i, p in parts.items():
    for e, a in p:
        if e in ("exit 3", "Fix pending", "exit 1") and a:
            groups[a].append(i)
pend = sorted(i for i, p in parts.items() if ("Fix pending", "pending") in p)
print("**Fix pending, copies that agree (T6 category P, T7 PASS)** (%d): %s\n" % (len(pend), ", ".join(pend)))
for a, (label, where, fix) in ANSWERS.items():
    ids = sorted(set(groups.get(a, [])))
    if ids:
        print("**%s** (%s; %s) (%d): %s\n" % (label, where, fix, len(ids), ", ".join(ids)))
open_fix = sorted(i for i, p in parts.items() if any(e in ("exit 3", "Fix pending") and a not in DONE and a != "lib4" for e, a in p))
print("Rows with a fix still to make (any part exit 3 or Fix pending, not done): %d\n" % len(open_fix))
