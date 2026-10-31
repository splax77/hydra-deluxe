# Audit of audits: the build, stopped part-way (2026-10-30, session adbda2b2)

This continues `docs/handoffs/2026-10-30-audit-of-audits-remaining-handoff.md`. The user stopped all work mid-build and asked for this handoff. Every running agent was stopped. Nothing here is pushed: `main` is 84 commits ahead of `origin/main`, counting other sessions' work.

## What the user ruled this session

Every ruling is in `docs/audit/2026-10-03-fix-decisions.md`.

- **D273** settles Q15 to Q18. The seventeen numbers stay. Q16 is dropped. The media guard comes back. There is no shadow mode for hook rules.
- **D276** holds the eight answers to the two build plans' questions.
- **D278** holds four later calls:
  - the time-left window is picked from one library run;
  - the yt-dlp ask stays;
  - a Cancelled report stays Cancelled after a close;
  - the report says "the 85 ms hit window".
- **D279** has two parts. A Refresh question closed with the window keeps the old rows. The user also allowed one short GUI analysis on a scratch copy of the library.

D274 and D275 record two old decisions that lived only in session cards. Another session took D277 at the same time, so this session's record became D278.

## What is merged

On Hydra's `main`:
- The record fixes, merged as 0715e9bf and a9ad4e0c.
- Task 3c: `hydra_replay --cap 0` reads 1. Merged as fda96a06.
- T6: one "Unknown option: --bogus" line for four tools. Merged as d2b02d72.
- Task 1: one `kHitWindowMs`, with no hit-window parameter anywhere. Merged as effe32a8.

In the hooks repo, the media guard was merged as 527e5fe. Hook code sits exactly at its 3,100-line budget.

## What is reviewed but not merged

Each of these branches has a CLEAN sign-off. All of them still lack something before they can merge.

- **T5**, the batch strip's dot (`claude/aoa-b5`, cecdb984):
  - The real-app check is done, and the dot has one space each side.
  - Its three repo-wide checks were stopped mid-run. Run them, then merge.
- **T7**, the report windows keep their rows (`claude/aoa-b7`, 6d7d08e6):
  - The real-app check is done, and reopening shows the same rows with no rebuild.
  - It waits on the user's decision about memory. D271 item 7 was weighed against "about 42 MB". Today, on a copy of the library, Hydra's private memory was 169 MB at start, 364 MB with the report open, and 350 MB after closing it. So about 180 MB stays held after a close.
  - Part of the reason is that the window keeps its own copy of the rows next to the report's. The main session's recommendation was to merge T7, then have the window share the report's rows and measure again.
  - The question was open when the user stopped the work.
  - Merging T7 also needs the whole test suite, because the precheck says so.
- **Task 2**, one Rating rule (`claude/aoa-a2`, 67e9557d):
  - It waits on its real-app check: Burnout's Rating cell, which should read `Insane+ (eff. 163.5 ms) <-- squeezed out (-260)`.

## What is unfinished

- **Task 3a**, the settings file refuses bad lines (`claude/aoa-a3a`, now 748f8c18):
  - The executor's work is at 5273332b.
  - The reviewer was stopped mid-review. It had already committed three fixes (f1a95628, 92d1b2a1, 748f8c18) but never signed off.
  - So 3a needs a fresh reviewer on its tip.
- **Task 3b**, what Hydra does with a refused settings file (`claude/aoa-a3b`):
  - The executor was stopped after writing its red test.
  - It left uncommitted edits in `src/ui/app_state.cpp`, `app_state.h`, `details_panel.cpp`, `library_toolbar.cpp` and `tests/test_app_click.cpp`.
  - A fresh executor should read them, then keep or redo them.
- **3a and 3b must merge together.** Until 3b lands, nothing catches `SettingsFileError`, so a bad settings file would stop Hydra at start. The user's own file is clean (D276).
- **Not started:**
  - T3 and T8 wait for T7 to merge.
  - Task 4 (the documents) waits for Task 2 and Task 3b.
  - T4-BUILD waits on the library run. The user agreed to it in D278 item 1, but the run itself still needs the exact message `allow library run` in chat.
- The two plans are approved, reviewed and committed on their own branches. Neither is on `main` yet:
  - `claude/aoa-plan-d269-d270` at 64f3cfb0;
  - `claude/aoa-plan-d271` at f12c3309.
  - Executors read them from those worktrees.
- The time-left measurement handoff sits on `claude/aoa-t-measure` (057689de). On the 97 test charts it showed nothing useful: the whole batch took 0.4 s of analysis.

## Notes the reviewers left for the user

- **T7:**
  - The "Show list" fold now stays open across a close.
  - The comparison window still can't be reopened without a new fetch.
  - With "Open automatically" on, a report that waited on its own batch no longer pops open by itself after a close. The user's own settings have `auto_open_report=0`.
- **Task 3c:** `--cap 2.5`, `--cap 4x` and a number too big for an int now refuse, where `atoi` used to read them.
- **Real-app check (D279):** more analysis ran than "one short run". Two redo batches of 87 and 146 charts each finished in about a second, before Stop could be clicked. A 7-chart batch also ran on each opening of the report, because the copied database had 7 charts with no result.
- **Leftover folders:** six snapshot folders of about 23 MB each sit in `C:\Users\Patrick\AppData\Local\Temp\hydra_live_*`. They are outside the agents' delete roots.

## Worktrees left

Under `.claude\worktrees\`: aoa-b5, aoa-b7, aoa-a2, aoa-a3a, aoa-a3b, aoa-plan-d269-d270, aoa-plan-d271 and aoa-t-measure, plus the merged ones (aoa-records, aoa-records-docs, aoa-a1, aoa-a3c, aoa-b6). In the hooks repo: `C:\Users\Patrick\.claude\worktrees\media-guard`.
