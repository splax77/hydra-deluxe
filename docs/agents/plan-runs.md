# Plan runs

A plan run is a small record that belongs to an approved plan. It makes a plan's work cost one build and one move of `main` per phase, instead of one per task. Think of it as a shared cart: every task's branch goes into the cart, the cart is checked out once, and `main` takes the whole cart in one move.

This page is the one home of what a plan must hold to get a run, and of how a session uses one. Two hooks enforce it. `C:\Users\Patrick\.claude\gates\plan_run.py` owns what a run is: it reads the plan, keeps the run's file and lists the open runs. `C:\Users\Patrick\.claude\gates\plan_run_judge.py` owns the two decisions about a run: may this session have a build pass, and may `main` take this commit. `C:\Users\Patrick\.claude\gates\README.md` says how they are wired in.

## What a plan holds

A plan that wants a run holds a section headed `## Plan run`. It names the run's slug (the short name used for its file and its branches), the repo the run works in, and one row per phase with the phase's name and its build count. The build count is 0 for a phase that needs no compiling, such as docs or hook scripts, and 1 for a phase that does. A plan that ships in one go has one phase, named `all`.

```
## Plan run

Slug: <slug>
Repo: <the repo the run works in>

| Phase | Builds |
|---|---|
| all | 1 |
```

The spelling is exact. The section heading is `## Plan run`. Its two lines start `Slug:` and `Repo:`. `Slug:` is the short name used for the run's file and its branches. `Repo:` is the repo the run works in. The phase table has the columns `Phase` and `Builds`; each row holds one phase name and its build count, `0` or `1`.

The plan's task table carries three columns the run reads, headed `Branch`, `Phase` and `Owned files`. `Branch` is the task's own branch, the one its executor commits on. `Phase` is the name of the phase the task belongs to, as written in the `## Plan run` table. `Owned files` lists the files the task may change, which the run records.

The opener fills nothing in. It refuses a plan with no `## Plan run` section, a task table without one of the three columns, a branch named twice, a phase with no build count, a build count other than 0 or 1, or a run already open for the slug. It also refuses a plan in which two tasks of the same phase own the same file, because those two tasks would then have to be merged by hand. Give the file to one task, or put the two tasks in different phases.

## The plan branch

Each phase has its own branch, `plan/<slug>/<phase>`. A plan with phases `hooks` and `pages` has the branches `plan/<slug>/hooks` and `plan/<slug>/pages`, and so two builds and two moves of `main`. "The plan branch" below means the branch of the phase that is open now. Task branches merge into it, never straight into `main`; the integrator's join (`docs/agents/integrate.md`) happens there.

## Opening, joining and the next phase

The session opens the run itself, right after the user says yes to the plan. No typed line opens one. It runs:

```
python.exe -I C:/Users/Patrick/.claude/gates/plan_run.py --open <plan path>
```

Opening cuts the first phase's branch from `main` and records the commit it started from.

A run belongs to the plan, not to a session. A plan that takes several sessions keeps one run. A later session joins it with `plan_run.py --join <plan path>`, naming the plan file's path exactly as the run recorded it. The run counts its builds from git and the build record, so a session that joins gets no fresh allowance. Unfinished tasks wait on their branches.

When a phase has landed on `main`, `plan_run.py --phase <name>` cuts the next phase's branch from the new `main`. It refuses while the earlier phase has not landed. `plan_run.py --status` prints the open runs.

## Files two runs share

The run records each task's owned files. When `--open` finds that another open run owns one of the same files, it names the file, that run's plan path and the sessions that joined it. It does not refuse. The two sessions then agree between themselves how their changes merge. The start-of-session list names the same overlaps every time, until one of the runs closes.

## The build and the move of `main`

Each task branch goes to its reviewer uncompiled. When every task branch of the phase is reviewed and merged onto the plan branch, and every gated file on it is signed, the main session asks `hydra_build_pass` for the phase's one pass. It builds once, with every target the tests need, and runs the tests (`docs/agents/derive-once-review.md`, "For the orchestrator", gives the order). A phase with build count 0 gets no pass.

If the build or a test fails, the main session reads every error, runs every test the binaries can still run, and sends all of it to one fresh executor on the plan branch. A fresh reviewer reads only that fix, and the main session builds once more. That second build is given only when the plan branch has gained a new signed commit since the first. If it fails too, the main session stops and tells the user; the gate never gives a third.

`main` then takes the phase once: either the plan branch's tip itself, or git's own clean merge of `main` and that tip. For a phase that builds, the build of that tip must have passed on a clean tree. If `main` moved after the build and git cannot merge cleanly, merge `main` into the plan branch and resolve the conflict by hand. That resolution is a new commit, so it gets its review, and the phase's second build covers it.

These checks apply only to a move of `main` that gains gated code, and only in a repo whose `main` holds a `.plan-run-gate` file. A session card, a plan or a ruling lands on `main` as before.

## The lines the user types

Each line below works only when the user types it exactly in chat. A hook records the line and checks later that the user, not an agent or a tool, typed it.

`allow build` lets one build pass through for a session that has no open run, such as a small edit the user asked for directly. Each such message lets one pass through.

`allow merge` lets one move of `main` that gains gated code through for a session that has no open run. Each such message lets one move through.

`close plan run <slug>` closes a finished run. `abandon plan run <slug>` ends a run whose work will not land. Only the user closes or abandons a run; nothing else ends one, and a closed or abandoned run then counts as no run. A finished run, whose last phase has landed, shows as finished in the start-of-session list until the user closes it.

## The start-of-session list

At the start of each session, a hook prints every open run: its plan, its open phase, which task branches are merged onto the plan branch, how many builds it has used, which sessions joined it, and any file it owns that another open run owns too. It only informs and blocks nothing.
