#include "audio/decode.h"

#include <cstring>
#include <memory>
#include <stdexcept>
#include <utility>

#include "audio/stem_reader.h"

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

// Reads a whole stem through its reader: exactly length_frames() frames are
// reserved up front, so the buffer never grows or copies. Should a decoder
// yield more than it promised, the extra is kept (the buffer grows), so the
// output always equals reading to the end. A decode error part way through
// throws, as the old whole-file decoders did.
DecodedAudio read_all(StemReader& r) {
    DecodedAudio out;
    out.channels = r.channels();
    out.sample_rate = r.sample_rate();
    if (out.channels <= 0) throw std::runtime_error("decode_audio: the stream has no channels");
    const std::size_t ch = static_cast<std::size_t>(out.channels);
    int64_t cap = r.length_frames();
    out.samples.resize(static_cast<std::size_t>(cap) * ch);
    int64_t n = 0;
    while (n < cap) {
        const int64_t got = r.read(out.samples.data() + static_cast<std::size_t>(n) * ch, cap - n);
        if (got <= 0) break;
        n += got;
    }
    if (n == cap) {
        // Promised length reached; anything past it lands in a side buffer.
        std::vector<float> extra(4096 * ch);
        while (const int64_t got = r.read(extra.data(), 4096)) {
            out.samples.resize(static_cast<std::size_t>(n) * ch);
            out.samples.insert(out.samples.end(), extra.begin(),
                               extra.begin() + static_cast<std::ptrdiff_t>(got * out.channels));
            n += got;
        }
    }
    if (r.failed()) throw std::runtime_error("decode_audio: the stream failed to decode");
    out.samples.resize(static_cast<std::size_t>(n) * ch);
    return out;
}

}  // namespace

AudioFormat sniff_format(const uint8_t* data, std::size_t size) {
    if (data == nullptr || size < 2) return AudioFormat::Unknown;

    if (has_tag(data, size, 0, "RIFF") && has_tag(data, size, 8, "WAVE"))
        return AudioFormat::Wav;
    if (has_tag(data, size, 0, "fLaC")) return AudioFormat::Flac;

    if (has_tag(data, size, 0, "OggS")) {
        // Two codecs share the OggS container; the first page names which.
        if (contains_tag(data, size, 64, "OpusHead")) return AudioFormat::OggOpus;
        if (contains_tag(data, size, 64, "vorbis")) return AudioFormat::OggVorbis;
        return AudioFormat::Unknown;
    }

    // MP3: an ID3v2 tag, or a raw frame sync (11 set bits: FF Ex/Fx).
    if (has_tag(data, size, 0, "ID3")) return AudioFormat::Mp3;
    if (data[0] == 0xFF && (data[1] & 0xE0) == 0xE0) return AudioFormat::Mp3;

    return AudioFormat::Unknown;
}

DecodedAudio decode_audio(const uint8_t* data, std::size_t size) {
    StemBytes bytes;
    if (data != nullptr) bytes.owned.assign(data, data + size);
    std::unique_ptr<StemReader> r = open_stem_reader(std::move(bytes));
    return read_all(*r);
}

DecodedAudio decode_stem(const app::PreviewAudioStem& stem) {
    std::unique_ptr<StemReader> r = open_stem_reader(stem);
    return read_all(*r);
}

}  // namespace hydra::audio
