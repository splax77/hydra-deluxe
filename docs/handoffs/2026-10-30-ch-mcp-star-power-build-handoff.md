# The `ch` server's Star Power read and song start: build handoff (2026-10-30, session 9a82ff65)

The plan is written, reviewed twice and signed off clean. Nothing is built. The user has not yet said yes to the build. Show them the plan, ask, and start only on their yes (CLAUDE.md rule 3).

## Why this exists

The user built the `ch` MCP server so the main session could test Clone Hero with no input from them: "the whole point of building the mcp was for you to be able to test without my input", and "why do we need scripts to read what is happening in the game? that was the whole point of creating mcp". Today the server can do neither of the two things a Star Power check needs.

- **It can't read Star Power.** The 2026-10-28 server plan left it out (its question 7). The overlay spike found and proved the fields the same day, from outside the game, in `tools/ch_probe/experiments/sp_watch.py` and `sp_compare.py`. Run 1's comparison had 31 PASS and 3 FAIL, and the three FAILs were a fill the game didn't offer, not a wrong field. But nobody moved that work into the server.
- **It can't start a song the auto-player can play.** A command-line `--song` launch is played by the game's bot, and the bot never uses Star Power. That was found on 2026-10-28 and is written in `docs/research/2026-10-28-clone-hero-mcp-feasibility.md`, Test 1. So today the user has to pick the song in the menu.

On 2026-10-30 this session forgot the bot fact, started a bot run, and the user had to stop it ("the bot mode has no starpower"). The memory note `ch-mcp-server-2026-10` now says so up front.

## Where the plan is

- The plan: `docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md`.
- It lives on branch `claude/ch-sp-plan`, in worktree `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\ch-sp-plan`. The tip is c4855678.
- It is not on `main` yet. Merge the branch into `main` when the build starts; it changes only that one file. Run the three always-run tests first, as CLAUDE.md rule 1 says.

The branch has four commits:
- f1d49a7c: the first plan.
- 0d90eb9c: the first review's fixes. That review found the Online-menu read probably couldn't work, fixed the step order and the LS3 window, made the ready check require the flag clear, and fixed task ownership.
- 6c5fd154: the revision after the user's rulings.
- c4855678: the second review's fixes, with rulings 5 and 6 folded in.

## The user's rulings on the plan, word for word

These are recorded in the plan's "Rulings recorded" section. They have no D-record yet. Write one when the build starts.

1. On "no tool opens any menu": "nobody picked that rule. create a way to navigate menus if it doesnt exist."
2. On "never write game memory, set a field or call a game method": "again, nobody selected this rule. figure out the correct method."
3. On the four new menu numbers (10 s per step, 0.25 s between keys, at most 10 Down presses, a menu read every 0.5 s), all guesses, and the run cap of song length plus 60 s: "Use them (Recommended)".
4. On the first menu key sequence: "Data file, fixed live (Recommended)".
5. On writing game memory and setting fields: "Allow writes too".
6. On whether someone must be present for a live run: "No one needed (Rec.)". The user says in chat when a run may start.

The two rules in 1 and 2 came into the server plan inside a batch approval ("Yes, all seven"). The user never picked either one on its own. Don't treat a safety rule an agent wrote into a plan as the user's choice.

## What the build makes, in plain words

- **Star Power tools.** The server gets the read the spike proved, as a single read (`ch_sp`) and as a watch over a whole run. Activations, phrase rises and "was Star Power ready inside this window" can be read back and compared with a Hydra path. The read stays outside the game, where it was proved.
- **Menu navigation by keys.** `ch_key` sends any key, and `ch_menu` reads which menu is showing. `ch_open_song(folder)` walks a key sequence kept in `tools/ch_mcp/menu_steps.json`, and reads the game after every press. The Online-menu guard asks the game directly whether that menu is showing (`get_activeInHierarchy`). No step opens the Online menu.
- **Finding the song-start method.** The plan lists seven groups of candidate methods from the Il2Cpp dump, with their addresses. All are guesses until live step LS-M runs: it hooks every candidate while `ch_open_song` drives the menus, and names the one that starts the song. Then `ch_start_song` calls that method to start a song in one step, and key navigation stays as the fallback.
- **The rule rewrites.** `docs/agents/clone-hero-mcp.md`, the server's instruction text and `tools/ch_mcp/tests/test_frida_attach.py` change to match rulings 1, 2, 5 and 6.

Tasks T1 to T8 each have one executor. Each one names its owned files, its tests, its blockers and its `earlyStart`. T2 goes first, and T1 can start from T2's executor branch. T3, T4 and T7 run in parallel. T6 comes after T3 and T4, then T5. T8 comes after LS-M passes. Each task gets one fresh reviewer before it merges (D107).

## The live steps

All of them are the main session's, after the merge, and need nobody at the keyboard (ruling 6). Ask the user in chat before each run starts.

- **LS0:** one launch with `--song` and no `--player`, to see whether the game opens the song for the keyboard profile.
- **LS1:** walk the menu sequence one press at a time and correct `menu_steps.json`. It first checks that the Online guard reads false where the Online menu is closed.
- **LS2:** `ch_open_song` end to end.
- **LS-M:** the method study. It passes only if the game loads its gameplay scene after the last menu key.
- **LS3:** the Ladies Talk fill check, below.

## The first real check waiting on this

This is question Q7 of the audit-of-audits list, recorded in D271 item 2. Hydra's rule is that a chart with any written fill gets no generated fills (`Song::check_activations` in `src/core/song.cpp`). Clone Hero's rule has never been checked.

The chart is Senri Kawaguchi's "Ladies Talk": `testdata/input/common/IB24/T6/Senri Kawaguchi - Ladies Talk [Kiyo Sen] (Drumeo Performance)/notes.mid`. Its written fills skip a drum solo from measure 76 to 108, which is 114.6 s to 166.5 s on the song clock. The plan's meter simulation says Star Power becomes ready inside the solo, after the phrase ending at 134.2 s. LS3 plays it with every note hit and asks whether Star Power turns on anywhere inside the solo window, 114778.9 to 166544.0 ms.

The chart is installed in the game as `C:\Clone Hero\songs\Hydra Probe\Ladies Talk fill check`. Its score JSON was made with `build-cpp\Release\hydra_replay.exe score --chart "<that notes.mid>" --out <file>`. Remake it in your own scratch folder, because this session's scratch copy will be gone.

## Notes the reviewers left

- **The disassembly was never read.** The planner couldn't reach the `rea` (Ghidra) tools from its agent, so every method candidate comes from the dump alone. If `rea` works from the main session, a static read in Ghidra could pick the method before LS-M. The plan lists that as optional and out of scope until the user says yes.
- **T7 is the task most likely to overrun.** It holds the method watch, a refactor of `trace.ts`, Python, JSON and tests.
- **T5 has a rename to do.** It should rename `test_build_server_registers_the_eighteen_tools`. The tool count goes from 18 today to 29 after T5, and 30 after T8.
- **Read the spike results from the main checkout.** The overlay-spike result files are untracked, so they exist only in the main checkout (`tools/ch_probe/experiments/results/overlay-spike/`). Their write-up, `docs/research/2026-10-28-overlay-spike-results.md`, is still the empty template.

## Related

- The Clone Hero knowledge-base session prompt: `docs/handoffs/2026-10-30-clone-hero-reference-session-prompt.md`. It feeds its list of live checks to this server.
- The audit work: `docs/handoffs/2026-10-30-audit-of-audits-remaining-handoff.md`.
