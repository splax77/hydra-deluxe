#include "parse/sng.h"

#include <algorithm>
#include <cstdint>

namespace hydra {

namespace {

// n bytes starting at pos lie inside something of `size` bytes (no overflow
// for any pos or n). The one bounds rule for the header walk and the entries.
bool fits(uint64_t size, uint64_t pos, uint64_t n) {
    return pos <= size && n <= size - pos;
}
bool fits(const std::vector<uint8_t>& buf, size_t pos, uint64_t n) {
    return fits(buf.size(), pos, n);
}

uint64_t u64_at(const std::vector<uint8_t>& buf, size_t pos) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v |= static_cast<uint64_t>(buf[pos + i]) << (8 * i);
    return v;
}

uint32_t u32_at(const std::vector<uint8_t>& buf, size_t pos) {
    uint32_t v = 0;
    for (int i = 0; i < 4; ++i) v |= static_cast<uint32_t>(buf[pos + i]) << (8 * i);
    return v;
}

std::string string_at(const std::vector<uint8_t>& buf, size_t pos, size_t len) {
    return std::string(reinterpret_cast<const char*>(buf.data() + pos), len);
}

// The bytes a walk needs to read n more bytes at pos (saturating, not wrapping).
uint64_t needs(size_t pos, uint64_t n) {
    return n > UINT64_MAX - pos ? UINT64_MAX : pos + n;
}

// Walks the header through the end of the file table, putting each entry that
// fits into `out` when it is given. Returns how many leading bytes of the file
// the walk needed: where the table ends when buf holds all of it, otherwise
// more than buf.size() (what the first piece that did not fit needs).
uint64_t walk_file_table(const std::vector<uint8_t>& buf, std::vector<SngFileEntry>* out) {
    if (!fits(buf, kSngMetadataLenOffset, 8)) return needs(kSngMetadataLenOffset, 8);
    const uint64_t metadata_len = u64_at(buf, kSngMetadataLenOffset);
    if (!fits(buf, kSngMetadataOffset, metadata_len)) return needs(kSngMetadataOffset, metadata_len);
    size_t pos = kSngMetadataOffset + static_cast<size_t>(metadata_len);
    if (!fits(buf, pos, 16)) return needs(pos, 16);
    pos += 8;  // the file section's length; entries carry absolute offsets
    const uint64_t count = u64_at(buf, pos);
    pos += 8;
    for (uint64_t i = 0; i < count; ++i) {
        if (!fits(buf, pos, 1)) return needs(pos, 1);
        const size_t name_len = buf[pos];
        pos += 1;
        if (!fits(buf, pos, name_len + 16)) return needs(pos, name_len + 16);
        if (out) {
            SngFileEntry e;
            e.name = string_at(buf, pos, name_len);
            e.length = u64_at(buf, pos + name_len);
            e.offset = u64_at(buf, pos + name_len + 8);
            out->push_back(std::move(e));
        }
        pos += name_len + 16;
    }
    return pos;
}

// Byte i of a file is stored XORed with mask[i % 16] ^ (i & 0xff). That key
// repeats every 256 bytes, so it is built once and the bytes are unmasked a
// block at a time. src and dst may be the same buffer.
void unmask(const uint8_t* mask, const uint8_t* src, size_t n, uint8_t* dst) {
    uint8_t key[256];
    for (size_t j = 0; j < 256; ++j)
        key[j] = static_cast<uint8_t>(mask[j % kSngXorMaskSize] ^ j);
    for (size_t base = 0; base < n; base += 256) {
        const size_t len = n - base < 256 ? n - base : 256;
        for (size_t j = 0; j < len; ++j)
            dst[base + j] = static_cast<uint8_t>(src[base + j] ^ key[j]);
    }
}

}  // namespace

std::vector<std::pair<std::string, std::string>> sng_read_metadata(const std::vector<uint8_t>& buf) {
    std::vector<std::pair<std::string, std::string>> out;
    if (!fits(buf, kSngMetadataOffset, 8)) return out;
    size_t pos = kSngMetadataOffset;
    const uint64_t count = u64_at(buf, pos);
    pos += 8;
    for (uint64_t i = 0; i < count; ++i) {
        if (!fits(buf, pos, 4)) break;
        const uint32_t key_len = u32_at(buf, pos);
        pos += 4;
        if (!fits(buf, pos, key_len)) break;
        std::string key = string_at(buf, pos, key_len);
        pos += key_len;
        if (!fits(buf, pos, 4)) break;
        const uint32_t value_len = u32_at(buf, pos);
        pos += 4;
        if (!fits(buf, pos, value_len)) break;
        std::string value = string_at(buf, pos, value_len);
        pos += value_len;
        out.emplace_back(std::move(key), std::move(value));
    }
    return out;
}

std::vector<SngFileEntry> sng_read_file_table(const std::vector<uint8_t>& buf) {
    std::vector<SngFileEntry> out;
    walk_file_table(buf, &out);
    return out;
}

bool sng_decode_file_into(const std::vector<uint8_t>& buf, const SngFileEntry& entry,
                          std::vector<uint8_t>& out) {
    if (!fits(buf, kSngXorMaskOffset, kSngXorMaskSize)) return false;
    if (!fits(buf.size(), entry.offset, entry.length)) return false;
    const size_t n = static_cast<size_t>(entry.length);
    out.resize(n);
    unmask(buf.data() + kSngXorMaskOffset, buf.data() + static_cast<size_t>(entry.offset), n,
           out.data());
    return true;
}

std::vector<uint8_t> sng_read_head(const ByteSource& src) {
    size_t asked = kFirstPieceRead;
    std::vector<uint8_t> head = src.read(0, asked);
    for (;;) {
        const uint64_t needed = walk_file_table(head, nullptr);
        // The whole table is in hand, or the read came back short: head is
        // the whole file (so it parses exactly as the file does).
        if (needed <= head.size() || head.size() < asked) return head;
        asked = static_cast<size_t>(std::min<uint64_t>(
            std::max<uint64_t>(needed, next_piece_read(asked)), SIZE_MAX));
        head = src.read(0, asked);
    }
}

std::optional<std::vector<uint8_t>> sng_read_file(const ByteSource& src,
                                                  const std::vector<uint8_t>& head,
                                                  const SngFileEntry& entry) {
    if (!fits(head, kSngXorMaskOffset, kSngXorMaskSize)) return std::nullopt;
    if (!fits(src.size, entry.offset, entry.length)) return std::nullopt;
    const size_t n = static_cast<size_t>(entry.length);
    std::vector<uint8_t> bytes = src.read(entry.offset, n);
    if (bytes.size() != n) return std::nullopt;  // a short read: the file shrank since it was opened
    unmask(head.data() + kSngXorMaskOffset, bytes.data(), n, bytes.data());
    return bytes;
}

std::optional<std::vector<uint8_t>> sng_decode_file(const std::vector<uint8_t>& buf,
                                                    const SngFileEntry& entry) {
    std::optional<std::vector<uint8_t>> out(std::in_place);
    if (!sng_decode_file_into(buf, entry, *out)) return std::nullopt;
    return out;
}

}  // namespace hydra
