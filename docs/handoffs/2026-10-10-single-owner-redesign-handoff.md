# Handoff: single-owner scan redesign (2026-10-10)

**For the next session: build this plan.** On 2026-10-10 the user read a summary of this plan and replied "approve all recommendations". That yes covers the design under repo rule 3. Every decision below is settled. Nothing has been built yet.

The plan is `docs/superpowers/plans/2026-10-10-single-owner-redesign.md`. Read it first; this note only records what was decided and how it fits with the other work running now.

## Why this exists

A read-only audit of the test suite on 2026-10-10 found that `tests/test_single_owner.cpp` has become a hot spot. It is 6,558 lines, most of it a table of about 357 regex rules written as C++. 352 of the 1,335 commits since 2026-10-03 touch it, nearly all to add one row, and rows are never retired. Its two "only shrink" baselines have not shrunk. 173 rows have no guard, so a renamed owner can silently turn a rule into a no-op. The audit is `docs/audit/2026-10-10-test-suite-audit.md`. The structure scout's evidence is `docs/handoffs/2026-10-10-test-audit/C-structure.md`.

This test is how the user enforces "every rule derived once". The redesign must not weaken that promise. A row that flags a line today must flag it after the move.

## What the user decided

The rules move to JSON, with one file per owner area rather than one big file. Every row must carry `date`, `decision`, `added_by`, `owner_file` and `owner_symbol`, and `note` and `anchor` are optional. The scan checks that the owner still exists and fails with "owner gone" when it does not.

Rows have no expiry date. A row retires when its owner goes, or when a person judges the copy impossible, helped by an age-listing tool.

Both baselines get emptied. After that, a new baseline entry needs a `remove_by` date at most 14 days out, and the suite fails once that date passes. The 14 is the planner's suggestion, which the user's yes adopted. The failure message must name the entry and whose task it was.

For speed, add per-row literal anchors first. Use threads only if the measured scan time stays above today's 4.2 seconds.

The docs test uses strictness level (b): code words outside comments and strings, plus production string literals and `CMakeLists.txt` names. The planner estimates 10 to 15 names in about 8 docs will need rewording or an allow-list entry with a reason. The docs test widens to `docs/agents/*.md` and `README.md`. The three names that fail there today get reworded rather than allowed.

No structural tool like clang-tidy query checks is adopted now.

## How to run it

Tasks 4, 5 and 6 can start at once, in parallel. Tasks 1, 2, 3 and 7 run in order on one branch, and they merge together. The derive-once precheck (`tools/derive_once_precheck.ps1`) reads the C++ table by position today, so it must switch to reading the JSON in the same merge as the table move, or every derive-once review breaks. The converter must run again at merge time against the merged tree, to carry rows other sessions added meanwhile. The plan estimates 8 to 10 agent-hours and four merges, each with one fresh Sonnet reviewer under D107. Executors run on Opus. The doc edits in tasks 6 and 7 also go through the doc-review gate.

## Fits with other work

The duplicates-and-naming plan (`docs/handoffs/2026-10-10-test-dedup-naming-handoff.md`) holds back its renames and splits of eight files until this redesign lands. Those files are named in single-owner rows and in the clone baseline, and the user chose to edit those rows once, in their new form. When this lands, tell that plan's session it can start its wave C.

Three sessions from the audit were already running when this was written. They are "Hook dispatchers: fail closed on a broken gate", "Cap every uncapped wait in the test suites" and "Trim the slow test and dead tests". The trim session removes or edits several test cases, and some of those may be named in single-owner rows. Check its merge before converting.

## Questions the plan left open

These are measurements, not user decisions. Task 3 measures whether regex compile time is a visible share of the 4.2 seconds. Task 5's agent decides whether fixing the 16 clones makes those tests harder to read, and may leave one dated entry with a reason. Task 2 lists any rows the precheck's .NET regex reads differently after the move.
