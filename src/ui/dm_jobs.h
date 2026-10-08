// The two dmleaderboards jobs: fetching the ladder for the user picker
// (DmFetchUsersJob) and building one user's comparison (DmReportJob).
// AppState owns both; library_dialogs.cpp's picker reads the fetch, and
// AppState collects the comparison into its dm_report slot. Both wrap
// net/dmbot_client.h calls, which the free-tier backend can leave hanging
// for tens of seconds.

#ifndef HYDRA_UI_DM_JOBS_H
#define HYDRA_UI_DM_JOBS_H

#include <memory>
#include <string>
#include <vector>

#include "app/dm_report.h"
#include "net/dmbot_client.h"
#include "store/record_store.h"
#include "ui/job_base.h"

namespace hydra::ui {

// ---- DmFetchUsersJob --------------------------------------------------

// Fetches the dmleaderboards ladder (GET /api/all-users) for the searchable
// picker. Off the render thread because the render.com backend cold-starts —
// the first request after an idle spell can take tens of seconds.
class DmFetchUsersJob : public ResultJobBase {
public:
    DmFetchUsersJob() = default;
    ~DmFetchUsersJob() { shutdown(); }

    void start();

    // Valid once finished() && ok(); the picker takes ownership (call once).
    std::vector<net::DmUser>& users() { return users_; }

private:
    void run();
    std::vector<net::DmUser> users_;
};

// ---- DmReportJob ------------------------------------------------------

// Fetches one user's scores (GET /api/user/{id}/scores) and joins them
// against the store by chart hash, in memory, for the comparison window
// (D103). Same off-thread + cold-start handling as DmFetchUsersJob. It
// writes and opens nothing.
class DmReportJob : public ResultJobBase {
public:
    DmReportJob(store::RecordStore& store, std::string discord_id, std::string username,
                std::string chartmode, store::Lens lens);
    ~DmReportJob() { shutdown(); }

    void start();

    // Valid once finished() && ok(): the comparison, shared with AppState's
    // slot rather than copied.
    const std::shared_ptr<const app::dm_report::GeneratedDmReport>& result() const {
        return result_;
    }

private:
    void run();
    store::RecordStore& store_;
    std::string discord_id_;
    std::string username_;
    std::string chartmode_;
    store::Lens lens_;
    std::shared_ptr<const app::dm_report::GeneratedDmReport> result_;
};

}  // namespace hydra::ui

#endif  // HYDRA_UI_DM_JOBS_H
