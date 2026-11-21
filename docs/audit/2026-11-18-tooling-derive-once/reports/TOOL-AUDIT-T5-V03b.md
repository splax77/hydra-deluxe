# TOOL-AUDIT-T5-V03b (verifier of F03b)

All claims below are by reading. I ran nothing. H = C:\Users\Patrick\Downloads\Hydra\hydra-test; M = the memory folder. Budget: I took a targeted pass; the rows I did not reopen are listed at the end.

## I8-012 approved-work numbers: CONFIRMED
- M\approved-work-is-not-reasked.md:11 "about 130 ms (under the 0.15 s screen delay)"; :13 "(the threaded chart list, about 8 ms)".
- H\docs\agents\rulings\code-and-engine.md:63 "measured at 5.4 ms against 13.8 on 1". Whether it sits under ## Unconfirmed I did not re-check; the finder says it does.
- Code: H\src\ui\app_state.h:434 `kViewProgressDelaySeconds = 0.15`; :445 `kSearchThrottleSeconds = 0.15` (different question, same number; the finder is right).
- H\docs\agents\ui-testing.md:124 contains 0.15 s (grep hit; line too long to quote; I did not read it).
- Splits: (1) T7 "about 8 ms" (note) vs "5.4 ms on 8 threads vs 13.8 on 1" (ruling-file line); (2) the 0.15 s is stated in the note, ui-testing.md:124, ruling ui-and-display.md:97 and the code constant.
- Rulings: code-and-engine.md:8 "why are you trying to convince me to skip something i already approved?" and ui-and-display.md:9 "yes i approved the mockup and asked for option B. so why are you offering what we have today as an option?". Neither names a split; they are the rule "do not re-ask" in the user's words, not a pair of sides.
- K7 claim ("do not re-ask" has no checking home): by reading, I found no hook text for it; I searched only the repo rulings and the note, not the gates folder again. Open: nothing a run could settle.

## I8-050 dmleaderboards: CONFIRMED as K1, weak
Note M\dmleaderboards-comparison.md:19 "warm ~650ms, cold 30s+". The code (finder's quotes of http.cpp:172-185 and dmbot_client.h) says "tens of seconds" with a 120 s receive timeout. I did not reopen the code. "650 ms" has one home. No split.

## I8-057 4 of 50 lanes: CONFIRMED-CHANGED
One home for the count (M\drum-lanes-d112-2026-10.md:17). The finder's side note on drifted line numbers in docs\clone-hero\02-note-list.md was not re-checked by me.

## I8-080 base 361 commits old: CONFIRMED-CHANGED
Finder said one home. There are two: M\git-traps-in-shared-checkout.md:13 "Four executors once started on a base 361 commits old", and the archived note M\archive\agent-worktree-base-is-origin-main.md:11 "An `Agent` call with `isolation: worktree` cuts its worktree from `origin/main`, not local `main`. On 2026-11-07 that was `cdc505eb`, 361 commits behind, and all four timing executors stopped at their start check." The archive note gives the cause; the live note gives only the figure. Situation: an executor started with worktree isolation versus one cut by hand from main. The "no hook" claim I did not re-search.

## I8-126 781k tokens: CONFIRMED
M\one-reviewer-rule-d107.md:13 "~781k tokens and two refused merges". The decision-log copy I did not reopen. Both agree per the finder.

## I8-148 KEY_HOLD_S: CONFIRMED
H\tools\ch_probe\input_driver.py:41 `KEY_HOLD_S = 0.003`. H\tests\single_owner\rules\probe.json:508 owner "KEY_HOLD_S in tools/ch_probe/input_driver.py", with whitelist lines at :527 `KEY_HOLD_S = 0.003` and :532 `self.assertEqual(input_driver.KEY_HOLD_S, 0.003)`. H\tools\ch_probe\tests\test_input_driver.py:118 holds that pin. So this is two homes held equal by the scan rule plus a pin; not a split.

## I8-154 1,300 wheel notches: CONFIRMED
M\search-before-clicking-a-song.md:13 has "1,300 wheel notches". M\render-checks-use-the-corpus.md:19 says "Scroll to the middle of the list, click a song, compare before and after, close, and compare again." The finder's split (one note forbids unasked scrolling, the other tells the step) is real in the words. Whether the render-checks line means the guarded test-command scroll or a shell wheel post is not in the text; a run cannot settle it, only the user. Rulings (the finder gave lines 27 and 48 of ui-and-display.md): the quote at M:13 repeats them. Neither names the render-checks step, so neither names both sides.

## I8-163 video offset 19995.46: CONFIRMED
One home: M\sp-squeeze-mechanics.md:19 "video_ms + 19995.46 (constant, zero drift)". I searched M with a grep on 19995; only that hit besides the archive. 

## Squeeze window pointer (finder's extra): CONFIRMED
Owner: H\src\core\model.h:121 `constexpr double kSqueezeWindowMs = 500.0;` and :133 `within_squeeze_window`. Wrong pointers: skill SKILL.md:10 "`kSqueezeWindowMs`, src/search/graph.h"; squeeze_scout.py:34 `HYDRA_WINDOW_MS = 500.0     # kSqueezeWindowMs, src/search/graph.h`; the skill's line 5 in squeeze_scout.py says the same. graph.h only mentions within_squeeze_window in a comment (graph.h:222), so the pointer is wrong (K8) and the tool holds its own 500.0 and 85.0 (K1/K6, copies outside Hydra's repo). Splits: owner file model.h vs two pointers saying graph.h.

## I8-195 SqOut measured against pre-collection end: CONFIRMED (one home)
SKILL.md:23 and :31 only. My search of src, docs\clone-hero and tools for "pre-collection" found nothing, so the phrase has no code definition; the code's wording is "the one SP end" (model.h, per finder). The Enigma 4750/2083 figures have one home.

## I8-196 phrase collected extends SP +2 measures: CONFIRMED (one home in tooling)
SKILL.md:22 "A phrase whose completion note lands inside SP is collected → SP extends +2 measures." The finder's pointer to kMeasuresPerSpBar and docs\clone-hero\05-star-power.md was not reopened. Light search, as the finder said.

## I7-131 CHECK(acts > 1000) // D43: CONFIRMED
H\docs\agents\test-rubric-examples.md:123 `CHECK(acts > 1000);  // a floor the user approved (D43)`. Code: H\tests\test_sp_end.cpp:793 and :830. The same comment form is at H\tests\test_fills_and_skips.cpp:305 (`skipped > 100`). Decision log line 103 holds `acts > 1000`; the finder says it names test_search.cpp there (grep shows test_search.cpp appears 3 times in the log; I did not read line 103 itself). The test-rubric.md lines 13 and 88 also hold it (grep hits). Split: decision log's file name vs where the code is today (K2), as the finder said, unconfirmed on the line text.

## I7-133 "engine.cpp line 2015": CONFIRMED (K2)
test-rubric-examples.md:88 "The search stamps `bank_rise_ticks` (`src/search/engine.cpp` line 2015)". I read H\src\search\engine.cpp:2015: `const ScoreGraphNode* node = en.nodes[(size_t)oa.act_node];`. That is not a bank-rise line. Whether it ever was: open (git log would settle; I ran nothing).

## I7-155 stall harness: not reopened.

## I7-189 "two web searches on 2026-10-10": not reopened.

## I7-224 doc picture size limits: CONFIRMED (K6)
H\tools\doc_shots\shrink.py:20-21 `MAX_FILE_BYTES = 500 * 1024`, `MAX_FOLDER_BYTES = 4 * 1024 * 1024`; comment :19 "tests/test_docs_match_code.cpp checks the same two at merge." H\tests\test_docs_match_code.cpp:2024-2025 `kDocImageMaxBytes = 500 * 1024`, `kDocImagesMaxTotalBytes = 4 * 1024 * 1024`, :2021 "(D300 in docs/audit/2026-10-03-fix-decisions.md)". My grep of tools and tests for those four names found no file that reads one side's value to compare with the other. The finder's wording (shrink.py "fails a capture run on the same limits") was not seen in test_docs_match_code.cpp by me; instead :2021 cites D300. Values agree today.

## I7-226 doc-screenshots "When to run": CONFIRMED (one home)
H\docs\agents\doc-screenshots.md:21-23 "Run it before each release ... Also run it after any change to a label or a layout that one of the pictures shows." My grep of doc_shots.ps1 and test_docs_match_code.cpp for those phrases found nothing, so no checking home in those two files. Gates folder not searched by me.

## Splits listed
1. T7 time: 8 ms vs 5.4/13.8 ms.
2. Squeeze window owner: model.h vs graph.h (skill SKILL.md:10, squeeze_scout.py:34 and line 5).
3. squeeze_scout.py holds its own 85.0 and 500.0.
4. D43 file name: decision log vs test_sp_end.cpp.
5. engine.cpp line 2015 is not a bank-rise line.
6. render-checks "Scroll to the middle" vs never-scroll-unasked.
7. Picture limits in two code places.
8. 361-commit base: figure in the live note, cause only in the archived note.

## Not reached or not reopened
I8-126 decision-log copy, I8-050 code lines, I8-057 song.cpp pointers, I7-155, I7-189, and the gates folder search for K7 rows.
