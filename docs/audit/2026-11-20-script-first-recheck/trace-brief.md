# Trace brief: what each stand-in gives back

First read `C:\Users\Patrick\Downloads\Hydra\hydra-blind\BLIND-PREAMBLE.md` in full. Its rules bind you, and this brief adds the rest. Where the two disagree, stop and report it. That page stands in for the repo's own brief preamble, which sits outside the copy where you cannot read it.

## What your launch message adds

The message that launched you gives four things this brief leaves open: your task id, the session id, a number N for your output file, and the sections of `kinds.md` for the kinds you trace. There are at most eight kinds. Trace exactly those kinds and no others. If any of the four is missing, stop and report which one.

## The job

A test sometimes swaps a piece of production code for a stand-in: a small piece of test code that runs in its place. A search script has already found where the tests do this and sorted the finds into kinds. A kind is one stand-in form in one file. Its heading is a word, a space, then the path of the file the hit is in, in parentheses. Each kind's section in your launch message gives its hit count, the tests that use it and its hit lines.

For each kind, start at its hit lines, find the stand-in's code, and answer three questions with line cites.

1. What does the stand-in give back when the code under test calls it? Say it in plain words: a value, the content of a file, an error it throws, a wait followed by a call to the real function, or nothing. Cite the lines where the stand-in is made and where it gives the value back.
2. Which file under `code/testdata` or `code/tests` is that value read from, if any? Cite the file or folder with no line. Write `none` when the value is built in the test's code, and cite the lines that build it. Write `cannot tell` when you could not find where it comes from.
3. What does the test change in that value, or in the file's content, before the stand-in hands it over? Name each field, line or part it rewrites, with the lines that rewrite it. Write `nothing` when it is handed over as it was read or built, and cite the line that hands it over.

The paths in a kind's heading and hit lines name files under the copy's `code` folder. Every cite you write starts with `code/`.

If a kind's stand-ins do not all give back the same thing, write one case per different thing. Each case is its own three lines. Never fold two different answers into one line.

You state facts and nothing else. Do not say whether a stand-in's value is real or made up, whether a test depends on it, whether the test is right, or whether any record about it is right. Those are questions for the main session and the user.

## Your output file

Write one file, `C:\Users\Patrick\Downloads\Hydra\hydra-blind\out\traces-N.md`, with N the number your launch message gives. Write it with the Write tool. It holds one section per kind you were given, in this format, which a script checks:

```
The trace file has these line kinds and no others. Blank lines are allowed.
Nothing comes before the first heading.

## <kind, copied exactly from its heading in the kinds you were given>
returns: <what the stand-in gives back when the code under test calls it> | cites: <cites>
read from: <a file or folder cite, none, or cannot tell> | cites: <cites>
changes before use: <what the test rewrites in it before the stand-in hands it over, or nothing> | cites: <cites>

A traced kind holds one or more cases. A case is the three lines above, in that
order. When the kind's stand-ins do not all give back the same thing, write one
case per different thing.

A kind you did not trace holds one line and nothing else:

not traced: <what stopped you>

Cites in a "cites" field are joined by ";" with a space each side, and
there is at least one. A cite is one of:
  code/<path>:<N>          one line
  code/<path>:<A>-<B>      a range of at most 30 lines

The "read from" answer is a cite with no line, of a file or folder inside
code/testdata or code/tests (never one of those two folders itself); or
none, when the cites show the value built in code; or cannot tell.
Any other text shaped like a file and a line, anywhere in the file, must
also be a cite of the form above, of a line that exists.

No line holds any of these words or phrases, in any letter case:
  by design
  on purpose
  deliberately
  kept
  flagged
  contradicted
  says something else
  no record
  stand-in found
  not a problem
  covered by a ruling
```

Copy each heading from your launch message character for character. A heading that differs by one character counts as a kind nobody traced.

## Caps and the status line

Your limits are the preamble's: a warning at 100 calls or 20 minutes, and a stop at 150 calls or 30 minutes. Split them evenly across your kinds. Spend no more than about 12 calls on one kind, which is the warning's 100 calls shared across eight kinds; when a kind reaches that and is still unclear, write what you found, use `cannot tell` where you could not find an answer, and move on. At the warning, write `not traced: reached my limit` for every kind you have not started, write the file and hand back.

Write a status line every 5 minutes, exactly as the preamble says. It is the only shell command you may run.

## Rules every audit brief carries

These come from the repo's audit method, "What every audit brief carries".

A finding leaves an audit by one of four exits: fixed, a question to the user, covered by a ruling, or not a problem. Only the main session picks an exit. You pick none, and you write no status for any kind.

Every split is its own line. Here that means each different thing a kind's stand-ins give back is its own case.

"On purpose", "by design", "deliberately" and "kept" are claims about what someone wanted, not evidence. Do not write them. The script refuses a trace that holds them.

A search that finds nothing proves nothing on its own. Before you write `none` or `cannot tell`, widen the search past the first name you tried: search for the value's type, the function it is handed to and the file it might come from.

No whole-library run happens without the user's `allow library run` in chat. You run no program at all, so you start none.

The user's words live in the decision log, the question files, the plans and the session cards under `docs/sessions/`. A card keeps the user's typed words, the label they chose and the start of each question, not the full question. None of those files is in the copy, and this job needs none of them, because you judge nothing.

Your kinds are the exact ones your launch message lists. Do not trace a kind you were not given, even one you come across.

## Handing back

Hand back with a short report: the output file you wrote, how many kinds you traced and how many you marked `not traced`, and every call that was refused, with the exact call and the exact message.
