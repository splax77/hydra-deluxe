# Brief preamble (every agent reads this first)

These rules apply to every agent: implementers, reviewers, fix rounds, integrators and read-only scouts. Your task brief (the file that sent you here) adds the rest. It names your task id, the session id, your owned files, your worktree or checkout, your scratch folder and the tests you may run. Where the brief and this page disagree, stop and report it; do not pick one.

Each rule below exists because a hook refused it, or an agent lost time to it, in an earlier round. Following them up front costs nothing. Learning them from a denial costs a call and often a minute.

## Your agent id

You need your agent id for status lines and commit trailers. The progress hook tells you. Its first reply to your first tool call names your id and your status file. Use that id exactly as written.

## Status lines

Append one line to `C:\Users\Patrick\.claude\hooks\state\status\<your agent id>.md` every 10 tool calls or every 5 minutes, whichever comes first. The form is `HH:MM done ... | next: ...`. Say what you finished and what comes next, in plain words.

Write it with this one call, from PowerShell: `& "C:/Users/Patrick/.claude/hooks/status_append.ps1" <your agent id> "<line>"`. It needs no permission prompt. Do not write the file with Edit, a heredoc or `>>`.

A hook warns you when a line is due. Write the line then; an overdue line gets your next call refused.

## Reading files

Read files with the Read tool, using `offset` and `limit` for a range. Never use `sed`, `head`, `tail` or `cat` to read part of a file; the hook blocks them. Use Grep to find the lines first, then Read just those lines.

## Editing files

Edit source with the Edit tool. Never write source through a patch script, `sed -i` or a here-string. To replace most of a file, Write `<file>.new` and move it over the old one, because the shrink guard blocks large deletions made through Edit. A "file modified on disk" notice after your own edit is expected.

Touch only the files your brief owns, plus your scratch folder. Never delete, move or rewrite anything else: not other files in the repo, not other worktrees, not other agents' scratch. If you need a file you do not own, stop and report which file and why.

## Tests

Run only the tests your brief names. Use `-tc=` (test case) or `-sf=` (source file) filters on `build-cpp\Release\hydra_tests.exe`. Never run the whole suite. Never run `hydra_uitest --all`; run only the uitest scripts your brief names. Never run all the hook tests. The main session runs the full suite once per merge; a repeat from you buys nothing and slows every other agent on the machine.

If a timing test fails while the machine is busy, report it once. Do not rerun the suite to chase it.

If your brief asks for a failing-first test, run it red before the fix and keep the exact red output line for your report. The reviewer reuses that line instead of rebuilding the old code to see it fail.

## Builds

Your brief says how to make your first build. A cold build (one from nothing) takes three to six minutes and competes with every other build on the machine, so start at most one. Later builds are warm and fast. Never build to answer a question a Read or Grep can answer.

## Committing

Stage files by name: `git add <file> <file>`. Never `git add -A` or `git add .`. Other sessions share the main checkout and leave their own edits in it; a broad add sweeps their work into your commit.

Commit with the subject and body as `-m` arguments and each trailer as its own `--trailer` argument. Never pass the message on stdin, through `-F -` or through a heredoc; the hook cannot read those. Never give the trailers as separate `-m` arguments either: git reads each `-m` as its own paragraph and counts only the last paragraph as trailers, so `git-ref-gate` refuses to move the branch. The exact form is:

`git commit -m "<plain subject>" -m "<body, optional>" --trailer "Task: <task id>" --trailer "Agent: <your agent id>" --trailer "Session: <session id>" --trailer "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"`

Your brief gives the task id and session id. Use the model line your brief gives if it differs.

Commit before your 110th tool call, and after each finished step. An agent that runs out of calls with nothing committed leaves nothing behind. Never amend, rebase, reset or move a branch. Never push.

## Helpers and waiting

Run every helper and long command in the foreground. Never use `run_in_background`, and never end your turn waiting for a job. You have no helper agents; do the work yourself.

## When blocked

When something stops you, stop and report. Give the exact command and the exact error or hook message. Never wait on a prompt, never retry the same call hoping it passes, and never guess a way around a hook. A hook that blocks you wrongly gets fixed by the main session, not routed around.

## If you write code

Before you commit, do the reviewer's first step yourself. List each question your change answers in plain words ("is this note inside the SP window"). For each one, grep for the inputs it reads, not just for similar names, and check nothing else already answers it. Call the owner if one exists.

Every rule in Hydra is worked out in one place. A test pins a literal from one run or calls the production function; it never computes the expected value again. A new number (a threshold, depth, floor or tolerance) needs a user decision your brief names; if it has none, stop and ask.

## Plain English

Write plain English in every doc, comment, commit message and report. Lead with the plain version. One idea per sentence. Gloss any jargon in one line right after it. No bullet walls of file:line references; put sentences around them. See the "How to explain things" section of `CLAUDE.md`.

## Your final report

Your brief says what to return. If it does not, return: your branch and worktree (if any), your commit hashes, a plain summary of what changed, the exact test commands you ran with their pass counts, any numbers you measured, and anything you stopped on.
