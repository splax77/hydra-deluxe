# Handoff: use the chart's [end] event for a song's length, 2026-10-18

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
