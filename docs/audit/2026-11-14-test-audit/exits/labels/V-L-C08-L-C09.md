Agent id: ab178533fbfc4da86 (task T8-E3-V-L-C08-L-C09, session f3bbb32b)

## Counts

- L-C08: 126 rows in the batch, 126 lines in the label file (each batch row once, none extra, none doubled), 112 rows agreed, 14 rows changed.
- L-C09: 70 rows in the batch, 70 lines in the label file (each batch row once, none extra, none doubled), 65 rows agreed, 5 rows changed.

The ends tables of both files name only rows that are in their batch and carry the key. Two entries stop being true after my changes: L-C08 F-result "least-fitting" `counts/T5-IR17.md:96` (now `no key`), and L-C09 P-setting "least-fitting" `counts/T5-IR18.md:80` (now P-other, Z-cite). The main session should pick new ends for those two keys.

## What I opened

For L-C08 I opened `tools/ch_mcp/tests/test_game_files.py` lines 1 to 200 and 300 to 409 (the ini rows, the game-file rows and the FakeReg rows), `tools/ch_mcp/tests/test_frida_tools.py:758-782` (row :48), and `tests/test_song_input.cpp:53-81` (row :136). For L-C09 I opened `counts/T5-IR18.md:78-90` and grepped for WCAG in `tests/`. I did not open the other tests; those rows are agreed on the counter's reason alone, and the reasons name each fault plainly.

## The ini rows the scout doubted (`counts/T5-IR17.md:96` to `:105`)

I opened every one of the ten tests (`test_game_files.py:326-400`). In each, `G.set_game_setting`, `G.get_game_setting` or `G.song_ini_pro_drums` is the real code. It runs on a real temp file that the test writes with the ini text. The only thing the test made up is that ini text (`SETTINGS_FIXTURE`, a BOM-and-no-newline string, a `[song]` text). Nothing else is a stand-in: no fake reader, no fake clock, no stand-in for the game. `keys.md` (the opening of the X9 section, question 1) says a hand-written ini the real code reads is input, not a stand-in. So all ten rows are `no key`: the X9 mark describes hand-written input, which the key list does not count as a fault. (The ten tests do not name any other fault.) Row :104 also carries the counter's "split with song.cpp" remark; see Side notes.

## Rows I change

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-C08 | counts/T5-IR17.md:48 | O-gamemem | O-gamemem, P-message | The counter marked X10 and the test (`test_frida_tools.py:767-771`) requires typed strings such as `2, name "Pro Drums", id 9 (ProDrums)`, but the scout gave no key from the X10 family. |
| L-C08 | counts/T5-IR17.md:81 | F-result, P-name | P-name | The invented game files are hand-written input that the real `G.backup` reads from temp folders (no stand-in), and the typed folder name is what the key P-name covers. |
| L-C08 | counts/T5-IR17.md:82 | F-result | no key | `Folders` writes the game's files by hand as input, and `game_run()` is a few file writes that play the game's run; none of the keys fits that, and it is not a result, record or game memory. |
| L-C08 | counts/T5-IR17.md:83 | F-result, P-message | P-message | `_old_backup` writes an old-format backup folder and manifest by hand as input to the real `restore_all`, so there is no stand-in; only the typed "left as the game saved it" text is a fault. |
| L-C08 | counts/T5-IR17.md:96 | F-result | no key | `set_game_setting` runs for real on a temp file; only the ini text is typed, which `keys.md` question 1 calls input. |
| L-C08 | counts/T5-IR17.md:97 | F-result | no key | Same: hand-written ini text (BOM, no final newline) is input to the real function. |
| L-C08 | counts/T5-IR17.md:98 | F-result | no key | Same. |
| L-C08 | counts/T5-IR17.md:99 | F-result | no key | Same. |
| L-C08 | counts/T5-IR17.md:100 | F-result | no key | Same. |
| L-C08 | counts/T5-IR17.md:101 | F-result | no key | Same, for `get_game_setting`. |
| L-C08 | counts/T5-IR17.md:102 | F-result | no key | Same. |
| L-C08 | counts/T5-IR17.md:103 | F-result | no key | Same. |
| L-C08 | counts/T5-IR17.md:104 | F-result | no key | Hand-typed `song.ini` text is input to the real `song_ini_pro_drums`; the split with song.cpp is a side note, not a key. |
| L-C08 | counts/T5-IR17.md:105 | F-result | no key | Same, hand-typed `song.ini` text as input. |
| L-C09 | counts/T5-IR18.md:80 | P-setting, Z-cite | P-other, Z-cite | The typed 4.5 is the WCAG contrast limit, which is neither a settings default nor a screen word of Hydra's (the P-setting definition), so P-other fits; Z-cite stays for the WCAG source named in the header. |
| L-C09 | counts/T5-IR18.md:81 | P-setting, Z-cite | P-other, Z-cite | Same 4.5 and WCAG source as :80. |
| L-C09 | counts/T5-IR18.md:84 | P-setting, Z-cite | P-setting, P-other, Z-cite | `(0, 48, 48)` is a typed screen colour (P-setting), and the same record also types the 4.5 contrast limit, which is P-other. |
| L-C09 | counts/T5-IR18.md:86 | P-setting, Z-cite | P-other, Z-cite | 4.5 and the WCAG source, same as :80. |
| L-C09 | counts/T5-IR18.md:88 | P-setting, Z-cite | P-other, Z-cite | The 3.0 and 4.5 thresholds with the WCAG source, same as :80. |

(Note on the ten ini rows: for each, the counter's mark is X9 alone. After my change, the X9 mark has no key of its family on these rows; I judge that right, and it is the `no key` reason above, not an omission.)

On the six TH rows the user's doubt named (`:80`, `:81`, `:83`, `:84`, `:86`, `:88`): I move five; `:83` stays P-setting. `:83` types the alignment values 0.5 and 0.0, which are a screen setting Hydra's theme owns, and the cited "user's answer 6" is a Z-cite. Whether the whole group should be P-setting or P-other is the main session's call. The five I moved share one reason: the 4.5 and 3.0 limits are a standard's numbers, not values the code owns, and a comment cites the standard.

## Rows I checked and kept

- The C-clock and O-gamemem reading of the 65 Frida rows is right. `test_frida_tools.py` feeds a fake Frida module and a fake game, and rows :11 to :33 add a fake clock. The scout gave rows :34 to :75 O-gamemem without C-clock; the counters name no fake clock there, so I agree. "Fake Frida module" could also be read as C-stub; I left it as scouted.
- The FakeReg rows (:85 to :94) are C-stub, as scouted. `FakeReg` stands in for `reg.exe`, and the typed call list gives P-other.
- The rest of L-C09 (SNG, SL, SPS, SR, SRC, MC rows), and the report, search and song-input C++ rows of L-C08, fit their counters' reasons.

## Side notes

- `counts/T5-IR17.md:104` (`test_game_files.py:396`): the counter says song.ini's pro-drums read is also a split with song.cpp, two readers of one question. I did not open song.cpp; this is for the main session to chase as a finding.
- `tools/ch_mcp/tests/test_game_files.py:1-7`: the file header says "Every test runs on temp folders and a fake reg.exe". The fake reg.exe is real in the registry tests only; the ini rows use no fake.
- `tools/ch_mcp/tests/test_game_files.py:109`: a comment in `test_backup_copies_no_file` cites "The user's rulings of 2026-11-13" with no filed ruling named; that is a Z-cite in a test the counter marked X9, X10 only (row :81). I did not add Z-cite because the comment names a ruling date, not a value's source.
- `counts/T5-IR18.md:36` / `test_theme.cpp:2`: the WCAG comment cites a standard that is not code. The same question (R-cite) applies to every WCAG-cited test; `tests/test_report_page.cpp:287` and `tests/wcag_util.h` also cite it. The counter did not mark them; I did not label outside my batches.

## Not checked

No test beyond those listed under "What I opened". L-C09 rows were agreed from the counter's reasons; the tests were not opened, so a fault the counter missed is not caught here.
