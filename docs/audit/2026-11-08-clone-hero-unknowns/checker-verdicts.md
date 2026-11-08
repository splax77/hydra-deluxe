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
