#include "search/graph.h"

#include <algorithm>
#include <cstdint>
#include <stdexcept>

#include "core/scoring.h"
#include "core/sqout_chord.h"

namespace hydra {

// kSqueezeWindowMs (core/model.h) is how far apart (ms) a note and a
// deactivation can be and still be a SqIn/SqOut, and how far back
// edge.backends must stay complete.
namespace {
// Min-heap comparator on ticks (std::*_heap build a max-heap by default, so
// `greater` yields a min-heap whose front() is the earliest tick).
struct TickGreater {
    bool operator()(const Timecode& a, const Timecode& b) const {
        return a.ticks() > b.ticks();
    }
};

// D32: whether a squeeze-in's new SP end, one SP bar past an SP end, lands
// at or before the phrase that squeezes in. The graph builds nodes in chart
// order, so a node for such an end has to be added when the SP end it moves
// is handled (add_deact_edge), not when the phrase comes up (build), which is
// too late: the node would then sit behind the phrase on the track. Only
// possible when 500 ms spans more than one SP bar; no library chart is that
// fast. The one statement of the test: build and add_deact_edge both ask it.
bool sqin_end_by_phrase(int64_t end_tick, int64_t phrase_tick) {
    return end_tick <= phrase_tick;
}
}  // namespace

ScoreGraphNode* ScoreGraph::new_node(const Timecode& tc, bool is_sp) {
    node_pool_.emplace_back();
    ScoreGraphNode* n = &node_pool_.back();
    n->timecode = tc;
    n->is_sp = is_sp;
    return n;
}

ScoreGraphEdge* ScoreGraph::new_edge() {
    edge_pool_.emplace_back();
    return &edge_pool_.back();
}

Timecode ScoreGraph::plusmeasure(const Timecode& tc, int64_t add_measures) {
    auto key = std::make_pair(tc.ticks(), add_measures);
    auto it = plusmeasure_cache_.find(key);
    if (it != plusmeasure_cache_.end()) return it->second;
    Timecode r = song_.timing().plusmeasure(tc, add_measures);
    plusmeasure_cache_.emplace(key, r);
    return r;
}

double activation_fill_deadline_ms(const SongTiming& timing,
                                   int64_t fill_end_tick,
                                   int64_t fill_length_ticks,
                                   FillDeadlineRule rule) {
    if (rule == FillDeadlineRule::Ch11) {
        // E threshold is 4 beats before the fill marker.
        int64_t tick_E =
            fill_end_tick - fill_length_ticks - 4 * timing.tick_resolution();
        return timing.timecode(tick_E).ms();
    }

    // Clone Hero 1.0, ported operation-for-operation from Python Hydra 1.2.
    // The lead time is the fill's own length plus a sixteenth-of-a-beat pad,
    // measured in real time back from the fill's end, then clamped.
    //
    // This uses ms_at_tick_f, which core/timing.h marks as outside the
    // bit-for-bit scoring surface; the legacy mode is deliberately off that
    // surface, and never writes the database the GUI reads (docs/adr/0010).
    // The res/16 pad matches Python Hydra 1.2 output; kept fixed (see ADR 0010).
    const double res = static_cast<double>(timing.tick_resolution());
    const double fend = timing.ms_index().at(fill_end_tick);
    const double fst = timing.ms_index().at(fill_end_tick - fill_length_ticks);
    const double pad = timing.ms_index().ms_at_tick_f(
        static_cast<double>(fill_end_tick) -
        (static_cast<double>(fill_length_ticks) + res / 16.0));
    const double fill_len_ms = fend - fst;
    const double preroll = std::max(250.0, std::min(fend - pad, 10000.0));
    return fend - fill_len_ms - preroll;
}

ScoreGraph::ScoreGraph(const Song& song, std::optional<int> sp_meter_cap,
                       FillDeadlineRule rule, const core::Rules& rules)
    : song_(song), sp_meter_cap_(sp_meter_cap), rule_(rule), rules_(rules) {
    start_ = new_node(song_.start_time(), false);
    base_track_head_ = start_;
    sp_track_head_ = new_node(song_.start_time(), true);
    proto_base_edge_ = new_edge();
    proto_sp_edge_ = new_edge();
    sp_phrase_count_ = song.sp_phrase_count();

    build();
}

void ScoreGraph::build() {
    TickGreater cmp;

    for (const SongTimestamp& timestamp : song_.sequence) {
        // SP can fall off between timestamps: handle deacts due before this one.
        while (!deact_heap_.empty() &&
               deact_heap_.front().ticks() < timestamp.timecode.ticks()) {
            std::pop_heap(deact_heap_.begin(), deact_heap_.end(), cmp);
            Timecode pending_deact = deact_heap_.back();
            deact_heap_.pop_back();
            if (pending_deacts_.find(pending_deact.ticks()) ==
                pending_deacts_.end())
                continue;  // stale entry, already handled or extended past
            set_head_time(pending_deact);
            handle_deact(pending_deact, std::nullopt);
        }

        set_head_time(timestamp.timecode);

        store_notecount(timestamp.chord.count());
        if (timestamp.flag_solo)
            store_soloscore(kSoloBonusPerNote * timestamp.chord.count());

        CategoryScores sg = category_scores(timestamp.chord, combo_, nullptr, rules_.sqout_rule);

        if (MultSqueeze::applies(timestamp.chord, combo_))
            store_multsqueeze(MultSqueeze(timestamp.chord, combo_));

        store_basescore(sg.base);
        store_comboscore(sg.combo);
        store_spscore(sg.sp);
        store_accentscore(sg.accent);
        store_ghostscore(sg.ghost);

        combo_ += timestamp.chord.count();

        store_new_backend(timestamp, sg.sp, sg.sqout_sp());

        if (timestamp.flag_sp) {
            // Deacts within the squeeze window keep a non-extended copy (SqOut).
            std::vector<Timecode> sqout_deacts;
            for (const auto& kv : pending_deacts_)
                if (within_squeeze_window(
                        offset_from_sp_end(timestamp.timecode.ms(), kv.second.ms())))
                    sqout_deacts.push_back(kv.second);

            // Timecodes this SP phrase can extend: pending deacts plus very
            // recently handled deacts (SqIn). Deduped by ticks via std::map.
            std::map<int64_t, Timecode> extendable;
            for (const auto& kv : pending_deacts_)
                extendable.emplace(kv.first, kv.second);
            for (ScoreGraphEdge* e : recent_deact_edges_)
                extendable.emplace(e->dest->timecode.ticks(), e->dest->timecode);

            std::vector<Timecode> extendable_list;
            extendable_list.reserve(extendable.size());
            for (const auto& kv : extendable) extendable_list.push_back(kv.second);

            std::vector<DeactExtension> ext =
                extend_deacts(extendable_list, timestamp.timecode);

            std::map<int64_t, SpExtension> ext_map;
            std::unordered_map<int64_t, Timecode> new_pending;
            for (const DeactExtension& de : ext) {
                // A pending end is never behind the head, so only a recently
                // handled SP end (a squeeze-in's) can move to here or before.
                // That end is no node to add now: add_deact_edge already added
                // it. This phrase sits in that SP end's window (the end is
                // recent), so it is one of the end's squeeze choices, and
                // add_deact_edge adds the node for every late one. A path
                // still holding that end has already ended on it
                // (Engine::deactivation_type).
                if (sqin_end_by_phrase(de.to.ticks(), timestamp.timecode.ticks())) continue;
                // A clamp on a phrase in the moved end's own window (the SqOut
                // copies above): that end's deactivation edge offers this
                // phrase as a squeeze-out back to it (finding 37).
                const bool sqout_reach =
                    de.clamped && timestamp.timecode.ticks() <= de.from.ticks() &&
                    within_squeeze_window(
                        offset_from_sp_end(timestamp.timecode.ms(), de.from.ms()));
                ext_map[de.from.ticks()] = SpExtension{de.to.ticks(), de.clamped, sqout_reach};
                new_pending[de.to.ticks()] = de.to;
            }
            for (const Timecode& t : sqout_deacts)
                new_pending[t.ticks()] = t;

            pending_deacts_ = std::move(new_pending);
            deact_heap_.clear();
            for (const auto& kv : pending_deacts_)
                deact_heap_.push_back(kv.second);
            std::make_heap(deact_heap_.begin(), deact_heap_.end(), cmp);

            proto_base_edge_->sp_times.push_back({timestamp.timecode, {}});
            proto_sp_edge_->sp_times.push_back({timestamp.timecode, std::move(ext_map)});
        }

        if (timestamp.has_activation()) {
            advance_tracks(timestamp.timecode, timestamp.chord);
            ScoreGraphEdge* act_edge = add_act_edge(sg.sp, *timestamp.activation_length);

            for (const auto& kv : act_edge->activation_initial_end_times) {
                const Timecode& end_time = kv.second;
                if (pending_deacts_.find(end_time.ticks()) ==
                    pending_deacts_.end()) {
                    pending_deacts_[end_time.ticks()] = end_time;
                    deact_heap_.push_back(end_time);
                    std::push_heap(deact_heap_.begin(), deact_heap_.end(), cmp);
                }
            }
        }

        if (pending_deacts_.find(timestamp.timecode.ticks()) !=
            pending_deacts_.end())
            handle_deact(timestamp.timecode, timestamp.chord);
    }

    const SongTimestamp& last = song_.sequence.back();
    advance_tracks(last.timecode, last.chord);
}

void ScoreGraph::store_notecount(int64_t count) {
    proto_base_edge_->notecount += count;
    proto_sp_edge_->notecount += count;
}
void ScoreGraph::store_soloscore(int64_t points) {
    proto_base_edge_->soloscore += points;
    proto_sp_edge_->soloscore += points;
}
void ScoreGraph::store_basescore(int64_t points) {
    proto_base_edge_->basescore += points;
    proto_sp_edge_->basescore += points;
}
void ScoreGraph::store_comboscore(int64_t points) {
    proto_base_edge_->comboscore += points;
    proto_sp_edge_->comboscore += points;
}
void ScoreGraph::store_spscore(int64_t points) {
    proto_sp_edge_->spscore += points;
}
void ScoreGraph::store_accentscore(int64_t points) {
    proto_base_edge_->accentscore += points;
    proto_sp_edge_->accentscore += points;
}
void ScoreGraph::store_ghostscore(int64_t points) {
    proto_base_edge_->ghostscore += points;
    proto_sp_edge_->ghostscore += points;
}
void ScoreGraph::store_multsqueeze(const MultSqueeze& msq) {
    multsqueezes_.push_back(msq);
}

void ScoreGraph::store_new_backend(const SongTimestamp& ts, int sp_points,
                                   int sqout_points) {
    BackendSqueeze backend;
    backend.timecode = ts.timecode;
    backend.chord = ts.chord;
    backend.points = sp_points;
    backend.sqout_points = sqout_points;

    recent_backends_.push_back(backend);

    // Copy this row onto every deactivation edge still inside its window.
    // Which phrase chord an edge squeezes out is decided once, in
    // add_deact_edge (core/sqout_chord.h), not here.
    for (ScoreGraphEdge* recent_edge : recent_deact_edges_) {
        BackendSqueeze copy = backend;
        copy.offset_ms =
            offset_from_sp_end(ts.timecode.ms(), recent_edge->dest->timecode.ms());
        recent_edge->backends.push_back(copy);
    }
}

int ScoreGraph::max_sp_bars() const {
    if (!sp_meter_cap_.has_value()) return sp_phrase_count_;
    return std::min(*sp_meter_cap_, sp_phrase_count_);
}

std::vector<ScoreGraph::DeactExtension> ScoreGraph::extend_deacts(
    const std::vector<Timecode>& deact_tcs, const Timecode& sp_timecode) {
    std::vector<DeactExtension> out;
    out.reserve(deact_tcs.size());

    if (!sp_meter_cap_.has_value()) {
        for (const Timecode& tc : deact_tcs)
            out.push_back({tc, plusmeasure(tc, sp_bars_to_measures(1)), false});
        return out;
    }

    Timecode ceiling = plusmeasure(sp_timecode, sp_bars_to_measures(*sp_meter_cap_));
    for (const Timecode& tc : deact_tcs) {
        Timecode ext = plusmeasure(tc, sp_bars_to_measures(1));
        // min(ext, ceiling): ceiling only when it is strictly earlier. When
        // it wins, the meter was full to the cap on this phrase, and from
        // here on the SP end is measured from this collecting note (a tie
        // leaves the previous anchor in charge, like the engine's own min).
        const bool clamped = ceiling.ticks() < ext.ticks();
        out.push_back({tc, clamped ? ceiling : ext, clamped});
    }
    return out;
}

bool ScoreGraph::is_recent_to_head(const Timecode& tc) const {
    // The distance is symmetric, so this one check serves both an edge's
    // end (is the SP end still near the head?) and a note (is the note
    // still near the head?). tc is never after the head here.
    return within_squeeze_window(offset_from_sp_end(tc.ms(), head_time_.ms()));
}

void ScoreGraph::set_head_time(const Timecode& tc) {
    head_time_ = tc;

    std::vector<ScoreGraphEdge*> keep_edges;
    for (ScoreGraphEdge* edge : recent_deact_edges_)
        if (is_recent_to_head(edge->dest->timecode))
            keep_edges.push_back(edge);
    recent_deact_edges_ = std::move(keep_edges);

    std::vector<BackendSqueeze> keep_be;
    for (const BackendSqueeze& be : recent_backends_)
        if (is_recent_to_head(be.timecode)) keep_be.push_back(be);
    recent_backends_ = std::move(keep_be);
}

void ScoreGraph::handle_deact(const Timecode& deact_tc,
                              const std::optional<Chord>& chord) {
    if (pending_deacts_.find(deact_tc.ticks()) == pending_deacts_.end())
        return;
    advance_tracks(deact_tc, chord);
    add_deact_edge();
    pending_deacts_.erase(deact_tc.ticks());
}

void ScoreGraph::advance_tracks(const Timecode& tc,
                                const std::optional<Chord>& chord) {
    if (base_track_head_->timecode.ticks() >= tc.ticks()) return;

    proto_base_edge_->dest = new_node(tc, false);
    proto_sp_edge_->dest = new_node(tc, true);
    proto_base_edge_->dest->chord = chord;
    proto_sp_edge_->dest->chord = chord;

    base_track_head_->adv_edge = proto_base_edge_;
    sp_track_head_->adv_edge = proto_sp_edge_;

    proto_base_edge_ = new_edge();
    proto_sp_edge_ = new_edge();

    base_track_head_ = base_track_head_->adv_edge->dest;
    sp_track_head_ = sp_track_head_->adv_edge->dest;
}

ScoreGraphEdge* ScoreGraph::add_act_edge(int frontend_points, int64_t fill_length_ticks) {
    ScoreGraphEdge* act_edge = new_edge();
    act_edge->dest = sp_track_head_;

    act_edge->frontend_points = frontend_points;

    act_edge->activation_fill_deadline_ms = activation_fill_deadline_ms(
        song_.timing(), act_edge->dest->timecode.ticks(), fill_length_ticks,
        rule_);

    for (int sp = 2; sp <= max_sp_bars(); ++sp) {
        act_edge->activation_initial_end_times[sp] =
            plusmeasure(act_edge->dest->timecode, sp_bars_to_measures(sp));
    }

    // A banked phrase some later SP end could still hold in its window
    // (core/sqout_chord.h). Almost always none: 500 ms has to span an SP bar.
    if (const SongTimestamp* banked = core::banked_phrase_in_reach(
            song_, act_edge->dest->timecode.ticks(),
            plusmeasure(act_edge->dest->timecode, sp_bars_to_measures(1)))) {
        int ordinal = 0;
        for (const SongTimestamp& ts : song_.sequence) {
            if (ts.flag_sp) ++ordinal;
            if (&ts == banked) break;
        }
        act_edge->banked_phrase_ordinal = ordinal;
    }

    base_track_head_->branch_edge = act_edge;
    return act_edge;
}

void ScoreGraph::add_deact_edge() {
    ScoreGraphEdge* deact_edge = new_edge();
    deact_edge->dest = base_track_head_;
    const Timecode& end = deact_edge->dest->timecode;

    for (const BackendSqueeze& recent_backend : recent_backends_) {
        BackendSqueeze copy = recent_backend;
        copy.offset_ms = offset_from_sp_end(recent_backend.timecode.ms(), end.ms());
        deact_edge->backends.push_back(copy);
    }

    // The phrase chords this SP end can squeeze, in chart order
    // (core/sqout_chord.h). The engine offers a path the first one its
    // running window can still squeeze. Where collecting a chord moves this
    // end is extend_deacts' answer, the same one the chord's own advance edge
    // carries, cap included (finding 37). On or before the end, both
    // branches' ends move there. After the end, only a late SqIn reaches it,
    // so only the SqIn end moves; a chord after the end can never reach the
    // ceiling (it sits 2 x cap measures past a note later than the end).
    const std::vector<const SongTimestamp*> window = core::squeeze_window_phrases(song_, end);
    for (const SongTimestamp* c : window) {
        const DeactExtension moved = extend_deacts({end}, c->timecode).front();
        SqueezeChoice choice;
        choice.chord = c->timecode;
        choice.timing = offset_from_sp_end(c->timecode.ms(), end.ms());
        choice.late = c->timecode.ticks() > end.ticks();
        choice.sqout_time = choice.late ? end : moved.to;
        choice.sqin_time = moved.to;
        choice.clamped = moved.clamped;
        deact_edge->squeeze_choices.push_back(choice);
        // A late SqIn's end can come before its own phrase: add its node
        // now, while it is still ahead (sqin_end_by_phrase). Any late
        // chord in the window can be the one offered.
        if (choice.late && sqin_end_by_phrase(moved.to.ticks(), c->timecode.ticks()) &&
            pending_deacts_.find(moved.to.ticks()) == pending_deacts_.end()) {
            pending_deacts_[moved.to.ticks()] = moved.to;
            deact_heap_.push_back(moved.to);
            std::push_heap(deact_heap_.begin(), deact_heap_.end(), TickGreater{});
        }
    }

    recent_deact_edges_.push_back(deact_edge);
    sp_track_head_->branch_edge = deact_edge;
}

}  // namespace hydra
