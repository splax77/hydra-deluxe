# TOOL-AUDIT-T5-V30b (verifier of F30b). All claims "by reading".
Repo read: main checkout. The agent files live in C:\Users\Patrick\.claude\agents (not gates\agents as the finder wrote); they exist.

## I6-138 CONFIRMED (one home)
audit-method.md:87: the main session greps the quoted words of each exit 3 PASS; checkquotes.py "is one". Single home; K8 not shown.

## I6-139 CONFIRMED
Three lists with different members. audit-method.md:126: decision log, question files, plans, session cards (no rulings/ folder). CLAUDE.md:41: only a `## Confirmed` line may be cited. premise-map.md:45: "The quote must sit in a line under `## Confirmed` in a file under `docs/agents/rulings/`, in a session card's quoted words under `docs/sessions/`, or in the user's message as the main session quotes it in the reviewer's brief and confirms." The third place there (a chat message the main session quotes) appears in neither of the others.
Splits: (a) AM:126 omits rulings/; (b) CLAUDE.md:41 omits decision log, plans, cards; (c) premise-map.md:45 adds the chat-quote path and leaves out the decision log and plans. Situation: a closure checker and a plan's reviewer search different sets. No test holds them equal (finder's search; not repeated). No Confirmed ruling names a side.

## I6-140 CONFIRMED-CHANGED
AM:98: "Name the options without leaning. Then give a recommendation and the strongest case against it." Memory counts-on-screen-must-agree.md:21: "If the code already disagrees with itself, ask 'which one, everywhere' ... Never recommend keeping one side 'unchanged' or 'as today'." The split is real; the cases differ (a split in code vs a question in general) and AM:98 does not say so. Missed by the finder: doc-review.md:61 holds its own list for decision questions (predicate quoted, an example from each end, no leaning, strongest case against), a further home of AM:96-98. AM:102 quotes the user ("yes everything should use one rule."), but that is a quote inside a doc, not a filed line.
Rulings: questions-and-audits.md:26 "Yes, require it" backs rule 11 only; :20 "Yes, ban them (Recommended)" backs rule 12 only. Neither covers AM:96-102. A memory note is not the user's words.

## I6-145 CONFIRMED-CHANGED
AM:118-127 lists the items as the finder says. Second copies: AM:26 spells the claim words again; CLAUDE.md:33 the exits; rule 8 / agent-rules.md:14 the library run. review_rules.py:51 has LIBRARY_ALLOW_TEXT = "allow library run". Tests hold the literal: test_progress.py:427,437,455,577,584; test_review_dispatch.py:309; test_review_rules.py:839; and test_posture.py:197,226,234,246 (the finder said test_posture had none; that was wrong). Whether any test also imports the constant: not opened; the shown lines are literals.

## I6-157 CONFIRMED
OM:33 and rule 15 agree. Memory derive-display-from-engine-truth.md:18: "If none exists, propose one rather than copying." scout.md:11: "Never propose a new owner." OM stops: "Nobody writes a new owner for it, because a missing answer to a game question is the user's call, not an agent's." Split: who supplies a missing owner. The cases differ a little (calculation in code vs a question in a plan) and the note does not say so. The finder's rule-15 vs OM `none` row point is text only (a made-up rule vs a missing owner): K3 candidate at most. No ruling quoted.

## I6-161 CONFIRMED
reviewer.md:17 restates OM:47's two steps (open each owner and check its own words claim the question; read the steps with the map closed) and adds BLOCKED on a contradiction, pointing at OM "Who checks it" and "The stops". Two homes. No test (finder's search).

## I6-254 CONFIRMED
public-doc-style.md:3 points at the global file. brief-preamble.md "Plain English" lists lead plain, one idea per sentence, gloss jargon, no file:line bullet walls, then "That file is the one home of those rules." doc-review.md:81 says the same of the global file and lists a different set (gloss on first use, short sentences, no wall of bullets, no chat leftovers). Each restates part while naming one home.

## I7-074 CONFIRMED
kit_tools.py:129 KIT_DEVICES = ("keyboard", "mouse"); devices_problem (533-543) reads f"{d}_now" and refuses unless the value is False ("after setup, not off"). kit.ts:199 `type Device = "keyboard" | "mouse"`, with _now keys also in kit.ts. Two code files hold the device list and key names. Confirmed lines behind the 2026-10-30 answers: only the kit_tools.py:125-128 comment was read.

## I7-089 CONFIRMED-CHANGED
path_index_refusal (game_tools.py:451) refuses a non-integer and "was made from path {index} ... but path_index is {path_index}; nothing started": true. Stronger than the finder's "possible K2": play_chart.py:99 RETIMED_RANGE = "range", applied at :418-438 to every press read outside read_range_ms, asserted in test_play_chart.py:1782-1886. CH:49 names only retimed=late/inside/plain. So the page's word list is incomplete against code that emits `range`. Open: whether the live ch_play_chart path emits `range` in a real run; only a play run settles it, which I did not do.

## I7-041 CONFIRMED
CMakePresets.json:20-26 preset "ship" sets HYDRA_UITEST_ATTACHED OFF. CMakeLists.txt:593-597 default ON, "off for the installer". ui-testing.md:175 and :205 agree today. ADR 0030:139 not reopened. No test (not searched by me).

## Gates tests
Not rerun beyond the grep. Correction above on test_posture.py. The claim that none compares hook text with a doc was not reopened.

## All splits, one per line
1 AM:126 / CLAUDE.md:41 / premise-map.md:45 lists of where the user's words are.
2 AM:98 recommend vs memory "never recommend keeping one side".
3 memory "propose one" vs scout.md:11 and OM stops.
4 BP plain-English list vs doc-review.md:81 list vs the global file.
5 "allow library run" literal in four test files vs review_rules.py:51.
6 device list and key names in kit_tools.py and kit.ts.
7 retimed words: CH:49 omits `range`.
8 percall.py library flag text vs docs' "any run over the whole library" (finder's S3; not reopened).
9 "default 0" in four signatures plus CH:49 (not reopened).
10 OM:47 vs reviewer.md:17; doc-review.md:61 vs AM:96-98.
