// ScoreGraph — a song modelled as a two-track graph (base / SP) of timecode
// nodes joined by advance edges (deeper into the song, accruing points) and
// branch edges (toggling SP without advancing time). The engine in
// search/engine.cpp searches over it.

#ifndef HYDRA_SEARCH_GRAPH_H
#define HYDRA_SEARCH_GRAPH_H

#include <cstdint>
#include <deque>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/model.h"
#include "core/rules.h"
#include "core/timing.h"
#include "parse/song.h"

namespace hydra {

// Which game version's rule decides whether a drum fill spawns at all.
//
// A fill only appears in-game if the player's SP meter was already full by
// some deadline before the fill. The two versions disagree on that deadline:
//
//   * Ch11 — Clone Hero 1.1 and later: a flat 4 beats before the fill starts.
//     This is Hydra's normal rule and the default everywhere.
//   * Ch10 — Clone Hero 1.0, as Python Hydra 1.2 modelled it: roughly one
//     fill-length of lead time before the fill, clamped to 250..10000 ms.
//
// Ch10 answers "what could a 1.0 player have reached?": the app's "1.0 fills"
// setting and hydra_batch --legacy-fills. A stored result carries the rule it
// ran under in its key (store::Lens::legacy_fills, docs/adr/0010).
enum class FillDeadlineRule { Ch11, Ch10 };

// The engine_mode string hydra_batch stamps into a database's meta table so
// hydra_fillcompare can tell which rule produced it later (docs/adr/0010).
inline const char* engine_mode_stamp(FillDeadlineRule rule) {
    return rule == FillDeadlineRule::Ch10 ? "ch10" : "ch11";
}

// How a fill rule's name is written: Long in sentences and labels, Short in
// narrow table columns (D48, Q13).
enum class FillRuleNameStyle { Long, Short };

// The one name of each fill rule wherever it is shown: "Clone Hero 1.0" or
// "Clone Hero 1.1", and "CH 1.0" or "CH 1.1" in the short style. The "1.0
// fills" checkbox keeps its own label (docs/adr/0010).
inline const char* fill_rule_name(FillDeadlineRule rule, FillRuleNameStyle style) {
    const bool is_short = style == FillRuleNameStyle::Short;
    if (rule == FillDeadlineRule::Ch10) return is_short ? "CH 1.0" : "Clone Hero 1.0";
    return is_short ? "CH 1.1" : "Clone Hero 1.1";
}

// One sentence saying when each rule's fill deadline falls, in the words the
// fill comparison report's footer uses.
inline std::string fill_rule_description(FillDeadlineRule rule) {
    const std::string name = fill_rule_name(rule, FillRuleNameStyle::Long);
    return rule == FillDeadlineRule::Ch10
               ? name + " gave you until about one fill-length before the fill."
               : name + " made it a flat 4 beats.";
}

// The latest a player's SP may become ready and still have this fill spawn.
// `fill_end_tick` is the fill marker's end; `fill_length_ticks` its length.
double activation_fill_deadline_ms(const SongTiming& timing,
                                   int64_t fill_end_tick,
                                   int64_t fill_length_ticks,
                                   FillDeadlineRule rule);

struct ScoreGraphEdge;

// Where one pending SP end moves when a phrase is collected: +2 measures,
// unless the SP cap's ceiling (2 * cap measures past the collecting note) is
// earlier. `clamped` says the ceiling won -- the end is now pinned to the
// collecting note, not to whatever anchored it before. The deactivation edge
// asks extend_deacts for the same answer when it prices a squeeze-in, so the
// two never disagree (finding 37).
struct SpExtension {
    int64_t to_tick = 0;
    bool clamped = false;
    // The end it moved from holds the collecting phrase in its squeeze
    // window, so the graph keeps that end as a node (ScoreGraph::build) and
    // its deactivation edge lists the phrase. Only there can a squeeze-out
    // give this phrase back (D36): the engine copies the answer into the
    // step it writes (EndNode::sqout_at) and never re-derives it.
    bool sqout_node = false;
};

// One phrase chord an SP end can squeeze in or out, as its deactivation edge
// lists it (ScoreGraphEdge::squeeze_choices).
struct SqueezeChoice {
    Timecode chord;
    // The chord's ms minus the SP end's ms: what the search ranks by and the
    // record shows.
    double timing = 0.0;
    // A late chord only: where a squeeze-in on it leaves the SP end,
    // extend_deacts' answer for this end and this chord (finding 37). An
    // early chord has none: its squeeze-in keeps the end its own step
    // already moved (SpExtension::to_tick, D36).
    std::optional<Timecode> sqin_time;
    // After the SP end (core::after_sp_end). Which chord a path is offered,
    // early or late, is core::offered_phrase's answer (D36): an early chord
    // only to the path whose newest step moved its end from here, a late one
    // only to a path whose end is here.
    bool late = false;
};

struct ScoreGraphNode {
    Timecode timecode;
    ScoreGraphEdge* adv_edge = nullptr;
    ScoreGraphEdge* branch_edge = nullptr;
    bool is_sp = false;
    std::optional<Chord> chord;
};

struct ScoreGraphEdge {
    ScoreGraphNode* dest = nullptr;

    int64_t notecount = 0;
    int64_t basescore = 0;
    int64_t comboscore = 0;
    int64_t spscore = 0;
    int64_t soloscore = 0;
    int64_t accentscore = 0;
    int64_t ghostscore = 0;

    // (sp_timecode, extension_map) per SP phrase collected on this advance edge.
    // The extension map is from_tick -> where that end moves (and whether the
    // cap pinned it there); the engine scans it by key. Only the SP track moves
    // an SP end, so base-track edges leave the map empty and read the times.
    std::vector<std::pair<Timecode, std::map<int64_t, SpExtension>>> sp_times;

    // The frontend chord's SP points: set only on activation edges, 0 elsewhere.
    int frontend_points = 0;
    std::vector<BackendSqueeze> backends;

    // Set only on activation edges; absent (nullopt / empty) otherwise.
    std::optional<double> activation_fill_deadline_ms;
    std::map<int, Timecode> activation_initial_end_times;  // SP meter -> Timecode
    // Activation edges only: which banked phrase a later SP end of this
    // activation could still have in its squeeze window
    // (core::banked_phrase_in_reach), as its 1-based place among the chart's
    // phrase chords; 0 when none. The engine groups running paths by it.
    int banked_phrase_ordinal = 0;

    // Deactivation edges only: every phrase chord in this SP end's squeeze
    // window, in chart order (core::squeeze_window_phrases). The engine
    // offers a path at most one of them (core::offered_phrase, D36).
    // Each late choice carries where a squeeze-in on it moves this SP end
    // (SqueezeChoice::sqin_time): under a cap it depends on the chord.
    std::vector<SqueezeChoice> squeeze_choices;
};

class ScoreGraph {
public:
    // sp_meter_cap: bars the meter holds, or nullopt for no ceiling.
    // rule: which game version decides when a fill spawns (default Ch11).
    // rules: the user's rule choices; the graph prices squeeze-outs by
    // rules.sqout_rule and hands the rest to the engine through rules().
    ScoreGraph(const Song& song, std::optional<int> sp_meter_cap,
               FillDeadlineRule rule = FillDeadlineRule::Ch11,
               const core::Rules& rules = core::default_rules());

    ScoreGraphNode* start() const { return start_; }
    std::optional<int> sp_meter_cap() const { return sp_meter_cap_; }
    const core::Rules& rules() const { return rules_; }
    const SongTiming& timing() const { return song_.timing(); }
    // The chart's multiplier squeezes, in chart order. One list: the combo
    // that decides them never depends on the path.
    const std::vector<MultSqueeze>& multsqueezes() const { return multsqueezes_; }

    // The notes left in the squeeze window (kSqueezeWindowMs) before the song's
    // last timestamp, i.e. the trailing notes no deactivation edge ever got to
    // claim. offset_ms is unset on these: offsets are only stamped on the copies
    // an edge keeps, measured against that edge's own destination. This list
    // is wider than any one activation needs: the engine narrows it to the
    // window around each activation's own SP end (within_squeeze_window)
    // when it copies the rows in.
    const std::vector<BackendSqueeze>& tail_backends() const {
        return recent_backends_;
    }

private:
    // Graph construction.
    void build();

    void store_notecount(int64_t count);
    void store_soloscore(int64_t points);
    void store_basescore(int64_t points);
    void store_comboscore(int64_t points);
    void store_spscore(int64_t points);
    void store_accentscore(int64_t points);
    void store_ghostscore(int64_t points);
    void store_multsqueeze(const MultSqueeze& msq);
    void store_new_backend(const SongTimestamp& ts, int sp_points,
                           int sqout_points);

    int max_sp_bars() const;
    // One moved end per given deact timecode, in order: where it goes and
    // whether the SP cap's ceiling (measured from sp_timecode) is what put
    // it there.
    struct DeactExtension {
        Timecode from;
        Timecode to;
        bool clamped = false;
    };
    std::vector<DeactExtension> extend_deacts(
        const std::vector<Timecode>& deact_tcs, const Timecode& sp_timecode);

    bool is_recent_to_head(const Timecode& tc) const;
    void set_head_time(const Timecode& tc);
    void handle_deact(const Timecode& deact_tc,
                      const std::optional<Chord>& chord);
    void advance_tracks(const Timecode& tc, const std::optional<Chord>& chord);
    ScoreGraphEdge* add_act_edge(int frontend_points, int64_t fill_length_ticks);
    void add_deact_edge();

    Timecode plusmeasure(const Timecode& tc, int64_t add_measures);

    ScoreGraphNode* new_node(const Timecode& tc, bool is_sp);
    ScoreGraphEdge* new_edge();

    const Song& song_;
    std::optional<int> sp_meter_cap_;
    FillDeadlineRule rule_ = FillDeadlineRule::Ch11;
    core::Rules rules_;

    // Stable-address storage for the graph. deque never invalidates element
    // references on push_back.
    std::deque<ScoreGraphNode> node_pool_;
    std::deque<ScoreGraphEdge> edge_pool_;

    ScoreGraphNode* start_ = nullptr;

    // Processing state.
    Timecode head_time_;
    ScoreGraphNode* base_track_head_ = nullptr;
    ScoreGraphNode* sp_track_head_ = nullptr;
    int combo_ = 0;
    std::vector<MultSqueeze> multsqueezes_;
    // The Song's sp_phrase_count(), kept here for max_sp_bars() -- not tallied
    // by this graph itself.
    int sp_phrase_count_ = 0;
    std::unordered_map<int64_t, Timecode> pending_deacts_;  // ticks -> Timecode
    std::vector<Timecode> deact_heap_;                      // min-heap on ticks
    std::vector<ScoreGraphEdge*> recent_deact_edges_;
    std::vector<BackendSqueeze> recent_backends_;
    ScoreGraphEdge* proto_base_edge_ = nullptr;
    ScoreGraphEdge* proto_sp_edge_ = nullptr;

    std::map<std::pair<int64_t, int64_t>, Timecode> plusmeasure_cache_;
};

}  // namespace hydra

#endif  // HYDRA_SEARCH_GRAPH_H
