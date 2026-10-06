#include "audio/stem_reader.h"

#include <stdexcept>
#include <utility>

#include "app/preview_source.h"
#include "audio/decode.h"

namespace hydra::audio {

std::unique_ptr<StemReader> open_stem_reader(StemBytes bytes,
                                             const OpenProgress& progress) {
    const AudioFormat format = sniff_format(bytes.data(), bytes.size());
    switch (format) {
        case AudioFormat::Wav:
        case AudioFormat::Mp3:
        case AudioFormat::Flac:
            return detail::open_ma_reader(std::move(bytes), format);
        case AudioFormat::OggVorbis:
            return detail::open_vorbis_reader(std::move(bytes));
        case AudioFormat::OggOpus:
            return detail::open_opus_reader(std::move(bytes), progress);
        default:
            throw KindedError(ErrorKind::AudioDecode, "decode_audio: unrecognized audio container");
    }
}

std::unique_ptr<StemReader> open_stem_reader(const app::PreviewAudioStem& stem,
                                             const OpenProgress& progress) {
    StemBytes bytes;
    if (stem.from_file())
        bytes.mapped = MappedFile::open(stem.path);
    else
        bytes.owned = stem.bytes;
    return open_stem_reader(std::move(bytes), progress);
}

}  // namespace hydra::audio
