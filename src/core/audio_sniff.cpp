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

}  // namespace

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

    // MP3: an ID3v2 tag, or a raw frame sync (11 set bits: FF Ex/Fx).
    if (has_tag(data, size, 0, "ID3")) return AudioFormat::Mp3;
    if (data[0] == 0xFF && (data[1] & 0xE0) == 0xE0) return AudioFormat::Mp3;

    return AudioFormat::Unknown;
}

}  // namespace hydra::audio
