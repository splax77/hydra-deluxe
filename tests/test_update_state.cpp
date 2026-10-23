// Tests for the updater's state in AppState (ui/update_job.h, ui/app_state.h)
// and the "is a check due" rule (app::update_check_due). Plan:
// docs/superpowers/plans/2026-10-22-auto-updater.md, task T3a, criteria C2 to
// C9 (C1 is in test_config.cpp). Nothing here touches the network: the release
// reply, the download and the installer start all go through the test seams.

#include "doctest.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "app/config.h"
#include "app/update_verify.h"  // sha256_file
#include "app/user_messages.h"
#include "core/app_version.h"
#include "core/error_kind.h"
#include "core/winstr.h"
#include "json.hpp"
#include "net/http.h"
#include "scoped_hook.h"
#include "scratch_paths.h"
#include "temp_util.h"
#include "testdata_dirs.h"
#include "ui/app_state.h"
#include "ui/update_job.h"
#include "wait_util.h"

using hydra::ErrorKind;
using hydra::KindedError;
using hydra::app::Settings;
using hydra::ui::AppState;
using Phase = hydra::ui::UpdateState::Phase;
using json = nlohmann::json;
using std::chrono::hours;
using std::chrono::system_clock;

namespace {

std::string update_dir() { return corpus::testdata_root() + "/update"; }
std::string sample_path() { return update_dir() + "/sample.bin"; }

std::string read_bytes(const std::string& path) {
    const std::vector<uint8_t> b = hydra::read_file_bytes(path);
    return std::string(b.begin(), b.end());
}

// test-public-key.txt: the 64 key bytes as hex, as release_keygen.ps1 wrote them.
std::array<uint8_t, 64> test_public_key() {
    std::string text = hydra::read_file_text(update_dir() + "/test-public-key.txt");
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) text.pop_back();
    REQUIRE(text.size() == 128);
    std::array<uint8_t, 64> key{};
    for (size_t i = 0; i < key.size(); ++i)
        key[i] = static_cast<uint8_t>(std::stoul(text.substr(2 * i, 2), nullptr, 16));
    return key;
}

std::string running_text() { return hydra::core::version_text(hydra::core::running_version()); }

// The real v2.10.2 reply with its tag set to `tag`, and its installer asset
// describing testdata/update/sample.bin (size and digest), so a download of
// sample.bin passes the digest check.
std::string reply_at(const std::string& tag) {
    json reply = json::parse(hydra::read_file_text(update_dir() + "/latest.json"));
    reply["tag_name"] = tag;
    for (json& asset : reply["assets"]) {
        const std::string name = asset["name"];
        if (name.size() > 4 && name.substr(name.size() - 4) == ".exe") {
            asset["size"] = std::filesystem::file_size(hydra::os_path(sample_path()));
            asset["digest"] = "sha256:" + hydra::app::sha256_file(sample_path());
        }
    }
    return reply.dump();
}

// What the canned network serves. `fail`, when set, is thrown by the
// releases/latest fetch instead of the reply.
struct Canned {
    std::string reply = reply_at("v9.9.9");
    std::string sig = read_bytes(update_dir() + "/sample.bin.sig");
    std::function<void()> fail;
    std::atomic<int> downloads{0};
    // The download's body: copies sample.bin unless replaced.
    std::function<void(const std::string& path, std::atomic<uint64_t>* received,
                       const std::atomic<bool>* cancel)>
        download = [](const std::string& path, std::atomic<uint64_t>* received,
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
                     [this](const std::string&, const std::string& path, uint64_t,
                            std::atomic<uint64_t>* received, const std::atomic<bool>* cancel) {
                         ++canned.downloads;
                         canned.download(path, received, cancel);
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

// The offer of the canned v9.9.9, from a manual check.
std::unique_ptr<AppState> offered_app(UpdateWorld& w) {
    std::unique_ptr<AppState> app = w.app();
    settle_check(*app);  // the construction's automatic check
    REQUIRE(app->update.phase == Phase::Offered);
    return app;
}

bool exists(const std::string& path) { return hydra::file_exists_utf8(path); }

}  // namespace

// ---- C2: update_check_due --------------------------------------------------

TEST_CASE("update_check_due: on, installed and more than a day since the last check") {
    const system_clock::time_point now = system_clock::from_time_t(1'800'000'000);
    Settings s;
    s.check_updates = true;
    {
        const ScopedHook installed(hydra::app::set_installed_for_test, std::optional<bool>(true));
        CHECK(hydra::app::update_check_due(s, now));  // no timestamp yet

        s.last_update_check = hydra::app::update_check_time_text(now - hours(23));
        CHECK_FALSE(hydra::app::update_check_due(s, now));
        s.last_update_check = hydra::app::update_check_time_text(now - hours(24));
        CHECK_FALSE(hydra::app::update_check_due(s, now));  // "more than", so the edge waits
        s.last_update_check =
            hydra::app::update_check_time_text(now - hours(24) - std::chrono::seconds(1));
        CHECK(hydra::app::update_check_due(s, now));
        s.last_update_check = hydra::app::update_check_time_text(now - hours(25));
        CHECK(hydra::app::update_check_due(s, now));
        // A time after now, or one that can't be read, doesn't hold the check back.
        s.last_update_check = hydra::app::update_check_time_text(now + hours(1));
        CHECK(hydra::app::update_check_due(s, now));
        s.last_update_check = "yesterday";
        CHECK(hydra::app::update_check_due(s, now));

        s.last_update_check.clear();
        s.check_updates = false;
        CHECK_FALSE(hydra::app::update_check_due(s, now));  // off
        s.check_updates = true;
    }
    const ScopedHook not_installed(hydra::app::set_installed_for_test, std::optional<bool>(false));
    CHECK_FALSE(hydra::app::update_check_due(s, now));  // not the installed exe
}

TEST_CASE("the last-check time is ISO-8601 UTC and reads back to the second") {
    const system_clock::time_point t = system_clock::from_time_t(1'800'000'000);
    CHECK(hydra::app::update_check_time_text(t) == "2027-01-15T08:00:00Z");
    CHECK(hydra::app::update_check_time(hydra::app::update_check_time_text(t)) == t);
    CHECK_FALSE(hydra::app::update_check_time("").has_value());
    CHECK_FALSE(hydra::app::update_check_time("2027-01-15T08:00:00").has_value());  // no zone
    CHECK_FALSE(hydra::app::update_check_time("2027-02-31T08:00:00Z").has_value());
    CHECK_FALSE(hydra::app::update_check_time("2027-01-15T08:00:00Z junk").has_value());
}

// ---- C3, C4, C5: the check, Skip this version and Later --------------------

TEST_CASE("a check offers a newer release and stays quiet about the same or an older one") {
    UpdateWorld w("update_offer");
    {
        std::unique_ptr<AppState> app = w.app();
        settle_check(*app);
        CHECK(app->update.phase == Phase::Offered);
        CHECK(app->update.text == "Hydra Deluxe 9.9.9 is available.");
        REQUIRE(app->update.release.has_value());
        CHECK(hydra::core::version_text(app->update.release->version) == "9.9.9");
        // The finished check starts the next day's wait.
        CHECK_FALSE(Settings::load_file(w.paths.ini).last_update_check.empty());
    }
    for (const std::string& tag : {"v" + running_text(), std::string("v0.0.1")}) {
        CAPTURE(tag);
        w.canned.reply = reply_at(tag);
        std::unique_ptr<AppState> app = w.app();
        settle_check(*app);  // the automatic check
        CHECK(app->update.phase == Phase::None);
        CHECK(app->update.text.empty());
        app->check_updates_now();
        settle_check(*app);
        CHECK(app->update.phase == Phase::None);
        CHECK(app->update.text.empty());
    }
}

TEST_CASE("Skip this version hides that version only") {
    UpdateWorld w("update_skip");
    std::unique_ptr<AppState> app = offered_app(w);
    app->skip_version();
    CHECK(app->update.phase == Phase::None);
    CHECK(Settings::load_file(w.paths.ini).skipped_update_version == "9.9.9");

    app->check_updates_now();
    settle_check(*app);
    CHECK(app->update.phase == Phase::None);  // 9.9.9 again: skipped

    w.canned.reply = reply_at("v9.9.10");
    app->check_updates_now();
    settle_check(*app);
    CHECK(app->update.phase == Phase::Offered);
    CHECK(app->update.text == "Hydra Deluxe 9.9.10 is available.");
}

TEST_CASE("Later hides the offer and writes nothing") {
    UpdateWorld w("update_later");
    std::unique_ptr<AppState> app = offered_app(w);
    const std::string before = hydra::read_file_text(w.paths.ini);
    const Settings saved = Settings::load_file(w.paths.ini);
    CHECK_FALSE(saved.last_update_check.empty());  // the check's own write
    CHECK(saved.skipped_update_version.empty());

    app->later();
    CHECK(app->update.phase == Phase::None);
    CHECK(app->update.text.empty());
    CHECK(hydra::read_file_text(w.paths.ini) == before);
}

// ---- C6: failures ----------------------------------------------------------

TEST_CASE("an automatic check fails silently; Check now shows the failure") {
    UpdateWorld w("update_fail");
    const std::vector<KindedError> failures = {
        KindedError(ErrorKind::NetHttpStatus, "GitHub returned HTTP 403", 403),
        KindedError(ErrorKind::NetUnreachable, "could not connect to the GitHub server"),
    };
    for (const KindedError& failure : failures) {
        CAPTURE(failure.what());
        w.canned.fail = [failure] { throw failure; };
        std::unique_ptr<AppState> app = w.app();
        REQUIRE(app->update_check_running());  // no check yet, so one is due
        settle_check(*app);
        CHECK(app->update.phase == Phase::None);
        CHECK(app->update.text.empty());
        CHECK(app->update.detail.empty());

        app->check_updates_now();
        settle_check(*app);
        CHECK(app->update.phase == Phase::Failed);
        CHECK(app->update.text == hydra::app::plain_error(failure));
        CHECK(app->update.detail == hydra::app::plain_error_detail(failure));
        CHECK_FALSE(app->update.text.empty());
    }
}

// ---- C7, C8: Update, the installer start and Cancel ------------------------

TEST_CASE("Update downloads, checks and starts the installer; Hydra then waits") {
    UpdateWorld w("update_started");
    std::unique_ptr<AppState> app = offered_app(w);
    app->update_now();
    CHECK(app->update.phase == Phase::Downloading);
    CHECK(app->update.text == "Downloading Hydra Deluxe 9.9.9...");
    tick_until(*app, [&] { return app->update.phase == Phase::Launching; }, "the installer to start");
    CHECK(app->update.text == "Starting the installer...");
    CHECK(w.canned.launched == hydra::join_folder(w.folder, "HydraDeluxe-9.9.9-setup.exe"));
    CHECK(w.canned.log == hydra::join_folder(w.folder, "setup.log"));
    CHECK(exists(w.canned.launched));
    // The installer still runs: Hydra stays as it is until the installer
    // closes it, and the strip's buttons do nothing meanwhile.
    for (int i = 0; i < 5; ++i) app->tick(0.0);
    app->later();
    app->check_updates_now();
    CHECK(app->update.phase == Phase::Launching);
    CHECK_FALSE(app->update_check_running());
}

TEST_CASE("an installer that ends while Hydra runs leaves decision 10's state and the file") {
    UpdateWorld w("update_declined");
    auto ended = std::make_shared<std::atomic<bool>>(false);
    w.canned.launch = [ended] { return hydra::ui::InstallerEnded([ended] { return ended->load(); }); };
    std::unique_ptr<AppState> app = offered_app(w);
    app->update_now();
    tick_until(*app, [&] { return app->update.phase == Phase::Launching; }, "the installer to start");
    ended->store(true);  // the UAC prompt was declined, so Setup exits
    app->tick(0.0);
    CHECK(app->update.phase == Phase::Declined);
    CHECK(app->update.text == "The installer didn't start, so Hydra wasn't updated.");
    CHECK(exists(w.canned.launched));
    CHECK(w.canned.downloads == 1);

    // Update again starts the kept file, checked again, not downloaded again.
    ended->store(false);
    app->update_now();
    tick_until(*app, [&] { return app->update.phase == Phase::Launching; }, "the second start");
    CHECK(w.canned.downloads == 1);
}

TEST_CASE("an installer that can't start fails through plain_error") {
    UpdateWorld w("update_nostart");
    const std::runtime_error why("the installer didn't start: access denied");
    w.canned.launch = [why]() -> hydra::ui::InstallerEnded { throw why; };
    std::unique_ptr<AppState> app = offered_app(w);
    app->update_now();
    tick_until(*app, [&] { return app->update.phase == Phase::Failed; }, "the start to fail");
    CHECK(app->update.text == hydra::app::plain_error(why));
    CHECK(app->update.detail == hydra::app::plain_error_detail(why));
}

TEST_CASE("a download whose signature doesn't verify is refused and deleted") {
    UpdateWorld w("update_badsig");
    w.canned.sig[10] = static_cast<char>(w.canned.sig[10] ^ 0x01);
    std::unique_ptr<AppState> app = offered_app(w);
    app->update_now();
    tick_until(*app, [&] { return app->update.phase == Phase::Failed; }, "the refusal");
    CHECK(app->update.text == hydra::app::kUpdateUnverifiedSentence);
    CHECK_FALSE(app->update.detail.empty());
    CHECK(w.canned.launched.empty());
    CHECK_FALSE(exists(hydra::join_folder(w.folder, "HydraDeluxe-9.9.9-setup.exe")));
}

TEST_CASE("Cancel during the download goes back to the offer and leaves no part-file") {
    UpdateWorld w("update_cancel");
    std::atomic<bool> started{false};
    // A body that hands out 100 bytes, then waits for the Cancel; the real
    // download_to_file writes the part-file and removes it on the cancel.
    w.canned.download = [&started](const std::string& path, std::atomic<uint64_t>* received,
                                   const std::atomic<bool>* cancel) {
        int reads = 0;
        hydra::net::download_to_file(
            [&](char* buffer, size_t capacity) -> size_t {
                if (reads++ > 0) {
                    started.store(true);
                    testwait::wait_until([&] { return cancel->load(); }, "the Cancel");
                }
                const size_t n = std::min<size_t>(100, capacity);
                std::fill(buffer, buffer + n, 'x');
                return n;
            },
            path, 1'000'000, received, cancel);
    };
    std::unique_ptr<AppState> app = offered_app(w);
    app->update_now();
    tick_until(*app, [&] { return started.load(); }, "the download to start");
    const std::string installer = hydra::join_folder(w.folder, "HydraDeluxe-9.9.9-setup.exe");
    CHECK(exists(installer + ".part"));
    CHECK(app->update_received() == 100);
    CHECK(app->update_megabytes_text() == "0.0 of 0.0 MB");

    app->cancel_update();
    tick_until(*app, [&] { return app->update.phase == Phase::Offered; }, "the offer to come back");
    CHECK(app->update.text == "Hydra Deluxe 9.9.9 is available.");
    CHECK_FALSE(exists(installer + ".part"));
    CHECK_FALSE(exists(installer));
    CHECK(w.canned.launched.empty());
}

// ---- C9: the first start after a version change ----------------------------

TEST_CASE("the first start after a version change says so once") {
    UpdateWorld w("update_updated");
    const ScopedHook off(hydra::app::set_installed_for_test, std::optional<bool>(false));
    {
        Settings s;
        s.last_run_version = "0.0.1";
        std::unique_ptr<AppState> app = w.app_with(s);
        CHECK(app->update.phase == Phase::Updated);
        CHECK(app->update.text == "Updated to Hydra Deluxe " + running_text() + ".");
        CHECK(Settings::load_file(w.paths.ini).last_run_version == running_text());
        app->dismiss_updated();
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
}

TEST_CASE("the next start deletes the kept download folder") {
    UpdateWorld w("update_leftover");
    const ScopedHook off(hydra::app::set_installed_for_test, std::optional<bool>(false));
    const std::string kept = hydra::join_folder(w.folder, "HydraDeluxe-9.9.9-setup.exe");
    testtemp::write_bytes(kept, {1, 2, 3});
    REQUIRE(exists(kept));
    std::unique_ptr<AppState> app = w.app();
    CHECK_FALSE(exists(kept));
}
