# FL-08: a written drum fill that starts on or after the coda never lands (2026-11-08)

Task FL08-PLAN, session 7b39835b. The user ruled "Match the game" for FL-08 on 2026-11-08 (`docs/agents/rulings/clone-hero.md`). This plan makes Hydra skip every written fill whose start tick is at or after the chart's first coda event, the way Clone Hero build 6142 does.

## The short version

Clone Hero reads one text event called `coda` (or `[coda]`) from a chart. Once it has that tick, it marks every written fill that starts at or after it, and the marked fills never become activation points. Hydra lands every written fill today. `place_authored_fills` in `src/parse/song.cpp` is already the one place both chart readers hand their fills to, so the skip goes there, as one extra check before a fill is placed. Each reader only has to find the coda tick and pass it in.

The main session measured what changes on 2026-11-08, with `hydra_replay dump --cap 5` on a build of main 2a350fc6, Expert Pro Drums with 2x bass. The method: score each chart as it is, then score a scratch copy with every fill that starts at or after the first coda removed; an untouched round-trip copy matched the original on all 327, so the copying itself changes nothing. 330 library charts have a coda. 327 of them have a written fill at or after it. 323 keep their best score. Four drop: Nirvana – Endless, Nameless Setlist (Discography) (`.chart`) 39,273,950 to 34,193,190; Nirvana – Bleach (Full Album) (`.chart`) 5,629,865 to 4,692,885; Ghoul – Bringer of War (`.chart`) 574,110 to 572,930; John Wasson – Caravan (`.mid`) 2,361,350 to 2,360,430. Caravan's dropped fill starts exactly on the coda tick, 660,600, so it is the boundary case from the library.

## What the game does (the rule Hydra copies)

The skip byte is the written fill's one own field. The track finish calls method `0x215DA70`, which walks the track's fill list and stores `fill start >= coda tick` in that byte (P1). The landing step passes over any fill whose byte is set (P2). So the exact boundary is: a fill starting on the coda tick is skipped; a fill starting one tick before lands as it does today. A chart with no coda event skips nothing.

For a `.mid`, the coda is a text event on the events track. The text is trimmed, then compared whole and case-sensitively with `coda` and `[coda]`; `[CODA]` does not count. Only the first such event is kept (P3, P4). The events track is the first track whose name equals `events` ignoring case, which is what Hydra already reads (P5, D252/D253).

For a `.chart`, the game skips the same way. The scout FL08-CHART-SCOUT traced it on 2026-11-08 and the main session checked each address against the scout's listings: the `.chart` loader `0x2161A80` parses every section first (the section dispatch at 0x2161CC4 into `0x2150C00`), then runs each track's finish `0x215C320` (the call at 0x2161DEE), which calls the skip `0x215DA70`. So the coda is set before any skip, wherever `[Events]` sits in the file. The dispatcher sends `Events` to the reader `0x2151830` (the call at 0x2150DFB). That reader takes the quoted text of an `E "..."` line, trims it, strips one surrounding `[ ]` pair, compares it exactly and case-sensitively with `coda`, and stores only the first one (the null test `cmp [chart+0x100], 0` at 0x21523A7, the constructor `0x211BA30` at 0x21523D4, the store at 0x21523E0). So `E "coda"` and `E "[coda]"` both count in `[Events]`. A track-local `E coda` inside `[ExpertDrums]` does not count: the track event handler `0x213CAF0` loads no coda string. The three listings (`docs/clone-hero/code/6142/events_section_2151830.asm`, `chart_loader_2161a80.asm` and `section_loop_2150c00.asm`, each with its `.md`) are in the main session's hands and land with T1's commit, so row P6 stays `unchecked` until T1's executor opens them in the repo; T1 names that row and reads the listing at 0x21523A7 to 0x21523E0 before it writes the `.chart` branch. Three sub-cases stay open and out of the rule (see "Out of scope").

Clone Hero 1.0 is narrower: its generator skips only a fill that starts exactly on the coda tick (`==`, FL-39, P7). Hydra has a 1.0 mode for generated fills (`FillDeadlineRule::Ch10`). Whether written fills in that mode follow 1.0's `==` or 1.1's `>=` is question 2.

## Global constraints

The skip rule is worked out in one place, `place_authored_fills`, and nowhere else; the readers only find the coda tick. No new number is introduced, so `hydra_rules.ini` gains no key. Agents build nothing and run no `hydra_tests.exe`; the main session builds each branch and runs the merge's tests, including the docs check `hydra_tests.exe "-tc=docs match code*"`. Every task is one executor under 100 tool calls. The plan file leaves the repo in the merge that finishes T1 and T2, and any line that still names it says `docs/superpowers/plans/2026-11-08-fl08-coda-fills.md` (removed; `git show <commit>:<path>`).

## What the user will see (rule 11)

The changed value is which chords are activation points, and so the best path and its score. These screens read it. The numbers are the main session's measurement above; the Library cell's spelling is row P12.

- Library view, column `Best path` (`best_path_label` in `src/ui/library_model.cpp`, P12): Caravan's cell starts `2,361,350` today and `2,360,430` after, once its result is re-analysed (question 1 decides when); the path text after the score loses the activation at the coda chord.
- Paths tab: Caravan's best path loses the activation at the coda chord; every activation row and `Score breakdown` shows the new path.
- Preview tab: the fill drawn before that chord (`src/app/preview_view.cpp` line 319 draws fills from `activation_length`) disappears, and `Act >` skips it.
- Path report window (`###pathreport`) and the Path Builder: Caravan's row and timeline show the new path and score.
- `hydra_replay dump` and `hydra_batch` text: the same four charts' scores change; every other chart's output is byte-identical.

## Questions

1. **Bump the results stamp?** `kResultsStamp` in `src/store/stored_versions.h` (line 55, `2.12.1+ch10grid`) says to bump when a chart reads differently, so a stale saved result reads Stale and re-analyses. Sides: (a) bump to the release that ships this, so every saved result in the library re-analyses on next open (D249 analyses the whole library on open); (b) no bump, so the four charts keep their old `Best path` score until the user re-analyses them or a later release bumps. Precedent, for context only: on 2026-10-17 the user chose "No bump (Recommended)" for the events-track reading change. The stamp covers the whole library, not just the coda charts. What you would see: under (a) every row in the Library reads Stale until it is re-analysed (D249 does that on open), and Caravan then reads 2,360,430; under (b) Caravan keeps reading 2,361,350 until you re-analyse it, while a fresh analysis of the same chart reads 2,360,430, so one chart can show two numbers on one machine until then. Recommended: (b), following the precedent, because 323 of the 327 coda charts compute the same score, so a whole-library re-analysis buys four corrected numbers. If (a), T1 also edits line 55.
2. **Clone Hero 1.0 mode.** Build 1.0 skips a written fill only when it starts exactly on the coda tick (`code/4045/fill_generator.cs` line 88, `==`). Sides: (a) one rule in every mode: `>=`, as ruled for FL-08; Q-40 stays open and records that 1.0 differs; (b) `==` when `FillDeadlineRule::Ch10` is on, `>=` otherwise, which settles Q-40 too. What you would see: nothing changes in the default mode under either answer. In 1.0 mode, a fill starting one tick after the coda is skipped under (a) and lands under (b); Caravan's fill, on the tick, is skipped under both, so its score is the same either way. Recommended: (b). You ruled "Match the game", Hydra already has a 1.0 mode whose job is to match build 1.0, and build 1.0 skips only the fill on the tick (P7). It costs one comparison inside the same function, so the rule is still written once. It also settles Q-40, which is otherwise left open.

## Tasks

T1 and T2 run in parallel in their own worktrees cut from main. T2's Hydra line names `place_authored_fills`, which already exists, so it does not wait on T1 to start; but T2 cites the three `.chart` listings T1 commits, so T2 merges after T1, and the docs check runs on the merge of both.

### T1: read the coda tick in both readers and skip in the one owner

Owned files: `src/parse/song.cpp`, `src/parse/song.h`, `tests/test_fill_landing.cpp`, `tests/single_owner/rules/parse-fills.json`, the three `.chart` listings `docs/clone-hero/code/6142/events_section_2151830.asm`, `chart_loader_2161a80.asm` and `section_loop_2150c00.asm` with their `.md` files (the main session puts them in T1's worktree from its scratch before T1 starts), and `src/store/stored_versions.h` only if question 1 is (a).

Before coding, T1 settles row P6: it opens `events_section_2151830.asm` at 0x21523A7 to 0x21523E0 in its worktree and confirms the trim, the one-pair bracket strip, the exact compare with `coda` and the first-only store. If the listing is missing or says otherwise, T1 stops and reports.

What to build. `place_authored_fills` takes one more argument, the coda tick as `std::optional<int64_t>`, and skips a fill whose `start >= coda` before it looks for a chord (that is the only new predicate, and it sits beside the comment that names 0x215DA70 as its source). If question 2 is (b), it also takes `FillDeadlineRule` (both parsers already hold it as `fills_rule_`) and the predicate becomes `start == coda` under `Ch10`. The `.mid` reader must know the coda tick before it places fills. Today the order is wrong for that: the drum pass places fills at line 1118, and the events loop that reads the end event runs after it (pass 3) and only looks at text inside `[ ]` (P18). So that loop cannot simply move up; a bare `coda` would never reach it. T1 adds a small first pass over `mid.events_track()`, before the drum pass, that keeps the tick of the first text whose trimmed body is `coda` or `[coda]`, exact case, and leaves pass 3 where it is. The `.chart` reader records, next to `events_end` in `ChartParser::parse_sections`, the tick of the first `E` line of any `[Events]` section whose quoted text, trimmed and with one `[ ]` pair taken off, is exactly `coda`. Unlike `events_end`, it is not cleared when a second `[Events]` header opens (P19). An `E` line inside the drum section never counts. Both readers share one small function for "is this body the coda", beside `is_end_event`, so the spelling lives once. Comments point at the owner; they do not restate the compare.

Tests, all in `tests/test_fill_landing.cpp`, built on the `chart_landing` and `mid_landing` shapes already there (each gains a coda argument). In `mid_landing` the fill starts at tick 960 and the extra chord `after` is 15, so an uncoda'd fill lands on 1935; in `chart_landing` at resolution 192 the fill starts at 384. The cases, with the coda tick set against the fill start (the boundary is on both ends):
- `.mid`, coda text `coda` at 960 (the fill's start tick): no activation. Same with text `[coda]`: no activation. This is the skipped end.
- `.mid`, coda at 961 (the fill's start plus one, so the fill starts one tick before the coda): the fill lands on 1935 as today. This is the landing end.
- `.mid`, coda text ` coda ` (padded) at 960: no activation, so the trim is tested. `[CODA]` at 960: the fill lands (case counts).
- `.mid`, two coda events, the first at 960 and a second at 2400: the fill is skipped, because only the first counts (a reader that kept the last would land it).
- Every `.mid` case above puts the events track after the drum track in the file, which is the order that fails if the coda is read too late.
- `.chart`: the skip, landing, padded, `[CODA]` and first-only cases above, with `E "coda"` and `E "[coda]"` lines in `[Events]` at 384 and 385; a case with the `[Events]` section after the drum section in the file; a case with two `[Events]` sections, a coda at 384 in the first and one at 2400 in the second (the fill is skipped: the first coda is kept across sections); and one case where `E coda` sits inside `ExpertDrums` at the fill's start and the fill still lands.
- A chart with no coda keeps every existing test green.
- If question 2 is (b): the same fill one tick after the coda skips when called with `FillDeadlineRule::Ch11` and lands with `Ch10`, and a fill exactly on the coda skips in both.

The single-owner rule: one row in `parse-fills.json` with the question "Which written fills does the coda skip?", owner `place_authored_fills`, pattern on the coda compare, so a second copy anywhere in `src/` fails `tests/test_single_owner`.

Acceptance: every test case above passes; in `grep -i coda src/` the only code that decides anything is the `>=` predicate in `place_authored_fills`, the one body test beside `is_end_event`, and the call to it in each reader (names, comments and the stored tick are not decisions); the main session's `hydra_replay dump` on the built branch reads 2,360,430 for Caravan and 34,193,190, 4,692,885 and 572,930 for the three `.chart` charts.

### T2: the Clone Hero page, the questions page, the counts and the listings

Owned files: `docs/clone-hero/04-fills.md`, `docs/clone-hero/questions.md`, `docs/clone-hero/README.md`, `docs/clone-hero/code/6142/fill_skip_from_coda_215DA70.asm` and `.md`, `mid_events_coda_21544E0.asm` and `.md`, `fill_init_20FB9A0.asm` and `.md`, `fill_skip_before_215F610.asm` and `.md`, and `docs/clone-hero/code/facts-6142.tsv`.

What to write. FL-08's heading becomes "A written fill that starts on or after the coda never lands". Rule: the fill's one own byte holds `start >= first coda tick`, set by the track finish; the landing step passes over a set byte; both formats set the coda before any finish runs. Label: Proven in the game's code. Evidence: the listings and lines named in rows P1 to P4 and P6, with the caveat the W6 checker asked for: the `.mid` events track is read before the drum tracks, so the coda is set before the skip runs; for `.chart`, the loader's parse-then-finish order. Game code (6142): `0x215DA70`, `0x21544E0`, `0x20FB9A0`, `0x2161A80`, `0x2151830`, `0x20CFF60` lines 86 to 87. Hydra: `place_authored_fills` in src/parse/song.cpp. T2 also corrects the sentence in `mid_events_coda_21544E0.md` that says the trim "was not read": the listing calls `MemoryExtensions$$Trim` at 021546e3 (P4). Ruling: the 2026-11-08 line "Match the game" in `docs/agents/rulings/clone-hero.md`. Found: 2026-10-30, session b02ab3e6; 2026-11-08, session 7b39835b. FL-39's Hydra line changes from "no owner" to `place_authored_fills` with the `>=` rule, and Q-40's "Other end" drops "Whether 1.1 does the same was not read" and states 1.1's `>=` rule (Q-40 itself closes only if question 2 is (b)). README's count table: page 04 goes from Unknown 7 to 6 and Proven in the game's code 32 to 33, and the All pages row follows (96 to 95, 225 to 226).

The four listings are on branch `claude/ch-unk-w6` and not on main (P9). If they have not merged by the time T2 starts, T2 takes them with `git checkout claude/ch-unk-w6 -- <the eight paths>`. Every address the block names needs a facts row; T2 runs `py -I tools/ch_mcp/dump_facts.py --check` and, if rows are missing, `--tsv` to write them (the dump is on this machine, P10).

Acceptance: the docs check `hydra_tests.exe "-tc=docs match code*"`, run by the main session, passes: the seven lines in order, every listing path exists with its `.md`, every address has a facts row, and Q-40 is still named by FL-39's Ruling line. The README counts match a count of `###` headings and `Label:` lines on page 04.

## The live steps

The main session runs these after T1 is built, in window mode on a snapshot copy of the library, to show the user the screens rule 11 names. Cap: 2 runs and 10 minutes each. On a failure, report the step and the tool's exact error and stop. The song is reached by its exact title, never by scrolling.

### LS1: start Hydra on the library snapshot

Calls: `hydra_start('window')` (never tried)

Pass: `hydra_state` reports the window open with the library loaded.

### LS2: open Caravan

Calls: `hydra_open_song('Caravan')` (never tried)

Pass: the Details view shows John Wasson – Caravan.

### LS3: read the Library row and the Paths tab

Calls: `hydra_text` on the Library `Best path` cell and the Paths tab (never tried)

Pass: the `Best path` cell starts with 2,360,430 (or reads Stale, under question 1 branch (a) before re-analysis), and the Paths tab has no activation row at tick 660,600's chord. The text is pasted into the merge report as the after half of the before/after; the before half is 2,361,350 from the measurement above.

### LS4: stop

Calls: `hydra_stop` (never tried)

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| A fill starting on the coda tick is skipped | "What the game does", boundary | T1 | `.mid` and `.chart` test: fill at 960 with coda at 960 gives no activation |
| A fill starting one tick before the coda lands as today | same | T1 | test: coda at 961 with the fill at 960 lands on 1935 |
| Only the first coda event counts | same | T1 | test: first coda at 960, second at 2400, fill at 960 is skipped |
| The compare is trimmed and case-sensitive | same | T1 | test: `[CODA]` skips nothing; a padded ` coda ` skips |
| A chart with no coda changes nothing | same | T1 | every existing fill test stays green; 323 of 327 scores unchanged in the main session's dump |
| The rule lives in `place_authored_fills` only | Global constraints | T1 | `parse-fills.json` row; `tests/test_single_owner` |
| No new rules key | Global constraints | T1 | `hydra_rules.ini` and `src/core/rules.h` untouched in the diff |
| `.chart` coda comes only from `[Events]`, with one bracket pair stripped; a drum-section `E coda` never counts | "What the game does", `.chart` paragraph | T1 | tests: `E "coda"` and `E "[coda]"` in `[Events]` skip; `E coda` in `ExpertDrums` lands |
| T1 opens the `.chart` listing before writing the `.chart` branch | T1, "Before coding" | T1 | the three listings and their `.md` are in T1's commit; the reviewer refuses a `.chart` branch with P6 still `unchecked` |
| The `.mid` coda is known before fills are placed, whatever the track order in the file | T1, "What to build" | T1 | the `.mid` tests put the events track after the drum track and still skip |
| A second `[Events]` section or a second coda event never replaces the first | T1, "What to build" | T1 | the two-coda `.mid` case and the two-section `.chart` case both skip |
| 1.0 mode follows question 2 | question 2 | T1 | if (b): the `Ch10` and `Ch11` one-tick-after cases differ as listed; if (a): no `Ch10` test |
| Stamp bump follows question 1 | question 1 | T1 | line 55 of `stored_versions.h` changed if and only if (a) |
| FL-08 block in the page shape, every listing present, every address a facts row | T2 | T2 | docs check passes on the main session's run |
| README counts match the page | T2 | T2 | reviewer counts `###` and `Label:` lines on page 04 |
| Q-40 remains named by a block | T2 | T2 | docs check's Q-nn rule |
| Every screen shows the new value | rule 11 section | LS1 to LS4 | `hydra_text` output pasted into the merge report |
| Plan leaves the repo with its last merge | Global constraints | main session | docs check on the merge |

## Out of scope

FL-10 (two written fills on one chord) and the generation gate for skipped fills (whether a skipped fill still stops generated fills, FL-27) stay as they are; the W6 checker left the gate untraced, so a change there would be a guess. Three `.chart` sub-cases the scout left open stay out of the rule and are recorded on the page as open, not decided: an unquoted `E coda` line in `[Events]` (the game's quote scan has no newline stop, so what it reads is not known), the byte `[ctx+0x2c]` that gates both the coda store and the finish (a normal full load takes both), and a twin reader `0x2163170` with no direct callers. Caller lists are direct-only (byte scan).

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: method `0x215DA70` stores `fill start >= coda tick` in the fill's byte at `+0x20` for every fill of the track | code | `docs/clone-hero/code/6142/fill_skip_from_coda_215DA70.asm` on branch `claude/ch-unk-w6`, the `cmp qword ptr [rcx + 0x10], rax; setge al; mov byte ptr [rcx + 0x20], al` at 0215db69 to 0215db70, read with `git show` | checked 2026-11-08 |
| P2: the landing step passes over a fill whose byte is set | code | `docs/clone-hero/code/6142/fill_to_activation_20CFF60.asm` lines 86 to 87, `cmp byte ptr [rsi + 0x20], 0` then `jne` back to the loop head, opened on main | checked 2026-11-08 |
| P3: the `.mid` events reader accepts the text `[coda]` or `coda` and keeps only the first | code | `docs/clone-hero/code/6142/mid_events_coda_21544E0.asm` on branch `claude/ch-unk-w6`, the two `SequenceEqual` compares ending at 02154a5c and the `cmp qword ptr [rsi + 0x100], 0` at 02154a6b, read with `git show` | checked 2026-11-08 |
| P4: the text is trimmed and compared case-sensitively | code | the same listing: `call MemoryExtensions$$Trim` at 021546e3, and the trimmed span in `rbx`, `r15d`, `edi` feeds every compare after it; `SequenceEqual<char>` is an ordinal compare. The listing's `.md` says the trim "was not read", so T2 corrects that sentence; `checker-verdicts.md` FL-08 on branch `claude/ch-unk-handoff` agrees ("trimmed then compared ordinal case-sensitive") | checked 2026-11-08 |
| P5: the game's events track is the first track named `events`, any case, and Hydra's `MidiFile::events_track` reads the same | data | `docs/clone-hero/01-reading-a-chart.md` CR-19, label Proven in the game's code; `src/parse/midi.cpp` lines 452 to 498 | checked 2026-11-08 |
| P6: the `.chart` loader parses every section, then runs each track's finish; the `[Events]` reader `0x2151830` trims the quoted text, strips one `[ ]` pair, compares exactly with `coda` and keeps the first; a drum-section `E` line sets no coda | unchecked | the scout FL08-CHART-SCOUT's listings, to be committed as `docs/clone-hero/code/6142/events_section_2151830.asm` (0x21523A7 to 0x21523E0), `chart_loader_2161a80.asm` (0x2161CC4, 0x2161DEE) and `section_loop_2150c00.asm` (0x2150DFB); the main session checked the addresses against them, this planner did not open them | T1 |
| P7: build 1.0 skips only a fill whose start equals the coda tick | code | `docs/clone-hero/code/4045/fill_generator.cs` lines 88 to 91, the `==` compare and `continue` | checked 2026-11-08 |
| P8: Hydra lands every written fill with no coda rule, through `place_authored_fills`, which both readers call | code | `src/parse/song.cpp` lines 341 to 389, calls at 1118 and 1872; a case-insensitive grep of `src/` for `coda` finds nothing | checked 2026-11-08 |
| P9: the four coda listings and their `.md` files are on `claude/ch-unk-w6` and not on main | code | `git ls-tree claude/ch-unk-w6 docs/clone-hero/code/6142/` lists them; `Test-Path` on main's `docs/clone-hero/code/6142/fill_skip_from_coda_215DA70.asm` is false | checked 2026-11-08 |
| P10: the facts table needs a row for every backticked address, written by `tools/ch_mcp/dump_facts.py --tsv` from the dump on this machine | code | `docs/clone-hero/README.md` lines 88 and 94 | checked 2026-11-08 |
| P11: the results stamp's rule is to bump when a chart reads differently, and the user chose no bump for the events-track change | code | `src/store/stored_versions.h` lines 42 to 55; `docs/agents/rulings/clone-hero.md` line for 2026-10-17 | checked 2026-11-08 |
| P12: the Library `Best path` cell of a Ready row reads the score with comma grouping, two spaces, then the path string, as `2,361,350  <path>`; a Stale row reads `Stale` | code | `src/ui/library_model.cpp` lines 91 to 130, `best_path_label` | checked 2026-11-08 |
| P13: Hydra's `.mid` text reader trims nothing, so T1 must trim | code | `src/parse/midi.cpp` line 131, `decode_chart_text` with no trim; `src/parse/song.cpp` line 1131 tests the raw string's first and last bytes | checked 2026-11-08 |
| P18: Hydra's `.mid` events loop is pass 3, after the drum pass has placed fills, and it keeps only text that starts with `[` and ends with `]`, so a bare `coda` never reaches it | code | `src/parse/song.cpp` lines 1118 (the `place_authored_fills` call) and 1125 to 1138 | checked 2026-11-08 |
| P19: the `.chart` reader clears `events_end` when a second `[Events]` header opens, while the game's coda slot sits on the chart object, so the game's first coda is first across the whole file | code | `src/parse/song.cpp` lines 1611 to 1614; the game's null test `cmp [chart+0x100], 0` is in row P6's listing | checked 2026-11-08 |
| P20: a `.mid` test can put an events track after the drum track | code | `tests/midi_util.h`: `text_event` (line 66), `smf_tracks`, and `smf_drums` (line 108) which builds timing then drums | checked 2026-11-08 |
| P14: the `.chart` reader already keeps `[Events]` lines and the first end event before the drum lines are placed | code | `src/parse/song.cpp` lines 1575 to 1588 and 1872 | checked 2026-11-08 |
| P15: the measured scores and the four dropping charts | unchecked | the main session's own `hydra_replay dump --cap 5` runs on 2026-11-08, stated in this plan's short version; no committed result folder | T1 |
| P16: the user ruled "Match the game" for FL-08 | ruling | `docs/agents/rulings/clone-hero.md`, the 2026-11-08 line | checked 2026-11-08 |
| P17: the docs test checks page shape, listing paths, facts rows and Q-nn naming, and this plan's Premise map shape | code | `tests/test_docs_match_code.cpp` lines 18 to 34, 1509 to 1536 and 1779; `docs/clone-hero/README.md` lines 84 to 92 | checked 2026-11-08 |
