# Handoff: build column shares, no-scroll clicks and left-aligned text, 2026-10-14

The plan is written and every question is answered. Nothing is built. The next session starts the build, but only after the user says yes to it (CLAUDE.md rule 3). The user asked for this handoff and an updated plan. They did not say yes to the build.

## Start here

Read the plan: `docs/superpowers/plans/2026-10-14-column-shares-scroll-left-align.md`. Its "The user's answers" section holds all eleven decisions, and where any later section still offers a choice, the answers win. Its "Overlap with D115" section says when each task may start. That section was rewritten at 18:45 from the D115 branches as they stood, after the D115 build had started in another session.

The plan fixes three things the user showed in videos on 2026-10-13 and 2026-10-14. They called each one "unacceptable":

1. Library columns jump whenever an analysis setting changes. The width rule measures the Best path text, and a setting change turns every Best path into "Not analyzed". The fix gives each text column a fixed share of the room, written once next to its table's column list. Library: Title 17%, Artist 10%, Charter 9%, Folder 39%, Best path 25%. Path report: Song 27%, Artist 15%, Charter 14%, Mode 12%, Path 32%. dmleaderboards: Song 48%, Artist 27%, Charter 25%. Number columns stay exactly as wide as their widest value.
2. Clicking a row scrolls the list. Any change of selection centres the row (`SetScrollHereY(0.5f)` in `library_table.cpp`). The fix is one owner, `selection_scroll`. A click never scrolls. Previous, Next, the arrow keys and a report-row click scroll only when the row is off screen, and only to the nearest edge.
3. All text left-aligned, decided in one place (`apply_theme`). Button labels stay centred (the user's answer 6). Buttons placed at a window's right edge stay there (answer 7). The score breakdown's numbers start in one column (answer 11, with the exact example in the plan).

The shares come from a measurement of the user's real library: 19,512 charts in a copy of the installed `hydra.db`, measured in the app's own fonts. The scripts, their output and the three mocks the user approved the shares from are in `docs/superpowers/plans/2026-10-14-column-shares/`. The plan says how to run them again.

## The D115 build is running in another session

At 18:45 the session "One record per chart build" had D115's wave 1 going in eight worktrees under `.claude/worktrees/d115-*`. Three of its tasks touch this plan's files:

- A2-5 rewrote the report table's `cell` with a `FittedTextCache` and renamed three report labels (committed 8003b5d6, not merged).
- A2-1 is editing `app_state`, the library model, the toolbar and the Library uitests. Its edits were uncommitted at 18:45.
- B1-6 and B1-7 (D115 wave 8) rename the score breakdown's fields and labels.

So no task here starts until the D115 tasks touching its files have merged. T1 waits for A2-5. T2 and T5a wait for A2-1. T5b, the score breakdown alone, waits for B1-7. T3 and T4 fork from the tip holding T1, T2 and T5a. The plan's overlap section gives the reasons and the Mode-column rule (answer 3: when D115's A2-2 drops Mode, the path report becomes Song 31, Artist 17, Charter 16, Path 36). Before dispatching anything, run `git worktree list` and diff the D115 branches again. The D115 session will have moved on.

Put one line into D115 A2-2's brief if T1 lands before it: "scale the path report shares to Song 31, Artist 17, Charter 16, Path 36 when you drop Mode". That brief belongs to the other session. Tell the user, or message that session, rather than editing its files.

## Still in flight when this was written

The session's journal reported this subagent as active and unfinished:

```
agent-a764d5771db875e84: unfinished, last tool call SubagentHandback, touched 18:38
```

That is the Fable planner that wrote the plan. It had already handed back its final report, and the plan file it wrote is the one above. It changed no code. Nothing else from this session is running.

`docs/handoffs/2026-10-11-path-report-speed-handoff.md` and `docs/superpowers/plans/2026-10-11-profiler-flame-graph.md` are still untracked in the main checkout. They belong to an earlier session, so this session left them alone.

## The first steps

1. Get the user's yes on the build.
2. Check which of A2-5, A2-1 and B1-7 have merged on main. Dispatch every task whose D115 dependency has merged, together, one Opus executor each in its own worktree (rule 6). Each brief starts with `docs/agents/brief-preamble.md`. Then it gives the task id, this plan, the owned files from the plan's "Tasks, waves and files", the tests it may run and the status rule.
3. One fresh Sonnet reviewer per change (D107), plus the derive-once review the hook asks for.

## The real-app check before merge

The user's top rule ("UI changes need a real-app check") applies. Tests checked against the rule's own output proved nothing on 2026-10-10. Run the real `Hydra.exe` on a copy of the installed `hydra.db` (with `-wal` and `-shm` if present) and the user's `hydra_ui.ini`, as the plan's "The real-app check before merge" lists. Flip 2x Bass and watch that no column moves. Click one spot six times and confirm it is the same song. Press Next past the bottom edge and confirm it scrolls one row. Look at the timeline's start and end labels side by side (answer 8). Then show the user screenshots of the Library (panel open and closed) and the path report beside the approved mocks before merging. Opening the app on the user's library is not a library run. A batch over it is, and needs the user's yes in chat (rule 8).

## Rules the build must keep

- Agents run only their own tests. The main session runs the full suite once per merge (rule 1).
- Stage files by name in the shared checkout. The D115 session's worktrees and commits share this repo.
- Nothing here changes stored data or `src/store/stored_versions.h`.
