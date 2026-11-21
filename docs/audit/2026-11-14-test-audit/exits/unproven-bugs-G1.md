G1 result: none of the four items shows a bug that can be fixed without a decision. Item 33 shows a real Hydra-versus-game difference, but it is already filed as open question Q-74 and waits for the user.

Item 31, running status after sysex: DOES NOT SHOW against the source the file cites.
- src/parse/midi.cpp lines 219-225: any byte with the high bit set is stored as the running status, except 0xFF. The comment says "A meta event must not become the running status (mido excludes only 0xFF)". So after 0xF0 or 0xF7, running = 0xF0 or 0xF7.
- Lines 226-228: a following data byte reuses that status. Lines 245-249 then read it as another sysex (length, skip). The bytes after a sysex are therefore not read as note data.
- The file's own cited source is mido (midi.cpp:222, and midi.h:50-88 "as mido"). I did not open mido's source. From memory, its track reader also sets last_status = status for everything except 0xFF, so it does the same. This is a claim from memory, not a check.
- The SMF spec says a sysex cancels running status. midi.cpp does not cite the spec. The two only differ on a malformed file with a data byte straight after a sysex. No real chart is shown to hit that.
- tests/test_midi.cpp lines 261-272 cover sysex time but not running status after it.
- No bug task. If the user wants spec behaviour instead of mido behaviour, that is a ruling for them to make.

Item 32, search highlight on "ß" folding to "ss": DOES NOT SHOW.
- src/app/library_query.cpp line 95: the Latin fold table maps ß (0xDF) to "ss". Lines 193-199: fold_into emits those two bytes with the source range of the whole two-byte ß.
- Lines 137-141: emit gives each output byte a SourceRange of the whole character.
- match_spans (lines 478-493) searches the folded text and maps the span back through source[at].begin and source[at+len-1].end.
- On a title like "Straße": query "ss" or "strasse" highlights the whole ß. A one-letter query "s" also highlights the whole ß. A match starting or ending inside the fold never splits the character, never reads out of range, and never crashes.
- The only oddity is that "s" over-highlights the whole ß. Lines 121-122 state that behaviour as deliberate ("the whole character ... that produced it").

Item 33, whether Clone Hero counts a vocals track's low notes toward the song end: the pages settle it, and Hydra differs.
- I read docs/clone-hero/README.md first.
- docs/clone-hero/01-reading-a-chart.md lines 463-471 (CR-44) are labelled "Proven in the game's code". They say: "a vocal note never counts toward the last note end. Hydra counts a vocals track's low notes."
- The evidence is decompile lines 43955-44246, function FUN_1821579b0 at RVA 0x21579B0. It writes only the lyric list and never calls the gem adder 0x20CEC60.
- The page admits that two callees were not read and that the .chart vocals and binary loaders were not checked. No live run was done; live-checks.md line 39 lists CR-44 as a check still to do.
- Hydra: MidiFile::last_note_end_tick in src/parse/midi.cpp lines 468-470 counts vocals low pitches (equals_ci(track.name, kVocalsTrackName) with last_low_note_tick).
- docs/clone-hero/questions.md Q-74 (lines 175-182) records this as an open split with no ruling. It notes D255 item 3 ruled "All instruments" before the vocals method was read, and that ruling does not say whether vocals count as an instrument.
- So this shows a difference, but it goes to the user, not to a bug task. Charts with vocals running past the last instrument note, no end event and short audio get a different song length in Hydra than in the game.
- If the user rules for the game's behaviour, the task would be: remove the vocals branch at midi.cpp:468-470 and the last_low_note_tick bookkeeping at lines 348 and 526. The test would be a .mid whose vocals track runs past the last gem, expecting last_note_end_tick to follow the gems.

Item 34, hooks repo: DOES NOT SHOW, on both halves.
(a) The key lock and route "Keys":
- C:\Users\Patrick\.claude\gates\percall.py line 18 sets KEY_ROUTE_TOOLS = {"mcp__ch__ch_play_chart": "keys"}. Line 80 is `if route == KEY_ROUTE_TOOLS[tool]`, an exact, case-sensitive match. So "Keys" returns None and the lock does not fire.
- gates/tests/test_percall_key.py lines 95-98 pin exactly that: for route in ("", "Keys", "kit") the result is assertIsNone.
- This is not a bypass, because the server refuses it. tools/ch_mcp/game_tools.py lines 70-72 define ROUTES = ("kit", "keys"), and lines 1104-1105 return 'unknown route "Keys"' before any key is pressed. I found no lower() applied to route; the grep hits were for lane names and menu keys only.
- So "Keys" never presses keys. The lock only has to match what the server accepts, and it does.
(b) prepass.py and 'typed':
- C:\Users\Patrick\.claude\tools\session_cards\prepass.py lines 219-225 pick human messages by common.classify_user. common.py lines 404-406 decide that from origin.kind == "human", not from promptSource.
- "typed" at prepass.py lines 223 and 225 is only the source label the tool writes. It is never required of the input.
- A grep of promptSource under .claude found it only in test fixtures. Fixtures with "promptSource":"sdk" and origin.kind "human" are used in tools/session_cards/test_session_cards.py:235 and test_reclass_clicks.py:70.
- So lines with sdk are accepted.

No files changed in any repo.
