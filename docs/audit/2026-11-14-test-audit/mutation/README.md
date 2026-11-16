# Mutation run 20261116-123523: the evidence T8 keeps

Mutation run evidence, with four limits: (1) the tool plants its own kinds of bug, not the change an auditor named, so caught nothing is not proof a test is empty; (2) the run went several bugs at once, so a timed-out bug lists its red tests but credits none of them as catches; (3) each verdict comes from one run of its bug; (4) the run held only the C++ tests in hydra_tests, so a test outside it has no evidence here, which is not the same as catching nothing. See docs/audit/2026-11-14-test-audit/mutation/README.md.

These files come from the run folder `worktrees/mutation-tool/build-mutation/runs/20261116-123523`. That folder sits in a worktree's build folder and goes when the worktree does, so the text files T8 reads are kept here. `exits/mutation_evidence.py` copied them and is their one reader. The plan is `docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md`, step E1c.

| File | Bytes | What it is |
|---|---|---|
| `report.txt` | 1108678 | copied whole: the run's limits, counts and lists of bugs by state |
| `tests.txt` | 598964 | copied whole: each test's caught and reaches counts, the tests that caught none first |
| `summary.json` | 794167 | copied whole: the run's counts, its crashes and its catches through shared state |
| `timed-out-bugs.txt` | 1493262 | the 636 timed-out bugs' blocks of the run's bugs.txt, byte for byte |
| `survivors-by-file.md` | 47681 | written here: report.txt's survivors grouped by source file |

The rest of bugs.txt is not kept. It repeats what tests.txt lists per test. The bugs folder and map.json are too big to keep.

## The four limits

1. The tool plants its own kinds of bug, not the change an auditor named. So caught nothing is not proof that a test is empty. The report's own words: "A test that caught nothing has shown no way to fail against these bugs. That is a question to look at, not proof the test is empty."
2. The run went 8 bugs at once (summary.json, "processes"), so a slow test can be counted as caught on a time-out. A timed-out bug's red tests are listed but never credited as catches.
3. Each verdict comes from one run of its bug. 14 of the 636 timed-out blocks name no test running at the time limit.
4. The run held only the C++ tests in `hydra_tests`. A Python test, a `hydra_uitest` test or a helper function has no evidence here at all. Having no evidence is not the same as catching nothing.
