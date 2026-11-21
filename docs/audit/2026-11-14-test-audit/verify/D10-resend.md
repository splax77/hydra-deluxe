Agent a9204ec5406d07b09 (T5-V-RESEND-D10), verify re-send of the 11 records `verify/D10.md` did not reach.

# Verify re-send, slice D10

Marks: "by reading" means I opened the lines and the claim holds. I ran nothing. Files read in full: `tools/ch_probe/tests/test_engine_finder.py` (all), `tools/ch_probe/tests/test_overlay_logs.py` (all), `tools/ch_probe/engine_finder.py` lines 20-247, `tools/ch_probe/experiments/overlay_logs.py` (all), the `fake_process` reader in `tools/ch_probe/tests/fakes.py:54-76`, `constants.py:295-298`. Q3 follows the first verifier's rule: a claim about a real heap or real log content needs a saved real capture; the records' Q3 for these files is accepted as written.

## Counts

11 records checked. 9 agreed on verdict (two of them with a note), 2 disputed (both in engine_finder).
No record left unreached.

## test_engine_finder.py (4 records)

- `test_gives_up_after_the_cap_with_the_exact_message` (:129): Rewrite, agree. By reading I worked the fake clock through. Each round sleeps 0.12 then 0.4 (0.52 s). `pause` raises when `now() - start >= cap_s` (`engine_finder.py:196`). Rounds 1 to 4 end at 2.08 s, and round 5's first `pause` sees 2.08 >= 2 and raises. So 8 sleeps, `[REREAD_GAP_S, RETRY_PAUSE_S] * 4`, as the test says. The message "2 s" comes from `{cap_s:g}` of 2.0 (line 198). The record's point that the 4 is a snapshot of the two pause constants holds: change either pause and the 4 changes with no bug.
- `test_default_cap_message_names_120_seconds` (:145): record says Remove. I dispute: should be Rewrite (fold into :73 so both entry points are checked), not Remove. Reason: the record says the wrapper's default "cannot differ without an edit". An edit is exactly what a test is for. The wrapper has its own default, `cap_s: float = ENGINE_SEARCH_CAP_S` (`engine_finder.py:236`). Live callers use that wrapper default: `play_chart.py:1274` and `sp_watch.py:173` pass no cap. The bare-finder test (:73) reaches the other default (line 173), so :145 is the only test of the wrapper's default. Q3: the fake clock stands in for the real one, claim not checked beyond that (the cap is a timing claim; a real-clock run would need a 120 s wait, F7 says the 120 has no source).
- `test_returns_the_engine_found_inside_the_cap` (:159): record says Remove, "repeats :90". I dispute: should be Investigate, "claim not checked only", not Remove. The wrapper's `return` (line 246) is checked by no other test. :129 and `test_find_checksum.py:309` only reach the raising path (the second goes through `run_finder` and the exit code 2 path). If the wrapper lost its `return`, every live caller would get `None` and only :159 turns red, at line 168. Dropped forwarding of `scan`, `sleep` or `now` would also fail here at line 169-170, but :129 catches those too.
- `test_all_patterns_tries_the_precision_pair_both_ways` (:174): Investigate, claim not checked only, agree. Lines 182-184 match `normal_pattern` and `all_patterns` (`engine_finder.py:154-164`): normal, (p1,p2), (p2,p1), each read from its own RVA through distinct byte strings, so a swapped RVA or a missing order turns it red. F4 (open Q-34) stands.

## test_overlay_logs.py (7 records)

All seven test bodies are a typed list compared with a module constant, and each typed list matches the module today (I counted: 18, 25, 23, 16, 16, 15, 10 names; the lengths agree with the concatenation at `overlay_logs.py:145-158`). So all seven pass today, and each goes red on any edit to a list, deliberate or not.

- `test_watch_columns` (:23): Rewrite, agree. Writers and readers already use the module's own list: `test_sp_tools.py:204` compares a written table's header to `WATCH_COLUMNS`, and `sp_compare.py:153` reads logs against it. So a rewrite on the writer side already exists; this test is the one place a rename is pinned independently, which the module's own docstring (lines 3-5) says it should not need. Verdict Rewrite stands, but a Remove is the alternative the main session may weigh.
- `test_press_log_columns` (:44): Rewrite, agree, same note: `test_play_chart.py:356`, `:986` and `:995` already compare a written press log's header to `PRESS_LOG_COLUMNS`, and `score_compare.py:115-117` already accepts the whole older family. The record's proposed rewrite ("check a log `play_chart` writes has these columns and each reader takes it") is already those tests; the record did not find them (it lists `test_play_chart.py` as not read).
- The five "before the ..." tests (:54, :64, :71, :78, :85): Rewrite, agree. I checked the record's saved-log claim on the real files: all four `*_presses.csv` under `tools/ch_probe/experiments/results/overlay-spike/` have only `index,chart_ms,clock_ms_at_press,score_before,score_after,hit` (the six `KEYS_PRESS_COLUMNS`), so none of the five lists has a real log behind it. Whether the readers accept each older list is decided at `score_compare.py:115-117`, which the test never exercises with the older lists. The property that can really break is that each list stays the prefix or swap of the next (`overlay_logs.py:145-158`); the typed copies catch that only as a side effect. One detail the record gets right: the test named "before the send column" checks `PRESS_COLUMNS_WITH_HIT`, the list that still has `hit`, as the module docstring (lines 41-44) says.

## Splits, one line each

- S1: the cap's default is written twice, once in `find_live_engine` (line 173) and once in `find_live_engine_capped` (line 236), both from `ENGINE_SEARCH_CAP_S`, and the docstring at 244-245 says "the cap itself lives in find_live_engine". The record treats the wrapper's default as unreachable; the live callers use it.
- S2: the press-log and watch-log column lists are pinned three ways (typed in `test_overlay_logs.py`, against the module in `test_play_chart.py` and `test_sp_tools.py`, accepted by `score_compare.py:115`). The records name only the first.
- S3: a side note on `test_a_candidate_two_patterns_find_is_checked_once_in_scan_order` (:102, not in my list, read in passing): if the dict at `engine_finder.py:203` became a list, a repeated candidate's second read would pop the clock and the test would hang in the `while True` loop rather than fail cleanly.

No rewrite in these records proposes a saved expected-output file.
