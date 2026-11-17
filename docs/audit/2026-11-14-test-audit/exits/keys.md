# The key list: what a faulty test fakes, pins, echoes or cites

T8 step E3 (`docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md`). Written by the main session, session f3bbb32b, 2026-11-16.

Records that break the same ruling in the same way share one question for the user. A key names that pair: the ruling the test breaks, and what the test fakes, pins, echoes or cites. Two tests that both feed a fake analyzer share a key. A test that feeds a fake analyzer and a test that pins a tier name do not.

Every key below comes from something the audit already wrote. Its source is named on its row. No key is made up here. Scouts label each faulty ledger row with one or more keys, or with "no key" and a reason. The list is revised once, after the first round of scouts, from their "no key" rows. After that, a "no key" record is a question of its own.

A key is a label, not an exit. Only the main session picks an exit, at E7.

## The rulings the keys name

These are the user's words under `## Confirmed` in `docs/agents/rulings/tests-and-runs.md`, each filed in session 63c70014 or 88ea48bb.

| Id | The user's words | Line |
|---|---|---|
| R-fake | "tests should never be based on anything fake. testing fake made up data is useless and says nothing about how the app behaves in a real situation" | 10 |
| R-const | "tests should not be pinned to typed constants. they should be pinned to the behavior of the code." | 11 |
| R-cite | "tests exist to verify the behavior of the code. a test citing something that isn't code is always wrong" | 12 |
| R-string | "tests should test their intended behavior directly, not make up an arbitrary meaningless string" | 13 |
| R-db | "we have a test corpus we can use for testing database code" | 9 |

Some keys break no filed ruling. They break a rubric question in `docs/agents/test-rubric.md`, and their row says "rubric" and the question's number. A rubric question is not the user's ruling, so a group under such a key is asked as its own question, never closed by a quote.

## X9: a made-up stand-in (R-fake)

The kinds are the tallier's, from `counts/TOTALS.md` line 9, plus three the user's answers added (`questions.md`, questions 2, 3 and 5). A stand-in is something the test types or builds by hand in place of what the real code or the real world would give it. A hand-written chart, ini, settings file, doc, note, rule, source text or fuzz bytes that the real code reads is input, not a stand-in (question 1). A hand-typed hook payload in `test_review_dispatch.py` is input too (question 4).

### Stand-ins for the code's own output

| Key | What the test hand-builds | Source |
|---|---|---|
| F-result | results: activations, squeezes, paths, scores, best-path text such as "3- 1 2  (optimal)" | TOTALS line 9; cross-findings X9 |
| F-row | rows: Library summary rows (`ready(...)`, `analyzed(...)`), library entries (`fake_charts`, `library_entry`, `timed_library`), report rows | TOTALS line 9; cross-findings X9 and "The Keep recount" |
| F-record | records written to the store and read back | TOTALS line 9; cross-findings X9 |
| F-summary | summaries | TOTALS line 9 |
| F-schedule | schedules and replay JSON fed to the replay tool's readers | TOTALS line 9; cross-findings X9 |
| F-sample | recorder samples | TOTALS line 9 |
| F-presslog | press logs | TOTALS line 9 |
| F-watchlog | watcher logs | TOTALS line 9 |
| F-verdict | review verdicts, waiver records and real-app records (`clean()`, `record()`, `waive()`) | TOTALS line 9; `counts/T5-IR4.md` line 9 |
| F-db | a database written by hand in SQL in place of one Hydra stored; R-db names the fix | question 3; `counts/TOTALS.md` section 6 item 2 |
| F-window | hand-typed saved-window text in place of what Hydra writes | question 5 |

### Stand-ins for outside data

| Key | What the test hand-builds | Source |
|---|---|---|
| O-reply | a service's reply: dmleaderboards (`canned_dm_users()`, `kDmUsersJson`), GitHub | TOTALS line 9; cross-findings X5 and X9 |
| O-transcript | a transcript (`_typed`, `_answer`, `human()`) | TOTALS line 9; `T5-IR4.md` line 9 |
| O-gamemem | game memory, the game's method calls or messages | TOTALS line 9 |
| O-process | a process list | TOTALS line 9 |
| O-payload | a hook or agent payload outside `test_review_dispatch.py`, which question 4 did not cover | TOTALS line 9; `questions.md`, question 4 |
| O-scorestats | the game's own scorestats, stats or score JSON | TOTALS line 9; question 5 |
| O-ocr | OCR text | TOTALS line 9 |

### Fake code

| Key | What the test puts in place of real code | Source |
|---|---|---|
| C-analyzer | a fake analyzer, including one that returns a real corpus result (`gated_scores`, `set_analyzer_for_test`) | TOTALS line 9; cross-findings "The Keep recount" |
| C-reader | a fake reader | TOTALS line 9 |
| C-worker | a fake worker | TOTALS line 9 |
| C-rig | a rig | TOTALS line 9 |
| C-clock | a fake clock | TOTALS line 9 |
| C-session | a fake session | TOTALS line 9 |
| C-stub | a stub program | TOTALS line 9 |
| C-gate | a stub gate module or a fake rules file (`rules("allow")`) | TOTALS line 9; `T5-IR4.md` line 9 |
| C-ownchecker | a checker the test file defines itself, then tests | question 2 |

## X10 and X1 to X8: a typed constant (R-const)

A test types the code's answer, or a constant the code owns, instead of checking what the code does. The owners are named in `cross-findings.md`.

| Key | What is typed | Source |
|---|---|---|
| P-burnout | Burnout's best path, score and other figures from one run | cross-findings X1 |
| P-pbticks | the Path Builder's fill ticks and their clock text | cross-findings X2 |
| P-allister | Allister's offered fills | cross-findings X3 |
| P-tiers | the squeeze timing tiers, their names and edges | cross-findings X4 |
| P-message | a message the code under test prints: Hydra's own in `src/`, or a tool's or hook's (widened in revision 1, below) | cross-findings X5 and X10 |
| P-placeholder | the unknown title, artist or charter text | cross-findings X6 |
| P-setting | a settings default or a screen word | cross-findings X7 |
| P-name | a name or key the product writes | cross-findings X8 |
| P-onefile | a constant pinned in one test file only (`kReportConfirmAbove` 500, the star cutoffs, the column widths and the rest) | cross-findings "Pins in one file only" |
| P-builtchart | the engine's answer on a chart the test built, typed in (12960, Allies' best-path ticks) | cross-findings "Engine answers on charts the tests built" |
| P-other | any other typed constant the code owns, not one of the above | cross-findings X10 ("records that keep a constant pin may exist in every slice") |

## X11, X12 and the snapshot

| Key | What the test does | Ruling | Source |
|---|---|---|---|
| E-echo | types a value in and requires the same value back | rubric question 8; no filed ruling | cross-findings X11 |
| Z-cite | names its expected value's source in a comment, not in code ("From one run", "The engine's record", "as PS0b saved it") | R-cite | cross-findings X12 |
| S-snapshot | expects a number that is what the code printed once, such as a total over `testdata/input` | R-const; rubric "Two kinds of number" | rubric; approved plan "What the audit looks for" |

## Rewrite and Remove records: the rubric's kinds

A Rewrite or Remove record was judged by the rubric, before the user's test rulings were filed. It takes one of these kinds, and any of the keys above that its text also shows. The kinds are the approved plan's table "What the audit looks for" (`docs/superpowers/plans/2026-11-14-test-suite-audit.md` line 33). The snapshot kind is S-snapshot above.

| Key | What the record says is wrong | Rubric question |
|---|---|---|
| K-invented | the claim is a rule a display invented | 2, 3, 6 |
| K-cantfail | a check that cannot fail | 8 |
| K-floor | a floor with no reason | 5, 9 |
| K-skip | a silent skip | 9 |
| K-selfmade | a value checked against the function that made it, or made by production code | 5, 9 |
| K-comment | a raw result whose meaning lives in a comment | 4 |
| K-unreachable | a scenario no real chart can reach | 11 |

## Revision 1, after the first round (2026-11-16)

The first round labelled four batches (L-C01, L-C14, L-R01 and L-R12), 323 rows in all. Six rows got `no key`. Each named a fault the list lacked, so the keys below were added from them. This is the one revision the plan allows. After it, a `no key` row is a question of its own.

| Key | What the test does | Ruling | Where it came from |
|---|---|---|---|
| O-toolout | feeds hand-typed output of an outside program, such as `git diff` lines, in place of running it (a stand-in for outside data) | R-fake | `counts/T5-IR10.md` line 59, labelled in L-C01; TOTALS line 9's class "outside data" |
| K-duplicate | repeats another test's check, so the record says Remove | rubric; no filed ruling | `records/D1.md:606` and `records/D10.md:470`, labelled in L-R12 |
| K-namebody | its name promises a check its body never makes | rubric question 1 | `records/D10.md:385`, labelled in L-R12; `capture/byreading/SUMMARY.md`, "A title that promises what the code does not do" |
| K-partial | claims "every" case and checks a hand-picked list of them | rubric question 1 | `records/A10.md:617`, labelled in L-R01 |
| K-nocode | checks no Hydra behaviour: only helpers in the test file, a language's or Windows' own behaviour, or a function's signature or source text | rubric question 2 | `records/D11.md:445`, labelled in L-R12; SUMMARY, "Tests that check no Hydra code" |

One key's meaning is widened. All four scouts used P-message for the text a tool or hook prints, such as a refusal or a usage line, as well as for Hydra's own messages in `src/`. The question is the same for both: the test types the message instead of checking when it is shown and what it names (cross-findings X10, "For a message, the behaviour is when it is shown and what it names"). So P-message now covers a message any code under test prints. The row names the owner, so the question can show both kinds.

The six rows take these keys: `counts/T5-IR10.md:59` O-toolout; `records/D1.md:606` and `records/D10.md:470` K-duplicate; `records/D10.md:385` K-namebody; `records/A10.md:617` K-partial; `records/D11.md:445` K-nocode. The count is now 53 keys.

## How a scout uses this list

A row gets every key that fits. A row with two faults gets two keys and sits in two questions. For each key, a scout marks the plainest row in its batch and the one that fits least well, so the question can show both ends. A row no key fits gets "no key" with the reason in one sentence.

The count is 48 keys: 11 F, 7 O, 9 C, 11 P, the three above, and 7 K. The plan's stop line is 80.
