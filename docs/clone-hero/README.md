# The Clone Hero reference

This folder is the one home of what the project knows about how Clone Hero works.
It describes build 1.1 (v1.1.0.6142) unless a page says otherwise; the fills page also covers 1.0 (v1.0.0.4045).
Last write-back: 2026-10-31.

This page is the index. It says what each page holds and how the pages are kept honest. It states no rule of the game; the rules live on the area pages.

## When to read it

Read `docs/clone-hero/README.md` before any Clone Hero question, and before any plan, test or probe that touches the game.

## What the reference is

Each area page holds the game's rules for one part of the game, one rule per block. A block has a stable id, so a card, a test or another page can point at it. Every block says how we know the rule, where the game's code for it lives, which part of Hydra owns the same question, and whether a ruling covers any difference between the two.

Where Hydra and the game differ and no ruling covers it, the block points at the questions page. Where only a run of the real game can settle a rule, the block's id is on the live-checks page.

## The four labels

Every block carries exactly one of these four labels, and the evidence the label asks for.

- **Proven in the game's code:** the function and address, and the annotated file.
- **Proven in a live run:** the run, its date and its result file.
- **Hydra assumes it, never checked:** the Hydra code that holds the assumption.
- **Unknown.** The block's evidence line says where we looked. A claim nobody checked is Unknown, never a fact.

## What a block may cite

A block's `Evidence:` and `Found:` lines cite only permanent records. Those are files in the repo (a listing under `code/`, a file under `docs/audit/` or `tools/`, Hydra's code, the decision log, an ADR, a session card under `docs/sessions/`), and the game's own files and dumps. They never cite an agent's working notes, such as a claim table an agent kept while it worked, or any file in a session's temporary folder. Those are gone once the session ends, so a reader cannot follow the pointer.

A session's transcript is not a source either, condensed or not. The user ruled that transcripts "arent kept long term. they shouldnt be expected as a reference". A session card is cited as `docs/sessions/<date>-<id8>.md`. When a fact came from a session but its card does not hold it, the block says the session's own record is not kept, and words the fact no more firmly than that allows.

When the only record of a fact sat in such a place, open the file it pointed at and cite that instead. If that file is not in the repo, or does not show the rule, the block is Unknown and its evidence line says where we looked. When a file a block names was never kept, the block says so ("the listing was not kept") and does not give its old path.

A `Found:` line gives the date and the session that found the rule, such as `2026-10-30, session b02ab3e6 (the reference build)`. It names an agent only by its session, never by a role in a plan's wave.

## The pages

- `01-reading-a-chart.md` [CR]: finding and reading a chart file, the song id, tracks, song.ini keys, sections, solos and the song end.
- `02-note-list.md` [NL]: how the game turns chart notes into the note list it scores.
- `03-timing.md` [TM]: Offset and delay, the song clock, the hit window, calibration and playback speed.
- `04-fills.md` [FL]: written fills, generated fills in 1.1 and 1.0, and their deadlines.
- `05-star-power.md` [SP]: phrases, the meter, activation, drain and the Star Power end.
- `06-scoring.md` [SC]: note values, multiplier, accents and ghosts, solo bonus, stars and the score breakdown.
- `07-internals.md` [IN]: the game's classes and fields, the per-note hooks, the bot and the two builds' naming.
- `questions.md`: every open split between the game and Hydra that has no ruling yet.
- `live-checks.md`: the rows only a live run can settle.
- `code/README.md`: the index of every annotated listing, and the facts tables.

## How many blocks each page holds

These counts were taken from the pages on 2026-10-31, by counting their `###` headings and `Label:` lines.

| Page | Blocks | Proven in the game's code | Proven in a live run | Hydra assumes it, never checked | Unknown |
|---|---|---|---|---|---|
| 01 reading a chart | 53 | 23 | 4 | 9 | 17 |
| 02 note list | 71 | 38 | 4 | 4 | 25 |
| 03 timing | 46 | 25 | 3 | 5 | 13 |
| 04 fills | 45 | 32 | 2 | 4 | 7 |
| 05 Star Power | 46 | 22 | 3 | 9 | 12 |
| 06 scoring | 37 | 17 | 7 | 3 | 10 |
| 07 internals | 67 | 37 | 11 | 0 | 19 |
| All pages | 365 | 194 | 34 | 34 | 103 |

Eleven of page 4's blocks describe build 1.0. The questions page holds 24 open questions, and the live-checks page holds 72 checks covering 82 rows.

## Names in obfuscated code

The game scrambles most of its class, field and method names. Where a page or listing note names such a member, the name is our own, chosen from what the code does; `code/names-6142.tsv` and `code/names-4045.tsv` map each one to the dump's name, so you can find it in a listing.

## Row ids

A block's id is its page's prefix, a dash and two digits, such as `TM-04`. The seven prefixes are CR, NL, TM, FL, SP, SC and IN, in page order. A question's id is `Q-` and two digits.

An id is never reused. A block that turns out wrong is corrected or relabelled under its own id, and a new rule takes the next free number on its page. A question that gets settled loses its heading but keeps its id retired, so an old card that names it still means one thing.

## The write-back rule

A session or agent that learns a Clone Hero fact (a game address, a field, a rule or a live result) updates its page here in the same commit as the work that found it, never in a memory note. An agent whose brief does not own that page puts the fact in its report, and the main session adds it. When a session ends with a fact that never reached its page, its session card (under `docs/sessions/`) says so with a `write-back:` line in its open threads naming the page, and the card carries the `clone-hero` tag. The rule is stated in `CLAUDE.md` and in `docs/agents/brief-preamble.md`; each page's "Last write-back" line gives the date it last took one.

## How the pages are checked

The docs test, tests/test_docs_match_code.cpp, reads every page in this folder at every merge to `main`. It checks four things.

Every backticked Hydra name, such as a function, a constant or a file's underscored name, must exist in Hydra's code or under `tools/`.

Every game fact must have a row in the facts table for its build: `code/facts-6142.tsv` for 1.1 and `code/facts-4045.tsv` for 1.0. A game fact is a backticked address, a class and member name joined by a dot, or a member or offset of a class known only by its index number. The test also checks that every method address and field offset named in `tools/ch_probe/constants.py` has a row in the 1.1 table, so the two homes cannot drift apart.

Every block on an area page must have its seven lines in order, with one of the four labels. Every listing path a block cites must exist, and every listing under `code/6142/` must have its `.md` explanation beside it.

Every `Q-nn` that a block's `Ruling:` line names must be a heading on the questions page, and every heading there must be named by some block.

The facts tables come from the game's dumps, which live outside the repo, so no test in CI can read them. The tool tools/ch_mcp/dump_facts.py writes them with `--tsv`, reading the pages to see which facts they cite. Run with `--check`, it rebuilds a table and reports any difference from the copy on disk. That check is run by hand, on this machine. Each table's header carries the MD5 of the game file it was read from, so a table made from a different copy of the game shows.

## What stays unchecked

Enum values and _TypeInfo addresses are not checked. A page cites them by name and says "not checked" in its evidence line.

The hex values of the game's read-only constants are not checked either. Their one home is `tools/ch_probe/constants.py`, and a page writes such a constant only by its name there.

A real address cited under the wrong rule passes every test, because the test proves only that the fact exists in the game. Catching that is the reviewer's job and the annotated listing's, since the listing shows the instructions that decide the rule.

The 1.0 facts are checked by name only, because the 1.0 dump holds no addresses.

## Page rules

This section is the home of the rules a page is written to. The docs test (tests/test_docs_match_code.cpp, check 5) and tools/ch_mcp/dump_facts.py hold the pages to them. They were set in the plan that built this reference, and were copied here, mostly word for word, when that plan left the repo.

### The page template

Every rule is one block with a stable id, so a card, a test or a pointer can name it. The block is a `###` heading and seven fixed lines, in this order, because a fixed shape is what the test and the card writer can grep for.

```
### SP-07 Star Power ends when the meter reaches zero, checked per frame

Rule: <the rule in plain words, one or two sentences>
Label: <exactly one of: Proven in the game's code | Proven in a live run | Hydra assumes it, never checked | Unknown>
Evidence: <for code: the annotated file under code/ and the lines; for a live run: the run, its date, its result file; for assumed: the Hydra code that holds it; for unknown: where we looked>
Game code (6142): <Class.member or #<TypeDefIndex>.member>, RVA `0x20F6800`, `code/6142/sp_update_20F6800.asm` lines 41 to 58; or "not found: looked in ..."
Hydra: <owner function or constant, file:line, e.g. `activation_initial_end_times` in src/search/graph.h; or "no owner: <where we looked>">
Ruling: <D-number or ADR with the user's quoted words, or "open split: see questions page Q-nn", or "none needed: same rule">
Found: <date>, session <id>
```

The `Found:` line names sessions only, as "What a block may cite" says. A fact put together from several sessions names each of them.

Game names are written as the dump writes them, Class.member, never `Class::member` or `method()`, because the docs test treats a span with `::` or `()` as a Hydra name and would fail it. An obfuscated class is written `#<TypeDefIndex>.member`, or `#<TypeDefIndex>+0x<offset>` when only the offset is known. An RVA is always backticked `0x` plus at least five hex digits. A `.rdata` constant is written by its `constants.py` name, never as hex. A 1.0 fact writes `Game code (4045): Class.Method, code/4045/<file>.cs lines ...` with no RVA. Hydra names are the ones the C++ or Python code holds: write `kLegacyFillsFlag`, not `legacy_fills`, and never a name that lives only in a `.js` agent file, which the docs test does not walk.

Each page opens with three lines: what the page covers, which build it describes, and the date of the last write-back. Then the rule blocks, grouped under `##` headings. Each page ends with "Rows waiting on a live run" (ids only, pointing at the live-checks page) and "Open splits" (ids only, pointing at the questions page).

### Game facts and their forms

Each game fact has one home. Method addresses and field offsets that the tools use live in `tools/ch_probe/constants.py`; everything else lives in the game's dumps, outside the repo. A page's copy of a fact is checked against the facts table for its build, which holds only the facts the pages cite.

What each kind of fact looks like on a page, and how it is checked. A page writes each one in backticks; the examples sit in a code block here only so the docs test does not look them up.

```
GameManager.songTime    a method or field of a readable class, checked by name
#8255.sp_amount         a member of an obfuscated class (TypeDefIndex, then the member), checked by index and name
#8255+0x68              a field known only by its offset, checked by index and offset
0x20F6800               an RVA, checked by address
RVA_CONST_NORMAL_BACK   a .rdata constant, by its constants.py name; the hex is never written on a page
```

The docs test already checks a `.rdata` constant's name against `tools/`, because it has an underscore. Of the `RVA_*` names in `constants.py`, `RVA_CONST_*`, `RVA_FORMULA_*` and `RVA_HITCHECK_THRESHOLD` are data addresses in the DLL's `.rdata` section (read-only constants), not methods, so no dump line names them. Every other `RVA_*` name there is a method, and every method address and `OFF_*` offset there must have a row in the 6142 table.

The facts tables are `code/facts-6142.tsv` and `code/facts-4045.tsv`. One row per fact: build, kind (method, field, class, enum), TypeDefIndex, class name (blank when obfuscated), member, RVA (blank for 4045, whose dump has no addresses), field offset. `dump_facts.py` adds one more column, the span as the page writes it, which is what the test matches a page span by (its `TABLE_COLUMNS` are the columns). The header carries the build's version string and the MD5 of `GameAssembly.dll` computed from the file on disk when the table is written (not copied from a log).

A span is looked up in one build's table. A file under `code/<build>/` takes that build for every span in it. Otherwise a span in a block takes the build its block's `Game code (<build>)` line names. A span outside any block, or in a block with no such line, takes the build of the nearest heading above it that names one, such as page 4's "Build 1.0.0.4045" heading, and 1.1 when no heading does. The test's `scan_page` and the tool's `span_builds` own the exact pick, and they must agree.

What stays unchecked, and why, is in "What stays unchecked" above.
