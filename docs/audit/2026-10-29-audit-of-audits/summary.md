# Audit of the past audits: summary (2026-10-29, session 61cee421)

This audit checked how every past Hydra audit closed its findings. The question for each closure was simple: did the user ever rule on exactly this, in words that name both sides? It ran by `docs/agents/audit-method.md` and the handoff `docs/handoffs/2026-10-29-audit-of-audits-handoff.md` (removed; `git show 444631e6:docs/handoffs/2026-10-29-audit-of-audits-handoff.md`).

Twelve read-only scouts did the checking, nine in a first wave and three in a second. One planner designed the single Backend timings rating rule the user asked for. Every scout's blocks are in `blocks/`, and the common brief and the two helper scripts are in `briefs/`. The main session checked every PASS quote with `briefs/checkquotes.py`. That script looks for each quoted span in the repo's docs, the code, the session cards and the hooks folder. All of them were found, except three, which the main session settled by hand (see "What the main session changed").

## Counts

Each closure gets one final verdict. Where a second-wave scout re-checked an item, its verdict replaces the first one. Where the main session overrode a scout, the override counts. The counts come from the blocks, by the tally run on 2026-10-29.

| Audit | Closures checked | PASS | REOPEN | GONE |
|---|---|---|---|---|
| 2026-10-03 derivation audit (every finding that carries a decision, plus the eight rows not marked fixed) | 134 | 85 | 13 | 36 |
| 2026-10-11 concept audit (every closure without a question, including the code-only list) | 64 | 13 | 9 | 42 |
| Smaller audits (09-24, 09-25, phase 6 recheck, test audits, hook audits) | 21 | 10 | 2 | 9 |

The smaller audits' two REOPENs are the 09-24 audit's finding 34 (the same split as M39) and the media guard. The hook design audit's shadow mode never got a verdict line, because nobody ever answered it; it goes on the question list as well.

PASS means the user's own words cover the closure. REOPEN means they don't, so the item becomes a question. GONE means today's code no longer has the problem, and the block names the commit.

The 3 ms against 10 ms Backend timings split shows up four times: as M39, findings 306 and 33, and the 09-24 audit's finding 34. Those count as one question.

## Where closures leaked

Two paths account for nearly every REOPEN.

The first is a question that showed less than the code does. The user answered "all recommended" to a batch, but a question's text named only one side of the split, or named a topic without its numbers. Findings 306 and 33 asked about the 10 ms edge and never mentioned the 3 ms one. Finding 66 asked about one row of a table that had three. Findings 327, R7.32, R7.34 and R7.36 were "confirmed" in phase 3-5 question 33, which named the topic but showed none of the numbers. That is the M39 story again, nine more times.

The second is a closure written by the agent that found the problem, with a word standing in for a ruling. The concept audit's "Already decided, no question" section and its "code-only" list let M04, M09, M25, M39, M46 and M64 out with "by design", "kept", "stays" or "left alone" and no quoted ruling. The 2026-10-25 hook rebuild dropped the media guard the user had ruled on (D189), because the rebuild plan's list of what it dropped never mentioned the guard.

A third path showed up once, but it matters. A question described code that had already changed. Phase 6 question 4 (2026-10-04) told the user the Preview's xN disc stays doubled after Star Power ends. Commit 4983e1f1 had already made it drop at the SP end on 2026-10-03, as D10 and D11 had ruled. The user answered "leave it" (D53 item 4) to a description of behaviour that no longer existed. The code is right, but the record is wrong.

Fixes that quietly picked a side were less common than feared. Of the GONE items the scouts flagged with "Side picked without a ruling", most did have a ruling once someone searched the session cards. The 09-24 plan decisions 3, 4, 5, 7, 9 and 13 were all asked on 2026-09-24 (card `2026-09-24-da599242`). Finding 53 is covered by the user's "why is 1.0 using its own frozen copy of the code? other than following a different rule it shouldnt be re-deriving anything" and "Yes. Move the 1.0 generator onto MeasureIndex/MeasureCount, keep only its own fill rule, and show me which test-corpus charts' 1.0 fills and scores move before merging." (card `2026-10-24-a124ccd9`). The ones still without a ruling change nothing the user sees, or touch only developer tools (38, 59, 84, 133, 317, 321). Finding 143, where the same chart in two folders is two library rows, was first put on the question list. Review took it off: you ruled it on 2026-10-05 (D76, D77, D79: "Count every copy"), and `questions.md` says where.

## What the main session changed

Finding 315 became a REOPEN. Its PASS quoted "one authored fill stops generation" as words of the D26 question, and the card has no such words. The real question asked only when to run a live Clone Hero check, and the user answered "Not now". The rule itself (`if (ts.has_activation()) return;` in `src/core/song.cpp`) has no ruling, and Clone Hero's own answer is unknown (`docs/audit/ch-evidence.md`).

Finding 53 became a PASS, on the two quotes above. Three scouts had marked it GONE with a side picked silently, because none of them read that card.

The xN disc (finding 1's second half, D53 item 4) moved from PASS to GONE: the code has dropped the disc at the SP end since 4983e1f1.

Finding 202 is GONE: `hydra_report` was deleted in 079afd77 (D103 item 5), so the app-against-tool folder pair no longer exists.

Findings 60 and 67 left the REOPEN list when the question drafter re-read them. Finding 60's code now gives blank artists and blank charters the same placeholder, as D50 item 5 and D266 call 2 ruled. Finding 67 is covered by D56 item 1, which names both the report pages' plain search and the Library's query language.

Two quotes in the smaller-audits blocks used single quotes where the source has double quotes. The words are in the cited files, so they count as found.

## The user's ruling in this session

Asked "should every Backend timings row use one rating rule, with the same labels and the same edges, whether or not the row is squeezed out?", the user said: "yes everything should use one rule." A first design (`blocks/AOA-R1-design.md`) rated every row by its raw offset from the SP end. The user rejected it: "what really matters here is the effective timing for the player, not the absolute offset. hydra already shows this with the badges next to each activation. -100 for a backend and a squeeze out are not the same thing". A second design rates the effective timing each row asks of the player (`blocks/AOA-R2-design.md`). The first design also found a second split: the badge and the squeeze sentence put the SP end at 0 ms, while the engine's counted test and the "(uncounted)" tag put it at the leeway.

## Wrong closures, one line each

The derivation audit: 139 (a Star Power cap of 0 reads three ways), 219 (the 6 s status fade was never asked), 306 and 33 (the 3 ms against 10 ms band), 315 (one authored fill stops fill generation), 322 (a hit window below 1 goes to 85, not to 1), 325 (the rules file's lower bounds were never shown), 327, R7.32, R7.34 and R7.36 (numbers confirmed unseen), 61 (.sng and .srb find their notes file differently), 66 (two INI readers).

The concept audit: M04 (two empty-search sentences and two hints), M09 (the comparison window keeps its rows on close), M17's cap half (Refresh builds a comparison the toolbar refuses), M25 (two spellings of "unknown option"), M33 part B (the tick test against the ms test at an absurd tempo), M39 (the Backend timings band), M46 (the " · " separator's spacing), M64 (which files count as song stems), M76's side claim (two time-left rules).

The smaller audits: the 09-24 audit's finding 34 (the M39 band again) and the media guard (D189), lost in the hook rebuild. The hook design audit's shadow mode was never answered.

## Records to correct

Each of these is a false statement about today's code or rulings, or dead code with a comment. None needs a question.

- D53 item 4 says the xN disc stays doubled after SP ends and was never checked on video. D10 says it was checked, and the code drops it (4983e1f1).
- ADR 0023 lines 92-93 still list the late meter walk as live. Commits b57a2155 and 5677f8d3 removed it.
- ADR 0008 lines 62-63 say the loading bar switches to minutes at 59.5 s. Commit bcf5663f replaced that with "about m:ss left".
- The `auto_open_report` comment in `src/app/config.h` says the setting covers the leaderboard comparison. D103 item 13 made it cover only the path report.
- CONTEXT.md lines 735-737 say the quarter-note beat rules match Clone Hero's code. ADR 0023 and `ch-evidence.md` say the 6/8 beat count is unknown.
- A comment in `src/core/model.h` and phase 7 question 4 say D13 decided the squeeze-out-at-0 case. D13 covers only a squeeze-in at 0.
- The 2026-09-24 plan decisions and the 2026-09-26 answers about the 09-25 audit's section 5 exist only in session cards `2026-09-24-da599242` and `2026-09-26-4908325e`. They have no D-record.
- `last_note_start_ms` in `src/app/song_length.h` has no caller in `src` or `tools`. Only one test (`tests/test_preview_scene.cpp:91`) calls it. Its comment says it agrees with the Preview's `last_note_ms`, and that test is the only thing that pins it.

## Open, by the stop rule

The audit stopped after two waves of scouts. These were not reached, or need a live run, and stay open with where to look:

- Second claims inside concept-audit items the first wave closed: M03 and M05's fill-page claims (V1.md:85, 172), M06's `posted` sink (V1.md:201), M07's colour tokens (V1.md:235), M17's "is this SP cap Clone Hero's" check (V2.md:161), M21's blank `sig` (V3.md:29), M23's blank charter (V3.md:70), V4's Ch10 sentence (V4.md:174), M72 and M73's junk policy (V8.md:151), M74's stored-key byte (V8.md:195), and M77 and M81 to M83 (V9). The "Side notes" list at the end of each V file was not read.
- D114 items 17 and 18 answered questions about the Preview's playback range, which D255 item 5 later replaced. Those two answers need a re-check against D255.
- The concept audit's side note about `note_shuffle`'s `prev_tick = 0` needs a live Clone Hero run.
- The concept side notes about `Playhead::read_frames` (the class is gone) and the audio headers' "throws std::runtime_error" wording (checked only for `device.h`).
- Weak PASSes, read from the finding's own tables and not re-checked in code: M16, M34, M36, M37 and M48.

## Files here

`blocks/AOA-*.md` holds each scout's blocks. A1 and A2 cover the derivation audit's assumptions, D1 to D3 its drifts and double readings, and U its duplicates and its rows not marked fixed. C1 and C2 cover the concept audit, and S the smaller audits. W2a to W2c are the second wave, and R1 is the rating-rule design. `briefs/common.md` is the brief every scout read. `questions.md` is the question list for the user.
