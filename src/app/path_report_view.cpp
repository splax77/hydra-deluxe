#include "app/path_report_view.h"

#include "app/display_format.h"  // format_ms, format_avg_mult, kAverageMultiplierLabel
#include "app/library_query.h"   // search_field
#include "core/model.h"          // group_thousands
#include "core/squeeze_rating.h" // timing_tiers, beyond_tier, none_tier

namespace hydra::app::path_report_view {

using report_view::CellKind;
using report_view::CellLook;
using report::kDash;
using report_view::optional_key;
using report_view::SortKey;
using report_view::text_column;

namespace {

// A timing cell: the app's own text (format_ms), or a dash with none.
std::string ms_cell(const std::optional<double>& ms) {
    return ms ? format_ms(*ms) : std::string(kDash);
}

// A column of whole numbers read straight from the row, grouped in
// thousands where the page grouped them.
template <class T>
Column<ReportRow> count_column(std::string id, std::string title, std::string definition,
                               T ReportRow::*field, bool grouped) {
    Column<ReportRow> c;
    c.id = std::move(id);
    c.title = std::move(title);
    c.numeric = true;
    c.look.kind = CellKind::Figure;
    c.definition = std::move(definition);
    c.sort_key = [field](const ReportRow& r) { return SortKey{static_cast<double>(r.*field)}; };
    c.cell = [field, grouped](const ReportRow& r) {
        const int64_t v = static_cast<int64_t>(r.*field);
        return grouped ? group_thousands(v) : std::to_string(v);
    };
    return c;
}

}  // namespace

std::string tier_label(const std::string& tier_name) {
    const std::vector<TimingTier> tiers = timing_tiers();
    if (tier_name == beyond_tier(tiers).name)
        return "Beyond " + report::beyond_edge_text() + " ms";
    if (tier_name == none_tier(tiers).name) return "No timing needed";
    return tier_name;
}

std::vector<TimingChoice> timing_choices() {
    std::vector<TimingChoice> out;
    out.push_back({std::nullopt, "All timing tiers"});
    for (const TimingTier& t : timing_tiers())
        out.push_back({std::string(t.name), tier_label(t.name)});
    return out;
}

std::vector<Column<ReportRow>> path_columns() {
    CellLook trunc;
    trunc.truncate = true;
    CellLook dim_trunc = trunc;
    dim_trunc.dim = true;
    CellLook notation_trunc = trunc;
    notation_trunc.kind = CellKind::Notation;

    std::vector<Column<ReportRow>> cols;
    cols.push_back(text_column("song", "Song", "", &ReportRow::song, with_share(trunc, 31)));
    cols.push_back(text_column("artist", "Artist", "", &ReportRow::artist, with_share(dim_trunc, 17)));
    cols.push_back(text_column("charter", "Charter", "", &ReportRow::charter, with_share(dim_trunc, 16)));
    cols.push_back(text_column("path", "Path",
                               "The path in path notation: one entry per activation, with its "
                               "skip count and squeeze symbols.",
                               &ReportRow::path, with_share(notation_trunc, 36)));
    cols.push_back(count_column("score", "Score", "The total score the path reaches.",
                                &ReportRow::score, true));
    cols.push_back(count_column("acts", "Acts",
                                "Activations: how many times the path uses Star Power.",
                                &ReportRow::acts, false));
    cols.push_back(count_column("skip", "Max skip",
                                "The most fills any one activation passes over before "
                                "activating.",
                                &ReportRow::skip, false));
    {
        Column<ReportRow> c;
        c.id = "ms";
        c.title = "Hardest ms";
        c.numeric = true;
        c.look.kind = CellKind::Figure;
        c.definition =
            "The hardest squeeze or required early fill the path needs, in ms: the gap at the "
            "SP end, or at the fill deadline. It leaves out the frontend timing scale. A dash "
            "means it needs none.";
        c.sort_key = [](const ReportRow& r) { return optional_key(r.ms); };
        c.cell = [](const ReportRow& r) { return ms_cell(r.ms); };
        cols.push_back(std::move(c));
    }
    {
        // The chip names the tier tier_for gave the row. The column sorts on
        // that tier's place in timing_tiers(), the ladder the Timing dropdown
        // lists, not on its name (D284 item 1).
        Column<ReportRow> c;
        c.id = "tier";
        c.title = "Timing";
        c.definition = "How hard Hardest ms is, in bands of the " + report::hit_window_text() +
                       " ms hit window. Beyond means more than twice the hit window.";
        c.sort_key = [tiers = timing_tiers()](const ReportRow& r) {
            for (size_t i = 0; i < tiers.size(); ++i)
                if (tiers[i].name == r.tier) return SortKey{static_cast<double>(i)};
            return SortKey{};
        };
        c.cell = [](const ReportRow& r) { return tier_label(r.tier); };
        c.look.chip = true;
        cols.push_back(std::move(c));
    }
    {
        Column<ReportRow> c;
        c.id = "efill";
        c.title = "Early fill (ms)";
        c.numeric = true;
        c.look.kind = CellKind::Figure;
        c.definition =
            "The hardest early fill on the path (written E0 in the path): how many ms early you "
            "must hit to summon the fill. Negative means slack. A dash means the path has none.";
        c.sort_key = [](const ReportRow& r) { return optional_key(r.efill); };
        c.cell = [](const ReportRow& r) { return ms_cell(r.efill); };
        cols.push_back(std::move(c));
    }
    {
        Column<ReportRow> c;
        c.id = "mult";
        c.title = kAverageMultiplierLabel;
        c.numeric = true;
        c.look.kind = CellKind::Figure;
        c.definition =
            "Average multiplier: the score without solo bonuses divided by the base score "
            "(every note at 1x).";
        c.sort_key = [](const ReportRow& r) { return SortKey{r.mult}; };
        c.cell = [](const ReportRow& r) { return format_avg_mult(r.mult); };
        cols.push_back(std::move(c));
    }
    cols.push_back(count_column("sqin", "SqIn",
                                "SP phrase notes squeezed into an active SP window (+ in the "
                                "path).",
                                &ReportRow::sqin, false));
    cols.push_back(count_column("sqout", "SqOut",
                                "SP phrase notes squeezed out of an active SP window (- in the "
                                "path).",
                                &ReportRow::sqout, false));
    cols.push_back(count_column("notes", "Note count", "Notes in the chart.", &ReportRow::notes, true));
    return cols;
}

SortSpec path_first_sort() { return {"score", report_view::SortDir::Descending}; }

std::function<bool(const ReportRow&)> path_keep(std::optional<std::string> tier, bool best_only) {
    // The row's tier is the label tier_for gave it; nothing here bands a
    // timing again.
    return [tier = std::move(tier), best_only](const ReportRow& r) {
        if (best_only && !r.optimal) return false;
        if (tier && r.tier != *tier) return false;
        return true;
    };
}

std::string path_search_text(const ReportRow& row) {
    return search_field(row.song, row.artist, row.charter, row.path);
}

}  // namespace hydra::app::path_report_view
