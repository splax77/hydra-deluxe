# Path Builder: every piece reviewed, waiting on the real-app check (2026-10-29, session c9b0cb46)

This continues `docs/handoffs/2026-10-29-path-builder-build-handoff.md`. That handoff left PB-2a half built. This session finished the builder: PB-2a, PB-2b and PB-4 are built and reviewed, and so is a follow-up task, PB-2c, for the user's later rulings. HY-1 is caught up with main and reviewed again. Nothing is merged to `main` and nothing is pushed. The user asked for this handoff while the real-app check was set up but not yet done.

## Where everything is

The builder's work sits on one chain of branches. Each one holds the one before it.

- `claude/pb-pb2a`, tip bd120447, signed CLEAN. It holds PB-1 (the window, tip 9affa72c, signed earlier) and TM-1 (one decimal, D265, tip 2a164148, signed earlier). It also carries the PB-1 crash fix: `load_open_chart` returns a `unique_ptr`, and `PbState` keeps the chart alive in a `shared_ptr`.
- `claude/pb-pb2b`, tip 80ac8c99, signed CLEAN. It adds the strip, clicks on the highway and strip, removing a window, and `PreviewController::fill_at_image`.
- `claude/pb-pb4`, tip 79185476, signed CLEAN. It adds Copy, Paste, "Open path..." and "Save path...", plus four new error kinds.
- `claude/pb-2c`, tip 705fb03f, signed CLEAN. This is PB-2b and PB-4 merged together, plus the later rulings below. **This tip holds every builder task.** It is 17 commits behind `main`.
- `claude/pb-hy1`, tip cfd1fe9c, signed CLEAN (a resolution review of its catch-up merge). It is up to date with `main`. The catch-up added `src/app/launch.cpp` to the D268 scan rule's exempt list: its file-not-folder check reads no chart folder.
- `claude/integrate`, tip f4d313b8, in `.claude\worktrees\integrate`. It is the test bed: `main` plus `pb-2c` plus `pb-hy1`. It needs no review.

The worktrees are all under `.claude\worktrees\`: `pb-pb2a`, `pb-pb2b`, `pb-pb4`, `pb-2c`, `pb-hy1` and `integrate`. The older ones (`pb-en1`, `pb-en2`, `pb-en3`, `pb-pb1`, `pb-pb3`, `pb-pb3b`, `pb-rd1`, `pb-rd2`, `pb-plan-rev`, `tm-1`) are merged or superseded and can be cleaned up.

## Last combined run (on `claude/integrate` at f4d313b8)

The precheck asked for the whole suite and the three always-checks. All of them passed:
- the whole suite: 1,839 of 1,839 test cases;
- `"-tc=docs match code*"`: 6 of 6;
- `"-sf=*single_owner*"`: 19 of 19;
- `"-sf=*test_precheck*"`: 100 of 100.

The GUI tests passed too. All 11 builder tests in `hydra_pathbuilder_uitest` passed; that runner has no `--all`, so list them with `--list` and pass each with `--test`. All 11 Hydra GUI tests the change touches passed: the five song-panel ones (including `open-in-path-builder-greyed` and `-enabled`), `paths-rows` and five Preview ones.

The run before that caught one real clash: HY-1's launcher tripped the D268 missing-folder scan rule. The exemption above fixed it.

## The user's rulings this session, in their words

Every one is built and tested. DOC-1's D-record must quote them.

- PB-2a wording (labels and warning lines): "Keep as written".
- Later windows when an earlier one changes: "Keep it if possible", meaning "The later window keeps the end you chose while it's still possible. Hydra picks a new end only when it isn't, and says so." Its notice is approved: "The end you chose for a later window isn't possible any more, so Hydra picked a new one." ("Use it").
- A Path limit below 0 ms: "Note, limit only", a violet note such as "over your limit of -10 ms".
- No windows yet: "Hide the line" for the decision list, and "Hide it" for the strip's step line.
- The File menu: "Save path / Open path", beside the existing "Open...".
- A paste that doesn't fit: "Its own sentence", which reads "This text doesn't match a path on this chart, so your path stays as it was." The four other PB-4 sentences were kept as written: the bad file, the unreadable file, the failed save, and the dialog that didn't open.
- A click result that arrives after the path changed: "Drop it".
- Removing a window that breaks a later one: "Own sentence", which reads "Removing this window makes one of your later windows impossible, so the path stays as it was."
- A click while one of Hydra's paths is on screen: "Copy it, then apply" (one undo step).
- Opening the same path file twice: "Show each path once".
- The real-app check: "Check it all later", meaning one check once the builder can build a path.

## Next

1. **The real-app check (RA-1) with the user.** Copy `claude/integrate`'s build into a scratch folder, with a fresh copy of `hydra.db`, `hydra_settings.ini` and `hydra_ui.ini` from `C:\Program Files\Hydra`. Do it with the installed Hydra closed, then start that `Hydra.exe`. Never use `hydra_start("window")` for a check the user drives: that window ignores the mouse (memory: branch-window-check-recipe). This session set it up in `scratchpad\ra1_app2`, but the user never reported back. Things to show:
   - TM-1's wider badges;
   - HY-1's button;
   - the builder: highway and strip clicks, "Edit a copy", end choices, "Finish for me", Copy and Paste, Save path and Open path;
   - the sizes: the left column's 0.22 share, the strip one frame tall, markers 3 px wide.
2. **One open question for the user.** A click that fits no end, because no later window can still happen afterwards, is refused today with "This window makes one of your later windows impossible, so the path stays as it was." The example is Burnout, fill tick 36480, on a copy of Best path. The other choice is to offer to drop the later windows. Recommended: keep the refusal.
3. **DOC-1** (plan section DOC-1). It covers the User Guide's "Path Builder" section, a README line, the D-record with the rulings above, a `CONTEXT.md` paragraph ("decision list", "end choice", "Finish for me"), and `ui-testing.md` on `hydra_pathbuilder_uitest`. Build it from `claude/pb-2c`. The two public pages get a doc review.
4. **The merges, in order.**
   - First, catch `claude/pb-2c` up with `main`. PB-1's D267 entry will clash with D268 in `docs/audit/2026-10-03-fix-decisions.md`; keep both, in number order. Then send a resolution reviewer with the merge's key.
   - Merge `pb-2c` (it carries PB-1, TM-1, PB-2a, PB-2b and PB-4), then `pb-hy1` (it may need one more small catch-up), then DOC-1.
   - Run what the precheck prints at each merge.
5. **Screenshots** before the next release: `docs/images/preview-tab.png`, `main-screen.png` and `search-tips.png`.

## Notes the reviewers left (none hold a merge)

- `pb_strip`'s marker sizes copy the Preview's scrub-mark literals instead of sharing a constant. Judge them at RA-1.
- `fill_at_image` reads the transport's `now_ms()`, while `render()` draws at `tick()`. They differ by microseconds within a frame.
- The strip's step line updates one frame after a change, because it reads the decision list's run, which refreshes later in the frame.
- Undo after "Open path" restores the player's path, but the saved rows stay in the left list until another chart opens. That matches the plan.
- "Open path" and "Save path" have no headless GUI test, because they open the native file dialog.
- Path files use `.json` (`kPathFileExtension` in `pb_open.h`), because nothing else sets an extension.
- Paste runs on the calling thread. It took 14 ms on The Faceless's "The Spiraling Void", the largest corpus chart.
- The precheck warns that every range is over 400 lines, because each range also carries PB-1 and TM-1.
- A PB-2a reviewer's report gave a wrong full hash for its tip. The short hash and the branch were right. Merge by branch name, or check a hash with `git rev-parse`.

## Open items carried from the last handoff, still unruled

- Two rating ladders (concept audit M39): a plain row's centre band is -10 to under 3 ms, while a squeezed-out row's is -10 to under 10 ms.
- `hydra_ui` still carries `imgui_test_engine` (ADR 0032 says so).
- The layering test checks "at least one library" per file, while the plan promised "exactly one".
- A song panel with no artist and no charter reads "(unknown) · charted by (unknown)".
- The decision audit didn't reach the updater, font-role and note-shuffle plans, D1 to D47, or the ADRs.
- `docs/superpowers/plans/2026-10-25-rescan-on-open.md` is untracked in the main checkout, and D266 and D268 cite it.
- The "user_messages: run_tool turns an escaping error into exit code 1" test prints a harmless database line to stderr.
