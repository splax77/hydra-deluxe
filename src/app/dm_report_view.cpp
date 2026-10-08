#include "app/dm_report_view.h"

#include "app/display_format.h"  // format_percent
#include "app/html_page.h"       // search_field, replace_all
#include "core/model.h"          // group_thousands, kCloneHeroSpCap

namespace hydra::app::dm_report_view {

using report_view::CellLook;
using report_view::kDash;
using report_view::SortKey;
using report_view::Tone;

namespace {

// The definitions name the base speed through __BASE_SPEED__ and Clone
// Hero's cap through __SP_CAP__, filled from net::kBaseSpeedPercent and
// kCloneHeroSpCap as the page was.
std::string fill_constants(std::string definition) {
    return html::replace_all(
        html::replace_all(std::move(definition), "__BASE_SPEED__",
                          std::to_string(net::kBaseSpeedPercent)),
        "__SP_CAP__", std::to_string(kCloneHeroSpCap));
}

Column<DmReportRow> column(std::string id, std::string title, bool numeric,
                           std::string definition) {
    Column<DmReportRow> c;
    c.id = std::move(id);
    c.title = std::move(title);
    c.numeric = numeric;
    c.definition = fill_constants(std::move(definition));
    return c;
}

Column<DmReportRow> text_column(std::string id, std::string title,
                                std::string DmReportRow::*field, CellLook look) {
    Column<DmReportRow> c = column(std::move(id), std::move(title), false, "");
    c.sort_key = [field](const DmReportRow& r) { return SortKey{r.*field}; };
    c.cell = [field](const DmReportRow& r) { return r.*field; };
    c.look = look;
    return c;
}

template <class T>
SortKey optional_key(const std::optional<T>& v) {
    return v ? SortKey{static_cast<double>(*v)} : SortKey{};
}

// The row's percent of optimal, when the page had one: the payload carried
// pct_h only beside an optimal score.
bool has_percent(const DmReportRow& r) { return r.pct_h && r.optimal; }

}  // namespace

std::vector<Column<DmReportRow>> dm_columns() {
    CellLook trunc;
    trunc.truncate = true;
    CellLook dim_trunc = trunc;
    dim_trunc.dim = true;

    std::vector<Column<DmReportRow>> cols;
    cols.push_back(text_column("song", "Song", &DmReportRow::song, trunc));
    cols.push_back(text_column("artist", "Artist", &DmReportRow::artist, dim_trunc));
    cols.push_back(text_column("charter", "Charter", &DmReportRow::charter, dim_trunc));
    {
        Column<DmReportRow> c = column("actual", "Actual", true, "The score the player posted.");
        c.sort_key = [](const DmReportRow& r) { return SortKey{static_cast<double>(r.actual)}; };
        c.cell = [](const DmReportRow& r) { return group_thousands(r.actual); };
        cols.push_back(std::move(c));
    }
    {
        Column<DmReportRow> c = column(
            "optimal", "Hydra opt", true,
            "The optimal score Hydra found for the chart at SP cap __SP_CAP__, the Clone Hero "
            "rule.");
        c.sort_key = [](const DmReportRow& r) { return optional_key(r.optimal); };
        c.cell = [](const DmReportRow& r) {
            return r.optimal ? group_thousands(*r.optimal) : std::string(kDash);
        };
        cols.push_back(std::move(c));
    }
    {
        Column<DmReportRow> c = column(
            "delta", "Points left", true,
            "Hydra opt minus Actual. Marked over when the posted score is higher.");
        c.sort_key = [](const DmReportRow& r) { return optional_key(r.delta); };
        // An over-optimal score reads by how much it is over (D64), kept
        // even at another speed.
        c.cell = [](const DmReportRow& r) {
            if (!r.delta) return std::string(kDash);
            if (r.above_optimal) return "+" + group_thousands(-*r.delta) + " over";
            return group_thousands(*r.delta);
        };
        c.tone = [](const DmReportRow& r) {
            if (!r.delta || r.status == "other speed") return Tone::Dim;
            return r.status == "above optimal" ? Tone::Alert : Tone::Normal;
        };
        cols.push_back(std::move(c));
    }
    {
        // Sorts on the whole hundredths (percent_steps) the cell's text,
        // format_percent's, is written from.
        Column<DmReportRow> c = column(
            "pct_h", "% of opt", true,
            "Actual as a percent of Hydra opt. Only for scores played at __BASE_SPEED__% "
            "speed.");
        c.sort_key = [](const DmReportRow& r) {
            return has_percent(r) ? SortKey{static_cast<double>(*r.pct_h)} : SortKey{};
        };
        c.cell = [](const DmReportRow& r) {
            return has_percent(r) ? format_percent(r.actual, *r.optimal, 2) : std::string(kDash);
        };
        cols.push_back(std::move(c));
    }
    {
        Column<DmReportRow> c = column("fc", "FC", true, "Full combo: every note hit.");
        c.sort_key = [](const DmReportRow& r) { return SortKey{r.is_fc ? 1.0 : 0.0}; };
        c.cell = [](const DmReportRow& r) {
            return r.is_fc ? std::string("\xE2\x9C\x93") : std::string(kDash);  // U+2713
        };
        cols.push_back(std::move(c));
    }
    {
        Column<DmReportRow> c = column("percent", "Percent", true,
                                       "The percent the leaderboard lists for this score.");
        c.sort_key = [](const DmReportRow& r) { return SortKey{static_cast<double>(r.percent)}; };
        c.cell = [](const DmReportRow& r) { return std::to_string(r.percent) + "%"; };
        cols.push_back(std::move(c));
    }
    {
        Column<DmReportRow> c = column(
            "speed", "Speed", true,
            "The playback speed the score was set at. __BASE_SPEED__% is normal speed.");
        c.sort_key = [](const DmReportRow& r) { return SortKey{static_cast<double>(r.speed)}; };
        c.cell = [](const DmReportRow& r) { return std::to_string(r.speed) + "%"; };
        cols.push_back(std::move(c));
    }
    {
        Column<DmReportRow> c =
            column("rank", "Rank", true, "The score rank on this chart leaderboard.");
        c.sort_key = [](const DmReportRow& r) { return optional_key(r.rank); };
        c.cell = [](const DmReportRow& r) {
            return r.rank ? "#" + std::to_string(*r.rank) : std::string(kDash);
        };
        cols.push_back(std::move(c));
    }
    {
        // The page sorted the timestamp as text and showed its date part.
        Column<DmReportRow> c = column("posted", "Posted", false, "The date the score was posted.");
        c.sort_key = [](const DmReportRow& r) { return SortKey{r.posted}; };
        c.cell = [](const DmReportRow& r) {
            return r.posted.empty() ? std::string(kDash) : r.posted.substr(0, 10);
        };
        c.look.dim = true;
        cols.push_back(std::move(c));
    }
    {
        Column<DmReportRow> c = column(
            "status", "Status", false,
            "Under optimal, At optimal or Above optimal when Hydra has a result. Not analyzed: "
            "the chart is in your library but has no current result for this mode at SP cap "
            "__SP_CAP__. No paths: analyzed, but the analysis kept no path. Not in your "
            "library: the last scan did not find it. Other speed: played at a speed other than "
            "__BASE_SPEED__%. Clone Hero keeps a separate leaderboard per speed, so it is shown "
            "but not compared.");
        c.sort_key = [](const DmReportRow& r) { return SortKey{r.status}; };
        c.cell = [](const DmReportRow& r) { return r.status; };
        c.look.chip = true;
        cols.push_back(std::move(c));
    }
    return cols;
}

SortSpec dm_first_sort() { return {"delta", report_view::SortDir::Descending}; }

std::vector<StatusChoice> status_choices() {
    // The statuses are collect_dm_rows' words for DmReportRow::status.
    return {
        {std::nullopt, "All charts"},
        {std::string("under optimal"), "Under optimal"},
        {std::string("at optimal"), "At optimal"},
        {std::string("above optimal"), "Above optimal"},
        {std::string("not analyzed"), "Not analyzed (in your library)"},
        {std::string("no paths"), "No paths (analyzed, none kept)"},
        {std::string("not in library"), "Not in your library"},
        {std::string("other speed"), "Other speed"},
    };
}

std::function<bool(const DmReportRow&)> dm_keep(std::optional<std::string> status) {
    return [status = std::move(status)](const DmReportRow& r) {
        return !status || r.status == *status;
    };
}

std::string dm_search_text(const DmReportRow& row) {
    return html::search_field(row.song, row.artist, row.charter);
}

}  // namespace hydra::app::dm_report_view
