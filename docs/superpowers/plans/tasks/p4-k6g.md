# Task p4-k6g: the User Guide and development.md say what the app does (phase 4, K6 guide half)

Plan: `docs/superpowers/plans/2026-10-04-phases-3-5.md`, task K6. Handoff: `docs/handoffs/2026-10-04-phase4-handoff.md`. The recheck with evidence is `docs/audit/2026-10-04-session-audit/step45.md` (section A). The user's answers are D48 in `docs/audit/2026-10-03-fix-decisions.md`; each question's "Recommended" line in `docs/audit/2026-10-04-phase-3-5-questions.md` is the decision. Read all four first, with offset and limit.

Docs only. No build needed. You edit two files.

## Worktree

From the main checkout: `git worktree add .claude\worktrees\p4-k6g -b claude/p4-k6g main`. Work only there.

## Part 1: sentences that contradict the code (approved in D48 Q32)

Check each against the code before you write it.

- **39** (line ~162): the drain box measures a full meter "at the current tempo" (the bar length at the playhead, `build_drain_box`).
- **40** (line ~118): the early-fill number is positive when you must hit that many ms early; negative is slack. Write `0.0ms` where it writes `0ms`. Owner: `early_fill_difficulty` in `src/core/model.h`.
- **42** (line ~122): the uncounted-backend range runs from the backend leeway (3 ms by default, `backend_leeway_ms`) up to the hit window, instead of a fixed "3ms".
- **43** (line ~170): a MIDI chart without the dynamics tag shows no ghost or accent counts, and the tab says why (`MidiParser::op_note` turns every note Normal without the tag). Check what the Dynamics tab actually prints in that case (`src/ui/dynamics_tab.cpp`, `src/app/dynamics_breakdown.cpp`).
- **91** (line ~126): the overfill warning needs both a clamped window and a listed squeeze or an uncounted/squeezed-out row (`rate_activation`, `squeeze_rating.cpp`), and a phrase that only ties the cap does not clamp.
- **93** (line ~112): an activation row's bars are the bars banked when you activate (`act.sp_meter()`), not bars spent (D48 Q9).
- **104** (guide line ~203 and `docs/development.md` ~38): the path report lists every chart mode at the current cap and fill rule, top N per chart and mode (`collect_rows`).

## Part 2: describe the new on-screen wording (D48)

Phase 3 is changing on-screen wording at the same time as you. Write the guide to match D48's answers, not what main shows today. Go through Q1 to Q31 of the questions file and fix every guide sentence that describes the old behaviour. At least:

- Q1: paths that tie for the top score are all optimal, in the report too.
- Q7: the report tile is "Hardest ms".
- Q9: bars banked when you activate.
- Q12: counts are singular at 1 ("1 bar") with commas from 1,000.
- Q13: fill rules are "Clone Hero 1.0" in sentences and "CH 1.0" in narrow columns; "(legacy)" is gone; the checkbox stays "1.0 fills".
- Q28: an empty report names the settings it looked under when other records exist.
- Q29: the leaderboard chips are "Under optimal", "At optimal" and "Above optimal" (around guide line ~216); fill-comparison rows are labelled by which database holds a record.
- Q6, Q8, Q11, Q15 to Q27, Q30, Q31: wherever the guide describes those screens.

## Part 3: defaults the docs test checks

Task p4-c1 adds `tests/test_docs_match_code.cpp`. It checks a default in prose when an HTML comment follows it directly, with no space: the last number before the marker on that line must equal the code's value, and for a flag the word "on" or "off" before it must match. Use exactly these marker names:

- `10 ms<!-- default: Settings::mslimit_value -->` and `starts on<!-- default: Settings::mslimit_enabled -->` (Path limit, D48 Q33).
- `50 ms<!-- default: Settings::backendlimit_value -->` and `starts off<!-- default: Settings::backendlimit_enabled -->` ("Hide backend rows beyond").
- `3 ms<!-- default: Rules::backend_leeway_ms -->` (backend leeway). If the guide writes it `3ms`, the number still parses.
- `500 ms<!-- default: kSqueezeWindowMs -->` (the squeeze window), wherever the guide states it.

Write the two Path-limit defaults into the guide where it describes that setting (D48 Q33 confirmed them) and mark them. Mark the leeway and the squeeze window wherever the guide states them.

The test also requires every backticked code name in the guide and development.md to exist in `src/`, `tools/` or `tests/` (names like `fn(`, `Type::name`, `kName`, snake_case with an underscore). Only backtick names that exist; grep each one. If you find an existing stale backticked name, fix the sentence.

The test also checks that the guide's `hydra_rules.ini` sample equals `core::Rules{}`'s defaults (`src/core/rules.h`). Read it and fix the sample if it differs.

You can't run that test here (it lives on another branch). The orchestrator runs it after joining.

## Owned files

`docs/UserGuide.md`, `docs/development.md`. Nothing else.

## Style

Plain English, one idea per sentence, short sentences, the guide's existing voice. Don't add new sections unless a fix needs one.

## Commits

One commit for Part 1, one for Parts 2 and 3. Trailers: `Task: p4-k6g`.
