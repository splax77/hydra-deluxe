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

A block's `Evidence:` and `Found:` lines cite only permanent records. Those are files in the repo (a listing under `code/`, a file under `docs/audit/` or `tools/`, Hydra's code, the decision log, an ADR), session cards and condensed transcripts, and the game's own files and dumps. They never cite an agent's working notes, such as a claim table an agent kept while it worked, or any file in a session's temporary folder. Those are gone once the session ends, so a reader cannot follow the pointer.

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
| 07 internals | 55 | 25 | 11 | 0 | 19 |
| All pages | 353 | 182 | 34 | 34 | 103 |

Eleven of page 4's blocks describe build 1.0. The questions page holds 24 open questions, and the live-checks page holds 72 checks covering 82 rows.

## Names in obfuscated code

The game scrambles most of its class, field and method names. Where a page or listing note names such a member, the name is our own, chosen from what the code does; `code/names-6142.tsv` and `code/names-4045.tsv` map each one to the dump's name, so you can find it in a listing.

## Row ids

A block's id is its page's prefix, a dash and two digits, such as `TM-04`. The seven prefixes are CR, NL, TM, FL, SP, SC and IN, in page order. A question's id is `Q-` and two digits.

An id is never reused. A block that turns out wrong is corrected or relabelled under its own id, and a new rule takes the next free number on its page. A question that gets settled loses its heading but keeps its id retired, so an old card that names it still means one thing.

## The write-back rule

A session or agent that learns a Clone Hero fact (a game address, a field, a rule or a live result) updates its page here in the same commit as the work that found it, never in a memory note. An agent whose brief does not own that page puts the fact in its report, and the main session adds it. When a session ends with a fact that never reached its page, its session card says so with a `write-back:` line in its open threads naming the page, and the card carries the `clone-hero` tag. The rule is stated in `CLAUDE.md` and in `docs/agents/brief-preamble.md`; each page's "Last write-back" line gives the date it last took one.

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
