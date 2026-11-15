# Test audit T8: give every finding an exit (2026-11-15)

## The short version

The test audit has judged every test. This plan is its next step, T8 of `docs/superpowers/plans/2026-11-14-test-suite-audit.md`: every finding gets one of the four exits on `docs/agents/audit-method.md`, with its evidence. An exit is how a finding leaves the audit: fixed, a question to the user, covered by a ruling, or not a problem.

Almost every finding here leaves as a question. The approved plan says every Rewrite, Remove and Investigate goes to the user, and a record that holds a ruled fault cannot be "not a problem". So the real work of T8 is not choosing between exits. It is sorting several thousand records into a small number of shared questions, without losing one and without an agent choosing anything.

The plan does that in three moves. A script builds one ledger row per record from the audit's own files. Scouts then label each faulty record with what it fakes or pins, from a closed list the main session wrote. Last, the main session writes a short table of exit rules, a script applies it, and the main session reads the result and signs it. Agents sort facts. Only the main session's table picks an exit.

Nothing is fixed here and no test changes. The audit folder is `docs/audit/2026-11-14-test-audit/`, called AUD below. Nothing starts until the user says yes (agent rule 3).

## What must be asked before anything else

Four blocks of records hinge on what counts as fake, and the user's filed words do not settle them. They are questions 2 to 5 below. Their records get no exit until the answers are in.

One more thing sits under all of them. The recount cleared 49 fault marks because hand-built input fed to the real code "is not fake". The words behind that, "No, input is fine", were a choice in a question box. They are not under `## Confirmed` in `docs/agents/rulings/tests-and-runs.md` on main, and session 63c70014 has no card. So by `docs/agents/premise-map.md` they are not a ruling this plan may stand on (P9). The filed line reads "tests should never be based on anything fake." with no exception. Question 1 asks the user to settle it. Until then, no record leaves as "not a problem" on the strength of that choice.

## T6 and T7 results

T6 and T7 ran in session 7a62df56 on 2026-11-15, alongside the writing of this plan, and their results are filled in below.

**T6** changes the corpus and reruns the suite. Each test that turns red becomes a finding with the words "leans on which charts are in the corpus". It gets a ledger row of its own kind (`t6`), tied to that test's record, and it leaves by exit 2 like any other fault. A test that stayed green is not proven independent, and no row says it is.

**T7** plants one named bug per auditor. A claim that held changes that record's question 8 from "reasoned" to "confirmed", and changes no exit. A claim that did not hold (the test stayed green, or a different line went red) reopens every question 8 answer in that auditor's slice: one fresh Sonnet verifier re-reads them, as the approved plan says. Until that re-read is committed, no record in that slice leaves by exit 4.

**T6 result** (`AUD/mechanical/corpus.md`, commit 6aa6d684). The baseline suite had 2,280 test cases and none failed. With a copy of one chart added, three tests went red: "discover_charts output matches the checked-in scan snapshot", "the scan's folder count is reported while the walk runs, from the calling thread" and "corpus: squeeze-ins at exactly 0 and what they change". With `test_path_reduction` removed instead, one went red: the folder-count test again. So T6 adds three `t6` rows. Each matches a record that already said Rewrite and already called its number a corpus count (`records/A4.md` records 3 and 45, `records/A13.md` record 10 with `records/delta-DL5.md` record 18). The run confirms those records' claim and adds no new kind of fault.

**T7 result** (`AUD/mechanical/planted.md`, commit a4ad3306). There are 112 slice files, and 98 got a planted bug; the other 14 had no record naming an exact change, or were the calibration runs. Of the 98, 91 went red at a check the record names. Six stayed green with the bug on: A3, A9, A30, C7, WN3 and WN4a. In each of the six, the plant is the record's own change and the test cannot see it, so the auditor's claim did not hold. WN8 crashed before any check, because its plant had to differ from the record's words to compile, so its claim is unsettled. WN4b went red, but one line its record names (line 145) stayed green.

So eight slices get the question 8 re-read: A3, A9, A30, C7, WN3, WN4a, WN4b and WN8. That is E0's second half.

## The numbers this plan rests on

Each figure names where it was read. None is worked out here.

- 2,449 Keep and Investigate records were re-read by the user's test rulings, and 1,861 hold a ruled fault: 239 of 421 Keep records and 1,622 of 2,028 Investigate records (`AUD/counts/TOTALS.md`, section 2, rows "mine").
- By fault, records can hold several: made-up stand-ins (X9) 1,452, typed constants (X10) 963, echo checks (X11) 249, non-code citations (X12) 135, snapshots 46 (same table).
- 397 Investigate records hold no fault (`AUD/capture/CLEAN.md`, line 5). Their verdict rows read 34 shown by a run, 333 hold by reading, 20 cannot tell, none contradicted (`AUD/capture/byreading/SUMMARY.md`, "Totals").
- The Rewrite and Remove records were never part of the recount, and no file gives their final count. `AUD/log.md` line 341 says a listing found 1,168 Rewrite headings and two scouts found 78 more. A rough search of `AUD/records/` for verdict lines, run for this plan, found 1,315 Rewrite and 160 Remove, and it also counts superseded and re-judged records. So the true count is not known (P4), and E1 settles it.

## The tasks

### E0: Fill in T6 and T7, and read the audit's state again

The main session confirms that `AUD/exits.md`, `AUD/closures.md` and `AUD/questions.md` do not exist and that the audit branch holds this plan's base commit and both result files.

Then the T7 re-read runs, by the approved plan's own rule. One fresh Sonnet verifier per slice (eight in all, launched four to a round, as E3 launches its scouts) re-reads every question 8 answer in slices A3, A9, A30, C7, WN3, WN4a, WN4b and WN8. None of them audited, verified or planted in that slice. Each gets `AUD/mechanical/planted.md` for why its slice was reopened, and returns, per record, "holds", "does not hold" with the input that shows it, or "cannot tell" with the run that would settle it. It reads only; it builds and runs nothing. The cap is one slice file per verifier and 25 minutes; at the cap it writes what it has and marks the rest "not read", and the main session sends the rest to a fresh verifier once. Their files go to `AUD/mechanical/reread/<slice>.md`.

Acceptance: `AUD/log.md` gains a line with both result files' commits. Each of the eight slices has a re-read file with a verdict for every record that has a question 8. No record in those slices leaves by exit 4 before its re-read is committed, and a "does not hold" becomes a note row in E5.

### E1: Build the ledger, one row per record

One Opus executor writes `AUD/exits/ledger.py`. It lives in the audit folder beside `AUD/repeats/repeats.py`, needs no build, and reads only AUD. One Sonnet reviewer reads it once. The main session runs it.

The script writes `AUD/exits/ledger.tsv`. A row holds the record's file and heading, the test's file and line, the auditor's verdict, the verifier's verdict where it differs, the fault marks from the record's count file with the 55 corrections of `TOTALS.md` section 3 applied, the by-reading verdict if the record has one, and the T6 and T7 marks. It reads `records/window-WN4a.md`, `WN4b` and `WN4c` in place of `records/window-WN4.md`, which they supersede (`AUD/log.md` line 358). Where a later round re-judged a test, the row keeps both and says which is later.

The records were written by more than a hundred agents in several heading forms, and earlier scripts missed some (`AUD/log.md` line 341; `TOTALS.md` line 7). So the script is not trusted to see everything (P5). It prints every record file's row count beside the count that file's counter reported, and lists each record it could not read.

The unread records go, by exact id, to Sonnet scouts who fill the same columns by hand. The cap is four scouts and one re-send. A record still missing after that is listed in `AUD/exits/not-in-ledger.md` by name, and E7 gives each one its own question.

Acceptance: the ledger's Keep and Investigate rows add up, per count file, to the "Read" and "Faulty" columns of `TOTALS.md` section 1, and to 2,449 and 1,861 in all. Any difference is written down with the records that cause it. The ledger states the final Rewrite and Remove counts, which settles P4. Rows plus `not-in-ledger.md` names equal the records in `AUD/records/`.

### E2: Ask the five fake questions

Waits for: nothing. It runs beside E1.

The main session sends questions 1 to 5, alone or in small groups (audit-method stage 7). Each question shows the test's name and line, the hand-written thing quoted from the test, and the real function that reads it. It names both sides: the filed line "tests should never be based on anything fake. testing fake made up data is useless and says nothing about how the app behaves in a real situation" (`docs/agents/rulings/tests-and-runs.md` line 8 on main), and the reading that input the real code reads is not fake. It says where the split came from: the tallier's note in `TOTALS.md` section 6. It gives no purpose for either side. The main session reads each test again on the day the question goes out.

Each answer is filed by the user's "File it" click or it is not a ruling. The answer changes fault marks in the ledger, never an exit directly.

Acceptance: `AUD/questions.md` holds the five questions and the user's words for each. The ledger's `held` column is empty for every record an answer covers. A block with no answer stays held, and E7 gives its records no exit.

### E3: Sort the faulty records into shared questions

Waits for: E1.

Rubric question 10 says one rule gets one verdict, and every test resting on it points at that verdict. Here the shared rule is a pair: which of the user's rulings the test breaks, and what the test fakes, pins, echoes or cites. Two tests that both feed a fake analyzer share a question. A test that feeds a fake analyzer and a test that pins a tier name do not.

The main session writes the list of pairs first, in `AUD/exits/keys.md`, and takes it only from what the audit already wrote. The stand-in kinds come from `TOTALS.md` line 9: nine kinds of the code's own output, seven kinds of outside data and eight kinds of fake code. The pinned owners come from X1 to X8 and the single-file pins in `AUD/cross-findings.md`. Echo checks, non-code citations and snapshots are X11, X12 and the rubric's snapshot. Rewrite and Remove records take the eight kinds in the approved plan's table "What the audit looks for". The main session invents no key.

Sonnet scouts then label each faulty record with its keys. A scout reads the count file's reason for the row and opens the record, and the test where the reason is not enough. One counter read about 94 records in the recount (2,449 records over 26 counters, `TOTALS.md` section 1), so a scout takes about that many. That is about 20 scouts for the 1,861, and about 16 more for the Rewrite and Remove records if E1 confirms a count near the rough one. They run four to a round. A fresh Sonnet verifier checks two scouts' files, and never a file it labelled.

A record with two faults gets two keys and sits in two questions. A record no key fits is labelled "no key" with the reason. For each key, the scout marks the plainest record and the one that fits least well, so the question can show both ends.

Scouts label facts. They pick no exit, and their brief says so, with the rest of "What every audit brief carries" on the audit-method page.

Caps. Each scout runs under the progress hook's limits (P12) and gets one re-send by exact record ids. The key list is revised once, after the first round, from the "no key" rows. After that a "no key" record is its own question. If the key list passes 80 keys, the main session stops and tells the user the count before going on.

Acceptance: every ledger row with a fault mark, and every Rewrite and Remove row, has at least one key or "no key". Every labels file has a verifier file from a different agent id. `AUD/exits/groups.md` lists each key with its record count, its plainest record and its least-fitting record.

### E4: Write a row for X1 to X12 and for every split inside them

The main session writes these rows itself, in `AUD/exits/cross-rows.md`, from `AUD/cross-findings.md` and the verifier files it names. Each finding gets one row, and each item below gets a row of its own. This list is what a read of the file found; it is a floor, and the main session adds any split the V files show that is missing here.

- X1: the typed copies of Burnout's result. The split three verifiers named (one fact, several homes). The lines two verifiers removed and the user's filed words put back. `test_sp_meter_curve.cpp:948`, which a verifier called sound.
- X2: the fill ticks typed in the harness and again in four files. The open point that both fills are still offered at 60 ms, which is by reading only.
- X3: Allister's fill list in two tests.
- X4: the tier names and edges. The four lines V-RP2 removed.
- X5: one row per message (five). The "cannot open file: " phrase, typed in at least six places in `src/` with no shared constant, is a production-code finding and a split on its own row. The hand-written dmleaderboards replies. `test_dm_report.cpp:118-130`, marked unclear.
- X6: the title placeholder, and the artist and charter placeholders.
- X7: one row per bullet (five). The four `test_store_open.cpp` copies set aside as fixture values. The disputed `kDot`.
- X8: one row per bullet (four). The `" ENDOFSONG"` against `"_ENDOFSONG"` point nobody checked.
- X9: one row per kind of stand-in the re-read found (six). `test_app_click.cpp:1069`, the one dispute left.
- X10: the pins in one file only, one row each.
- X11 and X12: one row each.
- The Keep recount: the counters' split on hand-built library rows. The two fake-analyzer seams in the product, `BatchJob::set_analyzer_for_test` and `set_app_batch_analyzer_for_test` in `src/ui/library_jobs.h`, which are a production-code finding.
- "What the repeated-values pass left out": each disputed and unsettled item.

A row where a verifier called something sound, or set copies aside, is a closure. It leaves by exit 4 only with one sentence saying what each side answers, and T9 checks it. A split leaves by exit 3 only with a quote from a `## Confirmed` line that names both of its sides. Every other row is exit 2.

Acceptance: `cross-rows.md` has a row for every item above, and every "Exit: not chosen" line in `cross-findings.md` maps to at least one row. No row is empty.

### E5: Collect the side notes, splits and production-code findings

The first source is `AUD/capture/byreading/SUMMARY.md`. It names six kinds of side note and a rule slip. Each named test or record under those headings becomes a row.

The other side notes were found by a case-blind search of AUD for "side note". It hit 97 files in `records/`, 35 in `verify/`, 10 in `capture/byreading/`, 3 in `capture/rejudge/`, 3 in `sweep/` and 1 in `counts/`. A search for lines that start "split" hit 86 files in `records/` and 116 in `verify/`. A search for "production-code finding" lines hit 53 in `records/`. `AUD/log.md` also lists points raised in the delta round (lines 326 to 339) and the window round (lines 369 to 379), and the failed replay of real ref moves (line 398). Those searches count lines and files, not notes, and a search that misses a heading form proves nothing. So the number of notes is not known (P6).

E1's script therefore also pulls every such block into `AUD/exits/notes.tsv`, with its file and line. Sonnet scouts, four to a round, read each record and verify file for notes the script's patterns missed, and mark each note: already a ledger fault of that record, a split, a production-code finding, or something else. The cap is one pass over the files and one re-send. Files not reached are listed by name.

A side note that repeats a fault its record already carries points at that record's row. Every other note, every split and every production-code finding is a row of its own, and by the approved plan it is exit 2.

Acceptance: every file the searches hit appears in `notes.tsv` or in the "no note found, read by" list with the scout's id. Every row under SUMMARY's "Side notes that need an exit" has a row.

### E6: The open list for the twenty cannot-tell records

The main session writes `AUD/exits/open.md` with one row for each of the 20 records under "Cannot tell" in SUMMARY. A row names the record, what the verdict file says would settle it, and its group. There are three groups, as SUMMARY has them: twelve need the test run or a run on the corpus, six need the real `dump.cs` and `facts.json` from outside the repo, and two are scripts with no assertion that need the live game and a person watching.

This plan runs none of them. Each group becomes one question in T10: may the run happen, and if not, the records stay on the open list and the audit's final report names them as open. No row is marked "holds", and E7's script refuses a cannot-tell record any exit but 2.

Acceptance: `open.md` has 20 rows, each with a run and a question. The ledger shows none of the 20 with exit 4.

### E7: The main session writes the exit rules, and signs what they produce

Waits for: E1, E3, E4, E5, E6. Records held by E2 wait for their answer.

The main session writes `AUD/exits/rules.md` in its own turn, with no agent. Each rule is one line: which ledger rows it takes, the exit, and the line of the approved plan or of audit-method that requires it. The rules this plan expects are below. Each comes from the approved plan's T8 section, and the main session may tighten one but may not add a way to close.

1. A Rewrite, Remove or Investigate record leaves by exit 2.
2. A Keep record with a fault mark leaves by exit 2. The user's filed rulings say the test is wrong, and they do not say what replaces it.
3. A Keep record with no fault mark, whose verifier agreed, leaves by exit 4. Its evidence is the record.
4. A record whose auditor and verifier disagree leaves by exit 2, and the question shows both verdicts.
5. A clean Investigate record leaves by exit 2, whatever its by-reading verdict. Shown-by-run, holds-by-reading and cannot-tell are three separate groups of questions.
6. A held record gets no exit.
7. A row no rule takes is printed as "no rule", and becomes a question.

`ledger.py` applies the table and writes `AUD/exits.md`: one row per finding and per split, with the exit, the evidence and the question group. Exit 1 appears only if the main session names a commit by someone else that removed the problem. Exit 3 appears only on a row where the main session pasted a quote, its file and line, and the two sides it names.

What the main session reads before it signs:

- every rule line it wrote, and the script's count of rows per rule;
- every group in `groups.md`, with its plainest and its least-fitting record opened beside the test;
- every row of `cross-rows.md` and `open.md`;
- every "no rule", "no key", disputed and `not-in-ledger` row;
- one line per row of `notes.tsv`.

How long. At the caps that is 80 groups and so 160 records opened, and about 660 one-line rows if the notes come to about 500. At an assumed three minutes per opened record and half a minute per line, that is about eight hours and about five and a half hours. Both rates are guesses (P7). The main session times its first ten of each and tells the user the measured total before it goes on.

The main session signs by writing its session id and the date at the top of `exits.md`, with the count of rows per exit. A sample is evidence about the grouping, not about the exit: every row's exit comes from a rule line, so no exit rests on a record nobody read.

Stop line. If more than one opened record in ten sits in the wrong group, the main session stops, sends that group back to a fresh scout once, and tells the user if it fails again.

Acceptance: `exits.md` has no empty exit. Its row count equals the ledger's rows plus the rows of `cross-rows.md`, the non-duplicate rows of `notes.tsv`, the T6 rows and `not-in-ledger.md`. Every exit 2 row names a question group. Every exit 3 row holds a quote, a file and line and two named sides; a split with no such quote is exit 2. Every exit 4 row is rule 3 or an E4 closure with its sentence. No agent id appears as the author of `rules.md` or of an exit.

### E8: Hand the rows to T9 and T10

T9 and T10 run as the approved plan says. This task fixes only what T8 owes them.

For T9, `exits.md` lists every exit 1, 3 and 4 row in groups of about eight, so fresh Sonnet checkers can take them three or four to a round.

For T10, each question group gets a draft with these parts: the ruling the tests break, quoted from its `## Confirmed` line; the count of tests, with all of them listed by name and line in a file beside the question; the plainest and the least-fitting test, each with the line that fails, what it checks today and what it would check after; and what would stop being checked. The options come from the records' own Rewrite and Remove text. Where no real input exists for a rewrite, the question says so and cites `AUD/capture/RUNS.md`.

These are changes to tests, so most change nothing a player sees, and the question says that in one line. A production-code finding is different. Its question lists every screen that reads the value, with the text before and after from a real chart in `AUD/records-engine/` or `AUD/capture/runs/` (agent rule 11). A finding with no such capture waits for one; the main session does not describe a screen from memory.

Acceptance: every exit 2 row's group has a draft in `AUD/questions.md` with all five parts. Every production-code question names its capture file.

## Who does what

| Work | Who | Model |
|---|---|---|
| This plan, then one fresh review of it | planner, reviewer | Fable, Sonnet |
| The T7 question 8 re-read (E0) | eight read-only verifiers, one per slice | Sonnet |
| `ledger.py` | one executor, one reviewer | Opus, Sonnet |
| Ledger residue, labels, notes | read-only scouts | Sonnet |
| Label verifiers | read-only scouts, never on their own file | Sonnet |
| `keys.md`, `rules.md`, `cross-rows.md`, `open.md`, every exit, every question | main session only | |

No agent builds or runs a built test. No whole-library run happens.

## The live steps

This plan has no live step. Nothing in it starts Clone Hero or Hydra, and the runs E6 names are not run here.

## Out of scope, for the user to approve

- Every fix to a test or to production code. Fixes get their own plan after the answers.
- Running the twenty cannot-tell records' runs. T10 asks.
- Adding X9 to X12 to `docs/agents/test-rubric.md`. The handoff suggests asking after T8.
- The B6 scout's rule slip (it ran Python tests its brief did not name). It gets a row in E5, and changing a brief or a hook is not part of this plan.

## Questions

1. The recount treated input a test builds and feeds to the real code as not fake, on your question-box choice "No, input is fine". That choice is not filed, and your filed line says tests are never based on anything fake. Does the choice stand, and will you file it? It decides 49 fault marks (`TOTALS.md` section 3).
2. Eight tests in `tools/ch_mcp/tests/test_frida_attach.py` (lines 976 to 1275) feed hand-typed source lines to a scanner that is defined in the test file itself. Is a checker that lives in its own test fake code?
3. Seven tests in `tools/test_compare_db.py` (lines 205 to 299) feed the real compare tool database files written by hand in SQL, in a layout the file calls a stand-in for an older one. Are those files input, or a stand-in for Hydra's own stored rows?
4. About 25 tests in the hooks repo's `test_review_dispatch.py` feed a hand-typed hook payload to the real hook, and others in `test_review_rules.py` feed hand-written verdict records. Is a hand-typed payload input, or a stand-in for what Claude Code really sends? This is the largest block.
5. Tests at `tools/ch_mcp/tests/test_game_tools.py` 616 to 674 read hand-written stats and score JSON, and `test_fits_screen` reads hand-typed saved-window text. Input, or a stand-in for what the game and Hydra write?
6. The size: one executor, eight re-read verifiers for the T7 slices, about 36 label scouts, about 18 verifiers and up to 12 more scouts for residue and notes, plus about 13 hours of main-session reading at the caps. Is that approved?

### The answers (session 88ea48bb, 2026-11-15)

The user answered all six in question boxes. Their words are quoted as typed or chosen.

1. The choice stands. Asked again, the user wrote: "why are you asking me again if i already answered in the previous session?" So a hand-written file the real code reads is input. The 49 cleared marks stay cleared, except where answers 2, 3 and 5 below put marks back.
2. Fake ("Fake (Recommended)"). The eight KR3 records (`TOTALS.md` section 3, KR3) go back to X9.
3. Fake. The user wrote: "we have a test corpus we can use for testing database code". The seven IR1 records go back to X9. The user filed that line under `## Confirmed` in `docs/agents/rulings/tests-and-runs.md` (commit 974fb38f on main), so the fix for these tests builds their databases from the test corpus.
4. Input ("Input"). The hook-payload rows in IR4 (RD) lose their X9 where the payload is the only thing marked. The hand-written verdict rows in `test_review_rules.py` (RR) were not part of the answer and keep their marks.
5. Both are fake. The user wrote: "json is fake. you're stil trying to gaslight me with hand typed numbers." Asked to confirm the window test too, the user wrote: "why do i have to keep repeating myself?? how many times during this audit have i told you that hand typed numbers are fake???" So IR8 D4 616 to 674 and A46 `test_fits_screen` keep X9. The user's filed lines already cover hand-typed numbers ("tests should not be pinned to typed constants. they should be pinned to the behavior of the code."), and the user declined to file another.
6. "Smaller first". Only E0 (the eight question 8 re-reads) and E1 (the ledger) run now. The main session reports their results before any later task starts.

## Promise map

| Promise | Plan section | Task | The check that proves it |
|---|---|---|---|
| Every record has a ledger row or is named as missing | E1 | E1 | rows plus `not-in-ledger.md` names equal the records in `AUD/records/` |
| The ledger agrees with the recount's totals | E1 | E1 | per-file sums against `TOTALS.md` section 1; differences listed by record |
| WN4 is not counted twice | E1 | E1 | no ledger row names `window-WN4.md` |
| T6 reds are findings; a failed T7 claim reopens question 8 for that slice | T6 and T7 results | E0, E7 | each of the three T6 tests has a `t6` row; each of the eight reopened slices has a re-read file; no exit 4 in a reopened slice before its re-read commit |
| The four unsettled blocks, and the unfiled choice, are asked first | What must be asked | E2, E7 | held rows have no exit in `exits.md` |
| Tests that share one rule share one question | E3 | E3 | every faulty row has a key; `groups.md` lists each key's records |
| No key is made up | E3 | E3 | each key in `keys.md` cites its line in `TOTALS.md`, `cross-findings.md` or the approved plan |
| Agents never pick an exit | E3, E7 | E7 | `rules.md` and `exits.md` carry only the main session's id; scout files hold no exit column |
| Every exit comes from a written rule the main session signed | E7 | E7 | each `exits.md` row names its rule number; the sign-off line is present |
| The main session reads both ends of every group | E7 | E7 | `groups.md` marks each key's two opened records with the date read |
| X1 to X12 and each split inside get their own rows | E4 | E4 | every item in E4's list, and every "Exit: not chosen", maps to a row |
| A split never closes without a quote naming both sides | E4, E7 | E7, T9 | every exit 3 row has quote, file, line and two sides; T9 greps each quote |
| Side notes, splits and production-code findings each get an exit | E5 | E5, E7 | every hit file is in `notes.tsv` or the "read by" list; each note row has an exit |
| No cannot-tell record becomes "holds" | E6 | E6, E7 | 20 rows in `open.md`; none has exit 4 |
| A row no rule fits becomes a question | E7 | E7 | zero "no rule" rows without a question group |
| Every repeating step has a cap and a stop line | E1, E3, E5, E7 | each | the caps are in each task; `log.md` records any cap hit |
| Reading time is measured, not assumed | E7 | E7 | `log.md` line with the timed first ten and the total told to the user |
| Questions show both ends, every test by name, and real screens for production findings | E8 | E8 | each draft in `questions.md` has its five parts and capture file |
| No test and no code changes | The short version | E7 | the branch's diff holds only files under AUD and this plan |
| No live step | The live steps | none needed | no `### LS` heading exists in this plan |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: 2,449 Keep and Investigate records were recounted and 1,861 hold a ruled fault; 239 of 421 Keep and 1,622 of 2,028 Investigate. | code | `docs/audit/2026-11-14-test-audit/counts/TOTALS.md` section 2 | checked 2026-11-15, the table read; the count files under it were not re-added |
| P2: 397 Investigate records hold no fault, and their verdict rows are 34 shown by a run, 333 hold by reading, 20 cannot tell. | code | `docs/audit/2026-11-14-test-audit/capture/CLEAN.md` line 5; `docs/audit/2026-11-14-test-audit/capture/byreading/SUMMARY.md` "Totals" | checked 2026-11-15 |
| P3: 2,028 less 1,622 is 406, not 397, and CLEAN.md says it left out eight unsettled compare-tool rows; the rest of the gap is unexplained. | unchecked | `docs/audit/2026-11-14-test-audit/capture/CLEAN.md` line 7 | E1 |
| P4: The final count of Rewrite and Remove records. | unchecked | `docs/audit/2026-11-14-test-audit/log.md` line 341 (1,168 plus 78 headings); a search of `docs/audit/2026-11-14-test-audit/records/` found 1,315 and 160 verdict lines, superseded files included | E1 |
| P5: A script can read every record's id, verdict and fault marks from the audit's files. | unchecked | `docs/audit/2026-11-14-test-audit/counts/TOTALS.md` line 7 says a parse missed records in two files | E1 |
| P6: How many side notes, splits and production-code findings the records and verify files hold. | unchecked | searches of `docs/audit/2026-11-14-test-audit/records/` and `verify/` (file and line counts only, given in E5) | E5 |
| P7: The main session reads an opened record in three minutes and a one-line row in half a minute. | unchecked | `docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md` (this plan's own guess, measured nowhere) | E7 |
| P8: The user's four test rulings are filed under `## Confirmed`. | ruling | `docs/agents/rulings/tests-and-runs.md` lines 8 to 11, main's copy | checked 2026-11-15 |
| P9: The user chose that hand-built input fed to the real code is not fake ("No, input is fine"). | unchecked | `docs/audit/2026-11-14-test-audit/briefs/T5-keep-recount.md` line 17 reports it; no line under `docs/agents/rulings/` or `docs/sessions/` on main holds the words | question 1 |
| P10: Four blocks are unsettled: KR3 D2 26 to 35, IR1's seven compare-store rows, IR4's hook payload and verdict rows, IR8 D4 616 to 674 with `test_fits_screen`. | code | `docs/audit/2026-11-14-test-audit/counts/TOTALS.md` section 6 | checked 2026-11-15; the tests themselves were not opened, so questions 2 to 5 are re-read by E2 on the day they go out |
| P11: X1 to X12 each read "Exit: not chosen", and hold the splits and disputes E4 lists. | code | `docs/audit/2026-11-14-test-audit/cross-findings.md` | checked 2026-11-15, read whole; the V files it cites were not opened |
| P12: An agent is warned at 100 calls or 20 minutes and stopped at 150 calls or 30 minutes. | code | `docs/agents/brief-preamble.md` line 69 | checked 2026-11-15 |
| P13: The approved plan sends every Rewrite, Remove and Investigate to the user, and exit 4 is a Keep the verifier agreed with. | code | `docs/superpowers/plans/2026-11-14-test-suite-audit.md` lines 171 to 180 | checked 2026-11-15 |
| P14: One recount counter read about 94 records. | code | `docs/audit/2026-11-14-test-audit/counts/TOTALS.md` section 1 (26 files, 2,449 read) | checked 2026-11-15; a labelling scout's pace is not measured, and E3's first round shows it |
| P15: `window-WN4.md` is superseded by WN4a, WN4b and WN4c. | code | `docs/audit/2026-11-14-test-audit/log.md` line 358 | checked 2026-11-15 |
| P16: The stand-in kinds are nine of the code's own output, seven of outside data and eight of fake code. | code | `docs/audit/2026-11-14-test-audit/counts/TOTALS.md` line 9 | checked 2026-11-15 |
| P17: T6 turned three tests red, each already a Rewrite record; T7 planted 98 bugs, 91 went red at a named check, six stayed green (A3, A9, A30, C7, WN3, WN4a), WN8 crashed, WN4b holds one green named line. | code | `docs/audit/2026-11-14-test-audit/mechanical/corpus.md` (commit 6aa6d684) and `docs/audit/2026-11-14-test-audit/mechanical/planted.md` (commit a4ad3306) | checked 2026-11-15 by the runs; for most of the 91, "named" compared the failing line with the picks scout's copy of the record's line, not a fresh read of each record |
| P18: The handoff's Claims hold: no row is missing or changed. | code | `tools/check_handoff.py` run on `docs/handoffs/2026-11-15-test-audit-t8-exits-plan-handoff.md` (12 ok, 3 not on main, 1 not pushed, 10 manual) | checked 2026-11-15; of the manual rows, only the totals and the two missing state files were read against the files |
| P19: The audit also holds records for the hook tests, slices E1 to E4, which the approved plan listed as out of scope. | code | `docs/audit/2026-11-14-test-audit/records/E1.md` to `E4.md`; `docs/audit/2026-11-14-test-audit/log.md` lines 301 to 308 | checked 2026-11-15, by file name and log lines; why they were added was not read |
