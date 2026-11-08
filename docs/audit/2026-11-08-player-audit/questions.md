# Player audit: questions for the user (S6)

Session 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e, 2026-11-08. Each question names the ledger rows it settles. Questions go out five or fewer at a time. The code was read on 2026-11-08 at main `fd93c684`.

## Batch 1: what gets pressed, and whether the run is playing what it thinks

### Q1. Refuse a built agent or exe that does not match its source (L01, L36 to L40)

The game runs the built `tools/ch_mcp/agent/*.js` files, not the `.ts` sources. The loaders' only test is whether the file exists: `if not os.path.isfile(path)` (`frida_tools.py:377`; the same test in `bot_run.py:631` and `trace_hits.py:300`). The kit's built file is from 2026-11-06 21:31, and its source changed on 2026-11-07 (commit `2709008f`). The tests run the `.ts` in Node, never the `.js` (`test_kit_tools.py:1247-1271`). Nothing records which build a run used. The same gap holds for `hydra_replay.exe`: the main checkout's exe is from 11-08 11:14, and `replay_schedule.cpp` changed at 12:26.

What it cost: LS-M2 lost 14 notes, and with them the extension of activation 2 (`ls-m2-run1`: 3,753 of 3,767 notes hit). Freak run 1 lost 4 of its 8 misses the same way.

What you would see: today a stale agent attaches with no message. After the fix, `ch_attach`, `ch_run_chart` and `ch_play_chart` would refuse with a line naming the stale file and the command that rebuilds it. Each run folder would also record which build ran.

- (a) The loaders refuse a built agent whose source is newer or differs, by a stamp the build writes. The run folder records the stamp and the `hydra_replay` build. The tests run the built file too.
- (b) Keep the loaders as they are, and add a build-all step before each live run.

Recommended: (a). A build-all step is a habit, and the last session skipped one. The case against (a): it adds a stamp file to the build, and a refusal can block a run while the rebuild is a minute away.

### Q2. The player refuses a reply or file that is missing what it needs (L01 player side, L14, L65)

When a split chord's reply has no `groups` list, the player keeps the first reply only and goes on: `if replies and later and isinstance(pressed.get("groups"), list):` (`play_chart.py:1166`). The later rows then get no reply values, and no line says so. The same "accept and go on" holds for score files: a missing `path_source`, `activations`, `tick` or `multiplier` changes what is pressed with no message (`play_chart.py:683-686`, `:761`, `:782-783`, `:790-791`). The tests' fake replies are `{}` or `None`, so no test catches a missing `groups` (`test_play_chart.py:505, 584, 1482-1496`).

What you would see: today LS-M2 printed nothing about the 12 chords whose second part never went down. After the fix, the run would stop at chord 3 (the first split chord) with a line naming the missing `groups`.

- (a) Stop the run on a reply that lacks what was asked for, and refuse a score file that lacks a key the play depends on. Make the test fakes send the real reply shape.
- (b) Keep going, but print one line per chord that lost its reply.

Recommended: (a). A run that has already lost notes cannot reach Hydra's score, so going on proves nothing. The case against: a stop ends a long run that might still have shown other faults.

### Q3. The three ways of leaving gems unpressed on a fill the path passes over (L08, L59)

When Hydra's path lets a fill pass, the game would activate Star Power if the fill's last chord were hit in full. The player has three modes for that chord (`play_chart.py:123-126`, `_kept_gems` at `:709-728`):

- `all_but_top_pad` presses every gem but the highest pad (the gem that activates, FL-09).
- `kick_only` presses only the first kick.
- `nothing` presses nothing.

With no mode given, every gem is pressed, and the game activates where Hydra's path does not.

What the game does with the gem left out: LS-M2 left one pad out at each of nine chords (239, 800, 827, 1004, 1029, 1328, 1617, 1662, 1989). The game credited every one as hit (`ls-m2-run1` score steps 460 or 600 each, equal to Hydra's), with 9 activations, the same as the plan. So `all_but_top_pad` cost no note and no streak on two songs (Senri chord 405 was the first). `kick_only` and `nothing` leave more gems out, and no run has tested them.

What the user said: the modes were approved inside two batch answers, "Yes, take all seven" (`docs/sessions/2026-11-07-5b92c813.md:21`) and "yes all create handoff" (`docs/superpowers/plans/2026-11-07-player-refinements.md:111`). Neither names `kick_only` or `nothing`. Later you said "you cant just choose to not hit notes!". That was about a wrong diagnosis of LS-M2's misses, but no ruling since says which mode, if any, you want.

- (a) Keep only `all_but_top_pad`, and remove `kick_only` and `nothing`.
- (b) Keep all three as they are.
- (c) Remove declining altogether, so every gem is pressed and the game activates at every fill. The player could then not follow any path that passes a fill.

Recommended: (a). It is the one mode two runs proved costs nothing, and the others were never chosen in your words. The case against: a fill that ends on a kick alone has no pad, so `all_but_top_pad` sends nothing there (`_kept_gems` returns an empty list when no pad gem exists). Whether the game credits that kick is unproven, and `kick_only` was the mode that would have pressed it.

### Q4. Which gem of a split chord goes first (L09, L10, L19)

Some chords are sent in two parts, 8 ms apart, so the game scores the gems in a set order (`play_chart.py:488-527`, `CHORD_ORDER_GAP_MS = 8` at `:113`). At an activation chord, the player sends the activating cymbal first and the kick 8 ms later. A session card holds your words: "in both cases you have to hit the kick first and cymbals are interchangeable so t doesnt matter which is hit first" (`docs/sessions/2026-09-29-56ac5b71.md:17`). The page block SC-38 rests on one run, and says which gem activates "was not read in the code". The 8 ms gap and "Only where it matters" are your words (`player-refinements.md:204`).

Two smaller facts sit beside it. The 8 ms is made three ways: the C++ plan, the kit's 2 ms tick, and a `time.sleep` on the keys route. A frame longer than 8 ms would put both parts in one frame. And a squeezed-out chord is sent as two messages 30 ms apart, a number with no named source (chord 1893 in `ls-m2-run1`).

No run has yet sent a later part with the current kit agent, so whether the order works is unproven.

- (a) Read the game's code for which gem activates and in what order a chord's gems score, before any change.
- (b) Switch the activation chord to kick first, cymbal 8 ms later, per the card.
- (c) Keep the order as it is.

Recommended: (a). Your card's words were about a squeeze, and this is an activation chord, so the card may not apply. The game's code settles it. The case against: it delays the rerun of LS-M2 by one code read.

### Q5. Check that the run plays the song, path and settings its files are for (L16, L17, L20, L21)

Nothing compares the score file with the song the game has loaded. In `game_tools.py:1026-1109`, the song folder is used only for the run cap (`:1055`) and the watch (`:1075`). A score for one song played into another would run and report hits. The judging scripts never read the stats file's `checksum`, `playback_speed`, `no_fail` or `modifiers` (`compare_scorestats.py:81-134`). `ch_open_song` can end with "done: a song is loaded, but its folder is X, not the target Y" as an ordinary reply (`menu_tools.py:285-288`). Its first two steps check words that already show before any press (IN-79), and no step checks the instrument or difficulty. `ch_run_chart` always uses dump path 0. It refuses a mismatched score that records its path, but accepts one with no `path_source` (`play_chart.py:683-686`).

What you would see: today a wrong song, speed or modifier runs and is judged as if right. After the fix, the run refuses before the first press, and the judge fails on a mismatched checksum or speed.

- (a) Refuse to start unless the loaded song's folder matches the score's chart. Have the judge compare checksum, speed and modifiers. Make `ch_open_song`'s wrong song a refusal, check instrument and difficulty, and give `ch_run_chart` a `path_index`.
- (b) Only add the song-folder check before the first press.

Recommended: (a). Each piece closes a way a run can be judged against the wrong thing. The case against: (a) touches four tools at once, and `ch_open_song` has no live proof yet (LS-K3 was struck), so its new checks need a live run.
