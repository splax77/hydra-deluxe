#include "ui/dm_jobs.h"

#include <memory>
#include <stdexcept>
#include <utility>

#include "app/dm_report.h"
#include "core/error_kind.h"
#include "net/dmbot_client.h"

namespace hydra::ui {

// ---- DmFetchUsersJob --------------------------------------------------

void DmFetchUsersJob::start() { spawn([this] { run(); }); }

void DmFetchUsersJob::run() {
    run_guarded([this] {
        users_ = net::fetch_users(net::kDefaultApiBase, &cancel_);
        app::dm_report::sort_users_by_name(users_);
        // A cancel can land while the server is still answering; the fetch
        // then returns normally. The picker asked to stop, so stop.
        return !is_cancelled();
    });
}

// ---- DmReportJob ------------------------------------------------------

DmReportJob::DmReportJob(store::RecordStore& store, std::string discord_id, std::string username,
                         store::Profile profile)
    : store_(store),
      discord_id_(std::move(discord_id)),
      username_(std::move(username)),
      profile_(std::move(profile)) {}

void DmReportJob::start() { spawn([this] { run(); }); }

void DmReportJob::run() {
    run_guarded([this] {
        std::vector<net::DmScore> scores =
            net::fetch_scores(discord_id_, net::kDefaultApiBase, &cancel_);
        // The fetch only checks cancel between read chunks, so a cancel
        // pressed while the server was waking up arrives here. Stop before
        // anything is built.
        if (is_cancelled()) return false;
        // Join, tally, and framing all live behind generate_dm_report; the
        // job only fetches and hands the result over.
        // The store is read only from here, after the network wait, through a
        // connection of the report's own under one hold, as ReportJob::run
        // reads it.
        app::dm_report::GeneratedDmReport report = [&] {
            const std::unique_ptr<store::OwnReader> reader = store_.open_own_reader();
            const store::OwnReader::Hold moment = reader->hold();
            return app::dm_report::generate_dm_report(reader->reads(), scores, profile_,
                                                      username_);
        }();
        if (report.stats.total == 0)
            throw KindedError(ErrorKind::NoScores, "this user has no scores to compare");

        result_ = std::make_shared<const app::dm_report::GeneratedDmReport>(std::move(report));
        return true;
    });
}

}  // namespace hydra::ui
