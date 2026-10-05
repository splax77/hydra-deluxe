// The WAV / MP3 / FLAC stem readers, on miniaudio's dr_libs.
//
// Output is float at the file's own channel count and rate, as the old
// whole-file decode (ma_decoder) gave.
//
// WAV and FLAC go through ma_decoder; its seeks are exact and instant.
//
// MP3 goes through ma_dr_mp3 directly, because the vendored dr_mp3
// (miniaudio 0.11.25) seeks wrong in two ways:
//   * It counts frames two ways. Reading skips the LAME encoder delay (often
//     1105 frames), but its cursor and its seeks count the delay too. So the
//     reader seeks to "stem frame + delay" (a raw frame).
//   * Its own seek table lands thousands of frames off. After a seek it
//     resets the decoder, and the first frame or two then fail for want of
//     the bit reservoir (bytes an MP3 frame borrows from the frames before
//     it). dr_mp3 skips a failed frame silently, so its frame count slips.
// So the reader builds its own seek points at open. It walks the frame
// headers (no audio decoding) and simulates the bit reservoir, so each seek
// point names exactly how many frames dr_mp3 will really decode before the
// target. dr_mp3 then seeks with them through ma_dr_mp3_bind_seek_table.
// Each seek point also decodes two whole frames before any target, so the
// decoder's overlap and filter state are settled where the audio starts.
// After every table seek the reader checks that dr_mp3 landed on the frame
// the scan predicted. If not (a damaged stream), it falls back to the old
// exact way: restart and decode forward.

#include "audio/stem_reader.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <vector>

// miniaudio's configuration macros come from the miniaudio target
// (CMakeLists.txt), the same set its implementation TU is compiled with.
#include "miniaudio.h"

#include "audio/decode.h"

// miniaudio.h declares its dr_mp3 API only inside its implementation section
// (compiled once, in miniaudio.c), so it is invisible to other files. These
// are the vendored 0.11.25 declarations, copied verbatim from the dr_mp3
// header part of third_party/miniaudio/miniaudio.h; none of them depend on a
// configuration macro. The functions themselves link from the miniaudio
// target. If miniaudio is ever updated, re-copy them (the assert below fires).
static_assert(MA_VERSION_MAJOR == 0 && MA_VERSION_MINOR == 11 && MA_VERSION_REVISION == 25,
              "miniaudio changed: re-copy the dr_mp3 declarations in ma_reader.cpp");
extern "C" {
#define MA_DR_MP3_MAX_PCM_FRAMES_PER_MP3_FRAME  1152
#define MA_DR_MP3_MAX_SAMPLES_PER_FRAME         (MA_DR_MP3_MAX_PCM_FRAMES_PER_MP3_FRAME*2)
#define MA_DR_MP3_MAX_BITRESERVOIR_BYTES      511
#define MA_DR_MP3_MAX_FREE_FORMAT_FRAME_SIZE  2304
#define MA_DR_MP3_MAX_L3_FRAME_PAYLOAD_BYTES  MA_DR_MP3_MAX_FREE_FORMAT_FRAME_SIZE
typedef struct
{
    const ma_uint8 *buf;
    int pos, limit;
} ma_dr_mp3_bs;
typedef struct
{
    const ma_uint8 *sfbtab;
    ma_uint16 part_23_length, big_values, scalefac_compress;
    ma_uint8 global_gain, block_type, mixed_block_flag, n_long_sfb, n_short_sfb;
    ma_uint8 table_select[3], region_count[3], subblock_gain[3];
    ma_uint8 preflag, scalefac_scale, count1_table, scfsi;
} ma_dr_mp3_L3_gr_info;
typedef struct
{
    ma_dr_mp3_bs bs;
    ma_uint8 maindata[MA_DR_MP3_MAX_BITRESERVOIR_BYTES + MA_DR_MP3_MAX_L3_FRAME_PAYLOAD_BYTES];
    ma_dr_mp3_L3_gr_info gr_info[4];
    float grbuf[2][576], scf[40], syn[18 + 15][2*32];
    ma_uint8 ist_pos[2][39];
} ma_dr_mp3dec_scratch;
typedef struct
{
    float mdct_overlap[2][9*32], qmf_state[15*2*32];
    int reserv, free_format_bytes;
    ma_uint8 header[4], reserv_buf[511];
    ma_dr_mp3dec_scratch scratch;
} ma_dr_mp3dec;
typedef enum
{
    MA_DR_MP3_SEEK_SET,
    MA_DR_MP3_SEEK_CUR,
    MA_DR_MP3_SEEK_END
} ma_dr_mp3_seek_origin;
typedef struct
{
    ma_uint64 seekPosInBytes;
    ma_uint64 pcmFrameIndex;
    ma_uint16 mp3FramesToDiscard;
    ma_uint16 pcmFramesToDiscard;
} ma_dr_mp3_seek_point;
typedef enum
{
    MA_DR_MP3_METADATA_TYPE_ID3V1,
    MA_DR_MP3_METADATA_TYPE_ID3V2,
    MA_DR_MP3_METADATA_TYPE_APE,
    MA_DR_MP3_METADATA_TYPE_XING,
    MA_DR_MP3_METADATA_TYPE_VBRI
} ma_dr_mp3_metadata_type;
typedef struct
{
    ma_dr_mp3_metadata_type type;
    const void* pRawData;
    size_t rawDataSize;
} ma_dr_mp3_metadata;
typedef size_t (* ma_dr_mp3_read_proc)(void* pUserData, void* pBufferOut, size_t bytesToRead);
typedef ma_bool32 (* ma_dr_mp3_seek_proc)(void* pUserData, int offset, ma_dr_mp3_seek_origin origin);
typedef ma_bool32 (* ma_dr_mp3_tell_proc)(void* pUserData, ma_int64* pCursor);
typedef void (* ma_dr_mp3_meta_proc)(void* pUserData, const ma_dr_mp3_metadata* pMetadata);
typedef struct
{
    ma_dr_mp3dec decoder;
    ma_uint32 channels;
    ma_uint32 sampleRate;
    ma_dr_mp3_read_proc onRead;
    ma_dr_mp3_seek_proc onSeek;
    ma_dr_mp3_meta_proc onMeta;
    void* pUserData;
    void* pUserDataMeta;
    ma_allocation_callbacks allocationCallbacks;
    ma_uint32 mp3FrameChannels;
    ma_uint32 mp3FrameSampleRate;
    ma_uint32 pcmFramesConsumedInMP3Frame;
    ma_uint32 pcmFramesRemainingInMP3Frame;
    ma_uint8 pcmFrames[sizeof(float)*MA_DR_MP3_MAX_SAMPLES_PER_FRAME];
    ma_uint64 currentPCMFrame;
    ma_uint64 streamCursor;
    ma_uint64 streamLength;
    ma_uint64 streamStartOffset;
    ma_dr_mp3_seek_point* pSeekPoints;
    ma_uint32 seekPointCount;
    ma_uint32 delayInPCMFrames;
    ma_uint32 paddingInPCMFrames;
    ma_uint64 totalPCMFrameCount;
    ma_bool32 isVBR;
    ma_bool32 isCBR;
    size_t dataSize;
    size_t dataCapacity;
    size_t dataConsumed;
    ma_uint8* pData;
    ma_bool32 atEnd;
    struct
    {
        const ma_uint8* pData;
        size_t dataSize;
        size_t currentReadPos;
    } memory;
} ma_dr_mp3;
MA_API void ma_dr_mp3_version(ma_uint32* pMajor, ma_uint32* pMinor, ma_uint32* pRevision);
MA_API ma_bool32 ma_dr_mp3_init_memory(ma_dr_mp3* pMP3, const void* pData, size_t dataSize, const ma_allocation_callbacks* pAllocationCallbacks);
MA_API void ma_dr_mp3_uninit(ma_dr_mp3* pMP3);
MA_API ma_uint64 ma_dr_mp3_read_pcm_frames_f32(ma_dr_mp3* pMP3, ma_uint64 framesToRead, float* pBufferOut);
MA_API ma_bool32 ma_dr_mp3_seek_to_pcm_frame(ma_dr_mp3* pMP3, ma_uint64 frameIndex);
MA_API ma_uint64 ma_dr_mp3_get_pcm_frame_count(ma_dr_mp3* pMP3);
MA_API ma_bool32 ma_dr_mp3_bind_seek_table(ma_dr_mp3* pMP3, ma_uint32 seekPointCount, ma_dr_mp3_seek_point* pSeekPoints);
}  // extern "C"

namespace hydra::audio::detail {

namespace {

constexpr ma_uint64 kSkipChunk = 4096;

// ---- WAV / FLAC (and anything dr_mp3 itself can't open) -----------------

class MaReader final : public StemReader {
public:
    explicit MaReader(StemBytes bytes) : bytes_(std::move(bytes)) {
        ma_decoder_config cfg = ma_decoder_config_init(ma_format_f32, 0, 0);
        if (ma_decoder_init_memory(bytes_.data(), bytes_.size(), &cfg, &dec_) != MA_SUCCESS)
            throw std::runtime_error("decode_audio: miniaudio could not open the stream");
        channels_ = static_cast<int>(dec_.outputChannels);
        rate_ = static_cast<int>(dec_.outputSampleRate);
        if (channels_ <= 0) {
            ma_decoder_uninit(&dec_);
            throw std::runtime_error("decode_audio: miniaudio could not open the stream");
        }
        ma_uint64 len = 0;
        if (ma_decoder_get_length_in_pcm_frames(&dec_, &len) != MA_SUCCESS) len = 0;
        // A header length of 0 is counted (CountedLength). A WAV can't land
        // there with audio in it, because dr_wav takes its length from the
        // data chunk's size, so in practice it is a FLAC. dr_flac clamps every
        // seek target to the header's total, so a counted stem's seeks would
        // all land on frame 0; CountedLength seeks by decoding instead.
        scratch_.resize(static_cast<std::size_t>(kSkipChunk) * static_cast<std::size_t>(channels_));
        length_ = counted_.length(
            static_cast<int64_t>(len), static_cast<int64_t>(kSkipChunk), at_end_,
            [this] { return restart(); }, [this](int64_t n) { return skip(n); });
    }

    ~MaReader() override { ma_decoder_uninit(&dec_); }

    int channels() const override { return channels_; }
    int sample_rate() const override { return rate_; }
    int64_t length_frames() const override { return length_; }
    bool failed() const override { return failed_; }

    int64_t read(float* out, int64_t frames) override {
        int64_t done = 0;
        while (!at_end_ && done < frames) {
            ma_uint64 got = 0;
            const ma_result r = ma_decoder_read_pcm_frames(
                &dec_, out + static_cast<std::size_t>(done) * channels_,
                static_cast<ma_uint64>(frames - done), &got);
            done += static_cast<int64_t>(got);
            if (r == MA_AT_END || got == 0) {
                at_end_ = true;
            } else if (r != MA_SUCCESS) {
                failed_ = true;
                at_end_ = true;
            }
        }
        pos_ += done;
        return done;
    }

    void seek(int64_t frame) override {
        frame = std::clamp<int64_t>(frame, 0, length_);
        if (frame >= length_) {
            at_end_ = true;
            pos_ = length_;
            return;
        }
        if (counted_.counted()) {
            // As in read(): a decode error ends the stem, and failed() stays true.
            if (counted_.seek(frame, static_cast<int64_t>(kSkipChunk), pos_, at_end_,
                              [this] { return restart(); }, [this](int64_t n) { return skip(n); }))
                failed_ = true;
            return;
        }
        at_end_ = ma_decoder_seek_to_pcm_frame(&dec_, static_cast<ma_uint64>(frame)) != MA_SUCCESS;
        pos_ = frame;
    }

private:
    // CountedLength's two decoder calls.
    bool restart() { return ma_decoder_seek_to_pcm_frame(&dec_, 0) == MA_SUCCESS; }
    DecodeStep skip(int64_t frames) {
        ma_uint64 got = 0;
        const ma_result r = ma_decoder_read_pcm_frames(&dec_, scratch_.data(),
                                                       static_cast<ma_uint64>(frames), &got);
        DecodeStep step;
        step.frames = static_cast<int64_t>(got);
        step.more = r == MA_SUCCESS && got != 0;
        step.error = r != MA_SUCCESS && r != MA_AT_END && got != 0;
        return step;
    }

    StemBytes bytes_;
    ma_decoder dec_{};
    int channels_ = 0;
    int rate_ = 0;
    int64_t length_ = 0;
    int64_t pos_ = 0;         // the frame the next read returns
    CountedLength counted_;   // the header said 0 frames, so length_ is a count
    std::vector<float> scratch_;  // decode target for counted and skipped frames
    bool at_end_ = false;
    bool failed_ = false;
};

// ---- MP3 frame headers, read the way dr_mp3 (minimp3) reads them ---------

bool hdr_valid(const uint8_t* h) {
    return h[0] == 0xFF && ((h[1] & 0xF0) == 0xF0 || (h[1] & 0xFE) == 0xE2) &&
           ((h[1] >> 1) & 3) != 0 && (h[2] >> 4) != 15 && ((h[2] >> 2) & 3) != 3;
}
bool hdr_mpeg1(const uint8_t* h) { return (h[1] & 0x08) != 0; }
bool hdr_layer1(const uint8_t* h) { return (h[1] & 6) == 6; }
bool hdr_layer3(const uint8_t* h) { return ((h[1] >> 1) & 3) == 1; }
bool hdr_free_format(const uint8_t* h) { return (h[2] & 0xF0) == 0; }
// Same version, layer, sample rate and free-format-ness: dr_mp3 keeps its
// state (reservoir included) only across frames that compare equal.
bool hdr_compare(const uint8_t* a, const uint8_t* b) {
    return hdr_valid(b) && ((a[1] ^ b[1]) & 0xFE) == 0 && ((a[2] ^ b[2]) & 0x0C) == 0 &&
           hdr_free_format(a) == hdr_free_format(b);
}
unsigned hdr_bitrate_kbps(const uint8_t* h) {
    static const uint8_t halfrate[2][3][15] = {
        {{0, 4, 8, 12, 16, 20, 24, 28, 32, 40, 48, 56, 64, 72, 80},
         {0, 4, 8, 12, 16, 20, 24, 28, 32, 40, 48, 56, 64, 72, 80},
         {0, 16, 24, 28, 32, 40, 48, 56, 64, 72, 80, 88, 96, 112, 128}},
        {{0, 16, 20, 24, 28, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160},
         {0, 16, 24, 28, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192},
         {0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224}},
    };
    return 2u * halfrate[hdr_mpeg1(h) ? 1 : 0][((h[1] >> 1) & 3) - 1][h[2] >> 4];
}
unsigned hdr_sample_rate_hz(const uint8_t* h) {
    static const unsigned hz[3] = {44100, 48000, 32000};
    return hz[(h[2] >> 2) & 3] >> (hdr_mpeg1(h) ? 0 : 1) >> ((h[1] & 0x10) ? 0 : 1);
}
unsigned hdr_frame_samples(const uint8_t* h) {
    if (hdr_layer1(h)) return 384;
    return (h[1] & 14) == 2 ? 576 : 1152;  // MPEG-2/2.5 Layer III: 576
}
// Whole frame size in bytes, padding included.
unsigned hdr_frame_size(const uint8_t* h) {
    unsigned bytes = hdr_frame_samples(h) * hdr_bitrate_kbps(h) * 125 / hdr_sample_rate_hz(h);
    if (hdr_layer1(h)) bytes &= ~3u;
    const unsigned pad = (h[2] & 0x02) ? (hdr_layer1(h) ? 4u : 1u) : 0u;
    return bytes + pad;
}

constexpr int kMaxReservoir = 511;  // dr_mp3's MA_DR_MP3_MAX_BITRESERVOIR_BYTES

// What a header walk learns about one stream: where each frame ends, and
// per frame how far back its audio data starts (main_data_begin) and how many
// audio-data bytes it carries itself. Layer I/II frames borrow nothing.
struct FrameScan {
    std::size_t start = 0;            // byte offset of the first frame
    std::vector<uint32_t> end;        // byte offset just past each frame
    std::vector<uint16_t> back;       // main_data_begin
    std::vector<uint16_t> own;        // the frame's own audio-data bytes
    unsigned samples = 0;             // PCM frames per MP3 frame
    std::size_t first_frame_start(std::size_t j) const { return j == 0 ? start : end[j - 1]; }
};

// Walks frames from `start` while they chain the way dr_mp3 decodes them
// without resyncing: each header valid, comparing equal to the one before,
// and followed by another such header or by the exact end of the data (else
// dr_mp3 drops its state at that frame). Stops at the first frame that
// breaks the chain; seeks past the scanned part still work, unchecked.
FrameScan scan_frames(const uint8_t* d, std::size_t size, std::size_t start) {
    FrameScan s;
    s.start = start;
    if (size > 0xFFFFFFFFull) return s;  // offsets stored in 32 bits
    std::size_t pos = start;
    const uint8_t* prev = nullptr;
    while (pos + 4 <= size) {
        const uint8_t* h = d + pos;
        if (!hdr_valid(h) || hdr_free_format(h)) break;
        if (prev && !hdr_compare(prev, h)) break;
        const std::size_t fb = hdr_frame_size(h);
        if (fb < 4 || pos + fb > size) break;
        if (pos + fb != size && (pos + fb + 4 > size || !hdr_compare(h, d + pos + fb))) break;
        unsigned back = 0, own = 0;
        if (hdr_layer3(h)) {
            const std::size_t crc = (h[1] & 1) ? 0 : 2;
            const bool mono = (h[3] & 0xC0) == 0xC0;
            const std::size_t side = hdr_mpeg1(h) ? (mono ? 17 : 32) : (mono ? 9 : 17);
            if (4 + crc + side > fb) break;
            const uint8_t* si = h + 4 + crc;
            back = hdr_mpeg1(h) ? (static_cast<unsigned>(si[0]) << 1) | (si[1] >> 7) : si[0];
            own = static_cast<unsigned>(fb - 4 - crc - side);
        }
        if (s.end.empty()) s.samples = hdr_frame_samples(h);
        s.end.push_back(static_cast<uint32_t>(pos + fb));
        s.back.push_back(static_cast<uint16_t>(back));
        s.own.push_back(static_cast<uint16_t>(own));
        prev = h;
        pos += fb;
    }
    return s;
}

// dr_mp3 decoding frames [first, last] from a freshly reset decoder, with
// the audio left undecoded (only the reservoir kept), as its seek-table
// discard loop does. A frame decodes only if the reservoir holds the bytes
// it borrows; a failed frame is skipped and yields no samples. Returns how
// many frames decoded, or -1 if `last` itself would fail.
int simulate_reset(const FrameScan& s, std::size_t first, std::size_t last) {
    int reserv = 0;
    int decoded = 0;
    for (std::size_t j = first; j <= last; ++j) {
        const int back = s.back[j];
        const bool ok = reserv >= back;
        reserv = std::min(kMaxReservoir, std::min(reserv, back) + static_cast<int>(s.own[j]));
        if (ok) ++decoded;
        if (j == last) return ok ? decoded : -1;
    }
    return -1;
}

// ---- the MP3 reader --------------------------------------------------------

class Mp3Reader final : public StemReader {
public:
    // Never throws for a stream dr_mp3 can't open: ok() stays false and the
    // bytes can be taken back for the ma_decoder fallback.
    explicit Mp3Reader(StemBytes bytes) : bytes_(std::move(bytes)) {
        if (!ma_dr_mp3_init_memory(&mp3_, bytes_.data(), bytes_.size(), nullptr)) return;
        inited_ = true;
        channels_ = static_cast<int>(mp3_.channels);
        rate_ = static_cast<int>(mp3_.sampleRate);
        if (channels_ <= 0 || rate_ <= 0) return;
        length_ = static_cast<int64_t>(ma_dr_mp3_get_pcm_frame_count(&mp3_));
        delay_ = mp3_.delayInPCMFrames;
        scratch_.resize(static_cast<std::size_t>(kSkipChunk) * static_cast<std::size_t>(channels_));
        build_seek_points();
        if (!seek_points_.empty())
            ma_dr_mp3_bind_seek_table(&mp3_, static_cast<ma_uint32>(seek_points_.size()),
                                      seek_points_.data());
        // A straight read starts from a freshly reset decoder, exactly as the
        // old whole-file decode did.
        ok_ = ma_dr_mp3_seek_to_pcm_frame(&mp3_, 0) != MA_FALSE;
    }

    ~Mp3Reader() override {
        if (inited_) ma_dr_mp3_uninit(&mp3_);
    }

    bool ok() const { return ok_; }
    StemBytes take_bytes() { return std::move(bytes_); }

    int channels() const override { return channels_; }
    int sample_rate() const override { return rate_; }
    int64_t length_frames() const override { return length_; }

    int64_t read(float* out, int64_t frames) override {
        if (at_end_ || frames <= 0) return 0;
        const ma_uint64 got =
            ma_dr_mp3_read_pcm_frames_f32(&mp3_, static_cast<ma_uint64>(frames), out);
        if (got < static_cast<ma_uint64>(frames)) at_end_ = true;
        pos_ += static_cast<int64_t>(got);
        return static_cast<int64_t>(got);
    }

    void seek(int64_t frame) override {
        frame = std::clamp<int64_t>(frame, 0, length_);
        if (frame >= length_) {
            at_end_ = true;
            pos_ = length_;
            return;
        }
        if (frame == pos_ && !at_end_) return;
        // A short hop forward: decode ahead from here. That stays bit-exact
        // with a straight read, and is cheaper than a table seek.
        if (frame > pos_ && !at_end_ && frame - pos_ <= rate_) {
            at_end_ = !skip(static_cast<ma_uint64>(frame - pos_));
            pos_ = frame;
            return;
        }
        const ma_uint64 raw = static_cast<ma_uint64>(frame) + delay_;
        if (frame > 0 && !seek_points_.empty() && raw >= seek_points_.front().pcmFrameIndex &&
            ma_dr_mp3_seek_to_pcm_frame(&mp3_, raw) && landed(raw)) {
            at_end_ = false;
            pos_ = frame;
            return;
        }
        at_end_ = !restart_and_skip(static_cast<ma_uint64>(frame));
        pos_ = frame;
    }

private:
    // Seek points about every 0.5 s. Each one for target frame t starts the
    // decoder at a frame s before t - 2, discards up to and including frame
    // t - 2 (at least two decoded frames, the reservoir refilled), then
    // decodes frames t - 2 and t - 1 whole before the audio it hands out.
    void build_seek_points() {
        const FrameScan s = scan_frames(bytes_.data(), mp3_.memory.dataSize,
                                        static_cast<std::size_t>(mp3_.streamStartOffset));
        const std::size_t n = s.end.size();
        if (n < 8 || s.samples == 0) return;
        // A straight read from the start: frames before the first one with
        // its whole reservoir fail and yield nothing; after it, all decode.
        std::size_t first = n;
        int reserv = 0;
        for (std::size_t j = 0; j < n; ++j) {
            if (reserv >= s.back[j]) {
                first = j;
                break;
            }
            reserv = std::min(kMaxReservoir, std::min(reserv, static_cast<int>(s.back[j])) +
                                                 static_cast<int>(s.own[j]));
        }
        if (first >= n) return;
        auto raw_start = [&](std::size_t j) {
            return static_cast<ma_uint64>(j - first) * s.samples;
        };
        const std::size_t step = std::max<std::size_t>(
            1, (static_cast<std::size_t>(rate_) / 2 + s.samples / 2) / s.samples);
        for (std::size_t t = first + 3; t < n; t += step) {
            const std::size_t last = t - 2;  // the final discarded frame
            // dr_mp3 skips the encoder delay only from the very start; a
            // seek must resume past it.
            if (raw_start(last) < delay_) continue;
            for (std::size_t from = last - 1;; --from) {
                const int decoded = simulate_reset(s, from, last);
                if (decoded >= 2) {
                    ma_dr_mp3_seek_point p{};
                    p.seekPosInBytes = s.first_frame_start(from);
                    p.pcmFrameIndex = raw_start(t);
                    p.mp3FramesToDiscard = static_cast<ma_uint16>(decoded);
                    p.pcmFramesToDiscard = static_cast<ma_uint16>(raw_start(t) - raw_start(last));
                    seek_points_.push_back(p);
                    break;
                }
                if (from == 0 || last - from >= 32) break;
            }
        }
        // Kept for checking where a table seek lands.
        scan_first_ = first;
        scan_samples_ = s.samples;
        frame_end_ = s.end;
    }

    // After a table seek to raw frame `raw`, dr_mp3 holds the frame that
    // contains raw - 1, with raw - (its first frame) of it consumed. Checks
    // that against the scan; a slipped frame count fails it.
    bool landed(ma_uint64 raw) const {
        if (mp3_.currentPCMFrame != raw || raw == 0) return false;
        const ma_uint64 j = scan_first_ + (raw - 1) / scan_samples_;
        if (j >= frame_end_.size()) return true;  // past the scanned part
        const ma_uint64 consumed = raw - (j - scan_first_) * scan_samples_;
        return mp3_.memory.currentReadPos == frame_end_[static_cast<std::size_t>(j)] &&
               mp3_.pcmFramesConsumedInMP3Frame == consumed;
    }

    bool skip(ma_uint64 frames) {
        while (frames > 0) {
            const ma_uint64 got = ma_dr_mp3_read_pcm_frames_f32(
                &mp3_, std::min(frames, kSkipChunk), scratch_.data());
            if (got == 0) return false;
            frames -= got;
        }
        return true;
    }

    // The always-exact way: back to the start, decode forward.
    bool restart_and_skip(ma_uint64 frame) {
        if (!ma_dr_mp3_seek_to_pcm_frame(&mp3_, 0)) return false;
        return skip(frame);
    }

    StemBytes bytes_;
    ma_dr_mp3 mp3_{};
    bool inited_ = false;
    bool ok_ = false;
    int channels_ = 0;
    int rate_ = 0;
    int64_t length_ = 0;
    ma_uint64 delay_ = 0;  // raw frames dr_mp3 skips before stem frame 0
    int64_t pos_ = 0;      // the stem frame the next read returns
    bool at_end_ = false;
    std::vector<float> scratch_;  // decode target for skipped frames
    std::vector<ma_dr_mp3_seek_point> seek_points_;
    std::vector<uint32_t> frame_end_;
    ma_uint64 scan_first_ = 0;
    ma_uint64 scan_samples_ = 1;
};

// An ID3v2 tag in front of a FLAC stream: sniff_format calls it MP3, but
// ma_decoder (which tries FLAC before MP3) always decoded it as FLAC.
bool flac_behind_id3(const StemBytes& b) {
    const uint8_t* d = b.data();
    if (b.size() < 10 || std::memcmp(d, "ID3", 3) != 0) return false;
    std::size_t tag = (static_cast<std::size_t>(d[6] & 0x7F) << 21) |
                      (static_cast<std::size_t>(d[7] & 0x7F) << 14) |
                      (static_cast<std::size_t>(d[8] & 0x7F) << 7) |
                      static_cast<std::size_t>(d[9] & 0x7F);
    tag += 10;
    if (d[5] & 0x10) tag += 10;
    return tag + 4 <= b.size() && std::memcmp(d + tag, "fLaC", 4) == 0;
}

}  // namespace

std::unique_ptr<StemReader> open_ma_reader(StemBytes bytes) {
    if (sniff_format(bytes.data(), bytes.size()) == AudioFormat::Mp3 && !flac_behind_id3(bytes)) {
        auto mp3 = std::make_unique<Mp3Reader>(std::move(bytes));
        if (mp3->ok()) return mp3;
        bytes = mp3->take_bytes();  // let ma_decoder try, and report as before
    }
    return std::make_unique<MaReader>(std::move(bytes));
}

}  // namespace hydra::audio::detail
