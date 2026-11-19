# TOOL-AUDIT-T5-V17 (verifier of F17)

All claims below are "by reading". I opened the homes named; I did not re-read every line the finder cites. Budget: I checked the candidates that carry splits or code claims in full, and the copy-only (K1/K5) ones by sampling the copies.

## Candidate checks

1. K1 allow phrase copies (Q1, Q2). CONFIRMED. review_rules.py:50-55 holds `LIBRARY_ALLOW_TEXT = "allow library run"` and the key twin; brief-preamble.md:49 and agent-rules.md:14 state the phrase again. Differs for: any agent reading the preamble vs the hook.
2. K2 "exactly" vs case/./!. CONFIRMED. brief-preamble.md:49 says "sent exactly `allow library run`". review_rules.py:49-51 comment: "a trailing . or ! is allowed", and `re.IGNORECASE`. Edge: a message `Allow Library Run!`.
3. K7 rule 8 vs hook sees only the flag. CONFIRMED. agent-rules.md:14 covers "any run over the user's whole song library ... whole-library compare or timing run"; percall.py:14 `LIBRARY_FLAG = "--whole-library"`, brief-preamble.md:49 says the hook refuses "any shell command that holds `--whole-library`". Situation: a compare run with no flag in the command text. The hook gap exists (general words only).
4. K1 sign-off sentence x4 (Q3). CONFIRMED. The "no sign-off line is recorded CLEAN" sentence is in reviewer.md:33 area, doc-review.md:125, derive-once-review.md:151 area, README. Owner agent_stop.py:23 `_BLOCKED, _CLEAN`. I saw reviewer.md:28-33 and doc-review.md:120-125 directly; README:43 and derive-once-review.md:149-151 by the finder's quote plus a grep hit at 151.
5. K2 "markdown" vs suffixes (Q6). CONFIRMED. agent_stop.py:163 `DOC_SUFFIXES = (".md", ".html", ".htm")`; derive-once-review.md:151 "fingerprints every markdown file the range changes"; reviewer.md:33 same area; derive-once-review.md:202 "every markdown file in the range". Situation: an .html page in the range.
6. K3 stall.py sentence (Q4). CONFIRMED. stall.py:280-281 ends every relayed problem with "A move of main to a tip with no verdict is refused." agent_stop.py:243 sets `problem = "Reviewer %s's verdict: ..."` when `outcome == "verdict recorded"` (a verdict WAS recorded); line 250 is the no-verdict text. Situation: recorded verdict with problems (missing sign-off line, fallback tip).
7. K3 percall "closed" inside prompt.py (Q12). CONFIRMED-CHANGED. prompt.py:17 `PARTS = (("posture","loud"),("percall","closed"),("waiver","loud"))` and prompt.py:67 `common.run(body, "loud")`; prompt.py:6-7 says a deny becomes systemMessage. So percall's "closed" is a mode string run per part, and the module docstring says it is deliberate; the real two-home fact is K1/K6: percall's mode is written in percall.py:176 and again in prompt.py PARTS. Whether a crash inside the part really informs (not refuses) is a run question: a crash drill of prompt.py (not run).
8. K6 brief hash made in two files. CONFIRMED. review_dispatch.py:172 `hashlib.sha256(prompt.encode("utf-8"))`; agent_stop.py:199 `hashlib.sha256(first.encode("utf-8"))`. Whether both strings are byte-equal (join of text blocks vs prompt) is a run question: a dispatch plus stop drill.
9. K6 "did the user type it". CONFIRMED. review_rules.py:103 reads `turnOrigin`; session_cards/common.py:397 and :433 read `origin.kind`, :433 also requires `commandMode == "prompt"`. Two code homes with different fields. Which transcripts carry which field is a run/data question (not checked).
10. K6 "which ref is main" (six places). CONFIRMED. ref_judge.py:33; agent_stop.py:170; publish_gate.py:47; review_dispatch.py:111,123,128,136; cleanup_sweep.ps1:270; review_rules.py has more uses. "Gated repo" in three files: publish_gate.py:47 (marker on main), review_dispatch.py:128-136 (_has_main plus marker), review_rules._has_marker: seen.
11. K6 `--whole-library` in percall.py:14 and src/cli/batch_scope.h. The percall.py side CONFIRMED; I did not open batch_scope.h (not reached). Listed as unchecked.
12. K1 public pages list. CONFIRMED on the code home (review_rules.py:24 PUBLIC_DOCS, four pages) and CLAUDE.md "four public pages"; doc-review.md:17 and public-doc-style.md:3 not opened by me; finder's quote taken. Comment at review_rules.py:23 says "(the user's call, 2026-10-10)"; I did not find a Confirmed line for it.
13. K4 waiver `<key>` vs hex prefix. CONFIRMED. review_rules.py:26-28 accept 7-40 hex (derive-once, real-app) and 7-64 (doc-review); docs say `<key>`. Edge: user types 7 hex, a 40-char hash, or the refusal's first-10.
14. K8 markers. CONFIRMED. `.derive-once-gate` says "Test-DeriveOnceGatedPath in the user-scope hook (hooks\lib\derive_once_rules.ps1)"; `.doc-review-gate` says "(hooks\lib\doc_review_rules.ps1)". `C:\Users\Patrick\.claude\hooks` does not exist; `gates\lib` holds only hook_input.ps1; no derive_once_rules.ps1 outside worktrees (find, depth 3). review_rules.py:21 BUILTIN_CODE and :24 PUBLIC_DOCS are the live lists. Searched: .claude depth 3 excluding worktrees; deeper not searched.
15. K5 (docs spelling hook rules). CONFIRMED as a pattern for preamble:49, agent-rules:14, reviewer.md:31-33, doc-review.md:123-125. Not opened: clone-hero-mcp.md:29, server.py:146-148 (ch server text in this session does say "a hook refuses them unless the user has typed allow key run", so that copy exists).
16. Small-text limit one home. CONFIRMED the home: publish_gate.py:13-14 `SMALL_LINES, SMALL_CHARS = 20, 2000`; comment "user's decisions of 2026-10-10". "Over 20 lines" is in review-and-hooks.md:22 per the finder (not re-opened); 2000 has no Confirmed line (as the finder says; I did not grep every ruling file for "2000").

## Splits (one line each; the finder's eight, checked, plus mine)
1. CONFIRMED. Docs "exactly `allow library run`" vs review_rules.py:49-51 accepting case and a trailing `.` or `!`.
2. CONFIRMED. Rule 8 / agent-rules.md:14 cover any whole-library compare or timing run vs percall.py:14 judging only a command holding `--whole-library`.
3. CONFIRMED. stall.py:280-281 vs agent_stop.py:243.
4. CONFIRMED. "markdown file" (derive-once-review.md:151,202; reviewer.md:33) vs agent_stop.py:163 three suffixes.
5. CONFIRMED. Marker files name hooks\lib\*.ps1; review_rules.py:21-24 holds the lists.
6. CONFIRMED. review_rules.py:103 (`turnOrigin`) vs session_cards/common.py:397,433 (`origin.kind`, `commandMode`).
7. CONFIRMED-CHANGED. derive-once-review.md:200 lists "no key, a key already on main, a key git does not know or more than one commit"; README.md:39 adds "a commit in a repo that has no local `main` branch". Four vs five. (The finder cited derive-once-review.md:200; the actual list sits in the text beginning at line 200 and the longer README text matches the finder.) Same hook: review_dispatch.py:128.
8. CONFIRMED. rulings/README.md:20 "copied from memory notes on 2026-10-31"; review-and-hooks.md:10 is a 2026-11-06 line (I did not check that line 10 sits under `## Unconfirmed`; the finder says lines 40-43 do).
9. (mine) percall "closed" in percall.py:176 and prompt.py PARTS vs prompt.py:6-7 "A prompt cannot be refused here": a hook that is "closed" when run alone and informing when run through prompt.py.
10. (mine) review_dispatch.py:128-136 returns not-gated when there is no local main; publish_gate.py:42-60 per finder throws on unreadable git. Not re-read for the throw; marked by reading of review_dispatch only.

## Rulings
A `## Confirmed` line naming both sides of a split: none found among the ones the finder quoted. review-and-hooks.md:16 (user "wants to give explicit approval for a full library run") names neither the phrase nor its case rules. Line :19 (no sign-off line records CLEAN) names the rule, not the four copies. I did not re-open the ruling files; quotes are the finder's.

## Not reached
batch_scope.h (candidate 11 second home), clone-hero-mcp.md:29, server.py, doc-review.md:17, public-doc-style.md:3, README:43/45/55, rulings lines, publish_gate.py throw behavior, live settings.json (as F17).

## Summary
Candidates checked: 16 (finder's K1-K8 groups). CONFIRMED 14; CONFIRMED-CHANGED 2 (percall-in-prompt, 4-vs-5 dispatch list); REFUTED 0. Splits added: 2. Not reached: 7 homes (listed above).
