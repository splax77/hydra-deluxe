Agent id: a02766b0aa1d5f55a (task T8-E3-V-L-C06-L-C07, session f3bbb32b)

Not the scouts' ids (a7682c5853cd449ff for L-C06, aa5cc74ed3fd44c70 for L-C07). Read from the tests: test_library_model.cpp, test_model.cpp, test_list_scroll.cpp, test_live_protocol.cpp, test_note_shuffle.cpp, test_midi.cpp, and under tools/ch_probe/tests the files test_play_in_game.py, test_prep_inputs.py, test_probe_chart.py, test_probe_songs.py, test_process.py, test_process_readonly.py and test_score_compare.py. The records (D12, B2, C5) were not opened beyond what the counter's reason says; the GF and the other MIDI rows were taken from the counter's reason.

## Counts

- L-C06: 85 rows in the batch, 85 lines in the label file, 79 rows agreed, 6 rows changed.
- L-C07: 54 rows in the batch, 54 lines in the label file, 46 rows agreed, 8 rows changed.

Every batch row has exactly one line in its label file. Each ends-table row names rows that are in the batch and carry that key in the scout's file. After my changes the ends tables need three edits, listed after the table below.

## Rows I change

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-C06 | counts/T5-IR15.md:44 | F-result | F-row | The made-up -315.8 and 0.0 are the fourth argument of `ready(1000, "1", 7, ...)` (test_library_model.cpp:703), a Library summary row, which the key list files under F-row. |
| L-C06 | counts/T5-IR15.md:61 | Z-cite, S-snapshot, K-unreachable | Z-cite, S-snapshot, K-unreachable, F-result | The counter marked X9 and none of the three keys is an F, O or C key; the fault it names is the typed expected rows (and a tick no chart can reach), so F-result is the nearest F key. |
| L-C06 | counts/T5-IR15.md:67 | P-setting, E-echo | P-name, E-echo | The typed "+", "-", "SqIn" and "SqOut" are symbols and type names the product writes (test_model.cpp:138-144), which cross-findings X8 calls names, not screen words. |
| L-C06 | counts/T5-IR15.md:79 | F-row | F-row, K-unreachable | The test's own comment says backend rows past a squeeze-out are impossible in game (test_model.cpp:747-750), and the counter says "a state the engine cannot make"; this is the same fault the scout keyed on :61. |
| L-C06 | counts/T5-IR15.md:103 | P-setting | P-setting, P-name | `dynamic_label` words ("Ghost") are screen words, but `dynamic_str` ("ghost", "none") is text the product writes (test_model.cpp:1383-1390). |
| L-C06 | counts/T5-IR15.md:104 | P-setting | P-name | "SqIn" and "SqOut" are the product's type names, read back by `squeeze_kind_from_name`, not screen words (test_model.cpp:1392-1400). |
| L-C07 | counts/T5-IR16.md:14 | O-gamemem, P-message, E-echo | O-gamemem, E-echo | The counter marks X9 and X11 only; the test types the agent's error text, passes it in through FakePlayer and asserts it is in the message (test_play_in_game.py:240-247), which is an echo, and it types no sentence of its own message. |
| L-C07 | counts/T5-IR16.md:42 | O-process | O-gamemem | `fake_process({}, base)` is a Process over fake game memory (test_process.py:189), not a process list; O-gamemem is "game memory". |
| L-C07 | counts/T5-IR16.md:46 | F-presslog | F-presslog, F-summary | SC:91 runs `_statuses` on `_presses` and the hand-written `_SCORE` (test_score_compare.py:33-59, 91-93), the same two stand-ins the scout keyed on :45. |
| L-C07 | counts/T5-IR16.md:47 | F-presslog | F-presslog, F-summary | SC:95 uses the same `_presses` and `_SCORE` (test_score_compare.py:95-100). |
| L-C07 | counts/T5-IR16.md:48 | F-presslog | F-presslog, F-summary | SC:102 checks `_statuses_of(rows)` against `_SCORE` (test_score_compare.py:102-107, 130-132). |
| L-C07 | counts/T5-IR16.md:49 | F-presslog, P-message | F-presslog, F-summary, P-message | SC:109 reports on `_SCORE` and typed presses and types the FAIL message (test_score_compare.py:109-113). |
| L-C07 | counts/T5-IR16.md:50 | F-presslog, P-message, E-echo | F-presslog, F-summary, P-message, E-echo | SC:136 calls `_report` on `_SCORE` as well (test_score_compare.py:136-148). |
| L-C07 | counts/T5-IR16.md:96 | P-tiers, Z-cite | P-other | The test types velocity bands of the drum-lane reader (45, 96, 25, test_midi.cpp:892-906), which are not the squeeze timing tiers, and its comments name no source, so there is nothing for Z-cite. |

## The scouts' own doubts

- L-C06, P-setting against P-message. Right as the scout drew it: row strings, status words and chip words are screen words (cross-findings X7); sentences and advice ("Hit [Yellow cymbal] last.", "No Hard Pro Drums notes in this chart.") are messages. I changed only :67, :103 and :104, and those for P-name, not for this split.
- L-C07, fake-player rows PG:140 to PG:333 under O-gamemem. I agree with O-gamemem for the typed events (they stand in for the in-game agent's messages). The player itself is fake code (a stand-in for `GamePlayer`), and no key names a fake player. C-session is a poor fit and I did not add it; see the side notes.
- L-C07, row :14 (PG:236). Changed, above.
- L-C07, MIDI:892. Changed, above.
- L-C07, SC rows :91, :95, :102. F-presslog is right: all three call `_presses`. The scout missed F-summary, because `_SCORE` is the hand-written score summary. Changed, above.

## Ends tables, after my changes

- L-C06: P-name is a new key in the table. Its plainest row is counts/T5-IR15.md:104, its least-fitting row counts/T5-IR15.md:67. P-setting's least-fitting row was :67, which no longer carries P-setting; the new least-fitting row is :103. K-unreachable gains :79 (plainest stays :61, least-fitting is :79). F-result's least-fitting row :66 stands, and :61 now carries it too.
- L-C07: P-tiers now has no row in this batch, so it leaves the table. O-process has no row either and leaves the table. P-other's least-fitting row was :80; it is now :96, which carries no X12 and is a different kind of band. F-summary gains :46 to :50 (plainest stays :45).

## Side notes

- test_play_in_game.py:72-98: `FakePlayer` is a hand-built stand-in for `kit_tools.GamePlayer`, with typed events fed to the play loop. None of the 53 keys names "a fake player". O-gamemem fits the events, and C-session, C-rig and C-worker each fit the player poorly. This affects ten rows (IR16:11 to :20) and is a gap in the key list; the scout's guess of O-gamemem is the best of the closest keys.
- counts/T5-IR15.md:28 (test_library_model.cpp:435-449): the made-up `KindedError(ChartUnreadable, "expected a [section] header")` is a hand-built error object, not a row. The scout's F-row passes the family rule, but F-result is the closer key. Left unchanged, since either satisfies the rule.
- counts/T5-IR15.md:55 (test_live_protocol.cpp:63-65): the counter marks X9 for the typed path "C:\\Program Files\\Hydra\\hydra.db", and the scout gave P-other. No F, O or C key fits a typed path handed to `is_under_temp`; the row carries an X9 mark with no key of its family, so a main-session ruling is needed on whether P-other stands.
- counts/T5-IR15.md:21 and :85, :86 (test_model.cpp:875-938): the backend leeway edges 2.999 and 3.0 and the labels "Normal" and "Hard (uncounted)" are typed. The scout keyed them P-tiers (squeeze timing tiers). They are the backend leeway (`Rules::backend_leeway_ms`), not a squeeze tier, so P-tiers stretches here; I left it, since the key list is closed and no better key exists (P-other is the alternative).
- counts/T5-IR16.md:21 and :22 (test_prep_inputs.py:88-92): `DEFAULT_CAP`/`DEFAULT_MS` and `REPLAY_CAP_S` are a tool's constants, not Hydra settings; P-setting is the scout's choice and P-other would fit as well. Left as the scout had it.
- Possible split: test_model.cpp:882-884 (and :928-929) types the backend leeway as 3.0 while the test at :876 reads it as `default_rules().backend_leeway_ms`. Both are in the same file, and the second asserts `lw == 3.0`. This is a typed constant in one file, not two owners answering differently.
- Not opened: the D12, B2 and C5 records themselves (the counter's reasons plus the tests settled every row I changed). The GF rows (:58 to :62), MIDI rows (:63, :76 to :80, :84, :86, :91, :92), PI:119, PC:84 and S2:47 were agreed from the counter's reason and the scout's note without opening their test lines.

## Not checked

Rows agreed from the counter's reason without reading the test: L-C06 :15 to :20, :22 to :25, :29 to :43, :45, :47 to :53, :56 to :60, :62 to :66, :68 to :78, :80 to :102 (apart from the ones named above); L-C07 :58 to :62 (GF), :63, :76 to :80, :84, :86, :91, :92 (test_generated_fills.cpp and most of test_midi.cpp), and :44 (test_s2_window_constants.py:47).
