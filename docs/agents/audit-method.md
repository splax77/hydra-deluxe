# How to run an audit

An audit here means agents reading Hydra's code to find places where it breaks its own rules. Two parts that answer one question differently, a number nobody chose, and a display that guesses a fact the engine knows are typical finds. Each finding then has to leave the audit in a way the user can trust: it gets fixed, it goes to the user as a question, or a user ruling already covers it.

This page is for the main session when it plans an audit, and for every brief it writes during one. Each rule below is here because an earlier audit broke it. "What went wrong before" at the end tells those stories.

The short version: past audits were good at finding problems and bad at closing them. Nearly every miss happened after a finding was written, when someone decided it needed no question. So most of this page is about that step.

## The words this page uses

A **finding** is one problem an audit confirmed against the code.

A **split** is a finding where two parts of Hydra answer one question differently. The Paths tab's backend rows are an example. A plain row reads "Standard" from -10 ms to under 3 ms, and the squeezed-out row reads "Standard SqOut" from -10 ms to under 10 ms. Both sides of a split are things the user sees or relies on.

An **exit** is the way a finding leaves the audit. There are four, listed in the next section.

A **closure** is any exit other than a question to the user. Closures are where past audits leaked.

## The four exits

Every finding leaves by exactly one of these. An audit that reports a finding with no exit is not finished.

1. **Fixed.** The code no longer has the problem. The evidence is the commit that removed it, and a read of the code at that commit. Giving a number a name, moving it to a constant, or rewording a comment does not remove a split. Finding 306 was marked "fixed" because the 10 ms got the name kBackendInnerBandMs (a constant D269 has since deleted), and both edges stayed.
2. **A question to the user.** This is the default for every split and for every finding whose fix changes what the user sees, a stored record, a score or a count. "How to write a question" below says how.
3. **Covered by a ruling.** The user already decided this exact thing. The evidence is a verbatim quote of their ruling, with its file and line, and the quote names both sides of the split. When the user answered a batch with "all recommended", their words are the question's own text and its Recommended line. So that text must name both sides. A ruling about the same topic is not enough. D50 covered "two ladders" (report tiers against row labels), and M39 was a different pair of ladders, so D50 does not cover M39.
4. **Not a problem.** The two parts look alike but answer different questions, or the "copy" is a call to the owner. The evidence is one sentence saying what each side answers, which a fresh agent agrees with. "On purpose", "by design", "deliberately" and "kept" are not evidence. They are claims about what someone wanted, so they need a ruling (exit 3) or they become a question (exit 2).

"Code-only" is not an exit. It describes a fix that changes no displayed word, no stored field, no score or count, and picks no behaviour over another. A code-only finding still leaves by exit 1 once fixed. A split is never code-only, because fixing it picks one side.

## The stages

### 1. Plan

The plan says what the audit reads and what it leaves out, and why. It names the kinds of finding it looks for, the four exits above, each agent's caps and the stop rule. It goes to the user and waits for their yes (CLAUDE.md agent rule 3).

The scope is the user's words, not a narrower version of them. Narrowing it is its own question to the user. The user has set audit scope wide each time: "the first pass should cover everything" (the words behind D113, from card `2026-10-11-8bffee2b`), and "Leave nothing in a "pre-existing, left alone" pile." (D119).

Before any finder starts, the plan lists the decisions and plans the audit must read: `docs/audit/2026-10-03-fix-decisions.md`, `docs/adr/` and the plans under `docs/superpowers/plans/` for the area. The 2026-10-03 audit skipped the one-squeeze-rating-rule plan, so three things the user had already decided showed up as undecided.

Every stage that repeats needs a cap and a line saying what happens at the cap. The 2026-10-03 derivation audit was done only when "a completeness pass finds nothing new". Every pass and round found something, and the user stopped it after round six. A stop rule that can never be met is no stop rule.

### 2. Calibrate

Before the finders start, one agent gets a file with known problems and no hint of what they are. It must find them. The 2026-10-03 audit did this with `src/core/squeeze_rating.cpp` at 707c285, and the agent found both known problems. If the calibration agent misses a known problem, stop and report the miss to the user, and fix the brief before sending anyone else.

The calibration brief must not describe the known problems, even as an example. That audit's first calibration brief worded its example too close to the two bugs, so the run proved nothing and was thrown away.

### 3. Find

Finders each take one family of questions across the code, read-only. A finder reports candidates: each home (the place in the code that answers the question), and a table of inputs and answers for each home, with the edges in. Edges are values like an empty list, exactly 0, a value right at a window edge, or a missing optional. A finder does not grade its own candidates.

Finders search by the question each copy answers, not by a formula or a name, and they split the work by family of questions, not by folder. The 2026-09-24 audit did both of those the other way, and missed a duplicate that a later session found.

A candidate's table is the most valuable thing the audit makes. When a table shows two homes giving different answers for one input, that row is a split. It must reach the user even if the candidate was written up as something else. Finding 306 was titled "a fixed 10 ms nobody chose", yet its own table showed the split at +3 ms. The question that came out of it asked only about the 10.

### 4. Verify

Fresh agents check every candidate against the code. A verifier never gets a candidate from a group it scouted. A candidate survives only if the verifier finds every home it relies on. For a disagreement, the verifier names the input where the homes differ. It marks each claim "by reading" or "proven by" a named run.

The verifier also lists every split its tables show, as separate lines. One candidate can hold more than one split. M39's verifier wrote tables showing two splits: the report tiers against the row labels, and the plain row against the squeezed-out row. The second one was lost because it was never written down as its own line.

Side notes are findings too. A scout or verifier that sees something outside its job writes it down with file and line, and it gets an exit like any other finding. The concept audit's five side notes were parked, and a later session had to check them. Four held, and one showed the brief preamble was wrong.

A claim that needs a live game, a real window or a run to settle stays open until that run happens. It goes on the audit's open list with the run that would settle it. It never silently becomes "by reading".

### 5. Choose each finding's exit

This is the step where past audits leaked. For each finding, and for each split inside it, the main session picks one exit and writes down its evidence:

- for **fixed**, the commit, and what the code at that commit does at the input where the homes used to differ;
- for **a question**, a pointer to the question;
- for **covered by a ruling**, the verbatim quote, its file and line, and the two sides the quote names;
- for **not a problem**, the sentence saying what each side answers.

A closure needs a ruling that names both values. If the main session can't find one, the finding becomes a question. That is the whole rule, and the main session does not get to bend it to keep the question list short. A long question list is the audit working. A short one with silent closures is the failure this page exists to stop.

When the plan has no exit for a case, the finding becomes a question. The concept audit's plan said every K2 finding (same word, different rules) gets a question. Its questions file then added an "Already decided, no question" section, which the plan never defined. M39 left the audit through that section.

### 6. Check every closure

A fresh agent checks every closure: every finding that left by exit 1, 3 or 4. The agent that found or verified a finding never checks its closure. The checker reads the evidence against the code and the cited ruling. It returns one of three verdicts per closure:

- **PASS:** the evidence holds. For exit 3, the quote names both sides.
- **REOPEN:** it doesn't hold. The finding becomes a question.
- **GONE:** the code no longer has the split. The checker names the commit that removed it.

Then the main session checks each exit 3 PASS itself: it greps the quoted words in the cited file. A quote that isn't there turns the PASS into REOPEN. This check needs no judgment, so it gets no exceptions. A script can do the grep for every block at once; `docs/audit/2026-10-29-audit-of-audits/briefs/checkquotes.py` is one. A checker that shortens file names must write a legend for them at the top of its file, or the script can't tell which file a quote claims to come from.

Before a checker calls something "no ruling found", it searches the session cards, not just the decision log. Many rulings were typed in chat and never got a D-record: the 2026-09-24 plan decisions, the 2026-09-26 answers about the 09-25 audit, and the user's words behind finding 53 all live only in cards. Three checkers marked finding 53 "side picked without a ruling" because none of them read card `2026-10-24-a124ccd9`.

### 7. Ask the user

Write each question so the user can rule on exactly what the code does:

- Name both sides and their real values, as the code has them. Quote the predicate, the code's actual test, not a paraphrase.
- Give one example from each end, and one from the gap where the two sides disagree. Use game terms: a note 5 ms after the Star Power end reads "Hard (uncounted)" on one row and "Standard SqOut (uncounted)" on the other.
- Name the options without leaning. Then give a recommendation and the strongest case against it.
- Ask about the split itself, not something next to it. D48 question 4 asked whether 10 ms should follow the leeway, and never said that the other row uses 3 ms.
- Show every value the question asks the user to confirm. Phase 3-5 question 33 asked the user to confirm "the audio reader limits" and "the loading bar's shares and pacing" and showed none of the numbers. Its "all recommended" confirmed nothing the user had seen.
- Read the code again on the day the question goes out, and describe what it does that day. Phase 6 question 4 told the user the Preview's xN disc stays doubled after Star Power ends. A commit the day before had already made it drop.
- Never give a split a purpose nobody wrote down. Say where the split came from (the commit and its author) and that nobody chose it, then ask whether one rule should replace it. The main session broke this in the audit of the past audits, and the user's answer is the rule: "yes everything should use one rule."

Send splits alone or in small groups. The user approved that rule on 2026-10-29 ("yes both"). The number is not the user's: "five or fewer per message" is the main session's own suggestion for what "small" means, so it can change. A batch answer such as "all recommended" covers only what each question's text says. The phase 3-5 file put 33 questions in one message, and its one-line answer settled things nobody had shown the user.

### 8. Record

Each answer becomes a D-record in `docs/audit/2026-10-03-fix-decisions.md`. A record for a split names both sides and quotes the question's text, so a later audit can test "does this ruling name both sides?" by reading one entry. A triage file that tracks findings (such as `docs/audit/2026-10-03-fix-triage.json`) records each finding's exit and its evidence, not just a status word. "fixed" alone is the kind of status that hid finding 306.

The merge that fixes a finding updates its row in the same step. On 2026-10-05 the triage file still showed 192 of 396 findings open after phase 6 had fixed its share, because no merge had marked its rows.

### 9. Fix

Fixes go to executors on Opus, one worktree each, grouped so that no two of them own the same file. Each merge gets one fresh reviewer (D107). A fix that changes what the user sees cites the D-record behind it.

## What every audit brief carries

Every finder, verifier and closure-checker brief starts with `docs/agents/brief-preamble.md`. Then it says:

- the agent's task id, the session id, its files or batch, its caps, and the status rule;
- the four exits, with the rule that only the main session picks an exit;
- that every split in a table is its own line in the report;
- that "on purpose", "by design", "deliberately" and "kept" are claims, not evidence;
- that a search that finds nothing proves nothing on its own (the preamble's rule);
- that no whole-library run happens without the user's `allow library run` in chat (CLAUDE.md agent rule 8). A verifier in the 2026-10-03 audit started `hydra_batch` with no arguments by accident;
- where the user's words can be: the decision log, the question files, the plans, and the session cards in `docs/sessions/`. A card keeps the user's typed words, the label they chose and the start of each question, not the full question or option text, so a closure that needs the full wording finds it only where a D-record or question file copied it;
- for a second-wave agent, the exact ids it takes, not "whatever the first wave did not cover". A second-wave scout in the audit of the past audits was told to skip items that already had blocks. It redid twelve of them and never reached the ones it was sent for.

## What went wrong before

Each story below is a real miss, with where its record lives. Most sources are session cards in `docs/sessions/`, named by date and session id. The rule each story led to is in the stages above.

### The split two audits saw and neither asked about (M39)

The Backend timings table rates each note by how far it lands from the Star Power end. A plain row reads "Standard" from -10 ms to under 3 ms, where 3 is the default of the leeway setting (the most a note can land after the end and still count). The squeezed-out row reads "Standard SqOut" from -10 ms to under 10 ms, where 10 was kBackendInnerBandMs, a constant D269 has since deleted along with both ladders. So a note 5 ms after the end reads "Hard (uncounted)" on one row and "Standard SqOut (uncounted)" on the other. Public Hydra's author added both ladders in commit c9c00cf7 (2025-08-12). That was a year before the user's first commit.

Four steps passed the split by:

1. A 2026-08-21 session asked about "two squeeze rating ladders" (card `2026-08-21-266e74ad`). The user chose "Behaviour-preserving only". That question was about the report tiers against the row labels, a different pair.
2. Derivation audit finding 306 (2026-10-03) had the split in its own table, at +3 and +10 ms. It was titled "Backend rating bands start at a fixed 10 ms nobody chose". Its question became D48 question 4, which described only the 10 ms edges: a row "turns "Easy SqOut" at +10 ms and "Easy" at -10 ms". It never said that the plain row changes at +3. The user answered all 33 questions in that file with "all recommended". The triage then marked 306 "fixed", because the 10 got a name.
3. D50 item 1 reworded the report footer. It settled the report-against-rows pair and nothing else.
4. The concept audit (2026-10-11) re-found it as M39. Verifier V5 wrote both row tables, then matched the finding to D50 item 1 by topic, since both were about two ladders. `docs/audit/2026-10-11-concept-audit/questions.md` listed it under "Already decided, no question", a section the audit's plan never defined.

The user found it on 2026-10-29, when the Path Builder handoff's list of open items named it. Two other items in that same "already decided" list show the same pattern. M04 says "no decision covers that" and is still listed as decided. M60 says the charter "deliberately differs" and names no ruling.

### A question narrower than the code (D51 and D80)

Phase 7 question 4 described a free squeeze as "a squeeze-in at exactly the SP end" (SP is Star Power). The code's rule was "at or before the SP end, however early" (D80). The user approved the batch with "all recommended" (D51), and the fix built from that wording hid row badges the question never mentioned. The user then asked why a 5+ activation had no badge, and the badges were restored as D80 (card `2026-10-06-dcdf0961`). The user's memory file `questions-describe-the-real-rule` holds their reaction and three more cases of the same kind.

### Audits that ignored what was already decided

The 2026-10-03 audit did not read the one-squeeze-rating-rule plan. So the 1 ms cutoff, the orange colour and the flat x1.00 case showed as undecided, though the user had decided them (card `2026-10-03-b5aa7c31`). The rule now: the plan lists the decisions and plans to read before anyone starts.

### A search by name that missed a copy

The 2026-09-24 audit searched by formula and by name, and split its agents by folder. It missed a duplicate that answered the same question under other names (card `2026-10-03-d7f92775`). The next audit split its finders by family of questions instead.

### A calibration that gave away the answer

The 2026-10-03 audit's first calibration brief described the two known bugs too closely in its example. The run proved nothing and was thrown away, then rerun with a clean brief (card `2026-10-03-b5aa7c31`).

### A stop rule that was never met

The 2026-10-03 audit was to stop when a completeness round found nothing new. Rounds 3 to 6 confirmed 26, 14, 11 and 12 new findings. The user stopped it after round 6, and a seventh round after delivery added 44 more (`docs/audit/2026-10-03-derivation-audit.md`, section 3 and "Round 7"). The concept audit's plan then dropped completeness rounds and relied on its phase 7 recurrence gate instead.

### A triage file that drifted

On 2026-10-05 the triage file showed 192 of 396 findings open, though phase 6 had finished its findings. No merge had marked its rows (card `2026-10-05-18216767`). The phase 6 rows were then marked from the merge records, with 16 of them spot-checked against the code, and read-only agents checked the other open rows.

### Side notes left for later

The concept audit's scouts raised five side notes outside their jobs. They were parked, and a later session checked them (card `2026-10-11-8eb32145`). Four were real problems: a function returned the wrong number, four audio headers named the wrong error type, a stem file that another program held open for writing dropped from the Preview with no message, and the brief preamble promised an agent its id on its first hook reply when the hook gave it only after five minutes. The fifth matched Clone Hero and needed no fix.

### What the audit of the past audits found (2026-10-29)

Twelve fresh scouts checked 219 closures from every past audit by the rules on this page (`docs/audit/2026-10-29-audit-of-audits/summary.md`). 24 of them, about one in nine, did not hold. Nearly all the leaks came by two paths.

The first was a question that showed less than the code does. Finding 66 asked about one row of a three-row table. Phase 3-5 question 33 asked the user to confirm a list of tuning numbers by topic and showed none of them. That is the M39 story again, nine more times.

The second was a closure the finding's own agent wrote, with a word in place of a ruling. The concept audit's "code-only" list and its "Already decided" section let six splits out with "kept", "stays" or "left alone". A third kind showed up once. The 2026-10-25 hook rebuild replaced every hook, and its list of what it dropped never named the media guard the user had ruled on (D189). A plan that replaces a whole system lists every ruled behaviour it keeps and every one it drops.

The audit also found a question that described code which had already changed (phase 6 question 4, the xN disc). And the main session made the mistake this page warns about. Asked about M39, it explained the two ladders as one row rating "getting a note in" and the other "pushing it out", a purpose nobody had written down. The user's answer: "why are we even using two different systems? why are you still trying to convince me of more convoluted bullshit?", and then "yes everything should use one rule."

### What these have in common

The finding step worked in every story above. Each miss happened later: a question written about something next to the problem, a closure matched by topic, a batch answer stretched past what its text said, a decision or note nobody read, or a status word standing in for evidence. That is why the four exits and the closure check are the core of this page.

## Sources

The M39 trail is in `docs/audit/2026-10-03-derivation-audit.md` (finding 306), `docs/audit/2026-10-04-phase-3-5-questions.md` (question 4), `docs/audit/2026-10-03-fix-decisions.md` (D48 and D50), `docs/audit/2026-10-11-concept-audit/findings/V5.md` (M39) and `docs/audit/2026-10-11-concept-audit/questions.md` ("Already decided, no question"). The split itself is `BackendSqueeze::summarystr` in `src/core/model.cpp`. The audit of every past audit, which checks every closure by this page's rules, is recorded in `docs/audit/2026-10-29-audit-of-audits/`. It was handed off in `docs/handoffs/2026-10-29-audit-of-audits-handoff.md` (removed; `git show 444631e6:docs/handoffs/2026-10-29-audit-of-audits-handoff.md`).
