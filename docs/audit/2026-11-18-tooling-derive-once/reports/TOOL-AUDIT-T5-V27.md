# TOOL-AUDIT-T5-V27 (verifier of F27). All claims "by reading". Nothing was run.

Budget note: I checked the candidates that carry the finder's splits and owner claims. The rest were not reached (list at the end). No `## Confirmed` ruling names both sides of any split below; I did not find one for them.

## Checked

### I9-098 (parked cleared in two places): CONFIRMED
- gates/progress.py:77-79: "A call means the agent is no longer parked ... " then pops `parked` and `parked_since`.
- gates/stall.py:~236: shows "parked since" only `if rec.get("parked") and _is_time(since) and not (_is_time(last) and last > since)`.
- Two code places answer "does a later call end parking". Differ only at `last == since`; by reading, progress clears the flag on any call, so stall's test is a second guard. Situation: a stale record where `parked` is set but `last` is not newer.

### I9-131 (document key): CONFIRMED
- review_rules.py:65-71 `normalized`: drops a UTF-8 BOM and does `replace(b"\r\n", b"\n")` only. Docstring: "publish_gate.py's small-text count reads the same bytes."
- publish_gate.py:19-29: calls `review_rules.normalized(raw)` and then also `.replace("\r\n","\n").replace("\r","\n")`, and decodes UTF-16 itself (a UTF-16 file never goes through `normalized`).
- So the count and the key treat a lone CR and UTF-16 differently. This contradicts the `normalized` docstring "reads the same bytes" for UTF-16 input. Split (new): the key path and the count path disagree on UTF-16 input and on lone CR.
- The "CRs removed" wording vs CRLF-only code: the code side is confirmed (CRLF only); I did not open the test row text.

### Hand-back tool names (side note 3 / I9-092): CONFIRMED as a K6 pair
- progress.py:19 `REPORT_TOOLS = ("SubagentHandback", "StructuredOutput")`; percall.py:168 uses `progress.REPORT_TOOLS`, but percall.py:102 hard-codes `== "SubagentHandback"` for saving the message. Two spellings of "which call is a hand-back": one list covers both tools, the save covers only SubagentHandback. Situation: an agent that ends with StructuredOutput is exempt from the budget but its message is not saved.

### I10-007 (removed-file citation): CONFIRMED
- tools/check_pointers.py:38: `` `git show ([0-9a-fA-F]{7,40}):([^`\s]+)` `` (backticks required; upper or lower hex).
- tests/test_docs_match_code.cpp:1334-1346 `is_removed_citation`: matches "never committed" (any case), or a hash `[0-9a-f]{7,40}:` (lowercase) followed by the docs path, with no backticks needed.
- Splits (separate lines): (1) backticks required in Python, not in C++; (2) uppercase hash accepted by Python, refused by C++ (the lead-in class takes A-F but the hash itself is lowercase only); (3) "never committed" accepted by C++ only.
- Note: IN-76 line 5 in 07-internals.md uses the form `(removed; git show 6804353f:...)` without backticks around git show; the C++ test accepts it, the Python form needs backticks. Not run.

### I10-017 (what a commit hash looks like): CONFIRMED
- check_handoff.py:28 `_FULL_HASH` = `^[0-9a-fA-F]{40}$` (upper allowed).
- review_rules.py:56 `_HEX` = `^[0-9a-f]{40}([0-9a-f]{24})?$` (lower only, 40 or 64).
- review_rules.py:26-28 waivers: `{7,40}`, `{7,64}`, `{7,40}`, all case-insensitive via `(?i)`.
- doc_tokens.py:39 `_HEX` 7-40 either case; :50 `_BARE_HEX` 7-40 lower only.
- Splits: (a) upper-case 40-hex passes check_handoff, fails review_rules._HEX; (b) doc_tokens `_HEX` takes upper, `_BARE_HEX` does not; (c) doc-review waiver takes up to 64, the other two waivers up to 40; (d) waiver phrases are case-insensitive while `_HEX` in the same file is not. Finder missed (d). I did not open session_cards' HEX or the digit rules.

### I10b-022/023 (song.ini reader): CONFIRMED
- Python tools/ch_mcp/game_files.py:276-290 `_song_section_lines`: docstring "Keys before any section header count too", `section: Optional[str] = None`.
- C++ src/parse/song.cpp:2217-2238: `bool in_song_section = false;` and `if (!in_song_section) continue;`, so a key before any `[song]` header is ignored.
- Split 1: keys before any section count in Python, not in the engine.
- pro_drums: Python game_files.py:348-350 `.lower() in ("true", "1")` (no trim of the value beyond what read_song_ini does; not checked). Engine song.cpp:2274-2283 `ini_drum_key_is_true`: any non-zero int32 or trimmed case-insensitive "true". Split 2: `2`, `-1` count in the engine only.
- Python's own use is a printed note (per finder, bot_run.py:762-764; not opened).
- The `.mid` vs `.chart` order split: not checked.

### I10b-028 (dynamics-stop bit): CONFIRMED
- frida_tools.py:66-68 `DYNAMICS_STOP_BIT = 0x10`, comment names SC-16; 06-scoring.md:175-177 says "bit 0x10 of the engine flags word". constants.py has no 0x10 entry (grep of `0x10\b` empty). Same value, two homes in different formats; constants.py's "ONE place" header (quoted by finder, not re-opened) does not cover it.

### I10b-033/034/035 (IN-76 versus IN-82): CONFIRMED-CHANGED, mostly REFUTED as a wrong owner claim
- kit_tools.py:62-63: "The map setup loads onto the kit's player, by its map category ("Gameplay") and layout ("Drums") ... The fact's home is block IN-76". The comment is about the map category and layout only, not the definition id or name.
- IN-76 (07-internals.md:833-835) does hold it: "Player1 to Player4 each list map category 1 ("Gameplay") with layouts 0 ("Default") and 2 ("Drums")". So the finder's claim that IN-76 does not hold the fact is wrong for the values the comment covers.
- IN-82 (:904-908) also states "id 2 at 984680, category 1 at 984684, layout 2 at 984688" and ends "The action names are those of `KIT_BINDINGS` in tools/ch_mcp/kit_tools.py". So category 1 and layout 2 sit in two doc blocks (IN-76 as the player's map list, IN-82 as the kit's map record) and in kit_tools.py. Same values, no disagreement found.
- Real remaining candidate: the action names live in kit_tools.py and IN-82 points at the code (a doc that names the code as the owner), while the code comment for definition id/name (`KIT_DEFINITION_ID`, "MIDI Drums", kit_tools.py:~59-60) names no home. Not graded.

### Test claims
- test_docs_match_code.cpp:1334 and check_pointers: checked above. The other "tests hold homes equal" (test_frida_tools.py:789, cpp:996-1010, :2036, test_docs_numbers.py:42) were not opened.

## Splits added by me (beyond the finder)
- publish_gate count vs review_rules key: UTF-16 input and lone CR (above).
- percall save-list (SubagentHandback only) vs progress.REPORT_TOOLS (two tools).
- Waiver phrases case-insensitive, `_HEX` lowercase only, same file.
- Check_pointers "git show" lowercase/uppercase vs C++ lowercase-only hash, listed per item.

## Not reached (not checked against files)
I9-092, I9-097, I9-109, I9-116, I9-140, I9-143, I9-146, I9-154, I9-163, I9-174, I10-006, I10-011, I10-012, I10-014, I10-018, I10-019, I10-020, I10-025, I10-029/030 (only via I10-017), I10-041, I10-060, I10-062, I10-077, I10-081, I10-087, I10b-012, 014, 017/018, 030, 039, 041, 051, 053, 054, 059, 066, 072, 080, 090, 101, 108, 111, 115.

## Counts
Candidates checked: 8 (I9-098, I9-131, hand-back list, I10-007, I10-017, I10b-022/023, I10b-028, I10b-033/034/035).
CONFIRMED: 7. CONFIRMED-CHANGED: 1 (IN-76 vs IN-82). REFUTED: 0 whole (the IN-76 "does not hold the fact" part is refuted).
Splits added: 4.
Not reached: about 45 rows.
