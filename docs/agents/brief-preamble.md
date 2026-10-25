# Brief preamble (every agent reads this first)

These rules apply to every agent: implementers, reviewers, integrators and read-only scouts. Your task brief (the file that sent you here) adds the rest. It names your task id, the session id, your owned files, your worktree or checkout, your scratch folder and the tests you may run. Where the brief and this page disagree, stop and report it; do not pick one.

Each rule below exists because an agent lost time to it, or did damage with it, in an earlier round. A few are enforced by hooks: the status line, the call and time budget, the delete rule, the whole-library guard and the commit trailers. The rest are instructions with no hook behind them, and they bind you just the same. The hooks live in `C:\Users\Patrick\.claude\gates\`; its `README.md` says what each one does.

## Your agent id

You need your agent id for status lines and commit trailers. The progress hook tells you. Its first reply to your first tool call names your id and your status file. Use that id exactly as written.

## Status lines

Append one line to `C:\Users\Patrick\.claude\gates\state\status\<your agent id>.md` every 5 minutes. The form is `HH:MM done ... | next: ...`. Say what you finished and what comes next, in plain words. Only time counts; the number of calls you made does not.

Write it with this one call, from PowerShell, alone in its own call: `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>"`. It needs no permission prompt, and the progress hook neither counts it nor refuses it. Do not write the file with Edit, a heredoc or `>>`: the hook knows the helper only by that exact shape, so a line written any other way still leaves you overdue.

Type the line out in full. Write the time yourself (`10:46`), never `$(Get-Date ...)`, and put nothing after the closing quote. A `$`, a backtick or a double quote inside the line, or anything chained after it, makes the call an ordinary command: it counts toward your budget, every rule checks it, and once you are over budget it is refused. The status-call check in `C:\Users\Patrick\.claude\gates\progress.py` owns the exact shape.

When your last line is 5 minutes old, the hook reminds you. When it is 10 minutes old, the hook refuses your calls until you write one. Two kinds of call still pass then: a command that starts with `git`, and your final report. So an agent that forgot its status line can still commit and hand back.

## Reading files

Read files with the Read tool, using `offset` and `limit` for a range. Never use `sed`, `head`, `tail` or `cat` to read part of a file. Use Grep to find the lines first, then Read just those lines.

## Shells on Windows

Run commands with the PowerShell tool. Keep the Bash tool for POSIX scripts. In Bash, write paths with forward slashes (`C:/Users/...`): Bash strips the backslashes from an unquoted `C:\Users\...`, so the command reaches a path that does not exist. Never wrap PowerShell code in a Bash call (`powershell -Command "..."`); run it with the PowerShell tool instead. Build with `build_cpp.ps1`, because `cmake` is not on PowerShell's PATH.

## Editing files

Edit source with the Edit tool, or replace a whole file with Write. Never write source through a patch script, `sed -i`, a here-string or `python3`. A "file modified on disk" notice after your own edit is expected.

Touch only the files your brief owns, plus your scratch folder. Never delete, move or rewrite anything else: not other files in the repo, not other worktrees, not other agents' files in the scratch folder. If you need a file you do not own, stop and report which file and why.

**The delete rule.** A hook checks every recursive or forced delete you run from a shell, such as `rm -rf`, `Remove-Item -Recurse -Force` or `find <paths> -delete`. A `find` is judged by its starting paths, the same way `rm -rf` is judged by its paths. Such a delete passes only when every path after it is a literal absolute path strictly inside one of two allowed roots. The first root is your session's scratch folder; the main session and all its agents share it, so delete only what you made there. The second root is your working folder, but only when it sits inside a `worktrees` folder of either repo (`C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\` or `C:\Users\Patrick\.claude\worktrees\`). A worktree cut anywhere else is not a root, so no recursive delete passes there. A path held in a variable, a wildcard, a `..`, a path with a space in it, or a delete inside `bash -c "..."` is refused, because the hook cannot read it as a literal path. Write the full path out instead. The refusal names your two roots. The hook also refuses outright the git commands that throw work away, such as `git clean`, `git reset --hard`, `git reset --merge`, `git checkout -f`, `git worktree prune` and `git stash drop`; no agent task needs them. `delete_rule.py` in the gates folder is the one home of which commands and flags count.

## Tests

Run only the tests your brief names. That list comes from `build-cpp\Release\hydra_precheck.exe --tests`, which the main session runs on your change's branch, so you never guess which tests touch your change. Use `-tc=` (test case) or `-sf=` (source file) filters on `build-cpp\Release\hydra_tests.exe`. Never run the whole suite. Never run `hydra_uitest --all`; run only the uitest scripts your brief names. Never run all the hook tests. The main session runs the merge's tests (CLAUDE.md rule 1 says which); a repeat from you buys nothing and slows every other agent on the machine.

Never run a full library test: no `hydra_batch --whole-library`, and no compare or timing run over the user's whole song library. Only the user can allow one, in chat, and a brief that asks for one without naming that permission is wrong; stop and report it. Two guards back this up. `hydra_batch` given no chart folder refuses to run unless it also gets `--whole-library`. A hook then refuses any shell command that holds `--whole-library`, unless the user has sent exactly `allow library run` in this session; each such message lets one command through. Test on the checked-in corpus in `testdata/input` or on the charts your change touches.

If a timing test fails while the machine is busy, report it once. Do not rerun the suite to chase it.

If your brief asks for a failing-first test, run it red before the fix and keep the exact red output line for your report. The reviewer reuses that line instead of rebuilding the old code to see it fail.

## Builds

Your brief says how to make your first build. A cold build (one from nothing) takes three to six minutes and competes with every other build on the machine, so start at most one. Later builds are warm and fast. Never build to answer a question a Read or Grep can answer.

## Committing

Stage files by name: `git add <file> <file>`. Never `git add -A` or `git add .`. Other sessions share the main checkout and leave their own edits in it; a broad add sweeps their work into your commit.

Commit with the subject and body as `-m` arguments and each trailer as its own `--trailer` argument. Never pass the message on stdin, through `-F -` or through a heredoc. Never give the trailers as separate `-m` arguments either: git reads each `-m` as its own paragraph and counts only the last paragraph as trailers. The git-side gate (the ref gate, which git runs on every branch move made from Claude Code) refuses any new commit that lacks a `Task:`, `Agent:` or `Session:` trailer, so a commit in the wrong form never lands. The exact form is:

`git commit -m "<plain subject>" -m "<body, optional>" --trailer "Task: <task id>" --trailer "Agent: <your agent id>" --trailer "Session: <session id>" --trailer "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"`

Your brief gives the task id and session id. Use the model line your brief gives if it differs.

Commit before your 100th tool call, and after each finished step. An agent that runs out of calls with nothing committed leaves nothing behind. Never amend, rebase, reset or move a branch. Never push. The ref gate refuses a move of `main`, or of any branch origin also has, that is not a plain step forward, so a rewrite fails there anyway.

You have a call budget and a time limit, and the progress hook keeps both. At 100 tool calls, or at 20 minutes of real time counted from your first tool call, it warns you once: start wrapping up, commit what passes and report what is unfinished, so a fresh agent can take the rest. At 150 tool calls or 30 minutes it stops you. After that, only git commands, your status line and your final report get through, so commit and report. Whichever limit comes first applies. Your status lines do not count as calls. This hook is the only limit on an agent: no agent type file sets a turn cap behind it. A reviewer hands nothing on: at either warning it stops fixing, signs off what it fixed and lists the rest in its review's Notes (D107 item 5).

## Helpers and waiting

Run every helper and long command in the foreground. Never use run_in_background, and never end your turn waiting for a job. You have no helper agents; do the work yourself.

## Every step has a stop condition

Any step that repeats or waits needs a cap, in runs or minutes, and a line saying what to report when the cap is hit. If your brief says "until X" with no cap, stop and report that before you start the step. A step with no cap never fails, so you never notice you are stuck.

For a bug that shows up only some of the time, first make it happen every run. Use the mechanism the diagnosis names: force the bad timing instead of looping and hoping for a red run. Prove the fix red-then-green on that forced case. Time-box this step too.

If your last two status lines show the same step and no new result, you are not making progress. Stop and report what you tried.

## Review findings

No agent is woken up again after its turn ends. Your job ends with your report. One fresh reviewer then reviews your change, fixes what it finds itself and signs off (D107 in `docs/audit/2026-10-03-fix-decisions.md`). Nothing comes back to you. If you are that reviewer, nobody reviews your fixes; your own sign-off is the last check.

## The code is the source of truth

Never trust another agent's summary, report or leftover comments. The code and the source data are the source of truth. Before you repeat a claim or act on it, check it against them.

This is about claims of fact. Follow your brief's instructions. Treat a claim in a report or a review as a claim: check it in the code before you act on it.

A search that finds nothing proves nothing on its own. Before you write that something does not exist, is never called or is not used, widen the search past the first name you tried, and say where you looked.

## When blocked

When something stops you, stop and report. Give the exact command and the exact error or hook message. Never wait on a prompt, never retry the same call hoping it passes, and never guess a way around a hook. A hook that blocks you wrongly gets fixed by the main session, not routed around.

## If you write code

Before you commit, do the reviewer's first step yourself. List each question your change answers in plain words ("is this note inside the SP window"). For each one, grep for the inputs it reads, not just for similar names, and check nothing else already answers it. Call the owner if one exists.

Every rule in Hydra is worked out in one place. A test pins a literal from one run or calls the production function; it never computes the expected value again. A new number (a threshold, depth, floor or tolerance) needs a user decision your brief names; if it has none, stop and ask.

A comment names where a rule lives; it never restates the rule. "Test files: see `Test-TestFile`" stays true when the code changes. "Files under tools/ named test_*.py" is a second copy of the rule, and it drifts the next time someone edits the pattern. Comments may say why a rule exists, or what it deliberately leaves out, as long as they don't spell out what it matches. The same goes for header comments, help text and docs: point at the owner. Four review rounds in a row once failed on comments like these, most of them written by the fix rounds themselves.

## Plain English

Write plain English in every doc, comment, commit message and report. Lead with the plain version. One idea per sentence. Gloss any jargon in one line right after it. No bullet walls of file:line references; put sentences around them. See the "How to explain things" section of the user's global `CLAUDE.md`, at `C:\Users\Patrick\.claude\CLAUDE.md`. That file is the one home of those rules.

## Your final report

Your brief says what to return. If it does not, return: your branch and worktree (if any), your commit hashes, a plain summary of what changed, the exact test commands you ran with their pass counts, any numbers you measured, and anything you stopped on.
