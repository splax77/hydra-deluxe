# Clone Hero evidence for the twelve open questions

Gathered 2026-10-03. Read-only: nothing in the repo was edited, built or run.

## Where the evidence came from

Most answers below come from reading Clone Hero's own code. The game is Clone Hero v1.1.0.6142 (`C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`). An earlier session had already run Il2CppDumper on it. Its output sits in `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\9168b711-0081-4bbb-9a2b-20ca0ba79902\scratchpad\il2cpp\`. That folder holds `dump.cs`, `stringliteral.json` and the disassembler `chdis.py`.

Il2CppDumper turns the game's compiled code back into class and method lists. The names are scrambled, so I found the code by its string literals ("[mix", "ENABLE_CHART_DYNAMICS", "solo") and by its data shapes. Then I read the machine code with capstone, a disassembler. All function addresses below are RVAs, which means offsets from the start of GameAssembly.dll.

My disassembly listings are saved in `...\cebf3006-...\scratchpad\triage\re\` (one `f<rva>.asm` file per function). `mxref.py` in that folder finds every piece of code that uses a given string or address.

Strength scale, strongest first:

1. **CH code**: read straight from Clone Hero's own code.
2. **Measured**: a live reading taken from the running game.
3. **Hydra doc**: a Hydra document that records a proof.
4. **Format doc**: a public description of the chart formats.
5. **Other tool**: another program that copies Clone Hero's rules.

A caution that applies to every answer: reading code statically means tracing it by hand without running it. It is strong evidence, but it is not a test. Each answer names the exact address, so anyone can re-check it.

---

## 11. Is disco flip set per difficulty, or only from Expert's markers?

**Answer: per difficulty, in both .chart and .mid.** `[mix 2 drums0d]` turns disco flip on for Hard only. Expert's `[mix 3 ...]` markers do not touch Hard. Strength: CH code.

Evidence:

- `0x215C750` is Clone Hero's single "mix" event parser. It trims the text and drops the `[ ]`. It turns `_` into spaces and requires the first word to be `mix`. It reads the number with `Int32.TryParse` and turns it into a difficulty with `0x210D990`. Then it requires the rest of the text to start with `drums`. It returns the difficulty, the rest of the text and the tick. Listing: `re\f215C750.asm`.
- In .chart files, `0x213CAF0` handles `E` events inside a track. When the text starts with `[mix` or `mix`, it calls the parser above. It then keeps the event only if the parsed difficulty equals the difficulty of the track being read. That check is at `0x213D076`: `cmp bl, byte ptr [rsi+0x59]`. The track class keeps its difficulty in the field at +0x59. That field's type is the enum Easy=0, Medium=1, Hard=2, Expert=3 (`dump.cs` line 550049). Listing: `re\f213caf0.asm`.
- In .mid files, `0x2155050` reads the drum track. On a `[mix`/`mix` text event it calls the same parser. It then looks up the track for the parsed difficulty with `0x1820CFD40(chart, instrument, difficulty)` and adds the disco event to that track (`0x215574C`–`0x2155793`). So each marker goes to its own difficulty. Listing: `re\f2155050.asm`.

Hydra today: `re_disco_on`/`re_disco_off` in `src/parse/song.cpp` hard-code `mix 3`, so Expert's spans are applied at every difficulty. That is the opposite of Clone Hero.

Side note: the parser accepts any text that starts with `drums`. So `drums0dnoflip` reaches the disco code as its own string, and how it is read is decided later in `0x215CD50`. That fits the known Clone Hero dnoflip bug in the memory note `ch-disco-noflip-rule.md`. I did not re-trace `0x215CD50`.

---

## 21. Which edge ends a Star Power phrase?

**Answer:**

- A note counts as inside the phrase when start ≤ note tick < start + length. The end is half-open, so the end tick itself is outside.
- A chord exactly on the end tick is **not** in the phrase.
- A zero-length phrase awards **nothing**.
- A phrase that runs past the last note is awarded on its last note.
- .chart and .mid follow the same rule.

Strength: CH code.

Evidence:

- Both formats store the phrase as the raw tick and length:
  - .chart `S 2 <len>` is handled by `0x213E0C0`, which passes the length unchanged (`re\f213e0c0.asm`).
  - .mid note 116 is handled in `0x2155050`. It computes length = note-off tick − note-on tick and calls `0x20CF0B0`.
  - Both add the phrase to the track's phrase list at +0x70.
- `0x20D2440` gives phrases to notes (`re\f20d2440.asm`). It is a static method on the note-list builder that takes the note list and the phrase list. For each note it does three things:
  - It skips the note if it comes before the phrase start.
  - If tick ≥ start + length, it moves on to the next phrase. It moves once per note, using `0x20D0330`.
  - If start ≤ tick < start + length, it marks the note "in phrase" (flag `0x80`). If there is no next note, or the next note's tick ≥ start + length, it also marks this chord "last note of phrase" (flag `0x100`) on every note of the chord.
- With length 0 the test `start ≤ tick < start` is never true, so no note is marked.
- A separate cleanup step deletes phrases that hold no notes: `0x5DB320`, called from the track finish step `0x215C320` for the phrase list. Its window is [start, start+len−1], or [start, start] when the length is 0. So a zero-length phrase on a note survives cleanup, but the assignment step still gives it to no note.

Hydra today: the audit's table says .mid flags match Clone Hero in all three rows. The .chart side differs in two rows. It awards zero-length phrases, which Clone Hero never does. It does not award a phrase that runs past the last note, which Clone Hero does. In practice that means 26 zero-length phrases in the library (Muse - Feeling Good) should not be awarded, and Modern Value's last phrase should be.

---

## 37. Does Clone Hero clamp the SP end to the cap when a phrase overfills the meter during Star Power?

**Answer: yes.** During Star Power, the SP amount is clamped to the full-meter maximum after a phrase is added. The SP end time is then worked out again from the clamped amount. The overflow is thrown away. Strength: CH code.

Evidence:

- The engine keeps the SP amount as a whole number of chart ticks at `engine+0x68`. The maximum is at `engine+0x1B0`. Activation is at `0x20F5640`: it sets the SP-active flag at +0x42 and doubles the multiplier at +0x48. It works out the end time at +0x120 using `0x20F37E0(amount, activation time)`.
- The per-update SP routine is `0x20F6800`, a virtual method in slot 12 of the base engine class. It is in `re\engine_all.asm`, lines starting `20F6800:`.
  - While SP is active it drains the amount.
  - Then, at `0x20F6932`–`0x20F6954`: if amount > max, it sets amount = max and sets the flag at +0x130.
  - When that flag is set, it recomputes the end time from (ticks already drained + clamped amount) at `0x20F6968`–`0x20F6986`.
  - When SP is not active, the same clamp runs at `0x20F681D`–`0x20F683B`.
- CHOpt agrees (other tool). Its `add_phrase` does `m_sp = std::min(m_sp + phrase_amount, 1.0)`. See `9168b711-...\scratchpad\chopt_processed.cpp` line 79.

So the end becomes: where the clamp hit, plus what a full meter lasts. That is what `ScoreGraph::extend_deacts` computes. The +1 bar with no ceiling in `ScoreGraph::add_deact_edge` does not match Clone Hero.

Caveat: Clone Hero's cap is a full meter. Hydra's docs say the meter holds 4 bars (`docs/differences-from-public-hydra.md` line 72). Caps other than 4 are a Hydra what-if setting with no game equivalent. The audit's example uses cap 2, so it is only a question about Hydra's model.

---

## 53. With two meter changes between chords, which meter sets a generated fill's length?

**Answer: unknown.** I could not find Clone Hero's fill generator.

What I checked:

- Fill objects are created in only five places, all found by searching for the fill pool's "rent" call (`0x3CF8060`):
  - .mid note 120, at `0x20CEDC0`.
  - .chart `S 64`, at `0x213E0C0`.
  - The binary song-cache reader, at `0x20E5E70` and `0x215B7C0`.
  - An add-fill method, `0x215BD50(tick, len, flag)`. No code calls it directly, so it is probably reached through a delegate.
- None of these places contains generation logic.
- The fill-to-activation step `0x20CFF60` only reads the track's fill list at +0x88.

Hydra today: the fill length comes from `Song::check_activations`'s own meter walk, which can lag one meter change behind.

Cheapest way to settle it:

- Trace what calls `0x215BD50` at run time. One breakpoint with `tools/ch_probe`'s debugger on a chart with no authored fills would show the caller. That needs a live session, which was out of scope here.
- Or make a probe chart with no fills and two meter changes between chords, and see where the activation fill shows up in game.
- Failing both, ask the user.

---

## 64. If `[ENABLE_CHART_DYNAMICS]` comes after some notes in a .mid, are the earlier ghosts and accents scored?

**Answer: no.** Notes before the tag stay plain. Clone Hero reads the drum track's events in file order and checks the tag as it goes. That is the same per-note gate Hydra uses. Strength: CH code.

Evidence:

- In `0x2155050`, a local flag at `[rsp+0x41]` starts at 0 (`0x215524D`). It turns on at `0x21557CD` when a text event equals `ENABLE_CHART_DYNAMICS` or `[ENABLE_CHART_DYNAMICS]` exactly (`String.op_Equality`, `0x21557A5` and `0x21557BB`).
- Each note-on checks the flag at that moment, at `0x21555F1`. If it is set, velocity 1 adds the ghost bit (bit 9) and velocity 127 adds the accent bit (bit 10).
- Clone Hero has no flag for the whole chart. The local flag is never saved to the track.

Two edges worth knowing:

- **Same tick.** At a shared tick, the order of events in the file decides. A note-on written before the tag stays plain. Hydra says "that note priced", so Hydra may differ here.
- **Which track.** The flag is local to the pass over one drum track. So the tag only counts when it sits in the drum track itself. I did not check whether Hydra accepts it from another track, such as EVENTS.

Hydra today: the per-note gate matches Clone Hero. The "Dynamics enabled: yes" line on the Dynamics tab has no Clone Hero equivalent.

---

## 77. Is the whole drum hit-window cap 171.43 ms or 170 ms?

**Answer: 171.43 ms.** The cap is not a clamp on the window. It comes from a clamp on the formula's input. Strength: CH code plus a measured run.

Evidence:

- `0x20F3A90` is the base engine's per-note window routine (`re\f20f3a90.asm`). It calls `0x20F7210` twice, once for the gap before the note and once for the gap after.
- `0x20F7210` returns Clamp((next time − this time) × 0.5, front, back). Here front = 0.0375 s and back = 0.085 s, read from engine +0x38 and +0x30. If a neighbour is missing it returns back. So each half-gap is held between 37.5 and 85 ms.
- The two half-gaps are added together and passed to the drum formula, the virtual method in slot 19 at `0x20DDDA0`, through the call at `0x20F3B9E`. So the formula's input is held between 75 and 170 ms.
- I evaluated the formula with its constants read from the DLL: 0.0110924369747899, 2.628539e-05, 170, 20, exponent 2, divisor 1000. Its output at 170 ms is **171.4313 ms**. Its output at 75 ms is **96.2932 ms**.
- The measured run agrees. `tools/ch_probe/experiments/results/poll_windows.csv` tops out at 171.431308 (6 rows) and bottoms out at 96.293167 (22 rows). Both match the computed values exactly. So 171.43 is the cap and 96.29 ms is the floor.
- The routine at `0x20F3A90` returns half the formula's value. Its caller compares hits against that half.

Hydra today: Hydra's engine uses a fixed 85 ms per side. The probe scripts disagree with each other. `watch_window.py` uses 171.43, which is right. `passive_probe.py` and `poll_windows.py` use 2 × back = 170, which is wrong.

---

## 141. How does Clone Hero read a quoted .chart event such as `E "solo"`?

**Answer: it does not recognise it.** `E "solo"` is not a solo start. `E "[mix 3 drums0d]"` is not a disco marker. Strength: CH code.

Evidence:

- The .chart line dispatcher `0x213D2D0` sends `E` lines to `0x213CAF0` and passes the raw text with its position (`re\f213d2d0.asm`).
- `0x213CAF0` reads to the end of the line and calls `MemoryExtensions.Trim`, which removes whitespace only, at `0x213CCCB`. It then tests in this order:
  - starts with `[mix`
  - starts with `mix`
  - equals `solo`
  - equals `soloend`
- The equality check `0x20FE510` is called with its ignore-case argument false. Read that way, it is an exact, case-sensitive comparison of every character (`re\f20FE510.asm`). No code removes quotes.
- So `"solo"` matches nothing and is dropped.

Hydra today: matches. Hydra compares raw text for solo, soloend and disco events.

Side note: this handler covers per-track `E` events. Clone Hero also takes `mix_3_drums0d` with underscores, and `mix 3 drums0d` without brackets. Hydra's regex may not; I did not check that.

---

## 222. Does a playback speed other than 100% change a Clone Hero score?

**Answer: not through the scoring math.** Each hit adds note points × the current multiplier, with no speed term. Star Power and sustains are counted in chart ticks, not seconds. Leaderboards keep each speed separate. Strength: CH code for the math, plus public docs for the leaderboards.

Evidence:

- Score is added at `engine+0x94` in `0x20F5F70` (`add [rcx+0x94], points*multiplier`) and in `0x20F6EB0` / `0x20F6A50` (`imul eax,[rsi+0x48]; add [rsi+0x94],eax`). No other factor appears there.
- The SP amount is a whole number of ticks (see question 37), so it does not depend on speed.
- The Clone Hero v1.1 leaderboard announcement says leaderboards show only scores for the selected modifiers and speed: https://clonehero.net/2024/11/10/clonehero-leaderboards.html.

What speed can still change: hit windows are in milliseconds while the chart moves faster or slower. So which squeezes and timings are possible can change, and with it what score a player can reach. The points for a given set of hits stay the same.

Hydra today: no rule. On the dmleaderboards page, a non-100% score hides its percent but still counts toward "above optimal".

Cheapest way to settle the rest: this part is the user's call, not a game fact. Should off-speed runs count as "above optimal"? Since Clone Hero ranks each speed separately, the natural choice is to leave them out.

---

## 304. Does a backend note exactly on the 3 ms leeway edge count?

**Answer: no Clone Hero fact covers it.** The 3 ms leeway is Hydra's own modelling choice. I found no 3 ms (0.003) constant in the 126 engine methods I disassembled; the rest of the game was not searched.

Evidence:

- `docs/differences-from-public-hydra.md` line 74 calls the backend leeway one of "Hydra Deluxe's own choices, not facts read from Clone Hero". The value 3.0 is the user's decision (decisions 7 and 13 in the 2026-09-24 derivation-fixes plan).
- No 0.003 constant appears in any of the 126 engine methods I disassembled (`re\engine_all.asm`).
- In the game, SP ends at a time (`engine+0x120`), checked once per frame. Any real "leeway" comes from frame timing, not from a fixed edge.

Hydra today: the test is strict (`off < leeway`), so +3.0 ms does not count.

Cheapest way to settle it: ask the user, since it is a model setting. A real offset of exactly 3.000 ms almost never happens, so the choice is close to cosmetic.

---

## 314. Does Clone Hero count fill-length beats as quarter notes in 6/8 and 7/8?

**Answer: unknown.** I did not find the code that checks the 1.1 fill deadline ("4 beats").

One related fact: when Clone Hero attaches a fill to a chord, it measures its slop as a share of the chart resolution (ticks per quarter note), not of the time signature. See question 315. That hints that Clone Hero thinks in quarter-note beats, but it does not prove it for the 4-beat deadline.

Hydra today: one beat is one resolution's worth of ticks in every meter.

Cheapest way to settle it:

- Option A: find the deadline check. Look for the caller that compares the SP-ready time against a fill's start, near the fill-to-activation code `0x20CFF60` or the engine's activation code `0x20F5640`.
- Option B, if A fails: build a 6/8 probe chart where SP becomes ready between 4 eighth-notes and 4 quarter-notes before a fill, then see whether the fill appears.

---

## 315. Are these Hydra fill rules Clone Hero's?

There are four rules. Clone Hero's code settles two of them and partly settles a third.

| Rule | Clone Hero | Strength |
|---|---|---|
| Ties go to the later chord | **Yes**, for attaching a fill to a chord | CH code |
| Distances cut to whole ticks | **Partly**, and the window is one tick wider than Hydra's | CH code |
| A fill ending before its start is dropped | **Yes in effect**: no chord in the window means no activation | CH code |
| One authored fill stops fill generation | **Unknown**: generator not found | none |

Evidence for the first three:

- `0x20CFF60` builds the activation list from the track's fills (`re\f20cff60.asm`).
- The slop is (whole ticks of resolution × 0.03125) + 1. That is computed at `0x20D0061`–`0x20D008D`: a `cvttsd2si` (round down to a whole number), then `inc`.
- For each fill it calls `0x5DE030(notes, target = fill end, low = fill start, high = fill end + slop)`.
- `0x5DE030` takes the nearest chord at or before the fill end, but not before the fill start. It also takes the nearest chord after the fill end, but not past fill end + slop.
  - If neither exists, the result is null and the fill is skipped.
  - If both exist, it compares distances: `if (after − target) <= (target − before) → after`, at `0x5DE1C5`–`0x5DE1CE`. So a tie goes to the later chord.
- The chosen chord gets the activation flag `0x800`. Fills with the bool at +0x20 set are skipped.

Hydra today, in `fill_lands_on_chord` (`src/parse/song.cpp` lines 106–113): the later chord wins only within `trunc(resolution × slop_beats)` ticks. Clone Hero allows one more tick: floor(res/32) + 1. A chord exactly floor(res/32) + 1 ticks after the fill end lands in Clone Hero but not in Hydra. That is 7 ticks at resolution 192 and 16 ticks at 480. Ties and dropped fills match.

For the rest: the length and placement of generated fills, and whether one authored fill turns generation off, are open. See question 53 for the cheapest check.

---

## 320. Does Clone Hero score .chart ghost and accent notes with no enabling tag?

**Answer: yes.** Strength: CH code.

Evidence:

- The .chart `N` handler `0x213D620` maps each modifier note through `0x213D160` (`re\f213d620.asm`):
  - N 34–38 become Accent (enum value 9).
  - N 40–44 become Ghost (10).
  - N 66–68 become Cymbal (7).
  - The enum is at `dump.cs` line 548173.
- Each modifier is stored in the track's modifier lists with no condition.
- The drum finish step `0x215E280` applies Cymbal, Tom, Flam, Ghost and Accent through `0x215DBD0` with no flag check (`re\f215e280.asm`, `re\f215dbd0.asm`). `0x215DBD0` never reads the track flags at +0xA1/+0xA2 or any dynamics flag.
- Only MIDI code uses the string `ENABLE_CHART_DYNAMICS`: the drum track reader `0x2155050`, plus `0x211BAD0`, `0x211C270` and `0x211E4C0`. Those last three take a whole string and look like a writer, not the .chart reader. I did not open them.

Hydra today: matches. .chart accents and ghosts are always applied.

---

## Things that surprised me

1. **Disco flip (11) is per difficulty.** Hydra applies Expert's disco spans at Hard and below. Clone Hero does not, so Hydra's Hard/Medium/Easy Pro Drums scores are wrong on charts with disco sections.
2. **The .chart phrase-end rule (21) is wrong in both edge cases.** Zero-length phrases are awarded when they should not be. Phrases that run past the last note are not awarded when they should be. Hydra's .mid side already matches Clone Hero.
3. **The per-side hit window at the widest spacing is about 85.7 ms, not 85.** The whole window at the cap is 171.43 ms, and the engine uses half of it. Clone Hero's floor is 96.29 ms whole (about 48.1 per side), at gaps of 75 ms or less. Each note's window follows both the gap before it and the gap after it. Each half-gap is clamped to 37.5–85 ms. This also answers steps 1–3 of `docs/superpowers/plans/2026-09-25-hit-window-testing.md` (cap, floor, which gap), with no live run needed.
4. **The fill-landing window is one tick wider in Clone Hero** (floor(res/32) + 1) than in Hydra.
5. **Clone Hero counts SP in chart ticks, not seconds.** The amount at +0x68 and the maximum at +0x1B0 are 64-bit whole numbers.
6. **MIDI 2x kick on every difficulty.** Clone Hero sets a 2x-kick flag (8) for MIDI notes 59, 71, 83 and 95, at `0x21555CD`. That means 2x kick exists at every difficulty, not just Expert+. This is not one of the twelve questions.
