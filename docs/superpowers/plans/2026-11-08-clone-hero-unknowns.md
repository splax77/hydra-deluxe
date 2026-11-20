# Checking every Unknown block of the Clone Hero reference

The user asked: "every claim labeled unknown should be checked". Asked whether the 30 "Hydra assumes it, never checked" blocks come too, they chose "Unknown only". So this plan covers exactly the blocks whose Label: line reads Unknown on the seven area pages under `docs/clone-hero/`. There are 96 today, counted from the pages on 2026-11-08 (page 1 has 15, page 2 has 21, page 3 has 10, page 4 has 7, page 5 has 11, page 6 has 6, page 7 has 26; the README's table says the same). Nothing is added and nothing is dropped. A block this plan thinks cannot be checked stays in the table below with its reason and goes to the user as a question.

The 96 ids: CR-03, CR-05, CR-07, CR-12, CR-17, CR-18, CR-24, CR-29, CR-30, CR-35, CR-45, CR-47, CR-48, CR-49, CR-51; NL-12, NL-13, NL-14, NL-23, NL-28, NL-30, NL-31, NL-36, NL-39, NL-42, NL-48, NL-52, NL-54, NL-56, NL-57, NL-58, NL-65, NL-69, NL-70, NL-71, NL-73; TM-06, TM-08, TM-10, TM-16, TM-32, TM-36, TM-37, TM-40, TM-43, TM-46; FL-08, FL-10, FL-22, FL-23, FL-29, FL-32, FL-45; SP-06, SP-07, SP-08, SP-14, SP-15, SP-18, SP-29, SP-30, SP-35, SP-43, SP-46; SC-09, SC-11, SC-23, SC-26, SC-29, SC-37; IN-14, IN-19, IN-20, IN-24, IN-29, IN-30, IN-39, IN-40, IN-41, IN-42, IN-43, IN-44, IN-45, IN-47, IN-50, IN-55, IN-59, IN-60, IN-71, IN-73, IN-80, IN-88, IN-89, IN-90, IN-91, IN-92.

Every row below was written from the block's own Rule, Evidence, Game code and Hydra lines on its page, not from a summary of it. The plan does not treat any block's text as a fact: the block says what is unknown and where people looked, and the row says how to look further.

## How a check ends

A check has one of three results, and each result moves the label in one way.

If a listing of the game's code shows the rule, the block becomes "Proven in the game's code", and its Evidence line names the new listing under `docs/clone-hero/code/6142/` (or `code/4045/` for build 1.0) with its lines. If a run of the game shows the rule, the block becomes "Proven in a live run", and the Evidence line names the run folder under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\` and its result file. If the check finds nothing, the block stays Unknown, and its Evidence line grows by exactly what was searched and where, so the next reader does not search the same place twice.

Two kinds of result need the user before any page changes. First, a result that shows the game differs from what Hydra does becomes a question on `docs/clone-hero/questions.md` with a new Q-id, and the block's Ruling line points at it. Nobody changes Hydra's rule from this plan; a change is a question for the user, asked whole, with both sides and an example from each end (agent rule 12, and "How to write a question" in `docs/agents/audit-method.md`). Second, a result that moves a block from Unknown to Proven while a Hydra rule reads the block (its Hydra line names an owner) goes to the user as a list of label moves before the page executor writes it, so the user sees which rules now rest on proof. Label moves on blocks whose Hydra line says "no owner" are written straight after the verifier passes them.

A fact read from the game's files or dumps, but not from a listing of its code and not from a run (such as a count over `dump.cs`, or the measured length of a bundle's audio), fits none of the four labels well. Question 1 asks the user which label such a fact takes; until it is answered, those blocks keep Unknown with the result file cited in their Evidence line.

## Routes

Each row names one of five routes, and the cheapest route that can actually settle the block is the one named. Where code alone cannot settle a block, the row says so and names the run.

- (a) Read a listing that is already in the repo, under `docs/audit/` or `docs/clone-hero/code/6142/`, or run a script that is already committed and keep its output.
- (b) Make a new listing from the game's `GameAssembly.dll` with `tools/ch_mcp/chdis.py` (it takes an RVA and the build's dump folder), find callers and readers with the `rea` MCP server's cross-reference tools, and read `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs` for class and field names, with Ghidra's decompile at `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\ghidra\ghidra_decompile_all.txt` for a function's shape. Every new listing is committed under `docs/clone-hero/code/6142/` with its `.md` beside it, and its facts go into `docs/clone-hero/code/facts-6142.tsv` through `tools/ch_mcp/dump_facts.py`.
- (c) Read a run that already exists under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\` (folders dated 2026-10-28 to 2026-11-08).
- (d) A new live run through the `ch` MCP server. The row names the chart, the probe and the pass line, and says whether `docs/clone-hero/live-checks.md` already holds the check. Live runs come last, one at a time, never needing the window in front, never over the library.
- (e) A public source on the web, cited by its title and address (a video by its title and YouTube id).

## The rows

Each row gives the block id, what exactly is unknown in one sentence, the route, and what result moves the label where. "Stays" means the block stays Unknown with its Evidence line widened.

### Page 1: reading a chart

| Id | What is unknown | Route | Result and label |
|---|---|---|---|
| CR-03 | Whether the game's .srb reader reads the layout the bytes show (16-byte header, four DEFLATE streams, encrypted audio); its listing was lost. | (b) Find the reader through the callers of the header functions already listed (`code/6142/header_decode_214EFD0.asm`, `code/6142/header_encode_214F0A0.asm`) and list it. | Listing shows the four inflates and the audio section: Proven in the game's code. Reader not found: stays. |
| CR-05 | Which header bytes the game passes into the two header functions. | (b) Same caller as CR-03; read the bytes it loads before each call. | Caller shows bytes 0 to 7 as the key and 8 to 15 as the size: Proven in the game's code. Otherwise the block is reworded to what the caller does and labelled by it. |
| CR-07 | Whether the audio is AES-128-CFB with the swapped-halves IV; the decryptor listing was lost. | (b) From `code/6142/aes_key_build_215AFE0.asm` find who consumes the key and list the decryptor. | Listing shows the mode and IV build: Proven in the game's code. Not found: stays. |
| CR-12 | Which bytes of a .sng or folder chart the game hashes into the song cache. | (b) Find the cache writer by the songcache.bin literal in `stringliteral.json`, list the hashing step. Reading the user's cache again is route (c) and needs the user: question 2. | Listing shows the notes file's MD5 for both: Proven in the game's code. |
| CR-17 | What sets the .mid reader's thread-local switch and what the other path (RVA 0x2154180) does. | (b) List 0x2154180 and the setter of the static 0x21581E0 returns; read the decompile for both. | Both read: Proven in the game's code, with the rule reworded to what the path does. |
| CR-18 | What the track flags at +0xA1 and +0xA2 do downstream. | (b) Cross-reference reads of the two offsets on the track class; list each reader. | A reader shows the effect: Proven in the game's code. None found after the xref scan: stays, with the scan named. |
| CR-24 | What the coda and _ENDOFSONG branches store. | (b) Read decompile lines 41771 to 42120 and list the branches' stores. | Stores read: Proven in the game's code. If a branch changes the song end, that is a new question for the user (Hydra reads none of them). |
| CR-29 | Whether a .mid `[sectionX]` event makes a section in the game. | (d) LS-U1: a bot run of a probe .mid with one `[sectionX]` event; section_stats in the stats copy shows the name. live-checks.md holds CR-29 and says "needs a menu start"; the stats file's section list makes a bot run enough. | A section named from the text: Proven in a live run; Q-15 stays a question for the user. No such section: the block is reworded and Q-15 is closed by the user. |
| CR-30 | Whether the 2,718-row section table matches the game's; the listing is outside the repo. | (b) List RVA 0x2095040 with chdis, commit it, and compare its rows with `data/clone-hero-1.1.0.6142-section-names.tsv`. | Equal rows: Proven in the game's code. A difference: a question for the user (Hydra ships the copy, D228). |
| CR-35 | How the score code reads a solo record (is a note on the soloend tick inside the solo). | (b) Read `code/6142/solo_flags_20D18F0.asm` and list the solo list's reader in the engine. | The boundary read: Proven in the game's code. If it differs from Hydra's "soloend after the tick's notes": a new question, tied to Q-11. |
| CR-45 | Whether the song ends when the clock reaches GameManager's songLength (+0x50); the Update listing was lost. | (b) List GameManager.Update, RVA 0xC05A0, and commit it. | The compare read: Proven in the game's code. |
| CR-47 | Whether the game's audio length is its longest stem; the listing of RVA 0x36B700 was lost. | (b) List 0x36B700 and commit it. (d) live check CR-47 only if the listing does not settle it. | Listing shows the maximum over stems: Proven in the game's code. |
| CR-48 | Whether song.ini song_length or [music_end] is read on the song-end path. | (b) Read the unread lines of `code/6142/gm_load_coroutine_D8D80.asm` (101 to 260 and 487 to 700) and the Update listing from CR-45. | The whole path read with no such read: Proven in the game's code, worded as "not read on this path". A read found: a new question for the user. |
| CR-49 | Whether a bundle's stated length can exceed its real audio (Pressure, 273 s against 223 s). | (a) Pull Pressure's audio out of its bundle in the game's install folder with a short script (the AES key and IV are in `docs/srb-format.md` "The audio section"; Hydra's `extract_srb_audio` does the same but needs a built tool, so it is the main session's), then read the Ogg Opus length from the last page's granule position and compare it with 273 s. The folder `hydra-data\srb_audio_dump` holds stems of one song and none named for Pressure (P21), so it does not serve. Keep the numbers in a result file. | Measured: the label is question 1's answer. Not decodable: stays. |
| CR-51 | The rest of the song-info layout and the audio table, and the version constant 20210228. | (b) Find the reader class by the constant 0x01346234 in the decompile, list it, commit it. | Listing matches the field list: Proven in the game's code. |

### Page 2: the note list

| Id | What is unknown | Route | Result and label |
|---|---|---|---|
| NL-12 | Whether the drum track's event text is trimmed where the event is built. | (b) Read how `docs/audit/note-shuffle/step0/open1/mtrk_read_event_2150010.asm` builds the text and whether any trim runs before the drum reader. (d) live check NL-12 only if the code is unclear. | No trim on the path: Proven in the game's code. A trim: a new question (Hydra compares the raw text, D24). |
| NL-13 | How the drum track's event list is built and sorted, so whether a note before the tag at one tick stays plain. | (b) Read the list builder the mtrk reader fills and any sort in the finalizer at RVA 0x215C320. (d) live check NL-13 if the sort is not found. | Stable file order: Proven in the game's code. A sort by kind: a new question (D24). |
| NL-14 | What sets a kick's performed mark (+0x84 bit 4), so whether a ghost kick pays double. | (b) Read the kick branch of `code/6142/note_hit_dynamics_20DE770.asm` and the lane byte a kick uses. | The mark set for a kick by the same rule: Proven in the game's code. A different rule: a new question. |
| NL-23 | How many library charts carry drums0dnoflip (48 or 22). | A grep over the user's whole song library; that is a library-wide pass and needs the user's yes in chat: question 3. | With the yes: the count is kept in a result file and the block is reworded to it. Without: stays. |
| NL-28 | The live note counts of songs F1 and F2 with Double Kick off and on (11 and 14) are recorded only in prose. | (d) LS-U2a and LS-U2b: bot runs of F1 and F2 with the modifier as the bot launch sets it; total_notes in the stats copy. A bot start carries no modifier flag (P9), so one run shows only the count for the state the launch gives; the other count (11 or 14) needs the modifier changed, which is question 4. | Both counts seen and matching: Proven in a live run. Only one seen: the block is reworded to the seen count and stays Unknown for the other. They differ: a question for the user (Hydra pins them in tests/test_kick2x.cpp). |
| NL-30 | Whether songs F1 and F2 held both kick file orders. | (a) Open the two chart files and read chords 3 and 5; then LS-U2a and LS-U2b cover the run. | Both orders present and the counts match: Proven in a live run. |
| NL-31 | Whether the kick key scores a 2x-kick gem, and the Slipping pitch counts. | (c) Check whether Trippin SP Spike holds a 2x-kick gem (the main session makes its `hydra_replay` dump) against `tools/ch_probe/experiments/results/overlay-spike/run1_cal0_scorestats.json`'s 1,257 of 1,257; (a) recount the Slipping folder's pitches with a short script, output kept. | A 2x gem in the chart and all hit: Proven in a live run. No 2x gem in that chart: stays, and LS-U2 (F1 has 2x kicks) settles it on the kit instead. |
| NL-36 | What object the .chart N handler reads at track +0x50, field +0x114. | (b) Follow the track class in `dump.cs` to the field's type. | Named: Proven in the game's code. |
| NL-39 | Whether RVA 0x215E280 sets the flam flag 0x20000 for a .chart note. | (b) List 0x215E280 and read its writes to +0x7C. (d) live check NL-39 if unclear. | No .chart source of the flag: Proven in the game's code. A source: a new question (Hydra never flams a .chart). |
| NL-42 | Which pads the shuffle draws from per instrument; the jump-table decode has no saved output. | (a) Run `docs/audit/note-shuffle/step0/open34-refute/jumptables.py` against the DLL and commit its output beside it. | Output shows N = 8 with Pro Drums and N = 5 on plain Drums: Proven in the game's code. |
| NL-48 | Whether the redraw loop ends from any non-zero state; the check's output is not kept. | (a) Run `docs/audit/note-shuffle/probes/reach/reach_check.py` and commit its output beside it. | Full period shown: Proven in the game's code (the step is NL-41's listing). |
| NL-52 | The filmed Note Shuffle results of songs A1, A2, C2, D2 and E. | (d) Bot runs with Note Shuffle on; the command line carries no modifier (premise P9), so this waits on question 4. | With a route to the modifier: Proven in a live run. Without: stays. |
| NL-54 | Whether 0x20CEEC0 hands its last argument to the lane constructor, making S 65 single and S 66 double. | (b) List 0x20CEEC0 and follow the argument. | Handed on: Proven in the game's code. |
| NL-56 | Whether only the drum engine reads the lane flags 0x4000, 0x8000 and 0x10000. | (b) Cross-reference every reader of note +0x7C that masks those bits. | All readers in the drum engine: Proven in the game's code, worded as "every reader found". Another reader: the block is reworded to name it. |
| NL-57 | How the game draws a lane under Note Shuffle; the draw routine was never read. | (b) Find the lane renderer class in `dump.cs` and list its draw method. A human-eye run (live check NL-57) cannot run unattended, so no live step. | Listing shows which pads it lights: Proven in the game's code; D254 keeps Hydra's choice either way. |
| NL-58 | The value of the run-time divisor (+0x68 static) in the late-lane test. | (b) Find the static's writer in `dump.cs` and its listing. | Value read: Proven in the game's code. |
| NL-65 | Whether a .mid loses its dynamics on plain Drums like a .chart. | (d) LS-U3: a bot run on plain Drums of a .mid with the dynamics tag; ghost_notes_total and accent_notes_total in the stats copy. live-checks.md holds NL-65. | Both 0: Proven in a live run. Counts present: a new question (Hydra keeps them, the user's 2026-10-28 ruling). |
| NL-69 | What the modifier steps at RVAs 0x20F1900, 0x20F1250 and 0x20F06A0 do. | (b) List all three and commit them. | Read: Proven in the game's code, one rule per step. |
| NL-70 | Whether the game ever opens an `expert+.mid`, and the Slipping pitch counts. | (b) Search `stringliteral.json` for expert+ and list any reader; (a) recount the two files' pitches, output kept. | No literal and no reader: Proven in the game's code for the game half; the count half takes question 1's label. |
| NL-71 | The 22-of-22 video match rests on scripts with no kept output and a local video. | (a) Run `docs/audit/note-shuffle/probes/order_probe.py` and commit its output. The video is a local file, which the README does not count as a record: question 5. | Output kept and the user rules the local video counts: Proven in a live run. Otherwise stays, with the output cited. |
| NL-73 | What the builder's byte that equals 3 means. | (b) Trace the builder's second argument to its caller and the object's class in `dump.cs`. | Meaning read: Proven in the game's code. If a player can meet the step, it is a new question (Hydra has no such step). |

### Page 3: timing

| Id | What is unknown | Route | Result and label |
|---|---|---|---|
| TM-06 | Whether the .sng reader at RVA 0x1EFF90 follows the "delay wins when nonzero" rule. | (b) List 0x1EFF90 and commit it. | Same rule: Proven in the game's code. A different rule: a new question (D275 item 4). |
| TM-08 | Whether the getter at RVA 0x1F0830 that divides by 1582 has any caller. | (b) List it; cross-reference virtual and delegate callers with the `rea` tools. | No caller found by xref: Proven in the game's code, worded "no caller found by cross-reference". A caller: the block is reworded to it. |
| TM-10 | Whether GameManager.Update stores both clocks every frame. | (b) The Update listing from CR-45 (one listing serves both). | Stores read: Proven in the game's code. |
| TM-16 | Whether there is about 3 ms of input polling delay; the +2.5 ms is a lost screenshot. | (b) Find the writer of the on-screen average offset and what it adds. Code may not settle a latency; if not, question 6 asks whether the block is reworded to "not measured". | Writer shows an added constant: Proven in the game's code. Nothing: stays or reworded per question 6. |
| TM-32 | Whether the Precision Mode option sets bit 0x1000 of #8288+0x198, and the live window it gives. | (b) The enum value PrecisionMode = 4096 equals the bit; trace the modifiers word into the engine's +0x198. The live window needs the modifier on: question 4. | The copy read: Proven in the game's code for the bit; the window stays until a run. |
| TM-36 | Whether anything that judges a hit reads the video clock (about 30 readers of RVA 0x16C490 untraced). | (b) Cross-reference every caller of 0x16C490 and sort each as drawing or judging. | All readers draw or store for drawing: Proven in the game's code. A judging reader: a new question. |
| TM-37 | How the calibration screen picks its values; only class metadata was read. | (b) List Calibrator.Update, RVA 0x2FB0B0. | Read: Proven in the game's code. |
| TM-40 | Whether the audio position the clock divides is in file seconds or stretched seconds. | (b) List the audio manager member at RVA 0x365F10 and the BASS call it makes. | Unit read: Proven in the game's code. |
| TM-43 | Whether the hit window and clock run in chart or real time at another speed. | (b) Follow `code/6142/speed_ratio_210C090.asm` into the note times and the window. (d) live check TM-40 and TM-43 needs a speed setting: question 4. | Scaling read: Proven in the game's code. |
| TM-46 | Whether a tom key and a cymbal key of one colour for one gem leave a pad over, so the gem is missed. | (b) Read which pad bits each key sets (`code/6142/drums_input_source.asm`); then (d) LS-U4: one kit chord of yellow tom plus yellow cymbal on a single yellow gem; pass is excess_hits 1 and that gem missed. live-checks.md holds TM-46 as a key check; the kit replaces the keys. | Run shows the miss: Proven in a live run. The gem hits: the block is reworded. |

### Page 4: fills

| Id | What is unknown | Route | Result and label |
|---|---|---|---|
| FL-08 | What a written fill's skip byte at +0x20 means and what sets it. | (b) List RVA 0x20FB9A0 and the writers of +0x20. | Read: Proven in the game's code. |
| FL-10 | What the game does when two written fills pick one chord. | (b) Read `code/6142/fill_to_activation_20CFF60.asm` for the second landing on a flagged chord. | Both kept: Proven in the game's code; the difference from Hydra (keeps the later length) is a new question. |
| FL-22 | Whether engine +0x108 is the current tick or the clock in whole milliseconds. | (a) IN-88's listing reads +0x108 as whole milliseconds; (b) find the field's writer. | Writer read: Proven in the game's code. |
| FL-23 | Whether Hydra clears a fill at exactly fill start minus 4 x resolution, matching the game's boundary. | (a) Read src/search/engine.cpp's `index_fills`, `ready_class` and `branch_activate`; a test pins the exact boundary (boundary cases follow the rule). | Boundary equal: the Hydra side is proven by the test and the game side by line 45 of the deadline listing, so Proven in the game's code. Unequal: a question for the user. |
| FL-29 | What the .mid reader does with pitch 120 across the four difficulties. | (b) Read `code/6142/mid_fill_reader_20CEDC0.asm` and the drum reader's pitch-120 path; (a) check Hydra's .mid parser hands each difficulty the same fills. | Read on both sides: Proven in the game's code. A difference: a new question. |
| FL-32 | When the game draws a fill on the highway. | (b) Find the fill renderer in `dump.cs` and list it. A capture run (live check FL-32) is question 7 if code does not settle it. | Draw time read: Proven in the game's code. |
| FL-45 | Whether "11 of 97 corpus charts changed fills" and the Colors numbers hold. | (c) The main session reruns Hydra's 1.0 fills over `testdata/input` (a corpus run, not the library) and opens the Colors entry in the video-tools refs file; where that file is, is premise P12, unchecked. | Numbers reproduced: the block is reworded to them; it is a Hydra fact, so question 1 names its label. |

### Page 5: Star Power

| Id | What is unknown | Route | Result and label |
|---|---|---|---|
| SP-06 | The sort order of the phrase list, so whether a nested phrase ever awards. | (b) Read the loader's phrase add and the clean-up at RVA 0x5DB320 (shared with SP-08). (d) live check SP-06 (a chart load and a flag read) if the sort is not found. | Order read: Proven in the game's code; if a nested phrase cannot award, a new question (Hydra awards every phrase). |
| SP-07 | Whether a .chart phrase and a .mid phrase are stored the same way. | (b) List RVA 0x20D5E60 and the note-116 path in the .mid drum reader. | Same record and list: Proven in the game's code. |
| SP-08 | What the clean-up at RVA 0x5DB320 deletes. | (b) List 0x5DB320 and commit it. | Read: Proven in the game's code. |
| SP-14 | Why run 1's first activation started from 2,879, not 2,880. | (c) Read the first active row of every Star Power watch CSV under the 2026-11-06 and 2026-11-07 run folders. | Every other run starts at a full bar: Proven in a live run, worded "noise seen once". A repeat: a new question. |
| SP-15 | Whether the game's Star Power length follows the time signature. | (b) Read the two segment fields at +0x30 and +0x40 in RVA 0x20D1D20 and relist 0x20D2930. (d) live check SP-15 later, on a chart with a 3/4 section, if code does not settle it. | Fields read: Proven in the game's code; a difference from D231 is a question. |
| SP-18 | Whether drums can activate without a fill note. | (c) Runs that declined fills (`decline_mode`) under the 2026-11-07 and 2026-11-08 folders: phrases earned with sp_activations 0. | Such a run exists: Proven in a live run. None: (d) is a later run with decline_mode `nothing` on Window Map. |
| SP-29 | Whether a hit is scored inside the frame's first step, before the end check. | (b) Find the caller of the press handler (RVA 0x20DEAA0) from the frame update, through `docs/audit/drum-lanes/play/f20de070_poll_input.asm` and `f20f5810_base_update.asm`. | Call order read: Proven in the game's code; Q-50 then goes to the user with the real rule. |
| SP-30 | Whether a hit is judged at a sub-step clock between frames. | (b) Same reading as SP-29: which clock the press handler reads. (d) live check SP-30 (a hit trace with clocks) if the code leaves it open. | Read: Proven in the game's code. |
| SP-35 | Which call credits a phrase on a hit. | (b) Resolve the virtual call in `docs/audit/drum-lanes/play/f20f63f0_base_hit.asm` lines 206 to 216 through the vtable in `dump.cs`, list the target. | Target writes the meter: Proven in the game's code. |
| SP-43 | Whether the drum engine holds a 3 ms constant as an immediate operand. | (b) Scan immediate operands of every method in #8288, #8253, #8255 and #8544 for 0.003, 3.0 and 3000.0. Absence by scan is not proof; question 8 asks whether a complete scan is enough to label it. | Scan complete and empty: the label follows question 8. A constant found: a new question. |
| SP-46 | Whether the multiplier disc is the only Star Power sign on the 1.1 drums screen. | (e) A public 1.1 drums video, cited by title and YouTube id; (b) search `dump.cs` for a Star Power meter class on the drums HUD. | Video and no meter class: Proven in a live run. A meter class drawn: the block is reworded. |

### Page 6: scoring

| Id | What is unknown | Route | Result and label |
|---|---|---|---|
| SC-09 | What the fourth total at +0xD0 and its two note flags mean. | (b) Read the two getters' bits against `docs/audit/note-shuffle/scout-flag/note_getters.txt` and the stats writer's source for squeezed_score (shared with SC-37). | Meaning read: Proven in the game's code. |
| SC-11 | Whether the combo carries into the next song of a setlist. | (e) The game's changelog or wiki; a setlist run needs the menus, which the tools do not drive for setlists: question 9. | A public source states it: Proven in a live run (the source is the record). None: stays. |
| SC-23 | The solo box's start, percent and message ladder; the listing was lost. | (b) Find the method by the Awesome Choke! literal in `stringliteral.json`, list it. | Read: Proven in the game's code; any difference from Hydra's box is a question (D251). |
| SC-26 | Whether the rounding helper's software path also rounds up. | (b) List the helper's software routine. | Rounds up: Proven in the game's code. |
| SC-29 | Whether the game counts stars on the score without the solo bonus. | (b) List the four callers of RVA 0x210DD00 in #8351. (d) live check SC-29 if unclear. | Read: Proven in the game's code; a difference from Hydra is a question. |
| SC-37 | Which engine counter feeds each of the 46 per-player stats keys. | (b) Find the per-player serialiser by the solo_bonus_total literal's cross-references, list it, fill the table's third column. | Writers traced: Proven in the game's code. |

### Page 7: internals

| Id | What is unknown | Route | Result and label |
|---|---|---|---|
| IN-14 | Whether old engine objects stay in memory after a restart. | (c) Engine-finder output in run folders that saw more than one engine; (b) nothing frees them that was read. No tool restarts a song, so no live step. | Two engines in one run's output: Proven in a live run. None: stays. |
| IN-19 | Whether RVA 0x20DEEC0 fires only for hit notes; the trace is lost. | (d) LS-U5: a kit run with deliberate misses (the edge probe pressing late) and the hit trace on; pass is calls equal to notes_hit. live-checks.md holds IN-19. | Equal: Proven in a live run. |
| IN-20 | Whether RVA 0x20DE770 fires for a missed note. | (d) LS-U5, with the trace hook on this method in a second pass of the same run shape. | Count equals hits: Proven in a live run. |
| IN-24 | Whether a breakpoint written into the code crashes the game. | No safe route: redoing it risks a crash and the tools hook with Frida now: question 10. | Per the user's answer: reworded as "observed once, not retried" or retried once. |
| IN-29 | Whether GlobalVariables.currentSongEntry holds the playing song. | (b) Cross-reference the field's writers. | Writer at song start: Proven in the game's code. |
| IN-30 | What the song-select classes do; none of their methods was read. | (b) List the method that starts a song from the list. The block's claim is a location, so question 11 asks what it should claim after the read. | Read: Proven in the game's code, reworded per question 11. |
| IN-39 | Who writes Bot into GlobalVariables.scoreDisabled in 1.1. | (b) Cross-reference writers of the field. | Found: Proven in the game's code. |
| IN-40 | Whether the 1.0 bot earns Star Power (the scrambled test method's body). | (a) Read the method in the 4045 decompile under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\4045`. | Read: Proven in the game's code (build 1.0). |
| IN-41 | Whether SendInput keys reach the game for all eight drum keys. | Needs the key route and focus, which the user ruled out for runs: question 12. | Recommended: reworded as superseded by the kit route; stays Unknown. |
| IN-42 | Whether the kick key scored a 2x-kick gem on run 1. | (c) As NL-31: whether Trippin SP Spike holds a 2x gem. | A 2x gem present: Proven in a live run. |
| IN-43 | Whether a command-line song logs only its loading line at first. | (c) Game logs copied under the run folders, if any; else (d) the next bot run copies Player.log. | A log read: Proven in a live run. |
| IN-44 | Whether a setting fills the HitDetection logs. | (b) Cross-reference the HitDetection literals to the method that opens them and its condition. | Condition read: Proven in the game's code. |
| IN-45 | The string count, the 776 source paths, and the combo: {0} method at RVA 0x1168290. | (a) Recount `stringliteral.json` and the paths in `dump.cs`; (b) list 0x1168290. | Counts kept and the method read: Proven in the game's code. |
| IN-47 | The scrambled method counts per assembly. | (a) Recount from `dump.cs` with a short script, output kept. | Counts kept: the label is question 1's answer. |
| IN-50 | Whether the 1.1 engine name is absent from 1.0 and only 13 GameManager fields are shared. | (a) Compare `dump.cs` with the 4045 facts file, output kept. | Compared: the label is question 1's answer. |
| IN-55 | The format of scoredata.bin and whether play_count spans instruments. | (b) Find the writer by the scoredata.bin literal, list it; (c) compare a run folder's backup with the file after the run. | Writer read: Proven in the game's code. |
| IN-59 | Whether the player screen's instrument and difficulty fields are the profile's saved copy. | (b) List the screen's open method that writes +0x240 and +0x241. | Read: Proven in the game's code. |
| IN-60 | How Green, Yellow and Blue move the player screen; only a decompile summary exists. | (b) List the input methods and commit them. | Read: Proven in the game's code. |
| IN-71 | Whether the loader's profile clone is ever saved to profiles.ini. | (c) `ch_stop`'s restore lines in the human-run folders say whether profiles.ini changed; (b) the save method's name test. | Unchanged in every human run and the code agrees: Proven in the game's code. |
| IN-73 | Whether a finished song posts anything online with nobody logged in. | (b) Cross-reference RVA 0x285AD0 through method tables and delegates; (c) the game log lines in the run folders. Code may not prove absence: question 13 asks whether to stop there. | A sender found: the block is reworded. None after the xref: per question 13. |
| IN-80 | What the keyboard does in the menus. | Needs the key route and focus: question 12 (with IN-41). | Recommended: stays Unknown, worded as superseded by the kit route. |
| IN-88 | What the items of the engine's +0x1F8 list are. | (b) Find the list's writer in the engine constructor or the loader and the item class in `dump.cs`. | Named: Proven in the game's code. |
| IN-89 | The meaning of the unnamed note getters. | (b) Cross-reference each unnamed getter's callers and name its meaning where a caller shows it. | Those read: named in the block; the label is Proven in the game's code when every getter has a reader or a stated "no caller found". |
| IN-90 | The meaning of the constructor's unnamed fields. | (b) Cross-reference readers of each field. | Same rule as IN-89. |
| IN-91 | Whether RVA 0x32F590 runs once a frame. | (b) Find its caller (Update or a coroutine) in the decompile. | A per-frame caller: Proven in the game's code. |
| IN-92 | Where the game keeps its bindings and settings (registry key, schema file, poll_rate). | (b) Cross-reference the poll_rate and settings_schema literals; (e) Unity's PlayerPrefs page for the registry path, cited. | Readers listed and the page cited: Proven in the game's code. |

Rows per route, counting each block once by its first route: (a) 10 (NL-30, NL-42, NL-48, NL-71, CR-49, FL-22, FL-23, IN-40, IN-47, IN-50), (b) 65, (c) 8 (NL-31, SP-14, SP-18, IN-14, IN-42, IN-43, IN-71, FL-45), (d) 7 (CR-29, NL-28, NL-52, NL-65, IN-19, IN-20, TM-46 after its code half), (e) 2 (SC-11, SP-46), and 4 go to the user first with no route of their own (NL-23, IN-24, IN-41, IN-80). That is 96 rows.

## Workstreams

Code-reading rounds come first and run in parallel, three or four agents a round, about eight blocks each, each agent inside 100 tool calls and 20 minutes. Each agent owns the new listings it makes under `docs/clone-hero/code/6142/`, the result files it keeps, and nothing else; it does not edit an area page. Live runs come after every code round is in, one at a time on this machine.

Round 1, four agents, pages 1 to 3 by code.

- W1, chart files: CR-03, CR-05, CR-07, CR-51, CR-12, CR-17, CR-18, CR-24.
- W2, the song end and sections: CR-30, CR-35, CR-45, CR-47, CR-48, TM-10, TM-06, TM-08.
- W3, note list part one: NL-12, NL-13, NL-14, NL-36, NL-39, NL-70, NL-73, NL-31's chart check.
- W4, timing: TM-16, TM-32, TM-36, TM-37, TM-40, TM-43, TM-46's code half, NL-54.

Round 2, four agents, pages 2 and 4 to 7 by code.

- W5, note list part two and the committed scripts: NL-56, NL-57, NL-58, NL-69, NL-42, NL-48, NL-71's script, NL-30's chart read.
- W6, fills and phrases: FL-08, FL-10, FL-22, FL-29, FL-32, SP-06, SP-07, SP-08.
- W7, the engine's Star Power path: SP-15, SP-29, SP-30, SP-35, SP-43, IN-88, IN-90, FL-23.
- W8, scoring and stats: SC-09, SC-23, SC-26, SC-29, SC-37, IN-44, IN-55, IN-45.

Round 3, three agents, internals by code plus the data and run reads.

- W9, internals: IN-29, IN-30, IN-39, IN-59, IN-60, IN-91, IN-71's code half, IN-73.
- W10, more internals and the unnamed members: IN-89, IN-92, IN-40, SP-46's class search, SC-11's public source, SP-46's video.
- W11, existing runs and game files: SP-14, SP-18, IN-14, IN-43, IN-71's restore lines, IN-42, CR-49, IN-47, IN-50.

Round 4, the live steps below, one at a time, after the user's yes to each. FL-45's corpus rerun and NL-31's chart dump are the main session's, because they need built tools (agent rule 14).

Every agent brief starts with `docs/agents/brief-preamble.md` and carries what `docs/agents/audit-method.md` lists for an audit brief: the exits, that only the main session picks a block's exit, that "on purpose" and "by design" are claims, that an empty search proves nothing, and that no whole-library run happens without the user's words. Each agent's report lists, per block, the result and the proposed label move, with the listing lines or the run file that prove it.

## Who writes the pages, who checks

Nobody writes an area page until the user has seen the moves that touch a Hydra rule. After each round, the main session groups the proposed moves into two lists: moves on blocks whose Hydra line says "no owner", and moves on blocks that name a Hydra owner. The second list goes to the user with each block's old and new label and the proof. Any result that shows the game differing from Hydra goes in the same message as its own question, written to audit-method's "How to write a question", and is never folded into a label move.

Then one executor on Opus per page set writes the pages: set A is pages 01 and 03, set B is page 02, set C is pages 04 and 05, set D is pages 06 and 07. Each also updates the README's counts table, `live-checks.md` (a check that a run settled leaves the page) and `questions.md` (new Q-ids). The executor runs `tools/ch_mcp/dump_facts.py --check` for the facts table.

A fresh agent on Sonnet then checks every label move, per audit-method stage 6: it opens the listing lines or the run file the new Evidence line names and returns PASS, REOPEN or GONE for each move. A move with REOPEN goes back to Unknown before merge. The checker never checked a block it found or wrote.

## The live steps

Each step names its chart, its probe and its pass line, and lists its calls. Before a step runs, the main session shows the user its `Calls:` line, names each never-tried call and each unchecked premise row it uses, and waits for the go. Every run ends with `ch_stop` and its restore lines. Each step runs at most twice; a second failure stops the step and is reported, not retried.

### LS-U1: CR-29, a .mid section with no space

Chart: a new probe .mid under `C:\Clone Hero\songs\Hydra Probe\` with one `[sectionX]` text event and a few notes, made by the main session with the tool that made Window Map (premise P7). Probe: a bot run from the command line; copy the stats before the stop. Pass: section_stats holds a section whose name is `X` or empty, as the game cuts from character 9. Fail: the fallback "0% - 10%" sections only. Uses P7 and P8.

Calls: `ch_start(song_folder)` (seen: the 2026-10-28 bot runs, whose stats copies are in hydra-data/ch-mcp-runs/2026-10-28/l1/bot), `ch_wait_stats` (seen: the same runs), `ch_stop` (seen: the same runs)

Every call in this step has been seen before. The new piece is the probe chart.

### LS-U2a: NL-28 and NL-30, song F1

Chart: song F1 of the Note Shuffle game tests (premise P10 names where its folder is). Probe: a bot run; the stats copy. Pass: total_notes 14 with Double Kick on, as the bot launch sets it; 11 if the launch has it off; the stats copy also names which state the run had, and the step reports it. Fail: another count. Uses P10.

Calls: `ch_start(song_folder)` (seen: the 2026-10-28 bot runs, hydra-data/ch-mcp-runs/2026-10-28/l1/bot), `ch_wait_stats` (seen: the same runs), `ch_stop` (seen: the same runs)

### LS-U2b: NL-28 and NL-30, song F2

The same as LS-U2a on song F2. Uses P10.

Calls: `ch_start(song_folder)` (seen: the 2026-10-28 bot runs), `ch_wait_stats` (seen: the same runs), `ch_stop` (seen: the same runs)

### LS-U3: NL-65, a .mid on plain Drums

Chart: the .mid chart LS-O3 run 1 played, which has 368 accents and 726 ghosts (premise P11). Probe: a bot run with the player `Drums,Expert`. Pass: accent_notes_total and ghost_notes_total both 0 and the score below LS-O3's by the dynamics points. Fail: either count above 0. Uses P11.

Calls: `ch_start(song_folder, player="Drums,Expert")` (never tried), `ch_wait_stats` (seen: the 2026-10-28 bot runs), `ch_stop` (seen: the 2026-10-28 bot runs)

### LS-U4: TM-46, a tom and a cymbal on one gem

Chart: Edge Walk Pair, as LS-E run 1 played it (premise P6). Probe: a human launch, attach, kit on System, then one `ch_kit_press` of yellow tom plus yellow cymbal at a single yellow gem's time, the rest of the chart left unplayed. Pass: excess_hits 1 and notes_hit 0 in the stats copy. Fail: notes_hit 1. Uses P6.

Calls: `ch_start(song_folder, human=True)` (seen: LS-E run 1, hydra-data/ch-mcp-runs/2026-11-07/ls-e-run1), `ch_attach` (seen: LS-E run 1), `ch_kit_setup(player="System")` (seen: LS-E run 1), `ch_kit_press` (seen: LS-D1b run 1, hydra-data/ch-mcp-runs/2026-11-06/ls-d1b-run1; the two-item chord is the new piece, not the call), `ch_wait_stats` (seen: LS-E run 1), `ch_stop` (seen: LS-E run 1)

### LS-U5: IN-19, the score method on a run with misses

Chart: Edge Walk Pair. Probe: LS-E run 1's own `probes.json` (hydra-data/ch-mcp-runs/2026-11-07/ls-e-run1), which pressed four chords about 83 to 89 ms late (they missed) and the rest on time (117 of 121 hit), with the hit trace on the score method. A run where every note misses would pass even with a dead hook, so the mix is what tells "fires only for hit notes" from "does not fire". Pass: the trace's call count equals notes_hit (117) and misses are 4. Fail: any other count. IN-20 is the same run shape a second time with the trace on the dynamics method (`with_dynamics=True`), and runs only after this one passes. Uses P6.

Calls: `ch_start(song_folder, human=True)` (seen: LS-E run 1), `ch_attach` (seen: LS-E run 1), `ch_kit_setup(player="System")` (seen: LS-E run 1), `ch_trace_start` (never tried) on the kit route (the key route used it in hydra-data/ch-mcp-runs/2026-10-28/l3), `ch_edge_probe` (seen: LS-E run 1), `ch_trace_stop` (seen: hydra-data/ch-mcp-runs/2026-10-28/l3), `ch_wait_stats` (seen: LS-E run 1), `ch_stop` (seen: LS-E run 1)

Fallback live steps for NL-12, NL-13, NL-39, SP-06, SP-15, SP-18, SP-30, SC-29 and CR-47 are written only if their code route leaves them Unknown; each would be a bot run or a chart load as live-checks.md already describes, and gets its own `Calls:` line before it is proposed.

## Questions

1. A fact measured from the game's files or dumps, not from its code and not from a run (CR-49, IN-47, IN-50, the count halves of NL-70 and NL-31, FL-45): which label does it take? Options: keep Unknown with the result file cited; count it as "Proven in the game's code" when the file is a dump of the code's own data; add nothing and reword the block to the measured number. Recommended: the last, with the kept result file in the Evidence line. Against: a reworded block with an Unknown label still reads as unproven.
2. CR-12: may an agent read the game's songcache.bin once more against a sample of chart folders from your library (300 folders, seconds, no analysis)? The code route runs first either way. Recommended: no, the code route is enough.
3. NL-23: may an agent grep every chart file in your library for drums0dnoflip (a file scan, minutes, no analysis)? Without it the 48-against-22 count stays unsettled. Recommended: yes, once, with the count kept in the repo.
4. NL-52, TM-32's window, TM-43's live half and the second count of NL-28 need a modifier or a speed set, and a command-line start carries none (P9). Should an agent set it through a game setter on a started game (writes were allowed on 2026-10-30), or should these blocks stay Unknown with the code result only? Recommended: let W4 read whether the game's own loader takes a modifier first, and ask again with what it finds; if it takes none, the setter route is a separate plan with its own Calls line.
5. NL-71: the only record of the 22-of-22 match is a local video. Does a local video, with a committed frame reading, count as a record for "Proven in a live run"? The README counts a public video by its id. Recommended: no; keep the block Unknown with the probe's committed output.
6. TM-16: if no code adds a polling delay to the on-screen average, should the block be reworded to "not measured" and stay Unknown, or dropped from its claim? Recommended: reworded.
7. FL-32: if the renderer listing does not give the draw time, is a capture run (screen recording beside a bot run) wanted? Recommended: no; Hydra has its own rule (FL-33).
8. SP-43: is a complete scan of every immediate operand in the four engine classes enough to label "no 3 ms constant" Proven in the game's code? Recommended: yes, worded as what was scanned.
9. SC-11: a setlist run needs the menus. Is a public source enough, or should the menu walk be extended to setlists? Recommended: the public source; Hydra's one-combo-per-chart stays.
10. IN-24: retry the breakpoint write once (it may crash the game; the tools no longer use it), or reword the block as "observed once, not retried"? Recommended: reword.
11. IN-30: after the song-start method is read, what should the block claim? Recommended: the method that starts a song from the list and its arguments.
12. IN-41 and IN-80 need SendInput keys and the window in front, which you ruled out for runs. Keep them Unknown, worded as superseded by the kit route? Recommended: yes.
13. IN-73: if the cross-reference finds no sender, stop there and word the block "no sender found by cross-reference", or capture the game's network during one bot run? Recommended: stop there.
14. The `rea` server's cross-reference tools have not been tried on the game's `GameAssembly.dll` in this project (P14). If the first agent to try them finds they cannot read the file, should the agents fall back to a text search of the Ghidra decompile for callers, which misses virtual calls? Recommended: yes, and the block's Evidence line then says "direct callers only".

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| Every Unknown block, and only those, is in the plan (96). | The 96 ids, first section | the rows | A count of Label: Unknown lines over `docs/clone-hero/0*.md` equals the ids listed; the plan reviewer runs it. |
| Every row names one route and the result that moves the label. | "The rows" | the rows | The reviewer reads each row for a route letter and a label in its last cell. |
| No Hydra rule changes from this plan; a difference is a question. | "How a check ends" | the page executors | The reviewer greps the executors' diffs for `src/` and `tools/` changes; none allowed. |
| Moves on blocks with a Hydra owner wait for the user. | "Who writes the pages" | the main session | The go message to the executors quotes the user's yes. |
| A fresh agent checks every label move. | "Who writes the pages" | the checker | The checker's file has one verdict per moved block. |
| Live runs run one at a time, never needing focus, never over the library. | "The live steps" | LS-U1 to LS-U5 | Each step's Calls line holds only kit-route tools; no step names the whole-library flag or the key route. |
| Each agent stays inside 100 calls and 20 minutes, about eight blocks. | "Workstreams" | W1 to W11 | The progress hook's status files; a workstream list of nine or fewer blocks. |
| Every new listing is committed with its `.md` and its facts rowed. | "Routes" (b) | W1 to W10 | `tools/ch_mcp/dump_facts.py --check`. |
| A block that cannot be checked goes to the user. | first section | questions 2 to 13 | Each such row's Route cell names its question number. |
| Each live step has a cap of two runs. | "The live steps" | LS-U1 to LS-U5 | The go message names the run number; a third is refused. |

## Out of scope

The 30 "Hydra assumes it, never checked" blocks (the user chose "Unknown only"). The 24 open questions on `questions.md` as rulings; this plan may add questions and never closes one without the user's words.

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: 96 blocks carry Label: Unknown on the seven area pages, 15, 21, 10, 7, 11, 6 and 26 by page. | code | a count of Label: Unknown lines over `docs/clone-hero/0*.md` on 2026-11-08, matching `docs/clone-hero/README.md` line 64 | checked 2026-11-08 |
| P2: the four labels and what each needs are as `docs/clone-hero/README.md` "The four labels" says. | code | `docs/clone-hero/README.md` lines 19 to 26 | checked 2026-11-08 |
| P3: `tools/ch_mcp/chdis.py` takes an RVA, a byte count, `--dll` and `--dumps`. | code | `tools/ch_mcp/chdis.py` lines 235 to 241 | checked 2026-11-08 |
| P4: the 6142 dump folder holds `dump.cs`, `script.json`, `stringliteral.json` and `ghidra/ghidra_decompile_all.txt`; a 4045 folder exists beside it. | code | a directory listing of `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps` on 2026-11-08 | checked 2026-11-08 |
| P5: run folders exist under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\` for 2026-10-28, 10-31, 11-06, 11-07 and 11-08, including `2026-11-07/ls-e-run1` and `2026-11-08/ls-m2-run1`. | run | a directory listing of `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs` on 2026-11-08 | checked 2026-11-08 |
| P6: LS-E run 1 played Edge Walk Pair with `ch_start(human=True)`, `ch_attach`, `ch_kit_setup(player="System")`, `ch_edge_probe`, `ch_wait_stats` and `ch_stop`. | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-e-run1\replies.txt` and its `probes.json` (117 of 121 chords hit, four late probes missed) | checked 2026-11-08 |
| P7: the probe charts under `C:\Clone Hero\songs\Hydra Probe\` are Edge Walk, Edge Walk Pair, Ladies Talk fill check, Trippin SP Spike and Window Map, and the main session has a tool that wrote them. | unchecked | a directory listing of `C:\Clone Hero\songs\Hydra Probe` on 2026-11-08 shows the five folders; the tool that made them was not found | LS-U1 |
| P8: `ch_wait_stats` copies scorestats for a command-line start, and the file's section_stats lists section names. | code | `docs/agents/clone-hero-mcp.md` line 58 and `docs/clone-hero/06-scoring.md` SC-37's table (section_stats row) | checked 2026-11-08 |
| P9: a command-line bot start carries no modifier such as Note Shuffle or Precision Mode; its command is `--song`, `--player` and `--profile` only. | code | `tools/ch_mcp/bot_run.py` `launch_command`, lines 413 to 422 | checked 2026-11-08 |
| P10: songs F1 and F2 of the Note Shuffle game tests exist as folders the game can load. | code | `C:\Clone Hero\songs\Hydra Note Shuffle tests\NS F1 2x kick shared tick mid` and `...\NS F2 2x kick shared tick chart`, each with a notes file, song.ini and song.ogg, listed 2026-11-08 | checked 2026-11-08 |
| P21: `C:\Users\Patrick\Downloads\Hydra\hydra-data\srb_audio_dump` holds stems of one song and nothing named for Pressure. | code | a directory listing of `C:\Users\Patrick\Downloads\Hydra\hydra-data\srb_audio_dump` on 2026-11-08 (27 files: stem WAVs and first_read bins) | checked 2026-11-08 |
| P11: LS-O3 run 1 played a .mid chart with 368 accents and 726 ghosts. | unchecked | `docs/clone-hero/06-scoring.md` SC-37's table names LS-O3 run 1's counts; the chart's format was not opened | LS-U3 |
| P12: the Colors (BTBAM) run's `refs.json` entry exists in the video-tools folder. | unchecked | `docs/clone-hero/04-fills.md` FL-45 names the entry; no file was opened | question 1 |
| P13: `tools/ch_mcp/dump_facts.py --check` rebuilds the facts table and reports differences. | code | `docs/clone-hero/README.md` line 94 | checked 2026-11-08 |
| P14: the `rea` MCP server offers cross-reference tools for native binaries. | unchecked | the server's instructions text as this session lists it, which names `GameAssembly.dll` nowhere; no tool was called | question 14 |
| P16: the user chose "Unknown only" for this plan's scope. | ruling | `docs/agents/rulings/clone-hero.md` line 5, which holds the user's words "every claim labeled unknown should be checked" and their choice "Unknown only" | checked 2026-11-08 |
| P17: writes to game fields through setters were allowed by the user on 2026-10-30 ("Allow writes too"). | ruling | `docs/agents/rulings/clone-hero.md` line 12 | checked 2026-11-08 |
| P18: runs must not need the window in front ("yes we need to do autonomous runs. that waws the whole point of this"). | ruling | `docs/agents/rulings/clone-hero.md` line 11 | checked 2026-11-08 |
| P19: a whole-library run needs the user's yes in chat ("I did not approve a full library test and I did not approve a 40 minute test"). | ruling | `docs/agents/rulings/tests-and-runs.md` line 11 | checked 2026-11-08 |
| P20: the committed scripts `jumptables.py`, `reach_check.py` and `order_probe.py` exist at the paths the rows name. | code | `docs/audit/note-shuffle/step0/open34-refute/jumptables.py`, `docs/audit/note-shuffle/probes/reach/reach_check.py`, `docs/audit/note-shuffle/probes/order_probe.py`, existence checked 2026-11-08 | checked 2026-11-08 |
