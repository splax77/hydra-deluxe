# T8 fold: the 115 re-checked rows and their exits

Built by the main session from reports/TOOL-AUDIT-T8-R1.md to R6.md, one line per row. A report section that names several ids gives each id its own line. Each REAL row leaves by the user's answer that names its kind, and a row with more than one kind lists an exit per kind. NOT A SPLIT rows leave by exit 4 with the report's sentence, and GONE rows as gone. None of these closures counts until the T8 closure check (`briefs/T8-closure.md`, reports `reports/TOOL-AUDIT-T8-K<n>.md`) passes it.

Counted by row with `scripts/parse.py` and `scripts/fold.py`: REAL 64, NOT A SPLIT 49, GONE 2, with the L-F22-24 correction (before it, REAL 63 and GONE 3). The earlier counts in log.md (REAL 42, 44, 45) were taken over report sections, not rows, and missed the kinds three R3 sections wrote inside the verdict line.

Of the 64 REAL rows, 54 fold whole, L-F21-09 and L-F32-10 fold in part, and 8 are questions. The main session's own readings are in the last column, each with its reason.

| Row | Report | Verdict | Kinds | Exit | Main session's note |
|---|---|---|---|---|---|
| L-F02-02 | R4 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F02-12 | R2 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F02-13 | R4 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F02-20 | R4 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F03-02 | R4 | REAL | code-vs-code;moved-pointer | code-vs-code: exit 3, "Merge, I pick" (log.md, T8 batch 1); pick goes in the fix plan; moved-pointer: exit 3, "Fix without asking" (log.md, T6) |  |
| L-F03-08 | R4 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F03-20 | R4 | REAL | homeless-rule;memory-vs-other;ruling-vs-other | exit 2, question: the render note's scrolling step against the Confirmed line at rulings/ui-and-display.md:27 | no answer covers ruling-vs-other |
| L-F04-07 | R2 | REAL | ruling-vs-other | exit 2, question: theme 17, whether any fixed line budget should exist | no answer covers ruling-vs-other |
| L-F04-13 | R6 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F04-18 | R6 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F05-06 | R6 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F05-07 | R4 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F05-08 | R4 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F05-12 | R4 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F06-13 | R2 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F06-15 | R2 | REAL | doc-vs-doc;memory-vs-other | doc-vs-doc: exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan; memory-vs-other: exit 3, "Note is wrong" (log.md, T6) |  |
| L-F06-16 | R6 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan | R6 said doc-vs-code, but the stall notice is text a hook prints and enforces nothing, so both sides are instructions |
| L-F07-01 | R2 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F07-02 | R4 | REAL | doc-vs-code;memory-vs-other | doc-vs-code: exit 3, "The code wins" (log.md, T6); memory-vs-other: exit 3, "Note is wrong" (log.md, T6) |  |
| L-F07-09 | R2 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F07-10 | R4 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F07-16 | R2 | REAL | ruling-vs-other | exit 2, question: theme 17, whether any fixed line budget should exist | no answer covers ruling-vs-other |
| L-F07-17 | R4 | REAL | memory-vs-other | exit 3, "Note is wrong" (log.md, T6) |  |
| L-F08-09 | R6 | REAL | code-vs-code | exit 3, "Merge, I pick" (log.md, T8 batch 1); pick goes in the fix plan |  |
| L-F09-12 | R6 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F09-13 | R3 | REAL | doc-vs-code;doc-vs-doc | doc-vs-code: exit 3, "The code wins" (log.md, T6); doc-vs-doc: exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F10-23 | R4 | REAL | doc-vs-code | exit 3, "The code wins" (log.md, T6) |  |
| L-F11-04 | R4 | REAL | moved-pointer | exit 3, "Fix without asking" (log.md, T6) |  |
| L-F13-06 | R3 | REAL | doc-vs-code;doc-vs-doc | doc-vs-code: exit 3, "The code wins" (log.md, T6); doc-vs-doc: exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F13-08 | R3 | REAL | doc-vs-code;doc-vs-doc | exit 2, question: whether gh issue close --comment, which posts text with no deny rule, should pass the publish gate | "The code wins" keeps an ungated way to post text; "Merge, I pick" takes the safer reading; the two answers point opposite ways |
| L-F13-09 | R2 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F13-17 | R3 | REAL | doc-vs-code;doc-vs-doc | doc-vs-code: exit 3, "The code wins" (log.md, T6); doc-vs-doc: exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F13-20 | R2 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F13-23 | R3 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F13-24 | R4 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F14-01 | R6 | REAL | doc-vs-code;doc-vs-doc;memory-vs-other | doc-vs-code: exit 3, "The code wins" (log.md, T6); doc-vs-doc: exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan; memory-vs-other: exit 3, "Note is wrong" (log.md, T6) |  |
| L-F14-02 | R3 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F14-03 | R3 | REAL | doc-vs-code;doc-vs-doc | doc-vs-code: exit 3, "The code wins" (log.md, T6); doc-vs-doc: exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F14-06 | R6 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F14-10 | R4 | REAL | homeless-rule;memory-vs-other;ruling-vs-other | exit 2, question: same split as L-F03-20 | no answer covers ruling-vs-other |
| L-F14-11 | R4 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F14-12 | R3 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F15-01 | R3 | REAL | memory-vs-other | exit 3, "Note is wrong" (log.md, T6) |  |
| L-F15-15 | R4 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F16-08 | R3 | REAL | doc-vs-code;doc-vs-doc | exit 2, question: same split as L-F13-08 | duplicate of L-F13-08 |
| L-F16-19 | R4 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F17-02 | R6 | REAL | doc-vs-code | exit 3, "The code wins" (log.md, T6) |  |
| L-F17-16 | R4 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F18-02 | R1 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F18-04 | R1 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F18-12 | R1 | REAL | doc-vs-code;doc-vs-doc | doc-vs-code: exit 3, "The code wins" (log.md, T6); doc-vs-doc: exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F18-14 | R6 | REAL | doc-vs-doc;moved-pointer | doc-vs-doc: exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan; moved-pointer: exit 3, "Fix without asking" (log.md, T6) |  |
| L-F18-18 | R4 | REAL | doc-vs-code | exit 3, "The code wins" (log.md, T6) |  |
| L-F18-20 | R1 | REAL | doc-vs-code;doc-vs-doc | doc-vs-code: exit 3, "The code wins" (log.md, T6); doc-vs-doc: exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F18-21 | R4 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F18-22 | R4 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F19-01 | R1 | REAL | claimed-ruling;memory-vs-other | claimed-ruling: exit 3, "Drop the claim" (log.md, T8 batch 1); memory-vs-other: exit 3, "Note is wrong" (log.md, T6) |  |
| L-F19-12 | R5 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F19-14 | R5 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F19-21 | R3 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F20-01 | R5 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F20-04 | R5 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F20-06 | R5 | GONE | - | closure: GONE, the split is no longer in the files |  |
| L-F20-07 | R5 | REAL | doc-vs-code;doc-vs-doc | doc-vs-code: exit 3, "The code wins" (log.md, T6); doc-vs-doc: exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F21-04 | R6 | REAL | code-vs-code | exit 3, "Merge, I pick" (log.md, T8 batch 1); pick goes in the fix plan |  |
| L-F21-06 | R2 | REAL | ruling-vs-other | exit 2, question: theme 17, whether any fixed line budget should exist | no answer covers ruling-vs-other |
| L-F21-09 | R6 | REAL | moved-pointer | D49's missing file name: exit 3, "Fix without asking"; the rest: open, sub-items no agent has read | T7 C14 split the row; R6 read only the D49 piece |
| L-F22-04 | R6 | REAL | doc-vs-code | exit 3, "The code wins" (log.md, T6) |  |
| L-F22-09 | R6 | REAL | doc-vs-code | exit 3, "The code wins" (log.md, T6) |  |
| L-F22-13 | R5 | REAL | moved-pointer | exit 3, "Fix without asking" (log.md, T6) |  |
| L-F22-24 | R2 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan | R2 said GONE after looking in Hydra; the brief is in ~\.claude\tools\session_cards (log.md, T8 re-check) |
| L-F24-09 | R1 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F24-11 | R5 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F24-12 | R2 | REAL | doc-vs-doc;memory-vs-other | doc-vs-doc: exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan; memory-vs-other: exit 3, "Note is wrong" (log.md, T6) |  |
| L-F25-16 | R6 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F25-18 | R6 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F25-19 | R5 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F26-02 | R5 | REAL | doc-vs-doc;memory-vs-other | doc-vs-doc: exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan; memory-vs-other: exit 3, "Note is wrong" (log.md, T6) |  |
| L-F26-04 | R6 | REAL | homeless-rule;memory-vs-other | exit 2, question: whether status relays skip test_ agents | "Note is wrong" drops the skip and "Give each a home" keeps it; the two answers point opposite ways |
| L-F26-13 | R3 | REAL | memory-vs-other | exit 3, "Note is wrong" (log.md, T6) |  |
| L-F26-15 | R5 | REAL | doc-vs-code;memory-vs-other | doc-vs-code: exit 3, "The code wins" (log.md, T6); memory-vs-other: exit 3, "Note is wrong" (log.md, T6) |  |
| L-F27-23 | R5 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F27-27 | R6 | REAL | doc-vs-code | exit 3, "The code wins" (log.md, T6) | closure checker: say whether SPEC.md's 0x0313ECB0 and constants.py's c1 answer the same question; they may be different constants |
| L-F28-06 | R5 | REAL | moved-pointer | exit 3, "Fix without asking" (log.md, T6) |  |
| L-F28-13 | R3 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F28-15 | R1 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F28-16 | R2 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F28-18 | R1 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F28-21 | R2 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F29-01 | R2 | REAL | doc-vs-doc;memory-vs-other | doc-vs-doc: exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan; memory-vs-other: exit 3, "Note is wrong" (log.md, T6) |  |
| L-F29-03 | R3 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F29-07 | R2 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F29-08 | R3 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F29-10 | R2 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F29-11 | R5 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F29-13 | R1 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F29-18 | R2 | REAL | doc-vs-doc;memory-vs-other | doc-vs-doc: exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan; memory-vs-other: exit 3, "Note is wrong" (log.md, T6) |  |
| L-F30-04 | R1 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F30-09 | R1 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F30-10 | R5 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F30-11 | R5 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F30-13 | R5 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F30-15 | R1 | REAL | memory-vs-other | exit 3, "Note is wrong" (log.md, T6) |  |
| L-F30-17 | R3 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F30-23 | R5 | GONE | - | closure: GONE, the split is no longer in the files |  |
| L-F31-01 | R1 | REAL | memory-vs-other | exit 3, "Note is wrong" (log.md, T6) |  |
| L-F31-03 | R1 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F31-06 | R2 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F31-14 | R5 | REAL | doc-vs-doc | exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan |  |
| L-F31-15 | R5 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F31-23 | R5 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F31-25 | R5 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F32-01 | R2 | REAL | doc-vs-doc;memory-vs-other | doc-vs-doc: exit 3, "Merge, I pick" (log.md, T8 batch 3); pick goes in the fix plan; memory-vs-other: exit 3, "Note is wrong" (log.md, T6) |  |
| L-F32-06 | R5 | NOT A SPLIT | - | exit 4, not a problem: the report's sentence |  |
| L-F32-10 | R6 | REAL | claimed-ruling;memory-vs-other | claimed-ruling: exit 3, "Drop the claim"; the test_ skip: exit 2, question with L-F26-04 | its memory half is the L-F26-04 skip |

