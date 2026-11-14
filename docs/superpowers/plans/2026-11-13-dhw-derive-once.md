# Dynamic Hit Window row: the frontend and backend window, each with its note

## The short version

The Dynamic Hit Window row on the Paths tab stops contradicting the sentence above it. The row shows the window on the frontend and the window on the backend, each followed by the note or chord on that end, and then the small tag at the right edge. The tag keeps today's figure and gets a plain definition and new words: "xx ms safe" at 0.0 or more, "xx ms short" below 0.

On the user's own screen (Satellites, activation 3) the sentence says "no more than 83.3 ms late" and the row under it says `73.6 ms late   53.1 ms early` with a `210.0 ms spare` tag. After this plan the row reads `frontend 73.6 ms [K Y  ]   backend 53.1 ms [ R   ]` with a `210.0 ms safe` tag.

The cause of the contradiction is that three pieces of code each decide "which end is each note on" by themselves. This plan gives that question one owner in the core. The same owner also picks which side of the transfer scale the anchor's window counts through, which is the one place the tag's figure can change (Dead on Arrival, activation 3: 608.7 today, 609.3 after; the user chose this, question 4).

Nothing is built until the user says yes. The mockup is at `docs/superpowers/specs/2026-11-13-dhw-derive-mockup/index.html`. Four questions for the user are at the end.

## Background

A squeeze has two notes. The **anchor** is the note whose timing moves the Star Power end: the activation chord, or the chord that overfilled the meter. The **squeezed note** is the SP phrase's last note. An early fill has one note: the one that makes Star Power ready.

The user's rulings of 2026-11-13, in their own words (P23 to P25):

- "i think early and late are the issue here causing confusion. change the display text to frontend and backend"
- "helps? hurts? why do you keep inventing your own metrics i never asked for?" and "you're STILL inventing your own metrics. why do you keep lying? squeeze 3 has a timing."
- "just show what the window is for the frontend and the backend and what the note or chord is on each end. dont invent your own labls or metrics."

And, later the same day, on the little badge showing the timing leeway (P27): "the little badge showing the timing leeway is fine, but it needs to have a solid definition, not something you invented. i say keep it simple: xx ms safe >=0 ms, xx ms short <0ms".

So the row holds the label, the frontend end, the backend end and the tag. Each end is the word, that note's window, then that note's chord. The tag is the one figure Hydra ships today, under its new words. Nothing else is on the row. The earlier draft of this plan added lead words and new tag wordings; they are gone.

The tag's definition, in one sentence: the windows on the row's two ends added together, with the anchor's window counted through the stored transfer scale, minus the badge's own timing figure (`SPSqueeze::difficulty()` for a squeeze, `Activation::e_difficulty(true)` for an early fill). This is today's "spare" sum, renamed "safe". Its owner is `squeeze_safe_ms` and `early_fill_safe_ms` in `src/core/note_window.h`, the two functions that are called `..._spare_ms` today. The tag prints the size without a minus sign, "safe" where the sum is 0.0 or more and "short" where it is below 0. There is no other formula and no separate wording for negative figures.

How three owners came about. The squeeze sentence has picked "early" or "late" from one core test since commit c68982cf (2026-10-03). The schedule tool, written later (9d5bc5d5, 2026-11-08), picks direction by squeeze kind alone; that is safe there because the tool skips the squeezes where the two rules differ (P6). The row (970cb916, 2026-11-12) picks by kind alone too, but it shows on every badge, including the ones the tool skips. The brief says the row copied the tool's rule through the old plan's premise P10; that plan is removed and I did not open it.

What is wrong today, each checked in the code:

1. The row's direction words are fixed by squeeze kind (P4). The sentence's word follows a different test (P3). On Satellites activation 3 they contradict each other.
2. The tag's sum is "windows minus the badge figure" (P5). A negative figure makes it bigger: 73.6 + 53.1 + 83.3 = 210.0. The user ruled the tag stays, with the plain definition above and the words "safe" and "short" (P27).
3. The row never says which note a figure is for. The anchor is named nowhere on screen.
4. The scale side the anchor's window counts through is picked by squeeze kind (P5), a copy of the tool's rule. For a squeeze whose timing the sentence says "no more than" the anchor is on the other end than its kind implies, so the kind-based pick reads the scale side of the other end.

The badge figure and the schedule file use opposite signs for a squeeze out. The badge shows `offset_ms` for a squeeze in and `offset_ms` negated for a squeeze out (P2). Satellites activation 2 has `offset_ms` -83.3 in the file and `squeeze out 83.3 ms` on the badge. The tag reads the badge's own figure through `SPSqueeze::difficulty()`, which has the badge's sign for both kinds, so the file's sign never reaches the screen.

## The design

### One owner

| Question | Owner | Status |
|---|---|---|
| Which end (frontend or backend) is each note of a squeeze on? | new `squeeze_ends(const SPSqueeze&)` in `src/core/note_window.h` | new |
| Which end is an early fill's note on? | new `early_fill_end(double e_offset)` in `src/core/note_window.h` | new |
| What is each end's window, chord and the tag's sum? | new `squeeze_window_ends(act, k, chords)` and `early_fill_window_end(act, chords)`, replacing `squeeze_out_window`, `squeeze_in_window`, `early_fill_window` | new |
| Which side of the transfer scale does the anchor's window count through? | `squeeze_window_ends`, reading the anchor's end from `squeeze_ends`: frontend takes the scale's early side, backend its late side | new; replaces the pick by squeeze kind |
| What is the tag's sum? | `squeeze_safe_ms` and `early_fill_safe_ms` (`src/core/note_window.h`) | exist as `squeeze_spare_ms` and `early_fill_spare_ms`; renamed, same sum |
| Is the tag "safe" or "short"? | `safe_is_short` (`src/core/note_window.h`) | exists as `spare_is_short`; renamed |
| How wide is a note's window? | `chord_sides_ms` (`src/core/note_window.h`) | exists; unchanged |
| What is a chord called? | `Chord::notationstr` (`src/core/model.cpp`), the form the Backend timings table and the squeeze-out sentence print | exists; unchanged |

The rulings this plan quotes are about what the row shows. They bind every row of this table. Every row feeds the screen a window side, an end word, a chord name or the tag's sum. The sum already exists in the core and the plan only renames it. The one row that touches an engine-owned quantity is the scale side. The plan does not convert the scale or move any press with it: it only reads the stored scale's early or late side, as today, from the one owner of the anchor's end instead of from the squeeze kind. Every figure is read from the chart's chord times and the record's stored scales.

### Which end each note is on

`squeeze_ends` is the code the sentence uses today, moved into the core (P3). In code terms it branches on `SPSqueeze::needs_no_timing()` and the squeeze kind. It returns the squeezed note's end; the anchor gets the other end.

The squeezed note's end is always the word its sentence prints: "late" is the backend, "early" is the frontend. The sentence asks the same function, so the two cannot disagree.

`early_fill_end` reads the sign the "Early fill:" line reads (`Activation::e_difficulty`, P10). A figure of 0 or more is the frontend. A figure below 0 is the backend.

### The row

The row reads: `Dynamic Hit Window:`, then `frontend`, its window and its chord, then `backend`, its window and its chord, then the tag at the right edge, as `draw_window_tag` places it today. Frontend always comes first; that is the fixed order, and the words themselves show it. An early fill has one end.

Each window is that note's own window side from the game's rule, as today (`chord_sides_ms`). Each chord is `Chord::notationstr`, the short form the Backend timings table prints: five slots in K R Y B G order, a letter where the chord has that note and a space where it does not, so kick and blue read `[K  B ]` (P19). The user chose this form on 2026-11-13 (question 1, "Short table form"). It reads the same with Pro Drums on or off.

### The tag

The tag's text is the size of the sum without a minus sign, then " ms safe" where the sum is 0.0 or more and " ms short" where it is below 0. A sum of exactly 0.0 reads "0.0 ms safe". The test is on the unrounded sum, so a sum of -0.04 reads "0.0 ms short".

The sum is `squeeze_safe_ms` for a squeeze: the anchor's window times the scale side (below), plus the squeezed note's window, minus `SPSqueeze::difficulty()`. For an early fill it is `early_fill_safe_ms`: the window of the note that makes Star Power ready, minus `Activation::e_difficulty(true)`. Both are today's code (`src/core/note_window.cpp`, lines 65 to 69); the plan changes their names and the scale side, and nothing in the arithmetic.

### The scale side

Today the anchor's window counts through the stored scale's early side for a squeeze out and its late side for a squeeze in (`squeeze_out_window`, `squeeze_in_window`). That is the kind-based direction rule the sentence does not use. After the plan the owner reads the anchor's end from `squeeze_ends`: an anchor on the frontend counts through the scale's early side, an anchor on the backend through its late side.

Which stored scale it is stays as today: a squeeze out's is `Activation::transfer_post` and a squeeze in's is `SPSqueeze::transfer`. Those say where the record keeps the scale, not which way a note is hit. The figure changes only when the anchor's end differs from today's pick and the scale's two sides differ. That is a squeeze whose timing the sentence says "no more than", on a chart whose scale is not 1.0 on both sides. Of the six squeeze panels below, only Dead on Arrival activation 3 changes: its early side is 1.0044 and its late side 0.9970.

### What goes and what is renamed

Renamed, with the arithmetic untouched: `squeeze_spare_ms` to `squeeze_safe_ms`, `early_fill_spare_ms` to `early_fill_safe_ms`, `spare_is_short` to `safe_is_short`, `WindowRow::spare_ms` to `safe_ms`. In `src/ui/badge_pill.h` the tag drawing stays, and its comment says "safe" where it says "spare". `WindowRowView::tag` and `is_short` stay.

Removed: `squeeze_out_window` and `squeeze_in_window` (one function, `squeeze_window_ends`, replaces both) and `early_fill_window` (replaced by `early_fill_window_end`). The tag's wording in `window_row_view` moves from " spare" to " safe". The direction words `"early"` and `"late"` leave `window_row_view`.

The tests that pin the old names and wording are edited to the new ones, never loosened. T2 lists by name every test case in `tests/test_note_window.cpp`, `tests/test_activation_rows.cpp`, `tests/test_badge_pill.cpp` and `tests/ui/uitest_paths.cpp` that mentions the spare, the tag or the removed functions (P18; I counted the mentions but did not read the cases).

### What each caller changes

The squeeze sentence (`squeeze_sentences` in `src/app/path_view.cpp`) asks `squeeze_ends` for the squeezed note's end and prints "early" or "late" from it. Its text does not change on any chart; the user chose "Row only" (question 2).

The row view reads `squeeze_window_ends` and `early_fill_window_end`. It holds no direction word of its own. It names each note from the chord at that tick; `ChordTimes` gains the chord at each place, so one lookup gives the window and the name.

The schedule tool (`tools/replay_schedule.cpp`) reads the anchor's end from `squeeze_ends` where it picks a direction by kind today: the aim's sign, the "early" or "late" word in its refusals, its choice of steady span, and the scale side. `moving_side` stops taking the squeeze kind: it takes the anchor's end and returns the scale's early side for the frontend and its late side for the backend, the same rule as the row's tag. `SqueezeGeometry` gains the anchor's end, filled once where the geometry is built from the `SPSqueeze`. The tool builds a geometry only for squeezes whose timing needs hitting (it does not time the others), and for those the anchor's end is the one its kind gives today, so the tool's answers do not change. Its output must not change by a digit. Where the tool picks by kind for a reason that is not direction, such as which scoring rule pays a squeeze out's late gems, it keeps the kind. "schedule: Freak 1144, Rescued 561 and 931, Ghost 756" pins the aims and rooms of four squeeze-outs to the cent (P17).

Prior art, from one web search whose result pages I did not open: rhythm-game tools commonly show timing per note, and a write-up warns that players confuse the sign of an offset with early or late ([DEV Community](https://dev.to/skywalker_2de7de5f97df567/building-fair-rhythm-game-judgement-windows-in-javascript-1og9)). I found no tool that shows two notes' windows for one timing.

## What the user will see

Only the Paths tab's Dynamic Hit Window row changes: its direction words, its chords, the tag's word and, on one panel, the tag's last digit. Badges, sentences, ratings, tiers, eff. figures and the Backend timings table stay as they are.

Each panel is in the mockup. "Today" is what Hydra 3.0.0's code computes. The windows, gems and scales come from the figures file (P13). The tag's value on each panel is the figures file's window sides and scales put through the sum above, worked out again for this plan (the sum: anchor window times scale side, plus squeezed window, minus the badge figure). Chord words are the Pro Drums-on words, as on the user's screen (P21).

**Satellites, activation 2** (`squeeze out 83.3 ms`; sentence: "more than 83.3 ms late"; scale 1.0 both sides)
- Today: `85.7 ms early   53.1 ms late` · tag `55.4 ms spare` (85.7 + 53.1 - 83.3)
- After: `frontend 85.7 ms [K  B ]   backend 53.1 ms [ R   ]` · tag `55.4 ms safe`

**Satellites, activation 3** (`squeeze in -83.3 ms`; sentence: "no more than 83.3 ms late"; scale 1.0 both sides)
- Today: `73.6 ms late   53.1 ms early` · tag `210.0 ms spare` (73.6 + 53.1 + 83.3)
- After: `frontend 73.6 ms [K Y  ]   backend 53.1 ms [ R   ]` · tag `210.0 ms safe`

**Satellites, activation 5** (`squeeze in 166.7 ms`; sentence: "more than 166.7 ms early"; scale 1.0 both sides)
- Today: `85.7 ms late   53.1 ms early` · tag `27.9 ms short` (85.7 + 53.1 - 166.7 = -27.9)
- After: `frontend 53.1 ms [ R   ]   backend 85.7 ms [K Y  ]` · tag `27.9 ms short`

**Dead on Arrival (O), activation 3** (`squeeze in -444.9 ms`; sentence: "no more than 444.9 ms late"; scale early 1.0044, late 0.9970)
- Today: `83.4 ms late   80.7 ms early` · tag `608.7 ms spare`: 83.4 x 0.9970 (the late side, picked by squeeze in) + 80.7 + 444.9
- After: `frontend 83.4 ms [K Y  ]   backend 80.7 ms [K    ]` · tag `609.3 ms safe`: 83.4 x 1.0044 (the early side, because the anchor is on the frontend) + 80.7 + 444.9
- Keeping today's late side would print `608.7 ms safe`. The user chose the anchor's end (question 4).

**Burnout, activation 1** (`squeeze out 163.0 ms`; sentence: "more than 163.0 ms late"; scale early 0.9946, late 1.0224)
- Today: `84.5 ms early   84.3 ms late` · tag `5.4 ms spare` (84.5 x 0.9946 + 84.3 - 163.0)
- After: `frontend 84.5 ms [K   G]   backend 84.3 ms [  Y  ]` · tag `5.4 ms safe`

**Focus, activation 2** (`squeeze in 25.0 ms`; sentence: "more than 25.0 ms early"; scale 1.0 both sides)
- Today: `61.9 ms late   48.1 ms early` · tag `85.1 ms spare` (61.9 + 48.1 - 25.0)
- After: `frontend 48.1 ms [   B ]   backend 61.9 ms [K    ]` · tag `85.1 ms safe`

**Tapped Out, activation 3** (`early fill 0.0 ms`, required; P14)
- Today: `82.9 ms early` · tag `82.9 ms spare` (82.9 - 0.0)
- After: `frontend 82.9 ms [K   G]` · tag `82.9 ms safe`

I worked each "Today" value out from the figures file with today's rule, and each "After" value from the same file with the rule above. Only Dead on Arrival's differs. The one visible word change on every tag is "spare" to "safe"; "short" is unchanged. T5 reads the real screens to confirm (P13).

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
- The anchor's scale side follows its end. On a fixture whose scale reads early 1.0044 and late 0.9970, a squeeze out with a timing and a squeeze in with a timing use the early side for a squeeze out and the late side for a squeeze in. A squeeze in and a squeeze out whose timing needs no hitting use the early side for the squeeze in and the late side for the squeeze out. The test fails if the side is picked by kind.
- The safe sum is today's, renamed: the existing sum checks (the squeeze-out sums, the early fill row) pass with only the names changed and no expected value edited. Exactly 0.0 is not short and any sum below 0.0 is short (`safe_is_short`).
- The new functions return no figure but the two window sides and the sum. The reviewer reads the header for this.

### T2: the sentence, the row and the tag's new words

Waits for: T1 merged.

Owned files: `src/app/path_view.h`, `src/app/path_view.cpp`, `src/ui/paths_tab.cpp`, `src/ui/badge_pill.h`, `tests/test_activation_rows.cpp`, `tests/test_badge_pill.cpp`, `tests/ui/uitest_paths.cpp`, and the renames and removals in `src/core/note_window.h`, `src/core/note_window.cpp` and `tests/test_note_window.cpp`.

First list, by name, every test case in the four test files that mentions the tag, the spare or a removed function, and put the list in the report. Then make the changes in "What each caller changes" and "What goes and what is renamed".

Acceptance criteria. The main session runs `hydra_tests.exe "-tc=note window*"`, the activation-row and badge-pill cases, and the Paths uitest; the executor gives the exact case names in its report.

- The sentence text of every corpus activation is the same before and after. The existing sentence tests pin this, and none is edited.
- Burnout activation 1 and Focus activation 2 (both in the corpus) print the "After" rows and tags above, pinned as literals. The chords are pinned in the `Chord::notationstr` form, which does not depend on the Pro Drums setting.
- Every corpus tag reads its size without a minus sign, then " ms short" exactly when the sum is below 0.0 and " ms safe" otherwise. A test reads every corpus tag for this.
- On every corpus squeeze, the row's end for the squeezed note matches its sentence: `backend` where the sentence says "late", `frontend` where it says "early". The test reads both from the built view and fails if either is picked by kind.
- Every row lists the frontend end first, and an early fill row has one end.
- Every squeeze in, squeeze out and early fill badge still has a row (P12): rows equal badges on every corpus path.
- A grep of `src/`, `tools/` and `tests/` finds none of `squeeze_spare_ms`, `early_fill_spare_ms`, `spare_is_short`, `spare_ms`, `squeeze_out_window`, `squeeze_in_window`, `early_fill_window(` or `" spare"`.
- No row text outside the tag holds "early", "late", "spare" or "short". A test reads every corpus row for this.
- Ratings, tiers and eff. figures are unchanged: their existing tests pass unedited.

### T3: the schedule tool

Waits for: T1 merged.

Owned files: `tools/replay_schedule.h`, `tools/replay_schedule.cpp`, `tests/test_replay_schedule.cpp`.

Route the tool's by-kind direction choices through `squeeze_ends`, as "What each caller changes" says, including `moving_side` taking the anchor's end. First list every place in `tools/replay_schedule.cpp` that reads `SqueezeKind` (lines 127, 139, 161, 177, 182, 199, 216, 633, 673, 867 and 896 today), say for each whether it picks a direction or a scoring rule, and put the list in the report.

Acceptance criteria, run by the main session with `hydra_tests.exe "-tc=schedule:*"`:

- Every existing schedule test passes with no expected value edited. That includes "schedule: Freak 1144, Rescued 561 and 931, Ghost 756" and "schedule: the equal-room split of a squeeze-in is the mirror of a squeeze-out's".
- A grep of `tools/replay_schedule.cpp` finds no direction picked from `SqueezeKind` alone. A use that picks a scoring rule by kind (the squeeze-out late-gem rule) may stay, and the report names it.
- `moving_side` takes the anchor's end, not the kind, and a test calls it for both ends.

### T4: the pages

Waits for: T2 merged.

Owned files: `docs/UserGuide.md`, `CONTEXT.md`, `docs/clone-hero/03-timing.md`, `docs/clone-hero/04-fills.md`, and the removal of this plan and both mockup folders in the same merge.

`docs/UserGuide.md` describes the row near line 273 (P16). Rewrite that passage for the new row in the public doc style, with every label checked against the code. It is a public page, so it goes past a doc reviewer. Update the other three pages where they describe the row or the spare; `04-fills.md` names `early_fill_spare_ms` (P15). The 3.0.0 release notes are history and stay.

Acceptance criteria: the main session runs `hydra_tests.exe "-tc=docs match code*"`, and the reviewer checks each quoted label against `src/app/path_view.cpp`.

### T5: the main session's screen checks

Before the user sees the mockup, open it and click every activation header and every "Collapse all" button. After T2 is built, open Satellites, Dead on Arrival (O) and Tapped Out by exact title with the `hydra` tools and read the rows of the activations in "What the user will see". This settles P13, P14 and P21. A mismatch is reported to the user, not patched.

## Out of scope

These are left out on purpose and need the user's nod.

- **An anchor shared by two timings.** Each row treats its timing alone.
- **Judge lag.** The game judges a press about one frame after it is sent. The tool counts that; the row does not.
- **The 3.0.0 release notes.** They are not rewritten.

## Promise map

| Promise | Plan section | Task | Check |
|---|---|---|---|
| One core function owns which end each note is on | One owner | T1 | the +0.001 / 0.0 / -0.001 cases in `note window*` |
| The early fill's end follows the sign the "Early fill:" line reads | Which end each note is on | T1 | the three `e_offset` cases |
| The new functions carry no figure but the two window sides and today's sum | The row; The tag | T1 | the reviewer's read of the header |
| The tag is today's sum under the words "safe" (0.0 or more) and "short" (below 0), size without a minus sign | The tag | T1, T2 | the renamed sum tests with no expected value edited; the corpus tag test |
| The anchor's scale side follows its end from the one owner | The scale side | T1 | the fixture with early 1.0044 and late 0.9970 |
| The tool's `moving_side` takes the anchor's end | What each caller changes | T3 | the `moving_side` test for both ends |
| The sentence reads the owner and its text does not change | What each caller changes | T2 | existing sentence tests, unedited |
| The squeezed note's end is the word its sentence prints | Which end each note is on | T2 | the corpus row-against-sentence test |
| The row is label, frontend end, backend end; frontend first; one end for an early fill | The row | T2 | the pinned Burnout and Focus rows; the order test |
| Each end names its chord in `Chord::notationstr` form | The row | T2 | the pinned Burnout and Focus rows |
| The kind-based row functions and the word "spare" are gone | What goes and what is renamed | T2 | the grep |
| No row text outside the tag says "early" or "late" | Background | T2 | the corpus row-text test |
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
| P5: the row's spare is the anchor's window times a scale side, plus the squeezed window, minus `difficulty()`; the scale side is picked by squeeze kind (early for a squeeze out, late for a squeeze in); only the spare reads a scale; the early fill's is its window minus `e_difficulty(true)` | code | `src/core/note_window.cpp:65` to `:71`, `:98`, `:104` to `:116` and `:123` | checked 2026-11-13 |
| P6: the tool picks side, aim sign and word by kind, and builds no timed geometry for a squeeze where `needs_no_timing` is true (it presses that squeeze's note at a fixed early lead) | code | `tools/replay_schedule.cpp:127`, `:177`, `:216`, `:631`, `:679` and `:874` to `:875` | checked 2026-11-13 |
| P8: a squeeze's anchor is the latest overfill at or before its step, else the activation; its squeezed note is the squeeze-out chord or the squeeze-in step's chord | code | `src/core/model.cpp:758` and `src/core/model.cpp:786` | checked 2026-11-13 |
| P10: the early fill's note is the second bar arrival; the "Early fill:" line prints `e_difficulty(true)`, the negated `e_offset`, positive when early | code | `src/core/model.cpp:695`, `src/core/model.cpp:477`, `src/app/path_view.cpp:274` to `:279` | checked 2026-11-13 |
| P12: every squeeze in, squeeze out and early fill badge gets the row; "a maximum of 8 words for the new info" | ruling | `docs/agents/rulings/ui-and-display.md` lines 7 and 9 | checked 2026-11-13 |
| P13: the window sides, gems and transfer scales of the seven panels, and the tag values worked from them | unchecked | `dhw_figures.json` in the session scratchpad, a tool's report read by the main session; the sums were worked again by a reviewer from that file | T2 pins Burnout and Focus; T5 reads the others |
| P14: Tapped Out activation 3's badge reads `early fill 0.0 ms` and the activation is a required (E0) early fill | run | the main session's headless read of Tapped Out on `testdata/input` with the `hydra` tools, as the 2026-11-13 review brief states it; no result folder was kept | checked 2026-11-13 |
| P15: the fills page names the early fill's spare function and `early_fill_window` | code | `docs/clone-hero/04-fills.md:248` | checked 2026-11-13 |
| P16: the User Guide describes the row, its yellow `spare` tag and orange `short` tag | code | `docs/UserGuide.md:273` to `:277` | checked 2026-11-13 |
| P17: one test pins four squeeze-outs' aims and rooms to the cent | code | `tests/test_replay_schedule.cpp:473` | checked 2026-11-13 |
| P18: which test cases in the four test files pin the tag, the spare or a removed function | unchecked | `tests/test_note_window.cpp`, `tests/test_activation_rows.cpp`, `tests/test_badge_pill.cpp`, `tests/ui/uitest_paths.cpp`; mentions counted, cases not read | T2 |
| P19: `Chord::notationstr` prints five slots in K R Y B G order, a letter for each note the chord has and a space for each it lacks, with no Pro Drums words | code | `src/core/model.cpp:202` to `:210` | checked 2026-11-13 |
| P20: every chord in a song carries its `Chord` | code | `src/core/song.h:30` | checked 2026-11-13 |
| P21: the panels' chords are `Chord::notationstr` of each chord's gems in the figures file, so they do not depend on the Pro Drums setting | code | `src/core/model.cpp:202` to `:210`; the figures file | checked 2026-11-13 |
| P21b: a 2x kick prints as "K" like a plain kick in this form | code | `color_notationstr`, `src/core/model.cpp:50` to `:59` | checked 2026-11-13 |
| P22: the three commits exist with the dates the background gives | code | `git show -s c68982cf 9d5bc5d5 970cb916` | checked 2026-11-13 |
| P23: "i think early and late are the issue here causing confusion. change the display text to frontend and backend" | ruling | the user's message of 2026-11-13 as the main session quoted it to the planner (session `7d28ae9a`); the main session records it in `docs/agents/rulings/` | checked 2026-11-13 |
| P24: "helps? hurts? why do you keep inventing your own metrics i never asked for?" and "you're STILL inventing your own metrics. why do you keep lying? squeeze 3 has a timing." | ruling | the user's messages of 2026-11-13 as the main session quoted them to the planner (session `7d28ae9a`) | checked 2026-11-13 |
| P25: "just show what the window is for the frontend and the backend and what the note or chord is on each end. dont invent your own labls or metrics." | ruling | the user's message of 2026-11-13 as the main session quoted it to the planner (session `7d28ae9a`) | checked 2026-11-13 |
| P27: "the little badge showing the timing leeway is fine, but it needs to have a solid definition, not something you invented. i say keep it simple: xx ms safe >=0 ms, xx ms short <0ms" | ruling | the user's message of 2026-11-13 as the main session quoted it in the reviewer's brief (session `7d28ae9a`); the main session adds the line to `docs/agents/rulings/ui-and-display.md` after the review | checked 2026-11-13 |
| P28: `SPSqueeze::difficulty()` has the badge's sign for both kinds, and `Activation::e_difficulty(true)` is positive when the fill is hit early | code | `src/core/model.h:271`, `src/core/model.h:397` and `src/core/model.h:994` | checked 2026-11-13 |
| P26: `CONTEXT.md` already defines Frontend squeeze, Backend squeeze and Backend row (D115) | code | `CONTEXT.md:570` to `:583` | checked 2026-11-13 |

## Questions, answered

The user answered all four on 2026-11-13. Each answer is the label the user picked.

1. **Row length.** "Short table form". Each chord prints as `Chord::notationstr`, the Backend timings table's form (`[K  B ]`). A two-end row is 8 words without the tag and 11 with it.
2. **Sentences.** "Row only". The squeeze sentences and the "Early fill:" line keep "early" and "late". Only the Dynamic Hit Window row says frontend and backend.
3. **The word "backend".** "Keep frontend/backend". The row uses the words as ruled, and the User Guide says what they mean on this row.
4. **Scale side.** "Follow the anchor's end (Recommended)". The anchor's window counts through the scale's early side when it is on the frontend and its late side on the backend. Dead on Arrival activation 3 reads `609.3 ms safe`.
