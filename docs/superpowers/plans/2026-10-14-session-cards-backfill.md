# Session cards: a card for every past session (plan, 2026-10-14)

**Status:** draft for the user. Nothing is built. Planner a851f4043869cc32a (task SC-PLAN, session 205c56ae) measured everything below on 2026-10-14 against the transcripts on disk; every number says where it came from. The main session added your two later answers (decisions 7 and 8, Task 8) and spot-checked the term count, the D114 log end, the example commits, the `.gitignore` whitelist and the cleanup sweep's folders.

## What this is, in one paragraph

Claude forgets past sessions. Each one is saved as a transcript file, but those files are huge (one gigabyte for this project's main sessions alone) and full of tool output, so no session can read them. This plan turns every past session into a card of about 30 lines: what the session was for, what went wrong, what you ruled in your own words, and how it ended. A script condenses each transcript first, Sonnet agents write the cards from the condensed text, a second script checks every card against your words as extracted from the transcript, and a third builds a one-line-per-session index. Once a session's card passes, its raw transcript is no longer needed: the condensed text (about 10 MB for all sessions) and the extracted user text stay for good as the evidence a card is checked against, and the transcripts are removed at the end (Task 9). The cards quote you, so they stay on this machine next to the memory folder and never enter the repo.

## The settled design (from you, not reopened here)

Three layers. One index file, one line per session, built by a script from the cards. One card per session with fixed headings: Goal, Topics, Roadblocks, Rulings, Outcome, Open threads. The topic layer is the existing memory files, each gaining a "Sessions:" line that points at its cards. Rulings are your exact words, quoted, with the D-number when there is one; a paraphrase is not allowed. Roadblocks follow one shape: what went wrong, why, how it ended (fixed, worked around, dead end). A replaced ruling is marked "replaced by Dxxx", never deleted. Tags come from a fixed list. Card writers are Sonnet agents that read only your messages and Claude's replies, never tool output. Every session gets a card, including the ones before D1 (2026-10-03); their rulings are quoted with no number, and nothing old gets a new number. Rulings made on or after 2026-10-03 that never got a D-number do get one, assigned in one step after the cards exist (decision 7 below), and written into the repo's decision log by a normal commit on `main` with the usual review. Scope is this project only: the main folder and its 28 worktree folders. The other projects in `C:\Users\Patrick\.claude\projects\` (terraria-mods, horse, Hydra-v1-3-1, words, Smugshot, the scratch workspaces) are out.

## What I measured

The transcripts live under `C:\Users\Patrick\.claude\projects\`. Each project folder is named after its working directory. This project has one main folder, `C--Users-Patrick-Downloads-Hydra-hydra-test`, and 28 more folders for sessions that ran inside a worktree (`...--claude-worktrees-<name>`). A directory listing on 2026-10-14 gave these counts. The main folder holds 292 top-level transcripts (959 MB) plus 2,544 transcripts under `subagents` folders (1.9 GB more, 2.9 GB in all). Of the 28 worktree folders, 23 hold top-level transcripts (25 in all) and 45 subagent transcripts; the rest hold no top-level transcript, so my listing skipped them. That is 317 top-level sessions and about 2,590 agent transcripts, close to the main session's count of 2,903 files in all.

The main folder's top-level files run from 2 KB to 29 MB, median 2.0 MB, with 55 files over 5 MB and 2 over 20 MB (file sizes, same listing). Sessions start on 2026-08-09, not 2026-08-27 as the file dates suggest: the first message timestamp inside the oldest file is 2026-08-09T22:30Z, and the file date is only its last write. By month there are 82 sessions in August, 78 in September and 152 in October (first-message timestamps from my pre-pass output). 160 sessions started before 2026-10-03, the day of D1, and 153 on or after it.

I wrote a prototype pre-pass (`condense.py`, in my scratch folder) and ran it over all 317 top-level transcripts. It read 1,003 MB and wrote 9.71 MB of condensed text in 12 seconds (its own stats output). That is 103 times smaller. Per session the condensed text is 21.5 KB at the median and 59.8 KB at the 90th percentile; the biggest is 239.5 KB ("SRB files local copies", 33 user messages). By size: 56 sessions under 10 KB, 136 between 10 and 30 KB, 95 between 30 and 60 KB, 22 between 60 and 120 KB, and 8 over 120 KB. The 317 sessions hold 1,315 real user messages in all. Most sessions are short conversations: 157 have one to three user messages, 126 have four to ten, 14 have eleven to thirty, and 2 have more. 18 sessions have no user message at all (they were driven by another session or a workflow; more below). 37 sessions contain at least one compaction summary. 316 of 317 carry a title from the desktop app.

The sample I inspected by hand, before the full run, was ten transcripts chosen by size: 69 KB, 159 KB, 1 MB, 2 MB, 4 MB, 8.6 MB, 19.6 MB, 25 MB, 29 MB, and one 757 KB subagent transcript. Their condensed sizes ran from 0.9 KB to 225 KB, 50 to 824 times smaller than the source.

## The transcript format, as found

Each line of a transcript is one JSON object with a `type`. Only `user` and `assistant` lines carry conversation. The rest are bookkeeping: `attachment` (hook and system context Claude saw), `custom-title` (the desktop app's session title; the last one wins), `agent-name`, `last-prompt`, `cost-state` (totals at the end), `file-history-snapshot`, `queue-operation`, `bridge-session`, `system` (hook results and API retries), and a few more. I confirmed this on the ten sample files and the full run produced no parse failure on 317.

A `user` line is one of six things, and the fields below tell them apart. I checked every rule on the sample. The code that applies them is `classify_user` in `tools\session_cards\common.py`; where this section and that function differ, the function wins.

A real typed prompt has `origin.kind == "human"`, no `isMeta`, and no `toolUseResult`. Its `message.content` is either a string or a list of `text` and `image` blocks. This field is present in the oldest session (2026-08-09) and the newest, so one rule covers the whole range. Task 1 checked this group against all 318 transcripts and corrected the sample's reading in four places. A message that starts with a `<system-reminder>` block still holds your words after it in all 48 cases, so the pre-pass cuts out the reminder blocks and keeps the rest, dropping the message only if nothing is left. A message starting `<command-message>` is a skill you ran as a slash command (13 cases); your words are its `<command-args>`, kept and labelled with the command name, and a run with no arguments is dropped. Pasted content stays in the message as you sent it, and an image shows as `[image pasted]`. Task notifications are not in this group at all: all 1,584 carry their own `origin.kind`, `task-notification`, so the origin rule drops them. A message starting `[Request interrupted` is dropped.

A tool result has `toolUseResult` set, and its content holds a `tool_result` block. These are 80 to 95 percent of user lines (sample: 74 of 77, 207 of 216, 245 of 261). The card writers never see them. One kind of tool result is the exception: an `AskUserQuestion` result carries `toolUseResult.answers`, a map from each question to the answer you clicked or typed. Those answers are yours. The pre-pass keeps them, labelled as answers, and the checker accepts quotes from them (decision 3 below). A denied `AskUserQuestion` (its `toolUseResult` is an error string, not a map) is dropped. An answer can be a list (15 cases across all transcripts) and can be typed text rather than an option label (32 cases), so the pre-pass stores the chosen labels and the typed text of each answer separately, and the checker accepts quotes only from the typed text. The `annotations` field holds only Claude's own preview text and is ignored.

Injected context has `isMeta: true`: skill text, the "[Image: ...]" companion of a pasted screenshot, "Stop hook feedback", and messages from a linked session (`origin.kind == "peer"`). Dropped.

A compaction summary has `isCompactSummary: true` and starts "This session is being continued from a previous conversation". Kept, but labelled "COMPACTION-SUMMARY (not the user's words)", so the writer can use it for the Goal and the checker never accepts a quote from it.

Local slash commands (`<command-name>/model</command-name>`, `<local-command-stdout>`) and interruptions have no `origin` at all. Dropped.

A subagent transcript's first user line is the brief from the parent, with `isSidechain: true` and `parentUuid: null`. Not the user's words.

An `assistant` line's `message.content` is a list of `thinking`, `text` and `tool_use` blocks. The pre-pass keeps `text`, drops `thinking`, and turns each `tool_use` into one short line (tool name plus the file path or description), so the writer can see what Claude did without the output. `Edit` and `Write` inputs give the files edited. `Agent` inputs give the subagents launched. Git commit output in tool results (`[branch abc1234] subject`) and seven-plus-digit hex strings in Claude's replies give the candidate commit hashes.

## Decisions this plan makes

1. **Subagent transcripts are skipped.** There are about 2,600 of them (1.9 GB in the main folder alone), they hold no user words, and the parent session already has their brief (the `Agent` call's input) and their final report (its result), which the main session then relays to you in plain English. Folding them in would multiply the condensed text by ten for no ruling. The card header says how many agents the session launched, from the pre-pass count, so a future reader knows the detail exists in the `subagents` folder. The only loss is roadblocks that an agent hit and the parent never mentioned; a future reader who needs those opens the agent file the card points at.

2. **Sessions with no user message still get a card.** The 18 such sessions were driven by another session (their first message is a status note about linked sessions, `promptSource: "sdk"`, no `origin`). Their card has an empty Rulings section and a header note "driven by another session; no user messages". They are small (229 KB condensed together) and their Outcome and Roadblocks are still worth having.

3. **Answers to Claude's questions count as rulings, in two forms.** A typed answer ("i will do it in a separate session. do nothing for now.") is quoted like any message. A clicked option is written as `chose "Only when not exactly 1 (Recommended)"` with the question shortened after it, because the label is Claude's wording and your act was the choice. The checker accepts a quote only if it is an exact substring of a real message or a typed answer, and accepts a `chose` line only if the label is exactly one of that question's answers.

4. **Tags come from two lists in one file.** `CONTEXT.md` has 77 bold terms under Language (a grep for `^**term**:` on 2026-10-14). A script lowercases them and joins words with hyphens (`scanned-chart`, `library-row`). Those terms cover Hydra but not the sessions about hooks, agents and plans, so a short process list is added by hand: `hooks`, `agents`, `review`, `planning`, `memory`, `release`, `installer`, `ci`, `tests`, `performance`, `docs`, `transcripts`, `clone-hero`, `video`, `leaderboards`. Both go into one `tags.txt`, regenerated from `CONTEXT.md` plus the hand list by the script, so a new `CONTEXT.md` term becomes a tag on the next run. A new process tag is one line in the hand list, added by you or by the main session after you say yes. A writer that wants a tag the list lacks writes it in a `proposed-tags:` line in the card's Open threads; the checker ignores that line, the index builder collects them, and the main session puts the collected proposals to you once at the end. The checker rejects any tag outside the list.

5. **Where everything lives.** The cards go next to the memory folder, as you asked: `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\session-cards\`, holding `cards\` (one file per session, named `<start date>-<first 8 of the session id>.md`), `condensed\` (the pre-pass output plus `user_text.json`, kept for good because it replaces the transcripts), `index.md`, `manifest.json`, `tags.txt` and `batches\`. That folder is outside every git repo: `C:\Users\Patrick\.claude\.gitignore` ignores everything under `/*` and whitelists only settings, hooks, skills and agents, so `projects\` never enters the `.claude` repo. The cleanup-sweep hook cannot touch it either: `cleanup_sweep.ps1` removes only leftover git worktrees and scratch entries under `Temp\claude` (its header comment and line 52), and `projects\` is neither. Claude Code's own transcript cleanup (`cleanupPeriodDays`) is not set in `settings.json`. You decided on 2026-10-14 that the transcripts go once the cards exist ("the whole point of this system is to not need 3 GB of transcripts"), so the plan no longer tries to keep them. Task 1 reports what the documentation says about how that setting picks files, and the setting is left alone until the end-of-session card hook (see "Later") exists, so no new session loses its transcript before it has a card.

   The scripts go in a new folder in the `.claude` repo, `C:\Users\Patrick\.claude\tools\session_cards\`, with one whitelist line `!/tools/` added to that repo's `.gitignore`. They contain no quotes, they are about Claude's transcripts rather than Hydra, and the follow-up hook that writes a card at the end of each session will import them from the same repo. Work on them follows `C:\Users\Patrick\.claude\hooks\README.md`: a worktree under `C:\Users\Patrick\.claude\worktrees\<task>`, commits with the three trailers, and a fast-forward merge by the main session. Their tests use small made-up JSONL fixtures checked in beside them, never a real transcript, so no quote of yours enters a repo. The alternative is the Hydra repo's `tools\` folder, which is public on GitHub; it gains nothing and puts Claude-tooling in a player-facing repo.

6. **Which sessions are "still being written".** The pre-pass skips any transcript whose file was last written less than two hours ago, and the manifest records each session's file size and write time at card time. A later run sees a transcript that grew after its card was written and marks the card `stale` in the index; it is redone only when you ask, because a resumed old session is rare and a stale mark is enough.

7. **Rulings before D1 stay unnumbered; rulings after it get numbers.** Sessions from 2026-08-09 to 2026-10-02 (160 of them) quote the ruling with its timestamp and nothing else, as you decided. Rulings from 2026-10-03 on that never got a D-number get one, also your decision. Card writers never assign numbers, because ten writers running at once would hand out the same number twice. Task 8 does it in one pass after every card exists.

8. **Only this project.** The main `hydra-test` folder and its 28 worktree folders are in scope. `Hydra-v1-3-1`, `terraria-mods`, `horse`, `words`, `Smugshot` and the scratch workspaces are out, as you decided. The scripts still take a project folder as an argument, so adding one later is one command.

## The scripts

All seven (the six below plus `prune_list.py`, Task 9) are Python, run as `py -I script.py ...` with the system Python 3.14. Each reads transcripts as data and never executes anything from them.

`prepass.py` walks the project folders, applies the rules in "The transcript format, as found", and writes one condensed file per session plus one `user_text.json` per session holding the real user messages and the typed and clicked answers with their timestamps. It also writes the session's metadata block at the top of the condensed file: session id, title, first and last timestamp, cwd, branches, Claude version, files edited, subagents launched, D-numbers mentioned, and commit hashes. It verifies each candidate hash with `git cat-file -e` against the Hydra repo and the `.claude` repo and keeps only the ones that exist, so writers never see a false hash. It updates `manifest.json` (status `condensed`) and is idempotent: a session already condensed from an unchanged file is skipped. My prototype did everything but the hash check and the answers in 12 seconds for 317 sessions, so the real one is a minute at most.

`batch.py` reads the manifest and groups the sessions that need a card into batches: at most 200 KB of condensed text or 10 sessions per batch, whichever comes first, largest sessions first so the eight over 120 KB each get a batch of their own. It writes `batches\wave-N-batch-M.txt` listing the sessions, and prints the count. On my measured sizes that is about 60 batches: the 56 small sessions fill about 6, the 136 medium ones about 14, the 95 at 30 to 60 KB about 21, the 22 at 60 to 120 KB about 11, and the 8 big ones 8. The real count comes from the script before wave 1 launches.

`check.py` runs over every card. It fails a card when a heading is missing or out of order, the header line does not parse, the session id in the header does not match the file name, a tag is not in `tags.txt`, a quoted ruling is not an exact substring (after collapsing runs of whitespace, nothing else) of a message in that session's `user_text.json`, a `chose` label is not one of that question's recorded answers, a cited D-number has no `**Dnn` entry in `docs\audit\2026-10-03-fix-decisions.md` (D1 to D114 on 2026-10-14; D115 is named in plans but has no entry there yet), a commit hash fails `git cat-file -e` in both repos, a roadblock line lacks its three parts, or the card runs past 45 lines. It prints one line per failure with the card path and the rule, and writes the result into the manifest (`checked` or `failed`, with the messages).

`index.py` reads every checked card's header and Outcome and writes `index.md`: one line per session, oldest first, with date, the first 8 characters of the session id, the title, the tags, and the one-line outcome. It also appends a "stale" or "failed" marker from the manifest and a short "proposed tags" list at the bottom. It is deterministic, so it can be rerun at any time.

`number_rulings.py` is the script behind Task 8, in two modes. `collect` lists every Rulings line without a D-number from the checked cards whose session started on or after 2026-10-03, one line per ruling with its session id, timestamp and the card's Goal, and prints the count; the scout sorts that list. `assign` runs only after your yes: it reads the scout's sorting file, finds the next free number as one more than the highest `D<n>` mentioned anywhere under `docs\` or in the memory folder (D115 is a name with no entry, so today the first new number would be one above it; this plan deliberately names no new number, because the scan would read it), numbers the standing decisions upward in session-time order, appends the entries to `docs\audit\2026-10-03-fix-decisions.md`, writes an existing number into the card for an already-numbered ruling, and rewrites the affected card lines and headers. It refuses to run while any card is `failed` in the manifest, and a rerun on the same sorting file produces the same numbers.

`link_memory.py` adds the "Sessions:" lines. For each live memory file (89 files in the memory folder on 2026-10-14; the `archive` subfolder is left alone) it collects two kinds of cards: sessions whose pre-pass `files_edited` list includes that memory file (deterministic, from the `Edit` and `Write` inputs), and sessions whose card names the memory file in a `memory:` line the writer added. It rewrites one line at the end of the memory file, `Sessions: 2026-10-03-d7f92775, ...`, replacing the previous one if present, so it is idempotent and one process edits the files. No agent edits a memory file; that removes the two-agents-one-file problem.

## Tasks

Every task waits for your yes. Agents follow `docs\agents\brief-preamble.md`: status lines every 10 calls or 5 minutes, 100 calls or 20 minutes to start wrapping up, 150 or 30 as the hard stop.

**Task 1: the scripts.** Executor, Opus. Owns `C:\Users\Patrick\.claude\tools\session_cards\` in a worktree of the `.claude` repo, plus the one `.gitignore` line. Writes the five scripts and their tests on made-up fixtures that cover each user-line kind above (typed prompt, tool result, answers map, meta, compaction, slash command, interruption, subagent brief, human-origin system reminder). May run its own tests and the real `prepass.py` over this project's folders into the `session-cards` folder, since that reads transcripts and writes only the new folder. Stop condition: tests green, `prepass.py` over all top-level transcripts finishes under two minutes with zero failures, `batch.py` prints its count, and `check.py` on an empty cards folder exits clean. It reports the `settings.json` line (`cleanupPeriodDays`) for the main session to add. If the real run shows a user-line kind the rules above do not cover, it adds a rule, a fixture and a sentence to this plan's format section, and says so in its report.

**Task 2: review of the scripts.** Reviewer, Sonnet, fresh, per D107. Reads the scripts, the tests and this plan's format section, runs the tests, fixes what it finds, commits, signs off. Stop condition: sign-off written, or the budget warning, at which point it lists the rest in its Notes. The main session then fast-forwards `.claude` main, as the hooks README describes.

**Task 3: condense and batch.** No agent. The main session runs `prepass.py` and `batch.py`, reads the batch count, and reports it to you with the wave count (batches divided by 10, rounded up). Nothing else happens until the next task.

**Task 4: card writing, in waves.** Card writers, Sonnet, at most 10 per workflow, one batch each, waves one after another because each wave is a workflow of its own. Each writer owns only the card files for the sessions in its batch. Its brief carries the card format, the roadblock shape, the quoting rule, `tags.txt`, the `MEMORY.md` index (so it can fill the `memory:` line), and the path to its batch. For each session it reads the condensed file, writes the card, runs `check.py` on that one card, and fixes the card once if the checker fails; a second failure is left for Task 5. It may run only `check.py`. It touches no transcript, no memory file and no other card. Stop condition: every card in the batch written and checked, or the 100-call warning, at which point it writes the cards it has and reports the sessions it did not reach; the manifest still shows those as `condensed`, so the next run of `batch.py` hands them to a fresh writer. Expected calls per session: two to three (one or two reads for a long condensed file, one write, one check), so a 10-session batch is about 30 calls.

**Task 5: check everything, retry once.** No agent for the check. The main session runs `check.py` over all cards and `index.py`. Failed cards go back through `batch.py` to fresh Sonnet writers, with the checker's messages in the brief. One retry per card, no more. A card that fails twice keeps its `failed` status, the index marks it, and the main session's report to you lists each one with the rule it broke, so you can decide whether to fix it by hand or drop it.

**Task 6: Sessions lines.** No agent. The main session runs `link_memory.py` once after the last wave, then reads two or three memory files to confirm the line is there and the frontmatter is untouched.

**Task 7: spot-check of the cards.** Reviewer, Sonnet, fresh. Reads 12 cards chosen by a fixed rule so the choice is not the reviewer's: the 2 largest, the 2 smallest, 4 picked at even spacing from the sorted list, 2 from before D1 and 2 with compaction summaries. For each it opens the condensed file and judges what the checker cannot: whether the Goal is right, whether each roadblock names a real cause and a real ending, whether a quoted ruling is actually a ruling and not a question, and whether a "replaced by" mark is missing. It fixes those 12 itself, reruns `check.py` on them, signs off, and lists in its Notes any pattern it saw that probably affects other cards. Stop condition: 12 cards done, or the budget warning. If its Notes name a pattern, the main session puts it to you with a count from a grep before any further writer run.

**Task 8: number the rulings made after D1.** Three steps, run after Task 7 so the spot-check's fixes are in.

First, a script collects every unnumbered ruling dated 2026-10-03 or later from the checked cards, with its session id, timestamp and the card's Goal line. It prints the count.

Second, a scout (Sonnet) sorts each candidate into one of three groups. "Already numbered" means an existing entry in `docs\audit\2026-10-03-fix-decisions.md` records the same ruling. The example card's 1e-9 lead and D14 show how this happens: a session can make a ruling that got its number later without saying so. "Standing decision" means a rule about how Hydra, the tools or the process should work from then on. "One-off instruction" means a task step like "save an updated prompt", which binds nothing after the session. Only standing decisions get a number. The scout writes its sorting to a file in the `session-cards` folder, one line per candidate with its reason, and edits nothing.

Third, the main session reports the three counts to you, with five examples from each group. Nothing is written to the log until you say yes. This is the one point where the plan comes back to you, because it adds entries to your decision log.

After your yes, a script gives out numbers. It finds the highest D-number used anywhere in the repo, not only in the log, because D115 is already a name for the one-record-per-chart work without an entry (grep on 2026-10-14). It numbers the standing decisions upward from there, oldest first. Each new entry goes at the end of the log in the existing shape: `**Dnnn (session <first 8 of id>), <date>.**`, then your quoted words and one plain sentence of context from the card. Already-numbered rulings get their existing number written into the card instead. The script then rewrites each affected card's Rulings line and header with the number, and `check.py` and `index.py` rerun. The log edit is a normal commit on main, made with explicit paths, and it goes through the usual fresh review before the move of main.

Stop condition: the scout sorts every candidate or reaches its budget warning. In that case it sorts what it can, and the rest go to a second scout with the first one's file as input. If a second scout also runs out, the main session reports the unsorted count to you rather than launching a third.

**Task 9: remove the transcripts.** Added 2026-10-14 at your request. A seventh script, `prune_list.py`, writes `session-cards\prune-list.txt`. It lists the transcript and its agent folder for each session whose card is `checked`, whose condensed file and user text exist, and whose transcript was not written in the last two hours. It never lists anything in `memory\` or `session-cards\`, anything outside this project's folders, or any session without a checked card, and its tests prove that. It deletes nothing. The main session reads the list, reports the count and size to you, and gives you one command that removes exactly the listed paths. You run it yourself, because Claude does not permanently delete files. Sessions that fail their card twice stay on disk until you decide about them. Removing a transcript means that session can no longer be resumed.

## Cost and time, estimated

The condensed text is 9.71 MB in total (measured). At the usual rule of thumb of four bytes per token that is about 2.5 million input tokens read once across all writers. Each writer also reads its brief, the tag list and the memory index, about 20,000 tokens, which over about 60 writers is another 1.2 million. Output is about 1,500 tokens per card, so 317 cards are about 0.5 million. Call it 4 to 5 million tokens before the cached re-reads that every tool call makes, which are billed at the cached rate. The first wave's real usage replaces this estimate; the main session reports it to you before wave 2.

Wall time: the pre-pass is a minute. Each wave is at most 20 minutes by the agent budget, and about 6 waves make two hours of writing plus the time between waves. The script review, the spot-check, the Task 8 scout and the review of the log commit are one agent each.

## Prior art (web, 2026-10-14)

Nothing I found builds a per-session card with checked quotes, but several tools cover pieces of this and confirm the format reading above.

- [claude-code-transcripts (Simon Willison)](https://simonwillison.net/2025/Dec/25/claude-code-transcripts/) turns a local session into HTML pages with an index that lists the prompts and the commits. It is the closest to the index layer; it keeps everything rather than condensing.
- [cc2md](https://github.com/magarcia/cc2md) converts a session to Markdown and, for a session with subagents, writes an `index.md` for the main conversation with links to the subagent files, which is the same shape as decision 1.
- [cc-transcript](https://pypi.org/project/cc-transcript/12.0.0/) renders one compact line per event and has a `--signal` mode that keeps only the substantive turns, the same idea as the pre-pass.
- [cc-session-tool](https://awesome.ecosyste.ms/projects/github.com%2Fmgacy%2Fcc-session-tool) compresses a raw transcript (it quotes 50,000 to 120,000 tokens each) into a focused JSON for agents to read.
- [extract-transcripts](https://skills.cat/skills/aiskillstore/marketplace/extract-transcripts) keeps an incremental DuckDB index of sessions, which is the manifest idea; its listing carries a "Risky" rating from Snyk.
- [SessionScribe](https://www.claudepluginhub.com/plugins/ormizl-sessionscribe) and [claude-session-summary](https://tomevault.io/tome/cosgroveb/claude-session-summary) write a Markdown summary from a Stop hook at the end of each session; that is the shape of the "Later" item, not of this backfill.
- [session-recall](https://awesome.ecosyste.ms/projects/github.com%2Fbuildingopen%2Fsession-recall) parses the same JSONL to recover context lost to compaction.

None of them verifies a quote against the transcript or restricts the summarizer to user and assistant text, so those two parts are ours.

## Example card, from a real session

Built by hand from the condensed text of session `d7f92775-f421-4da9-9f43-5ccf6e0b461d` (2026-10-03, title from the desktop app). Every quote below is a substring of a real message in that transcript, and the three commits exist in the Hydra repo (`git log -1` on 2026-10-14). The D-number field is empty because the session names none; D14's 1e-9 rule looks like it came out of this plan, but the card may not claim that without a source, so it sits in Open threads as a lead.

```
# Missing effective timing display (2026-10-03)
session: d7f92775-f421-4da9-9f43-5ccf6e0b461d | project: hydra-test | commits: 707c285, a4bbad6, dea4228 | D: none | tags: squeeze, path-details, review, planning | agents: 0

## Goal
Find out why a backend row with a x0.99 multiplier showed no "eff." figure, then plan the fix.

## Topics
Effective timing figures; the 1 ms display cutoff; the "which multiplier governs a note" rule; the derive-once audit method.

## Roadblocks
- The figure was hidden by a 1 ms cutoff -> Claude added that rule itself on 2026-08-19 (dea4228) and tightened it on 2026-08-28 (a4bbad6), with no request from the user; the 2026-09-29 session kept it -> fixed in the plan (the cutoff is deleted).
- Claude's first edit patched a flag on top of three copies of the same rule -> it searched for copies of the formula, not of the decision, so the four row branches and the second loop looked like different code -> worked around: the edit was reverted and the plan puts the rule in one function, rate_note().
- The 2026-09-24 derive-once audit had missed both duplicates -> it searched by name and module, let a code comment justify a threshold, and never proved what it had examined -> worked around: a new audit prompt with a coverage ledger and canaries pinned to commit 707c285.

## Rulings
- "keep it in the table row only, but change it to always display. it should be calculate as the exact figure, not based on a rounded 0.99. make sure all of this is only derived once." (05:14 UTC)
- "plan out the real fix before changing anything" (05:18 UTC)
- chose "Only when not exactly 1 (Recommended)" to "When the multiplier is exactly x1.00 ... should rows still show an effective figure?" (05:22 UTC)
- chose "Give them the early one (Recommended)" to "Counted rows sitting at the SP end or just past it ... Should they get the early multiplier?" (05:22 UTC)
- "i will do it in a separate session. do nothing for now." (05:29 UTC, answer to how to execute the plan)
- "save an updated prompt" (05:30 UTC)

## Outcome
No code changed. Two files written and left uncommitted: docs/superpowers/plans/2026-10-03-one-squeeze-rating-rule.md and docs/handoffs/2026-10-03-derive-once-audit-prompt.md.

## Open threads
The plan was to be executed in another session; D14 (a multiplier within 1e-9 of 1 counts as 1) matches the plan's tolerance and is the lead to check. The orange warning can change in one rare case the user has not seen yet.
memory: derivation-audit-2026-09, gui-text-probe-for-stored-numbers

## Sources
transcript: C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\d7f92775-f421-4da9-9f43-5ccf6e0b461d.jsonl
condensed: ...\session-cards\condensed\d7f92775-f421-4da9-9f43-5ccf6e0b461d.condensed.md
```

That is 34 lines. The tags `squeeze` and `path-details` are placeholders until Task 1 generates `tags.txt`; the checker would reject them if they are not in it, which is the point.

## Later (not planned here)

A card at the end of every new session. The pieces are the same scripts: a Stop or SessionEnd hook in the `.claude` repo runs `prepass.py` on the session's own transcript, a Sonnet agent writes the one card, `check.py` checks it, and `index.py` and `link_memory.py` rerun. The open parts are the hook's time budget and whether a session that ends mid-task should wait for its last compaction. That is its own plan after this backfill has run once and the card format has settled.

## Open questions for the user

You already settled two questions: rulings after D1 get numbers (decision 7, Task 8), and other projects are out of scope (decision 8).

1. **The process tag list.** Decision 4 proposes fifteen hand tags (`hooks`, `agents`, `review`, `planning`, `memory`, `release`, `installer`, `ci`, `tests`, `performance`, `docs`, `transcripts`, `clone-hero`, `video`, `leaderboards`) next to the 77 terms from `CONTEXT.md`. My recommendation is to start with these and let the proposed-tags list from the writers extend it once, after the first run. The trade-off: a short list means some cards carry a near-miss tag until then; a long list invented now would mostly be guesses.

2. **Where the scripts live.** Decision 5 puts them in a new `tools\session_cards\` folder in the `.claude` repo, which needs one whitelist line in that repo's `.gitignore`. My recommendation is that folder, for the reasons given there. The alternative is the Hydra repo, where the derive-once and doc-review hooks already cover reviews, but the scripts would then sit in a public repo and have nothing to do with Hydra.

3. **The cutoff for a running session.** Decision 6 skips any transcript written in the last two hours and marks a card stale if its transcript grows later. My recommendation is to keep that and never redo a stale card automatically. The trade-off is that a session you resume days later keeps an old card until you ask for it to be redone; the index shows the stale mark so it is not silent.
