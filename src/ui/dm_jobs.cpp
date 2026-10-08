#include "ui/dm_jobs.h"

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
        // A cancel can land while the server is still answering; the fetch
        // then returns normally. The picker asked to stop, so stop.
        return !is_cancelled();
    });
}

// ---- DmReportJob ------------------------------------------------------

DmReportJob::DmReportJob(store::RecordStore& store, std::string discord_id, std::string username,
                         std::string chartmode, store::Lens lens)
    : store_(store),
      discord_id_(std::move(discord_id)),
      username_(std::move(username)),
      chartmode_(std::move(chartmode)),
      lens_(lens) {}

void DmReportJob::start() { spawn([this] { run(); }); }

void DmReportJob::run() {
    run_guarded([this] {
        std::vector<net::DmScore> scores =
            net::fetch_scores(discord_id_, net::kDefaultApiBase, &cancel_);
        // The fetch only checks cancel between read chunks, so a cancel
        // pressed while the server was waking up arrives here. Stop before
        // anything is built (audit B3).
        if (is_cancelled()) return false;
        // Join, tally, and framing all live behind generate_dm_report; the
        // job only fetches and hands the result over.
        app::dm_report::GeneratedDmReport report =
            app::dm_report::generate_dm_report(store_, scores, chartmode_, lens_,
                                               username_);
        if (report.stats.total == 0)
            throw KindedError(ErrorKind::NoScores, "this user has no scores to compare");

        // Nothing reads the page any more; the window draws the rows. Its
        // text goes now rather than living on with the rows (T7 removes the
        // field).
        std::string().swap(report.html);
        result_ = std::make_shared<const app::dm_report::GeneratedDmReport>(std::move(report));
        return true;
    });
}

}  // namespace hydra::ui
