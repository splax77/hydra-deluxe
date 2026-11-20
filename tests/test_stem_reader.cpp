// Tests for audio/stem_reader: the seekable per-format decoders behind the
// Preview's audio. The reference for every check is a test-only whole-file
// decoder (old_full_decode below), so the readers are pinned to exactly what
// decoding the whole file gives. Fixtures: the public-domain 220 Hz sine under
// testdata/audio in each format (the WAV and FLAC ones were made with ffmpeg's
// sine generator, 1 s at 44.1 kHz; the FLAC is stereo).

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <ogg/ogg.h>
#include <opus.h>

#include "miniaudio.h"
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

#include "app/preview_source.h"
#include "audio/decode.h"
#include "audio/stem_reader.h"
#include "core/little_endian.h"
#include "core/winstr.h"

#include "audio_util.h"

using namespace hydra;
using namespace hydra::audio;

using testaudio::fixture_path;
using testaudio::id3_tag;
using testaudio::read_fixture;

namespace {

const char* const kFixtures[] = {"sine220.wav", "sine220.mp3", "sine220.flac",
                                 "sine220.ogg", "sine220.opus"};

// ---- the old whole-file decoders, copied verbatim as the reference --------

DecodedAudio old_decode_with_miniaudio(const uint8_t* data, std::size_t size) {
    ma_decoder_config cfg = ma_decoder_config_init(ma_format_f32, 0, 0);
    ma_decoder dec;
    if (ma_decoder_init_memory(data, size, &cfg, &dec) != MA_SUCCESS)
        throw std::runtime_error("old miniaudio open failed");
    DecodedAudio out;
    out.channels = static_cast<int>(dec.outputChannels);
    out.sample_rate = static_cast<int>(dec.outputSampleRate);
    const ma_uint64 kChunkFrames = 4096;
    for (;;) {
        std::size_t base = out.samples.size();
        out.samples.resize(base + static_cast<std::size_t>(kChunkFrames) * out.channels);
        ma_uint64 read = 0;
        ma_result r = ma_decoder_read_pcm_frames(&dec, out.samples.data() + base, kChunkFrames,
                                                 &read);
        out.samples.resize(base + static_cast<std::size_t>(read) * out.channels);
        if (read == 0) break;
        if (r != MA_SUCCESS && r != MA_AT_END) {
            ma_decoder_uninit(&dec);
            throw std::runtime_error("old miniaudio read failed");
        }
        if (r == MA_AT_END) break;
    }
    ma_decoder_uninit(&dec);
    return out;
}

DecodedAudio old_decode_ogg_vorbis(const uint8_t* data, std::size_t size) {
    int channels = 0, rate = 0;
    short* pcm = nullptr;
    int frames = stb_vorbis_decode_memory(data, static_cast<int>(size), &channels, &rate, &pcm);
    if (frames < 0 || pcm == nullptr) throw std::runtime_error("old stb_vorbis failed");
    DecodedAudio out;
    out.channels = channels;
    out.sample_rate = rate;
    out.samples.resize(static_cast<std::size_t>(frames) * channels);
    for (std::size_t i = 0; i < out.samples.size(); ++i) out.samples[i] = pcm[i] / 32768.0f;
    std::free(pcm);
    return out;
}

DecodedAudio old_decode_ogg_opus(const uint8_t* data, std::size_t size) {
    ogg_sync_state oy;
    ogg_sync_init(&oy);
    ogg_stream_state os;
    bool stream_ready = false;
    OpusDecoder* dec = nullptr;
    DecodedAudio out;
    out.sample_rate = 48000;
    char* buf = ogg_sync_buffer(&oy, static_cast<long>(size));
    std::memcpy(buf, data, size);
    ogg_sync_wrote(&oy, static_cast<long>(size));
    int channels = 0;
    long skip_remaining = 0;
    long packet_index = 0;
    const int kMaxFrame = 5760;
    std::vector<float> pcm;
    ogg_page og;
    while (ogg_sync_pageout(&oy, &og) == 1) {
        if (!stream_ready) {
            ogg_stream_init(&os, ogg_page_serialno(&og));
            stream_ready = true;
        }
        ogg_stream_pagein(&os, &og);
        ogg_packet op;
        while (ogg_stream_packetout(&os, &op) == 1) {
            if (packet_index == 0) {
                channels = op.packet[9];
                skip_remaining = core::read_le_u16(op.packet + 10);
                int err = 0;
                dec = opus_decoder_create(48000, channels, &err);
                out.channels = channels;
                pcm.resize(static_cast<std::size_t>(kMaxFrame) * channels);
            } else if (packet_index > 1) {
                int n = opus_decode_float(dec, op.packet, static_cast<opus_int32>(op.bytes),
                                          pcm.data(), kMaxFrame, 0);
                REQUIRE(n >= 0);
                int start = 0;
                if (skip_remaining > 0) {
                    int drop = static_cast<int>(std::min<long>(skip_remaining, n));
                    start = drop;
                    skip_remaining -= drop;
                }
                out.samples.insert(out.samples.end(),
                                   pcm.begin() + static_cast<std::size_t>(start) * channels,
                                   pcm.begin() + static_cast<std::size_t>(n) * channels);
            }
            ++packet_index;
        }
    }
    if (dec) opus_decoder_destroy(dec);
    if (stream_ready) ogg_stream_clear(&os);
    ogg_sync_clear(&oy);
    return out;
}

DecodedAudio old_full_decode(const std::vector<uint8_t>& b) {
    switch (sniff_format(b)) {
        case AudioFormat::OggVorbis: return old_decode_ogg_vorbis(b.data(), b.size());
        case AudioFormat::OggOpus: return old_decode_ogg_opus(b.data(), b.size());
        default: return old_decode_with_miniaudio(b.data(), b.size());
    }
}

// ---- helpers ---------------------------------------------------------------

std::vector<float> read_to_end(StemReader& r, int64_t chunk = 997) {
    std::vector<float> got;
    std::vector<float> buf(static_cast<std::size_t>(chunk) * r.channels());
    while (int64_t k = r.read(buf.data(), chunk))
        got.insert(got.end(), buf.begin(), buf.begin() + k * r.channels());
    return got;
}

// Seeks to `frame` and reads up to `frames` frames from there (fewer at the
// end of the stem).
std::vector<float> read_after_seek(StemReader& r, int64_t frame, int64_t frames) {
    r.seek(frame);
    std::vector<float> got;
    std::vector<float> buf(static_cast<std::size_t>(frames) * r.channels());
    int64_t n = 0;
    while (n < frames) {
        const int64_t k = r.read(buf.data(), frames - n);
        if (k == 0) break;
        got.insert(got.end(), buf.begin(), buf.begin() + k * r.channels());
        n += k;
    }
    return got;
}

// Seeks `r` to `frame`, reads `frames` frames, and checks they are the
// straight decode's frames from `frame` on, sample for sample, with no
// warm-up left out. Each reader settles its decoder before the target
// itself: ma_decoder's WAV and FLAC seeks are exact, each MP3 seek point
// decodes two whole frames before the target (both in ma_reader.cpp), the
// Opus reader decodes a pre-roll first (kPreRoll, opus_reader.cpp),
// and stb_vorbis_seek decodes the packet before the target's packet for its
// overlap (stb_vorbis_seek_frame in third_party/stb/stb_vorbis.c).
void check_seek_lands(StemReader& r, const DecodedAudio& full, int64_t frame, int64_t frames) {
    CAPTURE(frame);
    const std::vector<float> got = read_after_seek(r, frame, frames);
    // The straight decode's frames from `frame`: `frames` of them, or as many
    // as are left before the end.
    const int64_t count = (std::min)(frames, full.frames() - frame);
    const auto from = full.samples.begin() + static_cast<std::ptrdiff_t>(frame) * full.channels;
    const std::vector<float> want(from, from + static_cast<std::ptrdiff_t>(count) * full.channels);
    CHECK(got == want);
}

// One packet of an Ogg stream and the index of the stream's page that
// completed it.
struct PagedPacket {
    std::vector<unsigned char> bytes;
    int page = 0;
};

// Every Opus packet of the file's first stream, in order (headers included),
// each with the page that completed it.
std::vector<PagedPacket> opus_paged_packets(const std::vector<uint8_t>& b) {
    std::vector<PagedPacket> out;
    ogg_sync_state oy;
    ogg_sync_init(&oy);
    char* buf = ogg_sync_buffer(&oy, static_cast<long>(b.size()));
    std::memcpy(buf, b.data(), b.size());
    ogg_sync_wrote(&oy, static_cast<long>(b.size()));
    ogg_stream_state os;
    bool ready = false;
    int page = 0;
    ogg_page og;
    while (ogg_sync_pageout(&oy, &og) == 1) {
        if (!ready) {
            ogg_stream_init(&os, ogg_page_serialno(&og));
            ready = true;
        }
        if (ogg_stream_pagein(&os, &og) != 0) continue;  // a later link's page
        ogg_packet op;
        while (ogg_stream_packetout(&os, &op) == 1) {
            PagedPacket p;
            p.bytes.assign(op.packet, op.packet + op.bytes);
            p.page = page;
            out.push_back(std::move(p));
        }
        ++page;
    }
    if (ready) ogg_stream_clear(&os);
    ogg_sync_clear(&oy);
    return out;
}

// Every Opus packet of the file's first stream, in order (headers included).
std::vector<std::vector<unsigned char>> opus_packets(const std::vector<uint8_t>& b) {
    std::vector<std::vector<unsigned char>> out;
    for (PagedPacket& p : opus_paged_packets(b)) out.push_back(std::move(p.bytes));
    return out;
}

// The samples one Opus audio packet decodes to, from its TOC byte.
int64_t opus_samples(const std::vector<unsigned char>& packet) {
    return opus_packet_get_nb_samples(packet.data(), static_cast<opus_int32>(packet.size()),
                                      48000);
}

// The little-endian 16-bit field at bytes i and i + 1 of an Opus header
// packet; at() fails the test on a header too short to hold it.
uint16_t opus_head_u16(const std::vector<unsigned char>& head, std::size_t i) {
    const uint8_t two[2] = {head.at(i), head.at(i + 1)};
    return core::read_le_u16(two);
}

// The pre-skip an OpusHead packet states (bytes 10-11).
int64_t opus_preskip(const std::vector<unsigned char>& head) {
    return opus_head_u16(head, 10);
}

// Writes `q78` into an OpusHead packet's output gain field (bytes 16-17,
// a signed Q7.8 dB value).
void set_opus_head_gain(std::vector<unsigned char>& head, int16_t q78) {
    const uint16_t u = static_cast<uint16_t>(q78);
    head.at(16) = static_cast<unsigned char>(u & 0xFF);
    head.at(17) = static_cast<unsigned char>(u >> 8);
}

// The linear output gain the OpusHead of `file`'s first stream asks for: its
// Q7.8 dB field as 10^(dB / 20), in float, as OpusReader::start_link
// (opus_reader.cpp) computes it. That owner is private to the reader.
float opus_head_gain(const std::vector<uint8_t>& file) {
    const std::vector<std::vector<unsigned char>> pk = opus_packets(file);
    REQUIRE(!pk.empty());
    const int16_t q78 = static_cast<int16_t>(opus_head_u16(pk[0], 16));
    return q78 == 0 ? 1.0f : static_cast<float>(std::pow(10.0, q78 / (20.0 * 256.0)));
}

// Every sample of `x` times `gain`, the multiply the reader applies.
std::vector<float> times_gain(const std::vector<float>& x, float gain) {
    std::vector<float> out(x.size());
    std::transform(x.begin(), x.end(), out.begin(), [gain](float s) { return s * gain; });
    return out;
}

// An Opus audio packet of the file's first stream that its page holds whole
// and not first, so the packet starts strictly inside the page: its first
// stem frame (after the pre-skip) and its length in frames.
struct PacketSpan {
    int64_t start = 0;
    int64_t frames = 0;
};

std::vector<PacketSpan> opus_mid_page_packets(const std::vector<uint8_t>& b) {
    const std::vector<PagedPacket> pk = opus_paged_packets(b);
    REQUIRE(pk.size() > 2);
    const int64_t preskip = opus_preskip(pk[0].bytes);
    std::vector<PacketSpan> out;
    int64_t g = 0;  // link samples before packet i
    for (std::size_t i = 2; i < pk.size(); ++i) {
        const int64_t n = opus_samples(pk[i].bytes);
        REQUIRE(n > 0);
        if (pk[i].page == pk[i - 1].page && g >= preskip) {
            PacketSpan s;
            s.start = g - preskip;
            s.frames = n;
            out.push_back(s);
        }
        g += n;
    }
    return out;
}

// Re-pages Opus packets as one Ogg stream with the given serial, the headers
// on their own pages and every audio packet's granule set from the sample
// counts (libogg writes fresh CRCs). Appends to `out`.
void page_opus(const std::vector<std::vector<unsigned char>>& pk, int serial,
               std::vector<uint8_t>& out) {
    ogg_stream_state os;
    ogg_stream_init(&os, serial);
    int64_t g = 0;
    auto emit = [&](const ogg_page& p) {
        out.insert(out.end(), p.header, p.header + p.header_len);
        out.insert(out.end(), p.body, p.body + p.body_len);
    };
    ogg_page og;
    for (std::size_t i = 0; i < pk.size(); ++i) {
        ogg_packet op{};
        op.packet = const_cast<unsigned char*>(pk[i].data());
        op.bytes = static_cast<long>(pk[i].size());
        op.b_o_s = i == 0;
        op.e_o_s = i + 1 == pk.size();
        op.packetno = static_cast<ogg_int64_t>(i);
        if (i >= 2) g += opus_samples(pk[i]);
        op.granulepos = g;
        ogg_stream_packetin(&os, &op);
        if (i < 2)
            while (ogg_stream_flush(&os, &og)) emit(og);
        else
            while (ogg_stream_pageout(&os, &og)) emit(og);
    }
    while (ogg_stream_flush(&os, &og)) emit(og);
    ogg_stream_clear(&os);
}

}  // namespace

TEST_CASE("StemReader: straight-through read equals the full decode") {
    for (const char* name : kFixtures) {
        CAPTURE(std::string(name));
        std::vector<uint8_t> bytes = read_fixture(name);
        DecodedAudio full = old_full_decode(bytes);
        REQUIRE(full.frames() > 0);

        auto r = open_stem_reader(StemBytes{bytes, nullptr});
        CHECK(r->channels() == full.channels);
        CHECK(r->sample_rate() == full.sample_rate);
        CHECK(r->length_frames() == full.frames());
        std::vector<float> got = read_to_end(*r);
        CHECK(static_cast<int64_t>(got.size()) == full.frames() * full.channels);
        CHECK(got == full.samples);
        CHECK_FALSE(r->failed());

        // The same through a memory-mapped file.
        app::PreviewAudioStem stem;
        stem.path = fixture_path(name);
        auto m = open_stem_reader(stem);
        CHECK(m->length_frames() == full.frames());
        CHECK(read_to_end(*m, 4096) == full.samples);

        // decode_audio is "open a reader, read it all".
        CHECK(decode_audio(bytes).samples == full.samples);
    }
}

TEST_CASE("StemReader: a seek lands on the straight decode's frames") {
    for (const char* name : kFixtures) {
        CAPTURE(std::string(name));
        std::vector<uint8_t> bytes = read_fixture(name);
        DecodedAudio full = old_full_decode(bytes);
        auto r = open_stem_reader(StemBytes{bytes, nullptr});
        const int64_t len = r->length_frames();
        // len / 3 last: a backward seek after reading near the end.
        for (int64_t f : {int64_t{0}, int64_t{1}, len / 2, len - 100, len / 3})
            check_seek_lands(*r, full, f, 4800);
    }
}

TEST_CASE("StemReader: seeking past the end reads nothing; seeking to 0 restarts") {
    for (const char* name : kFixtures) {
        CAPTURE(std::string(name));
        std::vector<uint8_t> bytes = read_fixture(name);
        DecodedAudio full = old_full_decode(bytes);
        auto r = open_stem_reader(StemBytes{bytes, nullptr});
        std::vector<float> buf(4096 * static_cast<std::size_t>(r->channels()));

        r->seek(r->length_frames() + 1000);
        CHECK(r->read(buf.data(), 4096) == 0);
        r->seek(r->length_frames());
        CHECK(r->read(buf.data(), 4096) == 0);

        // Drain, then start over: the replay is the first full read again.
        read_to_end(*r);
        r->seek(0);
        CHECK(read_to_end(*r) == full.samples);
    }
}

TEST_CASE("StemReader: an Opus seek inside a page and a packet lands on the straight decode") {
    // Each target is the middle of an audio packet that its page holds whole
    // and not first (read from the file's own pages), so the seek resumes on
    // an earlier page and throws samples away up to a point inside a packet.
    std::vector<uint8_t> bytes = read_fixture("sine220.opus");
    DecodedAudio full = old_full_decode(bytes);
    auto r = open_stem_reader(StemBytes{bytes, nullptr});
    const std::vector<PacketSpan> spans = opus_mid_page_packets(bytes);
    REQUIRE(spans.size() >= 3);
    for (std::size_t k : {spans.size() / 4, spans.size() / 2, spans.size() - 1}) {
        const PacketSpan& s = spans[k];
        check_seek_lands(*r, full, s.start + s.frames / 2, 2000);
    }
}

TEST_CASE("StemReader: Opus header gain is applied") {
    std::vector<std::vector<unsigned char>> pk = opus_packets(read_fixture("sine220.opus"));
    REQUIRE(pk.size() > 2);
    std::vector<uint8_t> plain;
    page_opus(pk, 7, plain);
    set_opus_head_gain(pk[0], 1541);  // the test's input: +6.02 dB in Q7.8
    std::vector<uint8_t> loud;
    page_opus(pk, 7, loud);
    const float gain = opus_head_gain(loud);
    REQUIRE(gain != 1.0f);

    auto a = open_stem_reader(StemBytes{plain, nullptr});
    auto b = open_stem_reader(StemBytes{loud, nullptr});
    std::vector<float> x = read_to_end(*a);
    std::vector<float> y = read_to_end(*b);
    REQUIRE(x.size() == y.size());
    CHECK(x == old_full_decode(read_fixture("sine220.opus")).samples);  // re-paging is lossless
    // The same packets through the same decoder, times the header's gain.
    CHECK(y == times_gain(x, gain));
}

TEST_CASE("StemReader: a chained Opus file plays as one stream") {
    std::vector<uint8_t> one = read_fixture("sine220.opus");
    DecodedAudio full = old_full_decode(one);
    REQUIRE(opus_head_gain(one) == 1.0f);
    std::vector<std::vector<unsigned char>> pk = opus_packets(one);
    std::vector<uint8_t> chained;
    page_opus(pk, 1, chained);
    // The second link's header asks for its own output gain, so a reader that
    // kept the first link's header for the second would play it unscaled.
    set_opus_head_gain(pk[0], 1541);
    std::vector<uint8_t> second;
    page_opus(pk, 2, second);
    const float gain = opus_head_gain(second);
    REQUIRE(gain != 1.0f);
    chained.insert(chained.end(), second.begin(), second.end());

    // What the chain plays: the single file, then the single file times the
    // second link's gain. Each link starts from a fresh decoder and drops its
    // own pre-skip.
    DecodedAudio want;
    want.channels = full.channels;
    want.sample_rate = full.sample_rate;
    want.samples = full.samples;
    const std::vector<float> louder = times_gain(full.samples, gain);
    want.samples.insert(want.samples.end(), louder.begin(), louder.end());

    auto r = open_stem_reader(StemBytes{chained, nullptr});
    CHECK(r->length_frames() == want.frames());
    CHECK(read_to_end(*r) == want.samples);

    // A seek into the second link lands in it.
    check_seek_lands(*r, want, full.frames() + full.frames() / 2, 2000);
}

TEST_CASE("StemReader: Opus open reports progress and can be cancelled") {
    std::vector<uint8_t> bytes = read_fixture("sine220.opus");
    int calls = 0;
    CHECK_THROWS_AS(open_stem_reader(StemBytes{bytes, nullptr},
                                     [&](uint64_t, uint64_t) {
                                         ++calls;
                                         return false;
                                     }),
                    OpenCancelled);
    CHECK(calls == 1);

    uint64_t last_done = 0, last_total = 0;
    auto r = open_stem_reader(StemBytes{bytes, nullptr}, [&](uint64_t done, uint64_t total) {
        last_done = done;
        last_total = total;
        return true;
    });
    CHECK(last_total == bytes.size());
    CHECK(last_done == bytes.size());
}

TEST_CASE("StemReader: unrecognized bytes and a missing file throw their error kinds") {
    // The kind is the contract (stem_reader.h): plain_error picks the
    // player's sentence from it alone.
    std::vector<uint8_t> junk = {0x89, 'P', 'N', 'G', 0, 0, 0, 0};
    try {
        open_stem_reader(StemBytes{junk, nullptr});
        FAIL("expected a throw");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::AudioDecode);
    }
    app::PreviewAudioStem missing;
    missing.path = fixture_path("no_such_file.opus");
    try {
        open_stem_reader(missing);
        FAIL("expected a throw");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::SongFileMissing);
    }
}

namespace {

// Seeks to every `stride`-th frame, in an order that jumps from one end to
// the other (every seek a long jump, half of them backward), and checks each
// lands on the straight decode (check_seek_lands).
void seek_sweep(StemReader& r, const DecodedAudio& full, int64_t stride) {
    std::vector<int64_t> targets;
    for (int64_t f = 0; f < r.length_frames(); f += stride) targets.push_back(f);
    for (std::size_t a = 0, b = targets.size(); a < b;) {
        check_seek_lands(r, full, targets[a++], 4800);
        if (a < b) check_seek_lands(r, full, targets[--b], 4800);
    }
}

// Byte size of the MPEG-1 Layer III frame whose header starts at `h`.
std::size_t mp3_l3_frame_size(const uint8_t* h) {
    static const int kbps[16] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
    static const int hz[4] = {44100, 48000, 32000, 0};
    return static_cast<std::size_t>(144000 * kbps[h[2] >> 4] / hz[(h[2] >> 2) & 3] + ((h[2] >> 1) & 1));
}

// The MPEG audio frames of a stream that holds nothing else, walked by their
// headers from byte 0 to the end.
int64_t mp3_frame_count(const std::vector<uint8_t>& b) {
    int64_t n = 0;
    std::size_t pos = 0;
    while (pos < b.size()) {
        REQUIRE(pos + 4 <= b.size());
        REQUIRE(b[pos] == 0xFF);
        const std::size_t size = mp3_l3_frame_size(&b[pos]);
        REQUIRE(size > 0);
        pos += size;
        ++n;
    }
    REQUIRE(pos == b.size());
    return n;
}

// The fixture's audio frames with no ID3 or Xing tag, 24 times over. With no
// tag dr_mp3 counts every frame and skips no encoder delay, and the stream
// spans many of the reader's seek points (about one each half second,
// Mp3Reader::build_seek_points in ma_reader.cpp).
std::vector<uint8_t> long_untagged_mp3() {
    std::vector<uint8_t> b = read_fixture("sine220.mp3");
    std::size_t pos = id3v2_tag_length(b.data(), b.size());
    REQUIRE(pos + 4 < b.size());
    REQUIRE(b[pos] == 0xFF);
    pos += mp3_l3_frame_size(&b[pos]);  // drop the Xing/Info frame
    std::vector<uint8_t> out;
    for (int i = 0; i < 24; ++i) out.insert(out.end(), b.begin() + static_cast<std::ptrdiff_t>(pos), b.end());
    return out;
}

}  // namespace

TEST_CASE("StemReader: an MP3 seek anywhere lands on the straight decode") {
    std::vector<uint8_t> bytes = read_fixture("sine220.mp3");
    DecodedAudio full = old_full_decode(bytes);
    auto r = open_stem_reader(StemBytes{bytes, nullptr});
    REQUIRE(r->length_frames() == full.frames());
    seek_sweep(*r, full, 1777);
}

TEST_CASE("StemReader: MP3 seeks through the seek points on a long stream") {
    std::vector<uint8_t> bytes = long_untagged_mp3();
    DecodedAudio full = old_full_decode(bytes);
    // Every frame decodes to the same number of samples, so the frame length
    // is the decode's length over the frames the headers count.
    const int64_t frames = mp3_frame_count(bytes);
    REQUIRE(frames > 0);
    const int64_t per_frame = full.frames() / frames;
    REQUIRE(per_frame * frames == full.frames());
    auto r = open_stem_reader(StemBytes{bytes, nullptr});
    REQUIRE(r->length_frames() == full.frames());
    CHECK(read_to_end(*r) == full.samples);  // straight read still bit for bit
    seek_sweep(*r, full, 1777);
    // Targets on and beside the first frame boundary, on and just past half a
    // second (where the reader's seek points are spaced), past the middle,
    // one frame and one sample before the end, and the last frame.
    const int64_t half_second = r->sample_rate() / 2;
    const int64_t len = full.frames();
    for (int64_t f : {per_frame - 1, per_frame, per_frame + 1, half_second, half_second + 1,
                      len / 2 + 1, len - per_frame - 1, len - 1})
        check_seek_lands(*r, full, f, 2048);
}

TEST_CASE("StemReader: a FLAC whose header says 0 frames is counted on open") {
    std::vector<uint8_t> original = read_fixture("sine220.flac");
    const DecodedAudio ref = old_full_decode(original);
    auto o = open_stem_reader(StemBytes{original, nullptr});
    // A wrong byte offset would change the rate or channel count instead of
    // the length, so the unmodified file is checked against the independent
    // decode first.
    REQUIRE(ref.frames() > 0);
    REQUIRE(o->channels() == ref.channels);
    REQUIRE(o->sample_rate() == ref.sample_rate);
    REQUIRE(o->length_frames() == ref.frames());
    const std::vector<float> want = read_to_end(*o);

    std::vector<uint8_t> zeroed = testaudio::flac_with_unknown_length();
    auto r = open_stem_reader(StemBytes{zeroed, nullptr});
    CHECK(r->channels() == o->channels());
    CHECK(r->sample_rate() == o->sample_rate());
    CHECK(r->length_frames() == o->length_frames());
    CHECK(r->length_frames() == ref.frames());
    CHECK(read_to_end(*r) == want);
    CHECK_FALSE(r->failed());
    r->seek(0);
    CHECK(read_to_end(*r) == want);

    // A seek into the middle lands where the unmodified file's seek lands.
    const int64_t mid = o->length_frames() / 2;
    o->seek(mid);
    r->seek(mid);
    CHECK(read_to_end(*r) == read_to_end(*o));

    // A seek back to an earlier frame before the stem has ended lands where
    // the unmodified file's seek lands too.
    std::vector<float> part(static_cast<std::size_t>(mid) * r->channels());
    o->seek(mid);
    r->seek(mid);
    o->read(part.data(), mid / 2);
    r->read(part.data(), mid / 2);
    o->seek(mid / 2);
    r->seek(mid / 2);
    CHECK(read_to_end(*r) == read_to_end(*o));
}

TEST_CASE("StemReader: a FLAC behind an ID3 tag decodes the same as the plain file") {
    const std::vector<uint8_t> plain = read_fixture("sine220.flac");
    std::vector<uint8_t> tagged = id3_tag(0);
    tagged.insert(tagged.end(), plain.begin(), plain.end());

    auto p = open_stem_reader(StemBytes{plain, nullptr});
    auto t = open_stem_reader(StemBytes{tagged, nullptr});
    CHECK(t->channels() == p->channels());
    CHECK(t->sample_rate() == p->sample_rate());
    CHECK(t->length_frames() == p->length_frames());
    CHECK(read_to_end(*t) == read_to_end(*p));
    CHECK_FALSE(t->failed());
}

TEST_CASE("StemReader: a damaged Opus stem plays again after a seek to before the damage") {
    std::vector<uint8_t> original = read_fixture("sine220.opus");
    DecodedAudio full = old_full_decode(original);
    std::vector<std::vector<unsigned char>> pk = opus_packets(original);
    // A code-3 packet (several frames) whose frame count is zero: libopus
    // rejects it as an invalid packet (RFC 6716 section 3.2.5). The packet
    // halfway through is an audio packet (the first two are the headers).
    const std::size_t bad = pk.size() / 2;
    REQUIRE(bad >= 2);
    REQUIRE(bad + 1 < pk.size());
    // The stem ends where the damaged packet starts: the samples of the audio
    // packets before it, less the pre-skip.
    int64_t before = -opus_preskip(pk[0]);
    for (std::size_t i = 2; i < bad; ++i) before += opus_samples(pk[i]);
    REQUIRE(before > 0);
    pk[bad] = {static_cast<unsigned char>(pk[bad][0] | 0x03), 0};
    std::vector<uint8_t> damaged;
    page_opus(pk, 7, damaged);

    auto r = open_stem_reader(StemBytes{damaged, nullptr});
    const int ch = r->channels();
    const std::vector<float> first = read_to_end(*r);
    REQUIRE(static_cast<int64_t>(first.size()) == before * ch);
    CHECK(before < r->length_frames());
    CHECK(r->failed());
    // Up to the damage the stem is the straight decode of the good file.
    REQUIRE(first.size() <= full.samples.size());
    CHECK(std::equal(first.begin(), first.end(), full.samples.begin()));

    // The replay from frame 0 is the first read again: same bytes, same
    // decoder, the same stop at the damage.
    r->seek(0);
    CHECK(read_to_end(*r) == first);
    CHECK(r->failed());  // the error is still remembered

    // The whole-file path keeps failing loudly on the same bytes.
    CHECK_THROWS_AS(decode_audio(damaged), std::runtime_error);
}
