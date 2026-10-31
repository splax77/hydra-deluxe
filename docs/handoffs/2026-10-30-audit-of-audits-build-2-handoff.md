# Audit of audits: the build, second stop (2026-10-30, session a1eb703d)

This continues `docs/handoffs/2026-10-30-audit-of-audits-build-handoff.md`. The user stopped all work again and asked for this handoff. Every running agent was stopped. Nothing is pushed: `main` (c8debd48) is 150 commits ahead of `origin/main`, counting other sessions' work.

## What the user ruled this session

Both rulings are in `docs/audit/2026-10-03-fix-decisions.md`.

- **D280.** T7 merges, then the report windows share the report's rows instead of copying them, and memory is measured again. The re-measure is written under D280. After that the user chose "Pin down the 60 MiB (Recommended)": a scout names the owner of the held memory the code doesn't explain, before any change.
- **D281.** The comparison's footer names SP cap 4. It ends "...at SP cap 4: set SP cap to 4, analyze them, then compare again." Text only.

The user also said "Merge as is (Recommended)" for Task 2, T3 and Tasks 3a+3b after their real-app checks. Those answers are in chat only, not in the decisions file.

## What merged to main this session

Each merge had its precheck tests plus the three `always:` checks run first. A merge that needed the whole suite got it.

- T5, the batch strip's dot (8f5d82d0).
- T7, closing a report window only hides it (566b61ff). Whole suite 1805/1805.
- Task 2, one Rating rule (cfc63495). Whole suite 1808/1808. Seen in the real window: Burnout reads `Insane+ (eff. 163.5 ms) <-- squeezed out (-260)`.
- Row sharing, D280 (724d5069). Its reviewer also made a hidden window drop an old result the frame a new one lands.
- T3, one "Nothing matches." sentence and one hint pattern (3494b168).
- Tasks 3a+3b, the settings file refuses bad lines (2eadbd1f). Whole suite 1817/1818; the one failure is below.
  - The first whole-suite run on 3a+3b caught two layering failures neither reviewer had run into. `ini_errors.h` was in no library's source list, and `widgets.h` included a `hydra_ui` header.
  - A fix moved `report_state.h` into `hydra_uibase` and listed `ini_errors.h`. It had its own executor and reviewer.
  - Its real-app check left the user's real `hydra_settings.ini` byte-identical: 371 bytes, SHA-256 `2BEFD0...ACEAB`.
- D280's re-measure and D281 (c8debd48).

## Main fails one check, and it is not this session's

`hydra_tests "-tc=docs match code*"` fails on `main` at `docs/agents/clone-hero-mcp.md:21`. That line names `FORBIDDEN_KEYS`, which another session's Clone Hero menu change removed from `tools/ch_mcp/menu_tools.py`. Every merge since then shows this one failure. It was left for the user to route to that session.

## What is reviewed but not merged

**T8**, Compare works at any SP cap (`claude/aoa-t8`, 79147657).
- CLEAN sign-off. The reviewer fixed `tests/test_dm_other_speed.cpp`, which the plan missed. It also fixed two User Guide sentences.
- The real-app check at cap 8 is done. The button is on. Its tooltip and the line under the header match D276 item 7 word for word.
- The footer's text was read back but not pictured, because the capture script failed twice.
- The user was asked whether to merge T8 and the footer change, and answered by stopping the work. So T8 waits on the user's go-ahead.

## What is unfinished

**The footer change, D281** (`claude/aoa-t8f`, d86aea33, cut from T8's tip).
- The executor's work is committed. Both 4s come from `kCloneHeroSpCap`.
- "SP cap" is typed, because the settings bar's label lives in `hydra_ui` and `hydra_dm` may not include it.
- Its reviewer was stopped mid-review and left one uncommitted edit in `tests/single_owner/rules/core.json` (1 line). A fresh reviewer should read that edit, then keep or redo it.
- It merges after T8.

**Task 4**, the documents (`claude/aoa-a4`, cut from c8debd48).
- Stopped after reading the plan. Nothing was committed or edited.
- A fresh executor starts from the brief this session used. It adds three things to the plan's Task 4:
  - D278 item 4 already settled "the 85 ms hit window".
  - The "Settings file error" entry the 3a and 3b reviewers added to `CONTEXT.md` must be merged with the plan's new "Settings files" entry, so there is one entry.
  - Every `hit_window_ms` mention as a setting must go from `docs/UserGuide.md` and `docs/differences-from-public-hydra.md`.

**The held memory, D280.** After the path report is opened and closed, Hydra holds about 121 MB more than at start. On a copy of the user's library, Private Bytes read 168.3 MB at start and 289.2 MB after close. It is live heap: mimalloc rises from 42.6 to 155.0 MiB.
- The first scout explained about 50 MiB from the code:
  - the rows (about 26 MiB);
  - a second copy of the chart index inside the report (about 17 MiB);
  - the search texts (about 7 MiB).
- The second scout was stopped part-way. Its scratch notes are in `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\a1eb703d-71a3-41c3-abc5-06500a73b413\scratchpad\scout-held-mem2\`. Its status lines said the 26 MiB of 96-byte blocks, about 284,000 of them, are `FittedTextCache` nodes: a key plus an entry holding two strings, for 4 cut columns × 68,135 rows. So every row gets fitted, not just the drawn ones.
  - It had not yet found why every row is fitted.
  - It had not identified the large blocks. It saw 28.3, 9.4, 4.2, 3.4 and 2.4 MiB.
  - Its Frida heap walk crashed the app now and then.
- Next: finish naming the owners, starting with why the clipper fits every row, then take the breakdown to the user. Nothing is built until they choose.

**Not started:**
- T4-BUILD, the time-left window. It waits on the library run, which needs the exact message `allow library run` in chat (D278 item 1).

## Small points the user saw and accepted

- **Task 2:** long Rating cells wrap mid-figure at a 1294 px window.
- **T3:** the two report windows' hints fit with only 6–7 px spare.
- **3b:**
  - a greyed Scan button shows no tooltip;
  - the Paths tab is blank under a refused file;
  - the path report window's copy of the orange lines can't be reached at start.

## Worktrees left

Under `.claude\worktrees\` the open ones are aoa-t8, aoa-t8f, aoa-a4, the two plan worktrees and aoa-t-measure. These are merged and can go: aoa-b5, aoa-b7, aoa-b7s, aoa-a2, aoa-a3a, aoa-a3b and aoa-t3. Several of the merged worktrees are left on a detached merge commit, not on their branch. The helpers' `hydra_live_*` folders in `C:\Users\Patrick\AppData\Local\Temp` were kept and sit outside the delete roots.
