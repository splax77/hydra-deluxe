// The file half of net::http_download (download_to_file): the chunked write,
// the size cap and the part-file rename, driven by a body reader with no
// socket. Plus the set_downloader seam that stands in for the network.

#include "doctest.h"

#include <algorithm>
#include <atomic>
#include <functional>
#include <string>
#include <vector>

#include "core/error_kind.h"
#include "core/winstr.h"
#include "net/http.h"
#include "scoped_hook.h"
#include "temp_util.h"

using hydra::ErrorKind;
using hydra::KindedError;

namespace {

// A body of `total` bytes (0, 1, 2, ... mod 256) handed out at most `piece`
// bytes per read, so the writer sees several chunks. `on_read`, when set, runs
// before each read with the bytes handed out so far.
struct FakeBody {
    size_t total;
    size_t piece;
    size_t sent = 0;
    std::function<void(size_t)> on_read;

    size_t operator()(char* buffer, size_t capacity) {
        if (on_read) on_read(sent);
        const size_t n = std::min({piece, capacity, total - sent});
        for (size_t i = 0; i < n; ++i) buffer[i] = static_cast<char>((sent + i) & 0xFF);
        sent += n;
        return n;
    }
};

// A fresh scratch folder and the download's final path inside it.
struct DownloadDir {
    std::string dir = testtemp::temp_dir("http_download");
    std::string path = hydra::join_folder(dir, "setup.exe");
    DownloadDir() {
        testtemp::remove_dir(dir);
        testtemp::temp_dir("http_download");
    }
    ~DownloadDir() { testtemp::remove_dir(dir); }
    bool final_exists() const { return hydra::file_exists_utf8(path); }
    bool part_exists() const { return hydra::file_exists_utf8(path + ".part"); }
};

}  // namespace

TEST_CASE("download_to_file writes a body that fits its cap and counts every byte") {
    DownloadDir d;
    std::atomic<uint64_t> received{0};
    FakeBody body{100, 30};
    hydra::net::download_to_file(std::ref(body), d.path, 100, &received, nullptr);

    CHECK(received.load() == 100);
    CHECK_FALSE(d.part_exists());
    REQUIRE(d.final_exists());
    const std::vector<uint8_t> bytes = hydra::read_file_bytes(d.path);
    REQUIRE(bytes.size() == 100);
    CHECK(bytes[0] == 0);
    CHECK(bytes[99] == 99);
}

TEST_CASE("download_to_file refuses a body past its cap and leaves no file") {
    DownloadDir d;
    std::atomic<uint64_t> received{0};
    FakeBody body{100, 30};
    try {
        hydra::net::download_to_file(std::ref(body), d.path, 99, &received, nullptr);
        FAIL("a 100-byte body under a 99-byte cap was written");
    } catch (const KindedError& e) {
        // UpdateUnverified does not exist yet (task T2a adds it); the
        // transport refuses with NetBadReply until then.
        CHECK(e.kind() == ErrorKind::NetBadReply);
    }
    CHECK_FALSE(d.part_exists());
    CHECK_FALSE(d.final_exists());
}

TEST_CASE("download_to_file stops on cancel mid-way and leaves no file") {
    DownloadDir d;
    std::atomic<bool> cancel{false};
    FakeBody body{100, 30};
    body.on_read = [&](size_t sent) {
        if (sent >= 60) cancel = true;
    };
    CHECK_THROWS_AS(hydra::net::download_to_file(std::ref(body), d.path, 100, nullptr, &cancel),
                    hydra::Cancelled);
    CHECK_FALSE(d.part_exists());
    CHECK_FALSE(d.final_exists());
}

TEST_CASE("download_to_file passes a reader's own error on and leaves no file") {
    DownloadDir d;
    FakeBody body{100, 30};
    body.on_read = [](size_t sent) {
        if (sent >= 30)
            throw KindedError(ErrorKind::NetTimeout, "could not read the response (error 12002)");
    };
    CHECK_THROWS_WITH_AS(hydra::net::download_to_file(std::ref(body), d.path, 100, nullptr, nullptr),
                         "could not read the response (error 12002)", KindedError);
    CHECK_FALSE(d.part_exists());
    CHECK_FALSE(d.final_exists());
}

TEST_CASE("http_download calls the installed downloader in place of the network") {
    std::string seen_url, seen_path;
    uint64_t seen_cap = 0;
    const ScopedHook downloader(
        hydra::net::set_downloader,
        [&](const std::string& url, const std::string& path, uint64_t size_cap,
            std::atomic<uint64_t>* received, const std::atomic<bool>*) {
            seen_url = url;
            seen_path = path;
            seen_cap = size_cap;
            if (received) received->store(size_cap);
        });
    std::atomic<uint64_t> received{0};
    hydra::net::http_download("https://example.invalid/setup.exe", "C:\\nowhere\\setup.exe", 42,
                              &received, nullptr);
    CHECK(seen_url == "https://example.invalid/setup.exe");
    CHECK(seen_path == "C:\\nowhere\\setup.exe");
    CHECK(seen_cap == 42);
    CHECK(received.load() == 42);
}
