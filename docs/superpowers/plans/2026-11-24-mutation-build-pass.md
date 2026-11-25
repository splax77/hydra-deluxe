# Mutation build pass: one extra build per phase, for planted bugs only

## The short version

You answered question 1 of the test-guard plan (`docs/superpowers/plans/2026-11-24-test-guard.md`) with "Allow it". The text of that choice was: "One extra mutation-only build per phase that touches a C++ test. Needs its own reviewed change to the build gate in the hooks repo." This plan is that change. That answer was given in a question box in session `b885c6f8` on 2026-11-24. It is a session record, not a filed ruling: nothing under `## Confirmed` in `docs/agents/rulings/` holds it yet.

Today the build gate cannot do it, for two reasons found in the code.

First, the gate does not know what a session is about to build. `hydra_build_pass` takes no argument, and the preset (the named build setup, such as `default` or `mutation`) is chosen later, on the `build_cpp.ps1` command line. So "mutation only" has nothing to stand on yet (premises P3 and P4).

Second, no tool prints "the C++ test files this range changed". The precheck owns the rule for what a test file is, but its `--tests` mode answers a different question: which test files to run (premises P8 and P9).

So the plan has three parts.

1. **The pass carries its preset.** The session names the preset when it asks for a pass. The pass is then good for that preset and no other; the build refuses any other. Think of a ticket with the seat printed on it.
2. **The precheck lists changed C++ test files.** One new mode prints them, using the precheck's own test-file rule.
3. **The gate gives the extra pass.** The judge allows one `mutation` pass per phase when five recorded facts hold. It reads every one of them; it guesses none.

The Hydra half (parts 1 and 2, and the page) lands first, as one phase with one build. The hooks half (part 3) lands after it.

## The rule the judge applies

Every request for a pass names its preset. A request with no preset is refused, whoever asks.

**A request for the `mutation` preset** gets a pass only when all of these hold. Each one is read from a record, and a record that is missing or broken refuses and names the file.

1. The session has joined exactly one open plan run, and the open phase has a build count of 1. (Today's checks, unchanged.)
2. Every task branch of the phase is merged onto the plan branch, and every gated file on it is signed. (Today's checks, unchanged.)
3. The plan branch's tip holds the marker file `.build-pass-preset`. The marker says this branch's build scripts tie a pass to its preset. The pass tool is the main checkout's file (`.mcp.json` starts `tools/hydra_ui_mcp.py` from the project folder), but the claim that enforces the preset is the script in the tree being built. A phase branch cut before H1 landed has the old script, which ignores the preset. Without the marker the judge cannot trust that a mutation pass builds only the mutation preset, so it refuses.
4. A normal build of the tip itself has passed on a clean tree. "Normal" means any recorded build whose `preset` field is not `mutation`. This is the same test `main` already needs before it takes the phase.
5. The build record holds no `mutation` build for this phase yet. A build that started and never finished counts as used, as it does today.
6. The phase changed at least one C++ test file. The judge asks the precheck's new mode for the range `main...<tip>` and needs at least one line back.

**A request for any other preset** follows today's rule: one build, then one rebuild after a new reviewed commit, then stop. One thing changes, and only on a branch that holds the marker: `mutation` builds are left out of that count. On a branch without the marker the count is exactly today's, presets ignored.

### Why the extra build cannot become a third normal build

Three things close that door, and each has a test.

The normal builds are counted alone. A mutation build never adds a normal build and never gives one back. After a failed first build and a failed rebuild, a normal request is still refused, with or without a mutation build in the record.

A mutation pass cannot build another preset. The pass's lock file holds the preset, and the build's claim refuses a different one. This is checked at the moment the build starts, not from what the session said it would do. The two words must match letter for letter, capitals included. PowerShell would accept `-Preset Mutation` for the preset `mutation` and `build_cpp.ps1` would record it as typed, so a loose match would let a mutation build be recorded under a name the judge does not read as `mutation` (premise P24).

A mutation pass is given only at a tip whose normal build already passed clean. A phase that still needs a rebuild has no such tip, so it gets no mutation pass until its fix is built.

One consequence is worth saying plainly. The extra build is one per phase, not one per commit. If a session takes it early and a later fix changes the tests, there is no second one. So the page tells the main session to ask for it last, after the phase's tests pass.

### What the rule does not look at

The judge uses the precheck's test-file rule as it is. That rule counts every C++ file under `tests/`, which includes the files of `hydra_uitest`. The mutation build builds only `hydra_tests` (`CMakePresets.json`, the `mutation` build preset). So a phase that changes only a `hydra_uitest` file may take the extra build, and the mutation gate will then judge none of its tests. The judge does not narrow the rule to fix that; narrowing it would be a second copy of the precheck's rule.

The judge does not check that the mutation build ran on a clean tree. The build record says so in its `dirty` field, as it does for every build, and nothing reads that for a mutation build.

## Existing owners (agent rule 15)

"May this session have a build pass?" is `pass_refusal` in `C:\Users\Patrick\.claude\gates\plan_run_judge.py`. The new rule goes inside it. No second decision is added.

"Did a build of this tip pass on a clean tree?" is already answered inside `main_refusal`, in the same file (premise P2). K1 lifts that test into one helper and both decisions call it.

"Which preset did a build use?" is the `preset` field of the build record, written by `tools/build_record.ps1` (premise P5). The judge already requires the field on every line and reads it nowhere.

"Is a pass live, and whose is it?" is `tools/build_slot.ps1` (premise P6). The preset line goes into the lock file that script already owns.

"Is this a C++ test file?" is `test_file` and `is_cpp` in `tools/precheck/range.cpp` (premise P8). "Which files does a range touch?" is `AddedLines::from_diff(...).files()` in the same file. The new mode calls those three and adds no rule. I searched `tools/precheck/main.cpp` for a mode that already prints the answer and found four modes, none of which does (premise P9). One edge of that owner: a file the range deletes is not in its answer, because the diff parser keeps only paths that exist at the tip (premise P10). So a phase that only deletes a test file does not qualify, which is right, since there is then no test left to judge.

"Does the marker's meaning have an owner?" `.plan-run-gate` is the one marker the hooks read, and it says only that a repo's `main` enforces plan runs (premise P22). It cannot also say that a branch's scripts enforce the preset, because a branch cut before H1 landed has the first and not the second. I searched the gates and the build scripts for a version number, a capability line or a marker that says which scripts a branch holds and found none. The other way to know is to read the script's text from the hook, which is reading prose, and a hook never does that. So the marker is new, and it is a plain file whose presence the hook reads, as P22's hook reads `.plan-run-gate`. It changes nothing a player or the user sees.

"What is the mutation preset called?" is `CMakePresets.json` (premise P7). The judge holds the one word `mutation` with a comment that points there.

## What the plan does not need

It changes hooks, build scripts, a developer tool and two pages. It does not change what Hydra decides or shows, so it needs no Owner map. It has no live step, because nothing here drives Clone Hero.

One thing changes for main sessions, and it comes with the choice you made. Every `hydra_build_pass` call now names its preset, and the pass builds that preset only. A session started before the change lands keeps the old tool, which cannot name one, so it is refused a pass until it restarts (premise P16). Nothing you see in Hydra changes.

## Plan run

Slug: mutation-build-pass
Repo: C:\Users\Patrick\Downloads\Hydra\hydra-test

| Phase | Builds |
|---|---|
| all | 1 |

## The tasks

| Task | Branch | Phase | Owned files |
|---|---|---|---|
| H1 | `claude/mbp-pass-preset` | all | `tools/build_slot.ps1`, `tools/hydra_ui_mcp.py`, `build_cpp.ps1`, `tools/test_build_pass.ps1`, `tools/test_hydra_ui_mcp.py`, `.build-pass-preset` |
| H2 | `claude/mbp-changed-tests` | all | `tools/precheck/main.cpp`, `tests/test_precheck_changed_tests_exe.cpp` |
| H3 | `claude/mbp-page` | all | `docs/agents/plan-runs.md`, `docs/development.md` |

The hooks task is not in that table, because the run works in one repo and the hooks live in another. It is task K1. Its executor works in the worktree `C:\Users\Patrick\.claude\worktrees\mutation-build-pass`, on the hooks repo's `claude/mutation-build-pass`. It owns `gates\plan_run_judge.py`, `gates\percall.py`, `gates\tests\test_plan_run_judge.py`, `gates\tests\test_percall_build_pass.py` and `gates\README.md`. It needs no compile. It gets its own review and lands on the hooks repo's `main` after the Hydra phase has landed and step X1 is done.

H1, H2 and H3 are independent and are dispatched together. K1 is written at the same time, since it reads only what this plan fixes, but it is merged last.

### H1: The pass carries its preset

`hydra_build_pass` gains one argument, `preset`, with no default. `build_pass` in `tools/hydra_ui_mcp.py` hands it to `tools/build_slot.ps1 -NewPass` as `-Preset`. `New-BuildPass` writes it into the lock as a `preset=` line, beside the `pass=` and `session=` lines it writes today, and prints the build command with that preset in it.

The claim then checks it. `build_cpp.ps1` passes its own `-Preset` to `build_slot.ps1 -Claim`. `Enter-BuildPass` compares the two. If the lock has a `preset=` line and the claim's preset is a different word, or the claim gave none, it throws and the build stops before configure. The check comes before the "an enclosing claim already covers this build" return, so a nested build under a claimed pass is held to the same preset. The rewrite of the lock at claim time keeps the `preset=` line.

The two presets are compared letter for letter, capitals included (premise P24).

`-NewPass` with no `-Preset` is refused, with text that says to name one. The script has no default, as the tool has none. Every existing `-NewPass` case in the two test files gets a preset added.

`Read-BuildSlot` reads only the `pass`, `session` and `expires` lines today (premise P25). H1 adds `preset` to what it reads and returns, because the claim and the test both read the lock through it.

A lock with no `preset=` line is claimed as it is today. Such a lock comes from a server started before this change, and K1 refuses those calls anyway once it lands, because they carry no preset.

The callers of `-Claim` and `-Hold` are known (premise P12). `build_cpp.ps1` is the only caller of `-Claim`, and it passes its own preset. Nothing calls `-Hold`, but it gets the same `-Preset` argument and the same check, so a later caller cannot skip it. The script's own `-Repo` mode takes a plain slot and runs `build_cpp.ps1` with its default preset, so under a pass it builds `default` only, and a `mutation` pass is refused there by the same claim.

H1 also adds the marker file `.build-pass-preset` at the repo root. It holds two plain sentences saying what it means and that the hook reads only that it exists, the way `.plan-run-gate` does.

Acceptance criteria:

- `tools/test_build_pass.ps1` has a case for each of these, each run through the real script on a temporary slot folder: a pass asked for `mutation` and claimed with `mutation` is claimed; the same pass claimed with `default` is refused and the lock is left unclaimed; the same pass claimed with `Mutation` is refused; the same pass claimed with no preset is refused; a nested claim under a claimed `mutation` pass with `default` is refused; a lock written with no `preset=` line is claimed with any preset; `-NewPass` with no `-Preset` is refused.
- After a claim, the lock still holds its `preset=` line. Proof: the test reads the lock through `Read-BuildSlot`.
- `tools/test_hydra_ui_mcp.py` has a case showing `build_pass("mutation")` returns a pass whose lock holds `preset=mutation`, through the real script, and that the printed build command names `-Preset mutation`.
- Every caller of `-Claim` or `-Hold` passes a preset. Proof: the reviewer greps `tools`, the repo root and `.github` for the two switches and finds none without one.
- `.build-pass-preset` exists at the repo root.
- Both test files need no build (premise P13). H1 runs `tools/test_build_pass.ps1`, the `BuildPassTool` cases of `tools/test_hydra_ui_mcp.py`, and `tools/test_build_record.ps1` (its fixture copies the real `build_cpp.ps1` and `build_slot.ps1`), and reports each result.

### H2: The precheck lists the C++ test files a range changed

`hydra_precheck` gains one mode, `--changed-tests [range]`. It resolves the range the way every other mode does, takes the files the range changed from `AddedLines::from_diff` (a file the range deletes is not among them, premise P10), keeps those for which `test_file` and `is_cpp` are both true, and prints one path per line in byte order. With none it prints nothing and exits 0. A bad range exits 1 with git's error, as `--tests` does.

The mode adds no rule about what a test file is. It is three calls to code that exists.

The test-guard plan's gate (its task G2) takes the changed test files as input, and no tool hands them over today. This mode can serve it too. That is a note for whoever briefs G2, not work in this plan.

Acceptance criteria:

- `tests/test_precheck_changed_tests_exe.cpp` runs the real exe on a throwaway git repository the test creates. One case changes a `.cpp` file under `tests/` and requires that path, alone. One changes only a file under `src/` and requires no output and exit 0. One changes a Python test under `tools/` and requires no output, because it is a test file but not C++. One deletes a `.cpp` file under `tests/` and requires no output. One gives a bad range and requires exit 1.
- The expected paths are the ones the test itself committed. No expected line is copied from a run.
- The first case also shows the mode sees files under `tests/` at all, which premise P10's source already says.
- The usage text and the header comment of `main.cpp` name the mode and say nothing the code does not do.
- H2 hands back uncompiled. The main session runs the new file's cases after the phase's one build.

### H3: The page says when the extra build is given

`docs/agents/plan-runs.md`, "The build and the move of `main`", gets a short part. `docs/development.md`, "Building under Claude Code", is the other page that shows the call. It shows `hydra_build_pass` with no preset and a build command with no `-Preset`, and it says "Any other target or preset goes the same way". H3 makes it say the call names a preset and the pass builds that preset only. The plan-runs part says: a pass is asked for by preset; a phase that changed a C++ test file may take one extra build, of the `mutation` preset only, on the tip whose normal build passed; the main session asks for it last, after the phase's tests pass, because there is only one; and the extra build never counts as the phase's build or its rebuild. It points at `plan_run_judge.py` for the rule and does not restate the six conditions.

The sentence "the gate never gives a third" stays true for normal builds. H3 words it so nobody reads the extra build as that third.

The main checkout holds an uncommitted edit to `plan-runs.md` line 36: the pointer text now reads "start with `plan` followed by a slash". The main session made it in this session, so it is not a found edit. H3 works on its branch, leaves line 36 alone and edits only the section above, so the two do not overlap. The main session commits its edit before the phase lands, so the merge does not stop on it.

Acceptance criteria:

- The page states the user's choice in the user's words, quoted, with the date 2026-11-24.
- The page says the extra build is asked for after the phase's tests pass, and why.
- The page gives no build time or cost figure.
- Every `hydra_build_pass` call the page shows names a preset.
- Every `hydra_build_pass` call that `docs/development.md` shows names a preset, and its build command carries `-Preset`.
- The reviewer greps `docs/agents`, `docs/development.md` and `CLAUDE.md` for `hydra_build_pass`. Premise P14 lists the hits: `CLAUDE.md`, `agent-rules.md` and `brief-preamble.md` only name the tool and say nothing about its argument, so they stay. The reviewer lists any other hit that says or shows the pass taking no argument.

### K1: The gate gives the extra pass

`percall.py` reads the `preset` value from the tool call's input and hands it to `pass_refusal`. A call with no `preset`, or one that is not a non-empty string, is refused with text that says to name the preset. This applies to every main session, with or without a run, so a typed `allow build` also needs a preset on the call.

`plan_run_judge.py` then applies "The rule the judge applies" above. In detail:

- The marker test is one git read: does the tip's tree hold `.build-pass-preset`.
- The "passed clean on this tip" test is lifted out of `main_refusal` into one helper. `main_refusal` calls it with the builds it reads today, so its answers do not change.
- The judge reads the preset exactly: only the word `mutation`, letter for letter, is the mutation preset. `Mutation` is another preset and follows the normal count.
- The changed-test question runs `<run repo>\build-cpp\Release\hydra_precheck.exe --repo <run repo> --changed-tests main...<tip>`. This is the main checkout's exe, the one every reviewer already uses (premise P11). A missing exe, a non-zero exit or output on stderr refuses, and the refusal names the exe and quotes what it said. Nothing is assumed in its place. An exe built before H2 does not know the mode and exits non-zero, so it refuses too (X1 makes the exe current). The command is a parameter of the function, with the line above as its default, so the tests can hand in a stub that prints chosen lines; the missing-exe case uses the default path in a repo that has no `build-cpp`.
- Each refusal says which of the six conditions failed and what to do instead.
- `builds_used`, which the start-of-session list prints, is not changed. It goes on counting every build of the phase.

K1 adds no new number. The exe call has no time limit of its own, as the git calls beside it have none (`review_rules.git`, premise P28); the hook's own timeout bounds both.

Acceptance criteria, each a case in `gates\tests\test_plan_run_judge.py` built on real git repositories and real record lines, the way the file's existing cases are (premise P15):

- A `mutation` request passes when all six conditions hold.
- It is refused, with text naming the cause, when exactly one condition is broken, one case per condition: no marker at the tip; no normal build of the tip; the tip's only normal build failed; the tip's only passed normal build was dirty; a `mutation` build already recorded, passed; a `mutation` build already recorded, started only; the phase changed no C++ test file; the exe is missing; the exe exits non-zero; the phase's build count is 0; a task branch is unmerged.
- The back door, with the marker present: after a failed build and a failed rebuild, a `default` request is refused. It is still refused after a `mutation` build is added to the record. And a `mutation` request at a tip whose latest normal build failed is refused.
- The count, with the marker present: one passed normal build, one `mutation` build, then a new signed commit; a `default` request passes. This fails if mutation builds are counted as normal ones.
- The spelling, with the marker present: a recorded build whose preset is `Mutation` counts as a normal build, and a request for `Mutation` follows the normal count.
- Without the marker: the same record refuses the `default` request, as today. This fails if the new count leaks onto unmarked branches.
- Every existing case in the file passes with only the new `preset` argument added to its call.
- `main_refusal`'s existing cases pass untouched.

And in `gates\tests\test_percall_build_pass.py`:

- A main-session call with no `preset` is denied. A call with `preset` set to a number or an empty string is denied.
- A call with a preset reaches `pass_refusal` with that exact value.
- A session with no run and a typed `allow build` gets through only with a preset on the call.

`gates\README.md` says, in the plan-run gate's paragraphs, that a pass is asked for by preset and that the extra build's rule lives in `plan_run_judge.py`. It does not restate the conditions.

K1 runs those two test files and no others. The change touches no shared code (`gates\common.py`, `gates\lib`, `gates\settings.fragment.json`, `gates\tests\lib`), so the whole hooks suite is not run. If K1 finds it must touch one of those, it stops and reports.

Waits for: the Hydra phase landing on `main`, and X1.

### X1: The main session makes the new pieces live

This is the main session's step, after the Hydra phase lands and before K1 is merged. It has three parts, in order.

It asks you to type `allow build`, in chat, in a session that has joined no run (the finished run closed first, or a fresh session). That is the existing line for one build outside a run (`docs/agents/plan-runs.md`, "The lines the user types"), so this plan puts no question to you; the typed line is the answer, and no build starts without it. The session then rebuilds `hydra_precheck` in the main checkout from the new `main`, so the exe K1 calls has the new mode. The exe in the main checkout was built 2026-11-24 19:25 (premise P23), before H2 exists, so until this build the judge would refuse every mutation request, correctly.

It runs `build-cpp\Release\hydra_precheck.exe --changed-tests` on the phase's own range and checks the output names `tests/test_precheck_changed_tests_exe.cpp`.

It tells you that sessions started before the phase landed still run the old `hydra_build_pass`, which cannot send a preset (premise P16). Once K1 lands, those sessions are refused a pass until they restart. So K1 is merged when you say no other session is mid-build.

Stop condition: one build and one run of the exe. If either fails, X1 stops and reports the exact output; K1 stays unmerged.

Acceptance criteria:

- The exe prints the new test file for the phase's range. The main session quotes the line in its report.
- K1 is not merged before that line exists.

## Global constraints

- Agents never build and never run `hydra_tests.exe` (agent rule 14). H1, H2 and H3 hand back uncompiled.
- No task adds a threshold, a timeout or any other new number. A task that needs one stops and reports.
- The judge defaults nothing. Each missing value or broken record refuses and names the file. K1's one-broken-condition cases are the proof.
- The rule for what a test file is stays in `tools/precheck/range.cpp`. No Python or PowerShell file gains a copy. Proof: the K1 reviewer greps the hooks change for `tests/` and `.cpp` used as a pattern and finds none outside the test files.
- The order is fixed: Hydra phase, then X1, then K1. Reversing it would refuse every build pass on the machine, because the tool could not yet send a preset.

## Questions for the user

None. The one build outside the phase's count (step X1) is not a question, because a line you type already owns it: `allow build` (`docs/agents/plan-runs.md`, "The lines the user types") lets one build through for a session that has joined no run. X1 waits for you to type it.

## Out of scope, for the user to approve

- Requiring a clean tree for the mutation build. The record shows a dirty one; no rule reads it.
- Narrowing "C++ test file" to files `hydra_tests` builds. The precheck's rule is used as it is.
- A second mutation build after a late fix. The user's choice says one per phase.
- Step M1 of the test-guard plan. This rule gives it no mutation pass, and the plan says so here so nobody finds out at the gate. M1 runs after that plan's `gate` phase lands. The open phase is then `gate`, which has a build count of 0, so condition 1 refuses it; the `scans` phase, which has a count of 1 and holds the C++ tests M1 measures, has already landed and is no longer open, and the gate does not exist yet while it is open. So M1 needs your typed `allow build`, in a session that has joined no run (the finished run closed first, or a fresh session), and a `hydra_build_pass` call that names the preset `mutation`. The typed line skips the six conditions and gives one pass.
- Any hook that forces the mutation build or the gate to run. This plan only allows the build.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| A request with no preset is refused, whoever asks | "The rule the judge applies", first paragraph | K1 | `test_percall_build_pass.py`: no preset, a number and an empty string are each denied |
| A typed `allow build` also needs a preset | K1, first paragraph | K1 | `test_percall_build_pass.py`: the no-run case passes only with a preset |
| A mutation pass needs all six conditions | "The rule the judge applies", conditions 1 to 6 | K1 | `test_plan_run_judge.py`: one passing case, and one refused case per broken condition |
| A missing or broken record or exe refuses and names the file | "The rule the judge applies" and Global constraints | K1 | The exe-missing and exe-fails cases, and the existing broken-record cases with the new argument |
| One mutation build per phase; a started-only build counts | Condition 5 | K1 | The two "already recorded" cases |
| Normal builds are counted alone on a marked branch | "Why the extra build cannot become a third normal build" | K1 | The count case: a `default` request passes after one normal and one mutation build and a new commit |
| The extra build never opens a third normal build | Same section | K1 | The back-door cases: `default` refused after two failed normal builds, with and without a mutation build |
| No mutation pass at a tip that still needs a rebuild | Same section | K1 | The case with the tip's latest normal build failed |
| A branch without the marker gets exactly today's count | "A request for any other preset" | K1 | The without-marker case |
| `main_refusal`'s answers do not change | K1, second bullet | K1 | Its existing cases pass untouched |
| `builds_used` is not changed | K1, fifth bullet | K1 | The reviewer reads the diff and finds no edit to `builds_used` |
| A mutation pass cannot build another preset | "Why the extra build cannot become a third normal build" and H1 | H1 | `test_build_pass.ps1`: claimed with `default` is refused and the lock stays unclaimed |
| A claim with no preset on a bound pass is refused | H1, second paragraph | H1 | `test_build_pass.ps1`: the no-preset case |
| A nested build is held to the same preset | H1, second paragraph | H1 | `test_build_pass.ps1`: the nested case |
| The lock keeps its preset line after a claim | H1, second paragraph | H1 | `test_build_pass.ps1` reads the claimed lock |
| A lock with no preset line is claimed as today | H1, third paragraph | H1 | `test_build_pass.ps1`: the old-lock case |
| The tool has no default preset and writes it to the lock | H1, first paragraph | H1 | `test_hydra_ui_mcp.py`: `build_pass("mutation")` and the lock's line |
| Every caller of `-Claim` and `-Hold` passes a preset | H1, fourth paragraph | H1 | The reviewer's grep |
| The marker file exists | H1, fifth paragraph | H1 | The file is in the branch's tree |
| The new mode prints only changed C++ test files, by the precheck's own rule | H2 | H2 | The four cases of `test_precheck_changed_tests_exe.cpp` |
| The mode adds no test-file rule | H2 and Global constraints | H2, K1 | Reviewer reads `main.cpp`'s diff for calls only; K1 reviewer's grep |
| The page quotes the user's choice and says to ask last | H3 | H3 | Reviewer finds the quote, the date and the sentence |
| The page gives no cost figure and shows no call without a preset | H3 | H3 | Reviewer reads the changed part |
| Other pages that show the old call are listed | H3, last criterion | H3 | The grep's hits in the review's Notes |
| K1 runs only its two test files | K1, last paragraph | K1 | K1's report lists the two commands |
| K1 adds no new number | K1 and Global constraints | K1 | Reviewer reads the diff for a new constant |
| K1 lands only after the phase and X1 | X1 and Global constraints | X1 | The main session's report quotes the exe's line before the merge |
| X1 stops after one build and one run | X1, stop condition | X1 | The main session's report |
| Old sessions are named to the user before K1 lands | X1, third part | X1 | The main session's message to the user |
| Preset words match letter for letter, in the claim and in the judge | "Why the extra build cannot become a third normal build"; K1 | H1, K1 | `test_build_pass.ps1`: claimed with `Mutation` is refused; `test_plan_run_judge.py`: the spelling case |
| `-NewPass` with no preset is refused | H1, third paragraph | H1 | `test_build_pass.ps1`: the no-preset `-NewPass` case |
| The lock reader returns the preset | H1, `Read-BuildSlot` paragraph | H1 | `test_build_pass.ps1` reads the claimed lock through it |
| A deleted C++ test file is not listed | H2 | H2 | The deleted-file case of `test_precheck_changed_tests_exe.cpp` |
| `docs/development.md` shows no call without a preset | H3 | H3 | Reviewer reads the changed part |
| M1 is told plainly what it needs | "Out of scope", M1 line | none (words only) | Reviewer reads the line against test-guard's M1 text (premise P27) |
| The extra build outside the phase's count waits for the typed `allow build` | X1 | X1 | The main session's report quotes the typed line |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The judge counts every recorded build of the phase's branch whatever its preset, and allows one build plus one rebuild that needs a new reviewed commit | code | `C:\Users\Patrick\.claude\gates\plan_run_judge.py:110-123`, `C:\Users\Patrick\.claude\gates\plan_run_judge.py:166-193` | checked 2026-11-24 |
| P2: `main_refusal` already tests "a build of this tip passed with a clean tree at its start and its end" | code | `C:\Users\Patrick\.claude\gates\plan_run_judge.py:270-275` | checked 2026-11-24 |
| P3: `hydra_build_pass` takes no argument, and `build_pass` sends only the session and the holder to `build_slot.ps1` | code | `tools/hydra_ui_mcp.py:798-818`, `tools/hydra_ui_mcp.py:957-963` | checked 2026-11-24 |
| P4: The hook's build check hands `pass_refusal` only the session id, and a session that joined a run cannot use `allow build` | code | `C:\Users\Patrick\.claude\gates\percall.py:250-269` | checked 2026-11-24 |
| P5: Every build writes a started line and an end line that hold its preset, and the judge requires the `preset` field on each line | code | `tools/build_record.ps1:61-71`, `build_cpp.ps1:92-101`, `C:\Users\Patrick\.claude\gates\plan_run_judge.py:31-32` | checked 2026-11-24 |
| P6: A pass is a lock file with `pass=`, `session=` and `expires=` lines; the claim finds it by its token, rewrites it without `expires=`, and returns early when a parent process already holds it | code | `tools/build_slot.ps1:120-139`, `tools/build_slot.ps1:187-210`, `tools/build_slot.ps1:229-243` | checked 2026-11-24 |
| P7: The mutation preset is named `mutation`, builds into `build-mutation`, and its build preset builds only `hydra_tests`; `build_cpp.ps1` accepts it and claims its pass with `-Claim -Owner` | code | `CMakePresets.json:70-78`, `CMakePresets.json:117-122`, `build_cpp.ps1:36-37`, `build_cpp.ps1:112` | checked 2026-11-24 |
| P8: The precheck owns "is this a test file", "is this C++" and "which files does a range touch" | code | `tools/precheck/range.h:21-32`, `tools/precheck/range.cpp:56-77`, `tools/precheck/range.h:85-86`, `tools/precheck/range.cpp:151-155` | checked 2026-11-24 |
| P9: The precheck has four modes and none prints the test files a range changed; `--tests` prints the picker's files to run | code | `tools/precheck/main.cpp:1-135` | checked 2026-11-24 |
| P10: `AddedLines::from_diff` reads only the folders in `code_folders()`, which are `kWalkedFolders` = `src`, `tools`, `tests`; and it lists a file only when it exists at the tip (a deleted file has an empty tip path and gets no entry) | code | `tools/precheck/range.cpp:83-87`, `tests/source_tree.h:34`, `tools/precheck/range.cpp:160-161`, `tools/precheck/range.cpp:180` | checked 2026-11-24 |
| P11: The main session and reviewers run the main checkout's `build-cpp\Release\hydra_precheck.exe` on a branch | code | `docs/agents/brief-preamble.md:47` | checked 2026-11-24 |
| P12: `build_cpp.ps1` is the only caller of `build_slot.ps1 -Claim`; nothing calls `-Hold`; the script's `-Repo` mode takes a plain slot and runs `build_cpp.ps1` with its default preset | code | `build_cpp.ps1:112`, `tools/build_slot.ps1:312-325`, `tools/build_slot.ps1:341-350`; a grep of `*.ps1`, `*.py`, `*.cmd`, `*.yml` outside `docs` found no other caller | checked 2026-11-24 |
| P13: `tools/test_build_pass.ps1` and the `BuildPassTool` cases of `tools/test_hydra_ui_mcp.py` need no build: the first compiles nothing and uses a fake cmake, the second runs the real `build_slot.ps1` on a temporary slot folder | code | `tools/test_build_pass.ps1:1-20`, `tools/test_hydra_ui_mcp.py:795-810` | checked 2026-11-24 |
| P14: Pages that name `hydra_build_pass`: `CLAUDE.md` rule 14, `docs/agents/agent-rules.md` rules 14 and the hook paragraph, `docs/agents/brief-preamble.md`, `docs/agents/plan-runs.md`, and `docs/development.md`, which shows the call and a build command with no preset; only the last two describe the call | code | grep of `*.md` outside `docs/sessions`, `docs/audit`, `.superpowers`, `docs/superpowers/plans` and this plan; `docs/development.md:120-130`, `docs/agents/plan-runs.md:68` | checked 2026-11-24 |
| P15: The judge's tests build real git repositories (`Base.setUp` runs `git init`), write real record lines, and call the real functions | code | `C:\Users\Patrick\.claude\gates\tests\test_plan_run_judge.py:33-110` | checked 2026-11-24 |
| P16: A running session keeps the `hydra_build_pass` tool it started with: `.mcp.json` starts `tools/hydra_ui_mcp.py` as a stdio server from the project folder, the tool is defined once when the server starts, and the file has no reload | code | `.mcp.json:5`, `tools/hydra_ui_mcp.py:957-963`; a grep of the file for `reload` and `importlib` finds none | checked 2026-11-24 |
| P17: The page states the one-build rule and "the gate never gives a third" | code | `docs/agents/plan-runs.md:66-74` | checked 2026-11-24 |
| P18: A plan run works in one repo and reads every task branch there, so a missing branch blocks the build | code | `C:\Users\Patrick\.claude\gates\plan_run.py:399-420`, `C:\Users\Patrick\.claude\gates\plan_run_judge.py:174-178` | checked 2026-11-24 |
| P19: The task table is found by a column headed `Branch`, `Phase` or `Owned files`, so the hooks task is described in prose | code | `C:\Users\Patrick\.claude\gates\plan_run.py:175-187` | checked 2026-11-24 |
| P20: The user ruled "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." | ruling | `docs/agents/rulings/agents-and-workflow.md` line 10, under `## Confirmed` (session e3e9069c) | checked 2026-11-24 |
| P21: The user chose "Allow it" to "May a phase compile a second time for the mutation gate?", with the text "One extra mutation-only build per phase that touches a C++ test. Needs its own reviewed change to the build gate in the hooks repo." | session record, not a filed ruling | The user's answer in a question box in session `b885c6f8` on 2026-11-24, confirmed by the main session in the reviewer's brief. Not under `## Confirmed` in `docs/agents/rulings/` and not in a card | checked 2026-11-24 against the brief only; this plan rests on it as a session record |
| P22: The hooks' plan-run gate reads a marker file by its presence alone | code | `C:\Users\Patrick\.claude\gates\README.md:79` | checked 2026-11-24 |
| P23: The main checkout's `build-cpp\Release\hydra_precheck.exe` was built 2026-11-24 19:25, and no commit of `tools/precheck` since could have put the new mode in it, because the mode does not exist yet | code | file date read with `ls -l` on 2026-11-24 | checked 2026-11-24 |
| P24: PowerShell's `ValidateSet` ignores case by default, and `build_cpp.ps1` records `$Preset` as typed | code | `build_cpp.ps1:36-37`, `build_cpp.ps1:96`; the case rule is PowerShell's documented default and was not run here | unchecked; the plan's exact comparison is the same whichever way it is |
| P25: `Read-BuildSlot` reads only the `pass`, `session` and `expires` lines of a lock | code | `tools/build_slot.ps1:120-131` | checked 2026-11-24 |
| P26: The user's rule for builds outside a run is the typed `allow build`, one build per message, for a session with no open run | code | `docs/agents/plan-runs.md:80`, `C:\Users\Patrick\.claude\gates\percall.py:250-269` | checked 2026-11-24 |
| P27: Test-guard's step M1 runs after its `gate` phase lands, and that phase has a build count of 0; the `scans` phase has a count of 1 and comes before `gate` | code | `docs/superpowers/plans/2026-11-24-test-guard.md:54-58`, `:184-188` | checked 2026-11-24 |
| P28: The gates run git with no time limit of their own | code | `C:\Users\Patrick\.claude\gates\review_rules.py:175-184` | checked 2026-11-24 |
