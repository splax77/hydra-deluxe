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

The changed value is which chords are activation points, and so the best path and its score. These screens read it. The numbers are the main session's measurement above; the exact on-screen spelling (comma grouping, which row shows first) is row P12, settled by the live steps.

- Library view, column `Best path` (`src/ui/library_table.cpp` line 375): Caravan reads 2,361,350 today and 2,360,430 after, once its result is re-analysed (question 1 decides when).
- Paths tab: Caravan's best path loses the activation at the coda chord; every activation row and `Score breakdown` shows the new path.
- Preview tab: the fill drawn before that chord (`src/app/preview_view.cpp` line 319 draws fills from `activation_length`) disappears, and `Act >` skips it.
- Path report window (`###pathreport`) and the Path Builder: Caravan's row and timeline show the new path and score.
- `hydra_replay dump` and `hydra_batch` text: the same four charts' scores change; every other chart's output is byte-identical.

## Questions

1. **Bump the results stamp?** `kResultsStamp` in `src/store/stored_versions.h` (line 55, `2.12.1+ch10grid`) says to bump when a chart reads differently, so a stale saved result reads Stale and re-analyses. Sides: (a) bump to the release that ships this, so every saved result in the library re-analyses on next open (D249 analyses the whole library on open); (b) no bump, so the four charts keep their old `Best path` score until the user re-analyses them or a later release bumps. Precedent, for context only: on 2026-10-17 the user chose "No bump (Recommended)" for the events-track reading change. Examples from both ends: under (a) Caravan's row reads Stale, then 2,360,430 after re-analysis, and so do all 327; under (b) Caravan reads 2,361,350 until re-analysed, and a fresh analysis of the same chart reads 2,360,430, two numbers for one chart on one machine. Recommended: (b), following the precedent, because 323 of 327 coda charts compute the same score. If (a), T1 also edits line 55.
2. **Clone Hero 1.0 mode.** Build 1.0 skips a written fill only when it starts exactly on the coda tick (`code/4045/fill_generator.cs` line 88, `==`). Sides: (a) one rule in every mode: `>=`, as ruled for FL-08; Q-40 stays open and records that 1.0 differs; (b) `==` when `FillDeadlineRule::Ch10` is on, `>=` otherwise, which settles Q-40 too. Examples: a `.mid` fill starting one tick after the coda is skipped under (a) in both modes, and lands in 1.0 mode under (b); Caravan's fill, on the tick, is skipped under both. Recommended: (a); the ruling named build 6142, and (b) is a second copy of the rule.

## Tasks

T1 and T2 run in parallel in their own worktrees cut from main. T2's Hydra line names `place_authored_fills`, which already exists, so it does not wait on T1 to start; but T2 cites the three `.chart` listings T1 commits, so T2 merges after T1, and the docs check runs on the merge of both.

### T1: read the coda tick in both readers and skip in the one owner

Owned files: `src/parse/song.cpp`, `src/parse/song.h`, `tests/test_fill_landing.cpp`, `tests/single_owner/rules/parse-fills.json`, the three `.chart` listings `docs/clone-hero/code/6142/events_section_2151830.asm`, `chart_loader_2161a80.asm` and `section_loop_2150c00.asm` with their `.md` files (the main session puts them in T1's worktree from its scratch before T1 starts), and `src/store/stored_versions.h` only if question 1 is (a).

Before coding, T1 settles row P6: it opens `events_section_2151830.asm` at 0x21523A7 to 0x21523E0 in its worktree and confirms the trim, the one-pair bracket strip, the exact compare with `coda` and the first-only store. If the listing is missing or says otherwise, T1 stops and reports.

What to build. `place_authored_fills` takes one more argument, the coda tick as `std::optional<int64_t>`, and skips a fill whose `start >= coda` before it looks for a chord (that is the only new predicate, and it sits beside the comment that names 0x215DA70 as its source). The `.mid` reader finds the tick before pass 2 places fills: the events-track loop that already reads the end event moves ahead of the drum pass, or a small first pass over `mid.events_track()` keeps the first text whose trimmed body is `coda` or `[coda]`, exact case. The `.chart` reader records, next to `events_end` in `ChartParser::parse_sections`, the tick of the first `E` line of `[Events]` whose quoted text, trimmed and with one `[ ]` pair taken off, is exactly `coda`; an `E` line inside the drum section never counts. Both readers share one small function for "is this body the coda", beside `is_end_event`, so the spelling lives once. Comments point at the owner; they do not restate the compare.

Tests, all in `tests/test_fill_landing.cpp` using the `chart_landing` and `mid_landing` shapes already there. `.mid`: coda text event on the events track at tick 960 and a fill from 960 to 1920 gives no activation; the same with the fill from 959 lands on 1935 as today; `[CODA]` at 960 skips nothing; two coda events, the second earlier than a fill the first would spare, keep the first. `.chart`: the same four cases with `E "coda"` and `E "[coda]"` lines in `[Events]`, plus one case where `E coda` sits inside `ExpertDrums` at the fill's start and the fill still lands. A chart with no coda keeps every existing test green. If the test for the 1.0 mode differs (question 2 branch (b)), one more case with `FillDeadlineRule::Ch10` and a fill one tick after the coda.

The single-owner rule: one row in `parse-fills.json` with the question "Which written fills does the coda skip?", owner `place_authored_fills`, pattern on the coda compare, so a second copy anywhere in `src/` fails `tests/test_single_owner`.

Acceptance: the nine boundary cases above pass; `grep -i coda src/` finds the predicate in `place_authored_fills`, the one body test beside `is_end_event`, and its call in each reader, nothing else; the main session's `hydra_replay dump` on the built branch reads 2,360,430 for Caravan and 34,193,190, 4,692,885 and 572,930 for the three `.chart` charts.

### T2: the Clone Hero page, the questions page, the counts and the listings

Owned files: `docs/clone-hero/04-fills.md`, `docs/clone-hero/questions.md`, `docs/clone-hero/README.md`, `docs/clone-hero/code/6142/fill_skip_from_coda_215DA70.asm` and `.md`, `mid_events_coda_21544E0.asm` and `.md`, `fill_init_20FB9A0.asm` and `.md`, `fill_skip_before_215F610.asm` and `.md`, and `docs/clone-hero/code/facts-6142.tsv`.

What to write. FL-08's heading becomes "A written fill that starts on or after the coda never lands". Rule: the fill's one own byte holds `start >= first coda tick`, set by the track finish; the landing step passes over a set byte; both formats set the coda before any finish runs. Label: Proven in the game's code. Evidence: the listings and lines named in rows P1 to P4 and P6, with the caveat the W6 checker asked for: the `.mid` events track is read before the drum tracks, so the coda is set before the skip runs; for `.chart`, the loader's parse-then-finish order. Game code (6142): `0x215DA70`, `0x21544E0`, `0x20FB9A0`, `0x2161A80`, `0x2151830`, `0x20CFF60` lines 86 to 87. Hydra: `place_authored_fills` in src/parse/song.cpp. Ruling: the 2026-11-08 line "Match the game" in `docs/agents/rulings/clone-hero.md`. Found: 2026-10-30, session b02ab3e6; 2026-11-08, session 7b39835b. FL-39's Hydra line changes from "no owner" to `place_authored_fills` with the `>=` rule, and Q-40's "Other end" drops "Whether 1.1 does the same was not read" and states 1.1's `>=` rule (Q-40 itself closes only if question 2 is (b)). README's count table: page 04 goes from Unknown 7 to 6 and Proven in the game's code 32 to 33, and the All pages row follows (96 to 95, 225 to 226).

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

Pass: the `Best path` cell reads 2,360,430 (or Stale, under question 1 branch (a) before re-analysis), and the Paths tab has no activation row at tick 660,600's chord. The text is pasted into the merge report as the before/after of row P12.

### LS4: stop

Calls: `hydra_stop` (never tried)

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| A fill starting on the coda tick is skipped | "What the game does", boundary | T1 | `.mid` and `.chart` test: fill at 960 with coda at 960 gives no activation |
| A fill starting one tick before the coda lands as today | same | T1 | test: fill from 959 lands on 1935 |
| Only the first coda event counts | same | T1 | test: two coda events keep the first |
| The compare is trimmed and case-sensitive | same | T1 | test: `[CODA]` skips nothing; a padded ` coda ` skips |
| A chart with no coda changes nothing | same | T1 | every existing fill test stays green; 323 of 327 scores unchanged in the main session's dump |
| The rule lives in `place_authored_fills` only | Global constraints | T1 | `parse-fills.json` row; `tests/test_single_owner` |
| No new rules key | Global constraints | T1 | `hydra_rules.ini` and `src/core/rules.h` untouched in the diff |
| `.chart` coda comes only from `[Events]`, with one bracket pair stripped; a drum-section `E coda` never counts | "What the game does", `.chart` paragraph | T1 | tests: `E "coda"` and `E "[coda]"` in `[Events]` skip; `E coda` in `ExpertDrums` lands |
| T1 opens the `.chart` listing before writing the `.chart` branch | T1, "Before coding" | T1 | the three listings and their `.md` are in T1's commit; the reviewer refuses a `.chart` branch with P6 still `unchecked` |
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
| P2: the landing step passes over a fill whose byte is set | unchecked | `docs/audit/2026-11-08-clone-hero-unknowns/W6.md` section FL-08 on branch `claude/ch-unk-w6` names `fill_to_activation_20CFF60.asm` lines 86 to 87; the listing is on main but those lines were not opened here | T2 |
| P3: the `.mid` events reader keeps the first `coda` or `[coda]` text and ignores later ones | unchecked | `docs/audit/2026-11-08-clone-hero-unknowns/W6.md` FL-08 names `mid_events_coda_21544E0.asm` lines 245 to 312; not opened here | T2 |
| P4: the text is trimmed and compared case-sensitively | unchecked | `docs/audit/2026-11-08-clone-hero-unknowns/checker-verdicts.md` on branch `claude/ch-unk-handoff`, the FL-08 line, says "trimmed then compared ordinal case-sensitive"; a checker's summary, not the listing | T2 |
| P5: the game's events track is the first track named `events`, any case, and Hydra's `MidiFile::events_track` reads the same | data | `docs/clone-hero/01-reading-a-chart.md` CR-19, label Proven in the game's code; `src/parse/midi.cpp` lines 452 to 498 | checked 2026-11-08 |
| P6: the `.chart` loader parses every section, then runs each track's finish; the `[Events]` reader `0x2151830` trims the quoted text, strips one `[ ]` pair, compares exactly with `coda` and keeps the first; a drum-section `E` line sets no coda | unchecked | the scout FL08-CHART-SCOUT's listings, to be committed as `docs/clone-hero/code/6142/events_section_2151830.asm` (0x21523A7 to 0x21523E0), `chart_loader_2161a80.asm` (0x2161CC4, 0x2161DEE) and `section_loop_2150c00.asm` (0x2150DFB); the main session checked the addresses against them, this planner did not open them | T1 |
| P7: build 1.0 skips only a fill whose start equals the coda tick | code | `docs/clone-hero/code/4045/fill_generator.cs` lines 88 to 91, the `==` compare and `continue` | checked 2026-11-08 |
| P8: Hydra lands every written fill with no coda rule, through `place_authored_fills`, which both readers call | code | `src/parse/song.cpp` lines 341 to 389, calls at 1118 and 1872; a case-insensitive grep of `src/` for `coda` finds nothing | checked 2026-11-08 |
| P9: the four coda listings and their `.md` files are on `claude/ch-unk-w6` and not on main | code | `git ls-tree claude/ch-unk-w6 docs/clone-hero/code/6142/` lists them; `Test-Path` on main's `docs/clone-hero/code/6142/fill_skip_from_coda_215DA70.asm` is false | checked 2026-11-08 |
| P10: the facts table needs a row for every backticked address, written by `tools/ch_mcp/dump_facts.py --tsv` from the dump on this machine | code | `docs/clone-hero/README.md` lines 88 and 94 | checked 2026-11-08 |
| P11: the results stamp's rule is to bump when a chart reads differently, and the user chose no bump for the events-track change | code | `src/store/stored_versions.h` lines 42 to 55; `docs/agents/rulings/clone-hero.md` line for 2026-10-17 | checked 2026-11-08 |
| P12: the Library `Best path` cell shows the score with comma grouping, as 2,361,350 | unchecked | `src/ui/library_table.cpp` line 375 names the column and line 19 includes `group_thousands`, but the cell text function was not opened | LS3 |
| P13: Hydra's `.mid` text reader trims nothing, so T1 must trim | code | `src/parse/midi.cpp` line 131, `decode_chart_text` with no trim; `src/parse/song.cpp` line 1131 tests the raw string's first and last bytes | checked 2026-11-08 |
| P14: the `.chart` reader already keeps `[Events]` lines and the first end event before the drum lines are placed | code | `src/parse/song.cpp` lines 1575 to 1588 and 1872 | checked 2026-11-08 |
| P15: the measured scores and the four dropping charts | unchecked | the main session's own `hydra_replay dump --cap 5` runs on 2026-11-08, stated in this plan's short version; no committed result folder | T1 |
| P16: the user ruled "Match the game" for FL-08 | ruling | `docs/agents/rulings/clone-hero.md`, the 2026-11-08 line | checked 2026-11-08 |
| P17: the docs test checks page shape, listing paths, facts rows and Q-nn naming, and this plan's Premise map shape | code | `tests/test_docs_match_code.cpp` lines 18 to 34, 1509 to 1536 and 1779; `docs/clone-hero/README.md` lines 84 to 92 | checked 2026-11-08 |
