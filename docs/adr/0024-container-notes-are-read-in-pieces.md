# Container notes are read in pieces

The note loader used to read a whole `.sng` or `.srb` into memory to get its
notes file. A container is mostly audio. The library's three "Endless
Setlist" `.sng` files are about 1 GB each, and their notes are a few hundred
KB. Archives are found last, so a whole-library `hydra_batch` read all three
at once at the end. On 2026-10-04 that made the process peak at about 3.6 GB
of committed memory, almost all of it these three reads.

So the loader now reads only what the notes need.

A `.sng`: the header, metadata and file table, then just the notes entry's
bytes.

A `.srb`: the 16-byte header, then the metadata stream and the notes stream.
A raw deflate stream doesn't say how long it is, so it is inflated from reads
that grow until the stream ends.

How big the reads are: the first read is 64 KB. Each read after that asks for
twice as much as the last. A `.sng` header that doesn't fit is read again from
the start, at twice the size or at what the file table is known to need,
whichever is more. 64 KB holds any real `.sng` header, any `.srb` metadata
block and most notes streams in one read. Doubling keeps a big stream to a
handful of reads and at most about twice its size off disk. The user chose
these sizes on 2026-10-04. They change only how many bytes are read, never
what a load returns. `kFirstPieceRead` and `next_piece_read` in
`src/core/winstr.h` hold them; a scan row in `tests/test_single_owner.cpp`
keeps the first-read size in that one place.

One path for every source. A file and a buffer already in memory are both a
`ByteSource`: a size and a way to read any range. `load_songpath_reading` is
the one place a path's extension picks its loader, so the Preview (which
reads a container once and shares the bytes with its audio extractor) picks
the notes exactly as analysis does. `read_file_bytes` reads through the same
file source.

What stays the same: every result. A whole-library batch on 2026-10-04 stored
the same results and paths and failed the same 541 charts with the same
messages, and peaked at about 0.8 GB instead of 3.6 GB.
