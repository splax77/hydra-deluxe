// Tests for audio/stem_reader: the seekable per-format decoders behind the
// Preview's audio. The reference for every check is a copy of the old
// whole-file decoders (below, test-only), so the readers are pinned to exactly
// what the Preview played before they existed. Fixtures: the public-domain
// 220 Hz sine under testdata/audio in each format (the WAV and FLAC ones were
// made with ffmpeg's sine generator, 1 s at 44.1 kHz; the FLAC is stereo).

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include <ogg/ogg.h>
#include <opus.h>

#include "miniaudio.h"
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

#include "app/preview_source.h"
#include "app/user_messages.h"
#include "audio/decode.h"
#include "audio/stem_reader.h"
#include "core/little_endian.h"
#include "core/winstr.h"

#include "audio_util.h"

using namespace hydra;
using namespace hydra::audio;

using testaudio::fixture_path;
using testaudio::id3_tag;
using testaudio::max_diff;
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

// Every Opus packet of the file's first stream, in order (headers included).
std::vector<std::vector<unsigned char>> opus_packets(const std::vector<uint8_t>& b) {
    std::vector<std::vector<unsigned char>> out;
    ogg_sync_state oy;
    ogg_sync_init(&oy);
    char* buf = ogg_sync_buffer(&oy, static_cast<long>(b.size()));
    std::memcpy(buf, b.data(), b.size());
    ogg_sync_wrote(&oy, static_cast<long>(b.size()));
    ogg_stream_state os;
    bool ready = false;
    ogg_page og;
    while (ogg_sync_pageout(&oy, &og) == 1) {
        if (!ready) {
            ogg_stream_init(&os, ogg_page_serialno(&og));
            ready = true;
        }
        ogg_stream_pagein(&os, &og);
        ogg_packet op;
        while (ogg_stream_packetout(&os, &op) == 1)
            out.emplace_back(op.packet, op.packet + op.bytes);
    }
    if (ready) ogg_stream_clear(&os);
    ogg_sync_clear(&oy);
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
        if (i >= 2) g += opus_packet_get_nb_samples(pk[i].data(),
                                                     static_cast<opus_int32>(pk[i].size()), 48000);
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

TEST_CASE("StemReader: a seek lands on the same audio within tolerance") {
    for (const char* name : kFixtures) {
        CAPTURE(std::string(name));
        std::vector<uint8_t> bytes = read_fixture(name);
        DecodedAudio full = old_full_decode(bytes);
        auto r = open_stem_reader(StemBytes{bytes, nullptr});
        const int64_t len = r->length_frames();
        const int ch = r->channels();
        const int64_t settle = r->sample_rate() / 50;  // 20 ms warm-up
        // len / 3 last: a backward seek after reading near the end.
        for (int64_t f : {int64_t{0}, int64_t{1}, len / 2, len - 100, len / 3}) {
            CAPTURE(f);
            r->seek(f);
            std::vector<float> buf(4800 * static_cast<std::size_t>(ch));
            int64_t n = 0;
            while (n < 4800) {
                int64_t k = r->read(buf.data() + n * ch, 4800 - n);
                if (k == 0) break;
                n += k;
            }
            CHECK(n == std::min<int64_t>(4800, len - f));
            CHECK(max_diff(buf, full.samples.data() + f * ch, static_cast<std::size_t>(settle * ch),
                           static_cast<std::size_t>(n * ch)) <= 1e-3f);
        }
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

        read_to_end(*r);  // drain, then start over
        r->seek(0);
        std::vector<float> again = read_to_end(*r);
        REQUIRE(again.size() == full.samples.size());
        CHECK(max_diff(again, full.samples.data(), 0, again.size()) <= 1e-3f);
    }
}

TEST_CASE("StemReader: an Opus seek mid-page matches the straight decode") {
    // Opus decodes from a reset state through a pre-roll before the target
    // (kPreRoll in src/audio/opus_reader.cpp); odd targets land inside a packet
    // and inside a page.
    std::vector<uint8_t> bytes = read_fixture("sine220.opus");
    DecodedAudio full = old_full_decode(bytes);
    auto r = open_stem_reader(StemBytes{bytes, nullptr});
    for (int64_t f : {int64_t{48000}, int64_t{100001}, int64_t{200000}}) {
        CAPTURE(f);
        r->seek(f);
        std::vector<float> buf(2000);
        REQUIRE(r->read(buf.data(), 2000) == 2000);
        // After the first 20 ms.
        CHECK(max_diff(buf, full.samples.data() + f, 960, 2000) <= 1e-3f);
    }
}

TEST_CASE("StemReader: Opus header gain is applied") {
    std::vector<std::vector<unsigned char>> pk = opus_packets(read_fixture("sine220.opus"));
    REQUIRE(pk.size() > 2);
    std::vector<uint8_t> plain;
    page_opus(pk, 7, plain);
    pk[0][16] = static_cast<unsigned char>(1541 & 0xFF);  // +6.02 dB in Q7.8
    pk[0][17] = static_cast<unsigned char>(1541 >> 8);
    std::vector<uint8_t> loud;
    page_opus(pk, 7, loud);

    auto a = open_stem_reader(StemBytes{plain, nullptr});
    auto b = open_stem_reader(StemBytes{loud, nullptr});
    std::vector<float> x = read_to_end(*a);
    std::vector<float> y = read_to_end(*b);
    REQUIRE(x.size() == y.size());
    CHECK(x == old_full_decode(read_fixture("sine220.opus")).samples);  // re-paging is lossless
    std::vector<float> doubled(x.size());
    std::transform(x.begin(), x.end(), doubled.begin(), [](float s) { return 2.0f * s; });
    CHECK(max_diff(y, doubled.data(), 0, y.size()) <= 1e-3f);
}

TEST_CASE("StemReader: a chained Opus file plays as one stream") {
    std::vector<uint8_t> one = read_fixture("sine220.opus");
    DecodedAudio full = old_full_decode(one);
    std::vector<std::vector<unsigned char>> pk = opus_packets(one);
    std::vector<uint8_t> chained;
    page_opus(pk, 1, chained);
    page_opus(pk, 2, chained);

    auto r = open_stem_reader(StemBytes{chained, nullptr});
    CHECK(r->length_frames() == 2 * full.frames());
    std::vector<float> got = read_to_end(*r);
    REQUIRE(static_cast<int64_t>(got.size()) == 2 * full.frames());
    // Each link starts from a fresh decoder and drops its own pre-skip, so
    // each half is the single file exactly.
    CHECK(std::equal(full.samples.begin(), full.samples.end(), got.begin()));
    CHECK(std::equal(full.samples.begin(), full.samples.end(), got.begin() + full.samples.size()));

    // A seek into the second link lands in it.
    const int64_t f = full.frames() + 100000;
    r->seek(f);
    std::vector<float> buf(2000);
    REQUIRE(r->read(buf.data(), 2000) == 2000);
    CHECK(max_diff(buf, full.samples.data() + 100000, 960, 2000) <= 1e-3f);
}

TEST_CASE("StemReader: Opus end trimming stops at the last granule") {
    // The fixture's last page says 240312 samples with a 312 pre-skip, so
    // 240000 frames are real audio; the decoder emits 648 more of padding,
    // which the Preview has always played (trim_end off, the default).
    std::vector<uint8_t> bytes = read_fixture("sine220.opus");
    detail::OpusReaderOptions trim;
    trim.trim_end = true;
    auto t = detail::open_opus_reader(StemBytes{bytes, nullptr}, nullptr, trim);
    CHECK(t->length_frames() == 240000);
    std::vector<float> got = read_to_end(*t);
    CHECK(got.size() == 240000u);
    DecodedAudio full = old_full_decode(bytes);
    CHECK(std::equal(got.begin(), got.end(), full.samples.begin()));

    auto u = open_stem_reader(StemBytes{bytes, nullptr});
    CHECK(u->length_frames() == 240648);
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

TEST_CASE("StemReader: unrecognized bytes throw a decode_audio error") {
    std::vector<uint8_t> junk = {0x89, 'P', 'N', 'G', 0, 0, 0, 0};
    try {
        open_stem_reader(StemBytes{junk, nullptr});
        FAIL("expected a throw");
    } catch (const std::runtime_error& e) {
        CHECK(std::string(e.what()).rfind("decode_audio:", 0) == 0);
    }
    app::PreviewAudioStem missing;
    missing.path = fixture_path("no_such_file.opus");
    CHECK_THROWS_AS(open_stem_reader(missing), std::runtime_error);
}

TEST_CASE("StemReader: an unrecognized container reads as damaged audio") {
    std::vector<uint8_t> junk = {0x89, 'P', 'N', 'G', 0, 0, 0, 0};
    try {
        open_stem_reader(StemBytes{junk, nullptr});
        FAIL("expected a throw");
    } catch (const std::exception& e) {
        CHECK(app::plain_error(e) ==
              "Hydra couldn't decode this song's audio files. They may be damaged; try "
              "downloading the song again.");
    }
}

namespace {

// Seeks to every `stride`-th frame (in an order that jumps back and forth),
// reads 4800 frames, and returns the worst difference from the straight
// decode after the first 20 ms. Also checks each read is full length.
float mp3_seek_sweep(StemReader& r, const DecodedAudio& full, int64_t stride) {
    const int64_t len = r.length_frames();
    const int ch = r.channels();
    const int64_t settle = r.sample_rate() / 50;
    std::vector<int64_t> targets;
    for (int64_t f = 0; f < len; f += stride) targets.push_back(f);
    // Interleave from both ends: every seek is a long jump, half backward.
    std::vector<int64_t> order;
    for (std::size_t a = 0, b = targets.size(); a < b;) {
        order.push_back(targets[a++]);
        if (a < b) order.push_back(targets[--b]);
    }
    std::vector<float> buf(4800 * static_cast<std::size_t>(ch));
    float worst = 0.0f;
    for (int64_t f : order) {
        CAPTURE(f);
        r.seek(f);
        int64_t n = 0;
        while (n < 4800) {
            const int64_t k = r.read(buf.data() + n * ch, 4800 - n);
            if (k == 0) break;
            n += k;
        }
        CHECK(n == std::min<int64_t>(4800, len - f));
        worst = std::max(worst, static_cast<float>(max_diff(
                                    buf, full.samples.data() + f * ch,
                                    static_cast<std::size_t>(settle * ch),
                                    static_cast<std::size_t>(n * ch))));
    }
    return worst;
}

// Byte size of the MPEG-1 Layer III frame whose header starts at `h`.
std::size_t mp3_l3_frame_size(const uint8_t* h) {
    static const int kbps[16] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
    static const int hz[4] = {44100, 48000, 32000, 0};
    return static_cast<std::size_t>(144000 * kbps[h[2] >> 4] / hz[(h[2] >> 2) & 3] + ((h[2] >> 1) & 1));
}

// A ~40 s MP3 with no ID3 or Xing tag: the fixture's audio frames, 24 times
// over. With no tag dr_mp3 counts every frame and skips no encoder delay, and
// a stream this long exercises many seek points.
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
    CHECK(mp3_seek_sweep(*r, full, 1777) <= 1e-3f);
}

TEST_CASE("StemReader: MP3 seeks through the seek points on a long stream") {
    std::vector<uint8_t> bytes = long_untagged_mp3();
    DecodedAudio full = old_full_decode(bytes);
    REQUIRE(full.frames() > 40 * 44100 / 2);
    auto r = open_stem_reader(StemBytes{bytes, nullptr});
    REQUIRE(r->length_frames() == full.frames());
    CHECK(read_to_end(*r) == full.samples);  // straight read still bit for bit
    CHECK(mp3_seek_sweep(*r, full, 1777) <= 1e-3f);
    // Odd targets right around frame boundaries and seek points.
    for (int64_t f : {int64_t{1151}, int64_t{1152}, int64_t{1153}, int64_t{22050}, int64_t{22051},
                      full.frames() / 2 + 1, full.frames() - 1153, full.frames() - 1}) {
        CAPTURE(f);
        r->seek(f);
        const int ch = r->channels();
        std::vector<float> buf(static_cast<std::size_t>(ch) * 2048);
        const int64_t n = r->read(buf.data(), 2048);
        CHECK(n == std::min<int64_t>(2048, full.frames() - f));
        // Seek points decode two whole frames before the target, so the
        // audio matches from the very first frame, no warm-up.
        CHECK(max_diff(buf, full.samples.data() + f * ch, 0, static_cast<std::size_t>(n * ch)) <=
              1e-3f);
    }
}

TEST_CASE("StemReader: a FLAC whose header says 0 frames is counted on open") {
    std::vector<uint8_t> original = read_fixture("sine220.flac");
    auto o = open_stem_reader(StemBytes{original, nullptr});
    // A wrong byte offset would change the rate or channel count instead of
    // the length, so the unmodified file is checked first.
    REQUIRE(o->length_frames() == 44100);
    const std::vector<float> want = read_to_end(*o);

    std::vector<uint8_t> zeroed = testaudio::flac_with_unknown_length();
    auto r = open_stem_reader(StemBytes{zeroed, nullptr});
    CHECK(r->channels() == o->channels());
    CHECK(r->sample_rate() == o->sample_rate());
    CHECK(r->length_frames() == o->length_frames());
    CHECK(r->length_frames() == old_full_decode(original).frames());
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
    pk[bad] = {static_cast<unsigned char>(pk[bad][0] | 0x03), 0};
    std::vector<uint8_t> damaged;
    page_opus(pk, 7, damaged);

    auto r = open_stem_reader(StemBytes{damaged, nullptr});
    const int ch = r->channels();
    const std::vector<float> first = read_to_end(*r);
    const int64_t first_frames = static_cast<int64_t>(first.size()) / ch;
    CHECK(first_frames < r->length_frames());
    CHECK(r->failed());

    r->seek(0);
    const std::vector<float> again = read_to_end(*r);
    CHECK(static_cast<int64_t>(again.size()) / ch == first_frames);
    REQUIRE(static_cast<int64_t>(again.size()) >= 2 * 960 * ch);
    CHECK(max_diff(again, full.samples.data(), 960 * static_cast<std::size_t>(ch),
                   2 * 960 * static_cast<std::size_t>(ch)) <= 1e-3f);
    CHECK(r->failed());  // the error is still remembered

    // The whole-file path keeps failing loudly on the same bytes.
    CHECK_THROWS_AS(decode_audio(damaged), std::runtime_error);
}
