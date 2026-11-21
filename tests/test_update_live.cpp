// The one updater test that asks the real GitHub. It proves the two things the
// replay (tests/test_update_jobs_real.cpp) cannot: Hydra's own HTTPS
// connection to api.github.com works, and the reply GitHub sends today still
// parses. It makes one request and downloads no installer.
//
// It carries doctest's skip mark, so an ordinary run of hydra_tests leaves it
// out. The main session runs it by name with --no-skip before a release
// (docs/development.md, "Every release: build and sign").

#include "doctest.h"

#include <string>

#include "app/update_release.h"
#include "net/http.h"
#include "ui/update_job.h"
#include "wait_util.h"

using namespace hydra;

TEST_CASE("update live: GitHub's real latest-release reply reaches Hydra and parses" * doctest::skip()) {
    // The real transport and the real addresses, in case an earlier test in
    // the same run left a fetcher or an address map installed.
    net::set_fetcher({});
    net::set_address_map_for_test({});

    ui::UpdateCheckJob job;
    job.start();
    testwait::wait_until([&] { return job.finished(); }, "the live update check to finish");
    REQUIRE_MESSAGE(job.ok(), "the live check failed: " << job.message() << " (" << job.error() << ")");

    const app::UpdateRelease& release = job.release();
    CHECK_FALSE(release.installer_url.empty());
    CHECK_FALSE(release.sig_url.empty());
}
