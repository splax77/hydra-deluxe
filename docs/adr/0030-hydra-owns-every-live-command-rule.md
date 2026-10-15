# Hydra owns every live command rule

Claude can drive Hydra's GUI one command at a time through an MCP server.
MCP (Model Context Protocol) is how Claude Code starts a helper program, asks
it for its tools, and sends it each tool call. The system has three parts. A
C++ live channel runs inside Hydra and takes command lines from a pipe. A
Python helper, `tools/hydra_ui_mcp.py`, speaks MCP to Claude Code and passes
each tool call to that pipe. A stand-in Hydra, `tools/fake_hydra_live.py`,
lets the helper's tests fake a broken Hydra.

The user approved the design on 2026-10-14, in
docs/superpowers/plans/2026-10-14-hydra-mcp-server.md. The ownership rule
below came later the same day, as that plan's T8. How to use the server is in
docs/agents/ui-testing.md, "Live mode and the MCP server".

## The decision

Hydra's C++ side owns every command rule. That covers:

- what a verb means and how its arguments parse
- which lines are commands
- whether a number is valid, the timeout in force, and the longest a command may run
- line numbers
- the wording of every reply, and what counts as a failure
- filtering the `text` and `dump` output
- where Hydra writes its log and its crash dump

The Python helper is transport only. It launches Hydra and stops it, picks the
pipe's name, and keeps the tool list. It caps each wait using only numbers
Hydra reports, and cuts replies that are too long. It also takes the snapshot
of the user's library and checks that no `Hydra.exe` runs from that folder.
It sends each line exactly as given and returns Hydra's text as given.

The stand-in owns only transport failures: a hang, an exit, a closed pipe and
an oversized reply. It never answers a command the way Hydra would. Every
test of what a command does runs against the real headless
`hydra_uitest --live`.

The plan's T8 table lists each question with its owner. A comment or doc
names that owner and does not restate its rule.

## Why

The first build split the work into a C++ channel and a Python helper, but
never said which side owned what. So the helper drifted into copying Hydra's
rules. It parsed numbers, skipped lines, numbered lines, filtered
dumps and chose defaults, such as a copy of the 30 s timeout. The stand-in
copied Hydra's reply wording a third time.

Copies drift, and Claude saw the result: two numbers for one thing. After a
failed `hydra_run`, the live channel said "failed at line 23", counting every
line of the session. The helper said "stopped at line 4 of 5", counting the
script. The user rejected two numbers for one line, because they confuse the
model reading them. Numbers had the same problem on the C++ side: the
interpreter read `timeout 90s` as 90 and `wait abc` as 0, so a typo quietly
set a number nobody meant.

Think of the helper as a postal worker. It carries letters and says when
one never arrives. It never opens a letter or rewrites it. With that rule, a
question about a command has one answer, in C++, and the helper cannot
disagree with it.

So a live reply names no line number, and the helper numbers only the lines
of the script Claude sent it. Every reply carries the timeout in force, and
the helper reads its caps from there. Hydra refuses a malformed number in
both modes, and in live mode it refuses a `wait` longer than the timeout in
force. So no command outlasts a cap the helper already knows.

## The smaller decisions

**A named pipe.** Hydra and the helper talk over a Windows named pipe. Each
request is one line, and each reply is one JSON line, so the reader never
guesses where an answer ends. Stdout was no good: `hydra_uitest` prints its
test output and its crash report there, and `Hydra.exe` is a GUI program
with no console, whose output goes to a log file. TCP on localhost would have
needed a port chosen at run time and reported back through the log. Python
opens the pipe as a plain file, with no extra package. The helper picks the name and passes it with `--pipe`; Hydra never
makes one up. Hydra only writes JSON and never parses it, so it needs a
string escaper and no JSON library (`tests/ui/live_protocol.h`).

**Python, on the `mcp` package.** The helper is glue: it launches a process,
copies five files, times waits and trims text. The repo already keeps that
kind of tool in Python under `tools/`, and a change to it needs no C++
build. It uses the official `mcp` package, so the handshake, the tool list
and the message framing are the reference implementation's, not ours. The
UI driving stays in C++, where the test engine is.

**Two exes, one interpreter.** Headless mode runs `hydra_uitest --live` on
the test corpus. Window mode runs a dev build's `Hydra.exe --live` on a
snapshot copy of the user's library. Both run the same `live` test, and both
hand each line to the same `run_command`, which command files use too. So a
verb means the same thing in a script, headless and in the real window.

**The real `hydra.db` is never opened.** The helper copies the user's files
into a folder of its own under the temp folder and names only that copy on
Hydra's command line. Hydra copies the seed again into its own scratch folder
and opens only that second copy. And the live test refuses any seed that is
not under the temp folder (`testtemp::is_under_temp`), so even a helper bug
that passed the real path would build no app. The user asked for the real
library to be impossible to open in this mode, not just avoided, so the last
guard is checked by code, not by care.

**Dev builds only.** The live channel lives in the test harness and under
`HYDRA_UITEST_ATTACHED`. The `ship` preset turns that off, and the installer
never installs `hydra_uitest`. A shipped Hydra cannot be driven this way.

## Rejected

**Letting the helper understand commands.** This was the first build. It made
the copies above, and each one could disagree with Hydra.

**A hand-written MCP handshake** of about 150 lines with no dependency. It
would have had to track protocol versions by hand.

**TCP on localhost.** See "A named pipe" above. The plan kept it only as a
fallback in case Python could not open the pipe, and it could.
