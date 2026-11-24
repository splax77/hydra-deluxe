#include "app/dm_report_view.h"

#include "app/library_query.h"   // search_field
#include "core/model.h"          // group_thousands, kCloneHeroSpCap
#include "core/strutil.h"        // replace_all

namespace hydra::app::dm_report_view {

using report_view::CellKind;
using report_view::CellLook;
using report::kDash;
using report_view::optional_key;
using report_view::SortKey;
using report_view::text_column;
using report_view::Tone;

namespace {

// The definitions name the base speed through __BASE_SPEED__ and Clone
// Hero's cap through __SP_CAP__, filled from net::kBaseSpeedPercent and
// kCloneHeroSpCap.
std::string fill_constants(std::string definition) {
    return replace_all(
        replace_all(std::move(definition), "__BASE_SPEED__",
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

// A column of the counts and scores that line up down the table.
Column<DmReportRow> figure_column(std::string id, std::string title, std::string definition) {
    Column<DmReportRow> c = column(std::move(id), std::move(title), true, std::move(definition));
    c.look.kind = CellKind::Figure;
    return c;
}

// A value the row may lack, as a spreadsheet number: its number, or nothing
// where the cell writes a dash.
template <class T>
std::optional<double> optional_number(const std::optional<T>& v) {
    return v ? std::optional<double>{static_cast<double>(*v)} : std::nullopt;
}

// A whole percent held as a share of one, as Excel percents are: 97 is 0.97.
std::optional<double> whole_percent_number(int percent) { return percent / 100.0; }

// The Excel form of a "97%" cell, std::to_string's whole percent.
constexpr const char* kWholePercentFormat = "0%";

// The Excel form of the Points left text: a score under optimal reads as
// group_thousands does, and one above it (a negative delta) as "+1,234 over".
// Excel's second section is for negatives and shows the number without its
// minus sign.
constexpr const char* kPointsLeftFormat = "#,##0;\"+\"#,##0\" over\"";

// The Excel form of the Rank text, "#" and then the rank.
constexpr const char* kRankFormat = "\"#\"0";

}  // namespace

std::vector<Column<DmReportRow>> dm_columns() {
    CellLook trunc;
    trunc.truncate = true;
    CellLook dim_trunc = trunc;
    dim_trunc.dim = true;

    std::vector<Column<DmReportRow>> cols;
    cols.push_back(text_column("song", "Song", "", &DmReportRow::song, with_share(trunc, 48)));
    cols.push_back(text_column("artist", "Artist", "", &DmReportRow::artist, with_share(dim_trunc, 27)));
    cols.push_back(text_column("charter", "Charter", "", &DmReportRow::charter, with_share(dim_trunc, 25)));
    {
        Column<DmReportRow> c = figure_column("actual", "Actual","The score the player posted.");
        c.sort_key = [](const DmReportRow& r) { return SortKey{static_cast<double>(r.actual)}; };
        c.cell = [](const DmReportRow& r) { return group_thousands(r.actual); };
        c.number = [](const DmReportRow& r) {
            return std::optional<double>{static_cast<double>(r.actual)};
        };
        c.number_format = report_view::kGroupedNumberFormat;
        cols.push_back(std::move(c));
    }
    {
        Column<DmReportRow> c = figure_column(
            "optimal", "Hydra opt",
            "The optimal score Hydra found for the chart at SP cap __SP_CAP__, the Clone Hero "
            "rule.");
        c.sort_key = [](const DmReportRow& r) { return optional_key(r.optimal); };
        c.cell = [](const DmReportRow& r) {
            return r.optimal ? group_thousands(*r.optimal) : std::string(kDash);
        };
        c.number = [](const DmReportRow& r) { return optional_number(r.optimal); };
        c.number_format = report_view::kGroupedNumberFormat;
        cols.push_back(std::move(c));
    }
    {
        Column<DmReportRow> c = figure_column(
            "delta", "Points left",
            "Hydra opt minus Actual. Marked over when the posted score is higher.");
        c.sort_key = [](const DmReportRow& r) { return optional_key(r.delta); };
        // An over-optimal score reads by how much it is over (D64), kept
        // even at another speed.
        c.cell = [](const DmReportRow& r) {
            if (!r.delta) return std::string(kDash);
            if (r.above_optimal) return "+" + group_thousands(-*r.delta) + " over";
            return group_thousands(*r.delta);
        };
        c.number = [](const DmReportRow& r) { return optional_number(r.delta); };
        c.number_format = kPointsLeftFormat;
        c.tone = [](const DmReportRow& r) {
            if (!r.delta || r.status == dm_report::kStatusOtherSpeed) return Tone::Dim;
            return r.status == dm_report::kStatusAboveOptimal ? Tone::Alert : Tone::Normal;
        };
        cols.push_back(std::move(c));
    }
    {
        // The path report's Hardest ms for the best path, written and
        // exported the way that column is (report_view's ms helpers).
        Column<DmReportRow> c = figure_column(
            "timing", "Timing required",
            "The hardest squeeze or required early fill Hydra's optimal path needs, in ms: the "
            "same figure as the path report's Hardest ms. A dash means it needs none, or Hydra "
            "has no result.");
        c.sort_key = [](const DmReportRow& r) { return optional_key(r.timing_required); };
        c.cell = [](const DmReportRow& r) { return report_view::ms_cell(r.timing_required); };
        c.number = [](const DmReportRow& r) { return report_view::ms_number(r.timing_required); };
        c.number_format = report_view::kMsNumberFormat;
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
        Column<DmReportRow> c = figure_column("percent", "Percent",
                                              "The percent of notes the player hit.");
        c.sort_key = [](const DmReportRow& r) { return SortKey{static_cast<double>(r.percent)}; };
        c.cell = [](const DmReportRow& r) { return std::to_string(r.percent) + "%"; };
        c.number = [](const DmReportRow& r) { return whole_percent_number(r.percent); };
        c.number_format = kWholePercentFormat;
        cols.push_back(std::move(c));
    }
    {
        Column<DmReportRow> c = figure_column(
            "speed", "Speed",
            "The playback speed the score was set at. __BASE_SPEED__% is normal speed.");
        c.sort_key = [](const DmReportRow& r) { return SortKey{static_cast<double>(r.speed)}; };
        c.cell = [](const DmReportRow& r) { return std::to_string(r.speed) + "%"; };
        c.number = [](const DmReportRow& r) { return whole_percent_number(r.speed); };
        c.number_format = kWholePercentFormat;
        cols.push_back(std::move(c));
    }
    {
        Column<DmReportRow> c =
            figure_column("rank", "Rank","The score's rank on this chart's leaderboard.");
        c.sort_key = [](const DmReportRow& r) { return optional_key(r.rank); };
        c.cell = [](const DmReportRow& r) {
            return r.rank ? "#" + std::to_string(*r.rank) : std::string(kDash);
        };
        c.number = [](const DmReportRow& r) { return optional_number(r.rank); };
        c.number_format = kRankFormat;
        cols.push_back(std::move(c));
    }
    {
        // Sorts on the timestamp as text and shows its date part. An empty
        // date is an empty value (D103 item 17).
        Column<DmReportRow> c = column("posted", "Posted", false, "The date the score was posted.");
        c.sort_key = [](const DmReportRow& r) {
            return r.posted.empty() ? SortKey{} : SortKey{r.posted};
        };
        c.cell = [](const DmReportRow& r) {
            return r.posted.empty() ? std::string(kDash) : r.posted.substr(0, 10);
        };
        c.look.dim = true;
        cols.push_back(std::move(c));
    }
    {
        // The three optimal statuses together, then each other status's name
        // and gloss from dm_report::kStatusWords.
        using dm_report::status_words;
        std::string definition = std::string(status_words(dm_report::kStatusUnderOptimal).name) +
                                 ", " + status_words(dm_report::kStatusAtOptimal).name + " or " +
                                 status_words(dm_report::kStatusAboveOptimal).name +
                                 " when Hydra has a result.";
        for (const dm_report::StatusWords& w : dm_report::kStatusWords)
            if (*w.gloss) definition += std::string(" ") + w.name + ": " + w.gloss;
        Column<DmReportRow> c = column("status", "Status", false, std::move(definition));
        c.sort_key = [](const DmReportRow& r) { return SortKey{r.status}; };
        c.cell = [](const DmReportRow& r) { return r.status; };
        c.look.chip = true;
        cols.push_back(std::move(c));
    }
    return cols;
}

SortSpec dm_first_sort() { return {"delta", report_view::SortDir::Descending}; }

std::vector<StatusChoice> status_choices() {
    std::vector<StatusChoice> out = {{std::nullopt, "All charts"}};
    for (const dm_report::StatusWords& w : dm_report::kStatusWords)
        out.push_back({std::string(w.status), std::string(w.name) + w.choice_note});
    return out;
}

std::function<bool(const DmReportRow&)> dm_keep(std::optional<std::string> status) {
    return [status = std::move(status)](const DmReportRow& r) {
        return !status || r.status == *status;
    };
}

std::string dm_search_text(const DmReportRow& row) {
    return search_field(row.song, row.artist, row.charter);
}

}  // namespace hydra::app::dm_report_view
