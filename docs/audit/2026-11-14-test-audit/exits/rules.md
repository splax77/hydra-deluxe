# The exit rules

T8 step E7 (`docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md`). Written by the main session, session f3bbb32b-5f4d-4400-8cda-86f04d67f2a4, 2026-11-18, in its own turn, with no agent.

Every finding leaves the audit by exactly one of four exits (`docs/agents/audit-method.md`, "The four exits"): 1 fixed, 2 a question for the user, 3 covered by a ruling quoted in the user's words, 4 not a problem. The rules below give every row its exit. `ledger.py` applies them in table order, and the first rule that takes a row decides it. A row no rule takes is printed as `no rule` (rule R7).

## What the rules may not do

- **No new way to close.** The plan lists seven rules (plan lines 275 to 281). Every rule below is one of them, or a tightening that sends more rows to exit 2. None adds a way to reach exit 1, 3 or 4.
- **Run evidence closes nothing.** No rule names a `mut_` column (plan line 283). `ledger.py` refuses any rule line that names a `mut_` column and gives exit 1, 3 or 4.
- **Exit 1** needs a commit by someone else that removed the problem, named here by the main session. None is named, so no row takes exit 1.
- **Exit 3** needs a quote of the user's words, its file and line, and the two sides it names, pasted here by the main session (plan line 285). None is pasted yet, so no row takes exit 3. A split with no such quote is exit 2.
- **Cannot-tell records** take exit 2 and nothing else (plan line 265).

## Where the groups come from

Every exit 2 row names a question group. Rows in one group share one question.

- **Keys** (`labels/labels.tsv`, from E3): a Rewrite or Remove record, and a count row with a fault mark, take every key on their row. A row whose label says `no key` takes the group `NK:<row>`, a question of its own (`keys.md`, "Revision 1").
- **A record's count rows:** a Keep or Investigate record with a faulty count row joined to it (`joins_record`) takes the keys of those count rows.
- **By-reading verdicts** (`byreading_verdict`): `BR-shown`, `BR-holds` and `BR-cannot`, three separate groups (plan line 279).
- **Investigate groups** (`investigate-groups.tsv`): the ten `I-` groups of the 1,796 clean, unjudged Investigate records.
- **Note marks:** `N-split`, `N-prod` and `N-finding`.
- **Cross rows** (`cross-rows.md`): the X family named on the row, such as `X1`.

## The rules for ledger rows

"Faulty count row" means a count row whose fault mark, after answer 4 (`answer4.tsv`), is not `none`. "Clean" means a record with no faulty count row joined to it.

| Rule | Takes | Exit | Group | Required by |
|---|---|---|---|---|
| R6 | any row whose `held` column is not empty | none | none | plan line 280 |
| R4 | a record whose `verifier_verdict_if_different` is not empty | 2 | `D-disagree`; the question shows both verdicts | plan line 278 |
| R1a | a Rewrite or Remove record | 2 | its keys, or `NK:<row>` | plan line 275 |
| R1b | an Investigate record with a faulty count row | 2 | its count rows' keys | plan line 275 |
| R5 | a clean Investigate record with a by-reading verdict | 2 | `BR-shown`, `BR-holds` or `BR-cannot` | plan line 279 |
| R1p | a clean Investigate record with no by-reading verdict whose group in `investigate-groups.tsv` is `I-pointer` | 2 | the group its target record gets (the record in the same file whose title holds the line the verdict names); `NK:<row>` if no target is found | tightens plan line 275: a "same as" verdict is its target's question |
| R1c | a clean Investigate record with no by-reading verdict | 2 | its group in `investigate-groups.tsv` | plan line 275 |
| R2 | a Keep record with a faulty count row | 2 | its count rows' keys | plan line 276 |
| R3t | a Keep record with `t7_reopened` = `yes` | 2 | `K-reopened` | tightens plan line 277: a record reopened at T7 is not settled |
| R3 | a clean Keep record whose verifier agreed (`verifier_verdict_if_different` empty) | 4 | none; the evidence is the record, named by file and line | plan line 277 |
| C1 | a faulty count row | 2 | its keys, or `NK:<row>` | plan line 276; a fault mark is a ruled fault |
| C2 | a count row that is not faulty and is joined to a record | its record's exit | its record's group | the count row and its record are one test; it reaches exit 4 only through R3 |
| R7 | any row no rule above takes, including a count row that is not faulty and joins no record | `no rule` | none; a question | plan line 281 |

## The rules for rows outside the ledger

| Rule | Takes | Exit | Group | Required by |
|---|---|---|---|---|
| N0 | a note marked `empty` in `notes/marks.tsv` or `notes/new-rows.tsv` | not a row | none | not a finding (`notes/README.md`); counted and listed, but given no exit |
| N1 | a note marked `repeat` | not a row of its own | points at its record's row (`ledger_id`) | plan line 257 |
| N2 | a note, a new row or an extra row (`notes/extra-rows.tsv`) marked `split` | 2 | `N-split` | plan line 257; audit-method "a split goes to the user" |
| N3 | marked `prod` | 2 | `N-prod` | plan line 257 |
| N4 | marked `finding` | 2 | `N-finding` | plan line 257 |
| X2 | a `cross-rows.md` row routed `2` | 2 | its X family | plan line 225 onward (E4) |
| Xp | a `cross-rows.md` row routed `prod` | 2 | `N-prod` | E4 |
| Xc | a `cross-rows.md` row routed `closure?` | 4 only when this file's "Closures" section holds the main session's sentence for it; otherwise `no rule` | none | plan line 303: an exit 4 row is rule 3 or an E4 closure with its sentence |
| L1 | a record title in `not-in-ledger.md` (a record with no verdict the script could read) | `no rule` | none; a question | plan line 281 |

## How the acceptance count is made

The plan's count (line 303) adds the ledger's rows, the rows of `cross-rows.md`, the non-duplicate rows of `notes.tsv`, the T6 rows and `not-in-ledger.md`. Three readings keep each row from being counted twice:

- **Notes:** the "non-duplicate" notes are the ones not marked `empty` and not marked `repeat`. An `empty` note is not a finding, and a `repeat` note is its record's row. The new rows and the extra rows count the same way.
- **T6:** the four T6 rows are already ledger records (`t6` = `t6`), so they are counted once, as ledger rows.
- **`not-in-ledger.md`:** its 586 count rows are already ledger count rows, so only its 208 record titles add rows.

## Closures

The main session writes one line here for each `cross-rows.md` row it closes at exit 4: the row, and one sentence saying why it is not a problem. None is written yet.

## Exit 3 quotes

The main session pastes one block here for each row it closes at exit 3: the row, the quote of the user's words, its file and line under `## Confirmed`, and the two sides of the split the quote names. None is pasted yet.
