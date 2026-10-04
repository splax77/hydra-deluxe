#include "parse/srb.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>

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
        throw std::runtime_error("SRB inflate init failed.");

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
        throw std::runtime_error("SRB stream starts past end of file.");
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
            throw std::runtime_error("SRB stream is corrupt.");
        }
        out.insert(out.end(), chunk, chunk + (sizeof(chunk) - s.avail_out));
        if (out.size() > max_out) {
            mz_inflateEnd(&s);
            throw std::runtime_error("SRB stream exceeds size limit.");
        }
        // All input consumed without reaching the stream's end marker, and
        // the file holds no more.
        if (status == MZ_OK && s.avail_in == 0 && s.avail_out != 0) {
            refill();
            if (s.avail_in == 0) {
                mz_inflateEnd(&s);
                throw std::runtime_error("SRB stream is truncated.");
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
        if (pos >= size) return {nullptr, 0};
        const size_t n = std::min(size - pos, kMaxPiece);
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
    const NextInput next = [&]() -> std::pair<const uint8_t*, size_t> {
        piece = src.read(pos, std::min(ask, kMaxPiece));
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
    for (std::string* field : fields) {
        if (pos + 4 > meta.size()) break;
        uint32_t len = 0;
        for (int i = 0; i < 4; ++i)
            len |= static_cast<uint32_t>(meta[pos + i]) << (8 * i);
        pos += 4;
        if (len > meta.size() - pos) break;
        field->assign(reinterpret_cast<const char*>(meta.data() + pos), len);
        pos += len;
    }
    return true;
}

}  // namespace hydra
