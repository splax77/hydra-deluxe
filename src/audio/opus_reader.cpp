// The Ogg Opus stem reader.
//
// Hydra decodes Opus by hand with libogg + libopus (ADR 0006); there is no
// opusfile. To seek, the reader first builds a page index: one fast pass over
// the Ogg page headers and segment tables (no decoding, no CRC check) that
// records where each page starts and how many samples come before it. A seek
// then binary-searches that index, resets the decoder, and decodes forward
// from a page at least 400 ms before the target, throwing away samples until
// it reaches the target. RFC 7845 section 4.6 asks for at least 80 ms of
// pre-roll; on the sine fixture 80 ms still left errors of 0.03 twenty ms past
// the target, while 400 ms gives the straight decode's samples exactly. It
// costs a few ms of decoding per seek.
//
// Positions come from counting each packet's samples (its TOC byte, via
// opus_packet_get_nb_samples), not from the pages' granule positions. The count
// is exactly what the decoder will produce, so it stays right even for the
// packet that libogg drops after a reset (the tail of a packet that began on
// the page before), and for the last page, whose granule is trimmed.
//
// Links: a chained file (several Ogg Opus streams back to back, each with its
// own serial number) plays as one stream. Each link has its own OpusHead, so
// its own pre-skip and output gain. A later link whose channel count differs
// from the first one ends the stream there (rare; a mixer can't change channel
// count mid-stem).
//
// Straight-through output is bit-identical to the old whole-file decoder: the
// same packets go through the same opus_decode_float calls into a 120 ms
// (kMaxFrame) buffer, the pre-skip is dropped from each link's start, and nothing is
// scaled when the gain is zero.

#include "audio/stem_reader.h"

#include "audio/decode.h"
#include "audio/frames.h"
#include "core/little_endian.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <vector>

#include <ogg/ogg.h>
#include <opus.h>

namespace hydra::audio::detail {

namespace {

constexpr int kRate = 48000;           // Opus always decodes at 48 kHz
// 120 ms, the largest Opus packet, in frames at kRate (frames_of_ms owns the
// conversion).
const int kMaxFrame = static_cast<int>(frames_of_ms(120.0, kRate));
// 400 ms decoder warm-up before a seek target, in frames at kRate.
const int64_t kPreRoll = frames_of_ms(400.0, kRate);
constexpr uint64_t kProgressStep = 4ull << 20;  // report at least every 4 MB
constexpr std::size_t kMaxPageBytes = 27 + 255 + 255 * 255;

// The samples one packet decodes to, read from its leading bytes. A zero-byte
// packet counts as a full 120 ms buffer because opus_decode_float conceals a
// lost packet for the whole frame_size it is given (kMaxFrame); an invalid one
// counts 0 (the decoder will reject it and end the stem there).
int64_t packet_samples(const unsigned char* p, long bytes) {
    if (bytes <= 0) return kMaxFrame;
    int n = opus_packet_get_nb_samples(p, static_cast<opus_int32>(bytes), kRate);
    return n < 0 ? 0 : n;
}

struct PageEntry {
    uint64_t offset = 0;   // where "OggS" starts
    uint32_t length = 0;   // header + segment table + body
    int64_t start_g = 0;   // link samples (pre-skip included) of every audio
                           // packet that began before this page
};

struct Link {
    uint32_t serial = 0;
    std::size_t first_page = 0;  // its BOS page (index into pages)
    std::size_t audio_page = 0;  // first page after the two header packets
    std::size_t end_page = 0;    // one past its last page
    int channels = 0;
    int preskip = 0;
    int gain_q78 = 0;            // OpusHead output gain, signed Q7.8 dB
    int64_t decoded_total = 0;   // samples all its audio packets decode to
    int64_t last_granule = -1;   // last page's granule position
    int64_t end_g = 0;           // where reading stops, in link samples
    int64_t start_frame = 0;     // first stream frame this link supplies
    int64_t length = 0;          // frames it supplies (end_g - preskip)
};

// One pass over the bytes: the page index and the links. Throws on a stream
// whose first link isn't a mono/stereo OpusHead, as the old decoder did.
void build_index(const uint8_t* d, std::size_t size, const OpenProgress& progress,
                 std::vector<PageEntry>& pages, std::vector<Link>& links) {
    uint64_t next_report = kProgressStep;
    if (progress && !progress(0, size)) throw OpenCancelled();

    Link* link = nullptr;      // the link pages are being added to
    bool link_closed = false;  // its EOS page has been seen
    bool in_packet = false;    // the last segment was 255 (packet continues)
    int packets_seen = 0;      // packets completed in this link so far
    int64_t link_g = 0;        // samples of this link's audio packets so far

    std::size_t off = 0;
    while (off + 27 <= size) {
        if (progress && off >= next_report) {
            if (!progress(off, size)) throw OpenCancelled();
            while (next_report <= off) next_report += kProgressStep;
        }
        if (std::memcmp(d + off, "OggS", 4) != 0) {
            // Damaged or padded file: resync on the next capture pattern.
            std::size_t next = size;
            for (std::size_t i = off + 1; i + 4 <= size; ++i) {
                if (d[i] == 'O' && std::memcmp(d + i, "OggS", 4) == 0) {
                    next = i;
                    break;
                }
            }
            if (next == size) break;
            off = next;
            continue;
        }
        const uint8_t* h = d + off;
        const int nseg = h[26];
        if (off + 27 + nseg > size) break;
        std::size_t body = 0;
        for (int i = 0; i < nseg; ++i) body += h[27 + i];
        const std::size_t page_len = 27 + static_cast<std::size_t>(nseg) + body;
        if (off + page_len > size) break;  // truncated last page

        const uint8_t flags = h[5];
        const bool continued = (flags & 0x01) != 0;
        const bool bos = (flags & 0x02) != 0;
        const bool eos = (flags & 0x04) != 0;
        const int64_t granule = static_cast<int64_t>(core::read_le_u64(h + 6));
        const uint32_t serial = core::read_le_u32(h + 14);

        if (bos && (link == nullptr || link_closed || link->audio_page != 0)) {
            // A new link starts here (the first, or the next in a chain). A BOS
            // of another stream multiplexed beside a link's headers is ignored.
            links.push_back(Link{});
            link = &links.back();
            link->serial = serial;
            link->first_page = pages.size();
            link_closed = false;
            in_packet = false;
            packets_seen = 0;
            link_g = 0;
        }
        if (link == nullptr || serial != link->serial || link_closed) {
            off += page_len;
            continue;
        }

        PageEntry pe;
        pe.offset = off;
        pe.length = static_cast<uint32_t>(page_len);
        pe.start_g = link_g;
        pages.push_back(pe);

        // Walk the segment table: where each packet starts and ends.
        const uint8_t* seg = h + 27;
        const uint8_t* pos = h + 27 + nseg;
        int i = 0;
        if (continued && !in_packet) {
            // The packet's start was lost; libogg skips its tail, so do we.
            while (i < nseg && seg[i] == 255) pos += seg[i++];
            if (i < nseg) pos += seg[i++];
        } else if (!continued && in_packet) {
            in_packet = false;  // the unfinished packet was cut off
        }
        for (; i < nseg; ++i) {
            if (!in_packet) {
                // A packet starts here. The OpusHead is the link's first.
                if (packets_seen == 0) {
                    if (seg[i] < 19 || std::memcmp(pos, kOpusHeadTag, sizeof kOpusHeadTag - 1) != 0) {
                        if (links.size() == 1)
                            throw std::runtime_error("decode_audio: Opus stream has no OpusHead");
                        link->channels = -1;  // unusable link: the chain ends before it
                    } else {
                        link->channels = pos[9];
                        link->preskip = core::read_le_u16(pos + 10);
                        link->gain_q78 = static_cast<int16_t>(core::read_le_u16(pos + 16));
                    }
                } else if (packets_seen >= 2) {
                    const long first = seg[i] >= 2 ? 2 : seg[i];
                    link_g += packet_samples(pos, first);
                }
            }
            in_packet = seg[i] == 255;
            if (!in_packet) {
                ++packets_seen;
                if (packets_seen == 2) link->audio_page = pages.size();  // next page
            }
            pos += seg[i];
        }
        if (granule != -1) link->last_granule = granule;
        link->end_page = pages.size();
        link->decoded_total = link_g;
        if (eos) link_closed = true;
        off += page_len;
    }
    if (progress && !progress(size, size)) throw OpenCancelled();
}

class OpusReader final : public StemReader {
public:
    OpusReader(StemBytes bytes, const OpenProgress& progress, const OpusReaderOptions& opt)
        : bytes_(std::move(bytes)) {
        build_index(bytes_.data(), bytes_.size(), progress, pages_, links_);
        if (links_.empty())
            throw std::runtime_error("decode_audio: Opus stream has no OpusHead");
        channels_ = links_[0].channels;
        if (channels_ < 1 || channels_ > 2)
            throw std::runtime_error("decode_audio: only mono/stereo Opus is supported");

        // Keep the links that can play as one stream with the first.
        std::size_t keep = 0;
        int64_t total = 0;
        for (Link& l : links_) {
            if (l.channels != channels_ || l.audio_page == 0) break;
            const int64_t end = opt.trim_end && l.last_granule >= 0
                                    ? std::min(l.last_granule, l.decoded_total)
                                    : l.decoded_total;
            l.end_g = std::max<int64_t>(end, l.preskip);
            l.length = l.end_g - l.preskip;
            l.start_frame = total;
            total += l.length;
            ++keep;
        }
        links_.resize(keep);
        length_ = total;
        if (length_ <= 0) throw std::runtime_error("decode_audio: no Opus audio decoded");

        pcm_.resize(static_cast<std::size_t>(kMaxFrame) * channels_);
        page_buf_.resize(kMaxPageBytes);
        int err = 0;
        dec_ = opus_decoder_create(kRate, channels_, &err);
        if (err != OPUS_OK || dec_ == nullptr)
            throw std::runtime_error("decode_audio: opus_decoder_create failed");
        ogg_stream_init(&os_, static_cast<int>(links_[0].serial));
        start_link(0, links_[0].preskip, links_[0].first_page, 2, 0);
    }

    ~OpusReader() override {
        if (dec_ != nullptr) opus_decoder_destroy(dec_);
        ogg_stream_clear(&os_);
    }

    int channels() const override { return channels_; }
    int sample_rate() const override { return kRate; }
    int64_t length_frames() const override { return length_; }
    bool failed() const override { return failed_; }

    int64_t read(float* out, int64_t frames) override {
        int64_t done = 0;
        while (done < frames) {
            if (pcm_count_ > 0) {
                const int64_t n = std::min<int64_t>(pcm_count_, frames - done);
                const float* src = pcm_.data() + static_cast<std::size_t>(pcm_pos_) * channels_;
                float* dst = out + static_cast<std::size_t>(done) * channels_;
                const std::size_t count = static_cast<std::size_t>(n) * channels_;
                if (gain_ == 1.0f)
                    std::memcpy(dst, src, count * sizeof(float));
                else
                    for (std::size_t i = 0; i < count; ++i) dst[i] = src[i] * gain_;
                pcm_pos_ += static_cast<int>(n);
                pcm_count_ -= static_cast<int>(n);
                done += n;
                continue;
            }
            if (at_end_ || !decode_next()) break;
        }
        return done;
    }

    void seek(int64_t frame) override {
        frame = std::clamp<int64_t>(frame, 0, length_);
        pcm_count_ = 0;
        pcm_pos_ = 0;
        // No early return on failed_: a seek to before the damage plays again
        // from there (start_link resets the decoder and clears at_end_), while
        // failed_ stays set so decode_audio still reports the damaged stream.
        if (frame == length_) {
            at_end_ = true;
            return;
        }
        std::size_t li = 0;
        while (li + 1 < links_.size() && links_[li + 1].start_frame <= frame) ++li;
        const Link& l = links_[li];
        const int64_t target = frame - l.start_frame + l.preskip;

        // The last audio page whose first new packet starts at least the
        // pre-roll before the target. Its start_g is where decoding resumes.
        const int64_t want = std::max<int64_t>(0, target - kPreRoll);
        auto first = pages_.begin() + static_cast<std::ptrdiff_t>(l.audio_page);
        auto last = pages_.begin() + static_cast<std::ptrdiff_t>(l.end_page);
        auto it = std::upper_bound(first, last, want, [](int64_t v, const PageEntry& p) {
            return v < p.start_g;
        });
        const std::size_t page = static_cast<std::size_t>((it - pages_.begin()) - 1);
        start_link(li, target, page, 0, pages_[page].start_g);
    }

private:
    // Points decoding at `page` of link `li`: a fresh decoder state, `headers`
    // header packets still to skip, the next packet starting at link sample
    // `gpos`, and output kept from link sample `want` on.
    void start_link(std::size_t li, int64_t want, std::size_t page, int headers, int64_t gpos) {
        const Link& l = links_[li];
        link_ = li;
        next_page_ = page;
        headers_left_ = headers;
        gpos_ = gpos;
        want_g_ = std::max<int64_t>(want, l.preskip);
        gain_ = l.gain_q78 == 0
                    ? 1.0f
                    : static_cast<float>(std::pow(10.0, l.gain_q78 / (20.0 * 256.0)));
        pk_count_ = pk_next_ = 0;
        at_end_ = false;
        opus_decoder_ctl(dec_, OPUS_RESET_STATE);
        ogg_stream_reset_serialno(&os_, static_cast<int>(l.serial));
    }

    // Feeds the next page of the link to libogg and queues every packet it
    // completes. False when the link has no pages left.
    bool feed_page() {
        const Link& l = links_[link_];
        while (next_page_ < l.end_page) {
            const PageEntry& pe = pages_[next_page_++];
            // libogg checks each page's CRC as the old sync-layer decode did; a
            // damaged page is skipped. The check rewrites the header, so it runs
            // on a copy (the bytes may be a read-only map).
            std::memcpy(page_buf_.data(), bytes_.data() + pe.offset, pe.length);
            ogg_page og;
            og.header = page_buf_.data();
            og.header_len = 27 + page_buf_[26];
            og.body = page_buf_.data() + og.header_len;
            og.body_len = static_cast<long>(pe.length) - og.header_len;
            unsigned char crc[4];
            std::memcpy(crc, page_buf_.data() + 22, 4);
            ogg_page_checksum_set(&og);
            if (std::memcmp(crc, page_buf_.data() + 22, 4) != 0) continue;
            if (ogg_stream_pagein(&os_, &og) != 0) continue;
            pk_count_ = pk_next_ = 0;
            for (;;) {
                const int r = ogg_stream_packetout(&os_, &packets_[pk_count_]);
                if (r == 0) break;
                if (r == 1 && ++pk_count_ == packets_.size()) break;
            }
            return true;
        }
        return false;
    }

    // Decodes the next packet into pcm_ (moving to the next link when this one
    // runs out). False at the end of the stream or on a decode error.
    bool decode_next() {
        for (;;) {
            if (pk_next_ == pk_count_) {
                if (gpos_ < links_[link_].end_g && feed_page()) continue;
                if (link_ + 1 < links_.size()) {
                    const Link& n = links_[link_ + 1];
                    start_link(link_ + 1, n.preskip, n.first_page, 2, 0);
                    continue;
                }
                at_end_ = true;
                return false;
            }
            ogg_packet& op = packets_[pk_next_++];
            if (headers_left_ > 0) {
                --headers_left_;  // OpusHead (read by the index) and OpusTags
                continue;
            }
            const int n = opus_decode_float(dec_, op.packet, static_cast<opus_int32>(op.bytes),
                                            pcm_.data(), kMaxFrame, 0);
            if (n < 0) {
                failed_ = true;
                at_end_ = true;
                return false;
            }
            const int64_t begin = gpos_;
            gpos_ += n;
            const int64_t keep_from = std::max(begin, want_g_);
            const int64_t keep_to = std::min(gpos_, links_[link_].end_g);
            if (keep_to > keep_from) {
                pcm_pos_ = static_cast<int>(keep_from - begin);
                pcm_count_ = static_cast<int>(keep_to - keep_from);
                return true;
            }
            if (begin >= links_[link_].end_g) pk_next_ = pk_count_;  // link done
        }
    }

    StemBytes bytes_;
    std::vector<PageEntry> pages_;
    std::vector<Link> links_;
    int channels_ = 0;
    int64_t length_ = 0;

    OpusDecoder* dec_ = nullptr;
    ogg_stream_state os_{};
    std::vector<float> pcm_;               // one decoded packet
    std::vector<unsigned char> page_buf_;  // the page being fed (CRC check copy)
    std::array<ogg_packet, 512> packets_{};
    std::size_t pk_count_ = 0, pk_next_ = 0;

    std::size_t link_ = 0;
    std::size_t next_page_ = 0;
    int headers_left_ = 0;
    int64_t gpos_ = 0;     // link sample where the next packet starts
    int64_t want_g_ = 0;   // first link sample to output
    float gain_ = 1.0f;
    int pcm_pos_ = 0, pcm_count_ = 0;
    bool at_end_ = false;
    bool failed_ = false;
};

}  // namespace

std::unique_ptr<StemReader> open_opus_reader(StemBytes bytes, const OpenProgress& progress,
                                             const OpusReaderOptions& options) {
    return std::make_unique<OpusReader>(std::move(bytes), progress, options);
}

}  // namespace hydra::audio::detail
