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

#include "app/preview_view.h"  // scrub_thumb_ms
#include "audio/decode.h"
#include "audio/player.h"
#include "audio/stem_reader.h"
#include "audio/stream_mix.h"
#include "ui/preview_transport.h"
#include "audio_util.h"

using hydra::audio::DecodedAudio;
using hydra::audio::Playhead;
using hydra::ui::PreviewTransport;
using testaudio::make_ramp;

namespace {

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

TEST_CASE("the transport's end is the song's length; unload clears it") {
    PreviewTransport transport([] { return 0.0; });

    transport.load(make_playhead(1000.0));
    CHECK(transport.length_ms() == doctest::Approx(0.0));  // no song length yet
    CHECK(transport.has_audio());
    CHECK(transport.channels() == 2);
    CHECK(transport.sample_rate() == 48000);

    transport.set_song_length_ms(1500.0);
    CHECK(transport.length_ms() == doctest::Approx(1500.0));
    transport.set_song_length_ms(0.0);  // not a length has_song_length accepts
    CHECK(transport.length_ms() == doctest::Approx(0.0));

    transport.set_song_length_ms(1500.0);
    transport.unload();
    CHECK(transport.length_ms() == doctest::Approx(0.0));
    CHECK_FALSE(transport.song_length_ms().has_value());
    CHECK_FALSE(transport.has_audio());
    CHECK(transport.now_ms() == doctest::Approx(0.0));
}

// The Preview swaps in another mode's notes without reloading the audio, and
// hands the transport that mode's song length. A clock left past a shorter
// end comes back to it. The play state stays.
TEST_CASE("set_song_length_ms moves the end and pulls the clock back to it") {
    double t = 0.0;
    PreviewTransport transport([&] { return t; });
    auto* playhead = new Playhead(make_ramp(48000));  // 1000 ms
    transport.load(std::unique_ptr<Playhead>(playhead));
    transport.set_song_length_ms(3000.0);
    REQUIRE(transport.length_ms() == doctest::Approx(3000.0));
    transport.seek_ms(1200.0);
    transport.play();

    // A longer song: the end grows and the clock stays put.
    transport.set_song_length_ms(4000.0);
    CHECK(transport.length_ms() == doctest::Approx(4000.0));
    CHECK(transport.now_ms() == doctest::Approx(1200.0));
    CHECK(transport.playing());

    // A shorter song: the clock is pulled back to its end.
    transport.set_song_length_ms(800.0);
    CHECK(transport.length_ms() == doctest::Approx(800.0));
    CHECK(transport.now_ms() == doctest::Approx(800.0));  // pulled back from 1200
    CHECK(playhead->position_ms() == doctest::Approx(800.0));
    CHECK(transport.has_audio());  // the audio was never dropped

    // Inside the end nothing moves.
    transport.seek_ms(500.0);
    transport.set_song_length_ms(900.0);
    CHECK(transport.now_ms() == doctest::Approx(500.0));
    CHECK(playhead->position_ms() == doctest::Approx(500.0));
}

// Playback ends at the song's length (app::song_length_ms; D255, the user's
// answer 5), even when the audio runs on past it: 5 s of audio, a 4 s song.
TEST_CASE("playback stops at the song's length even when the audio runs longer") {
    double t = 0.0;
    PreviewTransport transport([&] { return t; });
    transport.load(make_playhead(5000.0));
    transport.set_song_length_ms(4000.0);
    CHECK(transport.length_ms() == doctest::Approx(4000.0));

    transport.play();
    t += 4.5;  // past the song's end, inside the audio
    CHECK(transport.tick() == doctest::Approx(4000.0));
    CHECK_FALSE(transport.playing());

    transport.seek_ms(4800.0);  // a drag into the audio past the end
    CHECK(transport.now_ms() == doctest::Approx(4000.0));
}

// With no song length at all, nothing stops the clock: the audio's own end is
// never an input to where playback ends (D255, the user's answer 5).
TEST_CASE("with no song length the clock runs free past the audio's end") {
    double t = 0.0;
    PreviewTransport transport([&] { return t; });
    transport.load(make_playhead(1000.0));
    CHECK(transport.length_ms() == doctest::Approx(0.0));

    transport.play();
    t += 1.5;
    CHECK(transport.tick() == doctest::Approx(1500.0));
    CHECK(transport.playing());
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
    transport.load(make_playhead(1000.0));
    transport.set_song_length_ms(1000.0);
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
    transport.load(std::unique_ptr<Playhead>(playhead));
    transport.set_song_length_ms(2000.0);
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
    transport.load(make_playhead(1000.0));
    transport.set_song_length_ms(1000.0);

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
    transport.load(std::unique_ptr<Playhead>(playhead), /*audio_offset_ms=*/500.0);

    transport.seek_ms(250.0);
    CHECK(transport.now_ms() == doctest::Approx(250.0));
    CHECK(playhead->position_ms() == doctest::Approx(750.0));

    transport.play();
    CHECK(playhead->position_ms() == doctest::Approx(750.0));
}

// A song length past the audio's end: playback runs on to it in silence, so
// a drag to the scrubber's right end lands there and the thumb stays at the
// edge.
TEST_CASE("scrubber: a song length past the audio ends playback there") {
    PreviewTransport transport([] { return 0.0; });
    transport.load(make_playhead(5000.0));
    transport.set_song_length_ms(9000.0);
    CHECK(transport.length_ms() == doctest::Approx(9000.0));

    transport.seek_ms(transport.length_ms());
    CHECK(transport.now_ms() == doctest::Approx(9000.0));
    CHECK(hydra::app::scrub_thumb_ms(transport.now_ms(), transport.length_ms()) ==
          doctest::Approx(9000.0));
}

// No audio at all: the song's length is still the end.
TEST_CASE("scrubber: with no audio the end is still the song's length") {
    PreviewTransport transport([] { return 0.0; });
    transport.set_song_length_ms(4000.0);
    CHECK(transport.length_ms() == doctest::Approx(4000.0));
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
