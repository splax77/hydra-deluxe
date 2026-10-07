# The mimalloc trial (D86.4), 2026-10-07

This measured Microsoft's mimalloc memory allocator against the Windows segment heap that Hydra already uses. Nothing from the trial merged. It ends in this report, and shipping is the user's call.

## The answer

mimalloc made Hydra no faster and used far more memory. Re-analysing the whole library took 9.6 s with mimalloc and 9.7 s without it, which is the same within one run's noise. Peak memory went from 412 MB to 735 MB. My recommendation is not to ship it. The numbers show a cost and no gain.

| `hydra_batch --redo` on a fresh copy of the real database | Wall time | Peak working set | Peak private memory |
|---|---|---|---|
| Without mimalloc (segment heap) | 9.7 s | 412 MB | 448 MB |
| With mimalloc v3.5.3 | 9.6 s | 735 MB | 838 MB |

Both runs went through the benchmark lock with 0 compiler processes running, one after the other, with the song files already in memory. The "without" run is the new-build run from the combined timing ([2026-10-07-perf-speedups-results.md](2026-10-07-perf-speedups-results.md)). Both builds come from the same commit, c2fdb43; only the allocator differs.

## Why it didn't help (measured after the first report)

mimalloc did its job, but its job isn't what limits the run. A second pair of runs also recorded how much CPU time the process used:

| `--redo`, 16 cores | Wall time | CPU time (user + kernel) | Peak working set |
|---|---|---|---|
| Without mimalloc | 10.4 s | 17.2 s + 9.6 s | 425 MB |
| With mimalloc | 9.6 s | 15.5 s + 9.2 s | 745 MB |
| With mimalloc, `MIMALLOC_PURGE_DELAY=0` | 13.1 s | 18.8 s + 14.9 s | 333 MB |

mimalloc cut the program's own CPU work by about 10% (17.2 s to 15.5 s). That is a real allocator gain, and it shows the redirect was working. But the machine has 16 cores, so a 10-second run has 160 core-seconds to spend, and Hydra used only about 27 of them. On average fewer than 3 cores were busy. The run spends most of its time waiting on something else, not computing, so making the computing faster barely moves the clock. The 0.1 to 0.8 s differences between these pairs are within run-to-run noise.

The third row tries mimalloc's main memory setting. `MIMALLOC_PURGE_DELAY=0` makes it hand freed memory back to Windows at once instead of keeping it for reuse. That brought the peak below the segment heap's (333 MB), but the run got slower (13.1 s), because handing memory back and asking for it again is kernel work. So mimalloc's extra memory and its small CPU gain come together. No setting gives both.

The trial build had mimalloc's statistics option on (its release default). Turning it off would trim a little more CPU work. It can't change the wall time for the same reason, and trying it would need the source downloaded again.

What the batch waits on is the real lever, and it was not measured here. The kernel time, about 9 s in every run, suggests file reads and database writes rather than computing, but that is a guess until someone times the stages.

## It changes nothing stored

A fresh-database `hydra_batch` run with mimalloc, compared table by table with `tools/compare_db.py` against the baseline's `fresh.db`, matched in every table: results 18,811 rows, paths 90,674, path_refs 90,674, songmeta 18,811, dynamics 18,811, charts 19,436, meta 3. 0 rows differ. That run took 11.3 s with a peak of 811 MB.

## How it was built

You approved one download in chat: `mimalloc-v3.5.3-source.tar.gz`, 1,458,292 bytes, from the v3.5.3 release at github.com/microsoft/mimalloc. The file matched that size. mimalloc's README marks v3.5.3 as the recommended release.

It was built with mimalloc's own CMake, Release, with `-DMI_BUILD_SHARED=ON -DMI_OVERRIDE=ON -DMI_WIN_REDIRECT=ON -DMI_BUILD_STATIC=OFF -DMI_BUILD_OBJECT=OFF -DMI_BUILD_TESTS=OFF` and upstream defaults otherwise. Hydra uses the dynamic C runtime, so this is the override-DLL route. In v3 the override DLL is called `mimalloc.dll` (184,320 bytes), not `mimalloc-override.dll` as the handoff expected. Its partner is `mimalloc-redirect.dll` (60,416 bytes), which comes prebuilt in the source tarball.

Only `hydra_batch` was linked against it, in a throwaway worktree. The change linked `mimalloc.dll.lib` and added the linker switch `/include:mi_version`, which mimalloc's README gives to make sure the DLL loads. `dumpbin /dependents` showed `mimalloc.dll` as the exe's first import. With `MIMALLOC_VERBOSE=1`, a one-chart run printed:

```
mimalloc: process init: 0xE8BB155000
mimalloc: reserved 1048576 KiB memory
v3.5.3, release
mimalloc: malloc is redirected.
mimalloc: process done 128
```

The timed runs did not set `MIMALLOC_VERBOSE`. The staged build is in `~\.claude\hooks\state\bench\mimalloc-v3.5.3\`, with a README giving the exact setup.

## What shipping would take

If you wanted it anyway, this is the work. mimalloc's source would go in `third_party/mimalloc`, added with `add_subdirectory(... EXCLUDE_FROM_ALL)` the way ogg and opus are (`CMakeLists.txt` around line 163). Every Hydra exe would link the import library and the `/include:mi_version` switch. The two DLLs need a post-build copy into `build-cpp\Release`, because `tests/test_cli.cpp` starts `hydra_batch.exe` from there and the exe won't start without `mimalloc.dll` beside it. An `install(FILES ...)` line next to the ship list (`CMakeLists.txt` lines 534-537) would stage both DLLs beside the exes. The installer then picks them up, because `installer/hydra.iss` takes everything in the stage.

The repo has no third-party licence list today. Vendored libraries are named in a comment at the top of `CMakeLists.txt` and in `docs/development.md`, and the installer shows only Hydra's own GPL. mimalloc is MIT-licensed, so shipping it means either starting a real third-party notice or at least adding it to those lists.

The installer's guards would still pass. "No user data leaked" looks for databases and settings files, and the DLLs are neither. "No symbol files" finds no `.pdb`, because mimalloc's own install of its `.pdb` is switched off upstream. "No repo path" only checks `Hydra.exe`, but the built `mimalloc.dll` was checked by hand and holds no repo path either.

## Shipping is the user's call

The numbers support leaving mimalloc out. It added two DLLs and 323 MB of peak memory for no measurable speed.
