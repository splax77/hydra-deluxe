# Path Builder redesign: the real-app check failed, a new mockup comes next (2026-10-30, session 28fa9d03)

This continues `docs/handoffs/2026-10-30-path-builder-redesign-handoff.md`. That session approved the redesign plan and started its first wave. This session built the whole plan, reviewed and merged every piece into the integration branch, and then ran the real-app check (RA-2) with the user. The check failed. The user found six problems, and their last words were "yeah no this is unreadable. we need a new approach." Nothing is merged to `main` and nothing is pushed.

The short version of what comes next: one new clickable mockup that fixes all six problems, which the user clicks through and answers four questions on. Then a planner writes the plan from it, and only then does anything get built. The rule behind that is the memory `new-screens-start-from-a-clickable-mockup`.

## What was built this session

Every task in the plan `docs/superpowers/plans/2026-10-30-path-builder-redesign.md` is built, reviewed by one fresh reviewer and merged into `claude/pb-redesign`, which now sits at 5efa659e. That covers EN-4 (the engine records the fills after a path's last activation), CO-2 (fill costs and the queue that prices them), PV-3 (the whole-song timeline), CO-1 (the sentences), PV-3b (the ghost path's row and the hidden "End" section), PB-5 (the screen, in four passes: PB-5, PB-5b, PB-5c and PB-5d), PB-6 (one GUI test that builds a whole path by clicking) and PV-4 (the first try at fixing crowded cost labels). The last full suite before PB-6 and PV-4 passed 1,851 of 1,851 cases. PB-6 and PV-4 then passed what `hydra_precheck` listed for them.

The user's answers from this session are recorded in the plan, in their words, under "The user's answers (2026-10-30)". They are in four blocks: "User decisions (PV-3)", "More answers from the wave 1 merges", "User decisions (PB-5)" and "User decisions (PV-4)". DOC-2 (the design notes, the decision record and the user guide) has not started, and it should wait for the new design.

Three times a reviewer signed off CLEAN and the merge tests then caught something it missed. EN-4 had a copied 8-line test chart. PV-3 had two single-owner scan failures, which its reviewer had called false without running the scan. CO-1 gave the wrong reason for a fill sitting exactly on Star Power's end tick. The main session fixed all three itself (f3b519c1, 28fa2471 and 91e558d5). Those fixes, the plan edit a32d200f and the main session's merge commits have had no fresh reviewer. The final merge of the whole branch to `main` needs one review of the whole range anyway.

## The six problems the user found

Each one below says what the user saw, what the cause is (checked in the code or the data), and the fix I recommend. The recommendations are mine. The user has not ruled on any of them yet, apart from asking for this handoff. The user's three screenshots are in `docs/handoffs/2026-10-30-path-builder-real-app-check-2/`, and the song was Blue Öyster Cult "(Don't Fear) The Reaper (Rh)", Expert Pro Drums with 2x bass.

### 1. The "Open in Path Builder" button was missing

The user asked "where did the open in path builder button go? why did you remove it?" Nothing removed it. The button was built in HY-1, which lives on its own branch, `claude/pb-hy1` (cfd1fe9c). The plan merges that branch only after the redesign reaches `main`, and the first check copy was built from `claude/pb-redesign` alone. That was the main session's mistake: the plan's check says to press the button, and the build didn't have it.

It is fixed for checks. `claude/pbr-ra2-check` (9d3d1925) is a throwaway branch: the redesign, plus `claude/pb-hy1`, plus PV-4. The one conflict was the known one in `docs/audit/2026-10-03-fix-decisions.md`, and both D267 and D268 are kept. HY-1's two button GUI tests pass on it. Every later check copy must be built from a branch like this, never from the redesign alone.

### 2. The cost labels under the fills can't be read

The first screenshot shows the costs running into each other wherever fills sit close together ("−8.7k−10.0k−11.4k" near 1:50, "84.9k84.7k" near 4:40). PV-4 tried the fix the user picked: stagger a label to a second line when it would touch the one before it, and use the mockup's smaller text sizes. The second screenshot shows the result, and the user called it unreadable.

There are two causes. The first is a bug in PV-4. It wrote the mockup's sizes into `src/ui/text_role.h` as fixed pixel sizes (`TimelineCost` 10.5, `TimelineSection` 11.5, `TimelineAxis` 11). Every other role there passes 0, which means "the user's own font size". The user's font size is far larger than the mockup's base size, so the timeline's text came out at about half the size of everything around it. Any new design must size these roles relative to the user's font size, not in fixed pixels.

The second cause is the design itself, and fixing the size doesn't save it. The Reaper has about 30 glowing fills across one row, about 60 px apart, and a figure like "−116.3k" needs most of that at a readable size. The mockup got away with one number under every fill because its test song had fewer fills.

The options offered to the user:

1. Colour instead of numbers: each glowing fill is tinted by its cost tier, only the best one says "best", and the hover tooltip keeps the exact figure.
2. A "next choices" list: the timeline shows no numbers, and a short panel lists the fills you can activate at next, sorted by cost. Hovering a row lights its fill and moves the highway there, and clicking it activates there.
3. A zoomable timeline: a 60-second window with an overview strip.
4. Two rows: the song's first half above, the second half below.

My recommendation is option 2 with option 1's colours. It is the only one that stays readable on a song this busy, and a sorted list answers the question the player is asking at each step ("where should I go next?"). The mockup should show it with "colours only" as a second tab to compare. The user had not picked one when they asked for this handoff.

### 3. The steps panel takes up far too much room

The user: "the table next to the highway is takin up way too much space". The steps panel takes the whole width beside a highway fixed at 600 px (`kHighwayW` in `src/pathbuilder/pb_window.cpp`, taken from the mockup). Most of the panel is empty. With the Preview panel brought in properly (problem 4), the highway should size itself the way Hydra's Preview does, and the steps should move into a narrower column. The exact split is for the mockup.

### 4. None of the Preview's features came across, only the highway

The user: "none of the features of the preview were added to the builder. only the highway itself." This is the most serious one, because it goes against the user's own words when the plan was ordered: "make sure the plan is using the preview code that already exists, not reinventing it".

Hydra's Preview has the transport buttons ("< Act", "Act >", jump and step), the volume slider, the scrub bar with its gold activation marks, and the boxes drawn over the highway: the time box, the score box, the SP gauge, the drain box, the next-activation box and the solo box. All of it is drawn by one function, `render_preview_panel` in `src/ui/preview_tab.cpp`, and that function takes Hydra's whole `AppState`, which the builder doesn't have. The plan quietly kept only the highway image, Play and the clock. That cut the user's words down, and it should have been asked as a question (memory `build-what-was-asked-not-a-nearby-mechanism`).

The fix: move the body of the Preview panel (the transport row, volume, scrub bar, highway image and all its overlay boxes) into shared code in `hydra_uibase` that takes the `PreviewController` and the few settings it reads, such as the volume, instead of `AppState`. Hydra's Preview tab and the builder then both call it, and Hydra's tab looks exactly as before. The path picker stays in Hydra's tab, because the builder's path is the one being built. This part is a move of existing code with no new look, so it needs no design approval. But the build still waits for the user's yes, and the builder's layout around it goes in the mockup.

### 5. A hollow ring means two different things

The user: "why is one part of the interface telling me i can activate and another telling me i cant?" The main session's first answer pointed at the wrong fills, so treat this as one real design flaw, found while looking for the contradiction the user meant.

The timeline draws one hollow ring for two different reasons. The first is that the game won't let you activate there: you have fewer than 2 phrases saved, or Star Power is already running. The second is that the builder won't let you: the fill comes before your last activation, and the user ruled (Q1) that the player builds left to right. Both come from one list, built from `fill_reason` in `src/pathbuilder/pb_text.cpp`. In the second case the saved line can sit above the activate line while the ring says you can't, as it does between activations 3 and 4 in the third screenshot.

The options offered: a separate, dimmed "locked" look for fills before your last activation, with its own legend entry, so the hollow ring only ever means the game won't let you (my recommendation, and it keeps Q1). Or make those fills clickable, so a click drops every activation from there on and activates at that fill as one undo step. That goes back on Q1, so it is the user's call.

### 6. "2 phrases: enough to activate" reads like the answer

This is the contradiction the user actually meant: "the interface very clearly says 2 phrases enough to activate. why am i getting 2 different answers?" They were hovering the fill at m180, after activation 4. The tooltip said "You have 1 phrase saved here. You need 2 to activate."

The tooltip is right, and the data shows it. The engine (`hydra_replay target` with the user's four activations at ticks 59520, 90240, 165120 and 291840, saved as `reaper-user-path-target.json` in the screenshot folder) prices the path at 864,325, the same as the screen. Activation 4 ends at tick 314,800. After that the chart has exactly one more Star Power phrase, ending at tick 316,320 (m165). So from m168 to m180 the player has 1 phrase, and the game shows no fill there. Measured off the third screenshot, the saved line sits at pixel row 308 there, one step below the dashed activate line at row 301 (rows 315, 308, 301, 294 and 288 are 0 to 4 phrases).

The trouble is the caption. "2 phrases: enough to activate" names the dashed line, but it is drawn at the row's right end, right where the saved line finishes, in a bright colour, worded like a status. Anyone reads it as "you have enough here". It came from the mockup and nobody caught it there.

The options offered: move the caption to the left, under the row's name, worded as what the line means ("dashed line = 2 phrases, needed to activate"), and shade the stretches where the saved line is at or above it, in the timeline's teal "you can activate here" colour (my recommendation). Or just move the caption, with no shading.

## What else is known and not yet settled

These came up during the build and are not ruled on. Hovering a fill pauses the song and moving off resumes it, as the mockup did. Nobody has judged that in the real window. End-choice costs are written in full ("−1,234") and fill labels short ("−1.2k"), following the mockup. The star inside "Save 2 more phrases" can land one line off in one wrapping edge case. The single-owner scan's `scope` takes only a top folder or one file, so the builder's two scan rows list its 16 files by name, and a new file in `src/pathbuilder/` goes unscanned until someone adds it. A small change to `tests/single_owner/engine.cpp` would fix that. The fonts plan `docs/superpowers/plans/2026-10-22-font-role-single-owner.md` now lists the three timeline roles, and it must change with them when the size bug is fixed.

## The best approach from here

First, one clickable mockup covering all six problems. Start from the folder `docs/superpowers/specs/2026-10-30-path-builder-mockup/`, whose scripts rebuild the mockup's data from `hydra_replay`. Use a dense chart from `testdata/input`, because the old mockup's song hid the crowding. The Reaper would show it best, but it is in the user's library, so ask before using it. The mockup should show:

1. The full Preview panel, in the real Preview's look, with its controls and overlay boxes.
2. A narrower steps column.
3. The timeline with colours and "best" marked, plus the sorted "next choices" list, with a "colours only" tab to compare.
4. The locked look for fills before your last activation.
5. The saved row with the caption on the left and the shading.

Each recommendation above should be the default, with the other options one click away, so the user can answer all four open questions in the mockup:

1. Which way do costs show?
2. How do fills before your last activation work?
3. How does the saved row show "enough"?
4. Is the layout right?

Second, once the user approves the mockup, a planner (Fable) writes the plan from it. It needs at least these tasks. A code-only task moves the Preview panel into shared code, with Hydra's tab unchanged and its GUI tests (`preview`, `preview-path-overlay`, `preview-controls`) passing. A timeline task fixes the font roles so they follow the user's font size, and applies whatever the user picks for the costs, the locked look and the saved row. A screen task lays out the builder around the shared Preview panel and adds the list if chosen, with PB-6's click-through test updated. Then a real-app check runs on a build that includes `claude/pb-hy1`. Every number the user reads must be checked against the data before it is called right or wrong (memory `check-the-screen-against-the-data`).

Third, after that check passes: DOC-2, one fresh review of the whole `claude/pb-redesign` range, and the merge order from the earlier handoffs. Catch the branch up with `main`, keep both D267 and D268, merge it, then `claude/pb-hy1`, then DOC-2.

## Where everything is

The integration branch is `claude/pb-redesign` at 5efa659e. The check branch is `claude/pbr-ra2-check` at 9d3d1925; throw it away once a new check branch replaces it. The older branches are unchanged: `claude/pb-hy1` (cfd1fe9c), `claude/pb-2c` (705fb03f) and `claude/integrate` (f4d313b8). The task branches `claude/pbr-en-4`, `claude/pbr-pb-5`, `claude/pbr-pb-6` and `claude/pbr-pv-4` are all inside `claude/pb-redesign` now.

The worktrees under `.claude\worktrees\` kept older folder names when their branches changed. `pbr-co-2` holds `claude/pbr-ra2-check`, with a warm build of Hydra and the builder. `pbr-pv-3` holds `claude/pbr-pv-4`. `pbr-co-1` holds `claude/pbr-pb-6`. `pbr-en-4` holds `claude/pbr-en-4`, and `pb-redesign-plan` holds the plan branch. All are clean.

The check copy of Hydra runs from this session's scratch folder, `ra2\`, on copies of the user's `hydra.db`, `hydra_settings.ini` and `hydra_ui.ini`. It may still be open. Close it before the next check, and copy the database and settings fresh, with installed Hydra closed. The recipe is in the memories `ui-changes-need-real-app-check` and `branch-window-check-recipe`.
