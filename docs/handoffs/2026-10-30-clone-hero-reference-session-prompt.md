# Prompt for a new session: the complete Clone Hero reference

Paste everything below the line into a new session in `C:\Users\Patrick\Downloads\Hydra\hydra-test`.

---

Write one complete reference for how Clone Hero handles everything Hydra models: what the game does, where in its code it does it, and how sure we are. Today that knowledge is spread over about 53 past sessions and a dozen files. Each one answers one narrow question, and nothing gives the whole picture. That has already cost us. On 2026-10-30 one session started a bot run to test Star Power. The fact that the bot never uses Star Power was written in one file, the Star Power memory fields were in another, and nothing put the two side by side.

This is a plan-first task. Write the plan, show it to me, and wait for my yes before any scout or writer starts (CLAUDE.md rule 3). Planning agents run on Fable, read-only scouts on Sonnet, and the one reviewer on Sonnet (rule 2).

## What the reference covers

Cover Pro Drums on Clone Hero v1.1.0.6142, the build the `ch` server runs (`C:\Clone Hero\1.1 june\Clone Hero`). Also cover v1.0.0.4045 wherever Hydra supports its rule (the 1.0 fill rule, `legacy_fills`). Follow the game's own order:

1. Finding and reading a chart: `.mid`, `.chart`, `.sng` and `.srb`, track names, `song.ini`, sections, solos.
2. Building the note list: cymbals and toms, dynamics and the tags that enable them, disco flip, 2x kick, Note Shuffle, and what the plain Drums mode drops.
3. Timing: chart Offset against the `song.ini` delay, the song clock, the hit window and its cap, calibration, playback speed.
4. Fills: written fills, generated fills under 1.0 and 1.1, fill deadlines, when a fill shows up, and whether written fills stop generated ones.
5. Star Power: phrases, the meter and the full-bar size, activation on drums, drain, where Star Power ends, phrases collected during Star Power, squeezes in and out, and DragonDelgar's 3 ms leeway.
6. Scoring: note values, multiplier and combo, accents and ghosts, the solo bonus, star cutoffs, and every field of `scorestats.json`.
7. The game internals our tools touch: `GameManager`, the drum engine and its fields, the per-note hooks, the Star Power fields, the song-select menu, and what the bot does and doesn't do.

## "Fully documented code"

For every rule, name the game function or functions that own it. Give each one's class and method name from the Il2Cpp dump, its RVA (the address relative to the start of `GameAssembly.dll`), and the instructions or decompiled lines that decide the rule. Explain each branch in plain words. Keep the annotated code as files under `docs/clone-hero/code/`, the way `docs/audit/note-shuffle/step0/` already keeps its `.asm` files. Where the game's code was never found for a rule, say so. Never write a function from memory.

The tools for this are already on the machine. Ghidra 12.1.3 is at `C:\Users\Patrick\Downloads\ghidra_12.1.3_PUBLIC_20260817\ghidra_12.1.3_PUBLIC`. The `rea` MCP server is set up with it, and the `reverse-engineer-anything` skill explains how to use it. The dumps for both builds are in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\{4045,6142}`, and `tools/ch_mcp/README.md` says how they were made. I approved reverse engineering the game for this project on 2026-10-28 (memory `ch-mcp-feasibility-2026-10`).

## Every rule says how we know it

Give each rule exactly one of these four labels, plus its evidence:

- **Proven in the game's code:** the function and address, and the annotated file.
- **Proven in a live run:** the run, its date and its result file.
- **Hydra assumes it, never checked:** the Hydra code that holds the assumption.
- **Unknown.**

Each rule also names the place in Hydra's code that owns it, so the reference maps the game's rules onto Hydra's. Where Hydra differs from the game on purpose, cite the ruling (ADR 0023 lists them, and the D-records in `docs/audit/2026-10-03-fix-decisions.md`). Where Hydra and the game differ and no ruling covers it, that is a question for me, with both sides shown. Never settle it quietly (memory `never-keep-a-split-quietly`).

## The record to mine

Check every claim against the code, the dumps and the result files before you use it. A past agent's summary is a pointer, not proof (agent rule 9). The sources are:

- The session cards, and the `condensed\` files beside them, in `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\session-cards\`. Grep `index.md` for `clone-hero`, `hit-window`, `squeeze`, `early-fill`, `ch-probe`, `note-shuffle` and similar tags.
- The memory notes whose names start with `ch-`, plus `sp-squeeze-mechanics`, `squeeze-transfer-scale`, `fill-spawn-comparison`, `early-fill-orange-85-d250`, `sp-end-is-one-point-leeway-is-delgars`, `srb-format-support` and `stitched-chart-auto-fills`.
- In `docs/`: `docs/audit/ch-evidence.md`, `docs/adr/0023-chart-rules-follow-clone-hero-code.md`, ADR 0028, `docs/srb-format.md`, `docs/audit/note-shuffle/`, `docs/research/2026-10-28-clone-hero-mcp-feasibility.md`, `docs/agents/clone-hero-mcp.md`, and the `ch_probe` and `ch_mcp` plans and handoffs.
- In `tools/`: `tools/ch_probe/` (`constants.py`, `engine.py`, `experiments/`, and `experiments/results/overlay-spike/il2cpp-fields.md`), and `tools/ch_mcp/`.
- The public Hydra (DragonDelgar's, last commit 3b49012c), for the rules Hydra Deluxe took from it.

## How to build it

Use a few read-only scouts, three or four per wave (memory `lean-agent-fanout`). Each scout takes one slice of the record and returns every claim it finds, with its evidence and the label it earns. Then reverse-engineering scouts fill the "unknown" and "never checked" rows they can reach in the dumps. One writer assembles the pages. Afterwards the old documents point to the reference instead of keeping their own copies: `ch-evidence.md`, ADR 0023's "not verified" list, and the memory notes. One fresh reviewer checks the result against its sources and signs it off (D107).

The rows that only a live run can settle become a list of checks for the `ch` server. A plan is being written now (branch `claude/ch-sp-plan`, `docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md`) to let that server read Star Power and start a song for the auto-player without me. Don't duplicate it. Feed it the list. The first check waiting on it is whether Clone Hero offers a fill inside the drum solo of Senri Kawaguchi's "Ladies Talk" (measures 76 to 108). The chart's written fills skip that solo, and Hydra's rule says a chart with any written fills gets no generated ones.

## This is the first knowledge base, so build it as the pattern

Today, project knowledge is filed by when it was learned rather than by what it is about. Session cards are history, memory notes hold one fact each, and the decision log holds rulings. Finding out how one topic works means searching all of them and putting the picture together, and most sessions don't do that. The fix is topic pages in the repo: each fact has one home, and everything else points to it. `docs/clone-hero/` is the first one. Later ones (Hydra's engine, storage, the GUI, the hooks) will copy what you set up here, so set up all five of these and write them into the plan:

1. **The pages are the home.** Plain Markdown in the repo, organized by topic, with an index. No vector database or search index: git already gives history, review and grep, and an index would hide the text and go stale without anyone noticing.
2. **One home per fact.** Once a fact is on a page, every other copy is replaced by a one-line pointer to it. That covers `ch-evidence.md`, ADR 0023's "not verified" list, handoffs, and the Clone Hero memory notes (the `ch-` notes and the others listed above). Memory keeps only what memory is for: my preferences and rulings about how to work. Moving those facts out also shortens `MEMORY.md`, which loads into every prompt. Show me the list of memory notes you plan to empty or delete before you change any, since memory is mine.
3. **Easy to find.** Add one line to `CLAUDE.md` under "Agent skills" that names `docs/clone-hero/README.md` and says when to read it: before any Clone Hero question, and before any plan, test or probe that touches the game. Every session reads that line, so no one has to know the right grep.
4. **Written back when something is learned.** Add a rule, in `CLAUDE.md` and in `docs/agents/brief-preamble.md`: a session or agent that learns a Clone Hero fact updates its page in the same commit as the work that found it, not in a memory note. Also have the session-card writer flag a missed write-back. When a finished session learned a fact its page lacks, the card says so, and the next session fixes the page. The writer's instructions are in two places. The brief is `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\session-cards\writer-brief.md`, which no repo tracks. The agent file is `C:\Users\Patrick\.claude\agents\card-writer.md`, which the hooks repo tracks, so a change to it goes through a hooks worktree and its own review, as CLAUDE.md says.
5. **Checked against the code.** `tests/test_docs_match_code.cpp` already checks that every backticked code name in a listed doc still exists, and that a number marked `<!-- default: Name -->` equals the code's value. Add `docs/clone-hero/` to the docs it reads, so every Hydra name and constant on the pages is checked at every merge. Game facts (an RVA, a field offset, a class name from the dump) have no Hydra constant to check against. Give each one a single home (today the RVAs sit in `tools/ch_probe/constants.py`, and the dumps hold the rest). In the plan, say how a page's copy is checked against that home. If you see no clean way, ask me. Don't leave it unchecked without saying so.

Steps 3, 4 and 5 change `CLAUDE.md`, the preamble, a test and a hook brief. They go into the plan as their own tasks, sized for one executor each, and wait for my yes like the rest.

## Rules that hold throughout

- Don't run a full library test (rule 8). Use `testdata/input`.
- Don't start Clone Hero or attach to it unless the plan says so and I've said yes.
- Every number says where it came from.
- Write plain English, following `C:\Users\Patrick\.claude\CLAUDE.md`.

## Where it lives, and what done looks like

The pages go in `docs/clone-hero/`, with an index at `docs/clone-hero/README.md`, one page per area above, and `code/` for the annotated functions. The pages are internal docs, so no gate requires a review of them. They still get the one fresh review described above, because the whole point is that they can be trusted. The test change is code, so it goes through the derive-once review like any other code. Done means:

- every area above has a page;
- every rule on it has a label and evidence;
- the questions for me and the list of live checks are written out;
- the old documents and the Clone Hero memory notes point to the reference instead of repeating it;
- `CLAUDE.md` names the reference and the write-back rule;
- the docs-vs-code test reads the pages, and every game fact on them has a single home and a stated check;
- the card writer flags missed write-backs;
- one fresh reviewer has signed it off.
