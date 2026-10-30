# AOA-R1: one rating rule for every Backend timings row

Design only. Nothing here is built. The user ruled on 2026-10-29: "yes everything should use one rule." Today's two ladders came from public Hydra commit c9c00cf7 (2025-08-12); nobody chose them.

## 1. What the offset means, and what the path needs from each row

A backend row is one chord near the end of an activation's Star Power. Its offset is how many ms the chord sits from the SP end: negative is before the end (inside SP), positive is after. The engine pays a chord under SP when it is at or before the end, or less than the leeway past it (`core::counted_without_squeeze`, `src/core/backend_value.h:41`; the leeway is `Rules::backend_leeway_ms`, default 3 ms, and exactly 3.0 ms is not counted, D29). That one test prices every row in the search, the replay and the table (`backend_row_value`, `paid_by_sp`).

From a plain row the path needs nothing of the player beyond a normal hit. If the engine counts it, the score already includes it; a player who hits it late enough to land after the SP end loses those points. If the engine does not count it (offset at or past the leeway), the row shows 0 points and the stored score never assumed it; the player can still win it by hitting early so it lands inside SP, which is the backend squeeze the User Guide describes. The hit window (85 ms by default) is the most a hit can be early, so a row more than a window past the end cannot be won by an early hit.

From the squeezed-out row the path needs the opposite. The engine assumed the player hits this chord after SP ends so that its SP phrase is not eaten and banks for later. The row is paid `sqout_sp_points` when the engine counts it, and the gap between that and its full value is what the squeeze-out costs (`sqout_cost`; the table's "(-N)"). The player must hit it later than `-offset` ms (`SPSqueeze::timing`); a row already past the end needs no timing (`SPSqueeze::needs_no_timing`, `src/core/model.h:383`). That timing is shown in three other places already: the row badge ("squeeze out 163 ms"), the path's hardest ms, and the open row's sentence ("Hit the Green tom note more than 163 ms late..."). So the Rating cell does not have to carry it.

## 2. Every place that rates a backend note or a squeeze by its timing

Each line says what the place answers and whether the one rule should cover it. The main session asks the user which of these "everything" includes.

1. `BackendSqueeze::summarystr` (`src/core/model.cpp:339-361`): the Rating word for each table row; two ladders today. This is the ruling's target. Yes.
2. The " (uncounted)" tag, same function: does the engine count this row. Reads `counted_without_squeeze`. Yes, it is part of the row's label.
3. The " <-- squeezed out (-N)" suffix and orange colour (`src/app/path_view.cpp:360-376`): which row is the squeeze-out and what it costs. Reads the engine's price. Keep; it is a fact, not a rating.
4. `SPSqueeze::needs_no_timing` (`src/core/model.h:383`): is a SqIn or SqOut already done with no timing. It tests the SP end at 0 ms (`paid_by_sp_walk`). Split, see below.
5. `rate_activation`'s `inside` pick (`src/core/squeeze_rating.cpp:123-125`): which frontend scale side (early or late) a row's eff. figure reads. The squeezed-out row tests 0 ms; every plain row tests the leeway. Same split as 4, in a second place.
6. The open row's squeeze sentence (`src/app/path_view.cpp:106-130`): "no more than X early" or "more than X late" comes from item 4 (0 ms); "It scores N fewer points" comes from the leeway. Split, same as 4.
7. `is_frontend_decided` (`src/core/squeeze_rating.cpp:93`): does this row help fire the overfill warning. Any squeezed-out row, or a plain row at or past the leeway. Reads the engine's edge; no change needed, but it is a reader to re-check.
8. Row badges and hardest ms (`Activation::badge_timings`, `hardest`, `src/core/model.cpp:487-520`): a squeeze's figure and whether it is required, from items 4 and `SPSqueeze::difficulty`; orange past 2 ms (`kDifficultMs`, D48 Q3). A different question (how late or early the player must hit), with its own ruled edge. Probably not covered; ask.
9. Report timing tiers (`timing_tiers`, `src/core/squeeze_rating.cpp:175-186`; User Guide lines 478-483): a path's hardest timing banded at 2 ms, W/2, W, 1.5W, 2W as Normal, Hard, Extreme, Insane, Insane+, Beyond. A different question (one path's hardest frontend timing, not one note's place against the SP end), but it shares the hit window and the words Hard and Insane at other edges. D50 item 1 only reworded the footer to say the two differ. Ask whether "everything" reaches it.
10. The Preview's xN disc (`core::paid_by_sp`, D2) and the replay's SP points (`src/core/replay.cpp:180-190`): did SP pay this chord. These read the engine's own price. Already one rule; no change.
11. `hydra_replay` (`tools/replay.cpp`) and `replay_json.cpp`: print SqOut offsets and match typed ones; no rating word. No change.
12. `display_backends` and the "Hide backend rows beyond" setting (`kSqueezeWindowMs` 500 ms, `src/core/model.h:114`): which rows appear at all. A limit, not a rating. No change.
13. Text: User Guide lines 271-275, `CONTEXT.md` lines 664-680 (which say the Rating word and `needs_no_timing` are "a different test"), `docs/differences-from-public-hydra.md` version table. Yes, they follow the code.

**The split.** For "is this note past the SP end?" two edges are in use. `needs_no_timing`, the badge, the sentence's wording and the squeezed-out row's scale side say the end is at 0 ms. `counted_without_squeeze`, the points, the cost, the "(uncounted)" tag and every plain row's scale side say the end is at the leeway (3 ms). At +2 ms a squeezed-out row therefore reads "squeeze out -2 ms" (needs no timing) while the same row shows "(-N)" points lost because the engine still counts it. No ruling names both sides; D13 chose 0 for a SqIn only. This is a question for the user, separate from the ladder.

## 3. The one rule

Every row, squeezed out or not, gets its word from where the chord sits against the SP end, using only the two edges the engine already has: the hit window and the counted edge. Inside SP by more than a hit window: "Free". Counted by the engine (inside SP, or within the leeway): "Standard". Not counted, but within a hit window past the end: "Hard (uncounted)". A hit window or more past the end: "Insane (uncounted)". Said out loud: free, standard, hard, insane, by hit window and by whether the engine counts it.

This drops the 10 ms band (`kBackendInnerBandMs`). It is a number nobody chose, and the leeway already marks the edge the engine prices with. It drops the five "SqOut" words, because the squeeze-out's own difficulty lives in the badge and the sentence. The "(uncounted)" tag stays exactly where the engine pays 0 (user decision 19, D48 Q6).

## 4. Before and after, defaults (leeway 3 ms, window 85 ms)

| Offset | Plain row today | Squeezed-out row today | Both rows, proposed |
|---|---|---|---|
| -100 | Free | Insane SqOut | Free |
| -50 | Easy | Hard SqOut | Standard |
| -15 | Easy | Hard SqOut | Standard |
| -10 | Standard | Standard SqOut | Standard |
| -5 | Standard | Standard SqOut | Standard |
| 0 | Standard | Standard SqOut | Standard |
| +2 | Standard | Standard SqOut | Standard |
| +3 | Hard (uncounted) | Standard SqOut (uncounted) | Hard (uncounted) |
| +5 | Hard (uncounted) | Standard SqOut (uncounted) | Hard (uncounted) |
| +10 | Hard (uncounted) | Easy SqOut (uncounted) | Hard (uncounted) |
| +15 | Hard (uncounted) | Easy SqOut (uncounted) | Hard (uncounted) |
| +50 | Hard (uncounted) | Easy SqOut (uncounted) | Hard (uncounted) |
| +85 | Insane (uncounted) | Free SqOut (uncounted) | Insane (uncounted) |
| +100 | Insane (uncounted) | Free SqOut (uncounted) | Insane (uncounted) |

Today's edges, read from the code: plain Free below -85, Easy below -10, Standard while counted (below +3), Hard below +85, else Insane. Squeezed-out: Insane SqOut below -85, Hard SqOut below -10, Standard SqOut below +10, Easy SqOut below +85, else Free SqOut, with the tag from +3 on. Exactly -10, +3 and +85 fall on the higher rung in every ladder because each edge is "less than".

## 5. The strongest case against, and one alternative

The case against: a counted note a few ms inside SP reads "Standard" like a note 50 ms inside, so the word no longer warns that a slightly late hit drops it out. The Timing column still shows the ms, and the badge still names a squeeze-out's figure, but the word itself says less than "Easy" and "Standard" did. The second cost is that "Free" on a squeezed-out row at -100 ms names the chord's place, not the squeeze-out, which the "<-- squeezed out" suffix and the "squeeze out 100 ms" badge must carry.

The alternative single rule: keep today's plain ladder unchanged, 10 ms band included, and give the squeezed-out row the same five words. The one-line difference from the table above: -50 and -15 read "Easy" instead of "Standard", for both row kinds. It changes only the squeezed-out column and keeps the 10 ms, so the user would be choosing that number for the first time (D48 Q4 kept it without the split being raised).

## 6. What a build touches

The rule lives in `BackendSqueeze::summarystr` alone: its squeezed-out branch and `kBackendInnerBandMs` go, the `squeezed_out` parameter can go with them, and its header comment in `src/core/model.h` follows. The pinned labels in `tests/test_rules.cpp` (about lines 170-235), `tests/test_model.cpp:838-840`, `tests/test_squeeze_rating.cpp:814-828`, `tests/test_activation_rows.cpp:443-531` and the "Insane SqOut (eff. 163.5 ms)" lines in `tests/ui/uitest_paths.cpp` (376, 605, 618) are re-pinned from one run, with a boundary test at -85, +3 and +85 for both row kinds. The User Guide paragraph at line 275, `CONTEXT.md` lines 664-680 and the differences page's version table change; no stored record changes and no results stamp moves, because the word is computed at display time from the stored offset, so the docs-vs-code check and the single-owner scan are the whole-repo tests that run at merge.

## Promise map

Each promise this design makes, and the check that would prove it in a build. The split in item 2 and the "ask the user" items (8, 9) are out of scope for the build until the user rules; they are listed so nothing is closed quietly.

| Promise | Where above | Check in the build |
|---|---|---|
| Both row kinds give the same word at the same offset | Item 3, table in 4 | `summarystr` test: every offset in the table, squeezed out and not, same string |
| Edges are -W, the counted edge, +W; nothing at 10 ms | Item 3 | Boundary test at -85, +3, +85 for both kinds; `kBackendInnerBandMs` no longer exists (grep is empty) |
| "(uncounted)" appears exactly when the engine pays 0 | Item 3 | Test pairs `points == "0"` with the tag on every row in `test_activation_rows.cpp` |
| The leeway setting still moves the Standard/Hard edge | Item 1 | Existing `test_rules.cpp:178-179` style check with leeway 5 |
| No stored record or results stamp changes | Item 6 | `src/store/stored_versions.h` untouched in the diff; a stored path re-opens with the new word |
| Guide, CONTEXT.md and differences page match the code | Item 2 line 13, item 6 | `hydra_tests.exe "-tc=docs match code*"` and a reviewer read of line 275 |
| 0 ms vs leeway for "past the SP end" is asked, not fixed here | Item 2, "The split" | Out of scope: a question to the user, naming both sides |
| Report tiers and badge difficulty stay as they are unless the user says otherwise | Item 2 lines 8-9 | Out of scope: a question to the user |
