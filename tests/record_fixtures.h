// Hand-built songs and record fixtures shared by the test files.
//
// Two kinds of thing live here. The songs are small charts built to give the
// search one exact shape (a late squeeze-in, an early squeeze-out, a clamp).
// The fixture helpers are where a test writes a single stored fact onto a
// hand-built Activation or Path, so when that fact changes shape only the
// helper's body changes. The exception is a multi-step SP-end history: a test
// that needs one assigns `sp_end_steps` directly, writing the steps as literal
// ticks. A few small test helpers several files share sit at the end.
#ifndef HYDRA_TESTS_RECORD_FIXTURES_H
#define HYDRA_TESTS_RECORD_FIXTURES_H

#include <algorithm>
#include <cstdint>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "doctest.h"

#include "app/config.h"
#include "core/model.h"
#include "parse/song.h"
#include "search/engine.h"
#include "search/pather.h"

namespace hydra::test {

// ---- songs ----------------------------------------------------------------

struct FixtureNote {
    int64_t tick;
    bool phrase = false;
    int64_t fill_length = 0;  // 0 = no fill ends here
};

// One red note per entry, flat tempo and 4/4.
inline Song build_fixture_song(int resolution, double bpm, const std::vector<FixtureNote>& notes) {
    Song song(resolution);
    song.bpm_changes[0] = bpm;
    song.build_timing();
    for (const FixtureNote& n : notes) {
        SongTimestamp ts;
        ts.timecode = song.timecode(n.tick);
        ts.chord.add_note(NoteColor::Red);
        if (n.phrase) {
            ts.flag_sp = true;
            ts.sp_phrase_start = n.tick >= resolution ? n.tick - resolution : 0;
        }
        if (n.fill_length > 0) ts.activation_length = n.fill_length;
        song.sequence.push_back(std::move(ts));
    }
    return song;
}

// A note on every beat from 0 to `last`, with the phrase ends and fills
// given. 240 BPM in 4/4 at 480 ticks a beat: a measure is 1920 ticks and
// 1000 ms, and one SP bar (two measures) burns 2000 ms. A note every 250 ms.
inline Song beat_song(const std::vector<int64_t>& phrases,
                      const std::vector<int64_t>& fills, int64_t last) {
    std::vector<FixtureNote> notes;
    for (int64_t t = 0; t <= last; t += 480) {
        FixtureNote n{t};
        n.phrase = std::find(phrases.begin(), phrases.end(), t) != phrases.end();
        if (std::find(fills.begin(), fills.end(), t) != fills.end()) n.fill_length = 960;
        notes.push_back(n);
    }
    return build_fixture_song(480, 240.0, notes);
}

// The Epidermis shape (finding 5). Phrases end at 480 and 1920, so two bars
// are banked by 1000 ms. The fill at 5760 (3000 ms) activates, and two bars
// run four measures, so the plain SP end is 13440 (7000 ms). The third
// phrase ends at 13920 (7250 ms), a quarter measure after that end. The
// player squeezes it in by hitting it early. The engine extends SP one bar
// from the old end, to 17280 (9000 ms), so at the phrase the meter holds
// 1 - 0.25 / 2 = 0.875 bars.
inline Song make_late_sqin_song() {
    return beat_song({480, 1920, 13920}, {5760}, 21120);
}

// An early squeeze-out: the third phrase ends at 12960 (6750 ms), inside the
// first window. Hit late, it lands after SP ends and banks one bar instead.
// The fourth phrase, at 14400 (7500 ms), makes two bars for the second fill
// at 17280 (9000 ms). Search it with target ticks {5760, 17280}: only the
// squeeze-out path can take that second fill.
inline Song make_early_sqout_song() {
    return beat_song({480, 1920, 12960, 14400}, {5760, 17280}, 26880);
}

// Finding 52's case, at 120 BPM with 480 ticks a beat (tick t is t*500/480
// ms). Phrases end at 960 (1000 ms) and 18144 (18900 ms): SP is ready at
// 18900 ms. Fill A ends at 19200 (20000 ms) and is 480 ticks (500 ms) long.
// Fill B ends at 24960 (26000 ms) and is 3456 ticks (3600 ms) long. Fill C
// ends at 28800 (30000 ms). Under the 1.0 rule A's deadline is 18968.75 ms
// and B's is 18768.75 ms: A is shown and passed over, B never spawns. Search
// it with FillDeadlineRule::Ch10 and target ticks {28800}.
inline Song make_ch10_fill_song() {
    std::vector<FixtureNote> notes;
    for (int64_t t = 0; t <= 38400; t += 960) {
        FixtureNote n{t};
        n.phrase = t == 960;
        if (t == 19200 || t == 28800) n.fill_length = 480;
        if (t == 24960) n.fill_length = 3456;
        notes.push_back(n);
    }
    notes.push_back({18144, true, 0});
    std::sort(notes.begin(), notes.end(),
              [](const FixtureNote& a, const FixtureNote& b) { return a.tick < b.tick; });
    return build_fixture_song(480, 120.0, notes);
}

// A note for the 192-ticks-a-beat songs below: a phrase end, a fill end
// (384 ticks long), or a plain note.
struct TailNote {
    int64_t tick;
    bool sp_phrase = false;
    bool activation = false;
};

// 192 ticks a beat, 768 a measure, with the tempo map given.
inline Song build_tempo_song(const std::vector<TailNote>& notes,
                             const std::map<int64_t, double>& bpm) {
    Song song(192);
    song.tpm_changes[0] = 768;
    for (const auto& kv : bpm) song.bpm_changes[kv.first] = kv.second;
    song.build_timing();
    for (const TailNote& n : notes) {
        SongTimestamp ts;
        ts.timecode = song.timecode(n.tick);
        ts.chord.add_note(NoteColor::Red);
        ts.flag_sp = n.sp_phrase;
        if (n.activation) ts.activation_length = 384;
        song.sequence.push_back(ts);
    }
    return song;
}

// build_tempo_song at a flat 120 BPM.
inline Song build_tail_song(const std::vector<TailNote>& notes) {
    return build_tempo_song(notes, {{0, 120.0}});
}

// The SqIn shape finding 27 is about. Two phrases bank 2 bars; the fill at
// 2304 activates, so SP ends at X = 2304 + 4 measures = 5376. The phrase at
// 5280 sits 250 ms before X: collecting it is the SqIn, and moves the end to
// 6912. The phrase at 6144 is then collected mid-SP and moves it to 8448.
// The tempo halves at 6000, so X (120 BPM) and D (60 BPM) differ, and the old
// "D minus two measures" rebuild (6912, 60 BPM) is wrong.
inline Song sqin_then_collect_song() {
    return build_tempo_song({{0, true},  {768, true},  {1536},       {2304, false, true},
                             {3072},     {3840},       {4608},       {5280, true},
                             {5376},     {6144, true}, {6912},       {7680},
                             {8448},     {8544}},
                            {{0, 120.0}, {6000, 60.0}});
}

// The clamp shape of finding 28, at an SP cap of 2. The meter is full when
// the activation at 2304 starts, so collecting the phrase at 3072 clamps the
// end to plusmeasure(3072, 4) = 6144 instead of 6912. The tempo halves at
// 2700, between the activation and the collecting note.
inline Song clamp_song() {
    return build_tempo_song({{0, true}, {768, true}, {1536}, {2304, false, true},
                             {3072, true}, {3840}, {4608}, {5376}, {6144}, {6912}},
                            {{0, 120.0}, {2700, 60.0}});
}

// A squeeze window that reaches back past the activation. At 4000 BPM a
// measure (768 ticks) lasts 60 ms, so the 500 ms squeeze window spans more
// than eight measures. Phrases at 0, 768 and 1536 bank three bars before the
// fill at 2304 (180 ms) activates, so SP ends at 2304 + 6 measures = 6912
// (540 ms). The SP end two measures earlier, 5376 (420 ms), has the phrase at
// 0 inside its window; the one at 6912 has the phrase at 768. Both phrases
// were banked before SP started, so this activation can squeeze neither in
// nor out: its SP simply ends at 6912.
inline Song banked_phrase_window_song() {
    return build_tempo_song({{0, true},     {768, true},  {1536, true}, {2304, false, true},
                             {3072},        {3840},       {4608},       {5376},
                             {6144},        {6912},       {7680},       {8448},
                             {9216}},
                            {{0, 4000.0}});
}

// Two activations that tie on one SP end but not on what they banked. 120 BPM
// up to tick 9600, then 4000 BPM (a measure is 60 ms). Phrases end at 768,
// 2304, 8448 and 12288; fills end at 4608, 10752 and 13824. Path '1' banks
// three bars, activates at 10752 (SP end 15360) and collects 12288, so its
// end moves to 16896. Path '0 E0' activates at 4608, banks 8448 and 12288 and
// activates at 13824 with two bars: SP end 16896 too, with the same score at
// 13824. The SP end at 15360 has 12288 in its window. '1' can squeeze it out
// there; '0 E0' banked it before its activation and cannot, so the two must
// not be folded into one future. The trimmed review probe probe1.
inline Song banked_phrase_fold_song() {
    return build_tempo_song({{768, true},   {2304, true},  {4608, false, true}, {6144},
                             {8448, true},  {10752, false, true}, {12288, true},
                             {13824, false, true}, {17000}},
                            {{0, 120.0}, {9600, 4000.0}});
}

// Two paths that meet with the same SP meter but not the same ready time.
// 120 BPM; search it at an SP cap of 2. Phrases at 0 and 768 make SP ready at
// 2000 ms. Path A activates at the fill at 2304 (few notes under it, SP ends
// at 5376), then banks the phrases at 6144 and 6912: ready again at 18000 ms.
// Path B passes that fill; the cap wastes those two phrases, so it stays
// ready from 2000 ms. A fill's deadline is 6 beats before its end (fills are
// 384 ticks), so the fills at 7296 (16000 ms) and 7680 (17000 ms) both
// refuse A and spawn for B. At 7296 both hold 2 bars and A leads on score,
// as B's twin that passed that fill. Sixteenth notes run from 7728 to 10752:
// the last eight sit under an activation at 7680 (SP ends at 10752) but not
// one at 7296 (SP ends at 10368), so B activating at 7680 is the best path.
inline Song ready_time_fold_song() {
    std::vector<TailNote> notes{{0, true},    {768, true},  {1536},       {2304, false, true},
                                {3072},       {3840},       {4608},       {5376},
                                {6144, true}, {6912, true}, {7296, false, true},
                                {7680, false, true}};
    for (int64_t t = 7728; t <= 10752; t += 48) notes.push_back({t});
    notes.push_back({11520});
    return build_tail_song(notes);
}

// Keep every path, not only the best score (EngineOptions' default depth is
// 0), so a SqOut or SqIn branch the fixture creates is in the output even
// when it is not optimal.
inline EngineOptions wide_search() {
    EngineOptions o;
    o.depth_mode = DepthMode::Points;
    o.depth_value = kKeepEveryPathBand;
    return o;
}

// ---- lone pricing (decision D3) --------------------------------------------

// Every window of a path written out: activation, SP-end steps, squeezes,
// squeezed-out note, backend rows, the bars it spends, and the early-fill
// facts (D38); then the bars banked after the last window. Doubles are
// written in full, so two paths with the same text store the same facts.
inline std::string windows_text(const Path& p) {
    std::ostringstream o;
    o.precision(17);
    for (const Activation& a : p.walk_activations()) {
        o << a.timecode.ticks() << " [steps";
        for (const SpEndStep& s : a.sp_end_steps)
            o << ' ' << s.tick << '>' << s.end_tick << ':' << static_cast<int>(s.kind);
        o << " | sq";
        for (const SPSqueeze& q : a.sqinouts) o << ' ' << q.symbol() << q.offset_ms;
        o << " | out " << a.sqout_tick.value_or(-1) << " | backends";
        for (const BackendSqueeze& b : a.backends)
            o << ' ' << b.timecode.ticks() << '/' << b.points << '/' << b.sqout_points << '/'
              << (b.offset_ms ? *b.offset_ms : -1.0);
        o << " | bank";
        for (const int64_t t : a.bank_rise_ticks) o << ' ' << t;
        o << " | e " << a.e_offset << " | passed";
        for (const int64_t t : a.skipped_fill_ticks) o << ' ' << t;
        o << "] ";
    }
    o << "trailing";
    for (const int64_t t : p.trailing_bank_ticks) o << ' ' << t;
    return o.str();
}

// What a search pricing a variant's activations alone (search_target) says
// about it. A lone match is a root of that search with the variant's total
// and windows_text: only a root the engine itself left unfolded is an
// oracle. A root search_target promoted (a variant of a root it dropped,
// D45) was folded, so it is no oracle. When no oracle matches but a
// promoted root or one of the lone search's own tied variants does, the
// lone search folded it too: `tied_under_root` is set, and a caller that
// cannot judge such a variant may skip it.
struct LonePricing {
    std::string diff;              // "" on a lone match, else what differed
    bool tied_under_root = false;  // no root matches, a lone variant does
};

inline void collect_tied(const Path& p, std::vector<const Path*>& out) {
    for (const Path& v : p.variants) {
        out.push_back(&v);
        collect_tied(v, out);
    }
}

// Every root's tied variants, root by root.
inline std::vector<const Path*> all_tied(const std::vector<Path>& roots) {
    std::vector<const Path*> out;
    for (const Path& r : roots) collect_tied(r, out);
    return out;
}

// The ticks a path activates on, in order: what search_target is told.
inline std::vector<int64_t> act_ticks(const Path& p) {
    std::vector<int64_t> at;
    for (const Activation& a : p.walk_activations()) at.push_back(a.timecode.ticks());
    return at;
}

inline LonePricing lone_pricing(const Song& song, const SearchSettings& settings,
                                const Path& variant) {
    const std::vector<int64_t> ticks = act_ticks(variant);
    const std::string mine = windows_text(variant);
    std::vector<bool> promoted;
    const std::vector<Path> lone = search_target(song, settings, ticks, &promoted);
    LonePricing out;
    std::string seen;
    for (size_t i = 0; i < lone.size(); ++i) {
        const Path& t = lone[i];
        if (promoted[i] || t.totalscore() != variant.totalscore()) continue;
        const std::string theirs = windows_text(t);
        if (theirs == mine) return out;
        seen += "\n  lone:    " + theirs;
    }
    for (size_t i = 0; i < lone.size(); ++i) {
        const Path& r = lone[i];
        std::vector<const Path*> tied;
        if (promoted[i]) tied.push_back(&r);
        collect_tied(r, tied);
        for (const Path* t : tied)
            if (t->totalscore() == variant.totalscore() && windows_text(*t) == mine)
                out.tied_under_root = true;
    }
    out.diff = "'" + variant.pathstring() + "' " + std::to_string(variant.totalscore()) +
               "\n  variant: " + mine +
               (out.tied_under_root ? "\n  the lone search tied it under a root too" : "") +
               (seen.empty() ? "\n  no lone root ties it" : seen);
    return out;
}

// "" when a lone root stores what the variant stores, else what differed.
// A variant the lone search folded under a root of its own has no oracle and
// also gives "" (skipped); test_replay counts those and pins the count.
inline std::string lone_pricing_mismatch(const Song& song, const SearchSettings& settings,
                                         const Path& variant) {
    const LonePricing lone = lone_pricing(song, settings, variant);
    return lone.tied_under_root ? std::string() : lone.diff;
}

// The first activation, over every output path, that matches `pred`. It
// walks the paths in place (walk_activations copies nothing), so the pointer
// stays valid as long as `paths` does.
template <typename Pred>
const Activation* find_act(const std::vector<Path>& paths, Pred pred) {
    for (const Path& p : paths)
        for (const Activation& a : p.walk_activations())
            if (pred(a)) return &a;
    return nullptr;
}

// ---- fixture helpers --------------------------------------------------------
//
// Every hand-built record in the tests writes its stored facts through these.

// A window that collected nothing: SP ends where the banked bars put it.
inline void set_plain_window(Activation& a, int64_t end_tick) {
    a.sp_end_steps = {{a.timecode.ticks(), end_tick, SpEndKind::Activation}};
}

// A window the cap clamped at `clamp_tick`, ending at `end_tick`. A display
// test that states only those two facts gets a plain end equal to the final
// one; nothing it checks reads the plain end.
inline void set_clamped_window(Activation& a, int64_t clamp_tick, int64_t end_tick) {
    a.sp_end_steps = {{a.timecode.ticks(), end_tick, SpEndKind::Activation},
                      {clamp_tick, end_tick, SpEndKind::Clamped}};
}

// The bank an activation spends, for a test that needs only the count: one
// arrival per bar on the ticks just before the activation. Only the count
// (sp_meter()) matters to such a test.
inline void set_bank(Activation& a, int bars) {
    a.bank_rise_ticks.clear();
    for (int k = bars; k > 0; --k) a.bank_rise_ticks.push_back(a.timecode.ticks() - k);
}

// The bars of SP an activation spends, for a test that needs only the count.
inline void set_sp_meter(Activation& a, int bars) {
    set_bank(a, bars);
}

// The fills an activation passed over before it, for a test that needs only
// the count: one fill per skip on placeholder ticks just before the
// activation. Only the count (skips()) matters to such a test.
inline void set_skips(Activation& a, int skips) {
    a.skipped_fill_ticks.clear();
    for (int k = skips; k > 0; --k) a.skipped_fill_ticks.push_back(a.timecode.ticks() - k);
}

// The bars banked after a path's last window, for a test that needs only the
// count: one arrival per bar on placeholder ticks just after the path's last
// activation. Only the count (leftover_sp()) matters to such a test.
inline void set_leftover(Path& p, int bars) {
    const int64_t after = p.activations.empty() ? 0 : p.activations.back().timecode.ticks();
    p.trailing_bank_ticks.clear();
    for (int k = 1; k <= bars; ++k) p.trailing_bank_ticks.push_back(after + k);
}

// The stored transfer scales. `squeeze` is the scale every SqIn stores (the
// SqIn lines read it), `post` the one the backend rows read. Call it after
// the activation's squeezes are pushed, so each SqIn takes its scale.
inline void set_transfer(Activation& a, TransferScale squeeze, TransferScale post) {
    for (SPSqueeze& sq : a.sqinouts)
        if (sq.kind == SqueezeKind::SqIn) sq.transfer = squeeze;
    a.transfer_post = post;
}

// One stored scale per SqIn, in sqinouts order, and the SP end's. Call it
// after the squeezes are pushed; `per_sqin` has one entry per SqIn.
inline void set_sqin_transfers(Activation& a, const std::vector<TransferScale>& per_sqin,
                               TransferScale post) {
    for (auto it = a.sqinouts.begin(); it != a.sqinouts.end(); ++it)
        if (is_sqin_squeeze(*it))
            it->transfer = per_sqin.at(sqin_rank(a.sqinouts.begin(), it, is_sqin_squeeze));
    a.transfer_post = post;
}

// One stored scale for the SqIn lines and the backend rows alike.
inline void set_transfer(Activation& a, TransferScale scale) {
    set_transfer(a, scale, scale);
}

// ---- shared test helpers ----------------------------------------------------

// The app's default settings at SP cap `cap`, keeping the top 40 scores
// (D43's test depth) with no ms limit: the settings the hand-made D-tests
// analyze at.
inline app::AnalysisSettings scores_settings(int cap) {
    app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    cfg.sp_cap = cap;
    cfg.depth_mode = DepthMode::Scores;
    cfg.depth_value = 40;
    cfg.ms_filter = std::nullopt;
    return cfg;
}

// The path in `paths` whose path string is `s`, or nullptr.
inline const Path* path_named(const std::vector<Path>& paths, const std::string& s) {
    for (const Path& p : paths)
        if (p.pathstring() == s) return &p;
    return nullptr;
}
inline const Path* path_named(const std::vector<const Path*>& paths, const std::string& s) {
    for (const Path* p : paths)
        if (p->pathstring() == s) return p;
    return nullptr;
}

// Built lines against pinned ones. On a mismatch the lines actually built are
// printed as C++ literals, so a deliberate change can be read, checked by
// hand and pasted.
inline void check_lines(const std::vector<std::string>& got, const std::vector<std::string>& want,
                        const std::string& what) {
    std::string literals;
    if (got != want)
        for (const std::string& l : got) literals += "    \"" + l + "\",\n";
    INFO(what << " built:\n" << literals);
    CHECK(got == want);
}

}  // namespace hydra::test

#endif  // HYDRA_TESTS_RECORD_FIXTURES_H
