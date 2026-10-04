# Handoff: timing a full library analysis, Hydra Deluxe vs public Hydra

Written 2026-09-29. Nothing from this session is still running.

## The short version

We set out to time one full analysis of the library (`C:\Clone Hero`, about 19,900 charts) under Hydra Deluxe 2.0.0 and under DragonDelgar's public Hydra, both at default settings.

Hydra Deluxe finished the whole library in 47 seconds. The public version never finished. Our attempt to speed it up, 20 copies at once, used up all 31 GB of RAM. Windows then swapped memory to the SSD (disk at 100%) and the whole PC slowed down. That swapping inflated the public timings by an unknown amount, so we threw those numbers out.

The next session should run the public version again with a few workers, no stored results and a RAM watchdog. The script for that is ready in `2026-09-29-public-timing-scripts/time_pool.py`. It has been syntax-checked but not yet run.

## Settings both runs used

Both apps default to Expert, Pro Drums, 2x Bass, the top 4 scores, and "limit timings" on at 10 ms. Hydra Deluxe also defaults to a 4-bar SP cap, which is fixed at 4 in the public version. Those are the settings to compare at.

## Hydra Deluxe: 47 seconds

We ran the installed 2.0.0 `hydra_batch.exe` into a fresh database, so nothing was skipped and the real `hydra.db` wasn't touched. It reads the settings file next to the exe, and the analysis settings there were already at the defaults.

```powershell
& "C:\Program Files\Hydra\hydra_batch.exe" --db <scratch>\timing.db
```

It took 47 seconds of wall time, measured around the whole command. That was 26 s to find and read the charts and 21 s to analyze them, on 8 threads. It found 19,883 charts. It analyzed 19,342 of them and skipped 541, all with "No Expert Pro Drums notes in this chart".

If someone asks for a per-core comparison, 21 s on 8 threads is about 3 CPU-minutes. For a true one-core number, rerun it pinned to a single core with `start /affinity 1`. We didn't do that.

## Public Hydra: how we ran it

The public app has no "analyze the whole library" button. It analyzes one song at a time. DragonDelgar's own folder script, `hydra_runfolder.py`, is the closest stand-in. The user picked the version on the public main branch (commit `1073ccd`, 2026-09-23). That's newer than the v1.3.1 release: it adds dynamic kicks and handling for SP that runs past the last note, about 60 lines of engine change.

The script has one gap. It never passes the 10 ms timing limit, so it searches with no limit at all. We added that one argument, plus timing printouts. The exact edit is in `2026-09-29-public-timing-scripts/runfolder_timed.diff`.

We ran it on the system Python 3.14 (`C:\Users\Patrick\AppData\Local\Python\pythoncore-3.14-64\python.exe`). That install already has DragonDelgar's mido fork (commit `ae9262d8`), the same one the released app bundles. The released app itself runs Python 3.13, so allow a few percent either way.

Get the code again with a long-paths clone, because some test folder names are too long for Windows' default:

```powershell
git -c core.longpaths=true clone https://github.com/DragonDelgar/hydra.git pubhydra
git -C pubhydra checkout 1073ccd
```

## What we learned

**The one clean public number.** A single process, with nothing else running, got through the first 2,249 charts in 394.5 seconds. That's about 0.175 s per chart for ordinary songs. Before that it spent 128 s finding and reading the charts; a second scan, once the files were cached, took only 27 s. Its checkpoints came every 250 charts, at 39.6, 78.8, 118.2, 164.6, 210.0, 255.7, 301.3, 351.3 and 394.5 s after the scan. That run was stopped on purpose, not by a problem, so these numbers are sound.

**Giant charts dominate the public version's time.** Ordinary songs take a fraction of a second. Discography and full-album charts take minutes each. Rise Against's "Discography (2024)" had run for over 18 minutes when we stopped it. Hawthorne Heights' "The Silence In Black And White (Album)" had run for over 14 minutes, and "Endless Setlist I" for over 7. Those were measured while the machine was overloaded, so their true alone-times are shorter, but they'd still be minutes. Hydra Deluxe did those same charts inside its 21 seconds.

**Why memory ran out.** It wasn't the JSON files being written. Each section wrote its file once, at the end, about 19 seconds in total. The problem was what was held in memory before that. The runfolder script keeps every finished result in memory until the end so it can write the JSON in one go, so each process keeps growing. The giant charts also need a lot of memory while they're analyzed. Twenty processes at once multiplied both effects until RAM was full.

**Splitting the library into fixed sections balances badly.** The giant charts cluster in a few folders. Of the 20 sections, 15 finished in 2½ to 9 minutes each. The other 5 each sat on a single huge chart. A shared queue, where each worker takes the next chart when it's free, avoids that.

**You can see inside a running Python 3.14 worker.** `sys.remote_exec(pid, "script.py")` runs a small script inside another Python process without stopping it. We used it to read each worker's loop variables: which chart it was on and how many it had done. It answers only when that process next runs Python code, so a worker deep in a long C call can take a minute to reply.

## The discarded numbers, for context only

With 20 processes, the 15 sections that finished covered 14,452 charts in 5,104 summed seconds. Those processes ran 2.17× slower than alone, measured on the same first 2,249 charts, which puts those charts at about 39 minutes on one core. But we don't know when swapping started. Treat that figure as unreliable, and don't quote a public total from this run.

## How the next session gets a better estimate

1. **Start with a quiet machine.** Close anything heavy, and keep Task Manager's memory and disk graphs in view. Check that no leftover `python.exe` is running.

2. **Rebuild the chart list.** Clone the public code as above. Copy `split_discover.py` into the clone and run it from there:

   ```powershell
   python split_discover.py "C:\Clone Hero" charts.json
   ```

   It finds charts exactly the way the runfolder does, drops duplicates the same way, and records each chart's original position. That position is what lines the new timings up with the 394.5 s baseline.

3. **Smoke-test the new runner on 20 charts** before trusting it:

   ```powershell
   python time_pool.py charts.json out_smoke --hydra-root pubhydra --workers 2 --limit 20
   ```

   Check that `out_smoke\times.csv` has 20 rows with sensible times and memory figures, and that `ram.csv` is filling in.

4. **Time the biggest charts alone first.** Run the 40 largest one at a time, with a fresh process for each:

   ```powershell
   python time_pool.py charts.json out_big --hydra-root pubhydra --workers 1 --recycle 1 --only-largest 40
   ```

   This gives clean times for the expensive charts, and the `peak_mb` column shows the most memory one chart needs. Divide the RAM you can spare (say 20 GB) by that peak. That's how many workers are safe. File size is only a rough guide to which charts are biggest, since a `.sng` also holds audio. Look at the slowest rows, and raise the count if giant charts are still showing up near the 40th.

5. **Time everything else with a few workers.** Run the rest with at most 6 workers. There are 8 physical cores, and staying below that keeps the workers from slowing each other much:

   ```powershell
   python time_pool.py charts.json out_rest --hydra-root pubhydra --workers 4 --skip-largest 40
   ```

   The runner keeps no results and restarts each worker every 50 charts. It warns if free RAM drops below 6 GB and stops itself below 3 GB. A stopped run keeps every row it already wrote.

6. **Add it up.** The one-core estimate is the sum of `wall_s` from both runs, plus the discovery time. In `summary.json`, `baseline_ratio` compares this run's times for the first 2,249 charts against the clean 394.5 s. Near 1.0 means the workers didn't slow each other; if it's higher, divide the rest-run sum by it. If `baseline_ratio` is missing, one of those first 2,249 charts went to the big-chart run instead; compute the ratio by hand from both `times.csv` files. Any row with `low_ram` = 1 ran while RAM was tight, so check those before trusting the total. `sum_cpu_s` is a second estimate that ignores waiting, and it should land close to the wall sum.

7. **Compare fairly.** Put the public one-core estimate next to Hydra Deluxe's 47 s wall time (21 s of analysis on 8 threads). If one-core is the fair comparison, add the `start /affinity 1` Deluxe run too.

The runner leaves out the JSON and CSV writing the real runfolder does at the end. On 2026-09-29 that writing took about 19 s across 14,452 charts, too small to matter.

## Where the speedup comes from: blink-182 tests (finished 2026-10-03, results in the next section)

The user then asked why Hydra Deluxe is so much faster, and to test each reason on the blink-182 discography chart (`C:\Clone Hero\songs\Misc Downloads\blink-182 - Discography\notes.mid`, one 3 MB MIDI). The user paused this partway. Nothing is running.

**Hydra Deluxe on blink:** 0.23 s in total, from `build-cpp\Release\hydra_bench.exe "<blink folder>"` at the app defaults. That was 0.09 s reading the file and 0.14 s searching, best score 70,757,755, 20 paths. It was run twice with the same result.

**The test tool.** `blink/blink_probe.py` runs the public engine on one chart with the runfolder's settings and the 10 ms limit. It records where the time goes: reading the file, building the score graph (the map of notes and scoring options the search walks), the pruning step, path copying and the timing-limit check. It also records the pool size and memory. It stops itself if free RAM drops under 5 GB or it runs past `--timeout`, and still writes its numbers. Run it from inside the public clone, since it imports `hydra.*`. It changes no engine code; its options only wrap methods at run time. They are:

- `--fix` adds Hydra Deluxe's pruning of too-hard paths: a path that fails the timing limit drops out once more than 4 distinct scores in its group beat it.
- `--share` stops each branch from deep-copying its tied variants. Branches share them, and each variant is copied once, right before `prepare_variants` fills it in at the end.
- `--nolimit` turns the 10 ms limit off.

**What we learned so far:**

With `--fix` on, the public code still hit the 15-minute timeout at 66% of blink. That was not a memory problem: RAM peaked at 1.2 GB, and free RAM never dropped below 12.7 GB.

- **Reading the file took 117 s**, against Deluxe's 0.09 s. That's the mido MIDI library checking a 3 MB file byte by byte.
- **Building the score graph took 26 s.**
- **The fix kept the pool tiny:** at most 125 paths, at most 21 per group, with 23,091 too-hard paths dropped. The every-pair pruning cost only 23 s of the run. So on blink, pairwise pruning is not the bottleneck once the fix is in.
- **Copying tied variants was the bottleneck:** 704 s. There were 88,752 path copies, and they dragged along 70 million variant copies, growing faster as the song went on. Between 56% and 66% through, variant copies went from 21.5 million to 70 million.
- **The timing-limit check** (it re-walks each path's activations every step) cost 21 s.

**Checking that the two changes don't alter results.** Before trusting `--fix` and `--share` on blink, each chart runs four ways (`orig`, `share`, `fix`, `both`), and the paths and scores are compared exactly (`blink/compare.py`). Four charts are done, and all four modes gave identical results on each: two ordinary songs and two full-album `.chart` files. The stop interrupted the fifth, Helloween's "Halloween (2x Bass Pedal)", at 92% of its untouched run. Its partial numbers matter: the pool reached 3,464 paths, with 829 in one group, and pruning took 6.1 of its 7.1 s. So pairwise pruning does dominate on some charts, just not on blink. Charts 6–8 (Metallica's St. Anger, Protest The Hero's Fortress, The Faceless' Planetary Duality) haven't run. Their paths are in the verify loop in the section below.

**To resume:**

1. Re-clone the public code as described above, and copy `blink/blink_probe.py` into the clone.
2. Rerun the verification loop for charts 4–7 (the loop below), then run `compare.py` on the results folder. Every mode must say SAME before any blink number is trusted.
3. Run blink with `--fix --share --timeout 1800`. If that finishes, it gives the public engine's time with both of Deluxe's algorithm changes, and a best score to compare with Deluxe's 70,757,755. Deluxe's other rule changes (ghost/accent kicks, the 500 ms squeeze window) may shift the score slightly. Then run `--share` alone (limit on, no fix) to see what the pruning fix alone is worth, and the untouched code with the same timeout to record how far it gets.
4. Write up each reason with its measured share. Two reasons can't be tested on blink. Multiple cores only help across many charts, and one chart runs on one thread in both apps. C++ versus Python is measured by comparing the `--fix --share` Python time against Deluxe's 0.23 s, with the file-reading difference (117 s vs 0.09 s) reported separately.

The verify loop, run from inside the clone. Point `$s` at `2026-09-29-public-timing-scripts/blink/results`, so the new files sit beside the first four and `compare.py` finds them all:

```powershell
$py = "C:\Users\Patrick\AppData\Local\Python\pythoncore-3.14-64\python.exe"
$charts = @(
  "C:\Clone Hero\songs\synchotic\Sync Charts\Rock Band\Rock Band Network\Wrong Side of Dawn - The Grinder's Tale\notes.mid",
  "C:\Clone Hero\songs\synchotic\Sync Charts\Rock Band\Rock Band Network\Wretched - Dilated Disappointment (2x Bass Pedal Expert+)\notes.mid",
  "C:\Clone Hero\songs\synchotic\Sync Charts\Fig Drum Charts\The Callous Daoboys\(2025) I Don't Want to See You in Heaven\14. - I Don't Want to See You in Heaven (Full Album)\notes.chart",
  "C:\Clone Hero\songs\synchotic\Sync Charts\Fig Drum Charts\Knocked Loose\(2024) You Won't Go Before You're Supposed To\You Won't Go Before You're Supposed To (Full Album)\notes.chart",
  "C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\Xane60\Helloween\Helloween - Halloween (2x Bass Pedal)\notes.mid",
  "C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\MMSRhino\Metallica - St Anger (Full Album)\notes.mid",
  "C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\Xane60\Protest The Hero\Protest The Hero - Fortress (2x Bass Pedal)\notes.mid",
  "C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\SoundHaven\The Faceless\The Faceless - Planetary Duality (B)\notes.mid")
$modes = @{ orig = @(); share = @('--share'); fix = @('--fix'); both = @('--fix', '--share') }
for ($i = 4; $i -lt $charts.Count; $i++) {
  foreach ($m in 'orig', 'share', 'fix', 'both') {
    & $py blink_probe.py $charts[$i] "$s\c$i-$m.json" @($modes[$m]) --timeout 300 | Out-Null
  }
}
```

## Blink results (2026-10-03)

**The two changes don't alter results.** All four modes gave identical paths and scores on all eight test charts. St. Anger tests nothing, though: its search never branched (one path, zero copies), so the real evidence is seven charts. The pruning fix alone made the heavy ones much faster: Helloween went from 9.2 s to 0.12 s, Fortress from 12.8 s to 1.0 s, and Planetary Duality from 24.8 s to 0.77 s. Sharing alone barely helped them.

**Blink needs both changes to finish.** Each change fixes a different wall, and either wall alone stops the run:

| Run | Outcome | Where the time went |
|---|---|---|
| Untouched | Stopped at 12% after 14 min: free RAM fell under 5 GB (the process reached 5.8 GB) | Copying tied variants, 671 s (42 million copies) |
| Pruning fix only (2026-09-29) | Timed out at 66% after 15 min | Copying tied variants, 704 s (70 million copies) |
| Sharing only | Timed out at 83% after 30 min | Pairwise pruning, 1,568 s (3.2 billion comparisons, up to 2,946 paths in one group) |
| Both | **Finished in 205 s** | Reading the file 125 s, building the score graph 26 s, searching 51 s |

With both changes, the search held at most 130 paths and used 315 MB at peak.

**What's left is Python versus C++.** With both of Deluxe's algorithm changes in, the public engine takes 205 s on blink. Deluxe takes 0.23 s. Reading the MIDI file is 125 s against 0.09 s, which is the mido library walking 3 MB byte by byte. Building the graph and searching is 77 s against 0.14 s, about 550 times slower. Those Python times include the probe's timing wrappers, which add a little.

**The best scores differ slightly.** Public with both changes found 70,747,675. Deluxe finds 70,757,755, which is 10,080 higher. Deluxe's other rule changes (ghost/accent kicks, the 500 ms squeeze window) are the likely reason, but nobody has checked which one. Public also reports 13,824 paths against Deluxe's 20. That's most likely each tied variant counted as its own path, but that hasn't been checked either.

Multiple cores were not tested here. They only help across many charts, and one chart runs on one thread in both apps. The library timing above is the place to measure that.

The result files are `blink-both.json`, `blink-share.json` and `blink-orig.json` in `blink/results/`, beside `c4`–`c7`.

## Giants: fixed runs, then an untouched estimate (2026-10-03)

The user didn't want to run the untouched public code on the whole library locally. Instead we timed the giants with both fixes, then estimated their untouched time from short untouched runs.

**Which charts count as giants.** There are 239: every chart whose `song.ini` gives a length of 30 minutes or more (185), plus all 54 `.sng` files. The ranking script doesn't read a `.sng`'s length, and most of the 54 are full albums or setlists, including the three Endless Setlists at about 1 GB each. The chart list came from `split_discover.py`: 19,846 found, 19,269 unique, 198 s to discover.

**With both fixes, the giants are cheap.** 237 of the 239 finished, in 1,524 s summed (about 25 minutes, run 4 at a time). Running 4 at once inflates the times: blink took 272 s here against 205 s alone. Only blink, Nirvana's Endless Nameless Setlist, Rise Against, Hail The Sun and Endless Setlist I took over a minute. Endless Setlist II and III got to the end of the song, then ran out of memory in the last step. That step gives every tied variant its own copy of the remaining activations (`prepare_variants`). Endless Setlist II passed 8 GB there. The untouched code builds the same final results, so neither version finishes those two on this PC with the user's usual apps open (about 10 GB free).

**Untouched, 226 giants finish quickly.** Each giant got an untouched run, 2 at a time, capped at 2 minutes of search, with a progress sample every 2 s (`--trace`). 226 finished inside the cap, in 1,163 s summed. Those are exact.

**13 giants hit the cap and were projected.** The projection assumes the rest of the chart runs at the speed of the run's last quarter. The untouched code keeps slowing down, so this is a lower bound. On 13 charts that did finish, a projection cut at a quarter of their real time came out 1.2–2.4 times too low; cut at halfway, 1.0–1.5 times. It never came out too high. The 13 projections sum to at least 87,166 s (24.2 h):

| Chart | Reached in 2 min | Untouched, at least |
|---|---|---|
| blink-182 Discography | 12% | 13.6 h (about 24 h using the 2026-10-03 full run, which reached 12.5% after 713 s of search and was still slowing) |
| SoundHaven UwU Daddy (1 Hour) | 20% | 3.6 h |
| Rise Against Discography | 41% | 2.3 h |
| Neck Deep Discography | 55% | 1.4 h |
| Hail The Sun Discography | 41% | 58 min |
| Area 11 Discography | 76% | 46 min |
| Endless Setlist II | 39% | 25 min, and the final step won't fit in memory anyway |
| Endless Setlist III | 31% | 22 min, same memory problem |
| blink-182 NINE | 70% | 16 min |
| Endless Setlist I | 70% | 16 min |
| blink-182 ONE MORE TIME... | 68% | 9 min |
| Hawthorne Heights, The Silence In Black And White | 79% | 6 min |
| Killswitch Engage, As Daylight Dies | 99% | 2 min |

So the giants alone need at least 24.5 hours untouched, or about 35 hours if you use the longer blink run. The true figure is likely well above that, since every projection that could be checked was too low. Memory is a second wall. Several of these were at 1.5–3 GB after 2 minutes and still climbing. Whether they'd fit in 31 GB before finishing is unknown, and the script's memory projection is a crude straight line, so it isn't quoted.

The rest of the library, about 19,030 ordinary charts, wasn't run this time. At the 2026-09-29 clean rate of 0.175 s per chart, that's roughly 55 minutes on one core.

**Score range 0 barely helps the worst giants.** The user asked how the giants do with the score range set to 0 (only the optimal path, the setting in the app's More Paths box). We added `--depth N` to the probe and repeated the untouched pass at depth 0. 227 giants finished inside the 2-minute cap (1,007 s summed, against 1,163 s for 226 at depth 4), and 12 hit the cap. The plainest comparison is how far each capped chart got in the same 2 minutes:

| Chart | Depth 4 | Depth 0 |
|---|---|---|
| blink-182 Discography | 12% | 12% |
| SoundHaven UwU Daddy (1 Hour) | 20% | 20% |
| Rise Against Discography | 41% | 42% |
| Hail The Sun Discography | 41% | 41% |
| Neck Deep Discography | 55% | 56% |
| Area 11 Discography | 76% | 78% |
| blink-182 NINE | 70% | 73% |
| blink-182 ONE MORE TIME... | 68% | 75% |
| Hawthorne Heights, The Silence In Black And White | 79% | 89% |
| Endless Setlist I | 70% | 88% |
| Endless Setlist II | 39% | 75% |
| Endless Setlist III | 31% | 64% |
| Killswitch Engage, As Daylight Dies | 99% | finished, 45 s |

The reason is that depth 0 still keeps every tie as a variant, and copying tied variants is what eats the time on these charts. The depth-0 projections sum to at least 53,844 s (15 h). That's lower than depth 4's 24 h, but those two figures shouldn't be compared closely: projections made from 12–40% of a chart are noisy. Rise Against, for example, projected higher at depth 0 than at depth 4 despite reaching the same point. One calibration chart at depth 0 (Dream Theater, Awake) projected 1.2 times too high from a quarter-way cut, so the "lower bound" is a strong tendency, not a guarantee.

The first depth-0 pass was hit by something else on the machine using nearly all the RAM: free memory touched 0.1 GB while each probe used about 50 MB. That stopped 65 runs, which were rerun cleanly. Ten small charts finished during that crunch, 6 s in all, which is too little to matter. The cause wasn't found. A `preview_bench` process from another session started during the run but wasn't confirmed as the cause. SignalRGB showed 11.5 GB of private bytes, but only about 135 MB of it was in RAM, so it wasn't the cause.

The scripts are `blink/run_giants.py` (runs the probe over the giants, resumable), `blink/estimate.py` (exact or projected time per giant; `--calibrate` checks the projection on charts that finished) and the new `blink_probe.py` options `--trace`, `--search-timeout` and `--min-free-gb`. The run outputs were in the session scratchpad and are gone; the table above is the record.

## Best guess for a full library run (2026-10-03)

This is the untouched public code at default settings (score range 4, 10 ms limit), on one core, assuming enough memory. It's built only from runs already made.

| Part | Charts | Measured or projected | Best guess |
|---|---|---|---|
| Finding the charts | — | 128–198 s cold, 27 s warm | 3 min |
| Ordinary charts | 19,030 | 0.175 s each (the clean 2026-09-29 run of the first 2,249, which hold no giants) | 56 min |
| Giants that finished untouched | 226 | 1,163 s, exact | 19 min |
| Capped giants, except blink | 12 | at least 10.6 h projected | 17 h (projection × 1.65) |
| blink-182 Discography | 1 | at least 24 h | 40 h (× 1.65) |
| **Total** | 19,269 | **at least 36 h** | **about 2½ days** |

The 1.65 is the middle of the calibration: projections from a quarter-way cut came out 1.2–2.4 times too low, with a middle value of 1.65. The capped giants were cut far earlier than a quarter of their time, so the true factor is probably larger. Using the worst calibration factor, 2.4, gives about 3½ days. So the honest range is 1½ days at the very least, 2½ as the best guess, 3½ or more as plausible.

More workers don't shorten it much. blink runs on one thread, so even on a big machine the run lasts at least as long as blink: a day at minimum, closer to two by the best guess. Everything else fits alongside it on a few workers.

Memory probably stops it on this PC first. blink was at 5.8 GB when 12.5% through and still climbing, so it likely needs more than 31 GB. Endless Setlist II and III need more than 8 GB for their final step, which only fits with other apps closed.

Score range 0 doesn't change the picture. The four slowest giants made the same progress at either setting.

For contrast, Hydra Deluxe does the whole library in 47 s. The public code with Deluxe's two search changes did all 237 giants it could finish in about 25 minutes. Its ordinary charts weren't run that way, but they can't take longer than the untouched 56 minutes, so that whole run would come in under about 1½ hours.

## Files

Everything this session made lived in its scratch folder, which is gone now. What's worth keeping is beside this file, in `2026-09-29-public-timing-scripts/`. `split_discover.py` builds the chart list. `runfolder_timed.diff` is the 10 ms and timing edit to DragonDelgar's script. `time_pool.py` is the new memory-safe runner. `blink/` holds the one-chart test tool, the result checker, and every result file from the blink and verification runs so far.
