# Handoffs: what one holds and how to check one

A handoff is the note one session leaves so the next session can carry on. It is another agent's summary, so CLAUDE.md rule 9 applies to it: before any new work starts from a handoff, every claim in it is checked against the code. This page says what shape a handoff takes, so its claims can be checked, and how the check runs. The `/handoff` command (`.claude/commands/handoff.md`) writes one in this shape.

## Where a handoff lives

A handoff is a markdown file at `docs/handoffs/<YYYY-MM-DD>-<slug>-handoff.md`, committed on a branch. The date in the name is the day it was written. The checker reads that date to decide what "changed since" means.

## The shape

A handoff holds prose and one table.

The prose says three things in plain English: the goal, what was done, and what is next. It also quotes the user's rulings from the session in their own words, each with its session id.

The table sits under a heading that is exactly `## Claims`. It lists every fact the next session will act on, one row per claim, in four columns. Write the Value column in backticks, because the checker reads backticked tokens.

| Kind | Value | Claim | How to check |
|---|---|---|---|
| commit | the full 40-character hash, in backticks | what the commit holds, and whether it is on `main` or pushed | the checker finds the hash; `git branch --contains <hash>`; `git log origin/main --oneline -1` |
| branch | the branch name, in backticks | what the branch holds and whether it is merged | the checker finds the branch; `git log main..<branch>` |
| path | a repo-relative path, or a full absolute path outside the repo, in backticks | what the file holds | the checker finds the file |
| function | `<name>()` or `<Class>::<method>`, in backticks | what it does and where the handoff says it lives | the checker finds it with `git grep` |
| test | the exact command, in backticks | the line it printed last time (the pass count, or the one red line) | run it; the same line prints |
| ruling | the session id or card id | the user's words, in quotes | the card or transcript holds those words |
| state | a plain fact ("the worktree X exists", "Y is not yet merged") | the fact | the command that shows it |

After the table comes a heading `## Checker output`, holding what the checker printed when the handoff was written.

## Rules for the table

A claim with no row is not a claim, and the next session may not act on it.

A row whose check fails is wrong until someone fixes the handoff.

Write every hash in full, all 40 characters. Never abbreviate one: a short hash can stop being unique, and the checker marks it `missing`.

Name no scratch path. A scratch folder is cleaned up and is not in the repo, so the next session cannot check a claim that points there. Put what matters into the handoff itself, or into a committed file it names. (The docs test's scratch-path check skips `docs/handoffs/`, so this rule holds by instruction there, not by a test.)

The checker does not judge the Claim column, and it leaves the `test`, `ruling` and `state` rows to the session (it marks them `manual`). The session reads those against the code and runs the tests itself.

## The handoff checker

The checker is a script in Hydra's `tools/` folder. The command is:

`py -I tools/check_handoff.py <handoff.md> [--repo <root>] [--since <YYYY-MM-DD>]`

The repo defaults to the one that holds the script. Pass `--repo <root>` to check commits and paths in another repo, such as the hooks repo. The checker has two modes, chosen by whether the file has a `## Claims` heading. `--since` matters only in scrape mode.

**Table mode** is for a handoff with a `## Claims` table, which every new handoff has. The checker reads only the rows under that heading, and it takes each row's kind from the Kind column. It covers all seven kinds:

- A `commit` row is `ok`, `missing` (git does not know the hash, or it is not the full 40 characters), `not-on-main` (the commit is not on `main`) or `not-pushed` (it is on `main` but not on `origin/main`).
- A `branch` row is `ok` when git knows the branch, else `missing`.
- A `path` row is `ok` when the file exists, else `missing`.
- A `function` row is `ok` when `git grep` finds the name in the tree, else `missing`.
- A `test`, `ruling` or `state` row is `manual`: the checker cannot judge it, so the session does.

Each row prints one line: its status word, then its kind, value and claim. The last line is one summary line. The run exits 0 only when every row the checker judges (every row but `manual`) is `ok`; it exits 1 otherwise and 2 on a bad argument. A `not-on-main` or `not-pushed` commit is not an error in the handoff when its Claim says so, but the run still exits 1, so read the status words and not only the exit code.

**Scrape mode** is for an older handoff with no table. The date defaults to the date in the handoff's file name; `--since` overrides it. The checker reads every token in backticks, plus bare hex words and bare paths, and sorts each by its shape. A hex word is a commit. A branch name git knows is a branch. A token with a path separator or a known extension is a path. A token shaped like `<name>()` or `<Class>::<method>` is a function. A token of none of these shapes is ignored, so prose in backticks never fails the check. A line that names the hooks repo's root folder is also checked in that repo. The checker prints three sections, `EXISTS`, `MISSING` and `CHANGED SINCE <date>`, then one summary line: `checked N claims: E exist, M missing, C changed since <date>`. It exits 0 when nothing is missing, 1 when anything is, and 2 on a bad argument.

The checker proves that a name exists. Whether the handoff's sentence about that name is true is still the session's job, by reading the code.

## Who runs the checker, and when

The `/handoff` command runs it right after writing a handoff, fixes every `missing` row it finds, and pastes what it printed under `## Checker output`: in table mode the row lines and the summary line.

The session that continues a handoff runs it again before any work starts, because the repo may have moved since. It reports each `MISSING` or `CHANGED SINCE` line (scrape mode), and each Claims row not marked `ok` or `manual` (table mode), before it builds anything. Then it reads every claim against the code, including the rows the checker leaves to it. The preamble's "Continuing from a handoff" paragraph gives an agent the same steps.
