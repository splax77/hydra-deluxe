// The updater's two jobs (src/ui/update_job.h), run on GitHub's saved files
// through Hydra's real network code. The replay (tests/update_replay.h) is the
// only stand-in: it sends GitHub's addresses to a server on this PC. There is
// no "installed" switch, no launcher, no fetcher or downloader and no test
// key: the download is checked with the key built into Hydra.
//
// The jobs are run directly, outside AppState. AppState is the only caller of
// ui::launch_installer, so nothing here can start an installer.
//
// The saved files come from testdata/update/real (update_reply.h, real_dir).

#include "update_replay.h"  // first: it brings in winsock2.h ahead of windows.h

#include "doctest.h"

#include <filesystem>
#include <optional>
#include <string>

#include "app/update_key.h"  // kUpdatePublicKey
#include "app/update_release.h"
#include "app/user_messages.h"  // plain_error, kUpdateUnverifiedSentence
#include "core/app_version.h"   // parse_version
#include "core/error_kind.h"
#include "core/winstr.h"
#include "json.hpp"
#include "net/http.h"
#include "ui/update_job.h"
#include "update_reply.h"
#include "wait_util.h"

using namespace hydra;
using update_fixtures::GitHubReplay;
using update_fixtures::ScopedTempFolder;

namespace {

bool file_exists(const std::string& path) { return std::filesystem::exists(os_path(path)); }

// Starts `job` and waits for it to finish, by testwait's cap.
template <class Job>
void run_to_end(Job& job, const std::string& what) {
    job.start();
    testwait::wait_until([&] { return job.finished(); }, what);
}

// A string field of the saved reply, read by the test straight from the JSON.
std::string saved_field(const char* key) {
    return nlohmann::json::parse(update_fixtures::real_reply_text()).at(key).get<std::string>();
}

// The release the check job reads from the replay's saved reply. Fails the
// test, with the job's own message, when the check does not finish ok.
app::UpdateRelease checked_release() {
    ui::UpdateCheckJob check;
    run_to_end(check, "the update check to finish");
    REQUIRE_MESSAGE(check.ok(), "the check failed: " << check.message() << " (" << check.error() << ")");
    return check.release();
}

// The update folder Hydra itself works out (ui::update_folder), which the
// scratch temp folder `temp` must hold. Fails the test otherwise, before the
// job writes or deletes anything.
std::string scratch_update_folder(const ScopedTempFolder& temp) {
    const std::string folder = ui::update_folder();
    REQUIRE_MESSAGE(same_folder(folder, join_folder(temp.folder(), "HydraDeluxe-update")),
                    "update_folder() is " << folder << ", not inside the scratch folder " << temp.folder());
    return folder;
}

}  // namespace

TEST_CASE("update jobs, real: the check reads GitHub's saved reply through the real transport") {
    net::set_fetcher({});  // the real WinHTTP transport
    GitHubReplay replay;

    ui::UpdateCheckJob job;
    run_to_end(job, "the update check to finish");
    REQUIRE_MESSAGE(job.ok(), "the check failed: " << job.message() << " (" << job.error() << ")");

    // The version is the reply's own tag, read by the one version parser.
    const std::optional<core::AppVersion> tagged = core::parse_version(saved_field("tag_name"));
    REQUIRE(tagged.has_value());
    CHECK(job.release().version == *tagged);
    CHECK(job.release().notes == saved_field("body"));
    CHECK(replay.server().requests(replay.check_route()) == 1);
    CHECK(replay.server().total_requests() == 1);
}

TEST_CASE("update jobs, real: the saved installer downloads and passes the key built into Hydra") {
    net::set_fetcher({});  // the real WinHTTP transport
    net::set_downloader({});
    ScopedTempFolder temp("update_jobs_real_download");
    GitHubReplay replay;
    const std::string folder = scratch_update_folder(temp);

    ui::UpdateDownloadJob job(checked_release(), folder, app::kUpdatePublicKey, /*reuse=*/false);
    run_to_end(job, "the update download to finish");
    REQUIRE_MESSAGE(job.ok(), "the download failed: " << job.message() << " (" << job.error() << ")");

    // The download is checked and ready: the last step the automated tests
    // cover. Compared outside CHECK, so a failure prints the sizes, not 30 MB
    // of bytes.
    const std::string saved = read_file_text(replay.installer_file());
    const std::string got = file_exists(job.installer_path()) ? read_file_text(job.installer_path()) : "";
    const bool same = got == saved;
    CHECK_MESSAGE(same, "the file at " << job.installer_path() << " holds " << got.size()
                                       << " bytes; the saved installer holds " << saved.size());
    CHECK_FALSE(file_exists(job.installer_path() + ".part"));
    CHECK(replay.server().requests(replay.installer_route()) == 1);
    CHECK(replay.server().requests(replay.signature_route()) == 1);
}

TEST_CASE("update jobs, real: another release's real signature is refused and no installer is left") {
    net::set_fetcher({});  // the real WinHTTP transport
    net::set_downloader({});
    ScopedTempFolder temp("update_jobs_real_other_sig");
    GitHubReplay replay;
    const std::string folder = scratch_update_folder(temp);

    // A real signature, of a different file: the earlier release's .sig.
    const std::string other_sig = read_file_text(update_fixtures::other_release_sig_path());
    REQUIRE(other_sig != read_file_text(update_fixtures::real_asset_path(replay.release().sig_url)));
    replay.server().set_route(replay.signature_route(), "200 OK", other_sig);

    ui::UpdateDownloadJob job(checked_release(), folder, app::kUpdatePublicKey, /*reuse=*/false);
    run_to_end(job, "the refused update download to finish");
    CHECK_FALSE(job.ok());
    CHECK(job.outcome() == ui::JobOutcome::Failed);
    CHECK(job.message() == app::kUpdateUnverifiedSentence);
    CHECK_FALSE(file_exists(job.installer_path()));
    CHECK_FALSE(file_exists(job.installer_path() + ".part"));
    // The installer really came down, and the signature was asked for: the
    // refusal is the check's, not a failed transfer's.
    CHECK(replay.server().requests(replay.installer_route()) == 1);
    CHECK(replay.server().requests(replay.signature_route()) == 1);
}

TEST_CASE("update jobs, real: a check whose server is down fails with the transport's own sentence") {
    net::set_fetcher({});  // the real WinHTTP transport
    GitHubReplay replay;
    // The map stays, so GitHub's address still goes to the local port, where
    // nothing listens now: the connection is really refused.
    replay.stop();

    // The error the real transport throws for the same address, in this test.
    std::optional<ErrorKind> kind;
    std::string expected;
    try {
        (void)net::http_get(ui::kLatestReleaseUrl, nullptr, NetService::GitHub);
    } catch (const KindedError& e) {
        kind = e.kind();
        expected = app::plain_error(e);
    }
    REQUIRE_MESSAGE(kind.has_value(), "http_get did not throw a KindedError for the stopped server");
    REQUIRE(*kind == ErrorKind::NetUnreachable);
    REQUIRE_FALSE(expected.empty());

    ui::UpdateCheckJob job;
    run_to_end(job, "the failed update check to finish");
    CHECK_FALSE(job.ok());
    CHECK(job.message() == expected);
}
