# One build and one merge per plan: the plan-run gate

Status: a plan for review. Nothing here is approved to build.

## The goal, in the user's words

"sessions keep wasting my time with constant merges and builds. memory is not a reliable way to enforce rules. how can i design a hook to enforce one merge and one build at the end of a plan?"

"sometimes a plan takes multiple sessions. how would the hook handle that?"

## The short version

Today a session builds each task's branch, sometimes twice, and merges each one into `main`. A five-task plan costs five merges and five to ten builds.

This plan adds a "plan run": a small record that belongs to an approved plan. While a run is open, its task branches gather on one branch, `plan/<slug>`. A hook gives one build pass for that branch, once every task on it is reviewed. A second hook lets `main` take that branch once, after the build passed. The run belongs to the plan, not to a session, so a second session joins the same run and gets no fresh allowance.

The plan also rewrites the instruction text that asks for a build per task, because a gate that refuses what the instructions ask for only produces refusals.

## Why it happens today (the root cause)

The main session's instruction file, `C:\Users\Patrick\.claude\model-postures.md`, says in its sixth paragraph (line 11 of the file): "When an executor returns, check its claims against the code, then build its branch (`hydra_build_pass`, one build at a time) and run its tests, before review." Later in the same paragraph: "After sign-off, rebuild if the reviewer changed code, run the merge's tests, then merge." That is one or two builds and one merge for every task (P1).

Two things in the handoff were wrong and are corrected here. The text is line 11, not "paragraph 11". And `posture.py` does not add it to every turn: it sends it on a session's first prompt, again after 60 minutes, and after a compaction (P2). The effect is the same, because the text is in every session's context from the start.

The same build-per-task order is written in four Hydra pages (P3, P4, P5): agent rule 14 ("A build or test that fails before review goes to a fresh executor on the same branch"), the brief preamble, step 2 of the derive-once review page ("Build the branch and run the merge's tests yourself" before the reviewer is dispatched), and the short forms in `CLAUDE.md`. A memory note, "one build per wave", says the opposite, and the instruction text wins because a hook delivers it.

## What cannot work as the starting design stated it

Four parts of the starting design do not survive a read of the code. Each is replaced below.

1. **"One pass, given once every task is stamped" cannot know what gets built or whether it passed.** `hydra_build_pass` takes no argument and sends only the session id and its own process id (P6). The pass is asked for before the build, so no argument could bind it to a result either. The build itself knows more: `build_cpp.ps1` claims the pass with its own checkout folder as the owner, and it is the one place that sees the build end (P7). So the build script must write a build record, and both gates read that record. Without it, "after the one build passed" has no source. Decision: the pass request is judged from the run state, and no argument is added to the tool.
2. **The run file should not store reviewer stamps or a build count.** The signatures already live in `reviews.jsonl`, and `review_rules.check` already answers "is every gated file on this commit signed, as it would land on main" (P10). A second copy in the run file would drift. The run file holds only what nothing else knows: the plan, its branches, its phases and the sessions that joined.
3. **"Main moves once, as a fast-forward" forces the extra merges and builds the user objected to.** If `main` gained a commit after the build, a fast-forward is impossible until `main` is merged into the plan branch, and then the built tree is no longer the tree that lands. The gate instead lets `main` take either the built commit itself, or git's clean merge of old `main` and the built commit. `ref_judge.py` already has that exact test (P12). What happens when that merge conflicts is question 3.
4. **"A session with no run can't move main" would refuse every doc commit.** Session cards, plans, handoffs and rulings land on `main` many times a day and need no build. Whether those stay free is question 1.

## How it works

### The plan run

A plan that wants a run holds a section headed `## Plan run`. It names the slug, the repo, and one row per phase with its build count (0 or 1). The plan's task table gains two columns: the task's branch and its phase. A new page, `docs/agents/plan-runs.md`, is the one home of that shape. The opener refuses a plan with no such section, a missing column, a branch named twice or a phase with no build count. It fills nothing in.

A new module in the hooks repo, `gates\plan_run.py`, is the one owner of runs. It keeps one file per run, `gates\state\runs\<slug>.json`, written with `common.write_json_atomic` (P13). It answers three questions and nothing else: may this session have a build pass, may `main` take this commit, and what runs are open. Everything else it needs it asks an existing owner for (see the Owner map).

Opening a run cuts the phase's branch, `plan/<slug>`, from `main`, and records that starting commit. How a run is opened, by the session or by a line the user types, is question 2.

### The build gate

`percall.py` already sees every `hydra_build_pass` call and refuses it to agents (P8). The main-session side gains one check, which asks `plan_run.pass_refusal`. A pass is given only when all of these hold:

1. The session has joined exactly one run as its current run.
2. Every task branch of the current phase is merged into `plan/<slug>` (git's own ancestor test).
3. `review_rules.check` says every gated file on `plan/<slug>` is signed. If it reports a conflict with `main`, the pass is refused with its text.
4. The phase's build count is 1, and the build records allow one more build.

The build records allow a first build when the phase has none. They allow a second only when the plan branch has gained new commits since the first build and step 3 still passes. That is the existing exception: a fix that fails the build or a test goes to a fresh executor, and a fresh reviewer signs only that fix (P4). The same rule already says "If that fix fails too, the main session stops and tells the user", so a third build is never given by the gate; it needs the user's typed line.

### The build record

`build_cpp.ps1` appends one line when it claims its pass and one when it ends. The lines go to `builds.jsonl` in the repo's git common folder, the same place and pattern as the real-app record (P11). Each line holds the commit, its tree, whether the tree had uncommitted changes, the targets, and `started`, `passed` or `failed`. A build counts against a run when its commit is on that run's plan branch.

### The merge gate

`ref_judge.py` already asks `review_rules.main_move_refusal` about every move of `main` (P9). It gains a second question, to `plan_run.main_refusal`, asked only in a repo whose `main` holds a `.plan-run-gate` marker file. That is how the two existing gates are switched on per repo (P14). The move passes when:

1. The new tip is the plan branch's tip, or git's clean merge of old `main` and that tip (P12).
2. The phase's build count is 0, or a `passed` build record with a clean tree exists for that tip.
3. The phase has not landed already.

The existing review check still runs first and unchanged. A clean join of several signed task branches passes it by reading the code: a file one task changed matches its signature exactly, and a file two tasks changed is found by the "carry" walk, which applies each signature with git's own three-way merge until it reaches the landed content (P15). A hand-resolved conflict does not pass and needs its one resolution review, as today. No test covers the join of two signed branches followed by a move of `main` to a plan branch (P16), so task G4 adds it.

After the last phase lands, the run is finished. It shows as finished in the start-of-session list until the user closes it.

### Several sessions, several days

A new session joins by naming the plan file's path exactly. Its id is added to the run. Both gates count per run, from git and the build records, so nothing resets. Unfinished tasks wait on their branches.

Only the user closes or abandons a run, with a typed line that `waiver.py` records the way it records `allow library run` (P17). Shipping part of a plan early works only when the plan named phases up front; each phase has its own branch, build and merge.

### With no run

A session with no run gets no build pass, and cannot move `main` in a way the gate judges, unless the user types a one-off line. `allow build` lets one pass through and `allow merge` lets one move through. Both reuse the one-use allow that already backs `allow library run` and `allow key run` (P18).

### The start-of-session list

A new start-of-session hook prints each open run: its plan, its phase, which task branches are merged into the plan branch, how many builds it has used, and which sessions joined. A forgotten run shows up every time until the user closes it.

## What you will notice

**Refused today, never before.** A second build of the same plan with no new reviewed fix: "No build pass: plan run `<slug>` has used its build for this phase, and `plan/<slug>` has no new reviewed commit since. Instead: read every error in the build log, fix them all in one change, get it reviewed, then ask again."

**Refused today, never before.** A build before the tasks are joined: "No build pass: plan run `<slug>` builds once, after every task is reviewed and merged into `plan/<slug>`. Not merged yet: `<branches>`. Not signed: `<files>`."

**Refused today, never before.** A task branch merged straight into `main`: "main was not moved: this commit is not plan run `<slug>`'s branch `plan/<slug>`, and no other open run names it. Instead: merge the task into `plan/<slug>`; main takes that branch once, after its build passed."

**Refused today, never before.** With no run open: "No build pass: this session has joined no plan run. Instead: join the run for the plan you are working on, or ask the user; they allow one build by sending exactly: allow build".

**Changed text.** Sessions stop saying "building branch X before review". They say which tasks are waiting on `plan/<slug>` and that the build comes at the end.

**New at session start.** One short block listing open runs.

**The cost.** Reviewers read code that has never been compiled, and a compile error shows up once, at the end, instead of after each task.

## Prior art

GitLab's merge trains queue merge requests and test each one combined with the ones ahead of it, then merge on success ([GitLab docs](https://docs.gitlab.com/ci/pipelines/merge_trains/)). GitHub's merge queue builds a temporary branch holding the queued pull requests and runs CI on that group before the base branch moves ([comparison by Graphite](https://graphite.com/guides/merge-queue-comparison-github-gitlab-graphite)). Batching several changes into one CI run to cut the number of runs is the usual answer for large repos ([The New Stack](https://thenewstack.io/merge-strategies-to-keep-builds-green-in-large-monorepos/); [Rush's merge queue notes](https://rushjs.io/pages/best_practices/merge_queue/)). This plan is the same idea at the size of one plan: an integration branch, one test of the combined tree, then one move of the main branch. Those tools throw a failing change out of the batch and retry; here a failing build gets one reviewed fix and then stops for the user.

## Owner map

This plan changes what the hooks decide, so each question is listed beside the code that already answers it. The planner wrote this map itself from the files it read; no scout ran.

| Question | Owner | Its own words | Status |
|---|---|---|---|
| O1: is every gated file on this commit signed, as it would land on main | `review_rules.check`, `C:\Users\Patrick\.claude\gates\review_rules.py:598` | "would main's code gate pass git's clean merge of main and commit?" | found |
| O2: did the user type this one-use allow, and is it unused | `_allow_refusal`, `C:\Users\Patrick\.claude\gates\percall.py:23` | "None when this session holds an unused allow of this kind that the user typed, and that allow is used up" | found |
| O3: did the user type this exact message | `human_prompt_text`, `C:\Users\Patrick\.claude\gates\review_rules.py:89` | "The whole text of the transcript's user record with this promptId when the user typed it; else None." | found |
| O4: is this commit a clean merge git made of old main and one other commit | `made_from_existing`, `C:\Users\Patrick\.claude\gates\ref_judge.py:330` | "it is a clean merge of cur and one other parent, so git made it" | found |
| O5: is commit A contained in commit B | `is_ancestor`, `C:\Users\Patrick\.claude\gates\ref_judge.py:96` | none; a one-line call of `git merge-base --is-ancestor` | found |
| O6: which folder built, and when did the build end | `build_cpp.ps1:76` and `build_cpp.ps1:137-140` | "the finally below is what frees the slot" | carry out (task H1 writes the record there) |
| O7: how many builds may run at once, and how long a pass lasts | `tools/build_slot.ps1:75-80` | "This file is the one place that knows the slot rule, the pass rule and their numbers." | found; untouched |
| O8: which plan does this build or merge belong to | none. Searched `C:\Users\Patrick\.claude\gates\*.py` for run, plan and wave state: `premise_evidence.py` reads plans only for their Premise map, and no state folder holds a run | | none; `plan_run.py` is the new owner (task G1). This is the thing the user asked to have designed |
| O9: how task branches are joined onto one branch | `docs/agents/integrate.md:27` | "Merge each task branch onto the wave branch in the order your brief gives" | found; the plan branch is that wave branch |

## Tasks

Hook tasks run in worktrees of `C:\Users\Patrick\.claude`, under `C:\Users\Patrick\.claude\worktrees\`, as `gates\README.md` requires (P19). Hydra tasks run in worktrees of the Hydra repo. Each is one executor's job, under 100 tool calls and 20 minutes. Executors run only the Python tests named; none needs a build.

| Id | Owned files | Tests | Depends on | earlyStart |
|---|---|---|---|---|
| G4 | `gates/review_rules.py`, `gates/waiver.py`, `gates/tests/test_review_rules.py`, `gates/tests/test_waiver.py` | those two test files | none | |
| G1 | `gates/plan_run.py`, `gates/tests/test_plan_run.py` | `test_plan_run.py` | G4 | G4 |
| G2 | `gates/percall.py`, `gates/tests/test_percall.py` | `test_percall.py` | G1 | G1 |
| G3 | `gates/ref_judge.py`, `gates/tests/test_ref_judge.py` | `test_ref_judge.py` | G1 | G1 |
| G5 | `gates/plan_runs_start.py`, `gates/tests/test_plan_runs_start.py`, `gates/settings.fragment.json`, `gates/tests/test_settings_fragment.py` | those two test files | G1 | G1 |
| G6 | `model-postures.md`, `gates/README.md` | none (text) | none | |
| H1 | `build_cpp.ps1`, `tools/build_record.ps1`, `tools/tests/test_build_record.ps1` | the new test, which needs no build | none | |
| H2 | `CLAUDE.md`, `docs/agents/agent-rules.md`, `docs/agents/brief-preamble.md`, `docs/agents/derive-once-review.md`, `docs/agents/integrate.md`, `docs/agents/plan-runs.md` (new), `.plan-run-gate` (new) | `py -I tools/check_pointers.py` | none | |
| M1 | the memory notes (main session, no repo) | none | G6, H2 | |

G4, G6, H1 and H2 start together. G1 starts when G4's executor commits; G2, G3 and G5 start together when G1's executor commits. The test file names for `percall.py`, `ref_judge.py` and `waiver.py` are the planner's guess at existing names (P20); each executor uses the test file that already covers its script and says so.

### G4: the typed lines, and the missing join test

Add the exact-match tests for the new typed lines beside `is_library_allow`: `allow build`, `allow merge`, and the lines that close or abandon a run (and the line that opens one, if question 2 says so). `waiver.py` records them the way it records the two existing allows.

Add the test P16 says is missing. Two branches each change a different gated file and one file in common, on different lines; each gets `signed` records; a third branch merges both cleanly; `main` moves to it.

Acceptance: `main_move_refusal` returns None for that move. It returns a refusal naming the file when the join's content differs by one byte from git's merge. It returns a refusal when the same join is resolved by hand. An allow line with any extra word is not recorded. If the first check fails, the executor stops and reports: the design's merge gate then needs a change to `_file_signed`, which is its own question.

### G1: the run owner

Write `plan_run.py`: the reader of a plan's `## Plan run` section and task table, the run file, the commands `--open`, `--join` and `--status`, and the three answers `pass_refusal`, `main_refusal` and `summary`. It calls O1, O3, O4 and O5 and copies none of them. Every state function takes a `root`, and every git function a repo folder, so tests use temp folders.

Acceptance, each a test in `test_plan_run.py`:

- `--open` refuses, and writes no file, for each of: no `## Plan run` section, a missing branch or phase column, a branch named twice, a phase with no build count, a build count that is not 0 or 1, a run already open for the slug.
- `--join` refuses a path that is not exactly the open run's plan path, and a second session that joins gets the same counts as the first.
- `pass_refusal` refuses for each of: no current run, a task branch not merged, an unsigned gated file, a conflict with main, a phase whose count is 0, one build recorded and no new commit, two builds recorded. It allows the first build, and the second after a new signed commit.
- `main_refusal` refuses for each of: a tip no run names, no `passed` record, a `passed` record whose tree was dirty, a `failed` record only, a merge that is not git's clean merge, a phase that already landed. It allows the plan tip itself and the clean merge of old main and the plan tip.
- A run file that is missing a field, or is not JSON, refuses both questions and names the file. Nothing is defaulted.
- A closed or abandoned run answers like no run.

### G2: the build gate

In `percall.py`'s main-session path, a `hydra_build_pass` call asks `plan_run.pass_refusal`; with no run, O2 decides whether a typed `allow build` lets it through. It sits with the other refusals, before `stall`, so a crash denies.

Acceptance: a main-session pass call with no run and no allow is denied with the "no plan run" text; with a typed allow it passes once and the next is denied; a crash inside `plan_run` denies. An agent's call is still denied with today's text. A call of any other tool never imports `plan_run`.

### G3: the merge gate

In `ref_judge.py`, after `review_reason`, a move of `main` asks `plan_run.main_refusal` when the repo's `main` holds `.plan-run-gate` at either end. A missing or broken `plan_run.py` refuses the move, as a missing `review_rules.py` does. A push is not asked again.

Acceptance: in a fixture repo with no marker, every existing `test_ref_judge.py` case is unchanged. With the marker: a task branch merged straight into `main` is refused and `restore_checkout` puts the files back; the plan tip with a passed record moves; a typed `allow merge` lets one other move through and the next is refused. `ref_judge.py --selftest` still prints its ok line.

### G5: the start-of-session list

Write `plan_runs_start.py`, which prints `plan_run.summary()` as context, and register it in `settings.fragment.json` under the `startup|resume` SessionStart block. It only informs, so it runs in "loud" mode.

Acceptance: with no open run it prints nothing; with one it names the plan, phase, unmerged branches, builds used and sessions; a crash prints a `systemMessage` and blocks nothing. `test_settings_fragment.py` checks the registration exists.

### G6: the instruction text

Rewrite the two sentences quoted under "Why it happens today". New text: "When an executor returns, check its claims against the code, then send its branch to its reviewer. Do not build it. When every task of the phase is signed, merge the branches into `plan/<slug>`, ask for the one build pass, build once with every target the tests need, and run the tests. On a failure, read every error and run every test the binaries can still run, then send all of it to one fresh executor and one fresh reviewer, and build once more. If that fails too, stop and tell the user. Then move main once." Add a paragraph to `gates\README.md` for `plan_run.py`, the two checks and the new start hook.

Acceptance: `model-postures.md` no longer holds "build its branch" or "rebuild if the reviewer changed code" (a grep the reviewer runs); the README names `plan_run.py` as the owner of runs.

### H1: the build record

`build_cpp.ps1` calls a new helper, `tools/build_record.ps1`, once after the claim and once in its `finally`. The helper owns the file name and fields, as `tools/hydra_ui_mcp.py` owns the real-app record's.

Acceptance, in a test that runs the helper against a temp git repo and needs no build: a `started` line then a `passed` line for exit 0; `failed` when the build threw; `dirty` true when the tree has an uncommitted change; a failed write prints an error and does not hide the build's own result.

### H2: the Hydra pages

Change each place that asks for a build or a merge per task, so the pages and the gate agree:

- `CLAUDE.md` and `docs/agents/agent-rules.md`, rules 5 and 14: "A build or test that fails before review goes to a fresh executor on the same branch" becomes the build at the end of the phase, on the plan branch.
- `docs/agents/brief-preamble.md`, "Tests" and "Builds": "If the build or a test fails before review" becomes "after the phase's one build".
- `docs/agents/derive-once-review.md`, step 2 of the set-up order: the build moves from before the reviewer's dispatch to after every task of the phase is signed.
- `docs/agents/integrate.md`: the wave branch is the run's `plan/<slug>` branch; its "builds the joined branch once" stays.
- New `docs/agents/plan-runs.md`: the shape of `## Plan run` and the task table, how to open and join, the typed lines.
- New `.plan-run-gate` marker. This file is what switches the merge gate on, so it lands last (see "Order of going live").

Acceptance: `check_pointers.py` prints nothing; the reviewer greps the five pages for "before review" and finds no build instruction left.

### M1: the memory notes

The main session updates "one build per wave" and "plan gates before dispatch" to point at the gate and `docs/agents/plan-runs.md`, and fixes the index lines. The second note was not opened for this plan (P21).

## Order of going live

This plan's own work needs no C++ build until the last step, and it cannot use a gate that does not exist yet.

1. The hook branches are reviewed, joined on one branch in the hooks repo, and the main session runs the tests for the changed scripts. G5 changes `settings.fragment.json`, which is shared, so the whole hook suite runs once (P19).
2. `main` of the hooks repo is fast-forwarded, then `sync_settings.py` runs from the main checkout (P19). The gate is live but does nothing, because no repo holds the marker.
3. Hydra's H1 and H2 are reviewed and joined. They hold no C++, so they need no build to merge.
4. The two live steps below run.
5. Rolling back: revert the marker commit. That move of Hydra's `main` is judged by the gate, so the user types `allow merge`, or makes the revert from their own terminal, which no gate reads (P19).

## The live steps

### LS1: a pass is refused with no run

The gate is live and Hydra's `main` holds the marker. The main session asks for a pass with no run open. Pass line: the call is denied with the "no plan run" text and one line appears in `refusals.jsonl`.

Calls: `mcp__hydra__hydra_build_pass` (never tried)

### LS2: one real build writes its record

The user types `allow build`. The main session asks for a pass and builds one small target from the main checkout. Pass line: `builds.jsonl` in the git common folder gains a `started` and a `passed` line naming the checkout's commit. Settles P22.

Calls: `mcp__hydra__hydra_build_pass` (seen: LS1's passing run), `build_cpp.ps1 -Target hydra_precheck` (never tried)

## Questions for the user

1. **Should commits that change no code stay free?** Session cards, plans, handoffs and rulings land on `main` many times a day and need no build. One reading: the gate judges only a move of `main` that changes code (the same files that already need a review). The other: every move of `main` needs a run or a typed `allow merge`, so each batch of session cards would need a line from you. G1 and G3 wait for this answer.
2. **Who opens a run?** Either the session opens it after your yes to a plan, or it opens only when you type a line such as `start plan <slug>`. The first costs you nothing but trusts the session to open the right plan. The second means one more typed line per plan, and no session can open a run you did not ask for. G4 and G1 wait for this answer.
3. **What if main moved after the build and git cannot merge cleanly?** A clean merge lands with no new build. A conflict needs a hand fix, and the fixed tree is not the tree that was built. Options: stop and ask you each time, or allow one more build after the fix is reviewed. A session's note records you saying, on 2026-11-23, not to re-test the combination each time main moves; no session card holds those words yet, so this plan does not cite them (P23).
4. **Small edits you ask for directly.** Today "when the user asks for one specific edit, the request is the approval". With the gate, such an edit to code would need `allow merge` from you, and `allow build` if it needs compiling. Is that acceptable, or should a small edit have another route?
5. **Should the hooks repo be gated too?** It has no build, and its merges are already fast-forward only. This plan leaves it ungated, because a broken gate there could block its own repair. Say if you want it gated.

## Out of scope, for the user to approve

- The gate does not know whether tests passed. It reads only the build's result. Running the tests stays an instruction.
- The gate does not judge which targets a build names. One call of `build_cpp.ps1` with several targets is one build (P7).
- A pass that runs out unclaimed after 120 seconds is not counted, because only a started build writes a record.
- Builds from the user's own terminal and from CI are untouched, as today (P24).

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| The opener refuses a plan with no task table and fills nothing in | "The plan run" | G1 | `test_plan_run.py`: each malformed plan refuses and writes no file |
| One build per phase; a second only after a new signed commit; never a third | "The build gate" | G1 | `test_plan_run.py`: the seven refusing cases and two allowing cases of `pass_refusal` |
| No pass before every task is merged and signed | "The build gate" | G1 | same file: unmerged branch, unsigned file |
| The hook refuses a main-session pass with no run | "With no run" | G2 | `test_percall.py`: denied; LS1 |
| Agents are still refused a pass | G2 acceptance | G2 | `test_percall.py`: today's text unchanged |
| A typed `allow build` or `allow merge` lets exactly one through | "With no run" | G2, G3, G4 | allow passes once, next denied, extra word not recorded |
| Main takes only the plan tip or its clean merge with old main | "The merge gate" | G1, G3 | `main_refusal` cases; `test_ref_judge.py` marker cases |
| Main needs a passed, clean-tree build record when the phase builds | "The merge gate" | G1 | no record, dirty record, failed record all refuse |
| A phase lands once | "The merge gate" | G1 | second landing refused |
| A clean join of signed branches passes the review check; a hand fix does not | "The merge gate" | G4 | the new join test, three cases |
| A repo with no marker behaves as today | G3 | G3 | existing `test_ref_judge.py` cases unchanged |
| A broken or missing `plan_run.py`, or a bad run file, refuses | G1, G3 | G1, G3 | bad-file test; missing-module test |
| A second session gets no fresh allowance | "Several sessions" | G1 | join test: same counts |
| Only the user closes or abandons a run | "Several sessions" | G4, G1 | a close line not typed by the user changes nothing |
| The build writes started, passed or failed, and dirty | "The build record" | H1 | `test_build_record.ps1`; LS2 |
| The start list shows open runs and blocks nothing | G5 | G5 | `test_plan_runs_start.py` |
| The instruction text no longer asks for a build per task | G6, H2 | G6, H2 | the reviewer's greps named in each task |
| Hook changes follow the hooks README | "Order of going live" | steps 1 and 2 | the main session's test run and `sync_settings.py --check` |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: the main session's instructions ask for a build before review, a rebuild after sign-off and a merge per task | code | `C:\Users\Patrick\.claude\model-postures.md:11` | checked 2026-11-24 |
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
| P16: no test joins two signed branches and then moves main to the join | unchecked | `C:\Users\Patrick\.claude\gates\tests\test_review_rules.py`, searched by test name only, not read | G4 |
| P17: `waiver.py` records a typed allow with its prompt id, and the check that the user typed it runs later | code | `C:\Users\Patrick\.claude\gates\waiver.py:16-46` | checked 2026-11-24 |
| P18: one typed allow lets one call through and is then marked used | code | `C:\Users\Patrick\.claude\gates\percall.py:23-42` | checked 2026-11-24 |
| P19: hook work goes in a worktree, gets one reviewer, runs the changed scripts' tests (all of them for shared files), merges fast-forward and then syncs; the user's terminal is not gated | code | `C:\Users\Patrick\.claude\gates\README.md:150-180` | checked 2026-11-24 |
| P20: the test files for `percall.py`, `ref_judge.py` and `waiver.py` have these names | unchecked | `C:\Users\Patrick\.claude\gates\tests`, not listed | G2 |
| P21: the memory note "plan gates before dispatch" asks for builds in a way the gate changes | unchecked | `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory\plan-gates-before-dispatch.md`, not opened | M1 |
| P22: a real build under Claude Code reaches the new record lines | unchecked | `build_cpp.ps1`, where the lines do not exist yet | LS2; blocks nothing |
| P23: the user said not to re-test or rebuild when main moves | unchecked | `docs/sessions`, searched for the words and not found | question 3 |
| P24: outside Claude Code nothing gates a build | code | `tools/build_slot.ps1:5-8` and `tools/build_slot.ps1:90` | checked 2026-11-24 |
| P25: the user's limit is three builds at once, main session only | ruling | `docs/agents/rulings/agents-and-workflow.md`, the 2026-11-07 line under Confirmed | checked 2026-11-24 |
| P26: an integrator already joins task branches onto one wave branch | code | `docs/agents/integrate.md:27` and `docs/agents/integrate.md:41` | checked 2026-11-24 |
