# A collapsed activation row shows every timing it has (2026-10-22)

Task id BADGE2-plan. Planner: Fable. Nothing here is built yet; every task below waits for the user's yes (agent rule 3). Display change, so the design goes to the user first.

## Conflict flag, read this first

An earlier ruling says the row badge names one timing. D80 (2026-10-05, `docs/audit/2026-10-03-fix-decisions.md` line 359) reads: "when nothing on the activation needs timing, the badge names its hardest free squeeze with its figure, 0 or below ('squeeze in -316 ms'). `Activation::badge_timing` owns it." The session card `2026-10-06-dcdf0961` records the user's choice as "Back to 2.0 exactly". D51 call 4 and D60 item 2 (line 258) add: "An optional early fill with time to spare shows no badge, because there is nothing to time."

The user's new request cannot be met inside D80's words. On "The Age of Betrayal" the `+` on rows 1 and 2 is a free squeeze-in (shown below from the code), and D80 lets a free squeeze onto the row only "when nothing on the activation needs timing". The early fill does need timing (zero slack), so D80 hides the squeeze. This plan extends D80: a free squeeze keeps its badge next to a fill too. Everything else D80 protected (path button, Path limit, library filter, stored hardest ms, report, all-0 list) stays as it is. D51 call 4 / D60 item 2 (a fill with time to spare gets no badge) also stays. Nobody has ruled "one badge per row" in those words; the one-badge shape came from the 2.0 design, not from a decision. The user should confirm the extension (question 1 below).

## What the player sees, before and after

The song is "The Age of Betrayal" (Halcyon Way), optimal path `E3+ E2+ 2 2 3`. The inputs below come from the user's report of the screen and from the code's definitions; the one unknown figure is marked.

| Row | Notation | What the notation says (`Activation::notationstr`, `model.cpp` line 556) | Badge today (user's report) | Early fill figure | Squeeze figure |
|---|---|---|---|---|---|
| 1 | `E3+` | E-critical, 3 fills skipped, one squeeze-in | `early fill 0 ms` | 0 ms, optional (3 skips, so not E0) | at or below 0 ms: a free squeeze-in, exact figure to read from the open row |
| 2 | `E2+` | E-critical, 2 fills skipped, one squeeze-in | `early fill 0 ms` | 0 ms, optional | same kind as row 1 |
| 3 | `2` | 2 fills skipped, no squeeze | none | none | none |
| 4 | `2` | same | none | none | none |
| 5 | `3` | 3 fills skipped | none | none | none |

Why the squeeze must be free, from the code (`hardest_part`, `src/core/model.cpp` lines 491 to 513): squeezes are looked at first; a squeeze that needs timing always sets `best`. The optional fill of an E activation with skips is reached only when `best` is still empty (`if (!best && fill_needs_timing && act.is_e_critical())`, line 510). Row 1's badge is that optional fill, so `best` was empty, so the `+` was skipped by `if (!untimed_squeezes && sq.needs_no_timing()) continue;` (line 497). `SPSqueeze::needs_no_timing` (`model.h` line 378) is true for a SqIn when `paid_by_sp_walk(offset_ms)`, that is `offset_ms <= 0.0` (`backend_value.h` line 30). So the note already lands inside Star Power and its badge figure (`squeeze_difficulty`, the SqIn's offset) is 0 or below. The badge then falls back to the fill (`badge_timing`, line 529: `if (auto h = hardest()) return h;`). The fill's 0 comes from `early_fill_difficulty(0) = 0` and `early_fill_needs_timing` is `>= 0.0` (`model.h` line 319), so a zero-slack fill counts.

Before: row 1 reads `1  E3+  m..  N bars  [early fill 0 ms]`. The squeeze is visible only after a click, in the sentence "Hit the SP phrase's last note no more than X ms late so it lands before Star Power ends."

After: row 1 reads `1  E3+  m..  N bars  [early fill 0 ms] [squeeze in -X ms]`, both grey. X is the same whole number Copy path writes for that activation: `E3+ (0/-X ms)` (`notationstr_verbose`, `model.cpp` line 563, uses `format_ms_whole` like the badge). Rows 3 to 5 stay bare. The timeline marks for rows 1 and 2 stay grey-outlined, as today.

A second worked case, from the test fixtures so the colour side is seen: an E0 activation with a 30 ms required fill and a 5 ms squeeze-out (`test_activation_rows.cpp` lines 614 to 618) reads `early fill 30 ms` today. After: `[early fill 30 ms] [squeeze out 5 ms]`, both orange, because each is past the difficult floor (`past_difficult_floor`: `ms > kDifficultMs`, 2.0 ms, `model.h` lines 90 to 95). The timeline mark stays orange.

## How the badge picks today, and what the new design keeps

Three rules live in `hardest_part` and `badge_timing`, all quoted above.

1. Tie rule: "only a strictly larger value takes over" (`d > best->ms`), squeezes first, so a fill tied with a squeeze loses and two tied squeezes name the first in list order. Kept for `hardest()` and `Path::difficulty()`, which the path button, the Path limit, the library filter and the stored summary read. The badges no longer need a tie rule, because every timing is shown.
2. The optional-fill rule (D48 Q10): an E activation that skipped fills shows its optional early fill "when nothing else does". Kept as the fill's own condition (`early_fill_needs_timing && (is_E0() || is_e_critical())`), no longer gated on the squeezes. A fill with time to spare (`early_fill_needs_timing` false) still gets no badge (D51 call 4, D60 item 2).
3. The D80 rule: a squeeze that needs no timing gets a badge "only when nothing else needs timing". Changed: such a squeeze always gets its badge, with its 0-or-below figure and never orange. Next to an early fill it reads, for the user's row, `[early fill 0 ms] [squeeze in -X ms]`.

Both ends of the new rule: an activation whose only timing is a free squeeze still shows exactly `squeeze in -316 ms` (D80's example, unchanged); an activation with a 10 ms slack required fill and nothing else still shows no badge (D51 call 4, `test_activation_rows.cpp` lines 608 to 611, unchanged).

## Design questions for the user, with recommendations

The user's answers, 2026-10-22, in chat: question 1 "Yes, show both"; question 2 "One per timing"; question 5 "Drop to a second line". Questions 3 and 4 follow the recommendations below.

1. Extend D80 so a free squeeze keeps its badge beside an early fill? Recommended: yes. It is the user's own case; without it the request cannot be met.
2. One badge per squeeze, or only the hardest squeeze? `act.sqinouts` can hold two (notation `E2+-` in `test_model.cpp` line 516; paths named `0+- 0++` in `test_search_target.cpp`). Recommended: one badge per timing, in the notation's order (fill first, then each squeeze in list order), the same order Copy path and the Preview print the figures. Three badges is the most a row can hold. The alternative (hardest squeeze only) would need a second "hardest" rule for one slot, and the collapsed row would again hide a timing the open row shows.
3. Colour: each badge is orange by the predicate its own open-row line already uses. A squeeze badge is orange when `SPSqueeze::is_difficult()` (its sentence's colour, `squeeze_sentences` line 162). A fill badge is orange when the fill is required (`is_E0()`) and `past_difficult_floor(early_fill_difficulty(e_offset))`; that is what `Activation::is_difficult()` says whenever the fill is the hardest part. A free squeeze's figure is 0 or below, so it is never orange. No new threshold. Recommended as stated; no alternative worth a question.
4. Order left to right: fill, then squeezes (matches Copy path). Alternative: hardest on the right. Recommended: fill first, one order everywhere.
5. Narrow widths. The details column's minimum is `kMinPathDetailsW = 500` px at scale 1 (`details_view.h` line 20). One badge of the longest wording (`longest_activation_badge()`, "squeeze out -500 ms", 19 characters, about 133 px at the default 13 px font) fits beside `12 bars` ending near 245 px, as `test_paths_row_layout` proves today. Two such badges need about 290 px plus the 14 px right margin and two 8 px pads; the first pill's left edge lands near 160 px, inside the bars. So two longest badges do not fit on one line at the narrowest panel. Recommended: the badges stay flush right on the row's line when they fit; when the first pill would cross the bars' end, every badge moves to a second line inside the same row, still flush right, and that row is one text line taller. The rule is arithmetic in `activation_row_layout` so a unit test pins it. The existing row-layout rule stays: identity (number, notation, measure, bars) on the left, timings on the right (the "names left, controls right" principle the user chose for the settings panel, memory `settings-summary-button-2026-10`). Alternatives: raise `kMinPathDetailsW` (a number change, needs the user), or shorten the wording when two badges show (new words, needs the user). Not recommended.

## Everything that reads the badge today

Each reader, its owner, and whether it changes.

- `Activation::badge_timing()` (`src/core/model.cpp` line 524), owner core. Replaced by `Activation::badge_timings()` returning every badge timing in display order. `hardest()`, `difficulty()`, `needs_timing()`, `is_difficult()` do not change; their tests in `test_model.cpp` (lines 290 to 345) must keep passing untouched.
- `activation_badge()` and `ActivationRowView::badge` / `difficult` (`src/app/path_view.cpp` line 84, `path_view.h` lines 79 to 80), owner app. `badge` becomes `badges`, a vector of `{text, warn}`. `difficult` stays `Activation::is_difficult()`; a test proves it equals "any badge is orange" so the two never disagree.
- `longest_activation_badge()` (`path_view.cpp` line 95), owner app. Unchanged in meaning: the longest one badge. The single-owner rule in `tests/single_owner/rules/app.json` line 1144 pins that tests call it rather than typing a width; the new layout test keeps calling it.
- `activation_row_layout` (`src/ui/activation_row_layout.h`), owner ui. Takes the badges' widths (a vector) and the bars' end instead of one `badge_w`; returns each pill's x and whether the badges sit on a second line. One badge that fits gives the same `badge_x` as today (regression check).
- `render_activation_row` (`src/ui/paths_tab.cpp` lines 276 to 315), owner ui. Draws one pill per badge, each in its own colour (`warn_or_dim(badge.warn)`), row height one line taller when the layout says second line.
- `render_timeline` (`paths_tab.cpp` lines 199 to 268), owner ui. `!a.badge.empty()` becomes `!a.badges.empty()` at lines 221 and 236; the outline colour keeps `warn_or_dim(a.difficult)`; the hover tooltip (lines 251 and 257) appends every badge text, two spaces apart, in badge order.
- Path button timing, `Path::difficulty()` (`path_view.cpp` line 534), owner app. Unchanged (D80: the path button still treats a free squeeze as nothing to time). `test_activation_rows.cpp` line 656 (`"12.6 ms"`) stays.
- Stored summary `hardest_ms` (`record_store.cpp` line 579), the library filter (`library_query.cpp` line 389), the path report window (`report.cpp` lines 54 and 257, reads the stored `hardest_ms`), the all-0 list. Unchanged; no stamp bump.
- Copy path and the Preview's activation label (`Path::pathstring_verbose`, `Activation::notationstr_verbose`, `preview_view.cpp` line 416). Unchanged. They already print every figure with `format_ms_whole`; the badge figures must equal them (test).
- MCP server `tools/hydra_ui_mcp.py` and the uitest dump: no badge parsing (grep for "badge" finds nothing). `hydra_text` and `on_screen` read drawn text (`uitest_harness.cpp` line 561, `visible_text`), and today's `on_screen(h, "squeeze out 163 ms")` proves pill text is included, so both pills appear with no change there.
- Tests that pin badge text: `tests/test_activation_rows.cpp` (lines 63, 76, 514, 528, 573 to 680), `tests/test_activation_row_layout.cpp`, `tests/ui/uitest_paths.cpp` (`paths-rows` lines 246 to 319, `paths-row-layout` lines 624 to 654). All change as the tasks say.
- Docs: `docs/UserGuide.md` lines 193, 197 and 199. `CONTEXT.md` and the ADRs do not mention the badge (grep), so nothing there. The decision gets a D-number in `docs/audit/2026-10-03-fix-decisions.md`.

## The build

Four executor tasks on Opus. T1, T2a and T3 run in parallel from main. T2b forks from T1's join tip once T1 has merged, because it needs `ActivationRowView::badges`. The shared interface, fixed here so T1 and T2a agree without talking: in `path_view.h`, `struct ActivationBadge { std::string text; bool warn = false; };` and `std::vector<ActivationBadge> badges;` replacing `badge`; `difficult` stays. In `activation_row_layout.h`, `activation_row_layout(float widest_measure_w, float bars_w, const std::vector<float>& badge_ws, float row_w, float scale)` returning `measure_x`, `bars_x`, `std::vector<float> badge_x` (one per badge, left edges), `badge_pill_min` (the first pill's left edge) and `bool badges_below`.

### T1: the core list and the app wording (owner: `src/core/model.h`, `src/core/model.cpp`, `src/app/path_view.h`, `src/app/path_view.cpp`, `tests/test_model.cpp`, `tests/test_activation_rows.cpp`)

Replace `badge_timing()` with `badge_timings()`: the early fill first when `early_fill_needs_timing(e_offset)` and (`is_E0()` or `is_e_critical()`), then every squeeze in `sqinouts` order with its `difficulty()`, free ones included. `hardest_part` keeps its filter and tie rule; both functions read the same candidate list so the rule is derived once (one private helper lists candidates, `hardest_part` takes the strict max over the timed ones). `activation_badges(act)` words each entry with `kBadgeWords` and `format_ms_whole`, and sets `warn` as design question 3 says. `build_activations` fills `badges`.

Failing-first test (run red before the change, keep the red line): the user's row, `skips 3, e_offset 0.0, sqinouts {SqIn, -12.0}`, expects badges `["early fill 0 ms", "squeeze in -12 ms"]`, both `warn == false`. Today it returns one badge, so it fails.

Acceptance criteria, each a test in `test_model.cpp` or `test_activation_rows.cpp`:
- A1 the user's row above gives two grey badges in that order.
- A2 free squeeze alone gives exactly `squeeze in -316 ms` (D80 kept).
- A3 required fill with 10 ms slack and nothing else gives no badge (D51 call 4 kept).
- A4 E0 fill 30 ms plus SqOut 5 ms gives `early fill 30 ms`, `squeeze out 5 ms`, both orange; the tie fixture (fill 30, SqOut 30) gives both, order fill first.
- A5 two squeezes give two squeeze badges in list order; three timings give three badges.
- A6 `hardest()` and `difficulty()` answers in `test_model.cpp` lines 290 to 345 are unchanged (the test case runs green with no edit).
- A7 for every fixture in A1 to A5, `av.difficult == any(badges.warn)`.
- A8 copied path agreement: for the user's row, `notationstr_verbose()` is `E3+ (0/-12 ms)` and each badge's number appears in it; the path button for a path of that one activation shows no timing (free squeeze and optional fill are nothing to time).
- A9 `longest_activation_badge()` is still `squeeze out -500 ms` and at least as long as every badge in A1 to A5.

Tests it may run: `build-cpp\Release\hydra_tests.exe -sf=*test_model.cpp` and `-sf=*test_activation_rows.cpp`. Build with `build_cpp.ps1`, test target only.

### T2a: the layout arithmetic (owner: `src/ui/activation_row_layout.h`, `tests/test_activation_row_layout.cpp`)

Extend the helper as the interface says. Rule: pills are laid flush right, last badge ending at `row_w - kRowBadgeRight`, a gap of `kRowBadgePad` between pills; if `badge_pill_min < bars_x + bars_w`, set `badges_below` and keep the same x positions on the second line. No new constant; the gap reuses `kRowBadgePad`.

Acceptance criteria, all in `test_activation_row_layout.cpp`:
- B1 one badge gives the same `badge_x` and `badge_pill_min` as today's formula (`row_w - badge_w - 14*scale`, minus `8*scale`).
- B2 two longest badges (`longest_activation_badge()` width, twice) at a 470 px row with `12 bars` give `badges_below == true`; the same two at a 900 px row give `false` and the first pill clear of the bars.
- B3 with `badges_below` false, the first pill's left edge is greater than `bars_x + bars_w` (never overlaps), for one, two and three badges at the widths where they fit.
- B4 badge x's are increasing and the last ends at `row_w - kRowBadgeRight`.

Tests it may run: `build-cpp\Release\hydra_tests.exe -sf=*test_activation_row_layout.cpp`.

### T2b: drawing (owner: `src/ui/paths_tab.cpp`, `tests/ui/uitest_paths.cpp`; forks from T1's merge, needs T2a merged too)

`render_activation_row` measures each badge, calls the helper with the bars' measured width, draws one pill per badge in `warn_or_dim(b.warn)`, and when `badges_below` makes the Selectable one `GetTextLineHeight()` taller and draws the pills on the second line. `render_timeline` switches to `badges` as the readers list says.

Acceptance criteria, in the named uitest scripts:
- C1 `paths-rows`: Burnout row 1 still shows `squeeze out 163 ms` and an orange mark; Beg rows 1 and 4 show `squeeze out 0 ms` and `early fill 0 ms` with grey marks (today's checks stay green).
- C2 `paths-rows`: hover tooltip on a mark lists every badge of that row (add a check for Burnout row 1's tooltip containing `squeeze out 163 ms`).
- C3 `paths-row-layout`: at the narrowest panel, the check runs with one, two and three `longest_activation_badge()` badges; with one the bars end before the first pill; with two or three either the bars end before the first pill or `badges_below` is true and the row is one line taller.
- C4 `paths-row-layout`: a row with `badges_below` false has its Selectable height equal to today's `px(34)`.

Tests it may run: `hydra_uitest paths-rows` and `hydra_uitest paths-row-layout` only.

### T3: docs and the decision record (owner: `docs/UserGuide.md`, `docs/audit/2026-10-03-fix-decisions.md`)

`docs/UserGuide.md` line 193: "A mark is outlined when its row has a timing badge, in orange only when that timing is orange too." becomes "A mark is outlined when its row has a timing badge, in orange when any of its badges is orange." Line 197: "and a badge for its hardest timing, like `squeeze out 163 ms` or `early fill 20 ms`" becomes "and a badge for each of its timings, like `squeeze out 163 ms` or `early fill 20 ms`; a row with an early fill and a squeeze shows both, the fill first." Line 199: "A squeeze that happens on its own still gets one when nothing else on the row needs timing, with a figure of 0 or below" becomes "A squeeze that happens on its own still gets one, with a figure of 0 or below". The rest of those sentences stays. Add the new D-number paragraph to the fix-decisions file, naming D80 as extended and quoting the user's yes. Public doc style rules apply (`docs/agents/public-doc-style.md`); the guide gets its doc review on the move of main.

Acceptance criteria: D1 the three sentences read as above (the reviewer checks the words against `activation_badges` and `render_timeline`); D2 the D-record names D80, D51 call 4 and D60 item 2 and says which parts stay.

## The real-app check before merge

Memory `ui-changes-need-real-app-check` is the rule: tests against the rule's own arithmetic prove nothing about the real screen. Before merging T2b, the main session copies `build-cpp\Release\Hydra.exe`, the mimalloc DLLs, `assets\`, `resource\`, the user's `hydra_settings.ini`, `hydra_ui.ini` and a copy of `hydra.db` (db, wal and shm) into a scratch folder, opens "The Age of Betrayal", and captures the Paths tab with `PrintWindow` (the memory's recipe) at the user's normal window size and at the narrowest song panel. The user sees both screenshots: rows 1 and 2 with two grey pills, and the narrow case showing whether the pills dropped to a second line. If the screenshot disagrees with the unit tests, the screenshot wins and the merge waits.

## Out of scope (for the user to approve)

- Changing the Preview's or Copy path's text. They already carry every figure.
- Showing a fill with time to spare on the row (D51 call 4 / D60 item 2 stand).
- Any change to `Path::difficulty()`, the Path limit, the library filter, the stored `hardest_ms`, the report window or the all-0 list (D80's protected list).
- Raising `kMinPathDetailsW` or shortening badge words; both need a user decision and neither is needed with the second-line rule.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| A free squeeze keeps its badge beside an early fill | Design question 1; "How the badge picks" rule 3 | T1 | A1 (failing first) |
| A free squeeze alone still reads `squeeze in -316 ms` (D80 kept) | Rule 3, both ends | T1 | A2 |
| A fill with time to spare gets no badge (D51 call 4, D60 item 2) | Rule 2 | T1 | A3 |
| Fill first, then squeezes in list order; every squeeze shown | Design questions 2 and 4 | T1 | A1, A4, A5 |
| Colour per badge by its own existing predicate; free squeeze never orange | Design question 3 | T1 | A1 (both grey), A4 (both orange) |
| `hardest()`, `difficulty()`, path button, stored summary unchanged | Readers list | T1 | A6, A8 |
| Timeline outline colour agrees with the badges | Readers list, `render_timeline` | T1, T2b | A7, C1 |
| Badge figures equal Copy path's figures | After picture; Readers list | T1 | A8 |
| `longest_activation_badge()` still bounds every badge | Readers list | T1 | A9 |
| One badge lays out exactly as today | Design question 5 | T2a | B1 |
| Pills never overlap the bars; they drop to a second line when they would | Design question 5 | T2a, T2b | B2, B3, C3 |
| Badges flush right with a fixed gap, no new constant | Design question 5 | T2a | B4 |
| Row height unchanged when the badges fit | Design question 5 | T2b | C4 |
| Tooltip lists every badge | Readers list | T2b | C2 |
| Existing uitest rows and marks still hold (Burnout, Beg) | Readers list | T2b | C1 |
| MCP text and uitest dump need no change | Readers list | none (verified by grep and `on_screen` today); C1 reruns `on_screen` on pill text | C1 |
| User guide sentences match the code | Docs section | T3 | D1 |
| Decision recorded with D80's protected list | Conflict flag | T3 | D2 |
| Real-app screenshots before merge | Real-app check | main session | the user sees both captures |
