# Test audit questions

This file holds every question the test audit puts to the user, with the user's words for each answer. T8's step E2 opened it with the five fake-data questions, which were answered before T8 began. Step E8 adds a draft per question group for T10.

A question-box choice quoted here is the user's answer to that question only. It is a ruling only where its line sits under `## Confirmed` in `docs/agents/rulings/`.

## Questions 1 to 5: what counts as fake (answered 2026-11-15)

These were questions 1 to 5 of the T8 plan (`docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md`, "Questions"). They came from the four blocks the recount's tallier left unsettled (`counts/TOTALS.md`, section 6) and the recount's own reading of "made-up input" (`TOTALS.md` line 9). The user answered them in session 88ea48bb. The quotes below are from that session's card, `docs/sessions/2026-11-15-88ea48bb.md`, lines 22 to 27.

The filed line both sides were set against is "tests should never be based on anything fake. testing fake made up data is useless and says nothing about how the app behaves in a real situation" (`docs/agents/rulings/tests-and-runs.md` line 10, session 63c70014).

### Question 1. Is a hand-built input that the real code reads fake?

The recount cleared 49 X9 marks on the reading that a hand-written chart, ini, settings, SQL or source text that the real code reads is input, not fake (`TOTALS.md` line 9 and section 3). That reading came from a question-box choice, "No, input is fine", that was never filed.

The user's words: "why are you asking me again if i already answered in the previous session?" (card line 22).

What it changes: the choice stands. The 49 cleared marks stay cleared, except where answers 2, 3 and 5 put marks back. No ledger row changes from this answer alone.

### Question 2. Is a checker defined in its own test file fake code?

Eight tests in `tools/ch_mcp/tests/test_frida_attach.py` (lines 976 to 1275) feed hand-typed source lines to a scanner the test file defines itself. These are the KR3 rows of `TOTALS.md` section 3.

The user chose "Fake (Recommended)" (card line 24).

What it changes: the eight KR3 removals are undone and X9 stays. In `exits/ledger.tsv` these rows carry `answer 2` and the correction "TOTALS s3 removal undone by answer 2".

### Question 3. Are hand-written SQL stores input, or a stand-in for Hydra's stored rows?

Seven tests in `tools/test_compare_db.py` (lines 205 to 299) feed the real compare tool databases written by hand in SQL. These are the IR1 rows of `TOTALS.md` section 3.

The user's words: "we have a test corpus we can use for testing database code" (card line 23). The user filed that line under `## Confirmed` in `docs/agents/rulings/tests-and-runs.md` (line 9).

What it changes: the seven IR1 removals are undone and X9 stays. In the ledger these rows carry `answer 3`. The fix for these tests builds their databases from the test corpus.

### Question 4. Is a hand-typed hook payload input, or a stand-in for what Claude Code sends?

About 25 tests in the hooks repo's `test_review_dispatch.py` feed a hand-typed hook payload to the real hook (the RD rows of `counts/T5-IR4.md`). The question as the card keeps it began "About 25 hook tests hand-type the message Claude Code sends a hook ...".

The user chose "Input" (card line 25).

What it changes. The hand-written verdict rows in `test_review_rules.py` (RR) were not part of the answer, so they keep their marks. The RD rows lose X9 where the payload is the only thing X9 marks. E1's script could not tell that from the counter's reason text, so it left 33 RD rows for the main session to settle.

The main session read all 33 tests on 2026-11-16 (`C:\Users\Patrick\.claude\gates\tests\test_review_dispatch.py`, lines 68 to 351). Each one builds its payload in `ReviewDispatch.dispatch` (line 69) or `Waiver.prompt` (line 290), or by hand in the one crash test (line 201). It passes the payload to the real `review_dispatch.body` or `waiver.body`. The repos it judges are real git repos the test builds and commits into, which the counter's own key (`T5-IR4.md` line 9) does not mark. So in each of the 33, the payload is the only thing X9 marks, and X9 goes. Their X10 and X11 marks are separate faults and stay. 13 of the 33 held X9 alone and now hold no fault; 20 keep X10, X11 or both. The settlement is `exits/answer4.tsv`, one row per test.

Payload rows the answer does not cover. A search of the ledger's count rows for "payload" found seven more, outside `test_review_dispatch.py`. The question named that file's hook messages only, so its answer does not reach these, and each keeps its X9:

| Count row | Test | Marks | The counter's reason |
|---|---|---|---|
| `counts/T5-IR4.md` line 25 | RF:52 crash deny logged | X9, X11 | hand payload `call()`, stand-in hook that raises |
| `counts/T5-IR12.md` line 16 | `test_method_watch.py` :214 | X9, X10, X11 | hand-written error payload |
| `counts/T5-IR12.md` line 19 | `test_method_watch.py` :278 | X9, X10 | three hand-written call payloads |
| `counts/T5-IR12.md` line 21 | `test_method_watch.py` :305 | X9, X10 | hand-written payloads |
| `counts/T5-IR5.md` line 70 | record "no" | X9 | hand-built agent payload |
| `counts/T5-KR1.md` line 24 | MW :324 | X9, X10 | three hand-written agent payloads |
| `counts/T5-KR2.md` line 58 | record 1 | X9, X10, X11 | hand-typed `call()` payload |

`counts/T5-IR1.md` line 69 also says "payload", but it means bytes in a hand-built store, which answer 3 covers. Whether any of the seven is a hook payload in the sense of question 4 is not settled here. Each goes to T10 with its group, and E7 gives it exit 2 like every X9 row.

### Question 5. Are hand-written game JSON and saved-window text input?

Tests at `tools/ch_mcp/tests/test_game_tools.py` lines 616 to 674 read hand-written stats and score JSON, and `test_fits_screen` reads hand-typed saved-window text (IR8 in `TOTALS.md` section 6).

The user's words: "json is fake. you're stil trying to gaslight me with hand typed numbers." Asked to confirm the window test too: "how many times during this audit have i told you that hand typed numbers are fake???" (card lines 26 and 27).

What it changes: both keep X9. In the ledger these rows carry `answer 5 (keeps X9)`. The user's filed line already covers hand-typed numbers: "tests should not be pinned to typed constants. they should be pinned to the behavior of the code." (`tests-and-runs.md` line 11).

### What is held

Nothing. All five questions are answered, so the ledger's `held` column is empty on every row once the answer-4 settlement is read. A row would be held only if a question that covers it had no answer.

## Question 7: which count is "caught" (answered 2026-11-16)

The T8 plan's question 7 asked which of two counts of a test's caught bugs a later question may call "caught". The user chose "Count 1; fix F1 (Recommended)" in session 899dc0dd. So "caught" is `tests.txt`'s count, the ledger's `mut_caught`, and `mut_caught_shared` stays beside it. The plan's question 7 holds the full question and both options.
