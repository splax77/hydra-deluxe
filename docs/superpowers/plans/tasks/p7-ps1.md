Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task PS1: song stems (findings 74 and 101)

Task id: PS1. Base: main at 81a2519. Branch: claude/p7-ps1 (worktree `.claude\worktrees\p7-ps1`, made as the preamble says).

Plan row: `docs/superpowers/plans/2026-10-04-phase-7.md`, wave 1 table, "PS1 Song stems". Decision: D51 Q21 in `docs/audit/2026-10-04-phase-7-questions.md`: preview clips are left out of the song mix everywhere, and one "is this playable audio" rule, `audio::sniff_format`, decides. Finding texts: `docs/audit/2026-10-03-derivation-audit.md`, headings `#### 74.` and `#### 101.`.

## Goal

Two questions about a chart's audio get one answer each. "Are these bytes playable audio" is answered by the decoder's sniff, which is what must actually accept the bytes. "Is this file a song stem" is answered by one predicate that every source kind calls, so a .sng's preview clip is left out the same way a loose folder's already is. This is Preview-only. No score, path or stored record moves.

## What the code does today

`looks_like_audio` in `src/app/preview_source.cpp` (around line 167) takes any bytes that start with "OggS" or "RIFF", plus fLaC, an ID3 tag or an MP3 frame sync. `audio::sniff_format` in `src/audio/decode.cpp` (around line 70; declared in `src/audio/decode.h`, with a `std::vector<uint8_t>` overload) is stricter: RIFF needs "WAVE" at byte 8, and OggS needs "OpusHead" or "vorbis" somewhere in the first 64 bytes. So a RIFF holding video, or an Ogg carrying Theora or Speex, counts as audio to one and not the other. `srb_audio_from` (the body behind `extract_srb_audio`) calls `looks_like_audio` twice: once on each trailing DEFLATE stream (around line 242), and once on each decrypted blob (around line 289). Around line 255 it returns early as soon as the DEFLATE walk found any stem, so an odd stream that only `looks_like_audio` accepts stops the walk before the real, encrypted audio is reached, and the Preview plays silence.

`find_loose_audio` (around line 180) skips a file whose base name, lower-cased, is "preview". `sng_audio_from` (around line 137) keeps every entry whose name passes `is_audio_filename`, preview or not. The header comment on `find_loose_audio` (`preview_source.h`, around line 139) documents the exclusion for loose folders only.

## What changes

**Owner for the bytes question: `audio::sniff_format`, unchanged.** `looks_like_audio` keeps its name and both callers, and its body becomes one line: the sniff's answer is not `AudioFormat::Unknown`. Include `audio/decode.h`. Its header comment says it asks the decoder's rule, so what the extractor keeps is exactly what the decoder can open. With the stricter rule, an odd OggS or RIFF stream no longer counts as a stem, so the early return around line 255 is not taken for it and the walk goes on to the encrypted section. Check that the early return still reads right once an untagged stream can fall through; its comment should say a stream counts only when the decoder would open it.

**Owner for the file question: one `is_song_stem(filename)`** in `preview_source.cpp`, declared in the header beside `is_audio_filename`. It is true when the name has an audio extension (`is_audio_filename`) and its base name (`stem_of`) is not "preview" in any case. `find_loose_audio` and `sng_audio_from` both call it in place of their own checks. A .srb has no file names, so its path is untouched. Update the header comments on `find_loose_audio` and `extract_sng_audio` to say both leave out a standalone "preview" clip, and say why in one line: it is a short clip, not part of the song.

No scan row: `tests/test_single_owner.cpp` is not yours. The pins below guard both rules.

## Owned files (only these may change)

- `src/app/preview_source.cpp`, `src/app/preview_source.h`
- `tests/test_preview_source.cpp` (the plan's filter points at it, and every case below lives there)

Not yours: `src/audio/decode.cpp`/`.h` (PS1 only calls it), `src/audio/stem_reader.cpp`, `tests/srb_util.h`, `CMakeLists.txt`, `CONTEXT.md`, ADR 0019.

## Test cases to add or change (in `tests/test_preview_source.cpp`)

Write each red first, then green. Use `bytes_of`, `make_sng` and `make_srb` as they are; add no new fixture helper.

1. "is_audio_filename / looks_like_audio recognize the formats" (around line 160) is re-pinned to the decoder's rule. Its `OggS\x00\x02` check flips to false. Add: OggS with "OpusHead" inside the first 64 bytes is true; OggS with "vorbis" inside the first 64 bytes is true; "RIFF....WAVE" stays true; RIFF with something other than WAVE at byte 8 is false. fLaC, ID3, the MP3 frame sync, PNG and empty keep their answers.

2. New: `is_song_stem: an audio file that is not a standalone preview clip`. Pins from finding 101's table: "song.ogg" true; "Preview.OGG" and "preview.opus" false; "preview2.ogg" true; "album.jpg" and "notes.chart" false.

3. "extract_sng_audio: audio entries come back XOR-demasked" (around line 196): its `make_sng` list gains a `preview.ogg` entry carrying audio bytes. The case still expects exactly two stems, "song" then "drums", and `resolve_preview_source` still reports two. That is the red-first proof of 101.

4. "extract_srb_audio: trailing audio streams inflate; art is skipped" (around line 222) keeps its one-stem pin, so its OggS fixture gains a codec tag (put "OpusHead" or "vorbis" inside its first 64 bytes). Add one check to it, or a new case `extract_srb_audio: a stream with an audio magic but no codec is not a stem`: a bundle whose trailing streams are a bare "OggS" stream first and a tagged Ogg second gives exactly one stem, the tagged one. That pins the walk continuing past the odd stream (finding 74's effect).

5. Every other `OggS ...` byte string that reaches an .srb fixture gains a codec tag: "readonce.srb" (around line 441), "halves.srb" (around line 473) and the `eq_*.srb` cases (around lines 508 to 509 and 527 to 529). Those vectors are shared with .sng and loose fixtures, where the name decides, so tagging them there is harmless. Loose fixtures that only need a name ("OggS song", "OggS fallback audio") need no tag.

Existing cases that must pass unchanged, as proof nothing else moved: "find_loose_audio: stems beside the notes, preview and art excluded", the fallback case "a .srb with no extractable audio falls back to a loose file beside it", "containers pass the difficulty through to the chart inside", the delay and Offset cases, "reads a .sng or .srb from disk once", "the song and stem halves share one container read", and "container charts give the same Song, stems and offset as the chart inside".

The only new literals you may type are the magic strings above and the file names in finding 101's table. No new number of any kind.

## Test filters you may run

- `build-cpp\Release\hydra_tests.exe -sf=*preview_source*`

Nothing else. Never the full suite.

## Not in this task

- `sniff_format`'s own rule (the 64-byte search, the two codec tags) stays as it is; ADR 0019 already records it.
- Decoding, mixing and the damaged-stem behaviour are AU1's and PV's.
- `is_audio_filename` answers a different question (the extension) and is not a copy; it stays and `is_song_stem` calls it.

## Done when

- `looks_like_audio` has no magic-byte list of its own; it returns the sniff's answer. `find_loose_audio` and `sng_audio_from` both call `is_song_stem`; no `"preview"` comparison remains outside it.
- The cases above are green, and every existing case in the filter passes with no pin edited beyond the codec tags and the preview.ogg entry named here.
- `git diff --stat 81a2519..HEAD` lists only the three owned files.
- No score, path or stored record changes; the results stamp stays "2.1.0".

## Open questions

- Link shape. `preview_source.cpp` is in the `hydra_core` library and `sniff_format` is in `hydra_audio`, which already depends on `hydra_core` and is kept out of the CLI tools on purpose (CMakeLists.txt comment near line 204). The call links today because only `src/ui` and `src/audio` pull `preview_source` in, and both link `hydra_audio`; the CLI tools never reference it. So no CMake change is needed for this task, but the dependency is now implicit. `CMakeLists.txt` and `decode.cpp` are not PS1's; if the main session wants the sniff moved into `hydra_core` so the dependency is declared, that is a follow-up.
- The header's top comment (`preview_source.h`, lines 1 to 18) says nothing about preview clips. Recommended: one clause under the loose-folder bullet; not a display change.

## Commits

One commit, trailers `Task: PS1` plus the preamble's others. Report as the preamble says: owner, file, one sentence per change, each case's red line and green result, the diff file list.
