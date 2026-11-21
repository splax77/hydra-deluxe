// Tests for net::set_address_map_for_test (src/net/http.h) and the GitHub
// replay built on it (tests/update_replay.h): with the map installed, Hydra's
// real transport asks the shared local server for GitHub's addresses and
// hands back the saved files unchanged, and an address no pair matches never
// leaves the process. Also the scratch temp folder the updater's tests run
// under, and the route form's pause.
//
// The saved files come from testdata/update/real (update_reply.h, real_dir).

#include "update_replay.h"  // first: it brings in winsock2.h ahead of windows.h

#include "doctest.h"

#include <atomic>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <initializer_list>
#include <optional>
#include <string>
#include <thread>

#include "app/update_release.h"
#include "core/error_kind.h"
#include "core/winstr.h"
#include "net/http.h"
#include "temp_util.h"
#include "ui/update_job.h"
#include "update_reply.h"
#include "wait_util.h"

using namespace hydra;
using update_fixtures::GitHubReplay;
using update_fixtures::ScopedTempFolder;

namespace {

// The text of what `call` throws; empty when it throws nothing.
template <class Call>
std::string thrown_text(Call call) {
    try {
        call();
    } catch (const std::exception& e) {
        return e.what();
    }
    return {};
}

bool file_exists(const std::string& path) { return std::filesystem::exists(os_path(path)); }

}  // namespace

TEST_CASE("address map: a mapped GitHub address is answered by the local server, byte for byte") {
    net::set_fetcher({});  // the real WinHTTP transport
    GitHubReplay replay;

    const std::string body = net::http_get(ui::kLatestReleaseUrl, nullptr, NetService::GitHub);
    CHECK(body == update_fixtures::real_reply_text());
    CHECK(replay.server().requests(replay.check_route()) == 1);
    CHECK(replay.server().total_requests() == 1);
}

TEST_CASE("address map: an address no pair matches throws, naming it, and nothing is sent") {
    net::set_fetcher({});  // the real WinHTTP transport
    net::set_downloader({});
    GitHubReplay replay;

    // A host the map does not hold, and one that only starts with a mapped
    // origin's letters.
    const std::string mapped_origin = update_fixtures::url_origin(ui::kLatestReleaseUrl);
    for (const std::string url : {std::string("https://example.com/releases/latest"),
                                  mapped_origin + ".example/releases/latest"}) {
        CAPTURE(url);
        const std::string get_error =
            thrown_text([&] { (void)net::http_get(url, nullptr, NetService::GitHub); });
        CHECK(get_error.find(url) != std::string::npos);

        testtemp::ScopedFile file("address_map_unmapped", ".bin");
        const std::string download_error = thrown_text([&] {
            net::http_download(url, file.path, replay.release().installer_size, nullptr, nullptr);
        });
        CHECK(download_error.find(url) != std::string::npos);
        CHECK_FALSE(file_exists(file.path));
        CHECK_FALSE(file_exists(file.path + ".part"));
    }
    CHECK(replay.server().total_requests() == 0);
}

TEST_CASE("address map: the saved installer downloads through the map unchanged") {
    net::set_downloader({});  // the real WinHTTP transport
    GitHubReplay replay;
    const app::UpdateRelease& release = replay.release();

    testtemp::ScopedFile file("address_map_installer", ".bin");
    std::atomic<uint64_t> received{0};
    net::http_download(release.installer_url, file.path, release.installer_size, &received, nullptr);

    const std::string saved = read_file_text(replay.installer_file());
    const std::string got = read_file_text(file.path);
    // Compared outside CHECK, so a failure prints the sizes, not 30 MB of bytes.
    const bool same = got == saved;
    CHECK_MESSAGE(same, "downloaded " << got.size() << " bytes; the saved installer holds "
                                      << saved.size());
    CHECK(received.load() == saved.size());
    CHECK_FALSE(file_exists(file.path + ".part"));
    CHECK(replay.server().requests(replay.installer_route()) == 1);
}

TEST_CASE("address map: GitHub's saved 404 reply reaches the one status rule") {
    net::set_fetcher({});  // the real WinHTTP transport
    GitHubReplay replay;
    replay.server().set_route(replay.check_route(), "404 Not Found",
                              update_fixtures::real_not_found_text());

    std::optional<ErrorKind> kind;
    std::optional<int> status;
    try {
        (void)net::http_get(ui::kLatestReleaseUrl, nullptr, NetService::GitHub);
    } catch (const KindedError& e) {
        kind = e.kind();
        status = e.http_status();
    }
    REQUIRE_MESSAGE(kind.has_value(), "http_get did not throw a KindedError for the 404 route");
    CHECK(*kind == ErrorKind::NetHttpStatus);
    REQUIRE(status.has_value());
    CHECK(*status == 404);  // the status the route sent
    CHECK(replay.server().requests(replay.check_route()) == 1);
}

TEST_CASE("scratch temp folder: update_folder lies inside it only while the helper lives") {
    std::string scratch;
    {
        ScopedTempFolder temp("address_map_tmp");
        scratch = temp.folder();
        CHECK(same_folder(ui::update_folder(), join_folder(scratch, "HydraDeluxe-update")));
    }
    CHECK_FALSE(same_folder(ui::update_folder(), join_folder(scratch, "HydraDeluxe-update")));
}

TEST_CASE("loopback server: a paused route holds a real download part-way") {
    net::set_downloader({});  // the real WinHTTP transport
    GitHubReplay replay;
    const app::UpdateRelease& release = replay.release();
    const std::string route = replay.installer_route();
    // One byte: the least that shows the body has started.
    replay.server().pause(route, 1);

    testtemp::ScopedFile file("address_map_paused", ".bin");
    const std::string part = file.path + ".part";
    std::atomic<uint64_t> received{0};
    std::atomic<bool> cancel{false};
    std::string error;
    std::thread download([&] {
        try {
            net::http_download(release.installer_url, file.path, release.installer_size, &received,
                               &cancel);
        } catch (const std::exception& e) {
            error = e.what();
        }
    });
    // The download thread is joined on every way out, so a check that stops
    // the test never leaves it running; each subcase joins it first itself.
    struct JoinOnExit {
        std::thread& thread;
        std::atomic<bool>& stop;
        ~JoinOnExit() {
            if (!thread.joinable()) return;
            stop = true;
            thread.join();
        }
    } joiner{download, cancel};
    std::string wait_error;
    try {
        testwait::wait_until([&] { return replay.server().held(route) && received.load() > 0; },
                             "the paused installer route to hold the download");
    } catch (const testwait::WaitTimeout& e) {
        wait_error = e.what();
    }
    const bool part_while_held = file_exists(part);
    const bool file_while_held = file_exists(file.path);

    SUBCASE("released: the rest arrives and the file is whole") {
        replay.server().release(route);
        download.join();
        REQUIRE_MESSAGE(wait_error.empty(), wait_error);
        CHECK(part_while_held);
        CHECK_FALSE(file_while_held);
        CHECK_MESSAGE(error.empty(), error);
        const std::string saved = read_file_text(replay.installer_file());
        const bool same = file_exists(file.path) && read_file_text(file.path) == saved;
        CHECK_MESSAGE(same, "the released download is not the saved installer");
        CHECK_FALSE(file_exists(part));
    }

    SUBCASE("cancelled: the download ends cancelled and leaves no file") {
        cancel = true;
        download.join();
        REQUIRE_MESSAGE(wait_error.empty(), wait_error);
        CHECK(part_while_held);
        CHECK(error == Cancelled().what());
        CHECK_FALSE(file_exists(file.path));
        CHECK_FALSE(file_exists(part));
    }

    CHECK(replay.server().requests(route) == 1);
}
