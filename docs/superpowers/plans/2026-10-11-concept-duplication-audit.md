# Plan: the concept duplication audit (module-level copies across src/)

Planner: task concept-audit-plan, session 8bffee2b, 2026-10-11. Read-only; nothing here is built yet. Every number below says where it comes from. The file and line counts are from `Get-Content` over the `.cpp` and `.h` files in the checkout at commit 60725a73 on 2026-10-11. Every line count in this plan counts non-blank lines; `wc -l` counts 44,999 lines in all, blanks included. The lead counts are from four small PowerShell probes run in this session over `src/`; the probes are described in phase 1 and their thresholds are question 8 for the user.

## What is wrong and what this audit looks for

The derive-once review finds a rule written twice when the two copies share inputs: the same field, constant or unit shows up in both, and the reviewer's grep meets the original. That catches the same formula, the same threshold, the same sentence. It does not catch a module that builds its own version of an idea under its own names. The path report and the dmleaderboards (DM) report each have a row type, a status vocabulary, a column builder and a window. A reviewer reading a diff of one of them sees a well-formed module, not a second copy of the other.

A module-level copy, for this audit, is a second home for one idea. An idea is a question in plain words with named inputs: "does Hydra have a result for this chart", "which settings does this page read", "how is an empty cell sorted". A home is a type, an enum, a set of status words, a helper family, or a pipeline step that answers it. When two modules each hold a home for one idea, the next change to the idea has to be made twice, and one day it will be made once.

Three leads from the main session, checked against the code in this session, show the three shapes it takes.

**One word, two rules.** The library table's status word comes from `status_label` in `src/ui/library_model.cpp` (lines 67 to 74). It maps the store's `RecordStatus` enum (`src/store/record_store.h` line 199) to "Analyzed", "Stale" or "Not analyzed", one word per enum value. The DM report sets its own status in `collect_dm_rows` in `src/app/dm_report.cpp` (lines 109 to 128). A chart with no joined record is "not analyzed" when the library lists its hash and "not in library" otherwise; a joined record with no scored path is "no paths". The joined records come from `records_by_hash` in `src/app/report.cpp` (lines 155 to 169), which reads `store.list_records`, and CONTEXT.md says a listing returns only ready rows. So a chart whose row is Stale reads "Stale" in the library and "Not analyzed (in your library)" on the DM page (the label is in `status_choices`, `src/app/dm_report_view.cpp` line 189). The DM page also sees only one chart mode at SP cap 4. These are two rules under one word, which the user's one-word-one-meaning rule for public text forbids. The lead holds; the scout for group G1a confirms the Stale case against `list_records` in `src/store/record_store.cpp` before it is called a finding. A third home sits next to them: the tooltip sentence `kNotAnalyzedText` in `src/app/user_messages.h` (line 71), which has its own scan row already.

**Half-shared machinery.** Both report views build on `Column<Row>` and `TableView<Row>` in `src/app/report_view.h`, which is the shared framework. But `text_column` is written in `src/app/path_report_view.cpp` (lines 26 to 36) and again in `src/app/dm_report_view.cpp` (lines 37 to 44), differing only in a `definition` argument. `ms_key` (path, lines 21 to 23) and `optional_key` (DM, lines 46 to 49) are one rule, "an empty optional sorts as an empty key, else as its number", in two spellings. The Song, Artist and Charter columns are declared with the same look in both (path lines 83 to 85, DM lines 64 to 66). The clone scan (ADR 0025) cannot see any of this, because each copy is under its eight-line window. The lead holds with one correction: `src/app/fill_report.cpp` is not a third `TableView` report. It is an HTML page only, built on `html::page_template` from `src/app/html_page.cpp` (its local `page_template` at line 157 is a wrapper that fills two rule names, not a copy). It does carry a third status vocabulary of its own ("1.1 higher", "1.0 higher", "same", "only 1.0", "only 1.1", "in both", lines 50 to 56), so it belongs in the same side-by-side read.

**Two hand-kept lists of one fact.** `settings_change_touches` exists in `src/app/report.cpp` (lines 431 to 437) and `src/app/dm_report.cpp` (lines 237 to 241). Each returns whether a settings change reaches its page, by comparing the two or three settings its author believed the generator reads, and each defends the list in a comment. Neither is derived from what `generate_report` or `generate_dm_report` reads. The caller in `src/ui/app_state.cpp` (lines 866 to 870) asks both. The lead holds. It is the cleanest example of the kind: the fact "which settings does this page depend on" has no owner, so two modules each guess it.

All three leads were also found by the mechanical probes in phase 1 without anyone pointing at them: the string probe lists "Not analyzed" in four files across two folders, and the function-name probe lists `text_column` and `settings_change_touches`. That is the evidence that phase 1 earns its place at the front.

## What counts as a finding

A finding is one idea with two or more homes in `src/`, where the evidence is written as the derive-once review's step 3 asks: both homes as tables of inputs to answers, with the edges in (an empty list, exactly 0, a Stale row, a chart in no mode, a value at the window edge), and either the input where the tables differ or a sentence saying they agree today and what change would split them.

Findings come in three kinds, and the kind decides what happens next.

**K1, same idea, same rule, two homes.** The tables agree on every input. This is a merge candidate: one home stays, the other calls it. `ms_key` and `optional_key` are K1. A K1 fix is code-only unless the merge changes a displayed text, a stored record or a score.

**K2, same word, different rules.** The tables differ, and at least one home is a word the user sees, a stored value or a score. "Not analyzed" is K2. A K2 finding is never fixed by an agent on its own; it becomes a question for the user, written with the real predicates quoted from both homes and an example input from each end (the user's rule "Questions describe the real rule").

**K3, parallel machinery on purpose.** Two modules specialise one shared framework, and the parallel parts hold only the per-module facts. The test is this: list what each parallel part knows. If every entry is a fact about that module (which columns, which status choices, which noun the count line uses) and every rule (how to sort, how to filter, how to measure widths, how to draw the frame) is called from the shared place, it is K3 and not a finding; the scout records it as "read, parallel on purpose" so nobody re-raises it. If any entry is a rule, that rule is a K1 or K2 finding on its own. `DmWindow` in `src/ui/dm_report_window.cpp` and `PathWindow` in `src/ui/path_report_window.cpp` share eight field names (the struct probe) and each has its own `rebuild`, `apply_filters` and `draw_body`. Both sit on `report_frame` and `TableView`. Whether the trio is K3 or hides a K1 is exactly what the group G1b scout decides by that test.

**K0, not a finding.** Two things look alike and answer different questions. The probes raise these too, and the scouts drop them with one sentence. Two examples from this session: `ChipToken` appears in `src/app/report.h` and `src/ui/theme.h`, but the second is a forward declaration of the first (theme.h line 18); and `DepthMode` in `src/search/engine.h` is the search's score-range unit while `DepthMode` in `src/render/highway_draw.h` is a renderer depth test. The second pair is still a naming note, because one word names two things, but it is internal and not a copy.

Why the evidence has to be tables and not a judgement: type-4 clones (same behaviour, different syntax) are the kind every text tool misses by construction, and the one measurement I found of a language model reading for them alone gave a recall of 0.25 on type-4 against 0.92 on type-3 ([Dou et al., survey of LLMs on clone detection, arXiv 2308.01191](https://arxiv.org/pdf/2308.01191); the type taxonomy is from [Svajlenko and Roy's evaluation survey, arXiv 2006.15682](https://arxiv.org/pdf/2006.15682)). A scout that says "these look like the same idea" is guessing three times out of four. A scout that writes both tables and finds the input where they differ, or shows they agree on the edges, is not.

## The phases

Seven phases. Phases 1 to 3 find candidates, 4 verifies them, 5 takes the user's decisions, 6 fixes, 7 stops it recurring. Phases 2 and 3 run at the same time. Every agent reads `docs/agents/brief-preamble.md` first and writes its status line to `hooks\state\status`. Rule 2 applies: scouts and verifiers on Sonnet, executors on Opus, reviewers on Sonnet. Every agent fits in 100 tool calls and 20 minutes, and the splits below are sized for that. No agent runs a build or a test in phases 1 to 5; nothing in them needs one.

### Phase 1: mechanical leads

One script, run once by the main session, prints clues that two modules share an idea. It is not a scan test and finds no findings; it hands the scouts a list to start from. It reads `src/` only. Its five probes, with the counts they gave in this session:

1. A string literal of 12 or more characters that appears in two or more files: 56 strings, of which 17 cross a top-level folder. The cross-folder ones include "Not analyzed" (four files), "Show in Preview" (four), "cannot open file:" (four), "Analyze library" (three, including `src/cli/batch.cpp`), "Clone Hero 1.1" and "Clone Hero 1.0" (`src/search/graph.h` and `src/ui/library_parts.h`), "squeeze in -316 ms" (`src/app/path_view.h` and `src/core/model.h`) and "Not in your library" (`src/app/dm_report_view.cpp` and `src/ui/dm_report_window.cpp`).
2. A free function name defined in two or more `.cpp` files: 9 names. `rebuild` (three files), `apply_filters`, `draw_body` and `text_column` (the two report pairs), `settings_change_touches`, `count_line` (`src/app/report_view.cpp` and `src/ui/report_window.cpp`), `read_all` (`src/audio/decode.cpp` and `src/core/winstr.cpp`), `starts_with` (`src/core/strutil.cpp` and `src/core/winstr.cpp`), `page_template` (the wrapper above, a K0).
3. Two structs in different files sharing four or more field names: 17 pairs out of 194 structs with three or more fields. The strongest: `DmReportRow` and `net::DmScore` (8 shared), `ScanItem` and `ChartLibraryEntry` (8), `DmWindow` and `PathWindow` (8), `ScanItem` and `ChartCacheEntry` (6), `BatchProgress` and `library_jobs::Snapshot` (6), `ReplayScore` and `ChordScoreRow` (6), `SummaryLookup` and `LibraryRow` (5), `ReplayScore` and `CategoryScores` (5), `EdgeView` and `ScoreGraphEdge` (5), and `ChartMeta` against each of `ScanItem`, `ChartLibraryEntry` and `ChartCacheEntry` (4 each). The last group says a chart's identity and metadata has four shapes across `src/app/analysis` and `src/store/record_store.h`.
4. A `RecordStore` method called from two or more files: 5 methods (`chart_library_cache`, `engine_mode`, `get_summaries`, `list_chart_library`, `save_analysis`). Two callers of one owner is fine on its own; the lead is the logic around the call, which may be worked out twice.
5. An enum or struct name defined twice: `ChipToken` (a K0, above), `DepthMode`, `State` (`src/app/preview_view.h` and `src/ui/app_state.h`), `Step` (`src/ui/library_jobs.h` and `src/ui/preview_load_job.h`). From the enum listing, 66 enums in `src/`.

Output: `docs/audit/2026-10-11-concept-audit/leads.md`, one line per clue with the files, grouped by the sibling group that will read it (phase 2) and marked with the concept it touches where one is obvious. The script itself is saved beside it as `leads.ps1` so the numbers can be reproduced; it is a record of the audit, not a tool, and it is not added under `tools/` (question 3).

Who: the main session, since the probes already exist in this session's scratch and need only tidying. Cap: one hour of main-session time and one run; if a probe takes over five minutes on the tree, drop it and say so in `leads.md`. Stop condition: the file exists with every probe's count at the top.

Thresholds: 12 characters, 4 shared fields, 3 fields minimum. These are numbers with no decision behind them, so they are question 8. They only shape the lead list; a lower threshold adds noise, a higher one drops clues, and neither changes what a finding is.

### Phase 2: side-by-side reads of sibling groups

Each group is a set of modules doing one kind of job, read by one Sonnet scout with one question: what does each module work out that another module in the group already has? The scout reads every file in its group in full, takes the phase 1 leads for its group as its first stops, and writes one candidate per idea it finds with two homes. It does not grade candidates as findings; it writes the two homes and its first draft of both tables, and leaves the verdict to phase 4.

The groups, with their line counts from the per-file listing. A file can sit in two groups when the question needs both sides; the overlap is on purpose and is marked. Each group is sized at or under about 3,800 lines, which is my estimate of what one scout can read carefully and still write up inside 100 calls and 20 minutes; a Read call returns about 800 to 1,000 lines of code before the tool truncates, so 3,800 lines is four to five reads plus greps. The sums are of the per-file counts, added by hand and checked once; the group lines add to more than 39,724 because `search/graph.h` and `store/record_store.h` are read by two groups each. The earlier audit's sweep agents read about 185 functions each (`docs/audit/2026-10-03-derivation-audit.md`, step 4: 1,847 functions over ten agents), which is a similar load.

| Group | Question | Files | Lines |
|---|---|---|---|
| G1a | Do the three reports each build their own generator pieces? | app/report.cpp+h, dm_report.cpp+h, fill_report.cpp+h, path_report_view.cpp+h, dm_report_view.cpp+h, report_view.cpp+h, html_page.cpp+h, display_format.cpp+h | 2,727 |
| G1b | Do the two report windows and the library table each own table rules? | ui/report_window.cpp+h, path_report_window.cpp, dm_report_window.cpp, report_state.h, library_table.cpp, library_model.cpp+h, column_widths.cpp+h, app/library_query.cpp+h | 3,009 |
| G2 | How many shapes does "a chart and its library row" have? | app/analysis.cpp+h, store/record_store.h, ui/library_jobs.cpp+h, cli/batch.cpp, cli/fillcompare.cpp | 3,109 |
| G3 | Who knows which settings an analysis, a page or a view depends on? | app/config.cpp+h, search/pather.h, search/engine.h, ui/settings_bar.cpp, ui/app_state.h, plus the `settings_change_touches` pair and `app_state.cpp` lines 800 to 900 | 1,677 plus the excerpt |
| G4a | Is a chord's score broken down in more than one place? | core/scoring.cpp+h, core/replay.cpp+h, core/backend_value.h, core/stars.cpp+h, search/graph.h, app/dynamics_breakdown.cpp+h, ui/dynamics_tab.cpp, ui/stars_tab.cpp | 1,850 |
| G4b | Does the engine keep its own views of the graph's types? | search/engine.cpp, search/graph.cpp+h, search/pather.cpp | 3,031 |
| G5 | Is a path, an activation or a squeeze described twice? | app/path_view.cpp+h, core/model.cpp+h, core/squeeze_rating.cpp+h, core/sqout_chord.h, ui/paths_tab.cpp, ui/details_panel.cpp, details_parts.h, details_view.h, activation_row_layout.h | 3,823 |
| G6a | Does the Preview's app side and UI side each hold the playhead, the overlay or the scrub rule? | app/preview_view.cpp+h, preview_source.cpp+h, preview_clock.h, song_length.cpp+h, ui/preview_tab.cpp, preview_controller.cpp+h, preview_transport.cpp+h | 3,569 |
| G6b | Do the renderer and the load job re-derive what the Preview app side stores? | render/ (all 12 files), ui/preview_load_job.cpp+h | 3,196 |
| G7a | Does the song model work out the same thing for each format or difficulty? | parse/song.cpp+h, timesig.h, note_shuffle.cpp+h | 2,397 |
| G7b | Do the four file readers share one idea of a chart file, its timing and its sniffing? | parse/midi.cpp+h, chart_files.cpp+h, sng.cpp+h, srb.cpp+h, core/timing.cpp+h, core/audio_sniff.cpp+h, core/little_endian.h, image/decode.cpp+h | 1,704 |
| G8 | Do the audio readers each own the read loop, the length rule or the error path? | audio/ (all 20 files) | 2,278 |
| G9 | Are strings, errors, file access and the network client written once? | core/strutil.cpp+h, winstr.cpp+h, error_kind.h, version.h, app/user_messages.cpp+h, rules_file.cpp+h, core/rules.cpp+h, app/report_files.cpp+h, allocator.cpp+h, work_pool.h, net/dmbot_client.cpp+h | 1,886 |
| G10 | Does the store's callers' logic repeat what the store decides? | store/record_store.cpp, upgrade_files.cpp+h, stored_versions.h, and the call sites of the five methods probe 4 lists | 1,691 plus the call sites |
| G11a | Do the app shell, the state and main each hold a copy of window or job state? | ui/app_state.cpp, app_shell.cpp+h, main.cpp, job_base.h, dm_jobs.cpp+h, generation.h | 2,299 |
| G11b | Do the library's dialogs, toolbar and widgets restate rules the model owns? | ui/library_dialogs.cpp, library_toolbar.cpp, library_view.cpp+h, library_parts.h, widgets.h, theme.cpp+h, icons.cpp+h, fonts.h, label_layout.h, win32_dialogs.cpp+h, imgui_files.cpp, resource.h | 1,741 |

Sixteen scouts. The groups cover every file in `src/` at least once; the main session checks that with a two-line script before dispatch (every `src/` path appears in some group's list) and fixes the table if one is missing. Totals: 181 files and 39,724 non-blank lines in `src/` (app 8,396; ui 12,207; core 4,484; parse 3,526; search 3,253; store 2,255; render 2,501; audio 2,278; net 388; cli 383; image 53).

Each scout gets: its group's files, the phase 1 leads for its group, CONTEXT.md, the finding definition and kinds from this plan, and the note that `docs/audit/2026-10-03-derivation-audit.md` already lists line-level copies (a candidate that only repeats one of those is marked "known, line-level" and not written up again). It returns one file, `groups/<id>.md` in the audit folder, with: a candidates section (one block per candidate: the idea as a question, each home as file, symbol and lines, the inputs, a first draft of both tables, the kind it looks like), a "parallel on purpose" section listing the K3 pairs it cleared and why, a "dropped leads" section with one sentence per phase 1 lead it found to be K0, and a "not read" section naming any file it did not finish.

Cap: 100 calls or 20 minutes, whichever first. A scout that hits the cap writes what it has; the main session sends one more scout for the "not read" files of that group only, once, and never a third.

Order: two waves (question 2). Wave A takes G1a, G1b, G2, G3, G4a, G5, G6a and G7a, the groups where the leads are densest, beside the two phase 3 scouts, ten agents in all. Wave B takes the other eight groups and carries wave A's candidate list, so a wave B scout that meets a candidate already written marks it "also seen in G<id>" and adds only what is new. This is the lesson from the earlier fan-outs (feed later rounds the earlier rounds' notes), traded against one wave's speed.

### Phase 3: the concept inventory

Phase 2 finds pairs inside groups nobody disputes. Phase 3 finds the pairs nobody thought to group, by starting from the idea rather than the code. This is concept location by hand: name a concept, then find every piece of code that implements it, through identifiers and strings ([Dit, Revelle, Gethers and Poshyvanyk, "Feature location in source code: a taxonomy and survey", 2013](https://www.cs.wm.edu/~denys/pubs/JSME-FL-SurveyCRCV1.pdf); the identifier-and-text approach goes back to [Marcus et al., 2004](https://www.cs.kent.edu/~jmaletic/cs63902/Papers/Marcus04.pdf)). That survey's one practical lesson for us is that combining approaches beats any one of them, which is why phases 1, 2 and 3 all run instead of picking one.

Two Sonnet scouts, in wave A beside the phase 2 scouts.

Scout C1 starts from CONTEXT.md. It takes each term in the Language section (about 45 entries across Library, Paths, Squeezes and Preview; the count is mine from reading the file) and for each one greps `src/` for its identifiers, its "Avoid" words and its user-visible strings, then records the one owner CONTEXT.md names (where it names one) and every other type, enum, status word set or helper that defines the same idea. A term with two homes is a candidate. A term with no owner named and more than one implementation is a candidate too.

Scout C2 starts from the code. It takes the 66 enums and the 194 structs with three or more fields from phase 1 and sorts each into a concept: "a chart's identity", "a record's status", "a score breakdown", "a job's progress", "a window's state", and so on. Two types under one concept in different modules is a candidate unless one is built from the other through a named converter (the scout names the converter, such as `to_library_entry` in `src/app/analysis.cpp`, which has a scan row). Its output is also the first draft of the glossary phase 7 keeps: concept, owner, other homes.

Output: `inventory.md` in the audit folder, one table (concept, owner, other homes, candidate yes or no, note), plus the candidate blocks in the same shape as phase 2's. Cap: 100 calls or 20 minutes each; a scout that runs out lists the terms it did not reach, and the main session sends one more scout for those only, once.

### Phase 4: triage and truth tables

The main session merges the candidates from `groups/*.md` and `inventory.md` into one numbered list, `candidates.md`, dropping exact repeats and keeping every "also seen" note. My estimate of the size is 60 to 120 candidates: the probes alone gave 48 clues (17 strings, 9 names, 17 struct pairs, 5 store methods), the scouts will add more and drop some, and the line-level audit's 302 first-round candidates were for a finer grain. That estimate is mine, not measured.

Then Sonnet verifiers, each with about 10 candidates (so 6 to 12 verifiers in one wave), do what the earlier audit's verifiers did (`docs/audit/2026-10-03-derivation-audit.md`, step 3): a candidate survives only if the verifier finds every home it relies on and writes both tables with the edges in; for a disagreement, the verifier also gives the input where they differ. A verifier never gets a candidate from a group it scouted. It assigns the kind (K1, K2, K3, K0), proposes the owner, and classes the fix (code-only, or needs the user: any change to a displayed text, a stored record, a score, or a choice between two behaviours). Where a claim needs a run to settle (for example, which status a Stale chart gets on the DM page), the verifier says "by reading" and names the run that would prove it; nobody builds in this phase.

Output: `findings.md` in the audit folder (shape below). Cap: 100 calls or 20 minutes per verifier; a candidate left unverified at the cap goes into `findings.md` as "unverified, looked in ..." and gets no second verifier. There are no completeness rounds. The earlier audit's completeness passes never met their own stop rule and the user stopped them after six rounds; this audit stops after one verification pass and relies on phase 7 for what it missed.

### Phase 5: user decisions

The main session writes `questions.md` in the audit folder: one question per K2 finding and per K1 finding whose fix touches a displayed text, a stored record or a score. Each question quotes the predicate from each home as the code has it, gives one example input from each end, names the options without leaning, and ends with a recommendation and the strongest case against it. The user's answers are recorded as D-records in `docs/audit/2026-10-03-fix-decisions.md`, where the numbering continues (D107 and D112 are there, at lines 498 and 523), and the precheck reads that file for decisions (`tools/derive_once_precheck.ps1`, check 3). The K1 code-only findings are listed in the same file with the owner the verifier recommends, so the user can veto any, but they need no answer to proceed (agent rule 3: a code-only change's design is picked and recommended, and the build waits for the yes).

The user says yes to the fix wave as a whole, or picks which findings go in. Nothing in phase 6 starts before that.

### Phase 6: fixes

Executor tasks on Opus, one worktree each, grouped so no two writers own one file. The main session builds the groups from `findings.md` by owned file: every finding whose homes sit in one group of files goes to that group's executor. My estimate is 6 to 12 executors over two or three waves, depending on the user's answers. Each executor's brief names its findings by id, the owner each one keeps, the tests it may run (`-tc=` or `-sf=` filters for the touched files, the single-owner scan with `-tc="single-owner*"` when it adds a row, named `hydra_uitest` scripts for a UI change), and the D-record behind any displayed or stored change. One cold build per executor at most.

Each merge gets one fresh Sonnet reviewer under `docs/agents/derive-once-review.md` (D107: it fixes its own findings and signs off; one review per change). Each module-level merge ends in one of two things, as the review page already asks of a kind A fix: a scan row in the owner's area file under `tests/single_owner/rules/` that would flag the removed home if it came back (for `ms_key` versus `optional_key`, a row on an optional-to-`SortKey` conversion outside the owner), or the one sentence in the commit body saying why no text search can catch it (a second row type with its own field names is the usual case; the recurrence gate in phase 7 is what catches those).

A finding the user wants fixed but whose fix needs a new number or a new sentence stops at the executor and comes back as a question; it does not get invented.

### Phase 7: the recurrence gate

The audit is one pass. What stops the next module from inventing its own status words is a step in the review and a probe in the precheck. The exact wording is in its own section below. It is one Opus executor for the precheck change (with its self-test) plus the CONTEXT.md glossary entries from phase 3, and one doc edit to `docs/agents/derive-once-review.md`. It can start as soon as phase 3 is done; it does not wait for phase 6.

## Where the outputs live

Everything goes in `docs/audit/2026-10-11-concept-audit/`: `leads.md` and `leads.ps1` (phase 1), `groups/G1a.md` to `groups/G11b.md` (phase 2), `inventory.md` (phase 3), `candidates.md` and `findings.md` (phase 4), `questions.md` (phase 5). Decisions go in `docs/audit/2026-10-03-fix-decisions.md`. Scout reports are internal documents and are not gated by the doc-review hook; `questions.md` is read by the user, and it is a committed audit document, so it is not gated either (CLAUDE.md lists the four public pages, Artifacts, plans shown with ExitPlanMode and `gh` text).

`findings.md` has one block per finding, in this shape, so phase 5, phase 6 and the user can read it without the scout reports:

```
### C7. Which settings does a report page depend on?
Kind: K1 (same rule, two hand-kept lists). Fix: code-only.
Homes: report::settings_change_touches, src/app/report.cpp 431-437 (cap_query, lens);
       dm_report::settings_change_touches, src/app/dm_report.cpp 237-241 (chartmode_key, lens).
Caller: ui/app_state.cpp 866-870.
Tables: [inputs: which setting changed -> page rebuilds?] both homes, edges: a setting
        neither lists (hit window), a setting only one lists (SP cap).
Disagree on: none today; a generator that starts reading a new setting splits them.
Owner proposed: each generator states the settings it reads once; one comparison reads both.
Found by: probe 2 (function names); G1a; verified by V3 by reading.
Scan row or reason: row on a hand-written settings comparison outside the owner, rules/app.json.
```

Kinds, fix classes and owners use the words defined in this plan and nowhere else, so the file has one vocabulary.

## How findings become decisions

A fix is code-only when every home is internal: no displayed word changes, no stored field changes, no score moves, and the merged rule gives the same answer the kept home gave. The verifier picks the owner and says why, the plan recommends it, and the user's yes to the wave covers it.

A fix needs the user first when any of these is true: a displayed text changes (the DM page's "Not analyzed" is one), a stored record changes (merging two chart-metadata shapes that reach the database would), a score or a count moves, or the two homes disagree and one behaviour has to win. For those, `questions.md` quotes the real predicates. For "Not analyzed" the question would read, in short: the library says Not analyzed only for `RecordStatus::NotAnalyzed` and says Stale for `RecordStatus::Stale` (`status_label`, `src/ui/library_model.cpp` lines 67 to 74); the DM page says "not analyzed" for any listed chart with no row in `list_records` for this mode at SP cap 4, which the store's listing rule makes include Stale rows (`collect_dm_rows`, `src/app/dm_report.cpp` lines 121 to 127); example from each end: a chart never analyzed (both say Not analyzed) and a chart analyzed under an older results stamp (library says Stale, DM says Not analyzed); options: the DM page gains a Stale status, or its label changes to a word that does not clash, or the library's word is widened; recommendation and the case against it stated. Whether the DM page's statuses are a K2 finding or an accepted specialisation is the user's call, and the question is written so either answer is a one-line decision.

## How fixes are dispatched

The main session groups findings by the files they touch and gives each group to one Opus executor in its own worktree, so two writers never own one file. A finding whose homes span two groups' files goes to the group that owns the kept home, and the other group's executor is told not to touch that symbol. Each executor commits one finding per commit, with the grep it ran and its one-line result in the body, and ends with the precheck run on its range. Each merge gets one Sonnet reviewer (D107). The reviewer's Notes carry any question it could not settle; the main session passes those to the user and does not hold the merge for them.

Every module-level merge ends with a scan row or a one-sentence reason, as the review page already requires for kind A. The reason is expected more often here than in line-level fixes, because a second row type does not share a spelling with the first; that is why phase 7 adds a structural probe to the precheck.

## The recurrence gate, in concrete terms

Three changes. The first two are to files in this repo and go through a normal worktree and a reviewer. The third is a convention, not a hook. No change to the hooks under `~\.claude` is needed: the merge gate reads the reviewer's `Verdict:` line and nothing else, and the precheck is run by the orchestrator from `tools/`. If the user later wants the new probe to block a merge on its own, that is a hook change and goes through a worktree of `~\.claude` as the hook overhaul plan describes; this plan does not propose it.

**1. `docs/agents/derive-once-review.md` gains a kind and a step.** Under "The five recurring kinds", add a sixth: "**F.** A second home for an idea that already has one: a new type, enum, set of status words or helper family that answers a question another module's type, enum, words or helpers already answer, under its own names." Under "How to review", add step 8: "**Check new homes.** For every type, enum, status word set or free helper the change adds, name the existing concept it extends: the CONTEXT.md term, or the owner symbol. The precheck's F lines list the candidates. If the concept has no entry in CONTEXT.md, the change adds one (the term, its owner, and what it deliberately leaves out). A new home for a concept that already has one is a kind F finding, fixed like kind A: the new code calls the owner, or the change says in its commit body why it cannot." The page's "The five recurring kinds" heading becomes "The six recurring kinds". The page is an internal doc, so the doc-review hook does not gate it; the sentence that says the precheck finds "kinds B to E" changes to "kinds B to F".

**2. `tools/derive_once_precheck.ps1` gains check 5 (kind F).** On the added lines of the range it prints: a struct or enum whose name already exists elsewhere in `src/`; a struct whose field names overlap a struct in another file by four or more (the phase 1 threshold, question 8); a free function whose name is already defined in another `src/` file; and a string literal of 12 or more characters that already exists in another file. Each line names the other home. The self-test `tools/test_derive_once_precheck.ps1` gets one case per probe and a `-Disable 5` case. The header's check list and the "What it cannot see" paragraph are updated. This is phase 1's script, kept, but as part of the one precheck rather than a second scanner; `leads.ps1` in the audit folder is then only a record of the run.

**3. CONTEXT.md is the glossary.** Phase 3's inventory becomes entries where a term has none, in the existing form (term, one owner, "Avoid" words). The review step above points at it. This borrows the domain-driven design rule that one term has one meaning inside a context and that synonyms are a defect ([Verraes on ubiquitous language](https://verraes.net/2011/05/ubiquitous-language/)), and the fitness-function idea that a design rule is a check that fails, not a page nobody reads ([ArchUnit user guide](https://www.archunit.org/userguide/html/000_Index.html); [Protean's model-drift checks](https://docs.proteanhq.com/guides/architecture-fitness-functions/) are the nearest tool I found that checks a domain model rather than package layering). None of those tools reads C++, and none checks vocabulary; the glossary-to-code check is ours, done by the reviewer with the precheck's list in hand.

Scan rows the gate adds on its own: none beyond the ones phase 6's fixes add. A row needs a spelling to match, and a second home with its own names has none; that is what check 5 and step 8 are for.

## Risks and what is left out

The biggest risk is false confidence in the scouts' verdicts, which is why no scout grades its own candidates and every finding carries two tables. The second is volume: if candidates run well past 120, the verification wave grows, and the main session caps it by sending the lowest-value candidates (K0-looking, internal, agreeing today) to `findings.md` as "not verified, parked" rather than adding a second wave. The third is the fix wave changing behaviour by accident; every merge has a reviewer, every displayed or stored change has a D-record, and the main session runs the full suite once per merge. The fourth is cost: 18 scouts, 6 to 12 verifiers, 6 to 12 executors and as many reviewers, which I estimate at 4,000 to 5,000 tool calls in all, assuming about 80 calls per agent; that is an estimate from the caps, not a measurement.

Left out on purpose: `tools/` and `tests/` (the user asked for `src/`; the test suite had its own audit on 2026-10-10 and the single-owner redesign fixed the known test copies); completeness rounds (see phase 4); any clone tool or semantic-diff tool (the single-owner redesign plan's prior-art section already found no C++ structural tool that runs on this Windows and MSVC setup, and the type-4 recall numbers above say a model reading alone is not one either); a hook change (see the gate section); and fixes to the line-level copies the 2026-10-03 audit already lists, which stay on that audit's fix list.

## User decisions needed before this starts

1. **Scope.** `src/` only, all eleven folders, no pilot. Recommendation: yes, as asked. `tools/` and `tests/` wait.

2. **Scout waves.** Sixteen group scouts and two inventory scouts as two waves (ten, then eight fed the first wave's candidates), or one wave of eighteen. Recommendation: two waves, for the dedupe; the cost is about twenty minutes of wall time.

3. **Where the lead script lives.** Saved beside the audit as a record (`leads.ps1`), with its probes folded into the precheck as check 5 in phase 7; or added under `tools/` now as its own script. Recommendation: the record plus check 5, so there is one precheck and not a second scanner.

4. **Fix the K1 findings that agree today, or only record them.** The line-level audit's "duplicates that agree today" were its largest group (166 of 352), and each is a future split. Recommendation: fix every K1 in `src/`, in waves the user can stop between.

5. **Where decisions are written.** Continue the D-numbers in `docs/audit/2026-10-03-fix-decisions.md`, or start a file in the audit folder. Recommendation: continue, because the precheck reads `docs/audit/*decisions*.md` and the file is where D107 and D112 already are.

6. **Kind F.** Add a sixth kind letter to the review page and the precheck, or fold module-level homes into kind A. Recommendation: kind F, so a reviewer's finding says which of the two it is and the precheck's line can be told from a row hit.

7. **K2 words up front.** Decide the user-visible status words (the library's, the DM page's, the fill page's) as one batch of questions before the fix wave, so the fix wave is code-only. Recommendation: yes; the questions are written from the verified predicates in phase 5.

8. **The probe thresholds.** A string of 12 or more characters, a struct with 3 or more fields, 4 or more shared field names. These shaped this session's lead counts and would shape check 5. Recommendation: keep them, recorded as a D-record, with the counts above as the reason; the first run of check 5 on a real range may tune them, as a reviewed change.

## What done looks like

`docs/audit/2026-10-11-concept-audit/` holds the leads, sixteen group reports, the inventory, one findings file in the shape above and the questions file. Every K2 word has a D-record. Every K1 the user approved is merged, each with its reviewer's CLEAN and a scan row or a stated reason. `docs/agents/derive-once-review.md` names kind F and step 8, the precheck prints F lines and its self-test covers them, and CONTEXT.md has an entry for every concept the inventory found with more than one home. A change that adds a second status vocabulary after that is a precheck line and a review finding, not a surprise.
