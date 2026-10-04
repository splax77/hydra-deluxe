#include "audio/player.h"

#include <algorithm>
#include <cstring>
#include <utility>

#include "audio/frames.h"

namespace hydra::audio {

Playhead::Playhead(DecodedAudio mixed)
    : Playhead(std::make_unique<BufferSource>(std::move(mixed))) {}

Playhead::Playhead(std::unique_ptr<MixSource> source)
    : source_(std::move(source)),
      channels_(source_ ? source_->channels() : 0),
      sample_rate_(source_ ? source_->sample_rate() : 0),
      length_(source_ && channels_ > 0 ? std::max<int64_t>(source_->length_frames(), 0) : 0) {}

void Playhead::play() { playing_ = true; }
void Playhead::pause() { playing_ = false; }
void Playhead::toggle() { playing_ = !playing_; }

void Playhead::seek_frames(int64_t frame) {
    position_ = std::clamp<int64_t>(frame, 0, length_);
}

// A rate of 0 means no source; the three ms calls below answer that here and
// leave the conversion itself to audio/frames.h.
void Playhead::seek_ms(double ms) {
    if (sample_rate_ <= 0) return;
    seek_frames(frames_of_ms(ms, sample_rate_));
}

double Playhead::position_ms() const {
    return sample_rate_ > 0 ? ms_of_frames(position_, sample_rate_) : 0.0;
}

double Playhead::length_ms() const {
    return sample_rate_ > 0 ? ms_of_frames(length_, sample_rate_) : 0.0;
}

int64_t Playhead::read_frames(float* out, int64_t frame_count) {
    if (frame_count <= 0) return 0;
    std::size_t total = static_cast<std::size_t>(frame_count) * channels_;

    if (!playing_) {
        std::memset(out, 0, total * sizeof(float));
        return 0;
    }

    int64_t avail = length_ - position_;
    int64_t n = std::min<int64_t>(frame_count, std::max<int64_t>(avail, 0));

    std::size_t written = 0;
    if (n > 0) {
        // A seek (or resume after one) moved the playhead: move the source
        // once, here on the device thread. A resume from where the source
        // already is needs no seek, so pause/play doesn't restart decoders.
        if (source_position_ != position_) {
            source_->seek(position_);
            source_position_ = position_;
        }
        const int64_t got = std::clamp<int64_t>(source_->read(out, n), 0, n);
        source_position_ += got;
        written = static_cast<std::size_t>(got) * channels_;
        if (gain_ != 1.0f)
            for (std::size_t i = 0; i < written; ++i) out[i] *= gain_;
    }
    // Silence any frames past the end of the mix (or a source that came up
    // short, which then gets re-seeked on the next read).
    if (written < total)
        std::memset(out + written, 0, (total - written) * sizeof(float));

    position_ += n;
    if (position_ >= length_) {
        position_ = length_;  // clamp; the master clock rests at the end
        playing_ = false;     // auto-pause at end of stream
    }
    return n;
}

}  // namespace hydra::audio
