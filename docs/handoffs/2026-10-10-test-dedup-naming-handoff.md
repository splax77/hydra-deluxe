# Handoff: test duplicates, naming and gaps (2026-10-10)

**For the next session: build waves A and B now; hold wave C until the single-owner redesign lands.** On 2026-10-10 the user read a summary of this plan and replied "approve all recommendations". That yes covers the design under repo rule 3. Every decision below is settled. Nothing has been built yet.

The plan is `docs/superpowers/plans/2026-10-10-test-dedup-naming-gaps.md`. Read it first; this note only records what was decided and how it fits with the other work running now.

## Why this exists

A read-only audit on 2026-10-10 (`docs/audit/2026-10-10-test-suite-audit.md`) found the same sentences and helpers copied across test files. It found files named after past plan steps (`test_s2_*`, "finding 37"), a grab-bag file, engine tests living in `test_search.cpp`, and nine files over 1,000 lines. It also flagged some pins as brittle and five source files as untested. The planner checked each claim in the code and rejected some of them. The scouts' evidence is in `docs/handoffs/2026-10-10-test-audit/`.

## What the user decided

Every decision tag ("finding 37", "(331)", "(R7.5)", "D105" and the rest) moves out of test names into a comment. The name says the behaviour.

The tooltip pins in `test_report_view.cpp` stay word for word, with a comment naming the decisions that fixed the wording. The user's rule is that UI text changes are deliberate, and the pin makes a reword a deliberate act.

The JS and CSS source pins in `test_report.cpp` keep their absence checks: no second fold, en-US grouping, and the D56, D57 and D48 decisions. The exact-expression pins loosen to the fact each one stands for.

Both vector-capacity pins stay, as a deliberate size-once design. The Song one gets renamed to say "sized once".

`test_search.cpp` splits into four topic files rather than taking one new name.

`icons.cpp` gets a short test that the four shipped PNGs decode, because its silent fallback is that file's one real risk. `dynamics_tab` and `stars_tab` were false alarms. `win32_dialogs` needs no test, and `imgui_files` is probably covered by `test_long_paths`; confirm that.

The two database sentences get exported as constants from `user_messages.h`. The four other files compare against those constants. They keep checking the sentence, because they prove it reaches the user through their path.

The renames and splits of the eight files coupled to the single-owner scan wait until that redesign lands. Those files are `test_song`, `test_app_state`, `test_search`, `test_preview_view`, `test_replay`, `test_report`, `test_store` and `test_path_view`. Everything else can go now.

## How to run it

Wave A starts now, and no two of its tasks touch the same file. Task 1.6 waits for task 1.2's merge, because both edit `test_app_state.cpp`. Wave B follows wave A's merge. Wave C (the coupled splits, then task 1.8) waits for the single-owner handoff, `docs/handoffs/2026-10-10-single-owner-redesign-handoff.md`, to land. Every move commit proves itself: run doctest's case and assertion counts with the same `-sf=` filter form before and after, write both numbers in the commit body, and use one `git mv` per commit. When a dedupe removes a clone, delete its single-owner baseline row in the same commit. The plan estimates about 40 commits over 20 tasks. Executors run on Opus, and each task gets one fresh Sonnet reviewer under D107. Run wave A when no open branch touches a file being renamed.

## Fits with other work

The "Trim the slow test and dead tests" session is editing `test_replay`, `test_rules`, `test_path_view`, `test_report`, `test_preview_load_progress` and `test_s2_dynamics_tag`, and removing some cases. Rebase task lists on its merge before touching those files.

The test-fidelity plan (`docs/handoffs/2026-10-10-test-fidelity-fixes-handoff.md`) owns `test_song.cpp`'s regex oracles, `test_fill_deadline`, the store's old-database fixtures, the golden test, the stem tests, the perf digest and the timing checks. Leave those parts alone here.

## Claims for the executor to check

The planner didn't confirm several claims it inherited from the scouts. These are the hand-rolled path overrides in `test_config.cpp` around line 139 and in `test_preview_controller.cpp` around lines 370 and 411, the `flac_with_unknown_length` declaration, and "a file from before AL" in `test_store`. It also read only the titles of `test_highway_draw.cpp` lines 331 to 813, `test_report.cpp` lines 300 to 548 and 620 to 890, and four s2 files. Check each claim in the code before acting on it.
