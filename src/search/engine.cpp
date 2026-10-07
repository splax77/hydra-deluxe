#include "search/engine.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/backend_value.h"
#include "core/sqout_chord.h"
#include "core/squeeze_rating.h"

namespace hydra {

namespace {

// The model's Path, aliased before the engine's own local `Path` (the search
// path state) shadows the name below.
using MPath = ::hydra::Path;

// "No value" for optional doubles a path carries; every comparison with NaN is
// false, so a missing timing fails loudly rather than quietly.
const double NO_DOUBLE = std::numeric_limits<double>::quiet_NaN();
inline bool has_value(double v) { return !std::isnan(v); }

const int64_t NO_TIME = -1;
const int32_t SQ_IN = 0;
const int32_t SQ_OUT = 1;
const int32_t DEACT_NONE = 0;
const int32_t DEACT_NORMAL = 1;
const int32_t DEACT_SQINOUT = 2;

const int32_t NODE_BROKEN = -2;

// Clone Hero's early-fill rule lives in core/model.h (fill_e_offset,
// fill_refuses, is_e0). branch_activate applies it; the search's group key
// (ready_class) counts the upcoming fills it refuses.

// Which fill an activation's early-fill offset is measured at: the first fill
// the path passed over, or the activation's own fill when it passed none.
// branch_activate and the D38 variant rebuild (own_early_fill) both ask here.
inline double recorded_e_offset(double first_passed, double own) {
    return has_value(first_passed) ? first_passed : own;
}

// ---- the graph, enumerated ----------------------------------------------
// The search is index-based (indices pack into memo keys and the output act
// records), so the graph is enumerated into index->object arrays, plus one
// value-view per node and edge that node()/edge() hand out.

// Cheap value-views over one node/edge, so the ported engine body keeps reading
// `n.tick` / `e.basescore`. enumerate() builds one per node and edge, once.
struct NodeView {
    int64_t tick;
    int32_t adv_edge, branch_edge, is_sp;
};
struct EdgeView {
    int32_t dest;
    int32_t basescore, comboscore, spscore, soloscore, accentscore, ghostscore;
    int32_t frontend_points;
    int32_t banked_phrase_ordinal;
    double activation_fill_deadline_ms;
    // A deactivation edge's squeeze choices: Enum::choices[choice_begin,
    // choice_end), in chart order (ScoreGraphEdge::squeeze_choices).
    int32_t choice_begin, choice_end;
};
// One squeeze choice of a deactivation edge, in ticks.
struct ChoiceView {
    int64_t chord;
    double timing;
    int64_t sqin_time;  // a late chord's (SqueezeChoice::sqin_time), else NO_TIME
    int32_t late;
};

struct Enum {
    std::vector<const ScoreGraphNode*> nodes;
    std::vector<const ScoreGraphEdge*> edges;
    // One view per node and per edge, in index order, built once at the end
    // of enumerate(): the hot loop indexes these instead of rebuilding a view
    // through hash-map lookups on every read.
    std::vector<NodeView> node_views;
    std::vector<EdgeView> edge_views;
    // Every deactivation edge's squeeze choices, back to back.
    std::vector<ChoiceView> choices;
    int32_t start = -1;
    // The graph's backend rows; each deactivation edge reads a range of them.
    const std::vector<BackendSqueeze>* all_backends = nullptr;
};

Enum enumerate(const ScoreGraph& graph) {
    Enum en;
    en.all_backends = &graph.all_backends();
    std::unordered_map<const ScoreGraphNode*, int32_t> node_idx;
    std::unordered_map<const ScoreGraphEdge*, int32_t> edge_idx;
    std::vector<const ScoreGraphNode*> pend_nodes;
    std::vector<const ScoreGraphEdge*> pend_edges;

    std::function<int32_t(const ScoreGraphNode*)> nid =
        [&](const ScoreGraphNode* n) -> int32_t {
        if (n == nullptr) return -1;
        auto it = node_idx.find(n);
        if (it != node_idx.end()) return it->second;
        int32_t idx = static_cast<int32_t>(en.nodes.size());
        node_idx[n] = idx;
        en.nodes.push_back(n);
        pend_nodes.push_back(n);
        return idx;
    };
    std::function<int32_t(const ScoreGraphEdge*)> eid =
        [&](const ScoreGraphEdge* e) -> int32_t {
        if (e == nullptr) return -1;
        auto it = edge_idx.find(e);
        if (it != edge_idx.end()) return it->second;
        int32_t idx = static_cast<int32_t>(en.edges.size());
        edge_idx[e] = idx;
        en.edges.push_back(e);
        pend_edges.push_back(e);
        return idx;
    };

    en.start = nid(graph.start());
    while (!pend_nodes.empty() || !pend_edges.empty()) {
        while (!pend_nodes.empty()) {
            const ScoreGraphNode* n = pend_nodes.back();
            pend_nodes.pop_back();
            eid(n->adv_edge);
            eid(n->branch_edge);
        }
        while (!pend_edges.empty()) {
            const ScoreGraphEdge* e = pend_edges.back();
            pend_edges.pop_back();
            nid(e->dest);
        }
    }

    // Every reachable pointer is indexed now, so each view resolves fully.
    auto node_of = [&](const ScoreGraphNode* n) -> int32_t {
        return n == nullptr ? -1 : node_idx.at(n);
    };
    auto edge_of = [&](const ScoreGraphEdge* e) -> int32_t {
        return e == nullptr ? -1 : edge_idx.at(e);
    };

    en.node_views.reserve(en.nodes.size());
    for (const ScoreGraphNode* o : en.nodes) {
        NodeView v;
        v.tick = o->timecode.ticks();
        v.adv_edge = edge_of(o->adv_edge);
        v.branch_edge = edge_of(o->branch_edge);
        v.is_sp = o->is_sp ? 1 : 0;
        en.node_views.push_back(v);
    }

    en.edge_views.reserve(en.edges.size());
    for (const ScoreGraphEdge* o : en.edges) {
        EdgeView v;
        v.dest = node_of(o->dest);
        v.basescore = (int32_t)o->basescore;
        v.comboscore = (int32_t)o->comboscore;
        v.spscore = (int32_t)o->spscore;
        v.soloscore = (int32_t)o->soloscore;
        v.accentscore = (int32_t)o->accentscore;
        v.ghostscore = (int32_t)o->ghostscore;
        v.frontend_points = o->frontend_points;
        v.banked_phrase_ordinal = o->banked_phrase_ordinal;
        // Unset on advance and deactivation edges. An activation edge without
        // one is refused in index_fills (audit R7.35).
        v.activation_fill_deadline_ms = o->activation_fill_deadline_ms
                                            ? *o->activation_fill_deadline_ms
                                            : NO_DOUBLE;
        v.choice_begin = (int32_t)en.choices.size();
        for (const SqueezeChoice& c : o->squeeze_choices)
            en.choices.push_back(ChoiceView{c.chord.ticks(), c.timing,
                                            c.sqin_time ? c.sqin_time->ticks() : NO_TIME,
                                            c.late ? 1 : 0});
        v.choice_end = (int32_t)en.choices.size();
        en.edge_views.push_back(v);
    }
    return en;
}

// ---- engine data structures ---------------------------------------------

struct Act {
    int32_t parent;
    int32_t act_node;
    // How many fills were passed over before it, for act_within_limit's E0
    // test. The record stores the fills themselves (skip_tail).
    int32_t skips;
    int32_t deact_edge;
    int32_t sq_tail;
    int32_t depth;
    double e_offset;
    // The SP-end steps so far, an index into ends_, or -1. Copied off the
    // live path when the activation closes. The clamp note and the collected
    // phrases are steps of it.
    int32_t end_tail;
    // Where each bar this activation spends arrived (an index into banks_),
    // or -1. Taken off the live path when the activation is made.
    int32_t bank_tail;
    // The fills passed over before this activation (an index into fills_),
    // or -1. Taken off the live path when the activation is made.
    int32_t skip_tail;
    // The phrase chord its closing SqOut squeezed out, or NO_TIME. The deact
    // edge lists several choices; this is the one the window was offered.
    int64_t sqout_phrase;
};
struct SqNode {
    int32_t prev;
    int32_t kind;
    double offset;
};
// One tick in a chain of ticks (a bank arrival or a passed-over fill),
// linked to the one before.
struct ColNode {
    int32_t prev;
    int64_t tick;
};
// One SP-end step of the running activation, linked to the one before.
struct EndNode {
    int32_t prev;
    int64_t tick;
    int64_t end;
    SpEndKind kind;
    // A Collected or Clamped step: the end it moved from, when that end can
    // give this phrase back (SpExtension::sqout_node), else NO_TIME. Only
    // there is the phrase offered as a squeeze (core::offered_phrase, D36).
    // A step relabelled SqIn keeps it: offered_phrase itself skips a phrase
    // the window squeezed in (D34). Engine state only; the record's
    // SpEndStep does not carry it.
    int64_t sqout_at;
};
struct Variant {
    int32_t prev;
    int32_t var_point;
    int32_t act_tail;
    int32_t var_head;
    int32_t tied_count;
    int64_t sp_end;
    int32_t end_tail;
    // Folded while its last activation's SP was still running. From the fold
    // on, the leader's closing of that window is the variant's own (D3).
    bool open_sp;
    // The chart tick of the node both paths stood on at the fold.
    int64_t fold_tick;
    // How many squeezes the leader's running window held at the fold. Any
    // later ones happened after it, on both paths.
    int32_t fold_sq_count;
    // The path had finished the song when it folded.
    bool finished;
    // Its banked bars at the fold (an index into banks_, or -1). For a finished
    // path this is its whole trailing list; between windows, its first m bars.
    int32_t bank_tail;
    // Its own early-fill facts at the fold (D38), for the activation its
    // leader takes next: its SP-ready time, the fills it passed over since
    // its last window (an index into fills_, or -1), and the e_offset at the
    // first of them (NO_DOUBLE when none).
    double sp_ready_ms;
    int32_t skip_tail;
    double skipped_e_offset;
};
struct Path {
    int32_t node;
    int32_t sp;
    int32_t currentskips;
    // Phrases still ahead of the path that a squeeze already accounted for.
    // They are the first phrases on the edges to come, in chart order: the
    // spent ones first, then the banked one.
    //  - spent: late-SqIn phrases (D32, D34). Their SP extension is already
    //    in sp_end_time and their SqIn step is written, so reaching one adds
    //    no step and no bar, on SP or off it.
    //  - banked_ahead: 0 or 1, off SP only. A late squeeze-out's phrase,
    //    whose bar the squeeze-out already banked (it is in sp and
    //    bank_tail). Reaching it adds nothing. An edge that passes without it
    //    hands that bar back, and the phrase's own edge adds it again.
    int32_t spent;
    int32_t banked_ahead;
    int32_t act_tail;
    int32_t var_head;
    int32_t tied_count;
    int32_t sc[6];
    int64_t score;
    int64_t sp_end_time;
    // The running activation's SP-end steps so far (an index into ends_),
    // or -1. Handed to the Act at deactivation.
    int32_t end_tail;
    // The bars banked since the last window closed, one arrival tick each
    // (an index into banks_), or -1. Always holds p.sp entries. Handed to
    // the Act at activation.
    int32_t bank_tail;
    // The fills passed over since the last activation, one tick each (an
    // index into fills_), or -1. Always holds currentskips entries. Handed to
    // the Act at activation.
    int32_t skip_tail;
    // The running activation's banked phrase in squeeze reach (the act
    // edge's banked_phrase_ordinal), 0 for none. Read only while SP runs.
    int32_t banked_phrase_ordinal;
    double sp_ready_ms;
    double skipped_e_offset;
    // Some closed activation already has a timing outside this run's limit
    // (act_within_limit). Once set it stays set.
    bool over_prefix;
};

// The decision-log output (was hy_out_*), rebuilt into core Paths (core/model.h) locally.
struct OutPath {
    int32_t score_base, score_combo, score_sp, score_solo, score_accents,
        score_ghosts;
    int32_t var_point, depth, act_begin, act_end;
    // The bars banked after the last window: out_ticks_[bank_begin, bank_end).
    int32_t bank_begin, bank_end;
    // On a root path only (depth 0): how many paths the engine folded into it,
    // its own running count for the tie limit. rebuild checks it against the
    // recount of the finished tree (Path::recount_tied_paths), which is the
    // count that is stored. 0 on a variant.
    int32_t tied_count;
};
struct OutAct {
    int32_t act_node, deact_edge, sq_begin, sq_end;
    double e_offset;
    // Only set (non-NO_TIME) on a path's last activation when it never
    // deactivated: the engine's tracked SP end, extensions included.
    int64_t final_sp_end;
    // The SP-end history: out_ends_[end_begin, end_end).
    int32_t end_begin, end_end;
    // Where each bar it spends arrived: out_ticks_[bank_begin, bank_end).
    int32_t bank_begin, bank_end;
    // The fills passed over before it: out_ticks_[skip_begin, skip_end).
    int32_t skip_begin, skip_end;
    // The phrase chord its closing SqOut squeezed out (Act::sqout_phrase).
    int64_t sqout_phrase;
};
struct OutSq {
    int32_t kind;
    double offset;
};

class StampMap {
public:
    void reset(size_t want) {
        size_t cap = 16;
        while (cap < want * 2) cap <<= 1;
        if (cap > mask_ + 1) {
            mask_ = cap - 1;
            keys_.assign(cap, 0);
            keys2_.assign(cap, 0);
            vals_.assign(cap, 0);
            stamps_.assign(cap, 0);
            stamp_ = 0;
        }
        if (++stamp_ == 0) {
            std::fill(stamps_.begin(), stamps_.end(), 0u);
            stamp_ = 1;
        }
    }
    int32_t get_or_insert(uint64_t key, int32_t want_value, bool* inserted) {
        return get_or_insert(key, 0, want_value, inserted);
    }
    // A two-word key, compared exactly on both words (the search's group
    // key, D36).
    int32_t get_or_insert(uint64_t key, uint64_t key2, int32_t want_value, bool* inserted) {
        size_t i = hash(key ^ hash(key2)) & mask_;
        for (;;) {
            if (stamps_[i] != stamp_) {
                stamps_[i] = stamp_;
                keys_[i] = key;
                keys2_[i] = key2;
                vals_[i] = want_value;
                *inserted = true;
                return want_value;
            }
            if (keys_[i] == key && keys2_[i] == key2) {
                *inserted = false;
                return vals_[i];
            }
            i = (i + 1) & mask_;
        }
    }

private:
    static uint64_t hash(uint64_t x) {
        x += 0x9E3779B97F4A7C15ull;
        x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
        x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
        return x ^ (x >> 31);
    }
    std::vector<uint64_t> keys_;
    std::vector<uint64_t> keys2_;
    std::vector<int32_t> vals_;
    std::vector<uint32_t> stamps_;
    size_t mask_ = 0;
    uint32_t stamp_ = 0;
};

// ---- the engine ----------------------------------------------------------

class Engine {
public:
    // `options` must outlive the engine: target_act_ticks_ points into it.
    Engine(const Enum& en, bool has_sp_cap, int32_t sp_cap,
           const EngineOptions& options, double backend_leeway_ms,
           int32_t max_tied_paths)
        : en_(en),
          backend_leeway_ms_(backend_leeway_ms),
          max_tied_paths_(max_tied_paths),
          has_sp_cap_(has_sp_cap),
          sp_cap_(sp_cap),
          depth_mode_(options.depth_mode),
          depth_value_(options.depth_value),
          has_ms_filter_(options.ms_filter.has_value()),
          ms_filter_(options.ms_filter.value_or(0.0)),
          no_skips_(options.no_skips),
          hard_ms_filter_(options.hard_ms_filter),
          target_act_ticks_(options.target_act_ticks ? &*options.target_act_ticks
                                                     : nullptr) {
        index_fills();
    }

    bool run();

    // Optional 0..1 progress sink, called from run()'s BFS sweep as the frontier
    // advances through the chart. Reported values are monotonic non-decreasing.
    void set_progress_cb(std::function<void(float)> cb) { progress_cb_ = std::move(cb); }

    const std::vector<OutPath>& out_paths() const { return out_paths_; }
    const std::vector<OutAct>& out_acts() const { return out_acts_; }
    const std::vector<OutSq>& out_sqs() const { return out_sqs_; }
    const std::vector<SpEndStep>& out_ends() const { return out_ends_; }
    const std::vector<int64_t>& out_ticks() const { return out_ticks_; }

private:
    const NodeView& node(int32_t i) const { return en_.node_views[(size_t)i]; }
    const EdgeView& edge(int32_t i) const { return en_.edge_views[(size_t)i]; }
    const ScoreGraphEdge* eobj(int32_t i) const { return en_.edges[(size_t)i]; }

    int32_t new_act(int32_t parent, int32_t act_node, int32_t skips, double e_offset) {
        Act a;
        a.parent = parent;
        a.act_node = act_node;
        a.skips = skips;
        a.deact_edge = -1;
        a.sq_tail = -1;
        a.depth = (parent < 0 ? 0 : acts_[(size_t)parent].depth) + 1;
        a.e_offset = e_offset;
        a.end_tail = -1;
        a.bank_tail = -1;
        a.skip_tail = -1;
        a.sqout_phrase = NO_TIME;
        acts_.push_back(a);
        return (int32_t)acts_.size() - 1;
    }
    int32_t clone_tail(int32_t tail) {
        if (tail < 0) return -1;
        acts_.push_back(acts_[(size_t)tail]);
        return (int32_t)acts_.size() - 1;
    }
    int32_t push_sq(int32_t prev, int32_t kind, double offset) {
        SqNode s;
        s.prev = prev;
        s.kind = kind;
        s.offset = offset;
        sqs_.push_back(s);
        return (int32_t)sqs_.size() - 1;
    }
    int32_t push_end(int32_t prev, int64_t tick, int64_t end, SpEndKind kind,
                     int64_t sqout_at = NO_TIME) {
        ends_.push_back(EndNode{prev, tick, end, kind, sqout_at});
        return (int32_t)ends_.size() - 1;
    }
    // Where the path's newest step can still be squeezed out (EndNode::
    // sqout_at), while that SP end is ahead of `tick`; NO_TIME otherwise.
    // The one future fact the squeeze rule reads that the SP end alone does
    // not say, so the group key holds it (reduce_iteration_paths, D36).
    int64_t pending_sqout_at(const Path& p, int64_t tick) const {
        if (p.end_tail < 0) return NO_TIME;
        const int64_t at = ends_[(size_t)p.end_tail].sqout_at;
        return at != NO_TIME && at > tick ? at : NO_TIME;
    }
    // A squeeze-out gives its phrase back, and with it every step at or after
    // the squeezed-out chord. The one statement of that rule: trim_ends and
    // close_folded_act both ask it.
    static bool given_back(int64_t step_tick, int64_t sqout_phrase) {
        return step_tick >= sqout_phrase;
    }
    // An early SqIn's step is the step on the squeezed-in chord's tick: a
    // phrase the gauge received, so Collected, or Clamped when the cap pinned
    // the end on that phrase. A lone path's is its newest step (D36), and
    // never an SqIn step: a phrase it squeezed in is never offered again. A folded variant
    // can, when its own window squeezed the phrase in before the fold
    // (extreme tempos only); the step stays SqIn. Never the Activation step, which stays
    // first in the window's history. The one statement of that rule:
    // relabel_sqin and close_folded_act both ask it.
    static bool is_sqin_step(int64_t step_tick, SpEndKind step_kind, int64_t sqin_tick) {
        return step_tick == sqin_tick && step_kind != SpEndKind::Activation;
    }
    // The chain without the steps a squeeze-out at `tick` gives back.
    int32_t trim_ends(int32_t tail, int64_t tick) const {
        while (tail >= 0 && given_back(ends_[(size_t)tail].tick, tick))
            tail = ends_[(size_t)tail].prev;
        return tail;
    }
    // The chain with the step at `tick` relabelled SqIn. Nodes are shared
    // between paths, so the steps from it on are copied, never edited. An
    // early SqIn's phrase was collected before this point, so an SqIn step
    // (is_sqin_step) must sit at `tick`. When none does the state is
    // impossible: this returns false,
    // leaves `tail` alone, and the caller marks the path broken (the run then
    // throws, as for every other impossible state).
    bool relabel_sqin(int32_t& tail, int64_t tick) {
        std::vector<EndNode> after;
        int32_t t = tail;
        while (t >= 0 && ends_[(size_t)t].tick > tick) {
            after.push_back(ends_[(size_t)t]);
            t = ends_[(size_t)t].prev;
        }
        if (t < 0 || !is_sqin_step(ends_[(size_t)t].tick, ends_[(size_t)t].kind, tick))
            return false;
        const EndNode found = ends_[(size_t)t];
        int32_t out = push_end(found.prev, found.tick, found.end, SpEndKind::SqIn, found.sqout_at);
        for (size_t k = after.size(); k-- > 0;)
            out = push_end(out, after[k].tick, after[k].end, after[k].kind, after[k].sqout_at);
        tail = out;
        return true;
    }
    // Whether the running window already squeezed the phrase on `tick` in:
    // it holds an SqIn step there. Steps run in tick order, so the walk stops
    // at the first one before `tick`.
    bool squeezed_in(const Path& p, int64_t tick) const {
        for (int32_t t = p.end_tail; t >= 0; t = ends_[(size_t)t].prev) {
            const EndNode& s = ends_[(size_t)t];
            if (s.tick < tick) return false;
            if (is_sqin_step_on(s.tick, s.kind, tick)) return true;
        }
        return false;
    }
    int32_t act_count(const Path& p) const {
        return p.act_tail < 0 ? 0 : acts_[(size_t)p.act_tail].depth;
    }
    int32_t sq_count(int32_t sq_tail) const {
        int32_t n = 0;
        for (int32_t s = sq_tail; s >= 0; s = sqs_[(size_t)s].prev) ++n;
        return n;
    }

    void advance(Path& p);
    bool branch_activate(Path& p, Path* child);
    bool branch_deactivate(Path& p, Path* child, bool* has_child);
    // `sq` is the squeeze choice a SqOut child squeezes out; nullptr for a
    // plain end.
    void create_deactivated_path(const Path& p, Path* child, const ChoiceView* sq);
    // `*offered` is set to the choice the path is offered, when the answer
    // is DEACT_SQINOUT.
    int32_t deactivation_type(const EdgeView& e, const Path& p,
                              const ChoiceView** offered) const;

    // Is every timing of this activation inside this run's limit? True for
    // no activation, and for every activation when the run has no limit.
    bool act_within_limit(int32_t act) const;
    bool passes_ms_filter(const Path& p) const;

    // The fills on the base track in chart order, for ready_class.
    void index_fills();
    uint64_t ready_class(const Path& p) const;
    void close_last_activation(Path& p) const;

    void reduce_iteration_paths();
    void reduce_group(const int32_t* members, int32_t n);
    bool outside_depth_band(int64_t score, int64_t best, int32_t outscored_by) const;
    bool can_outscore(int32_t idx) const;
    std::optional<int64_t> best_eligible_score(const int32_t* members, int32_t n) const;

    void emit_path(const Path& p);
    void emit_variant(int32_t v, int32_t depth, const std::vector<int32_t>& parent_walk,
                      int32_t parent_trail_begin, int32_t parent_trail_end);
    void close_folded_act(int32_t own, int32_t lead, const Variant& var);
    void own_early_fill(const Variant& var, OutAct* next);
    // The deadline of the base-track fill on `tick` (index_fills' list).
    double fill_deadline_at(int64_t tick) const {
        const auto it = std::lower_bound(fill_tick_.begin(), fill_tick_.end(), tick);
        if (it == fill_tick_.end() || *it != tick)
            throw std::logic_error("no fill on a passed-over fill's tick");
        return fill_deadline_[(size_t)(it - fill_tick_.begin())];
    }
    void emit_acts(int32_t act_tail, int64_t sp_end_time, int32_t end_tail,
                   int32_t* begin, int32_t* end);
    int32_t push_tick(std::vector<ColNode>& pool, int32_t prev, int64_t tick) {
        pool.push_back(ColNode{prev, tick});
        return (int32_t)pool.size() - 1;
    }
    // A tick chain copied out in order, into out_ticks_[*begin, *end).
    void emit_ticks(const std::vector<ColNode>& pool, int32_t tail, int32_t* begin, int32_t* end) {
        tick_scratch_.clear();
        for (int32_t c = tail; c >= 0; c = pool[(size_t)c].prev) tick_scratch_.push_back(c);
        *begin = (int32_t)out_ticks_.size();
        for (size_t k = tick_scratch_.size(); k-- > 0;)
            out_ticks_.push_back(pool[(size_t)tick_scratch_[k]].tick);
        *end = (int32_t)out_ticks_.size();
    }
    // A variant folded between windows (or at the song's end): its own banked
    // bars at the fold, m of them, then the leader's list from position m on,
    // into out_ticks_[*begin, *end). Both held m bars at the fold, and the
    // leader's list only grew after it (a bar is handed back only on the edge
    // after a squeeze-out, and the search never folds a path holding a
    // banked or spent phrase ahead), so its
    // first m are its own past. Counting, not ticks, marks the split: a
    // squeezed-out bar can sit past the fold node.
    void splice_bank(int32_t own_tail, int32_t lead_begin, int32_t lead_end,
                     int32_t* begin, int32_t* end) {
        tick_scratch_.clear();
        for (int32_t c = own_tail; c >= 0; c = banks_[(size_t)c].prev) tick_scratch_.push_back(c);
        const int32_t m = (int32_t)tick_scratch_.size();
        if (lead_end - lead_begin < m)
            throw std::logic_error("a folded variant banked more bars than its leader");
        *begin = (int32_t)out_ticks_.size();
        for (size_t k = tick_scratch_.size(); k-- > 0;)
            out_ticks_.push_back(banks_[(size_t)tick_scratch_[k]].tick);
        for (int32_t k = lead_begin + m; k < lead_end; ++k) {
            // Copied to a local first: the push can reallocate out_ticks_.
            const int64_t t = out_ticks_[(size_t)k];
            out_ticks_.push_back(t);
        }
        *end = (int32_t)out_ticks_.size();
    }
    void emit_ends(int32_t tail, int32_t* begin, int32_t* end) {
        end_scratch_.clear();
        for (int32_t s = tail; s >= 0; s = ends_[(size_t)s].prev) end_scratch_.push_back(s);
        *begin = (int32_t)out_ends_.size();
        for (size_t k = end_scratch_.size(); k-- > 0;) {
            const EndNode& e = ends_[(size_t)end_scratch_[k]];
            out_ends_.push_back(SpEndStep{e.tick, e.end, e.kind});
        }
        *end = (int32_t)out_ends_.size();
    }

    const Enum& en_;
    // hydra_rules.ini: the backend leeway edge and the tied-path fold limit.
    double backend_leeway_ms_;
    int32_t max_tied_paths_;
    bool has_sp_cap_;
    int32_t sp_cap_;
    DepthMode depth_mode_;
    int32_t depth_value_;
    bool has_ms_filter_;
    double ms_filter_;
    // Every activation must record skips == 0: the declining parent is dropped
    // whenever branch_activate produced a real child. BFS only.
    bool no_skips_;
    // Treat ms_filter_ as a requirement rather than a preference
    // (EngineOptions::hard_ms_filter): a path over the limit is dropped
    // outright instead of surviving while nothing outscores it. BFS only. See
    // reduce_iteration_paths for why this is exact.
    bool hard_ms_filter_;
    // When set, the exact node ticks the search must activate at, ascending.
    // Every other fill is declined. Owned by the caller for the run's duration.
    const std::vector<int64_t>* target_act_ticks_ = nullptr;

    std::vector<Act> acts_;
    std::vector<SqNode> sqs_;
    // Every SP-end step any path took, as linked chains.
    std::vector<EndNode> ends_;
    // Every bank arrival any path made, as linked chains.
    std::vector<ColNode> banks_;
    // Every fill any path passed over, as linked chains.
    std::vector<ColNode> fills_;
    std::vector<Variant> variants_;

    std::vector<Path> cur_;
    std::vector<Path> next_;

    std::vector<uint8_t> filtered_;
    std::vector<uint8_t> removed_;
    std::vector<int32_t> owner_;
    std::vector<int32_t> counts_;
    std::vector<int32_t> group_members_;
    std::vector<int32_t> group_begin_;
    std::vector<int32_t> group_end_;
    std::vector<int32_t> survivors_;
    std::vector<int64_t> beating_;
    std::vector<int64_t> dominating_;
    StampMap group_map_;
    StampMap tie_map_;

    bool has_optimal_ = false;
    int64_t optimal_score_ = 0;

    std::vector<OutPath> out_paths_;
    std::vector<OutAct> out_acts_;
    std::vector<OutSq> out_sqs_;
    std::vector<SpEndStep> out_ends_;
    // Every tick list copied out (bank arrivals, passed-over fills), as ranges.
    std::vector<int64_t> out_ticks_;
    std::vector<int32_t> chain_scratch_;
    std::vector<int32_t> sq_scratch_;
    std::vector<int32_t> end_scratch_;
    std::vector<int32_t> tick_scratch_;

    std::function<void(float)> progress_cb_;
    float progress_reported_ = -1.0f;

    // The base track's fills in chart order: each one's deadline, and the
    // earliest deadline from it on (so ready_class can stop early). Per node,
    // the first fill after it. Built once, by index_fills.
    std::vector<int64_t> fill_tick_;
    std::vector<double> fill_deadline_;
    std::vector<double> fill_min_deadline_;
    std::vector<int32_t> next_fill_;
};

// --- index_fills ---------------------------------------------------------
void Engine::index_fills() {
    std::vector<std::pair<int64_t, double>> fills;  // tick, deadline
    for (const NodeView& n : en_.node_views) {
        if (n.is_sp || n.branch_edge < 0) continue;
        const double deadline = edge(n.branch_edge).activation_fill_deadline_ms;
        // A NaN deadline would read as "no fill to time" at every check
        // downstream, so a fill with no deadline refuses the graph here.
        if (!has_value(deadline))
            throw std::logic_error("the activation edge at tick " + std::to_string(n.tick) +
                                   " has no fill deadline");
        fills.emplace_back(n.tick, deadline);
    }
    std::sort(fills.begin(), fills.end());
    std::vector<int64_t>& ticks = fill_tick_;
    ticks.reserve(fills.size());
    fill_deadline_.reserve(fills.size());
    for (const auto& f : fills) {
        ticks.push_back(f.first);
        fill_deadline_.push_back(f.second);
    }
    fill_min_deadline_ = fill_deadline_;
    for (size_t k = fill_min_deadline_.size(); k-- > 1;)
        fill_min_deadline_[k - 1] = std::min(fill_min_deadline_[k - 1], fill_min_deadline_[k]);
    next_fill_.reserve(en_.node_views.size());
    for (const NodeView& n : en_.node_views)
        next_fill_.push_back(
            (int32_t)(std::upper_bound(ticks.begin(), ticks.end(), n.tick) - ticks.begin()));
}

// --- ready_class ---------------------------------------------------------
// What a waiting path's SP-ready time decides about its future, as a group
// key part. Two paths with the same meter, standing on the same node, can
// differ only in which upcoming fills spawn for them (the early-fill rule)
// and, under an ms limit, which of those early fills would put them over it
// (an E0's difficulty is its ready time past the deadline). Each of those is
// one cut-off ready time per fill: a later ready time is refused, or over
// the limit, at every fill an earlier one is, and more. So the number of
// upcoming fills on the far side of each cut-off names the set exactly, and
// two paths with equal counts are interchangeable. Low 16 bits: fills whose
// early fill would be over the limit (only while nothing was passed over, so
// the next activation can be an E0). High 16: fills that refuse it. 0 for a
// path under kSpActivationBars: its ready time is not set yet.
uint64_t Engine::ready_class(const Path& p) const {
    if (p.sp < kSpActivationBars || !has_value(p.sp_ready_ms)) return 0;
    uint64_t refused = 0, over = 0;
    for (size_t k = (size_t)next_fill_[(size_t)p.node]; k < fill_deadline_.size(); ++k) {
        // Every deadline from here on is at least this one. When even this
        // one leaves too much slack for an E0, none is an E0, and none
        // refuses (a fill that refuses is always inside the E0 window).
        if (!is_e0(fill_e_offset(fill_min_deadline_[k], p.sp_ready_ms), 0)) break;
        const double e_offset = fill_e_offset(fill_deadline_[k], p.sp_ready_ms);
        if (fill_refuses(e_offset)) ++refused;
        if (has_ms_filter_ && !fill_within_limit(e_offset, p.currentskips, ms_filter_)) ++over;
    }
    if (refused > 0xFFFF || over > 0xFFFF) throw std::logic_error("search group key out of range");
    return (refused << 16) | over;
}

// --- advance -------------------------------------------------------------
void Engine::advance(Path& p) {
    if (p.node < 0) return;

    const NodeView n = node(p.node);
    if (n.adv_edge < 0) {
        p.node = -1;
        return;
    }

    const EdgeView e = edge(n.adv_edge);
    const ScoreGraphEdge* eo = eobj(n.adv_edge);

    p.sc[0] += e.basescore;
    p.sc[1] += e.comboscore;
    p.sc[2] += e.spscore;
    p.sc[3] += e.soloscore;
    p.sc[4] += e.accentscore;
    p.sc[5] += e.ghostscore;
    p.score += (int64_t)e.basescore + e.comboscore + e.spscore + e.soloscore +
               e.accentscore + e.ghostscore;

    const int32_t sp_n = (int32_t)eo->sp_times.size();

    if (n.is_sp) {
        // A squeeze-out's banked bar exists only between its SP end and the
        // first edge off SP, which settles it (below).
        if (p.banked_ahead != 0) throw std::logic_error("a banked squeeze-out phrase on SP");
        if (sp_n > 0) {
            int32_t spent = p.spent;
            int64_t sp_end_time = p.sp_end_time;
            for (int32_t i = 0; i < sp_n; ++i) {
                // A spent (late-SqIn) phrase's extension is already in
                // sp_end_time, and its step was written at the deact node.
                if (spent > 0) {
                    --spent;
                    continue;
                }
                const auto& emap = eo->sp_times[(size_t)i].second;
                auto mit = emap.find(sp_end_time);
                if (mit == emap.end()) {
                    p.node = NODE_BROKEN;
                    return;
                }
                // The step remembers the end it moved, while the old end is
                // still at hand, when that end can give the phrase back
                // (D36; this also tells two ends clamped to one tick apart,
                // finding 37).
                const int64_t moved_from = sp_end_time;
                sp_end_time = mit->second.to_tick;
                // Every other phrase the gauge receives is one step: Clamped
                // when the cap pinned the end to it, else Collected.
                p.end_tail = push_end(p.end_tail, eo->sp_times[(size_t)i].first.ticks(),
                                      sp_end_time,
                                      mit->second.clamped ? SpEndKind::Clamped
                                                          : SpEndKind::Collected,
                                      mit->second.sqout_node ? moved_from : NO_TIME);
            }
            p.sp_end_time = sp_end_time;
            p.spent = spent;
        }
    } else {
        // The edge's phrases, in chart order: first any spent ones, then the
        // banked one, then fresh phrases. D32/D34: a path that late-squeezed
        // phrases in can end SP before reaching them (deactivation_type), so
        // spent phrases can still be ahead here, with the meter empty or
        // holding only a squeeze-out's bar. A spent phrase adds nothing when
        // its edge comes; until then it stays spent. A banked phrase not on
        // this edge hands its bar back (the only loss) and its own edge adds
        // it again as a fresh phrase. Either way the spent or banked phrases
        // this edge passes without stay first in line, so nothing fresh can
        // be on it.
        const int32_t old_sp = p.sp;
        const int32_t spent_here = std::min(sp_n, p.spent);
        const int32_t banked_here = std::min(sp_n - spent_here, p.banked_ahead);
        const int32_t fresh = sp_n - spent_here - banked_here;
        const int32_t handed_back = p.banked_ahead - banked_here;
        const int32_t kept = old_sp - handed_back;
        // The meter holds the banked bar it hands back, so this never goes
        // below zero. If it does, the bookkeeping above is wrong: fail loudly
        // instead of quietly skipping a phrase the path should bank.
        if (kept < 0) throw std::logic_error("the SP meter went below zero off SP");
        int32_t sp = kept + fresh;
        if (has_sp_cap_ && sp > sp_cap_) sp = sp_cap_;
        p.sp = sp;
        // Keep the banked bars in step with p.sp: the handed-back bar is the
        // newest (a squeeze-out banks onto an empty list), and each gain is a
        // fresh phrase past the spent and banked ones.
        for (int32_t k = 0; k < handed_back; ++k)
            p.bank_tail = banks_[(size_t)p.bank_tail].prev;
        for (int32_t k = 0; k < sp - kept; ++k)
            p.bank_tail = push_tick(
                banks_, p.bank_tail,
                eo->sp_times[(size_t)(spent_here + banked_here + k)].first.ticks());

        if (kept < kSpActivationBars && sp >= kSpActivationBars) {
            const int32_t k = spent_here + banked_here + (kSpActivationBars - 1) - kept;
            if (k < 0 || k >= sp_n) {
                p.node = NODE_BROKEN;
                return;
            }
            p.sp_ready_ms = eo->sp_times[(size_t)k].first.ms();
        }
        p.spent -= spent_here;
        p.banked_ahead = 0;
    }

    p.node = e.dest;
}

// --- branch_activate -----------------------------------------------------
bool Engine::branch_activate(Path& p, Path* child) {
    const NodeView n = node(p.node);
    if (n.branch_edge < 0) return false;
    if (p.sp < kSpActivationBars) return false;

    const EdgeView e = edge(n.branch_edge);
    const ScoreGraphEdge* eo = eobj(n.branch_edge);

    const double e_offset = fill_e_offset(e.activation_fill_deadline_ms, p.sp_ready_ms);
    if (fill_refuses(e_offset)) return false;

    // activation_initial_end_times, keyed by SP meter. The flat form was a list
    // with NO_TIME gaps in range [0, top]; here the map has meters
    // kSpActivationBars..max.
    int64_t aiet_val = NO_TIME;
    bool in_range = false;
    if (!eo->activation_initial_end_times.empty()) {
        int top = eo->activation_initial_end_times.rbegin()->first;
        if (p.sp >= 0 && p.sp <= top) {
            in_range = true;
            auto ait = eo->activation_initial_end_times.find(p.sp);
            aiet_val = (ait == eo->activation_initial_end_times.end())
                           ? NO_TIME
                           : ait->second.ticks();
        }
    }
    if (!in_range || aiet_val == NO_TIME) {
        p.node = NODE_BROKEN;
        return false;
    }

    Path c = p;
    c.node = e.dest;
    c.currentskips = 0;
    c.sp = 0;

    close_last_activation(c);

    c.act_tail = new_act(p.act_tail, p.node, p.currentskips,
                         recorded_e_offset(p.skipped_e_offset, e_offset));
    acts_[(size_t)c.act_tail].bank_tail = p.bank_tail;
    c.bank_tail = -1;
    acts_[(size_t)c.act_tail].skip_tail = p.skip_tail;
    c.skip_tail = -1;
    c.sc[2] += e.frontend_points;
    c.score += e.frontend_points;
    c.skipped_e_offset = NO_DOUBLE;
    c.sp_ready_ms = NO_DOUBLE;
    c.sp_end_time = aiet_val;
    c.end_tail = push_end(-1, n.tick, aiet_val, SpEndKind::Activation);
    c.banked_phrase_ordinal = e.banked_phrase_ordinal;

    p.currentskips += 1;
    // The fill just passed over, for the record: the Preview lights it.
    p.skip_tail = push_tick(fills_, p.skip_tail, n.tick);

    if (!has_value(p.skipped_e_offset)) p.skipped_e_offset = e_offset;

    *child = c;
    return true;
}

int32_t Engine::deactivation_type(const EdgeView& e, const Path& p,
                                  const ChoiceView** offered) const {
    // The edge lists the phrase chords in this SP end's window. The path is
    // offered at most one (core::offered_phrase, D36): its newest phrase,
    // when that phrase's step moved its end from this node (EndNode::
    // sqout_at) and it has not squeezed that phrase in, or, when its end is
    // this node, the first phrase after it that it has not squeezed in
    // (D34). A late SqIn's phrase still ahead of the path (Path::spent)
    // holds its SqIn step already, so it is skipped (D32: that phrase was
    // once squeezed in a second time at the next end and the search broke).
    // The newest-step rule also tells apart two ends that move to one tick:
    // two clamped to one ceiling, or a plain bar tying it (finding 37), or
    // two ends a tick apart (plusmeasure's rounding). Each offers the phrase
    // only to the path that moved from it. The graph keeps an end as a node,
    // and lists the phrase on its edge, exactly when the step says so
    // (SpExtension::sqout_node), so offered_phrase is asked on an empty
    // list too: it throws when the step's phrase is missing.
    const int64_t d = node(e.dest).tick;
    const EndNode& newest = ends_[(size_t)p.end_tail];
    const ChoiceView* first = en_.choices.data() + e.choice_begin;
    const ChoiceView* last = en_.choices.data() + e.choice_end;
    const ChoiceView* c = core::offered_phrase(
        first, last, d, p.sp_end_time, newest.tick,
        newest.sqout_at == NO_TIME ? std::nullopt : std::optional<int64_t>(newest.sqout_at),
        [](const ChoiceView& v) { return v.chord; },
        [this, &p](int64_t tick) { return squeezed_in(p, tick); });
    if (c != last) {
        *offered = c;
        return DEACT_SQINOUT;
    }
    // Otherwise SP ends here exactly when the path's end is this node. D32:
    // that includes a path whose end is this node while the window holds a
    // chord it could squeeze. At a normal tempo no such path exists: its end
    // sits at least one SP bar past that chord, and one bar outlasts the
    // squeeze window. When it does not, the path must still end here; before
    // D32 it ran on past its own end and broke the search.
    return p.sp_end_time == node(e.dest).tick ? DEACT_NORMAL : DEACT_NONE;
}

// --- create_deactivated_path ---------------------------------------------
void Engine::create_deactivated_path(const Path& p, Path* child, const ChoiceView* sq) {
    const int32_t deact_edge = node(p.node).branch_edge;
    const EdgeView e = edge(deact_edge);
    const ScoreGraphEdge* eo = eobj(deact_edge);

    Path c = p;
    c.node = e.dest;
    c.sp = sq ? 1 : 0;
    c.sp_end_time = NO_TIME;
    c.end_tail = -1;
    // A squeeze-out banks one bar when the player hits the phrase: just after
    // SP ends for an early phrase, on its own tick for a late one.
    const int64_t sp_end = node(e.dest).tick;
    c.bank_tail = sq ? push_tick(banks_, -1, core::after_sp_end(sq->chord, sp_end) ? sq->chord : sp_end)
                     : -1;

    c.act_tail = clone_tail(p.act_tail);
    if (c.act_tail >= 0) {
        Act& a = acts_[(size_t)c.act_tail];
        a.deact_edge = deact_edge;
        // A squeeze-out gives back its phrase and everything after it, so
        // the activation keeps only the steps before it.
        // D36: what is left ends on this node. An early squeeze-out gives
        // back only its own step, the newest, which moved the end from here.
        // rebuild checks it once for every stored window, variants included.
        a.end_tail = sq ? trim_ends(p.end_tail, sq->chord) : p.end_tail;
        if (sq) {
            a.sq_tail = push_sq(a.sq_tail, SQ_OUT, sq->timing);
            a.sqout_phrase = sq->chord;
        }
    }

    // Each row adds what it is worth on this path minus what the SP walk
    // already paid for it. The walk pays rows at or before the SP end in
    // full; core::backend_row_value is the one place that says what a row
    // is worth, shared with the replay and the details table.
    const std::optional<int64_t> sqout_phrase =
        sq ? std::optional<int64_t>(sq->chord) : std::nullopt;
    int32_t sp_delta = 0;
    // The edge's rows are a range of the graph's rows, read in place rather
    // than copied (ScoreGraph::edge_backends copies them for the record).
    for (int32_t bi = eo->backend_begin; bi < eo->backend_end; ++bi) {
        const BackendSqueeze& beo = (*en_.all_backends)[(size_t)bi];
        const double be_offset = edge_row_offset(*eo, beo);
        const core::SqOutPosition pos =
            core::sqout_position(beo.timecode.ticks(), sqout_phrase);
        const int32_t already_paid =
            core::paid_by_sp_walk(be_offset) ? beo.points : 0;
        sp_delta += core::backend_row_value(be_offset, beo.points,
                                            beo.sqout_points, pos,
                                            backend_leeway_ms_) -
                    already_paid;
    }

    if (sp_delta) {
        c.sc[2] += sp_delta;
        c.score += sp_delta;
    }

    *child = c;
}

// --- branch_deactivate ---------------------------------------------------
bool Engine::branch_deactivate(Path& p, Path* child, bool* has_child) {
    *has_child = false;

    const NodeView n = node(p.node);
    if (n.branch_edge < 0) return true;

    const EdgeView e = edge(n.branch_edge);
    const ChoiceView* sq = nullptr;
    const int32_t deact_type = deactivation_type(e, p, &sq);

    if (deact_type == DEACT_NONE) return true;

    if (deact_type == DEACT_NORMAL) {
        create_deactivated_path(p, child, nullptr);
        *has_child = true;
        return false;
    }

    create_deactivated_path(p, child, sq);
    *has_child = true;

    p.act_tail = clone_tail(p.act_tail);
    if (p.act_tail >= 0) {
        Act& a = acts_[(size_t)p.act_tail];
        a.sq_tail = push_sq(a.sq_tail, SQ_IN, sq->timing);
    }

    // The squeeze-in's step. A late one's phrase comes after this node and is
    // spent, so advance skips it: its step is written here, on the phrase.
    // An early one's phrase is already a Collected step (advance collected it
    // before this branch point, and the SqOut child above shares that step):
    // relabel it on this branch only. Its end is already the moved one (D36).
    if (sq->late) {
        p.end_tail = push_end(p.end_tail, sq->chord, sq->sqin_time, SpEndKind::SqIn);
        p.sp_end_time = sq->sqin_time;
    } else if (!relabel_sqin(p.end_tail, sq->chord)) {
        p.node = NODE_BROKEN;  // run() sees it and refuses the search
        return false;
    }

    // The path may already hold a late SqIn's phrase ahead of it, spent, when
    // this one is offered after it (D34). Both branches keep those spent. A
    // late phrase is the next one in line: this branch spends it too, and the
    // SqOut child holds it banked (its bar is already in the child's meter).
    child->spent = p.spent;
    child->banked_ahead = sq->late;
    p.spent += sq->late;
    return true;
}

// --- difficulty ----------------------------------------------------------
// The engine's own walk of an activation's timings (audit finding 152). Each
// squeeze answers through SPSqueeze::within_limit and the early fill through
// fill_within_limit, the pieces Activation::within_ms_limit asks too.
bool Engine::act_within_limit(int32_t act) const {
    if (act < 0 || !has_ms_filter_) return true;
    const Act& a = acts_[(size_t)act];
    for (int32_t s = a.sq_tail; s >= 0; s = sqs_[(size_t)s].prev) {
        const bool is_in = sqs_[(size_t)s].kind == SQ_IN;
        const SPSqueeze sq{is_in ? SqueezeKind::SqIn : SqueezeKind::SqOut, sqs_[(size_t)s].offset};
        if (!sq.within_limit(ms_filter_)) return false;
    }
    return fill_within_limit(a.e_offset, a.skips, ms_filter_);
}

void Engine::close_last_activation(Path& p) const {
    if (!act_within_limit(p.act_tail)) p.over_prefix = true;
}

bool Engine::passes_ms_filter(const Path& p) const {
    return !p.over_prefix && act_within_limit(p.act_tail);
}

// --- reduce_group --------------------------------------------------------

// How many distinct scores in `sorted` (ascending, no repeats) are above
// `score`.
static int32_t scores_above(const std::vector<int64_t>& sorted, int64_t score) {
    return (int32_t)(sorted.end() - std::upper_bound(sorted.begin(), sorted.end(), score));
}

// The depth band the user chose, said once: a path is outside it when it is
// more than depth_value_ points under `best` ("Within N points"), or when more
// than depth_value_ distinct scores outscore it ("Within N scores").
// `outscored_by` counts those scores in the same list `best` tops.
bool Engine::outside_depth_band(int64_t score, int64_t best, int32_t outscored_by) const {
    if (depth_mode_ == DepthMode::Points) return score + depth_value_ < best;
    return outscored_by > depth_value_;
}

// Whether a path's score may eliminate another path in its group: it is
// inside the Path limit, or it ties the optimal score (D51 call 2: an
// over-limit path that ties the optimal score stays kept).
bool Engine::can_outscore(int32_t idx) const {
    return !filtered_[(size_t)idx] ||
           (has_optimal_ && cur_[(size_t)idx].score == optimal_score_);
}

// The best score among the group's paths that may eliminate a path
// (can_outscore), or none when no path in the group may.
std::optional<int64_t> Engine::best_eligible_score(const int32_t* members, int32_t n) const {
    std::optional<int64_t> top;
    for (int32_t k = 0; k < n; ++k) {
        const int32_t idx = members[k];
        if (!can_outscore(idx)) continue;
        if (!top || cur_[(size_t)idx].score > *top) top = cur_[(size_t)idx].score;
    }
    return top;
}

void Engine::reduce_group(const int32_t* members, int32_t n) {
    // An over-limit path below the group's best eligible score is beaten
    // (D55 item 5), whether it ties an inside leader or leads its own score.
    // Both places that drop one ask `outscored`.
    const std::optional<int64_t> top = best_eligible_score(members, n);
    const auto outscored = [&top](int64_t score) { return top && score < *top; };

    // Fold ties. D51 call 1: the tie limit (max_tied_paths_) is one count per
    // score, so a complete path inside the Path limit meets a complete
    // over-limit path at the same score. The inside paths are offered first
    // and the over-limit ones after, so an inside path leads whenever one
    // exists, and when the limit is reached the paths left out are over-limit
    // ones. Each side keeps the group's own order.
    // D55 item 5: a running over-limit path keeps a key of its own. A later
    // group may still outscore it, and filed under an inside path as a
    // variant it could no longer be dropped then. It meets the inside paths
    // at its score once it is complete, where scores are final.
    survivors_.clear();
    tie_map_.reset((size_t)n);
    for (int32_t k = 0; k < 2 * n; ++k) {
        const int32_t idx = members[k % n];
        const bool over_limit_pass = k >= n;
        if ((filtered_[(size_t)idx] != 0) != over_limit_pass) continue;
        const bool apart = over_limit_pass && cur_[(size_t)idx].node >= 0;
        const uint64_t key = ((uint64_t)cur_[(size_t)idx].score << 1) | (apart ? 1ull : 0ull);

        bool inserted = false;
        const int32_t leader_idx = tie_map_.get_or_insert(key, idx, &inserted);
        if (inserted) {
            survivors_.push_back(idx);
            continue;
        }

        Path& leader = cur_[(size_t)leader_idx];
        const Path& p = cur_[(size_t)idx];
        // D55 item 5: a complete over-limit path tied with an inside leader
        // rides as its variant only at the top score. Below it the path is
        // dropped, as it would be if it led its own score. An over-limit
        // leader that is beaten goes in the final loop below, after its score
        // has counted toward the depth band.
        if (over_limit_pass && !filtered_[(size_t)leader_idx] && outscored(p.score)) {
            removed_[(size_t)idx] = 1;
            continue;
        }
        // Two running paths whose newest phrase can still be squeezed out at
        // different SP ends never meet here: the group key holds that end
        // (pending_sqout_at, D36), so a variant never takes its leader's
        // squeeze (the tied-clamps test, finding 37).
        if (leader.tied_count + p.tied_count <= max_tied_paths_) {
            Variant v;
            v.prev = leader.var_head;
            v.var_point = act_count(leader);
            v.act_tail = p.act_tail;
            v.var_head = p.var_head;
            v.tied_count = p.tied_count;
            v.sp_end = p.sp_end_time;
            v.end_tail = p.end_tail;
            // Both paths stand on the same node. On an SP node the window is
            // still open, and the leader will close it for both.
            v.open_sp = p.node >= 0 && node(p.node).is_sp && leader.act_tail >= 0;
            v.fold_tick = p.node >= 0 ? node(p.node).tick : NO_TIME;
            v.fold_sq_count =
                v.open_sp ? sq_count(acts_[(size_t)leader.act_tail].sq_tail) : 0;
            v.finished = p.node < 0;
            v.bank_tail = p.bank_tail;
            v.sp_ready_ms = p.sp_ready_ms;
            v.skip_tail = p.skip_tail;
            v.skipped_e_offset = p.skipped_e_offset;
            variants_.push_back(v);
            leader.var_head = (int32_t)variants_.size() - 1;
            leader.tied_count += p.tied_count;
        }
        removed_[(size_t)idx] = 1;
    }

    // A lone leader has nothing to compete with.
    if (survivors_.size() < 2) return;

    // From here on each survivor is a leader with its tied variants riding
    // along. A complete leader is filtered only when every path at its score
    // is over the limit: an inside path at that score would have led. A
    // running over-limit leader may share its score with an inside one.

    // Distinct scores over the whole group, achievable or not. A filtered
    // leader stays only while it is within the depth band here -- while it
    // could still be the single best path, which is shown even when
    // unachievable. Without this band a filtered path is dropped only when an
    // achievable path beats it, and on an uncapped chart the achievable
    // frontier scores far below the hard paths, so they all survive and the
    // frontier explodes. The band collapses each group back to the depth
    // setting, as an unfiltered search does, and cannot drop the eventual
    // best path (score dominance keeps the top band at every step).
    dominating_.clear();
    for (size_t i = 0; i < survivors_.size(); ++i) {
        dominating_.push_back(cur_[(size_t)survivors_[i]].score);
    }
    std::sort(dominating_.begin(), dominating_.end());
    dominating_.erase(std::unique(dominating_.begin(), dominating_.end()),
                      dominating_.end());
    const int64_t best_all = dominating_.back();

    // The distinct scores allowed to eliminate an achievable path
    // (can_outscore), for the "Within N scores" count; `top` heads them. An
    // over-limit path riding as an inside leader's variant got here only at
    // `top`, and its leader is outscored by nothing either.
    // May be empty mid-search (every live path in this group is filtered):
    // there is then no achievable path to prune, and the band above still
    // reins the filtered ones in.
    beating_.clear();
    for (size_t i = 0; i < survivors_.size(); ++i) {
        const int32_t idx = survivors_[i];
        if (can_outscore(idx)) beating_.push_back(cur_[(size_t)idx].score);
    }
    std::sort(beating_.begin(), beating_.end());
    beating_.erase(std::unique(beating_.begin(), beating_.end()),
                   beating_.end());

    for (size_t i = 0; i < survivors_.size(); ++i) {
        const int32_t idx = survivors_[i];
        const int64_t score = cur_[(size_t)idx].score;
        bool drop;
        if (filtered_[(size_t)idx]) {
            // An over-limit leader goes as soon as an achievable score beats
            // it, else when it leaves the band over every score.
            drop = outscored(score) ||
                   outside_depth_band(score, best_all, scores_above(dominating_, score));
        } else {
            // An inside leader may eliminate, so `top` is set.
            drop = outside_depth_band(score, *top, scores_above(beating_, score));
        }
        if (drop) removed_[(size_t)idx] = 1;
    }
}

// --- reduce_iteration_paths ----------------------------------------------
void Engine::reduce_iteration_paths() {
    const int32_t n = (int32_t)cur_.size();

    filtered_.assign((size_t)n, 0);
    removed_.assign((size_t)n, 0);

    if (has_ms_filter_) {
        for (int32_t i = 0; i < n; ++i) {
            if (!passes_ms_filter(cur_[(size_t)i])) filtered_[(size_t)i] = 1;
        }
    }

    has_optimal_ = false;
    optimal_score_ = 0;

    owner_.assign((size_t)n, -1);
    group_map_.reset((size_t)n);
    int32_t n_groups = 0;

    for (int32_t i = 0; i < n; ++i) {
        const Path& p = cur_[(size_t)i];
        const bool is_complete = p.node < 0;

        // Hard mode kills an over-limit path here instead of handing it to
        // reduce_group, which keeps one while nothing outscores it -- a
        // preference, not a requirement. Dropping it now is exact:
        // over_prefix only ever gets set in close_last_activation, and a
        // tail's squeeze list only grows, so a path already outside the limit
        // can never come back inside it. It also prunes, since the whole
        // subtree below it is outside too.
        if (hard_ms_filter_ && filtered_[(size_t)i]) {
            removed_[(size_t)i] = 1;
            continue;
        }

        if (is_complete && (!has_optimal_ || p.score > optimal_score_)) {
            has_optimal_ = true;
            optimal_score_ = p.score;
        }

        if (p.spent != 0 || p.banked_ahead != 0) continue;

        // Group by what decides the path's future: its SP meter, or its SP end
        // while active. While waiting, also which upcoming fills its SP-ready
        // time decides (ready_class), not the time itself: branch_activate
        // reads it for the early-fill rule, and a higher-scoring path that
        // became ready later must not knock out one an upcoming fill still
        // spawns for. The raw time in the key splits paths no fill tells
        // apart; over the library it grew the frontier 13-22x (2026-10).
        // This key found the same best paths (7 higher at cap 2, none at
        // cap 4) with the frontier 0.15% larger at cap 2 and unchanged at 4.
        // While SP runs, the activation's banked phrase in squeeze reach is
        // part of that future too: two activations that differ there can
        // face different squeeze choices at the same SP end
        // (core::banked_phrase_in_reach). It is 0 on almost every path, which
        // leaves the groups as they were. The key keeps the SP end's low 47
        // bits, the ordinal (a phrase count) in 16 and the SP flag in 1. Any
        // 2^47 consecutive values differ in their low 47 bits, so an end in
        // [-2^46, 2^46) packs exactly; the check below refuses the rest.
        // These widths, the meter's 2^30 and ready_class's 16 bits per count
        // are decision D40 (recorded in ADR 0014).
        // While SP runs, the key's second word is the SP end where the
        // path's newest phrase can still be squeezed out, while that end is
        // ahead (pending_sqout_at, D36): the one other fact the squeeze rule
        // reads. Two paths at one node with one SP end and one such end face
        // the same offers, because their newest phrase is the same. It is
        // NO_TIME on almost every path, which leaves the groups as they were.
        // A second word, not more bits in the first: no lossy packing.
        const bool is_sp = !is_complete && node(p.node).is_sp;
        const uint64_t key2 = is_sp ? (uint64_t)pending_sqout_at(p, node(p.node).tick)
                                    : (uint64_t)NO_TIME;
        const int64_t sp_value =
            is_complete ? 0 : (is_sp ? p.sp_end_time : (int64_t)p.sp);
        uint64_t key_value = (uint64_t)sp_value;
        if (is_sp) {
            if (sp_value < -(int64_t(1) << 46) || sp_value >= (int64_t(1) << 46) ||
                p.banked_phrase_ordinal < 0 || p.banked_phrase_ordinal > 0xFFFF)
                throw std::logic_error("search group key out of range");
            key_value = (key_value << 16) | (uint64_t)p.banked_phrase_ordinal;
        } else if (!is_complete) {
            // The meter in the high bits, ready_class's 32 below it.
            if (p.sp < 0 || p.sp >= (1 << 30))
                throw std::logic_error("search group key out of range");
            key_value = (key_value << 32) | ready_class(p);
        }
        const uint64_t key = (key_value << 1) | (is_sp ? 1ull : 0ull);

        bool inserted = false;
        const int32_t g = group_map_.get_or_insert(key, key2, n_groups, &inserted);
        if (inserted) ++n_groups;
        owner_[(size_t)i] = g;
    }

    counts_.assign((size_t)n_groups, 0);
    for (int32_t i = 0; i < n; ++i) {
        if (owner_[(size_t)i] >= 0) ++counts_[(size_t)owner_[(size_t)i]];
    }
    group_begin_.assign((size_t)n_groups, 0);
    group_end_.assign((size_t)n_groups, 0);
    int32_t running = 0;
    for (int32_t g = 0; g < n_groups; ++g) {
        group_begin_[(size_t)g] = running;
        group_end_[(size_t)g] = running;
        running += counts_[(size_t)g];
    }
    group_members_.assign((size_t)running, 0);
    for (int32_t i = 0; i < n; ++i) {
        const int32_t g = owner_[(size_t)i];
        if (g >= 0) group_members_[(size_t)group_end_[(size_t)g]++] = i;
    }

    for (int32_t g = 0; g < n_groups; ++g) {
        const int32_t begin = group_begin_[(size_t)g], end = group_end_[(size_t)g];
        if (end - begin > 1) {
            reduce_group(&group_members_[(size_t)begin], end - begin);
        }
    }

    int32_t w = 0;
    for (int32_t i = 0; i < n; ++i) {
        if (!removed_[(size_t)i]) {
            if (w != i) cur_[(size_t)w] = cur_[(size_t)i];
            ++w;
        }
    }
    cur_.resize((size_t)w);
}

// --- output --------------------------------------------------------------
void Engine::emit_acts(int32_t act_tail, int64_t sp_end_time, int32_t end_tail,
                       int32_t* begin, int32_t* end) {
    chain_scratch_.clear();
    for (int32_t a = act_tail; a >= 0; a = acts_[(size_t)a].parent) {
        chain_scratch_.push_back(a);
    }

    *begin = (int32_t)out_acts_.size();
    for (size_t i = chain_scratch_.size(); i-- > 0;) {
        const Act& a = acts_[(size_t)chain_scratch_[i]];

        sq_scratch_.clear();
        for (int32_t s = a.sq_tail; s >= 0; s = sqs_[(size_t)s].prev) {
            sq_scratch_.push_back(s);
        }

        OutAct oa;
        oa.act_node = a.act_node;
        oa.deact_edge = a.deact_edge;
        oa.e_offset = a.e_offset;
        oa.sq_begin = (int32_t)out_sqs_.size();
        for (size_t k = sq_scratch_.size(); k-- > 0;) {
            OutSq os;
            os.kind = sqs_[(size_t)sq_scratch_[k]].kind;
            os.offset = sqs_[(size_t)sq_scratch_[k]].offset;
            out_sqs_.push_back(os);
        }
        oa.sq_end = (int32_t)out_sqs_.size();
        oa.final_sp_end = NO_TIME;
        oa.sqout_phrase = a.sqout_phrase;
        emit_ends(a.end_tail, &oa.end_begin, &oa.end_end);
        emit_ticks(banks_, a.bank_tail, &oa.bank_begin, &oa.bank_end);
        emit_ticks(fills_, a.skip_tail, &oa.skip_begin, &oa.skip_end);
        out_acts_.push_back(oa);
    }
    *end = (int32_t)out_acts_.size();

    // The newest activation is the only one that can still be running at the
    // end of the song. When it is (no deact edge), hand the rebuild step the
    // SP end the search tracked, so it can measure the trailing notes.
    if (*end > *begin) {
        OutAct& last = out_acts_[(size_t)(*end - 1)];
        if (last.deact_edge < 0) {
            last.final_sp_end = sp_end_time;
            // Still running at the song's end: its steps are on the live path.
            emit_ends(end_tail, &last.end_begin, &last.end_end);
        }
    }
}

// D3: the variant was folded while its last activation's SP was running. From
// the fold node on, both paths met the same notes with the same SP end, so the
// leader's closing of that window is the variant's: its later SP end steps,
// its deactivation and (D7) its closing squeeze. Before the fold the variant
// keeps its own: activation, bank, skips, early-fill offset, squeezes, steps.
// `own` and `lead` index out_acts_. `lead` is the same window on the path the
// variant folded into, already closed.
void Engine::close_folded_act(int32_t own_i, int32_t lead_i, const Variant& var) {
    // Work on copies and write back once. Each entry is copied to a local
    // before its push, because a push can reallocate the vector it reads.
    const OutAct lead = out_acts_[(size_t)lead_i];
    OutAct own = out_acts_[(size_t)own_i];

    own.deact_edge = lead.deact_edge;
    own.sqout_phrase = lead.sqout_phrase;
    // Set only when the leader's SP outlasted the chart: the end it tracked.
    own.final_sp_end = lead.final_sp_end;

    const int32_t sq_begin = (int32_t)out_sqs_.size();
    for (int32_t k = own.sq_begin; k < own.sq_end; ++k) {
        const OutSq s = out_sqs_[(size_t)k];
        out_sqs_.push_back(s);
    }
    for (int32_t k = lead.sq_begin + var.fold_sq_count; k < lead.sq_end; ++k) {
        const OutSq s = out_sqs_[(size_t)k];
        out_sqs_.push_back(s);
    }
    own.sq_begin = sq_begin;
    own.sq_end = (int32_t)out_sqs_.size();

    // A closing SqOut gives back its phrase and the steps given_back names,
    // the trim create_deactivated_path makes (trim_ends). The leader's
    // sqout_phrase is the squeezed-out chord.
    int64_t give_back = NO_TIME;
    for (int32_t k = own.sq_begin; k < own.sq_end; ++k)
        if (out_sqs_[(size_t)k].kind == SQ_OUT && own.deact_edge >= 0)
            give_back = own.sqout_phrase;

    // An early SqIn the leader took after the fold may sit on a phrase from
    // before the fold. branch_deactivate relabels that step SqIn on the
    // leader (relabel_sqin); the variant's own step there gets the same label.
    // The variant always holds that step: it ran SP over the phrase, so the
    // gauge received it. The step is Collected, or Clamped when the variant's
    // longer meter let the cap pin its end there (the clamped_sqin test
    // charts). is_sqin_step accepts both, as relabel_sqin does on a lone
    // path. A path that banked the phrase before activating cannot squeeze it
    // (core::activation_can_squeeze), and the search never folds it into one
    // that can (banked_phrase_ordinal in the group key; the folded_sqin test
    // charts), so no variant here lacks the step. The n-th SqIn in the
    // leader's list owns its n-th SqIn step, so the SqIns from the fold on
    // own the leader's SqIn steps from that rank on.
    const auto lead_sqs = out_sqs_.begin() + lead.sq_begin;
    const size_t sqins_before_fold =
        sqin_rank(lead_sqs, lead_sqs + var.fold_sq_count,
                  [](const auto& sq) { return sq.kind == SQ_IN; });
    std::vector<int64_t> relabel_at;
    const auto lead_last = out_ends_.begin() + lead.end_end;
    for (auto s = nth_sqin_step(out_ends_.begin() + lead.end_begin, lead_last, sqins_before_fold);
         s != lead_last; ++s)
        if (is_sqin_kind(s->kind) && s->tick <= var.fold_tick) relabel_at.push_back(s->tick);

    const int32_t end_begin = (int32_t)out_ends_.size();
    for (int32_t k = own.end_begin; k < own.end_end; ++k) {
        SpEndStep s = out_ends_[(size_t)k];
        if (s.tick > var.fold_tick)
            throw std::logic_error("a folded variant holds a step past its fold");
        if (give_back != NO_TIME && given_back(s.tick, give_back)) continue;
        const auto sqin =
            std::find_if(relabel_at.begin(), relabel_at.end(),
                         [&s](int64_t t) { return is_sqin_step(s.tick, s.kind, t); });
        if (sqin != relabel_at.end()) {
            s.kind = SpEndKind::SqIn;
            relabel_at.erase(sqin);
        }
        out_ends_.push_back(s);
    }
    if (!relabel_at.empty())
        throw std::logic_error("a folded variant holds no SqIn step on its leader's SqIn phrase");
    for (int32_t k = lead.end_begin; k < lead.end_end; ++k) {
        const SpEndStep s = out_ends_[(size_t)k];
        if (s.tick > var.fold_tick) out_ends_.push_back(s);
    }
    own.end_begin = end_begin;
    own.end_end = (int32_t)out_ends_.size();

    out_acts_[(size_t)own_i] = own;
}

// D38: the leader's next activation, as the variant folded between windows
// reaches it. From the fold on both paths met the same notes and passed the
// same fills, so the activation itself and the fills passed after the fold
// are shared. Before the fold the variant had its own SP-ready time and its
// own passed fills, so its early-fill facts are its own: its passed fills,
// then the shared ones, and the e_offset the early-fill rule
// (core/model.h) gives its ready time at the first fill it passed, or at the
// activation's fill when it passed none.
void Engine::own_early_fill(const Variant& var, OutAct* next) {
    std::vector<int64_t> shared;
    for (int32_t k = next->skip_begin; k < next->skip_end; ++k)
        if (out_ticks_[(size_t)k] > var.fold_tick) shared.push_back(out_ticks_[(size_t)k]);

    // The offset at a fill, from the variant's own ready time.
    const auto at_ready = [&](double deadline) {
        return has_value(var.sp_ready_ms) ? fill_e_offset(deadline, var.sp_ready_ms) : NO_DOUBLE;
    };
    // The first fill it passed is its own, else the first shared one.
    const double first_passed =
        has_value(var.skipped_e_offset) ? var.skipped_e_offset
        : shared.empty()                ? NO_DOUBLE
                                        : at_ready(fill_deadline_at(shared.front()));
    const double own = at_ready(edge(node(next->act_node).branch_edge).activation_fill_deadline_ms);
    if (has_value(first_passed) || has_value(own)) {
        next->e_offset = recorded_e_offset(first_passed, own);
    } else if (var.skip_tail >= 0 || (int32_t)shared.size() != next->skip_end - next->skip_begin) {
        // Under kSpActivationBars at the fold, as its leader was (same meter): neither
        // could have passed a fill yet, and both became ready later, at the
        // same phrase. The leader's offset is the variant's.
        throw std::logic_error("a variant under kSpActivationBars at its fold passed a fill");
    }

    emit_ticks(fills_, var.skip_tail, &next->skip_begin, &next->skip_end);
    for (const int64_t t : shared) out_ticks_.push_back(t);
    next->skip_end = (int32_t)out_ticks_.size();
}

void Engine::emit_variant(int32_t v, int32_t depth, const std::vector<int32_t>& parent_walk,
                          int32_t parent_trail_begin, int32_t parent_trail_end) {
    std::vector<int32_t> order;
    for (int32_t i = v; i >= 0; i = variants_[(size_t)i].prev)
        order.push_back(i);

    for (size_t k = order.size(); k-- > 0;) {
        const Variant& var = variants_[(size_t)order[k]];
        // A variant ties its parent's score, and prepare_variants copies the
        // parent's score totals and note count onto it, so the engine hands
        // none of those (docs/adr/0017). Its banked bars are its own (D3,
        // finding 89), set below.
        OutPath op{};
        op.var_point = var.var_point;
        op.depth = depth;
        emit_acts(var.act_tail, var.sp_end, var.end_tail, &op.act_begin, &op.act_end);
        // At the fold the leader held var_point activations and its newest was
        // the open one. In the leader's final walk that window, now closed,
        // sits at var_point - 1.
        if (var.open_sp) {
            if (op.act_end <= op.act_begin || var.var_point < 1 ||
                (size_t)var.var_point > parent_walk.size())
                throw std::logic_error("a variant folded mid-SP has no window to close");
            close_folded_act(op.act_end - 1, parent_walk[(size_t)var.var_point - 1], var);
        }

        // The variant's banked bars (D3, finding 89). Which are its own
        // depends on where it folded.
        if (var.finished) {
            // Both had finished the song: its bank at the fold is its whole
            // trailing list, whatever the leader's holds.
            emit_ticks(banks_, var.bank_tail, &op.bank_begin, &op.bank_end);
        } else if (var.open_sp) {
            // Folded inside SP, where both banks are empty: every bar banked
            // later is shared.
            op.bank_begin = parent_trail_begin;
            op.bank_end = parent_trail_end;
        } else if ((size_t)var.var_point < parent_walk.size()) {
            // Folded between windows, and the leader activated again. That
            // activation spent the variant's own bars up to the fold, so the
            // variant stores its own copy of it and reads its parent from the
            // one after. The copy lands right after the variant's own
            // activations (nothing else is pushed to out_acts_ in between),
            // so [act_begin, act_end) stays one range. After it both banks
            // reset, so later activations stay shared.
            if (op.act_end != (int32_t)out_acts_.size())
                throw std::logic_error("a variant's activations are not one range");
            OutAct next = out_acts_[(size_t)parent_walk[(size_t)var.var_point]];
            splice_bank(var.bank_tail, next.bank_begin, next.bank_end, &next.bank_begin,
                        &next.bank_end);
            own_early_fill(var, &next);
            out_acts_.push_back(next);
            op.act_end = (int32_t)out_acts_.size();
            op.var_point = var.var_point + 1;
            op.bank_begin = parent_trail_begin;
            op.bank_end = parent_trail_end;
        } else {
            // Folded between windows; the leader never activated again.
            splice_bank(var.bank_tail, parent_trail_begin, parent_trail_end, &op.bank_begin,
                        &op.bank_end);
        }
        out_paths_.push_back(op);

        // This variant's activations as its own variants read them: its own,
        // then its parent's from its var_point on (prepare_variants' order).
        std::vector<int32_t> walk;
        for (int32_t j = op.act_begin; j < op.act_end; ++j) walk.push_back(j);
        for (size_t j = (size_t)op.var_point; j < parent_walk.size(); ++j)
            walk.push_back(parent_walk[j]);
        emit_variant(var.var_head, depth + 1, walk, op.bank_begin, op.bank_end);
    }
}

void Engine::emit_path(const Path& p) {
    // The search ranks by its running p.score; the record stores the six
    // categories. Every stored total passes through here (a variant copies its
    // leader's, ADR 0017), so this is where the two must agree (audit 165).
    const int64_t six = score_total(p.sc[0], p.sc[1], p.sc[2], p.sc[3], p.sc[4], p.sc[5]);
    if (p.score != six)
        throw std::logic_error("copy-out: the search's running score " +
                               std::to_string(p.score) +
                               " differs from its six categories' total " +
                               std::to_string(six));
    OutPath op;
    op.score_base = p.sc[0];
    op.score_combo = p.sc[1];
    op.score_sp = p.sc[2];
    op.score_solo = p.sc[3];
    op.score_accents = p.sc[4];
    op.score_ghosts = p.sc[5];
    op.var_point = -1;
    op.depth = 0;
    op.tied_count = p.tied_count;
    emit_ticks(banks_, p.bank_tail, &op.bank_begin, &op.bank_end);
    emit_acts(p.act_tail, p.sp_end_time, p.end_tail, &op.act_begin, &op.act_end);
    out_paths_.push_back(op);

    std::vector<int32_t> walk;
    for (int32_t j = op.act_begin; j < op.act_end; ++j) walk.push_back(j);
    emit_variant(p.var_head, 1, walk, op.bank_begin, op.bank_end);
}

// --- BFS driver ----------------------------------------------------------
bool Engine::run() {
    Path root;
    std::memset(&root, 0, sizeof(root));
    root.node = en_.start;
    root.act_tail = -1;
    root.var_head = -1;
    root.tied_count = 1;
    root.sp_end_time = NO_TIME;
    root.end_tail = -1;
    root.bank_tail = -1;
    root.skip_tail = -1;
    root.sp_ready_ms = NO_DOUBLE;
    root.skipped_e_offset = NO_DOUBLE;
    root.over_prefix = false;

    cur_.clear();
    cur_.push_back(root);

    for (;;) {
        bool any_live = false;
        int32_t furthest = 0;
        for (size_t i = 0; i < cur_.size(); ++i) {
            if (cur_[i].node >= 0) {
                any_live = true;
                if (cur_[i].node > furthest) furthest = cur_[i].node;
            }
        }
        if (!any_live) break;

        // Report how far the frontier has advanced through the chart. Node
        // indices increase along the timeline, so the furthest live node over
        // the node count is a fair progress fraction; clamp it monotonic so a
        // finishing lead path can't make the bar step backward.
        if (progress_cb_ && !en_.nodes.empty()) {
            float f = static_cast<float>(furthest) / static_cast<float>(en_.nodes.size());
            // Report every half percent: fine enough for a smooth bar, and it
            // bounds the callback (which may throw to cancel) to 200 calls.
            if (f >= progress_reported_ + 0.005f) {
                progress_reported_ = f;
                progress_cb_(f);
            }
        }

        next_.clear();
        next_.reserve(cur_.size() * 2);

        for (size_t i = 0; i < cur_.size(); ++i) {
            Path p = cur_[i];
            advance(p);
            if (p.node == NODE_BROKEN) return false;

            if (p.node < 0) {
                next_.push_back(p);
                continue;
            }

            Path child;
            bool has_child = false;
            if (node(p.node).is_sp) {
                const bool can_extend = branch_deactivate(p, &child, &has_child);
                if (p.node == NODE_BROKEN) return false;
                if (can_extend) next_.push_back(p);
            } else {
                // Read the fill's tick before branching: a refused activation
                // can leave p.node as NODE_BROKEN.
                const int64_t fill_tick = node(p.node).tick;
                has_child = branch_activate(p, &child);
                if (p.node == NODE_BROKEN) return false;
                if (target_act_ticks_) {
                    // A targeted search follows the caller's activation set and
                    // nothing else. On a listed fill only the activating child
                    // survives; if the engine refused that activation there is
                    // no child and this path simply dies. On any other fill only
                    // the declining parent survives -- branch_activate has
                    // already charged it the skip, which is the right
                    // bookkeeping, since skips count fills passed while
                    // activation was possible.
                    const bool wanted =
                        std::binary_search(target_act_ticks_->begin(),
                                           target_act_ticks_->end(), fill_tick);
                    if (!wanted) {
                        next_.push_back(p);
                        has_child = false;
                    }
                } else if (!(no_skips_ && has_child)) {
                    // The parent is the path that declined this opportunity, and
                    // branch_activate has just charged it a skip. Under no_skips_
                    // that parent can no longer reach an all-0 path, so drop it and
                    // keep only the activating child. branch_activate charges the
                    // skip solely on the branch that produced a child -- a refused
                    // opportunity (no branch edge, SP under kSpActivationBars, blown fill
                    // deadline) leaves currentskips alone, so the surviving path is
                    // still free to activate later and still read as 0 skips.
                    next_.push_back(p);
                }
            }
            if (has_child) next_.push_back(child);
        }

        cur_.swap(next_);
        reduce_iteration_paths();

        if (cur_.empty()) return false;
    }

    std::vector<int32_t> order(cur_.size());
    for (size_t i = 0; i < order.size(); ++i) order[i] = (int32_t)i;
    std::stable_sort(order.begin(), order.end(), [this](int32_t a, int32_t b) {
        return cur_[(size_t)a].score > cur_[(size_t)b].score;
    });

    for (size_t i = 0; i < order.size(); ++i)
        emit_path(cur_[(size_t)order[i]]);
    return true;
}

// ---- rebuild the decision log into core Paths (core/model.h) -------------
// Inflates the engine's flat decision log back into hydra::Path objects,
// reading the graph objects straight off the enumeration.

// One rebuilt path plus its variant children, referenced by index so the pool
// can reallocate freely.
struct BuildNode {
    MPath path;
    std::vector<int> children;
};

std::vector<MPath> rebuild(const ScoreGraph& graph, const Enum& en,
                           const std::vector<OutPath>& out_paths,
                           const std::vector<OutAct>& out_acts,
                           const std::vector<OutSq>& out_sqs,
                           const std::vector<SpEndStep>& out_ends,
                           const std::vector<int64_t>& out_ticks,
                           const std::vector<BackendSqueeze>& tail_backends,
                           const SongTiming& timing, int note_count) {
    std::vector<BuildNode> pool;
    pool.reserve(out_paths.size());
    std::vector<int> top_level;
    // The engine's running tie count for each root, in top_level's order.
    std::vector<int32_t> engine_tied;
    std::vector<int> by_depth;

    for (const OutPath& op : out_paths) {
        MPath path;
        path.score_base = op.score_base;
        path.score_combo = op.score_combo;
        path.score_sp = op.score_sp;
        path.score_solo = op.score_solo;
        path.score_accents = op.score_accents;
        path.score_ghosts = op.score_ghosts;
        // Every path covers the whole chart, so its note total is the
        // chart's (ScoreGraph::note_count).
        path.notecount = note_count;
        path.trailing_bank_ticks.assign(out_ticks.begin() + op.bank_begin,
                                        out_ticks.begin() + op.bank_end);

        for (int j = op.act_begin; j < op.act_end; ++j) {
            const OutAct& oa = out_acts[(size_t)j];
            const ScoreGraphNode* node = en.nodes[(size_t)oa.act_node];

            Activation act;
            act.timecode = node->timecode;
            // An activation node is a chart note, so it always carries the
            // chord hit there.
            act.chord = node->chord.value();
            act.bank_rise_ticks.assign(out_ticks.begin() + oa.bank_begin,
                                       out_ticks.begin() + oa.bank_end);
            act.skipped_fill_ticks.assign(out_ticks.begin() + oa.skip_begin,
                                          out_ticks.begin() + oa.skip_end);
            act.frontend_points = node->branch_edge->frontend_points;
            act.e_offset = oa.e_offset;
            if (oa.deact_edge >= 0) {
                act.backends = graph.edge_backends(*en.edges[(size_t)oa.deact_edge]);
            } else if (oa.final_sp_end != NO_TIME) {
                // SP outlasted the chart, so no deact edge was ever built and
                // no edge holds these notes. Measure the song's trailing notes
                // against the SP end the search tracked -- the same offset rule
                // add_deact_edge uses, just against a node the graph never made.
                const double end_ms = timing.ms_index().at(oa.final_sp_end);
                for (const BackendSqueeze& b : tail_backends) {
                    const double off = offset_from_sp_end(b.timecode.ms(), end_ms);
                    // D5: the same window as every other row, from this
                    // activation's own SP end, not from the song's last note.
                    if (!within_squeeze_window(off)) continue;
                    BackendSqueeze copy = b;
                    copy.offset_ms = off;
                    act.backends.push_back(copy);
                }
            }
            // The SqIns go in here. A SqOut is always the last squeeze, and
            // set_sqout below builds its entry from its row, so its offset is
            // stored once. The search ranked the path by its own SqOut offset
            // (its squeeze choice's timing), so keep it to check against the row.
            bool took_sqout = false;
            double searched_sqout_ms = 0.0;
            for (int k = oa.sq_begin; k < oa.sq_end; ++k) {
                const OutSq& os = out_sqs[(size_t)k];
                if (os.kind == SQ_OUT) {
                    took_sqout = true;
                    searched_sqout_ms = os.offset;
                    continue;
                }
                act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, os.offset});
            }

            // A squeezed-out note is hit after SP has already ended, and every
            // note past it is hit after it, so nothing beyond the sqout note
            // can be a backend squeeze -- there is no way to still be in SP
            // for it. The deact edge's backend list is shared by every path
            // that deactivates there, squeezing out or not, so the trim
            // belongs here, per activation (set_sqout does it). This mirrors
            // the is_after_sqout arm of create_deactivated_path, which
            // already takes those notes back out of the score. The chord is
            // the one the window was offered (Act::sqout_phrase), not always
            // the window's first.
            if (took_sqout && oa.deact_edge >= 0 && oa.sqout_phrase != NO_TIME) {
                act.set_sqout(oa.sqout_phrase);
                // The record shows the row's offset; the search ranked by
                // its own. They must be the same number, or the record
                // would show a squeeze the search didn't rank.
                const double stored_ms = act.sqout_row()->offset();
                if (stored_ms != searched_sqout_ms)
                    throw std::logic_error(
                        "rebuild: the search's SqOut offset " +
                        std::to_string(searched_sqout_ms) + " ms differs from its row's " +
                        std::to_string(stored_ms) + " ms at tick " +
                        std::to_string(oa.sqout_phrase));
            }

            // Every place this window's SP end moved, as the search did it.
            // Its last end is the deactivation node D (deact_tick()): the node
            // every backend row's offset was measured against (graph.cpp
            // add_deact_edge), or, when SP outlasted the chart, the end the
            // search tracked. Nothing here invents a value.
            act.sp_end_steps.assign(out_ends.begin() + oa.end_begin,
                                    out_ends.begin() + oa.end_end);
            // D36: a closed window's record ends on the node SP ended at, tied
            // variants included (close_folded_act). A squeeze-out that kept a
            // later phrase's step once broke this by one bar.
            if (oa.deact_edge >= 0 &&
                act.deact_tick() !=
                    std::optional<int64_t>(en.edges[(size_t)oa.deact_edge]->dest->timecode.ticks()))
                throw std::logic_error("rebuild: a closed window's record ends off its deactivation node");

            // Stamp the frontend transfer scales through the one function
            // that computes them, from the SP-end steps just stamped. When the
            // scales can't be computed, every one stays unknown, never a
            // silent x1.00 (D4). No fresh record reaches this: see 'no fresh
            // record stores an unknown transfer scale'.
            if (auto scales = frontend_transfer_scales(act, timing)) {
                act.transfer_post = scales->post;
                const auto first = act.sqinouts.begin();
                for (auto sq = first; sq != act.sqinouts.end(); ++sq)
                    if (is_sqin_squeeze(*sq))
                        sq->transfer = scales->sqins[sqin_rank(first, sq, is_sqin_squeeze)];
            }
            path.activations.push_back(std::move(act));
        }

        int depth = op.depth;
        int idx = static_cast<int>(pool.size());
        BuildNode bn;
        bn.path = std::move(path);
        pool.push_back(std::move(bn));

        if (depth == 0) {
            top_level.push_back(idx);
            engine_tied.push_back(op.tied_count);
        } else {
            pool[(size_t)idx].path.var_point = op.var_point;
            pool[(size_t)by_depth[(size_t)(depth - 1)]].children.push_back(idx);
        }

        if (static_cast<int>(by_depth.size()) == depth)
            by_depth.push_back(idx);
        else
            by_depth[(size_t)depth] = idx;
    }

    std::function<MPath(int)> assemble = [&](int i) -> MPath {
        MPath p = std::move(pool[(size_t)i].path);
        for (int c : pool[(size_t)i].children)
            p.variants.push_back(assemble(c));
        return p;
    };

    std::vector<MPath> result;
    result.reserve(top_level.size());
    for (size_t k = 0; k < top_level.size(); ++k) {
        MPath p = assemble(top_level[k]);
        // Finding 179: the recount of the finished tree is the count stored
        // and shown; the engine's running sum only enforced the tie limit.
        // They must be the same number, or the limit the engine enforced is
        // not the one the record shows.
        const int recounted = p.recount_tied_paths();
        if (recounted != engine_tied[k])
            throw std::logic_error("rebuild: the search counted " + std::to_string(engine_tied[k]) +
                                   " tied paths but the stored tree holds " +
                                   std::to_string(recounted));
        p.prepare_variants();
        result.push_back(std::move(p));
    }
    return result;
}

}  // namespace

std::vector<MPath> run_search(const ScoreGraph& graph, const EngineOptions& options,
                              const std::function<void(float)>& on_progress) {
    Enum en = enumerate(graph);

    const bool has_cap = graph.sp_meter_cap().has_value();
    const int32_t cap = static_cast<int32_t>(graph.sp_meter_cap().value_or(0));

    Engine engine(en, has_cap, cap, options, graph.rules().backend_leeway_ms,
                  static_cast<int32_t>(graph.rules().max_tied_paths));
    if (on_progress) engine.set_progress_cb(on_progress);

    if (!engine.run())
        throw KindedError(ErrorKind::SearchBroken, "search reached a broken state");

    return rebuild(graph, en, engine.out_paths(), engine.out_acts(), engine.out_sqs(),
                   engine.out_ends(), engine.out_ticks(), graph.tail_backends(),
                   graph.timing(), graph.note_count());
}

}  // namespace hydra
