# I-notread findings, batch 3 (E1.md records 182 to 218)

Written by a Sonnet scout (agent a2ea5eb98b40ad0f3, task T8-E7-notread-3, session 98fa2925-a91a-4a7a-b151-88fd62c7df0d) on 2026-11-18. The scout opened what each verdict said was not read. It picks no exit. The main session copied its report here, because a scout cannot write files, and spot-checked `common.py` lines 10, 57, 103-113 and `test_common.py:126` against it.

Where the scout looked: `C:\Users\Patrick\.claude\gates\common.py` (all 256 lines), `C:\Users\Patrick\.claude\gates\tests\test_common.py` (lines 70-261) and a grep of `gates\README.md`.

The test file has moved since the audit. The records cite test lines 78-177, and the same tests now sit at lines 102-256 under the same names. The file now also holds lock tests (`update_agent`, `race`) that no record lists; the scout did not judge them.

## records/E1.md:182
`common.run` (common.py:52-75) decodes stdin and runs `json.loads` inside the same `try` as the body (lines 61-63). A bad parse reaches the `except` at line 64. In mode "closed", line 69 returns `deny("Gate hook crashed, so the call is refused: ...")`. The test (now line 102) feeds `b"not json"` in closed mode and checks the decision is "deny". The code does deny, so the question is answered. Still open: nothing from common.py. The test checks only the decision, not the reason text.

## records/E1.md:185
The `.decode("utf-8")` at common.py:62 is inside the same `try`. Bytes `\xff\xfe{` raise, and the loud branch (line 70) returns `system_message("Gate hook crashed and did nothing: ...")`. The test (now line 107) uses those bytes in "loud" mode and checks a `systemMessage` key exists. Answered. Still open: nothing.

## records/E1.md:188
common.py:56-57 raises `ValueError("mode must be 'closed' or 'loud'")` for any other mode, before any input is read. The test (now line 112) passes "maybe" and expects ValueError. Answered. Still open: nothing.

## records/E1.md:191
`write_json_atomic` (common.py:88-100) makes the folder with `os.makedirs(..., exist_ok=True)`, writes to a `mkstemp` file in the same folder, then does `os.replace`. On any error it removes the temp file and re-raises. `read_json` (103-110) reads the file back. The test (now 117-123) uses a real temp folder with nested "a/b" and an "é" value, and checks the round trip and that the folder holds only `s.json`. The code does what the test claims. Answered. Still open: nothing.

## records/E1.md:194
`read_json` (common.py:103-110) catches only `FileNotFoundError` and returns `default`. The test (now line 125) reads "Z:/no/such/file.json". With no Z: drive, Windows raises a not-found error, so the test passes. The scout did not check which drives this machine has. A Z: drive that is offline or not ready might raise a different OSError, which `read_json` would not catch. Still open: the test depends on drive letter Z; a path inside a temp folder would not.

## records/E1.md:200
common.py:10 reads `STATE_ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "state")`. The test (now 157-159) builds `os.path.join(dirname(abspath(common.__file__)), "state")`, the same expression written a second time, so it is a copy of the production rule. `gates\README.md` line 134 says `gates\state\` stays out of git; the scout found no README line stating "beside common.py" as a rule. Still open: whether anything relies on the folder being `state` beside common.py, and whether the test should pin a fixed literal instead of the copy.

## records/E1.md:203
`write_agent` (common.py:135-137) writes through `_agent_path` (127-128), which gives `<root>/agents/<session_id>/<agent_id>.json`; both ids pass through `check_id`. `read_agent` returns None for a missing file. The test (now 161-165) checks None first, then the round trip, then that the file exists at `root/agents/s1/a1.json`. The layout matches the code. Answered. Still open: nothing.

## records/E1.md:206
`check_id` (common.py:113-116) raises ValueError for an empty id, ".", "..", or any id containing `/`, a backslash or `:`. `agents_dir` and `_agent_path` both call it. The test (now 167-171) tries ".." as a session id on write and "a/b" as an agent id on read, and both raise. Answered for those two shapes. Still open: the code also refuses empty ids, ".", backslash and colon, and no test tries those.

## records/E1.md:209
`append_review` (common.py:239-243) appends one line to `reviews.jsonl` with a single O_APPEND write (`_append_line`, 209-217). `read_reviews` (246-247) reads the lines back in file order through `records`. The test (now 226-233) appends a handover and a verdict and expects `[h, v]`; both carry every field that REVIEW_KINDS (line 11) names. Answered. Still open: nothing.

## records/E1.md:212
`append_review` (common.py:240-242) looks up `REVIEW_KINDS.get(record.get("kind"))`; an unknown kind, or a missing named field, raises ValueError. The test (now 235-239) tries kind "other" and a verdict with only `key`, and both raise. Answered. Still open: nothing.

## records/E1.md:215
`append_waiver` (common.py:250-251) appends with no field check; `read_waivers` (254-255) reads the lines back through `records`. The test (now 241-245) writes one waiver and reads back `[w]`. Answered. Still open: the code checks no waiver fields, so the test says nothing about which fields a waiver needs, and common.py does not say whether such a rule is wanted.

## records/E1.md:218
`records` (common.py:220-229) tries `json.loads` on each line, skips a line on `ValueError`, and drops lines that are not JSON objects. The test (now 247-252) appends one good waiver, then `{"prefix": "trunc` with no newline, and expects one record. Answered. Still open: the code skips any bad line, not only the last, and the test covers only the last-line case.
