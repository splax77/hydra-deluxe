#include "audio/stream_mix.h"

#include <algorithm>
#include <cstring>
#include <numeric>
#include <stdexcept>
#include <utility>

#include "audio/mixer.h"  // stem_converter_config, shared with mix_stems
#include "core/error_kind.h"

// miniaudio's configuration macros come from the miniaudio target.
#include "miniaudio.h"

namespace hydra::audio {

// ---- BufferSource ---------------------------------------------------------

BufferSource::BufferSource(DecodedAudio audio)
    : audio_(std::move(audio)), length_(audio_.frames()) {}

int64_t BufferSource::read(float* out, int64_t frames) {
    const int64_t n = std::clamp<int64_t>(length_ - pos_, 0, std::max<int64_t>(frames, 0));
    if (n > 0) {
        const std::size_t ch = static_cast<std::size_t>(audio_.channels);
        std::memcpy(out, audio_.samples.data() + static_cast<std::size_t>(pos_) * ch,
                    static_cast<std::size_t>(n) * ch * sizeof(float));
        pos_ += n;
    }
    return n;
}

void BufferSource::seek(int64_t frame) { pos_ = std::clamp<int64_t>(frame, 0, length_); }

// ---- StreamMix ------------------------------------------------------------

// One stem: its reader, and the converter that turns the reader's frames into
// output-format frames. Held by pointer because an initialized
// ma_data_converter points into itself and must never move.
struct StreamMix::Stem {
    std::unique_ptr<StemReader> reader;
    int in_channels = 0;
    int out_channels = 0;
    bool passthrough = true;  // from stem_converter_config: no converter needed

    // The converter lives in a heap block sized once at construction. A seek
    // re-initializes it in that same block instead of calling
    // ma_data_converter_reset, which is broken in the vendored miniaudio
    // 0.11.25: its first-order low-pass "clear" zeroes the filter coefficient
    // (not the filter history), and its biquad "clear" only clears channel 0.
    // Re-init gives exactly a fresh converter and allocates nothing.
    ma_data_converter_config cfg{};
    ma_data_converter conv{};
    std::vector<std::max_align_t> conv_heap;
    bool conv_ready = false;
    std::vector<float> in_buf;  // reader frames waiting to be converted
    int64_t in_pos = 0;         // first unconverted frame in in_buf
    int64_t in_count = 0;       // frames held in in_buf
    bool reader_done = false;   // the reader returned 0

    // The resampler only lines up with the output grid where an output frame
    // falls on a whole input frame: every `align_out` output frames, which is
    // `align_in` input frames (160 and 147 for 44.1 -> 48 kHz). A seek restarts
    // the converter on such a frame, so the audio after it lines up with a
    // straight read sample for sample (see seek_to for odd rate pairs).
    int64_t align_out = 1;
    int64_t align_in = 1;

    int64_t converted_length = 0;  // output frames the whole stem converts to
    int64_t out_pos = 0;           // output frames produced since the stem start
    bool ended = false;            // silent until the next seek

    Stem() = default;
    Stem(const Stem&) = delete;
    Stem& operator=(const Stem&) = delete;
    ~Stem() {
        if (conv_ready) ma_data_converter_uninit(&conv, nullptr);
    }

    // Sets up the converter for `config` in a heap block it keeps. Throws on
    // failure (construction only).
    void init_converter(const ma_data_converter_config& config) {
        cfg = config;
        std::size_t heap_bytes = 0;
        if (ma_data_converter_get_heap_size(&cfg, &heap_bytes) != MA_SUCCESS)
            throw KindedError(ErrorKind::AudioDecode,"StreamMix: data converter init failed");
        conv_heap.resize(heap_bytes / sizeof(std::max_align_t) + 1);
        if (!restart_converter())
            throw KindedError(ErrorKind::AudioDecode,"StreamMix: data converter init failed");
    }

    // A fresh converter in the same heap block: no state from before.
    bool restart_converter() {
        if (conv_ready) ma_data_converter_uninit(&conv, nullptr);  // frees nothing
        conv_ready = ma_data_converter_init_preallocated(&cfg, conv_heap.data(), &conv) == MA_SUCCESS;
        return conv_ready;
    }

    // Writes up to `want` output-format frames into `dst`; returns how many.
    // Fewer than `want` means the stem has nothing more to give.
    int64_t produce(float* dst, int64_t want) {
        int64_t got = 0;
        if (passthrough) {
            while (got < want && !reader_done) {
                const int64_t k = reader->read(dst + static_cast<std::size_t>(got) * out_channels,
                                               want - got);
                if (k <= 0) {
                    reader_done = true;
                    break;
                }
                got += k;
            }
            return got;
        }
        const int64_t in_cap = static_cast<int64_t>(in_buf.size()) / in_channels;
        while (got < want) {
            if (in_pos == in_count && !reader_done) {
                const int64_t k = reader->read(in_buf.data(), in_cap);
                in_pos = 0;
                in_count = k > 0 ? k : 0;
                if (k <= 0) reader_done = true;
            }
            // With no input left, the converter can still hand out the frames
            // it holds, exactly as a single whole-stem call would.
            ma_uint64 fin = static_cast<ma_uint64>(in_count - in_pos);
            ma_uint64 fout = static_cast<ma_uint64>(want - got);
            const ma_result r = ma_data_converter_process_pcm_frames(
                &conv, in_buf.data() + static_cast<std::size_t>(in_pos) * in_channels, &fin,
                dst + static_cast<std::size_t>(got) * out_channels, &fout);
            if (r != MA_SUCCESS) break;
            in_pos += static_cast<int64_t>(fin);
            got += static_cast<int64_t>(fout);
            if (fin == 0 && fout == 0 && (reader_done || in_pos < in_count)) break;
        }
        return got;
    }

    // Moves this stem to output frame `rel` (counted from the stem's start).
    // `scratch` holds one block of output frames for the frames thrown away
    // between the aligned restart point and `rel`.
    void seek_to(int64_t rel, float* scratch) {
        ended = false;
        reader_done = false;
        in_pos = in_count = 0;
        if (rel >= converted_length) {
            out_pos = converted_length;
            ended = true;
            return;
        }
        if (passthrough) {
            reader->seek(rel);
            out_pos = rel;
            return;
        }
        // Restart on the last aligned frame at or before `rel`, then throw away
        // the few frames up to it. An odd rate pair whose alignment points lie
        // over a block apart restarts at `rel` itself instead, within one
        // input frame, so a seek never decodes seconds of audio to get there.
        int64_t start = rel;
        int64_t in_frame = rel * align_in / align_out;
        if (align_out <= kBlockFrames) {
            start = rel / align_out * align_out;
            in_frame = start / align_out * align_in;
        }
        reader->seek(in_frame);
        if (!restart_converter()) {
            ended = true;
            return;
        }
        out_pos = start;
        while (out_pos < rel) {
            const int64_t want = std::min(kBlockFrames, rel - out_pos);
            const int64_t got = produce(scratch, want);
            out_pos += got;
            if (got < want) {
                ended = true;
                break;
            }
        }
    }
};

StreamMix::StreamMix(std::vector<std::unique_ptr<StemReader>> stems, int out_rate,
                     int out_channels, int64_t front_pad_frames)
    : out_rate_(out_rate),
      out_channels_(out_channels),
      pad_(std::max<int64_t>(front_pad_frames, 0)) {
    if (out_rate <= 0 || out_channels <= 0)
        throw KindedError(ErrorKind::AudioDecode,"StreamMix: invalid output format");
    int64_t longest = 0;
    for (std::unique_ptr<StemReader>& r : stems) {
        if (!r || r->channels() <= 0 || r->sample_rate() <= 0) continue;
        auto s = std::make_unique<Stem>();
        s->in_channels = r->channels();
        s->out_channels = out_channels;
        const int in_rate = r->sample_rate();
        const int64_t in_len = std::max<int64_t>(r->length_frames(), 0);
        s->reader = std::move(r);
        // The same converter setup mix_stems uses, so the output matches.
        const StemConverter sc =
            stem_converter_config(in_rate, s->in_channels, out_rate, out_channels);
        s->passthrough = sc.passthrough;
        if (s->passthrough) {
            s->converted_length = in_len;
        } else {
            s->init_converter(sc.config);
            ma_uint64 expected = 0;
            ma_data_converter_get_expected_output_frame_count(
                &s->conv, static_cast<ma_uint64>(in_len), &expected);
            s->converted_length = static_cast<int64_t>(expected);
            s->in_buf.resize(static_cast<std::size_t>(kBlockFrames) * s->in_channels);
            const int64_t g = std::gcd<int64_t, int64_t>(in_rate, out_rate);
            s->align_out = out_rate / g;
            s->align_in = in_rate / g;
        }
        longest = std::max(longest, s->converted_length);
        stems_.push_back(std::move(s));
    }
    length_ = pad_ + longest;
    block_.resize(static_cast<std::size_t>(kBlockFrames) * out_channels);
}

StreamMix::~StreamMix() = default;

int64_t StreamMix::read(float* out, int64_t frames) {
    const int64_t total = std::clamp<int64_t>(length_ - pos_, 0, std::max<int64_t>(frames, 0));
    const std::size_t oc = static_cast<std::size_t>(out_channels_);
    int64_t done = 0;
    while (done < total) {
        const int64_t n = std::min(kBlockFrames, total - done);
        float* dst = out + static_cast<std::size_t>(done) * oc;
        std::fill(dst, dst + static_cast<std::size_t>(n) * oc, 0.0f);
        // Front pad: silence before every stem.
        const int64_t lead = pos_ < pad_ ? std::min(n, pad_ - pos_) : 0;
        const int64_t m = n - lead;
        if (m > 0) {
            float* mix = dst + static_cast<std::size_t>(lead) * oc;
            for (std::unique_ptr<Stem>& s : stems_) {
                if (s->ended) continue;
                const int64_t want = std::min(m, s->converted_length - s->out_pos);
                if (want <= 0) continue;
                const int64_t got = s->produce(block_.data(), want);
                const std::size_t count = static_cast<std::size_t>(got) * oc;
                for (std::size_t i = 0; i < count; ++i) mix[i] += block_[i];
                s->out_pos += got;
                if (got < want) s->ended = true;  // decode error: silent from here
            }
        }
        pos_ += n;
        done += n;
    }
    return done;
}

void StreamMix::seek(int64_t frame) {
    pos_ = std::clamp<int64_t>(frame, 0, length_);
    const int64_t rel = std::max<int64_t>(pos_ - pad_, 0);
    for (std::unique_ptr<Stem>& s : stems_) s->seek_to(rel, block_.data());
}

}  // namespace hydra::audio
