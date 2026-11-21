// Tests for audio/decode: classifying and decoding a chart stem's bytes to
// float PCM. The format sniff is exercised with both hand-built magic bytes and
// the real public-domain sine fixtures under testdata/audio (a 220 Hz tone
// encoded as OGG Vorbis, Ogg-Opus, and MP3), because the one hard case — telling
// Vorbis and Opus apart when both start with "OggS" — only shows up on real
// container headers.

#include "doctest.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "app/preview_source.h"
#include "audio/decode.h"
#include "audio/stem_reader.h"
#include "audio_util.h"
#include "midi_util.h"  // testmidi::concat

using namespace hydra;
using namespace hydra::audio;
using testaudio::estimate_freq_hz;
using testaudio::fixture_path;
using testaudio::id3_tag;
using testaudio::read_fixture;

namespace {

std::vector<uint8_t> bytes(std::initializer_list<int> vals) {
    std::vector<uint8_t> out;
    out.reserve(vals.size());
    for (int v : vals) out.push_back(static_cast<uint8_t>(v));
    return out;
}

float peak_abs(const DecodedAudio& a) {
    float m = 0.0f;
    for (float s : a.samples) {
        float v = s < 0 ? -s : s;
        if (v > m) m = v;
    }
    return m;
}

}  // namespace

TEST_CASE("sniff_format classifies audio containers by their magic bytes") {
    // Hand-built magics for the simple containers.
    CHECK(sniff_format(bytes({'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V',
                              'E'})) == AudioFormat::Wav);
    CHECK(sniff_format(bytes({'f', 'L', 'a', 'C'})) == AudioFormat::Flac);
    std::vector<uint8_t> cut_tag = id3_tag(0);
    cut_tag.resize(6);  // an ID3 header cut short
    CHECK(sniff_format(cut_tag) == AudioFormat::Mp3);
    CHECK(sniff_format(bytes({0xFF, 0xFB, 0x90, 0x00})) ==
          AudioFormat::Mp3);  // raw MP3 frame sync

    // The real discrimination: both Ogg streams start "OggS"; only the codec tag
    // in the first page separates Vorbis from Opus.
    CHECK(sniff_format(read_fixture("sine220.ogg")) == AudioFormat::OggVorbis);
    CHECK(sniff_format(read_fixture("sine220.opus")) == AudioFormat::OggOpus);
    CHECK(sniff_format(read_fixture("sine220.mp3")) == AudioFormat::Mp3);

    // Too short or unrecognized -> Unknown, never a wrong guess.
    CHECK(sniff_format(std::vector<uint8_t>{}) == AudioFormat::Unknown);
    CHECK(sniff_format(bytes({0x89, 'P', 'N', 'G'})) == AudioFormat::Unknown);
}

TEST_CASE("sniff_format: a FLAC behind an ID3 tag is Flac, a tagged MP3 stays Mp3") {
    using testmidi::concat;
    const std::vector<uint8_t> flac = read_fixture("sine220.flac");
    const std::vector<uint8_t> mp3 = read_fixture("sine220.mp3");
    const std::vector<uint8_t> tag = id3_tag(0);
    const std::vector<uint8_t> tag_footer = id3_tag(0x10);

    CHECK(sniff_format(concat({tag, flac})) == AudioFormat::Flac);
    CHECK(sniff_format(concat({tag_footer, flac})) == AudioFormat::Flac);
    CHECK(sniff_format(concat({tag, tag, flac})) == AudioFormat::Flac);
    CHECK(sniff_format(concat({tag, mp3})) == AudioFormat::Mp3);
    CHECK(sniff_format(tag) == AudioFormat::Mp3);
}

TEST_CASE("audio_sniff: the ID3v2 tag length counts the header, the size and the footer") {
    using testmidi::concat;
    const std::vector<uint8_t> tag = id3_tag(0);
    const std::vector<uint8_t> tag_footer = id3_tag(0x10);
    const std::vector<uint8_t> flac = read_fixture("sine220.flac");

    // The length is where the audio behind the tag starts. With a real FLAC
    // after each tag, the length must land on the FLAC's first byte: it covers
    // every byte of the tag (its header, the size its header gives, and the
    // footer when flag 0x10 is set) and none of the audio.
    for (const std::vector<uint8_t>* t : {&tag, &tag_footer}) {
        const std::vector<uint8_t> tagged = concat({*t, flac});
        const std::size_t length = id3v2_tag_length(tagged.data(), tagged.size());
        CHECK(length == t->size());
        REQUIRE(length <= tagged.size() - flac.size());
        CHECK(std::equal(flac.begin(), flac.end(), tagged.begin() + length));
    }
    // The footer is counted: the same tag with the footer flag is longer.
    CHECK(id3v2_tag_length(tag_footer.data(), tag_footer.size()) >
          id3v2_tag_length(tag.data(), tag.size()));

    CHECK(id3v2_tag_length(flac.data(), flac.size()) == 0);
    CHECK(id3v2_tag_length(tag.data(), 9) == 0);
}

TEST_CASE("decode_audio: PCM16 WAV decodes to matching float samples") {
    const uint32_t rate = 8000;
    const std::vector<int16_t> pcm = {0, 16384, -16384, 32767, -32768};
    DecodedAudio out = decode_audio(testaudio::pcm16_wav(1, rate, pcm));

    CHECK(out.channels == 1);
    CHECK(out.sample_rate == static_cast<int>(rate));
    REQUIRE(out.frames() == static_cast<int64_t>(pcm.size()));
    // int16 -> float within a sample's quantization; 32767 lands just under 1.0.
    const float expect[] = {0.0f, 0.5f, -0.5f, 1.0f, -1.0f};
    for (size_t i = 0; i < pcm.size(); ++i)
        CHECK(out.samples[i] == doctest::Approx(expect[i]).epsilon(0.001));
}

TEST_CASE("decode_audio: MP3 fixture decodes to the 220 Hz sine") {
    DecodedAudio out = decode_audio(read_fixture("sine220.mp3"));

    CHECK(out.channels == 2);
    CHECK(out.sample_rate == 44100);
    REQUIRE(out.frames() > out.sample_rate / 10);  // at least ~0.1 s
    CHECK(peak_abs(out) <= 1.0001f);
    CHECK(estimate_freq_hz(out, 0) == doctest::Approx(220.0).epsilon(0.01));
}

TEST_CASE("decode_audio: OGG Vorbis fixture decodes to the 220 Hz sine") {
    DecodedAudio out = decode_audio(read_fixture("sine220.ogg"));

    CHECK(out.channels == 1);
    CHECK(out.sample_rate == 44100);
    REQUIRE(out.frames() > out.sample_rate / 10);
    CHECK(peak_abs(out) <= 1.0001f);
    CHECK(estimate_freq_hz(out, 0) == doctest::Approx(220.0).epsilon(0.01));
}

TEST_CASE("decode_stem: a file-path stem and a bytes stem decode identically") {
    hydra::app::PreviewAudioStem file_stem;
    file_stem.label = "song";
    file_stem.path = fixture_path("sine220.ogg");

    hydra::app::PreviewAudioStem mem_stem;
    mem_stem.label = "song";
    mem_stem.bytes = read_fixture("sine220.ogg");

    REQUIRE(file_stem.from_file());
    REQUIRE_FALSE(mem_stem.from_file());

    DecodedAudio a = decode_stem(file_stem);
    DecodedAudio b = decode_stem(mem_stem);

    CHECK(a.channels == 1);
    CHECK(b.channels == 1);
    CHECK(a.sample_rate == 44100);
    CHECK(b.sample_rate == 44100);
    REQUIRE(a.samples.size() == b.samples.size());
    CHECK(a.samples == b.samples);  // byte-for-byte the same PCM
    CHECK(estimate_freq_hz(a, 0) == doctest::Approx(220.0).epsilon(0.01));
}

TEST_CASE("decode_audio: Ogg-Opus fixture decodes to the 220 Hz sine at 48 kHz") {
    DecodedAudio out = decode_audio(read_fixture("sine220.opus"));

    CHECK(out.channels == 1);
    CHECK(out.sample_rate == 48000);  // Opus always decodes at 48 kHz
    REQUIRE(out.frames() > out.sample_rate / 10);
    CHECK(peak_abs(out) <= 1.0001f);
    CHECK(estimate_freq_hz(out, 0) == doctest::Approx(220.0).epsilon(0.01));
}

// The Opus reader reuses one decode buffer across reads. Reading the stream
// one frame per call, so every call meets the reused buffer mid-packet, must
// give the same samples bit for bit as decode_audio's read of the whole stem.
TEST_CASE("decode_audio: Ogg-Opus read one frame at a time equals the whole decode") {
    const std::vector<uint8_t> bytes = read_fixture("sine220.opus");
    const DecodedAudio whole = decode_audio(bytes);
    REQUIRE(whole.frames() > 0);

    std::unique_ptr<StemReader> reader = open_stem_reader(StemBytes{bytes, nullptr});
    REQUIRE(reader->channels() == whole.channels);
    std::vector<float> stepped;
    std::vector<float> frame(static_cast<std::size_t>(reader->channels()));
    while (reader->read(frame.data(), 1) == 1)
        stepped.insert(stepped.end(), frame.begin(), frame.end());

    CHECK_FALSE(reader->failed());
    CHECK(static_cast<int64_t>(stepped.size()) == whole.frames() * whole.channels);
    CHECK(stepped == whole.samples);

    // The Preview plays the encoder's padding at the end (OpusReaderOptions):
    // the whole decode runs past the stream's last granule, where a trimmed
    // reader stops.
    hydra::audio::detail::OpusReaderOptions trim;
    trim.trim_end = true;
    const std::unique_ptr<StemReader> trimmed =
        hydra::audio::detail::open_opus_reader(StemBytes{bytes, nullptr}, nullptr, trim);
    CHECK(trimmed->length_frames() < whole.frames());
}