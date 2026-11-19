# Tooling derive-once audit: the questions and the user's answers

This file lists every question the user was asked in this audit, in the order asked. Each one says what was asked, the two sides it named, the rows it covers, the user's answer, what the answer did to those rows, and the D-record that holds it.

The source for every answer is `log.md`. The user's words are copied from there exactly, typos included. A label in quotes, such as "The code wins (Recommended)", is the option the user clicked. A sentence in quotes with no option label is what the user typed.

These are answers given in chat. They are not filed rulings under `docs/agents/rulings/`. Only three answers rest on a filed line, and each says so with the file and line.

Where `log.md` holds only the option text, and not the question as it was sent, this file says so. In those cases the two sides are taken from the theme's text in `reports/TOOL-AUDIT-T8-G1.md` (G1 below), and are marked as G1's words. Theme numbers are G1's. Row ids are written as the ledger writes them (L-F01-06).

## Before the audit started: the plan's three questions

The plan, `docs/superpowers/plans/2026-11-18-tooling-derive-once-audit.md` (branch `claude/tooling-derive-once-audit-plan`, section "Questions and the user's answers"), asked three questions before any agent ran. The user answered on 2026-11-18. Question 1, on scope: "why is there a whole folder of retired hooks? delete that shit", plus the labels "Memory notes", "Skills and /handoff" and "Hydra tools and hook tests". Question 2, on size: "16". Question 3, on hook gaps in the records: "Keep steps out (Recommended)". The user then approved the plan with "Approve (Recommended)" (log.md, line 3).

These set the audit's scope and size. No ledger row leaves by them. They are recorded in the plan, and none of D345 to D350 holds them.

## T6: the four answers on splits (2026-11-18, D345)

These four came after the user said "every split? im not manually reviewing hundreds of questions". At that point 258 ledger rows carried a split (log.md, "T6: the user's four answers").

### Q1. Doc against hook

**Asked:** "when a doc, brief or CLAUDE.md line says something different from the hook or script that actually enforces it (example: rule 5 says the posture file comes "each prompt", but posture.py sends it on the first prompt, every 60 minutes and after compaction), which side wins?"

**The two sides:** the doc, brief or CLAUDE.md line, against the hook or script that enforces the rule.

**Answer:** "The code wins (Recommended)". Option text: "The doc is fixed to point at the hook and stop restating it. Closes those splits as covered."

**Rows:** the 79 category A rows in `exits.md`. In the T8 fold, the doc-against-code half of 14 more rows: L-F07-02, L-F09-12, L-F09-13, L-F10-23, L-F13-06, L-F14-01, L-F14-03, L-F17-02, L-F18-18, L-F18-20, L-F20-07, L-F22-04, L-F22-09 and L-F26-15. Q12 also sends theme 4's phrase half here.

**What it did:** exit 3. The doc is fixed to point at the code. That fix is still to be made.

**D-record:** D345 item 1.

### Q2. Memory notes

**Asked:** "when a memory note disagrees with the docs or the code (examples: a note names a different GitHub repo owner than CLAUDE.md and the repo; a note restates the 150-call cap), which side wins?"

**The two sides:** the memory note, against the docs or the code.

**Answer:** "Note is wrong (Recommended)". Option text: "Memory notes are a session's notes, not yours; the note gets fixed or pruned."

**Rows:** the 33 category B rows in `exits.md`. In the T8 fold, the memory half of L-F06-15, L-F07-02, L-F07-17, L-F14-01, L-F15-01, L-F19-01, L-F24-12, L-F26-02, L-F26-04, L-F26-15, L-F30-15, L-F31-01 and L-F32-10, and the "what a constant means" half of L-F26-13.

**What it did:** exit 3. The note gets fixed or pruned, in the fix plan. Q21 later overruled this answer for one note, one-build-per-wave.md.

**D-record:** D345 item 2.

### Q3. Stale references

**Asked:** "docs that cite a file, path or line number that has moved (examples: a call site cited at line 2009 that is now at 2204; a squeeze-window owner cited in graph.h that now lives in model.h): fix them to the current location without asking?"

**The two sides:** the old location a doc cites, against where the thing is now.

**Answer:** "Fix without asking (Recommended)". Option text: "Mechanical: point at where the thing is now."

**Rows:** the 21 category C rows in `exits.md`. In the T8 fold, the moved-pointer half of L-F03-02, L-F11-04, L-F18-14, L-F21-09 (D49's missing file name), L-F22-13 and L-F28-06.

**What it did:** exit 3. The pointer gets fixed, in the fix plan.

**D-record:** D345 item 3.

### Q4. The splits left over

**Asked:** how the splits left after Q1 to Q3 should come to the user: "two hooks deciding one thing differently, two filed rulings that disagree, real behaviour choices".

**The two sides:** log.md gives only the chosen option.

**Answer:** "Grouped by theme (Recommended)". The option: one question per theme, each listing its rows, a few at a time.

**Rows:** none leave by this answer. It decided how Q5 onward were asked. The T6 sorters then placed 238 rows in category Q (exit 2), and G1 grouped them into 20 themes.

**D-record:** D345 item 4.

## T8 batch 1 (2026-11-18, D346)

### Q5. Claimed rulings (theme 1)

**Asked:** "24 places in code or docs say "the user ruled/answered" with no matching line under ## Confirmed (examples tools/ch_mcp/quiet.py:43 and gates/README.md:19)."

**The two sides (G1's words):** "keep the claim and file the ruling", against "drop the claim and treat the value as an unconfirmed pick".

**Answer:** "Drop the claim (Recommended)". Option text: "The value stays as a default; the comment stops naming you. You file a line later only if you want it to bind."

**Rows:** theme 1's 24 rows. In the T8 fold, the claimed-ruling half of L-F19-01 and L-F32-10.

**What it did:** exit 3. Each comment or doc stops naming the user, and the value stays. The fix is still to be made.

**D-record:** D346 item 1.

### Q6. Code against code (themes 6 to 9, and the code half of themes 5 and 15)

**Asked:** log.md holds the option text only, not the question as sent.

**The two sides (G1's words):** two code paths answer one question differently. Theme 6 is a quote-blind cut against a quote-aware one. Theme 7 is a strict reading of an id, hash or key against a looser one. Theme 8 is two scripts holding different values for one setting. Theme 9 is two gates or tools answering one question differently.

**Answer:** "Merge, I pick (Recommended)". Option text: "Each pair becomes one owner using the stricter or safer reading; every choice is listed in the fix plan, which still needs your yes."

**Rows:** themes 6 (7 rows), 7 (19), 8 (11) and 9 (26). From theme 5, the command-list half, which G1 gives as L-F12-07, L-F13-04 and L-F16-03. Theme 15's code half was re-checked in T8, so the fold decides those rows. In the T8 fold, the code-against-code half of L-F03-02, L-F08-09, L-F09-12, L-F19-01, L-F21-04 and L-F25-19.

**What it did:** exit 3. Each pick goes in the fix plan, which needs the user's yes.

**D-record:** D346 item 2.

### Q7. Rules with no home (themes 2 and 3)

**Asked:** log.md holds the option text only.

**The two sides (G1's words):** theme 2 is "give it a home" against "leave it in memory". Theme 3 is "add a check" against "instruction only".

**Answer:** "Give each a home (Recommended)". Option text: "Move each into agent-rules.md or the preamble; add a hook only where a check is cheap and exact (like the scout write limit). Listed in the fix plan."

**Rows:** theme 2's 17 rows and theme 3's 10. In the T8 fold, the "no history in comments" half of L-F26-13.

**What it did:** exit 3. Each rule gets a home in the fix plan.

**D-record:** D346 item 3.

### Q8. The status rule (theme 16, two Confirmed lines)

**Asked:** log.md holds the option text only. The question was which of two filed lines rules: `docs/agents/rulings/review-and-hooks.md:33` and `:34`.

**The two sides:** the Confirmed line at `review-and-hooks.md:33`, "demand status every 10 tool calls and reject the next call after 15" (2026-09-04, session 48cdbb34), against the Confirmed line at `:34`, "demand status updates every 5 minutes of real time and reject any calls with no updates after 10 minutes" (same date and session).

**Answer:** "Minutes (line 34) (Recommended)". Option text: "Matches rule 7 and the hook; line 33 is retired."

**Rows:** theme 16's calls-against-minutes rows, which G1 names as L-F01-01, L-F01-08 and L-F07-08.

**What it did:** exit 3. Line 34 rules and line 33 is retired. Line 33 still sits under `## Confirmed` today (read 2026-11-19), so its retirement is on the fix list.

**D-record:** D346 item 4.

## T8 batch 2 (2026-11-18, D347)

### Q9. What counts as the user's words (theme 10)

**Asked:** log.md holds the option text only.

**The two sides (G1's words):** "Confirmed only", against a wider list that also accepts a session card's quote, a brief's quote, chat and plans.

**Answer:** "Confirmed only (Recommended)". Option text: "A card or chat quote can point to a ruling but never closes anything; the reviewer and premise-map pages are fixed to say so."

**Rows:** theme 10's 8 rows.

**What it did:** exit 3. The reviewer and premise-map pages get fixed, in the fix plan.

**D-record:** D347 item 1.

### Q10. What an agent does at the warning (theme 16)

**Asked:** log.md holds the option text only.

**The two sides (G1's words):** three texts say three things. `writer-brief.md:101` says "stop starting new cards", `progress.py:94-100` says "commit and report", and `reviewer.md:11` says "stop fixing, sign off".

**Answer:** "Finish, report the rest (Recommended)". Option text: "Finish the item in hand, commit or sign off what passes, list what's left. The main session decides about a fresh agent; reviewers never hand on."

**Rows:** theme 16's warning row, L-F01-02.

**What it did:** exit 3. The three texts become one rule, in the fix plan.

**D-record:** D347 item 2.

### Q11. Which worktree folders the delete rule trusts (theme 5)

**Asked:** log.md holds the option text only.

**The two sides (G1's words):** the narrow list, where `delete_rule.py:14-18` trusts only `.claude\worktrees` and the user-level folder, against the wider practice, where `cleanup_sweep.ps1:174-178` also trusts `<root>\worktrees`.

**Answer:** "Trust both". Option text: "The delete rule accepts both folders."

**Rows:** theme 5's folder rows. G1 names L-F10-11, and exits.md gives L-F07-11, L-F13-05 and L-F33-02 the theme "worktree folder location".

**What it did:** exit 3. The delete rule is changed to accept both folders, in the fix plan.

**D-record:** D347 item 3.

### Q12. What counts as a whole-library run (theme 4)

**Asked:** log.md does not give the question as sent. It says the question offered "compare over thousands of charts without the flag" as its case.

**The two sides (G1's words):** the docs, "any run over the whole library, phrase asked for in chat", against the hook, which checks only the library flag and accepts the phrase in any case with a trailing . or !.

**Answer:** the user asked a question back instead of choosing: "is this a real concern or an edge case you invented".

**What the main session checked:** `src/cli/batch_scope.h` makes hydra_batch refuse a run over the whole library unless its library flag is given. So the flag is the only way to start one, and the hook's flag check in percall.py covers it. The case in the question was the main session's own framing, not a case anyone found.

**Rows:** theme 4's 5 rows: L-F12-10, L-F13-22, L-F17-01, L-F18-01 and L-F20-02.

**How they left:** log.md splits the theme into two halves, not by row. The runs half leaves by exit 4, because the docs' "any run over the whole library" and the hook's flag check answer the same question once the program makes the flag the only door. The phrase half (review_rules.py's tolerance for case and a trailing . or !) is a doc against code, and follows Q1, "The code wins".

**D-record:** D347 item 4.

## T8 batch 3 (2026-11-18, D348)

### Q13. Doc against doc

**Asked:** "about 25 real rows are two docs giving different instructions with no code deciding (examples: model-postures.md:11 and derive-once-review.md:198 build each branch before review, integrate.md:41 builds the joined branch once; CLAUDE.md:9 has any agent update the Clone Hero page, brief-preamble.md:105 and docs/clone-hero/README.md:80 have an agent whose brief does not own the page report the fact)."

**The two sides:** each pair of docs that give different instructions where no code decides.

**Answer:** "Merge, I pick (Recommended)". Option text: "Same as your code-vs-code answer: each pair becomes one home with the stricter or safer reading, the other points at it; every pick is listed in the fix plan for your yes."

**Rows:** in the T8 fold, the doc-against-doc half of 33 rows: L-F06-15, L-F06-16, L-F09-13, L-F13-06, L-F13-09, L-F13-23, L-F14-01, L-F14-03, L-F15-15, L-F17-16, L-F18-02, L-F18-12, L-F18-14, L-F18-20, L-F18-22, L-F20-01, L-F20-07, L-F22-24, L-F24-09, L-F24-12, L-F26-02, L-F28-13, L-F28-15, L-F28-21, L-F29-01, L-F29-03, L-F29-08, L-F29-13, L-F29-18, L-F30-09, L-F30-10, L-F31-14 and L-F32-01.

**What it did:** exit 3. Each pick goes in the fix plan, which needs the user's yes.

**D-record:** D348 item 1.

### Q14. The line budget, first time asked (theme 17)

**Asked:** log.md does not give the question's text. It covered L-F04-07, L-F07-16 and L-F21-06.

**The two sides (G1's words):** the Confirmed line at `review-and-hooks.md:13`, where the user chose "Raise cap to 3,050", against `replay.py:41`, `LINE_BUDGET = 3100`.

**Answer:** the user asked a question back instead of choosing: "what is the current size?"

**What the main session checked:** replay.py's own line_total over `C:\Users\Patrick\.claude\gates` gave 5,824 lines over 27 files. That is against LINE_BUDGET 3100 (replay.py:41) and the Confirmed 3,050. Nothing runs replay.py as a gate. Later, in session 0e741144, the count was 6,045 lines, after the Workflow-reviewer merge (log.md, "T8: the fold and its closure check").

**How the rows left:** they stayed open, and the question was asked again as Q16. Two hook-size scouts then measured what could be trimmed. log.md gives the user's remark that followed only in its own words ("The user said a fixed budget made around the features of the day does not fit new features"), not as a quote, so it is not recorded as the user's words here.

**D-record:** D348 item 2.

### Q15. Session cards held up by the docs check

**Asked:** this came from the user, not from a ledger row. The user asked "why is the docs check failing on a research note? why are session cards getting held up by a research note? why does this design exist?"

**The two sides:** the docs check read untracked, unignored files as part of the tree (`tests/source_tree.h:58` listed files with `git ls-files --cached --others --exclude-standard`), and the card hook commits only after the whole docs check passes. So an untracked research note stopped the cards.

**Answer:** after the main session explained, the user said "go ahead".

**How it left:** the docs check now reads only committed or staged files (merge 2722ab29 on main, reviewed clean). The seven waiting session cards were committed (16ebaff6). The single-owner scan keeps its committed-plus-new list. No ledger row leaves by this.

**D-record:** D348 item 3.

## T8 batch 4 (2026-11-19, D349)

### Q16. The line budget, asked again (theme 17)

**Asked:** "replay.py's LINE_BUDGET = 3100 is called "the user's hard budget", the only Confirmed number found was 3,050, hook code is 6,045 lines and nothing enforces a cap; what replaces it?"

**The two sides:** the comment's "the user's hard budget" of 3,100 (and the Confirmed 3,050), against 6,045 lines of hook code with no cap enforced.

**Answer:** the user asked a question back instead of choosing: "why does the comment call it the user's hard budget if nothing was ever enforcing that budget? sounds like we never actually had a budget".

**What the main session checked:** the 3,000 came from the hooks-from-scratch plan, where session b8d57825 chose "Trim to under 3,000 (Recommended)". replay.py (hooks repo 2eb0990, 2026-10-25) was a one-off replay proof run by hand, and no hook or settings entry ever ran it. The cap was raised to 3,050 (2026-10-25) and to 3,100 (8da257a, 2026-10-30) when a session's count went over. Then on 2026-10-31 the user said "im not too concerned about the exact number as much as efficiency and functionality. if a hook is doing something unnecessary or contains a lot of arbitrary rules that needs to scrutinized". That line is filed under `## Confirmed` at `docs/agents/rulings/review-and-hooks.md:11`. The T8 re-check scout R2 missed it.

**Rows:** L-F04-07, L-F07-16 and L-F21-06.

**How they left:** exit 3, by the Confirmed line at `review-and-hooks.md:11`. The comment's "user's hard budget" claim goes, and the size is information only.

**D-record:** D349 item 1.

### Q17. Scrolling during render checks (theme 20)

**Asked:** "the note render-checks-use-the-corpus.md says "Scroll to the middle of the list, click a song" with no ask, against the Confirmed 2026-10-30 line at ui-and-display.md:27; which rule holds?"

**The two sides:** the memory note's "Scroll to the middle of the list, click a song", against the Confirmed line at `docs/agents/rulings/ui-and-display.md:27`: "ive had enough of this shit. something like 10 times now you waste my time by scrolling thorugh the entire library for minutes to reach the song you want with no explanation. this should never happenw tihout my approval." (2026-10-30, session 83eb8bac).

**Answer:** "delete that memory note. search function exists for a reason."

**Rows:** L-F03-20 and L-F14-10.

**What it did:** exit 3. The main session deleted the note, its MEMORY.md line and its three links the same day. No doc held the scroll step, and the Confirmed no-scroll line stands.

**D-record:** D349 item 2.

### Q18. `gh issue close --comment`, first time asked (theme 19)

**Asked:** log.md does not give this first question's text. It covered L-F13-08, L-F16-08 and L-F13-17.

**The two sides (G1's words):** `issue-tracker.md:14` "hand-types gh issue close --comment", against `this-machine.md:15`, "text never goes through hand-typed gh".

**Answer:** the user asked a question back instead of choosing: "is this a real concern or an edge case you invented".

**What the main session checked:** splax77/hydra-deluxe has had one issue ever. It was closed on 2026-09-05 with a comment posted one second before the close, before the publish gate existed (2026-10-25).

**How the rows left:** they stayed open, and the question was asked again as Q20.

**D-record:** D349 item 3.

### Q19. constants.py's swapped formula tables

**Asked:** log.md does not give the question's text. It covered L-F27-27, whose split is that `docs/clone-hero/03-timing.md:355` proves constants.py's two formula tables are swapped.

**The two sides:** the names in `tools/ch_probe/constants.py`, against the disassembly that `03-timing.md:355` records.

**Answer:** the user asked questions back instead of choosing: "if no value changes then why is this even a question? what probe is reading these? are the values ever printed where i will see them?"

**What the main session checked:** `EngineModel.constants()` in `tools/ch_probe/engine.py` is called only by `tools/ch_probe/experiments/milestone1.py`, an experiment run by hand, and by the tests. No MCP tool or screen prints the values.

**How the row left:** Fix pending. The two tables are renamed to match the disassembly, and no value changes. No further question was asked.

**D-record:** D349 item 4.

### Q20. `gh issue close --comment`, settled

**Asked:** "the comment on close posts text with no gate; fix the doc line so it goes through the publish gate, or drop the item?"

**The two sides:** fix `issue-tracker.md:14` so a closing comment goes through the publish gate, or drop the item.

**Answer:** "Fix the doc line".

**Rows:** L-F13-08, L-F16-08 and L-F13-17.

**What it did:** exit 1. `docs/agents/issue-tracker.md:14` now says to post any closing comment with `publish.ps1 -Kind issue-comment`, then a plain `gh issue close`, matching line 47 (commit f2a89466, merge 6acfbed4 on main). These three rows are the one exception to the plan's "no row leaves as exit 1", made on the user's direct request.

**D-record:** D349 item 5.

## T8 batch 5 (2026-11-19, D350)

### Q21. Whether a wave's build count needs the user's yes (theme 14)

**Asked:** "Before agents launch, must the build count get your yes?"

**The two sides:** the memory note `one-build-per-wave.md`, "before launching work that will need builds, say how many builds it needs and get a yes", against `docs/agents/agent-rules.md:9` (rule 3), "The main session's compiling and test runs inside approved work need no separate yes (rule 14)." The note comes from the user's 2026-11-18 words "3 builds at once? what? i didnt accept doing lots of builds. what do you need all those builds for?" and their choice "One build per wave", in session 8e6a2908.

**Answer:** "Yes, state the count".

**Rows:** the memory halves of L-F29-01, L-F29-18 and L-F32-01. Their doc-against-doc halves left by Q13.

**What it did:** exit 3. The note's rule wins over Q2's "Note is wrong" for this one note. The fix plan changes rule 3 to say the build count is stated and needs a yes before work that needs builds launches, and gives the one-build-per-wave rule its home in agent-rules.md.

**D-record:** D350 item 1.

## Every exit 2 row and its question

T6 put 238 rows at exit 2 (category Q in `exits.md`). A script over `exits.md`, G1 and `fold-T8.md` confirms that all 238 are in G1, each once. The 115 rows of `fold-T8.md` are exactly themes 11 to 15 and 17 to 20 (96 rows) plus the 19 rows T7 reopened, which are in no G1 theme.

For the 115 fold rows, `fold-T8.md` gives each row its exit, and it wins over the theme. Most fold rows left by an answer to Q1, Q2, Q3, Q6, Q7 or Q13. The re-check found 47 of them not a split (exit 4) and 2 gone. The twelve fold rows that went to the user as new questions are Q16 (L-F04-07, L-F07-16, L-F21-06), Q17 (L-F03-20, L-F14-10), Q18 and Q20 (L-F13-08, L-F16-08, L-F13-17), Q19 (L-F27-27) and Q21 (L-F29-01, L-F29-18, L-F32-01). fold-T8.md says no row of the 115 is still a question.

The other 142 rows are in themes 1 to 10 and 16, and leave by theme. Theme 1 goes to Q5. Themes 2 and 3 go to Q7. Theme 4 goes to Q12. Theme 5's folder rows go to Q11, and its command rows to Q6. Themes 6 to 9 go to Q6. Theme 10 goes to Q9. Theme 16's calls-against-minutes rows go to Q8, and its warning row to Q10.

Four theme 16 rows are named by neither Q8 nor Q10: L-F01-04 and L-F04-05 (the 500K context warning's text, stall.py against model-postures.md and the settings), L-F01-09 (the git exemption for chained commands, the preamble against progress.py:119) and L-F28-02 (a stuck agent nudged or stopped, model-postures.md:11 against stall.py:124,143). The main session placed them, with L-F04-14, which never reached exits.md, in `late-folds.md`, each under an answer above. The T9 closure check passed all five.

The T9 closure check (`briefs/T9-closure.md`, `reports/TOOL-AUDIT-T9-K1.md` to `K8.md`) then read the 143 rows that left by theme or late fold: PASS 121, REOPEN 18, GONE 4. The main session refolded each REOPEN and GONE from the checker's reading (`scripts/final_exits.py`, `T9_REFOLD`, each with its checker). Two of them went to the user, as Q22 and Q23. After them, no row is a question.

### Q22. Which worktree folders the hooks trust, asked again (theme 5)

**Asked:** "Worktree folders: every task worktree goes in hydra-test\worktrees (the prompt hook tells the main session so), and that folder now holds 123. But the delete rule (delete_rule.py:17-18) trusts only hydra-test\.claude\worktrees and ~\.claude\worktrees, and the start-of-session sweep (cleanup_sweep.ps1:174-178) trusts .claude\worktrees in any repo plus ~\.claude\worktrees. So agents can't delete inside their own worktrees, and the sweep never removes them. Your earlier "Trust both" was asked when the sweep was thought to trust that folder already. Which fix?" Options: "Trust both folders (Recommended)", "Move to .claude\worktrees".

**Why it was asked again:** T9 checker K8 found that Q11's side "cleanup_sweep.ps1:174-178 also trusts `<root>\worktrees`" holds only for the hooks repo, not Hydra.

**Answer:** "if i already picked an option why are you asking again". So Q11's "Trust both" stands, read as the user meant it: the delete rule and the sweep accept `<repo>\worktrees` as well as `<repo>\.claude\worktrees`, in both repos.

**Rows:** L-F07-11, L-F10-11, L-F13-05, L-F33-02. **What it did:** exit 3 by Q11. **D-record:** D351 item 1.

### Q23. The rules that left with the deleted render-check note

**Asked:** "Lost rules: the memory note you had deleted (render-checks-use-the-corpus.md) also held four rules that no doc has: zoom a capture 2x and check gaps, alignment and baselines before showing it; a test that compares against the rule's own output proves nothing; if you say "just merge", still say plainly the real-app check wasn't done; grep for code that depends on what changed. The scroll step stays gone either way. Keep them?" Options: "Move into ui-testing.md (Recommended)", "Let them go".

**Answer:** "Move into ui-testing.md (Recommended)".

**Rows:** L-F20-12 (T9 checker K6 found the 2x-zoom rule's only home gone). **What it did:** exit 3; the four rules get a home in docs/agents/ui-testing.md, in the fix plan. **D-record:** D351 item 2.
