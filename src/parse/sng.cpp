#include "parse/sng.h"

namespace hydra {

namespace {

// n bytes starting at pos lie inside buf (no overflow for any n).
bool fits(const std::vector<uint8_t>& buf, size_t pos, uint64_t n) {
    return pos <= buf.size() && n <= buf.size() - pos;
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
    if (!fits(buf, kSngMetadataLenOffset, 8)) return out;
    const uint64_t metadata_len = u64_at(buf, kSngMetadataLenOffset);
    if (!fits(buf, kSngMetadataOffset, metadata_len)) return out;
    size_t pos = kSngMetadataOffset + static_cast<size_t>(metadata_len);
    if (!fits(buf, pos, 16)) return out;
    pos += 8;  // the file section's length; entries carry absolute offsets
    const uint64_t count = u64_at(buf, pos);
    pos += 8;
    for (uint64_t i = 0; i < count; ++i) {
        if (!fits(buf, pos, 1)) break;
        const size_t name_len = buf[pos];
        pos += 1;
        if (!fits(buf, pos, name_len + 16)) break;
        SngFileEntry e;
        e.name = string_at(buf, pos, name_len);
        pos += name_len;
        e.length = u64_at(buf, pos);
        pos += 8;
        e.offset = u64_at(buf, pos);
        pos += 8;
        out.push_back(std::move(e));
    }
    return out;
}

bool sng_decode_file_into(const std::vector<uint8_t>& buf, const SngFileEntry& entry,
                          std::vector<uint8_t>& out) {
    if (!fits(buf, kSngXorMaskOffset, kSngXorMaskSize)) return false;
    if (entry.offset > buf.size() || entry.length > buf.size() - entry.offset) return false;
    // Byte i's key, mask[i % 16] ^ (i & 0xff), repeats every 256 bytes, so it
    // is built once and the bytes are unmasked a block at a time.
    const uint8_t* mask = buf.data() + kSngXorMaskOffset;
    uint8_t key[256];
    for (size_t j = 0; j < 256; ++j)
        key[j] = static_cast<uint8_t>(mask[j % kSngXorMaskSize] ^ j);
    const uint8_t* src = buf.data() + static_cast<size_t>(entry.offset);
    const size_t n = static_cast<size_t>(entry.length);
    out.resize(n);
    uint8_t* dst = out.data();
    for (size_t base = 0; base < n; base += 256) {
        const size_t len = n - base < 256 ? n - base : 256;
        for (size_t j = 0; j < len; ++j)
            dst[base + j] = static_cast<uint8_t>(src[base + j] ^ key[j]);
    }
    return true;
}

std::optional<std::vector<uint8_t>> sng_decode_file(const std::vector<uint8_t>& buf,
                                                    const SngFileEntry& entry) {
    std::optional<std::vector<uint8_t>> out(std::in_place);
    if (!sng_decode_file_into(buf, entry, *out)) return std::nullopt;
    return out;
}

}  // namespace hydra
