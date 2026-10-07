// The digests that prove a change left Hydra's output identical. One owner,
// shared by hydra_bench's --engine and --parse modes (tools/bench.cpp) and the
// tests that pin digests (tests/test_perf_digest.cpp, and the crafted edge
// files in tests/test_song.cpp), so the tool and the tests can never hash
// differently.
//
// row_hash covers one chart's answer: the whole engine result (record_hash)
// and its results row's best path and summary columns. song_digest covers a
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
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/dynamics_breakdown.h"
#include "core/model.h"
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

// A list's length first, then each element through `each`.
template <class T, class F>
inline uint64_t fnv_list(uint64_t h, const std::vector<T>& xs, F each) {
    const uint64_t n = xs.size();
    h = fnv(h, &n, sizeof n);
    for (const T& x : xs) h = each(h, x);
    return h;
}

inline uint64_t fnv_scale(uint64_t h, const std::optional<TransferScale>& s) {
    const unsigned char has = s.has_value();
    h = fnv(h, &has, 1);
    if (s) {
        h = fnv(h, &s->early, sizeof s->early);
        h = fnv(h, &s->late, sizeof s->late);
    }
    return h;
}

// One activation, field by field: its tick and chord, the frontend points,
// the backend rows the details view shows (Activation::display_backends),
// every squeeze with its offset and transfer scale, the early-fill offset,
// the SP end's transfer scale, the squeezed-out tick, the SP-end history,
// the bank arrivals and the passed-over fills.
inline uint64_t activation_hash(uint64_t h, const Activation& a) {
    auto v = [&](const auto& x) { h = fnv(h, &x, sizeof x); };
    v(a.timecode.ticks());
    h = fnv_str(h, a.chord.code());
    v(a.frontend_points);
    h = fnv_list(h, a.display_backends(), [](uint64_t g, const BackendSqueeze& b) {
        const int64_t tick = b.timecode.ticks();
        g = fnv(g, &tick, sizeof tick);
        g = fnv_str(g, b.chord.code());
        g = fnv(g, &b.points, sizeof b.points);
        g = fnv(g, &b.sqout_points, sizeof b.sqout_points);
        return fnv_opt(g, b.offset_ms);
    });
    h = fnv_list(h, a.sqinouts, [](uint64_t g, const SPSqueeze& q) {
        g = fnv(g, &q.kind, sizeof q.kind);
        g = fnv(g, &q.offset_ms, sizeof q.offset_ms);
        return fnv_scale(g, q.transfer);
    });
    v(a.e_offset);
    h = fnv_scale(h, a.transfer_post);
    h = fnv_opt(h, a.sqout_tick);
    h = fnv_list(h, a.sp_end_steps, [](uint64_t g, const SpEndStep& s) {
        g = fnv(g, &s.tick, sizeof s.tick);
        g = fnv(g, &s.end_tick, sizeof s.end_tick);
        return fnv(g, &s.kind, sizeof s.kind);
    });
    auto ticks = [](uint64_t g, const int64_t& t) { return fnv(g, &t, sizeof t); };
    h = fnv_list(h, a.bank_rise_ticks, ticks);
    h = fnv_list(h, a.skipped_fill_ticks, ticks);
    return h;
}

// One path and its variants, depth first: its activations and variant tail,
// note count, trailing bank, score split, tie count and variation point.
inline uint64_t path_hash(uint64_t h, const Path& p) {
    auto v = [&](const auto& x) { h = fnv(h, &x, sizeof x); };
    h = fnv_list(h, p.activations, activation_hash);
    h = fnv_list(h, p.variant_tail, activation_hash);
    v(p.notecount);
    h = fnv_list(h, p.trailing_bank_ticks,
                 [](uint64_t g, const int64_t& t) { return fnv(g, &t, sizeof t); });
    v(p.score_base);
    v(p.score_combo);
    v(p.score_sp);
    v(p.score_solo);
    v(p.score_accents);
    v(p.score_ghosts);
    v(p.tied_count);
    h = fnv_opt(h, p.var_point);
    return fnv_list(h, p.variants, path_hash);
}

// A whole engine result, field by field from the structs (R16): the record's
// rules fingerprint, ms limit, SP cap and fill rule, its multiplier
// squeezes, and every path tree, the all-0 ones included. Two records that
// hash the same are the same answer. hydra.db no longer stores any of this
// (D87), so this walk, not a store round trip, is how a test proves a change
// left the engine's whole answer alone.
inline uint64_t record_hash(const HydraRecord& r) {
    uint64_t h = kSeed;
    auto v = [&](const auto& x) { h = fnv(h, &x, sizeof x); };
    v(r.rules_fingerprint);
    h = fnv_opt(h, r.ms_limit);
    h = fnv_opt(h, r.sp_cap);
    v(r.sp_cap_converged);
    v(r.legacy_fills);
    h = fnv_list(h, r.multsqueezes, [](uint64_t g, const MultSqueeze& m) {
        g = fnv_str(g, m.chord().code());
        const int combo = m.combo();
        return fnv(g, &combo, sizeof combo);
    });
    h = fnv_list(h, r.paths, path_hash);
    return fnv_list(h, r.allzero_paths, path_hash);
}

// One chart's answer: its results row (bestpath and the summary columns) and
// the whole engine result behind it (record_hash). The row's identity and
// build stamp (see store::PreparedRow) are left out: they come from the
// settings and the build, not from the analysis.
inline uint64_t row_hash(const store::PreparedRow& r, const HydraRecord& record) {
    uint64_t h = record_hash(record);
    h = fnv_str(h, r.bestpath);
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

// A parsed chart's digest with its dynamics count added (app::count_dynamics:
// the enabled flag, every row's three counts, and the late tag's fields).
inline uint64_t with_dynamics(uint64_t song_hash, const app::DynamicsBreakdown& b) {
    uint64_t h = song_hash;
    auto v = [&](const auto& x) { h = fnv(h, &x, sizeof x); };
    v(b.dynamics_enabled);
    for (const app::DynamicsCounts& c : b.rows) {
        v(c.ghost);
        v(c.accent);
        v(c.normal);
    }
    h = fnv_opt(h, b.late_tag_ms);
    v(b.marks_before_tag);
    return h;
}

// A chart that fails to parse is hashed by its exception's type and message,
// so a change that fails a chart differently shows up too.
inline std::string failure_text(const std::exception& e) {
    return std::string(typeid(e).name()) + ": " + e.what();
}
inline uint64_t failure_hash(const std::string& text) { return fnv_str(kSeed, text); }

// One chart's parse digest, the per-chart hash hydra_bench --parse folds: the
// parsed song with its dynamics count (with_dynamics), or its failure's hash. `fail`,
// when given, gets the failure text, and stays as it was when the chart
// parses. The tests call this; tools/bench.cpp keeps its own loop because it
// times the parse and the dynamics pass apart.
inline uint64_t chart_parse_hash(const std::string& notespath, const app::AnalysisSettings& st,
                                 std::string* fail = nullptr) {
    try {
        const Song song = load_songpath_with_notes(notespath, st.prodrums, st.bass2x,
                                                   st.difficulty, st.rules);
        const uint64_t h = song_digest(song);
        // The count needs the 2x kicks kept (app::analysis_parse_counts_dynamics).
        return app::analysis_parse_counts_dynamics(st.bass2x)
                   ? with_dynamics(h, app::count_dynamics(song))
                   : h;
    } catch (const std::exception& e) {
        const std::string text = failure_text(e);
        if (fail) *fail = text;
        return failure_hash(text);
    }
}

// The settings a digest is pinned under, built the way the app builds them
// from an ini that sets only these three things. A test that pins more
// fields sets them on the result before it calls batch_run.
inline app::Settings digest_settings(const char* difficulty, bool pro, bool bass2x) {
    app::Settings s;
    s.view_difficulty = difficulty;
    s.view_prodrums = pro;
    s.view_bass2x = bass2x;
    return s;
}

}  // namespace hydra::digest

#endif  // HYDRA_TESTS_SONG_DIGEST_H
