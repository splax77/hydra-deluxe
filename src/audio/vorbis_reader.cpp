// The Ogg Vorbis stem reader, on stb_vorbis's pull decoder.
//
// The old whole-file decode called stb_vorbis_decode_memory, whose output
// buffer size is a 32-bit int that doubles as it fills; past 2^29 frames
// (about 3.1 hours of stereo 48 kHz) it overflowed and the stem played silent.
// Pulling a few thousand frames at a time never builds that buffer.
//
// Samples go through int16 exactly as before (stb's short conversion, then
// / 32768), so a straight-through read is bit-identical to the old decode.

#include "audio/stem_reader.h"

#include <algorithm>
#include <climits>
#include <stdexcept>
#include <utility>
#include <vector>

// stb_vorbis is compiled as its own TU (third_party/stb/stb_vorbis.c); take only
// its prototypes here and link the implementation.
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

namespace hydra::audio::detail {

namespace {

constexpr int kChunkFrames = 4096;

class VorbisReader final : public StemReader {
public:
    explicit VorbisReader(StemBytes bytes) : bytes_(std::move(bytes)) {
        if (bytes_.size() > static_cast<std::size_t>(INT_MAX))
            throw std::runtime_error(
                "decode_audio: Ogg Vorbis stream is over 2 GB, which stb_vorbis can't open");
        int err = 0;
        v_ = stb_vorbis_open_memory(bytes_.data(), static_cast<int>(bytes_.size()), &err,
                                    nullptr);
        if (v_ == nullptr)
            throw std::runtime_error("decode_audio: stb_vorbis could not decode the stream");
        const stb_vorbis_info info = stb_vorbis_get_info(v_);
        channels_ = info.channels;
        rate_ = static_cast<int>(info.sample_rate);
        length_ = stb_vorbis_stream_length_in_samples(v_);
        if (channels_ <= 0 || rate_ <= 0) {
            stb_vorbis_close(v_);
            throw std::runtime_error("decode_audio: stb_vorbis could not decode the stream");
        }
        scratch_.resize(static_cast<std::size_t>(kChunkFrames) * channels_);
    }

    ~VorbisReader() override { stb_vorbis_close(v_); }

    int channels() const override { return channels_; }
    int sample_rate() const override { return rate_; }
    int64_t length_frames() const override { return length_; }

    int64_t read(float* out, int64_t frames) override {
        int64_t done = 0;
        while (!at_end_ && done < frames) {
            const int want = static_cast<int>(std::min<int64_t>(kChunkFrames, frames - done));
            const int got = stb_vorbis_get_samples_short_interleaved(
                v_, channels_, scratch_.data(), want * channels_);
            if (got <= 0) {
                at_end_ = true;
                break;
            }
            float* dst = out + static_cast<std::size_t>(done) * channels_;
            const std::size_t count = static_cast<std::size_t>(got) * channels_;
            for (std::size_t i = 0; i < count; ++i) dst[i] = scratch_[i] / 32768.0f;
            done += got;
        }
        return done;
    }

    void seek(int64_t frame) override {
        frame = std::clamp<int64_t>(frame, 0, length_);
        if (frame >= length_) {
            at_end_ = true;
            return;
        }
        at_end_ = stb_vorbis_seek(v_, static_cast<unsigned>(frame)) == 0;
    }

private:
    StemBytes bytes_;
    stb_vorbis* v_ = nullptr;
    int channels_ = 0;
    int rate_ = 0;
    int64_t length_ = 0;
    std::vector<short> scratch_;
    bool at_end_ = false;
};

}  // namespace

std::unique_ptr<StemReader> open_vorbis_reader(StemBytes bytes) {
    return std::make_unique<VorbisReader>(std::move(bytes));
}

}  // namespace hydra::audio::detail
