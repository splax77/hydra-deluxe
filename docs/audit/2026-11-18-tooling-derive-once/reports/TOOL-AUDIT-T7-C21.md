# TOOL-AUDIT-T7-C21 (batch 20; sorting S10, ledger L10)
Legend: current files read in the main checkout C:\Users\Patrick\Downloads\Hydra\hydra-test (docs/agents/...), line numbers are today's. CH = clone-hero-mcp.md, DOR = derive-once-review.md, AM = audit-method.md, OM = owner-map.md, BP = brief-preamble.md, AR = agent-rules.md, PC = CLAUDE.md.

### L-F30-08
**Verdict:** PASS (P)
**Homes read:** docs/agents/triage-labels.md:6-12 (all five rows have identical left and right strings; :16 "Edit the right-hand column..."); CLAUDE.md:17 "each label string equal to its name".
**Why:** The two homes say the same today. A pointer changes no behaviour and needs no choice.

### L-F30-14
**Verdict:** PASS (P)
**Homes read:** AM:116-127 (list); AM:26 (the "on purpose, by design, deliberately, kept" words, same list as AM:123); AM:19,121 (four exits); BP:49 and AR:14 (rule 8: both say `allow library run`, same guards as AM:125); BP:103 ("A search that finds nothing proves nothing on its own", the rule AM:124 points at); test-rubric.md:189 (four exits, fresh checker). Searched docs/agents/*.md and CLAUDE.md for "by design|on purpose" and "four exits|allow library run": no other list that differs.
**Why:** Each copy restates or points at the same rule; none contradicts AM:118-127. Replacing copies with a pointer changes nothing.

### L-F30-16
**Verdict:** PASS (P)
**Homes read:** OM:47 ("opens each owner and reads its own words against the row's question... reads the plan's steps with the map closed... fixes by naming the row's owner... If no owner exists, BLOCKED"); ~/.claude/agents/reviewer.md:17 (same two checks, BLOCKED on contradiction, points to OM "Who checks it" and "The stops"); planner.md:19; scout.md:11; AR:25; PC:55-56.
**Why:** reviewer.md:17 is the same two steps with a pointer; it leaves out OM's no-owner BLOCKED line but points at the page that has it, so nothing disagrees. Note: the agent file lives outside the repo.

### L-F30-18
**Verdict:** PASS (P)
**Homes read:** tools/ch_mcp/kit_tools.py:129 `KIT_DEVICES = ("keyboard", "mouse")`, :533-543 `devices_problem` (reads f"{d}_now", only False passes); tools/ch_mcp/agent/kit.ts:199 `type Device = "keyboard" | "mouse"`, :380-381, :425-427, :759-760 (keyboard_now, mouse_now); CH:25, CH:27. Searched ch_mcp .py/.ts for other uses of keyboard_now/KIT_DEVICES: none outside these two files.
**Why:** The device list and reply keys are the same in both programs today. Doc agrees with code.

### L-F30-19
**Verdict:** PASS (A)
**Homes read:** CH:49 (names only retimed=inside, retimed=plain, retimed=late; no "range"); tools/ch_probe/experiments/play_chart.py:96-99 (RETIMED_LATE/INSIDE/PLAIN/RANGE="range"), :418 and :438 (a press read outside read_range_ms gets RETIMED_RANGE).
**Why:** The one split is a doc word list against the code that emits the words. Doc against code; the code wins. The "path_index default 0" part has no disagreement.

### L-F30-20
**Verdict:** PASS (A)
**Homes read:** DOR:66 ("moves each later press by the read end minus the planned end, one for one", and the table row below it saying the arithmetic carries one for one); tools/ch_probe/experiments/play_chart.py:361-369 `own_end_now_ms` (own_stored_end_ms + conversion[side] * (read_end_ms - read_stored_end_ms)), :390-440 `retime_from_end`; CH:49 (the player multiplies the read's error by the file's `conversion`).
**Why:** The code multiplies by the file's conversion; DOR:66 says one for one. Doc against code, code wins. Caveat: DOR:66's table row ("none: ... does not call the engine") was built on the one-for-one claim, so the fix should re-read that table, but that is part of the doc fix, not a second split.

### L-F30-21
**Verdict:** PASS (P)
**Homes read:** CMakePresets.json:20-26 (ship sets HYDRA_UITEST_ATTACHED OFF); CMakeLists.txt:595-596 (option default ON, "off for the installer"); ui-testing.md:175, :205; docs/adr/0030-hydra-owns-every-live-command-rule.md:139-141.
**Why:** All say the ship preset leaves it out and dev builds have it. No disagreement.

### L-F30-22
**Verdict:** PASS (P)
**Homes read:** AM:87 (names checkquotes.py as "one" script for the quote check); docs/audit/2026-10-29-audit-of-audits/briefs/checkquotes.py exists.
**Why:** Single home, no split.

Ids not reached: none.
