Read `docs/agents/brief-preamble.md` first; its rules apply. Task id: EFF-SCORE-<task>. Session id: <session id>. Write a status line every 5 minutes with `gates\status_append.ps1`, as the preamble says.

You are scoring six finished attempts at one coding task. The attempts are named R1 to R6. The names say nothing about the attempts, and the order means nothing.

Read these files and no others:

The task's checklist is at `<checklist file>`. Each of its lines is one thing the finished work must do, numbered C1, C2 and so on.

The reference diff is at `<reference diff file>`. It is one correct way to do the work. An attempt may do the same job a different way, and that counts as done.

The attempts are in `<bundle folder>`. Each one has `R1.diff`, the attempt's whole change, and `R1.test.txt`, the output of the task's named tests on that attempt.

Do not open any other file or folder, and do not run git. You change nothing and commit nothing. If a file is missing or empty, say so in your report and score that attempt from what is there.

Score each attempt against each checklist line, on the diff and the test output alone. Use one of three words. `done` means the attempt does what the line says. `partly` means it does some of it, or does it in a way that breaks part of the line. `missing` means it does not do it.

Then read each diff for changes that are outside the task, meaning anything the checklist and the reference diff give no reason to touch. Name the file and say what changed in one plain sentence.

Your report has exactly this shape, with one line for each attempt and checklist line, R1 first, and a short reason after the word:

```
R1 C1 done: <reason>
R1 C2 partly: <reason>
R1 outside: <file>: <what changed>
R1 outside: none
```

After the six attempts, add one line per attempt saying whether its named tests passed: `R1 tests: passed` or `R1 tests: failed`. Do not rank the attempts, and do not guess how any attempt was made.
