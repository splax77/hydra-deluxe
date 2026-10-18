#include "ui/preview_transport.h"

#include <utility>

#include "app/preview_view.h"  // has_song_length

namespace hydra::ui {

PreviewTransport::PreviewTransport(app::PreviewClock::Now now)
    : clock_(std::move(now)) {}

void PreviewTransport::load(std::unique_ptr<audio::Playhead> playhead, double audio_offset_ms) {
    std::lock_guard<std::mutex> lock(mu_);
    playhead_ = std::move(playhead);
    audio_offset_ms_ = audio_offset_ms;
    if (playhead_) {
        playhead_->pause();
        playhead_->seek_ms(audio_ms_of_chart_ms(0.0, audio_offset_ms_));
        playhead_->set_gain(gain_);
    }
    clock_.pause();
    clock_.seek_ms(0.0);
}

void PreviewTransport::unload() {
    std::lock_guard<std::mutex> lock(mu_);
    playhead_.reset();
    song_length_ms_.reset();
    audio_offset_ms_ = 0.0;
    clock_.pause();
    clock_.seek_ms(0.0);
}

void PreviewTransport::play() {
    {
        std::lock_guard<std::mutex> lock(mu_);
        if (playhead_) {
            playhead_->seek_ms(audio_ms_of_chart_ms(clock_.now_ms(), audio_offset_ms_));
            playhead_->play();
        }
    }
    clock_.play();  // a chart with no audio still plays: the clock is the master
}

void PreviewTransport::pause() {
    clock_.pause();
    std::lock_guard<std::mutex> lock(mu_);
    if (playhead_) playhead_->pause();
}

void PreviewTransport::toggle() {
    if (clock_.playing())
        pause();
    else
        play();
}

void PreviewTransport::seek_ms(double ms) {
    const double end = length_ms();
    if (ms < 0.0) ms = 0.0;
    if (end > 0.0 && ms > end) ms = end;
    clock_.seek_ms(ms);
    std::lock_guard<std::mutex> lock(mu_);
    if (!playhead_) return;
    playhead_->seek_ms(audio_ms_of_chart_ms(ms, audio_offset_ms_));
    // The clock is the master: while it plays, the audio plays too. The
    // playhead pauses itself when its audio runs out, so a jump back from
    // past that end would otherwise stay silent (finding 73).
    if (clock_.playing()) playhead_->play();
}

bool PreviewTransport::playing() const { return clock_.playing(); }

double PreviewTransport::length_ms() const {
    return song_length_ms_ && app::has_song_length(*song_length_ms_) ? *song_length_ms_ : 0.0;
}

void PreviewTransport::set_song_length_ms(std::optional<double> song_length_ms) {
    song_length_ms_ = song_length_ms;
    // A shorter end must not leave the clock past it. seek_ms owns the clamp,
    // so hand it the clock's own time.
    const double end = length_ms();
    if (end > 0.0 && clock_.now_ms() > end) seek_ms(clock_.now_ms());
}

bool PreviewTransport::has_audio() const {
    std::lock_guard<std::mutex> lock(mu_);
    return playhead_ && playhead_->length_frames() > 0;
}

double PreviewTransport::tick() {
    // Stop where playback ends (length_ms) rather than scrolling into the void.
    const double end = length_ms();
    if (clock_.playing() && end > 0.0 && clock_.now_ms() >= end) {
        pause();
        seek_ms(end);
    }
    return now_ms();
}

double PreviewTransport::now_ms() const { return clock_.now_ms(); }

// Remembered as given; audio::Playhead::set_gain owns the floor at 0.
void PreviewTransport::set_gain(float gain) {
    gain_ = gain;
    std::lock_guard<std::mutex> lock(mu_);
    if (playhead_) playhead_->set_gain(gain_);
}

int64_t PreviewTransport::read_frames(float* out, int64_t frame_count) {
    std::lock_guard<std::mutex> lock(mu_);
    if (playhead_) return playhead_->read_frames(out, frame_count);
    return 0;
}

int PreviewTransport::channels() const {
    std::lock_guard<std::mutex> lock(mu_);
    return playhead_ ? playhead_->channels() : 0;
}

int PreviewTransport::sample_rate() const {
    std::lock_guard<std::mutex> lock(mu_);
    return playhead_ ? playhead_->sample_rate() : 0;
}

}  // namespace hydra::ui
