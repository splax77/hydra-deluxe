# The single-owner rules are data

The single-owner scan enforces the user's rule that every question in the code
is answered in one place. Each of its rows names a question, the one place
that answers it (the owner), and a pattern for any other line that answers it
again. Those rows used to be a table of C++ initializers inside
tests/test_single_owner.cpp. They are now JSON files in
tests/single_owner/rules/, one file per area. The C++ in tests/single_owner/
only reads the rows and runs them. The user approved this on 2026-10-10, with
every recommendation in docs/superpowers/plans/2026-10-10-single-owner-redesign.md.

## Why the rows moved

Five things were wrong with the table, and all five came from the rows being
C++ rather than data. The counts below are the plan's, taken from the checkout
and `git log` at commit 784e47cc.

The precheck carried a C++ parser. tools/derive_once_precheck.ps1 needs the
rows too, so it read them out of the C++ by brace position, and kept its own
self-test to notice when a struct member moved. That was a parser for C++
written in PowerShell, and it existed only because the rules were not data.
Now it reads the JSON with `ConvertFrom-Json`.

Every new row recompiled the largest test file. The file had grown to 6,558
lines, about 5,600 of them the table.

One file was the hot spot for every merge. A derive-once fix ends by adding a
row, so 352 of the 1,335 commits since 3 October touched that one file. Two
agents adding rows at once was the main merge conflict between parallel
tasks. With one file per area, two agents adding rows for different owners
never touch the same file.

Rows went silently vacuous, meaning they still passed but checked nothing. A
row without owner lines only ever says "nothing else may say this", so it kept
passing after its owner was renamed or deleted. 173 of the 357 rows had no
guard against that.

The baselines never shrank. They were meant to: each entry is a known copy
parked until a fix removes it. But all four known copies were still there,
and so were all sixteen clone entries from 5 October, because nothing made
them leave.

## Where a row goes

A row goes in its owner's area file. For an owner under src/, that is the
file named for the owner's folder: an owner in src/core/ goes in
rules/core.json, one in src/ui/ in rules/ui.json. Owners under tests/ go in
rules/tests.json, owners under tools/ in rules/tools.json, and owners at the
repository root in rules/root.json. The rows that read Python files go in
rules/probe.json.

The loader reads every `.json` file in that folder, and in the folders one
level down (`select_rule_files` in tests/single_owner/engine.cpp), so a new area file
needs no code change.

**2026-10-10: a new row may go in its own file.** Eight commits on main's
first-parent line changed rules/tests.json on 2026-10-10 alone (`git log
--first-parent`, up to a69a2a4b), most of them by adding a row at the end of
its array. Two branches that each add a row at the end of one array conflict
every time. So a new row may instead go
in a new file in a folder named after its area, for example
rules/tests/scratch_paths.json, in the same `{"rules": [...]}` form. A failure
names that file by its path, as `rules/tests/scratch_paths.json`. The rows
already in an area file stay where they are; nothing is moved.

## What a row carries

`OwnerRule` in tests/single_owner/rows.h holds each field's exact meaning.
`parse_rule` in engine.cpp reads them. A row missing a required field, carrying
a field the loader does not know, or holding a value of the wrong type fails
the load with the rules file and the row's question, so a typo cannot drop a
field quietly.

| Field | Required | Meaning |
|---|---|---|
| question | required | The question in plain English, shown on a failure. |
| owner | required | The one place that answers it, in words. |
| owner_file | required | The repository path of the owner's file. The owner-gone guard reads it. |
| owner_symbol | required | The function, constant or type in that file that answers the question. |
| date | required | The day the row was added. |
| decision | required | The ADR, D-number or review key that decided it. |
| added_by | required | The task or review that added the row. |
| pattern | required | The regular expression for a line that answers the question again. |
| anchor | optional | Plain text every match contains. The scan runs the pattern only on lines that hold it, which keeps the scan fast. |
| note | optional | Why the pattern is shaped as it is. JSON has no comments, so this is where that reason lives. |
| calls_owner | optional | A pattern for a line that calls the owner. A flagged line that also matches it passes. |
| owner_files | optional | Files that may match freely. |
| exempt | optional | Files whose matches answer a different question, each with the reason. |
| must_match | optional | Example lines the row must flag. |
| must_not_match | optional | Example lines the row must not flag. |
| owner_lines | optional | The owner's own lines that answer the question, each of which must still match. |
| scope | optional | The folders or single files the row reads, when not the default. |
| scan_comments | optional | Set when comments must not use the spelling either. |
| language | optional | "py" for a row that reads Python files; left out for C++. |
| function_file, function | optional | Limit the row to one function's body in one file. |

The regular expressions need their backslashes doubled in JSON. A slip there
fails at once: the self-test "single-owner rules match their own examples"
checks every example line, and checks that every example and owner line holds
the row's anchor (`anchor_misses`).

A row that fails names its rules file at the end of the message, so the fixer
opens the right one. A self-test pins that message on a made-up file, so a
reword has to be deliberate.

## The owner-gone guard

A row fails when its owner_file no longer exists, or no longer names its
owner_symbol outside comments (`owner_gone`). The message says to name the new
owner or retire the row. So a renamed or deleted owner is a test failure, not
a chance find. The idea comes from ArchUnit, a Java architecture-test library,
where a rule whose subject has gone fails instead of passing empty.

A check that the pattern matched nothing anywhere would be the wrong guard.
Many rows are meant to match nothing on a clean tree: the owner spells the
answer another way, and nothing else may spell it the forbidden way.

## Rows have no expiry date

A row stays until it is retired, and nothing fails on a calendar day. A rule
about where a question is answered stays true until the owner changes. A date
would teach agents to bump dates, not review rows. A row is retired when its
owner goes, which the guard forces, or when someone judges that the copy can no
longer be written at all. tools/single_owner_rows.ps1 lists the rows oldest
first, with their decisions, for that review.

## The baselines carry a removal date

tests/single_owner/baseline.json holds the two baselines. `known_copies` lists
lines the row scan tolerates, each with the fix that will remove it.
`known_clones` lists the copied blocks the clone scan tolerates (ADR 0025). An
entry that no longer matches fails, so the lists only get shorter.

Every entry also carries the day it was added, who added it, and a
`remove_by` date. The loader refuses a `remove_by` more than 14 days after the
entry's date (`kBaselineMaxDays`), and the scan fails the day after it
(`baseline_due`). So a copy may be parked, but never kept. The user chose 14
days in the plan's question 5. The dated entry lets an agent that can't edit a
file another task owns still land its change, while the clock runs.

## The precheck reads the same files

tools/derive_once_precheck.ps1 reads the rules files that `select_rule_files` in
tests/single_owner/engine.cpp names, and baseline.json, at the revision it
checks, with `ConvertFrom-Json`. The scan and the precheck read one set of
files, and neither parses the other's language.

## Rejected

**C++ include files, one per area.** These need no escaping. But every row
edit would still recompile, and the precheck would keep its C++ parser.

**An expiry date on every row.** See "Rows have no expiry date" above.

**A structural tool such as clang-tidy.** clang-tidy's query-based custom
checks were considered and not adopted; the plan's "Prior art" section gives
the reasons.
