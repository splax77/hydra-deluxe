# Handoff: the concept audit's fix wave, mid wave 2 (2026-10-11)

**For the next session: the fix wave is approved and half done. Do not ask for a new yes to continue it.** The user approved the whole fix wave with "go with recommended" (recorded as D114 in `docs/audit/2026-10-03-fix-decisions.md`). The waves can be stopped between, and the user stopped this session after G5 to start a fresh one. The next step is in "Where to start" below.

The plan is `docs/audit/2026-10-11-concept-audit/fix-wave.md`. Its tables say which files each task owns and which tests it may run. Its sections under "The briefs" say what each task changes. The executor brief is `fix-brief.md`, and the reviewer brief is `fix-review-brief.md`, both in the same folder. Read the plan before dispatching anything. This note only records where things stand and the traps this session hit.

## Where things stand

Wave 1 is finished. All eight tasks, F1 to F8, are merged on main.

Wave 2 is mostly merged. G2, G2b, G3, G4, G5 and G6 are on main. G1 is reviewed but not merged: it waits on a real-window check. Both are described under "The last two tasks" and "Session end" below.

Main passed the full suite after the G4 merge: 1,340 of 1,340 unit test cases, plus 95 of 95 GUI tests. One unit test failed first. It was the docs-match-code test, which caught ADR 0011 still naming the deleted `activation_deact_tick()`. Commit `ff8bea56` fixed the ADR, and that test then passed.

Nothing is pushed. Local main is far ahead of `origin/main` (`0997bbac`). Don't push unless the user asks.

## The last two tasks

**G5 (settings keys and the settings file write).** Its executor did M18, M14 and M74, and part of M19. It hit its time limit before M53. Its branch is `worktree-agent-abe7782c5238491fa`, with key `1bc3b729853674c8137de4e31db8a27607e0f091`, and a reviewer was dispatched on it. RESULT: see the G5 line under "Session end" at the bottom of this note.

**G1 (messages, gates and modal ids).** All seven findings are committed on `worktree-agent-a5cc827e9ccb75bf3`, with key `fe86522dbdc40a138827e8ccde449b8a291ac3ac`. A reviewer was dispatched. G1 changes text you can see. The song panel's analysis-off sentence is longer now and wraps. So G1 needs a real-window check before merge. To trigger the sentence, put a broken `hydra_rules.ini` (a line like `x = nan`) in the scratch app folder, then capture the song panel. RESULT: see the G1 line under "Session end".

## Where to start

1. Do these three in parallel:
   - Start the CRLF fix for D114 item 28 (see the G5 line under "Session end").
   - Take G1's real-window screenshot and show it to the user. Then merge G1.
   - Launch G7.

   After each merge, build and run the full suite once.
2. G5 is on main, so G7 can start now. G7 does M53 first: it moves the overlay boxes' positioning into `overlay_layout`, with no box moving by a pixel. Then it builds the shared box helper the user asked for: "create a shared helper for every box instead of using different rules for each one". D114 item 27 gives the rule:
   - All four text boxes (time, score, SP drain, next activation) draw and measure through one helper.
   - Text is left-aligned in every box.
   - The inset is the preview config's time-box margin on all four sides: 10 px in the shipped file, scaled like the rest.

   G7 changes what the user sees, so take real-window screenshots before merging it.
3. Wave 3: H1, H2 and H3 together. Then H4 (the ScanItem half of M21), which needs H1's and H3's files. Then H5 (the rest of M19, plus `AnalysisSettings` holding a `ChartReading`), which needs H1's, H2's and H4's files.
   - H1 now also carries D114 item 26: the fillcompare summary line says "in both, not both scored".
   - H3 now also carries D114 item 25: the User Guide's scrubber sentence, which goes through the public doc review. It also carries the two M07 phrases moved over from F2.
4. Phase 7, the recurrence gate. Its third change still owes `CONTEXT.md` glossary entries for the concepts that had more than one home. Pass the side notes in `docs/audit/2026-10-11-concept-audit/side-notes.md` on to the user.

## Decisions made this session (D114 items 23 to 27)

Item 23 labels `hydra_batch`'s fill setting "Fills". Item 24 makes whole-number keys in rules.ini accept "4.0" and "1e1". Item 25 adds the scrubber sentence to the User Guide. Item 26 changes the fillcompare summary words. Item 27 is the one Preview box helper with a 10 px inset. Each is written out in full in the decisions file.

## Reviewer notes worth a follow-up

None of these blocks a merge. Each could be a small follow-up task, or go into a later wave task that owns the file.

- **Parser (F8).** The .chart accent and ghost note numbers still map pads to colours by hand. They could call `chart_pad_color` instead.
- **Path separators (G2).** `join_folder` in `winstr.cpp` still spells its own separator test. A row in `rules/core.json` quotes that line, and rows there are append-only.
- **Scan rows (G3).**
  - The M64 row catches only `ends_with_ci(... ".ext")`, so an `ext == ".ogg"` spelling slips past it.
  - The M63 row can't see the `chart_kind_of` switch in analysis.cpp or the switch in song.cpp.
- **Preview jump buttons (F5).** A chart with an activation but no scrubber length now keeps its jump buttons live. That's a side effect nobody listed, and the jump works.
- **Rescan failure (F4).** A failed reload after a rescan now keeps the old md5 until the next good reload. It shows a problem message, so it isn't silent.

## Traps this session hit

**Worktree base.** Agent-tool worktrees branch from `origin/main` (`0997bbac`), not from local main. Every wave 2 dispatch therefore told the executor to run `git merge --ff-only main` as its first git command. Keep doing that for G7 and wave 3. The shared briefs still say not to move the branch, so the dispatch has to override that one line.

**Time limit.** Executors run out at 20 minutes. G2 and G5 each finished only part of their findings. A task with more than four findings, or one that touches files another task owns, will split. Plan the split before dispatching.

**Owner lines in rules files.** A fix that changes a line some `tests/single_owner/rules/*.json` row quotes as an owner line fails the scan. The task has to own that rules file too. This is what blocked M20 in G2 and the ScanItem half of M21 in G2b.

**Real-window captures.** Captures come out white (about 27 KB) when the user's monitor is asleep. Hydra skips drawing while Windows says its window is hidden. Check the PNG's size: a real capture is about 780 KB. The recipe that worked:
- Copy into a scratch folder: `Hydra.exe`, `mimalloc*.dll`, `assets\`, `resource\`, and `hydra_settings.ini`, `hydra_ui.ini`, `hydra.db` with its `-shm` and `-wal` files, all from `C:\Program Files\Hydra`.
- Use `tools/capture_window.ps1`: `Start-Hydra`, then `Save-WindowShot`.
- Click with `Send-WindowMessage`, sending 0x0200 (mouse move), then 0x0201 (button down), then 0x0202 (button up).
- Client coordinates are the full-size capture's pixel minus (8, 31) on this maximized window. The first library row was at (69, 196), and the panel's Preview tab at (1109, 183).

When the screen is off, `hydra_uitest --script <file> --db <copy of the user's db>` shows the user's real library headlessly, as a fallback. It isn't the real-window check, so say so when you use it.

**Merging during a build.** Don't merge while a suite build is running in the main checkout. Wait for the build to finish.

**Docs-match-code.** The suite's docs-match-code test fails when an ADR or doc names, in backticks, a function a fix deleted. Grep `docs/` for the removed names before merging a deletion.

## Journal state at the stop

These lines came from the session journal. Most of these agents had already handed back. The journal flags them because their last call was the hand-back.

```
agent-a00e089542ad857c8: unfinished, last tool call SubagentHandback, touched 14:05
agent-a2c804aae40369edf: unfinished, last tool call SubagentHandback, touched 13:46
agent-a3a96c37cc0a94993: unfinished, last tool call SubagentHandback, touched 14:03
agent-a54f6794994bc9a1f: unfinished, last tool call SubagentHandback, touched 13:44
agent-a58fed012ede05653: unfinished, last tool call PowerShell, touched 14:13
agent-a5cc827e9ccb75bf3: unfinished, last tool call SubagentHandback, touched 14:12
agent-a617b6f4c8deeaae8: unfinished, last tool call SubagentHandback, touched 14:07
agent-aafc8507acb6b7428: unfinished, last tool call SubagentHandback, touched 13:44
agent-abe7782c5238491fa: unfinished, last tool call SubagentHandback, touched 14:11
agent-ae7a4379a33fcdf1c: unfinished, last tool call SubagentHandback, touched 14:09
agent-ae8c01adea44267ee: unfinished, last tool call SubagentHandback, touched 13:49
agent-af46e51273de15bdc: unfinished, last tool call PowerShell, touched 14:13
```

`a58fed012ede05653` is G1's reviewer. `af46e51273de15bdc` is G5's reviewer. Both were still working at 14:13.

## Session end

- G5: reviewed CLEAN (key `e7a5cd3e0b9c04387e7ac9dbe2e351ac9c1a128a`; reviewer added a read-only save test, a CONTEXT.md line and the missing M14 scan row) and MERGED as `6de1c26c`. The suite run after it is in session ec019463's scratchpad as `suite-G5.log`; its result is under "Suite after G5" below. **First job next session: D114 item 28.** G5's `write_file_replacing` (`src/app/report_files.*`) writes binary, so `hydra_settings.ini` turns LF-only on its next save; the user chose to keep CRLF. Make the writer write the settings file with CRLF (a small executor task with a test that a saved file's bytes contain `\r\n`, plus one reviewer), before anything ships. Reviewer note: M19 is partial and for now adds `ChartReading` beside `AnalysisSettings` and `PreviewSongKey` (same four fields); H5 finishes it.
- G1: reviewed CLEAN, signed off on key `6bfe1d797537ea2bca0e32a4bd0833396a444b3b` (one comment fix, `6bfe1d79`); review file `g1-review.md` in session ec019463's scratchpad. NOT merged: the real-window check of the song panel's wrapped analysis-off sentence (broken `hydra_rules.ini` in the scratch folder) comes first, shown to the user, then merge and the suite. Reviewer notes: the tools now print "hydra_rules.ini has an error:" and the loader's line on two lines, exit code 2 (the old tools printed only the loader's line, which starts "hydra_rules.ini:"); test_cli pins this through hydra_batch only.

## Suite after G5

Main at `6de1c26c` (G5 merged) passed the full suite: 1,343 of 1,343 unit test cases and 95 of 95 GUI tests (from `suite-G5.log` in session ec019463's scratchpad).
