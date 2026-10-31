# Audit of the past audits: what is left (2026-10-30, session 9a82ff65)

This replaces `docs/handoffs/2026-10-29-audit-of-audits-followup-handoff.md`. The audit's results are in `docs/audit/2026-10-29-audit-of-audits/` (`summary.md`, `questions.md`). Every ruling so far is in the decision log `docs/audit/2026-10-03-fix-decisions.md`, as D269, D270 and D271. Nothing in this handoff has been built, and each piece waits for the user's yes (CLAUDE.md rule 3).

## How the user wants questions asked (learned the hard way this session)

- **Lead with whether a real case exists.** Asked about Q5 and Q6, the user said: "is there any real chart where this actually matters or are you just inventing situations to waste my time", then "point me to a real chart where this could be an issue, not one you created, and i'll consider it". Show a real chart, file or screen for each question. Say so up front when there is none.
- **Never use an invented number as if it were real.** An example value used only to illustrate (90 from `90abc`, a 70 ms hit window) confused the user: "where are all these numbers coming from?" Name the source of every number, and mark made-up examples as made up.
- **Show the explanation in the question itself.** Text written before a question box doesn't reach the user. Say "now that the keys are explained" only if the explanation is in the box.
- **A rule nobody picked is not a ruling.** Two hit window numbers, and two `ch` server safety rules, came in through agents' plans and batch approvals. Trace where a rule came from before treating it as the user's.

## 1. The builds the user has already ruled on

None has a plan yet. Several overlap, so plan them together and split them into tasks by the files they touch.

- **D269: one rating rule for every Backend timings row.** The design is `docs/audit/2026-10-29-audit-of-audits/blocks/AOA-R2-design.md`, read with D269's calls 2 to 5. Don't build from AOA-R1. The Rating cell changes on screen, so the memory rule `ui-changes-need-real-app-check` applies: the user sees it in the real app before the merge. The 29-10 handoff's section 1 lists the files it touches.
- **D270: the settings and rules files.** Out-of-range values land on the nearest edge everywhere, including `sp_cap` and `hydra_replay --cap 0`. Anything bad refuses with an error naming the file, key and value, in both files: a line with no `=`, junk numbers, a bad on/off, an unknown difficulty word, and an unknown key. Keys Hydra used to write are skipped and dropped at the next save. `hit_window_ms` is removed, and one fixed 85 ms constant serves the badges, the ratings, the report tiers and the search (today these are `kDefaultHitWindowMs` and `kEarlyFillReachMs`, both 85).
  - D270 and D269 overlap. D269's rating tiers read the hit window that D270 makes fixed.
  - The search's number doesn't change, so no stamp moves.
  - What a refused settings file looks like at start is a display change, so the plan shows its design to the user.
  - `CONTEXT.md`'s "Hit window" entry and the User Guide's `hit_window_ms` lines change with it.
- **D271 items 3 to 8.**
  - One "nothing matches" sentence and hint pattern (Q8).
  - Time left from the rate over recent work in both places (Q10). The batch's window has to be measured first: an agent times a batch on the test charts with each candidate window, and the user picks after seeing the estimates.
  - One space each side of the batch strip's dot (Q11).
  - One "unknown option" line for four tools (Q12).
  - Both report windows keep their rows on close (Q13). This reverses D103 item 28, including its build-cancel on close and its strip-button drop.
  - The toolbar's Compare button works at any SP cap, and the page says it compares at cap 4 (Q14).
  - All of these change what the app shows, so each shows its design to the user first.

## 2. The questions not yet sent

They are in `questions.md`. Before sending any of them, re-read the code it quotes on that day, and lead with the real case.

- **Q15: the table of seventeen numbers** the user once confirmed without seeing them (network timeouts, the 6 s status fade, and others). Each number's source is in the table. If the user keeps them, the docs that cite "D48, Q33" for these values should cite the new record instead: `CONTEXT.md`, ADR 0008 and ADR 0019, at the lines `questions.md` lists.
- **Q16: the SP end by tick against by milliseconds.** They can only disagree at a tempo near 1e14 BPM, which no real chart has. By the user's rule above, recommend dropping it the way Q5 and Q6 were dropped, and say why in one line.
- **Q17: the media guard (D189)** that the 2026-10-25 hook rebuild dropped without naming it. **Q18: shadow mode for new hook deny rules.** Both are about the hooks repo, `C:\Users\Patrick\.claude`.

Q7 (whether written fills stop generated ones) is not a question any more. It waits on the `ch` server's live check, live step LS3 in `docs/handoffs/2026-10-30-ch-mcp-star-power-build-handoff.md`. Record its result against D271 item 2.

## 3. Records to correct

None of these needs a question. They are false statements about today's code or rulings. Fix each against the code first (agent rule 9).

- **The 3 ms leeway's origin is wrong in three places.** `CONTEXT.md` (Backend leeway) calls it "Hydra's own setting". `docs/audit/ch-evidence.md` finding 304 calls it "the user's decision". `docs/differences-from-public-hydra.md` lists it among Hydra Deluxe's own choices. It is DragonDelgar's (D269 call 4, and public Hydra's `hydra/hypath.py` line 678 at 3b49012c). The differences page is public, so its edit goes through the doc-review gate and `docs/agents/public-doc-style.md`.
- **The other false records, from `summary.md`'s "Records to correct":**
  - D53 item 4: the xN disc.
  - ADR 0023 lines 92-93: the late meter walk.
  - ADR 0008 lines 62-63: the 59.5 s switch.
  - The `auto_open_report` comment in `src/app/config.h`.
  - `CONTEXT.md` lines 735-737: the 6/8 beat claim.
  - The D13 comment in `src/core/model.h`.
  - The uncalled `last_note_start_ms`.
  - The 2026-09-24 and 2026-09-26 decisions that live only in session cards `2026-09-24-da599242` and `2026-09-26-4908325e`.
- **Two raw audit blocks.** `blocks/AOA-S.md` and `blocks/AOA-W2b.md` still say the media guard's branch was "never merged". It was merged as hooks repo 1380a37 on 2026-10-10.

The two `src/` comment fixes are code. They go on a branch, with a reviewer.

## 4. Open by the audit's stop rule

`summary.md`'s "Open, by the stop rule" lists what the audit's two waves didn't reach: second claims inside concept-audit items, D114 items 17 and 18 against D255, three concept side notes, and five weak PASSes. Take these only if the user asks.

## State at handoff

- `main` is at a18b85dd, the D271 record, which is the only commit not pushed. Another session pushed the earlier ones, f6143801 (D269 and D270) and the reference prompt commits. This handoff's commit comes on top.
- The Clone Hero knowledge-base prompt is `docs/handoffs/2026-10-30-clone-hero-reference-session-prompt.md`. Its first page could settle several "unknown" rows the audit keeps meeting.
- Memory updated this session: `ch-mcp-server-2026-10`. It records the bot facts and that the user lifted the two server rules.
