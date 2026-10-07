REPORT: compact storage for Hydra's SQLite data (read-only web research, no repo files touched)

Source note: the page fetches were summarised by a small model. The numbers below are as returned. Where a source gave no number, I say so rather than guess.

## 1. SQLite storage efficiency

**Hash as 16-byte BLOB instead of 32-char hex TEXT.**
- Plain version: store the 16 raw bytes, not the hex string.
- Saving: exactly half of each hash column. A forum thread agrees that BLOB is 2x smaller, e.g. 32 vs 64 bytes for SHA-256. https://www.sqlite.org/forum/forumpost/c7771e9442?t=c
- That thread gives no measured numbers. Richard Hipp says both work fine. Fossil keeps hashes as text for easier debugging.
- One search snippet claims a user saw no file-size difference on 2.23M rows. I could not verify that.
- Cost: hashes are no longer readable in the sqlite shell. You need `hex()` or a helper.
- My arithmetic for Hydra: the `paths` key is 32 + 32 + 16 hex chars, and the same key is repeated in `path_refs`, in its index and in the `paths` autoindex. Shrinking those is likely the biggest win from this technique.

**WITHOUT ROWID.**
- Plain version: the table is stored as a single B-tree keyed by the primary key. An ordinary table with a non-integer primary key stores that key twice, once in the table and once in an automatic index.
- Saving: "about half the amount of disk space and... nearly twice as fast" in the best case. https://www.sqlite.org/withoutrowid.html
- Applies when keys are composite or non-integer and rows are small. The rule of thumb is rows under about 1/20 of a page, so under about 200 bytes at 4 KiB pages.
- Not for large blobs. Single INTEGER primary keys are faster as ordinary rowid tables.
- Fit for Hydra:
  - `path_refs` is nearly all key columns, so it is a good fit. Its separate secondary index could also be dropped if the primary key order serves the main lookups.
  - `paths` has big payload blobs, so WITHOUT ROWID is probably wrong there. An alternative is a small integer surrogate id for (hyhash, chartmode), with the 16-byte phash as a BLOB key.

**Redundant indexes.**
- Every index copies its key columns. The sqlite3_analyzer report shows each index's bytes and share of the file, so you can find indexes that cost more than they save. https://www.sqlite.org/sqlanalyze.html
- `PRAGMA optimize` after schema changes is recommended. https://www.sqlite.org/pragma.html

**Record format and varints.**
- Integers are stored in 0, 1, 2, 3, 4, 6 or 8 bytes depending on value. The constants 0 and 1 take zero bytes in the body. REAL is always 8 bytes. TEXT and BLOB take their length plus one header varint. https://www.sqlite.org/fileformat2.html
- So integer columns are already compact, and 0/1 flags are free.
- Source of waste: columns that hold hex text, REALs that are always whole numbers (store as INTEGER), and blobs of fixed-width integers.

**Overflow pages.**
- A table-leaf cell keeps up to U-35 bytes on the page (about 4061 at 4 KiB). Anything larger spills into a linked chain of overflow pages. https://www.sqlite.org/fileformat2.html
- A blob just over the threshold wastes the tail of its last overflow page. Page size changes where this bites.
- SQLite's own test: blobs under about 100 KB read faster inside the database than as files, and 8 KiB or 16 KiB pages did best for large blobs. https://www.sqlite.org/intern-v-extern-blob.html
- That test was for large blobs on ext4, so check it on Hydra's own data.

**page_size.**
- It can be any power of two from 512 to 65536. The default has been 4096 since 3.12.0. https://www.sqlite.org/pragma.html
- Changing it needs a VACUUM, and in WAL mode VACUUM cannot change it. You must switch out of WAL, set the size, VACUUM, then switch back.
- Larger pages mean fewer overflow chains for multi-KB blobs. Smaller pages waste less on tiny rows. Measure both with dbstat.

**VACUUM and freelist.**
- VACUUM repacks the file. It needs up to twice the file size in free disk. https://www.sqlite.org/lang_vacuum.html
- `VACUUM INTO newfile` writes a minimal copy and leaves the original alone. Same page.
- `auto_vacuum=INCREMENTAL` lets `incremental_vacuum(N)` return free pages gradually. It must be set before the first table exists, or applied with a VACUUM. https://www.sqlite.org/pragma.html
- `PRAGMA freelist_count` tells you how much space is reusable. With `--redo` style re-analysis you delete and rewrite a lot, so this matters.

**WAL growth.**
- The WAL normally checkpoints near 1000 pages (about 4 MB). It can grow without bound if readers always overlap. https://www.sqlite.org/wal.html
- `journal_size_limit` truncates the WAL back after a checkpoint. `wal_autocheckpoint` sets the trigger size. https://www.sqlite.org/pragma.html
- For Hydra: run `PRAGMA wal_checkpoint(TRUNCATE)` when a batch ends. This affects the extra .wal file, not the main db size.

**Measuring.**
- The `dbstat` virtual table gives per-table pages, payload and unused bytes. https://www.sqlite.org/dbstat.html
- `sqlite3_analyzer` is the same data as a report, with overflow and fan-out stats. https://www.sqlite.org/sqlanalyze.html
- Do this first. It tells you whether the 280 MB is `paths` payloads, `structure` blobs, or keys and indexes.

## 2. Compressing blobs

**Per-row zlib/zstd/lz4 in the app.**
- Plain version: compress each blob before insert, decompress on read.
- Ratio data (Silesia corpus, one CPU): zstd -1 gives 2.90x at 510 MB/s compress and 1550 MB/s decompress. zlib -1 gives 2.74x at 105 and 390 MB/s. lz4 gives 2.10x at 675 and 3850 MB/s. https://facebook.github.io/zstd/
- Cost: small blobs compress poorly because the compressor has no history to learn from. Your app already links miniz, so zlib is free to try.
- SQLite ships zlib-based compression: the `compress`/`uncompress` SQL functions in https://sqlite.org/src/doc/tip/ext/misc/compress.c and the sqlar archive format https://sqlite.org/sqlar

**Trained zstd dictionaries.**
- Plain version: train a dictionary on sample records so each small record can compress against shared patterns.
- Saving: on 1,000 small JSON records, ratio went from 2.8x to 6.9x. https://engineering.fb.com/2016/08/31/core-infra/smaller-and-faster-data-compression-with-zstandard/
- Zstd's docs say dictionaries help most on small data and also speed up compression and decompression. https://facebook.github.io/zstd/
- Cost: you must store and version the dictionary, and keep it matched to the rows. Retraining means recompressing. Packed binary integers compress less than JSON text, so expect a smaller gain than that example.

**sqlite-zstd (phiresky).**
- Plain version: a SQLite extension that compresses chosen columns transparently, with dictionaries trained per group of rows. https://github.com/phiresky/sqlite-zstd
- Saving: on a 2.0 GB database of 215-byte average JSON rows, it came out at 528 MB, about 74% smaller. One app reported 800 MB to 72 MB. https://phiresky.github.io/blog/2022/sqlite-zstd/
- The same post compared its own data: whole-file compression -90% (but not queryable), app-level compression -23%, sqlite-zstd -75%. Random reads were not slower.
- Caveats from the README: it renames the table behind a view, `sqlite3_changes()` returns 0, the streaming blob API stops working, and the file only shrinks after VACUUM.
- Cost for Hydra: this is a loadable C extension and a Rust build. You could instead copy the design in-app: a small `dicts` table, a dict id per row, and `ZSTD_compress_usingDict`.

**ZIPVFS / CEROD.**
- ZIPVFS compresses at the VFS layer: pages are compressed as written, so they become variable-sized. https://sqlite.org/zipvfs/doc/trunk/www/howitworks.wiki
- CEROD is a compressed, optionally encrypted, read-only variant that uses zlib. https://www.sqlite.org/cerod/doc/trunk/www/readme.wiki
- Both are paid SQLite add-ons and neither gave a ratio number. Not practical here, and CEROD is read-only.

**NTFS compression.**
- It is transparent, but compresses in large chunks, so random page reads are costly and database I/O loses asynchrony. That is Microsoft's own guidance for SQL Server. https://learn.microsoft.com/en-us/archive/blogs/sqlserverstorageengine/why-not-use-compressed-disk-files-or-disk-volumes
- Cheap to try for a cold archive, but I would not recommend it for a live WAL database.

## 3. Compact numeric encoding

**Varint / LEB128 and ZigZag.**
- Plain version: small numbers take 1 byte, with 7 payload bits per byte. ZigZag maps negatives to small positives, e.g. 0, -1, 1, -2 become 0, 1, 2, 3. Without it a negative int64 costs 10 bytes. https://protobuf.dev/programming-guides/encoding/
- Saving: ticks as u32 cost 4 bytes each. Counts under 128 cost 1 byte.
- Cost: no random access into the middle of a list.

**Delta encoding of sorted tick lists.**
- Plain version: store the gap to the previous tick instead of the tick itself. Gaps are small, so the varints are 1 to 2 bytes.
- Parquet's DELTA_BINARY_PACKED does this by blocks. It takes the minimum delta per block as a frame of reference, then bit-packs the remainder. https://parquet.apache.org/docs/file-format/data-pages/encodings/
- I found no tidy single "x% saved" figure for it.
- In my judgment this is the best fit for your tick lists, since drum ticks are sorted.

**Delta-of-delta.**
- Plain version: take the difference of the differences. Regular spacing turns into zeros.
- Gorilla's regular timestamps shrink from 64 bits to as little as 1. Overall it averaged 1.37 bytes per point, a 12x cut. Over 50% of doubles compressed to 1 bit. https://www.tigerdata.com/blog/time-series-compression-algorithms-explained
- Applies only if your tick gaps are regular (steady 16th-note grids). Worth a quick histogram first.

**Bit-packing and frame-of-reference.**
- Plain version: store a block of values using only as many bits as the largest needs.
- Lemire's SIMD scheme decodes at as little as 0.7 CPU cycles per 32-bit integer. https://arxiv.org/abs/1401.6399
- Stream VByte decodes over 4 billion delta-coded integers per second on one Haswell core. https://ar5iv.arxiv.org/html/1709.08990
- Decode speed is not your problem, but this shows the speed ceiling is far above what you need.

**Columnar vs row layout.**
- Plain version: store all ticks together, all counts together, so each compresses better.
- Parquet's dictionary plus RLE/bit-packing encodings come from this idea. Same page as above.
- Cost: more code. Pays off mainly when followed by zstd.

**Serialization formats (one C++ benchmark, GCC 11, nested game-style data).** https://github.com/fraillt/cpp_serializers_benchmark

| Library | Size (bytes) | Serialize (ms) | Deserialize (ms) |
|---|---|---|---|
| bitsery | 6,913 | 1,470 | 1,524 |
| zpp_bits | 8,413 | 733 | 693 |
| msgpack | 8,857 | 2,770 | 14,033 |
| protobuf | 10,018 | 19,929 | 20,592 |
| cereal | 10,413 | 10,777 | 9,088 |
| flatbuffers | 14,924 | 8,757 | 3,361 |

- Takeaway: FlatBuffers is the biggest and Protobuf is slow. Bitsery is smallest. Neither beats a custom varint layout by much.
- Cap'n Proto's packing only squeezes zero bytes. It recommends LZ4 or zlib on top. https://capnproto.org/encoding.html
- Your format is already custom and little-endian, so adopting a library likely buys little. Changing the integer encoding inside it is where the saving is.

## 4. Others who store results compactly

- **Lichess (chess):** each move becomes its index in a heuristically ordered legal-move list, then Huffman coded. This gave "275% improved" compression and about 70 GB saved on 680M games. https://lichess.org/blog/Wqa7GiAAAOIpBLoY/developer-blog-i-just-optimized-lichess-database
- A follow-up with arithmetic coding and tuned heuristics reached 3.7 bits per move, versus 9.5 for a hand-rolled start. https://mbuffett.com/posts/compressing-chess-moves-even-further/
- Lesson: predict the value from domain knowledge, then store only the surprise.
- **osu!:** replays are an LZMA stream inside a small binary header, and the beatmap md5 is stored in the header. https://osu.ppy.sh/wiki/en/Client/File_formats/Osr_%28file_format%29. The osu!.db uses ULEB128 lengths and length-prefixed strings. https://github.com/ppy/osu/wiki/Legacy-database-file-structure
- Lesson: this is the same shape as your `structure` blob. It is a binary header with a compressed tail.
- **Syzygy tablebases:** Re-Pair plus Huffman compression, with the generator not storing what can be recomputed at probe time. https://talkchess.com/viewtopic.php?p=753610
- Lesson: don't store anything you can rederive cheaply.

## Ranked shortlist for Hydra's data

1. **Measure first with dbstat / sqlite3_analyzer.** It costs nothing and tells you which of keys, indexes, `paths` payloads or `structure` blobs holds the 280 MB. Every ranking below depends on it.
2. **Per-blob compression, trained-dictionary zstd if you can add it, else zlib via miniz.** Blobs are uncompressed today, and many similar small integer-heavy records are the best case for dictionaries (2.8x to 6.9x on a comparable small-record case). Expect a large, cheap win. Try miniz at level 1 to 6 on a sample first.
3. **Delta + varint + ZigZag inside the blobs, before compression.** Tick lists are sorted and ticks are u32, so 4 bytes drops to roughly 1 to 2. It also makes the zstd or zlib pass more effective. Do it together with item 2 and measure each step.
4. **Raw 16-byte BLOB hashes, and a smaller key for `paths` and `path_refs`.** The 48 hex chars per row are repeated in the table, autoindex and secondary index. This halves them. I rank it below the blob work because it only matters if dbstat shows keys and indexes are a big share.
5. **WITHOUT ROWID for `path_refs` (key-only rows) and drop redundant indexes.** Same idea as item 4: removes the duplicate copy of the key. Not for `paths`, whose rows are large.
6. **Housekeeping:** check `freelist_count`, VACUUM INTO after big redo runs, truncate the WAL at batch end, and test page_size 8192 or 16384. These are cheap and reclaim waste, but are one-time wins.

Lower priority or avoid: ZIPVFS and CEROD (paid, and CEROD is read-only), NTFS compression (poor fit for a live database), swapping in Protobuf or FlatBuffers (bigger or slower than your custom format).
