# G7a report: the song model (parse/song, timesig, note_shuffle)

Scout: concept-audit-G7a. Files read in full: src/parse/song.cpp (2,074 lines), song.h (384), timesig.h (21), note_shuffle.cpp (190), note_shuffle.h (34). Outside the group I read only what was needed: core/model.cpp (Chord, lines 10-310), core/timing.cpp (all), core/replay.cpp 180-211, and greps.

Short answer to the group question: the song model mostly does each step once. Both parsers (.chart and .mid) hand their finished facts to the same helpers for time signatures, SP phrases, authored fills, solo sections, drum lanes, practice sections, disco flip and note merging. The .sng and .srb loaders add no rules of their own: they find the notes file inside the container and call the .mid or .chart loader. Difficulty is one table (kDifficultyChartCodes, song.cpp 735-740) and everything reads it. I found four candidates, none of them a big split, and one known finding that is still open.

## Candidates

### G7a-1. In what order does the game list a drum pad's lane bits (red, tom pads, cymbal pads)?
Looks like: K1 (two spellings of one game fact; agree today)
Homes: lane_bit_rank, src/parse/song.cpp 112-116 (a note's colour and cymbal flag; used by lane_pads 118-135 to find the lowest and highest pad of a chord);
       lane_bit, src/parse/note_shuffle.cpp 24-33 (a note's colour and cymbal flag; used to build the shuffle's lane masks). Two more spellings of the same fact sit in note_shuffle.cpp: moved_to 37-51 (bit back to colour and flag) and pads() 55-58 (cymbal bit = tom bit shifted left by 3).
Table, home 1 (lane_bit_rank, rank from low to high): Red tom 0; Yellow tom 1; Blue tom 2; Green tom 3; Yellow cymbal 4; Blue cymbal 5; Green cymbal 6. Edges: a Red note with a cymbal flag would rank 3, the same as Green tom (never produced; Chord::code throws on that flag, model.cpp 125). Kick is skipped by the caller (song.cpp 122); the formula alone would give -1.
Table, home 2 (lane_bit, bit value): Kick 0x01; Red 0x02 (a stray cymbal flag is ignored); Yellow tom 0x04, cymbal 0x20; Blue tom 0x08, cymbal 0x40; Green tom 0x10, cymbal 0x80.
Differ on: agree today. Reading the bit values low to high gives the same order as the ranks (Red, Y/B/G toms, Y/B/G cymbals). The only difference is the unreachable stray-flag input. The change that would split them is a correction to the game's lane order or a new pad: it would have to be made in song.cpp as a rank formula and in note_shuffle.cpp as bit constants, and the two files cite different sources (song.cpp says "as read from the game's lane builder", note_shuffle.cpp says "the reference's docstring").
Note: this is a claim from reading. I did not check the game addresses. The shuffle tables carry extra game facts (kick 0x01, the 5-lane and 8-lane draw ranges, the generator) that are not duplicates; only the pad order is shared.

### G7a-2. When is a drum note a cymbal? (Pro Drums gate on cymbals)
Looks like: K1 (line-level; agree today)
Homes: MidiParser::op_note, src/parse/song.cpp 903-911 (`allows_cymbals(color) && mode_pro_` then the tom-marker state);
       ChartParser::note_optype, src/parse/song.cpp 1714-1722 (N 66, 67, 68 only `if (mode_pro_)`, mapped by hand to Yellow, Blue, Green).
       Further copies outside the group, for the verifier: render/track_state.cpp 42 (`pro && n.cymbal && allows_cymbals(...)`), core/model.cpp 60-66 (note_label).
Table, home 1 (.mid): Yellow/Blue/Green with Pro on -> the cymbal flag the last tom marker left (default Cymbal; a note-on 110-112 gives tom, its note-off gives cymbal again); Pro off -> plain; Red and Kick -> plain.
Table, home 2 (.chart): Yellow/Blue/Green with Pro on and the matching N 66/67/68 line -> Cymbal; with Pro on and no marker -> plain tom; Pro off -> marker ignored; Red and Kick -> no marker exists.
Differ on: agree today for every parsed input. The default is the opposite per format (a .mid note is a cymbal unless a tom marker is open; a .chart note is a tom unless a cymbal marker is present), and that is a format fact. The split would come if Pro Drums changed what it gates, since the .mid test and the .chart test are separate lines.
Note: the second half of the .chart rule (which colours may carry a cymbal) is spelled twice there too: allows_cymbals in model.cpp (owner) and the three hand-written cases 66/67/68. Model::apply_cymbal itself does not check the colour.

### G7a-3. What happens to one tick's events when one of them is malformed, and where does the 2x bass step sit?
Looks like: K1 (same rule in both parsers; agree today)
Homes: MidiParser::run_ops and push_timestamp, src/parse/song.cpp 1095-1136 (catch ChartFileError per op and carry on; `apply_2x_bass(tick)` after the notes, before the chord is emitted);
       ChartParser::push_timestamp, src/parse/song.cpp 1746-1784 (the same catch inside run_phase; `chord_.apply_2x_bass(mode_bass2x_)` after the Notes phase, before the Pre/emit phases).
Table, home 1: an op that throws ChartFileError (duplicate note, marker with no note under it) -> that op is skipped, the rest of the tick runs; Time-phase ops in pass 1 are not wrapped (line 1161); 2x bass runs after Notes and before emit.
Table, home 2: the same four rows (the Time ops at 1827 are also not wrapped).
Differ on: agree today. They would split if one parser's phase list changed. The .mid wrapper also keeps the "marks before the dynamics tag" count correct when the kick is removed (926-931); that is a .mid-only fact.
Note: a smaller copy sits inside the .mid parser: `dynamics_enabled_ ? dyn : NoteDynamicType::Normal` appears in op_note (905) and op_kick (916). Same rule, same file. The .chart parser has no tag, so it never asks.

### G7a-4. What does the library show for a song with no title, artist or charter?
Looks like: K2 (the same idea gives different words for title, artist and charter; the stored and shown values differ)
Homes: title_or_unknown, src/parse/song.cpp 459-463, with name_or_unknown 452-455 (stored title text);
       artist_or_unknown, src/parse/song.cpp 550-552 (stored artist text);
       charter_or_unknown, src/parse/song.cpp 554-556 (stored charter text);
       display_title 535-537, display_artist 539-542, display_charter 544 (what screens show).
Table, stored value (what the scan writes, applied once in app/analysis.cpp 728-730): empty title -> "(unknown)"; an old "<unknown title>" -> "(unknown)"; empty artist -> "<unknown artist>"; empty charter -> "<unknown charter>".
Table, shown value: title empty or only tags/spaces -> "(unknown)"; artist empty, only tags/spaces, or the stored "<unknown artist>" -> "(unknown)"; charter empty or only tags -> blank (no fallback); the stored "<unknown charter>" -> shown as written.
Differ on: a charter made of only tags or spaces. The artist and title read "(unknown)", the charter reads blank. The stored charter placeholder shows as "<unknown charter>" while the stored artist placeholder shows as "(unknown)". song.h 122-137 says this is on purpose. A row stored before the placeholder existed (empty charter, if one survived the rescan) would show blank, while a new row shows the placeholder.
Note: song.h 94-139 and the comment at analysis.cpp 725-727 describe it; D48/D50/D56 are named there. Graded by phase 4, not me. Whitespace-only stored titles stay as typed in the database (title_or_unknown does not trim); only display_title trims.

### G7a-5. Which meter is in force just before a chord? (check_activations' own walk)
Looks like: K1 with a real difference on some inputs. KNOWN, line-level: finding 53 in docs/audit/2026-10-03-derivation-audit.md (lines 1086-1108).
Homes: Song::check_activations, src/parse/song.cpp 593-617 and 651-655 (its own walk over tpm_changes, one change per chord, writes `pre_tpm`);
       MeasureIndex::section_at / tpm_at, src/core/timing.cpp 109-114 (the owner).
Differ on: two or more meter changes between two chords (the walk lags by one). I re-read lines 593-618 and the finding's description still holds: the code is unchanged. The dead tempo walk (`bpm_pos`, 599 and 618) is also still there.
Note: I did not re-run the finding's two worked inputs. Not written up again.

## Parallel on purpose

**MidiParser and ChartParser (song.cpp 659-1224 and 1228-1885), including MPhase/CPhase, MAct/CAct, MOp/COp and the mop_*/cop_* constructors.** Each side knows only its own file format:
- .mid side: the pitch of each pad and the 2x kick per difficulty (read from difficulty_chart_codes), velocity 127 as accent and 1 as ghost, the marker pitches (103, 109-112, 116, 120, 126, 127), note-on/note-off meaning for tom, solo and lane markers, the dynamics tag text and the marks-before-tag count, the lane velocity ranges per difficulty, microseconds per beat to BPM, the EVENTS track text.
- .chart side: the line grammar (key = value, `TS`, `B`, `E`, `N`, `S`), the N numbers (0-4, 32, 34-37, 40-43, 66-68), the S numbers (2, 64, 65, 66), one drum section per difficulty, the Resolution and Offset properties, a phrase that ends at the first tick at or past its end, BPM as thousandths, dynamics always on.
Each of those is a fact about the file layout. Where a rule is shared the code already routes it through one helper: apply_timesig, check_timing_maps, close_sp_phrase, place_authored_fills, find_solo_sections, light_drum_lanes, sort_practice_sections, section_name_of, emit_chord_timestamp (disco flip, flam), is_disco_on_marker / is_disco_off_marker, and the Chord merge functions in core/model.cpp. The two rules that are spelled in both parsers are G7a-2 and G7a-3.
The solo end differs between formats (a .mid solo stops before its note-off tick, a .chart `soloend` includes its tick, lines 1033-1037 and 1672-1675) and is pinned by named tests, so it is a format fact, not a split rule.

**load_container_sng and load_container_srb (song.cpp 1966-1999) against the .mid/.chart loaders.** The container loaders know only how to find the notes bytes (sng_read_notes, srb_read_metadata with the stream offset) and which format they are. The srb path sniffs "MThd" when the stored name is unexpected (1995); that is a fallback for a file type the name rule cannot decide, and it is the only place that sniffs. midi.cpp 171 then checks the header again to refuse a bad file; I treat that as validation of its own input, not a second decision about the format.

**note_shuffle.cpp against the song model.** It reads the Chord API (note_list, at, insert_note, is_cymbal) and uses set_lane_flag, the owner of "which lanes can carry a flag". It owns game facts only: the seed from the first four notes, the xorshift generator, the 5-lane and 8-lane draw ranges (Pro on or off, line 127), the copy rule, and the freeze checks. It restates none of the song model's rules except the pad order (G7a-1). Mid-flow order is documented: lanes are lit before the shuffle (song.cpp 1915-1925).

**timesig.h against apply_timesig.** timesig_denominator knows the exponent range; apply_timesig knows the meter rule (ticks per measure). Each holds one fact and both parsers and midi.cpp go through them.

## Dropped leads

- "<unknown title>" and "<unknown artist>" in app/analysis.cpp and parse/song (lead from G2): the second home does not exist. The only occurrences in analysis.cpp are two comments (lines 725 and 727). Its code calls title_or_unknown, artist_or_unknown and charter_or_unknown at lines 728-730. **parse/song decides the fallback.** The literal "<unknown title>" survives only as `kOldPlaceholder` inside title_or_unknown (song.cpp 461). The display rule is separate, and G7a-4 above lists how it differs by field.

## Not read

None.

## Other notes from the read (not candidates)

- note_shuffle.cpp 142: `prev_tick` starts at 0, so a first non-kick note at tick 0 counts as being in the same chord as an earlier one. I did not check whether the reference does the same. It is not a duplication question, so I left it alone.
- Difficulty is one table with static_asserts for its size and pitch range. Outside song.* only config.h 68 holds the string "Expert" as a default and core/squeeze_rating.cpp 170 holds an unrelated "Hard" (a squeeze difficulty, K0).
