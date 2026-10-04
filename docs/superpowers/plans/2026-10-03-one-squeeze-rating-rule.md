# One Squeeze-Rating Rule Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers-extended-cc:subagent-driven-development (recommended) or superpowers-extended-cc:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Every note near a Star Power end is rated by one rule in one function. Its effective figure shows whenever its multiplier isn't 1, with no 1 ms cutoff.

**Architecture:** A new `rate_note()` in `src/core/squeeze_rating.cpp` answers the one question for a single note. Table rows and SqIn notes both call it. The squeeze-out is rated only as its table row. The details view (`src/app/path_view.cpp`) reads the answer and decides nothing itself.

**Tech Stack:** C++20, doctest (`hydra_tests`), Dear ImGui test engine (`hydra_uitest`), CMake through `build_cpp.ps1`.

**Spec:** this file. The design was settled in the 2026-10-03 conversation; the user decisions are below.

## Global Constraints

Display only. Nothing the search computes or stores changes, so there is no `kResultsStamp` or path-format bump, and no re-analysis. Difficulty, the ms filter, the tiers and every raw ms figure stay raw gap ms (ADR 0001). The multipliers come from the record (`act.transfer_pre` / `act.transfer_post`, stored as 64-bit doubles) and are never recomputed or rounded before use. Commits are staged by file name, never `git add -A`, because other sessions leave edits in this checkout. Every commit carries `Task:`, `Agent:` and `Session:` trailers.

**User decisions (already made):**
1. "if a multiplier exists there should be a corresponding display": the 1 ms cutoff goes.
2. "keep it in the table row only": a squeeze-out's figure stays on its table row, not in its sentence. The SqIn sentence keeps its figure (asked for on 2026-09-29).
3. "it should be calculate as the exact figure, not based on a rounded 0.99".
4. "make sure all of this is only derived once".
5. Flat x1.00: the figure shows only when the stored multiplier isn't exactly 1. The scale line uses the same test and prints extra decimals when two would read x1.00.
6. Counted rows at the SP end or inside the leeway get the early multiplier like every other counted row.
7. Orange means "a multiplier shown on the line governs at least one row or SqIn on this activation". No threshold.

---

## What is wrong today, in plain words

The rating code asks one question about every note near the SP end: which multiplier governs it, and what is its margin worth on the normal scale? That question is answered in two places, in different words. Table rows go through four hand-written branches. SqIn and SqOut notes go through a second loop that reasons from the squeeze kind and the sign of its difficulty. Both loops also carry their own copy of an invented 1 ms cutoff, which hides any figure that moves by less than 1 ms. That cutoff is why the `[RY]` row on Sinner's Vengeance shows no figure under its x0.99 multiplier.

The squeeze-out is worse: it is rated twice. Its table row is rated against the final SP end. Its squeeze-list entry is rated against the end from before the SqIn extended Star Power, which is 2 bars earlier and has a different multiplier (x0.97 on that chart). The engine proves the row is the right one: both numbers are the same stored value, `sqinout_timing` of the deact edge (`graph.cpp` `add_deact_edge`, `engine.cpp` `create_deactivated_path`). The hidden copy's figure is never printed, but it can still turn the scale line orange.

The display layer adds a third, smaller copy. `path_view.cpp` decides for itself which multipliers count as "shown" and "the same", using a 0.005 rounding tolerance.

## The one rule

The side of the SP end a note sits on decides which activation-hit direction moves it across. A note inside Star Power is crossed by an early hit pulling the end back over it. A note outside is crossed by a late hit pushing the end past it. That's true whether the crossing loses the note (a counted row, a free squeeze) or wins it (a squeeze you still have to earn), so the squeeze kind never matters. Today's four branches and the second loop's kind-and-sign logic both reduce to this.

"Inside" comes from the rules that already own it in `core/backend_value.h`. A table row is about its points, so it's inside when `counted_without_squeeze` says so (at or before the end, or inside the leeway). A squeezed-out row and a SqIn are about their phrase, which the SP end itself decides, so they're inside when `paid_by_sp_walk` says so (at or before the end). This matches today's behaviour for every row except the leeway rows in decision 6.

A multiplier "exists" when it differs from 1 by more than floating-point noise (1e-9). The noise tolerance only absorbs two equal measure lengths reached through different tempos. It is not a display cutoff: x0.999 counts.

## What you'll see change

The `[RY]` row on Sinner's Vengeance gains its figure: about "(eff. 188.4ms)", exact value from the stored multiplier. Any row or SqIn whose multiplier isn't exactly 1 now always shows its figure, however small the shift. Counted rows at the SP end or inside the leeway get the early multiplier, so a row at exactly 0.0 under a scaled early side reads "(eff. 0.0ms)" (Gore act 4 is one). The scale line prints a third decimal (or more) when two would read x1.00, so you never see a figure without its multiplier. The row's hover tooltip prints the multiplier the same way, and its two budgets to one decimal, so it can't read "170ms, not 170ms". Orange now means a shown multiplier governs something on that activation. Charts with no tempo or meter change at the activation or SP end look exactly as before.

## What others do

Collapsing several conditionals that compute the same answer into one named function is Fowler's "Consolidate Conditional Expression" ([refactoring.guru](https://refactoring.guru/refactoring/techniques/simplifying-conditional-expressions), [Fowler, ch. 7](https://www.laputan.org/pub/patterns/fowler/Seven.pdf)). Comparing a computed ratio to exactly 1 needs a small tolerance chosen for the use, not machine epsilon ([Amy Tabb on float equality](https://amytabb.com/tips/2022/01/23/floating-point/)). Here that tolerance is 1e-9: far below anything a chart's tempo map can produce on purpose, far above division round-off.

## Files

`src/core/squeeze_rating.h` / `.cpp` own the rule: `is_scaled`, `NoteRating`, `rate_note`, and `rate_activation` rewired onto them. `transfer_is_material` and `kTransferImpactMs` are deleted, since nothing is left that uses them. `src/app/path_view.cpp` reads the rule's answer and owns only formatting (`format_scale`). Tests: `tests/test_squeeze_rating.cpp`, `tests/test_path_view.cpp`.

---

### Task 1: One rule in the rating core

**Goal:** `rate_activation` rates every table row and every SqIn through one `rate_note()`, rates the squeeze-out only as its row, and always sets a figure when the multiplier isn't 1.

**Files:**
- Modify: `src/core/squeeze_rating.h` (rating pieces section, `BackendRating`, `ActivationRating`)
- Modify: `src/core/squeeze_rating.cpp:63-172` (`transfer_is_material` removed, `rate_note` added, `rate_activation` body)
- Modify: `src/app/path_view.cpp:256-259, 295-305, 323-327` (field renames and the orange flag only, so the build stays green)
- Test: `tests/test_squeeze_rating.cpp` (cases at lines 375-612 replaced; lines 632 and 773 edited)
- Test: `tests/test_path_view.cpp:285-296` (the Gore case, whose expected orange and figure this task changes)

**Acceptance Criteria:**
- [ ] `rate_note(-187.5, true, {0.9912, 1.0}, 85)` returns early, scale 0.9912, effective 375/1.9912 exactly.
- [ ] `rate_note` returns no figure at exactly 1.0 and at 1.0 + 1e-12.
- [ ] A squeezed-out row reads `transfer_post`, never `transfer_pre`; the SqOut's entry in `note_effective_ms` is empty.
- [ ] A SqIn reads `transfer_pre`, by its side of the end.
- [ ] A counted row at 0.0 or inside the leeway reads the early side.
- [ ] `grep -n "transfer_is_material\|kTransferImpactMs\|_warns" src tests` finds nothing.
- [ ] Full `hydra_tests` passes.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe` → `Status: SUCCESS!`

**Steps:**

- [ ] **Step 1: Replace the rating test cases.** In `tests/test_squeeze_rating.cpp`, delete the six cases from `"rate_activation: SqIns warn late, SqOuts early, with no backend rows"` (line 375) through `"rate_activation: SqIn/SqOut eff. figures, one per squeeze"` (ending line 612), and put these in their place:

```cpp
TEST_CASE("is_scaled: exactly 1 up to float noise, nothing coarser") {
    CHECK_FALSE(is_scaled(1.0));
    CHECK_FALSE(is_scaled(1.0 + 1e-12));
    CHECK(is_scaled(0.999));
    CHECK(is_scaled(1.0001));
}

TEST_CASE("rate_note: the side of the end picks the multiplier") {
    const TransferScale s{0.5, 2.0};
    // Inside SP: an early hit is what moves the end across it.
    NoteRating in = rate_note(-50.0, true, s, 85.0);
    CHECK(in.early);
    CHECK(in.scale == 0.5);
    CHECK(in.budget_ms == doctest::Approx(squeeze_budget_ms(0.5, 85.0)));
    REQUIRE(in.effective_ms.has_value());
    CHECK(*in.effective_ms == doctest::Approx(effective_backend_ms(-50.0, 0.5)));
    // Outside: a late hit.
    NoteRating out = rate_note(50.0, false, s, 85.0);
    CHECK_FALSE(out.early);
    CHECK(out.scale == 2.0);
    REQUIRE(out.effective_ms.has_value());
    CHECK(*out.effective_ms == doctest::Approx(effective_backend_ms(50.0, 2.0)));
}

TEST_CASE("rate_note: a figure whenever the multiplier is not 1, however close") {
    // Sinner's Vengeance act 3 shape: 187.5 ms at an early scale just under
    // 1 moves by under 1 ms, which the old 1 ms floor hid. Full precision.
    const double r = 0.9912;
    NoteRating n = rate_note(-187.5, true, TransferScale{r, 1.0}, 85.0);
    REQUIRE(n.effective_ms.has_value());
    CHECK(*n.effective_ms == doctest::Approx(375.0 / (1.0 + r)).epsilon(1e-12));
    // Exactly 1, or float noise around it: no figure.
    CHECK_FALSE(rate_note(-187.5, true, TransferScale{1.0, 1.0}, 85.0).effective_ms.has_value());
    CHECK_FALSE(
        rate_note(-187.5, true, TransferScale{1.0 + 1e-12, 1.0}, 85.0).effective_ms.has_value());
    // A note on the end itself keeps its figure when scaled: 0 ms.
    NoteRating z = rate_note(0.0, true, TransferScale{8.59, 8.76}, 85.0);
    REQUIRE(z.effective_ms.has_value());
    CHECK(*z.effective_ms == 0.0);
}

TEST_CASE("rate_activation: every row reads post, by its side of the end") {
    REQUIRE(core::default_rules().backend_leeway_ms > 1.5);
    Activation act;
    act.skips = 0;
    act.e_offset = 300.0;  // not e-critical
    act.transfer_pre = TransferScale{3.0, 4.0};  // a row never reads pre
    act.transfer_post = TransferScale{0.5, 2.0};
    auto add = [&act](int64_t tick, double off) {
        BackendSqueeze b;
        b.timecode = Timecode::raw(tick);
        b.offset_ms = off;
        act.backends.push_back(b);
    };
    add(1000, -40.0);  // counted, inside SP: early
    add(1001, 0.0);    // counted, on the end: early
    add(1002, 1.5);    // counted inside the leeway: early (decision 6)
    add(1003, 50.0);   // uncounted, past the leeway: late

    ActivationRating r = rate_activation(act, 85.0);
    REQUIRE(r.backends.size() == 4);
    CHECK(r.backends[0].note.early);
    CHECK(r.backends[1].note.early);
    CHECK(r.backends[2].note.early);
    CHECK(r.backends[2].note.scale == 0.5);
    CHECK_FALSE(r.backends[3].note.early);
    CHECK(r.backends[3].note.scale == 2.0);
    for (const BackendRating& b : r.backends) {
        REQUIRE(b.note.effective_ms.has_value());
        CHECK(*b.note.effective_ms ==
              doctest::Approx(effective_backend_ms(*b.row.offset_ms, b.note.scale)));
    }
    CHECK(r.scale_governs);
}

TEST_CASE("rate_activation: the squeeze-out is rated once, as its row, at post") {
    // Sinner's Vengeance act 3 shape: a SqIn extended SP, then the path
    // squeezed out the [RY] phrase 187.5 ms before the final end. The SqOut
    // entry and the row hold the same stored number (the deact edge's
    // sqinout_timing), measured from the final end, so the row's post
    // multiplier is the only one that applies.
    Activation act;
    act.skips = 0;
    act.e_offset = 300.0;
    act.transfer_pre = TransferScale{0.97, 0.974};
    act.transfer_post = TransferScale{0.9912, 0.98};
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, -92.1});
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -187.5});
    BackendSqueeze ry;
    ry.timecode = Timecode::raw(5000);
    ry.offset_ms = -187.5;
    ry.is_sp = true;
    act.backends.push_back(ry);
    act.sqout_tick = 5000;

    ActivationRating r = rate_activation(act, 85.0);
    REQUIRE(r.backends.size() == 1);
    CHECK(r.backends[0].squeezed_out);
    CHECK(r.backends[0].note.early);
    CHECK(r.backends[0].note.scale == 0.9912);
    REQUIRE(r.backends[0].note.effective_ms.has_value());
    CHECK(*r.backends[0].note.effective_ms == doctest::Approx(375.0 / 1.9912).epsilon(1e-12));
    REQUIRE(r.note_effective_ms.size() == 2);
    // The SqIn: free by 92.1 ms at the pre end, so pre's early side.
    REQUIRE(r.note_effective_ms[0].has_value());
    CHECK(*r.note_effective_ms[0] == doctest::Approx(184.2 / 1.97).epsilon(1e-12));
    // The SqOut: its row above is its only rating.
    CHECK_FALSE(r.note_effective_ms[1].has_value());

    // A free squeeze-out, already past the end, reads post's late side.
    act.backends[0].offset_ms = 40.0;
    act.sqinouts[1] = SPSqueeze{SqueezeKind::SqOut, 40.0};
    r = rate_activation(act, 85.0);
    CHECK_FALSE(r.backends[0].note.early);
    CHECK(r.backends[0].note.scale == 0.98);
}

TEST_CASE("rate_activation: a SqIn reads pre, by its side of the end") {
    Activation act;
    act.skips = 0;
    act.e_offset = 300.0;
    act.transfer_pre = TransferScale{1.6, 0.5};
    act.transfer_post = TransferScale{3.0, 4.0};  // a SqIn never reads post
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, -400.0});  // free: early
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 50.0});    // to earn: late
    ActivationRating r = rate_activation(act, 85.0);
    REQUIRE(r.note_effective_ms.size() == 2);
    REQUIRE(r.note_effective_ms[0].has_value());
    REQUIRE(r.note_effective_ms[1].has_value());
    // Sun of Nothing act 5: 400 ms free at early x1.60 is worth 307.7 ms.
    CHECK(*r.note_effective_ms[0] == doctest::Approx(800.0 / 2.6).epsilon(1e-12));
    CHECK(*r.note_effective_ms[1] == doctest::Approx(100.0 / 1.5).epsilon(1e-12));
    CHECK(r.scale_governs);
}

TEST_CASE("rate_activation: scale_governs only for a scaled multiplier on a rated note") {
    Activation act;
    act.skips = 0;
    act.e_offset = 300.0;
    // The late side is scaled, but the only row is inside SP (early, x1.00).
    act.transfer_post = TransferScale{1.0, 6.33};
    act.transfer_pre = act.transfer_post;
    BackendSqueeze in;
    in.offset_ms = -40.0;
    act.backends.push_back(in);
    ActivationRating r = rate_activation(act, 85.0);
    CHECK_FALSE(r.scale_governs);
    CHECK_FALSE(r.backends[0].note.effective_ms.has_value());

    // A flat activation never governs, even with a gap past the budget.
    Activation flat = act;
    flat.transfer_post = TransferScale{};
    flat.transfer_pre = TransferScale{};
    flat.backends[0].offset_ms = 222.2;
    CHECK_FALSE(rate_activation(flat, 85.0).scale_governs);
}
```

Then in `"rate_activation: the stored scales are the only scales"` change `CHECK(r.late_warns);` to `CHECK(r.scale_governs);`. In `"rate_activation: a plain row inside the leeway is not a frontend squeeze"` change `CHECK_FALSE(r.late_backend_warns);` to:

```cpp
    // Counted inside the leeway, so the early side governs it -- x1.00 here.
    CHECK_FALSE(r.scale_governs);
```

and leave its `CHECK_FALSE(r.backends[0].effective_ms.has_value());` but spell it `r.backends[0].note.effective_ms`.

In `tests/test_path_view.cpp`, replace the Gore block (lines 285-296) with:

```cpp
    // Gore act 4: the only row sits exactly on the SP end. It is inside SP,
    // so the early x8.59 governs it: its 0 ms margin is still 0 ms, and the
    // line is orange because a shown multiplier governs a row.
    Activation gore = base;
    gore.transfer_post = TransferScale{8.59, 8.76};
    gore.transfer_pre = gore.transfer_post;
    BackendSqueeze on_end;
    on_end.offset_ms = 0.0;
    gore.backends.push_back(on_end);
    ActivationRowView av = row_of(gore);
    CHECK(av.scale_warning ==
          "Frontend timing scales x8.59 (early) / x8.76 (late) at the SP end.");
    CHECK(av.scale_warn);
    REQUIRE(av.backends.size() == 1);
    CHECK(av.backends[0].rating.find(" (eff. 0.0ms)") != std::string::npos);
```

- [ ] **Step 2: Build and watch it fail.** Run `.\build_cpp.ps1 -Target hydra_tests`. Expected: compile errors naming `is_scaled`, `rate_note`, `NoteRating`, `scale_governs` and `note`.

- [ ] **Step 3: The header.** In `src/core/squeeze_rating.h`, replace the block from `// The transfer scale is shown when it is material` through the `transfer_is_material` declaration with:

```cpp
// A multiplier is x1.00 only when it equals 1 up to the round-off of the
// measure-length division (two equal measures reached through different
// tempos). This is not a display cutoff: x0.999 is scaled.
constexpr double kScaleIdentityTolerance = 1e-9;
inline bool is_scaled(double r) { return std::abs(r - 1.0) > kScaleIdentityTolerance; }

// The one rule for a single note near a Star Power end. The side of the end
// it sits on decides which activation-hit direction moves the end across it:
// a note inside SP is crossed by an early hit pulling the end back, a note
// outside by a late hit pushing the end past it. That holds whether the
// crossing loses the note (a counted row, a free squeeze) or wins it (a
// squeeze still to earn), so the squeeze kind never enters.
struct NoteRating {
    bool early = false;                  // which side of the end's TransferScale governs
    double scale = 1.0;                  // that side's stored multiplier, full precision
    double budget_ms = 0.0;              // squeeze_budget_ms(scale, W)
    std::optional<double> effective_ms;  // set exactly when is_scaled(scale)
};

// offset_ms: the note's ms minus the SP end's ms (negative = before it).
// inside: whether the note is inside SP on this path, by the counting rule
// that owns it (core/backend_value.h). at_end: the multipliers stored for
// the SP end that offset is measured from.
NoteRating rate_note(double offset_ms, bool inside, const TransferScale& at_end,
                     double hit_window_ms = kDefaultHitWindowMs);
```

Add `#include <cmath>` to the header's includes. Replace `BackendRating` with:

```cpp
// One backend table row, resolved: the display row, whether it is the
// squeezed-out note, and its rating at the deact-node end. A row with no
// offset keeps the default rating (x1.00, no figure).
struct BackendRating {
    BackendSqueeze row;
    bool squeezed_out = false;
    NoteRating note;
};
```

In `ActivationRating`, replace the six `*_warns` fields and their comment with:

```cpp
    // True when a multiplier that isn't 1 governs at least one row or SqIn
    // on this activation: the scale line turns orange.
    bool scale_governs = false;
```

and change the `note_effective_ms` comment to:

```cpp
    // One entry per act.sqinouts, in that order: a SqIn's figure, rated at
    // the pre end. Always empty for a SqOut, whose note is its squeezed-out
    // backend row and is rated there.
```

- [ ] **Step 4: The implementation.** In `src/core/squeeze_rating.cpp`, delete `transfer_is_material` and add after `squeeze_budget_ms`:

```cpp
NoteRating rate_note(double offset_ms, bool inside, const TransferScale& at_end,
                     double hit_window_ms) {
    NoteRating n;
    n.early = inside;
    n.scale = inside ? at_end.early : at_end.late;
    n.budget_ms = squeeze_budget_ms(n.scale, hit_window_ms);
    if (is_scaled(n.scale)) n.effective_ms = effective_backend_ms(offset_ms, n.scale);
    return n;
}
```

Replace the body of `rate_activation` from `// The scale that governs each row` through `out.early_warns = ...;` with:

```cpp
    // Backend rows: every offset is measured from the deact node D, so they
    // read `post`. A squeezed-out row is about its phrase, which the SP end
    // itself decides; every other row is about its points, which the engine
    // counts up to the leeway.
    std::vector<BackendSqueeze> backends = act.display_backends();
    out.backends.reserve(backends.size());
    for (const BackendSqueeze& bsq : backends) {
        BackendRating row;
        row.row = bsq;
        row.squeezed_out = act.is_sqout_backend(bsq);
        if (bsq.offset_ms) {
            const double o = *bsq.offset_ms;
            const bool inside = row.squeezed_out
                                    ? core::paid_by_sp_walk(o)
                                    : core::counted_without_squeeze(o, backend_leeway_ms);
            row.note = rate_note(o, inside, out.scales.post, hit_window_ms);
            out.scale_governs |= row.note.effective_ms.has_value();
        }
        out.backends.push_back(std::move(row));
    }

    // SqIn phrase notes: the offset is measured from the end before the
    // phrase extended SP, so they read `pre`. A SqOut is not rated here: its
    // offset is its squeezed-out row's offset (both are the deact edge's
    // sqinout_timing), and that row was rated above, at the end it is
    // measured from.
    out.note_effective_ms.reserve(act.sqinouts.size());
    for (const SPSqueeze& sq : act.sqinouts) {
        if (sq.kind == SqueezeKind::SqOut) {
            out.note_effective_ms.push_back(std::nullopt);
            continue;
        }
        const NoteRating n = rate_note(sq.offset_ms, core::paid_by_sp_walk(sq.offset_ms),
                                       out.scales.pre, hit_window_ms);
        out.scale_governs |= n.effective_ms.has_value();
        out.note_effective_ms.push_back(n.effective_ms);
    }
```

Fix the module comment at the top of `squeeze_rating.h` ("when a scale is material enough to warn about") to read "which multiplier governs a note, and what its margin is worth on the nominal scale".

- [ ] **Step 5: Keep the display compiling.** In `src/app/path_view.cpp`, change `br.effective_ms` to `br.note.effective_ms`, `br.scale` to `br.note.scale` and `br.budget_ms` to `br.note.budget_ms` (lines 295-305 and 323-327). Replace the `av.scale_warn = ...` statement (lines 256-259) with `av.scale_warn = rate.scale_governs;`. Task 2 does the rest of the display.

- [ ] **Step 6: Build and run.** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe`. Expected: `Status: SUCCESS!`. If any other case asserts an old orange flag or a missing figure, it was pinning the 1 ms cutoff or the double rating: update its expectation to the rule above, name it in the commit message, and tell the user which one.

- [ ] **Step 7: Commit.**

```bash
git add src/core/squeeze_rating.h src/core/squeeze_rating.cpp src/app/path_view.cpp tests/test_squeeze_rating.cpp tests/test_path_view.cpp
git commit -m "Rate every squeeze note through one rule, figure whenever scaled

Task: 1 one-squeeze-rating-rule
Agent: main
Session: d7f92775-f421-4da9-9f43-5ccf6e0b461d
Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

---

### Task 2: The display reads the rule and only formats it

**Goal:** The scale line and the row tooltip show a multiplier exactly when `is_scaled` says so, print enough decimals that it never reads x1.00, and the line turns orange exactly when `scale_governs`.

**Files:**
- Modify: `src/app/path_view.cpp:221-255` (scale line), `:296-306` (row tooltip), anonymous namespace (new `format_scale`), includes (`<cstdlib>`)
- Test: `tests/test_path_view.cpp`, new case after line 864

**Acceptance Criteria:**
- [ ] A post early scale of 0.9973 prints `x0.997 (early)` on the line and in the row tooltip; the squeezed-out row shows `(eff. X ms)` with X = 375 / 1.9973 to one decimal; the SqOut sentence has no `eff.`.
- [ ] The tooltip's two budgets print to one decimal (169.8 and 170.0 at x0.9973, not "170ms, not 170ms").
- [ ] `grep -n "0.005" src/app/path_view.cpp` finds nothing.
- [ ] Full `hydra_tests` and `hydra_uitest --all --jobs 4` pass.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe; .\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --all --jobs 4` → `Status: SUCCESS!` and every test `[PASS]`

**Steps:**

- [ ] **Step 1: Tests.** In `tests/test_path_view.cpp`, change the comment `// A side that prints as x1.00 is left out.` to `// A side at exactly x1.00 is left out.` After the case ending at line 864, add:

```cpp
TEST_CASE("build_activations: a near-1 multiplier prints its decimals and its row's figure") {
    HydraRecord rec;
    Activation act;
    act.skips = 0;
    act.e_offset = 300.0;  // not e-critical
    act.transfer_post = TransferScale{0.9973, 1.0};
    act.transfer_pre = act.transfer_post;
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -187.5});
    BackendSqueeze ry;
    ry.timecode = Timecode::raw(5000);
    ry.offset_ms = -187.5;
    ry.is_sp = true;
    act.backends.push_back(ry);
    act.sqout_tick = 5000;
    Path p;
    p.activations.push_back(act);

    ActivationsView v = build_activations(p, rec, nullptr, 85.0);
    REQUIRE(v.acts.size() == 1);
    CHECK(v.acts[0].scale_warning == "Frontend timing scales x0.997 (early) at the SP end.");
    CHECK(v.acts[0].scale_warn);
    REQUIRE(v.acts[0].backends.size() == 1);
    // 375 / 1.9973 = 187.75... -> "187.7" or "187.8" per %.1f; compute it.
    char want[32];
    std::snprintf(want, sizeof(want), " (eff. %.1fms)", 375.0 / 1.9973);
    CHECK(v.acts[0].backends[0].rating.find(want) != std::string::npos);
    const std::string& tip = v.acts[0].backends[0].tooltip;
    CHECK(tip.find("scales x0.997 here") != std::string::npos);
    CHECK(tip.find("budget is 169.8ms, not 170.0ms") != std::string::npos);
    // The squeeze-out's figure lives on its row only (decision 2).
    REQUIRE(v.acts[0].squeeze_sentences.size() == 1);
    CHECK(v.acts[0].squeeze_sentences[0].text.find("eff.") == std::string::npos);
}
```

- [ ] **Step 2: Run and watch the new case fail.** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="build_activations*"`. Expected: the near-1 case fails on the line text (today it prints `x1.00 (early)`).

- [ ] **Step 3: `format_scale`.** Add `#include <cstdlib>` to `src/app/path_view.cpp`, and in its anonymous namespace:

```cpp
// A multiplier as the scale line prints it: two decimals, or as many more as
// it takes not to read as x1.00 (nine reach past is_scaled's tolerance), so
// a multiplier that governs a figure never prints as 1.
std::string format_scale(double r) {
    char buf[32];
    for (int digits = 2; digits <= 9; ++digits) {
        std::snprintf(buf, sizeof(buf), "x%.*f", digits, r);
        if (std::strtod(buf + 1, nullptr) != 1.0) break;
    }
    return buf;
}
```

- [ ] **Step 4: The scale line.** Replace lines 221-259 (from `// The line shows every scale that isn't x1.00` through the `av.scale_warn` statement) with:

```cpp
        // The line names every multiplier that isn't 1, early first: at the
        // SP end, then at the SqIn's end when that prints differently. It is
        // orange when one of them governs a row or SqIn on this activation.
        auto sides = [](const TransferScale& s) {
            std::string out;
            if (is_scaled(s.early)) out = format_scale(s.early) + " (early)";
            if (is_scaled(s.late)) {
                if (!out.empty()) out += " / ";
                out += format_scale(s.late) + " (late)";
            }
            return out;
        };
        const std::string post_part = sides(rate.scales.post);
        const std::string pre_part = sides(rate.scales.pre);
        std::string clauses;
        if (!post_part.empty()) clauses = post_part + " at the SP end";
        if (!pre_part.empty() && pre_part != post_part) {
            if (!clauses.empty()) clauses += "; ";
            clauses += pre_part + " at the SqIn's SP end";
        }
        if (!clauses.empty()) av.scale_warning = "Frontend timing scales " + clauses + ".";
        av.scale_warn = rate.scale_governs;
```

- [ ] **Step 5: The row tooltip.** Replace the tooltip `snprintf` (lines 300-305) so the multiplier goes through `format_scale` and the budgets print to one decimal:

```cpp
                std::snprintf(tip, sizeof(tip),
                              "Effectively %.1fms on the normal %.0fms scale:\n"
                              "frontend timing scales %s here, so the combined\n"
                              "squeeze budget is %.1fms, not %.1fms.",
                              *br.note.effective_ms, normal_budget,
                              format_scale(br.note.scale).c_str(), br.note.budget_ms,
                              normal_budget);
```

- [ ] **Step 6: Run everything.** Unit tests, then `.\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --all --jobs 4`. Expected: all pass.

- [ ] **Step 7: Commit.**

```bash
git add src/app/path_view.cpp tests/test_path_view.cpp
git commit -m "Scale line: any multiplier that isn't 1, enough decimals, orange when it governs

Task: 2 one-squeeze-rating-rule
Agent: main
Session: d7f92775-f421-4da9-9f43-5ccf6e0b461d
Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

---

### Task 3: Check it on Sinner's Vengeance

**Goal:** Print what the details panel now shows for Sinner's Vengeance (2X Bass Pedal), optimal path, activation 3, from the real stored record, and check the `[RY]` figure against the formula with the stored multiplier.

**Files:**
- Create (scratchpad, not committed): `probe.cpp`, copied from `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\96ae6b89-0cef-4bb4-804e-e995aa36f9eb\scratchpad\probe.cpp`

**Acceptance Criteria:**
- [ ] The probe prints the stored `transfer_post.early` for activation 3 at full precision.
- [ ] The `[RY]` row prints `(eff. X ms)` with X = 375 / (1 + that value) to one decimal.
- [ ] The SqIn sentence still prints `(eff. 93.3 ms)` (its pre multiplier is unchanged).
- [ ] The scale line text and colour are reported as printed.

**Verify:** the probe output, quoted in the report to the user.

**Steps:**

- [ ] **Step 1: Copy the DB.** Copy `C:\Program Files\Hydra\hydra.db`, `hydra.db-wal` and `hydra.db-shm` into the scratchpad (WAL mode: all three).

- [ ] **Step 2: Find the hash.** `python -c "import sqlite3,sys; c=sqlite3.connect(sys.argv[1]); print(c.execute(\"select hyhash, ref_name from songmeta where ref_name like 'Sinner''s Vengeance%'\").fetchall())" <scratch>\hydra.db`

- [ ] **Step 3: Extend the probe.** In the copy, print for every activation of button 0: `%.17g` of `transfer_pre.early/late` and `transfer_post.early/late` (from `b.path->walk_activations()`), every backend row's timing, chord and rating, and every squeeze sentence. Remove the `squeezed out` filter.

- [ ] **Step 4: Build and run it.** In a shell with `C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat` loaded: `cl /MD /std:c++20 /EHsc /utf-8 /DHYDRA_VERSION=\"x\" /I src probe.cpp build-cpp\Release\hydra_core.lib build-cpp\Release\sqlite3.lib build-cpp\Release\miniz.lib bcrypt.lib winhttp.lib shell32.lib ole32.lib uuid.lib`, then `probe.exe <scratch>\hydra.db <hyhash>`.

- [ ] **Step 5: Report.** Quote activation 3's lines and the arithmetic: stored multiplier, 375 / (1 + r), printed figure.
