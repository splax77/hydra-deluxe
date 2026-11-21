# TOOL-AUDIT-T5-V18b (verifier of F18b). All claims "by reading". Spot-checked the load-bearing homes; rows not fully re-read are marked.
Paths: hooks = C:\Users\Patrick\.claude ; repo = C:\Users\Patrick\Downloads\Hydra\hydra-test

## I5-171 real-app record: CONFIRMED
review_rules.py:48 comment says the tool owns the name; :49 `REALAPP_FILE = "realapp.jsonl"`. tools/hydra_ui_mcp.py:81 `REALAPP_FILE = "realapp.jsonl"`. Fields: writer hydra_ui_mcp.py:194-195 `{"kind": "realapp", "commit", "dirty", "commands", "shots"...}`; reader review_rules.py:384-385 `r.get("kind") != "realapp" ... r.get("dirty") is not False ... r.get("commands")`. Two string copies of name and fields. Situation: one side renames, the other still passes its own tests (a test comparing both was not found by the finder; I did not open test files).

## I5-131 budget numbers: CONFIRMED
progress.py:13-15 "The one home of these numbers ... Docs quote them; code reads them here": 100/150, 20/30. Copies found: model-postures.md:5 "inside 100 tool calls and 20 minutes" (task sizing); hooks tools/session_cards/writer-brief.md:101 "At 20 minutes or 100 tool calls, stop starting new cards: finish the one in hand and report"; reviewer.md:11 "at 100 calls or 20 minutes you stop fixing". test_docs_numbers.py:25 DOCS tuple (CLAUDE.md, agent rules, preamble ...), :40 REVIEW_DOCS_HERE lists reviewer.md and the gates README only, :44 regex `tool calls?`. So reviewer.md's "100 calls" would not match the regex; writer-brief is not read by the test. Splits: "calls" vs "tool calls" wording; writer-brief's "stop starting new cards, finish and report" vs the hook's warn-then-refuse. The Confirmed ruling review-and-hooks.md:24 was not re-read by me.

## I5-135 / I5-137: CONFIRMED-CHANGED (partly unchecked)
I read reviewer.md only; the "same branch" lines in DOR/DR and agent_stop.py/README were not reopened (open). Cross-repo: review_dispatch.py:68 docstring says "one exception the user approved on 2026-10-24". A grep of the whole docs/agents/rulings folder for "2026-10-24" found only other subjects (review-and-hooks.md:48 "just move everything to c++"; tests-and-runs.md:15,16,33; ui-and-display.md:47-51). No ruling there approves cross-repo review. I did not search session cards or the decision log.

## I5-170, I5-191: NOT RE-VERIFIED
Out of budget. The finder's "no hook refuses a non-fast-forward merge in ~/.claude" is a negative grep by the finder; open.

## I6-013 kinds: CONFIRMED
DOR:28 "Seven are listed here; kind G is under ..."; DOR:30-36 lists A, B, C, D, E, F, H. DOR:38 "The precheck's lines cover kinds B, C and E, kind D's new numbers, and kind F's candidates." The finder's contrast with DOR:21 was not reopened.

## I6-015 known_clones: CONFIRMED
DOR:38 "A change that adds an entry to `known_clones` is a finding." The ADR is docs/adr/0029-the-single-owner-rules-are-data.md: :138 "14 days", :140 "never kept", :141 "an agent that can't edit a file ... still land its change". So the ADR allows a dated parked entry; DOR calls any add a finding. Which one a reviewer follows is stated nowhere. No Confirmed line naming both sides was seen.

## I6-016 ten steps: PARTLY CONFIRMED
DOR:42-53 are numbered 1-10 as claimed (step 9 visible changes, 10 boundaries). Citations from AR:18, AR:41, check_5.cpp:32, changed_names.h:3 not opened (open).

## I6-024 D13 example: CONFIRMED
DOR:52 "D13 approved "free", not the sentence "no more than 0.0 ms late"". fix-decisions.md:33 is D13; :1680 "D302 ... 2026-10-31 ... This reverses D13". Split: DOR names a decision the user reversed on the same date as the ruling it cites. Whether "free" is the user's own words: not checked by me.

## I6-026 step 10 / Pitch Dark: CONFIRMED
DOR:57-64 chord 466: ratio 1.1067, plan 158,254.77, game 158,268.99, 14.21 ms. docs/clone-hero/05-star-power.md:437 has 1.3926 and 158,262.53 for chord 466 (grep hit both); 1.1067 does not occur on that page. Split: one chord, two pages, different ratio and plan (the page's line was truncated in my view; its activation-2 wording is the finder's). DOR:106 "alright approve b": rulings/review-and-hooks.md has `## Confirmed` at line 7 and `## Unconfirmed` at line 37; the "alright approve b" line is line 39, so it is Unconfirmed, yet DOR cites it as the user's ruling.

## Splits list
1. realapp name and fields written as literals in both repos.
2. Budget wording: reviewer.md vs rules/preamble; writer-brief meaning differs.
3. DOR:38 vs ADR 0029:138-141 on adding a known_clones entry.
4. DOR:52 cites D13, reversed by D302.
5. DOR:57-64 vs 05-star-power.md:437 on chord 466.
6. DOR:106 cites an Unconfirmed line as the user's ruling.
7. review_dispatch.py:68 claims a user approval with no Confirmed line found in the rulings folder.
No Confirmed line names both sides of any of these.

## Not reached
I5-170, I5-191, I5-135 code side, I6-016 citations, a full read of test_docs_numbers.py.
