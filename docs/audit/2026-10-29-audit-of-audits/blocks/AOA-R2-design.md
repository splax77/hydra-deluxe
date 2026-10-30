# AOA-R2: one rating rule, by the timing the player needs

Design only. Nothing here is built. The user rejected AOA-R1 (raw offset for every row) with: "what really matters here is the effective timing for the player, not the absolute offset. hydra already shows this with the badges next to each activation. -100 for a backend and a squeeze out are not the same thing". This design rates each row by the hit it asks of the player, using the figure and the difficulty idea the badges already use.

## 1. What each row asks of the player today, and how the figure is computed

A backend row's offset is where the chord sits from the SP end: negative inside SP, positive after. The engine counts a chord under SP when it is at or before the end, or less than the leeway past it (`core::counted_without_squeeze`, `src/core/backend_value.h:41`; leeway 3 ms by default).

A plain counted row asks nothing. The path's score already has its points. The offset is slack: how far a hit can drift before the chord drops out of SP. The user said this out loud: a note 100 ms inside SP needs nothing.

A plain uncounted row (offset at or past the leeway) asks nothing either, for the path's score, which gives it 0 points. The player can still try to win it by hitting the chord early enough to land inside SP; that is the backend squeeze the User Guide names (line 275). The hit that wins it is about `offset` ms early. It is an optional timing, like the optional early fill of D48 Q10.

The squeezed-out row asks a required hit. The engine assumed the chord lands after the SP end so its phrase banks. The player must hit it more than `-offset` ms late (`SPSqueeze::timing`, `difficulty`, `src/core/model.h:368-369`). A squeezed-out chord already past the end needs nothing (`needs_no_timing`, model.h:383, tests the end at 0 ms).

The badges word exactly this. `Activation::badge_timings` (model.cpp:487-501) lists each squeeze's `difficulty()` (SqIn: offset; SqOut: -offset; positive means a hit is needed, zero or negative means slack, D80), marks it required when `!needs_no_timing()`, and colours it orange when `is_difficult()`, which is `past_difficult_floor(ms)`: more than `kDifficultMs` = 2 ms (model.h:90-95, D48 Q3). `hardest` (model.cpp:503) keeps only required timings. The badge prints the raw ms (`format_ms_whole(t.ms)`, path_view.cpp:74).

The eff. figure is the row's, not the badge's. `rate_note` (squeeze_rating.cpp:74-81) picks the scale side (early for a note inside SP, late for one past it), and when the side is scaled (`is_scaled`, D256) sets `effective_ms = |offset| x 2 / (1 + r)` (`effective_backend_ms`, line 57): the raw gap mapped onto the nominal two-hit budget 2W. A scale above 1 makes the gap effectively smaller (the frontend hit moves the end more), below 1 larger. `rate_activation` (lines 115-131) chooses the side with `paid_by_sp_walk` (0 ms) for the squeezed-out row and `counted_without_squeeze` (leeway) for every other row (D117 chose the leeway for plain rows). path_view.cpp:357-359 then takes `summarystr`'s word from the raw offset and appends " (eff. N ms)". So today the word never reads the eff. figure it sits next to: at -50 ms a squeezed-out row says "Hard SqOut (eff. 33.3 ms)" and "Hard SqOut (eff. 66.7 ms)" alike.

## 2. The one rule

Each row's word comes from the timing the player needs for that row, in eff. ms when a scale applies and raw ms otherwise, banded on the report's timing tiers. The needed timing is one function of the row: a plain counted row needs 0; a plain uncounted row needs `offset` (optional); the squeezed-out row needs `SPSqueeze::difficulty`, which is `-offset` inside SP and 0 or less past the end. That is `squeeze_difficulty(is_sqin = !squeezed_out, offset)` from model.h:275, clipped at 0 for the counted plain row because the engine already pays it. The scale side stays the one `rate_note` picks today, so the eff. magnitude is the figure already on screen.

The ladder is `timing_tiers` (squeeze_rating.cpp:175-186): Normal up to 2 ms, Hard up to W/2, Extreme up to W, Insane up to 1.5W, Insane+ up to 2W, Beyond past that, each edge inside its band (`tier_for`, report.cpp:82-94, D48 Q3). Zero and negative read Normal. The "(uncounted)" tag stays exactly where the engine pays 0 (D48 Q6). The "<-- squeezed out (-N)" suffix and its orange stay as they are: they are the cost, a fact, not a rating.

Why this ladder. Hydra has two ladders for "how hard is this timing" and they agree: the badge's one edge (orange past 2 ms) is the tiers' Normal edge, by name (`timing_tiers` reads `kDifficultMs`). The tiers add the hit-window steps above it, and their top edge is the nominal budget 2W, which is the very scale eff. maps onto (`nominal_budget_ms`, squeeze_rating.h:72-81). So the row's eff. figure, banded on the tiers, is self-consistent. `summarystr`'s ladder is the odd one out: it has its own words at its own edges (W, 10 ms, the leeway), and uses "Hard" and "Insane" for different ranges than the tiers do. The report's footer only says the two differ (D50 item 1); nobody chose that. One word, one meaning in public text rules it out.

One disagreement remains, and the rule does not settle it. The badge and the report band raw ms (the badge prints `t.ms`; the report's `hardest_ms` is `Path::difficulty()`, record_store.cpp:584, raw). The rows show eff. ms (D116: a shown multiplier always comes with its figure). Under this rule a row can read Hard while its badge, on the same activation, reads the raw 50 ms a player would call Extreme. I would keep raw on the badge and the report for now, because changing the stored `hardest_ms` moves a results stamp and D116 only asked about rows; but it is a split, and the user should hear it. It is a question, not a choice made here.

## 3. Before and after at defaults (leeway 3 ms, W 85 ms, x1.00)

"Needs" is the player's effective timing for the row, in ms; 0 means nothing. Tier edges: Normal ≤ 2, Hard ≤ 42.5, Extreme ≤ 85, Insane ≤ 127.5, Insane+ ≤ 170.

| Offset | Plain row today | Plain needs | Plain proposed | Squeezed-out today | SqOut needs | Squeezed-out proposed |
|---|---|---|---|---|---|---|
| -100 | Free | 0 | Normal | Insane SqOut | 100 late | Insane |
| -50 | Easy | 0 | Normal | Hard SqOut | 50 late | Extreme |
| -15 | Easy | 0 | Normal | Hard SqOut | 15 late | Hard |
| -10 | Standard | 0 | Normal | Standard SqOut | 10 late | Hard |
| -5 | Standard | 0 | Normal | Standard SqOut | 5 late | Hard |
| 0 | Standard | 0 | Normal | Standard SqOut | 0 (any late hit) | Normal |
| +2 | Standard | 0 | Normal | Standard SqOut | 0 (see section 4) | Normal |
| +3 | Hard (uncounted) | 3 early, optional | Hard (uncounted) | Standard SqOut (uncounted) | 0 | Normal (uncounted) |
| +5 | Hard (uncounted) | 5 early, optional | Hard (uncounted) | Standard SqOut (uncounted) | 0 | Normal (uncounted) |
| +10 | Hard (uncounted) | 10 early, optional | Hard (uncounted) | Easy SqOut (uncounted) | 0 | Normal (uncounted) |
| +15 | Hard (uncounted) | 15 early, optional | Hard (uncounted) | Easy SqOut (uncounted) | 0 | Normal (uncounted) |
| +50 | Hard (uncounted) | 50 early, optional | Extreme (uncounted) | Easy SqOut (uncounted) | 0 | Normal (uncounted) |
| +85 | Insane (uncounted) | 85 early, optional | Extreme (uncounted) | Free SqOut (uncounted) | 0 | Normal (uncounted) |
| +100 | Insane (uncounted) | 100 early, optional | Insane (uncounted) | Free SqOut (uncounted) | 0 | Normal (uncounted) |

Today's edges, read from model.cpp:339-361: plain Free below -85, Easy below -10, Standard while counted, Hard below +85, else Insane; squeezed-out Insane SqOut below -85, Hard SqOut below -10, Standard SqOut below +10, Easy SqOut below +85, else Free SqOut. Note +85 flips from Insane to Extreme because the tiers put an edge inside its band (D48 Q3) and `summarystr`'s "less than" puts it above.

With a scale, the word follows the eff. figure. Squeezed-out row at -50 ms, early side scaled (eff. = 50 x 2 / (1 + r)):

| Scale | Today | Eff. needed | Proposed |
|---|---|---|---|
| x0.50 | Hard SqOut (eff. 66.7 ms) | 66.7 late | Extreme (eff. 66.7 ms) |
| x2.00 | Hard SqOut (eff. 33.3 ms) | 33.3 late | Hard (eff. 33.3 ms) |

The same happens on a plain uncounted row at +50 with the late side at x2.00: eff. 33.3 ms, Hard (uncounted) instead of Extreme (uncounted). The eff. suffix stays as it is (D116).

## 4. The 0 ms vs leeway split

The rule inherits both edges and does not pick between them. The squeezed-out row's needed timing comes from `SPSqueeze::difficulty` and `needs_no_timing`, which test the SP end at 0 ms; the "(uncounted)" tag and the "(-N)" cost come from `counted_without_squeeze`, which tests the leeway. So at +2 ms a squeezed-out row would read "Normal <-- squeezed out (-N)" while its badge reads "squeeze out -2 ms": the word says the player needs nothing, the suffix says the squeeze-out still costs points because the engine counts the chord. That is today's contradiction with a new word on it. D13 chose 0 ms for a SqIn only; D117 chose the leeway for plain rows' scale side; no ruling names the SqOut at +1 to +2 ms. It still needs the user's answer, asked with both sides shown.

## 5. The strongest case against, and one alternative

The case against: every counted plain row reads Normal, from 100 ms inside SP to 2 ms past the end. Today's Free/Easy/Standard told the player how close to the edge a counted chord sits, and a chord 5 ms inside does drop out of SP on a slightly early activation hit or a slightly late chord hit. The Timing column still shows -5.0, and the row's eff. tooltip still names the scale, but the word no longer warns. The second cost: an uncounted plain row at +100 reads "Insane (uncounted)", which can read as a demand when it is an optional win the path never counted. The tag and the guide's sentence have to carry "optional".

The alternative: the badge's ladder instead of the report's. Two words only: Normal when the needed eff. timing is 2 ms or less, otherwise the figure itself in the Rating cell ("50 ms late", "15 ms early (uncounted)"), orange when required and past the floor, exactly as `badge_timings` colours. It adds no words, so no word can disagree with the badge; it loses the Hard/Extreme/Insane banding the user reads in the report. If the user wants the rows to say what the report says, pick the tiers; if the rows should say what the badge says, pick this.

## 6. What a build touches

`BackendSqueeze::summarystr` (model.cpp:339-361) becomes: compute the needed timing from one new owner (a `backend_row_timing(offset, squeezed_out, leeway)` beside `squeeze_difficulty` in model.h), take the eff. magnitude from the row's `NoteRating` when it has one, and look the figure up in `timing_tiers`. The lookup `tier_for` moves from `src/app/report.cpp` into core beside `timing_tiers`, since model.cpp cannot read the app layer; the report keeps calling it. `kBackendInnerBandMs` and the five SqOut words go. The pinned words in `tests/test_rules.cpp`, `tests/test_model.cpp:838-840`, `tests/test_squeeze_rating.cpp:814-828`, `tests/test_activation_rows.cpp:443-531` and `tests/ui/uitest_paths.cpp` (376, 605, 618) are re-pinned from one run, with a boundary test at 2, 42.5, 85, 127.5 and 170 ms for both row kinds and one scaled case per kind. The User Guide line 275, `CONTEXT.md` 664-680 and the differences page follow. No stored record or results stamp moves: the word is computed at display time from the stored offset and scale, and `hardest_ms` stays raw.

## Promise map

| Promise | Plan line | Task (build) | Check that proves it |
|---|---|---|---|
| Each row's word comes from the timing the player needs, one function for both row kinds | Section 2, first paragraph | summarystr rewrite | `backend_row_timing` test: counted plain row gives 0; uncounted gives offset; squeezed-out gives -offset inside, ≤ 0 past the end |
| The word reads the eff. figure when a scale applies | Section 2, 3 (scale table) | summarystr rewrite | Test: -50 ms squeezed out at x0.50 reads Extreme, at x2.00 reads Hard; x1.00 reads Extreme |
| One ladder, the report's tiers, for every row | Section 2, second paragraph | summarystr rewrite | Test: every offset in the section 3 table, both kinds, matches `tier_for` on the needed figure; grep for "SqOut" words and `kBackendInnerBandMs` is empty |
| The badge's orange and the row's "past Normal" agree | Section 2, "Why this ladder" | summarystr rewrite | Test: for each squeezed-out row, `is_difficult()` is true exactly when the word is not Normal (at x1.00) |
| Edges inside their band (D48 Q3) | Section 3 note | summarystr rewrite | Boundary test at 2, 42.5, 85, 127.5, 170 for both kinds |
| "(uncounted)" exactly where the engine pays 0 | Section 2 | summarystr rewrite | Existing pairing test in `test_activation_rows.cpp`: `points == "0"` iff tag |
| The "(-N)" suffix and its orange do not change | Section 2 | none (unchanged) | uitest_paths.cpp pins of the suffix stay green |
| No stored record or results stamp moves | Section 6 | summarystr rewrite | `src/store/stored_versions.h` untouched; a stored path reopens with the new word |
| Guide, CONTEXT.md, differences page match the code | Section 6 | docs follow | `hydra_tests.exe "-tc=docs match code*"` and the reviewer's read of guide line 275 |
| 0 ms vs leeway for the squeezed-out row at +1..+2 ms | Section 4 | Out of scope | A question to the user naming both sides |
| Badge and report stay raw while rows use eff. | Section 2, last paragraph | Out of scope | A question to the user; changing `hardest_ms` moves a stamp |
| Tiers vs the badge's two-word ladder | Section 5 | Out of scope | The user picks; this design recommends the tiers |
