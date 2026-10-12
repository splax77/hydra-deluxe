# Analyze button follows the status chip (decided 2026-10-11)

## The user's choice

The user picked "Follow the chip" in chat on 2026-10-11. The Analyze button
counts and runs the rows on screen, which is the same set the library heading
counts. Example: a search with 10 matches (6 Not analyzed, 1 Stale,
3 Analyzed) and the Not analyzed chip on. The heading reads "6 of 97 charts",
and the button must read 6 and run 6. Today it reads 10 and runs 7, because it
ignores the chip and then skips the 3 current charts.

## What the code does today (checked against the code, main at 7cd003f7)

- The heading reads `library_shown_count()`, which is `library.order().size()`.
  `refilter()` in src/ui/library_model.cpp pushes a row into `order_` when it
  matches the query and the chip.
- The button reads `library_match_count()`, which is `library.counts().all`.
  That counts query matches only. It is the same number as the All chip.
- The batch runs `library_matches()` (src/ui/app_state.cpp), built from
  `LibraryModel::matches()`. That is the query matches, whatever the chip.
  `open_batch_confirm` builds both plans from it (D79).
- With no search but a chip on, the button reads "Analyze library..." and runs
  every chart (`searching()` is false).
- The "whatever the chip" rule came from a code comment in the 2026-09-27 UI
  redesign plan. It was never a user decision.

## Design: one owner for "the rows on screen"

`LibraryModel::order()` already owns the rows on screen. The heading, the
button's count and the batch's charts should all read it, and nothing else.

1. In LibraryModel, delete `matches()`. Add `narrowed()`, which is
   `searching() || chip_ != StatusChip::All`. It answers whether the table
   shows fewer rows than the library holds because of the user's own choice.
2. In AppState, delete `library_match_count()`. `library_matches()` reads
   `library.order()` instead of `library.matches()`. Its comment changes to
   "the rows on screen".
3. In library_toolbar.cpp, the button asks `library.narrowed()` instead of
   `searching()`. Its count is `library_shown_count()`, the heading's own
   number. The test for whether the button is off (`analyzable == 0`) stays as
   it is.
4. The label. "Analyze search (N)..." is wrong when only a chip narrows the
   table. Recommended: one label, "Analyze shown (N)...", whenever `narrowed()`
   is true, and "Analyze library..." otherwise. This changes text on screen,
   so the user confirms the wording together with the yes to build.
5. Tests. test_library_model.cpp:326 moves to `order()`. test_app_library.cpp
   lines 51 and 256 use `library_shown_count()`. uitest_batch_reports.cpp
   lines 65 and 311 do the same. test_app_click.cpp lines 560-561 follow the
   new label. Add one test: search plus the Not analyzed chip gives
   `batch_scope_charts() == library_shown_count()`, smaller than the All count.
   Add one more: a chip with no search narrows the button.
6. Docs. docs/UserGuide.md lines 49 and 302 say the button follows the search
   and the chip. That page is public, so it goes through the doc-review gate.

Edge cases, which the existing code already covers:
- A chip whose count drops to 0 falls back to All (`refilter`). The button then
  follows All.
- During a batch, rows change status and can leave the chip. The batch runs
  the plan made at confirm time (D79), so the run doesn't change. Only the
  button's number moves, and the button is off while a batch runs.
- With the Analyzed chip on and the re-analyze box unticked, the confirm
  already says "Every chart here already has a result." and won't start.

This design doesn't touch the store or the report files that the
one-record-per-chart-and-profile spec changes. Its files are
src/ui/library_model.{h,cpp}, src/ui/app_state.{h,cpp},
src/ui/library_toolbar.cpp, the tests listed above, and docs/UserGuide.md.

Status: decision recorded. Nothing is built. The build waits for the user's
yes and the label wording (CLAUDE.md rule 3).
