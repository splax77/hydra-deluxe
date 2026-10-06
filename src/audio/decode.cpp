#include "audio/decode.h"

#include <memory>
#include <stdexcept>
#include <utility>

#include "audio/stem_reader.h"

namespace hydra::audio {

namespace {

// Reads a whole stem through its reader: exactly length_frames() frames are
// reserved up front, so the buffer never grows or copies. Should a decoder
// yield more than it promised, the extra is kept (the buffer grows), so the
// output always equals reading to the end. A decode error part way through
// throws, as the old whole-file decoders did.
DecodedAudio read_all(StemReader& r) {
    DecodedAudio out;
    out.channels = r.channels();
    out.sample_rate = r.sample_rate();
    if (out.channels <= 0) throw KindedError(ErrorKind::AudioDecode, "decode_audio: the stream has no channels");
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
    if (r.failed()) throw KindedError(ErrorKind::AudioDecode, "decode_audio: the stream failed to decode");
    out.samples.resize(static_cast<std::size_t>(n) * ch);
    return out;
}

}  // namespace

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
