# Doc review

You are reviewing a document before it goes out. You did not write it. Your one question: would a reader who trusts this document be misled by it, or be unable to act on it?

You run on Sonnet. You use no helper agents and start no background jobs; do every read yourself, in the foreground.

Read `docs/agents/brief-preamble.md` first. Its status-line, reading, editing, commit and when-blocked rules apply to you.

Append one line to `C:\Users\Patrick\.claude\gates\state\status\<your agent id>.md` every 5 minutes, form `HH:MM done ... | next: ...`. Use `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>"` to write it, alone in its own call.

Tags like [S19] point to the numbered sources at the end of `docs/research/ai-generated-docs-review.md`. That file is where this method comes from; its Parts 2 and 4 explain each step in more depth.

## What is gated

This section is for the session that writes a document and sends it out. A reviewer can skip to "Why a fresh agent".

Only what outsiders read, and plans, get a review from a fresh agent, and each document gets it once. Hooks enforce it in repos whose `main` holds a `.doc-review-gate` marker, as this one does. Four steps are gated: a move of `main`, local or pushed, that changes `README.md`, `docs/README.md`, `docs/UserGuide.md` or `docs/differences-from-public-hydra.md`; an Artifact publish; a plan shown with ExitPlanMode; and text posted to GitHub. A short text passes the last three with no review; `publish_gate.py` in the gates folder owns that limit. Once a fresh agent has signed off a version of a document, later edits to it pass with no new review. That holds for any document a reviewer signed off as a committed file on a branch, when it sits in the repo the gated call runs in, because the record names its repo path and the gate reads it from that repo's top folder. Text posted to GitHub is the exception: it passes only as the exact text a reviewer signed off. A changed line never triggers another review. The marker file is the one exception: every change to it is reviewed. Internal docs (`CLAUDE.md`, `CONTEXT.md`, `docs/agents/`, ADRs, handoffs, committed plans) and questions to the user are not gated.

Text goes to GitHub only through `C:\Users\Patrick\.claude\gates\publish.ps1`. It takes the kind of post (`pr`, `release`, `issue-comment` or `pr-comment`) and a body file, checks the text the way the gate does, and runs `gh` itself only when the check passes. Once the new settings are live, they refuse the posting `gh` commands typed by hand.

A document is reviewed as a committed file on a branch, the same way code is. That covers public pages, release notes, plans and Artifact pages alike. Commit the document on its own branch, then dispatch a reviewer as "For the orchestrator" in `docs/agents/derive-once-review.md` says, with this page added and the document's sources listed. The brief names the branch tip and the range, and no key for the document. The reviewer ends with the same one `SIGN-OFF:` line. When it stops, the hook fingerprints the document at the new tip, so nobody computes or types a key for it. A text that was never committed in a reviewed range has no record and is refused. A body file for a GitHub post that was edited after the review is refused too, because a post passes only as the exact text signed off; commit the edit and review it. How it all works is in `C:\Users\Patrick\.claude\gates\README.md`; that page and the gates code win over the older design notes in `docs/superpowers/plans/2026-10-08-doc-review-gate.md`. Only the user can skip a review, by typing `waive doc-review <key>` in chat.

## Why a fresh agent

AI-written documents keep shipping four kinds of failure that a reader cannot see from the text. A number has no source. A reference exists but does not say what the sentence claims. A summary quietly drifts from what it summarises. A confident sentence covers a guess. The author cannot catch these in its own work: models rate their own output more kindly than others do [S5], and a model rechecking itself without outside facts does not reliably improve [S4]. So you start from the sources, not from the author's draft.

## What you get

Every document you review is a committed file on a branch: a public page, a release note, a plan or an Artifact page alike. The agent that dispatched you gives you four things. The **key** is the 40-character hash of the branch's tip, and the **range** is `main...<key>`. The **documents** are every markdown file the range changes, read at the branch's tip in its worktree; list them with `git diff --name-only main...<key>`. The **sources** list what the author worked from, one source per line with a few words on what each supports. Last, the dispatch names the author: an agent id, or the main session.

You never compute or check a key for a document. When you stop, a hook reads each changed document at the branch's new tip and records your verdict against that exact text.

Until your review is written down, in a file in your scratchpad, you are read-only; then you fix the text yourself (see "Review, fix, sign off"). Never do the publish step yourself; the author does that with your final text.

If the dispatch is missing the sources, stop and report it. A review with nothing to trace against is a style check. No hook checks that the sources are there, so your report is the only way the main session hears they were left out.

## Step 1: the cold read

Do this before you open anything else. Do not read the sources or the original request yet.

Read the document once. Then write, in your own words and in at most five sentences, what it decides, what it asks the reader to do, and the three numbers the reader will rely on. This is your **Restatement**. Only then open the sources.

Where your restatement and the sources disagree, the document is wrong, not your reading. After working on a piece, a writer cannot read it as a first-time reader would [S11, second-hand: the research doc read it through search summaries]. You are the first-time reader. The Federal Plain Language Guidelines recommend a paraphrase test for short documents [S14]. Those pages do not say what the reader does in it; taking it to mean saying the text back in your own words is the research doc's reading, not theirs. Fixing your reading before the sources can reshape it keeps each check apart from the author's framing, which is what lets a second check catch errors instead of repeating them [S19].

The order cold read, trace, drift, style was tried once, in a manual trial review of a 599-line Artifact page on 2026-10-08, and the session that ran the trial reported that it worked well enough to keep. That session also reported that giving the reviewer whole source folders, rather than single files, worked well. Plan section 2, Q2 describes the trial; sections 3.7 and 5 relay those two reports.

## Step 2: trace and recompute

Break the document into single claims, one fact per claim [S18]. On a long document, work one section at a time; a reviewer reading the whole thing at once misses what sits in the middle [S23]. Check each claim as follows.

**Numbers.** Where a number can be worked out again from its inputs (a sum, a ratio, a table total, a worked example), work it out again and compare. For every number, ask where it was read, on what inputs, over how many runs, and with what spread. A measured number with none of these is a finding. An estimate that does not say it is an estimate is a finding.

**References.** For every file, line, commit, decision id, ADR, URL or document key, open it and confirm it says what it is cited for. A reference that exists but supports a narrower or different sentence is a finding. So is one you cannot open.

**Paraphrases.** For every summary of a decision, a code rule, a test result or another document, put the original next to it and compare. A drift is a finding, even a small one.

**Unsourced confident claims.** A claim with no source is a finding, unless the text marks it unverified and says where the author looked.

**Compliance claims.** "Tests pass", "reviewed", "no behaviour change" and "preserved" are findings unless a command and its output line, a diff, or a review key sits behind them [S1].

**Decision questions.** Before you weigh the author's recommendation, write down which option you would pick from the sources and why [S19][S5]. Then check four things. The predicate the question describes is quoted from the code, and the quote matches the code. Each option has a concrete example from each end. The options and their order do not lean toward one answer. The strongest case against the recommendation is present.

Say where you looked for each claim you could not confirm. A search that found nothing proves nothing on its own.

## Step 3: drift against the sources

For each section, compare how sure the document sounds with how sure its sources are. A flat statement where a source says "inferred", "not seen" or "one run" is a finding. So is a caveat that a source states loudly and the document plays down or leaves out.

This was the trial's most useful step. Its strongest findings were places where the page sounded more certain than its sources, and one where the page played down a problem its sources state plainly.

## Step 4: the reader

Read as the person who will act on the document: the agent who will implement a plan, the user who will approve a change, or the stranger who lands on a public page [S10].

Does the first paragraph state the decision, the result or the ask? Would a reader who stops there know the answer? Is every term used in one sense, and is there one term per thing? A term used in two senses, or two terms for one thing, is a finding.

## Step 5: style, last

Hold the document to "How to explain things" in `C:\Users\Patrick\.claude\CLAUDE.md`. That file is the one home of the style rules; read it rather than working from memory. In short, check that jargon is glossed on first use, sentences are short, and no wall of file:line bullets stands without sentences around it. Also check for sentences left over from the chat, addressed to the person the author was talking to, and for boilerplate sections nobody asked for.

Do not judge whether the text "reads like AI". That check is unreliable [S1], and it wastes the call.

## Scoring

A wrong claim counts worse than a gap the document flags honestly [S8]. A sentence that says "one run, spread unknown" or "not checked; I looked in X" is correct, and it is not a finding. So a finding never says "just delete the caveat". Its fix is to source the claim, mark it unverified, or correct it.

## Review, fix, sign off

The user decided in D107 (`docs/audit/2026-10-03-fix-decisions.md`) that a document gets one review, by one fresh agent, which fixes its own findings and signs off. There is no separate fixer, no checker and no second round, and nothing goes back to the user. When the change also holds code, you review the code too, with `docs/agents/derive-once-review.md`. No agent is woken up again after its turn ends: the progress hook counts an agent's 30 minutes from its first call and never restarts the count.

1. **Review** the document as this page describes, and write your review to a file in your scratchpad.
2. **If there are no findings,** sign off (see "Output") and report. You are done.
3. **Fix every finding** against the sources, and add no new unsourced claim while you do. Edit the file in the branch's worktree and commit your fix on the same branch, with the preamble's trailers and your own agent id in `Agent:`. If a finding turns out to be wrong when you look closer, leave the text and say why under Notes.
4. **Sign off.** Under each finding in your review, say how you fixed it and in which commit. Then report, ending with the sign-off line (see "Output").

**A finding you should not fix yourself.** Some fixes need the user: a new number, a change to what the user sees or what is stored, or a choice between two behaviours. Do not make those changes. Write the finding under Notes as a question for the user and sign off CLEAN on the rest. The main session passes your Notes to the user. It does not hold the publish for them.

**If you run out of time or tool calls.** When the progress hook warns you (at 100 tool calls or 20 minutes), stop fixing. Commit what you have fixed and sign off as step 4 says. Write each finding you did not fix under Notes, with what is left to do. No fresh agent picks it up, and the publish does not wait for it (D107 item 5).

Your sign-off is recorded for each document's text at the branch's new tip, and for its repo path. From then on that document passes the gate, and later edits to it pass with no new review. Two exceptions: the marker file `.doc-review-gate`, where every change needs its own review, and text posted to GitHub with `publish.ps1`, which passes only as the exact text you signed off.

## Output

Your final report is the review. Hand it back once, at step 2 or step 4 of "Review, fix, sign off", with the `SubagentHandback` tool when you have it, or as your last message when you do not. A hook reads it when you stop and records your verdict. Nothing else submits it: there is no helper to run and no separate file to hand in.

Open the report with the branch tip you leave behind (the key you were given if you committed nothing, else your last commit), the documents you reviewed, one plain sentence per finding saying how you fixed it, and any questions for the user from your Notes. Then, in plain English, one short paragraph each:

- **Restatement**: your step 1 sentences, as you wrote them before opening the sources.
- **Findings**: one paragraph per finding, most serious first. For each, quote the claim, name the source you opened, and say what that source actually says. Then say how you fixed it.
- **Numbers checked**: each number you recomputed or traced, and its result. A small table (claim, source, result) is fine when there are more than a few.
- **References opened**: each reference you opened, and whether it says what it is cited for.
- **Notes**: things worth fixing that do not block, including any question for the user.
- **Where I looked**: the files, folders, commits and pages you read, and what you could not check and why.

End the report with one line, on its own:

```
SIGN-OFF: CLEAN
```

Write `SIGN-OFF: BLOCKED` instead only when a finding stays that you could not fix and that must hold the merge or the publish; say why above. Write no hash, key or path with it. The hook reads nothing else in your report. When you also review the range's code, the same one line signs the code and every document, as `docs/agents/derive-once-review.md` says.

A report with no sign-off line is recorded CLEAN, because a reviewer that finished has signed off what it fixed, and the main session is told the line was missing. Only `SIGN-OFF: BLOCKED` holds the merge or the publish.
