# Audit of the past audits: common brief for every scout (2026-10-29)

Session id: `61cee421-c19b-4c26-979c-a253c696f7f5`. Your task id and your items are in the short brief that sent you here.

## Read first, in this order

1. `docs/agents/brief-preamble.md`. Its rules bind you: status lines every 5 minutes through `status_append.ps1`, the 100-call / 20-minute wrap-up, Read with offset and limit, no `sed`/`head`/`cat` for reading.
2. `docs/agents/audit-method.md`, at least "The words this page uses", "The four exits", stage 6 "Check every closure" and "What went wrong before". This is the method you check closures by.
3. `docs/handoffs/2026-10-29-audit-of-audits-handoff.md` (removed; `git show 444631e6:docs/handoffs/2026-10-29-audit-of-audits-handoff.md`), sections "Why: what went wrong with M39", "The rules every verdict follows" and "What each verdict looks like".

All repo paths are relative to the checkout `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\fervent-bohr-838562`, which sits on branch `audit-of-audits` at `main` (1fc55723). Read code and docs there. "The code today" means that commit. Use `git log` / `git log -S` / `git show` in that folder to find when something changed.

## What you are doing

Past audits found problems and then closed many of them without asking the user. You check, for each of your items, whether the way it left its audit holds up. You are read-only. Change no file in the repo. Write only your own output file (below).

For each item:

1. **Find every split.** A split is two parts of Hydra (or of the hooks, for hook audits) answering one question differently, where both sides are things the user sees or relies on. Read the finding's own tables and the code. One finding can hold more than one split, and a split can hide in a finding titled as something else (finding 306 was titled "a fixed 10 ms nobody chose", and its table showed the 3 ms vs 10 ms split). Write each split as its own line, with both sides' real values and the file:line of each side in today's code. If the item holds no split, say what it is instead (a number nobody chose, a doc error, a code copy that gives the same answer) in one line.
2. **Find how it left the audit.** One of: a question the user answered, a ruling it was matched to, "fixed", "not a problem / no fix / kept / by design", "code-only", or nothing at all. Quote the closure's own words with file and line.
3. **Find the user's words behind it.** For a question: the question's own text and its Recommended line (when the user answered a batch with "all recommended", those are the words that count), plus the D-record that records the answer, all quoted verbatim with file and line. Decisions live in `docs/audit/2026-10-03-fix-decisions.md`. Question files are `docs/audit/2026-10-04-phase-3-5-questions.md`, `-phase-6-questions.md`, `-phase-7-questions.md` and `docs/audit/2026-10-11-concept-audit/questions.md`. Plans that recorded code-only calls "as recommended" are under `docs/superpowers/plans/` (for example `2026-10-04-phases-3-5.md`, `2026-10-04-phase-6.md`, `2026-10-04-phase-7.md`; removed, `git show 444631e6:docs/superpowers/plans/<name>`). Many early rulings (D1 to D47) were asked in chat: the user's exact words for those may be in the session cards in `docs/sessions/` (grep the cards and `index.md` by D-number or finding id). A search that finds nothing proves nothing on its own: say where you searched.
4. **Read the code today** at the input where the two sides used to differ. Mark each claim "by reading" (with file:line) or "proven by" a named run. You may not run anything over the user's song library (no `hydra_batch` without a chart folder, nothing with `--whole-library`). You do not need builds or tests for this job; reading is enough.
5. **Give one verdict.**
   - **PASS:** the closure's evidence holds. For a ruling or a question: the quoted words name both sides of every split (both values or both behaviours), and describe the code's real rule, not a narrower one. For "not a problem": you can write one sentence saying what each side answers, and they really are different questions. For a non-split item (say a number nobody chose): the question the user saw described what the code really does, and the code matches the answer.
   - **REOPEN:** the evidence does not hold. Examples: the words name only one side; the closure matched a decision by topic; "on purpose", "by design", "deliberately", "kept" or "code-only" with no ruling behind them; the question was narrower than the code's rule; nothing was ever asked about something the user sees or that changes a score, count or stored record.
   - **GONE:** the code no longer has the split or problem. Name the commit that removed it and what the code does now at the input where the sides differed. If the fix picked one side of a split with no ruling behind that choice, still write GONE, but add the line `Side picked without a ruling:` saying which side won and where. The main session decides whether that goes to the user.

"Fixed" in `docs/audit/2026-10-03-fix-triage.json` proves nothing. Finding 306 is "fixed" because the 10 got a name, and both edges are still in the code. Check the code yourself (CLAUDE.md agent rule 9: never trust another agent's summary or a status word).

Your items' packets (where you have one) are verbatim extracts the main session's script made from the files. They save you lookups. They are not evidence: check every line you quote against the file itself.

## Output

Write your blocks to `<your task id>.md` in the session's scratch folder. That folder was not kept; each scout's finished blocks were copied to `docs/audit/2026-10-29-audit-of-audits/blocks/`. Create it with Write at the start, with a heading and the line `<!-- next -->`. After each item, use Edit to replace `<!-- next -->` with your new block followed by `<!-- next -->`. So a stop at any point leaves every finished block on disk.

One block per item, in this form:

```
### <audit> <item id>: <short title>

- **Splits:** one line per split: "<question both sides answer>: <side A, value, file:line> vs <side B, value, file:line>". Or "None: <what the item is instead>".
- **How it left:** <exit>. Closure's words: "<verbatim>" (<file>:<line>).
- **User's words:** "<verbatim question text and Recommended line, or ruling>" (<file>:<line>), answered as <D-number>: "<verbatim>" (<file>:<line>). Or "None found; searched <where>".
- **Code today:** <what it does at the input that matters>, <file:line>, by reading | proven by <run>.
- **Verdict:** PASS | REOPEN | GONE
- **Why:** one or two plain sentences.
- **Side picked without a ruling:** (GONE only, when it applies)
- **Examples:** (REOPEN only) one example from each end and one from the gap where the sides disagree, in game terms, with each side's real value. Quote the code's predicate (its actual test), not a paraphrase.
```

Quote exactly. The main session greps every PASS quote in its cited file, and a quote that is not there turns the PASS into a REOPEN.

End the file with two sections:

- `## Not reached`: the ids you did not finish, so a fresh scout can take them.
- `## Side notes`: anything you saw outside your items that looks wrong, with file:line. Side notes are findings too.

Your final report to the main session: the counts of PASS, REOPEN and GONE, the REOPEN ids with one line each, the ids not reached, and the output file path. Keep it short; the blocks are in the file.
