# Dynamic Hit Window row: the frontend and backend window, each with its note

## The short version

The Dynamic Hit Window row on the Paths tab gets simpler and stops contradicting the sentence above it. The new row shows two things only: the window on the frontend and the window on the backend, each followed by the note or chord on that end. The spare tag and all its arithmetic go.

On the user's own screen (Satellites, activation 3) the sentence says "no more than 83.3 ms late" and the row under it says `73.6 ms late   53.1 ms early` with a `210.0 ms spare` tag. After this plan the row reads `frontend 73.6 ms [Kick - Yellow cymbal]   backend 53.1 ms [Red]`.

The cause of the contradiction is that three pieces of code each decide "which end is each note on" by themselves. This plan gives that question one owner in the core.

Nothing is built until the user says yes. The mockup is at `docs/superpowers/specs/2026-11-13-dhw-derive-mockup/index.html`. Three questions for the user are at the end.

## Background

A squeeze has two notes. The **anchor** is the note whose timing moves the Star Power end: the activation chord, or the chord that overfilled the meter. The **squeezed note** is the SP phrase's last note. An early fill has one note: the one that makes Star Power ready.

The user's rulings of 2026-11-13, in their own words (P23 to P25):

- "i think early and late are the issue here causing confusion. change the display text to frontend and backend"
- "helps? hurts? why do you keep inventing your own metrics i never asked for?" and "you're STILL inventing your own metrics. why do you keep lying? squeeze 3 has a timing."
- "just show what the window is for the frontend and the backend and what the note or chord is on each end. dont invent your own labls or metrics."

So the row holds the label, the frontend end and the backend end. Each end is the word, that note's window, then that note's chord. Nothing else is on the row. The earlier draft of this plan added lead words and new tag wordings; they are gone.

How three owners came about. The squeeze sentence has picked "early" or "late" from one core test since commit c68982cf (2026-10-03). The schedule tool, written later (9d5bc5d5, 2026-11-08), picks direction by squeeze kind alone; that is safe there because the tool skips the squeezes where the two rules differ (P6). The row (970cb916, 2026-11-12) picks by kind alone too, but it shows on every badge, including the ones the tool skips. The brief says the row copied the tool's rule through the old plan's premise P10; that plan is removed and I did not open it.

What is wrong today, each checked in the code:

1. The row's direction words are fixed by squeeze kind (P4). The sentence's word follows a different test (P3). On Satellites activation 3 they contradict each other.
2. The spare tag is always "windows minus the badge figure" (P5). A negative figure makes it bigger: 73.6 + 53.1 + 83.3 = 210.0. The user did not ask for this figure, and it goes.
3. The row never says which note a figure is for. The anchor is named nowhere on screen.

The badge figure and the schedule file use opposite signs for a squeeze out. The badge shows `offset_ms` for a squeeze in and `offset_ms` negated for a squeeze out (P2). Satellites activation 2 has `offset_ms` -83.3 in the file and `squeeze out 83.3 ms` on the badge. The new row prints neither; it prints window sides only.

## The design

### One owner

| Question | Owner | Status |
|---|---|---|
| Which end (frontend or backend) is each note of a squeeze on? | new `squeeze_ends(const SPSqueeze&)` in `src/core/note_window.h` | new |
| Which end is an early fill's note on? | new `early_fill_end(double e_offset)` in `src/core/note_window.h` | new |
| What is each end's window and chord? | new `squeeze_window_ends(act, k, chords)` and `early_fill_window_end(act, chords)`, replacing `squeeze_out_window`, `squeeze_in_window`, `early_fill_window` | new |
| How wide is a note's window? | `chord_sides_ms` (`src/core/note_window.h`) | exists; unchanged |
| What is a chord called? | `Chord::rowstr` (`src/core/model.cpp`) | exists; unchanged |

The rulings this plan quotes are about what the row shows. They bind every row of this table: each one feeds the screen only a window side, an end word or a chord name. No row moves or converts a quantity the engine owns. Every figure is read from the chart's chord times.

### Which end each note is on

`squeeze_ends` is the code the sentence uses today, moved into the core (P3). In code terms it branches on `SPSqueeze::needs_no_timing()` and the squeeze kind. It returns the squeezed note's end; the anchor gets the other end.

The squeezed note's end is always the word its sentence prints: "late" is the backend, "early" is the frontend. The sentence asks the same function, so the two cannot disagree.

`early_fill_end` reads the sign the "Early fill:" line reads (`Activation::e_difficulty`, P10). A figure of 0 or more is the frontend. A figure below 0 is the backend.

### The row

The row reads: `Dynamic Hit Window:`, then `frontend`, its window and its chord, then `backend`, its window and its chord. Frontend always comes first; that is the fixed order, and the words themselves show it. An early fill has one end.

Each window is that note's own window side from the game's rule, as today (`chord_sides_ms`). Each chord is `Chord::rowstr` in the record's Pro Drums setting, the way the "Chord" line above it is (P19).

### The scale side is out of scope

The earlier draft fixed which side of the transfer scale the anchor uses. That only ever fed the spare. The new row prints two unscaled window sides and nothing else, so no figure on it reads a scale. The fix is dropped. `squeeze_spare_ms` was the only reader of the scale in the row's code, and it is removed.

### What goes

These are removed, with every test that pins them:

- `squeeze_spare_ms`, `early_fill_spare_ms`, `spare_is_short`, `WindowRow::spare_ms` and `WindowRow::is_short` in `src/core/note_window.h` and `.cpp`.
- `WindowRowView::tag` and `WindowRowView::is_short` in `src/app/path_view.h`, and the tag text in `window_row_view`.
- The tag drawing: `draw_window_tag` and `window_tag_width` in `src/ui/badge_pill.h`, and their calls in `window_row` in `src/ui/paths_tab.cpp`.
- In `tests/test_note_window.cpp`: "note window: the squeeze-out sums from the mockup" and "note window: exactly 0 is spare, anything below is short" whole, and the `spare_ms` checks inside "note window: each squeeze reads its own side of its own scale" and "note window: the early fill row reads the SP-ready note".
- The tag's checks in `tests/test_activation_rows.cpp`, `tests/test_badge_pill.cpp` and `tests/ui/uitest_paths.cpp`. I counted the mentions in those files but did not read the cases (P18); T2 lists each one by name before it deletes any.

### What each caller changes

The squeeze sentence (`squeeze_sentences` in `src/app/path_view.cpp`) asks `squeeze_ends` for the squeezed note's end and prints "early" or "late" from it. Its text does not change on any chart (question 2 asks whether it should).

The row view reads `squeeze_window_ends` and `early_fill_window_end`. It holds no direction word of its own. It names each note from the chord at that tick; `ChordTimes` gains the chord at each place, so one lookup gives the window and the name.

The schedule tool (`tools/replay_schedule.cpp`) reads the anchor's end from `squeeze_ends` where it picks by kind today: the aim's sign, the "early" or "late" word in its refusals, its choice of steady span, and the argument of `moving_side`. It still skips the squeezes it skips today, so on every squeeze it times the answer is the one it has now. Its output must not change by a digit. "schedule: Freak 1144, Rescued 561 and 931, Ghost 756" pins the aims and rooms of four squeeze-outs to the cent (P17).

Prior art, from one web search whose result pages I did not open: rhythm-game tools commonly show timing per note, and a write-up warns that players confuse the sign of an offset with early or late ([DEV Community](https://dev.to/skywalker_2de7de5f97df567/building-fair-rhythm-game-judgement-windows-in-javascript-1og9)). I found no tool that shows two notes' windows for one timing.

## What the user will see

Only the Paths tab's Dynamic Hit Window row changes. Badges, sentences, ratings, tiers, eff. figures and the Backend timings table stay as they are.

Each panel is in the mockup. "Today" is what Hydra 3.0.0's code computes. The windows and gems come from the figures file (P13). Chord words are written as with Pro Drums off (P21).

**Satellites, activation 2** (`squeeze out 83.3 ms`; sentence: "more than 83.3 ms late")
- Today: `85.7 ms early   53.1 ms late` · tag `55.4 ms spare`
- After: `frontend 85.7 ms [Kick - Blue cymbal]   backend 53.1 ms [Red]`

**Satellites, activation 3** (`squeeze in -83.3 ms`; sentence: "no more than 83.3 ms late")
- Today: `73.6 ms late   53.1 ms early` · tag `210.0 ms spare`
- After: `frontend 73.6 ms [Kick - Yellow cymbal]   backend 53.1 ms [Red]`

**Satellites, activation 5** (`squeeze in 166.7 ms`; sentence: "more than 166.7 ms early")
- Today: `85.7 ms late   53.1 ms early` · tag `27.9 ms short`
- After: `frontend 53.1 ms [Red]   backend 85.7 ms [Kick - Yellow cymbal]`

**Dead on Arrival (O), activation 3** (`squeeze in -444.9 ms`; sentence: "no more than 444.9 ms late")
- Today: `83.4 ms late   80.7 ms early` · tag `608.7 ms spare`
- After: `frontend 83.4 ms [Kick - Yellow cymbal]   backend 80.7 ms [Kick]`

**Burnout, activation 1** (`squeeze out 163.0 ms`; sentence: "more than 163.0 ms late")
- Today: `84.5 ms early   84.3 ms late` · tag `5.4 ms spare`
- After: `frontend 84.5 ms [Kick - Green cymbal]   backend 84.3 ms [Yellow cymbal]`

**Focus, activation 2** (`squeeze in 25.0 ms`; sentence: "more than 25.0 ms early")
- Today: `61.9 ms late   48.1 ms early` · tag `85.1 ms spare`
- After: `frontend 48.1 ms [Blue]   backend 61.9 ms [Kick]`

**Tapped Out, activation 3** (`early fill 0.0 ms`, required). The 0.0 is from the old mockup, not from the figures file (P14).
- Today: `82.9 ms early` · tag `82.9 ms spare`
- After: `frontend 82.9 ms [Kick - Green cymbal]`

I could not click the mockup in a browser myself: headless Edge returned nothing on two tries. The main session clicks it before the user sees it (T5).

## Tasks

T1 goes first. T2 and T3 start together once T1 is merged. T4 follows T2. T5 is the main session's. No agent builds or runs `hydra_tests`, `hydra_uitest` or `hydra_replay`; the main session builds each branch and runs the named tests.

### T1: the core owner

Owned files: `src/core/note_window.h`, `src/core/note_window.cpp`, `tests/test_note_window.cpp`.

Add `squeeze_ends`, `early_fill_end`, `squeeze_window_ends` and `early_fill_window_end`. `ChordTimes` gains the chord at each place, filled by `chord_times` from `Song::sequence` (P20). Leave the old row functions in place; T2 removes them, so T1 builds on its own.

Acceptance criteria, each a test in `tests/test_note_window.cpp` that the main session runs with `hydra_tests.exe "-tc=note window*"`:

- For a squeeze in and a squeeze out, each at an offset of +0.001, 0.0 and -0.001, the squeezed note's end is the one the design names and the anchor's is the other. The test fails if the function picks by kind alone.
- For an early fill at an `e_offset` of +0.001, 0.0 and -0.001, the end follows the sign of `e_difficulty`.
- `squeeze_window_ends` returns each end's window side and tick, with the frontend first, on a fixture whose two notes have different sides.
- The new functions return no spare, no scale and no figure but the two window sides. The reviewer reads the header for this.

### T2: the sentence, the row and the tag's removal

Waits for: T1 merged.

Owned files: `src/app/path_view.h`, `src/app/path_view.cpp`, `src/ui/paths_tab.cpp`, `src/ui/badge_pill.h`, `tests/test_activation_rows.cpp`, `tests/test_badge_pill.cpp`, `tests/ui/uitest_paths.cpp`, and the removals from `src/core/note_window.h`, `src/core/note_window.cpp` and `tests/test_note_window.cpp`.

First list, by name, every test case in the three test files that mentions the tag or the spare, and put the list in the report. Then make the changes in "What each caller changes" and "What goes".

Acceptance criteria. The main session runs `hydra_tests.exe "-tc=note window*"`, the activation-row and badge-pill cases, and the Paths uitest; the executor gives the exact case names in its report.

- The sentence text of every corpus activation is the same before and after. The existing sentence tests pin this, and none is edited.
- Burnout activation 1 and Focus activation 2 (both in the corpus) print the "After" rows above, pinned as literals.
- On every corpus squeeze, the row's end for the squeezed note matches its sentence: `backend` where the sentence says "late", `frontend` where it says "early". The test reads both from the built view and fails if either is picked by kind.
- Every row lists the frontend end first, and an early fill row has one end.
- Every squeeze in, squeeze out and early fill badge still has a row (P12): rows equal badges on every corpus path.
- A grep of `src/`, `tools/` and `tests/` finds none of `squeeze_spare_ms`, `early_fill_spare_ms`, `spare_is_short`, `spare_ms`, `draw_window_tag`, `window_tag_width`, `" spare"` or `" short"`.
- No row text holds "early", "late", "spare" or "short". A test reads every corpus row for this.
- Ratings, tiers and eff. figures are unchanged: their existing tests pass unedited.

### T3: the schedule tool

Waits for: T1 merged.

Owned files: `tools/replay_schedule.h`, `tools/replay_schedule.cpp`, `tests/test_replay_schedule.cpp`.

Route the tool's by-kind direction choices through `squeeze_ends`, as "What each caller changes" says.

Acceptance criteria, run by the main session with `hydra_tests.exe "-tc=schedule:*"`:

- Every existing schedule test passes with no expected value edited. That includes "schedule: Freak 1144, Rescued 561 and 931, Ghost 756" and "schedule: the equal-room split of a squeeze-in is the mirror of a squeeze-out's".
- A grep of `tools/replay_schedule.cpp` finds no direction picked from `SqueezeKind` alone.

### T4: the pages

Waits for: T2 merged.

Owned files: `docs/UserGuide.md`, `CONTEXT.md`, `docs/clone-hero/03-timing.md`, `docs/clone-hero/04-fills.md`, and the removal of this plan and both mockup folders in the same merge.

`docs/UserGuide.md` describes the row near line 273 (P16). Rewrite that passage for the new row in the public doc style, with every label checked against the code. It is a public page, so it goes past a doc reviewer. Update the other three pages where they describe the row or the spare; `04-fills.md` names `early_fill_spare_ms` (P15). The 3.0.0 release notes are history and stay.

Acceptance criteria: the main session runs `hydra_tests.exe "-tc=docs match code*"`, and the reviewer checks each quoted label against `src/app/path_view.cpp`.

### T5: the main session's screen checks

Before the user sees the mockup, open it and click every activation header and every "Collapse all" button. After T2 is built, open Satellites, Dead on Arrival (O) and Tapped Out by exact title with the `hydra` tools and read the rows of the activations in "What the user will see". This settles P13, P14 and P21. A mismatch is reported to the user, not patched.

## Out of scope

These are left out on purpose and need the user's nod.

- **The scale side.** No figure on the new row reads a scale (see "The scale side is out of scope").
- **An anchor shared by two timings.** Each row treats its timing alone.
- **Judge lag.** The game judges a press about one frame after it is sent. The tool counts that; the row does not.
- **The 3.0.0 release notes.** They are not rewritten.

## Promise map

| Promise | Plan section | Task | Check |
|---|---|---|---|
| One core function owns which end each note is on | One owner | T1 | the +0.001 / 0.0 / -0.001 cases in `note window*` |
| The early fill's end follows the sign the "Early fill:" line reads | Which end each note is on | T1 | the three `e_offset` cases |
| The new functions carry no spare, scale or other metric | The row; What goes | T1 | the reviewer's read of the header; T2's grep |
| The sentence reads the owner and its text does not change | What each caller changes | T2 | existing sentence tests, unedited |
| The squeezed note's end is the word its sentence prints | Which end each note is on | T2 | the corpus row-against-sentence test |
| The row is label, frontend end, backend end; frontend first; one end for an early fill | The row | T2 | the pinned Burnout and Focus rows; the order test |
| Each end names its chord in `Chord::rowstr` words | The row | T2 | the pinned Burnout and Focus rows |
| The tag, the spare functions and their tests are removed | What goes | T2 | the grep; the corpus test for "spare" and "short" |
| No row says "early" or "late" | Background | T2 | the corpus row-text test |
| Every badge keeps its row | One owner | T2 | rows equal badges on every corpus path |
| Ratings, tiers and eff. figures do not change | What the user will see | T2 | their existing tests, unedited |
| The tool's output does not change | What each caller changes | T3 | every `schedule:*` test, no expected value edited |
| The tool picks no direction by kind | What each caller changes | T3 | the grep |
| The public page matches the new row | T4 | T4 | doc review and `docs match code*` |
| The plan and mockups leave the repo with the work | T4 | T4 | `docs match code*` |
| The mockup is clicked before the user sees it | What the user will see | T5 | the main session's click-through |
| The real screens match the plan's rows | What the user will see | T5 | the main session's read of three songs |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: `needs_no_timing` is true exactly when a squeeze's `difficulty()` is below 0 | code | `src/core/model.h:411` and `src/core/model.h:332` | checked 2026-11-13 |
| P2: the badge figure is `offset_ms` for a squeeze in and its negation for a squeeze out | code | `src/core/model.h:271` and `src/core/model.cpp:494` | checked 2026-11-13 |
| P3: the sentence picks "early" or "late" for the squeezed note from the kind and `needs_no_timing` | code | `src/app/path_view.cpp:113` and `src/app/path_view.cpp:135` | checked 2026-11-13 |
| P4: the row's direction words are fixed by kind, and the early fill's is fixed "early" | code | `src/app/path_view.cpp:223` to `:227` and `:239` | checked 2026-11-13 |
| P5: the row's spare is windows minus the figure, and only the spare reads a scale | code | `src/core/note_window.cpp:65`, `:98` and `:104` to `:116` | checked 2026-11-13 |
| P6: the tool picks side, aim sign and word by kind, and skips squeezes where `needs_no_timing` is true | code | `tools/replay_schedule.cpp:127`, `:177`, `:216`, `:631` | checked 2026-11-13 |
| P8: a squeeze's anchor is the latest overfill at or before its step, else the activation; its squeezed note is the squeeze-out chord or the squeeze-in step's chord | code | `src/core/model.cpp:758` and `src/core/model.cpp:786` | checked 2026-11-13 |
| P10: the early fill's note is the second bar arrival; the "Early fill:" line prints `e_difficulty(true)`, the negated `e_offset`, positive when early | code | `src/core/model.cpp:695`, `src/core/model.cpp:477`, `src/app/path_view.cpp:274` to `:279` | checked 2026-11-13 |
| P12: every squeeze in, squeeze out and early fill badge gets the row; "a maximum of 8 words for the new info" | ruling | `docs/agents/rulings/ui-and-display.md` lines 5 and 7 | checked 2026-11-13 |
| P13: the window sides and gems of the seven panels | unchecked | `dhw_figures.json` in the session scratchpad, a tool's report read by the main session | T2 pins Burnout and Focus; T5 reads the others |
| P14: Tapped Out activation 3 is a required early fill of 0.0 ms | unchecked | `docs/superpowers/specs/2026-11-12-dynamic-hit-window-mockup/index.html` line 185, another agent's mockup | T5 |
| P15: the fills page names the early fill's spare function | code | `docs/clone-hero/04-fills.md:248` | checked 2026-11-13 |
| P16: the User Guide mentions the row | code | `docs/UserGuide.md:273` to `:277`; the label is there, the passage itself was not read | checked 2026-11-13 |
| P17: one test pins four squeeze-outs' aims and rooms to the cent | code | `tests/test_replay_schedule.cpp:473` | checked 2026-11-13 |
| P18: which test cases in the three app and UI test files pin the tag | unchecked | `tests/test_activation_rows.cpp`, `tests/test_badge_pill.cpp`, `tests/ui/uitest_paths.cpp`; mentions counted, cases not read | T2 |
| P19: `Chord::rowstr` joins each note's label with " - " in brackets; a label is "Kick", "Red", "Blue" or "Yellow cymbal" with Pro Drums off, and "Red snare" or "Blue tom" with it on | code | `src/core/model.cpp:192` and `src/core/model.cpp:61` | checked 2026-11-13 |
| P20: every chord in a song carries its `Chord` | code | `src/core/song.h:30` | checked 2026-11-13 |
| P21: the mockup's songs print their chords in the Pro Drums-off words and their kicks are not 2x | unchecked | `dhw_figures.json` holds each gem's colour and cymbal flag only | T5 |
| P22: the three commits exist with the dates the background gives | code | `git show -s c68982cf 9d5bc5d5 970cb916` | checked 2026-11-13 |
| P23: "i think early and late are the issue here causing confusion. change the display text to frontend and backend" | ruling | the user's message of 2026-11-13 as the main session quoted it to the planner (session `7d28ae9a`); the main session records it in `docs/agents/rulings/` | checked 2026-11-13 |
| P24: "helps? hurts? why do you keep inventing your own metrics i never asked for?" and "you're STILL inventing your own metrics. why do you keep lying? squeeze 3 has a timing." | ruling | the user's messages of 2026-11-13 as the main session quoted them to the planner (session `7d28ae9a`) | checked 2026-11-13 |
| P25: "just show what the window is for the frontend and the backend and what the note or chord is on each end. dont invent your own labls or metrics." | ruling | the user's message of 2026-11-13 as the main session quoted it to the planner (session `7d28ae9a`) | checked 2026-11-13 |
| P26: `CONTEXT.md` already defines Frontend squeeze, Backend squeeze and Backend row (D115) | code | `CONTEXT.md:570` to `:583` | checked 2026-11-13 |

## Questions

1. **The new row is longer than 8 words on most panels. Is that all right?** Your ruling of 2026-11-12 was "a maximum of 8 words for the new info". Counting every word after the label: Tapped Out 6 (`frontend 82.9 ms [Kick - Green cymbal]`), Focus 8 (`frontend 48.1 ms [Blue]   backend 61.9 ms [Kick]`), the three Satellites rows and Dead on Arrival 10 each, Burnout 11 (`frontend 84.5 ms [Kick - Green cymbal]   backend 84.3 ms [Yellow cymbal]`). A three-note chord on each end would give 14. Today's row is 6 words plus a 3-word tag, and the tag goes. The length comes only from the chord names your 2026-11-13 ruling asks for. The choices: (a) the row as mocked, which wraps in a narrow panel; (b) the Backend table's short chord form, `[K  B ]`, which makes every two-note row 8 words.

2. **Do the squeeze sentence and the early fill text switch to frontend and backend too, or only the row?** Your words were "change the display text to frontend and backend". The plan changes the row only, which may be narrower than you meant. Examples, Satellites activation 2 and activation 5:
   - (a) Row only. The sentences stay "Hit the [ R   ] note more than 83.3 ms late so it lands after Star Power ends." and "Hit the SP phrase's last note more than 166.7 ms early so it lands before Star Power ends." The rows under them read `backend 53.1 ms [Red]` and `frontend 53.1 ms [Red]` for that note.
   - (b) The row and the squeeze sentences. With the words swapped one for one they would read "Hit the [ R   ] note more than 83.3 ms backend so it lands after Star Power ends." and "Hit the SP phrase's last note more than 166.7 ms frontend so it lands before Star Power ends." The exact sentence wording would be mocked up for you first.
   - (c) Everything, including the name "early fill". Today the badge reads `early fill 0.0 ms` and the line `Early fill: 0.0 ms (required)`. The line holds no direction word, only the name. A new name would need to come from you.

3. **Is `backend` on the row all right next to the "Backend timings" table?** Hydra's glossary already uses the word. `CONTEXT.md` says a **Backend squeeze** is "Hitting a note near the SP end early, so it lands inside Star Power", a **Frontend squeeze** is "Hitting the activation chord's activation note first, so the chord's other notes score under Star Power", and a **Backend row** is "A chord within the squeeze window of an SP end, as the Backend timings table lists it". The row sits just above that table. Examples from both ends: on Satellites activation 2 the note at the SP end reads `backend 53.1 ms [Red]`, which happens to fit the table's sense. On Satellites activation 5 the note at the SP end reads `frontend 53.1 ms [Red]` and the activation chord reads `backend 85.7 ms [Kick - Yellow cymbal]`, the reverse of the glossary's sense. The choices: (a) use the words as ruled, and the User Guide says what they mean on this row; (b) you pick other words.
