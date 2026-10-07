# parse agent notes (key `parse`, agent a69bbcdcb48c8dfa1)

Worktree: C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\perf-parse (detached, uncommitted)
baseline\ = unmodified parse code + engine agent's --engine harness + my --parse harness (tools/bench.cpp), with
the engine agent's hydra_settings.ini (Expert, pro, bass2x on). all.txt = 18,869 notes paths from prof's B_real.csv
(every chart the batch ran, 58 fail). slow50.txt = 50 slowest parses from that CSV.

Harness `hydra_bench --parse <dir|list.txt> [--reps N] [--out tsv]`: single thread, per chart load_songpath_with_notes
(best of reps), digest of the whole Song (sequence, timecodes, maps, features, dynamics fields, practice/solo sections)
plus the stored dynamics blob; a failure hashes typeid + what(). Same hash = same parse output.

## Baseline whole library (lock, 0 compilers): parse 15.05 / 15.38 s single thread, dynamics count 3.85 s, hash 937d9aed20f59dff, 58 failed

## Profile (temporary phase timers, whole library single thread, lock, 0 compilers)
.mid (14.5k charts): file read 0.42 s, MidiFile decode (every track into 64-byte Messages) 2.77 s,
  pass 1 tempo 0.10, pass 2 drum track 3.0 s, pass 3 events 0.08, check_activations 0.24, MidiFile free ~1 s (unmeasured remainder).
  Pass 2 split (rdtsc): push_timestamp 3.6 s of 4.0 (classify+run ops 1.8, emit chord 1.1). It is volume: the drum
  track holds all four difficulties plus every note-off, and every message is classified.
.chart (4.2k): read 0.13, load_sections 4.96 s (!), sync 0.04, drum section 0.78, events 0.06, check_act 0.07,
  section-map free ~0.27.
.sng/.srb: 0.65 s total.
count_dynamics: 3.85 s (Chord::notes() allocates a vector per timestamp).
Swallowed ChartFileError throws: 215 in the whole library (.mid only) -> not a cost.

## Changes (all in the worktree, uncommitted; parse.patch). One build, HYDRA_PERF_OFF=dyn,midi,chart|all switches each off.
- dyn: count_dynamics reads the chord's five lanes in place instead of Chord::notes() (a vector per timestamp).
- midi: MidiFile::lean walks every track once for its name (and the same length checks/throws), then decodes only
  track 0's tempo/meter, the first PART DRUMS track's text metas + the note pitches MidiParser can act on
  (difficulty's 5 pads, its 2x kick when 2x Bass is on, marker on/off), and EVENTS text metas. Deltas fold forward.
- chart: load_sections_lean keeps full entries only for [Song]/[SyncTrack]; the drum section becomes a flat
  vector of 24-byte lines (is_sorted check, else stable_sort by tick); [Events] keeps only practice sections;
  every other section is only classified so it throws where the full read would. Numbers: [-]digits fast path
  (<=9 digits for stoi, <=18 for stoll), everything else goes to the original std::stoi/stoll.

## Results (lock, 0 compilers at start every run)
Parse-only harness, whole library 18,869 charts, single thread:
  quiet runs: base 14.88 / 15.31 s; dyn-only parse same, dyn 3.8 -> 0.48 s; midi-only 13.13 / 12.41; chart-only 11.69 / 12.00;
  all 9.13 / 9.92 (+ earlier 8.85). A,B x3 interleaved (noisy machine): parse medians A 20.2 s, B 11.4 s; dyn A 4.3, B 0.51.
  slow50 (best of 3 reps) A,B x3: A 1.465/1.380/1.360, B 0.611/0.611/0.618 -> 2.26x.
--engine harness: parse 15.81 -> 9.10 s, graph/analyze unchanged; row hash 8d17f958172bd6f4 both (18,811 charts), per-chart files identical.
hydra_batch --redo real db copy, 6 pairs over two runs: A 15.1 13.8 11.2 11.5 11.9 24.5 ; B 20.6 10.4 10.8 11.2 11.4 12.4.
  Medians 12.9 vs 11.3 but quiet pairs only differ ~0.3-0.8 s: batch stays bound by the single writer.

## Correctness
- per-chart Song digest + dynamics blob + failure type/message: whole library 18,869 charts, 0 differ, 58 failures same text,
  for each variant alone and combined; again with Hard / Pro off / 2x Bass off settings: 18,869 compared, 0 differ, 5,744 failures same.
- 41 crafted files (edge\, gen_edge.py): bad/overflowing numbers in skipped/drum/events/song sections, odd keys, duplicate and
  unterminated sections, unsorted ticks, CRLF, disco markers, missing Song/Resolution, bad header; MIDI with two drum tracks,
  late names, oversize meta in other/drum track, format 0, SMPTE, truncation, running status after meta, unknown chunk, bad status,
  zero tempo. 0 differ in either settings set; 21 / 32 failures with identical type + message.
- hydra_batch fresh dbs (db_A vs db_B, cmpdb.py): 18,811 charts; dynamics, paths, songmeta, meta identical; results/path_refs
  differ only in result_id (13,340 charts, write order).
- The Hard-settings alt run timing (A 105 s) is not trustworthy: the machine was busy (chart failures alone took 8x longer).
