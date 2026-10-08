#include "app/path_report_view.h"

#include "app/display_format.h"  // format_ms, format_avg_mult
#include "app/html_page.h"       // search_field
#include "core/model.h"          // group_thousands
#include "core/squeeze_rating.h" // timing_tiers, beyond_edge_ms

namespace hydra::app::path_report_view {

using report_view::CellLook;
using report_view::kDash;
using report_view::SortKey;

namespace {

// A timing cell: the app's own text (format_ms), or a dash with none.
std::string ms_cell(const std::optional<double>& ms) {
    return ms ? format_ms(*ms) : std::string(kDash);
}

SortKey ms_key(const std::optional<double>& ms) {
    return ms ? SortKey{*ms} : SortKey{};
}

// The Beyond edge as the chip prints it. A copy of beyond_edge_text in
// report.cpp, which the footer and the page's payload read; that one is
// private to report.cpp, owned by another task this wave, so the merge
// should leave one of the two.
std::string beyond_edge_text(double hit_window_ms) {
    return std::to_string(static_cast<int64_t>(beyond_edge_ms(hit_window_ms)));
}

// A column of text read straight from the row.
Column<ReportRow> text_column(std::string id, std::string title, std::string definition,
                              std::string ReportRow::*field, CellLook look) {
    Column<ReportRow> c;
    c.id = std::move(id);
    c.title = std::move(title);
    c.definition = std::move(definition);
    c.sort_key = [field](const ReportRow& r) { return SortKey{r.*field}; };
    c.cell = [field](const ReportRow& r) { return r.*field; };
    c.look = look;
    return c;
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
    c.definition = std::move(definition);
    c.sort_key = [field](const ReportRow& r) { return SortKey{static_cast<double>(r.*field)}; };
    c.cell = [field, grouped](const ReportRow& r) {
        const int64_t v = static_cast<int64_t>(r.*field);
        return grouped ? group_thousands(v) : std::to_string(v);
    };
    return c;
}

}  // namespace

std::string tier_label(const std::string& tier_name, double hit_window_ms) {
    // The two open entries are timing_tiers' last two, as tier_for reads
    // them: Beyond, then None.
    const std::vector<TimingTier> tiers = timing_tiers(hit_window_ms);
    if (tier_name == tiers[tiers.size() - 2].name)
        return "Beyond " + beyond_edge_text(hit_window_ms) + " ms";
    if (tier_name == tiers.back().name) return "No squeezes";
    return tier_name;
}

std::vector<TimingChoice> timing_choices(double hit_window_ms) {
    std::vector<TimingChoice> out;
    out.push_back({std::nullopt, "All timing tiers"});
    for (const TimingTier& t : timing_tiers(hit_window_ms))
        out.push_back({std::string(t.name), tier_label(t.name, hit_window_ms)});
    return out;
}

std::vector<Column<ReportRow>> path_columns(double hit_window_ms) {
    CellLook trunc;
    trunc.truncate = true;
    CellLook dim_trunc = trunc;
    dim_trunc.dim = true;
    CellLook mono_trunc = trunc;
    mono_trunc.mono = true;

    std::vector<Column<ReportRow>> cols;
    cols.push_back(text_column("song", "Song", "", &ReportRow::song, trunc));
    cols.push_back(text_column("artist", "Artist", "", &ReportRow::artist, dim_trunc));
    cols.push_back(text_column("charter", "Charter", "", &ReportRow::charter, dim_trunc));
    cols.push_back(text_column("mode", "Mode",
                               "The difficulty and drum options the path was found for.",
                               &ReportRow::mode, dim_trunc));
    cols.push_back(text_column("path", "Path",
                               "The path in path notation: one entry per activation, with its "
                               "skip count and squeeze symbols.",
                               &ReportRow::path, mono_trunc));
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
        c.definition =
            "The hardest squeeze or required early fill the path needs, in raw ms. A dash "
            "means it needs none.";
        c.sort_key = [](const ReportRow& r) { return ms_key(r.ms); };
        c.cell = [](const ReportRow& r) { return ms_cell(r.ms); };
        cols.push_back(std::move(c));
    }
    {
        // The chip names the tier tier_for gave the row; the column sorts on
        // that name.
        Column<ReportRow> c;
        c.id = "tier";
        c.title = "Timing";
        c.definition =
            "How hard Hardest ms is, in bands of your hit window. Beyond means more than "
            "twice the hit window.";
        c.sort_key = [](const ReportRow& r) { return SortKey{r.tier}; };
        c.cell = [hit_window_ms](const ReportRow& r) { return tier_label(r.tier, hit_window_ms); };
        c.look.chip = true;
        cols.push_back(std::move(c));
    }
    {
        Column<ReportRow> c;
        c.id = "efill";
        c.title = "Early fill (ms)";
        c.numeric = true;
        c.definition =
            "The hardest early fill (E0) on the path: how many ms early you must hit to summon "
            "the fill. Negative means slack. A dash means the path has none.";
        c.sort_key = [](const ReportRow& r) { return ms_key(r.efill); };
        c.cell = [](const ReportRow& r) { return ms_cell(r.efill); };
        cols.push_back(std::move(c));
    }
    {
        Column<ReportRow> c;
        c.id = "mult";
        c.title = "Avg multiplier";
        c.numeric = true;
        c.definition =
            "Average multiplier: the score without solo bonuses divided by the base score "
            "(every note at 1x).";
        c.sort_key = [](const ReportRow& r) { return SortKey{r.mult}; };
        c.cell = [](const ReportRow& r) { return format_avg_mult(r.mult); };
        cols.push_back(std::move(c));
    }
    cols.push_back(count_column("sqin", "SqIn",
                                "SP phrase notes squeezed into an active Star Power window (+ "
                                "in the path).",
                                &ReportRow::sqin, false));
    cols.push_back(count_column("sqout", "SqOut",
                                "SP phrase notes squeezed out of an active Star Power window (- "
                                "in the path).",
                                &ReportRow::sqout, false));
    cols.push_back(count_column("notes", "Notes", "Notes in the chart.", &ReportRow::notes, true));
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
    return html::search_field(row.song, row.artist, row.charter, row.path);
}

}  // namespace hydra::app::path_report_view
