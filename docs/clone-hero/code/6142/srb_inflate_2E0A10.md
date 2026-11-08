# srb_inflate_2E0A10 (build 6142, RVA 0x2E0A10)
A static helper that takes a byte array and returns it inflated. Called twice by `srb_reader_2B84A0` (lines 171 and 400).
The calls show its shape: two MemoryStream constructors (one over the input), a System.IO.Compression.DeflateStream constructor over the input stream, and Stream.CopyTo into the output stream. DeflateStream reads raw DEFLATE, with no zlib or gzip header.
