#include "audio/stretch_mix.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <string>
#include <utility>

#include "core/error_kind.h"

// SoundTouch is third-party code: its header is read with warnings off, the
// way its own target builds (CMakeLists.txt).
#if defined(_MSC_VER)
#pragma warning(push, 0)
#endif
#include "SoundTouch.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace hydra::audio {

namespace {

// Frames per block fed into SoundTouch, and the most played frames taken out
// in one go. The same block the stem mix reads in.
constexpr int64_t kBlockFrames = StreamMix::kBlockFrames;

// Where the start-delay probe puts its impulse, in frames after a restart, and
// it looks for the impulse in the first twice that many frames out. After a
// restart SoundTouch copies its first sequence straight through, with no
// overlap (TDStretch::processSamples, the isBeginning branch). That sequence is
// SoundTouch's sequence length less two overlaps; its auto settings never go
// below 40 ms less two 8 ms overlaps (TDStretch.cpp, AUTOSEQ_AT_MAX and
// DEFAULT_OVERLAP_MS), 192 frames even at 8 kHz. So the impulse and the whole
// search stay inside that straight copy, where one input frame is one output
// frame and nothing else can echo the impulse.
constexpr int64_t kProbeFrame = 64;

[[noreturn]] void fail(const std::string& what) {
    throw KindedError(ErrorKind::AudioDecode, "StretchSource: " + what);
}

unsigned int as_count(int64_t frames) { return static_cast<unsigned int>(std::max<int64_t>(frames, 0)); }

}  // namespace

StretchSource::StretchSource(std::unique_ptr<MixSource> plain, double speed_ratio)
    : plain_(std::move(plain)),
      ratio_(speed_ratio),
      channels_(plain_ ? plain_->channels() : 0),
      sample_rate_(plain_ ? plain_->sample_rate() : 0),
      file_length_(plain_ ? std::max<int64_t>(plain_->length_frames(), 0) : 0),
      length_(0) {
    if (!plain_) fail("no source to play");
    if (channels_ <= 0 || sample_rate_ <= 0) fail("the source has no playable format");
    if (!std::isfinite(ratio_) || ratio_ <= 0.0) fail("the speed ratio must be above 0");
    length_ = std::llround(static_cast<double>(file_length_) / ratio_);
    in_.assign(static_cast<std::size_t>(kBlockFrames) * channels_, 0.0f);
    try {
        st_ = std::make_unique<soundtouch::SoundTouch>();
        st_->setSampleRate(static_cast<unsigned int>(sample_rate_));
        st_->setChannels(static_cast<unsigned int>(channels_));
        st_->setTempo(ratio_);
        // Only the tempo changes and the rate stays at 1, so nothing is
        // resampled and the anti-alias filter has nothing to remove. Left on,
        // its 64-tap filter is an even length, which moves every frame by half
        // a frame. The filter's state is cleared by the next clear(), which
        // measure_start_delay() makes first.
        st_->setSetting(SETTING_USE_AA_FILTER, 0);
        measure_start_delay();
        grow_buffers();
        // Past the wrapped source's end, enough silence to push out every
        // frame SoundTouch still holds: under its initial latency waits in its
        // input, and each silent block pushes a block through.
        max_tail_feeds_ = std::max(st_->getSetting(SETTING_INITIAL_LATENCY), 0) / kBlockFrames + 2;
    } catch (const KindedError&) {
        throw;
    } catch (const std::exception& e) {
        fail(e.what());
    }
    seek(0);
}

StretchSource::~StretchSource() = default;

int64_t StretchSource::file_frame_of(int64_t played_frame) const {
    return std::clamp<int64_t>(std::llround(static_cast<double>(played_frame) * ratio_), 0,
                               file_length_);
}

// Feeds a buffer of silence with one impulse in it through a fresh SoundTouch
// and reads where the impulse comes out. The impulse goes in at kProbeFrame;
// if it comes out at frame q, every restart puts the first fed frame out
// q - kProbeFrame frames late. seek() uses this to start each restart on time.
void StretchSource::measure_start_delay() {
    const auto ch = static_cast<std::size_t>(channels_);
    const int64_t window = 2 * kProbeFrame;
    st_->clear();
    std::fill(in_.begin(), in_.end(), 0.0f);
    for (std::size_t c = 0; c < ch; ++c) in_[static_cast<std::size_t>(kProbeFrame) * ch + c] = 1.0f;
    st_->putSamples(in_.data(), as_count(kBlockFrames));
    std::fill(in_.begin(), in_.end(), 0.0f);
    // SoundTouch gives nothing out until its initial latency has gone in.
    const int64_t max_feeds = std::max(st_->getSetting(SETTING_INITIAL_LATENCY), 0) / kBlockFrames + 2;
    for (int64_t i = 0; i < max_feeds && static_cast<int64_t>(st_->numSamples()) < window; ++i)
        st_->putSamples(in_.data(), as_count(kBlockFrames));
    if (static_cast<int64_t>(st_->numSamples()) < window)
        fail("SoundTouch gave no output for the start-delay probe");
    st_->receiveSamples(in_.data(), as_count(window));
    int64_t peak = -1;
    float loudest = 0.0f;
    for (int64_t i = 0; i < window; ++i) {
        const float v = std::fabs(in_[static_cast<std::size_t>(i) * ch]);
        if (v > loudest) {
            loudest = v;
            peak = i;
        }
    }
    std::fill(in_.begin(), in_.end(), 0.0f);
    st_->clear();
    if (peak < 0) fail("the start-delay probe's impulse never came out");
    start_delay_ = peak - kProbeFrame;
}

// SoundTouch's buffers (FIFOSampleBuffer) grow when a put needs more room and
// never shrink, and clear() keeps them. So the constructor pushes more through
// than any read can hold at once, and from then on read() and seek() allocate
// nothing on the device thread.
//
// What a read can hold at once: SoundTouch's input keeps under its initial
// latency between feeds, and a feed adds one block. Its output holds under one
// read piece before a feed, and a feed adds at most one output sequence per
// input sequence it completes, plus the two short ones after a restart.
void StretchSource::grow_buffers() {
    const auto ch = static_cast<std::size_t>(channels_);
    const int64_t latency = std::max(st_->getSetting(SETTING_INITIAL_LATENCY), 0);
    const int64_t in_seq = std::max(st_->getSetting(SETTING_NOMINAL_INPUT_SEQUENCE), 1);
    const int64_t out_seq = std::max(st_->getSetting(SETTING_NOMINAL_OUTPUT_SEQUENCE), 1);
    const int64_t out_per_feed = ((latency + kBlockFrames) / in_seq + 2) * out_seq;
    // Twice what a read can hold, so a buffer's growth never lands on the edge.
    const int64_t in_target = 2 * (latency + kBlockFrames);
    const int64_t out_target = 2 * (kBlockFrames + out_per_feed);

    st_->clear();
    const std::vector<float> silence(static_cast<std::size_t>(in_target) * ch, 0.0f);
    st_->putSamples(silence.data(), as_count(in_target));
    // Each block fed gives about a block / ratio frames out.
    const auto max_feeds = static_cast<int64_t>(
        std::ceil(static_cast<double>(out_target) * ratio_ / static_cast<double>(kBlockFrames)) + 4);
    for (int64_t i = 0; i < max_feeds && static_cast<int64_t>(st_->numSamples()) < out_target; ++i)
        st_->putSamples(in_.data(), as_count(kBlockFrames));
    st_->clear();
}

bool StretchSource::feed() {
    const int64_t got = std::clamp<int64_t>(plain_->read(in_.data(), kBlockFrames), 0, kBlockFrames);
    if (got > 0) {
        st_->putSamples(in_.data(), as_count(got));
        return true;
    }
    if (tail_feeds_ >= max_tail_feeds_) return false;
    ++tail_feeds_;
    std::fill(in_.begin(), in_.end(), 0.0f);
    st_->putSamples(in_.data(), as_count(kBlockFrames));
    return true;
}

bool StretchSource::fill(int64_t need) {
    for (;;) {
        if (pending_drop_ > 0)
            pending_drop_ -= static_cast<int64_t>(st_->receiveSamples(as_count(pending_drop_)));
        if (pending_drop_ == 0 && static_cast<int64_t>(st_->numSamples()) >= need) return true;
        if (!feed()) return false;
    }
}

int64_t StretchSource::read(float* out, int64_t frames) {
    const int64_t want = std::clamp<int64_t>(frames, 0, std::max<int64_t>(length_ - pos_, 0));
    const auto ch = static_cast<std::size_t>(channels_);
    int64_t done = 0;
    try {
        while (done < want) {
            const int64_t piece = std::min(want - done, kBlockFrames);
            const bool ready = fill(piece);
            const auto got = static_cast<int64_t>(
                st_->receiveSamples(out + static_cast<std::size_t>(done) * ch, as_count(piece)));
            done += got;
            if (!ready || got == 0) break;
        }
    } catch (...) {
        // SoundTouch throws only for a missing rate or channel count, both set
        // in the constructor. Stop here rather than throw on the device thread.
    }
    pos_ += done;
    return done;
}

void StretchSource::seek(int64_t frame) {
    pos_ = std::clamp<int64_t>(frame, 0, length_);
    const int64_t file = file_frame_of(pos_);
    // A late start (delay above 0) is dropped from the output; an early one
    // (below 0) is fed that much sooner, so the frame at `file` comes out
    // first either way. At the file's start there is nothing sooner to feed.
    const int64_t start = std::max<int64_t>(0, file - std::max<int64_t>(0, -start_delay_));
    pending_drop_ = std::max<int64_t>(0, file - start + start_delay_);
    tail_feeds_ = 0;
    st_->clear();
    plain_->seek(start);
}

std::unique_ptr<MixSource> play_at_speed(std::unique_ptr<MixSource> plain, double speed_ratio) {
    if (!plain || speed_ratio == 1.0) return plain;
    return std::make_unique<StretchSource>(std::move(plain), speed_ratio);
}

}  // namespace hydra::audio
