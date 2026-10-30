# Audit of the past audits: follow-up handoff (2026-10-29, session 61cee421)

The audit of every past audit is done and merged to `main` at dfcf1cd1. It is not pushed. It ran by `docs/agents/audit-method.md` and the earlier handoff `docs/handoffs/2026-10-29-audit-of-audits-handoff.md`. The results live in `docs/audit/2026-10-29-audit-of-audits/`: `summary.md` has the counts and the record, and `questions.md` has the questions for the user.

In short: twelve read-only scouts checked 219 closures from every past audit. 24 did not hold, so they become questions for the user. One fresh reviewer signed the results off clean, and the three always-run tests passed at the merge. The method page gained this audit's lessons in the same merge.

Three things are left: build the one Backend timings rating rule, put the rest of the question list to the user, and correct the records listed below. None has started, and each waits for the user's yes (CLAUDE.md rule 3).

## The user's rulings from this session, word for word

None of these has a D-record yet. The decision log's last record is D268. Record these as the next D-records before or with the build that uses them.

1. Asked "should every Backend timings row use one rating rule, with the same labels and the same edges, whether or not the row is squeezed out?", the user answered: "yes everything should use one rule."
2. Rejecting a first design that rated rows by their raw offset: "what really matters here is the effective timing for the player, not the absolute offset. hydra already shows this with the badges next to each activation. -100 for a backend and a squeeze out are not the same thing"
3. On the rating scale in the second design: "the report scale sounds good".
4. On where Star Power ends: "there should never be two different answers for where star power ends. ever. the 3ms rule is for whether a backend is included in the optimal score or uncounted. it's an arbitrary number chosen by dragondelgar and i am fine with keeping it so our scores use the same methodology as delgar's hydra"
5. On badges and the report: "keep the badges and reports as they are. if you made them use the effective figure then you'd need to remove the timing scale text or users would understandably think those multipliers need to be aplied again"

The user was also angry twice in this session, and both times it was the same mistake. First, the main session explained the two rating ladders with a purpose nobody had written down ("one row rates getting a note in, the other pushing it out"). The user's words: "why are we even using two different systems? why are you still trying to convince me of more convoluted bullshit?" Second, it passed off a big question as a small one: "you're trying to hide a very very very important question by passing it off as a small thing". Memory now holds both lessons, in `never-keep-a-split-quietly` and in the new top rule `sp-end-is-one-point-leeway-is-delgars`.

## 1. Build the one Backend timings rating rule

The design is `docs/audit/2026-10-29-audit-of-audits/blocks/AOA-R2-design.md`. Read it with rulings 2 to 5 above, which settle its three open points. Here is what the build does, in plain words.

Each Backend timings row on the Paths tab gets its word from the timing the player needs for that row. That is the eff. figure when a frontend scale applies, and raw ms otherwise. The word is banded on the report's timing tiers (`timing_tiers` in `src/core/squeeze_rating.cpp`): Normal up to 2 ms, then Hard, Extreme, Insane, Insane+ and Beyond at half a hit window, one, one and a half and two hit windows.

- A plain row the engine counts needs nothing, so it reads Normal.
- A plain uncounted row needs about its offset early, and it keeps "(uncounted)".
- The squeezed-out row needs `SPSqueeze::difficulty`, the same figure its badge shows.

`kBackendInnerBandMs` and the five "SqOut" words go. The "<-- squeezed out (-N)" suffix stays as it is. Badges, the hardest ms and the report stay on raw ms (ruling 5). Nothing stored changes, and no results stamp moves.

The Star Power end needs no code change. The main session checked this by reading, after ruling 4. Every "inside or past SP" question already uses the bare end: phrase collection (`after_sp_end` in the graph), the engine's SP walk, `needs_no_timing`, the badges and the squeeze sentences. The leeway sits only in `counted_without_squeeze`. That one test decides whether a backend row's score counts toward the optimal. D117's frontend-scale pick for counted plain rows reads it too, but only to ask what could make a counted row stop counting.

The first design (`blocks/AOA-R1-design.md`) calls the leeway a second "past the SP end" edge. That is wrong, and the user rejected it. Do not build from AOA-R1.

The leeway is DragonDelgar's. Public Hydra has `is_leeway = be.offset_ms > 0 and be.offset_ms < 3` in `hydra/hypath.py` at line 678 of his last commit, 3b49012c. That includes the reduced squeeze-out points for a leeway row.

What the build touches, per AOA-R2's section 6:

- `BackendSqueeze::summarystr` and its header comment, with `tier_for` moved from `src/app/report.cpp` into core.
- The pinned labels in `tests/test_rules.cpp`, `tests/test_model.cpp`, `tests/test_squeeze_rating.cpp`, `tests/test_activation_rows.cpp` and `tests/ui/uitest_paths.cpp`, plus boundary tests at the tier edges.
- User Guide line 275, `CONTEXT.md`'s Backend leeway and rating text, and the differences page.

The Rating cell changes on screen, so the memory rule `ui-changes-need-real-app-check` applies: the user sees it in the real app before the merge. Write a plan first and show it to the user.

## 2. Put the question list to the user

`docs/audit/2026-10-29-audit-of-audits/questions.md` holds 17 questions in seven groups. Q9 is withdrawn, because D76 already ruled it. Group 1 (Q1 to Q4, out-of-range settings) went to the user at the end of this session, with no answer yet. Groups 2 to 7 have not been sent:

- Group 2: how Hydra reads charts and song files (Q5 to Q7).
- Group 3: search and lists (Q8).
- Group 4: wording and small behaviour differences (Q10 to Q14).
- Group 5: numbers you confirmed without seeing them (Q15).
- Group 6: rare cases (Q16).
- Group 7: hooks (Q17 and Q18).

Send one group at a time, at most five questions each (the method page, stage 7). Before sending a group, re-read the code each question quotes. It must still say the same thing on the day it goes out, because a question once described code that had already changed. Record each answer as a D-record that names both sides and quotes the question.

Q7 (whether one authored fill stops Hydra's generated fills) recommends a live Clone Hero check. If the user agrees, that is a separate task using the `ch` MCP server.

## 3. Correct the records

`summary.md` lists them under "Records to correct". None needs a question. The biggest:

- D53 item 4 says the Preview's xN disc stays doubled after Star Power ends. The code has dropped it at the SP end since 4983e1f1, as D10 and D11 ruled.
- The 3 ms leeway's origin is wrong in three places. `CONTEXT.md` (Backend leeway) calls it "Hydra's own setting". `docs/audit/ch-evidence.md` finding 304 calls it "the user's decision". `docs/differences-from-public-hydra.md` lists it among "Hydra Deluxe's own choices". It is DragonDelgar's (ruling 4). The differences page is a public page, so its edit goes through the doc-review gate.
- ADR 0023 lines 92-93 still list the late meter walk, which is gone. ADR 0008 lines 62-63 still give the 59.5 s minutes switch, which is gone too.
- Q15's tuning numbers are cited as "D48, Q33" in `CONTEXT.md`, ADR 0008 and ADR 0019. Once the user answers Q15, those cites should point at the new record.
- The 2026-09-24 plan decisions and the 2026-09-26 answers about the 09-25 audit exist only in session cards (`2026-09-24-da599242`, `2026-09-26-4908325e`). They have no D-record.

Two blocks (`AOA-S.md`, `AOA-W2b.md`) say the media guard's branch was "never merged". It was merged (hooks repo 1380a37, 2026-10-10). The reviewer fixed this in `summary.md` and `questions.md` but left the raw blocks as written.

## 4. Open, by the audit's stop rule

`summary.md` lists them under "Open, by the stop rule": second claims inside concept-audit items that nobody reached, D114 items 17 and 18 against D255, three concept side notes, and five weak PASSes. Take them only if the user asks. The audit stopped after two waves on purpose.

## Where things are

- Results: `docs/audit/2026-10-29-audit-of-audits/`. That folder holds `summary.md` and `questions.md`, plus `blocks/` with each scout's raw verdicts and the two designs, and `briefs/` with the common scout brief and the `packets.py` and `checkquotes.py` scripts.
- The shared notes for this group of sessions: `C:\Users\Patrick\AppData\Roaming\Claude\side-session-notes\local_26a8b905-e48f-4371-a207-b9f334f133eb\audit-of-audits-ruling-backend-rows.md` holds the five rulings above.
- Memory: `never-keep-a-split-quietly`, plus the new top rule `sp-end-is-one-point-leeway-is-delgars`.
- `main` is at dfcf1cd1 and is not pushed. The worktree `fervent-bohr-838562` sits on branch `audit-of-audits` at dfcf1cd1, plus this handoff's commit.
