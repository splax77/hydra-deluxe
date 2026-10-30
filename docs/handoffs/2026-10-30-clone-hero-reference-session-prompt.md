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

## Rules that hold throughout

- Don't run a full library test (rule 8). Use `testdata/input`.
- Don't start Clone Hero or attach to it unless the plan says so and I've said yes.
- Every number says where it came from.
- Write plain English, following `C:\Users\Patrick\.claude\CLAUDE.md`.

## Where it lives, and what done looks like

The pages go in `docs/clone-hero/`, with an index at `docs/clone-hero/README.md`, one page per area above, and `code/` for the annotated functions. They are internal docs, so no gate requires a review. They still get the one fresh review described above, because the whole point is that they can be trusted. Done means:

- every area above has a page;
- every rule on it has a label and evidence;
- the questions for me and the list of live checks are written out;
- the old documents point to the reference;
- one fresh reviewer has signed it off.
