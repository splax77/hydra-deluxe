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

The spelling is exact, and the opener reads nothing else. These are its rules.

The `## Plan run` section:

- The heading is the line `## Plan run`. The section runs to the next line that starts with `# ` or `## `.
- It holds only the `Slug:` line, the `Repo:` line (in that order, one space after the colon) and the phase table. Blank lines are fine. Any other line, such as a note or a second table, is refused.
- `Slug:` is the short name used for the run's file and its branches. It is letters and digits, joined by single `.`, `_` or `-`. It does not end in `.lock` and is not a Windows device name (`con`, `nul`, `com1` and the like).
- `Repo:` is the repo's top folder, written as a bare absolute path, with no backticks. It must hold a git checkout with a `main` branch.
- The phase table has the columns `Phase` and `Builds`. Each row holds one phase name, written by the same rules as a slug and used once, and its build count, `0` or `1`. The first row is the first phase.

The task table:

- It is the one table outside the `## Plan run` section that has a column headed `Branch`, `Phase` or `Owned files`. A second such table, even one with only one of the three, is refused. So is a task table that lacks one of the three columns or repeats one.
- Every line of it is a row that starts and ends with `|`, has the header's number of cells and has a `|---|` line under the header. It needs at least one task.
- A `Branch` or `Phase` cell is one word: bare, or in backticks. `Branch` is the task's own branch, the one its executor commits on. It must be a name git accepts, used once, and it may not be `main` or start with `plan` followed by a slash. `Phase` is a phase name from the `## Plan run` table.
- An `Owned files` cell is one or more paths, each in backticks and separated by commas. It lists the files the task may change, which the run records. A path listed twice in one cell is refused.

Fenced code (a block between ``` lines or ~~~ lines) is never read, so a plan may show the section's shape as an example. The plan's own path, which `--open` and `--join` take, is absolute.

The opener fills nothing in. It refuses a plan that breaks any rule above, a plan with no `## Plan run` section or with two, a branch named twice, a phase with no build count or a build count other than 0 or 1, and a slug an earlier run already used: any run file that holds the slug, closed or abandoned too, and any run whose slug matches it ignoring case (the run files sit on a file system that does not tell `Demo` from `demo`). It also refuses a plan in which two tasks of the same phase own the same file, because those two tasks would then have to be merged by hand. Give the file to one task, or put the two tasks in different phases.


## The plan branch

Each phase has its own branch, `plan/<slug>/<phase>`. A plan with phases `hooks` and `pages` has the branches `plan/<slug>/hooks` and `plan/<slug>/pages`, and so two builds and two moves of `main`. "The plan branch" below means the branch of the phase that is open now. Task branches merge into it, never straight into `main`; the integrator's join (`docs/agents/integrate.md`) happens there.

## Opening, joining and the next phase

The session opens the run itself, right after the user says yes to the plan. No typed line opens one. It runs:

```
python.exe -I C:/Users/Patrick/.claude/gates/plan_run.py --open <plan path>
```

Opening cuts the first phase's branch from `main` and records the commit it started from.

A run belongs to the plan, not to a session. A plan that takes several sessions keeps one run. A later session joins it with `plan_run.py --join <plan path>`, naming the plan file's path exactly as the run recorded it. The run counts its builds from git and the build record, so a session that joins gets no fresh allowance. Unfinished tasks wait on their branches.

When a phase has landed on `main`, `plan_run.py --phase <name>` cuts the next phase's branch from the new `main`. `<name>` must be the phase right after the open one; no other name is accepted. It refuses while the earlier phase has not landed. A phase has landed only when its plan branch has at least one commit since it was cut and `main` holds the branch's tip, so a branch that was cut and never committed on has not landed. `plan_run.py --status` prints the open runs.

## Files two runs share

The run records each task's owned files. When `--open` finds that another open run owns one of the same files, it names the file, that run's plan path and the sessions that joined it. It does not refuse. The two sessions then agree between themselves how their changes merge. The start-of-session list names the same overlaps every time, until one of the runs closes.

## The build and the move of `main`

Each task branch goes to its reviewer uncompiled. When every task branch of the phase is reviewed and merged onto the plan branch, and every gated file on it is signed, the main session asks `hydra_build_pass` for the phase's one pass, with `preset` set to the preset it will build, such as `default`. It builds once, with every target the tests need, and runs the tests (`docs/agents/derive-once-review.md`, "For the orchestrator", gives the order). A phase with build count 0 gets no pass.

If the build or a test fails, the main session reads every error, runs every test the binaries can still run, and sends all of it to one fresh executor on the plan branch. A fresh reviewer reads only that fix, and the main session builds once more. That second build is given only when the plan branch has gained a new signed commit since the first. If it fails too, the main session stops and tells the user; the gate never gives a third normal build. A normal build is any build of a preset other than `mutation`, and the extra mutation build below is never that third.

`main` then takes the phase once: either the plan branch's tip itself, or git's own clean merge of `main` and that tip. For a phase that builds, the build of that tip must have passed on a clean tree. If `main` moved after the build and git cannot merge cleanly, merge `main` into the plan branch and resolve the conflict by hand. That resolution is a new commit, so it gets its review, and the phase's second build covers it.

These checks apply only to a move of `main` that gains gated code, and only in a repo whose `main` holds a `.plan-run-gate` file. A session card, a plan or a ruling lands on `main` as before.

### Every pass names its preset

A preset is the named build setup in `CMakePresets.json`, such as `default`, `ship` or `mutation`. Every call to `hydra_build_pass` names one, and there is no default. A call with no preset is refused, whoever makes it, and that includes a session using a typed `allow build`. The pass is then good for that preset only: `build_cpp.ps1` refuses to build any other with it. The two words must match letter for letter, so `Mutation` is not `mutation`.

A session started before this rule landed still has the old `hydra_build_pass`, which cannot name a preset. It is refused a pass until it restarts.

### The extra mutation build

On 2026-11-24 the user was asked "May a phase compile a second time for the mutation gate?" and chose "Allow it", whose text was "One extra mutation-only build per phase that touches a C++ test." That answer was given in a question box in session `b885c6f8`. It is a session record, not a filed ruling.

So a phase that changed at least one C++ test file may take one more build, of the `mutation` preset only. It is given at the plan branch's tip, and only once a normal build of that same tip has passed on a clean tree. `plan_run_judge.py` in the hooks owns the full rule; the refusal names the condition that failed.

The main session asks for it last, after the phase's tests pass. There is one per phase, not one per commit. If it is taken early and a later fix changes the tests, no second one is given.

The extra build stands outside the phase's count. Normal builds are counted alone: a mutation build never counts as the phase's build or its rebuild, and never gives one back. A phase whose first build and rebuild both failed is still refused a third normal build, whether or not a mutation build is in the record.

This holds on a plan branch whose tip has the `.build-pass-preset` file at the repo root. That file says the branch's build scripts tie a pass to its preset, and every branch cut from `main` after this rule landed has it. An older branch gets no mutation pass, and its builds are counted as before, every preset alike.

## The lines the user types

Each line below works only when the user types it exactly in chat. A hook records the line and checks later that the user, not an agent or a tool, typed it.

`allow build` lets one build pass through for a session that has no open run, such as a small edit the user asked for directly. Each such message lets one pass through.

`allow merge` lets one move of `main` that gains gated code through for a session that has no open run. Each such message lets one move through.

`close plan run <slug>` closes a finished run. `abandon plan run <slug>` ends a run whose work will not land. Only the user closes or abandons a run; nothing else ends one, and a closed or abandoned run then counts as no run. A finished run, whose last phase has landed, shows as finished in the start-of-session list until the user closes it.

## The start-of-session list

At the start of each session, a hook prints every open run: its plan, its open phase, which task branches are merged onto the plan branch, how many builds it has used, which sessions joined it, and any file it owns that another open run owns too. It only informs and blocks nothing.
