# Tooling derive-once audit: summary

## What the audit was for

A card writer stopped early on a time it had made up, and the user asked for a derive-once audit of our own tooling. Read-only scouts read the hooks in `C:\Users\Patrick\.claude\gates\`, the session-card tools, the agent files and hook prompt texts, both `CLAUDE.md` files, Hydra's `docs/agents/`, the memory notes, the skills, and the Hydra tools agents run. For every value in those files (a number, a limit, a path, a list or a rule), they asked who owns it, and whether any other place restates it, recomputes it or asks someone to guess it. The audit fixed nothing on its own. Its output is this folder, and the fixes wait for a second plan and the user's yes. That is the plan's "The short version" (`docs/superpowers/plans/2026-11-18-tooling-derive-once-audit.md` on branch `claude/tooling-derive-once-audit-plan`).

## Where the numbers come from

Every count and id list below is printed by `scripts/final_exits.py`, run from this folder as `py -I scripts/final_exits.py .`. It reads this folder's files and writes nothing. The section "Script output" at the end is its output on 2026-11-19, pasted whole.

The script gives each of the 661 ledger rows one final exit, in this order of precedence:

1. It starts from `exits.md` (the T6 category of 660 rows).
2. It applies T7's closure verdicts (`reports/TOOL-AUDIT-T7-C01.md` to `C23.md`).
3. For the question rows of themes 1 to 10 and 16 (`reports/TOOL-AUDIT-T8-G1.md`), it applies the user's answer for the theme, or for themes 4, 5 and 16, the half of the answer each row's Sides text names (`ROW_ANSWERS`).
4. It applies `fold-T8.md` for the 115 rows the T8 re-check read, which win.
5. It applies `late-folds.md` for five rows no stage had placed, and `T9_REFOLD` for the rows the T9 closure check reopened or found gone.

When a row has parts with different exits, the part that still needs work wins: still open, exit 2, Fix pending, exit 3, exit 1, exit 4, GONE, refuted.

## The counts

- **Ledger:** 661 rows. `exits.md` has 660; L-F04-14 never reached it (late-folds.md).
- **By verdict** (exits.md): CONFIRMED 479, CONFIRMED-CHANGED 170, REFUTED 7, NOT VERIFIED 4. L-F04-14 is CONFIRMED in its ledger file.
- **T6 categories** (exits.md): A 79, B 33, C 21, P 282, Q 238, R 7.
- **T7** (by row): 415 rows, PASS 393, REOPEN 19, GONE 3. log.md's PASS 400 counts 422 verdicts, since some rows were checked as bundles.
- **T8** (fold-T8.md, "Counts"): 115 rows re-checked; after its closure check (K1 to K7), REAL 66, NOT A SPLIT 47, GONE 2.
- **T9 closure check** (briefs/T9-closure.md, reports/TOOL-AUDIT-T9-K1.md to K8.md): the 142 theme-answered rows and L-F04-14, never checked by a fresh agent before. PASS 121, REOPEN 18, GONE 4.

Final exits, all 661 rows:

| Final exit | Rows |
|---|---|
| Still open (NOT VERIFIED) | 3 |
| Fix pending (copies that agree, or the rename) | 272 |
| Exit 3, closed by the user's answer | 314 |
| Exit 1, fixed | 3 |
| Exit 4, not a problem | 54 |
| GONE | 8 |
| Refuted | 7 |
| Total | 661 |

No row is a question (exit 2), and no row is a gap. 587 rows still have a fix to make: every row with a Fix pending part or an exit 3 part whose fix is not done.

Every question the user was asked, with the answer and the rows it closed, is in `questions.md` (Q1 to Q23). Each batch of answers is a D-record in `docs/audit/2026-10-03-fix-decisions.md`: D345 to D351. They are the user's answers in chat, not filed rulings, except where an item cites a `## Confirmed` line.

## What was not audited, and why

The plan's Scope section leaves out every fix (each needs its own plan and yes), whether any hook works or should exist (the two earlier hook audits asked that), Hydra's game code and tests (other audits cover them), the old `C:\Users\Patrick\.claude\hooks\` folder (the user had it deleted rather than read), and running anything, so no claim here is proven by a run.

`coverage.md` names what no scout inventoried, though finders still searched it for copies: the function bodies of `tools/ch_mcp` and its tests, `ch_probe` beyond `constants.py`, `agent\*.ts` beyond its top constants, the precheck check bodies and the replay bodies, the mutate bodies and `mutate\test_*.py`, `test_hydra_ui_mcp.py`, `tools\test_*` beyond their heads, `shots.txt` after about line 30, and the other `tools\` scripts in section 5 of I10's report. The memory `archive\` folder and `skills\synced\` were out of scope.

## The rows still NOT VERIFIED

`exits.md` keeps a row NOT VERIFIED after both verifier rounds open whatever its category. Three remain: L-F12-17 (category P; T7 C08 passed it), L-F19-20 (P; C13 passed it "with a limit stated") and L-F20-13 (B; C13 passed it with items unopened). A T7 pass reads the closure, not the finding, so these stay open until a verifier confirms them. The fourth, L-F25-16, is in fold-T8.md, which wins: R6 found it not a split and K4 passed that, so it is exit 4.

## The Fix pending list

The audit is not closed while this list has rows. The script prints every id under the answer that sends it there, in "Fix pending and exit 3 rows by answer" below. In short:

- **Already done.** L-F13-08, L-F13-17 and L-F16-08 left by exit 1: `docs/agents/issue-tracker.md:14` now posts a closing comment through publish.ps1 (merge 6acfbed4). L-F03-20 and L-F14-10 need nothing more: their memory note was deleted the same day, on the user's "delete that memory note. search function exists for a reason."
- **Copies that agree, to become pointers to their owner:** 272 rows (category P with a T7 pass). Two are still NOT VERIFIED.
- **The rename:** L-F27-27, constants.py's two swapped formula tables, no value change.
- **Exit 3 rows, by answer:** "The code wins" 94, "Note is wrong" 46, "Fix without asking" 28, "Drop the claim" 25, "Merge, I pick" 105 (each pick listed for the user's yes), "Give each a home" 21, "Confirmed only" 6, "Trust both" 4, "Minutes (line 34)" 3, "Finish, report the rest" 1, the Confirmed budget line 3, "Yes, state the count" 3, the Confirmed SP-end line 1 and "Move into ui-testing.md" 1. One more row, L-F02-08, is Fix pending rather than exit 3: T9 K1 found its value (the chart's resolution) has no owner and is read from the chart instead of typed by the agent. The script's "by answer" list below prints it with the exit 3 answers, but its final exit is Fix pending.

Fix-plan items outside the rows, from log.md:

- **The time-stamp fix.** Agents type HH:MM into status lines, though status_append.ps1 could stamp the time itself (L-F22-24 belongs with it).
- **status_append.ps1 appends to any .md path.** It checks an agent record only inside the status folder, and its allow rule lets other paths through with no prompt.
- **tools/build_slot.ps1 keeps its state under the removed `C:\Users\Patrick\.claude\hooks\` folder** (L-F11-01 and its theme 8 rows).
- **The hook trims.** About 272 lines can go with no behaviour change, and about 550 to 600 more from the cleanup sweep, the card start hook and the rulings checker, plus the sweep's log pruning if the user drops it. Those sizes were measured at 5,824 lines; the hooks are 6,045 now. review-and-hooks.md:11 asks about efficiency and function, not a number.
- **The build-count change** to agent-rules.md rule 3, and the one-build-per-wave rule's home there (D350).
- **Hook gaps the checkers recorded,** named only, with no steps: the ref gate does not check that the `Agent:` trailer names the agent that made the commit; nothing reads the "Holds the merge:" prefix; a plan pasted into chat has no hook behind it; no test pins agent_stop's never-block registration; a chained command that starts with `git` passes the overdue-status refusal (L-F01-09); a `SIGN-OFF: BLOCKED` line with text after it is recorded clean (L-F24-08).
- **A missing session card.** The build-count words in D350 come from session 8e6a2908, which has no card in `docs/sessions/`.

## What happens next

The fixes are a separate plan that needs the user's yes. The audit stays open while the Fix pending list has rows and while the three NOT VERIFIED rows are unconfirmed.

## Script output

### Counts

Ledger rows (ledger/): 661. Rows in exits.md: 660.

By verdict (exits.md Verdict column): CONFIRMED 479, CONFIRMED-CHANGED 170, REFUTED 7, NOT VERIFIED 4.

By kind (ledger Kind lines; a row with several kinds counts once under each): K1 432, K2 110, K3 143, K4 19, K5 90, K6 163, K7 115, K8 119.

T6 categories (exits.md): A 79, B 33, C 21, P 282, Q 238, R 7.

T7 verdicts by row (reports/TOOL-AUDIT-T7-C*.md): 415 rows; GONE 3, PASS 393, REOPEN 19.

fold-T8.md: 115 rows; verdicts after the closure check: GONE 2, NOT A SPLIT 47, REAL 66.

Final exit by row: still open 3, Fix pending 272, exit 3 314, exit 1 3, exit 4 54, GONE 8, refuted 7; gaps 0; total 661.

Where each final exit came from: T7 GONE 3, exits.md 7, exits.md + T7 PASS 393, fold-T8.md 115, late-folds.md 1, theme 1 24, theme 10 8, theme 16 8, theme 2 17, theme 3 10, theme 4 5, theme 5 7, theme 6 7, theme 7 19, theme 8 11, theme 9 26.

### Gaps


### Rows by final exit

**still open** (3): L-F12-17, L-F19-20, L-F20-13

**Fix pending** (272): L-F01-10, L-F01-12, L-F01-13, L-F01-14, L-F01-16, L-F01-17, L-F01-18, L-F01-19, L-F01-20, L-F01-21, L-F01-22, L-F01-23, L-F01-24, L-F01-25, L-F02-01, L-F02-03, L-F02-04, L-F02-07, L-F02-08, L-F02-09, L-F02-14, L-F02-15, L-F02-16, L-F02-17, L-F02-18, L-F02-19, L-F03-03, L-F03-07, L-F03-09, L-F03-15, L-F03-18, L-F03-19, L-F03-21, L-F03-22, L-F04-01, L-F04-02, L-F04-03, L-F04-04, L-F04-08, L-F04-09, L-F04-12, L-F04-19, L-F05-09, L-F05-10, L-F05-11, L-F05-14, L-F05-15, L-F05-16, L-F05-17, L-F05-18, L-F05-19, L-F06-01, L-F06-02, L-F06-03, L-F06-11, L-F06-18, L-F06-19, L-F07-04, L-F07-12, L-F07-13, L-F07-14, L-F07-15, L-F08-01, L-F08-02, L-F08-03, L-F08-04, L-F08-05, L-F08-07, L-F08-08, L-F08-16, L-F09-01, L-F09-02, L-F09-04, L-F09-05, L-F09-07, L-F09-10, L-F09-11, L-F09-14, L-F09-15, L-F09-17, L-F09-18, L-F09-19, L-F10-05, L-F10-10, L-F10-12, L-F10-14, L-F10-15, L-F10-16, L-F10-17, L-F10-18, L-F10-20, L-F11-05, L-F11-10, L-F11-12, L-F11-13, L-F11-14, L-F11-15, L-F11-17, L-F12-08, L-F12-09, L-F12-11, L-F12-13, L-F12-14, L-F12-18, L-F13-12, L-F13-13, L-F13-14, L-F13-16, L-F13-18, L-F13-19, L-F14-09, L-F14-13, L-F14-15, L-F14-16, L-F14-17, L-F15-02, L-F15-03, L-F15-04, L-F15-05, L-F15-06, L-F15-07, L-F15-08, L-F15-13, L-F16-04, L-F16-05, L-F16-09, L-F16-10, L-F16-11, L-F16-12, L-F16-15, L-F16-16, L-F16-17, L-F16-22, L-F17-03, L-F17-05, L-F17-10, L-F17-15, L-F17-18, L-F17-19, L-F17-20, L-F18-06, L-F18-08, L-F18-09, L-F18-15, L-F18-19, L-F19-09, L-F19-10, L-F19-11, L-F19-13, L-F19-15, L-F19-16, L-F19-18, L-F19-19, L-F20-08, L-F21-01, L-F21-05, L-F21-08, L-F22-01, L-F22-03, L-F22-05, L-F22-06, L-F22-07, L-F22-10, L-F22-15, L-F22-17, L-F22-18, L-F22-19, L-F22-20, L-F22-21, L-F22-23, L-F23-01, L-F23-02, L-F23-03, L-F23-04, L-F23-05, L-F23-11, L-F23-12, L-F24-03, L-F24-04, L-F24-05, L-F24-06, L-F24-07, L-F24-13, L-F24-14, L-F25-03, L-F25-05, L-F25-06, L-F25-08, L-F25-09, L-F25-10, L-F25-11, L-F25-14, L-F25-22, L-F25-24, L-F26-05, L-F26-06, L-F26-08, L-F26-14, L-F26-16, L-F26-17, L-F26-18, L-F27-07, L-F27-08, L-F27-13, L-F27-14, L-F27-16, L-F27-17, L-F27-19, L-F27-20, L-F27-24, L-F27-25, L-F27-26, L-F27-27, L-F27-29, L-F28-01, L-F28-03, L-F28-05, L-F28-07, L-F28-11, L-F28-12, L-F28-22, L-F29-02, L-F29-05, L-F29-12, L-F29-20, L-F30-01, L-F30-03, L-F30-05, L-F30-07, L-F30-08, L-F30-14, L-F30-16, L-F30-18, L-F30-21, L-F30-22, L-F31-02, L-F31-04, L-F31-05, L-F31-10, L-F31-12, L-F31-13, L-F31-16, L-F31-17, L-F31-18, L-F31-19, L-F31-20, L-F31-21, L-F31-24, L-F31-26, L-F32-02, L-F32-04, L-F32-05, L-F32-11, L-F32-13, L-F32-14, L-F32-16, L-F32-17, L-F32-18, L-F32-19, L-F32-22, L-F33-01, L-F33-05, L-F33-07, L-F33-09, L-F33-13, L-F33-14, L-F33-15, L-F33-16, L-F33-17, L-F33-18, L-F33-20, L-F33-21

**exit 3** (314): L-F01-01, L-F01-02, L-F01-03, L-F01-04, L-F01-05, L-F01-06, L-F01-07, L-F01-08, L-F01-11, L-F01-15, L-F01-26, L-F02-05, L-F02-06, L-F02-10, L-F02-11, L-F03-01, L-F03-02, L-F03-04, L-F03-05, L-F03-06, L-F03-10, L-F03-11, L-F03-12, L-F03-13, L-F03-14, L-F03-16, L-F03-17, L-F03-20, L-F04-05, L-F04-06, L-F04-07, L-F04-10, L-F04-14, L-F04-15, L-F04-16, L-F04-17, L-F04-20, L-F05-01, L-F05-02, L-F05-03, L-F05-04, L-F05-05, L-F05-13, L-F06-04, L-F06-05, L-F06-06, L-F06-07, L-F06-08, L-F06-10, L-F06-12, L-F06-14, L-F06-15, L-F06-16, L-F06-17, L-F07-02, L-F07-03, L-F07-05, L-F07-06, L-F07-07, L-F07-08, L-F07-11, L-F07-16, L-F07-17, L-F08-09, L-F08-10, L-F08-11, L-F08-12, L-F08-13, L-F08-15, L-F09-03, L-F09-06, L-F09-08, L-F09-09, L-F09-12, L-F09-13, L-F09-16, L-F10-01, L-F10-02, L-F10-03, L-F10-04, L-F10-06, L-F10-07, L-F10-08, L-F10-09, L-F10-11, L-F10-13, L-F10-19, L-F10-21, L-F10-22, L-F10-23, L-F11-01, L-F11-02, L-F11-03, L-F11-04, L-F11-06, L-F11-07, L-F11-08, L-F11-09, L-F11-11, L-F11-16, L-F12-01, L-F12-02, L-F12-03, L-F12-04, L-F12-05, L-F12-06, L-F12-07, L-F12-12, L-F12-15, L-F12-16, L-F13-01, L-F13-02, L-F13-03, L-F13-04, L-F13-05, L-F13-06, L-F13-07, L-F13-09, L-F13-10, L-F13-11, L-F13-15, L-F13-21, L-F13-22, L-F13-23, L-F14-01, L-F14-03, L-F14-04, L-F14-05, L-F14-07, L-F14-08, L-F14-10, L-F14-14, L-F15-01, L-F15-09, L-F15-10, L-F15-11, L-F15-12, L-F15-14, L-F15-15, L-F16-01, L-F16-02, L-F16-03, L-F16-06, L-F16-07, L-F16-14, L-F16-20, L-F16-21, L-F16-23, L-F17-01, L-F17-02, L-F17-04, L-F17-07, L-F17-08, L-F17-09, L-F17-11, L-F17-12, L-F17-13, L-F17-14, L-F17-16, L-F17-17, L-F18-01, L-F18-02, L-F18-03, L-F18-05, L-F18-07, L-F18-10, L-F18-12, L-F18-13, L-F18-14, L-F18-17, L-F18-18, L-F18-20, L-F18-22, L-F19-01, L-F19-02, L-F19-03, L-F19-05, L-F19-06, L-F19-07, L-F19-08, L-F19-17, L-F20-01, L-F20-02, L-F20-03, L-F20-05, L-F20-07, L-F20-09, L-F20-10, L-F20-11, L-F20-12, L-F21-02, L-F21-03, L-F21-04, L-F21-06, L-F21-07, L-F21-09, L-F22-02, L-F22-04, L-F22-08, L-F22-09, L-F22-11, L-F22-12, L-F22-13, L-F22-14, L-F22-16, L-F22-22, L-F22-24, L-F23-06, L-F23-07, L-F23-08, L-F23-09, L-F23-10, L-F23-13, L-F24-01, L-F24-02, L-F24-08, L-F24-09, L-F24-10, L-F24-12, L-F24-15, L-F25-01, L-F25-02, L-F25-04, L-F25-07, L-F25-13, L-F25-15, L-F25-17, L-F25-19, L-F25-20, L-F25-21, L-F25-23, L-F26-01, L-F26-02, L-F26-03, L-F26-04, L-F26-09, L-F26-10, L-F26-11, L-F26-12, L-F26-13, L-F26-15, L-F26-19, L-F26-20, L-F27-01, L-F27-03, L-F27-04, L-F27-05, L-F27-06, L-F27-09, L-F27-10, L-F27-11, L-F27-12, L-F27-15, L-F27-21, L-F27-22, L-F27-28, L-F28-02, L-F28-04, L-F28-06, L-F28-08, L-F28-10, L-F28-13, L-F28-14, L-F28-15, L-F28-17, L-F28-19, L-F28-20, L-F28-21, L-F29-01, L-F29-03, L-F29-04, L-F29-06, L-F29-08, L-F29-09, L-F29-13, L-F29-14, L-F29-15, L-F29-16, L-F29-17, L-F29-18, L-F29-19, L-F30-02, L-F30-09, L-F30-10, L-F30-12, L-F30-15, L-F30-19, L-F30-20, L-F31-01, L-F31-07, L-F31-08, L-F31-09, L-F31-11, L-F31-14, L-F31-22, L-F31-27, L-F32-01, L-F32-03, L-F32-07, L-F32-08, L-F32-09, L-F32-10, L-F32-12, L-F32-15, L-F32-20, L-F32-21, L-F33-02, L-F33-03, L-F33-04, L-F33-06, L-F33-08, L-F33-11, L-F33-12, L-F33-19

**exit 1** (3): L-F13-08, L-F13-17, L-F16-08

**exit 4** (54): L-F01-09, L-F02-02, L-F02-12, L-F02-13, L-F02-20, L-F03-08, L-F04-13, L-F04-18, L-F05-06, L-F05-07, L-F05-08, L-F05-12, L-F06-13, L-F07-01, L-F07-09, L-F07-10, L-F12-10, L-F13-20, L-F13-24, L-F14-02, L-F14-06, L-F14-11, L-F14-12, L-F16-19, L-F18-04, L-F18-11, L-F18-21, L-F19-12, L-F19-14, L-F19-21, L-F20-04, L-F24-11, L-F25-12, L-F25-16, L-F25-18, L-F27-02, L-F27-18, L-F27-23, L-F28-09, L-F28-16, L-F28-18, L-F29-07, L-F29-10, L-F29-11, L-F30-04, L-F30-11, L-F30-13, L-F30-17, L-F31-03, L-F31-06, L-F31-15, L-F31-23, L-F31-25, L-F32-06

**GONE** (8): L-F08-06, L-F16-18, L-F17-06, L-F18-16, L-F19-04, L-F20-06, L-F30-23, L-F33-10

**refuted** (7): L-F04-11, L-F06-09, L-F08-14, L-F16-13, L-F23-14, L-F26-07, L-F30-06

### Fix pending and exit 3 rows by answer

**Fix pending, copies that agree (T6 category P, T7 PASS)** (272): L-F01-10, L-F01-12, L-F01-13, L-F01-14, L-F01-16, L-F01-17, L-F01-18, L-F01-19, L-F01-20, L-F01-21, L-F01-22, L-F01-23, L-F01-24, L-F01-25, L-F02-01, L-F02-03, L-F02-04, L-F02-07, L-F02-09, L-F02-14, L-F02-15, L-F02-16, L-F02-17, L-F02-18, L-F02-19, L-F03-03, L-F03-07, L-F03-09, L-F03-15, L-F03-18, L-F03-19, L-F03-21, L-F03-22, L-F04-01, L-F04-02, L-F04-03, L-F04-04, L-F04-08, L-F04-09, L-F04-12, L-F04-19, L-F05-09, L-F05-10, L-F05-11, L-F05-14, L-F05-15, L-F05-16, L-F05-17, L-F05-18, L-F05-19, L-F06-01, L-F06-02, L-F06-03, L-F06-11, L-F06-18, L-F06-19, L-F07-04, L-F07-12, L-F07-13, L-F07-14, L-F07-15, L-F08-01, L-F08-02, L-F08-03, L-F08-04, L-F08-05, L-F08-07, L-F08-08, L-F08-16, L-F09-01, L-F09-02, L-F09-04, L-F09-05, L-F09-07, L-F09-10, L-F09-11, L-F09-14, L-F09-15, L-F09-17, L-F09-18, L-F09-19, L-F10-05, L-F10-10, L-F10-12, L-F10-14, L-F10-15, L-F10-16, L-F10-17, L-F10-18, L-F10-20, L-F11-05, L-F11-10, L-F11-12, L-F11-13, L-F11-14, L-F11-15, L-F11-17, L-F12-08, L-F12-09, L-F12-11, L-F12-13, L-F12-14, L-F12-17, L-F12-18, L-F13-12, L-F13-13, L-F13-14, L-F13-16, L-F13-18, L-F13-19, L-F14-09, L-F14-13, L-F14-15, L-F14-16, L-F14-17, L-F15-02, L-F15-03, L-F15-04, L-F15-05, L-F15-06, L-F15-07, L-F15-08, L-F15-13, L-F16-04, L-F16-05, L-F16-09, L-F16-10, L-F16-11, L-F16-12, L-F16-15, L-F16-16, L-F16-17, L-F16-22, L-F17-03, L-F17-05, L-F17-10, L-F17-15, L-F17-18, L-F17-19, L-F17-20, L-F18-06, L-F18-08, L-F18-09, L-F18-15, L-F18-19, L-F19-09, L-F19-10, L-F19-11, L-F19-13, L-F19-15, L-F19-16, L-F19-18, L-F19-19, L-F19-20, L-F20-08, L-F21-01, L-F21-05, L-F21-08, L-F22-01, L-F22-03, L-F22-05, L-F22-06, L-F22-07, L-F22-10, L-F22-15, L-F22-17, L-F22-18, L-F22-19, L-F22-20, L-F22-21, L-F22-23, L-F23-01, L-F23-02, L-F23-03, L-F23-04, L-F23-05, L-F23-11, L-F23-12, L-F24-03, L-F24-04, L-F24-05, L-F24-06, L-F24-07, L-F24-13, L-F24-14, L-F25-03, L-F25-05, L-F25-06, L-F25-08, L-F25-09, L-F25-10, L-F25-11, L-F25-14, L-F25-22, L-F25-24, L-F26-05, L-F26-06, L-F26-08, L-F26-14, L-F26-16, L-F26-17, L-F26-18, L-F27-07, L-F27-08, L-F27-13, L-F27-14, L-F27-16, L-F27-17, L-F27-19, L-F27-20, L-F27-24, L-F27-25, L-F27-26, L-F27-29, L-F28-01, L-F28-03, L-F28-05, L-F28-07, L-F28-11, L-F28-12, L-F28-22, L-F29-02, L-F29-05, L-F29-12, L-F29-20, L-F30-01, L-F30-03, L-F30-05, L-F30-07, L-F30-08, L-F30-14, L-F30-16, L-F30-18, L-F30-21, L-F30-22, L-F31-02, L-F31-04, L-F31-05, L-F31-10, L-F31-12, L-F31-13, L-F31-16, L-F31-17, L-F31-18, L-F31-19, L-F31-20, L-F31-21, L-F31-24, L-F31-26, L-F32-02, L-F32-04, L-F32-05, L-F32-11, L-F32-13, L-F32-14, L-F32-16, L-F32-17, L-F32-18, L-F32-19, L-F32-22, L-F33-01, L-F33-05, L-F33-07, L-F33-09, L-F33-13, L-F33-14, L-F33-15, L-F33-16, L-F33-17, L-F33-18, L-F33-20, L-F33-21

**"The code wins"** (log.md, T6; the doc is fixed to point at the code) (94): L-F01-07, L-F01-11, L-F04-05, L-F04-15, L-F04-17, L-F04-20, L-F05-01, L-F05-02, L-F06-04, L-F06-07, L-F06-10, L-F06-17, L-F07-02, L-F07-03, L-F07-05, L-F07-07, L-F08-11, L-F09-06, L-F09-12, L-F09-13, L-F09-16, L-F10-01, L-F10-13, L-F10-19, L-F10-23, L-F11-06, L-F11-07, L-F11-08, L-F11-09, L-F12-01, L-F12-06, L-F12-16, L-F13-02, L-F13-03, L-F13-04, L-F13-06, L-F13-15, L-F13-22, L-F14-01, L-F14-03, L-F14-04, L-F14-05, L-F14-07, L-F14-14, L-F15-12, L-F16-06, L-F17-01, L-F17-02, L-F17-07, L-F17-09, L-F18-01, L-F18-05, L-F18-18, L-F18-20, L-F19-02, L-F19-03, L-F19-05, L-F19-07, L-F19-08, L-F20-07, L-F22-04, L-F22-09, L-F22-12, L-F22-16, L-F23-06, L-F23-07, L-F23-13, L-F24-15, L-F25-02, L-F25-07, L-F25-13, L-F25-15, L-F25-17, L-F25-20, L-F25-21, L-F25-23, L-F26-11, L-F26-12, L-F26-15, L-F27-10, L-F27-11, L-F27-21, L-F27-28, L-F28-04, L-F28-08, L-F29-14, L-F29-15, L-F29-17, L-F29-19, L-F30-02, L-F30-19, L-F30-20, L-F32-20, L-F33-19

**"Note is wrong"** (log.md, T6; the memory note is fixed or pruned) (46): L-F02-06, L-F03-01, L-F03-05, L-F03-06, L-F03-13, L-F06-05, L-F06-14, L-F06-15, L-F07-02, L-F07-17, L-F09-03, L-F10-03, L-F10-04, L-F10-21, L-F13-10, L-F13-11, L-F14-01, L-F15-01, L-F15-09, L-F15-10, L-F18-01, L-F19-01, L-F20-02, L-F20-03, L-F20-05, L-F20-10, L-F20-13, L-F22-14, L-F23-10, L-F24-12, L-F26-01, L-F26-02, L-F26-04, L-F26-13, L-F26-15, L-F26-19, L-F26-20, L-F28-14, L-F29-09, L-F30-15, L-F31-01, L-F31-07, L-F31-09, L-F32-08, L-F32-09, L-F32-10

**"Fix without asking"** (log.md, T6; the pointer is moved to where the thing is now) (28): L-F03-02, L-F03-10, L-F03-11, L-F03-16, L-F04-16, L-F08-12, L-F09-08, L-F10-02, L-F10-07, L-F11-04, L-F12-12, L-F15-11, L-F16-21, L-F17-11, L-F18-07, L-F18-14, L-F19-17, L-F21-07, L-F21-09, L-F22-13, L-F26-10, L-F28-06, L-F28-10, L-F31-08, L-F31-11, L-F31-27, L-F32-15, L-F33-11

**"Drop the claim"** (log.md, T8 batch 1; the claim that the user ruled is dropped) (25): L-F01-06, L-F01-15, L-F02-10, L-F02-11, L-F04-06, L-F04-10, L-F05-03, L-F05-04, L-F05-05, L-F08-11, L-F08-13, L-F09-06, L-F10-08, L-F12-15, L-F13-11, L-F16-23, L-F17-17, L-F18-03, L-F18-17, L-F19-01, L-F22-11, L-F28-17, L-F32-10, L-F32-21, L-F33-04

**"Merge, I pick"** (log.md, T8 batches 1 and 3; the pick is listed in the fix plan for the user's yes) (105): L-F01-03, L-F01-04, L-F01-05, L-F03-02, L-F04-14, L-F05-13, L-F06-06, L-F06-08, L-F06-12, L-F06-15, L-F06-16, L-F07-06, L-F08-09, L-F08-10, L-F08-15, L-F09-06, L-F09-12, L-F09-13, L-F10-06, L-F11-01, L-F11-02, L-F11-03, L-F11-11, L-F11-16, L-F12-02, L-F12-03, L-F12-04, L-F12-05, L-F12-07, L-F13-01, L-F13-04, L-F13-06, L-F13-09, L-F13-23, L-F14-01, L-F14-03, L-F14-08, L-F15-15, L-F16-01, L-F16-02, L-F16-03, L-F16-07, L-F16-14, L-F16-20, L-F17-04, L-F17-08, L-F17-12, L-F17-13, L-F17-14, L-F17-16, L-F18-02, L-F18-12, L-F18-13, L-F18-14, L-F18-20, L-F18-22, L-F19-06, L-F20-01, L-F20-07, L-F21-02, L-F21-03, L-F21-04, L-F22-02, L-F22-08, L-F22-22, L-F22-24, L-F23-08, L-F23-09, L-F24-09, L-F24-10, L-F24-12, L-F25-04, L-F25-19, L-F26-02, L-F26-09, L-F27-01, L-F27-03, L-F27-04, L-F27-05, L-F27-06, L-F27-09, L-F27-12, L-F27-15, L-F27-22, L-F28-02, L-F28-13, L-F28-15, L-F28-20, L-F28-21, L-F29-01, L-F29-03, L-F29-04, L-F29-06, L-F29-08, L-F29-13, L-F29-16, L-F29-18, L-F30-09, L-F30-10, L-F31-14, L-F32-01, L-F33-03, L-F33-06, L-F33-08, L-F33-12

**"Give each a home"** (log.md, T8 batch 1; the rule is given a home in agent-rules.md or the preamble) (21): L-F01-26, L-F02-05, L-F03-04, L-F03-12, L-F03-14, L-F03-17, L-F09-09, L-F10-09, L-F10-22, L-F13-07, L-F13-11, L-F13-21, L-F15-14, L-F20-09, L-F20-11, L-F24-08, L-F26-13, L-F28-19, L-F31-22, L-F32-07, L-F32-12

**"Minutes (line 34)"** (log.md, T8 batch 1; review-and-hooks.md line 33 is retired and the copies follow line 34) (3): L-F01-01, L-F01-08, L-F07-08

**"Finish, report the rest"** (log.md, T8 batch 2; the warning texts are made to say: finish the item in hand, commit, list the rest) (1): L-F01-02

**"Confirmed only"** (log.md, T8 batch 2; the reviewer and premise-map pages are fixed to say a card or chat quote closes nothing) (6): L-F18-10, L-F24-01, L-F24-02, L-F25-01, L-F26-03, L-F30-12

**"Trust both"** (log.md, T8 batches 2 and 6; the delete rule and the cleanup sweep accept <repo>/worktrees as well as <repo>/.claude/worktrees, in both repos) (4): L-F07-11, L-F10-11, L-F13-05, L-F33-02

**the Confirmed line review-and-hooks.md:11** (log.md, T8 batch 4; the "user's hard budget" comment is dropped) (3): L-F04-07, L-F07-16, L-F21-06

**"Yes, state the count"** (log.md, T8 batch 5; agent-rules.md rule 3 says the build count needs a yes, and the one-build-per-wave rule gets its home there) (3): L-F29-01, L-F29-18, L-F32-01

**"if no value changes then why is this even a question?"** (log.md, T8 batch 4; constants.py's two formula tables are renamed to match the disassembly) (1): L-F27-27

**"delete that memory note. search function exists for a reason."** (log.md, T8 batch 4; done: the note was deleted the same day) (2): L-F03-20, L-F14-10

**"Fix the doc line"** (log.md, T8 batch 4; done: merge 6acfbed4) (3): L-F13-08, L-F13-17, L-F16-08

**no owner (T9 K1)** (reports/TOOL-AUDIT-T9-K1.md; the value is read from the chart instead of typed by the agent) (1): L-F02-08

**the Confirmed line code-and-engine.md:12** (docs/agents/rulings/code-and-engine.md; the copies follow the one SP end; 3 ms decides counted or uncounted only) (1): L-F32-03

**"Move into ui-testing.md"** (log.md, T8 batch 6; the four rules the deleted note held get a home in docs/agents/ui-testing.md) (1): L-F20-12

Rows with a fix still to make (any part exit 3 or Fix pending, not done): 587

