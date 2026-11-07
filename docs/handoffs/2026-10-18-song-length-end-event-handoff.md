# Handoff: use the chart's [end] event for a song's length, 2026-10-18

Status, 2026-10-18: the user answered the plan's four questions and approved the plan (`docs/superpowers/plans/2026-10-18-song-length-end-event.md`), and the build is under way; the decision is D255.

This is for a session that plans the change. Nothing is built yet, and the user has not ruled on the design. The plan needs the user's yes before any build starts (agent rule 3).

## How this came up

The user opened Iron Maiden - Rime Of The Ancient Mariner (Rh) in the Preview. The chart's folder is `C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\MMSRhino\Iron Maiden - Rime Of The Ancient Mariner (Rh)`. The scrub bar ended at about 6:17, while the time box said 13:41. The playhead sat at the bar's right edge at 6:15, and the activations after 6:17 piled up at that edge.

The cause is the chart's song.ini. It says `song_length = 377034`, which is 6:17, but the song runs 13:41. D75 says a song's length is the length its metadata states, so Hydra believed the wrong number. One length drives the scrub bar, the Paths timeline and the Preview's SP meter.

## What is already on main

Commit 0fd29fa (not pushed) adds a sanity check to `app::song_length_ms` in `src/app/song_length.cpp`. A stated length that ends before the start of the last Expert drum note, measured in chart time, is now ignored, and that note's start is used instead. A fresh reviewer signed it off. It also updated the "Song length" paragraph in CONTEXT.md. The tests are in `tests/test_song_length.cpp`.

There is one cost. To make that check, the Expert chart is always parsed. Before, it was parsed only when no length was stated. A click analyzed at another difficulty, or with 2x kick off, now parses once more with notes only. The batch doesn't work out a length, so it doesn't pay this cost.

For this chart the fix gives 13:23.8, the last drum note. The user then asked whether the chart's own end event would be better. That question is what this handoff covers.

## What was measured

All of these were measured in this session, and each gives its source.

This chart's `notes.mid` has both `[music_end]` and `[end]` text events on its EVENTS track. A throwaway MIDI reader, which used the file's own tempo map, placed them like this:

| Point | Tick | Time |
|---|---|---|
| Last PART DRUMS note-on | 774360 | 803,783 ms (13:23.8) |
| `[music_end]` | 776040 | 806,603 ms (13:26.6) |
| `[end]` | 781320 | 815,463 ms (13:35.5) |
| Audio end (time box) | | 13:41.464 |
| song.ini `song_length` | | 377,034 ms (6:17) |

In the checked-in corpus (`testdata/input`, 97 `notes.mid`/`notes.chart` files, no .sng or .srb), 52 contain the text `[end]` (a raw byte search, not a parse). 81 have a song.ini with a nonzero `song_length`.

Hydra doesn't read `[end]` anywhere today. The only "end" in `src` is the section-name table entry in `src/parse/section_names_table.cpp`.

The GuitarGame_ChartFormats docs (`docs/Chart-File-Formats/mid-format/Tracks/Global-Events.md`, repo TheNathannator/GuitarGame_ChartFormats) describe `[end]` only as marking "the end point of a song". Their .chart overview says events may appear with or without square brackets. **Nobody has checked whether Clone Hero actually stops the song at `[end]`.** Don't claim it does until that's checked.

## Questions the plan must answer

1. **Does Clone Hero use `[end]`, and how?** Find out whether CH ends play at `[end]`, at the audio's end, or at whichever comes first. Also check whether it reads `[music_end]` or `[coda]`, and whether a .chart `end` event without brackets counts. Use the decompiled Clone Hero build: see the memories on Ghidra, REA and the CH dynamic input probe. Hydra copies CH's behaviour, never its bugs (see the drums dnoflip memory).
2. **The order of sources (the user's ruling).** This changes D75 item 1, so it's the user's call. Questions to the user quote the real rule with examples from both ends (memory "Questions describe the real rule"). The option this session suggested, which the user hasn't answered: `[end]` first, then the stated metadata length, then the last note, with the 0fd29fa check applied to each. Show the user this chart's numbers from the table above as the worked example.
3. **An `[end]` before the last note.** Should it be ignored the same way a stated length is? Probably yes, but confirm against what CH does.
4. **Where the event is read.** The length is worked out per click from the parsed Song (`chart_song_length_ms`, called by `app::analysis_song_length` and by the Preview's load job). Check whether the parsers keep global text events on `Song`, for .mid, .chart, .sng and .srb (`docs/srb-format.md`, with the newer notes in the .srb memory). If they don't, the plan adds one stored fact to Song, read once (memory "Derive display from engine truth"). The reading must not add an audio open (D75 item 5).
5. **Time base.** `[end]` is in chart time already, like the last note. A stated length is in audio time and is moved by the delay or Offset. Keep that difference explicit in the code and the tests.
6. **What else moves.** Check that nothing stored depends on the length (`store::SongLength` says nothing stores it) and that no stamp in `src/store/stored_versions.h` needs a bump. Check the Preview's playback range (D48, `PreviewTransport::length_ms`), which is a separate question and should stay one. Search every copy of the rule (`tests/single_owner/rules/app.json` holds the owner rows for `song_length_ms` and `analysis_song_length`), not only the formula.
7. **Records.** Write a new D-record that amends D75. Update CONTEXT.md's "Song length" paragraph and the comments in `src/app/song_length.h`. If the user's guide says where a length comes from, update it too (public doc style applies).

## What Clone Hero does (scout findings, 2026-10-18)

A read-only scout decompiled Clone Hero v1.1 (`C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`). It got names from Il2CppDumper and decompiled with headless Ghidra. The REA tools did not load for it. RVAs below are offsets from the image base 0x180000000. The main session checked the length lines (point 2) in the decompiled output itself. The rest is the scout's reading, so check it before relying on it (agent rule 9). The scout's files are in this session's scratchpad under `ch-song-end\`: `out1.c` holds the load coroutine, `dump\` holds the Il2CppDumper output, and `proj\` holds the Ghidra project.

1. **When play ends.** `GameManager.Update` (RVA 0xC05A0) ends the song once the song clock reaches `GameManager.songLength`. The song.ini `song_length` is never read on this path.
2. **How that length is built (offline play).** The load coroutine is at RVA 0xD8D80. At 100% speed, the length is `Max(audio length, last note end)`. A usable end event then caps it: `Min(end event time, that max)`. One second is added at the end (the constant at RVA 0x3063DC0 is the double 1.0). An end event can only shorten the song. It can never make it longer than the audio or the last note.
3. **What "audio length" and "last note end" mean.** The audio length is the longest loaded audio stem, in raw audio seconds (BassAudioManager, RVA 0x36B700). The last note end is the largest `start + sustain length` across every track and difficulty, not only Expert drums (RVA 0x212E130). It is converted through the tempo map with no offset (RVA 0x212B9C0).
4. **When an end event counts.** RVA 0x212C7E0 accepts it only when its tick is at least the last note end plus one beat. One beat here is the resolution times a scale; the scale is a setting the scout did not read, likely 1.0. An `[end]` before that point is ignored, and the length falls back to `Max(audio, last note) + 1 s`. With no end event, the length is the same fallback. A second scout settled the margin. It is the chart's `Resolution` times Song Speed / 100, truncated to whole ticks (RVA 0x212C0B0, 0x210C090; the setting is named `song_speed`). The compare is `<=`, so a tick exactly on the margin counts. At 100% speed, `[end]` counts when its tick is at least the last note end plus `Resolution` ticks, which is one beat.
5. **Where it is read.** In .mid, the EVENTS text handler (RVA 0x21544E0) matches `[end]` with brackets only. In .chart, the [Events] reader is RVA 0x2163170. The first scout named 0x2151650, but that is only a line-skipping helper. The reader strips one pair of brackets, so `end` and `[end]` both count. That check is skipped when a parse-settings flag (`settings+0x2C`) is set. That flag is 0 on the play path: the load coroutine calls `FUN_18035c110(song,0,0,1,0)`, which the main session checked in `out1.c`. Only the library-scan and command-line paths set it. So in ordinary play, .chart `end` is read. The first end event wins (setter RVA 0x212E310, stored at chart +0x108).
6. **Other events.** `[music_end]` is not among Clone Hero's string literals, so it is very likely not read. `[coda]` and `coda` are read and stored, but nothing on the end path reads them.
7. **Delay and Offset.** The delay shifts when the clock starts, not the length. So the end event is in chart time. The audio length is compared in raw audio seconds without any adjustment.
8. **song.ini `song_length`.** It is stored on the song entry and used by the song list, sorting and the cache. When it is missing, Clone Hero writes the audio's length into it (RVA 0x358850). So it is usually Clone Hero's own record of the audio length, which is why it is a fair stand-in when it is right.
9. **For Rime** (the scout's arithmetic on the table above, not a game run): Clone Hero would end at `Min(815.46, Max(821.46, last note)) + 1.0 = 816.46 s` (13:36.5). It ignores the 6:17 `song_length`.

Still open. The second scout settled the .chart flag and the one-beat margin; its files are in the scratchpad under `ch-song-end2\`. Not needed for Hydra: the online-room branch, the speed constant (Hydra works at 100%), and whatever reads the coda event. Still unknown: whether the match is case-sensitive (a corpus search can settle it), and how .sng and .srb deliver end events.

The design question for the user: Clone Hero's rule needs the audio length, and Hydra never opens audio (D75 item 5). The plan must put to the user what stands in for it.

## Testing limits

Test on `testdata/input` and on this one chart. Don't run anything over the whole library without the user's yes in chat (agent rule 8). Check the scrub bar in the real app on a scratch copy before merge (memory "UI changes need a real-app check").

## State of this session at handoff

The hook's journal listed reviewer agent `a3c6322a0540ab82d` as unfinished, with last tool call SubagentHandback at 00:12. It handed back its review of 0fd29fa, and that commit is on main. Nothing else is running or was launched.


---

## Addendum: what Clone Hero ChartLength walks (scout, 2026-10-18)


Task id: song-length-end-event. Read-only scout. Nothing in the code was changed.

### The plain answer

ChartLength (RVA 0x212E130) asks every instrument track for its "last gem end", and takes the biggest. The per-track helper (RVA 0x215FED0) walks one list only: the track's gem notes (the notes with Green, Red, Kick drum and so on). It does not walk the vocal notes. It does not walk the phrase or marker lists (star power, solo and the like).

So vocals do not count in ChartLength. That part is inferred from the code, not quoted from it; see "What is proven and what is inferred" below.

The rule around it is not the one in the brief. The code that uses ChartLength to set the song's length (RVA 0xD8D80) does Min(E, Max(audioLength, N)) plus 1 second. It never reads a "stated length" in that branch. The brief's rule needs a second look (see "The surrounding rule").

### How the track list is laid out (from the Il2CppDumper output)

Image base 0x180000000. Names in the game are obfuscated, so I give each class a short label and its obfuscated name once.

- Chart class (obfuscated `ˁʿʺʲʲʹˀʴʾʻʻ`, TypeDefIndex 8538). Field 0xD0 is `List<Track>`, one entry per instrument. ChartLength is its method `ʿʼʿʿʵʵʲʿʷˁʻ(bool)` at RVA 0x212E130. Field 0xB0 is a double (the chart's offset). Field 0x120 is a `Nullable<long>` that caches the answer.
- Track class (obfuscated `ʷʿʽʽʵʻʶʹʼʼʺ`, TypeDefIndex 8661). Field 0x58 is the instrument enum, which includes `Vocals = 13`, `Drums = 6`, `ProDrums = 9`. The track holds these lists:
  - 0x60: `List<GemNote>` (obfuscated `ʾʳʿʽʸʸʷʾʼʴʿ`). Its enum has None, Open, Green, Red, Yellow, Blue, Orange, B1-B3, W1-W3, KickDrum, RedDrum, YellowDrum, BlueDrum, GreenDrum, OrangeDrum. Its flags enum has Strum, Hopo, Tap, DoubleKick, Tom, Cymbal, DiscoNoFlip, Disco, Flam, Ghost, Accent. These are the playable notes.
  - 0x68: `List<VocalNote>` (obfuscated `ʷʲʷʴʹʸʲʲʿʹʾ`). It has an int pitch field at 0x20 and an enum at 0x24 with None, Vocals, HarmonyOne, HarmonyTwo, HarmonyThree, HarmonyAll.
  - 0x70, 0x78: lists of two classes with no fields of their own (`ʵʷʳˁʶʺʼʲʵʴʴ`, `ʸʵʵʾʿˀʺʽʲʾˁ`). They only carry a start and a length.
  - 0x80: a class with a difficulty enum and a bool (`ʳʷʸʽʳʹʶˁʺʳʿ`).
  - 0x88: a class with one bool (`ʲʵʺʹʿʵʹʷʿʲʻ`).
  - 0x90: a class with a difficulty enum and a string (`ʲˀʿʹʹʻʵʺʴʻʷ`), which looks like per-difficulty text events.
  - 0x98: a dictionary of lists.
- All of these note and marker classes inherit from one base class (obfuscated `ʼʿʷʵʲʶʽʵʿʳʷ`), which adds a `long` at 0x18 (the length). Its own base (`ʺʷʽʿʲʸˀʿʳʳʺ`) adds a `long` at 0x10 (the start tick). So every note is "start at 0x10, length at 0x18".

I did not name what the lists at 0x70 to 0x90 are. The scout did not need to, because the helper does not read them.

### The two functions, quoted

ChartLength, RVA 0x212E130 (decompiled; Ghidra 12.1.3, `FUN_18212e130`):

```
if (*(char *)(param_1 + 0x120) == '\0') {          // cache empty
    ...
    FUN_18071c480(&uStack_50,*(longlong *)(param_1 + 0xd0),uRam0000000183cd12c0);   // enumerate List<Track> at +0xD0
    ...
    while( true ) {
      cVar2 = FUN_1814fb3b0(&uStack_38,uRam0000000183cc0630);                         // MoveNext
      if (cVar2 == '\0') break;
      if (lStack_28 != 0) {
        lVar3 = FUN_18215fed0(lStack_28,0);                                           // per-track helper
        if (lVar4 < lVar3) { lVar4 = lVar3; }                                         // keep the maximum
      }
    }
    ...  *(undefined4 *)(param_1 + 0x120) = ...                                        // store in Nullable<long> at +0x120
}
lVar4 = FUN_1808689b0(param_1 + 0x120,uRam0000000183d03270);                          // read cached value
if (param_2 != '\0') { lVar4 = lVar4 + (longlong)*(double *)(param_1 + 0xb0); }       // optional + chart offset
return lVar4;
```

So ChartLength is the maximum over every track in list 0xD0, then optionally plus the chart's offset (field 0xB0). It is format-agnostic: it works on the already-loaded tracks, whether they came from .chart or .mid.

The per-track helper, RVA 0x215FED0 (`FUN_18215fed0`):

```
if ((*(longlong *)(param_1 + 0x60) == 0) || (lVar2 = *(longlong *)(param_1 + 0x60), *(int *)(lVar2 + 0x18) == 0)) {
    return 0;                                                  // empty gem list gives 0
}
...
FUN_180462d10(&lStack_18,lVar2);                               // copies the list's items to an array
...
lVar3 = *(longlong *)(lStack_18 + uVar5 * 8);                  // each note
lVar3 = *(longlong *)(lVar3 + 0x18) + *(longlong *)(lVar3 + 0x10);   // length + start = note end
if (lVar3 <= lVar2) { lVar3 = lVar2; }                         // running maximum
```

It reads only the list at track + 0x60, and for each note takes start (0x10) plus length (0x18). A track whose gem list is empty returns 0.

### What is proven and what is inferred

Quoted from the code (proven): the helper reads only list 0x60. It takes start plus length. ChartLength takes the maximum over the list at chart + 0xD0. The dump says 0x60 is `List<GemNote>`, and the GemNote enums are the gem colours and flags.

Inferred, not shown: that vocal notes never also land in list 0x60. My evidence is that the track method at RVA 0x215E970 (`ʻʶˁˀʷʼʻʵʾʶʼ(int, VocalEnum, long, long)`) builds a VocalNote with constructor RVA 0x20E78A0 and appends it to the list at track + 0x68, not 0x60 (decompiled in this scout: `lVar2 = *(longlong *)(param_1 + 0x68)` at the top). I could not find who calls 0x215E970. A scan of every `call` and `jmp` in the code section finds no direct caller, though the same scan does find the callers of 0x215C560, so the scan works. The loader probably reaches it some other way. I did not read the vocal loader, so "vocals are only ever added to 0x68" rests on the method's shape, not on seeing the vocals loader run.

Also inferred: that the lists at 0x70 to 0x98 are phrase or marker data. I only know they are not the list the helper walks.

Not checked at all: how the game's .chart loader fills the gem list (for example whether N 5 or N 6 flag lines count as notes with length 0). ChartLength sees only loaded tracks, so the check belongs to the loader.

### The surrounding rule

The brief said: stated length S gives Min(E, S); else E; else N; plus 1 second. I read the one caller that turns ChartLength into the song's length: RVA 0xD8D80, around the call at 0x1800D9354. Its disassembly (from my own Ghidra run) reads:

```
1800d9354  CALL 0x18212e130        ; ChartLength(false)  -> RAX = chart ticks
1800d9362  CALL 0x18212b9c0        ; ticks -> seconds (tempo map)       -> XMM8 = N seconds
1800d93a5  MULSD XMM6,XMM8         ; XMM6 = speed * N
1800d93aa  MOVSD XMM8,[RCX + 0x40] ; RCX = BassAudioManager.instance; +0x40 = "audioLength" (double)
1800d93cb  MOVAPS XMM0,XMM8        ; first argument  = audioLength
1800d93cf  CALL 0x181b51dc0        ; Max(audioLength, speed*N)
1800d93ea  CALL 0x18212c7e0        ; is the [end] event usable?
...
1800d941d  CALL 0x18212b9c0        ; end-event tick -> seconds
1800d944c  CALL 0x181b51fb0        ; Min(speed*E, previous)
1800d946f  ADDSD XMM9,[0x183063dc0]; + 1.0 second (the constant at 0x183063DC0 reads 1.0)
```

I decompiled the two small functions: 0x1B51DC0 returns the second argument when the first is less than or equal to it, so it is Max. 0x1B51FB0 returns the second argument when it is less than or equal to the first, so it is Min.

The class behind the static 0x183CBBE58 is `BassAudioManager` (script.json: address 63684184 is `BassAudioManager_TypeInfo`), and its field 0x40 is `public double audioLength`. So in this branch the game uses:

length = Min(E, Max(audioLength, N)) + 1 second, or Max(audioLength, N) + 1 second when the [end] event is not usable.

"Usable [end]" is the function at RVA 0x212C7E0: the chart has an end-event object (field 0x108, its tick at +0x10) and `ChartLength(true) <= end tick`. The end event is set by 0x212E310.

Two things in this branch do not match the brief's rule.

1. The brief's S ("stated length") does not appear. The slot where S would go holds `audioLength`, the length of the loaded audio. I did not trace how audioLength is set, so I do not know whether it follows a song.ini stated length. The first job for whoever picks this up is to find who writes `BassAudioManager.audioLength` (field 0x40).
2. The combine is Max with N, not "S replaces N". If audioLength is shorter than the last note, the game still uses N. The brief's rule (and Hydra's, below) drops a stated length that ends before the last note and then uses E, not N.

A second path exists. The branch at the top of 0xD8D80 only runs the calculation above when a flag at +0x30 is true. The flag comes from two functions on `CHNetManager` (0x180145FE0, 0x180146A90). When the flag is false, the game reads the length already stored in the song metadata (`GlobalVariables.currentSongMetadata`, field 0x30 of that object) and does not compute anything. This is probably where a stored or stated length is used. I did not decode those two functions, so I cannot say when the flag is false.

Three other callers of ChartLength set the song list's length column: RVAs 0x285830, 0x289460 and 0x28A880. Each calls ChartLength, converts to seconds, multiplies by a constant, and stores an int at song entry + 0x80. They do not apply an end event. I did not decode the multipliers beyond 0x285830's (832.0, which I do not understand as a unit and leave unexplained).

### Hydra's code against this

Hydra's `MidiFile::last_note_end_tick` (src/parse/midi.cpp, line 476) takes the latest note on the drum track and on each other instrument track, once per instrument name. The instrument list (src/parse/midi.h, `kOtherInstrumentTrackNames`) includes `part vocals`. For the vocals track only, it also counts pitches 36 to 57 (`last_low_note_tick`). So Hydra counts vocal notes in the song's last note.

That does not match ChartLength. By the helper, a vocals track contributes nothing (inferred, see above). The effect: a MIDI where the vocals run past the last gem gets a longer N in Hydra than in the game. That moves N and, when there is no [end] event and no usable stated length, moves the song length.

The gem part matches. Hydra counts only gem pitches 58 to 102 on instrument tracks (`is_gem_pitch`, midi.h line 144) and leaves out the markers from pitch 103 up (solo, star power, and so on). That agrees with the helper reading only the gem list.

Hydra's `.chart` path (src/parse/song.cpp, lines 1823 to 1828 and 1916 to 1919) takes the largest `tick + length` of every `N` line in every section. It does not treat vocals specially, since .chart vocals are `E` events, not `N` lines. I did not check how the game's .chart loader fills the gem list, so I cannot say the two agree there.

Hydra's rule is in `song_length_ms` (src/app/song_length.cpp, lines 37 to 65): if a stated length exists and is not before the last note, length = Min(E, S) or S; otherwise E, otherwise N; plus `kSongEndPadMs` (1000 ms). The +1 second matches the constant at 0x183063DC0. The "usable E" test in Hydra (`end_event_counts`, line 31) needs the end event to be at least one beat (`tick_resolution()`) past the last note. The game's test (0x212C7E0) is `ChartLength(true) <= end tick`, with no extra beat. I did not find where a beat comes from in the game's code. The two tests differ by up to a beat, and the game's version also adds the chart offset (field 0xB0) on its side.

### What to do next

1. Decide on vocals. If the inference holds, Hydra should stop counting the vocals track in `last_note_end_tick`. First confirm that the game's MIDI vocal loader only writes to list 0x68. Find the loader's route to 0x215E970 (no direct callers exist).
2. Find who writes `BassAudioManager.audioLength` (field 0x40). That decides whether the brief's "stated length" is that slot and whether Min(E, Max(S, N)) is the right form.
3. Decode 0x180145FE0 and 0x180146A90 to learn when the flag at +0x30 is false and the stored metadata length is used instead.
4. Check the "usable E" test: game uses `ChartLength(true) <= end tick`; Hydra uses end tick at least one beat past the last note.

### Where my work files are

My scratch folder is `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\b71e9156-719a-4a9c-b8f0-1b1b79571011\scratchpad\w`. It holds a copy of the earlier Ghidra project, the decompiled output (`q1.c`, `q2.c`, `q3.c`), the disassembly of 0xD8D80 (`all1.txt`) and a call-site scanner (`callscan.py`). The earlier scouts' folders were not modified.

## Addendum: do vocals reach what ChartLength walks? (follow-up, same day)

Plain answer: almost certainly no, and now with direct evidence for the .mid loader. The Clone Hero .mid loader sends vocals to a chart-wide lyric list, not to any track's gem list. Two things are still not proven, listed at the end.

### 1. Who calls the vocal-note adder at 0x215E970 and who writes list 0x68

Still unknown, and the search says why. I searched the whole code section (`il2cpp` and `.text` sections of GameAssembly.dll) for `call` and `jmp` instructions that land on 0x215E970, on the VocalNote constructor 0x20E78A0, and on the matching adders. A scan for `lea`/`mov` references to those addresses found none either. Results:

- 0x215E970 has no direct caller. The VocalNote constructor 0x20E78A0 has exactly one caller: 0x215E970 itself (site 0x215E9F3). So nothing else builds a VocalNote, and list 0x68 is written only by 0x215E970.
- The only references to 0x215E970 are an 8-byte pointer in a method-pointer table in `.data` (RVA 0x3E90FF8; neighbours 0x3E91000 and so on are the Track class's other methods, in dump order). That table is for the runtime's own use (reflection and delegates), not for ordinary calls. I found no code that reads it.
- This does not prove 0x215E970 is dead. A delegate or reflection call would not show up. But the same is true of the gem adder 0x215BBF0 (also no direct caller; pointer at 0x3E91038). The gem notes reach list 0x60 by another route, so the missing caller tells us little by itself. What matters is the next point.

### 2. Where the loaders decide which list a note goes into

The .mid loader is the group of static methods at RVAs 0x2154180 to 0x21579B0 (class obfuscated `ʶʲʻʾʾʺʴˀʷʼˀ`). Its entry, 0x2155C00, switches on the track name (Hydra's `midi.h` already cites 0x2155F30 and 0x2155F8D in it) and calls one method per instrument. I decompiled all of them (`FUN_182155c00` and the methods it calls).

Quoted from `FUN_182155c00`: the Vocals case calls `FUN_1821579b0(uVar11,param_2,0xd,0)` (0xd is the Vocals enum value); the gem instruments call `FUN_182156990(uVar11,param_2,N)` and `FUN_182155050(uVar11,param_2,6)` and the like.

The gem methods (0x2154180, 0x2155050, 0x2156990) call the gem adder `FUN_1820cec60` (RVA 0x20CEC60: it builds a GemNote with constructor 0x214D240 and appends it to the track's list at +0x60). Call sites: 0x215439E, 0x215565F, 0x2157002, 0x21576A0, all inside those gem methods.

The vocals method, 0x21579B0 (decompiled; the MIDI note range it tests is `(byte)(c - 0x24) <= 0x30`, that is pitches 36 to 84, and the phrase markers are 105 and 106 `(0x69, 0x6a)`), never calls 0x20CEC60. Everything it writes goes through one function, `FUN_182152cc0(chart, kind, tick, text)`, with kind 0, 1 or 2:

```
FUN_182152cc0(lVar2,0,plVar14[2],_DAT_183cb3e70,0);   // kind 0
FUN_182152cc0(lVar2,1,*(undefined8 *)(lVar4 + 0x10),_DAT_183cb3e70,0);   // kind 1
FUN_182152cc0(lVar2,2,lVar3,plVar10[4],0);   // kind 2, with the lyric text
```

`lVar2` is the chart (`*(*param_2 + 0x30)`). The function `FUN_182152cc0` (0x2152CC0), decompiled, reads `lVar2 = *(longlong *)(param_1 + 0xf0)`, builds an object with `FUN_182141ab0` (RVA 0x2141AB0) and appends it to that list. In the dump, Chart field 0xF0 is `List<ʹʿʸʷʲʻˁʺʿʷʵ>`, and that class is the lyric event: its enum is `PhraseStart = 0`, `PhraseEnd = 1`, `LyricText = 2`, plus a string at 0x18. The kinds match. So in the .mid loader, vocals go to the chart-wide lyric list at Chart + 0xF0. ChartLength walks the track list at Chart + 0xD0 and then only list 0x60 of each track. List 0xF0 is not reached.

The vocals method also calls `FUN_1820cfd40` (RVA 0x20CFD40, at 0x2157EDD) with instrument 0xd and difficulty 3. That function is "get or create the track for this instrument and difficulty" in the track list at Chart + 0xD0 (it indexes `*(param_1 + 0xd0)` and creates the track with 0x215E6E0 when missing). So a Vocals track can exist in the list ChartLength walks, but the vocals method puts no note in it, so the helper reads an empty 0x60 and returns 0 for it.

Other loaders: `FUN_182152cc0` is also called from 0x21524FC (in the function starting 0x2151650) and from 0x2163E3C (in the function starting 0x2163170). That means at least two more loaders send lyric events to the same Chart-level list. I did not decompile them to confirm their format. 0x2163170 and 0x2151650 are likely the .chart text loader and the container loader; that is a guess from their position and what they call, not a checked fact. Three more functions (0x20E45C0, 0x20E53D0, 0x20E69A0) read a binary note stream and add GemNotes straight into list 0x60 of whatever track they are given. I did not check what track those get, so I cannot rule out a binary format that stores vocals as gems.

### 3. Does the helper skip or special-case any track type

Proven by quoted code: no. The ChartLength loop (quoted above) calls `FUN_18215fed0(lStack_28,0)` on every track and keeps the maximum. It never reads the instrument byte at track + 0x58. The helper (quoted above) reads only list 0x60 and has no instrument test either. So a Vocals track is walked like any other; it contributes whatever is in its 0x60 list, and the .mid vocals method puts nothing there.

### What is proven and what is not

Proven by quoted code or decompiled output:

- ChartLength and the helper do not special-case any instrument, and they read only the gem list (0x60).
- The .mid vocals method (0x21579B0) writes only to the chart-level lyric list (Chart + 0xF0, via 0x2152CC0), and never calls the gem adder.
- The VocalNote constructor has one caller, 0x215E970, and 0x215E970 has no direct caller.

Still unproven:

- The .chart text loader and the container loader: they also call `FUN_182152cc0`, but I did not read them, so I cannot say they never put a vocal-derived note in a track's gem list.
- The three binary-stream loaders (0x20E45C0, 0x20E53D0, 0x20E69A0): they add GemNotes to whatever track is passed in; I did not check whether it can be a Vocals track.
- An indirect call (delegate or reflection) to 0x215E970 cannot be excluded by a call-site scan. Even if it exists, it would fill list 0x68, which the helper does not read.
- Whether some step copies lyric or phrase data into list 0x60 later (for example in the commit method 0x215C320 and its helpers 0x215DA70, 0x215E280, 0x215ED20, 0x215E120, 0x215C040, 0x215B8C0). I grepped those for writes to list offsets and found only sorts and fix-ups on list 0x60 (reading existing gems), no construction of new notes from lyric data. That is a grep of decompiled output, not a line-by-line read.

So for MIDI files: vocals do not reach what ChartLength walks (proven for the .mid loader path). For .chart and the container: very likely the same, not proven.

My added work files: `callscan.py`, `refscan.py`, `ripscan.py`, `rangescan.py`, `pdata.pkl` (function-start table from `.pdata`) and the decompiled output `ld.c`, `cv.c`, `ad.c`, `cf.c`, `gm.c`, `bd.c`, `tk.c` in the scratch folder named above.

### Library count: how often do vocals in N change the song length (scout vocals-length-count, 2026-10-18)

Short answer: one chart in 19,432. A read-only Python script (not Hydra) read every notes.mid and notes.chart under C:\Clone Hero, the chartfolder in C:\Program Files\Hydra\hydra_settings.ini, and followed Hydra's rule from midi.cpp, song.cpp and song_length.cpp. It scanned 19,432 folder charts (15,037 .mid, 4,395 .chart), with no parse errors. It skipped 54 .sng and 30 .srb containers. Hydra's library db holds 19,512 charts, which is those 19,432 plus the 84 containers minus 4. Its stated_length_ms agrees with the script's song.ini reading for all 19,428 charts both have.

6,020 .mid charts have a PART VOCALS track. In 846 of them the vocal notes run past the last instrument note, so N differs. Only one of those 846 changes the final length by more than 1 ms: Megadeth - Sleepwalker (Z), under synchotic\Sync Charts\BirdmanExe Drive\Grinnz. Vocals push N to tick 482948 (instruments end at 480060), the stated 359.2 s ends before that, so it is dropped and the [end] event (tick 487679) decides: 6:05.7 with vocals, 6:00.2 without, 5.4 s apart. .chart files never count vocals: N reads only N lines, and a .chart has no vocal notes.

Where N sits: 12,726 charts are decided by a stated length capped by an accepted [end], 6,552 by a stated length alone, 150 by a dropped stated length (62 fall to [end], 88 to N), 4 by N with nothing else, none by [end] alone, none without a length. So N is the answer for 92 charts and a gate for the rest. Of the 150 dropped, 149 are dropped even without vocals. The script's final-length arithmetic was not checked against Hydra's own output. Full table and per-branch counts: C:\Users\Patrick\AppData\Roaming\Claude\side-session-notes\local_3144cf42-73bd-40f6-82a2-bac48c1f4ab6\vocals-length-count.md.
