// Tests for the updater's state in AppState (ui/update_job.h, ui/app_state.h)
// and the version bar's status (AppState::version_bar_status), as D260 rules.
// Tests on ReplayWorld run Hydra's real network code against GitHub's saved
// files, replayed on this PC (update_replay.h). Tests on UpdateWorld replace
// the reply, the download and the installer start through the test seams.

#include "update_replay.h"  // first: it brings in winsock2.h ahead of windows.h

#include "doctest.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "app/config.h"
#include "app/user_messages.h"
#include "core/app_version.h"
#include "core/error_kind.h"
#include "core/winstr.h"
#include "json.hpp"
#include "net/http.h"
#include "scoped_hook.h"
#include "scratch_paths.h"
#include "temp_util.h"
#include "ui/app_state.h"
#include "ui/update_job.h"
#include "update_fixtures.h"
#include "wait_util.h"

using hydra::ErrorKind;
using hydra::KindedError;
using update_fixtures::GitHubReplay;
using update_fixtures::real_not_found_text;
using update_fixtures::real_reply_at;
using update_fixtures::real_reply_text;
using update_fixtures::release_reply_at;
using update_fixtures::sample_path;
using update_fixtures::ScopedTempFolder;
using update_fixtures::test_public_key;
using update_fixtures::update_dir;
using hydra::app::Settings;
using hydra::ui::AppState;
using hydra::ui::VersionBarStatus;
using Phase = hydra::ui::UpdateState::Phase;
using Dot = hydra::ui::VersionBarStatus::Dot;

namespace {

std::string running_text() { return hydra::core::version_text(hydra::core::running_version()); }

// What the canned network serves. `fail`, when set, is thrown by the
// releases/latest fetch instead of the reply.
struct Canned {
    std::string reply = release_reply_at("v9.9.9");
    std::string sig = hydra::read_file_text(update_dir() + "/sample.bin.sig");
    std::function<void()> fail;
    std::atomic<int> downloads{0};
    // The download's body: copies sample.bin unless replaced. `size_cap` is
    // the release's stated installer size, as the job passes it.
    std::function<void(const std::string& path, uint64_t size_cap,
                       std::atomic<uint64_t>* received, const std::atomic<bool>* cancel)>
        download = [](const std::string& path, uint64_t, std::atomic<uint64_t>* received,
                      const std::atomic<bool>*) {
            std::filesystem::copy_file(hydra::os_path(sample_path()), hydra::os_path(path),
                                       std::filesystem::copy_options::overwrite_existing);
            if (received) received->store(std::filesystem::file_size(hydra::os_path(path)));
        };
    // The installer start: "started", with a process that never ends, unless
    // replaced. `launched` is the installer path it was given.
    std::string launched;
    std::string log;
    std::function<hydra::ui::InstallerEnded()> launch = [] {
        return hydra::ui::InstallerEnded([] { return false; });
    };
};

// Everything an update test stands on: scratch INI and download folder, the
// installed-exe answer, and the three network and launch seams, all declared
// before the AppState the test builds so they outlive its jobs.
struct UpdateWorld {
    ScratchPaths paths;
    std::string folder;
    Canned canned;
    ScopedHook installed{hydra::app::set_installed_for_test, std::optional<bool>(true)};
    ScopedHook folder_seam;
    ScopedHook fetcher;
    ScopedHook downloader;
    ScopedHook launcher;

    explicit UpdateWorld(const std::string& tag)
        : paths(tag),
          folder(testtemp::temp_dir(tag + "_dl")),
          folder_seam(hydra::ui::set_update_folder_for_test, folder),
          fetcher(hydra::net::set_fetcher,
                  [this](const std::string& url, const std::atomic<bool>*) -> std::string {
                      if (url == hydra::ui::kLatestReleaseUrl) {
                          if (canned.fail) canned.fail();
                          return canned.reply;
                      }
                      if (url.size() > 4 && url.substr(url.size() - 4) == ".sig") return canned.sig;
                      throw std::runtime_error("no canned reply for " + url);
                  }),
          downloader(hydra::net::set_downloader,
                     [this](const std::string&, const std::string& path, uint64_t size_cap,
                            std::atomic<uint64_t>* received, const std::atomic<bool>* cancel) {
                         ++canned.downloads;
                         canned.download(path, size_cap, received, cancel);
                     }),
          launcher(hydra::ui::set_launcher_for_test,
                   [this](const std::string& installer, const std::string& log) {
                       canned.launched = installer;
                       canned.log = log;
                       return canned.launch();
                   }) {}
    ~UpdateWorld() { testtemp::remove_dir(folder); }

    // Settings as a start at the running version leaves them, so no
    // "Updated to" notice gets in the way.
    Settings settings() const {
        Settings s;
        s.last_run_version = running_text();
        return s;
    }
    std::unique_ptr<AppState> app() { return app_with(settings()); }
    std::unique_ptr<AppState> app_with(Settings s) {
        auto a = std::make_unique<AppState>(std::move(s), nullptr);
        a->update_public_key = test_public_key();
        return a;
    }
};

// Everything an update test on real inputs stands on: scratch INI, a scratch
// temp folder for this process (so update_folder() lies inside it), GitHub
// replayed on this PC, and the launcher guard. The release reply, the
// installer, its signature and Hydra's built-in release key are all real.
//
// Two stand-ins remain. The address: GitHubReplay sends GitHub's addresses to
// the local server (net::set_address_map_for_test). And "installed": AppState
// runs no check unless the build counts as installed, and a test build never
// does, so each test that needs a check sets the installed switch itself, in
// one named line. The fixture never sets it.
//
// The launcher seam is a guard, not a stand-in: no test may start an
// installer, so it counts any call and throws, and each test requires the
// count is zero.
struct ReplayWorld {
    ScratchPaths paths;     // first: its names are taken from the real temp folder
    ScopedTempFolder temp;
    GitHubReplay replay;
    std::atomic<int> launcher_calls{0};
    ScopedHook launcher;

    // `reply` is what the check's address answers; serve() changes it.
    explicit ReplayWorld(const std::string& tag, const std::string& reply = real_reply_text())
        : paths(tag),
          temp(tag + "_tmp"),
          replay(reply),
          launcher(hydra::ui::set_launcher_for_test,
                   [this](const std::string& installer,
                          const std::string&) -> hydra::ui::InstallerEnded {
                       ++launcher_calls;
                       throw std::logic_error("a test reached the installer start, for " + installer +
                                              "; no test may start an installer");
                   }) {
        require_scratch_update_folder();
    }

    // The check's address answers `reply` from the next request on. The
    // installer and signature routes stay, so `reply` must name the saved
    // release's assets (the saved reply, or real_reply_at of a tag).
    void serve(const std::string& reply) {
        replay.server().set_route(replay.check_route(), "200 OK", reply);
    }

    // AppState deletes update_folder() at every start, so a test that builds
    // one, or writes there, stops first unless that folder is inside the
    // scratch temp folder.
    void require_scratch_update_folder() const {
        const std::string scratch = hydra::join_folder(temp.folder(), "HydraDeluxe-update");
        const std::string actual = hydra::ui::update_folder();
        REQUIRE_MESSAGE(hydra::same_folder(actual, scratch),
                        "update_folder() is " << actual << ", not inside the scratch temp folder "
                                              << temp.folder());
    }

    // Settings as a start at the running version leaves them, so no
    // "Updated to" notice gets in the way.
    Settings settings() const {
        Settings s;
        s.last_run_version = running_text();
        return s;
    }
    std::unique_ptr<AppState> app() { return app_with(settings()); }
    std::unique_ptr<AppState> app_with(Settings s) {
        require_scratch_update_folder();
        return std::make_unique<AppState>(std::move(s), nullptr);
    }
};

// The version of the saved reply, read from its tag_name.
hydra::core::AppVersion saved_reply_version() {
    const std::string tag =
        nlohmann::json::parse(real_reply_text()).at("tag_name").get<std::string>();
    const std::optional<hydra::core::AppVersion> version = hydra::core::parse_version(tag);
    REQUIRE_MESSAGE(version.has_value(), "the saved reply's tag_name is not a version: " << tag);
    return *version;
}

// The release notes of the saved reply, read from its body.
std::string saved_reply_notes() {
    return nlohmann::json::parse(real_reply_text()).at("body").get<std::string>();
}

// Ticks the app until `done`, as its frames would.
void tick_until(AppState& app, const std::function<bool()>& done, const std::string& what) {
    testwait::wait_until(
        [&] {
            app.tick(0.0);
            return done();
        },
        what);
}

void settle_check(AppState& app) {
    tick_until(app, [&] { return !app.update_check_running(); }, "the update check to finish");
}

// Ticks the app until `done`, for a step that only the running download job
// reaches: its own body, or the tick that collects it
// (AppState::collect_update_download). Once that job has ended, the step can
// no longer come, so the wait fails at once instead of at the cap. The tick
// sits inside `done`, so the second ask that wait_until_or_gone makes after
// `gone` also collects a job that finished between two ticks.
void tick_until_download_step(AppState& app, const std::function<bool()>& done,
                              const std::string& what) {
    testwait::wait_until_or_gone(
        [&] {
            app.tick(0.0);
            return done();
        },
        [&] { return !app.update_download_job || app.update_download_job->finished(); }, what,
        "the download job has ended, and nothing else moves the update on, so it can no "
        "longer happen");
}

// The offer of the canned v9.9.9, from the start-up check.
std::unique_ptr<AppState> offered_app(UpdateWorld& w) {
    std::unique_ptr<AppState> app = w.app();
    settle_check(*app);  // the construction's automatic check
    REQUIRE(app->update.phase == Phase::Offered);
    return app;
}

// The offer of the release the replay serves, from the start-up check. The
// test has set the installed switch.
std::unique_ptr<AppState> offered_app(ReplayWorld& w) {
    std::unique_ptr<AppState> app = w.app();
    settle_check(*app);  // the construction's automatic check
    REQUIRE(app->update.phase == Phase::Offered);
    return app;
}

bool exists(const std::string& path) { return hydra::file_exists_utf8(path); }

}  // namespace

// ---- Every installed start checks; other copies never do -------------------

TEST_CASE("every start of the installed exe checks, and a copy never does") {
    ReplayWorld w("update_on_start");
    {
        // The installed exe: AppState checks only for it.
        const ScopedHook installed(hydra::app::set_installed_for_test, std::optional<bool>(true));
        // Each start checks: no time since the last check holds one back.
        const int starts = 2;
        for (int start = 0; start < starts; ++start) {
            CAPTURE(start);
            std::unique_ptr<AppState> app = w.app();
            CHECK(app->update_check_running());  // before the first tick
            const VersionBarStatus status = app->version_bar_status();
            CHECK(status.text == hydra::ui::kCheckingForUpdatesText);
            CHECK(status.dimmed.empty());
            CHECK(status.dot == Dot::Neutral);
            CHECK_FALSE(status.update_button);
            settle_check(*app);
        }
        CHECK(w.replay.server().requests(w.replay.check_route()) == starts);
    }
    // A test build is not the copy the installer put in place, so with no
    // switch set the registry check itself says so.
    REQUIRE_FALSE(hydra::app::installed_by_setup());
    const int asked_before = w.replay.server().total_requests();
    std::unique_ptr<AppState> app = w.app();
    CHECK_FALSE(app->update_check_running());
    // A check starts inside check_updates_on_start or not at all, so the
    // answer is known as soon as the call returns.
    app->check_updates_on_start();
    CHECK_FALSE(app->update_check_running());
    CHECK(app->update.phase == Phase::None);
    const VersionBarStatus status = app->version_bar_status();
    CHECK(status.text.empty());
    CHECK(status.dimmed == hydra::ui::kOnlyInstalledChecksText);
    CHECK(status.dot == Dot::Neutral);
    CHECK_FALSE(status.update_button);
    CHECK(w.replay.server().total_requests() == asked_before);  // nothing was asked
    CHECK(w.launcher_calls.load() == 0);
}

// ---- The offer, Later, Skip this version and Up to date --------------------

TEST_CASE("a newer release is offered; Later keeps an Update button and writes nothing") {
    ReplayWorld w("update_later", real_reply_at("v9.9.9"));
    // The installed exe: AppState checks only for it.
    const ScopedHook installed(hydra::app::set_installed_for_test, std::optional<bool>(true));
    std::unique_ptr<AppState> app = offered_app(w);
    CHECK(app->update.text == "Hydra Deluxe 9.9.9 is available.");
    REQUIRE(app->update.release.has_value());
    CHECK(hydra::core::version_text(app->update.release->version) == "9.9.9");
    {
        const VersionBarStatus offered = app->version_bar_status();
        CHECK(offered.text == "Hydra Deluxe 9.9.9 is available.");
        CHECK(offered.dot == Dot::Available);
        CHECK_FALSE(offered.update_button);  // the phase draws its own three
    }
    // The start at the running version wrote no INI; Later writes none either.
    const auto ini_text = [&] { return exists(w.paths.ini) ? hydra::read_file_text(w.paths.ini) : ""; };
    const std::string before = ini_text();
    CHECK(Settings::load_file(w.paths.ini).skipped_update_version.empty());

    app->later();
    CHECK(app->update.phase == Phase::None);
    CHECK(app->update.text.empty());
    CHECK(ini_text() == before);
    const VersionBarStatus status = app->version_bar_status();
    CHECK(status.text == "Hydra Deluxe 9.9.9 is available.");
    CHECK(status.dot == Dot::Available);
    CHECK(status.dimmed.empty());
    CHECK(status.update_button);
    REQUIRE(app->update.found.has_value());
    // The notes are the saved reply's body, as GitHub sent it.
    const std::string notes = saved_reply_notes();
    CHECK_FALSE(notes.empty());
    CHECK(app->update.found->notes == notes);

    // The resting bar's Update starts the download as the offer's does. The
    // installer route holds the real download after its first byte, so the
    // download never ends and no installer start can come.
    const std::string route = w.replay.installer_route();
    w.replay.server().pause(route, 1);
    app->update_now();
    CHECK(app->update.phase == Phase::Downloading);
    CHECK(app->update.text == "Downloading Hydra Deluxe 9.9.9...");
    tick_until_download_step(*app, [&] { return w.replay.server().held(route); },
        "the installer download to be held part-way");
    CHECK(app->update.phase == Phase::Downloading);
    CHECK(app->update.text == "Downloading Hydra Deluxe 9.9.9...");
    REQUIRE(app->update.release.has_value());
    const std::string installer = hydra::join_folder(
        hydra::ui::update_folder(), hydra::ui::update_installer_name(*app->update.release));

    // Cancel goes back to the offer and leaves no file.
    app->cancel_update();
    tick_until_download_step(*app, [&] { return app->update.phase == Phase::Offered; },
        "the offer to come back");
    CHECK(app->update.text == "Hydra Deluxe 9.9.9 is available.");
    CHECK_FALSE(exists(installer + ".part"));
    CHECK_FALSE(exists(installer));
    CHECK(w.launcher_calls.load() == 0);
}

TEST_CASE("Skip this version names the release as skipped; a newer one is offered") {
    ReplayWorld w("update_skip", real_reply_at("v9.9.9"));
    // The installed exe: AppState checks only for it.
    const ScopedHook installed(hydra::app::set_installed_for_test, std::optional<bool>(true));
    {
        std::unique_ptr<AppState> app = offered_app(w);
        app->skip_version();
        CHECK(app->update.phase == Phase::None);
        CHECK(Settings::load_file(w.paths.ini).skipped_update_version == "9.9.9");
        const VersionBarStatus status = app->version_bar_status();
        CHECK(status.text == "Hydra Deluxe 9.9.9 is available.");
        CHECK(status.dimmed == "Skipped.");
        CHECK(status.dot == Dot::Available);
        CHECK_FALSE(status.update_button);
        app->update_now();  // no button, so nothing starts
        CHECK(app->update.phase == Phase::None);
    }
    // A fresh start honors the skip and still names the release.
    {
        std::unique_ptr<AppState> app = w.app_with(Settings::load_file(w.paths.ini));
        settle_check(*app);
        CHECK(app->update.phase == Phase::None);
        CHECK(app->update.text.empty());
        const VersionBarStatus status = app->version_bar_status();
        CHECK(status.text == "Hydra Deluxe 9.9.9 is available.");
        CHECK(status.dimmed == "Skipped.");
        CHECK(status.dot == Dot::Available);
        CHECK_FALSE(status.update_button);
    }
    // The next start with 9.9.10 out offers it.
    w.serve(real_reply_at("v9.9.10"));
    std::unique_ptr<AppState> app = w.app_with(Settings::load_file(w.paths.ini));
    settle_check(*app);
    CHECK(app->update.phase == Phase::Offered);
    CHECK(app->update.text == "Hydra Deluxe 9.9.10 is available.");
    CHECK(w.launcher_calls.load() == 0);
}

TEST_CASE("the same or an older release lands in the resting Up to date") {
    ReplayWorld w("update_uptodate");
    // The installed exe: AppState checks only for it.
    const ScopedHook installed(hydra::app::set_installed_for_test, std::optional<bool>(true));
    // GitHub's saved reply, untouched, names a release that is not newer than
    // this build: a real "not newer" reply.
    REQUIRE_FALSE(hydra::core::is_newer_version(saved_reply_version(), hydra::core::running_version()));
    const std::vector<std::pair<std::string, std::string>> replies = {
        {"the saved reply", real_reply_text()},
        {"the saved reply at the running version", real_reply_at("v" + running_text())},
    };
    for (const std::pair<std::string, std::string>& named_reply : replies) {
        CAPTURE(named_reply.first);
        w.serve(named_reply.second);
        std::unique_ptr<AppState> app = w.app();
        settle_check(*app);
        CHECK(app->update.phase == Phase::None);
        CHECK(app->update.text.empty());
        CHECK_FALSE(app->update.release.has_value());
        CHECK_FALSE(app->update.found.has_value());
        const VersionBarStatus status = app->version_bar_status();
        CHECK(status.text == "Up to date");
        CHECK(status.dimmed.empty());
        CHECK(status.dot == Dot::UpToDate);
        CHECK_FALSE(status.update_button);
    }
    CHECK(w.launcher_calls.load() == 0);
}

// ---- Failures --------------------------------------------------------------

TEST_CASE("a failed start-up check shows in the resting status with its plain_error") {
    ReplayWorld w("update_fail");
    // The installed exe: AppState checks only for it.
    const ScopedHook installed(hydra::app::set_installed_for_test, std::optional<bool>(true));

    // The two real failures, one per subcase: GitHub's saved 404 reply at the
    // check's address, and a server that is down, so the connection is really
    // refused.
    SUBCASE("GitHub answers the check with its saved 404") {
        w.replay.server().set_route(w.replay.check_route(), "404 Not Found", real_not_found_text());
    }
    SUBCASE("the server is down") { w.replay.stop(); }

    // The bar's dimmed line is plain_error of the error Hydra's transport
    // throws for the check's address as it answers now.
    std::string sentence;
    try {
        (void)hydra::net::http_get(hydra::ui::kLatestReleaseUrl, nullptr, hydra::NetService::GitHub);
    } catch (const std::exception& e) {
        sentence = hydra::app::plain_error(e);
    }
    REQUIRE_FALSE(sentence.empty());

    std::unique_ptr<AppState> app = w.app();
    // Only a tick collects the check's job, so before the first tick the
    // check is still running, whatever its request has done meanwhile.
    REQUIRE(app->update_check_running());
    {
        const VersionBarStatus checking = app->version_bar_status();
        CHECK(checking.text == hydra::ui::kCheckingForUpdatesText);
        CHECK(checking.dot == Dot::Neutral);
    }
    settle_check(*app);
    CHECK(app->update.phase == Phase::None);
    CHECK(app->update.text.empty());
    CHECK(app->update.detail.empty());
    const VersionBarStatus status = app->version_bar_status();
    CHECK(status.text == hydra::ui::kCouldNotCheckText);
    CHECK(status.dimmed == sentence);
    CHECK(status.dot == Dot::Neutral);
    CHECK_FALSE(status.update_button);
    CHECK(w.launcher_calls.load() == 0);
}

// ---- The Updated note gives way to the offer or the resting status ---------

TEST_CASE("the Updated note's OK offers a found release, or gives way to the resting status") {
    ReplayWorld w("update_note_then_offer", real_reply_at("v9.9.9"));
    // The installed exe: AppState checks only for it.
    const ScopedHook installed(hydra::app::set_installed_for_test, std::optional<bool>(true));
    Settings s;
    s.last_run_version = "0.0.1";
    {
        std::unique_ptr<AppState> app = w.app_with(s);
        settle_check(*app);
        CHECK(app->update.phase == Phase::Updated);
        CHECK(app->version_bar_status().dot == Dot::Neutral);
        app->dismiss_update_note();
        CHECK(app->update.phase == Phase::Offered);
        CHECK(app->update.text == "Hydra Deluxe 9.9.9 is available.");
    }
    {
        s.skipped_update_version = "9.9.9";
        std::unique_ptr<AppState> app = w.app_with(s);
        settle_check(*app);
        CHECK(app->update.phase == Phase::Updated);
        app->dismiss_update_note();
        CHECK(app->update.phase == Phase::None);
        const VersionBarStatus status = app->version_bar_status();
        CHECK(status.text == "Hydra Deluxe 9.9.9 is available.");
        CHECK(status.dimmed == "Skipped.");
        CHECK(status.dot == Dot::Available);
    }
    {
        // GitHub's saved reply, untouched, is a real "not newer" reply.
        REQUIRE_FALSE(
            hydra::core::is_newer_version(saved_reply_version(), hydra::core::running_version()));
        s.skipped_update_version.clear();
        w.serve(real_reply_text());
        std::unique_ptr<AppState> app = w.app_with(s);
        settle_check(*app);
        CHECK(app->update.phase == Phase::Updated);
        app->dismiss_update_note();
        CHECK(app->update.phase == Phase::None);
        CHECK(app->version_bar_status().text == "Up to date");
        CHECK(app->version_bar_status().dot == Dot::UpToDate);
    }
    CHECK(w.launcher_calls.load() == 0);
}

// ---- The one text of a version ---------------------------------------------

TEST_CASE("named_version is the app's name and the version") {
    CHECK(hydra::core::named_version(hydra::core::AppVersion{2, 11, 1}) == "Hydra Deluxe 2.11.1");
}

// ---- Update, the installer start and Cancel --------------------------------

TEST_CASE("once the installer is started, Later, Skip and Update do nothing and no check runs") {
    UpdateWorld w("update_started");
    std::unique_ptr<AppState> app = offered_app(w);
    app->update_now();
    tick_until_download_step(*app, [&] { return app->update.phase == Phase::Launching; },
        "the installer to start");
    CHECK(app->update.text == hydra::ui::kUpdateLaunchingText);
    REQUIRE(app->update.release.has_value());
    CHECK(w.canned.launched ==
          hydra::join_folder(w.folder, hydra::ui::update_installer_name(*app->update.release)));
    // The buttons are ignored by phase: Hydra stays as it is until the
    // installer closes it.
    app->later();
    app->skip_version();
    app->update_now();
    CHECK(app->update.phase == Phase::Launching);
    CHECK(app->update.release.has_value());
    CHECK(Settings::load_file(w.paths.ini).skipped_update_version.empty());
    CHECK_FALSE(app->update_check_running());
    CHECK(w.canned.downloads == 1);
}

TEST_CASE("an installer that ends while Hydra runs leaves decision 10's state and the file") {
    UpdateWorld w("update_declined");
    auto ended = std::make_shared<std::atomic<bool>>(false);
    w.canned.launch = [ended] { return hydra::ui::InstallerEnded([ended] { return ended->load(); }); };
    std::unique_ptr<AppState> app = offered_app(w);
    app->update_now();
    tick_until_download_step(*app, [&] { return app->update.phase == Phase::Launching; },
        "the installer to start");
    ended->store(true);  // the UAC prompt was declined, so Setup exits
    app->tick(0.0);
    CHECK(app->update.phase == Phase::Declined);
    CHECK(app->update.text == "The installer didn't start, so Hydra wasn't updated.");
    CHECK(exists(w.canned.launched));
    CHECK(w.canned.downloads == 1);

    // Update again starts the kept file, checked again, not downloaded again.
    ended->store(false);
    app->update_now();
    tick_until_download_step(*app, [&] { return app->update.phase == Phase::Launching; },
        "the second start");
    CHECK(w.canned.downloads == 1);
}

TEST_CASE("an installer that can't start leaves decision 10's state with Windows' reason") {
    UpdateWorld w("update_nostart");
    const std::runtime_error why("Access is denied.");
    w.canned.launch = [why]() -> hydra::ui::InstallerEnded { throw why; };
    std::unique_ptr<AppState> app = offered_app(w);
    app->update_now();
    tick_until_download_step(*app, [&] { return app->update.phase == Phase::Declined; },
        "the start to fail");
    CHECK(app->update.text == "The installer didn't start, so Hydra wasn't updated.");
    CHECK(app->update.detail == "Access is denied.");
    REQUIRE(app->update.release.has_value());
    CHECK(exists(w.canned.launched));  // kept for Update to try again
    CHECK(w.canned.downloads == 1);

    // The three buttons are back: Update starts the kept file again.
    w.canned.launch = [] { return hydra::ui::InstallerEnded([] { return false; }); };
    app->update_now();
    tick_until_download_step(*app, [&] { return app->update.phase == Phase::Launching; },
        "the second start");
    CHECK(app->update.detail.empty());
    CHECK(w.canned.downloads == 1);
}

TEST_CASE("a download whose signature doesn't verify is refused and deleted") {
    UpdateWorld w("update_badsig");
    w.canned.sig[10] = static_cast<char>(w.canned.sig[10] ^ 0x01);
    std::unique_ptr<AppState> app = offered_app(w);
    app->update_now();
    tick_until_download_step(*app, [&] { return app->update.phase == Phase::Failed; },
        "the refusal");
    CHECK(app->update.text == hydra::app::kUpdateUnverifiedSentence);
    CHECK_FALSE(app->update.detail.empty());
    CHECK(w.canned.launched.empty());
    CHECK_FALSE(exists(hydra::join_folder(w.folder, "HydraDeluxe-9.9.9-setup.exe")));
}

TEST_CASE("Cancel during the download goes back to the offer and leaves no part-file") {
    UpdateWorld w("update_cancel");
    // GitHub's real v2.10.2 reply (latest.json) with only its tag moved past
    // the running version, so the start-up check offers it. Its installer
    // keeps the size GitHub stated; the download is cancelled before any
    // digest check, so sample.bin's digest is not needed here.
    {
        nlohmann::json reply =
            nlohmann::json::parse(hydra::read_file_text(update_dir() + "/latest.json"));
        reply["tag_name"] = "v9.9.9";
        w.canned.reply = reply.dump();
    }
    const uint64_t stated_size = hydra::app::parse_latest_release_json(w.canned.reply).installer_size;
    REQUIRE(stated_size > 0);
    std::atomic<bool> started{false};
    // A body that fills the first buffer the real download_to_file offers,
    // then waits for the Cancel; download_to_file writes the part-file and
    // removes it on the cancel.
    w.canned.download = [&started](const std::string& path, uint64_t size_cap,
                                   std::atomic<uint64_t>* received,
                                   const std::atomic<bool>* cancel) {
        int reads = 0;
        hydra::net::download_to_file(
            [&](char* buffer, size_t capacity) -> size_t {
                if (reads++ > 0) {
                    started.store(true);
                    testwait::wait_until([&] { return cancel->load(); }, "the Cancel");
                }
                std::fill(buffer, buffer + capacity, 'x');
                return capacity;
            },
            path, size_cap, received, cancel);
    };
    std::unique_ptr<AppState> app = offered_app(w);
    REQUIRE(app->update.release.has_value());
    CHECK(app->update.release->installer_size == stated_size);
    const std::string installer =
        hydra::join_folder(w.folder, hydra::ui::update_installer_name(*app->update.release));
    app->update_now();
    tick_until_download_step(*app, [&] { return started.load(); }, "the download to start");
    CHECK(exists(installer + ".part"));
    const uint64_t received = app->update_received();
    CHECK(received > 0);
    // The strip's line reads the bytes so far, then the stated size, each in
    // megabytes of 1,000,000 bytes shown to one decimal place, so each figure
    // is within half a tenth of the exact one.
    const std::string line = app->update_megabytes_text();
    INFO(line);
    std::istringstream words(line);
    double shown_received = -1.0;
    double shown_total = -1.0;
    std::string of;
    std::string unit;
    words >> shown_received >> of >> shown_total >> unit;
    CHECK_FALSE(words.fail());
    CHECK(words.eof());
    CHECK(of == "of");
    CHECK(unit == "MB");
    const double half_tenth = 0.05 + 1e-9;
    CHECK(std::abs(shown_received - static_cast<double>(received) / 1e6) <= half_tenth);
    CHECK(std::abs(shown_total - static_cast<double>(stated_size) / 1e6) <= half_tenth);

    app->cancel_update();
    tick_until_download_step(*app, [&] { return app->update.phase == Phase::Offered; },
        "the offer to come back");
    CHECK(app->update.text == "Hydra Deluxe 9.9.9 is available.");
    CHECK_FALSE(exists(installer + ".part"));
    CHECK_FALSE(exists(installer));
    CHECK(w.canned.launched.empty());
}

// ---- The first start after a version change --------------------------------

TEST_CASE("the first start after a version change says so once") {
    ReplayWorld w("update_updated");
    // A test build is not the installed exe, so no check runs beside the note.
    REQUIRE_FALSE(hydra::app::installed_by_setup());
    {
        Settings s;
        s.last_run_version = "0.0.1";
        std::unique_ptr<AppState> app = w.app_with(s);
        CHECK(app->update.phase == Phase::Updated);
        CHECK(app->update.text == "Updated to Hydra Deluxe " + running_text() + ".");
        CHECK(Settings::load_file(w.paths.ini).last_run_version == running_text());
        app->dismiss_update_note();
        CHECK(app->update.phase == Phase::None);
        CHECK(app->update.text.empty());
    }
    // The next start, at the same version, shows nothing and writes nothing.
    {
        const std::string before = hydra::read_file_text(w.paths.ini);
        std::unique_ptr<AppState> app = w.app_with(Settings::load_file(w.paths.ini));
        CHECK(app->update.phase == Phase::None);
        CHECK(hydra::read_file_text(w.paths.ini) == before);
    }
    // A first start ever (no value) shows nothing and writes the version once.
    {
        std::remove(w.paths.ini.c_str());
        std::unique_ptr<AppState> app = w.app_with(Settings{});
        CHECK(app->update.phase == Phase::None);
        CHECK(Settings::load_file(w.paths.ini).last_run_version == running_text());
    }
    CHECK(w.launcher_calls.load() == 0);
}

TEST_CASE("the next start deletes the kept download folder") {
    ReplayWorld w("update_leftover");
    // A test build is not the installed exe, so the start runs no check.
    REQUIRE_FALSE(hydra::app::installed_by_setup());
    // The real update folder's answer, which ReplayWorld has required lies
    // inside the scratch temp folder.
    const std::string folder = hydra::ui::update_folder();
    // What a declined start leaves: the installer under the name the job
    // gives it, the installer's log beside it ("setup.log", named by
    // installer_log_path in app_state.cpp, which is not public), and a file
    // in a folder below, so a removal of single files would leave something.
    const hydra::app::UpdateRelease& release = w.replay.release();
    const std::string sub = hydra::join_folder(folder, "sub");
    const std::vector<std::string> left = {
        hydra::join_folder(folder, hydra::ui::update_installer_name(release)),
        hydra::join_folder(folder, "setup.log"),
        hydra::join_folder(sub, "inner.txt"),
    };
    std::filesystem::create_directories(hydra::os_path(sub));
    for (const std::string& path : left) {
        testtemp::write_bytes(path, {1, 2, 3});
        REQUIRE(exists(path));
    }
    std::unique_ptr<AppState> app = w.app();
    for (const std::string& path : left) {
        CAPTURE(path);
        CHECK_FALSE(exists(path));
    }
    CHECK_FALSE(std::filesystem::exists(hydra::os_path(folder)));
    CHECK(w.launcher_calls.load() == 0);
}
