#include "report_samples.h"

#include <cstdint>
#include <optional>
#include <string>

namespace report_samples {

using namespace hydra::app;

std::vector<report::ReportRow> sample_path_rows() {
    std::vector<report::ReportRow> paths;
    auto add_path = [&](const std::string& song, const char* mode, int rank,
                        const std::string& path, int64_t score, std::optional<double> ms,
                        std::optional<double> efill) {
        report::ReportRow r;
        r.song = song;
        r.artist = "Artist of " + song;
        r.charter = "Charter & Co";
        r.mode = mode;
        r.rank = rank;
        r.path = path;
        r.score = score;
        r.acts = 3 + rank;
        r.skip = rank - 1;
        r.ms = ms;
        auto [tier, tok] = report::tier_for(ms, kSampleHitWindowMs);
        r.tier = tier;
        r.tok = tok;
        r.efill = efill;
        r.mult = 2.345 + rank;
        r.sqin = rank;
        r.sqout = 2 - rank % 2;
        r.notes = 1200 + rank;
        r.hyhash = song;
        r.copies = 1;
        paths.push_back(r);
    };
    std::string long_path;
    for (int i = 0; i < 80; ++i) long_path += "1-E2+ ";
    add_path("Song A", "Expert Pro Drums, 2x Bass", 1, "1-E2+ 0-E1", 123456, 12.5, -3.25);
    add_path("Song A", "Expert Pro Drums, 2x Bass", 2, "1-E2 0-E1-", 123000, 48.0, std::nullopt);
    add_path("Song A", "Hard Drums, 1x Bass", 1, "0 0 1", 98000, std::nullopt, std::nullopt);
    add_path("Song B", "Expert Pro Drums, 2x Bass", 1, long_path, 250000, 171.0, 4.5);
    add_path("Song B", "Expert Pro Drums, 2x Bass", 2, "2 1-E3", 249500, 1.5, 0.0);
    add_path("Song C", "Expert Drums, 1x Bass", 1, "1 1 1", 77000, 90.0, std::nullopt);
    return paths;
}

std::vector<dm_report::DmReportRow> sample_dm_rows() {
    std::vector<dm_report::DmReportRow> dm;
    // The delta, the percent (in hundredths, as the payload carries it) and
    // whether the score is above optimal are typed, not worked out again from
    // the two scores.
    auto add_dm = [&](const char* song, int64_t actual, std::optional<int64_t> optimal,
                      std::optional<int64_t> delta, std::optional<int64_t> pct_h,
                      const char* status, bool above, bool fc, std::optional<int> rank) {
        dm_report::DmReportRow r;
        r.song = song;
        r.artist = "Artist";
        r.charter = "Charter";
        r.identifier = "hash";
        r.actual = actual;
        r.optimal = optimal;
        r.delta = delta;
        r.pct_h = pct_h;
        r.is_fc = fc;
        r.percent = fc ? 100 : 97;
        r.speed = 100;
        r.rank = rank;
        r.posted = "2026-09-20T12:34:56Z";
        r.status = status;
        r.above_optimal = above;
        dm.push_back(r);
    };
    add_dm("Song A", 120000, 123456, 3456, 9720, "under optimal", false, true, 3);
    add_dm("Song B", 251000, 250000, -1000, 10040, "above optimal", true, false, 1);
    add_dm("Song C", 90000, std::nullopt, std::nullopt, std::nullopt, "not in library", false,
           false, std::nullopt);
    add_dm("Song D", 80000, std::nullopt, std::nullopt, std::nullopt, "not analyzed", false,
           false, std::nullopt);
    return dm;
}

}  // namespace report_samples
