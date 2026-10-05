#include "core/audio_sniff.h"

#include <cstring>

namespace hydra::audio {

namespace {

// data[off..] begins with the literal `lit` (a plain ASCII tag, no NUL).
bool has_tag(const uint8_t* data, std::size_t size, std::size_t off,
             const char* lit) {
    std::size_t n = std::strlen(lit);
    if (off + n > size) return false;
    return std::memcmp(data + off, lit, n) == 0;
}

// The literal `lit` appears anywhere in the first `limit` bytes. Used to read
// the codec tag out of an Ogg stream's first page without parsing the page.
bool contains_tag(const uint8_t* data, std::size_t size, std::size_t limit,
                  const char* lit) {
    std::size_t n = std::strlen(lit);
    std::size_t end = size < limit ? size : limit;
    if (n == 0 || end < n) return false;
    for (std::size_t i = 0; i + n <= end; ++i)
        if (std::memcmp(data + i, lit, n) == 0) return true;
    return false;
}

// ID3v2's fixed sizes (the ID3v2.4 structure document, section 3): the
// header and the optional footer are 10 bytes each; header byte 5 holds the
// flags, and flag 0x10 says a footer follows the tag.
constexpr std::size_t kId3HeaderBytes = 10;
constexpr std::size_t kId3FooterBytes = 10;
constexpr uint8_t kId3FooterFlag = 0x10;

}  // namespace

std::size_t id3v2_tag_length(const uint8_t* data, std::size_t size) {
    if (data == nullptr || size < kId3HeaderBytes || !has_tag(data, size, 0, "ID3")) return 0;
    // The size is syncsafe: four bytes of seven bits each, high byte first.
    const std::size_t body = (static_cast<std::size_t>(data[6] & 0x7F) << 21) |
                             (static_cast<std::size_t>(data[7] & 0x7F) << 14) |
                             (static_cast<std::size_t>(data[8] & 0x7F) << 7) |
                             static_cast<std::size_t>(data[9] & 0x7F);
    const std::size_t footer = (data[5] & kId3FooterFlag) ? kId3FooterBytes : 0;
    return kId3HeaderBytes + body + footer;
}

AudioFormat sniff_format(const uint8_t* data, std::size_t size) {
    if (data == nullptr || size < 2) return AudioFormat::Unknown;

    if (has_tag(data, size, 0, "RIFF") && has_tag(data, size, 8, "WAVE"))
        return AudioFormat::Wav;
    if (has_tag(data, size, 0, "fLaC")) return AudioFormat::Flac;

    if (has_tag(data, size, 0, "OggS")) {
        // Two codecs share the OggS container; the first page names which.
        if (contains_tag(data, size, 64, kOpusHeadTag)) return AudioFormat::OggOpus;
        if (contains_tag(data, size, 64, "vorbis")) return AudioFormat::OggVorbis;
        return AudioFormat::Unknown;
    }

    // An ID3v2 tag: skip it, and any tag stacked after it, as dr_flac does,
    // then classify what follows. "fLaC" is a FLAC stream someone tagged;
    // anything else, a tag that runs past the bytes given included, is MP3.
    if (has_tag(data, size, 0, "ID3")) {
        std::size_t off = 0;
        while (off < size) {
            const std::size_t tag = id3v2_tag_length(data + off, size - off);
            if (tag == 0) break;
            off += tag;
        }
        return has_tag(data, size, off, "fLaC") ? AudioFormat::Flac : AudioFormat::Mp3;
    }
    // MP3: a raw frame sync (11 set bits: FF Ex/Fx).
    if (data[0] == 0xFF && (data[1] & 0xE0) == 0xE0) return AudioFormat::Mp3;

    return AudioFormat::Unknown;
}

}  // namespace hydra::audio
