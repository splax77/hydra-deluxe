// Tests for ui/preview_transport: the Preview's Transport — play, pause and
// seek together with the master clock, and the audio playhead that follows it.
// The monotonic source is a fake, so the timing is exact; no device is opened,
// and read_frames() is exactly what a device callback would pull.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "app/preview_source.h"  // audio_ms_of_chart_ms, chart_ms_of_audio_ms
#include "app/preview_view.h"    // scrub_thumb_ms
#include "audio/decode.h"
#include "audio/frames.h"
#include "audio/player.h"
#include "audio/stretch_mix.h"
#include "ui/preview_transport.h"
#include "audio_util.h"

using hydra::audio::DecodedAudio;
using hydra::audio::Playhead;
using hydra::audio::StretchSource;
using hydra::ui::PreviewTransport;
using testaudio::ramp_source;

namespace {

// A playhead `ms` milliseconds long at 48 kHz stereo.
std::unique_ptr<Playhead> make_playhead(double ms) {
    return std::make_unique<Playhead>(ramp_source(static_cast<int>(ms * 48.0)));
}

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
    auto* playhead = new Playhead(ramp_source(48000));  // 1000 ms
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

    auto* playhead = new Playhead(ramp_source(48000));  // 1000 ms
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
// the audio runs out while the clock plays on. A jump back that moved the
// playhead without restarting it would scroll the highway in silence with the
// play button lit (finding 73). The clock is the master, so a seek while it
// plays sets the playhead playing again.
TEST_CASE("a seek while playing brings the audio back after it ran out") {
    double t = 0.0;
    PreviewTransport transport([&] { return t; });
    auto* playhead = new Playhead(ramp_source(48000));  // 1000 ms
    transport.load(std::unique_ptr<Playhead>(playhead));
    transport.set_song_length_ms(2000.0);
    transport.play();

    std::vector<float> out(48004 * 2, -1.0f);
    CHECK(transport.read_frames(out.data(), 48004) == 48000);  // the audio runs out
    REQUIRE_FALSE(playhead->playing());
    REQUIRE(transport.playing());  // the notes go on

    const double seek_to_ms = 500.0;
    transport.seek_ms(seek_to_ms);
    CHECK(playhead->playing());
    CHECK(playhead->position_ms() == doctest::Approx(seek_to_ms));
    CHECK(transport.read_frames(out.data(), 4) == 4);
    // make_ramp's frame i holds L = i and R = i + 0.5, so the first frame
    // served is the one at the seek's time, at the audio's own rate.
    const double seek_frame = seek_to_ms * transport.sample_rate() / 1000.0;
    CHECK(out[0] == doctest::Approx(seek_frame));
    CHECK(out[1] == doctest::Approx(seek_frame + 0.5));
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
    transport.load(std::make_unique<Playhead>(ramp_source(5)), 0.0);

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

    auto* playhead = new Playhead(ramp_source(100));
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

    auto* playhead = new Playhead(ramp_source(96000));  // 2000 ms
    transport.load(std::unique_ptr<Playhead>(playhead), /*audio_offset_ms=*/500.0);

    transport.seek_ms(250.0);
    CHECK(transport.now_ms() == doctest::Approx(250.0));
    CHECK(playhead->position_ms() == doctest::Approx(750.0));

    transport.play();
    CHECK(playhead->position_ms() == doctest::Approx(750.0));
}

// At another song speed the playhead plays the stretched mix, which counts
// played frames (audio::StretchSource). Every transport seek still lands on
// the sync rule's file time: at 200% chart time t is file time 2t plus the
// delay (app::audio_ms_of_chart_ms, TM-05 on docs/clone-hero/03-timing.md),
// and that file time read back through the rule is t again. load, seek_ms and
// play each seek the playhead, so each is checked.
TEST_CASE("at 200% the transport seeks the stretched audio to file time 2t plus the delay") {
    constexpr double kRatio = 2.0;
    constexpr double kDelayMs = 500.0;
    constexpr int kRate = 48000;
    constexpr int kChannels = 2;
    // Chart 1500 ms is where play() starts below. Its file time holds an
    // impulse kAfter frames on, so the sound itself shows where play landed.
    constexpr double kPlayChartMs = 1500.0;
    constexpr int64_t kAfter = 100;
    const int64_t play_file_frame =
        hydra::audio::frames_of_ms(kRatio * kPlayChartMs + kDelayMs, kRate);

    DecodedAudio file;  // 10 s of stereo silence with that one impulse
    file.channels = kChannels;
    file.sample_rate = kRate;
    file.samples.assign(static_cast<std::size_t>(10 * kRate) * kChannels, 0.0f);
    for (int c = 0; c < kChannels; ++c)
        file.samples[static_cast<std::size_t>(play_file_frame + kAfter) * kChannels + c] = 1.0f;

    double t = 0.0;
    PreviewTransport transport([&] { return t; });
    auto stretched = std::make_unique<StretchSource>(
        std::make_unique<testaudio::BufferSource>(std::move(file)), kRatio);
    const StretchSource* stretch = stretched.get();
    auto* playhead = new Playhead(std::move(stretched));
    transport.load(std::unique_ptr<Playhead>(playhead), kDelayMs, kRatio);

    // The file time the playhead sits at, through the stretched source's own
    // map from played frames to file frames.
    const auto file_ms = [&] {
        return hydra::audio::ms_of_frames(stretch->file_frame_of(playhead->position_frames()),
                                          kRate);
    };
    // One frame of file time either way: the playhead holds whole frames.
    const double frame_ms = hydra::audio::ms_of_frames(1, kRate);

    // Loaded: chart time 0 is the delay.
    CHECK(std::abs(file_ms() - kDelayMs) <= frame_ms);

    for (const double chart_ms : {1000.0, 2750.0, 0.0, 4000.0}) {
        CAPTURE(chart_ms);
        transport.seek_ms(chart_ms);
        CHECK(std::abs(file_ms() - (kRatio * chart_ms + kDelayMs)) <= frame_ms);
        CHECK(std::abs(hydra::app::chart_ms_of_audio_ms(file_ms(), kDelayMs, kRatio) - chart_ms) <=
              frame_ms);
    }

    // play() seeks to the clock: from 1000 ms, half a second of clock later.
    transport.seek_ms(1000.0);
    transport.play();
    t += 0.5;
    transport.pause();
    REQUIRE(transport.now_ms() == doctest::Approx(kPlayChartMs));
    transport.play();
    CHECK(std::abs(file_ms() - (kRatio * kPlayChartMs + kDelayMs)) <= frame_ms);
    // The first played frames after a seek are copied straight through
    // (stretch_mix.cpp, kProbeFrame, says why), so the impulse kAfter file
    // frames past the seek comes out kAfter played frames in.
    std::vector<float> got(static_cast<std::size_t>(2 * kAfter) * kChannels, 0.0f);
    REQUIRE(transport.read_frames(got.data(), 2 * kAfter) == 2 * kAfter);
    int64_t loudest = -1;
    float peak = 0.0f;
    for (int64_t i = 0; i < 2 * kAfter; ++i) {
        const float v = std::abs(got[static_cast<std::size_t>(i) * kChannels]);
        if (v > peak) {
            peak = v;
            loudest = i;
        }
    }
    CHECK(loudest == kAfter);
}

// At normal speed the transport passes the plain playhead's seeks straight
// through the sync rule, as it always has: the playhead sits at the sync
// rule's file time with no stretch in between.
TEST_CASE("at 100% the transport seeks the plain playhead to the sync rule's file time") {
    constexpr double kDelayMs = 500.0;
    PreviewTransport transport([] { return 0.0; });
    auto* playhead = new Playhead(ramp_source(96000));  // 2000 ms
    transport.load(std::unique_ptr<Playhead>(playhead), kDelayMs, 1.0);
    transport.seek_ms(250.0);
    CHECK(playhead->position_ms() ==
          doctest::Approx(hydra::app::audio_ms_of_chart_ms(250.0, kDelayMs, 1.0)));
}

// A reload at another song speed keeps the place in the song, which is the
// place in the file, not the clock time (PreviewController::open). file_ms
// reads it through the sync rule at the loaded speed, and seek_file_ms puts
// it back at the next load's speed: the same file place, at that speed's own
// clock time, clamped to the new end like any seek.
TEST_CASE("the place in the file survives a reload at another speed") {
    constexpr double kDelayMs = 500.0;
    PreviewTransport transport([] { return 0.0; });
    transport.load(make_playhead(20000.0), kDelayMs, 1.0);
    transport.seek_ms(8000.0);
    const double place = transport.file_ms();
    CHECK(place == doctest::Approx(hydra::app::audio_ms_of_chart_ms(8000.0, kDelayMs, 1.0)));

    // The next load at 200% starts at chart time 0; seek_file_ms goes back.
    transport.load(make_playhead(10000.0), kDelayMs, 2.0);
    transport.set_song_length_ms(9000.0);
    REQUIRE(transport.now_ms() == doctest::Approx(0.0));
    transport.seek_file_ms(place);
    CHECK(transport.now_ms() ==
          doctest::Approx(hydra::app::chart_ms_of_audio_ms(place, kDelayMs, 2.0)));
    CHECK(transport.file_ms() == doctest::Approx(place));
    CHECK_FALSE(transport.playing());  // a seek never starts playback

    // A place past the new end lands on the end, as seek_ms clamps.
    transport.set_song_length_ms(1000.0);
    transport.seek_ms(0.0);
    transport.seek_file_ms(place);
    CHECK(transport.now_ms() == doctest::Approx(transport.length_ms()));
}

// "reload what needs to reload but dont reset your place in the song": new
// notes at another speed re-time the loaded song in place
// (PreviewTransport::set_timing). The playhead and its mix stay, wrapped again
// at the new speed; the place in the file stays, at the new speed's clock
// time; the end is the new length; and playing stays playing.
TEST_CASE("set_timing at another speed keeps the playhead, the mix and the place in the file") {
    constexpr double kDelayMs = 500.0;
    constexpr int kFileFrames = 20 * 48000;  // 20 s
    double t = 0.0;
    PreviewTransport transport([&] { return t; });
    auto* playhead = new Playhead(ramp_source(kFileFrames));
    transport.load(std::unique_ptr<Playhead>(playhead), kDelayMs, 1.0);
    transport.set_song_length_ms(19000.0);
    transport.seek_ms(8000.0);
    const double place = transport.file_ms();

    // Paused, then twice as fast: still paused, at the same file place.
    transport.set_timing(2.0, 9500.0);
    CHECK(transport.speed_ratio() == 2.0);
    CHECK_FALSE(transport.playing());
    CHECK(transport.length_ms() == doctest::Approx(9500.0));
    CHECK(transport.file_ms() == doctest::Approx(place));
    CHECK(transport.now_ms() ==
          doctest::Approx(hydra::app::chart_ms_of_audio_ms(place, kDelayMs, 2.0)));
    // The same playhead now plays the mix stretched to 200%.
    CHECK(transport.has_audio());
    CHECK(playhead->length_frames() ==
          StretchSource(ramp_source(kFileFrames), 2.0).length_frames());
    CHECK(playhead->position_ms() ==
          doctest::Approx(hydra::app::played_ms_of_chart_ms(transport.now_ms(), kDelayMs, 2.0)));

    // Playing, then back to normal speed: it plays on from the same place.
    transport.play();
    t += 1.0;
    const double left = transport.file_ms();
    transport.set_timing(1.0, 19000.0);
    CHECK(transport.speed_ratio() == 1.0);
    CHECK(transport.playing());
    CHECK(playhead->playing());
    CHECK(transport.file_ms() == doctest::Approx(left));
    CHECK(playhead->length_frames() == kFileFrames);
    // The plain mix came back unstretched: the ramp's own frame at the place
    // plays first (make_ramp's frame i holds L = i).
    std::vector<float> out(4 * 2, -1.0f);
    REQUIRE(transport.read_frames(out.data(), 4) == 4);
    CHECK(out[0] == doctest::Approx(static_cast<float>(hydra::audio::frames_of_ms(left, 48000))));

    // At the loaded speed it only moves the end, as set_song_length_ms does.
    transport.set_timing(1.0, 1000.0);
    CHECK(transport.length_ms() == doctest::Approx(1000.0));
    CHECK(transport.now_ms() == doctest::Approx(1000.0));
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
