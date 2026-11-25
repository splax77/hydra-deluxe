# One multiplier: one owner for the frontend scale, and the Effective line

Date: 2026-11-24. Status: waiting for the user's yes. Nothing here is built.

## The short version

The user asked: "create a plan to make one function that decides what a multiplier is and when to show it and add this effective thing you mocked up too".
Today two places decide which side of a squeeze's multiplier counts. The safe badge asks `squeeze_presses`. The scale line and the "eff." figure ask `rate_note`, which picks by where the note sits. They disagree for one case: a squeeze in at exactly 0.0 ms. The badge takes the late side and the scale line takes the early side. On Bloom the late side is x0.40 and the early side is x1.00, so the badge uses a multiplier the screen never shows.

The plan makes one owner in `src/core/squeeze_rating`. It answers three things for a squeeze or a backend row: which stored multiplier belongs to it, which side of it counts, and whether that side is worth showing. Its side rule for a squeeze is the badge's, `squeeze_presses`. The badge, the Dynamic Hit Window row, the scale lines, the eff. figures, the Rating words and the `hydra_replay schedule` tool all call it. The scale lines then print exactly the sides the owner handed out.

The plan also adds the mockups' one line, `Effective: frontend N ms`, under the Dynamic Hit Window line. N is the frontend note's window times the multiplier the badge used. The engine already works that product out inside the safe sum and throws it away. A task carries it out, so the screen computes nothing.

The user settled the 0.0 ms side already. Offered the choice between the badge's side and the scale line's side, the user answered: "why are you even offering an option that would make the display wrong?" So the badge's side wins and the plan does not ask again.

## What you will see (rules 11 and 12)

None of the four charts below is in `testdata/input`; the brief named them, and they are in the user's library. Each "before" line is the text the brief gave from the user's screen. Each "after" figure that the code could not give me is marked with its Premise map row, and a live step reads it from the real app.

Only one screen reads the changed values: the opened activation row on the Paths tab. `rate_activation` has one caller outside tests, `build_activations` in `src/app/path_view.cpp` (P13). The safe-paths search and the schedule tool read the safe sum, which does not move (P5).

**Bloom** (Between the Buried and Me, charted by Grinnz), Expert drums, third path `3 5+ 0`, activation 2. This is the case that is wrong today.

| Line | Before | After |
|---|---|---|
| Safe tag | 103.6 ms safe | 103.6 ms safe |
| Sentence | Hit the SP phrase's last note more than 0.0 ms early so it lands before Star Power ends. | Hit the SP phrase's last note more than 0.0 ms early (eff. 0.0 ms) so it lands before Star Power ends. (see "The eff. figure on Bloom") |
| Dynamic Hit Window | frontend 74.0 ms [ RY  ] backend 74.0 ms [ RY  ] | the same |
| Effective | no line | Effective: frontend 29.6 ms (P16) |
| SqIn scale line | no line | Frontend timing scales x0.40 (late) at the SqIn's SP end. |
| SP end scale line | Frontend timing scales x2.25 (early) / x0.90 (late) at the SP end. | the same |

**Gethsemane** (Rise Against), optimal path, activation 3. The badge "squeeze out 220.6 ms", the tag "41.7 ms safe", "frontend 85.7 ms", "backend 78.3 ms", "Insane+ (eff. 140.2 ms)" and "Frontend timing scales x2.15 (early) at the SP end." all stay. One line is new: "Effective: frontend 184.0 ms" (P17).

**Waste of Air** (Leprous), optimal path `1 0+ 3 1+- 2`, activation 4. Both boxes keep every line they have. The squeeze in's box gains "Effective: frontend 172.5 ms" and the squeeze out's box gains "Effective: frontend 80.5 ms" (P18).

**Subdivisions** (Rush), optimal path `2 4+ 0 3 0`, activation 2. The badge "squeeze in -338.4 ms", "497.2 ms safe", "frontend 78.2 ms", "backend 78.6 ms", "(eff. 334.0 ms)" and "x1.03 (early) at the SqIn's SP end" all stay. One line is new: "Effective: frontend 80.2 ms" (P19).

**Every other squeeze.** A squeeze whose priced side is x1.00 gets no Effective line, the same as it gets no scale line today (see "When the Effective line shows"). No Rating word and no backend row's eff. figure moves on any chart, because every backend row keeps the side it reads today (P7, P8). No squeeze out changes at all. A squeeze in changes only at exactly 0.0 ms, where it now reads the late side like its badge.

## When the Effective line shows

It shows exactly when the owner says the priced side is a scale, and never at x1.00. Three things settle this, so it is not a question. The user's request is for "one function that decides what a multiplier is and when to show it", so the Effective line takes its "when" from the same owner as the scale line. That owner's existing test is `is_scaled` (owner map O6), whose own words are "A multiplier counts as no scale when it is less than the tolerance away from 1 ... The user chose a thousandth" (D256). The user's ruling D116, "if a multiplier exists there should be a corresponding display", is about a multiplier that exists, and a x1.00 side is no multiplier by D256. All three mockups show a scaled multiplier and none shows a x1.00 line. At x1.00 the line would only repeat the figure on the Dynamic Hit Window line above it.

An early fill's row has no multiplier, so it never gets the line.

## The eff. figure on Bloom

Reading the late side gives Bloom's squeeze in a scaled multiplier, so the existing rule prints its eff. figure and turns the scale lines orange: "Hit the SP phrase's last note more than 0.0 ms early (eff. 0.0 ms) so it lands before Star Power ends." This is not a question. D116 (`docs/agents/rulings/ui-and-display.md:80`) already says a multiplier that exists gets its figure: "if a multiplier exists there should be a corresponding display". The figure is 0.0 ms because the squeeze sits exactly on the end (P9). Whether the lines are orange today is P15, read by LS4.

## What stays as the mockup has it

The words "Effective" and "eff." mean two things. The new line is in real ms, and the "eff. N ms" figures are shrunk onto the normal 170.5 ms budget. The user asked to "add this effective thing you mocked up", so the plan builds the mockup's words and changes no "eff." figure. The wording is not a question for the user, who can ask for a rename later.

## Owner map

| Question | Owner | Its own words | Status |
|---|---|---|---|
| O1: Which side of the multiplier (early or late) prices a squeeze in's or a squeeze out's frontend note, and which side does the screen's scale line and "eff." figure read for the same squeeze? | (a) The badge, the Dynamic Hit Window row, `safe_ms` and the schedule tool: `squeeze_presses` and `transfer_side`, `src/core/note_window.cpp:79-88`, used at `:128`. It picks by kind plus `needs_no_timing()`. (b) The scale line and the SqIn eff. figure: `rate_note` (`src/core/squeeze_rating.cpp:140-147`), called for a SqIn at `:222` with `core::paid_by_sp_walk(sq.offset_ms)`. It picks early when offset <= 0. | (a) `note_window.h:92-94`: "This is the one owner of which way each goes." Its comment at `note_window.cpp:77`: "The squeeze sentence ... prints its 'early' or 'late' from this answer, and replay_schedule presses the anchor from it." (b) `squeeze_rating.h:143-148`: "The side of the end it sits on decides which activation-hit direction moves the end across it ... the squeeze kind never enters." | `split`. They agree for every squeeze except a SqIn at exactly 0.0 ms. At 0.0 ms, (a) says late, (b) says early. Bloom is that case. The badge uses the late side (103.6 ms safe); the scale line reads the early side, 1.0, which `is_scaled` rejects, so no line and no eff. figure. |
| O2: Which side does each Backend timings row read? | `rate_activation`, `src/core/squeeze_rating.cpp:184-190`. A squeezed-out row uses `core::paid_by_sp_walk(o)` (`backend_value.h:28`). A plain row uses `core::counted_without_squeeze(o, leeway)` (`backend_value.h:39`, `rate_note` called at `:189`). Separately, the SqOut's badge reads the same SqOut via `squeeze_presses` (O1 (a)). | `backend_value.h:26`: "The SP walk pays every chord at or before the SP end". `:36`: "Counted under Star Power with no squeeze: at or before the SP end, or less than the leeway ... after it." `squeeze_rating.cpp:182`: "inside: whether the note is inside SP on this path, by the counting rule that owns it". | `split`. A plain row 0 to 3 ms past the end reads the early side through the leeway rule. A SqIn reads `paid_by_sp_walk`. The SqOut row and the SqOut badge reach the same side by two routes, and today they agree. |
| O3: What is the multiplier for an activation's SP end? | `step_transfer_scale` and `transfer_scale_between`, `src/core/squeeze_rating.cpp:12-30`, via `frontend_transfer_scales` (`:32-51`). Stored as `Activation::transfer_post` at `src/search/engine.cpp:469-470`. | `squeeze_rating.h:56-61`: "The one owner of a step's ratio; frontend_transfer_scales reads every stored scale through it." `model.h:739`: "Computed by the search at copy-out and stored". | `found` |
| O4: What is the multiplier for a squeeze in's own SP end? | The same functions; `frontend_transfer_scales` takes the step `act.squeeze_end_step(k)` and stores it as `SPSqueeze::transfer` (`engine.cpp:471-474`, `model.h:456-463`). | `model.h:456`: "A SqIn's frontend transfer scale: at squeeze_end_tick, measured from squeeze_anchor_tick. Stamped by the search at copy-out". CONTEXT.md "Transfer scale": "Each SqIn stores its own scale, at the SP end it was measured from". | `found` |
| O5: Which stored multiplier belongs to a given squeeze (its own, or the activation's)? | Four spellings of the same choice: `squeeze_window_ends` (`note_window.cpp:116-117`), `stored_transfer_scales` plus `sqin_rank` (`squeeze_rating.cpp:53-63`, `:220`), `tools/replay_schedule.cpp:415`, `tools/replay_schedule.cpp:619-620`. | `note_window.h:150-152`: "the stored scale is Activation::transfer_post for a squeeze out and the squeeze's own SPSqueeze::transfer for a squeeze in." `squeeze_rating.h:75-77`: "or nothing when any one is unknown: all or nothing". | `split` |
| O6: When does the scale line show a side, and what does it print? | `sides` lambda and callers, `src/app/path_view.cpp:266-312`. Shows a side only when `rate_activation` marked it read (`SidesRead`, `squeeze_rating.cpp:152`) and `is_scaled` holds (`squeeze_rating.h:126`). Text from `format_scale` (`path_view.cpp:28`). A SqIn clause is dropped when its text equals the SP-end line's text (`:301`). Orange when `scale_governs`. | `squeeze_rating.h:113-117`: "A multiplier counts as no scale when it is less than the tolerance away from 1 ... The user chose a thousandth". `path_view.cpp:261`: "The lines name the sides the activation's rows and squeezes read (rate_activation records them), when they aren't 1, early first." | `found`. Which sides count as read is O1 and O2. |
| O7: When is there no line, or an "unknown" line? | `rate.scales` unset gives "Transfer scale unknown." and orange (`path_view.cpp:275-279`). Otherwise no line when no read side is scaled (O6). | `path_view.cpp:276`: "D4: a guard that shows a bug. A test proves fresh records never reach it." | `found` |
| O8: What does the safe/short badge add up? | `squeeze_safe_ms(anchor_side, ratio, squeezed_side, needed)` = `anchor*ratio + squeezed - needed` (`note_window.cpp:67-69`), fed by `squeeze_window_ends` (`:113-130`). `safe_is_short` is `< 0` (`:73`). Tag text is `window_row_view` (`path_view.cpp:187-201`). | `note_window.h:75-76`: "Each sum is 'what the game's windows give, minus what the timing needs'." `:88-90`: the 2026-11-13 ruling, "xx ms safe >=0 ms, xx ms short <0ms". | `found` |
| O9: What are the "eff. N ms" figures in the backend table and in the SqIn sentence? | `effective_backend_ms(offset, r)` = `|offset| * 2 / (1 + r)` (`squeeze_rating.cpp:123-130`), set by `rate_note` only when `is_scaled(scale)` (`:145`). `eff_suffix` (`path_view.cpp:39`) prints it. SqIn figure on its sentence (`path_view.cpp:121-124`). SqOut figure on its squeezed-out backend row (`:400-401`) and the row tooltip (`:372-384`). Rating word banded on it in `BackendSqueeze::summarystr` (`model.cpp:362-376`). | `squeeze_rating.h:102-106`: "A backend squeeze's raw ms mapped onto the nominal budget the ratings assume". `NoteRating::effective_ms`: "set exactly when is_scaled(scale)". | `found`. The side it uses is O1 and O2. |
| O10: What is the frontend note's window after the multiplier (the mockup's `Effective: frontend N ms`)? | The engine works it out and keeps it nowhere. `squeeze_window_ends` computes `*anchor` and `transfer_side(*scale, squeeze_presses(q).anchor)` at `note_window.cpp:127-128` and folds them into `safe_ms`. `WindowEnds` (`note_window.h:142-146`) keeps only `ends` (each with an unscaled `side_ms`) and `safe_ms`. | `squeeze_safe_ms` at `note_window.h:78-81`: "the anchor's side moves the SP end by `ratio` per ms". The mockup figures fit `anchor_side * ratio` on the side `squeeze_presses` gives (Gethsemane 184.0, Waste of Air 172.5 and 80.5). | `carry out`: `squeeze_window_ends`, `src/core/note_window.cpp:113-130`. |
| O11: Does a note exactly on the SP end (0.0 ms) count as inside, or need timing? | "Inside Star Power?" is `core::paid_by_sp_walk` (offset <= 0, `backend_value.h:28`), with by-tick twin `after_sp_end` (`:34`). "Needs timing?" is `timing_needs_hitting` (difficulty >= 0, `model.h:371`), through `SPSqueeze::needs_no_timing` (`model.h:451`). | `model.h:368-370`: "A figure of 0.0 ms is still a timing to hit, not 'no timing' (D302)." `model.h:448-450`: "Which side of the SP end the note sits on is core::paid_by_sp_walk's question, not this one." Clone Hero SP-33, "Hydra assumes it, never checked". | `found` for each question. The conflict is only O1. |
| O12: Does the search or the path limit read the multiplier? | No. Difficulty and limits use raw gap ms: `squeeze_difficulty` and `timing_within_limit` (`model.h:302,330`). | `squeeze_rating.h:6-8`: "The search never reads the judgement side; difficulty and the ms filter stay raw gap ms". CONTEXT.md "Difficulty": "never scaled by the transfer scale." | `found` |

## How the splits merge

**O5, which stored multiplier.** One function, `stored_scale(act, k)`, returns the squeeze's stored scale: the activation's for a squeeze out, the squeeze's own for a squeeze in. The four spellings in the map all call it. `stored_transfer_scales` keeps its all-or-nothing job (O7) and builds its list through it.

**O1, which side for a squeeze.** One function, `squeeze_multiplier(act, k)`, returns the stored scale from `stored_scale`, the anchor's press from `squeeze_presses`, and the one number, `transfer_side` of the two. It is unset when the stored scale is unset. `squeeze_window_ends` prices the safe sum from it. `rate_activation` rates each squeeze in from it, in place of `paid_by_sp_walk`. The schedule tool presses the anchor from it. This is the badge's rule used as it is; nothing is added to it.

**O2, which side for a backend row.** One function, `row_multiplier(act, row, leeway)`. The squeezed-out row is its squeeze out's note, so it returns that squeeze's `squeeze_multiplier`. That is the merge: the row and its badge reach one side by one route. A plain row is not a squeeze and has no press, so the only question left for it is the engine's own, "is this row counted without a squeeze?", and `row_multiplier` asks `core::counted_without_squeeze` for it (`backend_value.h`, which owns that question) and reads the early side when it holds and the late side when it does not. `row_multiplier` adds no condition and no rule of its own to that answer. It changes nothing on screen (P7). `rate_activation` calls `row_multiplier` for every row, and keeps today's guard: when `stored_transfer_scales` is unset, nothing is asked of the owner and the "Transfer scale unknown." line stays (O7).

**When to show.** The owner's answer carries `shown()`, which is `is_scaled` of its one number (O6). The scale lines, the eff. figures and the Effective line all read it.

The owner lives in `src/core/squeeze_rating.h`, whose header already claims the job: "which scale directions matter, which multiplier governs a note". `squeeze_presses` stays in `note_window.h` as the owner of the press, and `transfer_side` stays there as the owner of "the number on this side of a scale"; the new functions call them. `tools/replay_schedule.cpp` also applies a press to scales that are not the squeeze's own (the read ends, `moving_side`), which is `transfer_side`'s own job and stays a call to it.

One header dependency has to run one way. `squeeze_rating.h` cannot include `note_window.h`, because `WindowEnds` in `note_window.h` holds a `PricedScale`. So the `Press` enum moves from `note_window.h` into `squeeze_rating.h`, and `note_window.h` includes `squeeze_rating.h`. Every file that sees `Press` today includes `note_window.h` and keeps seeing it. `squeeze_rating.cpp` includes `note_window.h` for `squeeze_presses`.

The exact shapes, so the code tasks can work side by side without a build:

```
// core/squeeze_rating.h
enum class Press { Early, Late };   // moved here from core/note_window.h
struct PricedScale {
    TransferScale stored;   // the squeeze's or row's stored scale, both sides
    Press press;            // the side that counts
    double ratio;           // the number on that side of `stored`
    bool shown() const;     // is_scaled(ratio)
};
const std::optional<TransferScale>& stored_scale(const Activation& act, size_t k);
std::optional<PricedScale> squeeze_multiplier(const Activation& act, size_t k);
std::optional<PricedScale> row_multiplier(const Activation& act, const BackendSqueeze& row, double leeway_ms);

// core/note_window.h (includes core/squeeze_rating.h; Press is no longer declared here)
double scaled_anchor_ms(double anchor_side_ms, double ratio);   // the product squeeze_safe_ms adds
struct WindowEnds { ...; std::optional<PricedScale> priced; std::optional<double> frontend_effective_ms; };

// app/path_view.h
struct WindowRowView { ...; std::optional<WindowFigure> effective; };   // word "frontend", figure "29.6 ms", no chord
inline constexpr const char* kEffectiveLabel = "Effective:";
```

## Plan run

Slug: one-multiplier
Repo: C:\Users\Patrick\Downloads\Hydra\hydra-test

| Phase | Builds |
|---|---|
| all | 1 |

## The tasks

Each task is one Opus executor, under 100 tool calls and 20 minutes. Agents never build. Every branch goes to its reviewer uncompiled, and the main session builds `plan/one-multiplier/all` once.

| Task | Branch | Phase | Owned files |
|---|---|---|---|
| M1 | `one-multiplier/m1-owner` | all | `src/core/squeeze_rating.h`, `src/core/squeeze_rating.cpp`, `tests/test_squeeze_rating.cpp` |
| M2 | `one-multiplier/m2-window` | all | `src/core/note_window.h`, `src/core/note_window.cpp`, `tests/test_note_window.cpp` |
| M3 | `one-multiplier/m3-view` | all | `src/app/path_view.h`, `src/app/path_view.cpp`, `tests/test_activation_rows.cpp` |
| M4 | `one-multiplier/m4-schedule` | all | `tools/replay_schedule.cpp`, `tools/replay_schedule.h`, `tests/test_replay_schedule.cpp` |
| M5 | `one-multiplier/m5-pages` | all | `docs/clone-hero/05-star-power.md`, `docs/audit/2026-10-03-fix-decisions.md`, `docs/UserGuide.md`, `CONTEXT.md` |
| M6 | `one-multiplier/m6-rule` | all | `tests/single_owner/rules/core.json` |
| M7 | `one-multiplier/m7-draw` | all | `src/ui/paths_tab.cpp`, `tests/ui/uitest_paths.cpp` |

### M1: The owner, and the rating reads it

Uses O1, O2, O3, O4, O5, O6, O9, O11.

Add `Press`, `PricedScale`, `stored_scale`, `squeeze_multiplier` and `row_multiplier` to `src/core/squeeze_rating`, in the shapes above. `rate_note` takes the owner's answer in place of its `inside` flag, so it no longer picks a side. `rate_activation` asks `row_multiplier` for each backend row and `squeeze_multiplier` for each squeeze, and marks `post_read` and `sqins_read` from those answers. A squeeze out marks its side on `post_read` too, so a priced side can never be missing from the line. `stored_transfer_scales` builds through `stored_scale` and keeps its all-or-nothing rule.

The test at `tests/test_squeeze_rating.cpp:780-810` pins the old side and is rewritten, not deleted. The direct `rate_note` calls in the same file (lines 587 to 623) take the new argument; their expected values stay as they are.

Acceptance criteria, each with its test in `tests/test_squeeze_rating.cpp`:

1. A squeeze in at exactly 0.0 ms reads the late side. The test builds it with only the late side scaled and requires a figure, then with only the early side scaled and requires none. It fails if `rate_activation` still asks `paid_by_sp_walk`.
2. For a squeeze in and a squeeze out at offsets just before, on and just past the end, the side `rate_activation` records equals `squeeze_multiplier(act, k).press`. The test compares the two engine answers and computes no side itself.
3. The squeezed-out row's rating equals its squeeze out's `squeeze_multiplier` at the same three offsets. It fails if the row and the squeeze ever take different routes again.
4. A plain row reads the early side when `counted_without_squeeze` holds and the late side when it does not, at an offset inside the leeway and one at it. The existing plain-row tests stay as they are; the reviewer confirms none of their expected values was edited.
5. `stored_scale` returns the activation's scale for a squeeze out and the squeeze's own for a squeeze in, and `squeeze_multiplier` is unset when that scale is unset.
6. `shown()` is true exactly when `is_scaled(ratio)` is.
7. With one SqIn's stored scale unset, `rate_activation` leaves `scales` unset and rates no row and no squeeze, as today.

### M2: The Dynamic Hit Window row is priced by the owner and keeps the product

Uses O1, O5, O8, O10.

`squeeze_window_ends` asks `squeeze_multiplier` in place of its own pick at lines 116 to 117 and 128. `squeeze_safe_ms` adds `scaled_anchor_ms(anchor_side_ms, ratio)` in place of its inline product, and `squeeze_window_ends` stores that same call's result as `WindowEnds::frontend_effective_ms`, beside the owner's answer in `WindowEnds::priced`. An early fill's row leaves both unset. `note_window.h` drops the `Press` declaration and includes `squeeze_rating.h`, which M1 gives it (see "How the splits merge"); `transfer_side` and `squeeze_presses` stay.

Acceptance criteria, each with its test in `tests/test_note_window.cpp`:

1. On the file's existing hand-built squeezes, a squeeze in and a squeeze out (its `t.act` cases), `safe_ms` equals `squeeze_safe_ms` of the frontend end's `side_ms`, `priced->ratio`, the backend end's `side_ms` and the squeeze's `difficulty()`. It fails if the stored answer is not the one the sum used.
2. `frontend_effective_ms` equals `scaled_anchor_ms(frontend side_ms, priced->ratio)` on the same squeezes, and `safe_ms` equals `frontend_effective_ms` plus the backend side less the difficulty.
3. No existing expected `safe_ms` in the file changes. The reviewer reads the diff and says so; a changed one means the badge moved, which the user ruled out.
4. `early_fill_row` returns a row with `priced` and `frontend_effective_ms` unset.

### M3: The view builds the Effective text and the scale lines read the owner

Uses O6, O7, O9, O10. Rests on P21.

In `src/app/path_view.cpp` the scale lines keep their wording and read `post_read` and `sqins_read` as M1 fills them. `WindowRowView` gains `effective`, an optional `WindowFigure` built in `window_row_view` from the row's `frontend_effective_ms`: its word is `role_word(EndRole::Frontend)`, its figure is `format_ms` of the number and its chord is empty. It is unset unless `priced->shown()`. `kEffectiveLabel` (`Effective:`) is one constant beside `kDynamicHitWindowLabel` in `src/app/path_view.h`. The comment in `squeeze_sentences` that calls the eff. side "a different question" is corrected to name the owner.

Acceptance criteria, each in `tests/test_activation_rows.cpp`:

1. On a real chart's squeeze whose `priced->shown()` is true, the view's `effective` holds the word `frontend` and `format_ms` of the engine's `frontend_effective_ms`. The expected figure is read from `squeeze_window_ends`, never worked out in the test.
2. On a real chart's squeeze whose `priced->shown()` is false, the view's `effective` is unset. So is an early fill's.
3. For every activation of a real chart, a side appears on a scale line exactly when the owner handed that side out and it is `shown()`. It fails if the line and the owner disagree in either direction.

The test finds its charts by scanning the fast-tempo charts under `testdata/input/test_fast_tempo` (read the way `late_sqin_twice()` reads one) and the corpus songs, for the first squeeze with `shown()` true and one with it false. If the scan finds no scaled squeeze, the executor stops and reports it; it does not hand-build one in place of a real chart.

### M7: The box draws the Effective line

Uses O6, O10.

`window_row` in `src/ui/paths_tab.cpp` draws `WindowRowView::effective`, when it is set, on its own line under the Dynamic Hit Window line, as the mockups show: `kEffectiveLabel` in the box's text colour, then the word in the row's word colour and the figure in its figure colour. It takes the word and the figure as the view hands them out and builds no text of its own.

Acceptance criterion, in `tests/ui/uitest_paths.cpp`: a headless test opens a corpus song whose squeeze has `shown()` true (found by the same scan as M3's, at test time; P21) and requires the Effective line on screen directly after the Dynamic Hit Window line of the same box, and none in an early fill's box. If no corpus song has such a squeeze, the executor stops and reports it.

### M4: The schedule tool asks the owner

Uses O1, O5.

`tools/replay_schedule.cpp` picks a squeeze's stored scale at lines 415 and 619 to 620, and the anchor's press at line 626. The scale pick becomes a call to `stored_scale` and the press a read of `squeeze_multiplier(act, k)->press`. `moving_side` stays a call to `transfer_side`: the tool applies a press to the read ends' scales too (lines 224, 749 and 766), which are not the squeeze's own, and "the number on this side of a scale" is `transfer_side`'s own question.

Acceptance criteria, in `tests/test_replay_schedule.cpp`:

1. For each squeeze of the test's existing chart, the schedule's `ratio` equals `stored_scale` and its anchor press equals `squeeze_multiplier(act, k).press`.
2. No existing expected value in the file changes; the reviewer reads the diff and says so. The tool's output is the same on every chart, because the tool already used the badge's rule.

### M5: The pages

The user's answer to question 1 comes with the go for the plan, so the dispatch brief carries it. If the answer is no, `CONTEXT.md` is not edited and the other three files go ahead.

`docs/clone-hero/05-star-power.md`, block SP-39: its last Rule sentence, "The scale's direction follows the sign of the difficulty, not the squeeze kind", does not match the code. It is corrected to point at the owner, and its `Hydra:` line names the new functions. `docs/audit/2026-10-03-fix-decisions.md` gets a new entry that quotes the user's answer of 2026-11-24 and says D302 item 2's sentence "The rating keeps reading the note's side from paid_by_sp_walk" no longer holds for a squeeze. `docs/UserGuide.md` gets one sentence on the Effective line, in the public doc style. `CONTEXT.md`, "Needs no timing", takes the wording the user approves in question 1.

Acceptance criteria: the reviewer checks every backticked name on SP-39 exists in the code (the Clone Hero README's first check), that the decision entry's quote is the user's exact words, and that `CONTEXT.md` holds the approved wording and nothing else changed in it. `docs/UserGuide.md` goes through the doc review gate (`docs/agents/doc-review.md`).

### M6: The single-owner rule

Uses O5.

Add one rule to `tests/single_owner/rules/core.json`, in the shape its neighbours have (`engine.cpp:93-130` lists the keys: `question`, `owner`, `owner_file`, `owner_symbol`, `date`, `decision`, `added_by`, `pattern`, `anchor`, `must_match`, `must_not_match`, `owner_lines`, `scope`, `note`). Its question is "Which stored transfer scale belongs to this squeeze?", its owner `stored_scale` in `src/core/squeeze_rating.cpp`, its `scope` `src` and `tools`, and its `decision` the user's words of 2026-11-24, "why are you even offering an option that would make the display wrong?", with this plan's path. Its pattern matches a pick between `transfer_post` and `SPSqueeze::transfer`, such as `kind == SqueezeKind::SqOut ? act.transfer_post : q.transfer` and `is_in ? q.transfer : act.transfer_post`, and its `owner_lines` list the owner's one line. Reads of `transfer_post` that pick nothing (the search storing it, `tools/replay_json.cpp` printing it) do not match; `must_not_match` pins two of them. `transfer_side` calls are not covered: `moving_side` in the schedule tool legitimately calls it.

Acceptance criteria:

1. Each `must_match` line, spelled as the four old spellings in `note_window.cpp`, `squeeze_rating.cpp` and `replay_schedule.cpp` were, matches the pattern, and each `must_not_match` line does not.
2. The rule finds nothing in `src` or `tools` once M1, M2 and M4 have landed, so no `baseline.json` entry is added. The executor checks this by reading the files, because it cannot run the scan; the main session's build runs it.

## The GUI checks

M7 changes text and adds a line to a box, so it needs two checks from `docs/agents/ui-testing.md`. Case 1, behaviour and text, is M3's tests and M7's headless test, run by the main session after the phase's one build. Case 2, what the user sees, is the live steps below: the new line is a layout change, so the real dev app is opened on a copy of the user's library and the captures are shown to the user before `main` moves.

## The live steps

The main session runs these with the `hydra` MCP server (P22). It runs them twice. The first run is on the current build, before any task starts, to read the "before" text. It starts the window with `hydra_start("window", keep=True)`. The second run is on the plan branch's build, after the main session built it, and starts with `hydra_start("window", keep=True, worktree=<the plan branch's worktree folder>)`, so the real-app record names that branch. The steps only read the screen. The result of a run is each step's reply text and its screenshots, kept in the helper's scratch folder because of `keep=True`; the main session shows both to the user, and none of it blocks a task. Titles go to `hydra_open_song` exactly: "Bloom", "Gethsemane", "Waste of Air", "Subdivisions".

### LS1: Start the real window

Calls: `hydra_start` (never tried)

Pass: the reply names a running window on a copy of the user's library. Stop after one try and report the reply if it does not.

### LS2: Open Bloom

Calls: `hydra_open_song` (never tried)

Pass: the song's tabs settle. This step starts only after LS1 passed.

### LS3a: Read the Paths tab's widget labels

Calls: `hydra_dump` (never tried)

Pass: the reply, narrowed with `contains` to the path rows and the `##act` ids (the row id is `##act<number>`, `src/app/path_view.h`), names the label of Bloom's third path and of its activation 2.

### LS3b: Open Bloom's third path and activation 2

Calls: `hydra_click` (never tried)

Pass: activation 2's row is open on path `3 5+ 0`.

### LS4: Read Bloom's box

Calls: `hydra_text` (never tried)

Pass, after the build: the box holds every "after" line of the Bloom table above, the tag still reads "103.6 ms safe", and the Effective figure settles P16. Before the build it settles P15 (whether the SP end line is orange today).

### LS5: Capture Bloom's box

Calls: `hydra_screenshot` (never tried)

Pass: a capture of the open box exists and is shown to the user.

### LS6: Open Gethsemane

Calls: `hydra_open_song` (seen: LS2's passing run)

Pass: the song's tabs settle.

### LS7: Read Gethsemane's activation 3

Calls: `hydra_click` (seen: LS3b's passing run), `hydra_text` (seen: LS4's passing run), `hydra_screenshot` (seen: LS5's passing run)

Pass: every line the brief quoted is unchanged and the new line settles P17.

### LS8: Open Waste of Air

Calls: `hydra_open_song` (seen: LS2's passing run)

Pass: the song's tabs settle.

### LS9: Read Waste of Air's activation 4

Calls: `hydra_click` (seen: LS3b's passing run), `hydra_text` (seen: LS4's passing run), `hydra_screenshot` (seen: LS5's passing run)

Pass: both boxes keep their lines and the two new lines settle P18.

### LS10: Open Subdivisions

Calls: `hydra_open_song` (seen: LS2's passing run)

Pass: the song's tabs settle.

### LS11: Read Subdivisions' activation 2

Calls: `hydra_click` (seen: LS3b's passing run), `hydra_text` (seen: LS4's passing run), `hydra_screenshot` (seen: LS5's passing run)

Pass: the quoted lines are unchanged and the new line settles P19.

### LS12: Close the window

Calls: `hydra_stop` (never tried)

Pass: the reply says the window closed and names the real-app record it wrote.

## Global constraints

1. The safe tag never moves. Every squeeze's `safe_ms` is the same number before and after (M2 criterion 3, M4 criterion 2).
2. Nothing stored changes. The work touches no stored field and no search decision: the search reads only the safe sum (P5) and the stored scales are written as before (O3, O4). So `kResultsStamp` in `src/store/stored_versions.h` does not move and no saved result reads Stale. The user's ruling (D302 item 4): "nothing in the analysis is changing. only display text".
3. No Rating word and no backend row's eff. figure moves (M1 criteria 3 and 4).
4. The screen decides nothing. Every figure and every "show or not" on the Effective line and the scale lines is the owner's (M3 criteria 1 to 3).
5. The search and the path limit still never read the multiplier (O12). No task touches `src/search`.
6. This plan and the mockup folder leave the repo in the merge that finishes the work.

## Questions for the user

1. **The glossary line.** `CONTEXT.md`, "Needs no timing", ends: "The rating does not: it reads which side of the SP end the note sits on (`core::paid_by_sp_walk`)." After this work that is false for a squeeze: its rating reads the side its badge reads. You already answered the choice itself: "why are you even offering an option that would make the display wrong?" Only you change a glossary term, so: may that sentence become "A squeeze's rating reads the same side as its safe badge (`squeeze_multiplier`); a plain backend row still reads which side of the SP end it sits on."? You will see: nothing on screen from this wording.

That is the only question. Two more that an earlier draft held are settled by what is already ruled: the eff. figure on Bloom by D116 (see "The eff. figure on Bloom"), and the "Effective" against "eff." wording by the mockup the user asked for (see "What stays as the mockup has it").

## Out of scope

Changing what any "eff. N ms" figure means or how it is worded is not built. Relabelling the frontend window, and a longer Effective line than the mockups show, are not built either.

## Promise map

| Promise | Plan line | Task | The check that proves it |
|---|---|---|---|
| One owner decides which stored multiplier a squeeze has | "O5, which stored multiplier" | M1, M6 | M1 criterion 5 and M6 criterion 1 |
| One owner decides the side, by the badge's rule | "O1, which side for a squeeze" | M1 | M1 criteria 1 and 2 |
| The squeezed-out row and its squeeze out take one route | "O2, which side for a backend row" | M1 | M1 criterion 3 |
| A plain row keeps its side | "O2", constraint 3 | M1 | M1 criterion 4 |
| The safe sum is priced by the owner | M2 | M2 | M2 criterion 1 |
| The safe tag never moves | constraint 1 | M2, M4 | M2 criterion 3, M4 criterion 2, LS4's "103.6 ms safe" |
| The Effective figure is the engine's own product | M2, constraint 4 | M2, M3 | M2 criterion 2, M3 criterion 1 |
| The Effective line shows exactly when the owner says so | "When the Effective line shows" | M3 | M3 criteria 1 and 2 |
| An early fill gets no Effective line | "When the Effective line shows" | M2, M3 | M2 criterion 4, M3 criterion 2 |
| The line sits under the Dynamic Hit Window line | M7 | M7 | M7 criterion, LS5's capture |
| A missing stored scale still gives no rating | M1 | M1 | M1 criterion 7 |
| A scale line shows exactly the sides the owner handed out | "The short version" | M3 | M3 criterion 3 |
| Bloom shows x0.40 (late) at the SqIn's SP end | Bloom table | M1, M3 | M1 criterion 1, LS4 |
| The schedule tool calls the owner and its output is unchanged | M4 | M4 | M4 criteria 1 and 2 |
| No Rating word or row eff. figure moves | constraint 3 | M1 | M1 criteria 3 and 4 |
| Nothing stored changes, the stamp stays | constraint 2 | M2 | M2 criterion 3; the reviewer confirms no file under `src/search` or `src/store` is in any task's diff |
| The search never reads the multiplier | constraint 5 | all | no task owns a file under `src/search` (the task table) |
| SP-39 is corrected | M5 | M5 | M5's reviewer check of the block |
| `CONTEXT.md` changes only with the user's yes | question 1 | M5 | M5's reviewer check |
| The real app is seen before merge | "The GUI checks" | LS1 to LS12 | the captures shown to the user |
| The plan and mockups leave with the work | constraint 6 | main session | the finishing merge's file list |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The badge's side for a squeeze is the anchor's press, from the squeeze's kind and `needs_no_timing` together. | code | `src/core/note_window.cpp:79-88` | checked 2026-11-24 |
| P2: The safe sum multiplies the anchor's side by `transfer_side` of the squeeze's stored scale and keeps only the sum. | code | `src/core/note_window.cpp:113-130`, `src/core/note_window.cpp:67-69`, `src/core/note_window.h:142-146` | checked 2026-11-24 |
| P3: The rating picks a squeeze in's side from `paid_by_sp_walk`, so at 0.0 ms it reads early. | code | `src/core/squeeze_rating.cpp:140-147`, `src/core/squeeze_rating.cpp:220-225`, `src/core/backend_value.h:28` | checked 2026-11-24 |
| P4: A squeeze in at 0.0 ms needs timing, so the badge presses its anchor late. | code | `src/core/model.h:302-304`, `src/core/model.h:371`, `src/core/model.h:451` | checked 2026-11-24 |
| P5: The safe-paths search reads only whether a row is short, from `squeeze_window_ends`. | code | `src/search/engine.cpp:482-489` | checked 2026-11-24 |
| P6: The stored scales are written by the search at copy-out and by nothing in this plan's files. | code | `src/search/engine.cpp:464-475` | checked 2026-11-24 |
| P7: A plain row's side comes from `counted_without_squeeze`, and a squeezed-out row's from `paid_by_sp_walk`. | code | `src/core/squeeze_rating.cpp:184-190`, `src/core/backend_value.h:39-41` | checked 2026-11-24 |
| P8: A squeeze out's offset is its squeezed-out row's offset, so the badge's rule and the row's rule give one side at every offset: offset at or before the end is difficulty 0 or more, pressed early; past the end is below 0, pressed late. | code | `src/core/model.cpp:841-861`, `src/core/model.h:302-304`, `src/core/note_window.cpp:79-84` | checked 2026-11-24 |
| P9: An eff. figure is set exactly when the side's multiplier is scaled, and is 0.0 for an offset of 0.0. | code | `src/core/squeeze_rating.cpp:123-130`, `src/core/squeeze_rating.cpp:145`, `tests/test_squeeze_rating.cpp:797-798` | checked 2026-11-24 |
| P10: The Rating word is banded on a backend row's eff. figure only; a squeeze in has no row. | code | `src/core/model.cpp:362-376`, `src/app/path_view.cpp:121-124` | checked 2026-11-24 |
| P11: The scale lines print only sides `rate_activation` marked read and `is_scaled` accepts. | code | `src/app/path_view.cpp:266-312`, `src/core/squeeze_rating.h:126` | checked 2026-11-24 |
| P12: The schedule tool picks the stored scale twice and the anchor press once, by its own spelling. | code | `tools/replay_schedule.cpp:415`, `tools/replay_schedule.cpp:619-626`, `tools/replay_schedule.cpp:91` | checked 2026-11-24 |
| P13: `rate_activation` has one caller outside tests. | code | `src/app/path_view.cpp:259` | checked 2026-11-24; a grep of `src` and `tools` for `rate_activation(`, `note_effective_ms`, `post_read` and `sqins_read` found no other reader |
| P14: The Dynamic Hit Window row is drawn by `window_row`, inside the squeeze's box. | code | `src/ui/paths_tab.cpp:361-418`, `src/app/path_view.h:78-101` | checked 2026-11-24 |
| P15: Whether Bloom's SP end scale line is orange today. | unchecked | `src/app/path_view.cpp:311` sets the colour from rows this plan could not read | LS4; blocks nothing |
| P16: Bloom's Effective figure prints as 29.6 ms. | unchecked | the brief's figures, 74.0 ms and late 0.4, from `hydra_replay schedule` | LS4; blocks nothing |
| P17: Gethsemane's Effective figure prints as 184.0 ms. | unchecked | `docs/superpowers/specs/2026-11-24-effective-line-mockup/index.html` | LS7; blocks nothing |
| P18: Waste of Air's Effective figures print as 172.5 ms and 80.5 ms. | unchecked | `docs/superpowers/specs/2026-11-24-effective-line-mockup/both-squeezes.html` | LS9; blocks nothing |
| P19: Subdivisions' Effective figure prints as 80.2 ms. | unchecked | `docs/superpowers/specs/2026-11-24-effective-line-mockup/subdivisions.html` | LS11; blocks nothing |
| P20: A single-owner rule is a JSON row with `question`, `owner`, `owner_file`, `owner_symbol`, `date`, `decision`, `added_by`, `pattern`, optional `anchor`, `must_match`, `must_not_match`, `owner_lines`, `scope` and `note`, and `core.json` already holds such rows, one for which scale belongs to a SqIn. | code | `tests/single_owner/engine.cpp:93-130`, `tests/single_owner/rules/core.json:804-830` | checked 2026-11-24 |
| P21: `testdata/input` holds a squeeze whose priced side is scaled and one whose side is not. | unchecked | `testdata/input/test_fast_tempo/late_sqin_twice.chart` has a tempo jump to 2000 BPM (line 10), but a read cannot say which squeezes the search finds there or what their scales are | M3 and M7 scan for them at test time and stop and report if none is found; blocks nothing else |
| P22: The `hydra` MCP tools the live steps name exist, and `hydra_start` takes `keep` and `worktree`. | code | `tools/hydra_ui_mcp.py:843-955` | checked 2026-11-24 |
| P27: `Press` is declared in `note_window.h` only, and `squeeze_rating.h` includes neither it nor `note_window.h`, so the owner needs `Press` moved. | code | `src/core/note_window.h:92-104`, `src/core/squeeze_rating.h:24-35` | checked 2026-11-24 |
| P28: The schedule tool applies a press to scales that are not the squeeze's own, through `moving_side`. | code | `tools/replay_schedule.cpp:89-93`, `tools/replay_schedule.cpp:218-229` | checked 2026-11-24 |
| P29: `rate_note` is called directly in tests with a bool for the side. | code | `tests/test_squeeze_rating.cpp:584-625` | checked 2026-11-24 |
| P30: A multiplier within a thousandth of 1 is no scale, so `is_scaled` is the one test of whether a multiplier exists. | code | `src/core/squeeze_rating.h:112-126` | checked 2026-11-24 |
| P23: The user chose the badge's side at 0.0 ms. | ruling | the user's message, quoted in this plan's brief: `why are you even offering an option that would make the display wrong?` | checked 2026-11-24 |
| P24: A multiplier that exists gets a display. | ruling | `docs/agents/rulings/ui-and-display.md:80` | checked 2026-11-24 |
| P25: Display-only changes do not move the results stamp. | code | `docs/audit/2026-10-03-fix-decisions.md:1687`, `src/store/stored_versions.h:42-55` | checked 2026-11-24 |
| P26: D302 item 2 and `CONTEXT.md` both say the rating reads `paid_by_sp_walk`, and SP-39 says the direction follows the sign of the difficulty. | code | `docs/audit/2026-10-03-fix-decisions.md:1685`, `CONTEXT.md:777-783`, `docs/clone-hero/05-star-power.md:445` | checked 2026-11-24 |
