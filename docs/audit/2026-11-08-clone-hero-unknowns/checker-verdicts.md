# Round 1 checker verdicts (for page executors and the user's list)

## W5 (checker a21d5e3e, on cb453227)
- NL-56 PASS for "every reader found by a direct-constant sweep is in the drum engine"; list open routes: register-held mask, flag word copied and masked later or elsewhere, movups copies, shifted base, reflection, native .text section, runtime patching. Sweep script not committed (commit it or word by routes).
- NL-58 divisor 192.0 PASS (lane_divisor_cctor_212E3D0 268-269). Cut-off compare f20d12c0 lines 145-147: lane skipped when chord +0x40 > lane start + cut-off (strict; at exactly start+cutoff kept). Chord = first chord with a non-kick pad from start-1 (0x5DE790 mask 0xFE), same chord Hydra's game_lane_pads uses. The cut-off also widens the lane: +0x18 from start-cutoff, length (chord-start)+2*cutoff (lines 242-262, 277-286). Round via 0x2C49A60 (looks like Math.Round). +0xB8 = resolution: W5b says shown from loaders (commit 69f8a067) -> verify. Hydra light_drum_lanes 188-203 no cut-off; comment 189-190 stale once ruled.
- NL-69 PASS; drum mirror pass skips a note whose lane word is exactly 0xF; RemoveAll predicate 0x6D5CB0 is n == null (read).
- NL-42 PASS (tables read from DLL). NL-48 PASS; remove W5's wrong caveat: btr edx,0x1f on 32-bit reg zero-extends -> game uses low 31 bits, same as part 2 and Hydra.
- NL-30 confirmed, no move.

## W6a (checker adebb6f0, on 0b353e59)
- FL-08 PASS game half; DIFFERS holds (no coda handling in src/). Skip byte = (fill start >= first coda tick) (fill_skip_from_coda_215DA70 lines 49-55, setge); landing step fill_to_activation_20CFF60 86-87 passes skipped fills. Evidence must add: .mid loads the "events" track (case-insensitive name) before other tracks, so coda is set before the skip runs; .chart order not read. Coda text is trimmed then compared ordinal case-sensitive ("[CODA]" does not count). Skipped fills stay in the list (FL-27/28 generation gate not traced).
- FL-10 PASS both halves; DIFFERS holds. Proven part: Hydra's deadline is later than the game's by the gap between the two fill starts (480 ticks in the example). Lead 4*resolution is shown (engine_ctor_20DF680 57, 68-69). Absolute 7680 vs 8160 rests on +0x108 being the current tick (FL-22 Unknown). Which field activation tests (note 0x800 or record byte) not settled. Assumes fills in start order.
- FL-29 not checked hard; nothing contradicts.

## W2 (checker a7574e79, on tip 65297ed3; 12 listings regenerated identical)
- CR-30 PASS (own recount: 2,718 Add pairs = TSV rows in order). Hydra table section_names_table.cpp has 2,718 rows, same key set; values not compared.
- CR-45 PASS narrow: runs when songTime >= songLength (gm_update_C05A0 178-180) -> 0xBD640 isSongOver; GlobalVariables +0x72 is isPracticeEnabled (dump.cs 55818): in practice mode 0xBF510 instead (unread). Do not say "the song ends" for every mode.
- CR-47 PASS worded: "longest loaded stem" (gate at audio_load lines 213-220 via 0x36DB60); encrypted branch 552-605 rejoins the same max step. Other writers of +0x40 not searched. Hydra half right; D255 item 1 "Option C (Recommended)".
- CR-35 REOPEN small: add evidence: 0x20F62C0 called from 0x20F36AB, 0x20F6633 (gated by SoloBegin test 0x214B630); 0x20F7610 from 0x20F36C9, 0x20F3A59, 0x20F6738 (SoloEnd test 0x214B7D0); +0x168 hit counter at 0x20F666E-685, +0x164 = 100*hit/total. Say "no other reader found by metadata-slot scan". Drum-only branch 0x20D1ED0 lines 185-194 omitted.
- CR-48 PASS offline: flag 1 when not Connected; build at coroutine 310-389, clock ctor, line 478. Side fact: song.ini writer 0x358850 writes song_length from SongEntry+0x80 or audioLength when missing.
- TM-10 PASS narrow; player-to-engine hop Unknown.
- TM-06 PASS narrow; Offset step for .sng and key comparer not proven.
- TM-08 PASS worded: "no caller found by direct call or jump, pointer-table / slot / address reference, vtable, delegate, string lookup, inlined copy; an unnamed reflection walk is not ruled out". Not a bare "no caller".

## W4 (checker a2541f35, on tip f3a1ce77)
- NL-54 PASS: "type 65 gives a single lane and type 66 a double lane"; drop "in a difficulty's section" (not shown). Hydra agrees: song.cpp 1451-1452, 1651-1653, 1725, op_lane 1403. Hydra-owned -> user list.
- TM-32 bit half PASS (menu 0x32D360 -> toggle 0x21236D0 -> profile +0x14 -> 11676A0 -> record +8 -> engine ctor +0x198; all three menu arrays hold 4096). Open: leaderboard filter in 32CE60; menu profile = player profile at run time not shown; "Precision Mode" text tied to 4096 by enum name only. Window half needs the live run.
- Q4 PASS for the switch (twelve options) and the loader 0x3968D0 (writes 1, 2 or replay +0x70). WRONG sentence: "no other -- option": `--lbsToken` exists, read by 0x395AA0 into PlayerPrefs sessionToken. Safe wording: "the option switch and the loader write modifiers only as None, DoubleKick, or the replay's own word".
- TM-16 narrowing PASS; say "the value the hit method stores" (+0xC4 other writers not searched).
- TM-36 REOPEN wording: drop "songTime every time", "press method reads no time" (it reads engine +0x100 at f20deaa0 135-138, 183-186), and "close to Proven". Non-direct time sites in BasePlayer.Update at 0x20A3BD, 0x20A425.
- TM-40 REOPEN: tempo stream only when arg != -1; song-load call 0x36B95A needs byte this+0x68 set (writers unsearched); case is "any speed incl. 100 once +0x68 set". 99% -> -1 reading holds. Side fact: tempo_create sets pitch 12*log2(speed/100) when a bool setting is on (lines 109-117).
- TM-43 REOPEN: drop "window is a span of chart time"; note-time scaling unread. Setter 0xFE5F50 PASS; song_speed default 100, limits 10-5000 (settings_load_374D50).
- TM-46 narrowing PASS.

## W3 (checker aaca0b0e, on tip db3f38fa)
- NL-12 PASS. Gap: Hydra's decode_chart_text (src/parse/text_decode.cpp 119-139) drops a leading BOM; game GetString does not (only matters for a text starting with a BOM). No full-game scan for other +0x20 writers.
- NL-13 PASS worded: "no direct or metadata route sorts or inserts"; interface-call route (IList Sort/Insert) not ruled out.
- NL-14 PASS worded: "a ghost kick hit softly enough (lane value below engine+0x2C0) is paid double by the same code as a ghost pad"; the 512-to-0x40000 copy step is unread; NL-15's live result covers only the accent bit; say "mask 4", not "bit 4".
- NL-36 PASS; "sustain cutoff" is the reading of the compare, not a code name; which settings field +0x24 is, and song.ini link, unread.
- NL-73 PASS: five-lane drums path only (enum 8309 FiveLaneDrums = 3). Not a DIFFERS: Hydra has no five-lane mode (question for user only if raised: model five-lane play?).
- NL-39 PASS marker route; block stays Unknown (flag 256 writer and copy to 0x20000 unread).
- NL-70 counts PASS (own recount and tick check). REOPEN the "game never opens expert+.mid / plays notes.mid" half: the `.mid` literal (stringliteral.json line 70855) reader unread; split counts (Proven) from opening claim (Unknown).

## W1 (checker a570de8e, on tip 15bee3c0)
- CR-03 PASS: header, song info + chart DEFLATE, image run skipped, stems. Streams 3-4 (images) DEFLATE stays Unknown.
- CR-05 PASS: decode(a,b) is a function of a xor b; "key"/"scrambled size" are role names. Count > file length returns quietly. No Hydra owner.
- CR-51 PASS narrowed: version check 0x1346234 and layout through audio entry. Keep Unknown: "offset" for first per-stem array, "indices" for flag-gated words, "2 album art / 1 background", stem-type names, "IV halves". DIFFERS holds: Hydra never checks the version (srb.h line 18 comment only).
- CR-24 PASS game half; wording "[end] calls the song's method 0x212E310 with the tick" (what it stores unread). NEW DIFFERS: Hydra owner is src/parse/section_names.cpp lines 10-21 (`is_section_marker`, `section_display_name`): accepts "section "/"prc_", no _ENDOFSONG veto; game vetoes a `[section` text containing _ENDOFSONG and tests `[section` with no space.
- CR-17 REOPEN on one phrase: light scan adds one zero-valued placeholder note per difficulty seen (mid_alt_path_2154180.asm lines 115-124, midi_sink_20CEC60.asm lines 31-41). Normal play load passes false; CliGameLauncher `--song` passes true at 0x397A79 to check instrument and difficulty. Hydra midi.cpp line uncited.
- CR-18 REOPEN: gate is `cmp byte [r14+0x58], 2`, enum #8326: 2 = Rhythm (Drums 6, ProDrums 9). Wording: "the song cache ORs the two flags with the song.ini force switches to decide a mask update; which entry it adds is unread".
- CR-12 REOPEN narrow: "SongEntry.checksum is the MD5 of the count bytes of the array 0x1313F0 (folder) or 0x262100 (.sng) returns; those callees and the cache writer are unread". 0x1313F0 uses ArrayPool + cache. Hydra "agrees" overreaches (.sng file pick vs chartName not compared).
- CR-07 PASS worded: "has the shape of AES-128 encryption (ten rounds, 16-byte block, table-driven); table contents not compared; CFB feedback and IV placement still Unknown".

# Round 2 second passes and round 3 checker verdicts (session 428cf079, 2026-11-11)

Nine fresh read-only checkers, each on a result it did not write. Every caller scan they relied on was rerun over both code sections (`il2cpp` and `.text`) with pointers over every section; the section that holds most game code is `il2cpp`, and a `.text`-only scan finds nothing. Branch tips: w6 3fa81d86, w7 703fa92e, w8 439f6107, w8b b64c0c0c, w9a 14a58c7b, w9b 67ac329e, w10a a0a850da, w10b b34ad182, w11a 6320bd9d, w11b 8dc18a3f.

## W6 second pass (on 3fa81d86)
- The track finish (f215c320, dedupe 0x5DB0C0, trim 0x5DB4C0, drop 0x5DB320): PASS; all five listings regenerated identical.
- SP-08 PASS: "the finish removes every phrase with no note starting in [start, start+length-1] ([start, start] at length 0); it runs after the overlap trim". The page's "so the step changes no score" holds only for phrases that do not overlap. A zero-length phrase inside a longer one is stretched by the trim and becomes a real phrase.
- SP-06 PASS narrowed: phrases are appended in read order, not sorted; dedupe (same start: keep the longest, an equal length keeps the earlier) and the trim (a phrase starting inside an earlier one cuts it at its start and runs to the later end) reshape the list, so nested and overlapping phrases award. "May never award" is wrong. Fix "every phrase that survives can award": a zero-length phrase with a note at its start survives the drop and holds no note (sp_phrase_assign_20D2440 tests start <= tick < end). Open: .mid note-116 pairing, and .chart phrase lines out of tick order.
- SP-07 PASS: a .chart phrase and a .mid drum phrase are the same #8252 record and go through the same finish and assignment 0x20D2440; the .mid reader copies Expert's list into the other difficulties by AddRange.
- FL-22 PASS: +0x108 is the chart tick of the engine's current time (the long half of the pair at +0x100); whole milliseconds is refuted. Initial pair is (-2.0 s, tick 0).
- FL-32 stays Unknown. Lead: the 0x800 fill flag is read by note accessor 0x214BB50, called from the DrumNoteSprites helper struct #1138 (0x2B09A0 and four more) and six #8255 methods.
- DIFFERS (SP-06/SP-08, phrase overlap): game trims and stretches (track_trim_overlaps_5DB4C0.asm 0x5db64b-0x5db684) and dedupes on one start (track_dedupe_5DB0C0.asm 0x5db1d4-0x5db1d8); Hydra keeps one open phrase and a new start replaces it (src/parse/song.cpp `op_sp_start`, lines 1457-1460 for .chart, 830 for .mid; `close_sp_phrase` line 429). Example, notes 10, 30, 50: `S 2 0 100` then `S 2 20 20` awards 10 and 50 in the game, 30 only in Hydra; `S 2 0 100` then `S 2 0 40` awards 50 in the game, 30 in Hydra. Real-chart effect not measured.

## W7 (on 703fa92e)
- SP-15 PASS narrowed: one timing segment per tempo or time-signature change; a measure of any signature counts 3.75 x scale units, so the drain follows the time signature, as Hydra's rule does in kind. "Scale" is chart +0xB0, which is not read; only +0xB8 = 192 was seen live.
- SP-29 REOPEN: the call order holds, but the consequence does not. Slot 8 (advance_events_20F6A50.asm) queues the SP end when old clock < end < frame clock, and the step runs a nested frame update at that time with hits off (event_catch_up_20F6FF0.asm, call at 0x20F719A), whose end check turns SP off before the real frame's hits. Hits come first only when the frame clock equals the end. Q-50's example (end 10,000, hit judged at 10,011.7) pays single, not doubled; Q-50 and page 05 SP-28 must be rewritten before use. Possible DIFFERS: the game judges at the frame clock, Hydra pays by chart time against the end (src/core/backend_value.h line 28); not quantified.
- SP-30 REOPEN in part: slot 21 judges at the frame clock; slot 18 in a nested update judges at that update's event time. Whether a real press uses slot 18 is unread.
- SP-35 PASS as written; the gain is skipped for Bot and RemotePlayer kinds.
- SP-43 PASS narrowed: no 0.003, 3.0 as a time, 3000.0 or 3000 immediate in #8253, #8255, #8288 or #8544; the 13 integer 3s are counts, caps, bit tests and interface numbers. The mov r9d,3 at lane_flag_calls_20DF4C0 line 53 is "an argument to an unread helper".
- IN-88 PASS narrowed: +0x1F8 items are #8323 fill windows (time, seconds, tick, ticks), one per fill whose +0x20 byte is non-zero; the builder's result is traced into the struct constructor's fifth argument.
- IN-90: claims PASS, block stays Unknown; +0x300 = 4 x chart +0xB8 goes in as a Proven sentence. The tick conversion reads +0xB0, and +0xB0 = +0xB8 is unshown.
- FL-23 PASS game side. DIFFERS at the edge: the game clears a fill when tick(clock) reaches the deadline tick (fill_deadline_20DDB70.asm `jg` at 0x20DDC24, so equal is cleared); Hydra keeps it until ready time passes deadline + 85 ms and keeps it at exactly 85 (src/core/model.h line 318, pinned by tests/test_model.cpp lines 191-192 and tests/test_fills_and_skips.cpp edge 7285). The "85 ms both sides" ruling (rulings/clone-hero.md line 22) is about the fill-comparison tool and does not name this edge. Real-chart effect not measured.

## W8 (on 439f6107)
- SC-29 PASS: stars count the score before the solo bonus (engine +0xA0) and before engine +0xA4 (clean_play_bonus). Possible DIFFERS: +0xA4 has no traced writer, was 0 in every saved file, and Hydra has no such bonus.
- SC-09 PASS narrowed: "squeezed" is read from the counters' names, not the code.
- SC-23: the message ladder and choke override PASS (100 Perfect, >=95 Awesome, >=90 Great, >=80 Good, >=70 Solid, ==69 nice, >=60 Okay, else Messy with the bonus zeroed; the flag gives Awesome Choke!). REOPEN for a whole-block Proven label: percent formula, start rule, choke trigger and timing are unread.
- SC-26 PASS. SC-37 PASS narrowed: a dozen keys (instrument, difficulty, profile_name, modifiers, is_bot and others) stay untraced.

## W8c (on b64c0c0c) and W10a (on a0a850da)
- IN-44 PASS for "both files made at start-up; each sink keeps only its tag; no setting read". REOPEN for "the shipped build never makes a tagged entry because [Conditional] drops the calls": the generic Log instances have compiled bodies (0x604210), and a rel32 scan cannot see generic-instance calls. Keep "no direct caller found".
- IN-55 PASS, including the merge: the larger +4 int wins and brings its other fields; a tie takes the new run.
- IN-45 PASS narrowed: the literal is `combo: {0}` plus a newline; 776 is the `\Assets` part of 1,346 paths.
- IN-92 PASS. Extra: this machine's registry key holds Rewired's controller maps, and settings_schema.json gives poll_rate default 656, min 60, max 1000; the default is clamped into 60 to 1000.
- IN-89 PASS split: the bit each getter tests is Proven in the game's code; the names from dump.cs take question 1's label; the three +0x80 getters stay Unknown.

## W9a (on 14a58c7b) and W9b (on 67ac329e)
- Highlighted row CONFIRMED: the game reads the cursor alone on Confirm (0x32E3F7) and Difficulty (0x32E2C0), cursor plus scroll on Instrument and Modifiers. tools/ch_mcp/frida_tools.py line 232 always adds the scroll. They agree when scroll is 0; whether it can be non-zero there is unread. Tools only, no Hydra owner.
- IN-29 PASS narrowed (store before any use; whole-song hold Unknown). IN-39 PASS once worded: a player must pass 0x337540 as well as the is_bot test; "two writers were found", not "the only two". A third store in EndOfSong 0x161C70 seems to write 0 when a bot is present (unproven).
- IN-91 PASS. IN-59 PASS narrowed ("not on cursor moves"; the fields change on a confirm).
- IN-60 PASS with W9b's three corrections.
- IN-30 stays Unknown; REOPEN the sentence "Green does not reach SongSelect": a press reaches only the base slot 31, but SongSelect overrides slot 57 (Green held), 36, 10 and 21. Lead: SongOptions 0x297240's Green handler calls the song-load method.
- IN-73 stays Unknown with the widened Evidence; see the slot-map note below.

## W10b (on b34ad182) and W11a (on 6320bd9d)
- IN-40 PASS (1.0 bot earns no Star Power); add that BasePlayer.cs 1108 sets isBot to 0 in network play. IN-47 PASS (counts reproduce exactly). SC-11 stays Unknown (no public source). SP-46 PASS for the code half (SPBar exists and is fed); the screen half stays Unknown.
- SP-14 PASS narrowed: "20 of 62" (not 63); "fits the drain", not "because the drain has started". run1_cal0_sp_compare.txt line 20 puts chord 162 at 30,341.1 ms; the 2.9 ms end gap fits inside the 6 ms sample gap but stays Unknown.
- SP-18 PASS only as "in these 8 runs, 4,207 hits with at least half a bar banked never turned SP on"; the button half stays Unknown.
- IN-14 stays Unknown. IN-43 PASS narrowed to "did not appear in 300 s in that run".

## W11b (on 8dc18a3f) and IN-42
- IN-71 PASS narrowed to the save method 0x2112DB0; the block stays Unknown (five other ini writers unread; the runs cannot tell "skipped" from "nothing dirty").
- CR-49 PASS: Pressure states 273,467 ms, all five stems end at 223.130 s. Possible DIFFERS: Hydra ends a bundle at its stated length (src/parse/srb.cpp line 180, src/app/song_length.cpp lines 37-55); the game's end is Min(end event, Max(audio, last note end)) + 1 s (page 01 CR-46), but whether the game uses that for a bundle, and Pressure's last note, are unread.
- IN-50 PASS; add that names are scrambled per build.
- IN-42 (main session, no result file) PASS: Trippin SP Spike's [ExpertDrums] has 402 N 0 and no N 32, and Hydra's replay output counts 402 Kick; a 2x kick would also show as Kick, so the chart is the proof. Stays Unknown.

## Rounds 1 and 2 rescan, and the string slot map
- 45 caller claims rerun over il2cpp and .text; all hold except three. W1 CR-12: 0x20FA8F0 also has three tail-jump callers (0x282C04, 0x285783, 0x2EFD42), so song_hash_md5_20FA8F0.md line 3 needs them. W2 CR-35: the solo-slot hits also include 0x215E6E0, 0x215C320 and 0x20E8B60, unread. W5 NL-57: 0x41BE70 has 10 sites in 5 methods, not 3 methods.
- The str: map is right against the file: all 23,972 slots decode, as the game's own init helper does, to the text chdis shows. The 0x1F53B0 NameToID loads of two non-layer strings stay unexplained (a live read of the two slots would settle it). A str: label in a method's opening init block does not show the method uses the string.
