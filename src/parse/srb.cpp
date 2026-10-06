#include "parse/srb.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>

#include "core/error_kind.h"
#include "core/little_endian.h"
#include "miniz.h"

namespace hydra {

namespace {

// The compressed input, a piece at a time: the next piece's start and size,
// or a size of 0 once the file has ended.
using NextInput = std::function<std::pair<const uint8_t*, size_t>()>;

// The one inflate loop: both entry points feed it, the whole buffer at once
// or the file in growing reads. Returns the decompressed bytes and sets
// `consumed` to the stream's compressed length.
std::vector<uint8_t> inflate_raw(const NextInput& next_input, size_t max_out,
                                 uint64_t* consumed) {
    mz_stream s{};
    // Negative window bits selects a raw deflate stream, zlib-style.
    if (mz_inflateInit2(&s, -MZ_DEFAULT_WINDOW_BITS) != MZ_OK)
        throw KindedError(ErrorKind::ChartUnreadable, "SRB inflate init failed.");

    // Hands the inflater the next piece once it has used up the last one.
    bool file_ended = false;
    auto refill = [&] {
        if (s.avail_in != 0 || file_ended) return;
        const auto [data, size] = next_input();
        if (size == 0) {
            file_ended = true;
            return;
        }
        s.next_in = data;
        s.avail_in = static_cast<unsigned int>(size);
    };
    refill();
    if (file_ended) {
        mz_inflateEnd(&s);
        throw KindedError(ErrorKind::ChartUnreadable, "SRB stream starts past end of file.");
    }

    std::vector<uint8_t> out;
    uint8_t chunk[64 * 1024];
    int status = MZ_OK;
    while (status != MZ_STREAM_END) {
        refill();
        s.next_out = chunk;
        s.avail_out = sizeof(chunk);
        status = mz_inflate(&s, MZ_NO_FLUSH);
        if (status != MZ_OK && status != MZ_STREAM_END) {
            mz_inflateEnd(&s);
            throw KindedError(ErrorKind::ChartUnreadable, "SRB stream is corrupt.");
        }
        out.insert(out.end(), chunk, chunk + (sizeof(chunk) - s.avail_out));
        if (out.size() > max_out) {
            mz_inflateEnd(&s);
            throw KindedError(ErrorKind::ChartUnreadable, "SRB stream exceeds size limit.");
        }
        // All input consumed without reaching the stream's end marker, and
        // the file holds no more.
        if (status == MZ_OK && s.avail_in == 0 && s.avail_out != 0) {
            refill();
            if (s.avail_in == 0) {
                mz_inflateEnd(&s);
                throw KindedError(ErrorKind::ChartUnreadable, "SRB stream is truncated.");
            }
        }
    }

    *consumed = s.total_in;
    mz_inflateEnd(&s);
    return out;
}

// The most the inflater takes in one piece: all its input counter holds.
constexpr size_t kMaxPiece = std::numeric_limits<decltype(mz_stream::avail_in)>::max();

}  // namespace

std::vector<uint8_t> srb_inflate_stream(const uint8_t* data, size_t size,
                                        size_t offset, size_t max_out,
                                        size_t* end_offset) {
    size_t pos = offset;
    const NextInput next = [&]() -> std::pair<const uint8_t*, size_t> {
        const size_t n = range_length(size, pos, kMaxPiece);
        pos += n;
        return {data + pos - n, n};
    };
    uint64_t consumed = 0;
    std::vector<uint8_t> out = inflate_raw(next, max_out, &consumed);
    if (end_offset) *end_offset = offset + static_cast<size_t>(consumed);
    return out;
}

std::vector<uint8_t> srb_inflate_stream_reading(const ByteSource& src, uint64_t offset,
                                                size_t max_out, uint64_t* end_offset) {
    std::vector<uint8_t> piece;
    uint64_t pos = offset;
    size_t ask = kFirstPieceRead;
    bool ended = false;  // a read came back short: the source ends there
    const NextInput next = [&]() -> std::pair<const uint8_t*, size_t> {
        if (ended) return {nullptr, 0};
        const size_t asked = std::min(ask, kMaxPiece);
        piece = src.read(pos, asked);
        ended = piece.size() < asked;
        pos += piece.size();
        ask = next_piece_read(ask);
        return {piece.data(), piece.size()};
    };
    uint64_t consumed = 0;
    std::vector<uint8_t> out = inflate_raw(next, max_out, &consumed);
    if (end_offset) *end_offset = offset + consumed;
    return out;
}

bool srb_parse_metadata(const std::vector<uint8_t>& meta, SrbMetadata& out) {
    const size_t kPrefixSize = 4;  // "4b4\x01" in every known file; not validated.
    if (meta.size() < kPrefixSize) return false;

    size_t pos = kPrefixSize;
    std::string* fields[] = {&out.notes_filename, &out.name,    &out.artist,
                             &out.album,          &out.genre,   &out.charter,
                             &out.year,           &out.description};
    // A little-endian u32 at `pos`, moving past it; false when it does not fit.
    const auto read_u32 = [&](uint32_t* into) {
        if (sizeof(uint32_t) > meta.size() - pos) return false;
        *into = core::read_le_u32(meta.data() + pos);
        pos += sizeof(uint32_t);
        return true;
    };
    // A length-prefixed string at `pos`, moving past it; false when it does
    // not fit.
    const auto read_string = [&](std::string* into) {
        uint32_t len = 0;
        if (!read_u32(&len) || len > meta.size() - pos) return false;
        if (into) into->assign(reinterpret_cast<const char*>(meta.data() + pos), len);
        pos += len;
        return true;
    };
    // `n` bytes skipped at `pos`; false when they do not fit.
    const auto skip = [&](size_t n) {
        if (n > meta.size() - pos) return false;
        pos += n;
        return true;
    };
    for (std::string* field : fields)
        if (!read_string(field)) return true;

    // The binary fields after the strings, in order (the user's .srb format
    // reference): twelve difficulty bytes, the preview start (i32), the icon
    // name (a string), the playlist and album track numbers (i32 each), then
    // song_length_ms (i32). The checksum and table of contents that follow are
    // not read.
    uint32_t scratch = 0;
    if (!skip(12) || !read_u32(&scratch) || !read_string(nullptr) || !read_u32(&scratch) ||
        !read_u32(&scratch))
        return true;
    uint32_t song_length = 0;
    if (read_u32(&song_length)) out.song_length_ms = static_cast<int32_t>(song_length);
    return true;
}

SrbMetadataRead srb_read_metadata(const ByteSource& src) {
    if (src.size <= kSrbHeaderSize) throw KindedError(ErrorKind::ChartUnreadable, "Truncated SRB file.");
    SrbMetadataRead out;
    const std::vector<uint8_t> meta =
        srb_inflate_stream_reading(src, kSrbHeaderSize, kSrbMaxMetadata, &out.notes_offset);
    out.parsed = srb_parse_metadata(meta, out.fields);
    return out;
}

}  // namespace hydra
