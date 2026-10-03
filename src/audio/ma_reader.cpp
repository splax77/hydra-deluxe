// The WAV / MP3 / FLAC stem reader, on miniaudio's ma_decoder (its dr_libs).
//
// Output is float at the file's own channel count and rate, as the old
// whole-file decode gave. WAV and FLAC seek exactly and instantly.
//
// MP3 needs two workarounds for the vendored dr_mp3 (miniaudio 0.11.25):
//   * An MP3 with a LAME/Info tag starts with encoder delay (often 1105
//     frames) that reading skips, but dr_mp3 seeks in raw frames that still
//     count it, so a plain seek lands the delay early. The reader measures the
//     delay once at open (read one frame, see how far the backend's raw cursor
//     moved) and seeks to frame + delay.
//   * Its seek table (seekPointCount > 0) lands thousands of frames off on the
//     sine fixture, so it stays off (seekPointCount = 0). A forward seek then
//     decodes ahead from where the reader is; a backward one restarts and
//     decodes forward. Both are exact; a far backward seek in a long MP3 costs
//     decoding time.

#include "audio/stem_reader.h"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

// miniaudio's configuration macros come from the miniaudio target
// (CMakeLists.txt), the same set its implementation TU is compiled with.
#include "miniaudio.h"

namespace hydra::audio::detail {

namespace {

constexpr ma_uint64 kSkipChunk = 4096;

class MaReader final : public StemReader {
public:
    explicit MaReader(StemBytes bytes) : bytes_(std::move(bytes)) {
        ma_decoder_config cfg = ma_decoder_config_init(ma_format_f32, 0, 0);
        cfg.seekPointCount = 0;  // dr_mp3's seek table is wrong; see the top
        if (ma_decoder_init_memory(bytes_.data(), bytes_.size(), &cfg, &dec_) != MA_SUCCESS)
            throw std::runtime_error("decode_audio: miniaudio could not open the stream");
        channels_ = static_cast<int>(dec_.outputChannels);
        rate_ = static_cast<int>(dec_.outputSampleRate);
        if (channels_ <= 0) {
            ma_decoder_uninit(&dec_);
            throw std::runtime_error("decode_audio: miniaudio could not open the stream");
        }
        ma_uint64 len = 0;
        if (ma_decoder_get_length_in_pcm_frames(&dec_, &len) != MA_SUCCESS) len = 0;
        length_ = static_cast<int64_t>(len);
        try {
            scratch_.resize(static_cast<std::size_t>(kSkipChunk) * channels_);
            measure_delay();
        } catch (...) {
            ma_decoder_uninit(&dec_);
            throw;
        }
    }

    ~MaReader() override { ma_decoder_uninit(&dec_); }

    int channels() const override { return channels_; }
    int sample_rate() const override { return rate_; }
    int64_t length_frames() const override { return length_; }
    bool failed() const override { return failed_; }

    int64_t read(float* out, int64_t frames) override {
        int64_t done = 0;
        while (!at_end_ && done < frames) {
            ma_uint64 got = 0;
            const ma_result r = ma_decoder_read_pcm_frames(
                &dec_, out + static_cast<std::size_t>(done) * channels_,
                static_cast<ma_uint64>(frames - done), &got);
            done += static_cast<int64_t>(got);
            if (r == MA_AT_END || got == 0) {
                at_end_ = true;
            } else if (r != MA_SUCCESS) {
                failed_ = true;
                at_end_ = true;
            }
        }
        return done;
    }

    void seek(int64_t frame) override {
        frame = std::clamp<int64_t>(frame, 0, length_);
        if (frame >= length_) {
            at_end_ = true;
            return;
        }
        at_end_ = !seek_exact(static_cast<ma_uint64>(frame));
    }

private:
    // The backend's raw frame cursor (MP3: counts the skipped encoder delay).
    ma_uint64 raw_cursor() {
        ma_uint64 c = 0;
        if (ma_data_source_get_cursor_in_pcm_frames(dec_.pBackend, &c) != MA_SUCCESS) return ~0ull;
        return c;
    }

    // Reads one frame from the start and sees how far the raw cursor moved;
    // anything past one frame is delay the reader skipped. Then rewinds, so
    // a straight read still starts from a freshly reset decoder.
    void measure_delay() {
        if (!dec_.converter.isPassthrough || length_ <= 0) return;
        const ma_uint64 before = raw_cursor();
        ma_uint64 got = 0;
        ma_decoder_read_pcm_frames(&dec_, scratch_.data(), 1, &got);
        const ma_uint64 after = raw_cursor();
        if (got == 1 && before != ~0ull && after != ~0ull && after > before)
            delay_ = after - before - 1;
        if (ma_decoder_seek_to_pcm_frame(&dec_, 0) != MA_SUCCESS)
            throw std::runtime_error("decode_audio: miniaudio could not rewind the stream");
    }

    bool seek_exact(ma_uint64 frame) {
        // No delay (WAV, FLAC, an MP3 without a LAME tag): raw frames are
        // stem frames, and miniaudio's own seek is exact.
        if (delay_ == 0 || frame == 0)
            return ma_decoder_seek_to_pcm_frame(&dec_, frame) == MA_SUCCESS;
        // Forward from a position past the delay, dr_mp3 decodes ahead in raw
        // frames, which is exact once the delay is added.
        const ma_uint64 raw = frame + delay_;
        const ma_uint64 cur = raw_cursor();
        if (cur != ~0ull && cur >= delay_ && raw >= cur &&
            ma_decoder_seek_to_pcm_frame(&dec_, raw) == MA_SUCCESS && raw_cursor() == raw)
            return true;
        // Backward (or anything unexpected): restart and decode forward.
        if (ma_decoder_seek_to_pcm_frame(&dec_, 0) != MA_SUCCESS) return false;
        ma_uint64 left = frame;
        while (left > 0) {
            ma_uint64 got = 0;
            ma_decoder_read_pcm_frames(&dec_, scratch_.data(), std::min(left, kSkipChunk), &got);
            if (got == 0) return false;
            left -= got;
        }
        return true;
    }

    StemBytes bytes_;
    ma_decoder dec_{};
    int channels_ = 0;
    int rate_ = 0;
    int64_t length_ = 0;
    ma_uint64 delay_ = 0;          // raw frames skipped before frame 0
    std::vector<float> scratch_;   // decode target for the delay probe / skip
    bool at_end_ = false;
    bool failed_ = false;
};

}  // namespace

std::unique_ptr<StemReader> open_ma_reader(StemBytes bytes) {
    return std::make_unique<MaReader>(std::move(bytes));
}

}  // namespace hydra::audio::detail
