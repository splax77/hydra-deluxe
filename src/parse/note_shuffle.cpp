// Clone Hero 1.1's Note Shuffle: see note_shuffle.h.
//
// Each step below follows docs/audit/note-shuffle/probes/capped/
// shuffle_ref_capped.py (the drums path of `shuffle`), which names the game
// addresses it was read from. The names here match the reference's.

#include "parse/note_shuffle.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "core/model.h"

namespace hydra {

namespace {

// The game's drum lane bits (the reference's docstring).
constexpr uint32_t kKickBit = 0x1;
constexpr uint32_t kRedBit = 0x2;

uint32_t lane_bit(const ChordNote& note) {
    switch (note.colortype) {
        case NoteColor::Kick: return kKickBit;
        case NoteColor::Red: return kRedBit;
        case NoteColor::Yellow: return note.is_cymbal() ? 0x20u : 0x04u;
        case NoteColor::Blue: return note.is_cymbal() ? 0x40u : 0x08u;
        case NoteColor::Green: return note.is_cymbal() ? 0x80u : 0x10u;
    }
    throw std::logic_error("note shuffle: a note with no lane");
}

// The note `note` becomes on lane `bit`: same ghost or accent, colour and
// cymbal flag from the lane.
ChordNote moved_to(const ChordNote& note, uint32_t bit) {
    ChordNote out = note;
    out.cymbaltype = NoteCymbalType::Normal;
    switch (bit) {
        case 0x02: out.colortype = NoteColor::Red; break;
        case 0x04: out.colortype = NoteColor::Yellow; break;
        case 0x08: out.colortype = NoteColor::Blue; break;
        case 0x10: out.colortype = NoteColor::Green; break;
        case 0x20: out.colortype = NoteColor::Yellow; set_lane_flag(out); break;
        case 0x40: out.colortype = NoteColor::Blue; set_lane_flag(out); break;
        case 0x80: out.colortype = NoteColor::Green; set_lane_flag(out); break;
        default: throw std::logic_error("note shuffle: no pad lane " + std::to_string(bit));
    }
    return out;
}

// The reference's `pads`: the lanes the lanes in m block, each colour in both
// its tom and its cymbal form.
uint32_t pads(uint32_t m) {
    const uint32_t cx = ((((m >> 3) & 0xFFFDu) | m) & 0x1Eu) & 0xFFFFu;
    return (cx | ((cx & 0x1Cu) << 3)) & 0xFFFFu;
}

// The reference's lane index generator (xorshift128+, `lane_index`).
class LaneGenerator {
public:
    LaneGenerator(uint64_t seed, int n_lanes) : s0_(seed << 3), s1_(seed >> 3), n_lanes_(n_lanes) {}

    uint32_t next_lane_index() {
        uint64_t x = s0_;
        const uint64_t y = s1_;
        x ^= x << 23;
        const uint64_t s1 = x ^ y ^ (x >> 17) ^ (y >> 26);
        s0_ = y;
        s1_ = s1;
        const uint64_t r = (y + s1) & 0x7FFFFFFFu;
        return static_cast<uint32_t>(((r * static_cast<uint64_t>(n_lanes_ - 1)) >> 31) + 1);
    }

    // A zero state stays zero and only ever gives index 1, the red pad.
    bool state_is_zero() const { return s0_ == 0 && s1_ == 0; }

private:
    uint64_t s0_;
    uint64_t s1_;
    int n_lanes_;
};

// The reference's bits_low_to_high, indexed: the idx-th set bit of x counting
// from the lowest, or 0 when x has fewer.
uint32_t nth_bit_low_to_high(uint32_t x, size_t idx) {
    for (size_t i = 0; x != 0; ++i) {
        const uint32_t low = x & (~x + 1u);
        if (i == idx) return low;
        x ^= low;
    }
    return 0;
}

// Where `bit` sits among the set bits of x, counting from the lowest;
// kNoIndex when x lacks it.
constexpr size_t kNoIndex = static_cast<size_t>(-1);
size_t index_low_to_high(uint32_t x, uint32_t bit) {
    for (size_t i = 0; x != 0; ++i) {
        const uint32_t low = x & (~x + 1u);
        if (low == bit) return i;
        x ^= low;
    }
    return kNoIndex;
}

struct GameNote {
    size_t ts;  // index into the sequence
    ChordNote note;
    uint32_t mask;
    int64_t tick;
};

}  // namespace

NoteShuffleResult apply_note_shuffle(std::vector<SongTimestamp>& sequence, bool pro) {
    // The game's note list: one note per lane, chords in tick order, each
    // chord's notes in Chord::note_list's order.
    std::vector<GameNote> notes;
    for (size_t i = 0; i < sequence.size(); ++i) {
        for (const ChordNote& n : sequence[i].chord.note_list())
            notes.push_back({i, n, lane_bit(n), sequence[i].timecode.ticks()});
    }

    // Pro Drums on is the reference's instrument 9, off is instrument 6.
    const int n_lanes = pro ? 8 : 5;
    const uint32_t drawable = ((1u << n_lanes) - 1u) & ~kKickBit;

    uint64_t seed = 0;
    for (size_t i = 0; i < notes.size() && i < 4; ++i)
        seed += static_cast<uint64_t>(notes[i].mask) * static_cast<uint64_t>(notes[i].tick);
    LaneGenerator gen(seed, n_lanes);

    std::vector<uint32_t> out_mask(notes.size());
    int64_t prev_tick = 0;
    uint32_t cur_pat = 0, prev_pat = 0, cur_out = 0, prev_out = 0;
    for (size_t k = 0; k < notes.size(); ++k) {
        const GameNote& nt = notes[k];
        out_mask[k] = nt.mask;
        if (nt.mask & kKickBit) continue;
        const bool same_chord = nt.tick == prev_tick;
        uint32_t pat = cur_pat;
        if (!same_chord) {
            pat = 0;
            for (const ChordNote& c : sequence[nt.ts].chord.note_list()) pat |= lane_bit(c);
            pat &= 0xFEu;
        }
        uint32_t out = 0;
        if (prev_pat != 0 && prev_pat == pat) {
            const size_t idx = index_low_to_high(pat, nt.mask);
            if (idx != kNoIndex) out = nth_bit_low_to_high(prev_out, idx);
            if (out == 0 || (same_chord && (out & pads(cur_out))))
                throw std::logic_error("note shuffle: the copy rule gave tick " + std::to_string(nt.tick) +
                                       (out == 0 ? " a note with no pad" : " two notes of one colour"));
        } else {
            const uint32_t used = pads(cur_out);
            if ((used & drawable) == drawable) return NoteShuffleResult::GameFreezes;
            if (gen.state_is_zero() && (used & kRedBit)) return NoteShuffleResult::GameFreezes;
            // No counter: from a non-zero state the generator reaches every
            // pad (docs/audit/note-shuffle/probes/reach/reach_check.py).
            do {
                out = (1u << (gen.next_lane_index() & 31u)) & 0xFFFFu;
            } while (out & used);
        }
        if (same_chord) {
            cur_out |= out;
        } else {
            prev_out = cur_out;
            cur_out = out;
            prev_pat = cur_pat;
            cur_pat = pat;
        }
        out_mask[k] = out;
        prev_tick = nt.tick;
    }

    std::vector<Chord> chords(sequence.size());
    for (size_t k = 0; k < notes.size(); ++k) {
        const GameNote& nt = notes[k];
        const ChordNote placed = (nt.mask & kKickBit) ? nt.note : moved_to(nt.note, out_mask[k]);
        if (chords[nt.ts].at(placed.colortype))
            throw std::logic_error("note shuffle: two notes of one colour at tick " + std::to_string(nt.tick));
        chords[nt.ts].insert_note(placed);
    }
    for (size_t i = 0; i < sequence.size(); ++i) sequence[i].chord = chords[i];
    return NoteShuffleResult::Shuffled;
}

}  // namespace hydra
