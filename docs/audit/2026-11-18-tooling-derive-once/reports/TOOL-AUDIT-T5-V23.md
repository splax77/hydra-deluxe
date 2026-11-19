# TOOL-AUDIT-T5-V23 (verifier of F23). All claims by reading; nothing run. SC = C:\Users\Patrick\.claude\tools\session_cards, GT = C:\Users\Patrick\.claude\gates.

Candidates checked: 7 candidate lines plus 5 splits. Not reached: none. The finder's 8 second-hand test rows were not reread by me either.

## Candidates
1. K6 human_prompt_text vs classify_user: CONFIRMED. GT\review_rules.py:103-104 decides by `turnOrigin` == "human" and not isMeta/isSidechain/toolUseResult. SC\common.py:397-402 decides by `origin.kind == "human"` (plus typed command args). They differ in the field read, and in slash-command and queued prompts (only common handles them). Whether they ever disagree on one real record needs a run over a transcript.
2. K6 `...` fragment rule: CONFIRMED. check.py:153 splits the chose question on "..." and ruling_rules.py:290 does the same, with the same all-fragments-in-question test (check.py:155, ruling_rules.py:291). ruling_rules imports CHOSE from the owner but re-codes this step.
3. K6/K4 card file name: CONFIRMED. common.py:298-299 `card_name` builds `{start[:10]}-{sid[:8]}.md`; check.py:38 `CARD_FILE` parses it. check.py:90-98 also rewrites `agents:` from meta.json, while the brief asks the writer to type it (K4). The brief's file-name lines were not reopened by me.
4. K1 docs/sessions path: CONFIRMED. session_cards_start.ps1:274 `$rel = 'docs/sessions'`; Hydra tests\test_docs_match_code.cpp:1352 `kSessionCardsDir = "docs/sessions/"`; common.py CARDS_ROOT; plus prose. The write-back rule in five places is confirmed in kind (check.py:40-43,193-198 and the brief); the count of five was not independently checked.
5. K1 headings/keys/endings/statuses: CONFIRMED-CHANGED. The statuses "checked"/"failed" are bare literals in check.py:294, index.py:41, link_memory.py:41, number_rulings.py:41,70, prune_list.py:94 and prepass.py:400 (seven files, not five); batch.py:49 also uses "failed"/"condensed". Headings in common.py:51 and the brief exist.
6. K7 "0 to 8", tag count 2-6, "no secret on a card": CONFIRMED for the first two (check.py counts neither; I read check.py). The secret rule: masking is applied to evidence in prepass, not to the finished card; not searched further.
7. K5/K1 hook message and brief restating ref-gate and progress.py rules: CONFIRMED in part. session_cards_start.ps1:268 comment says docs/sessions-only commits need no reviewer and every commit needs the Task, Agent and Session trailers; the owner is the ref gate. The progress.py numbers in the brief were not reopened: open.

## Splits (each its own line)
S1 writer-brief.md:63 ("memory file names from MEMORY.md") vs :87 ("any file ... whether or not MEMORY.md lists it"): CONFIRMED, both lines read. Situation: a memory file the index does not list.
S2 writer-brief.md:79 ("Never write any other 7-to-40-character hex string in Roadblocks or Outcome") vs check.py:176-179 and common.py:78 (HEX needs a digit; check also needs a letter a-f and excludes session-id pieces): CONFIRMED. Situation: an all-digit 7+ char string, or a session-id piece, passes the check but the brief forbids it.
S3 writer-brief.md:75 ("parentheses after a quote may not contain ( or )") vs common.py:96-97 RULING_NOTE (allows one inner level; its comment says so): CONFIRMED. Situation: a note like "(one foo() call)".
S4 batch.py:26 BATCHES_PER_WAVE = 10 vs memory lean-agent-fanout.md:21 "3-4 agents per round": CONFIRMED-CHANGED, weak. I opened the note: its 3-4 limit is for repeated discovery rounds, a different job from card-writer waves, and its other rule says approved independent tasks launch together. Not shown to be one question. batch.py holds no text I found naming the "at-once limit" the hook message cites.
S5 decision log, two entry shapes: CONFIRMED. Log line 7 is `**D1 (finding 28), 2026-10-03.**`; number_rulings.py:104 writes `**{number} (session {sid[:8]}), {date}.**`; 213 log lines start `**D<n> (session`. DECISION_ENTRY reads only the number, so no consumer misreads it; there are two writers' shapes.
Missed splits: none. Side: check.py:39 IGNORED_LINES has only its definition in SC (searched all SC files): unused.

## Rulings
docs\agents\rulings\agents-and-workflow.md:14 (Confirmed): "the important part is more organizing for claude so it can easily reference past sessions and see what topics were covered, what roadblocks were encoutnered, and what the user ruled." It names neither side of S1-S5.

## Summary
Candidates checked 7 (+5 splits); CONFIRMED 5 (1,2,3,4,6); CONFIRMED-CHANGED 2 (5, 7) plus split S4; REFUTED 0; splits added 0; not reached 0.
