# Dynamic Hit Window row: one owner for which way each note is pressed

## The short version

The Dynamic Hit Window row on the Paths tab is wrong on every squeeze that needs no timing. It names the wrong press direction, reads the wrong side of the scale, and prints a spare that is too big. On the user's own screen (Satellites, activation 3) the sentence says "no more than 83.3 ms late" and the row under it says `73.6 ms late   53.1 ms early` with a `210.0 ms spare` tag. The honest reading of that timing is the opposite: the game's windows are wide enough to break it by 43.3 ms.

The cause is that three pieces of code each decide "which way is each note pressed" on their own. This plan gives that question one owner in the core. The sentence, the row and the schedule tool all read it. The row also starts naming each note, so the figures say which note they belong to.

Nothing is built until the user says yes. The mockup is at `docs/superpowers/specs/2026-11-13-dhw-derive-mockup/index.html`. Four questions for the user are at the end.

## Background

A squeeze has two notes. The **anchor** is the note whose timing moves the Star Power end: the activation chord, or the chord that overfilled the meter. The **squeezed note** is the SP phrase's last note, which has to land on one side of that end. An early fill has one note: the one that makes Star Power ready.

Hydra gives each squeeze a figure in ms. On the badge, a squeeze in shows its `offset_ms` and a squeeze out shows `offset_ms` negated (`squeeze_difficulty`, P2). A figure of 0 or more is a timing the player has to hit. A figure below 0 means the squeeze happens with plain presses, and the figure is how much room there is before bad presses undo it (`needs_no_timing`, P1). The schedule file writes the raw `offset_ms`, so a squeeze out's sign there is the opposite of the badge's: Satellites activation 2 has `offset_ms` -83.3 and the badge `squeeze out 83.3 ms`.

How three owners came about. The squeeze sentence has picked its words from `needs_no_timing` since commit c68982cf (2026-10-03). The schedule tool, written later (9d5bc5d5, 2026-11-08), picks direction and scale side by squeeze kind alone; that is safe there because the tool skips every squeeze that needs no timing (P6). The row (970cb916, 2026-11-12) picks by kind alone too, but it shows on every badge, including the ones the tool skips. The brief says the row copied the tool's rule through the old plan's premise P10; that plan is removed and I did not open it.

The user ruled on the row's words on 2026-11-13: "i think early and late are the issue here causing confusion. change the display text to frontend and backend" (P23). So each figure on the row reads `frontend` or `backend` in place of "early" or "late". `frontend` is the side of a note's window before the note, reached by a press ahead of it. `backend` is the side after the note, reached by a press behind it. The words still come from the one core owner. The ruling does not say whether the squeeze sentence and the "Early fill" line change too; that is question 3.

What is wrong today, each checked in the code:

1. The row's direction words are fixed by squeeze kind (P4). The sentence's words follow `needs_no_timing` (P3). On a squeeze that needs no timing they contradict each other.
2. The spare is always "windows minus the figure" (P5). A negative figure makes it bigger: 73.6 + 53.1 + 83.3 = 210.0. The early fill row has the same sum and a fixed word "early" (P4).
3. The scale side is fixed by kind too (P5). A squeeze in always reads the late side. When the squeeze needs no timing, the press that matters is an early anchor press, so the early side applies. Dead on Arrival (O) activation 3 has early 1.0044 and late 0.9970, so the two readings differ there.
4. The row never says which note a figure is for. The anchor is named nowhere on screen. The order (anchor first) is written only in comments.

## The design

### One owner

A new set of functions in `src/core/note_window.h`, beside the row functions it replaces, answers these questions once.

| Question | Owner | Status |
|---|---|---|
| Does this timing need hitting? | `SPSqueeze::needs_no_timing`, `early_fill_needs_timing` (`src/core/model.h`) | exists; unchanged |
| Which way is each note of this timing pressed, and are those presses the ones that make it or the ones that undo it? | new `squeeze_ways(const SPSqueeze&)` and `early_fill_way(double e_offset)` | new |
| Which stored scale belongs to squeeze `k`? | new `squeeze_scale(const Activation&, size_t k)` | new; replaces the copies at P5 and P6 |
| Which side of that scale does the anchor's press use? | new `anchor_scale(PressWay, const TransferScale&)` | new; replaces `moving_side` |
| What do the game's windows allow, against what the timing needs? | new `squeeze_presses(act, k, chords)` and `early_fill_presses(act, chords)` | new; replace `squeeze_out_window`, `squeeze_in_window`, `early_fill_window` |
| What is a chord called? | `Chord::rowstr` (`src/core/model.cpp`) | exists; unchanged |
| Which scale side does the eff. figure read? | `rate_note` (`src/core/squeeze_rating.cpp`) | exists; unchanged; see question 2 |

The two rulings this plan quotes (P12) are about what the row shows, and they bind the rows above that feed the screen: every squeeze in, squeeze out and early fill badge gets a row, and ratings, tiers and eff. figures stay on the fixed 85. No row of the table moves or converts a quantity the engine owns. Every figure is read from the stored record and the chart's chord times.

### The direction rule

`squeeze_ways` branches on `needs_no_timing()`, the same owner the sentence reads.

When the timing needs hitting, it returns the presses that **help**. A squeeze out: anchor early, squeezed note late. A squeeze in: anchor late, squeezed note early. This is what the row and the tool do today.

When the timing needs none, it returns the presses that **hurt**. These are the mirror. A squeeze out that needs no timing: anchor late, squeezed note early. A squeeze in that needs no timing: anchor early, squeezed note late. The squeezed note's word is the one the sentence already prints ("no more than 83.3 ms late").

The early fill follows the same shape with one note. When it needs hitting (`early_fill_needs_timing`), the note that makes Star Power ready is pressed early, and that helps. When it has time to spare, a late press of that note is what hurts.

### The scale side

A scale has an early side and a late side. The early side is how far an early anchor press moves the end, and the late side is the same for a late press (P7). So the side follows the anchor's press direction, and `anchor_scale` is that one line. The anchor's direction comes from `squeeze_ways`, so nothing picks a side by kind any more.

Away from an offset of exactly 0, this picks the same side `rate_note` picks for the eff. figure (P11). At exactly 0 on a squeeze in, the two differ. That is question 2.

### The spare

The spare gets one meaning: **room left, and positive is good**.

When the presses help, the spare is what the windows reach minus what the timing needs. This is today's sum. Zero or more reads `N ms spare`. Below zero reads `N ms short`.

When the presses hurt, the spare is the room the timing has minus what the windows reach. Zero or more reads `safe by N ms`: even the worst presses the game still accepts cannot undo it. Below zero reads `can break by N ms`: presses inside the game's windows can undo it, by that much.

In both cases the reach is the anchor's window side times the scale side, plus the squeezed note's window side. For an early fill it is the one note's side.

The good tags (`spare`, `safe by`) are yellow and the bad ones (`short`, `can break by`) are orange, as `spare` and `short` are today. The edge at exactly 0 reads the good word in both cases, as `spare_is_short` does today (P5). No new number is added.

### What the row says

The row reads, left to right: the label, one lead word, the first note with its figure and direction, the word `then`, the second note with its figure and direction, and the tag.

- The lead word is `Helps:` when the presses make the timing and `Hurts:` when they undo it. The same direction words mean opposite advice in the two cases, so the row has to say which it is.
- Each figure's direction word is `frontend` for an early press and `backend` for a late one (the ruling, P23). The owner returns the direction; one function in `src/app/path_view.cpp` turns it into the word, so no caller spells either word itself.
- These words already mean other things in Hydra (P24). `CONTEXT.md` defines a **Frontend squeeze** as "Hitting the activation chord's activation note first, so the chord's other notes score under Star Power", a **Backend squeeze** as "Hitting a note near the SP end early, so it lands inside Star Power", and a **Backend row** as "A chord within the squeeze window of an SP end, as the Backend timings table lists it"; it adds that "In public text "backend" names a scoring effect, a table row and a squeeze" (D115). The row sits a few lines above the "Backend timings" table. There, "backend" means "near the SP end", and a backend squeeze is hit early. On the row, `backend` would mean "the late side of this note". So `[Red snare] 53.1 ms backend` could be read as "this is the backend note" instead of "press it late", and on a squeeze in the SP-end note carries the word `frontend`. Whether that is acceptable is the user's call; it is question 4.
- Each note is named in `Chord::rowstr` words, in the record's Pro Drums setting, the way the "Chord" line above it is (P19).
- The order is **song order**: the note played first is written first. The word `then` shows the rule on the row itself. I picked song order because it needs no knowledge of what an anchor is. The anchor is played before the squeezed note, so this is also the order the row has today. That fact is P9, which is unchecked; task T1 checks it on every corpus squeeze and stops if it fails.

Prior art, from one web search whose result pages I did not open: rhythm-game tools commonly show timing as a signed error per note, and a write-up warns that players confuse the sign of an offset with early or late ([DEV Community](https://dev.to/skywalker_2de7de5f97df567/building-fair-rhythm-game-judgement-windows-in-javascript-1og9), [TECHMANIA docs](https://techmania-team.github.io/techmania-docs/English/Offset_and_latency.html)). That supports naming the direction in words and never by sign. I found no tool that shows two notes' windows against one timing.

### What each caller changes

The squeeze sentence (`squeeze_sentences` in `src/app/path_view.cpp`) stops choosing "early" or "late" itself. It asks `squeeze_ways` for the squeezed note's direction and for helps or hurts, and builds "more than X late" or "no more than X early" from those. Its text does not change on any chart.

The row (`squeeze_window_view`, `early_fill_window_view`, `window_row_view`) reads `squeeze_presses` and `early_fill_presses`. It no longer holds a direction word. It looks up each note's chord by tick to name it; `ChordTimes` gains the chord at each place so one lookup gives the side and the name.

The schedule tool (`tools/replay_schedule.cpp`) loses `moving_side`. Its `moving_ratio`, the aim's sign, the "early"/"late" word in its refusals and its choice of steady span all read the anchor direction from `squeeze_ways` and the side from `anchor_scale`. Its "which scale" line reads `squeeze_scale`. The tool still skips squeezes that need no timing, so on every squeeze it times the answer is the one it computes today. Its output on timed squeezes must not change by a digit. "schedule: Freak 1144, Rescued 561 and 931, Ghost 756" pins the aims and rooms of four timed squeeze-outs to the cent (P17). Three more tests look, by name, like they pin the rest; T3 reads them first (P18).

## What the user will see

Only the Paths tab's Dynamic Hit Window row changes. Badges, sentences, ratings, tiers, eff. figures and the Backend timings table stay as they are.

Each panel below is in the mockup. "Today" is what Hydra 3.0.0's code computes from the same figures. Sides and offsets come from the figures file (P13). The sums are worked by hand from those figures, so the tests in T1 and T2 pin the real ones.

**Satellites, activation 2** (`squeeze out 83.3 ms`). 85.7 + 53.1 = 138.8 reach against 83.3 needed.
- Today: `85.7 ms early   53.1 ms late` · `55.4 ms spare`
- After: `Helps: [Kick - Blue cymbal] 85.7 ms frontend then [Red snare] 53.1 ms backend` · `55.4 ms spare`

**Satellites, activation 3** (`squeeze in -83.3 ms`). 73.6 + 53.1 = 126.6 reach against 83.3 of room. The brief's 43.4 came from rounded inputs; the unrounded sum is 43.30.
- Today: `73.6 ms late   53.1 ms early` · `210.0 ms spare`
- After: `Hurts: [Kick - Yellow cymbal] 73.6 ms frontend then [Red snare] 53.1 ms backend` · `can break by 43.3 ms`

**Satellites, activation 5** (`squeeze in 166.7 ms`). 138.8 reach against 166.7 needed.
- Today: `85.7 ms late   53.1 ms early` · `27.9 ms short`
- After: `Helps: [Kick - Yellow cymbal] 85.7 ms backend then [Red snare] 53.1 ms frontend` · `27.9 ms short`

**Dead on Arrival (O), activation 3** (`squeeze in -444.9 ms`). Today reads the late side, 0.9970. After reads the early side, 1.0044: 83.39 x 1.0044 + 80.69 = 164.4 reach against 444.9 of room.
- Today: `83.4 ms late   80.7 ms early` · `608.7 ms spare`
- After: `Hurts: [Kick - Yellow cymbal] 83.4 ms frontend then [Kick] 80.7 ms backend` · `safe by 280.4 ms`

**Burnout, activation 1** (`squeeze out 163.0 ms`). 84.52 x 0.9946 + 84.33 = 168.4 reach against 163.0 needed.
- Today: `84.5 ms early   84.3 ms late` · `5.4 ms spare`
- After: `Helps: [Kick - Green cymbal] 84.5 ms frontend then [Yellow cymbal] 84.3 ms backend` · `5.4 ms spare`

**Focus, activation 2** (`squeeze in 25.0 ms`). 61.9 + 48.1 = 110.1 reach against 25.0 needed.
- Today: `61.9 ms late   48.1 ms early` · `85.1 ms spare`
- After: `Helps: [Kick] 61.9 ms backend then [Blue tom] 48.1 ms frontend` · `85.1 ms spare`

**Tapped Out, activation 3** (`early fill 0.0 ms`, required). The 0.0 is from the old mockup, not from the figures file (P14).
- Today: `82.9 ms early` · `82.9 ms spare`
- After: `Helps: [Kick - Green cymbal] 82.9 ms frontend` · `82.9 ms spare`

The figures file holds no early fill with time to spare, so the mockup has no `Hurts:` early fill. It would read `Hurts: [note] N ms backend` with `safe by` or `can break by`. T2 finds one in the corpus and pins it.

The mockup's note names assume Pro Drums on and a plain kick (P21). I could not click the mockup in a browser myself: headless Edge returned nothing on two tries. The main session clicks it before the user sees it (T5).

## Tasks

T1 goes first. T2 and T3 start together once T1 is merged. T4 follows T2. T5 is the main session's. No agent builds or runs `hydra_tests`, `hydra_uitest` or `hydra_replay`; the main session builds each branch and runs the named tests.

### T1: the core owner

Owned files: `src/core/note_window.h`, `src/core/note_window.cpp`, `tests/test_note_window.cpp`.

Add `PressWay`, `squeeze_ways`, `early_fill_way`, `squeeze_scale`, `anchor_scale`, `squeeze_presses` and `early_fill_presses` as the design says. `ChordTimes` gains the chord at each place, filled by `chord_times` from `Song::sequence`. Leave the old row functions in place; T2 removes them, so T1 builds on its own.

Acceptance criteria, each a test in `tests/test_note_window.cpp` that the main session runs with `hydra_tests.exe "-tc=note window*"`:

- The four squeeze cases and the two early fill cases each return the directions and the helps or hurts answer the design names. The test builds each case at a figure of +0.001, 0.0 and -0.001, so it fails if the branch reads anything but `needs_no_timing` or `early_fill_needs_timing`.
- `anchor_scale` returns the early side for an early press and the late side for a late one, on a scale whose sides differ.
- A squeeze in that needs no timing, on a scale of early 1.0044 and late 0.9970, reads 1.0044. This is the Dead on Arrival case; the test fails on today's rule.
- The spare is "reach minus needed" when the presses help and "room minus reach" when they hurt, pinned as literals from one run. Exactly 0 reads the good word in both.
- On every squeeze of every path in `testdata/input/common`, the anchor's tick is before the squeezed note's tick (P9). If this fails, stop and report; the order rule needs a new decision.
- On every corpus squeeze whose offset is not exactly 0, the side `anchor_scale` picks is the side `rate_activation` marks as read. The test logs how many corpus squeeze-ins sit at exactly 0 (question 2).

### T2: the sentence and the row

Waits for: T1 merged.

Owned files: `src/app/path_view.h`, `src/app/path_view.cpp`, `src/ui/paths_tab.cpp`, `src/ui/badge_pill.h`, `tests/test_activation_rows.cpp`, `tests/test_badge_pill.cpp`, `tests/ui/uitest_paths.cpp`, and the removal of the old row functions and their tests from `src/core/note_window.h`, `src/core/note_window.cpp` and `tests/test_note_window.cpp`.

The sentence and the row read the owner as "What each caller changes" says. The row view gains the lead word, each note's name and the joining word. The tag gains its two new wordings. Remove `squeeze_out_window`, `squeeze_in_window`, `early_fill_window`, `WindowRow`, `squeeze_spare_ms` and `early_fill_spare_ms` once nothing calls them.

Acceptance criteria. The main session runs `hydra_tests.exe "-tc=*activation row*"`, `"-tc=note window*"`, `"-tc=*badge*"` and the Paths uitest; the executor confirms the exact case names when it reads the files.

- The sentence text of every corpus activation is the same before and after. The existing sentence tests pin this; none is edited.
- Burnout activation 1 and Focus activation 2 (both in the corpus) print the "After" rows above, pinned as literals.
- A corpus squeeze that needs no timing prints `Hurts:`, the mirrored words and a `safe by` or `can break by` tag. The row's squeezed-note direction and the sentence's word are the same owner answer (`backend` on the row where the sentence says "late"). The test fails if either is picked by kind.
- A corpus early fill with time to spare prints `Hurts: … backend`. If the corpus has none, stop and report.
- Every direction word on a row is `frontend` or `backend`, and both come from the one function that words the owner's direction. The test reads every corpus row and fails on "early" or "late" in a row's figures.
- Every squeeze in, squeeze out and early fill badge still has a row (ruling, P12): the count of rows equals the count of badges on every corpus path.
- Ratings, tiers and eff. figures are unchanged: the existing tests for them pass unedited.
- A grep for `"early"` and `"late"` string literals in `src/app/path_view.cpp` finds none outside the sentence's fixed phrases built from the owner's answer.

### T3: the schedule tool

Waits for: T1 merged.

Owned files: `tools/replay_schedule.h`, `tools/replay_schedule.cpp`, `tests/test_replay_schedule.cpp`.

Read the four tests P17 and P18 name first, and report which pin timed-squeeze output. Then remove `moving_side` and route the tool's by-kind choices through the owner, as "What each caller changes" says.

Acceptance criteria, run by the main session with `hydra_tests.exe "-tc=schedule:*"`:

- Every existing schedule test passes with no expected value edited. That includes "schedule: Freak 1144, Rescued 561 and 931, Ghost 756" and "schedule: a squeeze that needs no timing is pressed plain".
- `moving_side` is gone, and a grep of `tools/replay_schedule.cpp` finds no `SqueezeKind::SqOut ? … early` choice left.
- One new test feeds the tool a timed squeeze out and a timed squeeze in on a scale whose sides differ, and checks the ratio it uses equals `anchor_scale` of the owner's anchor direction.

### T4: the pages

Waits for: T2 merged.

Owned files: `docs/UserGuide.md`, `CONTEXT.md`, `docs/clone-hero/03-timing.md`, `docs/clone-hero/04-fills.md`, and the removal of this plan and both mockup folders in the same merge.

`docs/UserGuide.md` describes the row near line 273 (P16). Rewrite that passage for the new row in the public doc style, with every label checked against the code. It is a public page, so it goes past a doc reviewer. Update the other three pages where they describe the row. The 3.0.0 release notes are history and stay.

Acceptance criteria: the main session runs `hydra_tests.exe "-tc=docs match code*"`, and the reviewer checks each quoted label against `src/app/path_view.cpp`.

### T5: the main session's screen checks

Before the user sees the mockup, open it and click every activation header and every "Collapse all" button. After T2 is built, open Satellites, Dead on Arrival (O) and Tapped Out by exact title with the `hydra` tools and read activations 2, 3 and 5, activation 3, and activation 3. Compare each row with "What the user will see". This settles P13, P14 and P21. A mismatch is reported to the user, not patched.

## Out of scope

These are left out on purpose and need the user's nod.

- **An anchor shared by two timings.** One anchor can serve a squeeze in and a squeeze out, which need opposite presses. Each row treats its timing alone.
- **Judge lag.** The game judges a press about one frame after it is sent. The tool counts that; the row does not.
- **Where the anchor is.** The row names the anchor's chord, not its measure.
- **`rate_activation`'s own read of the stored scales.** It keeps reading the stored fields directly.
- **The 3.0.0 release notes.** They are not rewritten.

## Promise map

| Promise | Plan section | Task | Check |
|---|---|---|---|
| One core function owns direction, side and spare, branching on `needs_no_timing` | The design, One owner | T1 | the +0.001 / 0.0 / -0.001 cases in `note window*` |
| The scale side follows the anchor's press | The scale side | T1 | the `anchor_scale` test and the Dead on Arrival case |
| The spare has one meaning; 0 reads the good word | The spare | T1 | the pinned spare literals and the 0 edge test |
| Song order is the row's order | What the row says | T1 | the corpus "anchor before squeezed" test |
| Away from 0 the row's side equals the eff. figure's side | The scale side | T1 | the corpus comparison with `rate_activation` |
| The sentence reads the owner and its text does not change | What each caller changes | T2 | existing sentence tests, unedited |
| The row names each note in `Chord::rowstr` words | What the row says | T2 | the pinned Burnout and Focus rows |
| A no-timing row reads `Hurts:` with `safe by` / `can break by`, and agrees with its sentence | What the row says | T2 | the corpus no-timing row test |
| An early fill with time to spare reads `Hurts: … backend` | What the user will see | T2 | the corpus early fill test |
| The row's direction words are `frontend` and `backend`, worded in one place | Background; What the row says | T2 | the corpus test that fails on "early" or "late" in a row's figures |
| Every badge keeps its row | One owner | T2 | rows equal badges on every corpus path |
| Ratings, tiers and eff. figures do not change | What the user will see | T2 | their existing tests, unedited |
| The tool's output on timed squeezes does not change | What each caller changes | T3 | every `schedule:*` test, no expected value edited |
| `moving_side` and the tool's by-kind direction go | What each caller changes | T3 | the grep and the new ratio test |
| The public page matches the new row | T4 | T4 | doc review and `docs match code*` |
| The plan and mockups leave the repo with the work | T4 | T4 | `docs match code*` |
| The mockup is clicked before the user sees it | What the user will see | T5 | the main session's click-through |
| The real screens match the plan's rows | What the user will see | T5 | the main session's read of three songs |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: a squeeze needs no timing exactly when its figure is below 0 | code | `src/core/model.h:411` and `src/core/model.h:332` | checked 2026-11-13 |
| P2: the badge figure is `offset_ms` for a squeeze in and its negation for a squeeze out; the sentence prints the size of `timing()` | code | `src/core/model.h:271`, `src/core/model.h:396`, `src/app/path_view.cpp:106` | checked 2026-11-13 |
| P3: the sentence picks its direction word from `needs_no_timing` | code | `src/app/path_view.cpp:113` and `src/app/path_view.cpp:135` | checked 2026-11-13 |
| P4: the row's direction words are fixed by kind, and the early fill's is fixed "early" | code | `src/app/path_view.cpp:223` to `:227` and `:239` | checked 2026-11-13 |
| P5: the row's scale side is fixed by kind, its spare is windows minus the figure, and exactly 0 reads "spare" | code | `src/core/note_window.cpp:104` to `:116`, `:98`, `:65`, `:71` | checked 2026-11-13 |
| P6: the tool picks side, aim sign, word and scale by kind, and skips squeezes that need no timing | code | `tools/replay_schedule.cpp:127`, `:177`, `:216`, `:631`, `:632` | checked 2026-11-13 |
| P7: a scale's early side is for an early anchor press and its late side for a late one | code | `src/core/model.h:371` to `:381` | checked 2026-11-13 |
| P8: a squeeze's anchor is the latest overfill at or before its step, else the activation; its squeezed note is the squeeze-out chord or the squeeze-in step's chord | code | `src/core/model.cpp:758` and `src/core/model.cpp:786` | checked 2026-11-13 |
| P9: the anchor is always played before the squeezed note | unchecked | `src/core/model.cpp:780` gives the anchor's tick but does not compare it with the squeezed note's | T1 |
| P10: the early fill's note is the second bar arrival, and its figure is the negated `e_offset` | code | `src/core/model.cpp:695`, `src/core/model.cpp:477`, `src/core/model.h:327` | checked 2026-11-13 |
| P11: the eff. figure reads the early side when the offset is 0 or below, else the late side | code | `src/core/squeeze_rating.cpp:222`, `src/core/squeeze_rating.cpp:143`, `src/core/backend_value.h:28` | checked 2026-11-13 |
| P12: every squeeze in, squeeze out and early fill badge gets the row; it shows alongside the fixed 85 with at most 8 words of new info | ruling | `docs/agents/rulings/ui-and-display.md` lines 5 and 7 | checked 2026-11-13 |
| P13: the sides, offsets, scales and gems of the seven panels | unchecked | `dhw_figures.json` in the session scratchpad, a tool's report read by the main session | T2 pins Burnout and Focus; T5 reads the others |
| P14: Tapped Out activation 3 is a required early fill of 0.0 ms | unchecked | `docs/superpowers/specs/2026-11-12-dynamic-hit-window-mockup/index.html` line 185, another agent's mockup | T5 |
| P15: the early fill row ships while the game's early fill rule (FL-24) is unproven | ruling | `docs/agents/rulings/ui-and-display.md` line 6, answer 1 | checked 2026-11-13 |
| P16: the User Guide describes the row | code | `docs/UserGuide.md:273` to `:277`; the label is there, the passage itself was not read | checked 2026-11-13 |
| P17: one test pins four timed squeeze-outs' aims and rooms to the cent | code | `tests/test_replay_schedule.cpp:473` | checked 2026-11-13 |
| P18: three more tests pin the tool's timed output (the Freak split, the squeeze-in mirror, "the same schedule as before") | unchecked | `tests/test_replay_schedule.cpp:419`, `:773`, `:949`; names read, bodies not | T3 |
| P19: `Chord::rowstr` joins each note's label with " - " in brackets; a label is "Kick", "Red snare", "Blue tom" or "Yellow cymbal" with Pro Drums on | code | `src/core/model.cpp:192` and `src/core/model.cpp:61` | checked 2026-11-13 |
| P20: every chord in a song carries its `Chord` | code | `src/core/song.h:30` | checked 2026-11-13 |
| P21: the mockup's songs were analysed with Pro Drums on and their kicks are not 2x | unchecked | `dhw_figures.json` holds each gem's colour and cymbal flag only | T5 |
| P22: the three commits exist with the dates the background gives | code | `git show -s c68982cf 9d5bc5d5 970cb916` | checked 2026-11-13 |
| P23: the row's words change: "i think early and late are the issue here causing confusion. change the display text to frontend and backend" | ruling | the user's message of 2026-11-13 as the main session quoted it to the planner (session `7d28ae9a`); not yet in `docs/agents/rulings/ui-and-display.md`, where the main session adds it | checked 2026-11-13 |
| P24: `CONTEXT.md` already defines Frontend squeeze, Backend squeeze and Backend row, and says "backend" names a scoring effect, a table row and a squeeze (D115) | code | `CONTEXT.md:570` to `:583` | checked 2026-11-13 |

## Questions

1. **The row cannot name both notes in 8 words. May it be longer?** Your ruling was "a maximum of 8 words for the new info". Today's row is 6 words plus a 3-word tag. With each note named in full, the shortest row in the mockup is 11 words (`Helps: [Kick] 61.9 ms backend then [Blue tom] 48.1 ms frontend`, Focus). The longest is 14 (`Helps: [Kick - Green cymbal] 84.5 ms frontend then [Yellow cymbal] 84.3 ms backend`, Burnout), and a three-note anchor adds two more. The choices: (a) the longer row as mocked, which wraps to a second line in a narrow panel; (b) name each note in the Backend table's short form, `[K  B ]`, which gives 10 words on every row; (c) keep 8 words and leave the notes unnamed, which leaves fault 4 unfixed. I recommend (a), because the full words match the "Chord" line just above. The case against it: the row gets about twice as long.

2. **A squeeze in at exactly 0.0 ms: which scale side does the row read?** Two rules meet here. The eff. figure reads the early side when `offset_ms <= 0.0` (`paid_by_sp_walk`). The row would read the late side when `difficulty() >= 0.0` (`timing_needs_hitting`), because a 0.0 ms squeeze in is still a timing to hit and its anchor is pressed late. At +0.001 both read late. At -0.001 both read early. At exactly 0.0 the eff. figure reads early and the row reads late, in the same box. The choices: (a) the row follows the direction rule and reads late; (b) the row copies the eff. figure and reads early. I recommend (a): the row then never names a direction that disagrees with its own scale side. The case against it: two figures in one box use different sides on that one edge. T1 counts how many corpus squeeze-ins sit at exactly 0, so you will know if this ever shows.

3. **Does "change the display text to frontend and backend" cover the squeeze sentence and the early fill text too, or only the row?** Your words name "the display text". The plan and mockup change the row only, which may be narrower than you meant. The sentence picks "early" or "late" by the same rule as the row (`needs_no_timing()`). Examples from both ends, Satellites activation 2 (needs timing) and activation 3 (needs none):
   - (a) Row only. The sentences stay "Hit the [ R   ] note more than 83.3 ms late so it lands after Star Power ends." and "Hit the SP phrase's last note no more than 83.3 ms late so it lands before Star Power ends." The row under each says `backend` where its sentence says "late".
   - (b) The row and the squeeze sentences. With the words swapped one for one, they would read "Hit the [ R   ] note more than 83.3 ms backend so it lands after Star Power ends." and "Hit the SP phrase's last note no more than 83.3 ms backend so it lands before Star Power ends." That reads badly, so (b) would get its own mockup of the sentence before any build.
   - (c) Everything, including the name "early fill". Today the badge reads `early fill 0.0 ms` and the line `Early fill: 0.0 ms (required)`. The line holds no direction word, only the name. Renaming it needs a word from you; I have not proposed one.
   I make no recommendation here. The case for (a): the sentence is plain English and was not the text you were looking at. The case against (a): one box then uses two vocabularies for the same press.

4. **Is `backend` on the row acceptable next to the "Backend timings" table?** The predicate is the owner's press direction: a late press prints `backend`, an early one `frontend`. In Hydra's glossary a backend squeeze is "Hitting a note near the SP end early", and the table below the row is "Backend timings". Examples from both ends: on Satellites activation 2 (squeeze out) the SP-end note reads `[Red snare] 53.1 ms backend`, which matches the table's sense by luck. On Satellites activation 5 (squeeze in) the SP-end note reads `[Red snare] 53.1 ms frontend` and the activation chord reads `85.7 ms backend`, which is the reverse of the glossary's sense. The choices: (a) use the two words as ruled and add one line to the User Guide and `CONTEXT.md` saying what they mean on this row; (b) pick other words. I make no recommendation; this is your wording.
