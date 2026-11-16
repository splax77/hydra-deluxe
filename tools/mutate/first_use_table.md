# F1: the 62 disputed rows against the full mutation run 20261116-123523

The tool plants its own kinds of bug. It does not plant the change a row names. So a row whose test caught nothing is "not shown to fail by this run". That is all it says. No row holds an agent's verdict.

One re-read row can name several lines, so the table has one line per named line (75 lines from 61 rows). The 62nd row names no line and is listed at the end.

"Bugs it reaches" is how many planted bugs sit in code that test runs. "Bugs it caught" is how many bugs made the test fail, the same count as the run's tests.txt. It counts a bug the test reaches, and also a bug the test does not reach but an earlier test in the same run did, so the bug could reach it through state the run keeps between tests. The part in brackets is how many of the caught bugs were of that second kind. "This line failed in" counts the bugs where the check on that exact line was one of the failed checks. A line with no check of its own (a comment, a closing brace, or a message string inside a check) can never match, so it reads none.

"Outside every test case" means the line sits in a helper function, not in a test. No test is named for it. The table still says whether a check on that line failed.

| Slice | Re-read line | Test line | Test | Bugs it reaches | Bugs it caught | This line failed in |
|---|---|---|---|---|---|---|
| A3 | 10 | tests/test_activation_rows.cpp:850 | activation badges: an early fill with time to spare shows, grey (D261) | 12 | 6 | none |
| A3 | 11 | tests/test_activation_rows.cpp:851 | activation badges: an early fill with time to spare shows, grey (D261) | 12 | 6 | 4 bugs (1120, 1122, 1123, 1134) |
| A3 | 12 | tests/test_activation_rows.cpp:866 | activation badges: the row is difficult exactly when a badge is orange | 48 | 11 | 11 bugs (1118, 1120, 1121, 1127, 1129, 1133, 1134, 1135, ...) |
| A3 | 29 | tests/test_activation_rows.cpp:942 | longest_activation_badge: the widest wording with the window's figure | 7 | 0: not shown to fail by this run | none |
| A3 | 60 | tests/test_activation_rows.cpp:1319 | dynamic hit window: Tapped Out's squeeze out is out of reach | 725 | 173 (13 through shared state) | 1 bugs (1631) |
| A3 | 60 | tests/test_activation_rows.cpp:1320 | dynamic hit window: Tapped Out's squeeze out is out of reach | 725 | 173 (13 through shared state) | none |
| A3 | 65 | tests/test_activation_rows.cpp:1346 | dynamic hit window: Burnout's squeeze out has 5.4 ms safe | 103 | 160 (61 through shared state) | none |
| A3 | 66 | tests/test_activation_rows.cpp:1347 | dynamic hit window: Burnout's squeeze out has 5.4 ms safe | 103 | 160 (61 through shared state) | 1 bugs (361) |
| A3 | 74 | tests/test_activation_rows.cpp:1423 | dynamic hit window: an early fill gets one window, required or optional | 109 | 159 (71 through shared state) | 1 bugs (363) |
| A9 | 14 | tests/test_config.cpp:56 | outside every test case | | | 1 bugs (1357) |
| A9 | 23 | tests/test_config.cpp:175 | the 1.0 fills setting reaches the search and the result's key together | 10 | 0: not shown to fail by this run | none |
| A9 | 24 | tests/test_config.cpp:176 | the 1.0 fills setting reaches the search and the result's key together | 10 | 0: not shown to fail by this run | none |
| A9 | 37 | tests/test_config.cpp:310 | config: the default depth is the search's, 4 | 0 | 0: not shown to fail by this run | none |
| A9 | 37 | tests/test_config.cpp:311 | config: the default depth is the search's, 4 | 0 | 0: not shown to fail by this run | none |
| A9 | 38 | tests/test_config.cpp:321 | blank and comment lines load, and keys and values are trimmed | 25 | 11 | 2 bugs (99, 101) |
| A9 | 40 | tests/test_config.cpp:325 | blank and comment lines load, and keys and values are trimmed | 25 | 11 | 1 bugs (101) |
| A9 | 85 | tests/test_config.cpp:675 | profile moves with every key part | 10 | 2 | 1 bugs (1357) |
| A9 | 85 | tests/test_config.cpp:676 | profile moves with every key part | 10 | 2 | none |
| A9 | 85 | tests/test_config.cpp:677 | profile moves with every key part | 10 | 2 | 1 bugs (108) |
| A9 | 98 | tests/test_config.cpp:791 | batch_run bundles one Settings' profile and search settings | 11 | 4 | none |
| A9 | 98 | tests/test_config.cpp:793 | batch_run bundles one Settings' profile and search settings | 11 | 4 | none |
| A30 | 25 | tests/test_squeeze_rating.cpp:234 | transfer_scale_between: a measure that is not a positive finite length is refused | 13 | 0: not shown to fail by this run | none |
| A30 | 26 | tests/test_squeeze_rating.cpp:237 | transfer_scale_between: a measure that is not a positive finite length is refused | 13 | 0: not shown to fail by this run | none |
| A30 | 33 | tests/test_squeeze_rating.cpp:337 | field fixture: What's My Age Again? (Sync Chart) SqOut | 42 | 5 | none |
| A30 | 33 | tests/test_squeeze_rating.cpp:338 | field fixture: What's My Age Again? (Sync Chart) SqOut | 42 | 5 | none |
| A30 | 44 | tests/test_squeeze_rating.cpp:428 | field fixture: Dumpweed SqOut end anchored on the deact node | 25 | 0: not shown to fail by this run | none |
| A30 | 44 | tests/test_squeeze_rating.cpp:429 | field fixture: Dumpweed SqOut end anchored on the deact node | 25 | 0: not shown to fail by this run | none |
| A30 | 48 | tests/test_squeeze_rating.cpp:463 | is_scaled: within a thousandth of 1 is no scale (D256) | 0 | 0: not shown to fail by this run | none |
| A30 | 48 | tests/test_squeeze_rating.cpp:464 | is_scaled: within a thousandth of 1 is no scale (D256) | 0 | 0: not shown to fail by this run | none |
| A30 | 48 | tests/test_squeeze_rating.cpp:465 | is_scaled: within a thousandth of 1 is no scale (D256) | 0 | 0: not shown to fail by this run | none |
| A30 | 48 | tests/test_squeeze_rating.cpp:466 | is_scaled: within a thousandth of 1 is no scale (D256) | 0 | 0: not shown to fail by this run | none |
| A30 | 58 | tests/test_squeeze_rating.cpp:542 | rate_activation: every row reads post, by its side of the end | 7 | 5 | 2 bugs (1184, 1325) |
| A30 | 92 | tests/test_squeeze_rating.cpp:912 | rate_activation: a plain row inside the leeway is not a frontend squeeze | 17 | 9 | 4 bugs (1184, 1336, 1337, 1342) |
| A30 | 98 | tests/test_squeeze_rating.cpp:961 | BackendSqueeze::summarystr: a scaled row's word follows its eff. figure | 16 | 9 | 8 bugs (1103, 1104, 1184, 1185, 1186, 1187, 1343, 1344) |
| A30 | 103 | tests/test_squeeze_rating.cpp:989 | rate_activation: an unknown scale rates nothing | 9 | 2 | 1 bugs (1338) |
| A30 | 103 | tests/test_squeeze_rating.cpp:991 | rate_activation: an unknown scale rates nothing | 9 | 2 | none |
| A30 | 104 | tests/test_squeeze_rating.cpp:993 | rate_activation: an unknown scale rates nothing | 9 | 2 | none |
| C7 | 18 | tests/test_pb_coach.cpp:118 | coach click: a refusal is current only while the player's path stays as it was | 735 | 69 | none |
| C7 | 39 | tests/test_pb_coach.cpp:286 | coach end: the kept end is the window's own end | 760 | 123 | none |
| C7 | 66 | tests/test_pb_files.cpp:65 | pb files: a saved path file opens back as the same path, as the player's | 788 | 125 | none |
| C7 | 66 | tests/test_pb_files.cpp:66 | pb files: a saved path file opens back as the same path, as the player's | 788 | 125 | 12 bugs (1047, 1049, 1051, 1053, 1055, 1056, 1057, 1059, ...) |
| C7 | 92 | tests/test_phrase_end.cpp:87 | phrase end: a phrase ending on or between chords flags the last chord inside | 290 | 68 | 43 bugs (1284, 1286, 1288, 1758, 1759, 1760, 1768, 1770, ...) |
| C7 | 93 | tests/test_phrase_end.cpp:88 | phrase end: a phrase ending on or between chords flags the last chord inside | 290 | 68 | 29 bugs (1284, 1286, 1288, 1586, 1589, 1592, 1594, 1595, ...) |
| C7 | 95 | tests/test_phrase_end.cpp:96 | phrase end: a phrase running past the last note flags that note | 290 | 67 | 28 bugs (1284, 1286, 1288, 1586, 1589, 1592, 1594, 1595, ...) |
| C7 | 99 | tests/test_phrase_end.cpp:121 | phrase end: a zero-length phrase on a chord flags nothing | 290 | 54 | 36 bugs (1284, 1286, 1288, 1758, 1760, 1768, 1770, 1771, ...) |
| C7 | 100 | tests/test_phrase_end.cpp:122 | phrase end: a zero-length phrase on a chord flags nothing | 290 | 54 | 22 bugs (1284, 1286, 1288, 1586, 1589, 1592, 1594, 1595, ...) |
| WN3 | 8 | tests/test_engine_sweeps.cpp:174 | the engine's limit walk agrees with Activation::within_ms_limit | 8 | 150 (128 through shared state) | none |
| WN3 | 9 | tests/test_engine_sweeps.cpp:189 | the engine's limit walk agrees with Activation::within_ms_limit | 8 | 150 (128 through shared state) | none |
| WN3 | 19 | tests/test_fills_and_skips.cpp:305 | Skipped fills: every corpus root passes over real fills in order | 11 | 44 (40 through shared state) | 1 bugs (1351) |
| WN3 | 39 | tests/test_path_costs.cpp:155 | cost list: no cost is below zero | 359 | 82 | 3 bugs (1138, 1139, 3080) |
| WN4a | 6 | tests/test_replay.cpp:142 | replay reproduces the engine's score for every corpus path | 961 | 175 | 44 bugs (1298, 1299, 1301, 1473, 1475, 1480, 1483, 1484, ...) |
| WN4a | 7 | tests/test_replay.cpp:141 | replay reproduces the engine's score for every corpus path | 961 | 175 | none |
| WN4a | 8 | tests/test_replay.cpp:253 | targeted search reproduces every corpus path | 366 | 163 (92 through shared state) | 11 bugs (2800, 2801, 2837, 2943, 2944, 2945, 3001, 3061, ...) |
| WN4a | 9 | tests/test_replay.cpp:251 | targeted search reproduces every corpus path | 366 | 163 (92 through shared state) | 4 bugs (2917, 2927, 2932, 2935) |
| WN4a | 38 | tests/test_preview_controller.cpp:266 | the Preview overlay job builds the highway timeline on its worker | 812 | 53 (6 through shared state) | 1 bugs (4324) |
| WN4a | 38 | tests/test_preview_controller.cpp:267 | the Preview overlay job builds the highway timeline on its worker | 812 | 53 (6 through shared state) | 1 bugs (4324) |
| WN4a | 38 | tests/test_preview_controller.cpp:268 | the Preview overlay job builds the highway timeline on its worker | 812 | 53 (6 through shared state) | none |
| WN4b | 8 | tests/test_pb_coach.cpp:69 | coach rows: a two-click path fills the ladder, the rows, the ends and Finish the rest for me | 760 | 151 | none |
| WN4b | 9 | tests/test_pb_coach.cpp:95 | coach rows: a two-click path fills the ladder, the rows, the ends and Finish the rest for me | 760 | 151 | none |
| WN4b | 12 | tests/test_pb_coach.cpp:145 | coach rows: a window over the Path limit carries the tight-window note | 778 | 127 | 3 bugs (1138, 1139, 3080) |
| WN4b | 24 | tests/test_pb_coach.cpp:346 | coach end: an earlier window's endings are the same list | 794 | 146 | none |
| WN4b | 25 | tests/test_pb_coach.cpp:361 | coach end: an earlier window's endings are the same list | 794 | 146 | none |
| WN4b | 34 | tests/test_pb_coach.cpp:614 | candidate costs: the best next fill costs 0 and every other cost is BF(n) minus its best finish | 782 | 157 | 5 bugs (335, 1041, 1587, 1881, 2896) |
| WN4b | 45 | tests/test_pb_text.cpp:126 | choice order: cheapest first, ties in chart order, pending last | 238 | 16 | 4 bugs (2396, 2397, 2398, 2399) |
| WN4b | 46 | tests/test_pb_text.cpp:139 | step short: start, end and phrases | 111 | 35 (30 through shared state) | 1 bugs (1165) |
| WN4b | 47 | tests/test_pb_text.cpp:142 | step short: start, end and phrases | 111 | 35 (30 through shared state) | 1 bugs (1165) |
| WN4b | 51 | tests/test_pb_text.cpp:196 | end choice text: the end measure and the Paths tab's badge words | 111 | 39 (31 through shared state) | 2 bugs (352, 1119) |
| WN4b | 56 | tests/test_pb_text.cpp:310 | fill card: an offered fill reads the heading, three rows and two grey lines | 407 | 61 | 20 bugs (486, 487, 490, 499, 503, 1165, 1201, 1202, ...) |
| WN4b | 62 | tests/test_saved_figure_fill.cpp:118 | a saved path with a squeeze-in on the SP end gets its figure back | 817 | 163 | 1 bugs (642) |
| WN4b | 69 | tests/test_saved_figure_fill.cpp:208 | a chart whose replay throws keeps its rows empty, and the fill goes on | 959 | 184 (1 through shared state) | 66 bugs (616, 618, 620, 622, 623, 624, 628, 632, ...) |
| WN4b | 72 | tests/test_saved_figure_fill.cpp:328 | the fill hands every row of one chart the same graph | 814 | 154 | none |
| WN4b | 73 | tests/test_saved_figure_fill.cpp:329 | the fill hands every row of one chart the same graph | 814 | 154 | none |
| WN4b | 77 | tests/test_saved_figure_fill.cpp:398 | a row under an older stamp is left alone by the fill | 138 | 93 (70 through shared state) | none |
| WN8 | 10 | tests/test_dm_report.cpp:85 | collect_dm_rows joins scores to records and labels them | 138 | 24 | 4 bugs (3230, 3231, 3306, 3309) |
| WN8 | 12 | tests/test_report_rows.cpp:419 | report counts every library copy of a chart, and leaves out a chart the library dropped | 914 | 39 | 9 bugs (1724, 1877, 1908, 1912, 1913, 1914, 1922, 2839, ...) |

Rows the copy could not place on a line, as the copy gave them:

- WN3	10	the line cell is the sentence "at 60 fewer fills are E0 than at 85, so model_rejected can fall" and names no line number
