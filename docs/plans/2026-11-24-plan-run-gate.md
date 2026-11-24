# One build and one merge per plan: the plan-run gate

Status: a plan for review. Nothing here is approved to build.

## The goal, in the user's words

"sessions keep wasting my time with constant merges and builds. memory is not a reliable way to enforce rules. how can i design a hook to enforce one merge and one build at the end of a plan?"

"sometimes a plan takes multiple sessions. how would the hook handle that?"

## The short version

Today a session builds each task's branch, sometimes twice, and merges each one into `main`. A five-task plan costs five merges and five to ten builds.

This plan adds a "plan run": a small record that belongs to an approved plan. While a run is open, its task branches gather on one branch per phase, `plan/<slug>/<phase>`. A hook gives one build pass for that branch, once every task on it is reviewed. A second hook lets `main` take that branch once, after the build passed. The run belongs to the plan, not to a session, so a second session joins the same run and gets no fresh allowance.

The plan also rewrites the instruction text that asks for a build per task, because a gate that refuses what the instructions ask for only produces refusals.

## Why it happens today (the root cause)

The main session's instruction file, `C:\Users\Patrick\.claude\model-postures.md`, says in its sixth paragraph (line 11 of the file): "When an executor returns, check its claims against the code, then build its branch (`hydra_build_pass`, one build at a time) and run its tests, before review." Later in the same paragraph: "After sign-off, rebuild if the reviewer changed code, run the merge's tests, then merge." That is one or two builds and one merge for every task (P1).

Two things in the handoff were wrong and are corrected here. The text is line 11, not "paragraph 11". And `posture.py` does not add it to every turn: it sends it on a session's first prompt, again after 60 minutes, and after a compaction (P2). The effect is the same, because the text is in every session's context from the start.

The same build-per-task order is written in more Hydra pages (P3, P4, P5, P27): agent rule 14 ("A build or test that fails before review goes to a fresh executor on the same branch"), the brief preamble, step 2 of the derive-once review page ("Build the branch and run the merge's tests yourself" before the reviewer is dispatched) and its merge step ("If the reviewer's fixes touch C++, build that tip again"), the integrator page (it builds the joined branch before its review), and the short forms in `CLAUDE.md`. The same `model-postures.md` also tells the main session to merge each blocker after its reviewer signs off (line 7 of the file, "Merge the blocker only after its own reviewer signs off"), which reads as a merge per task. A memory note, "one build per wave", says the opposite, and the instruction text wins because a hook delivers it.

The search for other text that asks for a build or a merge per task covered `model-postures.md`, the three agent type files in `C:\Users\Patrick\.claude\agents\`, every non-archived memory note, `docs/agents/`, `docs/development.md` and the 2026-11-18 tooling audit report. The agent type files (`executor.md`, `reviewer.md`, `planner.md`) hold no instruction to build or merge per task: they say agents never build, which stays true. The notes `one-build-per-wave` and `plan-gates-before-dispatch` are the only memory notes that speak to builds per task; the rest mention the words only in passing. `docs/development.md` has one section that needs a line, "Building under Claude Code" (P28). The 2026-11-18 audit report records the same disagreement, between the postures text and the integrator page, as its SPLIT 1. No filed ruling names both sides, so the user's yes to this plan is what closes it, the way the user's words in the goal ask.

## What cannot work as the starting design stated it

Four parts of the starting design do not survive a read of the code. Each is replaced below.

1. **"One pass, given once every task is stamped" cannot know what gets built or whether it passed.** `hydra_build_pass` takes no argument and sends only the session id and its own process id (P6). The pass is asked for before the build, so no argument could bind it to a result either. The build itself knows more: `build_cpp.ps1` claims the pass with its own checkout folder as the owner, and it is the one place that sees the build end (P7). So the build script must write a build record, and both gates read that record. Without it, "after the one build passed" has no source. Decision: the pass request is judged from the run state, and no argument is added to the tool.
2. **The run file should not store reviewer stamps or a build count.** The signatures already live in `reviews.jsonl`, and `review_rules.check` already answers "is every gated file on this commit signed, as it would land on main" (P10). A second copy in the run file would drift. The run file holds only what nothing else knows: the plan, its branches, its phases and the sessions that joined.
3. **"Main moves once, as a fast-forward" forces the extra merges and builds the user objected to.** If `main` gained a commit after the build, a fast-forward is impossible until `main` is merged into the plan branch, and then the built tree is no longer the tree that lands. The gate instead lets `main` take either the built commit itself, or git's clean merge of old `main` and the built commit. `ref_judge.py` already has that exact test (P12). When that merge conflicts, the session merges `main` into the plan branch and fixes the conflict by hand. That fix is a new commit that needs its own review, so the existing second-build rule below covers it, and no new rule is added. The run's record of each task's files makes such a conflict rarer (see "Files each run owns").
4. **"A session with no run can't move main" would refuse every doc commit.** Session cards, plans, handoffs and rulings land on `main` many times a day and need no build. The review gate already knows the difference: it asks which gated code files a move gains, and a move that gains none needs no review (`_code_changes`, P10). The merge gate asks the same question and judges only a move that gains gated code. That is a read of an existing answer, so it is not a question for the user.

## How it works

### The plan run

A plan that wants a run holds a section headed `## Plan run`. It names the slug, the repo, and one row per phase with its name and its build count (0 or 1). The plan's task table gains two columns: the task's branch and its phase. A new page, `docs/agents/plan-runs.md`, is the one home of that shape. The opener refuses a plan with no such section, a missing column, a branch named twice or a phase with no build count. It fills nothing in.

A phase's own branch is `plan/<slug>/<phase>`, so a plan that ships in two phases has two branches and two builds. A plan with one phase names it `all`. Below, "the plan branch" means the open phase's branch.

Two new modules in the hooks repo share the work, and each answers a different question. `gates\plan_run.py` owns what a run is: it reads the plan's `## Plan run` section and task table, keeps one file per run, `gates\state\runs\<slug>.json`, written with `common.write_json_atomic` (P13), and lists the open runs. `gates\plan_run_judge.py` owns the two decisions about a run: may this session have a build pass, and may `main` take this commit. It reads the run through `plan_run.py` and asks existing owners for everything else (see the Owner map).

The session opens the run itself, with `plan_run.py --open <plan path>`, right after the user says yes to the plan (the user's answer, below). Opening a run cuts the first phase's branch from `main` and records that starting commit. When a phase has landed, `--phase <name>` cuts the next phase's branch from the new `main`.

### Files each run owns

The run file also holds each task's owned files, read from the task table's "Owned files" column. Two checks use them, both at `--open`:

1. Inside one plan, `--open` refuses a plan in which two tasks of the same phase own the same file. Today that rule is only a reviewer's instruction, and three of the five hand-fixed merges on Hydra's `main` since 2026-11-01 came from two pieces of one plan changing the same file (`sp_compare.py` on 11-07, `front.py` on 11-06, `py_mutate.py` on 11-15; read with `git show --remerge-diff` on each merge).
2. Across plans, `--open` prints every other open run that owns one of the same files, with its plan path and the sessions that joined it. It does not refuse. The two sessions then agree between them how their changes merge (the user's answer, below). The start-of-session list shows the same overlaps.

### The build gate

`percall.py` already sees every `hydra_build_pass` call and refuses it to agents (P8). The main-session side gains one check, which asks `plan_run_judge.pass_refusal`. A pass is given only when all of these hold:

1. The session has joined exactly one run as its current run.
2. Every task branch of the current phase is merged into `plan/<slug>/<phase>` (git's own ancestor test).
3. `review_rules.check` says every gated file on `plan/<slug>/<phase>` is signed. If it reports a conflict with `main`, the pass is refused with its text.
4. The phase's build count is 1, and the build records allow one more build.

The build records allow a first build when the phase has none. They allow a second only when the plan branch has gained new commits since the first build and step 3 still passes. That is the existing exception: a fix that fails the build or a test goes to a fresh executor, and a fresh reviewer signs only that fix (P4). The same rule already says "If that fix fails too, the main session stops and tells the user", so a third build is never given by the gate; it needs the user's typed line.

### The build record

`build_cpp.ps1` appends one line when it claims its pass and one when it ends. The lines go to `builds.jsonl` in the repo's git common folder, the same place and pattern as the real-app record (P11). Each line holds the commit, its tree, whether the tree had uncommitted changes, the preset, the targets, and `started`, `passed` or `failed`. A build counts against a run when its commit is on that run's plan branch. Every preset counts the same: a second build with another preset is a second build. The script writes the record for every run, so a build from the user's own terminal counts too; the gate reads only its own commits' lines.

### The merge gate

`ref_judge.py` already asks `review_rules.main_move_refusal` about every move of `main` (P9). It gains a second question, to `plan_run_judge.main_refusal`, asked only in a repo whose `main` holds a `.plan-run-gate` marker file, and only for a move that gains gated code. That is how the two existing gates are switched on per repo (P14), and the review gate's own `_code_changes` answers the second part (task G4 exposes it as one public function, so the merge gate calls it and writes no copy). A move that gains no gated code, such as a session card, a plan or a ruling, is never asked. The move passes when:

1. The new tip is the plan branch's tip, or git's clean merge of old `main` and that tip (P12).
2. The phase's build count is 0, or a `passed` build record with a clean tree exists for that tip.
3. The phase has not landed already, which git answers: the plan branch's tip is not yet contained in `main` (O5).

The existing review check still runs first and unchanged. A clean join of several signed task branches passes it by reading the code: a file one task changed matches its signature exactly, and a file two tasks changed is found by the "carry" walk, which applies each signature with git's own three-way merge until it reaches the landed content (P15). A hand-resolved conflict does not pass and needs its one resolution review, as today. Existing tests already cover this (P16): `test_signed_content.py` joins signed branches in two orders and passes both as "carry", and refuses a hand-repaired conflict by naming the repaired file. No new join test is needed.

When a session asks to move `main` with no run and no typed `allow merge`, `ref_judge.py` has no hook input to read. It takes the session id from `CLAUDE_CODE_SESSION_ID`, as its ruling check already does (P29), and passes it to the one-use allow check (O2).

After the last phase lands, the run is finished. It shows as finished in the start-of-session list until the user closes it.

### Several sessions, several days

A new session joins by naming the plan file's path exactly. Its id is added to the run. Both gates count per run, from git and the build records, so nothing resets. Unfinished tasks wait on their branches.

Only the user closes or abandons a run. The typed lines are `close plan run <slug>` and `abandon plan run <slug>`. `waiver.py` records each the way it records `allow library run` (P17), and `plan_run.py` accepts it only after `review_rules.human_prompt_text` shows the user typed it (O3). The run file itself is never edited by a typed line. Shipping part of a plan early works only when the plan named phases up front; each phase has its own branch, build and merge.

### With no run

A session with no run gets no build pass, and cannot move `main` with a change to gated code, unless the user types a one-off line. `allow build` lets one pass through and `allow merge` lets one move through. Both reuse the one-use allow that already backs `allow library run` and `allow key run` (P18).

### The start-of-session list

A new start-of-session hook prints each open run: its plan, its phase, which task branches are merged into the plan branch, how many builds it has used, which sessions joined, and any file it owns that another open run owns too. A forgotten run shows up every time until the user closes it.

## What you will notice

**Refused today, never before.** A second build of the same plan with no new reviewed fix: "No build pass: plan run `<slug>` has used its build for this phase, and `plan/<slug>/<phase>` has no new reviewed commit since. Instead: read every error in the build log, fix them all in one change, get it reviewed, then ask again."

**Refused today, never before.** A build before the tasks are joined: "No build pass: plan run `<slug>` builds once, after every task is reviewed and merged into `plan/<slug>/<phase>`. Not merged yet: `<branches>`. Not signed: `<files>`."

**Refused today, never before.** A task branch merged straight into `main`: "main was not moved: this commit is not plan run `<slug>`'s branch `plan/<slug>/<phase>`, and no other open run names it. Instead: merge the task into `plan/<slug>/<phase>`; main takes that branch once, after its build passed."

**Refused today, never before.** With no run open: "No build pass: this session has joined no plan run. Instead: join the run for the plan you are working on, or ask the user; they allow one build by sending exactly: allow build".

**Changed text.** Sessions stop saying "building branch X before review". They say which tasks are waiting on `plan/<slug>/<phase>` and that the build comes at the end.

**New at session start.** One short block listing open runs, and any files two open runs both own.

**New when a run opens.** A plan whose tasks in one phase share a file is refused: "plan run not opened: tasks `<A>` and `<B>` of phase `<phase>` both own `<file>`. Instead: give the file to one task, or put the two tasks in different phases." A file another open run owns is named, not refused: "`<file>` is also owned by open plan run `<slug>` (`<plan path>`), joined by `<sessions>`. Agree with that session how the two changes merge."

**The cost.** Reviewers read code that has never been compiled, and a compile error shows up once, at the end, instead of after each task.

## Prior art

GitLab's merge trains queue merge requests and test each one combined with the ones ahead of it, then merge on success ([GitLab docs](https://docs.gitlab.com/ci/pipelines/merge_trains/)). GitHub's merge queue builds a temporary branch holding the queued pull requests and runs CI on that group before the base branch moves ([comparison by Graphite](https://graphite.com/guides/merge-queue-comparison-github-gitlab-graphite)). Batching several changes into one CI run to cut the number of runs is the usual answer for large repos ([The New Stack](https://thenewstack.io/merge-strategies-to-keep-builds-green-in-large-monorepos/); [Rush's merge queue notes](https://rushjs.io/pages/best_practices/merge_queue/)). This plan is the same idea at the size of one plan: an integration branch, one test of the combined tree, then one move of the main branch. Those tools throw a failing change out of the batch and retry; here a failing build gets one reviewed fix and then stops for the user.

## Owner map

This plan changes what the hooks decide, so each question is listed beside the code that already answers it. The planner wrote this map itself from the files it read; no scout ran.

| Question | Owner | Its own words | Status |
|---|---|---|---|
| O1: is every gated file on this commit signed, as it would land on main | `review_rules.check`, `C:\Users\Patrick\.claude\gates\review_rules.py:598` | "would main's code gate pass git's clean merge of main and commit?" | found |
| O2: did the user type this one-use allow, and is it unused | `_allow_refusal`, `C:\Users\Patrick\.claude\gates\percall.py:23` | "None when this session holds an unused allow of this kind that the user typed, and that allow is used up" | found; its phrase tests are `is_library_allow` and `is_key_allow` in `review_rules.py:81-86`, which the new phrases sit beside |
| O3: did the user type this exact message | `human_prompt_text`, `C:\Users\Patrick\.claude\gates\review_rules.py:89` | "The whole text of the transcript's user record with this promptId when the user typed it; else None." | found |
| O4: is this commit a clean merge git made of old main and one other commit | `made_from_existing`, `C:\Users\Patrick\.claude\gates\ref_judge.py:330` | "it is a clean merge of cur and one other parent, so git made it" | found |
| O5: is commit A contained in commit B | `is_ancestor`, `C:\Users\Patrick\.claude\gates\ref_judge.py:96` | none; a one-line call of `git merge-base --is-ancestor` | found |
| O6: which folder built, and when did the build end | `build_cpp.ps1:76` and `build_cpp.ps1:137-140` | "the finally below is what frees the slot" | carry out (task H1 writes the record there) |
| O7: how many builds may run at once, and how long a pass lasts | `tools/build_slot.ps1:75-80` | "This file is the one place that knows the slot rule, the pass rule and their numbers." | found; untouched |
| O8: which plan does this build or merge belong to | none. Searched `C:\Users\Patrick\.claude\gates\*.py` for run, plan and wave state: `premise_evidence.py` reads plans only for their Premise map, and no state folder holds a run | | none; `plan_run.py` is the new owner of what a run is (task G1) and `plan_run_judge.py` of the two decisions about it (task G1b). This is the thing the user asked to have designed |
| O9: how task branches are joined onto one branch | `docs/agents/integrate.md:27` | "Merge each task branch onto the wave branch in the order your brief gives" | found; the plan branch is that wave branch |
| O10: does this move of main gain gated code | `review_rules._code_changes` with `_code_paths`, as `explain_main_move` calls them at `C:\Users\Patrick\.claude\gates\review_rules.py:536-538` | "gated code main gains when it moves from base to new" | found; private, so task G4 adds one public function that wraps exactly that call |
| O11: has this phase already landed on main | O5, `is_ancestor` of the plan branch's tip in `main` | none | found; no "landed" flag is stored |
| O12: which commit and session is the one-use allow for, when no hook input exists | `ref_judge.ruling_reason` reads `CLAUDE_CODE_SESSION_ID` (`ref_judge.py:292`) | none | found; the merge gate does the same and passes it to O2 |

## Tasks

Hook tasks run in worktrees of `C:\Users\Patrick\.claude`, under `C:\Users\Patrick\.claude\worktrees\`, as `gates\README.md` requires (P19). Hydra tasks run in worktrees of the Hydra repo. Each is one executor's job, under 100 tool calls and 20 minutes. Executors run only the Python tests named; none needs a build.

| Id | Owned files | Tests | Depends on | earlyStart |
|---|---|---|---|---|
| G4 | `gates/review_rules.py`, `gates/waiver.py`, `gates/tests/test_review_rules.py`, `gates/tests/test_review_dispatch.py` | those two test files | none | |
| G1 | `gates/plan_run.py`, `gates/tests/test_plan_run.py` | `test_plan_run.py` | G4 | G4 |
| G1b | `gates/plan_run_judge.py`, `gates/tests/test_plan_run_judge.py` | `test_plan_run_judge.py` | G1 | G1 |
| G2 | `gates/percall.py`, `gates/tests/test_percall_key.py`, `gates/tests/test_percall_build_pass.py` (new) | those two test files | G1b | G1b |
| G3 | `gates/ref_judge.py`, `gates/tests/test_ref_judge.py` | `test_ref_judge.py` | G1b | G1b |
| G5 | `gates/plan_runs_start.py`, `gates/tests/test_plan_runs_start.py`, `gates/settings.fragment.json`, `gates/tests/test_settings_fragment.py` | those two test files | G1 | G1 |
| G6 | `model-postures.md`, `gates/README.md` | none (text) | none | |
| H1 | `build_cpp.ps1`, `tools/build_record.ps1`, `tools/test_build_record.ps1` | the new test, `tools/test_build_pass.ps1`, which needs no build | none | |
| H2 | `CLAUDE.md`, `docs/agents/agent-rules.md`, `docs/agents/brief-preamble.md`, `docs/agents/derive-once-review.md`, `docs/agents/integrate.md`, `docs/development.md`, `docs/agents/plan-runs.md` (new), `.plan-run-gate` (new) | `py -I tools/check_pointers.py` | none | |
| M1 | the memory notes (main session, no repo) | none | G6, H2 | |

G4, G6, H1 and H2 start together. G1 starts when G4's executor commits, G1b when G1's, and G2 and G3 together when G1b's; G5 starts with G1b, since it needs only G1. No two tasks that run together own the same file. G2 and G3 both touch the one-use allow (O2): G2 must keep `percall._allow_refusal`'s name and arguments, because G3 calls it as it is. G1 is split from G1b because one executor cannot write the plan reader, the run file, four commands and both decisions with their tests inside 100 calls.

The test files named for the old tasks were wrong (P20): there is no `test_percall.py` or `test_waiver.py`. The pass tool's tests are in `test_percall_key.py` (the agent refusal at line 196 and the main-session pass at line 211, which G2 must change because that call now needs a run or an allow). The waiver tests are the `Waiver` class in `test_review_dispatch.py`, and the phrase tests are in `test_review_rules.py` beside `is_library_allow`.

### G4: the typed lines, and the one public question

Add the exact-match phrase tests in `review_rules.py` beside `is_library_allow`: `allow build`, `allow merge`, `close plan run <slug>`, `abandon plan run <slug>`. `waiver.py` records each the way it records the two existing allows. Add one public function to `review_rules.py`, `gains_gated_code(repo_dir, old, new)`, which makes the same call `explain_main_move` makes at `review_rules.py:536-538` (O10) and returns True when the move gains gated code; `explain_main_move` then calls it too, so the answer has one home.

The join of signed branches needs no new test: `test_signed_content.py` already has it (P16).

Acceptance: each phrase is recorded for the exact text, with the trailing `.` or `!` the existing phrases allow, and not for text with any extra word, which is `test_review_rules.py` and the `Waiver` class in `test_review_dispatch.py` (the existing `allow library run` cases are the model). `gains_gated_code` is False for a move that changes only documents, False in a repo with no code marker, and True for a move that changes a gated file; `explain_main_move`'s existing tests still pass.

### G1: the run owner

Write `plan_run.py`: the reader of a plan's `## Plan run` section and task table, the run file, the commands `--open`, `--join`, `--phase` and `--status`, and `summary`. It calls O3 and O5 and copies neither. Every state function takes a `root`, and every git function a repo folder, so tests use temp folders.

Acceptance, each a test in `test_plan_run.py`:

- `--open` refuses, and writes no file, for each of: no `## Plan run` section, a missing branch, phase or owned-files column, a branch named twice, a phase with no build count, a build count that is not 0 or 1, a run already open for the slug, two tasks of one phase owning the same file.
- `--open` stores each task's owned files, and names (without refusing) each file that another open run owns, with that run's plan path and sessions. `summary` names the same overlaps.
- `--join` refuses a path that is not exactly the open run's plan path, and a second session that joins gets the same counts as the first.
- `--phase` refuses while the earlier phase has not landed in `main`, and cuts the next phase's branch from `main` once it has.
- A run file that is missing a field, or is not JSON, makes every reader raise an error that names the file. Nothing is defaulted.
- A closed or abandoned run (a typed line that O3 confirms) answers like no run; a line O3 cannot confirm changes nothing.

### G1b: the two decisions

Write `plan_run_judge.py` with `pass_refusal` and `main_refusal`. They read the run through `plan_run.py`, and call O1, O4, O5, O10 and the build records, and copy none of them. A missing or broken run file refuses and names the file; nothing is defaulted.

Acceptance, each a test in `test_plan_run_judge.py`:

- `pass_refusal` refuses for each of: no current run, a task branch not merged, an unsigned gated file, a conflict with main, a phase whose count is 0, one build recorded and no new commit, two builds recorded. It allows the first build, and the second after a new signed commit.
- `main_refusal` refuses for each of: a tip no run names, no `passed` record, a `passed` record whose tree was dirty, a `failed` record only, a merge that is not git's clean merge, a phase that already landed. It allows the plan tip itself and the clean merge of old main and the plan tip. It is not asked, and so never refuses, for a move that gains no gated code.
- A run file that is missing a field, or is not JSON, refuses both questions and names the file.

### G2: the build gate

In `percall.py`'s main-session path (`_main_session`, `percall.py:250-263`), a `hydra_build_pass` call asks `plan_run_judge.pass_refusal`; with no run, O2 decides whether a typed `allow build` lets it through. It sits with the other refusals, before `stall`, so a crash denies.

Acceptance, in `test_percall_build_pass.py` and the changed case in `test_percall_key.py`: a main-session pass call with no run and no allow is denied with the "no plan run" text; with a typed allow it passes once and the next is denied; a crash inside `plan_run_judge` denies. An agent's call is still denied with today's text. A call of any other tool never imports `plan_run_judge`.

### G3: the merge gate

In `ref_judge.py`, after `review_reason`, a move of `main` that gains gated code asks `plan_run_judge.main_refusal` when the repo's `main` holds `.plan-run-gate` at either end. A missing or broken `plan_run_judge.py` refuses the move, as a missing `review_rules.py` does. A push is not asked again. With no run it takes the session id from `CLAUDE_CODE_SESSION_ID` (O12) and asks O2 whether a typed `allow merge` lets this one move through.

Acceptance: in a fixture repo with no marker, every existing `test_ref_judge.py` case is unchanged. With the marker: a task branch merged straight into `main` is refused and `restore_checkout` puts the files back; the plan tip with a passed record moves; a move that changes only documents moves with no run and no allow; a typed `allow merge` lets one other code move through and the next is refused; a push to origin is not judged a second time. `ref_judge.py --selftest` still prints its ok line.

### G5: the start-of-session list

Write `plan_runs_start.py`, which prints `plan_run.summary()` as context, and register it in `settings.fragment.json` in the first SessionStart block, the one whose matcher is `startup|resume` (it now holds `cleanup_sweep.ps1`, `session_cards_start.ps1` and Hydra's `check_pointers.py --session-start`; the new entry takes the exe form the README names: `python.exe`, args `-I -S <script>`). It only informs, so it runs in "loud" mode (`common.run(body, "loud")`).

Acceptance: with no open run it prints nothing; with one it names the plan, phase, unmerged branches, builds used and sessions; with two runs that own a common file it names that file under both; a crash prints a `systemMessage` and blocks nothing. `test_settings_fragment.py` gains a test that the registration exists in the fragment. Its existing test that the live `settings.json` carries the fragment fails until `sync_settings.py` has run, so the main session runs that test after the sync, not before (see "Order of going live").

### G6: the instruction text

Rewrite the two sentences quoted under "Why it happens today". New text: "When an executor returns, check its claims against the code, then send its branch to its reviewer. Do not build it. When every task of the phase is signed, merge the branches into `plan/<slug>/<phase>`, ask for the one build pass, build once with every target the tests need, and run the tests. On a failure, read every error and run every test the binaries can still run, then send all of it to one fresh executor and one fresh reviewer, and build once more. If that fails too, stop and tell the user. Then move main once."

Two more sentences change. In line 5, the closing exception ("a reviewer's fix that fails your build goes to a fresh executor and a fresh reviewer reads only that fix") stays, with "your build" read as the phase's build. In line 7, "Merge the blocker only after its own reviewer signs off" and "merge that reviewed blocker into the dependent's branch" gain "into the plan branch" where they mean the phase's join, so they no longer read as a merge to `main`.

Add a paragraph to `gates\README.md` for `plan_run.py`, `plan_run_judge.py`, the two checks, the typed lines (`allow build`, `allow merge`, the close and abandon lines, and the record forms `waiver.py` writes for them) and the new start hook.

Acceptance: `model-postures.md` no longer holds "build its branch" or "rebuild if the reviewer changed code" (a grep the reviewer runs); its line 7 names the plan branch where it says merge; the README names `plan_run.py` as the owner of what a run is and `plan_run_judge.py` as the owner of the two decisions.

### H1: the build record

`build_cpp.ps1` calls a new helper, `tools/build_record.ps1`, once after the claim (line 76) and once in its `finally` (lines 137-140). The helper owns the file name and fields, as `tools/hydra_ui_mcp.py` owns the real-app record's. The fields are the ones named under "The build record", the preset included. The PowerShell tests of this repo sit directly in `tools/` as `test_*.ps1`, so the new test is `tools/test_build_record.ps1`; `tools/test_build_pass.ps1` is the existing test of the claim and must still pass.

Acceptance, in a test that runs the helper against a temp git repo and needs no build: a `started` line then a `passed` line for exit 0; `failed` when the build threw; `dirty` true when the tree has an uncommitted change; a failed write prints an error and does not hide the build's own result. `build_cpp.ps1` is gated code, so it needs its own review like any code change.

### H2: the Hydra pages

Change each place that asks for a build or a merge per task, so the pages and the gate agree:

- `CLAUDE.md` and `docs/agents/agent-rules.md`, rules 5 and 14: "A build or test that fails before review goes to a fresh executor on the same branch" becomes the build at the end of the phase, on the plan branch.
- `docs/agents/brief-preamble.md`, "Tests" and "Builds": "If the build or a test fails before review" becomes "after the phase's one build".
- `docs/agents/derive-once-review.md`, step 2 of the set-up order (line 202): the build moves from before the reviewer's dispatch to after every task of the phase is signed. Its merge step (line 215) says "If the reviewer's fixes touch C++, build that tip again"; that becomes the phase's one allowed second build, after a reviewed fix, and "If that fix fails the build too, stop and tell the user" stays.
- `docs/agents/integrate.md`: the wave branch is the run's `plan/<slug>/<phase>` branch. Its lines 41 and 53 build the joined branch before the review ("Once the branch builds and its scan test passes ... hands ... to a fresh Sonnet reviewer"); the order becomes review first, then the one build, and line 45 (a failed build or scan test goes to a fresh executor on the wave branch) stays as the phase's second build.
- `docs/development.md`, "Building under Claude Code" (lines 112-155): a paragraph that says the pass is given by the plan run, with a pointer to `docs/agents/plan-runs.md`, and that `build_cpp.ps1` records each build (P28).
- New `docs/agents/plan-runs.md`: the shape of `## Plan run` and the task table, the branch name `plan/<slug>/<phase>`, how to open, join and start the next phase, the typed lines.

H2 does not add the `.plan-run-gate` marker. The main session commits it by itself, last, because it is what switches the merge gate on (see "Order of going live").

Acceptance: `check_pointers.py` prints nothing; the reviewer greps `CLAUDE.md`, `agent-rules.md`, `brief-preamble.md`, `derive-once-review.md`, `integrate.md` and `docs/development.md` for "before review", "build the branch", "builds the joined branch" and "build that tip again", and finds no build instruction that still sits before a review.

### M1: the memory notes

The main session updates "one build per wave" and "plan gates before dispatch" to point at the gate and `docs/agents/plan-runs.md`, and fixes the index lines. "One build per wave" already says review first, then one build, and that a failing build is read whole and fixed in one rebuild; it gains the gate's name and loses its own count rules where the gate now enforces them. "Plan gates before dispatch" asks for one real-app check for the src/ui branches; that stays true, because one build of the plan branch is the build that check opens (P21).

## Order of going live

This plan's own work needs no C++ build until the last step, and it cannot use a gate that does not exist yet.

1. The hook branches are reviewed, joined on one branch in the hooks repo, and the main session runs the tests for the changed scripts. G5 changes `settings.fragment.json`, which is shared, so the whole hook suite runs once (P19). Its test that the live `settings.json` carries the fragment is expected to fail here, and passes after step 2.
2. `main` of the hooks repo is fast-forwarded, then `sync_settings.py` runs from the main checkout (P19). The gate is live but does nothing, because no repo holds the marker. The main session reruns the settings test.
3. Hydra's H1 and H2 are reviewed and joined. They hold no C++, so they need no build to merge. `build_cpp.ps1` is gated code, so that move needs its review, and with no marker yet the merge gate does not judge it.
4. The main session commits the `.plan-run-gate` marker to Hydra's `main`. It is not gated code, so the move needs no review and no allow.
5. The two live steps below run.
6. Rolling back: revert the marker commit. That move gains no gated code, so the merge gate does not judge it, and no allow is needed.

## The live steps

### LS1: a pass is refused with no run

The gate is live and Hydra's `main` holds the marker. The main session asks for a pass with no run open. Pass line: the call is denied with the "no plan run" text and one line appears in `refusals.jsonl`.

Calls: `mcp__hydra__hydra_build_pass` (never tried)

### LS2: one real build writes its record

The user types `allow build`. The main session asks for a pass and builds one small target from the main checkout. Pass line: `builds.jsonl` in the git common folder gains a `started` and a `passed` line naming the checkout's commit. Settles P22.

Calls: `mcp__hydra__hydra_build_pass` (a refusal is seen in LS1; a pass handed out is never tried), `build_cpp.ps1 -Target hydra_precheck` (never tried)

## The user's answers

The user answered the plan's three questions in session 974771b7-864b-4140-b29d-828363c85d7f on 2026-11-24. None is a filed ruling.

1. **Who opens a run?** The user chose "Session, after my yes". The session runs `--open` itself after the user's yes to the plan; no typed line opens a run.
2. **What if main moved after the build and git cannot merge cleanly?** The user asked why such merges fail, and then said: "so if the new gate already keeps a record, then that covers it. if theres ever a situation where two sessions are working on the same files at the same time, they can coordinate how to merge their changes." So the run records each task's files ("Files each run owns"), overlaps inside a plan are refused, overlaps across plans are named so the sessions coordinate, and a hand-fixed merge goes through the existing reviewed second build. No other rule is added.
3. **Small edits you ask for directly.** The user chose "Yes, I'll type allow". With no plan run open, a code edit needs the user's typed `allow merge`, and `allow build` if it needs compiling.

Two questions the first draft asked are answered by the code and were not put to the user. Whether commits that change no code stay free is read from the review gate's own answer about gated code (O10). Whether the hooks repo is gated too is a scope line below.

## Out of scope, for the user to approve

- The gate does not know whether tests passed. It reads only the build's result. Running the tests stays an instruction.
- The gate does not judge which targets or preset a build names. One call of `build_cpp.ps1` with several targets is one build (P7), and a second build with another preset is a second build.
- A pass that runs out unclaimed after 120 seconds is not counted, because only a started build writes a record.
- Builds from the user's own terminal and from CI are not gated, as today (P24). A terminal build writes the same record, so it counts as the phase's build if it is of a commit on the plan branch.
- The hooks repo is not gated. It has no build, and its merges are already fast-forward only (P19). A broken gate there could block its own repair. Say if you want it gated.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| The opener refuses a plan with no task table and fills nothing in | "The plan run" | G1 | `test_plan_run.py`: each malformed plan refuses and writes no file |
| One build per phase; a second only after a new signed commit; never a third | "The build gate" | G1b | `test_plan_run_judge.py`: the seven refusing cases and two allowing cases of `pass_refusal` |
| No pass before every task is merged and signed | "The build gate" | G1b | same file: unmerged branch, unsigned file |
| The hook refuses a main-session pass with no run | "With no run" | G2 | `test_percall_build_pass.py`: denied; LS1 |
| Agents are still refused a pass | G2 acceptance | G2 | `test_percall_key.py`: today's text unchanged |
| A typed `allow build` or `allow merge` lets exactly one through | "With no run" | G2, G3, G4 | allow passes once, next denied, extra word not recorded |
| Main takes only the plan tip or its clean merge with old main | "The merge gate" | G1b, G3 | `main_refusal` cases; `test_ref_judge.py` marker cases |
| Main needs a passed, clean-tree build record when the phase builds | "The merge gate" | G1b | no record, dirty record, failed record all refuse |
| A phase lands once | "The merge gate" | G1b | second landing refused |
| A clean join of signed branches passes the review check; a hand fix does not | "The merge gate" | none new | the existing `CarriedByGit` tests (P16) |
| A move that gains no gated code is never judged by the merge gate | "The merge gate" | G4, G1b, G3 | `gains_gated_code` cases; `main_refusal` not asked; a document-only move passes in `test_ref_judge.py` |
| A next phase starts only after the earlier one landed | "The plan run" | G1 | `--phase` refuses before landing |
| A repo with no marker behaves as today | G3 | G3 | existing `test_ref_judge.py` cases unchanged |
| A broken or missing `plan_run_judge.py`, or a bad run file, refuses | G1, G1b, G3 | G1, G1b, G3 | bad-file tests; missing-module test |
| Two tasks of one phase never own the same file; a file another open run owns is named, not refused | "Files each run owns" | G1, G5 | `test_plan_run.py` overlap cases; `test_plan_runs_start.py` two-run case |
| A second session gets no fresh allowance | "Several sessions" | G1 | join test: same counts |
| Only the user closes or abandons a run | "Several sessions" | G4, G1 | the phrase tests; a close line `human_prompt_text` does not confirm changes nothing |
| The build writes started, passed or failed, dirty and its preset | "The build record" | H1 | `tools/test_build_record.ps1`; LS2 |
| The start list shows open runs and blocks nothing | G5 | G5 | `test_plan_runs_start.py` |
| The instruction text no longer asks for a build or a merge to main per task | G6, H2 | G6, H2 | the reviewer's greps named in each task, including `model-postures.md` lines 7 and 11, `integrate.md` and `docs/development.md` |
| Hook changes follow the hooks README | "Order of going live" | steps 1 and 2 | the main session's test run and `sync_settings.py --check` |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: the main session's instructions ask for a build before review, a rebuild after sign-off and a merge per task | code | `C:\Users\Patrick\.claude\model-postures.md:7` and `:11` | checked 2026-11-24 |
| P2: the posture is sent on the first prompt, after 60 minutes and after a compaction, not every turn | code | `C:\Users\Patrick\.claude\gates\posture.py:33-51` | checked 2026-11-24 |
| P3: agent rule 14 sends a pre-review build failure to a fresh executor | code | `docs/agents/agent-rules.md:20` | checked 2026-11-24 |
| P4: a reviewer's fix that fails the build goes to a fresh executor and reviewer, and a second failure stops for the user | code | `docs/agents/agent-rules.md:11` | checked 2026-11-24 |
| P5: the preamble and the review page put the build before the review | code | `docs/agents/brief-preamble.md:53-59` and `docs/agents/derive-once-review.md:202` | checked 2026-11-24 |
| P6: `build_pass()` takes no argument and sends only the session id and its process id | code | `tools/hydra_ui_mcp.py:798-818` | checked 2026-11-24 |
| P7: `build_cpp.ps1` claims the pass with its own folder as owner, releases it in a `finally`, and builds several targets in one call | code | `build_cpp.ps1:7`, `build_cpp.ps1:76` and `build_cpp.ps1:137-140` | checked 2026-11-24 |
| P8: `percall.py` refuses the pass tool to agents by tool name, and the main-session path has no build check | code | `C:\Users\Patrick\.claude\gates\percall.py:20`, `C:\Users\Patrick\.claude\gates\percall.py:231-234` and `C:\Users\Patrick\.claude\gates\percall.py:250-263` | checked 2026-11-24 |
| P9: every move of main is handed to `review_rules.main_move_refusal` | code | `C:\Users\Patrick\.claude\gates\ref_judge.py:260-266` and `C:\Users\Patrick\.claude\gates\ref_judge.py:314-327` | checked 2026-11-24 |
| P10: `check` judges git's clean merge of main and a commit, and reports a conflict | code | `C:\Users\Patrick\.claude\gates\review_rules.py:598-620` | checked 2026-11-24 |
| P11: the real-app record is a jsonl file in the git common folder that the gate only reads | code | `C:\Users\Patrick\.claude\gates\README.md:71` | checked 2026-11-24 |
| P12: `made_from_existing` tests for a clean merge of the current tip and one other parent | code | `C:\Users\Patrick\.claude\gates\ref_judge.py:330-341` | checked 2026-11-24 |
| P13: state files are written atomically by `write_json_atomic` | code | `C:\Users\Patrick\.claude\gates\common.py:90-102` | checked 2026-11-24 |
| P14: a gate is switched on per repo by a marker file read at either end of the move | code | `C:\Users\Patrick\.claude\gates\review_rules.py:18` and `C:\Users\Patrick\.claude\gates\review_rules.py:524-527` | checked 2026-11-24 |
| P15: a file is signed by an exact signature, or by a chain of signatures applied with git's three-way merge | code | `C:\Users\Patrick\.claude\gates\review_rules.py:289-325` and `C:\Users\Patrick\.claude\gates\review_rules.py:328-347` | checked 2026-11-24 |
| P16: tests already join signed branches and judge the move of main to the join: two orders pass as "carry", a hand-repaired conflict is refused naming its file | code | `C:\Users\Patrick\.claude\gates\tests\test_signed_content.py`, class `CarriedByGit` (`test_a_joined_wave_passes_in_one_order`, `..._in_the_other_order`, `test_a_hand_repaired_conflict_names_only_the_repaired_file`), and `test_content_stamp_join.py:116` | checked 2026-11-24 |
| P17: `waiver.py` records a typed allow with its prompt id, and the check that the user typed it runs later | code | `C:\Users\Patrick\.claude\gates\waiver.py:16-46` | checked 2026-11-24 |
| P18: one typed allow lets one call through and is then marked used | code | `C:\Users\Patrick\.claude\gates\percall.py:23-42` | checked 2026-11-24 |
| P19: hook work goes in a worktree, gets one reviewer, runs the changed scripts' tests (all of them for shared files), merges fast-forward and then syncs; the user's terminal is not gated | code | `C:\Users\Patrick\.claude\gates\README.md:150-180` | checked 2026-11-24 |
| P20: the tests are `test_ref_judge.py` for `ref_judge.py`; `test_percall_key.py` (lines 196 and 211) for the pass tool; the `Waiver` class in `test_review_dispatch.py` and `test_review_rules.py:1280-1307` for `waiver.py` and the typed phrases; there is no `test_percall.py` or `test_waiver.py` | code | `C:\Users\Patrick\.claude\gates\tests` listed and searched | checked 2026-11-24 |
| P21: the memory note "plan gates before dispatch" asks for one real-app check for the src/ui branches and counting builds before dispatch; the note "one build per wave" already says review first, then one build | code | `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory\plan-gates-before-dispatch.md:11` and `one-build-per-wave.md:11-17`, and `C:\Users\Patrick\.claude\gates\tests\test_review_rules.py:774` (a record on a joined branch covers another branch with the same UI files) | checked 2026-11-24 |
| P22: a real build under Claude Code reaches the new record lines | unchecked | `build_cpp.ps1`, where the lines do not exist yet | LS2; blocks nothing |
| P23: a conflict after the build is handled by the run's file record, coordination between sessions and the existing reviewed second build | user's words | session 974771b7-864b-4140-b29d-828363c85d7f, 2026-11-24, quoted under "The user's answers"; not a filed ruling | answered; blocks nothing |
| P33: three of the five hand-fixed merges on `main` since 2026-11-01 were two pieces of one plan changing the same file | code | `git show --remerge-diff` of 882e915ba, 8ced4ed26, e2c54cb4e (overlap inside one plan), 4427e76fc and faf63254c (across sessions, or a week-long branch) | checked 2026-11-24 |
| P24: outside Claude Code nothing gates a build | code | `tools/build_slot.ps1:5-8` and `tools/build_slot.ps1:90` | checked 2026-11-24 |
| P25: the user's limit is three builds at once, main session only | ruling | `docs/agents/rulings/agents-and-workflow.md`, the 2026-11-07 line under Confirmed | checked 2026-11-24 |
| P26: an integrator already joins task branches onto one wave branch | code | `docs/agents/integrate.md:27` and `docs/agents/integrate.md:41` | checked 2026-11-24 |
| P27: the integrator page and the review page's merge step put a build before a review | code | `docs/agents/integrate.md:41` and `:53`, `docs/agents/derive-once-review.md:215` | checked 2026-11-24 |
| P28: `docs/development.md` has the one section on how a build pass is asked for, and nothing in it says a pass needs a plan run | code | `docs/development.md:112-155` | checked 2026-11-24 |
| P29: `ref_judge.py` reads the session id from `CLAUDE_CODE_SESSION_ID` where it has no hook input | code | `C:\Users\Patrick\.claude\gates\ref_judge.py:292` | checked 2026-11-24 |
| P30: the typed phrases and their tests live in `review_rules.py`, and `waiver.py` records what they accept | code | `C:\Users\Patrick\.claude\gates\review_rules.py:26-28`, `:51-55`, `:81-86` and `C:\Users\Patrick\.claude\gates\waiver.py:16-27` | checked 2026-11-24 |
| P31: `settings.fragment.json` holds a SessionStart block with matcher `startup|resume` with three entries, and `test_settings_fragment.py` holds a test that the live settings carry the fragment | code | `C:\Users\Patrick\.claude\gates\settings.fragment.json:68-108` and `C:\Users\Patrick\.claude\gates\tests\test_settings_fragment.py:41` | checked 2026-11-24 |
| P32: a pass is asked for through `tools/build_slot.ps1 -NewPass`, and Hydra's PowerShell tests are `tools/test_*.ps1`, with no `tools/tests` folder | code | `tools/hydra_ui_mcp.py:808` and the `tools/` listing | checked 2026-11-24 |
