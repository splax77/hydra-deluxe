# Hydra memory audit (2026-10-07)

You asked for a full audit of Hydra's memory: what really needs to be in memory, what could stay out, how analysis manages memory, and whether Hydra frees what it no longer needs. Three agents read the code (the open app, the analysis pipeline, and freeing), three scouts searched the web for C++ memory practice, tools and libraries, and one whole-library run measured where the batch's peak goes. Nothing was changed; this is a report.

## The short answer

Hydra has no leak that grows in normal use. Every database handle, audio decoder, file map, network handle and GPU object it opens is closed again, on every path we traced. One real leak exists, but only when the Preview fails to start (a broken install or GPU), and it repeats once per song opened in that state.

The memory Hydra uses splits into two very different pictures. The open app at rest is small: about 125 MB with your library loaded, measured. A whole-library analysis is big: about 700 MB at its peak with mimalloc, measured today. But more than half of that peak is not data Hydra is using. It is memory mimalloc has already been given back and keeps for reuse. The same build with mimalloc switched off peaks at 444 MB.

The biggest real costs, in order: mimalloc's kept memory during a batch (it commits about 400 MB more than the segment heap at the peak), a few giant charts that can run side by side and briefly need tens of MB each, every finished chart's full parsed data waiting in line to be written (about 25 MB, more when a giant is in line), the fonts (27 MB for the app's whole life), and the library list (about 27 MB).

## What one whole-library run showed

This is the run the analysis agent asked for, done today through the benchmark lock with 0 compilers running: `hydra_batch --redo` on a fresh backup copy of your installed database, with today's main build (so it includes the summary-only storage change, D87, and its one-time upgrade of the copy). The same exe ran twice: once with mimalloc reporting its statistics, once with mimalloc's redirect switched off so the Windows segment heap did the work.

| Allocator | Peak working set | Peak private (committed) memory |
|---|---|---|
| mimalloc (shipped) | 700 MB | 874 MB |
| Windows segment heap, same exe | 444 MB | 479 MB |

mimalloc's own statistics say the memory Hydra was actually holding peaked at about 190 MiB in ordinary blocks, plus up to 166 MiB in very large blocks (a big chart's file buffer or note arrays). Those two peaks may not have happened at the same moment, so the true live peak is somewhere between 190 and 355 MiB. mimalloc had 845 MiB committed at its peak. So somewhere between 490 and 655 MB of that commit was freed memory mimalloc was keeping, and against the segment heap's 479 MB, mimalloc costs about 400 MB at the peak.

The same statistics show something else worth knowing: the run made 65 million allocations totalling 85 GiB, to analyze about 19,000 charts. That is about 4.5 MB allocated and freed per chart, against a peak of 1 to 2 MB per ordinary chart. Hydra allocates and frees the same memory over and over. That churn is what an allocator's cache is built for, and it is why mimalloc cut CPU time by 10%. It is also why mimalloc keeps so much: it holds on to what it expects to be asked for again. (The mimalloc run's wall time, 26 s, is not comparable with the second run's 10.7 s; it ran first and read the song files from disk.)

## What needs to be in memory, and what could stay out

Here is what the open app keeps, measured where it says so and estimated otherwise. The harness that measured it was the headless UI test runner on a copy of your database: 88 MB with an empty library, levelling off at 124 MB with yours.

| What | About how big | Needs to be in memory? |
|---|---|---|
| Library list, one row per chart (19,436 rows) | ~27 MB est.; the load step measured ~25 MB working set | The core does: sorting and filtering run in C++ on purpose, and the Stale/Ready rule lives there. About 10 MB could go (below). |
| Font files | 27 MB, measured file sizes | No. They could be memory-mapped instead of copied, so Windows only keeps the pages it uses. |
| Preview's GPU render targets | ~40 MB est. at a typical panel size | Only while the Preview is open. They are kept after it closes, by design, to reopen faster. |
| Report data after a library batch | ~45 MB est. | No. It is kept until you dismiss the finished strip, though the report is already written. |
| The open song's result and Preview scene | small for normal songs, MBs for the longest | Yes, while the song is open. |
| Container song audio (.sng/.srb stems, compressed) | ~5 to 10 MB typical | While previewing. Freed on close. |
| SQLite's page cache | ≤2 MB (defaults; no cache or mmap settings anywhere) | Fine as is. |

Things that are already right: Hydra never holds a song's decoded audio whole (it decodes 4,096 frames at a time, and loose audio files are memory-mapped). It never keeps a per-chart cache between songs. Texture pixel data is freed right after it goes to the graphics card. No thread pool outlives its job. The database going from 292 MB to 13 MB under D87 barely changes RAM, because SQLite never had the old file in memory.

The library rows carry about 10 MB that only the clicked song needs: a third copy of each title, artist and charter (beside the display and search copies), the chart's signature text, seven path-summary fields the table never reads, a pre-built "best" label that could be formatted for the ~40 visible rows, and the md5 as 32 characters of text instead of 16 bytes. Paging the whole list from SQLite instead is not worth it: the filter is in C++ by design, and paging would mean writing the status rule a second time in SQL.

The two fonts may also overlap. The bundled Shippori Antique B1 is a Japanese typeface, and Yu Gothic is merged in as the Japanese fallback; if Shippori already covers the characters, dropping the merge saves 14 MB. That changes which glyphs some titles draw with, so it is your call, and it needs a check of which titles would change first.

## Memory during analysis

One ordinary chart needs only 1 to 2 MB at its peak; one real testdata chart measured 1.3 MiB. Memory grows in a straight line with chart length, about 0.6 KB per chord. Synthetic charts at 16 and 40 times normal length measured 30 and 67 MiB. Parsing is about 85% of a chart's peak. The library's giants (the Endless Setlists, the discographies) are in that range, so each one needs an estimated 50 to 150 MB at its peak.

Five things drive the batch's live memory. All five are proven by reading the code; the sizes are estimates unless marked.

First, every finished chart waits to be written while still carrying its whole parsed song and its whole analysis result, though the writer only needs the song to encode the tempo map. Up to about 40 finished charts wait at once (2 per worker, the 16 in line, and the open save group). That is about 25 MB for ordinary charts, plus 20 to 60 MB for each giant in line. The perf exploration already built the fix (encode the tempo map on the worker, then drop the song and result) and found it costs no time.

Second, the big per-chord arrays are grown one push at a time with no size given up front. While an array grows, the old and new copies both exist, so a giant's parse briefly needs about 2.5 times its final size. The x40 chart's measured 58 MiB parse peak matches this almost exactly. The arrays also keep up to 50% spare room for the rest of their life. The score graph's row array grows the same way, though its final length is known in advance.

Third, the score graph keeps a second copy of every chord's timing and notes, and each activation copies those rows again. Fourth, path variants deep-copy their parent's activations instead of sharing them. Fifth, the search keeps every branch it ever tried, live or dead, until the chart is done.

Giants can also run together. The batch hands out charts in folder order, and the three Endless Setlist files sit next to each other, so they start on three workers at once. The earlier write experiments saw the peak rise when big charts were grouped.

Two smaller items: `hydra_batch` keeps its scan results (about 30 MB est.) alive for the whole run, and a chart with no stated length is parsed a second time to find it while the first parse is still alive.

## Does Hydra free what it no longer needs?

Yes, with these exceptions.

The one real leak is in the Preview renderer (`src/render/preview_renderer.cpp`). It is created with a plain `new`, and if its setup fails part-way (a missing asset file or a GPU error), its destructor never runs. Everything it had made so far stays allocated. Opening another song tries again and leaks again. It needs a broken install or GPU to happen at all. The fix is to hold it in a `std::unique_ptr`. Four tiny icon views are also never released before the GPU device at exit, about 16 KB, harmless.

Several things are kept longer than they need to be. The library batch's report data (about 45 MB est.) stays until you dismiss the finished strip. Each waiting chart's analysis result stays after it has been turned into bytes for the database. A .sng or .srb container file stays in memory through the rest of the Preview's load, after its audio has been copied out. The Preview's GPU targets stay after it closes. A 1 MB scan buffer stays on each reused scan thread. None of these grow over time; they are held, then freed.

The headless UI test runner has a real problem of its own. It draws frames without ever waiting for the software GPU to finish them, so frames pile up during a long `wait`: one test script climbed to 21 GB before draining. The real Hydra.exe waits for each frame (vsync), so it cannot do this. The test runner needs a flush and a wait every few frames.

Hydra has no leak checking anywhere today: no CRT debug heap checks, no AddressSanitizer build, no test that watches memory.

## What the web research says, for C++ and for Hydra

**Standards.** The rules that matter most here are the C++ Core Guidelines' resource rules: every resource is owned by an object that frees it (RAII), no naked `new`/`delete` (R.11, which MSVC can enforce as warning C26409), and `unique_ptr` for single ownership. The Preview renderer's leak is exactly the case R.11 exists for. Two library facts explain several findings: `vector::clear()` keeps its capacity ([cppreference](https://en.cppreference.com/w/cpp/container/vector/clear)), so a reused buffer holds its biggest-ever size, and `reserve()` up front avoids the double copy during growth ([Abseil performance hints](https://abseil.io/fast/hints.html)).

**Windows facts.** Freeing memory does not promise Windows gets it back. Microsoft says the heap "cannot guarantee" a freed block is decommitted ([Microsoft Learn](https://learn.microsoft.com/troubleshoot/windows/win32/heap-manager-may-not-decommit-memory)), and the same is true of mimalloc until it purges. Working set (what Task Manager shows) is not the same as committed memory; to ask "did Hydra give it back", watch private bytes. Trimming the working set by force only hides pages that fault straight back in.

**mimalloc's own controls.** mimalloc returns freed memory after a purge delay (1 second by default in v3); 0 returns it at once ([mimalloc environment docs](https://microsoft.github.io/mimalloc/environment.html)). Hydra's trial measured that cost: purge delay 0 dropped the peak to 333 MB but slowed the batch from 9.6 to 13.1 s. A middle path is one `mi_collect(true)` call when a batch ends, which forces a purge once ([mimalloc docs](https://microsoft.github.io/mimalloc/group__extended.html)). mimalloc also has per-task heaps: create one for a chart's analysis and destroy it in one call, which frees everything the chart allocated at once ([mimalloc heap docs](https://microsoft.github.io/mimalloc/group__heap.html)). Version 3 allows these heaps across threads. One outside field test (Tor relays, January 2026) found mimalloc v3 used far more memory than v2 on its workload ([tor-relays list](https://lists.torproject.org/mailman3/hyperkitty/list/tor-relays@lists.torproject.org/thread/QPTWCADWNPJYTOPRNGH2ND3QXHNF5M3S/)); that is a Linux server, so it is a reason to measure, not a verdict.

**Arenas.** The standard's `std::pmr::monotonic_buffer_resource` is the textbook "build, use, free all at once" allocator, a good fit for one chart's analysis. Its published speed gains are micro-benchmarks; real programs saw 1.1 to 1.2 times ([Meeting C++](https://meetingcpp.com/blog/items/Could-a-polymorphic-memory-resource--PMR--improve-last-weeks-results-.html)). The stronger reason to use one is releasing a chart's memory in one step. A mimalloc heap does the same without changing every container's type.

**Tools.** Because mimalloc replaces `malloc`, most Windows memory tools see only its big chunks. The scouts' recommended kit is three things. mimalloc's own statistics (`MIMALLOC_SHOW_STATS=1`) for "how much and what peak"; that is what measured today's split. Visual Studio's Memory Usage profiler or Windows Performance Analyzer, run with mimalloc's redirect switched off (`MIMALLOC_DISABLE_REDIRECT=1`), for "which code allocates it". And a CRT debug-heap leak test in the doctest suite (a Debug build with `_CrtMemCheckpoint` before and after a test, asserting nothing is left), for "is anything leaking". AddressSanitizer is worth a CI build for memory corruption, but it does not find leaks on Windows ([Microsoft Learn](https://learn.microsoft.com/en-us/cpp/sanitizers/asan)). Intel Inspector is discontinued, the original Visual Leak Detector is dead (a Microsoft fork is maintained), and Heaptrack is Linux-only.

**Libraries we'd skip.** jemalloc and tcmalloc have weak Windows support; rpmalloc and snmalloc used more memory than mimalloc in the one Windows comparison found ([LLVM, 2020](https://lists.llvm.org/pipermail/cfe-dev/2020-July/066101.html)). Flat maps and small vectors only pay where a profile shows millions of tiny containers; nothing here shows that yet.

## Recommendations

These change only code, so they can go ahead on my recommendation and be reported after:

1. **Free each chart's song and result on its worker** once the database row is built (the HYDRA_LEAN prototype). Saves about 25 MB steady and 20 to 60 MB per giant in line; no time cost was measured before.
2. **Give the big per-chord arrays their size up front**, and trim the song after parsing. Cuts a giant's parse peak by about a third, and should cut some of the kernel time spent committing fresh memory.
3. **Hold the Preview renderer in a `unique_ptr`**, which fixes the one real leak.
4. **Free the batch report data as soon as the report is written**, and the scan leftovers in `hydra_batch`.
5. **Map the font files instead of copying them** (no visible change), saving up to 27 MB of private memory.
6. **Trim the library rows** of the fields only the clicked song needs (about 10 MB).
7. **Fix the UI test runner's frame backlog** (flush and wait every few frames).
8. **Add a leak test**: a Debug doctest helper using CRT checkpoints with mimalloc's redirect off.

These need your decision first:

- **What to do about mimalloc's kept memory.** The options are one `mi_collect(true)` when a batch ends (cheapest, needs a timing run), a per-chart mimalloc heap destroyed when the chart is done, a purge delay between 0 and 1 second, or going back to the segment heap. It is worth about 400 MB of committed memory at the batch peak.
- **Not letting giants run side by side**, for example only one chart above some size parsing at a time. That needs a size threshold, which is a new number.
- **Dropping the Yu Gothic font merge** (14 MB), which may change how some titles draw.
- **Releasing the Preview's GPU targets when it closes** (about 40 MB), which makes reopening it a few ms slower.

## Where this came from

Agent reports, each read-only: the open app (measured with `hydra_uitest` on a database copy), the analysis pipeline (measured with `hydra_bench` on one real and four synthetic charts), and freeing (code trace plus one scripted test run). Three web scouts on standards, tools and libraries; their full source lists are in this session's transcript, and the links above are the ones the findings rest on. The whole-library split was measured by the main session through `tools/bench_run.ps1`. Line numbers in the agents' notes were read while other sessions had uncommitted edits in the shared checkout, so treat them as approximate.
