# Plan the automatic run that matches Hydra's optimal score (2026-11-06, session 70bfa33d)

## The goal

The user ruled that the work is not finished until an automatic Clone Hero run reproduces Hydra's optimal score. The next session writes that plan. It builds nothing and launches nothing until the user says yes. The first target is the Senri chart, `C:\Clone Hero\songs\Hydra Probe\Ladies Talk fill check`, played as `ProDrums,Expert` through the `ch` server's virtual kit. A pass means the game's own stats file reads `score` 1,229,730.

## What happened in this session

The session continued `docs/handoffs/2026-10-31-ls-d1a-passed-handoff.md`, which is only on the unmerged branch `claude/handoff-ls-d1a-passed`. Every claim in it checked out.

**Every direct-start live step up to LS-D3 passed, each on its first run.** LS-D1b to LS-D1d proved a kit press on System reaches the drums input, with a traced hit 8 ms from note 132. LS-D2a proved the direct start as `ProDrums,Expert`. LS-D2b and LS-D2c proved the Star Power watch on a human launch. LS-D2d to LS-D3 played the whole Senri chart: 2,342 presses, 2,342 hits, `is_bot` false, 25 phrases, 9 activations, and the compare passed. Rows P22, P30, P32, P35 and P40 are checked. The user ruled that `KIT_PLAYER` stays `Player1`, so a human launch names `System` in `ch_kit_setup`.

**The run's score was far below Hydra's, and the gap is now explained to the point.** The game scored 900,870. The path with those 9 fills scores 1,226,170 in Hydra, when the Star Power ends are the "flag cleared" times and not the "end time" `ch_sp_watch_status` prints. A phrase landing during an activation extends it, which is why the printed time is too early. The gap of 325,300 is four things: 320,415 for accents and ghosts, 2,245 for chords 0 to 6 and the late multiplier ramp, 1,980 for the first solo at 76 of 83 notes, and 660 for chords 2044 and 2320. A gem-by-gem rebuild from chord 7 with no dynamics matches the game's running score from the watch CSV on every chord but those two.

**Why no dynamics registered.** The helper at RVA 0x337E10 gives each lane's velocity byte. It reads a real velocity only when the player's slot is in MIDIInputManager's list, or when the player holds custom controller id 6. The kit is controller 0 and no MIDI kit exists, so every hit carried -1, and -1 marks neither accent nor ghost. The velocity is round((GetAxis - 0.5) * 2 * 127). A real MIDI kit is a custom controller from definition 1, with velocity v written as axis 0.5 + v / 254. The kit is definition 6. Pages 04 and 06 and `docs/clone-hero/code/6142/lane_velocity_337E10.md` hold this.

**Why the 11 notes were lost.** They are the gems of chords 0 to 6. The song starts by itself, the first note is at 4.14 s, and attach, kit setup, the watch and the player ran as separate tool calls. The first press was chord 7 at 5.40 s. Every press sent hit.

## What the three read-only scouts found

Read these as leads, not facts: the main session checked the items marked checked, and the next session checks the rest against the code.

**The target path.** `hydra_replay dump` on that chart gives the top path, "0 1+ 0 0 0 0 0 0", 1,229,730, with 8 activations, ticks act:deact: 26880:36960, 61200:74640, 95520:107040, 126240:132960, 193200:210000, 226800:233520, 247920:259440, 284400:294960. Checked by this session: `score --path` on it gives 1,229,730. It declines fill chord 405 at tick 54480 and needs no squeeze timing; its one squeeze-in at -103 ms is free. All 8 ends sit exactly on a chord. The best path with no skip is 1,226,170, the 9 fills the game took.

**Dynamics on this chart (checked).** 1,094 of 2,996 gems: 368 accents and 726 ghosts. All 631 kick gems are plain, so the kit's button kick does not matter here. They are worth about 322,000 points on the top path. Declined chord 405 is `n...N` with no dynamic.

**Ways past the gate, best first.** Controller id 6 through Rewired's id counter, if it never reuses ids: create and destroy six throwaway controllers, unproven, one short launch settles it. Then adding the kit to MIDIInputManager's list with a hand-made entry whose +0x48 byte is 0 and whose +0x30 holds a real controller. The user was told a virtual MIDI driver is ruled out as a machine install; that is the session's recommendation, not a ruling. Hooking the helper is the last resort. Unknown: whether the id is compared as the scout infers, how Rewired numbers controllers, and where the threshold at engine +0x2C0 lives. It can be read live. A ghost needs a value at or above 0 and below the threshold, and an accent one at or above it, so axis 1.0 serves accents. Whether a very low axis still reads as a press is unproven.

**Declining a fill (checked as a reading, not run).** The miss handler's grant jumps through slot +0x1E8, which is slot 11 at 16 bytes a slot, the plain hit handler RVA `0x20DE770`, not the press handler that activates. So not pressing should score a hit and not activate. FL-31 on page 04 holds this. Which gem of a kick-plus-cymbal chord carries the flag is unknown, and a live test must find it.

**Press accuracy and start.** Presses average 2.4 ms early, 97.9% within 5 ms, no drift, 327 fps, the worst 29.7 ms late, inside the hit window. A chained tool that shares one engine search could press from about 1 s; `first_note_index` also skips anything within 150 ms of the clock. The paused launch does not hold the song clock.

**Star Power end chords.** Hydra pays a chord exactly on the end as inside (SP-33, never checked), and the game paid 2044 and 2320 outside. The session's idea, unproven: press those chords a few ms early, and aim activation presses exactly, because an early activation moves the end (SP-39).

## What is next

1. Run the checker, read every claim, and check the scouts' leads the plan rests on.
2. Dispatch the `planner` agent on Fable to write the plan, with a Premise map as `docs/agents/premise-map.md` says, and a `Calls:` line on every live step. Search the web for prior art on Rewired id numbering (memory `search-web-before-finalizing-plans`).
3. The approach the user was shown, in order: one short probe launch (can the kit reach id 6, does a ghost-strength press count as a hit, does a dynamic register); then in parallel a chained start, per-gem strength from each gem's `dynamic`, fill declining, and an exact score check in `compare_scorestats.py`; then full runs on Senri against 1,229,730.
4. The plan must fold in LS-K4 and LS-K5, which were left open. The user was asked whether to accept LS-K5 from the LS-D2d run and has not answered: `ch_sp_between(114778.9, 166544.0)` read no activation inside, ready at 134,182.1 ms, full at 14,400. LS-K4's "142 hits" can only be reached once the start is fixed, because 13 notes fall before 5.4 s.
5. Each live run needs the user's go, with its `Calls:` line shown and its unchecked rows named. Take the DLL's MD5 before and after. Copy the watch CSV and stats out before `ch_stop`.

## Open points

- **Not covered by this approach.** A chart whose first note is under about 1.2 s, one with kick dynamics (the kit's kick is a button; definition 1 has an axis kick, map unproven), and one whose optimal path needs real squeeze timing. The user was told these come after Senri matches.
- **Label trap.** `ch_sp_watch_status` prints "end time" beside "flag cleared"; only the second is when Star Power ended. Not fixed.
- **Stale comment.** The comment above `KIT_PLAYER` in `tools/ch_mcp/kit_tools.py` still says the live step decides whether it becomes System. It waits for the next code change.
- **`ch_status` shows the last run's trace.** Both replies in LS-D2a printed "trace: ended: stopped, 1 calls" with no trace running.
- **Focus grabs.** A second grab came at 48.9 s in LS-D2d, cause unknown; `foreground.py` keeps the handles but prints only the times.
- **`note_score` does not map to Hydra's `notes`.** 162,390 against 168,235; the total matches anyway. SC-32 is Unknown.
- **`senri-acts.json` in the run folder is wrong.** Its ends are too short. `senri-acts-true-ends.json` is the right one.
- **Pushing.** `main` is 15 commits ahead of `origin/main`, some from other sessions. Nothing here was pushed.

## Claims

| Kind | Value | Claim | How to check |
|---|---|---|---|
| commit | `d277877302af2f016ebd21492b8ee8503ab75a11` | pages: LS-D1b to LS-D1d pass; on `main`, not pushed | `git branch --contains`; `git log origin/main` |
| commit | `caf69cd991818968682c8019e60ec0ea359bcb82` | `KIT_PLAYER` stays `Player1` recorded; on `main`, not pushed | as above |
| commit | `30207a3c8402f087dbc07f75015ae033286a371e` | IN-74: LS-D2a passes; on `main`, not pushed | as above |
| commit | `e9f26f2a9c6b822bd2992d6bdeb922ce87f737c5` | pages 05, 06, 07: LS-D2d to LS-D3 pass; on `main`, not pushed | as above |
| commit | `5ac1efbb3a076dde7ebde6f983a7556001ca08f2` | SC-14: the run rebuilds with no dynamics credit; on `main`, not pushed | as above |
| commit | `53c2171e56506264beddeda6de7802ba2e639261` | pages 04 and 06 and the lane velocity listing; `main`'s tip when this was written; not pushed | as above |
| branch | `claude/handoff-ls-d1a-passed` | holds the handoff this session continued; not merged, superseded by this one | `git log main..claude/handoff-ls-d1a-passed` |
| path | `docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md` | its LS-K4 and LS-K5 are the steps left open | the checker finds the file |
| path | `docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md` | holds LS-K4's pass line | the checker finds the file |
| path | `docs/clone-hero/code/6142/lane_velocity_337E10.md` | explains the velocity gate | the checker finds the file |
| path | `docs/clone-hero/code/6142/lane_velocity_337E10.asm` | the gate's listing | the checker finds the file |
| path | `docs/clone-hero/06-scoring.md` | SC-13 and SC-14 hold the gate and the run's result | the checker finds the file |
| path | `docs/clone-hero/04-fills.md` | FL-31 holds where a missed fill note's grant goes | the checker finds the file |
| path | `docs/agents/premise-map.md` | the shape of the plan's Premise map and `Calls:` lines | the checker finds the file |
| path | `tools/ch_probe/experiments/play_chart.py` | `load_dump_chords` reads only index, ms and each note's color and cymbal | the checker finds the file |
| path | `tools/ch_mcp/compare_scorestats.py` | passes or fails only note and phrase counts | the checker finds the file |
| path | `tools/ch_mcp/kit_tools.py` | `KIT_AXIS_VALUE` is 1.0 and `KIT_PLAYER` is `Player1` | the checker finds the file |
| path | `tools/ch_mcp/agent/kit.ts` | creates the kit and writes each pad's value | the checker finds the file |
| path | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-run1\replies.txt` | the full run, with the gap's breakdown | the checker finds the file |
| path | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-run1\senri-acts-true-ends.json` | Hydra's score of the 9 fills with true ends, 1,226,170 | the checker finds the file |
| path | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-run1\sp_watch-2.csv` | the watch's rows, with the game's running score | the checker finds the file |
| path | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d1b-run1\replies.txt` | LS-D1b to LS-D1d, with where each press landed | the checker finds the file |
| path | `C:\Clone Hero\songs\Hydra Probe\Ladies Talk fill check` | the Senri chart folder | the checker finds the file |
| function | `load_dump_chords()` | reads a score JSON's chords, in `tools/ch_probe/experiments/play_chart.py` | `git grep` |
| function | `first_note_index()` | skips notes close to the clock at the start, in `tools/ch_probe/experiments/live.py` | `git grep` |
| test | `py -m pytest tools/ch_mcp/tests/test_frida_attach.py tools/ch_mcp/tests/test_kit_tools.py -q` | `135 passed, 98 subtests passed` on `main` at `53c2171e56506264beddeda6de7802ba2e639261` | run it from the main checkout |
| test | `build-cpp\Release\hydra_tests.exe "-tc=docs match code*"` | `[doctest] assertions: 546 \| 546 passed \| 0 failed \|` on `main` at `53c2171e56506264beddeda6de7802ba2e639261` | run it from the main checkout |
| ruling | `70bfa33d` | "our work is not finished until you can replicate hydra's optimal score in an automatic run. now that you know the gaps, you need to figure out how we can accomplish that." | this session's card |
| ruling | `70bfa33d` | "Keep Player1" | this session's card; `docs/agents/rulings/clone-hero.md` |
| ruling | `70bfa33d` | "where are you getting 1170 from? why were so many points missed? this needs to be explained before we go any further" | this session's card |
| ruling | `70bfa33d` | "ok but why were notes mised? why did no dynamics register? this is a huge discrepancy and youre still avoiding it" | this session's card |
| ruling | `70bfa33d` | "Go, LS-D1b", "Go, LS-D2a", "Go, LS-D2b", "Go, LS-D2d" | this session's card |
| ruling | `70bfa33d` | "create a handoff to plan" | this session's card |
| state | the game DLL's MD5 is `106E15E5BF799EA62F916E8CEAF38BC6` | the same before and after every run of this session | `Get-FileHash -Algorithm MD5` on `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll` |
| state | `hydra_replay dump` on the Senri chart's `notes.mid` | the top path scores 1,229,730 with the 8 activations above | run it, then `score --path <dump> --index 0 --prodrums 1 --bass2x 1` |
| state | no plan for this goal exists and nothing is built | the user has not approved a build | `git log main --oneline -5` |
| state | LS-K4 and LS-K5 have not run; the LS-K5 question to the user is unanswered | no `ls-k4` or `ls-k5` folder exists | `Get-ChildItem C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06` |
| state | nothing this session committed is pushed | `origin/main` is at `51319e88f68d3681012ea91a2a881da855d88486` | `git log origin/main --oneline -1` |

## Checker output

```text
not-pushed | commit | d277877302af2f016ebd21492b8ee8503ab75a11 | pages: LS-D1b to LS-D1d pass; on `main`, not pushed
not-pushed | commit | caf69cd991818968682c8019e60ec0ea359bcb82 | `KIT_PLAYER` stays `Player1` recorded; on `main`, not pushed
not-pushed | commit | 30207a3c8402f087dbc07f75015ae033286a371e | IN-74: LS-D2a passes; on `main`, not pushed
not-pushed | commit | e9f26f2a9c6b822bd2992d6bdeb922ce87f737c5 | pages 05, 06, 07: LS-D2d to LS-D3 pass; on `main`, not pushed
not-pushed | commit | 5ac1efbb3a076dde7ebde6f983a7556001ca08f2 | SC-14: the run rebuilds with no dynamics credit; on `main`, not pushed
not-pushed | commit | 53c2171e56506264beddeda6de7802ba2e639261 | pages 04 and 06 and the lane velocity listing; `main`'s tip when this was written; not pushed
ok | branch | claude/handoff-ls-d1a-passed | holds the handoff this session continued; not merged, superseded by this one
ok | path | docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md | its LS-K4 and LS-K5 are the steps left open
ok | path | docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md | holds LS-K4's pass line
ok | path | docs/clone-hero/code/6142/lane_velocity_337E10.md | explains the velocity gate
ok | path | docs/clone-hero/code/6142/lane_velocity_337E10.asm | the gate's listing
ok | path | docs/clone-hero/06-scoring.md | SC-13 and SC-14 hold the gate and the run's result
ok | path | docs/clone-hero/04-fills.md | FL-31 holds where a missed fill note's grant goes
ok | path | docs/agents/premise-map.md | the shape of the plan's Premise map and `Calls:` lines
ok | path | tools/ch_probe/experiments/play_chart.py | `load_dump_chords` reads only index, ms and each note's color and cymbal
ok | path | tools/ch_mcp/compare_scorestats.py | passes or fails only note and phrase counts
ok | path | tools/ch_mcp/kit_tools.py | `KIT_AXIS_VALUE` is 1.0 and `KIT_PLAYER` is `Player1`
ok | path | tools/ch_mcp/agent/kit.ts | creates the kit and writes each pad's value
ok | path | C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-run1\replies.txt | the full run, with the gap's breakdown
ok | path | C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-run1\senri-acts-true-ends.json | Hydra's score of the 9 fills with true ends, 1,226,170
ok | path | C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-run1\sp_watch-2.csv | the watch's rows, with the game's running score
ok | path | C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d1b-run1\replies.txt | LS-D1b to LS-D1d, with where each press landed
ok | path | C:\Clone Hero\songs\Hydra Probe\Ladies Talk fill check | the Senri chart folder
ok | function | load_dump_chords() | reads a score JSON's chords, in `tools/ch_probe/experiments/play_chart.py`
ok | function | first_note_index() | skips notes close to the clock at the start, in `tools/ch_probe/experiments/live.py`
manual | test | py -m pytest tools/ch_mcp/tests/test_frida_attach.py tools/ch_mcp/tests/test_kit_tools.py -q | `135 passed, 98 subtests passed` on `main` at `53c2171e56506264beddeda6de7802ba2e639261`
manual | test | build-cpp\Release\hydra_tests.exe "-tc=docs match code*" | `[doctest] assertions: 546 \| 546 passed \| 0 failed \|` on `main` at `53c2171e56506264beddeda6de7802ba2e639261`
manual | ruling | 70bfa33d | "our work is not finished until you can replicate hydra's optimal score in an automatic run. now that you know the gaps, you need to figure out how we can accomplish that."
manual | ruling | 70bfa33d | "Keep Player1"
manual | ruling | 70bfa33d | "where are you getting 1170 from? why were so many points missed? this needs to be explained before we go any further"
manual | ruling | 70bfa33d | "ok but why were notes mised? why did no dynamics register? this is a huge discrepancy and youre still avoiding it"
manual | ruling | 70bfa33d | "Go, LS-D1b", "Go, LS-D2a", "Go, LS-D2b", "Go, LS-D2d"
manual | ruling | 70bfa33d | "create a handoff to plan"
manual | state | 106E15E5BF799EA62F916E8CEAF38BC6 | the same before and after every run of this session
manual | state | hydra_replay dump | the top path scores 1,229,730 with the 8 activations above
manual | state | no plan for this goal exists and nothing is built | the user has not approved a build
manual | state | LS-K4 and LS-K5 have not run; the LS-K5 question to the user is unanswered | no `ls-k4` or `ls-k5` folder exists
manual | state | nothing this session committed is pushed | `origin/main` is at `51319e88f68d3681012ea91a2a881da855d88486`
checked 38 rows: 19 ok, 0 missing, 0 not-on-main, 6 not-pushed, 13 manual
```
