#include "app/path_view.h"
#include "app/config.h"  // Settings::search_depth_mode, bars_text, score_range_text
#include "app/display_format.h"
#include "app/preview_view.h"  // song_fraction, has_song_length

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <optional>
#include <stdexcept>

#include "core/backend_value.h"
#include "core/timing.h"  // kSpActivationBars

namespace hydra::app {

namespace {

// A multiplier as the scale line prints it: two decimals, or as many more as
// it takes not to read as x1.00, up to kScaleIdentityDigits (derived from
// is_scaled's tolerance), so a multiplier that governs a figure never prints
// as 1.
std::string format_scale(double r) {
    char buf[32];
    for (int digits = 2; digits <= kScaleIdentityDigits; ++digits) {
        std::snprintf(buf, sizeof(buf), "x%.*f", digits, r);
        if (std::strtod(buf + 1, nullptr) != 1.0) break;
    }
    return buf;
}

// A note's effective timing as both the SqIn sentence and the backend row
// add it: " (eff. 163.5 ms)".
std::string eff_suffix(double effective_ms) {
    return " (eff. " + format_ms(effective_ms) + ")";
}

}  // namespace

// A default Timecode is the chart's first tick (core/timing.h), so this
// needs no timing.
std::string first_measure_label() { return format_measure(Timecode{}); }

namespace {

// The badge's word for each part Activation::badge_timings() can list.
struct BadgeWord {
    TimingPart part;
    const char* word;
};
constexpr BadgeWord kBadgeWords[] = {
    {TimingPart::SqueezeIn, "squeeze in"},
    {TimingPart::SqueezeOut, "squeeze out"},
    {TimingPart::EarlyFill, "early fill"},
};

std::string badge_word(TimingPart part) {
    for (const BadgeWord& w : kBadgeWords)
        if (w.part == part) return w.word;
    throw std::logic_error("activation_badges: no word for this TimingPart");
}

}  // namespace

std::vector<ActivationBadge> activation_badges(const Activation& act) {
    // The activation says which timings it has, in which order, and which
    // are difficult; the badges only word them.
    std::vector<ActivationBadge> out;
    for (const ActivationTiming& t : act.badge_timings())
        out.push_back({badge_word(t.part) + " " + format_ms_whole(t.ms), t.difficult});
    return out;
}

std::string longest_activation_badge() {
    // Why the squeeze window bounds every figure a badge shows: every squeeze
    // offset is one the engine kept inside within_squeeze_window, so a
    // squeeze's figure lies strictly between -kSqueezeWindowMs and
    // +kSqueezeWindowMs. An early fill is named only on an E-critical
    // activation, so its figure is above minus kEarlyFillSlackMs (is_e0),
    // and at most kHitWindowMs (fill_refuses): inside the squeeze window
    // too. A squeeze that needs no timing (SPSqueeze::needs_no_timing) has a
    // figure of 0 or less (D80), so the widest figure is the window's, minus
    // sign included.
    size_t longest = 0;
    for (size_t i = 1; i < std::size(kBadgeWords); ++i)
        if (std::strlen(kBadgeWords[i].word) > std::strlen(kBadgeWords[longest].word))
            longest = i;
    return std::string(kBadgeWords[longest].word) + " " + format_ms_whole(-kSqueezeWindowMs);
}

std::vector<TextLine> squeeze_sentences(const Activation& act,
                                        const BackendRating* squeezed_out,
                                        double leeway_ms,
                                        const std::vector<std::optional<double>>& note_effective_ms) {
    std::vector<TextLine> out;
    for (size_t i = 0; i < act.sqinouts.size(); ++i) {
        const SPSqueeze& sq = act.sqinouts[i];
        // timing() is the edge the sentence below prints: a SqOut must be hit
        // later than it, a SqIn earlier than it. Which wording applies
        // is SPSqueeze::needs_no_timing's answer, the one the rating reads
        // too (D13).
        const double t = sq.timing();
        const std::string edge = format_ms(std::fabs(t));
        std::string text;
        if (sq.kind == SqueezeKind::SqOut) {
            const std::string note =
                squeezed_out ? "the " + squeezed_out->row.chord.notationstr() + " note"
                             : std::string("the SP phrase's last note");
            const std::string when = sq.needs_no_timing() ? "no more than " + edge + " early"
                                                          : "more than " + edge + " late";
            text = "Hit " + note + " " + when + " so it lands after Star Power ends.";
            if (squeezed_out) {
                // What the squeeze-out costs: core::sqout_cost, which the
                // table's "(-N)" reads too. A counted row's cost is never 0
                // (test_stars.cpp, "squeezing out any chord always costs
                // points").
                const BackendSqueeze& row = squeezed_out->row;
                const double off = row.offset();
                if (core::counted_without_squeeze(off, leeway_ms)) {
                    const int lost = core::sqout_cost(off, row.sp_points, row.sqout_sp_points, leeway_ms);
                    text += " It scores " + std::to_string(lost) +
                            " fewer points, and its SP phrase banks for later.";
                } else {
                    text += " It costs no points, because Hydra's score never counted that "
                            "note under Star Power, and its SP phrase banks for later.";
                }
            } else {
                text += " Its SP phrase banks for later.";
            }
        } else {
            std::string when = sq.needs_no_timing() ? "no more than " + edge + " late"
                                                    : "more than " + edge + " early";
            // A SqIn has no backend row to carry its eff. figure (a SqOut's
            // sits on its squeezed-out row), so the sentence carries it.
            if (i < note_effective_ms.size() && note_effective_ms[i])
                when += eff_suffix(*note_effective_ms[i]);
            text = "Hit the SP phrase's last note " + when +
                   " so it lands before Star Power ends. The phrase then counts while Star "
                   "Power runs, which makes Star Power last longer.";
        }
        out.push_back({std::move(text), sq.is_difficult()});
    }
    return out;
}

namespace {

// The squeeze rows, naming each note in the words of the Pro Drums setting
// `pro_drums` (note_label, through Chord::rowstr and MultSqueeze::howto).
// The Paths tab's cache passes the record's setting.
std::vector<MultSqueezeView> multsqueeze_views(const HydraRecord& record, bool pro_drums) {
    std::vector<MultSqueezeView> out;
    out.reserve(record.multsqueezes.size());
    for (const MultSqueeze& msq : record.multsqueezes) {
        MultSqueezeView v;
        v.label = msq.notationstr() + "   (+" + std::to_string(msq.gain()) +
                  " pts):   " + msq.chord().rowstr(pro_drums);
        v.howto = msq.howto(pro_drums);
        v.gain = msq.gain();
        out.push_back(std::move(v));
    }
    return out;
}

}  // namespace

// In the Pro Drums-on words; the Paths tab goes through PathsTabCache::details,
// which passes the record's own setting.
std::vector<MultSqueezeView> build_multsqueezes(const HydraRecord& record) {
    return multsqueeze_views(record, /*pro_drums=*/true);
}

std::string multsqueeze_summary(const std::vector<MultSqueezeView>& squeezes) {
    if (squeezes.empty()) return "none";
    int total = 0;
    for (const MultSqueezeView& s : squeezes) total += s.gain;
    const std::string sum = "+" + group_thousands(total);
    return squeezes.size() == 1 ? sum : std::to_string(squeezes.size()) + kFactSeparator + sum;
}

const char* const kTransferScaleHint =
    "Star Power length is counted in measures, not milliseconds. Where measures at the SP end "
    "last a different time than at the activation, only part of your activation timing carries "
    "to the SP end. Hitting the activation 50 ms late might move the SP end only 25 ms. Early "
    "and late hits can scale differently when the activation or the SP end sits right on a "
    "tempo or time signature change.";

const char* const kOverfillHint =
    "A phrase collected during Star Power would have pushed the meter past the SP cap, so the "
    "cap cut the SP end short. The note that filled the meter now moves the SP end, instead of "
    "the activation. The ms figures are still measured at the SP end. Only the note whose "
    "timing moves them changes.";

// `record` fed the old footer's "SP meter" line, gone with the old Paths tab;
// the parameter stays so the callers keep their shape.
ActivationsView build_activations(const Path& path, const HydraRecord& /*record*/,
                                  const SongTiming* timing,
                                  std::optional<double> backend_limit_ms,
                                  const core::Rules& rules,
                                  std::optional<double> song_length_ms,
                                  bool pro_drums) {
    ActivationsView view;

    const ActivationWalk acts = path.walk_activations();
    for (const Activation& act : acts) {
        ActivationRowView av;

        std::string ntn = act.notationstr();
        std::string meas = format_measure(act.timecode);
        av.difficult = act.is_difficult();
        av.number = static_cast<int>(view.acts.size()) + 1;
        av.notation = ntn;
        av.measure = meas;
        av.sp_bars = act.sp_meter();
        av.bars = bars_text(act.sp_meter());
        av.badges = activation_badges(act);
        av.chord = act.chord.rowstr(pro_drums);
        if (timing && song_length_ms)
            av.song_fraction =
                song_fraction(timing->timecode(act.timecode.ticks()).ms(), *song_length_ms);

        if (act.is_e_critical()) {
            // Positive = hit early, the same sign as the report:
            // e_difficulty(true) is -e_offset for every E-critical activation.
            av.early_fill = "Early fill: " +
                             format_ms(*act.e_difficulty(/*verbose=*/true)) +
                             (act.is_E0() ? " (required)" : " (optional)");
        }

        ActivationRating rate = rate_activation(act, rules.backend_leeway_ms);

        // The line names the sides the activation's rows and squeezes read
        // (rate_activation records them), when they aren't 1, early first:
        // at the SP end, then at the SqIn's end when that prints differently.
        // It is orange when one of them governs a row or SqIn on this
        // activation.
        auto sides = [](const TransferScale& s, const SidesRead& read) {
            std::string out;
            if (read.early && is_scaled(s.early)) out = format_scale(s.early) + " (early)";
            if (read.late && is_scaled(s.late)) {
                if (!out.empty()) out += " / ";
                out += format_scale(s.late) + " (late)";
            }
            return out;
        };
        if (!rate.scales) {
            // D4: a guard that shows a bug. A test proves fresh records never
            // reach it.
            av.scale_warning = "Transfer scale unknown.";
            av.scale_warn = true;
        } else {
            const std::string post_part = sides(rate.scales->post, rate.post_read);
            // Each SqIn's scale, when it prints differently from the SP
            // end's. When every SqIn prints alike, one clause covers them:
            // "the SqIn's SP end" for a single SqIn, "each SqIn's SP end" for
            // two or more (D16). When they differ, each gets its own clause,
            // numbered by its place among the SqIns (Q5).
            std::vector<std::string> sqin_parts;
            bool sqins_alike = true;
            // rate_activation pairs sqins_read with scales->sqins, entry for
            // entry; the two lists are walked together.
            auto read = rate.sqins_read.begin();
            for (const TransferScale& s : rate.scales->sqins) {
                sqin_parts.push_back(sides(s, *read++));
                sqins_alike = sqins_alike && sqin_parts.back() == sqin_parts.front();
            }
            std::string clauses;
            if (!post_part.empty()) clauses = post_part + " at the SP end";
            for (size_t i = 0; i < sqin_parts.size(); ++i) {
                const std::string& part = sqin_parts[i];
                if (part.empty() || part == post_part) continue;
                if (!clauses.empty()) clauses += "; ";
                if (sqins_alike) {
                    clauses += part + (sqin_parts.size() > 1 ? " at each SqIn's SP end"
                                                             : " at the SqIn's SP end");
                    break;
                }
                clauses += part + " at SqIn " + std::to_string(i + 1) + "'s SP end";
            }
            if (!clauses.empty()) av.scale_warning = "Frontend timing scales " + clauses + ".";
            av.scale_warn = rate.scale_governs;
        }

        // When the SP window was cap-clamped and this activation lists a
        // squeeze the frontend decides, warn that the anchor is the
        // collecting note, not the activation.
        if (rate.cap_clamped) {
            const std::optional<int64_t> clamp = act.clamp_tick();
            if (timing && clamp) {
                av.overfill_warning =
                    "SP overfilled at " + format_measure(*timing, *clamp);
            } else {
                av.overfill_warning = "SP overfilled";
            }
        }

        const BackendRating* squeezed_out = nullptr;
        for (const BackendRating& br : rate.backends)
            if (br.squeezed_out) squeezed_out = &br;
        av.squeeze_sentences = squeeze_sentences(act, squeezed_out, rules.backend_leeway_ms,
                                                 rate.note_effective_ms);

        av.backends.reserve(rate.backends.size());
        // The rows the backend limit hides, by side of the SP end. A hidden
        // row is never on the end itself, so it is one side or the other.
        int64_t hidden_before = 0;
        int64_t hidden_after = 0;
        for (const BackendRating& br : rate.backends) {
            const BackendSqueeze& bsq = br.row;

            // The display window the user asked for. It only drops rows from
            // the table: the ratings above (and the warns feeding the scale
            // line) still read every stored row. A squeezed-out note is the
            // reason the row matters, so it always shows.
            if (backend_limit_ms && !br.squeezed_out &&
                std::fabs(bsq.offset()) > *backend_limit_ms) {
                ++(bsq.offset() < 0 ? hidden_before : hidden_after);
                continue;
            }

            BackendRowView row;
            char tbuf[32];
            std::snprintf(tbuf, sizeof(tbuf), "%.1f", bsq.offset());
            row.timing = tbuf;
            if (br.note.effective_ms) {
                // The budget at identity scale (x1.00): what the combined
                // budget would be with no frontend-timing scale.
                // Written once and said twice, so both read the same
                // (D57 item 2: "on the normal 170.5 ms scale ... not 170.5 ms").
                const std::string normal_budget = format_ms(nominal_budget_ms());
                row.tooltip = "Effectively " + format_ms(*br.note.effective_ms) +
                              " on the normal " + normal_budget +
                              " scale:\nfrontend timing scales " +
                              format_scale(br.note.scale) +
                              " here, so the combined\nsqueeze budget is " +
                              format_ms(br.note.budget_ms) + ", not " + normal_budget + ".";
            }
            row.chord = bsq.chord.notationstr();
            // What the engine actually paid for this row on this path, from
            // the same function the search calls. display_backends already
            // dropped every row past the squeezed-out chord, so a row here is
            // either that chord or before it.
            const double off = bsq.offset();
            const bool counted =
                core::counted_without_squeeze(off, rules.backend_leeway_ms);
            const int value = core::backend_row_value(
                off, bsq.sp_points, bsq.sqout_sp_points,
                core::sqout_position(bsq.timecode.ticks(), act.sqout_tick),
                rules.backend_leeway_ms);
            row.points = std::to_string(value);
            row.rating = bsq.summarystr(br.squeezed_out, rules.backend_leeway_ms);
            if (br.note.effective_ms)
                row.rating += eff_suffix(*br.note.effective_ms);
            if (br.squeezed_out) {
                // "(-N)" and the warning colour only when the squeeze-out
                // really costs points. A row the engine never counted costs
                // nothing either way (user decisions 1 and 19); its label
                // already carries summarystr's "(uncounted)" tag (D48, Q6).
                char extra[48];
                if (counted) {
                    std::snprintf(extra, sizeof(extra),
                                  " <-- squeezed out (-%d)",
                                  core::sqout_cost(off, bsq.sp_points, bsq.sqout_sp_points,
                                                   rules.backend_leeway_ms));
                    row.warn = true;
                } else {
                    std::snprintf(extra, sizeof(extra), " <-- squeezed out");
                }
                row.rating += extra;
            }
            av.backends.push_back(std::move(row));
        }
        // The heading counts every stored row, shown or hidden. When the
        // limit hides some, the heading adds how many on each side, and a
        // line in the fold says the same with the limit. Both sides are
        // always named, even when one is 0.
        const int64_t shown = static_cast<int64_t>(av.backends.size());
        const int64_t hidden = hidden_before + hidden_after;
        const int64_t total = shown + hidden;
        av.backends_label = counted(total, "note", "notes") + " near the SP end";
        if (hidden > 0) {
            const std::string n_hidden = std::to_string(hidden) + " hidden: ";
            av.backends_label += kFactSeparator + n_hidden + std::to_string(hidden_before) +
                                 " before, " + std::to_string(hidden_after) + " after";
            // "All 1 hidden" reads badly, so a single row never says "All".
            const std::string all = shown == 0 && total > 1 ? "All " : "";
            av.backends_hidden_line = all + n_hidden + std::to_string(hidden_before) +
                                      " before the SP end and " +
                                      std::to_string(hidden_after) + " after, beyond " +
                                      format_ms_whole(*backend_limit_ms) + ".";
        }

        view.acts.push_back(std::move(av));
    }

    // The line beside the heading: how many, and what is left.
    if (!view.acts.empty()) {
        const std::string left = path.leftover_sp() == 0
                                     ? std::string("no SP left over")
                                     : bars_text(path.leftover_sp()) + " of SP left over";
        view.summary = std::to_string(view.acts.size()) + kFactSeparator + left;
    }
    if (timing && song_length_ms && has_song_length(*song_length_ms)) {
        const int64_t end_tick = timing->display_tick_at_ms(*song_length_ms);
        view.timeline_start = measure_label(timing->timecode(0));
        view.timeline_end = measure_label(timing->timecode(end_tick));
    }

    return view;
}

std::vector<std::string> build_score_breakdown(const Path& path) {
    // The labels are written once, here. Every number starts in one column,
    // two spaces after the longest label, and is not padded (answer 11 of
    // docs/superpowers/plans/2026-10-14-column-shares-scroll-left-align.md),
    // so a renamed label moves the column with it.
    struct Row {
        std::string label;
        std::string value;
        bool blank_above;  // a leading '\n'; there is no other separator
    };
    const Row rows[] = {
        // Rounded, not truncated, so it matches the report's mult column.
        {std::string(kAverageMultiplierLabel) + ":", format_avg_mult(path.avg_mult()) + "x", false},
        {"Notes:", group_thousands(path.notes_points), true},
        {"Combo Bonus:", group_thousands(path.score_combo), false},
        {"Star Power:", group_thousands(path.score_sp), false},
        {"Solo Bonus:", group_thousands(path.score_solo), false},
        {"Accent Notes:", group_thousands(path.score_accents), false},
        {"Ghost Notes:", group_thousands(path.score_ghosts), false},
        {"Total Score:", group_thousands(path.totalscore()), true},
    };
    size_t longest = 0;
    for (const Row& r : rows) longest = std::max(longest, r.label.size());

    std::vector<std::string> lines;
    for (const Row& r : rows) {
        std::string line = r.blank_above ? "\n" : "";
        line += r.label;
        line.append(longest - r.label.size() + 2, ' ');
        line += r.value;
        lines.push_back(std::move(line));
    }
    return lines;
}

PathListView build_path_list(const HydraRecord& record) {
    PathListView view;
    std::vector<const Path*> flat = record.all_paths();

    // Every unique score along the traversal gets its own group, labelled
    // with the score in thousands-separated digits.
    int64_t current_score = INT64_MIN;
    for (const Path* p : flat) {
        if (p->totalscore() != current_score) {
            current_score = p->totalscore();
            view.groups.push_back({group_thousands(current_score), {}});
        }
        view.groups.back().paths.push_back(p);
    }

    // An all-0 path is only worth showing when the generated list does not
    // already contain it. "The same path" is path_identity, the one rule the
    // Preview's overlay key reads too. A duplicate hides only itself (D51
    // call 4b); the section shows when any all-0 path survives.
    for (const Path* z : record.all_allzero_paths()) {
        const std::string identity = path_identity(*z);
        const bool listed = std::any_of(flat.begin(), flat.end(), [&](const Path* p) {
            return path_identity(*p) == identity;
        });
        if (!listed) view.allzero.push_back(z);
    }
    if (view.allzero.empty()) return view;
    view.show_allzero = true;
    // The score alone: what the all-0 path costs against the optimal one is
    // its button's "N below optimal" line (build_path_buttons).
    view.allzero_label = group_thousands(view.allzero.front()->totalscore());
    return view;
}

namespace {

// The heading over the non-optimal paths, worded by score_range_text.
std::string within_label(DepthMode depth_mode, int depth_value) {
    return "Within " + score_range_text(depth_mode, depth_value);
}

}  // namespace

PathButtonsView build_path_buttons(const HydraRecord& record, int depth_mode, int depth_value) {
    PathButtonsView view;
    // The callers hold the INI's int; Settings::search_depth_mode (SE1) owns
    // what it means.
    Settings settings;
    settings.depth_mode = depth_mode;
    view.within_label = within_label(settings.search_depth_mode(), depth_value);
    const PathListView list = build_path_list(record);
    const std::optional<int64_t> optimal = record.optimal_score();
    // A cap below kSpActivationBars can never activate (D51 call 16).
    const bool cap_never_activates = record.sp_cap && *record.sp_cap < kSpActivationBars;

    // `fold` is the score group the button sits in; null for the all-0 path.
    auto add = [&](const Path* p, PathButtonView::Group group, const PathGroupView* fold) {
        PathButtonView b;
        b.path = p;
        b.group = group;
        b.notation = p->pathstring();
        if (fold) {
            b.fold = fold->score_label;
            b.fold_count =
                counted(static_cast<int64_t>(fold->paths.size()), "path", "paths");
            b.title = b.notation;
        } else {
            b.title = group_thousands(p->totalscore()) + kFactSeparator + b.notation;
        }
        if (std::optional<double> hardest = p->difficulty()) {
            b.timing = format_ms(*hardest);
            b.timing_warn = p->is_difficult();
        }
        if (group == PathButtonView::Group::AllZero && optimal) {
            // What the all-0 path costs against the optimal one. It never
            // scores above it (test_engine_sweeps pins that on the corpus).
            const int64_t shortfall = *optimal - p->totalscore();
            if (shortfall > 0) b.detail = group_thousands(shortfall) + " below optimal";
        }
        if (cap_never_activates) b.detail = "A 1-bar cap can never activate Star Power.";
        view.buttons.push_back(std::move(b));
    };
    for (const PathGroupView& g : list.groups)
        for (const Path* p : g.paths)
            add(p,
                record.is_optimal(*p) ? PathButtonView::Group::Optimal
                                      : PathButtonView::Group::Within,
                &g);
    if (list.show_allzero)
        for (const Path* p : list.allzero) add(p, PathButtonView::Group::AllZero, nullptr);
    return view;
}

void PathsTabUi::reset(size_t rows) {
    act_open.assign(rows, 0);
    backends_open.assign(rows, 1);
}

bool PathsTabUi::all_open() const {
    if (act_open.empty()) return false;
    for (char open : act_open)
        if (!open) return false;
    return true;
}

void PathsTabUi::set_all(bool open) {
    std::fill(act_open.begin(), act_open.end(), static_cast<char>(open ? 1 : 0));
}

void PathsTabUi::click_row(size_t i) {
    if (i >= act_open.size()) return;
    if (act_open[i]) {
        act_open[i] = 0;
        return;
    }
    std::fill(act_open.begin(), act_open.end(), static_cast<char>(0));
    act_open[i] = 1;
}


const PathsTabCache::Details& PathsTabCache::details(
    const Path& path, const HydraRecord& record, int record_generation,
    const SongTiming* timing, std::optional<double> backend_limit_ms,
    const core::Rules& rules, std::optional<double> song_length_ms, bool pro_drums) {
    const bool new_path = record_generation != details_generation_ || &path != details_path_;
    if (new_path || backend_limit_ms != details_backend_limit_ms_ ||
        song_length_ms != details_song_length_ms_ || pro_drums != details_pro_drums_) {
        details_.squeezes = multsqueeze_views(record, pro_drums);
        details_.activations = build_activations(path, record, timing, backend_limit_ms, rules,
                                                 song_length_ms, pro_drums);
        details_.breakdown = build_score_breakdown(path);
        details_generation_ = record_generation;
        details_path_ = &path;
        details_backend_limit_ms_ = backend_limit_ms;
        details_song_length_ms_ = song_length_ms;
        details_pro_drums_ = pro_drums;
        // What is unfolded belongs to the path; a display setting keeps it.
        if (new_path) ui_.reset(details_.activations.acts.size());
        ++details_builds_;
    }
    return details_;
}

const PathButtonsView& PathsTabCache::buttons(const HydraRecord& record, int record_generation,
                                              int depth_mode, int depth_value) {
    if (record_generation != buttons_generation_ || depth_mode != buttons_depth_mode_ ||
        depth_value != buttons_depth_value_) {
        buttons_ = build_path_buttons(record, depth_mode, depth_value);
        buttons_generation_ = record_generation;
        buttons_depth_mode_ = depth_mode;
        buttons_depth_value_ = depth_value;
        ++buttons_builds_;
    }
    return buttons_;
}

}  // namespace hydra::app
