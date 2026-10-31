# Survey: the history record for the Clone Hero reference

Written 2026-10-30 by planning agent a78fb6e1dafcb3beb (task `ch-reference-plan`, session 3d58c2ba). Read-only; nothing in the repo or the memory folder was changed. Every number below says where it came from. Sizes are file lengths in bytes from `Get-ChildItem` on the session-cards folder; nothing was estimated.

## 1. The session cards

### How I made the cut

`index.md` has 432 lines (`Measure-Object -Line`), one per carded session, 416 card files in `cards\`, and 1,248 files in `condensed\` (three per session: `.condensed.md`, `.meta.json`, `.user_text.json`). All condensed text together is 15,120,880 bytes.

I grepped `index.md` (case-insensitive) for every tag and word the brief named plus the related tags I found in `tags.txt`: `clone-hero`, `hit-window`, `squeeze`, `early-fill`, `ch-probe`/`ch_probe`, `note-shuffle`, `star-power`, `scorestats`, `srb`, `dynamics`, `disco`, `delay`, `offset`, `calibration`, `fill-spawn`, `generated-fill`, `fill-rule`, `authored-fill`, `star-cutoff`, `sqin-sqout`, `disco-flip`, `2x-kick`, `backend-leeway`, `sp-end-history`, `transfer-scale`, `chart-reading`, `drum-track`, `ch_mcp`, `frida`, `il2cpp`, `ghidra`, `autoplay`, `bot`. That matched 170 index lines. About 55 of them are noise (hooks, CI, performance, release sessions whose summary happens to contain "offset" or "bot" or a squeeze tag because a Hydra display changed). I dropped those. I also added 15 sessions that no tag matched but a memory note's `Sessions:` line names (for example 2b845aef, 7bd3ac91, 4908325e).

That leaves 114 sessions, which I split in two tiers. The brief's "about 53" is the clone-hero tag alone plus a few; the wider cut is what a scout actually has to read, because the squeeze and fill facts live in sessions tagged `squeeze` or `early-fill-e`, not `clone-hero`.

Tier A (77 sessions) is where a Clone Hero fact was found or ruled on: game code read, a probe or live run, a format decoded, a video measured, or the user ruling on a game-vs-Hydra split. Tier B (37 sessions) applied Hydra's rules to one song or one display; they hold no new game fact that I could see from the index line and the memory notes, but they may hold a ruling, so a scout skims their cards and opens the condensed file only if the card's Rulings section names the game.

Totals (from the PowerShell sum over the file lengths):

| Tier | Sessions | Condensed bytes | Condensed lines | Card bytes |
|---|---|---|---|---|
| A | 77 | 2,574,566 | 21,428 | 308,820 |
| B | 37 | 762,154 | 6,407 | 119,107 |
| Both | 114 | 3,336,720 | 27,835 | 427,927 |

Area numbers in the tables are the brief's seven: 1 chart reading, 2 note list, 3 timing, 4 fills, 5 Star Power, 6 scoring, 7 game internals and tools.

### Tier A: sessions that found or ruled on a game fact

| Date | Id | Topic (one line) | Areas | Condensed bytes |
|---|---|---|---|---|
| 2026-08-16 | cdf10e6d | .srb container decoded empirically; Hydra reads the 30 bundled songs | 1 | 12,327 |
| 2026-08-16 | f1784fd5 | SP length is measure-based; frontend timing transfers to the SP end (checked against DragonDelgar's numbers) | 5 | 35,408 |
| 2026-08-17 | e61236dc | Swim to the Moon video: solo bonus paid at solo end, per-note SP values, score curve method | 5, 6 | 36,575 |
| 2026-08-20 | c73697df | Squeeze budget W(1+r) shipped as v1.5.0 | 5 | 63,992 |
| 2026-08-20 | 9f5a3914 | Dumpweed: deact node and mid-SP phrase extension, proved by the FC video | 5 | 32,843 |
| 2026-08-23 | 4a6a3538 | .srb audio: loose-audio fallback | 1 | 19,725 |
| 2026-08-29 | 6e7764db | Timing and backend limits (hit window in settings) | 3 | 15,159 |
| 2026-08-29 | fd73e1ee | Backend leeway: user rejects "safe by 7 ms" | 5 | 17,528 |
| 2026-08-29 | 93d37c20 | Enigma Machine: what SqIn/SqOut are, 500 ms candidate window | 5 | 26,995 |
| 2026-08-30 | d69fdea1 | CH 1.0 vs 1.1 fill deadline; user chose "85 ms both sides" | 4 | 17,671 |
| 2026-08-31 | 13441c5f | SP meter in the Preview (meter and bar size) | 5 | 22,897 |
| 2026-09-02 | a28ef5f2 | SP bar behaviour, SP end history | 5 | 23,417 |
| 2026-09-02 | 8b24db8f | SP end reconstruction with no backend rows | 5 | 12,570 |
| 2026-09-02 | 219bf8c2 | FC video workflow built; loss kinds | 5, 6 | 39,752 |
| 2026-09-03 | d46444c2 | Desecration Day: a CH 1.0 run starts a fill 1.1 rejects (live evidence for the 1.0 deadline) | 4 | 7,949 |
| 2026-09-05 | f520253b | Colors album on CH 1.0, priced with --legacy-fills | 4 | 9,729 |
| 2026-09-06 | 528e137d | Path scoring: squeeze-out pricing checked by hand | 6 | 57,621 |
| 2026-09-15 | 450488b5 | .srb audio extraction for the Preview | 1 | 245,706 |
| 2026-09-15 | 23b23b12 | Cap-clamped hit window and the squeeze anchor | 3, 5 | 42,250 |
| 2026-09-16 | 49d07b9c | Cap-clamp squeeze anchor warning | 3, 5 | 24,153 |
| 2026-09-17 | 3ee237e9 | Static RE of the drum hit window: FUN_1820ddda0 returns with no clamp; cap question open | 3 | 57,197 |
| 2026-09-18 | 28e6f29d | Design of the ch_probe debugger harness | 7 | 32,643 |
| 2026-09-18 | e59042f9 | ch_probe built (constants.py, engine.py, experiments) | 7 | 15,069 |
| 2026-09-21 | 35a607e6 | E fills range filter (early fill vs hit window) | 4 | 15,272 |
| 2026-09-21 | 1136a257 | Dynamics tab and stored accent/ghost counts | 2 | 38,036 |
| 2026-09-25 | 2a88c065 | Derivation fixes; song.ini delay vs .chart Offset measured from a recording | 3 | 78,101 |
| 2026-09-25 | 8157f2dc | play_chart live: engine offsets, tom/cymbal markers, 2x kick = MIDI 95, key bindings | 2, 3, 7 | 230,984 |
| 2026-09-26 | 1ac71f52 | Hit window: three questions answered | 3 | 17,586 |
| 2026-09-26 | ea0b884c | Probe songs built under "Hydra Probe"; hit-window test plan | 3 | 13,049 |
| 2026-09-26 | ae74ccb6 | Hit-window test code written, live steps never run | 3 | 9,127 |
| 2026-09-26 | 4908325e | Audit decisions; delay rule adopted; T21 live gate left open | 3 | 73,614 |
| 2026-09-27 | 9168b711 | Star cutoff math decoded (table, float32, cap 7, base score) | 6 | 20,976 |
| 2026-09-27 | 13cc65d5 | Stars tab built from that math | 6 | 33,723 |
| 2026-09-28 | 1b0a7cd8 | Marigold: game ignores drums0dnoflip; user ruled it a CH bug | 2 | 15,252 |
| 2026-09-29 | 9f4f8eb5 | Preview accent kicks (dynamics on kicks) | 2 | 7,444 |
| 2026-09-29 | 7c71d4dd | .srb fully decoded from the game code (RVAs 0x214EFD0, 0x215AFE0, 0x214F0A0) | 1 | 66,366 |
| 2026-09-30 | da828f4f | 1.0 fills checkbox and docs | 4 | 28,617 |
| 2026-10-04 | bbf41dc4 | Parser rules: disco flip, 2x kick, authored fill, SP phrase | 2, 4 | 54,621 |
| 2026-10-04 | 6d4811a7 | Read only the notes file from .sng/.srb | 1 | 46,055 |
| 2026-10-06 | d3bdb873 | Chart analysis failures (Hormone's Echo) | 1 | 12,552 |
| 2026-10-09 | 6c19d0ef | Note Shuffle algorithm decoded (0x20F0C30, xorshift128+) | 2 | 32,726 |
| 2026-10-09 | cfb6f1fa | Note Shuffle plan (D104) | 2 | 8,879 |
| 2026-10-09 | 3a5029e4 | Note Shuffle page review fixes | 2 | 32,662 |
| 2026-10-09 | 30beeb6e | Note Shuffle step 0: chord order, star base after shuffle, freezes confirmed at the game | 2 | 43,747 |
| 2026-10-09 | 27b9bbc4 | Note Shuffle built; D105 kick merge; accent+ghost rule from builder 0x20D4BE0 | 2 | 55,400 |
| 2026-10-10 | bc0144a4 | Song id = CH's stored checksum (songcache.bin), ADR 0028 | 1 | 32,446 |
| 2026-10-10 | f368067a | .srb parser hang (fuzzer) | 1 | 10,304 |
| 2026-10-10 | 6f165054 | MIDI parser crash (fuzzer) | 1 | 12,342 |
| 2026-10-11 | 14ef4882 | Drum lanes in the Preview (D112): CH pad and difficulty rules | 2 | 47,736 |
| 2026-10-11 | 27bb17bc | How CH's engine reads lanes during play (note+0x7C bits, engine fields, 0.075 s) | 2, 3, 7 | 30,666 |
| 2026-10-11 | 436770bd | Stitched chart: generated fills vanish next to written fills | 4 | 16,931 |
| 2026-10-11 | 18665965 | Slowest SP drain sections (drain rate) | 5 | 12,769 |
| 2026-10-12 | 1bae448b | Score breakdown lines decoded (0x20F3DC0, 0x20F6EB0); D208 | 6 | 10,311 |
| 2026-10-16 | c35a4b7d | Early fill orange 85 / slack 60 (D250) | 4 | 15,767 |
| 2026-10-16 | 1aaf0ac4 | Solo bonus formula | 6 | 14,722 |
| 2026-10-16 | 5b35957b | CH solo box and bonus decoded; D251 | 6 | 25,138 |
| 2026-10-16 | 2b845aef | Solo box and SP box in the Preview | 5, 6 | 18,944 |
| 2026-10-16 | d6095df6 | Guitar feasibility: four game facts found in the code (SP bar, score categories) | 5, 7 | 33,926 |
| 2026-10-17 | 77c0ebab | PART DRUM read like the game (D252) | 1 | 14,048 |
| 2026-10-17 | cfb159b5 | Track 0 and EVENTS name: four "Match the game" choices (D253) | 1 | 7,813 |
| 2026-10-17 | 565cde2d | D253 built from midi_reader_2155C00.asm | 1 | 8,171 |
| 2026-10-17 | 65b16299 | Preview wrong lane (drum-track) | 2 | 23,148 |
| 2026-10-17 | ecd07c4b | REA plugin and Ghidra set up | 7 | 5,594 |
| 2026-10-18 | c433b17f | Song end decompiled: Min(E, S) + 1,000 ms (D255) | 3 | 35,908 |
| 2026-10-18 | b71e9156 | Does CH count vocals in the song end (unproven) | 3 | 15,782 |
| 2026-10-20 | 7bd3ac91 | Activation behaviour on the stitched chart | 4 | 15,532 |
| 2026-10-24 | a124ccd9 | Six bugs incl. the 1.0 fill change; results stamp bump | 4 | 32,498 |
| 2026-10-25 | 9a1bf293 | 1.0-fills Preview test fix | 4 | 4,632 |
| 2026-10-27 | 03ae1d25 | 2x kick count visible; forced rescan | 2 | 36,681 |
| 2026-10-28 | f756d547 | Overlay spike: sp_watch/sp_compare, calibration answers, il2cpp-fields.md | 3, 5, 7 | 84,243 |
| 2026-10-28 | 856d0fac | CH MCP feasibility study and four-test plan | 7 | 17,977 |
| 2026-10-29 | 6a549de0 | Four tests run: bot never uses SP, plain Drums drops dynamics, per-note hooks, 4045 vs 6142 | 2, 5, 7 | 53,706 |
| 2026-10-29 | 12d82c30 | Three ch_mcp tool faults fixed | 7 | 10,569 |
| 2026-10-29 | af7dd926 | `ch` server built and merged | 7 | 34,434 |
| 2026-10-29 | e9b04efc | Live L1-L3: hook fires on hits only; bot results screen ignores keys | 7 | 22,444 |
| 2026-10-29 | 07074f3d | Public Hydra's -2.7 ms is a rounding bug in its measure maths | 5 | 8,639 |
| 2026-10-30 | 61cee421 | Audit of audits: one SP end; the 3 ms leeway is Delgar's scoring rule | 5 | 56,780 |

### Tier B: applied analyses, skim the card for rulings

| Date | Id | Topic (one line) | Areas | Condensed bytes |
|---|---|---|---|---|
| 2026-08-13 | 7dd4a0af | Search optimisation vs public Hydra (clone-hero tag) | 5 | 39,611 |
| 2026-08-15 | daff883a | Calibration fill window back to +/-70 ms (old Python Hydra) | 4 | 39,619 |
| 2026-08-16 | e3a57d11 | Score scraping and comparison (dmleaderboards) | 6 | 28,774 |
| 2026-08-22 | ee38a528 | Onyx previewer port | 2 | 50,029 |
| 2026-08-23 | 90b3c45d | Preview activation fills issue | 4 | 9,370 |
| 2026-08-29 | 76d18037 | Display x1.00 multiplier | 5 | 12,188 |
| 2026-08-29 | a21909dc | Effective text on backends | 5 | 6,555 |
| 2026-08-29 | 99e42ceb | Backend calculation beyond squeeze | 5 | 10,140 |
| 2026-08-29 | 700792ed | 4.45 multiplier backend | 5 | 13,220 |
| 2026-08-29 | aea2ed4b | Squeeze at 5:05 (impossible, 12x budget) | 5 | 4,892 |
| 2026-08-29 | 40780692 | Astral Body squeezes | 5 | 2,146 |
| 2026-08-30 | 8ad88c66 | Squeeze at 1:35 | 5 | 5,573 |
| 2026-08-31 | 3a74c627 | Squeeze at 4:09 | 5 | 5,738 |
| 2026-09-02 | 2059d526 | Video path analysis report | 6 | 16,002 |
| 2026-09-03 | 1e4a8682 | Aether squeeze (no playable squeeze) | 5 | 3,152 |
| 2026-09-03 | e05522d2 | Video optimisation analysis | 3 | 12,236 |
| 2026-09-08 | f883ef7a | Optimal path SP coverage | 5 | 8,375 |
| 2026-09-09 | 72c8a4a3 | Score difference 0 0 vs 18 | 5, 6 | 8,481 |
| 2026-09-09 | 44235a02 | SP paths in anti eulogy | 5 | 15,093 |
| 2026-09-10 | 4cc9d4fc | Activation optimality question | 5 | 4,050 |
| 2026-09-10 | ac03a364 | Aliens activation timing | 5 | 8,842 |
| 2026-09-24 | 8070412a | Squeeze-out points display | 5 | 14,223 |
| 2026-09-26 | 9405caaa | Library squeeze-out charts | 5 | 35,279 |
| 2026-09-26 | b4057c7f | Codebase audit (clone-hero tag) | - | 13,040 |
| 2026-09-27 | 3acde6e6 | Kick reach to highway edge | 2 | 9,696 |
| 2026-09-29 | 56ac5b71 | 3-note squeeze messages | 5 | 11,814 |
| 2026-09-29 | 2c65cb43 | Effective timing removal from backend notes | 5 | 21,027 |
| 2026-09-29 | 96ae6b89 | Extreme multiplier causes; scale line always shown | 5 | 43,399 |
| 2026-10-03 | b5aa7c31 | Derivation audit | 5, 6 | 55,766 |
| 2026-10-03 | cebf3006 | Derive-once gate, round 7 planning | 5 | 39,286 |
| 2026-10-10 | c8856afd | Missing wins in Splax vs Boddy | 6 | 38,652 |
| 2026-10-12 | 135630ac | Squeeze-out cost's two figures | 5 | 14,201 |
| 2026-10-14 | c0e1259d | One record per chart (generated-fill, fill-rule tags) | 4 | 130,983 |
| 2026-10-17 | 160c1448 | Red and kick note speed arithmetic | 2 | 4,019 |
| 2026-10-21 | 2dd7f198 | Squeeze timing analysis | 5 | 6,127 |
| 2026-10-24 | 5016a44a | Derivation of 1.54 and 0.65 | 5 | 7,125 |
| 2026-10-29 | bea11e68 | Squeeze-tools repo fix | 5 | 13,431 |

Not in the index yet: the 2026-10-30 session that started the bot run to test Star Power (the one the brief opens with). The index's last line is 61cee421 (2026-10-30, the audit of audits). Its card will exist only after the start-of-session hook condenses it, so the scout plan should allow for one more Tier A session in area 5/7.

Also outside the cards: `memory\archive\ch10-fill-rule-video-runs.md` (the Desecration Day numbers, 2026-09-03) is archived and not indexed in MEMORY.md, but it holds the one live piece of evidence for the CH 1.0 fill deadline, so the fills scout should read it.

### How to cut this into scout slices

What one Sonnet scout can read. I assume one scout can take about 250 KB of condensed text, plus the cards for its sessions, inside 100 calls and 20 minutes. Reasoning: the Read tool returns up to 2,000 lines per call, and the condensed files run about 90 bytes per line (8157f2dc is 230,984 bytes in 2,534 lines; 3ee237e9 is 57,197 bytes in 656 lines; e9b04efc is 22,444 bytes in 218 lines), so one Read covers roughly 180 KB and most sessions are one call each. Reading is not the cost. The cost is that the brief wants every claim checked against the code, the dumps and the result files, and that takes two or three calls per claim (grep the Hydra owner, open the dump or `.asm`, open the result file). A scout that finds 25 claims spends 60 to 75 calls checking them. So the budget is set by claims, not bytes, and 250 KB is the point where a dense session (8157f2dc alone yields dozens of claims) still fits. The two giant files, 450488b5 (245,706 bytes, mostly audio-extraction tool work) and 8157f2dc (230,984), each get a scout to themselves or are read only in their fact-bearing stretches found by Grep.

Read the cards first, then the condensed text. Every card has a Rulings section with the user's exact words (writer-brief rule 1), and an Outcome. The 77 Tier A cards total 308,820 bytes and the 37 Tier B cards 119,107 bytes. A scout reads its area's cards in two or three calls, then opens a condensed file only where the card points at a fact or where a memory note's `Sessions:` line names the session. That cuts the condensed reading to maybe a third of the tier's bytes.

Proposed slices, by the brief's seven areas, so each scout's claims land on one page. Byte counts are the Tier A condensed totals for the sessions listed, summed from the table above.

Slice H1, chart reading (area 1): cdf10e6d, 4a6a3538, 450488b5, 7c71d4dd, 6d4811a7, d3bdb873, bc0144a4, f368067a, 6f165054, 77c0ebab, cfb159b5, 565cde2d. That is 12 sessions and 487,855 bytes, but 450488b5 is half of it and is audio tooling; the scout greps it for "srb" format facts and reads only those stretches. Memory notes: srb-format-support, midi-drum-track-names-d252. Docs: docs/srb-format.md, ADR 0028, the open1 midi_reader asm.

Slice H2, note list (area 2): 1136a257, 8157f2dc (the parse and marker parts), 1b0a7cd8, 9f4f8eb5, bbf41dc4, 6c19d0ef, cfb6f1fa, 3a5029e4, 30beeb6e, 27b9bbc4, 14ef4882, 27bb17bc, 65b16299, 03ae1d25, and 6a549de0 for plain Drums. Fourteen sessions without 8157f2dc come to 480,704 bytes; this is the biggest slice by claims (Note Shuffle alone is five sessions), so split it: H2a Note Shuffle (6c19d0ef, cfb6f1fa, 3a5029e4, 30beeb6e, 27b9bbc4: 173,414 bytes, plus docs/audit/note-shuffle/), H2b everything else in area 2 (307,290 bytes plus the marker stretches of 8157f2dc). Memory notes: ch-note-shuffle-algorithm, ch-disco-noflip-rule, ch-plain-drums-drops-dynamics, drum-lanes-d112-2026-10, derivation-audit-2026-10-03 (the open 2x-kick question).

Slice H3, timing (area 3): 6e7764db, 23b23b12, 49d07b9c, 3ee237e9, 2a88c065, 8157f2dc (the engine-offset and hit stretches), 1ac71f52, ea0b884c, ae74ccb6, 4908325e, c433b17f, b71e9156, f756d547 (calibration answers). Without 8157f2dc that is 12 sessions and 466,169 bytes; 4908325e and 2a88c065 are audit sessions where only the delay/Offset part matters, so Grep for "delay" and "Offset" first. Memory notes: ch-audio-delay-offset-rule, ch-dynamic-input-probe-2026-09, song-end-one-rule-d255. Code: tools/ch_probe/constants.py, engine.py.

Slice H4, fills (area 4): d69fdea1, d46444c2, f520253b, 35a607e6, da828f4f, 436770bd, 7bd3ac91, c35a4b7d, a124ccd9, 9a1bf293, plus Tier B daff883a and c0e1259d skimmed by card. Ten Tier A sessions, 164,598 bytes. Memory notes: fill-spawn-comparison, early-fill-orange-85-d250, stitched-chart-auto-fills, archive/ch10-fill-rule-video-runs. This scout also writes the first entry of the live-check list (Ladies Talk, measures 76 to 108).

Slice H5, Star Power (area 5): f1784fd5, c73697df, 9f5a3914, fd73e1ee, 93d37c20, 13441c5f, a28ef5f2, 8b24db8f, 18665965, 2b845aef, d6095df6, 07074f3d, 61cee421, f756d547 (sp_watch part), and the Tier B squeeze sessions by card only. Thirteen Tier A sessions without f756d547 come to 366,708 bytes, so split: H5a mechanics and the SP end (f1784fd5, c73697df, 9f5a3914, 93d37c20, 61cee421, 07074f3d: 224,657 bytes) and H5b meter, drain, bar and the overlay spike (fd73e1ee, 13441c5f, a28ef5f2, 8b24db8f, 18665965, 2b845aef, d6095df6, f756d547: 226,294 bytes, of which f756d547 is read for sp_watch and il2cpp-fields.md). Memory notes: sp-squeeze-mechanics, squeeze-transfer-scale, sp-end-is-one-point-leeway-is-delgars.

Slice H6, scoring (area 6): e61236dc, 219bf8c2, 528e137d, 9168b711, 13cc65d5, 1bae448b, 1aaf0ac4, 5b35957b, plus Tier B e3a57d11 and c8856afd by card. Eight Tier A sessions, 238,818 bytes. Memory notes: ch-star-cutoffs, ch-score-breakdown-lines, preview-solo-box-10ms-step-2026-10, gameplay-video-score-analysis (two game facts), dmleaderboards-comparison (scoredata.bin only). Result files: scorestats.json copies in hydra-data\ch-mcp-runs.

Slice H7, internals and tools (area 7): 28e6f29d, e59042f9, ecd07c4b, 856d0fac, 6a549de0, 12d82c30, af7dd926, e9b04efc, and 8157f2dc whole. Eight sessions without 8157f2dc are 192,436 bytes; with it 423,420. Give 8157f2dc its own half: H7a the MCP sessions (ecd07c4b, 856d0fac, 6a549de0, 12d82c30, af7dd926, e9b04efc: 144,724 bytes), H7b 8157f2dc plus 28e6f29d and e59042f9 (278,696 bytes). All slice sums were checked by a PowerShell loop over the file lengths after the hand sums proved wrong in six of eleven places. Memory notes: ch-mcp-feasibility-2026-10, ch-mcp-server-2026-10, ch-autoplay-slipping-2026-09, ch-dynamic-input-probe-2026-09. Code: tools/ch_mcp, tools/ch_probe.

That is 11 scouts (H1, H2a, H2b, H3, H4, H5a, H5b, H6, H7a, H7b, plus one for the Tier B cards and any ruling they hold, about 119 KB of cards). At three or four per wave (memory lean-agent-fanout) it is three waves. If the user wants fewer, merge H7a into H6 and H2a into H2b and accept that those two scouts stop at the 100-call warning with a handoff list.

Each scout returns a claims table: claim, area, the session and line it came from, the evidence it checked (file and line, or "not found, looked in ..."), and the label it earns under the brief's four labels. A claim whose evidence a scout could not open in its budget is returned as "unchecked", never as proven.

## 2. The memory notes

Notes named by the brief, plus the others that hold Clone Hero facts (found by grepping the memory folder for "Clone Hero", "CH", "GameAssembly", "RVA", "song.ini" and "scorestats": 38 files matched, of which 27 are about the game). Classes: (a) pure game facts, move to the pages and delete the note; (b) mixed, a game fact plus a ruling, where the ruling's exact words are quoted; (c) ruling or preference only, stays. The MEMORY.md line is from a grep of that file on 2026-10-30. Where a note paraphrases a ruling instead of quoting it, I say so and name the card that holds the exact words (cards quote the user verbatim by writer-brief rule 1).

| Note | MEMORY.md line | Class | What stays, in the user's words |
|---|---|---|---|
| ch-audio-delay-offset-rule | 59 | (a) | Nothing. Measured facts and the Hydra owner (`kOffsetDirection`). The untested edge (delay equal to Offset) goes on the timing page as unknown. |
| ch-autoplay-slipping-2026-09 | 43 | (a) | Nothing. Game facts (MIDI 95 vs 96, tom markers, engine +0x100/+0x94/+0x20, key bindings) go to pages 2, 3 and 7; the play_chart gotchas go to tools/ch_probe's README, not a page. |
| ch-dynamic-input-probe-2026-09 | 46 | (a) | Nothing new; its process lesson is already its own note (names-from-data-not-recall, verify-before-claiming-a-source-is-wrong). The open T21 live gate goes to the live-check list. |
| ch-mcp-feasibility-2026-10 | 44 | (b) | "Proceed, my call" (the EULA ruling, 2026-10-28). The rest (bot never uses SP, hook RVAs, Frida tips) moves to page 7 and tools/ch_mcp/README.md. |
| ch-mcp-server-2026-10 | 45 | (b) | "Yes, all seven" and "Yes, fix both" are approvals of finished work, not standing rules; the standing part is the note's own line that the user expected the MCP to test Star Power without them and the gap must be said up front. Facts move to page 7 and docs/agents/clone-hero-mcp.md. |
| ch-note-shuffle-algorithm | 58 | (b) | "go with the game's rule" (accent+ghost plays as accent, D105). Everything else is a fact or a build history and moves to page 2 and docs/audit/note-shuffle/. |
| ch-plain-drums-drops-dynamics | 39 | (b) | "there is no way this is intentional, it must be a bug in clone hero" and "Keep Hydra as is" (2026-10-28). The code facts (0x20D32E5, 0x20EFEE0 clears 0x40000/0x80000) move to page 2. |
| ch-score-breakdown-lines | 56 | (b) | The note paraphrases ("The user decided 2026-10-11: Hydra keeps matching the game"). Card 1bae448b has the words: D208 "we don't need to change anything, matching the game is correct." Facts move to page 6. |
| ch-star-cutoffs | 57 | (a) | Nothing. Table, float32 rule, cap 7, base score, re-dump recipe (the recipe goes to tools/ch_mcp/README.md, which already says how the dumps were made). |
| ch-disco-noflip-rule | 38 | (b) | The note paraphrases the ruling. Card 1b0a7cd8 has it: "this is a bug in clone hero, not an intentional feature. it will be fixed in a future update." The code and chart facts move to page 2. |
| sp-squeeze-mechanics | 55 | (a) | Nothing from the user. Most of it is Hydra's own mechanics (kSqueezeWindowMs 500 is Hydra's, not the game's); the game part is the SqIn/SqOut definition and goes to page 5. The 500 ms window belongs to a later Hydra-engine knowledge base. |
| squeeze-transfer-scale | 77 | (b) | One standing display ruling, paraphrased in the note; card 96ae6b89 has the words: "the app should always display a multiplier when one exists." The game fact (SP length is measure-based, checked against DragonDelgar's numbers) moves to page 5; the rest is Hydra display history. |
| fill-spawn-comparison | 54 | (b) | Paraphrased "(kept 85 on both sides per user)"; card d69fdea1 has the click: chose "85 ms both sides (Recommended)". The 1.0 vs 1.1 deadline facts and the 18,494-chart results move to page 4. |
| early-fill-orange-85-d250 | 18 | (c) | "Keep them. The extra E fills in the notation are fine." This is a Hydra constant decision (D250), not a game fact; it stays, and D250 in the decision log is its other home. |
| sp-end-is-one-point-leeway-is-delgars | 1 | (b) | "there should never be two different answers for where star power ends. ever. the 3ms rule is for whether a backend is included in the optimal score or uncounted. it's an arbitrary number chosen by dragondelgar and i am fine with keeping it so our scores use the same methodology as delgar's hydra". The ruling stays; the public-Hydra citation (hypath.py:678 at 3b49012c) moves to page 5. |
| srb-format-support | 75 | (a) | Nothing. Container layout, RVAs, Saurian King failure, song-id rule; "(user's call)" on the stamp bump is D170 in card bc0144a4 ("yes, go ahead, let the 84 results drop"). Moves to page 1 and docs/srb-format.md. |
| stitched-chart-auto-fills | 104 | (a) | Nothing. The fact (generated fills stop when any written fill exists) goes to page 4; the merge recipe is a tool note. |
| midi-drum-track-names-d252 | 114 | (b) | Clicked choices, in card cfb159b5: chose "Match the game (Recommended)" twice, "No bump (Recommended)", "No, skip the count (Recommended)". The game facts (loop from track 1, OrdinalIgnoreCase "events") move to page 1. |
| song-end-one-rule-d255 | 116 | (b) | "there should one rule that every function uses... we don't need different places deriving different rules." and "Ignore it". The decompiled rule moves to page 3; the vocals question goes to the unknown list. |
| drum-lanes-d112-2026-10 | 99 | (b) | "clone hero only draws the blue. this will be an intentional deviation from the game. it looks better with both." Engine lane fields move to pages 2 and 7. |
| preview-solo-box-10ms-step-2026-10 | 111 | (a) | Nothing beyond D251, which lives in the decision log. The solo box and bonus formula move to page 6. |
| gameplay-video-score-analysis | 79 | (a), partly | Two game facts move (solo bonus paid as a lump at solo end; per-note SP values at 4x). The rest is the manual video method, superseded by fc-video-workflow; whether to archive it is the user's call, not this project's. |
| fc-video-workflow | 50 | (b) | The rules paragraph is marked "user-enforced" but paraphrased; the exact words sit in cards 219bf8c2 and d46444c2. Only one game fact (CH 1.0's fill deadline is one fill length) moves to page 4. The tool note stays. |
| dmleaderboards-comparison | 76 | (c), mostly | Not a game-rules note. One CH fact (scoredata.bin holds one best score per chart per instrument) could go to page 7; the "one leaderboard for every speed" line is the user's correction. Leave it. |
| derivation-audit-2026-10-03 | 60 | question | Its one open question (2x kick before a normal kick on one tick) becomes a row on page 2 under "questions for the user"; then the note can go. |
| ghidra-location | 48 | (c) | Tool location, how-to-work. Stays. |
| best-all-0-path-always-shown | 24 | (c) | Hydra ruling D102, not a game fact. Out of scope; stays. |
| archive/ch10-fill-rule-video-runs | not indexed | (a) | The Desecration Day evidence (CH v1.0.0.4080-final run, 160 above the 1.1 optimal) moves to page 4 as a "proven in a live run" row. |

Count: 10 notes class (a), 12 class (b), 4 class (c), 1 open question. Deleting the (a) notes and shortening the (b) notes to their rulings removes about 16 lines from MEMORY.md (the lines listed above, less the four that stay).

## 3. The card writer

### The card format

The format is fixed by `writer-brief.md` (9,540 bytes, in the session-cards folder, tracked by no repo). A card is at most 60 lines: a `#` title with the start date, a header line with six `key: value` parts (session, project, commits, D, tags, agents), then seven `##` sections in a fixed order: Goal, Topics, Roadblocks, Rulings, Outcome, Open threads, Sources. Rulings are the user's exact words copied from `user_text.json` (rule 1), or `chose "<label>" to "<question> ..."` for clicked options (rule 2). Open threads ends with a `memory:` line naming memory files. Tags come only from `tags.txt` (rule 3). The checker `check.py` enforces the rules after each wave; the hook tells the main session to run `check.py`, `index.py` and `link_memory.py` in that order. The sample card I read (2026-10-29-e9b04efc.md, 36 lines) follows the shape exactly.

### Where a "missed write-back" flag fits

The natural home is the Open threads section, which already carries "what was left unfinished" and the `memory:` line. Add one more fixed line, `write-back: <page or none>`, after `memory:`. When the writer sees that the session learned a Clone Hero fact and the session's commits did not touch `docs/clone-hero/`, it writes `write-back: docs/clone-hero/<area>.md (fact in one line)`. Otherwise `write-back: none`. The checker gets one new rule: the line exists, and when it is not `none` it names a path under `docs/clone-hero/`. `index.py` can then carry a `[write-back]` marker on the index line the way it carries `[stale]`, so the next session sees it without opening the card. The `clone-hero` tag would be required on any card with a non-none write-back.

### How the writer can tell a fact is missing from a page

The writer runs on Haiku with Read, Glob, Grep and Write only (card-writer.md line 5), reads one condensed file per session, and knows nothing about the pages today. Three signals it can use, cheapest first. One: the metadata `commits:` list; if any commit touched `docs/clone-hero/`, the write-back happened (the writer cannot run git, so the condensed text or the meta.json has to carry the touched paths; today meta.json lists commits only, so prepass.py would add the files each commit touched, which is a hooks-repo change). Two: a Grep of `docs/clone-hero/` for the session id, since the write-back rule should have the page cite the session that found the fact ("Found 2026-10-29, session e9b04efc"); no hit means no write-back. Three: a Grep of the page for the claim's own number or name (an RVA, a constant such as `0x20DEEC0`, a field offset). The brief should tell the writer to use signal two as the test and signal three only to describe the missing fact in one line. The writer must never try to judge whether a fact is right; it only flags that the page lacks it.

### Tracking and the hooks repo

`C:\Users\Patrick\.claude\agents\card-writer.md` is tracked by the hooks repo. `git -C C:/Users/Patrick/.claude log -3 --oneline -- agents/card-writer.md` shows three commits: c7b9b46 "agents: read the preamble of the repo the brief names, not Hydra's", 220b4ad "Posture names card-writer (Haiku) as a fifth agent type", 89e471b "Session cards: a card-writer agent type limited to read, write and the status line". So a change to it goes on a branch in a worktree under `C:\Users\Patrick\.claude\worktrees\` and gets its own derive-once review, as CLAUDE.md says. `writer-brief.md` is tracked by nothing, so its edit is a plain file write the user approves. Two things I noticed in the brief while reading it: step 3 under "How to work" names `C:/Users/Patrick/.claude/hooks/status_append.ps1`, and `Test-Path` says that file still exists, as does the current `gates\status_append.ps1`; the gates README line 39 says the housekeeping scripts moved from `hooks\` to `gates\`. The brief works today but points at the old folder, and the write-back task should fix the path while it is in there. The brief also says "Status line every 10 tool calls or 5 minutes", while the preamble and CLAUDE.md rule 7 say time only.

### The start-of-session hook

CLAUDE.md "Past sessions" (lines 19 to 25) says every past session has a card and that a start-of-session hook asks the main session to write cards for finished sessions with none. That hook is `C:\Users\Patrick\.claude\gates\session_cards_start.ps1` (its header comment is the spec; tested by `tests\test_h53_session_cards_start.ps1`). On every session start inside the Hydra repo it runs `prepass.py` (condenses finished transcripts) and `batch.py` (writes batch files), claims the work for two hours, and returns instructions as `additionalContext`: launch one card-writer agent per batch file, tell it to read `writer-brief.md` first, then run `check.py`, `index.py`, `link_memory.py`. So the write-back flag reaches a user only through this chain: the next session's hook condenses the finished session, a Haiku writer reads the brief and writes the `write-back:` line, `check.py` accepts it, and `index.py` surfaces it. The task that adds the flag therefore touches the brief (untracked), card-writer.md (hooks repo, reviewed), and `tools\session_cards\check.py` and `index.py`, which `git -C C:/Users/Patrick/.claude ls-files tools/session_cards` confirms are tracked by the hooks repo (with batch.py, common.py, link_memory.py, number_rulings.py and a fixture), so they are reviewed there too. The hook script itself needs no change.
