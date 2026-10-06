# A copied block of code fails the source scan

A block of code pasted into a second place is the plainest way to write a rule
twice. The derive-once review catches some of these, but only when the
reviewer happens to notice. A text scan catches every one, every time. That
leaves the reviewer's attention for the hard kind: two different-looking
derivations of the same fact, which no text scan can see.

The research behind this is the report "Maintaining large codebases with AI"
(5 October 2026, recommendation 1). Its main point is that copy-paste rises
fastest in AI-built codebases, and that deterministic checks hold the line
where AI review only sometimes does.

## What the scan does

The scan lives in tests/test_single_owner.cpp, next to the row scan, and reads
the same files through tests/source_tree.h. That keeps one scan of the source
tree, as the review's kind E asks.

It boils each line down to the code on it, so lines that carry no code drop
out and spacing stops counting (`clone_line_text` owns the rule). Then it
looks for a run of eight such lines that appears in two places, in two files
or twice in one. Overlapping runs merge into one copied block, so a failure
names each block once, with both places (`find_clones`).

## The window: eight lines

The user chose "about eight lines" in chat on 2026-10-05 (task MR1), and
asked for the counts at 6, 8 and 10 before the number was fixed. On that
day's tree the scan found 102 copied blocks at 6 lines, 26 at 8 and 12 at 10.

At 6 lines the list floods with boilerplate. Most of it is the opening lines
that many UI tests share, which pair up dozens of ways, plus Windows header
guards and the command-line tools' shared start-up lines. At 8 lines nearly
every block is a real copy: the report pages' shared HTML, two audio readers'
read loops, and repeated test setup. At 10 lines every copy in the shipped
code under src/ slips through, because those copies are 8 and 9 lines long.
So the window is eight (`kCloneWindowLines`).

## The baseline only shrinks

The 26 blocks found on 2026-10-05 are listed in `known_clones`, so the test
passed on the day it arrived. A new copy fails. An entry that no longer
matches a block also fails, so the list can only get shorter. When a fix
removes a copy, its entry goes too.

Each entry is keyed on the two files, the block's length in code lines and
its first code line. Line numbers are left out, so an edit elsewhere in either
file doesn't break the entry. If a fix only shortens a block, the old entry
goes stale and the shorter block shows up as new; the entry is then replaced
with the shorter one. Each entry covers one block, by the same rule every
other listed line in that file follows (`take_listed_line`).

## What it doesn't catch

It only sees text. A copy that renames its variables or reorders its lines
passes. So does a copy shorter than eight code lines. A rule worked out twice
in two different ways is still the review's job.
