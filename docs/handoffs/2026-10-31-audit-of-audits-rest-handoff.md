# Audit of audits: what is left (2026-10-31, session e3dd3907)

This continues `docs/handoffs/2026-10-30-audit-of-audits-build-2-handoff.md` (removed; `git show ec91f8d1:docs/handoffs/2026-10-30-audit-of-audits-build-2-handoff.md`) and replaces `docs/handoffs/2026-10-30-audit-of-audits-remaining-handoff.md` (removed; `git show ec91f8d1:docs/handoffs/2026-10-30-audit-of-audits-remaining-handoff.md`). The audit's results are in `docs/audit/2026-10-29-audit-of-audits/` (`summary.md`, `questions.md`). Every ruling is in `docs/audit/2026-10-03-fix-decisions.md`.

Everything the user ruled on in this audit is built and on `main`. The records the audit found false are corrected. Two kinds of work are left: one live check that belongs to the Clone Hero work, and the list the audit never reached. Nothing below has started, and each piece waits for the user's yes (CLAUDE.md rule 3).

## What landed this session

All of it is on `main`, each merge with its precheck tests and the three `always:` checks.

T8 merged first: Compare works at any SP cap. The comparison footer change (D281) came next; its reviewer found the leftover scan edit could never fire and fixed the rule's anchor. Task 4, the documents for D269 and D270, merged after a doc review.

The held-memory question (D280) turned out to be mostly the test harness. The harness copies the screen's text every frame with `ImGui::LogToBuffer()`, which turns ImGui's list clipping off, so every report row is drawn and fitted every frame. A normally launched Hydra never does that. To measure without the harness, a dev-build option `Hydra.exe --open-report` was added (fe3f25e0). It opens the path report once the toolbar offers it; posting `WM_CLOSE` to the report's own window closes it. Posted mouse clicks can't drive Hydra, because ImGui's Win32 backend picks the hovered window from the real cursor. Measured plain, Hydra held 56.8 MiB after a close. The user chose two trims (D282, merged as 27c9be8d): the row list is sized to its rows, and the report keeps each row's copy count instead of the whole chart index. The re-measure read 32.6 MiB held. Both figures are written under D282.

The user then allowed the one library run D278 item 1 waited on. `hydra_batch` over `C:\Clone Hero` into a scratch database found 19,557 charts in 10.6 s and analyzed them in 2.8 s, so every time-left window read "about 0:00 left" or "about 0:01 left". The user picked E, the last 2W finished charts (D283). T4-BUILD merged as 99c732cd: one owner, `app::TimeLeftClock` in `src/app/time_left.h`, serves the batch strip and the Preview.

At handoff `main` was 35 commits ahead of `origin/main`, counting other sessions' work. Nothing from this session was pushed by this session; another session pushed `main` mid-session, which carried the earlier merges up to 27c9be8d.

## 1. Q7 waits on a live Clone Hero check

Q7 asks whether a chart's written fills stop Hydra generating its own. It is not a question for the user any more. It waits on live step LS3 of the Clone Hero server, which plays Senri Kawaguchi's "Ladies Talk" with every note hit and asks whether Star Power can turn on inside its drum solo. The steps and the chart's numbers are in `docs/handoffs/2026-10-30-ch-mcp-star-power-build-handoff.md` and `docs/handoffs/2026-10-30-ch-mcp-live-steps-handoff.md`. That work belongs to the Clone Hero sessions. When LS3 runs, record its result against D271 item 2.

## 2. What the audit never reached

The audit stopped after two waves of scouts, by its own stop rule. `summary.md` lists what was not reached under "Open, by the stop rule". Take it up only if the user asks, and read `docs/agents/audit-method.md` before planning it: every finding leaves by one of four exits, and a split goes to the user unless a ruling in the user's own words names both sides.

The biggest piece is about a dozen second claims inside concept-audit findings that the first wave closed. Each is named with its V file and line in `summary.md`: M03 and M05's fill-page claims, M06's `posted` sink, M07's colour tokens, M17's "is this SP cap Clone Hero's" check, M21's blank `sig`, M23's blank charter, V4's Ch10 sentence, M72 and M73's junk policy, M74's stored-key byte, and M77 and M81 to M83. The "Side notes" list at the end of each V file was not read either.

D114 items 17 and 18 answered questions about the Preview's playback range before D255 item 5 replaced that rule. They need a check against D255.

Three side notes are open. The note about `note_shuffle`'s `prev_tick = 0` needs a live Clone Hero run, so it also belongs with the Clone Hero work. The note about `Playhead::read_frames` may be moot, because the class is gone; confirm and close it. The audio headers' "throws std::runtime_error" wording was checked only for `device.h`.

Five PASSes are weak: M16, M34, M36, M37 and M48. Their closures were read from the finding's own tables and never re-checked in the code.

A third wave would be read-only scouts on Sonnet, a few jobs each, briefed with `docs/audit/2026-10-29-audit-of-audits/briefs/common.md`. Quotes are checked with `briefs/checkquotes.py`. The plan goes to the user before any scout runs.

## 3. Lessons from this session

How the user wants questions asked is in the remaining handoff (removed; `git show ec91f8d1:docs/handoffs/2026-10-30-audit-of-audits-remaining-handoff.md`) and still holds: lead with a real case, name every number's source, and mark made-up examples as made up.

A memory figure measured through the hydra MCP's window mode includes the harness. For a plain number, copy the build next to snapshot copies of `hydra.db` and `hydra_settings.ini` (plain Hydra ignores `--db` and `--ini` and reads them beside the exe), launch with `--open-report`, and close the report with `WM_CLOSE`. Read Private Bytes 90 s after the close, against a no-report run on the same files. Never move the user's cursor or raise a window over their screen to make a click land.

`main` moves often while the Clone Hero sessions work. A fast-forward of `main` failed five times this session because another session had merged in between. Each time the fix was the same: redo the merge on the new `main` in the spare worktree, check which files came in, re-run the three repo-wide checks (and the merge's own tests if code came in), then fast-forward.

## 4. Housekeeping

These worktrees under `.claude\worktrees\` are finished and merged: aoa-t8, aoa-t8f, aoa-a4, aoa-open-report, aoa-trim and aoa-t4-build, plus the ones the build-2 handoff listed (aoa-b5, aoa-b7, aoa-b7s, aoa-a2, aoa-a3a, aoa-a3b, aoa-t3). Several sit on a detached merge commit. aoa-t-measure holds the T-MEASURE handoff file on its own unmerged branch (`claude/aoa-t-measure`, 057689de); D283 now records its outcome, so that branch can go once the user agrees. The two plan worktrees (aoa-plan-d269-d270, aoa-plan-d271) hold the plans the builds followed, on unmerged branches. The cleanup hook removes merged idle worktrees; anything it leaves needs the user's word before it is deleted.
