#include "audio/decode.h"

#include <memory>
#include <stdexcept>
#include <utility>

#include "audio/stem_reader.h"

namespace hydra::audio {

namespace {

// Reads a whole stem through its reader, to the end. The buffer grows as
// frames arrive; nothing is sized from length_frames(), because that count
// comes from the file's header and a damaged header can promise far more than
// the decoder delivers (a fuzzed FLAC asked for hundreds of gigabytes). The
// output is exactly what the reader yields, so frames past the promise are
// kept and a stem that ends early is shorter. Which readers report a decode
// error is said at StemReader::failed(); when one does, this throws, as the
// old whole-file decoders did.
DecodedAudio read_all(StemReader& r) {
    DecodedAudio out;
    out.channels = r.channels();
    out.sample_rate = r.sample_rate();
    if (out.channels <= 0) throw KindedError(ErrorKind::AudioDecode, "decode_audio: the stream has no channels");
    const std::size_t ch = static_cast<std::size_t>(out.channels);
    constexpr int64_t kChunkFrames = 4096;
    std::vector<float> chunk(static_cast<std::size_t>(kChunkFrames) * ch);
    while (true) {
        const int64_t got = r.read(chunk.data(), kChunkFrames);
        if (got <= 0) break;
        out.samples.insert(out.samples.end(), chunk.begin(),
                           chunk.begin() + static_cast<std::ptrdiff_t>(static_cast<std::size_t>(got) * ch));
    }
    if (r.failed()) throw KindedError(ErrorKind::AudioDecode, "decode_audio: the stream failed to decode");
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
