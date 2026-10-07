// The digests that prove a change left Hydra's output identical. One owner,
// shared by hydra_bench's --engine and --parse modes (tools/bench.cpp) and the
// test that pins the corpus's digests (tests/test_perf_digest.cpp), so the
// tool and the test can never hash differently.
//
// row_hash covers what one chart stores in its results row: the best path,
// the path tree and its nodes, and the summary columns. song_digest covers a
// parsed Song: the timing maps, every timestamp's chord and flags, and the
// display fields. Both are FNV-1a over the raw bytes, so they are stable
// within one build of one compiler, which is all an A-against-B comparison
// needs. The speedups plan's "Proving identical results" section is the
// recipe they serve.

#ifndef HYDRA_TESTS_SONG_DIGEST_H
#define HYDRA_TESTS_SONG_DIGEST_H

#include <cstddef>
#include <cstdint>
#include <exception>
#include <optional>
#include <string>
#include <typeinfo>

#include "parse/song.h"
#include "store/record_store.h"

namespace hydra::digest {

// FNV-1a's 64-bit offset basis and prime.
inline constexpr uint64_t kSeed = 1469598103934665603ULL;
inline constexpr uint64_t kPrime = 1099511628211ULL;

inline uint64_t fnv(uint64_t h, const void* p, size_t n) {
    const unsigned char* b = static_cast<const unsigned char*>(p);
    for (size_t i = 0; i < n; ++i) {
        h ^= b[i];
        h *= kPrime;
    }
    return h;
}

// A string's length first, so "ab"+"c" and "a"+"bc" hash apart.
inline uint64_t fnv_str(uint64_t h, const std::string& s) {
    const uint64_t n = s.size();
    h = fnv(h, &n, sizeof n);
    return fnv(h, s.data(), s.size());
}

// Whether the value is set, then the value.
template <class T>
inline uint64_t fnv_opt(uint64_t h, const std::optional<T>& o) {
    const unsigned char has = o.has_value();
    h = fnv(h, &has, 1);
    if (o) h = fnv(h, &*o, sizeof(T));
    return h;
}

// Adds one chart's hash to a running corpus digest (which starts at kSeed).
inline uint64_t fold(uint64_t all, uint64_t chart) { return fnv(all, &chart, sizeof chart); }

// One chart's stored result row: bestpath, the structure blob, every node's
// hash and payload, and the summary columns. The key columns (hash,
// chartmode, cap, lens) are left out: they come from the settings, not the
// analysis.
inline uint64_t row_hash(const store::PreparedRow& r) {
    uint64_t h = kSeed;
    h = fnv_str(h, r.bestpath);
    uint64_t n = r.structure.size();
    h = fnv(h, &n, sizeof n);
    h = fnv(h, r.structure.data(), r.structure.size());
    for (const auto& node : r.nodes) {
        h = fnv_str(h, node.hash);
        n = node.payload.size();
        h = fnv(h, &n, sizeof n);
        h = fnv(h, node.payload.data(), node.payload.size());
    }
    const store::PathSummary& s = r.summary;
    h = fnv_opt(h, s.score);
    h = fnv_opt(h, s.actcount);
    h = fnv_opt(h, s.maxskip);
    h = fnv_opt(h, s.hardest_ms);
    h = fnv_opt(h, s.avgmult);
    h = fnv_opt(h, s.notecount);
    h = fnv_opt(h, s.sqin_count);
    h = fnv_opt(h, s.sqout_count);
    h = fnv_opt(h, s.pathcount);
    h = fnv_opt(h, s.stars);
    return h;
}

// One parsed Song: resolution, the tempo, meter and time-signature maps,
// every timestamp (its timecode in all four forms, each lane's note, the solo
// and SP flags, the activation length and phrase start), the features, the
// dynamics tag fields, the offset, the practice sections and the solo
// sections.
inline uint64_t song_digest(const Song& s) {
    uint64_t h = kSeed;
    auto v = [&](const auto& x) { h = fnv(h, &x, sizeof x); };
    v(s.tick_resolution());
    for (const auto& [k, x] : s.tpm_changes) {
        v(k);
        v(x);
    }
    for (const auto& [k, x] : s.bpm_changes) {
        v(k);
        v(x);
    }
    for (const auto& [k, x] : s.timesig_changes) {
        v(k);
        v(x.first);
        v(x.second);
    }
    const uint64_t n = s.sequence.size();
    v(n);
    for (const SongTimestamp& ts : s.sequence) {
        v(ts.timecode.ticks());
        const double ms = ts.timecode.ms(), md = ts.timecode.measures_decimal();
        v(ms);
        v(md);
        for (int i = 0; i < 3; ++i) v(ts.timecode.measure_beats_ticks()[i]);
        for (NoteColor c : {NoteColor::Kick, NoteColor::Red, NoteColor::Yellow, NoteColor::Blue,
                            NoteColor::Green}) {
            const std::optional<ChordNote>& o = ts.chord.at(c);
            const unsigned char has = o.has_value();
            v(has);
            if (o) {
                v(o->colortype);
                v(o->dynamictype);
                v(o->cymbaltype);
                v(o->is2x);
            }
        }
        v(ts.flag_solo);
        v(ts.flag_sp);
        h = fnv_opt(h, ts.activation_length);
        h = fnv_opt(h, ts.sp_phrase_start);
    }
    for (const auto& f : s.features) h = fnv_str(h, f);
    v(s.dynamics_enabled);
    h = fnv_opt(h, s.dynamics_late_tag_tick);
    v(s.dynamics_marks_before_tag);
    h = fnv_opt(h, s.chart_offset_s);
    for (const auto& p : s.practice_sections) {
        v(p.tick);
        h = fnv_str(h, p.name);
    }
    for (const auto& p : s.solo_sections) {
        v(p.first);
        v(p.last);
    }
    return h;
}

// A parsed chart's digest with its stored dynamics blob added (none when the
// chart stores no dynamics row).
inline uint64_t with_dynamics(uint64_t song_hash, const std::optional<store::DynamicsEntry>& e) {
    return e ? fnv(song_hash, e->blob.data(), e->blob.size()) : song_hash;
}

// A chart that fails to parse is hashed by its exception's type and message,
// so a change that fails a chart differently shows up too.
inline std::string failure_text(const std::exception& e) {
    return std::string(typeid(e).name()) + ": " + e.what();
}
inline uint64_t failure_hash(const std::string& text) { return fnv_str(kSeed, text); }

}  // namespace hydra::digest

#endif  // HYDRA_TESTS_SONG_DIGEST_H
