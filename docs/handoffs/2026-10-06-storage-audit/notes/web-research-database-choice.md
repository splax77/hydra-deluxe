# Hydra storage engine research: is switching from SQLite worth it?

Short answer: no. Compress the blobs and fix the writer path, and keep SQLite. Everything below is cited. Items I could not verify are marked "unverified".

Two fetches failed with SSL errors: the LMDB microbenchmark page and the lmdb.tech docs. So I have no first-party LMDB numbers. Several search-summary sources were weak and I did not use them. The weak ones were getgalaxy, hackernoon, and oneuptime. I did not use their numbers.

## 1. SQLite for this workload

**Blob size sweet spot.** Hydra holds about 280 MB over about 19,000 charts. That is about 15 KB per chart, which I computed myself. SQLite's own measurements say blobs around 10 KB are faster inside the database than as separate files.
- SQLite's kvtest used 100,000 blobs of 8-12 KB. It found about 35% faster than the filesystem, and about 20% less disk space (1.02 GB vs 1.23 GB). The test ran in June 2017 on SQLite 3.19/3.20. SQLite ran it, so treat it as the vendor's own number. The page itself warns the 35% varies by hardware and filesystem cache state. [fasterthanfs](https://www.sqlite.org/fasterthanfs.html)
- A 2011 test (SQLite 3.7.8, ext4) found the break-even near 100 KB. About 10 KB was 1.5-2.4x faster in the database. [intern-v-extern-blob](https://www.sqlite.org/intern-v-extern-blob.html)
- The same page recommends 8 KB or 16 KB pages for large-blob I/O. [intern-v-extern-blob](https://www.sqlite.org/intern-v-extern-blob.html)

**Storage overhead is small.** [fileformat2](https://www.sqlite.org/fileformat2.html)
- Each page has an 8-byte header (leaf).
- Each row costs 2 bytes in the pointer array plus 1-9 byte varints.
- Overflow pages carry a 4-byte header.
- A blob's last page is partly empty, so the average waste per blob is about half a page.
- With about 15 KB records and 4 KB pages, that waste is a few percent, not a big saving.

**The single writer is a hard rule.** There is one writer at a time per database. Write transactions usually finish in milliseconds and writers queue. [whentouse](https://www.sqlite.org/whentouse.html) Funnelling worker output through one writer thread therefore matches how SQLite works. The bottleneck is how that thread is used, not the engine.

**WAL behaviour.** [wal](https://www.sqlite.org/wal.html), [pragma](https://www.sqlite.org/pragma.html)
- Auto-checkpoint runs at about 1000 pages (about 4 MB). It is a PASSIVE checkpoint, so it cannot reset the WAL while a reader holds it open.
- The WAL grows when checkpoints are starved by overlapping readers, or when one transaction is very large.
- SQLite advises against transactions over about 100 MB in WAL mode.
- Your 45 MB WAL suggests one of those causes. It is not necessarily a problem, but `PRAGMA journal_size_limit` and a `wal_checkpoint(TRUNCATE)` after a batch are the standard fixes.
- With `synchronous=NORMAL` in WAL mode, commits do not sync. You lose durability on power loss but keep atomicity, consistency and isolation. SQLite calls this the recommended trade-off.

**Write throughput tips.**
- One big transaction plus prepared statements is the main lever. A 2021 Rust test reached about 1.67M rows/s with prepared statements, and about 3.1M rows/s with batching of 50 plus threads. Those rows are tiny, and the test used `journal_mode=OFF` and `synchronous=0`. It ran on a 2019 MacBook Pro i5. [avi.im, July 2021](https://avi.im/blag/2021/fast-sqlite-inserts/)
- That blog also found about 99% of the time went to generating rows, not to the database.
- phiresky's tuning post recommends WAL, `synchronous=normal`, `temp_store=memory`, a large `mmap_size`, and a 32 KB page size for large blobs. [phiresky](https://phiresky.github.io/blog/2020/sqlite-performance-tuning/)
- `cache_size` defaults to about 2 MB. [pragma](https://www.sqlite.org/pragma.html)
- Do compression and hashing in the worker threads, so the writer only runs `sqlite3_step` inside a large transaction. That is inference from the numbers above, not a cited claim.

**Limits are irrelevant at this size.** A blob can be up to 1 GB by default. The database can reach about 281 TB. [limits](https://www.sqlite.org/limits.html)

**SQLite's own positioning.** It lists desktop application file formats as an appropriate use. It lists many concurrent writers and terabyte-scale data as reasons to look elsewhere. [whentouse](https://www.sqlite.org/whentouse.html) Its application-file-format page claims atomic transactions, incremental updates, and one file across platforms. [appfileformat](https://www.sqlite.org/appfileformat.html)

## 2. Alternatives and comparison table

**The only same-dataset size and speed table I found** is from redb's README, so it is the redb author's own benchmark. It ran on a Ryzen 9950X3D with a Samsung 9100 PRO NVMe. The dataset description (key and value sizes) was not in the page I fetched, so the sizes depend on that unknown data. [redb README](https://raw.githubusercontent.com/cberner/redb/master/README.md)

| Metric | redb | LMDB | RocksDB | fjall | SQLite |
|---|---|---|---|---|---|
| Bulk load | 17.1 s | 9.2 s | 14.0 s | 18.6 s | 15.3 s |
| Individual writes | 0.92 s | 1.60 s | 2.43 s | 3.49 s | 7.04 s |
| Batch writes | 1.60 s | 0.94 s | 0.45 s | 0.35 s | 2.63 s |
| Random reads | 1.14 s | 0.64 s | 2.91 s | 2.18 s | 4.28 s |
| Compacted size | 1.69 GiB | 1.26 GiB | 455 MiB | 1001 MiB | 557 MiB |

How to read it:
- SQLite was slowest on reads and batch writes, and its settings are unknown. It was still smaller than LMDB and redb.
- Only RocksDB beat SQLite on size, by about 18%. Its compression is on by default, per [RocksDB docs](https://github.com/facebook/rocksdb/wiki/Compression).

**Per-engine notes.**
- **LMDB.**
  - Memory-mapped B+tree. Reads are very fast. The file never shrinks.
  - The map size is also the maximum database size. Long-lived read transactions make the file grow. It has no compression. Keys are limited to 511 bytes. [lmdb.h](https://raw.githubusercontent.com/LMDB/lmdb/mdb.master/libraries/liblmdb/lmdb.h)
  - On Windows, stale writer locks clear automatically. [lmdb.h](https://raw.githubusercontent.com/LMDB/lmdb/mdb.master/libraries/liblmdb/lmdb.h)
  - License: I believe it is the OpenLDAP Public License, which is permissive. Unverified; the GitHub page did not state it. [mirror](https://github.com/LMDB/lmdb)
- **libmdbx.**
  - LMDB fork. The author claims 10-30% faster than LMDB, tested on tmpfs, with no independent check. [libmdbx](https://github.com/erthink/libmdbx)
  - Apache 2.0 as of May 2024. Windows is supported but uses `LockFileEx`, which the author says affects speed on Windows. It has no compression.
  - Used in Ethereum clients. Key limit is about 2 KB. [libmdbx](https://github.com/erthink/libmdbx)
- **RocksDB.**
  - LSM tree (log-structured merge). Dual-licensed GPLv2 and Apache 2.0. [rocksdb](https://github.com/facebook/rocksdb)
  - Compression: LZ4 or Snappy on upper levels, ZSTD on the bottom level. Optional dictionary compression. [wiki](https://github.com/facebook/rocksdb/wiki/Compression)
  - Facebook's benchmark (May 2022, v7.2.2, AWS m5d.2xlarge NVMe, Linux) shows fillseq at 1.0M ops/s and overwrite at 86.6K ops/s with 9.5x write amplification. [benchmarks](https://github.com/facebook/rocksdb/wiki/Performance-Benchmarks)
  - You would have to build and ship a large C++ library. It has no summary-table sorting; you would have to maintain your own secondary index.
- **LevelDB.** Snappy compression, BSD-3, builds on Windows with CMake. The repo says it gets "very limited maintenance". Single process only. No indexes. [leveldb](https://github.com/google/leveldb)
- **DuckDB.**
  - Columnar, single file. Compression includes RLE, bit-packing, dictionary, FSST and Zstd. [storage](https://duckdb.org/docs/current/internals/storage.html)
  - Backward-compatible storage since 0.10. [storage](https://duckdb.org/docs/current/internals/storage.html)
  - Built for analytics. Appends from several threads do not conflict, but row-by-row inserts are discouraged. [concurrency](https://duckdb.org/docs/current/connect/concurrency.html), [insert](https://duckdb.org/docs/current/data/insert.html)
  - One published size comparison exists, on 100M NYC taxi rows: 28 GB for DuckDB vs 92 GB for SQLite. It comes from a vendor-style blog, on an M2 Pro, with DuckDB 0.10.2 and SQLite 3.45.2. [getgalaxy](https://www.getgalaxy.io/learn/glossary/duckdb-vs-sqlite-benchmarks) That is numeric table data. Opaque binary blobs would not get that benefit.
- **Berkeley DB.** AGPLv3 from 2013, with a paid licence for proprietary redistribution. Debian moved to LMDB because of this. Disqualified. [LWN / search](https://lwn.net/Articles/557820)
- **UnQLite.** BSD, but the maintainer says it is no longer actively developed. [unqlite README](https://github.com/symisc/unqlite/blob/master/README.md)
- **ObjectBox.** Supports C++ and Windows. The core is proprietary, under an "ObjectBox Binary License". Only the bindings are Apache 2.0. [FAQ](https://objectbox.io/faq/) Fails your permissive-licence requirement.
- **Realm.** MongoDB deprecated the Atlas Device SDKs in September 2024, with end-of-life on 30 September 2025. The on-device database continues as open source. [deprecation](https://mongodb.com/docs/atlas/device-sdks/deprecation) Not worth adopting.
- **redb (Rust).** Copy-on-write B+tree, crash-safe, stable file format, Apache/MIT. No compression or C API mentioned. [redb](https://github.com/cberner/redb) You would pay the FFI (foreign function interface) cost and gain nothing on size.
- **sled.** Beta. Its README says "If reliability is your primary constraint, use SQLite". The on-disk format will change, and it uses more disk than RocksDB. [sled](https://github.com/spacejam/sled)
- **Turso/Limbo.** MIT, SQLite-compatible file format and C API. Not yet 1.0, and the team says to "keep independent backups". Its headline feature is concurrent writers (BEGIN CONCURRENT). [turso](https://github.com/tursodatabase/turso) Your bottleneck is one funnelled writer, not writer concurrency. Too young.
- **Flat files or a custom pack file.** SQLite's page says a pile of files is not a single file and has no incremental update. Custom formats mean writing your own crash recovery. [appfileformat](https://www.sqlite.org/appfileformat.html) You would be rebuilding what SQLite already gives you.

| Engine | Data model | Compression | Write speed | Read speed | Crash safety | Windows / C++ fit | License | Fit for Hydra |
|---|---|---|---|---|---|---|---|---|
| SQLite (now) | Tables plus blobs, SQL | None built in | Good when batched | Good | Strong (WAL) | Excellent, C API | Public domain | Best |
| LMDB | Ordered key-value | None | Fast bulk | Best | Strong (copy-on-write) | Good, C | OpenLDAP (unverified) | No sort or filter for the summary table; file never shrinks |
| libmdbx | Ordered key-value | None | Faster than LMDB (author claim) | Fast | Strong | Fair (slower on Windows) | Apache 2.0 | Same gaps as LMDB |
| RocksDB | LSM key-value | LZ4/Zstd | Best batch writes | Slower | Strong | Heavy build | GPLv2 / Apache 2.0 | Smaller on disk, but big dependency and no SQL |
| LevelDB | LSM key-value | Snappy | Good | Fair | Good | Builds on Windows | BSD-3 | Barely maintained |
| DuckDB | Columnar SQL | Strong (columns) | Weak per row | Good for scans | Good | OK, C++ API | MIT (unverified) | Wrong shape for blobs |
| Berkeley DB | Key-value | None | n/a | n/a | n/a | n/a | AGPLv3 | Excluded |
| ObjectBox | Object store | None cited | Claimed fast | Claimed fast | n/a | Windows, C++ | Proprietary core | Excluded |
| redb / sled | Rust key-value | None / optional zstd | OK | OK | redb strong; sled beta | FFI needed | Apache/MIT | Excluded |
| Turso | SQLite-compatible | None | Concurrent writers | n/a | Pre-1.0 | Windows, C API | MIT | Too young |

## 3. Hybrid options

- **App-level zstd per blob.** Compress in the worker threads before the writer sees the data. The writer stays dumb and fast. I found no citable benchmark for your data; you must measure your blobs. A hint from phiresky's tests: on one dataset, app-level compression got only 23% off while dictionary-based row compression got 75%. Small, similar records benefit from a shared dictionary. [phiresky 2022](https://phiresky.github.io/blog/2022/sqlite-zstd/)
- **sqlite-zstd (phiresky extension).** Transparent row compression with trained dictionaries. His tests: IMDB data 2.0 GB to 528 MB, and a 7.6 GB database cut 75%. Updates to random rows slow down significantly. [phiresky 2022](https://phiresky.github.io/blog/2022/sqlite-zstd/) The README says "I wouldn't trust it with my data (yet)". It is Rust and LGPL-3.0, so you would ship an extension DLL. [sqlite-zstd](https://github.com/phiresky/sqlite-zstd)
- **sqlite_zstd_vfs (mlin).** Page-level compression. It cut a TPC-H database from 1182 MiB to 433 MiB, but bulk load took 33.7 s vs 2.4 s (6.9 s with 8 threads). It does not support WAL ("do not touch"), forces EXCLUSIVE locking, and is Unix x86-64 oriented. [README](https://raw.githubusercontent.com/mlin/sqlite_zstd_vfs/main/README.md) Disqualified: Windows and WAL.
- **ZIPVFS.** Official SQLite compression add-on, paid: $4,000 perpetual source licence. [zipvfs](https://sqlite.org/com/zipvfs.html) It is proprietary, so it does not meet your requirement for a permissive licence.
- **sqlite3mc.** It is an encryption VFS (virtual file system), not compression. [repo](https://github.com/utelle/sqlite3multipleciphers) I found nothing on CEVFS.
- **DuckDB attached to SQLite.** Not researched in depth. Nothing I found suggests a benefit for blob lookups, and it adds a second engine.

## 4. Verdict

Switching engines would not buy meaningful space or time. Reasons:
1. **Only compression moves the size, not the engine.** In the one same-dataset table, only RocksDB beat SQLite on size, and that is down to compression. SQLite beat LMDB and redb. SQLite plus zstd on the blobs gets you the same benefit.
2. **Your workload fits SQLite's strengths.** It has about 15 KB blobs, a sortable summary table, and one GUI reader. LMDB, RocksDB and LevelDB are key-value stores. You would hand-build sorting and filtering for the 20k-row summary and give up SQL joins on path nodes.
3. **The writer bottleneck is not SQLite's single-writer rule.** SQLite documents commits finishing in milliseconds. If the writer thread is slow, the likely causes are small transactions, per-row statement preparation, compression or hashing done on the writer thread, or WAL checkpoint behaviour. All are fixable inside SQLite. I did not profile your code, so this is a hypothesis.
4. **The community consensus matches.** SQLite lists desktop application files as an appropriate use. [whentouse](https://www.sqlite.org/whentouse.html) sled's own README tells reliability-first users to use SQLite. [sled](https://github.com/spacejam/sled) I found no reputable post recommending a switch for this scale. I found little first-hand experience writing beyond these.

**Ranked recommendation.**
1. **Stay on SQLite and tune the writer.**
   - One large transaction per batch, one prepared statement per statement kind, and `journal_size_limit` plus a `wal_checkpoint(TRUNCATE)` after batches.
   - Do hashing and compression in the workers.
   - Consider 8-16 KB pages and a larger `cache_size`. Treat `mmap_size` as an experiment.
2. **Add zstd on the structure and path-node blobs, measured on a real copy of the database.** Try plain per-blob zstd first. Try a trained dictionary only if the gain justifies it. The dictionary approach is the one with published wins.
3. **Run `VACUUM` once after a large delete or garbage-collection pass.** Otherwise freed pages stay in the file. [pragma](https://www.sqlite.org/pragma.html)
4. **Only if size still matters after step 2, prototype RocksDB with ZSTD.** It is the one engine with a published size win. Expect a big dependency and a hand-built summary index.

Not recommended: LMDB or libmdbx (no compression, no sorting), DuckDB (wrong shape), Berkeley DB, ObjectBox, Realm, sled, Turso (licence or maturity), sqlite_zstd_vfs (no Windows, no WAL) and ZIPVFS (proprietary).

Status lines were appended to C:\Users\Patrick\.claude\hooks\state\status\a6b2e3c3568a00024.md and storage-web-research.md. No repo files were changed.
