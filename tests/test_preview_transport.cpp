// Tests for ui/preview_transport: the Preview's Transport — play, pause and
// seek together with the master clock, and the audio playhead that follows it.
// The monotonic source is a fake, so the timing is exact; no device is opened,
// and read_frames() is exactly what a device callback would pull.

#include "doctest.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "app/preview_view.h"  // scrub_end_ms
#include "audio/decode.h"
#include "audio/player.h"
#include "audio/stem_reader.h"
#include "audio/stream_mix.h"
#include "display_fixtures.h"  // audio_tail_chart
#include "ui/preview_transport.h"

using hydra::audio::DecodedAudio;
using hydra::audio::Playhead;
using hydra::ui::PreviewTransport;

namespace {

// `frames` stereo frames at 48 kHz where frame i holds {L=i, R=i+0.5}, so a
// copied block is trivially recognizable (as in test_audio_player).
DecodedAudio make_ramp(int frames) {
    DecodedAudio a;
    a.channels = 2;
    a.sample_rate = 48000;
    a.samples.resize(static_cast<size_t>(frames) * 2);
    for (int i = 0; i < frames; ++i) {
        a.samples[i * 2] = static_cast<float>(i);
        a.samples[i * 2 + 1] = static_cast<float>(i) + 0.5f;
    }
    return a;
}

// A playhead `ms` milliseconds long at 48 kHz stereo.
std::unique_ptr<Playhead> make_playhead(double ms) {
    return std::make_unique<Playhead>(make_ramp(static_cast<int>(ms * 48.0)));
}

// A stem reader over an already-decoded buffer, for StreamMix.
class RampReader : public hydra::audio::StemReader {
public:
    explicit RampReader(DecodedAudio a) : a_(std::move(a)) {}
    int channels() const override { return a_.channels; }
    int sample_rate() const override { return a_.sample_rate; }
    int64_t length_frames() const override { return a_.frames(); }
    int64_t read(float* out, int64_t frames) override {
        const int64_t n = std::min(frames, a_.frames() - pos_);
        if (n <= 0) return 0;
        std::memcpy(out, a_.samples.data() + pos_ * a_.channels,
                    static_cast<size_t>(n * a_.channels) * sizeof(float));
        pos_ += n;
        return n;
    }
    void seek(int64_t frame) override { pos_ = std::clamp<int64_t>(frame, 0, a_.frames()); }

private:
    DecodedAudio a_;
    int64_t pos_ = 0;
};

}  // namespace

TEST_CASE("an unloaded transport plays and scrubs with no audio") {
    double t = 10.0;  // fake monotonic seconds
    PreviewTransport transport([&] { return t; });

    CHECK_FALSE(transport.has_audio());
    CHECK(transport.length_ms() == doctest::Approx(0.0));
    CHECK(transport.channels() == 0);
    CHECK(transport.sample_rate() == 0);

    transport.play();
    CHECK(transport.playing());
    t += 0.5;
    CHECK(transport.now_ms() == doctest::Approx(500.0));
    // With no length there is no end to stop at: tick() never pauses.
    CHECK(transport.tick() == doctest::Approx(500.0));
    CHECK(transport.playing());

    transport.seek_ms(20000.0);  // scrubbing past a zero length is allowed
    CHECK(transport.now_ms() == doctest::Approx(20000.0));
}

TEST_CASE("load takes the later of last note and audio end as the length") {
    PreviewTransport transport([] { return 0.0; });

    transport.load(make_playhead(1000.0), 1500.0);
    CHECK(transport.length_ms() == doctest::Approx(1500.0));  // notes run longer
    CHECK(transport.has_audio());
    CHECK(transport.channels() == 2);
    CHECK(transport.sample_rate() == 48000);

    transport.load(make_playhead(1000.0), 200.0);
    CHECK(transport.length_ms() == doctest::Approx(1000.0));  // audio runs longer

    transport.unload();
    CHECK(transport.length_ms() == doctest::Approx(0.0));
    CHECK_FALSE(transport.has_audio());
    CHECK(transport.now_ms() == doctest::Approx(0.0));
}

TEST_CASE("play seeks the playhead to the clock and both run") {
    double t = 0.0;
    PreviewTransport transport([&] { return t; });

    auto* playhead = new Playhead(make_ramp(48000));  // 1000 ms
    transport.load(std::unique_ptr<Playhead>(playhead), 0.0);
    CHECK_FALSE(playhead->playing());

    transport.seek_ms(250.0);
    CHECK(playhead->position_ms() == doctest::Approx(250.0));

    transport.play();
    CHECK(transport.playing());
    CHECK(playhead->playing());
    CHECK(playhead->position_ms() == doctest::Approx(250.0));

    transport.pause();
    CHECK_FALSE(transport.playing());
    CHECK_FALSE(playhead->playing());

    transport.toggle();
    CHECK(transport.playing());
    CHECK(playhead->playing());
    transport.toggle();
    CHECK_FALSE(transport.playing());
    CHECK_FALSE(playhead->playing());
}

TEST_CASE("tick pauses at the end and pins the time") {
    double t = 0.0;
    PreviewTransport transport([&] { return t; });
    transport.load(make_playhead(1000.0), 0.0);
    CHECK(transport.length_ms() == doctest::Approx(1000.0));

    transport.play();
    t += 0.4;
    CHECK(transport.tick() == doctest::Approx(400.0));  // still short of the end
    CHECK(transport.playing());

    t += 1.1;  // 1500 ms in, past the end
    CHECK(transport.tick() == doctest::Approx(1000.0));
    CHECK_FALSE(transport.playing());
    CHECK(transport.now_ms() == doctest::Approx(1000.0));

    t += 5.0;  // paused: the time stays pinned
    CHECK(transport.tick() == doctest::Approx(1000.0));
}

// On a chart whose notes outlast its audio, the playhead pauses itself when
// the audio runs out while the clock plays on. A jump back used to move the
// playhead without restarting it: the highway scrolled in silence with the
// play button lit (finding 73). The clock is the master, so a seek while it
// plays sets the playhead playing again.
TEST_CASE("a seek while playing brings the audio back after it ran out") {
    double t = 0.0;
    PreviewTransport transport([&] { return t; });
    auto* playhead = new Playhead(make_ramp(48000));  // 1000 ms
    transport.load(std::unique_ptr<Playhead>(playhead), 2000.0);
    transport.play();

    std::vector<float> out(48004 * 2, -1.0f);
    CHECK(transport.read_frames(out.data(), 48004) == 48000);  // the audio runs out
    REQUIRE_FALSE(playhead->playing());
    REQUIRE(transport.playing());  // the notes go on

    transport.seek_ms(500.0);
    CHECK(playhead->playing());
    CHECK(playhead->position_ms() == doctest::Approx(500.0));
    CHECK(transport.read_frames(out.data(), 4) == 4);
    CHECK(out[0] == doctest::Approx(24000.0f));  // frame 24000, L
    CHECK(out[1] == doctest::Approx(24000.5f));  // frame 24000, R
}

TEST_CASE("seek clamps to [0, length]") {
    PreviewTransport transport([] { return 0.0; });
    transport.load(make_playhead(1000.0), 0.0);

    transport.seek_ms(-5.0);
    CHECK(transport.now_ms() == doctest::Approx(0.0));
    transport.seek_ms(400.0);
    CHECK(transport.now_ms() == doctest::Approx(400.0));
    transport.seek_ms(9999.0);
    CHECK(transport.now_ms() == doctest::Approx(transport.length_ms()));
}

TEST_CASE("read_frames serves audio while playing and silence while paused") {
    PreviewTransport transport([] { return 0.0; });
    transport.load(std::make_unique<Playhead>(make_ramp(5)), 0.0);

    std::vector<float> out(8, -1.0f);  // 4 stereo frames, sentinel-filled
    CHECK(transport.read_frames(out.data(), 4) == 0);  // paused: silence
    for (float v : out) CHECK(v == 0.0f);

    transport.play();
    CHECK(transport.read_frames(out.data(), 3) == 3);
    const float expect[] = {0, 0.5f, 1, 1.5f, 2, 2.5f};
    for (int i = 0; i < 6; ++i) CHECK(out[i] == doctest::Approx(expect[i]));

    // Two frames left: the tail is zero-filled and only the real frames count.
    CHECK(transport.read_frames(out.data(), 4) == 2);
    CHECK(out[0] == doctest::Approx(3.0f));
    CHECK(out[4] == doctest::Approx(0.0f));

    // With no playhead at all the pull is a no-op, not a crash.
    transport.unload();
    CHECK(transport.read_frames(out.data(), 4) == 0);
}

TEST_CASE("gain set before load applies to the next playhead") {
    PreviewTransport transport([] { return 0.0; });
    CHECK(transport.gain() == doctest::Approx(1.0f));

    transport.set_gain(0.5f);
    CHECK(transport.gain() == doctest::Approx(0.5f));

    auto* playhead = new Playhead(make_ramp(100));
    transport.load(std::unique_ptr<Playhead>(playhead), 0.0);
    CHECK(playhead->gain() == doctest::Approx(0.5f));

    transport.play();
    std::vector<float> out(8, -1.0f);
    CHECK(transport.read_frames(out.data(), 4) == 4);
    CHECK(out[2] == doctest::Approx(1.0f * 0.5f));  // frame 1, L
    CHECK(out[3] == doctest::Approx(1.5f * 0.5f));  // frame 1, R

    // A later change reaches the loaded playhead too.
    transport.set_gain(0.25f);
    CHECK(playhead->gain() == doctest::Approx(0.25f));
}

TEST_CASE("an audio offset seeks the playhead ahead of the clock") {
    double t = 0.0;
    PreviewTransport transport([&] { return t; });

    auto* playhead = new Playhead(make_ramp(96000));  // 2000 ms
    transport.load(std::unique_ptr<Playhead>(playhead), 0.0, /*audio_offset_ms=*/500.0);
    // The audio past the chart's end still plays out: 2000 - 500.
    CHECK(transport.length_ms() == doctest::Approx(1500.0));

    transport.seek_ms(250.0);
    CHECK(transport.now_ms() == doctest::Approx(250.0));
    CHECK(playhead->position_ms() == doctest::Approx(750.0));

    transport.play();
    CHECK(playhead->position_ms() == doctest::Approx(750.0));
}

// The scrubber's right edge is the last note (D50 item 4). On the audio-tail
// chart a drag to that edge seeks to the last note at 1000 ms, not to the
// audio's end at 6000 ms, while playback can still run on to 6000 ms.
TEST_CASE("scrubber: a drag to the right end seeks to the last note, not the audio end") {
    const hydra::test::AudioTailChart c = hydra::test::audio_tail_chart();
    PreviewTransport transport([] { return 0.0; });
    transport.load(make_playhead(c.audio_end_ms), c.last_note_ms);
    CHECK(transport.length_ms() == doctest::Approx(6000.0));  // the tail still plays

    const double right_end = hydra::app::scrub_end_ms(c.last_note_ms, transport.length_ms());
    transport.seek_ms(right_end);
    CHECK(transport.now_ms() == doctest::Approx(1000.0));

    // With no usable song length the edge stays the transport's length.
    CHECK(hydra::app::scrub_end_ms(0.0, transport.length_ms()) == doctest::Approx(6000.0));
}

// The transport's audio end comes from audio_end_chart_ms, the owner the load
// job uses, not from a second copy. The owner is asked about the very kind of
// audio the transport holds, a Playhead: 2000 ms of audio with chart time 0 at
// 500 ms ends at 1500 ms both ways.
TEST_CASE("single-owner: the transport's audio end is audio_end_chart_ms") {
    const double offset_ms = 500.0;
    const Playhead same_audio(make_ramp(96000));  // 2000 ms
    const std::optional<double> owner = hydra::ui::audio_end_chart_ms(same_audio, offset_ms);
    REQUIRE(owner.has_value());
    CHECK(*owner == doctest::Approx(1500.0));

    PreviewTransport transport([] { return 0.0; });
    transport.load(std::make_unique<Playhead>(make_ramp(96000)), 0.0, offset_ms);
    CHECK(transport.length_ms() == *owner);
}

TEST_CASE("load with no audio offset behaves exactly as before") {
    PreviewTransport transport([] { return 0.0; });
    auto* playhead = new Playhead(make_ramp(48000));
    transport.load(std::unique_ptr<Playhead>(playhead), 0.0);
    transport.seek_ms(250.0);
    CHECK(playhead->position_ms() == doctest::Approx(250.0));
}

// A negative chart offset is silence in front of the stems: the load job
// hands StreamMix round(-offset_ms * 48) frames of front pad (this was the
// pad_front_ms case).
TEST_CASE("StreamMix's front pad adds silence before the first sample") {
    std::vector<std::unique_ptr<hydra::audio::StemReader>> stems;
    stems.push_back(std::make_unique<RampReader>(make_ramp(4)));
    hydra::audio::StreamMix mix(std::move(stems), 48000, 2, 48);  // 1 ms at 48 kHz
    REQUIRE(mix.length_frames() == 52);
    std::vector<float> a(52 * 2, -1.0f);
    REQUIRE(mix.read(a.data(), 52) == 52);
    CHECK(a[0] == 0.0f);
    CHECK(a[47 * 2 + 1] == 0.0f);
    CHECK(a[48 * 2] == 0.0f);       // the ramp's frame 0, L = 0
    CHECK(a[49 * 2] == 1.0f);       // the ramp's frame 1, L = 1
    CHECK(a[49 * 2 + 1] == 1.5f);
}
